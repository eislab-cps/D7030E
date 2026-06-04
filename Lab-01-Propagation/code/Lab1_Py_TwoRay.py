#!/usr/bin/env python3
# Lab 1 Python Starter: Two-Ray Ground Propagation Loss

import ns.core
import ns.network
import ns.wifi
import ns.mobility
import ns.internet
import ns.applications
import ns.flow_monitor
import ns.netanim

from ns.core import CommandLine, Seconds


def main():
    # Parse distance
    distance = 50.0
    cmd = CommandLine()
    cmd.AddValue('distance', 'Distance (m)', distance)
    cmd.Parse()

    ns.core.Time.SetResolution(ns.core.Time.NS)

    # Nodes
    nodes = ns.network.NodeContainer()
    nodes.Create(2)

    # Channel + Two-Ray
    channel = ns.wifi.YansWifiChannelHelper()
    channel.SetPropagationDelay('ns3::ConstantSpeedPropagationDelayModel')
    channel.AddPropagationLoss('ns3::TwoRayGroundPropagationLossModel')

    phy = ns.wifi.YansWifiPhyHelper()
    phy.SetChannel(channel.Create())

    wifi = ns.wifi.WifiHelper()
    wifi.SetStandard(ns.wifi.WIFI_STANDARD_80211a)
    wifi.SetRemoteStationManager('ns3::ConstantRateWifiManager',
                                 'DataMode', 'OfdmRate6Mbps',
                                 'ControlMode', 'OfdmRate6Mbps')

    mac = ns.wifi.WifiMacHelper()
    mac.SetType('ns3::AdhocWifiMac')

    devices = wifi.Install(phy, mac, nodes)

    # Mobility
    pos = ns.mobility.ListPositionAllocator()
    pos.Add((0.0, 0.0, 0.0))
    pos.Add((distance, 0.0, 0.0))
    mobility = ns.mobility.MobilityHelper()
    mobility.SetPositionAllocator(pos)
    mobility.SetMobilityModel('ns3::ConstantPositionMobilityModel')
    mobility.Install(nodes)

    # Internet
    stack = ns.internet.InternetStackHelper()
    stack.Install(nodes)

    address = ns.internet.Ipv4AddressHelper()
    address.SetBase('10.1.1.0', '255.255.255.0')
    ifaces = address.Assign(devices)

    # OnOff
    onoff = ns.applications.OnOffHelper('ns3::UdpSocketFactory',
                                        (ifaces.GetAddress(1), 9))
    onoff.SetAttribute('DataRate', ns.core.StringValue('1Mbps'))
    onoff.SetAttribute('PacketSize', ns.core.UintegerValue(1000))
    apps = onoff.Install(nodes.Get(0))
    apps.Start(Seconds(1.0))
    apps.Stop(Seconds(10.0))

    # Sink
    sink = ns.applications.PacketSinkHelper('ns3::UdpSocketFactory',
                                            ns.network.InetSocketAddress(
                                                ns.network.Ipv4Address.GetAny(), 9))
    apps = sink.Install(nodes.Get(1))
    apps.Start(Seconds(0.0))
    apps.Stop(Seconds(10.0))

    # FlowMonitor & NetAnim
    fmHelper = ns.flow_monitor.FlowMonitorHelper()
    monitor = fmHelper.InstallAll()
    anim = ns.netanim.AnimationInterface('Lab1_TwoRay.xml')

    ns.core.Simulator.Stop(Seconds(10.0))
    ns.core.Simulator.Run()
    stats = monitor.GetFlowStats()[1]
    throughput = stats.rxBytes * 8.0 / 9.0
    print(f"TwoRay Python throughput: {throughput} bps")

    ns.core.Simulator.Destroy()

if __name__ == '__main__':
    main()