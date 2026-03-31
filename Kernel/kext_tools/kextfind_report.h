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
#ifndef _KEXTFIND_REPORT_H_
#define _KEXTFIND_REPORT_H_

#include "QEQuery.h"
#include "kextfind_tables.h"
#include "kextfind_query.h"

/* These arenn't processed by getopt or QEQuery, we just look for them.
 */
#define kKeywordReport   "-report"
#define kNoReportHeader  "-no-header"

#define kPredNameSymbol  "-symbol"
#define kPredCharSymbol  "-sym"

/*******************************************************************************
* Query Engine Callbacks
*
* The Query Engine invokes these as it finds keywords from the above list
* in the command line or the query being reportEvaluated.
*******************************************************************************/
Boolean reportParseProperty(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean reportParseShorthand(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean reportEvalProperty(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean reportEvalMatchProperty(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean reportParseFlag(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean reportEvalFlag(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean reportParseArch(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean reportEvalArch(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean reportEvalArchExact(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean reportParseCommand(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean reportParseDefinesOrReferencesSymbol(
    CFMutableDictionaryRef element,
    int argc,
    char * const argv[],
    uint32_t * num_used,
    void * user_data,
    QEQueryError * error);

Boolean reportEvalDefinesOrReferencesSymbol(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error);

Boolean reportEvalCommand(
    CFDictionaryRef element,
    void * object,
    void * user_data,
    QEQueryError * error);

#endif /* _KEXTFIND_REPORT_H_ */
