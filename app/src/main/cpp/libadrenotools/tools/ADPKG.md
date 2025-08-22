# Android Driver Package Format

## Contents
- `meta.json` that follows `Schema`
- `<driver>.so` main driver shared library
- `*.so` all libraries depended on by the main driver library, these must have their sonames altered if they wish to replace the system version of the library

## Example
- `meta.json`:
```json
{
  "schemaVersion": 1,
  "name": "Qualcomm v615 Vulkan driver",
  "description": "Proprietary Vulkan driver for Adreno 6xx and Adreno 7xx",
  "author": "ByLaws",
  "packageVersion": "2",
  "vendor": "Qualcomm",
  "driverVersion": "0.615.0",
  "minApi": 27,
  "libraryName": "vulkan.ad0615.so"
}
```

- `vulkan.ad0615.so`: main patched driver
- `notadreno_utils.so`, `notdmabufheap.so`, `notgsl.so`, `notllvm-glnext.so`, `notllvm-qgl.so`: patched-soname versions of libraries specific to each driver version
- `android.hardware.graphics.mappershim.so`, `vendor.qti.hardware.display.mapperextensionsshim.so`, `vendor.qti.hardware.display.mappershim.so`: qtimapper-shim libs

## Creation
I recommend running blob-patcher.py to patch a set of blobs from a device dump with mapper ver as 5, then copying in the qtimapper-shim files from the adrenotools releases page.
```bash
$ mkdir outpkg
$ patch.py <device dump> outpkg vulkan.adreno.so vulkan.ad0615.so 1
$ vim outpkg/meta.json
$ cp qtimapper-shim-rel/* outpkg
```

## Schema
```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "ADPKG Schema",
  "type": "object",
  "properties": {
    "schemaVersion": {
      "type": "number",
      "example": 1
    },
    "name": {
      "type": "string",
      "example": "Qualcomm Adreno Driver"
    },
    "vendor": {
      "enum": [
        "Qualcomm",
        "Mesa"
      ],
    },
    "driverVersion": {
      "type": "string",
      "pattern": "^[0-9]+.[0-9]+.[0-9]+(-.+)?$",
      "example": "512.604.0"
    },
    "author": {
      "type": "string",
      "example": "Billy Laws"
    },
    "description": {
      "type": "string",
      "description": "Additional description of the driver, this shouldn't contain redundant information that is already covered by the other fields such as the version and only denote details important for the user"
    },
    "packageVersion": {
      "type": "string",
      "example": "3029-bylaws"
    },
    "minApi": {
      "type": "number",
      "description": "The minimum Android API version required by the driver to function correctly",
      "example": 27
    },
    "libraryName": {
      "type": "string",
      "description": "The name of the main shared library object",
      "example": "vulkan.adreno.so"
    },
  },
  "required": [
    "schemaVersion",
    "name",
    "author",
    "packageVersion",
    "vendor",
    "driverVersion",
    "minApi",
  ]
}
```
