#!/usr/bin/env bash

set -ex

export LLVM_CONFIG="llvm-config-${LLVM_VERSION:?"llvm unset!"}"
LLVM_TAG="llvmorg-15.0.7"

$LLVM_CONFIG --version

git config --global user.email "mesa@example.com"
git config --global user.name "Mesa CI"
git clone \
    https://github.com/llvm/llvm-project \
    --depth 1 \
    -b "${LLVM_TAG}" \
    /llvm-project

mkdir /libclc
pushd /libclc
cmake -S /llvm-project/libclc -B . -G Ninja -DLLVM_CONFIG="$LLVM_CONFIG" -DLIBCLC_TARGETS_TO_BUILD="spirv-mesa3d-;spirv64-mesa3d-" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DLLVM_SPIRV=/usr/bin/llvm-spirv
ninja
ninja install
popd

# workaroud cmake vs debian packaging.
mkdir -p /usr/lib/clc
ln -s /usr/share/clc/spirv64-mesa3d-.spv /usr/lib/clc/
ln -s /usr/share/clc/spirv-mesa3d-.spv /usr/lib/clc/

du -sh ./*
rm -rf /libclc /llvm-project
