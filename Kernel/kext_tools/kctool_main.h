#ifndef _KCTOOL_MAIN_H
#define _KCTOOL_MAIN_H

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
    kKctoolExitOK          = EX_OK,

    // don't think we use it
    kKctoolExitUnspecified = 11,

    // don't actually exit with this, it's just a sentinel value
    kKctoolExitHelp        = 33,
};

#pragma mark Command-line Option Definitions
/*******************************************************************************
* Command-line options. This data is used by getopt_long_only().
*
* Options common to all kext tools are in kext_tools_util.h.
*******************************************************************************/

#define kOptArch   'a'

#define kOptChars  "a:h"

int longopt = 0;

struct option sOptInfo[] = {
    { kOptNameHelp,                  no_argument,        NULL,     kOptHelp },
    { kOptNameArch,                  required_argument,  NULL,     kOptArch },

    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};

typedef struct {
    const NXArchInfo * archInfo;

    char             * kernelcachePath;
    CFStringRef        kextID;
    const char       * segmentName;
    const char       * sectionName;

    const UInt8      * kernelcacheImageBytes;
    CFPropertyListRef  kernelcacheInfoPlist;

} KctoolArgs;

#pragma mark Function Prototypes
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
ExitStatus readArgs(
    int            * argc,
    char * const  ** argv,
    KctoolArgs  * toolArgs);
ExitStatus printKextInfo(KctoolArgs * toolArgs);
Boolean getKextAddressAndSize(CFDictionaryRef infoDict, uint64_t *addr, uint64_t *size);

void usage(UsageLevel usageLevel);

#endif /* _KCTOOL_MAIN_H */
