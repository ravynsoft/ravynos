

// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-in-init.exe $BUILD_DIR/libfoo.dylib

// RUN:  ./dlopen-in-init.exe

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "test_support.h"

__attribute__((constructor))
void myinit()
{

}


int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    // The test is for libdyld.dylib to not crash when libfoo.dylib dlopen() stuff in its initializer
    PASS("Success");
}

