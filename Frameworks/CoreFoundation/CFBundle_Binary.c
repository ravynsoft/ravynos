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

/*      CFBundle_Binary.c
        Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
        Responsibility: Tony Parker
*/

#include "CFBundle_Internal.h"

#if defined(BINARY_SUPPORT_DYLD)
// Import the mach-o headers that define the macho magic numbers
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/arch.h>
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <crt_externs.h>
#if defined(USE_DYLD_PRIV)
#include <mach-o/dyld_priv.h>
#endif /* USE_DYLD_PRIV */
#endif /* BINARY_SUPPORT_DYLD */

#if defined(BINARY_SUPPORT_DLFCN)
#include <dlfcn.h>
#endif /* BINARY_SUPPORT_DLFCN */

#if defined(BINARY_SUPPORT_DYLD)
static CFStringRef _CFBundleDYLDCopyLoadedImagePathForPointer(void *p);
#if !defined(BINARY_SUPPORT_DLFCN)
static void *_CFBundleDYLDGetSymbolByNameWithSearch(CFBundleRef bundle, CFStringRef symbolName, Boolean globalSearch);
#endif /* !BINARY_SUPPORT_DLFCN */
#endif /* BINARY_SUPPORT_DYLD */
#if defined(BINARY_SUPPORT_DLFCN)
static void *_CFBundleDlfcnGetSymbolByNameWithSearch(CFBundleRef bundle, CFStringRef symbolName, Boolean globalSearch);
#if !defined(BINARY_SUPPORT_DYLD)
static CFStringRef _CFBundleDlfcnCopyLoadedImagePathForPointer(void *p);
#endif /* !BINARY_SUPPORT_DYLD */
#endif /* BINARY_SUPPORT_DLFCN */


CF_PRIVATE SInt32 _CFBundleCurrentArchitecture(void) {
    SInt32 arch = 0;
#if defined(__ppc__)
    arch = kCFBundleExecutableArchitecturePPC;
#elif defined(__ppc64__)
    arch = kCFBundleExecutableArchitecturePPC64;
#elif defined(__i386__)
    arch = kCFBundleExecutableArchitectureI386;
#elif defined(__x86_64__)
    arch = kCFBundleExecutableArchitectureX86_64;
#elif defined(BINARY_SUPPORT_DYLD)
    const NXArchInfo *archInfo = NXGetLocalArchInfo();
    if (archInfo) arch = archInfo->cputype;
#endif
    return arch;
}

#if defined(BINARY_SUPPORT_DYLD)

CF_PRIVATE CFArrayRef _CFBundleDYLDCopyLoadedImagePathsForHint(CFStringRef hint) {
    uint32_t i, numImages = _dyld_image_count();
    CFMutableArrayRef result = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
    CFRange range = CFRangeMake(0, CFStringGetLength(hint)), altRange = CFRangeMake(0, 0), testRange = CFRangeMake(0, 0);
    const char *processPath = _CFProcessPath();
    const void *mhp = (const void *)_NSGetMachExecuteHeader();
    
    if (range.length > 14) {
        // handle some common variations on framework bundle identifiers
        if (CFStringFindWithOptions(hint, CFSTR(".framework"), range, kCFCompareAnchored|kCFCompareBackwards|kCFCompareCaseInsensitive, &testRange) && testRange.location > 0 && testRange.length > 0) {
            // identifier has .framework appended
            altRange.length = testRange.location;
        } else if (CFStringFindWithOptions(hint, CFSTR("framework"), range, kCFCompareAnchored|kCFCompareBackwards|kCFCompareCaseInsensitive, &testRange) && testRange.location > 0 && testRange.length > 0) {
            // identifier has Framework appended
            altRange.length = testRange.location;
        } else if (CFStringFindWithOptions(hint, CFSTR("fw"), range, kCFCompareAnchored|kCFCompareBackwards|kCFCompareCaseInsensitive, &testRange) && testRange.location > 0 && testRange.length > 0) {
            // identifier has FW appended
            altRange.length = testRange.location;
        }
    }
    for (i = 0; i < numImages; i++) {
        const char *curName = _dyld_get_image_name(i), *lastComponent = NULL;
        if (curName && (!processPath || 0 != strcmp(curName, processPath)) && mhp != (void *)_dyld_get_image_header(i)) lastComponent = strrchr(curName, '/');
        if (lastComponent) {
            CFStringRef str = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, lastComponent + 1);
            if (str) {
                if (CFStringFindWithOptions(hint, str, range, kCFCompareAnchored|kCFCompareBackwards|kCFCompareCaseInsensitive, NULL) || (altRange.length > 0 && CFStringFindWithOptions(hint, str, altRange, kCFCompareAnchored|kCFCompareBackwards|kCFCompareCaseInsensitive, NULL))) {
                    CFStringRef curStr = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, curName);
                    if (curStr) {
                        CFArrayAppendValue(result, curStr);
                        CFRelease(curStr);
                    }
                }
                CFRelease(str);
            }
        }
    }
    return result;
}

