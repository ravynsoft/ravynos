/*
 *  kextload_main.c
 *  kext_tools
 *
 *  Created by Nik Gervae on 11/08/08.
 *  Copyright 2008 Apple Inc. All rights reserved.
 *
 */
#include "kextload_main.h"
#include "kext_tools_util.h"
#include "security.h"
#include "staging.h"
#include "kextaudit.h"

#include <libc.h>
#include <servers/bootstrap.h>
#include <sysexits.h>
#include <Security/SecKeychainPriv.h>

#include <IOKit/kext/KextManager.h>
#include <IOKit/kext/KextManagerPriv.h>
#include <IOKit/kext/kextmanager_types.h>
#include <IOKit/kext/OSKextPrivate.h>

#pragma mark Constants
/*******************************************************************************
* Constants
*******************************************************************************/

#pragma mark Global/Static Variables
/*******************************************************************************
* Global/Static Variables
*******************************************************************************/
const char      * progname     = "(unknown)";
static Boolean    sKextdActive = FALSE;

#pragma mark Main Routine
/*******************************************************************************
* Global variables.
*******************************************************************************/
ExitStatus
main(int argc, char * const * argv)
{
    ExitStatus   result = EX_SOFTWARE;
    KextloadArgs toolArgs;

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
    result = readArgs(argc, argv, &toolArgs);
    if (result != EX_OK) {
        if (result == kKextloadExitHelp) {
            result = EX_OK;
        }
        goto finish;
    }

    result = checkArgs(&toolArgs);
    if (result != EX_OK) {
        goto finish;
    }

    result = checkAccess();
    if (result != EX_OK) {
        goto finish;
    }

   /*****
    * Assemble the list of URLs to scan, in this order (the OSKext lib inverts it
    * for last-opened-wins semantics):
    * 1. System repository directories (if not asking kextd to load).
    * 2. Named kexts (always given after -repository & -dependency on command line).
    * 3. Named repository directories (-repository/-r).
    * 4. Named dependencies get priority (-dependency/-d).
    *
    * #2 is necessary since one might try to run kextload on two kexts,
    * one of which depends on the other.
    */
    if (!sKextdActive) {
        CFArrayRef sysExtFolders = OSKextGetSystemExtensionsFolderURLs();
        if (!sysExtFolders) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Can't get system extensions folders.");
            result = EX_OSERR;
            goto finish;
        }
        CFArrayAppendArray(toolArgs.scanURLs,
            sysExtFolders, RANGE_ALL(sysExtFolders));
    }
    CFArrayAppendArray(toolArgs.scanURLs, toolArgs.kextURLs,
        RANGE_ALL(toolArgs.kextURLs));
    CFArrayAppendArray(toolArgs.scanURLs, toolArgs.repositoryURLs,
        RANGE_ALL(toolArgs.repositoryURLs));
    CFArrayAppendArray(toolArgs.scanURLs, toolArgs.dependencyURLs,
        RANGE_ALL(toolArgs.dependencyURLs));

    if (sKextdActive) {
        result = loadKextsViaKextd(&toolArgs);
    } else {
        result = loadKextsIntoKernel(&toolArgs);
    }

