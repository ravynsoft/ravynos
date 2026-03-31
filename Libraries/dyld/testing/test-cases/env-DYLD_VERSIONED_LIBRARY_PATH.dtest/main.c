// BUILD_ONLY: MacOSX

// BUILD:  $CC foo.c -dynamiclib -DRESULT=9  -current_version 9  -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/alt9/libfoo.dylib
// BUILD:  $CC foo.c -dynamiclib -DRESULT=10 -current_version 10 -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC foo.c -dynamiclib -DRESULT=11 -current_version 11 -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/alt11/libfoo.dylib
// BUILD:  $CC foo.c -dynamiclib -DRESULT=12 -current_version 12 -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/alt12/libfoo.dylib

// BUILD:  $CC foo.c -dynamiclib -DRESULT=10 -current_version 10 -install_name $RUN_DIR/libfoo2.dylib -o $BUILD_DIR/libfoo2.dylib
// BUILD:  $CC foo.c -dynamiclib -DRESULT=12 -current_version 12 -install_name $RUN_DIR/libfoo2.dylib -o $BUILD_DIR/alt12/libfoo2.dylib

// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH.exe $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-10.exe $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-11.exe $BUILD_DIR/libfoo.dylib -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@loader_path/alt11
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-911.exe $BUILD_DIR/libfoo.dylib -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@loader_path/alt9 -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@loader_path/alt11
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-911b.exe $BUILD_DIR/libfoo.dylib -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@loader_path/alt9:@loader_path/alt11
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-911c.exe $BUILD_DIR/libfoo.dylib -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@executable_path/alt9:@executable_path/alt11
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-1112.exe $BUILD_DIR/libfoo.dylib -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@loader_path/alt11 -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@loader_path/alt12
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-1112b.exe $BUILD_DIR/libfoo.dylib -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@loader_path/alt11:@loader_path/alt12
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-1211.exe $BUILD_DIR/libfoo.dylib -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@loader_path/alt12 -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@loader_path/alt11
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-1211b.exe $BUILD_DIR/libfoo.dylib -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@loader_path/alt12:@loader_path/alt11
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-missing.exe $BUILD_DIR/libfoo2.dylib -Wl,-dyld_env,DYLD_VERSIONED_LIBRARY_PATH=@loader_path/alt12
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-dlopen.exe -DUSE_DLOPEN -DRUN_DIR="$RUN_DIR" -DDYLIB_NAME="libfoo.dylib"
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-missing-dlopen.exe -DUSE_DLOPEN -DRUN_DIR="$RUN_DIR" -DDYLIB_NAME="libfoo2.dylib"

// BUILD:  $SKIP_INSTALL $BUILD_DIR/libfoo2.dylib

// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH.exe
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/env-DYLD_VERSIONED_LIBRARY_PATH-11.exe

// RUN: ./env-DYLD_VERSIONED_LIBRARY_PATH.exe 10
// RUN: DYLD_VERSIONED_LIBRARY_PATH=$RUN_DIR/alt11 ./env-DYLD_VERSIONED_LIBRARY_PATH.exe 11
// RUN: DYLD_VERSIONED_LIBRARY_PATH=$RUN_DIR/alt9 ./env-DYLD_VERSIONED_LIBRARY_PATH.exe 10
// RUN: DYLD_VERSIONED_LIBRARY_PATH=$RUN_DIR/alt9:$RUN_DIR/alt11 ./env-DYLD_VERSIONED_LIBRARY_PATH.exe 11
// RUN: DYLD_VERSIONED_LIBRARY_PATH=$RUN_DIR/alt11:$RUN_DIR/alt12 ./env-DYLD_VERSIONED_LIBRARY_PATH.exe 12
// RUN: ./env-DYLD_VERSIONED_LIBRARY_PATH-10.exe 10
// RUN: ./env-DYLD_VERSIONED_LIBRARY_PATH-11.exe 11
// RUN: ./env-DYLD_VERSIONED_LIBRARY_PATH-911.exe 11
// RUN: ./env-DYLD_VERSIONED_LIBRARY_PATH-911b.exe 11
// RUN: ./env-DYLD_VERSIONED_LIBRARY_PATH-911c.exe 11
// RUN: ./env-DYLD_VERSIONED_LIBRARY_PATH-1112.exe 12
// RUN: ./env-DYLD_VERSIONED_LIBRARY_PATH-1112b.exe 12
// RUN: ./env-DYLD_VERSIONED_LIBRARY_PATH-1211.exe 12
// RUN: ./env-DYLD_VERSIONED_LIBRARY_PATH-1211b.exe 12
// RUN: DYLD_VERSIONED_LIBRARY_PATH=$RUN_DIR/alt9 ./env-DYLD_VERSIONED_LIBRARY_PATH-11.exe 11
// RUN: DYLD_VERSIONED_LIBRARY_PATH=$RUN_DIR/alt12 ./env-DYLD_VERSIONED_LIBRARY_PATH-11.exe 12
// FIXME: Forcibly disable testing with closures since macOS does not use them and they are currently broken
// RUN: DYLD_USE_CLOSURES=0 ./env-DYLD_VERSIONED_LIBRARY_PATH-missing.exe 12

// RUN: ./env-DYLD_VERSIONED_LIBRARY_PATH-dlopen.exe 10
// RUN: DYLD_VERSIONED_LIBRARY_PATH=$RUN_DIR/alt11 ./env-DYLD_VERSIONED_LIBRARY_PATH-dlopen.exe 11
// RUN: DYLD_VERSIONED_LIBRARY_PATH=$RUN_DIR/alt9 ./env-DYLD_VERSIONED_LIBRARY_PATH-dlopen.exe 10
// RUN: DYLD_VERSIONED_LIBRARY_PATH="/AppleInternal/CoreOS/tests/dyld/env-DYLD_VERSIONED_LIBRARY_PATH/alt12" ./env-DYLD_VERSIONED_LIBRARY_PATH-missing-dlopen.exe 12

#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <string.h>
#include <stdlib.h> // for atoi()

#include "test_support.h"

#if USE_DLOPEN
#include <dlfcn.h>
#else
extern int foo();
#endif

int main(int argc, const char* argv[])
{
	int expectedResult = atoi(argv[1]);
#if USE_DLOPEN
    void * handle = dlopen(RUN_DIR "/" DYLIB_NAME, RTLD_LAZY);
    if (!handle) {
        FAIL("dlopen(\"%s\") failed with error \"%s\"", RUN_DIR "/" DYLIB_NAME, dlerror());
    }
    int (*foo)() = dlsym(handle, "foo");
    if (!foo) {
        FAIL("dlsym(\"foo\") failed with error \"%s\"", dlerror());
    }
#endif
	int actualResult = foo();
    if ( actualResult != expectedResult ) {
		FAIL("Using wrong dylib. foo() returned %d, expected %d", actualResult, expectedResult);
    } else {
		PASS("Success");
    }
	return 0;
}

