# Time

**Package:** `src/common/time.yaml`

```yaml
# src/common/time.yaml
time:
  - platform: homeassistant
    id: ${id}_homeassistant_time
```

---

## Purpose

Synchronises the device clock with Home Assistant over the existing native API connection.
A timezone-aware time source becomes available on the device under the scoped ID
`${id}_homeassistant_time`.

---

## How It Works

The `homeassistant` platform piggybacks on the already-established native API connection —
no additional network configuration is required. Home Assistant pushes the current time to
the device periodically (default: every 15 minutes) and immediately after the API connection
is established.

If the connection is lost, the device continues running from its last known time until
synchronisation can resume.

> This platform requires the native API connection to Home Assistant.
> Devices using MQTT only cannot use this time source.

---

## ID Scoping

The time component ID is prefixed with `${id}_` to avoid conflicts when multiple packages
are merged into a single device firmware:

```yaml
id: ${id}_homeassistant_time
# resolves to e.g.: nous_a1t_01_homeassistant_time
```

---

## Related

|               |                                                                                   |
| ------------- | --------------------------------------------------------------------------------- |
| Upstream docs | [ESPHome: Home Assistant Time](https://esphome.io/components/time/homeassistant/) |
