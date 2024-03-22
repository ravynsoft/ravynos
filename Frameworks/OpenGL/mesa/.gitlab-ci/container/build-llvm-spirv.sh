#!/usr/bin/env bash

set -ex

VER="${LLVM_VERSION:?llvm not set}.0.0"

curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
    -O "https://github.com/KhronosGroup/SPIRV-LLVM-Translator/archive/refs/tags/v${VER}.tar.gz"
tar -xvf "v${VER}.tar.gz" && rm "v${VER}.tar.gz"

mkdir "SPIRV-LLVM-Translator-${VER}/build"
pushd "SPIRV-LLVM-Translator-${VER}/build"
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
ninja
ninja install
# For some reason llvm-spirv is not installed by default
ninja llvm-spirv
cp tools/llvm-spirv/llvm-spirv /usr/bin/
popd

du -sh "SPIRV-LLVM-Translator-${VER}"
rm -rf "SPIRV-LLVM-Translator-${VER}"
