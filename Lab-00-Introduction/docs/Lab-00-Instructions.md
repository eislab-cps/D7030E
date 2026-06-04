# Lab 00: Introduction to ns-3

**Language Options:** Complete this lab in **C++ or Python** (choose one).  
**ns-3 version:** 3.47

---

## Objectives

By the end of this lab you will be able to:

1. Install and build ns-3 (C++ and Python modes).  
2. Run a minimal “Hello, ns-3!” simulation in C++.  
3. Run the equivalent simulation in Python.  
4. Generate a NetAnim XML and visualize packet exchanges.  
5. Consult shared setup, troubleshooting, and API links.

---

## Prerequisites & Setup

See [common/setup.md](../common/setup.md) for detailed installation and build instructions.

---

## Part 1: “Hello, Simulator!” in C++

**Task (C++):** Compile and run a minimal C++ ns-3 program.

1. **Copy** the starter code from `code/Lab0_Cpp_Hello.cc` into the ns-3 examples folder:
   ```bash
   cp code/Lab0_Cpp_Hello.cc \
     ~/ns-allinone-3.47/ns-3.47/examples/tutorial/hello-simulator-0.cc
    ```

2. **Rebuild** ns-3:

   ```bash
   cd ~/ns-allinone-3.47/ns-3.47
   cmake --build build -j$(nproc)
   ```
3. **Locate & run** the new example:

   ```bash
   HELLO=$(find build -type f -executable \
     | grep 'hello-simulator-0$')
   "$HELLO"
   ```

   You should see:

   ```
   Hello Simulator
   ```

**Likely Issues:**

* **Missing `cmake`:** see [1.1 in common/troubleshooting.md](../common/troubleshooting.md#11-error-cmake-not-found).
* **Binary not found:** use the `find` command above to locate.

---

## Part 2: “Hello, Simulator!” in Python

**Task (Python):** Run the Python binding version.

1. **Make sure** your Python bindings are on `PYTHONPATH`:

   ```bash
   export PYTHONPATH=~/ns-allinone-3.47/ns-3.47/build/bindings/python:$PYTHONPATH
   ```
2. **Copy** or open the script:

   ```bash
   cp code/Lab0_Py_Hello.py ~/ns-allinone-3.47/ns-3.47/scratch/
   chmod +x ~/ns-allinone-3.47/ns-3.47/scratch/Lab0_Py_Hello.py
   ```
3. **Run** it:

   ```bash
   python3 ~/ns-allinone-3.47/ns-3.47/scratch/Lab0_Py_Hello.py
   ```

   Expected output:

   ```
   Hello from Python after 1 second
   ```

**Likely Issue:**

* **ImportError:** see [1.3 in common/troubleshooting.md](../common/troubleshooting.md#13-importerror-no-module-named-nscore).

---

## Part 3: NetAnim Visualization

**Task:** Generate and view an animation of your simulation.

1. **Enable** NetAnim in your C++ code:

   ```cpp
   #include "ns3/netanim-module.h"
   // after creating nodes:
   AnimationInterface anim("lab0-anim.xml");
   ```
2. **Rebuild** and **run** as in Part 1. You’ll now get `lab0-anim.xml`.
3. **Launch** NetAnim:

   ```bash
   netanim ~/ns-allinone-3.47/ns-3.47/lab0-anim.xml
   ```

For Python, ensure the script imports and instantiates:

```python
import ns.netanim
anim = ns.netanim.AnimationInterface("lab0-anim.xml")
```

Run as in Part 2, then open the resulting XML in NetAnim.

**Likely Issue:**

* **XML not generated:** see [4.1 in common/troubleshooting.md](../common/troubleshooting.md#41-xml-not-generated).

---

## Deliverables

See [`deliverables.md`](deliverables.md).

---

## Cross-References

* Shared setup: [common/setup.md](../common/setup.md)
* Troubleshooting: [common/troubleshooting.md](../common/troubleshooting.md)
* API & tutorial links: [common/links.md](../common/links.md)

---
