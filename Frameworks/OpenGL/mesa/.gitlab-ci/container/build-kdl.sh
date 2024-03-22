#!/usr/bin/env bash
# shellcheck disable=SC1091  # the path is created by the script

set -ex

KDL_REVISION="5056f71b100a68b72b285c6fc845a66a2ed25985"

mkdir ci-kdl.git
pushd ci-kdl.git
git init
git remote add origin https://gitlab.freedesktop.org/gfx-ci/ci-kdl.git
git fetch --depth 1 origin ${KDL_REVISION}
git checkout FETCH_HEAD
popd

python3 -m venv ci-kdl.venv
source ci-kdl.venv/bin/activate
pushd ci-kdl.git
pip install -r requirements.txt
pip install .
popd

rm -rf ci-kdl.git
