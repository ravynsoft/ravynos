/*      CFBundle.c
	Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
        Responsibility: Tony Parker
*/

#include <CoreFoundation/CoreFoundation.h>
#include "CFBundle_Internal.h"
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFSet.h>
#include <CoreFoundation/CFURLAccess.h>
#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFError_Private.h>
#include <string.h>
#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#include <CoreFoundation/CFByteOrder.h>
#include "CFBundle_BinaryTypes.h"
#include <ctype.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>


#if defined(BINARY_SUPPORT_DYLD)
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <crt_externs.h>
#endif /* BINARY_SUPPORT_DYLD */

#if defined(BINARY_SUPPORT_DLFCN)
#include <dlfcn.h>
#ifndef RTLD_FIRST
#define RTLD_FIRST 0
#endif
#endif /* BINARY_SUPPORT_DLFCN */

#if TARGET_OS_MAC
#include <fcntl.h>
#elif TARGET_OS_WIN32
#include <fcntl.h>
#include <io.h>
#endif


#define LOG_BUNDLE_LOAD 0

// Public CFBundle Info plist keys
CONST_STRING_DECL(kCFBundleInfoDictionaryVersionKey, "CFBundleInfoDictionaryVersion")
CONST_STRING_DECL(kCFBundleExecutableKey, "CFBundleExecutable")
CONST_STRING_DECL(kCFBundleIdentifierKey, "CFBundleIdentifier")
CONST_STRING_DECL(kCFBundleVersionKey, "CFBundleVersion")
CONST_STRING_DECL(kCFBundleDevelopmentRegionKey, "CFBundleDevelopmentRegion")
CONST_STRING_DECL(kCFBundleLocalizationsKey, "CFBundleLocalizations")

// Private CFBundle key, used to mean 'I support the same localizations as CoreFoundation'
CONST_STRING_DECL(_kCFBundleUseAppleLocalizationsKey, "_CFBundleUseAppleLocalizations")

// Private CFBundle Info plist keys, possible candidates for public constants
CONST_STRING_DECL(_kCFBundleAllowMixedLocalizationsKey, "CFBundleAllowMixedLocalizations")
CONST_STRING_DECL(_kCFBundleSupportedPlatformsKey, "CFBundleSupportedPlatforms")
CONST_STRING_DECL(_kCFBundleResourceSpecificationKey, "CFBundleResourceSpecification")

// Finder stuff
CONST_STRING_DECL(_kCFBundlePackageTypeKey, "CFBundlePackageType")
CONST_STRING_DECL(_kCFBundleSignatureKey, "CFBundleSignature")
CONST_STRING_DECL(_kCFBundleIconFileKey, "CFBundleIconFile")
CONST_STRING_DECL(_kCFBundleDocumentTypesKey, "CFBundleDocumentTypes")
CONST_STRING_DECL(_kCFBundleURLTypesKey, "CFBundleURLTypes")

// Keys that are usually localized in InfoPlist.strings
CONST_STRING_DECL(kCFBundleNameKey, "CFBundleName")
CONST_STRING_DECL(_kCFBundleDisplayNameKey, "CFBundleDisplayName")
CONST_STRING_DECL(_kCFBundleShortVersionStringKey, "CFBundleShortVersionString")
CONST_STRING_DECL(_kCFBundleGetInfoStringKey, "CFBundleGetInfoString")
CONST_STRING_DECL(_kCFBundleGetInfoHTMLKey, "CFBundleGetInfoHTML")

// Sub-keys for CFBundleDocumentTypes dictionaries
CONST_STRING_DECL(_kCFBundleTypeNameKey, "CFBundleTypeName")
CONST_STRING_DECL(_kCFBundleTypeRoleKey, "CFBundleTypeRole")
CONST_STRING_DECL(_kCFBundleTypeIconFileKey, "CFBundleTypeIconFile")
CONST_STRING_DECL(_kCFBundleTypeOSTypesKey, "CFBundleTypeOSTypes")
CONST_STRING_DECL(_kCFBundleTypeExtensionsKey, "CFBundleTypeExtensions")
CONST_STRING_DECL(_kCFBundleTypeMIMETypesKey, "CFBundleTypeMIMETypes")

// Sub-keys for CFBundleURLTypes dictionaries
CONST_STRING_DECL(_kCFBundleURLNameKey, "CFBundleURLName")
CONST_STRING_DECL(_kCFBundleURLIconFileKey, "CFBundleURLIconFile")
CONST_STRING_DECL(_kCFBundleURLSchemesKey, "CFBundleURLSchemes")

// Compatibility key names
CONST_STRING_DECL(_kCFBundleOldExecutableKey, "NSExecutable")
CONST_STRING_DECL(_kCFBundleOldInfoDictionaryVersionKey, "NSInfoPlistVersion")
CONST_STRING_DECL(_kCFBundleOldNameKey, "NSHumanReadableName")
CONST_STRING_DECL(_kCFBundleOldIconFileKey, "NSIcon")
CONST_STRING_DECL(_kCFBundleOldDocumentTypesKey, "NSTypes")
CONST_STRING_DECL(_kCFBundleOldShortVersionStringKey, "NSAppVersion")

// Compatibility CFBundleDocumentTypes key names
CONST_STRING_DECL(_kCFBundleOldTypeNameKey, "NSName")
CONST_STRING_DECL(_kCFBundleOldTypeRoleKey, "NSRole")
CONST_STRING_DECL(_kCFBundleOldTypeIconFileKey, "NSIcon")
CONST_STRING_DECL(_kCFBundleOldTypeExtensions1Key, "NSUnixExtensions")
CONST_STRING_DECL(_kCFBundleOldTypeExtensions2Key, "NSDOSExtensions")
CONST_STRING_DECL(_kCFBundleOldTypeOSTypesKey, "NSMacOSType")

// Internally used keys for loaded Info plists.
CONST_STRING_DECL(_kCFBundleInfoPlistURLKey, "CFBundleInfoPlistURL")
CONST_STRING_DECL(_kCFBundleRawInfoPlistURLKey, "CFBundleRawInfoPlistURL")
CONST_STRING_DECL(_kCFBundleNumericVersionKey, "CFBundleNumericVersion")
CONST_STRING_DECL(_kCFBundleExecutablePathKey, "CFBundleExecutablePath")
CONST_STRING_DECL(_kCFBundleResourcesFileMappedKey, "CSResourcesFileMapped")
CONST_STRING_DECL(_kCFBundleCFMLoadAsBundleKey, "CFBundleCFMLoadAsBundle")

// Keys used by NSBundle for loaded Info plists.
CONST_STRING_DECL(_kCFBundlePrincipalClassKey, "NSPrincipalClass")

static _CFMutex CFBundleGlobalDataLock = _CF_MUTEX_STATIC_INITIALIZER;

static CFMutableDictionaryRef _bundlesByIdentifier = NULL;
static CFMutableDictionaryRef _bundlesByURL = NULL;
static CFMutableArrayRef _allBundles = NULL;

static CFBundleRef _CFBundleCreate(CFAllocatorRef allocator, CFURLRef bundleURL, Boolean doFinalProcessing, Boolean unique, Boolean addToTables);
static void _CFBundleEnsureBundlesUpToDateWithHint(CFStringRef hint);
static void _CFBundleEnsureAllBundlesUpToDate(void);
static void _CFBundleEnsureBundleExistsForImagePath(CFStringRef imagePath, Boolean permissive);
static void _CFBundleEnsureBundlesExistForImagePaths(CFArrayRef imagePaths);

#pragma mark -

#if !DEPLOYMENT_RUNTIME_OBJC && !TARGET_OS_WIN32 && !TARGET_OS_ANDROID

// Functions and constants for FHS bundles:
#define _CFBundleFHSDirectory_share CFSTR("share")

static Boolean _CFBundleURLIsForFHSInstalledBundle(CFURLRef bundleURL) {
    // Paths of this form are FHS installed bundles:
    // <anywhere>/share/<name>.resources
    
    CFStringRef extension = CFURLCopyPathExtension(bundleURL);
    CFURLRef parentURL = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorSystemDefault, bundleURL);
    CFStringRef containingDirectoryName = parentURL ? CFURLCopyLastPathComponent(parentURL) : NULL;
    
    Boolean isFHSBundle =
        extension &&
        containingDirectoryName &&
        CFEqual(extension, _CFBundleSiblingResourceDirectoryExtension) &&
        CFEqual(containingDirectoryName, _CFBundleFHSDirectory_share);
    
    if (extension) CFRelease(extension);
    if (parentURL) CFRelease(parentURL);
    if (containingDirectoryName) CFRelease(containingDirectoryName);
    
    return isFHSBundle;
}
#endif // !DEPLOYMENT_RUNTIME_OBJC && !TARGET_OS_WIN32 && !TARGET_OS_ANDROID

CF_CROSS_PLATFORM_EXPORT Boolean _CFBundleSupportsFHSBundles() {
#if !DEPLOYMENT_RUNTIME_OBJC && !TARGET_OS_WIN32 && !TARGET_OS_ANDROID
    return true;
#else
    return false;
#endif
}

CF_CROSS_PLATFORM_EXPORT Boolean _CFBundleSupportsFreestandingBundles() {
#if !DEPLOYMENT_RUNTIME_OBJC
    return true;
#else
    return false;
#endif
}

#pragma mark -

CF_PRIVATE os_log_t _CFBundleResourceLogger(void) {
    static os_log_t _log;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _log = os_log_create("com.apple.CFBundle", "resources");
    });
    return _log;
}

CF_PRIVATE os_log_t _CFBundleLocalizedStringLogger(void) {
    static os_log_t _log;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _log = os_log_create("com.apple.CFBundle", "strings");
    });
    return _log;
}

CF_PRIVATE os_log_t _CFBundleLoadingLogger(void) {
    static os_log_t _log;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _log = os_log_create("com.apple.CFBundle", "loading");
    });
    return _log;
}

#pragma mark -

#if TARGET_OS_OSX
// Some apps may rely on the fact that CFBundle used to allow bundle objects to be deallocated (despite handing out unretained pointers via CFBundleGetBundleWithIdentifier or CFBundleGetAllBundles). To remain compatible even in the face of unsafe behavior, we can optionally use unsafe-unretained memory management for holding on to bundles.
static Boolean _useUnsafeUnretainedTables(void) {
    return false;
}
#endif

#pragma mark -
#pragma mark Utilities

// Create an absolute URL after creating a URL with the supplied URLString, baseURL.
// This is used to resolve symbolic links in the path (For example, for wrapped bundles)
CF_PRIVATE CFURLRef _CFURLCreateResolvedDirectoryWithString(CFAllocatorRef allocator, CFStringRef URLString, CFURLRef baseURL) {
    CFURLRef relativeURL = CFURLCreateWithString(allocator, URLString, baseURL);
    CFURLRef absoluteURL = CFURLCopyAbsoluteURL(relativeURL);
    CFRelease(relativeURL);
    CFStringRef absolutePath = CFURLCopyFileSystemPath(absoluteURL, PLATFORM_PATH_STYLE);
    CFRelease(absoluteURL);
    
    char absolutePathCString[PATH_MAX];
    Boolean success = CFStringGetFileSystemRepresentation(absolutePath, absolutePathCString, PATH_MAX);
    CFRelease(absolutePath);
    if (success) {
        int fd = open(absolutePathCString, O_RDONLY);
        if (fd > 0) {
            char resolvedPathCString[PATH_MAX];
            if (_CFGetPathFromFileDescriptor(fd, resolvedPathCString)) {
                os_log_error(_CFBundleResourceLogger(), "Unable to resolve directory (%d)", errno);
                close(fd);
            } else {
                close(fd);
                CFStringRef resolvedPath = CFStringCreateWithFileSystemRepresentation(allocator, resolvedPathCString);
                CFURLRef result = CFURLCreateWithFileSystemPath(allocator, resolvedPath, PLATFORM_PATH_STYLE, true);
                CFRelease(resolvedPath);
                return result;
            }
        }
    }
    
    return NULL;
}