static char *_cleanedPathForPath(const char *curName) {
    char *thePath = strdup(curName);
    if (thePath) {
        // We are going to process the buffer replacing all "/./" and "//" with "/"
        CFIndex srcIndex = 0, dstIndex = 0;
        CFIndex len = strlen(thePath);
        for (srcIndex=0; srcIndex<len; srcIndex++) {
            thePath[dstIndex] = thePath[srcIndex];
            dstIndex++;
            while (srcIndex < len-1 && thePath[srcIndex] == '/' && (thePath[srcIndex+1] == '/' || (thePath[srcIndex+1] == '.' && srcIndex < len-2 && thePath[srcIndex+2] == '/'))) srcIndex += (thePath[srcIndex+1] == '/' ? 1 : 2);
        }
        thePath[dstIndex] = 0;
    }
    return thePath;
}

CF_PRIVATE CFArrayRef _CFBundleDYLDCopyLoadedImagePathsIfChanged(void) {
    // This returns an array of the paths of all the dyld images in the process.  These paths may not be absolute, they may point at things that are not bundles, they may be staticly linked bundles or dynamically loaded bundles, they may be NULL.
    uint32_t i, numImages = _dyld_image_count();
    CFMutableArrayRef result = NULL;
    static uint32_t _cachedDYLDImageCount = -1;

    if (numImages != _cachedDYLDImageCount) {
        const char *curName;
        char *cleanedCurName = NULL;
        CFStringRef curStr;
        const char *processPath = _CFProcessPath();
        const void *mhp = (const void *)_NSGetMachExecuteHeader();

        result = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);

        for (i = 0; i < numImages; i++) {
            curName = _dyld_get_image_name(i);
            if (curName && i == 0) cleanedCurName = _cleanedPathForPath(curName);
            if (curName && (!processPath || 0 != strcmp(curName, processPath)) && (!processPath || !cleanedCurName || 0 != strcmp(cleanedCurName, processPath)) && mhp != (void *)_dyld_get_image_header(i)) {
                curStr = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, curName);
                if (curStr) {
                    CFArrayAppendValue(result, curStr);
                    CFRelease(curStr);
                }
            }
            if (cleanedCurName) {
                free(cleanedCurName);
                cleanedCurName = NULL;
            }
        }
        _cachedDYLDImageCount = numImages;
    }
    return result;
}

static CFStringRef _CFBundleDYLDCopyLoadedImagePathForPointer(void *p) {
    CFStringRef result = NULL;
#if defined(USE_DYLD_PRIV)
    const char *name = dyld_image_path_containing_address(p);
    if (name) result = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, name);
#else /* USE_DYLD_PRIV */
    if (!result) {
        uint32_t i, j, n = _dyld_image_count();
        Boolean foundit = false;
        const char *name;
#if __LP64__
#define MACH_HEADER_TYPE struct mach_header_64
#define MACH_SEGMENT_CMD_TYPE struct segment_command_64
#define MACH_SEGMENT_FLAVOR LC_SEGMENT_64
#else
#define MACH_HEADER_TYPE struct mach_header
#define MACH_SEGMENT_CMD_TYPE struct segment_command
#define MACH_SEGMENT_FLAVOR LC_SEGMENT
#endif
        for (i = 0; !foundit && i < n; i++) {
            const MACH_HEADER_TYPE *mh = (const MACH_HEADER_TYPE *)_dyld_get_image_header(i);
            uintptr_t addr = (uintptr_t)p - _dyld_get_image_vmaddr_slide(i);
            if (mh) {
                struct load_command *lc = (struct load_command *)((char *)mh + sizeof(MACH_HEADER_TYPE));
                for (j = 0; !foundit && j < mh->ncmds; j++, lc = (struct load_command *)((char *)lc + lc->cmdsize)) {
                    if (MACH_SEGMENT_FLAVOR == lc->cmd && ((MACH_SEGMENT_CMD_TYPE *)lc)->vmaddr <= addr && addr < ((MACH_SEGMENT_CMD_TYPE *)lc)->vmaddr + ((MACH_SEGMENT_CMD_TYPE *)lc)->vmsize) {
                        foundit = true;
                        name = _dyld_get_image_name(i);
                        if (name) result = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, name);
                    }
                }
            }
        }
#undef MACH_HEADER_TYPE
#undef MACH_SEGMENT_CMD_TYPE
#undef MACH_SEGMENT_FLAVOR
    }
#endif /* USE_DYLD_PRIV */
#if LOG_BUNDLE_LOAD
    printf("dyld image path for pointer %p is %p\n", p, result);
#endif /* LOG_BUNDLE_LOAD */
    return result;
}

#if !defined(BINARY_SUPPORT_DLFCN) 

static const void *__CFBundleDYLDFindImage(char *buff) {
    const void *header = NULL;
    uint32_t i, numImages = _dyld_image_count(), numMatches = 0;
    const char *curName, *p, *q;

    for (i = 0; !header && i < numImages; i++) {
        curName = _dyld_get_image_name(i);
        if (curName && 0 == strncmp(curName, buff, CFMaxPathSize)) {
            header = _dyld_get_image_header(i);
            numMatches = 1;
        }
    }
    if (!header) {
        for (i = 0; i < numImages; i++) {
            curName = _dyld_get_image_name(i);
            if (curName) {
                for (p = buff, q = curName; *p && *q && (q - curName < CFMaxPathSize); p++, q++) {
                    if (*p != *q && (q - curName > 11) && 0 == strncmp(q - 11, ".framework/Versions/", 20) && *(q + 9) && '/' == *(q + 10)) q += 11;
                    else if (*p != *q && (q - curName > 12) && 0 == strncmp(q - 12, ".framework/Versions/", 20) && *(q + 8) && '/' == *(q + 9)) q += 10;
                    if (*p != *q) break;
                }
                if (*p == *q) {
                    header = _dyld_get_image_header(i);
                    numMatches++;
                }
            }
        }
    }
    return (numMatches == 1) ? header : NULL;
}

