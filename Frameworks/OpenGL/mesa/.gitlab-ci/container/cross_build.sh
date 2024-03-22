#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

set -e
set -o xtrace

export DEBIAN_FRONTEND=noninteractive
export LLVM_VERSION="${LLVM_VERSION:=15}"

# Ephemeral packages (installed for this script and removed again at the end)
EPHEMERAL=(
)

DEPS=(
    "crossbuild-essential-$arch"
    "pkgconf:$arch"
    "libasan8:$arch"
    "libdrm-dev:$arch"
    "libelf-dev:$arch"
    "libexpat1-dev:$arch"
    "libffi-dev:$arch"
    "libpciaccess-dev:$arch"
    "libstdc++6:$arch"
    "libvulkan-dev:$arch"
    "libx11-dev:$arch"
    "libx11-xcb-dev:$arch"
    "libxcb-dri2-0-dev:$arch"
    "libxcb-dri3-dev:$arch"
    "libxcb-glx0-dev:$arch"
    "libxcb-present-dev:$arch"
    "libxcb-randr0-dev:$arch"
    "libxcb-shm0-dev:$arch"
    "libxcb-xfixes0-dev:$arch"
    "libxdamage-dev:$arch"
    "libxext-dev:$arch"
    "libxrandr-dev:$arch"
    "libxshmfence-dev:$arch"
    "libxxf86vm-dev:$arch"
    "libwayland-dev:$arch"
)

dpkg --add-architecture $arch

echo "deb [trusted=yes] https://gitlab.freedesktop.org/gfx-ci/ci-deb-repo/-/raw/${PKG_REPO_REV}/ ${FDO_DISTRIBUTION_VERSION%-*} main" | tee /etc/apt/sources.list.d/gfx-ci_.list

apt-get update

apt-get install -y --no-remove "${DEPS[@]}" "${EPHEMERAL[@]}" \
    $EXTRA_LOCAL_PACKAGES

if [[ $arch != "armhf" ]]; then
    # We don't need clang-format for the crossbuilds, but the installed amd64
    # package will conflict with libclang. Uninstall clang-format (and its
    # problematic dependency) to fix.
    apt-get remove -y "clang-format-${LLVM_VERSION}" "libclang-cpp${LLVM_VERSION}"

    # llvm-*-tools:$arch conflicts with python3:amd64. Install dependencies only
    # with apt-get, then force-install llvm-*-{dev,tools}:$arch with dpkg to get
    # around this.
    apt-get install -y --no-remove --no-install-recommends \
            "libclang-cpp${LLVM_VERSION}:$arch" \
            "libgcc-s1:$arch" \
            "libtinfo-dev:$arch" \
            "libz3-dev:$arch" \
            "llvm-${LLVM_VERSION}:$arch" \
            zlib1g
fi

. .gitlab-ci/container/create-cross-file.sh $arch


. .gitlab-ci/container/container_pre_build.sh


# dependencies where we want a specific version
MULTIARCH_PATH=$(dpkg-architecture -A $arch -qDEB_TARGET_MULTIARCH)
export EXTRA_MESON_ARGS="--cross-file=/cross_file-${arch}.txt -D libdir=lib/${MULTIARCH_PATH}"
. .gitlab-ci/container/build-wayland.sh

. .gitlab-ci/container/build-directx-headers.sh

apt-get purge -y "${EPHEMERAL[@]}"

. .gitlab-ci/container/container_post_build.sh

# This needs to be done after container_post_build.sh, or apt-get breaks in there
if [[ $arch != "armhf" ]]; then
    apt-get download llvm-"${LLVM_VERSION}"-{dev,tools}:"$arch"
    dpkg -i --force-depends llvm-"${LLVM_VERSION}"-*_"${arch}".deb
    rm llvm-"${LLVM_VERSION}"-*_"${arch}".deb
fi
