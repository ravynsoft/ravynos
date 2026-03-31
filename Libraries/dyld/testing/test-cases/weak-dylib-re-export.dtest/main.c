

// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/libbar.dylib -install_name $RUN_DIR/libbar.dylib
// BUILD:  $CC foo.c -dynamiclib -L$BUILD_DIR -weak-lbar -Wl,-reexported_symbols_list,$SRC_DIR/symbols.txt -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dylib-re-export.exe $BUILD_DIR/libfoo.dylib -L$BUILD_DIR

// BUILD: $SKIP_INSTALL $BUILD_DIR/libbar.dylib

// RUN:  ./dylib-re-export.exe


#include <stdio.h>

#include "test_support.h"

__attribute__((weak_import))
extern int bar();

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    if ( &bar == 0 )
        PASS("SUCCESS");
    else
        FAIL("wrong value");

	return 0;
}


