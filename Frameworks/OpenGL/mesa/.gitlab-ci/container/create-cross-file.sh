#!/bin/bash

arch=$1
cross_file="/cross_file-$arch.txt"
meson env2mfile --cross --debarch "$arch" -o "$cross_file"

# Explicitly set ccache path for cross compilers
sed -i "s|/usr/bin/\([^-]*\)-linux-gnu\([^-]*\)-g|/usr/lib/ccache/\\1-linux-gnu\\2-g|g" "$cross_file"

# Rely on qemu-user being configured in binfmt_misc on the host
# shellcheck disable=SC1003 # how this sed doesn't seems to work for me locally
sed -i -e '/\[properties\]/a\' -e "needs_exe_wrapper = False" "$cross_file"

# Add a line for rustc, which meson env2mfile is missing.
cc=$(sed -n "s|^c\s*=\s*\[?'\(.*\)'\]?|\1|p" < "$cross_file")

if [[ "$arch" = "arm64" ]]; then
    rust_target=aarch64-unknown-linux-gnu
elif [[ "$arch" = "armhf" ]]; then
    rust_target=armv7-unknown-linux-gnueabihf
elif [[ "$arch" = "i386" ]]; then
    rust_target=i686-unknown-linux-gnu
elif [[ "$arch" = "ppc64el" ]]; then
    rust_target=powerpc64le-unknown-linux-gnu
elif [[ "$arch" = "s390x" ]]; then
    rust_target=s390x-unknown-linux-gnu
else
    echo "Needs rustc target mapping"
fi

# shellcheck disable=SC1003 # how this sed doesn't seems to work for me locally
sed -i -e '/\[binaries\]/a\' -e "rust = ['rustc', '--target=$rust_target', '-C', 'linker=$cc']" "$cross_file"

# Set up cmake cross compile toolchain file for dEQP builds
toolchain_file="/toolchain-$arch.cmake"
if [[ "$arch" = "arm64" ]]; then
    GCC_ARCH="aarch64-linux-gnu"
    DE_CPU="DE_CPU_ARM_64"
elif [[ "$arch" = "armhf" ]]; then
    GCC_ARCH="arm-linux-gnueabihf"
    DE_CPU="DE_CPU_ARM"
fi

if [[ -n "$GCC_ARCH" ]]; then
    {
        echo "set(CMAKE_SYSTEM_NAME Linux)";
        echo "set(CMAKE_SYSTEM_PROCESSOR arm)";
        echo "set(CMAKE_C_COMPILER /usr/lib/ccache/$GCC_ARCH-gcc)";
        echo "set(CMAKE_CXX_COMPILER /usr/lib/ccache/$GCC_ARCH-g++)";
        echo "set(CMAKE_CXX_FLAGS_INIT \"-Wno-psabi\")";  # makes ABI warnings quiet for ARMv7
        echo "set(ENV{PKG_CONFIG} \"/usr/bin/$GCC_ARCH-pkgconf\")";
        echo "set(DE_CPU $DE_CPU)";
    } > "$toolchain_file"
fi
