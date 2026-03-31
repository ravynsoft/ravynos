
// This tests that our header such as dlfcn.h pass unix conformance.

// BUILD_ONLY: MacOSX

// BUILD:  $CC main.c -o $BUILD_DIR/unix-conformance.exe -D_XOPEN_SOURCE=600
// BUILD:  $CC main.c -o $BUILD_DIR/scratch.exe -D_XOPEN_SOURCE=600 -D_POSIX_C_SOURCE=200112

// BUILD: $SKIP_INSTALL $BUILD_DIR/scratch.exe

// RUN:  ./unix-conformance.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <dlfcn.h> 

#include "test_support.h"

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    PASS("Success");
}

