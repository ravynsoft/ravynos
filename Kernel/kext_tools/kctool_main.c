#include <CoreFoundation/CoreFoundation.h>
#include <System/libkern/OSKextLibPrivate.h>
#include <System/libkern/prelink.h>
#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>
#include <IOKit/IOCFUnserialize.h>
#include <IOKit/kext/macho_util.h>

#include <architecture/byte_order.h>
#include <errno.h>
#include <libc.h>
#include <mach-o/fat.h>
#include <sys/mman.h>

#include "kctool_main.h"
#include "compression.h"

/*******************************************************************************
* Program Globals
*******************************************************************************/
const char * progname = "(unknown)";

/*******************************************************************************
*******************************************************************************/
int main(int argc, char * const argv[])
{
    ExitStatus           result             = EX_SOFTWARE;
    KctoolArgs           toolArgs;
    int                  kernelcache_fd     = -1;  // must close()
    void               * fat_header         = NULL;  // must unmapFatHeaderPage()
    struct fat_arch    * fat_arch           = NULL;
    CFDataRef            rawKernelcache     = NULL;  // must release
    CFDataRef            kernelcacheImage   = NULL;  // must release

    void               * prelinkInfoSect    = NULL;

    const char         * prelinkInfoBytes   = NULL;
    CFPropertyListRef    prelinkInfoPlist   = NULL;  // must release

    bzero(&toolArgs, sizeof(toolArgs));

   /*****
    * Find out what the program was invoked as.
    */
    progname = rindex(argv[0], '/');
    if (progname) {
        progname++;   // go past the '/'
    } else {
        progname = (char *)argv[0];
    }

   /* Set the OSKext log callback right away.
    */
    OSKextSetLogOutputFunction(&tool_log);

   /*****
    * Process args & check for permission to load.
    */
    result = readArgs(&argc, &argv, &toolArgs);
    if (result != EX_OK) {
        if (result == kKctoolExitHelp) {
            result = EX_OK;
        }
        goto finish;
    }

    kernelcache_fd = open(toolArgs.kernelcachePath, O_RDONLY);
    if (kernelcache_fd == -1) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't open %s: %s.", toolArgs.kernelcachePath, strerror(errno));
        result = EX_OSERR;
        goto finish;
    }
    fat_header = mapAndSwapFatHeaderPage(kernelcache_fd);
    if (!fat_header) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't map %s: %s.", toolArgs.kernelcachePath, strerror(errno));
        result = EX_OSERR;
        goto finish;
    }

    fat_arch = getFirstFatArch(fat_header);
    if (fat_arch && !toolArgs.archInfo) {
        toolArgs.archInfo = NXGetArchInfoFromCpuType(fat_arch->cputype, fat_arch->cpusubtype);
    }

    rawKernelcache = readMachOSliceForArch(toolArgs.kernelcachePath, toolArgs.archInfo,
        /* checkArch */ FALSE);
    if (!rawKernelcache) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't read arch %s from %s.", toolArgs.archInfo->name, toolArgs.kernelcachePath);
        goto finish;
    }

    if (MAGIC32(CFDataGetBytePtr(rawKernelcache)) == OSSwapHostToBigInt32('comp')) {
        kernelcacheImage = uncompressPrelinkedSlice(rawKernelcache);
        if (!kernelcacheImage) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Can't uncompress kernelcache slice.");
            goto finish;
        }
    } else {
        kernelcacheImage = CFRetain(rawKernelcache);
    }

    toolArgs.kernelcacheImageBytes = CFDataGetBytePtr(kernelcacheImage);

    if (ISMACHO64(MAGIC32(toolArgs.kernelcacheImageBytes))) {
        prelinkInfoSect = (void *)macho_get_section_by_name_64(
            (struct mach_header_64 *)toolArgs.kernelcacheImageBytes,
            "__PRELINK_INFO", "__info");

    } else {
        prelinkInfoSect = (void *)macho_get_section_by_name(
            (struct mach_header *)toolArgs.kernelcacheImageBytes,
            "__PRELINK_INFO", "__info");
    }

    if (!prelinkInfoSect) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't find prelink info section.");
        goto finish;
    }

    if (ISMACHO64(MAGIC32(toolArgs.kernelcacheImageBytes))) {
        prelinkInfoBytes = ((char *)toolArgs.kernelcacheImageBytes) +
            ((struct section_64 *)prelinkInfoSect)->offset;
    } else {
        prelinkInfoBytes = ((char *)toolArgs.kernelcacheImageBytes) +
            ((struct section *)prelinkInfoSect)->offset;
    }

    toolArgs.kernelcacheInfoPlist = (CFPropertyListRef)IOCFUnserialize(prelinkInfoBytes,
        kCFAllocatorDefault, /* options */ 0, /* errorString */ NULL);
    if (!toolArgs.kernelcacheInfoPlist) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't unserialize prelink info.");
        goto finish;
    }

    result = printKextInfo(&toolArgs);
    if (result != EX_OK) {
        goto finish;
    }

    result = EX_OK;

