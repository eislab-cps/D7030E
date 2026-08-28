# Usability test report — lab documentation restructure

For course staff. Students do not need this document.

**Date:** 2026-08-28 · **Baseline commit:** `bb3e78b` · **Branch:** `main`
**Participants: none.** All testing was performed by AI agents. No student,
teacher, supervisor or other person was asked to take part or to do follow-up
work.

## Methods

| Method | What it did |
|---|---|
| First-time-student cognitive walkthrough | Fresh-context agent, no knowledge of the restructure, entering at `README.md` and navigating by links only; answered 12 fixed questions per lab |
| Independent link and consistency audit | Fresh-context agent: resolved every markdown link and image with case sensitivity, mapped reachability from `README.md`, compared documented command-line flags against `cmd.AddValue` in the C++ sources, and listed duplicated requirements |
| Programmatic link check | Script resolving all relative links and images in all 31 markdown files |
| Technical verification | `git diff --check`, SHA-256 comparison of all protected artefacts against the baseline, and real ns-3.47 runs of Labs 00–04 |
| Round-2 fresh-context retest | Third agent, no prior context, re-running the walkthrough against the corrected documentation, restricted to presentation |
| Round-3 fresh-context confirmation | Fourth agent, no prior context, asked to confirm or refute eight specific claims about links, reachability, command accuracy and rendering |

Labs tested: 00, 01, 02, 03, 04 — all five, in every round.

## Round 1 findings and what was done

Findings that could only be fixed by changing lab code, a task, or a deliverable
were **not** fixed; they are recorded in [teacher-notes.md](teacher-notes.md).

| Sev | Finding | Action |
|---|---|---|
| Critical | `docs/archive/README.md` and the staff documents were untracked, so the four "Archived — not current" banners would 404 on GitHub | Staged with the commit |
| Critical | Lab 01 requires two seeds; no Lab 01 program has a seed argument | Documentation lists only real arguments; recorded as L1-2 |
| Critical | Lab 02 deliverables require payload-sweep and hidden-terminal runs that no Lab 02 program supports | Invalid command line removed from the lab README; recorded as L2-1 |
| High | Lab READMEs claimed multi-hop needs `--routing` or throughput is zero — false, the starter defaults to OLSR | Corrected in `Lab-03-Adhoc/README.md` and `docs/troubleshooting.md` |
| High | The four `docs/background.md` files were named but not linked, so nothing reached them | Linked from each lab README's file table |
| High | Lab READMEs were reachable only through GitHub's folder rendering | Root README now links `Lab-0X-…/README.md` directly |
| High | `background.md` in Labs 03 and 04 restated the deliverables, and disagreed with them | Those sections replaced by a link to `deliverables.md` |
| High | Instruction documents hard-coded `~/ns-allinone-3.47/ns-3.47`, wrong for the recommended Docker route | Replaced with `"$NS3_DIR"` (execution context only; no task changed) |
| High | Lab 00 README added a third way to run the hello program | Removed; it now defers to Part 1 of the instructions |
| Medium | `Lab-03-Adhoc/docs/background.md` listed four command-line flags and omitted `--routing` | Replaced by a pointer to the lab README's argument table |
| Medium | Lab 02 README duplicated the rate/seed sweep loop from the instructions | Removed from the README |
| Medium | Lab 01 README pointed at `common/ns3DemoCommands.txt` as a "worked example", but it writes different file names and columns than the deliverables require | Pointer removed |
| Medium | Lab 02 README asserted that Lab 01 supplies *dᵢ* for node placement; the programs use a fixed 10 m triangle | Claim removed from the prerequisite |
| Low | Root README named the two staff-only files, advertising them | Sentence removed |
| Low | Deliverables titles read "Lab 1"…"Lab 4" against "Lab 00"…"Lab 04" everywhere else | Padded to two digits (titles only) |
| Low | Two absolute `/`-rooted links in an archived document | Made relative |

## Technical verification

- **Link check:** all relative links and images in all 31 markdown files resolve,
  with matching case; no `/`-rooted links remain; the three heading anchors used
  exist in their target.
- **`git diff --check`:** clean apart from five pre-existing markdown hard line
  breaks (two trailing spaces before image captions in the Lab 01 and Lab 02
  instructions), which are deliberate markdown and were left alone.
- **Protected artefacts:** SHA-256 of every `.cc`, `.sh`, `.ps1`, `.txt`, `.yml`,
  `Makefile`, `Dockerfile` and `CMakeLists.txt` is identical to the baseline.
- **Markdown linting:** no linter is installed offline; not run.
- **Lab execution on ns-3.47** (native build, GCC 13.3.0, CMake 3.28.3). Every
  documented command was executed and its output location confirmed:

| Run | Result |
|---|---|
| `Lab0_Cpp_Hello` | prints `Hello Simulator` |
| `Lab0_Cpp_Anim` | writes `scratch/Lab0outputs/lab0_cpp_anim.xml` |
| `Lab1_Cpp_Friis --distance=100` | prints the `CSV,model=Friis,…` line; writes `scratch/Lab1outputs/Lab1_Friis.xml` and two PCAPs |
| `Lab2_Cpp_Scenario1 --rate=11 --seed=1` | 2.73 Mb/s; writes `scenario1_anim.xml` |
| `Lab2_Cpp_Roaming --speed=5 --simDuration=25 --seed=1` | writes `roaming_throughput.csv` |
| `Lab3_Cpp_Adhoc` | works at 3 nodes; **zero beyond two hops with the default OLSR** — see L3-5 |
| `Lab3_Cpp_TCP` | 0.864 Mb/s at 50–100 m; **zero at its default 200 m** — see L3-6 |
| `Lab3_Cpp_Hidden --enableRtsCts=0` | reports per-STA PDR |
| `Lab4_Cpp_LTE --dataRate=10Mbps --distance=100 --antenna=isotropic` | 10.0 Mb/s; writes the four `*Stats.txt` traces and `server_trace-5-1.pcap` |

