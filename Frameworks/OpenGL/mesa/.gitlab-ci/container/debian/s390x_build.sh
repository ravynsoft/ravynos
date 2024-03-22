#!/usr/bin/env bash

set -e

arch=s390x

# Ephemeral packages (installed for this script and removed again at the end)
EPHEMERAL=(
    libssl-dev
)

apt-get -y install "${EPHEMERAL[@]}"

. .gitlab-ci/container/build-mold.sh

apt-get purge -y "${EPHEMERAL[@]}"

. .gitlab-ci/container/cross_build.sh
