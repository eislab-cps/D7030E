## Option A (easiest on Win11): **WSLg = Linux NetAnim GUI on Windows**

Works only on **Windows 11** (or Win10 with WSLg backport).

1. Install WSL Ubuntu:

```powershell
wsl --install -d Ubuntu
```

2. In Ubuntu (WSL) terminal, install Qt build deps:

```bash
sudo apt-get update
sudo apt-get install -y qtbase5-dev qttools5-dev-tools build-essential curl
```

3. Get & build NetAnim:

```bash
mkdir -p ~/netanim && cd ~/netanim
curl -LO https://www.nsnam.org/tools/netanim/netanim-3.108.tar.bz2
tar xjf netanim-3.108.tar.bz2
cd netanim-3.108
qmake NetAnim.pro
make -j"$(nproc)"
```

4. Run it:

```bash
./NetAnim
```

A **native Windows window** pops up (that’s WSLg). Open your XML (e.g., from your repo at `\\wsl$\Ubuntu\home\<you>\...` or wherever you saved it).

**Why this is nice:** no X server setup; it “just works” on Win11.

---

## Option B (Win10/Win11): **WSL + X server**

If you’re on **Windows 10** (no WSLg), install an X server like **VcXsrv** or **Xming** on Windows, then:

1. Start VcXsrv (disable access control or allow your WSL IP).

2. In WSL Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y qtbase5-dev qttools5-dev-tools build-essential curl
export DISPLAY=$(ip route | awk '/default/ {print $3}'):0
# (Optionally: echo "export DISPLAY=..." >> ~/.bashrc)
mkdir -p ~/netanim && cd ~/netanim
curl -LO https://www.nsnam.org/tools/netanim/netanim-3.108.tar.bz2
tar xjf netanim-3.108.tar.bz2
cd netanim-3.108
qmake NetAnim.pro
make -j"$(nproc)"
./NetAnim
```

**If nothing appears:** your DISPLAY or firewall is wrong; fix X server settings (common pitfall).

---

## Option C (fully native on Windows): **Build NetAnim with Qt for Windows**

Use either **MSYS2/MinGW** or **Qt/MSVC**. This avoids WSL entirely.

### C1) MSYS2/MinGW route (straightforward)

1. Install **MSYS2** from msys2.org, then open **“MSYS2 MinGW UCRT64”** shell.

2. Install toolchain & Qt5:

```bash
pacman -Syu --noconfirm
pacman -S --noconfirm mingw-w64-ucrt-x86_64-{qt5-base,qt5-tools,make,gcc} unzip git
```

3. Build:

```bash
mkdir -p ~/netanim && cd ~/netanim
curl -LO https://www.nsnam.org/tools/netanim/netanim-3.108.tar.bz2
tar xjf netanim-3.108.tar.bz2
cd netanim-3.108
qmake NetAnim.pro
mingw32-make -j
```

4. Run:

```bash
./NetAnim.exe
```

### C2) Qt/MSVC route (if you already have Visual Studio + Qt)

* Install **Qt 5.15.x** with **MSVC 2019/2022** kit via the Qt online installer.
* Open the **“x64 Native Tools Command Prompt for VS”**, then the **Qt 5.15.x (MSVC) Command Prompt** or set `qmake` on PATH.
* Build:

  ```bat
  qmake NetAnim.pro
  nmake
  ```
* Run `NetAnim.exe`.

**Pitfalls (native build):**

* Use **Qt5**, not Qt6 (qmake project expects Qt5).
* Make sure you’re in the **right shell** (MinGW vs MSVC) that matches the toolchain you installed.
* If `qmake` not found, your Qt bin dir isn’t on PATH.

---


* Run simulations in **Docker** (or WSL), write the animation XML **into the repo** (Windows can see it):

  ```bash
  # inside the container
  ./ns3 run "scratch/Lab0_Cpp_AnimRich --animFile=/work/Lab-00-Introduction/submission/lab0_cpp_anim_rich.xml"
  ```
* Open that XML from NetAnim (WSLg / X server / native Windows build).

---

* **Windows 11:** Use **WSLg**; build NetAnim in WSL; launch it—no X server needed.
* **Windows 10:** Use WSL + **VcXsrv** (X server) or build NetAnim **natively** with MSYS2/Qt.
* **Do not** try to run NetAnim inside your Docker container; it’s a GUI and we keep the image lean.
* The only file NetAnim needs is the **XML** your sim produced—open it and screenshot.
