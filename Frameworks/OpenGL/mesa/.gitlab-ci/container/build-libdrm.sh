#!/usr/bin/env bash
# Script used for Android and Fedora builds
# shellcheck disable=SC2086 # we want word splitting

set -ex

export LIBDRM_VERSION=libdrm-2.4.119

curl -L -O --retry 4 -f --retry-all-errors --retry-delay 60 \
    https://dri.freedesktop.org/libdrm/"$LIBDRM_VERSION".tar.xz
tar -xvf "$LIBDRM_VERSION".tar.xz && rm "$LIBDRM_VERSION".tar.xz
cd "$LIBDRM_VERSION"
meson setup build -D vc4=disabled -D freedreno=disabled -D etnaviv=disabled $EXTRA_MESON_ARGS
meson install -C build
cd ..
rm -rf "$LIBDRM_VERSION"
