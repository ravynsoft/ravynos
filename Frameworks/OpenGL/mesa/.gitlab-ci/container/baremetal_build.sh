#!/usr/bin/env bash

set -e
set -o xtrace

# Fetch the arm-built rootfs image and unpack it in our x86_64 container (saves
# network transfer, disk usage, and runtime on test jobs)

# shellcheck disable=SC2154 # arch is assigned in previous scripts
if curl -X HEAD -s "${ARTIFACTS_PREFIX}/${FDO_UPSTREAM_REPO}/${ARTIFACTS_SUFFIX}/${arch}/done"; then
  ARTIFACTS_URL="${ARTIFACTS_PREFIX}/${FDO_UPSTREAM_REPO}/${ARTIFACTS_SUFFIX}/${arch}"
else
  ARTIFACTS_URL="${ARTIFACTS_PREFIX}/${CI_PROJECT_PATH}/${ARTIFACTS_SUFFIX}/${arch}"
fi

curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
    "${ARTIFACTS_URL}"/lava-rootfs.tar.zst -o rootfs.tar.zst
mkdir -p /rootfs-"$arch"
tar -C /rootfs-"$arch" '--exclude=./dev/*' --zstd -xf rootfs.tar.zst
rm rootfs.tar.zst

if [[ $arch == "arm64" ]]; then
    mkdir -p /baremetal-files
    pushd /baremetal-files

    curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
	-O "${KERNEL_IMAGE_BASE}"/arm64/Image
    curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
        -O "${KERNEL_IMAGE_BASE}"/arm64/Image.gz
    curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
        -O "${KERNEL_IMAGE_BASE}"/arm64/cheza-kernel

    DEVICE_TREES=""
    DEVICE_TREES="$DEVICE_TREES apq8016-sbc.dtb"
    DEVICE_TREES="$DEVICE_TREES apq8096-db820c.dtb"
    DEVICE_TREES="$DEVICE_TREES tegra210-p3450-0000.dtb"
    DEVICE_TREES="$DEVICE_TREES imx8mq-nitrogen.dtb"

    for DTB in $DEVICE_TREES; do
	curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
            -O "${KERNEL_IMAGE_BASE}/arm64/$DTB"
    done

    popd
elif [[ $arch == "armhf" ]]; then
    mkdir -p /baremetal-files
    pushd /baremetal-files

    curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
        -O "${KERNEL_IMAGE_BASE}"/armhf/zImage

    DEVICE_TREES=""
    DEVICE_TREES="$DEVICE_TREES imx6q-cubox-i.dtb"
    DEVICE_TREES="$DEVICE_TREES tegra124-jetson-tk1.dtb"

    for DTB in $DEVICE_TREES; do
	curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
            -O "${KERNEL_IMAGE_BASE}/armhf/$DTB"
    done

    popd
fi
