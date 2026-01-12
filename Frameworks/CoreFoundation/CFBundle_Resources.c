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

/*      CFBundle_Resources.c
        Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
        Responsibility: Tony Parker
*/

#include "CFBundle_Internal.h"
#include <CoreFoundation/CFURLAccess.h>
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFByteOrder.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFPreferences.h>
#include <string.h>
#include "CFInternal.h"
#include <CoreFoundation/CFPriv.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>


#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#include <unistd.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

#if DEPLOYMENT_TARGET_WINDOWS
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define close _close
#define write _write
#define read _read
#define open _NS_open
#define stat _NS_stat
#define fstat _fstat
#define mkdir(a,b) _NS_mkdir(a)
#define rmdir _NS_rmdir
#define unlink _NS_unlink

#endif

#pragma mark -
#pragma mark Directory Contents and Caches

// These are here for compatibility, but they do nothing anymore
CF_EXPORT void _CFBundleFlushCachesForURL(CFURLRef url) { }
CF_EXPORT void _CFBundleFlushCaches(void) { }

CF_PRIVATE void _CFBundleFlushQueryTableCache(CFBundleRef bundle) {
    __CFLock(&bundle->_queryLock);
    if (bundle->_queryTable) {
        CFDictionaryRemoveAllValues(bundle->_queryTable);
    }
    __CFUnlock(&bundle->_queryLock);
}

#pragma mark -
#pragma mark Resource URL Lookup

static Boolean _CFIsResourceCommon(char *path, Boolean *isDir) {
    Boolean exists;
    SInt32 mode;
    if (_CFGetPathProperties(kCFAllocatorSystemDefault, path, &exists, &mode, NULL, NULL, NULL, NULL) == 0) {
        if (isDir) *isDir = ((exists && ((mode & S_IFMT) == S_IFDIR)) ? true : false);
        return (exists && (mode & 0444));
    }
    return false;
}

CF_PRIVATE Boolean _CFIsResourceAtURL(CFURLRef url, Boolean *isDir) {
    char path[CFMaxPathSize];
    if (!CFURLGetFileSystemRepresentation(url, true, (uint8_t *)path, CFMaxPathLength)) return false;
    
    return _CFIsResourceCommon(path, isDir);
}

CF_PRIVATE Boolean _CFIsResourceAtPath(CFStringRef path, Boolean *isDir) {
    char pathBuf[CFMaxPathSize];
    if (!CFStringGetFileSystemRepresentation(path, pathBuf, CFMaxPathSize)) return false;
    
    return _CFIsResourceCommon(pathBuf, isDir);
}


static CFStringRef _CFBundleGetResourceDirForVersion(uint8_t version) {
    if (1 == version) {
        return _CFBundleSupportFilesDirectoryName1WithResources;
    } else if (2 == version) {
        return _CFBundleSupportFilesDirectoryName2WithResources;
    } else if (0 == version) {
        return _CFBundleResourcesDirectoryName;
    }
    return CFSTR("");
}

CF_PRIVATE void _CFBundleAppendResourceDir(CFMutableStringRef path, uint8_t version) {
    if (1 == version) {
        // /path/to/bundle/Support Files/
        CFStringAppend(path, _CFBundleSupportFilesDirectoryName1);
        _CFAppendTrailingPathSlash2(path);
    } else if (2 == version) {
        // /path/to/bundle/Contents/
        CFStringAppend(path, _CFBundleSupportFilesDirectoryName2);
        _CFAppendTrailingPathSlash2(path);
    }
    if (0 == version || 1 == version || 2 == version) {
        // /path/to/bundle/<above>/Resources
        CFStringAppend(path, _CFBundleResourcesDirectoryName);
    }
}

CF_EXPORT CFURLRef CFBundleCopyResourceURL(CFBundleRef bundle, CFStringRef resourceName, CFStringRef resourceType, CFStringRef subDirName) {
    if (!bundle) return NULL;
    CFURLRef result = (CFURLRef) _CFBundleCopyFindResources(bundle, NULL, NULL, resourceName, resourceType, subDirName, NULL, false, false, NULL);
    return result;
}

CF_EXPORT CFArrayRef CFBundleCopyResourceURLsOfType(CFBundleRef bundle, CFStringRef resourceType, CFStringRef subDirName) {
    if (!bundle) return CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    CFArrayRef result = (CFArrayRef) _CFBundleCopyFindResources(bundle, NULL, NULL, NULL, resourceType, subDirName, NULL, true, false, NULL);
    return result;
}

CF_EXPORT CFURLRef _CFBundleCopyResourceURLForLanguage(CFBundleRef bundle, CFStringRef resourceName, CFStringRef resourceType, CFStringRef subDirName, CFStringRef language) {
    return CFBundleCopyResourceURLForLocalization(bundle, resourceName, resourceType, subDirName, language);
}

CF_EXPORT CFURLRef CFBundleCopyResourceURLForLocalization(CFBundleRef bundle, CFStringRef resourceName, CFStringRef resourceType, CFStringRef subDirName, CFStringRef localizationName) {
    if (!bundle) return NULL;
    CFURLRef result = (CFURLRef) _CFBundleCopyFindResources(bundle, NULL, NULL, resourceName, resourceType, subDirName, localizationName, false, true, NULL);
    return result;
}

CF_EXPORT CFArrayRef _CFBundleCopyResourceURLsOfTypeForLanguage(CFBundleRef bundle, CFStringRef resourceType, CFStringRef subDirName, CFStringRef language) {
    return CFBundleCopyResourceURLsOfTypeForLocalization(bundle, resourceType, subDirName, language);
}

CF_EXPORT CFArrayRef CFBundleCopyResourceURLsOfTypeForLocalization(CFBundleRef bundle, CFStringRef resourceType, CFStringRef subDirName, CFStringRef localizationName) {
    if (!bundle) return CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    CFArrayRef result = (CFArrayRef) _CFBundleCopyFindResources(bundle, NULL, NULL, NULL, resourceType, subDirName, localizationName, true, true, NULL);
    return result;
}

CF_EXPORT CFURLRef CFBundleCopyResourceURLInDirectory(CFURLRef bundleURL, CFStringRef resourceName, CFStringRef resourceType, CFStringRef subDirName) {
    CFURLRef result = NULL;
    unsigned char buff[CFMaxPathSize];
    CFURLRef newURL = NULL;
    
    if (!CFURLGetFileSystemRepresentation(bundleURL, true, buff, CFMaxPathSize)) return NULL;
    
    newURL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, buff, strlen((char *)buff), true);
    if (!newURL) newURL = (CFURLRef)CFRetain(bundleURL);
    if (_CFBundleCouldBeBundle(newURL)) {
        result = (CFURLRef) _CFBundleCopyFindResources(NULL, bundleURL, NULL, resourceName, resourceType, subDirName, NULL, false, false, NULL);
    }
    if (newURL) CFRelease(newURL);
    return result;
}

