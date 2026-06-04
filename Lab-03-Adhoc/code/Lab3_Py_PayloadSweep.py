#!/usr/bin/env python3
# Lab 3 Python Starter: Payload Sweep for Ad-hoc Chain
# Usage: python3 Lab3_Py_PayloadSweep.py --numNodes <N> --distance <m> --seed <run>
# Packet sizes: [300, 700, 1200]

import ns.core
import ns.network
import ns.wifi
import ns.mobility
import ns.internet
import ns.applications
import ns.flow_monitor

from ns.core import CommandLine, Seconds, UintegerValue, StringValue
from ns.network import NodeContainer, Ipv4AddressHelper
from ns.wifi import YansWifiChannelHelper, YansWifiPhyHelper, WifiHelper, WifiMacHelper
from ns.mobility import MobilityHelper, ListPositionAllocator
from ns.internet import InternetStackHelper
from ns.applications import OnOffHelper, PacketSinkHelper
from ns.flow_monitor import FlowMonitorHelper


def main():
    numNodes = 3
    distance = 200.0
    seed = 1
    cmd = CommandLine()
    cmd.AddValue('numNodes', 'Number of nodes', numNodes)
    cmd.AddValue('distance', 'Distance between adjacent nodes', distance)
    cmd.AddValue('seed', 'RngRun seed value', seed)
    cmd.Parse()

    ns.core.RngSeedManager.SetSeed(1)
    ns.core.RngSeedManager.SetRun(seed)

    ns.core.Time.SetResolution(ns.core.Time.NS)

    pktSizes = [300, 700, 1200]
    for pktSize in pktSizes:
        nodes = NodeContainer(); nodes.Create(numNodes)

        channel = YansWifiChannelHelper()
        channel.SetPropagationDelay('ns3::ConstantSpeedPropagationDelayModel')
        channel.AddPropagationLoss('ns3::TwoRayGroundPropagationLossModel')

        phy = YansWifiPhyHelper()
        phy.SetChannel(channel.Create())

        wifi = WifiHelper()
        wifi.SetStandard(ns.wifi.WIFI_STANDARD_80211b)
        wifi.SetRemoteStationManager('ns3::ConstantRateWifiManager',
                                     'DataMode', StringValue('DsssRate1Mbps'),
                                     'ControlMode', StringValue('DsssRate1Mbps'))

        mac = WifiMacHelper()
        mac.SetType('ns3::AdhocWifiMac')
        devices = wifi.Install(phy, mac, nodes)

        posAlloc = ListPositionAllocator()
        for i in range(numNodes):
            posAlloc.Add((distance * i, 0.0, 0.0))
        mobility = MobilityHelper()
        mobility.SetPositionAllocator(posAlloc)
        mobility.SetMobilityModel('ns3::ConstantPositionMobilityModel')
        mobility.Install(nodes)

        stack = InternetStackHelper()
        stack.Install(nodes)

        address = Ipv4AddressHelper()
        address.SetBase('10.1.4.0', '255.255.255.0')
        interfaces = address.Assign(devices)

        onoff = OnOffHelper('ns3::UdpSocketFactory',
                            ns.network.InetSocketAddress(interfaces.GetAddress(numNodes-1), 9))
        onoff.SetAttribute('DataRate', StringValue('1Mbps'))
        onoff.SetAttribute('PacketSize', UintegerValue(pktSize))
        client = onoff.Install(nodes.Get(0))
        client.Start(Seconds(1.0)); client.Stop(Seconds(10.0))

        sink = ns.applications.PacketSinkHelper('ns3::UdpSocketFactory',
                                                ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), 9))
        server = sink.Install(nodes.Get(numNodes-1))
        server.Start(Seconds(0.0)); server.Stop(Seconds(10.0))

        fmHelper = FlowMonitorHelper()
        monitor = fmHelper.InstallAll()

        Simulator = ns.core.Simulator
        Simulator.Stop(Seconds(11.0))
        Simulator.Run()

        monitor.CheckForLostPackets()
        stats = monitor.GetFlowStats()
        throughput = stats[1].rxBytes * 8.0 / 9.0
        print(f"pktSize={pktSize}, throughput={throughput} bps")

        ns.core.Simulator.Destroy()

if __name__ == '__main__':
    main()