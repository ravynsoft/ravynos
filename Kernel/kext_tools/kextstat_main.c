/*
 * Copyright (c) 2006, 2014 Apple Computer, Inc. All rights reserved.
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
#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/param.h>

#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mach_host.h>

#include "kextstat_main.h"

// not a utility.[ch] customer yet
static const char * progname = "(unknown)";

void printPList_new(FILE * stream, CFTypeRef aPlist);
/*******************************************************************************
*
*******************************************************************************/
ExitStatus main(int argc, char * const * argv)
{
    ExitStatus          result        = EX_OK;
    KextstatArgs        toolArgs;
    CFDictionaryRef   * kextInfoList  = NULL;  // must free
    CFIndex             count, i;

    if (argv[0]) {
        progname = argv[0];
    }

   /* Set the OSKext log callback right away.
    */
    OSKextSetLogOutputFunction(&tool_log);

    result = readArgs(argc, argv, &toolArgs);
    if (result != EX_OK) {
        if (result == kKextstatExitHelp) {
            result = EX_OK;
        }
        goto finish;
    }

    toolArgs.runningKernelArch = OSKextGetRunningKernelArchitecture();
    if (!toolArgs.runningKernelArch) {
        result = EX_OSERR;
        goto finish;
    }

    toolArgs.loadedKextInfo = OSKextCopyLoadedKextInfo(toolArgs.bundleIDs,
        NULL /* all info */);

    if (!toolArgs.loadedKextInfo) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag | kOSKextLogIPCFlag,
            "Couldn't get list of loaded kexts from kernel.");
        result = EX_OSERR;
        goto finish;
    }

    if (!toolArgs.flagListOnly) {
        printf("Index Refs Address    ");
        if (toolArgs.runningKernelArch->cputype & CPU_ARCH_ABI64) {
            printf("        ");
        }
        printf("Size       Wired      ");
        if (toolArgs.flagShowArchitecture) {
            printf("Architecture       ");
        }
        printf("Name (Version) UUID <Linked Against>\n");
    }

    count = CFDictionaryGetCount(toolArgs.loadedKextInfo);
    if (!count) {
        goto finish;
    }

    kextInfoList = (CFDictionaryRef *)malloc(count * sizeof(CFDictionaryRef));
    if (!kextInfoList) {
        OSKextLogMemError();
        result = EX_OSERR;
        goto finish;
    }

    CFDictionaryGetKeysAndValues(toolArgs.loadedKextInfo, /* keys */ NULL,
        (const void **)kextInfoList);
    if (toolArgs.flagSortByLoadAddress) {
        qsort(kextInfoList, count, sizeof(CFDictionaryRef), &compareKextInfoLoadAddress);
    }
    else {
        qsort(kextInfoList, count, sizeof(CFDictionaryRef), &compareKextInfo);
    }
    for (i = 0; i < count; i++) {
        printKextInfo(kextInfoList[i], &toolArgs);
    }

