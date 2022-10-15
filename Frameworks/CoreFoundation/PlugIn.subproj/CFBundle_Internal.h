/*	CFBundle_Internal.h
	Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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
#include <CoreFoundation/CFByteOrder.h>

CF_EXTERN_C_BEGIN

#define __kCFLogBundle       3

#if TARGET_OS_WIN32
#define PLATFORM_PATH_STYLE kCFURLWindowsPathStyle
#else
#define PLATFORM_PATH_STYLE kCFURLPOSIXPathStyle
#endif

// FHS bundles are supported on the Swift and C runtimes, except on Windows.
#if !DEPLOYMENT_RUNTIME_OBJC && !TARGET_OS_WIN32

#if TARGET_OS_LINUX || TARGET_OS_BSD || TARGET_OS_ANDROID
#define _CFBundleFHSSharedLibraryFilenamePrefix CFSTR("lib")
#define _CFBundleFHSSharedLibraryFilenameSuffix CFSTR(".so")
#elif TARGET_OS_MAC
#define _CFBundleFHSSharedLibraryFilenamePrefix CFSTR("lib")
#define _CFBundleFHSSharedLibraryFilenameSuffix CFSTR(".dylib")
#else // a non-covered DEPLOYMENT_TARGET…
#error Disable FHS bundles or specify shared library prefixes and suffixes for this platform.
#endif // DEPLOYMENT_TARGET_…

#endif // !DEPLOYMENT_RUNTIME_OBJC && !TARGET_OS_WIN32

#define CFBundleExecutableNotFoundError             4
#define CFBundleExecutableNotLoadableError          3584
#define CFBundleExecutableArchitectureMismatchError 3585
#define CFBundleExecutableRuntimeMismatchError      3586
#define CFBundleExecutableLoadError                 3587
#define CFBundleExecutableLinkError                 3588

CF_PRIVATE char *__CFBundleMainID;


CF_INLINE uint32_t _CFBundleSwapInt32Conditional(uint32_t arg, Boolean swap) {return swap ? CFSwapInt32(arg) : arg;}
CF_INLINE uint32_t _CFBundleSwapInt64Conditional(uint64_t arg, Boolean swap) {return swap ? CFSwapInt64(arg) : arg;}

typedef struct __CFResourceData {
    Boolean _executableLacksResourceFork;
    Boolean _infoDictionaryFromResourceFork;
    char _padding[2];
} _CFResourceData;

typedef CF_ENUM(uint8_t, _CFBundleVersion) {
    _CFBundleVersionOldStyleResources = 0,
    _CFBundleVersionOldStyleSupportFiles = 1,
    _CFBundleVersionContentsResources = 2,
    _CFBundleVersionFlat = 3,
    _CFBundleVersionNotABundle = 4,
    _CFBundleVersionWrappedContentsResources = 12,
    _CFBundleVersionWrappedFlat = 13,
};

CF_PRIVATE _CFResourceData *__CFBundleGetResourceData(CFBundleRef bundle);

typedef struct __CFPlugInData {
    Boolean _isPlugIn;
    Boolean _loadOnDemand;
    Boolean _isDoingDynamicRegistration;
    Boolean _needsDynamicRegistration;
    Boolean _registeredFactory;
    UInt32 _instanceCount;
    UInt32 _unloadPreventionCount;
    CFMutableArrayRef _factories;
} _CFPlugInData;

struct __CFBundle {
    CFRuntimeBase _base;
    
    CFURLRef _url;
    
#if !DEPLOYMENT_RUNTIME_OBJC && !TARGET_OS_WIN32 && !TARGET_OS_ANDROID
    Boolean _isFHSInstalledBundle;
#endif
    
    CFDictionaryRef _infoDict;
    CFDictionaryRef _localInfoDict;
    CFArrayRef _searchLanguages;
    
    __CFPBinaryType _binaryType;
    _Atomic(Boolean) _isLoaded;
    _CFBundleVersion _version;
    
    Boolean _sharesStringsFiles;
    Boolean _isUnique;
    
    /* CFM goop */
    void *_connectionCookie;
    
    /* DYLD goop */
    const void *_imageCookie;
    const void *_moduleCookie;
    
    /* dlfcn goop */
    void *_handleCookie;
    
    /* Resource fork goop */
    _CFResourceData _resourceData;
    
    _CFPlugInData _plugInData;
    
    _CFMutex _bundleLoadingLock;
    
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
    
    CFLock_t _additionalResourceLock;
    CFMutableDictionaryRef _additionalResourceBundles;
    
    CFURLRef _infoPlistUrl;
    
