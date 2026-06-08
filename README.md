# ESPHome Config

Central place for [ESPHome](https://esphome.io/index.html) config elements. A small, reusable
YAML structure to create a self-hosted DIY IoT device platform. It allows, for example, the same
named sensors across different device types, with a focus on a "Don't repeat yourself" coding style.

## Structure

| **Folder**          | **Description**                                                                                |
|---------------------|------------------------------------------------------------------------------------------------|
| `src/*.yaml`        | Each device as a single YAML file to support a declarative setup of different types of IoT devices. |
| `docs`              | MkDocs structure to generate documentation with relevant information.                           |
| `src/common/*.yaml` | Reusable modules for different use cases, devices and sensor types.                             |


## Useful Commands

### Run

```sh
task esphome:run DEVICE_FILE=nous-a1t-08.yaml
```

for direct **ota** support use

```sh
task esphome:run DEVICE_FILE="nous-a1t-08.yaml --device=XXX.XXX.XXX.XXX"
```

### Compile
used for initial flashing. Mostly for switching Tasmota based devices into ESPHome.

```sh
task esphome:compile DEVICE_FILE=nous-a1t-09.yaml
```


### With Inventory

Load the device config by Ansible Inventory, given configs will be presented by a locally running NetBox deployment.

```sh
export ANSIBLE_INVENTORY=~/repos/github/argo-charts/src/applications/netbox/configuration/inventory/inventory.yaml

export DEVICE_NAME="nous-a1t-08"

task esphome:run \
  DEVICE_FILE="$(jq -r '"\(.local_context_data[0].esphome.config) --device=\(.ansible_host)"' <<< $(ansible-inventory -i $ANSIBLE_INVENTORY --list | jq '._meta.hostvars["$DEVICE_NAME"]' -r))"
```

## Project Structure


* [BeFlE/SoMoSe](https://github.com/BeFlE/SoMoSe) for good moisture sensors
* [lubeda/EspHoMaTriXv2](https://github.com/lubeda/EspHoMaTriXv2) better Pixel Matrix Support.
