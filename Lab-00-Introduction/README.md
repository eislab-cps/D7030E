````markdown
# Lab 00 – Introduction to ns-3 and NetAnim

This is your warm-up lab. The goal is to make sure your ns-3 environment is set up and that you can run a C++ simulation, capture output, and visualize results with NetAnim.

---

## Learning Goals
- Verify that you can compile and run a basic ns-3 simulation.
- Learn how to run C++ code by copying `.cc` files to `scratch/` and using `./ns3 run`.
- Generate a NetAnim XML trace and open it in the NetAnim GUI.
- Prepare submission files in the required format.

---

## Provided Files
- **docs/**
  - `Lab-00-Instructions.md` – step-by-step instructions.
  - `deliverables.md` – the official list of what you must submit.
- **code/**
  - `Lab0_Cpp_Hello.cc` – minimal C++ hello simulation.

````
## Running the Code

### C++ path
1. Copy `Lab0_Cpp_Hello.cc` into your ns-3 `scratch/` directory.
2. Build and run:
   ```bash
   cd ~/ns-allinone-3.47/ns-3.47
   ./ns3 build
   ./ns3 run scratch/Lab0_Cpp_Hello


3. Save the output:

   ```bash
   ./ns3 run scratch/Lab0_Cpp_Hello > Lab-00-Introduction/submission/hello_cpp_output.txt
   ```

---

## NetAnim

The starter code also creates a NetAnim XML trace. To view:

1. After running the sim, locate the generated XML (e.g., `lab0_anim.xml`).
2. Open NetAnim:

   ```bash
   ./NetAnim lab0_anim.xml
   ```
3. Take a screenshot and save as `anim_screenshot_cpp.png` in `submission/`.

---

## Deliverables (checklist)

See `docs/deliverables.md` for details. You must provide:

* Hello simulation output text file.
* NetAnim XML file.
* NetAnim screenshot (PNG).

⚠️ **Important:** File names must match exactly what's listed in `deliverables.md`.

---

## Common Pitfalls

* NetAnim XML empty → means the `AnimationInterface` object was destroyed before simulation ended.
* Wrong filenames in submission → auto-grader will reject them.

---
