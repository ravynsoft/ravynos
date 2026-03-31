
// BUILD:  $CC good.c -dynamiclib -install_name @rpath/libtest.dylib     -o $BUILD_DIR/good/libtest.dylib
// BUILD:  $CC bad.c  -dynamiclib -install_name @rpath/libtest.dylib     -o $BUILD_DIR/bad/libtest.dylib
// BUILD:  $CC dyn.c  -dynamiclib -install_name @rpath/libdynamic.dylib  -o $BUILD_DIR/dir/libdynamic.dylib $BUILD_DIR/good/libtest.dylib -rpath @loader_path/../bad
// BUILD:  $CC main.c $BUILD_DIR/good/libtest.dylib -DRUN_DIR="$RUN_DIR" -o $BUILD_DIR/dlopen-rpath-prev-override.exe -rpath @loader_path/good

// RUN:  ./dlopen-rpath-prev-override.exe

// Processing of @rpath in dlopen()s always checks existing loaded images before using LC_RPATHs to find images on disk
//
// main links with @rpath/libtest.dylib found via -rpath @loader_path/good
// main dlopen()s libdynamic.dylib which links with @rpath/libtest.dylib and has LR_PATH to find it in bad/,
// but since it was already loaded from good/, that one should be re-used.

#include <stdio.h>
#include <dlfcn.h>

#include "test_support.h"

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    void* handle = dlopen(RUN_DIR "/dir/libdynamic.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        FAIL("dlerror(): %s", dlerror());
    }

    PASS("Success");
}