#if defined(BINARY_SUPPORT_DLL)
    HMODULE _hModule;
#endif /* BINARY_SUPPORT_DLL */
};

CF_PRIVATE os_log_t _CFBundleResourceLogger(void);
CF_PRIVATE os_log_t _CFBundleLocalizedStringLogger(void);

extern _CFPlugInData *__CFBundleGetPlugInData(CFBundleRef bundle);

/* Private CFBundle API */

CF_PRIVATE CFErrorRef _CFBundleCreateErrorDebug(CFAllocatorRef allocator, CFBundleRef bundle, CFIndex code, CFStringRef debugString);
CF_PRIVATE CFURLRef _CFURLCreateResolvedDirectoryWithString(CFAllocatorRef allocator, CFStringRef URLString, CFURLRef baseURL);

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
CF_PRIVATE CFMutableDictionaryRef _CFBundleCreateInfoDictFromMainExecutable(void);
CF_PRIVATE Boolean _CFBundleGrokObjCImageInfoFromMainExecutable(uint32_t *objcVersion, uint32_t *objcFlags);
#endif

CF_PRIVATE CFStringRef _CFBundleCopyLoadedImagePathForPointer(void *p);

// Languages and locales

CF_PRIVATE CFArrayRef _CFBundleCopyLanguageSearchListInDirectory(CFURLRef url, _CFBundleVersion *version);
CF_PRIVATE CFArrayRef _CFBundleCopyLanguageSearchListInBundle(CFBundleRef bundle);

CF_PRIVATE Boolean CFBundleAllowMixedLocalizations(void);

// Misc

CF_PRIVATE Boolean _CFIsResourceAtURL(CFURLRef url, Boolean *isDir);
CF_PRIVATE Boolean _CFIsResourceAtPath(CFStringRef path, Boolean *isDir);

CF_PRIVATE _CFBundleVersion _CFBundleGetBundleVersionForURL(CFURLRef url);
CF_PRIVATE CFBundleRef _CFBundleCreateMain(CFAllocatorRef allocator, CFURLRef mainBundleURL);

CF_PRIVATE CFDictionaryRef _CFBundleCopyInfoDictionaryInDirectory(CFAllocatorRef alloc, CFURLRef url, _CFBundleVersion *version);
CF_PRIVATE CFURLRef _CFBundleCopyResourcesDirectoryURLInDirectory(CFURLRef bundleURL, _CFBundleVersion version);

CF_PRIVATE Boolean _CFBundleCouldBeBundle(CFURLRef url);
CF_PRIVATE CFDictionaryRef _CFBundleCopyInfoDictionaryInResourceForkWithAllocator(CFAllocatorRef alloc, CFURLRef url);
CF_PRIVATE CFStringRef _CFBundleCopyExecutableName(CFBundleRef bundle, CFURLRef url, CFDictionaryRef infoDict);
#if TARGET_OS_OSX
CF_PRIVATE CFStringRef _CFBundleCopyBundleDevelopmentRegionFromVersResource(CFBundleRef bundle);
#endif
CF_PRIVATE CFDictionaryRef _CFBundleCopyInfoDictionaryInExecutable(CFURLRef url);
CF_PRIVATE CFArrayRef _CFBundleCopyArchitecturesForExecutable(CFURLRef url);

CF_PRIVATE void _CFBundleRefreshInfoDictionaryAlreadyLocked(CFBundleRef bundle);

