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
 *  CFNetServiceBrowser.c
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on Sun Apr 18 2004.
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


#if 0
#pragma mark -
#pragma mark Extern Function Declarations
#endif

extern CFNetServiceRef _CFNetServiceCreateCommon(CFAllocatorRef alloc, CFStringRef domain, CFStringRef type, CFStringRef name, UInt32 port);


#if 0
#pragma mark -
#pragma mark Constant Strings
#endif

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFNetServiceBrowserBlockingMode	CFSTR("_kCFNetServiceBrowserBlockingMode")
#else
static CONST_STRING_DECL(_kCFNetServiceBrowserBlockingMode, "_kCFNetServiceBrowserBlockingMode")
#endif	/* __CONSTANT_CFSTRINGS__ */

static const char _kCFNetServiceBrowserClassName[] = "CFNetServiceBrowser";


#if 0
#pragma mark -
#pragma mark CFNetServiceBrowser struct
#endif

typedef struct {

	CFRuntimeBase						_base;
	
	CFSpinLock_t						_lock;

	Boolean								_domainSearch;
	CFStreamError						_error;

	CFTypeRef							_trigger;
	DNSServiceRef						_browse;
	
	CFMutableDictionaryRef				_found;			// All the found services with ref counts
	CFMutableArrayRef					_adds;			// Items that are to be added
	CFMutableArrayRef					_removes;		// Items that are to be removed

	CFMutableArrayRef					_schedules;		// List of loops and modes
	CFNetServiceBrowserClientCallBack	_callback;
	CFNetServiceClientContext			_client;
} __CFNetServiceBrowser;


#if 0
#pragma mark -
#pragma mark Static Function Declarations
#endif

static void _CFNetServiceBrowserRegisterClass(void);

static void _NetServiceBrowserDestroy(__CFNetServiceBrowser* browser);

static void _BrowserCancel(__CFNetServiceBrowser* browser);
static Boolean _BrowserBlockUntilComplete(__CFNetServiceBrowser* browser);

static void _DomainEnumReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
							 DNSServiceErrorType errorCode, const char* replyDomain, void* context);
static void _BrowseReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
						 DNSServiceErrorType errorCode, const char* serviceName, const char* regtype,
						 const char* replyDomain, void* context);
	
static void _SocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info);


#if 0
#pragma mark -
#pragma mark Globals
#endif

static _CFOnceLock _kCFNetServiceBrowserRegisterClass = _CFOnceInitializer;
static CFTypeID _kCFNetServiceBrowserTypeID = _kCFRuntimeNotATypeID;
static CFRuntimeClass* _kCFNetServiceBrowserClass = NULL;


#if 0
#pragma mark -
#pragma mark Static Function Definitions
#endif

/* static */ void
_CFNetServiceBrowserRegisterClass(void) {
	
	_kCFNetServiceBrowserClass = (CFRuntimeClass*)calloc(1, sizeof(_kCFNetServiceBrowserClass[0]));
	
	if (_kCFNetServiceBrowserClass) {
		
		_kCFNetServiceBrowserClass->version = 0;
		_kCFNetServiceBrowserClass->className = _kCFNetServiceBrowserClassName;
		_kCFNetServiceBrowserClass->finalize = (void(*)(CFTypeRef))_NetServiceBrowserDestroy;
		
		_kCFNetServiceBrowserTypeID = _CFRuntimeRegisterClass(_kCFNetServiceBrowserClass);
	}
}


/* static */ void
_NetServiceBrowserDestroy(__CFNetServiceBrowser* browser) {
	
	// Prevent anything else from taking hold
	__CFSpinLock(&(browser->_lock));
	
	// Release the user's context info if there is some and a release method
	if (browser->_client.info && browser->_client.release)
		browser->_client.release(browser->_client.info);
	
	// Cancel the outstanding trigger
	if (browser->_trigger) {
		
		// Remove the trigger from run loops and modes
		if (browser->_schedules)
			_CFTypeUnscheduleFromMultipleRunLoops(browser->_trigger, browser->_schedules);
		
		// Go ahead and invalidate the trigger
		_CFTypeInvalidate(browser->_trigger);
		
		// Release the browse now.
		CFRelease(browser->_trigger);
	}
	
	// Need to clean up the service discovery stuff if there is
	if (browser->_browse) {
		
		// Release the underlying service discovery reference
		DNSServiceRefDeallocate(browser->_browse);
	}
	
	// Release the found list
	if (browser->_found)
		CFRelease(browser->_found);
	
	// Release the list of adds
	if (browser->_adds)
		CFRelease(browser->_adds);
	
	// Release the list of removes
	if (browser->_removes)
		CFRelease(browser->_removes);	
	
	// Release the list of loops and modes
	if (browser->_schedules)
		CFRelease(browser->_schedules);
}


