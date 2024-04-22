#!/bin/sh
set -e

case "$1" in
    keymap|compose)
        ;;
    *)
        echo "usage: $0 keymap|compose" 1>&2
        exit 1
        ;;
esac

export CC=afl-clang-fast
export AFL_HARDEN=1
test -d fuzz/build || meson setup -Db_lto=true fuzz/build
meson compile -C fuzz/build
afl-fuzz -i fuzz/$1/testcases -x fuzz/$1/dict -o fuzz/$1/findings -t 200 -m 10 -- ./fuzz/build/fuzz-$1 @@