CF_PRIVATE Boolean _CFBundleDYLDCheckLoaded(CFBundleRef bundle) {
    if (!bundle->_isLoaded) {
        CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
        char buff[CFMaxPathSize];

        if (executableURL && CFURLGetFileSystemRepresentation(executableURL, true, (uint8_t *)buff, CFMaxPathSize)) {
            const void *header = __CFBundleDYLDFindImage(buff);
            if (header) {
                if (bundle->_binaryType == __CFBundleUnknownBinary) bundle->_binaryType = __CFBundleDYLDFrameworkBinary;
                if (!bundle->_imageCookie) {
                    bundle->_imageCookie = header;
#if LOG_BUNDLE_LOAD
                    printf("dyld check load bundle %p, find %s getting image %p\n", bundle, buff, bundle->_imageCookie);
#endif /* LOG_BUNDLE_LOAD */
                }
                bundle->_isLoaded = true;
            } else {
#if LOG_BUNDLE_LOAD
                printf("dyld check load bundle %p, find %s no image\n", bundle, buff);
#endif /* LOG_BUNDLE_LOAD */
            }
        }
        if (executableURL) CFRelease(executableURL);
    }
    return bundle->_isLoaded;
}

CF_PRIVATE Boolean _CFBundleDYLDLoadBundle(CFBundleRef bundle, Boolean forceGlobal, CFErrorRef *error) {
    CFErrorRef localError = NULL, *subError = (error ? &localError : NULL);
    NSLinkEditErrors c = NSLinkEditUndefinedError;
    int errorNumber = 0;
    const char *fileName = NULL;
    const char *errorString = NULL;

    if (!bundle->_isLoaded) {
        CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
        char buff[CFMaxPathSize];

        if (executableURL && CFURLGetFileSystemRepresentation(executableURL, true, (uint8_t *)buff, CFMaxPathSize)) {
            NSObjectFileImage image;
            NSObjectFileImageReturnCode retCode = NSCreateObjectFileImageFromFile(buff, &image);
#if LOG_BUNDLE_LOAD
            printf("dyld load bundle %p, create image of %s returns image %p retcode %d\n", bundle, buff, image, retCode);
#endif /* LOG_BUNDLE_LOAD */
            if (retCode == NSObjectFileImageSuccess) {
                uint32_t options = forceGlobal ? NSLINKMODULE_OPTION_RETURN_ON_ERROR : (NSLINKMODULE_OPTION_BINDNOW | NSLINKMODULE_OPTION_PRIVATE | NSLINKMODULE_OPTION_RETURN_ON_ERROR);
                NSModule module = NSLinkModule(image, buff, options);
#if LOG_BUNDLE_LOAD
                printf("dyld load bundle %p, link module of %s options 0x%x returns module %p image %p\n", bundle, buff, options, module, image);
#endif /* LOG_BUNDLE_LOAD */
                if (module) {
                    bundle->_imageCookie = image;
                    bundle->_moduleCookie = module;
                    bundle->_isLoaded = true;
                } else {
                    NSLinkEditError(&c, &errorNumber, &fileName, &errorString);
                    if (error) {
#if defined(BINARY_SUPPORT_DLFCN)
                        _CFBundleDlfcnPreflight(bundle, subError);
#endif /* BINARY_SUPPORT_DLFCN */
                        if (!localError) {
                            CFStringRef tempString = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, errorString), debugString = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("error code %d, error number %d (%@)"), c, errorNumber, tempString);
                            localError = _CFBundleCreateErrorDebug(CFGetAllocator(bundle), bundle, CFBundleExecutableLinkError, debugString);
                            if (tempString) CFRelease(tempString);
                            if (debugString) CFRelease(debugString);
                        }
                    } else {
                        CFStringRef tempString = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, errorString), executableString = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, fileName);
                        CFLog(__kCFLogBundle, CFSTR("Error loading %@:  error code %d, error number %d (%@)"), executableString, c, errorNumber, tempString);
                        if (tempString) CFRelease(tempString);
                        if (executableString) CFRelease(executableString);
                    }
                    (void)NSDestroyObjectFileImage(image);
                }
            } else {
                if (error) {
                    if (retCode == NSObjectFileImageArch) {
                        localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableArchitectureMismatchError);
                    } else if (retCode == NSObjectFileImageInappropriateFile) {
                        Boolean hasRuntimeMismatch = false;
                        uint32_t mainFlags = 0, bundleFlags = 0;
                        if (_CFBundleGrokObjCImageInfoFromMainExecutable(NULL, &mainFlags) && (mainFlags & 0x2) != 0) {
                            if (_CFBundleGetObjCImageInfo(bundle, NULL, &bundleFlags) && (bundleFlags & 0x2) == 0) hasRuntimeMismatch = true;
                        }
                        if (hasRuntimeMismatch) {
                            localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableRuntimeMismatchError);
                        } else {
                            localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotLoadableError);
                        }
                    } else {
#if defined(BINARY_SUPPORT_DLFCN)
                        _CFBundleDlfcnPreflight(bundle, subError);
#endif /* BINARY_SUPPORT_DLFCN */
                        if (!localError) {
                            CFStringRef debugString = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("dyld returns %d"), retCode);
                            localError = _CFBundleCreateErrorDebug(CFGetAllocator(bundle), bundle, CFBundleExecutableLinkError, debugString);
                            CFRelease(debugString);
                        }
                    }
                } else {
                    CFLog(__kCFLogBundle, CFSTR("dyld returns %d when trying to load %@"), retCode, executableURL);
                }
            }
        } else {
            if (error) {
                localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotFoundError);
            } else {
                CFLog(__kCFLogBundle, CFSTR("Cannot find executable for bundle %@"), bundle);
            }
        }
        if (executableURL) CFRelease(executableURL);
    }
    if (!bundle->_isLoaded && error) *error = localError;
    return bundle->_isLoaded;
}

