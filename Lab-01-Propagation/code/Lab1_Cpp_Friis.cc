// Lab 1: Friis (ns-3.40, IBSS 802.11a @ 6 Mbps on 5 GHz)
// Usage: ./ns3 run "scratch/Lab1_Cpp_Friis --distance=50"
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
  double distance = 50.0;
  CommandLine cmd; cmd.AddValue("distance","meters",distance); cmd.Parse(argc, argv);
  Time::SetResolution(Time::NS);

  NodeContainer nodes; nodes.Create(2);

  // Channel: start empty (no implicit loss), then add Friis @ 5.18 GHz (802.11a ch36-ish).
  YansWifiChannelHelper channel;
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                             "Frequency", DoubleValue(5.18e9));

  YansWifiPhyHelper phy; phy.SetChannel(channel.Create());
  // Reasonable link budget for 5 GHz
  phy.Set("TxPowerStart", DoubleValue(23.0));
  phy.Set("TxPowerEnd",   DoubleValue(23.0));
  // Keep thresholds realistic for 6 Mbps OFDM
  phy.Set("RxSensitivity",  DoubleValue(-92.0));
  phy.Set("CcaEdThreshold", DoubleValue(-92.0));

  WifiHelper wifi; wifi.SetStandard(WIFI_STANDARD_80211a);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode", StringValue("OfdmRate6Mbps"),
                               "ControlMode", StringValue("OfdmRate6Mbps"));

  WifiMacHelper mac; mac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer devs = wifi.Install(phy, mac, nodes);

  // Positions: IBSS peers at z=1.5 m
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
  pos->Add(Vector(0.0, 0.0, 1.5));
  pos->Add(Vector(distance, 0.0, 1.5));
  mobility.SetPositionAllocator(pos);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);

  // IP + UDP app (1000B payload)
  InternetStackHelper stack; stack.Install(nodes);
  Ipv4AddressHelper addr; addr.SetBase("10.1.1.0","255.255.255.0");
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

  // FlowMonitor + NetAnim + pcap for Wireshark
  FlowMonitorHelper fm; Ptr<FlowMonitor> m = fm.InstallAll();
  AnimationInterface anim("Lab1_Friis.xml");
  phy.EnablePcap("Lab1_Friis", devs, true);

  Simulator::Stop(Seconds(10.0));
  Simulator::Run();
  m->CheckForLostPackets();

  uint64_t rxBytes = 0;
  for (const auto& kv : m->GetFlowStats()) rxBytes += kv.second.rxBytes;
  Simulator::Destroy();

  const double thr_bps = (rxBytes * 8.0) / 9.0;
  std::cout << "CSV,model=Friis,distance_m=" << distance
            << ",rxBytes=" << rxBytes
            << ",throughput_bps=" << thr_bps << std::endl;
  return 0;
}