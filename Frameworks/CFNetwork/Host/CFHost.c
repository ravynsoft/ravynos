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
 *  CFHost.cpp
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on Thu Nov 28 2002.
 *  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 */

#if 0
#pragma mark Description
#endif

/*
	CFHost is built as a CFRuntimeBase object.  The actual registration of the class type
	takes place when the first call for the type id is made (CFHostGetTypeID).  The object
	instantiation functions use this call for creation, therefore any of the creators will
	cause registration of the class.
 
	CFHost's underlying lookups can be any asynchronous CFType (i.e. CFMachPort, CFSocket,
	SCNetworkReachability, etc.).  The lookup should be created and scheduled on the run
	loops and modes saved in the "schedules" array.  The array is maintained in order to
	allow scheduling separate from the lookup.  With this, lookup can be created after
	schedules have been placed on the object.  The lookup can then be scheduled the same
	as the object.  The schedules array contains a list of pairs of run loops and modes
	(e.g. [<rl1>, <mode1>, <rl2>, <mode2>, ...]).  There can be zero or more items in
	the array, but the count should always be divisible by 2.

	A cancel is just another type of lookup.  A custom CFRunLoopSource is created which
	is simply signalled instantly.  This will cause synchronous lookups on other run loops
	(threads) to cancel out immediately.
 
	All resolved information is stored in a dictionary on the host object.  The key is the
	CFHostInfoType with the value being specific to the type.  Value types should be
	documented with the CFHostInfoType declarations.  In the case where a lookup produces
	no data, kCFNull should be used for the value of the type.  This distinguishes the
	lookup as being performed and returning no data, which is different from not ever
	performing the lookup.
	
	Duplicate suppression is performed for hostname lookups.  The first hostname lookup
	that is performed creates a "master" lookup.  The master is just another CFHostRef
	whose lookup is started as a special info type.  This signals to it that it is the
	master and that there are clients of it.  The master is then placed in a global dictionary
	of outstanding lookups.  When a second is started, it is checked for existance in the
	global list.  If/When found, the second request is added to the list of clients.  The
	master lookup is scheduled on all loops and modes as the list of clients.  When the
	master lookup completes, all clients in the list are informed.  If all clients cancel,
	the master lookup will be canceled and removed from the master lookups list.
*/


#if 0
#pragma mark -
#pragma mark Includes
#endif
#include <CFNetwork/CFNetwork.h>
#include <CFNetwork/CFNetworkPriv.h>
#include "CFNetworkInternal.h"							/* for __CFSpinLock and __CFSpinUnlock */
#include "CFNetworkSchedule.h"

#include <math.h>										/* for fabs */
#include <sys/socket.h>
#include <netdb_async.h>
#include <SystemConfiguration/SystemConfiguration.h>	/* for SCNetworkReachability and flags */


#if 0
#pragma mark -
#pragma mark Constants
#endif

/* extern */ const SInt32 kCFStreamErrorDomainNetDB = 12;
/* extern */ const SInt32 kCFStreamErrorDomainSystemConfiguration = 13;

#define _kCFNullHostInfoType				((CFHostInfoType)0xFFFFFFFF)

#define _kCFHostIPv4Addresses				((CFHostInfoType)0x0000FFFE)
#define _kCFHostIPv6Addresses				((CFHostInfoType)0x0000FFFD)
#define _kCFHostMasterAddressLookup			((CFHostInfoType)0x0000FFFC)
#define _kCFHostByPassMasterAddressLookup	((CFHostInfoType)0x0000FFFB)

#define _kCFHostCacheMaxEntries				25
#define _kCFHostCacheTimeout				((CFTimeInterval)1.0)


#if 0
#pragma mark -
#pragma mark Constant Strings
#endif

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFHostBlockingMode	CFSTR("_kCFHostBlockingMode")
#define _kCFHostDescribeFormat	CFSTR("<CFHost 0x%x>{info=%@}")
#else
static CONST_STRING_DECL(_kCFHostBlockingMode, "_kCFHostBlockingMode")
static CONST_STRING_DECL(_kCFHostDescribeFormat, "<CFHost 0x%x>{info=%@}")
#endif	/* __CONSTANT_CFSTRINGS__ */


#if 0
#pragma mark -
#pragma mark CFHost struct
#endif

typedef struct {

	CFRuntimeBase 			_base;
	
	CFSpinLock_t			_lock;

	CFStreamError			_error;

	CFMutableDictionaryRef	_info;

	//CFMutableDictionaryRef  _lookups;		// key = CFHostInfoType and value = CFTypeRef
	CFTypeRef				_lookup;
	CFHostInfoType			_type;

	CFMutableArrayRef		_schedules;		// List of loops and modes
	CFHostClientCallBack	_callback;
	CFHostClientContext		_client;
} _CFHost;


#if 0
#pragma mark -
#pragma mark Static Function Declarations
#endif

static void _CFHostRegisterClass(void);
static _CFHost* _HostCreate(CFAllocatorRef allocator);

static void _HostDestroy(_CFHost* host);
static CFStringRef _HostDescribe(_CFHost* host);

static void _HostCancel(_CFHost* host);

static Boolean _HostBlockUntilComplete(_CFHost* host);

static Boolean _CreateLookup_NoLock(_CFHost* host, CFHostInfoType info, Boolean* _Radar4012176);

static CFMachPortRef _CreateMasterAddressLookup(CFStringRef name, CFHostInfoType info, CFTypeRef context, CFStreamError* error);
static CFTypeRef _CreateAddressLookup(CFStringRef name, CFHostInfoType info, void* context, CFStreamError* error);
static CFMachPortRef _CreateNameLookup(CFDataRef address, void* context, CFStreamError* error);
static SCNetworkReachabilityRef _CreateReachabilityLookup(CFTypeRef thing, void* context, CFStreamError* error);
static CFMachPortRef _CreateDNSLookup(CFTypeRef thing, CFHostInfoType type, void* context, CFStreamError* error);

static void _GetAddrInfoCallBack(int32_t status, struct addrinfo* res, void* ctxt);
static void _GetAddrInfoMachPortCallBack(CFMachPortRef port, void* msg, CFIndex size, void* info);

static void _GetNameInfoCallBack(int32_t status, char *hostname, char *serv, void* ctxt);
static void _GetNameInfoMachPortCallBack(CFMachPortRef port, void* msg, CFIndex size, void* info);

static void _NetworkReachabilityCallBack(SCNetworkReachabilityRef target, SCNetworkConnectionFlags flags, void* ctxt);
static void _NetworkReachabilityByIPCallBack(_CFHost* host);

static void _DNSCallBack(int32_t status, char *buf, uint32_t len, struct sockaddr *from, int fromlen, void *context);
static void _DNSMachPortCallBack(CFMachPortRef port, void* msg, CFIndex size, void* info);

static void _MasterCallBack(CFHostRef theHost, CFHostInfoType typeInfo, const CFStreamError *error, CFStringRef name);
static void _AddressLookupSchedule_NoLock(_CFHost* host, CFRunLoopRef rl, CFStringRef mode);
static void _AddressLookupPerform(_CFHost* host);

static void _ExpireCacheEntries(void);

static CFArrayRef _CFArrayCreateDeepCopy(CFAllocatorRef alloc, CFArrayRef array);
static Boolean _IsDottedIp(CFStringRef name);


#if 0
#pragma mark -
#pragma mark Globals
#endif

static _CFOnceLock _kCFHostRegisterClass = _CFOnceInitializer;
static CFTypeID _kCFHostTypeID = _kCFRuntimeNotATypeID;

static _CFMutex* _HostLock;						/* Lock used for cache and master list */
static CFMutableDictionaryRef _HostLookups;		/* Active hostname lookups; for duplicate supression */
static CFMutableDictionaryRef _HostCache;		/* Cached hostname lookups (successes only) */


#if 0
#pragma mark -
#pragma mark Static Function Definitions
#endif

/* static */ void
_CFHostRegisterClass(void) {
	
	static const CFRuntimeClass _kCFHostClass = {
		0,												// version
		"CFHost",										// class name
		NULL,      										// init
		NULL,      										// copy
		(void(*)(CFTypeRef))_HostDestroy,				// dealloc
		NULL,      										// equal
		NULL,      										// hash
		NULL,      										// copyFormattingDesc
		(CFStringRef(*)(CFTypeRef cf))_HostDescribe		// copyDebugDesc
	};
	
	
    _kCFHostTypeID = _CFRuntimeRegisterClass(&_kCFHostClass);

	/* Set up the "master" for simultaneous, duplicate lookups. */
	_HostLock = (_CFMutex*)CFAllocatorAllocate(kCFAllocatorDefault, sizeof(_HostLock[0]), 0);
	if (_HostLock) _CFMutexInit(_HostLock, FALSE);
	_HostLookups = CFDictionaryCreateMutable(kCFAllocatorDefault,
													0,
													&kCFTypeDictionaryKeyCallBacks,
													&kCFTypeDictionaryValueCallBacks);
	
	_HostCache = CFDictionaryCreateMutable(kCFAllocatorDefault,
										   0,
										   &kCFTypeDictionaryKeyCallBacks,
										   &kCFTypeDictionaryValueCallBacks);
}


