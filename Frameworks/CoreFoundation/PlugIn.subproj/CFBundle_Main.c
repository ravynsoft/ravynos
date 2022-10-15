/*      CFBundle_Main.c
 Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
 Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
 Licensed under Apache License v2.0 with Runtime Library Exception
 See http://swift.org/LICENSE.txt for license information
 See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 Responsibility: Tony Parker
 */

#include <CoreFoundation/CFBundle.h>
#include "CFBundle_Internal.h"



#if defined(BINARY_SUPPORT_DLFCN)
#include <dlfcn.h>
#ifndef RTLD_FIRST
#define RTLD_FIRST 0
#endif
#endif /* BINARY_SUPPORT_DLFCN */

static Boolean _initedMainBundle = false;
static CFBundleRef _mainBundle = NULL;
static char __CFBundleMainID__[1026] = {0};
char *__CFBundleMainID = __CFBundleMainID__;
static _CFMutex _mainBundleLock = _CF_MUTEX_STATIC_INITIALIZER;

#pragma mark -

static void _CFBundleInitializeMainBundleInfoDictionaryAlreadyLocked(CFStringRef executablePath) {
    CFBundleGetInfoDictionary(_mainBundle);
    if (!_mainBundle->_infoDict || CFDictionaryGetCount(_mainBundle->_infoDict) == 0) {
        // if type 3 bundle and no Info.plist, treat as unbundled, since this gives too many false positives
        if (_mainBundle->_version == _CFBundleVersionFlat) _mainBundle->_version = _CFBundleVersionNotABundle;
        if (_mainBundle->_version == _CFBundleVersionOldStyleResources) {
            // if type 0 bundle and no Info.plist and not main executable for bundle, treat as unbundled, since this gives too many false positives
            CFStringRef executableName = _CFBundleCopyExecutableName(_mainBundle, NULL, NULL);
            if (!executableName || !executablePath || !CFStringHasSuffix(executablePath, executableName)) _mainBundle->_version = _CFBundleVersionNotABundle;
            if (executableName) CFRelease(executableName);
        }
#if defined(BINARY_SUPPORT_DYLD)
        // We can fall into this case when the executable is sandboxed enough that it can't read its own executable file. We can still attempt to get the info dictionary from the main executable though. _CFBundleCreateInfoDictFromMainExecutable will correctly handle a case where the section does not exist.
        if (_mainBundle->_binaryType == __CFBundleDYLDExecutableBinary || _mainBundle->_binaryType == __CFBundleUnreadableBinary) {
            if (_mainBundle->_infoDict) CFRelease(_mainBundle->_infoDict);
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
            _mainBundle = _CFBundleCreateMain(kCFAllocatorSystemDefault, bundleURL);
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
#if TARGET_OS_MAC
                    _mainBundle->_handleCookie = RTLD_MAIN_ONLY;
#else
                    _mainBundle->_handleCookie = dlopen(NULL, 0);
#endif
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
                // It is safe to unlock and re-lock here because we don't really do anything under the lock after we are done. It is just re-locked to satisfy the 'already locked' contract.
                _CFMutexUnlock(&_mainBundleLock);
                CFDictionaryRef infoDict = CFBundleGetInfoDictionary(_mainBundle);
                // CFBundleInitPlugIn will return false if the factory ID already exists. For the main bundle, if that happens, we just continue on since we cannot just return NULL.
                _CFBundleInitPlugIn(_mainBundle, infoDict, NULL);
                _CFPlugInHandleDynamicRegistration(_mainBundle);
                _CFMutexLock(&_mainBundleLock);
            }
        }
        if (bundleURL) CFRelease(bundleURL);
        if (str) CFRelease(str);
        if (executableURL) CFRelease(executableURL);
        
    }
    return _mainBundle;
}

#pragma mark -
#pragma mark Exported Functions

CF_EXPORT CFURLRef _CFBundleCopyMainBundleExecutableURL(Boolean *looksLikeBundle) {
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
        if (mainBundle && (_CFBundleVersionFlat == mainBundle->_version || _CFBundleVersionNotABundle == mainBundle->_version)) mainBundle = NULL;
        *looksLikeBundle = (mainBundle ? true : false);
    }
    return executableURL;
}


CF_EXPORT CFBundleRef CFBundleGetMainBundle(void) {
    CFBundleRef mainBundle;
    _CFMutexLock(&_mainBundleLock);
    mainBundle = _CFBundleGetMainBundleAlreadyLocked();
    _CFMutexUnlock(&_mainBundleLock);
    return mainBundle;
}
