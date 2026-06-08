# Plant Watering

Use an external trigger, such as a Home Assistant automation, to start a watering interval. The
[Interval switch](../packages/intervall.md) package runs the pump for a set duration and repeats
the cycle a configured number of times.

You can stop the watering at any time by disabling the automation from the
[Interval switch](../packages/intervall.md).


## Configuration

**Example:**

```yaml
{%
   include "../../src/nous-a1t-02.yaml"
%}
```
