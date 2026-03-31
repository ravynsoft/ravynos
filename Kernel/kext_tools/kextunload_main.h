/*
 * Copyright (c) 2008 Apple Computer, Inc. All rights reserved.
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
#ifndef _KEXTUNLOAD_MAIN_H
#define _KEXTUNLOAD_MAIN_H

#include <IOKit/kext/OSKext.h>

#include <getopt.h>
#include <sysexits.h>

#include "kext_tools_util.h"

#pragma mark Basic Types & Constants
/*******************************************************************************
* Constants
*******************************************************************************/
enum {
    kKextunloadExitOK = EX_OK,
    kKextunloadExitNotFound = 1,
    kKextunloadExitNotPrivileged = 2,
    kKextunloadExitPartialFailure = 3,

    // don't think we use it
    kKextunloadExitUnspecified = 11,

    // don't actually exit with this, it's just a sentinel value
    kKextunloadExitHelp = 33
};

#pragma mark Command-line Option Definitions
/*******************************************************************************
* Command-line options. This data is used by getopt_long_only().
*
* Options common to all kext tools are in kext_tools_util.h.
*******************************************************************************/

#define kOptNameClassName           "class"
#define kOptNamePersonalitiesOnly   "personalities-only"

// really old option letter for "module", same as bundle id
#define kOptClassName          'c'
#define kOptModule             'm'
#define kOptPersonalitiesOnly  'p'

#define kOptChars  "b:c:hm:pqr:v"

int longopt = 0;

struct option sOptInfo[] = {
    { kOptNameHelp,                  no_argument,        NULL, kOptHelp },

    { kOptNameBundleIdentifier,      required_argument,  NULL, kOptBundleIdentifier },

    { kOptNameClassName,             required_argument,  NULL, kOptClassName },
    { kOptNamePersonalitiesOnly,     no_argument,        NULL, kOptPersonalitiesOnly },

    { kOptNameQuiet,                 required_argument,  NULL, kOptQuiet },
    { kOptNameVerbose,               optional_argument,  NULL, kOptVerbose },

    { NULL, 0, NULL, 0 }  // sentinel to terminate list
};


#pragma mark Tool Args Structure
/*******************************************************************************
* Tool Args Structure
*******************************************************************************/
typedef struct {
    Boolean           unloadPersonalities;  // -p
    uint32_t          terminateOption;      // -p

    CFMutableArrayRef kextURLs;             // args
    CFMutableArrayRef kextBundleIDs;        // -b/-m -- array of C strings!
    CFMutableArrayRef kextClassNames;       // -c    -- array of C strings!

    CFMutableArrayRef kexts;                // any kexts created from URLs
} KextunloadArgs;

#pragma mark Function Prototypes
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
ExitStatus   readArgs(
    int              argc,
    char * const *   argv,
    KextunloadArgs * toolArgs);
ExitStatus   checkArgs(KextunloadArgs * toolArgs);
ExitStatus   createKextsIfNecessary(KextunloadArgs * toolArgs);

ExitStatus terminateKextClasses(
    KextunloadArgs * toolArgs,
    Boolean * fatal);
ExitStatus unloadKextsByIdentifier(
    KextunloadArgs * toolArgs,
    Boolean * fatal);
ExitStatus unloadKextsByURL(
    KextunloadArgs * toolArgs,
    Boolean * fatal);
ExitStatus unloadKextWithIdentifier(
    CFStringRef      kextIdentifier,
    KextunloadArgs * toolArgs,
    Boolean        * fatal);

ExitStatus   formatKernResult(kern_return_t kernResult);
void         usage(UsageLevel usageLevel);

#endif /* _KEXTUNLOAD_MAIN_H */