CF_PRIVATE Boolean _CFBundleDYLDLoadFramework(CFBundleRef bundle, CFErrorRef *error) {
    // !!! Framework loading should be better.  Can't unload frameworks.
    CFErrorRef localError = NULL, *subError = (error ? &localError : NULL);
    NSLinkEditErrors c = NSLinkEditUndefinedError;
    int errorNumber = 0;
    const char *fileName = NULL;
    const char *errorString = NULL;

    if (!bundle->_isLoaded) {
        CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
        char buff[CFMaxPathSize];

        if (executableURL && CFURLGetFileSystemRepresentation(executableURL, true, (uint8_t *)buff, CFMaxPathSize)) {
            void *image = (void *)NSAddImage(buff, NSADDIMAGE_OPTION_RETURN_ON_ERROR);
#if LOG_BUNDLE_LOAD
            printf("dyld load framework %p, add image of %s returns image %p\n", bundle, buff, image);
#endif /* LOG_BUNDLE_LOAD */
            if (image) {
                bundle->_imageCookie = image;
                bundle->_isLoaded = true;
            } else {
                NSLinkEditError(&c, &errorNumber, &fileName, &errorString);
                if (error) {
#if defined(BINARY_SUPPORT_DLFCN)
                    _CFBundleDlfcnPreflight(bundle, subError);
#endif /* BINARY_SUPPORT_DLFCN */
                    if (!localError) {
                        CFStringRef tempString = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, errorString), debugString = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("error code %d, error number %d (%@)"), c, errorNumber, tempString);
                        localError = _CFBundleCreateErrorDebug(CFGetAllocator(bundle), bundle, CFBundleExecutableLinkError, debugString);
                        if (tempString) CFRelease(tempString);
                        if (debugString) CFRelease(debugString);
                    }
                } else {
                    CFStringRef tempString = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, errorString), executableString = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, fileName);
                    CFLog(__kCFLogBundle, CFSTR("Error loading %@:  error code %d, error number %d (%@)"), executableString, c, errorNumber, tempString);
                    if (tempString) CFRelease(tempString);
                    if (executableString) CFRelease(executableString);
                }
            }
        } else {
            if (error) {
                localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotFoundError);
            } else {
                CFLog(__kCFLogBundle, CFSTR("Cannot find executable for bundle %@"), bundle);
            }
        }
        if (executableURL) CFRelease(executableURL);
    }
    if (!bundle->_isLoaded && error) *error = localError;
    return bundle->_isLoaded;
}

CF_PRIVATE void _CFBundleDYLDUnloadBundle(CFBundleRef bundle) {
    if (bundle->_isLoaded) {
#if LOG_BUNDLE_LOAD
        printf("dyld unload bundle %p, handle %p module %p image %p\n", bundle, bundle->_handleCookie, bundle->_moduleCookie, bundle->_imageCookie);
#endif /* LOG_BUNDLE_LOAD */
        if (bundle->_moduleCookie && !NSUnLinkModule((NSModule)(bundle->_moduleCookie), NSUNLINKMODULE_OPTION_NONE)) {
            CFLog(__kCFLogBundle, CFSTR("Internal error unloading bundle %@"), bundle);
        } else {
            if (bundle->_moduleCookie && bundle->_imageCookie) (void)NSDestroyObjectFileImage((NSObjectFileImage)(bundle->_imageCookie));
            bundle->_connectionCookie = bundle->_handleCookie = NULL;
            bundle->_imageCookie = bundle->_moduleCookie = NULL;
            bundle->_isLoaded = false;
        }
    }
}

CF_PRIVATE void *_CFBundleDYLDGetSymbolByName(CFBundleRef bundle, CFStringRef symbolName) {
    return _CFBundleDYLDGetSymbolByNameWithSearch(bundle, symbolName, false);
}

