# multi point liquid level

* [CQRSENYW003](http://www.cqrobot.wiki/index.php/Contact_Multi-point_Photoelectric_Liquid_Level_Sensor_SKU:_CQRSENYW003
)

## Usage

This sensor reports the liquid level at several fixed points along a tank, so you can track its
level as it fills or empties. Include the shared component below in a device file.

## Implementation

```yaml
{%
   include "../../../src/common/sensor/cqrobot-multi-point-photoelectric-liquid-level.yaml"
%}
```
