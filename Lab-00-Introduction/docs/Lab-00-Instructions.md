# Lab 00: Introduction to ns-3

**Language:** C++  
**ns-3 version:** 3.47

---

## Objectives

By the end of this lab you will be able to:

1. Install and build ns-3 (C++ mode).  
2. Run a minimal "Hello, ns-3!" simulation in C++.  
3. Generate a NetAnim XML and visualize packet exchanges.  
4. Consult shared setup, troubleshooting, and API links.

---

## Prerequisites & Setup

See [docs/environment.md](../../docs/environment.md) for detailed installation and build instructions.

---

## Part 1: "Hello, Simulator!" in C++

**Task (C++):** Compile and run a minimal C++ ns-3 program.

1. **Copy** the starter code from `code/Lab0_Cpp_Hello.cc` into the ns-3 examples folder:
   ```bash
   cp code/Lab0_Cpp_Hello.cc \
     "$NS3_DIR"/examples/tutorial/hello-simulator-0.cc
    ```

2. **Rebuild** ns-3:

   ```bash
   cd "$NS3_DIR"
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

* **Missing `cmake`:** see [docs/troubleshooting.md](../../docs/troubleshooting.md#setup-and-build).
* **Binary not found:** use the `find` command above to locate.

---

## Part 2: NetAnim Visualization

**Task:** Generate and view an animation of your simulation.

1. **Enable** NetAnim in your C++ code:

   ```cpp
   #include "ns3/netanim-module.h"
   // after creating nodes:
   AnimationInterface anim("lab0-anim.xml");
   ```
2. **Rebuild** and **run** as in Part 1. You'll now get `lab0-anim.xml`.
3. **Launch** NetAnim:

   ```bash
   netanim "$NS3_DIR"/lab0-anim.xml
   ```

**Likely Issue:**

* **XML not generated:** see [docs/troubleshooting.md](../../docs/troubleshooting.md#output-files).

---

## Deliverables

See [`deliverables.md`](deliverables.md).

---

## Cross-References

* Shared setup: [docs/environment.md](../../docs/environment.md)
* Troubleshooting: [docs/troubleshooting.md](../../docs/troubleshooting.md)
* API & tutorial links: [docs/references.md](../../docs/references.md)

---