static void *_CFBundleDYLDGetSymbolByNameWithSearch(CFBundleRef bundle, CFStringRef symbolName, Boolean globalSearch) {
    void *result = NULL;
    char buff[1026];
    NSSymbol symbol = NULL;
    
    buff[0] = '_';
    if (CFStringGetCString(symbolName, &(buff[1]), 1024, kCFStringEncodingUTF8)) {
        if (bundle->_moduleCookie) {
            symbol = NSLookupSymbolInModule((NSModule)(bundle->_moduleCookie), buff);
        } else if (bundle->_imageCookie) {
            symbol = NSLookupSymbolInImage(bundle->_imageCookie, buff, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND|NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR);
        } 
        if (!symbol && !bundle->_moduleCookie && (!bundle->_imageCookie || globalSearch)) {
            char hintBuff[1026];
            CFStringRef executableName = _CFBundleCopyExecutableName(bundle, NULL, NULL);
            hintBuff[0] = '\0';
            if (executableName) {
                if (!CFStringGetCString(executableName, hintBuff, 1024, kCFStringEncodingUTF8)) hintBuff[0] = '\0';
                CFRelease(executableName);
            }
            // Nowdays, NSIsSymbolNameDefinedWithHint() and NSLookupAndBindSymbolWithHint()
            // are identical, except the first just returns a bool, so checking with the
            // Is function first just causes a redundant lookup.
            // This returns NULL on failure.
            symbol = NSLookupAndBindSymbolWithHint(buff, hintBuff);
        }
        if (symbol) result = NSAddressOfSymbol(symbol);
#if defined(DEBUG)
        if (!result) CFLog(__kCFLogBundle, CFSTR("dyld cannot find symbol %@ in %@"), symbolName, bundle);
#endif /* DEBUG */
#if LOG_BUNDLE_LOAD
        printf("bundle %p handle %p module %p image %p dyld returns symbol %p for %s\n", bundle, bundle->_handleCookie, bundle->_moduleCookie, bundle->_imageCookie, result, buff + 1);
#endif /* LOG_BUNDLE_LOAD */
    }
    return result;
}

#endif /* !BINARY_SUPPORT_DLFCN */
#endif /* BINARY_SUPPORT_DYLD */

#if defined(BINARY_SUPPORT_DLFCN)

CF_PRIVATE Boolean _CFBundleDlfcnCheckLoaded(CFBundleRef bundle) {
    if (!bundle->_isLoaded) {
        CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
        char buff[CFMaxPathSize];

        if (executableURL && CFURLGetFileSystemRepresentation(executableURL, true, (uint8_t *)buff, CFMaxPathSize)) {
            int mode = RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD | RTLD_FIRST;
            void *handle = dlopen(buff, mode);
            if (handle) {
                if (!bundle->_handleCookie) {
                    bundle->_handleCookie = handle;
#if LOG_BUNDLE_LOAD
                    printf("dlfcn check load bundle %p, dlopen of %s mode 0x%x getting handle %p\n", bundle, buff, mode, bundle->_handleCookie);
#endif /* LOG_BUNDLE_LOAD */
                }
                bundle->_isLoaded = true;
            } else {
#if LOG_BUNDLE_LOAD
                printf("dlfcn check load bundle %p, dlopen of %s mode 0x%x no handle\n", bundle, buff, mode);
#endif /* LOG_BUNDLE_LOAD */
            }
        }
        if (executableURL) CFRelease(executableURL);
    }
    return bundle->_isLoaded;
}

CF_EXPORT Boolean _CFBundleDlfcnPreflight(CFBundleRef bundle, CFErrorRef *error) {
    Boolean retval = true;
    CFErrorRef localError = NULL;
    if (!bundle->_isLoaded) {
        CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
        char buff[CFMaxPathSize];
        
        retval = false;
        if (executableURL && CFURLGetFileSystemRepresentation(executableURL, true, (uint8_t *)buff, CFMaxPathSize)) {
            retval = dlopen_preflight(buff);
            if (!retval && error) {
                CFArrayRef archs = CFBundleCopyExecutableArchitectures(bundle);
                CFStringRef debugString = NULL;
                const char *errorString = dlerror();
                if (errorString && strlen(errorString) > 0) debugString = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, errorString);
                if (archs) {
                    Boolean hasSuitableArch = false, hasRuntimeMismatch = false;
                    CFIndex i, count = CFArrayGetCount(archs);
                    SInt32 arch, curArch = _CFBundleCurrentArchitecture();
                    for (i = 0; !hasSuitableArch && i < count; i++) {
                        if (CFNumberGetValue((CFNumberRef)CFArrayGetValueAtIndex(archs, i), kCFNumberSInt32Type, (void *)&arch) && arch == curArch) hasSuitableArch = true;
                    }
#if defined(BINARY_SUPPORT_DYLD)
                    if (hasSuitableArch) {
                        uint32_t mainFlags = 0;
                        if (_CFBundleGrokObjCImageInfoFromMainExecutable(NULL, &mainFlags) && (mainFlags & 0x2) != 0) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
                            uint32_t bundleFlags = 0;
                            if (_CFBundleGetObjCImageInfo(bundle, NULL, &bundleFlags) && (bundleFlags & 0x2) == 0) hasRuntimeMismatch = true;
#endif
                        }
                    }
#endif /* BINARY_SUPPORT_DYLD */
                    if (hasRuntimeMismatch) {
                        localError = _CFBundleCreateErrorDebug(CFGetAllocator(bundle), bundle, CFBundleExecutableRuntimeMismatchError, debugString);
                    } else if (!hasSuitableArch) {
                        localError = _CFBundleCreateErrorDebug(CFGetAllocator(bundle), bundle, CFBundleExecutableArchitectureMismatchError, debugString);
                    } else {
                        localError = _CFBundleCreateErrorDebug(CFGetAllocator(bundle), bundle, CFBundleExecutableLoadError, debugString);
                    }
                    CFRelease(archs);
                } else {
                    localError = _CFBundleCreateErrorDebug(CFGetAllocator(bundle), bundle, CFBundleExecutableLoadError, debugString);
                }
                if (debugString) CFRelease(debugString);
            }
        } else {
            if (error) localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotFoundError);
        }
        if (executableURL) CFRelease(executableURL);
    }
    if (!retval && error) *error = localError;
    return retval;
}

