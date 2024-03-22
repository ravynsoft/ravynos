#!/bin/bash

ndk=$1
arch=$2
cpu_family=$3
cpu=$4
cross_file="/cross_file-$arch.txt"
sdk_version=$5

# armv7 has the toolchain split between two names.
arch2=${6:-$2}

# Note that we disable C++ exceptions, because Mesa doesn't use exceptions,
# and allowing it in code generation means we get unwind symbols that break
# the libEGL and driver symbol tests.

cat > "$cross_file" <<EOF
[binaries]
ar = '$ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar'
c = ['ccache', '$ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/${arch2}${sdk_version}-clang', '-fno-exceptions', '-fno-unwind-tables', '-fno-asynchronous-unwind-tables']
cpp = ['ccache', '$ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/${arch2}${sdk_version}-clang++', '-fno-exceptions', '-fno-unwind-tables', '-fno-asynchronous-unwind-tables', '-static-libstdc++']
c_ld = 'lld'
cpp_ld = 'lld'
strip = '$ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip'
pkgconfig = ['/usr/bin/pkgconf']

[host_machine]
system = 'android'
cpu_family = '$cpu_family'
cpu = '$cpu'
endian = 'little'

[properties]
needs_exe_wrapper = true
pkg_config_libdir = '/usr/local/lib/${arch2}/pkgconfig/:/${ndk}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/${arch2}/pkgconfig/'

EOF
