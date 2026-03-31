/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#include <CoreFoundation/CFBundlePriv.h>

#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>
#include <IOKit/kext/fat_util.h>
#include <IOKit/kext/macho_util.h>

#include "kextfind_commands.h"
#include "kextfind_main.h"
#include "kextfind_query.h"

/*******************************************************************************
*
*******************************************************************************/
CFStringRef copyPathForKext(
    OSKextRef theKext,
    PathSpec  pathSpec)
{
    CFStringRef result          = CFSTR("(can't determine kext path)");

    CFURLRef    kextURL         = OSKextGetURL(theKext);  // do not release
    CFURLRef    absURL          = NULL;  // must release
    OSKextRef   containerKext   = NULL;  // must release
    CFURLRef    containerURL    = NULL;  // do not release
    CFURLRef    containerAbsURL = NULL;  // must release
    CFURLRef    repositoryURL   = NULL;  // must release
    CFStringRef repositoryPath  = NULL;  // must release
    CFStringRef kextPath        = NULL;  // must release


    if (!kextURL) {
        OSKextLog(theKext,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Kext has no URL!");
        goto finish;
    }

    if (pathSpec == kPathsNone) {
        result = CFURLCopyLastPathComponent(kextURL);
    } else if (pathSpec == kPathsFull) {
        absURL = CFURLCopyAbsoluteURL(kextURL);
        if (!absURL) {
            OSKextLogMemError();
            goto finish;
        }
        result = CFURLCopyFileSystemPath(absURL, kCFURLPOSIXPathStyle);
    } else if (pathSpec == kPathsRelative) {
        CFRange relativeRange;
        absURL = CFURLCopyAbsoluteURL(kextURL);
        if (!absURL) {
            OSKextLogMemError();
            goto finish;
        }

        containerKext = OSKextCopyContainerForPluginKext(theKext);
        if (containerKext) {
            containerURL = OSKextGetURL(containerKext);
            if (!containerURL) {
                OSKextLog(containerKext,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Container kext has no URL!");
                goto finish;
            }
            containerAbsURL = CFURLCopyAbsoluteURL(containerURL);
            if (!containerAbsURL) {
                OSKextLogMemError();
                goto finish;
            }
            repositoryURL = CFURLCreateCopyDeletingLastPathComponent(
                kCFAllocatorDefault, containerAbsURL);
            if (!repositoryURL) {
                OSKextLogMemError();
                goto finish;
            }
        } else {
            repositoryURL = CFURLCreateCopyDeletingLastPathComponent(
                kCFAllocatorDefault, absURL);
            if (!repositoryURL) {
                OSKextLogMemError();
                goto finish;
            }
        }

        repositoryPath = CFURLCopyFileSystemPath(repositoryURL, kCFURLPOSIXPathStyle);
        kextPath = CFURLCopyFileSystemPath(absURL, kCFURLPOSIXPathStyle);
        if (!repositoryPath || !kextPath) {
            OSKextLogMemError();
            goto finish;
        }

       /* We add 1 to the length of the repositoryPath to handle the
        * intermediate '/' character.
        */
        relativeRange = CFRangeMake(1+CFStringGetLength(repositoryPath),
            CFStringGetLength(kextPath) - (1+CFStringGetLength(repositoryPath)));
        result = CFStringCreateWithSubstring(kCFAllocatorDefault,
            kextPath, relativeRange);
    } else {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Internal error.");
        goto finish;
    }

finish:
    SAFE_RELEASE(absURL);
    SAFE_RELEASE(containerKext);
    SAFE_RELEASE(containerAbsURL);
    SAFE_RELEASE(repositoryURL);
    SAFE_RELEASE(repositoryPath);
    SAFE_RELEASE(kextPath);

    return result;
}

/*******************************************************************************
*
*******************************************************************************/
void printKext(
    OSKextRef theKext,
    PathSpec pathSpec,
    Boolean extra_info,
    char lineEnd)
{
    CFStringRef   bundleID      = NULL;  // do NOT release
    CFStringRef   bundleVersion = NULL;  // do NOT release

    CFStringRef   kextPath      = NULL;  // must release
    char        * kext_path     = NULL;  // must free
    char        * bundle_id     = NULL;  // must free
    char        * bundle_version = NULL;  // must free

    kextPath = copyPathForKext(theKext, pathSpec);
    if (!kextPath) {
        OSKextLogMemError();
        goto finish;
    }

    kext_path = createUTF8CStringForCFString(kextPath);
    if (!kext_path) {
        OSKextLogMemError();
        goto finish;
    }

    if (extra_info) {
        bundleID = OSKextGetIdentifier(theKext);
        bundleVersion = OSKextGetValueForInfoDictionaryKey(theKext,
            kCFBundleVersionKey);

        bundle_id = createUTF8CStringForCFString(bundleID);
        bundle_version = createUTF8CStringForCFString(bundleVersion);
        if (!bundle_id || !bundle_version) {
            OSKextLogMemError();
            goto finish;
        }

        fprintf(stdout, "%s\t%s\t%s%c", kext_path, bundle_id,
            bundle_version, lineEnd);
    } else {
        fprintf(stdout, "%s%c", kext_path, lineEnd);
    }

finish:
    SAFE_RELEASE(kextPath);
    SAFE_FREE(kext_path);
    SAFE_FREE(bundle_id);
    SAFE_FREE(bundle_version);

    return;
}

/*******************************************************************************
*
*******************************************************************************/
#define kMaxPrintableCFDataLength    (36)
#define kByteGroupSize                (4)

char * stringForData(const UInt8 * data, int numBytes)
{
    char * result = NULL;
    char * scan = NULL;
    int numByteGroups;
    int i;
    char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

   /* Put a space between every 2 bytes (4 chars).
    */
    numByteGroups = numBytes / kByteGroupSize;

    result = malloc(((numBytes * 2) + numByteGroups + 1) * sizeof(char));
    if (!result) {
        goto finish;
    }

    scan = result;

    for (i = 0; i < numBytes; i++) {
        int binaryDigit1 = (data[i] & 0xF0) >> 4;
        int binaryDigit2 = data[i] & 0xF;
        if (i > 0 && i % kByteGroupSize == 0) {
            *scan++ = ' ';
        }
        *scan++ = digits[binaryDigit1];
        *scan++ = digits[binaryDigit2];
    }
    *scan++ = '\0';

finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
void printProperty(
    CFStringRef label,
    CFStringRef propKey,
    CFTypeRef value,
    char lineEnd)
{
    CFTypeID type = CFGetTypeID(value);
    CFStringRef propString = NULL;     // must release
    CFStringRef labeledString = NULL;  // must release
    CFStringRef outputString = NULL;   // do not release
    char * allocString = NULL;         // must free
    char * dataString = NULL;  // must free

    if (type == CFStringGetTypeID()) {
        propString = CFStringCreateWithFormat(
            kCFAllocatorDefault, NULL, CFSTR("%@ = \"%@\"%c"),
            propKey, value, lineEnd);
    } else if (type == CFNumberGetTypeID()) {
        propString = CFStringCreateWithFormat(
            kCFAllocatorDefault, NULL, CFSTR("%@ = %@%c"),
            propKey, value, lineEnd);
    } else if (type == CFBooleanGetTypeID()) {
        propString = CFStringCreateWithFormat(
            kCFAllocatorDefault, NULL, CFSTR("%@ = %@%c"),
            propKey, value, lineEnd);
    } else if (type == CFDataGetTypeID()) {
        CFIndex length = 0;
        length = CFDataGetLength(value);
        const UInt8 * data = CFDataGetBytePtr(value);
        if (!data) {
            propString = CFStringCreateWithFormat(
                kCFAllocatorDefault, NULL, CFSTR("%@[%zd] = <null data pointer>%c"),
                propKey, length, lineEnd);
        } else {
            int numBytes = (int)MIN(length, kMaxPrintableCFDataLength);
            dataString = stringForData(data, MIN(numBytes, kMaxPrintableCFDataLength));
            if (length > kMaxPrintableCFDataLength) {
                propString = CFStringCreateWithFormat(
                    kCFAllocatorDefault, NULL,
                    CFSTR("%@ = <data (%zd bytes): %s...>%c"),
                    propKey, length, dataString, lineEnd);
            } else {
                propString = CFStringCreateWithFormat(
                    kCFAllocatorDefault, NULL,
                    CFSTR("%@ = <data (%zd bytes): %s>%c"),
                    propKey, length, dataString, lineEnd);
            }
        }
    } else if (type == CFDictionaryGetTypeID()) {
        propString = CFStringCreateWithFormat(
            kCFAllocatorDefault, NULL, CFSTR("%@ = <dictionary of %zd items>%c"),
            propKey, CFDictionaryGetCount(value), lineEnd);
    } else if (type == CFArrayGetTypeID()) {
        propString = CFStringCreateWithFormat(
            kCFAllocatorDefault, NULL, CFSTR("%@ = <array of %zd items>%c"),
            propKey, CFArrayGetCount(value), lineEnd);
    } else {
        propString = CFStringCreateWithFormat(
            kCFAllocatorDefault, NULL, CFSTR("%@ = <value of unknown type>%c"),
            propKey, lineEnd);
    }

    if (!propString) {
        goto finish;
    }

    if (label) {
        labeledString = CFStringCreateWithFormat(
            kCFAllocatorDefault, NULL, CFSTR("%@: %@"), label, propString);
        outputString = labeledString;
    } else {
        labeledString = CFStringCreateWithFormat(
            kCFAllocatorDefault, NULL, CFSTR("%@"), propString);
        outputString = labeledString;
    }

    if (!outputString) {
        goto finish;
    }

    allocString = createUTF8CStringForCFString(outputString);
    if (!allocString) {
        goto finish;
    }

    fprintf(stdout, "%s", allocString);

finish:
    if (propString)     CFRelease(propString);
    if (labeledString)  CFRelease(labeledString);
    if (allocString)    free(allocString);
    if (dataString)     free(dataString);
    return;
}

/*******************************************************************************
*
*******************************************************************************/
void printKextProperty(
    OSKextRef theKext,
    CFStringRef propKey,
    char lineEnd)
{
    CFTypeRef       value = NULL;

    value = OSKextGetValueForInfoDictionaryKey(theKext, propKey);
    if (value) {
        printProperty(NULL, propKey, value, lineEnd);
    }

    return;
}

/*******************************************************************************
*
*******************************************************************************/
void printKextMatchProperty(
    OSKextRef theKext,
    CFStringRef propKey,
    char lineEnd)
{
    CFDictionaryRef personalitiesDict = NULL;
    CFStringRef * names = NULL;
    CFDictionaryRef * personalities = NULL;
    CFIndex numPersonalities;
    CFIndex i;

    personalitiesDict = OSKextGetValueForInfoDictionaryKey(theKext,
        CFSTR(kIOKitPersonalitiesKey));
    if (!personalitiesDict) {
        goto finish;
    }

    numPersonalities = CFDictionaryGetCount(personalitiesDict);
    if (!numPersonalities) {
        goto finish;
    }

    names = malloc(numPersonalities * sizeof(CFStringRef));
    personalities = malloc(numPersonalities * sizeof(CFDictionaryRef));
    if (!names || !personalities) {
        goto finish;
    }

    CFDictionaryGetKeysAndValues(personalitiesDict, (const void **)names,
        (const void **)personalities);

    for (i = 0; i < numPersonalities; i++) {
        CFTypeRef value = CFDictionaryGetValue(personalities[i], propKey);
        if (value) {
            printProperty(names[i], propKey, value, lineEnd);
        }
    }

finish:
    if (names)             free(names);
    if (personalities)     free(personalities);

    return;
}

/*******************************************************************************
*
*******************************************************************************/
void printKextArches(
    OSKextRef theKext,
    char lineEnd,
    Boolean printLineEnd)
{
    fat_iterator fiter = NULL;
    struct mach_header * farch = NULL;
    const NXArchInfo * archinfo = NULL;
    Boolean printedOne = false;

    fiter = createFatIteratorForKext(theKext);
    if (!fiter) {
        goto finish;
    }

    while ((farch = fat_iterator_next_arch(fiter, NULL))) {
        int swap = ISSWAPPEDMACHO(farch->magic);
        archinfo = NXGetArchInfoFromCpuType(CondSwapInt32(swap, farch->cputype),
            CondSwapInt32(swap, farch->cpusubtype));
        if (archinfo) {
            fprintf(stdout, "%s%s", printedOne ? "," : "", archinfo->name);
            printedOne = true;
        }
    }

finish:
    if (printLineEnd && printedOne) {
        fprintf(stdout, "%c", lineEnd);
    }
    if (fiter)  fat_iterator_close(fiter);

    return;
}

/*******************************************************************************
*
*******************************************************************************/
void printKextDependencies(
    OSKextRef theKext,
    PathSpec pathSpec,
    Boolean extra_info,
    char lineEnd)
{
    CFArrayRef kextDependencies = OSKextCopyAllDependencies(theKext,
        /* needAll? */ false);
    CFIndex count, i;

    if (!kextDependencies) {
        goto finish;
    }

    count = CFArrayGetCount(kextDependencies);
    for (i = 0; i < count; i++) {
        OSKextRef thisKext = (OSKextRef)CFArrayGetValueAtIndex(kextDependencies, i);
        printKext(thisKext, pathSpec, extra_info, lineEnd);
    }

finish:
    if (kextDependencies) CFRelease(kextDependencies);
    return;
}

/*******************************************************************************
*
*******************************************************************************/
void printKextDependents(
    OSKextRef theKext,
    PathSpec pathSpec,
    Boolean extra_info,
    char lineEnd)
{
    CFArrayRef kextDependents = OSKextCopyDependents(theKext,
        /* direct? */ false);
    CFIndex count, i;

    if (!kextDependents) {
        goto finish;
    }

    count = CFArrayGetCount(kextDependents);
    for (i = 0; i < count; i++) {
        OSKextRef thisKext = (OSKextRef)CFArrayGetValueAtIndex(kextDependents, i);
        printKext(thisKext, pathSpec, extra_info, lineEnd);
    }

finish:
    if (kextDependents) CFRelease(kextDependents);
    return;
}

/*******************************************************************************
*
*******************************************************************************/
void printKextPlugins(
    OSKextRef theKext,
    PathSpec pathSpec,
    Boolean extra_info,
    char lineEnd)
{
    CFArrayRef plugins = OSKextCopyPlugins(theKext);  // must release
    CFIndex count, i;

    if (!plugins) {
        goto finish;
    }

    count = CFArrayGetCount(plugins);
    for (i = 0; i < count; i++) {
        OSKextRef thisKext = (OSKextRef)CFArrayGetValueAtIndex(plugins, i);
        printKext(thisKext, pathSpec, extra_info, lineEnd);
    }

finish:
    SAFE_RELEASE(plugins);
    return;
}

/*******************************************************************************
* copyAdjustedPathForURL()
*
* This function takes an URL with a given kext, and adjusts it to be absolute
* or relative to the kext's containing repository, properly handling plugin
* kexts to include the repository-path of the containing kext as well.
*******************************************************************************/
CFStringRef copyAdjustedPathForURL(
    OSKextRef theKext,
    CFURLRef  urlToAdjust,
    PathSpec  pathSpec)
{
    CFStringRef result       = NULL;
    CFURLRef    absURL       = NULL;  // must release
    CFStringRef absPath      = NULL;  // must release
    CFStringRef kextAbsPath  = NULL;  // must release
    CFStringRef kextRelPath  = NULL;  // must release
    CFStringRef pathInKext   = NULL;  // must release
    CFRange     scratchRange;

    if (pathSpec != kPathsFull && pathSpec != kPathsRelative) {
        OSKextLog(theKext,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Invalid argument to copyAdjustedPathForURL().");
    }

    absURL = CFURLCopyAbsoluteURL(urlToAdjust);
    if (!absURL) {
        OSKextLogMemError();
        goto finish;
    }

    if (pathSpec == kPathsFull) {
        result = CFURLCopyFileSystemPath(absURL, kCFURLPOSIXPathStyle);
        goto finish;
    }

   /*****
    * Okay, we are doing repository-relative paths here. Here's how!
    * We are strip the matching part of the kext's absolute path
    * from the URL/path handed in, which gives us the path in the kext.
    * Then we tack that back onto the kext's repository-relative path. Got it?
    */

    kextAbsPath = copyPathForKext(theKext, kPathsFull);
    kextRelPath = copyPathForKext(theKext, kPathsRelative);
    absPath = CFURLCopyFileSystemPath(absURL, kCFURLPOSIXPathStyle);
    if (!kextAbsPath || !kextRelPath || !absPath) {
        goto finish;
    }

    scratchRange = CFRangeMake(CFStringGetLength(kextAbsPath),
        CFStringGetLength(absPath) - CFStringGetLength(kextAbsPath));
    pathInKext = CFStringCreateWithSubstring(kCFAllocatorDefault, absPath,
        scratchRange);
    if (!pathInKext) {
        OSKextLogMemError();
    }
    result = CFStringCreateWithFormat(kCFAllocatorDefault, /* options */ 0,
        CFSTR("%@%@"), kextRelPath, pathInKext);

finish:
    SAFE_RELEASE(absURL);
    SAFE_RELEASE(absPath);
    SAFE_RELEASE(kextAbsPath);
    SAFE_RELEASE(kextRelPath);
    SAFE_RELEASE(pathInKext);
    return result;
}

/*******************************************************************************
* XXX: I'm really not sure this is completely reliable for getting a relative
* XXX: path.
*******************************************************************************/
CFStringRef copyKextInfoDictionaryPath(
    OSKextRef theKext,
    PathSpec  pathSpec)
{
    CFStringRef   result      = NULL;
    CFURLRef      kextURL     = NULL;  // do not release
    CFURLRef      kextAbsURL  = NULL;  // must release
    CFBundleRef   kextBundle  = NULL;  // must release
    CFURLRef      infoDictURL = NULL;  // must release

    kextURL = OSKextGetURL(theKext);
    if (!kextURL) {
        OSKextLog(theKext,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Kext has no URL!");
        goto finish;
    }
    kextAbsURL = CFURLCopyAbsoluteURL(kextURL);
    if (!kextAbsURL) {
        OSKextLogMemError();
        goto finish;
    }

    kextBundle = CFBundleCreate(kCFAllocatorDefault, kextAbsURL);
    if (!kextBundle) {
        OSKextLogMemError();
        goto finish;
    }
    infoDictURL = _CFBundleCopyInfoPlistURL(kextBundle);
    if (!infoDictURL) {
        // not able to determine error here, bundle might have no plist
        // (well, we should never have gotten here if that were the case)
        result = CFStringCreateWithCString(kCFAllocatorDefault, "",
            kCFStringEncodingUTF8);
        goto finish;
    }

    result = copyAdjustedPathForURL(theKext, infoDictURL, pathSpec);

finish:
    SAFE_RELEASE(infoDictURL);
    SAFE_RELEASE(kextBundle);
    SAFE_RELEASE(kextAbsURL);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
void printKextInfoDictionary(
    OSKextRef theKext,
    PathSpec pathSpec,
    char lineEnd)
{
    CFStringRef   infoDictPath = NULL;  // must release
    char        * infoDictPathCString = NULL;  // must free

    infoDictPath = copyKextInfoDictionaryPath(theKext, pathSpec);
    if (!infoDictPath) {
        OSKextLogMemError();
        goto finish;
    }

    infoDictPathCString = createUTF8CStringForCFString(infoDictPath);
    if (!infoDictPathCString) {
        OSKextLogMemError();
        goto finish;
    }

    printf("%s%c", infoDictPathCString, lineEnd);


finish:
    SAFE_FREE(infoDictPathCString);
    SAFE_RELEASE(infoDictPath);
    return;
}

/*******************************************************************************
* XXX: I'm really not sure this is completely reliable for getting a relative
* XXX: path.
*******************************************************************************/
CFStringRef copyKextExecutablePath(
    OSKextRef theKext,
    PathSpec  pathSpec)
{
    CFStringRef   result = NULL;
    CFURLRef      kextURL       = NULL;  // do not release
    CFURLRef      kextAbsURL    = NULL;  // must release
    CFURLRef      executableURL = NULL;    // must release

    kextURL = OSKextGetURL(theKext);
    if (!kextURL) {
        OSKextLog(theKext,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Kext has no URL!");
        goto finish;
    }
    kextAbsURL = CFURLCopyAbsoluteURL(kextURL);
    if (!kextAbsURL) {
        OSKextLogMemError();
        goto finish;
    }

    executableURL = _CFBundleCopyExecutableURLInDirectory(kextAbsURL);
    if (!executableURL) {
        // not able to determine error here, bundle might have no executable
        result = CFStringCreateWithCString(kCFAllocatorDefault, "",
            kCFStringEncodingUTF8);
        goto finish;
    }
    result = copyAdjustedPathForURL(theKext, executableURL, pathSpec);

finish:
    SAFE_RELEASE(executableURL);
    SAFE_RELEASE(kextAbsURL);
    return result;
}

/*******************************************************************************
* XXX: I'm really not sure this is completely reliable for getting a relative
* XXX: path.
*******************************************************************************/
void printKextExecutable(
    OSKextRef theKext,
    PathSpec pathSpec,
    char lineEnd)
{
    CFStringRef   executablePath = NULL;  // must release
    char        * executablePathCString = NULL;  // must free

    executablePath = copyKextExecutablePath(theKext, pathSpec);
    if (!executablePath) {
        OSKextLogMemError();
        goto finish;
    }

    executablePathCString = createUTF8CStringForCFString(executablePath);
    if (!executablePathCString) {
        OSKextLogMemError();
        goto finish;
    }

    printf("%s%c", executablePathCString, lineEnd);


finish:
    SAFE_FREE(executablePathCString);
    SAFE_RELEASE(executablePath);
    return;
}
