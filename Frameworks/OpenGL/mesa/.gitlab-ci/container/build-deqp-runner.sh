#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

# When changing this file, you need to bump the following
# .gitlab-ci/image-tags.yml tags:
# DEBIAN_BASE_TAG
# DEBIAN_X86_64_TEST_ANDROID_TAG
# KERNEL_ROOTFS_TAG

set -ex

DEQP_RUNNER_VERSION=0.18.0

if [ -n "${DEQP_RUNNER_GIT_TAG}${DEQP_RUNNER_GIT_REV}" ]; then
    # Build and install from source
    DEQP_RUNNER_CARGO_ARGS="--git ${DEQP_RUNNER_GIT_URL:-https://gitlab.freedesktop.org/anholt/deqp-runner.git}"

    if [ -n "${DEQP_RUNNER_GIT_TAG}" ]; then
        DEQP_RUNNER_CARGO_ARGS="--tag ${DEQP_RUNNER_GIT_TAG} ${DEQP_RUNNER_CARGO_ARGS}"
    else
        DEQP_RUNNER_CARGO_ARGS="--rev ${DEQP_RUNNER_GIT_REV} ${DEQP_RUNNER_CARGO_ARGS}"
    fi

    DEQP_RUNNER_CARGO_ARGS="${DEQP_RUNNER_CARGO_ARGS} ${EXTRA_CARGO_ARGS}"
else
    # Install from package registry
    DEQP_RUNNER_CARGO_ARGS="--version ${DEQP_RUNNER_VERSION} ${EXTRA_CARGO_ARGS} -- deqp-runner"
fi

if [ -z "$ANDROID_NDK_HOME" ]; then
    cargo install --locked  \
        -j ${FDO_CI_CONCURRENT:-4} \
        --root /usr/local \
        ${DEQP_RUNNER_CARGO_ARGS}
else
    mkdir -p /deqp-runner
    pushd /deqp-runner
    git clone --branch v${DEQP_RUNNER_VERSION} --depth 1 https://gitlab.freedesktop.org/anholt/deqp-runner.git deqp-runner-git
    pushd deqp-runner-git

    cargo install --locked  \
        -j ${FDO_CI_CONCURRENT:-4} \
        --root /usr/local --version 2.10.0 \
        cargo-ndk

    rustup target add x86_64-linux-android
    RUSTFLAGS='-C target-feature=+crt-static' cargo ndk --target x86_64-linux-android build

    mv target/x86_64-linux-android/debug/deqp-runner /deqp-runner

    cargo uninstall --locked  \
        --root /usr/local \
        cargo-ndk

    popd
    rm -rf deqp-runner-git
    popd
fi

# remove unused test runners to shrink images for the Mesa CI build (not kernel,
# which chooses its own deqp branch)
if [ -z "${DEQP_RUNNER_GIT_TAG}${DEQP_RUNNER_GIT_REV}" ]; then
    rm -f /usr/local/bin/igt-runner
fi
