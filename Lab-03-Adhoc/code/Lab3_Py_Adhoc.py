#!/usr/bin/env python3
# Lab 3 Python Starter: Ad-hoc Multi-hop UDP Chain Performance
# Usage: python3 Lab3_Py_Adhoc.py --numNodes <N> --pktSize <bytes> --distance <m> --seed <run>

import ns.core
import ns.network
import ns.wifi
import ns.mobility
import ns.internet
import ns.applications
import ns.flow_monitor
import ns.netanim

from ns.core import CommandLine, Seconds, UintegerValue, DoubleValue, StringValue
from ns.network import NodeContainer, Ipv4AddressHelper
from ns.wifi import YansWifiChannelHelper, YansWifiPhyHelper, WifiHelper, WifiMacHelper
from ns.mobility import MobilityHelper, ListPositionAllocator
from ns.internet import InternetStackHelper, Ipv4ListRoutingHelper, Ipv4StaticRoutingHelper
from ns.internet import OlsrHelper, AodvHelper
from ns.applications import OnOffHelper, PacketSinkHelper
from ns.flow_monitor import FlowMonitorHelper
from ns.netanim import AnimationInterface
from ctypes import c_double, c_int, c_bool
import os


def main():
    os.makedirs("scratch/Lab3outputs", exist_ok=True)
    # Parse parameters
    numNodes = c_int(3)
    pktSize = c_int(1000)
    distance = c_double(200.0)
    seed = c_int(1)
    routing = 'olsr'
    cmd = CommandLine()
    cmd.AddValue('numNodes', 'Number of nodes in chain', numNodes)
    cmd.AddValue('pktSize', 'UDP packet size in bytes', pktSize)
    cmd.AddValue('distance', 'Distance between adjacent nodes (m)', distance)
    cmd.AddValue('seed', 'RngRun seed value', seed)
    cmd.AddValue('routing', 'Routing protocol: olsr or aodv', routing)
    cmd.Parse()

    # RNG setup
    ns.core.RngSeedManager.SetSeed(1)
    ns.core.RngSeedManager.SetRun(seed.value)

    ns.core.Time.SetResolution(ns.core.Time.NS)

    # Create nodes
    nodes = NodeContainer()
    nodes.Create(numNodes.value)

    # Wi-Fi ad-hoc setup
    channelHelper = YansWifiChannelHelper()
    channelHelper.SetPropagationDelay('ns3::ConstantSpeedPropagationDelayModel')
    channelHelper.AddPropagationLoss('ns3::RangePropagationLossModel',
                                     'MaxRange', DoubleValue(distance.value * 1.5))

    phy = YansWifiPhyHelper()
    phy.SetChannel(channelHelper.Create())

    wifi = WifiHelper()
    wifi.SetStandard(ns.wifi.WIFI_STANDARD_80211b)
    wifi.SetRemoteStationManager('ns3::ConstantRateWifiManager',
                                 'DataMode', 'DsssRate1Mbps',
                                 'ControlMode', 'DsssRate1Mbps')

    mac = WifiMacHelper()
    mac.SetType('ns3::AdhocWifiMac')
    devices = wifi.Install(phy, mac, nodes)

    # Mobility: place nodes in straight line
    posAlloc = ListPositionAllocator()
    for i in range(numNodes.value):
        posAlloc.Add((distance.value * i, 0.0, 0.0))
    mobility = MobilityHelper()
    mobility.SetPositionAllocator(posAlloc)
    mobility.SetMobilityModel('ns3::ConstantPositionMobilityModel')
    mobility.Install(nodes)

    # Internet stack with routing
    listRouting = Ipv4ListRoutingHelper()
    staticRh = Ipv4StaticRoutingHelper()
    if routing == 'aodv':
        aodv = AodvHelper()
        listRouting.Add(aodv, 10)
    else:
        olsr = OlsrHelper()
        listRouting.Add(olsr, 10)
    listRouting.Add(staticRh, 5)
    stack = InternetStackHelper()
    stack.SetRoutingHelper(listRouting)
    stack.Install(nodes)
    print(f"Routing protocol: {routing}")

    # IP addressing
    address = Ipv4AddressHelper()
    address.SetBase('10.1.4.0', '255.255.255.0')
    interfaces = address.Assign(devices)

    # OnOff UDP: first -> last
    onoff = OnOffHelper('ns3::UdpSocketFactory',
                        ns.network.InetSocketAddress(interfaces.GetAddress(numNodes.value-1), 9))
    onoff.SetAttribute('DataRate', ns.core.StringValue('1Mbps'))
    onoff.SetAttribute('PacketSize', UintegerValue(pktSize.value))
    clientApps = onoff.Install(nodes.Get(0))
    clientApps.Start(Seconds(1.0))
    clientApps.Stop(Seconds(10.0))

    # Sink on last node
    sink = PacketSinkHelper('ns3::UdpSocketFactory',
                            ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), 9))
    serverApps = sink.Install(nodes.Get(numNodes.value-1))
    serverApps.Start(Seconds(0.0))
    serverApps.Stop(Seconds(10.0))

    # FlowMonitor & NetAnim
    fmHelper = FlowMonitorHelper()
    monitor = fmHelper.InstallAll()
    anim = AnimationInterface('scratch/Lab3outputs/Lab3_Adhoc.xml')

    # Run simulation
    ns.core.Simulator.Stop(Seconds(11.0))
    ns.core.Simulator.Run()

    # Throughput calculation
    monitor.CheckForLostPackets()
    stats = monitor.GetFlowStats()
    throughput = stats[1].rxBytes * 8.0 / 9.0
    print(f"Ad-hoc UDP throughput: {throughput} bps")

    ns.core.Simulator.Destroy()

if __name__ == '__main__':
    main()
