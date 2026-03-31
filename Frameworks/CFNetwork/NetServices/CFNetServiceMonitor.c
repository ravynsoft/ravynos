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
 *  CFNetServiceMonitor.c
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on Sun May 09 2004.
 *  Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 */

#if 0
#pragma mark Includes
#endif
#include <CFNetwork/CFNetwork.h>
#include "CFNetworkInternal.h"			// for __CFSpinLock and __CFSpinUnlock
#include "CFNetworkSchedule.h"

#include <dns_sd.h>
#include <nameser.h>


#if 0
#pragma mark -
#pragma mark Extern Function Declarations
#endif

extern Boolean _CFNetServiceSetInfoNoPublish(CFNetServiceRef theService, UInt32 property, CFTypeRef value);


#if 0
#pragma mark -
#pragma mark Constant Strings
#endif

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFNetServiceMonitorBlockingMode	CFSTR("_kCFNetServiceMonitorBlockingMode")
#else
static CONST_STRING_DECL(_kCFNetServiceMonitorBlockingMode, "_kCFNetServiceMonitorBlockingMode")
#endif	/* __CONSTANT_CFSTRINGS__ */

static const char _kCFNetServiceMonitorClassName[] = "CFNetServiceMonitor";


#if 0
#pragma mark -
#pragma mark CFNetServiceMonitor struct
#endif

typedef struct {

	CFRuntimeBase						_base;
	
	CFSpinLock_t						_lock;

	CFStreamError						_error;

	CFNetServiceRef						_service;

	CFTypeRef							_trigger;
	DNSServiceRef						_monitor;
	CFNetServiceMonitorType				_type;

	CFMutableArrayRef					_schedules;		// List of loops and modes
	CFNetServiceMonitorClientCallBack	_callback;
	CFNetServiceClientContext			_client;
} __CFNetServiceMonitor;


#if 0
#pragma mark -
#pragma mark Static Function Declarations
#endif

static void _CFNetServiceMonitorRegisterClass(void);

static void _MonitorDestroy(__CFNetServiceMonitor* monitor);

static void _MonitorCancel(__CFNetServiceMonitor* monitor);
static Boolean _MonitorBlockUntilComplete(__CFNetServiceMonitor* monitor);

static void _QueryRecordReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
							  DNSServiceErrorType errorCode, const char* fullname, uint16_t rrtype,
							  uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context);
	
static void _SocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info);


#if 0
#pragma mark -
#pragma mark Globals
#endif

static _CFOnceLock _kCFNetServiceMonitorRegisterClass = _CFOnceInitializer;
static CFTypeID _kCFNetServiceMonitorTypeID = _kCFRuntimeNotATypeID;
static CFRuntimeClass* _kCFNetServiceMonitorClass = NULL;


#if 0
#pragma mark -
#pragma mark Static Function Definitions
#endif

/* static */ void
_CFNetServiceMonitorRegisterClass(void) {
	
	_kCFNetServiceMonitorClass = (CFRuntimeClass*)calloc(1, sizeof(_kCFNetServiceMonitorClass[0]));
	
	if (_kCFNetServiceMonitorClass) {
		
		_kCFNetServiceMonitorClass->version = 0;
		_kCFNetServiceMonitorClass->className = _kCFNetServiceMonitorClassName;
		_kCFNetServiceMonitorClass->finalize = (void(*)(CFTypeRef))_MonitorDestroy;
		
		_kCFNetServiceMonitorTypeID = _CFRuntimeRegisterClass(_kCFNetServiceMonitorClass);
	}
}


