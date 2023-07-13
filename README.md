# ESPHome Config

Central Place for [esphome.io](https://esphome.io/index.html) config elements.

## Local Development


```sh
docker run \
    --rm --privileged \
    -v "${PWD}":/config \
    -e ...
    -e ...
    -e ...
    --device=/dev/ttyUSB0 \
    -it ghcr.io/esphome/esphome run livingroom.yaml
```