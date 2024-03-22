#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

section_start test_setup "deqp: preparing test setup"

set -ex

# Needed so configuration files can contain paths to files in /install
ln -sf "$CI_PROJECT_DIR"/install /install

if [ -z "$GPU_VERSION" ]; then
   echo 'GPU_VERSION must be set to something like "llvmpipe" or "freedreno-a630" (the name used in .gitlab-ci/gpu-version-*.txt)'
   exit 1
fi

INSTALL=$(realpath -s "$PWD"/install)

# Set up the driver environment.
export LD_LIBRARY_PATH="$INSTALL"/lib/:$LD_LIBRARY_PATH
export EGL_PLATFORM=surfaceless
export VK_ICD_FILENAMES="$PWD"/install/share/vulkan/icd.d/"$VK_DRIVER"_icd.${VK_CPU:-$(uname -m)}.json
export OCL_ICD_VENDORS="$PWD"/install/etc/OpenCL/vendors/

if [ -n "$USE_ANGLE" ]; then
  export LD_LIBRARY_PATH=/angle:$LD_LIBRARY_PATH
fi

RESULTS="$PWD/${DEQP_RESULTS_DIR:-results}"
mkdir -p "$RESULTS"

# Ensure Mesa Shader Cache resides on tmpfs.
SHADER_CACHE_HOME=${XDG_CACHE_HOME:-${HOME}/.cache}
SHADER_CACHE_DIR=${MESA_SHADER_CACHE_DIR:-${SHADER_CACHE_HOME}/mesa_shader_cache}

findmnt -n tmpfs ${SHADER_CACHE_HOME} || findmnt -n tmpfs ${SHADER_CACHE_DIR} || {
    mkdir -p ${SHADER_CACHE_DIR}
    mount -t tmpfs -o nosuid,nodev,size=2G,mode=1755 tmpfs ${SHADER_CACHE_DIR}
}

if [ -z "$DEQP_SUITE" ]; then
    if [ -z "$DEQP_VER" ]; then
        echo 'DEQP_SUITE must be set to the name of your deqp-gpu_version.toml, or DEQP_VER must be set to something like "gles2", "gles31-khr" or "vk" for the test run'
        exit 1
    fi

    DEQP_WIDTH=${DEQP_WIDTH:-256}
    DEQP_HEIGHT=${DEQP_HEIGHT:-256}
    DEQP_CONFIG=${DEQP_CONFIG:-rgba8888d24s8ms0}
    DEQP_VARIANT=${DEQP_VARIANT:-master}

    DEQP_OPTIONS="$DEQP_OPTIONS --deqp-surface-width=$DEQP_WIDTH --deqp-surface-height=$DEQP_HEIGHT"
    DEQP_OPTIONS="$DEQP_OPTIONS --deqp-surface-type=${DEQP_SURFACE_TYPE:-pbuffer}"
    DEQP_OPTIONS="$DEQP_OPTIONS --deqp-gl-config-name=$DEQP_CONFIG"
    DEQP_OPTIONS="$DEQP_OPTIONS --deqp-visibility=hidden"

    if [ "$DEQP_VER" = "vk" ] && [ -z "$VK_DRIVER" ]; then
        echo 'VK_DRIVER must be to something like "radeon" or "intel" for the test run'
        exit 1
    fi

    # Generate test case list file.
    if [ "$DEQP_VER" = "vk" ]; then
       MUSTPASS=/deqp/mustpass/vk-$DEQP_VARIANT.txt
       DEQP=/deqp/external/vulkancts/modules/vulkan/deqp-vk
    elif [ "$DEQP_VER" = "gles2" ] || [ "$DEQP_VER" = "gles3" ] || [ "$DEQP_VER" = "gles31" ] || [ "$DEQP_VER" = "egl" ]; then
       MUSTPASS=/deqp/mustpass/$DEQP_VER-$DEQP_VARIANT.txt
       DEQP=/deqp/modules/$DEQP_VER/deqp-$DEQP_VER
    elif [ "$DEQP_VER" = "gles2-khr" ] || [ "$DEQP_VER" = "gles3-khr" ] || [ "$DEQP_VER" = "gles31-khr" ] || [ "$DEQP_VER" = "gles32-khr" ]; then
       MUSTPASS=/deqp/mustpass/$DEQP_VER-$DEQP_VARIANT.txt
       DEQP=/deqp/external/openglcts/modules/glcts
    else
       MUSTPASS=/deqp/mustpass/$DEQP_VER-$DEQP_VARIANT.txt
       DEQP=/deqp/external/openglcts/modules/glcts
    fi

    cp $MUSTPASS /tmp/case-list.txt

    # If the caselist is too long to run in a reasonable amount of time, let the job
    # specify what fraction (1/n) of the caselist we should run.  Note: N~M is a gnu
    # sed extension to match every nth line (first line is #1).
    if [ -n "$DEQP_FRACTION" ]; then
       sed -ni 1~$DEQP_FRACTION"p" /tmp/case-list.txt
    fi

    # If the job is parallel at the gitab job level, take the corresponding fraction
    # of the caselist.
    if [ -n "$CI_NODE_INDEX" ]; then
       sed -ni $CI_NODE_INDEX~$CI_NODE_TOTAL"p" /tmp/case-list.txt
    fi

    if [ ! -s /tmp/case-list.txt ]; then
        echo "Caselist generation failed"
        exit 1
    fi