CF_PRIVATE CFStringRef _CFBundleGetPlatformExecutablesSubdirectoryName(void);

CF_PRIVATE void _CFPlugInUnscheduleForUnloading(CFBundleRef bundle);
CF_PRIVATE void _CFPlugInUnloadScheduledPlugIns(void);
CF_PRIVATE void _CFBundleUnloadExecutable(CFBundleRef bundle, Boolean unloadingPlugins);
CF_PRIVATE void _CFPlugInHandleDynamicRegistration(CFBundleRef bundle);

CF_PRIVATE _CFBundleVersion _CFBundleLayoutVersion(CFBundleRef bundle);
CF_PRIVATE _CFBundleVersion _CFBundleEffectiveLayoutVersion(CFBundleRef bundle);


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

extern Boolean _CFBundleInitPlugIn(CFBundleRef bundle, CFDictionaryRef infoDict, CFBundleRef *existingPlugIn);
extern void _CFPlugInHandleDynamicRegistration(CFBundleRef bundle);
extern void _CFBundleDeallocatePlugIn(CFBundleRef bundle);

extern void _CFPlugInWillUnload(CFPlugInRef plugIn);

/* Strings for parsing bundle structure */
#define _CFBundleSupportFilesDirectoryName1 CFSTR("Support Files")
#define _CFBundleSupportFilesDirectoryName2 CFSTR("Contents")
#define _CFBundleResourcesDirectoryName CFSTR("Resources")
#define _CFBundleWrapperLinkName CFSTR("WrappedBundle")
#define _CFBundleWrapperDirectoryName CFSTR("Wrapper")
#define _CFBundleExecutablesDirectoryName CFSTR("Executables")
#define _CFBundleNonLocalizedResourcesDirectoryName CFSTR("Non-localized Resources")

#if TARGET_OS_WIN32
#define _CFBundleSupportFilesDirectoryName1WithResources CFSTR("Support Files\\Resources")
#define _CFBundleSupportFilesDirectoryName2WithResources CFSTR("Contents\\Resources")
#define _CFBundleWrappedSupportFilesDirectoryName2WithResources CFSTR("WrappedBundle\\Contents\\Resources")
#else
#define _CFBundleSupportFilesDirectoryName1WithResources CFSTR("Support Files/Resources")
#define _CFBundleSupportFilesDirectoryName2WithResources CFSTR("Contents/Resources")
#define _CFBundleWrappedSupportFilesDirectoryName2WithResources CFSTR("WrappedBundle/Contents/Resources")
#endif

#define _CFBundleSupportFilesURLFromBase1 CFSTR("Support%20Files/")
#define _CFBundleSupportFilesURLFromBase2 CFSTR("Contents/")
#define _CFBundleWrappedSupportFilesURLFromBase2 CFSTR("WrappedBundle/Contents/")
#define _CFBundleWrappedSupportFilesURLFromBase3 CFSTR("WrappedBundle/")
#define _CFBundleResourcesURLFromBase0 CFSTR("Resources/")
#define _CFBundleResourcesURLFromBase1 CFSTR("Support%20Files/Resources/")
#define _CFBundleResourcesURLFromBase2 CFSTR("Contents/Resources/")
#define _CFBundleWrappedResourcesURLFromBase2 CFSTR("WrappedBundle/Contents/Resources/")
#define _CFBundleWrappedResourcesURLFromBase3 CFSTR("WrappedBundle/")
#define _CFBundleAppStoreReceiptURLFromBase0 CFSTR("_MASReceipt/receipt")
#define _CFBundleAppStoreReceiptURLFromBase1 CFSTR("Support%20Files/_MASReceipt/receipt")
#define _CFBundleAppStoreReceiptURLFromBase2 CFSTR("Contents/_MASReceipt/receipt")
#define _CFBundleWrappedAppStoreReceiptURLFromBase2 CFSTR("WrappedBundle/Contents/_MASReceipt/receipt")
#define _CFBundleWrappedAppStoreReceiptURLFromBase3 CFSTR("WrappedBundle/_MASReceipt/receipt")
#define _CFBundleExecutablesURLFromBase1 CFSTR("Support%20Files/Executables/")
#define _CFBundleExecutablesURLFromBase2 CFSTR("Contents/")
#define _CFBundleWrappedExecutablesURLFromBase2 CFSTR("WrappedBundle/Contents/")
#define _CFBundleWrappedExecutablesURLFromBase3 CFSTR("WrappedBundle/")

