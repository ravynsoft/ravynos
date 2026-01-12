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

/*      CFBundle.c
        Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
        Responsibility: Tony Parker
*/

#include "CFBundle_Internal.h"
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFSet.h>
#include <CoreFoundation/CFURLAccess.h>
#include <CoreFoundation/CFError.h>
#include <string.h>
#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"
#include <CoreFoundation/CFByteOrder.h>
#include "CFBundle_BinaryTypes.h"
#include <ctype.h>
#include <sys/stat.h>
#include <stdlib.h>

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_WINDOWS
#else
#error Unknown deployment target
#endif

#define AVOID_WEAK_COLLECTIONS 1

#if !AVOID_WEAK_COLLECTIONS
#include "CFHashTable.h"
#include "CFMapTable.h"
#include "CFPointerArray.h"
#endif /* !AVOID_WEAK_COLLECTIONS */

#if defined(BINARY_SUPPORT_DYLD)
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <crt_externs.h>
#endif /* BINARY_SUPPORT_DYLD */

#if defined(BINARY_SUPPORT_DLFCN)
#include <dlfcn.h>
#endif /* BINARY_SUPPORT_DLFCN */

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#include <fcntl.h>
#elif DEPLOYMENT_TARGET_WINDOWS
#include <fcntl.h>
#include <io.h>
#endif


static void _CFBundleFlushBundleCachesAlreadyLocked(CFBundleRef bundle, Boolean alreadyLocked);

#define LOG_BUNDLE_LOAD 0

// Public CFBundle Info plist keys
CONST_STRING_DECL(kCFBundleInfoDictionaryVersionKey, "CFBundleInfoDictionaryVersion")
CONST_STRING_DECL(kCFBundleExecutableKey, "CFBundleExecutable")
CONST_STRING_DECL(kCFBundleIdentifierKey, "CFBundleIdentifier")
CONST_STRING_DECL(kCFBundleVersionKey, "CFBundleVersion")
CONST_STRING_DECL(kCFBundleDevelopmentRegionKey, "CFBundleDevelopmentRegion")
CONST_STRING_DECL(kCFBundleLocalizationsKey, "CFBundleLocalizations")

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

static char __CFBundleMainID__[1026] = {0};
CF_PRIVATE char *__CFBundleMainID = __CFBundleMainID__;

static CFTypeID __kCFBundleTypeID = _kCFRuntimeNotATypeID;

static pthread_mutex_t CFBundleGlobalDataLock = PTHREAD_MUTEX_INITIALIZER;

static CFMutableDictionaryRef _bundlesByIdentifier = NULL;
#if AVOID_WEAK_COLLECTIONS
static CFMutableDictionaryRef _bundlesByURL = NULL;
static CFMutableArrayRef _allBundles = NULL;
static CFMutableSetRef _bundlesToUnload = NULL;
#else /* AVOID_WEAK_COLLECTIONS */
static __CFHashTable *_allBundles = nil;
static __CFHashTable *_bundlesToUnload = nil;
#endif /* AVOID_WEAK_COLLECTIONS */
static Boolean _scheduledBundlesAreUnloading = false;

static Boolean _initedMainBundle = false;
static CFBundleRef _mainBundle = NULL;

// Forward declares functions.
static CFBundleRef _CFBundleCreate(CFAllocatorRef allocator, CFURLRef bundleURL, Boolean alreadyLocked, Boolean doFinalProcessing);
static CFURLRef _CFBundleCopyExecutableURLIgnoringCache(CFBundleRef bundle);
static void _CFBundleEnsureBundlesUpToDateWithHintAlreadyLocked(CFStringRef hint);
static void _CFBundleEnsureAllBundlesUpToDateAlreadyLocked(void);
static void _CFBundleEnsureBundleExistsForImagePath(CFStringRef imagePath);
static void _CFBundleEnsureBundlesExistForImagePaths(CFArrayRef imagePaths);

#pragma mark -




#if AVOID_WEAK_COLLECTIONS

static void _CFBundleAddToTables(CFBundleRef bundle, Boolean alreadyLocked) {
    CFStringRef bundleID = CFBundleGetIdentifier(bundle);

    if (!alreadyLocked) pthread_mutex_lock(&CFBundleGlobalDataLock);
    
    // Add to the _allBundles list
    if (!_allBundles) {
        CFArrayCallBacks nonRetainingArrayCallbacks = kCFTypeArrayCallBacks;
        nonRetainingArrayCallbacks.retain = NULL;
        nonRetainingArrayCallbacks.release = NULL;
        _allBundles = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &nonRetainingArrayCallbacks);
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
    if (!alreadyLocked) pthread_mutex_unlock(&CFBundleGlobalDataLock);
}

static void _CFBundleRemoveFromTables(CFBundleRef bundle, CFURLRef bundleURL, CFStringRef bundleID) {
    pthread_mutex_lock(&CFBundleGlobalDataLock);
    // Remove from the various lists
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
    pthread_mutex_unlock(&CFBundleGlobalDataLock);
}

static CFBundleRef _CFBundleCopyBundleForURL(CFURLRef url, Boolean alreadyLocked) {
    CFBundleRef result = NULL;
    if (!alreadyLocked) pthread_mutex_lock(&CFBundleGlobalDataLock);
    if (_bundlesByURL) result = (CFBundleRef)CFDictionaryGetValue(_bundlesByURL, url);
    if (result && !result->_url) {
        result = NULL;
        CFDictionaryRemoveValue(_bundlesByURL, url);
    }
    if (result) CFRetain(result);
    if (!alreadyLocked) pthread_mutex_unlock(&CFBundleGlobalDataLock);
    return result;
}

static CFBundleRef _CFBundlePrimitiveGetBundleWithIdentifierAlreadyLocked(CFStringRef bundleID) {
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

#else /* AVOID_WEAK_COLLECTIONS */

/*
 An explanation of what I'm doing here is probably in order.
 8029300 has cast suspicion on the correctness of __CFMapTable with strong keys and weak values, at least under non-GC. 
 An early attempt to work around it by inserting dummy values instead of removing things succeeded, as did turning on the AVOID_WEAK_COLLECTIONS #ifdef
 This indicates that it's not an overrelease in securityd, since AVOID_WEAK_COLLECTIONS wouldn't help in that case.
 Therefore, these functions following this comment allow us to have _bundlesByURL be a CFDictionary on non-GC and keep __CFMapTable to GC where it's needed.
 */
static inline id _getBundlesByURL() {
    static id _bundles = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (CF_USING_COLLECTABLE_MEMORY) {
            _bundles = [[__CFMapTable alloc] initWithKeyOptions:CFPointerFunctionsStrongMemory valueOptions:CFPointerFunctionsZeroingWeakMemory capacity:0];
        } else {
            CFDictionaryValueCallBacks nonRetainingDictionaryValueCallbacks = kCFTypeDictionaryValueCallBacks;
            nonRetainingDictionaryValueCallbacks.retain = NULL;
            nonRetainingDictionaryValueCallbacks.release = NULL;
            _bundles = (id)CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &nonRetainingDictionaryValueCallbacks);
        }
    });
    return _bundles;
}

#define _bundlesByURL _getBundlesByURL()

static void _setInBundlesByURL(CFURLRef key, CFBundleRef bundle) {
    if (CF_USING_COLLECTABLE_MEMORY) {
        [(__CFMapTable *)_bundlesByURL setObject:(id)bundle forKey:(id)key];
    } else {
        CFDictionarySetValue((CFMutableDictionaryRef)_bundlesByURL, key, bundle);
    }
}

static void _removeFromBundlesByURL(CFURLRef key) {
    if (CF_USING_COLLECTABLE_MEMORY) {
        [(__CFMapTable *)_bundlesByURL removeObjectForKey:(id)key];
    } else {
        CFDictionaryRemoveValue((CFMutableDictionaryRef)_bundlesByURL, key);
    }
}

static CFBundleRef _getFromBundlesByURL(CFURLRef key) {
    if (CF_USING_COLLECTABLE_MEMORY) {
        return (CFBundleRef)[(__CFMapTable *)_bundlesByURL objectForKey:(id)key];
    } else {
        return (CFBundleRef)CFDictionaryGetValue((CFMutableDictionaryRef)_bundlesByURL, key);
    }
}

static void _CFBundleAddToTables(CFBundleRef bundle, Boolean alreadyLocked) {
    CFStringRef bundleID = CFBundleGetIdentifier(bundle);

    if (!alreadyLocked) pthread_mutex_lock(&CFBundleGlobalDataLock);
    
    // Add to the _allBundles list
    if (!_allBundles) _allBundles = [[__CFHashTable alloc] initWithOptions:CFPointerFunctionsZeroingWeakMemory capacity:0];
    [_allBundles addObject:(id)bundle];
    
    // Add to the table that maps urls to bundles
    _setInBundlesByURL(bundle->_url, bundle);

    // Add to the table that maps identifiers to bundles
    if (bundleID) {
        __CFPointerArray *bundlesWithThisID = nil;
        CFBundleRef existingBundle = NULL;
        if (!_bundlesByIdentifier) {
            _bundlesByIdentifier = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        }
        bundlesWithThisID = (__CFPointerArray *)CFDictionaryGetValue(_bundlesByIdentifier, bundleID);
        if (bundlesWithThisID) {
            CFIndex i, count = (CFIndex)[bundlesWithThisID count];
            UInt32 existingVersion, newVersion = CFBundleGetVersionNumber(bundle);
            for (i = 0; i < count; i++) {
                existingBundle = (CFBundleRef)[bundlesWithThisID pointerAtIndex:i];
                if (!existingBundle) continue;
                existingVersion = CFBundleGetVersionNumber(existingBundle);
                // If you load two bundles with the same identifier and the same version, the last one wins.
                if (newVersion >= existingVersion) break;
            }
            if (i < count) {
                [bundlesWithThisID insertPointer:bundle atIndex:i];
            } else {
                [bundlesWithThisID addPointer:bundle];
            }
        } else {
            bundlesWithThisID = [[__CFPointerArray alloc] initWithOptions:CFPointerFunctionsZeroingWeakMemory];
            [bundlesWithThisID addPointer:bundle];
            CFDictionarySetValue(_bundlesByIdentifier, bundleID, bundlesWithThisID);
            [bundlesWithThisID release];
        }
    }
    if (!alreadyLocked) pthread_mutex_unlock(&CFBundleGlobalDataLock);
}

