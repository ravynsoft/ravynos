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
#include "kextfind_tables.h"
#include "kextfind_query.h"
#include "kextfind_report.h"

int longopt = 0;

struct option opt_info[] = {
    // real options
    { kOptNameHelp,             no_argument,        NULL,     kOptHelp },
    { kOptNameCaseInsensitive,  no_argument,        NULL,     kOptCaseInsensitive },
    { kOptNameSearchItem,       required_argument,  NULL,     kOptSearchItem },
    { kOptNameSystemExtensions, no_argument,        NULL,     kOptSystemExtensions },
    { kOptNameDefaultArch,      required_argument,  &longopt, kLongOptDefaultArch },
    { kOptNameSubstring,        no_argument,        NULL,     kOptSubstring },
#ifdef EXTRA_INFO
    { kOptNameExtraInfo,        no_argument,        &longopt, kLongOptExtraInfo },
#endif
    { kOptNameRelativePaths,    no_argument,        &longopt, kLongOptRelativePaths },
    { kOptNameNoPaths,          no_argument,        &longopt, kLongOptNoPaths },
#ifdef MEEK_PICKY
    { kOptNameMeek,             no_argument,        &longopt, kLongOptMeek },
    { kOptNamePicky,            no_argument,        &longopt, kLongOptPicky },
#endif /* MEEK_PICKY */

   /* We register the query predicates so that they are handled correctly
    * even with the few short options we have (without these, -invalid gets
    * recognized as -i plus more options, and -false as '-f alse').
    *
    * Query "options" are handled by simply terminating the getopt loop.
    */
    QUERY_PREDICATES

    QUERY_COMMANDS

    { &kKeywordReport[1],       no_argument,        &longopt, kLongOptQueryPredicate },

    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};

/* Each long ("name") predicate is used to register at least a parse callback,
 * but not always an eval callback; some sets of predicates get funneled
 * together *after* parsing, so we register them here with their own triggers
 * but at parse time their predicates get reset to the shared one so that at
 * eval time they use the same function.
 *
 * The -property, and -property-exists keywords, for example, both parse into
 * -property predicates, but the other two set some data in the query element
 * that the single evalProperty() function looks for and uses to tweak its
 * behavior.
 */
struct querySetup queryCallbackList[] = {
    {   CFSTR(kPredNameProperty), CFSTR(kPredCharProperty),
        parseProperty, evalProperty },
    {   CFSTR(kPredNamePropertyExists), CFSTR(kPredCharPropertyExists),
        parseProperty, NULL },

    {   CFSTR(kPredNameMatchProperty), CFSTR(kPredCharMatchProperty),
        parseMatchProperty, evalMatchProperty },
    {   CFSTR(kPredNameMatchPropertyExists), CFSTR(kPredCharMatchPropertyExists),
        parseMatchProperty, NULL },

    {   CFSTR(kPredNameLoaded), NULL,
        parseFlag, NULL },
    {   CFSTR(kPredNameValid), CFSTR(kPredCharValid),
        parseFlag, NULL },
    {   CFSTR(kPredNameAuthentic), CFSTR(kPredCharAuthentic),
        parseFlag, NULL },
    {   CFSTR(kPredNameDependenciesMet), CFSTR(kPredCharDependenciesMet),
        parseFlag, NULL },
    {   CFSTR(kPredNameLoadable), CFSTR(kPredCharLoadable),
        parseFlag, NULL },
    {   CFSTR(kPredNameWarnings), CFSTR(kPredCharWarnings),
        parseFlag, NULL },
    {   CFSTR(kPredNameIsLibrary), CFSTR(kPredCharIsLibrary),
        parseFlag, NULL },

    {   CFSTR(kPredNameDuplicate), CFSTR(kPredCharDuplicate),
        parseFlag, NULL },

    {   CFSTR(kPredNameInvalid), CFSTR(kPredCharInvalid),
        parseFlag, NULL },
    {   CFSTR(kPredNameInauthentic), CFSTR(kPredCharInauthentic),
        parseFlag, NULL },
    {   CFSTR(kPredNameDependenciesMissing), CFSTR(kPredCharDependenciesMissing),
        parseFlag, NULL },
    {   CFSTR(kPredNameNonloadable), CFSTR(kPredCharNonloadable),
        parseFlag, NULL },

