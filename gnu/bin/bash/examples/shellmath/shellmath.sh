#!/bin/env bash
################################################################################
# shellmath.sh
# Shell functions for floating-point arithmetic using only builtins
#
# Copyright (c) 2020 by Michael Wood. All rights reserved.
#
# Usage:
#
#    source  _thisPath_/_thisFileName_
#
#    # Conventional method: call the APIs by subshelling
#    mySum=$( _shellmath_add 202.895 6.00311 )
#    echo $mySum
#
#    # Optimized method: use hidden globals to simulate more flexible pass-and-return
#    _shellmath_isOptimized=1
#    _shellmath_add 44.2 -87
#    _shellmath_getReturnValue mySum
#    echo $mySum
# 
################################################################################


################################################################################
# Program constants
################################################################################
declare -A -r __shellmath_numericTypes=(
    [INTEGER]=0
    [DECIMAL]=1
)

declare -A -r __shellmath_returnCodes=(
    [SUCCESS]="0:Success"
    [FAIL]="1:General failure"
    [ILLEGAL_NUMBER]="2:Invalid argument; decimal number required: '%s'"
    [DIVIDE_BY_ZERO]="3:Divide by zero error"
)

declare -r -i __shellmath_true=1
declare -r -i __shellmath_false=0

declare __shellmath_SUCCESS  __shellmath_FAIL  __shellmath_ILLEGAL_NUMBER

################################################################################
# Program state
################################################################################
declare __shellmath_isOptimized=${__shellmath_false}
declare __shellmath_didPrecalc=${__shellmath_false}


################################################################################
# Error-handling utilities
################################################################################
function _shellmath_getReturnCode()
{
    local errorName=$1
    return "${__shellmath_returnCodes[$errorName]%%:*}"
}

function _shellmath_warn()
{
    # Generate an error message and return control to the caller
    _shellmath_handleError -r "$@"
    return $?
}

function _shellmath_exit()
{
    # Generate an error message and EXIT THE SCRIPT / interpreter
    _shellmath_handleError "$@"
}

function _shellmath_handleError()
{
    # Hidden option "-r" causes return instead of exit
    local returnDontExit=$__shellmath_false
    if [[ "$1" == "-r" ]]; then
        returnDontExit=${__shellmath_true}
        shift
    fi

    # Format of $1:  returnCode:msgTemplate
    [[ "$1" =~ ^([0-9]+):(.*) ]]
    returnCode=${BASH_REMATCH[1]}
    msgTemplate=${BASH_REMATCH[2]}
    shift
    
    # Display error msg, making parameter substitutions as needed
    msgParameters="$*"
    printf  "$msgTemplate" "${msgParameters[@]}"

    if ((returnDontExit)); then
        return "$returnCode"
    else
        exit "$returnCode"
    fi
}


################################################################################
# precalc()
#
# Pre-calculates certain global data and by setting the global variable
# "__shellmath_didPrecalc" records that this routine has been called. As an
# optimization, the caller should check that global to prevent needless
# invocations.
################################################################################
function _shellmath_precalc()
{
    # Set a few global constants
    _shellmath_getReturnCode SUCCESS; __shellmath_SUCCESS=$?
    _shellmath_getReturnCode FAIL; __shellmath_FAIL=$?
    _shellmath_getReturnCode ILLEGAL_NUMBER; __shellmath_ILLEGAL_NUMBER=$?

    # Determine the decimal precision to which we can accurately calculate.
    # To do this we probe for the threshold at which integers overflow and
    # take the integer floor of that number's base-10 logarithm.
    # We check the 64-bit, 32-bit and 16-bit thresholds only.
    if ((2**63 < 2**63-1)); then
        __shellmath_precision=18
        __shellmath_maxValue=$((2**63-1))
    elif ((2**31 < 2**31-1)); then
        __shellmath_precision=9
        __shellmath_maxValue=$((2**31-1))
    else     ## ((2**15 < 2**15-1))
        __shellmath_precision=4
        __shellmath_maxValue=$((2**15-1))
    fi

    __shellmath_didPrecalc=$__shellmath_true
}


################################################################################
# Simulate pass-and-return by reference using a secret global storage array
################################################################################

declare -a __shellmath_storage

