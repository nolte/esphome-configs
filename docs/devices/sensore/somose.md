# SoMoSe

* [BeFlE/SoMoSe](https://github.com/BeFlE/SoMoSe)

SoMoSe is a soil-moisture sensor integrated through a custom external component
(`src/my_components/somose/`) rather than a ready-made shared package. There's no `common/`
block for it yet.

## Implementation

A working proof-of-concept device configuration:

```yaml
{%
   include "../../../src/poc/poc-esp32-i2c-somose.yaml"
%}
```