finish:
    exit(result);

    SAFE_FREE(kextInfoList);

    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus readArgs(int argc, char * const * argv, KextstatArgs * toolArgs)
{
    ExitStatus   result        = EX_USAGE;
    CFStringRef  scratchString = NULL;  // must release
    int          optChar       = 0;

    bzero(toolArgs, sizeof(*toolArgs));

   /*****
    * Allocate collection objects needed for command line argument processing.
    */
    if (!createCFMutableArray(&toolArgs->bundleIDs, &kCFTypeArrayCallBacks)) {
        goto finish;
    }

   /*****
    * Process command-line arguments.
    */
    result = EX_USAGE;

    while ((optChar = getopt_long_only(argc, argv, kOptChars,
        sOptInfo, NULL)) != -1) {

        SAFE_RELEASE_NULL(scratchString);

        switch (optChar) {

            case kOptHelp:
                usage(kUsageLevelFull);
                result = kKextstatExitHelp;
                goto finish;
                break;

            case kOptNoKernelComponents:
                toolArgs->flagNoKernelComponents = true;
                break;

            case kOptListOnly:
                toolArgs->flagListOnly = true;
                break;

            case kOptBundleIdentifier:
                scratchString = CFStringCreateWithCString(kCFAllocatorDefault,
                    optarg, kCFStringEncodingUTF8);
                if (!scratchString) {
                    OSKextLogMemError();
                    result = EX_OSERR;
                    goto finish;
                }
                CFArrayAppendValue(toolArgs->bundleIDs, scratchString);
                break;

            case kOptArchitecture:
                toolArgs->flagShowArchitecture = true;
                break;

            case kOptSort:
                toolArgs->flagSortByLoadAddress = true;
                break;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Extra arguments starting at %s....", argv[0]);
        usage(kUsageLevelBrief);
        goto finish;
    }

    result = EX_OK;

finish:
    SAFE_RELEASE_NULL(scratchString);

    if (result == EX_USAGE) {
        usage(kUsageLevelBrief);
    }
    return result;
}

/*******************************************************************************
*******************************************************************************/
#define kStringInvalidShort    "??"
#define kStringInvalidLong   "????"

void printKextInfo(CFDictionaryRef kextInfo, KextstatArgs * toolArgs)
{
    CFBooleanRef      isKernelComponent      = NULL;  // do not release
    CFNumberRef       loadTag                = NULL;  // do not release
    CFNumberRef       retainCount            = NULL;  // do not release
    CFNumberRef       loadAddress            = NULL;  // do not release
    CFNumberRef       loadSize               = NULL;  // do not release
    CFNumberRef       wiredSize              = NULL;  // do not release
    CFStringRef       bundleID               = NULL;  // do not release
    CFStringRef       bundleVersion          = NULL;  // do not release
    CFArrayRef        dependencyLoadTags     = NULL;  // do not release
    CFMutableArrayRef sortedLoadTags         = NULL;  // must release
    CFDataRef         kextUUID               = NULL;  // do not release
    uuid_string_t     kextUUIDCString        = "";

    uint32_t          loadTagValue           = kOSKextInvalidLoadTag;
    uint32_t          retainCountValue       = (uint32_t)-1;
    uint64_t          loadAddressValue       = (uint64_t)-1;
    uint32_t          loadSizeValue          = (uint32_t)-1;
    uint32_t          wiredSizeValue         = (uint32_t)-1;
    uint32_t          cpuTypeValue           = (uint32_t)-1;
    uint32_t          cpuSubTypeValue        = (uint32_t)-1;
    char            * bundleIDCString        = NULL;  // must free
    char            * bundleVersionCString   = NULL;  // must free

    CFIndex           count, i;

    loadTag = (CFNumberRef)CFDictionaryGetValue(kextInfo,
        CFSTR(kOSBundleLoadTagKey));
    retainCount = (CFNumberRef)CFDictionaryGetValue(kextInfo,
        CFSTR(kOSBundleRetainCountKey));
    loadAddress = (CFNumberRef)CFDictionaryGetValue(kextInfo,
        CFSTR(kOSBundleLoadAddressKey));
    loadSize = (CFNumberRef)CFDictionaryGetValue(kextInfo,
        CFSTR(kOSBundleLoadSizeKey));
    wiredSize = (CFNumberRef)CFDictionaryGetValue(kextInfo,
        CFSTR(kOSBundleWiredSizeKey));
    bundleID = (CFStringRef)CFDictionaryGetValue(kextInfo,
        kCFBundleIdentifierKey);
    bundleVersion = (CFStringRef)CFDictionaryGetValue(kextInfo,
        kCFBundleVersionKey);
    dependencyLoadTags = (CFArrayRef)CFDictionaryGetValue(kextInfo,
                                                          CFSTR(kOSBundleDependenciesKey));
    kextUUID = (CFDataRef)CFDictionaryGetValue(kextInfo,
                                               CFSTR(kOSBundleUUIDKey));

   /* If the -k flag was given, skip any kernel components unless
    * they are explicitly requested.
    */
    if (toolArgs->flagNoKernelComponents) {
        isKernelComponent = (CFBooleanRef)CFDictionaryGetValue(kextInfo,
            CFSTR(kOSKernelResourceKey));
        if (isKernelComponent && CFBooleanGetValue(isKernelComponent)) {
            if (bundleID &&
                kCFNotFound == CFArrayGetFirstIndexOfValue(toolArgs->bundleIDs,
                    RANGE_ALL(toolArgs->bundleIDs), bundleID)) {

                goto finish;
            }
        }
    }

    if (!getNumValue(loadTag, kCFNumberSInt32Type, &loadTagValue)) {
        loadTagValue = kOSKextInvalidLoadTag;
    }

   /* Never print the info for the kernel (loadTag 0, id __kernel__).
    */
    if (loadTagValue == 0) {
        goto finish;
    }

    if (!getNumValue(retainCount, kCFNumberSInt32Type, &retainCountValue)) {
        retainCountValue = (uint32_t)-1;
    }
    if (!getNumValue(loadAddress, kCFNumberSInt64Type, &loadAddressValue)) {
        loadAddressValue = (uint64_t)-1;
    }
    if (!getNumValue(loadSize, kCFNumberSInt32Type, &loadSizeValue)) {
        loadSizeValue = (uint32_t)-1;
    }
    if (!getNumValue(wiredSize, kCFNumberSInt32Type, &wiredSizeValue)) {
        wiredSizeValue = (uint32_t)-1;
    }
    if (!getNumValue(((CFNumberRef)CFDictionaryGetValue(kextInfo, CFSTR(kOSBundleCPUTypeKey))), kCFNumberSInt32Type, &cpuTypeValue)) {
        cpuTypeValue = (uint32_t)-1;
    }
    if (!getNumValue(((CFNumberRef)CFDictionaryGetValue(kextInfo, CFSTR(kOSBundleCPUSubtypeKey))), kCFNumberSInt32Type, &cpuSubTypeValue)) {
        cpuSubTypeValue = (uint32_t)-1;
    }

    bundleIDCString = createUTF8CStringForCFString(bundleID);
    bundleVersionCString = createUTF8CStringForCFString(bundleVersion);

   /* First column has no leading space.
    *
    * These field widths are from the old kextstat, may want to change them.
    */
    if (loadTagValue == kOSKextInvalidLoadTag) {
        fprintf(stdout, "%5s", kStringInvalidShort);
    } else {
        fprintf(stdout, "%5d", loadTagValue);
    }

    if (retainCountValue == (uint32_t)-1) {
        fprintf(stdout, " %4s", kStringInvalidShort);
    } else {
        fprintf(stdout, " %4d", retainCountValue);
    }

    if (toolArgs->runningKernelArch->cputype & CPU_ARCH_ABI64) {
        if (loadAddressValue == (uint64_t)-1) {
            fprintf(stdout, " %-18s", kStringInvalidLong);
        } else {
            fprintf(stdout, " %#-18llx", (uint64_t)loadAddressValue);
        }
    } else {
        if (loadAddressValue == (uint64_t)-1) {
            fprintf(stdout, " %-10s", kStringInvalidLong);
        } else {
            fprintf(stdout, " %#-10x", (uint32_t)loadAddressValue);
        }
    }

    if (loadSizeValue == (uint32_t)-1) {
        fprintf(stdout, " %-10s", kStringInvalidLong);
    } else {
        fprintf(stdout, " %#-10x", loadSizeValue);
    }

    if (wiredSizeValue == (uint32_t)-1) {
        fprintf(stdout, " %-10s", kStringInvalidLong);
    } else {
        fprintf(stdout, " %#-10x", wiredSizeValue);
    }

    if (toolArgs->flagShowArchitecture) {
        // include kext cputype/cpusubtype info
        if (cpuTypeValue == (uint32_t) -1) {
            fprintf(stdout, " %10s/%-7s", kStringInvalidLong, kStringInvalidLong);
        }
        else {
            const NXArchInfo * archName = NXGetArchInfoFromCpuType(cpuTypeValue, cpuSubTypeValue);

            if (archName != NULL) {
                fprintf(stdout, " %-18s", archName->name);
            }
            else {
                fprintf(stdout, " %#010x/%#-7x", cpuTypeValue, cpuSubTypeValue);
            }
        }
    }

    fprintf(stdout, " %s",
        bundleIDCString ? bundleIDCString : kStringInvalidLong);

    fprintf(stdout, " (%s)",
        bundleVersionCString ? bundleVersionCString : kStringInvalidLong);

    if (kextUUID && (CFDataGetTypeID() == CFGetTypeID(kextUUID))) {
        uuid_unparse(CFDataGetBytePtr(kextUUID), kextUUIDCString);
        fprintf(stdout, " %s", kextUUIDCString);
    }
    else {
        fprintf(stdout, " no UUID");
    }

    if (dependencyLoadTags && CFArrayGetCount(dependencyLoadTags)) {
        sortedLoadTags = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0,
            dependencyLoadTags);
        if (!sortedLoadTags) {
            OSKextLogMemError();
            goto finish;
        }

        CFArraySortValues(sortedLoadTags, RANGE_ALL(sortedLoadTags),
            &compareNumbers, /* context */ NULL);

        fprintf(stdout, " <");
        count = CFArrayGetCount(sortedLoadTags);
        for (i = 0; i < count; i++) {
            loadTag = (CFNumberRef)CFArrayGetValueAtIndex(sortedLoadTags, i);
            if (!getNumValue(loadTag, kCFNumberSInt32Type, &loadTagValue)) {
                loadTagValue = kOSKextInvalidLoadTag;
            }

            if (loadTagValue == kOSKextInvalidLoadTag) {
                fprintf(stdout, "%s%s", i == 0 ? "" : " ", kStringInvalidShort);
            } else {
                fprintf(stdout, "%s%d", i == 0 ? "" : " ", loadTagValue);
            }

        }

        fprintf(stdout, ">");
    }

    fprintf(stdout, "\n");

finish:

    SAFE_RELEASE(sortedLoadTags);
    SAFE_FREE(bundleIDCString);
    SAFE_FREE(bundleVersionCString);
    return;
}

