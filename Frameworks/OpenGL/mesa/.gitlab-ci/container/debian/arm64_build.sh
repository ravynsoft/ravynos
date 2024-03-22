#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

set -e
set -o xtrace

export LLVM_VERSION="${LLVM_VERSION:=15}"

apt-get -y install ca-certificates
sed -i -e 's/http:\/\/deb/https:\/\/deb/g' /etc/apt/sources.list.d/*
echo "deb [trusted=yes] https://gitlab.freedesktop.org/gfx-ci/ci-deb-repo/-/raw/${PKG_REPO_REV}/ ${FDO_DISTRIBUTION_VERSION%-*} main" | tee /etc/apt/sources.list.d/gfx-ci_.list
apt-get update

# Ephemeral packages (installed for this script and removed again at the end)
EPHEMERAL=(
    libssl-dev
)

DEPS=(
    apt-utils
    android-libext4-utils
    autoconf
    automake
    bc
    bison
    ccache
    cmake
    curl
    fastboot
    flex
    g++
    git
    glslang-tools
    kmod
    libasan8
    libdrm-dev
    libelf-dev
    libexpat1-dev
    libvulkan-dev
    libx11-dev
    libx11-xcb-dev
    libxcb-dri2-0-dev
    libxcb-dri3-dev
    libxcb-glx0-dev
    libxcb-present-dev
    libxcb-randr0-dev
    libxcb-shm0-dev
    libxcb-xfixes0-dev
    libxdamage-dev
    libxext-dev
    libxrandr-dev
    libxshmfence-dev
    libxxf86vm-dev
    libwayland-dev
    libwayland-egl-backend-dev
    "llvm-${LLVM_VERSION}-dev"
    ninja-build
    meson
    openssh-server
    pkgconf
    python3-mako
    python3-pil
    python3-pip
    python3-requests
    python3-setuptools
    u-boot-tools
    xz-utils
    zlib1g-dev
    zstd
)

apt-get -y install "${DEPS[@]}" "${EPHEMERAL[@]}"

pip3 install --break-system-packages git+http://gitlab.freedesktop.org/freedesktop/ci-templates@ffe4d1b10aab7534489f0c4bbc4c5899df17d3f2

arch=armhf
. .gitlab-ci/container/cross_build.sh

. .gitlab-ci/container/container_pre_build.sh

. .gitlab-ci/container/build-mold.sh

. .gitlab-ci/container/build-wayland.sh

apt-get purge -y "${EPHEMERAL[@]}"

. .gitlab-ci/container/container_post_build.sh
