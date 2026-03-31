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
#ifndef _QEQUERY_H_
#define _QEQUERY_H_

#include <CoreFoundation/CoreFoundation.h>

typedef struct __QEQuery * QEQueryRef;

/*******************************************************************************
* Flexible Query Engine
*
* This engine builds complex logical expression evaluators with predicates you
* define via callbacks. The engine directly supports AND, OR, NOT operators,
* as well as grouping. The engine is currently designed to build queries by
* serially adding elements, and figures out the grouping by itself. Operator
* precedence is explicit groups, then NOT, then AND, then OR.
*
* Query elements are dictionaries with three basic attributes:
*
* - Predicate: the 'verb' to evaluate
* - Arguments: data for use by the predicate
* - Negated:   whether the evaluated result must be negated (this is handled
*              by the engine, and your callbacks needn't worry about it)
*
* Your callbacks can stuff arbitrary key/value pairs into the dictionary in
* lieu of using the arguments array. Note that the engine reserves keys of the
* form '__QE...__', so don't use those.
*
********************************************************************************
* Using a Query
*
* You start by calling QEQueryCreate(), which takes a user data pointer for
* context info. Then you set parse and evaluation callbacks for the predicates
* you want to handle, add elements to your query using one of the two methods
* just below, and then use QEQueryEvaluate() on the objects you want to check.
*
* QEQueryEvaluate() returns true if the query matches the object you provide,
* and false if it doesn't or if an evaluation error occurs. If it does return
* false, you should call QEQueryLastError() and possibly cancel further queries
* using that query engine instance if an error did occur.
*
**********
* Building a Query from Strings
*
* If you set parse callbacks, you need only call QEQueryAppendElementFromArgs()
* repeatedly until it returns false. After doing that you should check whether
* there were any errors using QEQueryLastError().
*
* By default, the parse engine automatically  handles the tokens '-and', '-or',
* '-not', '(', and ')', but does not give them any priority over your parse
* callbacks. That is, if your callback eats up an '-and' as an argument for a
* predicate, the query engine will just keep rolling along (and you might see a
* syntax error later).
*
* Your parse calllback will have as arguments:
*
* - The element being built, with its predicate already set to the token
*   that triggered the callback.
* - A pointer to a list of strings following that token,  which you can
*   proceed to parse.
* - An in/out 'num used' parameter giving how many tokens have been parsed
*   by the query engine as a whole; you should update this by the number of
*   tokens you consume in parsing, if successful--if parsing fails, do not
*   adjust it. This number is *not* to be used as an index into the list
*   of argument strings; that starts at zero.
* - A pointer to the query user data.
* - A pointer to the query's error code, which you should set to
*   kQEQueryErrorInvalidOrMissingArgument or kQEQueryErrorParseCallbackFailed
*   if something goes wrong.
*
* Your callback should update the element by changing its predicate if necessary
* (so you can have a single keyword routed to different evaluation calbacks,
* or multiple keywords routed into a single evaluation callback), and reading
* as many arguments as needed from the list of strings and adding them to the
* elements arguments array. You can also set arbitrary properties on the element
* as noted above. It should update the num_used parameter as it successfully
* handles arguments. Upon successfully parsing all potential arguments, it
* should  return true; otherwise, it should set the error code as appopriate
* and return false.
*
* Functions that you may find useful in a parse callback are:
*
* - QEQueryElementGetPredicate()
* - QEQueryElementSetPredicate()
* - QEQueryElementAppendArgument()
* - QEQueryElementSetArgumentsArray()
* - QEQueryElementSetArguments()
*
* NOTE: If you get a reference to query element's predicate, and then set the
* predicate, your original predicate will change! Always 're-fetch' the
* predicate if you need to check it after setting it. (I think this is a CF
* bug, but I haven't investigated it thoroughly).
*
**********
* Building a Query by Hand
*
* You can forgo using parse callbacks if you want to build query elements
* directly, using:
*
* - QEQueryCreateElement() - creates an element with a user-define predicate
* - QEQueryAppendElement()
* - QEQueryAppendAndOperator() and QEQueryAppendOrOperator()
* - QEQueryStartGroup() and QEQueryEndGroup()
*
* Just call them in sequence and the engine will produce the correct evaluation
* structure. These functions return true on success, and false if they would
* result in an incorrectly-structures query (for example, with two OR operators
* in a row). You can call QEQueryLastError() to find out the specific problem.
*
********************************************************************************
* Evaluating a Query
*
* Your evaluation callback will have as arguments:
*
* - The query element being evaluated.
* - A pointer to the object being checked.
* - A pointer to the query use data.
* - A pointer to the query's error code, which you should set to
*   kQEQueryErrorEvaluationCallbackFailed if something goes wrong.
*
* Your callback should call QEQueryElementGetPredicate(),
* QEQueryElementGetArguments(), and QEQueryElementGetArgumentAtIndex()
* as necessary to apply its logic to the object and return a true or false
* value. It can also get arbitrary keys from the dictionary, as noted above.
* Your callback does not need to handle whether the query element is negated;
* the query engine does that.
*
* If your callback suffers a failure, it should set the error and return false.
*
* Note that evaluation callbacks can perform operations on or with the object
* as well as just checking them against a query predicate. For example, you
* could define a '-print' predicate that just prints data from the object
* and returns true.
********************************************************************************
* TO DO:
* XXX: Add functions that take CF strings?
* XXX: Allow for inspection/traversal of query, and insertion?
* XXX: Make QEQueryPrint() output friendlier (always record raw input tokens?)
* XXX: Add internal parse item counter and formatted error messages listing
* XXX:     problem item and its number?
*******************************************************************************/
typedef enum {
    kQEQueryErrorNone = 0,
    kQEQueryErrorUnspecified,
    kQEQueryErrorNoMemory,
    kQEQueryErrorGroupNesting,  // doesn't distinguish lib error from user....
    kQEQueryErrorEmptyGroup,
    kQEQueryErrorSyntax,
    kQEQueryErrorNoParseCallback,
    kQEQueryErrorNoEvaluationCallback,
    kQEQueryErrorInvalidOrMissingArgument,   // set by user parse callback
    kQEQueryErrorParseCallbackFailed,        // set by user parse callback
    kQEQueryErrorEvaluationCallbackFailed,   // set by user eval callback
} QEQueryError;

