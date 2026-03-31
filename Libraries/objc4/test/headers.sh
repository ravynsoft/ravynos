#!/bin/sh

# Compile every exported ObjC header as if it were a file in every language.
# This script is executed by test headers.c's TEST_BUILD command.

TESTINCLUDEDIR=$1; shift
TESTLOCALINCLUDEDIR=$1; shift
COMPILE_C=$1; shift
COMPILE_CXX=$1; shift
COMPILE_M=$1; shift
COMPILE_MM=$1; shift
VERBOSE=$1; shift

# stop after any command error
set -e

# echo commands when verbose
if [ "$VERBOSE" != "0" ]; then
    set -x
fi

FILES="$TESTINCLUDEDIR/objc/*.h $TESTLOCALINCLUDEDIR/objc/*.h"
CFLAGS='-fsyntax-only -Wno-unused-function -D_OBJC_PRIVATE_H_'

$COMPILE_C $CFLAGS $FILES
$COMPILE_CXX $CFLAGS $FILES
$COMPILE_M $CFLAGS $FILES
$COMPILE_MM $CFLAGS $FILES
for STDC in '99' '11' ; do
    $COMPILE_C $CFLAGS $FILES -std=c$STDC
    $COMPILE_M $CFLAGS $FILES -std=c$STDC
done
for STDCXX in '98' '03' '11' '14' '17' ; do
    $COMPILE_CXX $CFLAGS $FILES -std=c++$STDCXX
    $COMPILE_MM $CFLAGS $FILES -std=c++$STDCXX
done

echo done
