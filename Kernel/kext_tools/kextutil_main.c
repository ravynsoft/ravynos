/*
 *  kextutil_main.c
 *  kext_tools
 *
 *  Created by Nik Gervae on 4/24/08.
 *  Copyright 2008 Apple Inc. All rights reserved.
 *
 */
#include "kextutil_main.h"
#include "kext_tools_util.h"
#include "security.h"
#include "staging.h"
#include "syspolicy.h"
#include "bootcaches.h"
#include "kextaudit.h"

#include <libc.h>
#include <sysexits.h>

#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>

#include <IOKit/kext/KextManager.h>
#include <IOKit/kext/KextManagerPriv.h>
#include <IOKit/kext/kextmanager_types.h>

#include <servers/bootstrap.h>    // bootstrap mach ports
#include <bootfiles.h>

#pragma mark Constants
/*******************************************************************************
* Constants
*******************************************************************************/
#define BAD_ADDRESS_SPEC "Address format is <bundle-id@address>, " \
                         "with nonzero hexadecimal address."

#pragma mark Global/Static Variables
/*******************************************************************************
* Global/Static Variables
*******************************************************************************/
const char * progname = "(unknown)";

#define LOCK_MAXTRIES 90
#define LOCK_DELAY     1
static mach_port_t sKextdPort = MACH_PORT_NULL;
static mach_port_t sLockPort = MACH_PORT_NULL;     // kext loading lock
static int sLockStatus = 0;
static bool sLockTaken = false;

// Simple helper macro that will only assign code to var if it's the first failure.
#define SET_FIRST_FAILURE(var, code) var = ((var == EX_OK) ? (code) : var)

static Boolean
isRunningAsRoot()
{
    return geteuid() == 0;
}

