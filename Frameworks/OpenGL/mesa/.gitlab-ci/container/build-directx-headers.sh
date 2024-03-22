#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

# When changing this file, you need to bump the following
# .gitlab-ci/image-tags.yml tags:
# DEBIAN_BUILD_TAG

set -ex

git clone https://github.com/microsoft/DirectX-Headers -b v1.611.0 --depth 1
pushd DirectX-Headers
meson setup build --backend=ninja --buildtype=release -Dbuild-test=false $EXTRA_MESON_ARGS
meson install -C build
popd
rm -rf DirectX-Headers
