
// BUILD:  $CC base.c -dynamiclib  -install_name $RUN_DIR/libbase.dylib  -o $BUILD_DIR/libbase.dylib
// BUILD:  $CC foo.c  -dynamiclib $BUILD_DIR/libbase.dylib -install_name $RUN_DIR/libfoo.dylib  -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC bar.c  -dynamiclib $BUILD_DIR/libbase.dylib -install_name $RUN_DIR/libbar.dylib  -o $BUILD_DIR/libbar.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlsym-handle.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlsym-handle.exe

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld_priv.h>

#include "test_support.h"

// verify RTLD_DEFAULT search order

int mainSymbol = 4;

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    void* fooHandle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LAZY);
    if ( fooHandle == NULL ) {
        FAIL("libfoo.dylib could not be loaded, %s", dlerror());
    }

    void* barHandle = dlopen(RUN_DIR "/libbar.dylib", RTLD_LAZY);
    if ( barHandle == NULL ) {
        FAIL("libbar.dylib could not be loaded, %s", dlerror());
    }

    // verify fooHandle does not find mainSymbol
    if ( dlsym(fooHandle, "mainSymbol") != NULL ) {
        FAIL("mainSymbol was found with fooHandle");
    }

    // verify fooHandle can find foo
    if ( dlsym(fooHandle, "foo") == NULL ) {
        FAIL("foo not found with fooHandle");
    }

    // verify fooHandle can find base
    if ( dlsym(fooHandle, "base") == NULL ) {
        FAIL("base not found with fooHandle");
    }

    // verify fooHandle cannot find bar
    if ( dlsym(fooHandle, "bar") != NULL ) {
        FAIL("bar found with fooHandle");
    }

    // verify barHandle can find bar
    if ( dlsym(barHandle, "bar") == NULL ) {
        FAIL("bar not found with barHandle");
    }

    // verify barHandle can find base
    if ( dlsym(barHandle, "base") == NULL ) {
        FAIL("base not found with barHandle");
    }

    // verify barHandle cannot find foo
    if ( dlsym(barHandle, "foo") != NULL ) {
        FAIL("foo found with barHandle");
    }

    // verify renamed and re-exported symbols work
    if ( dlsym(RTLD_DEFAULT, "strcmp") == NULL ) {
        FAIL("strcmp not found");
    }
    
    // verify bad handle errors
    if ( dlsym((void*)0xdeadbeef, "malloc") != NULL ) {
        FAIL("malloc found with bad handle");
    }
    else {
        const char* message = dlerror();
        if ( strstr(message, "invalid") == NULL ) {
            FAIL(" invalid handle error message missing 'invalid'");
        }
    }

    PASS("Success");
}

