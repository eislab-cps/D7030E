// Lab 2: Infrastructure WiFi — Scenario 1 (Part 2): Two simultaneous flows via one AP
// ------------------------------------------------------------------------------------
// GOAL (spec, Fig. 2 "two triangles"):
//   • FIVE nodes in total: 1 AP + 4 STAs arranged as TWO EQUILATERAL TRIANGLES (side 10 m)
//       Left triangle :  [Sender_L] — [AP] — [Receiver_L]
//       Right triangle:  [Sender_R] — [AP] — [Receiver_R]
//     Each triangle’s three vertices are 10 m apart pairwise.
//   • Two independent UDP flows run in parallel:
//       Flow A: Sender_L  ->  Receiver_L  (dest port 9)
//       Flow B: Sender_R  ->  Receiver_R  (dest port 10)
//   • Keep 802.11b and LOCK PHY rate to {1, 5.5, 11} Mb/s.
//   • Offered load is MUCH HIGHER than PHY for saturation (100 Mbps per flow).
//   • Measure per‑flow goodput and aggregate goodput. Two seeds per rate.
//   • Produce a NetAnim XML called `scenario2_anim.xml`.
//
// WHY THIS SHAPE?
//   Placing two disjoint triangles around a single AP equalizes path loss on each hop in a
//   triangle and keeps the two flows symmetric. Both flows must share the same channel
//   at the AP, so CSMA/CA contention and ACK/IFS overhead will determine the split.
//
// HOW TO RUN (from ns-3 root):
//   ./ns3 run "scratch/Lab2_Cpp_Scenario2 --rate=11 --seed=1"
// ------------------------------------------------------------------------------------

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include <filesystem>

using namespace ns3;