CF_PRIVATE Boolean _CFBundleDlfcnLoadBundle(CFBundleRef bundle, Boolean forceGlobal, CFErrorRef *error) {
    CFErrorRef localError = NULL, *subError = (error ? &localError : NULL);
    if (!bundle->_isLoaded) {
        CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
        char buff[CFMaxPathSize];
        if (executableURL && CFURLGetFileSystemRepresentation(executableURL, true, (uint8_t *)buff, CFMaxPathSize)) {
            int mode = forceGlobal ? (RTLD_LAZY | RTLD_GLOBAL | RTLD_FIRST) : (RTLD_NOW | RTLD_LOCAL | RTLD_FIRST);
            void *cookie = dlopen(buff, mode);
#if LOG_BUNDLE_LOAD
            printf("dlfcn load bundle %p, dlopen of %s mode 0x%x returns handle %p\n", bundle, buff, mode, cookie);
#endif /* LOG_BUNDLE_LOAD */
            if (cookie && cookie == bundle->_handleCookie) {
                // during the call to dlopen, arbitrary init routines may have run and caused bundle->_handleCookie to be set, in which case proper reference counting requires that reference to be released with dlclose
#if LOG_BUNDLE_LOAD
                printf("dlfcn load bundle %p closing existing reference to handle %p\n", bundle, cookie);
#endif /* LOG_BUNDLE_LOAD */
                dlclose(bundle->_handleCookie);
            }
            bundle->_handleCookie = cookie;
            if (bundle->_handleCookie) {
                bundle->_isLoaded = true;
            } else {
                const char *dyldError = dlerror();
                CFStringRef errorString = dyldError ? CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, dyldError) : NULL;
                if (error) {
                    _CFBundleDlfcnPreflight(bundle, subError);
                    if (!localError) localError = _CFBundleCreateErrorDebug(CFGetAllocator(bundle), bundle, CFBundleExecutableLinkError, errorString);
                } else {
                    CFStringRef executableString = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, buff);
                    CFLog(__kCFLogBundle, CFSTR("Error loading %@:  %@"), executableString, errorString ? errorString : CFSTR("(no additional info)"));
                    if (executableString) CFRelease(executableString);
                }
                if (errorString) CFRelease(errorString);
            }
        } else {
            if (error) {
                localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotFoundError);
            } else {
                CFLog(__kCFLogBundle, CFSTR("Cannot find executable for bundle %@"), bundle);
            }
        }
        if (executableURL) CFRelease(executableURL);
    }
    if (!bundle->_isLoaded && error) *error = localError;
    return bundle->_isLoaded;
}

CF_PRIVATE Boolean _CFBundleDlfcnLoadFramework(CFBundleRef bundle, CFErrorRef *error) {
    CFErrorRef localError = NULL, *subError = (error ? &localError : NULL);
    if (!bundle->_isLoaded) {
        CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
        char buff[CFMaxPathSize];
        if (executableURL && CFURLGetFileSystemRepresentation(executableURL, true, (uint8_t *)buff, CFMaxPathSize)) {
            int mode = RTLD_LAZY | RTLD_GLOBAL | RTLD_FIRST;
            void *cookie = dlopen(buff, mode);
#if LOG_BUNDLE_LOAD
            printf("dlfcn load framework %p, dlopen of %s mode 0x%x returns handle %p\n", bundle, buff, mode, cookie);
#endif /* LOG_BUNDLE_LOAD */
            if (cookie && cookie == bundle->_handleCookie) {
                // during the call to dlopen, arbitrary init routines may have run and caused bundle->_handleCookie to be set, in which case proper reference counting requires that reference to be released with dlclose
#if LOG_BUNDLE_LOAD
                printf("dlfcn load framework %p closing existing reference to handle %p\n", bundle, cookie);
#endif /* LOG_BUNDLE_LOAD */
                dlclose(bundle->_handleCookie);
            }
            bundle->_handleCookie = cookie;
            if (bundle->_handleCookie) {
                bundle->_isLoaded = true;
            } else {
                const char *dyldError = dlerror();
                CFStringRef errorString = dyldError ? CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, dyldError) : NULL;
                if (error) {
                    _CFBundleDlfcnPreflight(bundle, subError);
                    if (!localError) localError = _CFBundleCreateErrorDebug(CFGetAllocator(bundle), bundle, CFBundleExecutableLinkError, errorString);
                } else {
                    CFStringRef executableString = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, buff);
                    CFLog(__kCFLogBundle, CFSTR("Error loading %@:  %@"), executableString, errorString ? errorString : CFSTR("(no additional info)"));
                }
                if (errorString) CFRelease(errorString);
            }
        } else {
            if (error) {
                localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotFoundError);
            } else {
                CFLog(__kCFLogBundle, CFSTR("Cannot find executable for bundle %@"), bundle);
            }
        }
        if (executableURL) CFRelease(executableURL);
    }
    if (!bundle->_isLoaded && error) *error = localError;
    return bundle->_isLoaded;
}

