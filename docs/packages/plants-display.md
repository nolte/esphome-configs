# Plants Display

**Package:** `src/common/plants-display.yaml`

Rotates through your house plants on the ESP32-S3-BOX-3 panel: one plant per screen, with a
96 px state glyph for how thirsty it is, its next watering time, an alert line for open
tasks or overwintering, and a position indicator. Home Assistant owns the plant list; the
device owns the rotation.

The screen follows `spec/ha/esp32-s3-box-display-design`: colours come from the eleven named
Home Assistant dark palette roles, type from the four-step Roboto scale, icons from MDI at
the sanctioned sizes. No state is distinguished by colour alone.

Designed as the physical front end for the
[Kamerplanter integration](https://github.com/nolte/kamerplanter-ha), which already models
plants, care profiles, and tasks as Home Assistant entities. The package deliberately does
**not** know about Kamerplanter — the device takes pre-formatted strings and the mapping
lives in one Home Assistant automation, so any other plant source works just as well.

---

## Purpose

Turns the idle screen of an S3-BOX-3 voice satellite into a plant dashboard. Every other
screen — wake word, timers, pixel art, the degraded no-Wi-Fi / no-Home-Assistant states —
keeps working unchanged and takes precedence. Plants are what the box shows when it has
nothing else to say.

Kamerplanter ships Lovelace cards for the same data; this is the ambient counterpart — the
number you see from across the room without opening a dashboard.

---

## Composition

The package is additive and goes **next to** `common/esp32-s3-box-3-draw.yaml`, not inside
it. It relies on that package for the panel, the `casita_idle` image, and the `draw_display`
script. Colours, fonts, and icons it brings itself — the Casita screens next to it keep
their own Figtree fonts and per-phase background colours, untouched.

```yaml
# src/esp32-s3-box-3-02.yaml
substitutions:
  name: esp32-s3-box-3-02
  id: esp32_s3_box_3_02
  comment: "esp32-s3-box-3 02"
  idle_screen_script: show_plants_screen   # <- routes idle to the plant screen

packages:
  plug: !include
    file: common/esp32-s3-box-3-draw.yaml
  plants: !include
    file: common/plants-display.yaml
```

`idle_screen_script` is the extension point `esp32-s3-box-3-draw.yaml` exposes. Without it
the box keeps showing the Casita idle illustration, which is what `-01` still does.

Note that this sits **on top of a working voice satellite**: wake word, Assist, timers, and
pixel art keep their own screens and take precedence. The plants only fill the idle state,
so one box does both. `src/esp32-s3-box-3-03.yaml` holds the same composition as a
standalone device file, for a box dedicated to the display.

---

## Substitutions

| Variable                  | Default | Description                                                     |
| ------------------------- | ------- | --------------------------------------------------------------- |
| `plant_rotation_interval` | `60s`   | Auto-advance interval. A tap on the panel resets it.            |
| `plant_max_slots`         | `16`    | Upper bound on `total`, guards against a bad service call.      |
| `plant_dots_max`          | `12`    | Above this many plants the position shows as `3/14` instead of dots. |
| `plant_name_chars`        | `16`    | Byte budget for the plant name (`f_display` 28 px in 280 px).   |
| `plant_watering_chars`    | `16`    | Byte budget for the watering string (`f_display` 28 px in 280 px). |
| `plant_alert_chars`       | `26`    | Byte budget for the alert line (`f_body` 16 px in 248 px).      |

The character budgets are extrapolated from the reference config's measured value
(Figtree 15 px in a 280 px frame ≈ 32 characters), scaled to Roboto at the respective size
and rounded down, and **have not been measured on the panel**. Check them against your
longest plant name on the first flash. They count bytes, so an umlaut costs two — German
text truncates slightly earlier than necessary, but never mid-character.

---

## Data flow

Home Assistant fills numbered slots; the device rotates through them on its own. Rotation
and touch therefore need no network round trip, and a short Home Assistant outage does not
lose the plant list.

```
HA automation  --set_plant(0, 3, ...)-->  slot 0  ┐
               --set_plant(1, 3, ...)-->  slot 1  ├─ device rotates every 60s
               --set_plant(2, 3, ...)-->  slot 2  ┘
```

The automation runs **once per data change**, not on the rotation tick.

### `esphome.<node>_set_plant`

Writes one slot. Idempotent and order-independent — every call carries its own `total`, so
Home Assistant may send the slots in any order.

| Field           | Type     | Description                                                       |
| --------------- | -------- | ----------------------------------------------------------------- |
| `index`         | `int`    | 0-based position, must be `0 … total-1`                           |
| `total`         | `int`    | Number of plants, `1 … plant_max_slots`                           |
| `name`          | `string` | Display name                                                      |
| `next_watering` | `string` | Pre-formatted, e.g. `in 3 Tagen` or `Jetzt!`                      |
| `water_level`   | `int`    | `0` fine · `1` due soon · `2` due/overdue · `3` no data           |
| `alert`         | `string` | Open task or overwintering note, `""` for none                    |
| `alert_level`   | `int`    | `0` none · `1` notice · `2` urgent                                |

Out-of-range `index` / `total` are rejected with an error response instead of being acted
on. A `water_level` outside `0 … 3` is clamped to `3` (no data), never to `0` — a malformed
call must not paint a thirsty plant as healthy.

`water_level` is what carries the screen from across the room. It selects both the 96 px
glyph and the colour, so the state survives a dimmed backlight and red-green colour
blindness — colour alone never distinguishes two states:

| `water_level` | Glyph                     | Role        |
| ------------- | ------------------------- | ----------- |
| `0`           | `mdi:water-check`         | `c_ok`      |
| `1`           | `mdi:water-alert`         | `c_warn`    |
| `2`           | `mdi:watering-can`        | `c_alarm`   |
| `3`           | `mdi:help-circle-outline` | `c_disabled`|

The watering can at level `2` is deliberately the only silhouette that is not a droplet: at
48 mm the eye resolves outline, not detail, so the most urgent state gets the most distinct
shape.

The alert line works the same way — `mdi:information-outline` (circle, `c_warn`) against
`mdi:alert` (triangle, `c_alarm`).

Note that `next_watering` is a **string**, not a timestamp: formatting belongs on the Home
Assistant side, where the locale and the templating engine live. The device only positions
strings it has already been given.

### `esphome.<node>_clear_plants`

Empties the list. The box falls back to the Casita idle illustration.

### `esphome.plants_display_ready` (device → Home Assistant)

Fired by the device after the API client connects. After a reboot the slot buffer is empty,
so the device asks for the data rather than waiting for the automation's next scheduled
run. This needs **"Allow the device to perform Home Assistant actions"** enabled for the
device in the ESPHome integration.

The event is deliberately **not** fired from `on_client_connected` directly. At that moment
the connection exists but the client has not subscribed to actions yet, and the API drops
the event:

```
[W][api:437]: Home Assistant event 'esphome.plants_display_ready' dropped;
              client has not subscribed to actions (yet)
```

The device therefore waits 10 s and retries up to six times, stopping as soon as data
arrives. That also covers the case where the automation simply was not running on the first
attempt. After six attempts it goes quiet, so a device without a matching automation does
not emit events forever — the automation's `time_pattern` trigger picks it up instead.

---

## Home Assistant automation (Kamerplanter)

### How Kamerplanter models a plant

Each plant instance is its own Home Assistant **device**, created by the integration as:

```python
DeviceInfo(
    identifiers={(DOMAIN, f"{entry.entry_id}_plant_{key}")},
    name=plant_display_name(plant),          # "Monstera"
    model=f"Plant Instance ({code})",        # "Plant Instance (SPATH-0617-XUB)"
    model_id="plant_instance",
    ...
)
```

Two things follow from that, and both matter when you look plants up in the UI:

- **`model_id == "plant_instance"`** separates plants from planting runs, locations,
  tanks, and sites — all of which the same integration also exposes.
- **the plant-instance code** in `model` (`SPATH-0617-XUB`) is the backend's own
  identifier, the one behind `/v1/plant-instances/{plant_key}`. It is the reliable way to
  tell two similarly named plants apart, which is why the automation below carries it as a
  comment on every entry.

The per-plant entities then hang off that device:

| Entity                          | Scope        | Used for                                    |
| ------------------------------- | ------------ | ------------------------------------------- |
| `days_until_watering`           | per plant    | the watering string (state = day count)     |
| `pest_alert`                    | per plant    | alert level 2, "Schaedlingsalarm"           |
| `care_overdue`                  | **global**   | alert level 2, `overdue_count` open tasks   |
| `frost_forecast`                | **per site** | alert level 1, "Frost - ins Winterquartier" |

`days_until_watering` also carries `next_watering_date`, `last_watered`, and
`interval_days` as attributes. Only the day count is needed here — the device wants a
finished string, not a date.

Two traps worth knowing before writing templates against these, both found on a live
installation rather than in the source:

- **`needs_attention` does not exist for house plants.** The integration defines it, but
  only creates it for planting runs and locations. Do not build the alert line on it.
- **Entity-id prefixes are not uniform.** Some plants use the instance code
  (`sensor.draca_0616_owl_days_until_watering`), others the display name
  (`sensor.chinesischer_kolbenfaden_days_until_watering`), depending on when they were
  created. Match on the **suffix** only — matching a prefix silently misses plants.

### Selecting which plants reach the box

List one line per plant, in the order you want them on screen. This beats "every plant" in
practice: a Kamerplanter tenant may hold tent runs and seedlings that have no business on
the living-room box, and an explicit list keeps the **slot order stable**, so a plant does
not jump position when another is added.

**Write the entity IDs out rather than discovering them.** An earlier version of this
automation resolved plants dynamically through `integration_entities('kamerplanter')`,
`select('is_device_attr', 'model_id', 'plant_instance')` and `device_entities()`. It looked
elegant, validated cleanly through `check_config` — and then never ran, silently, because a
template condition evaluated to `false` without leaving anything in the log. With a handful
of plants the explicit list is more robust and, above all, debuggable: every line can be
checked against the entity list in the developer tools.

```yaml
# /config/automation/plants-display.yaml
# Bound via `automation: !include_dir_list automation` — one automation
# per file, as a MAPPING (no leading "- ").
id: plants_display_esp32_s3_box_3_02
alias: Pflanzen-Display (esp32-s3-box-3-02)
mode: single
max_exceeded: silent

triggers:
  - trigger: event
    event_type: esphome.plants_display_ready   # device rebooted, asks for data
  - trigger: homeassistant
    event: start
  - trigger: time_pattern
    minutes: "/15"                             # keeps relative times honest
  - trigger: state
    entity_id:
      - sensor.draca_0616_owl_days_until_watering
      - sensor.yucca_0617_dij_days_until_watering
      - sensor.spath_0617_xub_days_until_watering
      - sensor.chinesischer_kolbenfaden_days_until_watering
      - binary_sensor.kamerplanter_care_overdue
      - binary_sensor.wohnung_frost_forecast

variables:
  # One line per plant, in display order. The comment carries the
  # plant-instance code; the entity prefixes are NOT derivable from it.
  plants:
    - name: Drachenbaum                 # DRACA-0616-OWL
      days: sensor.draca_0616_owl_days_until_watering
      pest: binary_sensor.draca_0616_owl_pest_alert
    - name: Riesenpalmlilie             # YUCCA-0617-DIJ
      days: sensor.yucca_0617_dij_days_until_watering
      pest: binary_sensor.yucca_0617_dij_pest_alert
    - name: Peace Lily                  # SPATH-0617-XUB
      days: sensor.spath_0617_xub_days_until_watering
      pest: binary_sensor.spath_0617_xub_pest_alert
    - name: Chinesischer Kolbenfaden    # AGLAO-0617-RB5
      days: sensor.chinesischer_kolbenfaden_days_until_watering
      pest: binary_sensor.chinesischer_kolbenfaden_pest_alert

  # Frost is per site, care backlog is global — evaluated once, not per plant.
  frost: "{{ is_state('binary_sensor.wohnung_frost_forecast', 'on') }}"
  care_overdue: "{{ is_state('binary_sensor.kamerplanter_care_overdue', 'on') }}"
  overdue_count: >-
    {{ state_attr('binary_sensor.kamerplanter_care_overdue', 'overdue_count') | int(0) }}

actions:
  - repeat:
      for_each: "{{ plants }}"
      sequence:
        - action: esphome.esp32_s3_box_3_02_set_plant
          data:
            index: "{{ repeat.index - 1 }}"
            total: "{{ plants | count }}"
            name: "{{ repeat.item.name }}"
            next_watering: >-
              {% set d = states(repeat.item.days) | int(-99) %}
              {% if d == -99 %}keine Daten
              {% elif d < 0 %}ueberfaellig
              {% elif d == 0 %}Jetzt!
              {% elif d == 1 %}morgen
              {% else %}in {{ d }} Tagen{% endif %}
            # Selects the 96 px glyph and its colour. Same day count as
            # next_watering, mapped onto the four device states.
            water_level: >-
              {% set d = states(repeat.item.days) | int(-99) %}
              {% if d == -99 %}3
              {% elif d <= 0 %}2
              {% elif d == 1 %}1
              {% else %}0{% endif %}
            alert: >-
              {% if is_state(repeat.item.pest, 'on') %}Schaedlingsalarm
              {% elif care_overdue %}{{ overdue_count }} Aufgaben ueberfaellig
              {% elif frost %}Frost - ins Winterquartier
              {% else %}{% endif %}
            alert_level: >-
              {% if is_state(repeat.item.pest, 'on') or care_overdue %}2
              {% elif frost %}1
              {% else %}0{% endif %}
```

Codes you do not recognise are simply skipped, so a plant removed in Kamerplanter shrinks
the rotation instead of breaking the automation. Where to find a code: the plant's device
page in Home Assistant shows it as the model, `Plant Instance (SPATH-0617-XUB)`.

One thing worth keeping: the **`time_pattern` trigger** is not optional. `in 3 Tagen`
silently becomes wrong once a day passes, and nothing else would push a correction.

This automation is deployed on the live instance as
`/config/automation/plants-display.yaml`. Note that this Home Assistant binds automations
with `automation: !include_dir_list automation` — one automation per file, as a **mapping**
rather than a list item, and `automations.yaml` is not read at all. A new file only takes
effect after `automation.reload` or a restart, and it is worth running

```sh
python3 -m homeassistant --script check_config -c /config
```

first: under `!include_dir_list` a single broken file can take the whole `automation:`
block down with it.

### Umlauts

Kamerplanter's own German entity names avoid umlauts (`Naehrstoffplan`,
`Ueberfaellige Aufgaben`), and the strings above follow that. It is also the safer choice
on the device: the character budgets count bytes, and every umlaut costs two of them.

