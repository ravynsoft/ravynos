#!/usr/bin/env bash
set -eu

this_dir=$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")
readonly this_dir

exec \
  "$this_dir/../python-venv.sh" \
  "$this_dir/requirements.txt" \
  "$this_dir/update_traces_checksum.py" "$@"
