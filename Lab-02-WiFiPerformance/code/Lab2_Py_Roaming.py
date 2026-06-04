#!/usr/bin/env python3
# code/Lab2_Py_Roaming.py
# Lab 2 Python Starter: Smart-Building WiFi Roaming
# Usage: python3 Lab2_Py_Roaming.py --speed <m/s> --seed <run> --logInterval <s> --simDuration <s>
#
# Topology:
#   AP0 at (0,0,0) and AP1 at (50,0,0) — same SSID "SmartBuilding", ch1, 802.11b
#   STA starts at (-20,0,0), moves at --speed m/s toward (70,0,0)
#   Remote server connected via CSMA backbone bridged to both APs
#
# Output: roaming_throughput.csv (time_s,throughput_bps)

import ns.core
import ns.network
import ns.wifi
import ns.mobility
import ns.internet
import ns.applications
import ns.csma
import ns.bridge
import ns.netanim

from ns.core import CommandLine, Seconds, StringValue, UintegerValue, BooleanValue
from ns.network import NodeContainer, Ipv4AddressHelper, InetSocketAddress, Ipv4Address
from ns.wifi import YansWifiChannelHelper, YansWifiPhyHelper, WifiHelper, WifiMacHelper, Ssid
from ns.mobility import MobilityHelper, ListPositionAllocator
from ns.internet import InternetStackHelper
from ns.applications import OnOffHelper, PacketSinkHelper
from ns.csma import CsmaHelper
from ns.bridge import BridgeHelper
from ctypes import c_double, c_int, c_bool
import os

g_lastRx = 0
g_csv = None
log_interval = 1.0


def log_throughput(sink, now):
    global g_lastRx, g_csv, log_interval
    rx = sink.GetTotalRx()
    thr = (rx - g_lastRx) * 8.0 / log_interval
    g_lastRx = rx
    g_csv.write(f"{now},{int(thr)}\n")
    ns.core.Simulator.Schedule(
        Seconds(log_interval), log_throughput, sink, now + log_interval
    )


