/*
 * Copyright (c) 2006, 2012 Apple Computer, Inc. All rights reserved.
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
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBundlePriv.h>
#include <errno.h>
#include <libc.h>
#include <libgen.h>     // dirname()
#include <sys/types.h>
#include <sys/mman.h>
#include <fts.h>
#include <paths.h>
#include <mach-o/arch.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/swap.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mach_port.h>     // mach_port_allocate()
#include <mach/mach_types.h>
#include <mach/machine/vm_param.h>
#include <mach/kmod.h>
#include <notify.h>
#include <servers/bootstrap.h>  // bootstrap mach ports
#include <stdlib.h>
#include <unistd.h>             // sleep(3)
#include <sys/types.h>
#include <sys/stat.h>

#include <DiskArbitration/DiskArbitrationPrivate.h>
#include <IOKit/IOTypes.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitServer.h>
#include <IOKit/IOCFUnserialize.h>
#include <IOKit/IOCFSerialize.h>
#include <libkern/OSByteOrder.h>

#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>
#include <IOKit/kext/kextmanager_types.h>
#include <IOKit/kext/kextmanager_mig.h>

#if __has_include(<prelink.h>)
/* take prelink.h from host side tools SDK */
#include <prelink.h>
#else
#include <System/libkern/prelink.h>
#endif
#include <IOKit/pwr_mgt/IOPMLib.h>

#include "kcgen_main.h"
#include "compression.h"

/*******************************************************************************
* Program Globals
*******************************************************************************/
const char * progname = "(unknown)";

/*******************************************************************************
*******************************************************************************/
int main(int argc, char * const * argv)
{
    ExitStatus      result    = EX_SOFTWARE;
    KcgenArgs       toolArgs;
    Boolean         fatal     = false;

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

   /* Make the library not sort opened kexts by version for bundle ID lookups.
    */
    _OSKextSetStrictRecordingByLastOpened(TRUE);

   /*****
    * Check if we were spawned by kextd, set up straightaway
    * for service log filtering, and hook up to ASL.
    */
    if (getenv("KEXTD_SPAWNED")) {
        OSKextSetLogFilter(kDefaultServiceLogFilter | kOSKextLogKextOrGlobalMask,
            /* kernel? */ false);
        OSKextSetLogFilter(kDefaultServiceLogFilter | kOSKextLogKextOrGlobalMask,
            /* kernel? */ true);
        tool_openlog("com.apple.kcgen");
    }

   /*****
    * Process args & check for permission to load.
    */
    result = readArgs(&argc, &argv, &toolArgs);
    if (result != EX_OK) {
        if (result == kKcgenExitHelp) {
            result = EX_OK;
        }
        goto finish;
    }

    result = checkArgs(&toolArgs);
    if (result != EX_OK) {
        goto finish;
    }

   /* From here on out the default exit status is ok.
    */
    result = EX_OK;

   /* The whole point of this program is to update caches, so let's not
    * try to read any (we'll briefly turn this back on when checking them).
    */
    OSKextSetUsesCaches(false);

   /* If we're compressing the prelinked kernel, take care of that here
    * and exit.
    */
    if (toolArgs.prelinkedKernelPath && !CFArrayGetCount(toolArgs.argURLs) &&
        (toolArgs.compress || toolArgs.uncompress))
    {
        result = compressPrelinkedKernel(toolArgs.prelinkedKernelPath,
                                         toolArgs.compress,
                                         toolArgs.compressionType);
        goto finish;
    }

   /*****
    * Read the kexts we'll be working with; first the set of all kexts, then
    * the repository and named kexts for use with prelink kernel creation flags.
    */
    if (toolArgs.printTestResults) {
        OSKextSetRecordsDiagnostics(kOSKextDiagnosticsFlagAll);
    }
    toolArgs.allKexts = OSKextCreateKextsFromURLs(kCFAllocatorDefault, toolArgs.argURLs);
    if (!toolArgs.allKexts || !CFArrayGetCount(toolArgs.allKexts)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Error - no kernel extensions found.");
        result = EX_SOFTWARE;
        goto finish;
    }

    toolArgs.repositoryKexts = OSKextCreateKextsFromURLs(kCFAllocatorDefault,
        toolArgs.repositoryURLs);
    toolArgs.namedKexts = OSKextCreateKextsFromURLs(kCFAllocatorDefault,
        toolArgs.namedKextURLs);
    if (!toolArgs.repositoryKexts || !toolArgs.namedKexts) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Error - failed to read extensions.");
        result = EX_SOFTWARE;
        goto finish;
    }

    if (result != EX_OK) {
        goto finish;
    }

    if (toolArgs.prelinkedKernelPath) {
        result = createPrelinkedKernel(&toolArgs);
        if (result != EX_OK) {
            goto finish;
        }
    }
    if (toolArgs.plistsPath || toolArgs.loadListPath) {
        result = loadList(&toolArgs);
    }