#pragma mark Main Routine
/*******************************************************************************
* Global variables.
*******************************************************************************/
ExitStatus
main(int argc, char * const * argv)
{
    ExitStatus result         = EX_SOFTWARE;
    ExitStatus processResult  = EX_SOFTWARE;
    Boolean    fatal          = false;
    CFArrayRef allKexts       = NULL;  // must release
    CFArrayRef kextsToProcess = NULL;  // must release
    KextutilArgs   toolArgs;

   /*****
    * Find out what the program was invoked as.
    */
    progname = rindex(argv[0], '/');
    if (progname) {
        progname++;   // go past the '/'
    } else {
        progname = (char *)argv[0];
    }

   /* Set the OSKext log callback right away and hook up to get any
    * warnings & errors out of the kernel.
    */
    OSKextSetLogOutputFunction(&tool_log);
    OSKextSetLogFilter(kOSKextLogWarningLevel | kOSKextLogVerboseFlagsMask,
        /* kernel? */ true);

   /*****
    * Process args & check for permission to load.
    */
    result = readArgs(argc, argv, &toolArgs);
    if (result != EX_OK) {
        if (result == kKextutilExitHelp) {
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

   /*****
    * Turn on recording of all diagnostics.
    */
    OSKextSetRecordsDiagnostics(kOSKextDiagnosticsFlagAll);

    /*****
     * Configure authentication options - allow command line flag to skip all auth checks,
     * but only if the kext will not eventually be loaded into the running system.  Otherwise,
     * -z (skipAuthentication) can bypass all security checks.  checkArgs should have already
     * caught most of these cases, but for safety confirm them here.
     */
    Boolean isSecureOperation = toolArgs.doLoad || toolArgs.doStartMatching;
    Boolean doAuthBypass = toolArgs.skipAuthentication && !isSecureOperation;
    AuthOptions_t authenticationOptions = {
        .allowNetwork = true,
        .isCacheLoad = false,
        .performFilesystemValidation = !doAuthBypass,
        .performSignatureValidation = !doAuthBypass,
        .requireSecureLocation = !doAuthBypass,
        .respectSystemPolicy = !doAuthBypass,
        .checkDextApproval = !doAuthBypass,
        .is_kextcache = false,
    };

    /* System policy checks are meant to be called in the load path only,
     * so avoid calling them in any scenario where a load could not occur.
     * Otherwise, this could be a security issue (privilege escalation) when
     * the user approves.
     *
     * Skip this check if the user is not root and is not performing a secure
     * operation.  checkArgs has already ensured the user is root if any secure
     * operations will be performed, so this condition is only for diagnostics.
     *
     * Technically staging won't work as a non-root user either, but instead of
     * disabling the security check, leave it enabled and ensure no staging operations
     * are attempted unless being called as the root user.
     */
    if (!isSecureOperation && !isRunningAsRoot()) {
        authenticationOptions.respectSystemPolicy = false;
    }

    _OSKextSetAuthenticationFunction(&authenticateKext, &authenticationOptions);
    _OSKextSetStrictAuthentication(true);

    /* Set up auditing of kext loads, see kextaudit.c */
    _OSKextSetLoadAuditFunction(&KextAuditLoadCallback);
    setVariantSuffix();

   /*****
    * Create the set of kexts we'll be working from.
    *
    * If a secure location is required and the process is running as root, load a staged
    * copy of all kexts but also keep around a copy of the original kexts because some
    * of the diagnostic functionality requires lookup by URL.
    */
    if (authenticationOptions.requireSecureLocation && isRunningAsRoot()) {
        allKexts = createStagedKextsFromURLs(toolArgs.scanURLs, true);
    } else {
        allKexts = OSKextCreateKextsFromURLs(kCFAllocatorDefault, toolArgs.scanURLs);
    }

    if (!allKexts || !CFArrayGetCount(allKexts)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "No kernel extensions found.");
        result = EX_SOFTWARE;
        goto finish;
    }

    if (result != EX_OK) {
        goto finish;
    }

   /*****
    * Figure out which kexts we're actually loading/generating symbols for.
    * This must be done after the kext objects are created.
    */
    result = createKextsToProcess(&toolArgs, &kextsToProcess, &fatal);
    if (fatal) {
        goto finish;
    }

    if (!serializeLoad(&toolArgs, toolArgs.doLoad)) {
        result = EX_OSERR;
        goto finish;
    }

    processResult = processKexts(kextsToProcess, &toolArgs);
    if (result == EX_OK) {
        result = processResult;
    }

finish:
   /*****
    * Clean everything up. The mach stuff should be done even though we're
    * exiting so we don't mess up other processes.
    */
    if (sLockTaken) {
        kextmanager_unlock_kextload(sKextdPort, sLockPort);
    }
    if (sKextdPort != MACH_PORT_NULL) {
        mach_port_deallocate(mach_task_self(), sKextdPort);
    }
    if (sLockPort != MACH_PORT_NULL) {
        mach_port_mod_refs(mach_task_self(), sLockPort, MACH_PORT_RIGHT_RECEIVE, -1);
    }

   /* We're actually not going to free anything else because we're exiting!
    */
    exit(result);

    SAFE_RELEASE(allKexts);
    SAFE_RELEASE(toolArgs.loadAddresses);
    SAFE_RELEASE(toolArgs.kextIDs);
    SAFE_RELEASE(toolArgs.personalityNames);
    SAFE_RELEASE(toolArgs.dependencyURLs);
    SAFE_RELEASE(toolArgs.repositoryURLs);
    SAFE_RELEASE(toolArgs.kextURLs);
    SAFE_RELEASE(toolArgs.scanURLs);
    SAFE_RELEASE(toolArgs.kernelURL);
    SAFE_RELEASE(toolArgs.kernelFile);
    SAFE_RELEASE(toolArgs.symbolDirURL);

    return result;
}

#pragma mark Major Subroutines
/*******************************************************************************
* Major Subroutines
*******************************************************************************/
ExitStatus
readArgs(
    int            argc,
    char * const * argv,
    KextutilArgs * toolArgs)
{
    ExitStatus   result          = EX_USAGE;
    ExitStatus   scratchResult   = EX_USAGE;
    int          optchar;
    int          longindex;
    CFStringRef  scratchString   = NULL;  // must release
    CFNumberRef  scratchNumber   = NULL;  // must release
    CFNumberRef  existingAddress = NULL;  // do not release
    CFURLRef     scratchURL      = NULL;  // must release
    uint32_t     i;

    bzero(toolArgs, sizeof(*toolArgs));

   /* Set up default arg values.
    */
    toolArgs->useRepositoryCaches = true;
    toolArgs->useSystemExtensions = true;
    toolArgs->overwriteSymbols    = true;
    toolArgs->interactiveLevel    = kOSKextExcludeNone;
    toolArgs->doLoad              = true;
    toolArgs->doStartMatching     = true;
    // toolArgs->archInfo must remain NULL before checkArgs

   /*****
    * Allocate collection objects.
    */
    if (!createCFMutableDictionary(&toolArgs->loadAddresses)                       ||
        !createCFMutableArray(&toolArgs->kextIDs, &kCFTypeArrayCallBacks)          ||
        !createCFMutableArray(&toolArgs->personalityNames, &kCFTypeArrayCallBacks) ||
        !createCFMutableArray(&toolArgs->dependencyURLs, &kCFTypeArrayCallBacks)   ||
        !createCFMutableArray(&toolArgs->repositoryURLs, &kCFTypeArrayCallBacks)   ||
        !createCFMutableArray(&toolArgs->kextURLs, &kCFTypeArrayCallBacks)         ||
        !createCFMutableArray(&toolArgs->scanURLs, &kCFTypeArrayCallBacks)) {

        result = EX_OSERR;
        OSKextLogMemError();
        exit(result);
    }

    while ((optchar = getopt_long_only(argc, (char * const *)argv,
        kOptChars, sOptInfo, &longindex)) != -1) {

        char * address_string = NULL;  // don't free
        uint64_t address;

        SAFE_RELEASE_NULL(scratchString);
        SAFE_RELEASE_NULL(scratchNumber);
        SAFE_RELEASE_NULL(scratchURL);

        switch (optchar) {
            case kOptHelp:
                usage(kUsageLevelFull);
                result = kKextutilExitHelp;
                goto finish;
                break;
            case kOptBundleIdentifier:
                scratchString = CFStringCreateWithCString(kCFAllocatorDefault,
                    optarg, kCFStringEncodingUTF8);
                if (!scratchString) {
                    OSKextLogMemError();
                    result = EX_OSERR;
                    goto finish;
                }
                CFArrayAppendValue(toolArgs->kextIDs, scratchString);
                break;

            case kOptPersonality:
                scratchString = CFStringCreateWithCString(kCFAllocatorDefault,
                    optarg, kCFStringEncodingUTF8);
                if (!scratchString) {
                    OSKextLogMemError();
                    result = EX_OSERR;
                    goto finish;
                }
                CFArrayAppendValue(toolArgs->personalityNames, scratchString);
                break;

            case kOptKernel:
                if (toolArgs->kernelURL) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                        "Warning: multiple use of -%s (-%c); using last.",
                        kOptNameKernel, kOptKernel);
                    SAFE_RELEASE_NULL(toolArgs->kernelURL);
                }
                scratchURL = CFURLCreateFromFileSystemRepresentation(
                    kCFAllocatorDefault,
                    (const UInt8 *)optarg, strlen(optarg), true);
                if (!scratchURL) {
                    OSKextLogStringError(/* kext */ NULL);
                    result = EX_OSERR;
                    goto finish;
                }
                toolArgs->kernelURL = CFRetain(scratchURL);
                break;

            case kOptDependency:
                scratchURL = CFURLCreateFromFileSystemRepresentation(
                    kCFAllocatorDefault,
                    (const UInt8 *)optarg, strlen(optarg), true);
                if (!scratchURL) {
                    OSKextLogStringError(/* kext */ NULL);
                    result = EX_OSERR;
                    goto finish;
                }
                CFArrayAppendValue(toolArgs->dependencyURLs, scratchURL);
                break;

            case kOptRepository:
                scratchResult = checkPath(optarg, /* suffix */ NULL,
                    /* directoryRequired */ TRUE, /* writableRequired */ FALSE);
                if (scratchResult != EX_OK) {
                    result = scratchResult;
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
                CFArrayAppendValue(toolArgs->repositoryURLs, scratchURL);
                break;

            case kOptNoCaches:
                toolArgs->useRepositoryCaches = false;
                break;

            case kOptNoLoadedCheck:
                toolArgs->checkLoadedForDependencies = false;
                break;

            case kOptNoSystemExtensions:
                toolArgs->useSystemExtensions = false;
                break;

            case kOptInteractive:
                if (toolArgs->interactiveLevel) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                        "Warning: multiple use of -%s (-%c) or -%s (-%c); using last.",
                        kOptNameInteractive, kOptInteractive,
                        kOptNameInteractiveAll, kOptInteractiveAll);
                }
                toolArgs->overwriteSymbols = false;
                toolArgs->interactiveLevel = kOSKextExcludeKext;
                break;

            case kOptInteractiveAll:
                if (toolArgs->interactiveLevel) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                        "Warning: multiple use of -%s (-%c) or -%s (-%c); using last.",
                        kOptNameInteractive, kOptInteractive,
                        kOptNameInteractiveAll, kOptInteractiveAll);
                }
                toolArgs->overwriteSymbols = false;
                toolArgs->interactiveLevel = kOSKextExcludeAll;
                break;

            case kOptLoadOnly:
                toolArgs->flag_l = 1;
                break;

            case kOptMatchOnly:
                toolArgs->flag_m = 1;
                break;

            case kOptNoLoad:
                toolArgs->flag_n = 1;
                break;

            case kOptSymbolsDirectory:
                if (toolArgs->symbolDirURL) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                        "Warning: multiple use of -%s (-%c); using last",
                        kOptNameSymbolsDirectory, kOptSymbolsDirectory);
                    SAFE_RELEASE_NULL(toolArgs->symbolDirURL);
                }
                scratchResult = checkPath(optarg, /* suffix */ NULL,
                    /* directoryRequired? */ TRUE, /* writable? */ TRUE);
                if (scratchResult != EX_OK) {
                    result = scratchResult;
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
                toolArgs->symbolDirURL = CFRetain(scratchURL);
                break;

            case kOptAddress:
                toolArgs->flag_n = 1;  // -a implies -n

                address_string = index(optarg, '@');
                if (!address_string) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                        BAD_ADDRESS_SPEC);
                    goto finish;
                }
                address_string[0] = '\0';
                address_string++;

               /* Read a 64-bit int here; we'll check at load time
                * whether the address is too big.
                */
                address = strtoull(address_string, NULL, 16);
                if (!address) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                        BAD_ADDRESS_SPEC);
                    goto finish;
                }
                scratchNumber = CFNumberCreate(kCFAllocatorDefault,
                    kCFNumberSInt64Type, &address);
                if (!scratchNumber) {
                    OSKextLogMemError();
                    result = EX_OSERR;
                    goto finish;
                }
                scratchString = CFStringCreateWithCString(kCFAllocatorDefault,
                   optarg, kCFStringEncodingUTF8);
                if (!scratchString) {
                    OSKextLogMemError();
                    result = EX_OSERR;
                    goto finish;
                }
                existingAddress = CFDictionaryGetValue(toolArgs->loadAddresses,
                    scratchString);
                if (existingAddress && !CFEqual(scratchNumber, existingAddress)) {
                    OSKextLog(/* kext */ NULL, kOSKextLogWarningLevel,
                        "Warning: multiple addresses specified for %s; using last.",
                        optarg);
                }
                CFDictionarySetValue(toolArgs->loadAddresses, scratchString,
                    scratchNumber);
                break;

            case kOptUseKernelAddresses:
                toolArgs->flag_n = 1;   // -A implies -n
                toolArgs->getAddressesFromKernel = true;
                break;

            case kOptQuiet:
                beQuiet();
                toolArgs->logFilterChanged = true;
                break;

            case kOptVerbose:
                scratchResult = setLogFilterForOpt(argc, argv, /* forceOnFlags */ 0);
                if (scratchResult != EX_OK) {
                    result = scratchResult;
                    goto finish;
                }
                toolArgs->logFilterChanged = true;
                break;

            case kOptTests:
                toolArgs->printDiagnostics = true;
                break;

            case kOptSafeBoot:
                toolArgs->safeBootMode = true;
                toolArgs->useRepositoryCaches = false;  // -x implies -c
                break;

            case kOptNoAuthentication:
                toolArgs->skipAuthentication = true;
                break;

            case kOptNoResolveDependencies:
                toolArgs->skipDependencies = true;
                break;

            case 0:
                switch (longopt) {
                    case kLongOptArch:
                        if (toolArgs->archInfo) {
                            OSKextLog(/* kext */ NULL,
                                kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                                "Warning: multiple use of -%s; using last.",
                                kOptNameArch);
                        }
                        toolArgs->archInfo = NXGetArchInfoFromName(optarg);
                        if (!toolArgs->archInfo) {
                            OSKextLog(/* kext */ NULL,
                                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                                "Unknown architecture %s.", optarg);
                            goto finish;
                        }
                        break;

                   default:
                       /* getopt_long_only() prints an error message for us. */
                        goto finish;
                        break;
                }
                break;

            default:
               /* getopt_long_only() prints an error message for us. */
                goto finish;
                break;

        } /* switch (optchar) */
    } /* while (optchar = getopt_long_only(...) */

   /*****
    * Record the kext names from the command line.
    */
    for (i = optind; i < argc; i++) {
        SAFE_RELEASE_NULL(scratchURL);

        scratchResult = checkPath(argv[i], kOSKextBundleExtension,
            /* directoryRequired */ TRUE, /* writableRequired */ FALSE);
        if (scratchResult != EX_OK) {
            result = scratchResult;
            goto finish;
        }

        scratchURL = CFURLCreateFromFileSystemRepresentation(
            kCFAllocatorDefault,
            (const UInt8 *)argv[i], strlen(argv[i]), true);
        if (!scratchURL) {
            result = EX_OSERR;
            OSKextLogMemError();
            goto finish;
        }
        CFArrayAppendValue(toolArgs->kextURLs, scratchURL);
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
ExitStatus
checkArgs(KextutilArgs * toolArgs)
{
    ExitStatus         result         = EX_USAGE;
    char               kernelPathCString[PATH_MAX];
    const NXArchInfo * kernelArchInfo = OSKextGetRunningKernelArchitecture();

   /* We don't need this for everything, but it's unlikely to fail.
    */
    if (!kernelArchInfo) {
        result = EX_OSERR;
        goto finish;
    }

   /*****
    * Check for bad combinations of arguments and options.
    */
    if (toolArgs->flag_l + toolArgs->flag_m + toolArgs->flag_n > 1) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Only one of -%s (-%c), -%s (-%c), or -%s (-%c) is allowed;\n"
            "-%s (-%c) and -%s (-%c) imply -%s (-%c).",
            kOptNameLoadOnly, kOptLoadOnly,
            kOptNameMatchOnly, kOptMatchOnly,
            kOptNameNoLoad, kOptNoLoad,
            kOptNameAddress, kOptAddress,
            kOptNameUseKernelAddresses, kOptUseKernelAddresses,
            kOptNameNoLoad, kOptNoLoad);
        goto finish;
    } else if (toolArgs->flag_l) {
        toolArgs->doLoad = true;
        toolArgs->doStartMatching = false;
    } else if (toolArgs->flag_m) {
        toolArgs->doLoad = false;
        toolArgs->doStartMatching = true;
    } else if (toolArgs->flag_n) {
        toolArgs->doLoad = false;
        toolArgs->doStartMatching = false;
    }

    if ((toolArgs->interactiveLevel != kOSKextExcludeNone) &&
        !toolArgs->doLoad && !toolArgs->doStartMatching) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Use interactive modes only when loading or matching.");
        goto finish;
    }

   /* If running in quiet mode and the invocation might require
    * interaction, just exit with an error. Interaction required when:
    *
    * - Explicitly flagged with -i/-I.
    * - Saving symbols w/o loading, no addresses specified,
    *   not getting from kernel.
    */
    if (OSKextGetLogFilter(/* kernel? */ false) == kOSKextLogSilentFilter) {

        Boolean interactive = (toolArgs->interactiveLevel != kOSKextExcludeNone);
        Boolean needAddresses = (toolArgs->symbolDirURL &&
            !toolArgs->doLoad &&
            CFDictionaryGetCount(toolArgs->loadAddresses) == 0 &&
            !toolArgs->getAddressesFromKernel);

        if (interactive || needAddresses) {
            result = kKextutilExitLoadFailed;
            goto finish;
        }
    }

   /* Disallow -address(-a) with any load or getting addresses from kernel.
    */
    if (CFDictionaryGetCount(toolArgs->loadAddresses) > 0 &&
        (toolArgs->doLoad || toolArgs->getAddressesFromKernel)) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Don't use -%s (-%c) when loading, or with -%s (-%c).",
            kOptNameAddress, kOptAddress,
            kOptNameUseKernelAddresses, kOptUseKernelAddresses);
        goto finish;
    }

   /* If sending anything into the kernel, don't use a kernel file
    * and refuse to skip authentication.
    */
    if (toolArgs->doLoad || toolArgs->doStartMatching) {
        if (toolArgs->kernelURL) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "-%s (-%c) is allowed only when not loading "
                "or sending personalities.",
                kOptNameKernel, kOptKernel);
            goto finish;
        }

        if (toolArgs->skipAuthentication) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "-%s (-%c) is allowed only when not loading "
                "or sending personalities.",
                kOptNameNoAuthentication, kOptNoAuthentication);
            goto finish;
        }
    }

   /* If we aren't sending anything to the kernel and not doing full
    * tests, then don't bother authenticating. This lets developers
    * generate symbols more conveniently & allows basic checks with -n alone.
    */
    if (!toolArgs->doLoad &&
        !toolArgs->doStartMatching &&
        !toolArgs->printDiagnostics) {

        toolArgs-> skipAuthentication = true;
    }

   /****
    * If we're getting addresses from the kernel we have to call
    * down there, so we might as well check what's loaded before
    * resolving dependencies too (-A overrides -D). If we're not
    * performing a load, then don't check (-n/-m implies -D).
    */
    if (toolArgs->getAddressesFromKernel) {
        toolArgs->checkLoadedForDependencies = true;
    }

   /*****
    * -no-resolve-dependencies/-Z is only allowed
    * if you don't need to resolve dependencies (duh).
    * You need to resolve if loading or saving symbols.
    */
    if (toolArgs->skipDependencies) {
        if (toolArgs->doLoad ||
            toolArgs->symbolDirURL) {

            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Use -%s (-%c) only with -%s (-%c) or %s (-%c), "
                "and not with -%s (-%c).",
                kOptNameNoResolveDependencies, kOptNoResolveDependencies,
                kOptNameNoLoad, kOptNoLoad,
                kOptNameMatchOnly, kOptMatchOnly,
                kOptNameSymbolsDirectory, kOptSymbolsDirectory);
            goto finish;
        }
    }

    if (!CFArrayGetCount(toolArgs->kextURLs) &&
        !CFArrayGetCount(toolArgs->kextIDs) &&
        !CFDictionaryGetCount(toolArgs->loadAddresses)) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "No kernel extensions specified; name kernel extension bundles\n"
            "    following options, or use -%s (-%c) and -%s (-%c).",
            kOptNameBundleIdentifier, kOptBundleIdentifier,
            kOptNameAddress, kOptAddress);
        goto finish;
    }

   /*****
    * Check whether the user has permission to load into the kernel.
    */
    if ((toolArgs->doLoad || toolArgs->doStartMatching) && !isRunningAsRoot()) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "You must be running as root to load kexts "
            "or send personalities into the kernel.");
        result = EX_NOPERM;
        goto finish;
    }

   /*****
    * Set up which arch to work with and sanity-check it.
    */
    if (toolArgs->archInfo) {

       /* If loading into or getting addresses form the kernel,
        * any specified architecture must match that of the running kernel.
        */
        if (toolArgs->doLoad || toolArgs->getAddressesFromKernel) {

            if (kernelArchInfo != OSKextGetArchitecture()) {

                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Specified architecture %s does not match "
                    "running kernel architecture %s.",
                    toolArgs->archInfo->name, kernelArchInfo->name);
                goto finish;
            }
        }

       /* All good; set the default OSKext arch & safe boot mode.
        */
        OSKextSetArchitecture(toolArgs->archInfo);
        OSKextSetSimulatedSafeBoot(toolArgs->safeBootMode);
    }

   /* Give a notice to the user in case they're doing cross-arch work.
    */
    if (toolArgs->symbolDirURL && !toolArgs->archInfo &&
        !toolArgs->getAddressesFromKernel) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogWarningLevel | kOSKextLogLinkFlag,
            "Notice: Using running kernel architecture %s to generate symbols.",
            kernelArchInfo->name);
       /* not an error! */
    }

   /* Give a notice to the user if safe boot mode is actually on.
    */
    if (OSKextGetActualSafeBoot() && !toolArgs->safeBootMode) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogWarningLevel | kOSKextLogLoadFlag,
            "Notice: system is in safe boot mode; kernel may refuse loads.");
       /* not an error! */
    }

   /*****
    * Assemble the list of URLs to scan, in this order (the OSKext lib inverts it
    * for last-opened-wins semantics):
    * 1. System repository directories (-no-system-extensions/-e skips).
    * 2. Named kexts (always given after -repository & -dependency on command line).
    * 3. Named repository directories (-repository/-r).
    * 4. Named dependencies get priority (-dependency/-d).
    */
    if (toolArgs->useSystemExtensions) {
        CFArrayRef sysExtFolders = OSKextGetSystemExtensionsFolderURLs();
        // xxx - check it
        CFArrayAppendArray(toolArgs->scanURLs,
            sysExtFolders, RANGE_ALL(sysExtFolders));
    }
    CFArrayAppendArray(toolArgs->scanURLs, toolArgs->kextURLs,
        RANGE_ALL(toolArgs->kextURLs));
    CFArrayAppendArray(toolArgs->scanURLs, toolArgs->repositoryURLs,
        RANGE_ALL(toolArgs->repositoryURLs));
    CFArrayAppendArray(toolArgs->scanURLs, toolArgs->dependencyURLs,
        RANGE_ALL(toolArgs->dependencyURLs));

    if ( (CFArrayGetCount(toolArgs->kextIDs) ||
          CFDictionaryGetCount(toolArgs->loadAddresses)) &&
        !CFArrayGetCount(toolArgs->scanURLs)) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "No kexts to search for bundle IDs; "
            "-%s (-%c) requires kexts or repository directories "
            "be named by path.",
            kOptNameBundleIdentifier, kOptBundleIdentifier);
        goto finish;
    }

    /* If no explicit kernel image was provided by the user, default it
     * to KernelPath from /usr/standalone/bootcaches.plist
     * <= 10.9 /mach_kernel
     * > 10.9 /System/Library/Kernels/kernel
     */
    if (!toolArgs->kernelURL) {
        CFURLRef        scratchURL = NULL;

        // use KernelPath from /usr/standalone/bootcaches.plist
        if (getKernelPathForURL(NULL,
                                kernelPathCString,
                                sizeof(kernelPathCString)) == FALSE) {
            // no bootcaches.plist?  Forced to hardwire...
            strlcpy(kernelPathCString, "/System/Library/Kernels/kernel",
                    sizeof(kernelPathCString));
        }

        if (useDevelopmentKernel(kernelPathCString)) {
            if (strlen(kernelPathCString) + strlen(kDefaultDevKernelSuffix) + 1 < sizeof(kernelPathCString)) {
                strlcat(kernelPathCString,
                        kDefaultDevKernelSuffix,
                        sizeof(kernelPathCString));
            }
        }

        scratchURL = CFURLCreateFromFileSystemRepresentation(
                                                             kCFAllocatorDefault,
                                                             (const UInt8 *)kernelPathCString,
                                                             strlen(kernelPathCString),
                                                             false );
        if (!scratchURL) {
            OSKextLogStringError(/* kext */ NULL);
            result = EX_OSERR;
            goto finish;
        }
        toolArgs->kernelURL = scratchURL;
        OSKextLog(/* kext */ NULL,
                  kOSKextLogBasicLevel | kOSKextLogLoadFlag,
                  "Defaulting to kernel file '%s'",
                  kernelPathCString);
    }

    if (toolArgs->kernelURL) {
        /* create and fill our CFData object for toolArgs->kernelFile
         */
        if (!CFURLGetFileSystemRepresentation(toolArgs->kernelURL,
                                              true,
                                              (uint8_t *)kernelPathCString,
                                              sizeof(kernelPathCString))) {
            OSKextLogStringError(/* kext */ NULL);
            result = EX_OSFILE;
            goto finish;
       }

        if (!createCFDataFromFile(&toolArgs->kernelFile,
                                  kernelPathCString)) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                      "Can't read kernel file '%s'",
                      kernelPathCString);
            result = EX_OSFILE;
            goto finish;
        }
    }

    if (!toolArgs->doLoad || (toolArgs->interactiveLevel != kOSKextExcludeNone)) {
        adjustLogFilterForInteractive(toolArgs);
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
void adjustLogFilterForInteractive(KextutilArgs * toolArgs)
{
    if (!toolArgs->logFilterChanged) {
        OSKextSetLogFilter(kDefaultServiceLogFilter, /* kernel? */ false);
        OSKextSetLogFilter(kDefaultServiceLogFilter, /* kernel? */ true);
    }
}

/*******************************************************************************
*******************************************************************************/
ExitStatus
createKextsToProcess(
    KextutilArgs * toolArgs,
    CFArrayRef   * outArray,
    Boolean      * fatal)
{
    ExitStatus         result = EX_OK;
    CFMutableArrayRef  kextsToProcess  = NULL;  // returned by ref.

    CFURLRef           kextURL = NULL;          // do not release
    char               kextPathCString[PATH_MAX];

    CFStringRef      * addressKeys     = NULL;  // must free
    char             * kextIDCString   = NULL;  // must free
    OSKextRef          theKext         = NULL;  // do not release
    CFIndex            kextCount, idCount, kextIndex, idIndex;

    if (!createCFMutableArray(&kextsToProcess, &kCFTypeArrayCallBacks)) {
        result = EX_OSERR;
        goto finish;
    }

   /*****
    * If we got kext bundle names, then create kexts for them. Kexts named
    * by path always have priority so we're adding them first.
    */
    kextCount = CFArrayGetCount(toolArgs->kextURLs);
    for (kextIndex = 0; kextIndex < kextCount; kextIndex++) {

        kextURL = (CFURLRef)CFArrayGetValueAtIndex(
            toolArgs->kextURLs, kextIndex);

        if (!CFURLGetFileSystemRepresentation(kextURL,
            /* resolveToBase */ false,
            (u_char *)kextPathCString, sizeof(kextPathCString))) {

            OSKextLogStringError(/* kext */ NULL);
            result = EX_OSERR;
            *fatal = true;
            goto finish;
        }

        OSKextLog(/* kext */ NULL,
            kOSKextLogStepLevel | kOSKextLogKextBookkeepingFlag,
            "Looking up extension with URL %s.",
            kextPathCString);

       /* Use OSKextGetKextWithURL() to avoid double open error messages,
        * because we already tried to open all kexts in main(). That means
        * we don't log here if we don't find the kext.
        */
        theKext = OSKextGetKextWithURL(kextURL);
        if (!theKext) {
            result = kKextutilExitNotFound;
            // keep going
            continue;
        }
        addToArrayIfAbsent(kextsToProcess, theKext);

       /* Since we're running a developer tool on this kext, let's go ahead
        * and enable per-kext logging for it regardless of the in-bundle setting.
        * Would be nice to do this for all dependencies but the user can set
        * the verbose filter's 0x8 bit for that. It will log for all kexts
        * but that's not so bad.
        */
        OSKextSetLoggingEnabled(theKext, true);
    }

   /*****
    * If we got load addresses on the command line, add their bundle IDs
    * to the list of kext IDs to load.
    */
    idCount = CFDictionaryGetCount(toolArgs->loadAddresses);
    if (idCount) {
        addressKeys = (CFStringRef *)malloc(idCount * sizeof(CFStringRef));
        if (!addressKeys) {
            OSKextLogMemError();
            result = EX_OSERR;
            *fatal = true;
            goto finish;
        }
        CFDictionaryGetKeysAndValues(toolArgs->loadAddresses,
            (void *)addressKeys, /* values */ NULL);

        for (idIndex = 0; idIndex < idCount; idIndex++) {
            if (kCFNotFound == CFArrayGetFirstIndexOfValue(toolArgs->kextIDs,
                RANGE_ALL(toolArgs->kextIDs), addressKeys[idIndex])) {

                CFArrayAppendValue(toolArgs->kextIDs, addressKeys[idIndex]);
            }
        }
    }

   /*****
    * If we have CFBundleIdentifiers, then look them up. We just add to the
    * kextsToProcess array here.
    */
    idCount = CFArrayGetCount(toolArgs->kextIDs);
    for (idIndex = 0; idIndex < idCount; idIndex++) {
        CFStringRef thisKextID = (CFStringRef)
            CFArrayGetValueAtIndex(toolArgs->kextIDs, idIndex);

        SAFE_FREE_NULL(kextIDCString);

        kextIDCString = createUTF8CStringForCFString(thisKextID);
        if (!kextIDCString) {
            OSKextLogMemError();
            result = EX_OSERR;
            goto finish;
        }

       /* First see if we already have this kext in the list of kexts
        * to process. Only if we don't have it by name do we look it up
        * by identifier. This should allow kexts named by path on the command
        * line to take precendence over any bundle ID lookups; for example,
        * if the user is working with an older version of the kext, an
        * identifier lookup would find the newer.
        */
        kextCount = CFArrayGetCount(kextsToProcess);
        for (kextIndex = 0; kextIndex < kextCount; kextIndex++) {
            OSKextRef scanKext =  (OSKextRef)CFArrayGetValueAtIndex(
                kextsToProcess, kextIndex);
            CFStringRef scanKextID = OSKextGetIdentifier(scanKext);

            if (CFEqual(thisKextID, scanKextID)) {
                theKext = scanKext;
                break;
            }
        }

        if (!theKext) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogStepLevel | kOSKextLogKextBookkeepingFlag,
                "Looking up extension with identifier %s.",
                kextIDCString);

            theKext = OSKextGetKextWithIdentifier(thisKextID);
        }

        if (!theKext) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Can't find extension with identifier %s.",
                kextIDCString);
            result = kKextutilExitNotFound;
            continue;  // not fatal, keep trying the others
        }

        kextURL = OSKextGetURL(theKext);
        if (!CFURLGetFileSystemRepresentation(kextURL,
            /* resolveToBase */ false,
            (u_char *)kextPathCString, sizeof(kextPathCString))) {

            OSKextLogStringError(theKext);
            result = EX_OSERR;
            *fatal = true;
            goto finish;
        }

        OSKextLog(/* kext */ NULL,
            kOSKextLogStepLevel | kOSKextLogKextBookkeepingFlag,
            "Found %s for identifier %s.", kextPathCString, kextIDCString);

        addToArrayIfAbsent(kextsToProcess, theKext);

       /* As above, so below; enable logging for the kext we just looked up.
        */
        OSKextSetLoggingEnabled(theKext, true);
    }