finish:

   /* We're actually not going to free anything else because we're exiting!
    */
    exit(result);

    SAFE_RELEASE(toolArgs.kextIDs);
    SAFE_RELEASE(toolArgs.dependencyURLs);
    SAFE_RELEASE(toolArgs.repositoryURLs);
    SAFE_RELEASE(toolArgs.kextURLs);
    SAFE_RELEASE(toolArgs.scanURLs);
    SAFE_RELEASE(toolArgs.allKexts);

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
    KextloadArgs * toolArgs)
{
    ExitStatus   result          = EX_USAGE;
    ExitStatus   scratchResult   = EX_USAGE;
    int          optchar;
    int          longindex;
    CFStringRef  scratchString   = NULL;  // must release
    CFURLRef     scratchURL      = NULL;  // must release
    uint32_t     i;

   /* Set up default arg values.
    */
    bzero(toolArgs, sizeof(*toolArgs));

   /*****
    * Allocate collection objects needed for reading args.
    */
    if (!createCFMutableArray(&toolArgs->kextIDs, &kCFTypeArrayCallBacks)         ||
        !createCFMutableArray(&toolArgs->dependencyURLs, &kCFTypeArrayCallBacks)  ||
        !createCFMutableArray(&toolArgs->repositoryURLs, &kCFTypeArrayCallBacks)  ||
        !createCFMutableArray(&toolArgs->kextURLs, &kCFTypeArrayCallBacks)        ||
        !createCFMutableArray(&toolArgs->scanURLs, &kCFTypeArrayCallBacks)) {

        result = EX_OSERR;
        OSKextLogMemError();
        exit(result);
    }

    while ((optchar = getopt_long_only(argc, (char * const *)argv,
        kOptChars, sOptInfo, &longindex)) != -1) {

        SAFE_RELEASE_NULL(scratchString);
        SAFE_RELEASE_NULL(scratchURL);

        switch (optchar) {
            case kOptHelp:
                usage(kUsageLevelFull);
                result = kKextloadExitHelp;
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

            case kOptDependency:
            case kOptRepository:
                scratchURL = CFURLCreateFromFileSystemRepresentation(
                    kCFAllocatorDefault,
                    (const UInt8 *)optarg, strlen(optarg), true);
                if (!scratchURL) {
                    OSKextLogStringError(/* kext */ NULL);
                    result = EX_OSERR;
                    goto finish;
                }
                CFArrayAppendValue((optchar == kOptDependency) ?
                    toolArgs->dependencyURLs : toolArgs->repositoryURLs,
                    scratchURL);
                break;

            case kOptQuiet:
                beQuiet();
                break;

            case kOptVerbose:
                scratchResult = setLogFilterForOpt(argc, argv, /* forceOnFlags */ 0);
                if (scratchResult != EX_OK) {
                    result = scratchResult;
                    goto finish;
                }
                break;

            case kOptNoCaches:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                    "Notice: -%s (-%c) ignored; use kextutil(8) to test kexts.",
                    kOptNameNoCaches, kOptNoCaches);
                break;

            case kOptNoLoadedCheck:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                    "Notice: -%s (-%c) ignored.",
                    kOptNameNoLoadedCheck, kOptNoLoadedCheck);
                break;

            case kOptTests:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                    "Notice: -%s (-%c) ignored; use kextutil(8) to test kexts.",
                    kOptNameTests, kOptTests);
                break;

            case 0:
                switch (longopt) {
                   default:
                        OSKextLog(/* kext */ NULL,
                            kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                            "Use kextutil(8) for development loading of kexts.");
                        goto finish;
                        break;
                }
                break;

            default:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogWarningLevel | kOSKextLogGeneralFlag,
                    "Use kextutil(8) for development loading of kexts.");
                goto finish;
                break;

        } /* switch (optchar) */
    } /* while (optchar = getopt_long_only(...) */

   /*****
    * Record the kext names from the command line.
    */
    for (i = optind; (int)i < argc; i++) {
        SAFE_RELEASE_NULL(scratchURL);
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
    SAFE_RELEASE(scratchURL);

    if (result == EX_USAGE) {
        usage(kUsageLevelBrief);
    }
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus
checkArgs(KextloadArgs * toolArgs)
{
    ExitStatus         result         = EX_USAGE;

    if (!CFArrayGetCount(toolArgs->kextURLs) &&
        !CFArrayGetCount(toolArgs->kextIDs)) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "No kernel extensions specified; name kernel extension bundles\n"
            "    following options, or use -%s (-%c).",
            kOptNameBundleIdentifier, kOptBundleIdentifier);
        goto finish;
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
ExitStatus checkAccess(void)
{
    ExitStatus    result         = EX_OK;
#if !TARGET_OS_EMBEDDED
    kern_return_t kern_result    = kOSReturnError;
    mach_port_t   kextd_port     = MACH_PORT_NULL;

    kern_result = bootstrap_look_up(bootstrap_port,
        (char *)KEXTD_SERVER_NAME, &kextd_port);

    if (kern_result == kOSReturnSuccess) {
        sKextdActive = TRUE;
    } else {
        if (geteuid() == 0) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogBasicLevel | kOSKextLogGeneralFlag |
                kOSKextLogLoadFlag | kOSKextLogIPCFlag,
                "Can't contact kextd; attempting to load directly into kernel. "
                "Dexts will not be loadable.");
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag |
                kOSKextLogLoadFlag | kOSKextLogIPCFlag,
                "Can't contact kextd; must run as root to load kexts.");
            result = EX_NOPERM;
            goto finish;
        }
    }