#pragma mark -
#pragma mark Bundle Tables

static void _CFBundleAddToTablesLocked(CFBundleRef bundle, CFStringRef bundleID) {
    if (bundle->_isUnique) return;

    // Add to the _allBundles list
    if (!_allBundles) {
        CFArrayCallBacks callbacks = kCFTypeArrayCallBacks;
#if TARGET_OS_OSX
        if (_useUnsafeUnretainedTables()) {
            callbacks.retain = NULL;
            callbacks.release = NULL;
        }
#endif
        // The _allBundles array holds a strong reference on the bundle.
        // It does this to prevent a race on bundle deallocation / creation. See: <rdar://problem/6606482> CFBundle isn't thread-safe in RR mode
        // Also, the existence of the CFBundleGetBundleWithIdentifier / CFBundleGetAllBundles API means that any bundle we hand out from there must be permanently retained, or callers will potentially have an object that can be deallocated out from underneath them.
        _allBundles = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &callbacks);
    }
    CFArrayAppendValue(_allBundles, bundle);
    
    // Add to the table that maps urls to bundles
    if (!_bundlesByURL) {
        CFDictionaryValueCallBacks nonRetainingDictionaryValueCallbacks = kCFTypeDictionaryValueCallBacks;
        nonRetainingDictionaryValueCallbacks.retain = NULL;
        nonRetainingDictionaryValueCallbacks.release = NULL;
        _bundlesByURL = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &nonRetainingDictionaryValueCallbacks);
    }
    CFDictionarySetValue(_bundlesByURL, bundle->_url, bundle);

    // Add to the table that maps identifiers to bundles
    if (bundleID) {
        CFMutableArrayRef bundlesWithThisID = NULL;
        CFBundleRef existingBundle = NULL;
        if (!_bundlesByIdentifier) {
            _bundlesByIdentifier = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        }
        bundlesWithThisID = (CFMutableArrayRef)CFDictionaryGetValue(_bundlesByIdentifier, bundleID);
        if (bundlesWithThisID) {
            CFIndex i, count = CFArrayGetCount(bundlesWithThisID);
            UInt32 existingVersion, newVersion = CFBundleGetVersionNumber(bundle);
            for (i = 0; i < count; i++) {
                existingBundle = (CFBundleRef)CFArrayGetValueAtIndex(bundlesWithThisID, i);
                existingVersion = CFBundleGetVersionNumber(existingBundle);
                // If you load two bundles with the same identifier and the same version, the last one wins.
                if (newVersion >= existingVersion) break;
            }
            CFArrayInsertValueAtIndex(bundlesWithThisID, i, bundle);

            // We've encountered a bundle with this ID already.
            // Output some additional info here. It may not be an error (adding a newer version of a bundle is supported), so use os_log_debug.
            if (os_log_debug_enabled(_CFBundleResourceLogger())) {
                CFMutableArrayRef bundleList = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
                for (CFIndex i = 0; i < CFArrayGetCount(bundlesWithThisID); i++) {
                    CFBundleRef bundle = (CFBundleRef)CFArrayGetValueAtIndex(bundlesWithThisID, i);
                    CFStringRef desc = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("Bundle %p at %@"), bundle, bundle->_url);
                    CFArrayAppendValue(bundleList, desc);
                    CFRelease(desc);
                }
                os_log_debug(_CFBundleResourceLogger(), "More than one bundle with the same identifier has been added: %{public}@", bundleList);
                CFRelease(bundleList);
            }
        } else {
            CFArrayCallBacks nonRetainingArrayCallbacks = kCFTypeArrayCallBacks;
            nonRetainingArrayCallbacks.retain = NULL;
            nonRetainingArrayCallbacks.release = NULL;
            bundlesWithThisID = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &nonRetainingArrayCallbacks);
            CFArrayAppendValue(bundlesWithThisID, bundle);
            CFDictionarySetValue(_bundlesByIdentifier, bundleID, bundlesWithThisID);
            CFRelease(bundlesWithThisID);
        }
    }
}

static void _CFBundleRemoveFromTables(CFBundleRef bundle, CFURLRef bundleURL, CFStringRef bundleID) {
    // Since we no longer allow bundles to be removed from tables, this method does nothing. Modifying the tables during deallocation is risky because if the caller has over-released the bundle object then we will deadlock on the global lock.
#if TARGET_OS_OSX
    if (_useUnsafeUnretainedTables()) {
        // Except for special cases of unsafe-unretained, where we must clean up the table or risk handing out a zombie object. There may still be outstanding pointers to these bundes (e.g. the result of CFBundleGetBundleWithIdentifier) but there is nothing we can do about that after this point.
        
        // Unique bundles aren't in the tables anyway
        if (bundle->_isUnique) return;
        
        _CFMutexLock(&CFBundleGlobalDataLock);
        // Remove from the table of all bundles
        if (_allBundles) {
            CFIndex i = CFArrayGetFirstIndexOfValue(_allBundles, CFRangeMake(0, CFArrayGetCount(_allBundles)), bundle);
            if (i >= 0) CFArrayRemoveValueAtIndex(_allBundles, i);
        }
        
        // Remove from the table that maps urls to bundles
        if (bundleURL && _bundlesByURL) {
            CFBundleRef bundleForURL = (CFBundleRef)CFDictionaryGetValue(_bundlesByURL, bundleURL);
            if (bundleForURL == bundle) CFDictionaryRemoveValue(_bundlesByURL, bundleURL);
        }
        
        // Remove from the table that maps identifiers to bundles
        if (bundleID && _bundlesByIdentifier) {
            CFMutableArrayRef bundlesWithThisID = (CFMutableArrayRef)CFDictionaryGetValue(_bundlesByIdentifier, bundleID);
            if (bundlesWithThisID) {
                CFIndex count = CFArrayGetCount(bundlesWithThisID);
                while (count-- > 0) if (bundle == (CFBundleRef)CFArrayGetValueAtIndex(bundlesWithThisID, count)) CFArrayRemoveValueAtIndex(bundlesWithThisID, count);
                if (0 == CFArrayGetCount(bundlesWithThisID)) CFDictionaryRemoveValue(_bundlesByIdentifier, bundleID);
            }
        }
        _CFMutexUnlock(&CFBundleGlobalDataLock);
    }
#endif
}

static CFBundleRef _CFBundleGetFromTablesLocked(CFStringRef bundleID) {
    CFBundleRef result = NULL, bundle;
    if (_bundlesByIdentifier && bundleID) {
        // Note that this array is maintained in descending order by version number
        CFArrayRef bundlesWithThisID = (CFArrayRef)CFDictionaryGetValue(_bundlesByIdentifier, bundleID);
        if (bundlesWithThisID) {
            CFIndex i, count = CFArrayGetCount(bundlesWithThisID);
            if (count > 0) {
                // First check for loaded bundles so we will always prefer a loaded to an unloaded bundle
                for (i = 0; !result && i < count; i++) {
                    bundle = (CFBundleRef)CFArrayGetValueAtIndex(bundlesWithThisID, i);
                    if (CFBundleIsExecutableLoaded(bundle)) result = bundle;
                }
                // If no loaded bundle, simply take the first item in the array, i.e. the one with the latest version number
                if (!result) result = (CFBundleRef)CFArrayGetValueAtIndex(bundlesWithThisID, 0);
            }
        }
    }
    return result;
}

static CFBundleRef _CFBundleGetFromTables(CFStringRef bundleID) {
    _CFMutexLock(&CFBundleGlobalDataLock);
    CFBundleRef result = _CFBundleGetFromTablesLocked(bundleID);
    _CFMutexUnlock(&CFBundleGlobalDataLock);
    return result;
}

static CFBundleRef _CFBundleCopyFromTablesForURLLocked(CFURLRef url) {
    /*
     If you're curious why this doesn't consult the main bundle URL, consider the case where you have a directory structure like this:
     
     /S/L/F/Foo.framework/Foo
     /S/L/F/Foo.framework/food      (a daemon for the Foo framework)
     
     And the main executable is 'food'.
     
     This flat structure can happen on iOS, with its more common version 3 bundles. In this scenario, there are theoretically two different bundles that could be returned: one for the framework, one for the daemon. They have the same URL but different bundle identifiers.
     
     Since the main bundle is not part of the bundle tables, we can support this scenario by having the _bundlesByURL data structure hold the bundle for URL "/S/L/F/Foo.framework/Foo" and _mainBundle (in CFBundle_Main.c) hold the bundle for URL "/S/L/F/Foo.framework/food".
     */
    CFBundleRef result = NULL;
    if (_bundlesByURL) result = (CFBundleRef)CFDictionaryGetValue(_bundlesByURL, url);
    if (result && !result->_url) {
        result = NULL;
        CFDictionaryRemoveValue(_bundlesByURL, url);
    }
    if (result) CFRetain(result);
    return result;
}

static CFBundleRef _CFBundleCopyFromTablesForURL(CFURLRef url) {
    _CFMutexLock(&CFBundleGlobalDataLock);
    CFBundleRef result = _CFBundleCopyFromTablesForURLLocked(url);
    _CFMutexUnlock(&CFBundleGlobalDataLock);
    return result;
}

#pragma mark -

CF_PRIVATE _CFBundleVersion _CFBundleEffectiveLayoutVersion(CFBundleRef bundle) {
    _CFBundleVersion localVersion = bundle->_version;
    // exclude type 0 bundles with no binary (or CFM binary) and no Info.plist, since they give too many false positives
    if (_CFBundleVersionOldStyleResources == localVersion) {
        CFDictionaryRef infoDict = CFBundleGetInfoDictionary(bundle);
        if (!infoDict || 0 == CFDictionaryGetCount(infoDict)) {
#if defined(BINARY_SUPPORT_DYLD)
            CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
            if (executableURL) {
                if (bundle->_binaryType == __CFBundleUnknownBinary) bundle->_binaryType = _CFBundleGrokBinaryType(executableURL);
                if (bundle->_binaryType == __CFBundleCFMBinary || bundle->_binaryType == __CFBundleUnreadableBinary) {
                    localVersion = _CFBundleVersionNotABundle;
                } else {
                    bundle->_resourceData._executableLacksResourceFork = true;
                }
                CFRelease(executableURL);
            } else {
                localVersion = _CFBundleVersionNotABundle;
            }
#else 
            CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
            if (executableURL) {
                CFRelease(executableURL);
            } else {
                localVersion = _CFBundleVersionNotABundle;
            }
#endif /* BINARY_SUPPORT_DYLD */
        }
    }
    return localVersion;
}

CFBundleRef _CFBundleCreateIfLooksLikeBundle(CFAllocatorRef allocator, CFURLRef url) {
    // It is assumed that users of this SPI do not want this bundle to persist forever.
    CFBundleRef bundle = _CFBundleCreateUnique(allocator, url);
    if (bundle) {
        _CFBundleVersion localVersion = _CFBundleEffectiveLayoutVersion(bundle);
        if (_CFBundleVersionFlat == localVersion || _CFBundleVersionNotABundle == localVersion) {
            CFRelease(bundle);
            bundle = NULL;
        }
    }
    return bundle;
}

CF_EXPORT Boolean _CFBundleURLLooksLikeBundle(CFURLRef url) {
    Boolean result = false;
    CFBundleRef bundle = _CFBundleCreateIfLooksLikeBundle(kCFAllocatorSystemDefault, url);
    if (bundle) {
        result = true;
        CFRelease(bundle);
    }
    return result;
}

CFBundleRef _CFBundleGetMainBundleIfLooksLikeBundle(void) {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (mainBundle && (_CFBundleVersionFlat == mainBundle->_version || _CFBundleVersionNotABundle == mainBundle->_version)) mainBundle = NULL;
    return mainBundle;
}

