#ifndef _KCLIST_MAIN_H
#define _KCLIST_MAIN_H

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
    kKclistExitOK          = EX_OK,

    // don't think we use it
    kKclistExitUnspecified = 11,

    // don't actually exit with this, it's just a sentinel value
    kKclistExitHelp        = 33,
};

#pragma mark Command-line Option Definitions
/*******************************************************************************
* Command-line options. This data is used by getopt_long_only().
*
* Options common to all kext tools are in kext_tools_util.h.
*******************************************************************************/

#define kOptArch         'a'

#define kOptUUID         'u'
#define kOptNameUUID     "uuid"

#define kOptMachO        'M'
#define kOptNameMachO    "macho"

#define kOptSaveKext     'x'
#define kOptNameSaveKext "extract"

#define kOptJSON         'j'
#define kOptNameJSON     "json"

#define kOptPrelinkInfoDict     'i'
#define kOptNamePrelinkInfoDict "info"

#define kOptOutput       'o'
#define kOptNameOutput   "output"

#define kOptChars  "a:hijlMo:uvx:"

int longopt = 0;

struct option sOptInfo[] = {
    { kOptNameHelp,                  no_argument,        NULL,     kOptHelp },
    { kOptNameArch,                  required_argument,  NULL,     kOptArch },
    { kOptNameMachO,                 no_argument,        NULL,     kOptMachO},
    { kOptNameUUID,                  no_argument,        NULL,     kOptUUID },
    { kOptNameSaveKext,              required_argument,  NULL,     kOptSaveKext },
    { kOptNameVerbose,               no_argument,        NULL,     kOptVerbose },
    { kOptNameLayoutMap,             no_argument,        NULL,     kOptLayoutMap },
    { kOptNameJSON,                  no_argument,        NULL,     kOptJSON },
    { kOptNamePrelinkInfoDict,       no_argument,        NULL,     kOptPrelinkInfoDict },
    { kOptNameOutput,                required_argument,  NULL,     kOptOutput },

    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};

typedef struct {
    char             * kernelcachePath;
    char             * extractedKextPath;
    CFMutableSetRef    kextIDs;
    const NXArchInfo * archInfo;
    Boolean            printMachO;
    Boolean            verbose;
    Boolean            printUUIDs;
    Boolean            printMap;
    Boolean            printJSON;
    Boolean            printPrelinkInfoDict;
    const char       * outputPath;
} KclistArgs;

/*
 * When extracting kexts from a kernel cache, we cannot assume that the kext macho header will correspond to the on-disk
 * offsets of the segments in which the kext lives. Each kext macho is setup to be valid in device memory after iBoot
 * has physically copied the kernel cache such that the physical to virtual mapping is a simple offset.
 */
struct kcmap_entry {
    struct load_command *kc_lcp;
    /* offset and size in device memory as setup by iBoot */
    uint64_t             va_start;
    uint64_t             va_end;
    /* offset and size in kernel cache binary */
    uint64_t             kc_start;
    uint64_t             kc_end;
};

struct kcmap {
    const uint8_t *cache_start;
    uint64_t va_start;
    int nentries;
    struct kcmap_entry entries[];
};

struct ImageInfo;

#pragma mark Function Prototypes
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
ExitStatus readArgs(
    int            * argc,
    char * const  ** argv,
    KclistArgs  * toolArgs);
ExitStatus checkArgs(KclistArgs * toolArgs);
void listPrelinkedKexts(KclistArgs * toolArgs,
                        struct ImageInfo * ki,
                        struct kcmap *kcmap,
                        CFPropertyListRef kcInfoPlist,
                        const NXArchInfo * archInfo);
void printKextInfo(KclistArgs *toolArgs,
                   struct ImageInfo *ki,
                   struct kcmap *kcmap,
                   CFDictionaryRef kextPlist);

void usage(UsageLevel usageLevel);

#endif /* _KCLIST_MAIN_H */
