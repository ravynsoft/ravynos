// BUILD_ONLY: MacOSX

// BUILD:  $CC zzz.c -dynamiclib -o $BUILD_DIR/libzzz.dylib -install_name $RUN_DIR/libzzz.dylib
// BUILD:  $CC main.c            -o $BUILD_DIR/NSAddImage-basic.exe -Wno-deprecated-declarations

// RUN:  ./NSAddImage-basic.exe $RUN_DIR/libzzz.dylib
// RUN:  ./NSAddImage-basic.exe libzzz.dylib


#include <stdio.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>

#include "test_support.h"

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    const char* path = argv[1];

	const struct mach_header* mh = NSAddImage(path, NSADDIMAGE_OPTION_WITH_SEARCHING);
	if ( mh == NULL )
        FAIL("Could not load \"%s\"", path);
	else
        PASS("Success");
}