static void _CFBundleRemoveFromTables(CFBundleRef bundle, CFURLRef bundleURL, CFStringRef bundleID) {
    pthread_mutex_lock(&CFBundleGlobalDataLock);
    // Remove from the various lists
    if (_allBundles && [_allBundles member:(id)bundle]) [_allBundles removeObject:(id)bundle];

    // Remove from the table that maps urls to bundles
    if (bundleURL) {
        _removeFromBundlesByURL(bundleURL);
    }
    
    // Remove from the table that maps identifiers to bundles
    if (bundleID && _bundlesByIdentifier) {
        __CFPointerArray *bundlesWithThisID = (__CFPointerArray *)CFDictionaryGetValue(_bundlesByIdentifier, bundleID);
        if (bundlesWithThisID) {
            CFIndex count = (CFIndex)[bundlesWithThisID count];
            while (count-- > 0) if (bundle == (CFBundleRef)[bundlesWithThisID pointerAtIndex:count]) [bundlesWithThisID removePointerAtIndex:count];
            [bundlesWithThisID compact];
            if (0 == [bundlesWithThisID count]) CFDictionaryRemoveValue(_bundlesByIdentifier, bundleID);
        }
    }
    pthread_mutex_unlock(&CFBundleGlobalDataLock);
}

static CFBundleRef _CFBundleCopyBundleForURL(CFURLRef url, Boolean alreadyLocked) {
    CFBundleRef result = NULL;
    if (!alreadyLocked) pthread_mutex_lock(&CFBundleGlobalDataLock);
    result = _getFromBundlesByURL(url);
    if (result && !result->_url) {
        result = NULL;
        _removeFromBundlesByURL(url);
    }
    if (result) CFRetain(result);
    if (!alreadyLocked) pthread_mutex_unlock(&CFBundleGlobalDataLock);
    return result;
}

static CFBundleRef _CFBundlePrimitiveGetBundleWithIdentifierAlreadyLocked(CFStringRef bundleID) {
    CFBundleRef result = NULL;
    if (_bundlesByIdentifier && bundleID) {
        // Note that this array is maintained in descending order by version number
        __CFPointerArray *bundlesWithThisID = (__CFPointerArray *)CFDictionaryGetValue(_bundlesByIdentifier, bundleID);
        if (bundlesWithThisID && [bundlesWithThisID count] > 0) {
            // First check for loaded bundles so we will always prefer a loaded to an unloaded bundle
            for (id bundle in bundlesWithThisID) {
                if (bundle && CFBundleIsExecutableLoaded((CFBundleRef)bundle)) {
                    result = (CFBundleRef)bundle;
                    break;
                }
            }
            // If no loaded bundle, simply take the first item in the array, i.e. the one with the latest version number
            if (!result) {
                for (id bundle in bundlesWithThisID) {
                    if (bundle) {
                        result = (CFBundleRef)bundle;
                        break;
                    }
                }
            }
        }
    }
    return result;
}

#endif /* AVOID_WEAK_COLLECTIONS */

static CFURLRef _CFBundleCopyBundleURLForExecutablePath(CFStringRef str) {
    //!!! need to handle frameworks, NT; need to integrate with NSBundle - drd
    UniChar buff[CFMaxPathSize];
    CFIndex buffLen;
    CFURLRef url = NULL;
    CFStringRef outstr;
    
    buffLen = CFStringGetLength(str);
    if (buffLen > CFMaxPathSize) buffLen = CFMaxPathSize;
    CFStringGetCharacters(str, CFRangeMake(0, buffLen), buff);

#if DEPLOYMENT_TARGET_WINDOWS
    // Is this a .dll or .exe?
    if (buffLen >= 5 && (_wcsnicmp((wchar_t *)&(buff[buffLen-4]), L".dll", 4) == 0 || _wcsnicmp((wchar_t *)&(buff[buffLen-4]), L".exe", 4) == 0)) {
        CFIndex extensionLength = CFStringGetLength(_CFBundleWindowsResourceDirectoryExtension);
        buffLen -= 4;
        // If this is an _debug, we should strip that before looking for the bundle
        if (buffLen >= 7 && (_wcsnicmp((wchar_t *)&buff[buffLen-6], L"_debug", 6) == 0)) buffLen -= 6;

        if (buffLen + 1 + extensionLength < CFMaxPathSize) {
            buff[buffLen] = '.';
            buffLen ++;
            CFStringGetCharacters(_CFBundleWindowsResourceDirectoryExtension, CFRangeMake(0, extensionLength), buff + buffLen);
            buffLen += extensionLength;
            outstr = CFStringCreateWithCharactersNoCopy(kCFAllocatorSystemDefault, buff, buffLen, kCFAllocatorNull);
            url = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, outstr, PLATFORM_PATH_STYLE, true);
            CFRelease(outstr);
        }
    }
#endif

    if (!url) {
        buffLen = _CFLengthAfterDeletingLastPathComponent(buff, buffLen);  // Remove exe name

        if (buffLen > 0) {
            // See if this is a new bundle.  If it is, we have to remove more path components.
            CFIndex startOfLastDir = _CFStartOfLastPathComponent(buff, buffLen);
            if (startOfLastDir > 0 && startOfLastDir < buffLen) {
                CFStringRef lastDirName = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, &(buff[startOfLastDir]), buffLen - startOfLastDir);

                if (CFEqual(lastDirName, _CFBundleGetPlatformExecutablesSubdirectoryName()) || CFEqual(lastDirName, _CFBundleGetAlternatePlatformExecutablesSubdirectoryName()) || CFEqual(lastDirName, _CFBundleGetOtherPlatformExecutablesSubdirectoryName()) || CFEqual(lastDirName, _CFBundleGetOtherAlternatePlatformExecutablesSubdirectoryName())) {
                    // This is a new bundle.  Back off a few more levels
                    if (buffLen > 0) {
                        // Remove platform folder
                        buffLen = _CFLengthAfterDeletingLastPathComponent(buff, buffLen);
                    }
                    if (buffLen > 0) {
                        // Remove executables folder (if present)
                        CFIndex startOfNextDir = _CFStartOfLastPathComponent(buff, buffLen);
                        if (startOfNextDir > 0 && startOfNextDir < buffLen) {
                            CFStringRef nextDirName = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, &(buff[startOfNextDir]), buffLen - startOfNextDir);
                            if (CFEqual(nextDirName, _CFBundleExecutablesDirectoryName)) buffLen = _CFLengthAfterDeletingLastPathComponent(buff, buffLen);
                            CFRelease(nextDirName);
                        }
                    }
                    if (buffLen > 0) {
                        // Remove support files folder
                        buffLen = _CFLengthAfterDeletingLastPathComponent(buff, buffLen);
                    }
                }
                CFRelease(lastDirName);
            }
        }

        if (buffLen > 0) {
            outstr = CFStringCreateWithCharactersNoCopy(kCFAllocatorSystemDefault, buff, buffLen, kCFAllocatorNull);
            url = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, outstr, PLATFORM_PATH_STYLE, true);
            CFRelease(outstr);
        }
    }
    return url;
}

static CFURLRef _CFBundleCopyResolvedURLForExecutableURL(CFURLRef url) {
    // this is necessary so that we match any sanitization CFURL may perform on the result of _CFBundleCopyBundleURLForExecutableURL()
    CFURLRef absoluteURL, url1, url2, outURL = NULL;
    CFStringRef str, str1, str2;
    absoluteURL = CFURLCopyAbsoluteURL(url);
    str = CFURLCopyFileSystemPath(absoluteURL, PLATFORM_PATH_STYLE);
    if (str) {
        UniChar buff[CFMaxPathSize];
        CFIndex buffLen = CFStringGetLength(str), len1;
        if (buffLen > CFMaxPathSize) buffLen = CFMaxPathSize;
        CFStringGetCharacters(str, CFRangeMake(0, buffLen), buff);
        len1 = _CFLengthAfterDeletingLastPathComponent(buff, buffLen);
        if (len1 > 0 && len1 + 1 < buffLen) {          
            str1 = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, buff, len1);
            CFIndex skipSlashCount = 1;
#if DEPLOYMENT_TARGET_WINDOWS
            // On Windows, _CFLengthAfterDeletingLastPathComponent will return a value of 3 if the path is at the root (e.g. C:\). This includes the \, which is not the case for URLs with subdirectories
            if (len1 == 3 && buff[1] == ':' && buff[2] == '\\') {
                skipSlashCount = 0;
            }
#endif
            str2 = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, buff + len1 + skipSlashCount, buffLen - len1 - skipSlashCount);
            if (str1 && str2) {
                url1 = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, str1, PLATFORM_PATH_STYLE, true);
                if (url1) {
                    url2 = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, str2, PLATFORM_PATH_STYLE, false, url1);
                    if (url2) {
                        outURL = CFURLCopyAbsoluteURL(url2);
                        CFRelease(url2);
                    }
                    CFRelease(url1);
                }
            }
            if (str1) CFRelease(str1);
            if (str2) CFRelease(str2);
        }
        CFRelease(str);
    }
    if (!outURL) {
        outURL = absoluteURL;
    } else {
        CFRelease(absoluteURL);
    }
    return outURL;
}

CFURLRef _CFBundleCopyBundleURLForExecutableURL(CFURLRef url) {
    CFURLRef resolvedURL, outurl = NULL;
    CFStringRef str;
    resolvedURL = _CFBundleCopyResolvedURLForExecutableURL(url);
    str = CFURLCopyFileSystemPath(resolvedURL, PLATFORM_PATH_STYLE);
    if (str) {
        outurl = _CFBundleCopyBundleURLForExecutablePath(str);
        CFRelease(str);
    }
    CFRelease(resolvedURL);
    return outurl;
}

static uint8_t _CFBundleEffectiveLayoutVersion(CFBundleRef bundle) {
    uint8_t localVersion = bundle->_version;
    // exclude type 0 bundles with no binary (or CFM binary) and no Info.plist, since they give too many false positives
    if (0 == localVersion) {
        CFDictionaryRef infoDict = CFBundleGetInfoDictionary(bundle);
        if (!infoDict || 0 == CFDictionaryGetCount(infoDict)) {
#if defined(BINARY_SUPPORT_DYLD)
            CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
            if (executableURL) {
                if (bundle->_binaryType == __CFBundleUnknownBinary) bundle->_binaryType = _CFBundleGrokBinaryType(executableURL);
                if (bundle->_binaryType == __CFBundleCFMBinary || bundle->_binaryType == __CFBundleUnreadableBinary) {
                    localVersion = 4;
                } else {
                    bundle->_resourceData._executableLacksResourceFork = true;
                }
                CFRelease(executableURL);
            } else {
                localVersion = 4;
            }
#else 
            CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
            if (executableURL) {
                CFRelease(executableURL);
            } else {
                localVersion = 4;
            }
#endif /* BINARY_SUPPORT_DYLD */
        }
    }
    return localVersion;
}

