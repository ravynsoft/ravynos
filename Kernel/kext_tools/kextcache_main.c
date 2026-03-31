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
#include <mach/mach_types.h>
#include <mach/machine/vm_param.h>
#include <mach/kmod.h>
#include <notify.h>
#include <stdlib.h>
#include <unistd.h>             // sleep(3)
#include <sys/types.h>
#include <sys/stat.h>
#include <Security/SecKeychainPriv.h>
#include <sandbox/rootless.h>
#include <sys/csr.h>
#include <sys/sysctl.h>

#include <DiskArbitration/DiskArbitrationPrivate.h>
#include <IOKit/IOTypes.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitServer.h>
#include <IOKit/IOCFUnserialize.h>
#include <IOKit/IOCFSerialize.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <libkern/OSByteOrder.h>

#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>
#include <IOKit/kext/macho_util.h>
#include <bootfiles.h>

#include <IOKit/pwr_mgt/IOPMLib.h>

#include "kextcache_main.h"
#include "kext_tools_util.h"
#include "fork_program.h"
#if !NO_BOOT_ROOT
#include "bootcaches.h"
#include "bootroot_internal.h"
#endif /* !NO_BOOT_ROOT */
//#include "mkext1_file.h"
#include "compression.h"
#include "security.h"
#include "signposts.h"
#include "staging.h"
#include "syspolicy.h"
#include "driverkit.h"
#include "rosp_staging.h"

#if __has_include(<prelink.h>)
/* take prelink.h from host side tools SDK */
#include <prelink.h>
#else
#include <System/libkern/prelink.h>
#endif

// constants
#define PRELINK_KERNEL_PERMS             (0644)

/* The timeout we use when waiting for the system to get to a low load state.
 * We're shooting for about 10 minutes, but we don't want to collide with
 * everyone else who wants to do work 10 minutes after boot, so we just pick
 * a number in that ballpark.
 */
#define kOSKextSystemLoadTimeout        (8 * 60)
#define kOSKextSystemLoadPauseTime      (30)

#define kOSKextPrelinkedKernelSuffixedName _kOSKextPrelinkedKernelFileName "."
/*
 * The path to _kOSKextPrelinkedKernelsPath from _kOSKextCachesRootFolder.
 *
 * This is used to make a backwards-compatible symlink (so older systems
 * can see / boot into newer systems). This symlink path is also used by
 * older versions of utilities such as Network Image Utility.
 * This can go away once we move everything to apfs because older systems
 * won't even be able to see those partitions.
 */
#define kPLKDirSymlinkPrefix "../../../PrelinkedKernels/"

#define kPersonalizeMacOSTool "/usr/local/bin/personalize_macos"

#define kListCDHashesTool "/usr/sbin/klist_cdhashes"

/*******************************************************************************
* Program Globals
*******************************************************************************/
const char * progname = "(unknown)";

CFMutableDictionaryRef       sExcludedKextAlertDict = NULL;

/*******************************************************************************
* Utility and callback functions.
*******************************************************************************/
// put/take helpers
static void waitForIOKitQuiescence(void);
static void waitForGreatSystemLoad(void);

#define kMaxArchs 64
#define kRootPathLen 256

static u_int usecs_from_timeval(struct timeval *t);
static void timeval_from_usecs(struct timeval *t, u_int usecs);
static void timeval_difference(struct timeval *dst,
                               struct timeval *a, struct timeval *b);
static Boolean isValidKextSigningTargetVolume(CFURLRef theURL);
static Boolean wantsFastLibCompressionForTargetVolume(CFURLRef theURL);
static void _appendIfNewest(CFMutableArrayRef theArray, OSKextRef theKext);
static void removeStalePrelinkedKernels(KextcacheArgs * toolArgs);
static Boolean isRootVolURL(CFURLRef theURL);
static bool isValidPLKFile(KextcacheArgs *toolArgs);
static bool isSystemPLKPath(KextcacheArgs *toolArgs);
static bool isSystemKernelPath(KextcacheArgs *toolArgs);
static bool isProbablyProtectedPLK(KextcacheArgs *toolArgs);
static bool isProtectedPLK(int prelinkedKernel_fd);
static bool isProtectedAction(KextcacheArgs *toolArgs);
static bool isSecureAuthentication(KextcacheArgs *toolArgs);
static ExitStatus buildImmutableKernelcache(KextcacheArgs *toolArgs, const char *plk_filename);

static ExitStatus updateKextAllowList(void);

/*******************************************************************************
*******************************************************************************/
int main(int argc, char * const * argv)
{
    KextcacheArgs       toolArgs;
    ExitStatus          result          = EX_SOFTWARE;

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
    * Check if we were spawned by kextd, set up straightaway
    * for service log filtering, and hook up to ASL.
    */
    if (getenv("KEXTD_SPAWNED")) {
        OSKextSetLogFilter(kDefaultServiceLogFilter | kOSKextLogKextOrGlobalMask,
            /* kernel? */ false);
        OSKextSetLogFilter(kDefaultServiceLogFilter | kOSKextLogKextOrGlobalMask,
            /* kernel? */ true);
        tool_openlog("com.apple.kextcache");
    } else {
        tool_initlog();
    }

    if (isDebugSetInBootargs()) {
#if 0 // default to more logging when running with debug boot-args
        OSKextLogSpec   logFilter = kOSKextLogDetailLevel |
                                    kOSKextLogVerboseFlagsMask |
                                    kOSKextLogKextOrGlobalMask;
        OSKextSetLogFilter(logFilter, /* kernel? */ false);
        OSKextSetLogFilter(logFilter, /* kernel? */ true);
#endif

        /* show command and all arguments...
         */
        int     i;
        int     myBufSize = 0;
        char *  mybuf;

        for (i = 0; i < argc; i++) {
            myBufSize += strlen(argv[i]) + 1;
        }
        mybuf = malloc(myBufSize);
        if (mybuf) {
            mybuf[0] = 0x00;
            for (i = 0; i < argc; i++) {
                if (strlcat(mybuf, argv[i], myBufSize) >= myBufSize) {
                    break;
                }
                if (strlcat(mybuf, " ", myBufSize) >= myBufSize) {
                    break;
                }
            }
            OSKextLog(NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "%s",
                      mybuf);
            free(mybuf);
        }
    }

   /*****
    * Process args & check for permission to load.
    */
    result = readArgs(&argc, &argv, &toolArgs);
    if (result != EX_OK) {
        if (result == kKextcacheExitHelp) {
            result = EX_OK;
        }
        goto finish;
    }

   /*****
    * Now that we have a custom verbose level set by options,
    * check the filter kextd passed in and combine them.
    */
    checkKextdSpawnedFilter(/* kernel? */ false);
    checkKextdSpawnedFilter(/* kernel? */ true);

    result = checkArgs(&toolArgs);
    if (result != EX_OK) {
        goto finish;
    }

    /*
     * Now that toolArgs is validated, determine what level of authentication checks necessary.
     *
     * Any security sensitive checks should also have safety nets in the output path
     * that ensures they are enabled for SIP protected output prelinked kernels.
     *
     * Since volumeRootURL is controlled by the caller and may point to an insecure location,
     * we must determine if we want to enforce kext signing for the volume once and refer
     * to this value in the future.  If this function is used again directly, the return value
     * may change and make any security checks susceptible to TOCTOU issues.
     */
    bool protectedAction = isProtectedAction(&toolArgs);

    toolArgs.authenticationOptions.allowNetwork = isKextdRunning();
    toolArgs.authenticationOptions.isCacheLoad = true;
    toolArgs.authenticationOptions.performFilesystemValidation = !toolArgs.skipAuthentication;
    toolArgs.authenticationOptions.performSignatureValidation = !toolArgs.skipAuthentication && isValidKextSigningTargetVolume(toolArgs.volumeRootURL);
    toolArgs.authenticationOptions.requireSecureLocation = !toolArgs.skipAuthentication && protectedAction;
    toolArgs.authenticationOptions.respectSystemPolicy = !toolArgs.skipAuthentication && protectedAction;
    toolArgs.authenticationOptions.checkDextApproval = !toolArgs.skipAuthentication && protectedAction;
    toolArgs.authenticationOptions.is_kextcache = true;

    _OSKextSetAuthenticationFunction(&authenticateKext, &toolArgs.authenticationOptions);
    _OSKextSetStrictAuthentication(true);

    /* Add cdhashes of userland executables to personalities in the prelinked
     * kernel and personality cache. */
    _OSKextSetPersonalityPatcherFunction(&addCDHashToDextPersonality);

   /* From here on out the default exit status is ok.
    */
    result = EX_OK;

    /* Reduce our priority and throttle I/O, then wait for a good time to run.
     */
    if (toolArgs.lowPriorityFlag) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogDetailLevel | kOSKextLogGeneralFlag,
            "Running in low-priority background mode.");

        setpriority(PRIO_PROCESS, getpid(), 20); // run at really low priority
        setiopolicy_np(IOPOL_TYPE_DISK, IOPOL_SCOPE_PROCESS, IOPOL_UTILITY);

        /* When building the prelinked kernel, we try to wait for a good time
         * to do work.
         */
        if (toolArgs.prelinkedKernelPath) {
            waitForGreatSystemLoad();
        }
    }

   /* The whole point of this program is to update caches, so let's not
    * try to read any (we'll briefly turn this back on when checking them).
    */
    OSKextSetUsesCaches(false);

    /* Staging stuff.
     */
    if (toolArgs.clearStaging) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogDetailLevel | kOSKextLogGeneralFlag,
                  "Clearing staging directory.");
        clearStagingDirectory();
        goto finish;
    } else if (toolArgs.pruneStaging) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogDetailLevel | kOSKextLogGeneralFlag,
                  "Pruning staging directory.");
        pruneStagingDirectory();
        goto finish;
    }

    if ((toolArgs.updateOpts & kBRUExpectUpToDate) &&
        (toolArgs.updateOpts & kBRUEarlyBoot) &&
        toolArgs.updateVolumeURL) {
        bool gotVolPath = false;
        char volPath[PATH_MAX] = {};
        /*
         * Re-purpose "-Boot -U /volume/path" to re-write the list of 3rd party kexts
         * that will be allowed to load during this boot. This is intended to be used
         * by an early boot task (in the single-threaded launchd startup phase) that
         * will restrict the set of 3rd party kexts that can be loaded.
         */
        gotVolPath = CFURLGetFileSystemRepresentation(toolArgs.updateVolumeURL,
                                                      true,
                                                      (UInt8*)volPath,
                                                      PATH_MAX);
        bool targetingBootVol = (gotVolPath && strcmp(volPath, "/") == 0);
        if (!targetingBootVol) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                      "-Boot -U {vol} only works on the root/boot volume");
            goto finish;
        }

        takeVolumeForPath(volPath);
        result = updateKextAllowList();
        putVolumeForPath(volPath, result);
        goto finish;
    }

#if !NO_BOOT_ROOT
   /* If it's a Boot!=root update or -invalidate invocation (both will set
    * updateVolumeURL), call checkUpdateCachesAndBoots() with the
    * previously-programmed flags and then jump to exit.  These operations don't
    * combine with more manual cache-building operations.
    */
    if (toolArgs.updateVolumeURL) {
        Boolean     gotVolPath = false;
        Boolean     targetingBootVol = false;
        char        volPath[PATH_MAX];

        gotVolPath = CFURLGetFileSystemRepresentation(toolArgs.updateVolumeURL,
                                                      true,
                                                      (UInt8*)volPath,
                                                      PATH_MAX);
        targetingBootVol = (gotVolPath && strcmp(volPath, "/") == 0);

        takeVolumeForPath(volPath);

        // go ahead and do the update
        result = doUpdateVolume(&toolArgs);

        if (targetingBootVol) {
            // For any update or invalidate, always prune the staging directory.
            pruneStagingDirectory();

            if (result == 0 && toolArgs.updateOpts & kBRUInvalidateKextcache) {
                // 16803220 - make sure to update other cache files too if we're
                // targeting the boot volume and we invalidated the prelinkedkernel.
                // Don't care about result here (we want the doUpdateVolume result)
                // We will update:
                // /S/L/C/com.apple.kext.caches/Startup/IOKitPersonalities_x86_64.ioplist.gz
                // /S/L/C/com.apple.kext.caches/Directories/S/L/E/KextIdentifiers.plist.gz
                // /S/L/C/com.apple.kext.caches/Directories/L/E/KextIdentifiers.plist.gz
                // /S/L/C/com.apple.kext.caches/Startup/KextPropertyValues_OSBundleHelper_x86_64.plist.gz
                updateSystemPlistCaches(&toolArgs);
            }
        }

        putVolumeForPath(volPath, result);
        goto finish;
    }
#endif /* !NO_BOOT_ROOT */

    /* If we're uncompressing the prelinked kernel, take care of that here
     * and exit.
     */
    if (toolArgs.prelinkedKernelPath && !CFArrayGetCount(toolArgs.argURLs) &&
        (toolArgs.compress || toolArgs.uncompress))
    {
        result = compressPrelinkedKernel(toolArgs.volumeRootURL,
                                         toolArgs.prelinkedKernelPath,
                                         /* compress */ toolArgs.compress);
        goto finish;
    }

   /*****
    * Read the kexts we'll be working with; first the set of all kexts, then
    * the repository and named kexts for use with prelinked kernel creation flags.
    */
    if (toolArgs.printTestResults) {
        OSKextSetRecordsDiagnostics(kOSKextDiagnosticsFlagAll);
    }

    /* If a secure location is required, ensure all kext scans return staged variants.
     * Otherwise, just load them directly from the URLs provided.
     */
    if (toolArgs.authenticationOptions.requireSecureLocation) {
        toolArgs.allKexts = createStagedKextsFromURLs(toolArgs.argURLs, true);
        toolArgs.repositoryKexts = createStagedKextsFromURLs(toolArgs.repositoryURLs, true);
        toolArgs.namedKexts = createStagedKextsFromURLs(toolArgs.namedKextURLs, true);
    } else {
        toolArgs.allKexts = OSKextCreateKextsFromURLs(kCFAllocatorDefault,
                                                      toolArgs.argURLs);
        toolArgs.repositoryKexts = OSKextCreateKextsFromURLs(kCFAllocatorDefault,
                                                             toolArgs.repositoryURLs);
        toolArgs.namedKexts = OSKextCreateKextsFromURLs(kCFAllocatorDefault,
                                                        toolArgs.namedKextURLs);
    }

    if (!toolArgs.allKexts || !CFArrayGetCount(toolArgs.allKexts)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "No kernel extensions found.");
        result = EX_SOFTWARE;
        goto finish;
    }

    if (!toolArgs.repositoryKexts || !toolArgs.namedKexts) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Error reading extensions.");
        result = EX_SOFTWARE;
        goto finish;
    }

    if (result != EX_OK) {
        goto finish;
    }

    if (toolArgs.needLoadedKextInfo) {
        result = copyLoadedKextInfo(&toolArgs.loadedKexts, true);
        if (result != EX_OK) {
            goto finish;
        }
    }

    // xxx - we are potentially overwriting error results here
    if (toolArgs.updateSystemCaches) {
        result = updateSystemPlistCaches(&toolArgs);
        // don't goto finish on error here, we might be able to create
        // the other caches
    }

    if (toolArgs.prelinkedKernelPath) {
       /* If we're updating the system prelinked kernel, make sure we aren't
        * Safe Boot, or dire consequences shall result.
        */
        if (toolArgs.needDefaultPrelinkedKernelInfo &&
            OSKextGetActualSafeBoot()) {

            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Can't update the system prelinked kernel during safe boot.");
            result = EX_OSERR;
            goto finish;
        }

       /* Create/update the prelinked kernel as explicitly requested, or
        * for the running kernel.
        */
        result = createPrelinkedKernel(&toolArgs);
        if (result != EX_OK) {
            goto finish;
        }
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
    SAFE_RELEASE(toolArgs.loadedKexts);
    SAFE_RELEASE(toolArgs.symbolDirURL);
    SAFE_FREE(toolArgs.prelinkedKernelPath);
    SAFE_FREE(toolArgs.prelinkedKernelDirname);
    SAFE_FREE(toolArgs.kernelPath);

    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus readArgs(
    int            * argc,
    char * const  ** argv,
    KextcacheArgs  * toolArgs)
{
    ExitStatus   result         = EX_USAGE;
    ExitStatus   scratchResult  = EX_USAGE;
    CFStringRef  scratchString  = NULL;  // must release
    CFNumberRef  scratchNumber  = NULL;  // must release
    CFURLRef     scratchURL     = NULL;  // must release
    uint32_t     i              = 0;
    int          optchar        = 0;
    int          longindex      = -1;
    struct stat  sb;

    bzero(toolArgs, sizeof(*toolArgs));
    toolArgs->kernel_fd = -1;
    toolArgs->prelinkedKernel_fd = -1;
    toolArgs->prelinkedKernelDir_fd = -1;

   /*****
    * Allocate collection objects.
    */
    if (!createCFMutableSet(&toolArgs->kextIDs, &kCFTypeSetCallBacks)             ||
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
                        "%s: option requires an argument -- -%c.",
                        progname, optopt);
                    break;
            }
        }

       /* Catch a -e/-system-mkext and redirect to -system-prelinked-kernel.
        */
        if (optchar == kOptSystemMkext) {
            optchar = 0;
            longopt = kLongOptSystemPrelinkedKernel;
        }

        switch (optchar) {

            case kOptArch:
                if (!addArchForName(toolArgs, optarg)) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                        "Unknown architecture %s.", optarg);
                    goto finish;
                }
                toolArgs->explicitArch = true;
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


