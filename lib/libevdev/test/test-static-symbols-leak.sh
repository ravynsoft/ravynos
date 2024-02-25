#!/usr/bin/env bash
#
# Hack to check for leaking symbols in the static library.
# See https://bugs.freedesktop.org/show_bug.cgi?id=82785
# Note the spaces in the expressions! After the first grep, each line
# is " T symbol_name"

test -z "$RUNNING_ON_VALGRIND" || exit 77

builddir="$1"

test -f "$builddir/test-static-link" || (echo "Unable to find test file" && exit 1)
nm --extern-only "$builddir/test-static-link" |
	grep -o -e " T .*" | \
	grep -v -e " main\$" \
		-e " atexit" \
		-e " mangle_path" \
		-e " *gcov.*" \
		-e " _.*" \
		-e " libevdev_*" && \
		echo "Leaking symbols found" && exit 1 || exit 0
