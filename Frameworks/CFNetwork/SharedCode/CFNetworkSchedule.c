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
 *  CFNetworkSchedule.c
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on Sat May 01 2004.
 *  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 */

#include "CFNetworkSchedule.h"
#include <CFNetwork/CFNetwork.h>

#include <SystemConfiguration/SystemConfiguration.h>


/* extern */ void
_CFTypeScheduleOnRunLoop(CFTypeRef obj, CFRunLoopRef runLoop, CFStringRef runLoopMode) {
	
	CFTypeID t = CFGetTypeID(obj);
	CFTypeRef src = NULL;
	void(*fn)(CFTypeRef, CFRunLoopRef, CFStringRef);
	void(*fn2)(CFRunLoopRef, CFTypeRef, CFStringRef);
	
	fn = NULL;
	fn2 = (void(*)(CFRunLoopRef, CFTypeRef, CFStringRef))CFRunLoopAddSource;
	
	/* Get the correct source or function used for adding the object to the run loop. */
	if (t == CFRunLoopSourceGetTypeID()) {
		src = CFRetain(obj);
	}
	
	else if (t == CFMachPortGetTypeID()) {
		src = CFMachPortCreateRunLoopSource(CFGetAllocator(obj), (CFMachPortRef)obj, 0);
	}
	
	else if (t == CFSocketGetTypeID()) {
		src = CFSocketCreateRunLoopSource(CFGetAllocator(obj), (CFSocketRef)obj, 0);
	}
	
	else if (t == CFReadStreamGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFReadStreamScheduleWithRunLoop;
	}
	
	else if (t == CFWriteStreamGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFWriteStreamScheduleWithRunLoop;
	}
	
	else if (t == CFHostGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFHostScheduleWithRunLoop;
	}
	
	else if (t == SCNetworkReachabilityGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))SCNetworkReachabilityScheduleWithRunLoop;
	}
	
	else if (t == CFRunLoopTimerGetTypeID()) {
		src = CFRetain(obj);
		fn2 = (void(*)(CFRunLoopRef, CFTypeRef, CFStringRef))CFRunLoopAddTimer;
	}
	
	else if (t == CFNetServiceGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceScheduleWithRunLoop;
	}
	
	else if (t == CFNetServiceBrowserGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceBrowserScheduleWithRunLoop;
	}
	
	else if (t == CFNetServiceMonitorGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceMonitorScheduleWithRunLoop;
	}

	else if (t == SCNetworkConnectionGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))SCNetworkConnectionScheduleWithRunLoop;
	}
	
	
	/* If a source was retrieved, need to add the source */
	if (src) {
		fn2(runLoop, src, runLoopMode);
		CFRelease(src);
	}
	
	/* If a schedule function was retrieved, call it. */
	else if (fn) {
		fn(obj, runLoop, runLoopMode);
	}
}


/* extern */ void
_CFTypeUnscheduleFromRunLoop(CFTypeRef obj, CFRunLoopRef runLoop, CFStringRef runLoopMode) {
	
	CFTypeID t = CFGetTypeID(obj);
	CFTypeRef src = NULL;
	void(*fn)(CFTypeRef, CFRunLoopRef, CFStringRef);
	void(*fn2)(CFRunLoopRef, CFTypeRef, CFStringRef);
	
	fn = NULL;
	fn2 = (void(*)(CFRunLoopRef, CFTypeRef, CFStringRef))CFRunLoopRemoveSource;
	
	/* Get the proper source or function for removing the object from the run loop. */
	if (t == CFRunLoopSourceGetTypeID()) {
		src = CFRetain(obj);
	}
	
	else if (t == CFMachPortGetTypeID()) {
		src = CFMachPortCreateRunLoopSource(CFGetAllocator(obj), (CFMachPortRef)obj, 0);
	}
	
	else if (t == CFSocketGetTypeID()) {
		src = CFSocketCreateRunLoopSource(CFGetAllocator(obj), (CFSocketRef)obj, 0);
	}
	
	else if (t == CFReadStreamGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFReadStreamUnscheduleFromRunLoop;
	}
	
	else if (t == CFWriteStreamGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFWriteStreamUnscheduleFromRunLoop;
	}
	
	else if (t == CFHostGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFHostUnscheduleFromRunLoop;
	}
	
	else if (t == SCNetworkReachabilityGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))SCNetworkReachabilityUnscheduleFromRunLoop;
	}
	
	else if (t == CFRunLoopTimerGetTypeID()) {
		src = CFRetain(obj);
		fn2 = (void(*)(CFRunLoopRef, CFTypeRef, CFStringRef))CFRunLoopRemoveTimer;
	}
	
	else if (t == CFNetServiceGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceUnscheduleFromRunLoop;
	}
	
	else if (t == CFNetServiceBrowserGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceBrowserUnscheduleFromRunLoop;
	}
	
	else if (t == CFNetServiceMonitorGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceMonitorUnscheduleFromRunLoop;
	}
	
	else if (t == SCNetworkConnectionGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))SCNetworkConnectionUnscheduleFromRunLoop;
	}
	
	/* If a source was retrieved, need to remove it */
	if (src) {
		fn2(runLoop, src, runLoopMode);
		CFRelease(src);
	}
	
	/* If an unschedule function was retrieved, need to call it. */
	else if (fn) {
		fn(obj, runLoop, runLoopMode);
	}
}


