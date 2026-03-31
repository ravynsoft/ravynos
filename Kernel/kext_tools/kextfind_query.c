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
#include <IOKit/kext/fat_util.h>
#include <IOKit/kext/macho_util.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "QEQuery.h"

#include "kextfind_main.h"

#include "kextfind_query.h"
#include "kextfind_commands.h"

/*****
 * Version expressions on the command line get parsed into an operator
 * and one or two binary versions.
 */
typedef enum {
    kVersionNone,
    kVersionEqual,
    kVersionNotEqual,
    kVersionGreaterThan,
    kVersionGreaterOrEqual,
    kVersionLessThan,
    kVersionLessOrEqual,
    kVersionRange
} VersionOperator;


/*******************************************************************************
* Predicate option processing
*
* Include the query predicates and commands so that we can intelligently
* handle sub-options.
*******************************************************************************/
struct option echo_opt_info[] = {
    { kPredOptNameNoNewline, no_argument, NULL, kPredOptNoNewline },
    { kOptNameNulTerminate, no_argument, NULL,  kOptNulTerminate },
    QUERY_PREDICATES
    QUERY_COMMANDS
    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};

#define ECHO_OPTS  "0n"

/**********/

struct option property_opt_info[] = {
    { kPredOptNameCaseInsensitive, no_argument, NULL, kPredOptCaseInsensitive },
    { kPredOptNameSubstring,       no_argument, NULL, kPredOptSubstring },
    QUERY_PREDICATES
    QUERY_COMMANDS
    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};

#define PROPERTY_OPTS  "is"

/**********/

struct option print_opt_info[] = {
    { kOptNameNulTerminate, no_argument, NULL, kOptNulTerminate },
    QUERY_PREDICATES
    QUERY_COMMANDS
    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};

#define PRINT_OPTS  "0"


/*******************************************************************************
* Module-private functions.
*******************************************************************************/
static int parseVersionArg(const char * string,
    VersionOperator * versionOperator,
    OSKextVersion * version1,
    OSKextVersion * version2,
    uint32_t * error);


Boolean parseStringElementOptions(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * index,
    QueryContext * context,
    QEQueryError * error);

Boolean parseEchoOptions(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * index,
    QueryContext * context,
    QEQueryError * error);

Boolean parsePrintOptions(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * index,
    QueryContext * context,
    QEQueryError * error);

Boolean handleNonOption(int opt_char,
    char * const argv[],
    QueryContext * context,
    QEQueryError * error);

