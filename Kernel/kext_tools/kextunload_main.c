/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#include <libc.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitServer.h>
#include <IOKit/kext/OSKextPrivate.h>

#include "kextunload_main.h"

const char * progname = "(unknown)";

/*******************************************************************************
*******************************************************************************/
int main(int argc, char * const * argv)
{
    ExitStatus        result        = EX_OK;
    KextunloadArgs    toolArgs;
    ExitStatus        scratchResult = EX_OK;
    Boolean           fatal         = false;


   /*****
    * Find out what my name is.
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

    result = readArgs(argc, argv, &toolArgs);
    if (result != EX_OK) {
        goto finish;
    }

    result = checkArgs(&toolArgs);
    if (result != EX_OK) {
        goto finish;
    }

   /* If given URLs, create OSKext objects for them so we can get
    * bundle identifiers (that's what IOCatalogueTerminate() expects).
    * If we failed to open one, keep going on but save the not-found
    * error for our exit status.
    */
    result = createKextsIfNecessary(&toolArgs);
    if (result != EX_OK && result != kKextunloadExitNotFound) {
        goto finish;
    }

   /* Do the terminates & unloads. Catch the first nonfatal error as our
    * exit and don't overwrite it with later nonfatal errors.
    * *Do* overwrite it with a fatal error if we got one.
    */
    scratchResult = terminateKextClasses(&toolArgs, &fatal);
    if (result == EX_OK && scratchResult != EX_OK) {
        result = scratchResult;
    }
    if (fatal) {
        result = scratchResult;
        goto finish;
    }

    scratchResult = unloadKextsByIdentifier(&toolArgs, &fatal);
    if (result == EX_OK && scratchResult != EX_OK) {
        result = scratchResult;
    }
    if (fatal) {
        result = scratchResult;
        goto finish;
    }

    scratchResult = unloadKextsByURL(&toolArgs, &fatal);
    if (result == EX_OK && scratchResult != EX_OK) {
        result = scratchResult;
    }
    if (fatal) {
        result = scratchResult;
        goto finish;
    }

finish:

    if (result == kKextunloadExitHelp) {
        result = EX_OK;
    }

    exit(result);

   /*****
    * Clean everything up.
    */
    SAFE_RELEASE(toolArgs.kextURLs);
    SAFE_RELEASE(toolArgs.kextClassNames);
    SAFE_RELEASE(toolArgs.kextBundleIDs);

    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus readArgs(int argc, char * const * argv, KextunloadArgs * toolArgs)
{
    ExitStatus   result          = EX_USAGE;
    ExitStatus   scratchResult   = EX_USAGE;
    int          optchar         = 0;
    int          longindex       = -1;
    CFStringRef  scratchString   = NULL;  // must release
    CFNumberRef  scratchNumber   = NULL;  // must release
    CFURLRef     scratchURL      = NULL;  // must release
    CFIndex      i;

    bzero(toolArgs, sizeof(*toolArgs));

   /* Default is to unload both kext and driver personalities.
    */
    toolArgs->terminateOption = kIOCatalogModuleUnload;

   /*****
    * Allocate collection objects needed for command line argument processing.
    */
    if (!createCFMutableArray(&toolArgs->kextURLs, &kCFTypeArrayCallBacks)    ||
        !createCFMutableArray(&toolArgs->kextBundleIDs, NULL /* C strings */) ||
        !createCFMutableArray(&toolArgs->kextClassNames, NULL /* C strings */)) {

        result = EX_OSERR;
        OSKextLogMemError();
        goto finish;
    }

   /*****
    * Process command-line arguments.
    */
    result = EX_USAGE;

    /*****
    * Process command line arguments.
    */
    while ((optchar = getopt_long_only(argc, (char * const *)argv,
        kOptChars, sOptInfo, &longindex)) != -1) {

        SAFE_RELEASE_NULL(scratchString);
        SAFE_RELEASE_NULL(scratchNumber);
        SAFE_RELEASE_NULL(scratchURL);

        switch (optchar) {
            case kOptHelp:
                usage(kUsageLevelFull);
                result = kKextunloadExitHelp;
                goto finish;
                break;

            case kOptBundleIdentifier:
            case kOptModule:
                addToArrayIfAbsent(toolArgs->kextBundleIDs, optarg);
                break;

            case kOptClassName:
                addToArrayIfAbsent(toolArgs->kextClassNames, optarg);
                break;
              break;

            case kOptPersonalitiesOnly:
              toolArgs->terminateOption = kIOCatalogModuleTerminate;
              break;

            case kOptQuiet:
                beQuiet();
                break;

            case kOptVerbose:
                scratchResult = setLogFilterForOpt(argc, argv,
                    /* forceOnFlags */ kOSKextLogKextOrGlobalMask);
                if (scratchResult != EX_OK) {
                    result = scratchResult;
                    goto finish;
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

        scratchURL = CFURLCreateFromFileSystemRepresentation(
            kCFAllocatorDefault,
            (const UInt8 *)argv[i], strlen(argv[i]), true);
        if (!scratchURL) {
            result = EX_OSERR;
            OSKextLogMemError();
            goto finish;
        }
        addToArrayIfAbsent(toolArgs->kextURLs, scratchURL);
    }

    result = EX_OK;
finish:
    SAFE_RELEASE(scratchString);
    SAFE_RELEASE(scratchNumber);
    SAFE_RELEASE(scratchURL);

    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus
checkArgs(KextunloadArgs * toolArgs)
{
    ExitStatus         result = EX_USAGE;

    if (geteuid() != 0) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "You must be running as root to unload kexts, "
            "terminate services, or remove driver personalities.");
        result = EX_NOPERM;
        goto finish;
    }

    if (!CFArrayGetCount(toolArgs->kextURLs)      &&
        !CFArrayGetCount(toolArgs->kextBundleIDs) &&
        !CFArrayGetCount(toolArgs->kextClassNames)) {

       /* Put an extra newline for readability.
        */
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "No kernel extensions specified.");
        usage(kUsageLevelBrief);
        goto finish;
    }

    result = EX_OK;

finish:
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus
createKextsIfNecessary(KextunloadArgs * toolArgs)
{
    ExitStatus result = EX_OK;
    OSKextRef  aKext  = NULL;   // must release
    CFIndex    count, i;

    if (!CFArrayGetCount(toolArgs->kextURLs)) {
        goto finish;
    }

    if (!createCFMutableArray(&toolArgs->kexts, &kCFTypeArrayCallBacks)) {
        result = EX_OSERR;
        goto finish;
    }

    count = CFArrayGetCount(toolArgs->kextURLs);
    for (i = 0; i < count; i++) {
        CFURLRef kextURL = CFArrayGetValueAtIndex(toolArgs->kextURLs, i);
        char     kextPath[PATH_MAX] = "(unknown)";

        CFURLGetFileSystemRepresentation(kextURL,
            /* resolveToBase */ false,
            (UInt8 *)kextPath,
            sizeof(kextPath));

        SAFE_RELEASE_NULL(aKext);
        aKext = OSKextCreate(kCFAllocatorDefault, kextURL);
        if (!aKext) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Can't create %s.", kextPath);
            result = kKextunloadExitNotFound;
            continue; // not fatal!
        }

        addToArrayIfAbsent(toolArgs->kexts, aKext);
    }

finish:
    SAFE_RELEASE(aKext);
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus
terminateKextClasses(KextunloadArgs * toolArgs, Boolean * fatal)
{
    ExitStatus    result      = EX_OK;
    kern_return_t kernResult;
    CFIndex       count, i;

    count = CFArrayGetCount(toolArgs->kextClassNames);
    for (i = 0; i < count; i++) {
        char * className = NULL;  // do not free

        className = (char *)CFArrayGetValueAtIndex(toolArgs->kextClassNames, i);

        kernResult = IOCatalogueTerminate(kIOMasterPortDefault,
            kIOCatalogServiceTerminate,
            className);

        if (kernResult == kIOReturnNotPrivileged) {
             OSKextLog(/* kext */ NULL,
                 kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                 "You must be running as root to terminate IOService instances.");
             result = kKextunloadExitNotPrivileged;
             *fatal = true;
             goto finish;

        } else if (kernResult != KERN_SUCCESS) {
            result = kKextunloadExitPartialFailure;
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                "Failed to terminate class %s - %s.",
                className, safe_mach_error_string(kernResult));
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogBasicLevel | kOSKextLogIPCFlag,
                "All instances of class %s terminated.",
                className);
        }
    }

finish:
    if (result == kKextunloadExitPartialFailure) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Check the system/kernel logs for error messages from the I/O Kit.");
    }

    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus unloadKextsByIdentifier(KextunloadArgs * toolArgs, Boolean * fatal)
{
    ExitStatus      result = EX_OK;
    CFStringRef     kextIdentifier = NULL;  // must release
    CFIndex         count, i;

    count = CFArrayGetCount(toolArgs->kextBundleIDs);
    for (i = 0; i < count; i++) {
        char       * kextIDCString = NULL;  // do not free
        ExitStatus   thisResult;

        SAFE_RELEASE_NULL(kextIdentifier);
        kextIDCString = (char *)CFArrayGetValueAtIndex(toolArgs->kextBundleIDs, i);
        kextIdentifier = CFStringCreateWithCString(kCFAllocatorDefault,
            kextIDCString, kCFStringEncodingUTF8);
        thisResult = unloadKextWithIdentifier(kextIdentifier, toolArgs, fatal);

       /* Only nab the first nonfatal error.
        */
        if (result == EX_OK && thisResult != EX_OK) {
            result = thisResult;
        }
        if (*fatal) {
            result = thisResult;
            goto finish;
        }
    }

finish:
    SAFE_RELEASE(kextIdentifier);

   /* If we didn't suffer a catastrophic error, but only a routine
    * terminate failure, then recommend checking the system log.
    * Note that for unloads we capture kernel error logs from the OSKext
    * subsystem and print them to stderr.
    */
    if (!*fatal &&
        result != EX_OK &&
        toolArgs->terminateOption == kIOCatalogModuleTerminate) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Check the system/kernel logs for error messages from the I/O Kit.");
    }
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus unloadKextsByURL(KextunloadArgs * toolArgs, Boolean * fatal)
{
    ExitStatus      result  = EX_OK;
    CFIndex         count, i;

   /* This is one CF collection we only allocate when needed.
    */
    if (!toolArgs->kexts) {
        goto finish;
    }

    count = CFArrayGetCount(toolArgs->kexts);
    for (i = 0; i < count; i++) {
        ExitStatus  thisResult = EX_OK;
        OSKextRef   aKext      = NULL;  // do not release
        CFURLRef    kextURL    = NULL;  // do not release
        CFStringRef kextID     = NULL;  // do not release
        char        kextPath[PATH_MAX];

        aKext = (OSKextRef)CFArrayGetValueAtIndex(toolArgs->kexts, i);
        kextURL = OSKextGetURL(aKext);
        kextID = OSKextGetIdentifier(aKext);

        if (!CFURLGetFileSystemRepresentation(kextURL,
            /* resolveToBase */ false,
            (UInt8 *)kextPath,
            sizeof(kextPath))) {

            memcpy(kextPath, "(unknown)", sizeof("(unknown)"));
            continue;
        }

        thisResult = unloadKextWithIdentifier(kextID, toolArgs, fatal);

       /* Only nab the first nonfatal error.
        */
        if (result == EX_OK && thisResult != EX_OK) {
            result = thisResult;
        }
        if (*fatal) {
            result = thisResult;
            goto finish;
        }
    }

finish:
   /* If we didn't suffer a catastrophic error, but only a routine
    * terminate failure, then recommend checking the system log.
    * Note that for unloads we capture kernel error logs from the OSKext
    * subsystem and print them to stderr.
    */
    if (!*fatal &&
        result != EX_OK &&
        toolArgs->terminateOption == kIOCatalogModuleTerminate) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogIPCFlag,
            "Check the system/kernel logs for error messages from the I/O Kit.");
    }
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus unloadKextWithIdentifier(
    CFStringRef      kextIdentifier,
    KextunloadArgs * toolArgs,
    Boolean        * fatal)
{
    ExitStatus      result = EX_OK;
    char          * kextIdentifierCString = NULL;  // must free
    kern_return_t   kernResult;

    if (!kextIdentifierCString) {
        kextIdentifierCString = createUTF8CStringForCFString(kextIdentifier);
        if (!kextIdentifierCString) {
            OSKextLogMemError();
            result = EX_OSERR;
            *fatal = true;
            goto finish;
        }
    }

    if (toolArgs->terminateOption == kIOCatalogModuleTerminate) {
        kernResult = IOCatalogueTerminate(kIOMasterPortDefault,
            toolArgs->terminateOption, kextIdentifierCString);
    } else {
        kernResult = OSKextUnloadKextWithIdentifier(kextIdentifier,
            /* terminateAndRemovePersonalities */ true);
    }

    if (kernResult == kIOReturnNotPrivileged) {
         OSKextLog(/* kext */ NULL,
             kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
             "You must be running as root to unload kexts.");
         result = kKextunloadExitNotPrivileged;
         *fatal = true;
         goto finish;

    } else if (kernResult != KERN_SUCCESS) {
         result = kKextunloadExitPartialFailure;
        if (toolArgs->terminateOption == kIOCatalogModuleTerminate) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                "Terminate for %s failed - %s.",
                kextIdentifierCString, safe_mach_error_string(kernResult));
        } else {
            // OSKextUnloadKextWithIdentifier() logged an error
        }
    } else {
        if (toolArgs->terminateOption == kIOCatalogModuleTerminate) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogBasicLevel | kOSKextLogIPCFlag,
                "%s: services terminated and personalities removed "
                "(kext not unloaded).",
                kextIdentifierCString);
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogBasicLevel | kOSKextLogIPCFlag,
                "%s unloaded and personalities removed.",
                kextIdentifierCString);
        }
    }