/* extern */ void
_CFTypeScheduleOnMultipleRunLoops(CFTypeRef obj, CFArrayRef schedules) {
	
	CFTypeID t = CFGetTypeID(obj);
	CFTypeRef src = NULL;
	void(*fn)(CFTypeRef, CFRunLoopRef, CFStringRef);
	void(*fn2)(CFRunLoopRef, CFTypeRef, CFStringRef);
	
	fn = NULL;
	fn2 = (void(*)(CFRunLoopRef, CFTypeRef, CFStringRef))CFRunLoopAddSource;
	
	/* Get the correct source or function used for adding the object to the run loop. */
	if (t == CFRunLoopSourceGetTypeID()) {
		src = CFRetain(obj);
	}
	
	else if (t == CFRunLoopTimerGetTypeID()) {
		src = CFRetain(obj);
		fn2 = (void(*)(CFRunLoopRef, CFTypeRef, CFStringRef))CFRunLoopAddTimer;
	}
	
	else if (t == CFMachPortGetTypeID()) {
		src = CFMachPortCreateRunLoopSource(CFGetAllocator(obj), (CFMachPortRef)obj, 0);
	}
	
	else if (t == CFSocketGetTypeID()) {
		src = CFSocketCreateRunLoopSource(CFGetAllocator(obj), (CFSocketRef)obj, 0);
	}
	
	else if (t == CFReadStreamGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFReadStreamScheduleWithRunLoop;
	}
	
	else if (t == CFWriteStreamGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFWriteStreamScheduleWithRunLoop;
	}

	else if (t == CFHostGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFHostScheduleWithRunLoop;
	}
	
	else if (t == CFNetServiceGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceScheduleWithRunLoop;
	}
	
	else if (t == CFNetServiceBrowserGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceBrowserScheduleWithRunLoop;
	}
	
	else if (t == CFNetServiceMonitorGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceMonitorScheduleWithRunLoop;
	}
	
	else if (t == SCNetworkReachabilityGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))SCNetworkReachabilityScheduleWithRunLoop;
	}
	
	else if (t == SCNetworkConnectionGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))SCNetworkConnectionScheduleWithRunLoop;
	}
	
	/* If a source was retrieved, need to add the source to the list of run loops */
	if (src) {
		
		CFIndex i, length = CFArrayGetCount(schedules);
		
		for (i = 0; i < length; i += 2) {
			fn2((CFRunLoopRef)CFArrayGetValueAtIndex(schedules, i),
				src,
				(CFStringRef)CFArrayGetValueAtIndex(schedules, i + 1));
		}

		CFRelease(src);
	}
	
	/* If a schedule function was retrieved, call it for each schedule in the list. */
	else if (fn) {
		
		CFIndex i, length = CFArrayGetCount(schedules);
		
		for (i = 0; i < length; i += 2) {
			fn(obj, 
			   (CFRunLoopRef)CFArrayGetValueAtIndex(schedules, i),
			   (CFStringRef)CFArrayGetValueAtIndex(schedules, i + 1));
		}
	}
}


