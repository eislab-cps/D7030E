# Lab 03: UAV Mesh & Ad-hoc Network Performance

**Language Options:** Complete this lab in **C++ or Python** (choose one).  
**ns-3 version:** 3.47

---

## Scenario Context

| Setting | Model | ns-3 Feature |
|---------|-------|--------------|
| UAV swarm / drone mesh | Ad-hoc IBSS, multi-hop chain | `AdhocWifiMac`, OLSR/AODV |
| Industrial sensor network | Low-rate 802.11b, fixed spacing | `ConstantRateWifiManager` |
| Search-and-rescue relay | Reactive routing (AODV) | `AodvHelper` |
| Multi-hop UDP pipeline | FlowMonitor, payload sweep | `FlowMonitorHelper` |

---

## Objectives

By the end of this lab you will:

1. Build an ad-hoc WiFi chain and measure UDP throughput over multiple hops.  
2. Analyze throughput vs. number of hops and packet size.  
3. Compare TCP vs. UDP performance in a 3-node chain.  
4. Compare proactive routing (OLSR) vs. reactive routing (AODV) for a mesh scenario.  
5. Revisit the hidden-terminal problem in an ad-hoc setup with RTS/CTS on/off.  
6. Automate multiple seeds and visualize all scenarios in NetAnim.  

---

## Prerequisites & Setup

- Installation & build: [common/setup.md](../common/setup.md)  
- API & tutorials: [common/links.md](../common/links.md)  

---

## Part 1: Multi-Hop UDP Throughput

### Task (C++): Chain Setup & Measurement

1. **Copy** starter:
   ```bash
   cp code/Lab3_Cpp_Adhoc.cc ~/ns-allinone-3.47/ns-3.47/scratch/
    ```

2. **Rebuild**:

   ```bash
   cd ~/ns-allinone-3.47/ns-3.47
   cmake --build build -j$(nproc)
   ```
3. **Configure** number of nodes (`--numNodes`), distance (`--distance`), packet size (`--pktSize`) as script arguments.
4. **Run** multiple seeds:

   ```bash
   ./ns3 run "scratch/Lab3_Cpp_Adhoc --numNodes=4 --pktSize=500 --distance=200 --seed=1"
   ./ns3 run "scratch/Lab3_Cpp_Adhoc --numNodes=4 --pktSize=500 --distance=200 --seed=2"
   ```
5. **Record** UDP throughput for each seed and compute average.
6. **Visualize** with NetAnim (`AnimationInterface("lab3-adhoc.xml")`).

**Likely issues:**

* No routing → throughput=0: add OLSR or static routing helper (see troubleshooting).
* Python import error if using wrong build folder (for Python tasks).

### Task (Python): Chain Setup & Measurement

Repeat Part 1 in Python:

1. **Copy** `code/Lab3_Py_Adhoc.py` to `scratch/`.
2. **Set** `PYTHONPATH`:

   ```bash
   export PYTHONPATH=~/ns-allinone-3.47/ns-3.47/build/bindings/python:$PYTHONPATH
   ```
3. **Run** with the same arguments:

   ```bash
   python3 scratch/Lab3_Py_Adhoc.py --numNodes=5 --pktSize=700 --distance=150
   ```
4. **Collect** and average throughput; **generate** `lab3-py-adhoc.xml` for NetAnim.

---

## Part 2: Throughput vs. Hops & Packet Size

### Task (C++): Payload Sweep over Hop Counts

1. **Modify** or reuse `Lab3_Cpp_Adhoc.cc` to loop:

   ```cpp
   uint32_t hops[] = {3,4,5,6};
   uint32_t pkts[] = {300,700,1200};
   for (auto h: hops)
     for (auto p: pkts) { /* set numNodes=h, pktSize=p */ }
   ```
2. **Run** two seeds per experiment; **record** results in a table.
3. **Plot**:

   * Throughput vs. packet size for each hop count.
   * Throughput vs. number of hops for packet size = 1200 B.

### Task (Python): Sweep

Repeat Part 2 in Python with `Lab3_Py_PayloadSweep.py`; produce the same plots.

---

## Part 3: TCP vs. UDP Comparison

### Task (C++): TCP 3-Node Chain

1. **Copy** `Lab3_Cpp_TCP.cc` to `scratch/`.
2. **Set** TCP segment size to `pktSize`:

   ```cpp
   Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(pktSize));
   ```
3. **Run** for `numNodes=3`, `pktSize=300` and `1200`; two seeds each.
4. **Measure** and compare application throughput (TCP vs. UDP).

**Likely issue:**

* Low TCP throughput → adjust segment size (see troubleshooting).

### Task (Python): TCP

Repeat Part 3 in Python with `Lab3_Py_TCP.py`.

---

## Part 4: Routing Protocol Comparison — AODV vs. OLSR

The `--routing` flag in both `Lab3_Cpp_Adhoc.cc` and `Lab3_Py_Adhoc.py` selects between:

- **OLSR** (Optimized Link State Routing) — proactive, table-driven, RFC 3626.
- **AODV** (Ad hoc On-Demand Distance Vector) — reactive, route-on-demand, RFC 3561.

### Task

1. **Run** the 4-node chain (pktSize=1200, distance=200, seeds 1–2) with each protocol:

   ```bash
   ./ns3 run "scratch/Lab3_Cpp_Adhoc --numNodes=4 --pktSize=1200 --routing=olsr --seed=1"
   ./ns3 run "scratch/Lab3_Cpp_Adhoc --numNodes=4 --pktSize=1200 --routing=aodv --seed=1"
   ```

2. **Repeat** for 6-node chain (more hops = more routing overhead difference).

3. **Record** throughput and compare in `routing_comparison.csv`:

   ```
   routing,num_nodes,pkt_size,seed,throughput_bps
   ```

4. **Plot** throughput vs. number of hops for both routing protocols on the same graph.

5. **Discuss** in `routing_analysis.txt`:
   - Which protocol achieves higher throughput and why?
   - How does chain length affect the OLSR vs. AODV difference?
   - In a UAV swarm with frequent topology changes, which protocol would you prefer?

---

## Part 5: Hidden-Terminal in Ad-Hoc

### Task (C++): RTS/CTS Off vs. On

1. **Copy** `Lab3_Cpp_Hidden.cc` to `scratch/`.
2. **Place** STA0 at (0,0), AP at (`dᵢ`,0), STA1 at (2×`dᵢ`,0).
3. **Set** payload=1000 B, rate=1 Mbps.
4. **Run** with RTS/CTS disabled:

   ```cpp
   Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold","2200");
   ```
5. **Measure** throughput & packet-delivery ratio (PDR).
6. **Enable** RTS/CTS:

   ```cpp
   Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold","0");
   ```
7. **Rerun**, **re-measure**, and compare.

### Task (Python): Hidden-Terminal

Repeat Part 5 in Python with `Lab3_Py_Hidden.py`.

---

## Deliverables

See [`deliverables.md`](deliverables.md).

---

## Cross-References

* Setup: [common/setup.md](../common/setup.md)
* Troubleshooting: [common/troubleshooting.md](../common/troubleshooting.md)
* API & tutorial links: [common/links.md](../common/links.md)

---
