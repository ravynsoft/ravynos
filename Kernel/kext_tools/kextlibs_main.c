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

#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>

#include "kextlibs_main.h"
#include "kext_tools_util.h"


/*******************************************************************************
* Local function prototypes and misc grotty  bits.
*******************************************************************************/
const char * progname = "(unknown)";

/*******************************************************************************
*******************************************************************************/
int main(int argc, char * const * argv)
{
    ExitStatus          result              = EX_OSERR;
    ExitStatus          printResult         = EX_OSERR;

    KextlibsArgs        toolArgs;
    CFArrayRef          kexts               = NULL;  // must release

    const NXArchInfo ** arches              = NULL;  // must free
    KextlibsInfo      * libInfo             = NULL;  // must release contents & free
    Boolean             libsAreArchSpecific = FALSE;

    CFIndex             count, i;
    CFIndex             numArches;

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

    result = readArgs(argc, argv, &toolArgs);
    if (result != EX_OK) {
        if (result == kKextlibsExitHelp) {
            result = EX_OK;
        }
        goto finish;
    }

    result = EX_OSERR;

   /*****
    * If necessary or requested, add the extensions folders to the
    * BEGINNING of the list of folders.
    */
    count = CFArrayGetCount(toolArgs.repositoryURLs);
    if (!count || toolArgs.flagSysKexts) {
        CFArrayRef osExtFolders = OSKextGetSystemExtensionsFolderURLs(); // do not release

        if (!osExtFolders) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Library error - can't get system extensions folders.");
            goto finish;
        }

        count = CFArrayGetCount(osExtFolders);
        for (i = 0; i < count; i++) {
            CFTypeRef osExtFolder = CFArrayGetValueAtIndex(osExtFolders, i);
            CFIndex   folderIndex = CFArrayGetFirstIndexOfValue(
                toolArgs.repositoryURLs, RANGE_ALL(toolArgs.repositoryURLs),
                osExtFolder);
            if (folderIndex == kCFNotFound) {
                CFArrayInsertValueAtIndex(toolArgs.repositoryURLs, i,
                    osExtFolder);
            }
        }
    }

    kexts = OSKextCreateKextsFromURLs(kCFAllocatorDefault,
        toolArgs.repositoryURLs);
    if (!kexts) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't read kexts from folders.");
        goto finish;
    }

    toolArgs.kextURL = CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault, (u_char *)toolArgs.kextName,
        strlen(toolArgs.kextName), /* isDirectory */ true);
    if (!toolArgs.kextURL) {
        OSKextLogStringError(/* kext */ NULL);
        goto finish;
    }
    toolArgs.theKext = OSKextCreate(kCFAllocatorDefault,toolArgs. kextURL);
    if (!toolArgs.theKext) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't open %s.", toolArgs.kextName);
        goto finish;
    }

   /* A codeless kext is either a library redirect,
    * so we can't advise, or it doesn't need any libraries!
    */
    if (!OSKextDeclaresExecutable(toolArgs.theKext)) {
        if (OSKextIsLibrary(toolArgs.theKext)) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "%s is a library without an executable; "
                "defining its OSBundleLibraries (if any) is up to you.",
                toolArgs.kextName);
        } else {
            CFDictionaryRef libs = OSKextGetValueForInfoDictionaryKey(toolArgs.theKext,
                CFSTR(kOSBundleLibrariesKey));

            if (libs &&
                CFDictionaryGetTypeID() == CFGetTypeID(libs) &&
                CFDictionaryGetCount(libs)) {

                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "%s has no executable and should not declare OSBundleLibraries.",
                    toolArgs.kextName);
            } else {

               /* In this one case, the exit status will be EX_OK.
                */
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "%s has no executable and does not need to declare OSBundleLibraries.",
                    toolArgs.kextName);
                result = EX_OK;
            }
        }

        goto finish;
    }

    arches = OSKextCopyArchitectures(toolArgs.theKext);
    if (!arches || !arches[0]) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't determine architectures of %s.",
            toolArgs.kextName);
        goto finish;
    }

    result = EX_OK;

    for (numArches = 0; arches[numArches]; numArches++) {
        /* just counting */
    }

    libInfo = (KextlibsInfo *)malloc(numArches * sizeof(KextlibsInfo));
    if (!libInfo) {
        OSKextLogMemError();
        goto finish;
    }

   /* Find libraries for the kext for each architecture in the kext.
    */
    for (i = 0; i < numArches; i++) {
        OSKextSetArchitecture(arches[i]);

        libInfo[i].libKexts = OSKextFindLinkDependencies(toolArgs.theKext,
            toolArgs.flagNonKPI, toolArgs.flagAllowUnsupported,
            &libInfo[i].undefSymbols, &libInfo[i].onedefSymbols,
            &libInfo[i].multdefSymbols, &libInfo[i].multdefLibs);

        if (!libInfo[i].libKexts) {
            OSKextLogMemError();
            result = EX_OSERR;
            goto finish;
        }
    }

   /* If there's more than 1 arch, see if we have to print arch-specific
    * results.
    */
    if (numArches >= 2) {
        for (i = 0; i < numArches - 1; i++) {
            if (!CFEqual(libInfo[i].libKexts, libInfo[i+1].libKexts)) {
                libsAreArchSpecific = TRUE;
                break;
            }
        }
    }

   /* If all the libs are the same for all arches, then just print them
    * once at the top of the output. Otherwise, only when doing XML,
    * print the arch-specific XML declarations before the diagnostics.
    */
    if (!libsAreArchSpecific) {
        printResult = printLibs(&toolArgs, NULL, libInfo[0].libKexts,
            /* extraNewline? */ TRUE);

       /* Higher exit statuses always win.
        */
        if (printResult > result) {
            result = printResult;
        }
    } else if (toolArgs.flagXML) {

        for (i = 0; i < numArches; i++) {
            printResult = printLibs(&toolArgs, arches[i],
                libInfo[i].libKexts,
                /* extraNewline? */ i + 1 == numArches);
            if (printResult > result) {
                result = printResult;
            }
        }
    }

   /* Down here, for each arch, print arch-specific non-XML library declarations,
    * followed by any problems found for that arch.
    */
    for (i = 0; i < numArches; i++) {

        if (libsAreArchSpecific && !toolArgs.flagXML) {
            printResult = printLibs(&toolArgs, arches[i], libInfo[i].libKexts,
                /* extraNewline? */ i + 1 == numArches);
            if (printResult > result) {
                result = printResult;
            }
        }

        printResult = printProblems(&toolArgs, arches[i],
            libInfo[i].undefSymbols, libInfo[i].onedefSymbols,
            libInfo[i].multdefSymbols, libInfo[i].multdefLibs,
            /* printArchFlag */ !libsAreArchSpecific || toolArgs.flagXML,
            /* extraNewline? */ i + i < numArches);

        if (printResult != EX_OK) {
           /* Higher exit statuses always win.
            */
            if (printResult > result) {
                result = printResult;
            }
        }
    }


