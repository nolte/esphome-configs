# ESPHome Config

A DRY, self-hosted collection of [ESPHome](https://esphome.io/) device configurations. Concrete
devices are tiny YAML files that compose reusable building blocks, so the same behaviour (Wi-Fi,
diagnostics, relays, timers, sensors) is defined once and shared across many devices.

## How it works

Every file under `src/*.yaml` is one **device**. A device file stays small: it sets a few
substitutions and pulls in **packages** from `src/common/` through ESPHome's
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
sensors). The result: add a new plug by writing ~10 lines instead of copying a full config.
Wi-Fi and MQTT credentials are never stored in the repo; they're read from environment variables
at compile time (for example `!env_var WIFI_SSID`).

## Repository structure

| Path                  | Contents                                                                       |
| --------------------- | ------------------------------------------------------------------------------ |
| `src/*.yaml`          | Device files, one per physical device, composed from packages                  |
| `src/common/*.yaml`   | Reusable packages: hardware profiles, behaviours, time, sensors                |
| `src/common/sensor/`, `binary_sensor/`, `text_sensor/` | Snippet-level sensor includes                 |
| `src/include/`        | C++ headers used by lambdas (for example `somose.h`)                           |
| `src/my_components/`  | Custom ESPHome external components (for example `somose`)                      |
| `src/poc/`            | Proof-of-concept configs, not production devices                               |
| `src/archive/`        | Retired configs kept for reference                                             |
| `docs/`               | MkDocs documentation source                                                    |
| `.taskfiles/`         | `go-task` definitions for compile and flash                                    |

## Supported hardware

- **Smart plugs**: Gosund SP111, NOUS A1T (relay plus HLW8012 energy metering)
- **Cameras**: ESP32-CAM modules
- **Displays and voice**: ESP32-S3-BOX-3 (voice assistant, pixel-art display), Ulanzi TC001
- **Sensors**: SHT3x-D (temperature and humidity), SoMoSe (soil moisture), multi-point liquid level

Reusable behaviour packages include `active-duration` (daily on/off schedule), `switch-intervall`
(repeating pump cycles), `timer-cancelable` (countdown), and `switch-kill-sensor` (cut power on a
Home Assistant binary sensor).

## Prerequisites

- [Docker](https://docs.docker.com/get-docker/): ESPHome runs through `ghcr.io/esphome/esphome`,
  so no local Python install is needed
- [go-task](https://taskfile.dev/): the task runner (`task -l` lists everything)
- [`pass`](https://www.passwordstore.org/): supplies Wi-Fi secrets to the build
  (`network/wifi/ssid`, `network/wifi/password`)
- An ESP device on `/dev/ttyUSB0` for the first, serial flash; later updates go over the air

## Quickstart

List the available tasks:

```sh
task -l
```

**Initial flash over USB** (for example when converting a Tasmota device to ESPHome). Compiles the
firmware to `/tmp/firmware.bin`:

```sh
task esphome:compile DEVICE_FILE=nous-a1t-09.yaml
```

**Run and flash a device** over serial via `/dev/ttyUSB0`:

```sh
task esphome:run DEVICE_FILE=nous-a1t-08.yaml
```

**Over-the-air update** of an already-provisioned device:

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

## Related projects

- [BeFlE/SoMoSe](https://github.com/BeFlE/SoMoSe): soil moisture sensor used by the `somose` component
- [lubeda/EspHoMaTriXv2](https://github.com/lubeda/EspHoMaTriXv2): pixel matrix support for the Ulanzi display
