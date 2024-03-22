#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting
# shellcheck disable=SC2153

set -ex

mkdir -p kernel
pushd kernel

if [[ ${DEBIAN_ARCH} = "arm64" ]]; then
    KERNEL_IMAGE_NAME+=" cheza-kernel"
fi

for image in ${KERNEL_IMAGE_NAME}; do
    curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
      -o "/lava-files/${image}" "${KERNEL_IMAGE_BASE}/${DEBIAN_ARCH}/${image}"
done

for dtb in ${DEVICE_TREES}; do
    curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
      -o "/lava-files/${dtb}" "${KERNEL_IMAGE_BASE}/${DEBIAN_ARCH}/${dtb}"
  done

mkdir -p "/lava-files/rootfs-${DEBIAN_ARCH}"
curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
  -O "${KERNEL_IMAGE_BASE}/${DEBIAN_ARCH}/modules.tar.zst"
tar --keep-directory-symlink --zstd -xf modules.tar.zst -C "/lava-files/rootfs-${DEBIAN_ARCH}/"

popd
rm -rf kernel

