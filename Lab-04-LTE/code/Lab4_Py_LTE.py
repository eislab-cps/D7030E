#!/usr/bin/env python3
# code/Lab4_Py_LTE.py
# Lab 4 Python Starter: LTE EPC End-to-End Throughput
# Usage: python3 code/Lab4_Py_LTE.py --dataRate <Mbps> --antenna <type> --distance <m> --seed <run>

import ns.core
import ns.network
import ns.lte
import ns.internet
import ns.mobility
import ns.applications
import ns.flow_monitor
import ns.netanim

from ns.core import CommandLine, Seconds, UintegerValue, DataRateValue
from ns.network import NodeContainer, Ipv4AddressHelper
from ns.lte import LteHelper, PointToPointEpcHelper
from ns.internet import InternetStackHelper, Ipv4StaticRoutingHelper
from ns.mobility import MobilityHelper, ListPositionAllocator
from ns.applications import OnOffHelper, PacketSinkHelper
from ns.flow_monitor import FlowMonitorHelper
from ns.netanim import AnimationInterface

def main():
    # Parameters
    dataRate = 1.0
    antennaType = 'Isotropic'
    distance = 100.0
    seed = 1
    mobility = False
    ueSpeed = 10.0

    cmd = CommandLine()
    cmd.AddValue('dataRate', 'Application data rate in Mbps', dataRate)
    cmd.AddValue('antenna', 'Antenna model: Parabolic, Cosine, Isotropic', antennaType)
    cmd.AddValue('distance', 'UE distance (m)', distance)
    cmd.AddValue('seed', 'RngRun seed value', seed)
    cmd.AddValue('mobility', 'Enable UE mobility (ConstantVelocityMobilityModel)', mobility)
    cmd.AddValue('speed', 'UE speed in m/s when --mobility=true', ueSpeed)
    cmd.Parse()

    distance = float(distance)
    ueSpeed = float(ueSpeed)

    # RNG & time
    ns.core.RngSeedManager.SetSeed(1)
    ns.core.RngSeedManager.SetRun(seed)
    ns.core.Time.SetResolution(ns.core.Time.NS)

    # LTE + EPC setup
    lte = LteHelper()
    epc = PointToPointEpcHelper()
    lte.SetEpcHelper(epc)
    lte.SetAttribute('PathlossModel', ns.core.StringValue('ns3::TwoRayGroundPropagationLossModel'))
    lte.SetEnbAntennaModelType(antennaType)
    lte.SetSchedulerType('ns3::PfFfMacScheduler')
    lte.SetAttribute('DlEarfcn', ns.core.UintegerValue(100))
    lte.SetAttribute('UlEarfcn', ns.core.UintegerValue(18100))

    # Create nodes
    enbNodes = NodeContainer()
    enbNodes.Create(1)
    ueNodes = NodeContainer()
    ueNodes.Create(1)

    # Mobility
    mob = MobilityHelper()
    mob.SetMobilityModel('ns3::ConstantPositionMobilityModel')
    enbPos = ListPositionAllocator()
    enbPos.Add(ns.core.Vector(0.0, 0.0, 0.0))
    mob.SetPositionAllocator(enbPos)
    mob.Install(enbNodes)

    uePos = ListPositionAllocator()
    uePos.Add(ns.core.Vector(distance, 0.0, 0.0))
    if mobility:
        ueMob = MobilityHelper()
        ueMob.SetPositionAllocator(uePos)
        ueMob.SetMobilityModel('ns3::ConstantVelocityMobilityModel')
        ueMob.Install(ueNodes)
        ueNodes.Get(0).GetObject(ns.mobility.ConstantVelocityMobilityModel.GetTypeId()) \
               .SetVelocity(ns.core.Vector(-ueSpeed, 0.0, 0.0))
        print(f"Mobile UE: speed={ueSpeed} m/s (toward eNB)")
    else:
        mob.SetPositionAllocator(uePos)
        mob.Install(ueNodes)

    # Install devices
    enbDevs = lte.InstallEnbDevice(enbNodes)
    ueDevs = lte.InstallUeDevice(ueNodes)

    # Install IP stack on UE
    InternetStackHelper().Install(ueNodes)
    pgw = epc.GetPgwNode()
    remote = NodeContainer()
    remote.Create(1)
    InternetStackHelper().Install(remote)

    # Point-to-point link to PGW
    p2p = ns.internet.PointToPointHelper()
    p2p.SetDeviceAttribute('DataRate', ns.core.StringValue('100Gbps'))
    p2p.SetChannelAttribute('Delay', ns.core.StringValue('10ms'))
    devs = p2p.Install(pgw, remote.Get(0))

    # Assign IPs
    addr = Ipv4AddressHelper()
    addr.SetBase('1.0.0.0', '255.0.0.0')
    internetIfaces = addr.Assign(devs)

    ueIfaces = epc.AssignUeIpv4Address(ueDevs)
    lte.Attach(ueDevs.Get(0), enbDevs.Get(0))

    # Routing for remote host
    staticHelper = Ipv4StaticRoutingHelper()
    remoteStatic = staticHelper.GetStaticRouting(remote.Get(0).GetObject(ns.network.Ipv4))
    remoteStatic.AddNetworkRouteTo(ns.network.Ipv4Address('7.0.0.0'),
                                  ns.network.Ipv4Mask('255.0.0.0'), 1)

    # Applications: UDP OnOff from remote -> UE
    port = 8000
    app_start = 2.0
    sim_stop = (distance + 500.0) / ueSpeed + 5.0 if mobility else 22.0
    app_stop = sim_stop - 2.0

    onoff = OnOffHelper('ns3::UdpSocketFactory',
                        ns.network.InetSocketAddress(ueIfaces.GetAddress(0), port))
    onoff.SetAttribute('DataRate', DataRateValue(ns.core.DataRate(int(dataRate * 1e6))))
    onoff.SetAttribute('PacketSize', UintegerValue(1024))
    client = onoff.Install(remote.Get(0))
    client.Start(Seconds(app_start))
    client.Stop(Seconds(app_stop))

    sink = PacketSinkHelper('ns3::UdpSocketFactory',
                            ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), port))
    sinkApp = sink.Install(ueNodes.Get(0))
    sinkApp.Start(Seconds(1.0))
    sinkApp.Stop(Seconds(sim_stop))

    # Per-second CSV logging for mobile UE
    g_last_rx = [0]
    g_csv_f = [None]

    def log_mobile_throughput(sink_ptr, now):
        rx = sink_ptr.GetTotalRx()
        thr = (rx - g_last_rx[0]) * 8.0
        g_last_rx[0] = rx
        if g_csv_f[0]:
            g_csv_f[0].write(f"{now},{int(thr)}\n")
        ns.core.Simulator.Schedule(
            Seconds(1.0), log_mobile_throughput, sink_ptr, now + 1.0
        )

    if mobility:
        g_csv_f[0] = open("ue_mobile_throughput.csv", "w")
        g_csv_f[0].write("time_s,throughput_bps\n")
        sink_ptr = sinkApp.Get(0)
        ns.core.Simulator.Schedule(
            Seconds(app_start + 1.0), log_mobile_throughput, sink_ptr, app_start + 1.0
        )

    # FlowMonitor & NetAnim
    fm = FlowMonitorHelper()
    monitor = fm.InstallAll()
    anim = AnimationInterface('Lab4_LTE.xml')

    ns.core.Simulator.Stop(Seconds(sim_stop))
    ns.core.Simulator.Run()

    if g_csv_f[0]:
        g_csv_f[0].close()

    monitor.CheckForLostPackets()
    stats = monitor.GetFlowStats()
    thr = stats[1].rxBytes * 8.0 / (app_stop - app_start)
    print(f"LTE downlink throughput: {thr} bps")

    ns.core.Simulator.Destroy()

if __name__ == '__main__':
    main()
