
// BUILD:  $CC base.c  -dynamiclib -install_name $RUN_DIR/libbase.dylib  -o $BUILD_DIR/libbase.dylib
// BUILD:  $CC foo.c   -dynamiclib -install_name $RUN_DIR/libdynamic.dylib  -o $BUILD_DIR/libdynamic.dylib $BUILD_DIR/libbase.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/static-terminators.exe -DRUN_DIR="$RUN_DIR" $BUILD_DIR/libbase.dylib

// RUN:  ./static-terminators.exe

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld_priv.h>

#include "test_support.h"

// verify all static terminators run in proper order


extern void mainTerminated();

static __attribute__((destructor))
void myTerm()
{
    LOG("main's static terminator");
    mainTerminated();
}


int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    // load dylib
    void* handle = dlopen(RUN_DIR "/libdynamic.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        FAIL("libdynamic.dylib could not be loaded, %s", dlerror());
    }

    // PASS is printed in libbase.dylib terminator
}