#if !NO_BOOT_ROOT
            case kOptForce:
                toolArgs->updateOpts |= kBRUForceUpdateHelpers;
                break;
#endif /* !NO_BOOT_ROOT */

            case kOptLowPriorityFork:
                toolArgs->lowPriorityFlag = true;
                break;

            case kOptHelp:
                usage(kUsageLevelFull);
                result = kKextcacheExitHelp;
                goto finish;

            case kOptRepositoryCaches:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                    "-%c is no longer used; ignoring.",
                    kOptRepositoryCaches);
                break;

            case kOptKernel:
                if (toolArgs->kernelPath) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                        "Warning: kernel file already specified; using last.");
                } else {
                    toolArgs->kernelPath = malloc(PATH_MAX);
                    if (!toolArgs->kernelPath) {
                        OSKextLogMemError();
                        result = EX_OSERR;
                        goto finish;
                    }
                }
                char * resolved_path;
                resolved_path = realpath(optarg, toolArgs->kernelPath);
                if (resolved_path == NULL) {
                    OSKextLog(/* kext */ NULL,
                              kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                              "Error: kernel filename invalid");
                    goto finish;
                }
                break;

            case kOptLocalRoot:
                toolArgs->requiredFlagsRepositoriesOnly |=
                    kOSKextOSBundleRequiredLocalRootFlag;
                break;

            case kOptLocalRootAll:
                toolArgs->requiredFlagsAll |=
                    kOSKextOSBundleRequiredLocalRootFlag;
                break;

            case kOptNetworkRoot:
                toolArgs->requiredFlagsRepositoriesOnly |=
                    kOSKextOSBundleRequiredNetworkRootFlag;
                break;

            case kOptNetworkRootAll:
                toolArgs->requiredFlagsAll |=
                    kOSKextOSBundleRequiredNetworkRootFlag;
                break;

            case kOptAllLoaded:
                toolArgs->needLoadedKextInfo = true;
                break;

            case kOptSafeBoot:
                toolArgs->requiredFlagsRepositoriesOnly |=
                    kOSKextOSBundleRequiredSafeBootFlag;
                break;

            case kOptSafeBootAll:
                toolArgs->requiredFlagsAll |=
                    kOSKextOSBundleRequiredSafeBootFlag;
                break;

            case kOptTests:
                toolArgs->printTestResults = true;
                break;

            case kOptTargetOverride:
                toolArgs->targetForKextVariants = optarg;
                break;

#if !NO_BOOT_ROOT
            case kOptInvalidate:
                if (toolArgs->updateVolumeURL) {
                    OSKextLog(/* kext */ NULL,
                              kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                              "Warning: invalidate volume already specified; using last.");
                    SAFE_RELEASE_NULL(toolArgs->updateVolumeURL);
                }
                // sanity check that the volume exists
                if (stat(optarg, &sb)) {
                    OSKextLog(NULL,kOSKextLogWarningLevel|kOSKextLogFileAccessFlag,
                              "%s - %s.", optarg, strerror(errno));
                    result = EX_NOINPUT;
                    goto finish;
                }

                scratchURL = CFURLCreateFromFileSystemRepresentation(
                                                        kCFAllocatorDefault,
                                                        (const UInt8 *)optarg,
                                                        strlen(optarg),
                                                        true);
                if (!scratchURL) {
                    OSKextLogStringError(/* kext */ NULL);
                    result = EX_OSERR;
                    goto finish;
                }
                toolArgs->updateVolumeURL = CFRetain(scratchURL);
                toolArgs->updateOpts |= kBRUInvalidateKextcache;
                break;

#endif /* !NO_BOOT_ROOT */
            case kOptUpdate:
            case kOptCheckUpdate:
                if (toolArgs->updateVolumeURL) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                        "Warning: update volume already specified; using last.");
                    SAFE_RELEASE_NULL(toolArgs->updateVolumeURL);
                }
                // sanity check that the volume exists
                if (stat(optarg, &sb)) {
                    OSKextLog(NULL,kOSKextLogWarningLevel|kOSKextLogFileAccessFlag,
                              "%s - %s.", optarg, strerror(errno));
                    result = EX_NOINPUT;
                    goto finish;
                }

                scratchURL = CFURLCreateFromFileSystemRepresentation(
                    kCFAllocatorDefault,
                    (const UInt8 *)optarg, strlen(optarg), true);
                if (!scratchURL) {
                    OSKextLogStringError(/* kext */ NULL);
                    result = EX_OSERR;
                    goto finish;
                }
                toolArgs->updateVolumeURL = CFRetain(scratchURL);
                if (optchar == kOptCheckUpdate) {
                    toolArgs->updateOpts |= kBRUExpectUpToDate;
                    toolArgs->updateOpts |= kBRUCachesAnyRoot;
                }
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

            case kOptBuildImmutable:
                if (access(kPersonalizeMacOSTool, R_OK|X_OK) == 0) {
                    toolArgs->buildImmutableKernel = true;
                    toolArgs->updateOpts |= kBRUImmutableKernel;
                } else {
                    OSKextLog(/* kext */ NULL,
                              kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                              "WARNING: Cannot find personalization tool to build immutable kernel: skipping!");
                }
                break;

            case kOptNoAuthentication:
                toolArgs->skipAuthentication = true;
                break;

            case 0:
                switch (longopt) {
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


                    case kLongOptSystemCaches:
                        toolArgs->updateSystemCaches = true;
                        setSystemExtensionsFolders(toolArgs);
                        break;

                    case kLongOptCompressed:
                        toolArgs->compress = true;
                        break;

                    case kLongOptUncompressed:
                        toolArgs->uncompress = true;
                        break;

                    case kLongOptSymbols:
                        if (toolArgs->symbolDirURL) {
                            OSKextLog(/* kext */ NULL,
                                kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                                "Warning: symbol directory already specified; using last.");
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

                    case kLongOptSystemPrelinkedKernel:
                        scratchResult = setPrelinkedKernelArgs(toolArgs,
                            /* filename */ NULL);
                        if (scratchResult != EX_OK) {
                            result = scratchResult;
                            goto finish;
                        }
                        toolArgs->needLoadedKextInfo = true;
                        toolArgs->requiredFlagsRepositoriesOnly |=
                            kOSKextOSBundleRequiredLocalRootFlag;
                        break;

                    case kLongOptAllPersonalities:
                        toolArgs->includeAllPersonalities = true;
                        break;

                    case kLongOptNoLinkFailures:
                        toolArgs->noLinkFailures = true;
                        break;

                    case kLongOptStripSymbols:
                        toolArgs->stripSymbols = true;
                        break;

#if !NO_BOOT_ROOT
                    case kLongOptInstaller:
                        toolArgs->updateOpts |= kBRUHelpersOptional;
                        toolArgs->updateOpts |= kBRUForceUpdateHelpers;
                        break;
                    case kLongOptCachesOnly:
                        toolArgs->updateOpts |= kBRUCachesOnly;
                        break;
#endif /* !NO_BOOT_ROOT */
                    case kLongOptEarlyBoot:
                        toolArgs->updateOpts |= kBRUEarlyBoot;
                        break;

                    case kLongOptImmutableKexts:
                        /* -default-boot overrides all other kext filtering options */
                        toolArgs->requiredFlagsRepositoriesOnly = kImmutableKernelKextFilter;
                        break;
                    case kLongOptImmutableKextsAll:
                        /* -default-boot-all overrides all other kext filtering options */
                        toolArgs->requiredFlagsAll = kImmutableKernelKextFilter;
                        break;

                    case kLongOptClearStaging:
                        toolArgs->clearStaging = true;
                        break;
                    case kLongOptPruneStaging:
                        toolArgs->pruneStaging = true;
                        break;

                    default:
                       /* Because we use ':', getopt_long doesn't print an error message.
                        */
                        OSKextLog(/* kext */ NULL,
                            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                            "unrecognized option %s", (*argv)[optind-1]);
                        goto finish;
                        break;
                }
                break;

            default:
               /* Because we use ':', getopt_long doesn't print an error message.
                */
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

   /*****
    * If we aren't doing a boot<>root update, record the kext & directory names
    * from the command line. (If we are doing a boot<>root update, remaining
    * command line args are processed later.)
    */
    if (!toolArgs->updateVolumeURL) {
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
ExitStatus readPrelinkedKernelArgs(
    KextcacheArgs * toolArgs,
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
    KextcacheArgs * toolArgs,
    char          * filename)
{
    ExitStatus          result          = EX_USAGE;

    if (toolArgs->prelinkedKernelPath) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
            "Warning: prelinked kernel already specified; using last.");
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
#if NO_BOOT_ROOT
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Error: prelinked kernel filename required");
        goto finish;
#else
        if (!setDefaultPrelinkedKernel(toolArgs)) {
            goto finish;
        }
        toolArgs->needDefaultPrelinkedKernelInfo = true;
        setSystemExtensionsFolders(toolArgs);
#endif /* NO_BOOT_ROOT */
    } else {
        char *resolved_path;
        size_t len;
        resolved_path = realpath(filename, toolArgs->prelinkedKernelPath);
        if (resolved_path) {
            len = strlen(filename);
        } else {
            len = strlcpy(toolArgs->prelinkedKernelPath, filename, PATH_MAX);
        }
        if (len >= PATH_MAX) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "Error: prelinked kernel filename length exceeds PATH_MAX");
            goto finish;
        }
        toolArgs->prelinkedKernelDirname = malloc(PATH_MAX);
        if (!toolArgs->prelinkedKernelDirname) {
            OSKextLogMemError();
            result = EX_OSERR;
            goto finish;
        }
        (void)dirname_r(toolArgs->prelinkedKernelPath,
                        toolArgs->prelinkedKernelDirname);
    }
    result = EX_OK;
finish:
   return result;
}

#define kSyspolicyDB "/var/db/SystemPolicyConfiguration/KextPolicy"
#define kKextAllowListTmpFilePfx "/tmp/kextcache.klist.tmp"

static ExitStatus
updateKextAllowList(void)
{
    ExitStatus result = EX_USAGE;
    char bootuuid[37] = {};
    char tmpPath[PATH_MAX] = {};
    int cdhashesListFD = -1;
    int cacheDirFD = -1;
    size_t len;
    int rc;
    CFDataRef cdhashData = NULL; // must release
    CFStringRef hashStr = NULL; // must release

    /* NOTE: we assume here that we are updating the '/' volume */

    /*
     * grab the current boot's UUID
     */
    len = sizeof(bootuuid);
    if (sysctlbyname("kern.bootsessionuuid", bootuuid, &len, NULL, 0) < 0) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                  "ERROR getting kern.bootsessionuuid");
        return EX_OSERR;
    }
    bootuuid[36] = 0; /* NULL-terminate (and remove newline character, if present) */

    if (readKextHashAllowList(false, &hashStr, NULL, NULL, NULL)) {
        char str[37] = {};
        /*
         * We successfully read in the boot UUID from the hash allow list.
         * Check to see if it matches the current boot session UUID. If so,
         * then bail out of this function because we only update the file
         * once per boot!
         */
        const char *strptr = CFStringGetCStringPtr(hashStr, kCFStringEncodingUTF8);
        if (!strptr) {
            if (!CFStringGetCString(hashStr, str, sizeof(str), kCFStringEncodingUTF8)) {
                result = EX_OSERR;
                OSKextLog(/* kext */ NULL,
                          kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                          "Error handling boot session UUID string");
                goto out;
            } else {
                strptr = (const char *)str;
            }
        }
        if (strncmp(strptr, bootuuid, sizeof(bootuuid)) == 0) {
            /*
             * Same boot: deny the update
             */
            result = EX_NOPERM;
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                      "kext allow list already up-to-date for boot %s", bootuuid);
            goto out;
        } else {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogWarningLevel  | kOSKextLogGeneralFlag,
                      "Generating kext allow for boot %s (!= %s)", bootuuid, strptr);
        }
    }
    /* if we don't successfully load a hash allow list, that's OK: we're updating it! */

    /*
     * construct a path to the kext allow list
     */
    strlcpy(tmpPath, _kOSKextCachesRootFolder, PATH_MAX);

    cacheDirFD = open(tmpPath, O_RDONLY | O_DIRECTORY);
    if (cacheDirFD < 0) {
        result = EX_NOPERM;
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                  "Can't open cache directory '%s/'", tmpPath);
        goto out;
    }

    /*
     * We're now committed to updating the allowed 3rd party kext list.
     * First, use the tool to get a list of UUIDs in a temporary file.
     */
    strlcpy(tmpPath, kKextAllowListTmpFilePfx, sizeof(tmpPath));
    if (strlcat(tmpPath, ".XXXXXX", sizeof(tmpPath)) >= sizeof(tmpPath)) {
        result = EX_OSERR;
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                  "path to temp file too long? '%s'", tmpPath);
        goto out;
    }

    cdhashesListFD = mkstemp(tmpPath);
    if (cdhashesListFD < 0) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                  "ERROR opening tmp file at \"%s\"",
                  kKextAllowListTmpFilePfx);
        result = EX_OSERR;
        goto out;
    }

    char *listuuids_argv[] = {
        kListCDHashesTool,
        kSyspolicyDB,
        tmpPath,
        NULL
    };
    rc = fork_program(kListCDHashesTool, listuuids_argv, true /* wait */);
    unlink(tmpPath);

    if (rc != 0) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                  "ERROR(%d) gathering UUIDs from : \"%s : %s\"",
                  rc, kSyspolicyDB, tmpPath);
        result = EX_OSERR;
        goto out;
    }

    /* read in the content (just written by the tool) */
    if (!createCFDataFromFD(cdhashesListFD, &cdhashData)) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogGeneralFlag | kOSKextLogWarningLevel,
                  "ERROR reading cdhashes from : \"%s\": resetting to 0 allowed kexts!", tmpPath);
        const UInt8 zero = 0;
        cdhashData = CFDataCreate(kCFAllocatorDefault, &zero, 1);
    }

    /* save the contents + bootsession UUID to the allow list */
    result = writeKextAllowList(bootuuid, cdhashData, cacheDirFD, kThirdPartyKextAllowList);

 out:
    if (cdhashesListFD >= 0) {
        close(cdhashesListFD);
    }
    if (cacheDirFD >= 0) {
        close(cacheDirFD);
    }
    SAFE_RELEASE(cdhashData);
    SAFE_RELEASE(hashStr);

    return result;
}


#if !NO_BOOT_ROOT
#ifndef kIOPMAssertNoIdleSystemSleep
#define kIOPMAssertNoIdleSystemSleep \
            kIOPMAssertionTypePreventUserIdleSystemSleep
#endif
ExitStatus doUpdateVolume(KextcacheArgs *toolArgs)
{
    ExitStatus rval;                    // no goto's in this function
    int result;                         // errno-type value
    IOReturn pmres = kIOReturnError;    // init against future re-flow
    IOPMAssertionID awakeForUpdate;     // valid if pmres == 0
    os_signpost_id_t spid = generate_signpost_id();

    os_signpost_interval_begin(get_signpost_log(), spid, SIGNPOST_KEXTCACHE_UPDATE_VOLUME);

    // unless -F is passed, keep machine awake for for duration
    // (including waiting for any volume locks with kextd)
    if (toolArgs->lowPriorityFlag == false) {
        pmres = IOPMAssertionCreateWithName(kIOPMAssertNoIdleSystemSleep,
                            kIOPMAssertionLevelOn,
                            CFSTR("com.apple.kextmanager.update"),
                            &awakeForUpdate);
        if (pmres) {
            OSKextLog(NULL, kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                      "Warning: couldn't block sleep during cache update");
        } else {
            os_signpost_event_emit(get_signpost_log(), spid, SIGNPOST_EVENT_POWER_ASSERTION);
        }
    }

    result = checkUpdateCachesAndBoots(toolArgs->updateVolumeURL,
                                       toolArgs->updateOpts);
    // translate known errno -> sysexits(3) value
    switch (result) {
        case ENOENT:
        case EFTYPE: rval = EX_OSFILE; break;
        default: rval = result;
    }

    if (toolArgs->lowPriorityFlag == false && pmres == 0) {
        // drop assertion
        if (IOPMAssertionRelease(awakeForUpdate))
            OSKextLog(NULL, kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                      "Warning: error re-enabling sleep after cache update");
    }

    os_signpost_interval_end(get_signpost_log(), spid, SIGNPOST_KEXTCACHE_UPDATE_VOLUME);
    return rval;
}

/*******************************************************************************
*******************************************************************************/
Boolean setDefaultKernel(KextcacheArgs * toolArgs)
{
#if DEV_KERNEL_SUPPORT
    Boolean      addSuffix = FALSE;
#endif
    size_t       length = 0;
    struct stat  statBuf;

    if (!toolArgs->kernelPath) {
        toolArgs->kernelPath = malloc(PATH_MAX);
        if (!toolArgs->kernelPath) {
            OSKextLogMemError();
            return FALSE;
        }
    }

    while( true ) {
        // use KernelPath from /usr/standalone/bootcaches.plist
        if (getKernelPathForURL(toolArgs->volumeRootURL,
                                toolArgs->kernelPath,
                                PATH_MAX) == FALSE) {
            // no bootcaches.plist?  Forced to hardwire...
            strlcpy(toolArgs->kernelPath, "/System/Library/Kernels/kernel",
                    PATH_MAX);
        }

#if DEV_KERNEL_SUPPORT
        // for Apple Internal builds try to default to dev kernel
        // /System/Library/Kernels/kernel.development
        addSuffix = useDevelopmentKernel(toolArgs->kernelPath);
        if (addSuffix) {
            if (strlen(toolArgs->kernelPath) + strlen(kDefaultDevKernelSuffix) + 1 < PATH_MAX) {
                strlcat(toolArgs->kernelPath,
                        kDefaultDevKernelSuffix,
                        PATH_MAX);
            }
            else {
                addSuffix = FALSE;
            }
        }
#endif
        char * resolved_path = NULL;
        resolved_path = realpath(toolArgs->kernelPath, NULL);
        if (resolved_path == NULL) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "Error: invalid kernel path '%s'",
                      toolArgs->kernelPath);
            return FALSE;
        }
        free(toolArgs->kernelPath);
        toolArgs->kernelPath = resolved_path;
        break;

        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "Error: invalid kernel path '%s'",
                  toolArgs->kernelPath);
        return FALSE;
    } // while...

    TIMESPEC_TO_TIMEVAL(&toolArgs->kernelTimes[0], &statBuf.st_atimespec);
    TIMESPEC_TO_TIMEVAL(&toolArgs->kernelTimes[1], &statBuf.st_mtimespec);