/* static */ _CFHost*
_HostCreate(CFAllocatorRef allocator) {

	CFDictionaryKeyCallBacks keys = {0, NULL, NULL, NULL, NULL, NULL};
	
	_CFHost* result = (_CFHost*)_CFRuntimeCreateInstance(allocator,
														   CFHostGetTypeID(),
														   sizeof(result[0]) - sizeof(CFRuntimeBase),
														   NULL);

	if (result) {

		// Save a copy of the base so it's easier to zero the struct
		CFRuntimeBase copy = result->_base;

		// Clear everything.
		memset(result, 0, sizeof(result[0]));

		// Put back the base
		memmove(&(result->_base), &copy, sizeof(result->_base));

		// No lookup by default
		result->_type = _kCFNullHostInfoType;		

		// Create the dictionary of lookup information
		result->_info = CFDictionaryCreateMutable(allocator, 0, &keys, &kCFTypeDictionaryValueCallBacks);

		// Create the list of loops and modes
		result->_schedules = CFArrayCreateMutable(allocator, 0, &kCFTypeArrayCallBacks);

		// If any failed, need to release and return null
		if (!result->_info || !result->_schedules) {
			CFRelease((CFTypeRef)result);
			result = NULL;
		}
	}

	return result;
}


/* static */ void
_HostDestroy(_CFHost* host) {
	
	// Prevent anything else from taking hold
	__CFSpinLock(&(host->_lock));
	
	// Release the user's context info if there is some and a release method
	if (host->_client.info && host->_client.release)
		host->_client.release(host->_client.info);
	
	// If there is a lookup, release it.
	if (host->_lookup) {
		
		// Remove the lookup from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(host->_lookup, host->_schedules);
		
		// Go ahead and invalidate the lookup
		_CFTypeInvalidate(host->_lookup);
		
		// Release the lookup now.
		CFRelease(host->_lookup);
	}
	
	// Release any gathered information
	if (host->_info)
		CFRelease(host->_info);

	// Release the list of loops and modes
	if (host->_schedules)
		CFRelease(host->_schedules);
}


/* static */ CFStringRef
_HostDescribe(_CFHost* host) {
	
	CFStringRef result;
	
	__CFSpinLock(&host->_lock);
	
	result = CFStringCreateWithFormat(CFGetAllocator((CFHostRef)host),
									  NULL,
									  _kCFHostDescribeFormat,
									  host,
									  host->_info);
	
	__CFSpinUnlock(&host->_lock);
	
	return result;
}


/* static */ void
_HostCancel(_CFHost* host) {
	
	CFHostClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	CFHostInfoType type = _kCFNullHostInfoType;
	
	// Retain here to guarantee safety really after the lookups release,
	// but definitely before the callback.
	CFRetain((CFHostRef)host);
	
	// Lock the host
	__CFSpinLock(&host->_lock);
	
	// If the lookup canceled, don't need to do any of this.
	if (host->_lookup) {
		
		// Save the callback if there is one at this time.
		cb = host->_callback;
		
		// Save the type of lookup for the callback.
		type = host->_type;
		
		// Save the error and client information for the callback
		memmove(&error, &(host->_error), sizeof(error));
		info = host->_client.info;
		
		// Remove the lookup from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(host->_lookup, host->_schedules);
		
		// Invalidate the run loop source that got here
		CFRunLoopSourceInvalidate((CFRunLoopSourceRef)(host->_lookup));
		
		// Release the lookup now.
		CFRelease(host->_lookup);
		host->_lookup = NULL;
		host->_type = _kCFNullHostInfoType;
	}
	
	// Unlock the host so the callback can be made safely.
	__CFSpinUnlock(&host->_lock);
	
	// If there is a callback, inform the client of the finish.
	if (cb)
		cb((CFHostRef)host, type, &error, info);
	
	// Go ahead and release now that the callback is done.
	CFRelease((CFHostRef)host);
}


/* static */ Boolean
_HostBlockUntilComplete(_CFHost* host) {
	
	// Assume success by default
	Boolean result = TRUE;
	CFRunLoopRef rl = CFRunLoopGetCurrent();
	
	// Schedule in the blocking mode.
	CFHostScheduleWithRunLoop((CFHostRef)host, rl, _kCFHostBlockingMode);
	
	// Lock in order to check for lookup
	__CFSpinLock(&(host->_lock));
	
	// Check that lookup exists.
	while (host->_lookup) {
		
		// Unlock again so the host can continue to be processed.
		__CFSpinUnlock(&(host->_lock));
		
		// Run the loop in a private mode with it returning whenever a source
		// has been handled.
		CFRunLoopRunInMode(_kCFHostBlockingMode, DBL_MAX, TRUE);
		
		// Lock again in preparation for lookup check
		__CFSpinLock(&(host->_lock));		
	}
	
	// Fail if there was an error.
	if (host->_error.error)
		result = FALSE;
	
	// Unlock the host again.
	__CFSpinUnlock(&(host->_lock));
	
	// Unschedule from the blocking mode
	CFHostUnscheduleFromRunLoop((CFHostRef)host, rl, _kCFHostBlockingMode);
	
	return result;
}


/* static */ Boolean
_CreateLookup_NoLock(_CFHost* host, CFHostInfoType info, Boolean* _Radar4012176) {

	Boolean result = FALSE;

	// Get the existing names and addresses
	CFArrayRef names = (CFArrayRef)CFDictionaryGetValue(host->_info, (const void*)kCFHostNames);
	CFArrayRef addrs = (CFArrayRef)CFDictionaryGetValue(host->_info, (const void*)kCFHostAddresses);
	
	// Grab the first of each if they exist in order to perform any of the lookups
	CFStringRef name = names && ((CFTypeRef)names != kCFNull) && CFArrayGetCount(names) ? (CFStringRef)CFArrayGetValueAtIndex(names, 0) : NULL;
	CFDataRef addr = addrs && ((CFTypeRef)addrs != kCFNull) && CFArrayGetCount(addrs) ? (CFDataRef)CFArrayGetValueAtIndex(addrs, 0) : NULL;
	
	*_Radar4012176 = FALSE;
	
	// Only allow one lookup at a time
	if (host->_lookup)
		return result;
	
	switch (info) {

		// If a address lookup and there is a name, create and start the lookup.
		case kCFHostAddresses:
		
			if (name) {
				
				CFArrayRef cached = NULL;
				
				/* Expire any entries from the cache */
				_ExpireCacheEntries();
				
				/* Lock the cache */
				_CFMutexLock(_HostLock);
				
				/* Go for a cache entry. */
				if (_HostCache)
					cached = (CFArrayRef)CFDictionaryGetValue(_HostCache, name);
	
				if (cached)
					CFRetain(cached);
	
				_CFMutexUnlock(_HostLock);

				/* Create a lookup if no cache entry. */
				if (!cached)
					host->_lookup = _CreateAddressLookup(name, info, host, &(host->_error));
					
				else {
					
					CFAllocatorRef alloc = CFGetAllocator(name);
					
					/* Make a copy of the addresses in the cached entry. */
					CFArrayRef cp = _CFArrayCreateDeepCopy(alloc,
														   CFHostGetInfo((CFHostRef)CFArrayGetValueAtIndex(cached, 0), _kCFHostMasterAddressLookup, NULL));
					
					CFRunLoopSourceContext ctxt = {
						0,
						host,
						CFRetain,
						CFRelease,
						CFCopyDescription,
						NULL,
						NULL,
						NULL,
						NULL,
						(void (*)(void*))_AddressLookupPerform
					};
						
					/* Create the lookup source.  This source will be signalled immediately. */
					host->_lookup = CFRunLoopSourceCreate(alloc, 0, &ctxt);
					
					/* Upon success, add the data and signal the source. */
					if (host->_lookup && cp) {

						CFDictionaryAddValue(host->_info, (const void*)info, cp);

						CFRunLoopSourceSignal((CFRunLoopSourceRef)host->_lookup);
						*_Radar4012176 = TRUE;
					}
					
					else {
						
						host->_error.error = ENOMEM;
						host->_error.domain = kCFStreamErrorDomainPOSIX;
					}
					
					if (cp)
						CFRelease(cp);
					else if (host->_lookup) {
						CFRelease(host->_lookup);
						host->_lookup = NULL;
					}
					
					CFRelease(cached);
				}
			}
			
			break;

		// If a name lookup and there is an address, create and start the lookup.
		case kCFHostNames:
			if (addr) host->_lookup = _CreateNameLookup(addr, host, &(host->_error));
			break;

		// Create a reachability check using the address or name (prefers address).
		case kCFHostReachability:
			{
				CFTypeRef use = (addr != NULL) ? (CFTypeRef)addr : (CFTypeRef)name;
				
				/* Create the reachability lookup. */
				host->_lookup = _CreateReachabilityLookup(use, host, &(host->_error));
				
				/*
				** <rdar://problem/3612320> Check reachability by IP address doesn't work?
				**
				** Reachability when created with an IP has not future trigger point in
				** order to get the flags callback.  The behavior of the reachabilty object
				** can not change, so as a workaround, CFHost does an immediate flags
				** request and then creates the CFRunLoopSourceRef for the asynchronous
				** trigger.
				*/
				if (host->_lookup && ((use == addr) || _IsDottedIp(use))) {
					
					CFRunLoopSourceContext ctxt = {
						0,														// version
						host,													// info
						NULL,													// retain
						NULL,													// release
						NULL,													// copyDescription
						NULL,													// equal
						NULL,													// hash
						NULL,													// schedule
						NULL,													// cancel
						(void(*)(void*))(&_NetworkReachabilityByIPCallBack)		// perform
					};
					
					SCNetworkConnectionFlags flags = 0;
					CFAllocatorRef alloc = CFGetAllocator(host);
					
					/* Get the flags right away for dotted IP. */
					SCNetworkReachabilityGetFlags((SCNetworkReachabilityRef)(host->_lookup), &flags);
					
					/* Remove the callback that was set already. */
					SCNetworkReachabilitySetCallback((SCNetworkReachabilityRef)(host->_lookup), NULL, NULL);
					
					/* Toss out the lookup because a new one will be set up. */
					CFRelease(host->_lookup);
					host->_lookup = NULL;
					
					/* Create the asynchronous source */
					host->_lookup = CFRunLoopSourceCreate(alloc, 0, &ctxt);

					if (!host->_lookup) {
						host->_error.error = ENOMEM;
						host->_error.domain = kCFStreamErrorDomainPOSIX;
					}
					
					else {
					
						// Create the data for hanging off the host info dictionary					
						CFDataRef reachability = CFDataCreate(alloc, (const UInt8*)&flags, sizeof(flags));
						
						// Make sure to toss the cached info now.
						CFDictionaryRemoveValue(host->_info, (const void*)kCFHostReachability);
						
						// If didn't create the data, fail with out of memory.
						if (!reachability) {
							
							/* Release and toss the lookup. */
							CFRelease(host->_lookup);
							host->_lookup = NULL;
							
							host->_error.error = ENOMEM;
							host->_error.domain = kCFStreamErrorDomainPOSIX;
						}
						
						else {
							// Save the reachability information
							CFDictionaryAddValue(host->_info, (const void*)kCFHostReachability, reachability);
							CFRelease(reachability);
							
							/* Signal the reachability for immediate attention. */
							CFRunLoopSourceSignal((CFRunLoopSourceRef)(host->_lookup));
						}
					}
				}
			}
			break;
		
		case 0x0000FFFC /* _kCFHostMasterAddressLookup */:
			host->_lookup = _CreateMasterAddressLookup(name, info, host, &(host->_error));
			break;
		
		// Create a general DNS check using the name or address (prefers name).
		default:

			if (name) {
				if ((info == _kCFHostIPv4Addresses) || (info == _kCFHostIPv6Addresses) || (info == _kCFHostByPassMasterAddressLookup))
					host->_lookup = _CreateMasterAddressLookup(name, info, host, &(host->_error));
				else
					host->_lookup = _CreateDNSLookup(name, info, host, &(host->_error));
			}
			else if (addr) {
				
				name = _CFNetworkCFStringCreateWithCFDataAddress(CFGetAllocator(addr), addr);
				
				if (name) {
					
					host->_lookup = _CreateDNSLookup(name, info, host, &(host->_error));
					
					CFRelease(name);
				}
				
				else {
					
					host->_error.error = ENOMEM;
					host->_error.domain = kCFStreamErrorDomainPOSIX;
				}
			}
			break;
	}
	
	if (host->_lookup) {
		host->_type = info;
		result = TRUE;
	}
		
	return result;
}


