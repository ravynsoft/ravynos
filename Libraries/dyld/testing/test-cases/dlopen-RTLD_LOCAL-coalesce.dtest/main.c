
// BUILD:  $CC foo1.c -dynamiclib  -install_name $RUN_DIR/libfoo1.dylib -o $BUILD_DIR/libfoo1.dylib
// BUILD:  $CC foo2.c -dynamiclib  -install_name $RUN_DIR/libfoo2.dylib -o $BUILD_DIR/libfoo2.dylib
// BUILD:  $CC foo3.c -dynamiclib  -install_name $RUN_DIR/libfoo3.dylib -o $BUILD_DIR/libfoo3.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-RTLD_LOCAL-coalesce.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlopen-RTLD_LOCAL-coalesce.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "test_support.h"

///
/// This tests the interaction of RTLD_LOCAL and weak-def coalescing.
///
/// Normally, (for correct C++ ODR), dyld coalesces all weak-def symbols
/// across all images, so that only one copy of each weak symbol name
/// is in use.  But, dlopen with RTLD_LOCAL means to "hide" the symbols in
/// that one (top) image being loaded. 
///
///

//      main                *coalA
//      libfoo1.dylib        coalA *coalB
//      libfoo2.dylib        coalA  coalB  coalC        // loaded with RTLD_LOCAL
//      libfoo3.dylib        coalA  coalB *coalC
//

typedef int (*IntProc)(void);

int __attribute__((weak)) coalA = 0;

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    ///
    /// Load three foo dylibs in order
    ///
    void* handle1 = dlopen(RUN_DIR "/libfoo1.dylib", RTLD_GLOBAL);
    void* handle2 = dlopen(RUN_DIR "/libfoo2.dylib", RTLD_LOCAL);
    void* handle3 = dlopen(RUN_DIR "/libfoo3.dylib", RTLD_GLOBAL);
    if ( handle1 == NULL ) {
        FAIL("dlopen(\"libfoo1.dylib\", RTLD_GLOBAL) failed but it should have worked: %s", dlerror());
    }
    if ( handle2 == NULL ) {
        FAIL("dlopen(\"libfoo2.dylib\", RTLD_LOCAL) failed but it should have worked: %s", dlerror());
    }
    if ( handle3 == NULL ) {
        FAIL("dlopen(\"libfoo3.dylib\", RTLD_GLOBAL) failed but it should have worked: %s", dlerror());
    }

    ///
    /// Get accessor functions
    ///
    IntProc foo1_coalA = (IntProc)dlsym(handle1, "foo1_coalA");
    IntProc foo1_coalB = (IntProc)dlsym(handle1, "foo1_coalB");
    IntProc foo2_coalA = (IntProc)dlsym(handle2, "foo2_coalA");
    IntProc foo2_coalB = (IntProc)dlsym(handle2, "foo2_coalB");
    IntProc foo2_coalC = (IntProc)dlsym(handle2, "foo2_coalC");
    IntProc foo3_coalA = (IntProc)dlsym(handle3, "foo3_coalA");
    IntProc foo3_coalB = (IntProc)dlsym(handle3, "foo3_coalB");
    IntProc foo3_coalC = (IntProc)dlsym(handle3, "foo3_coalC");
    if ( !foo1_coalA || !foo1_coalB ||
         !foo2_coalA || !foo2_coalB || !foo2_coalC ||
         !foo3_coalA || !foo3_coalB || !foo3_coalC ) {
        FAIL("dlsym() failed");
    }

    ///
    /// Get values for each coal[ABC] seen in each image
    ///
    int foo1A = (*foo1_coalA)();
    int foo1B = (*foo1_coalB)();
    int foo2A = (*foo2_coalA)();
    int foo2B = (*foo2_coalB)();
    int foo2C = (*foo2_coalC)();
    int foo3A = (*foo3_coalA)();
    int foo3B = (*foo3_coalB)();
    int foo3C = (*foo3_coalC)();
    LOG("coalA in main:    %d", coalA);
    LOG("coalA in libfoo1: %d", foo1A);
    LOG("coalA in libfoo2: %d", foo2A);
    LOG("coalA in libfoo3: %d", foo3A);

    LOG("coalB in libfoo1: %d", foo1B);
    LOG("coalB in libfoo2: %d", foo2B);
    LOG("coalB in libfoo3: %d", foo3B);

    LOG("coalC in libfoo2: %d", foo2C);
    LOG("coalC in libfoo3: %d", foo3C);

    ///
    /// Verify coalescing was done properly (foo2 was skipped because of RTLD_LOCAL)
    ///
    if ( (foo1A != 0) || (foo2A != 0) || (foo3A != 0) || (coalA != 0) ) {
        FAIL("coalA was not coalesced properly");
    }
    if ( (foo1B != 1) || (foo2B != 1) || (foo3B != 1) ) {
        FAIL("coalB was not coalesced properly");
    }
    if ( (foo2C != 2) || (foo3C != 3) ) {
        FAIL("coalC was not coalesced properly");
    }

    PASS("Success");
}
