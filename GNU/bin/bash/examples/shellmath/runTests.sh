#!/bin/env bash

###############################################################################
# runTests.sh
#
# Usage: runTests.sh  [testFile]
#        where testFile defaults to testCases.in
#
# Processes a test file such as the testCases.in included with this package
###############################################################################

# Process one line from the test cases file. Invoked below through mapfile.
function _shellmath_runTests()
{
    local lineNumber=$1
    local text=$2

    # Trim leading whitespace
    [[ $text =~ ^[$' \t']*(.*) ]]
    text=${BASH_REMATCH[1]}

    # Skip comments and blank lines
    [[ "$text" =~ ^# || -z $text ]] && return 0

    # Check for line continuation
    local len="${#text}"
    if [[ ${text:$((len-1))} == '\' ]]; then

        # Eat the continuation character and add to the buffer
        __shellfloat_commandBuffer+="${text/%\\/ }"
        
        # Defer processing
        return

    # No line continuation
    else

        # Assemble the command
        local command=${__shellfloat_commandBuffer}${text}
        __shellfloat_commandBuffer=""

        words=($command)

        # Expand first word to an assertion function
        case ${words[0]} in

            Code)
                words[0]=_shellmath_assert_return${words[0]}

                # Validate next word as a positive integer
                if [[ ! "${words[1]}" =~ ^[0-9]+$ ]]; then
                    echo Line: "$lineNumber": Command "$command"
                    echo FAIL: \"Code\" requires integer return code
                    return 1
                else
                    nextWord=2
                fi
                ;;

            String)
                words[0]=_shellmath_assert_return${words[0]}
                # Allow multiword arguments if quoted
                if [[ ${words[1]} =~ ^\" ]]; then
                    if [[ ${words[1]} =~ \"$ ]]; then
                        nextWord=2
                    else
                        for ((nextWord=2;;nextWord++)); do
                            if [[ ${words[nextWord]} =~ \"$ ]]; then
                                ((nextWord++))
                                break
                            fi
                        done
                    fi
                else
                    nextWord=2
                fi
                ;;

            Both)
                ;;

            *)
                echo -e ${RED}FAIL${NO_COLOR} Line "$lineNumber": Command "$command": Code or String indicator required
                return 2
                ;;
        esac

        # Expand the next word to a shellmath function name
        words[nextWord]=_shellmath_${words[nextWord]}
        if ! type -t "${words[nextWord]}" >/dev/null; then
            echo "${RED}FAIL${NO_COLOR} Line $lineNumber: Command "$command": Syntax error. Required: String|Code  value  operation  args..."
            return 3
        fi

        # Run the command, being respectful of shell metacharacters
        fullCommand="${words[*]}"
        eval "$fullCommand"
        local returnString
        _shellmath_getReturnValue returnString
        echo -e "$returnString" Line "$lineNumber": "$command"

    fi

}


function _main()
{
    source shellmath.sh
    source assert.sh

    # Initialize certain globals. As "public" functions, the arithmetic
    # functions need to do this themselves, but there are some "private"
    # functions that need this here when they are auto-tested.
    _shellmath_precalc; __shellmath_didPrecalc=$__shellmath_true

    # Process the test file line-by-line using the above runTests() function
    mapfile -t -c 1 -C _shellmath_runTests -O 1 < "${1:-testCases.in}"
    
    exit 0
}

_main "$@"