Boolean _CFBundleMainBundleInfoDictionaryComesFromResourceFork(void) {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    return (mainBundle && mainBundle->_resourceData._infoDictionaryFromResourceFork);
}

CF_EXPORT CFBundleRef _CFBundleCreateIfMightBeBundle(CFAllocatorRef allocator, CFURLRef url) {
    // This function is obsolete
    CFBundleRef bundle = CFBundleCreate(allocator, url);
    return bundle;
}
        
CF_EXPORT void _CFBundleFlushBundleCaches(CFBundleRef bundle) {
    __CFLock(&bundle->_lock);
    CFDictionaryRef oldInfoDict = bundle->_infoDict;
    CFTypeRef val;
    
    bundle->_infoDict = NULL;
    if (bundle->_localInfoDict) {
        CFRelease(bundle->_localInfoDict);
        bundle->_localInfoDict = NULL;
    }
    if (bundle->_infoPlistUrl) {
        CFRelease(bundle->_infoPlistUrl);
        bundle->_infoPlistUrl = NULL;
    }
    if (bundle->_developmentRegion) {
        CFRelease(bundle->_developmentRegion);
        bundle->_developmentRegion = NULL;
    }
    if (bundle->_executablePath) {
        CFRelease(bundle->_executablePath);
        bundle->_executablePath = NULL;
    }
    if (bundle->_searchLanguages) {
        CFRelease(bundle->_searchLanguages);
        bundle->_searchLanguages = NULL;
    }
    if (bundle->_stringTable) {
        CFRelease(bundle->_stringTable);
        bundle->_stringTable = NULL;
    }
    _CFBundleRefreshInfoDictionaryAlreadyLocked(bundle);
    if (oldInfoDict) {
        if (!bundle->_infoDict) bundle->_infoDict = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        val = CFDictionaryGetValue(oldInfoDict, _kCFBundlePrincipalClassKey);
        if (val) CFDictionarySetValue((CFMutableDictionaryRef)bundle->_infoDict, _kCFBundlePrincipalClassKey, val);
        CFRelease(oldInfoDict);
    }
    _CFBundleFlushQueryTableCache(bundle);
    __CFUnlock(&bundle->_lock);
}

static CFBundleRef _CFBundleGetBundleWithIdentifier(CFStringRef bundleID, void *frameworkHint) {
    CFBundleRef result = NULL;
    if (bundleID) {
        CFBundleRef main = CFBundleGetMainBundle();
        if (main) {
            CFDictionaryRef infoDict = CFBundleGetInfoDictionary(main);
            if (infoDict) {
                CFStringRef mainBundleID = CFDictionaryGetValue(infoDict, kCFBundleIdentifierKey);
                if (mainBundleID && CFGetTypeID(mainBundleID) == CFStringGetTypeID() && CFEqual(mainBundleID, bundleID)) {
                    return main;
                }
            }
        }
        
        result = _CFBundleGetFromTables(bundleID);
#if TARGET_OS_MAC
        if (!result) {
            // Try to create the bundle for the caller and try again
            if (frameworkHint) {
                CFStringRef imagePath = _CFBundleCopyLoadedImagePathForPointer(frameworkHint);
                if (imagePath) {
                    // As this is a fast-path check, we don't want to be aggressive about assuming that the executable URL that we may have received from DYLD via _CFBundleCopyLoadedImagePathForPointer should be turned into a framework URL. If we do, then it is possible that an executable located inside a framework bundle which does not normally link that framework will cause us to load it unintentionally (31165928).
                    // For example:
                    // Foo.framework/
                    //               Resources/
                    //                         HelperTool
                    //
                    // With permissive set to 'true', this would make the 'Foo.framework' bundle exist, but there is no reason why HelperTool is required to have loaded Foo.framework.
                    
                    _CFBundleEnsureBundleExistsForImagePath(imagePath, false);
                    CFRelease(imagePath);
                }
                
                // Now try again
                result = _CFBundleGetFromTables(bundleID);
            }
        }
#endif
        if (!result) {
            // Try to guess the bundle from the identifier and try again
            _CFBundleEnsureBundlesUpToDateWithHint(bundleID);
            
            // Now try again
            result = _CFBundleGetFromTables(bundleID);
        }
    }
    
    if (!result) {
        // Make sure all bundles have been created and try again.
        _CFBundleEnsureAllBundlesUpToDate();

        // Now try again
        result = _CFBundleGetFromTables(bundleID);
    }

    return result;
}

CFBundleRef CFBundleGetBundleWithIdentifier(CFStringRef bundleID) {
    // Use the frame that called this as a hint
    return _CFBundleGetBundleWithIdentifier(bundleID, __builtin_return_address(0));
}

CFBundleRef _CFBundleGetBundleWithIdentifierWithHint(CFStringRef bundleID, void *pointer) {
    return _CFBundleGetBundleWithIdentifier(bundleID, pointer);
}

static CFStringRef __CFBundleCopyDescription(CFTypeRef cf) {
    char buff[CFMaxPathSize];
    CFStringRef path = NULL, binaryType = NULL, retval = NULL;
    if (((CFBundleRef)cf)->_url && CFURLGetFileSystemRepresentation(((CFBundleRef)cf)->_url, true, (uint8_t *)buff, CFMaxPathSize)) path = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, buff);
    switch (((CFBundleRef)cf)->_binaryType) {
        case __CFBundleCFMBinary:
            binaryType = CFSTR("");
            break;
        case __CFBundleDYLDExecutableBinary:
            binaryType = CFSTR("executable, ");
            break;
        case __CFBundleDYLDBundleBinary:
            binaryType = CFSTR("bundle, ");
            break;
        case __CFBundleDYLDFrameworkBinary:
            binaryType = CFSTR("framework, ");
            break;
        case __CFBundleDLLBinary:
            binaryType = CFSTR("DLL, ");
            break;
        case __CFBundleUnreadableBinary:
            binaryType = CFSTR("");
            break;
        default:
            binaryType = CFSTR("");
            break;
    }
    if (((CFBundleRef)cf)->_plugInData._isPlugIn) {
        retval = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("CFBundle/CFPlugIn %p <%@> (%@%@loaded)"), cf, path, binaryType, ((CFBundleRef)cf)->_isLoaded ? CFSTR("") : CFSTR("not "));
    } else {
        retval = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("CFBundle %p <%@> (%@%@loaded)"), cf, path, binaryType, ((CFBundleRef)cf)->_isLoaded ? CFSTR("") : CFSTR("not "));
    }
    if (path) CFRelease(path);
    return retval;
}

static void __CFBundleDeallocate(CFTypeRef cf) {
    CFBundleRef bundle = (CFBundleRef)cf;
    CFURLRef bundleURL;
    CFStringRef bundleID = NULL;
    
    __CFGenericValidateType(cf, CFBundleGetTypeID());
    bundleURL = bundle->_url;
    bundle->_url = NULL;
    if (bundle->_infoDict) bundleID = (CFStringRef)CFDictionaryGetValue(bundle->_infoDict, kCFBundleIdentifierKey);
    _CFBundleRemoveFromTables(bundle, bundleURL, bundleID);
    CFBundleUnloadExecutable(bundle);
    _CFBundleDeallocatePlugIn(bundle);    
    if (bundleURL) {
        CFRelease(bundleURL);
    }
    if (bundle->_infoDict) CFRelease(bundle->_infoDict);
    if (bundle->_localInfoDict) CFRelease(bundle->_localInfoDict);
    if (bundle->_searchLanguages) CFRelease(bundle->_searchLanguages);
    if (bundle->_executablePath) CFRelease(bundle->_executablePath);
    if (bundle->_developmentRegion) CFRelease(bundle->_developmentRegion);
    if (bundle->_infoPlistUrl) CFRelease(bundle->_infoPlistUrl);
    if (bundle->_stringTable) CFRelease(bundle->_stringTable);
    if (bundle->_bundleBasePath) CFRelease(bundle->_bundleBasePath);
    if (bundle->_queryTable) CFRelease(bundle->_queryTable);
    
    if (bundle->_localizations) CFRelease(bundle->_localizations);
    if (bundle->_resourceDirectoryContents) CFRelease(bundle->_resourceDirectoryContents);
    
    if (bundle->_additionalResourceBundles) CFRelease(bundle->_additionalResourceBundles);
    
    _CFMutexDestroy(&(bundle->_bundleLoadingLock));
}

const CFRuntimeClass __CFBundleClass = {
    _kCFRuntimeScannedObject,
    "CFBundle",
    NULL,      // init
    NULL,      // copy
    __CFBundleDeallocate,
    NULL,      // equal
    NULL,      // hash
    NULL,      // 
    __CFBundleCopyDescription
};

CFTypeID CFBundleGetTypeID(void) {
    return _kCFRuntimeIDCFBundle;
}

// TODO: Remove this SPI, it appears no one is using it
// <rdar://problem/30925651> Remove _CFBundleGetExistingBundleWithBundleURL
CFBundleRef _CFBundleGetExistingBundleWithBundleURL(CFURLRef bundleURL) {
    CFBundleRef bundle = NULL;
    char buff[CFMaxPathSize];
    CFURLRef newURL = NULL;
    
    if (!CFURLGetFileSystemRepresentation(bundleURL, true, (uint8_t *)buff, CFMaxPathSize)) return NULL;
    
    newURL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, (uint8_t *)buff, strlen(buff), true);
    if (!newURL) newURL = (CFURLRef)CFRetain(bundleURL);
    
    // First check the main bundle; otherwise fallback to the other tables
    CFBundleRef main = CFBundleGetMainBundle();
    if (main->_url && newURL && CFEqual(main->_url, newURL)) {
        CFRelease(newURL);
        return main;
    }

    bundle = _CFBundleCopyFromTablesForURL(newURL);
    if (bundle) CFRelease(bundle);
    CFRelease(newURL);
    return bundle;
}

static Boolean _CFBundlesHaveEquivalentURL(CFBundleRef b1, CFBundleRef b2) {
    CFStringRef path1 = b1->_bundleBasePath;
    CFStringRef path2 = b2->_bundleBasePath;
    char path1CString[PATH_MAX];
    char path2CString[PATH_MAX];

    Boolean result = false;
    if (CFStringGetFileSystemRepresentation(path1, path1CString, PATH_MAX) &&
        CFStringGetFileSystemRepresentation(path2, path2CString, PATH_MAX)) {
        int fd1 = open(path1CString, O_RDONLY);
        int fd2 = open(path2CString, O_RDONLY);

        if (fd1 > 0 && fd2 > 0) {
            struct stat statbuf1;
            struct stat statbuf2;
            
            if (fstat(fd1, &statbuf1) == 0 &&
                fstat(fd2, &statbuf2) == 0 &&
                statbuf1.st_dev == statbuf2.st_dev &&
                statbuf1.st_ino == statbuf2.st_ino)
            {
                    result = true;
            }
        }
        
        if (fd1 > 0) close(fd1);
        if (fd2 > 0) close(fd2);
    }
    
    return result;
}

/*
 The combinations of boolean arguments to this function are as follows:
 
 doFinal | unique | addToTables
 ------- | ------ | -----------
 true      false    true            // CFBundleCreate
 true      true     false           // _CFBundleCreateUnique
 false     false    false           // _CFBundleCreateMain
 false     false    true            // _CFBundleEnsureBundleExistsForImagePath
 
 n.b.
 (unique && addToTables) is an invalid configuration and this function will assert.
 (unique && doFinalProcessing) does nothing but there is no assert; doFinalProcessing is ignored
    This is because doFinalProcessing is primarily about initializing plugins, but unique bundles cannot be plugins.
 
 */
