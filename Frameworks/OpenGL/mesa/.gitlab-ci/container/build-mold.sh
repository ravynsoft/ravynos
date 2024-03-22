#!/usr/bin/env bash

set -ex

MOLD_VERSION="1.11.0"

git clone -b v"$MOLD_VERSION" --single-branch --depth 1 https://github.com/rui314/mold.git
pushd mold

cmake -DCMAKE_BUILD_TYPE=Release -D BUILD_TESTING=OFF -D MOLD_LTO=ON
cmake --build . --parallel
cmake --install .

popd
rm -rf mold