#else

    if (geteuid() != 0) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag |
            kOSKextLogLoadFlag | kOSKextLogIPCFlag,
            "You must be running as root to load kexts.");
        result = EX_NOPERM;
        goto finish;
    }

#endif /* !TARGET_OS_EMBEDDED */

finish:

#if !TARGET_OS_EMBEDDED
    if (kextd_port != MACH_PORT_NULL) {
        mach_port_deallocate(mach_task_self(), kextd_port);
    }
#endif /* !TARGET_OS_EMBEDDED */

    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus loadKextsViaKextd(KextloadArgs * toolArgs)
{
    ExitStatus result     = EX_OK;
    OSReturn   loadResult = kOSReturnError;
    char       scratchCString[PATH_MAX];
    CFIndex    count, index;

    count = CFArrayGetCount(toolArgs->kextIDs);
    for (index = 0; index < count; index++) {
        CFStringRef kextID  = CFArrayGetValueAtIndex(toolArgs->kextIDs, index);

        if (!CFStringGetCString(kextID, scratchCString, sizeof(scratchCString),
            kCFStringEncodingUTF8)) {

            strlcpy(scratchCString, "unknown", sizeof(scratchCString));
        }

        OSKextLog(/* kext */ NULL,
            kOSKextLogBasicLevel | kOSKextLogGeneralFlag |
            kOSKextLogLoadFlag | kOSKextLogIPCFlag,
            "Requesting load of %s.",
            scratchCString);

        loadResult = KextManagerLoadKextWithIdentifier(kextID,
            toolArgs->scanURLs);
        if (loadResult != kOSReturnSuccess) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogLoadFlag | kOSKextLogIPCFlag,
                "%s failed to load - %s; "
                "check the system/kernel logs for errors or try kextutil(8).",
                scratchCString, safe_mach_error_string(loadResult));
            if (result == EX_OK) {
                result = exitStatusForOSReturn(loadResult);
                // keep trying other kexts though
            }
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogBasicLevel | kOSKextLogGeneralFlag | kOSKextLogLoadFlag,
                "%s loaded successfully (or already loaded).",
                scratchCString);
        }
    }

    count = CFArrayGetCount(toolArgs->kextURLs);
    for (index = 0; index < count; index++) {
        CFURLRef kextURL = CFArrayGetValueAtIndex(toolArgs->kextURLs, index);
        if (!CFURLGetFileSystemRepresentation(kextURL, /* resolveToBase */ true,
            (UInt8 *)scratchCString, sizeof(scratchCString))) {

            strlcpy(scratchCString, "unknown", sizeof(scratchCString));
        }

        OSKextLog(/* kext */ NULL,
            kOSKextLogBasicLevel | kOSKextLogGeneralFlag |
            kOSKextLogLoadFlag | kOSKextLogIPCFlag,
            "Requesting load of %s.",
            scratchCString);

        loadResult = KextManagerLoadKextWithURL(kextURL,
            toolArgs->scanURLs);
        if (loadResult != kOSReturnSuccess) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogLoadFlag | kOSKextLogIPCFlag,
                "%s failed to load - %s; "
                "check the system/kernel logs for errors or try kextutil(8).",
                scratchCString, safe_mach_error_string(loadResult));
            if (result == EX_OK) {
                result = exitStatusForOSReturn(loadResult);
                // keep trying other kexts though
            }
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogBasicLevel | kOSKextLogGeneralFlag | kOSKextLogLoadFlag,
                "%s loaded successfully (or already loaded).",
                scratchCString);
        }
    }

    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus loadKextsIntoKernel(KextloadArgs * toolArgs)
{
    ExitStatus result     = EX_OK;
    OSReturn   loadResult = kOSReturnError;
    char       scratchCString[PATH_MAX];
    CFIndex    count, index;
    OSKextRef ownedKext = NULL;  // must release
#if !TARGET_OS_EMBEDDED
    Boolean         earlyBoot = false;
    Boolean         isNetbootEnvironment = false;
    AuthOptions_t   originalAuthOptions;
#endif

    OSKextLog(/* kext */ NULL,
        kOSKextLogProgressLevel | kOSKextLogGeneralFlag,
        "Reading extensions.");
    toolArgs->allKexts = OSKextCreateKextsFromURLs(kCFAllocatorDefault,
        toolArgs->scanURLs);
    if (!toolArgs->allKexts) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't read kexts from disk.");
        result = EX_OSERR;
        goto finish;
    }

