# esp32 s3 box-3

Smart device used as Voice Assistant [esp32-s3-box-3](https://www.elektor.de/products/esp32-s3-box-3?srsltid=AfmBOopI5QMMex6zqt0zUvIaYEyZWVcxCWe2w0gkai--bVcs36qIsK5v)

## Usage

```yaml
{%
   include "../../../src/esp32-s3-box-3-01.yaml"
%}
```


## Implementation

```yaml
{%
   include "../../../src/common/esp32-s3-box-3.yaml"
%}
```

## Pixel art variant

`esp32-s3-box-3-draw.yaml` is an extended version of the common config that adds a pixel-art
display mode on top of the voice assistant. It includes the [pixel art](../../packages/pixel_art.md)
package and registers the matching Home Assistant services in its `api:` block. Point a device file
at this variant instead of the base config to use it, as `src/esp32-s3-box-3-02.yaml` does:

```yaml
packages:
  plug: !include
    file: common/esp32-s3-box-3-draw.yaml
```

## Plant display variant

`esp32-s3-box-3-draw.yaml` exposes one extension point, the `idle_screen_script`
substitution, which decides what the box shows when it has nothing else to say. Left at its
default it stays on the Casita idle illustration, as devices `-01` and `-02` do.

The [plants display](../../packages/plants-display.md) package uses that hook to rotate
through your house plants on the idle screen — one plant per page, with a 96 px state glyph
readable from across the room and its next watering time below — while every voice-assistant
screen keeps working unchanged. It is the ambient front
end for the [Kamerplanter integration](https://github.com/nolte/kamerplanter-ha), which
supplies the plants and their care state. It is included **next to** the draw variant, as
`src/esp32-s3-box-3-03.yaml` does:

```yaml
substitutions:
  idle_screen_script: show_plants_screen

packages:
  plug: !include
    file: common/esp32-s3-box-3-draw.yaml
  plants: !include
    file: common/plants-display.yaml
```
