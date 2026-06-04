#!/usr/bin/env python3
# Lab 1 Python Starter: Friis Propagation Loss Model

import ns.core
import ns.network
import ns.wifi
import ns.mobility
import ns.internet
import ns.applications
import ns.flow_monitor
import ns.netanim

from ns.core import CommandLine, Seconds
from ctypes import c_double, c_int, c_bool


def main():
    # parse distance parameter
    distance = c_double(50.0)
    cmd = CommandLine()
    cmd.AddValue('distance', 'Distance between nodes (m)', distance)
    cmd.Parse()

    # set time resolution
    ns.core.Time.SetResolution(ns.core.Time.NS)

    # create two nodes
    nodes = ns.network.NodeContainer()
    nodes.Create(2)

    # set up channel with Friis model
    channelHelper = ns.wifi.YansWifiChannelHelper()
    channelHelper.SetPropagationDelay('ns3::ConstantSpeedPropagationDelayModel')
    channelHelper.AddPropagationLoss('ns3::FriisPropagationLossModel')

    phy = ns.wifi.YansWifiPhyHelper()
    phy.SetChannel(channelHelper.Create())

    wifi = ns.wifi.WifiHelper()
    wifi.SetStandard(ns.wifi.WIFI_STANDARD_80211a)
    wifi.SetRemoteStationManager('ns3::ConstantRateWifiManager',
                                 'DataMode', 'OfdmRate6Mbps',
                                 'ControlMode', 'OfdmRate6Mbps')

    mac = ns.wifi.WifiMacHelper()
    mac.SetType('ns3::AdhocWifiMac')
    devices = wifi.Install(phy, mac, nodes)

    # position nodes
    posAlloc = ns.mobility.ListPositionAllocator()
    posAlloc.Add((0.0, 0.0, 0.0))
    posAlloc.Add((distance.value, 0.0, 0.0))
    mobility = ns.mobility.MobilityHelper()
    mobility.SetPositionAllocator(posAlloc)
    mobility.SetMobilityModel('ns3::ConstantPositionMobilityModel')
    mobility.Install(nodes)

    # install internet stack
    stack = ns.internet.InternetStackHelper()
    stack.Install(nodes)

    # assign IP addresses
    address = ns.internet.Ipv4AddressHelper()
    address.SetBase('10.1.1.0', '255.255.255.0')
    interfaces = address.Assign(devices)

    # configure OnOff application (UDP)
    onoff = ns.applications.OnOffHelper('ns3::UdpSocketFactory',
                                        (interfaces.GetAddress(1), 9))
    onoff.SetAttribute('DataRate', ns.core.StringValue('1Mbps'))
    onoff.SetAttribute('PacketSize', ns.core.UintegerValue(1000))
    app = onoff.Install(nodes.Get(0))
    app.Start(Seconds(1.0))
    app.Stop(Seconds(10.0))

    # configure packet sink on node 1
    sink = ns.applications.PacketSinkHelper('ns3::UdpSocketFactory',
                                            ns.network.InetSocketAddress(
                                                ns.network.Ipv4Address.GetAny(), 9))
    app = sink.Install(nodes.Get(1))
    app.Start(Seconds(0.0))
    app.Stop(Seconds(10.0))

    # install flow monitor and NetAnim
    flowmonHelper = ns.flow_monitor.FlowMonitorHelper()
    monitor = flowmonHelper.InstallAll()
    anim = ns.netanim.AnimationInterface('Lab1_Friis.xml')

    # run simulation
    ns.core.Simulator.Stop(Seconds(10.0))
    ns.core.Simulator.Run()

    # retrieve and print throughput
    stats = monitor.GetFlowStats()[1]
    throughput = stats.rxBytes * 8.0 / 9.0
    print(f"Friis Python throughput: {throughput} bps")

    ns.core.Simulator.Destroy()

if __name__ == '__main__':
    main()
