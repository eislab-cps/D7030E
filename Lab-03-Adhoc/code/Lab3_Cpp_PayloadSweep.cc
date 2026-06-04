/*
 * Lab 03 — Payload & Hop-Count Sweep (UDP, IBSS)
 * -------------------------------------------------------------
 * This program automates the runs you need for Parts 1 & 2:
 *  - Varies NUMBER OF NODES (hop count) and UDP PAYLOAD size.
 *  - Uses 802.11b @ 1 Mb/s (ConstantRate), IBSS (ad-hoc).
 *  - Places nodes on a straight line, equally spaced (default 200 m).
 *  - Enables OLSR routing so multi-hop forwarding works.
 *  - Drives an OnOff UDP source from node 0 to the LAST node.
 *  - Computes application throughput over the ACTUAL TX WINDOW (1..10 s → 9 s).
 *  - Prints clean CSV so you can graph/aggregate easily.
 *
 * Why a separate file?
 *  - Keeps your "single-case" script small, while this one loops the grid.
 *  - Avoids copy/paste mess. This is the batch runner you’ll call for data.
 *
 *
 * Run (examples):
 *   # default sweep: nodes=3,4,5,6 ; pkt=300,700,1200 ; seeds=1,2 ; distance=200m
 *   --run "scratch/Lab3_Cpp_PayloadSweep"
 *
 *   # custom lists (comma-separated); write CSV to file:
 *   --run "scratch/Lab3_Cpp_PayloadSweep --nodes=3,6 --pkts=200,400,800,1200 --seeds=1,2,3 --csv=results.csv"
 *
 *   # change spacing or app rate; enable NetAnim/pcap for debugging:
 *   --run "scratch/Lab3_Cpp_PayloadSweep --distance=200 --appRate=1Mbps --enableAnim=0 --enablePcap=0"
 *
 * CSV columns:
 *   nodes,pktSize,seed,rxBytes,throughput_Mbps
 *
 * Notes:
 *   - If you see zero throughput: check that routing is enabled (OLSR here) and
 *     that spacing keeps only NEIGHBORS in range (Two-Ray + 200 m spacing).
 *   - We lock both DataMode and ControlMode to DsssRate1Mbps (no rate control).
 *   - Each (nodes, pktSize, seed) is its own fresh simulation (init→run→destroy).
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

#include <fstream>
#include <filesystem>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace ns3;

// ------------ small helpers for CLI parsing & banner ------------

static std::vector<uint32_t> ParseUintList(const std::string& csv)
{
  std::vector<uint32_t> out;
  std::stringstream ss(csv);
  std::string tok;
  while (std::getline(ss, tok, ','))
  {
    if (!tok.empty())
    {
      out.push_back(static_cast<uint32_t>(std::stoul(tok)));
    }
  }
  return out;
}

static void CsvPrintHeader(std::ostream& os)
{
  os << "nodes,pktSize,seed,rxBytes,throughput_Mbps\n";
}

static void Banner(const std::string& s)
{
  std::cout << "\n==== " << s << " ====\n";
}

// ------------ a single (nodes, pktSize, seed) simulation ------------

struct CaseResult
{
  uint32_t nodes;
  uint32_t pktSize;
  uint32_t seed;
  uint64_t rxBytes;
  double throughputMbps;
};

static CaseResult RunOneCase(uint32_t nodesCount,
                             uint32_t pktSize,
                             uint32_t seedRun,
                             double distance,
                             const std::string& appRate,
                             bool enablePcap,
                             bool enableAnim)
{
  // FIXED lab timing: send 1..10 s, stop at 11 s.
  const double appStart = 1.0;
  const double appStop  = 10.0;
  const double simStop  = 11.0;
  const double txWindow = appStop - appStart; // = 9.0

  // Deterministic seed + variable run (matches Lab convention).
  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(seedRun);

  // ---------------- nodes ----------------
  NodeContainer nodes;
  nodes.Create(nodesCount);

  // ---------------- wifi channel/phy ----------------
  YansWifiChannelHelper channel;
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss("ns3::TwoRayGroundPropagationLossModel");

  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);
  wifi.SetRemoteStationManager(
      "ns3::ConstantRateWifiManager",
      "DataMode",    StringValue("DsssRate1Mbps"),
      "ControlMode", StringValue("DsssRate1Mbps"));

  WifiMacHelper mac;
  mac.SetType("ns3::AdhocWifiMac"); // IBSS (no AP/STA roles)

  NetDeviceContainer devs = wifi.Install(phy, mac, nodes);

  if (enablePcap)
  {
    phy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcap("scratch/Lab3outputs/Lab3_PayloadSweep", devs, true /*promisc*/);
  }

  // ---------------- mobility (line, equally spaced) ----------------
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
  for (uint32_t i = 0; i < nodesCount; ++i)
  {
    pos->Add(Vector(distance * i, 0.0, 1.5));
  }
  mobility.SetPositionAllocator(pos);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  // ---------------- internet + routing (OLSR) ----------------
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
  Ipv4InterfaceContainer ifaces = ip.Assign(devs);

  // ---------------- apps ----------------
  const uint16_t port = 5000;
  Address sinkLocal(InetSocketAddress(Ipv4Address::GetAny(), port));
  PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", sinkLocal);
  ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(nodesCount - 1));
  sinkApp.Start(Seconds(0.0));
  sinkApp.Stop(Seconds(simStop));

  Address sinkRemote(InetSocketAddress(ifaces.GetAddress(nodesCount - 1), port));
  OnOffHelper onoff("ns3::UdpSocketFactory", sinkRemote);
  onoff.SetConstantRate(DataRate(appRate), pktSize);
  ApplicationContainer srcApp = onoff.Install(nodes.Get(0));
  srcApp.Start(Seconds(appStart));
  srcApp.Stop(Seconds(appStop));

  // ---------------- optional NetAnim ----------------
  AnimationInterface* anim = nullptr;
  if (enableAnim)
  {
    // Build a unique filename to avoid clobbering between runs.
    std::ostringstream fn;
    fn << "scratch/Lab3outputs/Lab3_PayloadSweep_n" << nodesCount
       << "_p" << pktSize
       << "_s" << seedRun
       << ".xml";
    anim = new AnimationInterface(fn.str());
    for (uint32_t i = 0; i < nodesCount; ++i)
    {
      anim->UpdateNodeDescription(nodes.Get(i), "n" + std::to_string(i));
      anim->UpdateNodeColor(nodes.Get(i), 200, 200, 200);
    }
  }

  // ---------------- run ----------------
  Simulator::Stop(Seconds(simStop));
  Simulator::Run();

  // ---------------- metrics (authoritative via sink) ----------------
  uint64_t rxBytes = 0;
  {
    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
    rxBytes = sink ? sink->GetTotalRx() : 0;
  }
  const double throughputMbps = (rxBytes * 8.0 / txWindow) / 1e6;

  if (anim) { delete anim; anim = nullptr; }
  Simulator::Destroy();

  return CaseResult{nodesCount, pktSize, seedRun, rxBytes, throughputMbps};
}