/* extern */ void
_CFTypeUnscheduleFromMultipleRunLoops(CFTypeRef obj, CFArrayRef schedules) {
	
	CFTypeID t = CFGetTypeID(obj);
	CFTypeRef src = NULL;
	void(*fn)(CFTypeRef, CFRunLoopRef, CFStringRef);
	void(*fn2)(CFRunLoopRef, CFTypeRef, CFStringRef);
	
	fn = NULL;
	fn2 = (void(*)(CFRunLoopRef, CFTypeRef, CFStringRef))CFRunLoopRemoveSource;
	
	/* Get the proper source or function for removing the object from the run loop. */
	if (t == CFRunLoopSourceGetTypeID()) {
		src = CFRetain(obj);
	}
	
	else if (t == CFMachPortGetTypeID()) {
		src = CFMachPortCreateRunLoopSource(CFGetAllocator(obj), (CFMachPortRef)obj, 0);
	}
	
	else if (t == CFSocketGetTypeID()) {
		src = CFSocketCreateRunLoopSource(CFGetAllocator(obj), (CFSocketRef)obj, 0);
	}
	
	else if (t == CFReadStreamGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFReadStreamUnscheduleFromRunLoop;
	}
	
	else if (t == CFWriteStreamGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFWriteStreamUnscheduleFromRunLoop;
	}
	
	else if (t == CFHostGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFHostUnscheduleFromRunLoop;
	}
	
	else if (t == SCNetworkReachabilityGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))SCNetworkReachabilityUnscheduleFromRunLoop;
	}
	
	else if (t == CFRunLoopTimerGetTypeID()) {
		src = CFRetain(obj);
		fn2 = (void(*)(CFRunLoopRef, CFTypeRef, CFStringRef))CFRunLoopRemoveTimer;
	}
	
	else if (t == CFNetServiceGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceUnscheduleFromRunLoop;
	}
	
	else if (t == CFNetServiceBrowserGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceBrowserUnscheduleFromRunLoop;
	}
	
	else if (t == CFNetServiceMonitorGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))CFNetServiceMonitorUnscheduleFromRunLoop;
	}
	
	else if (t == SCNetworkConnectionGetTypeID()) {
		fn = (void(*)(CFTypeRef, CFRunLoopRef, CFStringRef))SCNetworkConnectionUnscheduleFromRunLoop;
	}
	
	/* If a source was retrieved, need to remove it from the list of run loops*/
	if (src) {
		
		CFIndex i, length = CFArrayGetCount(schedules);
		
		for (i = 0; i < length; i += 2) {
			fn2((CFRunLoopRef)CFArrayGetValueAtIndex(schedules, i),
				src,
				(CFStringRef)CFArrayGetValueAtIndex(schedules, i + 1));
		}

		CFRelease(src);
	}

	/* If an unschedule function was retrieved, need to call it for each schedule in the list. */
	else if (fn) {
		
		CFIndex i, length = CFArrayGetCount(schedules);
		
		for (i = 0; i < length; i += 2) {
			fn(obj, 
			   (CFRunLoopRef)CFArrayGetValueAtIndex(schedules, i),
			   (CFStringRef)CFArrayGetValueAtIndex(schedules, i + 1));
		}
	}
}


/* extern */ void
_CFTypeInvalidate(CFTypeRef obj) {
	
	CFTypeID t = CFGetTypeID(obj);
	
	/* Invalidate according to type of object. */
	if (t == CFRunLoopSourceGetTypeID()) {
		CFRunLoopSourceInvalidate((CFRunLoopSourceRef)obj);
	}
	
	else if (t == CFMachPortGetTypeID()) {
		CFMachPortInvalidate((CFMachPortRef)obj);
	}
	
	else if (t == CFSocketGetTypeID()) {
		CFSocketInvalidate((CFSocketRef)obj);
	}
	
	/* For scheduled types of objects, it is invalidated by setting the client to NULL. */
	else if (t == CFReadStreamGetTypeID()) {
		CFReadStreamSetClient((CFReadStreamRef)obj, kCFStreamEventNone, NULL, NULL);
	}
	
	else if (t == CFWriteStreamGetTypeID()) {
		CFWriteStreamSetClient((CFWriteStreamRef)obj, kCFStreamEventNone, NULL, NULL);
	}
	
	else if (t == CFHostGetTypeID()) {
		CFHostSetClient((CFHostRef)obj, NULL, NULL);
	}
	
	else if (t == SCNetworkReachabilityGetTypeID()) {
		SCNetworkReachabilitySetCallback((SCNetworkReachabilityRef)obj, NULL, NULL);
	}
	
	else if (t == CFRunLoopTimerGetTypeID()) {
		CFRunLoopTimerInvalidate((CFRunLoopTimerRef)obj);
	}
	
	else if (t == CFNetServiceGetTypeID()) {
		CFNetServiceSetClient((CFNetServiceRef)obj, NULL, NULL);
	}
	
	else if (t == CFNetServiceBrowserGetTypeID()) {
		CFNetServiceBrowserInvalidate((CFNetServiceBrowserRef)obj);
	}

	else if (t == CFNetServiceMonitorGetTypeID()) {
		CFNetServiceMonitorInvalidate((CFNetServiceMonitorRef)obj);
	}
	
	else if (t == SCNetworkReachabilityGetTypeID()) {
		SCNetworkConnectionStop((SCNetworkConnectionRef)obj, FALSE);
	}
}


