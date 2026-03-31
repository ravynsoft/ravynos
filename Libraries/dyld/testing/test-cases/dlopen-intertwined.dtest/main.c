

// BUILD:  $CC base.c -dynamiclib -o $BUILD_DIR/libbase.dylib -install_name $RUN_DIR/libbase.dylib
// BUILD:  $CC A.c -dynamiclib -o $BUILD_DIR/libA.dylib -install_name $RUN_DIR/libA.dylib $BUILD_DIR/libbase.dylib
// BUILD:  $CC C.c -dynamiclib -o $BUILD_DIR/libC.dylib -install_name $RUN_DIR/libC.dylib $BUILD_DIR/libbase.dylib
// BUILD:  $CC B.c -dynamiclib -o $BUILD_DIR/libB.dylib -install_name $RUN_DIR/libB.dylib $BUILD_DIR/libbase.dylib $BUILD_DIR/libC.dylib
// BUILD:  $CC D.c -dynamiclib -o $BUILD_DIR/libD.dylib -install_name $RUN_DIR/libD.dylib $BUILD_DIR/libbase.dylib $BUILD_DIR/libC.dylib
// BUILD:  $CC E.c -dynamiclib -o $BUILD_DIR/libE.dylib -install_name $RUN_DIR/libE.dylib $BUILD_DIR/libbase.dylib $BUILD_DIR/libA.dylib
// BUILD:  $CC F.c -dynamiclib -o $BUILD_DIR/libF.dylib -install_name $RUN_DIR/libF.dylib $BUILD_DIR/libbase.dylib $BUILD_DIR/libD.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-intertwined.exe $BUILD_DIR/libbase.dylib $BUILD_DIR/libA.dylib -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlopen-intertwined.exe

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>

#include "test_support.h"

// main deps on A
// main dlopens B which deps on C
// main dlopens D which deps on C
// main dlopens E which deps on A
// main dlopens F which deps on D

extern void a(const char*);
extern void setState(const char* from);

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    a("main");
    
    void* handle = dlopen(RUN_DIR "/libB.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        FAIL("Error: %s", dlerror());
    }

    handle = dlopen(RUN_DIR "/libD.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        FAIL("Error: %s", dlerror());
    }

    handle = dlopen(RUN_DIR "/libE.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        FAIL("Error: %s", dlerror());
    }

    handle = dlopen(RUN_DIR "/libF.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        FAIL("Error: %s", dlerror());
    }

    setState("DONE");

    PASS("Success");
}