finish:

    SAFE_RELEASE(toolArgs.kernelcacheInfoPlist);
    SAFE_RELEASE(kernelcacheImage);
    SAFE_RELEASE(rawKernelcache);

    if (fat_header) {
        unmapFatHeaderPage(fat_header);
    }

    if (kernelcache_fd != -1) {
        close(kernelcache_fd);
    }
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus readArgs(
    int            * argc,
    char * const  ** argv,
    KctoolArgs     * toolArgs)
{
    ExitStatus   result         = EX_USAGE;
    ExitStatus   scratchResult  = EX_USAGE;
    int          optchar        = 0;
    int          longindex      = -1;

    bzero(toolArgs, sizeof(*toolArgs));

    /*****
    * Process command line arguments.
    */
    while ((optchar = getopt_long_only(*argc, *argv,
        kOptChars, sOptInfo, &longindex)) != -1) {

        switch (optchar) {

            case kOptArch:
                toolArgs->archInfo = NXGetArchInfoFromName(optarg);
                if (!toolArgs->archInfo) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                        "Unknown architecture %s.", optarg);
                    goto finish;
                }
                break;

            case kOptHelp:
                usage(kUsageLevelFull);
                result = kKctoolExitHelp;
                goto finish;

            default:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "unrecognized option %s", (*argv)[optind-1]);
                goto finish;
                break;

        }

       /* Reset longindex, because getopt_long_only() is stupid and doesn't.
        */
        longindex = -1;
    }

   /* Update the argc & argv seen by main() so that boot<>root calls
    * handle remaining args.
    */
    *argc -= optind;
    *argv += optind;

    if (*argc != 4) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "incorrect number of arguments");
        goto finish;
    }

   /*****
    * Record remaining args from the command line.
    */
    toolArgs->kernelcachePath = (*argv)[0];
    toolArgs->kextID = CFStringCreateWithCString(kCFAllocatorDefault, (*argv)[1], kCFStringEncodingUTF8);
    if (!toolArgs->kextID) {
        OSKextLogMemError();
        result = EX_OSERR;
        goto finish;
    }
    toolArgs->segmentName = (*argv)[2];
    toolArgs->sectionName = (*argv)[3];

    result = EX_OK;

finish:
    if (result == EX_USAGE) {
        usage(kUsageLevelBrief);
    }

    return result;
}

/*******************************************************************************
 *******************************************************************************/