#if !TARGET_OS_EMBEDDED
    // not perfect, but we check to see if kextd is running to determine
    // if we are in early boot.
    int     skc_result;

    earlyBoot = (isKextdRunning() == false);
    isNetbootEnvironment = isNetBooted();

    // Configure authentication required - do not use network or respect
    // system policy if we are in early boot.
    AuthOptions_t authOptions = {
        .allowNetwork = !earlyBoot,
        .isCacheLoad = false,
        .performFilesystemValidation = true,
        .performSignatureValidation = true,
        .requireSecureLocation = true,
        .respectSystemPolicy = !earlyBoot,
        .checkDextApproval = !earlyBoot,
        .is_kextcache = false,
    };
    _OSKextSetAuthenticationFunction(&authenticateKext, &authOptions);
    _OSKextSetStrictAuthentication(true);
    memcpy(&originalAuthOptions, &authOptions, sizeof(AuthOptions_t));

    /* Set up auditing of kext loads, see kextaudit.c */
    _OSKextSetLoadAuditFunction(&KextAuditLoadCallback);
    setVariantSuffix();

#endif

    count = CFArrayGetCount(toolArgs->kextIDs);
    for (index = 0; index < count; index++) {
        Boolean       isNetbootKext = false;
        OSKextRef     theKext = NULL;  // do not release
        CFStringRef   kextID  = CFArrayGetValueAtIndex(
                                                       toolArgs->kextIDs,
                                                       index);

        if (!CFStringGetCString(kextID, scratchCString, sizeof(scratchCString),
            kCFStringEncodingUTF8)) {

            strlcpy(scratchCString, "unknown", sizeof(scratchCString));
        }

        OSKextLog(/* kext */ NULL,
            kOSKextLogBasicLevel | kOSKextLogGeneralFlag |
            kOSKextLogLoadFlag | kOSKextLogIPCFlag,
            "Loading %s.",
            scratchCString);

        theKext = OSKextGetKextWithIdentifier(kextID);
        if (!theKext) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogLoadFlag | kOSKextLogIPCFlag,
                "Error: Kext %s - not found/unable to create.", scratchCString);
            result = exitStatusForOSReturn(kOSKextReturnNotFound);
            goto finish;
        }

        if (OSKextDeclaresUserExecutable(theKext)) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogLoadFlag | kOSKextLogIPCFlag,
                "Error: Dext %s can't be loaded while kextd is unreachable.", scratchCString);
            result = exitStatusForOSReturn(kOSKextReturnNotLoadable);
            goto finish;
        }

#if !TARGET_OS_EMBEDDED
        // Diskless netboot environment authentication workaround: 18367703
        isNetbootKext = ((CFStringCompare(kextID, CFSTR("com.apple.nke.asp-tcp"), 0) == kCFCompareEqualTo) ||
                         (CFStringCompare(kextID, CFSTR("com.apple.filesystems.afpfs"), 0) == kCFCompareEqualTo));

        if (isNetbootEnvironment && isNetbootKext) {
            // Disable authentication not available in diskless netboot environment.
            authOptions.performSignatureValidation = false;
            authOptions.requireSecureLocation = false;
            authOptions.respectSystemPolicy = false;
            OSKextLogCFString(NULL, kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                              CFSTR("Netboot, loading '%@'"), kextID);
        }

        if (authOptions.requireSecureLocation) {
            // Perform staging to ensure all kexts are in SIP protected locations.
            theKext = createStagedKext(theKext);
            if (!theKext) {
                OSKextLog(NULL,
                          kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                          kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                          "%s could not be staged, failing.",
                          scratchCString);
                result = exitStatusForOSReturn(kOSKextReturnNotLoadable);
                goto finish;
            }

            // theKext returned by staging is owned here, so make sure it gets released at the
            // end of the loop or in the error handler at the end of the function.
            ownedKext = theKext;
        }

        // Force authentication checks now so they can be reported gracefully.
        if (!OSKextIsAuthentic(theKext)) {
            OSKextLog(NULL,
                      kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                      kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                      "%s failed security checks; failing.", scratchCString);
            result = exitStatusForOSReturn(kOSKextReturnNotLoadable);
            goto finish;
        }

        if (!OSKextAuthenticateDependencies(theKext)) {
            OSKextLog(NULL,
                      kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                      kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                      "%s's dependencies failed security checks; failing.", scratchCString);
            result = exitStatusForOSReturn(kOSKextReturnNotLoadable);
            goto finish;
        }