finish:

   /* We're actually not going to free anything else because we're exiting!
    */
    exit(result);

    SAFE_RELEASE(toolArgs.kextIDs);
    SAFE_RELEASE(toolArgs.argURLs);
    SAFE_RELEASE(toolArgs.repositoryURLs);
    SAFE_RELEASE(toolArgs.namedKextURLs);
    SAFE_RELEASE(toolArgs.allKexts);
    SAFE_RELEASE(toolArgs.repositoryKexts);
    SAFE_RELEASE(toolArgs.namedKexts);
    SAFE_RELEASE(toolArgs.kernelFile);
    SAFE_RELEASE(toolArgs.symbolDirURL);
    SAFE_FREE(toolArgs.prelinkedKernelPath);
    SAFE_FREE(toolArgs.kernelPath);

    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus readArgs(
    int            * argc,
    char * const  ** argv,
    KcgenArgs  * toolArgs)
{
    ExitStatus   result         = EX_USAGE;
    ExitStatus   scratchResult  = EX_USAGE;
    CFStringRef  scratchString  = NULL;  // must release
    CFNumberRef  scratchNumber  = NULL;  // must release
    CFURLRef     scratchURL     = NULL;  // must release
    size_t       len            = 0;
    int32_t      i              = 0;
    int          optchar        = 0;
    int          longindex      = -1;

    bzero(toolArgs, sizeof(*toolArgs));

   /*****
    * Allocate collection objects.
    */
    if (!createCFMutableSet(&toolArgs->kextIDs, &kCFTypeSetCallBacks)             ||
        !createCFMutableSet(&toolArgs->optionalKextIDs, &kCFTypeSetCallBacks)     ||
        !createCFMutableArray(&toolArgs->argURLs, &kCFTypeArrayCallBacks)         ||
        !createCFMutableArray(&toolArgs->repositoryURLs, &kCFTypeArrayCallBacks)  ||
        !createCFMutableArray(&toolArgs->namedKextURLs, &kCFTypeArrayCallBacks)   ||
        !createCFMutableArray(&toolArgs->targetArchs, NULL)) {

        OSKextLogMemError();
        result = EX_OSERR;
        exit(result);
    }

    /*****
    * Process command line arguments.
    */
    while ((optchar = getopt_long_only(*argc, *argv,
        kOptChars, sOptInfo, &longindex)) != -1) {

        SAFE_RELEASE_NULL(scratchString);
        SAFE_RELEASE_NULL(scratchNumber);
        SAFE_RELEASE_NULL(scratchURL);

        /* When processing short (single-char) options, there is no way to
         * express optional arguments.  Instead, we suppress missing option
         * argument errors by adding a leading ':' to the option string.
         * When getopt detects a missing argument, it will return a ':' so that
         * we can screen for options that are not required to have an argument.
         */
        if (optchar == ':') {
            switch (optopt) {
                case kOptPrelinkedKernel:
                    optchar = optopt;
                    break;
                default:
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                        "Error - option requires an argument -- -%c.",
                        optopt);
                    break;
            }
        }

        switch (optchar) {

            case kOptArch:
                if (!addArchForName(toolArgs, optarg)) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                        "Error - unknown architecture %s.", optarg);
                    goto finish;
                }
                break;

            case kOptBundleIdentifier:
                scratchString = CFStringCreateWithCString(kCFAllocatorDefault,
                   optarg, kCFStringEncodingUTF8);
                if (!scratchString) {
                    OSKextLogMemError();
                    result = EX_OSERR;
                    goto finish;
                }
                CFSetAddValue(toolArgs->kextIDs, scratchString);
                break;

            case kOptPrelinkedKernel:
                scratchResult = readPrelinkedKernelArgs(toolArgs, *argc, *argv,
                    /* isLongopt */ longindex != -1);
                if (scratchResult != EX_OK) {
                    result = scratchResult;
                    goto finish;
                }
                break;

            case kOptHelp:
                usage(kUsageLevelFull);
                result = kKcgenExitHelp;
                goto finish;

            case kOptKernel:
                if (toolArgs->kernelPath) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                        "Warning - kernel file already specified; using last.");
                } else {
                    toolArgs->kernelPath = malloc(PATH_MAX);
                    if (!toolArgs->kernelPath) {
                        OSKextLogMemError();
                        result = EX_OSERR;
                        goto finish;
                    }
                }

                len = strlcpy(toolArgs->kernelPath, optarg, PATH_MAX);
                if (len >= PATH_MAX) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                        "Error - kernel filename length exceeds PATH_MAX");
                    goto finish;
                }
                break;

            case kOptTests:
                toolArgs->printTestResults = true;
                break;

            case kOptTargetOverride:
                toolArgs->targetForKextVariants = optarg;
                break;

            case kOptQuiet:
                beQuiet();
                break;

            case kOptVerbose:
                scratchResult = setLogFilterForOpt(*argc, *argv,
                    /* forceOnFlags */ kOSKextLogKextOrGlobalMask);
                if (scratchResult != EX_OK) {
                    result = scratchResult;
                    goto finish;
                }
                break;

            case kOptNoAuthentication:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                    "Note: -%s is implicitly set for %s.", kOptNameNoAuthentication, progname);
                break;

            case 0:
                switch (longopt) {
                    case kLongOptOptionalBundleIdentifier:
                        scratchString = CFStringCreateWithCString(kCFAllocatorDefault,
                           optarg, kCFStringEncodingUTF8);
                        if (!scratchString) {
                            OSKextLogMemError();
                            result = EX_OSERR;
                            goto finish;
                        }
                        CFSetAddValue(toolArgs->optionalKextIDs, scratchString);
                        break;

                    case kLongOptCompressed:
                        toolArgs->compress = true;
                        if (optarg == NULL) {
                            toolArgs->compressionType = COMP_TYPE_LZSS;
                        } else if (0 == strcasecmp(optarg, "lzvn")) {
                            if (!supportsFastLibCompression()) {
                                OSKextLog(/* kext */ NULL,
                                          kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                                          "Error - lzvn compression specified but not supported");
                                goto finish;
                            }
                            toolArgs->compressionType = COMP_TYPE_FASTLIB;
                        } else if (0 == strcasecmp(optarg, "lzss")) {
                            toolArgs->compressionType = COMP_TYPE_LZSS;
                        } else {
                            OSKextLog(/* kext */ NULL,
                                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                                      "Error - unrecognized compression type %s", optarg);
                            goto finish;
                        }
                        break;

                    case kLongOptUncompressed:
                        toolArgs->uncompress = true;
                        break;

                    case kLongOptSymbols:
                        if (toolArgs->symbolDirURL) {
                            OSKextLog(/* kext */ NULL,
                                kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                                "Warning - symbol directory already specified; using last.");
                            SAFE_RELEASE_NULL(toolArgs->symbolDirURL);
                        }

                        scratchURL = CFURLCreateFromFileSystemRepresentation(
                            kCFAllocatorDefault,
                            (const UInt8 *)optarg, strlen(optarg), true);
                        if (!scratchURL) {
                            OSKextLogStringError(/* kext */ NULL);
                            result = EX_OSERR;
                            goto finish;
                        }

                        toolArgs->symbolDirURL = CFRetain(scratchURL);
                        toolArgs->generatePrelinkedSymbols = true;
                        break;

                    case kLongOptVolumeRoot:
                        if (toolArgs->volumeRootURL) {
                            OSKextLog(/* kext */ NULL,
                                kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                                "Warning: volume root already specified; using last.");
                            SAFE_RELEASE_NULL(toolArgs->volumeRootURL);
                        }

                        scratchURL = CFURLCreateFromFileSystemRepresentation(
                            kCFAllocatorDefault,
                            (const UInt8 *)optarg, strlen(optarg), true);
                        if (!scratchURL) {
                            OSKextLogStringError(/* kext */ NULL);
                            result = EX_OSERR;
                            goto finish;
                        }

                        toolArgs->volumeRootURL = CFRetain(scratchURL);
                        break;

                    case kLongOptPLists:
                        toolArgs->plistsPath = optarg;
                        break;

                    case kLongOptLoadList:
                        toolArgs->loadListPath = optarg;
                        break;

                    case kLongOptAllPersonalities:
                        OSKextLog(/* kext */ NULL,
                            kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                            "Note: -%s is implicitly set for %s.", kOptNameAllPersonalities, progname);
                        break;

                    case kLongOptNoLinkFailures:
                        OSKextLog(/* kext */ NULL,
                            kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                            "Note: -%s is implicitly set for %s.", kOptNameNoLinkFailures, progname);
                        break;

                    case kLongOptStripSymbols:
                        toolArgs->stripSymbols = true;
                        break;

                    case kLongOptMaxSliceSize:
                        toolArgs->maxSliceSize = atol(optarg);
                        break;

                    default:
                       /* Because we use ':', getopt_long doesn't print an error message.
                        */
                        OSKextLog(/* kext */ NULL,
                            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                            "Error - unrecognized option %s", (*argv)[optind-1]);
                        goto finish;
                        break;
                }
                break;

            default:
               /* Because we use ':', getopt_long doesn't print an error message.
                */
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Error - unrecognized option %s", (*argv)[optind-1]);
                goto finish;
                break;

        }

       /* Reset longindex, because getopt_long_only() is stupid and doesn't.
        */
        longindex = -1;
    }

   /* Update the argc & argv seen by main().
    */
    *argc -= optind;
    *argv += optind;

   /*
    * Record the kext & directory names from the command line.
    */
    for (i = 0; i < *argc; i++) {
        SAFE_RELEASE_NULL(scratchURL);
        SAFE_RELEASE_NULL(scratchString);

        scratchURL = CFURLCreateFromFileSystemRepresentation(
            kCFAllocatorDefault,
            (const UInt8 *)(*argv)[i], strlen((*argv)[i]), true);
        if (!scratchURL) {
            OSKextLogMemError();
            result = EX_OSERR;
            goto finish;
        }
        CFArrayAppendValue(toolArgs->argURLs, scratchURL);

        scratchString = CFURLCopyPathExtension(scratchURL);
        if (scratchString && CFEqual(scratchString, CFSTR("kext"))) {
            CFArrayAppendValue(toolArgs->namedKextURLs, scratchURL);
        } else {
            CFArrayAppendValue(toolArgs->repositoryURLs, scratchURL);
        }
    }

    result = EX_OK;

