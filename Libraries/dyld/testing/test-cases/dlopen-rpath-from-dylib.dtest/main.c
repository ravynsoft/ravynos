
// BUILD:  $CC bar.c  -dynamiclib -install_name @rpath/libbar.dylib -o $BUILD_DIR/dir/libbar.dylib
// BUILD:  $CC test.c -dynamiclib -install_name $RUN_DIR/libtest.dylib -o $BUILD_DIR/libtest.dylib -rpath @loader_path/dir
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-rpath-from-dylib.exe $BUILD_DIR/libtest.dylib 

// RUN:  ./dlopen-rpath-from-dylib.exe

#include <stdio.h>
#include <dlfcn.h>
#include <stdbool.h>

#include "test_support.h"

/// test that a call to dlopen() from a dylib uses the LC_RPATH from that dylib

extern void test_dlopen();

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    test_dlopen();
    PASS("Succcess");
}

