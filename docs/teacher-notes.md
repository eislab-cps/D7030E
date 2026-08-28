# Teacher notes — open issues

For course staff. Not part of the student reading path.

These issues were found while restructuring the lab documentation (2026-08-28).
None of them could be fixed by documentation alone: each needs a change to lab
code, to a task, or to a deliverable, or a decision about which of two existing
statements applies. Nothing was invented to paper over a conflict, and no
protected content was changed.

Severity: **High** — a student may do or submit the wrong work.
**Medium** — the lab is doable but avoidable friction or ambiguity remains.

---

## R-1 · `docker-compose.yml` targets ns-3.40 · High · repository

**Paths:** `docker-compose.yml`, `run.ps1`, `Dockerfile`, `Makefile`,
`docs/archive/windows-runps1-workflow.md`

`docker-compose.yml` uses `image: ns3-3.40:latest` and bind-mounts `./exec` to
`/opt/ns-allinone-3.40/ns-3.40/scratch/exec`. `Dockerfile` builds ns-3.47 and
`make docker-build` tags the image `ns3-3.47:latest` at
`/opt/ns-allinone-3.47/ns-3.47`. So `docker compose up -d` — and therefore
`run.ps1`, the documented Windows workflow — cannot start against the image this
repository builds.

**Effect:** Windows students following that workflow get a missing-image error
before they run anything.

**Interim handling:** the Windows `run.ps1` workflow has been moved to
`docs/archive/`; `docs/environment.md` documents the `Makefile` Docker path and
the native path only.

**Decision needed:** update `docker-compose.yml` (image tag and mount path) to
3.47 and restore the workflow, or retire `run.ps1`/`exec/` and the compose file.

---

## R-2 · Submission channel is stated three ways · Medium · repository

**Paths:** `README.md` (previous version), `Lab-00-Introduction/docs/deliverables.md`,
`.gitignore`

The old root README said "submit a PDF report to Canvas". Lab 00's deliverables
say "Submit a single ZIP archive (or a pull request)". `.gitignore` ignores
`submission/`, so a pull request cannot carry the deliverables at all. The other
labs' deliverables documents name files but no channel.

**Effect:** students cannot tell what to hand in where; the pull-request option
is not workable as the repository is configured.

**Interim handling:** the root README now names Canvas as the course channel for
deadlines and submission format, and leaves file names to `deliverables.md`.

**Decision needed:** one submission channel, stated once, and whether the ZIP /
pull-request wording in Lab 00 should be removed.

---

## L0-1 · Three different procedures for the "Hello" deliverable · Medium · Lab 00

**Paths:** `Lab-00-Introduction/docs/Lab-00-Instructions.md` (Part 1),
`Lab-00-Introduction/docs/deliverables.md` (item 1)

The instructions tell students to copy `Lab0_Cpp_Hello.cc` over
`examples/tutorial/hello-simulator-0.cc`, rebuild with `cmake --build build` and
run the located binary. The deliverables document asks for the captured output of
`./build/src/core/examples/hello-simulator`, which is the stock ns-3 example, not
the lab's file. The rest of the repository uses the `scratch/` + `./ns3 run`
workflow.

**Effect:** all three print `Hello Simulator`, so the submitted file may not show
that the student compiled the provided program.

**Interim handling:** the Lab 00 README no longer adds a fourth variant; it
defers to Part 1 of the instructions.

**Decision needed:** one procedure for Part 1, reflected in both documents.

---

## L0-2 · Lab 00 Part 2 animates a program that has no nodes · High · Lab 00

**Paths:** `Lab-00-Introduction/docs/Lab-00-Instructions.md` (Part 2),
`Lab-00-Introduction/code/Lab0_Cpp_Hello.cc`,
`Lab-00-Introduction/code/Lab0_Cpp_Anim.cc`,
`Lab-00-Introduction/docs/deliverables.md`

