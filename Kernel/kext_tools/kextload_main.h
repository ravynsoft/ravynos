/*
 *  kextload_main.h
 *  kext_tools
 *
 *  Created by Nik Gervae on 11/08/08.
 *  Copyright 2008 Apple Inc. All rights reserved.
 *
 */
#ifndef _KEXTLOAD_MAIN_H
#define _KEXTLOAD_MAIN_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/kext/OSKext.h>

#include <getopt.h>
#include <sysexits.h>

#include <IOKit/kext/OSKext.h>

#include "kext_tools_util.h"

#pragma mark Basic Types & Constants
/*******************************************************************************
* Constants
*******************************************************************************/

enum {
    kKextloadExitOK          = EX_OK,

    kKextloadExitUnspecified = 10,

    // This publicy documented exit code (TN2459) has the same value across both
    // tools (kextload and kextutil) and corresponds to kOSKextReturnSystemPolicy
    // at the KextManager / OSKext API layer.
    kKextloadExitSystemPolicy = 27,

    // don't actually exit with this, it's just a sentinel value
    kKextloadExitHelp        = 33,
};

#pragma mark Command-line Option Definitions
/*******************************************************************************
* Command-line options. This data is used by getopt_long_only().
*
* Options common to all kext tools are in kext_tools_util.h.
*******************************************************************************/
#define kOptNameDependency              "dependency"

#define kOptNameTests                   "print-diagnostics"

// Obsolete options that might be used for a production load
#define kOptNameNoCaches                "no-caches"
#define kOptNameNoLoadedCheck           "no-loaded-check"
#define kOptNameTests                   "print-diagnostics"

#define kOptNameLongindexHack           "________"

#define kOptDependency            'd'
#define kOptRepository            'r'

// Obsolete options that might be used for a production load
#define kOptNoCaches              'c'
#define kOptNoLoadedCheck         'D'
#define kOptTests                 't'

/* Options with no single-letter variant.  */
// Do not use -1, that's getopt() end-of-args return value
// and can cause confusion
#define kLongOptLongindexHack    (-2)
#define kLongOptArch             (-3)

#define kOptChars                "b:cd:Dhqr:tv"

int longopt = 0;

struct option sOptInfo[] = {
    { kOptNameLongindexHack,         no_argument,        &longopt, kLongOptLongindexHack },
    { kOptNameHelp,                  no_argument,        NULL,     kOptHelp },
    { kOptNameBundleIdentifier,      required_argument,  NULL,     kOptBundleIdentifier },
    { kOptNameDependency,            required_argument,  NULL,     kOptDependency },
    { kOptNameRepository,            required_argument,  NULL,     kOptRepository },

    { kOptNameQuiet,                 no_argument,        NULL,     kOptQuiet },
    { kOptNameVerbose,               optional_argument,  NULL,     kOptVerbose },
    { kOptNameTests,                 no_argument,        NULL,     kOptTests },

    // Obsolete options that might be used for a production load
    { kOptNameNoCaches,              no_argument,        NULL,     kOptNoCaches },
    { kOptNameNoLoadedCheck,         no_argument,        NULL,     kOptNoLoadedCheck },
    { kOptNameTests,                 no_argument,        NULL,     kOptTests },

    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};

typedef struct {
    CFMutableArrayRef      kextIDs;          // -b; must release
    CFMutableArrayRef      dependencyURLs;   // -d; must release
    CFMutableArrayRef      repositoryURLs;   // -r; must release
    CFMutableArrayRef      kextURLs;         // kext args; must release
    CFMutableArrayRef      scanURLs;         // all URLs to scan
    CFArrayRef             allKexts;         // all opened kexts
} KextloadArgs;

#pragma mark Function Prototypes
/*******************************************************************************
* Function Prototypes
*******************************************************************************/

ExitStatus readArgs(
    int argc,
    char * const * argv,
    KextloadArgs * toolArgs);
ExitStatus checkArgs(KextloadArgs * toolArgs);

ExitStatus checkAccess(void);

ExitStatus loadKextsViaKextd(KextloadArgs * toolArgs);
ExitStatus loadKextsIntoKernel(KextloadArgs * toolArgs);

ExitStatus exitStatusForOSReturn(OSReturn osReturn);

static void usage(UsageLevel usageLevel);

#endif /* _KEXTLOAD_MAIN_H */