/* static */ void
_BrowserCancel(__CFNetServiceBrowser* browser) {
	
	CFNetServiceBrowserClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	// Retain here to guarantee safety really after the browser release,
	// but definitely before the callback.
	CFRetain(browser);
	
	// Lock the browser
	__CFSpinLock(&browser->_lock);
	
	// If the browse canceled, don't need to do any of this.
	if (browser->_trigger) {
		
		// Save the callback if there is one at this time.
		cb = browser->_callback;
		
		// Save the error and client information for the callback
		memmove(&error, &(browser->_error), sizeof(error));
		info = browser->_client.info;
		
		// Remove the trigger from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(browser->_trigger, browser->_schedules);
		
		// Invalidate the run loop source that got here
		CFRunLoopSourceInvalidate((CFRunLoopSourceRef)(browser->_trigger));
		
		// Release the trigger now.
		CFRelease(browser->_trigger);
		browser->_trigger = NULL;
	}

	// Unlock the browser so the callback can be made safely.
	__CFSpinUnlock(&browser->_lock);
	
	// If there is a callback, inform the client of the finish.
	if (cb)
		cb((CFNetServiceBrowserRef)browser, 0, NULL, &error, info);
	
	// Go ahead and release now that the callback is done.
	CFRelease(browser);
}


/* static */ Boolean
_BrowserBlockUntilComplete(__CFNetServiceBrowser* browser) {
	
	// Assume success by default
	Boolean result = TRUE;
	CFRunLoopRef rl = CFRunLoopGetCurrent();
	
	// Schedule in the blocking mode.
	CFNetServiceBrowserScheduleWithRunLoop((CFNetServiceBrowserRef)browser, rl, _kCFNetServiceBrowserBlockingMode);
	
	// Lock in order to check for trigger
	__CFSpinLock(&(browser->_lock));
	
	// Check that search exists.
	while (browser->_trigger) {
		
		// Unlock again so the browser can continue to be processed.
		__CFSpinUnlock(&(browser->_lock));
		
		// Run the loop in a private mode with it returning whenever a source
		// has been handled.
		CFRunLoopRunInMode(_kCFNetServiceBrowserBlockingMode, DBL_MAX, TRUE);
		
		// Lock again in preparation for trigger check
		__CFSpinLock(&(browser->_lock));		
	}
	
	// Fail if there was an error.
	if (browser->_error.error)
		result = FALSE;
	
	// Unlock the browser again.
	__CFSpinUnlock(&(browser->_lock));
	
	// Unschedule from the blocking mode
	CFNetServiceBrowserUnscheduleFromRunLoop((CFNetServiceBrowserRef)browser, rl, _kCFNetServiceBrowserBlockingMode);
	
	return result;
}


