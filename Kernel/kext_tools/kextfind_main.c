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
#include <CoreFoundation/CFBundlePriv.h>

#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>

#include "kextfind_main.h"
#include "kextfind_tables.h"
#include "kextfind_query.h"
#include "kextfind_commands.h"
#include "kextfind_report.h"
#include "QEQuery.h"

/*******************************************************************************
* Misc. macros.
*******************************************************************************/

#define kKextSuffix            ".kext"

/*******************************************************************************
* Global variables (non-static referenced by utility.c).
*******************************************************************************/

const char * progname = "(unknown)";

#pragma mark Main Routine
/*******************************************************************************
* Global variables.
*******************************************************************************/
int main(int argc, char * const *argv)
{
    int result = EX_OSERR;

    CFIndex count, i;

    QEQueryRef          query            = NULL;
    struct querySetup * queryCallback    = queryCallbackList;

    QEQueryRef          reportQuery      = NULL;
    struct querySetup * reportCallback   = reportCallbackList;
    uint32_t            reportStartIndex;

    QueryContext        queryContext;
    Boolean             queryStarted     = false;
    uint32_t            numArgsUsed      = 0;

    OSKextRef           theKext          = NULL;  // don't release
    CFArrayRef          allKexts         = NULL;  // must release

    bzero(&queryContext, sizeof(queryContext));

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

    result = readArgs(argc, argv, &queryContext);
    if (result != EX_OK) {
        if (result == kKextfindExitHelp) {
            result = EX_OK;
        }
        goto finish;
    }

    result = checkArgs(&queryContext);
    if (result != EX_OK) {
        goto finish;
    }

    if (queryContext.defaultArch) {
        OSKextSetArchitecture(queryContext.defaultArch);
    }

   /*****
    * Set up the query.
    */
    query = QEQueryCreate(&queryContext);
    if (!query) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't create query");
        goto finish;
    }

    while (queryCallback->longName) {
        if (queryCallback->parseCallback) {
            QEQuerySetParseCallbackForPredicate(query, queryCallback->longName,
                queryCallback->parseCallback);
            if (queryCallback->shortName) {
                QEQuerySetSynonymForPredicate(query, queryCallback->shortName,
                    queryCallback->longName);
            }
        }
        if (queryCallback->evalCallback) {
            QEQuerySetEvaluationCallbackForPredicate(query,
                queryCallback->longName,
                queryCallback->evalCallback);
        }
        queryCallback++;
    }
    QEQuerySetSynonymForPredicate(query, CFSTR("!"), CFSTR(kQEQueryTokenNot));

    numArgsUsed = optind;

   /* If we're not immediately doing a report spec, parse the query.
    */
    if (argv[numArgsUsed] && strcmp(argv[numArgsUsed], kKeywordReport)) {
        while (QEQueryAppendElementFromArgs(query, argc - numArgsUsed,
            &argv[numArgsUsed], &numArgsUsed)) {

            queryStarted = true;
            if (argv[numArgsUsed] && !strcmp(argv[numArgsUsed], kKeywordReport)) {
                break;
            }
        }

    }

    if (QEQueryLastError(query) != kQEQueryErrorNone) {
        switch (QEQueryLastError(query)) {
          case kQEQueryErrorNoMemory:
            OSKextLogMemError();
            break;
          case kQEQueryErrorEmptyGroup:
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Empty group near arg #%d.", numArgsUsed);
            break;
          case kQEQueryErrorSyntax:
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Query syntax error near '%s' (arg #%d).",
                argv[numArgsUsed], numArgsUsed);
            break;
          case kQEQueryErrorNoParseCallback:
            if (queryStarted) {
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Expected query predicate, found '%s' (arg #%d).",
                    argv[numArgsUsed], numArgsUsed);
            } else {
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Unknown option/query predicate '%s' (arg #%d).",
                    argv[numArgsUsed], numArgsUsed);
                usage(kUsageLevelBrief);
            }
            break;
          case kQEQueryErrorInvalidOrMissingArgument:
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Invalid/missing option or argument for '%s' (arg #%d).",
                argv[numArgsUsed], numArgsUsed);
            break;
          case kQEQueryErrorParseCallbackFailed:
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Query parsing callback failed.");
            break;
          default:
            break;
        }
        goto finish;
    }

    if (!QEQueryIsComplete(query)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Unbalanced groups or trailing operator.");
        goto finish;
    }

   /****************************************
    */
    if (argv[numArgsUsed] && !strcmp(argv[numArgsUsed], kKeywordReport)) {

        numArgsUsed++;

        if (argv[numArgsUsed] && !strcmp(argv[numArgsUsed], kNoReportHeader)) {
            numArgsUsed++;
            queryContext.reportStarted = true; // cause header to be skipped
        }

        if (queryContext.commandSpecified) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Can't do report; query has commands.");
            goto finish;
        }

        reportQuery = QEQueryCreate(&queryContext);
        if (!reportQuery) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Can't create report engine.");
            goto finish;
        }
        QEQuerySetShortCircuits(reportQuery, false);

        while (reportCallback->longName) {
            if (reportCallback->parseCallback) {
                QEQuerySetParseCallbackForPredicate(reportQuery,
                    reportCallback->longName,
                    reportCallback->parseCallback);
                if (reportCallback->shortName) {
                    QEQuerySetSynonymForPredicate(reportQuery,
                        reportCallback->shortName,
                        reportCallback->longName);
                }
            }
            if (reportCallback->evalCallback) {
                QEQuerySetEvaluationCallbackForPredicate(reportQuery,
                    reportCallback->longName,
                    reportCallback->evalCallback);
            }
            reportCallback++;
        }

        reportStartIndex = numArgsUsed;

        while (QEQueryAppendElementFromArgs(reportQuery, argc - numArgsUsed,
            &argv[numArgsUsed], &numArgsUsed)) {
        }

        if (reportStartIndex == numArgsUsed) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "No report predicates specified.");
            usage(kUsageLevelBrief);
            goto finish;
        }

        if (QEQueryLastError(reportQuery) != kQEQueryErrorNone) {
            switch (QEQueryLastError(reportQuery)) {
              case kQEQueryErrorNoMemory:
                OSKextLogMemError();
                break;
              case kQEQueryErrorEmptyGroup:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Empty group near arg #%d.", numArgsUsed);
                break;
              case kQEQueryErrorSyntax:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Report syntax error near '%s' (arg #%d).",
                    argv[numArgsUsed], numArgsUsed);
                break;
              case kQEQueryErrorNoParseCallback:
                if (1) {
                    OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Expected report predicate, found '%s' (arg #%d).",
                        argv[numArgsUsed], numArgsUsed);
                } else {
                    OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Unknown option/report predicate '%s' (arg #%d).",
                        argv[numArgsUsed], numArgsUsed);
                }
                break;
              case kQEQueryErrorInvalidOrMissingArgument:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Invalid/missing option or argument for '%s' (arg #%d).",
                    argv[numArgsUsed], numArgsUsed);
                break;
              case kQEQueryErrorParseCallbackFailed:
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Query parsing callback failed.");
                break;
              default:
                break;
            }
            goto finish;
        }

        if (!QEQueryIsComplete(reportQuery)) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Unbalanced groups or trailing operator.");
            goto finish;
        }
    }

    if ((int)numArgsUsed < argc) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Leftover elements '%s'... (arg #%d).",
            argv[numArgsUsed], numArgsUsed);
        goto finish;
    }

   /*****
    * Create the set of kexts we'll be searching/reporting.
    */
    OSKextSetRecordsDiagnostics(kOSKextDiagnosticsFlagAll);
    OSKextSetUsesCaches(false);
    allKexts = OSKextCreateKextsFromURLs(kCFAllocatorDefault,
        queryContext.searchURLs);
    if (!allKexts || !CFArrayGetCount(allKexts)) {
        // see comment below about clang not understanding exit() :P
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "No kernel extensions found.");
        result = EX_SOFTWARE;
        goto finish;
    }

    if (queryContext.checkLoaded) {
        if (kOSReturnSuccess != OSKextReadLoadedKextInfo(
            /* kextIdentifiers (all kexts) */ NULL,
            /* flushDependencies? */ false)) {

            result = EX_OSERR;
            goto finish;
        }
    }

    if (result != EX_OK) {
        goto finish;
    }

   /*****
    * Run the query!
    */
    count = CFArrayGetCount(allKexts);
    for (i = 0; i < count; i++) {

        theKext = (OSKextRef)CFArrayGetValueAtIndex(allKexts, i);

        if (QEQueryEvaluate(query, theKext)) {
            if (!queryContext.commandSpecified) {
                if (!reportQuery) {
                    printKext(theKext, queryContext.pathSpec,
                        queryContext.extraInfo, '\n');
                } else {
                    if (!queryContext.reportStarted) {
                        queryContext.reportRowStarted = false;
                        QEQueryEvaluate(reportQuery, theKext);
                        printf("\n");
                        if ((QEQueryLastError(reportQuery) != kQEQueryErrorNone)) {
                            OSKextLog(/* kext */ NULL,
                                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                                "Report evaluation error; aborting.");
                            goto finish;
                        }
                        queryContext.reportStarted = true;
                    }
                    queryContext.reportRowStarted = false;
                    QEQueryEvaluate(reportQuery, theKext);
                    printf("\n");
                    if ((QEQueryLastError(reportQuery) != kQEQueryErrorNone)) {
                        OSKextLog(/* kext */ NULL,
                            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                            "Report evaluation error; aborting.");
                        goto finish;
                    }
                }
            }
        } else if (QEQueryLastError(query) != kQEQueryErrorNone) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                "Query evaluation error; aborting.");
            goto finish;
        }
    }

    result = EX_OK;