CF_EXPORT CFArrayRef CFBundleCopyResourceURLsOfTypeInDirectory(CFURLRef bundleURL, CFStringRef resourceType, CFStringRef subDirName) {
    CFArrayRef array = NULL;
    unsigned char buff[CFMaxPathSize];
    CFURLRef newURL = NULL;
    
    if (!CFURLGetFileSystemRepresentation(bundleURL, true, buff, CFMaxPathSize)) return NULL;
    
    newURL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, buff, strlen((char *)buff), true);
    if (!newURL) newURL = (CFURLRef)CFRetain(bundleURL);
    if (_CFBundleCouldBeBundle(newURL)) {
        array = (CFArrayRef) _CFBundleCopyFindResources(NULL, bundleURL, NULL, NULL, resourceType, subDirName, NULL, true, false, NULL);
    }
    if (newURL) CFRelease(newURL);
    return array;
}

#pragma mark -

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_WINDOWS
// Note that subDirName is expected to be the string for a URL
CF_INLINE Boolean _CFBundleURLHasSubDir(CFURLRef url, CFStringRef subDirName) {
    Boolean isDir = false, result = false;
    CFURLRef dirURL = CFURLCreateWithString(kCFAllocatorSystemDefault, subDirName, url);
    if (dirURL) {
        if (_CFIsResourceAtURL(dirURL, &isDir) && isDir) result = true;
        CFRelease(dirURL);
    }
    return result;
}
#endif

CF_PRIVATE uint8_t _CFBundleGetBundleVersionForURL(CFURLRef url) {
    // check for existence of "Resources" or "Contents" or "Support Files"
    // but check for the most likely one first
    // version 0:  old-style "Resources" bundles
    // version 1:  obsolete "Support Files" bundles
    // version 2:  modern "Contents" bundles
    // version 3:  none of the above (see below)
    // version 4:  not a bundle (for main bundle only)
    
    CFURLRef absoluteURL = CFURLCopyAbsoluteURL(url);
    CFStringRef directoryPath = CFURLCopyFileSystemPath(absoluteURL, PLATFORM_PATH_STYLE);
    CFRelease(absoluteURL);
    
    Boolean hasFrameworkSuffix = CFStringHasSuffix(CFURLGetString(url), CFSTR(".framework/"));
#if DEPLOYMENT_TARGET_WINDOWS
    hasFrameworkSuffix = hasFrameworkSuffix || CFStringHasSuffix(CFURLGetString(url), CFSTR(".framework\\"));
#endif

    /*
     #define _CFBundleSupportFilesDirectoryName1 CFSTR("Support Files")
     #define _CFBundleSupportFilesDirectoryName2 CFSTR("Contents")
     #define _CFBundleResourcesDirectoryName CFSTR("Resources")
     #define _CFBundleExecutablesDirectoryName CFSTR("Executables")
     #define _CFBundleNonLocalizedResourcesDirectoryName CFSTR("Non-localized Resources")
    */
    __block uint8_t localVersion = 3;
    CFIndex resourcesDirectoryLength = CFStringGetLength(_CFBundleResourcesDirectoryName);
    CFIndex contentsDirectoryLength = CFStringGetLength(_CFBundleSupportFilesDirectoryName2);
    CFIndex supportFilesDirectoryLength = CFStringGetLength(_CFBundleSupportFilesDirectoryName1);
    
    __block Boolean foundResources = false;
    __block Boolean foundSupportFiles2 = false;
    __block Boolean foundSupportFiles1 = false;
    
    _CFIterateDirectory(directoryPath, ^Boolean (CFStringRef fileName, uint8_t fileType) {
        // We're looking for a few different names, and also some info on if it's a directory or not.
        // We don't stop looking once we find one of the names. Otherwise we could run into the situation where we have both "Contents" and "Resources" in a framework, and we see Contents first but Resources is more important.
        if (fileType == DT_DIR || fileType == DT_LNK) {
            CFIndex fileNameLen = CFStringGetLength(fileName);
            if (fileNameLen == resourcesDirectoryLength && CFStringCompareWithOptions(fileName, _CFBundleResourcesDirectoryName, CFRangeMake(0, resourcesDirectoryLength), kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                foundResources = true;
            } else if (fileNameLen == contentsDirectoryLength && CFStringCompareWithOptions(fileName, _CFBundleSupportFilesDirectoryName2, CFRangeMake(0, contentsDirectoryLength), kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                foundSupportFiles2 = true;
            } else if (fileNameLen == supportFilesDirectoryLength && CFStringCompareWithOptions(fileName, _CFBundleSupportFilesDirectoryName1, CFRangeMake(0, supportFilesDirectoryLength), kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                foundSupportFiles1 = true;
            }
        }
        return true;
    });
    
    // The order of these if statements is important - the Resources directory presence takes precedence over Contents, and so forth.
    if (hasFrameworkSuffix) {
        if (foundResources) {
            localVersion = 0;
        } else if (foundSupportFiles2) {
            localVersion = 2;
        } else if (foundSupportFiles1) {
            localVersion = 1;
        }
    } else {
        if (foundSupportFiles2) {
            localVersion = 2;
        } else if (foundResources) {
            localVersion = 0;
        } else if (foundSupportFiles1) {
            localVersion = 1;
        }
    }
    
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_WINDOWS
    // Do a more substantial check for the subdirectories that make up version 0/1/2 bundles. These are sometimes symlinks (like in Frameworks) and they would have been missed by our check above. Perhaps we can do a check for DT_LNK there as well, if it's sufficient instead of looking at the actual contents.
    if (localVersion == 3) {
        if (hasFrameworkSuffix) {
            if (_CFBundleURLHasSubDir(url, _CFBundleResourcesURLFromBase0)) localVersion = 0;
            else if (_CFBundleURLHasSubDir(url, _CFBundleSupportFilesURLFromBase2)) localVersion = 2;
            else if (_CFBundleURLHasSubDir(url, _CFBundleSupportFilesURLFromBase1)) localVersion = 1;
        } else {
            if (_CFBundleURLHasSubDir(url, _CFBundleSupportFilesURLFromBase2)) localVersion = 2;
            else if (_CFBundleURLHasSubDir(url, _CFBundleResourcesURLFromBase0)) localVersion = 0;
            else if (_CFBundleURLHasSubDir(url, _CFBundleSupportFilesURLFromBase1)) localVersion = 1;
        }
    }
#endif

    CFRelease(directoryPath);
    return localVersion;
}

#pragma mark -
#pragma mark Platforms

CF_EXPORT CFArrayRef _CFBundleGetSupportedPlatforms(CFBundleRef bundle) {
    // This function is obsolete
    return NULL;
}

CF_EXPORT CFStringRef _CFBundleGetCurrentPlatform(void) {
#if DEPLOYMENT_TARGET_MACOSX
    return CFSTR("MacOS");
#elif DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    return CFSTR("iPhoneOS");
#elif DEPLOYMENT_TARGET_WINDOWS
    return CFSTR("Windows");
#elif DEPLOYMENT_TARGET_SOLARIS
    return CFSTR("Solaris");
#elif DEPLOYMENT_TARGET_HPUX
    return CFSTR("HPUX");
#elif DEPLOYMENT_TARGET_LINUX
    return CFSTR("Linux");
#elif DEPLOYMENT_TARGET_FREEBSD
    return CFSTR("FreeBSD");
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
}

CF_PRIVATE CFStringRef _CFBundleGetPlatformExecutablesSubdirectoryName(void) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    return CFSTR("MacOS");
#elif DEPLOYMENT_TARGET_WINDOWS
    return CFSTR("Windows");
#elif DEPLOYMENT_TARGET_SOLARIS
    return CFSTR("Solaris");
#elif DEPLOYMENT_TARGET_HPUX
    return CFSTR("HPUX");
#elif DEPLOYMENT_TARGET_LINUX
    return CFSTR("Linux");
#elif DEPLOYMENT_TARGET_FREEBSD
    return CFSTR("FreeBSD");
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
}

CF_PRIVATE CFStringRef _CFBundleGetAlternatePlatformExecutablesSubdirectoryName(void) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    return CFSTR("Mac OS X");
#elif DEPLOYMENT_TARGET_WINDOWS
    return CFSTR("WinNT");
#elif DEPLOYMENT_TARGET_SOLARIS
    return CFSTR("Solaris");
#elif DEPLOYMENT_TARGET_HPUX
    return CFSTR("HP-UX");
#elif DEPLOYMENT_TARGET_LINUX
    return CFSTR("Linux");
#elif DEPLOYMENT_TARGET_FREEBSD
    return CFSTR("FreeBSD");
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
}

CF_PRIVATE CFStringRef _CFBundleGetOtherPlatformExecutablesSubdirectoryName(void) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    return CFSTR("MacOSClassic");
#elif DEPLOYMENT_TARGET_WINDOWS
    return CFSTR("Other");
#elif DEPLOYMENT_TARGET_HPUX
    return CFSTR("Other");
#elif DEPLOYMENT_TARGET_SOLARIS
    return CFSTR("Other");
#elif DEPLOYMENT_TARGET_LINUX
    return CFSTR("Other");
#elif DEPLOYMENT_TARGET_FREEBSD
    return CFSTR("Other");
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
}