finish:

   /* Theoretically, we could exit w/o cleaning up.  However, 12569152
      points out that while clang should know exit() cleans up, its
      analyzer *should* treat CF objects differently: releasing them might
      have (important, expected) side effects.  So we just clean up
      everything and return from main, letting the runtime exit(result).
    */


    SAFE_FREE(arches);

    SAFE_RELEASE(toolArgs.repositoryURLs);
    SAFE_RELEASE(toolArgs.kextURL);
    SAFE_RELEASE(toolArgs.theKext);
    SAFE_RELEASE(kexts);        // this is the one clang unexpectedly noticed

    if (libInfo) {
        for (i = 0; i < numArches; i++) {
            SAFE_RELEASE_NULL(libInfo[i].libKexts);
            SAFE_RELEASE_NULL(libInfo[i].undefSymbols);
            SAFE_RELEASE_NULL(libInfo[i].onedefSymbols);
            SAFE_RELEASE_NULL(libInfo[i].multdefSymbols);
            SAFE_RELEASE_NULL(libInfo[i].multdefLibs);
        }
        free(libInfo);
    }
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus readArgs(
    int            argc,
    char * const * argv,
    KextlibsArgs * toolArgs)
{
    ExitStatus  result         = EX_USAGE;
    ExitStatus  scratchResult  = EX_USAGE;
    int         optChar        = 0;

    bzero(toolArgs, sizeof(*toolArgs));

   /*****
    * Allocate collection objects needed for command line argument processing.
    */
    toolArgs->repositoryURLs = CFArrayCreateMutable(kCFAllocatorDefault, 0,
        &kCFTypeArrayCallBacks);
    if (!toolArgs->repositoryURLs) {
        OSKextLogMemError();
        result = EX_OSERR;
        goto finish;
    }

   /*****
    * Process command-line arguments.
    */
    result = EX_USAGE;

    while ((optChar = getopt_long_only(argc, argv, kOptChars,
        sOptInfo, NULL)) != -1) {

        switch (optChar) {

            case kOptHelp:
                usage(kUsageLevelFull);
                result = kKextlibsExitHelp;
                goto finish;
                break;

            case kOptRepository:
                scratchResult = addRepository(toolArgs, optarg);
                if (scratchResult != EX_OK) {
                    result = scratchResult;
                    goto finish;
                }
                break;

            case kOptCompatible:
                toolArgs->flagCompatible = true;
                break;

            case kOptSystemExtensions:
                toolArgs->flagSysKexts = true;
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

            case 0:
                switch (longopt) {
                    case kLongOptXML:
                        toolArgs->flagXML = true;
                        break;

                    case kLongOptAllSymbols:
                        toolArgs->flagPrintUndefSymbols = true;
                        toolArgs->flagPrintOnedefSymbols = true;
                        toolArgs->flagPrintMultdefSymbols = true;
                        break;

                    case kLongOptUndefSymbols:
                        toolArgs->flagPrintUndefSymbols = true;
                        break;

                    case kLongOptOnedefSymbols:
                        toolArgs->flagPrintOnedefSymbols = true;
                        break;

                    case kLongOptMultdefSymbols:
                        toolArgs->flagPrintMultdefSymbols = true;
                        break;

                    case kLongOptNonKPI:
                        toolArgs->flagNonKPI = true;
                        break;

                    case kLongOptUnsupported:
                        toolArgs->flagAllowUnsupported = true;
                        break;

                }
                break;

            default:
                usage(kUsageLevelBrief);
                goto finish;
                break;
            }
    }

    argc -= optind;
    argv += optind;

    if (!argv[0]) {
        fprintf(stderr, "No kext specified.");
        usage(kUsageLevelBrief);
        goto finish;
    }

    scratchResult = checkPath(argv[0], kOSKextBundleExtension,
        /* directoryRequired */ TRUE, /* writableRequired */ FALSE);
    if (scratchResult != EX_OK) {
        result = scratchResult;
        goto finish;
    }
    toolArgs->kextName = argv[0];

    argc--;
    argv++;

    if (argc) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Too many arguments starting at '%s'.", argv[0]);
        usage(kUsageLevelBrief);
        goto finish;
    }

    result = EX_OK;
