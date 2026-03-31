/*
 *  kcgen_main.h
 *  kext_tools
 *
 *  Created by nik on 5/20/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _KCGEN_MAIN_H
#define _KCGEN_MAIN_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/kext/OSKext.h>

#include <getopt.h>
#include <sysexits.h>

#include <IOKit/kext/OSKext.h>

#include "kext_tools_util.h"
#include "kernelcache.h"

#pragma mark Basic Types & Constants
/*******************************************************************************
* Constants
*******************************************************************************/

enum {
    kKcgenExitOK          = EX_OK,
    kKcgenExitNotFound,
    kKcgenExitArchNotFound,
    kKcgenExitKextBad,
    kKcgenExitStale,

    // don't think we use it
    kKcgenExitUnspecified = 11,

    // don't actually exit with this, it's just a sentinel value
    kKcgenExitHelp        = 33,
    kKcgenExitNoStart
};

#pragma mark Command-line Option Definitions
/*******************************************************************************
* Command-line options. This data is used by getopt_long_only().
*
* Options common to all kext tools are in kext_tools_util.h.
*******************************************************************************/

// kOptNameBundleIdentifier in kext_tools_util.h
// kOptNameSystemExtensions in kext_tools_util.h

#define kOptNameOptionalBundleIdentifier  "optional-bundle-id"

/* Prelinked-kernel-generation flags.
 */
#define kOptNamePrelinkedKernel         "prelinked-kernel"
#define kOptNameSystemPrelinkedKernel   "system-prelinked-kernel"
#define kOptNameKernel                  "kernel"
#define kOptNameAllLoaded               "all-loaded"
#define kOptNameSymbols                 "symbols"
#define kOptNameVolumeRoot              "volume-root"

/* Embedded prelinked-kernel-generation flags.
 */
#define kOptNameAllPersonalities        "all-personalities"
#define kOptNameNoLinkFailures          "no-link-failures"
#define kOptNameStripSymbols            "strip-symbols"

#define kOptNameMaxSliceSize            "max-slice-size"

#define kOptNamePLists                  "plists"
#define kOptNameLoadList                "load-list"
/* Misc flags.
 */
#define kOptNameNoAuthentication        "no-authentication"
#define kOptNameTests                   "print-diagnostics"
#define kOptNameCompressed              "compressed"
#define kOptNameUncompressed            "uncompressed"

#define kOptArch                  'a'
// 'b' in kext_tools_util.h
#define kOptPrelinkedKernel       'c'
// 'h' in kext_tools_util.h
#define kOptKernel                'K'
// 'q' in kext_tools_util.h
#define kOptTests                 't'
// 'T' in kext_tools_util.h
// 'v' in kext_tools_util.h
#define kOptNoAuthentication      'z'

/* Options with no single-letter variant.  */
// Do not use -1, that's getopt() end-of-args return value
// and can cause confusion
#define kLongOptLongindexHack             (-2)
#define kLongOptOptionalBundleIdentifier  (-3)
#define kLongOptCompressed                (-5)
#define kLongOptUncompressed              (-6)
#define kLongOptSymbols                   (-7)
#define kLongOptVolumeRoot               (-10)
#define kLongOptAllPersonalities         (-11)
#define kLongOptNoLinkFailures           (-12)
#define kLongOptStripSymbols             (-13)
#define kLongOptMaxSliceSize             (-14)
#define kLongOptPLists                   (-15)
#define kLongOptLoadList                 (-16)

#define kOptChars                ":a:b:c:ehK:lLnNqsStT:vz"

int longopt = 0;

struct option sOptInfo[] = {
    { kOptNameLongindexHack,            no_argument,        &longopt, kLongOptLongindexHack },

    { kOptNameHelp,                     no_argument,        NULL,     kOptHelp },
    { kOptNameQuiet,                    no_argument,        NULL,     kOptQuiet },
    { kOptNameVerbose,                  optional_argument,  NULL,     kOptVerbose },
    { kOptNameCompressed,               optional_argument,  &longopt, kLongOptCompressed },
    { kOptNameUncompressed,             no_argument,        &longopt, kLongOptUncompressed },

    { kOptNameArch,                     required_argument,  NULL,     kOptArch },
    { kOptNameTargetOverride,           required_argument,  NULL,     kOptTargetOverride },
    { kOptNameVolumeRoot,               required_argument,  &longopt, kLongOptVolumeRoot },

    { kOptNameBundleIdentifier,         required_argument,  NULL,     kOptBundleIdentifier },
    { kOptNameOptionalBundleIdentifier, required_argument,  &longopt, kLongOptOptionalBundleIdentifier },

    { kOptNamePrelinkedKernel,          optional_argument,  NULL,     kOptPrelinkedKernel },
    { kOptNameKernel,                   required_argument,  NULL,     kOptKernel },
    { kOptNameSymbols,                  required_argument,  &longopt, kLongOptSymbols },

