#!/usr/bin/env python3
# code/Lab2_Py_Scenario1.py
# Lab 2 Python Starter: Infrastructure WiFi Scenario 1 (Single AP, One Sender & Receiver)
# Usage: python3 Lab2_Py_Scenario1.py --rate <Mbps> --seed <run>

import ns.core
import ns.network
import ns.wifi
import ns.mobility
import ns.internet
import ns.applications
import ns.flow_monitor
import ns.netanim

from ns.core import CommandLine, Seconds, StringValue, UintegerValue, DoubleValue
from ns.network import NodeContainer, Ipv4AddressHelper
from ns.wifi import YansWifiChannelHelper, YansWifiPhyHelper, WifiHelper, WifiMacHelper, Ssid
from ns.mobility import MobilityHelper, ListPositionAllocator
from ns.internet import InternetStackHelper
from ns.applications import OnOffHelper, PacketSinkHelper
from ns.flow_monitor import FlowMonitorHelper
from ns.netanim import AnimationInterface
from ctypes import c_double, c_int, c_bool

def main():
    # Parse command-line arguments
    rate = c_double(1.0)   # Mbps
    seed = c_int(1)
    cmd = CommandLine()
    cmd.AddValue('rate', 'Physical layer data rate in Mbps', rate)
    cmd.AddValue('seed', 'RngRun seed value', seed)
    cmd.Parse()

    # Randomization
    ns.core.RngSeedManager.SetSeed(1)
    ns.core.RngSeedManager.SetRun(seed.value)

    ns.core.Time.SetResolution(ns.core.Time.NS)

    # Create nodes: two STAs and one AP
    wifiStaNodes = NodeContainer()
    wifiStaNodes.Create(2)
    wifiApNode = NodeContainer()
    wifiApNode.Create(1)

    # Configure WiFi channel and PHY
    channelHelper = YansWifiChannelHelper()
    channelHelper.SetPropagationDelay('ns3::ConstantSpeedPropagationDelayModel')
    phy = YansWifiPhyHelper()
    phy.SetChannel(channelHelper.Create())

    wifiHelper = WifiHelper()
    wifiHelper.SetStandard(ns.wifi.WIFI_STANDARD_80211b)
    dataMode = f"DsssRate{int(rate.value)}Mbps" if rate.value != 5.5 else "DsssRate5_5Mbps"
    wifiHelper.SetRemoteStationManager(
        "ns3::ConstantRateWifiManager",
        "DataMode", StringValue(dataMode),
        "ControlMode", StringValue(dataMode)
    )

    mac = WifiMacHelper()
    ssid = Ssid("lab2-ssid")

    # Install STA devices
    mac.SetType("ns3::StaWifiMac", "Ssid", ssid)
    staDevices = wifiHelper.Install(phy, mac, wifiStaNodes)

    # Install AP device
    mac.SetType("ns3::ApWifiMac", "Ssid", ssid)
    apDevices = wifiHelper.Install(phy, mac, wifiApNode)

    # Mobility: place nodes 10m apart in a row
    posAlloc = ListPositionAllocator()
    posAlloc.Add((0.0, 0.0, 0.0))
    posAlloc.Add((10.0, 0.0, 0.0))
    posAlloc.Add((20.0, 0.0, 0.0))
    mobility = MobilityHelper()
    mobility.SetPositionAllocator(posAlloc)
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel")
    mobility.Install(wifiStaNodes)
    mobility.Install(wifiApNode)

    # Internet stack
    stack = InternetStackHelper()
    stack.Install(wifiStaNodes)
    stack.Install(wifiApNode)

    # Assign IP addresses
    address = Ipv4AddressHelper()
    address.SetBase("10.1.2.0", "255.255.255.0")
    staInterfaces = address.Assign(staDevices)
    address.Assign(apDevices)

    # OnOff application: STA0 -> STA1
    onoff = OnOffHelper("ns3::UdpSocketFactory",
                        ns.network.InetSocketAddress(staInterfaces.GetAddress(1), 9))
    onoff.SetAttribute("DataRate", StringValue(f"{rate.value}Mbps"))
    onoff.SetAttribute("PacketSize", UintegerValue(1000))
    clientApp = onoff.Install(wifiStaNodes.Get(0))
    clientApp.Start(Seconds(1.0))
    clientApp.Stop(Seconds(10.0))

    # PacketSink on STA1
    sink = PacketSinkHelper("ns3::UdpSocketFactory",
                            ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), 9))
    serverApp = sink.Install(wifiStaNodes.Get(1))
    serverApp.Start(Seconds(0.0))
    serverApp.Stop(Seconds(10.0))

    # FlowMonitor and NetAnim
    fmHelper = FlowMonitorHelper()
    monitor = fmHelper.InstallAll()
    anim = AnimationInterface("Lab2_Scenario1.xml")

    # Run simulation
    ns.core.Simulator.Stop(Seconds(10.0))
    ns.core.Simulator.Run()

    # Compute throughput
    monitor.CheckForLostPackets()
    stats = monitor.GetFlowStats()
    rxBytes = stats[1].rxBytes
    throughput = rxBytes * 8.0 / 9.0  # bps
    print(f"Scenario1 Python throughput: {throughput} bps")

    ns.core.Simulator.Destroy()

if __name__ == "__main__":
    main()
