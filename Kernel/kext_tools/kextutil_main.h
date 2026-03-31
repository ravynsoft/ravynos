/*
 *  kextutil_main.h
 *  kext_tools
 *
 *  Created by Nik Gervae on 4/24/08.
 *  Copyright 2008 Apple Inc. All rights reserved.
 *
 */
#ifndef _KEXTUTIL_MAIN_H
#define _KEXTUTIL_MAIN_H

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
    kKextutilExitOK          = EX_OK,
    kKextutilExitNotFound,
    kKextutilExitArchNotFound,
    kKextutilExitUserAbort,
    kKextutilExitKextBad,
    kKextutilExitSafeBoot,
    kKextutilExitLoadFailed,
    kKextutilExitInteractionRequired,

    // don't think we use it
    kKextutilExitUnspecified = 11,

    // This publicy documented exit code (TN2459) has the same value across both
    // tools (kextload and kextutil) and corresponds to kOSKextReturnSystemPolicy
    // at the KextManager / OSKext API layer.
    kKextutilExitSystemPolicy = 27,

    // don't actually exit with this, it's just a sentinel value
    kKextutilExitHelp        = 33,
    kKextutilExitNoStart
};

#pragma mark Command-line Option Definitions
/*******************************************************************************
* Command-line options. This data is used by getopt_long_only().
*
* Options common to all kext tools are in kext_tools_util.h.
*******************************************************************************/
#define kOptNamePersonality             "personality"
#define kOptNameDependency              "dependency"

#define kOptNameNoCaches                "no-caches"
#define kOptNameNoLoadedCheck           "no-loaded-check"
#define kOptNameNoSystemExtensions      "no-system-extensions"

#define kOptNameInteractive             "interactive"
#define kOptNameInteractiveAll          "interactive-all"

#define kOptNameLoadOnly                "load-only"
#define kOptNameMatchOnly               "match-only"
#define kOptNameNoLoad                  "no-load"
#define kOptNameSymbolsDirectory        "symbols"
#define kOptNameAddress                 "address"
#define kOptNameUseKernelAddresses      "use-load-addresses"

#define kOptNameTests                   "print-diagnostics"
#define kOptNameNoResolveDependencies   "no-resolve-dependencies"

#define kOptNameLongindexHack           "________"

#define kOptPersonality           'p'
#define kOptKernel                'k'
#define kOptDependency            'd'
#define kOptRepository            'r'

#define kOptNoCaches              'c'
#define kOptNoLoadedCheck         'D'
#define kOptNoSystemExtensions    'e'

#define kOptInteractive           'i'
#define kOptInteractiveAll        'I'

#define kOptLoadOnly              'l'
#define kOptMatchOnly             'm'
#define kOptNoLoad                'n'
#define kOptSymbolsDirectory      's'
#define kOptAddress               'a'
#define kOptUseKernelAddresses    'A'
    // arch is down below

#define kOptTests                 't'
#define kOptSafeBoot              'x'
#define kOptNoAuthentication      'z'
#define kOptNoResolveDependencies 'Z'

/* Options with no single-letter variant.  */
// Do not use -1, that's getopt() end-of-args return value
// and can cause confusion
#define kLongOptLongindexHack    (-2)
#define kLongOptArch             (-3)
#define kLongOptLegacyLayout     (-4)

#define kOptChars                "a:Ab:cd:DehiIk:lmnp:qr:s:tvxzZ"

int longopt = 0;

struct option sOptInfo[] = {
    { kOptNameLongindexHack,         no_argument,        &longopt, kLongOptLongindexHack },
    { kOptNameHelp,                  no_argument,        NULL,     kOptHelp },
    { kOptNameBundleIdentifier,      required_argument,  NULL,     kOptBundleIdentifier },
    { kOptNamePersonality,           required_argument,  NULL,     kOptPersonality },
    { kOptNameKernel,                required_argument,  NULL,     kOptKernel },
    { kOptNameDependency,            required_argument,  NULL,     kOptDependency },
    { kOptNameRepository,            required_argument,  NULL,     kOptRepository },
    { kOptNameNoCaches,              no_argument,        NULL,     kOptNoCaches },
    { kOptNameNoLoadedCheck,         no_argument,        NULL,     kOptNoLoadedCheck },
    { kOptNameNoSystemExtensions,    no_argument,        NULL,     kOptNoSystemExtensions },
    { kOptNameInteractive,           no_argument,        NULL,     kOptInteractive },
    { kOptNameInteractiveAll,        no_argument,        NULL,     kOptInteractiveAll },
    { kOptNameLoadOnly,              no_argument,        NULL,     kOptLoadOnly },
    { kOptNameMatchOnly,             no_argument,        NULL,     kOptMatchOnly },
    { kOptNameNoLoad,                no_argument,        NULL,     kOptNoLoad },
    { kOptNameSymbolsDirectory,      required_argument,  NULL,     kOptSymbolsDirectory },
    { kOptNameAddress,               required_argument,  NULL,     kOptAddress },
    { kOptNameUseKernelAddresses,    no_argument,        NULL,     kOptUseKernelAddresses },
    { kOptNameQuiet,                 no_argument,        NULL,     kOptQuiet },
    { kOptNameVerbose,               optional_argument,  NULL,     kOptVerbose },
    { kOptNameTests,                 no_argument,        NULL,     kOptTests },
    { kOptNameSafeBoot,              no_argument,        NULL,     kOptSafeBoot },
    { kOptNameNoAuthentication,      no_argument,        NULL,     kOptNoAuthentication },
    { kOptNameNoResolveDependencies, no_argument,        NULL,     kOptNoResolveDependencies },
    { kOptNameArch,                  required_argument,  &longopt, kLongOptArch },

    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};

