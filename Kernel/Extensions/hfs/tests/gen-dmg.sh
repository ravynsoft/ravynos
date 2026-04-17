#!/bin/bash

#  gen-dmg.sh
#  hfs
#
#  Created by Max Holden on 11/3/15.
#

set -e

mkdir -p "$DERIVED_FILE_DIR"
env -i xcrun -sdk macosx.internal clang "$SRCROOT"/tests/img-to-c.c -lz -o "$DERIVED_FILE_DIR"/img-to-c

"$DERIVED_FILE_DIR"/img-to-c -size 2g -type SPARSE -fs JHFS+ -uid 501 -gid 501 >"$1"

echo "Created $1"
