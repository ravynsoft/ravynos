

// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/libbar.dylib -install_name $RUN_DIR/libbar.dylib
// BUILD:  $CC foo.c -dynamiclib $BUILD_DIR/libbar.dylib -sub_library libbar -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dylib-re-export.exe $BUILD_DIR/libfoo.dylib -L$BUILD_DIR

// RUN:  ./dylib-re-export.exe


#include <stdio.h>

#include "test_support.h"

extern int bar();


int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    if ( bar() == 42 )
        PASS("Success");
    else
        FAIL("Wrong value");

	return 0;
}