def main():
    global g_lastRx, g_csv, log_interval

    speed = c_double(5.0)
    seed = c_int(1)
    sim_duration = c_double(25.0)
    enable_anim = c_bool(False)
    os.makedirs("scratch/Lab2outputs", exist_ok=True)
    csv_path = "scratch/Lab2outputs/roaming_throughput.csv"

    cmd = CommandLine()
    cmd.AddValue("speed",       "STA velocity in m/s.",            speed)
    cmd.AddValue("seed",        "RNG run number.",                 seed)
    cmd.AddValue("logInterval", "Throughput sampling period (s).", log_interval)
    cmd.AddValue("simDuration", "Total simulation time (s).",      sim_duration)
    cmd.AddValue("enableAnim",  "Write roaming_anim.xml.",         enable_anim)
    cmd.Parse()

    ns.core.RngSeedManager.SetSeed(1)
    ns.core.RngSeedManager.SetRun(seed.value)
    ns.core.Time.SetResolution(ns.core.Time.NS)

    # --- Nodes ---
    apNodes = NodeContainer()
    apNodes.Create(2)      # AP0, AP1
    staNode = NodeContainer()
    staNode.Create(1)      # mobile STA
    serverNode = NodeContainer()
    serverNode.Create(1)   # remote server

    # --- WiFi (802.11b, SSID SmartBuilding) ---
    wifiCh = YansWifiChannelHelper()
    wifiCh.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel")
    wifiCh.AddPropagationLoss("ns3::LogDistancePropagationLossModel")
    phy = YansWifiPhyHelper()
    phy.SetChannel(wifiCh.Create())

    wifi = WifiHelper()
    wifi.SetStandard(ns.wifi.WIFI_STANDARD_80211b)
    wifi.SetRemoteStationManager("ns3::IdealWifiManager")

    # AP side
    ssid = Ssid("SmartBuilding")
    macAp = WifiMacHelper()
    macAp.SetType("ns3::ApWifiMac", "Ssid", ssid)
    apWifiDevs = wifi.Install(phy, macAp, apNodes)

    # STA side
    macSta = WifiMacHelper()
    macSta.SetType("ns3::StaWifiMac",
                   "Ssid", ssid,
                   "ActiveProbing", BooleanValue(True))
    staWifiDev = wifi.Install(phy, macSta, staNode)

    # --- CSMA backbone ---
    csma = CsmaHelper()
    csma.SetChannelAttribute("DataRate", StringValue("1Gbps"))
    csma.SetChannelAttribute("Delay",    StringValue("1ms"))
    backboneNodes = NodeContainer()
    backboneNodes.Add(apNodes)
    backboneNodes.Add(serverNode)
    backboneDevs = csma.Install(backboneNodes)

    # --- Bridge each AP: WiFi + CSMA ---
    bridgeHelper = BridgeHelper()
    for i in range(2):
        bridged = ns.network.NetDeviceContainer()
        bridged.Add(apWifiDevs.Get(i))
        bridged.Add(backboneDevs.Get(i))
        bridgeHelper.Install(apNodes.Get(i), bridged)

    # --- Internet stack ---
    stack = InternetStackHelper()
    stack.Install(staNode)
    stack.Install(serverNode)

    # Assign IPs
    addr = Ipv4AddressHelper()
    addr.SetBase("10.1.1.0", "255.255.255.0")
    addr.Assign(staWifiDev)
    addr.Assign(backboneDevs.Get(2))   # server's CSMA device

    # --- Mobility ---
    apPos = ListPositionAllocator()
    apPos.Add(ns.core.Vector(0.0,  0.0, 0.0))
    apPos.Add(ns.core.Vector(50.0, 0.0, 0.0))
    apMob = MobilityHelper()
    apMob.SetPositionAllocator(apPos)
    apMob.SetMobilityModel("ns3::ConstantPositionMobilityModel")
    apMob.Install(apNodes)

    staPos = ListPositionAllocator()
    staPos.Add(ns.core.Vector(-20.0, 0.0, 0.0))
    staMob = MobilityHelper()
    staMob.SetPositionAllocator(staPos)
    staMob.SetMobilityModel("ns3::ConstantVelocityMobilityModel")
    staMob.Install(staNode)
    cvmm = staNode.Get(0).GetObject(ns.mobility.ConstantVelocityMobilityModel.GetTypeId())
    cvmm.SetVelocity(ns.core.Vector(speed.value, 0.0, 0.0))

    serverMob = MobilityHelper()
    serverMob.SetMobilityModel("ns3::ConstantPositionMobilityModel")
    serverMob.Install(serverNode)

    # --- Applications ---
    port = 9
    app_start = 1.0
    app_stop  = sim_duration.value - 1.0

    staIpv4 = staNode.Get(0).GetObject(ns.internet.Ipv4.GetTypeId())
    staAddr = staIpv4.GetAddress(1, 0).GetLocal()

    onoff = OnOffHelper("ns3::UdpSocketFactory",
                        InetSocketAddress(staAddr, port))
    onoff.SetAttribute("DataRate",   StringValue("100Mbps"))
    onoff.SetAttribute("PacketSize", UintegerValue(1024))
    onoff.SetAttribute("OnTime",     StringValue("ns3::ConstantRandomVariable[Constant=1]"))
    onoff.SetAttribute("OffTime",    StringValue("ns3::ConstantRandomVariable[Constant=0]"))
    src = onoff.Install(serverNode.Get(0))
    src.Start(Seconds(app_start))
    src.Stop(Seconds(app_stop))

    sinkH = PacketSinkHelper("ns3::UdpSocketFactory",
                             InetSocketAddress(Ipv4Address.GetAny(), port))
    sinkApp = sinkH.Install(staNode.Get(0))
    sinkApp.Start(Seconds(0.0))
    sinkApp.Stop(Seconds(sim_duration.value))

    # --- CSV output ---
    g_csv = open(csv_path, "w")
    g_csv.write("time_s,throughput_bps\n")

    # --- NetAnim ---
    anim = None
    if enable_anim.value:
        anim = ns.netanim.AnimationInterface("scratch/Lab2outputs/roaming_anim.xml")

    # --- Schedule logging + run ---
    ns.core.Simulator.Stop(Seconds(sim_duration.value))
    sink_ptr = sinkApp.Get(0)
    ns.core.Simulator.Schedule(
        Seconds(log_interval), log_throughput, sink_ptr, log_interval
    )
    ns.core.Simulator.Run()

    g_csv.close()
    ns.core.Simulator.Destroy()


if __name__ == "__main__":
    main()
