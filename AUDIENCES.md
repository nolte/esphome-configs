# Audiences—`esphome-configs`

<!--
Produced via the `audience-identify` skill, following
spec/project/audience-identification/.
Do not add audiences without first declaring the bounded context below.
-->

## Bounded context

The ESPHome firmware configuration of one private smart-home installation, structured to avoid
repetition. Every physical device is a thin YAML file under `src/*.yaml` that composes shared
packages from `src/common/`. The context includes the custom external components under
`src/my_components/` and the C++ headers that lambdas use under `src/include/`. It also covers the
Taskfile-driven compile and flash workflow that runs through the ESPHome Docker image. Finally, it
covers the CI validation of every live device file and the MkDocs documentation under `docs/`.

The boundary runs at the firmware source level plus its build tooling and documentation: everything
that gets compiled and flashed, and everything that describes how.

Explicitly outside the context:

- The Home Assistant instance itself, its automations, and its dashboards.
- The physical hardware and its procurement.
- Secret material: Wi-Fi and Message Queuing Telemetry Transport (MQTT) credentials live in `pass`,
  and the build injects them at compile time.
- Upstream ESPHome and third-party components (`EspHoMaTriXv2`, `SoMoSe`), which this repository
  consumes rather than maintains.
- The Ansible and NetBox inventory that optionally resolves device addresses.

## Audiences

Each entry: label, relationship category, interaction surface, expectation, documentation `track`
(`user-docs` or `developer-docs` per `spec/project/docs-audience-tracks/`), open questions,
`confirmed` or `assumed`, criticality (primary / secondary / peripheral).

### Direct consumers

- **Do-it-yourself smart-home maker** (`diy-maker`)
  - _category_: direct-consumer
  - _surface_: the published MkDocs site, and direct reading of `src/common/` on GitHub
  - _expects_: a package is self-explanatory enough to adopt on its own
  - _track_: `user-docs`
  - _status_: `assumed`
  - _criticality_: secondary
  - _open questions_: which packages get adopted externally stays unknown. No telemetry exists,
    and nobody has filed an issue to date.

### Operators

- **Operator of this installation** (`home-operator`)
  - _category_: operator
  - _surface_: `task esphome:compile`, `task esphome:run`, `pass` for credentials, serial
    `/dev/ttyUSB0`, over-the-air updates, and optionally the Ansible and NetBox inventory
  - _expects_: an edit to a shared package never breaks an untouched device without warning
  - _track_: `developer-docs`
  - _status_: `confirmed`
  - _criticality_: primary
  - _open questions_: none.

- **Operator of a foreign installation** (`third-party-operator`)
  - _category_: operator
  - _surface_: the same task interface after adapting `.taskfiles/Taskfile_esphome.yml`, plus the
    per-device substitutions
  - _expects_: the substitutions carry across a different network, timezone, serial device, and
    secret store
  - _track_: `developer-docs`
  - _status_: `assumed`
  - _criticality_: peripheral
  - _open questions_: `.taskfiles/Taskfile_esphome.yml` hard-wires the `pass` entry paths, the
    timezone, and the serial device. No portability contract states which of those a foreign
    operator must change.

### Contributors / maintainers

- **Maintainer** (`maintainer`)
  - _category_: contributor
  - _surface_: `CLAUDE.md`, the package layout under `src/common/`, the workflows under
    `.github/workflows/`, `.pre-commit-config.yaml`, and Renovate pull requests
  - _expects_: the conventions for adding a device or a package stay findable as the fleet grows
  - _track_: `developer-docs`
  - _status_: `confirmed`
  - _criticality_: primary
  - _open questions_: none.

- **Occasional external contributor** (`external-contributor`)
  - _category_: contributor
  - _surface_: GitHub issues and pull requests, the README, and CI feedback
  - _expects_: a fast CI verdict on whether a package change still resolves for every device
  - _track_: `developer-docs`
  - _status_: `assumed`
  - _criticality_: peripheral
  - _open questions_: no `CONTRIBUTING.md` ships, so the contribution conventions stay implicit in
    `CLAUDE.md` and the existing layout.

### Governing parties

- `none`: a personal repository with no organization behind it, no compliance regime, and no
  approval body. Recording `none` here is a deliberate answer, not an omission. Related gap
  recorded under §Open questions: the repository ships no `LICENSE`, so the reuse terms for the
  external audiences stay undefined.

### Indirect audiences

- **Household member** (`household-member`)
  - _category_: indirect
  - _surface_: the physical behaviour of the devices (plug switching, display pages, and the voice
    satellite), never the repository
  - _expects_: behaviour doesn't change unannounced, and a device keeps working after an
    over-the-air update
  - _track_: `user-docs` (rationale: reachable only through user-facing documentation, if at all)
  - _status_: `assumed`
  - _criticality_: secondary
  - _open questions_: this entry infers their expectations from living with the devices rather
    than gathering them.

- **Home Assistant instance** (`ha-instance`)
  - _category_: indirect
  - _surface_: the ESPHome API contract each device exposes: entity IDs, device classes, and units
  - _expects_: entity identifiers stay stable, because a rename breaks automations and dashboards
    outside this repository
  - _track_: `developer-docs` (rationale: a machine consumer whose contract serves the operator
    maintaining those automations)
  - _status_: `assumed`
  - _criticality_: primary
  - _open questions_: no inventory records which entities the outside automations reference, so
    the impact of a rename stays unknown.

## Open questions (cross-cutting)

- The repository ships no `LICENSE` file. The `diy-maker` and `third-party-operator` audiences have
  no stated terms under which they may reuse the configurations.
- This document adds no optional subdivisions (geography, organizational unit, tenancy): none of
  them would change the expected deliverable for any of these audiences.

## Revisit triggers

- The repository gains a `LICENSE`, which changes the reuse contract for the external audiences.
- Packages get published for remote consumption (ESPHome `packages:` with a remote source), which
  turns `diy-maker` from a copy-paste reader into a direct dependant.
- A second person gains flash access to the installation, splitting `home-operator` into a role
  rather than a person.
- A device class enters `src/` whose consumer surface differs from plugs, sensors, cameras, and
  displays.
- A device starts handling personal data in a way that introduces a governing party where the
  category currently records `none`.