/*******************************************************************************
*
*******************************************************************************/
Boolean parseArgument(
    CFMutableDictionaryRef element,
    char * const argv[],
    uint32_t * num_used,
    void * user_data __unused,
    QEQueryError * error)
{
    Boolean result        = false;
    uint32_t index        = 0;    // this function starts with the arg itself
    CFStringRef arg      = NULL;  // must release

    if (!argv[index]) {
        *error = kQEQueryErrorInvalidOrMissingArgument;
        goto finish;
    }

    arg = CFStringCreateWithCString(kCFAllocatorDefault, argv[index],
        kCFStringEncodingUTF8);
    index++;

    *num_used += index;  // this is not a QE callback! :-)

    QEQueryElementAppendArgument(element, arg);

    result = true;
finish:
    if (arg)      CFRelease(arg);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseProperty(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    QueryContext * context = (QueryContext *)user_data;
    CFStringRef predicate = NULL;  // don't release
    Boolean needValue     = true;
    uint32_t index        = 1;

    predicate = QEQueryElementGetPredicate(element);

     if (CFEqual(predicate, CFSTR(kPredNamePropertyExists))) {

        CFDictionarySetValue(element, CFSTR(kSearchStyleKeyExists), kCFBooleanTrue);
        needValue = false;

    } else if (!parseStringElementOptions(element, argc, argv, &index,
        context, error)) {
        goto finish;
    }

   /* Fudge the predicate so we can use one eval callback.
    */
    QEQueryElementSetPredicate(element, CFSTR(kPredNameProperty));

    if (!parseArgument(element, &argv[index], &index, user_data, error)) {
        goto finish;
    }
    if (needValue) {
        if (!parseArgument(element, &argv[index], &index, user_data, error)) {
            goto finish;
        }
    }

    result = true;
finish:
    *num_used += index;
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseBundleName(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    QueryContext * context = (QueryContext *)user_data;
    uint32_t index = 1;

    if (!parseStringElementOptions(element, argc, argv, &index,
        context, error)) {
        goto finish;
    }

    if (!parseArgument(element, &argv[index], &index, user_data, error)) {
        goto finish;
    }

    QEQueryElementSetPredicate(element, CFSTR(kPredNameBundleName));
    result = true;

finish:
    *num_used += index;
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean evalBundleName(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error __unused)
{
    Boolean        result     = false;
    QueryContext * context    = (QueryContext *)user_data;
    OSKextRef      theKext    = (OSKextRef)object;
    CFStringRef    queryName  = QEQueryElementGetArgumentAtIndex(element, 0);
    CFURLRef       kextURL    = NULL;  // do not release
    CFStringRef    bundleName = NULL; // must release

    kextURL = OSKextGetURL(theKext);
    if (!kextURL) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Kext has no URL!");
        goto finish;
    }
    bundleName = CFURLCopyLastPathComponent(kextURL);
    if (!bundleName) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Kext has bad URL.");
        goto finish;
    }

    CFOptionFlags searchOptions = 0;

    if (context->caseInsensitive ||
        CFDictionaryGetValue(element,
            CFSTR(kSearchStyleCaseInsensitive))) {

         searchOptions |= kCFCompareCaseInsensitive;
    }

   /* If the global or predicate substring flag was set, do a substring
    * match.
    */
    if (context->substrings ||
        CFDictionaryGetValue(element, CFSTR(kSearchStyleSubstring))) {
        CFRange findResult = CFStringFind(bundleName,
            queryName, searchOptions);

        if (findResult.location != kCFNotFound) {
            result = true;
        }

    } else {
        CFComparisonResult compareResult = CFStringCompareWithOptions(
            bundleName, queryName,
            CFRangeMake(0, CFStringGetLength(bundleName)),
            searchOptions);

        if (compareResult == kCFCompareEqualTo) {
            result = true;
        }
    }

finish:
    SAFE_RELEASE(bundleName);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseShorthand(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    QueryContext * context = (QueryContext *)user_data;
    CFStringRef predicate = QEQueryElementGetPredicate(element);
    uint32_t index = 1;

    if (CFEqual(predicate, CFSTR(kPredNameBundleID))) {

        if (!parseStringElementOptions(element, argc, argv, &index,
            context, error)) {
            goto finish;
        }

        QEQueryElementAppendArgument(element, kCFBundleIdentifierKey);
        if (!parseArgument(element, &argv[index], &index, user_data, error)) {
            goto finish;
        }
    } else {

        if (CFEqual(predicate, CFSTR(kPredNameRoot))) {

            QEQueryElementAppendArgument(element, CFSTR(kOSBundleRequired));
            QEQueryElementAppendArgument(element, CFSTR(kOSBundleRequiredRoot));
            CFDictionarySetValue(element, CFSTR(kSearchStyleExact), kCFBooleanTrue);

        } else if (CFEqual(predicate, CFSTR(kPredNameConsole))) {

            QEQueryElementAppendArgument(element, CFSTR(kOSBundleRequired));
            QEQueryElementAppendArgument(element, CFSTR(kOSBundleRequiredConsole));
            CFDictionarySetValue(element, CFSTR(kSearchStyleExact), kCFBooleanTrue);

        } else if (CFEqual(predicate, CFSTR(kPredNameLocalRoot))) {

            QEQueryElementAppendArgument(element, CFSTR(kOSBundleRequired));
            QEQueryElementAppendArgument(element, CFSTR(kOSBundleRequiredLocalRoot));
            CFDictionarySetValue(element, CFSTR(kSearchStyleExact), kCFBooleanTrue);

        } else if (CFEqual(predicate, CFSTR(kPredNameNetworkRoot))) {

            QEQueryElementAppendArgument(element, CFSTR(kOSBundleRequired));
            QEQueryElementAppendArgument(element, CFSTR(kOSBundleRequiredNetworkRoot));
            CFDictionarySetValue(element, CFSTR(kSearchStyleExact), kCFBooleanTrue);

        } else if (CFEqual(predicate, CFSTR(kPredNameSafeBoot))) {

            QEQueryElementAppendArgument(element, CFSTR(kOSBundleRequired));
            QEQueryElementAppendArgument(element, CFSTR(kOSBundleRequiredSafeBoot));
            CFDictionarySetValue(element, CFSTR(kSearchStyleExact), kCFBooleanTrue);
        } else {
            goto finish;
        }
    }

    QEQueryElementSetPredicate(element, CFSTR(kPredNameProperty));
    result = true;

finish:
    *num_used += index;
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean _evalPropertyInDict(
    CFDictionaryRef   element,
    void            * object,
    CFDictionaryRef   searchDict,
    void            * user_data,
    QEQueryError    * error)
{
    Boolean              result         = false;
    OSKextRef            theKext        = (OSKextRef)object;
    QueryContext       * context        = (QueryContext *)user_data;
    CFStringRef          propName       = NULL;
    CFStringRef          queryValue     = NULL;
    CFTypeRef            foundValue     = NULL;
    CFTypeID             foundType      = 0;
    CFLocaleRef          locale         = NULL;  // must release
    CFNumberFormatterRef formatter      = NULL;  // must release
    CFNumberRef          searchNumber   = NULL;  // must release
    char               * scratchCString = NULL;  // must free

    propName = QEQueryElementGetArgumentAtIndex(element, 0);
    if (searchDict) {
        foundValue = CFDictionaryGetValue(searchDict, propName);
    } else {
        foundValue = OSKextGetValueForInfoDictionaryKey(theKext, propName);
    }
    if (!foundValue) {
        goto finish;
    }
    foundType = CFGetTypeID(foundValue);

    if (CFDictionaryGetValue(element, CFSTR(kSearchStyleKeyExists))) {
        if (foundValue) {
            result = true;
        }
        goto finish;
    }

    queryValue = QEQueryElementGetArgumentAtIndex(element, 1);

    if (foundType == CFStringGetTypeID()) {

       /* Exact searches trump any global settings in the query context. */
        if (CFDictionaryGetValue(element, CFSTR(kSearchStyleExact))) {
            result = CFEqual(foundValue, queryValue);
            goto finish;
        } else {
            CFOptionFlags searchOptions = 0;
            if (context->caseInsensitive ||
                CFDictionaryGetValue(element,
                    CFSTR(kSearchStyleCaseInsensitive))) {

                 searchOptions |= kCFCompareCaseInsensitive;
            }

           /* If the global or predicate substring flag was set, do a substring
            * match.
            */
            if (context->substrings ||
                CFDictionaryGetValue(element, CFSTR(kSearchStyleSubstring))) {
                CFRange findResult = CFStringFind(foundValue,
                    queryValue, searchOptions);

                if (findResult.location != kCFNotFound) {
                    result = true;
                }

            } else {
                CFComparisonResult compareResult = CFStringCompareWithOptions(
                    foundValue, queryValue,
                    CFRangeMake(0, CFStringGetLength(foundValue)),
                    searchOptions);

                if (compareResult == kCFCompareEqualTo) {
                    result = true;
                }
            }
            goto finish;
        }
    } else if (foundType == CFNumberGetTypeID()) {
       /* Don't apply the global query context's substrings flag to numbers. */
        if (CFDictionaryGetValue(element, CFSTR(kSearchStyleSubstring))) {

           /* Substrings on numeric values doesn't really make sense.
            */
            goto finish;
        } else {
            CFNumberRef foundNumber = NULL;         // do not release
            CFRange     stringRange;

            foundNumber = (CFNumberRef)foundValue;

            locale = CFLocaleCopyCurrent();
            if (!locale) {
                *error = kQEQueryErrorEvaluationCallbackFailed;
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Can't get current locale.");
                goto finish;
            }

            formatter = CFNumberFormatterCreate(kCFAllocatorDefault, locale,
                kCFNumberFormatterNoStyle);
            if (!formatter) {
                *error = kQEQueryErrorEvaluationCallbackFailed;
                goto finish;
            }

            stringRange = CFRangeMake(0, CFStringGetLength(queryValue));

            searchNumber = CFNumberFormatterCreateNumberFromString(
               kCFAllocatorDefault, formatter, queryValue,
               &stringRange, 0);
            if (!formatter) {
                *error = kQEQueryErrorEvaluationCallbackFailed;
                scratchCString = createUTF8CStringForCFString(queryValue);
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    "Failed to parse number string '%s'.",
                    scratchCString);
                SAFE_FREE_NULL(scratchCString);
                goto finish;
            }

            if (CFEqual(foundNumber, searchNumber)) {
                result = true;
                goto finish;
            }
        }


    } else if (foundType == CFBooleanGetTypeID()) {
       /* Don't apply the global query context's substrings flag to numbers. */
        if (CFDictionaryGetValue(element, CFSTR(kSearchStyleSubstring))) {

           /* Substrings on boolean values really don't make sense.
            */
            goto finish;
        } else {
            CFBooleanRef foundBoolean = (CFBooleanRef)foundValue;
            if (CFBooleanGetValue(foundBoolean) == true)  {
                if (CFEqual(queryValue, CFSTR(kWordTrue)) ||
                    CFEqual(queryValue, CFSTR(kWordYes)) ||
                    CFEqual(queryValue, CFSTR(kWord1))) {

                    result = true;
                    goto finish;
                }
            } else {
                if (CFEqual(queryValue, CFSTR(kWordFalse)) ||
                    CFEqual(queryValue, CFSTR(kWordNo)) ||
                    CFEqual(queryValue, CFSTR(kWord0))) {
                    result = true;
                    goto finish;
                }
            }
        }
    }

finish:

    SAFE_RELEASE(locale);
    SAFE_RELEASE(formatter);
    SAFE_RELEASE(searchNumber);
    SAFE_FREE(scratchCString);

    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean evalProperty(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error)
{
    Boolean   result  = false;

    result = _evalPropertyInDict(element, object, /* dict */ NULL,
        user_data, error);

    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseMatchProperty(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    Boolean result        = false;
    QueryContext * context = (QueryContext *)user_data;
    CFStringRef predicate = NULL;  // don't release
    Boolean needValue     = true;
    uint32_t index        = 1;

    predicate = QEQueryElementGetPredicate(element);

    if (CFEqual(predicate, CFSTR(kPredNameMatchPropertyExists))) {

        CFDictionarySetValue(element, CFSTR(kSearchStyleKeyExists), kCFBooleanTrue);
        needValue = false;

    } else {
        if (!parseStringElementOptions(element, argc, argv, &index,
            context, error)) {
            goto finish;
        }
    }

   /* Fudge the predicate so we can use one eval callback.
    */
    QEQueryElementSetPredicate(element, CFSTR(kPredNameMatchProperty));

    if (!parseArgument(element, &argv[index], &index, user_data, error)) {
        goto finish;
    }
    if (needValue) {
        if (!parseArgument(element, &argv[index], &index, user_data, error)) {
            goto finish;
        }
    }

    result = true;
finish:
    *num_used += index;
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean evalMatchProperty(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    OSKextRef theKext = (OSKextRef)object;
    CFArrayRef personalities = OSKextCopyPersonalitiesArray(theKext);
    CFIndex count, i;

    if (!personalities) {
        goto finish;
    }

    count = CFArrayGetCount(personalities);
    for (i = 0; i < count; i++) {
        CFDictionaryRef personality = CFArrayGetValueAtIndex(personalities, i);
        if (_evalPropertyInDict(element, object, personality,
            user_data, error)) {

            result = true;
            goto finish;
        }
    }

finish:
    if (personalities) CFRelease(personalities);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseIntegrity(
    CFMutableDictionaryRef element,
    int argc __unused,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    Boolean result        = false;
    QueryContext * context = (QueryContext *)user_data;
    uint32_t index        = 1;     // don't care about predicate
    CFStringRef name      = NULL;  // must release
    CFStringRef value     = NULL;  // must release

    if (!argv[index]) {
        *error = kQEQueryErrorInvalidOrMissingArgument;
        goto finish;
    }

    if (strcmp(argv[index], kIntegrityCorrect) &&
        strcmp(argv[index], kIntegrityUnknown) &&
        strcmp(argv[index], kIntegrityNotApple) &&
        strcmp(argv[index], kIntegrityNoReceipt) &&
        strcmp(argv[index], kIntegrityModified)) {

        *error = kQEQueryErrorInvalidOrMissingArgument;
        goto finish;
    }

    value = CFStringCreateWithCString(kCFAllocatorDefault, argv[index],
        kCFStringEncodingUTF8);
    index++;

    *num_used += index;

    QEQueryElementAppendArgument(element, value);

    context->checkIntegrity = true;

   /* Kext integrity is no longer used on SnowLeopard.
    */
    OSKextLog(/* kext */ NULL,
        kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
        "Integrity states are no longer used; no kexts will match.");

    result = true;
finish:
    if (name)  CFRelease(name);
    if (value) CFRelease(value);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean evalIntegrity(
    CFDictionaryRef element __unused,
    void * object __unused,
    void * user_data __unused,
    QEQueryError * error __unused)
{
    Boolean result = false;

   /* Kext integrity is no longer used on SnowLeopard.
    */
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseFlag(
    CFMutableDictionaryRef element,
    int argc __unused,
    char * const argv[] __unused,
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error __unused)
{
    CFStringRef flag = QEQueryElementGetPredicate(element);
    QueryContext * context = (QueryContext *)user_data;

    QEQueryElementSetPredicate(element, CFSTR(kPredNameFlag));
    CFDictionarySetValue(element, CFSTR(kKeywordFlag), flag);

    if (CFEqual(flag, CFSTR(kPredNameLoaded))) {
        context->checkLoaded = true;
    }

    *num_used += 1;

    return true;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean evalFlag(
    CFDictionaryRef element,
    void * object,
    void * user_data __unused,
    QEQueryError * error __unused)
{
    Boolean     result  = false;
    OSKextRef   theKext = (OSKextRef)object;
    CFStringRef flag    = CFDictionaryGetValue(element, CFSTR(kKeywordFlag));

    if (CFEqual(flag, CFSTR(kPredNameLoaded))) {
        return OSKextIsLoaded(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameDuplicate))) {
        CFStringRef kextIdentifier = OSKextGetIdentifier(theKext);
        if (kextIdentifier) {
            CFArrayRef kexts = OSKextCopyKextsWithIdentifier(kextIdentifier);
            if (kexts && CFArrayGetCount(kexts) > 1) {
                result = true;
            }
            SAFE_RELEASE(kexts);
        }
    } else if (CFEqual(flag, CFSTR(kPredNameValid))) {
        return OSKextIsValid(theKext);
    }  if (CFEqual(flag, CFSTR(kPredNameInvalid))) {
        return !OSKextIsValid(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameAuthentic))) {
        return OSKextIsAuthentic(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameInauthentic))) {
        return !OSKextIsAuthentic(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameDependenciesMet))) {
        return OSKextResolveDependencies(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameDependenciesMissing))) {
        return !OSKextResolveDependencies(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameLoadable))) {
        return OSKextIsLoadable(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameNonloadable))) {
        return !OSKextIsLoadable(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameWarnings))) {
        CFDictionaryRef warnings = OSKextCopyDiagnostics(theKext,
            kOSKextDiagnosticsFlagWarnings);
        if (warnings && CFDictionaryGetCount(warnings)) {
            result = true;
        }
        SAFE_RELEASE(warnings);
    } else if (CFEqual(flag, CFSTR(kPredNameIsLibrary))) {
        if (OSKextGetCompatibleVersion(theKext) > 0) {
            result = true;
        }
    } else if (CFEqual(flag, CFSTR(kPredNameHasPlugins))) {
        CFArrayRef plugins = OSKextCopyPlugins(theKext);
        if (plugins && CFArrayGetCount(plugins)) {
            result = true;
        }
        SAFE_RELEASE(plugins);
    } else if (CFEqual(flag, CFSTR(kPredNameIsPlugin))) {
        return OSKextIsPlugin(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameHasDebugProperties))) {
        return OSKextHasLogOrDebugFlags(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameIsKernelResource))) {
        return OSKextIsKernelComponent(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameExecutable))) {
        return OSKextDeclaresExecutable(theKext);
    } else if (CFEqual(flag, CFSTR(kPredNameNoExecutable))) {
        return !OSKextDeclaresExecutable(theKext);
    }

    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseVersion(
    CFMutableDictionaryRef element,
    int argc __unused,
    char * const argv[],
    uint32_t * num_used,
    void * user_data __unused,
    QEQueryError * error)
{
    Boolean result        = false;
    uint32_t index        = 1;   // don't care about predicate
    VersionOperator  versionOperator;
    OSKextVersion     version1 = 0;
    OSKextVersion     version2 = 0;
    CFNumberRef      vOpNum = NULL;     // must release
    CFDataRef        version1Data = NULL;  // must release
    CFDataRef        version2Data = NULL;  // must release

    if (!argv[index]) {
        *error = kQEQueryErrorInvalidOrMissingArgument;
        goto finish;
    }

    if (!parseVersionArg(argv[index], &versionOperator, &version1, &version2, error)) {
        goto finish;
    }
    index++;

    vOpNum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type,
        &versionOperator);
    if (!vOpNum) {
        *error = kQEQueryErrorNoMemory;
        goto finish;
    }

    version1Data = CFDataCreate(kCFAllocatorDefault, (UInt8 *)&version1, sizeof(version1));
    if (!version1Data) {
        *error = kQEQueryErrorNoMemory;
        goto finish;
    }

    version2Data = CFDataCreate(kCFAllocatorDefault, (UInt8 *)&version2, sizeof(version2));
    if (!version2Data) {
        *error = kQEQueryErrorNoMemory;
        goto finish;
    }

    QEQueryElementAppendArgument(element, vOpNum);
    QEQueryElementAppendArgument(element, version1Data);
    QEQueryElementAppendArgument(element, version2Data);

    result = true;
