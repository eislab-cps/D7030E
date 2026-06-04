#!/usr/bin/env python3
# Lab 1 Python Starter: Nakagami Fading Model (ns-3.47)
# Layers Nakagami small-scale fading on top of Friis large-scale path loss.
# Usage: python3 Lab1_Py_Nakagami.py --distance=100 --seed=1

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
import os


def main():
    os.makedirs("scratch/Lab1outputs", exist_ok=True)
    distance = c_double(50.0)
    seed = c_int(1)
    cmd = CommandLine()
    cmd.AddValue('distance', 'Distance between nodes (m)', distance)
    cmd.AddValue('seed', 'RNG run number', seed)
    cmd.Parse()

    ns.core.RngSeedManager.SetSeed(1)
    ns.core.RngSeedManager.SetRun(seed.value)
    ns.core.Time.SetResolution(ns.core.Time.NS)

    nodes = ns.network.NodeContainer()
    nodes.Create(2)

    # Channel: Friis large-scale + Nakagami small-scale fading
    channelHelper = ns.wifi.YansWifiChannelHelper()
    channelHelper.SetPropagationDelay('ns3::ConstantSpeedPropagationDelayModel')
    channelHelper.AddPropagationLoss('ns3::FriisPropagationLossModel')
    channelHelper.AddPropagationLoss('ns3::NakagamiPropagationLossModel',
                                      'm0', ns.core.DoubleValue(1.0),
                                      'm1', ns.core.DoubleValue(1.0),
                                      'm2', ns.core.DoubleValue(1.0))

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

    posAlloc = ns.mobility.ListPositionAllocator()
    posAlloc.Add((0.0, 0.0, 0.0))
    posAlloc.Add((distance.value, 0.0, 0.0))
    mobility = ns.mobility.MobilityHelper()
    mobility.SetPositionAllocator(posAlloc)
    mobility.SetMobilityModel('ns3::ConstantPositionMobilityModel')
    mobility.Install(nodes)

    stack = ns.internet.InternetStackHelper()
    stack.Install(nodes)

    address = ns.internet.Ipv4AddressHelper()
    address.SetBase('10.1.1.0', '255.255.255.0')
    interfaces = address.Assign(devices)

    onoff = ns.applications.OnOffHelper('ns3::UdpSocketFactory',
                                        (interfaces.GetAddress(1), 9))
    onoff.SetAttribute('DataRate', ns.core.StringValue('1Mbps'))
    onoff.SetAttribute('PacketSize', ns.core.UintegerValue(1000))
    app = onoff.Install(nodes.Get(0))
    app.Start(Seconds(1.0))
    app.Stop(Seconds(10.0))

    sink = ns.applications.PacketSinkHelper('ns3::UdpSocketFactory',
                                            ns.network.InetSocketAddress(
                                                ns.network.Ipv4Address.GetAny(), 9))
    sinkApp = sink.Install(nodes.Get(1))
    sinkApp.Start(Seconds(0.0))
    sinkApp.Stop(Seconds(10.0))

    flowmonHelper = ns.flow_monitor.FlowMonitorHelper()
    monitor = flowmonHelper.InstallAll()
    anim = ns.netanim.AnimationInterface('scratch/Lab1outputs/Lab1_Nakagami.xml')

    ns.core.Simulator.Stop(Seconds(11.0))
    ns.core.Simulator.Run()

    monitor.CheckForLostPackets()
    stats = monitor.GetFlowStats()
    throughput = stats[1].rxBytes * 8.0 / 9.0
    print(f"Nakagami Python throughput: {throughput:.0f} bps")

    ns.core.Simulator.Destroy()


if __name__ == '__main__':
    main()