CFBundleRef _CFBundleCreateIfLooksLikeBundle(CFAllocatorRef allocator, CFURLRef url) {
    CFBundleRef bundle = CFBundleCreate(allocator, url);
    if (bundle) {
        uint8_t localVersion = _CFBundleEffectiveLayoutVersion(bundle);
        if (3 == localVersion || 4 == localVersion) {
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
    if (mainBundle && (3 == mainBundle->_version || 4 == mainBundle->_version)) mainBundle = NULL;
    return mainBundle;
}

Boolean _CFBundleMainBundleInfoDictionaryComesFromResourceFork(void) {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    return (mainBundle && mainBundle->_resourceData._infoDictionaryFromResourceFork);
}

CFBundleRef _CFBundleCreateWithExecutableURLIfLooksLikeBundle(CFAllocatorRef allocator, CFURLRef url) {
    CFBundleRef bundle = NULL;
    CFURLRef bundleURL = _CFBundleCopyBundleURLForExecutableURL(url), resolvedURL = _CFBundleCopyResolvedURLForExecutableURL(url);
    if (bundleURL && resolvedURL) {
        // We used to call _CFBundleCreateIfLooksLikeBundle here, but switched to the regular CFBundleCreate because we want this to return a result for certain flat bundles as well.
        bundle = CFBundleCreate(allocator, bundleURL);
        if (bundle) {
            CFURLRef executableURL = _CFBundleCopyExecutableURLIgnoringCache(bundle);
            char buff1[CFMaxPathSize], buff2[CFMaxPathSize];
            if (!executableURL || !CFURLGetFileSystemRepresentation(resolvedURL, true, (uint8_t *)buff1, CFMaxPathSize) || !CFURLGetFileSystemRepresentation(executableURL, true, (uint8_t *)buff2, CFMaxPathSize) || 0 != strcmp(buff1, buff2)) {
                CFRelease(bundle);
                bundle = NULL;
            }
            if (executableURL) CFRelease(executableURL);
        }
    }
    if (bundleURL) CFRelease(bundleURL);
    if (resolvedURL) CFRelease(resolvedURL);
    return bundle;
}

CFBundleRef _CFBundleCreateIfMightBeBundle(CFAllocatorRef allocator, CFURLRef url) {
    // This function is obsolete
    CFBundleRef bundle = CFBundleCreate(allocator, url);
    return bundle;
}

CFBundleRef _CFBundleCreateWithExecutableURLIfMightBeBundle(CFAllocatorRef allocator, CFURLRef url) {
    CFBundleRef result = _CFBundleCreateWithExecutableURLIfLooksLikeBundle(allocator, url);
    
    // This function applies additional requirements on a bundle to return a result
    // The above makes sure that:
    //  0. CFBundleCreate must succeed using a URL derived from the executable URL
    //  1. The bundle must have an executableURL, and it must match the passed in executable URL
    
    // This function additionally requires that
    //  2. If flat, the bundle must have a non-empty Info.plist. (15663535)
    if (result) {
        uint8_t localVersion = _CFBundleEffectiveLayoutVersion(result);
        if (3 == localVersion || 4 == localVersion) {
            CFDictionaryRef infoPlist = CFBundleGetInfoDictionary(result);
            if (!infoPlist || (infoPlist && CFDictionaryGetCount(infoPlist) == 0)) {
                CFRelease(result);
                result = NULL;
            }
        }
    }
    return result;
}

CFURLRef _CFBundleCopyMainBundleExecutableURL(Boolean *looksLikeBundle) {
    // This function is for internal use only; _mainBundle is deliberately accessed outside of the lock to get around a reentrancy issue
    const char *processPath;
    CFStringRef str = NULL;
    CFURLRef executableURL = NULL;
    processPath = _CFProcessPath();
    if (processPath) {
        str = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, processPath);
        if (str) {
            executableURL = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, str, PLATFORM_PATH_STYLE, false);
            CFRelease(str);
        }
    }
    if (looksLikeBundle) {
        CFBundleRef mainBundle = _mainBundle;
        if (mainBundle && (3 == mainBundle->_version || 4 == mainBundle->_version)) mainBundle = NULL;
        *looksLikeBundle = (mainBundle ? true : false);
    }
    return executableURL;
}

static void _CFBundleInitializeMainBundleInfoDictionaryAlreadyLocked(CFStringRef executablePath) {
    CFBundleGetInfoDictionary(_mainBundle);
    if (!_mainBundle->_infoDict || CFDictionaryGetCount(_mainBundle->_infoDict) == 0) {
        // if type 3 bundle and no Info.plist, treat as unbundled, since this gives too many false positives
        if (_mainBundle->_version == 3) _mainBundle->_version = 4;
        if (_mainBundle->_version == 0) {
            // if type 0 bundle and no Info.plist and not main executable for bundle, treat as unbundled, since this gives too many false positives
            CFStringRef executableName = _CFBundleCopyExecutableName(_mainBundle, NULL, NULL);
            if (!executableName || !executablePath || !CFStringHasSuffix(executablePath, executableName)) _mainBundle->_version = 4;
            if (executableName) CFRelease(executableName);
        }
#if defined(BINARY_SUPPORT_DYLD)
        if (_mainBundle->_binaryType == __CFBundleDYLDExecutableBinary) {
            if (_mainBundle->_infoDict && !(0)) CFRelease(_mainBundle->_infoDict);
            _mainBundle->_infoDict = (CFDictionaryRef)_CFBundleCreateInfoDictFromMainExecutable();
        }
#endif /* BINARY_SUPPORT_DYLD */
    } else {
#if defined(BINARY_SUPPORT_DYLD)
        if (_mainBundle->_binaryType == __CFBundleDYLDExecutableBinary) {
            // if dyld and not main executable for bundle, prefer info dictionary from executable
            CFStringRef executableName = _CFBundleCopyExecutableName(_mainBundle, NULL, NULL);
            if (!executableName || !executablePath || !CFStringHasSuffix(executablePath, executableName)) {
                CFDictionaryRef infoDictFromExecutable = (CFDictionaryRef)_CFBundleCreateInfoDictFromMainExecutable();
                if (infoDictFromExecutable && CFDictionaryGetCount(infoDictFromExecutable) > 0) {
                    if (_mainBundle->_infoDict) CFRelease(_mainBundle->_infoDict);
                    _mainBundle->_infoDict = infoDictFromExecutable;
                } else if (infoDictFromExecutable) {
                    CFRelease(infoDictFromExecutable);
                }
            }
            if (executableName) CFRelease(executableName);
        }
#endif /* BINARY_SUPPORT_DYLD */
    }
    if (!_mainBundle->_infoDict) _mainBundle->_infoDict = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (!_mainBundle->_executablePath && executablePath) _mainBundle->_executablePath = (CFStringRef)CFRetain(executablePath);
    CFStringRef bundleID = (CFStringRef)CFDictionaryGetValue(_mainBundle->_infoDict, kCFBundleIdentifierKey);
    if (bundleID) {
        if (!CFStringGetCString(bundleID, __CFBundleMainID__, sizeof(__CFBundleMainID__) - 2, kCFStringEncodingUTF8)) {
            __CFBundleMainID__[0] = '\0';
        }
    }
}
        
static void _CFBundleFlushBundleCachesAlreadyLocked(CFBundleRef bundle, Boolean alreadyLocked) {
    CFDictionaryRef oldInfoDict = bundle->_infoDict;
    CFTypeRef val;
    
    bundle->_infoDict = NULL;
    if (bundle->_localInfoDict) {
        CFRelease(bundle->_localInfoDict);
        bundle->_localInfoDict = NULL;
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
    if (bundle == _mainBundle) {
        CFStringRef executablePath = bundle->_executablePath;
        if (!alreadyLocked) pthread_mutex_lock(&CFBundleGlobalDataLock);
        _CFBundleInitializeMainBundleInfoDictionaryAlreadyLocked(executablePath);
        if (!alreadyLocked) pthread_mutex_unlock(&CFBundleGlobalDataLock);
    } else {
        CFBundleGetInfoDictionary(bundle);
    }
    if (oldInfoDict) {
        if (!bundle->_infoDict) bundle->_infoDict = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        val = CFDictionaryGetValue(oldInfoDict, _kCFBundlePrincipalClassKey);
        if (val) CFDictionarySetValue((CFMutableDictionaryRef)bundle->_infoDict, _kCFBundlePrincipalClassKey, val);
        CFRelease(oldInfoDict);
    }
    
    _CFBundleFlushQueryTableCache(bundle);
}

CF_EXPORT void _CFBundleFlushBundleCaches(CFBundleRef bundle) {
    _CFBundleFlushBundleCachesAlreadyLocked(bundle, false);
}

static CFBundleRef _CFBundleGetMainBundleAlreadyLocked(void) {
    if (!_initedMainBundle) {
        const char *processPath;
        CFStringRef str = NULL;
        CFURLRef executableURL = NULL, bundleURL = NULL;
        _initedMainBundle = true;
        processPath = _CFProcessPath();
        if (processPath) {
            str = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, processPath);
            if (!executableURL) executableURL = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, str, PLATFORM_PATH_STYLE, false);
        }
        if (executableURL) bundleURL = _CFBundleCopyBundleURLForExecutableURL(executableURL);
        if (bundleURL) {
            // make sure that main bundle has executable path
            //??? what if we are not the main executable in the bundle?
            // NB doFinalProcessing must be false here, see below
            _mainBundle = _CFBundleCreate(kCFAllocatorSystemDefault, bundleURL, true, false);
            if (_mainBundle) {
                // make sure that the main bundle is listed as loaded, and mark it as executable
                _mainBundle->_isLoaded = true;
#if defined(BINARY_SUPPORT_DYLD)
                if (_mainBundle->_binaryType == __CFBundleUnknownBinary) {
                    if (!executableURL) {
                        _mainBundle->_binaryType = __CFBundleNoBinary;
                    } else {
                        _mainBundle->_binaryType = _CFBundleGrokBinaryType(executableURL);
                        if (_mainBundle->_binaryType != __CFBundleCFMBinary && _mainBundle->_binaryType != __CFBundleUnreadableBinary) _mainBundle->_resourceData._executableLacksResourceFork = true;
                    }
                }                
#endif /* BINARY_SUPPORT_DYLD */
                // get cookie for already-loaded main bundle
#if defined(BINARY_SUPPORT_DLFCN)
                if (!_mainBundle->_handleCookie) {
                    _mainBundle->_handleCookie = dlopen(NULL, RTLD_NOLOAD | RTLD_FIRST);
#if LOG_BUNDLE_LOAD
                    printf("main bundle %p getting handle %p\n", _mainBundle, _mainBundle->_handleCookie);
#endif /* LOG_BUNDLE_LOAD */
                }
#elif defined(BINARY_SUPPORT_DYLD)
                if (_mainBundle->_binaryType == __CFBundleDYLDExecutableBinary && !_mainBundle->_imageCookie) {
                    _mainBundle->_imageCookie = (void *)_dyld_get_image_header(0);
#if LOG_BUNDLE_LOAD
                    printf("main bundle %p getting image %p\n", _mainBundle, _mainBundle->_imageCookie);
#endif /* LOG_BUNDLE_LOAD */
                }
#endif /* BINARY_SUPPORT_DLFCN */
                _CFBundleInitializeMainBundleInfoDictionaryAlreadyLocked(str);
                // Perform delayed final processing steps.
                // This must be done after _isLoaded has been set, for security reasons (3624341).
                if (_CFBundleNeedsInitPlugIn(_mainBundle)) {
                    pthread_mutex_unlock(&CFBundleGlobalDataLock);
                    _CFBundleInitPlugIn(_mainBundle);
                    pthread_mutex_lock(&CFBundleGlobalDataLock);
                }
            }
        }
        if (bundleURL) CFRelease(bundleURL);
        if (str) CFRelease(str);
        if (executableURL) CFRelease(executableURL);
    }
    return _mainBundle;
}