finish:
    *num_used += index;

    if (vOpNum)          CFRelease(vOpNum);
    if (version1Data)    CFRelease(version1Data);
    if (version2Data)    CFRelease(version2Data);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean evalVersion(
    CFDictionaryRef element,
    void * object,
    void * user_data __unused,
    QEQueryError * error)
{
    Boolean result = false;
    OSKextRef theKext = (OSKextRef)object;
    OSKextVersion kextVers = OSKextGetVersion(theKext);
    CFNumberRef      vOpNum = NULL;     // must release
    CFDataRef        version1Data = NULL;  // must release
    CFDataRef        version2Data = NULL;  // must release
    VersionOperator  versionOperator;
    OSKextVersion     version1;
    OSKextVersion     version2;

    vOpNum = QEQueryElementGetArgumentAtIndex(element, 0);
    version1Data = QEQueryElementGetArgumentAtIndex(element, 1);
    version2Data = QEQueryElementGetArgumentAtIndex(element, 2);

    if (!CFNumberGetValue(vOpNum,kCFNumberSInt32Type,&versionOperator)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Internal error getting version operator.");
        *error = kQEQueryErrorEvaluationCallbackFailed;
        goto finish;
    }

    version1 = *(OSKextVersion *)CFDataGetBytePtr(version1Data);
    version2 = *(OSKextVersion *)CFDataGetBytePtr(version2Data);

    switch (versionOperator) {
      case kVersionEqual:
        if (kextVers == version1) {
            result = true;
        }
        break;
      case kVersionNotEqual:
        if (kextVers != version1) {
            result = true;
        }
        break;
      case kVersionGreaterThan:
        if (kextVers > version1) {
            result = true;
        }
        break;
      case kVersionGreaterOrEqual:
        if (kextVers >= version1) {
            result = true;
        }
        break;
      case kVersionLessThan:
        if (kextVers < version1) {
            result = true;
        }
        break;
      case kVersionLessOrEqual:
        if (kextVers <= version1) {
            result = true;
        }
        break;
      case kVersionRange:
        if ((kextVers >= version1) && (kextVers <= version2)) {
            result = true;
        }
        break;
      default:
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Internal evaluation error.");
        *error = kQEQueryErrorEvaluationCallbackFailed;
        break;
    }
finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseCompatibleWithVersion(
    CFMutableDictionaryRef element,
    int argc __unused,
    char * const argv[],
    uint32_t * num_used,
    void * user_data __unused,
    QEQueryError * error)
{
    Boolean      result = false;
    uint32_t     index = 1;           // don't care about predicate
    OSKextVersion compatible_version = 0;
    CFDataRef    versionData = NULL;  // must release

    if (!argv[index]) {
        *error = kQEQueryErrorInvalidOrMissingArgument;
        goto finish;
    }

    compatible_version = OSKextParseVersionString(argv[index]);
    if (compatible_version == -1) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Invalid version string '%s'.", argv[index]);
        goto finish;
    }
    index++;

    versionData = CFDataCreate(kCFAllocatorDefault,
        (UInt8 *)&compatible_version, sizeof(compatible_version));
    if (!versionData) {
        *error = kQEQueryErrorNoMemory;
        goto finish;
    }

    QEQueryElementAppendArgument(element, versionData);

    result = true;
finish:
    *num_used += index;
    if (versionData)    CFRelease(versionData);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean evalCompatibleWithVersion(
    CFDictionaryRef element,
    void * object,
    void * user_data __unused,
    QEQueryError * error __unused)
{
    Boolean result = false;
    OSKextRef theKext = (OSKextRef)object;
    CFDataRef    versionData = NULL;  // must release
    OSKextVersion compatible_version = 0;

    versionData = QEQueryElementGetArgumentAtIndex(element, 0);

    compatible_version = *(OSKextVersion *)CFDataGetBytePtr(versionData);

    if (OSKextIsCompatibleWithVersion(theKext, compatible_version)) {
        result = true;
    }

    return result;
}

/*******************************************************************************
* parseVersionArg()
*******************************************************************************/
static int parseVersionArg(const char * string,
    VersionOperator * versionOperator,
    OSKextVersion * version1,
    OSKextVersion * version2,
    uint32_t * error)
{
    int result = -1;        // assume parse error
    char * scratch = NULL;  // must free
    char * v1string = NULL; // don't free
    char * hyphen = NULL;   // don't free
    char * v2string = NULL; // don't free

    scratch = strdup(string);
    if (!scratch) {
        OSKextLogMemError();
        result = 0;
        goto finish;
    }

    v1string = scratch;

    *versionOperator = kVersionNone;

    switch (scratch[0]) {
      case 'e':
        *versionOperator = kVersionEqual;
        v1string = &scratch[1];
        break;
      case 'n':
        if (scratch[1] != 'e') {
            goto finish;
        }
        *versionOperator = kVersionNotEqual;
        v1string = &scratch[2];
        break;
      case 'g':
        if (scratch[1] == 'e') {
            *versionOperator = kVersionGreaterOrEqual;
        } else if (scratch[1] == 't') {
            *versionOperator = kVersionGreaterThan;
        } else {
            goto finish;
        }
        v1string = &scratch[2];

        break;
      case 'l':
        if (scratch[1] == 'e') {
            *versionOperator = kVersionLessOrEqual;
        } else if (scratch[1] == 't') {
            result = kVersionLessThan;
        } else {
            goto finish;
        }
        v1string = &scratch[2];
        break;
    }

    hyphen = index(v1string, '-');
    if (hyphen) {
        if (*versionOperator != kVersionNone) {
            goto finish;
        }
        *versionOperator = kVersionRange;
        v2string = hyphen + 1;
        *hyphen = '\0';
    }

    *version1 = OSKextParseVersionString(v1string);
    if (*version1 == -1) {
        goto finish;
    }

    if (hyphen) {
        *version2 = OSKextParseVersionString(v2string);
        if (*version2 == -1) {
            goto finish;
        }
    }

    if (*versionOperator == kVersionNone) {
        *versionOperator = kVersionEqual;
    }

    result = 1;

finish:
    if (scratch) {
        free(scratch);
        scratch = NULL;
    }
    if (result == -1) {
        *error = kQEQueryErrorInvalidOrMissingArgument;
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Invalid version string '%s'.", string);
        result = 0;
    }
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseArch(
    CFMutableDictionaryRef element,
    int argc __unused,
    char * const argv[],
    uint32_t * num_used,
    void * user_data __unused,
    QEQueryError * error)
{
    Boolean      result = false;
    uint32_t     index = 1;          // don't care about predicate
    CFStringRef archString = NULL;  // must release
    CFArrayRef   arches = NULL;  // must release
    CFIndex count, i;
    char * arch = NULL;  // must free

    if (!argv[index]) {
        *error = kQEQueryErrorInvalidOrMissingArgument;
        goto finish;
    }

    archString = CFStringCreateWithCString(kCFAllocatorDefault,
            argv[index], kCFStringEncodingUTF8);
    if (!archString) {
        *error = kQEQueryErrorNoMemory;
        goto finish;
    }
    index++;

    arches = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault,archString, CFSTR(","));
    if (!arches) {
        goto finish;
    }

    count = CFArrayGetCount(arches);
    for (i = 0; i < count; i++) {
        CFStringRef archString = NULL;
        const NXArchInfo * archinfo = NULL;

        archString = CFArrayGetValueAtIndex(arches, i);
        arch = createUTF8CStringForCFString(archString);
        if (!arch) {
            OSKextLogStringError(/* kext */ NULL);
            goto finish;
        }
        archinfo = NXGetArchInfoFromName(arch);
        if (!archinfo) {
            OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Unknown architecture %s.", arch);
            *error = kQEQueryErrorInvalidOrMissingArgument;
            goto finish;
        }
        free(arch);
        arch = NULL;
    }

    QEQueryElementSetArgumentsArray(element, arches);
    result = true;
finish:
    *num_used += index;
    if (archString) CFRelease(archString);
    if (arches) CFRelease(arches);
    if (arch)   free(arch);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean _checkArches(
    OSKextRef  theKext,
    CFArrayRef arches)
{
    Boolean result = false;
    CFIndex count, i;
    char * arch = NULL;  // must free

    count = CFArrayGetCount(arches);
    for (i = 0; i < count; i++) {
        CFStringRef archString = NULL;
        const NXArchInfo * archinfo = NULL;

        archString = CFArrayGetValueAtIndex(arches, i);
        arch = createUTF8CStringForCFString(archString);
        if (!arch) {
            OSKextLogStringError(theKext);
            goto finish;
        }
        archinfo = NXGetArchInfoFromName(arch);
        if (!archinfo) {
            goto finish;
        }
        free(arch);
        arch = NULL;
        if (!OSKextSupportsArchitecture(theKext, archinfo)) {
            goto finish;
        }
    }

    result = true;

finish:
    SAFE_FREE(arch);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean evalArch(
    CFDictionaryRef element,
    void * object,
    void * user_data __unused,
    QEQueryError * error __unused)
{
    OSKextRef theKext = (OSKextRef)object;
    CFArrayRef   arches = NULL;  // do not release

    arches = QEQueryElementGetArguments(element);
    if (!arches) {
        return false;
    }
    return _checkArches(theKext, arches);
}

/*******************************************************************************
*
*******************************************************************************/
typedef struct {
    struct fat_header fat_hdr;
    struct fat_arch   fat_arch;
} FakeFatHeader;

Boolean evalArchExact(
    CFDictionaryRef element,
    void * object,
    void * user_data __unused,
    QEQueryError * error __unused)
{
    Boolean result = false;
    OSKextRef theKext = (OSKextRef)object;
    CFArrayRef   arches = NULL;  // do not release
    CFIndex count, i;
    fat_iterator fiter = NULL;  // must close
    char * arch = NULL;  // must free
    struct mach_header * farch;
    const NXArchInfo * archinfo = NULL;

    fiter = createFatIteratorForKext(theKext);
    if (!fiter) {
        goto finish;
    }

    arches = QEQueryElementGetArguments(element);
    if (!arches) {
        goto finish;
    }
    count = CFArrayGetCount(arches);

   /* First make sure every architecture requested exists in the executable.
    */
    if (!_checkArches(theKext, arches)) {
        goto finish;
    }

   /* Now make sure every arch in the executable is represtented by at least
    * one requested arch.
    */
    while ((farch = (struct mach_header *)fat_iterator_next_arch(fiter, NULL))) {
        int swap = 0;
        struct fat_arch fakeFatArch;
        Boolean thisArchFound = false;
#if DEBUG
        const NXArchInfo * info;
#endif

        if (farch->magic == MH_CIGAM || farch->magic == MH_CIGAM_64) {
            swap = 1;
        }
        fakeFatArch.cputype = CondSwapInt32(swap, farch->cputype);
        fakeFatArch.cpusubtype = CondSwapInt32(swap, farch->cpusubtype);

#if DEBUG
info = NXGetArchInfoFromCpuType(fakeFatArch.cputype, fakeFatArch.cpusubtype);
fprintf(stderr, "      checking architecture %s\n", info->name);
#endif

       /* Find at least one requested arch that matches our faked-up fat
        * header.
        */
        for (i = 0; i < count; i++) {
            CFStringRef archString = CFArrayGetValueAtIndex(arches, i);
            arch = createUTF8CStringForCFString(archString);
            if (!arch) {
                OSKextLogStringError(theKext);
                goto finish;
            }
            archinfo = NXGetArchInfoFromName(arch);
            if (!archinfo) {
                goto finish;
            }
            free(arch);
            arch = NULL;
#if DEBUG
fprintf(stderr, "            %s? ", archinfo->name);
#endif
            if (NXFindBestFatArch(archinfo->cputype, archinfo->cpusubtype,
                 &fakeFatArch, 1)) {
                thisArchFound = true;
                break;

#if DEBUG
fprintf(stderr, "yes\n");
#endif
            } else {
#if DEBUG
fprintf(stderr, "no\n");
#endif
            }
        }

        if (!thisArchFound) {
            goto finish;
        }
    }

    result = true;

finish:
    if (arch)  free(arch);
    if (fiter) fat_iterator_close(fiter);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseDefinesOrReferencesSymbol(
    CFMutableDictionaryRef element,
    int argc __unused,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    uint32_t index = 1;  // don't care about predicate

    if (!parseArgument(element, &argv[index], &index, user_data, error)) {
        goto finish;
    }
    result = true;
finish:
    *num_used += index;
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean evalDefinesOrReferencesSymbol(
    CFDictionaryRef   element,
    void            * object,
    void            * user_data __unused,
    QEQueryError    * error __unused)
{
    Boolean              result           = false;
    OSKextRef            theKext          = (OSKextRef)object;
    CFStringRef          predicate        = NULL;  // don't release
    Boolean              seekingReference = false;
    char               * symbol           = NULL;  // must free
    fat_iterator         fiter            = NULL;  // must close
    struct mach_header * farch            = NULL;
    void               * farch_end        = NULL;
    Boolean              isKernelComponent;
    uint8_t              nlist_type;

    predicate = QEQueryElementGetPredicate(element);
    if (CFEqual(predicate, CFSTR(kPredNameReferencesSymbol))) {
        seekingReference = true;
    }

    symbol = createUTF8CStringForCFString(
        QEQueryElementGetArgumentAtIndex(element, 0));
    if (!symbol) {
        goto finish;
    }
    fiter = createFatIteratorForKext(theKext);
    if (!fiter) {
        goto finish;
    }

   /* KPI kexts have the symbols listed as undefined, and won't have
    * any unresolved references to anything. So, if seekingReference
    * is true, we have nothing to do.
    */
    isKernelComponent = OSKextIsKernelComponent(theKext);
    if (isKernelComponent && seekingReference) {
        goto finish;
    }

    while ((farch = fat_iterator_next_arch(fiter, &farch_end))) {
        macho_seek_result seek_result = macho_find_symbol(
            farch, farch_end, symbol, &nlist_type, NULL);

        if (seek_result == macho_seek_result_found_no_value ||
            seek_result == macho_seek_result_found) {

            uint8_t n_type = N_TYPE & nlist_type;

           /* A non-kernel component symbol matches if we're seeking:
            *   - a reference (-rsym) and the symbol n_type is N_UNDF, or
            *   - a definition (-dsym) and the symbol n_type is anything else.
            *
            * For kernel components we only care about defined symbols,
            * and in a KPI file those will be either N_UNDF or N_INDR.
            */
            if (!isKernelComponent) {
                if ((seekingReference && (n_type == N_UNDF)) ||
                    (!seekingReference && (n_type != N_UNDF))) {

                    result = true;
                }
            } else {
                if ((!seekingReference && (n_type == N_UNDF || n_type == N_INDR))) {
                    result = true;
                }
            }

            if (result) {
                goto finish;
            }

            if ((seekingReference && (n_type == N_UNDF || n_type == N_INDR)) ||
                (!seekingReference && (n_type != N_UNDF))) {

                result = true;
                goto finish;
            }
        }
    }
finish:
    if (fiter)  fat_iterator_close(fiter);
    if (symbol) free(symbol);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseCommand(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    QueryContext * context = (QueryContext *)user_data;
    CFStringRef predicate = QEQueryElementGetPredicate(element);
    uint32_t index = 1;

    CFDictionarySetValue(element, CFSTR(kKeywordCommand), predicate);
    QEQueryElementSetPredicate(element, CFSTR(kPredNameCommand));

   /* For the echo command, look for a -n option or -- to end options.
    */
    if (CFEqual(predicate, CFSTR(kPredNameEcho))) {
        if (!parseEchoOptions(element, argc, argv, &index,
            context, error)) {
            goto finish;
        }

    } else if (CFStringHasPrefix(predicate, CFSTR(kPredPrefixPrint)) &&
        !CFEqual(predicate, CFSTR(kPredNamePrint0)) &&
        !CFEqual(predicate, CFSTR(kPredNamePrintDiagnostics))) {

        if (!parsePrintOptions(element, argc, argv, &index,
            context, error)) {
            goto finish;
        }

    } else if (CFEqual(predicate, CFSTR(kPredNamePrint0))) {
        CFDictionarySetValue(element, CFSTR(kKeywordCommand),
            CFSTR(kPredNamePrint));
        CFDictionarySetValue(element, CFSTR(kOptNameNulTerminate),
            kCFBooleanTrue);
    }

    if (CFEqual(predicate, CFSTR(kPredNameEcho)) ||
        CFEqual(predicate, CFSTR(kPredNamePrintProperty)) ||
        CFEqual(predicate, CFSTR(kPredNamePrintMatchProperty))) {
        if (!parseArgument(element, &argv[index], &index, user_data, error)) {
            goto finish;
        }
    }
    context->commandSpecified = true;

    result = true;
finish:
    *num_used += index;
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
char terminatorForElement(CFDictionaryRef element)
{
    if (CFDictionaryGetValue(element, CFSTR(kOptNameNulTerminate))) {
        return '\0';
    }
    return '\n';
}

/*******************************************************************************
*
*******************************************************************************/
Boolean evalCommand(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error)
{
    OSKextRef theKext = (OSKextRef)object;
    QueryContext * context = (QueryContext *)user_data;
    CFStringRef command = CFDictionaryGetValue(element, CFSTR(kKeywordCommand));
    CFArrayRef arguments = NULL;
    CFStringRef arg = NULL;
    char * string = NULL;  // must free

    if (CFEqual(command, CFSTR(kPredNameEcho))) {


        arguments = QEQueryElementGetArguments(element);
        arg = CFArrayGetValueAtIndex(arguments, 0);
        string = createUTF8CStringForCFString(arg);
        printf("%s", string);
        if (!CFDictionaryGetValue(element, CFSTR(kPredOptNameNoNewline))) {
            printf("%c", terminatorForElement(element));
        }
        goto finish;
    } else if (CFEqual(command, CFSTR(kPredNamePrint))) {
        printKext(theKext, context->pathSpec, context->extraInfo,
            terminatorForElement(element));
    } else if (CFEqual(command, CFSTR(kPredNameBundleName))) {
        printKext(theKext, kPathsNone, context->extraInfo,
            terminatorForElement(element));
    } else if (CFEqual(command, CFSTR(kPredNamePrintDiagnostics))) {
        g_log_stream = stdout;
        OSKextLogDiagnostics(theKext, kOSKextDiagnosticsFlagAll);
        g_log_stream = stderr;
    } else if (CFEqual(command, CFSTR(kPredNamePrintProperty))) {
        arguments = QEQueryElementGetArguments(element);
        arg = CFArrayGetValueAtIndex(arguments, 0);
        printKextProperty(theKext, arg, terminatorForElement(element));
    } else if (CFEqual(command, CFSTR(kPredNamePrintMatchProperty))) {
        arguments = QEQueryElementGetArguments(element);
        arg = CFArrayGetValueAtIndex(arguments, 1);
        printKextMatchProperty(theKext, arg, terminatorForElement(element));
    } else if (CFEqual(command, CFSTR(kPredNamePrintArches))) {
        printKextArches(theKext, terminatorForElement(element), true);
    } else if (CFEqual(command, CFSTR(kPredNamePrintDependencies))) {
        printKextDependencies(theKext, context->pathSpec,
             context->extraInfo, terminatorForElement(element));
    } else if (CFEqual(command, CFSTR(kPredNamePrintDependents))) {
        printKextDependents(theKext, context->pathSpec,
            context->extraInfo, terminatorForElement(element));
    } else if (CFEqual(command, CFSTR(kPredNamePrintPlugins))) {
        printKextPlugins(theKext, context->pathSpec, context->extraInfo,
            terminatorForElement(element));
    } else if (CFEqual(command, CFSTR(kPredNamePrintIntegrity))) {
       /* Kext integrity is no longer used on SnowLeopard.
        */
        printf("%s%c", "n/a", terminatorForElement(element));
    } else if (CFEqual(command, CFSTR(kPredNamePrintInfoDictionary))) {
        printKextInfoDictionary(theKext, context->pathSpec,
            terminatorForElement(element));
    } else if (CFEqual(command, CFSTR(kPredNamePrintExecutable))) {
        printKextExecutable(theKext, context->pathSpec,
            terminatorForElement(element));
    } else {
        *error = kQEQueryErrorEvaluationCallbackFailed;
        goto finish;
    }

finish:
    if (string) free(string);
    return true;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parseExec(
    CFMutableDictionaryRef element,
    int argc __unused,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    QueryContext * context = (QueryContext *)user_data;
    uint32_t index = 1;  // don't care about predicate
    CFStringRef arg = NULL;    // must release

    context->commandSpecified = true;

    while (argv[index]) {
        if (!strcmp(argv[index], kExecTerminator)) {
            result = true;
            index++;
            goto finish;
        }
        SAFE_RELEASE_NULL(arg);
        arg = CFStringCreateWithCString(kCFAllocatorDefault,
            argv[index], kCFStringEncodingUTF8);

        if (!arg) {
            *error = kQEQueryErrorNoMemory;
            goto finish;
        }
        QEQueryElementAppendArgument(element, arg);
        index++;
    }

    OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "No terminating ; for %s.", kPredNameExec);
    *error = kQEQueryErrorInvalidOrMissingArgument;
finish:
    SAFE_RELEASE(arg);
    *num_used += index;
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean evalExec(
    CFDictionaryRef element,
    void * object,
    void * user_data __unused,
    QEQueryError * error)
{
    Boolean result = false;
    pid_t pid;
    int status;
    OSKextRef          theKext          = (OSKextRef)object;
    CFURLRef           kextURL          = NULL;  // do not release
    CFStringRef        kextPath         = NULL;  // must release
    CFBundleRef        kextBundle       = NULL;  // must release
    CFURLRef           infoDictURL      = NULL;  // must release
    CFStringRef        infoDictPath     = NULL;  // must release
    CFURLRef           executableURL    = NULL;  // must release
    CFStringRef        executablePath   = NULL;  // must release
    char            ** command_argv     = NULL;  // must free each, and whole
    CFArrayRef         arguments        = QEQueryElementGetArguments(element);
    CFMutableStringRef scratch          = NULL;  // must release
    char               kextPathBuffer[PATH_MAX];
    CFIndex            count, i;

    *error = kQEQueryErrorEvaluationCallbackFailed;

    if (!arguments) {
        goto finish;
    }

    count = CFArrayGetCount(arguments);

    kextURL = OSKextGetURL(theKext);
    if (!kextURL) {
        OSKextLog(theKext,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Kext has no URL!");
        *error = kQEQueryErrorUnspecified;
        goto finish;
    }
    if (!CFURLGetFileSystemRepresentation(kextURL, /* resolveToBase */ false,
        (UInt8 *)kextPathBuffer, sizeof(kextPathBuffer))) {

        OSKextLogStringError(theKext);
        *error = kQEQueryErrorUnspecified;
        goto finish;
    }
    kextPath = CFURLCopyFileSystemPath(kextURL, kCFURLPOSIXPathStyle);
    if (!kextPath) {
        OSKextLogMemError();
        *error = kQEQueryErrorNoMemory;
        goto finish;
    }
    kextBundle = CFBundleCreate(kCFAllocatorDefault, kextURL);
    if (!kextBundle) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
            "Can't create bundle for %s.", kextPathBuffer);
        *error = kQEQueryErrorNoMemory;
        goto finish;
    }

    infoDictURL = _CFBundleCopyInfoPlistURL(kextBundle);
    if (infoDictURL) {
        infoDictPath = CFURLCopyFileSystemPath(infoDictURL, kCFURLPOSIXPathStyle);
    }
    executableURL = CFBundleCopyExecutableURL(kextBundle);
    if (executableURL) {
        executablePath = CFURLCopyFileSystemPath(executableURL, kCFURLPOSIXPathStyle);
    }

   /* Not having these two is an error. The executable may not exist.
    */
    if (!kextPath || !infoDictPath) {
        goto finish;
    }

    command_argv = (char **)malloc((1 + count) * sizeof(char *));
    if (!command_argv) {
        goto finish;
    }
    bzero(&command_argv, sizeof((1 + count) * sizeof(char *)));
    for (i = 0; i < count; i++) {
        scratch = CFStringCreateMutableCopy(kCFAllocatorDefault,
            0, CFArrayGetValueAtIndex(arguments, i));
        if (!scratch) {
            goto finish;
        }

        CFStringFindAndReplace(scratch, CFSTR(kExecInfoDictionaryReplace),
            infoDictPath, CFRangeMake(0, CFStringGetLength(scratch)), 0);

        if (executablePath) {
            CFStringFindAndReplace(scratch, CFSTR(kExecExecutableReplace),
            executablePath, CFRangeMake(0, CFStringGetLength(scratch)), 0);
        }

        CFStringFindAndReplace(scratch, CFSTR(kExecBundlePathReplace),
            kextPath, CFRangeMake(0, CFStringGetLength(scratch)), 0);


        command_argv[i] = createUTF8CStringForCFString(scratch);

        if (!command_argv[i]) {
            goto finish;
        }
        CFRelease(scratch);
        scratch = NULL;
    }

    command_argv[i] = NULL;

    pid = fork();
    switch (pid) {
      case 0:  // child
        execvp(command_argv[0], command_argv);
        break;
      case -1: // error
        perror("error forking for -exec");
        goto finish;
        break;
      default: // parent
        waitpid(-1, &status, 0);
        if (WIFEXITED(status)) {
            // Zero exit status is true
            result = WEXITSTATUS(status) ? false : true;
        }

        break;
    }

    *error = kQEQueryErrorNone;

finish:
    SAFE_RELEASE(scratch);
    SAFE_RELEASE(kextPath);
    SAFE_RELEASE(kextBundle);
    SAFE_RELEASE(infoDictURL);
    SAFE_RELEASE(infoDictPath);
    SAFE_RELEASE(executableURL);
    SAFE_RELEASE(executablePath);

    if (command_argv) {
        char ** arg = command_argv;
        while (*arg) {
            free(*arg);
            arg++;
        }
        free(command_argv);
    }

    return result;
}

/*******************************************************************************
*
*******************************************************************************/
static int last_optind;

void reset_getopt(void)
{
    optreset = 1;
    opterr = 0;
    optind = 1;
    last_optind = optind;

    return;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
parseStringElementOptions(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * index,
    QueryContext * context,
    QEQueryError * error)
{
    Boolean result = true;  // have to assume success until we hit a bad one
    int opt_char;

    reset_getopt();
    while ((opt_char = getopt_long_only(argc, argv, PROPERTY_OPTS,
        property_opt_info, NULL)) != -1) {

        switch (opt_char) {

          case kPredOptCaseInsensitive:
            CFDictionarySetValue(element, CFSTR(kSearchStyleCaseInsensitive),
                kCFBooleanTrue);
            last_optind = optind; // save optind for a subsequent mismatch
            break;

          case kPredOptSubstring:
            CFDictionarySetValue(element, CFSTR(kSearchStyleSubstring),
                kCFBooleanTrue);
            last_optind = optind; // save optind for a subsequent mismatch
            break;

          case 0:
            /* Fall through. */
          default:
            result = handleNonOption(opt_char, argv, context, error);
            goto finish;
            break;
        }
    }

finish:
    if (index) {
        *index = optind;  // we set rather than adding cause of getopt
    }
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean parsePrintOptions(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * index,
    QueryContext * context,
    QEQueryError * error)
{
    Boolean result = true;  // have to assume success until we hit a bad one
    int opt_char;

    reset_getopt();
    while ((opt_char = getopt_long_only(argc, argv, PRINT_OPTS,
        print_opt_info, NULL)) != -1) {

        switch (opt_char) {

          case kOptNulTerminate:
            CFDictionarySetValue(element, CFSTR(kOptNameNulTerminate),
                kCFBooleanTrue);
            last_optind = optind;
            break;

          case 0:
           /* -print takes no arguments so if we see a query predicate we
            * are good to go.
            */
            if (longopt == kLongOptQueryPredicate) {
                optind = last_optind;
                goto finish;
            }
          default:
            result = handleNonOption(opt_char, argv, context, error);
            goto finish;
            break;
        }
    }
finish:
    if (index) {
        *index = optind;  // we set rather than adding cause of getopt
    }
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
parseEchoOptions(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * index,
    QueryContext * context,
    QEQueryError * error)
{
    Boolean result = true;  // have to assume success until we hit a bad one
    int opt_char;

    reset_getopt();
    while ((opt_char = getopt_long_only(argc, argv, ECHO_OPTS,
        echo_opt_info, NULL)) != -1) {

        switch (opt_char) {

          case kPredOptNoNewline:
            CFDictionarySetValue(element, CFSTR(kPredOptNameNoNewline),
                kCFBooleanTrue);
            last_optind = optind; // save optind for a subsequent mismatch
            break;

          case kOptNulTerminate:
            CFDictionarySetValue(element, CFSTR(kOptNameNulTerminate),
                kCFBooleanTrue);
            last_optind = optind;
            break;

          case 0:
            /* Fall through. */
          default:
            result = handleNonOption(opt_char, argv, context, error);
            goto finish;
            break;
        }
    }
finish:
    if (index) {
        *index = optind;  // we set rather than adding cause of getopt
    }
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
#define QUIBBLE_PRED "%s looks like a predicate, " \
                     "but is being used as a property flag argument."
#define QUIBBLE_OPT  "%s looks like an (illegal) option, " \
                     "but is being used as a property flag argument."
#define PICKY_PRED   "%s is a predicate keyword (use -- to end options " \
                     "before arguments starting with a hyphen)."
#define PICKY_OPT    "Invalid option %s for %s (use -- to end options " \
                     "before arguments starting with a hyphen)."

Boolean
handleNonOption(int opt_char,
    char * const argv[],
    QueryContext * context,
    QEQueryError * error)
{
    Boolean result = false;

   /* getopt_long's lookahead is SO NOT THE RIGHT BEHAVIOR.
    * Nor is its handling of empty arguments.
    */
    if ( (argv[last_optind][0] != '-') || !argv[last_optind][0]) {
        optind = last_optind;
        result = true;
        goto finish;
    }

    if (opt_char) {
        if (context->assertiveness == kKextfindQuibbling) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                QUIBBLE_OPT, argv[last_optind]);
            optind = last_optind;
        } else if (context->assertiveness == kKextfindPicky) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                PICKY_OPT, argv[last_optind], argv[0]);
            *error = kQEQueryErrorInvalidOrMissingArgument;
            goto finish;
        }
        optind = last_optind;
    } else {
        switch (longopt) {
          case kLongOptQueryPredicate:
            if (context->assertiveness == kKextfindQuibbling) {
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    QUIBBLE_PRED, argv[last_optind]);
                optind = last_optind;
            } else if (context->assertiveness == kKextfindPicky) {
                OSKextLog(/* kext */ NULL,
                    kOSKextLogErrorLevel | kOSKextLogGeneralFlag,
                    PICKY_PRED, argv[last_optind]);
                *error = kQEQueryErrorInvalidOrMissingArgument;
                goto finish;
            }
            optind = last_optind;
            break;

          default:  // should never see this
            *error = kQEQueryErrorParseCallbackFailed;
            optind = last_optind;
            goto finish;
            break;
        }
    }
    result = true;
finish:
    return result;
}