finish:
    SAFE_RELEASE(scratchString);
    SAFE_RELEASE(scratchNumber);
    SAFE_RELEASE(scratchURL);

    if (result == EX_USAGE) {
        usage(kUsageLevelBrief);
    }
    return result;
}

/*******************************************************************************
*******************************************************************************/
void logUsedKexts(
    KcgenArgs       * toolArgs,
    CFArrayRef        prelinkKexts)
{
    int               fd;
    int               ret;
    char            * kextTracePath;
    char            * outputName;
    char              tmpBuffer[PATH_MAX + 1 + 1];
    size_t            tmpBufLen;
    ssize_t           writeSize;
    CFIndex           count, i;

    /* Log used kext bundle identifiers to out-of-band log file */
    kextTracePath = getenv("KEXT_TRACE_FILE");
    if (!kextTracePath) {
        return;
    }

    fd = open(kextTracePath, O_WRONLY|O_APPEND|O_CREAT, DEFFILEMODE);
    if (fd < 0) {
        return;
    }

    outputName = toolArgs->prelinkedKernelPath;
    if (!outputName) {
        outputName = toolArgs->loadListPath;
    }
    snprintf(tmpBuffer, sizeof(tmpBuffer), "%s\n", outputName);
    tmpBufLen = strlen(tmpBuffer);
    writeSize = write(fd, tmpBuffer, tmpBufLen);
    if (writeSize != (ssize_t)tmpBufLen) {
        close(fd);
        return;
    }

    count = CFArrayGetCount(prelinkKexts);
    for (i = 0; i < count; i++) {
        CFStringRef kextID;
        OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(prelinkKexts, i);

        kextID = OSKextGetIdentifier(theKext);
        if (kextID) {
            char kextIDCString[KMOD_MAX_NAME];

            CFStringGetCString(kextID, kextIDCString, sizeof(kextIDCString),
                               kCFStringEncodingUTF8);

            snprintf(tmpBuffer, sizeof(tmpBuffer), "[Logging for XBS] Used kext bundle identifier: %s\n", kextIDCString);
            tmpBufLen = strlen(tmpBuffer);
            writeSize = write(fd, tmpBuffer, tmpBufLen);
            if (writeSize != (ssize_t)tmpBufLen) {
                close(fd);
                return;
            }
        }
    }

    close(fd);
}

/*******************************************************************************
*******************************************************************************/
ExitStatus readPrelinkedKernelArgs(
    KcgenArgs * toolArgs,
    int             argc,
    char * const  * argv,
    Boolean         isLongopt)
{
    char * filename = NULL;  // do not free

    if (optarg) {
        filename = optarg;
    } else if (isLongopt && optind < argc) {
        filename = argv[optind];
        optind++;
    }

    if (filename && !filename[0]) {
        filename = NULL;
    }

    return setPrelinkedKernelArgs(toolArgs, filename);
}

/*******************************************************************************
*******************************************************************************/
ExitStatus setPrelinkedKernelArgs(
    KcgenArgs * toolArgs,
    char      * filename)
{
    ExitStatus          result          = EX_USAGE;

    if (toolArgs->prelinkedKernelPath) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
            "Warning - prelinked kernel already specified; using last.");
    } else {
        toolArgs->prelinkedKernelPath = malloc(PATH_MAX);
        if (!toolArgs->prelinkedKernelPath) {
            OSKextLogMemError();
            result = EX_OSERR;
            goto finish;
        }
    }

   /* If we don't have a filename we construct a default one, automatically
    * add the system extensions folders, and note that we're using default
    * info.
    */
    if (!filename) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Error - prelinked kernel filename required");
        goto finish;
    } else {
        size_t len = strlcpy(toolArgs->prelinkedKernelPath, filename, PATH_MAX);
        if (len >= PATH_MAX) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Error - prelinked kernel filename length exceeds PATH_MAX");
            goto finish;
        }
    }

    result = EX_OK;