#if DEV_KERNEL_SUPPORT
    if (toolArgs->prelinkedKernelPath &&
        toolArgs->needDefaultPrelinkedKernelInfo &&
        addSuffix) {
        // we are using default prelinkedkernel name so add .development suffix
        length = strlcat(toolArgs->prelinkedKernelPath,
                         kDefaultDevKernelSuffix,
                         PATH_MAX);
        if (length >= PATH_MAX) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "Error: prelinkedkernel filename length exceeds PATH_MAX");
            return FALSE;
        }
    }
#endif

    return TRUE;
}



/*******************************************************************************
*******************************************************************************/
Boolean setDefaultPrelinkedKernel(KextcacheArgs * toolArgs)
{
    Boolean      result              = FALSE;
    const char * prelinkedKernelFile = NULL;
    size_t       length              = 0;

    prelinkedKernelFile =
            _kOSKextTemporaryPrelinkedKernelsPath "/"
            _kOSKextPrelinkedKernelFileName;

    length = strlcpy(toolArgs->prelinkedKernelPath,
        prelinkedKernelFile, PATH_MAX);
    if (length >= PATH_MAX) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Error: prelinked kernel filename length exceeds PATH_MAX");
        goto finish;
    }
    toolArgs->prelinkedKernelDirname = malloc(PATH_MAX);
    if (!toolArgs->prelinkedKernelDirname) {
        OSKextLogMemError();
        goto finish;
    }
    strlcpy(toolArgs->prelinkedKernelDirname, _kOSKextTemporaryPrelinkedKernelsPath, PATH_MAX);

    // Default prelinked kernel is targeted at the root volume
    if (!toolArgs->volumeRootURL) {
        toolArgs->volumeRootURL = CFURLCreateWithFileSystemPath(NULL, CFSTR("/"), kCFURLPOSIXPathStyle, true);
        if (!toolArgs->volumeRootURL) {
            OSKextLogMemError();
            goto finish;
        }
    }

    result = TRUE;

finish:
    return result;
}
#endif /* !NO_BOOT_ROOT */

/*******************************************************************************
*******************************************************************************/
void setSystemExtensionsFolders(KextcacheArgs * toolArgs)
{
    CFArrayRef sysExtensionsFolders = OSKextGetSystemExtensionsFolderURLs();

    CFArrayAppendArray(toolArgs->argURLs,
        sysExtensionsFolders, RANGE_ALL(sysExtensionsFolders));
    CFArrayAppendArray(toolArgs->repositoryURLs,
        sysExtensionsFolders, RANGE_ALL(sysExtensionsFolders));

    return;
}

/*******************************************************************************
*******************************************************************************/

static void
waitForIOKitQuiescence(void)
{
    kern_return_t   kern_result = 0;
    mach_timespec_t waitTime = { 40, 0 };

    // if kextd is not running yet (early boot) then IOKitWaitQuiet will
    // always time out.  So go ahead and bail out if there is no kextd.
    if ( isKextdRunning() == FALSE ) {
        return;
    }

    OSKextLog(/* kext */ NULL,
        kOSKextLogProgressLevel | kOSKextLogIPCFlag,
        "Waiting for I/O Kit to quiesce.");

    kern_result = IOKitWaitQuiet(kIOMasterPortDefault, &waitTime);
    if (kern_result == kIOReturnTimeout) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "IOKitWaitQuiet() timed out.");
    } else if (kern_result != kOSReturnSuccess) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "IOKitWaitQuiet() failed - %s.",
            safe_mach_error_string(kern_result));
    }
}

/*******************************************************************************
* Wait for the system to report that it's a good time to do work.  We define a
* good time to be when the IOSystemLoadAdvisory API returns a combined level of
* kIOSystemLoadAdvisoryLevelGreat, and we'll wait up to kOSKextSystemLoadTimeout
* seconds for the system to enter that state before we begin our work.  If there
* is an error in this function, we just return and get started with the work.
*******************************************************************************/
static void
waitForGreatSystemLoad(void)
{
    struct timeval currenttime;
    struct timeval endtime;
    struct timeval timeout;
    fd_set readfds;
    fd_set tmpfds;
    uint64_t systemLoadAdvisoryState            = 0;
    uint32_t notifyStatus                       = 0;
    uint32_t usecs                              = 0;
    int systemLoadAdvisoryFileDescriptor        = 0;    // closed by notify_cancel()
    int systemLoadAdvisoryToken                 = 0;    // must notify_cancel()
    int currentToken                            = 0;    // do not notify_cancel()
    int myResult;

    bzero(&currenttime, sizeof(currenttime));
    bzero(&endtime, sizeof(endtime));
    bzero(&timeout, sizeof(timeout));

    OSKextLog(/* kext */ NULL,
        kOSKextLogProgressLevel | kOSKextLogGeneralFlag,
        "Waiting for low system load.");

    /* Register for SystemLoadAdvisory notifications */

    notifyStatus = notify_register_file_descriptor(kIOSystemLoadAdvisoryNotifyName,
        &systemLoadAdvisoryFileDescriptor,
        /* flags */ 0, &systemLoadAdvisoryToken);
    if (notifyStatus != NOTIFY_STATUS_OK) {
        goto finish;
    }

    OSKextLog(/* kext */ NULL,
        kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
        "Received initial system load status %llu", systemLoadAdvisoryState);

    /* If it's a good time, we'll just return */

    notifyStatus = notify_get_state(systemLoadAdvisoryToken, &systemLoadAdvisoryState);
    if (notifyStatus != NOTIFY_STATUS_OK) {
        goto finish;
    }

    if (systemLoadAdvisoryState == kIOSystemLoadAdvisoryLevelGreat) {
        goto finish;
    }

    /* Set up the select timers */

    myResult = gettimeofday(&currenttime, NULL);
    if (myResult < 0) {
        goto finish;
    }

    endtime = currenttime;
    endtime.tv_sec += kOSKextSystemLoadTimeout;

    timeval_difference(&timeout, &endtime, &currenttime);
    usecs = usecs_from_timeval(&timeout);

    FD_ZERO(&readfds);
    FD_SET(systemLoadAdvisoryFileDescriptor, &readfds);

    /* Check SystemLoadAdvisory notifications until it's a great time to
     * do work or we hit the timeout.
     */

    while (usecs) {
        /* Wait for notifications or the timeout */

        FD_COPY(&readfds, &tmpfds);
        myResult = select(systemLoadAdvisoryFileDescriptor + 1,
            &tmpfds, NULL, NULL, &timeout);
        if (myResult < 0) {
            goto finish;
        }

        /* Set up the next timeout */

        myResult = gettimeofday(&currenttime, NULL);
        if (myResult < 0) {
            goto finish;
        }

        timeval_difference(&timeout, &endtime, &currenttime);
        usecs = usecs_from_timeval(&timeout);

        /* Check the system load state */

        if (!FD_ISSET(systemLoadAdvisoryFileDescriptor, &tmpfds)) {
            continue;
        }

        myResult = (int)read(systemLoadAdvisoryFileDescriptor,
            &currentToken, sizeof(currentToken));
        if (myResult < 0) {
            goto finish;
        }

        /* The token is written in network byte order. */
        currentToken = ntohl(currentToken);

        if (currentToken != systemLoadAdvisoryToken) {
            continue;
        }

        notifyStatus = notify_get_state(systemLoadAdvisoryToken,
            &systemLoadAdvisoryState);
        if (notifyStatus != NOTIFY_STATUS_OK) {
            goto finish;
        }

        OSKextLog(/* kext */ NULL,
            kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
            "Received updated system load status %llu", systemLoadAdvisoryState);

        if (systemLoadAdvisoryState == kIOSystemLoadAdvisoryLevelGreat) {
            break;
        }
    }

    OSKextLog(/* kext */ NULL,
        kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
        "Pausing for another %d seconds to avoid work contention",
        kOSKextSystemLoadPauseTime);

    /* We'll wait a random amount longer to avoid colliding with
     * other work that is waiting for a great time.
     */
    sleep(kOSKextSystemLoadPauseTime);

    OSKextLog(/* kext */ NULL,
        kOSKextLogDebugLevel | kOSKextLogGeneralFlag,
        "System load is low.  Proceeding.\n");
finish:
    if (systemLoadAdvisoryToken) {
        notify_cancel(systemLoadAdvisoryToken);
    }
    return;
}

/*******************************************************************************
*******************************************************************************/
static u_int
usecs_from_timeval(struct timeval *t)
{
    u_int usecs = 0;

    if (t) {
        usecs = (unsigned int)((t->tv_sec * 1000) + t->tv_usec);
    }

    return usecs;
}

/*******************************************************************************
*******************************************************************************/
static void
timeval_from_usecs(struct timeval *t, u_int usecs)
{
    if (t) {
        if (usecs > 0) {
            t->tv_sec = usecs / 1000;
            t->tv_usec = usecs % 1000;
        } else {
            bzero(t, sizeof(*t));
        }
    }
}

/*******************************************************************************
* dst = a - b
*******************************************************************************/
static void
timeval_difference(struct timeval *dst, struct timeval *a, struct timeval *b)
{
    u_int ausec = 0, busec = 0, dstusec = 0;

    if (dst) {
        ausec = usecs_from_timeval(a);
        busec = usecs_from_timeval(b);

        if (ausec > busec) {
            dstusec = ausec - busec;
        }

        timeval_from_usecs(dst, dstusec);
    }
}

#if !NO_BOOT_ROOT
/*******************************************************************************
*******************************************************************************/
void setDefaultArchesIfNeeded(KextcacheArgs * toolArgs)
{
   /* If no arches were explicitly specified, use the architecture of the
    * running kernel.
    */
    if (toolArgs->explicitArch) {
        return;
    }

    CFArrayRemoveAllValues(toolArgs->targetArchs);
    addArch(toolArgs, OSKextGetRunningKernelArchitecture());

    return;
}
#endif /* !NO_BOOT_ROOT */

/*******************************************************************************
********************************************************************************/
void addArch(
    KextcacheArgs * toolArgs,
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
    KextcacheArgs     * toolArgs,
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
void checkKextdSpawnedFilter(Boolean kernelFlag)
{
    const char * environmentVariable  = NULL;  // do not free
    char       * environmentLogFilterString = NULL;  // do not free

    if (kernelFlag) {
        environmentVariable = "KEXT_LOG_FILTER_KERNEL";
    } else {
        environmentVariable = "KEXT_LOG_FILTER_USER";
    }

    environmentLogFilterString = getenv(environmentVariable);

   /*****
    * If we have environment variables for a log spec, take the greater
    * of the log levels and OR together the flags from the environment's &
    * this process's command-line log specs. This way the most verbose setting
    * always applies.
    *
    * Otherwise, set the environment variable in case we spawn children.
    */
    if (environmentLogFilterString) {
        OSKextLogSpec toolLogSpec  = OSKextGetLogFilter(kernelFlag);
        OSKextLogSpec kextdLogSpec = (unsigned int)strtoul(environmentLogFilterString, NULL, 16);

        OSKextLogSpec toolLogLevel  = toolLogSpec & kOSKextLogLevelMask;
        OSKextLogSpec kextdLogLevel = kextdLogSpec & kOSKextLogLevelMask;
        OSKextLogSpec comboLogLevel = MAX(toolLogLevel, kextdLogLevel);

        OSKextLogSpec toolLogFlags  = toolLogSpec & kOSKextLogFlagsMask;
        OSKextLogSpec kextdLogFlags = kextdLogSpec & kOSKextLogFlagsMask;
        OSKextLogSpec comboLogFlags = toolLogFlags | kextdLogFlags |
            kOSKextLogKextOrGlobalMask;

        OSKextSetLogFilter(comboLogLevel | comboLogFlags, kernelFlag);
    } else {
        char logSpecBuffer[16];  // enough for a 64-bit hex value

        snprintf(logSpecBuffer, sizeof(logSpecBuffer), "0x%x",
            OSKextGetLogFilter(kernelFlag));
        setenv(environmentVariable, logSpecBuffer, /* overwrite */ 1);
    }

    return;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus checkArgs(KextcacheArgs * toolArgs)
{
    ExitStatus  result  = EX_USAGE;
    Boolean expectUpToDate = toolArgs->updateOpts & kBRUExpectUpToDate;

    if (!toolArgs->prelinkedKernelPath &&
        !toolArgs->updateVolumeURL && !toolArgs->updateSystemCaches &&
        !toolArgs->clearStaging && !toolArgs->pruneStaging)
    {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "No work to do; check options and try again.");
        goto finish;
    }

    if (toolArgs->clearStaging || toolArgs->pruneStaging)
    {
        if (toolArgs->prelinkedKernelPath ||
            toolArgs->updateVolumeURL ||
            toolArgs->updateSystemCaches)
        {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "Kernel extension staging functions must be used alone.");
            goto finish;
        }

        if (geteuid() != 0) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "You must be running as root to manage the kernel extension staging area.");
            result = EX_NOPERM;
            goto finish;
        }
    }

    if (toolArgs->volumeRootURL &&
        !toolArgs->prelinkedKernelPath)
    {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Use -%s only when creating a prelinked kernel.",
            kOptNameVolumeRoot);
        goto finish;
    }

    if (!toolArgs->updateVolumeURL && !CFArrayGetCount(toolArgs->argURLs) &&
        !toolArgs->compress && !toolArgs->uncompress &&
        !toolArgs->clearStaging && !toolArgs->pruneStaging)
    {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "No kexts or directories specified.");
        goto finish;
    }

    if (!toolArgs->compress && !toolArgs->uncompress) {
        toolArgs->compress = true;
    } else if (toolArgs->compress && toolArgs->uncompress) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Both -%s and -%s specified; using -%s.",
            kOptNameCompressed, kOptNameUncompressed, kOptNameCompressed);
        toolArgs->compress = true;
        toolArgs->uncompress = false;
    }

#if !NO_BOOT_ROOT
    if ((toolArgs->updateOpts & kBRUForceUpdateHelpers)
            && (toolArgs->updateOpts & kBRUCachesOnly)) {
        OSKextLog(NULL, kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "-%s (%-c) and %-s are mutually exclusive",
                  kOptNameForce, kOptForce, kOptNameCachesOnly);
        goto finish;
    }
    if (toolArgs->updateOpts & kBRUForceUpdateHelpers) {
        if (expectUpToDate || !toolArgs->updateVolumeURL) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "-%s (-%c) is allowed only with -%s (-%c).",
                kOptNameForce, kOptForce, kOptNameUpdate, kOptUpdate);
            goto finish;
        }
    }
    if (toolArgs->updateOpts & kBRUCachesOnly) {
        if (expectUpToDate || !toolArgs->updateVolumeURL) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "-%s is allowed only with -%s (-%c).",
                      kOptNameCachesOnly, kOptNameUpdate, kOptUpdate);
            goto finish;
        }
    }
#endif /* !NO_BOOT_ROOT */
    if (toolArgs->updateOpts & kBRUEarlyBoot) {
        if (!toolArgs->updateVolumeURL ||
            !(toolArgs->updateOpts & kBRUExpectUpToDate) ||
            !(toolArgs->updateOpts & kBRUCachesAnyRoot)) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "-%s requires -%c.",
                kOptNameEarlyBoot, kOptCheckUpdate);
            goto finish;
        }
    }

    if (toolArgs->updateVolumeURL) {
        if (toolArgs->prelinkedKernelPath) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Can't create prelinked kernel when updating volumes.");
        }
    }

    if (toolArgs->updateVolumeURL &&
        toolArgs->updateOpts & kBRUInvalidateKextcache &&
        geteuid() != 0) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "You must be running as root to update prelinked kernel.");
        result = EX_NOPERM;
        goto finish;
    }

#if !NO_BOOT_ROOT
    setDefaultArchesIfNeeded(toolArgs);
#endif /* !NO_BOOT_ROOT */

   /* 11860417 - we now support multiple extensions directories, get access and
    * mod times from extensions directory with the most current mode date.
    */
    if (toolArgs->extensionsDirTimes[1].tv_sec == 0 &&
        CFArrayGetCount(toolArgs->repositoryURLs)) {
        result = getLatestTimesFromCFURLArray(toolArgs->repositoryURLs,
                                              toolArgs->extensionsDirTimes);
        if (result != EX_OK) {
            OSKextLog(NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "%s: Can't get mod times", __FUNCTION__);
            goto finish;
        }
    }

#if !NO_BOOT_ROOT
    if (toolArgs->needDefaultPrelinkedKernelInfo && !toolArgs->kernelPath) {
        if (!setDefaultKernel(toolArgs)) {
            result = EX_USAGE;
            goto finish;
        }
    }
