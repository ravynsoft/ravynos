
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC bar.c -dynamiclib  -install_name $RUN_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-RTLD_LOCAL-hides.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlopen-RTLD_LOCAL-hides.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "test_support.h"

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    ///
    /// This tests that RTLD_LOCAL prevents RTLD_DEFAULT from finding symbols, but can be found via handle
    ///
    void* handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LOCAL);
    if ( handle == NULL ) {
        FAIL("dlopen(\"libfoo.dylib\", RTLD_LOCAL) failed but it should have worked: %s", dlerror());
    }
    void* sym = dlsym(handle, "foo");
    if ( sym == NULL ) {
        FAIL("dlsym(handle, \"foo\") failed but it should have worked: %s", dlerror());
    }
    void* sym2 = dlsym(RTLD_DEFAULT, "foo");
    if ( sym2 != NULL ) {
        FAIL("dlsym(RTLD_DEFAULT, \"foo\") succeeded but it should have failed");
    }

    ///
    /// This tests that RTLD_GLOBAL after RTLD_LOCAL allows RTLD_DEFAULT to find symbols
    ///
    void* handle2 = dlopen(RUN_DIR "/libfoo.dylib", RTLD_GLOBAL);
    if ( handle2 == NULL ) {
        FAIL("dlopen(\"libfoo.dylib\", RTLD_GLOBAL) failed but it should have worked: %s", dlerror());
    }
    void* sym3 = dlsym(RTLD_DEFAULT, "foo");
    if ( sym3 == NULL ) {
        FAIL("dlsym(RTLD_DEFAULT, \"foo\") failed after upgrading to RTLD_GLOBAL");
    }

    ///
    /// This tests that RTLD_LOCAL after RTLD_GLOBAL does not block RTLD_DEFAULT from finding symbols
    ///
    void* handle3 = dlopen(RUN_DIR "/libbar.dylib", RTLD_GLOBAL);
    if ( handle3 == NULL ) {
        FAIL("dlopen(\"libbar.dylib\", RTLD_GLOBAL) failed but it should have worked: %s", dlerror());
    }
    void* handle4 = dlopen(RUN_DIR "/libbar.dylib", RTLD_LOCAL);
    if ( handle4 == NULL ) {
        FAIL("dlopen(\"libbar.dylib\", RTLD_LOCAL) failed but it should have worked: %s", dlerror());
    }
    void* sym4 = dlsym(RTLD_DEFAULT, "bar");
    if ( sym4 == NULL ) {
        FAIL("dlsym(RTLD_DEFAULT, \"bar\") failed but it should have worked: %s", dlerror());
    }

    PASS("Success");
}
