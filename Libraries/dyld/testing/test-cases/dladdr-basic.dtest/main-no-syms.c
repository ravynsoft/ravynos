
// BUILD:  $CC main-no-syms.c            -o $BUILD_DIR/dladdr-stripped.exe
// BUILD:  $STRIP $BUILD_DIR/dladdr-stripped.exe

// RUN:  ./dladdr-stripped.exe


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld_priv.h>

#include "test_support.h"

///
/// verify dladdr() returns NULL for a symbol name in a fully stripped 
/// main executable (and not _mh_execute_header+nnn).
///

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    Dl_info info;
    if ( dladdr(&main, &info) == 0 ) {
        FAIL("dladdr(&main, xx) failed");
    }

    if ( info.dli_sname != NULL ){
        FAIL("%s\" instead of NULL", info.dli_sname);
    }

    PASS("Succes");
}
