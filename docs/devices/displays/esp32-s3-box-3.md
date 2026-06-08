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