#define _CFBundleInfoURLFromBase0 CFSTR("Resources/Info.plist")
#define _CFBundleInfoURLFromBase1 CFSTR("Support%20Files/Info.plist")
#define _CFBundleInfoURLFromBase2 CFSTR("Contents/Info.plist")
#define _CFBundleInfoURLFromBase3 CFSTR("Info.plist")
#define _CFBundleWrappedInfoURLFromBase2 CFSTR("WrappedBundle/Contents/Info.plist")
#define _CFBundleWrappedInfoURLFromBase3 CFSTR("WrappedBundle/Info.plist")
#define _CFBundleInfoURLFromBaseNoExtension3 CFSTR("Info")

#if TARGET_OS_OSX
#define _CFBundlePlatformInfoURLFromBase0 CFSTR("Resources/Info-macos.plist")
#define _CFBundlePlatformInfoURLFromBase1 CFSTR("Support%20Files/Info-macos.plist")
#define _CFBundlePlatformInfoURLFromBase2 CFSTR("Contents/Info-macos.plist")
#define _CFBundlePlatformInfoURLFromBase3 CFSTR("Info-macos.plist")
#define _CFBundleWrappedPlatformInfoURLFromBase2 CFSTR("WrappedBundle/Contents/Info-macos.plist")
#define _CFBundleWrappedPlatformInfoURLFromBase3 CFSTR("WrappedBundle/Info-macos.plist")
#elif TARGET_OS_IPHONE
#define _CFBundlePlatformInfoURLFromBase0 CFSTR("Resources/Info-iphoneos.plist")
#define _CFBundlePlatformInfoURLFromBase1 CFSTR("Support%20Files/Info-iphoneos.plist")
#define _CFBundlePlatformInfoURLFromBase2 CFSTR("Contents/Info-iphoneos.plist")
#define _CFBundlePlatformInfoURLFromBase3 CFSTR("Info-iphoneos.plist")
#define _CFBundleWrappedPlatformInfoURLFromBase2 CFSTR("WrappedBundle/Contents/Info-iphoneos.plist")
#define _CFBundleWrappedPlatformInfoURLFromBase3 CFSTR("WrappedBundle/Info-iphoneos.plist")
#else
// No platform-specific variants in these cases
#define _CFBundlePlatformInfoURLFromBase0 _CFBundleInfoURLFromBase0
#define _CFBundlePlatformInfoURLFromBase1 _CFBundleInfoURLFromBase1
#define _CFBundlePlatformInfoURLFromBase2 _CFBundleInfoURLFromBase2
#define _CFBundlePlatformInfoURLFromBase3 _CFBundleInfoURLFromBase3
#define _CFBundleWrappedPlatformInfoURLFromBase2 _CFBundleWrappedInfoURLFromBase2
#define _CFBundleWrappedPlatformInfoURLFromBase3 _CFBundleWrappedInfoURLFromBase3
#endif

#define _CFBundleInfoPlistName CFSTR("Info.plist")