### Without Kamerplanter

The device contract is just seven arguments. Any source works — a template sensor, a
`todo` list, a manually maintained `variables:` block — as long as `next_watering` arrives
as a finished string, `water_level` is `0 … 3`, and `alert_level` is `0`, `1`, or `2`.

---

## Screen layout

320×240, 20 px outer margin, bottom 15 px reserved for the status strip. One message per
screen — *this plant and how thirsty it is* — laid out in the spec's reading hierarchy.

| Zone         | Rectangle (x, y, w, h) | Role         | Content                                            |
| ------------ | ---------------------- | ------------ | -------------------------------------------------- |
| Background   | `0, 0, 320, 240`       | —            | `fill(c_bg)`                                       |
| Identity     | `20, 20, 280, 28`      | who          | plant name, `f_display` 28 px, `c_text_dim`, left  |
| Hero         | `112, 56, 96, 96`      | how it is    | 96 px state glyph, state colour                    |
| State        | `20, 156, 280, 28`     | how it is    | watering string, `f_display` 28 px, state colour   |
| Qualifier    | `20, 190, 280, 30`     | what else    | 24 px alert glyph at `x=20`, text at `x=52`        |
| Status strip | `0, 225, 320, 15`      | where am I   | position dots, or `3/14` above 12 plants           |

