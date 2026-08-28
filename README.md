# D7030E Advanced Wireless Networks — ns-3 Labs (2026)

Luleå University of Technology · ns-3 **3.47** · C++

This repository holds the five lab exercises for the course. Each lab is a folder
containing starter C++ code, its instructions, and its deliverables list.

## Start here

1. Prepare your environment — [docs/environment.md](docs/environment.md).
2. Do **Lab 00** first; it verifies your setup. Then Labs 01, 02, 03, 04 in order.
3. In each lab, read the lab's `README.md`, then follow its instructions document.

## Labs

| Lab | Topic | Instructions (tasks) | Deliverables |
|---|---|---|---|
| [Lab 00](Lab-00-Introduction/README.md) | Introduction to ns-3 and NetAnim | [Lab-00-Instructions.md](Lab-00-Introduction/docs/Lab-00-Instructions.md) | [deliverables.md](Lab-00-Introduction/docs/deliverables.md) |
| [Lab 01](Lab-01-Propagation/README.md) | Propagation models: Friis, Two-Ray, COST231-Hata, Nakagami | [Lab-01-Instructions.md](Lab-01-Propagation/docs/Lab-01-Instructions.md) | [deliverables.md](Lab-01-Propagation/docs/deliverables.md) |
| [Lab 02](Lab-02-WiFiPerformance/README.md) | Infrastructure Wi-Fi: rate, payload, hidden terminal, roaming | [Lab-02-Instructions.md](Lab-02-WiFiPerformance/docs/Lab-02-Instructions.md) | [deliverables.md](Lab-02-WiFiPerformance/docs/deliverables.md) |
| [Lab 03](Lab-03-Adhoc/README.md) | Ad-hoc multi-hop: hops, payload, TCP vs UDP, OLSR vs AODV | [Lab-03-Instructions.md](Lab-03-Adhoc/docs/Lab-03-Instructions.md) | [deliverables.md](Lab-03-Adhoc/docs/deliverables.md) |
| [Lab 04](Lab-04-LTE/README.md) | LTE/EPC downlink: antennas, data rate, distance, mobile UE | [Lab-04-Instructions.md](Lab-04-LTE/docs/Lab-04-Instructions.md) | [deliverables.md](Lab-04-LTE/docs/deliverables.md) |

For each lab, the instructions document defines the **required work** and
`deliverables.md` defines the **files you must hand in**. Nothing else in this
repository overrides them.

## Support documents

| Document | Use it for |
|---|---|
| [docs/environment.md](docs/environment.md) | Installing and running ns-3.47, NetAnim, Docker |
| [docs/troubleshooting.md](docs/troubleshooting.md) | Errors, empty results, missing output files |
| [docs/references.md](docs/references.md) | ns-3 API, tutorials, standards and reading links |
| [docs/simulation-background.md](docs/simulation-background.md) | Optional: how discrete-event simulation and RNG seeds work |

## Submitting

Deliverable file names must match `deliverables.md` exactly. CSV files need a
header row; plots need labelled axes and a legend. Bring your outputs to the
lab session. Canvas is the course channel for deadlines, submission format and
any corrections to these instructions — if Canvas and this repository disagree,
Canvas applies.

## Getting help

Check the lab's instructions, then [docs/troubleshooting.md](docs/troubleshooting.md).
If that is not enough, ask a supervisor and include the lab, the exact command
you ran, and the full error output.
