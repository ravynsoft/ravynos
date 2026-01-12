/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFBundle_Internal.h
	Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFBUNDLE_INTERNAL__)
#define __COREFOUNDATION_CFBUNDLE_INTERNAL__ 1

#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFPlugIn.h>
#include <CoreFoundation/CFError.h>
#include "CFInternal.h"
#include "CFPlugIn_Factory.h"
#include "CFBundle_BinaryTypes.h"
#include "CFByteOrder.h"

CF_EXTERN_C_BEGIN

#define __kCFLogBundle       3
#define __kCFLogPlugIn       3

#if DEPLOYMENT_TARGET_WINDOWS
#define PLATFORM_PATH_STYLE kCFURLWindowsPathStyle
#else
#define PLATFORM_PATH_STYLE kCFURLPOSIXPathStyle
#endif

#define CFBundleExecutableNotFoundError             4
#define CFBundleExecutableNotLoadableError          3584
#define CFBundleExecutableArchitectureMismatchError 3585
#define CFBundleExecutableRuntimeMismatchError      3586
#define CFBundleExecutableLoadError                 3587
#define CFBundleExecutableLinkError                 3588

CF_INLINE uint32_t _CFBundleSwapInt32Conditional(uint32_t arg, Boolean swap) {return swap ? CFSwapInt32(arg) : arg;}
CF_INLINE uint32_t _CFBundleSwapInt64Conditional(uint64_t arg, Boolean swap) {return swap ? CFSwapInt64(arg) : arg;}

// uncomment this to enable the checking for 8302591
//#define CFBUNDLE_NO_TRAVERSE_OUTSIDE

typedef struct __CFResourceData {
    Boolean _executableLacksResourceFork;
    Boolean _infoDictionaryFromResourceFork;
    char _padding[2];
} _CFResourceData;

CF_PRIVATE _CFResourceData *__CFBundleGetResourceData(CFBundleRef bundle);

typedef struct __CFPlugInData {
    Boolean _isPlugIn;
    Boolean _loadOnDemand;
    Boolean _isDoingDynamicRegistration;
    Boolean _unused1;
    UInt32 _instanceCount;
    CFMutableArrayRef _factories;
} _CFPlugInData;

struct __CFBundle {
    CFRuntimeBase _base;
    
    CFURLRef _url;
    CFDateRef _modDate;
    
    __strong CFDictionaryRef _infoDict;
    __strong CFDictionaryRef _localInfoDict;
    CFArrayRef _searchLanguages;
    
    __CFPBinaryType _binaryType;
    Boolean _isLoaded;
    uint8_t _version;
    Boolean _sharesStringsFiles;
    char _padding[1];
    
    /* CFM goop */
    void *_connectionCookie;
    
    /* DYLD goop */
    const void *_imageCookie;
    const void *_moduleCookie;
    
    /* dlfcn goop */
    void *_handleCookie;
    
    /* CFM<->DYLD glue */
    CFMutableDictionaryRef _glueDict;
    
    /* Resource fork goop */
    _CFResourceData _resourceData;
    
    _CFPlugInData _plugInData;
    
    pthread_mutex_t _bundleLoadingLock;
    
    CFStringRef _executablePath; // Calculated and cached here
    CFStringRef _developmentRegion; // Calculated and cached here
    dispatch_once_t _developmentRegionCalculated;
    
    CFLock_t _lock;
    
    CFArrayRef _localizations; // List of localizations, including the development language fallback if required
    Boolean _lookedForLocalizations;
    
    CFMutableDictionaryRef _resourceDirectoryContents;
    
    CFMutableDictionaryRef _stringTable;
    
    CFLock_t _queryLock;
    CFMutableDictionaryRef _queryTable;
    CFStringRef _bundleBasePath;
    
#if defined(BINARY_SUPPORT_DLL)
    HMODULE _hModule;
#endif /* BINARY_SUPPORT_DLL */
    
};

extern _CFPlugInData *__CFBundleGetPlugInData(CFBundleRef bundle);

/* Private CFBundle API */