finish:
    SAFE_FREE(addressKeys);
    SAFE_FREE(kextIDCString);

   /* Let go of these two tool args, we don't need them any more.
    */
    SAFE_RELEASE_NULL(toolArgs->kextURLs);
    SAFE_RELEASE_NULL(toolArgs->kextIDs);

    if (*fatal) {
        SAFE_RELEASE(kextsToProcess);
        *outArray = NULL;
    } else {
        *outArray = kextsToProcess;
    }
    return result;
}

/*******************************************************************************
*******************************************************************************/
typedef struct {
    Boolean fatal;
} SetAddressContext;

void
setKextLoadAddress(
    const void * vKey,
    const void * vValue,
    void       * vContext)
{
    CFStringRef         bundleID          = (CFStringRef)vKey;
    CFNumberRef         loadAddressNumber = (CFNumberRef)vValue;
    SetAddressContext * context           = (SetAddressContext *)vContext;
    CFArrayRef          kexts             = NULL;  // must release
    OSKextRef           theKext           = NULL;  // do not release
    uint64_t            loadAddress       = 0;
    CFIndex             count, i;

    if (context->fatal) {
        goto finish;
    }

    if (!CFNumberGetValue(loadAddressNumber, kCFNumberSInt64Type,
        &loadAddress)) {

        context->fatal = true;
        goto finish;
    }


    kexts = OSKextCopyKextsWithIdentifier(bundleID);
    if (!kexts) {
        goto finish;
    }

    count = CFArrayGetCount(kexts);
    for (i = 0; i < count; i++) {

        theKext = (OSKextRef)CFArrayGetValueAtIndex(kexts, i);
        if (!OSKextSetLoadAddress(theKext, loadAddress)) {
            context->fatal = true;
            goto finish;
        }
    }

finish:
    SAFE_RELEASE(kexts);
    return;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus
processKexts(
    CFArrayRef     kextsToProcess,
    KextutilArgs * toolArgs)
{
    ExitStatus          result             = EX_OK;
    OSReturn            readLoadInfoResult = kOSReturnError;
    Boolean             fatal              = false;
    SetAddressContext   setLoadAddressContext;
    CFIndex             count, i;

    if (toolArgs->getAddressesFromKernel) {
        readLoadInfoResult = OSKextReadLoadedKextInfo(
            /* kextIdentifiers */ NULL,
            /* flushDependencies */ true);
        if (readLoadInfoResult !=  kOSReturnSuccess) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                "Failed to read load info for kexts - %s.",
                safe_mach_error_string(readLoadInfoResult));
            result = EX_OSERR;
            goto finish;
        }
    } else if (toolArgs->loadAddresses) {
        setLoadAddressContext.fatal = false;
        CFDictionaryApplyFunction(toolArgs->loadAddresses, setKextLoadAddress,
            &setLoadAddressContext);
        if (setLoadAddressContext.fatal) {
            result = EX_OSERR;  // xxx - or may software
            goto finish;
        }
    }

    /* Get busy loading kexts.
     */
    count = CFArrayGetCount(kextsToProcess);
    for (i = 0; i < count; i++) {
        OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(kextsToProcess, i);

        int loadResult = processKext(theKext, toolArgs, &fatal);

       /* Save the first non-OK loadResult as the return value.
        */
        if (result == EX_OK && loadResult != EX_OK) {
            result = loadResult;
        }
        if (fatal) {
            goto finish;
        }
    }