    { kOptNameTests,                    no_argument,        NULL,     kOptTests },

    { kOptNameMaxSliceSize,             required_argument,  &longopt, kLongOptMaxSliceSize },

    { kOptNamePLists,                   required_argument,  &longopt, kLongOptPLists },
    { kOptNameLoadList,                 required_argument,  &longopt, kLongOptLoadList },

    /* Always on for kcgen; can be removed at some point. */
    { kOptNameAllPersonalities,         no_argument,        &longopt, kLongOptAllPersonalities },
    { kOptNameNoLinkFailures,           no_argument,        &longopt, kLongOptNoLinkFailures },
    { kOptNameStripSymbols,             no_argument,        &longopt, kLongOptStripSymbols },
    { kOptNameNoAuthentication,         no_argument,        NULL,     kOptNoAuthentication },

    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};

typedef struct {
    Boolean   printTestResults;     // -t

    char    * prelinkedKernelPath;            // -c option
    Boolean   generatePrelinkedSymbols;     // -symbols option
    Boolean   stripSymbols;                 // -strip-symbols option
    CFIndex   maxSliceSize;

    CFURLRef  compressedPrelinkedKernelURL; // -uncompress option

    char    * kernelPath;    // overriden by -K option
    CFDataRef kernelFile;    // contents of kernelURL
    CFURLRef  symbolDirURL;  // -s option;
    CFURLRef  volumeRootURL;

    char    * plistsPath;
    char    * loadListPath;

    CFMutableSetRef    kextIDs;          // -b; must release
    CFMutableSetRef    optionalKextIDs;  // -optional-bundle-id; must release
    CFMutableArrayRef  argURLs;          // directories & kexts in order
    CFMutableArrayRef  repositoryURLs;   // just non-kext directories
    CFMutableArrayRef  namedKextURLs;
    CFMutableArrayRef  targetArchs;
    char             * targetForKextVariants;

    CFArrayRef         allKexts;         // directories + named
    CFArrayRef         repositoryKexts;  // all from directories (may include named)
    CFArrayRef         namedKexts;

    Boolean     compress;
    uint32_t    compressionType;
    Boolean     uncompress;
} KcgenArgs;

#pragma mark Function Prototypes
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
ExitStatus readArgs(
    int            * argc,
    char * const  ** argv,
    KcgenArgs  * toolArgs);
const NXArchInfo * addArchForName(
    KcgenArgs * toolArgs,
    const char    * archname);
ExitStatus readPrelinkedKernelArgs(
    KcgenArgs * toolArgs,
    int             argc,
    char * const  * argv,
    Boolean         isLongopt);
ExitStatus setPrelinkedKernelArgs(
    KcgenArgs * toolArgs,
    char          * filename);

ExitStatus checkArgs(KcgenArgs * toolArgs);

ExitStatus writeFatFile(
    const char                * filePath,
    CFArrayRef                  fileSlices,
    CFArrayRef                  fileArchs,
    mode_t                      fileMode,
    const struct timeval        fileTimes[2]);
ExitStatus checkKextForProblems(
    KcgenArgs         * toolArgs,
    OSKextRef           theKext,
    const NXArchInfo  * arch);
ExitStatus filterKextsForCache(
    KcgenArgs     * toolArgs,
    CFMutableArrayRef   kextArray,
    const NXArchInfo  * arch,
    Boolean           * fatalOut);
ExitStatus createPrelinkedKernelArchs(
    KcgenArgs     * toolArgs,
    CFMutableArrayRef * prelinkArchsOut);
ExitStatus createExistingPrelinkedSlices(
    KcgenArgs     * toolArgs,
    CFMutableArrayRef * prelinkedSlicesOut,
    CFMutableArrayRef * prelinkedArchsOut);
ExitStatus createPrelinkedKernel(
    KcgenArgs     * toolArgs);
CFArrayRef mergeArchs(
    CFArrayRef  archSet1,
    CFArrayRef  archSet2);
ExitStatus createPrelinkedKernelForArch(
    KcgenArgs       * toolArgs,
    CFDataRef           * prelinkedKernelOut,
    CFDictionaryRef     * prelinkedSymbolsOut,
    const NXArchInfo    * archInfo);
ExitStatus compressPrelinkedKernel(
    const char        * prelinkedKernelPath,
    Boolean             compress,
    uint32_t            compressionType);
void logUsedKexts(
    KcgenArgs       * toolArgs,
    CFArrayRef        prelinkKexts);
ExitStatus
loadList(
    KcgenArgs     * toolArgs);

void usage(UsageLevel usageLevel);

#endif /* _KCGEN_MAIN_H */