/* static */ CFMachPortRef
_CreateMasterAddressLookup(CFStringRef name, CFHostInfoType info, CFTypeRef context, CFStreamError* error) {
	
	UInt8* buffer;
	CFAllocatorRef allocator = CFGetAllocator(name);
	CFIndex converted, length = CFStringGetLength(name);
	CFMachPortRef result = NULL;
	
	// Get the bytes of the conversion
	buffer =  _CFStringGetOrCreateCString(allocator, name, NULL, &converted, kCFStringEncodingUTF8);
	
	// If the buffer failed to create, set the error and bail.
	if (!buffer) {
	
		// Set the no memory error.
		error->error = ENOMEM;
		error->domain = kCFStreamErrorDomainPOSIX;
		
		// Bail
		return result;
	}
	
	// See if all the bytes got converted.
	if (converted != length) {
		
		// If not, this amounts to a host not found error.  This is to primarily
		// deal with embedded bad characters in host names coming from URL's
		// (e.g. www.apple.com%00www.notapple.com).
		error->error = HOST_NOT_FOUND;
		error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
	}
	
	// Got a good name to send to lookup.
	else {
		
        struct addrinfo hints;
		mach_port_t prt = MACH_PORT_NULL;
		CFMachPortContext ctxt = {0, (void*)context, CFRetain, CFRelease, CFCopyDescription};
		
		// Set up the hints for getaddrinfo
        memset(&hints, 0, sizeof(hints));
		
#ifdef AI_PARALLEL
        hints.ai_flags = AI_ADDRCONFIG | AI_PARALLEL;
#else
        hints.ai_flags = AI_ADDRCONFIG;
#endif /* AI_PARALLEL */
		
		hints.ai_socktype = SOCK_STREAM;
		
		hints.ai_family = (info == _kCFHostIPv4Addresses) ? AF_INET :
			(info == _kCFHostIPv6Addresses) ? AF_INET6 : AF_UNSPEC;
			
		// Start the async lookup
		error->error = getaddrinfo_async_start(&prt, (const char*)buffer, NULL, &hints, _GetAddrInfoCallBack, (void*)context);
		
		// If the callback port was created, attempt to create the CFMachPort wrapper on it.
		if (!prt ||
			!(result = CFMachPortCreateWithPort(allocator, prt, _GetAddrInfoMachPortCallBack, &ctxt, NULL)))
		{
			
			// Failure somewhere so setup error the proper way.  If error->error is
			// set already, it was a netdb error.
			if (error->error) {
				
				/* If it's a system error, get the real error otherwise it's a NetDB error. */
				if (EAI_SYSTEM != error->error)
					error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
				else {
					error->error = errno;
					error->domain = kCFStreamErrorDomainPOSIX;
				}
			}
			
			// No error set, see if errno has anything.  If so, mark the error as
			// a POSIX error.
			else if (error->error = errno)
				error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainPOSIX;
				
			// Don't know what happened, so mark it as an internal netdb error.
			else {
				error->error = NETDB_INTERNAL;
				error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
			}
		}
	}
	
	// Release the buffer that was allocated for the name
	CFAllocatorDeallocate(allocator, buffer);
	
	return result;
}


/* static */ CFTypeRef
_CreateAddressLookup(CFStringRef name, CFHostInfoType info, void* context, CFStreamError* error) {
	
	CFTypeRef result = NULL;
	
	memset(error, 0, sizeof(error[0]));

	if (info == _kCFHostMasterAddressLookup)
		result = _CreateMasterAddressLookup(name, info, context, error);
		
	else {
		CFHostRef host = NULL;
		CFMutableArrayRef list = NULL;
		
		/* Lock the master lookups list and cache */
		_CFMutexLock(_HostLock);

		/* Get the list with the host lookup and other sources for this name */
		list = (CFMutableArrayRef)CFDictionaryGetValue(_HostLookups, name);
		
		/* Get the host if there is a list.  Host is at index zero. */
		if (list)
			host = (CFHostRef)CFArrayGetValueAtIndex(list, 0);
		
		/* If there is no list, this is the first; so set everything up. */
		else {
			
			/* Create the list to hold the host and sources. */
			list = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
			
			/* Set up the error in case the list wasn't created. */
			if (!list) {
				error->error = ENOMEM;
				error->domain = kCFStreamErrorDomainPOSIX;
			}
			
			else {
				
				name = CFStringCreateCopy(kCFAllocatorDefault, name);
				
				/* Add the list of clients for the name to the dictionary. */
				CFDictionaryAddValue(_HostLookups, name, list);
				
				CFRelease(name);
				
				/* Dictionary holds it now. */
				CFRelease(list);
				
				/* Make the real lookup. */
				host = CFHostCreateWithName(kCFAllocatorDefault, name);
				
				if (!host) {
					error->error = ENOMEM;
					error->domain = kCFStreamErrorDomainPOSIX;
				}
				
				else {
					CFHostClientContext ctxt = {0, (void*)name, CFRetain, CFRelease, CFCopyDescription};
					
					/* Place the CFHost at index 0. */
					CFArrayAppendValue(list, host);
					
					/* The list holds it now. */
					CFRelease(host);
					
					/* Set the client for asynchronous callback. */
					CFHostSetClient(host, (CFHostClientCallBack)_MasterCallBack, &ctxt);
					
					/* Kick off the resolution.  NULL the client if the resolution can't start. */
					if (!CFHostStartInfoResolution(host, _kCFHostMasterAddressLookup, error)) {
					
						CFHostSetClient(host, NULL, NULL);
						
						/* If it failed, don't keep it in the outstanding lookups list. */
						CFDictionaryRemoveValue(_HostLookups, name);
					}
				}
			}
		}
		
		/* Everything is still good? */
		if (!error->error) {
			
			CFRunLoopSourceContext ctxt = {
				0,
				context,
				CFRetain,
				CFRelease,
				CFCopyDescription,
				NULL,
				NULL,
				(void (*)(void*, CFRunLoopRef, CFStringRef))_AddressLookupSchedule_NoLock,
				NULL,
				(void (*)(void*))_AddressLookupPerform
			};
				
			/* Create the lookup source.  This source will be signalled once the shared lookup finishes. */
			result = CFRunLoopSourceCreate(CFGetAllocator(name), 0, &ctxt);
			
			/* If it succeed, add it to the list of other pending clients. */
			if (result) {
				CFArrayAppendValue(list, result);
			}
			
			else {
				
				error->error = ENOMEM;
				error->domain = kCFStreamErrorDomainPOSIX;
				
				/* If this was going to be the only client, need to clean up. */
				if (host && CFArrayGetCount(list) == 1) {
					
					/* NULL the client for the Mmster lookup and cancel it. */
					CFHostSetClient(host, NULL, NULL);
					CFHostCancelInfoResolution(host, _kCFHostMasterAddressLookup);
					
					/* Remove it from the list of pending lookups and clients. */
					CFDictionaryRemoveValue(_HostLookups, name);
				}
			}
		}
		
		_CFMutexUnlock(_HostLock);
	}

	return result;
}


