
// BUILD:  $CC foo.c  -dynamiclib -install_name $RUN_DIR/libterm.dylib  -o $BUILD_DIR/libterm.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlclose-term.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlclose-term.exe

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld_priv.h>

#include "test_support.h"

// verify dlclose() runs static terminator

typedef void (*NotifyProc)(void);

static bool termDidRun = false;

static void termNotifyFunc()
{
    termDidRun = true;
}

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    // load dylib
    void* handle = dlopen(RUN_DIR "/libterm.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        FAIL("libterm.dylib could not be loaded, %s", dlerror());
    }

    // stuff pointer to my notifier
    NotifyProc* pointerAddress = (NotifyProc*)dlsym(handle, "gNotifer");
    if ( pointerAddress == NULL ) {
        FAIL("gNotifer not found in libterm.dylib");
    }
    *pointerAddress = &termNotifyFunc;

    // unload dylib
    dlclose(handle);

    if ( termDidRun )
        PASS("Success");
    else
        FAIL("terminator not run");
}

