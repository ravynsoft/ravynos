// BUILD:  $CC main.c -o $BUILD_DIR/dlsym-in-interposed-malloc.exe
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/dlsym-in-interposed-malloc.exe
// BUILD:  $CC interposer.c -dynamiclib -o $BUILD_DIR/libmyalloc.dylib -install_name libmyalloc.dylib

// RUN:    DYLD_INSERT_LIBRARIES=libmyalloc.dylib   ./dlsym-in-interposed-malloc.exe


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_support.h"

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    // malloc should have been called when dyld3's libdyld was initialized, but
    // call it one more time anyway just to make sure its working
    (void)malloc(1);
    
    PASS("Success");
}
