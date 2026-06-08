# Kill sensor

**Package:** `src/common/switch-kill-sensor.yaml`

```yaml
# src/common/switch-kill-sensor.yaml
defaults:
  kill_auto_restore: "false"
```

---

## Purpose

Turns a plug (`${id}_button_switch`) off the moment a Home Assistant binary sensor reports a
problem, for example a water-leak sensor under a washing machine. It's a safety layer you add
on top of a plug package such as [`gosund-sp111.yaml`](../devices/switch/gosund-sp111.md) or
[`nous-a1t.yaml`](../devices/switch/nous-a1t.md): the plug keeps working normally until the
watched sensor trips, and then it's cut immediately.

The state is cached on the device, so a short Home Assistant API outage doesn't re-enable the
plug on its own.

---

## Required variables

| Variable             | Default   | Description                                                                 |
| -------------------- | --------- | --------------------------------------------------------------------------- |
| `kill_sensor_entity` | *(none)*  | Home Assistant `entity_id` of the binary sensor to watch (for example `binary_sensor.sjcgq11lm_10_water_leak`). |
| `kill_auto_restore`  | `"false"` | `"true"` switches the plug back on once the sensor clears; `"false"` keeps it locked until a manual reset. |

---

## Exposed entities

### `${id}_kill_sensor` (binary sensor)

Mirrors the watched Home Assistant entity on the device. When it turns on, the plug is switched
off; when it clears, the behaviour depends on `kill_auto_restore`.

### `${name} - Kill Reset` (button, config)

Clears the kill state and switches the plug back on. The reset is refused while the sensor is
still active, so the plug can't be re-energised during an ongoing leak.

### `${name} - Kill Status` (text sensor, diagnostic)

Reports the current state: `OK`, `SENSOR AKTIV` (sensor currently tripped), or `GESPERRT`
(locked after a trip, waiting for a reset).

---

## Behaviour

```
HA binary sensor turns ON
  └── switch.turn_off ${id}_button_switch
        └── set ${id}_killed_by_sensor = true

HA binary sensor turns OFF
  ├── kill_auto_restore = true  → switch.turn_on ${id}_button_switch, clear flag
  └── kill_auto_restore = false → stay off, wait for the Kill Reset button
```

With `kill_auto_restore: "false"` (the default) the plug stays off after a trip even when the
sensor recovers. That's the safe choice for leak protection, where you want a human to check the
device before power returns.

---

## Usage

Include the package alongside the plug package and point it at the binary sensor to watch. Example
from `src/gosund-sp111-02.yaml`:

```yaml
packages:
  plug: !include
    file: common/gosund-sp111.yaml
  kill: !include
    file: common/switch-kill-sensor.yaml
    vars:
      kill_sensor_entity: binary_sensor.sjcgq11lm_10_water_leak
```

---

## Related

|                                |                                                          |
| ------------------------------ | -------------------------------------------------------- |
| `src/common/gosund-sp111.yaml` | Defines the `${id}_button_switch` this package controls  |
| `src/common/nous-a1t.yaml`     | Defines the `${id}_button_switch` this package controls  |