finish:
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus
processKext(
    OSKextRef      aKext,
    KextutilArgs * toolArgs,
    Boolean      * fatal)
{
    ExitStatus result   = EX_OK;
    bool       allowed  = FALSE;
    char       kextPathCString[PATH_MAX];
    OSKextRef  ownedKext = NULL; // must release

    if (!CFURLGetFileSystemRepresentation(OSKextGetURL(aKext),
        /* resolveToBase */ false,
        (u_char *)kextPathCString, sizeof(kextPathCString))) {

        OSKextLogMemError();
        result = EX_OSERR;
        *fatal = true;
        goto finish;
    }

    result = runTestsOnKext(aKext, kextPathCString, toolArgs, fatal);
    if (result != EX_OK) {
        goto finish;
    }

   /* If there's no more work to do beyond printing test results,
    * skip right to the end.
    */
    if (!toolArgs->doLoad &&
        !toolArgs->doStartMatching &&
        !toolArgs->symbolDirURL) {

        goto finish;
    }

    if (toolArgs->doLoad) {
        /* Perform staging to ensure the kext is in a SIP protected location.
         */
        aKext = createStagedKext(aKext);
        if (!aKext) {
            OSKextLog(NULL,
                      kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                      kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                      "%s could not be staged, failing.",
                      kextPathCString);
            result = kOSKextReturnNotLoadable;
            goto finish;
        }

        // aKext returned by staging is owned here, so release it at the end of the loop.
        ownedKext = aKext;

        /* Since OSKextIsAuthentic performs an exclude list check and considers
         * the kext not loadable, this check must happen prior to allow proper
         * logging and notifications.
         */
        if (OSKextIsInExcludeList(aKext, false)) {
            /* notify kextd we are trying to load an excluded kext.
             * <rdar://problem/12811081>
             */
            CFMutableDictionaryRef      myAlertInfoDict = NULL; // must release

            addKextToAlertDict(&myAlertInfoDict, aKext);
            if (myAlertInfoDict) {
                postNoteAboutKexts(CFSTR("Excluded Kext Notification"),
                                   myAlertInfoDict );
                SAFE_RELEASE(myAlertInfoDict);
            }

            messageTraceExcludedKext(aKext);
            OSKextLog(NULL,
                      kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                      kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                      "%s is in exclude list; omitting.", kextPathCString);
            result = kOSKextReturnNotLoadable;
            *fatal = true;
            goto finish;
        }

        if (!OSKextIsAuthentic(aKext)) {
            OSKextLog(NULL,
                      kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                      "ERROR: authentication checks failed for %s, will not load",
                      kextPathCString);
            result = kOSKextReturnNotLoadable;
            goto finish;
        }

        if (!OSKextAuthenticateDependencies(aKext)) {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogErrorLevel | kOSKextLogLoadFlag |
                      kOSKextLogDependenciesFlag | kOSKextLogIPCFlag,
                      "Authentication checks failed in dependencies for kext load request.");
            result = kOSKextReturnDependencyLoadError;
            goto finish;
        }

        allowed = TRUE;
        result = loadKext(aKext, kextPathCString, toolArgs, fatal);
    }
    if (result != EX_OK) {
        goto finish;
    }

    if (toolArgs->doLoad) {
        /* <rdar://problem/12435992> */
        recordKextLoadForMT(aKext, true);
    }

   /* Reread loaded kext info to reflect newly-loaded kexts
    * if we need to save symbols (which requires load addresses)
    * or do interactive stuff.
    *
    * xxx - Could optimize OSKextReadLoadedKextInfo() with list
    * xxx - of kextIdentifiers from load list.
    */
    if (toolArgs->doLoad &&
        (toolArgs->symbolDirURL ||
        toolArgs->interactiveLevel != kOSKextExcludeNone)) {

        OSReturn readLoadedResult =  OSKextReadLoadedKextInfo(
            /* kextIdentifiers */ NULL,
            /* flushDependencies */ true);
        if (kOSReturnSuccess != readLoadedResult) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                "Failed to reread load info after loading %s - %s.",
                kextPathCString,
                safe_mach_error_string(readLoadedResult));
            result = EX_OSERR;
            // xxx - fatal?
            goto finish;
        }
    }

    if (toolArgs->symbolDirURL) {
        result = generateKextSymbols(aKext, kextPathCString, toolArgs,
            /* save? */ TRUE, fatal);
        if (result != EX_OK) {
            goto finish;
        }
    }

    if (toolArgs->doLoad ||
        toolArgs->doStartMatching) {

        result = startKextsAndSendPersonalities(aKext, toolArgs,
            fatal);
        if (result != EX_OK) {
            goto finish;
        }
    }

