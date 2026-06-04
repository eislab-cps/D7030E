````markdown
# Lab 02 – Smart-Building Wi-Fi Performance

This lab explores how different factors impact Wi-Fi network performance in smart-building and industrial environments. You will evaluate how PHY data rate, packet size, and hidden terminals affect throughput and reliability, and simulate a mobile client roaming between two access points.

## Learning Goals
- Understand the relationship between PHY data rate and application throughput.
- Measure the effect of packet payload size on throughput.
- Explore the hidden terminal problem and observe how RTS/CTS improves fairness.
- Model WiFi roaming between two APs using a CSMA backbone with bridged APs.
- Gain experience using FlowMonitor, NetAnim, and per-second CSV logging.

## Provided Files
- **docs/**
  - `Lab-02-Instructions.md` – step-by-step instructions.
  - `deliverables.md` – list of required submission files.
- **code/**
  - `Lab2_Cpp_Scenario1.cc` – Scenario 1: one AP, one STA.
  - `Lab2_Cpp_Scenario2.cc` – Scenario 2: payload sweep & hidden terminals.
  - `Lab2_Cpp_Roaming.cc` – Scenario 3: smart-building roaming (two APs, mobile STA).
  - `Lab2_Py_Scenario1.py` – Python equivalent of Scenario 1.
  - `Lab2_Py_Scenario2.py` – Python equivalent of Scenario 2.
  - `Lab2_Py_Roaming.py` – Python equivalent of the roaming scenario.

````
## Running the Code

### Scenario 1 – Rate vs Throughput
- **C++**:
  ```bash
  cp Lab-02-WiFiPerformance/code/Lab2_Cpp_Scenario1.cc ~/ns-allinone-3.47/ns-3.47/scratch/
  cd ~/ns-allinone-3.47/ns-3.47
  ./ns3 build
  ./ns3 run scratch/Lab2_Cpp_Scenario1 --rate=11 --seed=1


* **Python**:

  ```bash
  python3 Lab-02-WiFiPerformance/code/Lab2_Py_Scenario1.py --rate=11 --seed=1
  ```

Repeat for multiple data rates and seeds as specified. Log the results into a CSV.

### Scenario 2 – Payload Sweep & Hidden Terminals

* **C++**:

  ```bash
  ./ns3 run scratch/Lab2_Cpp_Scenario2 --payload=500 --enableRtsCts=false
  ```
* **Python**:

  ```bash
  python3 Lab-02-WiFiPerformance/code/Lab2_Py_Scenario2.py --payload=500 --enableRtsCts=false
  ```

Vary payload sizes and toggle RTS/CTS (`--enableRtsCts=true/false`) to collect all required data.

### Scenario 3 – Smart-Building Roaming

* **C++**:

  ```bash
  ./ns3 run scratch/Lab2_Cpp_Roaming --speed=5 --simDuration=25 --seed=1
  ```
* **Python**:

  ```bash
  python3 Lab-02-WiFiPerformance/code/Lab2_Py_Roaming.py --speed=5 --simDuration=25 --seed=1
  ```

Vary STA speed (2, 5, 10 m/s). Output: `roaming_throughput.csv` with columns `time_s,throughput_bps`.

---

## Data Collection

* **Scenario 1 (Rate Sweep)**:

  * Run at several PHY rates with at least three seeds.
  * Save results in `scenario1_results.csv`.
  * Plot throughput vs PHY rate: `scenario1_plot.png`.
  * Save NetAnim output: `scenario1_anim.xml`, `scenario1_screenshot.png`.

* **Scenario 2 Part 1 (Payload Sweep)**:

  * Test multiple payload sizes at different rates.
  * Save combined results in `payload_sweep_results.csv`.
  * Plot throughput vs payload for each rate: `payload_sweep_plot.png`.

* **Scenario 2 Part 2 (Hidden Terminals)**:

  * Run with RTS/CTS disabled and enabled.
  * Save FlowMonitor results:

    * `hidden_off_results.csv`
    * `hidden_on_results.csv`
  * Plot comparison: `hidden_comparison.png`.
  * Save NetAnim outputs and screenshots:

    * `hidden_off_anim.xml`, `hidden_on_anim.xml`
    * `hidden_off_screenshot.png`, `hidden_on_screenshot.png`

* **Scenario 3 (Roaming)**:

  * Collect `roaming_throughput.csv` for three STA speeds.
  * Plot throughput vs time: `roaming_plot.png`.

---

## Deliverables Checklist

(see `docs/deliverables.md` for details)

* `choice.txt` – language used (`C++` or `Python`).
* Scenario 1: `scenario1_results.csv`, `scenario1_plot.png`, anim XML + screenshot.
* Scenario 2 Part 1: `payload_sweep_results.csv`, `payload_sweep_plot.png`.
* Scenario 2 Part 2: hidden terminal CSVs, plot, anim XMLs, screenshots.
* Scenario 3: `roaming_throughput.csv`, `roaming_plot.png`, `roaming_analysis.txt`.

---

## Common Pitfalls

* **RTS/CTS not applied:** Set `RtsCtsThreshold=0` *before* installing Wi-Fi devices, or the toggle will have no effect.
* **FlowMonitor placement:** Must be installed before starting traffic apps, otherwise throughput will log as zero.
* **Rate string typos:** Use exact Wi-Fi mode names (e.g., `DsssRate5_5Mbps` not `DsssRate5.5Mbps`).
* **Unlabeled plots:** Every figure must have axis labels and a legend, or points will be deducted.
* **Python API:** Use `WIFI_STANDARD_80211b` (not `WIFI_PHY_STANDARD_80211b`) and `YansWifiChannelHelper()` default constructor (not `.Default()`).

---
