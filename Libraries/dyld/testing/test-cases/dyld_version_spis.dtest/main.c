
// BUILD:  $CC main.c  -o $BUILD_DIR/versions.exe

// RUN:  ./versions.exe

#include <stdio.h>
#include <string.h>
#include <mach-o/dyld_priv.h>

#include "test_support.h"

extern struct mach_header __dso_handle;

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    dyld_platform_t active = dyld_get_active_platform();
    dyld_platform_t base = dyld_get_base_platform(active);
    dyld_build_version_t absoluteMin = { .platform = base, .version = 0 };
    dyld_build_version_t absoluteMax = { .platform = base, .version = 0xffffffff };

#if TARGET_OS_OSX
    if ( base != PLATFORM_MACOS ) {
        FAIL("base.platform %u incorrect for macOS", base);
    }
#elif TARGET_OS_IOS
    if ( base != PLATFORM_IOS ) {
        FAIL("base.platform %u incorrect for iOS", base);
    }
#elif TARGET_OS_TV
    if ( base != PLATFORM_TVOS ) {
        FAIL("base.platform %u incorrect for tvOS", base);
    }
#elif TARGET_OS_BRIDGE
    if ( base != PLATFORM_BRIDGEOS ) {
        FAIL("base.platform %u incorrect for wacthOS", base);
    }
#elif TARGET_OS_WATCH
    if ( base != PLATFORM_WATCHOS ) {
        FAIL("base.platform %u incorrect for bridgeOn", base);
    }
#else
    FAIL("Running on unknown platform");
#endif

#if TARGET_OS_SIMULATOR
    if (dyld_is_simulator_platform(active) != true) {
        FAIL("active platform %u should be a simulator", active);
    }
#else
    if (dyld_is_simulator_platform(active) == true) {
        FAIL("active platform %u should not be a simulator", active);
    }
#endif

    if (dyld_is_simulator_platform(base) == true) {
        FAIL("base platform %u should not be a simulator", base);
    }

    if (!dyld_sdk_at_least(&__dso_handle, absoluteMin)) {
        FAIL("executable sdk version should not < 1.0.0");
    }

    if (dyld_sdk_at_least(&__dso_handle, absoluteMax)) {
        FAIL("executable sdk version should not > 65536.0.0");
    }

    if (!dyld_minos_at_least(&__dso_handle, absoluteMin)) {
        FAIL("executable min version should not < 1.0.0");
    }

    if (dyld_minos_at_least(&__dso_handle, absoluteMax)) {
        FAIL("executable min version should not > 65536.0.0");
    }

    PASS("Success");
}