finish:
#if HAVE_DANGERZONE
    if (aKext) {
        dzRecordKextLoadBypass(aKext, allowed);
    }
#endif // HAVE_DANGERZONE
    SAFE_RELEASE(ownedKext);
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus
runTestsOnKext(
    OSKextRef      aKext,
    char         * kextPathCString,
    KextutilArgs * toolArgs,
    Boolean      * fatal)
{
    ExitStatus     result        = EX_OK;
    ExitStatus     tempResult    = EX_OK;
    Boolean        kextLooksGood = true;
    Boolean        tryLink       = false;
    OSKextLogSpec  logFilter     = OSKextGetLogFilter(/* kernel? */ false);
    OSStatus       sigResult     = 0;
    OSKextRef      ownedKext     = NULL; // must release

   /* Print message if not loadable in safe boot, but keep going
    * for further test results.
    */
    if (toolArgs->safeBootMode &&
        !OSKextIsLoadableInSafeBoot(aKext)) {

        Boolean        mustQualify = (toolArgs->doLoad || toolArgs->doStartMatching);
        OSKextLogSpec  msgLogLevel = kOSKextLogErrorLevel;

        if (mustQualify) {
            msgLogLevel = kOSKextLogWarningLevel;
        }

        OSKextLog(/* kext */ NULL,
            msgLogLevel | kOSKextLogLoadFlag,
            "%s%s is not eligible for loading during safe boot.",
            // label it just a notice if we won't be loading
            mustQualify ? "" : "Notice: ",
            kextPathCString);

        if (mustQualify) {
            SET_FIRST_FAILURE(result, kKextutilExitSafeBoot);
        }
    }

    if (OSKextHasLogOrDebugFlags(aKext)) {
        // xxx - check newline
        OSKextLog(/* kext */ NULL,
            kOSKextLogWarningLevel | kOSKextLogLoadFlag,
            "Notice: %s has debug properties set.", kextPathCString);
    }

   /* Run the tests for this kext. These would normally be done during
    * a load anyhow, but we need the results up-front. *Always* call
    * the test function before the && so it actually runs; we want all
    * tests performed, with some exceptions:
    *
    * Staging cannot be performed as any non-root user because the user
    * will not be able to write to the staging location.
    *
    * System policy checks are meant to be called in the load path only,
    * so avoid calling them in any scenario where a load could not occur.
    * Otherwise, this could be a security issue (privilege escalation) when
    * the user approves.
    *
    * Skip these checks if the process is not running as root.
    */
    kextLooksGood = OSKextValidate(aKext) && kextLooksGood;
    if (!toolArgs->skipAuthentication) {
        if (isRunningAsRoot()) {
            ownedKext = createStagedKext(aKext);
            if (!ownedKext) {
                OSKextLog(aKext,
                          kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                          "Kext failed to stage properly: %s:",
                          kextPathCString);
                kextLooksGood = false;
            } else {
                // If staging succeeded, make sure theKext refers to the new kext.
                aKext = ownedKext;
            }
        } else {
            OSKextLog(/* kext */ NULL,
                      kOSKextLogWarningLevel | kOSKextLogLoadFlag,
                      "Skipping staging and system policy checks because not running as "
                      "root, expect staging errors.");
        }

        if (!OSKextAuthenticate(aKext)) {
            kextLooksGood = false;

            // If authentication fails and the user is running as root, check
            // whether it was due to system policy so the tool can fail with
            // an error code documented in TN2459.
            if (isRunningAsRoot() && !SPAllowKextLoad(aKext)) {
                SET_FIRST_FAILURE(result, kKextutilExitSystemPolicy);
            }
        }
    }
    if (!toolArgs->skipDependencies) {
        kextLooksGood = OSKextResolveDependencies(aKext) && kextLooksGood;
        kextLooksGood = OSKextValidateDependencies(aKext) && kextLooksGood;
        if (!toolArgs->skipAuthentication) {
            kextLooksGood = OSKextAuthenticateDependencies(aKext) && kextLooksGood;
        }
    }

   /* Check for safe boot loadability. Note we do each check separately so as
    * not so short-circuit the checks; we want all diagnostics available.
    */
    if (toolArgs->safeBootMode) {
        kextLooksGood = OSKextIsLoadableInSafeBoot(aKext) && kextLooksGood;
        kextLooksGood = OSKextDependenciesAreLoadableInSafeBoot(aKext) && kextLooksGood;
    }

    /* Check code signature for diagnotic messages.
     */
    sigResult = checkKextSignature(aKext, false, true);
    if (sigResult != errSecSuccess) {
        if ((logFilter & kOSKextLogLevelMask) >= kOSKextLogErrorLevel) {
            OSKextLog(aKext,
                      kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                      "Code Signing Failure: %s",
                      (sigResult == errSecCSUnsigned
                       ? "not code signed" : "code signature is invalid") );
            OSKextLogDiagnostics(aKext, kOSKextDiagnosticsFlagAll);
        }
    }

   /*****
    * Print diagnostics/warnings as needed, set failure status if kext can't be used and
    * no other check has already set a specific failure status.
    */
    if (!kextLooksGood) {
        if ((logFilter & kOSKextLogLevelMask) >= kOSKextLogErrorLevel) {
            OSKextLog(aKext,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Diagnostics for %s:",
                kextPathCString);
            OSKextLogDiagnostics(aKext, kOSKextDiagnosticsFlagAll);
        }
        SET_FIRST_FAILURE(result, kKextutilExitKextBad);
        goto finish;
    }

   /* Print diagnostics/warnings as needed, set status if kext can't be used.
    */
    if (result == EX_OK) {
        if ((logFilter & kOSKextLogLevelMask) >= kOSKextLogWarningLevel) {
            // xxx - used to print "%s has potential problems:", kextPathCString
            OSKextLogDiagnostics(aKext, kOSKextDiagnosticsFlagWarnings);
        }
    }

   /* Do a trial link if we aren't otherwise linking, and replace a non-error
    * result with the tempResult of linking.
    */
    tryLink = !toolArgs->doLoad &&
        !toolArgs->symbolDirURL &&
        kextLooksGood;
    tryLink = tryLink &&
        ((OSKextGetArchitecture() == OSKextGetRunningKernelArchitecture()) ||
        toolArgs->kernelURL);
    if (tryLink) {

        tempResult = generateKextSymbols(aKext,
            kextPathCString, toolArgs, /* save? */ false, fatal);
        tryLink = true;
        if (result == EX_OK) {
            result = tempResult;
        }
        // Do not goto finish from here! We want to print diagnostics first.
    }

    if (result == EX_OK) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogBasicLevel | kOSKextLogLoadFlag,
            "%s appears to be loadable (%sincluding linkage for on-disk libraries).",
            kextPathCString, tryLink ? "" : "not ");
    }

#if 0
// just testing this
    OSKextLogDependencyGraph(aKext, /* bundleIDs */ true,
        /* linkGraph */ true);
#endif