```
+--------------------------------------+
| Drachenbaum                          |  f_display 28, c_text_dim, left
|                                      |
|              .-"""-.                 |
|             ( water )                |  96 px MDI, state colour
|              '-...-'                 |
|                                      |
|            in 3 Tagen                |  f_display 28, state colour
|                                      |
| (!) 2 Aufgaben ueberfaellig          |  24 px glyph + f_body 16
|            . . O . .                 |  status strip
+--------------------------------------+
```

Name and value share the 28 px step. The scale has nothing between 16 and 28, and 16 px was
too small for the name on the panel, so the hierarchy is carried by **colour** (dimmed
against state colour) and **alignment** (left against centred) rather than by size.

That costs name length: at 28 px the budget is 16 bytes, so `Chinesischer Kolbenfaden`
(24) truncates to `Chinesischer…`. Shorten the `name:` in the automation for plants whose
full name matters more than the size.

There are **no framed boxes**. On a black panel separation comes from contrast, not from
outlines — and the old white 280×54 value box was the brightest surface on the screen for a
secondary value, which is exactly what a design meant to survive a dimmed backlight must
not do.

The 96 px glyph is the one element legible from room distance (70′ at 0.5 m, 35′ at 1 m).
Nothing else on the screen is meant to be read from the sofa; 28 px reaches 10.3′ at 1 m,
which is detail you step closer for.