fi

if [ -e "$INSTALL/$GPU_VERSION-fails.txt" ]; then
    DEQP_RUNNER_OPTIONS="$DEQP_RUNNER_OPTIONS --baseline $INSTALL/$GPU_VERSION-fails.txt"
fi

# Default to an empty known flakes file if it doesn't exist.
touch $INSTALL/$GPU_VERSION-flakes.txt


if [ -n "$VK_DRIVER" ] && [ -e "$INSTALL/$VK_DRIVER-skips.txt" ]; then
    DEQP_SKIPS="$DEQP_SKIPS $INSTALL/$VK_DRIVER-skips.txt"
fi

if [ -n "$GALLIUM_DRIVER" ] && [ -e "$INSTALL/$GALLIUM_DRIVER-skips.txt" ]; then
    DEQP_SKIPS="$DEQP_SKIPS $INSTALL/$GALLIUM_DRIVER-skips.txt"
fi

if [ -n "$DRIVER_NAME" ] && [ -e "$INSTALL/$DRIVER_NAME-skips.txt" ]; then
    DEQP_SKIPS="$DEQP_SKIPS $INSTALL/$DRIVER_NAME-skips.txt"
fi

if [ -e "$INSTALL/$GPU_VERSION-skips.txt" ]; then
    DEQP_SKIPS="$DEQP_SKIPS $INSTALL/$GPU_VERSION-skips.txt"
fi

if [ "$PIGLIT_PLATFORM" != "gbm" ] ; then
    DEQP_SKIPS="$DEQP_SKIPS $INSTALL/x11-skips.txt"
fi

if [ "$PIGLIT_PLATFORM" = "gbm" ]; then
    DEQP_SKIPS="$DEQP_SKIPS $INSTALL/gbm-skips.txt"
fi

if [ -n "$VK_DRIVER" ] && [ -z "$DEQP_SUITE" ]; then
    # Bump the number of tests per group to reduce the startup time of VKCTS.
    DEQP_RUNNER_OPTIONS="$DEQP_RUNNER_OPTIONS --tests-per-group ${DEQP_RUNNER_TESTS_PER_GROUP:-5000}"
fi

# Set the path to VK validation layer settings (in case it ends up getting loaded)
# Note: If you change the format of this filename, look through the rest of the
# tree for other places that need to be kept in sync (e.g.
# src/gallium/drivers/zink/ci/gitlab-ci-inc.yml)
export VK_LAYER_SETTINGS_PATH=$INSTALL/$GPU_VERSION-validation-settings.txt