finish:
    // clang's analyzer now knows exit() never returns but doesn't realize
    // it frees resources. :P
    exit(result);  // we don't need to do the cleanup when exiting.

    if (query)                 QEQueryFree(query);
    if (allKexts)              CFRelease(allKexts);

    exit(result);
    return result;
}

#pragma mark Major Subroutines
/*******************************************************************************
* Major Subroutines
*******************************************************************************/
ExitStatus readArgs(
    int            argc,
    char * const * argv,
    QueryContext * toolArgs)
{
    ExitStatus result          = EX_USAGE;
    int        opt_char        = 0;
    int        last_optind;  // for recovering from getopt failures
    Boolean    readingOptions  = true;
    CFURLRef   scratchURL      = NULL;  // must release
    uint32_t   i;

    bzero(toolArgs, sizeof(*toolArgs));

   /*****
    * Allocate collection objects needed for command line argument processing.
    */
    if (!createCFMutableArray(&toolArgs->searchURLs, &kCFTypeArrayCallBacks)) {
        OSKextLogMemError();
        result = EX_OSERR;
        goto finish;
    }

    toolArgs->assertiveness = kKextfindPicky;

   /*****
    * Process command-line arguments.
    */
    opterr = 0;
    last_optind = optind;
    while (readingOptions &&
        -1 != (opt_char = getopt_long_only(argc, argv, kOPT_CHARS,
        opt_info, NULL))) {

        switch (opt_char) {

            case kOptHelp:
                usage(kUsageLevelFull);
                result = kKextfindExitHelp;
                goto finish;
                break;

            case kOptCaseInsensitive:
                toolArgs->caseInsensitive = true;
                break;

            case kOptSearchItem:
                if (!checkSearchItem(optarg, /* log? */ true)) {
                    goto finish;
                }

                SAFE_RELEASE_NULL(scratchURL);
                scratchURL = CFURLCreateFromFileSystemRepresentation(
                    kCFAllocatorDefault,
                    (const UInt8 *)optarg, strlen(optarg), true);
                if (!scratchURL) {
                    result = EX_OSERR;
                    OSKextLogMemError();
                    goto finish;
                }
                CFArrayAppendValue(toolArgs->searchURLs, scratchURL);
                break;

            case kOptSubstring:
                toolArgs->substrings = true;
                break;

            case kOptSystemExtensions:
              {
                    CFArrayRef sysExtFolders =
                        OSKextGetSystemExtensionsFolderURLs();
                    CFArrayAppendArray(toolArgs->searchURLs,
                        sysExtFolders, RANGE_ALL(sysExtFolders));
                }
                break;

            case 0:
                switch (longopt) {

                    case kLongOptQueryPredicate:
                        optind = last_optind;
                        readingOptions = false;
                        if (argv[last_optind] && (argv[last_optind][0] != '-')) {
                            // probably a directory; stupid getopt_long_only()
                            break;
                        }
                        break;

#ifdef EXTRA_INFO
                    case kLongOptExtraInfo:
                        toolArgs->extraInfo = true;
                        break;
#endif
                    case kLongOptRelativePaths:
                       /* Last one specified wins! */
                        toolArgs->pathSpec = kPathsRelative;
                        break;

                    case kLongOptDefaultArch:
                       /* Last one specified wins! */
                        toolArgs->defaultArch = NXGetArchInfoFromName(optarg);
                        if (!toolArgs->defaultArch) {
                            OSKextLog(/* kext */ NULL,
                                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                                "Unknown architecture %s.", optarg);
                            goto finish;
                        }
                        break;

                    case kLongOptNoPaths:
                       /* Last one specified wins! */
                        toolArgs->pathSpec = kPathsNone;
                        break;

#ifdef MEEK_PICKY
                    case kLongOptMeek:
                        toolArgs->assertiveness = kKextfindMeek;
                        break;

                    case kLongOptPicky:
                        toolArgs->assertiveness = kKextfindPicky;
                        break;
#endif

                    default:
                        OSKextLog(/* kext */ NULL,
                            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                            "Internal argument processing error.");
                        result = EX_SOFTWARE;
                        goto finish;
                        break;

                }
                longopt = 0;
                break;

            default:
               /* getopt_long() gives us '?' if we turn off errors, so just
                * move on to query parsing. Sometimes optind jumps ahead too
                * far, so we restore it to the value before the call to
                * getopt_long_only() -- we can't just decrement it.
                */
                optind = last_optind;
                readingOptions = false;
                break;
        }

        last_optind = optind;
    }

   /*****
    * Record the kext & directory names from the command line.
    */
    for (i = optind; (int)i < argc; i++) {
        SAFE_RELEASE_NULL(scratchURL);

       /* If the arg isn't a directory, break from the loop, and we'll
        * process remaining args as query elements.
        */
        if (!checkSearchItem(argv[i], /* log? */ false)) {
            break;
        }
        scratchURL = CFURLCreateFromFileSystemRepresentation(
            kCFAllocatorDefault,
            (const UInt8 *)argv[i], strlen(argv[i]), true);
        if (!scratchURL) {
            result = EX_OSERR;
            OSKextLogMemError();
            goto finish;
        }
        CFArrayAppendValue(toolArgs->searchURLs, scratchURL);
        optind++;
    }

    if (!CFArrayGetCount(toolArgs->searchURLs)) {
        CFArrayRef sysExtFolders =
            OSKextGetSystemExtensionsFolderURLs();
        CFArrayAppendArray(toolArgs->searchURLs,
            sysExtFolders, RANGE_ALL(sysExtFolders));
    }

    result = EX_OK;

finish:
    SAFE_RELEASE(scratchURL);

    if (result == EX_USAGE) {
        usage(kUsageLevelBrief);
    }
    return result;
}