/* extern */ Boolean
_SchedulesAddRunLoopAndMode(CFMutableArrayRef schedules, CFRunLoopRef runLoop, CFStringRef runLoopMode) {

	/* Get the number of items in schedules and create a range for searching */
    CFIndex count = CFArrayGetCount(schedules);
    CFRange range = CFRangeMake(0, count);

	/* Go through the list looking for this schedule */
    while (range.length) {

		/* Find the run loop in the list */
        CFIndex i = CFArrayGetFirstIndexOfValue(schedules, range, runLoop);

		/* If the loop wasn't found, then this has never been scheduled on this loop and mode */
        if (i == kCFNotFound)
            break;

		/* If the mode is the same, this is already scheduled here so bail */
        if (CFEqual(CFArrayGetValueAtIndex(schedules, i + 1), runLoopMode)) {

			/* Did not add the pair to the list */
            return FALSE;
		}

		/* Continue looking from here */
		range.location = i + 2;
		range.length = count - range.location;
    }

	/* Schedule wasn't found, so add it to the list. */
    CFArrayAppendValue(schedules, runLoop);
    CFArrayAppendValue(schedules, runLoopMode);
	
	/* Did add the pair to the list */
	return TRUE;
}


/* extern */ Boolean
_SchedulesRemoveRunLoopAndMode(CFMutableArrayRef schedules, CFRunLoopRef runLoop, CFStringRef runLoopMode) {

	/* Get the number of items in schedules and create a range for searching */
    CFIndex count = CFArrayGetCount(schedules);
    CFRange range = CFRangeMake(0, count);

	/* Go through the list looking for this schedule */
    while (range.length) {

		/* Find the run loop in the list */
        CFIndex i = CFArrayGetFirstIndexOfValue(schedules, range, runLoop);

		/* If the loop wasn't found, then this pair was never added. */
        if (i == kCFNotFound)
            break;

		/* If the mode is the same, this is already scheduled here so bail */
        if (CFEqual(CFArrayGetValueAtIndex(schedules, i + 1), runLoopMode)) {

			/* Remove the schedule from the list */
            range.location = i;
            range.length = 2;
            CFArrayReplaceValues(schedules, range, NULL, 0);

			/* Did remove the schedule from the list */
			return TRUE;
		}

		/* Continue looking from here */
		range.location = i + 2;
		range.length = count - range.location;
    }

	/* Did not remove the schedule from the list */
	return FALSE;
}


/* extern */ CFIndex
_SchedulesFind(CFArrayRef schedules, CFRunLoopRef runLoop, CFStringRef runLoopMode) {
	
	/* Get the number of items in schedules and create a range for searching */
    CFIndex count = CFArrayGetCount(schedules);
    CFRange range = CFRangeMake(0, count);
	
	/* Go through the list looking for this schedule */
    while (range.length) {
		
		/* Find the run loop in the list */
        CFIndex i = CFArrayGetFirstIndexOfValue(schedules, range, runLoop);
		
		/* If the loop wasn't found, then this pair was never added. */
        if (i == kCFNotFound)
            break;
		
		/* If the mode is the same, found it */
        if (CFEqual(CFArrayGetValueAtIndex(schedules, i + 1), runLoopMode))
			return i;
		
		/* Continue looking from here */
		range.location = i + 2;
		range.length = count - range.location;
    }
	
	/* Did not find the schedule in the list */
	return kCFNotFound;
}

