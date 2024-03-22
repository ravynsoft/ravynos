#!/bin/bash
# shellcheck disable=SC1090
# shellcheck disable=SC1091
# shellcheck disable=SC2086 # we want word splitting
# shellcheck disable=SC2155

# Second-stage init, used to set up devices and our job environment before
# running tests.

# Make sure to kill itself and all the children process from this script on
# exiting, since any console output may interfere with LAVA signals handling,
# which based on the log console.
cleanup() {
  if [ "$BACKGROUND_PIDS" = "" ]; then
    return 0
  fi

  set +x
  echo "Killing all child processes"
  for pid in $BACKGROUND_PIDS
  do
    kill "$pid" 2>/dev/null || true
  done

  # Sleep just a little to give enough time for subprocesses to be gracefully
  # killed. Then apply a SIGKILL if necessary.
  sleep 5
  for pid in $BACKGROUND_PIDS
  do
    kill -9 "$pid" 2>/dev/null || true
  done

  BACKGROUND_PIDS=
  set -x
}
trap cleanup INT TERM EXIT

# Space separated values with the PIDS of the processes started in the
# background by this script
BACKGROUND_PIDS=


for path in '/dut-env-vars.sh' '/set-job-env-vars.sh' './set-job-env-vars.sh'; do
    [ -f "$path" ] && source "$path"
done
. "$SCRIPTS_DIR"/setup-test-env.sh

set -ex

# Set up any devices required by the jobs
[ -z "$HWCI_KERNEL_MODULES" ] || {
    echo -n $HWCI_KERNEL_MODULES | xargs -d, -n1 /usr/sbin/modprobe
}

# Set up ZRAM
HWCI_ZRAM_SIZE=2G
if /sbin/zramctl --find --size $HWCI_ZRAM_SIZE -a zstd; then
    mkswap /dev/zram0
    swapon /dev/zram0
    echo "zram: $HWCI_ZRAM_SIZE activated"
else
    echo "zram: skipping, not supported"
fi

#
# Load the KVM module specific to the detected CPU virtualization extensions:
# - vmx for Intel VT
# - svm for AMD-V
#
# Additionally, download the kernel image to boot the VM via HWCI_TEST_SCRIPT.
#
if [ "$HWCI_KVM" = "true" ]; then
    unset KVM_KERNEL_MODULE
    {
      grep -qs '\bvmx\b' /proc/cpuinfo && KVM_KERNEL_MODULE=kvm_intel
    } || {
      grep -qs '\bsvm\b' /proc/cpuinfo && KVM_KERNEL_MODULE=kvm_amd
    }

    {
      [ -z "${KVM_KERNEL_MODULE}" ] && \
      echo "WARNING: Failed to detect CPU virtualization extensions"
    } || \
        modprobe ${KVM_KERNEL_MODULE}

    mkdir -p /lava-files
    curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
	-o "/lava-files/${KERNEL_IMAGE_NAME}" \
        "${KERNEL_IMAGE_BASE}/amd64/${KERNEL_IMAGE_NAME}"
fi

# Fix prefix confusion: the build installs to $CI_PROJECT_DIR, but we expect
# it in /install
ln -sf $CI_PROJECT_DIR/install /install
export LD_LIBRARY_PATH=/install/lib
export LIBGL_DRIVERS_PATH=/install/lib/dri

# https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/22495#note_1876691
# The navi21 boards seem to have trouble with ld.so.cache, so try explicitly
# telling it to look in /usr/local/lib.
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

# Store Mesa's disk cache under /tmp, rather than sending it out over NFS.
export XDG_CACHE_HOME=/tmp

# Make sure Python can find all our imports
export PYTHONPATH=$(python3 -c "import sys;print(\":\".join(sys.path))")

if [ "$HWCI_FREQ_MAX" = "true" ]; then
  # Ensure initialization of the DRM device (needed by MSM)
  head -0 /dev/dri/renderD128

  # Disable GPU frequency scaling
  DEVFREQ_GOVERNOR=$(find /sys/devices -name governor | grep gpu || true)
  test -z "$DEVFREQ_GOVERNOR" || echo performance > $DEVFREQ_GOVERNOR || true

  # Disable CPU frequency scaling
  echo performance | tee -a /sys/devices/system/cpu/cpufreq/policy*/scaling_governor || true

  # Disable GPU runtime power management
  GPU_AUTOSUSPEND=$(find /sys/devices -name autosuspend_delay_ms | grep gpu | head -1)
  test -z "$GPU_AUTOSUSPEND" || echo -1 > $GPU_AUTOSUSPEND || true
  # Lock Intel GPU frequency to 70% of the maximum allowed by hardware
  # and enable throttling detection & reporting.
  # Additionally, set the upper limit for CPU scaling frequency to 65% of the
  # maximum permitted, as an additional measure to mitigate thermal throttling.
  /intel-gpu-freq.sh -s 70% --cpu-set-max 65% -g all -d
