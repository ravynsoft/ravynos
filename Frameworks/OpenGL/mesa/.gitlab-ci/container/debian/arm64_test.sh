#!/usr/bin/env bash

set -e

arch=arm64 . .gitlab-ci/container/debian/arm_test.sh
