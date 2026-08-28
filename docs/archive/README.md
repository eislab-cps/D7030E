# Archive — not current

These documents are kept for reference only. They are **not** part of the lab
instructions and students are not expected to read them. Some describe removed
features, an older ns-3 version, or a workflow that no longer matches the
repository.

| File | What it was | Why it is archived |
|---|---|---|
| `lab00-walkthrough.md` | End-to-end Docker walkthrough for Lab 00 | Its run commands (`--animFile=…`, copying `Lab0_Cpp_Anim.xml`) do not match the current program, which writes `scratch/Lab0outputs/lab0_cpp_anim.xml`. Current steps: [Lab 00 README](../../Lab-00-Introduction/README.md), [environment.md](../environment.md) |
| `windows-runps1-workflow.md` | Windows `run.ps1` / `docker compose` workflow | `docker-compose.yml` still refers to the ns-3.40 image and paths, so this workflow does not run against the current ns-3.47 image |
| `netanim-on-windows.md` | Four ways to build and run NetAnim on Windows | Condensed into [environment.md](../environment.md) |
| `lab-validation-report-2026-06-04.md` | ns-3.47 validation report, 2026-06-04 | Predates the current repository: it inventories Python lab files that have since been removed and lists fixes already applied |
