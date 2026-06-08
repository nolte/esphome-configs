# Smart plugs

This section covers the two supported smart plug hardware profiles. Both share the same
underlying architecture - this page documents everything they have in common.

Device-specific pages cover only what differs between them:

- [Gosund SP111](gosund-sp111.md)
- [NOUS A1T](nous-a1t.md)

---

## Shared Hardware

Both plug types use the **ESP8285** - an ESP8266 with 1 MB of flash integrated in the same
package - and the **HLW8012** chip for energy measurement. The chip is configured as a standard
`esp8266` target with `board: esp8285` in ESPHome.

---

## Relay & Switch Pattern

Both packages expose an identical two-layer switch structure:

```
${id}_button_switch  (switch.template)  ← exposed to Home Assistant
        │
        ├── turn_on_action  → switch.turn_on:  relay / ${id}_relay
        └── turn_off_action → switch.turn_off: relay / ${id}_relay
```

The **`gpio` switch** (the actual relay) is not exposed to Home Assistant directly. All control
goes through the **template switch**, which also drives the LED alongside the relay.

The template switch runs in **optimistic mode** - it assumes the command succeeded immediately
and updates its state in the UI without waiting for hardware confirmation.

---

## Button

Both packages wire the physical button to `switch.toggle` on the template switch:

```yaml
binary_sensor:
  - platform: gpio
    id: ${id}_button_state
    on_press:
      - switch.toggle: "${id}_button_switch"
```

The button binary sensor is internal - it is not exposed to Home Assistant.

---

## Energy Measurement

Power measurement is handled by `src/common/sensor/hlw8012.yaml`. The HLW8012 chip
alternates between current and voltage measurement on CF1, switched by the SEL pin.
All three sensors update every 3 seconds; daily energy resets at midnight.

| Sensor       | Unit | Update         |
| ------------ | ---- | -------------- |
| Current      | A    | 3 s            |
| Voltage      | V    | 3 s            |
| Power        | W    | 3 s            |
| Daily energy | kWh  | midnight reset |

### Calibration

Both packages share the same calibration defaults:

| Variable      | Default   | Effect                                                 |
| ------------- | --------- | ------------------------------------------------------ |
| `current_res` | `0.00280` | Shunt resistor - higher → lower current / watt reading |
| `voltage_div` | `775`     | Voltage divider - lower → lower voltage reading        |

These can be overridden per device as `vars` when including the package.

> ⚠️ **Safety:** The HLW8012 circuit references mains voltage on its digital GND.
> Never touch the GPIO pins while the device is plugged into mains power.

---

## `restore_mode`

Both packages accept a `restore_mode` variable that controls relay behaviour after a power
cycle. The same value is applied to both the template switch and the hardware relay.

| Value                 | Behaviour after restart                             |
| --------------------- | --------------------------------------------------- |
| `RESTORE_DEFAULT_OFF` | Restore last known state; default to off if unknown |
| `RESTORE_DEFAULT_ON`  | Restore last known state; default to on if unknown  |
| `ALWAYS_OFF`          | Always switch off regardless of previous state      |
| `ALWAYS_ON`           | Always switch on regardless of previous state       |

Default for both packages: `RESTORE_DEFAULT_OFF`

---

## Included Sub-packages

Both hardware profiles include the same three sub-packages:

| Package                      | Purpose                                   |
| ---------------------------- | ----------------------------------------- |
| `common/base.yaml`           | Wi-Fi, API, OTA, web server, base sensors |
| `common/time.yaml`           | Home Assistant time synchronisation       |
| `common/sensor/hlw8012.yaml` | HLW8012 energy measurement                |
