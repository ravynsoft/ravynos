#!/usr/bin/env bash

# This is a ci-templates build script to generate a container for LAVA SSH client.

# shellcheck disable=SC1091
set -e
set -o xtrace

EPHEMERAL=(
)

# We only need these very basic packages to run the tests.
DEPS=(
    openssh-client  # for ssh
    iputils         # for ping
    bash
    curl
)


apk --no-cache add "${DEPS[@]}" "${EPHEMERAL[@]}"

. .gitlab-ci/container/container_pre_build.sh

############### Uninstall the build software

apk del "${EPHEMERAL[@]}"

. .gitlab-ci/container/container_post_build.sh
