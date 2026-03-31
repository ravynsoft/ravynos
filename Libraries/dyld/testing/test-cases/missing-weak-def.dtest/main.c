
// BUILD:  $CC bar.c -dynamiclib -install_name $RUN_DIR/libbar.dylib -o $BUILD_DIR/missing/libbar.dylib
// BUILD:  $CC bar-empty.c -dynamiclib -install_name $BUILD_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib
// BUILD:  $CC main.c $BUILD_DIR/missing/libbar.dylib -o $BUILD_DIR/missing-weak-def.exe

// BUILD: $SKIP_INSTALL $BUILD_DIR/missing/libbar.dylib

// RUN:  ./missing-weak-def.exe

// bar is a weak_import weak bind and the libbar we have at runtime is missing that symbol


#include <stdio.h>
#include <stdlib.h>

#include "test_support.h"

__attribute__((weak))
__attribute__((weak_import))
int bar();

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    if (&bar) {
        FAIL("bar from libbar not bound");
    }

    PASS("Success");
}

