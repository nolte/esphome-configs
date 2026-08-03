# esphome-configs

[![Static continuous integration tests](https://github.com/nolte/esphome-configs/actions/workflows/build-static-tests.yaml/badge.svg)](https://github.com/nolte/esphome-configs/actions/workflows/build-static-tests.yaml)

A duplication-free collection of [ESPHome](https://esphome.io/) device configurations for
do-it-yourself smart-home makers running a self-hosted
[Home Assistant](https://www.home-assistant.io/). Each device is a tiny YAML file composing
reusable building blocks, so Wi-Fi, diagnostics, relays, timers, and sensors have one definition.

## Purpose

- Manage many ESPHome devices without copying full configs: each device composes shared packages.
- Built for do-it-yourself smart-home makers running a self-hosted Home Assistant setup,
  comfortable with YAML and flashing firmware.
- Covers smart plugs (Gosund SP111, NOUS A1T), ESP32 cameras, displays and voice (ESP32-S3-BOX-3,
  Ulanzi TC001), and sensors (SHT3x-D, SoMoSe, multi-point liquid level).
- Keeps Wi-Fi and Message Queuing Telemetry Transport (MQTT) credentials out of the repository:
  they're read from environment variables at compile time.

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

`common/gosund-sp111.yaml` in turn includes `common/base.yaml`. That package supplies Wi-Fi, the
native API that Home Assistant connects over, over-the-air updates, and diagnostic sensors. A new
plug is therefore ~10 lines instead of a full config.

### Use a package outside this repository

The packages assume this repository's layout: `common/gosund-sp111.yaml` includes
`common/base.yaml`, and the behaviour packages expect the substitutions that `base.yaml` defines.
Copying one file into another project means copying `common/base.yaml` with it and adapting those
substitutions. These fragments ship only as part of this repository, not as a versioned package.

### Prerequisites

- [Docker](https://docs.docker.com/get-docker/): ESPHome runs through `ghcr.io/esphome/esphome`,
  so you don't need a local Python install.
- [go-task](https://taskfile.dev/): the task runner (`task -l` lists everything).
- [`pass`](https://www.passwordstore.org/): supplies Wi-Fi secrets to the build
  (`network/wifi/ssid`, `network/wifi/password`).
- `jq`: only for the Ansible / NetBox inventory path below.
- The target device on `/dev/ttyUSB0` for the first, serial flash; later updates go over the air.

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
variables from `pass`. Review `.taskfiles/Taskfile_esphome.yml` before the first run: the `pass`
entry paths (`network/wifi/ssid`, `network/wifi/password`), the `Europe/Berlin` timezone, and the
`/dev/ttyUSB0` serial device are hard-wired there. A different password manager means replacing
those `pass` calls.

### Flashing from an Ansible / NetBox inventory

Optional, and only relevant if you already keep your devices in a NetBox inventory. Instead of
naming the config file and the target address by hand, an Ansible inventory pulls both from the
device's NetBox record:

```sh
export ANSIBLE_INVENTORY=~/repos/github/argo-charts/src/applications/netbox/configuration/inventory/inventory.yaml
export DEVICE_NAME="nous-a1t-08"

task esphome:run \
  DEVICE_FILE="$(jq -r '"\(.local_context_data[0].esphome.config) --device=\(.ansible_host)"' <<< $(ansible-inventory -i $ANSIBLE_INVENTORY --list | jq '._meta.hostvars["$DEVICE_NAME"]' -r))"
```

## Documentation

Per-device and per-package documentation lives under `docs/`, and MkDocs builds the site:

```sh
task mkdocs:start   # live preview on http://localhost:8002
```

The `name` and `id` substitutions of a device determine the entity IDs Home Assistant sees.
Renaming them breaks every automation and dashboard that references the old IDs, so treat them as a
published contract rather than a local detail.

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

Personal, actively maintained collection. Device configs arrive and change along with the hardware
setup. Entries under `src/poc/` and `src/archive/` are experimental or retired rather than
production-ready.
