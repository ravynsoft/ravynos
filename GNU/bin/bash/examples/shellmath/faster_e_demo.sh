#!/usr/bin/env bash

###############################################################################
# This script performs the same task as "slower_e_demo.sh" but with a major
# performance optimization. The speedup is especially noticeable on GNU
# emulation layers for Windows such as Cygwin and minGW, where the overhead
# of subshelling is quite significant.
#
# The speedup uses global storage space to simulate pass-and-return by
# reference so that you can capture the side effects of a function call without
# writing to stdout and wrapping the call in a subshell. How to use:
#
#    Turn on  "__shellmath_isOptimized"  as shown below.
#    Then instead of invoking  "mySum = $(_shellmath_add  $x  $y)",
#    call  "_shellmath_add  $x  $y;  _shellmath_getReturnValue  mySum".
###############################################################################

source shellmath.sh

# Setting the '-t' flag will cause the script to time the algorithm
if [[ "$1" == '-t' ]]; then
    do_timing=${__shellmath_true}
    shift
fi

if [[ $# -ne 1 ]]; then
    echo "USAGE: ${BASH_SOURCE##*/}  [-t]  *N*"
    echo "       Approximates 'e' using the N-th order Maclaurin polynomial"
    echo "       (i.e. the Taylor polynomial centered at 0)."
    echo "       Specify the '-t' flag to time the main algorithm."
    exit 0
elif [[ ! "$1" =~ ^[0-9]+$ ]]; then
    echo "Illegal argument. Whole numbers only, please."
    exit 1
fi

__shellmath_isOptimized=${__shellmath_true}


function run_algorithm()
{
    # Initialize
    n=0;  N=$1;  zero_factorial=1

    # Initialize "e" to its zeroth-order term
    _shellmath_divide  1  $zero_factorial
    _shellmath_getReturnValue term
    e=$term

    # Compute successive terms T(n) := T(n-1)/n and accumulate into e
    for ((n=1; n<=N; n++)); do
        _shellmath_divide  "$term"  "$n"
        _shellmath_getReturnValue term
        _shellmath_add  "$e"  "$term"
        _shellmath_getReturnValue e
    done

    echo "e = $e"
}

if (( do_timing == __shellmath_true )); then
    time run_algorithm "$1"
else
    run_algorithm "$1"
fi

exit 0