/*******************************************************************************
*******************************************************************************/
ExitStatus checkArgs(QueryContext * toolArgs __unused)
{
    ExitStatus result = EX_USAGE;

    result = EX_OK;

    if (result == EX_USAGE) {
        usage(kUsageLevelBrief);
    }
    return result;
}

/*******************************************************************************
* checkSearchItem()
*
* This function makes sure that a given directory exists, and is writeable.
*******************************************************************************/
Boolean checkSearchItem(const char * pathname, Boolean logFlag)
{
    int result = false;
    struct stat stat_buf;

    if (stat(pathname, &stat_buf) != 0) {
        if (logFlag) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                "Can't stat %s - %s.", pathname, strerror(errno));
        }
        goto finish;
    }

    if ((stat_buf.st_mode & S_IFMT) != S_IFDIR) {
        if (logFlag) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                "%s - not a kext or directory.",
                pathname);
        }
        goto finish;
    }

    result = true;

finish:
    return result;
}

/*******************************************************************************
*******************************************************************************/
fat_iterator createFatIteratorForKext(OSKextRef aKext)
{
    fat_iterator result        = NULL;
    CFURLRef     kextURL       = NULL;  // do not release
    CFURLRef     executableURL = NULL;  // must release
    char         executablePath[PATH_MAX];

    kextURL = OSKextGetURL(aKext);
    if (!kextURL) {
        // xxx - log it?
        goto finish;
    }
    executableURL = _CFBundleCopyExecutableURLInDirectory(kextURL);
    if (!executableURL) {
        goto finish;
    }
    if (!CFURLGetFileSystemRepresentation(executableURL,
        /* resolveToBase? */ true, (UInt8 *)executablePath,
        sizeof(executablePath))) {

        OSKextLogStringError(aKext);
        goto finish;
    }
    result = fat_iterator_open(executablePath, /* macho_only? */ true);

finish:
    SAFE_RELEASE(executableURL);
    return result;
}

