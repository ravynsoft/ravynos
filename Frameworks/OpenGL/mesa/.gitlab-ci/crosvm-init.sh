#!/usr/bin/env bash
# shellcheck disable=SC1091 # The relative paths in this file only become valid at runtime.
# shellcheck disable=SC2086 # we want word splitting
set -e

VSOCK_STDOUT=$1
VSOCK_STDERR=$2
VM_TEMP_DIR=$3

mount -t proc none /proc
mount -t sysfs none /sys
mkdir -p /dev/pts
mount -t devpts devpts /dev/pts
mkdir /dev/shm
mount -t tmpfs -o noexec,nodev,nosuid tmpfs /dev/shm
mount -t tmpfs tmpfs /tmp

. ${VM_TEMP_DIR}/crosvm-env.sh
. ${VM_TEMP_DIR}/setup-test-env.sh

# .gitlab-ci.yml script variable is using relative paths to install directory,
# so change to that dir before running `crosvm-script`
cd "${CI_PROJECT_DIR}"

# The exception is the dEQP binary, as it needs to run from its own directory
[ -z "${DEQP_BIN_DIR}" ] || cd "${DEQP_BIN_DIR}"

# Use a FIFO to collect relevant error messages
STDERR_FIFO=/tmp/crosvm-stderr.fifo
mkfifo -m 600 ${STDERR_FIFO}

dmesg --level crit,err,warn -w > ${STDERR_FIFO} &
DMESG_PID=$!

# Transfer the errors and crosvm-script output via a pair of virtio-vsocks
socat -d -u pipe:${STDERR_FIFO} vsock-listen:${VSOCK_STDERR} &
socat -d -U vsock-listen:${VSOCK_STDOUT} \
    system:"stdbuf -eL bash ${VM_TEMP_DIR}/crosvm-script.sh 2> ${STDERR_FIFO}; echo \$? > ${VM_TEMP_DIR}/exit_code",nofork

kill ${DMESG_PID}
wait

sync
poweroff -d -n -f || true

sleep 1   # Just in case init would exit before the kernel shuts down the VM
