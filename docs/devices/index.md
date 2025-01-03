# Devices Configurations

```yaml
substitutions:
  name: cam-04
  id: cam_04
  comment: "Box4 CAM 01"
```

## Base Device Config

```yaml
packages:
  base: !include 
    file: ./base.yaml
    vars:
      project_name: "ESP.Smart-Wifi-CAM"
      project_version: "espcam32
```

### Full Config

```yaml
{%
   include "../../src/common/base.yaml"
%}
```