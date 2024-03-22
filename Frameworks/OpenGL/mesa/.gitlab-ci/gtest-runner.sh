#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

set -ex

INSTALL=$PWD/install

# Set up the driver environment.
export LD_LIBRARY_PATH=$INSTALL/lib/

RESULTS="$PWD/${GTEST_RESULTS_DIR:-results}"
mkdir -p "$RESULTS"

export LIBVA_DRIVERS_PATH=$INSTALL/lib/dri/
# libva spams driver open info by default, and that happens per testcase.
export LIBVA_MESSAGING_LEVEL=1

if [ -e "$INSTALL/$GPU_VERSION-fails.txt" ]; then
    GTEST_RUNNER_OPTIONS="$GTEST_RUNNER_OPTIONS --baseline $INSTALL/$GPU_VERSION-fails.txt"
fi

# Default to an empty known flakes file if it doesn't exist.
touch "$INSTALL/$GPU_VERSION-flakes.txt"

if [ -n "$GALLIUM_DRIVER" ] && [ -e "$INSTALL/$GALLIUM_DRIVER-skips.txt" ]; then
    GTEST_SKIPS="$GTEST_SKIPS --skips $INSTALL/$GALLIUM_DRIVER-skips.txt"
fi

if [ -n "$DRIVER_NAME" ] && [ -e "$INSTALL/$DRIVER_NAME-skips.txt" ]; then
    GTEST_SKIPS="$GTEST_SKIPS --skips $INSTALL/$DRIVER_NAME-skips.txt"
fi

if [ -e "$INSTALL/$GPU_VERSION-skips.txt" ]; then
    GTEST_SKIPS="$GTEST_SKIPS --skips $INSTALL/$GPU_VERSION-skips.txt"
fi

set +e

gtest-runner \
    run \
    --gtest $GTEST \
    --output ${RESULTS} \
    --jobs ${FDO_CI_CONCURRENT:-4} \
    $GTEST_SKIPS \
    --flakes $INSTALL/$GPU_VERSION-flakes.txt \
    --fraction-start ${CI_NODE_INDEX:-1} \
    --fraction $((${CI_NODE_TOTAL:-1} * ${GTEST_FRACTION:-1})) \
    --env "LD_PRELOAD=$TEST_LD_PRELOAD" \
    $GTEST_RUNNER_OPTIONS

GTEST_EXITCODE=$?

deqp-runner junit \
   --testsuite gtest \
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

exit $GTEST_EXITCODE
