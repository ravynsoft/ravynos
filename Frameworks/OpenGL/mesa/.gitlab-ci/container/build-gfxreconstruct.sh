#!/usr/bin/env bash

set -ex

GFXRECONSTRUCT_VERSION=761837794a1e57f918a85af7000b12e531b178ae

git clone https://github.com/LunarG/gfxreconstruct.git \
    --single-branch \
    -b master \
    --no-checkout \
    /gfxreconstruct
pushd /gfxreconstruct
git checkout "$GFXRECONSTRUCT_VERSION"
git submodule update --init
git submodule update
cmake -S . -B _build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=/gfxreconstruct/build -DBUILD_WERROR=OFF
cmake --build _build --parallel --target tools/{replay,info}/install/strip
find . -not -path './build' -not -path './build/*' -delete
popd
