
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c -DRUN_DIR="$RUN_DIR" $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/dlopen-RTLD_NOLOAD-basic.exe
// BUILD:  $SYMLINK libfoo.dylib $BUILD_DIR/libfoo-sym.dylib

// RUN:  ./dlopen-RTLD_NOLOAD-basic.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "test_support.h"

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    ///
    /// This tests that RTLD_NOLOAD finds existing dylib statically linked
    ///
    void* handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_NOLOAD);
    if ( handle == NULL ) {
        FAIL("dlopen(\"libfoo.dylib\", RTLD_NOLOAD) failed but it should have worked: %s", dlerror());
    }
    void* sym = dlsym(handle, "foo");
    if ( sym == NULL ) {
        FAIL("dlsym(handle, \"foo\") failed but it should have worked: %s", dlerror());
    }

    ///
    /// This tests that RTLD_NOLOAD verifies that non-existant dylib returns NULL
    ///
    void* handle2 = dlopen(RUN_DIR "/libfobbulate.dylib", RTLD_NOLOAD);
    if ( handle2 != NULL ) {
        FAIL("dlopen(\"libfobbulate.dylib\", RTLD_NOLOAD) succeeded but it should have failed");
    }


    ///
    /// This tests that RTLD_NOLOAD finds symlink to existing dylib
    ///
    void* handle3 = dlopen(RUN_DIR "/libfoo-sym.dylib", RTLD_NOLOAD);
    if ( handle3 == NULL ) {
        FAIL("dlopen(\"libfoo-sym.dylib\", RTLD_NOLOAD) failed but it should have worked: %s", dlerror());
    }


    ///
    /// This tests that RTLD_NOLOAD of something in the dyld cache that is not yet loaded returns NULL
    ///
    void* handle4 = dlopen("/usr/lib/libz.1.dylib", RTLD_NOLOAD);
    if ( handle4 != NULL ) {
        FAIL("dlopen(\"libz.dylib\", RTLD_NOLOAD) worked but it should have failed");
    }

    PASS("Success");
}
