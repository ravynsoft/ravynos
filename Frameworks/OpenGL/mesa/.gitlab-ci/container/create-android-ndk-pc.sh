#!/bin/sh
# shellcheck disable=SC2086 # we want word splitting

# Makes a .pc file in the Android NDK for meson to find its libraries.

set -ex

ndk="$1"
pc="$2"
cflags="$3"
libs="$4"
version="$5"
sdk_version="$6"

sysroot=$ndk/toolchains/llvm/prebuilt/linux-x86_64/sysroot

for arch in \
        x86_64-linux-android \
        i686-linux-android \
        aarch64-linux-android \
        arm-linux-androideabi; do
    pcdir=$sysroot/usr/lib/$arch/pkgconfig
    mkdir -p $pcdir

    cat >$pcdir/$pc <<EOF
prefix=$sysroot
exec_prefix=$sysroot
libdir=$sysroot/usr/lib/$arch/$sdk_version
sharedlibdir=$sysroot/usr/lib/$arch
includedir=$sysroot/usr/include

Name: zlib
Description: zlib compression library
Version: $version

Requires:
Libs: -L$sysroot/usr/lib/$arch/$sdk_version $libs
Cflags: -I$sysroot/usr/include $cflags
EOF
done
