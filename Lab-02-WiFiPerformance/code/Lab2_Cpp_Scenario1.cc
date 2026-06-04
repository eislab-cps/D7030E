// Lab 2: Infrastructure WiFi — Scenario 1 (Single flow via an AP)
// ------------------------------------------------------------------------------------
// GOAL (spec, Fig. 1):
//   • Three nodes form an EQUILATERAL TRIANGLE with side length = 10 m.
//     - One Access Point (AP) — infrastructure Wi‑Fi (IEEE 802.11b)
//     - One STA sender  ---->  one STA receiver (UDP traffic)
//   • Vary PHY data rate among {1, 5.5, 11} Mb/s (we pass this in as --rate)
//   • Offered load is intentionally MUCH HIGHER than the PHY (saturation), per spec note.
//   • Measure application-layer GOODPUT (bits received at the sink over active time).
//   • Repeat runs with different seeds (we pass this in as --seed).
//   • Produce a NetAnim XML called `scenario1_anim.xml`.
//
// WHAT THIS FILE FIXES vs the starter:
//   1) Correct geometry: equilateral triangle of side 10 m (NOT a line).
//   2) Saturating offered load: OnOff DataRate set to a large value (e.g., 100 Mbps),
//      independent of the PHY rate.
//   3) Deliverable‑compliant NetAnim filename: scenario1_anim.xml
//
// HOW TO RUN (from ns-3 root):
//   ./ns3 run "scratch/Lab2_Cpp_Scenario1 --rate=11 --seed=2"
// ------------------------------------------------------------------------------------

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

// --------- Utility: map numeric Mbps to a legal 802.11b WifiMode string ---------
// IEEE 802.11b supports DSSS/CCK modes at 1, 2, 5.5, and 11 Mbps.
static std::string
ModeForB (double rateMbps)
{
  if (rateMbps <= 1.0)   return "DsssRate1Mbps";
  if (rateMbps <= 2.0)   return "DsssRate2Mbps";
  if (rateMbps <= 5.5)   return "DsssRate5_5Mbps";
  /* else */             return "DsssRate11Mbps";
}