// ------------ main: parse CLI, loop grid, emit CSV ------------

int main(int argc, char* argv[])
{
  // Defaults match the lab spec.
  std::string nodesCsv = "3,4,5,6";    // chain sizes
  std::string pktsCsv  = "300,700,1200";
  std::string seedsCsv = "1,2";        // run two seeds and average offline
  double distance      = 200.0;        // meters
  std::string appRate  = "1Mbps";      // push to saturation
  bool enablePcap      = false;
  bool enableAnim      = false;
  std::string csvPath  = "";           // empty → print to stdout

  CommandLine cmd;
  cmd.AddValue("nodes",      "Comma-separated list of node counts (e.g., 3,4,5,6).", nodesCsv);
  cmd.AddValue("pkts",       "Comma-separated list of UDP payload sizes in bytes.",  pktsCsv);
  cmd.AddValue("seeds",      "Comma-separated list of RNG run numbers.",            seedsCsv);
  cmd.AddValue("distance",   "Inter-node spacing in meters.",                       distance);
  cmd.AddValue("appRate",    "OnOff application data rate (e.g., 1Mbps).",          appRate);
  cmd.AddValue("enablePcap", "Enable PCAP (promisc) dumps for debugging.",          enablePcap);
  cmd.AddValue("enableAnim", "Write NetAnim XML per run.",                           enableAnim);
  cmd.AddValue("csv",        "If non-empty, write CSV to this path; otherwise stdout.", csvPath);
  cmd.Parse(argc, argv);

  // Parse lists
  std::vector<uint32_t> nodesList = ParseUintList(nodesCsv);
  std::vector<uint32_t> pktsList  = ParseUintList(pktsCsv);
  std::vector<uint32_t> seedsList = ParseUintList(seedsCsv);

  std::filesystem::create_directories("scratch/Lab3outputs");
  if (nodesList.empty() || pktsList.empty() || seedsList.empty())
  {
    std::cerr << "ERROR: nodes/pkts/seeds lists must be non-empty.\n";
    return 1;
  }

  // Prepare CSV output stream
  std::ofstream ofs;
  std::ostream* out = &std::cout;
  if (!csvPath.empty())
  {
    ofs.open(csvPath, std::ios::out | std::ios::trunc);
    if (!ofs.is_open())
    {
      std::cerr << "ERROR: cannot open CSV file: " << csvPath << "\n";
      return 1;
    }
    out = &ofs;
  }

  CsvPrintHeader(*out);

  // Fixed order: for stable diffs/logs
  for (uint32_t n : nodesList)
  {
    if (n < 3)
    {
      std::cerr << "Skipping nodes=" << n << " (must be >= 3)\n";
      continue;
    }
    for (uint32_t p : pktsList)
    {
      for (uint32_t s : seedsList)
      {
        Banner("Run nodes=" + std::to_string(n) +
               " pkt=" + std::to_string(p) +
               " seed=" + std::to_string(s));

        CaseResult r = RunOneCase(n, p, s, distance, appRate, enablePcap, enableAnim);

        // CSV line
        (*out) << r.nodes << ","
               << r.pktSize << ","
               << r.seed << ","
               << r.rxBytes << ","
               << r.throughputMbps << "\n";
        out->flush();
      }
    }
  }

  if (ofs.is_open()) ofs.close();
  return 0;
}