CF_PRIVATE CFStringRef _CFBundleGetOtherAlternatePlatformExecutablesSubdirectoryName(void) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    return CFSTR("Mac OS 8");
#elif DEPLOYMENT_TARGET_WINDOWS
    return CFSTR("Other");
#elif DEPLOYMENT_TARGET_HPUX
    return CFSTR("Other");
#elif DEPLOYMENT_TARGET_SOLARIS
    return CFSTR("Other");
#elif DEPLOYMENT_TARGET_LINUX
    return CFSTR("Other");
#elif DEPLOYMENT_TARGET_FREEBSD
    return CFSTR("Other");
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
}

CFArrayRef CFBundleCopyExecutableArchitecturesForURL(CFURLRef url) {
    CFArrayRef result = NULL;
    CFBundleRef bundle = CFBundleCreate(kCFAllocatorSystemDefault, url);
    if (bundle) {
        result = CFBundleCopyExecutableArchitectures(bundle);
        CFRelease(bundle);
    } else {
        result = _CFBundleCopyArchitecturesForExecutable(url);
    }
    return result;
}

#pragma mark -
#pragma mark Resource Lookup - Query Table

static void _CFBundleAddValueForType(CFStringRef type, CFMutableDictionaryRef queryTable, CFMutableDictionaryRef typeDir, CFTypeRef value, CFMutableDictionaryRef addedTypes, Boolean firstLproj) {
    CFMutableArrayRef tFiles = (CFMutableArrayRef) CFDictionaryGetValue(typeDir, type);
    if (!tFiles) {
        CFStringRef key = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@.%@"), _CFBundleTypeIndicator, type);
        tFiles = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
        CFDictionarySetValue(queryTable, key, tFiles);
        CFDictionarySetValue(typeDir, type, tFiles);
        CFRelease(tFiles);
        CFRelease(key);
    }
    if (!addedTypes) {
        CFArrayAppendValue(tFiles, value);
    } else if (firstLproj) {
        CFDictionarySetValue(addedTypes, type, type);
        CFArrayAppendValue(tFiles, value);
    } else if (!(CFDictionaryGetValue(addedTypes, type))) {
        CFArrayAppendValue(tFiles, value);
    }
}

typedef enum {
    _CFBundleFileVersionNoProductNoPlatform = 1,
    _CFBundleFileVersionWithProductNoPlatform,
    _CFBundleFileVersionNoProductWithPlatform,
    _CFBundleFileVersionWithProductWithPlatform,
    _CFBundleFileVersionUnmatched
} _CFBundleFileVersion;

static _CFBundleFileVersion _CFBundleCheckFileProductAndPlatform(CFStringRef file, CFRange searchRange, CFStringRef product, CFStringRef platform)
{
    _CFBundleFileVersion version;
    Boolean foundprod, foundplat;
    foundplat = foundprod = NO;
    Boolean wrong = false;
    
    if (CFStringFindWithOptions(file, CFSTR("~"), searchRange, 0, NULL)) {
        if (CFStringGetLength(product) != 1) {
            // todo: really, search the same range again?
            if (CFStringFindWithOptions(file, product, searchRange, kCFCompareEqualTo, NULL)) {
                foundprod = YES;
            }
        }
        if (!foundprod) {
            wrong = _CFBundleSupportedProductName(file, searchRange);
        }
    }
    
    if (!wrong && CFStringFindWithOptions(file, CFSTR("-"), searchRange, 0, NULL)) {
        if (CFStringFindWithOptions(file, platform, searchRange, kCFCompareEqualTo, NULL)) {
            foundplat = YES;
        }
        if (!foundplat) {
            wrong = _CFBundleSupportedPlatformName(file, searchRange);
        }
    }
    
    if (wrong) {
        version = _CFBundleFileVersionUnmatched;
    } else if (foundplat && foundprod) {
        version = _CFBundleFileVersionWithProductWithPlatform;
    } else if (foundplat) {
        version = _CFBundleFileVersionNoProductWithPlatform;
    } else if (foundprod) {
        version = _CFBundleFileVersionWithProductNoPlatform;
    } else {
        version = _CFBundleFileVersionNoProductNoPlatform;
    }
    return version;
}

static _CFBundleFileVersion _CFBundleVersionForFileName(CFStringRef fileName, CFStringRef expectedProduct, CFStringRef expectedPlatform, CFRange *outProductRange, CFRange *outPlatformRange) {
    // Search for a product name, e.g.: foo~iphone.jpg or bar~ipad
    Boolean foundProduct = false;
    Boolean foundPlatform = false;
    CFIndex fileNameLen = CFStringGetLength(fileName);
    CFRange productRange;
    CFRange platformRange;
    
    CFIndex dotLocation = fileNameLen;
    for (CFIndex i = fileNameLen - 1; i > 0; i--) {
        UniChar c = CFStringGetCharacterAtIndex(fileName, i);
        if (c == '.') {
            dotLocation = i;
        }
#if DEPLOYMENT_TARGET_EMBEDDED
        // Product names are only supported on iOS
        // ref docs here: "iOS Supports Device-Specific Resources" in "Resource Programming Guide"
        else if (c == '~' && !foundProduct) {
            productRange = CFRangeMake(i, dotLocation - i);
            foundProduct = (CFStringCompareWithOptions(fileName, expectedProduct, productRange, kCFCompareAnchored) == kCFCompareEqualTo);
            if (foundProduct && outProductRange) *outProductRange = productRange;
        }
#endif
        else if (c == '-') {
            if (foundProduct) {
                platformRange = CFRangeMake(i, productRange.location - i);
            } else {
                platformRange = CFRangeMake(i, dotLocation - i);
            }
            foundPlatform = (CFStringCompareWithOptions(fileName, expectedPlatform, platformRange, kCFCompareAnchored) == kCFCompareEqualTo);
            if (foundPlatform && outPlatformRange) *outPlatformRange = platformRange;
            break;
        }
    }
    
    _CFBundleFileVersion version;
    if (foundPlatform && foundProduct) {
        version = _CFBundleFileVersionWithProductWithPlatform;
    } else if (foundPlatform) {
        version = _CFBundleFileVersionNoProductWithPlatform;
    } else if (foundProduct) {
        version = _CFBundleFileVersionWithProductNoPlatform;
    } else {
        version = _CFBundleFileVersionNoProductNoPlatform;
    }
    return version;
}

