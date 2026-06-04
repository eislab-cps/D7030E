/* 
 * Lab 03 — Ad-hoc Wi-Fi Multi-hop Chain (UDP)
 * -------------------------------------------------------------
 * This starter builds a linear IBSS (ad-hoc) Wi-Fi chain and measures
 * application-level throughput from node 0 to the last node.
 *
 * What this file does (and why):
 *  - Forces 802.11b @ 1 Mb/s (ConstantRate) to keep the radio mode fixed.
 *  - Uses 200 m spacing so only adjacent
 *    nodes can hear each other → true multi-hop path.
 *  - Enables proactive routing (OLSR) so packets actually forward.
 *  - Drives a UDP OnOff source hard enough to saturate the path.
 *  - Measures throughput using BOTH FlowMonitor and the sink app, over the
 *    actual send window (1..10 s → 9 s). The sink-based number is the
 *    authoritative one; FlowMonitor is printed for sanity.
 *  - Exposes clean CLI flags so you can sweep params from the shell.
 *
 * Run (examples):
 *   --run "scratch/Lab3_Cpp_Adhoc --numNodes=6 --pktSize=1200 --distance=200 --seed=2"
 *   --run "scratch/Lab3_Cpp_Adhoc --numNodes=4 --pktSize=700  --distance=200 --seed=1 --appRate=1Mbps"
 *
 * Key CLI flags:
 *   --numNodes   : number of stations in the chain (>= 3)
 *   --pktSize    : UDP payload bytes (e.g., 300, 700, 1200)
 *   --distance   : inter-node spacing in meters (default 200)
 *   --seed       : RNG run number (affects backoff, loss draws, etc.)
 *   --appRate    : OnOff application data rate (default 1Mbps)
 *   --enablePcap : 1→write per-node PCAPs for debugging (default 0)
 *   --enableAnim : 1→write NetAnim XML (default 0)
 *
 * Notes:
 *   - TX window is exactly [1s, 10s], so divide bytes by 9 s for throughput.
 *   - If you see ~0 throughput, you almost certainly disabled/removed routing,
 *     or broke the geometry (e.g., too small spacing so everyone hears everyone).
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/aodv-helper.h"
#include "ns3/netanim-module.h"   // only used if --enableAnim=1

using namespace ns3;

static void
Banner(const std::string& title)
{
  std::cout << "\n==== " << title << " ====\n";
}

int main(int argc, char* argv[])
{
  // -------- CLI defaults (safe, lab-accurate) --------
  uint32_t    numNodes   = 6;                // chain length (min 3)
  uint32_t    pktSize    = 1200;             // UDP payload (bytes)
  double      distance   = 200.0;            // meters between neighbors
  uint32_t    seedRun    = 1;                // RNG "run" selector
  std::string appRate    = "1Mbps";          // push traffic to saturate
  std::string routing    = "olsr";           // routing protocol: olsr|aodv
  bool        enablePcap = false;            // packet traces (pcap) off by default
  bool        enableAnim = false;            // NetAnim XML off by default

  // -------- Parse CLI --------
  CommandLine cmd;
  cmd.AddValue("numNodes",   "Number of nodes in chain (>=3).", numNodes);
  cmd.AddValue("pktSize",    "UDP payload size in bytes.",       pktSize);
  cmd.AddValue("distance",   "Inter-node spacing in meters.",    distance);
  cmd.AddValue("seed",       "RNG run number (RngSeedManager::SetRun).", seedRun);
  cmd.AddValue("appRate",    "OnOff application data rate.",     appRate);
  cmd.AddValue("routing",    "Routing protocol: olsr or aodv.",  routing);
  cmd.AddValue("enablePcap", "Enable per-node PCAP traces.",     enablePcap);
  cmd.AddValue("enableAnim", "Write NetAnim XML.",               enableAnim);
  cmd.Parse(argc, argv);

  if (numNodes < 3)
  {
    std::cerr << "ERROR: numNodes must be >= 3 for a multi-hop chain.\n";
    return 1;
  }

  // Fix the global seed (deterministic across machines), vary the run per CLI.
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(seedRun);

  // Simulation time: allow apps to run 1..10 s; stop at a little after to flush.
  const double appStart = 1.0;
  const double appStop  = 10.0;
  const double simStop  = 11.0;  // a bit of tail for stats/teardown
  const double txWindow = appStop - appStart; // should be 9.0 s

  // -------- Create nodes --------
  NodeContainer nodes;
  nodes.Create(numNodes);

  // -------- PHY + channel: 802.11b @ 1 Mb/s, Two-Ray Ground --------
  // Channel: Two-Ray so received power falls off with d^4 after cross-over,
  // giving us a reasonable adjacency-only range at ~200 m spacing.
  YansWifiChannelHelper channel;
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  // Pick MaxRange strictly between d and 2d (here: 1.5 * d)
  double R = distance * 1.5;
  channel.AddPropagationLoss("ns3::RangePropagationLossModel",
                           "MaxRange", DoubleValue(R));

  //channel.AddPropagationLoss("ns3::TwoRayGroundPropagationLossModel", "Frequency",   DoubleValue(2.412e9));
  //channel.AddPropagationLoss("ns3::TwoRayGroundPropagationLossModel", "Frequency",   DoubleValue(2.412e9));
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());
  // FORCE 2.4 GHz (channel 1 = 2412 MHz, 20 MHz)
  //phy.Set("OperatingChannel", StringValue("{1, 0, BAND_2_4GHZ, 0}"));
  // Optional: hold TX power fixed (default is usually fine for this lab)
  // phy.Set("TxPowerStart", DoubleValue(16.0)); 
  // phy.Set("TxPowerEnd",   DoubleValue(16.0));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);
  // Lock data/control to 1 Mb/s so the MAC/PHY don't change MCS with range.
  wifi.SetRemoteStationManager(
      "ns3::ConstantRateWifiManager",
      "DataMode",    StringValue("DsssRate1Mbps"),
      "ControlMode", StringValue("DsssRate1Mbps"));

  // Ad-hoc MAC (IBSS). No AP/STA roles here.
  WifiMacHelper mac;
  mac.SetType("ns3::AdhocWifiMac");

  NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

  // Optionally enable pcap traces (useful for debugging collisions, RTS/CTS, etc.)
  if (enablePcap)
  {
    phy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcap("Lab3_Adhoc", devices, true /* promiscuous */);
  }

  // -------- Mobility: straight line, equally spaced --------
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
  for (uint32_t i = 0; i < numNodes; ++i)
  {
    pos->Add(Vector(distance * i, 0.0, 0.0));
  }
  mobility.SetPositionAllocator(pos);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  // -------- Internet + routing (REQUIRED for multi-hop) --------
  Ipv4ListRoutingHelper list;
  Ipv4StaticRoutingHelper staticRh;
  if (routing == "aodv")
  {
    AodvHelper aodv;
    list.Add(aodv, 10);
  }
  else
  {
    OlsrHelper olsr;
    list.Add(olsr, 10);
  }
  list.Add(staticRh, 5);

  InternetStackHelper internet;
  internet.SetRoutingHelper(list);
  internet.Install(nodes);

  std::cout << "Routing protocol: " << routing << "\n";

  // One subnet for the whole IBSS is fine; everyone is in radio range of neighbors only.
  Ipv4AddressHelper ip;
  ip.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifaces = ip.Assign(devices);

  // -------- Applications: UDP sink (last node) + OnOff source (node 0) --------
  uint16_t port = 5000;

  // PacketSink (server) to count received bytes.
  Address sinkLocalAddr(InetSocketAddress(Ipv4Address::GetAny(), port));
  PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", sinkLocalAddr);
  ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(numNodes - 1));
  sinkApp.Start(Seconds(0.0));
  sinkApp.Stop(Seconds(simStop));

  // OnOff (client) to drive traffic toward the sink.
  Address sinkRemoteAddr(InetSocketAddress(ifaces.GetAddress(numNodes - 1), port));
  OnOffHelper onoff("ns3::UdpSocketFactory", sinkRemoteAddr);
  onoff.SetConstantRate(DataRate(appRate), pktSize);
  // Optional: you can lower DutyCycle randomness; defaults are fine for saturation.

  ApplicationContainer srcApp = onoff.Install(nodes.Get(0));
  srcApp.Start(Seconds(appStart));
  srcApp.Stop(Seconds(appStop));

  // -------- FlowMonitor (secondary stats; nice for debugging) --------
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> monitor = fmHelper.InstallAll();

  // -------- NetAnim (optional visualization) --------
  AnimationInterface* anim = nullptr;
  if (enableAnim)
  {
    anim = new AnimationInterface("Lab3_Adhoc.xml");
    // Label nodes with index to make hop-count obvious in the animator.
    for (uint32_t i = 0; i < numNodes; ++i)
    {
      anim->UpdateNodeDescription(nodes.Get(i), "n" + std::to_string(i));
      anim->UpdateNodeColor(nodes.Get(i), 200, 200, 200); // light gray
    }
  }

  // -------- Run --------
  Simulator::Stop(Seconds(simStop));
  Simulator::Run();

  // -------- Compute throughput over the real TX window (authoritative) --------
  // Use the sink app's byte counter — simplest and most robust.
  uint64_t rxBytes = 0;
  {
    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
    rxBytes = sink ? sink->GetTotalRx() : 0;
  }
  double throughput_bps   = (rxBytes * 8.0) / txWindow;

 // -------- Also print FlowMonitor's view (sanity check) --------
monitor->CheckForLostPackets();
auto stats = monitor->GetFlowStats();
Ptr<Ipv4FlowClassifier> classifier =
    DynamicCast<Ipv4FlowClassifier>(fmHelper.GetClassifier());

Banner("FlowMonitor (per-flow) — informational");
for (const auto& kv : stats)
{
  FlowId id = kv.first;
  const FlowMonitor::FlowStats& s = kv.second;

  // ns-3.40 API: FindFlow(flowId) -> FiveTuple
  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(id);

  std::cout << "Flow " << id
            << "  " << t.sourceAddress << ":" << t.sourcePort
            << " -> " << t.destinationAddress << ":" << t.destinationPort
            << " | rxBytes=" << s.rxBytes
            << " | rxPackets=" << s.rxPackets
            << " | delaySum=" << s.delaySum.GetSeconds() << " s"
            << " | lost=" << s.lostPackets
            << "\n";
}


  // -------- Cleanup --------
  if (anim) { delete anim; anim = nullptr; }
  Simulator::Destroy();
  return 0;
}