finish:
    SAFE_RELEASE(ownedKext);
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus
loadKext(
    OSKextRef      aKext,
    char         * kextPathCString,
    KextutilArgs * toolArgs,
    Boolean      * fatal)
{
    ExitStatus         result           = EX_OK;
    OSKextExcludeLevel startExclude     = toolArgs->interactiveLevel;
    OSKextExcludeLevel matchExclude     = toolArgs->interactiveLevel;
    CFArrayRef         personalityNames = toolArgs->personalityNames;
    CFStringRef        kextIdentifier   = NULL; // do not release
    OSReturn           loadResult       = kOSReturnError;

   /* INTERACTIVE: ask if ok to load kext and its dependencies
    */
    if (toolArgs->interactiveLevel != kOSKextExcludeNone) {

        switch (user_approve(/* ask_all */ FALSE, /* default_answer */ REPLY_YES,
            "Load %s and its dependencies into the kernel",
            kextPathCString)) {

            case REPLY_NO:
                fprintf(stderr, "Not loading %s.", kextPathCString);
                goto finish;  // result is EX_OK!
                break;
            case REPLY_YES:
                break;
            default:
                fprintf(stderr, "Failed to read response.");
                result = EX_SOFTWARE;
                *fatal = true;
                goto finish;
                break;
        }
    }

    OSKextLog(/* kext */ NULL, kOSKextLogBasicLevel | kOSKextLogLoadFlag,
        "Loading %s.", kextPathCString);

   /* Mask out the personality/matching args as required.
    */
    if (toolArgs->interactiveLevel != kOSKextExcludeNone) {
        personalityNames = NULL;
    }
    if (!toolArgs->doStartMatching) {
        matchExclude = kOSKextExcludeAll;
    }

    if (OSKextDeclaresUserExecutable(aKext)) {
        /* By design, kextutil is unable to start dext daemons on its own.
         * Instead, we ask kextd to do the load & start the daemon for us.
         * KextManager will look up kextd's port automagically. */
        kextIdentifier = OSKextGetIdentifier(aKext);
        loadResult = KextManagerLoadKextWithIdentifier(kextIdentifier, toolArgs->scanURLs);
    } else { /* !OSKextDeclaresUserExecutable(aKext) */
        loadResult = OSKextLoadWithOptions(aKext,
            startExclude, matchExclude, personalityNames,
            /* disableAutounload */ (startExclude != kOSKextExcludeNone));
    }

    if (loadResult == kOSReturnSuccess) {
        OSKextLog(/* kext */ NULL, kOSKextLogBasicLevel | kOSKextLogLoadFlag,
            "%s successfully loaded (or already loaded).",
            kextPathCString);
    } else {
        OSKextLog(/* kext */ NULL, kOSKextLogBasicLevel | kOSKextLogLoadFlag,
            "Failed to load %s - %s.",
            kextPathCString, safe_mach_error_string(loadResult));
    }

    if (loadResult == kOSKextReturnLinkError) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Check library declarations for your kext with kextlibs(8).");
    }

    if (loadResult != kOSReturnSuccess) {
        result = kKextutilExitLoadFailed;
    }

finish:
    return result;
}

/*******************************************************************************
*******************************************************************************/
// xxx - generally need to figure out when to flush what

ExitStatus generateKextSymbols(
    OSKextRef      aKext,
    char         * kextPathCString,
    KextutilArgs * toolArgs,
    Boolean        saveFlag,
    Boolean      * fatal)
{
    ExitStatus         result      = EX_OK;
    CFArrayRef         loadList    = NULL;    // must release
    CFDictionaryRef    kextSymbols = NULL;    // must release
    const NXArchInfo * archInfo    = NULL;    // do not free
    CFIndex            count, i;

    if (saveFlag && !toolArgs->symbolDirURL) {
        result = EX_USAGE;
        *fatal = TRUE;
        goto finish;
    }

    // xxx - we might want to check these before processing any kexts
    if (OSKextIsKernelComponent(aKext)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogLoadFlag,
            "%s is a kernel component; no symbols to generate.",
            kextPathCString);
        result = EX_DATAERR;
        goto finish;
    }

    if (OSKextIsInterface(aKext)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogLoadFlag,
            "%s is a an interface kext; no symbols to generate.",
            kextPathCString);
        result = EX_DATAERR;
        goto finish;
    }

    if (OSKextDeclaresUserExecutable(aKext)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogLoadFlag,
            "%s is a user extension; no symbols to generate.",
            kextPathCString);
        result = EX_DATAERR;
        goto finish;
    }

    archInfo = OSKextGetArchitecture();
    if (!OSKextSupportsArchitecture(aKext, archInfo)) {
        int native = (archInfo == NXGetLocalArchInfo());
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "%s does not contain code for %sarchitecture%s%s.",
            kextPathCString,
            native ? "this computer's " : "",
            native ? "" : " ",
            native ? "" : archInfo->name);
        result = kKextutilExitArchNotFound;
        goto finish;
    }

    /*****
     * If we don't have a load address for aKext, ask for load addresses for it
     * and any of its dependencies.
     * NOTE (9656777) - OSKextNeedsLoadAddressForDebugSymbols() needs to be
     * called even if we loaded the kext.  The reason is that loading the kext
     * does not necessarily mean the load address was set in the load info
     * kept in the kernel.  Calling OSKextNeedsLoadAddressForDebugSymbols()
     * will implicitly set the load address if the kernel has it thus
     * avoiding having to ask the user for it.  And without that load address
     * we silently give back a partial symbol file that gdb dislikes.
     */
    if (saveFlag &&
        OSKextNeedsLoadAddressForDebugSymbols(aKext)) {

        loadList = OSKextCopyLoadList(aKext, /* needAll */ true);
        if (!loadList) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                "Can't resolve dependencies for %s.",
                kextPathCString);
            result = EX_SOFTWARE;
            *fatal = true;
            goto finish;
        }

       /*****
        * For each kext w/o an address in loadAddresses, ask for an address.
        */

        fprintf(stderr, "\nEnter the hexadecimal load addresses for these extensions\n"
            "(press Return to skip symbol generation for an extension):\n\n");

        count = CFArrayGetCount(loadList);
        for (i = 0; i < count; i++) {
            OSKextRef thisKext = (OSKextRef)CFArrayGetValueAtIndex(loadList, i);
            Boolean   mainNeedsAddress = ((i + 1) == count);

            do {
                switch (requestLoadAddress(thisKext)) {
                    case -1: // error
                        result = EX_SOFTWARE;
                        goto finish;
                        break;
                    case 0: // user cancel
                        fprintf(stderr, "\nuser canceled address input; exiting\n");
                        result = kKextutilExitUserAbort;
                        goto finish;
                        break;
                    case 1: // ok to continue
                        break;
                } /* switch */

               /* If we didn't get a load address for the main kext, the user
                * probably hit Return too many times.
                */
                if (mainNeedsAddress && OSKextNeedsLoadAddressForDebugSymbols(aKext)) {
                    switch (user_approve(/* ask_all */ FALSE, /* default_anser */ REPLY_NO,
                        "\n%s is the main extension; really skip", kextPathCString)) {
                        case REPLY_NO:
                            break;
                        case REPLY_YES:
                            result = EX_OK;
                            goto finish;  // skip symbol gen.
                        default:
                            result = EX_SOFTWARE;
                            goto finish;
                    }
                }
            } while (mainNeedsAddress && OSKextNeedsLoadAddressForDebugSymbols(aKext));
        }
    }

    kextSymbols = OSKextGenerateDebugSymbols(aKext, toolArgs->kernelFile);
    if (!kextSymbols) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Check library declarations for your kext with kextlibs(8).");
        result = kKextutilExitKextBad;  // more probably than an EX_OSERR
        *fatal = true;
        goto finish;
    }

    if (saveFlag) {
        SaveFileContext saveFileContext;
        saveFileContext.saveDirURL = toolArgs->symbolDirURL;
        saveFileContext.overwrite = toolArgs->overwriteSymbols;
        saveFileContext.fatal = false;
        CFDictionaryApplyFunction(kextSymbols, &saveFile, &saveFileContext);
        if (saveFileContext.fatal) {
            *fatal = true;
            goto finish;
        }
    }

finish:
    SAFE_RELEASE(loadList);
    SAFE_RELEASE(kextSymbols);

    return result;
}


/*******************************************************************************
*******************************************************************************/
int
requestLoadAddress(
    OSKextRef aKext)
{
    int           result          = -1;
    char        * bundleIDCString = NULL;  // must free
    char        * user_response   = NULL;  // must free
    char        * scan_pointer    = NULL;  // do not free
    uint64_t      address = 0;
    Boolean       eof = false;

    bundleIDCString = createUTF8CStringForCFString(
        OSKextGetIdentifier(aKext));
    if (!bundleIDCString) {
        goto finish;
    }

    if (OSKextNeedsLoadAddressForDebugSymbols(aKext)) {

        while (1) {
            SAFE_FREE(user_response);

            user_response = (char *)user_input(&eof, "%s:", bundleIDCString);
            if (eof) {
                result = 0;
                goto finish;
            }
            if (!user_response) {
                goto finish;
            }

           /* User wants to skip this one, don't set address & return success.
            */
            if (user_response[0] == '\0') {
                result = 1;
                goto finish;
                break;
            }

            errno = 0;
            address = strtoull(user_response, &scan_pointer, 16);

            if (address == ULONG_LONG_MAX && errno == ERANGE) {
                fprintf(stderr, "input address %s is too large; try again\n",
                    user_response);
                continue;
            } else if (address == 0 && errno == EINVAL) {
                fprintf(stderr, "no address found in input '%s'; try again\n",
                    user_response);
                continue;
            } else if (address == 0) {
                fprintf(stderr, "invalid address %s\n",
                    user_response);
                continue;
            } else if (*scan_pointer != '\0') {
                fprintf(stderr,
                    "input '%s' not a plain hexadecimal address; try again\n",
                    user_response);
                continue;
            } else {
                break;
            }
        }

        OSKextSetLoadAddress(aKext, address);
    }

    result = 1;

finish:
    SAFE_FREE(bundleIDCString);
    SAFE_FREE(user_response);
    return result;
}

