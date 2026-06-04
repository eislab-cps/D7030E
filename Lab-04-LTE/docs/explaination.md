# What this lab is about

You’ll spin up a tiny LTE network in ns-3: one base station (eNodeB), one phone (UE), a core network box (PGW/EPC), and a remote server that sends UDP traffic to the phone. Then you’ll (1) try three antenna patterns, (2) sweep the app data rate to see where throughput levels off, and (3) move the UE farther away to see how distance hurts throughput. You’ll also save LTE trace files and a PCAP as evidence. The picture on page 1 of the PDF shows the exact setup (eNB ↔ UE through EPC to a server). 

* Build the LTE scenario with **LteHelper**/**EpcHelper** (ns-3.40). Configure: AMC model (PiroEW2010), scheduler (PF), EARFCN (DL 100 / UL 18100), and bandwidth (50 RBs). Then turn on **RLC** and **PDCP** traces. 
* Generate downlink UDP traffic from the server to the UE (OnOffHelper). Run the sim at **three different app data rates** to see how much the link can really deliver. Save the `.trace` files. 
* Repeat with **three antenna types** on the eNodeB/UE: parabolic, cosine, isotropic. Note what changes in the traces, especially when the UE isn’t aligned with the directional antennas. 
* Do two small studies:

  1. **Throughput vs. data rate** (keep distance fixed; change the app rate).
  2. **Throughput vs. distance** (use **isotropic** antenna; keep app rate fixed; move the UE away). 

# What to hand in

Submit the following: 

**Part 1 — Trace collection**

* `DlRlcStats.trace` (raw RLC trace)
* `DlPdcpStats.trace` (raw PDCP trace)
* `server_trace.pcap` (packet capture on the server side)

**Part 2 — Antennas**

* `antenna_config_comparison.txt` (short, plain-English note on what changed between **parabolic / cosine / isotropic**, and any effects when the UE isn’t aligned)

**Part 3 — Throughput vs. data rate**

* `throughput_vs_rate.csv`  *(columns: `data_rate_mbps,throughput_bps`)*
* `throughput_vs_rate_plot.png` *(plot of DL throughput vs app rate)*

**Part 4 — Throughput vs. distance**

* `throughput_vs_distance.csv`  *(columns: `distance_m,throughput_bps`)*
* `throughput_vs_distance_plot.png` *(plot of DL throughput vs distance; **isotropic** antenna)*

**Part 5 — Discussion**

* `trace_choice.txt` (state whether you used **PDCP** or **RLC** bytes for throughput and why)
* `conclusions.txt` (what trends you saw and why they make sense)

# Which trace to use for throughput

Both RLC and PDCP traces contain byte counts. **PDCP** is closer to “what the app actually received” (after lower-layer retransmissions), so its totals usually track end-to-end throughput more cleanly. **RLC** can include retransmissions and may overcount for “delivered over the air.” The lab asks you to look at both and **choose**; explain your choice briefly in `trace_choice.txt`. 