CF_PRIVATE void _CFBundleDlfcnUnload(CFBundleRef bundle) {
    if (bundle->_isLoaded) {
#if LOG_BUNDLE_LOAD
        printf("dlfcn unload bundle %p, handle %p module %p image %p\n", bundle, bundle->_handleCookie, bundle->_moduleCookie, bundle->_imageCookie);
#endif /* LOG_BUNDLE_LOAD */
        if (0 != dlclose(bundle->_handleCookie)) {
            CFLog(__kCFLogBundle, CFSTR("Internal error unloading bundle %@"), bundle);
        } else {
            bundle->_connectionCookie = bundle->_handleCookie = NULL;
            bundle->_imageCookie = bundle->_moduleCookie = NULL;
            bundle->_isLoaded = false;
        }
    }
}

CF_PRIVATE void *_CFBundleDlfcnGetSymbolByName(CFBundleRef bundle, CFStringRef symbolName) {
    return _CFBundleDlfcnGetSymbolByNameWithSearch(bundle, symbolName, false);
}

static void *_CFBundleDlfcnGetSymbolByNameWithSearch(CFBundleRef bundle, CFStringRef symbolName, Boolean globalSearch) {
    void *result = NULL;
    char buff[1026];
    
    if (CFStringGetCString(symbolName, buff, 1024, kCFStringEncodingUTF8)) {
        result = dlsym(bundle->_handleCookie, buff);
        if (!result && globalSearch) result = dlsym(RTLD_DEFAULT, buff);
#if defined(DEBUG)
        if (!result) CFLog(__kCFLogBundle, CFSTR("dlsym cannot find symbol %@ in %@"), symbolName, bundle);
#endif /* DEBUG */
#if LOG_BUNDLE_LOAD
        printf("bundle %p handle %p module %p image %p dlsym returns symbol %p for %s\n", bundle, bundle->_handleCookie, bundle->_moduleCookie, bundle->_imageCookie, result, buff);
#endif /* LOG_BUNDLE_LOAD */
    }
    return result;
}

#if !defined(BINARY_SUPPORT_DYLD)

static CFStringRef _CFBundleDlfcnCopyLoadedImagePathForPointer(void *p) {
    CFStringRef result = NULL;
    Dl_info info;
    if (0 != dladdr(p, &info) && info.dli_fname) result = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, info.dli_fname);
#if LOG_BUNDLE_LOAD
    printf("dlfcn image path for pointer %p is %p\n", p, result);
#endif /* LOG_BUNDLE_LOAD */
    return result;
}

#endif /* !BINARY_SUPPORT_DYLD */
#endif /* BINARY_SUPPORT_DLFCN */

#if defined(BINARY_SUPPORT_DLL)

CF_PRIVATE Boolean _CFBundleDLLLoad(CFBundleRef bundle, CFErrorRef *error) {
    CFErrorRef localError = NULL;
    if (!bundle->_isLoaded) {
        CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
        wchar_t buff[CFMaxPathSize];

        if (executableURL && _CFURLGetWideFileSystemRepresentation(executableURL, true, (wchar_t *)buff, CFMaxPathSize)) {
            bundle->_hModule = LoadLibraryW(buff);
            if (bundle->_hModule) {
                bundle->_isLoaded = true;
            } else {
                if (error) {
                    localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableLinkError);
                } else {
                    CFLog(__kCFLogBundle, CFSTR("Failed to load bundle %@"), bundle);
                }
            }
        } else {
            if (error) {
                localError = _CFBundleCreateError(CFGetAllocator(bundle), bundle, CFBundleExecutableNotFoundError);
            } else {
                CFLog(__kCFLogBundle, CFSTR("Cannot find executable for bundle %@"), bundle);
            }
        }
        if (executableURL) CFRelease(executableURL);
    }
    if (!bundle->_isLoaded && error) *error = localError;
    return bundle->_isLoaded;
}

CF_PRIVATE void _CFBundleDLLUnload(CFBundleRef bundle) {
    if (bundle->_isLoaded) {
       FreeLibrary(bundle->_hModule);
       bundle->_hModule = NULL;
       bundle->_isLoaded = false;
    }
}

CF_PRIVATE void *_CFBundleDLLGetSymbolByName(CFBundleRef bundle, CFStringRef symbolName) {
    void *result = NULL;
    char buff[1024];
    if (CFStringGetCString(symbolName, buff, 1024, kCFStringEncodingWindowsLatin1)) result = GetProcAddress(bundle->_hModule, buff);
    return result;
}

#endif /* BINARY_SUPPORT_DLL */

CF_PRIVATE CFStringRef _CFBundleCopyLoadedImagePathForPointer(void *p) {
    CFStringRef imagePath = NULL;
#if defined(BINARY_SUPPORT_DYLD)
    if (!imagePath) imagePath = _CFBundleDYLDCopyLoadedImagePathForPointer(p);
#elif defined(BINARY_SUPPORT_DLFCN)
    if (!imagePath) imagePath = _CFBundleDlfcnCopyLoadedImagePathForPointer(p);
#endif /* BINARY_SUPPORT_DYLD */
    return imagePath;
}

