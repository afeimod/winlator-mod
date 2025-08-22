## Adreno Compiler Collection parameter shim

### TODO
Make some sort of API for this so parameters can be changed through adrenotools in realtime.

### Compilation
```
$ sed -i 's/libllvm-glnext/notreal-glnext/g' libllvm-glnext.so
$ mv libllvm-glnext.so notreal-glnext.so
$ aarch64-linux-android28-clang  vk_acc_shim.cpp -o notllvm-glnext.so --shared -fpic
$ sed -i 's/libllvm-glnext/notllvm-glnext/g' vulkan.adreno.so
```
