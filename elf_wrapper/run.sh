#!/bin/bash

# echo md5sum  apktool/NativeActivity/lib/armeabi/libnative-activity.so
# md5sum  apktool/NativeActivity/lib/armeabi/libnative-activity.so

echo ./elf_wrapper apktool/NativeActivity/lib/armeabi/libnative-activity.so
./elf_wrapper apktool/NativeActivity/lib/armeabi/libnative-activity.so
