#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# Copyright © 2022 Collabora Limited
# Author: Guilherme Gallo <guilherme.gallo@collabora.com>
#
# When changing this file, you need to bump the following
# .gitlab-ci/image-tags.yml tags:
# KERNEL_ROOTFS_TAG

SKQP_BRANCH=android-cts-12.1_r5

# hack for skqp see the clang
pushd /usr/bin/
ln -s ../lib/llvm-15/bin/clang clang
ln -s ../lib/llvm-15/bin/clang++ clang++
popd

create_gn_args() {
    # gn can be configured to cross-compile skia and its tools
    # It is important to set the target_cpu to guarantee the intended target
    # machine
    cp "${BASE_ARGS_GN_FILE}" "${SKQP_OUT_DIR}"/args.gn
    echo "target_cpu = \"${SKQP_ARCH}\"" >> "${SKQP_OUT_DIR}"/args.gn
}


download_skia_source() {
    if [ -z ${SKIA_DIR+x} ]
    then
        return 1
    fi

    # Skia cloned from https://android.googlesource.com/platform/external/skqp
    # has all needed assets tracked on git-fs
    SKQP_REPO=https://android.googlesource.com/platform/external/skqp

    git clone --branch "${SKQP_BRANCH}" --depth 1 "${SKQP_REPO}" "${SKIA_DIR}"
}

set -ex

SCRIPT_DIR=$(realpath "$(dirname "$0")")
SKQP_PATCH_DIR="${SCRIPT_DIR}/patches"
BASE_ARGS_GN_FILE="${SCRIPT_DIR}/build-skqp_base.gn"

SKQP_ARCH=${SKQP_ARCH:-x64}
SKIA_DIR=${SKIA_DIR:-$(mktemp -d)}
SKQP_OUT_DIR=${SKIA_DIR}/out/${SKQP_ARCH}
SKQP_INSTALL_DIR=${SKQP_INSTALL_DIR:-/skqp}
SKQP_ASSETS_DIR="${SKQP_INSTALL_DIR}/assets"
SKQP_BINARIES=(skqp list_gpu_unit_tests list_gms)

download_skia_source

pushd "${SKIA_DIR}"

# Apply all skqp patches for Mesa CI
cat "${SKQP_PATCH_DIR}"/build-skqp_*.patch |
    patch -p1

# Fetch some needed build tools needed to build skia/skqp.
# Basically, it clones repositories with commits SHAs from ${SKIA_DIR}/DEPS
# directory.
python tools/git-sync-deps

mkdir -p "${SKQP_OUT_DIR}"
mkdir -p "${SKQP_INSTALL_DIR}"

create_gn_args

# Build and install skqp binaries
bin/gn gen "${SKQP_OUT_DIR}"

for BINARY in "${SKQP_BINARIES[@]}"
do
    /usr/bin/ninja -C "${SKQP_OUT_DIR}" "${BINARY}"
    # Strip binary, since gn is not stripping it even when `is_debug == false`
    ${STRIP_CMD:-strip} "${SKQP_OUT_DIR}/${BINARY}"
    install -m 0755 "${SKQP_OUT_DIR}/${BINARY}" "${SKQP_INSTALL_DIR}"
done

# Move assets to the target directory, which will reside in rootfs.
mv platform_tools/android/apps/skqp/src/main/assets/ "${SKQP_ASSETS_DIR}"

popd
rm -Rf "${SKIA_DIR}"

set +ex