#endif // not TARGET_OS_EMBEDDED

#if HAVE_DANGERZONE
        // Note: This code path is mainly only used in early boot before daemons aren't
        // available. While it is possible to exercise this code path after fully booting,
        // it is difficult and should be removed in the future in favor of ensuring
        // everything goes through kextd. For now, intentionally only logging allowed kexts.
        dzRecordKextLoadBypass(theKext, TRUE);
#endif // HAVE_DANGERZONE

        /* The codepath from this function will do any error logging
        * and cleanup needed.
        */
        loadResult = OSKextLoadWithOptions(theKext,
            /* statExclusion */ kOSKextExcludeNone,
            /* addPersonalitiesExclusion */ kOSKextExcludeNone,
            /* personalityNames */ NULL,
            /* delayAutounloadFlag */ false);

#if !TARGET_OS_EMBEDDED
        if (isNetbootEnvironment && isNetbootKext) {
            // Restore original authentication options for future loads.
            memcpy(&authOptions, &originalAuthOptions, sizeof(AuthOptions_t));
        }
#endif // not TARGET_OS_EMBEDDED

        if (loadResult != kOSReturnSuccess) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogLoadFlag | kOSKextLogIPCFlag,
                "%s failed to load - %s.",
                scratchCString, safe_mach_error_string(loadResult));
            if (result == EX_OK) {
                result = exitStatusForOSReturn(loadResult);
                // keep trying other kexts though
            }
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogBasicLevel | kOSKextLogGeneralFlag | kOSKextLogLoadFlag,
                "%s loaded successfully (or already loaded).",
                scratchCString);
        }

        // Clear ownedKext before iterating to the next kext ID.
        SAFE_RELEASE_NULL(ownedKext);
    }

    count = CFArrayGetCount(toolArgs->kextURLs);
    for (index = 0; index < count; index++) {
        Boolean       isNetbootKext = false;
        CFURLRef      kextURL        = CFArrayGetValueAtIndex(
            toolArgs->kextURLs,
            index);
        if (!CFURLGetFileSystemRepresentation(kextURL, /* resolveToBase */ true,
            (UInt8 *)scratchCString, sizeof(scratchCString))) {

            strlcpy(scratchCString, "unknown", sizeof(scratchCString));
        }

        OSKextLog(/* kext */ NULL,
            kOSKextLogBasicLevel | kOSKextLogGeneralFlag |
            kOSKextLogLoadFlag | kOSKextLogIPCFlag,
            "Loading %s.",
            scratchCString);

        OSKextRef theKext = NULL;  // do not release

       /* Use OSKextGetKextWithURL() to avoid double open error messages,
        * because we already tried to open all kexts above.
        * That means we don't log here if we don't find the kext.
        */
        loadResult = kOSKextReturnNotFound;
        theKext = OSKextGetKextWithURL(kextURL);
        if (theKext) {
            /* The codepath from OSKextLoadWithOptions will do any error logging
             * and cleanup needed.
             */
#if !TARGET_OS_EMBEDDED
            CFStringRef     myBundleID = NULL;         // do not release

            myBundleID = OSKextGetIdentifier(theKext);

            if (OSKextDeclaresUserExecutable(theKext)) {
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogLoadFlag | kOSKextLogIPCFlag,
                    "Error: Dext %s can't be loaded while kextd is unreachable.", scratchCString);
                result = exitStatusForOSReturn(kOSKextReturnNotLoadable);
                goto finish;
            }

            // Netboot environment authentication workaround: 18367703
            isNetbootKext = ((CFStringCompare(myBundleID, CFSTR("com.apple.nke.asp-tcp"), 0) == kCFCompareEqualTo) ||
                             (CFStringCompare(myBundleID, CFSTR("com.apple.filesystems.afpfs"), 0) == kCFCompareEqualTo));

            if (isNetbootEnvironment && isNetbootKext) {
                // Disable authentication not available in diskless netboot environment.
                authOptions.performSignatureValidation = false;
                authOptions.requireSecureLocation = false;
                authOptions.respectSystemPolicy = false;
                OSKextLogCFString(NULL, kOSKextLogErrorLevel | kOSKextLogLoadFlag,
                                  CFSTR("Netboot, loading '%@'"), myBundleID);
            }

            // Perform staging to ensure all kexts are in SIP protected locations.
            if (authOptions.requireSecureLocation) {
                theKext = createStagedKext(theKext);
                if (!theKext) {
                    OSKextLog(NULL,
                              kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                              kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                              "%s could not be staged, failing.",
                              scratchCString);
                    result = exitStatusForOSReturn(kOSKextReturnNotLoadable);
                    goto finish;
                }

                // theKext returned by staging is owned here, so make sure it gets released at the
                // end of the loop or in the error handler at the end of the function.
                ownedKext = theKext;
            }

            // Force authentication checks now so they can be reported gracefully.
            if (!OSKextIsAuthentic(theKext)) {
                OSKextLog(NULL,
                          kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                          kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                          "%s failed security checks; failing.", scratchCString);
                result = exitStatusForOSReturn(kOSKextReturnNotLoadable);
                goto finish;
            }

            if (!OSKextAuthenticateDependencies(theKext)) {
                OSKextLog(NULL,
                          kOSKextLogErrorLevel | kOSKextLogArchiveFlag |
                          kOSKextLogValidationFlag | kOSKextLogGeneralFlag,
                          "%s's dependencies failed security checks; failing.", scratchCString);
                result = exitStatusForOSReturn(kOSKextReturnNotLoadable);
                goto finish;
            }

#if HAVE_DANGERZONE
            // Note: This code path is mainly only used in early boot before daemons aren't
            // available. While it is possible to exercise this code path after fully booting,
            // it is difficult and should be removed in the future in favor of ensuring
            // everything goes through kextd. For now, intentionally only logging allowed kexts.
            dzRecordKextLoadBypass(theKext, TRUE);
#endif // HAVE_DANGERZONE

#endif // not TARGET_OS_EMBEDDED

            loadResult = OSKextLoadWithOptions(theKext,
                                               kOSKextExcludeNone,
                                               kOSKextExcludeNone,
                                               NULL,
                                               false);

#if !TARGET_OS_EMBEDDED
            if (isNetbootEnvironment && isNetbootKext) {
                // Restore original authentication options for future loads.
                memcpy(&authOptions, &originalAuthOptions, sizeof(AuthOptions_t));
            }
#endif // not TARGET_OS_EMBEDDED
        }
        if (loadResult != kOSReturnSuccess) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogLoadFlag | kOSKextLogIPCFlag,
                "%s failed to load - %s.",
                scratchCString, safe_mach_error_string(loadResult));
            if (result == EX_OK) {
                result = exitStatusForOSReturn(loadResult);
                // keep trying other kexts though
            }
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogBasicLevel | kOSKextLogGeneralFlag | kOSKextLogLoadFlag,
                "%s loaded successfully (or already loaded).",
                scratchCString);
        }

        // Clear ownedKext before iterating to the next kext URL.
        SAFE_RELEASE_NULL(ownedKext);
    }

