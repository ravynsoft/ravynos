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

#ifndef PlatformSupport_h
#define PlatformSupport_h

#include <mach-o/loader.h>

#include <set>

namespace ld {


enum class Platform
{
    unknown             = 0,
    macOS               = 1,    // PLATFORM_MACOS
    iOS                 = 2,    // PLATFORM_IOS
    tvOS                = 3,    // PLATFORM_TVOS
    watchOS             = 4,    // PLATFORM_WATCHOS
    bridgeOS            = 5,    // PLATFORM_BRIDGEOS
    iOSMac              = 6,    // PLATFORM_MACCATALYST
    iOS_simulator       = 7,    // PLATFORM_IOSSIMULATOR
    tvOS_simulator      = 8,    // PLATFORM_TVOSSIMULATOR
    watchOS_simulator   = 9,    // PLATFORM_WATCHOSSIMULATOR
    driverKit           = 10,   // PLATFORM_DRIVERKIT

    freestanding        = 100   // this never shows up in mach-o files, it is for internal tracking in ld64
};


const Platform basePlatform(Platform platform);

typedef std::set<Platform> PlatformSet;

enum class PlatEnforce { allow, warning, error, warnBI, warnInternalErrorExternal };

struct PlatformInfo
{
    Platform            platform;
    Platform            basePlatform;
    const char*         printName;
    const char*         commandLineArg;
    const char*         fallbackEnvVarName;
    uint32_t            firstOsVersionUsingBuildVersionLC;
    uint32_t            minimumOsVersion;
    uint32_t            loadCommandIfNotUsingBuildVersionLC;
    bool                supportsEmbeddedBitcode;
    bool                warnIfObjectFileTooNew;
    // DO NOT ACCESS DIRECTLY
    // Use checkObjectCrosslink() / checkDylibCrosslink()
    PlatEnforce         _linkingObjectFiles;
    PlatEnforce         _linkingDylibs;
};

const PlatformInfo& platformInfo(Platform platform);

void forEachSupportedPlatform(void (^handler)(const PlatformInfo& info, bool& stop));

Platform      platformForLoadCommand(uint32_t lc, const mach_header* mh);
Platform      platformFromBuildVersion(uint32_t plat, const mach_header* mh);
Platform      platformFromName(const char* platformName);
const char*   nameFromPlatform(Platform plat);


} // namespace

#endif // PlatformSupport_h