#endif /* !NO_BOOT_ROOT */

    if (toolArgs->prelinkedKernelPath && CFArrayGetCount(toolArgs->argURLs)) {
        struct stat     myStatBuf;

        if (!toolArgs->kernelPath) {
            if (!setDefaultKernel(toolArgs)) {
                OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "No kernel specified for prelinked kernel generation.");
                result = EX_USAGE;
                goto finish;
            }
        }
        result = statPath(toolArgs->kernelPath, &myStatBuf);
        if (result != EX_OK) {
            goto finish;
        }
        TIMESPEC_TO_TIMEVAL(&toolArgs->kernelTimes[0], &myStatBuf.st_atimespec);
        TIMESPEC_TO_TIMEVAL(&toolArgs->kernelTimes[1], &myStatBuf.st_mtimespec);
    }

   /* Updating system caches requires no additional kexts or repositories,
    * and must run as root.
    */
    if (toolArgs->needDefaultPrelinkedKernelInfo ||
            toolArgs->updateSystemCaches) {

        if (CFArrayGetCount(toolArgs->namedKextURLs) || CFSetGetCount(toolArgs->kextIDs) ||
            !CFEqual(toolArgs->repositoryURLs, OSKextGetSystemExtensionsFolderURLs())) {

            OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Custom kexts and repository directories are not allowed "
                    "when updating system kext caches.");
            result = EX_USAGE;
            goto finish;

        }

        if (geteuid() != 0) {
            OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "You must be running as root to update system kext caches.");
            result = EX_NOPERM;
            goto finish;
        }
    }

    if (toolArgs->buildImmutableKernel) {
#if DEV_KERNEL_SUPPORT
        /*
         * building and personalizing the immutable kernel requires root privilege
         */
        if (geteuid() != 0) {
            OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "You must be running as root to replace the immutable kernel.");
            result = EX_NOPERM;
            goto finish;
        }

        /*
         * If extra kexts or repositories are specified, output a warning that this
         * will be a non-standard immutable kernel.
         */
        if (CFArrayGetCount(toolArgs->namedKextURLs) || CFSetGetCount(toolArgs->kextIDs) ||
            ((CFArrayGetCount(toolArgs->repositoryURLs) > 0) &&
              !CFEqual(toolArgs->repositoryURLs, OSKextGetSystemExtensionsFolderURLs()))) {
            OSKextLog(/* kext */ NULL,
                    kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                    "WARNING: You have specified custom kexts or kext repository "
                    "directories, and you are replacing your immutable kernel. "
                    "The end result will *not* be a standard immutablekernel!");
        }

        /* default to building the standard immutable kernel */
        if (!toolArgs->requiredFlagsRepositoriesOnly && !toolArgs->requiredFlagsAll) {
            toolArgs->requiredFlagsRepositoriesOnly = kImmutableKernelKextFilter;
            toolArgs->requiredFlagsAll = kImmutableKernelKextFilter;
        }

        if (toolArgs->requiredFlagsRepositoriesOnly != kImmutableKernelKextFilter ||
            toolArgs->requiredFlagsAll != kImmutableKernelKextFilter) {
            OSKextLog(/* kext */ NULL,
                    kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                    "WARNING: You are building an immutable kernel with a non-standard filter!");
        }
#else /* !DEV_KERNEL_SUPPORT */
        /* this code path is not supported (probably not compiled anywhere) */
        OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "No support for updating the immutable kernel!");
        result = EX_NOPERM;
        goto finish;
#endif /* DEV_KERNEL_SUPPORT */
    }

    /* This is best-effort check to present nice error messages to users.  Since a static check
     * up-front is vulnerable to race conditions, these conditions will be validated again later
     * when it can make a race-free check on the actual file descriptor (that isn't available here).
     */
    if (isProbablyProtectedPLK(toolArgs)) {
        if (toolArgs->skipAuthentication) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "Cannot create a prelinked kernel in a SIP protected location without authentication!");
            result = EX_NOPERM;
            goto finish;
        }
    }

    result = EX_OK;

finish:
    if (result == EX_USAGE) {
        usage(kUsageLevelBrief);
    }
    return result;
}

#pragma mark System Plist Caches

/*******************************************************************************
*******************************************************************************/

ExitStatus
copyLoadedKextInfo(
    CFArrayRef *loadedKexts,
    bool waitToQuiesce)
{
    ExitStatus  result                  = EX_SOFTWARE;
    CFArrayRef  requestedIdentifiers    = NULL; // must release

    if (waitToQuiesce) {
        /* Let I/O Kit settle down before we poke at it.
         */
        (void) waitForIOKitQuiescence();
    }

    /* Get the list of requested bundle IDs from the kernel and find all of
     * the associated kexts.
     */

    requestedIdentifiers = OSKextCopyAllRequestedIdentifiers();
    if (!requestedIdentifiers) {
        goto finish;
    }

    if (loadedKexts) {
        *loadedKexts = OSKextCopyKextsWithIdentifiers(requestedIdentifiers);
        if (*loadedKexts == NULL) {
            goto finish;
        }
    }

    result = EX_OK;
finish:
    SAFE_RELEASE(requestedIdentifiers);
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus updateSystemPlistCaches(KextcacheArgs * toolArgs)
{
    ExitStatus         result               = EX_OSERR;
    ExitStatus         directoryResult      = EX_OK;  // flipped to error as needed
    CFArrayRef         systemExtensionsURLs = NULL;   // do not release
    CFArrayRef         unauthenticatedKexts = NULL;   // must release
    CFMutableArrayRef  authenticKexts       = NULL;   // must release
    CFURLRef           folderURL            = NULL;   // do not release
    char               folderPath[PATH_MAX] = "";
    const NXArchInfo * startArch            = OSKextGetArchitecture();
    CFArrayRef         directoryValues      = NULL;   // must release
    CFArrayRef         personalities        = NULL;   // must release
    os_signpost_id_t   spid                 = generate_signpost_id();
    CFIndex            count, i;

    os_signpost_interval_begin(get_signpost_log(), spid, SIGNPOST_KEXTCACHE_UPDATE_PLISTS);

   /* The system plist caches are always operating on the current system volume, so
    * we should ensure that the authentication options are secure, or SIP is disabled.
    */
    if (!isSecureAuthentication(toolArgs)) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                  "Unable to update system caches with current authentication options.");
        result = EX_NOPERM;
        goto finish;
    }

   /* We only care about updating info for the system extensions folders.
    */
    systemExtensionsURLs = OSKextGetSystemExtensionsFolderURLs();
    if (!systemExtensionsURLs) {
        OSKextLogMemError();
        result = EX_OSERR;
        goto finish;
    }

    /* Since only authenticated personalities can be sent to the kernel, and authentication
     * requires them being in a secure location, staging must happen here.  Since
     * this function is only called for the current system volume, it should always require
     * staging unless SIP is disabled, which staging already takes care of checking.
     */
    unauthenticatedKexts = createStagedKextsFromURLs(systemExtensionsURLs, false);
    if (!unauthenticatedKexts) {
        goto finish;
    }

    if (!createCFMutableArray(&authenticKexts, &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        result = EX_OSERR;
        goto finish;
    }

    /* Any kernel extension evaluated below is being evaluated for its personalities
     * and not for inclusion in the kernel cache, so update the authentication options for
     * the remainder of this function and ensure it gets set back to true in the end.
     */
    toolArgs->authenticationOptions.isCacheLoad = false;

    count = CFArrayGetCount(unauthenticatedKexts);
    for (i = 0; i < count; i++) {
        OSKextRef aKext = (OSKextRef)CFArrayGetValueAtIndex(unauthenticatedKexts, i);
        if (OSKextIsAuthentic(aKext)) {
            CFArrayAppendValue(authenticKexts, aKext);
        }
    }

   /* Update the global personalities & property-value caches, each per arch.
    */
    for (i = 0; i < CFArrayGetCount(toolArgs->targetArchs); i++) {
        const NXArchInfo * targetArch =
            CFArrayGetValueAtIndex(toolArgs->targetArchs, i);

        SAFE_RELEASE_NULL(personalities);

       /* Set the active architecture for scooping out personalities and such.
        */
        if (!OSKextSetArchitecture(targetArch)) {
            goto finish;
        }

        personalities = OSKextCopyPersonalitiesOfKexts(authenticKexts);
        if (!personalities) {
            goto finish;
        }

        if (!_OSKextWriteCache(systemExtensionsURLs, CFSTR(kIOKitPersonalitiesKey),
            targetArch, _kOSKextCacheFormatIOXML, personalities)) {

            goto finish;
        }

       /* Loginwindow asks us for this property so let's spare lots of I/O
        * by caching it. This read function call updates the caches for us;
        * we don't use the output.
        */
        if (!readSystemKextPropertyValues(CFSTR(kOSBundleHelperKey), targetArch,
                /* forceUpdate? */ true, /* values */ NULL)) {

            goto finish;
        }

        /* And kextd asks for this each time it starts */
        if (!readSystemKextPropertyValues(CFSTR("PGO"), targetArch,
                /* forceUpdate? */ true, /* values */ NULL)) {
            goto finish;
        }
    }

   /* Update per-directory caches. This is just KextIdentifiers any more.
    */
    count = CFArrayGetCount(systemExtensionsURLs);
    for (i = 0; i < count; i++) {

        folderURL = CFArrayGetValueAtIndex(systemExtensionsURLs, i);

        if (!CFURLGetFileSystemRepresentation(folderURL, /* resolveToBase */ true,
                    (UInt8 *)folderPath, sizeof(folderPath))) {

            OSKextLogStringError(/* kext */ NULL);
            goto finish;
        }
        if (EX_OK != updateDirectoryCaches(toolArgs, folderURL)) {
            directoryResult = EX_OSERR;
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogBasicLevel | kOSKextLogGeneralFlag,
                "Directory caches updated for %s.", folderPath);
        }
    }

    if (directoryResult == EX_OK) {
        result = EX_OK;
    }

finish:
    SAFE_RELEASE(unauthenticatedKexts);
    SAFE_RELEASE(authenticKexts);
    SAFE_RELEASE(directoryValues);
    SAFE_RELEASE(personalities);

    OSKextSetArchitecture(startArch);
    toolArgs->authenticationOptions.isCacheLoad = true;
    os_signpost_interval_end(get_signpost_log(), spid, SIGNPOST_KEXTCACHE_UPDATE_PLISTS);

    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus updateDirectoryCaches(
        KextcacheArgs * toolArgs,
        CFURLRef        folderURL)
{
    ExitStatus         result           = EX_OK;  // optimistic!
    CFArrayRef         kexts            = NULL;   // must release

    kexts = OSKextCreateKextsFromURL(kCFAllocatorDefault, folderURL);
    if (!kexts) {
        result = EX_OSERR;
        goto finish;
    }

    /* The identifier cache is insecure, so no need to check for authentic kexts here.
     * The bundle identifier cache is only used to speed up lookup of kexts by identifier
     * in IOKit.  Lookups by identifier result in an OSKext object, but  any attempt
     * to load the kext or its personalities will result in the kext being authenticated
     * in that environment.
     */
    if (!_OSKextWriteIdentifierCacheForKextsInDirectory(
                kexts, folderURL, /* force? */ true)) {
        result = EX_OSERR;
        goto finish;
    }

    result = EX_OK;

finish:
    SAFE_RELEASE(kexts);
    return result;
}

#pragma mark Misc Stuff

/*******************************************************************************
*******************************************************************************/
ExitStatus
getFileURLModTimePlusOne(
    CFURLRef            fileURL,
    struct timeval      *origModTime,
    struct timeval      cacheFileTimes[2])
{
    ExitStatus   result          = EX_SOFTWARE;
    char         path[PATH_MAX];

    if (!CFURLGetFileSystemRepresentation(fileURL, /* resolveToBase */ true,
            (UInt8 *)path, sizeof(path)))
    {
        OSKextLogStringError(/* kext */ NULL);
        goto finish;
    }
    result = getFilePathModTimePlusOne(path, origModTime, cacheFileTimes);

finish:
    return result;
}

/*******************************************************************************
 *******************************************************************************/
ExitStatus
getFileDescriptorModTimePlusOne(
                          int               the_fd,
                          struct timeval *  origModTime,
                          struct timeval    cacheFileTimes[2])
{
    ExitStatus          result          = EX_SOFTWARE;

    result = getFileDescriptorTimes(the_fd, cacheFileTimes);
    if (result != EX_OK) {
        goto finish;
    }

    /* If asked, check to see if mod time has changed */
    if (origModTime != NULL) {
        if (timercmp(origModTime, &cacheFileTimes[1], !=)) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag | kOSKextLogFileAccessFlag,
                      "Source item has changed since starting; "
                      "not saving cache file <%s %d>", __func__, __LINE__);
            result = kKextcacheExitStale;
            goto finish;
        }
    }

    /* bump modtime by 1 second */
    cacheFileTimes[1].tv_sec++;
    result = EX_OK;
finish:
    return result;
}

/*******************************************************************************
 *******************************************************************************/
ExitStatus
getFilePathModTimePlusOne(
                          const char        * filePath,
                          struct timeval    * origModTime,
                          struct timeval      cacheFileTimes[2])
{
    ExitStatus          result          = EX_SOFTWARE;

    result = getFilePathTimes(filePath, cacheFileTimes);
    if (result != EX_OK) {
        goto finish;
    }

    /* If asked, check to see if mod time has changed */
    if (origModTime != NULL) {
        if (timercmp(origModTime, &cacheFileTimes[1], !=)) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag | kOSKextLogFileAccessFlag,
                      "Source item %s has changed since starting; "
                      "not saving cache file", filePath);
            result = kKextcacheExitStale;
            goto finish;
        }
    }

    /* bump modtime by 1 second */
    cacheFileTimes[1].tv_sec++;
    result = EX_OK;
finish:
    return result;
}

/*******************************************************************************
*******************************************************************************/
typedef struct {
    KextcacheArgs     * toolArgs;
    CFMutableArrayRef   kextArray;
    Boolean             error;
} FilterIDContext;

void filterKextID(const void * vValue, void * vContext)
{
    CFStringRef       kextID  = (CFStringRef)vValue;
    FilterIDContext * context = (FilterIDContext *)vContext;
    OSKextRef       theKext = OSKextGetKextWithIdentifier(kextID);

   /* This should really be a fatal error but embedded counts on
    * having optional kexts specified by identifier.
    */
    if (!theKext) {
        char kextIDCString[KMOD_MAX_NAME];

        CFStringGetCString(kextID, kextIDCString, sizeof(kextIDCString),
            kCFStringEncodingUTF8);
        OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Can't find kext with optional identifier %s; skipping.", kextIDCString);
#if 0
        OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Error - can't find kext with identifier %s.", kextIDCString);
        context->error = TRUE;
#endif /* 0 */
        goto finish;
    }

    if (kextMatchesFilter(context->toolArgs, theKext,
            context->toolArgs->requiredFlagsAll) &&
        !CFArrayContainsValue(context->kextArray,
            RANGE_ALL(context->kextArray), theKext))
    {
        CFArrayAppendValue(context->kextArray, theKext);
    }

finish:
    return;
}