#if TARGET_OS_OSX
#define _CFBundlePlatformInfoPlistName CFSTR("Info-macos.plist")
#elif TARGET_OS_IPHONE
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
#define _CFBundleWrappedPrivateFrameworksURLFromBase2 CFSTR("WrappedBundle/Contents/Frameworks/")
#define _CFBundleWrappedPrivateFrameworksURLFromBase3 CFSTR("WrappedBundle/Frameworks/")
#define _CFBundleSharedFrameworksURLFromBase0 CFSTR("SharedFrameworks/")
#define _CFBundleSharedFrameworksURLFromBase1 CFSTR("Support%20Files/SharedFrameworks/")
#define _CFBundleSharedFrameworksURLFromBase2 CFSTR("Contents/SharedFrameworks/")
#define _CFBundleWrappedSharedFrameworksURLFromBase2 CFSTR("WrappedBundle/Contents/SharedFrameworks/")
#define _CFBundleWrappedSharedFrameworksURLFromBase3 CFSTR("WrappedBundle/SharedFrameworks/")
#define _CFBundleSharedSupportURLFromBase0 CFSTR("SharedSupport/")
#define _CFBundleSharedSupportURLFromBase1 CFSTR("Support%20Files/SharedSupport/")
#define _CFBundleSharedSupportURLFromBase2 CFSTR("Contents/SharedSupport/")
#define _CFBundleWrappedSharedSupportURLFromBase2 CFSTR("WrappedBundle/Contents/SharedSupport/")
#define _CFBundleWrappedSharedSupportURLFromBase3 CFSTR("WrappedBundle/SharedSupport/")
#define _CFBundleBuiltInPlugInsURLFromBase0 CFSTR("PlugIns/")
#define _CFBundleBuiltInPlugInsURLFromBase1 CFSTR("Support%20Files/PlugIns/")
#define _CFBundleBuiltInPlugInsURLFromBase2 CFSTR("Contents/PlugIns/")
#define _CFBundleWrappedBuiltInPlugInsURLFromBase2 CFSTR("WrappedBundle/Contents/PlugIns/")
#define _CFBundleWrappedBuiltInPlugInsURLFromBase3 CFSTR("WrappedBundle/PlugIns/")
#define _CFBundleAlternateBuiltInPlugInsURLFromBase0 CFSTR("Plug-ins/")
#define _CFBundleAlternateBuiltInPlugInsURLFromBase1 CFSTR("Support%20Files/Plug-ins/")
#define _CFBundleAlternateBuiltInPlugInsURLFromBase2 CFSTR("Contents/Plug-ins/")
#define _CFBundleWrappedAlternateBuiltInPlugInsURLFromBase2 CFSTR("WrappedBundle/Contents/Plug-ins/")
#define _CFBundleWrappedAlternateBuiltInPlugInsURLFromBase3 CFSTR("WrappedBundle/Plug-ins/")

#define _CFBundleLprojExtension CFSTR("lproj")
#define _CFBundleLprojExtensionWithDot CFSTR(".lproj")
#define _CFBundleDot CFSTR(".")
#define _CFBundleAllFiles CFSTR("_CFBAF_")
#define _CFBundleTypeIndicator CFSTR("_CFBT_")
// This directory contains resources (especially nibs) that may look up localized resources or may fall back to the development language resources
#define _CFBundleBaseDirectory CFSTR("Base")
#define _CFBundleBaseDirectoryWithLproj CFSTR("Base.lproj")

#define _CFBundleMacOSXPlatformName CFSTR("macos")
#define _CFBundleAlternateMacOSXPlatformName CFSTR("macosx")
#define _CFBundleiPhoneOSPlatformName CFSTR("iphoneos")
#define _CFBundleWatchOSPlatformName CFSTR("watchos")
#define _CFBundletvOSPlatformName CFSTR("tvos")
#define _CFBundleWindowsPlatformName CFSTR("windows")
#define _CFBundleHPUXPlatformName CFSTR("hpux")
#define _CFBundleSolarisPlatformName CFSTR("solaris")
#define _CFBundleLinuxPlatformName CFSTR("linux")
#define _CFBundleFreeBSDPlatformName CFSTR("freebsd")
#define _CFBundleMacOSXPlatformNameSuffix CFSTR("-macos")
#define _CFBundleAlternateMacOSXPlatformNameSuffix CFSTR("-macosx")
#define _CFBundleiPhoneOSPlatformNameSuffix CFSTR("-iphoneos")
#define _CFBundleWatchOSPlatformNameSuffix CFSTR("-watchos")
#define _CFBundletvOSPlatformNameSuffix CFSTR("-tvos")
#define _CFBundleWindowsPlatformNameSuffix CFSTR("-windows")
#define _CFBundleHPUXPlatformNameSuffix CFSTR("-hpux")
#define _CFBundleSolarisPlatformNameSuffix CFSTR("-solaris")
#define _CFBundleLinuxPlatformNameSuffix CFSTR("-linux")
#define _CFBundleFreeBSDPlatformNameSuffix CFSTR("-freebsd")