/* static */ void
_DomainEnumReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
				 DNSServiceErrorType errorCode, const char* replyDomain, void* context)
{
	__CFNetServiceBrowser* browser = context;
	CFNetServiceBrowserClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	CFStringRef domain = NULL;
	
	// Retain here to guarantee safety really after the trigger release,
	// but definitely before the callback.
	CFRetain(browser);
	
	// Lock the browser
	__CFSpinLock(&browser->_lock);
	
	// If the browse canceled, don't need to do any of this.
	if (browser->_browse) {
		
		// If there is an error, fold the browse.
		if (errorCode) {
			
			// Save the error
			browser->_error.error = errorCode;
			browser->_error.domain = kCFStreamErrorDomainNetServices;
			
			// Remove the browse from run loops and modes
			_CFTypeUnscheduleFromMultipleRunLoops(browser->_trigger, browser->_schedules);
			
			// Go ahead and invalidate the socket
			CFSocketInvalidate((CFSocketRef)(browser->_trigger));
			
			// Release the browse now.
			CFRelease(browser->_trigger);
			browser->_trigger = NULL;
			
			// Clean up the underlying service discovery stuff
			DNSServiceRefDeallocate(browser->_browse);
			browser->_browse = NULL;
		}
		
		// If got a domain from service discovery, create the CFString for the domain.
		else if (replyDomain) {
			domain = CFStringCreateWithCString(CFGetAllocator(browser),
											   replyDomain,
											   kCFStringEncodingUTF8);
		}
		
		cb = browser->_callback;
		
		// Save the error and client information for the callback
		memmove(&error, &(browser->_error), sizeof(error));
		info = browser->_client.info;
		
	}
	
	// Unlock the browser so the callback can be made safely.
	__CFSpinUnlock(&browser->_lock);
	
	// If there is a callback, inform the client of the finish.
	if (cb && domain) {
		
		// Time to translate the service discovery flags into CFNetServices
		// flags.  This is known to be a domain, so start there.
		UInt32 f = kCFNetServiceFlagIsDomain;
		
		// If more is coming, set that bit.
		if (flags & kDNSServiceFlagsMoreComing)
			f |= kCFNetServiceFlagMoreComing;
			
		// SD notifies that it's adding.  CFNetServices needs to translate to
		// ones that are going away.
		if (!(flags & kDNSServiceFlagsAdd))
			f |= kCFNetServiceFlagRemove;
			
		// Set the bit if this is a registration domain
		if (flags & kDNSServiceFlagsDefault)
			f |= kCFNetServiceFlagIsDefault;
			
		// Inform the client.
		cb((CFNetServiceBrowserRef)browser, f, domain, &error, info);
	}
	
	// No longer need this after the callback
	if (domain)
		CFRelease(domain);
	
	// Go ahead and release now that the callback is done.
	CFRelease(browser);
}