finish:
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus addRepository(
    KextlibsArgs * toolArgs,
    const char   * path)
{
    ExitStatus  result = EX_OSERR;
    CFURLRef    url    = NULL;   // must release

    result = checkPath(path, /* suffix */ NULL,
        /* dirRequired? */ TRUE, /* writableRequired? */ FALSE);
    if (result != EX_OK) {
        goto finish;
    }

    url = CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault, (const UInt8 *)path, strlen(path), true);
    if (!url) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't create CFURL for '%s'.", path);
        goto finish;
    }
    addToArrayIfAbsent(toolArgs->repositoryURLs, url);

    result = EX_OK;
finish:
    SAFE_RELEASE(url);
    return result;
}

/*******************************************************************************
* This function prints to stdout, as it represents the only real output of
* the program and we want to be able to pipe it into pbcopy.
*******************************************************************************/
ExitStatus printLibs(
    KextlibsArgs     * toolArgs,
    const NXArchInfo * arch,
    CFArrayRef         libKexts,
    Boolean            trailingNewlineFlag)
{
    ExitStatus         result        = EX_OSERR;
    const NXArchInfo * genericArch   = NULL;  // do not free
    char             * libIdentifier = NULL;  // must free
    CFIndex            count, i;

   /* Get the generic architecture name for the arch, except for PPC,
    * which we no longer support.
    */
    if (arch && arch->cputype != CPU_TYPE_POWERPC) {
        genericArch = NXGetArchInfoFromCpuType(arch->cputype,
            CPU_SUBTYPE_MULTIPLE);
        if (!genericArch) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Can't find generic NXArchInfo for %s.",
                arch->name);
            goto finish;
        }
    }

    count = CFArrayGetCount(libKexts);

   /* For XML output, don't print anything if we found no libraries;
    * an empty OSBundleLibraries might look like good output to somebody.
    */
    if (toolArgs->flagXML) {
        if (count) {
            fprintf(stdout, "\t<key>OSBundleLibraries%s%s</key>\n",
                genericArch ? "_" : "",
                genericArch ? genericArch->name : "");
            fprintf(stdout, "\t<dict>\n");
        }
    } else {
        if (arch) {
             fprintf(stderr, "For %s:\n", arch->name);
        } else {
             fprintf(stderr, "For all architectures:\n");
        }
        if (!count) {
            fprintf(stdout, "    No libraries found.\n");
        }
    }
    for (i = 0; i < count; i++) {
        OSKextRef      libKext = (OSKextRef)CFArrayGetValueAtIndex(libKexts, i);
        OSKextVersion  version;
        char           versCString[kOSKextVersionMaxLength];

        SAFE_FREE_NULL(libIdentifier);

        libIdentifier = createUTF8CStringForCFString(OSKextGetIdentifier(libKext));

        if (toolArgs->flagCompatible) {
            version = OSKextGetCompatibleVersion(libKext);
        } else {
            version = OSKextGetVersion(libKext);
        }

        if (libIdentifier && version > kOSKextVersionUndefined) {
            OSKextVersionGetString(version, versCString, sizeof(versCString));

            if (toolArgs->flagXML) {
                fprintf(stdout, "\t\t<key>%s</key>\n", libIdentifier);
                fprintf(stdout, "\t\t<string>%s</string>\n", versCString);
            } else {
                fprintf(stdout, "    %s = %s\n", libIdentifier, versCString);
            }
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Internal error generating library list.");
            goto finish;
        }
    }
    if (count && toolArgs->flagXML) {
        fprintf(stdout, "\t</dict>\n");
    }

    if (trailingNewlineFlag) {
        fprintf(stderr, "\n");
    }

    result = kKextlibsExitOK;
