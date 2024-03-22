#!/bin/bash

set -e

STRINGS=$(mktemp)
ERRORS=$(mktemp)

trap 'rm $STRINGS; rm $ERRORS;' EXIT

FILE=$1
shift 1

while getopts "f:e:" opt; do
  case $opt in
    f) echo "$OPTARG" >> "$STRINGS";;
    e) echo "$OPTARG" >> "$STRINGS" ; echo "$OPTARG" >> "$ERRORS";;
    *) exit
  esac
done
shift $((OPTIND -1))

echo "Waiting for $FILE to say one of following strings"
cat "$STRINGS"

while ! grep -E -wf "$STRINGS" "$FILE"; do
  sleep 2
done

if grep -E -wf "$ERRORS" "$FILE"; then
  exit 1
fi