finish:
    return result;
}

/*******************************************************************************
********************************************************************************/
void addArch(
    KcgenArgs * toolArgs,
    const NXArchInfo  * arch)
{
    if (CFArrayContainsValue(toolArgs->targetArchs,
        RANGE_ALL(toolArgs->targetArchs), arch))
    {
        return;
    }

    CFArrayAppendValue(toolArgs->targetArchs, arch);
}

/*******************************************************************************
*******************************************************************************/
const NXArchInfo * addArchForName(
    KcgenArgs     * toolArgs,
    const char    * archname)
{
    const NXArchInfo * result = NULL;

    result = NXGetArchInfoFromName(archname);
    if (!result) {
        goto finish;
    }

    addArch(toolArgs, result);

finish:
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus checkArgs(KcgenArgs * toolArgs)
{
    ExitStatus  result  = EX_USAGE;

    if (!toolArgs->prelinkedKernelPath && !toolArgs->loadListPath && !toolArgs->plistsPath)
    {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Error - no work to do; check options and try again.");
        goto finish;
    }

    if (!CFArrayGetCount(toolArgs->argURLs) &&
        !toolArgs->compress && !toolArgs->uncompress)
    {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Error - no kexts or directories specified.");
        goto finish;
    }

    if (!toolArgs->compress && !toolArgs->uncompress) {
        toolArgs->compress = true;
        toolArgs->compressionType = COMP_TYPE_LZSS;
    } else if (toolArgs->compress && toolArgs->uncompress) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Both -%s and -%s specified; using -%s.",
            kOptNameCompressed, kOptNameUncompressed, kOptNameCompressed);
        toolArgs->compress = true;
        toolArgs->uncompress = false;
    }

    result = EX_OK;

finish:
    if (result == EX_USAGE) {
        usage(kUsageLevelBrief);
    }
    return result;
}

/*******************************************************************************
*******************************************************************************/
typedef struct {
    KcgenArgs         * toolArgs;
    CFMutableArrayRef   kextArray;
    const NXArchInfo  * arch;
    Boolean             optional;
    Boolean             error;
} FilterIDContext;

void filterKextID(const void * vValue, void * vContext)
{
    CFStringRef       kextID  = (CFStringRef)vValue;
    FilterIDContext * context = (FilterIDContext *)vContext;
    OSKextRef         theKext = OSKextGetKextWithIdentifier(kextID);

    if (!theKext) {
        char kextIDCString[KMOD_MAX_NAME];

        CFStringGetCString(kextID, kextIDCString, sizeof(kextIDCString),
            kCFStringEncodingUTF8);

        if (context->optional) {
            OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Can't find kext with optional identifier %s; skipping.", kextIDCString);
        } else {
            OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Error - can't find kext with identifier %s.", kextIDCString);
            context->error = TRUE;
        }

        goto finish;
    }

    if (checkKextForProblems(context->toolArgs, theKext, context->arch)) {
        if (!context->optional) {
            OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Error - a required kext was omitted");
            context->error = true;
        }
        goto finish;
    }

    if (!CFArrayContainsValue(context->kextArray,
            RANGE_ALL(context->kextArray), theKext))
    {
        CFArrayAppendValue(context->kextArray, theKext);
    }

finish:
    return;
}

/*******************************************************************************
 *******************************************************************************/
ExitStatus checkKextForProblems(
        KcgenArgs         * toolArgs,
        OSKextRef           theKext,
        const NXArchInfo  * arch)
{
    ExitStatus          result        = EX_SOFTWARE;
    char                kextPath[PATH_MAX];
    CFURLRef            moduleURL, checkURL;
    boolean_t           present;

    if (!CFURLGetFileSystemRepresentation(OSKextGetURL(theKext),
                              /* resolveToBase */ false, (UInt8 *)kextPath, sizeof(kextPath)))
    {
        strlcpy(kextPath, "(unknown)", sizeof(kextPath));
    }

    /* Skip kexts we have no interest in for the current arch.
     */
    if (!OSKextSupportsArchitecture(theKext, arch)) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
                  "%s doesn't support architecture '%s'; ommiting.", kextPath,
                  arch->name);
        goto finish;
    }

    if (!OSKextIsValid(theKext)) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                  kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                  "%s is not valid; omitting.", kextPath);
        if (toolArgs->printTestResults) {
            OSKextLogDiagnostics(theKext, kOSKextDiagnosticsFlagAll);
        }
        goto finish;
    }

    if (!OSKextResolveDependencies(theKext)) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogWarningLevel | kOSKextLogArchiveFlag |
                  kOSKextLogDependenciesFlag | kOSKextLogGeneralFlag,
                  "%s is missing dependencies (including anyway; "
                  "dependencies may be available from elsewhere)", kextPath);
        if (toolArgs->printTestResults) {
            OSKextLogDiagnostics(theKext, kOSKextDiagnosticsFlagAll);
        }
    }

    result = EX_OK;

finish:
    return result;

}

/*******************************************************************************
*******************************************************************************/
ExitStatus filterKextsForCache(
        KcgenArgs         * toolArgs,
        CFMutableArrayRef   kextArray,
        const NXArchInfo  * arch,
        Boolean           * fatalOut __unused)
{
    ExitStatus          result        = EX_SOFTWARE;
    CFIndex             count, i;

    CFArrayRemoveAllValues(kextArray);

   /*****
    * Apply filters to select the kexts.
    *
    * If kexts have been specified by identifier, those are the only kexts we are going to use.
    * Otherwise run through the repository and named kexts and see which ones match the filter.
    */
    if (CFSetGetCount(toolArgs->kextIDs) || CFSetGetCount(toolArgs->optionalKextIDs)) {
        FilterIDContext context;

        context.toolArgs = toolArgs;
        context.kextArray = kextArray;
        context.arch = arch;
        context.optional = FALSE;
        context.error = FALSE;

        CFSetApplyFunction(toolArgs->kextIDs, filterKextID, &context);

        context.optional = TRUE;
        CFSetApplyFunction(toolArgs->optionalKextIDs, filterKextID, &context);

        if (context.error) {
            goto finish;
        }

    } else {

        count = CFArrayGetCount(toolArgs->repositoryKexts);
        for (i = 0; i < count; i++) {
            char kextPath[PATH_MAX];
            OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(
                    toolArgs->repositoryKexts, i);

            if (!CFArrayContainsValue(kextArray, RANGE_ALL(kextArray), theKext) &&
                !checkKextForProblems(toolArgs, theKext, arch)) {
                CFArrayAppendValue(kextArray, theKext);
            }
        }

        count = CFArrayGetCount(toolArgs->namedKexts);
        for (i = 0; i < count; i++) {
            OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(
                    toolArgs->namedKexts, i);
            if (!CFArrayContainsValue(kextArray, RANGE_ALL(kextArray), theKext) &&
                !checkKextForProblems(toolArgs, theKext, arch)) {
                CFArrayAppendValue(kextArray, theKext);
            }
        }
    }

    result = EX_OK;

finish:
    return result;
}