CFBundleRef CFBundleGetMainBundle(void) {
    CFBundleRef mainBundle;
    pthread_mutex_lock(&CFBundleGlobalDataLock);
    mainBundle = _CFBundleGetMainBundleAlreadyLocked();
    pthread_mutex_unlock(&CFBundleGlobalDataLock);
    return mainBundle;
}

CFBundleRef CFBundleGetBundleWithIdentifier(CFStringRef bundleID) {
    CFBundleRef result = NULL;
    if (bundleID) {
        pthread_mutex_lock(&CFBundleGlobalDataLock);
        (void)_CFBundleGetMainBundleAlreadyLocked();
        result = _CFBundlePrimitiveGetBundleWithIdentifierAlreadyLocked(bundleID);
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
        if (!result) {
            // Try to create the bundle for the caller and try again
            void *p = __builtin_return_address(0);
            if (p) {
                CFStringRef imagePath = _CFBundleCopyLoadedImagePathForPointer(p);
                if (imagePath) {
                    _CFBundleEnsureBundleExistsForImagePath(imagePath);
                    CFRelease(imagePath);
                }
                result = _CFBundlePrimitiveGetBundleWithIdentifierAlreadyLocked(bundleID);
            }
        }
#endif
        if (!result) {
            // Try to guess the bundle from the identifier and try again
            _CFBundleEnsureBundlesUpToDateWithHintAlreadyLocked(bundleID);
            result = _CFBundlePrimitiveGetBundleWithIdentifierAlreadyLocked(bundleID);
        }
        if (!result) {
            // Make sure all bundles have been created and try again.
            _CFBundleEnsureAllBundlesUpToDateAlreadyLocked();
            result = _CFBundlePrimitiveGetBundleWithIdentifierAlreadyLocked(bundleID);
        }
        pthread_mutex_unlock(&CFBundleGlobalDataLock);
    }
    return result;
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

static void _CFBundleDeallocateGlue(const void *key, const void *value, void *context) {
    CFAllocatorRef allocator = (CFAllocatorRef)context;
    if (value) CFAllocatorDeallocate(allocator, (void *)value);
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
    if (bundle->_infoDict && !(0)) CFRelease(bundle->_infoDict);
    if (bundle->_modDate) CFRelease(bundle->_modDate);
    if (bundle->_localInfoDict && !(0)) CFRelease(bundle->_localInfoDict);
    if (bundle->_searchLanguages) CFRelease(bundle->_searchLanguages);
    if (bundle->_executablePath) CFRelease(bundle->_executablePath);
    if (bundle->_developmentRegion) CFRelease(bundle->_developmentRegion);
    if (bundle->_glueDict) {
        CFDictionaryApplyFunction(bundle->_glueDict, _CFBundleDeallocateGlue, (void *)CFGetAllocator(bundle));
        CFRelease(bundle->_glueDict);
    }
    if (bundle->_stringTable) CFRelease(bundle->_stringTable);
    
    if (bundle->_bundleBasePath) CFRelease(bundle->_bundleBasePath);
    if (bundle->_queryTable) CFRelease(bundle->_queryTable);
    
    if (bundle->_localizations) CFRelease(bundle->_localizations);
    if (bundle->_resourceDirectoryContents) CFRelease(bundle->_resourceDirectoryContents);
    
    pthread_mutex_destroy(&(bundle->_bundleLoadingLock));
}

static const CFRuntimeClass __CFBundleClass = {
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

// From CFBundle_Resources.c
CF_PRIVATE void _CFBundleResourcesInitialize();

CFTypeID CFBundleGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFBundleTypeID = _CFRuntimeRegisterClass(&__CFBundleClass); _CFBundleResourcesInitialize(); });
    return __kCFBundleTypeID;
}

CFBundleRef _CFBundleGetExistingBundleWithBundleURL(CFURLRef bundleURL) {
    CFBundleRef bundle = NULL;
    char buff[CFMaxPathSize];
    CFURLRef newURL = NULL;
    
    if (!CFURLGetFileSystemRepresentation(bundleURL, true, (uint8_t *)buff, CFMaxPathSize)) return NULL;
    
    newURL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, (uint8_t *)buff, strlen(buff), true);
    if (!newURL) newURL = (CFURLRef)CFRetain(bundleURL);
    bundle = _CFBundleCopyBundleForURL(newURL, false);
    if (bundle) CFRelease(bundle);
    CFRelease(newURL);
    return bundle;
}

static CFBundleRef _CFBundleCreate(CFAllocatorRef allocator, CFURLRef bundleURL, Boolean alreadyLocked, Boolean doFinalProcessing) {
    CFBundleRef bundle = NULL;
    char buff[CFMaxPathSize];
    CFDateRef modDate = NULL; // do not actually fetch the modDate, since that can cause something like 7609956, unless absolutely found to be necessary in the future
    Boolean exists = false;
    SInt32 mode = 0;
    CFURLRef newURL = NULL;
    uint8_t localVersion = 0;
    
    if (!CFURLGetFileSystemRepresentation(bundleURL, true, (uint8_t *)buff, CFMaxPathSize)) return NULL;

    newURL = CFURLCreateFromFileSystemRepresentation(allocator, (uint8_t *)buff, strlen(buff), true);
    if (!newURL) newURL = (CFURLRef)CFRetain(bundleURL);
    bundle = _CFBundleCopyBundleForURL(newURL, alreadyLocked);
    if (bundle) {
        CFRelease(newURL);
        return bundle;
    }
    
    localVersion = _CFBundleGetBundleVersionForURL(newURL);
    if (localVersion == 3) {
        SInt32 res = _CFGetPathProperties(allocator, (char *)buff, &exists, &mode, NULL, NULL, NULL, NULL);
#if DEPLOYMENT_TARGET_WINDOWS
        if (!(res == 0 && exists && ((mode & S_IFMT) == S_IFDIR))) {
            // 2nd chance at finding a bundle path - remove the last path component (e.g., mybundle.resources) and try again
            if (modDate) {
                CFRelease(modDate);
                modDate = NULL;
            }
            CFURLRef shorterPath = CFURLCreateCopyDeletingLastPathComponent(allocator, newURL);
            CFRelease(newURL);
            newURL = shorterPath;
            res = _CFGetFileProperties(allocator, newURL, &exists, &mode, NULL, NULL, NULL, NULL);
        }
#endif
        if (res == 0) {
            if (!exists || ((mode & S_IFMT) != S_IFDIR)) {
                if (modDate) CFRelease(modDate);
                CFRelease(newURL);
                return NULL;
            }
        } else {
            CFRelease(newURL);
            return NULL;
        }
    }

    bundle = (CFBundleRef)_CFRuntimeCreateInstance(allocator, CFBundleGetTypeID(), sizeof(struct __CFBundle) - sizeof(CFRuntimeBase), NULL);
    if (!bundle) {
        CFRelease(newURL);
        return NULL;
    }

    bundle->_url = newURL;

    bundle->_modDate = modDate;
    bundle->_version = localVersion;
    bundle->_infoDict = NULL;
    bundle->_localInfoDict = NULL;
    bundle->_searchLanguages = NULL;
    bundle->_executablePath = NULL;
    bundle->_developmentRegion = NULL;
    bundle->_developmentRegionCalculated = 0;
#if defined(BINARY_SUPPORT_DYLD)
    /* We'll have to figure it out later */
    bundle->_binaryType = __CFBundleUnknownBinary;
#elif defined(BINARY_SUPPORT_DLL)
    /* We support DLL only */
    bundle->_binaryType = __CFBundleDLLBinary;
    bundle->_hModule = NULL;
#else
    /* We'll have to figure it out later */
    bundle->_binaryType = __CFBundleUnknownBinary;
#endif /* BINARY_SUPPORT_DYLD */

    bundle->_isLoaded = false;
    bundle->_sharesStringsFiles = false;
    
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    if (!__CFgetenv("CFBundleDisableStringsSharing") && 
        (strncmp(buff, "/System/Library/Frameworks", 26) == 0) && 
        (strncmp(buff + strlen(buff) - 10, ".framework", 10) == 0)) bundle->_sharesStringsFiles = true;
#endif

    bundle->_connectionCookie = NULL;
    bundle->_handleCookie = NULL;
    bundle->_imageCookie = NULL;
    bundle->_moduleCookie = NULL;

    bundle->_glueDict = NULL;
    
    bundle->_resourceData._executableLacksResourceFork = false;
    bundle->_resourceData._infoDictionaryFromResourceFork = false;

    bundle->_stringTable = NULL;

    bundle->_plugInData._isPlugIn = false;
    bundle->_plugInData._loadOnDemand = false;
    bundle->_plugInData._isDoingDynamicRegistration = false;
    bundle->_plugInData._instanceCount = 0;
    bundle->_plugInData._factories = NULL;

    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_DEFAULT);
    int32_t mret = pthread_mutex_init(&(bundle->_bundleLoadingLock), &mattr);
    pthread_mutexattr_destroy(&mattr);
    if (0 != mret) {
        CFLog(4, CFSTR("%s: failed to initialize bundle loading lock for bundle %@."), __PRETTY_FUNCTION__, bundle);
    }
    
    bundle->_lock = CFLockInit;
    bundle->_resourceDirectoryContents = NULL;
    
    bundle->_localizations = NULL;
    bundle->_lookedForLocalizations = false;
    
    bundle->_queryLock = CFLockInit;
    bundle->_queryTable = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFURLRef absoURL = CFURLCopyAbsoluteURL(bundle->_url);
    bundle->_bundleBasePath = CFURLCopyFileSystemPath(absoURL, PLATFORM_PATH_STYLE);
    CFRelease(absoURL);
    
    CFBundleGetInfoDictionary(bundle);
    
    // Do this so that we can use the dispatch_once on the ivar of this bundle safely
    OSMemoryBarrier();
    
    _CFBundleAddToTables(bundle, alreadyLocked);

    if (doFinalProcessing) {
        if (_CFBundleNeedsInitPlugIn(bundle)) {
            if (alreadyLocked) pthread_mutex_unlock(&CFBundleGlobalDataLock);
            _CFBundleInitPlugIn(bundle);
            if (alreadyLocked) pthread_mutex_lock(&CFBundleGlobalDataLock);
        }
    }
    
    return bundle;
}

