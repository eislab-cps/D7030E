# Lab 02 Deliverables

## Scenario 1 – Application Throughput (Infrastructure WiFi)

1. **Simulation results**

   * `scenario1_results.csv` — columns:

     ```
     rate_mbps,seed1_bps,seed2_bps,seed3_bps,avg_bps
     ```
   * `scenario1_plot.png` — mean application throughput vs. PHY rate.

2. **Animations & screenshots**

   * `scenario1_anim.xml` — NetAnim trace for one PHY rate run.
   * `scenario1_screenshot.png` — screenshot of NetAnim showing the AP, sender(s), and receiver.

---

## Scenario 2, Part 1 – Payload Sweep

1. **Simulation results**

   * `payload_sweep_results.csv` — columns:

     ```
     rate_mbps,payload_bytes,throughput_bps
     ```

     (include all experiments for all PHY rates × payload sizes).

2. **Plot**

   * `payload_sweep_plot.png` — throughput vs. payload size. One curve per PHY rate.

3. **Throughput table (for 1000B payload)**

   * `throughput_table.txt` — table showing PHY rate vs. measured throughput.

4. **Single packet transmission time**

   * `packet_time_calc.txt` — calculation of transmission time for 400B payload at 11 Mbps and explanation if it differs from 11 Mbps.

---

## Scenario 2, Part 2 – Hidden Terminal

1. **Simulation results**

   * `hidden_off_results.csv` — columns:

     ```
     flow_id,throughput_bps,pdr
     ```

     (RTS/CTS disabled).
   * `hidden_on_results.csv` — same columns (RTS/CTS enabled).

2. **Plots**

   * `hidden_comparison.png` — bar chart or line plot comparing throughput & PDR with and without RTS/CTS.

3. **Animations & screenshots**

   * `hidden_off_anim.xml` — NetAnim trace with RTS/CTS disabled.
   * `hidden_on_anim.xml` — NetAnim trace with RTS/CTS enabled.
   * `hidden_off_screenshot.png` — screenshot of RTS/CTS disabled run.
   * `hidden_on_screenshot.png` — screenshot of RTS/CTS enabled run.

4. **Discussion**

   * `hidden_terminal_discussion.txt` — short explanation of observed differences in throughput and packet delivery ratio between RTS/CTS off and on.

---

## Scenario 3 – Smart-Building WiFi Roaming

1. **Simulation results**

   * `roaming_throughput.csv` — columns:

     ```
     time_s,throughput_bps
     ```

     (collect for at least three STA speeds: 2, 5, 10 m/s).

2. **Plot**

   * `roaming_plot.png` — throughput vs time for each STA speed. Mark the approximate handoff point.

3. **Analysis**

   * `roaming_analysis.txt` — answers to:
     - At what time does handoff from AP0 to AP1 occur for each speed?
     - How does STA speed affect the throughput dip during roaming?
     - Compare average throughput before and after handoff.

---

## File Naming Summary

* `scenario1_results.csv`, `scenario1_plot.png`, `scenario1_anim.xml`, `scenario1_screenshot.png`
* `payload_sweep_results.csv`, `payload_sweep_plot.png`, `throughput_table.txt`, `packet_time_calc.txt`
* `hidden_off_results.csv`, `hidden_on_results.csv`, `hidden_comparison.png`
* `hidden_off_anim.xml`, `hidden_on_anim.xml`, `hidden_off_screenshot.png`, `hidden_on_screenshot.png`
* `hidden_terminal_discussion.txt`
* `roaming_throughput.csv`, `roaming_plot.png`, `roaming_analysis.txt`

---

**All CSV files must include a header row. All plots must have labeled axes and legends. File names must match exactly.**
