# Timed

**Package:** `src/common/active-duration.yaml`

```yaml
# src/common/active-duration.yaml
defaults:
  time_start: 07:00:00
  time_end: 01:00:00
  timezone: Europe/Berlin
  restore_mode_automation: RESTORE_DEFAULT_ON
```

---

## Purpose

Switches `${id}_button_switch` on at a configured start time and off at a configured end time,
every day. Both times are editable at runtime from Home Assistant and survive device reboots.
The automation can be paused without changing the schedule.

---

## Defaults

| Variable                  | Default              | Description                             |
| ------------------------- | -------------------- | --------------------------------------- |
| `time_start`              | `07:00:00`           | Initial switch-on time                  |
| `time_end`                | `01:00:00`           | Initial switch-off time                 |
| `timezone`                | `Europe/Berlin`      | Timezone for SNTP clock                 |
| `restore_mode_automation` | `RESTORE_DEFAULT_ON` | State of the enable switch after reboot |

---

## Exposed Entities

### `${id}_automation_active` (switch)

Enables or disables the automation without touching the schedule. When off, the `on_time`
triggers still fire but the scripts take no action.

### `${name} - Start` (Datetime entity)

Stores the switch-on time. Editable from Home Assistant. Fires `${id}_intervall_start`
every day when the device clock reaches this value.

### `${name} - End` (Datetime entity)

Stores the switch-off time. Editable from Home Assistant. Fires `${id}_intervall_stop`
every day when the device clock reaches this value.

---

## Execution Flow

```
clock reaches Start time
  └── on_time → script: ${id}_intervall_start
        └── if automation_active → switch.turn_on: ${id}_button_switch

clock reaches End time
  └── on_time → script: ${id}_intervall_stop
        └── if automation_active → switch.turn_off: ${id}_button_switch
```

The scripts act as a guard layer—disabling `automation_active` prevents any switch action
without removing or changing the scheduled times.

---

## Time Source

The package brings its own SNTP time source, independent of `common/time.yaml`:

```yaml
time:
  - platform: sntp
    id: ${id}_sntp_time
    timezone: ${timezone}
    servers:
      - 0.pool.ntp.org
      - 1.pool.ntp.org
      - 2.pool.ntp.org
```

The `datetime` entities with `on_time` automatically bind to this time source via
`${id}_sntp_time`. The package works without a Home Assistant connection, as long as
internet access is available for NTP.

---

## Midnight Crossover

Start and end times are handled as independent daily triggers. A window that spans midnight
(for example `time_start: 22:00:00`, `time_end: 06:00:00`) works correctly: the device
switches on at 22:00 and off at 06:00 the following morning.

---

## Related

|                                |                                                          |
| ------------------------------ | -------------------------------------------------------- |
| `src/common/time.yaml`         | Separate HA-based time source—not used by this package |
| `src/common/nous-a1t.yaml`     | Defines `${id}_button_switch` this package controls      |
| `src/common/gosund-sp111.yaml` | Defines `${id}_button_switch` this package controls      |
