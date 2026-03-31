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
#ifndef _KEXTFIND_H_
#define _KEXTFIND_H_

#include <CoreFoundation/CoreFoundation.h>
#include <libc.h>
#include <getopt.h>
#include <mach-o/arch.h>
#include <sysexits.h>

#include <IOKit/IOTypes.h>
#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/fat_util.h>


#include "kext_tools_util.h"
#include "QEQuery.h"

#pragma mark Basic Types & Constants
/*******************************************************************************
* Constants
*******************************************************************************/

enum {
    kKextfindExitOK          = EX_OK,

    // don't actually exit with this, it's just a sentinel value
    kKextfindExitHelp        = 33,
};

/*******************************************************************************
* Data types.
*******************************************************************************/

/* I originally thought this was a good idea, but changed my mind. I'm leaving
 * the code in case someone hates it, but kextfind will just always be picky,
 * now.
 */
typedef enum {
    kKextfindMeek = -1,
    kKextfindQuibbling = 0,
    kKextfindPicky = 1
} KextfindAssertiveness;

typedef enum {
    kPathsFull = 0,
    kPathsRelative,
    kPathsNone
} PathSpec;

/* The query context is passed as user data to the query engine.
 */
typedef struct {

   /* These fields are set by command-line options and govern global behavior
    * during the search.
    */
    KextfindAssertiveness assertiveness;
    const NXArchInfo    * defaultArch;

    Boolean  caseInsensitive;
    Boolean  extraInfo;       // currently unused, see EXTRA_INFO ifdefs
    PathSpec pathSpec;
    Boolean  substrings;

    CFMutableArrayRef searchURLs;

   /* These fields are set by the parsing callbacks to determine what
    * expensive operations the kext manager needs to perform before the
    * query can be evaluated.
    */
    Boolean checkLoaded;

   /* Kext integrity is no longer used on SnowLeopard. We read the
    * flags but no kext will ever match them now.
    */
    Boolean checkIntegrity;

   /* This field is set by the parsing callbacks. If no commands are given
    * in the query, a default "print" will be executed for each matching
    * kext.
    */
    Boolean commandSpecified;

   /* If false, the report logic will print the report header as needed.
    */
    Boolean reportStarted;

   /* If true, the report log will print a tab before the next value.
    */
    Boolean reportRowStarted;

} QueryContext;

/*******************************************************************************
* Function prototypes.
*******************************************************************************/
ExitStatus readArgs(
    int            argc,
    char * const * argv,
    QueryContext * toolArgs);
ExitStatus checkArgs(QueryContext * toolArgs);
Boolean checkSearchItem(const char * pathname, Boolean logFlag);
fat_iterator createFatIteratorForKext(OSKextRef aKext);
void usage(UsageLevel level);


#endif /* _KEXTFIND_H_ */