static CFBundleRef _CFBundleCreate(CFAllocatorRef allocator, CFURLRef bundleURL, Boolean doFinalProcessing, Boolean unique, Boolean addToTables) {
    char buff[CFMaxPathSize];
    CFURLRef newURL = NULL;
    if (!CFURLGetFileSystemRepresentation(bundleURL, true, (uint8_t *)buff, CFMaxPathSize)) return NULL;
    newURL = CFURLCreateFromFileSystemRepresentation(allocator, (uint8_t *)buff, strlen(buff), true);
    if (!newURL) newURL = (CFURLRef)CFRetain(bundleURL);
    
    // Don't go searching for the URL in the tables if the bundle is unique or the main bundle (addToTables == false)
    if (!unique && addToTables) {
        CFBundleRef existingBundle = _CFBundleCopyFromTablesForURL(newURL);
        if (existingBundle) {
            CFRelease(newURL);
            return existingBundle;
        }
    }
    
    
    // This section of _CFBundleCreate touches the disk, so it should be done outside the lock. This avoids blocking threads which are just attempting to get an existing bundle on file system access, potentially from a lower-piority thread.
    _CFBundleVersion localVersion = _CFBundleGetBundleVersionForURL(newURL);
    if (_CFBundleVersionFlat == localVersion) {
        Boolean exists = false;
        SInt32 mode = 0;
        SInt32 res = _CFGetPathProperties(allocator, (char *)buff, &exists, &mode, NULL, NULL, NULL, NULL);
#if DEPLOYMENT_RUNTIME_OBJC && TARGET_OS_WIN32
        if (!(res == 0 && exists && ((mode & S_IFMT) == S_IFDIR))) {
            // 2nd chance at finding a bundle path - remove the last path component (e.g., mybundle.resources) and try again
            CFURLRef shorterPath = CFURLCreateCopyDeletingLastPathComponent(allocator, newURL);
            CFRelease(newURL);
            newURL = shorterPath;
            res = _CFGetFileProperties(allocator, newURL, &exists, &mode, NULL, NULL, NULL, NULL);
        }
#endif
        if (res == 0) {
            if (!exists || ((mode & S_IFMT) != S_IFDIR)) {
                CFRelease(newURL);
                return NULL;
            }
        } else {
            CFRelease(newURL);
            return NULL;
        }
    }

    CFBundleRef bundle = (CFBundleRef)_CFRuntimeCreateInstance(allocator, CFBundleGetTypeID(), sizeof(struct __CFBundle) - sizeof(CFRuntimeBase), NULL);
    if (!bundle) {
        CFRelease(newURL);
        return NULL;
    }

    bundle->_url = newURL;
    
#if !DEPLOYMENT_RUNTIME_OBJC && !TARGET_OS_WIN32 && !TARGET_OS_ANDROID
    bundle->_isFHSInstalledBundle = _CFBundleURLIsForFHSInstalledBundle(newURL);
#endif

    bundle->_version = localVersion;

#if defined(BINARY_SUPPORT_DYLD)
    /* We'll have to figure it out later */
    bundle->_binaryType = __CFBundleUnknownBinary;
#elif defined(BINARY_SUPPORT_DLL)
    /* We support DLL only */
    bundle->_binaryType = __CFBundleDLLBinary;
#else
    /* We'll have to figure it out later */
    bundle->_binaryType = __CFBundleUnknownBinary;
#endif /* BINARY_SUPPORT_DYLD */

    bundle->_isUnique = unique;
    
#if TARGET_OS_OSX
    if (!__CFgetenv("CFBundleDisableStringsSharing") && 
        (strncmp(buff, "/System/Library/Frameworks", 26) == 0) && 
        (strncmp(buff + strlen(buff) - 10, ".framework", 10) == 0)) bundle->_sharesStringsFiles = true;
#endif

    _CFMutexCreate(&(bundle->_bundleLoadingLock));

    bundle->_lock = CFLockInit;
    bundle->_queryLock = CFLockInit;

    CFURLRef absoURL = CFURLCopyAbsoluteURL(bundle->_url);
    bundle->_bundleBasePath = CFURLCopyFileSystemPath(absoURL, PLATFORM_PATH_STYLE);
    CFRelease(absoURL);
    
    bundle->_additionalResourceLock = CFLockInit;
    
    CFDictionaryRef infoDict = CFBundleGetInfoDictionary(bundle);
    CFStringRef bundleID = CFBundleGetIdentifier(bundle);

    // Do this so that we can use the dispatch_once on the ivar of this bundle safely
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
    OSMemoryBarrier();
#pragma GCC diagnostic pop

    // We cannot add to tables for unique bundles. Return unique bundle results here without heading into the section below where we take a lock.
    if (unique) {
        assert(!addToTables);
        return bundle;
    }
    
    // Locked after here for the global tables
    _CFMutexLock(&CFBundleGlobalDataLock);
    
    // Check to see if we already have a bundle with the same identifier or URL. Someone may have raced us to this point.
    CFBundleRef existingBundleWithSameURL = _CFBundleCopyFromTablesForURLLocked(bundle->_url);
    if (existingBundleWithSameURL) {
        // Deallocating a bundle takes the lock when _useUnsafeUnretainedTables returns true, so release the lock first.
        _CFMutexUnlock(&CFBundleGlobalDataLock);
        CFRelease(bundle);

        return existingBundleWithSameURL;
    }
    
    // It's unfortunate, but valid, for two different bundles to have the same identifier. However, if they also have the same URL (resolving symlinks), then we can return the original one because they are really the same bundle after all.
    CFBundleRef existingBundleWithSameIdentifier = _CFBundleGetFromTablesLocked(bundleID);
    if (existingBundleWithSameIdentifier && _CFBundlesHaveEquivalentURL(bundle, existingBundleWithSameIdentifier)) {
        // Deallocating a bundle takes the lock when _useUnsafeUnretainedTables returns true, so release the lock first.
        _CFMutexUnlock(&CFBundleGlobalDataLock);
        CFRelease(bundle);
        return (CFBundleRef)CFRetain(existingBundleWithSameIdentifier);
    }
    
    if (doFinalProcessing) {
        // Initializing the tables for the list of plugins and initializing the tables for the list of bundles are inherently tied together, because plugins have references back to bundles.
        // In order to preserve safety, we take both locks. Always in the order of CFBundleGlobalDataLock -> CFPlugInGlobalDataLock.

        // We have to do this before adding the bundle to the tables, otherwise another thread coming along and fetching the bundle will get the bundle ref but the factories for PlugIn have not yet been registered.
        CFBundleRef duplicateFactoryBundle = NULL;
        if (!_CFBundleInitPlugIn(bundle, infoDict, &duplicateFactoryBundle)) {
            // At this point the most likely explanation is that we have the same factory ID but two different bundles. There is no way to really proceed safely here because the plugin system can only deal with one factory ID -> bundle pairing.
            // Get some info for error logging
            CFDictionaryRef factoryDict = (CFDictionaryRef)CFDictionaryGetValue(infoDict, kCFPlugInFactoriesKey);
            if (factoryDict && CFGetTypeID(factoryDict) != CFDictionaryGetTypeID()) factoryDict = NULL;
            os_log_error(_CFBundleLoadingLogger(), "More than one bundle with the same factory UUID detected: %{public}@ in %{public}@ and %{public}@", factoryDict, bundle, duplicateFactoryBundle);

            // Deallocating a bundle takes the lock when _useUnsafeUnretainedTables returns true, so release the lock first.
            _CFMutexUnlock(&CFBundleGlobalDataLock);

            // Release the allocated instance
            CFRelease(bundle);

            // Release the duplicate factory bundle
            if (duplicateFactoryBundle) CFRelease(duplicateFactoryBundle);
            return NULL;            
        }
    }
    
    if (addToTables) {
        _CFBundleAddToTablesLocked(bundle, bundleID);
    }
    
    _CFMutexUnlock(&CFBundleGlobalDataLock);

    if (doFinalProcessing) {
        // Outside of the lock, handle dynamic registration
        _CFPlugInHandleDynamicRegistration(bundle);
        
    }

    return bundle;
}

CFBundleRef CFBundleCreate(CFAllocatorRef allocator, CFURLRef bundleURL) {
    if (NULL == bundleURL) return NULL;

    // _CFBundleCreate doesn't know about the main bundle, so we have to check that first. If the URL passed in is the same as the main bundle, then we'll need to return that bundle first.
    // Result will be nil if the bundleURL passed in happened to have been the main bundle.
    // As a fallback, check now to see if the main bundle URL is equal to bundleURL. If so, return that bundle instead of nil (32988858).
    CFBundleRef main = CFBundleGetMainBundle();
    if (main && main->_url && CFEqual(main->_url, bundleURL)) {
        CFRetain(main);
        return main;
    }

    return _CFBundleCreate(allocator, bundleURL, true, false, true);
}

CFBundleRef _CFBundleCreateUnique(CFAllocatorRef allocator, CFURLRef bundleURL) {
    // This function can never return an existing CFBundleRef object.
    return _CFBundleCreate(allocator, bundleURL, true, true, false);
}

CF_PRIVATE CFBundleRef _CFBundleCreateMain(CFAllocatorRef allocator, CFURLRef mainBundleURL) {
    // Do not add the main bundle to tables
    return _CFBundleCreate(allocator, mainBundleURL, false, false, false);
}

CFArrayRef CFBundleCreateBundlesFromDirectory(CFAllocatorRef alloc, CFURLRef directoryURL, CFStringRef bundleType) {
    CFMutableArrayRef bundles = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
    CFArrayRef URLs = _CFCreateContentsOfDirectory(alloc, NULL, NULL, directoryURL, bundleType);
    if (URLs) {
        CFIndex c = CFArrayGetCount(URLs);
        CFURLRef curURL;
        CFBundleRef curBundle;

        for (CFIndex i = 0; i < c; i++) {
            curURL = (CFURLRef)CFArrayGetValueAtIndex(URLs, i);
            curBundle = CFBundleCreate(alloc, curURL);
            if (curBundle) {
                CFArrayAppendValue(bundles, curBundle);
#ifdef __clang_analyzer__
                // The contract of this function is that the bundles created inside here are 'not released'.
                CFRelease(curBundle);
#endif
            }
        }
        CFRelease(URLs);
    }

    return bundles;
}

CFURLRef CFBundleCopyBundleURL(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    if (bundle->_url) CFRetain(bundle->_url);
    return bundle->_url;
}

UInt32 CFBundleGetVersionNumber(CFBundleRef bundle) {
    CFDictionaryRef infoDict = CFBundleGetInfoDictionary(bundle);
    CFNumberRef versionValue = (CFNumberRef)CFDictionaryGetValue(infoDict, _kCFBundleNumericVersionKey);
    if (!versionValue || CFGetTypeID(versionValue) != CFNumberGetTypeID()) return 0;
    
    UInt32 vers = 0;
    CFNumberGetValue(versionValue, kCFNumberSInt32Type, &vers);
    return vers;
}

CFStringRef CFBundleGetDevelopmentRegion(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    dispatch_once(&bundle->_developmentRegionCalculated, ^{
        CFStringRef devRegion = NULL;
        CFDictionaryRef infoDict = CFBundleGetInfoDictionary(bundle);
        if (infoDict) {
            devRegion = (CFStringRef)CFDictionaryGetValue(infoDict, kCFBundleDevelopmentRegionKey);
            if (devRegion && (CFGetTypeID(devRegion) != CFStringGetTypeID() || CFStringGetLength(devRegion) == 0)) {
                devRegion = NULL;
            }
        }
        
        if (devRegion) bundle->_developmentRegion = (CFStringRef)CFRetain(devRegion);
    });
    return bundle->_developmentRegion;
}

Boolean _CFBundleGetHasChanged(CFBundleRef bundle) {
    // This SPI isn't very useful, so now we just return true (30211007)
    return true;
}

void _CFBundleSetStringsFilesShared(CFBundleRef bundle, Boolean flag) {
    bundle->_sharesStringsFiles = flag;
}

Boolean _CFBundleGetStringsFilesShared(CFBundleRef bundle) {
    return bundle->_sharesStringsFiles;
}