finish:
    SAFE_FREE(kextIdentifierCString);

    return result;
}

/*******************************************************************************
*
*******************************************************************************/
void usage(UsageLevel usageLevel)
{
    fprintf(stderr, "usage: %s [-h] [-v [0-6]]\n"
        "        [-p] [-c class_name] ... [-b bundle_id] ... [kext] ...\n",
        progname);

    if (usageLevel == kUsageLevelBrief) {
        goto finish;
    }

    fprintf(stderr, "kext: unload the named kext and all personalities for it\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "-%s <bundle_id> (-%c):\n"
        "        unload the kext and personalities for CFBundleIdentifier <bundle_id>\n",
        kOptNameBundleIdentifier, kOptBundleIdentifier);
    fprintf(stderr, "-%s <class_name> (-%c):\n"
        "        terminate all instances of IOService class <class_name> but do not\n"
        "        unload its kext or remove its personalities\n",
        kOptNameClassName, kOptClassName);
    fprintf(stderr, "\n");
    fprintf(stderr,
        "-%s (-%c):\n"
        "        terminate services and remove personalities only; do not unload kexts\n"
        "        (applies only to unload by bundle-id or kext)\n",
            kOptNamePersonalitiesOnly, kOptPersonalitiesOnly);

// :doc: meaning of -p inverted all these years

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

finish:
    return;
}
