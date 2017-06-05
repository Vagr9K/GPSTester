#!/usr/bin/env bash
####################################################################################################
# You need to create build_config.local for android source tree and system library folder selection!
# Example`
# ###################################################################################################
## build_config.local ###############################################################################
## export ANDROID_SRCDIR=/mnt/android-workspace
## export SYSDIR=/mnt/android-workspace/out/target/product/gtexslte/
## export AUTOPUSH=true
#####################################################################################################
if [ ! -f 'build_config.local' ]; then
	echo "No build_config.local found. Aborting."
	exit 1
fi

source ./'build_config.local'

ndk-build NDK_APPLICATION_MK=./Application.mk

if [ "$AUTOPUSH" = true ] ; then
    adb root
    adb push obj/local/armeabi-v7a/gps-test /data/data/gps-test
fi