/*******************************************************************************
*******************************************************************************/
ExitStatus filterKextsForCache(
        KextcacheArgs     * toolArgs,
        CFMutableArrayRef   kextArray,
        const NXArchInfo  * arch,
        Boolean           * fatalOut)
{
    ExitStatus          result        = EX_SOFTWARE;
    CFMutableArrayRef   firstPassArray = NULL;
    OSKextRequiredFlags requiredFlags;
    CFIndex             count, i;
    Boolean             earlyBoot = false;

    if (!createCFMutableArray(&firstPassArray, &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        goto finish;
    }

   /*****
    * Apply filters to select the kexts.
    *
    * If kexts have been specified by identifier, those are the only kexts we are going to use.
    * Otherwise run through the repository and named kexts and see which ones match the filter.
    */
    if (CFSetGetCount(toolArgs->kextIDs)) {
        FilterIDContext context;

        context.toolArgs = toolArgs;
        context.kextArray = firstPassArray;
        context.error = FALSE;
        CFSetApplyFunction(toolArgs->kextIDs, filterKextID, &context);

        if (context.error) {
            goto finish;
        }

    } else {

       /* Set up the required flags for repository kexts. If any are set from
        * the command line, toss in "Root," "Console," and "DriverKit," too.
        */
        requiredFlags = toolArgs->requiredFlagsRepositoriesOnly |
            toolArgs->requiredFlagsAll;
        if (requiredFlags) {
            requiredFlags |= kOSKextOSBundleRequiredRootFlag |
                kOSKextOSBundleRequiredConsoleFlag |
                kOSKextOSBundleRequiredDriverKitFlag;
        }

        count = CFArrayGetCount(toolArgs->repositoryKexts);
        for (i = 0; i < count; i++) {

            OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(
                    toolArgs->repositoryKexts, i);

            if (!kextMatchesFilter(toolArgs, theKext, requiredFlags)) {

                char kextPath[PATH_MAX];

                if (!CFURLGetFileSystemRepresentation(OSKextGetURL(theKext),
                    /* resolveToBase */ false, (UInt8 *)kextPath, sizeof(kextPath)))
                {
                    strlcpy(kextPath, "(unknown)", sizeof(kextPath));
                }

                if (toolArgs->prelinkedKernelPath) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogStepLevel | kOSKextLogArchiveFlag,
                        "%s is not demanded by OSBundleRequired conditions.",
                        kextPath);
                }
                continue;
            }

            if (!CFArrayContainsValue(firstPassArray, RANGE_ALL(firstPassArray), theKext)) {
                _appendIfNewest(firstPassArray, theKext);
            }
        }

       /* Set up the required flags for named kexts. If any are set from
        * the command line, toss in "Root," "Console," and "DriverKit," too.
        */
        requiredFlags = toolArgs->requiredFlagsAll;
        if (requiredFlags) {
            requiredFlags |= kOSKextOSBundleRequiredRootFlag |
                kOSKextOSBundleRequiredConsoleFlag |
                kOSKextOSBundleRequiredDriverKitFlag;
        }

        count = CFArrayGetCount(toolArgs->namedKexts);
        for (i = 0; i < count; i++) {
            OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(
                    toolArgs->namedKexts, i);

            if (!kextMatchesFilter(toolArgs, theKext, requiredFlags)) {

                char kextPath[PATH_MAX];

                if (!CFURLGetFileSystemRepresentation(OSKextGetURL(theKext),
                    /* resolveToBase */ false, (UInt8 *)kextPath, sizeof(kextPath)))
                {
                    strlcpy(kextPath, "(unknown)", sizeof(kextPath));
                }

                if (toolArgs->prelinkedKernelPath) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogStepLevel | kOSKextLogArchiveFlag,
                        "%s is not demanded by OSBundleRequired conditions.",
                        kextPath);
                }
                continue;
            }

            if (!CFArrayContainsValue(firstPassArray, RANGE_ALL(firstPassArray), theKext)) {
                _appendIfNewest(firstPassArray, theKext);
            }
        }
    }

   /*****
    * Take all the kexts that matched the filters above and check them for problems.
    */
    CFArrayRemoveAllValues(kextArray);

    count = CFArrayGetCount(firstPassArray);
    if (count) {

        if (callSecKeychainMDSInstall() != 0) {
            // this should never fail, so bail if it does.
            goto finish;
        }
        // not perfect, but we check to see if kextd is running to determine
        // if we are in early boot.
        earlyBoot = (isKextdRunning() == false);
        OSKextIsInExcludeList(NULL, false); // prime the exclude list cache
        isInStrictExceptionList(NULL, NULL, false); // prime the strict exception list cache
        isInExceptionList(NULL, NULL, false); // prime the exception list cache
        for (i = count - 1; i >= 0; i--) {
            char kextPath[PATH_MAX];
            OSKextRef ownedKext = NULL;
            OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(
                    firstPassArray, i);

            if (!CFURLGetFileSystemRepresentation(OSKextGetURL(theKext),
                /* resolveToBase */ false, (UInt8 *)kextPath, sizeof(kextPath)))
            {
                strlcpy(kextPath, "(unknown)", sizeof(kextPath));
            }

            /* Skip kexts we have no interest in for the current arch.
             */
            if (!OSKextSupportsArchitecture(theKext, arch)) {
                OSKextLog(/* kext */ NULL,
                    kOSKextLogStepLevel | kOSKextLogArchiveFlag,
                    "%s doesn't support architecture '%s'; skipping.", kextPath,
                    arch->name);
                goto loop_continue;
            }

            if (!OSKextIsValid(theKext)) {
                // xxx - should also use kOSKextLogArchiveFlag?
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                    kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                    "%s is not valid; omitting.", kextPath);
                if (toolArgs->printTestResults) {
                    OSKextLogDiagnostics(theKext, kOSKextDiagnosticsFlagAll);
                }
                goto loop_continue;
            }

            /*
             * Perform staging to ensure all kexts are in SIP protected locations.
             *
             * Allow the user to skip staging if a secure location is not required, since
             * the final output check will not let the prelinked kernel be written to a
             * SIP protected location if skip authentication was enabled.
             */
            if (toolArgs->authenticationOptions.requireSecureLocation) {
                theKext = createStagedKext(theKext);
                if (!theKext) {
                    OSKextLog(/* kext */ NULL,
                              kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                              kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                              "%s could not be staged properly; omitting.", kextPath);
                    if (toolArgs->printTestResults) {
                        OSKextLogDiagnostics(theKext, kOSKextDiagnosticsFlagAll);
                    }
                    goto loop_continue;
                }

                // theKext returned by staging must be released, but in normal cases theKext is
                // not owned or released.  Save theKext off to the side so it can be released
                // at the end of the loop.
                ownedKext = theKext;
            }

            // Later security checks would catch kexts in the exclude list and reject them,
            // but since kextcache has the special requirement to post a notification
            // to the user, it continues to check the condition explicitly.
            if (OSKextIsInExcludeList(theKext, true)) {
                addKextToAlertDict(&sExcludedKextAlertDict, theKext);
                messageTraceExcludedKext(theKext);
                OSKextLog(/* kext */ NULL,
                          kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                          kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                          "%s is in the exclude list; omitting.", kextPath);
                if (toolArgs->printTestResults) {
                    OSKextLogDiagnostics(theKext, kOSKextDiagnosticsFlagAll);
                }
                goto loop_continue;
            }

            // Authentication now performs all security checks.
            if (!OSKextIsAuthentic(theKext)) {
                OSKextLog(/* kext */ NULL,
                          kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                          kOSKextLogAuthenticationFlag | kOSKextLogGeneralFlag,
                          "%s does not authenticate; omitting.", kextPath);
                if (toolArgs->printTestResults) {
                    OSKextLogDiagnostics(theKext, kOSKextDiagnosticsFlagAll);
                }
                goto loop_continue;
            }

            // Resolving dependencies ensures authenticated dependencies can be found.
            if (!OSKextResolveDependencies(theKext)) {
                OSKextLog(/* kext */ NULL,
                        kOSKextLogWarningLevel | kOSKextLogArchiveFlag |
                        kOSKextLogDependenciesFlag | kOSKextLogGeneralFlag,
                        "%s is missing dependencies (including anyway; "
                        "dependencies may be available from elsewhere)", kextPath);
                if (toolArgs->printTestResults) {
                    OSKextLogDiagnostics(theKext, kOSKextDiagnosticsFlagAll);
                }
                goto loop_continue;
            }

            if (!CFArrayContainsValue(kextArray, RANGE_ALL(kextArray), theKext)) {
                CFArrayAppendValue(kextArray, theKext);
            }

        loop_continue:
            SAFE_RELEASE(ownedKext);
        } // for loop...
    } // count > 0

#if HAVE_DANGERZONE
    // First, check the volume root url to determine if this kextcache is intended for the
    // currently running system. We will not notify DZ for caches generated for other systems.
    if (isRootVolURL(toolArgs->volumeRootURL)) {
        CFArrayRef loadList = OSKextCopyLoadListForKexts(kextArray, false);

        // First, record all kexts that wound up in the load list because they will all be in the cache.
        count = CFArrayGetCount(loadList);
        for (i = count - 1; i >= 0; i--) {
            OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(loadList, i);
            dzRecordKextCacheAdd(theKext, true);
        }

        // Second, look for any kexts in the initial first pass array that didn't end up in the load
        // list.  Each of those are kexts that were rejected for some reason and were not allowed.
        count = CFArrayGetCount(firstPassArray);
        CFRange loadListRangeAll = RANGE_ALL(loadList);
        for (i = count - 1; i >= 0; i--) {
            OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(firstPassArray, i);
            if (!CFArrayContainsValue(loadList, loadListRangeAll, theKext)) {
                dzRecordKextCacheAdd(theKext, false);
            }
        }
    }
#endif // HAVE_DANGERZONE

    if (CFArrayGetCount(kextArray)) {
        if (earlyBoot == false) {
            recordKextLoadListForMT(kextArray, false);
        }
    }

    result = EX_OK;

finish:
   return result;
}


/* Append the kext if it is the newest bundle / version.  Remove older if found.
 */
static void _appendIfNewest(CFMutableArrayRef theArray, OSKextRef theKext)
{
    CFStringRef     theBundleID;            // do not release
    CFStringRef     theBundleVersion;       // do not release
    OSKextVersion   theKextVersion = -1;
    CFIndex         myCount, i;

    theBundleID = OSKextGetIdentifier(theKext);
    theBundleVersion = OSKextGetValueForInfoDictionaryKey(
                                                          theKext,
                                                          kCFBundleVersionKey );
    if (theBundleVersion == NULL) {
        return;
    }
    theKextVersion = OSKextParseVersionCFString(theBundleVersion);
    if (theKextVersion == -1) {
        return;
    }

    myCount = CFArrayGetCount(theArray);
    for (i = 0; i < myCount; i++) {
        OSKextRef       myKext;             // do not release
        CFStringRef     myBundleID;         // do not release
        CFStringRef     myBundleVersion;    // do not release
        OSKextVersion   myKextVersion = -1;

        myKext = (OSKextRef) CFArrayGetValueAtIndex(theArray, i);
        myBundleID = OSKextGetIdentifier(myKext);

        if ( CFStringCompare(myBundleID, theBundleID, 0) == kCFCompareEqualTo ) {
            myBundleVersion = OSKextGetValueForInfoDictionaryKey(
                                                                 myKext,
                                                                 kCFBundleVersionKey );
            if (myBundleVersion == NULL)  continue;
            myKextVersion = OSKextParseVersionCFString(myBundleVersion);
            if (myKextVersion > 0 && myKextVersion > theKextVersion ) {
                // already have newer version of this kext, do not add it
                OSKextLogCFString(NULL,
                                  kOSKextLogDebugLevel | kOSKextLogArchiveFlag,
                                  CFSTR("%s: found newer, skipping %@"),
                                  __func__, theKext);
                return;
            }
            if (myKextVersion > 0 && myKextVersion == theKextVersion ) {
                // already have same version of this kext, do not add it
                OSKextLogCFString(NULL,
                                  kOSKextLogDebugLevel | kOSKextLogArchiveFlag,
                                  CFSTR("%s: found dup, skipping %@"),
                                  __func__, theKext);
                return;
            }
            if (myKextVersion > 0 && myKextVersion < theKextVersion ) {
                // found older version of this kext, remove it and add this one
                OSKextLogCFString(NULL,
                                  kOSKextLogDebugLevel | kOSKextLogArchiveFlag,
                                  CFSTR("%s: found older, removing %@"),
                                  __func__, myKext);
                CFArrayRemoveValueAtIndex(theArray, i);
                break;
            }
        }
    }

    CFArrayAppendValue(theArray, theKext);
    return;
}

/* We only want to check code signatures for volumes running 10.9 or
 * later version of OS (which means a Kernelcache v1.3 or later)
 */
static Boolean isValidKextSigningTargetVolume(CFURLRef theVolRootURL)
{
    Boolean             myResult          = true;   // default to enforcement
    char                volRoot[PATH_MAX];
    struct bootCaches  *caches            = NULL;   // release
    CFDictionaryRef     myDict            = NULL;   // do not release
    CFDictionaryRef     postBootPathsDict = NULL;   // do not release

    // For safer behavior since theVolRootURL is attacker controlled,
    // assume we want kext signing on the target volume for any errors
    // until we actually find a bootcache.

    if (theVolRootURL) {
        if (!CFURLGetFileSystemRepresentation(theVolRootURL, /* resolve */ true,
                                              (UInt8*)volRoot, sizeof(volRoot))) {
            OSKextLogStringError(NULL);
            goto finish;
        }
    } else {
        // A NULL volume root url implies the system root.
        strlcpy(volRoot, "/", PATH_MAX);
    }

    caches = readBootCaches(volRoot, kBROptsNone);
    if (!caches)                goto finish;
    myDict = caches->cacheinfo;
    if (myDict) {
        // At this point, we will trust the bootcache that was read to allow disabling
        // codesigning.  This still represents a security hazard, since an attacker
        // could easily point to a fake bootcaches that disables signing.  There is a
        // safety net in the prelinked kernel generation path that ensures kext signing
        // is enabled if the output prelinked kernel is in a SIP protected location.
        myResult = false;

        postBootPathsDict = (CFDictionaryRef)
        CFDictionaryGetValue(myDict, kBCPostBootKey);

        if (postBootPathsDict &&
            CFGetTypeID(postBootPathsDict) == CFDictionaryGetTypeID()) {

            if (CFDictionaryContainsKey(postBootPathsDict, kBCKernelcacheV5Key)) {
                myResult = true;
            } else if (CFDictionaryContainsKey(postBootPathsDict, kBCKernelcacheV4Key)) {
                myResult = true;
            } else if (CFDictionaryContainsKey(postBootPathsDict, kBCKernelcacheV3Key)) {
                myResult = true;
            }
        }
    }

finish:
    if (caches)     destroyCaches(caches);

    return(myResult);
}

/*
 * Check to make sure the target prelinkedkernel file is indeed a
 * prelinkedkernel file.
 */
static bool isValidPLKFile(KextcacheArgs *toolArgs)
{
    bool                myResult            = false;
    CFDataRef           plkRef              = NULL;  // must release
    CFDataRef           uncompressed_plkRef = NULL;  // must release
    const UInt8 *       machoHeader         = NULL;
    const NXArchInfo *  archInfo            = NULL;

    void *              prelinkInfoSect     = NULL;

    while (true) {
        if (CFArrayGetCount(toolArgs->targetArchs) == 0) {
            break;
        }
        archInfo = CFArrayGetValueAtIndex(toolArgs->targetArchs, 0);
        if (archInfo == NULL) {
            break;
        }

        plkRef = readMachOSliceForArchWith_fd(toolArgs->prelinkedKernel_fd,
                                              archInfo,
                                              TRUE);
        if (plkRef == NULL) {
            break;
        }

        if (MAGIC32(CFDataGetBytePtr(plkRef)) == OSSwapHostToBigInt32('comp')) {
            uncompressed_plkRef = uncompressPrelinkedSlice(plkRef);
            if (uncompressed_plkRef == NULL) {
                break;
            }
        } else {
            uncompressed_plkRef = CFRetain(plkRef);
        }

        machoHeader = CFDataGetBytePtr(uncompressed_plkRef);
        if (ISMACHO64(MAGIC32(machoHeader))) {
            prelinkInfoSect = (void *)
            macho_get_section_by_name_64((struct mach_header_64 *)machoHeader,
                                         kPrelinkInfoSegment,
                                         kPrelinkInfoSection);
        } else {
            prelinkInfoSect = (void *)
            macho_get_section_by_name((struct mach_header *)machoHeader,
                                      kPrelinkInfoSegment,
                                      kPrelinkInfoSection);
        }
        if (prelinkInfoSect == NULL) {
            break;
        }
        myResult = true;
        break;
    } // while ...

    SAFE_RELEASE(plkRef);
    SAFE_RELEASE(uncompressed_plkRef);

    return(myResult);
}

/*
 * Ensure that if we are creating a prelinked kernel on a SIP-protected
 * volume, then the kernel comes from a valid location: /System/Library/Kernels/...
 */
static bool isSystemKernelPath(KextcacheArgs *toolArgs)
{
    char *kpath = "/System/Library/Kernels/kernel";
    size_t kpath_len = 0;
    struct bootCaches *bc = NULL;
    char volRootPath[PATH_MAX] = { 0, };

    if (CFURLGetFileSystemRepresentation(toolArgs->volumeRootURL, true,
                                         (UInt8 *)volRootPath, PATH_MAX)) {
        bc = readBootCaches(volRootPath, 0);
    }
    if (bc) {
        kpath = &bc->kernelpath[0];
    }

    kpath_len = strnlen(kpath, PATH_MAX);

    bool isvalid = (strncmp(kpath, toolArgs->kernelPath, kpath_len) == 0);

    if (bc) {
        destroyCaches(bc);
    }

    return isvalid;
}


/*
 * Ensure that we write PLK files _only_ to a valid PLK location
 * See: 29149883
 */
static bool isSystemPLKPath(KextcacheArgs *toolArgs)
{
    CFDictionaryRef bcDict = NULL; /* must release */
    CFDictionaryRef postBootPathsDict = NULL; /* do not release */
    CFDictionaryRef kcDict = NULL; /* do not release */
    CFStringRef tmpStr;
    char plkpath[PATH_MAX] = { 0, };
    size_t plkpath_len = 0;

    /* grab the PLK paths from /usr/standalone/bootcaches.plist */
    bcDict = copyBootCachesDictForURL(toolArgs->volumeRootURL);
    if (bcDict != NULL) {
        postBootPathsDict = (CFDictionaryRef)CFDictionaryGetValue(bcDict, kBCPostBootKey);

        /* this doesn't scale well if we do it every year... :/ */
        if (postBootPathsDict &&
            CFGetTypeID(postBootPathsDict) == CFDictionaryGetTypeID()) {
            kcDict = (CFDictionaryRef)CFDictionaryGetValue(postBootPathsDict,
                                                           kBCKernelcacheV5Key);
            if (!kcDict) {
                kcDict = (CFDictionaryRef)CFDictionaryGetValue(postBootPathsDict,
                                                               kBCKernelcacheV4Key);
            }
            if (!kcDict) {
                kcDict = (CFDictionaryRef)CFDictionaryGetValue(postBootPathsDict,
                                                               kBCKernelcacheV3Key);
            }
        }
    }

    if (!kcDict || CFGetTypeID(kcDict) != CFDictionaryGetTypeID()) {
        /* no PLK information in bootcaches.plist: use a hard-coded default */
        goto use_default_path;
    }

    tmpStr = (CFStringRef)CFDictionaryGetValue(kcDict, kBCPathKey);
    if (tmpStr == NULL || CFGetTypeID(tmpStr) != CFStringGetTypeID()) {
        /* no 'Path' key in the kernel cache dictionary?! */
        goto use_default_path;
    }

    if (!CFStringGetFileSystemRepresentation(tmpStr, plkpath, PATH_MAX)) {
        goto use_default_path;
    }

    goto compare_paths;

use_default_path:
    strlcpy(plkpath, _kOSKextTemporaryPrelinkedKernelsPath "/" _kOSKextPrelinkedKernelFileName, PATH_MAX);

compare_paths:
    plkpath_len = strnlen(plkpath, PATH_MAX);

    /*
     * Make sure the PLK path passed to this tool at least starts
     * with a valid PLK path. This will successfully match if the
     * bootcaches path is: /System/Library/PrelinkedKernels/prelinkedkernel and the
     * tool has been passed: /System/Library/PrelinkedKernels/prelinkedkernel.development.
     * That's exactly what we want.
     */
    if (strncmp(plkpath, toolArgs->prelinkedKernelPath, plkpath_len) == 0) {
        SAFE_RELEASE(bcDict);
        return true;
    }

    SAFE_RELEASE(bcDict);
    return false;
}

