---
title: "Timer Plug Package"
description: "Reusable ESPHome timer package for smart plugs with Home Assistant integration."
---

# Timer Plug Package

The `timer-plug` package adds a flexible countdown timer to any ESPHome smart plug. It is designed
to be hardware-agnostic and works alongside existing plug packages (e.g. `gosund-sp111.yaml`).

The timer is controlled entirely on the ESP — it continues to run even when Home Assistant is offline.

## Features

- **Configurable countdown** via a Home Assistant number entity (seconds, persistent in flash)
- **Decoupled switch** — the relay can be toggled independently from the timer
- **Long press** on the physical button starts the timer
- **Short press** toggles the relay without touching the timer
- **Water leak safety** — any Home Assistant binary sensor can cut power immediately
- **Timer Active** binary sensor for use in HA automations
- **Timer Status** text sensor showing current state

## File Locations

```text
src/
├── common/
│   ├── gosund-sp111.yaml      # Hardware package (example)
│   └── timer-plug.yaml        # ← This package
└── gosund-sp111-timer.yaml    # Device file (example)
```

## Behavior Reference

### Switch

| Action            | Result                               |
| ----------------- | ------------------------------------ |
| `switch.turn_on`  | Relay ON — **no timer started**      |
| `switch.turn_off` | Relay OFF + cancels timer if running |

The switch and timer are intentionally decoupled. Turning the switch on manually does **not**
start a countdown. Use the "Timer started" button or a long press on the physical button to
start the timer.

### Physical Button

| Action            | Result                             |
| ----------------- | ---------------------------------- |
| Short press (<1s) | Toggle relay (no timer)            |
| Long press (>1s)  | Start timer → relay ON + countdown |

### Water Leak Sensor

When the configured Home Assistant binary sensor transitions to `on`, the relay is switched off
immediately — regardless of whether the timer is running or the relay was switched on manually.

!!! note
The water leak sensor requires an active Home Assistant API connection. If HA is offline,
the last known state is retained on the ESP.

## Package Parameters

All parameters are optional. Defaults are designed for the Gosund SP111 plug.

| Parameter           | Default               | Description                                         |
| ------------------- | --------------------- | --------------------------------------------------- |
| `timer_switch_id`   | `${id}_button_switch` | ID of the template switch from the hardware package |
| `timer_button_id`   | `${id}_button_state`  | ID of the physical button binary sensor             |
| `timer_min`         | `300`                 | Minimum timer duration in seconds (5 min)           |
| `timer_max`         | `2400`                | Maximum timer duration in seconds (40 min)          |
| `timer_initial`     | `900`                 | Default timer duration in seconds (15 min)          |
| `water_leak_entity` | `binary_sensor.none`  | Home Assistant entity ID of the water leak sensor   |

## Home Assistant Entities

The following entities are exposed to Home Assistant after integration:

| Entity                     | Type            | Description                             |
| -------------------------- | --------------- | --------------------------------------- |
| `{name} - Timer Dauer`     | `number`        | Duration slider in seconds              |
| `{name} - Timer Starten`   | `button`        | Starts the timer (relay ON + countdown) |
| `{name} - Timer Abbrechen` | `button`        | Stops the timer and switches relay OFF  |
| `{name} - Timer Aktiv`     | `binary_sensor` | `on` while countdown is running         |
| `{name} - Timer Status`    | `text_sensor`   | Human-readable status string            |

## Usage

### Minimal — Gosund SP111 with water leak sensor

```yaml
# src/gosund-sp111-timer.yaml

substitutions:
  name: gosund-sp111-timer
  id: gosund_sp111_timer
  comment: "Timer Plug"

packages:
  plug: !include
    file: common/gosund-sp111.yaml
    vars:
      restore_mode: RESTORE_DEFAULT_OFF

  timer: !include
    file: common/timer-plug.yaml
    vars:
      water_leak_entity: binary_sensor.sjcgq11lm_09_water_leak
```

### Custom duration range