/* static */ void
_MonitorDestroy(__CFNetServiceMonitor* monitor) {
	
	// Prevent anything else from taking hold
	__CFSpinLock(&(monitor->_lock));
	
	// Release the user's context info if there is some and a release method
	if (monitor->_client.info && monitor->_client.release)
		monitor->_client.release(monitor->_client.info);
	
	// Cancel the outstanding trigger
	if (monitor->_trigger) {
		
		// Remove the trigger from run loops and modes
		if (monitor->_schedules)
			_CFTypeUnscheduleFromMultipleRunLoops(monitor->_trigger, monitor->_schedules);
		
		// Go ahead and invalidate the trigger
		_CFTypeInvalidate(monitor->_trigger);
		
		// Release the monitor now.
		CFRelease(monitor->_trigger);
	}
	
	// Need to clean up the service discovery stuff if there is
	if (monitor->_monitor) {
		
		// Release the underlying service discovery reference
		DNSServiceRefDeallocate(monitor->_monitor);
	}

	/* Release the service if there is one */
	if (monitor->_service) {
		CFRelease(monitor->_service);
		monitor->_service = NULL;
	}

	// Release the list of loops and modes
	if (monitor->_schedules)
		CFRelease(monitor->_schedules);
}


/* static */ void
_MonitorCancel(__CFNetServiceMonitor* monitor) {
	
	CFNetServiceMonitorClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	// Retain here to guarantee safety really after the monitor release,
	// but definitely before the callback.
	CFRetain(monitor);
	
	// Lock the monitor
	__CFSpinLock(&monitor->_lock);
	
	// If the monitor canceled, don't need to do any of this.
	if (monitor->_trigger) {
		
		// Save the callback if there is one at this time.
		cb = monitor->_callback;
		
		// Save the error and client information for the callback
		memmove(&error, &(monitor->_error), sizeof(error));
		info = monitor->_client.info;
		
		// Remove the trigger from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(monitor->_trigger, monitor->_schedules);
		
		// Invalidate the run loop source that got here
		CFRunLoopSourceInvalidate((CFRunLoopSourceRef)(monitor->_trigger));
		
		// Release the trigger now.
		CFRelease(monitor->_trigger);
		monitor->_trigger = NULL;
	}

	// Unlock the monitor so the callback can be made safely.
	__CFSpinUnlock(&monitor->_lock);
	
	// If there is a callback, inform the client of the finish.
	if (cb)
		cb((CFNetServiceMonitorRef)monitor, NULL, 0, NULL, &error, info);
	
	// Go ahead and release now that the callback is done.
	CFRelease(monitor);
}


/* static */ Boolean
_MonitorBlockUntilComplete(__CFNetServiceMonitor* monitor) {
	
	// Assume success by default
	Boolean result = TRUE;
	CFRunLoopRef rl = CFRunLoopGetCurrent();
	
	// Schedule in the blocking mode.
	CFNetServiceMonitorScheduleWithRunLoop((CFNetServiceMonitorRef)monitor, rl, _kCFNetServiceMonitorBlockingMode);
	
	// Lock in order to check for trigger
	__CFSpinLock(&(monitor->_lock));
	
	// Check that monitor exists.
	while (monitor->_trigger) {
		
		// Unlock again so the monitor can continue to be processed.
		__CFSpinUnlock(&(monitor->_lock));
		
		// Run the loop in a private mode with it returning whenever a source
		// has been handled.
		CFRunLoopRunInMode(_kCFNetServiceMonitorBlockingMode, DBL_MAX, TRUE);
		
		// Lock again in preparation for trigger check
		__CFSpinLock(&(monitor->_lock));		
	}
	
	// Fail if there was an error.
	if (monitor->_error.error)
		result = FALSE;
	
	// Unlock the monitor again.
	__CFSpinUnlock(&(monitor->_lock));
	
	// Unschedule from the blocking mode
	CFNetServiceMonitorUnscheduleFromRunLoop((CFNetServiceMonitorRef)monitor, rl, _kCFNetServiceMonitorBlockingMode);
	
	return result;
}