typedef struct {
   /* These are ints so we can check for multiply set real easy.
    */
    int                    flag_n;  // used to sanity-check -n, -l, -m
    int                    flag_l;  // before setting behavior-changing
    int                    flag_m;  // variables doLoad & doStartMatching

    Boolean                getAddressesFromKernel;          // -A
    Boolean                useRepositoryCaches;             // -c to turn off
    Boolean                useSystemExtensions;             // -e to turn off

    Boolean                overwriteSymbols;     // -i/-I turns off
    OSKextExcludeLevel     interactiveLevel;     // -i/-I turns on

    Boolean                doLoad;                          // -l: load but no matching
    Boolean                doStartMatching;                 // -m: don't load, start matching
                                             // -n: don't do either

    Boolean                printDiagnostics;                // -t
    Boolean                safeBootMode;                    // -x
    Boolean                skipAuthentication;              // -z
    Boolean                skipDependencies;                // -Z (and with -t only!)
    Boolean                checkLoadedForDependencies;      // -D to turn off (obsolete)
    Boolean                logFilterChanged;

    CFMutableDictionaryRef loadAddresses;    // -a; must release
    CFMutableArrayRef      kextIDs;          // -b; must release
    CFMutableArrayRef      personalityNames; // -p; must release
    CFMutableArrayRef      dependencyURLs;   // -d; must release
    CFMutableArrayRef      repositoryURLs;   // -r; must release
    CFMutableArrayRef      kextURLs;         // kext args; must release
    CFMutableArrayRef      scanURLs;         // all URLs to scan

    CFURLRef               kernelURL;        // overriden by -kernel option
    CFDataRef              kernelFile;       // contents of kernelURL
    CFURLRef               symbolDirURL;     // -s option;
    const NXArchInfo     * archInfo;         // set by -arch
} KextutilArgs;

#pragma mark Function Prototypes
/*******************************************************************************
* Function Prototypes
*******************************************************************************/

ExitStatus readArgs(
    int argc,
    char * const * argv,
    KextutilArgs * toolArgs);
ExitStatus checkArgs(KextutilArgs * toolArgs);
void adjustLogFilterForInteractive(KextutilArgs * toolArgs);
ExitStatus createKextsToProcess(
    KextutilArgs * toolArgs,
    CFArrayRef   * outArray,
    Boolean      * fatal);
ExitStatus processKexts(
    CFArrayRef     kextURLsToUse,
    KextutilArgs * toolArgs);
ExitStatus processKext(
    OSKextRef      aKext,
    KextutilArgs * toolArgs,
    Boolean      * fatal);
ExitStatus runTestsOnKext(
    OSKextRef aKext,
    char         * kextPathCString,
    KextutilArgs * toolArgs,
    Boolean      * fatal);
ExitStatus loadKext(
    OSKextRef     aKext,
    char         * kextPathCString,
    KextutilArgs * toolArgs,
    Boolean      * fatal);

void notifyNonsecureKextload(OSKextRef aKext);

ExitStatus generateKextSymbols(
    OSKextRef      aKext,
    char         * kextPathCString,
    KextutilArgs * toolArgs,
    Boolean        saveFlag,
    Boolean      * fatal);
void setKextLoadAddress(
    const void * vKey,
    const void * vValue,
    void       * vContext);
int requestLoadAddress(
    OSKextRef aKext);
ExitStatus startKextsAndSendPersonalities(
    OSKextRef      aKext,
    KextutilArgs * toolArgs,
    Boolean       * fatal);
ExitStatus startKext(
    OSKextRef      aKext,
    char         * kextPathCString,
    KextutilArgs * toolArgs,
    Boolean      * started,
    Boolean       * yesToAll,
    Boolean      * fatal);
ExitStatus sendPersonalities(
    OSKextRef      aKext,
    char         * kextPathCString,
    KextutilArgs * toolArgs,
    Boolean        isMainFlag,
    Boolean      * yesToAll,
    Boolean      * fatal);

Boolean serializeLoad(
    KextutilArgs * toolArgs,
    Boolean        loadFlag);
static void usage(UsageLevel usageLevel);

extern kern_return_t kextmanager_lock_kextload(
    mach_port_t server,
    mach_port_t client,
    int * lockstatus);
kern_return_t kextmanager_unlock_kextload(
    mach_port_t server,
    mach_port_t client);

#endif /* _KEXTUTIL_MAIN_H */
