# ESPHome Config

Central Place for [esphome.io](https://esphome.io/index.html) config elements.



## Local Development


```sh
docker run \
    --rm --privileged \
    -v "${PWD}":/config \
    -e WIFI_DOMAIN=".fritz.box" \
    -e WIFI_SSID="$(pass network/wifi/ssid)" \
    -e WIFI_PASSWORD="$(pass network/wifi/password)" \
    -e WIFI_FALLBACK_PASSWORD="$(pass network/wifi/password)" \
    -e MQTT_ENDPOINT="192.168.178.23" \
    -e MQTT_PORT="31883" \
    -e MQTT_USERNAME="esphome" \
    -e MQTT_PASSWORD="notset" \
    --device=/dev/ttyUSB0 \
    -it ghcr.io/esphome/esphome run poc-esp32-dht.yaml
```