/* static */ CFMachPortRef
_CreateNameLookup(CFDataRef address, void* context, CFStreamError* error) {
	
	mach_port_t prt = MACH_PORT_NULL;
	CFMachPortRef result = NULL;

	CFMachPortContext ctxt = {0, (void*)context, CFRetain, CFRelease, CFCopyDescription};		
	struct sockaddr* sa = (struct sockaddr*)CFDataGetBytePtr(address);
		
	// Start the async lookup
	error->error = getnameinfo_async_start(&prt, sa, sa->sa_len, 0, _GetNameInfoCallBack, (void*)context);
	
	// If the callback port was created, attempt to create the CFMachPort wrapper on it.
	if (!prt ||
		!(result = CFMachPortCreateWithPort(CFGetAllocator(address), prt, _GetNameInfoMachPortCallBack, &ctxt, NULL)))
	{
		
		// Failure somewhere so setup error the proper way.  If error->error is
		// set already, it was a netdb error.
		if (error->error) {
			
			/* If it's a system error, get the real error otherwise it's a NetDB error. */
			if (EAI_SYSTEM != error->error)
				error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
			else {
				error->error = errno;
				error->domain = kCFStreamErrorDomainPOSIX;
			}
		}
		
		// No error set, see if errno has anything.  If so, mark the error as
		// a POSIX error.
		else if (error->error = errno)
			error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainPOSIX;
			
		// Don't know what happened, so mark it as an internal netdb error.
		else {
			error->error = NETDB_INTERNAL;
			error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
		}
	}

	// Return the CFMachPortRef
	return result;
}


/* static */ SCNetworkReachabilityRef
_CreateReachabilityLookup(CFTypeRef thing, void* context, CFStreamError* error) {
	
	SCNetworkReachabilityRef result = NULL;
	
	// If the passed in argument is a CFData, create the reachability object
	// with the address.
	if (CFGetTypeID(thing) == CFDataGetTypeID()) {
		result = SCNetworkReachabilityCreateWithAddress(CFGetAllocator(thing),
														(struct sockaddr*)CFDataGetBytePtr((CFDataRef)thing));
	}
	
	// A CFStringRef means to create a reachability object by name.
	else {
		UInt8* buffer;
		CFAllocatorRef allocator = CFGetAllocator(thing);
		CFIndex converted, length = CFStringGetLength((CFStringRef)thing);
		
		// Get the bytes of the conversion
		buffer =  _CFStringGetOrCreateCString(allocator, (CFStringRef)thing, NULL, &converted, kCFStringEncodingUTF8);
		
		// If the buffer failed to create, set the error and bail.
		if (!buffer) {
			
			// Set the no memory error.
			error->error = ENOMEM;
			error->domain = kCFStreamErrorDomainPOSIX;
			
			// Bail
			return result;
		}
		
		// See if all the bytes got converted.
		if (converted != length) {
			
			// If not, this amounts to a host not found error.  This is to primarily
			// deal with embedded bad characters in host names coming from URL's
			// (e.g. www.apple.com%00www.notapple.com).
			error->error = HOST_NOT_FOUND;
			error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
		}
		
		// Got a good name to send to lookup.
		else {
			
			// Create the reachability lookup
			result = SCNetworkReachabilityCreateWithName(allocator, (const char*)buffer);
		}
		
		// Release the buffer that was allocated for the name
		CFAllocatorDeallocate(allocator, buffer);
	}
	
	// If the reachability object was created, need to set the callback context.
	if (result) {
		SCNetworkReachabilityContext ctxt = {0, (void*)context, CFRetain, CFRelease, CFCopyDescription};
		
		// Set the callback information
		SCNetworkReachabilitySetCallback(result, _NetworkReachabilityCallBack, &ctxt);
	}
	
	// If no reachability was created, make sure the error is set.
	else if (!error->error) {
		
		// Set it to errno
		error->error = errno;
		
		// If errno was set, place in the POSIX error domain.
		if (error->error)
			error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainPOSIX;
	}
	
	return result;
}


/* static */ CFMachPortRef
_CreateDNSLookup(CFTypeRef thing, CFHostInfoType type, void* context, CFStreamError* error) {
	
	UInt8* buffer;
	CFAllocatorRef allocator = CFGetAllocator(thing);
	CFIndex converted, length = CFStringGetLength((CFStringRef)thing);
	CFMachPortRef result = NULL;
	
	// Get the bytes of the conversion
	buffer =  _CFStringGetOrCreateCString(allocator, (CFStringRef)thing, NULL, &converted, kCFStringEncodingUTF8);
	
	// If the buffer failed to create, set the error and bail.
	if (!buffer) {
		
		// Set the no memory error.
		error->error = ENOMEM;
		error->domain = kCFStreamErrorDomainPOSIX;
		
		// Bail
		return result;
	}
	
	// See if all the bytes got converted.
	if (converted != length) {
		
		// If not, this amounts to a host not found error.  This is to primarily
		// deal with embedded bad characters in host names coming from URL's
		// (e.g. www.apple.com%00www.notapple.com).
		error->error = HOST_NOT_FOUND;
		error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
	}
	
	// Got a good name to send to lookup.
	else {
		
		mach_port_t prt = MACH_PORT_NULL;
		CFMachPortContext ctxt = {0, (void*)context, CFRetain, CFRelease, CFCopyDescription};
		
		// Start the async lookup
		error->error = dns_async_start(&prt, (const char*)buffer, ((type & 0xFFFF0000) >> 16), (type & 0x0000FFFF), 1, _DNSCallBack, (void*)context);
		
		// If the callback port was created, attempt to create the CFMachPort wrapper on it.
		if (!prt ||
			!(result = CFMachPortCreateWithPort(allocator, prt, _DNSMachPortCallBack, &ctxt, NULL)))
		{
			
			// Failure somewhere so setup error the proper way.  If error->error is
			// set already, it was a netdb error.
			if (error->error) {
				
				/* If it's a system error, get the real error otherwise it's a NetDB error. */
				if (EAI_SYSTEM != error->error)
					error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
				else {
					error->error = errno;
					error->domain = kCFStreamErrorDomainPOSIX;
				}
			}
			
			// No error set, see if errno has anything.  If so, mark the error as
			// a POSIX error.
			else if (error->error = errno)
				error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainPOSIX;
			
			// Don't know what happened, so mark it as an internal netdb error.
			else {
				error->error = NETDB_INTERNAL;
				error->domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
			}
		}
	}
	
	// Release the buffer that was allocated for the name
	CFAllocatorDeallocate(allocator, buffer);
	
	return result;
}


/* static */ void
_GetAddrInfoCallBack(int32_t status, struct addrinfo* res, void* ctxt) {

	_CFHost* host = (_CFHost*)ctxt;
	CFHostClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	CFHostInfoType type = _kCFNullHostInfoType;
	
	// Retain here to guarantee safety really after the lookups release,
	// but definitely before the callback.
	CFRetain((CFHostRef)host);
	
	// Lock the host
	__CFSpinLock(&host->_lock);

	// If the lookup canceled, don't need to do any of this.
	if (host->_lookup) {
		
		// Make sure to toss the cached info now.
		CFDictionaryRemoveValue(host->_info, (const void*)(host->_type));
		
		// Set the error if got one back from getaddrinfo
		if (status) {
			
			/* If it's a system error, get the real error. */
			if (EAI_SYSTEM == status) {
				host->_error.error = errno;
				host->_error.domain = kCFStreamErrorDomainPOSIX;
			}
			
			else {
				host->_error.error = status;
				host->_error.domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
			}
			
			// Mark to indicate the resolution was performed.
			CFDictionaryAddValue(host->_info, (const void*)(host->_type), kCFNull);
		}
		
		else {

			CFMutableArrayRef addrs;
			CFAllocatorRef allocator = CFGetAllocator((CFHostRef)host);
			
			// This is the list of new addresses to be saved.
			addrs = CFArrayCreateMutable(allocator, 0, &kCFTypeArrayCallBacks);
			
			// Save the memory error if the address cache failed to create.
			if (!addrs) {
				host->_error.error = ENOMEM;
				host->_error.domain = kCFStreamErrorDomainPOSIX;
				
				// Mark to indicate the resolution was performed.
				CFDictionaryAddValue(host->_info, (const void*)(host->_type), kCFNull);
			}
			
			else {
				struct addrinfo* i;
				
				// Loop through all of the addresses saving them in the array.
				for (i = res; i; i = i->ai_next) {
					
					CFDataRef data;
					
					// Bypass any address families that are not understood by CFSocketStream
					if (i->ai_addr->sa_family != AF_INET && i->ai_addr->sa_family != AF_INET6)
						continue;
					
					// Wrap the address in a CFData
					data = CFDataCreate(allocator, (UInt8*)(i->ai_addr), i->ai_addr->sa_len);
					
					// Fail with a memory error if the address wouldn't wrap.
					if (!data) {
						
						host->_error.error = ENOMEM;
						host->_error.domain = kCFStreamErrorDomainPOSIX;
						
						// Release the addresses and mark as NULL so as not to save later.
						CFRelease(addrs);
						addrs = NULL;
						
						// Just fail now.
						break;
					}
					
					// Add the address and continue on to the next.
					CFArrayAppendValue(addrs, data);
					CFRelease(data);
				}
				
				// If the list is still good, need to save it.
				if (addrs) {
					
					// Save the list of address on the host.
					CFDictionaryAddValue(host->_info, (const void*)(host->_type), addrs);
					CFRelease(addrs);
				}
			}
		}
		
		// Save the callback if there is one at this time.
		cb = host->_callback;
		
		type = host->_type;
		
		// Save the error and client information for the callback
		memmove(&error, &(host->_error), sizeof(error));
		info = host->_client.info;
		
		// Remove the lookup from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(host->_lookup, host->_schedules);
		
		// Go ahead and invalidate the lookup
		CFMachPortInvalidate((CFMachPortRef)(host->_lookup));
		
		// Release the lookup now.
		CFRelease(host->_lookup);
		host->_lookup = NULL;
		host->_type = _kCFNullHostInfoType;
	}
	
	// Unlock the host so the callback can be made safely.
	__CFSpinUnlock(&host->_lock);
    
	// Release the results if some were received.
    if (res)
        freeaddrinfo(res);
	
	// If there is a callback, inform the client of the finish.
	if (cb)
		cb((CFHostRef)host, type, &error, info);
	
	// Go ahead and release now that the callback is done.
	CFRelease((CFHostRef)host);	
}


