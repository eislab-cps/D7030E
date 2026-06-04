# Advanced Wireless Networks – ns-3 Lab Series (2026 Edition)

> **Course code:** D7030E  
> **University:** Luleå University of Technology  
> **Languages:** C++ **or** Python (choose one per lab)  
> **ns-3 version:** 3.47

This repository contains the laboratory exercises for **D7030E Advanced Wireless
Networks**.  The labs use [ns-3](https://www.nsnam.org/) (version 3.47) to
simulate realistic wireless scenarios drawn from modern industrial and research
contexts: industrial propagation environments, smart-building WiFi deployment,
UAV mesh networks, and private LTE for industrial automation.

Each lab can be completed in **either C++ or Python**. You only need to **choose one language path per lab (not both).**
Please submit a PDF report to Canvas, and keep the output/files ready for Lab sessions.

---

## Repository Structure

```
advanceWirelessNetworks/
├─ README.md                       # Start here (course overview, setup, FAQs)
├─ Makefile                        # Convenience targets: docker-build, shell, check, lab0, etc.
├─ Dockerfile                      # Pinned ns-3.47 image build (Ubuntu 22.04 + cppyy bindings)
├─ scripts/
│  ├─ setup_env.sh                 # Exports NS3_DIR, PYTHONPATH, LD_LIBRARY_PATH; sanity import check
│  └─ ci_smoke.sh                  # Quick end-to-end smoke test inside container (tutorial echo + cppyy)
│  # (add your own helpers here if needed)
│
├─ Lab-00-Introduction/
│  ├─ README.md                    # Lab-specific walkthrough & deliverables summary
│  ├─ docs/
│  │  ├─ Lab-00-Instructions.pdf
│  │  └─ deliverables.md           # Exact filenames expected
│  ├─ code/
│  │  ├─ Lab0_Py_Hello.py          # Python hello (cppyy-native; no pybindgen imports)
│  │  ├─ Lab0_Cpp_Hello.cc         # C++ hello
│  │  └─ Lab0_Cpp_AnimRich.cc      # NetAnim demo
│  └─ submission/                  # You create this; place outputs here (txt, xml, png)
│     └─ .gitkeep                  # (optional) keep folder in git
│
├─ Lab-01-Propagation/
│  ├─ README.md
│  ├─ docs/
│  │  ├─ Lab-01-Instructions.pdf
│  │  └─ deliverables.md
│  ├─ code/                        # Friis / Two-Ray / COST231 (C++ and/or Python starters)
│  └─ submission/                  # CSVs + plots comparing models; measured data if required
│
├─ Lab-02-WiFiPerformance/
│  ├─ README.md
│  ├─ docs/
│  │  ├─ Lab-02-Instructions.pdf
│  │  └─ deliverables.md
│  ├─ code/                        # Scenario1/Scenario2 (rate sweep, payload sweep, hidden terminals)
│  └─ submission/                  # scenario1_results.csv, payload_sweep_results.csv, plots, anims
│
├─ Lab-03-Adhoc/
│  ├─ README.md
│  ├─ docs/
│  │  ├─ Lab-03-Instructions.pdf
│  │  └─ deliverables.md
│  ├─ code/                        # Multi-hop UDP/TCP, payload sweep, hidden terminal variants
│  └─ submission/
│
├─ Lab-04-LTE/
│  ├─ README.md
│  ├─ docs/
│  │  ├─ Lab-04-Instructions.pdf
│  │  └─ deliverables.md
│  ├─ code/                        # LTE/EPC downlink scenario (C++ and/or Python)
│  └─ submission/                  # RLC/PDCP traces + throughput-vs-rate/distance CSVs & plots
│
├─ .devcontainer/                  # (optional) VS Code devcontainer
│  └─ devcontainer.json            # Reopen in Container → builds/uses the Docker image
└─ .vscode/                        # (optional) editor settings/tasks
   └─ settings.json
````

Each `Lab-XX` folder contains:
- `docs/` → instructions PDF + `deliverables.md`
- `code/` → starter programs (both C++ and Python)
- You will create a `submission/` subfolder with your results.

---

## Environment Setup

### Option A: Docker / VS Code Devcontainer (recommended)

The repo ships with a Dockerfile pinned to **ns-3.47**.

```bash
make docker-build   # Build the ns-3.47 image
make shell          # Open a shell inside the container
source scripts/setup_env.sh  # Set ns-3 environment
````
### Option B: Native Installation

1. Install dependencies (Ubuntu):

   ```bash
   sudo apt-get update && sudo apt-get install -y \
     build-essential cmake g++ python3 python3-dev python3-pip \
     qtbase5-dev qttools5-dev-tools \
     libxml2 libxml2-dev git pkg-config \
     python3-matplotlib gnuplot-x11 wireshark
   ```
2. Download & build ns-3.47:

   ```bash
   wget https://www.nsnam.org/releases/ns-allinone-3.47.tar.bz2
   tar xjf ns-allinone-3.47.tar.bz2
   cd ns-allinone-3.47
   ./build.py --enable-examples --enable-tests --enable-python-bindings
   ```
3. Set env vars (add to `.bashrc`):

   ```bash
   export NS3_DIR=$HOME/ns-allinone-3.47/ns-3.47
   export PYTHONPATH=$NS3_DIR/build/bindings/python:$PYTHONPATH
   export LD_LIBRARY_PATH=$NS3_DIR/build/lib:$LD_LIBRARY_PATH
   export PATH=$NS3_DIR:$NS3_DIR/build:$PATH
   ```

Verify:

```bash
$NS3_DIR/build/src/core/examples/hello-simulator
python3 -c "from ns import ns; print(ns.core.Simulator.Now())"
```

---

## Running Labs

* **C++:** Copy the lab’s `.cc` file into `ns-3.47/scratch/`, then:

  ```bash
  cd $NS3_DIR
  ./ns3 build
  ./ns3 run scratch/Lab1_Cpp_Friis --distance=100
  ```
* **Python:** Run directly:

  ```bash
  python3 Lab-01-Propagation/code/Lab1_Py_Friis.py --distance=100
  ```

Each lab requires multiple runs (e.g., sweeping distance, payload size, data rate, seeds). Use command-line arguments (`--rate`, `--payload`, `--seed`, etc.) as specified in the lab instructions.

Outputs:

* **Console logs** → redirect to `.txt`
* **FlowMonitor CSVs** → throughput, PDR results
* **Plots (PNG)** → use `plot_helper.py` or external tools
* **NetAnim XML** → open in NetAnim GUI, capture screenshot

---
## ⚠️ For Windows Users

[Alternate installation instructions](/common/windows.md)

### Running Advanced Wireless Networks Labs on Windows

This repo can be run on Windows using **WSL2** + **Docker Desktop**.  

---

### 1. Install prerequisites

Run **PowerShell as Administrator** and enable WSL + VM Platform:

```powershell
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
````

Reboot when prompted.

### Install WSL2 with Ubuntu

```powershell
wsl --install -d Ubuntu
```

Verify:

```powershell
wsl -l -v
```

Expected:

```
  NAME      STATE   VERSION
* Ubuntu    Running 2
```

If it shows `VERSION 1`, upgrade:

```powershell
wsl --set-version Ubuntu 2
```

### Update WSL

```powershell
wsl --update
wsl --shutdown
```

### Install Docker Desktop

1. Download: [https://www.docker.com/products/docker-desktop](https://www.docker.com/products/docker-desktop)
2. During install: ✅ “Use the WSL 2 based engine”.

---

### 2. Configure Docker Desktop

Open **Docker Desktop → Settings**:

* **General**: ✅ *Use the WSL 2 based engine*
* **Resources → WSL Integration**:

  * ✅ *Enable integration with my default WSL distro*
  * ✅ toggle **Ubuntu**

Click **Apply & Restart**.

> Note: `wsl -l -v` will list `docker-desktop` and `docker-desktop-data`. These distros are internal to Docker Desktop and may show as `Stopped`. This is normal — you never start them yourself.

---

### 3. Prepare the repo inside WSL

Do **not** build from `/mnt/c/...` (slow, CRLF issues). Copy to your Linux home:

```bash
# inside Ubuntu WSL shell
mkdir -p ~/work
cp -r /mnt/c/Users/<YourWindowsUser>/Downloads/advanceWirelessNetworks ~/work/
cd ~/work/advanceWirelessNetworks
```

---

### 4. Install tools inside WSL

```bash
sudo apt update
sudo apt install -y make git dos2unix
```

---

### 5. Workflow (same as Linux/macOS)

All commands are run inside the **Ubuntu WSL shell**:

```bash
# Build the ns-3.47 Docker image
make docker-build

# Quick smoke test
make check

# Enter the dev container
make shell

# Inside the container shell:
source scripts/setup_env.sh

# Example: run Lab 1
cp Lab-01-Propagation/code/Lab1_Cpp_Cost231.cc $NS3_DIR/scratch/
cd $NS3_DIR
./ns3 build
./ns3 run "scratch/Lab1_Cpp_Cost231 --distance=50"
```

---

### 6. GUI tools (NetAnim)

WSL has no X server by default. To run GUI apps:

* Install [VcXsrv](https://sourceforge.net/projects/vcxsrv/) or [X410](https://x410.dev/) on Windows.
* In WSL:

  ```bash
  export DISPLAY=:0
  ```
* Then run NetAnim inside the container.

Alternative: run NetAnim from a native Windows ns-3 install.

---

### 7. Troubleshooting

### `docker: command not found` inside Ubuntu

* Docker Desktop isn’t integrated with WSL.
* Open Docker Desktop → Settings → Resources → WSL Integration → toggle **Ubuntu** ON → Apply & Restart.
* Then in PowerShell:

  ```powershell
  wsl --shutdown
  ```

  Restart Docker Desktop, reopen Ubuntu.

### `wsl -l -v` shows `docker-desktop` as `Stopped`

* Normal. That distro is managed by Docker Desktop internally.

---

### 8. Sanity check

Inside Ubuntu WSL:

```bash
docker --version
docker run hello-world
```

Then from repo:

```bash
make docker-build
make check
```

Expected: the `=== ns-3.47 smoke test ===` output with all 3 steps passing.

---

### 9. Summary

* Use **WSL2 + Docker Desktop**.
* Keep the repo in your Linux home, not `/mnt/c`.
* Enforce **LF line endings**.
* Workflow is the same as Linux/macOS (`make docker-build → make check → make shell → source scripts/setup_env.sh`).
* GUI tools need an X server (VcXsrv/X410).
* Don’t install `docker.io` inside Ubuntu — use Docker Desktop integration.

👉 **Tip:** Always save your outputs into `Lab-XX-.../submission/` under `/work` in the container. That way they appear in your cloned repo on Windows.

---

## Lab Overview
| Lab                            | Theme                                      | Key Tasks                                                              |
| ------------------------------ | ------------------------------------------ | ---------------------------------------------------------------------- |
| **Lab 00** – Introduction      | Install check, Hello world, NetAnim basics | Run hello sim (C++/Py), generate XML, screenshot                       |
| **Lab 01** – Propagation       | Friis vs Two-Ray vs COST231 models         | Throughput vs distance, compare with measured path-loss                |
| **Lab 02** – Wi-Fi Performance | Infrastructure Wi-Fi                       | Rate sweep, payload sweep, hidden terminal (RTS/CTS)                   |
| **Lab 03** – Ad hoc            | Multi-hop Wi-Fi                            | UDP chain vs hops, payload sweep, TCP vs UDP, hidden terminal          |
| **Lab 04** – LTE               | Cellular downlink                          | Throughput vs offered rate, throughput vs distance, LTE trace analysis |

See each lab’s `deliverables.md` for exact filenames and submission requirements.

---

## Submission Guidelines

* Place all deliverables inside `Lab-XX-.../submission/`.
* **Must include** `choice.txt` with exactly one line: `C++` or `Python`.
* **File names must match** exactly what’s listed in `deliverables.md`.
* **CSV files:** include header rows.
* **Plots:** PNG format, axis labels, units, legends required.
* **NetAnim:** submit both `.xml` and a screenshot `.png`.

---

## Troubleshooting (Quick)

* **`ImportError: No module named ns.core`**
  → PYTHONPATH not set. Run `source scripts/setup_env.sh`.

* **Wi-Fi mode error (`no matching DataMode`)**
  → Use correct string: `DsssRate5_5Mbps`.

* **RTS/CTS toggle has no effect**
  → Set threshold *before* installing devices:

  ```cpp
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("0"));
  ```

* **FlowMonitor shows zero throughput**
  → Must install FlowMonitor before starting applications.

* **Multi-hop results all zero**
  → You forgot to enable routing (e.g., OLSR).

For more, see [common/troubleshooting.md](common/troubleshooting.md).

---

## Support

* Check lab markdowns and deliverables first.
* Consult `common/troubleshooting.md`.
* Post questions on the course discussion forum.
* Contact a TA if stuck (include lab, command, and error output).

---

Happy simulating! 🚀