    {   CFSTR(kPredNameHasPlugins), NULL,
        parseFlag, NULL },
    {   CFSTR(kPredNameIsPlugin), NULL,
        parseFlag, NULL },
    {   CFSTR(kPredNameHasDebugProperties), NULL,
        parseFlag, NULL },
    {   CFSTR(kPredNameIsKernelResource), NULL,
        parseFlag, NULL },

    {   CFSTR(kPredNameVersion), CFSTR(kPredCharVersion),
        parseVersion, evalVersion },
    {   CFSTR(kPredNameCompatibleWithVersion), NULL,
        parseCompatibleWithVersion, evalCompatibleWithVersion },
    {   CFSTR(kPredNameIntegrity), NULL,
        parseIntegrity, evalIntegrity },

    {   CFSTR(kPredNameArch), NULL,
        parseArch, evalArch },
    {   CFSTR(kPredNameArchExact), CFSTR(kPredCharArchExact),
        parseArch, evalArchExact },
    {   CFSTR(kPredNameExecutable), CFSTR(kPredCharExecutable),
        parseFlag, NULL },
    {   CFSTR(kPredNameNoExecutable), CFSTR(kPredCharNoExecutable),
        parseFlag, NULL },
    {   CFSTR(kPredNameDefinesSymbol), CFSTR(kPredCharDefinesSymbol),
        parseDefinesOrReferencesSymbol, evalDefinesOrReferencesSymbol },
    {   CFSTR(kPredNameReferencesSymbol), CFSTR(kPredCharReferencesSymbol),
        parseDefinesOrReferencesSymbol, evalDefinesOrReferencesSymbol },

    {   CFSTR(kPredNameBundleID), CFSTR(kPredCharBundleID),
        parseShorthand, NULL },
    {   CFSTR(kPredNameBundleName), CFSTR(kPredCharBundleName),
        parseBundleName, evalBundleName },

    {   CFSTR(kPredNameRoot), CFSTR(kPredCharRoot),
        parseShorthand, NULL },
    {   CFSTR(kPredNameConsole), CFSTR(kPredCharConsole),
        parseShorthand, NULL },
    {   CFSTR(kPredNameLocalRoot), CFSTR(kPredCharLocalRoot),
        parseShorthand, NULL },
    {   CFSTR(kPredNameNetworkRoot), CFSTR(kPredCharNetworkRoot),
        parseShorthand, NULL },
    {   CFSTR(kPredNameSafeBoot), CFSTR(kPredCharSafeBoot),
        parseShorthand, NULL },

    {   CFSTR(kPredNameEcho), NULL,
        parseCommand, NULL },
    {   CFSTR(kPredNamePrint), NULL,
        parseCommand, NULL },
    {   CFSTR(kPredNamePrint0), NULL,
        parseCommand, NULL },
    {   CFSTR(kPredNamePrintDiagnostics), CFSTR(kPredCharPrintDiagnostics),
        parseCommand, NULL },
    {   CFSTR(kPredNamePrintProperty), CFSTR(kPredCharPrintProperty),
        parseCommand, NULL },
    {   CFSTR(kPredNamePrintArches), CFSTR(kPredCharPrintArches),
        parseCommand, NULL },
    {   CFSTR(kPredNamePrintDependencies), NULL,
        parseCommand, NULL },
    {   CFSTR(kPredNamePrintDependents), NULL,
        parseCommand, NULL },
    {   CFSTR(kPredNamePrintIntegrity), NULL,
        parseCommand, NULL },
    {   CFSTR(kPredNamePrintPlugins), NULL,
        parseCommand, NULL },
    {   CFSTR(kPredNamePrintInfoDictionary), CFSTR(kPredCharPrintInfoDictionary),
        parseCommand, NULL },
    {   CFSTR(kPredNamePrintExecutable), CFSTR(kPredCharPrintExecutable),
        parseCommand, NULL },

    {   CFSTR(kPredNameExec), NULL, parseExec, evalExec },

   /* These two special predicates are used internally for all "flag" and
    * command precidates, which all reset their predicates at parse time and
    * save the original keyword.
    */
    {   CFSTR(kPredNameFlag), NULL,
        NULL, evalFlag },
    {   CFSTR(kPredNameCommand), NULL,
        NULL, evalCommand },

