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
    -it ghcr.io/esphome/esphome run cam-03.yaml
```



```sh
export DEVICE_NAME=nous-a1t-07

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
    -it ghcr.io/esphome/esphome compile $DEVICE_NAME.yaml

sudo mv /home/nolte/repos/github/esphome-configs/.esphome/build/$DEVICE_NAME/.pioenvs/$DEVICE_NAME/firmware.bin /tmp/ 
sudo chown nolte:nolte /tmp/firmware.bin
```