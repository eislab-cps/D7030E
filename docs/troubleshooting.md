# Troubleshooting

All labs target ns-3.47. None of the fixes below change an experiment — if a fix
seems to require editing lab parameters, ask a supervisor instead.

## Setup and build

| Symptom | Check | Resolution |
|---|---|---|
| `cmake: command not found` | `cmake --version` | `sudo apt-get install cmake` (Fedora: `sudo dnf install cmake`) |
| `Could not find qmake` when building NetAnim | `qmake --version` | `sudo apt-get install qtbase5-dev qttools5-dev-tools` |
| `./ns3: No such file or directory` | `echo $NS3_DIR`; are you in that directory? | `source scripts/setup_env.sh` (container) or export `NS3_DIR`, then `cd "$NS3_DIR"` |
| `hello-simulator: No such file or directory` | Examples live under `core`, not `applications` | `find build/src -type f -executable \| grep hello-simulator` and run that path |
| `./ns3 run` cannot find your program | Was the `.cc` copied into `$NS3_DIR/scratch/`? Did `./ns3 build` succeed? | Re-copy the file, rebuild, then run `scratch/<ProgramName>` without the `.cc` |
| `docker: command not found` inside WSL Ubuntu | Docker Desktop → Settings → Resources → WSL Integration | Enable your Ubuntu distribution, Apply & Restart, then `wsl --shutdown` in PowerShell |

## Running simulations

| Symptom | Check | Resolution |
|---|---|---|
| Your arguments are ignored | Is the whole string quoted? | `./ns3 run "scratch/Prog --flag=value"` |
| `error: no matching DataMode 'DsssRate5.5Mbps'` | Exact PHY mode spelling | Use `DsssRate1Mbps`, `DsssRate5_5Mbps`, `DsssRate11Mbps` |
| Throughput reported as zero | FlowMonitor must be installed before applications start; check the flow's IP/port five-tuple | Rerun without modifying the scenario; if it stays zero, report it to a supervisor |
| Multi-hop throughput zero beyond one hop | Ad-hoc multi-hop needs a routing protocol; the Lab 03 starters enable OLSR unless you pass `--routing=aodv` | Keep routing enabled. If a chain still reports zero with the arguments your lab asks for, record the command and report it to a supervisor — do not change the scenario to make a number appear |
| Enabling RTS/CTS changes nothing | The threshold must be set before the Wi-Fi devices are installed | Use the program's RTS/CTS argument rather than editing the scenario |
| Segmentation fault in `Simulator::Run()` | `Simulator::Run()` called twice, or ordering wrong | Order must be `Stop()` → `Run()` → `Destroy()` |
| Results differ between runs | Seeds — the lab README says whether that program takes a seed argument | Where one exists, set it and average over the number of runs your lab requires |

## Output files

| Symptom | Check | Resolution |
|---|---|---|
| "Where did my CSV / XML / PCAP go?" | Programs write to `$NS3_DIR/scratch/LabNoutputs/`, not into this repository | `ls "$NS3_DIR"/scratch/Lab*outputs/`, then copy what you need into the lab's `submission/` |
| Console output not saved | It is only printed | Redirect it: `./ns3 run "scratch/Prog --flag=1" > output.txt` |
| NetAnim XML missing or empty | The program only writes it if animation is enabled, and `AnimationInterface` must stay alive until the run ends | Use the program's animation argument where one exists; do not move the `AnimationInterface` out of scope |
| NetAnim window never appears (WSL) | `echo $DISPLAY`, X server running? | Windows 11: use WSLg. Windows 10: start VcXsrv/X410 and `export DISPLAY=:0` |
| A generated file name differs from the one `deliverables.md` requires | Compare the two names | Do not rename silently to make it fit — ask a supervisor which name applies |

## Still stuck

Ask a supervisor. Include: the lab, the exact command you ran, the full error
output, your operating system, and the contents of `$NS3_DIR/VERSION`. Deadlines
and any correction to a lab's requirements are announced on Canvas.
