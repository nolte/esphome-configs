# Pixel art

**Package:** `src/common/pixel_art.yaml`

```yaml
# included by src/common/esp32-s3-box-3-draw.yaml
packages:
  pixel_art: !include ./pixel_art.yaml
```

---

## Purpose

Draws a small 16x16 pixel-art icon on the ESP32-S3-BOX-3 display. Home Assistant sends the image
as a hex string through a service call, and a dedicated display page renders it. It's meant for
at-a-glance status icons (weather, presence, a custom logo) on top of the voice-assistant UI.

The package is wired into [`esp32-s3-box-3-draw.yaml`](../devices/displays/esp32-s3-box-3.md), the
pixel-art variant of the S3-BOX-3 common config. It can't run on its own, because it relies on the
display, fonts, and `voice_assistant_phase` handling from that package.

---

## Image protocol

An image is a 256-character hex string (16x16 = 256 pixels). Each character is a colour index from
`0` to `F`, drawn as an 8x8 block, for a 128x128 icon centred on the 320x240 display. A string that
isn't exactly 256 characters long shows a `Kein Pixel Art` placeholder instead (German for "no pixel art").

### Colour palette

| Index | Colour     | Hex      | Index | Colour      | Hex      |
| ----- | ---------- | -------- | ----- | ----------- | -------- |
| `0`   | Black      | `000000` | `8`   | Orange      | `FF8000` |
| `1`   | White      | `FFFFFF` | `9`   | Purple      | `800080` |
| `2`   | Red        | `FF0000` | `A`   | Dark green  | `008000` |
| `3`   | Lime       | `00FF00` | `B`   | Brown       | `8B4513` |
| `4`   | Blue       | `0000FF` | `C`   | Pink        | `FF69B4` |
| `5`   | Yellow     | `FFFF00` | `D`   | Light blue  | `ADD8E6` |
| `6`   | Cyan       | `00FFFF` | `E`   | Dark red    | `8B0000` |
| `7`   | Magenta    | `FF00FF` | `F`   | Gray        | `808080` |

---

## Exposed entities

### `Pixel Art Mode` (switch, config)

Turns pixel-art mode on or off. While on, the pixel-art page takes priority over the normal
voice-assistant display. Pressing the physical Home button also leaves the mode.

---

## Home Assistant services

The service actions live in the `api:` block of `esp32-s3-box-3-draw.yaml`, not in this package,
because ESPHome doesn't reliably merge `api:` blocks from nested packages.

| Service                            | Parameters             | Effect                                    |
| ---------------------------------- | ---------------------- | ----------------------------------------- |
| `esphome.{name}_set_pixel_art`     | `pixel_data`, `label`  | Shows the icon and a caption, enters mode |
| `esphome.{name}_clear_pixel_art`   | *(none)*               | Clears the image and leaves the mode      |

`{name}` is the device name, for example `esp32_s3_box_3_02`.

---

## Usage

The package ships through the pixel-art display variant. Point a device file at
`common/esp32-s3-box-3-draw.yaml` (as `src/esp32-s3-box-3-02.yaml` does), then call the service
from Home Assistant:

```yaml
service: esphome.esp32_s3_box_3_02_set_pixel_art
data:
  pixel_data: "0000...FFFF"   # 256 hex characters
  label: "Weather"
```

---

## Related

|                                          |                                                          |
| ---------------------------------------- | -------------------------------------------------------- |
| `src/common/esp32-s3-box-3-draw.yaml`    | S3-BOX-3 common config that includes this package        |
| `src/esp32-s3-box-3-02.yaml`             | Device file using the pixel-art display variant          |
