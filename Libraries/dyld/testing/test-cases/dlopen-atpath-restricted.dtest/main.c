// BUILD_ONLY: MacOSX

// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/test1/libtest1.dylib -install_name @rpath/libtest1.dylib
// BUILD:  $CC foo.c -bundle -o $BUILD_DIR/test1.bundle -Wl,-rpath,@loader_path/test1/ $BUILD_DIR/test1/libtest1.dylib

// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/test2/libtest2.dylib -install_name @loader_path/test2/libtest2.dylib
// BUILD:  $CC foo.c -bundle -o $BUILD_DIR/test2.bundle $BUILD_DIR/test2/libtest2.dylib

// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/test3/libtest3.dylib -install_name @rpath/libtest3.dylib
// BUILD:  $CC foo.c -bundle -o $BUILD_DIR/test3.bundle -Wl,-rpath,$RUN_DIR/test3  $BUILD_DIR/test3/libtest3.dylib 

// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/test4/libtest4.dylib -install_name @rpath/libtest4.dylib
// BUILD:  $CC foo.c -bundle -o $BUILD_DIR/test4.bundle -Wl,-rpath,@executable_path/test4/ $BUILD_DIR/test4/libtest4.dylib

// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/test5/libtest5.dylib -install_name @executable_path/test5/libtest5.dylib
// BUILD:  $CC foo.c -bundle -o $BUILD_DIR/test5.bundle $BUILD_DIR/test5/libtest5.dylib

// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-restricted.exe -DRUN_DIR="$RUN_DIR" -sectcreate __RESTRICT __restrict /dev/null

// RUN:  ./dlopen-restricted.exe

#include <stdio.h>
#include <dlfcn.h>

#include "test_support.h"

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {

    // test1: LC_RPATH not in main executable uses @loader_path
    void* handle = dlopen(RUN_DIR "/test1.bundle", RTLD_LAZY);
    if ( handle == NULL ) {
        FAIL("test1.bundle dlerror(): %s", dlerror());
    }

    // test2: @loader_path not in main executable
    handle = dlopen(RUN_DIR "/test2.bundle", RTLD_LAZY);
    if ( handle == NULL ) {
        FAIL("test2.bundle\n dlerror(): %s", dlerror());
    }

    // test3: LC_RPATH not in main executable uses absolute path
    handle = dlopen(RUN_DIR "/test3.bundle", RTLD_LAZY);
    if ( handle == NULL ) {
        FAIL("test3.bundle dlerror(): %s", dlerror());
    }

    // test4: [SHOULD FAIL] LC_RPATH not in main executable uses @executable_path
    handle = dlopen(RUN_DIR "/test4.bundle", RTLD_LAZY);
    if ( handle != NULL ) {
        FAIL("test4.bundle dlopen() should not work");
    }

    // test5: [SHOULD FAIL] @executable_path in LC_LOAD_DYLIB
    handle = dlopen(RUN_DIR "/test5.bundle", RTLD_LAZY);
    if ( handle != NULL ) {
        FAIL("test5.bundle dlopen() should not work");
    }

    PASS("Success");
}