// Map numeric Mbps to an 802.11b WifiMode string.
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
  std::filesystem::create_directories("scratch/Lab2outputs");
  double   rate = 11.0;       // PHY data rate (Mbps) to lock
  uint32_t seed = 1;          // RngRun; use 1 and 2 for the lab
  CommandLine cmd;
  cmd.AddValue("rate", "802.11b PHY data rate in Mbps (1, 2, 5.5, 11 -> rounded up)", rate);
  cmd.AddValue("seed", "RngRun value for repeatability (use 1 and 2 for the lab)", seed);
  cmd.Parse(argc, argv);

  // ----------------------------- Randomization setup -----------------------------
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(seed);
  Time::SetResolution(Time::NS);

  // ---------------------------- Topology: nodes & roles ---------------------------
  // Create 4 STA nodes (2 senders + 2 receivers) and 1 AP node.
  NodeContainer staSenders;   staSenders.Create(2);   // [0] -> Left sender, [1] -> Right sender
  NodeContainer staReceivers; staReceivers.Create(2); // [0] -> Left receiver, [1] -> Right receiver
  NodeContainer apNode;       apNode.Create(1);

  // --------------------------- Channel / PHY configuration ------------------------
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  // ------------------------------- Wi‑Fi MAC & rate -------------------------------
  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);
  const std::string mode = ModeForB(rate);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode",    StringValue(mode),
                               "ControlMode", StringValue(mode));

  WifiMacHelper mac;
  Ssid ssid = Ssid("lab2-ssid");

  // Install STA devices on senders and receivers (all in the same BSS).
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
  NetDeviceContainer devSenders   = wifi.Install(phy, mac, staSenders);
  NetDeviceContainer devReceivers = wifi.Install(phy, mac, staReceivers);

  // AP device
  mac.SetType("ns3::ApWifiMac",  "Ssid", SsidValue(ssid));
  NetDeviceContainer devAp = wifi.Install(phy, mac, apNode);

  // --------------------------------- Mobility model --------------------------------
  // Build TWO equilateral triangles (side = 10 m) that share the AP at the origin.
  //
  // Equilateral geometry helper:
  //   side s = 10 m; height h = s * sqrt(3) / 2 ≈ 8.660254 m
  // Left triangle  vertices:  (-10,0)  (-5,h)  (0,0) [AP]
  // Right triangle vertices:   (10,0)   (5,h)  (0,0) [AP]
  //
  // We choose sender at the "bottom" vertex and receiver at the "top" vertex for each side.
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
  const double s = 10.0;
  const double h = s * std::sqrt(3.0) / 2.0;

  // AP (shared vertex for both triangles)
  pos->Add(Vector(0.0, 0.0, 0.0)); // index 0 in "apNode"

  // Left triangle
  pos->Add(Vector(-10.0, 0.0, 0.0));  // Left sender   (staSenders[0])
  pos->Add(Vector(-5.0,   h,   0.0)); // Left receiver (staReceivers[0])

  // Right triangle
  pos->Add(Vector( 10.0, 0.0, 0.0));  // Right sender   (staSenders[1])
  pos->Add(Vector(  5.0,  h,  0.0));  // Right receiver (staReceivers[1])

  mobility.SetPositionAllocator(pos);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  // Install positions
  mobility.Install(apNode);
  mobility.Install(staSenders);
  mobility.Install(staReceivers);

  // ------------------------------- Internet + IP stack -----------------------------
  InternetStackHelper stack;
  stack.Install(apNode);
  stack.Install(staSenders);
  stack.Install(staReceivers);

  // Single IPv4 subnet
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer ifSenders   = ipv4.Assign(devSenders);
  Ipv4InterfaceContainer ifReceivers = ipv4.Assign(devReceivers);
  Ipv4InterfaceContainer ifAp        = ipv4.Assign(devAp);

  // -------------------------------- Applications (2 flows) ------------------------
  // Saturating offered load (100 Mbps PER FLOW), payload 1000 B, On in [1,10] s.
  // Flow A: Sender_L -> Receiver_L (port 9)
  OnOffHelper onoffA("ns3::UdpSocketFactory",
                     InetSocketAddress(ifReceivers.GetAddress(0), 9));
  onoffA.SetAttribute("DataRate",   StringValue("100Mbps"));
  onoffA.SetAttribute("PacketSize", UintegerValue(1000));
  onoffA.SetAttribute("OnTime",     StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoffA.SetAttribute("OffTime",    StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  ApplicationContainer appA = onoffA.Install(staSenders.Get(0)); // Left sender
  appA.Start(Seconds(1.0));
  appA.Stop (Seconds(10.0));

  PacketSinkHelper sinkA("ns3::UdpSocketFactory",
                         InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer srvA = sinkA.Install(staReceivers.Get(0)); // Left receiver
  srvA.Start(Seconds(0.0));
  srvA.Stop (Seconds(10.0));

  // Flow B: Sender_R -> Receiver_R (port 10)
  OnOffHelper onoffB("ns3::UdpSocketFactory",
                     InetSocketAddress(ifReceivers.GetAddress(1), 10));
  onoffB.SetAttribute("DataRate",   StringValue("100Mbps"));
  onoffB.SetAttribute("PacketSize", UintegerValue(1000));
  onoffB.SetAttribute("OnTime",     StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoffB.SetAttribute("OffTime",    StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  ApplicationContainer appB = onoffB.Install(staSenders.Get(1)); // Right sender
  appB.Start(Seconds(1.0));
  appB.Stop (Seconds(10.0));

  PacketSinkHelper sinkB("ns3::UdpSocketFactory",
                         InetSocketAddress(Ipv4Address::GetAny(), 10));
  ApplicationContainer srvB = sinkB.Install(staReceivers.Get(1)); // Right receiver
  srvB.Start(Seconds(0.0));
  srvB.Stop (Seconds(10.0));

  // ---------------------------- FlowMonitor + NetAnim ------------------------------
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> monitor = fmHelper.InstallAll();

  // NetAnim (deliverable name)
  AnimationInterface anim("scratch/Lab2outputs/scenario2_anim.xml");
  // Label nodes to make the animation self‑explanatory
  anim.UpdateNodeDescription(apNode.Get(0),           "AP");
  anim.UpdateNodeDescription(staSenders.Get(0),       "Sender_L");
  anim.UpdateNodeDescription(staReceivers.Get(0),     "Receiver_L");
  anim.UpdateNodeDescription(staSenders.Get(1),       "Sender_R");
  anim.UpdateNodeDescription(staReceivers.Get(1),     "Receiver_R");

  // Keep positions consistent in the animation
  anim.SetConstantPosition(apNode.Get(0),           0.0,  0.0);
  anim.SetConstantPosition(staSenders.Get(0),    -10.0,  0.0);
  anim.SetConstantPosition(staReceivers.Get(0),   -5.0,  h);
  anim.SetConstantPosition(staSenders.Get(1),     10.0,  0.0);
  anim.SetConstantPosition(staReceivers.Get(1),    5.0,  h);

  // ------------------------------- Run the simulation ------------------------------
  Simulator::Stop(Seconds(10.0));
  Simulator::Run();

  // ------------------------------ Throughput calculation ---------------------------
  // Compute per‑flow goodput by classifying flows using Ipv4FlowClassifier and
  // summing rxBytes for each destination port (9 and 10). Active window = 9 s.
  monitor->CheckForLostPackets();
  const auto &stats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier>(fmHelper.GetClassifier());

  uint64_t rxPort9 = 0, rxPort10 = 0;
  for (const auto &kv : stats)
  {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(kv.first);
    if (t.destinationPort == 9)   rxPort9  += kv.second.rxBytes;
    if (t.destinationPort == 10)  rxPort10 += kv.second.rxBytes;
  }

  const double activeSecs = 9.0;
  const double thrA = (rxPort9  * 8.0) / activeSecs; // bps
  const double thrB = (rxPort10 * 8.0) / activeSecs; // bps
  const double thrSum = thrA + thrB;

  std::cout << "[Scenario1-Part2] PHYMode=" << mode
            << "  offered(each)=100Mbps"
            << "  rxBytes(port9)="  << rxPort9
            << "  rxBytes(port10)=" << rxPort10 << std::endl;
  std::cout << "    throughput flowA(port9):  " << thrA    << " bps (" << thrA/1e6    << " Mbps)\n";
  std::cout << "    throughput flowB(port10): " << thrB    << " bps (" << thrB/1e6    << " Mbps)\n";
  std::cout << "    aggregate throughput:      " << thrSum  << " bps (" << thrSum/1e6  << " Mbps)\n";

  Simulator::Destroy();
  return 0;
}