CF_PRIVATE CFErrorRef _CFBundleCreateErrorDebug(CFAllocatorRef allocator, CFBundleRef bundle, CFIndex code, CFStringRef debugString);

CF_PRIVATE void _CFBundleInfoPlistProcessInfoDictionary(CFMutableDictionaryRef dict);
CF_PRIVATE Boolean _CFBundleSupportedProductName(CFStringRef fileName, CFRange searchRange);
CF_PRIVATE Boolean _CFBundleSupportedPlatformName(CFStringRef fileName, CFRange searchRange);

CF_EXPORT CFStringRef _CFGetProductName(void);
CF_EXPORT CFStringRef _CFGetPlatformName(void);
CF_EXPORT CFStringRef _CFGetAlternatePlatformName(void);

CF_PRIVATE void _CFBundleFlushQueryTableCache(CFBundleRef bundle);

CF_PRIVATE SInt32 _CFBundleCurrentArchitecture(void);
CF_PRIVATE Boolean _CFBundleGetObjCImageInfo(CFBundleRef bundle, uint32_t *objcVersion, uint32_t *objcFlags);

#if defined(BINARY_SUPPORT_DYLD)
CF_PRIVATE CFMutableDictionaryRef _CFBundleCreateInfoDictFromMainExecutable();
CF_PRIVATE Boolean _CFBundleGrokObjCImageInfoFromMainExecutable(uint32_t *objcVersion, uint32_t *objcFlags);
#endif

CF_PRIVATE CFStringRef _CFBundleCopyLoadedImagePathForPointer(void *p);

// Languages and locales

CF_PRIVATE CFArrayRef _CFBundleCopyLanguageSearchListInDirectory(CFURLRef url, uint8_t *version);
CF_PRIVATE CFArrayRef _CFBundleCopyLanguageSearchListInBundle(CFBundleRef bundle);

CF_PRIVATE Boolean CFBundleAllowMixedLocalizations(void);

// Misc

extern Boolean _CFIsResourceAtURL(CFURLRef url, Boolean *isDir);
extern Boolean _CFIsResourceAtPath(CFStringRef path, Boolean *isDir);

CF_PRIVATE uint8_t _CFBundleGetBundleVersionForURL(CFURLRef url);
extern CFDictionaryRef _CFBundleCopyInfoDictionaryInDirectory(CFAllocatorRef alloc, CFURLRef url, UInt8 *version);
extern CFDictionaryRef _CFBundleCopyInfoDictionaryInDirectoryWithVersion(CFAllocatorRef alloc, CFURLRef url, UInt8 version);
extern CFURLRef _CFBundleCopySupportFilesDirectoryURLInDirectory(CFURLRef bundleURL, UInt8 version);
extern CFURLRef _CFBundleCopyResourcesDirectoryURLInDirectory(CFURLRef bundleURL, UInt8 version);

extern Boolean _CFBundleCouldBeBundle(CFURLRef url);
extern CFDictionaryRef _CFBundleCopyInfoDictionaryInResourceForkWithAllocator(CFAllocatorRef alloc, CFURLRef url);
CF_PRIVATE CFStringRef _CFBundleCopyExecutableName(CFBundleRef bundle, CFURLRef url, CFDictionaryRef infoDict);
#if DEPLOYMENT_TARGET_MACOSX
CF_PRIVATE CFStringRef _CFBundleCopyBundleDevelopmentRegionFromVersResource(CFBundleRef bundle);
#endif
extern CFDictionaryRef _CFBundleCopyInfoDictionaryInExecutable(CFURLRef url);
extern CFArrayRef _CFBundleCopyArchitecturesForExecutable(CFURLRef url);

extern CFStringRef _CFBundleGetPlatformExecutablesSubdirectoryName(void);
extern CFStringRef _CFBundleGetAlternatePlatformExecutablesSubdirectoryName(void);
extern CFStringRef _CFBundleGetOtherPlatformExecutablesSubdirectoryName(void);
extern CFStringRef _CFBundleGetOtherAlternatePlatformExecutablesSubdirectoryName(void);

