# NOUS A1T

**Package:** `src/common/nous-a1t.yaml`  
**Chip:** ESP8285  
**Energy chip:** HLW8012  
**ESPHome device database:** https://devices.esphome.io/devices/Nous-A1t

For shared behaviour across all smart plug packages - relay pattern, button wiring,
energy measurement and `restore_mode` - see the [Smart Plugs index](index.md).

---

## GPIO Mapping

| GPIO   | Direction | Function    | Notes                                       |
| ------ | --------- | ----------- | ------------------------------------------- |
| GPIO00 | Input     | Button      | `INPUT_PULLUP`, active LOW (not inverted)   |
| GPIO12 | Output    | HLW8012 SEL | Inverted - selects current vs. voltage mode |
| GPIO13 | Output    | LED         | Inverted - HIGH = off                       |
| GPIO04 | Input     | HLW8012 CF  | Active power frequency signal               |
| GPIO05 | Input     | HLW8012 CF1 | Current / voltage frequency signal          |
| GPIO14 | Output    | Relay       | HIGH = on                                   |

---

## LED

The A1T has a single LED, configured as a `light` component using the `status_led` platform:

```yaml
light:
  - platform: status_led
    id: ${id}_led
    restore_mode: ${restore_mode}
    pin:
      number: GPIO13
      inverted: true
```

Using `light` instead of a bare `status_led` or `output` serves two purposes:

- The LED participates in the same `restore_mode` cycle as the relay - it restores its
  state correctly after a power cycle.
- The template switch can control the LED with `light.turn_on` / `light.turn_off` actions,
  keeping LED state in sync with the relay at all times.

---

## Relay & Switch Logic

The A1T template switch includes a **`lambda`** that reads the physical relay state directly,
rather than relying purely on optimistic tracking:

```yaml
switch:
  - platform: template
    id: "${id}_button_switch"
    optimistic: true
    restore_mode: ${restore_mode}
    lambda: |-
      if (id(${id}_relay).state) {
        return true;
      } else {
        return false;
      }
    turn_on_action:
      - switch.turn_on: ${id}_relay
      - light.turn_on: ${id}_led
    turn_off_action:
      - switch.turn_off: ${id}_relay
      - light.turn_off: ${id}_led
```

The lambda ensures the switch state shown in Home Assistant always reflects the actual relay
state - even after a reboot where the relay might have restored to a different state than
what the template switch would have assumed on its own.

The hardware relay (`${id}_relay`) uses a **scoped ID** (prefixed with `${id}_`) unlike the
Gosund SP111, where the relay ID is simply `relay`. This matters when combining multiple
packages in a single device file, as it avoids ID collisions.

---

## Chip Configuration

```yaml
esp8266:
  board: esp8285
  restore_from_flash: true
  early_pin_init: False
```

- **`restore_from_flash: true`** - Persists the last relay state to flash memory so it can
  be restored after a power cycle. This is what makes `RESTORE_DEFAULT_OFF` and
  `RESTORE_DEFAULT_ON` work correctly on this device.
- **`early_pin_init: False`** - Disables GPIO initialisation before the SDK boots. Without
  this, GPIO14 (the relay) might briefly pulse during boot, causing an unwanted relay click.

---

## HLW8012 Pin Assignment

The A1T uses the default pin assignment from `sensor/hlw8012.yaml` without overrides:

```yaml
hlw8012: !include ./sensor/hlw8012.yaml
# uses defaults: cf_pin: GPIO04, cf1_pin: GPIO05
```

This is the inverse of the Gosund SP111, which swaps CF and CF1.

---

## Differences from Gosund SP111

| Aspect               | NOUS A1T                    | Gosund SP111                               |
| -------------------- | --------------------------- | ------------------------------------------ |
| Relay GPIO           | GPIO14                      | GPIO15                                     |
| Relay ID             | `${id}_relay` (scoped)      | `relay` (global)                           |
| Button GPIO          | GPIO00 (not inverted)       | GPIO13 (inverted)                          |
| LED setup            | Single `light` (status_led) | Dual: `status_led` (blue) + `output` (red) |
| Template switch      | `lambda` + optimistic       | optimistic only                            |
| LED restore          | follows `restore_mode`      | not applicable (status_led)                |
| HLW8012 CF           | GPIO04 (default)            | GPIO05 (override)                          |
| HLW8012 CF1          | GPIO05 (default)            | GPIO04 (override)                          |
| `restore_from_flash` | `true`                      | not set                                    |
| `early_pin_init`     | `false`                     | not set                                    |
| Logger serial        | enabled (default)           | disabled (`baud_rate: 0`)                  |
