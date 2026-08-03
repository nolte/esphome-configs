# Implementation plan: esphome-config-review

> Foundational plan for worktree work on branch 'chore/esphome-config-review'. Authored BEFORE
> substantive work begins, so a fresh, resumable top-level session started in
> this worktree can pick the work up from a known starting point rather than
> reconstructing intent from a half-finished diff.
>
> - Branch: chore/esphome-config-review
> - Worktree: /home/nolte/repos/.worktrees/esphome-configs/esphome-config-review
> - Base: origin/develop (2ff97d5 — ahead of the primary checkout's local develop at afa70f0)

## 1. Goal

Produce one thorough, evidence-backed review of this ESPHome repository: a
repository-level fleet review (package architecture, substitution interface,
credential blast radius, CI coverage) plus a device-level review of every
top-level device configuration in `src/`, consolidated into a single findings
report with severities and named follow-up owners.

## 2. Current state (researched)

Repository layout (as of the worktree base commit):

- `src/*.yaml` — 34 top-level device configurations. Families: `nous-a1t-01..12`,
  `gosund-sp111-01..07` (+ `gosund-sp111-07-timer.yaml`), `cam-01..07`,
  `esp32-s3-box-3-01/02`, `ulanzi-tc001-01`, `box-01`,
  `box-02-water-reservoir`, `box-fementation`, `box-seeds-01`.
- `src/common/*.yaml` — 15 shared packages. Board/device packages
  (`nous-a1t`, `gosund-sp111`, `esp32-cam-mb`, `esp32-s3-box-3`,
  `esp3s-nodemcu-32s`, `esp32-az-delivery`, `ulanzi-tc001`) and behaviour
  packages (`base`, `time`, `timer-cancelable`, `active-duration`,
  `switch-intervall`, `switch-kill-sensor`, `pixel_art`,
  `esp32-s3-box-3-draw`).
- `src/archive/` — 20 retired files, incl. `*.yaml.snipped` fragments.
- `src/ulanzi-tc001-01.html` — non-YAML asset next to the device configs.

Toolchain and CI:

- `.taskfiles/Taskfile_esphome.yml` — `task esphome:compile` / `esphome:run`
  drive the `ghcr.io/esphome/esphome` container. Secrets come from `pass`
  (`network/wifi/*`); MQTT endpoint, port, user and TZ are hard-coded task
  literals (`MQTT_PASSWORD="notset"`).
- `.github/workflows/build-static-tests.yaml` — reusable pre-commit, Trivy and
  chain-bench from `nolte/gh-plumbing@v1.1.20`. **No `esphome config` validation
  of the device files runs in CI**, so a `src/common/` package edit cannot be
  caught before it breaks a device nobody touched — the exact failure mode a
  shared-package repository is built to produce.
- `Taskfile.yml` pulls remote includes from `nolte/taskfiles@develop`
  (unpinned `develop` ref) and does not include the worktree taskfile.
- Repository is `portfolio: excluded` per `CLAUDE.md`; it ships no
  `project/portfolio.yml`, so portfolio-level gates do not apply.

Available specialist reviewers (both read-only, both persist their own report):

- `claude-home-assistant:ha-esphome-fleet-reviewer` → `.audits/esphome-fleet-review/`
- `claude-home-assistant:ha-esphome-config-reviewer` → `.audits/esphome-config-review/`

## 3. Design decision

**Two-phase review, fleet first, device second — and the review stays
read-only in this pass.**

The fleet review runs first because its findings (package cut, substitution
defaults, credential handling, CI coverage) reframe what counts as a
device-level finding: a missing `api:` encryption key in 34 files is one fleet
finding, not 34 device findings. The device pass then runs against all 34
top-level configs, deduplicated against the fleet findings, and reviews only
what is genuinely device-specific.

Devices are reviewed in family batches (`nous-a1t`, `gosund-sp111`, `cam`,
`esp32-s3-box-3`, `ulanzi`, `box-*`), because within a family the configs are
near-identical — a per-family pass surfaces both the shared pattern and the
per-device deviations, with far less redundant work than 34 isolated reviews.

Fixes are deliberately **not** applied in the same pass. The reviewers are
read-only by design ("what generated a config never reviews it"), and a fix
applied mid-review contaminates the remaining findings. Fixes become a
separate, explicitly approved step after the report is accepted.

### Open questions — RESOLVED 2026-08-03

Resolved with the operator via `/nolte-shared:requirements-elicit`. The
authoritative record is `project/requirements/esphome-config-review.md`
(`U_gate = 0.82`, terminated by saturation, all requirements `confirmed`).
Two answers deviate from the plan's original defaults — marked ⚠ below, and
carried into §5.

1. **Fixes in this worktree or a follow-up branch?** → **This branch.**
   Approval is obtained **once at scope level** when the consolidated report is
   presented, *not* per individual finding (`R12`, `R15`). Behaviour-changing
   fixes are permitted; each commit states risk level and affected devices.
2. **Is `src/archive/` in scope?** → **Leak-check only** (`R4`). Retired
   configs are examined for credential leakage and live references, never
   reviewed for content. Non-YAML assets (`src/ulanzi-tc001-01.html`, already
   git-ignored) are out of scope entirely.
3. ⚠ **Should the report be committed?** → **No** (`R10`). `.audits/` is added
   to `.gitignore`; the consolidated report stays a local work product and is
   presented in-session. The branch carries only the fixes, not the findings.
4. **Is a live secrets baseline available?** → **No** (`R14`). The review runs
   statically against the configurations as written, without resolving
   substitutions from the `pass`-backed compile environment.
5. ⚠ **How far should the review push on CI?** → **Implement and activate**
   (`R11`). The workflow that runs `esphome config` over all 34 device files is
   built and switched on in this pass, not merely reported.

## 4. Work steps (ordered)

1. ✅ Capture the requirement precisely via `/nolte-shared:requirements-elicit`,
   and resolve the five open questions in §3 against the operator's answers.
   → `project/requirements/esphome-config-review.md`
2. Run `claude-home-assistant:ha-esphome-fleet-reviewer` over the repository;
   read its report under `.audits/esphome-fleet-review/`.
3. Triage the fleet findings into fleet-wide vs. per-device concerns, so the
   device pass does not re-report fleet issues 34 times.
4. Run `claude-home-assistant:ha-esphome-config-reviewer` per device family
   (6 batches: nous-a1t, gosund-sp111, cam, esp32-s3-box-3, ulanzi, box-*),
   covering all 34 top-level configs.
5. Consolidate every finding into one report: severity, evidence
   (`file:line`), fleet-vs-device scope, and the owning fix skill per finding.
   Every agent-reported finding is re-verified at its stated `file:line` before
   it enters the report (`R6`); coverage gaps are named explicitly (`R8`);
   possibly-deliberate deviations are recorded as questions, not defects
   (`R9`); identifiers only, never resolved credential values (`R7`).
6. Present the consolidated report to the operator and agree the fix scope —
   **one** scope-level approval, not per finding (`R15`). The report itself is
   not committed (`R10`).
7. After that approval: apply fixes in severity order, one logical change per
   commit, each commit stating risk level and affected devices (`R12`).
   Includes building and activating the `esphome config` CI workflow over all
   34 device files (`R11`) and adding `.audits/` to `.gitignore` (`R10`).
8. Open the PR via `nolte-shared:pull-request-create`.

## 5. Invariants / guardrails

- The primary checkout at `/home/nolte/repos/github/esphome-configs` stays on
  `develop`. All work happens in this worktree
  (`spec/project/parallel-working-copies/`).
- The review pass is read-only: the reviewers apply nothing, and no config is
  edited before the operator approves the fix scope. After that single
  scope-level approval, fixes are applied without a further per-finding gate
  (`R15`).
- The consolidated report is never committed; `.audits/` goes into
  `.gitignore` and the branch carries only fixes (`R10`).
- No finding enters the consolidated report unverified: each is re-checked at
  its stated `file:line` before it is reported (`R6`).
- Conventional-Commits messages; branch prefix `chore/` is already set
  (`spec/project/branching-model/`).
- `CLAUDE.md`: this repository is `portfolio: excluded` and ships no
  `project/portfolio.yml` — do not scaffold one, do not add portfolio gates.
- Operator-facing communication in German; code, commits, PR content and
  reports in English.
- Never commit resolved secrets. WiFi/MQTT credentials come from `pass` and the
  compile environment; findings quote variable names, never values.
- Do not flash or compile against real hardware as part of the review.

## 6. Status / resume anchors

- [x] Requirement captured via `/nolte-shared:requirements-elicit`; open
      questions 1–5 in §3 answered
      → `project/requirements/esphome-config-review.md`, `U_gate = 0.82`
- [x] Fleet review run; report read under `.audits/esphome-fleet-review/`
      → NEEDS-WORK: 4 high, 12 medium, 10 low, 4 open questions
- [x] Fleet findings triaged into fleet-wide vs. per-device scope
      → `.audits/esphome-config-review/triage-fleet.md`; all 4 HIGH plus
      M4/M5/M6 re-verified at their `file:line` per R6
- [x] Scope gap resolved with the operator → `R4a`: `src/include/**` and
      `src/poc/**` IN, `src/my_components/**` OUT (orphan reported, code not
      reviewed)
- [x] Device review: `nous-a1t-01..12` — 12/12 read, 5 findings verified
- [x] Device review: `gosund-sp111-01..07` + `gosund-sp111-07-timer` — 8/8 read,
      7 findings verified; **G1**: package-order-dependent clock binding that
      reversed between ESPHome versions (escalated to fleet)
- [x] Device review: `cam-01..07` — 7/7 read, 7 findings verified;
      **cam-07 does not validate today** (C1)
- [x] Device review: `esp32-s3-box-3-01/02` — 2/2 read, 8 findings verified;
      voice path explicitly clean
- [x] Device review: `ulanzi-tc001-01` — read in full, 6 findings verified
- [x] Device review: `box-01`, `box-02-water-reservoir`, `box-fementation`,
      `box-seeds-01` — 4/4 read, 6 findings verified; **box-01 is dead twice
      over** (dead includes + removed `platform: custom`)
- [x] Review `src/poc/` (19 YAML, per `R4a`) — 19/19 read, 7 findings verified;
      no credentials, no collision with a live device; 18/19 unloadable
- [x] Review `src/include/**` (per `R4a`) — 3 headers read in full, 9 findings
      → `.audits/esphome-config-review/findings-infra.md`
- [x] Toolchain & CI reviewed (`.taskfiles/Taskfile_esphome.yml`,
      `.github/workflows/**`, `Taskfile.yml`) — 6 + 5 findings, same file
- [x] `src/archive/` leak- and live-reference check (no content review)
      → **no credential leak**; but 16 live files still `!include` the moved
      `*.yaml.snipped` fragments (A1, sharpens fleet H3)
- [x] Consolidated findings report written (not committed)
      → `.audits/esphome-config-review/REPORT.md`
- [x] Report presented; fix scope agreed with the operator (one approval, `R15`)
      — **2026-08-03 decision, authoritative for the rest of this branch:**
      - **Packages A, B, C approved. Package D (H1/H2/M7/T1 hardening) NOT
        approved** — H1 `api:` encryption, H2 OTA password, M7 shared fallback
        password and T1 `--privileged` stay as reported, unfixed. They remain
        open findings, deliberately carried.
      - **box-01 → archive.** Moves to `src/archive/`. Drops A1(box-01), X1,
        X5, X6, I1, I2 as fixes. Live device count becomes **33**.
      - **`src/poc/` → excluded from CI, otherwise untouched.** P1–P7 are not
        fixed. `src/my_components/somose/` stays (two POCs consume it).
      - Package E (hygiene/low) not requested — not applied.
- [x] Fixes applied (only the approved ones), risk level per commit — 13 commits
      - Package A: archive box-01; `${comment}` default; SCCB bus for the 7 cams
        (+ psram annotation, `${name}_ Distance`); `project.name` namespace on
        the 2 box nodes
      - Package B: see below
      - Package C: timezone + explicit `time_id` (E1/G1); `early_pin_init` +
        `restore_from_flash` (G2/G3); `psram.ignore_not_found` (E2);
        `api.reboot_timeout: 0s` (X4/S6/G6); water-level `clamp` (X2); SCD30
        ASC off (X3); pixel-art mode leavable (S1)
      - Package D deliberately NOT applied (operator decision)
- [x] `esphome config` CI validation over all device files built and active
      → `.github/workflows/esphome-config-validate.yaml`, scoped to `src/*.yaml`
- [x] `.audits/` added to `.gitignore`
- [x] **Empirically validated** — ESPHome 2026.7.3 run locally over every device
      file (allowed by `R13`, placeholder env vars per `R14`):
      **before 30/34 pass (4 fail), after 33/33 pass.** Confirms C1 and M13,
      corrects my H4 misclassification, and surfaced two dated deprecations
      (`image:` block format and `neopixelbus`, both removal targeted 2027.1).
      Recorded in REPORT.md §8.
- [ ] **← NEXT:** decide what the branch commits besides the fixes
      (`project/requirements/**`, `.resume/**` — the open item flagged in the
      requirements artifact under "R10 versus this artifact"), then open the PR
      via `nolte-shared:pull-request-create`.

> Resume: cd into this worktree and run `task resume` (or `claude --resume`),
> then continue this plan from the first unchecked box.
