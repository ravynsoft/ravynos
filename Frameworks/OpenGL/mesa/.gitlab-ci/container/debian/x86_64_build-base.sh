#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

# When changing this file, you need to bump the following
# .gitlab-ci/image-tags.yml tags:
# DEBIAN_BUILD_TAG

set -e
set -o xtrace

export DEBIAN_FRONTEND=noninteractive
export LLVM_VERSION="${LLVM_VERSION:=15}"

apt-get install -y ca-certificates
sed -i -e 's/http:\/\/deb/https:\/\/deb/g' /etc/apt/sources.list.d/*
echo "deb [trusted=yes] https://gitlab.freedesktop.org/gfx-ci/ci-deb-repo/-/raw/${PKG_REPO_REV}/ ${FDO_DISTRIBUTION_VERSION%-*} main" | tee /etc/apt/sources.list.d/gfx-ci_.list

# Ephemeral packages (installed for this script and removed again at
# the end)
EPHEMERAL=(
)

DEPS=(
    apt-utils
    bison
    ccache
    curl
    "clang-format-${LLVM_VERSION}"
    dpkg-cross
    findutils
    flex
    g++
    cmake
    gcc
    git
    glslang-tools
    kmod
    "libclang-${LLVM_VERSION}-dev"
    "libclang-cpp${LLVM_VERSION}-dev"
    "libclang-common-${LLVM_VERSION}-dev"
    libelf-dev
    libepoxy-dev
    libexpat1-dev
    libgtk-3-dev
    "libllvm${LLVM_VERSION}"
    libomxil-bellagio-dev
    libpciaccess-dev
    libunwind-dev
    libva-dev
    libvdpau-dev
    libvulkan-dev
    libx11-dev
    libx11-xcb-dev
    libxext-dev
    libxml2-utils
    libxrandr-dev
    libxrender-dev
    libxshmfence-dev
    libxxf86vm-dev
    libwayland-egl-backend-dev
    make
    ninja-build
    openssh-server
    pkgconf
    python3-mako
    python3-pil
    python3-pip
    python3-ply
    python3-requests
    python3-setuptools
    qemu-user
    valgrind
    x11proto-dri2-dev
    x11proto-gl-dev
    x11proto-randr-dev
    xz-utils
    zlib1g-dev
    zstd
)

apt-get update

apt-get install -y --no-remove "${DEPS[@]}" "${EPHEMERAL[@]}" \
        $EXTRA_LOCAL_PACKAGES

# Needed for ci-fairy, this revision is able to upload files to S3
pip3 install --break-system-packages git+http://gitlab.freedesktop.org/freedesktop/ci-templates@ffe4d1b10aab7534489f0c4bbc4c5899df17d3f2

# We need at least 1.3.1 for rusticl
pip3 install --break-system-packages 'meson==1.3.1'

. .gitlab-ci/container/build-rust.sh

############### Uninstall ephemeral packages

apt-get purge -y "${EPHEMERAL[@]}"

. .gitlab-ci/container/container_post_build.sh
