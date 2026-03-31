// BUILD_ONLY:      MacOSX
// BUILD_MIN_OS:    10.5
// BUILD:           $CC bar.c -dynamiclib -install_name $RUN_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib
// BUILD:           $CC foo.c -dynamiclib $BUILD_DIR/libbar.dylib -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:           $CC main.c -o $BUILD_DIR/weak-def-bind-old-format.exe $BUILD_DIR/libfoo.dylib $BUILD_DIR/libbar.dylib -L$BUILD_DIR

// RUN:  ./weak-def-bind-old-format.exe


#include <stdio.h>

#include "test_support.h"

extern int foo();
extern int bar();


int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    if ( foo() != 42 ) {
        FAIL("weak-def-bind-old-format, wrong value");
	}
	if ( bar() != 42 ) {
        FAIL("weak-def-bind-old-format, wrong value");
	}

    PASS("weak-def-bind-old-format");
}


