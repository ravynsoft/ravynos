// BUILD_ONLY: MacOSX

// BUILD:  $CC foo.s -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/flat-namespace.exe -flat_namespace

// RUN:    ./flat-namespace.exe



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_support.h"

extern int myAbs1;
int* ptr = &myAbs1;

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    if ( ptr != 0 ) {
        FAIL("Absolute symbol not bound to zero with flat lookup");
    }

    PASS("Success");
}
