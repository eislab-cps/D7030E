# Lab 02 — Smart-Building Wi-Fi Performance

Infrastructure 802.11b: how PHY rate, payload size and hidden terminals affect
throughput, and how a station roams between two access points.

- **Required work:** [docs/Lab-02-Instructions.md](docs/Lab-02-Instructions.md)
- **Hand in:** [docs/deliverables.md](docs/deliverables.md)

## Prerequisites

Lab 00 completed. Environment: [docs/environment.md](../docs/environment.md).

## Files

| Path | What the program sets up |
|---|---|
| `code/Lab2_Cpp_Scenario1.cc` | One AP, one sender, one receiver (equilateral triangle, 10 m) |
| `code/Lab2_Cpp_Scenario2.cc` | One AP, four STAs in two triangles, two parallel UDP flows |
| `code/Lab2_Cpp_Roaming.cc` | Two bridged APs on a CSMA backbone, one moving STA |
| [`docs/Lab2_ns-3_wifi-1.pdf`](docs/Lab2_ns-3_wifi-1.pdf), [`docs/jayasuriya2004-hidden.pdf`](docs/jayasuriya2004-hidden.pdf) | Lab handout and hidden-terminal paper |
| [`docs/background.md`](docs/background.md) | What each scenario measures and why (background reading) |
| `submission/` | Put your deliverables here |

## Running

From the repository root, with `$NS3_DIR` set:

```bash
cp Lab-02-WiFiPerformance/code/Lab2_Cpp_*.cc "$NS3_DIR/scratch/"
cd "$NS3_DIR"
./ns3 build
./ns3 run "scratch/Lab2_Cpp_Scenario1 --rate=11 --seed=1"
./ns3 run "scratch/Lab2_Cpp_Scenario2 --rate=11 --seed=1"
./ns3 run "scratch/Lab2_Cpp_Roaming --speed=5 --simDuration=25 --seed=1"
```

| Argument | Programs | Meaning |
|---|---|---|
| `--rate` | Scenario1, Scenario2 | 802.11b PHY rate in Mbps (1, 2, 5.5, 11) |
| `--seed` | all | RNG run number |
| `--speed` | Roaming | STA velocity in m/s |
| `--simDuration`, `--logInterval` | Roaming | Total simulated seconds; CSV sampling period |
| `--enableAnim` | Roaming | Write the NetAnim XML (off by default) |

## Outputs

Throughput values are printed to stdout. Files land in
`$NS3_DIR/scratch/Lab2outputs/`: `scenario1_anim.xml`, `scenario2_anim.xml`,
`roaming_throughput.csv` (`time_s,throughput_bps`) and, with `--enableAnim`,
`roaming_anim.xml`. Copy what you need into `submission/` under the names in
[docs/deliverables.md](docs/deliverables.md).

> The payload sweep and hidden-terminal experiments listed in `deliverables.md`
> have no matching argument in these three programs. Ask a supervisor how to run
> them before you start that part.

Problems: [docs/troubleshooting.md](../docs/troubleshooting.md).
