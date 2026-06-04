# Troubleshooting & Common Pitfalls

All labs target **ns-3 version 3.40**. If you diverge from that version, results may vary.

---

## 1. Installation & Build Errors

### 1.1 `Error: CMake not found`
- **Cause:** `cmake` is not installed or not in `$PATH`.
- **Fix (Ubuntu):**  
  ```bash
  sudo apt-get install cmake
    ````

* **Fix (Fedora):**

  ```bash
  sudo dnf install cmake
  ```

### 1.2 `Could not find qmake` (NetAnim build)

* **Cause:** Qt development tools missing.
* **Fix (Ubuntu):**

  ```bash
  sudo apt-get install qtbase5-dev qttools5-dev-tools
  ```
* **Alternative:** pass path explicitly:

  ```bash
  ./build.py --qmake-path /usr/lib/qt5/bin/qmake
  ```

### 1.3 `ImportError: No module named ns.core`

* **Cause:** Python bindings not on `PYTHONPATH`.
* **Fix:**

  ```bash
  export PYTHONPATH=~/ns-allinone-3.40/ns-3.40/build/bindings:$PYTHONPATH
  ```

---

## 2. Example & Simulation Errors

### 2.1 Missing example binary

```bash
./build/src/applications/examples/hello-simulator: No such file or directory
```

* **Cause:** example lives under a different module (e.g. `core`).
* **Fix:** locate with:

  ```bash
  find build/src -type f -executable | grep hello-simulator
  ```

  then run the correct path, e.g.

  ```bash
  ./build/src/core/examples/hello-simulator
  ```

### 2.2 Segmentation fault in `Simulator::Run()`

* **Cause:** Incorrect ordering of Simulator calls or duplicate calls.
* **Fix:** ensure you call in this order:

  ```cpp
  Simulator::Stop(Seconds(t));
  Simulator::Run();
  Simulator::Destroy();
  ```

  and do not invoke `Run()` twice.

### 2.3 Zero throughput in FlowMonitor

* **Cause:** FlowMonitor installed after applications start or mismatch in IP/port.
* **Fix:**

  1. Install FlowMonitor *before* any application starts.
  2. Verify your classifier’s five-tuple (source IP, destination IP, ports, protocol).

---

## 3. WiFi & Propagation Issues

### 3.1 Mode name typo

```bash
error: no matching DataMode 'DsssRate5.5Mbps'
```

* **Cause:** incorrect PHY mode string.
* **Fix:** use exact Doxygen string, e.g.:

  * `DsssRate1Mbps`
  * `DsssRate5_5Mbps`
  * `DsssRate11Mbps`

### 3.2 RTS/CTS has no effect

* **Cause:** RtsCtsThreshold set after device installation or nodes still in range.
* **Fix:**

  1. Set threshold *before* `wifi.Install(...)`:

     ```cpp
     Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("0"));
     ```
  2. Increase node separation or lower Tx power to enforce hidden-terminal conditions.

---

## 4. NetAnim & Visualization

### 4.1 XML not generated

* **Cause:** missing NetAnim include or forgot to keep `AnimationInterface` alive.
* **Fix (C++):**

  ```cpp
  #include "ns3/netanim-module.h"
  AnimationInterface anim("file.xml");
  // keep `anim` in scope until after Simulator::Destroy()
  ```
* **Fix (Python):**

  ```python
  import ns.netanim
  anim = ns.netanim.AnimationInterface("file.xml")
  ```

### 4.2 GUI not displaying (WSL)

* **Cause:** DISPLAY not set or X server not running.
* **Fix:**

  1. Start X server on Windows (VcXsrv, X410).
  2. In WSL:

     ```bash
     echo "export DISPLAY=$(grep -m1 nameserver /etc/resolv.conf | awk '{print $2}'):0" >> ~/.bashrc
     source ~/.bashrc
     ```

---

## 5. Python Binding Specific

### 5.1 `No module named pybindgen`

* **Cause:** pybindgen not installed or virtualenv inactive.
* **Fix:**

  ```bash
  pip3 install pybindgen
  ```

### 5.2 `TypeError: Schedule() takes 2 arguments but 3 given`

* **Cause:** incorrect usage of `Simulator.Schedule` binding.
* **Fix:** use exactly two args:

  ```python
  Simulator.Schedule(Seconds(1.0), lambda: print("..."))
  ```

---