/* static */ void
_QueryRecordReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
				  DNSServiceErrorType errorCode, const char* fullname, uint16_t rrtype,
				  uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context)
{
	__CFNetServiceMonitor* monitor = context;
	CFNetServiceMonitorClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	CFDataRef data = NULL;
	CFNetServiceRef service = NULL;
	CFNetServiceMonitorType type = 0;
	
	// Retain here to guarantee safety really after the trigger release,
	// but definitely before the callback.
	CFRetain(monitor);
	
	// Lock the monitor
	__CFSpinLock(&monitor->_lock);
	
	// If the monitor canceled, don't need to do any of this.
	if (monitor->_monitor) {
		
		service = (CFNetServiceRef)CFRetain(monitor->_service);
		type = monitor->_type;
		
		// If there is an error, fold the monitor.
		if (errorCode) {
			
			// Save the error
			monitor->_error.error = errorCode;
			monitor->_error.domain = kCFStreamErrorDomainNetServices;
			
			// Remove the monitor from run loops and modes
			_CFTypeUnscheduleFromMultipleRunLoops(monitor->_trigger, monitor->_schedules);
			
			// Go ahead and invalidate the socket
			CFSocketInvalidate((CFSocketRef)(monitor->_trigger));
			
			// Release the monitor now.
			CFRelease(monitor->_trigger);
			monitor->_trigger = NULL;
			
			// Clean up the underlying service discovery stuff
			DNSServiceRefDeallocate(monitor->_monitor);
			monitor->_monitor = NULL;
		}
		
		else if (rdata && (flags & kDNSServiceFlagsAdd)) {
			
			data = CFDataCreate(CFGetAllocator(monitor), rdata, rdlen);
			
			/* Update the service with the info */
			_CFNetServiceSetInfoNoPublish(service, type, data);
		}
		
		cb = monitor->_callback;
		
		// Save the error and client information for the callback
		memmove(&error, &(monitor->_error), sizeof(error));
		info = monitor->_client.info;
		
	}
	
	// Unlock the monitor so the callback can be made safely.
	__CFSpinUnlock(&monitor->_lock);
	
	// If there is a callback, inform the client of the finish.
	if (cb && (flags & kDNSServiceFlagsAdd)) {
			
		// Inform the client.
		cb((CFNetServiceMonitorRef)monitor, service, type, data, &error, info);
	}
	
	// No longer need this after the callback
	if (service)
		CFRelease(service);
		
	/* Release the data if it was created */
	if (data)
		CFRelease(data);
	
	// Go ahead and release now that the callback is done.
	CFRelease(monitor);
}


/* static */ void
_SocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void* data, void* info) {
	
	DNSServiceErrorType err;
	__CFNetServiceMonitor* monitor = info;
	
	(void)s;		// unused
	(void)type;		// unused
	(void)address;  // unused
	(void)data;		// unused
	
	CFRetain(monitor);
	
	// Dispatch to process the result
	err = DNSServiceProcessResult(monitor->_monitor);
	
	// If there was an error, need to infor the client.
	if (err)
		_QueryRecordReply(monitor->_monitor, 0, 0, err, NULL, 0, 0, 0, NULL, 0, info);
	
	CFRelease(monitor);
}


#if 0
#pragma mark -
#pragma mark Extern Function Definitions (API)
#endif


/* CF_EXPORT */ CFTypeID
CFNetServiceMonitorGetTypeID(void) {

    _CFDoOnce(&_kCFNetServiceMonitorRegisterClass, _CFNetServiceMonitorRegisterClass);

    return _kCFNetServiceMonitorTypeID;
}

/* CF_EXPORT */ CFNetServiceMonitorRef
CFNetServiceMonitorCreate(CFAllocatorRef alloc, CFNetServiceRef theService, CFNetServiceMonitorClientCallBack clientCB, CFNetServiceClientContext* context) {
	
	__CFNetServiceMonitor* result = NULL;
	
	if (clientCB && context) {
		
		CFTypeID class_type = CFNetServiceMonitorGetTypeID();
	
		if (class_type != _kCFRuntimeNotATypeID) {
			result = (__CFNetServiceMonitor*)_CFRuntimeCreateInstance(alloc,
																	  class_type,
																	  sizeof(result[0]) - sizeof(CFRuntimeBase),
																	  NULL);
		}

		if (result) {

			// Save a copy of the base so it's easier to zero the struct
			CFRuntimeBase copy = result->_base;

			// Clear everything.
			memset(result, 0, sizeof(result[0]));

			// Put back the base
			memmove(&(result->_base), &copy, sizeof(result->_base));
			
			// Save the client's callback
			result->_callback = clientCB;

			// Copy the client's context
			memmove(&(result->_client), context, sizeof(result->_client));
			
			// If there is user data and a retain method, call it.
			if (result->_client.info && result->_client.retain)
				result->_client.info = (void*)(result->_client.retain(result->_client.info));

			// Create the list of loops and modes
			result->_schedules = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
			
			/* Need to save the service if successful */
			if (result->_schedules)
				result->_service = (CFNetServiceRef)CFRetain(theService);
				
			// If any failed, need to release and return null
			else {
				CFRelease((CFTypeRef)result);
				result = NULL;
			}
		}
	}
	
	return (CFNetServiceMonitorRef)result;
}


