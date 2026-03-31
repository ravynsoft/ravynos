/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#ifndef _KEXTD_MAIN_H
#define _KEXTD_MAIN_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/kext/OSKext.h>

#include <getopt.h>
#include <sysexits.h>

#include "kext_tools_util.h"


#pragma mark Basic Types & Constants
/*******************************************************************************
* Basic Types & Constants
*******************************************************************************/
enum {
    kKextdExitOK          = EX_OK,
    kKextdExitError,
    kKextdExitSigterm,

    // don't actually exit with this, it's just a sentinel value
    kKextdExitHelp        = 33
};

typedef struct {
    mach_msg_header_t header;
    int signum;
} kextd_mach_msg_signal_t;


#define kAppleSetupDonePath       "/var/db/.AppleSetupDone"
#define kKextcacheDelayStandard   (60)
#define kKextcacheDelayFirstBoot  (60 * 5)

#define kReleaseKextsDelay  (30)

#pragma mark Command-line Option Definitions
/*******************************************************************************
* Command-line options. This data is used by getopt_long_only().
*
* Options common to all kext tools are in kext_tools_util.h.
*******************************************************************************/

#define kOptNameNoCaches      "no-caches"
#define kOptNameDebug         "debug"
#define kOptNameNoJettison    "no-jettison"

#define kOptNoCaches          'c'
#define kOptDebug             'd'
#define kOptSafeBoot          'x'

#define kOptChars             "cdhqvx"

/* Options with no single-letter variant.  */
// Do not use -1, that's getopt() end-of-args return value
// and can cause confusion
#define kLongOptLongindexHack (-2)

#pragma mark Tool Args Structure
/*******************************************************************************
* Tool Args Structure
*******************************************************************************/
typedef struct {
    Boolean            useRepositoryCaches;
    Boolean            debugMode;
    Boolean            safeBootMode;     // actual or simulated

    Boolean            firstBoot;
} KextdArgs;

extern CFArrayRef gRepositoryURLs;

#pragma mark Function Prototypes
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
ExitStatus readArgs(int argc, char * const * argv, KextdArgs * toolArgs);

void       checkStartupMkext(KextdArgs * toolArgs);
Boolean    isNetboot(void);
void       sendActiveToKernel(void);
void       sendFinishedToKernel(void);
ExitStatus setUpServer(KextdArgs * toolArgs);

bool isBootRootActive(void);

void handleSignal(int signum);
void handleSignalInRunloop(
    CFMachPortRef  port,
        void     * msg,
        CFIndex    size,
        void     * info);

void readExtensions(void);
void scheduleReleaseExtensions(void);
void releaseExtensions(CFRunLoopTimerRef timer, void * context);
void rescanExtensions(void);

void enableNetworkAuthentication(void);

void usage(UsageLevel usageLevel);

#endif /* _KEXTD_MAIN_H */