// Splits up a string into its various parts. Note that the out-types must be released by the caller if they exist.
static void _CFBundleSplitFileName(CFStringRef fileName, CFStringRef *noProductOrPlatform, CFStringRef *endType, CFStringRef *startType, CFStringRef expectedProduct, CFStringRef expectedPlatform, _CFBundleFileVersion *version) {
    CFIndex fileNameLen = CFStringGetLength(fileName);
    
    if (endType || startType) {
        // Search for the type from the end (type defined as everything after the last '.')
        // e.g., a file name like foo.jpg has a type of 'jpg'
        Boolean foundDot = false;
        uint16_t dotLocation = 0;
        for (CFIndex i = fileNameLen; i > 0; i--) {
            if (CFStringGetCharacterAtIndex(fileName, i - 1) == '.') {
                foundDot = true;
                dotLocation = i - 1;
                break;
            }
        }
        
        if (foundDot && dotLocation != fileNameLen - 1) {
            if (endType) *endType = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, fileName, CFRangeMake(dotLocation + 1, CFStringGetLength(fileName) - dotLocation - 1));
        }
        
        // Search for the type from the beginning (type defined as everything after the first '.')
        // e.g., a file name like foo.jpg.gz has a type of 'jpg.gz'
        if (startType) {
            for (CFIndex i = 0; i < fileNameLen; i++) {
                if (CFStringGetCharacterAtIndex(fileName, i) == '.') {
                    // no need to create this again if it's the same as previous
                    if (i != dotLocation) {
                        *startType = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, fileName, CFRangeMake(i + 1, CFStringGetLength(fileName) - i - 1));
                    }
                    break;
                }
            }
        }
    }
    
    CFRange productRange, platformRange;
    *version = _CFBundleVersionForFileName(fileName, expectedProduct, expectedPlatform, &productRange, &platformRange);
    
    Boolean foundPlatform = (*version == _CFBundleFileVersionNoProductWithPlatform || *version == _CFBundleFileVersionWithProductWithPlatform);
    Boolean foundProduct = (*version == _CFBundleFileVersionWithProductNoPlatform || *version == _CFBundleFileVersionWithProductWithPlatform);
    // Create a string that excludes both platform and product name
    // e.g., foo-iphone~iphoneos.jpg -> foo.jpg
    if (foundPlatform || foundProduct) {
        CFMutableStringRef fileNameScratch = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, fileName);
        CFIndex start, length = 0;
        
        // Because the platform always comes first and is immediately followed by product if it exists, we'll use the platform start location as the start of our range to delete.
        if (foundPlatform) {
            start = platformRange.location;
        } else {
            start = productRange.location;
        }
        
        if (foundPlatform && foundProduct) {
            length = platformRange.length + productRange.length;
        } else if (foundPlatform) {
            length = platformRange.length;
        } else if (foundProduct) {
            length = productRange.length;
        }
        CFStringDelete(fileNameScratch, CFRangeMake(start, length));
        *noProductOrPlatform = (CFStringRef)fileNameScratch;
    }    
}