STATIC_CONST_STRING_DECL(_CFBundleMacDeviceName, "mac");
STATIC_CONST_STRING_DECL(_CFBundleiPhoneDeviceName, "iphone");
STATIC_CONST_STRING_DECL(_CFBundleiPodDeviceName, "ipod");
STATIC_CONST_STRING_DECL(_CFBundleiPadDeviceName, "ipad");
STATIC_CONST_STRING_DECL(_CFBundleAppleWatchDeviceName, "applewatch");
STATIC_CONST_STRING_DECL(_CFBundleAppleTVDeviceName, "appletv");

CF_PRIVATE CFStringRef _CFBundleGetProductNameSuffix(void);
CF_PRIVATE CFStringRef _CFBundleGetPlatformNameSuffix(void);

CF_PRIVATE const CFStringRef _kCFBundleUseAppleLocalizationsKey;

#define _CFBundleStringTableType CFSTR("strings")
#define _CFBundleStringDictTableType CFSTR("stringsdict")

#define _CFBundleUserLanguagesPreferenceName CFSTR("AppleLanguages")
#define _CFBundleOldUserLanguagesPreferenceName CFSTR("NSLanguages")

#define _CFBundleLocalizedResourceForkFileName CFSTR("Localized")

#define _CFBundleSiblingResourceDirectoryExtension CFSTR("resources")

#pragma mark -
#pragma mark Resolving paths from FDs

// The buffer must be PATH_MAX long or more.
static bool _CFGetPathFromFileDescriptor(int fd, char *path);

#if TARGET_OS_MAC || (TARGET_OS_BSD && !defined(__OpenBSD__))

static bool _CFGetPathFromFileDescriptor(int fd, char *path) {
    return fcntl(fd, F_GETPATH, path) != -1;
}

#elif TARGET_OS_LINUX

static bool _CFGetPathFromFileDescriptor(int fd, char *path) {
    char procfs[PATH_MAX] = { 0 };
    if (snprintf(procfs, PATH_MAX, "/proc/self/fd/%d", fd) < 0) {
        return false;
    }

    ssize_t size = readlink(procfs, path, PATH_MAX);
    if (size != -1) {
        return false;
    }

    if (size < PATH_MAX - 1) {
        path[size + 1] = 0;
    }

    return true;
}

#elif TARGET_OS_WIN32

static bool _CFGetPathFromFileDescriptor(int fd, char *path) {
    HANDLE hFile = _get_osfhandle(fd);
    if (hFile == INVALID_HANDLE_VALUE)
      return false;

    DWORD dwLength = GetFinalPathNameByHandleW(hFile, NULL, 0, 0);

    WCHAR *wszPath = (WCHAR *)malloc(dwLength);
    if (wszPath == NULL)
      return false;

    if (GetFinalPathNameByHandleW(hFile, wszPath, dwLength, 0) != dwLength - 1) {
      free(wszPath);
      return false;
    }

    CFStringRef location =
        CFStringCreateWithBytes(kCFAllocatorSystemDefault,
                                (const UInt8 *)wszPath, dwLength - 1,
                                kCFStringEncodingUTF16, false);
    path = strdup(CFStringGetCStringPtr(location, kCFStringEncodingUTF8));
    CFRelease(location);
    free(wszPath);
    return true;
}

#else

static bool _CFGetPathFromFileDescriptor(int fd, char *path) {
#warning This platform does not have a way to go back from an open file descriptor to a path.
    return false;
}

#endif

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFBUNDLE_INTERNAL__ */

