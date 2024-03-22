#!/usr/bin/env bash

set -e

arch=armhf . .gitlab-ci/container/debian/arm_test.sh
