# esphome-configs

[![Static CI Tests](https://github.com/nolte/esphome-configs/actions/workflows/build-static-tests.yaml/badge.svg)](https://github.com/nolte/esphome-configs/actions/workflows/build-static-tests.yaml)

A DRY, self-hosted collection of [ESPHome](https://esphome.io/) device configurations. Concrete
devices are tiny YAML files that compose reusable building blocks, so behaviour like Wi-Fi,
diagnostics, relays, timers, and sensors is defined once and shared across many devices.

## Purpose

- Manage many ESPHome devices without copying full configs: each device composes shared packages.
- Built for DIY smart-home makers running a self-hosted [Home Assistant](https://www.home-assistant.io/)
  setup, comfortable with YAML and flashing firmware.
- Covers smart plugs (Gosund SP111, NOUS A1T), ESP32 cameras, displays and voice (ESP32-S3-BOX-3,
  Ulanzi TC001), and sensors (SHT3x-D, SoMoSe, multi-point liquid level).
- Keeps Wi-Fi and MQTT credentials out of the repository: they're read from environment variables
  at compile time.

## Usage

Every file under `src/*.yaml` is one device. A device file stays small: it sets a few
substitutions and pulls in packages from `src/common/` through ESPHome's
[`packages:`](https://esphome.io/components/packages.html) include mechanism.

```yaml
# src/gosund-sp111-02.yaml  (a device)
substitutions:
  name: gosund-sp111-02
  id: gosund_sp111_02
  comment: "Washing machine plug"

packages:
  plug: !include
    file: common/gosund-sp111.yaml       # hardware profile (relay, button, energy chip)
  kill: !include
    file: common/switch-kill-sensor.yaml # reusable behaviour
    vars:
      kill_sensor_entity: binary_sensor.water_leak
```

`common/gosund-sp111.yaml` in turn includes `common/base.yaml` (Wi-Fi, API, OTA, diagnostic
sensors), so a new plug is ~10 lines instead of a full config.

### Prerequisites

- [Docker](https://docs.docker.com/get-docker/): ESPHome runs through `ghcr.io/esphome/esphome`,
  so no local Python install is needed.
- [go-task](https://taskfile.dev/): the task runner (`task -l` lists everything).
- [`pass`](https://www.passwordstore.org/): supplies Wi-Fi secrets to the build
  (`network/wifi/ssid`, `network/wifi/password`).
- An ESP device on `/dev/ttyUSB0` for the first, serial flash; later updates go over the air.

### Compile and flash

Initial flash over USB (for example when converting a device from other firmware to ESPHome),
which compiles to `/tmp/firmware.bin`:

```sh
task esphome:compile DEVICE_FILE=nous-a1t-09.yaml
```

Run and flash a device over serial via `/dev/ttyUSB0`:

```sh
task esphome:run DEVICE_FILE=nous-a1t-08.yaml
```

### Over-the-air update

```sh
task esphome:run DEVICE_FILE="nous-a1t-08.yaml --device=192.168.x.x"
```

The compile and run tasks inject the Wi-Fi (and optional MQTT) credentials as environment
variables from `pass`. Review `.taskfiles/Taskfile_esphome.yml` before the first run to match your
network, timezone, and serial device.

### Flashing from an Ansible / NetBox inventory

Device configs can also be resolved from a locally running NetBox deployment through an Ansible
inventory:

```sh
export ANSIBLE_INVENTORY=~/repos/github/argo-charts/src/applications/netbox/configuration/inventory/inventory.yaml
export DEVICE_NAME="nous-a1t-08"

task esphome:run \
  DEVICE_FILE="$(jq -r '"\(.local_context_data[0].esphome.config) --device=\(.ansible_host)"' <<< $(ansible-inventory -i $ANSIBLE_INVENTORY --list | jq '._meta.hostvars["$DEVICE_NAME"]' -r))"
```

## Documentation

Per-device and per-package documentation lives under `docs/` and is built with MkDocs:

```sh
task mkdocs:start   # live preview on http://localhost:8002
```

## Structure

```text
src/
├── *.yaml          # device files, one per physical device
├── common/         # reusable packages: hardware profiles, behaviours, time
│   ├── *.yaml
│   ├── sensor/     # snippet-level sensor includes
│   ├── binary_sensor/
│   └── text_sensor/
├── include/        # C++ headers used by lambdas (for example somose.h)
├── my_components/  # custom ESPHome external components (for example somose)
├── poc/            # proof-of-concept configs, not production devices
└── archive/        # retired configs kept for reference
docs/               # MkDocs documentation source
.taskfiles/         # go-task definitions for compile and flash
```

## Related repositories

- [nolte/taskfiles](https://github.com/nolte/taskfiles): shared go-task definitions used by the Taskfile
- [nolte/gh-plumbing](https://github.com/nolte/gh-plumbing): reusable GitHub Actions workflows (CI, spelling, automerge)
- [BeFlE/SoMoSe](https://github.com/BeFlE/SoMoSe): soil moisture sensor used by the `somose` component
- [lubeda/EspHoMaTriXv2](https://github.com/lubeda/EspHoMaTriXv2): pixel matrix support for the Ulanzi display

## Status

Personal, actively maintained collection. Device configs are added and revised as the hardware
setup changes; entries under `src/poc/` and `src/archive/` are experimental or retired and not
kept production-ready.