/* CF_EXPORT */ void
CFNetServiceMonitorInvalidate(CFNetServiceMonitorRef theMonitor) {
	
	__CFNetServiceMonitor* monitor = (__CFNetServiceMonitor*)theMonitor;
	
	// Lock the monitor
	__CFSpinLock(&(monitor->_lock));
	
	// Release the user's context info if there is some and a release method
	if (monitor->_client.info && monitor->_client.release)
		monitor->_client.release(monitor->_client.info);
	
	// Cancel the outstanding trigger
	if (monitor->_trigger) {
		
		// Remove the trigger from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(monitor->_trigger, monitor->_schedules);
		
		// Go ahead and invalidate the trigger
		_CFTypeInvalidate(monitor->_trigger);
		
		// Release the monitor now.
		CFRelease(monitor->_trigger);
		monitor->_trigger = NULL;
	}
	
	// Need to clean up the service discovery stuff if there is
	if (monitor->_monitor) {
		
		// Release the underlying service discovery reference
		DNSServiceRefDeallocate(monitor->_monitor);
		monitor->_monitor = NULL;
	}
	
	/* No longer need the service, so release it. */
	if (monitor->_service) {
		CFRelease(monitor->_service);
		monitor->_service = NULL;
	}
	
	// Zero out the callback and client context.
	monitor->_callback = NULL;
	memset(&(monitor->_client), 0, sizeof(monitor->_client));

	// Unlock the monitor.
	__CFSpinUnlock(&(monitor->_lock));
}


