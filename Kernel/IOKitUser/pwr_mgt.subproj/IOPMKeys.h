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

/*!
    @header IOPMKeys.h
    
    IOPMKeys.h defines C strings for use accessing power management data.
    Note that all of these C strings must be converted to CFStrings before use. You can wrap
    them with the CFSTR() macro, or create a CFStringRef (that you must later CFRelease()) using CFStringCreateWithCString()
 */
 
#ifndef _IOPMKEYS_H_
#define _IOPMKEYS_H_
 
/*
 * Types of power event
 * These are potential arguments to IOPMSchedulePowerEvent().
 * These are all potential values of the kIOPMPowerEventTypeKey in the CFDictionaries
 * returned by IOPMCopyScheduledPowerEvents().
 */
/*!
    @define kIOPMAutoWake 
    @abstract Value for scheduled wake from sleep.
*/
#define kIOPMAutoWake                                   "wake"

/*!
    @define kIOPMAutoPowerOn 
    @abstract Value for scheduled power on from off state.
*/
#define kIOPMAutoPowerOn                                "poweron"

/*!
    @define kIOPMAutoWakeOrPowerOn 
    @abstract Value for scheduled wake from sleep, or power on. The system will either wake OR
        power on, whichever is necessary.
*/

#define kIOPMAutoWakeOrPowerOn                          "wakepoweron"
/*!
    @define kIOPMAutoSleep
    @abstract Value for scheduled sleep.
*/

#define kIOPMAutoSleep                                   "sleep"
/*!
    @define kIOPMAutoShutdown 
    @abstract Value for scheduled shutdown.
*/

#define kIOPMAutoShutdown                               "shutdown"
/*!
    @define kIOPMAutoRestart 
    @abstract Value for scheduled restart.
*/

#define kIOPMAutoRestart                                "restart"

/*
 * Keys for evaluating the CFDictionaries returned by IOPMCopyScheduledPowerEvents()
 */
/*!
    @define kIOPMPowerEventTimeKey 
    @abstract Key for the time of the scheduled power event. Value is a CFDateRef.
*/
#define kIOPMPowerEventTimeKey                          "time"

/*!
    @define kIOPMPowerEventAppNameKey 
    @abstract Key for the CFBundleIdentifier of the app that scheduled the power event. Value is a CFStringRef.
*/
#define kIOPMPowerEventAppNameKey                       "scheduledby"

/*!
 @define kIOPMPowerEventAppPIDKey
 @abstract Key for the PID the App that scheduled the power event. Value is a CFNumber integer.
 */
#define kIOPMPowerEventAppPIDKey                       "appPID"

/*!
    @define kIOPMPowerEventTypeKey 
    @abstract Key for the type of power event. Value is a CFStringRef, with the c-string value of one of the "kIOPMAuto" strings.
*/
#define kIOPMPowerEventTypeKey                          "eventtype"

#endif
