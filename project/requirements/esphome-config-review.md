# Requirements — ESPHome repository review (fleet + all 34 device configurations)

<!--
Produced via the `requirements-elicit` skill, following
spec/project/requirements-elicitation/.
Do not record a requirement before declaring the bounded context below.
`c_d` is an uncertainty proxy (self-consistency-derived), not a calibrated
probability. A requirement is `confirmed` only after an explicit teach-back.
-->

- Slug: `esphome-config-review`
- Elicited: 2026-08-03
- Working copy: `/home/nolte/repos/.worktrees/esphome-configs/esphome-config-review` (branch `chore/esphome-config-review`)
- Upstream plan: `.resume/esphome-config-review/plan.md`
- Interview language: German (operator's language). Artifact language: English (repository primary language).

## Bounded context

- **What is being built** — one evidence-backed review of this repository, in two
  passes: a repository-level fleet review (package architecture, substitution
  interface, credential blast radius, CI coverage) followed by a device-level
  review of all 34 top-level configurations under `src/`, consolidated into a
  single findings report. The approved fixes are then applied on this branch,
  including the CI validation workflow.
- **For whom** — the repository operator as the sole human party. There is no
  external audience, no compliance addressee, no downstream consumer of the
  report.
- **Explicitly out of scope** — compiling or flashing against real hardware;
  non-YAML assets beside the configs (`src/ulanzi-tc001-01.html`, already
  git-ignored); full content review of `src/archive/`; portfolio-level gates
  (this repository is `portfolio: excluded` per `CLAUDE.md` and ships no
  `project/portfolio.yml`).

## Understanding KPI

- Thresholds: `τ_low = 0.4`, `τ_high = 0.8`, self-consistency `k = 2`, question budget = `8`
  <!-- spec defaults, unchanged -->
- Questions asked: **6** of 8
- `U_gate = min_d c_d` over required dimensions = **0.82**
- Termination: `saturation` — every applicable dimension is at or above `τ_high`
  with teach-back obtained, and no remaining candidate question carried positive
  net EVPI.

### Gap matrix

| Dimension | Applicable | `c_d` | Uncertainty source | Evidence event |
|---|---|---|---|---|
| `functional` | yes | 0.88 | specification | Q5 answer (CI validation "implement it") + teach-back R1–R3, R11 confirmed |
| `non_functional` | yes | 0.85 | specification | Q4 answer (report not committed) + Q6 answer (behaviour-changing fixes allowed, risk-labelled) + teach-back R10, R12 |
| `constraints` | yes | 0.88 | interpretation | plan §5 invariants, restated and teach-back confirmed as R13; no-hardware constraint reconfirmed via Q6 framing |
| `domain_objects` | yes | 0.85 | interpretation | repository inspection (34 `src/*.yaml`, `src/common/**` incl. `binary_sensor/`, `sensor/`, `text_sensor/`, `src/archive/`) + teach-back R3, R4 |
| `actors` | yes | 0.82 | interpretation | teach-back R16 confirmed (operator sole human party; reviewer agents, GitHub Actions, ESPHome container as non-human actors) |
| `acceptance_criteria` | yes | 0.88 | specification | Q1 answer ("actionable plan": severity + `file:line` + fleet/device scope + owning fix skill) + teach-back R5 |
| `edge_cases` | yes | 0.85 | specification | Q3 answer — all four review failure modes named as no-gos + teach-back R6–R9 |
| `scope_boundaries` | yes | 0.88 | specification | Q2 answer (toolchain/CI in; archive leak-check only; non-YAML assets out) + teach-back R4 |

**Self-consistency check (`k = 2`).** Two independent readings of "review" were
generated before Q1 and diverged materially — reading A produced a completeness
inventory whose success criterion is coverage, reading B produced a prioritised
defect list whose success criterion is actionability. That divergence set
`acceptance_criteria` to 0.30 (below `τ_low`) and forced Q1. A second `k = 2`
check before Q3 diverged on recall-versus-precision ("flag every deviation" vs.
"report only hard, verified findings") and forced the edge-case question.

**Withheld clarification (discretionary-zone restraint).** Open question 4 of the
plan (is a resolved-secrets baseline available?) was deliberately *not* asked.
With hardware compilation excluded by `R13` and secrets sourced from `pass`, the
static reading is the only viable one left, so expected information gain did not
exceed the question cost. It was carried into the teach-back as an assumption
(`R14`) instead and confirmed there.

**Forced clarifications (below `τ_low`).** Q1 (`acceptance_criteria`, 0.30),
Q2 (`scope_boundaries`, 0.35), Q3 (`edge_cases`, 0.35).

## Requirements

<!-- Each requirement in EARS/CNL form, tagged confirmed/assumed, with
     traceability to the user utterance(s) that produced it. -->

### Review execution

- **R1** — WHEN the review starts, the operator's agent SHALL run the fleet-level
  review over the repository first and read its report before any device-level
  review begins.
  - _dimension_: `functional` · _status_: `confirmed` · _source_: plan §3 design decision, teach-back turn 7 "Alles bestätigt"

- **R2** — WHEN fleet-level findings exist, the agent SHALL triage them into
  fleet-wide versus device-specific concerns, so that a fleet-wide finding is
  reported once rather than once per device.
  - _dimension_: `functional` · _status_: `confirmed` · _source_: plan §3, teach-back turn 7

- **R3** — WHEN the device-level review runs, it SHALL cover all 34 top-level
  configurations under `src/`, batched by device family (`nous-a1t-01..12`,
  `gosund-sp111-01..07` + `gosund-sp111-07-timer`, `cam-01..07`,
  `esp32-s3-box-3-01/02`, `ulanzi-tc001-01`, `box-01` / `box-02-water-reservoir`
  / `box-fementation` / `box-seeds-01`).
  - _dimension_: `functional` · _status_: `confirmed` · _source_: teach-back turn 7

- **R11** — WHEN the review reaches the CI finding, the agent SHALL build and
  activate a CI workflow that runs `esphome config` over all 34 device files,
  rather than only reporting the gap.
  - _dimension_: `functional` · _status_: `confirmed` · _source_: Q5 answer "Mit umsetzen"

### Scope

- **R4** — The review scope SHALL be `src/*.yaml` (34 files), `src/common/**`,
  `.taskfiles/Taskfile_esphome.yml`, `.github/workflows/**` and `Taskfile.yml`.
  `src/archive/` SHALL be examined only as a credential-leak and live-reference
  check, never reviewed for content. Non-YAML assets beside the configs SHALL be
  out of scope.
  - _dimension_: `scope_boundaries` · _status_: `confirmed` · _source_: Q2 answer "Toolchain & CI, src/archive/ nur als Leak-Check"

- **R4a** — *(scope revisit, 2026-08-03, after the fleet pass surfaced three
  trees `R4` had neither included nor excluded)* — `src/include/**` and
  `src/poc/**` SHALL additionally be in scope and reviewed for content.
  `src/my_components/**` SHALL be out of scope; its orphaned state is reported
  as a finding, its code is not reviewed.
  - _dimension_: `scope_boundaries` · _status_: `confirmed` · _source_: Q7 answer "src/include/*.h, src/poc/ (19 YAML)"
  - _rationale_: `src/include/*.h` ships inside live firmware via `includes:`,
    so leaving it out would have made the device pass structurally incomplete.
    `src/poc/` was taken in full rather than as a leak-check.

- **R13** — The review and the fix pass SHALL NOT compile or flash against real
  hardware. (`esphome config` validation in CI is not affected — it resolves and
  validates configuration without touching a device.)
  - _dimension_: `constraints` · _status_: `confirmed` · _source_: plan §5 invariant, teach-back turn 7

- **R14** — The review SHALL be performed statically against the configurations
  as written, without resolving substitutions from the `pass`-backed compile
  environment.
  - _dimension_: `constraints` · _status_: `confirmed` · _source_: teach-back turn 7 (resolves plan open question 4; the clarifying question itself was withheld on EVPI grounds)

### Report

- **R5** — Each finding in the consolidated report SHALL carry a severity, its
  evidence as `file:line`, its fleet-versus-device scope, and the owning fix
  skill.
  - _dimension_: `acceptance_criteria` · _status_: `confirmed` · _source_: Q1 answer "Umsetzbarer Handlungsplan"

- **R6** — WHEN a reviewer agent reports a finding, the agent SHALL verify it
  itself at the stated `file:line` before that finding may enter the
  consolidated report.
  - _dimension_: `edge_cases` · _status_: `confirmed` · _source_: Q3 answer, no-go "Unverifizierte Agent-Funde"

- **R7** — The report SHALL quote identifiers only and SHALL NEVER contain a
  resolved credential value — including values found in `src/archive/`.
  - _dimension_: `edge_cases` · _status_: `confirmed` · _source_: Q3 answer, no-go "Aufgelöste Secrets im Report"

- **R8** — The report SHALL state explicitly what was reviewed and what was not.
  WHEN a device family cannot be reviewed (agent abort, context limit), the
  report SHALL name that gap rather than read as full coverage.
  - _dimension_: `edge_cases` · _status_: `confirmed` · _source_: Q3 answer, no-go "Stille Lücken in der Abdeckung"

- **R9** — WHEN a deviation may be deliberate (for example
  `gosund-sp111-07-timer`, or the `box-fementation` naming variant), the report
  SHALL record it as a question to the operator rather than as a defect.
  - _dimension_: `edge_cases` · _status_: `confirmed` · _source_: Q3 answer, no-go "Absicht als Fehler gemeldet"

- **R10** — The consolidated report SHALL NOT be committed. `.audits/` SHALL be
  added to `.gitignore`, and the branch SHALL carry only the fixes, not the
  findings.
  - _dimension_: `non_functional` · _status_: `confirmed` · _source_: Q4 answer "Report nicht committen" (resolves plan open question 3, overriding the plan's default)

### Fixes

- **R12** — WHEN a fix is applied, its commit SHALL state the risk level and the
  affected devices explicitly. Behaviour-changing fixes ARE permitted; the
  residual risk is carried by the operator at flash time.
  - _dimension_: `non_functional` · _status_: `confirmed` · _source_: Q6 answer "Verhaltensänderung erlaubt, je Fix markiert"

- **R15** — Fix approval SHALL be obtained once, at the scope level, after the
  consolidated report is presented — not per individual finding.
  - _dimension_: `non_functional` · _status_: `confirmed` · _source_: teach-back turn 7 (Q6 explicitly rejected the per-finding gate)

### Actors

- **R16** — The sole human party is the repository operator. The non-human
  actors are the two read-only reviewer agents
  (`ha-esphome-fleet-reviewer`, `ha-esphome-config-reviewer`), GitHub Actions,
  and the `ghcr.io/esphome/esphome` container.
  - _dimension_: `actors` · _status_: `confirmed` · _source_: teach-back turn 7

## Resolution of the plan's open questions (§3 of `.resume/esphome-config-review/plan.md`)

| # | Question | Resolution | Deviates from plan default? |
|---|---|---|---|
| 1 | Fixes in this worktree or a follow-up branch? | This branch; approval once at scope level (`R12`, `R15`) | no |
| 2 | Is `src/archive/` in scope? | Leak- and live-reference check only (`R4`) | no |
| 3 | Should the report be committed? | **No** — `.audits/` goes into `.gitignore`, branch carries only fixes (`R10`) | **yes** |
| 4 | Is a live secrets baseline available? | No — static review as written (`R14`) | no |
| 5 | How far should the review push on CI? | **Implement and activate** the `esphome config` validation workflow (`R11`) | **yes** |

## Surviving assumptions / open risks

Every requirement above is `confirmed`; the risks below are residual, not
unconfirmed requirements.

- **Severity taxonomy not explicitly agreed.** `R5` requires a severity per
  finding but no scale was negotiated. The reviewer agents' own
  Critical / Warning / Suggestion / Info taxonomy will be used. Cheap to correct
  when the report is presented.
- **Behaviour-changing fixes ship unflashed.** `R12` permits them and `R13`
  forbids hardware verification, so correctness at the device is established
  only when the operator flashes. This is an accepted risk, not a gap.
- **The CI workflow cannot be proven correct before it merges.** `R11` activates
  a workflow whose first real signal arrives on this branch's own CI run; a
  green run on the branch is the only available evidence.
- **`R10` versus this artifact.** `R10` scopes the *findings report* out of the
  commit. Whether this requirements artifact (`project/requirements/`) and the
  `.gitignore` change are themselves committed was not asked; both are assumed
  committable as repository hygiene. Resolve before the PR is opened.
- **`R4` was incomplete as elicited.** The interview scoped `src/` by the file
  glob the plan named and never enumerated the subdirectories, so
  `src/include/`, `src/poc/` and `src/my_components/` fell through the gap
  until the fleet pass surfaced them. Corrected by `R4a`. The lesson for a
  future elicitation on a repository: enumerate the tree before agreeing a
  scope, rather than agreeing a scope in the requester's terms.
- **Minor drift in the plan's inventory.** `.resume/esphome-config-review/plan.md`
  §2 counts 15 shared packages; the tree holds 14 `src/common/*.yaml` plus three
  component subdirectories. The plan's file counts should be treated as
  indicative, the tree as authoritative.