finish:
    // In an error case, we may get here and still have a lingering ownership.
    SAFE_RELEASE(ownedKext);
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus exitStatusForOSReturn(OSReturn osReturn)
{
    ExitStatus result = EX_OSERR;

    switch (osReturn) {
    case kOSKextReturnNotPrivileged:
        result = EX_NOPERM;
        break;
    case kOSKextReturnSystemPolicy:
        result = kKextloadExitSystemPolicy;
        break;
    default:
        result = EX_OSERR;
        break;
    }
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
    fprintf(stderr, "-%s <kext> (-%c):\n"
        "        consider <kext> as a candidate dependency\n",
        kOptNameDependency, kOptDependency);
    fprintf(stderr, "-%s <directory> (-%c):\n"
        "        look in <directory> for kexts\n",
        kOptNameRepository, kOptRepository);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c):\n"
        "        quiet mode: print no informational or error messages\n",
        kOptNameQuiet, kOptQuiet);
    fprintf(stderr, "-%s [ 0-6 | 0x<flags> ] (-%c):\n"
        "        verbose mode; print info about analysis & loading\n",
        kOptNameVerbose, kOptVerbose);
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c): print this message and exit\n",
        kOptNameHelp, kOptHelp);
    fprintf(stderr, "\n");

    fprintf(stderr, "--: end of options\n");
    return;
}