static bool isProtectedPLK(int prelinkedKernel_fd)
{
    bool sip_enabled = csr_check(CSR_ALLOW_UNRESTRICTED_FS) != 0;
    bool protected_fd = rootless_check_trusted_fd(prelinkedKernel_fd) == 0;

    return sip_enabled && protected_fd;
}

static bool isProbablyProtectedPLK(KextcacheArgs *toolArgs)
{
    /* Note that this is simply a best effort check since it will be based
     * off a static path.  Should only used for non-security critical
     * decisions, with any actual security checks performed after a file
     * descriptor has been created using isProtectedPLK.
     */
    bool sip_enabled = csr_check(CSR_ALLOW_UNRESTRICTED_FS) != 0;

    if (toolArgs->prelinkedKernelPath == NULL) {
        // Since this is called unconditionally during setup, an empty path
        // indicates this isn't going to do any real work (ex. it will just
        // be spinning off children to do work), so assume unprotected.
        return false;
    }

    if (sip_enabled) {
        bool file_exists = access(toolArgs->prelinkedKernelPath, F_OK) == 0;
        if (file_exists) {
            // If the file exists, use rootless to check if it's protected.
            return rootless_check_trusted(toolArgs->prelinkedKernelPath) == 0;
        } else {
            char *parent_directory = dirname(toolArgs->prelinkedKernelPath);
            if (!parent_directory) {
                return false;
            }
            return rootless_check_trusted(parent_directory) == 0;
        }
    }

    // If SIP is disabled, it's never protected.
    return false;
}

static bool isProtectedAction(KextcacheArgs *toolArgs)
{
    Boolean targetingBootVol = false;

    // isRootVolURL assumes a NULL URL implies root volume, so only assume it means
    // targeting the boot volume when there is a real URL.
    if (toolArgs->updateVolumeURL) {
        targetingBootVol = isRootVolURL(toolArgs->updateVolumeURL);
    }

    return (isProbablyProtectedPLK(toolArgs) ||
            toolArgs->updateSystemCaches ||
            targetingBootVol);
}

static bool isSecureAuthentication(KextcacheArgs *toolArgs)
{
    bool sip_enabled = csr_check(CSR_ALLOW_UNRESTRICTED_FS) != 0;

    return (toolArgs->authenticationOptions.performFilesystemValidation &&
            toolArgs->authenticationOptions.performSignatureValidation &&
            toolArgs->authenticationOptions.requireSecureLocation &&
            toolArgs->authenticationOptions.respectSystemPolicy) || !sip_enabled;
}

/* Make sure target volume can support fast (lzvn) compression, as well as current runtime library environment */

static Boolean wantsFastLibCompressionForTargetVolume(CFURLRef theVolRootURL)
{
    Boolean             myResult          = false;
    char                volRoot[PATH_MAX];
    struct bootCaches  *caches            = NULL;   // release
    CFDictionaryRef     myDict            = NULL;   // do not release
    CFDictionaryRef     postBootPathsDict = NULL;   // do not release
    CFDictionaryRef     kernelCacheDict   = NULL;   // do not release

    if (theVolRootURL) {
        if (!CFURLGetFileSystemRepresentation(theVolRootURL, /* resolve */ true,
                                              (UInt8*)volRoot, sizeof(volRoot))) {
            OSKextLogStringError(NULL);
            goto finish;
        }
    } else {
        // A NULL volume root url implies the system root.
        strlcpy(volRoot, "/", PATH_MAX);
    }
    caches = readBootCaches(volRoot, kBROptsNone);
    if (!caches)                goto finish;
    myDict = caches->cacheinfo;
    if (myDict) {
        postBootPathsDict = (CFDictionaryRef)
            CFDictionaryGetValue(myDict, kBCPostBootKey);

        if (postBootPathsDict &&
            CFGetTypeID(postBootPathsDict) == CFDictionaryGetTypeID()) {

            kernelCacheDict = (CFDictionaryRef)
                CFDictionaryGetValue(postBootPathsDict, kBCKernelcacheV5Key);
            if (!kernelCacheDict) {
                kernelCacheDict = (CFDictionaryRef)
                    CFDictionaryGetValue(postBootPathsDict, kBCKernelcacheV4Key);
            }
            if (!kernelCacheDict) {
                kernelCacheDict = (CFDictionaryRef)
                    CFDictionaryGetValue(postBootPathsDict, kBCKernelcacheV3Key);
            }

            if (kernelCacheDict &&
                CFGetTypeID(kernelCacheDict) == CFDictionaryGetTypeID()) {
                CFStringRef     myTempStr;      // do not release

                myTempStr = (CFStringRef)
                CFDictionaryGetValue(kernelCacheDict,
                                     kBCPreferredCompressionKey);

                if (myTempStr && CFGetTypeID(myTempStr) == CFStringGetTypeID()) {
                    if (CFStringCompare(myTempStr, CFSTR("lzvn"), 0) == kCFCompareEqualTo) {
                        myResult = true;
                    }
                }
            } // kernelCacheDict
        } // postBootPathsDict
    } // myDict

    /* We may not be able to generate FastLib-compressed files */
    if (myResult && !supportsFastLibCompression()) {
        myResult = false;
    }

finish:
    if (caches)     destroyCaches(caches);

    return(myResult);
}

/*******************************************************************************
*******************************************************************************/
Boolean
kextMatchesFilter(
    KextcacheArgs             * toolArgs,
    OSKextRef                   theKext,
    OSKextRequiredFlags         requiredFlags)
{
    Boolean result = false;
    Boolean needLoadedKextInfo = toolArgs->needLoadedKextInfo &&
        (OSKextGetArchitecture() == OSKextGetRunningKernelArchitecture());

    if (needLoadedKextInfo) {
        result = (requiredFlags && OSKextMatchesRequiredFlags(theKext, requiredFlags)) ||
            CFArrayContainsValue(toolArgs->loadedKexts, RANGE_ALL(toolArgs->loadedKexts), theKext);
    } else {
        result = OSKextMatchesRequiredFlags(theKext, requiredFlags);
    }

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
    KextcacheArgs     * toolArgs,
    CFMutableArrayRef * prelinkArchsOut)
{
    ExitStatus          result          = EX_OSERR;
    CFMutableArrayRef   kernelArchs     = NULL;  // must release
    CFMutableArrayRef   prelinkArchs    = NULL;  // must release
    const NXArchInfo  * targetArch      = NULL;  // do not free
    u_int               i               = 0;

    result = readFatFileArchsWith_fd(toolArgs->kernel_fd, &kernelArchs);
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
                "Kernel file %s does not contain requested arch: %s",
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
 * If the existing prelinked kernel has a valid timestamp, this reads the slices
 * out of that prelinked kernel so we don't have to regenerate them.
 *******************************************************************************/
ExitStatus
createExistingPrelinkedSlices(
    KextcacheArgs     * toolArgs,
    CFMutableArrayRef * existingSlicesOut,
    CFMutableArrayRef * existingArchsOut)
{
    struct timeval      existingFileTimes[2];
    struct timeval      prelinkFileTimes[2];
    ExitStatus          result  = EX_SOFTWARE;

   /* If we aren't updating the system prelinked kernel, then we don't want
    * to reuse any existing slices.
    */
    if (!toolArgs->needDefaultPrelinkedKernelInfo) {
        result = EX_OK;
        goto finish;
    }

    bzero(&existingFileTimes, sizeof(existingFileTimes));
    bzero(&prelinkFileTimes, sizeof(prelinkFileTimes));

    result = getFileDescriptorTimes(toolArgs->prelinkedKernel_fd,
                                    existingFileTimes);
    if (result != EX_OK) {
        goto finish;
    }

    result = getExpectedPrelinkedKernelModTime(toolArgs,
                                               prelinkFileTimes, NULL);
    if (result != EX_OK) {
        goto finish;
    }

    /* We are testing that the existing prelinked kernel still has a valid
     * timestamp by comparing it to the timestamp we are going to use for
     * the new prelinked kernel. If they are equal, we can reuse slices
     * from the existing prelinked kernel.
     */
    if (!timevalcmp(&existingFileTimes[1], &prelinkFileTimes[1], ==)) {
        result = EX_SOFTWARE;
        goto finish;
    }

    result = readMachOSlicesWith_fd(toolArgs->prelinkedKernel_fd,
                                    existingSlicesOut,
                                    existingArchsOut, NULL, NULL);
    if (result != EX_OK) {
        existingSlicesOut = NULL;
        existingArchsOut = NULL;
        goto finish;
    }

    result = EX_OK;

finish:
    return result;
}


/*******************************************************************************
*******************************************************************************/
ExitStatus
createPrelinkedKernel(
    KextcacheArgs     * toolArgs)
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
    int                 plk_result          = 0;
    ino_t               plk_ino_t           = 0;
    dev_t               plk_dev_t           = 0;
    size_t              plk_size_t          = 0;
    ino_t               kern_ino_t          = 0;
    dev_t               kern_dev_t          = 0;
    bool                created_plk         = false;
    char               *plk_filename        = NULL;
    os_signpost_id_t    spid                = generate_signpost_id();
    bool                deferred_update     = false;

    os_signpost_interval_begin(get_signpost_log(), spid, SIGNPOST_KEXTCACHE_BUILD_PRELINKED_KERNEL);
    bzero(&prelinkFileTimes, sizeof(prelinkFileTimes));

    plk_filename = toolArgs->prelinkedKernelPath + strnlen(toolArgs->prelinkedKernelDirname, PATH_MAX);
    while (*plk_filename == '/') plk_filename++;

    os_signpost_event_emit(get_signpost_log(), spid, SIGNPOST_EVENT_PRELINKED_KERNEL_PATH,
                           "%s", toolArgs->prelinkedKernelPath);
    toolArgs->prelinkedKernelDir_fd = open(toolArgs->prelinkedKernelDirname,
                                           O_RDONLY | O_DIRECTORY);
    if (toolArgs->prelinkedKernelDir_fd < 0) {
        result = EX_NOPERM;
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                  "Prelinked kernel directory '%s/' cannot be used",
                  toolArgs->prelinkedKernelDirname);
        goto finish;
    }

    /*
     * Do not allow an untrusted kernel file if:
     * 1) file system is restricted, and
     * 2) the prelinkedkernel we are about to replace is restricted.
     *    18862985, 20349389, 20693294
     * OR
     * 3) we are replacing the immutable kernel
     */
    toolArgs->prelinkedKernel_fd = openat(toolArgs->prelinkedKernelDir_fd,
                                          plk_filename, O_RDONLY | O_SYMLINK);
    if (toolArgs->prelinkedKernel_fd != -1) {
        plk_result = getFileDevAndInoAndSizeWith_fd(toolArgs->prelinkedKernel_fd, &plk_dev_t, &plk_ino_t, &plk_size_t);
        if (plk_result == 0) {
            /* make sure this is an existing PLK file - <rdar://problem/25323859>
             * empty files are okay though, in case we get interrupted after calling openat with O_CREAT */
            if (plk_size_t != 0 && !isValidPLKFile(toolArgs)) {
                plk_result = -1; // force error
            }
        }
    }


    /*
     * It's possible that we got interrupted last time while trying to create a PLK.
     * Make an exception for empty files and pretend that everything's fine.
     */
    if (toolArgs->prelinkedKernel_fd != -1 && plk_result == 0 && plk_size_t == 0) {
        created_plk = true;
    } else if (toolArgs->prelinkedKernel_fd == -1 && errno == ENOENT) {
        /*
         * The prelinked kernel file doesn't exist, or is empty.
         * Create the file now so we can validate that this is exactly the location
         * we will write to by checking the dev and ino on the open FD. This should
         * close any races with malicious users attempting to play symlink games.
         * NOTE: previous validation should have already run realpath().
         * NOTE: we need to remove this file if any SIP validation fails
         */
        toolArgs->prelinkedKernel_fd = openat(toolArgs->prelinkedKernelDir_fd,
                                              plk_filename, O_RDWR | O_CREAT | O_EXCL);
        if (toolArgs->prelinkedKernel_fd != -1) {
            created_plk = true;
            plk_result = getFileDevAndInoWith_fd(toolArgs->prelinkedKernel_fd, &plk_dev_t, &plk_ino_t);
        }
    }

    if ((toolArgs->prelinkedKernel_fd == -1 && errno != ENOENT) ||
        plk_result == -1) {
        result = EX_NOPERM;
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                  "Bad prelinkedkernel path '%s/%s' cannot be used",
                  toolArgs->prelinkedKernelDirname, plk_filename);
        goto finish;
    }

    toolArgs->kernel_fd = open(toolArgs->kernelPath, O_RDONLY);
    if (toolArgs->kernel_fd < 0 ||
        getFileDevAndInoWith_fd(toolArgs->kernel_fd, &kern_dev_t, &kern_ino_t) != 0) {
        result = EX_NOPERM;
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                  "Bad kernel path '%s' cannot be used",
                  toolArgs->kernelPath);
        goto finish;
    }

    /*
     * Is the target prelinked kernel file in a SIP protected location?
     * 23382956 - use rootless_check_trusted_fd
     */
    if (isProtectedPLK(toolArgs->prelinkedKernel_fd)) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogDebugLevel  | kOSKextLogGeneralFlag,
                  "Creating SIP-protected prelinked kernel: %s",
                  toolArgs->prelinkedKernelPath);

        /*
         * Never allow a SIP-protected prelinked kernel to be created that
         * bypasses full authentication in any way, either via the flags
         * or with tricks based on the prelinked kernel path.
         */
        if (toolArgs->skipAuthentication ||
            !toolArgs->authenticationOptions.respectSystemPolicy ||
            !toolArgs->authenticationOptions.requireSecureLocation) {
            result = EX_NOPERM;
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                      "Invalid to create a prelinked kernel without authentication at path '%s'",
                      toolArgs->prelinkedKernelPath);
            goto finish;
        }

        /*
         * Never allow a SIP-protected prelinked kernel to be created that
         * does not enforce kext signing.  Since this is derived from the volume root,
         * we call it out specifically.  Other authentication issues would be caught above.
         */
        if (!toolArgs->authenticationOptions.performSignatureValidation) {
            result = EX_NOPERM;
            OSKextLogCFString(/* kext */ NULL,
                              kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                              CFSTR("Invalid to create a prelinked kernel without signature enforcement on volume: %@"),
                              toolArgs->volumeRootURL);
            goto finish;
        }

        /*
         * Validate that when we create a prelinked kernel on a SIP protected
         * volume in a SIP protected location, we only create a SIP protected
         * file in a valid prelinked kernel path (/System/Library/PrelinkedKernels)
         */
        if (!isSystemPLKPath(toolArgs)) {
            result = EX_NOPERM;
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                      "Invalid path '%s': SIP-protected prelinked kernels must be in /System/Library/PrelinkedKernels",
                      toolArgs->prelinkedKernelPath);
            goto finish;
        }
        /*
         * The kernel must be trusted if we are writing to an existing
         * protected PLK, or if we are writing to a PLK that doesn't yet exist
         * but whose path is in a trusted location.
         */
        if (rootless_check_trusted_fd(toolArgs->kernel_fd) != 0) {
            result = EX_NOPERM;
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                      "Untrusted kernel '%s' cannot be used to create a SIP-protected prelinked kernel",
                      toolArgs->kernelPath);
            goto finish;
        }
        /*
         * A trusted kernel used to build a trusted prelinked kernel must
         * come from a trusted path location: /System/Library/Kernels/kernel
         */
        if (!isSystemKernelPath(toolArgs)) {
            result = EX_NOPERM;
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                      "Invalid kernel path '%s': only kernels from /System/Library/Kernels can be used when writing SIP-protected prelinked kernels",
                      toolArgs->kernelPath);
            goto finish;
        }
    } else {
        /*
         * Allow building of non-SIP protected immutable kernel, but emit an
         * extra warning.
         */
        if (toolArgs->buildImmutableKernel) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogWarningLevel  | kOSKextLogGeneralFlag,
                      "WARNING: building immutablekernel in a non-SIP protected path '%s'",
                      toolArgs->prelinkedKernelPath);
        }
        OSKextLog(/* kext */ NULL,
                  kOSKextLogDebugLevel  | kOSKextLogGeneralFlag,
                  "Creating unprotected prelinked kernel: %s",
                  toolArgs->prelinkedKernelPath);
    }

#if !NO_BOOT_ROOT
    /* Try a lock on the volume for the prelinked kernel being updated.
     * The lock prevents kextd from starting up a competing kextcache.
     */
    // xxx - updateBoots * related should return only sysexit-type values, not errno
    result = takeVolumeForPath(toolArgs->prelinkedKernelPath);
    if (result != EX_OK) {
        goto finish;
    }