/* static */ void
_BrowseReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
			 DNSServiceErrorType errorCode, const char* serviceName, const char* regtype,
			 const char* replyDomain, void* context)
{
	__CFNetServiceBrowser* browser = context;
	CFNetServiceBrowserClientCallBack cb = NULL;
	CFStreamError error = {0, 0};
	void* info = NULL;
	CFNetServiceRef service = NULL;
	
	// Retain here to guarantee safety really after the trigger release,
	// but definitely before the callback.
	CFRetain(browser);
	
	// Lock the browser
	__CFSpinLock(&browser->_lock);
	
	// If the browse canceled, don't need to do any of this.
	if (browser->_browse) {
		
		// If there is an error, fold the browse.
		if (errorCode) {
			
			// Save the error
			browser->_error.error = errorCode;
			browser->_error.domain = kCFStreamErrorDomainNetServices;
			
			// Remove the browse from run loops and modes
			_CFTypeUnscheduleFromMultipleRunLoops(browser->_trigger, browser->_schedules);
			
			// Go ahead and invalidate the socket
			CFSocketInvalidate((CFSocketRef)(browser->_trigger));
			
			// Release the browse now.
			CFRelease(browser->_trigger);
			browser->_trigger = NULL;
			
			// Clean up the underlying service discovery stuff
			DNSServiceRefDeallocate(browser->_browse);
			browser->_browse = NULL;
			
			// Dump all the lists of items.
			CFDictionaryRemoveAllValues(browser->_found);
			CFArrayRemoveAllValues(browser->_adds);
			CFArrayRemoveAllValues(browser->_removes);
		}
		
		// If got service info from service discovery, create the CFNetServiceRef.
		else if (serviceName && regtype && replyDomain) {
		
			// Create CFString's for each of the service components
			CFAllocatorRef alloc = CFGetAllocator(browser);
			CFStringRef domain = CFStringCreateWithCString(alloc, replyDomain, kCFStringEncodingUTF8);
			CFStringRef type = CFStringCreateWithCString(alloc, regtype, kCFStringEncodingUTF8);
			CFStringRef name = CFStringCreateWithCString(alloc, serviceName, kCFStringEncodingUTF8);
	
			// Can only make the service if all the strings were created.  This
			// will skip over items that are not properly UTF8 encoded on the wire.
			if (domain && type && name)
				service = _CFNetServiceCreateCommon(alloc, domain, type, name, 0);
			
			if (domain) CFRelease(domain);
			if (type) CFRelease(type);
			if (name) CFRelease(name);
			
			if (service) {
				
				UInt32 count = (UInt32)CFDictionaryGetValue(browser->_found, service);
				
				if (flags & kDNSServiceFlagsAdd) {
					
					count++;
					
					if (count != 1)
						CFDictionaryReplaceValue(browser->_found, service, (const void*)count);
					
					else {
						CFIndex i = CFArrayGetFirstIndexOfValue(browser->_removes,
																CFRangeMake(0, CFArrayGetCount(browser->_removes)),
																service);
						
						CFDictionaryAddValue(browser->_found, service, (const void*)count);
						CFArrayAppendValue(browser->_adds, service);
						
						if (i != kCFNotFound)
							CFArrayRemoveValueAtIndex(browser->_removes, i);
					}
				}
				
				else {
					
					count--;
					if (count > 0)
						CFDictionaryReplaceValue(browser->_found, service, (const void*)count);
					else {
						CFIndex i = CFArrayGetFirstIndexOfValue(browser->_adds,
																CFRangeMake(0, CFArrayGetCount(browser->_adds)),
																service);

						CFDictionaryRemoveValue(browser->_found, service);
						CFArrayAppendValue(browser->_removes, service);
						
						if (i != kCFNotFound)
							CFArrayRemoveValueAtIndex(browser->_adds, i);
					}
				}
				
				CFRelease(service);
			}
		}
		
		cb = browser->_callback;
		
		// Save the error and client information for the callback
		memmove(&error, &(browser->_error), sizeof(error));
		info = browser->_client.info;
		
	}
	
	// If there is a callback, inform the client of the error.
	if (cb && error.error) {
		
		// Unlock the browser so the callback can be made safely.
		__CFSpinUnlock(&browser->_lock);
		
		cb((CFNetServiceBrowserRef)browser, 0, NULL, &error, info);
	}

	else if (cb && ((flags & kDNSServiceFlagsMoreComing) == 0)) {
		
		CFIndex i, adds = CFArrayGetCount(browser->_adds);
		CFIndex removes = CFArrayGetCount(browser->_removes);
		CFIndex total = adds + removes;
		
		for (i = 0; i < adds; i++) {
			
			const void* saved = browser->_trigger;
			service = (CFNetServiceRef)CFArrayGetValueAtIndex(browser->_adds, i);
			
			// Unlock the browser so the callback can be made safely.
			__CFSpinUnlock(&browser->_lock);
			
			cb((CFNetServiceBrowserRef)browser,
			   (i == (total - 1)) ? 0 : kCFNetServiceFlagMoreComing, 
			   service,
			   &error,
			   info);
			
			// Lock the browser
			__CFSpinLock(&browser->_lock);
			
			if (saved != browser->_trigger) {
				cb = NULL;
				break;
			}
		}
		
		if (cb) {
			for (i = 0; i < removes; i++) {
				
				const void* saved = browser->_trigger;
				service = (CFNetServiceRef)CFArrayGetValueAtIndex(browser->_removes, i);
				
				// Unlock the browser so the callback can be made safely.
				__CFSpinUnlock(&browser->_lock);
				
				cb((CFNetServiceBrowserRef)browser,
				   kCFNetServiceFlagRemove | ((i == (removes - 1)) ? 0 : kCFNetServiceFlagMoreComing), 
				   service,
				   &error,
				   info);
				
				// Lock the browser
				__CFSpinLock(&browser->_lock);
				
				if (saved != browser->_trigger)
					break;
			}
		}
		
		// Dump the lists of items, so can start new again.
		CFArrayRemoveAllValues(browser->_adds);
		CFArrayRemoveAllValues(browser->_removes);
		
		// Unlock the browser so the callback can be made safely.
		__CFSpinUnlock(&browser->_lock);
	}
	else
		__CFSpinUnlock(&browser->_lock);
	
	// Go ahead and release now that the callback is done.
	CFRelease(browser);
}


/* static */ void
_SocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void* data, void* info) {
	
	DNSServiceErrorType err;
	__CFNetServiceBrowser* browser = info;
	
	(void)s;		// unused
	(void)type;		// unused
	(void)address;  // unused
	(void)data;		// unused
	
	CFRetain(browser);
	
	// Dispatch to process the result
	err = DNSServiceProcessResult(browser->_browse);
	
	// If there was an error, need to infor the client.
	if (err) {
		
		// Dispatch based upon search type.
		if (browser->_domainSearch)
			_DomainEnumReply(browser->_browse, 0, 0, err, NULL, info);
		else
			_BrowseReply(browser->_browse, 0, 0, err, NULL, NULL, NULL, info);
	}
	
	CFRelease(browser);
}