int
main (int argc, char *argv[])
{
  // ------------------------- Simulation parameters (CLI) -------------------------
  // rate:   the *PHY* data rate we lock the Wi‑Fi manager to (ConstantRateWifiManager)
  // seed:   the RngRun so we can repeat with different contention/backoff timings
  double   rate = 11.0;       // Mbps (valid: 1, 2, 5.5, 11)
  uint32_t seed = 1;          // RngRun index (use 1 and 2 per spec)
  CommandLine cmd;
  cmd.AddValue("rate", "802.11b PHY data rate in Mbps (1, 2, 5.5, 11 -> rounded up)", rate);
  cmd.AddValue("seed", "RngRun value for repeatability (use 1 and 2 for the lab)", seed);
  cmd.Parse(argc, argv);

  // ----------------------------- Randomization setup -----------------------------
  // Keep the Seed constant across all runs; vary only the Run to randomize per‑trial.
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(seed);

  // Use nanosecond resolution for events (safe default for Wi‑Fi experiments).
  Time::SetResolution(Time::NS);

  // ---------------------------- Topology: nodes & roles ---------------------------
  // We create three nodes:
  //   • 1 AP node (infrastructure BSS)
  //   • 2 STA nodes: index 0 will be the *sender*, index 1 the *receiver*.
  NodeContainer staNodes;  staNodes.Create(2);
  NodeContainer apNode;    apNode.Create(1);

  // --------------------------- Channel / PHY configuration ------------------------
  // YansWifiChannelHelper::Default() sets Friis + LogDistance + RandomLoss by default.
  // We keep defaults; the geometry (10 m sides) gives comparable path loss on all links.
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

  // ns-3.40: create a YansWifiPhyHelper and explicitly bind it to the channel.
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  // ------------------------------- Wi‑Fi MAC & rate -------------------------------
  // Lock the standard to 802.11b and force both Data/Control to the requested mode,
  // so the PHY rate is *constant* during the experiment.
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);
  const std::string mode = ModeForB(rate);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode",    StringValue(mode),
                               "ControlMode", StringValue(mode));

  WifiMacHelper mac;
  Ssid ssid = Ssid("lab2-ssid");

  // STA devices (non-AP)
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
  NetDeviceContainer staDevs = wifi.Install(phy, mac, staNodes);

  // AP device
  mac.SetType("ns3::ApWifiMac",  "Ssid", SsidValue(ssid));
  NetDeviceContainer apDev  = wifi.Install(phy, mac, apNode);

  // --------------------------------- Mobility model --------------------------------
  // Place nodes as an EQUILATERAL TRIANGLE with side length = 10 m.
  // We choose coordinates so that all pairwise distances are exactly 10 m:
  //   Let s = 10 m;   height h = s * sqrt(3) / 2 = 8.6602540378 m.
  //
  //   AP         at (0, 0)
  //   Sender STA at (-5,  h)   [left vertex]
  //   Receiver   at ( 5,  h)   [right vertex]
  //
  // Distances:
  //   |AP - Sender|   = sqrt(5^2 + h^2) = sqrt(25 + 75) = 10
  //   |AP - Receiver| = 10
  //   |Sender - Receiver| = 10
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
  const double side = 10.0;
  const double h    = side * std::sqrt(3.0) / 2.0;
  pos->Add(Vector( 0.0, 0.0, 0.0));     // AP
  pos->Add(Vector(-side/2.0, h, 0.0));  // STA sender
  pos->Add(Vector( side/2.0, h, 0.0));  // STA receiver
  mobility.SetPositionAllocator(pos);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(apNode);
  mobility.Install(staNodes);

  // ------------------------------- Internet + IP stack -----------------------------
  // Install TCP/IP on all nodes so UDP sockets work end-to-end.
  InternetStackHelper stack;
  stack.Install(apNode);
  stack.Install(staNodes);

  // Single IPv4 subnet for simplicity.
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer staIfaces = ipv4.Assign(staDevs);
  Ipv4InterfaceContainer apIface   = ipv4.Assign(apDev);

  // ------------------------------ Applications (traffic) ---------------------------
  // One UDP OnOff client (sender -> receiver).  The IMPORTANT bit:
  //   • Offered load (OnOff DataRate) is set VERY HIGH (100 Mbps), far above the PHY,
  //     to force saturation — per NOTE1 in the spec.
  //   • Packet size is 1000 B (as required).
  //   • The app runs from t=[1s,10s]; we measure goodput over the 9 s active window.
  OnOffHelper onoff("ns3::UdpSocketFactory",
                    InetSocketAddress(staIfaces.GetAddress(1), 9)); // -> receiver:port 9
  onoff.SetAttribute("DataRate",   StringValue("100Mbps"));        // saturating offered load
  onoff.SetAttribute("PacketSize", UintegerValue(1000));           // payload size
  onoff.SetAttribute("OnTime",     StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute("OffTime",    StringValue("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer client = onoff.Install(staNodes.Get(0)); // sender index 0
  client.Start(Seconds(1.0));
  client.Stop (Seconds(10.0));

  // Receiver sink on the destination STA (port 9)
  PacketSinkHelper sink("ns3::UdpSocketFactory",
                        InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer server = sink.Install(staNodes.Get(1)); // receiver index 1
  server.Start(Seconds(0.0));
  server.Stop (Seconds(10.0));

  // ---------------------------- FlowMonitor + NetAnim ------------------------------
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> monitor = fmHelper.InstallAll();

  // NetAnim trace (deliverable‑compliant filename)
  AnimationInterface anim("scenario1_anim.xml");
  // Make the animation self‑describing
  anim.SetConstantPosition(apNode.Get(0),        0.0, 0.0);
  anim.SetConstantPosition(staNodes.Get(0), -side/2.0, h);
  anim.SetConstantPosition(staNodes.Get(1),  side/2.0, h);
  anim.UpdateNodeDescription(apNode.Get(0),        "AP");
  anim.UpdateNodeDescription(staNodes.Get(0), "STA sender");
  anim.UpdateNodeDescription(staNodes.Get(1), "STA receiver");

  // ------------------------------- Run the simulation ------------------------------
  Simulator::Stop(Seconds(10.0));
  Simulator::Run();

  // ------------------------------ Throughput calculation ---------------------------
  // Sum all bytes received at the sink(s) and divide by active duration (9 s).
  monitor->CheckForLostPackets();
  const auto &stats = monitor->GetFlowStats();

  uint64_t totalRxBytes = 0;
  for (const auto &kv : stats)
  {
    totalRxBytes += kv.second.rxBytes;
  }

  const double activeSecs = 9.0;  // apps active in [1,10]s
  const double goodput_bps = (totalRxBytes * 8.0) / activeSecs;

  std::cout << "[Scenario1] PHYMode=" << mode
            << "  offered=100Mbps"
            << "  totalRxBytes=" << totalRxBytes
            << "  throughput=" << goodput_bps << " bps (" << goodput_bps/1e6 << " Mbps)"
            << std::endl;

  Simulator::Destroy();
  return 0;
}
