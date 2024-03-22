#!/usr/bin/env bash
# shellcheck disable=SC2048
# shellcheck disable=SC2086 # we want word splitting
# shellcheck disable=SC2155 # mktemp usually not failing

function x_off {
    if [[ "$-" == *"x"* ]]; then
      state_x=1
      set +x
    else
      state_x=0
    fi
}

# TODO: implement x_on !

function error {
    x_off 2>/dev/null
    RED="\e[0;31m"
    ENDCOLOR="\e[0m"
    # we force the following to be not in a section
    section_end $CURRENT_SECTION

    DATE_S=$(date -u +"%s")
    JOB_START_S=$(date -u +"%s" -d "${CI_JOB_STARTED_AT:?}")
    CURR_TIME=$((DATE_S-JOB_START_S))
    CURR_MINSEC="$(printf "%02d" $((CURR_TIME/60))):$(printf "%02d" $((CURR_TIME%60)))"
    echo -e "\n${RED}[${CURR_MINSEC}] ERROR: $*${ENDCOLOR}\n"
    [ "$state_x" -eq 0 ] || set -x
}

function trap_err {
    error ${CURRENT_SECTION:-'unknown-section'}: ret code: $*
}

function build_section_start {
    local section_params=$1
    shift
    local section_name=$1
    CURRENT_SECTION=$section_name
    shift
    CYAN="\e[0;36m"
    ENDCOLOR="\e[0m"

    DATE_S=$(date -u +"%s")
    JOB_START_S=$(date -u +"%s" -d "${CI_JOB_STARTED_AT:?}")
    CURR_TIME=$((DATE_S-JOB_START_S))
    CURR_MINSEC="$(printf "%02d" $((CURR_TIME/60))):$(printf "%02d" $((CURR_TIME%60)))"
    echo -e "\n\e[0Ksection_start:$(date +%s):$section_name$section_params\r\e[0K${CYAN}[${CURR_MINSEC}] $*${ENDCOLOR}\n"
}

function section_start {
    x_off 2>/dev/null
    build_section_start "[collapsed=true]" $*
    [ "$state_x" -eq 0 ] || set -x
}

function build_section_end {
    echo -e "\e[0Ksection_end:$(date +%s):$1\r\e[0K"
    CURRENT_SECTION=""
}

function section_end {
    x_off >/dev/null
    build_section_end $*
    [ "$state_x" -eq 0 ] || set -x
}

function section_switch {
    x_off 2>/dev/null
    if [ -n "$CURRENT_SECTION" ]
    then
	build_section_end $CURRENT_SECTION
    fi
    build_section_start "[collapsed=true]" $*
    [ "$state_x" -eq 0 ] || set -x
}

function uncollapsed_section_switch {
    x_off 2>/dev/null
    if [ -n "$CURRENT_SECTION" ]
    then
	build_section_end $CURRENT_SECTION
    fi
    build_section_start "" $*
    [ "$state_x" -eq 0 ] || set -x
}

export -f x_off
export -f error
export -f trap_err
export -f build_section_start
export -f section_start
export -f build_section_end
export -f section_end
export -f section_switch
export -f uncollapsed_section_switch

# Freedesktop requirement (needed for Wayland)
[ -n "${XDG_RUNTIME_DIR}" ] || export XDG_RUNTIME_DIR="$(mktemp -p "$PWD" -d xdg-runtime-XXXXXX)"

set -E
trap 'trap_err $?' ERR