/* CF_EXPORT */ Boolean
CFNetServiceMonitorStart(CFNetServiceMonitorRef theMonitor, CFNetServiceMonitorType recordType, CFStreamError* error) {
	
	__CFNetServiceMonitor* monitor = (__CFNetServiceMonitor*)theMonitor;
	
	CFStreamError extra;
	Boolean result = FALSE;
	
	if (!error)
		error = &extra;
	
	memset(error, 0, sizeof(error[0]));
	
	// Retain so it doesn't go away underneath in the case of a callout.  This is really
	// no worry for async, but makes the memmove for the error more difficult to place
	// for synchronous without it being here.
	CFRetain(monitor);
	
	// Lock down the monitor to start monitor
	__CFSpinLock(&(monitor->_lock));
	
	do {
		UInt16 rrtype = (recordType & 0x0000FFFF);
		UInt16 rrclass = ((recordType & 0xFFFF0000) >> 16);
		
		int i;
		char properties[4][1024];
		CFStringRef values[] = {
			CFNetServiceGetDomain(monitor->_service),
			CFNetServiceGetType(monitor->_service),
			CFNetServiceGetName(monitor->_service)
		};
		
		CFSocketContext ctxt = {0, monitor, CFRetain, CFRelease, NULL};
		
		if (!monitor->_callback) {
			monitor->_error.error = kCFNetServicesErrorInvalid;
			monitor->_error.domain = kCFStreamErrorDomainNetServices;
			break;
		}
		
		// Check to see if there is an ongoing monitor already
		if (monitor->_trigger) {
		
			// If it's a mdns monitor, don't allow another.
			if (CFGetTypeID(monitor->_trigger) == CFSocketGetTypeID()) {
				monitor->_error.error = kCFNetServicesErrorInProgress;
				monitor->_error.domain = kCFStreamErrorDomainNetServices;
				break;
			}
			
			// It's just the cancel that hasn't fired yet, so cancel it.
			else {
				
				// Remove the trigger from run loops and modes
				_CFTypeUnscheduleFromMultipleRunLoops(monitor->_trigger, monitor->_schedules);
				
				// Invalidate the run loop source
				CFRunLoopSourceInvalidate((CFRunLoopSourceRef)(monitor->_trigger));
				
				// Release the trigger now.
				CFRelease(monitor->_trigger);
				monitor->_trigger = NULL;
			}
		}
		
		/* If it's the TXT monitor type, set up rrtype and rrclass correctly. */
		if (recordType == kCFNetServiceMonitorTXT) {
			rrtype = ns_t_txt;
			rrclass = ns_c_in;
		}
		
		/* Get the raw data for the properties to send down to mdns */
		for (i = 0; i < 3; i++) {
		
			CFIndex used;
			CFStringGetBytes(values[i],
							 CFRangeMake(0, CFStringGetLength(values[i])),
							 kCFStringEncodingUTF8,
							 0,
							 FALSE,
							 (UInt8*)properties[i],
							 sizeof(properties[i]) - 1,
							 &used);
			properties[i][used] = '\0';
		}
		
		DNSServiceConstructFullName(properties[3], properties[2], properties[1], properties[0]);
		
		monitor->_type = recordType;
		
		// Create the domain monitor at the service discovery level
		monitor->_error.error = DNSServiceQueryRecord(&monitor->_monitor,
													  kDNSServiceFlagsLongLivedQuery,
													  0,
													  properties[3],
													  rrtype,
													  rrclass,
													  _QueryRecordReply,
													  monitor);
		
		// Fail if it did.
		if (monitor->_error.error) {
			monitor->_error.domain = kCFStreamErrorDomainNetServices;
			break;
		}
		
		// Create the trigger for the monitor
		monitor->_trigger = CFSocketCreateWithNative(CFGetAllocator(monitor),
													 DNSServiceRefSockFD(monitor->_monitor),
													 kCFSocketReadCallBack,
													 _SocketCallBack,
													 &ctxt);
		
		// Make sure the CFSocket wrapper succeeded
		if (!monitor->_trigger) {
			
			// Try to use errno for the error
			monitor->_error.error = errno;
			
			// If it has no error in it, assume no memory
			if (!monitor->_error.error)
				monitor->_error.error = ENOMEM;
			
			// Correct domain and bail.
			monitor->_error.domain = kCFStreamErrorDomainPOSIX;
			
			DNSServiceRefDeallocate(monitor->_monitor);
			monitor->_monitor = NULL;
			
			break;
		}
		
		// Tell CFSocket not to close the native socket on invalidation.
		CFSocketSetSocketFlags((CFSocketRef)monitor->_trigger,
							   CFSocketGetSocketFlags((CFSocketRef)monitor->_trigger) & ~kCFSocketCloseOnInvalidate);
		
		// Async mode is complete at this point
		if (CFArrayGetCount(monitor->_schedules)) {
			
			// Schedule the trigger on the run loops and modes.
			_CFTypeScheduleOnMultipleRunLoops(monitor->_trigger, monitor->_schedules);
			
			// It's now succeeded.
			result = TRUE;
		}
		
		// If there is no callback, go into synchronous mode.
		else {
			
			// Unlock the monitor
			__CFSpinUnlock(&(monitor->_lock));
			
			// Wait for synchronous return
			result = _MonitorBlockUntilComplete(monitor);
			
			// Lock down the monitor
			__CFSpinLock(&(monitor->_lock));
		}
		
	} while (0);
	
	// Copy the error.
	memmove(error, &monitor->_error, sizeof(error[0]));
	
	// Unlock the monitor
	__CFSpinUnlock(&(monitor->_lock));
	
	// Release the earlier retain.
	CFRelease(monitor);
	
	return result;
}


