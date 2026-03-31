
// BUILD_ONLY: MacOSX

// BUILD:  $CC foo.c -dynamiclib -DRESULT=9  -current_version 9  -install_name $RUN_DIR/Foo.framework/Foo -o $BUILD_DIR/alt9/Foo.framework/Foo
// BUILD:  $CC foo.c -dynamiclib -DRESULT=10 -current_version 10 -install_name $RUN_DIR/Foo.framework/Foo -o $BUILD_DIR/Foo.framework/Foo
// BUILD:  $CC foo.c -dynamiclib -DRESULT=11 -current_version 11 -install_name $RUN_DIR/Foo.framework/Foo -o $BUILD_DIR/alt11/Foo.framework/Versions/A/Foo
// BUILD:  $CC foo.c -dynamiclib -DRESULT=12 -current_version 12 -install_name $RUN_DIR/Foo.framework/Foo -o $BUILD_DIR/alt12/Foo.framework/Foo

// BUILD:  $CC foo.c -dynamiclib -DRESULT=10 -current_version 10 -install_name $RUN_DIR/Foo2.framework/Foo2 -o $BUILD_DIR/Foo2.framework/Foo2
// BUILD:  $CC foo.c -dynamiclib -DRESULT=12 -current_version 12 -install_name $RUN_DIR/Foo2.framework/Foo2 -o $BUILD_DIR/alt12/Foo2.framework/Foo2

// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_FRAMEWORK_PATH.exe $BUILD_DIR/Foo.framework/Foo
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_VERSIONED_FRAMEWORK_PATH-missing.exe -Wl,-dyld_env,DYLD_VERSIONED_FRAMEWORK_PATH=@loader_path/alt12 $BUILD_DIR/Foo2.framework/Foo2

// BUILD:  $SYMLINK Versions/A/Foo  $BUILD_DIR/alt11/Foo.framework/Foo $DEPENDS_ON $BUILD_DIR/alt11/Foo.framework/Versions/A/Foo
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/env-DYLD_VERSIONED_FRAMEWORK_PATH.exe

// RUN: ./env-DYLD_VERSIONED_FRAMEWORK_PATH.exe 10
// RUN: DYLD_VERSIONED_FRAMEWORK_PATH=$RUN_DIR/alt11 ./env-DYLD_VERSIONED_FRAMEWORK_PATH.exe 11 "alt11/Foo.framework/Versions/A/Foo"
// RUN: DYLD_VERSIONED_FRAMEWORK_PATH=$RUN_DIR/alt9 ./env-DYLD_VERSIONED_FRAMEWORK_PATH.exe 10
// RUN: DYLD_VERSIONED_FRAMEWORK_PATH=$RUN_DIR/alt9:$RUN_DIR/alt11 ./env-DYLD_VERSIONED_FRAMEWORK_PATH.exe 11
// RUN: DYLD_VERSIONED_FRAMEWORK_PATH=$RUN_DIR/alt11:$RUN_DIR/alt12 ./env-DYLD_VERSIONED_FRAMEWORK_PATH.exe 12
// FIXME: Forcibly disable testing with closures since macOS does not use them and they are currently broken
// RUN: DYLD_USE_CLOSURES=0 ./env-DYLD_VERSIONED_FRAMEWORK_PATH-missing.exe 12

#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h> // for atoi()

#include <mach-o/dyld_priv.h>

#include "test_support.h"

extern int foo();

int main(int argc, const char* argv[])
{
    if ( argc > 2 ) {
        bool found = false;
        uint32_t count = _dyld_image_count();
        for(uint32_t i=0; i < count; ++i) {
            const char*  name = _dyld_get_image_name(i);
            if ( strstr(name, argv[2]) != NULL ) {
                found = true;
            }
        }
        if ( !found ) {
            FAIL("Dylib has wrong path");
            return EXIT_SUCCESS;
        }
    }

    int expectedResult = atoi(argv[1]);
    int actualResult = foo();
    if ( actualResult != expectedResult ) {
        FAIL("Using wrong dylib. foo() returned %d, expected %d", actualResult, expectedResult);
    } else {
        PASS("Success");
    }
    return 0;
}

