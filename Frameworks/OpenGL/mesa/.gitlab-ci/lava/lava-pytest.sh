#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# © Collabora Limited
# Author: Guilherme Gallo <guilherme.gallo@collabora.com>

# This script runs unit/integration tests related with LAVA CI tools
# shellcheck disable=SC1091 # The relative paths in this file only become valid at runtime.

set -ex

# Use this script in a python virtualenv for isolation
python3 -m venv .venv
. .venv/bin/activate
python3 -m pip install --break-system-packages -r "${CI_PROJECT_DIR}/.gitlab-ci/lava/requirements-test.txt"

TEST_DIR=${CI_PROJECT_DIR}/.gitlab-ci/tests

PYTHONPATH="${TEST_DIR}:${PYTHONPATH}" python3 -m \
    pytest "${TEST_DIR}" \
            -W ignore::DeprecationWarning \
            --junitxml=artifacts/ci_scripts_report.xml \
            -m 'not slow'
