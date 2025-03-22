# ESPHome Config

Central Place for [esphome.io](https://esphome.io/index.html) config elements.


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
