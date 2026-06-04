/*
 * Lab 03 — Hidden-Terminal Demonstration (Infrastructure MACs)
 * --------------------------------------------------------------------
 * Scenario:
 *   STA0  ---- d ----  AP  ---- d ----  STA1
 *
 * - Both STAs transmit UDP to the AP at the same time (t = 1..10 s).
 * - We FIX the PHY to 802.11b at 1 Mb/s (ConstantRate) to keep SNR/MCS constant.
 * - STA0 and STA1 are hidden from each other
 *   when d is large (e.g., 200 m), but both can hear the AP.
 * - We toggle RTS/CTS via the RtsCtsThreshold attribute on the station manager:
 *     * enableRtsCts = 0  ⇒ threshold 2200 (effectively OFF for normal frames)
 *     * enableRtsCts = 1  ⇒ threshold 0    (force RTS/CTS ON for all frames)
 *
 * What this program prints:
 *   - Throughput of each flow (Mb/s) computed over the ACTUAL TX WINDOW (1..10 s → 9 s).
 *   - Packet Delivery Ratio (PDR = rxPackets / txPackets) per flow from FlowMonitor.
 *   - A short summary + optional CSV row (for easy plotting).
 *
 * Why infrastructure MACs (AP/STA) here?
 *   Classic hidden-terminal demos use two STAs hidden from each other that collide at a
 *   common AP. That cleanly shows how RTS/CTS helps.
 *
 * Run (examples):
 *   # RTS/CTS OFF (threshold = 2200), default pktSize=1000, distance=200 m
 *   --run "scratch/Lab3_Cpp_Hidden --enableRtsCts=0"
 *
 *   # RTS/CTS ON (threshold = 0), seed=2, write CSV:
 *   --run "scratch/Lab3_Cpp_Hidden --enableRtsCts=1 --seed=2 --csv=hidden.csv"
 *
 *   # Debug with pcap and NetAnim:
 *   --run "scratch/Lab3_Cpp_Hidden --enablePcap=1 --enableAnim=1"
 *
 * CLI flags:
 *   --enableRtsCts : 0→OFF (2200), 1→ON (0)
 *   --pktSize      : UDP payload size in bytes (default 1000)
 *   --appRate      : OnOff application data rate (default 1Mbps)
 *   --distance     : spacing d in meters (default 200)
 *   --seed         : RNG run number (RngSeedManager::SetRun)
 *   --enablePcap   : 1→write per-node 802.11 Radiotap PCAPs (promisc)
 *   --enableAnim   : 1→write NetAnim XML (Lab3_Hidden.xml)
 *   --csv          : optional CSV path (append mode); if empty, prints to stdout
 *
 * CSV columns (one row per run):
 *   rtsCts,distance,pktSize,seed,thr_sta0_Mbps,thr_sta1_Mbps,thr_total_Mbps,
 *   pdr_sta0,pdr_sta1,tx0,rx0,tx1,rx1
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>

using namespace ns3;

static void Banner(const std::string& title)
{
  std::cout << "\n==== " << title << " ====\n";
}

int main(int argc, char* argv[])
{
  // -------- Defaults --------
  bool enableRtsCts = false;     // OFF by default (threshold=2200)
  uint32_t pktSize  = 1000;      // UDP payload (bytes)
  std::string appRate = "1Mbps"; // each STA source rate
  double distance   = 200.0;     // meters
  uint32_t seedRun  = 1;         // RNG run selector
  bool enablePcap   = false;     // packet capture off by default
  bool enableAnim   = false;     // NetAnim off by default
  std::string csvPath = "";      // append CSV here if non-empty

  CommandLine cmd;
  cmd.AddValue("enableRtsCts", "0→disable RTS/CTS, 1→enable RTS/CTS.", enableRtsCts);
  cmd.AddValue("pktSize",      "UDP payload bytes.",                     pktSize);
  cmd.AddValue("appRate",      "OnOff application rate (e.g., 1Mbps).",  appRate);
  cmd.AddValue("distance",     "STA0—AP and AP—STA1 spacing (m).",       distance);
  cmd.AddValue("seed",         "RNG run number.",                         seedRun);
  cmd.AddValue("enablePcap",   "Enable per-node PCAP traces.",            enablePcap);
  cmd.AddValue("enableAnim",   "Write NetAnim XML.",                      enableAnim);
  cmd.AddValue("csv",          "Append one CSV line to this path.",       csvPath);
  cmd.Parse(argc, argv);

  // Timing
  const double appStart = 1.0, appStop = 10.0, simStop = 11.0;
  const double txWindow = appStop - appStart; // 9 s

  // Seed/run
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(seedRun);

  // -------- Nodes --------
  NodeContainer sta0, ap, sta1;
  sta0.Create(1); ap.Create(1); sta1.Create(1);
  NodeContainer all; all.Add(sta0); all.Add(ap); all.Add(sta1);

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
  wifi.SetStandard(WIFI_STANDARD_80211n);
  // Lock data/control to 1 Mb/s so the MAC/PHY don't change MCS with range.
  wifi.SetRemoteStationManager(
      "ns3::ConstantRateWifiManager",
      "DataMode",    StringValue("DsssRate1Mbps"),
      "ControlMode", StringValue("DsssRate1Mbps"));
  uint32_t rtsThreshold = enableRtsCts ? 0 : 2200;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode",        StringValue("DsssRate1Mbps"),
                               "ControlMode",     StringValue("DsssRate1Mbps"),
                               "RtsCtsThreshold", UintegerValue(rtsThreshold));

  // -------- MACs (Infrastructure) --------
  Ssid ssid = Ssid("hidden-ssid");
  WifiMacHelper macSta, macAp;
  macSta.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid),
                 "ActiveProbing", BooleanValue(false));
  macAp.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));

  NetDeviceContainer devSta0 = wifi.Install(phy, macSta, sta0);
  NetDeviceContainer devAp   = wifi.Install(phy, macAp,  ap);
  NetDeviceContainer devSta1 = wifi.Install(phy, macSta, sta1);

  if (enablePcap)
  {
    phy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcap("Lab3_Hidden_sta0", devSta0, true);
    phy.EnablePcap("Lab3_Hidden_ap",   devAp,   true);
    phy.EnablePcap("Lab3_Hidden_sta1", devSta1, true);
  }

  // -------- Mobility --------
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
  pos->Add(Vector(0.0,           0.0, 0.0)); // STA0
  pos->Add(Vector(distance,      0.0, 0.0)); // AP
  pos->Add(Vector(2.0*distance,  0.0, 0.0)); // STA1
  mobility.SetPositionAllocator(pos);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(all);

  // -------- Internet --------
  InternetStackHelper internet; internet.Install(all);
  Ipv4AddressHelper ip; ip.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifSta0 = ip.Assign(devSta0);
  Ipv4InterfaceContainer ifAp   = ip.Assign(devAp);
  Ipv4InterfaceContainer ifSta1 = ip.Assign(devSta1);

  // -------- Apps: two UDP flows STA0→AP (port0) and STA1→AP (port1) --------
  const uint16_t port0 = 9000, port1 = 9001;

  // Keep explicit pointers to the two sinks (ns-3.40 PacketSink has no GetSocket()).
  Ptr<PacketSink> sink0Ptr, sink1Ptr;
  {
    PacketSinkHelper sink0("ns3::UdpSocketFactory",
                           InetSocketAddress(Ipv4Address::GetAny(), port0));
    PacketSinkHelper sink1("ns3::UdpSocketFactory",
                           InetSocketAddress(Ipv4Address::GetAny(), port1));
    ApplicationContainer a0 = sink0.Install(ap.Get(0));
    ApplicationContainer a1 = sink1.Install(ap.Get(0));
    a0.Start(Seconds(0.0)); a0.Stop(Seconds(simStop));
    a1.Start(Seconds(0.0)); a1.Stop(Seconds(simStop));
    sink0Ptr = DynamicCast<PacketSink>(a0.Get(0));
    sink1Ptr = DynamicCast<PacketSink>(a1.Get(0));
  }

  Address apAddr0(InetSocketAddress(ifAp.GetAddress(0), port0));
  Address apAddr1(InetSocketAddress(ifAp.GetAddress(0), port1));

  OnOffHelper onoff0("ns3::UdpSocketFactory", apAddr0);
  onoff0.SetConstantRate(DataRate(appRate), pktSize);
  OnOffHelper onoff1("ns3::UdpSocketFactory", apAddr1);
  onoff1.SetConstantRate(DataRate(appRate), pktSize);

  ApplicationContainer src0 = onoff0.Install(sta0.Get(0));
  ApplicationContainer src1 = onoff1.Install(sta1.Get(0));
  src0.Start(Seconds(appStart)); src0.Stop(Seconds(appStop));
  src1.Start(Seconds(appStart)); src1.Stop(Seconds(appStop));

  // -------- FlowMonitor (for PDR) --------
  FlowMonitorHelper fmHelper; Ptr<FlowMonitor> monitor = fmHelper.InstallAll();

  // -------- NetAnim (optional) --------
  AnimationInterface* anim = nullptr;
  if (enableAnim)
  {
    anim = new AnimationInterface("Lab3_Hidden.xml");
    anim->UpdateNodeDescription(sta0.Get(0), "STA0");
    anim->UpdateNodeDescription(ap.Get(0),   "AP");
    anim->UpdateNodeDescription(sta1.Get(0), "STA1");
    anim->UpdateNodeColor(sta0.Get(0), 200, 200, 200);
    anim->UpdateNodeColor(ap.Get(0),   255, 200, 200);
    anim->UpdateNodeColor(sta1.Get(0), 200, 200, 200);
  }

  // -------- Run --------
  Simulator::Stop(Seconds(simStop));
  Simulator::Run();

  // -------- Per-flow throughput from sink pointers --------
  uint64_t rxBytes0 = sink0Ptr ? sink0Ptr->GetTotalRx() : 0;
  uint64_t rxBytes1 = sink1Ptr ? sink1Ptr->GetTotalRx() : 0;
  const double thr0_Mbps = (rxBytes0 * 8.0 / txWindow) / 1e6;
  const double thr1_Mbps = (rxBytes1 * 8.0 / txWindow) / 1e6;
  const double thrT_Mbps = thr0_Mbps + thr1_Mbps;

  // -------- PDR via FlowMonitor --------
  monitor->CheckForLostPackets();
  auto stats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(fmHelper.GetClassifier());

  uint64_t tx0 = 0, rx0 = 0, tx1 = 0, rx1 = 0;
  for (const auto& kv : stats)
  {
    FlowId id = kv.first;
    const FlowMonitor::FlowStats& s = kv.second;
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(id);
    if (t.protocol != 17) continue; // 17=UDP
    if (t.destinationPort == port0) { tx0 += s.txPackets; rx0 += s.rxPackets; }
    else if (t.destinationPort == port1) { tx1 += s.txPackets; rx1 += s.rxPackets; }
  }
  const double pdr0 = (tx0 ? double(rx0)/tx0 : 0.0);
  const double pdr1 = (tx1 ? double(rx1)/tx1 : 0.0);

  // -------- Print summary --------
  Banner("Hidden-terminal results");
  std::cout << std::boolalpha
            << "RTS/CTS enabled : " << enableRtsCts << " (RtsCtsThreshold=" << (enableRtsCts ? 0 : 2200) << ")\n"
            << "Distance (m)    : " << distance << " (STA0 @ 0, AP @ " << distance << ", STA1 @ " << 2*distance << ")\n"
            << "Packet size (B) : " << pktSize << "\n"
            << "App rate        : " << appRate << " per STA\n"
            << "Seed(run)       : " << seedRun << "\n\n";

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "Throughput STA0→AP : " << thr0_Mbps << " Mb/s  ("
            << rxBytes0 << " bytes over " << txWindow << " s)\n";
  std::cout << "Throughput STA1→AP : " << thr1_Mbps << " Mb/s  ("
            << rxBytes1 << " bytes over " << txWindow << " s)\n";
  std::cout << "Total throughput   : " << thrT_Mbps << " Mb/s\n\n";

  std::cout << "PDR STA0 (rx/tx)   : " << rx0 << "/" << tx0
            << " = " << (pdr0 * 100.0) << "%\n";
  std::cout << "PDR STA1 (rx/tx)   : " << rx1 << "/" << tx1
            << " = " << (pdr1 * 100.0) << "%\n";

  // -------- Optional CSV append --------
  if (!csvPath.empty())
  {
    std::ofstream ofs(csvPath, std::ios::out | std::ios::app);
    if (ofs.is_open())
    {
      ofs << (enableRtsCts ? 1 : 0) << ","
          << distance << ","
          << pktSize << ","
          << seedRun << ","
          << thr0_Mbps << ","
          << thr1_Mbps << ","
          << thrT_Mbps << ","
          << pdr0 << ","
          << pdr1 << ","
          << tx0 << ","
          << rx0 << ","
          << tx1 << ","
          << rx1 << "\n";
      ofs.close();
      std::cout << "CSV appended: " << csvPath << "\n";
    }
    else
    {
      std::cerr << "ERROR: cannot open CSV path: " << csvPath << "\n";
    }
  }

  if (enableAnim) { delete anim; anim = nullptr; }
  Simulator::Destroy();
  return 0;
}