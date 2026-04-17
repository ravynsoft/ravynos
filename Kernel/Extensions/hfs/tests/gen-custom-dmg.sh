#!/bin/sh
set -e

mkdir -p "$DERIVED_FILE_DIR"
env -i xcrun -sdk macosx.internal clang "$SRCROOT"/tests/generate-compressed-image.c -lz -o "$DERIVED_FILE_DIR"/generate-compressed-image

"$DERIVED_FILE_DIR"/generate-compressed-image -size 1g -type SPARSE -fs "$1" -uid 501 -gid 501 >"$2"

echo "Created $2 of type $1"