/*******************************************************************************
*******************************************************************************/
Boolean getNumValue(CFNumberRef aNumber, CFNumberType type, void * valueOut)
{
    if (aNumber && (CFNumberGetTypeID() == CFGetTypeID(aNumber))) {
        return CFNumberGetValue(aNumber, type, valueOut);
    }
    return false;
}

/*******************************************************************************
 *******************************************************************************/
int compareKextInfo(const void * vKextInfo1, const void * vKextInfo2)
{
    int             result = 0;
    CFDictionaryRef kextInfo1 = *(CFDictionaryRef *)vKextInfo1;
    CFDictionaryRef kextInfo2 = *(CFDictionaryRef *)vKextInfo2;
    CFNumberRef     loadTag1 = CFDictionaryGetValue(kextInfo1, CFSTR(kOSBundleLoadTagKey));
    CFNumberRef     loadTag2 = CFDictionaryGetValue(kextInfo2, CFSTR(kOSBundleLoadTagKey));
    OSKextLoadTag   tag1Value = kOSKextInvalidLoadTag;
    OSKextLoadTag   tag2Value = kOSKextInvalidLoadTag;

    getNumValue(loadTag1, kCFNumberSInt32Type, &tag1Value);
    getNumValue(loadTag2, kCFNumberSInt32Type, &tag2Value);

    if (tag1Value == tag2Value) {
        /* Whether invalid or valid, same is same. */
        result = 0;
    } else if (tag1Value == kOSKextInvalidLoadTag) {
        result = -1;
    } else if (tag2Value == kOSKextInvalidLoadTag) {
        result = 1;
    } else if (tag1Value < tag2Value) {
        result = -1;
    } else if (tag2Value < tag1Value) {
        result = 1;
    }

    return result;
}

