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
#ifndef _KEXTFIND_TABLES_H_
#define _KEXTFIND_TABLES_H_

#include <CoreFoundation/CoreFoundation.h>
#include <getopt.h>
#include "QEQuery.h"
#include "kext_tools_util.h"

/*******************************************************************************
* This data structure associated query keywords with the parsing and evaluation
* function callbacks used by the query engine. Some callbacks handle several
* keywords because of similar arguments or evaluation logic.
*
* See kextfind_query.[hc] for the definitions of these things.
*******************************************************************************/
struct querySetup {
    CFStringRef longName;
    CFStringRef shortName;
    QEQueryParseCallback parseCallback;
    QEQueryEvaluationCallback evalCallback;
};

/*******************************************************************************
* Command-line options (as opposed to query predicate keywords).
* This data is used by getopt_long_only().
*******************************************************************************/
/* Would like a way to automatically combine these into the optstring.
 */
#define kOptNameHelp                    "help"
#define kOptNameCaseInsensitive         "case-insensitive"
#define kOptNameNulTerminate            "nul"
#define kOptNameSearchItem              "search-item"
#define kOptNameSubstring               "substring"
#define kOptNameDefaultArch             "set-arch"

#ifdef EXTRA_INFO
// I think there will be better ways to do this after getting some airtime
#define kOptNameExtraInfo               "extra-info"
#endif

#define kOptNameRelativePaths           "relative-paths"
#define kOptNameNoPaths                 "no-paths"

// Currently unused, although code does reference them
// Things are picky by default for now.
#define kOptNameMeek                    "meek"
#define kOptNamePicky                   "picky"

#define kOPT_CHARS  "0ef:his"

enum {
    kOptSystemExtensions = 'e',
    kOptCaseInsensitive = 'i',
    kOptNulTerminate = '0',
    kOptSearchItem = 'f',
    kOptSubstring = 's',
};

/* Options with no single-letter variant.  */
// Do not use -1, that's getopt() end-of-args return value
// and can cause confusion
enum {
    kLongOptQueryPredicate = -2,
#ifdef EXTRA_INFO
    kLongOptExtraInfo = -3,
#endif
    kLongOptRelativePaths = -4,
    kLongOptNoPaths = -5,
    kLongOptMeek = -6,
    kLongOptPicky = -7,
    kLongOptReport = -8,
    kLongOptDefaultArch = -9,
};

/*******************************************************************************
* The structure of info needed by getopt_long_only().
*******************************************************************************/
extern struct option opt_info[];
extern int           longopt;

/*******************************************************************************
* A list of predicate names, synonyms, and parse/eval callbacks for the
* query engine.
*******************************************************************************/
extern struct querySetup queryCallbackList[];
extern struct querySetup reportCallbackList[];


/*******************************************************************************
*
*******************************************************************************/
#define QUERY_PREDICATES  \
    { &kPredNameProperty[1],               no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePropertyExists[1],         no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredNameMatchProperty[1],          no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameMatchPropertyExists[1],    no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredNameLoaded[1],                 no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameValid[1],                  no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameAuthentic[1],              no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameDependenciesMet[1],        no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameLoadable[1],               no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameWarnings[1],               no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameIsLibrary[1],              no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kQEQueryTokenAnd[1],                no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kQEQueryTokenOr[1],                 no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kQEQueryTokenNot[1],                no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredNameVersion[1],                no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameCompatibleWithVersion[1],  no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameIntegrity[1],              no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredNameHasPlugins[1],             no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameIsPlugin[1],               no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameHasDebugProperties[1],     no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredNameArch[1],                   no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameArchExact[1],              no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameExecutable[1],             no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameNoExecutable[1],           no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameDefinesSymbol[1],          no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameReferencesSymbol[1],       no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredNameDuplicate[1],              no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredNameInvalid[1],                no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameInauthentic[1],            no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameDependenciesMissing[1],    no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameNonloadable[1],            no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredNameBundleID[1],               no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameBundleName[1],             no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredNameRoot[1],                   no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameConsole[1],                no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameLocalRoot[1],              no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameNetworkRoot[1],            no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNameSafeBoot[1],               no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredCharProperty[1],               no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharPropertyExists[1],         no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredCharMatchProperty[1],          no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharMatchPropertyExists[1],    no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredCharValid[1],                  no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharAuthentic[1],              no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharDependenciesMet[1],        no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharLoadable[1],               no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharWarnings[1],               no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kQEQueryTokenAnd[1],                no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kQEQueryTokenOr[1],                 no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kQEQueryTokenNot[1],                no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredCharVersion[1],                no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredCharArchExact[1],              no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharExecutable[1],             no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharNoExecutable[1],           no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharDefinesSymbol[1],          no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharReferencesSymbol[1],       no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredCharDuplicate[1],              no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredCharInvalid[1],                no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharInauthentic[1],            no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharDependenciesMissing[1],    no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharNonloadable[1],            no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredCharBundleID[1],               no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredCharRoot[1],                   no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharConsole[1],                no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharLocalRoot[1],              no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharNetworkRoot[1],            no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharSafeBoot[1],               no_argument, &longopt, kLongOptQueryPredicate },  \

  #define QUERY_COMMANDS  \
    { &kPredNameEcho[1],                   no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePrint[1],                  no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePrintDiagnostics[1],       no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePrintProperty[1],          no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePrintMatchProperty[1],     no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePrintArches[1],            no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePrintDependencies[1],      no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePrintDependents[1],        no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePrintIntegrity[1],         no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePrintPlugins[1],           no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePrintInfoDictionary[1],    no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredNamePrintExecutable[1],        no_argument, &longopt, kLongOptQueryPredicate },  \
    \
    { &kPredNameExec[1],                   no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharPrintDiagnostics[1],       no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharPrintProperty[1],          no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharPrintMatchProperty[1],     no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharPrintArches[1],            no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharPrintInfoDictionary[1],    no_argument, &longopt, kLongOptQueryPredicate },  \
    { &kPredCharPrintExecutable[1],        no_argument, &longopt, kLongOptQueryPredicate },


#endif /* _KEXTFIND_TABLES_H_ */
