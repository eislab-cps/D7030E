#!/usr/bin/env python3
# Lab 3 Python Starter: Hidden Terminal in Ad-hoc Network
# Usage: python3 Lab3_Py_Hidden.py --distance <m> --seed <run> --enableRtsCts <0|1>

import ns.core, ns.network, ns.wifi, ns.mobility, ns.internet, ns.applications, ns.flow_monitor, ns.netanim
from ns.core import CommandLine, Seconds, StringValue
from ns.network import NodeContainer, Ipv4AddressHelper
from ns.wifi import YansWifiChannelHelper, YansWifiPhyHelper, WifiHelper, WifiMacHelper, Ssid
from ns.mobility import MobilityHelper, ListPositionAllocator
from ns.internet import InternetStackHelper
from ns.applications import OnOffHelper, PacketSinkHelper
from ns.flow_monitor import FlowMonitorHelper
from ns.netanim import AnimationInterface
from ctypes import c_double, c_int, c_bool


def main():
    distance = c_double(200.0)
    seed = c_int(1)
    enableRtsCts = c_bool(False)
    cmd = CommandLine()
    cmd.AddValue('distance','Distance STA-AP (m)',distance)
    cmd.AddValue('seed','RngRun seed',seed)
    cmd.AddValue('enableRtsCts','Enable RTS/CTS (1) or disable (0)',enableRtsCts)
    cmd.Parse()

    ns.core.RngSeedManager.SetSeed(1)
    ns.core.RngSeedManager.SetRun(seed.value)
    ns.core.Time.SetResolution(ns.core.Time.NS)

    nodes = NodeContainer(); nodes.Create(3)
    # Configure RTS/CTS
    ns.core.Config.SetDefault('ns3::WifiRemoteStationManager::RtsCtsThreshold',
                               StringValue('0' if enableRtsCts.value else '2200'))

    channel = YansWifiChannelHelper()
    channel.SetPropagationDelay('ns3::ConstantSpeedPropagationDelayModel')
    channel.AddPropagationLoss('ns3::RangePropagationLossModel',
                               'MaxRange', ns.core.DoubleValue(distance.value * 1.5))
    phy = YansWifiPhyHelper(); phy.SetChannel(channel.Create())

    wifi = WifiHelper(); wifi.SetStandard(ns.wifi.WIFI_STANDARD_80211b)
    wifi.SetRemoteStationManager('ns3::ConstantRateWifiManager',
                                 'DataMode','DsssRate1Mbps','ControlMode','DsssRate1Mbps')
    mac = WifiMacHelper(); ssid=Ssid('hidden');
    mac.SetType('ns3::StaWifiMac','Ssid',ssid)
    staDev=wifi.Install(phy,mac,nodes.Get(0),nodes.Get(2))
    mac.SetType('ns3::ApWifiMac','Ssid',ssid)
    apDev=wifi.Install(phy,mac,nodes.Get(1))

    alloc=ListPositionAllocator()
    alloc.Add((0,0,0)); alloc.Add((distance.value,0,0)); alloc.Add((2*distance.value,0,0))
    mob=MobilityHelper(); mob.SetPositionAllocator(alloc); mob.SetMobilityModel('ns3::ConstantPositionMobilityModel'); mob.Install(nodes)

    InternetStackHelper().Install(nodes)
    addr=Ipv4AddressHelper(); addr.SetBase('10.1.5.0','255.255.255.0'); ifs=addr.Assign(staDev); addr.Assign(apDev)

    onoff1=OnOffHelper('ns3::UdpSocketFactory',ns.network.InetSocketAddress(ifs.GetAddress(0),9))
    onoff1.SetAttribute('DataRate',StringValue('1Mbps')); onoff1.SetAttribute('PacketSize',ns.core.UintegerValue(1000))
    app1=onoff1.Install(nodes.Get(0)); app1.Start(Seconds(1)); app1.Stop(Seconds(10))
    onoff2=OnOffHelper('ns3::UdpSocketFactory',ns.network.InetSocketAddress(ifs.GetAddress(2),10))
    onoff2.SetAttribute('DataRate',StringValue('1Mbps')); onoff2.SetAttribute('PacketSize',ns.core.UintegerValue(1000))
    app2=onoff2.Install(nodes.Get(2)); app2.Start(Seconds(1)); app2.Stop(Seconds(10))

    sink1=PacketSinkHelper('ns3::UdpSocketFactory',ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(),9))
    sink2=PacketSinkHelper('ns3::UdpSocketFactory',ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(),10))
    srv1=sink1.Install(nodes.Get(1)); srv1.Start(Seconds(0)); srv1.Stop(Seconds(10))
    srv2=sink2.Install(nodes.Get(1)); srv2.Start(Seconds(0)); srv2.Stop(Seconds(10))

    fm=FlowMonitorHelper(); monitor=fm.InstallAll(); AnimationInterface('Lab3_Hidden.xml')
    ns.core.Simulator.Stop(Seconds(11)); ns.core.Simulator.Run()
    monitor.CheckForLostPackets(); stats=monitor.GetFlowStats()
    print(f"Hidden flow1: {stats[1].rxBytes*8.0/9.0} bps")
    print(f"Hidden flow2: {stats[2].rxBytes*8.0/9.0} bps")
    ns.core.Simulator.Destroy()

if __name__=='__main__': main()
