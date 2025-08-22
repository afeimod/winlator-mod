## QtiMapper-shim

A shim that translates between the IQtiMapper(2,3,4) API used by newer Adreno graphics drivers and gralloc1, which is the only available gralloc API on sdm845.

### Method

Exposes the same ABI as the newer mapper blobs and stubs enough functions to force the HAL to fallback to the 2.0 mapper/mapperextension API which is then shimmed with a passthrough to gralloc1. The blobs can then be patched to load the stub libs instead, see the blob-patcher script.
The code is fairly ugly but functional enough for the purpose, some functions are missing and will need to be implemented in the case that the driver starts using them.


### Building

Chuck into a lineage tree, then:
```
make vendor.qti.hardware.display.mappershim
make vendor.qti.hardware.display.mapperextensionsshim
make android.hardware.graphics.mappershim
```
and grab SOs out of outputs dir