static Boolean _CFBundleReadDirectory(CFStringRef pathOfDir, CFStringRef subdirectory, CFMutableArrayRef allFiles, Boolean hasFileAdded, CFMutableDictionaryRef queryTable, CFMutableDictionaryRef typeDir, CFMutableDictionaryRef addedTypes, Boolean firstLproj, CFStringRef product, CFStringRef platform, CFStringRef lprojName, Boolean appendLprojCharacters) {
    
    Boolean result = true;
    CFMutableStringRef pathPrefix = NULL;
    if (lprojName) {
        pathPrefix = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, lprojName);
        if (appendLprojCharacters) _CFAppendPathExtension2(pathPrefix, _CFBundleLprojExtension);
        _CFAppendTrailingPathSlash2(pathPrefix);
    }
    if (subdirectory) {
        if (pathPrefix) {
            CFStringAppend(pathPrefix, subdirectory);
        } else {
            pathPrefix = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, subdirectory);
        }
        UniChar lastChar = CFStringGetCharacterAtIndex(subdirectory, CFStringGetLength(subdirectory)-1);
        if (lastChar != _CFGetSlash()) {
            _CFAppendTrailingPathSlash2(pathPrefix);
        }
    }
    
    _CFIterateDirectory(pathOfDir, ^Boolean(CFStringRef fileName, uint8_t fileType) {
        CFStringRef startType = NULL, endType = NULL, noProductOrPlatform = NULL;
        _CFBundleFileVersion fileVersion;
        _CFBundleSplitFileName(fileName, &noProductOrPlatform, &endType, &startType, product, platform, &fileVersion);
        
        CFStringRef pathToFile;
        if (pathPrefix && CFStringGetLength(pathPrefix) > 0) {
            CFMutableStringRef tmp = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, pathPrefix);
            CFStringAppend(tmp, fileName);
            pathToFile = (CFStringRef)tmp;
        } else {
            pathToFile = (CFStringRef)CFRetain(fileName);
        }

        // If this file is a directory, the path needs to include a trailing slash so we can later create the right kind of CFURL object
        Boolean appendSlash = false;
        if (fileType == DT_DIR) {
            appendSlash = true;
        }
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
        else if (fileType == DT_UNKNOWN) {
            Boolean isDir = false;
            char subdirPath[CFMaxPathLength];
            struct stat statBuf;
            if (CFStringGetFileSystemRepresentation(pathOfDir, subdirPath, sizeof(subdirPath))) {
                strlcat(subdirPath, "/", sizeof(subdirPath));
                char fileNameBuf[CFMaxPathLength];
                if (CFStringGetFileSystemRepresentation(fileName, fileNameBuf, sizeof(fileNameBuf))) {
                    strlcat(subdirPath, fileNameBuf, sizeof(subdirPath));
                    if (stat(subdirPath, &statBuf) == 0) {
                        isDir = ((statBuf.st_mode & S_IFMT) == S_IFDIR);
                    }
                    if (isDir) {
                        appendSlash = true;
                    }
                }
            }
        }
#endif
        if (appendSlash) {
            // This is fairly inefficient
            CFMutableStringRef tmp = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, pathToFile);
            _CFAppendTrailingPathSlash2(tmp);
            CFRelease(pathToFile);
            pathToFile = (CFStringRef)tmp;
        }
        
        // put it into all file array
        if (!hasFileAdded) {
            CFArrayAppendValue(allFiles, pathToFile);
        }
        
        if (startType) {
            _CFBundleAddValueForType(startType, queryTable, typeDir, pathToFile, addedTypes, firstLproj);
        }
        
        if (endType) {
            _CFBundleAddValueForType(endType, queryTable, typeDir, pathToFile, addedTypes, firstLproj);
        }
                
        if (fileVersion == _CFBundleFileVersionNoProductNoPlatform || fileVersion == _CFBundleFileVersionUnmatched) {
            // No product/no platform, or unmatched files get added directly to the query table.
            CFStringRef prevPath = (CFStringRef)CFDictionaryGetValue(queryTable, fileName);
            if (!prevPath) {
                CFDictionarySetValue(queryTable, fileName, pathToFile);
            }
        } else {
            // If the file has a product or platform extension, we add the full name to the query table so that it may be found using that name. But only if it doesn't already exist.
            CFStringRef prevPath = (CFStringRef)CFDictionaryGetValue(queryTable, fileName);
            if (!prevPath) {
                CFDictionarySetValue(queryTable, fileName, pathToFile);
            }
            
            // Then we add the more specific name as well, replacing the existing one if this is a more specific version.
            if (noProductOrPlatform) {
                // add the path of the key into the query table
                prevPath = (CFStringRef) CFDictionaryGetValue(queryTable, noProductOrPlatform);
                if (!prevPath) {
                    CFDictionarySetValue(queryTable, noProductOrPlatform, pathToFile);
                } else {
                    if (!lprojName || CFStringHasPrefix(prevPath, lprojName)) {
                        // we need to know the version of exisiting path to see if we can replace it by the current path
                        CFRange searchRange;
                        if (lprojName) {
                            searchRange.location = CFStringGetLength(lprojName);
                            searchRange.length = CFStringGetLength(prevPath) - searchRange.location;
                        } else {
                            searchRange.location = 0;
                            searchRange.length = CFStringGetLength(prevPath);
                        }
                        _CFBundleFileVersion prevFileVersion = _CFBundleCheckFileProductAndPlatform(prevPath, searchRange, product, platform);
                        switch (prevFileVersion) {
                            case _CFBundleFileVersionNoProductNoPlatform:
                                CFDictionarySetValue(queryTable, noProductOrPlatform, pathToFile);
                                break;
                            case _CFBundleFileVersionWithProductNoPlatform:
                                if (fileVersion == _CFBundleFileVersionWithProductWithPlatform) CFDictionarySetValue(queryTable, noProductOrPlatform, pathToFile);
                                break;
                            case _CFBundleFileVersionNoProductWithPlatform:
                                CFDictionarySetValue(queryTable, noProductOrPlatform, pathToFile);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
        }
        
        if (pathToFile) CFRelease(pathToFile);
        if (startType) CFRelease(startType);
        if (endType) CFRelease(endType);
        if (noProductOrPlatform) CFRelease(noProductOrPlatform);
        
        return true;
    });
    
    if (pathPrefix) CFRelease(pathPrefix);
    return result;
}


static CFDictionaryRef _CFBundleCreateQueryTableAtPath(CFStringRef inPath, CFArrayRef languages, CFStringRef resourcesDirectory, CFStringRef subdirectory)
{
    
    CFMutableDictionaryRef queryTable = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFMutableArrayRef allFiles = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    CFMutableDictionaryRef typeDir = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFStringRef productName = _CFGetProductName();//CFSTR("iphone");
    CFStringRef platformName = _CFGetPlatformName();//CFSTR("iphoneos");
    if (CFEqual(productName, CFSTR("ipod"))) {
        productName = CFSTR("iphone");
    }
    CFStringRef product = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("~%@"), productName);
    CFStringRef platform = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("-%@"), platformName);
    
    CFMutableStringRef path = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, inPath);
    
    if (resourcesDirectory) {
        _CFAppendPathComponent2(path, resourcesDirectory);
    }
    
    // Record the length of the base path, so we can strip off the stuff we'll be appending later
    CFIndex basePathLen = CFStringGetLength(path);
    
    if (subdirectory) {
        _CFAppendPathComponent2(path, subdirectory);
    }
    // read the content in sub dir and put them into query table
    _CFBundleReadDirectory(path, subdirectory, allFiles, false, queryTable, typeDir, NULL, false, product, platform, NULL, false);
    CFStringDelete(path, CFRangeMake(basePathLen, CFStringGetLength(path) - basePathLen));    // Strip the string back to the base path
    
    CFIndex numOfAllFiles = CFArrayGetCount(allFiles);
    
    CFIndex numLprojs = languages ? CFArrayGetCount(languages) : 0;
    CFMutableDictionaryRef addedTypes = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    Boolean hasFileAdded = false;
    Boolean firstLproj = true;
    
    // First, search lproj for user's chosen language
    if (numLprojs >= 1) {
        CFStringRef lprojTarget = (CFStringRef)CFArrayGetValueAtIndex(languages, 0);
        _CFAppendPathComponent2(path, lprojTarget);
        _CFAppendPathExtension2(path, _CFBundleLprojExtension);
        if (subdirectory) {
            _CFAppendPathComponent2(path, subdirectory);
        }
        _CFBundleReadDirectory(path, subdirectory, allFiles, hasFileAdded, queryTable, typeDir, addedTypes, firstLproj, product, platform, lprojTarget, true);
        CFStringDelete(path, CFRangeMake(basePathLen, CFStringGetLength(path) - basePathLen));         // Strip the string back to the base path

        if (!hasFileAdded && numOfAllFiles < CFArrayGetCount(allFiles)) {
            hasFileAdded = true;
        }
        firstLproj = false;
    }
    
    // Next, search Base.lproj folder
    _CFAppendPathComponent2(path, _CFBundleBaseDirectory);
    _CFAppendPathExtension2(path, _CFBundleLprojExtension);
    if (subdirectory) {
        _CFAppendPathComponent2(path, subdirectory);
    }
    _CFBundleReadDirectory(path, subdirectory, allFiles, hasFileAdded, queryTable, typeDir, addedTypes, YES, product, platform, _CFBundleBaseDirectory, true);
    CFStringDelete(path, CFRangeMake(basePathLen, CFStringGetLength(path) - basePathLen));    // Strip the string back to the base path
    
    if (!hasFileAdded && numOfAllFiles < CFArrayGetCount(allFiles)) {
        hasFileAdded = true;
    }
    
    // Finally, search remaining languages (development language first)
    if (numLprojs >= 2) {
        // for each lproj we are interested in, read the content and put them into query table
        for (CFIndex i = 1; i < CFArrayGetCount(languages); i++) {
            CFStringRef lprojTarget = (CFStringRef) CFArrayGetValueAtIndex(languages, i);
            _CFAppendPathComponent2(path, lprojTarget);
            _CFAppendPathExtension2(path, _CFBundleLprojExtension);
            if (subdirectory) {
                _CFAppendPathComponent2(path, subdirectory);
            }
            _CFBundleReadDirectory(path, subdirectory, allFiles, hasFileAdded, queryTable, typeDir, addedTypes, false, product, platform, lprojTarget, true);
            CFStringDelete(path, CFRangeMake(basePathLen, CFStringGetLength(path) - basePathLen));         // Strip the string back to the base path
            
            if (!hasFileAdded && numOfAllFiles < CFArrayGetCount(allFiles)) {
                hasFileAdded = true;
            }
        }
    }
    
    CFRelease(addedTypes);
    CFRelease(path);
    
    // put the array of all files in sub dir to the query table
    if (CFArrayGetCount(allFiles) > 0) {
        CFDictionarySetValue(queryTable, _CFBundleAllFiles, allFiles);
    }
    
    CFRelease(platform);
    CFRelease(product);
    CFRelease(allFiles);
    CFRelease(typeDir);
    
    return queryTable;
}   