/* static */ void
_GetAddrInfoMachPortCallBack(CFMachPortRef port, void* msg, CFIndex size, void* info) {
	
	getaddrinfo_async_handle_reply(msg);
}


/* static */ void
_GetNameInfoCallBack(int32_t status, char *hostname, char *serv, void* ctxt) {

	_CFHost* host = (_CFHost*)ctxt;
	CFHostClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	// Retain here to guarantee safety really after the lookups release,
	// but definitely before the callback.
	CFRetain((CFHostRef)host);
	
	// Lock the host
	__CFSpinLock(&host->_lock);
	
	// If the lookup canceled, don't need to do any of this.
	if (host->_lookup) {
		
		// Make sure to toss the cached info now.
		CFDictionaryRemoveValue(host->_info, (const void*)kCFHostNames);
		
		// Set the error if got one back from getnameinfo
		if (status) {
			
			/* If it's a system error, get the real error. */
			if (EAI_SYSTEM == status) {
				host->_error.error = errno;
				host->_error.error = kCFStreamErrorDomainPOSIX;
			}
			
			else {
				host->_error.error = status;
				host->_error.domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
			}
			
			// Mark to indicate the resolution was performed.
			CFDictionaryAddValue(host->_info, (const void*)kCFHostNames, kCFNull);
		}
		
		else {
				
			CFAllocatorRef allocator = CFGetAllocator((CFHostRef)host);
			
			// Create the name from the given response.
			CFStringRef name = CFStringCreateWithCString(allocator, hostname, kCFStringEncodingUTF8);
			
			// If didn't create the name, fail with out of memory.
			if (!name) {
				host->_error.error = ENOMEM;
				host->_error.domain = kCFStreamErrorDomainPOSIX;
			}
			
			else {
				// Create the list to hold the name.
				CFArrayRef names = CFArrayCreate(allocator, (const void**)(&name), 1, &kCFTypeArrayCallBacks);
				
				// Don't need the retain anymore
				CFRelease(name);
				
				// Failed to create the list of names so mark out of memory.
				if (!names) {
					host->_error.error = ENOMEM;
					host->_error.domain = kCFStreamErrorDomainPOSIX;
				}
				
				// Save the list of names on the host.
				else {
					CFDictionaryAddValue(host->_info, (const void*)kCFHostNames, names);
					CFRelease(names);					
				}
			}
		}
		
		// Save the callback if there is one at this time.
		cb = host->_callback;
		
		// Save the error and client information for the callback
		memmove(&error, &(host->_error), sizeof(error));
		info = host->_client.info;
		
		// Remove the lookup from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(host->_lookup, host->_schedules);
		
		// Go ahead and invalidate the lookup
		CFMachPortInvalidate((CFMachPortRef)(host->_lookup));
		
		// Release the lookup now.
		CFRelease(host->_lookup);
		host->_lookup = NULL;
		host->_type = _kCFNullHostInfoType;
	}
	
	// Unlock the host so the callback can be made safely.
	__CFSpinUnlock(&host->_lock);

	// Release the results if there were any.
	if (serv) free(serv);
	if (hostname) free(hostname);
	
	// If there is a callback, inform the client of the finish.
	if (cb)
		cb((CFHostRef)host, kCFHostNames, &error, info);
	
	// Go ahead and release now that the callback is done.
	CFRelease((CFHostRef)host);
}


/* static */ void
_GetNameInfoMachPortCallBack(CFMachPortRef port, void* msg, CFIndex size, void* info) {
	
	getnameinfo_async_handle_reply(msg);
}


/* static */ void
_NetworkReachabilityCallBack(SCNetworkReachabilityRef target, SCNetworkConnectionFlags flags, void* ctxt) {
	
	_CFHost* host = (_CFHost*)ctxt;
	CFHostClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	// Retain here to guarantee safety really after the lookups release,
	// but definitely before the callback.
	CFRetain((CFHostRef)host);
	
	// Lock the host
	__CFSpinLock(&host->_lock);
	
	// If the lookup canceled, don't need to do any of this.
	if (host->_lookup) {
		
		// Create the data for hanging off the host info dictionary
		CFDataRef reachability = CFDataCreate(CFGetAllocator(target), (const UInt8*)&flags, sizeof(flags));
		
		// Make sure to toss the cached info now.
		CFDictionaryRemoveValue(host->_info, (const void*)kCFHostReachability);
		
		// If didn't create the data, fail with out of memory.
		if (!reachability) {
			host->_error.error = ENOMEM;
			host->_error.domain = kCFStreamErrorDomainPOSIX;
		}
		
		else {
			// Save the reachability information
			CFDictionaryAddValue(host->_info, (const void*)kCFHostReachability, reachability);
			CFRelease(reachability);
		}
		
		// Save the callback if there is one at this time.
		cb = host->_callback;
		
		// Save the error and client information for the callback
		memmove(&error, &(host->_error), sizeof(error));
		info = host->_client.info;
		
		// Remove the lookup from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(host->_lookup, host->_schedules);
		
		// "Invalidate" the reachability object by removing the client
		SCNetworkReachabilitySetCallback((SCNetworkReachabilityRef)(host->_lookup), NULL, NULL);
		
		// Release the lookup now.
		CFRelease(host->_lookup);
		host->_lookup = NULL;
		host->_type = _kCFNullHostInfoType;
	}
	
	// Unlock the host so the callback can be made safely.
	__CFSpinUnlock(&host->_lock);
	
	// If there is a callback, inform the client of the finish.
	if (cb)
		cb((CFHostRef)host, kCFHostReachability, &error, info);
	
	// Go ahead and release now that the callback is done.
	CFRelease((CFHostRef)host);
}


/* static */ void
_NetworkReachabilityByIPCallBack(_CFHost* host) {
	
	CFHostClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	// Retain here to guarantee safety really after the lookups release,
	// but definitely before the callback.
	CFRetain((CFHostRef)host);
	
	// Lock the host
	__CFSpinLock(&host->_lock);
	
	// If the lookup canceled, don't need to do any of this.
	if (host->_lookup) {
		
		// Save the callback if there is one at this time.
		cb = host->_callback;
		
		// Save the error and client information for the callback
		memmove(&error, &(host->_error), sizeof(error));
		info = host->_client.info;
		
		// Remove the lookup from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(host->_lookup, host->_schedules);
		
		// Invalidate the run loop source that got here
		CFRunLoopSourceInvalidate((CFRunLoopSourceRef)(host->_lookup));
		
		// Release the lookup now.
		CFRelease(host->_lookup);
		host->_lookup = NULL;
		host->_type = _kCFNullHostInfoType;
	}
	
	// Unlock the host so the callback can be made safely.
	__CFSpinUnlock(&host->_lock);
	
	// If there is a callback, inform the client of the finish.
	if (cb)
		cb((CFHostRef)host, kCFHostReachability, &error, info);
	
	// Go ahead and release now that the callback is done.
	CFRelease((CFHostRef)host);
}


/* static */ void
_DNSCallBack(int32_t status, char *buf, uint32_t len, struct sockaddr *from, int fromlen, void *context) {
	
	_CFHost* host = (_CFHost*)context;
	CFHostClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	CFHostInfoType type = _kCFNullHostInfoType;
	
	// Retain here to guarantee safety really after the lookups release,
	// but definitely before the callback.
	CFRetain((CFHostRef)context);
	
	// Lock the host
	__CFSpinLock(&host->_lock);
	
	// If the lookup canceled, don't need to do any of this.
	if (host->_lookup) {
		
		// Make sure to toss the cached info now.
		CFDictionaryRemoveValue(host->_info, (const void*)(host->_type));
		
		// Set the error if got one back from the lookup
		if (status) {
			
			/* If it's a system error, get the real error. */
			if (EAI_SYSTEM == status) {
				host->_error.error = errno;
				host->_error.domain = kCFStreamErrorDomainPOSIX;
			}
			
			else {
				host->_error.error = status;
				host->_error.domain = (CFStreamErrorDomain)kCFStreamErrorDomainNetDB;
			}
			
			// Mark to indicate the resolution was performed.
			CFDictionaryAddValue(host->_info, (const void*)(host->_type), kCFNull);
		}
		
		else {
			CFAllocatorRef allocator = CFGetAllocator((CFHostRef)context);
			
			// Wrap the reply and the source of the reply
			CFDataRef rr = CFDataCreate(allocator, (const UInt8*)buf, len);
			CFDataRef sa = CFDataCreate(allocator, (const UInt8*)from, fromlen);
			
			// If couldn't wrap, fail with no memory error.
			if (!rr || !sa) {
				host->_error.error = ENOMEM;
				host->_error.domain = kCFStreamErrorDomainPOSIX;
			}
			
			else {
				
				// Create the information to put in the info dictionary.
				CFTypeRef list[2] = {rr, sa};
				CFArrayRef array = CFArrayCreate(allocator, list, sizeof(list) / sizeof(list[0]), &kCFTypeArrayCallBacks);
				
				// Make sure it was created and add it.
				if (array) {
					CFDictionaryAddValue(host->_info, (const void*)(host->_type), array);
					CFRelease(array);
				}
				
				// Did make the information list so fail with out of memory
				else {
					host->_error.error = ENOMEM;
					host->_error.domain = kCFStreamErrorDomainPOSIX;
				}
			}
			
			// Release the reply if it was created.
			if (rr)
				CFRelease(rr);
			
			// Release the sockaddr wrapper if it was created
			if (sa)
				CFRelease(sa);
		}
		
		// Save the callback if there is one at this time.
		cb = host->_callback;
		
		// Save the type of lookup for the callback.
		type = host->_type;
		
		// Save the error and client information for the callback
		memmove(&error, &(host->_error), sizeof(error));
		info = host->_client.info;
		
		// Remove the lookup from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(host->_lookup, host->_schedules);
		
		// Go ahead and invalidate the lookup
		CFMachPortInvalidate((CFMachPortRef)(host->_lookup));
		
		// Release the lookup now.
		CFRelease(host->_lookup);
		host->_lookup = NULL;
		host->_type = _kCFNullHostInfoType;
	}
	
	// Unlock the host so the callback can be made safely.
	__CFSpinUnlock(&host->_lock);
	
	// If there is a callback, inform the client of the finish.
	if (cb)
		cb((CFHostRef)context, type, &error, info);
	
	// Go ahead and release now that the callback is done.
	CFRelease((CFHostRef)context);
}