#if 0
#pragma mark -
#pragma mark Extern Function Definitions (API)
#endif


/* CF_EXPORT */ CFTypeID
CFNetServiceBrowserGetTypeID(void) {

    _CFDoOnce(&_kCFNetServiceBrowserRegisterClass, _CFNetServiceBrowserRegisterClass);

    return _kCFNetServiceBrowserTypeID;
}

/* CF_EXPORT */ CFNetServiceBrowserRef
CFNetServiceBrowserCreate(CFAllocatorRef alloc, CFNetServiceBrowserClientCallBack clientCB, CFNetServiceClientContext* context) {
	
	__CFNetServiceBrowser* result = NULL;
	
	if (clientCB && context) {
		
		CFTypeID class_type = CFNetServiceBrowserGetTypeID();
	
		if (class_type != _kCFRuntimeNotATypeID) {
			result = (__CFNetServiceBrowser*)_CFRuntimeCreateInstance(alloc,
																	  class_type,
																	  sizeof(result[0]) - sizeof(CFRuntimeBase),
																	  NULL);
		}

		if (result) {
			
			CFDictionaryValueCallBacks values = {0, NULL, NULL, NULL, NULL};
			
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
			
			// List of "unique" services found on the network.  value is the ref count for the service
			result->_found = CFDictionaryCreateMutable(alloc, 0, &kCFTypeDictionaryKeyCallBacks, &values);
			
			// Create list of items to be added
			result->_adds = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
			
			// Create list of items to be removed
			result->_removes = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
			
			// If any failed, need to release and return null
			if (!result->_schedules) {
				CFRelease((CFTypeRef)result);
				result = NULL;
			}
		}
	}
	
	return (CFNetServiceBrowserRef)result;
}


/* CF_EXPORT */ void
CFNetServiceBrowserInvalidate(CFNetServiceBrowserRef b) {
	
	__CFNetServiceBrowser* browser = (__CFNetServiceBrowser*)b;
	
	// Lock the browser
	__CFSpinLock(&(browser->_lock));
	
	// Release the user's context info if there is some and a release method
	if (browser->_client.info && browser->_client.release)
		browser->_client.release(browser->_client.info);
	
	// Cancel the outstanding trigger
	if (browser->_trigger) {
		
		// Remove the trigger from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(browser->_trigger, browser->_schedules);
		
		// Go ahead and invalidate the trigger
		_CFTypeInvalidate(browser->_trigger);
		
		// Release the browse now.
		CFRelease(browser->_trigger);
		browser->_trigger = NULL;
	}
	
	// Need to clean up the service discovery stuff if there is
	if (browser->_browse) {
		
		// Release the underlying service discovery reference
		DNSServiceRefDeallocate(browser->_browse);
		browser->_browse = NULL;
		
		// Dump all the lists of items.
		CFDictionaryRemoveAllValues(browser->_found);
		CFArrayRemoveAllValues(browser->_adds);
		CFArrayRemoveAllValues(browser->_removes);
	}
	
	// Zero out the callback and client context.
	browser->_callback = NULL;
	memset(&(browser->_client), 0, sizeof(browser->_client));

	// Unlock the browser.
	__CFSpinUnlock(&(browser->_lock));
}


