/*
 *  kextstat_main.h
 *  kext_tools
 *
 *  Created by Nik Gervae on 5/03/08.
 *  Copyright 2008, 2014 Apple Inc. All rights reserved.
 *
 */
#ifndef _KEXTSTAT_MAIN_H
#define _KEXTSTAT_MAIN_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/kext/OSKext.h>

#include <getopt.h>
#include <sysexits.h>

#include <mach-o/arch.h>

#include "kext_tools_util.h"

#pragma mark Basic Types & Constants
/*******************************************************************************
* Constants
*******************************************************************************/
enum {
    kKextstatExitOK          = EX_OK,
    kKextstatExitError  = 1,

    // don't think we use it
    kKextstatExitUnspecified = 11,

    // don't actually exit with this, it's just a sentinel value
    kKextstatExitHelp        = 33
};

#pragma mark Command-line Option Definitions
/*******************************************************************************
* Command-line options. This data is used by getopt_long_only().
*
* Options common to all kext tools are in kext_tools_util.h.
*******************************************************************************/

#define kOptNameNoKernelComponents  "no-kernel"
#define kOptNameListOnly            "list-only"
#define kOptNameArchitecture        "arch"
#define kOptNameSort        "sort"

#define kOptNoKernelComponents      'k'
#define kOptListOnly                'l'
#define kOptArchitecture            'a'
#define kOptSort                    's'

#if 0
// bundle-id,version,compatible-version,is-kernel,is-interface,retaincount,path,uuid,started,prelinked,index,address,size,wired,dependencies,classes,cputype,cpusubtype
CFBundleIdentifier
CFBundleVersion
kOSBundleCompatibleVersionKey
kOSKernelResourceKey
kOSBundleIsInterfaceKey
kOSBundlePathKey
kOSBundleUUIDKey
kOSBundleStartedKey
kOSBundlePrelinkedKey
kOSBundleLoadTagKey
kOSBundleLoadAddressKey
kOSBundleLoadSizeKey
kOSBundleWiredSizeKey
kOSBundleDependenciesKey
kOSBundleMetaClassesKey
#endif

#define kOptChars                "b:hkla"

/* Options with no single-letter variant.  */
// Do not use -1, that's getopt() end-of-args return value
// and can cause confusion
#define kLongOptLongindexHack (-2)

int longopt = 0;

struct option sOptInfo[] = {
    { kOptNameHelp,               no_argument,        NULL,     kOptHelp },
    { kOptNameBundleIdentifier,   required_argument,  NULL,     kOptBundleIdentifier },
    { kOptNameNoKernelComponents, no_argument,        NULL,     kOptNoKernelComponents },
    { kOptNameListOnly,           no_argument,        NULL,     kOptListOnly },
    { kOptNameSort,               no_argument,        NULL,     kOptSort },
    { kOptNameArch,               no_argument,        NULL,     kOptArchitecture },

    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};

#pragma mark Tool Args Structure
/*******************************************************************************
* Tool Args Structure
*******************************************************************************/
typedef struct {
    Boolean            flagNoKernelComponents;
    Boolean            flagListOnly;
    Boolean            flagShowArchitecture;
    Boolean            flagSortByLoadAddress;
    CFMutableArrayRef  bundleIDs;          // must release

    CFDictionaryRef    loadedKextInfo;     // must release
    const NXArchInfo * runningKernelArch;  // do not free
} KextstatArgs;

#pragma mark Function Prototypes
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
ExitStatus readArgs(int argc, char * const * argv, KextstatArgs * toolArgs);
void printKextInfo(CFDictionaryRef kextInfo, KextstatArgs * toolArgs);

Boolean getNumValue(CFNumberRef aNumber, CFNumberType type, void * valueOut);
int compareKextInfo(const void * vKextInfo1, const void * vKextInfo2);
int compareKextInfoLoadAddress(const void * vKextInfo1, const void * vKextInfo2);

CFComparisonResult compareNumbers(
    const void * val1,
    const void * val2,
          void * context);

static void usage(UsageLevel usageLevel);

#endif /* _KEXTSTAT_MAIN_H */