Part 2 tells students to add `AnimationInterface anim("lab0-anim.xml")` "after
creating nodes" to the Part 1 program. `Lab0_Cpp_Hello.cc` creates no nodes — it
schedules one `std::cout`. The provided two-node program `Lab0_Cpp_Anim.cc` is
never mentioned in the instructions, and it already writes
`scratch/Lab0outputs/lab0_cpp_anim.xml`, which is the name the deliverables
require; the instructions instead name `lab0-anim.xml` in the ns-3 root.

**Effect:** a student following Part 2 literally animates a topology-free program
and cannot produce the required screenshot of a "two-node Hello simulation".

**Interim handling:** the Lab 00 README documents the provided `Lab0_Cpp_Anim.cc`
run and the file it writes.

**Decision needed:** rewrite Part 2 around `Lab0_Cpp_Anim.cc`, or add node
creation to the Part 1 program.

---

## L1-1 · Lab 01 Part 2 is optional in one document, required in the other · High · Lab 01

**Paths:** `Lab-01-Propagation/docs/Lab-01-Instructions.md` (Part 2 heading:
"Real-World Propagation Measurements (**Optional**)"),
`Lab-01-Propagation/docs/deliverables.md` (Part II, plus five file names in the
naming summary)

**Effect:** a student following the instructions may skip work that the
deliverables list as required (`measured_rssi.csv`, `measured_pathloss.csv`,
`friis_pathloss.csv`, `pathloss_comparison.png`, `discussion.txt`).

**Decision needed:** is Part 2 required or optional? Both documents must say the
same thing.

---

## L1-2 · Lab 01 requires two seeds but no Lab 01 program has a seed argument · High · Lab 01

**Paths:** `Lab-01-Propagation/docs/Lab-01-Instructions.md` (Nakagami task,
"Run **two seeds** per distance point and average the results (same pattern as
the other models)"), `Lab-01-Propagation/code/Lab1_Cpp_*.cc`

`Lab1_Cpp_Friis.cc`, `Lab1_Cpp_Cost231.cc` and `Lab1_Cpp_Nakagami.cc` register
`--distance` only; `Lab1_Cpp_TwoRay.cc` registers `--distance` and
`--antHeight`. None of the four calls `RngSeedManager`. There is no documented
way to vary the seed, so two runs of the stochastic Nakagami scenario return the
same number.

**Effect:** students "average" two identical results and report a variance study
that did not happen.

**Interim handling:** the Lab 01 README lists only the arguments that exist. No
substitute mechanism was invented.

**Decision needed:** add a seed argument to the Lab 01 starters, name a supported
alternative, or drop the two-seed requirement.

---

## L1-3 · "Three models" in the objectives, four in the tasks · Medium · Lab 01