extern CFStringRef _CFCreateStringFromVersionNumber(CFAllocatorRef alloc, UInt32 vers);
extern UInt32 _CFVersionNumberFromString(CFStringRef versStr);

extern void _CFBundleScheduleForUnloading(CFBundleRef bundle);
extern void _CFBundleUnscheduleForUnloading(CFBundleRef bundle);
extern void _CFBundleUnloadScheduledBundles(void);

CF_PRIVATE void _CFBundleAppendResourceDir(CFMutableStringRef path, uint8_t version);

CF_PRIVATE UInt8 _CFBundleLayoutVersion(CFBundleRef bundle);

#if defined(BINARY_SUPPORT_DYLD)
// DYLD API
extern __CFPBinaryType _CFBundleGrokBinaryType(CFURLRef executableURL);
extern CFArrayRef _CFBundleDYLDCopyLoadedImagePathsIfChanged(void);
extern CFArrayRef _CFBundleDYLDCopyLoadedImagePathsForHint(CFStringRef hint);
#if !defined(BINARY_SUPPORT_DLFCN)
extern Boolean _CFBundleDYLDCheckLoaded(CFBundleRef bundle);
extern Boolean _CFBundleDYLDLoadBundle(CFBundleRef bundle, Boolean forceGlobal, CFErrorRef *error);
extern Boolean _CFBundleDYLDLoadFramework(CFBundleRef bundle, CFErrorRef *error);
extern void _CFBundleDYLDUnloadBundle(CFBundleRef bundle);
extern void *_CFBundleDYLDGetSymbolByName(CFBundleRef bundle, CFStringRef symbolName);
#endif /* !BINARY_SUPPORT_DLFCN */
#endif /* BINARY_SUPPORT_DYLD */

#if defined(BINARY_SUPPORT_DLFCN)
// dlfcn API
extern Boolean _CFBundleDlfcnCheckLoaded(CFBundleRef bundle);
extern Boolean _CFBundleDlfcnPreflight(CFBundleRef bundle, CFErrorRef *error);
extern Boolean _CFBundleDlfcnLoadBundle(CFBundleRef bundle, Boolean forceGlobal, CFErrorRef *error);
extern Boolean _CFBundleDlfcnLoadFramework(CFBundleRef bundle, CFErrorRef *error);
extern void _CFBundleDlfcnUnload(CFBundleRef bundle);
extern void *_CFBundleDlfcnGetSymbolByName(CFBundleRef bundle, CFStringRef symbolName);
#endif /* BINARY_SUPPORT_DLFCN */

#if defined(BINARY_SUPPORT_DLL)
extern Boolean _CFBundleDLLLoad(CFBundleRef bundle, CFErrorRef *error);
extern void _CFBundleDLLUnload(CFBundleRef bundle);
extern void *_CFBundleDLLGetSymbolByName(CFBundleRef bundle, CFStringRef symbolName);
#endif /* BINARY_SUPPORT_DLL */


/* Private PlugIn-related CFBundle API */

extern Boolean _CFBundleNeedsInitPlugIn(CFBundleRef bundle);
extern void _CFBundleInitPlugIn(CFBundleRef bundle);
extern void _CFBundlePlugInLoaded(CFBundleRef bundle);
extern void _CFBundleDeallocatePlugIn(CFBundleRef bundle);

extern void _CFPlugInWillUnload(CFPlugInRef plugIn);

extern void _CFPlugInAddPlugInInstance(CFPlugInRef plugIn);
extern void _CFPlugInRemovePlugInInstance(CFPlugInRef plugIn);

extern void _CFPlugInAddFactory(CFPlugInRef plugIn, _CFPFactoryRef factory);
extern void _CFPlugInRemoveFactory(CFPlugInRef plugIn, _CFPFactoryRef factory);


/* Strings for parsing bundle structure */
#define _CFBundleSupportFilesDirectoryName1 CFSTR("Support Files")
#define _CFBundleSupportFilesDirectoryName2 CFSTR("Contents")
#define _CFBundleResourcesDirectoryName CFSTR("Resources")
#define _CFBundleExecutablesDirectoryName CFSTR("Executables")
#define _CFBundleNonLocalizedResourcesDirectoryName CFSTR("Non-localized Resources")

