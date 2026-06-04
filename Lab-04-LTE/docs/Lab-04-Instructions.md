# Lab 04: LTE for Industrial Automation (Private LTE)

**Language Options:** Complete this lab in **C++ or Python** (choose one).  
**ns-3 version:** 3.47

---

## Scenario Context

| Setting | Model | ns-3 Feature |
|---------|-------|--------------|
| Private LTE factory floor | eNB + UE, EPC, UDP downlink | `LteHelper`, `PointToPointEpcHelper` |
| Antenna coverage planning | Parabolic / cosine / isotropic | `LteHelper::SetEnbAntennaModelType` |
| AMR / AGV vehicle tracking | Mobile UE, velocity model | `ConstantVelocityMobilityModel` |
| Industrial QoS monitoring | Per-second throughput CSV | `PacketSink` callback |

---

## Objectives

By the end of this lab you will:

1. Build an EPC/LTE scenario with eNodeB, UE, PGW, and remote server.  
2. Configure LteHelper attributes (AMC model, scheduler, EARFCN, bandwidth).  
3. Test different antenna configurations (parabolic, cosine, isotropic).  
4. Record and interpret LTE traces (DlRlcStats & DlPdcpStats).  
5. Measure DL throughput vs. application data rate.  
6. Measure DL throughput vs. UE distance (isotropic antenna).  
7. Simulate a mobile UE (factory vehicle) and log per-second throughput.  

---

## Prerequisites & Setup

- Installation & build: [common/setup.md](../common/setup.md)  
- API & tutorials: [common/links.md](../common/links.md)  

---

## Part 1: Scenario Implementation

### Task (C++): LTE Scenario

1. **Copy** starter:
   ```bash
   cp code/Lab4_Cpp_LTE.cc ~/ns-allinone-3.47/ns-3.47/scratch/
    ```

2. **Rebuild**:

   ```bash
   cd ~/ns-allinone-3.47/ns-3.47
   cmake --build build -j$(nproc)
   ```
3. **Configure** `LteHelper`:

   * AMC model: `ns3::LteAmc::PiroEW2010`
   * Pathloss: `TwoRayGroundPropagationLossModel`
   * Scheduler: `PfFfMacScheduler`
   * DL/UL EARFCN: `100` / `18100`
   * DL/UL bandwidth: `50` RBs
4. **Set** antenna type on eNodeB and UE:

   * Parabolic, cosine, isotropic
5. **Install** OnOffHelper (UDP) on remote server → UE.
6. **Enable** tracing:

   ```cpp
   lteHelper->EnablePdcpTraces();
   lteHelper->EnableRlcTraces();
   ```
7. **Run** for three application data rates (e.g. 5, 10, 20 Mbps).
8. **Capture** `DlRlcStats.trace` and `DlPdcpStats.trace` files.

**Likely issues:**

* Missing LTE module include: add `#include "ns3/lte-module.h"`.
* Trace files not generated: ensure `EnablePdcpTraces()` and `EnableRlcTraces()` are called before `Simulator::Run()`.

---

### Task (Python): LTE Scenario

Repeat Part 1 in Python using `code/Lab4_Py_LTE.py`:

1. **Copy** script to `scratch/`.
2. **Set** `PYTHONPATH`:

   ```bash
   export PYTHONPATH=~/ns-allinone-3.47/ns-3.47/build/bindings/python:$PYTHONPATH
   ```
3. **Run** for the same three data rates.
4. **Collect** the same trace files (`.trace`).

---

## Part 2: Throughput Analysis

### Task: Throughput vs. Data Rate

1. **Parse** PDCP trace (`DlPdcpStats.trace`): extract total bytes delivered over time interval.
2. **Compute** throughput = bytes × 8 / duration (bps) for each data rate.
3. **Plot** DL throughput vs. application data rate.

### Task: Throughput vs. Distance (Isotropic Antenna)

1. **Set** antenna to isotropic in code.
2. **Vary** UE distance from eNodeB (e.g. 50 m, 100 m, 150 m, 200 m).
3. **Keep** application data rate fixed (choose one).
4. **Run** and **compute** throughput for each distance.
5. **Plot** DL throughput vs. UE distance.

**Likely issues:**

* Trace parser errors: verify column formats in `.trace` files (timestamp, RNTI, bytes).
* Low throughput at large distances: confirm pathloss and antenna alignment.

---

## Part 3: Mobile UE (Factory Vehicle)

In industrial Private LTE deployments, AGVs (Automated Guided Vehicles) and AMRs (Autonomous Mobile Robots) move continuously within coverage. This part simulates a UE starting at `--distance` m from the eNB and moving toward it at `--speed` m/s.

### Task (C++)

1. **Enable** mobility and set speed:

   ```bash
   ./ns3 run "scratch/Lab4_Cpp_LTE --mobility=true --speed=10 --distance=300 --csv=mobile_results"
   ```

2. **Output**: `ue_mobile_throughput.csv` with columns `time_s,throughput_bps`.

3. **Repeat** for speeds {5, 10, 20} m/s.

4. **Plot** throughput vs. time for each speed; overlay the three curves.

5. **Discuss** in `mobile_analysis.txt`:
   - How does throughput change as the UE approaches the eNB?
   - At what distance does throughput peak?
   - How does UE speed affect throughput variability?

### Task (Python)

Repeat using:

```bash
python3 code/Lab4_Py_LTE.py --mobility=true --speed=10 --distance=300
```

---

## Deliverables

See [`deliverables.md`](deliverables.md).

---

## Cross-References

* Setup: [common/setup.md](../common/setup.md)
* Troubleshooting: [common/troubleshooting.md](../common/troubleshooting.md)
* API & tutorial links: [common/links.md](../common/links.md)
* Trace formats: refer to `doc/tutorial/html/lte-user.html` in the ns-3 docs.

---
