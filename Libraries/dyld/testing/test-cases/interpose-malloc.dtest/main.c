// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/interpose-malloc.exe
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/interpose-malloc.exe
// BUILD:  $CC interposer.c -dynamiclib -o $BUILD_DIR/libmyalloc.dylib -install_name libmyalloc.dylib

// RUN:    DYLD_INSERT_LIBRARIES=libmyalloc.dylib   ./interpose-malloc.exe



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_support.h"

extern void* myalloc1(size_t);
extern void* myalloc2(size_t);

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    char* p1 = malloc(10);
    if ( strncmp(p1+10, "##########", 10) != 0 ) {
        FAIL("malloc() from main executable not interposed");
    }

    void* p2 = myalloc1(6);
    if ( strncmp(p2+6, "######", 6) != 0 ) {
        FAIL("myalloc1() from libfoo.dylib not interposed");
    }

    void* p3 = myalloc2(10);
    if ( strncmp(p3+10, "##########", 10) != 0 ) {
        FAIL("myalloc2() from libfoo.dylib not interposed");
    }

    void* p4 = strdup("hello");
     if ( strncmp(p4+6, "#######", 6) != 0 ) {
        FAIL("malloc() from strdup not interposed");
    }

    LOG("%p %p %p %p", p1, p2, p3, p4);
    PASS("Success");
}
