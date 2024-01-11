#!/bin/sh
# Wrapper to make GTest tests output TAP syntax, because Automake's test
# drivers do not currently support passing the same command-line argument
# to each test executable. All GTest tests produce TAP output if invoked
# with the --tap option.
#
# Usage: "glib-tap-test.sh test-foo --verbose ..." is equivalent to
# "test-foo --tap --verbose ..."

set -e
t="$1"
shift

case "$t" in
    (*.exe)
        # We're running a Windows executable, possibly on a Unix
        # platform. Avoid having invalid TAP syntax like "ok 3\r\n"
        # where "ok 3\n" was intended.
        echo 1 > "$t".exit-status.tmp
        (
            set +e
            "$t" --tap "$@"
            echo "$?" > "$t".exit-status.tmp
        ) | sed -e 's/\r$//'
        e="$(cat "$t".exit-status.tmp)"
        rm "$t".exit-status.tmp
        exit "$e"
        ;;

    (*)
        exec "$t" --tap "$@"
        ;;
esac