function _shellmath_setReturnValues()
{
    local -i _i

    for ((_i=1; _i<=$#; _i++)); do
        __shellmath_storage[_i]="${!_i}"
    done

    __shellmath_storage[0]=$#
}

function _shellmath_getReturnValues()
{
    local -i _i
    local evalString

    for ((_i=1; _i<=$#; _i++)); do
        evalString+=${!_i}="${__shellmath_storage[_i]}"" "
    done

    eval "$evalString"
}

function _shellmath_setReturnValue() { __shellmath_storage=(1 "$1"); }
function _shellmath_getReturnValue() { eval "$1"=\"${__shellmath_storage[1]}\"; }
function _shellmath_getReturnValueCount() { eval "$1"=\"${__shellmath_storage[0]}\"; }

################################################################################
# validateAndParse(numericString)
# Return Code:      SUCCESS or ILLEGAL_NUMBER
# Return Signature: integerPart fractionalPart isNegative numericType isScientific
#
# Validate and parse arguments to the main arithmetic routines
################################################################################

function _shellmath_validateAndParse()
{
    local n="$1"
    local isNegative=${__shellmath_false}
    local isScientific=${__shellmath_false}
    local numericType returnCode

    ((returnCode = __shellmath_SUCCESS))
    
    # Accept decimals: leading digits (optional), decimal point, trailing digits
    if [[ "$n" =~ ^[-]?([0-9]*)\.([0-9]+)$ ]]; then
        local integerPart=${BASH_REMATCH[1]:-0}
        local fractionalPart=${BASH_REMATCH[2]}

        # Strip superfluous trailing zeros
        if [[ "$fractionalPart" =~ ^(.*[^0])0*$ ]]; then
            fractionalPart=${BASH_REMATCH[1]}
        fi

        numericType=${__shellmath_numericTypes[DECIMAL]}

        # Factor out the negative sign if it is present
        if [[ "$n" =~ ^- ]]; then
            isNegative=${__shellmath_true}
            n=${n:1}
        fi

        _shellmath_setReturnValues "$integerPart" "$fractionalPart" \
                    $isNegative "$numericType" $isScientific
        return "$returnCode"

    # Accept integers
    elif [[ "$n" =~ ^[-]?[0-9]+$ ]]; then
        numericType=${__shellmath_numericTypes[INTEGER]}

        # Factor out the negative sign if it is present
        if [[ "$n" =~ ^- ]]; then
            isNegative=${__shellmath_true}
            n=${n:1}
        fi

        _shellmath_setReturnValues "$n" 0 $isNegative "$numericType" $isScientific
        return "$returnCode"

    # Accept scientific notation: 1e5, 2.44E+10, etc.
    elif [[ "$n" =~ (.*)[Ee](.*) ]]; then
        local significand=${BASH_REMATCH[1]}
        local exponent=${BASH_REMATCH[2]}

        # Validate the significand: optional sign, integer part,
        # optional decimal point and fractional part
        if [[ "$significand" =~ ^[-]?([0-9]+)(\.([0-9]+))?$ ]]; then

            isScientific=${__shellmath_true}

            # Separate the integer and fractional parts
            local sigInteger=${BASH_REMATCH[1]}
            local sigIntLength=${#sigInteger}
            local sigFraction=${BASH_REMATCH[3]}

            # Strip superfluous trailing zeros
            if [[ "$sigFraction" =~ ^(.*[^0])0*$ ]]; then
                sigFraction=${BASH_REMATCH[1]}
            fi

            local sigFracLength=${#sigFraction}

            if [[ "$n" =~ ^- ]]; then
                isNegative=${__shellmath_true}
                n=${n:1}
            fi

            # Rewrite the scientifically-notated number in ordinary decimal notation.
            # IOW, realign the integer and fractional parts. Separate with a space
            # so they can be returned as two separate values
            if ((exponent > 0)); then
                local zeroCount integer fraction
                ((zeroCount = exponent - sigFracLength))
                if ((zeroCount > 0)); then
                    printf -v zeros "%0*d" "$zeroCount" 0
                    n=${sigInteger}${sigFraction}${zeros}" 0"
                    numericType=${__shellmath_numericTypes[INTEGER]}
                elif ((zeroCount < 0)); then
                    n=${sigInteger}${sigFraction:0:exponent}" "${sigFraction:exponent}
                    numericType=${__shellmath_numericTypes[DECIMAL]}
                else
                    n=${sigInteger}${sigFraction}" 0"
                    numericType=${__shellmath_numericTypes[INTEGER]}
                fi
                integer=${n% *}; fraction=${n#* }
                _shellmath_setReturnValues "$integer" "$fraction" $isNegative "$numericType" $isScientific
                return "$returnCode"

            elif ((exponent < 0)); then
                local zeroCount integer fraction
                ((zeroCount = -exponent - sigIntLength))
                if ((zeroCount > 0)); then
                    printf -v zeros "%0*d" "$zeroCount" 0
                    n="0 "${zeros}${sigInteger}${sigFraction}
                    numericType=${__shellmath_numericTypes[DECIMAL]}
                elif ((zeroCount < 0)); then
                    n=${sigInteger:0:-zeroCount}" "${sigInteger:(-zeroCount)}${sigFraction}
                    numericType=${__shellmath_numericTypes[DECIMAL]}
                else
                    n="0 "${sigInteger}${sigFraction}
                    numericType=${__shellmath_numericTypes[DECIMAL]}
                fi
                integer=${n% *}; fraction=${n#* }
                _shellmath_setReturnValues "$integer" "$fraction" $isNegative "$numericType" $isScientific
                return "$returnCode"

            else
                # exponent == 0 means the number is already aligned as desired
                numericType=${__shellmath_numericTypes[DECIMAL]}
                _shellmath_setReturnValues "$sigInteger" "$sigFraction" $isNegative "$numericType" $isScientific
                return "$returnCode"
            fi

        # Reject strings like xxx[Ee]yyy where xxx, yyy are not valid numbers
        else
            ((returnCode = __shellmath_ILLEGAL_NUMBER))
            _shellmath_setReturnValues ""
            return "$returnCode"
        fi

    # Reject everything else
    else
        ((returnCode = __shellmath_ILLEGAL_NUMBER))
        _shellmath_setReturnValues ""
        return "$returnCode"
    fi
}


################################################################################
# numToScientific (integerPart, fractionalPart)
#
# Format conversion utility function
################################################################################
function _shellmath_numToScientific()
{
    local integerPart=$1 fractionalPart=$2
    local exponent head tail scientific

    if ((integerPart > 0)); then
        ((exponent = ${#integerPart}-1))
        head=${integerPart:0:1}
        tail=${integerPart:1}${fractionalPart}
    elif ((integerPart < 0)); then
        ((exponent = ${#integerPart}-2))   # skip "-" and first digit
        head=${integerPart:0:2}
        tail=${integerPart:2}${fractionalPart}
    else
        [[ "$fractionalPart" =~ ^[-]?(0*)([^0])(.*)$ ]]
        exponent=$((-(${#BASH_REMATCH[1]} + 1)))
        head=${BASH_REMATCH[2]}
        tail=${BASH_REMATCH[3]}
    fi

    # Remove trailing zeros
    [[ $tail =~ ^.*[^0] ]]; tail=${BASH_REMATCH[0]:-0}

    printf -v scientific "%d.%de%d" "$head" "$tail" "$exponent"

    _shellmath_setReturnValue "$scientific"
}


################################################################################
# _shellmath_add (addend_1, addend_2)
################################################################################
function _shellmath_add()
{
    local n1="$1"
    local n2="$2"

    if ((! __shellmath_didPrecalc)); then
        _shellmath_precalc; __shellmath_didPrecalc=$__shellmath_true
    fi

    local isVerbose=$(( __shellmath_isOptimized == __shellmath_false ))

    # Is the caller itself an arithmetic function?
    local isSubcall=${__shellmath_false}
    local isMultiplication=${__shellmath_false}
    if [[ "${FUNCNAME[1]}" =~ shellmath_(add|subtract|multiply|divide)$ ]]; then
        isSubcall=${__shellmath_true}
        if [[ "${BASH_REMATCH[1]}" == multiply ]]; then
            isMultiplication=${__shellmath_true}
        fi
    fi

    # Handle corner cases where argument count is not 2
    local argCount=$#
    if ((argCount == 0)); then
        echo "Usage: ${FUNCNAME[0]}  addend_1  addend_2"
        return "$__shellmath_SUCCESS"
    elif ((argCount == 1)); then
        # Note the result as-is, print if running "normally", and return
        _shellmath_setReturnValue "$n1"
        (( isVerbose && ! isSubcall )) && echo "$n1"
        return "$__shellmath_SUCCESS"
    elif ((argCount > 2 && !isSubcall)); then
        local recursiveReturn

        # Use a binary recursion tree to add everything up
        # 1) left branch
        _shellmath_add "${@:1:$((argCount/2))}"; recursiveReturn=$?
        _shellmath_getReturnValue n1
        if (( recursiveReturn != __shellmath_SUCCESS )); then
            _shellmath_setReturnValue "$n1"
            return "$recursiveReturn"
        fi
        # 2) right branch
        _shellmath_add "${@:$((argCount/2+1))}"; recursiveReturn=$?
        _shellmath_getReturnValue n2
        if (( recursiveReturn != __shellmath_SUCCESS )); then
            _shellmath_setReturnValue "$n2"
            return "$recursiveReturn"
        fi
        # 3) head node
        local sum
        _shellmath_add "$n1" "$n2"; recursiveReturn=$?
        _shellmath_getReturnValue sum
        _shellmath_setReturnValue "$sum"
        if (( isVerbose && ! isSubcall )); then
            echo "$sum"
        fi
        return "$recursiveReturn"
    fi

    local integerPart1  fractionalPart1  integerPart2  fractionalPart2
    local isNegative1 type1 isScientific1 isNegative2 type2 isScientific2
    local flags

    if ((isMultiplication)); then
        integerPart1="$1"
        fractionalPart1="$2"
        integerPart2="$3"
        fractionalPart2="$4"

        type1=${__shellmath_numericTypes[DECIMAL]}
        type2=${__shellmath_numericTypes[DECIMAL]}
        isNegative1=$__shellmath_false
        isNegative2=$__shellmath_false
        isScientific1=$__shellmath_false
        isScientific2=$__shellmath_false
    else
        # Check and parse the arguments
        _shellmath_validateAndParse "$n1";  flags=$?
        _shellmath_getReturnValues  integerPart1  fractionalPart1  isNegative1  type1  isScientific1
        if ((flags == __shellmath_ILLEGAL_NUMBER)); then
            _shellmath_warn  "${__shellmath_returnCodes[ILLEGAL_NUMBER]}"  "$n1"
            return $?
        fi
        _shellmath_validateAndParse "$n2";  flags=$?
        _shellmath_getReturnValues  integerPart2  fractionalPart2  isNegative2  type2  isScientific2
        if ((flags == __shellmath_ILLEGAL_NUMBER)); then
            _shellmath_warn  "${__shellmath_returnCodes[ILLEGAL_NUMBER]}"  "$n2"
            return $?
        fi
    fi

    # Quick add & return for integer adds
    if ((type1==type2 && type1==__shellmath_numericTypes[INTEGER])); then
        ((isNegative1)) && ((integerPart1*=-1))
        ((isNegative2)) && ((integerPart2*=-1))
        local sum=$((integerPart1 + integerPart2))
        if (( (!isSubcall) && (isScientific1 || isScientific2) )); then
            _shellmath_numToScientific $sum "" 
            _shellmath_getReturnValue sum
        fi
        _shellmath_setReturnValue $sum
        if (( isVerbose && ! isSubcall )); then
            echo "$sum"
        fi
        return "$__shellmath_SUCCESS"
    fi

    # Right-pad both fractional parts with zeros to the same length
    local fractionalLen1=${#fractionalPart1}
    local fractionalLen2=${#fractionalPart2}
    if ((fractionalLen1 > fractionalLen2)); then
        # Use printf to zero-pad. This avoids mathematical side effects.
        printf -v fractionalPart2 %-*s "$fractionalLen1" "$fractionalPart2"
        fractionalPart2=${fractionalPart2// /0}
    elif ((fractionalLen2 > fractionalLen1)); then
        printf -v fractionalPart1 %-*s "$fractionalLen2" "$fractionalPart1"
        fractionalPart1=${fractionalPart1// /0}
    fi
    local unsignedFracLength=${#fractionalPart1}

    # Implement a sign convention that will enable us to detect carries by
    # comparing string lengths of addends and sums: propagate the sign across
    # both numeric parts (whether unsigned or zero).
    if ((isNegative1)); then
        fractionalPart1="-"$fractionalPart1
        integerPart1="-"$integerPart1
    fi
    if ((isNegative2)); then
        fractionalPart2="-"$fractionalPart2
        integerPart2="-"$integerPart2
    fi

    local integerSum=0
    local fractionalSum=0

    ((integerSum = integerPart1+integerPart2))

    # Summing the fractional parts is tricky: We need to override the shell's
    # default interpretation of leading zeros, but the operator for doing this
    # (the "10#" operator) cannot work directly with negative numbers. So we
    # break it all down.
    if ((isNegative1)); then
        ((fractionalSum += (-1) * 10#${fractionalPart1:1}))
    else
        ((fractionalSum += 10#$fractionalPart1))
    fi
    if ((isNegative2)); then
        ((fractionalSum += (-1) * 10#${fractionalPart2:1}))
    else
        ((fractionalSum += 10#$fractionalPart2))
    fi

    unsignedFracSumLength=${#fractionalSum}
    if [[ "$fractionalSum" =~ ^[-] ]]; then
        ((unsignedFracSumLength--))
    fi

    # Restore any leading zeroes that were lost when adding
    if ((unsignedFracSumLength < unsignedFracLength)); then
        local lengthDiff=$((unsignedFracLength - unsignedFracSumLength))
        local zeroPrefix
        printf -v zeroPrefix "%0*d" "$lengthDiff" 0
        if ((fractionalSum < 0)); then
            fractionalSum="-"${zeroPrefix}${fractionalSum:1}
        else
            fractionalSum=${zeroPrefix}${fractionalSum}
        fi
    fi

    # Carry a digit from fraction to integer if required
    if ((10#$fractionalSum!=0 && unsignedFracSumLength > unsignedFracLength)); then
        local carryAmount
        ((carryAmount = isNegative1?-1:1))
        ((integerSum += carryAmount))
        # Remove the leading 1-digit whether the fraction is + or -
        fractionalSum=${fractionalSum/1/}
    fi

    # Transform the partial sums from additive to concatenative. Example: the
    # pair (-2,3) is not -2.3 but rather (-2)+(0.3), i.e. -1.7 so we want to
    # transform (-2,3) to (-1,7). This transformation is meaningful when
    # the two parts have opposite signs, so that's what we look for.
    if ((integerSum < 0 && 10#$fractionalSum > 0)); then
        ((integerSum += 1))
        ((fractionalSum = 10#$fractionalSum - 10**unsignedFracSumLength))
    elif ((integerSum > 0 && 10#$fractionalSum < 0)); then
        ((integerSum -= 1))
        ((fractionalSum = 10**unsignedFracSumLength + 10#$fractionalSum))
    fi
    # This last case needs to function either as an "else" for the above,
    # or as a coda to the "if" clause when integerSum is -1 initially.
    if ((integerSum == 0 && 10#$fractionalSum < 0)); then
        integerSum="-"$integerSum
        ((fractionalSum *= -1))
    fi

    # Touch up the numbers for display
    local sum
    ((10#$fractionalSum < 0)) && fractionalSum=${fractionalSum:1}
    if (( (!isSubcall) && (isScientific1 || isScientific2) )); then
        _shellmath_numToScientific "$integerSum" "$fractionalSum"
        _shellmath_getReturnValue sum
    elif ((10#$fractionalSum)); then
        printf -v sum "%s.%s" "$integerSum" "$fractionalSum"
    else
        sum=$integerSum
    fi

    # Note the result, print if running "normally", and return
    _shellmath_setReturnValue $sum
    if (( isVerbose && ! isSubcall )); then
        echo "$sum"
    fi

    return "$__shellmath_SUCCESS"
}


################################################################################
# subtract (subtrahend, minuend)
################################################################################
function _shellmath_subtract()
{
    local n1="$1"
    local n2="$2"
    local isVerbose=$(( __shellmath_isOptimized == __shellmath_false ))

    if ((! __shellmath_didPrecalc)); then
        _shellmath_precalc; __shellmath_didPrecalc=$__shellmath_true
    fi

    if (( $# == 0 || $# > 2 )); then
        echo "Usage: ${FUNCNAME[0]}  subtrahend  minuend"
        return "$__shellmath_SUCCESS"
    elif (( $# == 1 )); then
        # Note the value as-is and return
        _shellmath_setReturnValue "$n1"
        ((isVerbose)) && echo "$n1"
        return "$__shellmath_SUCCESS"
    fi

    # Symbolically negate the second argument
    if [[ "$n2" =~ ^- ]]; then
        n2=${n2:1}
    else
        n2="-"$n2
    fi

    # Calculate, note the result, print if running "normally", and return
    local difference
    _shellmath_add "$n1" "$n2"
    _shellmath_getReturnValue difference
    if ((isVerbose)); then
        echo "$difference"
    fi

    return $?
}


################################################################################
# reduceOuterPairs (two integer parts [, two fractional parts])
#
# Examines the magnitudes of two numbers in advance of a multiplication
# and either chops off their lowest-order digits or pushes them to the
# corresponding lower-order parts in order to prevent overflow in the product.
# The choice depends on whether 2 or 4 arguments are supplied.
################################################################################
function _shellmath_reduceOuterPairs()
{
    local value1="$1" value2="$2" subvalue1="$3" subvalue2="$4"

    local digitExcess value1Len=${#value1} value2Len=${#value2}
    ((digitExcess = value1Len + value2Len - __shellmath_precision))

    # Be very precise about detecting overflow. The "digit excess" underestimates
    # this: floor(log_10(longLongMax)). We don't want to needlessly lose precision
    # when a product barely squeezes under the exact threshold.
    if ((digitExcess>1 || (digitExcess==1 && value1 > __shellmath_maxValue/value2) )); then

        # Identify the digit-tails to be pruned off and either discarded or
        # pushed past the decimal point. In pruning the two values we want to
        # retain as much "significance" as possible, so we try to equalize the
        # lengths of the remaining digit sequences.
        local tail1 tail2
        local lengthDiff leftOver

        # Which digit string is longer, and by how much?
        ((lengthDiff = value1Len - value2Len))
        if ((lengthDiff > 0)); then
            if ((digitExcess <= lengthDiff)); then
                # Chop digits from the longer string only
                tail1=${value1:(-digitExcess)}
                tail2=""     # do not chop anything
            else
                # Chop more digits from the longer string so the two strings
                # end up as nearly-equal in length as possible
                ((leftOver = digitExcess - lengthDiff))
                tail1=${value1:(-(lengthDiff + leftOver/2))}
                tail2=${value2:(-((leftOver+1)/2))}
            fi
        else
            ((lengthDiff *= -1))
            # Mirror the above code block but swap 1 and 2
            if ((digitExcess <= lengthDiff)); then
                tail1=""
                tail2=${value2:(-digitExcess)}
            else
                ((leftOver = digitExcess - lengthDiff))
                tail1=${value1:(-((leftOver+1)/2))}
                tail2=${value2:(-(lengthDiff + leftOver/2))}
            fi
        fi

        # Discard the least-significant digits or move them past the decimal point
        value1=${value1%${tail1}}
        [[ -n "$subvalue1" ]] && subvalue1=${tail1}${subvalue1%0}  # remove placeholder zero
        value2=${value2%${tail2}}
        [[ -n "$subvalue2" ]] && subvalue2=${tail2}${subvalue2%0}
    else
        # Signal the caller that no rescaling was actually done
        ((digitExcess = 0))
    fi

    _shellmath_setReturnValues "$value1" "$value2" \
                         "$subvalue1" "$subvalue2" "$digitExcess"
}

################################################################################
# rescaleValue(value, rescaleFactor)
#
# Upscales a decimal value by "factor" orders of magnitude:  3.14159 --> 3141.59
################################################################################
function _shellmath_rescaleValue()
{
    local value="$1" rescalingFactor="$2"
    local head tail zeroCount zeroTail

    [[ "$value" =~ ^(.*)\.(.*)$ ]]
    head=${BASH_REMATCH[1]}
    tail=${BASH_REMATCH[2]}
    ((zeroCount = rescalingFactor - ${#tail}))
    if ((zeroCount > 0)); then
        printf -v zeroTail "%0*d" "$zeroCount" 0
        value=${head}${tail}${zeroTail}
    elif ((zeroCount < 0)); then
        value=${head}${tail:0:rescalingFactor}"."${tail:rescalingFactor}
    else
        value=${head}${tail}
    fi

    _shellmath_setReturnValue "$value"
}

################################################################################
# reduceCrossPairs (two integer parts, two fractional parts)
#
# Examines the precision of the inner products (of "multiplication by parts")
# and if necessary truncates the fractional part(s) to prevent overflow
################################################################################
function _shellmath_reduceCrossPairs()
{
    local value1="$1" value2="$2" subvalue1="$3" subvalue2="$4"

    local digitExcess value1Len=${#value1} value2Len=${#value2}
    local subvalue1Len=${#subvalue1} subvalue2Len=${#subvalue2}

    # Check BOTH cross-products
    ((digitExcess = value1Len + subvalue2Len - __shellmath_precision))
    if ((digitExcess > 1 || (digitExcess==1 && value1 > __shellmath_maxValue/subvalue2) )); then
        subvalue2=${subvalue2:0:(-digitExcess)}
    fi
    ((digitExcess = value2Len + subvalue1Len - __shellmath_precision))
    if ((digitExcess > 1 || (digitExcess==1 && value2 > __shellmath_maxValue/subvalue1) )); then
        subvalue1=${subvalue1:0:(-digitExcess)}
    fi

    _shellmath_setReturnValues "$subvalue1" "$subvalue2"
}


function _shellmath_round()
{
    local number="$1" digitCount="$2"
    local nextDigit=${number:digitCount:1}

    number=${number:0:digitCount}
    if ((nextDigit >= 5)); then
        printf -v number "%0*d" "$digitCount" $((10#$number + 1))
    fi

    _shellmath_setReturnValue "$number"
}

################################################################################
# multiply (multiplicand, multiplier)
################################################################################
function _shellmath_multiply()
{
    local n1="$1"
    local n2="$2"

    if ((! __shellmath_didPrecalc)); then
        _shellmath_precalc; __shellmath_didPrecalc=$__shellmath_true
    fi

    local isVerbose=$(( __shellmath_isOptimized == __shellmath_false ))

    # Is the caller itself an arithmetic function?
    local isSubcall=${__shellmath_false}
    if [[ "${FUNCNAME[1]}" =~ shellmath_(add|subtract|multiply|divide)$ ]]; then
        isSubcall=${__shellmath_true}
    fi

    # Handle corner cases where argument count is not 2
    local argCount=$#
    if ((argCount == 0)); then
        echo "Usage: ${FUNCNAME[0]}  factor_1  factor_2"
        return "$__shellmath_SUCCESS"
    elif ((argCount == 1)); then
        # Note the value as-is and return
        _shellmath_setReturnValue "$n1"
        (( isVerbose && ! isSubcall )) && echo "$n1"
        return "$__shellmath_SUCCESS"
    elif ((argCount > 2)); then
        local recursiveReturn

        # Use a binary recursion tree to multiply everything out
        # 1) left branch
        _shellmath_multiply "${@:1:$((argCount/2))}"; recursiveReturn=$?
        _shellmath_getReturnValue n1
        if (( recursiveReturn != __shellmath_SUCCESS )); then
            _shellmath_setReturnValue "$n1"
            return "$recursiveReturn"
        fi
        # 2) right branch
        _shellmath_multiply "${@:$((argCount/2+1))}"; recursiveReturn=$?
        _shellmath_getReturnValue n2
        if (( recursiveReturn != __shellmath_SUCCESS )); then
            _shellmath_setReturnValue "$n2"
            return "$recursiveReturn"
        fi
        # 3) head node
        local product
        _shellmath_multiply "$n1" "$n2"; recursiveReturn=$?
        _shellmath_getReturnValue product
        _shellmath_setReturnValue "$product"
        if (( isVerbose && ! isSubcall )); then
            echo "$product"
        fi
        return "$recursiveReturn"
    fi

    local integerPart1  fractionalPart1  integerPart2  fractionalPart2
    local isNegative1 type1 isScientific1 isNegative2 type2 isScientific2
    local flags

    # Check and parse the arguments
    _shellmath_validateAndParse "$n1";  flags=$?
    _shellmath_getReturnValues  integerPart1  fractionalPart1  isNegative1  type1  isScientific1
    if ((flags == __shellmath_ILLEGAL_NUMBER)); then
        _shellmath_warn  "${__shellmath_returnCodes[ILLEGAL_NUMBER]}"  "$n1"
        return $?
    fi
    _shellmath_validateAndParse "$n2";  flags=$?
    _shellmath_getReturnValues  integerPart2  fractionalPart2  isNegative2  type2  isScientific2
    if ((flags == __shellmath_ILLEGAL_NUMBER)); then
        _shellmath_warn  "${__shellmath_returnCodes[ILLEGAL_NUMBER]}"  "$n2"
        return $?
    fi

    # Overflow / underflow detection and accommodation
    local rescalingFactor=0
    if ((${#integerPart1} + ${#integerPart2} + ${#fractionalPart1} + ${#fractionalPart2} >= ${__shellmath_precision})); then
        _shellmath_reduceOuterPairs "$integerPart1" "$integerPart2" "$fractionalPart1" "$fractionalPart2"
        _shellmath_getReturnValues integerPart1 integerPart2 fractionalPart1 fractionalPart2 rescalingFactor
        if ((10#$fractionalPart1)); then type1=${__shellmath_numericTypes[DECIMAL]}; fi
        if ((10#$fractionalPart2)); then type2=${__shellmath_numericTypes[DECIMAL]}; fi

        _shellmath_reduceCrossPairs "$integerPart1" "$integerPart2" "$fractionalPart1" "$fractionalPart2"
        _shellmath_getReturnValues fractionalPart1 fractionalPart2

        _shellmath_reduceOuterPairs "$fractionalPart1" "$fractionalPart2"
        _shellmath_getReturnValues fractionalPart1 fractionalPart2
    fi

    # Quick multiply & return for integer multiplies
    if ((type1==type2 && type1==__shellmath_numericTypes[INTEGER])); then
        ((isNegative1)) && ((integerPart1*=-1))
        ((isNegative2)) && ((integerPart2*=-1))
        local product=$((integerPart1 * integerPart2))
        if ((rescalingFactor > 0)); then
            _shellmath_rescaleValue "$product" "$rescalingFactor"
            _shellmath_getReturnValue product
        fi
        if (( (!isSubcall) && (isScientific1 || isScientific2) )); then
            _shellmath_numToScientific $product "" 
            _shellmath_getReturnValue product
        fi
        _shellmath_setReturnValue $product
        if (( isVerbose && ! isSubcall )); then
            echo "$product"
        fi
        return "$__shellmath_SUCCESS"
    fi

    # The product has four components per the distributive law
    local intProduct floatProduct innerProduct1 innerProduct2
    # Widths of the decimal parts
    local floatWidth fractionalWidth1 fractionalWidth2

    # Compute the integer and floating-point components
    ((intProduct = integerPart1 * integerPart2))

    fractionalWidth1=${#fractionalPart1}
    fractionalWidth2=${#fractionalPart2}
    ((floatWidth = fractionalWidth1 + fractionalWidth2))
    ((floatProduct = 10#$fractionalPart1 * 10#$fractionalPart2))
    if ((${#floatProduct} < floatWidth)); then
        printf -v floatProduct "%0*d" "$floatWidth" "$floatProduct"
    fi

    # Compute the inner products: First integer-multiply, then rescale
    ((innerProduct1 = integerPart1 * 10#$fractionalPart2))
    ((innerProduct2 = integerPart2 * 10#$fractionalPart1))

    # Rescale the inner products back to decimals so we can shellmath_add() them
    if ((fractionalWidth2 <= ${#innerProduct1})); then
        local innerInt1=${innerProduct1:0:(-$fractionalWidth2)}
        local innerFloat1=${innerProduct1:(-$fractionalWidth2)}
        integerPart1=${innerInt1}
        fractionalPart1=${innerFloat1}
    else
        integerPart1=0
        printf -v fractionalPart1 "%0*d" "$fractionalWidth2" "$innerProduct1"
    fi
    if ((fractionalWidth1 <= ${#innerProduct2})); then
        local innerInt2=${innerProduct2:0:(-$fractionalWidth1)}
        local innerFloat2=${innerProduct2:(-$fractionalWidth1)}
        integerPart2=${innerInt2}
        fractionalPart2=${innerFloat2}
    else
        integerPart2=0
        printf -v fractionalPart2 "%0*d" "$fractionalWidth1" "$innerProduct2"
    fi

    # Combine the distributed parts
    local innerSum product
    # Add the inner products to get the inner sum
    _shellmath_add  "$integerPart1" "$fractionalPart1" "$integerPart2" "$fractionalPart2"
    _shellmath_getReturnValue innerSum
    [[ "$innerSum" =~ (.*)\.(.*) ]]
    integerPart1=${BASH_REMATCH[1]}
    fractionalPart1=${BASH_REMATCH[2]}
    # Add inner sum + outer sum
    _shellmath_add "$integerPart1" "$fractionalPart1" "$intProduct" "$floatProduct"
    _shellmath_getReturnValue product

    # Determine the sign of the product
    if ((isNegative1 != isNegative2)); then
        product="-"$product
    fi

    # When we pre-detect overflow in the integer part of the computation,
    # we compensate by shrinking the inputs by some order of magnitude.
    # Having now finished the computation, we revert to the original magnitude.
    if ((rescalingFactor > 0)); then
        _shellmath_rescaleValue "$product" "$rescalingFactor"
        _shellmath_getReturnValue product
    fi

    # Convert to scientific notation if appropriate
    if (( (!isSubcall) && (isScientific1 || isScientific2) )); then
        _shellmath_numToScientific "${product%.*}" "${product#*.}"
        _shellmath_getReturnValue product
    fi

    # Note the result, print if running "normally", and return
    _shellmath_setReturnValue $product
    if (( isVerbose && ! isSubcall )); then
        echo "$product"
    fi

    return "$__shellmath_SUCCESS"
}


################################################################################
# divide (dividend, divisor)
################################################################################
function _shellmath_divide()
{
    local n1="$1"
    local n2="$2"
    local integerPart1  fractionalPart1  integerPart2  fractionalPart2
    local isNegative1 type1 isScientific1 isNegative2 type2 isScientific2

    if ((! __shellmath_didPrecalc)); then
        _shellmath_precalc; __shellmath_didPrecalc=$__shellmath_true
    fi

    local isVerbose=$(( __shellmath_isOptimized == __shellmath_false ))

    local isTesting=${__shellmath_false}
    if [[ "${FUNCNAME[1]}" == "_shellmath_assert_functionReturn" ]]; then
        isTesting=${__shellmath_true}
    fi

    if [[ $# -eq 0 || $# -gt 2 ]]; then
        echo "Usage: ${FUNCNAME[0]}  dividend  divisor"
        return "$__shellmath_SUCCESS"
    elif [[ $# -eq 1 ]]; then
        # Note the value as-is and return
        _shellmath_setReturnValue "$n1"
        ((isVerbose)) && echo "$n1"
        return "$__shellmath_SUCCESS"
    fi

    # Check and parse the arguments
    local flags
    _shellmath_validateAndParse "$n1";  flags=$?
    _shellmath_getReturnValues  integerPart1  fractionalPart1  isNegative1  type1  isScientific1
    if ((flags == __shellmath_ILLEGAL_NUMBER)); then
        _shellmath_warn  "${__shellmath_returnCodes[ILLEGAL_NUMBER]}"  "$n1"
        return $?
    fi
    _shellmath_validateAndParse "$n2";  flags=$?
    _shellmath_getReturnValues  integerPart2  fractionalPart2  isNegative2  type2  isScientific2
    if ((flags == __shellmath_ILLEGAL_NUMBER)); then
        _shellmath_warn  "${__shellmath_returnCodes[ILLEGAL_NUMBER]}"  "$n2"
        return $?
    fi

    # Throw error on divide by zero
    if ((integerPart2 == 0 && 10#$fractionalPart2 == 0)); then
        _shellmath_warn  "${__shellmath_returnCodes[DIVIDE_BY_ZERO]}"  "$n2"
        return $?
    fi

    # Convert the division problem to an *integer* division problem by rescaling
    # both inputs so as to lose their decimal points. To obtain maximal precision,
    # we scale up the numerator further, padding with as many zeros as it can hold
    local numerator denominator quotient
    local rescaleFactor zeroCount zeroTail

    if ((integerPart1 == 0)); then
        integerPart1=""
    fi
    ((zeroCount = __shellmath_precision - ${#integerPart1} - ${#fractionalPart1}))
    ((rescaleFactor = __shellmath_precision - ${#integerPart1} - ${#fractionalPart2}))
    if ((zeroCount > 0)); then
        printf -v zeroTail "%0*d" "$zeroCount" 0
    fi

    # Rescale and rewrite the fraction to be computed, and compute it
    numerator=${integerPart1}${fractionalPart1}${zeroTail}
    denominator=${integerPart2}${fractionalPart2}
    ((quotient = 10#$numerator / 10#$denominator))

    # For greater precision, re-divide by the remainder to get the next digits of the quotient
    local remainder quotient_2
    ((remainder = 10#$numerator % 10#$denominator))   # cannot exceed numerator or thus, maxValue
    ((zeroCount = __shellmath_precision - ${#remainder}))
    if ((zeroCount > 0)); then
        printf -v zeroTail "%0*d" "$zeroCount" 0
    else
        zeroTail=""
    fi
    # Derive the new numerator from the remainder. Do not change the denominator.
    numerator=${remainder}${zeroTail}
    ((quotient_2 = 10#$numerator / 10#$denominator))
    quotient=${quotient}${quotient_2}
    ((rescaleFactor += ${#quotient_2}))

    # Rescale back. For aesthetic reasons we also round off at the "precision"th decimal place
    ((zeroCount = rescaleFactor - ${#quotient}))
    if ((zeroCount >= 0)); then
        local zeroPrefix="" fractionalPart
        if ((zeroCount > 0)); then
            printf -v zeroPrefix "%0*d" "$((rescaleFactor - ${#quotient}))" 0
        fi
        fractionalPart=${zeroPrefix}${quotient}
        _shellmath_round "$fractionalPart" $__shellmath_precision
        _shellmath_getReturnValue fractionalPart
        quotient="0."${fractionalPart}
    else
        fractionalPart=${quotient:(-$rescaleFactor)}
        _shellmath_round "$fractionalPart" $__shellmath_precision
        _shellmath_getReturnValue fractionalPart
        quotient=${quotient:0:(-$rescaleFactor)}"."${fractionalPart}
    fi

    # Determine the sign of the quotient
    if ((isNegative1 != isNegative2)); then
        quotient="-"$quotient
    fi

    if ((isTesting)); then
        # Trim zeros. (Requires decimal point and zero tail.)
        if [[ "$quotient" =~ [\.].*0$ ]]; then
            # If the decimal point IMMEDIATELY precedes the 0s, remove that too
            [[ $quotient =~ [\.]?0+$ ]]
            quotient=${quotient%${BASH_REMATCH[0]}}
        fi
    fi

    # Convert to scientific notation if appropriate
    if ((isScientific1 || isScientific2)); then
        _shellmath_numToScientific "${quotient%.*}" "${quotient#*.}"
        _shellmath_getReturnValue quotient
    fi

    # Note the result, print if running "normally", and return
    _shellmath_setReturnValue "$quotient"
    if ((isVerbose)); then
        echo "$quotient"
    fi

    return "$__shellmath_SUCCESS"
}