/* CF_EXPORT */ Boolean
CFNetServiceBrowserSearchForDomains(CFNetServiceBrowserRef b, Boolean registrationDomains, CFStreamError* error) {
	
	__CFNetServiceBrowser* browser = (__CFNetServiceBrowser*)b;
	
	CFStreamError extra;
	Boolean result = FALSE;
	
	if (!error)
		error = &extra;
	
	memset(error, 0, sizeof(error[0]));
	
	// Retain so it doesn't go away underneath in the case of a callout.  This is really
	// no worry for async, but makes the memmove for the error more difficult to place
	// for synchronous without it being here.
	CFRetain(browser);
	
	// Lock down the browser to start search
	__CFSpinLock(&(browser->_lock));
	
	do {
		
		CFSocketContext ctxt = {0, browser, CFRetain, CFRelease, NULL};
		
		if (!browser->_callback) {
			browser->_error.error = kCFNetServicesErrorInvalid;
			browser->_error.domain = kCFStreamErrorDomainNetServices;
			break;
		}
		
		// Check to see if there is an ongoing search already
		if (browser->_trigger) {
		
			// If it's a mdns search, don't allow another.
			if (CFGetTypeID(browser->_trigger) == CFSocketGetTypeID()) {
				browser->_error.error = kCFNetServicesErrorInProgress;
				browser->_error.domain = kCFStreamErrorDomainNetServices;
				break;
			}
			
			// It's just the cancel that hasn't fired yet, so cancel it.
			else {
				
				// Remove the trigger from run loops and modes
				_CFTypeUnscheduleFromMultipleRunLoops(browser->_trigger, browser->_schedules);
				
				// Invalidate the run loop source
				CFRunLoopSourceInvalidate((CFRunLoopSourceRef)(browser->_trigger));
				
				// Release the trigger now.
				CFRelease(browser->_trigger);
				browser->_trigger = NULL;
			}
		}
		
		browser->_domainSearch = TRUE;
		
		// Create the domain search at the service discovery level
		browser->_error.error = DNSServiceEnumerateDomains(&browser->_browse,
														   registrationDomains ? kDNSServiceFlagsRegistrationDomains : kDNSServiceFlagsBrowseDomains,
														   0, 
														   _DomainEnumReply,
														   browser);
		
		// Fail if it did.
		if (browser->_error.error) {
			browser->_error.domain = kCFStreamErrorDomainNetServices;
			break;
		}
		
		// Create the trigger for the browse
		browser->_trigger = CFSocketCreateWithNative(CFGetAllocator(browser),
													 DNSServiceRefSockFD(browser->_browse),
													 kCFSocketReadCallBack,
													 _SocketCallBack,
													 &ctxt);
		
		// Make sure the CFSocket wrapper succeeded
		if (!browser->_trigger) {
			
			// Try to use errno for the error
			browser->_error.error = errno;
			
			// If it has no error in it, assume no memory
			if (!browser->_error.error)
				browser->_error.error = ENOMEM;
			
			// Correct domain and bail.
			browser->_error.domain = kCFStreamErrorDomainPOSIX;
			
			DNSServiceRefDeallocate(browser->_browse);
			browser->_browse = NULL;
			
			break;
		}
		
		// Tell CFSocket not to close the native socket on invalidation.
		CFSocketSetSocketFlags((CFSocketRef)browser->_trigger,
							   CFSocketGetSocketFlags((CFSocketRef)browser->_trigger) & ~kCFSocketCloseOnInvalidate);
		
		// Async mode is complete at this point
		if (CFArrayGetCount(browser->_schedules)) {
			
			// Schedule the trigger on the run loops and modes.
			_CFTypeScheduleOnMultipleRunLoops(browser->_trigger, browser->_schedules);
			
			// It's now succeeded.
			result = TRUE;
		}
		
		// If there is no callback, go into synchronous mode.
		else {
			
			// Unlock the browser
			__CFSpinUnlock(&(browser->_lock));
			
			// Wait for synchronous return
			result = _BrowserBlockUntilComplete(browser);
			
			// Lock down the browser
			__CFSpinLock(&(browser->_lock));
		}
		
	} while (0);
	
	// Copy the error.
	memmove(error, &browser->_error, sizeof(error[0]));
	
	// Unlock the browser
	__CFSpinUnlock(&(browser->_lock));
	
	// Release the earlier retain.
	CFRelease(browser);
	
	return result;
}


