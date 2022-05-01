/*
 * ravynOS LaunchServices
 *
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <CoreFoundation/CFURL.h>
#import <CoreFoundation/CFArray.h>

#ifdef __OBJC__
#import <Foundation/NSObject.h>

@interface LaunchServices: NSObject
+database;
@end
#endif

typedef UInt32 OptionBits;
#define _Nullable

// OSStatus result codes
enum {
    kLSAppInTrashErr = -10660,
    kLSUnknownErr = -10810,
    kLSNotAnApplicationErr = -10811,
    kLSDataUnavailableErr = -10813,
    kLSApplicationNotFoundErr = -10814,
    kLSDataErr = -10817,
    kLSLaunchInProgressErr = -10818,
    kLSServerCommunicationErr = -10822,
    kLSCannotSetInfoErr = -10823,
    kLSIncompatibleSystemVersionErr = -10825,
    kLSNoLaunchPermissionErr = -10826,
    kLSNoExecutableErr = -10827,
    kLSMultipleSessionsNotSupportedErr = -10829,

    kLSUnknownType = 0,
    kLSUnknownCreator = 0,
    kLSInvalidExtensionIndex = 0xFFFFFFFF
};

typedef enum LSLaunchFlags : OptionBits {
    kLSLaunchDefaults = 0x00000001,
    kLSLaunchAndPrint = 0x00000002,
    kLSLaunchAndWaitForExit = 0x00000004, // specific to ravynOS
    kLSLaunchAndDisplayErrors = 0x00000040,
    kLSLaunchDontAddToRecents = 0x00000100,
    kLSLaunchDontSwitch = 0x00000200,
    kLSLaunchAsync = 0x00010000,
    kLSLaunchNewInstance = 0x00080000,
    kLSLaunchAndHide = 0x00100000,
    kLSLaunchAndHideOthers = 0x00200000,
    kLSALaunchTaskEnvIsValid = 0x10000000,
    kLSALaunchTaskArgsIsValid = 0x20000000
} LSLaunchFlags;

typedef enum LSAcceptanceFlags : OptionBits {
    kLSAcceptDefault = 0x00000001,
    kLSAcceptAllowLoginUI = 0x00000002
} LSAcceptanceFlags;

// LSItemInfoFlags are deprecated - not implemented
// LSHandlerOptions are deprecated - not implemented
// LSRequestedInfo are deprecated - not implemented

typedef enum LSRolesMask : OptionBits {
    kLSRolesNone = 0x00000001,
    kLSRolesViewer = 0x00000002,
    kLSRolesEditor = 0x00000004,
    kLSRolesShell = 0x00000008,
    kLSRolesAll = (UInt32)0xFFFFFFFF
} LSRolesMask;

typedef struct LSLaunchURLSpec {
    CFURLRef appURL;
    void *asyncRefCon;
    CFArrayRef itemURLs;
    LSLaunchFlags launchFlags;
    const void *passThruParams; // ignored - not implemented
    CFArrayRef taskArgs;        // specific to ravynOS
    CFDictionaryRef taskEnv;    // specific to ravynOS
} LSLaunchURLSpec;

const CFStringRef    kCFBundleTypeExtensionsKey = CFSTR("CFBundleTypeExtensions");
const CFStringRef    kCFBundleURLSchemesKey = CFSTR("CFBundleURLSchemes");
const CFStringRef    kCFBundleTypeRoleKey = CFSTR("CFBundleTypeRole");
const CFStringRef    kLSItemContentTypesKey = CFSTR("LSItemContentTypes");

enum {
    kLSRankOwner = 0,
    kLSRankDefault = 1,
    kLSRankAlternate = 2
};

typedef struct OpaqueLSSharedFileListRef *LSSharedFileListRef;
typedef struct OpaqueLSSharedFileListItemRef *LSSharedFileListItemRef;

#ifdef __cplusplus
extern "C" {
#endif
void LSRevealInFiler(CFArrayRef inItemURLs);

//------------------------------------------------------------------------
// These two functions are used by Filer but are otherwise internal only
//------------------------------------------------------------------------

Boolean LSIsNSBundle(CFURLRef url);
Boolean LSIsAppDir(CFURLRef url);

//------------------------------------------------------------------------
//    PUBLIC API
//------------------------------------------------------------------------

OSStatus LSOpenCFURLRef(CFURLRef inURL, CFURLRef _Nullable *outLaunchedURL);
OSStatus LSOpenFromURLSpec(const LSLaunchURLSpec *inLaunchSpec, CFURLRef _Nullable *outLaunchedURL);
OSStatus LSRegisterURL(CFURLRef inURL, Boolean inUpdate);
OSStatus LSCanURLAcceptURL(CFURLRef inItemURL, CFURLRef inTargetURL, LSRolesMask inRoleMask, LSAcceptanceFlags inFlags, Boolean *outAcceptsItem);

#ifdef __cplusplus
}
#endif
