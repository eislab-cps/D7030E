# Setup

All labs have been developed and validated on **ns-3 version 3.47**.

---

## System Requirements

| Requirement | Minimum version |
|---|---|
| C++ standard | C++23 |
| CMake | 3.20 |
| GCC | 11.0 |
| Clang | 17 (alternative to GCC) |

These are the ns-3.47 minimum requirements.  Students compiling on university
systems or WSL should verify their toolchain before starting the labs.  The
recommended setup is to use the pre-built ns-allinone-3.47 binary provided
on the course server, which satisfies all requirements.

---

## Dependencies

### Ubuntu 22.04+

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential gcc g++ \
    autoconf automake libxmu-dev cvs git cmake p7zip-full \
    qtbase5-dev qttools5-dev-tools \
    gnuplot-x11 wireshark net-tools
````

### Fedora 35+

```bash
sudo dnf install -y \
    @development-tools gcc-c++ \
    autoconf automake libXmu-devel cvs git cmake p7zip \
    qt5-qtbase-devel qt5-qmake qt5-qttools-devel \
    gnuplot wireshark net-tools
```

### macOS Sonoma

```bash
brew install cmake qt wireshark gnuplot
```

---

## Windows 10/11 (WSL 2)

1. **Enable WSL & Virtual Machine Platform**
   Open PowerShell as Administrator and run:

   ```powershell
   dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
   dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
   Restart-Computer
   ```

2. **Set WSL2 as default**

   ```powershell
   wsl --set-default-version 2
   ```

3. **Install Ubuntu 22.04 LTS** from Microsoft Store. Launch and create UNIX user.

4. **Install Ubuntu dependencies**
   In the Ubuntu shell:

   ```bash
   sudo apt-get update
   sudo apt-get install -y \
       build-essential gcc g++ \
       autoconf automake libxmu-dev cvs git cmake p7zip-full \
       qtbase5-dev qttools5-dev-tools \
       gnuplot-x11 wireshark net-tools
   ```

5. **Enable GUI support (NetAnim)**

   * Install a Windows X server (e.g. VcXsrv, X410).
   * In Ubuntu:

     ```bash
     echo "export DISPLAY=$(grep -m1 nameserver /etc/resolv.conf | awk '{print $2}'):0" >> ~/.bashrc
     source ~/.bashrc
     ```

---

## Download & Build ns-3.47

1. **Download**

   ```bash
   cd ~
   wget https://www.nsnam.org/releases/ns-allinone-3.47.tar.bz2
   tar xjf ns-allinone-3.47.tar.bz2
   cd ns-allinone-3.47
   ```

2. **Build all components**

   ```bash
   ./build.py \
     --enable-examples \
     --enable-tests \
     --qmake-path /usr/lib/qt5/bin/qmake
   ```

3. **Configure & compile via CMake**

   ```bash
   cd ns-3.47
   cmake -S . -B build \
     -DNS3_BUILD_EXAMPLES=ON \
     -DNS3_BUILD_TESTS=ON
   cmake --build build -j$(nproc)
   ```

4. **Verify C++ build**

   ```bash
   ./build/src/core/examples/hello-simulator
   # should print "Hello Simulator"
   ```

5. **Set environment variables**

   ```bash
   export NS3_DIR=$HOME/ns-allinone-3.47/ns-3.47
   export LD_LIBRARY_PATH=$NS3_DIR/build/lib:$LD_LIBRARY_PATH
   export PATH=$NS3_DIR:$NS3_DIR/build:$PATH
   ```

---

### NetAnim availability

The installed ns-allinone-3.47 build on this server **includes** NetAnim
(`ns-3.47/src/netanim/`).

**Students downloading ns-allinone-3.47 from the official ns-3 website** will
**not** receive NetAnim by default — it is no longer bundled in ns-allinone
releases from ns-3.45 onward.  In that case, obtain NetAnim separately:

- Binary: `netanim-3.109` from the ns-3 App Store (https://apps.nsnam.org).
- Or build from the ns-3 App Store source.

Confirm NetAnim is present before starting any lab that requires animation output.

---