/*******************************************************************************
* usage()
*******************************************************************************/
void usage(UsageLevel usageLevel)
{
    FILE * stream = stderr;

    fprintf(stream,
      "usage: %s [options] [directory or extension ...] [query]\n"
      "    [-report [-no-header] report_predicate...]"
      "\n",
      progname);

    if (usageLevel == kUsageLevelBrief) {
        fprintf(stream, "use %s -%s for a list of options\n",
            progname, kOptNameHelp);
        return;
    }

    fprintf(stream, "Options\n");

    fprintf(stream, "    -%s                        -%s\n",
        kOptNameHelp, kOptNameCaseInsensitive);
#ifdef EXTRA_INFO
    fprintf(stream, "    -%s                  -%s\n",
        kOptNameExtraInfo, kOptNameNulTerminate);
#endif
    fprintf(stream, "    -%s              -%s\n",
        kOptNameRelativePaths, kOptNameSubstring);
    fprintf(stream, "    -%s\n",
        kOptNameNoPaths);

    fprintf(stream, "\n");

    fprintf(stream, "Handy Query Predicates\n");

    fprintf(stream, "    %s [-s] [-i] id\n", kPredNameBundleID);
    fprintf(stream, "    %s [-s] [-i] id\n", kPredNameBundleName);
    fprintf(stream, "    %s [-s] [-i] name value\n", kPredNameMatchProperty);
    fprintf(stream, "    %s [-s] [-i] name value\n", kPredNameProperty);

    fprintf(stream, "\n");

    fprintf(stream, "    %s                      %s\n",
        kPredNameLoaded, kPredNameNonloadable);
    fprintf(stream, "    %s                     %s\n",
        kPredNameInvalid, kPredNameInauthentic);
    fprintf(stream, "    %s        %s\n",
        kPredNameDependenciesMissing, kPredNameWarnings);

    fprintf(stream, "\n");

    fprintf(stream, "    %s arch1[,arch2...]       %s arch1[,arch2...]\n",
        kPredNameArch, kPredNameArchExact);
    fprintf(stream, "    %s                  %s\n",
        kPredNameExecutable, kPredNameIsLibrary);
    fprintf(stream, "    %s symbol       %s symbol\n",
        kPredNameDefinesSymbol, kPredNameReferencesSymbol);

    fprintf(stream, "\n");

    fprintf(stream, "See the man page for the full list.\n");

    return;
}