/* CF_EXPORT */ Boolean
CFNetServiceBrowserSearchForServices(CFNetServiceBrowserRef b, CFStringRef domain, CFStringRef type, CFStreamError* error) {
	
	__CFNetServiceBrowser* browser = (__CFNetServiceBrowser*)b;
	
	CFStreamError extra;
	Boolean result = FALSE;
	
	if (!error)
		error = &extra;
	
	memset(error, 0, sizeof(error[0]));
	
	// Retain so it doesn't go away underneath in the case of a callout.  This is really
	// no worry for async, but makes the memmove for the error more difficult to place
	// for synchronous without it being here.
	CFRetain(browser);
	
	// Lock down the browser to start search
	__CFSpinLock(&(browser->_lock));
	
	do {
        
		int i;
        char properties[2][1024];
        CFStringRef argProperties[] = {type, domain};		
		CFSocketContext ctxt = {0, browser, CFRetain, CFRelease, NULL};
		
		if (!browser->_callback) {
			browser->_error.error = kCFNetServicesErrorInvalid;
			browser->_error.domain = kCFStreamErrorDomainNetServices;
			break;
		}
		
		// Check to see if there is an ongoing search already
		if (browser->_trigger) {
		
			// If it's a mdns search, don't allow another.
			if (CFGetTypeID(browser->_trigger) == CFSocketGetTypeID()) {
				browser->_error.error = kCFNetServicesErrorInProgress;
				browser->_error.domain = kCFStreamErrorDomainNetServices;
				break;
			}
			
			// It's just the cancel that hasn't fired yet, so cancel it.
			else {
				
				// Remove the trigger from run loops and modes
				_CFTypeUnscheduleFromMultipleRunLoops(browser->_trigger, browser->_schedules);
				
				// Invalidate the run loop source
				CFRunLoopSourceInvalidate((CFRunLoopSourceRef)(browser->_trigger));
				
				// Release the trigger now.
				CFRelease(browser->_trigger);
				browser->_trigger = NULL;
			}
		}
        
		// Convert the type and domain to c strings to pass down
        for (i = 0; i < (sizeof(properties) / sizeof(properties[0])); i++) {
        
            if (!argProperties[i])
                properties[i][0] = '\0';
            else {
                CFIndex bytesUsed;
            
                CFStringGetBytes(argProperties[i],
                                CFRangeMake(0, CFStringGetLength(argProperties[i])),
                                kCFStringEncodingUTF8,
                                0,
                                FALSE,
                                (UInt8*)properties[i],
                                sizeof(properties[i]) - 1,
                                &bytesUsed);
                properties[i][bytesUsed] = '\0';
            }
        }
		
		browser->_domainSearch = FALSE;
		
		// Create the service search at the service discovery level
		browser->_error.error = DNSServiceBrowse(&browser->_browse,
												 0,
												 0,
												 properties[0],
												 properties[1],
												 _BrowseReply,
												 browser);
		
		// Fail if it did.
		if (browser->_error.error) {
			browser->_error.domain = kCFStreamErrorDomainNetServices;
			break;
		}
		
		// Create the trigger for the browse
		browser->_trigger = CFSocketCreateWithNative(CFGetAllocator(browser),
													 DNSServiceRefSockFD(browser->_browse),
													 kCFSocketReadCallBack,
													 _SocketCallBack,
													 &ctxt);
		
		// Make sure the CFSocket wrapper succeeded
		if (!browser->_trigger) {
			
			// Try to use errno for the error
			browser->_error.error = errno;
			
			// If it has no error in it, assume no memory
			if (!browser->_error.error)
				browser->_error.error = ENOMEM;
			
			// Correct domain and bail.
			browser->_error.domain = kCFStreamErrorDomainPOSIX;
			
			DNSServiceRefDeallocate(browser->_browse);
			browser->_browse = NULL;
			
			break;
		}
		
		// Tell CFSocket not to close the native socket on invalidation.
		CFSocketSetSocketFlags((CFSocketRef)browser->_trigger,
							   CFSocketGetSocketFlags((CFSocketRef)browser->_trigger) & ~kCFSocketCloseOnInvalidate);
		
		// Async mode is complete at this point
		if (CFArrayGetCount(browser->_schedules)) {
			
			// Schedule the trigger on the run loops and modes.
			_CFTypeScheduleOnMultipleRunLoops(browser->_trigger, browser->_schedules);
			
			// It's now succeeded.
			result = TRUE;
		}
		
		// If there is no callback, go into synchronous mode.
		else {
			
			// Unlock the browser
			__CFSpinUnlock(&(browser->_lock));
			
			// Wait for synchronous return
			result = _BrowserBlockUntilComplete(browser);
			
			// Lock down the browser
			__CFSpinLock(&(browser->_lock));
		}
		
	} while (0);
	
	// Copy the error.
	memmove(error, &browser->_error, sizeof(error[0]));
	
	// Unlock the browser
	__CFSpinUnlock(&(browser->_lock));
	
	// Release the earlier retain.
	CFRelease(browser);
	
	return result;
}


