
// BOOT_ARGS: dyld_flags=2

// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/librel.dylib -install_name @rpath/librel.dylib
// BUILD:  $CC main.c $BUILD_DIR/librel.dylib -o $BUILD_DIR/rpath-executable.exe       -rpath @executable_path
// BUILD:  $CC main.c $BUILD_DIR/librel.dylib -o $BUILD_DIR/rpath-executable-slash.exe -rpath @executable_path/
// BUILD:  $CC main.c $BUILD_DIR/librel.dylib -o $BUILD_DIR/rpath-loader.exe           -rpath @loader_path
// BUILD:  $CC main.c $BUILD_DIR/librel.dylib -o $BUILD_DIR/rpath-loader-slash.exe     -rpath @loader_path/

// RUN: ./rpath-executable.exe
// RUN: ./rpath-executable-slash.exe
// RUN: ./rpath-loader.exe
// RUN: ./rpath-loader-slash.exe

// main prog links with librel.dylib.  There are four variants of how LC_RPATH is set up.

#include <stdio.h>

#include "test_support.h"

extern char* __progname;


int main(int argc, const char* argv[])
{
    PASS("%s", __progname);
}


