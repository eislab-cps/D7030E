#!/usr/bin/env python3
# Lab 3 Python Starter: TCP vs UDP in Ad-hoc Chain
# Usage: python3 Lab3_Py_TCP.py --pktSize <bytes> --seed <run>

import ns.core, ns.network, ns.wifi, ns.mobility, ns.internet, ns.applications, ns.flow_monitor
from ns.core import CommandLine, Seconds, UintegerValue, StringValue
from ns.network import NodeContainer, Ipv4AddressHelper
from ns.wifi import YansWifiChannelHelper, YansWifiPhyHelper, WifiHelper, WifiMacHelper
from ns.mobility import MobilityHelper, ListPositionAllocator
from ns.internet import InternetStackHelper
from ns.applications import OnOffHelper, PacketSinkHelper
from ns.flow_monitor import FlowMonitorHelper
from ctypes import c_double, c_int, c_bool


def main():
    pktSize = c_int(300)
    seed = c_int(1)
    cmd = CommandLine()
    cmd.AddValue('pktSize', 'Segment/packet size', pktSize)
    cmd.AddValue('seed', 'RngRun seed', seed)
    cmd.Parse()

    ns.core.RngSeedManager.SetSeed(1)
    ns.core.RngSeedManager.SetRun(seed.value)
    ns.core.Time.SetResolution(ns.core.Time.NS)

    nodes = NodeContainer(); nodes.Create(3)
    channel = YansWifiChannelHelper(); channel.SetPropagationDelay('ns3::ConstantSpeedPropagationDelayModel'); channel.AddPropagationLoss('ns3::TwoRayGroundPropagationLossModel')
    phy = YansWifiPhyHelper(); phy.SetChannel(channel.Create())
    wifi = WifiHelper(); wifi.SetStandard(ns.wifi.WIFI_STANDARD_80211b); wifi.SetRemoteStationManager('ns3::ConstantRateWifiManager','DataMode','DsssRate1Mbps','ControlMode','DsssRate1Mbps')
    mac = WifiMacHelper(); mac.SetType('ns3::AdhocWifiMac')
    devices = wifi.Install(phy,mac,nodes)

    alloc = ListPositionAllocator(); alloc.Add((0,0,1.5)); alloc.Add((200,0,1.5)); alloc.Add((400,0,1.5))
    mob = MobilityHelper(); mob.SetPositionAllocator(alloc); mob.SetMobilityModel('ns3::ConstantPositionMobilityModel'); mob.Install(nodes)

    InternetStackHelper().Install(nodes)
    addr = Ipv4AddressHelper(); addr.SetBase('10.1.6.0','255.255.255.0'); ifs = addr.Assign(devices)

    # TCP server on node2
    sink = PacketSinkHelper('ns3::TcpSocketFactory', ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(),9))
    srv = sink.Install(nodes.Get(2)); srv.Start(Seconds(0)); srv.Stop(Seconds(10))

    # TCP client on node0
    ns.core.Config.SetDefault('ns3::TcpSocket::SegmentSize', UintegerValue(pktSize.value))
    client = OnOffHelper('ns3::TcpSocketFactory', ns.network.InetSocketAddress(ifs.GetAddress(2),9))
    client.SetAttribute('DataRate', StringValue('5Mbps'))
    client.SetAttribute('PacketSize', UintegerValue(pktSize.value))
    apps = client.Install(nodes.Get(0)); apps.Start(Seconds(1)); apps.Stop(Seconds(10))

    fm = FlowMonitorHelper(); monitor = fm.InstallAll()
    ns.core.Simulator.Stop(Seconds(11)); ns.core.Simulator.Run()
    monitor.CheckForLostPackets(); stats = monitor.GetFlowStats()
    throughput = stats[1].rxBytes * 8.0 / 10.0
    print(f"TCP throughput: {throughput} bps")
    ns.core.Simulator.Destroy()

if __name__=='__main__': main()