finish:
    SAFE_FREE(libIdentifier);
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus printProblems(
    KextlibsArgs     * toolArgs,
    const NXArchInfo * arch,
    CFDictionaryRef    undefSymbols,
    CFDictionaryRef    onedefSymbols,
    CFDictionaryRef    multdefSymbols,
    CFArrayRef         multdefLibs,
    Boolean            printArchFlag,
    Boolean            trailingNewlineFlag)
{
    ExitStatus         result              = EX_OSERR;
    CFIndex            undefCount, onedefCount, multdefCount;
    CFIndex            count, i;

    onedefCount  = CFDictionaryGetCount(onedefSymbols);
    undefCount   = CFDictionaryGetCount(undefSymbols);
    multdefCount = CFDictionaryGetCount(multdefSymbols);

    if (!toolArgs->flagPrintOnedefSymbols && !undefCount && !multdefCount) {
        result = kKextlibsExitOK;
        goto finish;
    }

    if (printArchFlag) {
        fprintf(stderr, "For %s:\n", arch->name);
    }

    if (toolArgs->flagPrintOnedefSymbols) {
        fprintf(stderr, "    %ld symbol%s found in one library kext each%s\n",
            onedefCount,
            onedefCount > 1 ? "s" : "",
            onedefCount && toolArgs->flagPrintOnedefSymbols ? ":" : ".");
        CFDictionaryApplyFunction(onedefSymbols, printOnedefSymbol,
            &(toolArgs->flagCompatible));
    }

    if (undefCount) {
        fprintf(stderr, "    %ld symbol%s not found in any library kext%s\n",
            undefCount,
            undefCount > 1 ? "s" : "",
            toolArgs->flagPrintUndefSymbols ? ":" : ".");
        if (toolArgs->flagPrintUndefSymbols) {
            CFDictionaryApplyFunction(undefSymbols, printUndefSymbol, NULL);
        }
        result = kKextlibsExitUndefineds;
    }
    if (multdefCount) {
        if (toolArgs->flagPrintMultdefSymbols) {
            fprintf(stderr, "    %ld symbol%s found in more than one library kext:\n",
                multdefCount,
                multdefCount > 1 ? "s" : "");
            CFDictionaryApplyFunction(multdefSymbols, printMultdefSymbol,
                &(toolArgs->flagCompatible));
        } else {
            count = CFArrayGetCount(multdefLibs);
            fprintf(stderr, "    Multiple symbols found among %ld libraries:\n",
                count);
            for (i = 0; i < count; i++) {
                OSKextRef lib = (OSKextRef)CFArrayGetValueAtIndex(multdefLibs, i);
                char * name = NULL; // must free
                name = createUTF8CStringForCFString(OSKextGetIdentifier(lib));
                if (name) {
                    fprintf(stderr, "\t%s\n", name);
                }
                SAFE_FREE(name);
            }
        }
        result = kKextlibsExitMultiples;
    }

    if (trailingNewlineFlag) {
        fprintf(stderr, "\n");
    }

    if (undefCount || multdefCount) {
        goto finish;
    }

    result = kKextlibsExitOK;

finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
void printUndefSymbol(const void * key,
                      const void * value __unused, void * context __unused)
{
    char * cSymbol = NULL; // must free

    cSymbol = createUTF8CStringForCFString((CFStringRef)key);
    if (cSymbol) {
        fprintf(stderr, "\t%s\n", cSymbol);
    }
    SAFE_FREE(cSymbol);
    return;
}

/*******************************************************************************
*
*******************************************************************************/
void printOnedefSymbol(const void * key, const void * value, void * context)
{
    Boolean       flagCompatible  = *(Boolean *)context;
    OSKextRef     libKext         = (OSKextRef)value;
    char        * cSymbol         = NULL; // must free
    char        * libVers         = NULL; // must free
    CFURLRef      libKextURL;
    char          libKextName[PATH_MAX];
    CFStringRef   libVersString = NULL; // do not release

    cSymbol = createUTF8CStringForCFString((CFStringRef)key);
    if (!cSymbol) {
        OSKextLogStringError(/* kext */ NULL);
        goto finish;
    }

    libKextURL = OSKextGetURL(libKext);
    if (!CFURLGetFileSystemRepresentation(libKextURL,
        /* resolveToBase */ false,
        (u_char *)libKextName, PATH_MAX)) {

        OSKextLogStringError(/* kext */ NULL);
        goto finish;
    }
    if (flagCompatible) {
        libVersString = OSKextGetValueForInfoDictionaryKey(libKext,
            CFSTR("OSBundleCompatibleVersion"));
    } else {
        libVersString = OSKextGetValueForInfoDictionaryKey(libKext,
            kCFBundleVersionKey);
    }

    libVers = createUTF8CStringForCFString(libVersString);
    if (!libVers) {
        OSKextLogStringError(/* kext */ NULL);
        goto finish;
    }

    fprintf(stderr, "    %s in %s (%s%s)\n", cSymbol, libKextName,
        flagCompatible ? "compatible version " : "", libVers);

finish:
    SAFE_FREE(cSymbol);
    SAFE_FREE(libVers);

    return;
}

/*******************************************************************************
*
*******************************************************************************/
void printMultdefSymbol(const void * key, const void * value, void * context)
{
    char        * cSymbol         = NULL; // must free
    char        * libVers         = NULL; // must free
    CFArrayRef   libs = (CFArrayRef)value;
    Boolean      flagCompatible   = *(Boolean *)context;
    CFIndex      count, i;

    cSymbol = createUTF8CStringForCFString((CFStringRef)key);
    if (cSymbol) {
        fprintf(stderr, "    %s: in\n", cSymbol);
    }

    count = CFArrayGetCount(libs);
    for (i = 0; i < count; i++) {
        OSKextRef     libKext = (OSKextRef)CFArrayGetValueAtIndex(libs, i);
        CFURLRef      libKextURL;
        char          libKextName[PATH_MAX];
        CFStringRef   libVersString = NULL; // do not release

        SAFE_FREE_NULL(libVers);

        libKextURL = OSKextGetURL(libKext);
        if (!CFURLGetFileSystemRepresentation(libKextURL,
            /* resolveToBase */ true,
            (u_char *)libKextName, PATH_MAX)) {

            fprintf(stderr, "string/url conversion error\n");
            goto finish;
        }
        if (flagCompatible) {
            libVersString = OSKextGetValueForInfoDictionaryKey(libKext,
                CFSTR("OSBundleCompatibleVersion"));
        } else {
            libVersString = OSKextGetValueForInfoDictionaryKey(libKext,
                kCFBundleVersionKey);
        }

        libVers = createUTF8CStringForCFString(libVersString);
        if (!libVers) {
            fprintf(stderr, "string/url conversion error\n");
            goto finish;
        }
        fprintf(stderr, "        %s (%s%s)\n", libKextName,
            flagCompatible ? "compatible version " : "", libVers);
    }

finish:
    SAFE_FREE(cSymbol);
    SAFE_FREE(libVers);

    return;
}

/*******************************************************************************
* usage()
*******************************************************************************/
static void usage(UsageLevel usageLevel)
{
    fprintf(stderr, "usage: %s [options] kext\n", progname);

    if (usageLevel == kUsageLevelBrief) {
        fprintf(stderr, "\nuse %s -%s for a list of options\n",
            progname, kOptNameHelp);
        return;
    }

    fprintf(stderr, "\n");

    // extra newline for spacing
    fprintf(stderr, "<kext>: the kext to find libraries for\n");

    fprintf(stderr, "-%s <arch>:\n", kOptNameArch);
    fprintf(stderr, "        resolve for architecture <arch> instead of running kernel's\n");
    fprintf(stderr, "-%s:   print XML fragment suitable for pasting\n", kOptNameXML);

     // fake out compiler for blank line
     fprintf(stderr, "\n");

    fprintf(stderr, "-%s (-%c):\n",
        kOptNameSystemExtensions, kOptSystemExtensions);
    fprintf(stderr,  "        look in the system exensions folder (assumed if no other folders\n"
        "        specified with %s)\n",kOptNameRepository);
    fprintf(stderr, "-%s <directory> (-%c):\n", kOptNameRepository, kOptRepository);
    fprintf(stderr, "        look in <directory> for library kexts\n");

    fprintf(stderr, "%s", "\n");

    fprintf(stderr, "-%s:\n", kOptNameAllSymbols);
    fprintf(stderr, "        list all symbols, found, not found, or found more than once\n");
    fprintf(stderr, "-%s:\n", kOptNameOnedefSymbols);
    fprintf(stderr, "        list all symbols found with the library kext they were found in\n");
    fprintf(stderr, "-%s:\n", kOptNameUndefSymbols);
    fprintf(stderr, "        list all symbols not found in any library\n");
    fprintf(stderr, "-%s:\n", kOptNameMultdefSymbols);
    fprintf(stderr, "        list all symbols found more than once with their library kexts\n");

    fprintf(stderr, "%s", "\n");

    fprintf(stderr, "-%s (-%c):\n", kOptNameCompatible, kOptCompatible);
    fprintf(stderr, "        use library kext compatble versions rather than current versions\n");
    fprintf(stderr, "-%s:\n", kOptNameUnsupported);
    fprintf(stderr, "        look in unsupported kexts for symbols\n");
    return;
}
