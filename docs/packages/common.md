# Common Configuration

Will be configure a Basement for different device Types. This will be allow the same Configuration like WIFI, and some Management settings, for all the Different devices. More information about Variable Usage take a look to ESPHome [substitutions](https://esphome.io/components/substitutions/).

## Usage

```yaml
packages:
  ...
  base: !include
    file: ./common/base.yaml
    vars:
      project_name: "watercheck.check"
      project_version: "esp32"
  ...
```  

## Implementation

```yaml
{%
   include "../../src/common/base.yaml"
%}
```
