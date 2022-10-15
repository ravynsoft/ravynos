/*      CFBundle_Tables.c
	Copyright (c) 1999-2017, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2017, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
        Responsibility: Tony Parker
*/

#if 0

#include <CoreFoundation/CFBundle.h>
#include "CFBundle_Internal.h"

static CFMutableDictionaryRef _bundlesByIdentifier = NULL;
static CFMutableDictionaryRef _bundlesByURL = NULL;
static CFMutableArrayRef _allBundles = NULL;

#if TARGET_OS_OSX
// Some apps may rely on the fact that CFBundle used to allow bundle objects to be deallocated (despite handing out unretained pointers via CFBundleGetBundleWithIdentifier or CFBundleGetAllBundles). To remain compatible even in the face of unsafe behavior, we can optionally use unsafe-unretained memory management for holding on to bundles.
static Boolean _useUnsafeUnretainedTables(void) {
    return false;
}
#endif

static void _CFBundleAddToTables(CFBundleRef bundle, Boolean alreadyLocked) {
    if (bundle->_isUnique) return;
    
    CFStringRef bundleID = CFBundleGetIdentifier(bundle);
    
    if (!alreadyLocked) _CFMutexLock(&CFBundleGlobalDataLock);
    
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
    if (!alreadyLocked) _CFMutexUnlock(&CFBundleGlobalDataLock);
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

CF_PRIVATE CFBundleRef _CFBundleCopyBundleForURL(CFURLRef url, Boolean alreadyLocked) {
    CFBundleRef result = NULL;
    CFBundleRef main = CFBundleGetMainBundle();
    if (main->_url && url && CFEqual(main->_url, url)) {
        return main;
    }
    if (!alreadyLocked) _CFMutexLock(&CFBundleGlobalDataLock);
    if (_bundlesByURL) result = (CFBundleRef)CFDictionaryGetValue(_bundlesByURL, url);
    if (result && !result->_url) {
        result = NULL;
        CFDictionaryRemoveValue(_bundlesByURL, url);
    }
    if (result) CFRetain(result);
    if (!alreadyLocked) _CFMutexUnlock(&CFBundleGlobalDataLock);
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

#pragma mark - Exported Functions

CF_EXPORT CFBundleRef CFBundleGetBundleWithIdentifier(CFStringRef bundleID) {
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
        _CFMutexLock(&CFBundleGlobalDataLock);
        result = _CFBundlePrimitiveGetBundleWithIdentifierAlreadyLocked(bundleID);
#if TARGET_OS_MAC
        if (!result) {
            // Try to create the bundle for the caller and try again
            void *p = __builtin_return_address(0);
            if (p) {
                CFStringRef imagePath = _CFBundleCopyLoadedImagePathForPointer(p);
                // If the pointer is in Foundation, we were called by NSBundle and we should look one more frame up the stack for a hint
                if (imagePath && CFStringHasSuffix(imagePath, CFSTR("/Foundation"))) {
                    CFRelease(imagePath);
                    // Reset to NULL in case p is null below, that will make us fall back through the right path
                    imagePath = NULL;
                    p = __builtin_return_address(1);
                    if (p) {
                        imagePath = _CFBundleCopyLoadedImagePathForPointer(p);
                    }
                }
                
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
        _CFMutexUnlock(&CFBundleGlobalDataLock);
    }
    
    if (!result) {
        _CFMutexLock(&CFBundleGlobalDataLock);
        // Make sure all bundles have been created and try again.
        _CFBundleEnsureAllBundlesUpToDateAlreadyLocked();
        result = _CFBundlePrimitiveGetBundleWithIdentifierAlreadyLocked(bundleID);
        _CFMutexUnlock(&CFBundleGlobalDataLock);
    }
    
    return result;
}

CF_EXPORT CFBundleRef _CFBundleGetExistingBundleWithBundleURL(CFURLRef bundleURL) {
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
#endif