// caller need to release the table
static CFDictionaryRef _CFBundleCopyQueryTable(CFBundleRef bundle, CFURLRef bundleURL, CFArrayRef languages, CFStringRef resourcesDirectory, CFStringRef subdirectory)
{
    CFDictionaryRef subTable = NULL;
    
    if (bundle && !languages) {
        languages = _CFBundleCopyLanguageSearchListInBundle(bundle);
    } else if (languages) {
        CFRetain(languages);
    }
    
    if (bundle) {
        CFMutableStringRef argDirStr = NULL;
        if (subdirectory) {
            argDirStr = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, resourcesDirectory);
            _CFAppendPathComponent2(argDirStr, subdirectory);
        } else {
            argDirStr = (CFMutableStringRef)CFRetain(resourcesDirectory);
        }
        
        __CFLock(&bundle->_queryLock);
        
        // check if the query table for the given sub dir has been created
        subTable = (CFDictionaryRef) CFDictionaryGetValue(bundle->_queryTable, argDirStr);
        
        if (!subTable) {
            // create the query table for the given sub dir
            subTable = _CFBundleCreateQueryTableAtPath(bundle->_bundleBasePath, languages, resourcesDirectory, subdirectory);
            
            CFDictionarySetValue(bundle->_queryTable, argDirStr, subTable);
        } else {
            CFRetain(subTable);
        }
        __CFUnlock(&bundle->_queryLock);
        CFRelease(argDirStr);
    } else {
        CFURLRef url = CFURLCopyAbsoluteURL(bundleURL);
        CFStringRef bundlePath = CFURLCopyFileSystemPath(url, PLATFORM_PATH_STYLE);
        CFRelease(url);
        subTable = _CFBundleCreateQueryTableAtPath(bundlePath, languages, resourcesDirectory, subdirectory);
        CFRelease(bundlePath);
    }
    
    if (languages) CFRelease(languages);
    return subTable;
}

static CFURLRef _CFBundleCreateRelativeURLFromBaseAndPath(CFStringRef path, CFURLRef base, UniChar slash, CFStringRef slashStr)
{
    CFURLRef url = NULL;
    CFRange resultRange;
    Boolean needToRelease = false;
    if (CFStringFindWithOptions(path, slashStr, CFRangeMake(0, CFStringGetLength(path)-1), kCFCompareBackwards, &resultRange)) {
        CFStringRef subPathCom = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, path, CFRangeMake(0, resultRange.location));
        base = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, base, subPathCom, YES);
        path = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, path, CFRangeMake(resultRange.location+1, CFStringGetLength(path)-resultRange.location-1));
        CFRelease(subPathCom);
        needToRelease = true;
    }
    if (CFStringGetCharacterAtIndex(path, CFStringGetLength(path)-1) == slash) {
        url = (CFURLRef)CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, path, PLATFORM_PATH_STYLE, true, base);
    } else {
        url = (CFURLRef)CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, path, PLATFORM_PATH_STYLE, false, base);
    }
    if (needToRelease) {
        CFRelease(base);
        CFRelease(path);
    }
    return url;
}

static void _CFBundleFindResourcesWithPredicate(CFMutableArrayRef interResult, CFDictionaryRef queryTable, Boolean (^predicate)(CFStringRef filename, Boolean *stop), Boolean *stop)
{
    CFIndex dictSize = CFDictionaryGetCount(queryTable);
    if (dictSize == 0) {
        return;
    }
    CFTypeRef *keys = (CFTypeRef *)malloc(sizeof(CFTypeRef) * dictSize);
    CFTypeRef *values = (CFTypeRef *)malloc(sizeof(CFTypeRef) * dictSize);
    if (!keys || !values) return;
    
    CFDictionaryGetKeysAndValues(queryTable, keys, values);
    for (CFIndex i = 0; i < dictSize; i++) {
        if (predicate((CFStringRef)keys[i], stop)) {
            if (CFGetTypeID(values[i]) == CFStringGetTypeID()) {
                CFArrayAppendValue(interResult, values[i]);
            } else {
                CFArrayAppendArray(interResult, (CFArrayRef)values[i], CFRangeMake(0, CFArrayGetCount((CFArrayRef)values[i])));
            }
        }
        
        if (*stop) break;
    }
    
    free(keys);
    free(values);
}

