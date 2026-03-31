/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
/*
 *  CFNetworkSchedule.h
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on Sat May 01 2004.
 *  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 */

#ifndef __CFNETWORKSCHEDULE__
#define __CFNETWORKSCHEDULE__

#ifndef __COREFOUNDATION__
#include <CoreFoundation/CoreFoundation.h>
#endif


#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Currently does not handle CFRunLoopObserverRef or _CFNetConnectionRef.
 */

void _CFTypeScheduleOnRunLoop(CFTypeRef obj, CFRunLoopRef runLoop, CFStringRef runLoopMode);
void _CFTypeUnscheduleFromRunLoop(CFTypeRef obj, CFRunLoopRef runLoop, CFStringRef runLoopMode);

void _CFTypeScheduleOnMultipleRunLoops(CFTypeRef obj, CFArrayRef schedules);
void _CFTypeUnscheduleFromMultipleRunLoops(CFTypeRef obj, CFArrayRef schedules);

void _CFTypeInvalidate(CFTypeRef obj);

/*
 * Return if the run loop and mode were added to the list or removed from the list, respectively.
 */
Boolean _SchedulesAddRunLoopAndMode(CFMutableArrayRef schedules, CFRunLoopRef runLoop, CFStringRef runLoopMode);
Boolean _SchedulesRemoveRunLoopAndMode(CFMutableArrayRef schedules, CFRunLoopRef runLoop, CFStringRef runLoopMode);

/*
 *	Find the given run loop and mode in the list of schedules.  kCFNotFound returned if not found.
 */
CFIndex _SchedulesFind(CFArrayRef schedules, CFRunLoopRef runLoop, CFStringRef runLoopMode);

#ifdef __cplusplus
}
#endif

#endif /* __CFNETWORKSCHEDULE__ */