    { NULL, NULL, NULL, NULL }  // sentinel to terminate list

};

/* The report callback list reuses many query and command predicate keywords,
 * but parses and evaluates a lot of them differently!
 */
struct querySetup reportCallbackList[] = {
    {   CFSTR(kPredNameProperty), CFSTR(kPredCharProperty),
        reportParseProperty, reportEvalProperty },

    {   CFSTR(kPredNameLoaded), NULL,
        reportParseFlag, NULL },
    {   CFSTR(kPredNameValid), CFSTR(kPredCharValid),
        reportParseFlag, NULL },
    {   CFSTR(kPredNameAuthentic), CFSTR(kPredCharAuthentic),
        reportParseFlag, NULL },
    {   CFSTR(kPredNameDependenciesMet), CFSTR(kPredCharDependenciesMet),
        reportParseFlag, NULL },
    {   CFSTR(kPredNameLoadable), CFSTR(kPredCharLoadable),
        reportParseFlag, NULL },
    {   CFSTR(kPredNameWarnings), CFSTR(kPredCharWarnings),
        reportParseFlag, NULL },
    {   CFSTR(kPredNameIsLibrary), CFSTR(kPredCharIsLibrary),
        reportParseFlag, NULL },

    {   CFSTR(kPredNameHasPlugins), NULL,
        reportParseFlag, NULL },
    {   CFSTR(kPredNameIsPlugin), NULL,
        reportParseFlag, NULL },
    {   CFSTR(kPredNameHasDebugProperties), NULL,
        reportParseFlag, NULL },
    {   CFSTR(kPredNameIsKernelResource), NULL,
        reportParseFlag, NULL },
    {   CFSTR(kPredNameIntegrity), NULL,
        reportParseFlag, NULL },

    {   CFSTR(kPredNameVersion), CFSTR(kPredCharVersion),
        reportParseShorthand, reportEvalProperty },

    {   CFSTR(kPredNameArch), NULL,
        reportParseArch, reportEvalArch },
    {   CFSTR(kPredNameArchExact), CFSTR(kPredCharArchExact),
        reportParseArch, reportEvalArchExact },
    {   CFSTR(kPredNameExecutable), CFSTR(kPredCharExecutable),
        reportParseFlag, NULL },
    {   CFSTR(kPredNameSymbol), CFSTR(kPredCharSymbol),
        reportParseDefinesOrReferencesSymbol, reportEvalDefinesOrReferencesSymbol },

    {   CFSTR(kPredNameBundleID), CFSTR(kPredCharBundleID),
        reportParseShorthand, NULL },
    {   CFSTR(kPredNameBundleName), CFSTR(kPredCharBundleName),
        reportParseCommand, NULL },

    {   CFSTR(kPredNameDuplicate), CFSTR(kPredCharDuplicate),
        reportParseFlag, NULL },

    {   CFSTR(kPredNamePrint), NULL,
        reportParseCommand, NULL },
    {   CFSTR(kPredNamePrintProperty), CFSTR(kPredCharPrintProperty),
        reportParseCommand, NULL },
    {   CFSTR(kPredNamePrintArches), CFSTR(kPredCharPrintArches),
        reportParseCommand, NULL },
    {   CFSTR(kPredNamePrintDependencies), NULL,
        reportParseCommand, NULL },
    {   CFSTR(kPredNamePrintDependents), NULL,
        reportParseCommand, NULL },
    {   CFSTR(kPredNamePrintPlugins), NULL,
        reportParseCommand, NULL },
    {   CFSTR(kPredNamePrintIntegrity), NULL,
        reportParseCommand, NULL },
    {   CFSTR(kPredNamePrintInfoDictionary), CFSTR(kPredCharPrintInfoDictionary),
        reportParseCommand, NULL },
    {   CFSTR(kPredNamePrintExecutable), CFSTR(kPredCharPrintExecutable),
        reportParseCommand, NULL },

   /* These two special predicates are used internally for all "flag" and
    * command precidates, which all reset their predicates at reportParse time and
    * save the original keyword.
    */
    {   CFSTR(kPredNameFlag), NULL,
        NULL, reportEvalFlag },
    {   CFSTR(kPredNameCommand), NULL,
        NULL, reportEvalCommand },

    { NULL, NULL, NULL, NULL }  // sentinel to terminate list

};
