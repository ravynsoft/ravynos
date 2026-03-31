
// BUILD_ONLY: MacOSX
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_FORCE_PLATFORM.exe -DENABLE_ALT_PLATFORMS=1
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_FORCE_PLATFORM-fail.exe
// BUILD:  $TASK_FOR_PID_ENABLE  $BUILD_DIR/env-DYLD_FORCE_PLATFORM.exe
// BUILD:  $TASK_FOR_PID_ENABLE  $BUILD_DIR/env-DYLD_FORCE_PLATFORM-fail.exe

// RUN:  DYLD_FORCE_PLATFORM=6 ./env-DYLD_FORCE_PLATFORM.exe
// RUN:  DYLD_FORCE_PLATFORM=6 ./env-DYLD_FORCE_PLATFORM-fail.exe

#include <mach-o/dyld_priv.h>
#include <dlfcn.h>

#include "test_support.h"

#if ENABLE_ALT_PLATFORMS
__attribute__((section("__DATA,__allow_alt_plat"))) uint64_t dummy;

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    dyld_build_version_t ios12 = { .platform = PLATFORM_IOS, .version = 0x000c0000 };
    if (dyld_get_active_platform() != PLATFORM_MACCATALYST) { FAIL("dyld_get_active_platform() should return PLATFORM_MACCATALYST"); }
    // libswiftUIKit.dylib exists in /System/iOSSupport/usr/lib/swift
    // We should be able to dlopen it only if we are correctly prepending the /System/iOSSupport root path
#if 0
    // FIXME: We don't want to bring in such large dylib graphs. We can repurpose root testing support for this
    if (!dlopen_preflight("/usr/lib/swift/libswiftUIKit.dylib")) { FAIL("Should be able to dlopen libswiftUIKit but %s", dlerror()); }
#endif
    if (!dyld_program_minos_at_least(ios12)) { FAIL("DYLD_FORCE_PLATFORM should synthesize an iOS min version greater than 12.0"); }
    if (!dyld_program_sdk_at_least(ios12)) { FAIL("DYLD_FORCE_PLATFORM should synthesize an iOS sdk versio greater than 12.0"); }
    PASS("Success");
}
#else
int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    if (dyld_get_active_platform() != PLATFORM_MACOS) { FAIL("dyld_get_active_platform() should return PLATFORM_MACOS"); }

    // libswiftUIKit.dylib exists in /System/iOSSupport/usr/lib/swift
    // We should not be able to dlopen this as we don't expect to find it in a macOS location.  If it starts
    // being in a macOS location then we should update this test
    if(dlopen_preflight("/usr/lib/swift/libswiftUIKit.dylib")) { FAIL("Should not be able to dlopen libswiftUIKit"); }
    PASS("Success");
}
#endif
