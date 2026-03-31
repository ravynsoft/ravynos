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
#include "kextfind_main.h"
#include "kextfind_report.h"
#include "kextfind_query.h"
#include "kextfind_commands.h"
#include "kext_tools_util.h"

#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>
#include <IOKit/kext/fat_util.h>
#include <IOKit/kext/macho_util.h>


/*******************************************************************************
*
*******************************************************************************/
Boolean reportParseProperty(
    CFMutableDictionaryRef element,
    int argc __unused,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    uint32_t index        = 1;

   /* Fudge the predicate so we can use one eval callback.
    */
    QEQueryElementSetPredicate(element, CFSTR(kPredNameProperty));

   /* Parse the property name to retrieve.
   */
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
Boolean reportParseShorthand(
    CFMutableDictionaryRef element,
    int argc __unused,
    char * const argv[] __unused,
    uint32_t * num_used,
    void * user_data __unused,
    QEQueryError * error __unused)
{
    Boolean result = false;
    CFStringRef predicate = QEQueryElementGetPredicate(element);
    uint32_t index = 1;

    if (CFEqual(predicate, CFSTR(kPredNameBundleID))) {
        QEQueryElementAppendArgument(element, kCFBundleIdentifierKey);
    } else if (CFEqual(predicate, CFSTR(kPredNameBundleName))) {
        QEQueryElementAppendArgument(element, kCFBundleNameKey);
    } else if (CFEqual(predicate, CFSTR(kPredNameVersion))) {
        QEQueryElementAppendArgument(element, kCFBundleVersionKey);
    } else {
        goto finish;
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
char * cStringForCFValue(CFTypeRef value)
{
    CFTypeID valueType;
    CFIndex count;
    char buffer[80];  // more than big enough for a number

    if (!value) {
        return strdup("<null>");
    }

    valueType = CFGetTypeID(value);

    if (CFStringGetTypeID() == valueType) {
        return createUTF8CStringForCFString(value);
    } else if (CFBooleanGetTypeID() == valueType) {
        return CFBooleanGetValue(value) ? strdup(kWordTrue) : strdup(kWordFalse);
    } else if (CFNumberGetTypeID() == valueType) {
    } else if (CFArrayGetTypeID() == valueType) {
        count = CFArrayGetCount(value);
        snprintf(buffer, (sizeof(buffer)/sizeof(char)), "<array of %ld>", count);
        return strdup(buffer);
    } else if (CFDictionaryGetTypeID() == valueType) {
        count = CFDictionaryGetCount(value);
        snprintf(buffer, (sizeof(buffer)/sizeof(char)), "<dict of %ld>", count);
        return strdup(buffer);
    } else if (CFDataGetTypeID() == valueType) {
        count = CFDataGetLength(value);
        snprintf(buffer, (sizeof(buffer)/sizeof(char)), "<data of %ld>", count);
        return strdup(buffer);
    } else {
        return strdup("<unknown CF type>");
    }
    return NULL;
}

/*******************************************************************************
* Note: reportEvalCommand() calls this.
*******************************************************************************/
Boolean reportEvalProperty(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    OSKextRef theKext = (OSKextRef)object;
    QueryContext * context = (QueryContext *)user_data;
    CFStringRef propKey = NULL;   // don't release
    CFTypeRef   propVal = NULL;   // don't release
    char *      cString = NULL;   // must free

    propKey = QEQueryElementGetArgumentAtIndex(element, 0);
    if (!propKey) {
        *error = kQEQueryErrorEvaluationCallbackFailed;
        goto finish;
    }

    if (!context->reportStarted) {
        cString = createUTF8CStringForCFString(propKey);
        if (!cString) {
            *error = kQEQueryErrorEvaluationCallbackFailed;
            goto finish;
        }
        printf("%s%s", context->reportRowStarted ? "\t" : "",
            cString);
    } else {
        // This is allowed to be null
        propVal = OSKextGetValueForInfoDictionaryKey(theKext, propKey);
        cString = cStringForCFValue(propVal);
        if (!cString) {
            *error = kQEQueryErrorEvaluationCallbackFailed;
            goto finish;
        }
        printf("%s%s", context->reportRowStarted ? "\t" : "",
            cString);
    }

    context->reportRowStarted = true;

    result = true;
finish:
    if (cString) free(cString);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean reportParseFlag(
    CFMutableDictionaryRef element,
    int argc __unused,
    char * const argv[] __unused,
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error __unused)
{
    Boolean result = false;
    CFStringRef flag = QEQueryElementGetPredicate(element);
    QueryContext * context = (QueryContext *)user_data;
    uint32_t index = 1;

    QEQueryElementSetPredicate(element, CFSTR(kPredNameFlag));
    CFDictionarySetValue(element, CFSTR(kKeywordFlag), flag);

    if (CFEqual(flag, CFSTR(kPredNameLoaded))) {
        context->checkLoaded = true;
    } else if (CFEqual(flag, CFSTR(kPredNameIntegrity))) {
       /* Kext integrity is no longer used on SnowLeopard. We read the
        * flags but no kext will ever match them now.
        */
        context->checkIntegrity = true;
    }

    result = true;

    *num_used += index;
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean reportEvalFlag(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    OSKextRef theKext = (OSKextRef)object;
    CFStringRef flag = CFDictionaryGetValue(element, CFSTR(kKeywordFlag));
    QueryContext * context = (QueryContext *)user_data;
    char *         cString = NULL;   // don't free!
    Boolean print = true;

    if (!context->reportStarted) {
        if (CFEqual(flag, CFSTR(kPredNameLoaded))) {
            cString = "Loaded";
        } else if (CFEqual(flag, CFSTR(kPredNameValid))) {
            cString = "Valid";
        } else if (CFEqual(flag, CFSTR(kPredNameAuthentic))) {
            cString = "Authentic";
        } else if (CFEqual(flag, CFSTR(kPredNameDependenciesMet))) {
            cString = "Dependencies Met";
        } else if (CFEqual(flag, CFSTR(kPredNameLoadable))) {
            cString = "Loadable";
        } else if (CFEqual(flag, CFSTR(kPredNameWarnings))) {
            cString = "Warnings";
        } else if (CFEqual(flag, CFSTR(kPredNameIsLibrary))) {
            cString = "Library";
        } else if (CFEqual(flag, CFSTR(kPredNameHasPlugins))) {
            cString = "Plugins";
        } else if (CFEqual(flag, CFSTR(kPredNameIsPlugin))) {
            cString = "Is Plugin";
        } else if (CFEqual(flag, CFSTR(kPredNameHasDebugProperties))) {
            cString = "Debug";
        } else if (CFEqual(flag, CFSTR(kPredNameIsKernelResource))) {
            cString = "Kernel Resource";
        } else if (CFEqual(flag, CFSTR(kPredNameIntegrity))) {
            cString = "Integrity";
        } else if (CFEqual(flag, CFSTR(kPredNameExecutable))) {
            cString = "Has Executable";
        } else if (CFEqual(flag, CFSTR(kPredNameDuplicate))) {
            cString = "Has Duplicates";
        } else {
            *error = kQEQueryErrorEvaluationCallbackFailed;
            goto finish;
        }

        printf("%s%s", context->reportRowStarted ? "\t" : "", cString);
    } else {

        if (CFEqual(flag, CFSTR(kPredNameLoaded))) {
            cString = OSKextIsLoaded(theKext) ? kWordYes : kWordNo;
        } else if (CFEqual(flag, CFSTR(kPredNameValid))) {
            cString = OSKextIsValid(theKext) ? kWordYes : kWordNo;
        } else if (CFEqual(flag, CFSTR(kPredNameAuthentic))) {
            cString = OSKextIsAuthentic(theKext) ? kWordYes : kWordNo;
        } else if (CFEqual(flag, CFSTR(kPredNameDependenciesMet))) {
            cString = OSKextResolveDependencies(theKext) ? kWordYes : kWordNo;
        } else if (CFEqual(flag, CFSTR(kPredNameLoadable))) {
            cString = OSKextIsLoadable(theKext) ?
                kWordYes : kWordNo;
        } else if (CFEqual(flag, CFSTR(kPredNameWarnings))) {
            CFDictionaryRef warnings = OSKextCopyDiagnostics(theKext,
                kOSKextDiagnosticsFlagWarnings);
            cString = (warnings && CFDictionaryGetCount(warnings)) ?
                kWordYes : kWordNo;
            SAFE_RELEASE(warnings);
        } else if (CFEqual(flag, CFSTR(kPredNameIsLibrary))) {
            cString = (OSKextGetCompatibleVersion(theKext) > 0) ?
                kWordYes : kWordNo;
        } else if (CFEqual(flag, CFSTR(kPredNameHasPlugins))) {
            CFArrayRef plugins = OSKextCopyPlugins(theKext);
            cString = (plugins && CFArrayGetCount(plugins)) ?
                kWordYes : kWordNo;
                SAFE_RELEASE(plugins);
        } else if (CFEqual(flag, CFSTR(kPredNameIsPlugin))) {
            cString = OSKextIsPlugin(theKext) ? kWordYes : kWordNo;
        } else if (CFEqual(flag, CFSTR(kPredNameHasDebugProperties))) {
            cString = OSKextHasLogOrDebugFlags(theKext) ? kWordYes : kWordNo;
        } else if (CFEqual(flag, CFSTR(kPredNameIsKernelResource))) {
            cString = OSKextIsKernelComponent(theKext) ? kWordYes : kWordNo;
        } else if (CFEqual(flag, CFSTR(kPredNameIntegrity))) {
           /* Note: As of SnowLeopard, integrity is no longer used.
            */
            printf("%s%s", context->reportRowStarted ? "\t" : "",
                "n/a");
            print = false;
        } else if (CFEqual(flag, CFSTR(kPredNameExecutable))) {
            cString = OSKextDeclaresExecutable(theKext) ? kWordYes : kWordNo;
        } else if (CFEqual(flag, CFSTR(kPredNameDuplicate))) {
            CFStringRef kextIdentifier = OSKextGetIdentifier(theKext);
            if (kextIdentifier) {
                if (kextIdentifier) {
                    CFArrayRef kexts = OSKextCopyKextsWithIdentifier(kextIdentifier);
                    if (!kexts) {
                        OSKextLogMemError();
                        goto finish;
                    }
                    cString = (CFArrayGetCount(kexts) > 1) ? kWordYes : kWordNo;
                    SAFE_RELEASE(kexts);
                }
            }
        } else {
            *error = kQEQueryErrorEvaluationCallbackFailed;
            goto finish;
        }

        if (print) {
            if (!cString) {
                *error = kQEQueryErrorEvaluationCallbackFailed;
                goto finish;
            }
            printf("%s%s", context->reportRowStarted ? "\t" : "",
                cString);
        }
    }

    context->reportRowStarted = true;

    result = true;
finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean reportParseArch(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    CFStringRef scratchString = NULL;

    if (!argv[(*num_used) + 1]) {
        goto finish;
    }

    scratchString = CFStringCreateWithCString(kCFAllocatorDefault,
        argv[(*num_used) + 1], kCFStringEncodingUTF8);

    CFDictionarySetValue(element, CFSTR("label"), scratchString);

    result = parseArch(element, argc, argv, num_used, user_data, error);
    if (!result) {
        goto finish;
    }

    result = true;
finish:
    SAFE_RELEASE(scratchString);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean reportEvalArch(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    QueryContext * context = (QueryContext *)user_data;
    CFStringRef string = NULL;   // don't release
    char *      cString = NULL;   // must free


    if (!context->reportStarted) {
        string = CFDictionaryGetValue(element, CFSTR("label"));
        if (!string) {
            *error = kQEQueryErrorEvaluationCallbackFailed;
            goto finish;
        }
        cString = createUTF8CStringForCFString(string);
        if (!cString) {
            *error = kQEQueryErrorEvaluationCallbackFailed;
            goto finish;
        }
        printf("%s%s", context->reportRowStarted ? "\t" : "",
            cString);
    } else {
        Boolean match = evalArch(element, object, user_data, error);
        if (*error != kQEQueryErrorNone) {
            goto finish;
        }
        printf("%s%s", context->reportRowStarted ? "\t" : "",
            match ? kWordYes : kWordNo);
    }

    context->reportRowStarted = true;

    result = true;
finish:
    if (cString) free(cString);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean reportEvalArchExact(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    QueryContext * context = (QueryContext *)user_data;
    CFStringRef string = NULL;   // don't release
    char *      cString = NULL;   // must free


    if (!context->reportStarted) {
        string = CFDictionaryGetValue(element, CFSTR("label"));
        if (!string) {
            *error = kQEQueryErrorEvaluationCallbackFailed;
            goto finish;
        }
        cString = createUTF8CStringForCFString(string);
        if (!cString) {
            *error = kQEQueryErrorEvaluationCallbackFailed;
            goto finish;
        }
        printf("%s%s (only)", context->reportRowStarted ? "\t" : "",
            cString);
    } else {
        Boolean match = evalArchExact(element, object, user_data, error);
        if (*error != kQEQueryErrorNone) {
            goto finish;
        }
        printf("%s%s", context->reportRowStarted ? "\t" : "",
            match ? kWordYes : kWordNo);
    }

    context->reportRowStarted = true;

    result = true;
finish:
    if (cString) free(cString);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean reportParseDefinesOrReferencesSymbol(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    return parseDefinesOrReferencesSymbol(element, argc, argv, num_used,
        user_data, error);
}

/*******************************************************************************
* xxx - if arches were specified on the command line, this should perhaps only
* xxx - check those arches
*******************************************************************************/
Boolean reportEvalDefinesOrReferencesSymbol(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    OSKextRef  theKext = (OSKextRef)object;
    QueryContext * context = (QueryContext *)user_data;
    CFStringRef symbol = QEQueryElementGetArgumentAtIndex(element, 0);
    char * cSymbol = NULL;   // must free
    const char * value = "";  // don't free
    fat_iterator fiter = NULL;  // must close
    struct mach_header * farch = NULL;
    void * farch_end = NULL;
    uint8_t nlist_type;

    if (!symbol) {
        *error = kQEQueryErrorEvaluationCallbackFailed;
        goto finish;
    }
    cSymbol = createUTF8CStringForCFString(symbol);
    if (!cSymbol) {
        *error = kQEQueryErrorEvaluationCallbackFailed;
        goto finish;
    }

    if (!context->reportStarted) {
        printf("%ssymbol %s", context->reportRowStarted ? "\t" : "",
            cSymbol);
    } else {

        fiter = createFatIteratorForKext(theKext);
        if (!fiter) {
            goto finish;
        }

        while ((farch = fat_iterator_next_arch(fiter, &farch_end))) {
            macho_seek_result seek_result = macho_find_symbol(
                farch, farch_end, cSymbol, &nlist_type, NULL);

            if (seek_result == macho_seek_result_found_no_value ||
                seek_result == macho_seek_result_found) {

                if ((N_TYPE & nlist_type) == N_UNDF) {
                    value = OSKextIsKernelComponent(theKext) ?
                        "defines" : "references";
                } else {
                    value = "defines";
                }
                break;
            }
        }
    }

    printf("%s%s", context->reportRowStarted ? "\t" : "", value);
    context->reportRowStarted = true;

    result = true;
finish:
    if (cSymbol) free(cSymbol);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean reportParseCommand(
    CFMutableDictionaryRef element,
    int argc __unused,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error)
{
    Boolean result = false;
    QueryContext * context = (QueryContext *)user_data;
    CFStringRef command = QEQueryElementGetPredicate(element);
    uint32_t index = 1;

    if (CFEqual(command, CFSTR(kPredNamePrintProperty))) {

       /* Fudge the predicate so we can use one eval callback.
        */
        QEQueryElementSetPredicate(element, CFSTR(kPredNameProperty));
        if (!parseArgument(element, &argv[index], &index, user_data, error)) {
            goto finish;
        }
    } else if (CFEqual(command, CFSTR(kPredNamePrintIntegrity))) {
       /* Kext integrity is no longer used on SnowLeopard. We read the
        * flags but no kext will ever match them now.
        */
        context->checkIntegrity = true;
    }

    CFDictionarySetValue(element, CFSTR(kKeywordCommand), command);
    QEQueryElementSetPredicate(element, CFSTR(kPredNameCommand));

    result = true;
finish:
    *num_used += index;
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean reportEvalCommand(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error)
{
    Boolean        result        = false;
    CFStringRef    command       = CFDictionaryGetValue(element, CFSTR(kKeywordCommand));
    OSKextRef      theKext       = (OSKextRef)object;
    QueryContext * context       = (QueryContext *)user_data;
    CFStringRef    scratchString = NULL;  // must release
    char         * cString       = NULL;  // must free

    // if we do arches, easier to print than generate a string
    Boolean        print         = true;

    CFArrayRef     dependencies  = NULL;  // must release
    CFArrayRef     dependents    = NULL;  // must release
    CFArrayRef     plugins       = NULL;  // do NOT release
    CFIndex        count;
    char           buffer[80];   // more than enough for an int

    if (!context->reportStarted) {
        if (CFEqual(command, CFSTR(kPredNamePrint)) ||
            CFEqual(command, CFSTR(kPredNameBundleName))) {
            cString = strdup("Bundle");
        } else if (CFEqual(command, CFSTR(kPredNamePrintProperty))) {
            result = reportEvalProperty(element, object, user_data, error);
            goto finish;
        } else if (CFEqual(command, CFSTR(kPredNamePrintArches))) {
            cString = strdup("Arches");
        } else if (CFEqual(command, CFSTR(kPredNamePrintDependencies))) {
            cString = strdup("# Dependencies");
        } else if (CFEqual(command, CFSTR(kPredNamePrintDependents))) {
            cString = strdup("# Dependents");
        } else if (CFEqual(command, CFSTR(kPredNamePrintPlugins))) {
            cString = strdup("# Plugins");
        } else if (CFEqual(command, CFSTR(kPredNamePrintIntegrity))) {
            cString = strdup("Integrity");
        } else if (CFEqual(command, CFSTR(kPredNamePrintInfoDictionary))) {
            cString = strdup("Info Dictionary");
        } else if (CFEqual(command, CFSTR(kPredNamePrintExecutable))) {
            cString = strdup("Executable");
        } else {
            *error = kQEQueryErrorEvaluationCallbackFailed;
            goto finish;
        }

        printf("%s%s", context->reportRowStarted ? "\t" : "", cString);
    } else {
        if (CFEqual(command, CFSTR(kPredNamePrint))) {
            scratchString = copyPathForKext(theKext, context->pathSpec);
            if (!scratchString) {
                OSKextLogMemError();
                goto finish;
            }
            cString = createUTF8CStringForCFString(scratchString);
        } else if (CFEqual(command, CFSTR(kPredNameBundleName))) {
            scratchString = copyPathForKext(theKext, kPathsNone);
            if (!scratchString) {
                OSKextLogMemError();
                goto finish;
            }
            cString = createUTF8CStringForCFString(scratchString);
        } else if (CFEqual(command, CFSTR(kPredNamePrintProperty))) {
            result = reportEvalProperty(element, object, user_data, error);
            goto finish;
        } else if (CFEqual(command, CFSTR(kPredNamePrintArches))) {
            printf("%s", context->reportRowStarted ? "\t" : "");
            printKextArches(theKext, 0, false /* print line end */);
            print = false;
        } else if (CFEqual(command, CFSTR(kPredNamePrintDependencies))) {
            dependencies = OSKextCopyAllDependencies(theKext,
                /* needAll? */ false);
            count = dependencies ? CFArrayGetCount(dependencies) : 0;
            snprintf(buffer, (sizeof(buffer)/sizeof(char)), "%ld", count);
            cString = strdup(buffer);
        } else if (CFEqual(command, CFSTR(kPredNamePrintDependents))) {
            dependencies = OSKextCopyDependents(theKext, /* direct? */ false);
            count = dependencies ? CFArrayGetCount(dependencies) : 0;
            snprintf(buffer, (sizeof(buffer)/sizeof(char)), "%ld", count);
            cString = strdup(buffer);
        } else if (CFEqual(command, CFSTR(kPredNamePrintPlugins))) {
            plugins = OSKextCopyPlugins(theKext);
            count = plugins ? CFArrayGetCount(plugins) : 0;
            snprintf(buffer, (sizeof(buffer)/sizeof(char)), "%ld", count);
            cString = strdup(buffer);
            SAFE_RELEASE(plugins);
        } else if (CFEqual(command, CFSTR(kPredNamePrintIntegrity))) {
           /* Note: As of SnowLeopard, integrity is no longer used.
            */
            printf("%s%s", context->reportRowStarted ? "\t" : "",
                "n/a");
            print = false;
        } else if (CFEqual(command, CFSTR(kPredNamePrintInfoDictionary))) {
            scratchString = copyKextInfoDictionaryPath(theKext, context->pathSpec);
            if (!scratchString) {
                OSKextLogMemError();
                goto finish;
            }
            cString = createUTF8CStringForCFString(scratchString);
        } else if (CFEqual(command, CFSTR(kPredNamePrintExecutable))) {
            scratchString = copyKextExecutablePath(theKext, context->pathSpec);
            if (!scratchString) {
                OSKextLogMemError();
                goto finish;
            }
            cString = createUTF8CStringForCFString(scratchString);
        } else {
            *error = kQEQueryErrorEvaluationCallbackFailed;
            goto finish;
        }

        if (print) {
            if (!cString) {
                *error = kQEQueryErrorEvaluationCallbackFailed;
                goto finish;
            }
            printf("%s%s", context->reportRowStarted ? "\t" : "",
                cString);
        }
    }

    context->reportRowStarted = true;
    result = true;
finish:
    SAFE_RELEASE(scratchString);
    SAFE_FREE(cString);
    SAFE_RELEASE(dependencies);
    SAFE_RELEASE(dependents);
    return result;
}