/* static */ void
_DNSMachPortCallBack(CFMachPortRef port, void* msg, CFIndex size, void* info) {
	
	dns_async_handle_reply(msg);
}


/* static */ void
_MasterCallBack(CFHostRef theHost, CFHostInfoType typeInfo, const CFStreamError *error, CFStringRef name) {
	
	CFArrayRef list;
	
	/* Shut down the host lookup. */
	CFHostSetClient(theHost, NULL, NULL);
	
	/* Lock the host master list and cache */
	_CFMutexLock(_HostLock);
	
	/* Get the list of clients. */
	list = CFDictionaryGetValue(_HostLookups, name);
	
	if (list) {

		CFRetain(list);
	
		/* Remove the entry from the list of master lookups. */
		CFDictionaryRemoveValue(_HostLookups, name);
	}
	
	_CFMutexUnlock(_HostLock);	
	
	if (list) {
		
		CFIndex i, count;
		CFArrayRef addrs = CFHostGetInfo(theHost, _kCFHostMasterAddressLookup, NULL);
		
		/* If no error, add the host to the cache. */
		if (!error->error) {
				
			/* The host will be saved for each name in the list of names for the host. */
			CFArrayRef names = CFHostGetInfo(theHost, kCFHostNames, NULL);
			
			if (names && ((CFTypeRef)names != kCFNull)) {
					
				/* Each host cache entry is a host with its fetch time. */
				CFTypeRef orig[2] = {theHost, CFDateCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent())};
				
				/* Only add the entries if the date was created. */
				if (orig[1]) {
					
					/* Create the CFArray to be added into the cache. */
					CFArrayRef items = CFArrayCreate(kCFAllocatorDefault, orig, sizeof(orig) / sizeof(orig[0]), &kCFTypeArrayCallBacks);
					
					CFRelease(orig[1]);
					
					/* Once again, only add if the list was created. */
					if (items) {
						
						/* Loop through all the names of the host. */
						count = CFArrayGetCount(names);
						
						/* Add an entry for each name. */
						for (i = 0; i < count; i++)
							CFDictionaryAddValue(_HostCache, CFArrayGetValueAtIndex(names, i), items);
						
						CFRelease(items);
					}
				}
			}
		}
		
		count = CFArrayGetCount(list);
		
		for (i = 1; i < count; i++) {
			
			_CFHost* client;
			CFRunLoopSourceContext ctxt = {0};
			CFRunLoopSourceRef src = (CFRunLoopSourceRef)CFArrayGetValueAtIndex(list, i);
			
			CFRunLoopSourceGetContext(src, &ctxt);
			client = (_CFHost*)ctxt.info;
			
			__CFSpinLock(&client->_lock);
			
			/* Make sure to toss the cached info now. */
			CFDictionaryRemoveValue(client->_info, (const void*)(client->_type));
			
			/* Deal with the error if there was one. */
			if (error->error) {
				
				/* Copy the error over to the client. */
				memmove(&client->_error, error, sizeof(error[0]));
				
				/* Mark to indicate the resolution was performed. */
				CFDictionaryAddValue(client->_info, (const void*)(client->_type), kCFNull);
			}
			
			else {
				
				/* Make a copy of the addresses with the client's allocator. */
				CFArrayRef cp = _CFArrayCreateDeepCopy(CFGetAllocator((CFHostRef)client), addrs);
				
				if (cp) {
					
					CFDictionaryAddValue(client->_info, (const void*)(client->_type), addrs);
				
					CFRelease(cp);
				}
				
				else {
					
					/* Make sure to error if couldn't create the list. */
					client->_error.error = ENOMEM;
					client->_error.domain = kCFStreamErrorDomainPOSIX;
					
					/* Mark to indicate the resolution was performed. */
					CFDictionaryAddValue(client->_info, (const void*)(client->_type), kCFNull);
				}
			}
			
			/* Signal the client for immediate attention. */
			CFRunLoopSourceSignal((CFRunLoopSourceRef)(client->_lookup));
			
			CFArrayRef schedules = client->_schedules;
			CFIndex j, c = CFArrayGetCount(schedules);
			
			/* Make sure the signal can make it through */
			for (j = 0; j < c; j += 2) {
				
				/* Grab the run loop for checking */
				CFRunLoopRef runloop = (CFRunLoopRef)CFArrayGetValueAtIndex(schedules, j);

				/* If it's sleeping, need to further check it. */
				if (CFRunLoopIsWaiting(runloop)) {
					
					/* Grab the mode for further check */
					CFStringRef mode = CFRunLoopCopyCurrentMode(runloop);
					
					if (mode) {
						
						/* If the lookup is in the right mode, need to wake up the run loop. */
						if (CFRunLoopContainsSource(runloop, (CFRunLoopSourceRef)(client->_lookup), mode)) {
							CFRunLoopWakeUp(runloop);
						}
						
						/* Don't need this anymore. */
						CFRelease(mode);
					}
				}
			}
			
			__CFSpinUnlock(&client->_lock);
		}
		
		CFRelease(list);
	}
}


/* static */ void
_AddressLookupSchedule_NoLock(_CFHost* host, CFRunLoopRef rl, CFStringRef mode) {
	
	CFArrayRef list;
	CFArrayRef names = (CFArrayRef)CFDictionaryGetValue(host->_info, (const void*)kCFHostNames);
	CFStringRef name = (CFStringRef)CFArrayGetValueAtIndex(names, 0);

	/* Lock the list of master lookups and cache */
	_CFMutexLock(_HostLock);
	
	list = CFDictionaryGetValue(_HostLookups, name);

	if (list)
		CFHostScheduleWithRunLoop((CFHostRef)CFArrayGetValueAtIndex(list, 0), rl, mode);

	_CFMutexUnlock(_HostLock);	
}


/* static */ void
_AddressLookupPerform(_CFHost* host) {
		
	CFHostClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	// Retain here to guarantee safety really after the lookups release,
	// but definitely before the callback.
	CFRetain((CFHostRef)host);
	
	// Lock the host
	__CFSpinLock(&host->_lock);
	
	// Save the callback if there is one at this time.
	cb = host->_callback;
	
	// Save the error and client information for the callback
	memmove(&error, &(host->_error), sizeof(error));
	info = host->_client.info;
	
	// Remove the lookup from run loops and modes
	_CFTypeUnscheduleFromMultipleRunLoops(host->_lookup, host->_schedules);
	
	// Go ahead and invalidate the lookup
	CFRunLoopSourceInvalidate((CFRunLoopSourceRef)(host->_lookup));
	
	// Release the lookup now.
	CFRelease(host->_lookup);
	host->_lookup = NULL;
	host->_type = _kCFNullHostInfoType;	
	
	// Unlock the host so the callback can be made safely.
	__CFSpinUnlock(&host->_lock);
	
	// If there is a callback, inform the client of the finish.
	if (cb)
		cb((CFHostRef)host, kCFHostAddresses, &error, info);
	
	// Go ahead and release now that the callback is done.
	CFRelease((CFHostRef)host);	
}


/* static */ void
_ExpireCacheEntries(void) {
	
	CFIndex count;
	
	CFStringRef keys_buffer[_kCFHostCacheMaxEntries];
	CFArrayRef values_buffer[_kCFHostCacheMaxEntries];
	
	CFStringRef* keys = &keys_buffer[0];
	CFArrayRef* values = &values_buffer[0];
	
	/* Lock the cache */
	_CFMutexLock(_HostLock);
	
	if (_HostCache) {
		
		/* Get the count for proper allocation if needed and for iteration. */
		count = CFDictionaryGetCount(_HostCache);
		
		/* Allocate buffers for keys and values if don't have large enough static buffers. */
		if (count > _kCFHostCacheMaxEntries) {
			
			keys = (CFStringRef*)CFAllocatorAllocate(kCFAllocatorDefault, sizeof(keys[0]) * count, 0);
			values = (CFArrayRef*)CFAllocatorAllocate(kCFAllocatorDefault, sizeof(values[0]) * count, 0);
		}
		
		/* Only iterate if buffers were allocated. */
		if (keys && values) {
		
			CFIndex i, j = 0;
			CFTimeInterval oldest = 0.0;
			
			/* Get "now" for comparison for freshness. */
			CFDateRef now = CFDateCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent());
			
			/* Get all the hosts in the cache */
			CFDictionaryGetKeysAndValues(_HostCache, (const void **)keys, (const void **)values);
			
			/* Iterate through and get rid of expired ones. */
			for (i = 0; i < count; i++) {
				
				/* How long since now?  Use abs in order to handle clock changes. */
				CFTimeInterval since = fabs(CFDateGetTimeIntervalSinceDate(now, (CFDateRef)CFArrayGetValueAtIndex(values[i], 1)));
				
				/* If timeout, remove the entry. */
				if (since >= _kCFHostCacheTimeout)
					CFDictionaryRemoveValue(_HostCache, keys[i]);
					
				/* If this one is older than the oldest, save it's index. */
				else if (since > oldest) {
					j = i;
					oldest = since;
				}
			}
			
			CFRelease(now);
			
			/* If the count still isn't in the bounds of maximum number of entries, remove the oldest. */
			if (CFDictionaryGetCount(_HostCache) >= _kCFHostCacheMaxEntries)
				CFDictionaryRemoveValue(_HostCache, keys[j]);
		}
		
		/* If space for keys was made, deallocate it. */
		if (keys && (keys != &keys_buffer[0]))
			CFAllocatorDeallocate(kCFAllocatorDefault, keys);
		
		/* If space for values was made, deallocate it. */
		if (values && (values != &values_buffer[0]))
			CFAllocatorDeallocate(kCFAllocatorDefault, values);
	}
	
	_CFMutexUnlock(_HostLock);
}