Alignment is deliberate: lines of text are left-aligned, only hero content is centred. A
centred plant name would jump horizontally on every rotation tick.

A slot Home Assistant has not filled yet renders as the `no data` state — its own glyph in
`c_disabled` plus a `...` placeholder — rather than as a healthy plant or an empty row.

---

## Behaviour

| Situation                        | What the box shows                                            |
| -------------------------------- | ------------------------------------------------------------- |
| No plant data yet                | Casita idle illustration, as before                           |
| Wake word, timer, pixel art      | The respective screen — plants only own the idle state        |
| Tap on the panel                 | Next plant, rotation timer restarts                           |
| Tap on the Home button           | Ignored by the rotation (it sits outside the panel area)      |
| Single plant                     | Shown permanently, no rotation                                |
| Voice dialogue in progress       | Rotation keeps its schedule but does not advance unseen       |
| Home Assistant unreachable       | The `no_ha_page` error screen, not a stale plant frame        |

`api.reboot_timeout` is `0s` in `common/base.yaml`, so a disconnected box does not reboot
on the usual 15-minute cycle.

---

## Rotation interval and the voice assistant

On a box that is also a voice satellite, the rotation interval is not a matter of taste.
A full-frame redraw blocks the ESPHome loop for about **95 ms**, measured on the device:

