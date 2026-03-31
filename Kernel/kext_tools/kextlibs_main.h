/*
 *  kextlibs_main.h
 *  kext_tools
 *
 *  Created by Nik Gervae on 5/03/08.
 *  Copyright 2008 Apple Inc. All rights reserved.
 *
 */
#ifndef _KEXTLIBS_MAIN_H
#define _KEXTLIBS_MAIN_H

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
    kKextlibsExitOK          = EX_OK,
    kKextlibsExitUndefineds  = 1,
    kKextlibsExitMultiples   = 2,

    // don't actually exit with this, it's just a sentinel value
    kKextlibsExitHelp        = 33
};

#pragma mark Command-line Option Definitions
/*******************************************************************************
* Command-line options. This data is used by getopt_long_only().
*
* Options common to all kext tools are in kext_tools_util.h.
*******************************************************************************/

#define kOptNameXML               "xml"
#define kOptNameCompatible        "compatible-versions"
#define kOptNameAllSymbols        "all-symbols"
#define kOptNameUndefSymbols      "undef-symbols"
#define kOptNameOnedefSymbols     "onedef-symbols"
#define kOptNameMultdefSymbols    "multdef-symbols"
#define kOptNameNonKPI            "non-kpi"
#define kOptNameUnsupported       "unsupported"
#define kOptNameLibraryRefs       "library-references"

#define kOptCompatible           'c'
#define kOptSystemExtensions     'e'
#define kOptRepository           'r'

#define kOptChars                "cehr:"

/* Options with no single-letter variant.  */
// Do not use -1, that's getopt() end-of-args return value
// and can cause confusion
#define kLongOptLongindexHack  (-2)
#define kLongOptXML            (-3)
#define kLongOptAllSymbols     (-4)
#define kLongOptUndefSymbols   (-5)
#define kLongOptOnedefSymbols  (-6)
#define kLongOptMultdefSymbols (-7)
#define kLongOptNonKPI         (-8)
#define kLongOptUnsupported    (-9)

int longopt = 0;

struct option sOptInfo[] = {
    { kOptNameHelp,             no_argument,        NULL,     kOptHelp },
    { kOptNameSystemExtensions, no_argument,        NULL,     kOptSystemExtensions },
    { kOptNameRepository,       required_argument,  NULL,     kOptRepository },
    { kOptNameCompatible,       no_argument,        NULL,     kOptCompatible },
    { kOptNameXML,              no_argument,        &longopt, kLongOptXML },
    { kOptNameAllSymbols,       no_argument,        &longopt, kLongOptAllSymbols },
    { kOptNameUndefSymbols,     no_argument,        &longopt, kLongOptUndefSymbols },
    { kOptNameOnedefSymbols,    no_argument,        &longopt, kLongOptOnedefSymbols },
    { kOptNameMultdefSymbols,   no_argument,        &longopt, kLongOptMultdefSymbols },
    { kOptNameNonKPI,           no_argument,        &longopt, kLongOptNonKPI },
    { kOptNameUnsupported,      no_argument,        &longopt, kLongOptUnsupported },

    { kOptNameQuiet,                 no_argument,        NULL,     kOptQuiet },
    { kOptNameVerbose,               optional_argument,  NULL,     kOptVerbose },

    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};

#pragma mark Tool Args Structure
/*******************************************************************************
* Tool Args Structure
*******************************************************************************/
typedef struct {
    Boolean            flagSysKexts;
    Boolean            flagXML;
    Boolean            flagCompatible;
    Boolean            flagPrintUndefSymbols;
    Boolean            flagPrintOnedefSymbols;
    Boolean            flagPrintMultdefSymbols;
    Boolean            flagNonKPI;
    Boolean            flagAllowUnsupported;
    CFMutableArrayRef  repositoryURLs;   // must release
    char             * kextName;         // do not free; from argv
    CFURLRef           kextURL;          // must release
    OSKextRef          theKext;
} KextlibsArgs;

typedef struct {
    CFArrayRef       libKexts;
    CFDictionaryRef  undefSymbols;
    CFDictionaryRef  onedefSymbols;
    CFDictionaryRef  multdefSymbols;
    CFArrayRef       multdefLibs;
} KextlibsInfo;

#pragma mark Function Prototypes
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
ExitStatus readArgs(
    int            argc,
    char * const * argv,
    KextlibsArgs * toolArgs);
ExitStatus addRepository(
    KextlibsArgs    * toolArgs,
    const char      * optarg);
ExitStatus printLibs(
    KextlibsArgs     * toolArgs,
    const NXArchInfo * arch,
    CFArrayRef         libKexts,
    Boolean            trailingNewlineFlag);
ExitStatus printProblems(
    KextlibsArgs     * toolArgs,
    const NXArchInfo * arch,
    CFDictionaryRef    undefSymbols,
    CFDictionaryRef    onedefSymbols,
    CFDictionaryRef    multdefSymbols,
    CFArrayRef         multdefLibs,
    Boolean            printArchFlag,
    Boolean            trailingNewlineFlag);
ExitStatus printResults(
    KextlibsArgs     * toolArgs,
    const NXArchInfo * arch,
    Boolean            archSpecific,
    CFArrayRef         libKexts,
    CFDictionaryRef    undefSymbols,
    CFDictionaryRef    onedefSymbols,
    CFDictionaryRef    multdefSymbols,
    CFArrayRef         multdefLibs);
void printUndefSymbol(const void * key, const void * value, void * context);
void printOnedefSymbol(const void * key, const void * value, void * context);
void printMultdefSymbol(const void * key, const void * value, void * context);

static void usage(UsageLevel usageLevel);

#endif /* _KEXTLIBS_MAIN_H */