/* static */ CFArrayRef
_CFArrayCreateDeepCopy(CFAllocatorRef alloc, CFArrayRef array) {
	
    CFArrayRef result = NULL;
    CFIndex i, c = CFArrayGetCount(array);
    CFTypeRef *values;
    if (c == 0) {
        result = CFArrayCreate(alloc, NULL, 0, &kCFTypeArrayCallBacks);
    } else if ((values = (CFTypeRef*)CFAllocatorAllocate(alloc, c*sizeof(CFTypeRef), 0)) != NULL) {
        CFArrayGetValues(array, CFRangeMake(0, c), values);
        if (CFGetTypeID(values[0]) == CFStringGetTypeID()) {
            for (i = 0; i < c; i ++) {
                values[i] = CFStringCreateCopy(alloc, (CFStringRef)values[i]);
                if (values[i] == NULL) {
                    break;
                }
            }
        }
        else if (CFGetTypeID(values[0]) == CFDataGetTypeID()) {
            for (i = 0; i < c; i ++) {
                values[i] = CFDataCreateCopy(alloc, (CFDataRef)values[i]);
                if (values[i] == NULL) {
                    break;
                }
            }
        }
        else {
            for (i = 0; i < c; i ++) {
                values[i] = CFPropertyListCreateDeepCopy(alloc, values[i], kCFPropertyListImmutable);
                if (values[i] == NULL) {
                    break;
                }
            }
        }
        
        result = (i == c) ? CFArrayCreate(alloc, values, c, &kCFTypeArrayCallBacks) : NULL;
        c = i;
        for (i = 0; i < c; i ++) {
            CFRelease(values[i]);
        }
        CFAllocatorDeallocate(alloc, values);
    }
    return result;
}


/* static */ Boolean
_IsDottedIp(CFStringRef name) {
	
	Boolean result = FALSE;
	UInt8 stack_buffer[1024];
	UInt8* buffer = stack_buffer;
	CFIndex length = sizeof(stack_buffer);
	CFAllocatorRef alloc = CFGetAllocator(name);

	buffer = _CFStringGetOrCreateCString(alloc, name, buffer, &length, kCFStringEncodingASCII);

	if (buffer) {
	
		struct addrinfo hints;
		struct addrinfo* results = NULL;
		
		memset(&hints, 0, sizeof(hints));
		hints.ai_flags = AI_NUMERICHOST;
		
		if (!getaddrinfo((const char*)buffer, NULL, &hints, &results)) {
			
			if (results) {
			
				if (results->ai_addr)
					result = TRUE;

				freeaddrinfo(results);
			}
		}
	}
	
	if (buffer != stack_buffer)
		CFAllocatorDeallocate(alloc, buffer);
	
	return result;
}


#if 0
#pragma mark -
#pragma mark Extern Function Definitions (API)
#endif

/* extern */ CFTypeID
CFHostGetTypeID(void) {

    _CFDoOnce(&_kCFHostRegisterClass, _CFHostRegisterClass);

    return _kCFHostTypeID;
}


/* extern */ CFHostRef
CFHostCreateWithName(CFAllocatorRef allocator, CFStringRef hostname) {

	// Create the base object
	_CFHost* result = _HostCreate(allocator);

	// Set the names only if succeeded
	if (result) {
		
		// Create the list of names
		CFArrayRef names = CFArrayCreate(allocator, (const void**)(&hostname), 1, &kCFTypeArrayCallBacks);

		// Add the list to the info if it succeeded
		if (names) {
			CFDictionaryAddValue(result->_info, (const void*)kCFHostNames, names);
			CFRelease(names);
		}

		// Failed so release the new host and return null
		else {
			CFRelease((CFTypeRef)result);
			result = NULL;
		}
	}

	return (CFHostRef)result;
}


/* extern */ CFHostRef
CFHostCreateWithAddress(CFAllocatorRef allocator, CFDataRef addr) {

	// Create the base object
	_CFHost* result = _HostCreate(allocator);

	// Set the names only if succeeded
	if (result) {

		// Create the list of addresses
		CFArrayRef addrs = CFArrayCreate(allocator, (const void**)(&addr), 1, &kCFTypeArrayCallBacks);

		// Add the list to the info if it succeeded
		if (addrs) {
			CFDictionaryAddValue(result->_info, (const void*)kCFHostAddresses, addrs);
			CFRelease(addrs);
		}

		// Failed so release the new host and return null
		else {
			CFRelease((CFTypeRef)result);
			result = NULL;
		}
	}

	return (CFHostRef)result;
}


/* extern */ CFHostRef
CFHostCreateCopy(CFAllocatorRef allocator, CFHostRef h) {

	_CFHost* host = (_CFHost*)h;

	// Create the base object
	_CFHost* result = _HostCreate(allocator);

	// Set the names only if succeeded
	if (result) {

		// Release the current, because a new one will be laid down
		CFRelease(result->_info);

		// Lock original before going to town on it
		__CFSpinLock(&(host->_lock));

		// Just make a copy of all the information
		result->_info = CFDictionaryCreateMutableCopy(allocator, 0, host->_info);

		// Let the original go
		__CFSpinUnlock(&(host->_lock));

		// If it failed, release the new host and return null
		if (!result->_info) {
			CFRelease((CFTypeRef)result);
			result = NULL;
		}
	}

	return (CFHostRef)result;
}


/* extern */ Boolean
CFHostStartInfoResolution(CFHostRef theHost, CFHostInfoType info, CFStreamError* error) {

	_CFHost* host = (_CFHost*)theHost;
	CFStreamError extra;
	Boolean result = FALSE;

	if (!error)
		error = &extra;

	memset(error, 0, sizeof(error[0]));

	// Retain so it doesn't go away underneath in the case of a callout.  This is really
	// no worry for async, but makes the memmove for the error more difficult to place
	// for synchronous without it being here.
	CFRetain(theHost);
	
	// Lock down the host to grab the info
	__CFSpinLock(&(host->_lock));

	do {
		
		Boolean wakeup = FALSE;
		
		// Create lookup.  Bail if it fails.
		if (!_CreateLookup_NoLock(host, info, &wakeup))
			break;

		// Async mode is complete at this point
		if (host->_callback) {
			
			// Schedule the lookup on the run loops and modes.
			_CFTypeScheduleOnMultipleRunLoops(host->_lookup, host->_schedules);
			
			// 4012176 If the source was signaled, wake up the run loop.
			if (wakeup) {
				
				CFArrayRef schedules = host->_schedules;
				CFIndex i, count = CFArrayGetCount(schedules);

				// Make sure the signal can make it through
				for (i = 0; i < count; i += 2) {
					
					// Wake up run loop
					CFRunLoopWakeUp((CFRunLoopRef)CFArrayGetValueAtIndex(schedules, i));
				}
			}
			
			// It's now succeeded.
			result = TRUE;
		}

		// If there is no callback, go into synchronous mode.
		else {
			
			// Unlock the host
			__CFSpinUnlock(&(host->_lock));

			// Wait for synchronous return
			result = _HostBlockUntilComplete(host);
			
			// Lock down the host to grab the info
			__CFSpinLock(&(host->_lock));
		}
		
	} while (0);

	// Copy the error.
	memmove(error, &host->_error, sizeof(error[0]));

	// Unlock the host
	__CFSpinUnlock(&(host->_lock));

	// Release the earlier retain.
	CFRelease(theHost);
	
	return result;
}


/* extern */ CFTypeRef
CFHostGetInfo(CFHostRef theHost, CFHostInfoType info, Boolean* hasBeenResolved) {

	_CFHost* host = (_CFHost*)theHost;
	Boolean extra;
	CFTypeRef result = NULL;

	// Just make sure there is something to dereference.
	if (!hasBeenResolved)
		hasBeenResolved = &extra;

	// By default, it hasn't been resolved.
	*hasBeenResolved = FALSE;

	// Lock down the host to grab the info
	__CFSpinLock(&(host->_lock));

	// Grab the requested information
	result = (CFTypeRef)CFDictionaryGetValue(host->_info, (const void*)info);

	// If there was a result, mark it as being resolved.
	if (result) {

		// If it was NULL, that means resolution actually returned nothing.
		if (CFEqual(result, kCFNull))
			result = NULL;

		// It's been resolved.
		*hasBeenResolved = TRUE;
	}

	// Unlock the host
	__CFSpinUnlock(&(host->_lock));

	return result;
}