#if DEPLOYMENT_TARGET_WINDOWS
#define _CFBundleSupportFilesDirectoryName1WithResources CFSTR("Support Files\\Resources")
#define _CFBundleSupportFilesDirectoryName2WithResources CFSTR("Contents\\Resources")
#else
#define _CFBundleSupportFilesDirectoryName1WithResources CFSTR("Support Files/Resources")
#define _CFBundleSupportFilesDirectoryName2WithResources CFSTR("Contents/Resources")
#endif

#define _CFBundleSupportFilesURLFromBase1 CFSTR("Support%20Files/")
#define _CFBundleSupportFilesURLFromBase2 CFSTR("Contents/")
#define _CFBundleResourcesURLFromBase0 CFSTR("Resources/")
#define _CFBundleResourcesURLFromBase1 CFSTR("Support%20Files/Resources/")
#define _CFBundleResourcesURLFromBase2 CFSTR("Contents/Resources/")
#define _CFBundleAppStoreReceiptURLFromBase0 CFSTR("_MASReceipt/receipt")
#define _CFBundleAppStoreReceiptURLFromBase1 CFSTR("Support%20Files/_MASReceipt/receipt")
#define _CFBundleAppStoreReceiptURLFromBase2 CFSTR("Contents/_MASReceipt/receipt")
#define _CFBundleExecutablesURLFromBase1 CFSTR("Support%20Files/Executables/")
#define _CFBundleExecutablesURLFromBase2 CFSTR("Contents/")

#define _CFBundleInfoURLFromBase0 CFSTR("Resources/Info.plist")
#define _CFBundleInfoURLFromBase1 CFSTR("Support%20Files/Info.plist")
#define _CFBundleInfoURLFromBase2 CFSTR("Contents/Info.plist")
#define _CFBundleInfoURLFromBase3 CFSTR("Info.plist")
#define _CFBundleInfoURLFromBaseNoExtension3 CFSTR("Info")

#if DEPLOYMENT_TARGET_MACOSX
#define _CFBundlePlatformInfoURLFromBase0 CFSTR("Resources/Info-macos.plist")
#define _CFBundlePlatformInfoURLFromBase1 CFSTR("Support%20Files/Info-macos.plist")
#define _CFBundlePlatformInfoURLFromBase2 CFSTR("Contents/Info-macos.plist")
#define _CFBundlePlatformInfoURLFromBase3 CFSTR("Info-macos.plist")
#elif DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#define _CFBundlePlatformInfoURLFromBase0 CFSTR("Resources/Info-iphoneos.plist")
#define _CFBundlePlatformInfoURLFromBase1 CFSTR("Support%20Files/Info-iphoneos.plist")
#define _CFBundlePlatformInfoURLFromBase2 CFSTR("Contents/Info-iphoneos.plist")
#define _CFBundlePlatformInfoURLFromBase3 CFSTR("Info-iphoneos.plist")
#else
// No platform-specific variants in these cases
#define _CFBundlePlatformInfoURLFromBase0 _CFBundleInfoURLFromBase0
#define _CFBundlePlatformInfoURLFromBase1 _CFBundleInfoURLFromBase1
#define _CFBundlePlatformInfoURLFromBase2 _CFBundleInfoURLFromBase2
#define _CFBundlePlatformInfoURLFromBase3 _CFBundleInfoURLFromBase3
#endif

#define _CFBundleInfoPlistName CFSTR("Info.plist")

#if DEPLOYMENT_TARGET_MACOSX
#define _CFBundlePlatformInfoPlistName CFSTR("Info-macos.plist")
#elif DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#define _CFBundlePlatformInfoPlistName CFSTR("Info-iphoneos.plist")
#else
// No platform-specific Info.plist for these
#define _CFBundlePlatformInfoPlistName _CFBundleInfoPlistName
#endif