/*******************************************************************************
xxx - should we be using paths or bundleids?
*******************************************************************************/
ExitStatus startKextsAndSendPersonalities(
    OSKextRef      aKext,
    KextutilArgs * toolArgs,
    Boolean      * fatal)
{
    ExitStatus          result                      = EX_OK;
    CFArrayRef          loadList                    = NULL;   // must release
    CFMutableArrayRef   kextIdentifiers             = NULL;  // must release
    char              * kextIDCString               = NULL;   // must free
    char              * thisKextIDCString           = NULL;   // must free
    Boolean             startedAndPersonalitiesSent = TRUE;   // loop breaks if ever false
    Boolean             yesToAllKexts               = FALSE;
    Boolean             yesToAllKextPersonalities   = FALSE;
    char                kextPath[PATH_MAX];
    CFIndex             count, i;  // used unothodoxically!

    if (!CFURLGetFileSystemRepresentation(OSKextGetURL(aKext),
        /* resoveToBase */ false, (UInt8*)kextPath, sizeof(kextPath))) {

        strlcpy(kextPath, "(unknown)", sizeof(kextPath));
    }

    kextIDCString = createUTF8CStringForCFString(OSKextGetIdentifier(aKext));
    if (!kextIDCString) {
        OSKextLogMemError();
        result = EX_OSERR;
    }

    loadList = OSKextCopyLoadList(aKext, /* needAll */ true);
    if (!loadList) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't get dependency list for %s.",
            kextIDCString);
        result = EX_OSERR;
        goto finish;
    }

    count = CFArrayGetCount(loadList);
    if (!count) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Internal error - empty load list.");
        result = EX_SOFTWARE;
        *fatal = true;
        goto finish;
    }

   /* We'll be using this in a call to OSKextReadLoadedKextInfo()
    * if something goes wrong.
    */
    kextIdentifiers = CFArrayCreateMutable(CFGetAllocator(aKext),
        count, &kCFTypeArrayCallBacks);
    if (!kextIdentifiers) {
        OSKextLogMemError();
        result = EX_OSERR;
        goto finish;
    }

   /*****
    * For interactive loading to do gdb on start functions,
    * go through the load list and print the started status of each kext.
    */
    if (toolArgs->interactiveLevel != kOSKextExcludeNone && toolArgs->doLoad) {

        fprintf(stderr, "\n"
            "%s and its dependencies are now loaded, and started as listed below. "
            "You can now return to the debugger to set breakpoints before starting "
            "any kexts that need to be started.\n\n",
            kextPath);

       /* If we're only doing interactive for the main kext, bump the loop
        * index to the last one.
        */
        if (toolArgs->interactiveLevel == kOSKextExcludeKext) {
            i = count - 1;
        } else {
            i = 0;
        }
        for (/* see above */ ; i < count; i++) {
            OSKextRef thisKext = (OSKextRef)CFArrayGetValueAtIndex(
                loadList, i);
            const char * status = NULL;

            SAFE_FREE_NULL(thisKextIDCString);

            if (!CFURLGetFileSystemRepresentation(OSKextGetURL(thisKext),
                /* resoveToBase */ false, (UInt8*)kextPath, sizeof(kextPath))) {

                strlcpy(kextPath, "(unknown)", sizeof(kextPath));
            }

            thisKextIDCString = createUTF8CStringForCFString(OSKextGetIdentifier(thisKext));
            if (!thisKextIDCString) {
                OSKextLogMemError();
                result = EX_OSERR;
                goto finish;
            }

            if (OSKextIsInterface(thisKext)) {
                status = "interface, not startable";
            } else if (OSKextDeclaresUserExecutable(thisKext)) {
                status = "user extension, already started"; // TODO: dext debugging support? output PID?
            } else if (!OSKextDeclaresExecutable(thisKext)) {
                status = "no executable, not startable";
            } else if (OSKextIsStarted(thisKext)) {
                status = "already started";
            } else {
                status = "not started";
            }
            fprintf(stderr, "    %s - %s\n", thisKextIDCString, status);
        }

        fprintf(stderr, "\n");
    }

   /*****
    * Now go through and actually process each kext.
    */

   /* If we're only doing interactive for the main kext, bump the loop
    * index to the last one.
    */
    if (toolArgs->interactiveLevel == kOSKextExcludeKext) {
        i = count - 1;
    } else {
        i = 0;
    }
    for (/* see above */ ; i < count; i++) {
        OSKextRef thisKext = (OSKextRef)CFArrayGetValueAtIndex(
            loadList, i);

        SAFE_FREE_NULL(thisKextIDCString);

        if (!CFURLGetFileSystemRepresentation(OSKextGetURL(thisKext),
            /* resoveToBase */ false, (UInt8*)kextPath, sizeof(kextPath))) {

            strlcpy(kextPath, "(unknown)", sizeof(kextPath));
        }

        CFArrayAppendValue(kextIdentifiers, OSKextGetIdentifier(thisKext));

        thisKextIDCString = createUTF8CStringForCFString(OSKextGetIdentifier(thisKext));
        if (!thisKextIDCString) {
            OSKextLogMemError();
            result = EX_OSERR;
            goto finish;
        }

       /* Normally the kext is started when loaded, so only try to start it here
        * if we're in interactive mode and we loaded it into the kernel.
        */
        if (toolArgs->interactiveLevel != kOSKextExcludeNone && toolArgs->doLoad) {
            if (!OSKextDeclaresUserExecutable(thisKext) && !OSKextIsStarted(thisKext)) {
                result = startKext(thisKext, kextPath, toolArgs,
                    &startedAndPersonalitiesSent, &yesToAllKexts, fatal);
                if (result != EX_OK || !startedAndPersonalitiesSent) {
                    break;
                }
            }
        }

       /* Normally the kext's personalities are sent when the kext is loaded,
        * so send them from here only if we are in interactive mode or
        * if we are only sending personalities and not loading.
        * It's ok to send personalities again if they're already in the kernel;
        * that just restarts matching.
        */
        if (toolArgs->interactiveLevel != kOSKextExcludeNone ||
            (toolArgs->doStartMatching && !toolArgs->doLoad)) {

            result = sendPersonalities(thisKext, kextPath, toolArgs,
                /* isMain */ (i + 1 == count), &yesToAllKextPersonalities, fatal);
            if (result != EX_OK) {
                startedAndPersonalitiesSent = false;
                break;
            }
        }
    }

   /* If the whole graph didn't start, go backward through it unloading
    * any kext that didn't start. We could optimize this a bit by providing
    * a list of the kext identifiers in the load list.
    */
    if (!startedAndPersonalitiesSent) {
        OSReturn readLoadedResult = OSKextReadLoadedKextInfo(
            kextIdentifiers, /* flushDependencies */ false);

        if (kOSReturnSuccess != readLoadedResult) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogLoadFlag | kOSKextLogIPCFlag,
                "Failed to read load info after starting - %s.",
                safe_mach_error_string(readLoadedResult));
            result = EX_OSERR;
            *fatal = true;
        }

        for (i = count - 1; i >= 0; i--) {
            OSKextRef thisKext = (OSKextRef)CFArrayGetValueAtIndex(
                loadList, i);

            if (!CFURLGetFileSystemRepresentation(OSKextGetURL(thisKext),
                /* resoveToBase */ false, (UInt8*)kextPath, sizeof(kextPath))) {

                strlcpy(kextPath, "(unknown)", sizeof(kextPath));
            }

            if (!OSKextIsStarted(thisKext)) {
                OSReturn unloadResult = OSKextUnload(thisKext,
                    /* terminateIOServicesAndRemovePersonalities */ true);

                OSKextLog(/* kext */ NULL,
                    kOSKextLogStepLevel | kOSKextLogLoadFlag,
                    "Unloading kext %s after failing to start/send personalities.", kextPath);

                if (kOSReturnSuccess != unloadResult) {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                        "Failed to unload kext %s after failing to start/send personalities - %s.",
                        kextPath,
                        safe_mach_error_string(unloadResult));
                    result = EX_OSERR;
                } else {
                    OSKextLog(/* kext */ NULL,
                        kOSKextLogStepLevel | kOSKextLogLoadFlag,
                        "%s unloaded.", kextPath);
                }
            }
        }
    }

finish:
    SAFE_RELEASE(loadList);
    SAFE_RELEASE(kextIdentifiers);
    SAFE_FREE(kextIDCString);
    SAFE_FREE(thisKextIDCString);
    return result;
}

/*******************************************************************************
* Be explicit about setting started either way here.
*******************************************************************************/
ExitStatus startKext(
    OSKextRef      aKext,
    char         * kextPathCString,
    KextutilArgs * toolArgs,
    Boolean      * started,
    Boolean      * yesToAll,
    Boolean      * fatal)
{
    ExitStatus    result      = EX_OK;
    OSReturn      startResult = kOSReturnError;

// xxx - check on the log calls in interactive mode; use fprintf instead?

   /* Check if the dependency is still loaded. It should be, of course.
    * This error is not fatal for the overall program, but we leave
    * this function since we can't start any other dependencies
    * up to the main kext that was loaded.
    */
    if (!OSKextIsLoaded(aKext)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogLoadFlag,
            "%s is unexpectedly not loaded after loading!",
            kextPathCString);
        *started = false;
        result = EX_OSERR;
        goto finish;
    }

    if (FALSE == *yesToAll) {
        switch (user_approve(/* ask_all */ TRUE,
            /* default_answer */ REPLY_YES,
            "Start %s",
            kextPathCString)) {

            case REPLY_NO:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogBasicLevel | kOSKextLogLoadFlag,
                    "Not starting %s.",
                    kextPathCString);
                *started = false;
                goto finish;  // result is EX_OK!
                break;
            case REPLY_YES:
                break;
            case REPLY_ALL:
                fprintf(stderr, "Starting all kexts just loaded.\n");
                *yesToAll = TRUE;
                break;
            default:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Error: couldn't read response.");
                result = EX_SOFTWARE;
                *started = false;
                *fatal = true;
                goto finish;
                break;
        }
    }

    startResult = OSKextStart(aKext);
    if (kOSReturnSuccess != startResult) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogLoadFlag,
            "%s failed to start - %s.",
            kextPathCString,
            safe_mach_error_string(startResult));

        *started = false;
        result = EX_OSERR;
        goto finish;
    } else {
        OSKextLog(/* kext */ NULL,
            kOSKextLogBasicLevel | kOSKextLogLoadFlag,
            "%s started.", kextPathCString);
        *started = true;
    }