CFBundleRef CFBundleCreate(CFAllocatorRef allocator, CFURLRef bundleURL) {
    return _CFBundleCreate(allocator, bundleURL, false, true);
}

CFArrayRef CFBundleCreateBundlesFromDirectory(CFAllocatorRef alloc, CFURLRef directoryURL, CFStringRef bundleType) {
    CFMutableArrayRef bundles = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
    CFArrayRef URLs = _CFCreateContentsOfDirectory(alloc, NULL, NULL, directoryURL, bundleType);
    if (URLs) {
        CFIndex i, c = CFArrayGetCount(URLs);
        CFURLRef curURL;
        CFBundleRef curBundle;

        for (i = 0; i < c; i++) {
            curURL = (CFURLRef)CFArrayGetValueAtIndex(URLs, i);
            curBundle = CFBundleCreate(alloc, curURL);
            if (curBundle) CFArrayAppendValue(bundles, curBundle);
        }
        CFRelease(URLs);
    }

    return bundles;
}

CFURLRef CFBundleCopyBundleURL(CFBundleRef bundle) {
    if (bundle->_url) CFRetain(bundle->_url);
    return bundle->_url;
}

#define DEVELOPMENT_STAGE 0x20
#define ALPHA_STAGE 0x40
#define BETA_STAGE 0x60
#define RELEASE_STAGE 0x80

#define MAX_VERS_LEN 10

CF_INLINE Boolean _isDigit(UniChar aChar) {return ((aChar >= (UniChar)'0' && aChar <= (UniChar)'9') ? true : false);}

CF_PRIVATE CFStringRef _CFCreateStringFromVersionNumber(CFAllocatorRef alloc, UInt32 vers) {
    CFStringRef result = NULL;
    uint8_t major1, major2, minor1, minor2, stage, build;

    major1 = (vers & 0xF0000000) >> 28;
    major2 = (vers & 0x0F000000) >> 24;
    minor1 = (vers & 0x00F00000) >> 20;
    minor2 = (vers & 0x000F0000) >> 16;
    stage = (vers & 0x0000FF00) >> 8;
    build = (vers & 0x000000FF);

    if (stage == RELEASE_STAGE) {
        if (major1 > 0) {
            result = CFStringCreateWithFormat(alloc, NULL, CFSTR("%d%d.%d.%d"), major1, major2, minor1, minor2);
        } else {
            result = CFStringCreateWithFormat(alloc, NULL, CFSTR("%d.%d.%d"), major2, minor1, minor2);
        }
    } else {
        if (major1 > 0) {
            result = CFStringCreateWithFormat(alloc, NULL, CFSTR("%d%d.%d.%d%c%d"), major1, major2, minor1, minor2, ((stage == DEVELOPMENT_STAGE) ? 'd' : ((stage == ALPHA_STAGE) ? 'a' : 'b')), build);
        } else {
            result = CFStringCreateWithFormat(alloc, NULL, CFSTR("%d.%d.%d%c%d"), major2, minor1, minor2, ((stage == DEVELOPMENT_STAGE) ? 'd' : ((stage == ALPHA_STAGE) ? 'a' : 'b')), build);
        }
    }
    return result;
}

CF_PRIVATE UInt32 _CFVersionNumberFromString(CFStringRef versStr) {
    // Parse version number from string.
    // String can begin with "." for major version number 0.  String can end at any point, but elements within the string cannot be skipped.
    UInt32 major1 = 0, major2 = 0, minor1 = 0, minor2 = 0, stage = RELEASE_STAGE, build = 0;
    UniChar versChars[MAX_VERS_LEN];
    UniChar *chars = NULL;
    CFIndex len;
    UInt32 theVers;
    Boolean digitsDone = false;

    if (!versStr) return 0;
    len = CFStringGetLength(versStr);
    if (len <= 0 || len > MAX_VERS_LEN) return 0;

    CFStringGetCharacters(versStr, CFRangeMake(0, len), versChars);
    chars = versChars;
    
    // Get major version number.
    major1 = major2 = 0;
    if (_isDigit(*chars)) {
        major2 = *chars - (UniChar)'0';
        chars++;
        len--;
        if (len > 0) {
            if (_isDigit(*chars)) {
                major1 = major2;
                major2 = *chars - (UniChar)'0';
                chars++;
                len--;
                if (len > 0) {
                    if (*chars == (UniChar)'.') {
                        chars++;
                        len--;
                    } else {
                        digitsDone = true;
                    }
                }
            } else if (*chars == (UniChar)'.') {
                chars++;
                len--;
            } else {
                digitsDone = true;
            }
        }
    } else if (*chars == (UniChar)'.') {
        chars++;
        len--;
    } else {
        digitsDone = true;
    }

    // Now major1 and major2 contain first and second digit of the major version number as ints.
    // Now either len is 0 or chars points at the first char beyond the first decimal point.

    // Get the first minor version number.  
    if (len > 0 && !digitsDone) {
        if (_isDigit(*chars)) {
            minor1 = *chars - (UniChar)'0';
            chars++;
            len--;
            if (len > 0) {
                if (*chars == (UniChar)'.') {
                    chars++;
                    len--;
                } else {
                    digitsDone = true;
                }
            }
        } else {
            digitsDone = true;
        }
    }

    // Now minor1 contains the first minor version number as an int.
    // Now either len is 0 or chars points at the first char beyond the second decimal point.

    // Get the second minor version number. 
    if (len > 0 && !digitsDone) {
        if (_isDigit(*chars)) {
            minor2 = *chars - (UniChar)'0';
            chars++;
            len--;
        } else {
            digitsDone = true;
        }
    }

    // Now minor2 contains the second minor version number as an int.
    // Now either len is 0 or chars points at the build stage letter.

    // Get the build stage letter.  We must find 'd', 'a', 'b', or 'f' next, if there is anything next.
    if (len > 0) {
        if (*chars == (UniChar)'d') {
            stage = DEVELOPMENT_STAGE;
        } else if (*chars == (UniChar)'a') {
            stage = ALPHA_STAGE;
        } else if (*chars == (UniChar)'b') {
            stage = BETA_STAGE;
        } else if (*chars == (UniChar)'f') {
            stage = RELEASE_STAGE;
        } else {
            return 0;
        }
        chars++;
        len--;
    }

    // Now stage contains the release stage.
    // Now either len is 0 or chars points at the build number.

    // Get the first digit of the build number.
    if (len > 0) {
        if (_isDigit(*chars)) {
            build = *chars - (UniChar)'0';
            chars++;
            len--;
        } else {
            return 0;
        }
    }
    // Get the second digit of the build number.
    if (len > 0) {
        if (_isDigit(*chars)) {
            build *= 10;
            build += *chars - (UniChar)'0';
            chars++;
            len--;
        } else {
            return 0;
        }
    }
    // Get the third digit of the build number.
    if (len > 0) {
        if (_isDigit(*chars)) {
            build *= 10;
            build += *chars - (UniChar)'0';
            chars++;
            len--;
        } else {
            return 0;
        }
    }

    // Range check the build number and make sure we exhausted the string.
    if (build > 0xFF || len > 0) return 0;

    // Build the number
    theVers = major1 << 28;
    theVers += major2 << 24;
    theVers += minor1 << 20;
    theVers += minor2 << 16;
    theVers += stage << 8;
    theVers += build;

    return theVers;
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
    CFDateRef modDate;
    Boolean result = false;
    Boolean exists = false;
    SInt32 mode = 0;

    if (_CFGetFileProperties(CFGetAllocator(bundle), bundle->_url, &exists, &mode, NULL, &modDate, NULL, NULL) == 0) {
        // If the bundle no longer exists or is not a folder, it must have "changed"
        if (!exists || ((mode & S_IFMT) != S_IFDIR)) result = true;
    } else {
        // Something is wrong.  The stat failed.
        result = true;
    }
    if (bundle->_modDate && !CFEqual(bundle->_modDate, modDate)) {
        // mod date is different from when we created.
        result = true;
    }
    CFRelease(modDate);
    return result;
}

void _CFBundleSetStringsFilesShared(CFBundleRef bundle, Boolean flag) {
    bundle->_sharesStringsFiles = flag;
}

Boolean _CFBundleGetStringsFilesShared(CFBundleRef bundle) {
    return bundle->_sharesStringsFiles;
}

static Boolean _urlExists(CFURLRef url) {
    Boolean exists;
    return url && (0 == _CFGetFileProperties(kCFAllocatorSystemDefault, url, &exists, NULL, NULL, NULL, NULL, NULL)) && exists;
}