CF_EXPORT CFURLRef CFBundleCopySupportFilesDirectoryURL(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    CFURLRef bundleURL = bundle->_url;
    _CFBundleVersion version = bundle->_version;
    CFURLRef result = NULL;
    if (bundleURL) {
        if (_CFBundleVersionOldStyleSupportFiles == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleSupportFilesURLFromBase1, bundleURL);
        } else if (_CFBundleVersionContentsResources == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleSupportFilesURLFromBase2, bundleURL);
        } else if (_CFBundleVersionWrappedContentsResources == version) {
            result = _CFURLCreateResolvedDirectoryWithString(kCFAllocatorSystemDefault, _CFBundleWrappedSupportFilesURLFromBase2, bundleURL);
        } else if (_CFBundleVersionWrappedFlat == version) {
            result = _CFURLCreateResolvedDirectoryWithString(kCFAllocatorSystemDefault, _CFBundleWrappedSupportFilesURLFromBase3, bundleURL);
        } else {
            result = (CFURLRef)CFRetain(bundleURL);
        }
    }
    return result;
}

CF_PRIVATE CFURLRef _CFBundleCopyResourcesDirectoryURLInDirectory(CFURLRef bundleURL, _CFBundleVersion version) {
    CFURLRef result = NULL;
    if (bundleURL) {
        if (_CFBundleVersionOldStyleResources == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleResourcesURLFromBase0, bundleURL);
        } else if (_CFBundleVersionOldStyleSupportFiles == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleResourcesURLFromBase1, bundleURL);
        } else if (_CFBundleVersionContentsResources == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleResourcesURLFromBase2, bundleURL);
        } else if (_CFBundleVersionWrappedContentsResources == version) {
            result = _CFURLCreateResolvedDirectoryWithString(kCFAllocatorSystemDefault, _CFBundleWrappedResourcesURLFromBase2, bundleURL);
        } else if (_CFBundleVersionWrappedFlat == version) {
            result = _CFURLCreateResolvedDirectoryWithString(kCFAllocatorSystemDefault, _CFBundleWrappedResourcesURLFromBase3, bundleURL);
        } else {
            result = (CFURLRef)CFRetain(bundleURL);
        }
    }
    return result;
}

CF_EXPORT CFURLRef CFBundleCopyResourcesDirectoryURL(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    return _CFBundleCopyResourcesDirectoryURLInDirectory(bundle->_url, bundle->_version);
}

CF_PRIVATE CFURLRef _CFBundleCopyAppStoreReceiptURLInDirectory(CFURLRef bundleURL, _CFBundleVersion version) {
    CFURLRef result = NULL;
    if (bundleURL) {
        if (_CFBundleVersionOldStyleResources == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleAppStoreReceiptURLFromBase0, bundleURL);
        } else if (_CFBundleVersionOldStyleSupportFiles == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleAppStoreReceiptURLFromBase1, bundleURL);
        } else if (_CFBundleVersionContentsResources == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleAppStoreReceiptURLFromBase2, bundleURL);
        } else if (_CFBundleVersionFlat == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleAppStoreReceiptURLFromBase0, bundleURL);
        } else if (_CFBundleVersionWrappedContentsResources == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleWrappedAppStoreReceiptURLFromBase2, bundleURL);
        } else if (_CFBundleVersionWrappedFlat == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleWrappedAppStoreReceiptURLFromBase3, bundleURL);
        }
    }
    return result;
}

CFURLRef _CFBundleCopyAppStoreReceiptURL(CFBundleRef bundle) {
    return _CFBundleCopyAppStoreReceiptURLInDirectory(bundle->_url, bundle->_version);
}

CF_EXPORT CFURLRef _CFBundleCopyWrappedBundleURL(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    _CFBundleVersion version = bundle->_version;
    if (version == _CFBundleVersionWrappedContentsResources || version == _CFBundleVersionWrappedFlat) {
        return _CFURLCreateResolvedDirectoryWithString(kCFAllocatorSystemDefault, _CFBundleWrapperLinkName, bundle->_url);
    } else {
        return NULL;
    }
}

CF_EXPORT CFURLRef _CFBundleCopyWrapperContainerURL(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    _CFBundleVersion version = bundle->_version;
    if (version == _CFBundleVersionWrappedContentsResources || version == _CFBundleVersionWrappedFlat) {
        // This is a directory, not a symlink
        return CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleWrapperDirectoryName, bundle->_url);
    } else {
        return NULL;
    }
}

CF_CROSS_PLATFORM_EXPORT CFStringRef _CFBundleCopyExecutablePath(CFBundleRef bundle) {
    return _CFBundleCopyExecutableName(bundle, NULL, NULL);
}

CF_PRIVATE CFStringRef _CFBundleCopyExecutableName(CFBundleRef bundle, CFURLRef url, CFDictionaryRef infoDict) {
    CFStringRef executableName = NULL;
    
    if (!infoDict && bundle) infoDict = CFBundleGetInfoDictionary(bundle);
    if (!url && bundle) url = bundle->_url;
    
    if (infoDict) {
        // Figure out the name of the executable.
        // First try for the new key in the plist.
        executableName = (CFStringRef)CFDictionaryGetValue(infoDict, kCFBundleExecutableKey);
        // Second try for the old key in the plist.
        if (!executableName) executableName = (CFStringRef)CFDictionaryGetValue(infoDict, _kCFBundleOldExecutableKey);
        if (executableName && CFGetTypeID(executableName) == CFStringGetTypeID() && CFStringGetLength(executableName) > 0) {
            CFRetain(executableName);
        } else {
            executableName = NULL;
        }
    }
    if (!executableName && url) {
        // Third, take the name of the bundle itself (with path extension stripped)
        CFURLRef absoluteURL = CFURLCopyAbsoluteURL(url);
        CFStringRef bundlePath = CFURLCopyFileSystemPath(absoluteURL, PLATFORM_PATH_STYLE);
        CFRelease(absoluteURL);
        if (bundlePath) {
            CFIndex len = CFStringGetLength(bundlePath);
            CFIndex startOfBundleName = _CFStartOfLastPathComponent2(bundlePath);
            CFIndex endOfBundleName = _CFLengthAfterDeletingPathExtension2(bundlePath);
            
            if (startOfBundleName <= len && endOfBundleName <= len && startOfBundleName < endOfBundleName) {
                executableName = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, bundlePath, CFRangeMake(startOfBundleName, endOfBundleName - startOfBundleName));
            }
            CFRelease(bundlePath);
        }
    }
    
    return executableName;
}

Boolean CFBundleIsExecutableLoaded(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    return bundle->_isLoaded;
}

CFBundleExecutableType CFBundleGetExecutableType(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    CFBundleExecutableType result = kCFBundleOtherExecutableType;
    CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);

    if (!executableURL) bundle->_binaryType = __CFBundleNoBinary;
#if defined(BINARY_SUPPORT_DYLD)
    if (bundle->_binaryType == __CFBundleUnknownBinary) {
        bundle->_binaryType = _CFBundleGrokBinaryType(executableURL);
        if (bundle->_binaryType != __CFBundleCFMBinary && bundle->_binaryType != __CFBundleUnreadableBinary) bundle->_resourceData._executableLacksResourceFork = true;
    }
#endif /* BINARY_SUPPORT_DYLD */
    if (executableURL) CFRelease(executableURL);

    if (bundle->_binaryType == __CFBundleCFMBinary) {
        result = kCFBundlePEFExecutableType;
    } else if (bundle->_binaryType == __CFBundleDYLDExecutableBinary || bundle->_binaryType == __CFBundleDYLDBundleBinary || bundle->_binaryType == __CFBundleDYLDFrameworkBinary) {
        result = kCFBundleMachOExecutableType;
    } else if (bundle->_binaryType == __CFBundleDLLBinary) {
        result = kCFBundleDLLExecutableType;
    } else if (bundle->_binaryType == __CFBundleELFBinary) {
        result = kCFBundleELFExecutableType;    
    }
    return result;
}

void _CFBundleSetCFMConnectionID(CFBundleRef bundle, void *connectionID) {
    bundle->_connectionCookie = connectionID;
    bundle->_isLoaded = true;
}

static CFStringRef _CFBundleCopyLastPathComponent(CFBundleRef bundle) {
    CFURLRef bundleURL = CFBundleCopyBundleURL(bundle);
    if (!bundleURL) {
        return CFSTR("<unknown>");
    }
    CFStringRef str = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
    UniChar buff[CFMaxPathSize];
    CFIndex buffLen = CFStringGetLength(str), startOfLastDir = 0;

    CFRelease(bundleURL);
    if (buffLen > CFMaxPathSize) buffLen = CFMaxPathSize;
    CFStringGetCharacters(str, CFRangeMake(0, buffLen), buff);
    CFRelease(str);
    if (buffLen > 0) startOfLastDir = _CFStartOfLastPathComponent(buff, buffLen);
    return CFStringCreateWithCharacters(kCFAllocatorSystemDefault, &(buff[startOfLastDir]), buffLen - startOfLastDir);
}

#pragma mark -