/* CF_EXPORT */ void
CFNetServiceMonitorStop(CFNetServiceMonitorRef theMonitor, CFStreamError* error) {
	
	__CFNetServiceMonitor* monitor = (__CFNetServiceMonitor*)theMonitor;
	
	// By default, the error is marked as a cancel
	CFStreamError extra = {kCFStreamErrorDomainNetServices , kCFNetServicesErrorCancel};
	
	// Make sure error has a value.
	if (!error)
		error = &extra;
	
	// Lock down the monitor
	__CFSpinLock(&(monitor->_lock));
	
	// Make sure there is something to cancel.
	if (monitor->_trigger) {

		CFRunLoopSourceContext ctxt = {
			0,									// version
			monitor,							// info
			NULL,								// retain
			NULL,								// release
			NULL,								// copyDescription
			NULL,								// equal
			NULL,								// hash
			NULL,								// schedule
			NULL,								// cancel
			(void(*)(void*))(&_MonitorCancel)  // perform
		};
		
		// Remove the trigger from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(monitor->_trigger, monitor->_schedules);
		
		// Go ahead and invalidate the trigger
		_CFTypeInvalidate(monitor->_trigger);
		
		// Release the trigger now.
		CFRelease(monitor->_trigger);
		
		// Need to clean up the service discovery stuff if there is
		if (monitor->_monitor) {
			
			// Release the underlying service discovery reference
			DNSServiceRefDeallocate(monitor->_monitor);
			monitor->_monitor = NULL;
		}
		
		// Copy the error into place
		memmove(&(monitor->_error), error, sizeof(error[0]));
		
		// Create the cancel source
		monitor->_trigger = CFRunLoopSourceCreate(CFGetAllocator(monitor), 0, &ctxt);
		
		// If the cancel was created, need to schedule and signal it.
		if (monitor->_trigger) {
			
			CFArrayRef schedules = monitor->_schedules;
			CFIndex i, count = CFArrayGetCount(schedules);
			
			// Schedule the new trigger
			_CFTypeScheduleOnMultipleRunLoops(monitor->_trigger, schedules);
			
			// Signal the cancel for immediate attention.
			CFRunLoopSourceSignal((CFRunLoopSourceRef)(monitor->_trigger));
			
			// Make sure the signal can make it through
			for (i = 0; i < count; i += 2) {
				
				// Grab the run loop for checking
				CFRunLoopRef runloop = (CFRunLoopRef)CFArrayGetValueAtIndex(schedules, i);
				
				// If it's sleeping, need to further check it.
				if (CFRunLoopIsWaiting(runloop)) {
					
					// Grab the mode for further check
					CFStringRef mode = CFRunLoopCopyCurrentMode(runloop);
					
					if (mode) {
						
						// If the trigger is in the right mode, need to wake up the run loop.
						if (CFRunLoopContainsSource(runloop, (CFRunLoopSourceRef)(monitor->_trigger), mode)) {
							CFRunLoopWakeUp(runloop);
						}
						
						// Don't need this anymore.
						CFRelease(mode);
					}
				}
			}
		}
	}
	
	// Unlock the monitor
	__CFSpinUnlock(&(monitor->_lock));
}


/* CF_EXPORT */ void
CFNetServiceMonitorScheduleWithRunLoop(CFNetServiceMonitorRef theMonitor, CFRunLoopRef runLoop, CFStringRef runLoopMode) {
	
	__CFNetServiceMonitor* monitor = (__CFNetServiceMonitor*)theMonitor;
	
	// Lock down the monitor before work
	__CFSpinLock(&(monitor->_lock));
	
	if (_SchedulesAddRunLoopAndMode(monitor->_schedules, runLoop, runLoopMode)) {

		// If there is a current monitor, need to schedule it.
		if (monitor->_trigger) {
			_CFTypeScheduleOnRunLoop(monitor->_trigger, runLoop, runLoopMode);
		}
	}
	
	// Unlock the monitor
	__CFSpinUnlock(&(monitor->_lock));
}


/* CF_EXPORT */ void
CFNetServiceMonitorUnscheduleFromRunLoop(CFNetServiceMonitorRef theMonitor, CFRunLoopRef runLoop, CFStringRef runLoopMode) {
	
	__CFNetServiceMonitor* monitor = (__CFNetServiceMonitor*)theMonitor;

	// Lock down the monitor before work
	__CFSpinLock(&(monitor->_lock));
	
	if (_SchedulesRemoveRunLoopAndMode(monitor->_schedules, runLoop, runLoopMode)) {

		// If there is a current monitor, need to unschedule it.
		if (monitor->_trigger) {
			_CFTypeUnscheduleFromRunLoop(monitor->_trigger, runLoop, runLoopMode);
		}
    }

	// Unlock the monitor
	__CFSpinUnlock(&(monitor->_lock));
}