// This is here because on iPhoneOS with the dyld shared cache, we remove binaries from their
// original locations on disk, so checking whether a binary's path exists is no longer sufficient.
// For performance reasons, we only call dlopen_preflight() after we've verified that the binary 
// does not exist at its original path with _urlExists().
// See <rdar://problem/6956670>
static Boolean _binaryLoadable(CFURLRef url) {
    Boolean loadable = _urlExists(url);
#if DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    if (!loadable) {
	uint8_t path[PATH_MAX];
	if (url && CFURLGetFileSystemRepresentation(url, true, path, sizeof(path))) {
	    loadable = dlopen_preflight((char *)path);
	}
    }
#endif
    return loadable;
}

CF_PRIVATE CFURLRef _CFBundleCopySupportFilesDirectoryURLInDirectory(CFURLRef bundleURL, uint8_t version) {
    CFURLRef result = NULL;
    if (bundleURL) {
        if (1 == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleSupportFilesURLFromBase1, bundleURL);
        } else if (2 == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleSupportFilesURLFromBase2, bundleURL);
        } else {
            result = (CFURLRef)CFRetain(bundleURL);
        }
    }
    return result;
}

CF_EXPORT CFURLRef CFBundleCopySupportFilesDirectoryURL(CFBundleRef bundle) {
    return _CFBundleCopySupportFilesDirectoryURLInDirectory(bundle->_url, bundle->_version);
}

CF_PRIVATE CFURLRef _CFBundleCopyResourcesDirectoryURLInDirectory(CFURLRef bundleURL, uint8_t version) {
    CFURLRef result = NULL;
    if (bundleURL) {
        if (0 == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleResourcesURLFromBase0, bundleURL);
        } else if (1 == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleResourcesURLFromBase1, bundleURL);
        } else if (2 == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleResourcesURLFromBase2, bundleURL);
        } else {
            result = (CFURLRef)CFRetain(bundleURL);
        }
    }
    return result;
}

CF_EXPORT CFURLRef CFBundleCopyResourcesDirectoryURL(CFBundleRef bundle) {
    return _CFBundleCopyResourcesDirectoryURLInDirectory(bundle->_url, bundle->_version);
}

CF_PRIVATE CFURLRef _CFBundleCopyAppStoreReceiptURLInDirectory(CFURLRef bundleURL, uint8_t version) {
    CFURLRef result = NULL;
    if (bundleURL) {
        if (0 == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleAppStoreReceiptURLFromBase0, bundleURL);
        } else if (1 == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleAppStoreReceiptURLFromBase1, bundleURL);
        } else if (2 == version) {
            result = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleAppStoreReceiptURLFromBase2, bundleURL);
        }
    }
    return result;
}

CFURLRef _CFBundleCopyAppStoreReceiptURL(CFBundleRef bundle) {
    return _CFBundleCopyAppStoreReceiptURLInDirectory(bundle->_url, bundle->_version);
}
        
static CFURLRef _CFBundleCopyExecutableURLRaw(CFURLRef urlPath, CFStringRef exeName) {
    // Given an url to a folder and a name, this returns the url to the executable in that folder with that name, if it exists, and NULL otherwise.  This function deals with appending the ".exe" or ".dll" on Windows.
    CFURLRef executableURL = NULL;
    if (!urlPath || !exeName) return NULL;
    
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    const uint8_t *image_suffix = (uint8_t *)__CFgetenv("DYLD_IMAGE_SUFFIX");
    if (image_suffix) {
        CFStringRef newExeName, imageSuffix;
        imageSuffix = CFStringCreateWithCString(kCFAllocatorSystemDefault, (char *)image_suffix, kCFStringEncodingUTF8);
        if (CFStringHasSuffix(exeName, CFSTR(".dylib"))) {
            CFStringRef bareExeName = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, exeName, CFRangeMake(0, CFStringGetLength(exeName)-6));
            newExeName = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@%@.dylib"), exeName, imageSuffix);
            CFRelease(bareExeName);
        } else {
            newExeName = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@%@"), exeName, imageSuffix);
        }
        executableURL = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, newExeName, kCFURLPOSIXPathStyle, false, urlPath);
        if (executableURL && !_binaryLoadable(executableURL)) {
            CFRelease(executableURL);
            executableURL = NULL;
        }
        CFRelease(newExeName);
        CFRelease(imageSuffix);
    }
    if (!executableURL) {
        executableURL = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, exeName, kCFURLPOSIXPathStyle, false, urlPath);
        if (executableURL && !_binaryLoadable(executableURL)) {
            CFRelease(executableURL);
            executableURL = NULL;
        }
    }
#elif DEPLOYMENT_TARGET_WINDOWS
    if (!executableURL) {
        executableURL = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, exeName, kCFURLWindowsPathStyle, false, urlPath);
        if (executableURL && !_urlExists(executableURL)) {
            CFRelease(executableURL);
            executableURL = NULL;
        }
    }
    if (!executableURL) {
        if (!CFStringFindWithOptions(exeName, CFSTR(".dll"), CFRangeMake(0, CFStringGetLength(exeName)), kCFCompareAnchored|kCFCompareBackwards|kCFCompareCaseInsensitive, NULL)) {
#if defined(DEBUG)
            CFStringRef extension = CFSTR("_debug.dll");
#else
            CFStringRef extension = CFSTR(".dll");
#endif
            CFStringRef newExeName = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@%@"), exeName, extension);
            executableURL = CFURLCreateWithString(kCFAllocatorSystemDefault, newExeName, urlPath);
            if (executableURL && !_binaryLoadable(executableURL)) {
                CFRelease(executableURL);
                executableURL = NULL;
            }
            CFRelease(newExeName);
        }
    }
    if (!executableURL) {
        if (!CFStringFindWithOptions(exeName, CFSTR(".exe"), CFRangeMake(0, CFStringGetLength(exeName)), kCFCompareAnchored|kCFCompareBackwards|kCFCompareCaseInsensitive, NULL)) {
#if defined(DEBUG)
            CFStringRef extension = CFSTR("_debug.exe");
#else
            CFStringRef extension = CFSTR(".exe");
#endif
            CFStringRef newExeName = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@%@"), exeName, extension);
            executableURL = CFURLCreateWithString(kCFAllocatorSystemDefault, newExeName, urlPath);
            if (executableURL && !_binaryLoadable(executableURL)) {
                CFRelease(executableURL);
                executableURL = NULL;
            }
            CFRelease(newExeName);
        }
    }
#endif
    return executableURL;
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

static CFURLRef _CFBundleCopyExecutableURLInDirectory2(CFBundleRef bundle, CFURLRef url, CFStringRef executableName, Boolean ignoreCache, Boolean useOtherPlatform) {
    uint8_t version = 0;
    CFDictionaryRef infoDict = NULL;
    CFStringRef executablePath = NULL;
    CFURLRef executableURL = NULL;
    Boolean foundIt = false;
    Boolean lookupMainExe = (executableName ? false : true);
    
    if (bundle) {
        infoDict = CFBundleGetInfoDictionary(bundle);
        version = bundle->_version;
    } else {
        infoDict = _CFBundleCopyInfoDictionaryInDirectory(kCFAllocatorSystemDefault, url, &version);
    }
    
    // If we have a bundle instance and an info dict, see if we have already cached the path
    if (lookupMainExe && !ignoreCache && !useOtherPlatform && bundle && bundle->_executablePath) {
        __CFLock(&bundle->_lock);
        executablePath = bundle->_executablePath;
        if (executablePath) CFRetain(executablePath);
        __CFUnlock(&bundle->_lock);
        if (executablePath) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
            executableURL = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, executablePath, kCFURLPOSIXPathStyle, false);
#elif DEPLOYMENT_TARGET_WINDOWS
            executableURL = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, executablePath, kCFURLWindowsPathStyle, false);
#endif
            if (executableURL) {
                foundIt = true;
            }
            CFRelease(executablePath);
        }
    }
    
    if (!foundIt) {
        if (lookupMainExe) executableName = _CFBundleCopyExecutableName(bundle, url, infoDict);
        if (executableName) {
#if (DEPLOYMENT_TARGET_EMBEDDED && !TARGET_IPHONE_SIMULATOR)
            Boolean doExecSearch = false;
#else
            Boolean doExecSearch = true;
#endif
            // Now, look for the executable inside the bundle.
            if (doExecSearch && 0 != version) {
                CFURLRef exeDirURL = NULL;
                CFURLRef exeSubdirURL;
                
                if (1 == version) {
                    exeDirURL = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleExecutablesURLFromBase1, url);
                } else if (2 == version) {
                    exeDirURL = CFURLCreateWithString(kCFAllocatorSystemDefault, _CFBundleExecutablesURLFromBase2, url);
                } else {
#if DEPLOYMENT_TARGET_WINDOWS
                    // On Windows, if the bundle URL is foo.resources, then the executable is at the same level as the .resources directory
                    CFStringRef extension = CFURLCopyPathExtension(url);
                    if (extension && CFEqual(extension, _CFBundleWindowsResourceDirectoryExtension)) {
                        exeDirURL = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorSystemDefault, url);
                    } else {
                        exeDirURL = (CFURLRef)CFRetain(url);
                    }
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
                    exeDirURL = (CFURLRef)CFRetain(url);
#endif
                }
                CFStringRef platformSubDir = useOtherPlatform ? _CFBundleGetOtherPlatformExecutablesSubdirectoryName() : _CFBundleGetPlatformExecutablesSubdirectoryName();
                exeSubdirURL = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, platformSubDir, kCFURLPOSIXPathStyle, true, exeDirURL);
                executableURL = _CFBundleCopyExecutableURLRaw(exeSubdirURL, executableName);
                if (!executableURL) {
                    CFRelease(exeSubdirURL);
                    platformSubDir = useOtherPlatform ? _CFBundleGetOtherAlternatePlatformExecutablesSubdirectoryName() : _CFBundleGetAlternatePlatformExecutablesSubdirectoryName();
                    exeSubdirURL = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, platformSubDir, kCFURLPOSIXPathStyle, true, exeDirURL);
                    executableURL = _CFBundleCopyExecutableURLRaw(exeSubdirURL, executableName);
                }
                if (!executableURL) {
                    CFRelease(exeSubdirURL);
                    platformSubDir = useOtherPlatform ? _CFBundleGetPlatformExecutablesSubdirectoryName() : _CFBundleGetOtherPlatformExecutablesSubdirectoryName();
                    exeSubdirURL = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, platformSubDir, kCFURLPOSIXPathStyle, true, exeDirURL);
                    executableURL = _CFBundleCopyExecutableURLRaw(exeSubdirURL, executableName);
                }
                if (!executableURL) {
                    CFRelease(exeSubdirURL);
                    platformSubDir = useOtherPlatform ? _CFBundleGetAlternatePlatformExecutablesSubdirectoryName() : _CFBundleGetOtherAlternatePlatformExecutablesSubdirectoryName();
                    exeSubdirURL = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, platformSubDir, kCFURLPOSIXPathStyle, true, exeDirURL);
                    executableURL = _CFBundleCopyExecutableURLRaw(exeSubdirURL, executableName);
                }
                if (!executableURL) executableURL = _CFBundleCopyExecutableURLRaw(exeDirURL, executableName);
                CFRelease(exeDirURL);
                CFRelease(exeSubdirURL);
            }
            
            // If this was an old bundle, or we did not find the executable in the Executables subdirectory, look directly in the bundle wrapper.
            if (!executableURL) executableURL = _CFBundleCopyExecutableURLRaw(url, executableName);
            
