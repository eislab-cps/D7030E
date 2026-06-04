#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
using namespace ns3;

int main(int argc, char** argv) {
  NodeContainer nodes; nodes.Create(2);

  WifiHelper wifi; wifi.SetStandard(WIFI_STANDARD_80211b);
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy; phy.SetChannel(channel.Create());
  WifiMacHelper mac; mac.SetType("ns3::AdhocWifiMac");

  NetDeviceContainer devs = wifi.Install(phy, mac, nodes);

  MobilityHelper mob;
  mob.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mob.Install(nodes);
  nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector(0,0,0));
  nodes.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector(5,0,0));

  InternetStackHelper stack; stack.Install(nodes);
  Ipv4AddressHelper ip; ip.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifs = ip.Assign(devs);

  // simple echo
  UdpEchoServerHelper srv(9);
  ApplicationContainer a1 = srv.Install(nodes.Get(1));
  a1.Start(Seconds(1.0)); a1.Stop(Seconds(10.0));

  UdpEchoClientHelper cli(ifs.GetAddress(1), 9);
  cli.SetAttribute("MaxPackets", UintegerValue(1));
  cli.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  cli.SetAttribute("PacketSize", UintegerValue(1024));
  ApplicationContainer a2 = cli.Install(nodes.Get(0));
  a2.Start(Seconds(2.0)); a2.Stop(Seconds(10.0));

  AnimationInterface anim("lab0_cpp_anim.xml"); // <-- NetAnim output
  anim.SetConstantPosition(nodes.Get(0), 0, 0);
  anim.SetConstantPosition(nodes.Get(1), 5, 0);

  Simulator::Stop(Seconds(11.0));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
