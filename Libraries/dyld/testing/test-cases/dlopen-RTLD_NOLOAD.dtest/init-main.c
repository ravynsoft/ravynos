
// BUILD:  $CC init-b.c -dynamiclib -DRUN_DIR="$RUN_DIR" -install_name $RUN_DIR/libInitB.dylib -o $BUILD_DIR/libInitB.dylib
// BUILD:  $CC init-a.c -dynamiclib                      -install_name $RUN_DIR/libInitA.dylib $BUILD_DIR/libInitB.dylib -o $BUILD_DIR/libInitA.dylib
// BUILD:  $CC init-main.c $BUILD_DIR/libInitA.dylib -o $BUILD_DIR/dlopen-RTLD_NOLOAD-in-initializer.exe

// RUN:  ./dlopen-RTLD_NOLOAD-in-initializer.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dlfcn.h>

#include "test_support.h"

extern bool initsInWrongOrder;
extern bool allInitsDone();

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    ///
    /// This tests that using RTLD_NOLOAD in an initializer does not trigger out of order initializers
    ///
    if ( initsInWrongOrder )
        FAIL("wrong init order");
    else if ( !allInitsDone() )
        FAIL("all initializers not run");
    else
        PASS("Success");
}
