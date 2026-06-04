// Lab 1: COST231-Hata (ns-3.40)
// Keep MAC/PHY at 802.11a; run COST231 @ 1.8 GHz per model validity.
// Usage: ./ns3 run "scratch/Lab1_Cpp_Cost231 --distance=60"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main (int argc, char* argv[])
{
  double distance = 60.0;
  CommandLine cmd; cmd.AddValue("distance","meters",distance); cmd.Parse(argc, argv);
  Time::SetResolution(Time::NS);

  NodeContainer nodes; nodes.Create(2);

  YansWifiChannelHelper channel;
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss("ns3::Cost231PropagationLossModel",
                             "Frequency",       DoubleValue(1.8e9),
                             "BSAntennaHeight", DoubleValue(15.0),
                             "SSAntennaHeight", DoubleValue(1.5),
                             "MinDistance",     DoubleValue(0.5));
  // NOTE: If your ns-3.40 build supports 'C' you can add: "C", DoubleValue(10.0)

  YansWifiPhyHelper phy; phy.SetChannel(channel.Create());
  phy.Set("TxPowerStart", DoubleValue(27.0));
  phy.Set("TxPowerEnd",   DoubleValue(27.0));
  phy.Set("RxSensitivity",  DoubleValue(-92.0));
  phy.Set("CcaEdThreshold", DoubleValue(-92.0));

  WifiHelper wifi; wifi.SetStandard(WIFI_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode", StringValue("OfdmRate6Mbps"),
                               "ControlMode", StringValue("OfdmRate6Mbps"));
  WifiMacHelper mac; mac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer devs = wifi.Install(phy, mac, nodes);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
  pos->Add(Vector(0.0, 0.0, 1.5));
  pos->Add(Vector(distance, 0.0, 1.5));
  mobility.SetPositionAllocator(pos);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  InternetStackHelper stack; stack.Install(nodes);
  Ipv4AddressHelper addr; addr.SetBase("10.1.3.0","255.255.255.0");
  Ipv4InterfaceContainer ifaces = addr.Assign(devs);

  OnOffHelper on("ns3::UdpSocketFactory", InetSocketAddress(ifaces.GetAddress(1), 9));
  on.SetAttribute("DataRate", StringValue("6Mbps"));
  on.SetAttribute("PacketSize", UintegerValue(1000));
  on.SetAttribute("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  on.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  ApplicationContainer tx = on.Install(nodes.Get(0));
  tx.Start(Seconds(1.0)); tx.Stop(Seconds(10.0));

  PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer rx = sink.Install(nodes.Get(1));
  rx.Start(Seconds(0.0)); rx.Stop(Seconds(10.0));

  FlowMonitorHelper fm; Ptr<FlowMonitor> m = fm.InstallAll();
  AnimationInterface anim("/work/Lab-01-Propagation/submission/Lab1_Cost231.xml");  // change file name per scenario
  anim.SetMobilityPollInterval(Seconds(0.5));   // how often positions are sampled

  // (Optional niceties)
  anim.UpdateNodeDescription(0, "Tx");
  anim.UpdateNodeDescription(1, "Rx");
  anim.UpdateNodeColor(0, 0, 128, 255);  // Tx = blue
  anim.UpdateNodeColor(1, 200, 0, 0);    // Rx = red

  // If you want per-packet metadata in the XML (larger files):
  anim.EnablePacketMetadata(true);

  // If your flows are heavy, keep XML size in check:
  //anim.SetMaxPktsPerTraceFile(50000);
  phy.EnablePcap("Lab1_Cost231", devs, true);

  Simulator::Stop(Seconds(10.0));
  Simulator::Run();
  m->CheckForLostPackets();

  uint64_t rxBytes = 0;
  for (const auto& kv : m->GetFlowStats()) rxBytes += kv.second.rxBytes;
  Simulator::Destroy();

  const double thr_bps = (rxBytes * 8.0) / 9.0;
  std::cout << "CSV,model=Cost231,distance_m=" << distance
            << ",rxBytes=" << rxBytes
            << ",throughput_bps=" << thr_bps << std::endl;
  return 0;
}