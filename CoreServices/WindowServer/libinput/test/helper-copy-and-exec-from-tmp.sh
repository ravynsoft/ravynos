#!/bin/bash -x
#
# Usage: helper-copy-and-exec-from-tmp.sh /path/to/binary [args]
#
# Copies the given binary into a unique file in /tmp and executes it with
# [args]. Exits with the same return code as the binary did.

executable="$1"
shift

target_name=$(mktemp)
cp "$executable" "$target_name"
chmod +x "$target_name"

"$target_name" "$@"
rc=$?
rm "$target_name"
exit $rc
