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
#ifndef _KEXTFIND_QUERY_H_
#define _KEXTFIND_QUERY_H_

#include "QEQuery.h"
#include "kextfind_tables.h"

/*******************************************************************************
* PREDICATES
*
* These are the basic atoms of a query expression.
*******************************************************************************/


/*****
 * Info Dictionary predicates.
 */
#define kPredNameProperty                "-property"
#define kPredNamePropertyExists          "-property-exists"
#define kPredNameMatchProperty           "-match-property"
#define kPredNameMatchPropertyExists     "-match-property-exists"

/*****
 * "Flag" predicates.
 *
 * Any predicate that is just "is the kext so?" is considered a boolean flag
 * and handled internally with the same eval callback, triggered by the
 * special kPredNameFlag predicate; the predicate keyword itself is stored
 * in the query element under the key "flag".
 */
#define kPredNameFlag                    "__flag__"

#define kPredNameLoaded                  "-loaded"

#define kPredNameValid                   "-valid"
#define kPredNameAuthentic               "-authentic"
#define kPredNameDependenciesMet         "-dependencies-met"
#define kPredNameLoadable                "-loadable"

#define kPredNameWarnings                "-warnings"

#define kPredNameIsLibrary               "-library"
#define kPredNameHasPlugins              "-has-plugins"
#define kPredNameIsPlugin                "-plugin"
#define kPredNameHasDebugProperties      "-debug"
#define kPredNameIsKernelResource        "-kernel-resource"

#define kPredNameDuplicate               "-duplicate-id"

// Flag Shorthands; I may dump these as they are all just logical
// negations of shorter predicates.
#define kPredNameInvalid                 "-invalid"
#define kPredNameInauthentic             "-inauthentic"
#define kPredNameDependenciesMissing     "-dependencies-missing"
#define kPredNameNonloadable             "-nonloadable"

/*****
 * Version-checking predicates.
 */
#define kPredNameVersion                 "-version"
#define kPredNameCompatibleWithVersion   "-compatible-with-version"

/*****
 * BOM-integrity predicates.
 *
 * Kext integrity is no longer used on SnowLeopard. We read the
 * flags but no kext will ever match them now.
 */
#define kPredNameIntegrity               "-integrity"

/*****
 * Executable-checking predicates.
 */
#define kPredNameArch                    "-arch"
#define kPredNameArchExact               "-arch-exact"
#define kPredNameExecutable              "-executable"
#define kPredNameNoExecutable            "-no-executable"

#define kPredNameDefinesSymbol           "-defines-symbol"
#define kPredNameReferencesSymbol        "-references-symbol"

/*****
 * Property shorthands.
 *
 * The bundle ID shorthands are direct property queries, and work with
 * the -case-insensitive command line option, but the OSBundleRequired
 * shorthands enforce strict case-matching for correct handling.
 */
#define kPredNameBundleID                "-bundle-id"
#define kPredNameBundleName              "-bundle-name"
#define kPredNameRoot                    "-root"
#define kPredNameConsole                 "-console"
#define kPredNameLocalRoot               "-local-root"
#define kPredNameNetworkRoot             "-network-root"
#define kPredNameSafeBoot                "-safe-boot"

/*****
 * Output commands.
 *
 * These all print some info and evaluate to true.
 *
 * All commands are handled internally with the same eval callback, triggered
 * by the special kPredNameCommand predicate; the command itself is stored
 * in the query element under the key "command".
 */
#define kPredNameCommand                 "__command__"

// Any command that begins with this, except for -print0 and -print-diagnostics,
// can take the -0 option to terminate output with a nul rather than a newline.
#define kPredPrefixPrint                 "-print"

#define kPredNameEcho                    "-echo"
#define kPredNameExec                    "-exec"
#define kPredNamePrint                   "-print"
#define kPredNamePrint0                  "-print0"
#define kPredNamePrintDiagnostics        "-print-diagnostics"
#define kPredNamePrintProperty           "-print-property"
#define kPredNamePrintMatchProperty      "-print-match-property"
#define kPredNamePrintArches             "-print-arches"
#define kPredNamePrintDependencies       "-print-dependencies"
#define kPredNamePrintDependents         "-print-dependents"
#define kPredNamePrintIntegrity          "-print-integrity"
#define kPredNamePrintPlugins            "-print-plugins"
#define kPredNamePrintInfoDictionary     "-print-info-dictionary"
#define kPredNamePrintExecutable         "-print-executable"


#define kExecInfoDictionaryReplace       "{info-dictionary}"
#define kExecExecutableReplace           "{executable}"
#define kExecBundlePathReplace           "{}"
#define kExecTerminator                  ";"

/*****
 * Shorter options for the more common predicates.
 */
#define kPredCharProperty                "-p"
#define kPredCharPropertyExists          "-pe"
#define kPredCharMatchProperty           "-m"
#define kPredCharMatchPropertyExists     "-me"