static CFTypeRef _CFBundleCopyURLsOfKey(CFBundleRef bundle, CFURLRef bundleURL, CFArrayRef bundleURLLanguages, CFStringRef resourcesDirectory, CFStringRef subDir, CFStringRef key, CFStringRef lproj, Boolean returnArray, Boolean localized, uint8_t bundleVersion, Boolean (^predicate)(CFStringRef filename, Boolean *stop))
{
    CFTypeRef value = NULL;
    Boolean stop = false; // for predicate
    CFMutableArrayRef interResult = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    CFDictionaryRef subTable = NULL;
    
    CFMutableStringRef path = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, resourcesDirectory);
    if (1 == bundleVersion) {
        CFIndex savedPathLength = CFStringGetLength(path);
        // add the non-localized resource dir
        _CFAppendPathComponent2(path, _CFBundleNonLocalizedResourcesDirectoryName);
        subTable = _CFBundleCopyQueryTable(bundle, bundleURL, bundleURLLanguages, path, subDir);
        if (predicate) {
            _CFBundleFindResourcesWithPredicate(interResult, subTable, predicate, &stop);
        } else {
            value = CFDictionaryGetValue(subTable, key);
        }
        CFStringDelete(path, CFRangeMake(savedPathLength, CFStringGetLength(path) - savedPathLength));    // Strip the string back to the base path
    }
    
    if (!value && !stop) {
        if (subTable) CFRelease(subTable);
        subTable = _CFBundleCopyQueryTable(bundle, bundleURL, bundleURLLanguages, path, subDir);
        if (predicate) {
            _CFBundleFindResourcesWithPredicate(interResult, subTable, predicate, &stop);
        } else {
            // get the path or paths for the given key
            value = CFDictionaryGetValue(subTable, key);
        }
    }
    
    // if localization is needed, we filter out the paths for the localization and put the valid ones in the interResult
    Boolean checkLP = true;
    CFIndex lpLen = lproj ? CFStringGetLength(lproj) : 0;
    if (localized && value) {
        
        if (CFGetTypeID(value) == CFStringGetTypeID()){
            // We had one result, but since we are going to do a search in a different localization, we will convert the one result into an array of results.
            value = CFArrayCreate(kCFAllocatorSystemDefault, (const void **)&value, 1, &kCFTypeArrayCallBacks);
        } else {
            CFRetain(value);
        }
        
        CFRange resultRange, searchRange;
        CFIndex pathValueLen;
        CFIndex limit = returnArray ? CFArrayGetCount((CFArrayRef)value) : 1;
        searchRange.location = 0;
        for (CFIndex i = 0; i < limit; i++) {
            CFStringRef pathValue = (CFStringRef) CFArrayGetValueAtIndex((CFArrayRef)value, i);
            pathValueLen = CFStringGetLength(pathValue);
            searchRange.length = pathValueLen;
            
            // if we have subdir, we find the subdir and see if it is after the base path (bundle path + res dir)
            Boolean searchForLocalization = false;
            if (subDir && CFStringGetLength(subDir) > 0) {
                if (CFStringFindWithOptions(pathValue, subDir, searchRange, kCFCompareEqualTo, &resultRange) && resultRange.location != searchRange.location) {
                    searchForLocalization = true;
                }
            } else if (!(subDir && CFStringGetLength(subDir) > 0) && searchRange.length != 0) {
                if (CFStringFindWithOptions(pathValue, _CFBundleLprojExtensionWithDot, searchRange, kCFCompareEqualTo, &resultRange) && resultRange.location + 7 < pathValueLen) {
                    searchForLocalization = true;
                }
            }
            
            if (searchForLocalization) {
                if (!lpLen || !(CFStringFindWithOptions(pathValue, lproj, searchRange, kCFCompareEqualTo | kCFCompareAnchored, &resultRange) && CFStringFindWithOptions(pathValue, CFSTR("."), CFRangeMake(resultRange.location + resultRange.length, 1), kCFCompareEqualTo, &resultRange))) {
                    break;
                }
                checkLP = false;
            }
            
            CFArrayAppendValue(interResult, pathValue);
        }
        
        CFRelease(value);
        
        if (!returnArray && CFArrayGetCount(interResult) != 0) {
            checkLP = false;
        }
    } else if (value) {
        if (CFGetTypeID(value) == CFArrayGetTypeID()) {
            CFArrayAppendArray(interResult, (CFArrayRef)value, CFRangeMake(0, CFArrayGetCount((CFArrayRef)value)));
        } else {
            CFArrayAppendValue(interResult, value);
        }
    }
    
    value = NULL;
    CFRelease(subTable);
    
    // we fetch the result for a given lproj and join them with the nonlocalized result fetched above
    if (lpLen && checkLP) {
        CFMutableStringRef lprojSubdirName = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, lproj);
        _CFAppendPathExtension2(lprojSubdirName, _CFBundleLprojExtension);
        if (subDir && CFStringGetLength(subDir) > 0) {
            _CFAppendPathComponent2(lprojSubdirName, subDir);
        }
        subTable = _CFBundleCopyQueryTable(bundle, bundleURL, bundleURLLanguages, path, lprojSubdirName);
        CFRelease(lprojSubdirName);
        value = CFDictionaryGetValue(subTable, key);
        
        if (value) {
            if (CFGetTypeID(value) == CFStringGetTypeID()) {
                CFArrayAppendValue(interResult, value);
            } else {
                CFArrayAppendArray(interResult, (CFArrayRef)value, CFRangeMake(0, CFArrayGetCount((CFArrayRef)value)));
            }
        }
        
        CFRelease(subTable);
    }
    
    // after getting paths, we create urls from the paths
    CFTypeRef result = NULL;
    if (CFArrayGetCount(interResult) > 0) {
        UniChar slash = _CFGetSlash();
        CFMutableStringRef urlStr = NULL;
        if (bundle) {
            urlStr = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, bundle->_bundleBasePath);
        } else {
            CFURLRef url = CFURLCopyAbsoluteURL(bundleURL);
            CFStringRef bundlePath = CFURLCopyFileSystemPath(url, PLATFORM_PATH_STYLE);
            urlStr = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, bundlePath);
            CFRelease(url);
            CFRelease(bundlePath);
        }
        
        if (resourcesDirectory && CFStringGetLength(resourcesDirectory)) {
            _CFAppendPathComponent2(urlStr, resourcesDirectory);
        }

        _CFAppendTrailingPathSlash2(urlStr);
        
        if (!returnArray) {
            Boolean isOnlyTypeOrAllFiles = CFStringHasPrefix(key, _CFBundleTypeIndicator);
            isOnlyTypeOrAllFiles |= CFStringHasPrefix(key, _CFBundleAllFiles);
            
            CFStringRef resultPath = (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)interResult, 0);
            if (!isOnlyTypeOrAllFiles) {
                // path is a part of an actual path in the query table, so it should not have a length greater than the buffer size
                CFStringAppend(urlStr, resultPath);
                if (CFStringGetCharacterAtIndex(resultPath, CFStringGetLength(resultPath)-1) == slash) {
                    result = (CFURLRef)CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, urlStr, PLATFORM_PATH_STYLE, true);
                } else {
                    result = (CFURLRef)CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, urlStr, PLATFORM_PATH_STYLE, false);
                }
            } else {
                // need to create relative URLs for binary compatibility issues
                CFURLRef base = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, urlStr, PLATFORM_PATH_STYLE, true);
                result = (CFURLRef)_CFBundleCreateRelativeURLFromBaseAndPath(resultPath, base, slash, _CFGetSlashStr());
                CFRelease(base);
            }
        } else {
            // need to create relative URLs for binary compatibility issues
            CFIndex numOfPaths = CFArrayGetCount((CFArrayRef)interResult);
            CFURLRef base = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, urlStr, PLATFORM_PATH_STYLE, true);
            CFMutableArrayRef urls = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
            for (CFIndex i = 0; i < numOfPaths; i++) {
                CFStringRef path = (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)interResult, i);
                CFURLRef url = _CFBundleCreateRelativeURLFromBaseAndPath(path, base, slash, _CFGetSlashStr());
                CFArrayAppendValue(urls, url);
                CFRelease(url);
            }
            result = urls;
            CFRelease(base);
        }
        CFRelease(urlStr);
    } else if (returnArray) {
        result = CFRetain(interResult);
    }
    if (path) CFRelease(path);
    CFRelease(interResult);
    return result;
}

#pragma mark -