static struct section *
getSectionByName(const UInt8 *file, const char *segname, const char *sectname)
{
    struct mach_header *machHeader;
    struct load_command *cmdHeader;
    struct segment_command *segmentHeader;
    struct section *sectionHeader;
    u_long offset;
    u_int i, j;

    machHeader = (struct mach_header *) file;
    if (machHeader->magic != MH_MAGIC) return NULL;

    offset = sizeof(*machHeader);
    for (i = 0; i < machHeader->ncmds; ++i, offset+=cmdHeader->cmdsize) {
        cmdHeader = (struct load_command *) (file + offset);
        if (cmdHeader->cmd != LC_SEGMENT) continue;

        segmentHeader = (struct segment_command *)cmdHeader;
        sectionHeader = (struct section *) (file + offset + sizeof(*segmentHeader));
        for (j = 0; j < segmentHeader->nsects; ++j, ++sectionHeader) {
            if (!strncmp(sectionHeader->segname, segname, sizeof(sectionHeader->segname)) &&
                !strncmp(sectionHeader->sectname, sectname, sizeof(sectionHeader->sectname)))
            {
                return sectionHeader;
            }
        }
    }

    return NULL;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus printKextInfo(KctoolArgs * toolArgs)
{
    ExitStatus result         = EX_SOFTWARE;
    CFArrayRef kextPlistArray = NULL;
    CFIndex    i, count;

    if (CFArrayGetTypeID() == CFGetTypeID(toolArgs->kernelcacheInfoPlist)) {
        kextPlistArray = (CFArrayRef)toolArgs->kernelcacheInfoPlist;
    } else if (CFDictionaryGetTypeID() == CFGetTypeID(toolArgs->kernelcacheInfoPlist)){
        kextPlistArray = (CFArrayRef)CFDictionaryGetValue(toolArgs->kernelcacheInfoPlist,
            CFSTR("_PrelinkInfoDictionary"));
    } else {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Unrecognized kernelcache plist data.");
        goto finish;
    }

    count = CFArrayGetCount(kextPlistArray);
    for (i = 0; i < count; i++) {
        CFDictionaryRef kextInfoDict = (CFDictionaryRef)CFArrayGetValueAtIndex(kextPlistArray, i);
        CFStringRef     thisKextID = CFDictionaryGetValue(kextInfoDict, kCFBundleIdentifierKey);

        if (thisKextID && CFEqual(thisKextID, toolArgs->kextID)) {
            uint64_t      kextAddr   = 0;
            uint64_t      kextSize   = 0;
            u_long        kextOffset = 0;
            const UInt8 * kextMachO  = NULL;  // do not free
            void        * section    = NULL;  // do not free

            if (!getKextAddressAndSize(kextInfoDict, &kextAddr, &kextSize)) {
                goto finish;
            }

            if (ISMACHO64(MAGIC32(toolArgs->kernelcacheImageBytes))) {
                section = (void *)macho_get_section_by_name_64(
                    (struct mach_header_64 *)toolArgs->kernelcacheImageBytes,
                    kPrelinkTextSegment, kPrelinkTextSection);

            } else {
                section = (void *)macho_get_section_by_name(
                    (struct mach_header *)toolArgs->kernelcacheImageBytes,
                    kPrelinkTextSegment, kPrelinkTextSection);
            }

            if (!section) {
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Cannot find %s,%s in kernelcache.",
                    kPrelinkTextSegment, kPrelinkTextSection);
                goto finish;
            }

            if (ISMACHO64(MAGIC32(toolArgs->kernelcacheImageBytes))) {
                kextOffset = ((struct section_64 *)section)->offset + (u_long)(kextAddr - ((struct section_64 *)section)->addr);
            } else {
                kextOffset = ((struct section *)section)->offset + (u_long)(kextAddr - ((struct section *)section)->addr);
            }
            kextMachO = toolArgs->kernelcacheImageBytes + kextOffset;

            /* Find the requested section's file offset and size */

            if (ISMACHO64(MAGIC32(toolArgs->kernelcacheImageBytes))) {
                section = (void *)macho_get_section_by_name_64(
                    (struct mach_header_64 *)kextMachO,
                    toolArgs->segmentName, toolArgs->sectionName);

            } else {
               /* macho_get_section_by_name doesn't work as the kexts don't have a __TEXT segment.
                * They just have a single segment named "" with all the sections dumped under it.
                */
                section = (void *)getSectionByName(
                    kextMachO,
                    toolArgs->segmentName, toolArgs->sectionName);
            }

            if (!section) {
                OSKextLogCFString(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    CFSTR("Cannot find %s,%s in kext %@\n"),
                    toolArgs->segmentName, toolArgs->sectionName, toolArgs->kextID);
                goto finish;
            }

            if (ISMACHO64(MAGIC32(toolArgs->kernelcacheImageBytes))) {
                printf("%#llx %#lx %#llx\n",
                    ((struct section_64 *)section)->addr,
                    kextOffset + ((struct section_64 *)section)->offset,
                    ((struct section_64 *)section)->size);
            } else {
                printf("%#x %#lx %#x\n",
                    ((struct section *)section)->addr,
                    kextOffset + ((struct section *)section)->offset,
                    ((struct section *)section)->size);
            }

            result = EX_OK;
            break;
        }
    }

finish:
    return result;
}

/*******************************************************************************
*******************************************************************************/
Boolean getKextAddressAndSize(CFDictionaryRef infoDict, uint64_t *addr, uint64_t *size)
{
    Boolean     result     = FALSE;
    CFNumberRef scratchNum = NULL;  // do not release

    scratchNum = CFDictionaryGetValue(infoDict, CFSTR(kPrelinkExecutableSourceKey));
    if (!scratchNum) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Cannot find kext load address");
        goto finish;
    }

    if (!CFNumberGetValue(scratchNum, kCFNumberSInt64Type, addr)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Cannot convert kext load address");
        goto finish;
    }

    scratchNum = CFDictionaryGetValue(infoDict, CFSTR(kPrelinkExecutableSizeKey));
    if (!scratchNum) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Cannot find kext size\n");
        goto finish;
    }

    if (!CFNumberGetValue(scratchNum, kCFNumberSInt64Type, size)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Cannot convert kext size\n");
        goto finish;
    }

    result = TRUE;

finish:
    return result;
}

/*******************************************************************************
* usage()
*******************************************************************************/
void usage(UsageLevel usageLevel)
{
    fprintf(stderr,
      "usage: %1$s [-arch archname] [--] kernelcache bundle-id segment section\n"
      "usage: %1$s -help\n"
      "\n",
      progname);

    if (usageLevel == kUsageLevelBrief) {
        fprintf(stderr, "use %s -%s for an explanation of each option\n",
            progname, kOptNameHelp);
    }

    if (usageLevel == kUsageLevelBrief) {
        return;
    }

    fprintf(stderr, "-%s <archname>:\n"
        "        list info for architecture <archname>\n",
        kOptNameArch);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c): print this message and exit\n",
        kOptNameHelp, kOptHelp);

    return;
}