**Paths:** `Lab-01-Propagation/docs/Lab-01-Instructions.md` (objective 1: "Use
three ns-3 propagation-loss models"), same file (four model tasks in Part 1),
`Lab-01-Propagation/docs/deliverables.md` ("Four models are required")

Left unchanged rather than editing the objective, since the objective states the
scope of the lab. Also in the same file: the COST231 and Friis tasks say "Repeat
steps 1–6" although the Two-Ray task they refer to has three steps, and the
Nakagami section reuses the Friis illustration.

**Decision needed:** correct the objective and the step reference.

---

## L1-4 · Two different measurement set-ups for Lab 01 Part 2 · Medium · Lab 01

**Paths:** same two documents

The instructions specify an ad-hoc link between two laptops, an industrial or
corridor environment, RSSI at 1 m, 2 m, … to at least 20 m. The deliverables
specify "a corridor (with at least one corner, as in Fig. 3 of the lab handout)"
and give no distance range.

**Decision needed:** one environment and one distance set, in whichever document
is authoritative.

---

## L2-1 · No starter code for the payload sweep and hidden-terminal experiments · High · Lab 02

**Paths:** `Lab-02-WiFiPerformance/code/`,
`Lab-02-WiFiPerformance/docs/deliverables.md` (Scenario 2 Parts 1 and 2),
`Lab-02-WiFiPerformance/docs/background.md`

`Lab2_Cpp_Scenario1.cc` and `Lab2_Cpp_Scenario2.cc` accept `--rate` and `--seed`
only; `Lab2_Cpp_Roaming.cc` accepts the roaming arguments. No Lab 02 program
accepts a payload size or an RTS/CTS toggle, and none builds the hidden-terminal
topology. The deliverables nevertheless require `payload_sweep_results.csv`,
`hidden_off_results.csv`, `hidden_on_results.csv`, PDR values, two animations and
two screenshots. The previous lab README documented
`./ns3 run scratch/Lab2_Cpp_Scenario2 --payload=500 --enableRtsCts=false`, which
the program rejects.

**Effect:** students cannot produce most of the Lab 02 deliverables from the
provided code. (`Lab3_Cpp_Hidden.cc` does implement a hidden-terminal scenario
with `--enableRtsCts`, but it belongs to Lab 03.)

**Interim handling:** the invalid command line has been removed from the Lab 02
README, which now states that these experiments have no matching argument and
tells students to ask a supervisor.

**Decision needed:** add or point to starter code, or amend the Lab 02
deliverables.

---

## L2-2 · "Scenario 2" names two different scenarios · High · Lab 02

**Paths:** `Lab-02-WiFiPerformance/docs/Lab-02-Instructions.md` (Part 1, "Scenario
2 – Two Triangles"), `.../deliverables.md` ("Scenario 2, Part 1 – Payload Sweep",
"Part 2 – Hidden Terminal"), `.../docs/background.md`,
`code/Lab2_Cpp_Scenario2.cc` (header comment: "Scenario 1 (Part 2): Two
simultaneous flows via one AP")

**Effect:** a student cannot tell which experiment a deliverable belongs to, or
which program produces it.

**Decision needed:** one scenario numbering across code, instructions and
deliverables.

---

## L2-3 · Required number of seeds differs · Medium · Lab 02

**Paths:** instructions ("running three seeds per rate"), deliverables
(`scenario1_results.csv` columns `seed1_bps,seed2_bps,seed3_bps`),
`docs/background.md` ("two different random seeds"),
`code/Lab2_Cpp_Scenario1.cc` (`--seed` help text: "use 1 and 2 for the lab")

**Decision needed:** two seeds or three, stated once.

---

## L2-4 · Lab 02 Part 2 has no task text · Medium · Lab 02

**Path:** `Lab-02-WiFiPerformance/docs/Lab-02-Instructions.md`, "Part 2: Packet
Size & Hidden-Terminal"

The section contains one image and the line "Refer to jayasuriya2004-hidden.pdf".
The concrete parameters — payload sizes {400, 700, 1000} B, node placement at
dᵢ/2 and dᵢ, Two-Ray Ground, 1000 B at 1 Mb/s for the hidden-terminal runs —
appear only in `docs/background.md`, which is presented to students as
explanatory reading.

**Effect:** the required parameters are only reachable through a document that is
not authoritative.

**Decision needed:** move those parameters into the instructions, or declare
`background.md` authoritative for Part 2. See also L2-1.

---

## L3-1 · Lab 03 hidden-terminal experiment has no deliverable · High · Lab 03

**Paths:** `Lab-03-Adhoc/docs/Lab-03-Instructions.md` (Part 5, RTS/CTS off vs on,
"measure throughput & PDR"), `Lab-03-Adhoc/docs/deliverables.md` (Part 5 is
"Discussion & Theory" — `throughput_vs_nominal.txt`, `best_packet_size.txt`)

The deliverables document lists no file for the hidden-terminal results. The
previous lab README invented names for them (`hidden_off.csv`, `hidden_on.csv`,
`hidden_comparison_plot.png`, two animations, two screenshots); those names have
no support elsewhere and have been removed rather than propagated.

**Effect:** students perform an experiment with nothing to hand in, or invent
file names that will not match grading.

**Decision needed:** add hidden-terminal deliverables, or state that Part 5 of
the instructions is not submitted.

---

## L3-2 · Part numbering differs between Lab 03 documents · Medium · Lab 03

**Paths:** instructions (Part 4 = OLSR vs AODV, Part 5 = hidden terminal),
`docs/background.md` (Part 4 = hidden terminal; the routing comparison is absent),
`deliverables.md` (Part 5 = discussion)

**Decision needed:** one part numbering.

---

## L3-3 · A stale defect claim was removed from the Lab 03 background note · Low · Lab 03

**Paths:** `Lab-03-Adhoc/docs/background.md`, `Lab-03-Adhoc/code/Lab3_Cpp_TCP.cc`

The background note told students that `Lab3_Cpp_TCP.cc` divides received bytes
by 10 s while sending for 9 s, and asked them to recompute. The code now uses
`txWindow = appStop - appStart` = 9.0 s, the same as the UDP starters — the
defect was fixed (commit `1933c6f`) but the note was not updated. Following it
would have inflated every TCP figure by about 11 %.

**Handled:** the incorrect sentences were removed from `background.md`. The code
was not touched. No decision needed; recorded because the note had been standing
advice to students.

---

## L3-4 · Lab 03 Part 2 asks students to hand-write a loop that a starter already implements · Medium · Lab 03

**Paths:** `Lab-03-Adhoc/docs/Lab-03-Instructions.md` (Part 2: "Modify or reuse
`Lab3_Cpp_Adhoc.cc` to loop", with a C++ snippet),
`Lab-03-Adhoc/code/Lab3_Cpp_PayloadSweep.cc`

`Lab3_Cpp_PayloadSweep.cc` performs exactly that sweep through
`--nodes`, `--pkts`, `--seeds` and `--csv`, and is never referenced by the
instructions.

**Effect:** students edit the starter instead of using the tool built for the
task, and their results diverge from the reference configuration.

**Decision needed:** point Part 2 at `Lab3_Cpp_PayloadSweep`.

---

## L3-5 · Verified: `Lab3_Cpp_Adhoc` delivers nothing beyond two hops with OLSR · High · Lab 03

**Path:** `Lab-03-Adhoc/code/Lab3_Cpp_Adhoc.cc`

Measured on ns-3.47 (native build, 2026-08-28), `--pktSize=1200 --seed=1`:

| Run | Result |
|---|---|
| `--numNodes=3 --distance=200` (default OLSR) | 357 348 rx bytes |
| `--numNodes=4 --distance=200` (default OLSR) | **0 rx bytes** |
| `--numNodes=4 --distance=100` (default OLSR) | **0 rx bytes** |
| `--numNodes=5 --distance=100` or `200` (default OLSR) | **0 rx bytes** |
| `--numNodes=4 --distance=200 --routing=aodv` | 334 016 rx bytes |
| `--numNodes=6 --distance=200 --routing=aodv` | 182 972 rx bytes |

It is independent of distance, so it is not a range problem: with the default
OLSR the routes for three or more hops are not in place inside the 1–10 s
application window, while AODV establishes them on demand.

**Effect:** Part 1 and Part 2 require hop counts {3, 4, 5, 6}; with the default
settings every point except the shortest is zero. Part 4 would report OLSR as
"zero throughput at every chain length", which is an artefact, not a protocol
property.

**Not changed** — lab code is protected.

**Decision needed:** lengthen the simulation (or shorten OLSR's HELLO/TC
intervals) so OLSR converges before traffic starts, or state in the instructions
which routing protocol the multi-hop parts use.

---

## L3-6 · Verified: `Lab3_Cpp_TCP` reports zero at its own default spacing · High · Lab 03

**Path:** `Lab-03-Adhoc/code/Lab3_Cpp_TCP.cc`

Measured on ns-3.47 (native build, 2026-08-28), `--pktSize=1200 --seed=1`:

| Run | Result |
|---|---|
| `--distance=50` | 0.864 Mb/s |
| `--distance=100` | 0.864 Mb/s |
| `--distance=200` (the program's default, and the spacing the lab mandates) | **0 Mb/s** |

**Effect:** the Part 3 TCP-vs-UDP comparison produces no TCP data at the required
geometry, while the UDP three-node chain at the same 200 m does deliver traffic.

**Not changed** — lab code is protected.

**Decision needed:** investigate the TCP chain at 200 m before the labs run.

---

## L4-1 · Lab 04 generated file names do not match the required ones · High · Lab 04

**Paths:** `Lab-04-LTE/code/Lab4_Cpp_LTE.cc`, `Lab-04-LTE/docs/deliverables.md`

The program writes, into `scratch/Lab4outputs/`:
`DlPdcpStats.txt`, `UlPdcpStats.txt`, `DlRlcStats.txt`, `UlRlcStats.txt`, and
`server_trace-<node>-<device>.pcap`. The deliverables require
`DlRlcStats.trace`, `DlPdcpStats.trace` and `server_trace.pcap`.

**Effect:** no run produces a file with a required name; students must rename and
may assume they ran the wrong thing.

**Interim handling:** the Lab 04 README states the names actually produced and
tells students to ask rather than rename silently.

**Decision needed:** change the required names, or change the trace file names in
the program.

---

## L4-2 · `--csv=mobile_results` produces the wrong file name · High · Lab 04

**Paths:** `Lab-04-LTE/docs/Lab-04-Instructions.md` (Part 3, step 1),
`Lab-04-LTE/code/Lab4_Cpp_LTE.cc`

The documented command passes `--csv=mobile_results`, and the program then writes
the per-second file as `<csv>.mobile.csv` — `mobile_results.mobile.csv`. Step 2
of the same task says the output is `ue_mobile_throughput.csv`, which is what the
program writes only when `--csv` is *not* passed, and it is the name the
deliverables require.

**Effect:** the documented command never produces the required file.

**Interim handling:** the Lab 04 README's example omits `--csv`.

**Decision needed:** remove `--csv=mobile_results` from the instructions, or
change how the program names that file. Related: L4-1.

---

## L4-3 · Lab 04 part numbering and the antenna deliverable · Medium · Lab 04

**Paths:** `Lab-04-LTE/docs/Lab-04-Instructions.md` (3 parts),
`Lab-04-LTE/docs/deliverables.md` (6 parts)

Deliverables Part 2 requires `antenna_config_comparison.txt` for parabolic,
cosine and isotropic antennas; in the instructions the antenna types are one step
inside Part 1 and are never revisited. Parts 5 and 6 of the deliverables
(`trace_choice.txt`, `conclusions.txt`) have no corresponding section in the
instructions.

**Decision needed:** align the part numbering and give the antenna comparison and
the two discussion files a task section.

---

## X-1 · `payload_sweep_results.csv` is required by two labs with different schemas · Medium · Labs 02 and 03

**Paths:** `Lab-02-WiFiPerformance/docs/deliverables.md`
(`rate_mbps,payload_bytes,throughput_bps`),
`Lab-03-Adhoc/docs/deliverables.md` (`num_nodes,pkt_size,throughput_bps`)

The same file name is required in both labs with different columns. If the two
labs are ever collected together, one overwrites the other.

**Decision needed:** rename one of them.

---

---

## X-2 · The instruction documents predate the starters' command-line arguments · High · all labs

Every lab's instruction document tells students to edit the starter source, while
the program already exposes the same thing as an argument:

| Instruction | Existing argument |
|---|---|
| Lab 01: "Edit `Lab1_Cpp_TwoRay.cc` to place nodes at (0,0) and (d,0)" | `--distance` |
| Lab 03 Part 2: "Modify or reuse `Lab3_Cpp_Adhoc.cc` to loop" (C++ snippet) | `Lab3_Cpp_PayloadSweep --nodes --pkts --seeds` |
| Lab 03 Part 5: `Config::SetDefault(… RtsCtsThreshold …)` | `Lab3_Cpp_Hidden --enableRtsCts` |
| Lab 02 Part 1: "Edit distances in the script to form an equilateral triangle of side 10 m" | `Lab2_Cpp_Scenario1.cc` already builds that geometry |
| Lab 04 Part 1/2: "Set antenna type", "Set antenna to isotropic in code", "Enable tracing" | `--antenna`; tracing is already on |
| Lab 00 Part 2: add `AnimationInterface` yourself | the provided `Lab0_Cpp_Anim.cc` |

**Effect:** students edit the starters and drift away from the reference
configuration, and the lab README (which documents the arguments) and the
instructions read as two different labs. The root README resolves such conflicts
in favour of the instructions, which is the older text.

**Not changed** — replacing a task's procedure is a change to the assignment.

**Decision needed:** update each instruction step to use the documented argument.
See also L0-2, L3-4, L4-3.

---

## Minor items not worth an issue each

- The instruction documents build with `cmake --build build -j$(nproc)` while the
  lab READMEs and `docs/environment.md` use `./ns3 build`. Both work; only the
  hard-coded `~/ns-allinone-3.47/ns-3.47` paths were replaced (with `"$NS3_DIR"`)
  because they are wrong for the recommended Docker route.
- `make help` advertises `lab0` as "Run the first Lab-00 Python script found",
  and `make lab0` now prints "No Lab-00 Python script found" — the Python labs
  were removed in commit 4807019.
- Analysis questions and CSV column lists are stated verbatim in both the
  instructions and the deliverables of Labs 02, 03 and 04. Harmless while they
  agree, but they are two places to keep in step.
- `Lab-02-WiFiPerformance/docs/Lab-02-Instructions.md` Part 1 numbers steps 1–5,
  then inserts two command subsections, then resumes with a fresh list "6." and
  "7."; its step 2 ("Rebuild ns-3:") has no command under it.
- `common/ns3DemoCommands.txt` and `common/scripts/seed_manager.sh` ship to
  students but nothing references them. The demo file's CSV loop writes
  `lab1_results.csv` with columns that do not match any Lab 01 deliverable, so it
  is no longer pointed at from the Lab 01 README.
- `Lab-04-LTE/docs/Lab-04-Instructions.md` also uses `.trace` for the LTE trace
  files (see L4-1), and points at
  `doc/tutorial/html/lte-user.html` for trace formats; no such page exists in an
  ns-3 tree.
- `scripts/ci_smoke.sh` (run by `make check`) verifies the installation through
  the Python bindings and runs `examples/tutorial/first.py`, although the labs
  are C++ only. It works, but a Python failure would look like a broken
  environment to a C++ student.
- NetAnim is referred to as `netanim-3.108` (tarball) in three documents and as
  `netanim-3.109` (App Store) in the old `common/setup.md`.
  `docs/environment.md` now describes the tarball build and mentions the App
  Store as an alternative.
- `Lab-01-Propagation/docs/Lab-01-Instructions.md` reuses the Friis illustration
  for the Nakagami section, and its COST231 and Friis tasks say "Repeat steps
  1–6" although the Two-Ray task they refer to has three steps.

---

## Communication

- **Canvas is authoritative** for deadlines, corrections and anything that
  affects assessment. The root README says so.
- Discord and discussion during lab sessions are fine for support, but any
  clarification that changes what students must do or hand in should also be
  published on Canvas.
- Supervisors should answer from the agreed decisions recorded here, so that two
  students asking the same question get the same answer.
- Interpretation questions that are not settled here should go to the course
  coordinator rather than being resolved per session.