CF_PRIVATE CFErrorRef _CFBundleCreateErrorDebug(CFAllocatorRef allocator, CFBundleRef bundle, CFIndex code, CFStringRef debugString) {
    const void *userInfoKeys[6], *userInfoValues[6];
    CFIndex numKeys = 0;
    CFURLRef bundleURL = CFBundleCopyBundleURL(bundle), absoluteURL = CFURLCopyAbsoluteURL(bundleURL), executableURL = CFBundleCopyExecutableURL(bundle);
    CFBundleRef bdl = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CoreFoundation"));
    CFStringRef bundlePath = CFURLCopyFileSystemPath(absoluteURL, PLATFORM_PATH_STYLE), executablePath = executableURL ? CFURLCopyFileSystemPath(executableURL, PLATFORM_PATH_STYLE) : NULL, descFormat = NULL, desc = NULL, reason = NULL, suggestion = NULL;
    CFErrorRef error;
    if (bdl) {
        CFStringRef name = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(bundle, kCFBundleNameKey);
        name = name ? (CFStringRef)CFRetain(name) : _CFBundleCopyLastPathComponent(bundle);
        if (CFBundleExecutableNotFoundError == code) {
            descFormat = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr4"), CFSTR("Error"), bdl, CFSTR("The bundle \\U201c%@\\U201d couldn\\U2019t be loaded because its executable couldn\\U2019t be located."), "NSFileNoSuchFileError");
            reason = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr4-C"), CFSTR("Error"), bdl, CFSTR("The bundle\\U2019s executable couldn\\U2019t be located."), "NSFileNoSuchFileError");
            suggestion = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr4-R"), CFSTR("Error"), bdl, CFSTR("Try reinstalling the bundle."), "NSFileNoSuchFileError");
        } else if (CFBundleExecutableNotLoadableError == code) {
            descFormat = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3584"), CFSTR("Error"), bdl, CFSTR("The bundle \\U201c%@\\U201d couldn\\U2019t be loaded because its executable isn\\U2019t loadable."), "NSExecutableNotLoadableError");
            reason = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3584-C"), CFSTR("Error"), bdl, CFSTR("The bundle\\U2019s executable isn\\U2019t loadable."), "NSExecutableNotLoadableError");
            suggestion = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3584-R"), CFSTR("Error"), bdl, CFSTR("Try reinstalling the bundle."), "NSExecutableNotLoadableError");
        } else if (CFBundleExecutableArchitectureMismatchError == code) {
            descFormat = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3585"), CFSTR("Error"), bdl, CFSTR("The bundle \\U201c%@\\U201d couldn\\U2019t be loaded because it doesn\\U2019t contain a version for the current architecture."), "NSExecutableArchitectureMismatchError");
            reason = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3585-C"), CFSTR("Error"), bdl, CFSTR("The bundle doesn\\U2019t contain a version for the current architecture."), "NSExecutableArchitectureMismatchError");
            suggestion = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3585-R"), CFSTR("Error"), bdl, CFSTR("Try installing a universal version of the bundle."), "NSExecutableArchitectureMismatchError");
        } else if (CFBundleExecutableRuntimeMismatchError == code) {
            descFormat = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3586"), CFSTR("Error"), bdl, CFSTR("The bundle \\U201c%@\\U201d couldn\\U2019t be loaded because it isn\\U2019t compatible with the current application."), "NSExecutableRuntimeMismatchError");
            reason = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3586-C"), CFSTR("Error"), bdl, CFSTR("The bundle isn\\U2019t compatible with this application."), "NSExecutableRuntimeMismatchError");
            suggestion = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3586-R"), CFSTR("Error"), bdl, CFSTR("Try installing a newer version of the bundle."), "NSExecutableRuntimeMismatchError");
        } else if (CFBundleExecutableLoadError == code) {
            descFormat = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3587"), CFSTR("Error"), bdl, CFSTR("The bundle \\U201c%@\\U201d couldn\\U2019t be loaded because it is damaged or missing necessary resources."), "NSExecutableLoadError");
            reason = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3587-C"), CFSTR("Error"), bdl, CFSTR("The bundle is damaged or missing necessary resources."), "NSExecutableLoadError");
            suggestion = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3587-R"), CFSTR("Error"), bdl, CFSTR("Try reinstalling the bundle."), "NSExecutableLoadError");
        } else if (CFBundleExecutableLinkError == code) {
            descFormat = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3588"), CFSTR("Error"), bdl, CFSTR("The bundle \\U201c%@\\U201d couldn\\U2019t be loaded."), "NSExecutableLinkError");
            reason = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3588-C"), CFSTR("Error"), bdl, CFSTR("The bundle couldn\\U2019t be loaded."), "NSExecutableLinkError");
            suggestion = CFCopyLocalizedStringWithDefaultValue(CFSTR("BundleErr3588-R"), CFSTR("Error"), bdl, CFSTR("Try reinstalling the bundle."), "NSExecutableLinkError");
        }
        if (descFormat) {
            desc = CFStringCreateWithFormat(allocator, NULL, descFormat, name);
            CFRelease(descFormat);
        }
        CFRelease(name);
    }
    if (bundlePath) {
        userInfoKeys[numKeys] = CFSTR("NSBundlePath");
        userInfoValues[numKeys] = bundlePath;
        numKeys++;
    }
    if (executablePath) {
        userInfoKeys[numKeys] = CFSTR("NSFilePath");
        userInfoValues[numKeys] = executablePath;
        numKeys++;
    }
    if (desc) {
        userInfoKeys[numKeys] = kCFErrorLocalizedDescriptionKey;
        userInfoValues[numKeys] = desc;
        numKeys++;
    }
    if (reason) {
        userInfoKeys[numKeys] = kCFErrorLocalizedFailureReasonKey;
        userInfoValues[numKeys] = reason;
        numKeys++;
    }
    if (suggestion) {
        userInfoKeys[numKeys] = kCFErrorLocalizedRecoverySuggestionKey;
        userInfoValues[numKeys] = suggestion;
        numKeys++;
    }
    if (debugString) {
        userInfoKeys[numKeys] = kCFErrorDebugDescriptionKey;
        userInfoValues[numKeys] = debugString;
        numKeys++;
    }
    error = CFErrorCreateWithUserInfoKeysAndValues(allocator, kCFErrorDomainCocoa, code, userInfoKeys, userInfoValues, numKeys);
    if (bundleURL) CFRelease(bundleURL);
    if (absoluteURL) CFRelease(absoluteURL);
    if (executableURL) CFRelease(executableURL);
    if (bundlePath) CFRelease(bundlePath);
    if (executablePath) CFRelease(executablePath);
    if (desc) CFRelease(desc);
    if (reason) CFRelease(reason);
    if (suggestion) CFRelease(suggestion);
    return error;
}

CFErrorRef _CFBundleCreateError(CFAllocatorRef allocator, CFBundleRef bundle, CFIndex code) {
    return _CFBundleCreateErrorDebug(allocator, bundle, code, NULL);
}

#pragma mark -

Boolean _CFBundleLoadExecutableAndReturnError(CFBundleRef bundle, Boolean forceGlobal, CFErrorRef *error) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    Boolean result = false;
    CFErrorRef localError = NULL, *subError = (error ? &localError : NULL);
    CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);


    _CFMutexLock(&(bundle->_bundleLoadingLock));
    if (!executableURL) bundle->_binaryType = __CFBundleNoBinary;
    // make sure we know whether bundle is already loaded or not
#if defined(BINARY_SUPPORT_DLFCN)
    if (!bundle->_isLoaded) _CFBundleDlfcnCheckLoaded(bundle);
#elif defined(BINARY_SUPPORT_DYLD)
    if (!bundle->_isLoaded) _CFBundleDYLDCheckLoaded(bundle);
#endif /* BINARY_SUPPORT_DLFCN */
#if defined(BINARY_SUPPORT_DYLD)
    // We might need to figure out what it is
    if (bundle->_binaryType == __CFBundleUnknownBinary) {
        bundle->_binaryType = _CFBundleGrokBinaryType(executableURL);
        if (bundle->_binaryType != __CFBundleCFMBinary && bundle->_binaryType != __CFBundleUnreadableBinary) bundle->_resourceData._executableLacksResourceFork = true;
    }
#endif /* BINARY_SUPPORT_DYLD */
    if (executableURL) CFRelease(executableURL);
    
    if (bundle->_isLoaded) {
        _CFMutexUnlock(&(bundle->_bundleLoadingLock));
        _CFPlugInUnscheduleForUnloading(bundle);
        return true;
    }
    
    // n.b. some applications may call API like CFBundleGetBundleWithIdentifier during library initialization. This could wind up recursively taking this lock. Therefore, even though it results in the potential for a race on the internals of bundle's data structures, we have to unlock before calling out to dlopen. rdar://67319441
    _CFMutexUnlock(&(bundle->_bundleLoadingLock));
    
    switch (bundle->_binaryType) {
#if defined(BINARY_SUPPORT_DLFCN)
        case __CFBundleUnreadableBinary:
            result = _CFBundleDlfcnLoadBundle(bundle, forceGlobal, subError);
            break;
#endif /* BINARY_SUPPORT_DLFCN */
#if defined(BINARY_SUPPORT_DYLD)
        case __CFBundleDYLDBundleBinary:
#if defined(BINARY_SUPPORT_DLFCN)
            result = _CFBundleDlfcnLoadBundle(bundle, forceGlobal, subError);
#else /* BINARY_SUPPORT_DLFCN */
            result = _CFBundleDYLDLoadBundle(bundle, forceGlobal, subError);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
        case __CFBundleDYLDFrameworkBinary:
#if defined(BINARY_SUPPORT_DLFCN)
            result = _CFBundleDlfcnLoadFramework(bundle, subError);
#else /* BINARY_SUPPORT_DLFCN */
            result = _CFBundleDYLDLoadFramework(bundle, subError);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
        case __CFBundleDYLDExecutableBinary:
            if (error) {
                localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotLoadableError);
            } else {
                CFLog(__kCFLogBundle, CFSTR("Attempt to load executable of a type that cannot be dynamically loaded for %@"), bundle);
            }
            break;
#endif /* BINARY_SUPPORT_DYLD */
#if defined(BINARY_SUPPORT_DLFCN)
        case __CFBundleUnknownBinary:
        case __CFBundleELFBinary:
            result = _CFBundleDlfcnLoadBundle(bundle, forceGlobal, subError);
            break;
#endif /* BINARY_SUPPORT_DLFCN */
#if defined(BINARY_SUPPORT_DLL)
        case __CFBundleDLLBinary:
            result = _CFBundleDLLLoad(bundle, subError);
            break;
#endif /* BINARY_SUPPORT_DLL */
        case __CFBundleNoBinary:
            if (error) {
                localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotFoundError);
            } else {
                CFLog(__kCFLogBundle, CFSTR("Cannot find executable for %@"), bundle);
            }
            break;     
        default:
            if (error) {
                localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotLoadableError);
            } else {
                CFLog(__kCFLogBundle, CFSTR("Cannot recognize type of executable for %@"), bundle);
            }
            break;
    }
    
    if (result && bundle->_plugInData._isPlugIn) {
        _CFPlugInHandleDynamicRegistration(bundle);
    }
    if (!result && error) *error = localError;
    return result;
}

Boolean CFBundleLoadExecutableAndReturnError(CFBundleRef bundle, CFErrorRef *error) {
    return _CFBundleLoadExecutableAndReturnError(bundle, false, error);
}

Boolean CFBundleLoadExecutable(CFBundleRef bundle) {
    return _CFBundleLoadExecutableAndReturnError(bundle, false, NULL);
}

Boolean CFBundlePreflightExecutable(CFBundleRef bundle, CFErrorRef *error) {
    Boolean result = false;
    CFErrorRef localError = NULL;
#if defined(BINARY_SUPPORT_DLFCN)
    CFErrorRef *subError = (error ? &localError : NULL);
#endif
    CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);

    _CFMutexLock(&(bundle->_bundleLoadingLock));
    if (!executableURL) bundle->_binaryType = __CFBundleNoBinary;
    // make sure we know whether bundle is already loaded or not
#if defined(BINARY_SUPPORT_DLFCN)
    if (!bundle->_isLoaded) _CFBundleDlfcnCheckLoaded(bundle);
#elif defined(BINARY_SUPPORT_DYLD)
    if (!bundle->_isLoaded) _CFBundleDYLDCheckLoaded(bundle);
#endif /* BINARY_SUPPORT_DLFCN */
#if defined(BINARY_SUPPORT_DYLD)
    // We might need to figure out what it is
    if (bundle->_binaryType == __CFBundleUnknownBinary) {
        bundle->_binaryType = _CFBundleGrokBinaryType(executableURL);
        if (bundle->_binaryType != __CFBundleCFMBinary && bundle->_binaryType != __CFBundleUnreadableBinary) bundle->_resourceData._executableLacksResourceFork = true;
    }
#endif /* BINARY_SUPPORT_DYLD */
    if (executableURL) CFRelease(executableURL);
    
    if (bundle->_isLoaded) {
        _CFMutexUnlock(&(bundle->_bundleLoadingLock));
        return true;
    }
    _CFMutexUnlock(&(bundle->_bundleLoadingLock));
    
    switch (bundle->_binaryType) {
#if defined(BINARY_SUPPORT_DLFCN)
        case __CFBundleUnreadableBinary:
            result = _CFBundleDlfcnPreflight(bundle, subError);
            break;
#endif /* BINARY_SUPPORT_DLFCN */
#if defined(BINARY_SUPPORT_DYLD)
        case __CFBundleDYLDBundleBinary:
            result = true;
#if defined(BINARY_SUPPORT_DLFCN)
            result = _CFBundleDlfcnPreflight(bundle, subError);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
        case __CFBundleDYLDFrameworkBinary:
            result = true;
#if defined(BINARY_SUPPORT_DLFCN)
            result = _CFBundleDlfcnPreflight(bundle, subError);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
        case __CFBundleDYLDExecutableBinary:
            if (error) localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotLoadableError);
            break;
#endif /* BINARY_SUPPORT_DYLD */
#if defined(BINARY_SUPPORT_DLFCN)
        case __CFBundleUnknownBinary:
        case __CFBundleELFBinary:
            result = _CFBundleDlfcnPreflight(bundle, subError);
            break;
#endif /* BINARY_SUPPORT_DLFCN */
#if defined(BINARY_SUPPORT_DLL)
        case __CFBundleDLLBinary:
            result = true;
            break;
#endif /* BINARY_SUPPORT_DLL */
        case __CFBundleNoBinary:
            if (error) localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotFoundError);
            break;     
        default:
            if (error) localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotLoadableError);
            break;
    }
    if (!result && error) *error = localError;
    return result;
}

