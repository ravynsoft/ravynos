#!/usr/bin/env bash
set -o errexit -o pipefail -o posix

# Copyright (c) 2019-2024 Cosmin Truta.
#
# Use, modification and distribution are subject to the MIT License.
# Please see the accompanying file LICENSE_MIT.txt
#
# SPDX-License-Identifier: MIT

# shellcheck source=ci/lib/ci.lib.sh
source "$(dirname "$0")/lib/ci.lib.sh"
cd "$CI_TOPLEVEL_DIR"

function ci_shellify_c {
    # Convert C preprocessor text, specifically originating
    # from png.h, to shell scripting text.
    # Select only the easy-to-parse definitions of PNG_LIBPNG_*.
    sed -n -e '/^\# *define * PNG_LIBPNG_[^ ]* * ["0-9A-Za-z_]/ p' |
        sed -e 's/^\# *define * PNG\([^ ]*\) * \([^ ]*\)/PNG\1=\2/' \
            -e 's/=PNG\([0-9A-Za-z_]*\)/=\${PNG\1}/' \
            -e 's/^\([^ ]*=[^ ]*\).*$/export \1;/'
}

function ci_shellify_autoconf {
    # Convert autoconf (M4) text, specifically originating
    # from configure.ac, to shell scripting text.
    # Select only the easy-to-parse definitions of PNGLIB_*.
    sed -n -e '/^ *PNGLIB_[^ ]*=[$"0-9A-Za-z_]/ p' |
        sed -e 's/^ *PNG\([0-9A-Za-z_]*\)=\([^# ]*\).*$/PNG\1=\2/' \
            -e 's/^\([^ ]*=[^ ]*\).*$/export \1;/'
}

function ci_shellify_cmake {
    # Convert CMake lists text, specifically originating
    # from CMakeLists.txt, to shell scripting text.
    # Select only the easy-to-parse definitions of PNGLIB_*.
    sed -n -e '/^ *set *(PNGLIB_[^ ]* * [$"0-9A-Za-z_].*)/ p' |
        sed -e 's/^ *set *(PNG\([^ ]*\) * \([^() ]*\)).*$/PNG\1=\2/' \
            -e 's/^\([^ ]*=[^ ]*\).*$/export \1;/'
}

function ci_shellify {
    local arg filename
    for arg in "$@"
    do
        test -f "$arg" || ci_err "no such file: '$arg'"
        filename="$(basename -- "$arg")"
        case "$filename" in
        ( *.[ch] )
            [[ $filename == png.h ]] || {
                ci_err "unable to shellify: '$filename' (expecting: 'png.h')"
            }
            ci_shellify_c <"$arg" ;;
        ( config* | *.ac )
            [[ $filename == configure.ac ]] || {
                ci_err "unable to shellify: '$filename' (expecting: 'configure.ac')"
            }
            ci_shellify_autoconf <"$arg" ;;
        ( *CMake* | *cmake* | *.txt )
            [[ $filename == [Cc][Mm]ake[Ll]ists.txt ]] || {
                ci_err "unable to shellify: '$filename' (expecting: 'CMakeLists.txt')"
            }
            ci_shellify_cmake <"$arg" ;;
        ( * )
            ci_err "unable to shellify: '$arg'" ;;
        esac
    done
}

function usage {
    echo "usage: $CI_SCRIPT_NAME [<options>] <files>..."
    echo "options: -?|-h|--help"
    echo "files: png.h|configure.ac|CMakeLists.txt"
    exit "${@:-0}"
}

function main {
    local opt
    while getopts ":" opt
    do
        # This ain't a while-loop. It only pretends to be.
        [[ $1 == -[?h]* || $1 == --help || $1 == --help=* ]] && usage 0
        ci_err "unknown option: '$1'"
    done
    shift $((OPTIND - 1))
    [[ $# -eq 0 ]] && usage 2
    # And... go!
    ci_shellify "$@"
}

main "$@"
