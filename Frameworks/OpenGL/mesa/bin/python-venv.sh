#!/usr/bin/env bash
set -eu

readonly requirements_file=$1
shift

venv_dir="$(dirname "$requirements_file")"/.venv
readonly venv_dir
readonly venv_req=$venv_dir/requirements.txt
readonly venv_python_version=$venv_dir/python-version.txt

if [ -d "$venv_dir" ]
then
  if [ ! -r "$venv_python_version" ]
  then
    echo "Python environment predates Python version checks."
    echo "It might be invalid and needs to be regenerated."
    rm -rf "$venv_dir"
  elif ! cmp --quiet <(python --version) "$venv_python_version"
  then
    old=$(cat "$venv_python_version")
    new=$(python --version)
    echo "Python version has changed ($old -> $new)."
    echo "Python environment needs to be regenerated."
    unset old new
    rm -rf "$venv_dir"
  fi
fi

if ! [ -r "$venv_dir/bin/activate" ]
then
  echo "Creating Python environment..."
  python -m venv "$venv_dir"
  python --version > "$venv_python_version"
fi

# shellcheck disable=1091
source "$venv_dir/bin/activate"

if ! cmp --quiet "$requirements_file" "$venv_req"
then
  echo "$(realpath --relative-to="$PWD" "$requirements_file") has changed, re-installing..."
  pip --disable-pip-version-check install --requirement "$requirements_file"
  cp "$requirements_file" "$venv_req"
fi

python "$@"