```yaml
timer: !include
  file: common/timer-plug.yaml
  vars:
    water_leak_entity: binary_sensor.sjcgq11lm_09_water_leak
    timer_min: "60" # 1 minute minimum
    timer_max: "3600" # 60 minutes maximum
    timer_initial: "1800" # 30 minutes default
```

### Different plug hardware

As long as the hardware package exposes a template switch and a button binary sensor,
the `timer-plug` package can be used with any plug. Override `timer_switch_id` and
`timer_button_id` if the IDs differ from the SP111 convention:

```yaml
substitutions:
  name: nous-a1t-timer
  id: nous_a1t_timer
  comment: "Timer Plug Living Room"

packages:
  plug: !include
    file: common/nous-a1t.yaml

  timer: !include
    file: common/timer-plug.yaml
    vars:
      timer_switch_id: "nous_a1t_timer_button_switch"
      timer_button_id: "nous_a1t_timer_button_state"
      water_leak_entity: binary_sensor.sjcgq11lm_03_water_leak
      timer_max: "3600"
```

## Package Source

```yaml title="src/common/timer-plug.yaml"
defaults:
  timer_switch_id: "${id}_button_switch"
  timer_button_id: "${id}_button_state"
  timer_min: "300"
  timer_max: "2400"
  timer_initial: "900"
  water_leak_entity: "binary_sensor.none"

globals:
  - id: timer_running
    type: bool
    restore_value: no
    initial_value: "false"

number:
  - platform: template
    id: timer_duration
    name: "${name} - Timer Dauer"
    icon: mdi:timer-outline
    unit_of_measurement: "s"
    min_value: ${timer_min}
    max_value: ${timer_max}
    step: 1
    initial_value: ${timer_initial}
    restore_value: true
    optimistic: true

script:
  - id: timer_script
    mode: restart
    then:
      - globals.set:
          id: timer_running
          value: "true"
      - switch.turn_on: ${timer_switch_id}
      - logger.log:
          format: "Timer started: %.0f seconds"
          args: ["id(timer_duration).state"]
      - delay: !lambda |-
          return (uint32_t)(id(timer_duration).state * 1000.0f);
      - globals.set:
          id: timer_running
          value: "false"
      - logger.log: "Timer elapsed – relay OFF"
      - switch.turn_off: ${timer_switch_id}

switch:
  - id: !extend ${timer_switch_id}
    turn_off_action:
      - script.stop: timer_script
      - globals.set:
          id: timer_running
          value: "false"

binary_sensor:
  - id: !extend ${timer_button_id}
    on_click:
      - min_length: 1000ms
        max_length: 10000ms
        then:
          - logger.log: "Timer started via long press"
          - script.execute: timer_script

  - platform: homeassistant
    id: water_leak_sensor
    entity_id: ${water_leak_entity}
    internal: true
    on_press:
      - logger.log: "Water leak detected – relay OFF!"
      - switch.turn_off: ${timer_switch_id}

  - platform: template
    id: timer_active
    name: "${name} - Timer Aktiv"
    icon: mdi:timer-check-outline
    device_class: running
    lambda: |-
      return id(timer_running);

button:
  - platform: template
    name: "${name} - Timer Starten"
    icon: mdi:timer-play-outline
    on_press:
      - script.execute: timer_script

  - platform: template
    name: "${name} - Timer Abbrechen"
    icon: mdi:timer-off-outline
    on_press:
      - switch.turn_off: ${timer_switch_id}

text_sensor:
  - platform: template
    name: "${name} - Timer Status"
    icon: mdi:information-outline
    lambda: |-
      if (id(timer_running)) {
        char buf[48];
        int sek = (int)id(timer_duration).state;
        snprintf(buf, sizeof(buf), "Running (%d min %02d s)", sek / 60, sek % 60);
        return std::string(buf);
      } else if (id(relay).state) {
        return std::string("On (no timer)");
      } else {
        return std::string("Off");
      }
    update_interval: 10s
```

## See Also

- [ESPHome Packages](https://esphome.io/components/packages)
- [ESPHome Substitutions](https://esphome.io/components/substitutions)
- [Gosund SP111 Package](gosund-sp111.md)
