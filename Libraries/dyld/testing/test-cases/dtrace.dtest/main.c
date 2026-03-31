// BUILD_ONLY: MacOSX

// if we ever re-enable this on iOS we will need to add // BOOT_ARGS: dtrace_dof_mode=1

// BUILD:  $DTRACE -h -s main.d -o $BUILD_DIR/probes.h
// BUILD:  $CC main.c -I$BUILD_DIR -o $BUILD_DIR/dtrace.exe $DEPENDS_ON $BUILD_DIR/probes.h
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/dtrace.exe

// RUN:    $SUDO /usr/sbin/dtrace -o /dev/null 2> /dev/null -n 'dyld_testing*:dtrace.exe:main:callback' -c $RUN_DIR/dtrace.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sdt.h>

#include "test_support.h"

#include "probes.h"


int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    DYLD_TESTING_CALLBACK();

    if (!DYLD_TESTING_CALLBACK_ENABLED())
        FAIL("DYLD_TESTING_CALLBACK_ENABLED() returned false");
    else
        PASS("Success");
}
