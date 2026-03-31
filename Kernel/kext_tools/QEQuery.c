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
#include "QEQuery.h"
#include "kext_tools_util.h"

/*******************************************************************************
* The internal query engine object definition.
*******************************************************************************/
struct __QEQuery {

   /* queryRoot: the outermost query element, always an OR group
    */
    CFMutableDictionaryRef     queryRoot;

   /* queryStack, queryStackTop: a lifo stack of query elements representing
    * open groups (the root is always considered open for appending).
    */
    CFMutableArrayRef          queryStack;
    CFMutableDictionaryRef     queryStackTop;

   /* The number of explicit OR groups that are open.
    */
    uint32_t                   nestingLevel;

   /* Is an AND or OR operator pending?
    */
    Boolean                    logicOpActive;

   /* End evaluation as soon as truth value known? (on by default)
    */
    Boolean                    shortCircuitEval;

   /* The last error to happen with this query.
    */
    QEQueryError               lastError;

   /* Client-defined synonyms to their underlying operator equivalents.
    */
    CFMutableDictionaryRef     synonyms;

   /* Lookup operators by default token to client-defined token (which
    * may be the default).
    */
    CFMutableDictionaryRef     operators;

   /* parse, eval callbacks keyed by client-registered keyword.
    */
    CFMutableDictionaryRef     parseCallbacks;
    CFMutableDictionaryRef     evaluationCallbacks;

   /* Client-defined data passed to all callbacks.
    */
    void *                     userData;
};

/*******************************************************************************
* Internal query element keys for the basic operators and such.
********************************************************************************
* IMPORTANT NOTE ABOUT GROUPS
* "Or" groups are always explicit, except for the query root. "And" groups are
* always implicit, and are created as soon as an "and" operator is seen. The
* query engine promotes the element preceding the "and" into the new "and"
* group. When an "or" group is closed, several checks are made to close implicit
* "and" groups as well as any trivial groups of one element.
* See QEQueryEndGroup() and particularly _QEQueryPopGroupAndCoalesce() for
* the gory details.
*******************************************************************************/
#define kQEQueryKeyPredicate    CFSTR("_QEQueryPredicate")
#define kQEQueryKeyArguments    CFSTR("_QEQueryArguments")

#define kQEQueryKeyNegated      CFSTR("_QEQueryNegated")

#define kQEQueryPredicateAnd    CFSTR("_QEQueryAndGroup")
#define kQEQueryPredicateOr     CFSTR("_QEQueryOrGroup")

#define kMsgStringConversionError  "string conversion error"

/*******************************************************************************
* Internal function prototypes.
*******************************************************************************/
Boolean _QEQueryStartGroup(
    QEQueryRef query,
    Boolean andFlag,
    Boolean negated);
CFMutableDictionaryRef _QEQueryCreateGroup(Boolean andFlag);
CF_RETURNS_RETAINED CFMutableDictionaryRef _QEQueryYankElement(QEQueryRef query);
Boolean _QEQueryStackTopIsAndGroup(QEQueryRef query);
Boolean _QEQueryStackTopIsOrGroup(QEQueryRef query);
Boolean _QEQueryStackTopHasCount(QEQueryRef query, CFIndex count);
Boolean _QEQueryPushGroup(QEQueryRef query, Boolean negated, Boolean andFlag);
Boolean _QEQueryPopGroup(QEQueryRef query);
Boolean _QEQueryPopGroupAndCoalesce(
    QEQueryRef query,
    Boolean onlyIfCanCoalesce);

CFStringRef _QEQueryOperatorForToken(
    QEQueryRef query,
    CFStringRef token);
CF_RETURNS_RETAINED CFStringRef _QEPredicateForString(
    QEQueryRef query,
    char * string);
QEQueryParseCallback _QEQueryParseCallbackForPredicate(
    QEQueryRef query,
    CFStringRef predicate);
QEQueryEvaluationCallback _QEQueryEvaluationCallbackForPredicate(
    QEQueryRef query,
    CFStringRef predicate);

Boolean _QEQueryElementIsNegated(CFDictionaryRef element);
void _QEQueryElementNegate(CFMutableDictionaryRef element);

#pragma mark Creation/Setup/Destruction

