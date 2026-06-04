/*
 * Lab 03 — TCP vs UDP on a 3-node Ad-hoc Chain (TCP focus)
 * -------------------------------------------------------------
 * This program builds a 3-node linear IBSS (ad-hoc) Wi-Fi chain and
 * measures TCP application throughput from node 0 to node 2
 * (multi-hop via node 1). It fixes the radio to 802.11b 1 Mb/s and
 * uses OLSR routing.
 *
 * Key points:
 *  - EXACT send window is [1s, 10s] ⇒ throughput divides by 9 seconds.
 *  - TCP segment size is set via Config::SetDefault("ns3::TcpSocket::SegmentSize", ...).
 *  - Traffic generator is an OnOff application using TcpSocketFactory and a high AppRate
 *    (default 5 Mb/s) to drive the path to saturation.
 *  - Topology is a straight line at 0 m, 200 m, 400 m.
 *  - Optional CSV output, PCAP dumps, and NetAnim visualization.
 *
 * Run (examples):
 *   # Default (pkt=1200, seed=1)
 *   --run "scratch/Lab3_Cpp_TCP"
 *
 *   # Segment size 300 B, seed 2, write CSV:
 *   --run "scratch/Lab3_Cpp_TCP --pktSize=300 --seed=2 --csv=tcp3.csv"
 *
 *   # Enable PCAP / NetAnim if you want to debug:
 *   --run "scratch/Lab3_Cpp_TCP --enablePcap=1 --enableAnim=1"
 *
 * CLI flags:
 *   --pktSize    : TCP segment size in bytes (e.g., 300, 1200)
 *   --seed       : RNG run number (RngSeedManager::SetRun)
 *   --distance   : inter-node spacing (default 200 m)
 *   --appRate    : OnOff application data rate (default 5Mbps)
 *   --enablePcap : 1 → write PCAPs (promiscuous) for all nodes
 *   --enableAnim : 1 → write NetAnim XML (Lab3_TCP.xml)
 *   --csv        : optional CSV path; if empty, prints to stdout
 *
 * CSV columns:
 *   pktSize,seed,rxBytes,throughput_Mbps
 *
 * NOTE: This is a SINGLE-CASE runner (one pktSize/seed per invocation).
 *       Use a shell script to loop if you want multiple cases.
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/netanim-module.h"

#include <fstream>
#include <string>
#include <iostream>

using namespace ns3;

static void
Banner(const std::string& title)
{
  std::cout << "\n==== " << title << " ====\n";
}

int main(int argc, char* argv[])
{
  // -------- Defaults (aligned with the lab spec) --------
  uint32_t pktSize    = 1200;    // TCP segment size (bytes)
  uint32_t seedRun    = 1;       // RNG run selector
  double   distance   = 200.0;   // meters between neighbors
  std::string appRate = "5Mbps"; // TCP OnOff offered rate (high to saturate)
  bool enablePcap     = false;   // PCAP off by default
  bool enableAnim     = false;   // NetAnim off by default
  std::string csvPath = "";      // empty → print results to stdout

  CommandLine cmd;
  cmd.AddValue("pktSize",    "TCP segment size (bytes).", pktSize);
  cmd.AddValue("seed",       "RNG run number.",            seedRun);
  cmd.AddValue("distance",   "Inter-node spacing (m).",    distance);
  cmd.AddValue("appRate",    "OnOff application data rate.", appRate);
  cmd.AddValue("enablePcap", "Enable per-node PCAP traces.", enablePcap);
  cmd.AddValue("enableAnim", "Write NetAnim XML.",           enableAnim);
  cmd.AddValue("csv",        "If non-empty, write CSV to this path.", csvPath);
  cmd.Parse(argc, argv);

  // Sanity
  if (pktSize < 64)
  {
    std::cerr << "WARNING: pktSize < 64 B; TCP may suffer excessive header overhead.\n";
  }

  // Fixed global seed; vary the run only (so students can average across runs)
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(seedRun);

  // Timing: app starts at 1 s and stops at 10 s => 9 s window
  const double appStart = 1.0;
  const double appStop  = 10.0;
  const double simStop  = 11.0;
  const double txWindow = appStop - appStart; // should be 9.0

  // -------- Topology: 3 nodes line (0 m, d, 2d) --------
  NodeContainer nodes;
  nodes.Create(3);

  // -------- PHY/Channel: 802.11b @ 1 Mb/s, Two-Ray Ground --------
  YansWifiChannelHelper channel;
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss("ns3::TwoRayGroundPropagationLossModel");

  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);
  // Lock both data and control to 1 Mb/s (no rate adaptation)
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode",    StringValue("DsssRate1Mbps"),
                               "ControlMode", StringValue("DsssRate1Mbps"));

  // Ad-hoc (IBSS) MAC
  WifiMacHelper mac;
  mac.SetType("ns3::AdhocWifiMac");

  NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

  // Optional PCAP (helpful for debugging TCP dynamics / retransmissions)
  if (enablePcap)
  {
    phy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcap("Lab3_TCP", devices, true /*promisc*/);
  }

  // -------- Mobility: place nodes on a straight line --------
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
  pos->Add(Vector(0.0,            0.0, 0.0)); // node 0 (sender)
  pos->Add(Vector(distance,       0.0, 0.0)); // node 1 (relay)
  pos->Add(Vector(2.0 * distance, 0.0, 0.0)); // node 2 (sink)
  mobility.SetPositionAllocator(pos);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  // -------- Internet + routing (OLSR) --------
  Ipv4ListRoutingHelper list;
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRh;
  list.Add(olsr, 10);
  list.Add(staticRh, 5);

  InternetStackHelper internet;
  internet.SetRoutingHelper(list);
  internet.Install(nodes);

  Ipv4AddressHelper ip;
  ip.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifaces = ip.Assign(devices);

  // -------- TCP configuration --------
  // Set TCP segment size = pktSize. This controls how TCP cuts application data
  // into segments (MSS ~ pktSize - TCP options/headers at lower layers).
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(pktSize));

  // -------- Applications: TCP sink on node 2, TCP OnOff on node 0 --------
  const uint16_t port = 5001;

  // Sink (server) to count received bytes at the application layer
  Address sinkLocalAddr(InetSocketAddress(Ipv4Address::GetAny(), port));
  PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkLocalAddr);
  ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(2));
  sinkApp.Start(Seconds(0.0));
  sinkApp.Stop(Seconds(simStop));

  // Source (client) — OnOff using TCP sockets, with constant data rate
  Address sinkRemoteAddr(InetSocketAddress(ifaces.GetAddress(2), port));
  OnOffHelper onoff("ns3::TcpSocketFactory", sinkRemoteAddr);
  // Drive hard to reveal TCP’s behavior; appRate should exceed bottleneck capacity.
  onoff.SetConstantRate(DataRate(appRate), pktSize);
  ApplicationContainer srcApp = onoff.Install(nodes.Get(0));
  srcApp.Start(Seconds(appStart));
  srcApp.Stop(Seconds(appStop));

  // -------- FlowMonitor (informational; handy for debugging) --------
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> monitor = fmHelper.InstallAll();

  // -------- NetAnim (optional) --------
  AnimationInterface* anim = nullptr;
  if (enableAnim)
  {
    anim = new AnimationInterface("Lab3_TCP.xml");
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
      anim->UpdateNodeDescription(nodes.Get(i), "n" + std::to_string(i));
      anim->UpdateNodeColor(nodes.Get(i), 200, 200, 255); // light bluish
    }
  }

  // -------- Run --------
  Simulator::Stop(Seconds(simStop));
  Simulator::Run();

  // -------- Primary metric: sink-based throughput over 9 s window --------
  uint64_t rxBytes = 0;
  {
    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
    rxBytes = sink ? sink->GetTotalRx() : 0;
  }
  const double throughputMbps = (rxBytes * 8.0 / txWindow) / 1e6;

  // -------- FlowMonitor: print per-flow stats (ns-3.40 API) --------
  monitor->CheckForLostPackets();
  auto stats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(fmHelper.GetClassifier());

  Banner("FlowMonitor (per-flow) — informational");
  for (const auto& kv : stats)
  {
    FlowId id = kv.first;
    const FlowMonitor::FlowStats& s = kv.second;
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(id);

    std::cout << "Flow " << id
              << "  " << t.sourceAddress << ":" << t.sourcePort
              << " -> " << t.destinationAddress << ":" << t.destinationPort
              << " | rxBytes=" << s.rxBytes
              << " | rxPackets=" << s.rxPackets
              << " | lost=" << s.lostPackets
              << " | delaySum=" << s.delaySum.GetSeconds() << " s"
              << "\n";
  }

  // -------- Human-readable summary --------
  Banner("TCP 3-node chain results");
  std::cout << "Config:\n"
            << "  pktSize   = " << pktSize << " B (TCP segment size)\n"
            << "  seed(run) = " << seedRun << "\n"
            << "  distance  = " << distance << " m (0, " << distance << ", " << 2*distance << ")\n"
            << "  appRate   = " << appRate << "\n\n";
  std::cout << "Throughput (sink-based, authoritative): "
            << throughputMbps << " Mb/s"
            << "  (" << rxBytes << " bytes over " << txWindow << " s)\n";

  // -------- Optional CSV output (one line) --------
  if (!csvPath.empty())
  {
    std::ofstream ofs(csvPath, std::ios::out | std::ios::trunc);
    if (ofs.is_open())
    {
      ofs << "pktSize,seed,rxBytes,throughput_Mbps\n";
      ofs << pktSize << "," << seedRun << "," << rxBytes << "," << throughputMbps << "\n";
      ofs.close();
      std::cout << "CSV written: " << csvPath << "\n";
    }
    else
    {
      std::cerr << "ERROR: cannot open CSV path: " << csvPath << "\n";
    }
  }

  // -------- Cleanup --------
  if (anim) { delete anim; anim = nullptr; }
  Simulator::Destroy();
  return 0;
}