finish:
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus sendPersonalities(
    OSKextRef      aKext,
    char         * kextPathCString,
    KextutilArgs * toolArgs,
    Boolean        isMainFlag,
    Boolean      * yesToAllKextPersonalities,
    Boolean      * fatal)
{
    ExitStatus        result                  = EX_OK;
    CFDictionaryRef   kextPersonalities       = NULL;  // do not release
    CFMutableArrayRef namesToSend             = NULL;  // must release
    CFStringRef     * names                   = NULL;  // must free
    char            * nameCString             = NULL;  // must free
    OSReturn          sendPersonalitiesResult = kOSReturnError;
    Boolean           yesToAllPersonalities   = FALSE;
    CFIndex           count, i;

    kextPersonalities = OSKextGetValueForInfoDictionaryKey(aKext,
        CFSTR(kIOKitPersonalitiesKey));
    if (!kextPersonalities || !CFDictionaryGetCount(kextPersonalities)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogStepLevel | kOSKextLogLoadFlag,
            "%s has no personalities to send.",
            kextPathCString);
        goto finish;
    }

    if (toolArgs->interactiveLevel != kOSKextExcludeNone) {
        if (FALSE == *yesToAllKextPersonalities) {
            switch (user_approve(/* ask_all */ TRUE, /* default_answer */ REPLY_YES,
                "Send personalities for %s",
                kextPathCString)) {

                case REPLY_NO:
                    fprintf(stderr, "Not sending personalities for %s.", kextPathCString);
                    goto finish;  // result is EX_OK!
                    break;
                case REPLY_YES:
                    break;
                case REPLY_ALL:
                    fprintf(stderr, "Sending personalities for all kexts just loaded.\n");
                    *yesToAllKextPersonalities = TRUE;
                    break;
                default:
                    fprintf(stderr, "Error: couldn't read response.");
                    result = EX_SOFTWARE;
                    *fatal = true;
                    goto finish;
                    break;
            }
        }
    }

    namesToSend = CFArrayCreateMutable(kCFAllocatorDefault, 0,
        &kCFTypeArrayCallBacks);
    if (!namesToSend) {
        OSKextLogMemError();
        result = EX_OSERR;
        *fatal = true;
        goto finish;
    }

    count = CFDictionaryGetCount(kextPersonalities);
    names = (CFStringRef *)malloc(count * sizeof(CFStringRef));
    if (!names) {
        OSKextLogMemError();
        result = EX_OSERR;
        *fatal = true;
        goto finish;
    }
    CFDictionaryGetKeysAndValues(kextPersonalities,
        (const void **)names, NULL);

    for (i = 0; i < count; i++) {
        Boolean includeIt = TRUE;

        SAFE_FREE_NULL(nameCString);

        nameCString = createUTF8CStringForCFString(names[i]);
        if (!nameCString) {
            OSKextLogMemError();
            result = EX_OSERR;
            *fatal = true;
            goto finish;
        }

       /* If -p was used on the command line, only consider those personalities.
        */
        if (isMainFlag && CFArrayGetCount(toolArgs->personalityNames) > 0) {
            if (kCFNotFound == CFArrayGetFirstIndexOfValue(
                toolArgs->personalityNames,
                RANGE_ALL(toolArgs->personalityNames), names[i])) {

                continue;
            }
        }

        if (toolArgs->interactiveLevel != kOSKextExcludeNone &&
            FALSE == *yesToAllKextPersonalities) {

            if (FALSE == yesToAllPersonalities) {
                switch (user_approve(/* ask_all */ TRUE, /* default_answer */ REPLY_YES,
                    "Send personality %s", nameCString)) {

                    case REPLY_NO:
                        includeIt = FALSE;
                        break;
                    case REPLY_YES:
                        includeIt = TRUE;
                        break;
                    case REPLY_ALL:
                        fprintf(stderr, "Sending all personalities for %s.\n",
                            kextPathCString);
                        includeIt = TRUE;
                        yesToAllPersonalities = TRUE;
                        break;
                    default:
                        fprintf(stderr, "Error: couldn't read response.");
                        result = EX_SOFTWARE;
                        *fatal = true;
                        goto finish;
                        break;
                } /* switch */
            } /* if (!*yesToAll) */
        } /* if (toolArgs->interactiveLevel ... ) */

        if (includeIt) {
            CFArrayAppendValue(namesToSend, names[i]);
        }
    } /* for */

    OSKextLog(/* kext */ NULL,
        kOSKextLogStepLevel | kOSKextLogLoadFlag | kOSKextLogIPCFlag,
        "Sending personalities of %s to the IOCatalogue.",
        kextPathCString);

    sendPersonalitiesResult = OSKextSendKextPersonalitiesToKernel(aKext, namesToSend);
    if (kOSReturnSuccess != sendPersonalitiesResult) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogLoadFlag | kOSKextLogIPCFlag,
            "Failed to send personalities for %s - %s.",
            kextPathCString,
            safe_mach_error_string(sendPersonalitiesResult));

        result = EX_OSERR;
        goto finish;
    } else {
        OSKextLog(/* kext */ NULL,
            kOSKextLogStepLevel | kOSKextLogLoadFlag | kOSKextLogIPCFlag,
            "Personalities sent for %s.", kextPathCString);
    }

finish:
    SAFE_RELEASE(namesToSend);
    SAFE_FREE(names);
    SAFE_FREE(nameCString);
    return result;
}

/*******************************************************************************
*******************************************************************************/
Boolean serializeLoad(KextutilArgs * toolArgs, Boolean loadFlag)
{
    Boolean       result       = false;
    kern_return_t kern_result;
    int           lock_retries = LOCK_MAXTRIES;

    if (!loadFlag) {
        result = true;
        goto finish;
    }

    /*****
    * Serialize running kextload processes that are actually loading. Note
    * that we can't bail on failing to contact kextd, we can only print
    * warnings, since kextload may need to be run in single-user mode. We
    * do bail on hard OS errors though.
    */
    kern_result = bootstrap_look_up(bootstrap_port,
        KEXTD_SERVER_NAME, &sKextdPort);
    if (kern_result != KERN_SUCCESS) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogWarningLevel | kOSKextLogIPCFlag,
            "Can't contact kextd (continuing anyway) - %s.",
            bootstrap_strerror(kern_result));
    }
    if (sKextdPort != MACH_PORT_NULL) {
        kern_result = mach_port_allocate(mach_task_self(),
            MACH_PORT_RIGHT_RECEIVE, &sLockPort);
        if (kern_result != KERN_SUCCESS) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                "Can't allocate kext loading serialization mach port.");
            goto finish;
        }
        do {
            kern_result = kextmanager_lock_kextload(sKextdPort, sLockPort,
                &sLockStatus);
            if (kern_result != KERN_SUCCESS) {
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                    "Can't acquire kextload serialization lock; aborting.");
                goto finish;
            }

            if (sLockStatus == EBUSY) {
                --lock_retries;
                OSKextLog(/* kext */ NULL,
                    kOSKextLogWarningLevel | kOSKextLogIPCFlag,
                    "Kext loading serialization lock busy; "
                    "sleeping (%d retries left).",
                    lock_retries);
                sleep(LOCK_DELAY);
            }
        } while (sLockStatus == EBUSY && lock_retries > 0);

        if (sLockStatus != 0) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                "Can't acquire kextload serialization lock; aborting.");
            goto finish;
        } else {
            sLockTaken = true;
        }
    }

    result = true;
finish:
    return result;
}

/*******************************************************************************
* usage()
*******************************************************************************/
void usage(UsageLevel usageLevel)
{
    fprintf(stderr, "usage: %s [options] [--] [kext] ...\n"
      "\n", progname);

    if (usageLevel == kUsageLevelBrief) {
        fprintf(stderr, "use %s -%s for an explanation of each option\n",
            progname, kOptNameHelp);
        return;
    }

    fprintf(stderr, "kext: a kext bundle to load or examine\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s <bundle_id> (-%c):\n"
        "        load/use the kext whose CFBundleIdentifier is <bundle_id>\n",
        kOptNameBundleIdentifier, kOptBundleIdentifier);
    fprintf(stderr, "-%s <personality> (-%c):\n"
        "        send the named personality to the catalog\n",
        kOptNamePersonality, kOptPersonality);
    fprintf(stderr, "-%s <kext> (-%c):\n"
        "        consider <kext> as a candidate dependency\n",
        kOptNameDependency, kOptDependency);
    fprintf(stderr, "-%s <directory> (-%c):\n"
        "        look in <directory> for kexts\n",
        kOptNameRepository, kOptRepository);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c):\n"
        "        don't use repository caches; scan repository folders\n",
        kOptNameNoCaches, kOptNoCaches);
    fprintf(stderr, "-%s (-%c):\n"
        "        don't check for loaded kexts when resolving dependencies "
        "(deprecated)\n",
        kOptNameNoLoadedCheck, kOptNoLoadedCheck);
    fprintf(stderr, "-%s (-%c):\n"
        "        don't use system extension folders\n",
        kOptNameNoSystemExtensions, kOptNoSystemExtensions);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c):\n"
        "        interactive mode\n",
        kOptNameInteractive, kOptInteractive);
    fprintf(stderr, "-%s (-%c):\n"
        "        interactive mode for extension and all its dependencies\n",
        kOptNameInteractiveAll, kOptInteractiveAll);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c):\n"
        "        load & start only; don't start matching\n",
        kOptNameLoadOnly, kOptLoadOnly);
    fprintf(stderr, "-%s (-%c):\n"
        "        start matching only, by sending personalities; "
        "don't load executable\n",
        kOptNameMatchOnly, kOptMatchOnly);
    fprintf(stderr, "-%s (-%c):\n"
        "        neither load nor start matching\n",
        kOptNameNoLoad, kOptNoLoad);
    fprintf(stderr, "-%s <directory> (-%c):\n"
        "        write symbol files into <directory>\n",
        kOptNameSymbolsDirectory, kOptSymbolsDirectory);
    fprintf(stderr, "-%s <archname>:\n"
        "        use architecture <archnaem>\n",
        kOptNameArch);
    fprintf(stderr, "-%s <kext_id@address> (-%c):\n"
        "        <kext_id> is loaded at address (for symbol generation)\n",
        kOptNameAddress, kOptAddress);
    fprintf(stderr, "-%s (-%c):\n"
        "        get load addresses for kexts from what's loaded "
        "(for symbol generation)\n",
        kOptNameUseKernelAddresses, kOptUseKernelAddresses);
    fprintf(stderr, "-%s <kernelFile> (-%c):\n"
        "        link against <kernelFile> (default is /System/Library/Kernels/kernel)\n",
        kOptNameKernel, kOptKernel);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c):\n"
        "        quiet mode: print no informational or error messages\n",
        kOptNameQuiet, kOptQuiet);
    fprintf(stderr, "-%s [ 0-6 | 0x<flags> ] (-%c):\n"
        "        verbose mode; print info about analysis & loading\n",
        kOptNameVerbose, kOptVerbose);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c):\n"
        "        perform all diagnostic tests and print a report on each kext\n",
        kOptNameTests, kOptTests);
    fprintf(stderr, "-%s (-%c):\n"
        "        simulate safe boot mode for diagnostic tests\n",
        kOptNameSafeBoot, kOptSafeBoot);
    fprintf(stderr, "-%s (-%c):\n"
        "        don't authenticate kexts (for use during development)\n",
        kOptNameNoAuthentication, kOptNoAuthentication);
    fprintf(stderr, "-%s (-%c):\n"
        "        don't check dependencies when diagnosing with\n"
        "        -%s & -%s (-%c%c)\n",
        kOptNameNoResolveDependencies, kOptNoResolveDependencies,
        kOptNameNoLoad, kOptNameTests,
        kOptNoLoad, kOptTests);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c): print this message and exit\n",
        kOptNameHelp, kOptHelp);
    fprintf(stderr, "\n");

    fprintf(stderr, "--: end of options\n");
    return;
}
