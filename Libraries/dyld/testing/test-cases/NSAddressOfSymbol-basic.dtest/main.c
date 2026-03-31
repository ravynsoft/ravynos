// BUILD_ONLY: MacOSX

// BUILD:  $CC main.c            -o $BUILD_DIR/NSAddressOfSymbol-basic.exe -Wno-deprecated-declarations

// RUN:  ./NSAddressOfSymbol-basic.exe



#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>

#include "test_support.h"

extern struct mach_header __dso_handle;

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    NSSymbol sym = NSLookupSymbolInImage(&__dso_handle, "_main", NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR);
    if ( sym == NULL ) {
        FAIL("can't find main");
    }
    void* mainAddr = NSAddressOfSymbol(sym);
    if ( mainAddr != &main ) {
        FAIL("address returned %p is not &main=%p", mainAddr, &main);
    }

    // verify NULL works
    if ( NSAddressOfSymbol(NULL) != NULL ) {
        FAIL("NULL not handle");
    }

    PASS("Success");
}

