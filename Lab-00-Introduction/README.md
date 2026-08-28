# Lab 00 — Introduction to ns-3 and NetAnim

Warm-up lab: confirm your ns-3.47 environment works, run a C++ simulation,
capture its output, and view a NetAnim animation.

- **Required work:** [docs/Lab-00-Instructions.md](docs/Lab-00-Instructions.md)
- **Hand in:** [docs/deliverables.md](docs/deliverables.md)

## Prerequisites

A working ns-3.47 environment and NetAnim — see
[docs/environment.md](../docs/environment.md). No earlier lab is required.

## Files

| Path | What it is |
|---|---|
| `code/Lab0_Cpp_Hello.cc` | Minimal "Hello Simulator" program |
| `code/Lab0_Cpp_Anim.cc` | Two-node program that writes a NetAnim XML |
| [`docs/Lab0_Introduction_to_ns-3.pdf`](docs/Lab0_Introduction_to_ns-3.pdf) | Lab handout |
| `submission/` | Put your deliverables here |

## Running

For the "Hello, Simulator!" part, follow Part 1 of the
[instructions](docs/Lab-00-Instructions.md). The output is printed to the
terminal only, so redirect it to a file yourself.

The animation program uses the standard workflow that every later lab also uses.
From the repository root, with `$NS3_DIR` set:

```bash
cp Lab-00-Introduction/code/Lab0_Cpp_Anim.cc "$NS3_DIR/scratch/"
cd "$NS3_DIR"
./ns3 build
./ns3 run scratch/Lab0_Cpp_Anim
```

It takes no arguments and writes
`$NS3_DIR/scratch/Lab0outputs/lab0_cpp_anim.xml`.

## Outputs

Copy the files you need into `submission/` under the names in
[docs/deliverables.md](docs/deliverables.md), open the XML in NetAnim, and take
the required screenshot.

Problems: [docs/troubleshooting.md](../docs/troubleshooting.md).
