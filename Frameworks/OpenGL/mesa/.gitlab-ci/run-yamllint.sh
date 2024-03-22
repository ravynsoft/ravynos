#!/usr/bin/env bash
set -e

# Run yamllint against all traces files.
find . -name '*traces*yml' -print0 | xargs -0 yamllint -d "{rules: {line-length: {max: 1000}}}"
