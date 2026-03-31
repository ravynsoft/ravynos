
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/libbar.dylib         -install_name $RUN_DIR/libbar.dylib
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/libbar-present.dylib  -install_name $RUN_DIR/libbar.dylib -DHAS_SYMBOL=1
// BUILD:  $CC foo.c -dynamiclib -Os -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib $BUILD_DIR/libbar-present.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-RTLD_NOW.exe -DRUN_DIR="$RUN_DIR"

// BUILD: $SKIP_INSTALL $BUILD_DIR/libbar-present.dylib

// RUN:  ./dlopen-RTLD_NOW.exe

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/getsect.h>

#include "test_support.h"

#if __LP64__
extern struct mach_header_64 __dso_handle;
#else
extern struct mach_header __dso_handle;
#endif

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    ///
    /// This tests that RTLD_NOW on dlopen() will return NULL because call from libfoo to libbar could not be bound
    ///
    void* handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_NOW);
    if ( handle != NULL ) {
        FAIL("dlopen(\"libfoo.dylib\", RTLD_NOW) should have failed");
    }


#if __arm64e__
    // arm64e always uses chained binds which does not support lazy binding
    bool supportsLazyBinding = false;
#else
    // other architectures may or may not use lazy binding
    unsigned long sectSize = 0;
    bool supportsLazyBinding = (getsectiondata(&__dso_handle, "__DATA", "__la_symbol_ptr", &sectSize) != NULL);
  #if __ARM_ARCH_7K__
    // armv7 has two names for lazy pointers section
    if ( !supportsLazyBinding )
        supportsLazyBinding = (getsectiondata(&__dso_handle, "__DATA", "__lazy_symbol", &sectSize) != NULL);
  #endif
#endif

    ///
    /// This tests that RTLD_LAZY on dlopen() will succeed if libfoo.dylib
    ///
    handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LAZY);
    if ( supportsLazyBinding ) {
        if ( handle == NULL ) {
            FAIL("dlopen(\"libfoo.dylib\", RTLD_LAZY) should have succeeded: %s", dlerror());
        }
    }
    else {
        if ( handle != NULL ) {
            FAIL("dlopen(\"libfoo.dylib\", RTLD_LAZY) should have failed becuase a symbol was missing");
        }
    }

    PASS("Success");
}
