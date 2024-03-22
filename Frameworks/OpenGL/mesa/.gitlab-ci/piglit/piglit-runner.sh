#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

set -ex

if [ -z "$GPU_VERSION" ]; then
   echo 'GPU_VERSION must be set to something like "llvmpipe" or "freedreno-a630" (the name used in your ci/gpu-version-*.txt)'
   exit 1
fi

INSTALL="$PWD/install"

# Set up the driver environment.
export LD_LIBRARY_PATH="$INSTALL/lib/"
export EGL_PLATFORM=surfaceless
export VK_ICD_FILENAMES="$INSTALL/share/vulkan/icd.d/${VK_DRIVER}_icd.${VK_CPU:-$(uname -m)}.json"

RESULTS=$PWD/${PIGLIT_RESULTS_DIR:-results}
mkdir -p $RESULTS

# Ensure Mesa Shader Cache resides on tmpfs.
SHADER_CACHE_HOME=${XDG_CACHE_HOME:-${HOME}/.cache}
SHADER_CACHE_DIR=${MESA_SHADER_CACHE_DIR:-${SHADER_CACHE_HOME}/mesa_shader_cache}

findmnt -n tmpfs ${SHADER_CACHE_HOME} || findmnt -n tmpfs ${SHADER_CACHE_DIR} || {
    mkdir -p ${SHADER_CACHE_DIR}
    mount -t tmpfs -o nosuid,nodev,size=2G,mode=1755 tmpfs ${SHADER_CACHE_DIR}
}

if [ "$GALLIUM_DRIVER" = "virpipe" ]; then
    # deqp is to use virpipe, and virgl_test_server llvmpipe
    export GALLIUM_DRIVER="$GALLIUM_DRIVER"

    VTEST_ARGS="--use-egl-surfaceless"
    if [ "$VIRGL_HOST_API" = "GLES" ]; then
        VTEST_ARGS="$VTEST_ARGS --use-gles"
    fi

    GALLIUM_DRIVER=llvmpipe \
    GALLIVM_PERF="nopt" \
    virgl_test_server $VTEST_ARGS >$RESULTS/vtest-log.txt 2>&1 &

    sleep 1
fi

if [ -n "$PIGLIT_FRACTION" ] || [ -n "$CI_NODE_INDEX" ]; then
    FRACTION=$((${PIGLIT_FRACTION:-1} * ${CI_NODE_TOTAL:-1}))
PIGLIT_RUNNER_OPTIONS="$PIGLIT_RUNNER_OPTIONS --fraction $FRACTION"
fi

# If the job is parallel at the gitab job level, take the corresponding fraction
# of the caselist.
if [ -n "$CI_NODE_INDEX" ]; then
   PIGLIT_RUNNER_OPTIONS="$PIGLIT_RUNNER_OPTIONS --fraction-start ${CI_NODE_INDEX}"
fi

if [ -e "$INSTALL/$GPU_VERSION-fails.txt" ]; then
    PIGLIT_RUNNER_OPTIONS="$PIGLIT_RUNNER_OPTIONS --baseline $INSTALL/$GPU_VERSION-fails.txt"
fi

# Default to an empty known flakes file if it doesn't exist.
touch $INSTALL/$GPU_VERSION-flakes.txt

if [ -n "$VK_DRIVER" ] && [ -e "$INSTALL/$VK_DRIVER-skips.txt" ]; then
    PIGLIT_SKIPS="$PIGLIT_SKIPS $INSTALL/$VK_DRIVER-skips.txt"
fi

if [ -n "$GALLIUM_DRIVER" ] && [ -e "$INSTALL/$GALLIUM_DRIVER-skips.txt" ]; then
    PIGLIT_SKIPS="$PIGLIT_SKIPS $INSTALL/$GALLIUM_DRIVER-skips.txt"
fi

if [ -n "$DRIVER_NAME" ] && [ -e "$INSTALL/$DRIVER_NAME-skips.txt" ]; then
    PIGLIT_SKIPS="$PIGLIT_SKIPS $INSTALL/$DRIVER_NAME-skips.txt"
fi

if [ -e "$INSTALL/$GPU_VERSION-skips.txt" ]; then
    PIGLIT_SKIPS="$PIGLIT_SKIPS $INSTALL/$GPU_VERSION-skips.txt"
fi

if [ "$PIGLIT_PLATFORM" != "gbm" ] ; then
    PIGLIT_SKIPS="$PIGLIT_SKIPS $INSTALL/x11-skips.txt"
fi

if [ "$PIGLIT_PLATFORM" = "gbm" ]; then
    PIGLIT_SKIPS="$PIGLIT_SKIPS $INSTALL/gbm-skips.txt"
fi

set +e

piglit-runner \
    run \
    --piglit-folder /piglit \
    --output $RESULTS \
    --jobs ${FDO_CI_CONCURRENT:-4} \
    --skips $INSTALL/all-skips.txt $PIGLIT_SKIPS \
    --flakes $INSTALL/$GPU_VERSION-flakes.txt \
    --profile $PIGLIT_PROFILES \
    --process-isolation \
    $PIGLIT_RUNNER_OPTIONS \
    -v -v

PIGLIT_EXITCODE=$?

deqp-runner junit \
   --testsuite $PIGLIT_PROFILES \
   --results $RESULTS/failures.csv \
   --output $RESULTS/junit.xml \
   --limit 50 \
   --template "See $ARTIFACTS_BASE_URL/results/{{testcase}}.xml"

# Report the flakes to the IRC channel for monitoring (if configured):
if [ -n "$FLAKES_CHANNEL" ]; then
  python3 $INSTALL/report-flakes.py \
         --host irc.oftc.net \
         --port 6667 \
         --results $RESULTS/results.csv \
         --known-flakes $INSTALL/$GPU_VERSION-flakes.txt \
         --channel "$FLAKES_CHANNEL" \
         --runner "$CI_RUNNER_DESCRIPTION" \
         --job "$CI_JOB_ID" \
         --url "$CI_JOB_URL" \
         --branch "${CI_MERGE_REQUEST_SOURCE_BRANCH_NAME:-$CI_COMMIT_BRANCH}" \
         --branch-title "${CI_MERGE_REQUEST_TITLE:-$CI_COMMIT_TITLE}" || true
fi

# Compress results.csv to save on bandwidth during the upload of artifacts to
# GitLab. This reduces a full piglit run to 550 KB, down from 6 MB, and takes
# 55ms on my Ryzen 5950X (with or without parallelism).
zstd --rm -T0 -8qc $RESULTS/results.csv -o $RESULTS/results.csv.zst

exit $PIGLIT_EXITCODE
