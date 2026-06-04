# Lab 02: Smart-Building WiFi Performance

**Language:** C++  
**ns-3 version:** 3.47

---

## Scenario Context

| Setting | Model | ns-3 Feature |
|---------|-------|--------------|
| Office / warehouse WiFi | Infrastructure BSS, single AP | `ApWifiMac` / `StaWifiMac` |
| Multi-AP smart building | Two APs, bridged backbone | `BridgeNetDevice`, `CsmaHelper` |
| IoT sensor roaming | Mobile STA, velocity model | `ConstantVelocityMobilityModel` |

---

## Objectives

By the end of this lab you will:

1. Measure application-layer throughput in an infrastructure WiFi network for two topologies.  
2. Sweep physical-layer data rate and packet payload to study throughput vs. packet size.  
3. Explore the hidden-terminal problem with and without RTS/CTS.  
4. Automate multiple seeds via RngSeedManager.  
5. Simulate a mobile client roaming between two APs in a smart-building layout.  
6. Visualize scenarios with NetAnim.  

---

## Prerequisites & Setup

- Installation & build: [common/setup.md](../common/setup.md)  
- API & tutorials: [common/links.md](../common/links.md)

---

## Part 1: Infrastructure WiFi Throughput

### Task (C++): Scenario 1 – Single AP, One Sender & Receiver

1. **Copy** `code/Lab2_Cpp_Scenario1.cc` to your `scratch/` directory.  
2. **Rebuild** ns-3:
3. **Edit** distances in the script to form an equilateral triangle of side 10 m.
4. **Set** MAC to IEEE 802.11b and traffic to UDP (payload=1000 B).
5. **Vary** PHY rate ∈ {1, 5.5, 11 Mbps}, running three seeds per rate:

### Scenario 1 (single flow, triangle)

Single run:

```bash
./ns3 run "scratch/Lab2_Cpp_Scenario1 --rate=11 --seed=1"
```

Full sweep (rates {1, 5.5, 11} × seeds {1, 2, 3}):

```bash
for r in 1 5.5 11; do
  for s in 1 2 3; do
    ./ns3 run "scratch/Lab2_Cpp_Scenario1 --rate=${r} --seed=${s}"
  done
done
```

### Scenario 2 (two flows, two triangles)

Single run:

```bash
./ns3 run "scratch/Lab2_Cpp_Scenario2 --rate=11 --seed=1"
```

Full sweep (rates {1, 5.5, 11} × seeds {1, 2, 3}):

```bash
for r in 1 5.5 11; do
  for s in 1 2 3; do
    ./ns3 run "scratch/Lab2_Cpp_Scenario2 --rate=${r} --seed=${s}"
  done
done
```

If you want to keep logs:

```bash
./ns3 run "scratch/Lab2_Cpp_Scenario1 --rate=11 --seed=1" | tee -a scenario1_runs.txt
./ns3 run "scratch/Lab2_Cpp_Scenario2 --rate=11 --seed=1" | tee -a scenario2_runs.txt
```


6. **Measure** per-trial and average application throughput.
7. **Visualize** with NetAnim: enable `AnimationInterface` and capture XML.

**Likely issues:**

* Mode-name typo: see [3.1 in common/troubleshooting.md](../common/troubleshooting.md#31-mode-name-typo).
* No routing in infra mode: install `InternetStackHelper` on all nodes.

---

### Task (C++): Scenario 2 – Two Triangles

1. **Copy** `code/Lab2_Cpp_Scenario2.cc` to `scratch/`.
2. **Configure** four STA nodes and one AP in two equilateral triangles.
3. **Install** two OnOff applications (ports 9 & 10) and sinks.
4. **Run** for each PHY rate and three seeds.
5. **Measure** per-flow and aggregate throughput.
6. **Visualize** with NetAnim.

**Likely issues:**

* FlowMonitor missing second flow: verify distinct port numbers.

---

## Part 2: Packet Size & Hidden-Terminal

![Two-Ray Ground Reflection Model](/common/images/hiddenTerminalScenario.png)  
*Refer to jayasuriya2004-hidden.pdf*

---

## Part 3: Smart-Building WiFi Roaming

In a smart building, IoT sensors and mobile devices roam between access points as they move through the building. This part models a mobile client moving between two APs and captures the throughput variations during roaming.

### Topology

```
Server ──CSMA── AP0(0,0) ────── AP1(50,0) ──CSMA── (backbone)
                  │                  │
              WiFi BSS0          WiFi BSS1   (same SSID: SmartBuilding)
                                    ↑
                        STA moving from x=-20 to x=70
```

### Task (C++)

1. **Copy** `code/Lab2_Cpp_Roaming.cc` to `scratch/`.
2. **Build** and run the default roaming scenario:

   ```bash
   ./ns3 run "scratch/Lab2_Cpp_Roaming --speed=5 --simDuration=25 --seed=1"
   ```

3. **Collect** `roaming_throughput.csv` (columns: `time_s`, `throughput_bps`).
4. **Repeat** for speeds {2, 5, 10} m/s.
5. **Plot** throughput vs time for each speed; mark the approximate handoff point.

### Analysis questions

- At what time (approximately) does the handoff from AP0 to AP1 occur?
- How does the STA speed affect the throughput dip during roaming?
- Compare throughput before and after handoff.

---

## Deliverables

See [`deliverables.md`](deliverables.md).

---

## Cross-References

* Setup instructions: [common/setup.md](../common/setup.md)
* Troubleshooting guide: [common/troubleshooting.md](../common/troubleshooting.md)
* API & tutorial links: [common/links.md](../common/links.md)

---