CFArrayRef CFBundleCopyExecutableArchitectures(CFBundleRef bundle) {
    CFArrayRef result = NULL;
    CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
    if (executableURL) {
        result = _CFBundleCopyArchitecturesForExecutable(executableURL);
        CFRelease(executableURL);
    }
    return result;
}

void CFBundleUnloadExecutable(CFBundleRef bundle) {
    _CFBundleUnloadExecutable(bundle, false);
}

CF_PRIVATE void _CFBundleUnloadExecutable(CFBundleRef bundle, Boolean unloadingPlugins) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    // First unload bundles scheduled for unloading (if that's not what we are already doing.)
    if (!unloadingPlugins) _CFPlugInUnloadScheduledPlugIns();
    
    if (!bundle->_isLoaded) return;

    // Remove from the scheduled unload set if we are there.
    _CFPlugInUnscheduleForUnloading(bundle);
    
    // Give the plugIn code a chance to realize this...
    _CFPlugInWillUnload(bundle);

    _CFMutexLock(&(bundle->_bundleLoadingLock));
    if (!bundle->_isLoaded) {
        _CFMutexUnlock(&(bundle->_bundleLoadingLock));
        return;
    }

    switch (bundle->_binaryType) {
#if defined(BINARY_SUPPORT_DYLD)
        case __CFBundleDYLDBundleBinary:
#if defined(BINARY_SUPPORT_DLFCN)
            if (bundle->_handleCookie) _CFBundleDlfcnUnload(bundle);
#else /* BINARY_SUPPORT_DLFCN */
            _CFBundleDYLDUnloadBundle(bundle);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
        case __CFBundleDYLDFrameworkBinary:
#if defined(BINARY_SUPPORT_DLFCN)
            if (bundle->_handleCookie && _CFExecutableLinkedOnOrAfter(CFSystemVersionLeopard)) _CFBundleDlfcnUnload(bundle);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
#endif /* BINARY_SUPPORT_DYLD */
#if defined(BINARY_SUPPORT_DLL)
        case __CFBundleDLLBinary:
            _CFBundleDLLUnload(bundle);
            break;
#endif /* BINARY_SUPPORT_DLL */
        default:
#if defined(BINARY_SUPPORT_DLFCN)
            if (bundle->_handleCookie) _CFBundleDlfcnUnload(bundle);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
    }
    
    _CFMutexUnlock(&(bundle->_bundleLoadingLock));
}

#pragma mark -

CF_PRIVATE _CFResourceData *__CFBundleGetResourceData(CFBundleRef bundle) {
    return &(bundle->_resourceData);
}

CFPlugInRef CFBundleGetPlugIn(CFBundleRef bundle) {
    return (bundle->_plugInData._isPlugIn) ? (CFPlugInRef)bundle : NULL;
}

CF_PRIVATE _CFPlugInData *__CFBundleGetPlugInData(CFBundleRef bundle) {
    return &(bundle->_plugInData);
}

CF_PRIVATE Boolean _CFBundleCouldBeBundle(CFURLRef url) {
    Boolean result = false;
    Boolean exists;
    SInt32 mode;
    if (_CFGetFileProperties(kCFAllocatorSystemDefault, url, &exists, &mode, NULL, NULL, NULL, NULL) == 0) result = (exists && (mode & S_IFMT) == S_IFDIR && (mode & 0444) != 0);
    return result;
}

#define LENGTH_OF(A) (sizeof(A) / sizeof(A[0]))
        
//If 'permissive' is set, we will maintain the historical behavior of returning frameworks with names that don't match, and frameworks for executables in Resources/
static CFURLRef __CFBundleCopyFrameworkURLForExecutablePath(CFStringRef executablePath, Boolean permissive) {
    // MF:!!! Implement me.  We need to be able to find the bundle from the exe, dealing with old vs. new as well as the Executables dir business on Windows.
#if TARGET_OS_WIN32
    UniChar executablesToFrameworksPathBuff[] = {'.', '.', '\\', 'F', 'r', 'a', 'm', 'e', 'w', 'o', 'r', 'k', 's'};
    UniChar executablesToPrivateFrameworksPathBuff[] = {'.', '.', '\\', 'P', 'r', 'i', 'v', 'a', 't', 'e', 'F', 'r', 'a', 'm', 'e', 'w', 'o', 'r', 'k', 's'};
    UniChar frameworksExtension[] = {'f', 'r', 'a', 'm', 'e', 'w', 'o', 'r', 'k'};
#endif
    UniChar pathBuff[CFMaxPathSize] = {0};
    UniChar nameBuff[CFMaxPathSize] = {0};
    CFIndex length, nameStart, nameLength, savedLength;
    CFMutableStringRef cheapStr = CFStringCreateMutableWithExternalCharactersNoCopy(kCFAllocatorSystemDefault, NULL, 0, 0, NULL);
    CFURLRef bundleURL = NULL;
    
    length = CFStringGetLength(executablePath);
    if (length > CFMaxPathSize) length = CFMaxPathSize;
    CFStringGetCharacters(executablePath, CFRangeMake(0, length), pathBuff);

    // Save the name in nameBuff
    length = _CFLengthAfterDeletingPathExtension(pathBuff, length);
    nameStart = _CFStartOfLastPathComponent(pathBuff, length);
    nameLength = length - nameStart;
    memmove(nameBuff, &(pathBuff[nameStart]), nameLength * sizeof(UniChar));

    // Strip the name from pathBuff
    length = _CFLengthAfterDeletingLastPathComponent(pathBuff, length);
    savedLength = length;

#if TARGET_OS_WIN32
    // * (Windows-only) First check the "Executables" directory parallel to the "Frameworks" directory case.
    if (_CFAppendPathComponent(pathBuff, &length, CFMaxPathSize, executablesToFrameworksPathBuff, LENGTH_OF(executablesToFrameworksPathBuff)) && _CFAppendPathComponent(pathBuff, &length, CFMaxPathSize, nameBuff, nameLength) && _CFAppendPathExtension(pathBuff, &length, CFMaxPathSize, frameworksExtension, LENGTH_OF(frameworksExtension))) {
        CFStringSetExternalCharactersNoCopy(cheapStr, pathBuff, length, CFMaxPathSize);
        bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, cheapStr, PLATFORM_PATH_STYLE, true);
        if (!_CFBundleCouldBeBundle(bundleURL)) {
            CFRelease(bundleURL);
            bundleURL = NULL;
        }
    }
    // * (Windows-only) Next check the "Executables" directory parallel to the "PrivateFrameworks" directory case.
    if (!bundleURL) {
        length = savedLength;
        if (_CFAppendPathComponent(pathBuff, &length, CFMaxPathSize, executablesToPrivateFrameworksPathBuff, LENGTH_OF(executablesToPrivateFrameworksPathBuff)) && _CFAppendPathComponent(pathBuff, &length, CFMaxPathSize, nameBuff, nameLength) && _CFAppendPathExtension(pathBuff, &length, CFMaxPathSize, frameworksExtension, LENGTH_OF(frameworksExtension))) {
            CFStringSetExternalCharactersNoCopy(cheapStr, pathBuff, length, CFMaxPathSize);
            bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, cheapStr, PLATFORM_PATH_STYLE, true);
            if (!_CFBundleCouldBeBundle(bundleURL)) {
                CFRelease(bundleURL);
                bundleURL = NULL;
            }
        }
    }
#endif
    // * Finally check the executable inside the framework case.
    if (!bundleURL) {        
        length = savedLength;
        // To catch all the cases, we just peel off level looking for one ending in .framework or one called "Supporting Files".
        
        CFStringRef name = permissive ? CFSTR("") : CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, (const char *)nameBuff);
        
        while (length > 0) {
            CFIndex curStart = _CFStartOfLastPathComponent(pathBuff, length);
            if (curStart >= length) break;
            CFStringSetExternalCharactersNoCopy(cheapStr, &(pathBuff[curStart]), length - curStart, CFMaxPathSize - curStart);
            if (!permissive && CFEqual(cheapStr, _CFBundleResourcesDirectoryName)) break;
            if (CFEqual(cheapStr, _CFBundleSupportFilesDirectoryName1) || CFEqual(cheapStr, _CFBundleSupportFilesDirectoryName2)) {
                if (!permissive) {
                    CFIndex fmwkStart = _CFStartOfLastPathComponent(pathBuff, length);
                    CFStringSetExternalCharactersNoCopy(cheapStr, &(pathBuff[fmwkStart]), length - fmwkStart, CFMaxPathSize - fmwkStart);
                }
                if (permissive || CFStringHasPrefix(cheapStr, name)) {
                    length = _CFLengthAfterDeletingLastPathComponent(pathBuff, length);
                    CFStringSetExternalCharactersNoCopy(cheapStr, pathBuff, length, CFMaxPathSize);
                    
                    bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, cheapStr, PLATFORM_PATH_STYLE, true);
                    if (!_CFBundleCouldBeBundle(bundleURL)) {
                        CFRelease(bundleURL);
                        bundleURL = NULL;
                    }
                    break;
                }
            } else if (CFStringHasSuffix(cheapStr, CFSTR(".framework")) && (permissive || CFStringHasPrefix(cheapStr, name))) {
                CFStringSetExternalCharactersNoCopy(cheapStr, pathBuff, length, CFMaxPathSize);
                bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, cheapStr, PLATFORM_PATH_STYLE, true);
                if (!_CFBundleCouldBeBundle(bundleURL)) {
                    CFRelease(bundleURL);
                    bundleURL = NULL;
                }
                break;
            }
            length = _CFLengthAfterDeletingLastPathComponent(pathBuff, length);
        }
        if (!permissive) CFRelease(name);
    }
    CFStringSetExternalCharactersNoCopy(cheapStr, NULL, 0, 0);
    CFRelease(cheapStr);

    return bundleURL;
}
        
//SPI version; separated out to minimize linkage changes
CFURLRef _CFBundleCopyFrameworkURLForExecutablePath(CFStringRef executablePath) {
    return __CFBundleCopyFrameworkURLForExecutablePath(executablePath, false);
}

static void _CFBundleEnsureBundleExistsForImagePath(CFStringRef imagePath, Boolean permissive) {
    // This finds the bundle for the given path.
    // If an image path corresponds to a bundle, we see if there is already a bundle instance.  If there is and it is NOT in the _dynamicBundles array, it is added to the staticBundles.  Do not add the main bundle to the list here.
    CFBundleRef bundle;
    CFURLRef curURL = __CFBundleCopyFrameworkURLForExecutablePath(imagePath, permissive);

    if (curURL) {
        // Ensure bundle exists by creating it if necessary. This will check the tables as a first step.
        // NB doFinalProcessing must be false here, see below
        bundle = _CFBundleCreate(kCFAllocatorSystemDefault, curURL, false, false, true);
        if (bundle) {
            _CFMutexLock(&(bundle->_bundleLoadingLock));
            if (!bundle->_isLoaded) {
                // make sure that these bundles listed as loaded, and mark them frameworks (we probably can't see anything else here, and we cannot unload them)
    #if defined(BINARY_SUPPORT_DLFCN)
                if (!bundle->_isLoaded) _CFBundleDlfcnCheckLoaded(bundle);
    #elif defined(BINARY_SUPPORT_DYLD)
                if (!bundle->_isLoaded) _CFBundleDYLDCheckLoaded(bundle);
    #endif /* BINARY_SUPPORT_DLFCN */
    #if defined(BINARY_SUPPORT_DYLD)
                if (bundle->_binaryType == __CFBundleUnknownBinary) bundle->_binaryType = __CFBundleDYLDFrameworkBinary;
                if (bundle->_binaryType != __CFBundleCFMBinary && bundle->_binaryType != __CFBundleUnreadableBinary) bundle->_resourceData._executableLacksResourceFork = true;
    #endif /* BINARY_SUPPORT_DYLD */
    #if LOG_BUNDLE_LOAD
                if (!bundle->_isLoaded) printf("ensure bundle %p set loaded fallback, handle %p image %p conn %p\n", bundle, bundle->_handleCookie, bundle->_imageCookie, bundle->_connectionCookie);
    #endif /* LOG_BUNDLE_LOAD */
                bundle->_isLoaded = true;
            }
            _CFMutexUnlock(&(bundle->_bundleLoadingLock));
            // Perform delayed final processing steps.
            // This must be done after _isLoaded has been set, for security reasons (3624341).
            CFDictionaryRef infoDict = CFBundleGetInfoDictionary(bundle);
            _CFBundleInitPlugIn(bundle, infoDict, NULL);
            _CFPlugInHandleDynamicRegistration(bundle);
        }
        CFRelease(curURL);
    }
}

