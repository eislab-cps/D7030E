# Environment

All labs target **ns-3.47**. Results from other ns-3 versions are not comparable.

## Versions

| Item | Version | Source |
|---|---|---|
| ns-3 | 3.47 | Required by every lab |
| C++ standard | C++23 | ns-3.47 minimum |
| CMake | ≥ 3.20 | ns-3.47 minimum |
| GCC (or Clang) | ≥ 11.0 (Clang ≥ 17) | ns-3.47 minimum |
| Docker image base | Ubuntu 22.04, tag `ns3-3.47:latest` | `Dockerfile`, `Makefile` |
| Verified on | Ubuntu 24.04 (WSL2), GCC 13.3.0, CMake 3.28.3 | Course staff test run |

## Option A — Docker (recommended)

Run from the repository root:

```bash
make docker-build     # once; builds the ns-3.47 image (slow)
make check            # smoke test
make shell            # interactive shell; repo is mounted at /work
```

Then, **inside the container**:

```bash
source scripts/setup_env.sh    # sets NS3_DIR (=/opt/ns-allinone-3.47/ns-3.47)
```

The repository root also contains `run.ps1`, `exec/` and `docker-compose.yml`
from an older Windows workflow. They are unsupported and are not part of any
lab — use the `make` targets above.

Windows users: install WSL2 and Docker Desktop with the WSL2 backend, enable WSL
integration for your Ubuntu distribution, and keep the repository in your Linux
home directory (`~/`), not under `/mnt/c` — building from `/mnt/c` is slow and
introduces CRLF problems. Run all commands from the Ubuntu shell.

## Option B — Native ns-3.47

```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential gcc g++ cmake git pkg-config \
    autoconf automake libxmu-dev p7zip-full \
    qtbase5-dev qttools5-dev-tools \
    gnuplot-x11 wireshark net-tools
```

(On Fedora use the equivalent `dnf` packages; on macOS `brew install cmake qt
wireshark gnuplot`.)

```bash
cd ~
wget https://www.nsnam.org/releases/ns-allinone-3.47.tar.bz2
tar xjf ns-allinone-3.47.tar.bz2
cd ns-allinone-3.47/ns-3.47
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

Set these in your shell (add to `~/.bashrc`):

```bash
export NS3_DIR=$HOME/ns-allinone-3.47/ns-3.47
export LD_LIBRARY_PATH=$NS3_DIR/build/lib:$LD_LIBRARY_PATH
export PATH=$NS3_DIR:$NS3_DIR/build:$PATH
```

Verify: `$NS3_DIR/build/src/core/examples/hello-simulator` prints `Hello Simulator`.

## Running a lab program

Every lab uses the same three steps. Run the first one from the repository
root, with `$NS3_DIR` already set.

```bash
cp Lab-01-Propagation/code/Lab1_Cpp_Friis.cc "$NS3_DIR/scratch/"
cd "$NS3_DIR"
./ns3 build
./ns3 run "scratch/Lab1_Cpp_Friis --distance=100"
```

Quote the whole program-plus-arguments string, as shown, whenever you pass
arguments. Each lab's README lists the arguments that lab uses.

**Where results appear.** The programs write their files under
`$NS3_DIR/scratch/LabNoutputs/` (for example `scratch/Lab1outputs/`), *not* into
this repository. Copy the files you need into the lab's `submission/` folder,
using the names required by that lab's `deliverables.md`. Console output is not
saved automatically — redirect it yourself, e.g. `./ns3 run "..." > out.txt`.

## NetAnim

NetAnim is a separate Qt5 GUI. It is **not** bundled with ns-allinone from
ns-3.45 onwards, and it is not installed in the Docker image — run your
simulation in the container or natively, then open the generated `.xml` with a
NetAnim you installed on your host.

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
