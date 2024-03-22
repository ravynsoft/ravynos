#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

set -ex

if [[ -z "$VK_DRIVER" ]]; then
    exit 1
fi

# Useful debug output, you rarely know what envirnoment you'll be
# running in within container-land, this can be a landmark.
ls -l

INSTALL=$(realpath -s "$PWD"/install)
RESULTS=$(realpath -s "$PWD"/results)

# Set up the driver environment.
# Modifiying here directly LD_LIBRARY_PATH may cause problems when
# using a command wrapper. Hence, we will just set it when running the
# command.
export __LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$INSTALL/lib/"

# Sanity check to ensure that our environment is sufficient to make our tests
# run against the Mesa built by CI, rather than any installed distro version.
MESA_VERSION=$(sed 's/\./\\./g' "$INSTALL/VERSION")

# Force the stdout and stderr streams to be unbuffered in python.
export PYTHONUNBUFFERED=1

# Set the Vulkan driver to use.
export VK_ICD_FILENAMES="$INSTALL/share/vulkan/icd.d/${VK_DRIVER}_icd.x86_64.json"
if [ "${VK_DRIVER}" = "radeon" ]; then
    # Disable vsync
    export MESA_VK_WSI_PRESENT_MODE=mailbox
    export vblank_mode=0
fi

# Set environment for Wine.
export WINEDEBUG="-all"
export WINEPREFIX="/dxvk-wine64"
export WINEESYNC=1

# Wait for amdgpu to be fully loaded
sleep 1

# Avoid having to perform nasty command pre-processing to insert the
# wine executable in front of the test executables. Instead, use the
# kernel's binfmt support to automatically use Wine as an interpreter
# when asked to load PE executables.
# TODO: Have boot2container mount this filesystem for all jobs?
mount -t binfmt_misc none /proc/sys/fs/binfmt_misc
echo ':DOSWin:M::MZ::/usr/bin/wine64:' > /proc/sys/fs/binfmt_misc/register

# Set environment for DXVK.
export DXVK_LOG_LEVEL="info"
export DXVK_LOG="$RESULTS/dxvk"
[ -d "$DXVK_LOG" ] || mkdir -pv "$DXVK_LOG"
export DXVK_STATE_CACHE=0

# Set environment for replaying traces.
export PATH="/apitrace-msvc-win64/bin:/gfxreconstruct/build/bin:$PATH"

SANITY_MESA_VERSION_CMD="vulkaninfo"

# Set up the Window System Interface (WSI)
# TODO: Can we get away with GBM?
if [ "${TEST_START_XORG:-0}" -eq 1 ]; then
    "$INSTALL"/common/start-x.sh "$INSTALL"
    export DISPLAY=:0
fi

wine64 --version

SANITY_MESA_VERSION_CMD="$SANITY_MESA_VERSION_CMD | tee /tmp/version.txt | grep \"Mesa $MESA_VERSION\(\s\|$\)\""

RUN_CMD="export LD_LIBRARY_PATH=$__LD_LIBRARY_PATH; $SANITY_MESA_VERSION_CMD"

set +e
if ! eval $RUN_CMD;
then
    printf "%s\n" "Found $(cat /tmp/version.txt), expected $MESA_VERSION"
fi
set -e

# Just to be sure...
chmod +x ./valvetraces-run.sh
./valvetraces-run.sh