/* CF_EXPORT */ void
CFNetServiceBrowserStopSearch(CFNetServiceBrowserRef b, CFStreamError* error) {
	
	__CFNetServiceBrowser* browser = (__CFNetServiceBrowser*)b;
	
	// By default, the error is marked as a cancel
	CFStreamError extra = {kCFStreamErrorDomainNetServices , kCFNetServicesErrorCancel};
	
	// Make sure error has a value.
	if (!error)
		error = &extra;
	
	// Lock down the browser
	__CFSpinLock(&(browser->_lock));
	
	// Make sure there is something to cancel.
	if (browser->_trigger) {

		CFRunLoopSourceContext ctxt = {
			0,									// version
			browser,							// info
			NULL,								// retain
			NULL,								// release
			NULL,								// copyDescription
			NULL,								// equal
			NULL,								// hash
			NULL,								// schedule
			NULL,								// cancel
			(void(*)(void*))(&_BrowserCancel)  // perform
		};
		
		// Remove the trigger from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(browser->_trigger, browser->_schedules);
		
		// Go ahead and invalidate the trigger
		_CFTypeInvalidate(browser->_trigger);
		
		// Release the trigger now.
		CFRelease(browser->_trigger);
		
		// Need to clean up the service discovery stuff if there is
		if (browser->_browse) {
			
			// Release the underlying service discovery reference
			DNSServiceRefDeallocate(browser->_browse);
			browser->_browse = NULL;
			
			// Dump all the lists of items.
			CFDictionaryRemoveAllValues(browser->_found);
			CFArrayRemoveAllValues(browser->_adds);
			CFArrayRemoveAllValues(browser->_removes);
		}
		
		// Copy the error into place
		memmove(&(browser->_error), error, sizeof(error[0]));
		
		// Create the cancel source
		browser->_trigger = CFRunLoopSourceCreate(CFGetAllocator(browser), 0, &ctxt);
		
		// If the cancel was created, need to schedule and signal it.
		if (browser->_trigger) {
			
			CFArrayRef schedules = browser->_schedules;
			CFIndex i, count = CFArrayGetCount(schedules);
			
			// Schedule the new trigger
			_CFTypeScheduleOnMultipleRunLoops(browser->_trigger, schedules);
			
			// Signal the cancel for immediate attention.
			CFRunLoopSourceSignal((CFRunLoopSourceRef)(browser->_trigger));
			
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
						if (CFRunLoopContainsSource(runloop, (CFRunLoopSourceRef)(browser->_trigger), mode)) {
							CFRunLoopWakeUp(runloop);
						}
						
						// Don't need this anymore.
						CFRelease(mode);
					}
				}
			}
		}
	}
	
	// Unlock the browser
	__CFSpinUnlock(&(browser->_lock));
}


/* CF_EXPORT */ void
CFNetServiceBrowserScheduleWithRunLoop(CFNetServiceBrowserRef b, CFRunLoopRef runLoop, CFStringRef runLoopMode) {
	
	__CFNetServiceBrowser* browser = (__CFNetServiceBrowser*)b;
	
	// Lock down the browser before work
	__CFSpinLock(&(browser->_lock));
	
	if (_SchedulesAddRunLoopAndMode(browser->_schedules, runLoop, runLoopMode)) {

		// If there is a current browse, need to schedule it.
		if (browser->_trigger) {
			_CFTypeScheduleOnRunLoop(browser->_trigger, runLoop, runLoopMode);
		}
	}
	
	// Unlock the browser
	__CFSpinUnlock(&(browser->_lock));
}


/* CF_EXPORT */ void
CFNetServiceBrowserUnscheduleFromRunLoop(CFNetServiceBrowserRef b, CFRunLoopRef runLoop, CFStringRef runLoopMode) {
	
	__CFNetServiceBrowser* browser = (__CFNetServiceBrowser*)b;

	// Lock down the browser before work
	__CFSpinLock(&(browser->_lock));
	
	if (_SchedulesRemoveRunLoopAndMode(browser->_schedules, runLoop, runLoopMode)) {

		// If there is a current browse, need to unschedule it.
		if (browser->_trigger) {
			_CFTypeUnscheduleFromRunLoop(browser->_trigger, runLoop, runLoopMode);
		}
    }

	// Unlock the browser
	__CFSpinUnlock(&(browser->_lock));
}

