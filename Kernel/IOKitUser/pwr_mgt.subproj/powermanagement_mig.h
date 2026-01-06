/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#ifndef	_powermanagement_mig_h_
#define	_powermanagement_mig_h_

#define kPMMIGStringLength                          1024

typedef char * string_t;

enum {
    kIOPMGetValueDWBTSupportOnAC                        = 1,
    kIOPMGetValueDWBTSupportOnBatt                      = 2
};

/* 
 * 'setMode' values in io_pm_set_debug_flags() MIG call
 */
enum {
    kIOPMDebugFlagsSetBits,
    kIOPMDebugFlagsResetBits,
    kIOPMDebugFlagsSetValue
};

/*
 * Arguments to powermanagement.defs MIG call io_pm_assertion_copy_details
 *    parameter "whichData"
 */
enum {
    kIOPMAssertionMIGCopyOneAssertionProperties     = 1,
    kIOPMAssertionMIGCopyAll                        = 2,
    kIOPMAssertionMIGCopyStatus                     = 3,
    kIOPMPowerEventsMIGCopyScheduledEvents          = 4,
    kIOPMPowerEventsMIGCopyRepeatEvents             = 5,
    kIOPMAssertionMIGCopyByType                     = 6,
    kIOPMAssertionMIGCopyInactive                   = 7,
};

/*
 * Arguments to powermanagement.defs MIG call io_pm_assertion_retain_release
 *    parameter "action"
 */
enum {
    kIOPMAssertionMIGDoRetain                       = 1,
    kIOPMAssertionMIGDoRelease                      = -1
};


/*
 * XPC based messaging keys
 */

#define kMsgReturnCode              "returnCode"
#define kClaimSystemWakeEvent       "claimSystemWakeEvent"

#define kUserActivityRegister           "userActivityRegister"
#define kUserActivityTimeoutUpdate      "userActivityTimeout"

#define kUserActivityTimeoutKey     "ActivityTimeout"
#define kUserActivityLevels         "UserActivityLevels"

#define kAssertionCreateMsg         "assertionCreate"
#define kAssertionReleaseMsg        "assertionRelease"
#define kAssertionPropertiesMsg     "assertionProperties"
#define kAssertionCheckMsg          "assertionCheck"
#define kAssertionTimeoutMsg        "assertionTimeout"
#define kAssertionSetStateMsg       "assertionSetState"

#define kAssertionDetailsKey        "assertionDictonary"
#define kAssertionIdKey             "assertionId"
#define kAssertionReleaseDateKey    "assertioReleaseDate"
#define kAssertionEnTrIntensityKey  "EnTrIntensity"
#define kAssertionCheckTokenKey     "assertionCheckToken"
#define kAssertionCheckCountKey     "assertionCheckCount"

#define kPSAdapterDetails           "adapterDetails"

#define kInactivityWindowKey        "inactivityWindow"
#define kInactivityWindowStart      "inactivityWindowStart"
#define kInactivityWindowDuration   "inactivityWindowDuration"
#define kStandbyAccelerationDelay   "standbyAccelerationDelay"


#endif