#define _CFBundleInfoExtension CFSTR("plist")
#define _CFBundleLocalInfoName CFSTR("InfoPlist")
#define _CFBundlePkgInfoURLFromBase1 CFSTR("Support%20Files/PkgInfo")
#define _CFBundlePkgInfoURLFromBase2 CFSTR("Contents/PkgInfo")
#define _CFBundlePseudoPkgInfoURLFromBase CFSTR("PkgInfo")
#define _CFBundlePrivateFrameworksURLFromBase0 CFSTR("Frameworks/")
#define _CFBundlePrivateFrameworksURLFromBase1 CFSTR("Support%20Files/Frameworks/")
#define _CFBundlePrivateFrameworksURLFromBase2 CFSTR("Contents/Frameworks/")
#define _CFBundleSharedFrameworksURLFromBase0 CFSTR("SharedFrameworks/")
#define _CFBundleSharedFrameworksURLFromBase1 CFSTR("Support%20Files/SharedFrameworks/")
#define _CFBundleSharedFrameworksURLFromBase2 CFSTR("Contents/SharedFrameworks/")
#define _CFBundleSharedSupportURLFromBase0 CFSTR("SharedSupport/")
#define _CFBundleSharedSupportURLFromBase1 CFSTR("Support%20Files/SharedSupport/")
#define _CFBundleSharedSupportURLFromBase2 CFSTR("Contents/SharedSupport/")
#define _CFBundleBuiltInPlugInsURLFromBase0 CFSTR("PlugIns/")
#define _CFBundleBuiltInPlugInsURLFromBase1 CFSTR("Support%20Files/PlugIns/")
#define _CFBundleBuiltInPlugInsURLFromBase2 CFSTR("Contents/PlugIns/")
#define _CFBundleAlternateBuiltInPlugInsURLFromBase0 CFSTR("Plug-ins/")
#define _CFBundleAlternateBuiltInPlugInsURLFromBase1 CFSTR("Support%20Files/Plug-ins/")
#define _CFBundleAlternateBuiltInPlugInsURLFromBase2 CFSTR("Contents/Plug-ins/")

#define _CFBundleLprojExtension CFSTR("lproj")
#define _CFBundleLprojExtensionWithDot CFSTR(".lproj")
#define _CFBundleDot CFSTR(".")
#define _CFBundleAllFiles CFSTR("_CFBAF_")
#define _CFBundleTypeIndicator CFSTR("_CFBT_")
// This directory contains resources (especially nibs) that may look up localized resources or may fall back to the development language resources
#define _CFBundleBaseDirectory CFSTR("Base")

#define _CFBundleMacOSXPlatformName CFSTR("macos")
#define _CFBundleAlternateMacOSXPlatformName CFSTR("macosx")
#define _CFBundleiPhoneOSPlatformName CFSTR("iphoneos")
#define _CFBundleMacOS8PlatformName CFSTR("macosclassic")
#define _CFBundleAlternateMacOS8PlatformName CFSTR("macos8")
#define _CFBundleWindowsPlatformName CFSTR("windows")
#define _CFBundleHPUXPlatformName CFSTR("hpux")
#define _CFBundleSolarisPlatformName CFSTR("solaris")
#define _CFBundleLinuxPlatformName CFSTR("linux")
#define _CFBundleFreeBSDPlatformName CFSTR("freebsd")

#define _CFBundleDefaultStringTableName CFSTR("Localizable")
#define _CFBundleStringTableType CFSTR("strings")
#define _CFBundleStringDictTableType CFSTR("stringsdict")

#define _CFBundleUserLanguagesPreferenceName CFSTR("AppleLanguages")
#define _CFBundleOldUserLanguagesPreferenceName CFSTR("NSLanguages")

#define _CFBundleLocalizedResourceForkFileName CFSTR("Localized")

#define _CFBundleWindowsResourceDirectoryExtension CFSTR("resources")

#define _CFBundleMacOSXInfoPlistPlatformName_OLD CFSTR("macos")
#define _CFBundleWindowsInfoPlistPlatformName_OLD CFSTR("win32")

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFBUNDLE_INTERNAL__ */

