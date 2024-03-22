#!/bin/bash
# shellcheck disable=SC1091 # The relative paths in this file only become valid at runtime.
# shellcheck disable=SC2034
# shellcheck disable=SC2086 # we want word splitting

# Boot script for Chrome OS devices attached to a servo debug connector, using
# NFS and TFTP to boot.

# We're run from the root of the repo, make a helper var for our paths
BM=$CI_PROJECT_DIR/install/bare-metal
CI_COMMON=$CI_PROJECT_DIR/install/common
CI_INSTALL=$CI_PROJECT_DIR/install

# Runner config checks
if [ -z "$BM_SERIAL" ]; then
  echo "Must set BM_SERIAL in your gitlab-runner config.toml [[runners]] environment"
  echo "This is the CPU serial device."
  exit 1
fi

if [ -z "$BM_SERIAL_EC" ]; then
  echo "Must set BM_SERIAL in your gitlab-runner config.toml [[runners]] environment"
  echo "This is the EC serial device for controlling board power"
  exit 1
fi

if [ ! -d /nfs ]; then
  echo "NFS rootfs directory needs to be mounted at /nfs by the gitlab runner"
  exit 1
fi

if [ ! -d /tftp ]; then
  echo "TFTP directory for this board needs to be mounted at /tftp by the gitlab runner"
  exit 1
fi

# job config checks
if [ -z "$BM_KERNEL" ]; then
  echo "Must set BM_KERNEL to your board's kernel FIT image"
  exit 1
fi

if [ -z "$BM_ROOTFS" ]; then
  echo "Must set BM_ROOTFS to your board's rootfs directory in the job's variables"
  exit 1
fi

if [ -z "$BM_CMDLINE" ]; then
  echo "Must set BM_CMDLINE to your board's kernel command line arguments"
  exit 1
fi

set -ex

# Clear out any previous run's artifacts.
rm -rf results/
mkdir -p results

# Create the rootfs in the NFS directory.  rm to make sure it's in a pristine
# state, since it's volume-mounted on the host.
rsync -a --delete $BM_ROOTFS/ /nfs/
mkdir -p /nfs/results
. $BM/rootfs-setup.sh /nfs

# Put the kernel/dtb image and the boot command line in the tftp directory for
# the board to find.  For normal Mesa development, we build the kernel and
# store it in the docker container that this script is running in.
#
# However, container builds are expensive, so when you're hacking on the
# kernel, it's nice to be able to skip the half hour container build and plus
# moving that container to the runner.  So, if BM_KERNEL is a URL, fetch it
# instead of looking in the container.  Note that the kernel build should be
# the output of:
#
# make Image.lzma
#
# mkimage \
#  -A arm64 \
#  -f auto \
#  -C lzma \
#  -d arch/arm64/boot/Image.lzma \
#  -b arch/arm64/boot/dts/qcom/sdm845-cheza-r3.dtb \
#  cheza-image.img

rm -rf /tftp/*
if echo "$BM_KERNEL" | grep -q http; then
  curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
      $BM_KERNEL -o /tftp/vmlinuz
elif [ -n "${FORCE_KERNEL_TAG}" ]; then
  curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
    "${FDO_HTTP_CACHE_URI:-}${KERNEL_IMAGE_BASE}/${DEBIAN_ARCH}/${BM_KERNEL}" -o /tftp/vmlinuz
  curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
    "${FDO_HTTP_CACHE_URI:-}${KERNEL_IMAGE_BASE}/${DEBIAN_ARCH}/modules.tar.zst" -o modules.tar.zst
  tar --keep-directory-symlink --zstd -xf modules.tar.zst -C "/nfs/"
  rm modules.tar.zst &
else
  cp /baremetal-files/"$BM_KERNEL" /tftp/vmlinuz
fi
echo "$BM_CMDLINE" > /tftp/cmdline

set +e
STRUCTURED_LOG_FILE=job_detail.json
python3 $CI_INSTALL/custom_logger.py ${STRUCTURED_LOG_FILE} --update dut_job_type "${DEVICE_TYPE}"
python3 $CI_INSTALL/custom_logger.py ${STRUCTURED_LOG_FILE} --update farm "${FARM}"
python3 $CI_INSTALL/custom_logger.py ${STRUCTURED_LOG_FILE} --create-dut-job dut_name "${CI_RUNNER_DESCRIPTION}"
python3 $CI_INSTALL/custom_logger.py ${STRUCTURED_LOG_FILE} --update-dut-time submit "${CI_JOB_STARTED_AT}"
python3 $BM/cros_servo_run.py \
        --cpu $BM_SERIAL \
        --ec $BM_SERIAL_EC \
        --test-timeout ${TEST_PHASE_TIMEOUT:-20}
ret=$?
python3 $CI_INSTALL/custom_logger.py ${STRUCTURED_LOG_FILE} --close-dut-job
python3 $CI_INSTALL/custom_logger.py ${STRUCTURED_LOG_FILE} --close
set -e

# Bring artifacts back from the NFS dir to the build dir where gitlab-runner
# will look for them.
cp -Rp /nfs/results/. results/
if [ -f "${STRUCTURED_LOG_FILE}" ]; then
  cp -p ${STRUCTURED_LOG_FILE} results/
  echo "Structured log file is available at https://${CI_PROJECT_ROOT_NAMESPACE}.pages.freedesktop.org/-/${CI_PROJECT_NAME}/-/jobs/${CI_JOB_ID}/artifacts/results/${STRUCTURED_LOG_FILE}"
fi

exit $ret
