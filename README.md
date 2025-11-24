# ESPHome Config

Central Place for [esphome.io](https://esphome.io/index.html) config elements. Small reuseable  
yaml structrure for create a Self Hosted DIY IOT device plattform, this allow for example same named sensores over different device types. With focus for  "Don`t repeat yourself" coding style. 

## Structure

| **Folder**          | **Description**                                                                                |
|---------------------|------------------------------------------------------------------------------------------------|
| `src/*.yaml`        | Each device as single yaml for supporte a delarative SetUp of different types of IOT devices.  |
| `docs`              | MkDocs Structure for generate Documentation with relevant informations.                        |
| `src/common/*.yaml` | Reuseable Modules for different Usecases, Devices and Sensore Types.                                                     |


## Usefull Commands

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

Load the device config by Ansible Inventory, given configs will be presented by Local Running Netbox Deployment. 

```sh
export ANSIBLE_INVENTORY=~/repos/github/argo-charts/src/applications/netbox/configuration/inventory/inventory.yaml

export DEVICE_NAME="nous-a1t-08"

task esphome:run \
  DEVICE_FILE="$(jq -r '"\(.local_context_data[0].esphome.config) --device=\(.ansible_host)"' <<< $(ansible-inventory -i $ANSIBLE_INVENTORY --list | jq '._meta.hostvars["$DEVICE_NAME"]' -r))"
```

## Project Structure


* [BeFlE/SoMoSe](https://github.com/BeFlE/SoMoSe)for good Moisture sensores
* [lubeda/EspHoMaTriXv2](https://github.com/lubeda/EspHoMaTriXv2) better Pixel Matrix Support.