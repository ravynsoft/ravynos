/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-*
 *
 * Copyright (c) 2018 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifdef __linux__
#include <algorithm>
#include <string.h>
namespace {
	typedef long double max_align_t;
}
#endif

#include <mach-o/loader.h>
#include "PlatformSupport.h"

//FIXME: Only needed until we move VersionSet into PlatformSupport
#include "ld.hpp"
#include "Options.h"

namespace ld {

const Platform basePlatform(Platform platform)
{
    switch(platform) {
        case ld::Platform::iOSMac:
        case ld::Platform::iOS_simulator:
            return ld::Platform::iOS;

        case ld::Platform::watchOS_simulator:
            return Platform::watchOS;

        case ld::Platform::tvOS_simulator:
            return ld::Platform::tvOS;

        default:
            return platform;
    }
}

static const PlatformInfo sAllSupportedPlatforms[] = {
    { Platform::unknown,           Platform::unknown,      "unknown",           "",                                NULL,                        0x00000000, 0x00000000, 0,                       false, true,  PlatEnforce::error,                     PlatEnforce::error },
    { Platform::macOS,             Platform::macOS,        "macOS",             "-macos_version_min",             "MACOSX_DEPLOYMENT_TARGET",   0x000A0E00, 0x000A0800, LC_VERSION_MIN_MACOSX,   false, true,  PlatEnforce::warnInternalErrorExternal, PlatEnforce::warning },
    { Platform::iOS,               Platform::iOS,          "iOS",               "-ios_version_min",               "IPHONEOS_DEPLOYMENT_TARGET", 0x000C0000, 0x00070000, LC_VERSION_MIN_IPHONEOS, true,  true,  PlatEnforce::warnInternalErrorExternal, PlatEnforce::warning },
    { Platform::tvOS,              Platform::tvOS,         "tvOS",              "-tvos_version_min",              "TVOS_DEPLOYMENT_TARGET",     0x000C0000, 0x00070000, LC_VERSION_MIN_TVOS,     true,  true,  PlatEnforce::warnInternalErrorExternal, PlatEnforce::warning },
    { Platform::watchOS,           Platform::watchOS,      "watchOS",           "-watchos_version_min",           "WATCHOS_DEPLOYMENT_TARGET",  0x00050000, 0x00020000, LC_VERSION_MIN_WATCHOS,  true,  true,  PlatEnforce::warnInternalErrorExternal, PlatEnforce::warning },
    { Platform::bridgeOS,          Platform::bridgeOS,     "bridgeOS",          "-bridgeos_version_min",          "BRIDGEOS_DEPLOYMENT_TARGET", 0x00010000, 0x00010000, 0,                       false, true,  PlatEnforce::warnInternalErrorExternal, PlatEnforce::warning },
    { Platform::iOSMac,            Platform::iOSMac,       "Mac Catalyst",      "-maccatalyst_version_min",       NULL,                         0x000D0000, 0x000D0000, 0,                       false, true,  PlatEnforce::warnInternalErrorExternal, PlatEnforce::warnInternalErrorExternal },
    { Platform::iOS_simulator,     Platform::iOS,          "iOS Simulator",     "-ios_simulator_version_min",     NULL,                         0x000D0000, 0x00080000, LC_VERSION_MIN_IPHONEOS, false, true,  PlatEnforce::warnInternalErrorExternal, PlatEnforce::error },
    { Platform::tvOS_simulator,    Platform::tvOS,         "tvOS Simulator",    "-tvos_simulator_version_min",    NULL,                         0x000D0000, 0x00080000, LC_VERSION_MIN_TVOS,     false, true,  PlatEnforce::warnInternalErrorExternal, PlatEnforce::error },
    { Platform::watchOS_simulator, Platform::watchOS,      "watchOS Simulator", "-watchos_simulator_version_min", NULL,                         0x00060000, 0x00020000, LC_VERSION_MIN_WATCHOS,  false, true,  PlatEnforce::warnInternalErrorExternal, PlatEnforce::error },
    { Platform::driverKit,         Platform::driverKit,    "DriverKit",         "-driverkit_version_min",         NULL,                         0x00130000, 0x00130000, 0,                       false, true,  PlatEnforce::error,                     PlatEnforce::error },

    { Platform::freestanding,     Platform::freestanding,  "free standing",     "-preload",                       NULL,                         0x00000000, 0,          0,                       false, false, PlatEnforce::allow,                     PlatEnforce::allow   },
};

static void versionToString(uint32_t value, char buffer[32])
{
    if ( value & 0xFF )
        sprintf(buffer, "%d.%d.%d", value >> 16, (value >> 8) & 0xFF, value & 0xFF);
    else
        sprintf(buffer, "%d.%d", value >> 16, (value >> 8) & 0xFF);
}

void VersionSet::checkObjectCrosslink(const VersionSet& objectPlatforms, const std::string& targetPath, bool internalSDK,
                                      bool bitcode, bool platformMismatchesAreWarning) const {
    // Check platform cross-linking.
    __block bool warned = false;
    forEach(^(ld::Platform cmdLinePlatform, uint32_t cmdLineMinVersion, uint32_t cmdLineSDKVersion, bool& stop) {
        // <rdar://50990574> if .o file is old and has no platform specified, don't complain (unless building for iOSMac)
        if ( objectPlatforms.empty() && !contains(ld::Platform::iOSMac))
            return;

        if ( objectPlatforms.contains(cmdLinePlatform) ) {
            uint32_t objMinOS = objectPlatforms.minOS(cmdLinePlatform);
            if ( (cmdLineMinVersion != 0) && (objMinOS > cmdLineMinVersion) ) {
                char t1[32];
                char t2[32];
                versionToString(objMinOS, t1);
                versionToString(cmdLineMinVersion, t2);
                warning("object file (%s) was built for newer %s version (%s) than being linked (%s)",
                        targetPath.c_str(), nameFromPlatform(cmdLinePlatform), t1, t2);
            }
        }
        else {
            if (bitcode) {
                throwf("building for %s, but linking in object file (%s) built for %s,",
                       to_str().c_str(), targetPath.c_str(), objectPlatforms.to_str().c_str());
            } else if ( (count() == 2) && (cmdLinePlatform == ld::Platform::iOSMac) )  {
                // clang is not emitting second LC_BUILD_VERSION in zippered .o files
            }
            else {
                auto enforce = platformInfo(cmdLinePlatform)._linkingObjectFiles;
                if (enforce == PlatEnforce::warnInternalErrorExternal) {
                    enforce = (internalSDK ? PlatEnforce::warning : PlatEnforce::error);
                }
                if ( platformMismatchesAreWarning && (enforce == PlatEnforce::error) )
                    enforce = PlatEnforce::warning;
                switch (enforce) {
                    case PlatEnforce::allow:
                        break;
                    case PlatEnforce::warnBI:
                        // only warn during B&I builds
                        if ( (getenv("RC_XBS") != NULL) && (getenv("RC_BUILDIT") == NULL) )
                            break;
                    case PlatEnforce::warning: {
                        if ( !warned ) {
                            warning("building for %s, but linking in object file (%s) built for %s",
                                    to_str().c_str(), targetPath.c_str(), objectPlatforms.to_str().c_str());
                            warned = true;
                        }
                    } break;
                    case PlatEnforce::error:
                        throwf("building for %s, but linking in object file built for %s,",
                               to_str().c_str(), objectPlatforms.to_str().c_str());
                        break;
                    case PlatEnforce::warnInternalErrorExternal:
                        assert(0);
                        break;
                }
            }
        }
    });
}

static bool startsWith(const char* str, const char* prefix)
{
    if ( str == nullptr )
        return false;
    return (strncmp(str, prefix, strlen(prefix)) == 0);
}

void VersionSet::checkDylibCrosslink(const VersionSet& dylibPlatforms, const std::string& targetPath,
                                     const std::string& dylibType, bool internalSDK, bool indirectDylib,
                                     bool bitcode, bool isUnzipperedTwin, const char* installName, bool fromSDK,
                                     bool platformMismatchesAreWarning) const {
    // Check platform cross-linking.
    __block bool warned = false;
    forEach(^(ld::Platform cmdLinePlatform, uint32_t cmdLineMinVersion, uint32_t cmdLineSDKVersion, bool& stop) {
        // <rdar://51768462> if dylib is old and has no platform specified, don't complain (unless building for iOSMac)
        if ( dylibPlatforms.empty() && !contains(ld::Platform::iOSMac))
            return;
        if ( dylibPlatforms.contains(cmdLinePlatform) ) {
            // <rdar://problem/53510264> check any non-OS dylibs do not have a newer min OS
            if ( !fromSDK && !startsWith(installName, "/usr/lib") && !startsWith(installName, "/System/Library/") ) {
                uint32_t dylibMinOS = dylibPlatforms.minOS(cmdLinePlatform);
                if ( (cmdLineMinVersion != 0) && (dylibMinOS > cmdLineMinVersion) ) {
                    char t1[32];
                    char t2[32];
                    versionToString(dylibMinOS, t1);
                    versionToString(cmdLineMinVersion, t2);
                    warning("dylib (%s) was built for newer %s version (%s) than being linked (%s)",
                            targetPath.c_str(), nameFromPlatform(cmdLinePlatform), t1, t2);
                }
            }
        }
        else {
            // Normally linked dylibs need to support all the platforms we are building for,
            // with the exception of zippered binaries linking to macOS only libraries in the OS.
            // To handle that case we exit early when the commandline platform is iOSMac, if
            // the platforms also contain macOS.
            if (cmdLinePlatform == ld::Platform::iOSMac && contains(ld::Platform::macOS) && !isUnzipperedTwin)
                return;
            // <rdar://problem/48416765> spurious warnings when iOSMac binary is built links with zippered dylib that links with macOS dylib
            // <rdar://problem/50517845> iOSMac app links simulator frameworks without warning/error
            if  ( indirectDylib && dylibPlatforms.contains(ld::Platform::macOS) && contains(ld::Platform::iOSMac) )
                return;
            // <rdar://problem/61607340> simulators can link with special three libsytem_* dylibs
            if ( this->contains(ld::simulatorPlatforms) && dylibPlatforms.contains(ld::Platform::macOS) && (installName != nullptr) ) {
                if ( strcmp(installName, "/usr/lib/system/libsystem_kernel.dylib") == 0 )
                    return;
                if ( strcmp(installName, "/usr/lib/system/libsystem_platform.dylib") == 0 )
                    return;
                if ( strcmp(installName, "/usr/lib/system/libsystem_pthread.dylib") == 0 )
                    return;
            }
            if ( bitcode ) {
                throwf("building for %s, but linking in %s file (%s) built for %s,",
                       to_str().c_str(),  dylibType.c_str(), targetPath.c_str(), dylibPlatforms.to_str().c_str());
            }
            else {
                auto enforce = platformInfo(cmdLinePlatform)._linkingDylibs;
                if (enforce == PlatEnforce::warnInternalErrorExternal) {
                    enforce = (internalSDK ? PlatEnforce::warning : PlatEnforce::error);
                }
                if ( platformMismatchesAreWarning && (enforce == PlatEnforce::error) )
                    enforce = PlatEnforce::warning;
                switch (enforce) {
                    case PlatEnforce::allow:
                        break;
                    case PlatEnforce::warnBI:
                        // only warn during B&I builds
                        if ( (getenv("RC_XBS") != NULL) && (getenv("RC_BUILDIT") == NULL) )
                            break;
                        [[clang::fallthrough]];
                    case PlatEnforce::warning:
                        if ( !warned ) {
                            if ( isUnzipperedTwin && (this->count() == 2) )
                                warning("building zippered for %s, but linking in unzippered twin %s file (%s) built for %s",
                                    to_str().c_str(),  dylibType.c_str(), targetPath.c_str(), dylibPlatforms.to_str().c_str());
                            else
                                warning("building for %s, but linking in %s file (%s) built for %s",
                                    to_str().c_str(),  dylibType.c_str(), targetPath.c_str(), dylibPlatforms.to_str().c_str());
                            warned = true;
                        }
                        break;
                    case PlatEnforce::error:
                        if ( isUnzipperedTwin && (this->count() == 2) )
                            throwf("building zippered for %s, but linking in in unzippered twin %s built for %s,",
                                    to_str().c_str(),  dylibType.c_str(),  dylibPlatforms.to_str().c_str());
                        else
                            throwf("building for %s, but linking in %s built for %s,",
                                    to_str().c_str(),  dylibType.c_str(),  dylibPlatforms.to_str().c_str());
                        break;
                    case PlatEnforce::warnInternalErrorExternal:
                        assert(0);
                        break;
                }
            }
        }
    });
}

const PlatformInfo& platformInfo(Platform platform)
{
    for (const PlatformInfo& info : sAllSupportedPlatforms) {
        if ( info.platform == platform )
            return info;
    }
    return sAllSupportedPlatforms[0];
}

void forEachSupportedPlatform(void (^handler)(const PlatformInfo& info, bool& stop))
{
    bool stop = false;
    for (const PlatformInfo& info : sAllSupportedPlatforms) {
        handler(info, stop);
        if ( stop )
            break;
    }
}

Platform platformForLoadCommand(uint32_t lc, const mach_header* mh)
{
    Platform result = Platform::unknown;
    bool isIntel = ((mh->cputype & ~CPU_ARCH_MASK) == CPU_TYPE_I386);
    for (const PlatformInfo& info : sAllSupportedPlatforms) {
        if ( info.loadCommandIfNotUsingBuildVersionLC == lc ) {
            // some LC_ are in multiple rows, disabiguate by only setting sim row on intel
            bool isSim = (info.platform != info.basePlatform);
            if ( !isSim || isIntel  )
                result = info.platform;
        }
    }
    return result;
}

// clang puts PLATFORM_IOS into sim .o files instead of PLATFORM_IOSSIMULATOR
Platform platformFromBuildVersion(uint32_t plat, const mach_header* mh)
{
    bool isIntel = ((mh->cputype & ~CPU_ARCH_MASK) == CPU_TYPE_I386);
    if ( (Platform)plat == Platform::iOS ) {
        if (isIntel)
            return Platform::iOS_simulator;
    }
    else if ( (Platform)plat == Platform::tvOS ) {
        if (isIntel)
            return Platform::tvOS_simulator;
    }
    else if ( (Platform)plat == Platform::watchOS ) {
        if (isIntel)
            return Platform::watchOS_simulator;
    }
    return (Platform)plat;
}

Platform platformFromName(const char* platformName)
{
    auto normalizedStringComp = [](char c1, char c2) {
        return (c1 == c2 || std::toupper(c1) == std::toupper(c2)
                || (c1 == ' ' && c2 == '-') || (c1 == '-' && c2 == ' '));
    };
    // check if this is a platform name in our table
    const size_t platformNameLen = strlen(platformName);
    for (const PlatformInfo& info : sAllSupportedPlatforms) {
		if ( platformNameLen != strlen(info.printName) )
            continue;
		if ( std::equal(platformName, &platformName[platformNameLen], &info.printName[0], normalizedStringComp) )
            return info.platform;
	}
    // check if this is a raw platform number
	char *end = nullptr;
	long num = strtol(platformName, &end, 10);
    if ( (end != nullptr) && (*end == '\0') )
        return (Platform)num;

    return Platform::unknown;
}

const char* nameFromPlatform(Platform platform)
{
    for (const PlatformInfo& info : sAllSupportedPlatforms) {
        if ( info.platform == platform )
            return info.printName;
    }
    static char strBuffer[32];
    sprintf(strBuffer, "platform=%u", (unsigned)platform);
    return strBuffer;
}

} // namespace
