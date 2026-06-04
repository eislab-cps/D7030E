/* 
 * Lab 04 — LTE Downlink End-to-End (EPC) with Antennas and Traces
 * ----------------------------------------------------------------
 * What this program builds:
 *   UE <--LTE--> eNodeB <--S1-U--> PGW <----> Remote Server (UDP OnOff)
 *
 * Why this exists:
 *   - Exactly matches the lab spec for LTE/EPC in ns-3.40:
 *       * AMC model:    LteAmc::PiroEW2010
 *       * Scheduler:    PfFfMacScheduler
 *       * EARFCN:       DL=100 / UL=18100
 *       * Bandwidth:    DL/UL 50 RBs
 *       * Antennas:     parabolic / cosine / isotropic (selectable)
 *   - Records the required trace files (PDCP & RLC) and a PCAP on the server.
 *   - Computes downlink throughput at the UE’s PacketSink over the real send window.
 *   - Optional one-line CSV output for easy plotting.
 *
 * CLI examples:
 *   ./ns3 run "scratch/Lab4_Cpp_LTE --dataRate=10Mbps --distance=100 --antenna=isotropic"
 *   ./ns3 run "scratch/Lab4_Cpp_LTE --dataRate=5Mbps  --distance=50  --antenna=parabolic --enbOrient=0 --ueOrient=0"
 *   ./ns3 run "scratch/Lab4_Cpp_LTE --dataRate=20Mbps --distance=150 --antenna=cosine    --csv=/work/throughput.csv"
 *
 * What to submit (see lab docs): the program produces PDCP/RLC traces automatically
 * (names come from ns-3 LTE helper), and a PCAP on the server side named server_trace-*.pcap.
 *
 *   - Throughput formula: bytes_delivered * 8 / (appStop - appStart)   [bits per second]
 *   - We measure at the UE’s PacketSink (application-layer delivery).
 *   - For "throughput vs distance" experiments, pick ANTENNA = isotropic (per instructions).
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/lte-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"          // optional
#include "ns3/flow-monitor-module.h"     // optional (useful while debugging)
#include <filesystem>

using namespace ns3;

// ---------- Per-second throughput logger (mobile UE) ----------
static std::ofstream g_mobCsv;
static uint64_t      g_mobLastRx  = 0;

static void
LogMobileThroughput(Ptr<PacketSink> sink, double now)
{
  uint64_t rx  = sink->GetTotalRx();
  double   thr = (rx - g_mobLastRx) * 8.0;   // bits in the last 1 s
  g_mobLastRx  = rx;
  g_mobCsv << now << "," << static_cast<long long>(thr) << "\n";
  Simulator::Schedule(Seconds(1.0), &LogMobileThroughput, sink, now + 1.0);
}

// ---------- Small helpers ----------

// Map CLI antenna strings -> ns-3 typeId names expected by LteHelper.
// Valid choices: isotropic, cosine, parabolic (case-insensitive).
static std::string
ResolveAntennaTypeId(const std::string& user)
{
  std::string s = user;
  // lower-case
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
  if (s == "isotropic") return "ns3::IsotropicAntennaModel";
  if (s == "cosine")    return "ns3::CosineAntennaModel";
  if (s == "parabolic") return "ns3::ParabolicAntennaModel";
  // default
  return "ns3::IsotropicAntennaModel";
}

// Human-friendly banner
static void
Banner(const std::string& title)
{
  std::cout << "\n==== " << title << " ====\n";
}

int main(int argc, char* argv[])
{
  // ---------------- CLI (defaults are sensible for Lab 4) ----------------
  // Application data rate (UDP downlink). Accepts ns-3 DataRate strings: "5Mbps", "10Mbps", etc.
  std::string appRate   = "10Mbps";
  // UE distance from eNB (meters)
  double      distance  = 100.0;
  // Antenna model (applies to BOTH eNB and UE; use "isotropic" for distance sweeps)
  std::string antenna   = "isotropic";   // isotropic | cosine | parabolic
  // Optional: orientation of lobe (degrees). Useful to explore "misalignment" with directional antennas.
  double      enbOrient = 0.0;
  double      ueOrient  = 0.0;
  // RNG run number so you can average over multiple trials
  uint32_t    seedRun   = 1;
  // Optional CSV path: if non-empty, write one summary row
  std::string csvPath   = "";
  // Mobile UE: if true, UE moves at --speed m/s from x=distance toward eNB
  bool        mobility        = false;   // mobile UE flag
  double      ueSpeed         = 10.0;   // m/s (typical vehicle speed in a factory)

  bool enableAnim = false;   // NetAnim XML off by default

  CommandLine cmd;
  cmd.AddValue("dataRate",   "OnOff application data rate (e.g., 5Mbps, 10Mbps, 20Mbps).", appRate);
  cmd.AddValue("distance",   "UE distance from eNodeB in meters.",                          distance);
  cmd.AddValue("antenna",    "Antenna: isotropic | cosine | parabolic.",                    antenna);
  cmd.AddValue("enbOrient",  "eNB antenna orientation (degrees).",                          enbOrient);
  cmd.AddValue("ueOrient",   "UE antenna orientation (degrees).",                           ueOrient);
  cmd.AddValue("seed",       "RNG run number for repeatability.",                            seedRun);
  cmd.AddValue("csv",        "If non-empty, write a 1-line CSV summary to this path.",       csvPath);
  cmd.AddValue("mobility",   "Enable UE mobility (ConstantVelocityMobilityModel).",         mobility);
  cmd.AddValue("speed",      "UE speed in m/s when --mobility=true.",                       ueSpeed);
  cmd.AddValue("enableAnim", "Write NetAnim XML (Lab4_LTE.xml).",                            enableAnim);
  cmd.Parse(argc, argv);

  // ---------------- Output directory ----------------
  std::filesystem::create_directories("scratch/Lab4outputs");

  // ---------------- Determinism & time base ----------------
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(seedRun);
  Time::SetResolution(Time::NS);

  // ---------------- LTE/EPC helpers and REQUIRED attributes (per lab spec) ----------------
  // Create LTE + EPC (S1) stack
  Ptr<LteHelper> lte = CreateObject<LteHelper>();
  Ptr<PointToPointEpcHelper> epc = CreateObject<PointToPointEpcHelper>();
  lte->SetEpcHelper(epc);

  // AMC model (global config), per spec: LteAmc::PiroEW2010
  Config::SetDefault("ns3::LteAmc::AmcModel", EnumValue(LteAmc::PiroEW2010));

  // eNB/UE channel config (from Lab 4 instructions)
  //lte->SetAttribute("PathlossModel", StringValue("ns3::TwoRayGroundPropagationLossModel"));
  lte->SetSchedulerType("ns3::PfFfMacScheduler");
  //lte->SetAttribute("DlEarfcn", UintegerValue(100));
  //lte->SetAttribute("UlEarfcn", UintegerValue(18100));
  // 50 RBs downlink/uplink
  //lte->SetAttribute("DlBandwidth", UintegerValue(50));
  //lte->SetAttribute("UlBandwidth", UintegerValue(50));
  lte->SetEnbDeviceAttribute("DlEarfcn",    UintegerValue(100));
  lte->SetEnbDeviceAttribute("UlEarfcn",    UintegerValue(18100));
  lte->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(50));
  lte->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(50));


  // Antenna types (eNB + UE)
  const std::string antTypeId = ResolveAntennaTypeId(antenna);
  lte->SetEnbAntennaModelType(antTypeId);
  lte->SetUeAntennaModelType(antTypeId);
  // Orientation attributes (degrees). Ignored by isotropic.
  //lte->SetEnbAntennaModelAttribute("Orientation", DoubleValue(enbOrient));
  //lte->SetUeAntennaModelAttribute ("Orientation", DoubleValue(ueOrient));

  // Only directional models expose Orientation; Isotropic does not.
  if (antTypeId == "ns3::CosineAntennaModel" || antTypeId == "ns3::ParabolicAntennaModel") {
    lte->SetEnbAntennaModelAttribute("Orientation", DoubleValue(enbOrient));
    lte->SetUeAntennaModelAttribute ("Orientation", DoubleValue(ueOrient));
  } else {
    if (enbOrient != 0.0 || ueOrient != 0.0) {
    std::cout << "[note] Ignoring --enbOrient/--ueOrient for isotropic antenna.\n";
    }
  }

  // ---------------- Nodes: one eNB, one UE, one remote server (over the Internet side of PGW) ----------------
  NodeContainer enbNodes; enbNodes.Create(1);
  NodeContainer ueNodes;  ueNodes.Create(1);
  Ptr<Node> pgw = epc->GetPgwNode();

  // Remote server host (on the "Internet" side of PGW)
  NodeContainer remoteHostCont; remoteHostCont.Create(1);
  InternetStackHelper internet;
  internet.Install(remoteHostCont);   // server side
  internet.Install(ueNodes);          // UE IP stack (EPC provides addressing)

  // ---------------- Topology & mobility ----------------
  // eNB is pinned at (0,0,0). UE starts at (distance,0,0).
  MobilityHelper mobHelper;
  mobHelper.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  // eNB position
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
  pos->Add(Vector(0.0, 0.0, 0.0)); // eNB
  mobHelper.SetPositionAllocator(pos);
  mobHelper.Install(enbNodes);

  // UE position/motion
  Ptr<ListPositionAllocator> posUe = CreateObject<ListPositionAllocator>();
  posUe->Add(Vector(distance, 0.0, 0.0)); // UE start
  if (mobility)
  {
    MobilityHelper ueMob;
    ueMob.SetPositionAllocator(posUe);
    ueMob.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    ueMob.Install(ueNodes);
    // Move toward eNB (negative x direction)
    ueNodes.Get(0)->GetObject<ConstantVelocityMobilityModel>()
           ->SetVelocity(Vector(-ueSpeed, 0.0, 0.0));
    std::cout << "Mobile UE: speed=" << ueSpeed << " m/s (toward eNB)\n";
  }
  else
  {
    mobHelper.SetPositionAllocator(posUe);
    mobHelper.Install(ueNodes);
  }

  // ---------------- Install LTE devices (air interface) ----------------
  NetDeviceContainer enbDevs = lte->InstallEnbDevice(enbNodes);
  NetDeviceContainer ueDevs  = lte->InstallUeDevice(ueNodes);

  // Assign UE IP via EPC and attach UE to the eNB
  Ipv4InterfaceContainer ueIfaces = epc->AssignUeIpv4Address(ueDevs);
  lte->Attach(ueDevs.Get(0), enbDevs.Get(0));

  // ---------------- Core network: PGW <-> remote server (point-to-point) ----------------
  // High-capacity core link so it never bottlenecks the LTE link.
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue("100Gbps"));
  p2p.SetChannelAttribute("Delay",    StringValue("5ms"));
  NetDeviceContainer internetDevs = p2p.Install(pgw, remoteHostCont.Get(0));

  // Assign IPs on the PGW<->server link
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIfs = ipv4h.Assign(internetDevs);

  // Static route on the remote server → UE subnet (EPC uses 7.0.0.0/8 by default)
  Ipv4StaticRoutingHelper srt;
  Ptr<Ipv4StaticRouting> rh = srt.GetStaticRouting(remoteHostCont.Get(0)->GetObject<Ipv4>());
  // Route 7.0.0.0/8 via the PGW address on this link (internetIfs.GetAddress(0))
  rh->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"),
                        internetIfs.GetAddress(0), /* interface index */ 1);

  // Optional: ensure UE has a default route via EPC (usually handled by EPC helper)
  // (kept for clarity; harmless if already set)
  Ptr<Ipv4StaticRouting> ueRt = srt.GetStaticRouting(ueNodes.Get(0)->GetObject<Ipv4>());
  ueRt->SetDefaultRoute(epc->GetUeDefaultGatewayAddress(), 1);

  // ---------------- Applications: UDP OnOff (server -> UE) + PacketSink on UE ----------------
  const uint16_t port = 8000;
  const double appStart = 2.0;     // start after EPC is up
  // If mobile, extend sim so UE has time to travel
  const double simStop  = mobility ? (distance + 500.0) / ueSpeed + 5.0 : 22.0;
  const double appStop  = simStop - 2.0;

  // Downlink UDP generator on the remote server
  OnOffHelper onoff("ns3::UdpSocketFactory",
                    InetSocketAddress(ueIfaces.GetAddress(0), port));
  onoff.SetAttribute("DataRate",   StringValue(appRate));     // e.g., "10Mbps"
  onoff.SetAttribute("PacketSize", UintegerValue(1024));      // 1 KB payloads
  onoff.SetAttribute("OnTime",     StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute("OffTime",    StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  ApplicationContainer client = onoff.Install(remoteHostCont.Get(0));
  client.Start(Seconds(appStart));
  client.Stop (Seconds(appStop));

  // UE sink to count bytes actually delivered at the application
  PacketSinkHelper sinkH("ns3::UdpSocketFactory",
                         InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer sinkApp = sinkH.Install(ueNodes.Get(0));
  sinkApp.Start(Seconds(0.5));
  sinkApp.Stop (Seconds(simStop));

  // ---------------- Tracing (REQUIRED by the lab) ----------------
  // LTE traces: PDCP + RLC — redirect to scratch/Lab4outputs/
  Config::SetDefault("ns3::RadioBearerStatsCalculator::DlPdcpOutputFilename",
                     StringValue("scratch/Lab4outputs/DlPdcpStats.txt"));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::UlPdcpOutputFilename",
                     StringValue("scratch/Lab4outputs/UlPdcpStats.txt"));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::DlRlcOutputFilename",
                     StringValue("scratch/Lab4outputs/DlRlcStats.txt"));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::UlRlcOutputFilename",
                     StringValue("scratch/Lab4outputs/UlRlcStats.txt"));
  lte->EnablePdcpTraces();
  lte->EnableRlcTraces();

  // PCAP evidence on the server side of the PGW link (deliverable: server_trace.pcap)
  // Enable only on the REMOTE HOST device (index 1 in 'internetDevs')
  p2p.EnablePcap("scratch/Lab4outputs/server_trace", internetDevs.Get(1), true /*promiscuous*/);

  // Optional: NetAnim for visualization
  AnimationInterface* anim = nullptr;
  if (enableAnim)
  {
    anim = new AnimationInterface("scratch/Lab4outputs/Lab4_LTE.xml");
    anim->UpdateNodeDescription(enbNodes.Get(0), "eNB");
    anim->UpdateNodeDescription(ueNodes.Get(0),  "UE");
    anim->UpdateNodeDescription(pgw,             "PGW");
    anim->UpdateNodeDescription(remoteHostCont.Get(0), "Server");
  }

  // Optional: FlowMonitor (informational; not required for submission)
  FlowMonitorHelper fmH;
  Ptr<FlowMonitor> fm = fmH.InstallAll();

  // Per-second CSV logging for mobile UE scenario
  if (mobility)
  {
    std::string mobCsvPath = csvPath.empty() ? "scratch/Lab4outputs/ue_mobile_throughput.csv" : csvPath + ".mobile.csv";
    g_mobCsv.open(mobCsvPath);
    g_mobCsv << "time_s,throughput_bps\n";
    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
    Simulator::Schedule(Seconds(appStart + 1.0), &LogMobileThroughput, sink, appStart + 1.0);
  }

  // ---------------- Run ----------------
  Simulator::Stop(Seconds(simStop));
  Simulator::Run();

  // ---------------- Throughput (authoritative app-level) ----------------
  // Bytes successfully received by the UE’s PacketSink during [appStart, appStop].
  uint64_t rxBytes = 0;
  if (sinkApp.GetN() > 0)
  {
    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
    rxBytes = sink ? sink->GetTotalRx() : 0;
  }
  const double txWindow = appStop - appStart;                 // seconds
  const double thr_bps  = (rxBytes * 8.0) / txWindow;         // bits/s
  const double thr_Mbps = thr_bps / 1e6;

  Banner("LTE DL throughput (UE PacketSink, app-level)");
  std::cout << "dataRate=" << appRate
            << "  distance=" << distance << " m"
            << "  antenna="  << antenna
            << "  seed="     << seedRun
            << "\n";
  std::cout << "rxBytes=" << rxBytes
            << " over "  << txWindow << " s"
            << "  -> throughput=" << thr_Mbps << " Mb/s\n";

  // ---------------- Optional CSV (one line) ----------------
  if (!csvPath.empty())
  {
    std::ofstream ofs(csvPath, std::ios::out | std::ios::app);
    if (ofs.is_open())
    {
      // Header (write only if file is new/empty would be nicer; keep simple for lab)
      // Format matches deliverables guidance: adjust as you like.
      ofs << "data_rate,distance_m,antenna,seed,rxBytes,throughput_bps\n";
      ofs << appRate << "," << distance << "," << antenna << "," << seedRun
          << "," << rxBytes << "," << thr_bps << "\n";
      ofs.close();
      std::cout << "CSV appended: " << csvPath << "\n";
    }
    else
    {
      std::cerr << "ERROR: cannot open CSV path: " << csvPath << "\n";
    }
  }

  // ---------------- Cleanup ----------------
  if (g_mobCsv.is_open()) { g_mobCsv.close(); }
  if (anim) { delete anim; anim = nullptr; }
  Simulator::Destroy();
  return 0;
}