#define kPredCharValid                   "-v"
#define kPredCharAuthentic               "-a"
#define kPredCharDependenciesMet         "-d"
#define kPredCharLoadable                "-l"

#define kPredCharDuplicate               "-dup"

#define kPredCharInvalid                 "-nv"
#define kPredCharInauthentic             "-na"
#define kPredCharDependenciesMissing     "-nd"
#define kPredCharNonloadable             "-nl"

#define kPredCharWarnings                "-w"

#define kPredCharVersion                 "-V"
#define kPredCharIsLibrary               "-lib"

#define kPredCharArchExact               "-ax"
#define kPredCharExecutable              "-x"
#define kPredCharNoExecutable            "-nx"
#define kPredCharDefinesSymbol           "-dsym"
#define kPredCharReferencesSymbol        "-rsym"

#define kPredCharBundleID                "-b"
#define kPredCharBundleName              "-B"

#define kPredCharRoot                    "-R"
#define kPredCharConsole                 "-C"
#define kPredCharLocalRoot               "-L"
#define kPredCharNetworkRoot             "-N"
#define kPredCharSafeBoot                "-S"

//#define kPredCharPrint0                 "-p0"
#define kPredCharPrintDiagnostics        "-pdiag"
#define kPredCharPrintProperty           "-pp"
#define kPredCharPrintMatchProperty      "-pm"
#define kPredCharPrintArches             "-pa"
//#define kPredCharPrintDependencies     "-print-dependencies"
//#define kPredCharPrintDependents       "-print-dependents"
//#define kPredCharPrintPlugins          "-print-plugins"
#define kPredCharPrintInfoDictionary     "-pid"
#define kPredCharPrintExecutable         "-px"

/*******************************************************************************
* Stuff for query & reporting parse/eval.
*******************************************************************************/

/*****
 * Options to predicates. These do *not* include the hyphen
 * cause they get used with getopt_long_only().
 */
// Do not use -1, that's getopt() end-of-args return value
#define kPredOptNameNoNewline            "no-newline"
#define kPredOptNoNewline                'n'
#define kPredOptNameCaseInsensitive      kOptNameCaseInsensitive
#define kPredOptCaseInsensitive          kOptCaseInsensitive
#define kPredOptNameSubstring            "substring"
#define kPredOptSubstring                's'

/*****
 * These keys get stuffed into a query element dictionary to alter how it's
 * evaluated. (Maybe they should get put into the arguments.)
 */
#define kSearchStyleExact           "exact"
#define kSearchStyleCaseInsensitive "case-insensitive"
#define kSearchStyleSubstring       "substring"
#define kSearchStyleKeyExists       "exists"

/*****
 * XXX: These OSBundleRequired definitions should be done by the kext library.
 */
#define kOSBundleRequired             "OSBundleRequired"
#define kOSBundleRequiredRoot         "Root"
#define kOSBundleRequiredConsole      "Console"
#define kOSBundleRequiredLocalRoot    "Local-Root"
#define kOSBundleRequiredNetworkRoot  "Network-Root"
#define kOSBundleRequiredSafeBoot     "Safe Boot"

/*****
 * Command-line keywords for the five possible integrity states.
 *
 * Kext integrity is no longer used on SnowLeopard. We read the
 * flags but no kext will ever match them now.
 */
#define kIntegrityCorrect             "correct"
#define kIntegrityUnknown             "unknown"
#define kIntegrityNotApple            "not-apple"
#define kIntegrityNoReceipt           "no-receipt"
#define kIntegrityModified            "modified"

#define kIntegrityNotApplicable       "n/a"

#define kKeywordFlag    "flag"
#define kKeywordCommand "command"

#define kWordTrue  "true"
#define kWordYes   "yes"
#define kWord1     "1"
#define kWordFalse "false"
#define kWordNo    "no"
#define kWord0     "0"

/*******************************************************************************
* Query Engine Callbacks
*
* The Query Engine invokes these as it finds keywords from the above list
* in the command line or the query being evaluated.
*******************************************************************************/
Boolean parseArgument(
    CFMutableDictionaryRef element,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean parseBundleName(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean evalBundleName(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean parseProperty(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean parseShorthand(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean evalProperty(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean parseMatchProperty(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean evalMatchProperty(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean parseIntegrity(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean evalIntegrity(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean parseFlag(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean evalFlag(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean parseVersion(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean evalVersion(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean parseCompatibleWithVersion(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean evalCompatibleWithVersion(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean parseArch(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean evalArch(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean evalArchExact(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean parseCommand(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean parseDefinesOrReferencesSymbol(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean evalDefinesOrReferencesSymbol(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean evalCommand(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean parseExec(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean evalExec(
    CFDictionaryRef queryElement,
    void * object,
    void * user_data,
    QEQueryError * error);

#endif /* _KEXTFIND_QUERY_H_ */