/*******************************************************************************
 * Creates a list of architectures to generate prelinked kernel slices for by
 * selecting the requested architectures for which the kernel has a slice.
 * Warns when a requested architecture does not have a corresponding kernel
 * slice.
 *******************************************************************************/
ExitStatus
createPrelinkedKernelArchs(
    KcgenArgs         * toolArgs,
    CFMutableArrayRef * prelinkArchsOut)
{
    ExitStatus          result          = EX_OSERR;
    CFMutableArrayRef   kernelArchs     = NULL;  // must release
    CFMutableArrayRef   prelinkArchs    = NULL;  // must release
    const NXArchInfo  * targetArch      = NULL;  // do not free
    int                 i               = 0;

    result = readFatFileArchsWithPath(toolArgs->kernelPath, &kernelArchs);
    if (result != EX_OK) {
        goto finish;
    }

    prelinkArchs = CFArrayCreateMutableCopy(kCFAllocatorDefault,
        /* capacity */ 0, toolArgs->targetArchs);
    if (!prelinkArchs) {
        OSKextLogMemError();
        result = EX_OSERR;
        goto finish;
    }

    for (i = 0; i < CFArrayGetCount(prelinkArchs); ++i) {
        targetArch = CFArrayGetValueAtIndex(prelinkArchs, i);
        if (!CFArrayContainsValue(kernelArchs,
            RANGE_ALL(kernelArchs), targetArch))
        {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogWarningLevel | kOSKextLogArchiveFlag,
                "Warning - kernel file %s does not contain requested arch: %s",
                toolArgs->kernelPath, targetArch->name);
            CFArrayRemoveValueAtIndex(prelinkArchs, i);
            i--;
            continue;
        }
    }

    *prelinkArchsOut = (CFMutableArrayRef) CFRetain(prelinkArchs);
    result = EX_OK;

finish:
    SAFE_RELEASE(kernelArchs);
    SAFE_RELEASE(prelinkArchs);

    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus
createPrelinkedKernel(
    KcgenArgs     * toolArgs)
{
    ExitStatus          result              = EX_OSERR;
    struct timeval      prelinkFileTimes[2];
    CFMutableArrayRef   generatedArchs      = NULL;  // must release
    CFMutableArrayRef   generatedSymbols    = NULL;  // must release
    CFMutableArrayRef   existingArchs       = NULL;  // must release
    CFMutableArrayRef   existingSlices      = NULL;  // must release
    CFMutableArrayRef   prelinkArchs        = NULL;  // must release
    CFMutableArrayRef   prelinkSlices       = NULL;  // must release
    CFDataRef           prelinkSlice        = NULL;  // must release
    CFDictionaryRef     sliceSymbols        = NULL;  // must release
    const NXArchInfo  * targetArch          = NULL;  // do not free
    Boolean             updateModTime       = false;
    u_int               numArchs            = 0;
    u_int               i                   = 0;
    int                 j                   = 0;

    bzero(prelinkFileTimes, sizeof(prelinkFileTimes));

    result = createPrelinkedKernelArchs(toolArgs, &prelinkArchs);
    if (result != EX_OK) {
        goto finish;
    }
    numArchs = (u_int)CFArrayGetCount(prelinkArchs);

    prelinkSlices = CFArrayCreateMutable(kCFAllocatorDefault,
        numArchs, &kCFTypeArrayCallBacks);
    generatedSymbols = CFArrayCreateMutable(kCFAllocatorDefault,
        numArchs, &kCFTypeArrayCallBacks);
    generatedArchs = CFArrayCreateMutable(kCFAllocatorDefault,
        numArchs, NULL);
    if (!prelinkSlices || !generatedSymbols || !generatedArchs) {
        OSKextLogMemError();
        result = EX_OSERR;
        goto finish;
    }

    for (i = 0; i < numArchs; i++) {
        targetArch = CFArrayGetValueAtIndex(prelinkArchs, i);

        SAFE_RELEASE_NULL(prelinkSlice);
        SAFE_RELEASE_NULL(sliceSymbols);

       /* We always create a new prelinked kernel for the current
        * running architecture if asked, but we'll reuse existing slices
        * for other architectures if possible.
        */
        if (existingArchs &&
            targetArch != OSKextGetRunningKernelArchitecture())
        {
            j = (int)CFArrayGetFirstIndexOfValue(existingArchs,
                RANGE_ALL(existingArchs), targetArch);
            if (j != -1) {
                prelinkSlice = CFArrayGetValueAtIndex(existingSlices, j);
                CFArrayAppendValue(prelinkSlices, prelinkSlice);
                prelinkSlice = NULL;
                OSKextLog(/* kext */ NULL,
                    kOSKextLogDebugLevel | kOSKextLogArchiveFlag,
                    "Using existing prelinked slice for arch %s",
                    targetArch->name);
                continue;
            }
        }

        OSKextLog(/* kext */ NULL,
                  kOSKextLogDebugLevel | kOSKextLogArchiveFlag,
                  "Generating a new prelinked slice for arch %s",
                  targetArch->name);

        result = createPrelinkedKernelForArch(toolArgs, &prelinkSlice,
            &sliceSymbols, targetArch);
        if (result != EX_OK) {
            goto finish;
        }

        if (toolArgs->maxSliceSize &&
            (CFDataGetLength(prelinkSlice) > toolArgs->maxSliceSize)) {

            result = EX_SOFTWARE;
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Error - prelink slice is larger (%ld) than requested maximum %ld.",
                (long int)CFDataGetLength(prelinkSlice),
                (long int)toolArgs->maxSliceSize);
            goto finish;
        }

        CFArrayAppendValue(prelinkSlices, prelinkSlice);
        CFArrayAppendValue(generatedSymbols, sliceSymbols);
        CFArrayAppendValue(generatedArchs, targetArch);
    }

    result = writeFatFile(toolArgs->prelinkedKernelPath, prelinkSlices,
        prelinkArchs, (0644),  // !!! - need macro for perms
        (updateModTime) ? prelinkFileTimes : NULL);
    if (result != EX_OK) {
        goto finish;
    }

    if (toolArgs->symbolDirURL) {
        result = writePrelinkedSymbols(toolArgs->symbolDirURL,
            generatedSymbols, generatedArchs);
        if (result != EX_OK) {
            goto finish;
        }
    }

    OSKextLog(/* kext */ NULL,
        kOSKextLogBasicLevel | kOSKextLogGeneralFlag | kOSKextLogArchiveFlag,
        "Created prelinked kernel %s.",
        toolArgs->prelinkedKernelPath);

    result = EX_OK;

