/*
 * Lab 02 — Smart-Building WiFi Roaming
 * ------------------------------------
 * Models a mobile Wi-Fi client (laptop/IoT sensor) moving between two APs.
 *
 * Topology:
 *   AP0 at (0,0,0) and AP1 at (50,0,0) — same SSID "SmartBuilding", ch1, 802.11b
 *   STA starts at (-20,0,0), moves at --speed m/s toward (70,0,0)
 *   Remote server connected via CSMA backbone bridged to both APs
 *
 * CLI flags:
 *   --speed        STA velocity m/s (default 5.0)
 *   --seed         RNG run (default 1)
 *   --logInterval  throughput sampling period s (default 1.0)
 *   --simDuration  total sim time s (default 25.0)
 *   --enableAnim   write roaming_anim.xml (default false)
 *
 * Output: roaming_throughput.csv (time_s,throughput_bps)
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/bridge-module.h"
#include "ns3/netanim-module.h"
#include <fstream>
#include <filesystem>

using namespace ns3;

static std::ofstream g_csv;
static uint64_t      g_lastRx      = 0;
static double        g_logInterval = 1.0;

static void
LogThroughput(Ptr<PacketSink> sink, double now)
{
  uint64_t rx  = sink->GetTotalRx();
  double   thr = (rx - g_lastRx) * 8.0 / g_logInterval;
  g_lastRx     = rx;
  g_csv << now << "," << static_cast<long long>(thr) << "\n";
  Simulator::Schedule(Seconds(g_logInterval), &LogThroughput, sink,
                      now + g_logInterval);
}

int
main(int argc, char* argv[])
{
  double      speed       = 5.0;
  uint32_t    seed        = 1;
  double      simDuration = 25.0;
  bool        enableAnim  = false;
  std::filesystem::create_directories("scratch/Lab2outputs");
  std::string csvPath     = "scratch/Lab2outputs/roaming_throughput.csv";

  CommandLine cmd;
  cmd.AddValue("speed",       "STA velocity in m/s.",            speed);
  cmd.AddValue("seed",        "RNG run number.",                 seed);
  cmd.AddValue("logInterval", "Throughput sampling period (s).", g_logInterval);
  cmd.AddValue("simDuration", "Total simulation time (s).",      simDuration);
  cmd.AddValue("enableAnim",  "Write roaming_anim.xml.",         enableAnim);
  cmd.Parse(argc, argv);

  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(seed);

  // --- Nodes ---
  NodeContainer apNodes; apNodes.Create(2);   // AP0, AP1
  NodeContainer staNode; staNode.Create(1);   // mobile STA
  NodeContainer server;  server.Create(1);    // remote server

  // --- WiFi (802.11b, SSID SmartBuilding) ---
  YansWifiChannelHelper wifiCh = YansWifiChannelHelper::Default();
  YansWifiPhyHelper     phy;
  phy.SetChannel(wifiCh.Create());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);
  wifi.SetRemoteStationManager("ns3::IdealWifiManager");

  // AP side
  Ssid         ssid = Ssid("SmartBuilding");
  WifiMacHelper macAp;
  macAp.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
  NetDeviceContainer apWifiDevs = wifi.Install(phy, macAp, apNodes);

  // STA side
  WifiMacHelper macSta;
  macSta.SetType("ns3::StaWifiMac",
                 "Ssid",            SsidValue(ssid),
                 "ActiveProbing",   BooleanValue(true));
  NetDeviceContainer staWifiDev = wifi.Install(phy, macSta, staNode);

  // --- CSMA backbone ---
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue("1Gbps"));
  csma.SetChannelAttribute("Delay",    TimeValue(MilliSeconds(1)));
  NodeContainer backboneNodes;
  backboneNodes.Add(apNodes);
  backboneNodes.Add(server);
  NetDeviceContainer backboneDevs = csma.Install(backboneNodes);

  // --- Bridge each AP: WiFi + CSMA ---
  BridgeHelper bridge;
  for (uint32_t i = 0; i < 2; ++i)
  {
    NetDeviceContainer bridged;
    bridged.Add(apWifiDevs.Get(i));
    bridged.Add(backboneDevs.Get(i));
    bridge.Install(apNodes.Get(i), bridged);
  }

  // --- Internet stack ---
  InternetStackHelper stack;
  stack.Install(staNode);
  stack.Install(server);

  // Assign IPs
  Ipv4AddressHelper addr;
  addr.SetBase("10.1.1.0", "255.255.255.0");
  addr.Assign(staWifiDev);
  addr.Assign(backboneDevs.Get(2)); // server's CSMA device

  // --- Mobility ---
  MobilityHelper apMob;
  Ptr<ListPositionAllocator> apPos = CreateObject<ListPositionAllocator>();
  apPos->Add(Vector(0.0,  0.0, 0.0));
  apPos->Add(Vector(50.0, 0.0, 0.0));
  apMob.SetPositionAllocator(apPos);
  apMob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  apMob.Install(apNodes);

  MobilityHelper staMob;
  Ptr<ListPositionAllocator> staPos = CreateObject<ListPositionAllocator>();
  staPos->Add(Vector(-20.0, 0.0, 0.0));
  staMob.SetPositionAllocator(staPos);
  staMob.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
  staMob.Install(staNode);
  staNode.Get(0)->GetObject<ConstantVelocityMobilityModel>()
         ->SetVelocity(Vector(speed, 0.0, 0.0));

  MobilityHelper serverMob;
  serverMob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  serverMob.Install(server);

  // --- Applications ---
  uint16_t port     = 9;
  double   appStart = 1.0;
  double   appStop  = simDuration - 1.0;

  // Get STA IP
  Ptr<Ipv4>      staIpv4 = staNode.Get(0)->GetObject<Ipv4>();
  Ipv4Address    staAddr = staIpv4->GetAddress(1, 0).GetLocal();

  OnOffHelper onoff("ns3::UdpSocketFactory",
                    InetSocketAddress(staAddr, port));
  onoff.SetAttribute("DataRate",   StringValue("100Mbps"));
  onoff.SetAttribute("PacketSize", UintegerValue(1024));
  onoff.SetAttribute("OnTime",     StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute("OffTime",    StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  ApplicationContainer src = onoff.Install(server.Get(0));
  src.Start(Seconds(appStart));
  src.Stop (Seconds(appStop));

  PacketSinkHelper sinkH("ns3::UdpSocketFactory",
                         InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer sinkApp = sinkH.Install(staNode.Get(0));
  sinkApp.Start(Seconds(0.0));
  sinkApp.Stop (Seconds(simDuration));

  // --- CSV output ---
  g_csv.open(csvPath);
  g_csv << "time_s,throughput_bps\n";

  // --- NetAnim ---
  AnimationInterface* anim = nullptr;
  if (enableAnim)
  {
    anim = new AnimationInterface("scratch/Lab2outputs/roaming_anim.xml");
  }

  // --- Schedule logging + run ---
  Simulator::Stop(Seconds(simDuration));
  Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
  Simulator::Schedule(Seconds(g_logInterval), &LogThroughput, sink,
                      g_logInterval);
  Simulator::Run();

  g_csv.close();
  if (anim) { delete anim; anim = nullptr; }
  Simulator::Destroy();
  return 0;
}
