#!/bin/bash

#adb push libmydyn1.so /data/local/tmp
adb push libintermediate.so /data/local/tmp
exit

adb push run.sh /data/local/tmp
adb push main   /data/local/tmp
adb push libintermediate.so /data/local/tmp
adb push libmydyn1.so /data/local/tmp
adb push libmydyn2.so /data/local/tmp