#if DEPLOYMENT_TARGET_WINDOWS
            // Windows only: If we still haven't found the exe, look in the Executables folder.
            // But only for the main bundle exe
            if (lookupMainExe && !executableURL) {
                CFURLRef exeDirURL = CFURLCreateWithString(kCFAllocatorSystemDefault, CFSTR("../../Executables"), url);
                executableURL = _CFBundleCopyExecutableURLRaw(exeDirURL, executableName);
                CFRelease(exeDirURL);
            }
#endif
            
            if (lookupMainExe && !ignoreCache && !useOtherPlatform && bundle && executableURL) {
                // We found it.  Cache the path.
                CFURLRef absURL = CFURLCopyAbsoluteURL(executableURL);
#if DEPLOYMENT_TARGET_WINDOWS
                executablePath = CFURLCopyFileSystemPath(absURL, kCFURLWindowsPathStyle);
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
                executablePath = CFURLCopyFileSystemPath(absURL, kCFURLPOSIXPathStyle);
#endif
                CFRelease(absURL);
                __CFLock(&bundle->_lock);
                bundle->_executablePath = (CFStringRef)CFRetain(executablePath);
                __CFUnlock(&bundle->_lock);
                CFRelease(executablePath);
            }
            if (lookupMainExe && !useOtherPlatform && bundle && !executableURL) bundle->_binaryType = __CFBundleNoBinary;
            if (lookupMainExe) CFRelease(executableName);
        }
    }
    if (!bundle && infoDict && !(0)) CFRelease(infoDict);
    return executableURL;
}


CFURLRef _CFBundleCopyExecutableURLInDirectory(CFURLRef url) {
    return _CFBundleCopyExecutableURLInDirectory2(NULL, url, NULL, true, false);
}

CFURLRef _CFBundleCopyOtherExecutableURLInDirectory(CFURLRef url) {
    return _CFBundleCopyExecutableURLInDirectory2(NULL, url, NULL, true, true);
}

CFURLRef CFBundleCopyExecutableURL(CFBundleRef bundle) {
    return _CFBundleCopyExecutableURLInDirectory2(bundle, bundle->_url, NULL, false, false);
}

static CFURLRef _CFBundleCopyExecutableURLIgnoringCache(CFBundleRef bundle) {
    return _CFBundleCopyExecutableURLInDirectory2(bundle, bundle->_url, NULL, true, false);
}

CFURLRef CFBundleCopyAuxiliaryExecutableURL(CFBundleRef bundle, CFStringRef executableName) {
    return _CFBundleCopyExecutableURLInDirectory2(bundle, bundle->_url, executableName, true, false);
}

Boolean CFBundleIsExecutableLoaded(CFBundleRef bundle) {
    return bundle->_isLoaded;
}

CFBundleExecutableType CFBundleGetExecutableType(CFBundleRef bundle) {
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
        userInfoKeys[numKeys] = CFSTR("NSDebugDescription");
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
    Boolean result = false;
    CFErrorRef localError = NULL, *subError = (error ? &localError : NULL);
    CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);


    pthread_mutex_lock(&(bundle->_bundleLoadingLock));
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
        pthread_mutex_unlock(&(bundle->_bundleLoadingLock));
        // Remove from the scheduled unload set if we are there.
        pthread_mutex_lock(&CFBundleGlobalDataLock);
#if AVOID_WEAK_COLLECTIONS
        if (_bundlesToUnload) CFSetRemoveValue(_bundlesToUnload, bundle);
#else /* AVOID_WEAK_COLLECTIONS */
        if (_bundlesToUnload) [_bundlesToUnload removeObject:(id)bundle];
#endif /* AVOID_WEAK_COLLECTIONS */
        pthread_mutex_unlock(&CFBundleGlobalDataLock);
        return true;
    }

    // Unload bundles scheduled for unloading
    if (!_scheduledBundlesAreUnloading) {
        pthread_mutex_unlock(&(bundle->_bundleLoadingLock));
        _CFBundleUnloadScheduledBundles();
        pthread_mutex_lock(&(bundle->_bundleLoadingLock));
    }
    
    if (bundle->_isLoaded) {
        pthread_mutex_unlock(&(bundle->_bundleLoadingLock));
        // Remove from the scheduled unload set if we are there.
        pthread_mutex_lock(&CFBundleGlobalDataLock);
#if AVOID_WEAK_COLLECTIONS
        if (_bundlesToUnload) CFSetRemoveValue(_bundlesToUnload, bundle);
#else /* AVOID_WEAK_COLLECTIONS */
        if (_bundlesToUnload) [_bundlesToUnload removeObject:(id)bundle];
#endif /* AVOID_WEAK_COLLECTIONS */
        pthread_mutex_unlock(&CFBundleGlobalDataLock);
        return true;
    }
    pthread_mutex_unlock(&(bundle->_bundleLoadingLock));

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
    if (result && bundle->_plugInData._isPlugIn) _CFBundlePlugInLoaded(bundle);
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

    pthread_mutex_lock(&(bundle->_bundleLoadingLock));
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
        pthread_mutex_unlock(&(bundle->_bundleLoadingLock));
        return true;
    }
    pthread_mutex_unlock(&(bundle->_bundleLoadingLock));
    
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
    // First unload bundles scheduled for unloading (if that's not what we are already doing.)
    if (!_scheduledBundlesAreUnloading) _CFBundleUnloadScheduledBundles();
    
    if (!bundle->_isLoaded) return;

    // Remove from the scheduled unload set if we are there.
    if (!_scheduledBundlesAreUnloading) pthread_mutex_lock(&CFBundleGlobalDataLock);
#if AVOID_WEAK_COLLECTIONS
    if (_bundlesToUnload) CFSetRemoveValue(_bundlesToUnload, bundle);
#else /* AVOID_WEAK_COLLECTIONS */
    if (_bundlesToUnload) [_bundlesToUnload removeObject:(id)bundle];
#endif /* AVOID_WEAK_COLLECTIONS */
    if (!_scheduledBundlesAreUnloading) pthread_mutex_unlock(&CFBundleGlobalDataLock);
    
    // Give the plugIn code a chance to realize this...
    _CFPlugInWillUnload(bundle);

    pthread_mutex_lock(&(bundle->_bundleLoadingLock));
    if (!bundle->_isLoaded) {
        pthread_mutex_unlock(&(bundle->_bundleLoadingLock));
        return;
    }
    pthread_mutex_unlock(&(bundle->_bundleLoadingLock));

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
    if (!bundle->_isLoaded && bundle->_glueDict) {
        CFDictionaryApplyFunction(bundle->_glueDict, _CFBundleDeallocateGlue, (void *)CFGetAllocator(bundle));
        CFRelease(bundle->_glueDict);
        bundle->_glueDict = NULL;
    }
}

#if AVOID_WEAK_COLLECTIONS

CF_PRIVATE void _CFBundleScheduleForUnloading(CFBundleRef bundle) {
    pthread_mutex_lock(&CFBundleGlobalDataLock);
    if (!_bundlesToUnload) {
        CFSetCallBacks nonRetainingCallbacks = kCFTypeSetCallBacks;
        nonRetainingCallbacks.retain = NULL;
        nonRetainingCallbacks.release = NULL;
        _bundlesToUnload = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &nonRetainingCallbacks);
    }
    CFSetAddValue(_bundlesToUnload, bundle);
    pthread_mutex_unlock(&CFBundleGlobalDataLock);
}

CF_PRIVATE void _CFBundleUnscheduleForUnloading(CFBundleRef bundle) {
    pthread_mutex_lock(&CFBundleGlobalDataLock);
    if (_bundlesToUnload) CFSetRemoveValue(_bundlesToUnload, bundle);
    pthread_mutex_unlock(&CFBundleGlobalDataLock);
}

CF_PRIVATE void _CFBundleUnloadScheduledBundles(void) {
    pthread_mutex_lock(&CFBundleGlobalDataLock);
    if (_bundlesToUnload) {
        CFIndex i, c = CFSetGetCount(_bundlesToUnload);
        if (c > 0) {
            CFBundleRef *unloadThese = (CFBundleRef *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(CFBundleRef) * c, 0);
            CFSetGetValues(_bundlesToUnload, (const void **)unloadThese);
            _scheduledBundlesAreUnloading = true;
            for (i = 0; i < c; i++) {
                // This will cause them to be removed from the set.  (Which is why we copied all the values out of the set up front.)
                CFBundleUnloadExecutable(unloadThese[i]);
            }
            _scheduledBundlesAreUnloading = false;
            CFAllocatorDeallocate(kCFAllocatorSystemDefault, unloadThese);
        }
    }
    pthread_mutex_unlock(&CFBundleGlobalDataLock);
}

#else /* AVOID_WEAK_COLLECTIONS */

CF_PRIVATE void _CFBundleScheduleForUnloading(CFBundleRef bundle) {
    pthread_mutex_lock(&CFBundleGlobalDataLock);
    if (!_bundlesToUnload) _bundlesToUnload = [[__CFHashTable alloc] initWithOptions:CFPointerFunctionsZeroingWeakMemory capacity:0];
    [_bundlesToUnload addObject:(id)bundle];
    pthread_mutex_unlock(&CFBundleGlobalDataLock);
}