```
[W][component:473]: plant_rotate_timer took a long time for an operation (95 ms), max is 50 ms
```

150 KB over SPI at 40 MHz is 30 ms of pure transfer, the rest is text rendering — there is
little room to make it cheaper. The problem is *when* it happens: in the idle state, which
is exactly when `micro_wake_word` is listening. A wake word falling into that window loses
audio frames and is missed. It reads as "works most of the time, sometimes not".

Hence the 60 s default: it cuts the number of blocking windows to a quarter of what a 15 s
rotation produces, while a tap still advances instantly. A display-only device without a
voice assistant can go far lower.

The same applies to touch — `touchscreen took a long time (103 ms)` on every tap — but that
is user-initiated and therefore harmless.

---

## Cost

Three Roboto sizes (12 / 16 / 28 px, `GF_Latin_Core`) and six MDI glyphs. Measured on
ESPHome 2026.7.3:

| Build                          | Flash                 | RAM                  |
| ------------------------------ | --------------------- | -------------------- |
| `esp32-s3-box-3-02`, no package | 4 442 359 B (54.7 %)  | 124 483 B (36.4 %)   |
| `esp32-s3-box-3-03`, old design | 4 479 811 B (55.1 %)  | 125 651 B (36.8 %)   |
| `esp32-s3-box-3-03`, current    | 4 568 495 B (56.2 %)  | 126 059 B (36.9 %)   |
| **Package total**              | **+123 KB**           | **+1.5 KB**          |
| *of which the redesign*        | *+87 KB*              | *+0.4 KB*            |

Where the 87 KB went: `f_display` at `bpp: 4` dominates it. Four bits per pixel over
`GF_Latin_Core` at 28 px is roughly 80 KB on its own — the price of not showing a staircase
on the one number the screen exists for. The six icons are cheap by comparison: four 96 px
glyphs at ~9 KiB and two 24 px glyphs at ~576 B, about 37 KiB together, because
`GRAYSCALE` + `alpha_channel` stores one byte per pixel and one asset carries every state
colour.

For scale: a single full-screen `RGB` + `alpha_channel` illustration — what the Casita
screens next to this package use — is 300 KiB each.

`f_hero` (48 px) is deliberately **not** declared. The room-distance carrier here is the
96 px glyph; a 48 px Roboto face over the same glyph set would have cost roughly 230 KB for
a channel this screen does not use.

No new full-screen images, so the ~150 KiB framebuffer budget is untouched. Devices that do
not include this package carry none of it.

---

## Related

|                                       |                                                              |
| ------------------------------------- | ------------------------------------------------------------ |
| [kamerplanter-ha](https://github.com/nolte/kamerplanter-ha) | Supplies the plants, care profiles, and alerts |
| [kamerplanter](https://github.com/nolte/kamerplanter) | The backend behind the integration           |
| `src/common/esp32-s3-box-3-draw.yaml` | Panel, fonts, voice assistant, and the `idle_screen_script` hook |
| `src/common/pixel_art.yaml`           | Pixel art mode, which takes precedence over the plant screen |
| `docs/usecases/plant-watering.md`     | The watering side — pumps driven by the interval switch      |
