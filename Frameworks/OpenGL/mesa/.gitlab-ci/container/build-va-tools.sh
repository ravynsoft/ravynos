#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting
# When changing this file, you need to bump the following
# .gitlab-ci/image-tags.yml tags:
# KERNEL_ROOTFS_TAG

set -ex

git config --global user.email "mesa@example.com"
git config --global user.name "Mesa CI"

git clone \
    https://github.com/intel/libva-utils.git \
    -b 2.18.1 \
    --depth 1 \
    /va-utils

pushd /va-utils
# Too old libva in Debian 11. TODO: when this PR gets in, refer to the patch.
curl -L https://github.com/intel/libva-utils/pull/329.patch | git am

meson setup build -D tests=true -Dprefix=/va $EXTRA_MESON_ARGS
meson install -C build
popd
rm -rf /va-utils
