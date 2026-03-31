
// BUILD:  $CC main.c           -o $BUILD_DIR/foo.exe
// BUILD:  $CC main.c           -o $BUILD_DIR/dyld_need_closure.exe

// RUN:    ./dyld_need_closure.exe

#include <stdio.h>
#include <stdlib.h>
#include <mach-o/dyld_priv.h>

#include "test_support.h"

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    // We only support trying to save to containerised paths, so anything not
    // of that form should fail
    if ( !dyld_need_closure("./foo.exe", "/tmp/Containers/Data/") ) {
        FAIL("Should have needed a closure for containerised path");
    }

    if ( dyld_need_closure("./foo.exe", "/tmp/Containers/Data2/") ) {
        FAIL("Should have rejected a closure for non-containerised path");
    }

    PASS("Success");
}
