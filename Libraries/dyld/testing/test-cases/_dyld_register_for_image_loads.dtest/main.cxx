
// BUILD:  $CC  foo.c -dynamiclib  -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CXX main.cxx -o $BUILD_DIR/dyld_register_test.exe $BUILD_DIR/libfoo.dylib -DRUN_DIR="$RUN_DIR"
// BUILD:  $CC  foo.c -dynamiclib  -install_name $RUN_DIR/libfoo2.dylib -o $BUILD_DIR/libfoo2.dylib
// BUILD:  $CC  foo.c -bundle -o $BUILD_DIR/foo.bundle
// BUILD:  $CC  up.c -dynamiclib  -install_name $RUN_DIR/libup.dylib -o $BUILD_DIR/libup.dylib
// BUILD:  $CC  baz.c -dynamiclib  -install_name $RUN_DIR/libbaz.dylib -o $BUILD_DIR/libbaz.dylib $BUILD_DIR/libup.dylib
// BUILD:  $CC  bar.c -dynamiclib  -install_name $RUN_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib -Wl,-upward_library,$BUILD_DIR/libup.dylib -DRUN_DIR="$RUN_DIR"

// RUN:  ./dyld_register_test.exe

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_priv.h>

#include <unordered_set>

#include "test_support.h"

extern "C" void foo();

extern mach_header __dso_handle;

static std::unordered_set<const mach_header*> sCurrentImages;
static bool expectedUnloadableState = false;

static void notify(const mach_header* mh, const char* path, bool unloadable)
{
    LOG("mh=%p, path=%s, unloadable=%d", mh, path, unloadable);
    if ( sCurrentImages.count(mh) != 0 ) {
        FAIL("notified twice about %p", mh);
    }
    sCurrentImages.insert(mh);

    const char* leaf = strrchr(path, '/');
    if ( unloadable != expectedUnloadableState ) {
        FAIL("image incorrectly marked unloadable(%s) but expected unloadable(%s) %p %s",
               unloadable ? "true" : "true", expectedUnloadableState ? "true" : "false", mh, path);
    }
}


int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    // Initially all images must not be unloadable as they are directly linked to the main executable
    expectedUnloadableState = false;
    _dyld_register_for_image_loads(&notify);

    // verify we were notified about already loaded images
    if ( sCurrentImages.count(&__dso_handle) == 0 ) {
        FAIL("did not notify us about main executable");
    }
    const mach_header* libSysMH = dyld_image_header_containing_address((void*)&printf);
    if ( sCurrentImages.count(libSysMH) == 0 ) {
        FAIL("did not notify us about libsystem_c.dylib");
    }
    const mach_header* libFoo = dyld_image_header_containing_address((void*)&foo);
    if ( sCurrentImages.count(libFoo) == 0 ) {
        FAIL("did not notify us about libfoo.dylib");
    }
    
    // These dlopen's can be unloaded
    expectedUnloadableState = true;

    // verify we were notified about load of libfoo2.dylib
    void* handle2 = dlopen(RUN_DIR "/libfoo2.dylib", RTLD_FIRST);
    if ( handle2 == NULL ) {
        FAIL("dlopen(\"%s\") failed with: %s", RUN_DIR "/libfoo.dylib", dlerror());
    }
    const void* libfoo2Foo = dlsym(handle2, "foo");
    const mach_header* libfoo2MH = dyld_image_header_containing_address(libfoo2Foo);
    if ( sCurrentImages.count(libfoo2MH) == 0 ) {
        FAIL("did not notify us about libfoo2.dylib");
    }

    // verify we were notified about load of foo.bundle
    void* handleB = dlopen(RUN_DIR "/foo.bundle", RTLD_FIRST);
    if ( handleB == NULL ) {
        FAIL("dlopen(\"%s\") failed with: %s", RUN_DIR "/foo.bundle", dlerror());
    }
    const void* libfooBFoo = dlsym(handle2, "foo");
    const mach_header* libfooB = dyld_image_header_containing_address(libfooBFoo);
    if ( sCurrentImages.count(libfooB) == 0 ) {
        FAIL("_dyld_register_for_image_loads() did not notify us about foo.bundle");
    }

    // Upward linking with dlopen may confuse the notifier as we may try to notify twice on the upwardly linked image
    // We test this with:
    //   libbar upward links libup
    //   libbar also dlopens libbaz
    //   libbaz links libup
    // We should not get a duplicate notification on libup
    void* handleBar = dlopen(RUN_DIR "/libbar.dylib", RTLD_FIRST);
    if ( handleBar == NULL ) {
        FAIL("dlopen(\"%s\") failed with: %s", RUN_DIR "/libbar.dylib", dlerror());
    }

    PASS("Success");
}

