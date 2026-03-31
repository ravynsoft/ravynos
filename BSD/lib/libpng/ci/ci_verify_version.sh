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

function ci_init_shellify {
    [[ -f $CI_SCRIPT_DIR/ci_shellify.sh ]] || {
        ci_err_internal "missing script: '$CI_SCRIPT_DIR/ci_shellify.sh'"
    }
}

function ci_run_shellify {
    ci_info "shellifying:" "$@"
    local my_result
    "$BASH" "$CI_SCRIPT_DIR/ci_shellify.sh" "$@"
    echo "$my_result" | "$BASH" --posix || ci_err "bad shellify output"
    echo "$my_result"
}

function ci_verify_version {
    ci_info "## START OF VERIFICATION ##"
    local my_env_libpng_ver my_env_autoconf_ver my_env_cmake_ver my_expect
    ci_init_shellify
    my_env_libpng_ver="$(ci_run_shellify png.h)"
    echo "$my_env_libpng_ver"
    my_env_autoconf_ver="$(ci_run_shellify configure.ac)"
    echo "$my_env_autoconf_ver"
    my_env_cmake_ver="$(ci_run_shellify CMakeLists.txt)"
    echo "$my_env_cmake_ver"
    ci_info "## VERIFYING: png.h version definitions ##"
    eval "$my_env_libpng_ver"
    local my_expect="${PNG_LIBPNG_VER_MAJOR}.${PNG_LIBPNG_VER_MINOR}.${PNG_LIBPNG_VER_RELEASE}"
    if [[ "$PNG_LIBPNG_VER_STRING" == "$my_expect"* ]]
    then
        ci_info "matched: \$PNG_LIBPNG_VER_STRING == $my_expect*"
    else
        ci_err "mismatched: \$PNG_LIBPNG_VER_STRING != $my_expect*"
    fi
    my_expect=$((PNG_LIBPNG_VER_MAJOR*10000 + PNG_LIBPNG_VER_MINOR*100 + PNG_LIBPNG_VER_RELEASE))
    if [[ "$PNG_LIBPNG_VER" == "$my_expect" ]]
    then
        ci_info "matched: \$PNG_LIBPNG_VER == $my_expect"
    else
        ci_err "mismatched: \$PNG_LIBPNG_VER != $my_expect"
    fi
    my_expect=$((PNG_LIBPNG_VER_MAJOR*10 + PNG_LIBPNG_VER_MINOR))
    if [[ "$PNG_LIBPNG_VER_SHAREDLIB" == "$my_expect" ]]
    then
        ci_info "matched: \$PNG_LIBPNG_VER_SHAREDLIB == $my_expect"
    else
        ci_err "mismatched: \$PNG_LIBPNG_VER_SHAREDLIB != $my_expect"
    fi
    if [[ "$PNG_LIBPNG_VER_SONUM" == "$my_expect" ]]
    then
        ci_info "matched: \$PNG_LIBPNG_VER_SONUM == $my_expect"
    else
        ci_err "mismatched: \$PNG_LIBPNG_VER_SONUM != $my_expect"
    fi
    if [[ "$PNG_LIBPNG_VER_DLLNUM" == "$my_expect" ]]
    then
        ci_info "matched: \$PNG_LIBPNG_VER_DLLNUM == $my_expect"
    else
        ci_err "mismatched: \$PNG_LIBPNG_VER_DLLNUM != $my_expect"
    fi
    if [[ "$PNG_LIBPNG_VER_BUILD" == [01] ]]
    then
        ci_info "matched: \$PNG_LIBPNG_VER_BUILD == [01]"
    else
        ci_err "mismatched: \$PNG_LIBPNG_VER_BUILD != [01]"
    fi
    ci_info "## VERIFYING: png.h build definitions ##"
    my_expect="${PNG_LIBPNG_VER_MAJOR}.${PNG_LIBPNG_VER_MINOR}.${PNG_LIBPNG_VER_RELEASE}"
    if [[ "$PNG_LIBPNG_VER_STRING" == "$my_expect" ]]
    then
        if [[ $PNG_LIBPNG_VER_BUILD -eq 0 ]]
        then
            ci_info "matched: \$PNG_LIBPNG_VER_BUILD -eq 0"
        else
            ci_err "mismatched: \$PNG_LIBPNG_VER_BUILD -ne 0"
        fi
        if [[ $PNG_LIBPNG_BUILD_BASE_TYPE -eq $PNG_LIBPNG_BUILD_STABLE ]]
        then
            ci_info "matched: \$PNG_LIBPNG_BUILD_BASE_TYPE -eq \$PNG_LIBPNG_BUILD_BETA"
        else
            ci_err "mismatched: \$PNG_LIBPNG_BUILD_BASE_TYPE -ne \$PNG_LIBPNG_BUILD_BETA"
        fi
    elif [[ "$PNG_LIBPNG_VER_STRING" == "$my_expect".git ]]
    then
        if [[ $PNG_LIBPNG_VER_BUILD -ne 0 ]]
        then
            ci_info "matched: \$PNG_LIBPNG_VER_BUILD -ne 0"
        else
            ci_err "mismatched: \$PNG_LIBPNG_VER_BUILD -eq 0"
        fi
        if [[ $PNG_LIBPNG_BUILD_BASE_TYPE -eq $PNG_LIBPNG_BUILD_BETA ]]
        then
            ci_info "matched: \$PNG_LIBPNG_BUILD_BASE_TYPE -eq \$PNG_LIBPNG_BUILD_BETA"
        else
            ci_err "mismatched: \$PNG_LIBPNG_BUILD_BASE_TYPE -ne \$PNG_LIBPNG_BUILD_BETA"
        fi
    else
        ci_err "unexpected: \$PNG_LIBPNG_VER_STRING == '$PNG_LIBPNG_VER_STRING'"
    fi
    ci_info "## VERIFYING: png.h type definitions ##"
    my_expect="$(echo "png_libpng_version_${PNG_LIBPNG_VER_STRING}" | tr . _)"
    ci_spawn grep -w -e "$my_expect" png.h
    ci_info "## VERIFYING: configure.ac version definitions ##"
    eval "$my_env_autoconf_ver"
    if [[ "$PNGLIB_VERSION" == "$PNG_LIBPNG_VER_STRING" ]]
    then
        ci_info "matched: \$PNGLIB_VERSION == \$PNG_LIBPNG_VER_STRING"
    else
        ci_err "mismatched: \$PNGLIB_VERSION != \$PNG_LIBPNG_VER_STRING"
    fi
    ci_info "## VERIFYING: CMakeLists.txt version definitions ##"
    eval "$my_env_cmake_ver"
    if [[ "$PNGLIB_VERSION" == "$PNG_LIBPNG_VER_STRING" && "$PNGLIB_SUBREVISION" == 0 ]]
    then
        ci_info "matched: \$PNGLIB_VERSION == \$PNG_LIBPNG_VER_STRING"
        ci_info "matched: \$PNGLIB_SUBREVISION == 0"
    elif [[ "$PNGLIB_VERSION.$PNGLIB_SUBREVISION" == "$PNG_LIBPNG_VER_STRING" ]]
    then
        ci_info "matched: \$PNGLIB_VERSION.\$PNGLIB_SUBREVISION == \$PNG_LIBPNG_VER_STRING"
    else
        ci_err "mismatched: \$PNGLIB_VERSION != \$PNG_LIBPNG_VER_STRING"
    fi
    ci_info "## END OF VERIFICATION ##"
    ci_info "success!"
}

function usage {
    echo "usage: $CI_SCRIPT_NAME [<options>]"
    echo "options: -?|-h|--help"
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
    [[ $# -eq 0 ]] || {
        echo >&2 "error: unexpected argument: '$1'"
        usage 2
    }
    # And... go!
    ci_verify_version
}

main "$@"
