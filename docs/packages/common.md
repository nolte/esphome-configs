# Common Configuration

**Package:** `src/common/base.yaml`

```yaml
# src/common/base.yaml
defaults:
  project_name: "NOUS.Smart-Wifi-Socket"
  project_version: "A1T"
```

---

## Purpose

The shared foundation for every device in this repository. Configures connectivity,
remote access, monitoring and a standard set of diagnostic sensors. All hardware profiles
(`nous-a1t.yaml`, `gosund-sp111.yaml`) include this package as their first dependency.

---

## Required Substitutions

These must be defined in the device file before including any package that pulls in `base.yaml`:

| Variable  | Description                                            | Example              |
| --------- | ------------------------------------------------------ | -------------------- |
| `name`    | Device hostname — used as ESPHome name and DNS address | `nous-a1t-01`        |
| `comment` | Free-text description — shown in HA device info        | `"Circulation pump"` |

---

## Defaults (package vars)

| Variable          | Default                  | Description                        |
| ----------------- | ------------------------ | ---------------------------------- |
| `project_name`    | `NOUS.Smart-Wifi-Socket` | ESPHome creator project identifier |
| `project_version` | `A1T`                    | Project version string             |

Hardware profiles override these when including `base.yaml`:

```yaml
# inside nous-a1t.yaml
packages:
  base: !include
    file: ./base.yaml
    vars:
      project_name: "NOUS.Smart-Wifi-Socket"
      project_version: "A1T"
```

---

## ESPHome Core

```yaml
esphome:
  name: ${name}
  comment: ${comment}
  name_add_mac_suffix: false
  project:
    name: ${project_name}
    version: ${project_version}
```

`name_add_mac_suffix: false` — devices use fixed, human-readable hostnames. Appending the
MAC suffix would break the `use_address` DNS resolution and OTA targeting by name.

---

## Wi-Fi

```yaml
wifi:
  domain: !env_var WIFI_DOMAIN
  ssid: !env_var WIFI_SSID
  password: !env_var WIFI_PASSWORD
  use_address: ${name}.fritz.box
  ap:
    ssid: "Livingroom Fallback Hotspot"
    password: !env_var WIFI_FALLBACK_PASSWORD
```

All credentials are loaded from environment variables at compile time — no secrets are
stored in the repository.

`use_address: ${name}.fritz.box` overrides the connection target for ESPHome CLI and OTA.
This requires the Fritz!Box to resolve devices by hostname, which works automatically with
DHCP leases. The `.fritz.box` suffix itself also comes from `WIFI_DOMAIN`.

### Fallback AP

When the device cannot reach the configured network, it opens a Wi-Fi access point named
`Livingroom Fallback Hotspot`. The `captive_portal` component activates automatically
alongside the fallback AP, providing a browser-based interface for Wi-Fi provisioning.

### Environment Variables

| Variable                 | Used for                             |
| ------------------------ | ------------------------------------ |
| `WIFI_SSID`              | Network name                         |
| `WIFI_PASSWORD`          | Network password                     |
| `WIFI_DOMAIN`            | Local DNS domain (e.g. `.fritz.box`) |
| `WIFI_FALLBACK_PASSWORD` | Fallback AP password                 |

---

## Services

| Component        | Configuration     | Notes                                      |
| ---------------- | ----------------- | ------------------------------------------ |
| `logger`         | defaults          | Serial logging at INFO level               |
| `api`            | no encryption     | Native API for Home Assistant — no key set |
| `ota`            | `platform: esphome` | No password set                            |
| `web_server`     | defaults          | Local UI at port 80                        |
| `prometheus`     | —                 | Metrics endpoint at `/metrics`             |
| `captive_portal` | —                 | Active only when fallback AP is running    |

> **Note on security:** `api` and `ota` run without encryption or password. This is
> intentional for a trusted local network, but should be considered before exposing
> devices to untrusted segments.

### MQTT

MQTT is present in the source but fully commented out. The native `api` is used instead.
The commented block documents the required env vars if MQTT is ever needed:

```yaml
# mqtt:
#   broker: !env_var MQTT_ENDPOINT
#   port:   !env_var MQTT_PORT
#   username: !env_var MQTT_USERNAME
#   password: !env_var MQTT_PASSWORD
```

---

## Included Sensors

All sensors are pulled in via `<<: !include` snippets from `src/common/`:

| Snippet                             | Entity name                           | Type            | Update    |
| ----------------------------------- | ------------------------------------- | --------------- | --------- |
| `binary_sensor/status.config.yaml`  | `${name} - Status`                    | `binary_sensor` | on change |
| `sensor/uptime.config.yaml`         | `${name} - Uptime`                    | `sensor` (s)    | 60 s      |
| `sensor/wifi_signal.config.yaml`    | `${name} - WiFi Signal`               | `sensor` (dBm)  | 60 s      |
| `text_sensor/wifi_info.config.yaml` | `${name} - IP Address` / SSID / BSSID | `text_sensor`   | on change |
| `text_sensor/version.config.yaml`   | `${name} - ESPHome Version`           | `text_sensor`   | on boot   |

---

## Related

|                                |                                                           |
| ------------------------------ | --------------------------------------------------------- |
| `src/common/nous-a1t.yaml`     | Includes this package                                     |
| `src/common/gosund-sp111.yaml` | Includes this package                                     |
| `src/common/time.yaml`         | Sibling package, included separately by hardware profiles |