finish:
    SAFE_RELEASE(generatedArchs);
    SAFE_RELEASE(generatedSymbols);
    SAFE_RELEASE(existingArchs);
    SAFE_RELEASE(existingSlices);
    SAFE_RELEASE(prelinkArchs);
    SAFE_RELEASE(prelinkSlices);
    SAFE_RELEASE(prelinkSlice);
    SAFE_RELEASE(sliceSymbols);

    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus createPrelinkedKernelForArch(
    KcgenArgs           * toolArgs,
    CFDataRef           * prelinkedKernelOut,
    CFDictionaryRef     * prelinkedSymbolsOut,
    const NXArchInfo    * archInfo)
{
    ExitStatus result = EX_OSERR;
    CFMutableArrayRef prelinkKexts = NULL;
    CFDataRef kernelImage = NULL;
    CFDataRef prelinkedKernel = NULL;
    uint32_t flags = 0;
    Boolean fatalOut = false;

    /* Retrieve the kernel image for the requested architecture.
     */
    kernelImage = readMachOSliceForArch(toolArgs->kernelPath, archInfo, /* checkArch */ TRUE);
    if (!kernelImage) {
        OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogArchiveFlag |  kOSKextLogFileAccessFlag,
                "Error - failed to read kernel file.");
        goto finish;
    }

    /* Set suffix for kext executables from kernel path
     */
    OSKextSetExecutableSuffix(NULL, toolArgs->kernelPath);

    /* Set current target if there is one */
    if (toolArgs->targetForKextVariants) {
        OSKextSetTargetString(toolArgs->targetForKextVariants);
    }

    /* Set the architecture in the OSKext library */

    if (!OSKextSetArchitecture(archInfo)) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "Error - can't set architecture %s to create prelinked kernel.",
                      archInfo->name);
            result = EX_OSERR;
            goto finish;
    }

   /*****
    * Figure out which kexts we're actually archiving.
    * This uses toolArgs->allKexts, which must already be created.
    */
    prelinkKexts = CFArrayCreateMutable(kCFAllocatorDefault, 0,
        &kCFTypeArrayCallBacks);
    if (!prelinkKexts) {
        OSKextLogMemError();
        result = EX_OSERR;
        goto finish;
    }

    result = filterKextsForCache(toolArgs, prelinkKexts,
            archInfo, &fatalOut);
    if (result != EX_OK || fatalOut) {
        goto finish;
    }

    result = EX_OSERR;

    if (!CFArrayGetCount(prelinkKexts)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Error - no kexts found for architecture %s.",
            archInfo->name);
        goto finish;
    }

   /* Create the prelinked kernel from the given kernel and kexts */

    flags |= kOSKextKernelcacheKASLRFlag;
    flags |= kOSKextKernelcacheNeedAllFlag;
    flags |= kOSKextKernelcacheSkipAuthenticationFlag;
    flags |= kOSKextKernelcacheIncludeAllPersonalitiesFlag;

    flags |= (toolArgs->stripSymbols) ? kOSKextKernelcacheStripSymbolsFlag : 0;
    flags |= (toolArgs->printTestResults) ? kOSKextKernelcachePrintDiagnosticsFlag : 0;

    prelinkedKernel = OSKextCreatePrelinkedKernel(kernelImage, prelinkKexts,
        toolArgs->volumeRootURL, flags, prelinkedSymbolsOut);
    if (!prelinkedKernel) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Error - failed to generate prelinked kernel.");
        result = EX_OSERR;
        goto finish;
    }

    /* Log used bundle identifiers for B&I */
    logUsedKexts(toolArgs, prelinkKexts);

   /* Compress the prelinked kernel if needed */

    if (toolArgs->compress) {
        *prelinkedKernelOut = compressPrelinkedSlice(toolArgs->compressionType,
                                                     prelinkedKernel,
                                                     true);
    } else {
        *prelinkedKernelOut = CFRetain(prelinkedKernel);
    }

    if (!*prelinkedKernelOut) {
        goto finish;
    }

    result = EX_OK;

finish:
    SAFE_RELEASE(kernelImage);
    SAFE_RELEASE(prelinkKexts);
    SAFE_RELEASE(prelinkedKernel);

    return result;
}


/*********************************************************************
 *********************************************************************/
