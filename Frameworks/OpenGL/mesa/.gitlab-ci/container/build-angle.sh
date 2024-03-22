#!/usr/bin/env bash

set -ex

ANGLE_REV="0518a3ff4d4e7e5b2ce8203358f719613a31c118"

# DEPOT tools
git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git
PWD=$(pwd)
export PATH=$PWD/depot_tools:$PATH
export DEPOT_TOOLS_UPDATE=0

mkdir /angle-build
pushd /angle-build
git init
git remote add origin https://chromium.googlesource.com/angle/angle.git
git fetch --depth 1 origin "$ANGLE_REV"
git checkout FETCH_HEAD

# source preparation
python3 scripts/bootstrap.py
mkdir -p build/config
gclient sync

sed -i "/catapult/d" testing/BUILD.gn

mkdir -p out/Release
echo '
is_debug = false
angle_enable_swiftshader = false
angle_enable_null = false
angle_enable_gl = false
angle_enable_vulkan = true
angle_has_histograms = false
build_angle_trace_perf_tests = false
build_angle_deqp_tests = false
angle_use_custom_libvulkan = false
dcheck_always_on=true
' > out/Release/args.gn

if [[ "$DEBIAN_ARCH" = "arm64" ]]; then
  build/linux/sysroot_scripts/install-sysroot.py --arch=arm64
fi

gn gen out/Release
# depot_tools overrides ninja with a version that doesn't work.  We want
# ninja with FDO_CI_CONCURRENT anyway.
/usr/local/bin/ninja -C out/Release/

mkdir /angle
cp out/Release/lib*GL*.so /angle/
ln -s libEGL.so /angle/libEGL.so.1
ln -s libGLESv2.so /angle/libGLESv2.so.2

rm -rf out

popd
rm -rf ./depot_tools