/*******************************************************************************
 *******************************************************************************/
int compareKextInfoLoadAddress(const void * vKextInfo1, const void * vKextInfo2)
{
    int             result = 0;
    CFDictionaryRef kextInfo1 = *(CFDictionaryRef *)vKextInfo1;
    CFDictionaryRef kextInfo2 = *(CFDictionaryRef *)vKextInfo2;

    CFNumberRef       loadAddress1            = NULL;  // do not release
    uint64_t          loadAddressValue1       = (uint64_t)-1;
    CFNumberRef       loadAddress2            = NULL;  // do not release
    uint64_t          loadAddressValue2       = (uint64_t)-1;

    loadAddress1 = (CFNumberRef) CFDictionaryGetValue(kextInfo1,
                                                      CFSTR(kOSBundleLoadAddressKey));
    loadAddress2 = (CFNumberRef) CFDictionaryGetValue(kextInfo2,
                                                      CFSTR(kOSBundleLoadAddressKey));

    if (!getNumValue(loadAddress1, kCFNumberSInt64Type, &loadAddressValue1)) {
        loadAddressValue1 = (uint64_t)-1;
    }
    if (!getNumValue(loadAddress2, kCFNumberSInt64Type, &loadAddressValue2)) {
        loadAddressValue2 = (uint64_t)-1;
    }

    if (loadAddressValue1 == loadAddressValue2) {
        /* Whether invalid or valid, same is same. */
        result = 0;
    } else if (loadAddressValue1 == (uint64_t)-1) {
        result = -1;
    } else if (loadAddressValue2 == (uint64_t)-1) {
        result = 1;
    } else if (loadAddressValue1 < loadAddressValue2) {
        result = -1;
    } else if (loadAddressValue2 < loadAddressValue1) {
        result = 1;
    }

    return result;
}