The two zero-throughput results were found by running the labs, not by reading
them. Neither was fixed; both are recorded with their measurements as L3-5 and
L3-6.

- **Existing test suites:** `dev-tests/` is git-ignored and its Python suites
  target lab files removed in commit `4807019`; not run.

## Rounds 2 and 3

Re-run from a fresh context against the corrected documentation, restricted to
presentation and organisation.

It confirmed: every link resolves with matching case and none is `/`-rooted; all
five lab READMEs, all five instruction documents, all five deliverables, the four
`background.md` files and the four support documents are reachable from
`README.md` by links alone; nothing in `docs/archive/` and neither staff document
is reachable from the student path, and every archived file carries a visible
"not current" marker; every flag in the student-facing commands exists in the
corresponding `.cc`; the root README is scannable and names one starting point.

It found further documentation-only defects, all fixed in a third pass:

| Sev | Finding | Action |
|---|---|---|
| High | `Lab-03-Adhoc/docs/background.md` told students that `Lab3_Cpp_TCP.cc` divides by 10 s and asked them to recompute; the code uses a 9 s window, so the advice would inflate every TCP figure by ~11 % | Sentences removed; recorded as L3-3 |
| Medium | `Lab-04-LTE/README.md` described `--csv` as appending one line (it appends a header and a data line every run) and did not say `ue_mobile_throughput.csv` appears only when `--csv` is absent; the outputs table omitted `Lab4_LTE.xml` | Corrected |
| Medium | `Lab-03-Adhoc/README.md` said `--csv` writes "instead of stdout"; the programs also print, and `Lab3_Cpp_Hidden` appends while the others overwrite | Corrected |
| Medium | `docs/troubleshooting.md` assumed every program has a seed argument; the Lab 01 starters have none | Row reworded to point at the lab README |
| Medium | `Lab-04-LTE/docs/background.md` said ns-3.40 | Corrected to 3.47 |
| Medium | `docs/references.md` cited only RFC 7181 for OLSR; ns-3's `olsr` module implements RFC 3626, which the Lab 03 instructions cite | RFC 3626 added |
| Medium | The `cp` step in four lab READMEs and `docs/environment.md` did not say which directory it runs in | "From the repository root" added |
| Medium | `Lab-03-Adhoc/docs/background.md` numbered the hidden-terminal section "Part 4" where the instructions use Part 5 | Heading now names the instructions' part |
| Low | "Required work" and "Hand in" rendered as one run-on line in all five lab READMEs | Made list items |
| Low | `docs/simulation-background.md` had an unclosed fence, escaped characters in a code block, unversioned doxygen links, and advised `--RngRun` where the labs use `--seed` | All corrected |
| Low | `run.ps1` / `docker-compose.yml` sit at the repository root undocumented, so a Windows student could adopt the broken workflow | One line in `docs/environment.md` marks them unsupported |
| Low | Lab handout PDFs were named but not linked | Linked |

Two round-3 suggestions were **not** taken, because acting on them would have
resolved a conflict without evidence or edited a task: reconciling the "two
seeds"/"three seeds" split in Lab 02 (L2-3), and rewriting instruction steps to
use the starters' arguments (X-2). One could not be checked offline: whether the
doxygen URLs in `docs/references.md` use the correct name mangling, since
external links cannot be resolved from this environment.

## Assessment against the success criteria

| Criterion group | Result |
|---|---|
| Navigation and findability | **Pass.** One entry point; every lab reachable in one click from the root README; no orphaned student document; no broken link; archive and staff notes off the student path and marked |
| Authority and consistency | **Pass for presentation, with recorded exceptions.** Each lab has one instruction path (`Lab-0X-Instructions.md`) and one deliverables document, and the duplicated paraphrases of them were removed from the lab READMEs and the background notes. The substantive conflicts *inside* the protected documents remain and are recorded, unresolved, in `teacher-notes.md` |
| Clarity and actionability | **Pass.** Prerequisites, first command, working directory, arguments, output locations and deliverables are separately headed in every lab README, and every documented argument was checked against the source and executed |
| Brevity and cognitive load | **Pass.** Root README 49 lines; each lab README 48–61; environment 111; troubleshooting 43. The root README lost 350 lines of duplicated Windows and setup material |
| Error prevention and recovery | **Pass.** Verified problems only; no fix changes an experiment; every table row ends in an action or a route to a supervisor; suspected defects are in teacher notes, not presented as student mistakes |
| Preservation | **Pass.** All protected artefacts byte-identical (SHA-256). In the task documents only links, one figure alt text, one missing italic marker, one cross-reference and the two-digit lab numbering in four titles changed — no task, parameter, deliverable, expected result or ordering |

**Verdict: conditional pass.** All navigation, link, authority-of-presentation,
clarity, brevity and preservation criteria pass, and no critical or high-severity
issue remains that documentation can fix. Twenty-two issues remain open in
`teacher-notes.md`, ten of them High. Every one of them requires a change to lab
code, to a task, to a deliverable, or a course-coordinator decision between two
existing statements, and each was deliberately left unresolved rather than
answered without evidence. The most consequential are L2-1 (Lab 02 deliverables
with no starter that can produce them), L3-5 and L3-6 (measured zero throughput
at the parameters the labs specify), L4-1 (no run produces a required file name)
and X-2 (the instruction documents predate the starters' arguments).

Three test–fix–retest iterations were run: initial walkthrough plus audit, a
fresh-context retest, and a fresh-context confirmation pass. Each was performed
by an AI agent with no prior context. **No human took part in any of it.**