static void _CFBundleEnsureBundlesExistForImagePaths(CFArrayRef imagePaths) {
    // This finds the bundles for the given paths.
    // If an image path corresponds to a bundle, we see if there is already a bundle instance.  If there is and it is NOT in the _dynamicBundles array, it is added to the staticBundles.  Do not add the main bundle to the list here (even if it appears in imagePaths).
    CFIndex i, imagePathCount = CFArrayGetCount(imagePaths);
    for (i = 0; i < imagePathCount; i++) _CFBundleEnsureBundleExistsForImagePath((CFStringRef)CFArrayGetValueAtIndex(imagePaths, i), true);
}

static void _CFBundleEnsureBundlesUpToDateWithHint(CFStringRef hint) {
    CFArrayRef imagePaths = NULL;
    // Tickle the main bundle into existence
    (void)CFBundleGetMainBundle();
#if defined(BINARY_SUPPORT_DYLD)
    imagePaths = _CFBundleDYLDCopyLoadedImagePathsForHint(hint);
#endif /* BINARY_SUPPORT_DYLD */
    if (imagePaths) {
        _CFBundleEnsureBundlesExistForImagePaths(imagePaths);
        CFRelease(imagePaths);
    }
}

CFBundleRef _CFBundleGetBundleWithIdentifierAndLibraryName(CFStringRef bundleID, CFStringRef libraryName) {
    if (libraryName) {
        _CFBundleEnsureBundlesUpToDateWithHint(libraryName);
    }
    return _CFBundleGetFromTables(bundleID);
}

static void _CFBundleEnsureAllBundlesUpToDate(void) {
    // This method returns all the statically linked bundles.  This includes the main bundle as well as any frameworks that the process was linked against at launch time.  It does not include frameworks or opther bundles that were loaded dynamically.
    CFArrayRef imagePaths = NULL;
    // Tickle the main bundle into existence
    (void)CFBundleGetMainBundle();

#if defined(BINARY_SUPPORT_DLL)
// Don't know how to find static bundles for DLLs
#endif /* BINARY_SUPPORT_DLL */

#if defined(BINARY_SUPPORT_DYLD)
    imagePaths = _CFBundleDYLDCopyLoadedImagePathsIfChanged();
#endif /* BINARY_SUPPORT_DYLD */
    if (imagePaths) {
        _CFBundleEnsureBundlesExistForImagePaths(imagePaths);
        CFRelease(imagePaths);
    }
}

CFArrayRef CFBundleGetAllBundles(void) {
    // This API is fundamentally broken from a thread safety point of view. To mitigate the issues, we keep around the last list we handed out. If the list of allBundles changed, we leak the last one and return a new copy. If no bundle loading is done this list would be static.
    // Fortunately this method is rarely used.
    CFArrayRef result = NULL;
    _CFMutexLock(&CFBundleGlobalDataLock);
    static CFArrayRef _lastBundleList = NULL;
    if (!_lastBundleList) {
        // This is the first time we've been asked for a list of all bundles
        // Unlock the global lock. CopyAllBundles will use it.
        _CFMutexUnlock(&CFBundleGlobalDataLock);
        result = _CFBundleCopyAllBundles();
        _CFMutexLock(&CFBundleGlobalDataLock);
        if (_lastBundleList) {
            // Another thread beat us here
            CFRelease(result);
        } else {
            _lastBundleList = result;
        }
    } else if (!CFEqual(_lastBundleList, _allBundles)) {
        // Check if the list of bundles has changed
        _CFMutexUnlock(&CFBundleGlobalDataLock);
        result = _CFBundleCopyAllBundles();
        _CFMutexLock(&CFBundleGlobalDataLock);
        // note: intentionally leak the last value in _lastBundleList, due to API contract of 'get'
        _lastBundleList = result;
    }
    result = _lastBundleList;
    _CFMutexUnlock(&CFBundleGlobalDataLock);
    return result;
}
        
CF_EXPORT CFArrayRef _CFBundleCopyAllBundles(void) {
    // To answer this properly, we have to have created the static bundles!
    _CFBundleEnsureAllBundlesUpToDate();
    CFBundleRef main = CFBundleGetMainBundle();
    _CFMutexLock(&CFBundleGlobalDataLock);
    // _allBundles does not include the main bundle, so insert it here.
    CFMutableArrayRef bundles = CFArrayCreateMutableCopy(kCFAllocatorSystemDefault, CFArrayGetCount(_allBundles) + 1, _allBundles);
    _CFMutexUnlock(&CFBundleGlobalDataLock);
    CFArrayInsertValueAtIndex(bundles, 0, main);
    return bundles;
}

CF_PRIVATE _CFBundleVersion _CFBundleLayoutVersion(CFBundleRef bundle) {
    return bundle->_version;
}
     
CF_EXPORT CFURLRef _CFBundleCopyPrivateFrameworksURL(CFBundleRef bundle) {
    return CFBundleCopyPrivateFrameworksURL(bundle);
}

CF_EXPORT CFURLRef CFBundleCopyPrivateFrameworksURL(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    CFURLRef result = NULL;

    if (_CFBundleVersionOldStyleSupportFiles == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundlePrivateFrameworksURLFromBase1, bundle->_url);
    } else if (_CFBundleVersionContentsResources == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundlePrivateFrameworksURLFromBase2, bundle->_url);
    } else if (_CFBundleVersionWrappedContentsResources == bundle->_version) {
        result = _CFURLCreateResolvedDirectoryWithString(CFGetAllocator(bundle), _CFBundleWrappedPrivateFrameworksURLFromBase2, bundle->_url);
    } else if (_CFBundleVersionWrappedFlat == bundle->_version) {
        result = _CFURLCreateResolvedDirectoryWithString(CFGetAllocator(bundle), _CFBundleWrappedPrivateFrameworksURLFromBase3, bundle->_url);
    } else {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundlePrivateFrameworksURLFromBase0, bundle->_url);
    }
    return result;
}

CF_EXPORT CFURLRef _CFBundleCopySharedFrameworksURL(CFBundleRef bundle) {
    return CFBundleCopySharedFrameworksURL(bundle);
}

CF_EXPORT CFURLRef CFBundleCopySharedFrameworksURL(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    CFURLRef result = NULL;

    if (_CFBundleVersionOldStyleSupportFiles == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedFrameworksURLFromBase1, bundle->_url);
    } else if (_CFBundleVersionContentsResources == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedFrameworksURLFromBase2, bundle->_url);
    } else if (_CFBundleVersionWrappedContentsResources == bundle->_version) {
        result = _CFURLCreateResolvedDirectoryWithString(CFGetAllocator(bundle), _CFBundleWrappedSharedFrameworksURLFromBase2, bundle->_url);
    } else if (_CFBundleVersionWrappedFlat == bundle->_version) {
        result = _CFURLCreateResolvedDirectoryWithString(CFGetAllocator(bundle), _CFBundleWrappedSharedFrameworksURLFromBase3, bundle->_url);
    } else {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedFrameworksURLFromBase0, bundle->_url);
    }
    return result;
}

CF_EXPORT CFURLRef _CFBundleCopySharedSupportURL(CFBundleRef bundle) {
    return CFBundleCopySharedSupportURL(bundle);
}

CF_EXPORT CFURLRef CFBundleCopySharedSupportURL(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    CFURLRef result = NULL;

    if (_CFBundleVersionOldStyleSupportFiles == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedSupportURLFromBase1, bundle->_url);
    } else if (_CFBundleVersionContentsResources == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedSupportURLFromBase2, bundle->_url);
    } else if (_CFBundleVersionWrappedContentsResources == bundle->_version) {
        result = _CFURLCreateResolvedDirectoryWithString(CFGetAllocator(bundle), _CFBundleWrappedSharedSupportURLFromBase2, bundle->_url);
    } else if (_CFBundleVersionWrappedFlat == bundle->_version) {
        result = _CFURLCreateResolvedDirectoryWithString(CFGetAllocator(bundle), _CFBundleWrappedSharedSupportURLFromBase3, bundle->_url);
    } else {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedSupportURLFromBase0, bundle->_url);
    }
    return result;
}

CF_PRIVATE CFURLRef _CFBundleCopyBuiltInPlugInsURL(CFBundleRef bundle) {
    return CFBundleCopyBuiltInPlugInsURL(bundle);
}

CF_EXPORT CFURLRef CFBundleCopyBuiltInPlugInsURL(CFBundleRef bundle) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFBundle, bundle);
    CFURLRef result = NULL, alternateResult = NULL;

    CFAllocatorRef alloc = CFGetAllocator(bundle);
    if (_CFBundleVersionOldStyleSupportFiles == bundle->_version) {
        result = CFURLCreateWithString(alloc, _CFBundleBuiltInPlugInsURLFromBase1, bundle->_url);
    } else if (_CFBundleVersionContentsResources == bundle->_version) {
        result = CFURLCreateWithString(alloc, _CFBundleBuiltInPlugInsURLFromBase2, bundle->_url);
    } else if (_CFBundleVersionWrappedContentsResources == bundle->_version) {
        result = _CFURLCreateResolvedDirectoryWithString(CFGetAllocator(bundle), _CFBundleWrappedBuiltInPlugInsURLFromBase2, bundle->_url);
    } else if (_CFBundleVersionWrappedFlat == bundle->_version) {
        result = _CFURLCreateResolvedDirectoryWithString(CFGetAllocator(bundle), _CFBundleWrappedBuiltInPlugInsURLFromBase3, bundle->_url);
    } else {
        result = CFURLCreateWithString(alloc, _CFBundleBuiltInPlugInsURLFromBase0, bundle->_url);
    }
    if (!result || !_CFURLExists(result)) {
        if (_CFBundleVersionOldStyleSupportFiles == bundle->_version) {
            alternateResult = CFURLCreateWithString(alloc, _CFBundleAlternateBuiltInPlugInsURLFromBase1, bundle->_url);
        } else if (_CFBundleVersionContentsResources == bundle->_version) {
            alternateResult = CFURLCreateWithString(alloc, _CFBundleAlternateBuiltInPlugInsURLFromBase2, bundle->_url);
        } else if (_CFBundleVersionWrappedContentsResources == bundle->_version) {
            alternateResult = _CFURLCreateResolvedDirectoryWithString(CFGetAllocator(bundle), _CFBundleWrappedAlternateBuiltInPlugInsURLFromBase2, bundle->_url);
        } else if (_CFBundleVersionWrappedFlat == bundle->_version) {
            alternateResult = _CFURLCreateResolvedDirectoryWithString(CFGetAllocator(bundle), _CFBundleWrappedAlternateBuiltInPlugInsURLFromBase3, bundle->_url);
        } else {
            alternateResult = CFURLCreateWithString(alloc, _CFBundleAlternateBuiltInPlugInsURLFromBase0, bundle->_url);
        }
        if (alternateResult && _CFURLExists(alternateResult)) {
            if (result) CFRelease(result);
            result = alternateResult;
        } else {
            if (alternateResult) CFRelease(alternateResult);
        }
    }
    return result;
}

