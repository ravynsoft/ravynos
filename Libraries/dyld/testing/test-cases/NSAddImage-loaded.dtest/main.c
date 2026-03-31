// BUILD_ONLY: MacOSX

// BUILD:  $CC main.c            -o $BUILD_DIR/NSAddImage-loaded.exe -Wno-deprecated-declarations

// RUN:  ./NSAddImage-loaded.exe return



#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>

#include "test_support.h"

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    // verify value is returned for image already loaded
    const struct mach_header* mh = NSAddImage("/usr/lib/libSystem.B.dylib", NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED);
    if ( mh == NULL )
        FAIL("Could not find mh for libSystem.B.dylib");

    // verify existing dylib is not loaded if it is not already loaded
    mh = NSAddImage("/usr/lib/libz.dylib", NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED);
    if ( mh != NULL )
        FAIL("Found mh for unloaded dylib libz.dylib");

    PASS("Success");
}