ExitStatus compressPrelinkedKernel(
                                   const char        * prelinkPath,
                                   Boolean             compress,
                                   uint32_t            compressionType)
{
    ExitStatus          result          = EX_SOFTWARE;
    struct timeval      prelinkedKernelTimes[2];
    CFMutableArrayRef   prelinkedSlices = NULL; // must release
    CFMutableArrayRef   prelinkedArchs  = NULL; // must release
    CFDataRef           prelinkedSlice  = NULL; // must release
    const NXArchInfo  * archInfo        = NULL; // do not free
    const u_char      * sliceBytes      = NULL; // do not free
    mode_t              fileMode        = 0;
    int                 i               = 0;

    result = readMachOSlices(prelinkPath, &prelinkedSlices,
        &prelinkedArchs, &fileMode, prelinkedKernelTimes);
    if (result != EX_OK) {
        goto finish;
    }

    /* Compress/uncompress each slice of the prelinked kernel.
     */

    for (i = 0; i < CFArrayGetCount(prelinkedSlices); ++i) {

        SAFE_RELEASE_NULL(prelinkedSlice);
        prelinkedSlice = CFArrayGetValueAtIndex(prelinkedSlices, i);

        if (compress) {
            prelinkedSlice = compressPrelinkedSlice(compressionType,
                                                    prelinkedSlice,
                                                    true);
            if (!prelinkedSlice) {
                result = EX_DATAERR;
                goto finish;
            }
        } else {
            prelinkedSlice = uncompressPrelinkedSlice(prelinkedSlice);
            if (!prelinkedSlice) {
                result = EX_DATAERR;
                goto finish;
            }
        }

        CFArraySetValueAtIndex(prelinkedSlices, i, prelinkedSlice);
    }
    SAFE_RELEASE_NULL(prelinkedSlice);

    /* Snow Leopard prelinked kernels are not wrapped in a fat header, so we
     * have to decompress the prelinked kernel and look at the mach header
     * to get the architecture information.
     */

    if (!prelinkedArchs && CFArrayGetCount(prelinkedSlices) == 1) {
        if (!createCFMutableArray(&prelinkedArchs, NULL)) {
            OSKextLogMemError();
            result = EX_OSERR;
            goto finish;
        }

        sliceBytes = CFDataGetBytePtr(CFArrayGetValueAtIndex(prelinkedSlices, 0));

        archInfo = getThinHeaderPageArch(sliceBytes);
        if (archInfo) {
            CFArrayAppendValue(prelinkedArchs, archInfo);
        } else {
            SAFE_RELEASE_NULL(prelinkedArchs);
        }
    }

    /* If we still don't have architecture information, then something
     * definitely went wrong.
     */

    if (!prelinkedArchs) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Couldn't determine prelinked kernel's architecture");
        result = EX_SOFTWARE;
        goto finish;
    }

    result = writeFatFile(prelinkPath, prelinkedSlices,
        prelinkedArchs, fileMode, prelinkedKernelTimes);
    if (result != EX_OK) {
        goto finish;
    }

    result = EX_OK;

finish:
    SAFE_RELEASE(prelinkedSlices);
    SAFE_RELEASE(prelinkedArchs);
    SAFE_RELEASE(prelinkedSlice);

    return result;
}

/*********************************************************************
 *********************************************************************/

ExitStatus
loadList(
    KcgenArgs     * toolArgs)
{
    ExitStatus             result           = EX_OSERR;
    CFMutableArrayRef      prelinkArchs     = NULL;  // must release
    const NXArchInfo     * targetArch       = NULL;  // do not free
    CFMutableArrayRef      prelinkKexts     = NULL;  // must release
    CFMutableArrayRef      loadList         = NULL;  // must release
    Boolean                needAllFlag      = TRUE;
    OSKextLogSpec          logLevel         = needAllFlag ? kOSKextLogErrorLevel :
                                                 kOSKextLogWarningLevel;
    CFMutableArrayRef      bundleList       = NULL;
    CFArrayRef             deps             = NULL;
    CFMutableDictionaryRef prelinkDict      = NULL;
    CFMutableDictionaryRef infoDict         = NULL;
    CFStringRef            executable       = NULL;
    CFURLRef               moduleURL        = NULL;
    CFDataRef              data             = NULL;
    CFNumberRef            num              = NULL;
    FILE                 * f                = NULL;
    Boolean                fatalOut         = FALSE;
    CFIndex                idx, kmodIdx, depIdx, count;
    char                   path[PATH_MAX];
    char                   kextPath[PATH_MAX];
    u_int                  numArchs;

    result = createPrelinkedKernelArchs(toolArgs, &prelinkArchs);
    if (result != EX_OK) {
        goto finish;
    }
    numArchs = (u_int)CFArrayGetCount(prelinkArchs);
    if (1 != numArchs) {
        result = EX_USAGE;
        goto finish;
    }

    targetArch = CFArrayGetValueAtIndex(prelinkArchs, 0);

    /* Set suffix for kext executables from kernel path */
    OSKextSetExecutableSuffix(NULL, toolArgs->kernelPath);

    /* Set current target if there is one */
    if (toolArgs->targetForKextVariants) {
        OSKextSetTargetString(toolArgs->targetForKextVariants);
    }

    /* Set the architecture in the OSKext library */
    if (!OSKextSetArchitecture(targetArch)) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "Error - can't set architecture %s to create prelinked kernel.",
                      targetArch->name);
            result = EX_OSERR;
            goto finish;
    }

   /*****
    * Figure out which kexts we're actually archiving.
    * This uses toolArgs->allKexts, which must already be created.
    */
    prelinkKexts = CFArrayCreateMutable(kCFAllocatorDefault, 0,
                                        &kCFTypeArrayCallBacks);
    if (!prelinkKexts) {
        OSKextLogMemError();
        result = EX_OSERR;
        goto finish;
    }

    result = filterKextsForCache(toolArgs, prelinkKexts,
            targetArch, &fatalOut);
    if (result != EX_OK || fatalOut) {
        goto finish;
    }

    result = EX_OSERR;

    if (!CFArrayGetCount(prelinkKexts)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Error - no kexts found for architecture %s.",
            targetArch->name);
        goto finish;
    }
    needAllFlag = TRUE;
    loadList = OSKextCopyLoadListForKexts(prelinkKexts, needAllFlag);
    if (!loadList)
    {
        OSKextLog(/* kext */ NULL, kOSKextLogErrorLevel,
            "Can't resolve dependencies amongst kexts for prelinked kernel.");
        goto finish;
    }

    f = fopen(toolArgs->loadListPath, "w");
    if (!f) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "%s not writable.",
            toolArgs->loadListPath);
        result = EX_OSFILE;
        goto finish;
    }

    prelinkDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                            &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    bundleList  = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

    for (kmodIdx = idx = 0; idx < CFArrayGetCount(loadList); idx++)
    {
        OSKextRef aKext = (OSKextRef) CFArrayGetValueAtIndex(loadList, idx);
        bool ok;

        ok = CFURLGetFileSystemRepresentation(OSKextGetURL(aKext),
                                /* resolveToBase */ TRUE, (UInt8 *)kextPath, PATH_MAX);
        if (!ok){
            OSKextLog(aKext, logLevel | kOSKextLogArchiveFlag, "%s no path.", kextPath);
            result = EX_OSERR;
            goto finish;
        }
        infoDict = OSKextCopyInfoDictionary(aKext);
        if (!infoDict) {
            OSKextLog(aKext, logLevel | kOSKextLogArchiveFlag, "%s no info. ", kextPath);
            result = EX_OSFILE;
            goto finish;
        }

        CFArrayAppendValue(bundleList, infoDict);
        CFRelease(infoDict);

        if (!OSKextIsInterface(aKext) && (moduleURL = OSKextGetKernelExecutableURL(aKext)))
        {
            if (!CFURLGetFileSystemRepresentation(moduleURL, true, (UInt8 *)path, PATH_MAX))
            {
                OSKextLog(aKext, logLevel | kOSKextLogArchiveFlag, "%s no module path. ", kextPath);
                result = EX_OSFILE;
                goto finish;
            }
            // "ModuleIndex"
            num = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &kmodIdx);
            CFDictionarySetValue(infoDict, CFSTR("ModuleIndex"), num);
            CFRelease(num);

            // kPrelinkBundlePathKey, kPrelinkExecutableRelativePathKey
            size_t executablePathLen = strnlen(path, PATH_MAX);
            size_t kextPathLen = strnlen(kextPath, PATH_MAX);
            if ((executablePathLen > kextPathLen) && !strncmp(kextPath, path, kextPathLen))
            {
                CFStringRef
                string = CFStringCreateWithBytes(kCFAllocatorDefault,
                    (UInt8 *)kextPath, kextPathLen,
                    kCFStringEncodingUTF8, false);
                if (string)
                {
                    CFDictionarySetValue(infoDict, CFSTR(kPrelinkBundlePathKey), string);
                    CFRelease(string);
                }
                string = CFStringCreateWithBytes(kCFAllocatorDefault,
                    (UInt8 *)path + kextPathLen + 1, executablePathLen - kextPathLen - 1,
                    kCFStringEncodingUTF8, false);
                if (string)
                {
                    CFDictionarySetValue(infoDict, CFSTR(kPrelinkExecutableRelativePathKey), string);
                    CFRelease(string);
                }
            }

            kmodIdx++;
            fprintf(f, "%s", path);
            deps = OSKextCopyDeclaredDependencies(aKext, TRUE /* needAll */);
            if (deps)
            {
                for (depIdx = 0; depIdx < CFArrayGetCount(deps); depIdx++)
                {
                    OSKextRef depKext = (OSKextRef) CFArrayGetValueAtIndex(deps, depIdx);
                    executable = OSKextCopyExecutableName(depKext);
                    if (executable && CFStringGetCString(executable, path, PATH_MAX, kCFStringEncodingUTF8))
                    {
                        fprintf(f, ":%s", path);
                    }
                    SAFE_RELEASE(executable);
                }
                SAFE_RELEASE(deps);
            }
            fprintf(f, "\n");
        }
    }
    if (ferror(f)) {
        result = EX_OSFILE;
        goto finish;
    }
    fclose(f);
    f = NULL;

    if (toolArgs->plistsPath)
    {
        CFDictionarySetValue(prelinkDict, CFSTR("_PrelinkInfoDictionary"), bundleList);
        data = IOCFSerialize(prelinkDict, kNilOptions);
        if (!data) {
            OSKextLog(NULL,
                logLevel | kOSKextLogArchiveFlag,
                "IOCFSerialize.");
            result = EX_OSERR;
            goto finish;
        }
        f = fopen(toolArgs->plistsPath, "w+");
        if (!f) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
                "%s not writable.",
                toolArgs->plistsPath);
            result = EX_OSFILE;
            goto finish;
        }
        fwrite(CFDataGetBytePtr(data), sizeof(UInt8), CFDataGetLength(data), f);
        fwrite("\0", sizeof(UInt8), 1, f);
    }
    if (ferror(f)) {
        result = EX_OSFILE;
        goto finish;
    }
    result = EX_OK;

    /* Log used bundle identifiers for B&I */
    logUsedKexts(toolArgs, prelinkKexts);

