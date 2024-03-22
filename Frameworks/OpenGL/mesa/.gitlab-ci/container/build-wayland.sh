#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

set -ex

export LIBWAYLAND_VERSION="1.21.0"
export WAYLAND_PROTOCOLS_VERSION="1.31"

git clone https://gitlab.freedesktop.org/wayland/wayland
cd wayland
git checkout "$LIBWAYLAND_VERSION"
meson setup -Ddocumentation=false -Ddtd_validation=false -Dlibraries=true _build $EXTRA_MESON_ARGS
meson install -C _build
cd ..
rm -rf wayland

git clone https://gitlab.freedesktop.org/wayland/wayland-protocols
cd wayland-protocols
git checkout "$WAYLAND_PROTOCOLS_VERSION"
meson setup _build $EXTRA_MESON_ARGS
meson install -C _build
cd ..
rm -rf wayland-protocols
