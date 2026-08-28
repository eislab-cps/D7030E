> **Background reading.** Explanatory notes only. The required work is defined by
> [Lab-03-Instructions.md](Lab-03-Instructions.md) and [deliverables.md](deliverables.md);
> where this note and those documents differ, they take precedence.

# Lab 03 — Ad-hoc Wi-Fi: Multi-hop Throughput, TCP vs UDP, Hidden Terminals (ns-3)


## What this lab is about

You’ll build small **ad-hoc Wi-Fi** networks in ns-3 and measure **application-level throughput** while you vary (a) **hops** and (b) **payload size**. Then you’ll compare **TCP vs UDP** on a 3-node chain. Finally, you’ll reproduce the **hidden-terminal** problem and show how **RTS/CTS** changes the results. 

Core settings you must stick to for the multi-hop experiments: **802.11b**, **constant 1 Mb/s PHY** (no rate fallback), nodes in a **line** with **200 m** spacing so **only adjacent nodes can hear each other** (that’s how we force multi-hop). Also: **routing must be enabled** between stations.

## What we provide (starters) and what they actually do

* **`Lab3_Cpp_Adhoc.cc`** — baseline **UDP** multi-hop chain. It sets **802.11b** with **ConstantRate 1 Mb/s**, **AdhocWifiMac**; places nodes on a line at `distance * i`; runs an **OnOff UDP** app from node 0 to the **last** node; records throughput with **FlowMonitor** over the **9 second** send window that follows the routing warm-up. It also writes a **NetAnim** XML.

* **`Lab3_Cpp_PayloadSweep.cc`** — same radio/topology, but loops over **payload sizes `{300,700,1200}`** and prints throughput per case. Use it to automate the payload sweep.  

* **`Lab3_Cpp_TCP.cc`** — 3-node **TCP** chain at positions **0/200/400 m**; sets TCP **segment size** to `pktSize`; drives a high **5 Mb/s** OnOff source to saturate the link; computes throughput from FlowMonitor.

* **`Lab3_Cpp_Hidden.cc`** — the **hidden-terminal** demo: three nodes on a line **STA0 — AP — STA1** (infrastructure MAC, on purpose), two **UDP** senders into the AP, payload **1000 B**, **1 Mb/s**, **RTS/CTS** toggled via `RtsCtsThreshold` (**0 = on**, **2200 = off**). Throughput is again measured over a 9 second window. This scenario is single-hop, so it needs no routing and no warm-up.

> Heads-up: the hidden-terminal starter uses **AP/STA** roles (infrastructure) because that cleanly demonstrates two hidden senders colliding at the AP. That’s fine for the concept, even though the chain experiments are **ad-hoc (IBSS)**. **Also, please read through the comments in the code.**

## What you must run and report

### Part 1 & 2 — Multi-hop UDP (the main study)

* Use **802.11b / DsssRate1Mbps**, **ConstantRate**, **200 m** spacing, **IBSS**. Sender is **node 0**; **last node** is the receiver. Measure **average application throughput**. 
* Vary **number of stations**: `{3, 4, 5, 6}`. Vary **UDP payload**: `{300, 700, 1200}`. For **each** combination, compute throughput. 
* Plots you must produce:

  * **Throughput vs packet size** for **each** hop count. 
  * **Throughput vs number of stations** at **1200 B**. 
* Also: **compare PHY nominal 1 Mb/s vs your measured app throughput** and explain the gap. 

### Part 3 — TCP vs UDP (3-node chain)

* Run **TCP** on the **3-node** chain with **segment size = {300, 1200}**, and compare **application throughput** to UDP. Keep the same radio/topology settings. 
* To change TCP segment size:
  `Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(pktSize));` (already in the starter). 

### Hidden terminals (RTS/CTS off vs on) — Part 5 in the instructions

* Two UDP senders (1000 B @ 1 Mb/s) into an AP; run **without** RTS/CTS, then **with** RTS/CTS (`RtsCtsThreshold = "2200"` → off; `"0"` → on). Report **throughput** and **PDR**, and compare. 

---

The command-line arguments of each starter are listed in the [lab README](../README.md).

1. **Seeds matter.** The code fixes the global seed and varies the **run**; do at least **two runs** per configuration and **average**. Example pattern: set `Run=1`, run; then `Run=2`, run again.  

2. **Visualization.** The ad-hoc and hidden starters write **NetAnim XML**; open these if you need to sanity-check topology and activity.  

## Throughput: how we compute it
* Each run first spends **30 s** letting OLSR converge, then sends for exactly **9 s**. Every starter measures over that same 9-second window, so the numbers are comparable across parts.

## What to look out for (common failure modes)

* **“Throughput = 0” on the chain?** Multi-hop packets won’t forward themselves — routing must be enabled, and it must have converged before traffic starts. The starters do both for you (OLSR, plus a 30 s warm-up). If you removed the warm-up or the routing helper while experimenting, put it back; if a run still reports zero with the arguments the lab asks for, tell a supervisor rather than adjusting the scenario.

* **Your nodes all hear each other?** Then you broke the geometry. Keep **200 m spacing** and place nodes **on a straight line** so only neighbors are in range. 

* **Hidden-terminal confusion:** That starter intentionally uses **AP/STA MACs** (infra) with `StaWifiMac` and `ApWifiMac`. That’s fine for demonstrating RTS/CTS, but don’t mix it up with IBSS used in the chain experiments. 

* **PHY vs app throughput:** Don’t expect **1 Mb/s** at the application. Airtime is burned on headers, ACKs, IFS, and contention. Comparing your measured numbers to the **nominal 1 Mb/s** is part of the task. 

* **Driving the link hard:** The app data rate is intentionally set high to saturate the link (UDP uses **1 Mb/s** in the starters; TCP uses **5 Mb/s**). That’s by design.   

* **FlowMonitor indexing:** Starters assume your main flow is **ID 1**. If you add more apps/flows, don’t hard-index blindly.  

## What to hand in

See [deliverables.md](deliverables.md).
