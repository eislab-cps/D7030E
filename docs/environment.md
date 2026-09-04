# Environment

All labs target **ns-3.47**. Results from other ns-3 versions are not comparable.
The supported environment is native ns-3.47 built inside Ubuntu on WSL2 —
there is no container in this repository's workflow.

## Windows quick start (copy-paste into PowerShell)

This section is for a normal PowerShell window (not Administrator, unless
noted). Everything below must be typed into **your own terminal**, not run
through a script for you — installing packages needs to ask for your Linux
password interactively.

**Step 0 — install WSL and Ubuntu.** Skip this if `wsl --list` already shows
`Ubuntu`. Open PowerShell **as Administrator** and run:

```powershell
wsl --install -d Ubuntu
```

Restart Windows if it asks you to. Then open the **Ubuntu** app once from the
Start menu and follow its prompts to create a Linux username and password.
This password is separate from your Windows password — **remember it**, you
will type it again in Step 3.

**Step 1 — open a normal (non-Administrator) PowerShell window in the
repository folder.** In File Explorer, open the folder where you cloned this
repository, then Shift+Right-click an empty area and choose "Open PowerShell
window here" (or "Open in Terminal"). Alternatively:

```powershell
cd D:\path\to\D7030E
```

(Replace `D:\path\to\D7030E` with wherever you actually cloned it.)

**Step 2 — translate that folder path for WSL and save it for the rest of
this session:**

```powershell
$repoWsl = (wsl -d Ubuntu -- wslpath -a ($PWD.Path -replace '\\','/')).Trim()
$repoWsl
```

The second line should print something like `/mnt/d/path/to/D7030E`. If you
close and reopen PowerShell later, `cd` back into the repo folder and re-run
this step before the commands below — `$repoWsl` does not persist across
windows.

**Step 3 — one-time install and build of ns-3.47.** This installs apt
packages (asks for the Linux password from Step 0), downloads
`ns-allinone-3.47`, and compiles it. It takes a while (build alone is
commonly 15–30 minutes) and needs a working internet connection.

```powershell
wsl -d Ubuntu -- bash -lc "cd '$repoWsl' && chmod +x scripts/*.sh && make setup"
```

**Step 4 — verify the install:**

```powershell
wsl -d Ubuntu -- bash -lc "cd '$repoWsl' && make check"
```

You should see `OK: pinned to ns-3.47` followed by a successful build.

**Step 5 — run the Lab 00 starter as a final check:**

```powershell
wsl -d Ubuntu -- bash -lc "cd '$repoWsl' && make lab0"
```

This should print `Hello Simulator`.

### Day-to-day: running a lab program

Easiest option — double-click `run.ps1`, or run it from PowerShell:

```powershell
.\run.ps1
```

It opens a file picker to choose the lab's `.cc` file, asks for arguments
(for example `--distance=100`), then builds and runs it inside WSL.

Or, from the same PowerShell window (after Step 2 above), run a specific
program directly:

```powershell
wsl -d Ubuntu -- bash -lc "cd '$repoWsl' && scripts/run_cpp.sh Lab-01-Propagation/code/Lab1_Cpp_Friis.cc --distance=100"
```

Change the `.cc` path and the arguments after it for whichever lab program
you are running — each lab's `README.md` lists the arguments that lab uses.

**Where results appear.** ns-3 writes output files under
`~/ns-allinone-3.47/ns-3.47/scratch/LabNoutputs/` **inside WSL**, not into
this repository. To copy them into the repo's `submission/` folder (still
from the same PowerShell window):

```powershell
wsl -d Ubuntu -- bash -lc "cp ~/ns-allinone-3.47/ns-3.47/scratch/Lab1outputs/*.csv '$repoWsl/Lab-01-Propagation/submission/'"
```

Adjust the `Lab1outputs` folder name and destination for the lab you ran.
Console output is not saved automatically — `run.ps1` prints it to the
PowerShell window; copy what you need from there, or redirect it yourself
when using `scripts/run_cpp.sh` (add `> out.txt` after the command).

## Versions

| Item | Version | Source |
|---|---|---|
| ns-3 | 3.47 | Required by every lab |
| C++ standard | C++23 | ns-3.47 minimum |
| CMake | ≥ 3.20 | ns-3.47 minimum |
| GCC (or Clang) | ≥ 11.0 (Clang ≥ 17) | ns-3.47 minimum |
| Verified on | Ubuntu 24.04 (WSL2), GCC 13.3.0, CMake 3.28.3 | Course staff test run |

## Working from inside Ubuntu (WSL) directly

If you'd rather work from an Ubuntu terminal instead of PowerShell — open the
**Ubuntu** app, or run `wsl -d Ubuntu` from PowerShell to drop into one — the
same `make` targets apply directly, without the `wsl -d Ubuntu -- bash -lc`
wrapper:

```bash
cd /mnt/d/path/to/D7030E   # or wherever you cloned the repo
scripts/install_wsl.sh     # or: make setup
make check
make lab0
scripts/run_cpp.sh Lab-01-Propagation/code/Lab1_Cpp_Friis.cc --distance=100
```

`scripts/install_wsl.sh` installs the required apt packages, downloads
`ns-allinone-3.47` to `$HOME/ns-allinone-3.47`, and builds it
(`./ns3 configure --enable-examples --enable-tests && ./ns3 build`). It only
needs to run once per machine — the repo itself can stay wherever you cloned
it (including under `/mnt/d/...`); only the ns-3 install and build live under
`$HOME`, so working from `/mnt/*` does not slow down `./ns3 build`.

`scripts/run_cpp.sh` stages the chosen `.cc` in this repository's `exec/`
folder (gitignored except for `exec/CMakeLists.txt`), copies it into
`$NS3_DIR/scratch/exec/`, builds, and runs it as `ns3 run exec -- <args>`.

Equivalent fully-manual workflow, if you want to drive `ns3` yourself:

```bash
source scripts/setup_env.sh
cp Lab-01-Propagation/code/Lab1_Cpp_Friis.cc "$NS3_DIR/scratch/"
cd "$NS3_DIR"
./ns3 build
./ns3 run "scratch/Lab1_Cpp_Friis --distance=100"
```

Quote the whole program-plus-arguments string, as shown, whenever you pass
arguments this way.

## NetAnim

NetAnim is a separate Qt5 GUI. It is **not** bundled with ns-allinone from
ns-3.45 onwards — build it yourself once you have a generated `.xml`:

```bash
sudo apt-get install -y qtbase5-dev qttools5-dev-tools build-essential curl
mkdir -p ~/netanim && cd ~/netanim
curl -LO https://www.nsnam.org/tools/netanim/netanim-3.108.tar.bz2
tar xjf netanim-3.108.tar.bz2 && cd netanim-3.108
qmake NetAnim.pro && make -j"$(nproc)"
./NetAnim
```

A pre-built NetAnim binary is also published on the ns-3 App Store
(<https://apps.nsnam.org>). On Windows 11, WSLg shows the window with no extra
setup; on Windows 10 you need an X server (VcXsrv, X410) and `export DISPLAY=:0`.

Open your XML with **File → Open**, then screenshot it where a lab asks for one.
