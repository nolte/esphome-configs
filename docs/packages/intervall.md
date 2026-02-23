# Interval Switch

**Package:** `src/common/switch-intervall.yaml`

```yaml
# src/common/switch-intervall.yaml
defaults:
  restore_mode: RESTORE_DEFAULT_OFF
  intervall_duration: "10"
  intervall_repeat: "1"
  intervall_duration_min_value: "0"
  intervall_duration_max_value: "30"
  intervall_duration_step: "5"
  intervall_repeat_min_value: "0"
  intervall_repeat_max_value: "5"
  intervall_repeat_step: "1"
  icon: "mdi:water-pump"
```

---

## Purpose

Extends a device with a repeating on/off cycle for `${id}_button_switch`. Designed for
pump control — turn the pump on for a set duration, off for a pause, and repeat N times.
All parameters are adjustable at runtime from Home Assistant and survive reboots.

---

## Defaults

| Variable                       | Default               | Description                         |
| ------------------------------ | --------------------- | ----------------------------------- |
| `restore_mode`                 | `RESTORE_DEFAULT_OFF` | State of both switches after reboot |
| `intervall_duration`           | `10`                  | Initial on-time in seconds          |
| `intervall_repeat`             | `1`                   | Initial number of repetitions       |
| `intervall_duration_min_value` | `0`                   | Slider minimum for duration         |
| `intervall_duration_max_value` | `30`                  | Slider maximum for duration         |
| `intervall_duration_step`      | `5`                   | Slider step size for duration       |
| `intervall_repeat_min_value`   | `0`                   | Slider minimum for repeat count     |
| `intervall_repeat_max_value`   | `5`                   | Slider maximum for repeat count     |
| `intervall_repeat_step`        | `1`                   | Slider step size for repeat count   |
| `icon`                         | `mdi:water-pump`      | Icon for buttons                    |

---

## Exposed Entities

### Switches

| ID                        | Name                          | Description                                                                                  |
| ------------------------- | ----------------------------- | -------------------------------------------------------------------------------------------- |
| `${id}_automation_active` | `${name} - automation active` | Enables the cycle. Turning it **off** immediately calls `${id}_intervall_stop`.              |
| `${id}_intervall_pump`    | `${name} - Intervall Pump`    | Reflects script state via lambda. Turning **on** starts the cycle; turning **off** stops it. |

### Numbers

| ID                         | Name                           | Unit  | Description                                                        |
| -------------------------- | ------------------------------ | ----- | ------------------------------------------------------------------ |
| `${id}_intervall_duration` | `${name} - Intervall Duration` | sec   | On-time per iteration. Pause between iterations is `duration × 2`. |
| `${id}_intervall_repeat`   | `${name} - Repeat Count`       | steps | Total number of on/off iterations to run.                          |
| `${id}_repeat_counter`     | `${name} - Repeat Counter`     | step  | Read-only current iteration index, reset to `0` on stop.           |

### Buttons

| ID                    | Name                  | Description                                                |
| --------------------- | --------------------- | ---------------------------------------------------------- |
| `${id}_intervall_btn` | `${name} - Intervall` | Starts one full cycle. Does not block further interaction. |
| `${id}_stop`          | `${name} - Stop`      | Stops any running cycle immediately.                       |

### Binary Sensor

| ID                             | Name              | Description                                       |
| ------------------------------ | ----------------- | ------------------------------------------------- |
| `${id}_intervall_plump_status` | `Intervall Plump` | `true` while `${id}_intervall` script is running. |

---

## Cycle Execution Flow

One complete cycle runs `repeat_count` iterations of this pattern:

```
for iteration 0 … repeat_count - 1:
  if automation_active:
    repeat_counter = iteration
    switch.turn_on:  ${id}_button_switch
    delay: duration seconds
    switch.turn_off: ${id}_button_switch
    delay: duration × 2 seconds   ← skipped on the last iteration
```

The pause between iterations is always twice the on-time. The final iteration has no trailing
pause — the cycle ends immediately after the last switch-off.

### Stop behaviour

`${id}_intervall_stop` can be called at any point:

```
script.stop: ${id}_intervall
switch.turn_off: ${id}_button_switch
number.set: ${id}_repeat_counter → 0
```

It cancels the running script, ensures the switch is off, and resets the counter.
This script is shared by the Stop button, the `automation_active` turn-off action, and the
`${id}_intervall_pump` turn-off action.

---

## `automation_active` Guard

The `${id}_automation_active` switch does not pause the cycle mid-run — it guards each
individual iteration. If `automation_active` is turned off while a cycle is running, the
current iteration completes and subsequent iterations are skipped because the lambda
condition fails.

Turning `automation_active` **off** also explicitly calls `${id}_intervall_stop`, which
cancels any running script and ensures the switch is left in the off state.

---

## Related

|                                |                                                     |
| ------------------------------ | --------------------------------------------------- |
| `src/common/nous-a1t.yaml`     | Defines `${id}_button_switch` this package controls |
| `src/common/gosund-sp111.yaml` | Defines `${id}_button_switch` this package controls |