fi

# Start a little daemon to capture sysfs records and produce a JSON file
if [ -x /kdl.sh ]; then
  echo "launch kdl.sh!"
  /kdl.sh &
  BACKGROUND_PIDS="$! $BACKGROUND_PIDS"
else
  echo "kdl.sh not found!"
fi

# Increase freedreno hangcheck timer because it's right at the edge of the
# spilling tests timing out (and some traces, too)
if [ -n "$FREEDRENO_HANGCHECK_MS" ]; then
    echo $FREEDRENO_HANGCHECK_MS | tee -a /sys/kernel/debug/dri/128/hangcheck_period_ms
fi

# Start a little daemon to capture the first devcoredump we encounter.  (They
# expire after 5 minutes, so we poll for them).
if [ -x /capture-devcoredump.sh ]; then
  /capture-devcoredump.sh &
  BACKGROUND_PIDS="$! $BACKGROUND_PIDS"
fi

# If we want Xorg to be running for the test, then we start it up before the
# HWCI_TEST_SCRIPT because we need to use xinit to start X (otherwise
# without using -displayfd you can race with Xorg's startup), but xinit will eat
# your client's return code
if [ -n "$HWCI_START_XORG" ]; then
  echo "touch /xorg-started; sleep 100000" > /xorg-script
  env \
    VK_ICD_FILENAMES="/install/share/vulkan/icd.d/${VK_DRIVER}_icd.$(uname -m).json" \
    xinit /bin/sh /xorg-script -- /usr/bin/Xorg -noreset -s 0 -dpms -logfile /Xorg.0.log &
  BACKGROUND_PIDS="$! $BACKGROUND_PIDS"

  # Wait for xorg to be ready for connections.
  for _ in 1 2 3 4 5; do
    if [ -e /xorg-started ]; then
      break
    fi
    sleep 5
  done
  export DISPLAY=:0
fi

if [ -n "$HWCI_START_WESTON" ]; then
  WESTON_X11_SOCK="/tmp/.X11-unix/X0"
  if [ -n "$HWCI_START_XORG" ]; then
    echo "Please consider dropping HWCI_START_XORG and instead using Weston XWayland for testing."
    WESTON_X11_SOCK="/tmp/.X11-unix/X1"
  fi
  export WAYLAND_DISPLAY=wayland-0

  # Display server is Weston Xwayland when HWCI_START_XORG is not set or Xorg when it's
  export DISPLAY=:0
  mkdir -p /tmp/.X11-unix

  env \
    VK_ICD_FILENAMES="/install/share/vulkan/icd.d/${VK_DRIVER}_icd.$(uname -m).json" \
    weston -Bheadless-backend.so --use-gl -Swayland-0 --xwayland --idle-time=0 &
  BACKGROUND_PIDS="$! $BACKGROUND_PIDS"

  while [ ! -S "$WESTON_X11_SOCK" ]; do sleep 1; done
fi

set +e
bash -c ". $SCRIPTS_DIR/setup-test-env.sh && $HWCI_TEST_SCRIPT"
EXIT_CODE=$?
set -e

# Let's make sure the results are always stored in current working directory
mv -f ${CI_PROJECT_DIR}/results ./ 2>/dev/null || true

[ ${EXIT_CODE} -ne 0 ] || rm -rf results/trace/"$PIGLIT_REPLAY_DEVICE_NAME"

# Make sure that capture-devcoredump is done before we start trying to tar up
# artifacts -- if it's writing while tar is reading, tar will throw an error and
# kill the job.
cleanup

# upload artifacts
if [ -n "$S3_RESULTS_UPLOAD" ]; then
  tar --zstd -cf results.tar.zst results/;
  ci-fairy s3cp --token-file "${CI_JOB_JWT_FILE}" results.tar.zst https://"$S3_RESULTS_UPLOAD"/results.tar.zst;
fi

# We still need to echo the hwci: mesa message, as some scripts rely on it, such
# as the python ones inside the bare-metal folder
[ ${EXIT_CODE} -eq 0 ] && RESULT=pass || RESULT=fail

set +x

# Print the final result; both bare-metal and LAVA look for this string to get
# the result of our run, so try really hard to get it out rather than losing
# the run. The device gets shut down right at this point, and a630 seems to
# enjoy corrupting the last line of serial output before shutdown.
for _ in $(seq 0 3); do echo "hwci: mesa: $RESULT"; sleep 1; echo; done

exit $EXIT_CODE
