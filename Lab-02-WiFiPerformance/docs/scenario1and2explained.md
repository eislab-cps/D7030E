# Scenario 1 — Infrastructure Wi-Fi throughput (one AP, stations talk through it)

**What this is:** A basic home/office Wi-Fi setup: one access point (AP) in the middle; stations (STAs) send UDP traffic through the AP to a receiver. You measure how much useful data per second (application-layer throughput) you actually get when you change the physical (PHY) data rate. 

**Two sub-cases you’ll compare:**

1. **One sender → AP → one receiver** (clean, no contention).
2. **Two independent senders → the same AP** (they must share the channel, so they contend and take turns). The topology uses equilateral triangles (10 m sides) so all links have similar path loss and the geometry doesn’t bias the results.  

**What you vary:** The Wi-Fi PHY mode for 802.11b: 1, 5.5, and 11 Mb/s. You repeat each experiment with two different random seeds so you’re not “learning” one lucky run. 

**What you’re really measuring:** Not the headline PHY rate, but **goodput** after MAC/PHY overhead, ACKs, inter-frame spaces, backoff, and collisions do their thing. Expect the measured throughput to be **below** the PHY rate, sometimes far below when two senders are contending. The lab intentionally offers more app traffic than the link can carry so you see saturation behavior (fair sharing, collision/backoff effects). 

**What the starter code sets up:**

* 802.11b standard, fixed data mode (ConstantRateWifiManager).
* A UDP **OnOff** app (1000-B payload) driving traffic; a **PacketSink** receives it.
* A **FlowMonitor** tallies bytes received so you can compute throughput over the active window.
  That’s the minimal scaffold for “AP + stations + traffic + measurement.”    

> Heads-up: PCAPs are captured in **promiscuous** mode, so a STA can “hear” packets not destined to it. If you try to compute throughput from a STA’s PCAP, you’ll double-count unless you correct for that. For this lab, FlowMonitor is the clean way to measure. 

# Scenario 2, Part 1 — Packet-size (payload) sweep

**What this is:** Same basic AP topology, but now the question is: *How does packet size (payload) change the goodput at different PHY rates?* You switch to the **Two-Ray Ground** propagation model and place nodes at **dᵢ/2** (a distance derived in your previous lab) so path loss matches the model’s assumptions.  

**What you vary:** PHY ∈ {1, 5.5, 11 Mb/s} × payload ∈ {400, 700, 1000 B}. For each combination, run two seeds and compute average throughput. 

**What’s really happening:**

* Each packet pays a **fixed overhead** (MAC header, PHY preamble, ACK, IFS, backoff).
* **Small payloads** waste a higher fraction of airtime on overhead → lower goodput.
* **Larger payloads** amortize overhead better → higher goodput (until losses/retries bite).
  You’ll also time **one packet** at 11 Mb/s with 400 B to see that “PHY rate ≠ goodput” in practice. 

# Scenario 2, Part 2 — Hidden-terminal experiment (with and without RTS/CTS)

**What this is:** The classic Wi-Fi failure mode: two senders can’t hear each other (**hidden**), but both talk to the **same AP**. They transmit at the same time, collide at the AP, and both lose. You build a 3-node line: **STA0 — AP — STA1**, spaced so the STAs are out of each other’s range (use distance **dᵢ** on each side with Two-Ray Ground). Traffic is UDP, 1000 B at 1 Mb/s, one flow per STA into the AP. 

**What you toggle:** **RTS/CTS**.

* **Off** (high threshold) → more collisions, lower **packet delivery ratio (PDR)** and throughput.
* **On** (threshold = 0) → senders handshake first (RTS/CTS), so the AP reserves the medium and collisions drop; good for hidden terminals and large frames, but adds extra control overhead. You measure throughput and PDR in both modes and compare.  

# How the ns-3 pieces map to Wi-Fi concepts (so the code makes conceptual sense)

* **Channel/PHY helpers** (YansWifiChannel/YansWifiPhy): a simplified radio + propagation model so frames take airtime and suffer path loss. 
* **Wi-Fi MAC** (WifiHelper/WifiMacHelper): sets 802.11b, data mode, and AP/STA roles — i.e., infrastructure mode with association to an SSID.  
* **Mobility/positions:** your geometry (triangles or line) controls who can hear whom and how strong links are; that’s why the distances matter so much.  
* **Internet stack + IP**: gives you sockets so an app can send UDP through the AP.  
* **Applications** (OnOff + PacketSink): generate steady UDP traffic toward a port; different ports = different flows.  
* **FlowMonitor**: counts bytes received per flow so you can compute throughput and PDR cleanly.  
* **Seeds** (RngSeedManager): change random backoff/collision timing so your results aren’t tied to one RNG outcome. 