finish:
    if (f) fclose(f);
    SAFE_RELEASE(bundleList);
    SAFE_RELEASE(prelinkDict);
    SAFE_RELEASE(data);
    SAFE_RELEASE(prelinkArchs);
    SAFE_RELEASE(loadList);
    SAFE_RELEASE(prelinkKexts);

    return result;
}

/*******************************************************************************
* usage()
*******************************************************************************/
void usage(UsageLevel usageLevel)
{
    fprintf(stderr,
      "usage: %1$s -prelinked-kernel <filename> [options] [--] [kext or directory]\n"
      "\n",
      progname);

    if (usageLevel == kUsageLevelBrief) {
        fprintf(stderr, "use %s -%s for an explanation of each option\n",
            progname, kOptNameHelp);
    }

    if (usageLevel == kUsageLevelBrief) {
        return;
    }

    fprintf(stderr, "-%s <filename> (-%c):\n"
        "        create/update prelinked kernel\n",
        kOptNamePrelinkedKernel, kOptPrelinkedKernel);

    fprintf(stderr, "-%s <filename>:\n"
        "        create load list of modules and dependencies\n",
        kOptNameLoadList);

    fprintf(stderr, "-%s <filename>:\n"
        "        create serialized property lists\n",
        kOptNamePLists);

    fprintf(stderr, "\n");

    fprintf(stderr,
        "kext or directory: Consider kext or all kexts in directory for inclusion\n");
    fprintf(stderr, "-%s <bundle_id> (-%c):\n"
        "        include the kext whose CFBundleIdentifier is <bundle_id>\n",
        kOptNameBundleIdentifier, kOptBundleIdentifier);
    fprintf(stderr, "-%s <bundle_id>:\n"
        "        include the kext whose CFBundleIdentifier is <bundle_id> if it exists\n",
        kOptNameOptionalBundleIdentifier);
    fprintf(stderr, "-%s <kernel_filename> (-%c): Use kernel_filename for a prelinked kernel\n",
        kOptNameKernel, kOptKernel);

    fprintf(stderr, "-%s <archname>:\n"
        "        include architecture <archname> in created cache(s)\n",
        kOptNameArch);
    fprintf(stderr, "-%s <volume>:\n"
        "        Save kext paths in the prelinked kernel relative to <volume>\n",
        kOptNameVolumeRoot);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c): quiet mode: print no informational or error messages\n",
        kOptNameQuiet, kOptQuiet);
    fprintf(stderr, "-%s [ 0-6 | 0x<flags> ] (-%c):\n"
        "        verbose mode; print info about analysis & loading\n",
        kOptNameVerbose, kOptVerbose);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c):\n"
        "        print diagnostics for kexts with problems\n",
        kOptNameTests, kOptTests);

    fprintf(stderr, "-%s (-%c): print this message and exit\n",
        kOptNameHelp, kOptHelp);

    return;
}
