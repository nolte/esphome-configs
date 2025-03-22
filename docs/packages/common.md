# Common Configs


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

## Configuration Parts

```yaml
{%
   include "../../src/common/base.yaml"
%}
```
