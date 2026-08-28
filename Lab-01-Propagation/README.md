# Lab 01 — Propagation Models

Measure how Friis, Two-Ray Ground, COST231-Hata and Nakagami fading change
throughput with distance, and compare simulated with measured path loss.

- **Required work:** [docs/Lab-01-Instructions.md](docs/Lab-01-Instructions.md)
- **Hand in:** [docs/deliverables.md](docs/deliverables.md)

## Prerequisites

Lab 00 completed (working ns-3.47 build). Environment:
[docs/environment.md](../docs/environment.md).

## Files

| Path | What it is |
|---|---|
| `code/Lab1_Cpp_Friis.cc`, `Lab1_Cpp_TwoRay.cc`, `Lab1_Cpp_Cost231.cc`, `Lab1_Cpp_Nakagami.cc` | One two-node Wi-Fi link per propagation model |
| [`docs/Lab1_ns-3_propagation.pdf`](docs/Lab1_ns-3_propagation.pdf) | Lab handout |
| [`docs/background.md`](docs/background.md) | Reference table comparing the four models (background reading) |
| `submission/` | Put your deliverables here |

## Running

From the repository root, with `$NS3_DIR` set:

```bash
cp Lab-01-Propagation/code/Lab1_Cpp_*.cc "$NS3_DIR/scratch/"
cd "$NS3_DIR"
./ns3 build
./ns3 run "scratch/Lab1_Cpp_Friis --distance=100"
```

| Argument | Programs | Meaning |
|---|---|---|
| `--distance` | all four | Node separation in metres |
| `--antHeight` | `Lab1_Cpp_TwoRay` only | Antenna height in metres |

## Outputs

Each run prints one line to stdout:

```
CSV,model=Friis,distance_m=100,rxBytes=...,throughput_bps=...
```

Animation and packet-capture files are written to `$NS3_DIR/scratch/Lab1outputs/`
(`Lab1_<Model>.xml`, `Lab1_<Model>-*.pcap`). Build your result CSVs from the
stdout lines and copy the files you need into `submission/` under the names in
[docs/deliverables.md](docs/deliverables.md).

Problems: [docs/troubleshooting.md](../docs/troubleshooting.md).
