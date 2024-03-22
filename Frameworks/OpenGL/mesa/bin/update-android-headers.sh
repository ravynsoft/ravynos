#!/bin/sh

set -eu

if [ ! -e .git ]; then
    echo must run from top-level directory;
    exit 1
fi

if [ ! -d platform-hardware-libhardware ]; then
    git clone --depth 1 https://android.googlesource.com/platform/frameworks/native platform-frameworks-native
    git clone --depth 1 https://android.googlesource.com/platform/hardware/libhardware platform-hardware-libhardware
    git clone --depth 1 https://android.googlesource.com/platform/system/core platform-system-core
    git clone --depth 1 https://android.googlesource.com/platform/system/logging platform-system-logging
    git clone --depth 1 https://android.googlesource.com/platform/system/unwinding platform-system-unwinding
fi

dest=include/android_stub

# Persist the frozen Android N system/window.h for backward compatibility

cp -av ${dest}/system/window.h platform-system-core/libsystem/include/system

rm -rf ${dest}
mkdir ${dest}


# These directories contains mostly only the files we need, so copy wholesale

cp -av                                                                  \
    platform-frameworks-native/libs/nativewindow/include/vndk           \
    platform-frameworks-native/libs/nativebase/include/nativebase       \
    platform-system-core/libsync/include/ndk                            \
    platform-system-core/libsync/include/sync                           \
    platform-system-core/libsystem/include/system                       \
    platform-system-logging/liblog/include/log                          \
    platform-system-unwinding/libbacktrace/include/backtrace            \
    ${dest}


# We only need a few files from these big directories so just copy those

mkdir ${dest}/hardware
cp -av platform-hardware-libhardware/include/hardware/{hardware,gralloc,gralloc1,fb}.h ${dest}/hardware
cp -av platform-frameworks-native/vulkan/include/hardware/hwvulkan.h ${dest}/hardware

mkdir ${dest}/cutils
cp -av platform-system-core/libcutils/include/cutils/{compiler,log,native_handle,properties,trace}.h ${dest}/cutils


# include/android has files from a few different projects

mkdir ${dest}/android
cp -av                                                                  \
    platform-frameworks-native/libs/nativewindow/include/android/*      \
    platform-frameworks-native/libs/arect/include/android/*             \
    platform-system-core/libsync/include/android/*                      \
    platform-system-logging/liblog/include/android/*                    \
    ${dest}/android