/*******************************************************************************
*
*******************************************************************************/
QEQueryRef
QEQueryCreate(void * userData)
{
    struct __QEQuery * result = NULL;   // returned
    Boolean ok = false;

    result = (QEQueryRef)malloc(sizeof(struct __QEQuery));
    if (!result) {
        goto finish;
    }

    bzero(result, sizeof(struct __QEQuery));

    result->userData = userData;

    result->queryRoot = _QEQueryCreateGroup(false /* andGroup */);
    if (!result->queryRoot) {
        goto finish;
    }

    result->queryStack = CFArrayCreateMutable(kCFAllocatorDefault,
        0, &kCFTypeArrayCallBacks);
    if (!result->queryStack) {
        goto finish;
    }

    result->parseCallbacks = CFDictionaryCreateMutable(kCFAllocatorDefault,
        0 /* no limit */, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!result->parseCallbacks) {
        goto finish;
    }

    result->operators = CFDictionaryCreateMutable(kCFAllocatorDefault,
        0 /* no limit */, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!result->operators) {
        goto finish;
    }

    result->synonyms = CFDictionaryCreateMutable(kCFAllocatorDefault,
        0 /* no limit */, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!result->synonyms) {
        goto finish;
    }

    result->evaluationCallbacks = CFDictionaryCreateMutable(kCFAllocatorDefault,
        0 /* no limit */, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!result->evaluationCallbacks) {
        goto finish;
    }

    CFArrayAppendValue(result->queryStack, result->queryRoot);
    result->queryStackTop = result->queryRoot;

    result->nestingLevel = 0;
    result->logicOpActive = false;
    result->shortCircuitEval = true;

    QEQuerySetOperators(result, NULL, NULL, NULL, NULL, NULL);

    ok = true;

finish:
    if (!ok && result) {
        QEQueryFree(result);
        result = NULL;
    }
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
void
QEQueryFree(QEQueryRef query)
{
    if (query->queryRoot)      CFRelease(query->queryRoot);
    if (query->queryStack)     CFRelease(query->queryStack);
    if (query->operators)      CFRelease(query->operators);
    if (query->parseCallbacks) CFRelease(query->parseCallbacks);
    if (query->evaluationCallbacks) CFRelease(query->evaluationCallbacks);
    if (query->synonyms)       CFRelease(query->synonyms);
    free(query);
    return;
}

/*******************************************************************************
*
*******************************************************************************/
void
QEQueryEmptyParseDictionaries(QEQueryRef query)
{
    CFDictionaryRemoveAllValues(query->parseCallbacks);
    CFDictionaryRemoveAllValues(query->evaluationCallbacks);
    CFDictionaryRemoveAllValues(query->synonyms);
    return;
}

/*******************************************************************************
* QEQuerySetOperators() -- all operators must have a definition, so if NULL is
* passed for any, the basic token defined in the header file is assigned.
*******************************************************************************/
void
QEQuerySetOperators(QEQueryRef query,
    CFStringRef andPredicate,
    CFStringRef orPredicate,
    CFStringRef notPredicate,
    CFStringRef groupStartPredicate,
    CFStringRef groupEndPredicate)
{
    CFDictionarySetValue(query->operators, CFSTR(kQEQueryTokenAnd),
        andPredicate ? andPredicate : CFSTR(kQEQueryTokenAnd));
    CFDictionarySetValue(query->operators, CFSTR(kQEQueryTokenOr),
        orPredicate ? orPredicate : CFSTR(kQEQueryTokenOr));
    CFDictionarySetValue(query->operators, CFSTR(kQEQueryTokenNot),
        notPredicate ? notPredicate : CFSTR(kQEQueryTokenNot));
    CFDictionarySetValue(query->operators, CFSTR(kQEQueryTokenGroupStart),
        groupStartPredicate ? groupStartPredicate : CFSTR(kQEQueryTokenGroupStart));
    CFDictionarySetValue(query->operators, CFSTR(kQEQueryTokenGroupEnd),
        groupEndPredicate ? groupEndPredicate : CFSTR(kQEQueryTokenGroupEnd));
    return;
}

/*******************************************************************************
*
*******************************************************************************/
void
QEQuerySetParseCallbackForPredicate(
    QEQueryRef query,
    CFStringRef predicate,
    QEQueryParseCallback parseCallback)
{
    CFDataRef pCallback = NULL;

    if (parseCallback) {
        pCallback = CFDataCreate(kCFAllocatorDefault,
            (void *)&parseCallback, sizeof(QEQueryParseCallback));
        if (!pCallback) {
            goto finish;
        }
        CFDictionarySetValue(query->parseCallbacks, predicate, pCallback);
    } else {
        CFDictionaryRemoveValue(query->parseCallbacks, predicate);
    }

finish:
    if (pCallback) CFRelease(pCallback);
    return;
}

/*******************************************************************************
*
*******************************************************************************/
void
QEQuerySetEvaluationCallbackForPredicate(
    QEQueryRef query,
    CFStringRef predicate,
    QEQueryEvaluationCallback evaluationCallback)
{
    CFDataRef eCallback = NULL;

    if (evaluationCallback) {
        eCallback = CFDataCreate(kCFAllocatorDefault,
            (void *)&evaluationCallback, sizeof(QEQueryEvaluationCallback));
        if (!eCallback) {
            goto finish;
        }
        CFDictionarySetValue(query->evaluationCallbacks, predicate, eCallback);
    } else {
        CFDictionaryRemoveValue(query->evaluationCallbacks, predicate);
    }

finish:
    if (eCallback) CFRelease(eCallback);
    return;
}

/*******************************************************************************
*
*******************************************************************************/
void
QEQuerySetSynonymForPredicate(
    QEQueryRef query,
    CFStringRef synonym,
    CFStringRef predicate)
{
    if (predicate) {
        CFDictionarySetValue(query->synonyms, synonym, predicate);
    } else {
        CFDictionaryRemoveValue(query->synonyms, synonym);
    }
    return;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
QEQueryIsComplete(QEQueryRef query)
{
    if (!query->nestingLevel  &&
        !query->logicOpActive &&
        query->lastError == kQEQueryErrorNone) {

        return true;
    }
    return false;
}

/*******************************************************************************
*
*******************************************************************************/
QEQueryError
QEQueryLastError(QEQueryRef query)
{
    return query->lastError;
}

#pragma mark Evaluation

/*******************************************************************************
*
*******************************************************************************/
void
QEQuerySetShortCircuits(
    QEQueryRef query,
    Boolean flag)
{
    query->shortCircuitEval = flag;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
QEQueryGetShortCircuits(QEQueryRef query)
{
    return query->shortCircuitEval;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
_QEQueryElementEvaluate(
    QEQueryRef query,
    CFDictionaryRef element,
    void * object)
{
    Boolean result = false;
    CFStringRef predicate = NULL;
    CFArrayRef elements = NULL;
    CFIndex count, i;

    predicate = CFDictionaryGetValue(element, kQEQueryKeyPredicate);


    if (CFEqual(predicate, kQEQueryPredicateAnd)) {
        elements = QEQueryElementGetArguments(element);

       /* Empty groups can't normally be created, except for the
        * root query, but empty groups are trivially true; basically,
        * an empty group is true.
        */
        count = CFArrayGetCount(elements);        if (!count) {
            result = true;
        }

       /* If any element in an AND group is false, stop evaluating the
        * group and return false, else return true.
        */
        for (i = 0; i < count; i++) {
            CFDictionaryRef thisElement = CFArrayGetValueAtIndex(elements, i);
            if (!_QEQueryElementEvaluate(query, thisElement, object)) {
                if (query->shortCircuitEval) {
                    goto finish;
                }
            }
        }
        result = true;
    } else if (CFEqual(predicate, kQEQueryPredicateOr)) {
        elements = QEQueryElementGetArguments(element);

       /* Empty groups can't normally be created, except for the
        * root query, but empty groups are trivially true; basically,
        * an empty group is true.
        */
        count = CFArrayGetCount(elements);
        if (!count) {
            result = true;
        }

       /* If any element in an OR group is true, stop evaluating the
        * group and return true, else return false.
        */
        for (i = 0; i < count; i++) {
            CFDictionaryRef thisElement = CFArrayGetValueAtIndex(elements, i);
            if (_QEQueryElementEvaluate(query, thisElement, object)) {
                result = true;
                if (query->shortCircuitEval) {
                    goto finish;
                }
            }
        }
    } else {

       /* Look for a client callback.
        */
        QEQueryEvaluationCallback evalCallback =
            _QEQueryEvaluationCallbackForPredicate(query, predicate);
        if (evalCallback) {
            result = evalCallback(element, object, query->userData,
                &query->lastError);
        } else {
            query->lastError = kQEQueryErrorNoEvaluationCallback;
        }
    }

finish:

   /* Flip the result if the element is negated.
     */
    if (_QEQueryElementIsNegated(element)) {
        result = !result;
    }

   /* Set the result to false upon any error.
    */
    if (query->lastError != kQEQueryErrorNone) {
        result = false;
    }
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
QEQueryEvaluate(
    QEQueryRef query,
    void * object)
{
    Boolean result = false;
    if (!QEQueryIsComplete(query) || query->lastError != kQEQueryErrorNone) {
        goto finish;
    }
    result = _QEQueryElementEvaluate(query, query->queryRoot, object);
finish:
    return result;
}

#pragma mark Command-Line Argument Processing

/*******************************************************************************
*
*******************************************************************************/
Boolean
QEQueryAppendElementFromArgs(
    QEQueryRef query,
    int argc,
    char * const argv[],
    uint32_t * num_used)
{
    Boolean result = false;
    uint32_t index = 0;
    Boolean negated = false;
    CFMutableDictionaryRef element = NULL;  // must release
    CFStringRef predicate = NULL;           // must release

    if (!argv[index]) {
        goto finish;
    }

    predicate = _QEPredicateForString(query, argv[index]);

    if (CFEqual(predicate,
        CFDictionaryGetValue(query->operators, CFSTR(kQEQueryTokenNot)))) {

        index++;
        negated = true;

        if (!argv[index]) {
            query->lastError = kQEQueryErrorSyntax;
            goto finish;
        }

    }

    if (predicate)  CFRelease(predicate);
    predicate = _QEPredicateForString(query, argv[index]);

   /* This is not an 'else'! Not applies to most of the following.
    */
    if (CFEqual(predicate,
        CFDictionaryGetValue(query->operators, CFSTR(kQEQueryTokenAnd)))) {

        if (negated) {
            query->lastError = kQEQueryErrorSyntax;
            goto finish;
        }
        if (!QEQueryAppendAndOperator(query)) {
            goto finish;
        }
        index++;
    } else if (CFEqual(predicate,
        CFDictionaryGetValue(query->operators, CFSTR(kQEQueryTokenOr)))) {

        if (negated) {
            query->lastError = kQEQueryErrorSyntax;
            goto finish;
        }
        if (!QEQueryAppendOrOperator(query)) {
            goto finish;
        }
        index++;
    } else if (CFEqual(predicate,
        CFDictionaryGetValue(query->operators, CFSTR(kQEQueryTokenGroupStart)))) {

        if (!_QEQueryStartGroup(query, false /* 'and' group */, negated)) {
            // called function sets query->lastError
            goto finish;
        }
        index++;
    } else if (CFEqual(predicate,
        CFDictionaryGetValue(query->operators, CFSTR(kQEQueryTokenGroupEnd)))) {

        if (!QEQueryEndGroup(query)) {
            // called function sets query->lastError
            goto finish;
        }
        index++;
    } else {
        QEQueryParseCallback parseCallback = NULL;
        uint32_t callbackNumUsed = 0;

        element = QEQueryCreateElement(query, predicate, NULL, negated);
        if (!element) {
            // called function sets query->lastError
            goto finish;
        }

        parseCallback = _QEQueryParseCallbackForPredicate(query, predicate);
        if (!parseCallback) {
            query->lastError = kQEQueryErrorNoParseCallback;
            goto finish;
        }

       /* Base args off index in case we consumed a NOT.
        */
        if (!parseCallback(element, argc - index, &argv[index],
            &callbackNumUsed, query->userData, &query->lastError)) {

            // user callback sets query->lastError
            goto finish;
        }

        index += callbackNumUsed;

        if (!QEQueryAppendElement(query, element)) {
            // called function sets query->lastError
            goto finish;
        }
    }

    result = true;

finish:
    if (num_used) {
        if (result) {
            *num_used += index;
        } else if (negated) {
            *num_used += 1;
        }
    }
    if (element) CFRelease(element);
    if (predicate) CFRelease(predicate);
    return result;
}

#pragma mark Hand-Building Queries

/*******************************************************************************
*
*******************************************************************************/
CFMutableDictionaryRef
QEQueryCreateElement(
    QEQueryRef  query,
    CFStringRef predicate,
    CFArrayRef  arguments,
    Boolean     negated)
{
    CFMutableDictionaryRef result = NULL;
    CFStringRef synonym = NULL;   // do not release (or retain!)

    if (!predicate) {
        goto finish;
    }

    result = CFDictionaryCreateMutable(kCFAllocatorDefault,
        0 /* no limit */, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!result) {
        query->lastError = kQEQueryErrorNoMemory;
        goto finish;
    }

    synonym = CFDictionaryGetValue(query->synonyms, predicate);
    if (synonym) {
        predicate = synonym;
    }

    QEQueryElementSetPredicate(result, predicate);

    if (arguments) {
        QEQueryElementSetArgumentsArray(result, arguments);
    }

    if (negated) {
        CFDictionarySetValue(result, kQEQueryKeyNegated,
            kCFBooleanTrue);
    }

finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
QEQueryAppendElement(
    QEQueryRef query,
    CFMutableDictionaryRef element)
{
    Boolean result = false;
    CFMutableArrayRef elements = NULL;

   /* If there is no logic operator active, and the current group has
    * at least one item, treat this as being ANDed to the current group.
    */
    if (!query->logicOpActive && !_QEQueryStackTopHasCount(query, 0)) {
        if (!QEQueryAppendAndOperator(query)) {
            // called function sets query->lastError
            goto finish;
        }
    }

    elements = (CFMutableArrayRef)CFDictionaryGetValue(
        query->queryStackTop, kQEQueryKeyArguments);

    CFArrayAppendValue(elements, (const void *)element);
    query->logicOpActive = false;

    result = true;
finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
QEQueryAppendAndOperator(QEQueryRef query)
{
    Boolean result = false;

    if (query->logicOpActive) {
        query->lastError = kQEQueryErrorSyntax;
        goto finish;
    }

    query->logicOpActive = true;

    if (_QEQueryStackTopIsOrGroup(query)) {
        if (!_QEQueryStartGroup(query, true /* 'and' group */,
            false /* negated */)) {

            // called function sets query->lastError
            goto finish;
        }
    }

    result = true;

finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
QEQueryAppendOrOperator(QEQueryRef query)
{
    Boolean result = false;

    if (query->logicOpActive) {
        query->lastError = kQEQueryErrorSyntax;
        goto finish;
    }

    query->logicOpActive = true;

    if (_QEQueryStackTopIsAndGroup(query)) {
        if (!_QEQueryPopGroup(query)) {
            goto finish;
        }
    }

    result = true;

finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
QEQueryStartGroup(
    QEQueryRef query,
    Boolean negated)
{
    return _QEQueryStartGroup(query, false /* not an 'and' group */, negated);
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
_QEQueryStartGroup(
    QEQueryRef query,
    Boolean andFlag,
    Boolean negated)
{
    Boolean result = false;

    if (!query->logicOpActive && _QEQueryStackTopIsOrGroup(query)) {
        if (!_QEQueryPushGroup(query, false /* negated */,
            true /* 'and' group */)) {

            // called function sets query->lastError
            goto finish;
        }
    }

    if (!_QEQueryPushGroup(query, negated, andFlag)) {
        // called function sets query->lastError
        goto finish;
    }

    result = true;
finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
QEQueryEndGroup(QEQueryRef query)
{
    Boolean result = false;

   /* A complete query has no open groups.
    */
    if (QEQueryIsComplete(query)) {
         query->lastError = kQEQueryErrorGroupNesting;
         goto finish;
    }

   /* Don't allow empty parentheses within an explicit group.
    * Why? Well, find(1) doesn't.
    */
    if (_QEQueryStackTopHasCount(query, 0)) {
         query->lastError = kQEQueryErrorEmptyGroup;
         goto finish;
    }

   /* If we are in an implicit group binding AND operators, end it
    * now so that the explicit group binding OR operators can be ended.
    */
    if (_QEQueryStackTopIsAndGroup(query)) {
        if (!_QEQueryPopGroup(query)) {
            // called function sets query->lastError
            goto finish;
        }
    }

   /* Two nested AND groups is an internal logic error.
    */
    if (_QEQueryStackTopIsAndGroup(query)) {
        query->lastError = kQEQueryErrorGroupNesting;
        goto finish;
    }

   /* Now, if the topmost group is not an OR group, something else
    * is wrong with out internal logic.
    */
    if (!_QEQueryStackTopIsOrGroup(query)) {
        query->lastError = kQEQueryErrorGroupNesting;
        goto finish;
    }

   /* At last we can do something! Pop that OR group off the stack to close
    * it, and if it only contains one element, coalesce it into its parent.
    */
    if (!_QEQueryPopGroupAndCoalesce(query, false /* pop only if can coalesce */)) {
        query->lastError = kQEQueryErrorGroupNesting;
        goto finish;
    }

   /* Decrement the stack's OR group nesting level.
    */
    query->nestingLevel--;

   /* Now, if we are left with an AND group at the top of the stack, see if
    * we can coalesce that too.
    */
    if (_QEQueryStackTopIsAndGroup(query)) {
        if (!_QEQueryPopGroupAndCoalesce(query, true /* pop only if can coalesce */)) {
            query->lastError = kQEQueryErrorGroupNesting;
            goto finish;
        }
    }

    result = true;
finish:
    return result;
}


#pragma mark Query Elements

/*******************************************************************************
*
*******************************************************************************/
CFStringRef
QEQueryElementGetPredicate(CFDictionaryRef element)
{
    return CFDictionaryGetValue(element, kQEQueryKeyPredicate);
}

/*******************************************************************************
*
*******************************************************************************/
CFMutableArrayRef
QEQueryElementGetArguments(CFDictionaryRef element)
{
    CFMutableDictionaryRef dict = (CFMutableDictionaryRef)element;
    CFMutableArrayRef args = (CFMutableArrayRef)CFDictionaryGetValue(
        element, kQEQueryKeyArguments);
    if (!args) {
        args = CFArrayCreateMutable(kCFAllocatorDefault,
            0, &kCFTypeArrayCallBacks);
        if (!args) {
            goto finish;
        }
        QEQueryElementSetArgumentsArray(dict, args);
        CFRelease(args);
    }
finish:
    return args;
}

/*******************************************************************************
*
*******************************************************************************/
CFTypeRef
QEQueryElementGetArgumentAtIndex(
    CFDictionaryRef element,
    CFIndex i)
{
    CFArrayRef arguments = QEQueryElementGetArguments(element);
    return CFArrayGetValueAtIndex(arguments, i);
}

/*******************************************************************************
*
*******************************************************************************/
void
QEQueryElementSetPredicate(
    CFMutableDictionaryRef element,
    CFStringRef predicate)
{
    CFDictionarySetValue(element, kQEQueryKeyPredicate, predicate);
    return;
}

/*******************************************************************************
*
*******************************************************************************/
void
QEQueryElementSetArgumentsArray(
    CFMutableDictionaryRef element,
    CFArrayRef arguments)
{
    CFDictionarySetValue(element, kQEQueryKeyArguments, arguments);
    return;
}

/*******************************************************************************
*
*******************************************************************************/
void
QEQueryElementAppendArgument(
    CFMutableDictionaryRef element,
    CFTypeRef argument)
{
    CFMutableArrayRef arguments = (CFMutableArrayRef)QEQueryElementGetArguments(element);
    CFArrayAppendValue(arguments, argument);
    return;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
QEQueryElementSetArguments(
    CFMutableDictionaryRef element,
    uint32_t numArgs,
    ...)
{
    Boolean result = false;
    CFMutableArrayRef arguments = NULL;
    va_list ap;
    uint32_t i;

    if (!numArgs) {
        goto finish;
    }

    arguments = QEQueryElementGetArguments(element);
    if (!arguments) {
        arguments = CFArrayCreateMutable(kCFAllocatorDefault,
            0, &kCFTypeArrayCallBacks);
        if (!arguments) {
            goto finish;
        }
        CFDictionarySetValue(element, kQEQueryKeyArguments, arguments);
        CFRelease(arguments);
    }

    va_start(ap, numArgs);
    for (i = 0; i < numArgs; i++) {
        char * arg = va_arg(ap, char *);
        if (arg) {
            CFStringRef string = CFStringCreateWithCString(kCFAllocatorDefault,
                arg, kCFStringEncodingUTF8);
            if (!string) {
                goto finish;
            }
            CFArrayAppendValue(arguments, string);
            CFRelease(string);
        }
    }

    result = true;

finish:
    va_end(ap);
    return result;
}

#pragma mark Debug Printing (Crufty)

/******************************************************************************/

void
_QEQueryElementPrint(CFDictionaryRef element, Boolean printOuterDelimiter)
{
    CFIndex count, i;
    char * andGroupOpen = "{";
    char * andGroupClose = "}";
    CFTypeRef value = NULL;

    if (_QEQueryElementIsNegated(element)) {
         printf("-not ");
    }

    value = CFDictionaryGetValue(element, kQEQueryKeyPredicate);
    if (CFEqual(value, kQEQueryPredicateAnd) ||
        CFEqual(value, kQEQueryPredicateOr)) {

        Boolean andGroup = CFEqual(value, kQEQueryPredicateAnd);
        CFArrayRef elements = CFDictionaryGetValue(element, kQEQueryKeyArguments);

        if (elements) {
            count = CFArrayGetCount(elements);
            if (count) {
                if (printOuterDelimiter) {
                    printf("%s", andGroup ? andGroupOpen : "(");fflush(stdout);
                }
                for (i = 0; i < count; i++) {
                    if (i) {
                        printf(" %s ", andGroup ? "-and" : "-or");fflush(stdout);
                    }
                    _QEQueryElementPrint(CFArrayGetValueAtIndex(elements, i),
                        true);
                }
                if (printOuterDelimiter) {
                    printf("%s", andGroup ? andGroupClose : ")");fflush(stdout);
                }
            }
        }
    } else {
        char * predicate = NULL;
        CFArrayRef arguments = NULL;

        predicate = createUTF8CStringForCFString(value);
        if (!predicate) {
            fprintf(stderr, "%s", kMsgStringConversionError);fflush(stderr);
            goto finish;
        }

        printf("%s", predicate);fflush(stdout);
        free(predicate);

        arguments = CFDictionaryGetValue(element, kQEQueryKeyArguments);
        if (arguments) {
            count = CFArrayGetCount(arguments);
            if (count) {
                for (i = 0; i < count; i++) {
                    CFStringRef argString = CFArrayGetValueAtIndex(arguments, i);
                    if (CFGetTypeID(argString) == CFStringGetTypeID()) {
                        char * arg = createUTF8CStringForCFString(argString);
                        if (!arg) {
                            fprintf(stderr, "%s", kMsgStringConversionError);fflush(stderr);
                            goto finish;
                        }
                        printf(" %s", arg);fflush(stdout);
                        free(arg);
                    } else {
                        printf(" <non-string>");fflush(stdout);
                    }
                }
            }
        }
    }
finish:
    return;
}

/*******************************************************************************
*
*******************************************************************************/
//extern void printPList(FILE * stream, CFTypeRef plist);

void
QEQueryPrint(QEQueryRef query)
{
//    printPList(stdout, query->queryRoot);
    _QEQueryElementPrint(query->queryRoot, false);
    printf("\n");
    return;
}

#pragma mark Internal Functions

/*******************************************************************************
* _QEQueryCreateGroup() creates a grouping dictionary and returns it. Returns
* NULL on failure.
*******************************************************************************/
CFMutableDictionaryRef
_QEQueryCreateGroup(Boolean andFlag)
{
    CFMutableDictionaryRef result = NULL;    // returned
    CFMutableArrayRef      elements = NULL;  // must release
    Boolean ok = false;

    result = CFDictionaryCreateMutable(kCFAllocatorDefault,
        0 /* no limit */, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!result) {
        goto finish;
    }

    CFDictionarySetValue(result, kQEQueryKeyPredicate,
        andFlag ? kQEQueryPredicateAnd : kQEQueryPredicateOr);

    elements = CFArrayCreateMutable(kCFAllocatorDefault,
        0, &kCFTypeArrayCallBacks);
    if (!elements) {
        goto finish;
    }

    CFDictionarySetValue(result, kQEQueryKeyArguments, elements);

    CFRelease(elements);
    elements = NULL;

    ok = true;

finish:
    if (!ok) {
        if (result) {
            CFRelease(result);
            result = NULL;
        }
    }
    return result;
}

/*******************************************************************************
* _QEQueryYankElement() pulls the last element, if any, from the group at the
* top of the query's group stack, and returns it. It's used to pull an
* element from an OR group when a higher-precendence AND operator is
* encountered.
*******************************************************************************/
CFMutableDictionaryRef
_QEQueryYankElement(QEQueryRef query)
{
    CFMutableDictionaryRef result = NULL;
    CFMutableArrayRef elements = NULL;
    CFIndex count;

    elements = (CFMutableArrayRef)CFDictionaryGetValue(
        query->queryStackTop, kQEQueryKeyArguments);
    count = CFArrayGetCount(elements);

    if (count) {
        result = (CFMutableDictionaryRef)CFArrayGetValueAtIndex(
            elements, count - 1);
        CFRetain(result);
        CFArrayRemoveValueAtIndex(elements, count - 1);
    }

    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
_QEQueryStackTopIsAndGroup(QEQueryRef query)
{
    CFStringRef predicate = CFDictionaryGetValue(query->queryStackTop,
        kQEQueryKeyPredicate);
    if (predicate && CFEqual(predicate, kQEQueryPredicateAnd)) {
        return true;
    }
    return false;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
_QEQueryStackTopIsOrGroup(QEQueryRef query)
{
    CFStringRef predicate = CFDictionaryGetValue(query->queryStackTop,
        kQEQueryKeyPredicate);
    if (predicate && CFEqual(predicate, kQEQueryPredicateOr)) {
        return true;
    }
    return false;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
_QEQueryStackTopHasCount(QEQueryRef query, CFIndex count)
{
    CFArrayRef elements = CFDictionaryGetValue(query->queryStackTop,
        kQEQueryKeyArguments);
    if (elements && CFArrayGetCount(elements) == count) {
        return true;
    } else if (!elements && (count == 0)) {
        return true;
    }
    return false;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
_QEQueryPushGroup(
    QEQueryRef query,
    Boolean negated,
    Boolean andFlag)
{
    Boolean result = false;
    CFMutableDictionaryRef newGroup = NULL;
    CFMutableDictionaryRef yankedElement = NULL;
    CFMutableArrayRef elements = NULL;

    newGroup = _QEQueryCreateGroup(andFlag);
    if (!newGroup) {
        query->lastError = kQEQueryErrorNoMemory;
        goto finish;
    }

    if (negated) {
        CFDictionarySetValue(newGroup, kQEQueryKeyNegated, kCFBooleanTrue);
    }

    if (andFlag) {
        yankedElement = _QEQueryYankElement(query);
    } else {
        query->nestingLevel++;
    }

    elements = (CFMutableArrayRef)CFDictionaryGetValue(
        query->queryStackTop, kQEQueryKeyArguments);

    CFArrayAppendValue(elements, (const void *)newGroup);
    CFArrayAppendValue(query->queryStack, (const void *)newGroup);
    query->queryStackTop = newGroup;

    if (yankedElement) {
        QEQueryAppendElement(query, yankedElement);
        CFRelease(yankedElement);
    }

    result = true;

finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
_QEQueryPopGroup(QEQueryRef query)
{
    Boolean result = false;
    CFIndex stackDepth;

    stackDepth = CFArrayGetCount(query->queryStack);
    if (stackDepth < 2) {
//        fprintf(stderr, "query stack error\n");
        query->lastError = kQEQueryErrorGroupNesting;
        goto finish;
    }
    CFArrayRemoveValueAtIndex(query->queryStack, stackDepth - 1);
    query->queryStackTop = (CFMutableDictionaryRef)CFArrayGetValueAtIndex(
        query->queryStack, stackDepth - 2);

    result = true;
finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
_QEQueryPopGroupAndCoalesce(
    QEQueryRef query,
    Boolean onlyIfCanCoalesce)
{
    Boolean result = false;
    CFMutableArrayRef topArgs = NULL;
    CFMutableDictionaryRef singleElement = NULL;
    CFMutableDictionaryRef groupOfOne = NULL;
    Boolean canCoalesce = false;

    topArgs = QEQueryElementGetArguments(query->queryStackTop);

    canCoalesce = (_QEQueryStackTopHasCount(query, 1) &&
        (query->queryStackTop != query->queryRoot));
    if (onlyIfCanCoalesce && !canCoalesce) {
        result = true;
        goto finish;
    }

    if (canCoalesce) {
        singleElement = (CFMutableDictionaryRef)CFArrayGetValueAtIndex(topArgs, 0);
        CFRetain(singleElement);
    }

    if (!_QEQueryPopGroup(query)) {
        // called function sets query->lastError
        goto finish;
    }

    topArgs = QEQueryElementGetArguments(query->queryStackTop);

    if (canCoalesce) {
        groupOfOne = _QEQueryYankElement(query);
        if (!groupOfOne) {
            query->lastError = kQEQueryErrorGroupNesting;
            goto finish;
        }
        CFRelease(groupOfOne);
        if (_QEQueryElementIsNegated(groupOfOne)) {
            _QEQueryElementNegate(singleElement);
        }
        CFArrayAppendValue(topArgs, singleElement);
    }

    result = true;
finish:
    if (singleElement) CFRelease(singleElement);
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
CFStringRef _QEQueryOperatorForToken(
    QEQueryRef query,
    CFStringRef token)
{
    return CFDictionaryGetValue(query->operators, token);
}

/*******************************************************************************
*
*******************************************************************************/
CFStringRef _QEPredicateForString(
    QEQueryRef query,
    char * string)
{
    CFStringRef predicate = NULL;
    CFStringRef synonym = NULL;   // do not release

    predicate = CFStringCreateWithCString(kCFAllocatorDefault,
        string, kCFStringEncodingUTF8);
    if (!predicate) {
        goto finish;
    }

    synonym = CFDictionaryGetValue(query->synonyms, predicate);
    if (synonym) {
        CFRelease(predicate);
        predicate = CFRetain(synonym);
    }

finish:
    return predicate;
}

/*******************************************************************************
*
*******************************************************************************/
QEQueryParseCallback
_QEQueryParseCallbackForPredicate(
    QEQueryRef query,
    CFStringRef predicate)
{
    QEQueryParseCallback result = NULL;
    CFDataRef callbackData = NULL;
    const void * dataPtr = NULL;
    QEQueryParseCallback * callback = NULL;

    callbackData = CFDictionaryGetValue(query->parseCallbacks, predicate);
    if (!callbackData) {
        goto finish;
    }
    dataPtr = CFDataGetBytePtr(callbackData);
    callback = (QEQueryParseCallback *)dataPtr;
    result = *callback;
finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
QEQueryEvaluationCallback
_QEQueryEvaluationCallbackForPredicate(
    QEQueryRef query,
    CFStringRef predicate)
{
    QEQueryEvaluationCallback result = NULL;
    CFDataRef callbackData = NULL;
    const void * dataPtr = NULL;
    QEQueryEvaluationCallback * callback = NULL;

    callbackData = CFDictionaryGetValue(query->evaluationCallbacks, predicate);
    if (!callbackData) {
        goto finish;
    }
    dataPtr = CFDataGetBytePtr(callbackData);
    callback = (QEQueryEvaluationCallback *)dataPtr;
    result = *callback;
finish:
    return result;
}

/*******************************************************************************
*
*******************************************************************************/
Boolean
_QEQueryElementIsNegated(CFDictionaryRef element)
{
    if (CFDictionaryGetValue(element, kQEQueryKeyNegated)) {
        return true;
    }
    return false;
}

/*******************************************************************************
*
*******************************************************************************/
void
_QEQueryElementNegate(CFMutableDictionaryRef element)
{
    if (CFDictionaryGetValue(element, kQEQueryKeyNegated)) {
        CFDictionaryRemoveValue(element, kQEQueryKeyNegated);
    } else {
        CFDictionarySetValue(element, kQEQueryKeyNegated,
            kCFBooleanTrue);
    }
    return;
}