// This is the main entry point for all resource lookup.
// Research shows that by far the most common scenario is to pass in a bundle object, a resource name, and a resource type, using the default localization.
// It is probably the case that more than a few resources will be looked up, making the cost of a readdir less than repeated stats. But it is a relative waste of memory to create strings for every file name in the bundle, especially since those are not what are returned to the caller (URLs are). So, an idea: cache the existence of the most common file names (Info.plist, en.lproj, etc) instead of creating entries for them. If other resources are requested, then go ahead and do the readdir and cache the rest of the file names.
// Another idea: if you want caching, you should create a bundle object. Otherwise we'll happily readdir each time.
CF_EXPORT CFTypeRef _CFBundleCopyFindResources(CFBundleRef bundle, CFURLRef bundleURL, CFArrayRef _unused_pass_null_, CFStringRef resourceName, CFStringRef resourceType, CFStringRef subPath, CFStringRef lproj, Boolean returnArray, Boolean localized, Boolean (^predicate)(CFStringRef filename, Boolean *stop))
{
    
    // Don't use any path info passed into the resource name
    CFStringRef realResourceName = NULL;
    CFStringRef subPathFromResourceName = NULL;

    if (resourceName) {
        CFIndex slashLocation = -1;
        realResourceName = _CFCreateLastPathComponent(kCFAllocatorSystemDefault, resourceName, &slashLocation);
        if (slashLocation > 0) {
            // do not include the /
            subPathFromResourceName = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, resourceName, CFRangeMake(0, slashLocation));
        }
        
        if (slashLocation > 0 && CFStringGetLength(realResourceName) == 0 && slashLocation == CFStringGetLength(resourceName) - 1) {
            // Did we have a name with just a single / at the end? Taking the lastPathComponent will end up with an empty resource name, which is probably not what was expected.
            // Reset the name to be just the directory name.
            CFRelease(realResourceName);
            realResourceName = CFStringCreateCopy(kCFAllocatorSystemDefault, subPathFromResourceName);
        }
        
        // Normalize the resource name by converting it to file system representation. Otherwise when we look for the key in our tables, it will not match.
        // TODO: remove this in some way to avoid the malloc?
        char buff[CFMaxPathSize];
        if (CFStringGetFileSystemRepresentation(realResourceName, buff, CFMaxPathSize)) {
            CFRelease(realResourceName);
            realResourceName = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, buff);
        }
    }
        
    CFMutableStringRef key = NULL;
    const static UniChar extensionSep = '.';
    
    if (realResourceName && CFStringGetLength(realResourceName) > 0 && resourceType && CFStringGetLength(resourceType) > 0) {
        // Testing shows that using a mutable string here is significantly faster than using the format functions.
        key = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, realResourceName);
        // Don't re-append a . if the resource name already has one
        if (CFStringGetCharacterAtIndex(resourceType, 0) != '.') CFStringAppendCharacters(key, &extensionSep, 1);
        CFStringAppend(key, resourceType);
    } else if (realResourceName && CFStringGetLength(realResourceName) > 0) {
        key = (CFMutableStringRef)CFRetain(realResourceName);
    } else if (resourceType && CFStringGetLength(resourceType) > 0) {
        key = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, _CFBundleTypeIndicator);
        // Don't re-append a . if the resource name already has one
        if (CFStringGetCharacterAtIndex(resourceType, 0) != '.') CFStringAppendCharacters(key, &extensionSep, 1);
        CFStringAppend(key, resourceType);
    } else {
        key = (CFMutableStringRef)CFRetain(_CFBundleAllFiles);
    }
    
    CFStringRef realSubdirectory = NULL;
    
    bool hasSubPath = subPath && CFStringGetLength(subPath);
    bool hasSubPathFromResourceName = subPathFromResourceName && CFStringGetLength(subPathFromResourceName);
    
    if (hasSubPath && !hasSubPathFromResourceName) {
        realSubdirectory = (CFStringRef)CFRetain(subPath);
    } else if (!hasSubPath && hasSubPathFromResourceName) {
        realSubdirectory = (CFStringRef)CFRetain(subPathFromResourceName);
    } else if (hasSubPath && hasSubPathFromResourceName) {
        // Multiple sub paths - we'll have to concatenate
        realSubdirectory = CFStringCreateMutableCopy(kCFAllocatorSystemDefault, 0, subPath);
        _CFAppendPathComponent2((CFMutableStringRef)realSubdirectory, subPathFromResourceName);
    }
    
    uint8_t bundleVersion = bundle ? _CFBundleLayoutVersion(bundle) : 0;
    CFArrayRef bundleURLLanguages = NULL;
    if (bundleURL) {
        bundleURLLanguages = _CFBundleCopyLanguageSearchListInDirectory(bundleURL, &bundleVersion);
    }
    
    CFStringRef resDir = _CFBundleGetResourceDirForVersion(bundleVersion);
    
    CFTypeRef returnValue = _CFBundleCopyURLsOfKey(bundle, bundleURL, bundleURLLanguages, resDir, realSubdirectory, key, lproj, returnArray, localized, bundleVersion, predicate);
    
    if ((!returnValue || (CFGetTypeID(returnValue) == CFArrayGetTypeID() && CFArrayGetCount((CFArrayRef)returnValue) == 0)) && (0 == bundleVersion || 2 == bundleVersion)) {
        CFStringRef bundlePath = NULL;
        if (bundle) {
            bundlePath = bundle->_bundleBasePath;
            CFRetain(bundlePath);
        } else {
            CFURLRef absoluteURL = CFURLCopyAbsoluteURL(bundleURL);
            bundlePath = CFURLCopyFileSystemPath(absoluteURL, PLATFORM_PATH_STYLE);
            CFRelease(absoluteURL);
        }
        if ((0 == bundleVersion) || CFEqual(CFSTR("/Library/Spotlight"), bundlePath)){
            if (returnValue) CFRelease(returnValue);
            if ((bundleVersion == 0 && realSubdirectory && CFEqual(realSubdirectory, CFSTR("Resources"))) || (bundleVersion == 2 && realSubdirectory && CFEqual(realSubdirectory, CFSTR("Contents/Resources")))) {
                if (realSubdirectory) CFRelease(realSubdirectory);
                realSubdirectory = CFSTR("");
            } else if (bundleVersion == 0 && realSubdirectory && CFStringGetLength(realSubdirectory) > 10 && CFStringHasPrefix(realSubdirectory, CFSTR("Resources/"))) {
                CFStringRef tmpRealSubdirectory = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, realSubdirectory, CFRangeMake(10, CFStringGetLength(realSubdirectory) - 10));
                if (realSubdirectory) CFRelease(realSubdirectory);
                realSubdirectory = tmpRealSubdirectory;
            } else if (bundleVersion == 2 && realSubdirectory && CFStringGetLength(realSubdirectory) > 19 && CFStringHasPrefix(realSubdirectory, CFSTR("Contents/Resources/"))) {
                CFStringRef tmpRealSubdirectory = CFStringCreateWithSubstring(kCFAllocatorSystemDefault, realSubdirectory, CFRangeMake(19, CFStringGetLength(realSubdirectory) - 19));
                if (realSubdirectory) CFRelease(realSubdirectory);
                realSubdirectory = tmpRealSubdirectory;
            } else {
                // Assume no resources directory
                resDir = CFSTR("");
            }
            returnValue = _CFBundleCopyURLsOfKey(bundle, bundleURL, bundleURLLanguages, resDir, realSubdirectory, key, lproj, returnArray, localized, bundleVersion, predicate);
        }
        CFRelease(bundlePath);
    }
    
    if (realResourceName) CFRelease(realResourceName);
    if (realSubdirectory) CFRelease(realSubdirectory);
    if (subPathFromResourceName) CFRelease(subPathFromResourceName);
    if (bundleURLLanguages) CFRelease(bundleURLLanguages);
    CFRelease(key);
    
    
    return returnValue;
}
