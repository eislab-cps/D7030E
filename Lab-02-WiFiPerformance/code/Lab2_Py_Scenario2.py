#!/usr/bin/env python3
# code/Lab2_Py_Scenario2.py
# Lab 2 Python Starter: Infrastructure WiFi Scenario 2 (Two Triangles)
# Usage: python3 Lab2_Py_Scenario2.py --rate <Mbps> --seed <run> --distance <m>

import ns.core
import ns.network
import ns.wifi
import ns.mobility
import ns.internet
import ns.applications
import ns.flow_monitor
import ns.netanim

from ns.core import CommandLine, Seconds, StringValue, UintegerValue
from ns.network import NodeContainer, Ipv4AddressHelper
from ns.wifi import YansWifiChannelHelper, YansWifiPhyHelper, WifiHelper, WifiMacHelper, Ssid
from ns.mobility import MobilityHelper, ListPositionAllocator
from ns.internet import InternetStackHelper
from ns.applications import OnOffHelper, PacketSinkHelper
from ns.flow_monitor import FlowMonitorHelper
from ns.netanim import AnimationInterface
from ctypes import c_double, c_int, c_bool
import os


def main():
    os.makedirs("scratch/Lab2outputs", exist_ok=True)
    # Parse parameters
    rate = c_double(1.0)    # Mbps
    seed = c_int(1)         # run number
    distance = c_double(10.0)  # separation (m)
    cmd = CommandLine()
    cmd.AddValue('rate', 'PHY data rate in Mbps', rate)
    cmd.AddValue('seed', 'RngRun seed value', seed)
    cmd.AddValue('distance', 'Distance between STA and AP (m)', distance)
    cmd.Parse()

    # Set RNG
    ns.core.RngSeedManager.SetSeed(1)
    ns.core.RngSeedManager.SetRun(seed.value)

    ns.core.Time.SetResolution(ns.core.Time.NS)

    # Create nodes
    staNodes = NodeContainer()
    staNodes.Create(2)
    apNode = NodeContainer()
    apNode.Create(1)

    # WiFi channel and PHY
    channelHelper = YansWifiChannelHelper()
    channelHelper.SetPropagationDelay('ns3::ConstantSpeedPropagationDelayModel')
    phy = YansWifiPhyHelper()
    phy.SetChannel(channelHelper.Create())

    # Wifi helper and MAC
    wifi = WifiHelper()
    wifi.SetStandard(ns.wifi.WIFI_STANDARD_80211b)
    dataMode = f"DsssRate{int(rate.value)}Mbps" if rate.value != 5.5 else "DsssRate5_5Mbps"
    wifi.SetRemoteStationManager(
        'ns3::ConstantRateWifiManager',
        'DataMode', StringValue(dataMode),
        'ControlMode', StringValue(dataMode)
    )

    mac = WifiMacHelper()
    ssid = Ssid('lab2-ssid')

    # Install STA devices
    mac.SetType('ns3::StaWifiMac', 'Ssid', ssid)
    staDevices = wifi.Install(phy, mac, staNodes)

    # Install AP device
    mac.SetType('ns3::ApWifiMac', 'Ssid', ssid)
    apDevices = wifi.Install(phy, mac, apNode)

    # Mobility: positions [-d,0], [0,0], [d,0]
    posAlloc = ListPositionAllocator()
    posAlloc.Add((-distance.value, 0.0, 0.0))
    posAlloc.Add((0.0, 0.0, 0.0))
    posAlloc.Add((distance.value, 0.0, 0.0))
    mobility = MobilityHelper()
    mobility.SetPositionAllocator(posAlloc)
    mobility.SetMobilityModel('ns3::ConstantPositionMobilityModel')
    mobility.Install(staNodes)
    mobility.Install(apNode)

    # Internet stack and IP
    stack = InternetStackHelper()
    stack.Install(staNodes)
    stack.Install(apNode)

    addressHelper = Ipv4AddressHelper()
    addressHelper.SetBase('10.1.3.0', '255.255.255.0')
    staIfaces = addressHelper.Assign(staDevices)
    addressHelper.Assign(apDevices)

    # OnOff for two flows: ports 9 & 10
    onoff1 = OnOffHelper('ns3::UdpSocketFactory',
                         ns.network.InetSocketAddress(staIfaces.GetAddress(2), 9))
    onoff1.SetAttribute('DataRate', StringValue(f'{rate.value}Mbps'))
    onoff1.SetAttribute('PacketSize', UintegerValue(1000))
    app1 = onoff1.Install(staNodes.Get(0))
    app1.Start(Seconds(1.0)); app1.Stop(Seconds(10.0))

    onoff2 = OnOffHelper('ns3::UdpSocketFactory',
                         ns.network.InetSocketAddress(staIfaces.GetAddress(2), 10))
    onoff2.SetAttribute('DataRate', StringValue(f'{rate.value}Mbps'))
    onoff2.SetAttribute('PacketSize', UintegerValue(1000))
    app2 = onoff2.Install(staNodes.Get(1))
    app2.Start(Seconds(1.0)); app2.Stop(Seconds(10.0))

    # Packet sinks on AP
    sink1 = PacketSinkHelper('ns3::UdpSocketFactory',
                             ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), 9))
    sink2 = PacketSinkHelper('ns3::UdpSocketFactory',
                             ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), 10))
    srv1 = sink1.Install(apNode.Get(0)); srv1.Start(Seconds(0.0)); srv1.Stop(Seconds(10.0))
    srv2 = sink2.Install(apNode.Get(0)); srv2.Start(Seconds(0.0)); srv2.Stop(Seconds(10.0))

    # FlowMonitor & NetAnim
    fmHelper = FlowMonitorHelper()
    monitor = fmHelper.InstallAll()
    anim = AnimationInterface('scratch/Lab2outputs/Lab2_Scenario2.xml')

    # Run simulation
    ns.core.Simulator.Stop(Seconds(10.0))
    ns.core.Simulator.Run()

    # Compute and print throughput
    monitor.CheckForLostPackets()
    stats = monitor.GetFlowStats()
    thr1 = stats[1].rxBytes * 8.0 / 9.0
    thr2 = stats[2].rxBytes * 8.0 / 9.0
    print(f"Scenario2 flow1 throughput: {thr1} bps")
    print(f"Scenario2 flow2 throughput: {thr2} bps")

    ns.core.Simulator.Destroy()

if __name__ == '__main__':
    main()
