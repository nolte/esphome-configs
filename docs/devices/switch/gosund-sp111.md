# Gosund SP111

**Package:** `src/common/gosund-sp111.yaml`  
**Chip:** ESP8285  
**Energy chip:** HLW8012  
**ESPHome device database:** https://devices.esphome.io/devices/Gosund-SP111

For shared behaviour across all smart plug packages - relay pattern, button wiring,
energy measurement and `restore_mode` - see the [Smart Plugs index](index.md).

---

## GPIO Mapping

| GPIO   | Direction | Function          | Notes                                       |
| ------ | --------- | ----------------- | ------------------------------------------- |
| GPIO00 | Output    | Red LED           | Inverted - HIGH = off                       |
| GPIO02 | Output    | Blue LED (status) | Inverted - HIGH = off                       |
| GPIO04 | Input     | HLW8012 CF1       | Current / voltage frequency signal          |
| GPIO05 | Input     | HLW8012 CF        | Active power frequency signal               |
| GPIO12 | Output    | HLW8012 SEL       | Inverted - selects current vs. voltage mode |
| GPIO13 | Input     | Button            | `INPUT_PULLUP`, inverted - LOW = pressed    |
| GPIO15 | Output    | Relay             | HIGH = on                                   |

---

## LEDs

The SP111 has two physically separate LEDs with distinct roles:

| ID         | Colour | Component       | Controlled by                                                             |
| ---------- | ------ | --------------- | ------------------------------------------------------------------------- |
| `led_blue` | Blue   | `status_led`    | ESPHome automatically - blinks during Wi-Fi connect, solid when API is up |
| `led_red`  | Red    | `output` (gpio) | Template switch `turn_on_action` / `turn_off_action`                      |

The blue LED requires no manual control. The red LED is toggled explicitly to mirror the
relay state:

```yaml
turn_on_action:
  - switch.turn_on: relay
  - output.turn_on: led_red
turn_off_action:
  - switch.turn_off: relay
  - output.turn_off: led_red
```

---

## Relay & Switch Logic

The SP111 template switch uses **optimistic mode only** - there is no `lambda` to read back
the hardware relay state. The relay ID is a global `relay` (not scoped with `${id}_`):

```yaml
switch:
  - platform: template
    id: ${id}_button_switch
    optimistic: true
    restore_mode: ${restore_mode}
    turn_on_action:
      - switch.turn_on: relay
      - output.turn_on: led_red
    turn_off_action:
      - switch.turn_off: relay
      - output.turn_off: led_red

  - platform: gpio
    restore_mode: ${restore_mode}
    pin: GPIO15
    id: relay
```

---

## HLW8012 Pin Assignment

The SP111 overrides the default CF / CF1 pin assignment:

```yaml
hlw8012: !include
  file: ./sensor/hlw8012.yaml
  vars:
    cf_pin: GPIO05
    cf1_pin: GPIO04
```

This is the inverse of the NOUS A1T, which uses the `hlw8012.yaml` defaults (CF=GPIO04, CF1=GPIO05).

---

## Chip Configuration

```yaml
esp8266:
  board: esp8285
  framework:
    version: recommended

logger:
  baud_rate: 0
```

- **`framework: version: recommended`** - Pins ESPHome to the recommended Arduino core
  version for this chip, avoiding regressions from automatic updates.
- **`baud_rate: 0`** - Disables serial logging. The TX/RX pins aren't accessible inside
  the SP111 enclosure, so serial output would be wasted and could interfere with HLW8012
  communication on shared pins.

---

## Differences from NOUS A1T

| Aspect               | Gosund SP111                               | NOUS A1T                    |
| -------------------- | ------------------------------------------ | --------------------------- |
| Relay GPIO           | GPIO15                                     | GPIO14                      |
| Relay ID             | `relay` (global)                           | `${id}_relay` (scoped)      |
| Button GPIO          | GPIO13 (inverted)                          | GPIO00 (not inverted)       |
| LED setup            | Dual: `status_led` (blue) + `output` (red) | Single `light` (status_led) |
| Template switch      | optimistic only                            | `lambda` + optimistic       |
| LED restore          | not applicable (status_led)                | follows `restore_mode`      |
| HLW8012 CF           | GPIO05 (override)                          | GPIO04 (default)            |
| HLW8012 CF1          | GPIO04 (override)                          | GPIO05 (default)            |
| `restore_from_flash` | not set                                    | `true`                      |
| `early_pin_init`     | not set                                    | `false`                     |
| Logger serial        | disabled (`baud_rate: 0`)                  | enabled (default)           |