#endif /* !NO_BOOT_ROOT */

    result = createPrelinkedKernelArchs(toolArgs, &prelinkArchs);
    if (result != EX_OK) {
        goto finish;
    }
    numArchs = (u_int)CFArrayGetCount(prelinkArchs);

    /* If we're generating symbols, we'll regenerate all slices.
     */
    if (!toolArgs->symbolDirURL) {
        result = createExistingPrelinkedSlices(toolArgs,
            &existingSlices, &existingArchs);
        if (result != EX_OK) {
            SAFE_RELEASE_NULL(existingSlices);
            SAFE_RELEASE_NULL(existingArchs);
        }
    }

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

        CFArrayAppendValue(prelinkSlices, prelinkSlice);
        CFArrayAppendValue(generatedSymbols, sliceSymbols);
        CFArrayAppendValue(generatedArchs, targetArch);
    }

    result = getExpectedPrelinkedKernelModTime(toolArgs,
                                               prelinkFileTimes, &updateModTime);
    if (result != EX_OK) {
        goto finish;
    }

    /* check to make sure kernel used is still the same */
    if (isSameFileDevAndInoWith_fd(toolArgs->kernel_fd,
                                   kern_dev_t,
                                   kern_ino_t) == FALSE) {
            OSKextLog(NULL,
                      kOSKextLogErrorLevel  | kOSKextLogGeneralFlag,
                      "File at path '%s' changed, cannot be used",
                      toolArgs->kernelPath);
            result = EX_NOPERM;
            goto finish;
    }

#if 0
    OSKextLogCFString(NULL,
                      kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                      CFSTR("%s: writing to %s"),
                      __func__, toolArgs->prelinkedKernelPath);
    if (updateModTime) {
        OSKextLogCFString(NULL,
                          kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                          CFSTR("%s: setting mod time to %ld"),
                          __func__, prelinkFileTimes[1].tv_sec);
    }
#endif

    result = writeFatFileWithValidation(toolArgs->prelinkedKernelPath,
                                        TRUE,
                                        plk_dev_t,
                                        plk_ino_t,
                                        prelinkSlices,
                                        prelinkArchs,
                                        PRELINK_KERNEL_PERMS,
                                        (updateModTime) ? prelinkFileTimes : NULL);
    if (result != EX_OK) {
        goto finish;
    }

    /*
     * the PLK that we wrote at 'toolArgs->prelinkedKernelPath' is now valid,
     * and should not be removed on subsequent errors.
     */
    created_plk = false;

    if (toolArgs->symbolDirURL) {
        result = writePrelinkedSymbols(toolArgs->symbolDirURL,
                                       generatedSymbols, generatedArchs);
        if (result != EX_OK) {
            goto finish;
        }
    }

    OSKextLog(/* kext */ NULL,
              kOSKextLogGeneralFlag | kOSKextLogBasicLevel,
              "Created prelinked kernel \"%s\"",
              toolArgs->prelinkedKernelPath);
    if (toolArgs->kernelPath) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogGeneralFlag | kOSKextLogBasicLevel,
                  "Created prelinked kernel using \"%s\"",
                  toolArgs->kernelPath);
    }

    if (toolArgs->buildImmutableKernel) {
        result = buildImmutableKernelcache(toolArgs, plk_filename);
        if (result != EX_OK) {
            goto finish;
        }
    }

    removeStalePrelinkedKernels(toolArgs);

    /*
     * "requiresDeferredUpdate" checks if this prelinked kernel destination
     * needs a two-step update, where kextcache will initially write the
     * prelinked kernel into a staging directory, and then move it into
     * its final location once it's finished with the build.
     *
     * We check to see if we're booted on a machine that has the ROSV enabled
     * by checking that the dev_t of the plk resides on a volume that is
     * mounted at the mountpoint "/System/Volumes/Data". If that's the case,
     * then we write out a small shell script that launchd will execute at
     * shutdown to manage the final staging step.
     */
    if (requiresDeferredUpdate(toolArgs->prelinkedKernelDirname)) {
        deferred_update = true;
        char mntname[MNAMELEN];
        /* If findmnt fails and we require deferred update, then nothing will happen. */
        if (findmnt(plk_dev_t, mntname, false) == 0) {
            if (strcmp(mntname, _kOSKextReadOnlyDataVolumePath) == 0) {
                createDeferredUpdateScript();
            } else {
                char volRootPath[PATH_MAX] = {};
                if (!CFURLGetFileSystemRepresentation(toolArgs->volumeRootURL,
                            /* resolveToBase */ true, (UInt8 *)volRootPath, sizeof(volRootPath))) {
                    OSKextLog(/* kext */ NULL,
                              kOSKextLogFileAccessFlag | kOSKextLogErrorLevel,
                              "Error getting volume root path.");
                    goto finish;
                }
                int copyres = copyKernelsInVolume(volRootPath);
                if (copyres != EX_OK) {
                    OSKextLog(/* kext */ NULL,
                              kOSKextLogFileAccessFlag | kOSKextLogErrorLevel,
                              "Error copying prelinked kernel: %d",
                              copyres);
                    /* silently fail */
                }
            }
        }
    }

    /*
     * <rdar://problem/24330917> need a symlink using the old kernelcache name
     * so Startup Disk on older systems can still work.  Startup Disk
     * running on systems before 10.12 have a bug that will not allow them to
     * find the prelinkedkernel files on 10.12 or later systems so we create
     * this symlink as a workaround.
     * 10.12 and later -> /System/Library/PrelinkedKernels/prelinkedkernel
     * 10.11 has both -> /System/Library/Caches/com.apple.kext.caches/Startup/kernelcache
     *                  and prelinkedkernel
     */
    char *sptr;
    sptr = strnstr(toolArgs->prelinkedKernelPath, _kOSKextPrelinkedKernelsPath, strlen(toolArgs->prelinkedKernelPath));
    if (sptr && !deferred_update) {
        long        prefixSize = 0;
        char        tmpBuffer[PATH_MAX];
        char        plkRelPath[PATH_MAX];

        /*
         * None of this is fatal if it fails...
         */

        prefixSize = sptr - toolArgs->prelinkedKernelPath;
        if (prefixSize >= sizeof(tmpBuffer))
            goto finish;

        /* copy the {volume} prefix, e.g., "/Volumes/Some HD" */
        strlcpy(tmpBuffer, toolArgs->prelinkedKernelPath, prefixSize + 1);

        /*
         * Copy the old-style kernelcache name at the old location.
         * The path should now look something like:
         * /Volumes/Some HD/System/Library/Caches/com.apple.kext.caches/Startup/kernelcache
         */
        PATHCAT(tmpBuffer, _kOSKextCachesRootFolder "/Startup/kernelcache", dontlink);

        /* locate the "prelinkedkernel" in the path */
        sptr = strnstr(toolArgs->prelinkedKernelPath, _kOSKextPrelinkedKernelFileName,
                       strlen(toolArgs->prelinkedKernelPath));
        if (!sptr) {
            goto dontlink;
        }

        /* construct a relative path to the prelinked kernel */
        PATHCPY(plkRelPath, kPLKDirSymlinkPrefix, dontlink);
        PATHCAT(plkRelPath, sptr, dontlink);

        /* add suffix (if any) to the old kernelcache location name */
        if (sptr && strlen(sptr) > strlen(_kOSKextPrelinkedKernelFileName)) {
            PATHCAT(tmpBuffer, sptr + strlen(_kOSKextPrelinkedKernelFileName), dontlink);
        }

        /* strip away any {Volume} prefix (the symlink exists on the same volume!) */
        OSKextLog(NULL, kOSKextLogGeneralFlag | kOSKextLogBasicLevel,
                  "Symlink \"%s\" -> \"%s\"",
                  tmpBuffer, plkRelPath);

        /* force a symlink (this symlink target was incorrect in the past) */
        struct stat sb;
        if (lstat(tmpBuffer, &sb) == 0) {
            if (S_ISLNK(sb.st_mode)) {
                /* remove the old symlink */
                unlink(tmpBuffer);
            }
        }

        if (symlink(plkRelPath, tmpBuffer) < 0) {
            if (errno != EEXIST) {
                OSKextLog(NULL,  kOSKextLogGeneralFlag | kOSKextLogWarningLevel,
                          "symlink(\"%s\", \"%s\") failed %d (%s)",
                          plkRelPath, tmpBuffer,
                          errno, strerror(errno));
            }
        }

        goto finish;

dontlink:
        OSKextLog(NULL, kOSKextLogGeneralFlag | kOSKextLogBasicLevel,
                  "Skipping com.apple.kext.caches symlink to: \"%s\"",
                  toolArgs->prelinkedKernelPath);
    } /* if (sptr), i.e. prelinkedKernelPath contained  _kOSKextPrelinkedKernelsPath */

finish:
    if (isKextdRunning() && isRootVolURL(toolArgs->volumeRootURL)) {
        // <rdar://problem/20688847> only post notifications if kextcache was
        // targeting the root volume
        if (sExcludedKextAlertDict) {
            /* notify kextd that we have some excluded kexts going into the
             * kernel cache.
             */
            postNoteAboutKexts(CFSTR("Excluded Kext Notification"),
                               sExcludedKextAlertDict);
        }
    }
    if (result != EX_OK && created_plk &&
        toolArgs->prelinkedKernel_fd != -1 &&
        toolArgs->prelinkedKernelDir_fd != -1) {
        /*
         * If we fail to build the PLK, and we had to initially create
         * the file, we should remove the empty / invalid PLK.
         */
        if (unlinkat(toolArgs->prelinkedKernelDir_fd, plk_filename, 0) < 0) {
            OSKextLog(NULL, kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                      "Error removing (now invalid) prelinked kernel: %s/%s",
                      toolArgs->prelinkedKernelDirname, plk_filename);
        }
    }
    if (toolArgs->prelinkedKernelDir_fd != -1) {
        close(toolArgs->prelinkedKernelDir_fd);
        toolArgs->prelinkedKernelDir_fd = -1;
    }
    if (toolArgs->prelinkedKernel_fd != -1) {
        close(toolArgs->prelinkedKernel_fd);
        toolArgs->prelinkedKernel_fd = -1;
    }
    if (toolArgs->kernel_fd != -1) {
        close(toolArgs->kernel_fd);
        toolArgs->kernel_fd = -1;
    }
    SAFE_RELEASE(generatedArchs);
    SAFE_RELEASE(generatedSymbols);
    SAFE_RELEASE(existingArchs);
    SAFE_RELEASE(existingSlices);
    SAFE_RELEASE(prelinkArchs);
    SAFE_RELEASE(prelinkSlices);
    SAFE_RELEASE(prelinkSlice);
    SAFE_RELEASE(sliceSymbols);

#if !NO_BOOT_ROOT
    putVolumeForPath(toolArgs->prelinkedKernelPath, result);
#endif /* !NO_BOOT_ROOT */

    os_signpost_event_emit(get_signpost_log(), spid, SIGNPOST_EVENT_RESULT, "%d", result);
    os_signpost_interval_end(get_signpost_log(), spid, SIGNPOST_KEXTCACHE_BUILD_PRELINKED_KERNEL);
    return result;
}

/* NOTE -> Null URL means no /Volumes/XXX prefix was used, also a null string
 * in the URL is also treated as root volume
 */
static Boolean isRootVolURL(CFURLRef theURL)
{
    Boolean     result = false;
    char        volRootBuf[PATH_MAX];
    char        realPathBuf[PATH_MAX];

    if (theURL == NULL) {
        result = true;
        goto finish;
    }

    volRootBuf[0] = 0x00;
    if (CFURLGetFileSystemRepresentation(theURL,
                                         true,
                                         (UInt8 *)volRootBuf,
                                         sizeof(volRootBuf)) == false) {
        // this should not happen, but just in case...
        volRootBuf[0] = 0x00;
    }

    if (volRootBuf[0] == 0x00) {
        // will count a null string also as root volume
        result = true;
    } else {
        // anything other than a null volume url should be resolved via realpath
        if (realpath(volRootBuf, realPathBuf)) {
            if (strlen(realPathBuf) == 1 && realPathBuf[0] == '/') {
                result = true;
            }
        }
    }

finish:
    return(result);

}

/*******************************************************************************
 *******************************************************************************/
