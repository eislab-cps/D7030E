# Lab 04 — Private LTE for Industrial Automation

One eNodeB, one UE and an EPC core: how downlink throughput depends on antenna
type, offered data rate and distance, and what happens when the UE moves.

- **Required work:** [docs/Lab-04-Instructions.md](docs/Lab-04-Instructions.md)
- **Hand in:** [docs/deliverables.md](docs/deliverables.md)

## Prerequisites

Lab 00 completed. Environment: [docs/environment.md](../docs/environment.md).

## Files

| Path | What it is |
|---|---|
| `code/Lab4_Cpp_LTE.cc` | LTE/EPC downlink scenario, all parts of this lab |
| [`docs/Lab4_ns-3_LTE.pdf`](docs/Lab4_ns-3_LTE.pdf) | Lab handout |
| [`docs/background.md`](docs/background.md) | What the scenario models and which trace to use (background reading) |
| `submission/` | Put your deliverables here |

## Running

From the repository root, with `$NS3_DIR` set:

```bash
cp Lab-04-LTE/code/Lab4_Cpp_LTE.cc "$NS3_DIR/scratch/"
cd "$NS3_DIR"
./ns3 build
./ns3 run "scratch/Lab4_Cpp_LTE --dataRate=10Mbps --distance=100 --antenna=isotropic"
./ns3 run "scratch/Lab4_Cpp_LTE --mobility=true --speed=10 --distance=300"
```

| Argument | Meaning |
|---|---|
| `--dataRate` | Offered downlink rate, e.g. `5Mbps`, `10Mbps`, `20Mbps` |
| `--distance` | UE distance from the eNodeB in metres |
| `--antenna` | `isotropic`, `cosine` or `parabolic` |
| `--enbOrient`, `--ueOrient` | Antenna orientation in degrees (ignored for isotropic) |
| `--mobility`, `--speed` | Move the UE toward the eNodeB at this speed in m/s |
| `--seed` | RNG run number |
| `--csv` | Also write the summary to this path (a header line and a data line are appended on every run) |
| `--enableAnim` | Write the NetAnim XML (off by default) |

Use `--antenna=isotropic` for the distance experiments.

## Outputs

Throughput is printed to stdout. Files are written to
`$NS3_DIR/scratch/Lab4outputs/`:

| File | Contents |
|---|---|
| `DlPdcpStats.txt`, `UlPdcpStats.txt` | PDCP-layer traces |
| `DlRlcStats.txt`, `UlRlcStats.txt` | RLC-layer traces |
| `server_trace-*.pcap` | Packet capture on the server link |
| `ue_mobile_throughput.csv` | Per-second throughput, written when `--mobility=true` and `--csv` is not given |
| `Lab4_LTE.xml` | NetAnim trace, written only with `--enableAnim` |

Copy what you need into `submission/` under the names in
[docs/deliverables.md](docs/deliverables.md). Where a generated file name differs
from the required one, ask a supervisor rather than guessing.

Problems: [docs/troubleshooting.md](../docs/troubleshooting.md).