void *CFBundleGetFunctionPointerForName(CFBundleRef bundle, CFStringRef funcName) {
    void *tvp = NULL;
    // Load if necessary
    if (!bundle->_isLoaded) {
        if (!CFBundleLoadExecutable(bundle)) return NULL;
    }
    
    switch (bundle->_binaryType) {
#if defined(BINARY_SUPPORT_DYLD)
        case __CFBundleDYLDBundleBinary:
        case __CFBundleDYLDFrameworkBinary:
        case __CFBundleDYLDExecutableBinary:
#if defined(BINARY_SUPPORT_DLFCN)
            if (bundle->_handleCookie) return _CFBundleDlfcnGetSymbolByName(bundle, funcName);
#else /* BINARY_SUPPORT_DLFCN */
            return _CFBundleDYLDGetSymbolByName(bundle, funcName);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
#endif /* BINARY_SUPPORT_DYLD */
#if defined(BINARY_SUPPORT_DLL)
        case __CFBundleDLLBinary:
            tvp = _CFBundleDLLGetSymbolByName(bundle, funcName);
            break;
#endif /* BINARY_SUPPORT_DLL */
        default:
#if defined(BINARY_SUPPORT_DLFCN)
            if (bundle->_handleCookie) return _CFBundleDlfcnGetSymbolByName(bundle, funcName);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
    }
    return tvp;
}

void *_CFBundleGetCFMFunctionPointerForName(CFBundleRef bundle, CFStringRef funcName) {
    void *fp = NULL;
    // Load if necessary
    if (!bundle->_isLoaded) {
        if (!CFBundleLoadExecutable(bundle)) return NULL;
    }
#if defined (BINARY_SUPPORT_DYLD) || defined (BINARY_SUPPORT_DLFCN)
    switch (bundle->_binaryType) {
#if defined(BINARY_SUPPORT_DYLD)
        case __CFBundleDYLDBundleBinary:
        case __CFBundleDYLDFrameworkBinary:
        case __CFBundleDYLDExecutableBinary:
#if defined(BINARY_SUPPORT_DLFCN)
            if (bundle->_handleCookie) fp = _CFBundleDlfcnGetSymbolByNameWithSearch(bundle, funcName, true);
#else /* BINARY_SUPPORT_DLFCN */
            fp = _CFBundleDYLDGetSymbolByNameWithSearch(bundle, funcName, true);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
#endif /* BINARY_SUPPORT_DYLD */
        default:
#if defined(BINARY_SUPPORT_DLFCN)
            if (bundle->_handleCookie) fp = _CFBundleDlfcnGetSymbolByNameWithSearch(bundle, funcName, true);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
    }
#endif /* BINARY_SUPPORT_DYLD || BINARY_SUPPORT_DLFCN */
    return fp;
}

void CFBundleGetFunctionPointersForNames(CFBundleRef bundle, CFArrayRef functionNames, void *ftbl[]) {
    SInt32 i, c;
    
    if (!ftbl) return;
    
    c = CFArrayGetCount(functionNames);
    for (i = 0; i < c; i++) ftbl[i] = CFBundleGetFunctionPointerForName(bundle, (CFStringRef)CFArrayGetValueAtIndex(functionNames, i));
}

void _CFBundleGetCFMFunctionPointersForNames(CFBundleRef bundle, CFArrayRef functionNames, void *ftbl[]) {
    SInt32 i, c;
    
    if (!ftbl) return;
    
    c = CFArrayGetCount(functionNames);
    for (i = 0; i < c; i++) ftbl[i] = _CFBundleGetCFMFunctionPointerForName(bundle, (CFStringRef)CFArrayGetValueAtIndex(functionNames, i));
}

void *CFBundleGetDataPointerForName(CFBundleRef bundle, CFStringRef symbolName) {
    void *dp = NULL;
    // Load if necessary
    if (!bundle->_isLoaded && !CFBundleLoadExecutable(bundle)) return NULL;
    
    switch (bundle->_binaryType) {
#if defined(BINARY_SUPPORT_DYLD)
        case __CFBundleDYLDBundleBinary:
        case __CFBundleDYLDFrameworkBinary:
        case __CFBundleDYLDExecutableBinary:
#if defined(BINARY_SUPPORT_DLFCN)
            if (bundle->_handleCookie) dp = _CFBundleDlfcnGetSymbolByName(bundle, symbolName);
#else /* BINARY_SUPPORT_DLFCN */
            dp = _CFBundleDYLDGetSymbolByName(bundle, symbolName);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
#endif /* BINARY_SUPPORT_DYLD */
#if defined(BINARY_SUPPORT_DLL)
        case __CFBundleDLLBinary:
            /* MF:!!! Handle this someday */
            break;
#endif /* BINARY_SUPPORT_DLL */
        default:
#if defined(BINARY_SUPPORT_DLFCN)
            if (bundle->_handleCookie) dp = _CFBundleDlfcnGetSymbolByName(bundle, symbolName);
#endif /* BINARY_SUPPORT_DLFCN */
            break;
    }
    return dp;
}

void CFBundleGetDataPointersForNames(CFBundleRef bundle, CFArrayRef symbolNames, void *stbl[]) {
    SInt32 i, c;
    
    if (!stbl) return;
    
    c = CFArrayGetCount(symbolNames);
    for (i = 0; i < c; i++) stbl[i] = CFBundleGetDataPointerForName(bundle, (CFStringRef)CFArrayGetValueAtIndex(symbolNames, i));
}