/* The default set of tokens understood by the engine.
 * You may need to handle these specially while processing
 * options.
 */
#define kQEQueryTokenAnd         "-and"
#define kQEQueryTokenOr          "-or"
#define kQEQueryTokenNot         "-not"
#define kQEQueryTokenGroupStart  "("
#define kQEQueryTokenGroupEnd    ")"

typedef Boolean (*QEQueryParseCallback)(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

typedef Boolean (*QEQueryEvaluationCallback)(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error);

/*******************************************************************************
* Create and set up a query.
*******************************************************************************/
QEQueryRef QEQueryCreate(void * userData);
void QEQueryFree(QEQueryRef query);

/* Replace the builtin logical operators with your own. If you pass NULL
 * for any one, the builtin is used (and for -not, ! is automatically registered
 * as a synonym). If you just want to add synonyms for logical operators, or
 * remove the default alias ! for -not, use QEQuerySetSynonymForPredicate().
 */
void QEQuerySetOperators(QEQueryRef query,
    CFStringRef andPredicate,
    CFStringRef orPredicate,
    CFStringRef notPredicate,
    CFStringRef groupStartPredicate,
    CFStringRef groupEndPredicate);

/* Empty all custom parse information: predicates, synonyms, and callbacks.
 */
void QEQueryEmptyParseDictionaries(QEQueryRef query);

/* You can remove a single callback by passing NULL.
 */
void QEQuerySetParseCallbackForPredicate(
    QEQueryRef query,
    CFStringRef predicate,
    QEQueryParseCallback parseCallback);

void QEQuerySetEvaluationCallbackForPredicate(
    QEQueryRef query,
    CFStringRef predicate,
    QEQueryEvaluationCallback evaluationCallback);

/* Causes 'synonym' to be automatically replaced with 'predicate' during
 * parsing and upon creation of an element dictionary with
 * QEQueryCreateElement(). If 'predicate' is NULL, the synonym is unregistered.
 */
void QEQuerySetSynonymForPredicate(
    QEQueryRef  query,
    CFStringRef synonym,
    CFStringRef predicate);

Boolean QEQueryIsComplete(QEQueryRef query);
QEQueryError QEQueryLastError(QEQueryRef query);

/*******************************************************************************
* Evaluate a query.
*******************************************************************************/
void    QEQuerySetShortCircuits(QEQueryRef query, Boolean flag);
Boolean QEQueryGetShortCircuits(QEQueryRef query);
Boolean QEQueryEvaluate(QEQueryRef query, void * object);

/*******************************************************************************
* Build a query from command-line arguments. See below for hand-building.
*******************************************************************************/
Boolean QEQueryAppendElementFromArgs(
    QEQueryRef query,
    int argc,
    char * const argv[],
    uint32_t * num_used);

/*******************************************************************************
* Functions for manually building a query.
*******************************************************************************/
CFMutableDictionaryRef QEQueryCreateElement(
    QEQueryRef  query,
    CFStringRef predicate,  // will be replaced if a synonym for another
    CFArrayRef  arguments,  // may be NULL
    Boolean     negated);

/* Do not manually build a dictionary element; use QEQueryCreateElement(),
 * which sets internal values needed by the query engine.
 */
Boolean QEQueryAppendElement(
    QEQueryRef query,
    CFMutableDictionaryRef element);

Boolean QEQueryAppendAndOperator(QEQueryRef query);
Boolean QEQueryAppendOrOperator(QEQueryRef query);

Boolean QEQueryStartGroup(QEQueryRef query, Boolean negated);
Boolean QEQueryEndGroup(QEQueryRef query);

/*******************************************************************************
* Functions for use by parse and evaluation callbacks.
*******************************************************************************/
CFStringRef       QEQueryElementGetPredicate(CFDictionaryRef element);
CFMutableArrayRef QEQueryElementGetArguments(CFDictionaryRef element);
CFTypeRef         QEQueryElementGetArgumentAtIndex(
    CFDictionaryRef element,
    CFIndex i);

/*******************************************************************************
* Functions for use by parse callbacks.
*******************************************************************************/
void QEQueryElementSetPredicate(CFMutableDictionaryRef element,
    CFStringRef predicate);
void QEQueryElementAppendArgument(CFMutableDictionaryRef element,
    CFTypeRef argument);
void QEQueryElementSetArgumentsArray(CFMutableDictionaryRef element,
    CFArrayRef arguments);
Boolean QEQueryElementSetArguments(CFMutableDictionaryRef element,
    uint32_t numArgs,
    ...);

/*******************************************************************************
* Print the raw CFPropertyList contents of a query.
*******************************************************************************/
void QEQueryPrint(QEQueryRef query);

#endif /* _QEQUERY_H_ */