/*******************************************************************************
*******************************************************************************/
CFComparisonResult compareNumbers(
    const void * val1,
    const void * val2,
          void * context)
{
    CFComparisonResult result = CFNumberCompare((CFNumberRef)val1,
         (CFNumberRef)val2, context);
     if (result == kCFCompareLessThan) {
         result = kCFCompareGreaterThan;
     } else if (result == kCFCompareGreaterThan) {
         result = kCFCompareLessThan;
     }
     return result;
}

/*******************************************************************************
*******************************************************************************/
static void usage(UsageLevel usageLevel)
{
    fprintf(stderr, "usage: %s [-a] [-k] [-l] [-b bundle_id] ...\n", progname);

    if (usageLevel == kUsageLevelBrief) {
        fprintf(stderr, "\nUse %s -%s (-%c) for a list of options.\n",
            progname, kOptNameHelp, kOptHelp);
        return;
    }

    fprintf(stderr, "-%s (-%c): show only loadable kexts (omit kernel components).\n",
        kOptNameNoKernelComponents, kOptNoKernelComponents);
    fprintf(stderr, "-%s (-%c): print the list only, omitting the header.\n",
        kOptNameListOnly, kOptListOnly);
    fprintf(stderr, "-%s (-%c) <bundle_id>: print info for kexts named by identifier.\n",
        kOptNameBundleIdentifier, kOptBundleIdentifier);
    fprintf(stderr, "-%s (-%c): Include architecture info in output.\n",
            kOptNameArchitecture, kOptArchitecture);
    fprintf(stderr, "-%s (-%c): Sort by load address.\n",
            kOptNameSort, kOptSort);

    return;
}