report_load() {
    echo "System load: $(cut -d' ' -f1-3 < /proc/loadavg)"
    echo "# of CPU cores: $(grep -c processor /proc/cpuinfo)"
}

if [ "$GALLIUM_DRIVER" = "virpipe" ]; then
    # deqp is to use virpipe, and virgl_test_server llvmpipe
    export GALLIUM_DRIVER="$GALLIUM_DRIVER"

    VTEST_ARGS="--use-egl-surfaceless"
    if [ "$VIRGL_HOST_API" = "GLES" ]; then
        VTEST_ARGS="$VTEST_ARGS --use-gles"
    fi

    GALLIUM_DRIVER=llvmpipe \
    virgl_test_server $VTEST_ARGS >$RESULTS/vtest-log.txt 2>&1 &

    sleep 1
fi

if [ -z "$DEQP_SUITE" ]; then
    if [ -n "$DEQP_EXPECTED_RENDERER" ]; then
        export DEQP_RUNNER_OPTIONS="$DEQP_RUNNER_OPTIONS --renderer-check $DEQP_EXPECTED_RENDERER"
    fi
    if [ $DEQP_VER != vk ] && [ $DEQP_VER != egl ]; then
        VER=$(sed 's/[() ]/./g' "$INSTALL/VERSION")
        export DEQP_RUNNER_OPTIONS="$DEQP_RUNNER_OPTIONS --version-check $VER"
    fi
fi

uncollapsed_section_switch deqp "deqp: deqp-runner"

cat /deqp/version-log

set +e
if [ -z "$DEQP_SUITE" ]; then
    deqp-runner \
        run \
        --deqp $DEQP \
        --output $RESULTS \
        --caselist /tmp/case-list.txt \
        --skips $INSTALL/all-skips.txt $DEQP_SKIPS \
        --flakes $INSTALL/$GPU_VERSION-flakes.txt \
        --testlog-to-xml /deqp/executor/testlog-to-xml \
        --jobs ${FDO_CI_CONCURRENT:-4} \
        $DEQP_RUNNER_OPTIONS \
        -- \
        $DEQP_OPTIONS
else
    # If you change the format of the suite toml filenames or the
    # $GPU_VERSION-{fails,flakes,skips}.txt filenames, look through the rest
    # of the tree for other places that need to be kept in sync (e.g.
    # src/**/ci/gitlab-ci*.yml)
    deqp-runner \
        suite \
        --suite $INSTALL/deqp-$DEQP_SUITE.toml \
        --output $RESULTS \
        --skips $INSTALL/all-skips.txt $DEQP_SKIPS \
        --flakes $INSTALL/$GPU_VERSION-flakes.txt \
        --testlog-to-xml /deqp/executor/testlog-to-xml \
        --fraction-start $CI_NODE_INDEX \
        --fraction $((CI_NODE_TOTAL * ${DEQP_FRACTION:-1})) \
        --jobs ${FDO_CI_CONCURRENT:-4} \
        $DEQP_RUNNER_OPTIONS
fi

DEQP_EXITCODE=$?
set -e

set +x

report_load

section_switch test_post_process "deqp: post-processing test results"
set -x

# Remove all but the first 50 individual XML files uploaded as artifacts, to
# save fd.o space when you break everything.
find $RESULTS -name \*.xml | \
    sort -n |
    sed -n '1,+49!p' | \
    xargs rm -f

# If any QPA XMLs are there, then include the XSL/CSS in our artifacts.
find $RESULTS -name \*.xml \
    -exec cp /deqp/testlog.css /deqp/testlog.xsl "$RESULTS/" ";" \
    -quit

deqp-runner junit \
   --testsuite dEQP \
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
# GitLab. This reduces the size in a VKCTS run from 135 to 7.6MB, and takes
# 0.17s on a Ryzen 5950X (16 threads, 0.95s when limited to 1 thread).
zstd --rm -T0 -8q "$RESULTS/results.csv" -o "$RESULTS/results.csv.zst"

section_end test_post_process

exit $DEQP_EXITCODE