ExitStatus createPrelinkedKernelForArch(
    KextcacheArgs       * toolArgs,
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
    Boolean kernelSupportsKASLR = false;
    macho_seek_result machoResult;
    const UInt8 * kernelStart;
    const UInt8 * kernelEnd;

    /* Retrieve the kernel image for the requested architecture.
     */
    kernelImage = readMachOSliceForArchWith_fd(toolArgs->kernel_fd,
                                               archInfo,
                                               /* checkArch */ TRUE);
    if (!kernelImage) {
        OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogArchiveFlag |  kOSKextLogFileAccessFlag,
                "Failed to read kernel file.");
        goto finish;
    }

    /* Set suffix for kext executables from kernel path */
    OSKextSetExecutableSuffix(NULL, toolArgs->kernelPath);

    /* Set current target if there is one */
    if (toolArgs->targetForKextVariants) {
        OSKextSetTargetString(toolArgs->targetForKextVariants);
    }

    /* Set the architecture in the OSKext library */
    if (!OSKextSetArchitecture(archInfo)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't set architecture %s to create prelinked kernel.",
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

    // Only perform GPU bundle staging when building a kextcache for the currently running system.
    if (isRootVolURL(toolArgs->volumeRootURL)) {
        // Iterate all kernel extensions chosen for inclusion and stage any non-SIP protected
        // companion usermode GPU bundles into a SIP protected location.
        for (int i = 0; i < CFArrayGetCount(prelinkKexts); i++) {
            OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(prelinkKexts, i);
            if (needsGPUBundlesStaged(theKext)) {
                // Errors are ignored for GPU bundle staging because it is all best-effort,
                // and nothing is logged here because the functions themselves already log
                // any errors for debugging or awareness.
                stageGPUBundles(theKext);
            }
        }
    }

    result = EX_OSERR;

    if (!CFArrayGetCount(prelinkKexts)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "No kexts found for architecture %s.",
            archInfo->name);
        goto finish;
    }

   /* Create the prelinked kernel from the given kernel and kexts. */
    flags |= (toolArgs->noLinkFailures) ? kOSKextKernelcacheNeedAllFlag : 0;
    flags |= (toolArgs->printTestResults) ? kOSKextKernelcachePrintDiagnosticsFlag : 0;
    flags |= (toolArgs->includeAllPersonalities) ? kOSKextKernelcacheIncludeAllPersonalitiesFlag : 0;
    flags |= (toolArgs->stripSymbols) ? kOSKextKernelcacheStripSymbolsFlag : 0;

    kernelStart = CFDataGetBytePtr(kernelImage);
    kernelEnd = kernelStart + CFDataGetLength(kernelImage) - 1;
    machoResult = macho_find_dysymtab(kernelStart, kernelEnd, NULL);
    /* this kernel supports KASLR if there is a LC_DYSYMTAB load command */
    kernelSupportsKASLR = (machoResult == macho_seek_result_found);
    if (kernelSupportsKASLR) {
        flags |= kOSKextKernelcacheKASLRFlag;
    }

    prelinkedKernel = OSKextCreatePrelinkedKernel(kernelImage, prelinkKexts,
        toolArgs->volumeRootURL, flags, prelinkedSymbolsOut);
    if (!prelinkedKernel) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Failed to generate prelinked kernel.");
        result = EX_OSERR;
        goto finish;
    }

   /* Compress the prelinked kernel if needed */

    if (toolArgs->compress) {
        Boolean     wantsFastLib = wantsFastLibCompressionForTargetVolume(toolArgs->volumeRootURL);
        uint32_t    compressionType = wantsFastLib ? COMP_TYPE_FASTLIB : COMP_TYPE_LZSS;

        *prelinkedKernelOut = compressPrelinkedSlice(compressionType,
                                                     prelinkedKernel,
                                                     kernelSupportsKASLR);
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

/*****************************************************************************
 *****************************************************************************/
ExitStatus
getExpectedPrelinkedKernelModTime(
    KextcacheArgs  * toolArgs,
    struct timeval   cacheFileTimes[2],
    Boolean        * updateModTimeOut)
{
    struct timeval  kextTimes[2];
    struct timeval  kernelTimes[2];
    ExitStatus      result          = EX_SOFTWARE;
    Boolean         updateModTime   = false;

    /* bail out if we don't have modtimes for extensions directory or kernel file
     */
    if (toolArgs->extensionsDirTimes[1].tv_sec == 0 ||
        toolArgs->kernelTimes[1].tv_sec == 0) {
        result = EX_OK;
        goto finish;
    }

    result = getLatestTimesFromCFURLArray(toolArgs->repositoryURLs,
                                          kextTimes);
    if (result != EX_OK) {
        goto finish;
    }

    /* bump kexts modtime by 1 second */
    kextTimes[1].tv_sec++;

    /* Check kernel mod time */
    result = getFileDescriptorModTimePlusOne(toolArgs->kernel_fd,
                                             &toolArgs->kernelTimes[1],
                                             kernelTimes);
    if (result != EX_OK) {
        goto finish;
    }
#if 0
    OSKextLogCFString(NULL,
                      kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                      CFSTR("%s: kernelPath %s"),
                      __func__, toolArgs->kernelPath);
    OSKextLogCFString(NULL,
                      kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                      CFSTR("%s: %ld <- latest kext mod time"),
                      __func__, kextTimes[1].tv_sec);
    OSKextLogCFString(NULL,
                      kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                      CFSTR("%s: %ld <- latest kernels mod time"),
                      __func__, kernelTimes[1].tv_sec);
#endif

    /* Get the access and mod times of the latest modified of the kernel,
     * or kext repositories.  For example:
     * kextTimes -> /System/Library/Extensions/ and /Library/Extensions/
     * kernelTimes -> /System/Library/Kernels/kernel
     * cacheFileTimes -> /System/Library/PrelinkedKernels/prelinkedkernel
     */
    cacheFileTimes[0].tv_sec = kextTimes[0].tv_sec;     // access time
    cacheFileTimes[0].tv_usec = kextTimes[0].tv_usec;
    cacheFileTimes[1].tv_sec = kextTimes[1].tv_sec;     // mod time
    cacheFileTimes[1].tv_usec = kextTimes[1].tv_usec;
    if (timercmp(&kernelTimes[1], &kextTimes[1], >)) {
        cacheFileTimes[0].tv_sec = kernelTimes[0].tv_sec;   // access time
        cacheFileTimes[0].tv_usec = kernelTimes[0].tv_usec;
        cacheFileTimes[1].tv_sec = kernelTimes[1].tv_sec;   // mod time
        cacheFileTimes[1].tv_usec = kernelTimes[1].tv_usec;
    }
#if 0
    OSKextLogCFString(NULL,
                      kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                      CFSTR("%s: %ld <- using this mod time"),
                      __func__, cacheFileTimes[1].tv_sec);
#endif

    /* Set the mod time of the prelinkedkernel relative to the kernel */
    updateModTime = true;
    result = EX_OK;

finish:
    if (updateModTimeOut) *updateModTimeOut = updateModTime;

    return result;
}

/*********************************************************************
 *********************************************************************/
ExitStatus
compressPrelinkedKernel(
                        CFURLRef            volumeRootURL,
                        const char        * prelinkPath,
                        Boolean             compress)
{
    ExitStatus          result          = EX_SOFTWARE;
    struct timeval      prelinkedKernelTimes[2];
    CFMutableArrayRef   prelinkedSlices = NULL; // must release
    CFMutableArrayRef   prelinkedArchs  = NULL; // must release
    CFDataRef           prelinkedSlice  = NULL; // must release
   const NXArchInfo  * archInfo         = NULL; // do not free
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
            const PrelinkedKernelHeader *header = (const PrelinkedKernelHeader *)
                CFDataGetBytePtr(prelinkedSlice);
            Boolean     wantsFastLib = wantsFastLibCompressionForTargetVolume(volumeRootURL);
            uint32_t    compressionType = wantsFastLib ? COMP_TYPE_FASTLIB : COMP_TYPE_LZSS;


            prelinkedSlice = compressPrelinkedSlice(compressionType,
                                                    prelinkedSlice,
                                                    (OSSwapHostToBigInt32(header->prelinkVersion) == 1));
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

        sliceBytes = CFDataGetBytePtr(
            CFArrayGetValueAtIndex(prelinkedSlices, 0));

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
 * buildImmutableKernelcache
 *
 * We assume that the file 'plk_filename' has been created and
 * validated. This function will create an immutablekernel file at the
 * same file path (toolArgs->prelinkedKernelDir_fd), then invoke the
 * personalize_macos binary.
 *********************************************************************/
static ExitStatus
buildImmutableKernelcache(KextcacheArgs *toolArgs, const char *plk_filename)
{
    ExitStatus result = EX_OK;
    int dirfd = toolArgs->prelinkedKernelDir_fd;
    bool have_backup = false;
    char imk_filename[256] = {};
    char imk_backupname[256] = {};

    // validate toolArgs
    if (dirfd < 0) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                  "Prelinked kernel path \"%s\" no longer seems to be open.",
                  toolArgs->prelinkedKernelDirname);
        result = EX_OSERR;
        goto finish;
    }

    if (!translatePrelinkedToImmutablePath(plk_filename, imk_filename, sizeof(imk_filename))) {
        result = EX_OSERR;
        goto finish;
    }

    OSKextLog(/* kext */ NULL,
              kOSKextLogGeneralFlag | kOSKextLogBasicLevel,
              "Rebuilding immutable kernel: \"%s/%s\"",
              toolArgs->prelinkedKernelDirname, imk_filename);

    if (strlcpy(imk_backupname, imk_filename, sizeof(imk_backupname)) > sizeof(imk_backupname) ||
        strlcat(imk_backupname, ".bak", sizeof(imk_backupname)) > sizeof(imk_backupname)) {
        result = EX_SOFTWARE;
        goto finish;
    }

    // make a backup copy (hardlink) of the immutable kernel
    // NOTE: this will overwrite any existing backup file
    if (renameat(dirfd, imk_filename, dirfd, imk_backupname) != 0 && errno != ENOENT) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                  "ERROR: failed to backup immutable kernel to: \"%s/%s\"",
                  toolArgs->prelinkedKernelDirname, imk_backupname);
        result = EX_OSERR;
        goto finish;
    }
    have_backup = true;

    // hard link the immutable kernel to the newly built prelinked kernel
    if (linkat(dirfd, plk_filename, dirfd, imk_filename, 0) != 0) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                  "ERROR: link failed: \"%s/%s\" -> \"%s/%s\"",
                  toolArgs->prelinkedKernelDirname, imk_filename,
                  toolArgs->prelinkedKernelDirname, plk_filename);
        result = EX_OSERR;
        goto finish;
    }

    // personalize the new immutable kernel
    {
        char root_volume[PATH_MAX] ={};
        char imk_path[PATH_MAX] = {};
        char *personalize_argv[] = {
            kPersonalizeMacOSTool,
            "--volume",
            root_volume,     /* content to be filled in below */
            "--kernelCache",
            imk_path,
            NULL
        };

        if (toolArgs->volumeRootURL) {
            if (CFURLGetFileSystemRepresentation(toolArgs->volumeRootURL, true,
                                                 (UInt8 *)root_volume,
                                                 sizeof(root_volume)) == false) {
                result = EX_SOFTWARE;
                goto finish;
            }
        } else {
            root_volume[0] = '/';
            root_volume[1] = 0;
        }

        if (strlcpy(imk_path, toolArgs->prelinkedKernelDirname, sizeof(imk_path)) > sizeof(imk_path) ||
            strlcat(imk_path, "/", sizeof(imk_path)) > sizeof(imk_path) ||
            strlcat(imk_path, imk_filename, sizeof(imk_path)) > sizeof(imk_path)) {
            result = EX_SOFTWARE;
            goto finish;
        }

        int rval = fork_program(kPersonalizeMacOSTool, personalize_argv, true /* wait */);
        if (rval != 0) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                      "ERROR(%d): personalization failed for : \"%s/%s\"",
                      rval, toolArgs->prelinkedKernelDirname, imk_filename);
            result = EX_OSERR;
            goto finish;
        }
    }

    /* for now, we leave the immutable kernel backup file for easier recovery */

finish:
    if (have_backup && result != EX_OK) {
        /* clean up from a failed attempt: put back the old immutable kernel */
        renameat(dirfd, imk_backupname, dirfd, imk_filename);
    }
    if (result != EX_OK) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                  "ERROR(%d): building/personalizing the immutable kernel: \"%s/%s\"",
                  result, toolArgs->prelinkedKernelDirname, imk_filename);
    }
    return result;
}

#pragma mark Boot!=Root

/*******************************************************************************
* usage()
*******************************************************************************/
void usage(UsageLevel usageLevel)
{
    fprintf(stderr,
      "usage: %1$s -prelinked-kernel <filename> [options] [--] [kext or directory]\n"
      "       %1$s -system-prelinked-kernel\n"
      "       %1$s [options] -prelinked-kernel\n"
#if !NO_BOOT_ROOT
    "       %1$s -invalidate <volume> \n"
    "       %1$s -update-volume <volume> [options]\n"
#endif /* !NO_BOOT_ROOT */
      "       %1$s -system-caches [options]\n"
      "\n",
      progname);

    if (usageLevel == kUsageLevelBrief) {
        fprintf(stderr, "use %s -%s for an explanation of each option\n",
            progname, kOptNameHelp);
    }

    if (usageLevel == kUsageLevelBrief) {
        return;
    }

    fprintf(stderr, "-%s [<filename>] (-%c):\n"
        "        create/update prelinked kernel (must be last if no filename given)\n",
        kOptNamePrelinkedKernel, kOptPrelinkedKernel);
    fprintf(stderr, "-%s:\n"
        "        create/update system prelinked kernel\n",
        kOptNameSystemPrelinkedKernel);
#if !NO_BOOT_ROOT
    fprintf(stderr, "-%s <volume> (-%c): invalidate system kext caches for <volume>\n",
            kOptNameInvalidate, kOptInvalidate);
    fprintf(stderr, "-%s <volume> (-%c): update system kext caches for <volume>\n",
            kOptNameUpdate, kOptUpdate);
    fprintf(stderr, "-%s called us, modify behavior appropriately\n",
            kOptNameInstaller);
    fprintf(stderr, "-%s skips updating any helper partitions even if they appear out of date\n",
            kOptNameCachesOnly);
#endif /* !NO_BOOT_ROOT */
#if 0
// don't print this system-use option
    fprintf(stderr, "-%c <volume>:\n"
        "        check system kext caches for <volume> (nonzero exit if out of date)\n",
        kOptCheckUpdate);
#endif
    fprintf(stderr, "-%s: update system kext info caches for the root volume\n",
        kOptNameSystemCaches);
    fprintf(stderr, "\n");

    fprintf(stderr,
        "kext or directory: Consider kext or all kexts in directory for inclusion\n");
    fprintf(stderr, "-%s <bundle_id> (-%c):\n"
        "        include the kext whose CFBundleIdentifier is <bundle_id>\n",
        kOptNameBundleIdentifier, kOptBundleIdentifier);
    fprintf(stderr, "-%s <volume>:\n"
        "        Save kext paths in a prelinked kernel "
        " relative to <volume>\n",
        kOptNameVolumeRoot);
    fprintf(stderr, "-%s <kernel_filename> (-%c): Use kernel_filename for a prelinked kernel\n",
        kOptNameKernel, kOptKernel);
    fprintf(stderr, "-%s (-%c): Include all kexts ever loaded in prelinked kernel\n",
        kOptNameAllLoaded, kOptAllLoaded);
#if !NO_BOOT_ROOT
    fprintf(stderr, "-%s (-%c): Update volumes even if they look up to date\n",
        kOptNameForce, kOptForce);
    fprintf(stderr, "\n");
#endif /* !NO_BOOT_ROOT */

    fprintf(stderr, "-%s <archname>:\n"
        "        include architecture <archname> in created cache(s)\n",
        kOptNameArch);
    fprintf(stderr, "-%c: run at low priority\n",
        kOptLowPriorityFork);
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
    fprintf(stderr, "-%s (-%c): don't authenticate kexts (for use during development)\n",
        kOptNameNoAuthentication, kOptNoAuthentication);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c): print this message and exit\n",
        kOptNameHelp, kOptHelp);

    // print out immutable kernel building options only if the
    // running system is able to personalize
    if (access(kPersonalizeMacOSTool, R_OK|X_OK) == 0) {
        fprintf(stderr, "-%s (-%c): rebuild and personalize an immutablekernel (hardlinked to the new prelinkedkernel)\n",
                kOptNameBuildImmutable, kOptBuildImmutable);
    }

    return;
}

#include "safecalls.h"

static void removeStalePrelinkedKernels(KextcacheArgs * toolArgs)
{
    int                 my_fd;
    struct stat         statBuf;
    char *              tmpPath                 = NULL; // must free
    char *              volRootPath             = NULL; // must free
    char *              suffixPtr               = NULL; // must free
    CFURLRef            myURL                   = NULL; // must release
    CFURLEnumeratorRef  myEnumerator            = NULL; // must release
    CFURLRef            enumURL                 = NULL; // do not release
    CFStringRef         tmpCFString             = NULL; // must release
    CFArrayRef          resultArray             = NULL; // must release

    // we currently only do this for AppleInternal builds
    while (statPath(kAppleInternalPath, &statBuf) == EX_OK) {
        tmpPath = malloc(PATH_MAX);
        volRootPath = malloc(PATH_MAX);

        if (tmpPath == NULL || volRootPath == NULL)  {
            break;
        };
        volRootPath[0] = 0x00;

        if (toolArgs->volumeRootURL) {
            if (CFURLGetFileSystemRepresentation(toolArgs->volumeRootURL,
                                                 true,
                                                 (UInt8 *)volRootPath,
                                                 PATH_MAX) == false) {
                // this should not happen, but just in case...
                volRootPath[0] = 0x00;
            }
        }

        // get full path to PrelinkedKernels directory
        // /{VOL}/System/Library/PrelinkedKernels
        if (strlen(volRootPath) > 1) {
            if (strlcpy(tmpPath, volRootPath, PATH_MAX) >= PATH_MAX)  break;
            if (strlcat(tmpPath, _kOSKextPrelinkedKernelsPath, PATH_MAX) >= PATH_MAX)  break;
        }
        else {
            if (strlcpy(tmpPath, _kOSKextPrelinkedKernelsPath, PATH_MAX) >= PATH_MAX)  break;
        }
        myURL = CFURLCreateFromFileSystemRepresentation(NULL,
                                                        (const UInt8 *)tmpPath,
                                                        strlen(tmpPath),
                                                        true);
        if (myURL) {
            myEnumerator = CFURLEnumeratorCreateForDirectoryURL(
                                                                NULL,
                                                                myURL,
                                                                kCFURLEnumeratorDefaultBehavior,
                                                                NULL);
        }
        while (myEnumerator &&
               CFURLEnumeratorGetNextURL(myEnumerator,
                                         &enumURL,
                                         NULL) == kCFURLEnumeratorSuccess) {
            SAFE_RELEASE_NULL(tmpCFString);
            SAFE_FREE_NULL(suffixPtr);

            // valid prelinked kernel name must be in the form of:
            // "prelinkedkernel" or
            // "prelinkedkernel."
            tmpCFString = CFURLCopyLastPathComponent(enumURL);
            if (tmpCFString == NULL)       continue;

            if (kCFCompareEqualTo == CFStringCompare(tmpCFString,
                                                     CFSTR("prelinkedkernel"),
                                                     kCFCompareAnchored)) {
                /* this is "prelinkedkernel" which is always valid */
                continue;
            }

            // only want prelinkedkernel. from here on
            if (CFStringHasPrefix(tmpCFString,
                                  CFSTR("prelinkedkernel.")) == false) {
                continue;
            }

            // skip any names with more than one '.' character.
            // For example: prelinkedkernel.foo.bar
            SAFE_RELEASE_NULL(resultArray);
            resultArray = CFStringCreateArrayWithFindResults(
                                                             NULL,
                                                             tmpCFString,
                                                             CFSTR("."),
                                                             CFRangeMake(0, CFStringGetLength(tmpCFString)),
                                                             0);
            if (resultArray && CFArrayGetCount(resultArray) > 1) {
                continue;
            }
            SAFE_RELEASE(tmpCFString);
            tmpCFString =  CFURLCopyPathExtension(enumURL);
            if (tmpCFString == NULL)   continue;
            suffixPtr = createUTF8CStringForCFString(tmpCFString);
            if (suffixPtr == NULL)    continue;

            // Is there a corresponding kernel with this suffix?
            // get full path to Kernels directory
            if (strlen(volRootPath) > 1) {
                if (strlcpy(tmpPath, volRootPath, PATH_MAX) >= PATH_MAX)  continue;
                if (strlcat(tmpPath, kDefaultKernelPath, PATH_MAX) >= PATH_MAX)  continue;
            }
            else {
                if (strlcpy(tmpPath, kDefaultKernelPath, PATH_MAX) >= PATH_MAX)  continue;
            }
            if (strlcat(tmpPath, ".", PATH_MAX) >= PATH_MAX)  continue;
            if (strlcat(tmpPath, suffixPtr, PATH_MAX) >= PATH_MAX)  continue;


            if (statPath(tmpPath, &statBuf) == EX_OK) {
                // found matching kernel, nothing to clean up
                continue;
            }

            // OK, we have a stale prelinked kernel (no matching kernel)
            // get rid of stale prelinked kernel
            if (CFURLGetFileSystemRepresentation(enumURL,
                                                 true,
                                                 (UInt8 *)tmpPath,
                                                 PATH_MAX) == false) {
                continue;
            }

            my_fd = open(tmpPath, O_RDONLY);
            if (my_fd != -1) {
                if (sunlink(my_fd, tmpPath) == 0) {
                    OSKextLogCFString(NULL,
                                      kOSKextLogErrorLevel | kOSKextLogArchiveFlag |  kOSKextLogFileAccessFlag,
                                      CFSTR("stale prelinked kernel, removing '%s'"),
                                      tmpPath);
                }
                else {
                    OSKextLogCFString(NULL,
                                      kOSKextLogGeneralFlag | kOSKextLogErrorLevel,
                                      CFSTR("%s: sunlink failed for '%s' "),
                                      __func__, tmpPath);
                }
                close(my_fd);
            }
        } // while CFURLEnumeratorGetNextURL for prelinked kernels

        SAFE_RELEASE_NULL(myURL);
        SAFE_RELEASE_NULL(myEnumerator);
        break;
    } // while AppleInternal...

    SAFE_FREE(tmpPath);
    SAFE_FREE(volRootPath);
    SAFE_FREE(suffixPtr);
    SAFE_RELEASE(myURL);
    SAFE_RELEASE(myEnumerator);
    SAFE_RELEASE(tmpCFString);
    SAFE_RELEASE(resultArray);

    return;
}