/* extern */ CFArrayRef
CFHostGetAddressing(CFHostRef theHost, Boolean* hasBeenResolved) {

	return (CFArrayRef)CFHostGetInfo(theHost, kCFHostAddresses, hasBeenResolved);
}


/* extern */ CFArrayRef
CFHostGetNames(CFHostRef theHost, Boolean* hasBeenResolved) {

	return (CFArrayRef)CFHostGetInfo(theHost, kCFHostNames, hasBeenResolved);
}


#if defined(__MACH__)
/* extern */ CFDataRef
CFHostGetReachability(CFHostRef theHost, Boolean* hasBeenResolved) {

	return (CFDataRef)CFHostGetInfo(theHost, kCFHostReachability, hasBeenResolved);
}
#endif


/* extern */ void
CFHostCancelInfoResolution(CFHostRef theHost, CFHostInfoType info) {
	
	_CFHost* host = (_CFHost*)theHost;
	
	// Lock down the host
	__CFSpinLock(&(host->_lock));
	
	// Make sure there is something to cancel.
	if (host->_lookup) {
		
		CFRunLoopSourceContext ctxt = {
			0,								// version
			NULL,							// info
			NULL,							// retain
			NULL,							// release
			NULL,							// copyDescription
			NULL,							// equal
			NULL,							// hash
			NULL,							// schedule
			NULL,							// cancel
			(void(*)(void*))(&_HostCancel)  // perform
		};

		// Remove the lookup from run loops and modes
		_CFTypeUnscheduleFromMultipleRunLoops(host->_lookup, host->_schedules);
		
		// Go ahead and invalidate the lookup
		_CFTypeInvalidate(host->_lookup);
		
		// Pull the lookup out of the list in the master list.
		if (host->_type == kCFHostAddresses) {
			
			CFMutableArrayRef list;
			CFArrayRef names = (CFArrayRef)CFDictionaryGetValue(host->_info, (const void*)kCFHostNames);
			CFStringRef name = (CFStringRef)CFArrayGetValueAtIndex(names, 0);
			
			/* Lock the master lookup list and cache */
			_CFMutexLock(_HostLock);
			
			/* Get the list of pending clients */
			list = (CFMutableArrayRef)CFDictionaryGetValue(_HostLookups, name);
			
			if (list) {
				
				/* Try to find this lookup in the list of clients. */
				CFIndex count = CFArrayGetCount(list);
				CFIndex idx = CFArrayGetFirstIndexOfValue(list, CFRangeMake(0, count), host->_lookup);
				
				if (idx != kCFNotFound) {
					
					/* Remove this lookup. */
					CFArrayRemoveValueAtIndex(list, idx);
					
					/* If this was the last client, kill the lookup. */
					if (count == 2) {
						
						CFHostRef lookup = (CFHostRef)CFArrayGetValueAtIndex(list, 0);
						
						/* NULL the client for the master lookup and cancel it. */
						CFHostSetClient(lookup, NULL, NULL);
						CFHostCancelInfoResolution(lookup, _kCFHostMasterAddressLookup);
						
						/* Remove it from the list of pending lookups and clients. */
						CFDictionaryRemoveValue(_HostLookups, name);
					}
				}
			}
			
			_CFMutexUnlock(_HostLock);	
		}
		
		// Release the lookup now.
		CFRelease(host->_lookup);
		
		// Create the cancel source
		host->_lookup = CFRunLoopSourceCreate(CFGetAllocator(theHost), 0, &ctxt);
		
		// If the cancel was created, need to schedule and signal it.
		if (host->_lookup) {
			
			CFArrayRef schedules = host->_schedules;
			CFIndex i, count = CFArrayGetCount(schedules);

			// Schedule the new lookup
			_CFTypeScheduleOnMultipleRunLoops(host->_lookup, schedules);
			
			// Signal the cancel for immediate attention.
			CFRunLoopSourceSignal((CFRunLoopSourceRef)(host->_lookup));
			
			// Make sure the signal can make it through
			for (i = 0; i < count; i += 2) {
				
				// Grab the run loop for checking
				CFRunLoopRef runloop = (CFRunLoopRef)CFArrayGetValueAtIndex(schedules, i);
				
				// If it's sleeping, need to further check it.
				if (CFRunLoopIsWaiting(runloop)) {
					
					// Grab the mode for further check
					CFStringRef mode = CFRunLoopCopyCurrentMode(runloop);
					
					if (mode) {
						
						// If the lookup is in the right mode, need to wake up the run loop.
						if (CFRunLoopContainsSource(runloop, (CFRunLoopSourceRef)(host->_lookup), mode)) {
							CFRunLoopWakeUp(runloop);
						}
						
						// Don't need this anymore.
						CFRelease(mode);
					}
				}
			}
		}
	}
	
	// Unlock the host
	__CFSpinUnlock(&(host->_lock));
}


/* extern */ Boolean
CFHostSetClient(CFHostRef theHost, CFHostClientCallBack clientCB, CFHostClientContext* clientContext) {

	_CFHost* host = (_CFHost*)theHost;

	// Lock down the host
	__CFSpinLock(&(host->_lock));

	// Release the user's context info if there is some and a release method
	if (host->_client.info && host->_client.release)
		host->_client.release(host->_client.info);
	
	// NULL callback or context signals to remove the client
	if (!clientCB || !clientContext) {
		
		// Cancel the outstanding lookup
		if (host->_lookup) {
			
			// Remove the lookup from run loops and modes
			_CFTypeUnscheduleFromMultipleRunLoops(host->_lookup, host->_schedules);
			
			// Go ahead and invalidate the lookup
			_CFTypeInvalidate(host->_lookup);
			
			// Pull the lookup out of the master lookups.
			if (host->_type == kCFHostAddresses) {
				
				CFMutableArrayRef list;
				CFArrayRef names = (CFArrayRef)CFDictionaryGetValue(host->_info, (const void*)kCFHostNames);
				CFStringRef name = (CFStringRef)CFArrayGetValueAtIndex(names, 0);
				
				/* Lock the masters list and cache */
				_CFMutexLock(_HostLock);
				
				/* Get the list of pending clients */
				list = (CFMutableArrayRef)CFDictionaryGetValue(_HostLookups, name);
				
				if (list) {
					
					/* Try to find this lookup in the list of clients. */
					CFIndex count = CFArrayGetCount(list);
					CFIndex idx = CFArrayGetFirstIndexOfValue(list, CFRangeMake(0, count), host->_lookup);
					
					if (idx != kCFNotFound) {
						
						/* Remove this lookup. */
						CFArrayRemoveValueAtIndex(list, idx);
						
						/* If this was the last client, kill the lookup. */
						if (count == 2) {
							
							CFHostRef lookup = (CFHostRef)CFArrayGetValueAtIndex(list, 0);
							
							/* NULL the client for the master lookup and cancel it. */
							CFHostSetClient(lookup, NULL, NULL);
							CFHostCancelInfoResolution(lookup, _kCFHostMasterAddressLookup);
							
							/* Remove it from the list of pending lookups and clients. */
							CFDictionaryRemoveValue(_HostLookups, name);
						}
					}
				}
				
				_CFMutexUnlock(_HostLock);	
			}
			
			// Release the lookup now.
			CFRelease(host->_lookup);
			host->_lookup = NULL;
			host->_type = _kCFNullHostInfoType;
		}

		// Zero out the callback and client context.
		host->_callback = NULL;
		memset(&(host->_client), 0, sizeof(host->_client));
	}

	//
	else {
		
		// Schedule any lookup on the run loops and modes if it hasn't been scheduled
		// already.  If there had previously been a callback, the lookup will have
		// already been scheduled.
		if (!host->_callback && host->_lookup)
			_CFTypeScheduleOnMultipleRunLoops(host->_lookup, host->_schedules);
		
		// Save the client's new callback
		host->_callback = clientCB;

		// Copy the client's context
		memmove(&(host->_client), clientContext, sizeof(host->_client));

		// If there is user data and a retain method, call it.
		if (host->_client.info && host->_client.retain)
			host->_client.info = (void*)(host->_client.retain(host->_client.info));
	}
	
	// Unlock the host
	__CFSpinUnlock(&(host->_lock));

	return TRUE;
}


/* extern */ void
CFHostScheduleWithRunLoop(CFHostRef theHost, CFRunLoopRef runLoop, CFStringRef runLoopMode) {

	_CFHost* host = (_CFHost*)theHost;
	
	/* Lock down the host before work */
	__CFSpinLock(&(host->_lock));

	/* Try adding the schedule to the list.  If it's added, need to do more work. */
	if (_SchedulesAddRunLoopAndMode(host->_schedules, runLoop, runLoopMode)) {

		/* If there is a current lookup, need to schedule it. */
		if (host->_lookup) {
			_CFTypeScheduleOnRunLoop(host->_lookup, runLoop, runLoopMode);
		}
	}
	
	/* Unlock the host */
	__CFSpinUnlock(&(host->_lock));
}


/* extern */ void
CFHostUnscheduleFromRunLoop(CFHostRef theHost, CFRunLoopRef runLoop, CFStringRef runLoopMode) {

	_CFHost* host = (_CFHost*)theHost;

	/* Lock down the host before work */
	__CFSpinLock(&(host->_lock));

	/* Try to remove the schedule from the list.  If it is removed, need to do more. */
	if (_SchedulesRemoveRunLoopAndMode(host->_schedules, runLoop, runLoopMode)) {

		/* If there is a current lookup, need to unschedule it. */
		if (host->_lookup) {
			_CFTypeUnscheduleFromRunLoop(host->_lookup, runLoop, runLoopMode);			
		}
	}

	/* Unlock the host */
	__CFSpinUnlock(&(host->_lock));
}