CF_PRIVATE void _CFBundleUnscheduleForUnloading(CFBundleRef bundle) {
    pthread_mutex_lock(&CFBundleGlobalDataLock);
    if (_bundlesToUnload) [_bundlesToUnload removeObject:(id)bundle];
    pthread_mutex_unlock(&CFBundleGlobalDataLock);
}

CF_PRIVATE void _CFBundleUnloadScheduledBundles(void) {
    pthread_mutex_lock(&CFBundleGlobalDataLock);
    if (_bundlesToUnload && [_bundlesToUnload count] > 0) {
        CFIndex i, c;
        CFMutableArrayRef unloadThese = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
        for (id value in _bundlesToUnload) CFArrayAppendValue(unloadThese, value);
        c = CFArrayGetCount(unloadThese);
        if (c > 0) {
            _scheduledBundlesAreUnloading = true;
            for (i = 0; i < c; i++) {
                // This will cause them to be removed from the set.  (Which is why we copied all the values out of the set up front.)
                CFBundleUnloadExecutable((CFBundleRef)CFArrayGetValueAtIndex(unloadThese, i));
            }
            _scheduledBundlesAreUnloading = false;
        }
        CFRelease(unloadThese);
    }
    pthread_mutex_unlock(&CFBundleGlobalDataLock);
}

#endif /* AVOID_WEAK_COLLECTIONS */

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
#if DEPLOYMENT_TARGET_WINDOWS
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

#if DEPLOYMENT_TARGET_WINDOWS
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

static void _CFBundleEnsureBundleExistsForImagePath(CFStringRef imagePath) {
    // This finds the bundle for the given path.
    // If an image path corresponds to a bundle, we see if there is already a bundle instance.  If there is and it is NOT in the _dynamicBundles array, it is added to the staticBundles.  Do not add the main bundle to the list here.
    CFBundleRef bundle;
    CFURLRef curURL = __CFBundleCopyFrameworkURLForExecutablePath(imagePath, true);
    Boolean createdBundle = false;

    if (curURL) {
        bundle = _CFBundleCopyBundleForURL(curURL, true);
        if (!bundle) {
            // Ensure bundle exists by creating it if necessary
            // NB doFinalProcessing must be false here, see below
            bundle = _CFBundleCreate(kCFAllocatorSystemDefault, curURL, true, false);
            createdBundle = true;
        }
        if (bundle) {
            pthread_mutex_lock(&(bundle->_bundleLoadingLock));
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
            pthread_mutex_unlock(&(bundle->_bundleLoadingLock));
            if (createdBundle) {
                // Perform delayed final processing steps.
                // This must be done after _isLoaded has been set, for security reasons (3624341).
                if (_CFBundleNeedsInitPlugIn(bundle)) {
                    pthread_mutex_unlock(&CFBundleGlobalDataLock);
                    _CFBundleInitPlugIn(bundle);
                    pthread_mutex_lock(&CFBundleGlobalDataLock);
                }
            } else {
                // Release the bundle if we did not create it here
                CFRelease(bundle);
            }
        }
        CFRelease(curURL);
    }
}

static void _CFBundleEnsureBundlesExistForImagePaths(CFArrayRef imagePaths) {
    // This finds the bundles for the given paths.
    // If an image path corresponds to a bundle, we see if there is already a bundle instance.  If there is and it is NOT in the _dynamicBundles array, it is added to the staticBundles.  Do not add the main bundle to the list here (even if it appears in imagePaths).
    CFIndex i, imagePathCount = CFArrayGetCount(imagePaths);
    for (i = 0; i < imagePathCount; i++) _CFBundleEnsureBundleExistsForImagePath((CFStringRef)CFArrayGetValueAtIndex(imagePaths, i));
}

static void _CFBundleEnsureBundlesUpToDateWithHintAlreadyLocked(CFStringRef hint) {
    CFArrayRef imagePaths = NULL;
    // Tickle the main bundle into existence
    (void)_CFBundleGetMainBundleAlreadyLocked();
#if defined(BINARY_SUPPORT_DYLD)
    imagePaths = _CFBundleDYLDCopyLoadedImagePathsForHint(hint);
#endif /* BINARY_SUPPORT_DYLD */
    if (imagePaths) {
        _CFBundleEnsureBundlesExistForImagePaths(imagePaths);
        CFRelease(imagePaths);
    }
}

static void _CFBundleEnsureAllBundlesUpToDateAlreadyLocked(void) {
    // This method returns all the statically linked bundles.  This includes the main bundle as well as any frameworks that the process was linked against at launch time.  It does not include frameworks or opther bundles that were loaded dynamically.
    CFArrayRef imagePaths = NULL;
    // Tickle the main bundle into existence
    (void)_CFBundleGetMainBundleAlreadyLocked();

#if defined(BINARY_SUPPORT_DLL)
// Dont know how to find static bundles for DLLs
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
    // To answer this properly, we have to have created the static bundles!
#if !AVOID_WEAK_COLLECTIONS
    static CFMutableArrayRef externalAllBundles = NULL;
#endif /* AVOID_WEAK_COLLECTIONS */
    CFArrayRef bundles;
    pthread_mutex_lock(&CFBundleGlobalDataLock);
    _CFBundleEnsureAllBundlesUpToDateAlreadyLocked();
#if AVOID_WEAK_COLLECTIONS
    bundles = _allBundles;
#else /* AVOID_WEAK_COLLECTIONS */
    if (!externalAllBundles) {
        CFArrayCallBacks nonRetainingArrayCallbacks = kCFTypeArrayCallBacks;
        nonRetainingArrayCallbacks.retain = NULL;
        nonRetainingArrayCallbacks.release = NULL;
        externalAllBundles = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &nonRetainingArrayCallbacks);
    }
    CFArrayRemoveAllValues(externalAllBundles);
    for (id value in _allBundles) CFArrayAppendValue(externalAllBundles, value);
    bundles = externalAllBundles;
#endif /* AVOID_WEAK_COLLECTIONS */
    pthread_mutex_unlock(&CFBundleGlobalDataLock);
    return bundles;
}
        
CF_EXPORT CFArrayRef _CFBundleCopyAllBundles(void) {
    // To answer this properly, we have to have created the static bundles!
    pthread_mutex_lock(&CFBundleGlobalDataLock);
    _CFBundleEnsureAllBundlesUpToDateAlreadyLocked();
#if AVOID_WEAK_COLLECTIONS
    CFArrayRef bundles = CFArrayCreateCopy(kCFAllocatorSystemDefault, _allBundles);
#else /* AVOID_WEAK_COLLECTIONS */
    CFMutableArrayRef bundles = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    for (id value in _allBundles) CFArrayAppendValue(bundles, value);
#endif /* AVOID_WEAK_COLLECTIONS */
    pthread_mutex_unlock(&CFBundleGlobalDataLock);
    return bundles;
}

CF_PRIVATE uint8_t _CFBundleLayoutVersion(CFBundleRef bundle) {
    return bundle->_version;
}
     
CF_EXPORT CFURLRef _CFBundleCopyPrivateFrameworksURL(CFBundleRef bundle) {
    return CFBundleCopyPrivateFrameworksURL(bundle);
}

CF_EXPORT CFURLRef CFBundleCopyPrivateFrameworksURL(CFBundleRef bundle) {
    CFURLRef result = NULL;

    if (1 == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundlePrivateFrameworksURLFromBase1, bundle->_url);
    } else if (2 == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundlePrivateFrameworksURLFromBase2, bundle->_url);
    } else {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundlePrivateFrameworksURLFromBase0, bundle->_url);
    }
    return result;
}

CF_EXPORT CFURLRef _CFBundleCopySharedFrameworksURL(CFBundleRef bundle) {
    return CFBundleCopySharedFrameworksURL(bundle);
}

CF_EXPORT CFURLRef CFBundleCopySharedFrameworksURL(CFBundleRef bundle) {
    CFURLRef result = NULL;

    if (1 == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedFrameworksURLFromBase1, bundle->_url);
    } else if (2 == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedFrameworksURLFromBase2, bundle->_url);
    } else {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedFrameworksURLFromBase0, bundle->_url);
    }
    return result;
}

CF_EXPORT CFURLRef _CFBundleCopySharedSupportURL(CFBundleRef bundle) {
    return CFBundleCopySharedSupportURL(bundle);
}

CF_EXPORT CFURLRef CFBundleCopySharedSupportURL(CFBundleRef bundle) {
    CFURLRef result = NULL;

    if (1 == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedSupportURLFromBase1, bundle->_url);
    } else if (2 == bundle->_version) {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedSupportURLFromBase2, bundle->_url);
    } else {
        result = CFURLCreateWithString(CFGetAllocator(bundle), _CFBundleSharedSupportURLFromBase0, bundle->_url);
    }
    return result;
}

CF_PRIVATE CFURLRef _CFBundleCopyBuiltInPlugInsURL(CFBundleRef bundle) {
    return CFBundleCopyBuiltInPlugInsURL(bundle);
}

CF_EXPORT CFURLRef CFBundleCopyBuiltInPlugInsURL(CFBundleRef bundle) {
    CFURLRef result = NULL, alternateResult = NULL;

    CFAllocatorRef alloc = CFGetAllocator(bundle);
    if (1 == bundle->_version) {
        result = CFURLCreateWithString(alloc, _CFBundleBuiltInPlugInsURLFromBase1, bundle->_url);
    } else if (2 == bundle->_version) {
        result = CFURLCreateWithString(alloc, _CFBundleBuiltInPlugInsURLFromBase2, bundle->_url);
    } else {
        result = CFURLCreateWithString(alloc, _CFBundleBuiltInPlugInsURLFromBase0, bundle->_url);
    }
    if (!result || !_urlExists(result)) {
        if (1 == bundle->_version) {
            alternateResult = CFURLCreateWithString(alloc, _CFBundleAlternateBuiltInPlugInsURLFromBase1, bundle->_url);
        } else if (2 == bundle->_version) {
            alternateResult = CFURLCreateWithString(alloc, _CFBundleAlternateBuiltInPlugInsURLFromBase2, bundle->_url);
        } else {
            alternateResult = CFURLCreateWithString(alloc, _CFBundleAlternateBuiltInPlugInsURLFromBase0, bundle->_url);
        }
        if (alternateResult && _urlExists(alternateResult)) {
            if (result) CFRelease(result);
            result = alternateResult;
        } else {
            if (alternateResult) CFRelease(alternateResult);
        }
    }
    return result;
}

