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
 *  CFNetServices.c
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on Wed Feb 06 2002.
 *  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 */

#if 0
#pragma mark Includes
#endif
#include <CFNetwork/CFNetwork.h>
#include <CFNetwork/CFNetworkPriv.h>
#include "CFNetworkInternal.h"			/* for __CFSpinLock and __CFSpinUnlock */
#include "CFNetworkSchedule.h"

#include "DeprecatedDNSServiceDiscovery.h"
#include <SystemConfiguration/SystemConfiguration.h>

#include <dns_sd.h>
#include <nameser.h>


#if 0
#pragma mark -
#pragma mark Constants
#endif

/* extern */ const SInt32 kCFStreamErrorDomainNetServices = 10;
/* extern */ const SInt32 kCFStreamErrorDomainMach = 11;

/* Timeout for the short timer */
#define kCFNetServiceShortTimeout ((CFTimeInterval)0.25)

#define _kCFNetServiceDomain 0x00000000UL
#define _kCFNetServiceType 0x00000004UL
#define _kCFNetServiceName 0x00000002UL
#define _kCFNetServiceAddress 0x00000003UL
#define _kCFNetServiceTXT 0x00000001UL
#define _kCFNetServiceTargetHost 0x00000005UL


#if 0
#pragma mark -
#pragma mark Constant Strings
#endif

#ifdef __CONSTANT_CFSTRINGS__
#define _kCFNetServiceBlockingMode			CFSTR("_kCFNetServiceBlockingMode")
#define _kCFNetServiceEmptyString			CFSTR("")
#define _kCFNetServiceDebugFormatString		CFSTR("<CFNetService 0x%x>{domain=%@, type=%@, name=%@, specific=%@, addresses=%@}")
#else
static CONST_STRING_DECL(_kCFNetServiceBlockingMode, "_kCFNetServiceBlockingMode")
static CONST_STRING_DECL(_kCFNetServiceEmptyString, "")
static CONST_STRING_DECL(_kCFNetServiceDebugFormatString, "<CFNetService 0x%x>{domain=%@, type=%@, name=%@, specific=%@, addresses=%@}")
#endif	/* __CONSTANT_CFSTRINGS__ */

static const char _kCFNetServiceClassName[] = "CFNetService";


#if 0
#pragma mark -
#pragma mark Enum Values
#endif

enum {
    /* __CFNetService flags */
	kFlagBitLegacyService   = 0,
	kFlagBitAComplete,
	kFlagBitAAAAComplete,
	kFlagBitAReceived,
	kFlagBitAAAAReceived,
	kFlagBitActiveResolve,
	kFlagBitActiveRegister,
	kFlagBitCancel
};


#if 0
#pragma mark -
#pragma mark CFNetService struct
#endif

typedef struct {

	CFRuntimeBase						_base;
	
	CFSpinLock_t						_lock;
	
	UInt32								_flags;

	CFStreamError						_error;

	CFMutableDictionaryRef				_info;
	UInt32								_port;			/* Saved here for now.  Could be made a CFType and placed in info. */

	CFMutableArrayRef					_sources;		/* List of different things being performed */
	UInt32								_interface;
	
	union {
		dns_service_discovery_ref		_old_service;
		DNSServiceRef					_new_service;
	};
	
	DNSServiceRef						_a;
	DNSServiceRef						_aaaa;
	
	CFMutableDictionaryRef				_records;		/* Published records (DNSRecordRef's) */

	CFMutableArrayRef					_schedules;		/* List of loops and modes */
	CFNetServiceClientCallBack			_callback;
	CFNetServiceClientContext			_client;
} __CFNetService;


#if 0
#pragma mark -
#pragma mark Extern Function Declarations
#endif

/*
** Exported for CFNetServiceBrowser so that it can create items found on the wire
** without using normalization.
*/
extern CFNetServiceRef _CFNetServiceCreateCommon(CFAllocatorRef alloc, CFStringRef domain, CFStringRef type, CFStringRef name, UInt32 port);
	
extern dns_service_discovery_ref _CFNetServiceGetDNSServiceDiscovery(CFNetServiceRef theService);

/*
** Exported for CFNetServiceMonitor so that it can keep the service up-to-date
** with monitored records.  As it sees the updated records, it will update them
** on the service.  If the service happens to be a registration object, the
** SetInfo call should not force another update on the wire, thus the "no update."
*/
extern Boolean _CFNetServiceSetInfoNoPublish(CFNetServiceRef theService, UInt32 property, CFTypeRef value);


#if 0
#pragma mark -
#pragma mark Static Function Declarations
#endif

static void _CFNetServiceRegisterClass(void);

static void _ServiceDestroy(__CFNetService* service);
static Boolean _ServiceEqual(__CFNetService* s1, __CFNetService* s2);
static CFHashCode _ServiceHash(__CFNetService* service);
static CFStringRef _ServiceDescribe(__CFNetService* service);

static Boolean _ServiceSetInfo(__CFNetService* service, UInt32 property, CFTypeRef value, Boolean publish);
static void _ServiceCancel(__CFNetService* service);
static void _ServiceCancelDNSService_NoLock(__CFNetService* service, DNSServiceRef cancel);
static void _ServiceCreateQuery_NoLock(__CFNetService* service, ns_type rrtype, const char* name,
									   const char* regtype, const char* domain, Boolean schedule);
static Boolean _ServiceBlockUntilComplete(__CFNetService* service);
static void _MachPortCallBack(CFMachPortRef port, void *msg, CFIndex size, void *info);
static void _LegacyRegistrationReply(int error, void* context);
static void _LegacyResolverReply(struct sockaddr* interface, struct sockaddr* address, const char* txtRecord,
								 DNSServiceDiscoveryReplyFlags flags, void* context);
static void _RegisterReply(DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType errorCode,
						   const char* name, const char* regtype, const char* domain, void* context);
static void _ResolveReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
						  DNSServiceErrorType errorCode, const char* fullname, const char* hosttarget,
						  uint16_t port, uint16_t txtLen, const char* txtRecord, void* context);
static void _SocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info);
static void _AQuerySocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info);
static void _AAAAQuerySocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info);
static void _LongTimerCallBack(CFRunLoopTimerRef timer, void *context);
static void _ShortTimerCallBack(CFRunLoopTimerRef timer, void *context);
static void _AddressQueryRecordReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
									 DNSServiceErrorType errorCode, const char* fullname, uint16_t rrtype,
									 uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context);

static CFDataRef _CFDataCreateWithRecord(CFAllocatorRef allocator, uint16_t rrtype, uint16_t rdlen,
										 const void* rdata, u_short port, uint32_t interfaceIndex);
static void _AddRecords(const void *key, const void *value, void *context);
static void _DictionaryApplier(const void *key, const void *value, void *context);
static void _ScheduleSources(CFArrayRef sources, CFArrayRef schedules);
static void _UnscheduleSources(CFArrayRef sources, CFArrayRef schedules);
static void _InvalidateSources(CFMutableArrayRef sources);

static const void* TXTDictionaryKeyRetain(CFAllocatorRef allocator, CFStringRef key);
static void TXTDictionaryKeyRelease(CFAllocatorRef allocator, CFStringRef key);
static Boolean TXTDictionaryKeyEqual(CFStringRef key1, CFStringRef key2);
static CFHashCode TXTDictionaryKeyHash(CFStringRef key);


#if 0
#pragma mark -
#pragma mark Globals
#endif

static _CFOnceLock _kCFNetServiceRegisterClass = _CFOnceInitializer;
static CFTypeID _kCFNetServiceTypeID = _kCFRuntimeNotATypeID;
static CFRuntimeClass* _kCFNetServiceClass = NULL;


#if 0
#pragma mark -
#pragma mark Static Function Definitions
#endif

/* static */ void
_CFNetServiceRegisterClass(void) {
	
	_kCFNetServiceClass = (CFRuntimeClass*)calloc(1, sizeof(_kCFNetServiceClass[0]));
	
	if (_kCFNetServiceClass) {
		
		_kCFNetServiceClass->version = 0;
		_kCFNetServiceClass->className = _kCFNetServiceClassName;
		_kCFNetServiceClass->finalize = (void(*)(CFTypeRef))_ServiceDestroy;
		_kCFNetServiceClass->equal = (Boolean(*)(CFTypeRef, CFTypeRef))_ServiceEqual;
		_kCFNetServiceClass->hash = (CFHashCode(*)(CFTypeRef))_ServiceHash;
		_kCFNetServiceClass->copyDebugDesc = (CFStringRef(*)(CFTypeRef cf))_ServiceDescribe;
		
		_kCFNetServiceTypeID = _CFRuntimeRegisterClass(_kCFNetServiceClass);
	}
}


#if 0
#pragma mark * Service Methods
#endif



/* static */ void
_ServiceDestroy(__CFNetService* service) {
	
	/* Prevent anything else from taking hold */
	__CFSpinLock(&(service->_lock));
	
	/* Release the user's context info if there is some and a release method */
	if (service->_client.info && service->_client.release)
		service->_client.release(service->_client.info);
	
	/* Cancel the outstanding sources */
	if (service->_sources) {
		
		/* Remove the source from run loops and modes */
		if (service->_schedules)
			_UnscheduleSources(service->_sources, service->_schedules);
		
		/* Go ahead and invalidate the sources */
		_InvalidateSources(service->_sources);
		
		/* Release the sources now. */
		CFRelease(service->_sources);
	}
	
	/* Different way to clean up based upon whether it's legacy or not. */
	if (__CFBitIsSet(service->_flags, kFlagBitLegacyService)) {
		
		/* Need to clean up the service discovery stuff if there is */
		if (service->_old_service) {
			
			/* Release the underlying service discovery reference */
			DNSServiceDiscoveryDeallocate_Deprecated(service->_old_service);
		}
	}
	else {
		
		/* Need to clean up the service discovery stuff if there is */
		if (service->_new_service) {

			/* Release the underlying service discovery reference */
			DNSServiceRefDeallocate(service->_new_service);
		}
	}
	
	/* Clean up A record lookup if there is one */
	if (service->_a)
		DNSServiceRefDeallocate(service->_a);
	
	/* Clean up AAAA record lookup if there is one */
	if (service->_aaaa)
		DNSServiceRefDeallocate(service->_aaaa);
	
	/* Dump all the records that may have been published */
	if (service->_records)
		CFRelease(service->_records);
	
	/* Release any gathered information */
	if (service->_info)
		CFRelease(service->_info);

	/* Release the list of loops and modes */
	if (service->_schedules)
		CFRelease(service->_schedules);
}


/* static */ Boolean
_ServiceEqual(__CFNetService* s1, __CFNetService* s2) {
	
	Boolean result = FALSE;
	CFStringRef t1, t2;

	/*
    ** Two services which have different address are not caught here, since
    ** performing CFEqual on arrays requires order.  Passing on this now
    ** and only fix if there are bugs with this comparison.  **FIXME**
	*/
	
	/* Lock the services */
	__CFSpinLock(&s1->_lock);
	__CFSpinLock(&s2->_lock);
	
	/* Get the types for comparison */
	t1 = CFDictionaryGetValue(s1->_info, (const void*)_kCFNetServiceType);
	t2 = CFDictionaryGetValue(s2->_info, (const void*)_kCFNetServiceType);
	
	/* Can't be equal if the types aren't the same. */
	if (CFEqual(t1, t2)) {
		
		/* Get the domains for comparison */
		CFStringRef d1 = CFDictionaryGetValue(s1->_info, (const void*)_kCFNetServiceDomain);
		CFStringRef d2 = CFDictionaryGetValue(s2->_info, (const void*)_kCFNetServiceDomain);
	
		/* If either is the emtpy string need to shove in "local." */
		if (CFEqual(d1, _kCFNetServiceEmptyString))
			d1 = _kCFNetServiceEmptyString;
			
		if (CFEqual(d2, _kCFNetServiceEmptyString))
			d2 = _kCFNetServiceEmptyString;
		
		/* No need to do the name check unless the domains are the same */
		if (CFEqual(d1, d2)) {
		
			/* Get the names for comparison. */
			CFStringRef n1 = CFDictionaryGetValue(s1->_info, (const void*)_kCFNetServiceName);
			CFStringRef n2 = CFDictionaryGetValue(s2->_info, (const void*)_kCFNetServiceName);
			
			/* If the names are the same, they are equal. */
			if (CFEqual(n1, n2))
				result = TRUE;
				
			/* If there is an empty string, need to fill in the "default" name for the computer */
			else if (CFEqual(n1, _kCFNetServiceEmptyString) || CFEqual(n2, _kCFNetServiceEmptyString)) {
			
				/* Get the computer name */
				CFStringRef computer_name = SCDynamicStoreCopyLocalHostName(NULL);
				if (computer_name) {
					
					/* Set the default name for any that have no name */
					if (CFEqual(n1, _kCFNetServiceEmptyString))
						n1 = computer_name;
					
					if (CFEqual(n2, _kCFNetServiceEmptyString))
						n2 = computer_name;
						
					/* If the names are the same, they are equal */
					if (CFEqual(n1, n2))
						result = TRUE;
						
					CFRelease(computer_name);
				}
			}
		}
	}
	

	/* Unlock the services */
	__CFSpinUnlock(&s1->_lock);
	__CFSpinUnlock(&s2->_lock);

	return result;
}


/* static */ CFHashCode
_ServiceHash(__CFNetService* service) {
	
	CFHashCode result;
	CFStringRef name;
	
	/* Lock the service */
	__CFSpinLock(&service->_lock);
	
	/* Get the hash for the name on the service */
	name = CFDictionaryGetValue(service->_info, (const void*)_kCFNetServiceName);
	result = CFHash(name);
	
	/* If it's an empty name, need to go for the default */
	if (CFEqual(name, _kCFNetServiceEmptyString)) {
		
		/* Get the default name */
		name = SCDynamicStoreCopyLocalHostName(NULL);
		
		/* If got a name, create the hash from it. */
		if (name) {
			CFHash(name);
			CFRelease(name);
		}
	}

	/* Unlock the service */
	__CFSpinUnlock(&service->_lock);
	
	return result;
}


/* static */ CFStringRef
_ServiceDescribe(__CFNetService* service) {

	CFStringRef result = NULL;
	
	/* Lock the service */
	__CFSpinLock(&service->_lock);
	
	result = CFStringCreateWithFormat(CFGetAllocator((CFNetServiceRef)service),
									  NULL,
									  _kCFNetServiceDebugFormatString,
									  service,
									  CFDictionaryGetValue(service->_info, (const void*)_kCFNetServiceDomain),
									  CFDictionaryGetValue(service->_info, (const void*)_kCFNetServiceType),
									  CFDictionaryGetValue(service->_info, (const void*)_kCFNetServiceName),
									  CFDictionaryGetValue(service->_info, (const void*)_kCFNetServiceTXT),
									  CFDictionaryGetValue(service->_info, (const void*)_kCFNetServiceAddress));

	/* Unlock the service so the callback can be made safely. */
	__CFSpinUnlock(&service->_lock);

	return result;
}


/* static */ Boolean
_ServiceSetInfo(__CFNetService* service, UInt32 property, CFTypeRef value, Boolean publish) {
	
	Boolean result = FALSE;
	
	__CFSpinLock(&(service->_lock));
	
	/* Don't allow setting on a legacy service or on a resolve. */
	if (!__CFBitIsSet(service->_flags, kFlagBitLegacyService) &&
		!__CFBitIsSet(service->_flags, kFlagBitActiveResolve))
	{
		/* Save the value */
		if (value)
			CFDictionarySetValue(service->_info, (const void*)property, value);
		else
			CFDictionaryRemoveValue(service->_info, (const void*)property);
		
		/* Assume success */
		result = TRUE;
		
		/* Send it to the wire if actively registered */
		if (publish && service->_new_service) {
			
			DNSServiceErrorType err = 0;
			
			/* Just update the primary txt record when given TXT property. */
			if (property == _kCFNetServiceTXT) {
				
				err = DNSServiceUpdateRecord(service->_new_service,
											 NULL,
											 0,
											 value ? CFDataGetLength(value) : 0,
											 value ? CFDataGetBytePtr(value) : NULL,
											 0);
				
				if (err)
					result = FALSE;
			}
			
			/* High word is the class for the record type.  Only support Internet. */
			else if ((0xFFFF0000 & property) == 0x00010000) {
				
				/* Get the existing published record. */
				DNSRecordRef record = (DNSRecordRef)CFDictionaryGetValue(service->_records, (const void*)property);
				
				/* No value indicates to remove the record. */
				if (!value) {
					if (record)
						err = DNSServiceRemoveRecord(service->_new_service, record, 0);
					CFDictionaryRemoveValue(service->_records, (const void*)property);
				}
				
				/* If it exists, only need to update. */
				else if (record) {
					err = DNSServiceUpdateRecord(service->_new_service,
												 record,
												 0,
												 CFDataGetLength(value),
												 CFDataGetBytePtr(value),
												 0);
				}
				
				/* Not an update, but an add. */
				else {
					err = DNSServiceAddRecord(service->_new_service,
											  &record,
											  0,
											  (0x0000FFFF & property),
											  CFDataGetLength(value),
											  CFDataGetBytePtr(value),
											  0);
					
					CFDictionaryAddValue(service->_records, (const void*)property, record);
				}
				
				/* If there was an error, remove the published record. */
				if (err) {
					
					if (record)
						DNSServiceRemoveRecord(service->_new_service, record, 0);
					
					CFDictionaryRemoveValue(service->_records, (const void*)property);
					
					result = FALSE;
				}
			}
		}
	}
	
	__CFSpinUnlock(&(service->_lock));
	
	return result;
}


/* static */ void
_ServiceCancel(__CFNetService* service) {
	
	CFNetServiceClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	/*
	 *  Retain here to guarantee safety really after the service release,
	 *  but definitely before the callback.
	 */
	CFRetain(service);
	
	/* Lock the service */
	__CFSpinLock(&service->_lock);
	
	/* If canceled, don't need to do any of this. */
	if (CFArrayGetCount(service->_sources)) {
		
		/* Save the callback if there is one at this time. */
		cb = service->_callback;
		
		/* Save the error and client information for the callback */
		memmove(&error, &(service->_error), sizeof(error));
		info = service->_client.info;
		
		/* Remove the sources from run loops and modes */
		_UnscheduleSources(service->_sources, service->_schedules);
		
		/* Invalidate the run loop source that got here */
		_InvalidateSources(service->_sources);
	}

	/* No longer cancel */
	__CFBitClear(service->_flags, kFlagBitCancel);
	
	/* Unlock the service so the callback can be made safely. */
	__CFSpinUnlock(&service->_lock);
	
	/* If there is a callback, inform the client of the finish. */
	if (cb)
		cb((CFNetServiceRef)service, &error, info);
	
	/* Go ahead and release now that the callback is done. */
	CFRelease(service);
}


/* static */ void
_ServiceCancelDNSService_NoLock(__CFNetService* service, DNSServiceRef cancel) {
	
	CFTypeID t = CFSocketGetTypeID();
	int fd = DNSServiceRefSockFD(cancel);
	int i, count = CFArrayGetCount(service->_sources);
	
	/* Loop through the sources trying to find the socket associated with the cancel request */
	for (i = 0; i < count; i++) {
	
		/* Get the CF object at the current location */
		CFTypeRef sock = CFArrayGetValueAtIndex(service->_sources, i);
		
		/* Make sure it's a socket for continuing */
		if (t == CFGetTypeID(sock)) {
			
			/* If the native socket is the same as the mdns socket, need to kill it */
			if (CFSocketGetNative((CFSocketRef)sock) == fd) {
				
				/* First remove it from the run loops */
				_CFTypeUnscheduleFromMultipleRunLoops(sock, service->_schedules);
				
				/* Invalidate it */
				_CFTypeInvalidate(sock);
				
				/* Kill the mdns portion */
				DNSServiceRefDeallocate(cancel);
				
				/* Remove the sources from the list of sources */
				CFArrayRemoveValueAtIndex(service->_sources, i);
				
				/* Bail now */
				break;
			}
		}
	}
}


/* static */ void
_ServiceCreateQuery_NoLock(__CFNetService* service, ns_type rrtype, const char* name,
						   const char* regtype, const char* domain, Boolean schedule)
{
	DNSServiceRef* which = NULL;
	CFSocketCallBack cb = _SocketCallBack;
	
	/* ns_t_invalid indicates to do a regular resolve */
	if (rrtype == ns_t_invalid) {
		
		/* Which ivar is being used for service creation */
		which = &(service->_new_service);
		
		/* Start on no special interface.  Resolve against all interfaces. */
		service->_interface = 0;
		
		/* Create the resolve */
		service->_error.error = DNSServiceResolve(which,
												  0,
												  service->_interface,
												  name,
												  regtype,
												  domain,
												  _ResolveReply,
												  service);
	}
	
	/* Some other type of query */
	else {
		
		DNSServiceQueryRecordReply reply = NULL;
		
		/* Set up everything else for the query */
		switch (rrtype) {
		
			case ns_t_a:
				which = &(service->_a);				/* Hold in A record lookup */
				reply = _AddressQueryRecordReply;   /* Reply callback is for addresses */
				cb = _AQuerySocketCallBack;
				break;
			
			case ns_t_aaaa:
				which = &(service->_aaaa);			/* Hold in AAAA record lookup */
				reply = _AddressQueryRecordReply;   /* Reply callback is for addresses */
				cb = _AAAAQuerySocketCallBack;
				break;
				
			default:
				break;
		}
		
		/* If got something for lookup, start the query */
		if (which) {
			
			service->_error.error = DNSServiceQueryRecord(which,
														  0,
														  service->_interface,
														  name,
														  rrtype,
														  ns_c_in,
														  reply,
														  service);
		}
	}
	
	/* Set the domain if an error occurred */
	if (service->_error.error)
		service->_error.domain = kCFStreamErrorDomainNetServices;
	
	/* No error, so wrap the query for run loop integration. */
	else {

		CFSocketContext ctxt = {0, service, CFRetain, CFRelease, NULL};

		/* Create a CFSocket wrapper on the query */
		CFSocketRef sock = CFSocketCreateWithNative(CFGetAllocator((CFNetServiceRef)service),
													DNSServiceRefSockFD(*which),
													kCFSocketReadCallBack,
													cb,
													&ctxt);
		
		/* Add the socket to the sources if succeeded. */
		if (sock) {
			
			/* Tell CFSocket not to close the native socket on invalidation. */
			CFSocketSetSocketFlags(sock, CFSocketGetSocketFlags(sock) & ~kCFSocketCloseOnInvalidate);

			CFArrayAppendValue(service->_sources, sock);
			
			/* Schedule on run loops and modes, as required. */
			if (schedule)
				_CFTypeScheduleOnMultipleRunLoops(sock, service->_schedules);
			
			CFRelease(sock);
		}
		
		/* Need to record the error if it failed */
		else {
			
			/* Set error to whatever happened. */
			service->_error.error = errno;
			if (!service->_error.error)
				service->_error.error = ENOMEM;
			
			service->_error.domain = kCFStreamErrorDomainPOSIX;
			
			/* Cancel the query now if there was an error */
			DNSServiceRefDeallocate(*which);
			*which = NULL;
		}
	}
}


/* static */ Boolean
_ServiceBlockUntilComplete(__CFNetService* service) {
	
	/* Assume success by default */
	Boolean result = TRUE;
	CFRunLoopRef rl = CFRunLoopGetCurrent();
	
	/* Schedule in the blocking mode. */
	CFNetServiceScheduleWithRunLoop((CFNetServiceRef)service, rl, _kCFNetServiceBlockingMode);
	
	/* Lock in order to check for sources */
	__CFSpinLock(&(service->_lock));
	
	/* Check that there are sources. */
	while (CFArrayGetCount(service->_sources)) {
		
		/* Unlock again so the service can continue to be processed. */
		__CFSpinUnlock(&(service->_lock));
		
		/*
		 *  Run the loop in a private mode with it returning whenever a source
		 *  has been handled.
		 */
		CFRunLoopRunInMode(_kCFNetServiceBlockingMode, DBL_MAX, TRUE);
		
		/* Lock again in preparation for sources check */
		__CFSpinLock(&(service->_lock));		
	}
	
	/* Fail if there was an error. */
	if (service->_error.error)
		result = FALSE;
	
	/* Unlock the service again. */
	__CFSpinUnlock(&(service->_lock));
	
	/* Unschedule from the blocking mode */
	CFNetServiceUnscheduleFromRunLoop((CFNetServiceRef)service, rl, _kCFNetServiceBlockingMode);
	
	return result;
}


#if 0
#pragma mark * Service Discovery CallBacks
#endif


/* static */ void
_SocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void* data, void* info) {
	
	DNSServiceErrorType err;
	__CFNetService* service = info;
	
	(void)s;		/* unused */
	(void)type;		/* unused */
	(void)address;  /* unused */
	(void)data;		/* unused */
	
	CFRetain(service);
	
	// Dispatch to process the result
	err = DNSServiceProcessResult(service->_new_service);
	
	// If there was an error, need to infor the client.
	if (err) {
		
		// Dispatch based upon search type.
		if (__CFBitIsSet(service->_flags, kFlagBitActiveResolve))
			_ResolveReply(service->_new_service, 0, 0, err, NULL, NULL, 0, 0, NULL, info);
		
		else {
			
			void* info = NULL;
			CFStreamError error = {kCFStreamErrorDomainNetServices, err};
			CFNetServiceClientCallBack cb = NULL;
			
			/* Lock the service */
			__CFSpinLock(&service->_lock);
			
			/* Remove the registration from run loops and modes */
			_UnscheduleSources(service->_sources, service->_schedules);
			
			/* Go ahead and invalidate the sources */
			_InvalidateSources(service->_sources);
			
			/* Kill the registration. */
			DNSServiceRefDeallocate(service->_new_service);
			service->_new_service = NULL;
			
			/* Clear the flags */
			__CFBitClear(service->_flags, kFlagBitActiveRegister);
			
			/* Grab the callback and client info */
			cb = service->_callback;
			info = service->_client.info;
			
			/* Save the error in the client. */
			memmove(&(service->_error), &error, sizeof(error));
			
			/* Unlock the service so the callback can be made safely. */
			__CFSpinUnlock(&service->_lock);
			
			/* If there is a callback, inform the client of the error. */
			if (cb) {
				
				/* Inform the client. */
				cb((CFNetServiceRef)service, &error, info);
			}
		}
	}
	
	CFRelease(service);
}


/* static */ void
_AQuerySocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void* data, void* info) {
	
	DNSServiceErrorType err;
	__CFNetService* service = info;
	
	(void)s;		/* unused */
	(void)type;		/* unused */
	(void)address;  /* unused */
	(void)data;		/* unused */
	
	CFRetain(service);
	
	// Dispatch to process the result
	err = DNSServiceProcessResult(service->_a);
	
	// If there was an error, need to inform the client.
	if (err)
		_AddressQueryRecordReply(service->_a, 0, 0, err, NULL, 0, 0, 0, NULL, 0, info);
	
	CFRelease(service);
}


/* static */ void
_AAAAQuerySocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void* data, void* info) {
	
	DNSServiceErrorType err;
	__CFNetService* service = info;
	
	(void)s;		/* unused */
	(void)type;		/* unused */
	(void)address;  /* unused */
	(void)data;		/* unused */
	
	CFRetain(service);
	
	// Dispatch to process the result
	err = DNSServiceProcessResult(service->_aaaa);
	
	// If there was an error, need to inform the client.
	if (err)
		_AddressQueryRecordReply(service->_aaaa, 0, 0, err, NULL, 0, 0, 0, NULL, 0, info);
	
	CFRelease(service);
}


/* static */ void
_RegisterReply(DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType errorCode,
			   const char* name, const char* regtype, const char* domain, void* context)
{
	
	__CFNetService* service = (__CFNetService*)context;
	CFNetServiceClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	/*
	** Retain here to guarantee safety really after the source release,
	** but definitely before the callback.
	*/
	CFRetain(service);
	
	/* Lock the service */
	__CFSpinLock(&service->_lock);
	
	/* If the register canceled, don't need to do any of this. */
	if (service->_new_service) {
		
		int i;
		CFAllocatorRef alloc = CFGetAllocator((CFNetServiceRef)service);
		const char* values[] = {name, regtype, domain};
		UInt32 keys[] = {_kCFNetServiceName, _kCFNetServiceType, _kCFNetServiceDomain};
		
		/* If there is an error, fold the registration. */
		if (errorCode) {
			
			/* Save the error */
			service->_error.error = errorCode;
			service->_error.domain = kCFStreamErrorDomainNetServices;


		}
		
		/* Save the registered values */
		for (i = 0; i < (sizeof(keys) / sizeof(keys[0])); i++) {
			
			/* Only save if there is a value */
			if (values[i]) {
				
				/* Create the CFString to go into the info dictionary */
				CFStringRef string = CFStringCreateWithCString(alloc, values[i], kCFStringEncodingUTF8);
				
				/* Save the value if it was created. */
				if (string) {
					CFDictionarySetValue(service->_info, (const void*)(keys[i]), string);
					CFRelease(string);
				}
			}
		}
		
		/* Grab the callback */
		cb = service->_callback;
		
		/* Save the error and client information for the callback */
		memmove(&error, &(service->_error), sizeof(error));
		info = service->_client.info;
	}
	
	/* Unlock the service so the callback can be made safely. */
	__CFSpinUnlock(&service->_lock);
	
	/* If there is a callback, inform the client of the error. */
	if (cb) {
		
		/* Inform the client. */
		cb((CFNetServiceRef)service, &error, info);
	}
	
	/* Go ahead and release now that the callback is done. */
	CFRelease(service);
}



/* static */ void
_ResolveReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
			  DNSServiceErrorType errorCode, const char* fullname, const char* hosttarget,
			  uint16_t port, uint16_t txtLen, const char* txtRecord, void* context)
{
	__CFNetService* service = (__CFNetService*)context;
	CFNetServiceClientCallBack cb = NULL;
	CFStreamError error = {0, 0};
	void* info = NULL;
	
	/*
	** Retain here to guarantee safety really after the source release,
	** but definitely before the callback.
	*/
	CFRetain(service);
	
	/* Lock the service */
	__CFSpinLock(&service->_lock);
	
	/* If the register canceled, don't need to do any of this. */
	if (service->_new_service) {
		
		if (errorCode) {
			
			/* Save the error */
			service->_error.error = errorCode;
			service->_error.domain = kCFStreamErrorDomainNetServices;
			
			/* Remove the registration from run loops and modes */
			_UnscheduleSources(service->_sources, service->_schedules);
			
			/* Go ahead and invalidate the sources */
			_InvalidateSources(service->_sources);
			
			/* Kill the A record lookup */
			if (service->_a) {
				DNSServiceRefDeallocate(service->_a);
				service->_a = NULL;
			}
			
			/* Kill the AAAA record lookup */
			if (service->_aaaa) {
				DNSServiceRefDeallocate(service->_aaaa);
				service->_aaaa = NULL;
			}
			
			/* Mark these as being done. */
			__CFBitSet(service->_flags, kFlagBitAComplete);
			__CFBitSet(service->_flags, kFlagBitAAAAComplete);
		}
		
		else {
			
			CFAllocatorRef alloc = CFGetAllocator((CFNetServiceRef)service);
			CFDataRef txt = txtRecord ? CFDataCreate(alloc, (const UInt8*)txtRecord, txtLen) : NULL;
			CFStringRef tgt = hosttarget ? CFStringCreateWithCString(alloc, hosttarget, kCFStringEncodingUTF8) : NULL;
			
			/* Save the port */
			service->_port = ntohs(port);
			
			/* Remove the saved txt data */
			CFDictionaryRemoveValue(service->_info, (const void*)_kCFNetServiceTXT);
			
			/* Add the new one back if there is one */
			if (txt) {
				CFDictionaryAddValue(service->_info, (const void*)_kCFNetServiceTXT, txt);
				CFRelease(txt);
			}
			
			/* Remove the saved target host */
			CFDictionaryRemoveValue(service->_info, (const void*)_kCFNetServiceTargetHost);
			
			/* Add the new one back if there is one */
			if (tgt) {
				CFDictionaryAddValue(service->_info, (const void*)_kCFNetServiceTargetHost, tgt);
				CFRelease(tgt);
			}
			
			/* Save the interface so other queries are on the same interface */
			service->_interface = interfaceIndex;
			
			/* Kick off the address lookups */
			_ServiceCreateQuery_NoLock(service, ns_t_a, hosttarget, NULL, NULL, TRUE);
			_ServiceCreateQuery_NoLock(service, ns_t_aaaa, hosttarget, NULL, NULL, TRUE);
			
			/* If there was an error, need to mark as done */
			if (service->_error.error) {
				
				/* Remove the registration from run loops and modes */
				_UnscheduleSources(service->_sources, service->_schedules);
				
				/* Go ahead and invalidate the sources */
				_InvalidateSources(service->_sources);
				
				/* Kill the A record lookup */
				if (service->_a) {
					DNSServiceRefDeallocate(service->_a);
					service->_a = NULL;
				}
				
				/* Kill the AAAA record lookup */
				if (service->_aaaa) {
					DNSServiceRefDeallocate(service->_aaaa);
					service->_aaaa = NULL;
				}
				
				/* Mark these as being done. */
				__CFBitSet(service->_flags, kFlagBitAComplete);
				__CFBitSet(service->_flags, kFlagBitAAAAComplete);
			}			
		}
		
		/* Kill the mdns service */
		_ServiceCancelDNSService_NoLock(service, service->_new_service);
		service->_new_service = NULL;
		
		/* If all lookups are done, need to perform the callback. */
		if (__CFBitIsSet(service->_flags, kFlagBitAComplete) &&
			__CFBitIsSet(service->_flags, kFlagBitAAAAComplete))
		{
			/* Grab the callback */
			cb = service->_callback;
			
			/* Save the error and client information for the callback */
			memmove(&error, &(service->_error), sizeof(error));
			info = service->_client.info;
		}
	}
	
	/* Unlock the service so the callback can be made safely. */
	__CFSpinUnlock(&service->_lock);
	
	/* If there is a callback, inform the client. */
	if (cb) {
		
		/* Inform the client. */
		cb((CFNetServiceRef)service, &error, info);
	}
	
	/* Go ahead and release now that the callback is done. */
	CFRelease(service);
}


/* static */ void
_LongTimerCallBack(CFRunLoopTimerRef timer, void *context) {
	
	__CFNetService* service = (__CFNetService*)context;
	CFNetServiceClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	/*
	 ** Retain here to guarantee safety really after the source release,
	 ** but definitely before the callback.
	 */
	CFRetain(service);
	
	/* Lock the service */
	__CFSpinLock(&service->_lock);
	
	if (!__CFBitIsSet(service->_flags, kFlagBitAReceived) &&
		!__CFBitIsSet(service->_flags, kFlagBitAAAAReceived) &&
		CFArrayGetCount(service->_sources))
	{
		int i;
		CFArrayRef list = CFDictionaryGetValue(service->_info, (const void*)_kCFNetServiceAddress);
		DNSServiceRef* lookups[] = {
			&(service->_new_service),
			&(service->_a),
			&(service->_aaaa)
		};
		
		/* If no results were retrieved, mark as a timeout error. */
		if (service->_new_service || !list || (CFArrayGetCount(list) == 0)) {
			service->_error.error = kCFNetServicesErrorTimeout;
			service->_error.domain = kCFStreamErrorDomainNetServices;
		}

		/* Remove all the sources from run loops and modes */
		_UnscheduleSources(service->_sources, service->_schedules);
		
		/* Invalidate everything */
		_InvalidateSources(service->_sources);
		
		/* Stop and release the underlying mdns queries */
		for (i = 0; i < (sizeof(lookups) / sizeof(lookups[0])); i++) {
			if (*lookups[i]) {
				DNSServiceRefDeallocate(*lookups[i]);
				*lookups[i] = NULL;
			}
		}
		
		/* Grab the callback */
		cb = service->_callback;
		
		/* Save the error and client information for the callback */
		memmove(&error, &(service->_error), sizeof(error));
		info = service->_client.info;
	}
	
	/* Unlock the service so the callback can be made safely. */
	__CFSpinUnlock(&service->_lock);
	
	/* If there is a callback, inform the client. */
	if (cb) {
		
		/* Inform the client. */
		cb((CFNetServiceRef)service, &error, info);
	}
	
	/* Go ahead and release now that the callback is done. */
	CFRelease(service);
}


/* static */ void
_ShortTimerCallBack(CFRunLoopTimerRef timer, void *context) {
	
	__CFNetService* service = (__CFNetService*)context;
	CFNetServiceClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	Boolean a_complete, aaaa_complete;
	
	/*
	 ** Retain here to guarantee safety really after the source release,
	 ** but definitely before the callback.
	 */
	CFRetain(service);
	
	/* Lock the service */
	__CFSpinLock(&service->_lock);
	
	/*
	** START 4031485
	**
	** On occasions, the short timer fires just before the CFSocket for the query
	** fires.  The last, outstanding query has actually completed, but it just
	** hasn't been serviced.  This change will quickly poll the CFSocket in order
	** to pump along the answer before the service resolve is timed out.
	**
	** NOTE that this change is entirely in its own context.
	*/
	{
		int fd = -1;					/* Used to find the matching CFSocketRef. */
		CFSocketCallBack c = NULL;		/* Call out to poll. */
		
		/* Check for the type of the outstanding query. */
		if (service->_a) {
			fd = DNSServiceRefSockFD(service->_a);
			c = _AQuerySocketCallBack;
		}
		else if (service->_aaaa) {
			fd = DNSServiceRefSockFD(service->_aaaa);
			c = _AAAAQuerySocketCallBack;
		}
		
		/* Need to poll the line if there is an fd. */
		if (fd != -1) {
		
			int val;
			fd_set	set;
			fd_set* setptr = &set;
			
			struct timeval timeout = {0, 0};
			
			FD_ZERO(setptr);
			
			if (fd >= FD_SETSIZE) {
				
				val = howmany(fd + 1, NFDBITS) * sizeof(fd_mask);
				
				setptr = (fd_set*)malloc(val);
				bzero(setptr, val);
			}
			
			FD_SET(fd, setptr);
			
			val = select(fd + 1, setptr, NULL, NULL, &timeout);
			
			if (setptr != &set)
				free(setptr);
				
			if (val > 0) {
			
				CFSocketRef s = NULL;
				CFTypeID socket_type = CFSocketGetTypeID();
				CFIndex i, count = CFArrayGetCount(service->_sources);
				
				/* Go trolling through the sources for the corresponding CFSocketRef. */
				for (i = 0; i < count; i++) {
					CFTypeRef obj = (CFTypeRef)CFArrayGetValueAtIndex(service->_sources, i);
					if ((CFGetTypeID(obj) == socket_type) && (CFSocketGetNative((CFSocketRef)obj) == fd)) {
						s = (CFSocketRef)obj;
						break;
					}
				}
				
				/* This should always hit, but late in Tiger, it's needed. */
				if (s) {
				
					/* Unlock the service so the callback can be made safely. */
					__CFSpinUnlock(&service->_lock);
					
					/* Make the callback which could alter the state of the service. */
					c(s, kCFSocketReadCallBack, NULL, NULL, service);
			
					/* Lock the service */
					__CFSpinLock(&service->_lock);
				}
			}
		}
	}
	
	/* As a result of the possible call to the address callback, the timeout may not occur. */
	
	/* END 4031485 */
	
	a_complete = __CFBitIsSet(service->_flags, kFlagBitAComplete);
	aaaa_complete = __CFBitIsSet(service->_flags, kFlagBitAAAAComplete);
	
	if (CFArrayGetCount(service->_sources) &&
		((a_complete && aaaa_complete) ||
		 (a_complete && !__CFBitIsSet(service->_flags, kFlagBitAAAAReceived)) ||
		 (aaaa_complete && !__CFBitIsSet(service->_flags, kFlagBitAReceived))))
	{
		int i;
		DNSServiceRef* lookups[] = {
			&(service->_new_service),
			&(service->_a),
			&(service->_aaaa)
		};
		
		/* Remove all the sources from run loops and modes */
		_UnscheduleSources(service->_sources, service->_schedules);
		
		/* Invalidate everything */
		_InvalidateSources(service->_sources);
		
		/* Stop and release the underlying mdns queries */
		for (i = 0; i < (sizeof(lookups) / sizeof(lookups[0])); i++) {
			if (*lookups[i]) {
				DNSServiceRefDeallocate(*lookups[i]);
				*lookups[i] = NULL;
			}
		}
		
		/* Grab the callback */
		cb = service->_callback;
		
		/* Save the error and client information for the callback */
		memmove(&error, &(service->_error), sizeof(error));
		info = service->_client.info;
	}
	
	/* Unlock the service so the callback can be made safely. */
	__CFSpinUnlock(&service->_lock);
	
	/* If there is a callback, inform the client. */
	if (cb) {
		
		/* Inform the client. */
		cb((CFNetServiceRef)service, &error, info);
	}
	
	/* Go ahead and release now that the callback is done. */
	CFRelease(service);
}	


/* static */ void
_AddressQueryRecordReply(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
						 DNSServiceErrorType errorCode, const char* fullname, uint16_t rrtype,
						 uint16_t rrclass, uint16_t rdlen, const void* rdata, uint32_t ttl, void* context)
{
	__CFNetService* service = (__CFNetService*)context;
	CFNetServiceClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	/*
	** Retain here to guarantee safety really after the source release,
	** but definitely before the callback.
	*/
	CFRetain(service);
	
	/* Lock the service */
	__CFSpinLock(&service->_lock);
	
	/* Only perform the work if there are outstanding queries */
	if (service->_a || service->_aaaa) {
		
		UInt32 service_flags_copy = service->_flags;
		CFAllocatorRef alloc = CFGetAllocator((CFNetServiceRef)service);
		
		/* Get the list of addresses to add this one */
		CFMutableArrayRef list = (CFMutableArrayRef)CFDictionaryGetValue(service->_info, (const void*)_kCFNetServiceAddress);
		
		/* If there is not a list, need to create one */
		if (!list) {
			
			/* Create the list */
			list = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
			
			/* If the list was created, add it back to the list of info */
			if (list) {
				CFDictionaryAddValue(service->_info, (const void*)_kCFNetServiceAddress, list);
				CFRelease(list);
			}
		}
		
		/* If there is a list of addresses, create the new address to add */
		if (list) {
			
			/* Create the address from the record data */
			CFDataRef addr = _CFDataCreateWithRecord(alloc,
													 rrtype,
													 rdlen,
													 rdata,
													 (service->_port & 0x0000FFFF),
													 interfaceIndex);
			
			/* If created an address, add it to the list and release it */
			if (addr) {
				CFArrayAppendValue(list, addr);
				CFRelease(addr);
			}
		}
		
		if (rrtype == ns_t_a)
			__CFBitSet(service->_flags, kFlagBitAReceived);
		else if (rrtype == ns_t_aaaa)
			__CFBitSet(service->_flags, kFlagBitAAAAReceived);
		
		/* If done or there was an error, need to close down the query */
		if (!(flags & kDNSServiceFlagsMoreComing) || errorCode) {
			
			/* Kill the mdns service */
			_ServiceCancelDNSService_NoLock(service, sdRef);
			
			/* If it's an A record lookup, clear and mark as done. */
			if (rrtype == ns_t_a) {
				service->_a = NULL;
				__CFBitSet(service->_flags, kFlagBitAComplete);
			}
			
			/* If it's a AAAA record, clear and mark that one. */
			else if (rrtype == ns_t_aaaa) {
				service->_aaaa = NULL;
				__CFBitSet(service->_flags, kFlagBitAAAAComplete);
			}
		}
		
		/* If all lookups are done, need to perform the callback. */
		if (__CFBitIsSet(service->_flags, kFlagBitAComplete) &&
			__CFBitIsSet(service->_flags, kFlagBitAAAAComplete) &&
			!service->_new_service)
		{
			/* Remove the timers */
			_UnscheduleSources(service->_sources, service->_schedules);
			
			/* Go ahead and invalidate the sources */
			_InvalidateSources(service->_sources);

			/* Grab the callback */
			cb = service->_callback;
			
			/* Save the error and client information for the callback */
			memmove(&error, &(service->_error), sizeof(error));
			info = service->_client.info;
		}
		
		/*
		** Not all done, but set the short timer if got the first
		** "no more coming" flag.
		*/
		else if ((!__CFBitIsSet(service_flags_copy, kFlagBitAComplete) &&
				  !__CFBitIsSet(service_flags_copy, kFlagBitAAAAComplete)) &&
				  (__CFBitIsSet(service->_flags, kFlagBitAComplete) ||
				   __CFBitIsSet(service->_flags, kFlagBitAAAAComplete)))
		{
			
			CFRunLoopTimerContext c = {0, service, NULL, NULL, NULL};
			
			/* Create the timer used for the longest amount of time willing to wait. */
			CFRunLoopTimerRef timer = CFRunLoopTimerCreate(alloc,
														   CFAbsoluteTimeGetCurrent() + kCFNetServiceShortTimeout,
														   0.0,
														   0,
														   0,
														   _ShortTimerCallBack,
														   &c);
			
			/* Need to add the timer to the list of sources on success */
			if (timer) {
				
				CFArrayAppendValue(service->_sources, timer);
				
				/* Need to schedule it */
				_CFTypeScheduleOnMultipleRunLoops(timer, service->_schedules);
				
				CFRelease(timer);
			}
		}
	}
	
	/* Unlock the service. */
	__CFSpinUnlock(&service->_lock);
	
	/* If there is a callback, inform the client of the error. */
	if (cb) {
		
		/* Inform the client. */
		cb((CFNetServiceRef)service, &error, info);
	}
	
	/* Go ahead and release now that the callback is done. */
	CFRelease(service);
}


#if 0
#pragma mark * Utility Functions
#endif


/* static */ CFDataRef
_CFDataCreateWithRecord(CFAllocatorRef allocator, uint16_t rrtype, uint16_t rdlen,
						const void* rdata, u_short port, uint32_t interfaceIndex)
{	
	CFDataRef result = NULL;
	UInt8 buffer[512];
	struct sockaddr* sa = (struct sockaddr*)(&buffer[0]);
	UInt8* addr = NULL;
	
	memset(sa, 0, sizeof(buffer));
	
	/* Need to bundle up the A record into a sockaddr */
	if (rrtype == ns_t_a) {
		
		if (rdlen == sizeof(struct in_addr)) {
			
			sa->sa_len = sizeof(struct sockaddr_in);
			sa->sa_family = AF_INET;
			
			((struct sockaddr_in*)sa)->sin_port = htons(port);
			
			addr = (UInt8*)(&(((struct sockaddr_in*)sa)->sin_addr));
		}
	}
	
	/* Need to bundle up the AAAA record into a sockaddr */
	else if (rrtype == ns_t_aaaa) {
		
		if (rdlen == sizeof(struct in6_addr)) {
			
			sa->sa_len = sizeof(struct sockaddr_in6);
			sa->sa_family = AF_INET6;
			
			((struct sockaddr_in6*)sa)->sin6_port = htons(port);
			((struct sockaddr_in6*)sa)->sin6_scope_id = htonl(interfaceIndex);
			
			addr = (UInt8*)(&(((struct sockaddr_in6*)sa)->sin6_addr));
		}
	}
	
	if (addr) {
		memmove(addr, rdata, rdlen);
		result = CFDataCreate(allocator, (const UInt8*)sa, sa->sa_len);
	}
	
	return result;
}


/* static */ void
_AddRecords(const void *key, const void *value, void *context) {
	
	if (((0xFFFF0000 & (UInt32)key) == 0x00010000) &&
		(((__CFNetService*)context)->_error.error == 0))
	{
		DNSRecordRef record;
		
		((__CFNetService*)context)->_error.error = DNSServiceAddRecord(((__CFNetService*)context)->_new_service,
																	   &record,
																	   0,
																	   (0x0000FFFF & (UInt32)key),
																	   CFDataGetLength((CFDataRef)value),
																	   CFDataGetBytePtr((CFDataRef)value),
																	   0);
		
		if (!((__CFNetService*)context)->_error.error)
			CFDictionaryAddValue(((__CFNetService*)context)->_records, key, record);
	}
}


/* static */ void
_DictionaryApplier(const void *key, const void *value, void *context) {
	
	/* Get the type in order to figure out how to copy */
	CFTypeID t = CFGetTypeID((CFTypeRef)value);
	
	/* Strings get a copy */
	if (t == CFStringGetTypeID()) {
		CFStringRef c = CFStringCreateCopy(CFGetAllocator((CFTypeRef)context), (CFStringRef)value);
		
		/* Only added if copy succeeded. */
		if (c) {
			CFDictionaryAddValue((CFMutableDictionaryRef)context, key, c);
			CFRelease(c);
		}
	}
	
	/* Arrays get a copy */
	else if (t == CFArrayGetTypeID()) {
		CFArrayRef c = CFArrayCreateCopy(CFGetAllocator((CFTypeRef)context), (CFArrayRef)value);
		
		/* Only added if copy succeeded. */
		if (c) {
			CFDictionaryAddValue((CFMutableDictionaryRef)context, key, c);
			CFRelease(c);
		}
	}
	
	else
		CFDictionaryAddValue((CFMutableDictionaryRef)context, key, value);
}


/* static */ void
_ScheduleSources(CFArrayRef sources, CFArrayRef schedules) {
	
	int i, count = CFArrayGetCount(sources);
	for (i = 0; i < count; i++)
		_CFTypeScheduleOnMultipleRunLoops(CFArrayGetValueAtIndex(sources, i), schedules);
}


/* static */ void
_UnscheduleSources(CFArrayRef sources, CFArrayRef schedules) {
	
	int i, count = CFArrayGetCount(sources);
	for (i = 0; i < count; i++)
		_CFTypeUnscheduleFromMultipleRunLoops(CFArrayGetValueAtIndex(sources, i), schedules);
}


/* static */ void
_InvalidateSources(CFMutableArrayRef sources) {
	
	int i, count = CFArrayGetCount(sources);
	for (i = 0; i < count; i++)
		_CFTypeInvalidate(CFArrayGetValueAtIndex(sources, i));
	
	/* Dump all the sources. */
	CFArrayRemoveAllValues(sources);
}


#if 0
#pragma mark * Legacy Support
#endif


/* static */ void
_MachPortCallBack(CFMachPortRef port, void *msg, CFIndex size, void *info) {

	/* Call Service Discovery to do the dispatch. */
	DNSServiceDiscovery_handleReply(msg);
}


/* static */ void
_LegacyRegistrationReply(int errorCode, void* context) {

	__CFNetService* service = (__CFNetService*)context;
	CFNetServiceClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	/*
	** Retain here to guarantee safety really after the source release,
	** but definitely before the callback.
	*/
	CFRetain(service);
	
	/* Lock the service */
	__CFSpinLock(&service->_lock);
	
	/* If the register canceled, don't need to do any of this. */
	if (service->_old_service) {
		
		/* If there is an error, fold the registration. */
		if (errorCode) {
			
			/* Save the error.  3869179 All errors under Jaguar and Panther were collisions. */
			service->_error.error = kCFNetServicesErrorCollision;
			service->_error.domain = kCFStreamErrorDomainNetServices;
			
			/* Remove the registration from run loops and modes */
			_UnscheduleSources(service->_sources, service->_schedules);
			
			/* Go ahead and invalidate the sources */
			_InvalidateSources(service->_sources);

			/* Clean up the underlying service discovery stuff */
			DNSServiceDiscoveryDeallocate_Deprecated(service->_old_service);
			service->_old_service = NULL;
			
			/* Clear the flags */
			__CFBitClear(service->_flags, kFlagBitLegacyService);
			__CFBitClear(service->_flags, kFlagBitActiveResolve);
			
			/* Grab the callback */
			cb = service->_callback;
			
			/* Save the error and client information for the callback */
			memmove(&error, &(service->_error), sizeof(error));
			info = service->_client.info;
		}
	}
	
	/* Unlock the service so the callback can be made safely. */
	__CFSpinUnlock(&service->_lock);
	
	/* If there is a callback, inform the client of the error. */
	if (cb) {
			
		/* Inform the client. */
		cb((CFNetServiceRef)service, &error, info);
	}
	
	/* Go ahead and release now that the callback is done. */
	CFRelease(service);
}


/* static */ void
_LegacyResolverReply(struct sockaddr* interface, struct sockaddr* address, const char* txtRecord,
					 DNSServiceDiscoveryReplyFlags flags, void* context)
{
	__CFNetService* service = (__CFNetService*)context;
	CFNetServiceClientCallBack cb = NULL;
	CFStreamError error;
	void* info = NULL;
	
	/*
	** Retain here to guarantee safety really after the source release,
	** but definitely before the callback.
	*/
	CFRetain(service);
	
	/* Lock the service */
	__CFSpinLock(&service->_lock);
	
	/* If the register canceled, don't need to do any of this. */
	if (service->_old_service) {
		
		CFAllocatorRef alloc = CFGetAllocator((CFNetServiceRef)service);
		
		/* Get the list of addresses to add this one */
		CFMutableArrayRef list = (CFMutableArrayRef)CFDictionaryGetValue(service->_info, (const void*)_kCFNetServiceAddress);
		
		/* If there is not a list, need to create one */
		if (!list) {
			
			/* Create the list */
			list = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
			
			/* If the list was created, add it back to the list of info */
			if (list) {
				CFDictionaryAddValue(service->_info, (const void*)_kCFNetServiceAddress, list);
				CFRelease(list);
			}
		}
		
		/* If there is a list, need to add the address */
		if (list) {

			int i;
			
			/* Search for this address in the list. */
			for (i = CFArrayGetCount(list) - 1; i >= 0; i--) {
				
				struct sockaddr* saved = (struct sockaddr*)CFDataGetBytePtr((CFDataRef)CFArrayGetValueAtIndex(list, i));
				
				/*
				** The length on an AF_INET address structure is 16 but only the first 8 bytes
				** are used.  The rest are supposed to be zero, but mDNSResponder does not
				** zero them.
				*/
				int compare = (saved->sa_family == AF_INET) ? 8 : saved->sa_len;

				/* Break if found */
				if (!memcmp(saved, address, compare))
					break;
			}
	
			/* If it wasn't found, need to add it. */
			if (i < 0) {
				
				/* Wrap the sockaddr */
				CFDataRef data = CFDataCreate(alloc, (const UInt8*)address, address->sa_len);
				
				/* Add the address to the list if wrapped. */
				if (data) {
					CFArrayAppendValue(list, data);
					CFRelease(data);
				}
			}
		}
		
		/* Remove the old TXT data. */
		CFDictionaryRemoveValue(service->_info, (const void*)_kCFNetServiceTXT);
		
		/* If there was a TXT record, need to wrap it as a string. */
		if (txtRecord) {
			
			/* Create the string */
			CFStringRef t = CFStringCreateWithCString(alloc, txtRecord, kCFStringEncodingUTF8);
			
			/* If it worked, add it back into the info */
			if (t) {
				CFDictionaryAddValue(service->_info, (const void*)_kCFNetServiceTXT, t);
				CFRelease(t);
			}
		}
		
		/* Grab the callback */
		cb = service->_callback;
		
		/* If not an asynchronous resolve, need to finish up now */
		if (!cb) {
			
			/* Remove the resolve from run loops and modes */
			_UnscheduleSources(service->_sources, service->_schedules);
			
			/* Go ahead and invalidate the sources */
			_InvalidateSources(service->_sources);
			
			/* Clean up the underlying service discovery stuff */
			DNSServiceDiscoveryDeallocate_Deprecated(service->_old_service);
			service->_old_service = NULL;
			
			/* Clear the flags */
			__CFBitClear(service->_flags, kFlagBitLegacyService);
			__CFBitClear(service->_flags, kFlagBitActiveRegister);
		}
		
		/* Save the error and client information for the callback */
		memmove(&error, &(service->_error), sizeof(error));
		info = service->_client.info;
	}
	
	/* Unlock the service so the callback can be made safely. */
	__CFSpinUnlock(&service->_lock);
	
	/* If there is a callback, inform the client of the error. */
	if (cb) {
			
		/* Inform the client. */
		cb((CFNetServiceRef)service, &error, info);
	}
	
	/* Go ahead and release now that the callback is done. */
	CFRelease(service);
}


#if 0
#pragma mark * TXT Dictionary Key Callbacks
#endif

/* static */ const void*
TXTDictionaryKeyRetain(CFAllocatorRef allocator, CFStringRef key) {
	(void)allocator;	/* unsused */
	return (const void*)CFRetain(key);
}


/* static */ void
TXTDictionaryKeyRelease(CFAllocatorRef allocator, CFStringRef key) {
	(void)allocator;	/* unused */
	CFRelease(key);
}


/* static */ Boolean
TXTDictionaryKeyEqual(CFStringRef key1, CFStringRef key2) {
	return (CFStringCompare(key1, key2, kCFCompareCaseInsensitive) == kCFCompareEqualTo);
}


/* static */ CFHashCode
TXTDictionaryKeyHash(CFStringRef key) {
	return (CFHashCode)CFStringGetLength(key);
}


#if 0
#pragma mark -
#pragma mark Extern Function Definitions (API)
#endif


/* extern */ CFTypeID
CFNetServiceGetTypeID(void) {

    _CFDoOnce(&_kCFNetServiceRegisterClass, _CFNetServiceRegisterClass);

    return _kCFNetServiceTypeID;
}


/* extern */ CFNetServiceRef
CFNetServiceCreate(CFAllocatorRef alloc, CFStringRef domain, CFStringRef type, CFStringRef name, UInt32 port) {
	
	CFNetServiceRef result = NULL;
	
	/* Domain, type, and name must be specified */
	if (domain && type && name) {
		
		/* Create copies for normalization */
		CFMutableStringRef d = CFStringCreateMutableCopy(alloc, 0, domain);
		CFMutableStringRef t = CFStringCreateMutableCopy(alloc, 0, type);
		CFMutableStringRef n = CFStringCreateMutableCopy(alloc, 0, name);
		
		if (d && t && n) {
			
			/* Normalization for on-the-wire transfer */
			CFStringNormalize(d, kCFStringNormalizationFormC);
			CFStringNormalize(t, kCFStringNormalizationFormC);
			CFStringNormalize(n, kCFStringNormalizationFormC);
			
			result = _CFNetServiceCreateCommon(alloc, d, t, n, port);
		}
			
		/* Release the copies created for normalization */
		if (d)
			CFRelease(d);
		if (t)
			CFRelease(t);
		if (n)
			CFRelease(n);
	}
	
	return (CFNetServiceRef)result;
}


/* extern */ CFNetServiceRef
CFNetServiceCreateCopy(CFAllocatorRef alloc, CFNetServiceRef service) {
	
	__CFNetService* result = NULL;
	__CFNetService* s = (__CFNetService*)service;
	CFTypeID class_type = CFNetServiceGetTypeID();
	
	__CFSpinLock(&(s->_lock));

	if (class_type != _kCFRuntimeNotATypeID) {
		result = (__CFNetService*)_CFRuntimeCreateInstance(alloc,
														   class_type,
														   sizeof(result[0]) - sizeof(CFRuntimeBase),
														   NULL);
	}
	
	if (result) {
		
		CFDictionaryKeyCallBacks keys = {0, NULL, NULL, NULL, NULL, NULL};
		CFDictionaryValueCallBacks values = {0, NULL, NULL, NULL, NULL};

		/* Save a copy of the base so it's easier to zero the struct */
		CFRuntimeBase copy = result->_base;

		/* Clear everything. */
		memset(result, 0, sizeof(result[0]));

		/* Put back the base */
		memmove(&(result->_base), &copy, sizeof(result->_base));

		/* Create the dictionary of information */
		result->_info = CFDictionaryCreateMutable(alloc, 0, &keys, &kCFTypeDictionaryValueCallBacks);

		/* Create the dictionary for holding the published records */
		result->_records = CFDictionaryCreateMutable(alloc, 0, &keys, &values);
		
		/* Copy all the info from the original */
		if (result->_info)
			CFDictionaryApplyFunction(s->_info, _DictionaryApplier, result->_info);

		/* Create the list of loops and modes */
		result->_schedules = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
		
		/* Create the list of sources */
		result->_sources = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
		
		/* Failure needs to release and return NULL. */
		if (!result->_info ||
			!result->_schedules ||
			!result->_sources ||
			!result->_records ||
			(CFDictionaryGetCount(result->_info) != CFDictionaryGetCount(s->_info)))
		{
			CFRelease((CFTypeRef)result);
			result = NULL;
		}
	}
	
	__CFSpinUnlock(&(s->_lock));

	return (CFNetServiceRef)result;
}


/* extern */ CFStringRef
CFNetServiceGetDomain(CFNetServiceRef theService) {
	
	return (CFStringRef)CFNetServiceGetInfo(theService, _kCFNetServiceDomain);
}


/* extern */ CFStringRef
CFNetServiceGetType(CFNetServiceRef theService) {
	
	return (CFStringRef)CFNetServiceGetInfo(theService, _kCFNetServiceType);
}


/* extern */ CFStringRef
CFNetServiceGetName(CFNetServiceRef theService) {

	return (CFStringRef)CFNetServiceGetInfo(theService, _kCFNetServiceName);
}


/* extern */ CFArrayRef
CFNetServiceGetAddressing(CFNetServiceRef theService) {

	return (CFArrayRef)CFNetServiceGetInfo(theService, _kCFNetServiceAddress);
}

/* extern */ CFStringRef 
CFNetServiceGetTargetHost(CFNetServiceRef theService) {
	
	return (CFStringRef)CFNetServiceGetInfo(theService, _kCFNetServiceTargetHost);
}


/* extern */ CFDataRef 
CFNetServiceGetTXTData(CFNetServiceRef theService) {
	
	CFTypeRef result = CFNetServiceGetInfo(theService, _kCFNetServiceTXT);
	
	/*
	** This shouldn't really happen.  This is here in order to protect
	** against using the new TXT calls in conjunction with the old
	** deprecated calls.
	*/
	if (result && (CFGetTypeID(result) != CFDataGetTypeID()))
		result = NULL;
	
	return (CFDataRef)result;
}


/* extern */ Boolean 
CFNetServiceSetTXTData(CFNetServiceRef theService, CFDataRef txtRecord) {
	
	return CFNetServiceSetInfo(theService, _kCFNetServiceTXT, txtRecord);
}


/* extern */ Boolean 
CFNetServiceRegisterWithOptions(CFNetServiceRef theService, CFOptionFlags options, CFStreamError* error) {
	
	__CFNetService* service = (__CFNetService*)theService;
	
	CFStreamError extra;
	Boolean result = FALSE;
	
	if (!error)
		error = &extra;
	
	memset(error, 0, sizeof(error[0]));
	
	/*
	** Retain so it doesn't go away underneath in the case of a callout.  This is really
	** no worry for async, but makes the memmove for the error more difficult to place
	** for synchronous without it being here.
	*/
	CFRetain(theService);
	
	/* Lock down the service to start */
	__CFSpinLock(&(service->_lock));
	
	do {
		
		int i;
		char properties[3][1024];
		CFSocketRef sock;
		CFSocketContext ctxt = {0, service, CFRetain, CFRelease, NULL};
		UInt32 keys[] = {_kCFNetServiceName, _kCFNetServiceType, _kCFNetServiceDomain};
		DNSServiceFlags flags = ((options == kCFNetServiceFlagNoAutoRename) ? kDNSServiceFlagsNoAutoRename : 0);
		CFDataRef txt = (CFDataRef)CFDictionaryGetValue(service->_info, (const void*)_kCFNetServiceTXT);
		
		/* Check to see if there is an ongoing process already */
		if (CFArrayGetCount(service->_sources)) {
			
			/* If there's already mdns activity, don't allow another. */
			if (!__CFBitIsSet(service->_flags, kFlagBitCancel)) {
				service->_error.error = kCFNetServicesErrorInProgress;
				service->_error.domain = kCFStreamErrorDomainNetServices;
			}
			
			/* It's just the cancel that hasn't fired yet, so cancel it. */
			else {
				
				/* Remove the cancel from run loops and modes */
				_UnscheduleSources(service->_sources, service->_schedules);
				
				/* Invalidate the run loop source */
				_InvalidateSources(service->_sources);
			}
		}
		
		/* Get the raw data for the properties to send down to mdns */
		for (i = 0; i < (sizeof(keys) / sizeof(keys[0])); i++) {
			
			CFStringRef value = (CFStringRef)CFDictionaryGetValue(service->_info, (const void*)(keys[i]));
			if (!value)
				properties[i][0] = '\0';
			else {
				CFIndex used;
				CFStringGetBytes(value,
								 CFRangeMake(0, CFStringGetLength(value)),
								 kCFStringEncodingUTF8,
								 0,
								 FALSE,
								 (UInt8*)properties[i],
								 sizeof(properties[i]) - 1,
								 &used);
				properties[i][used] = '\0';
			}
		}
		
		/* Create the registration */
		service->_error.error = DNSServiceRegister(&(service->_new_service),
												   flags,
												   0,
												   properties[0],
												   properties[1],
												   properties[2],
												   NULL,
												   htons((service->_port & 0x0000FFFF)),
												   txt ? CFDataGetLength(txt) : 0,
												   txt ? CFDataGetBytePtr(txt) : NULL,
												   _RegisterReply,
												   service);
		
		if (service->_error.error) {
			service->_error.domain = kCFStreamErrorDomainNetServices;
			break;
		}
		
		CFDictionaryApplyFunction(service->_info, _AddRecords, service);
		if (service->_error.error) {
			
			service->_error.domain = kCFStreamErrorDomainNetServices;
			
			/* Stop right away on failure */
			DNSServiceRefDeallocate(service->_new_service);
			service->_new_service = NULL;
			
			/* Dump all the records that may have been published */
			CFDictionaryRemoveAllValues(service->_records);
			
			break;
		}
		
		/* Create a CFSocket wrapper on the register */
		sock = CFSocketCreateWithNative(CFGetAllocator(theService),
										DNSServiceRefSockFD(service->_new_service),
										kCFSocketReadCallBack,
										_SocketCallBack,
										&ctxt);
		
		/* Need to bail if it failed */
		if (!sock) {
			
			/* Set error to whatever happened. */
			service->_error.error = errno;
			if (!service->_error.error)
				service->_error.error = ENOMEM;
			
			service->_error.domain = kCFStreamErrorDomainPOSIX;
			
			/* Stop right away on failure */
			DNSServiceRefDeallocate(service->_new_service);
			service->_new_service = NULL;
			
			/* Dump all the records that may have been published */
			CFDictionaryRemoveAllValues(service->_records);
			
			break;
		}
		
		/* Tell CFSocket not to close the native socket on invalidation. */
		CFSocketSetSocketFlags(sock, CFSocketGetSocketFlags(sock) & ~kCFSocketCloseOnInvalidate);
		
		/* Add the socket to the list of sources */
		CFArrayAppendValue(service->_sources, sock);
		CFRelease(sock);
		
		/* Start with no error. */
		service->_error.error = 0;
		service->_error.domain = 0;
		
		/* Set the flags indicating a new, Panther-type registration */
		__CFBitClear(service->_flags, kFlagBitLegacyService);
		__CFBitSet(service->_flags, kFlagBitActiveRegister);
		
		/* Async mode is complete at this point */
		if (CFArrayGetCount(service->_schedules)) {
			
			/* Schedule the sources on the run loops and modes. */
			_ScheduleSources(service->_sources, service->_schedules);
			
			/* It's now succeeded. */
			result = TRUE;
		}
		
		/* Go into synchronous mode. */
		else {
			
			/* Unlock the service */
			__CFSpinUnlock(&(service->_lock));
			
			/* Wait for synchronous return */
			result = _ServiceBlockUntilComplete(service);
			
			/* Lock down the service */
			__CFSpinLock(&(service->_lock));
		}
		
	} while (0);
	
	/* Copy the error. */
	memmove(error, &service->_error, sizeof(error[0]));
	
	/* Unlock the service */
	__CFSpinUnlock(&(service->_lock));
	
	/* Release the earlier retain. */
	CFRelease(theService);
	
	return result;
}


/* extern */ Boolean 
CFNetServiceResolveWithTimeout(CFNetServiceRef theService, CFTimeInterval timeout, CFStreamError* error) {
	
	__CFNetService* service = (__CFNetService*)theService;
	
	CFStreamError extra;
	Boolean result = FALSE;
	
	if (!error)
		error = &extra;
	
	memset(error, 0, sizeof(error[0]));
	
	/*
	** Retain so it doesn't go away underneath in the case of a callout.  This is really
	** no worry for async, but makes the memmove for the error more difficult to place
	** for synchronous without it being here.
	*/
	CFRetain(theService);
	
	/* Lock down the service to start */
	__CFSpinLock(&(service->_lock));
	
	do {
		
		int i;
		char properties[3][1024];
		UInt32 keys[] = {_kCFNetServiceName, _kCFNetServiceType, _kCFNetServiceDomain};
		
		/* Check to see if there is an ongoing process already */
		if (CFArrayGetCount(service->_sources)) {
		
			/* If there's already mdns activity, don't allow another. */
			if (!__CFBitIsSet(service->_flags, kFlagBitCancel)) {
				service->_error.error = kCFNetServicesErrorInProgress;
				service->_error.domain = kCFStreamErrorDomainNetServices;
			}
			
			/* It's just the cancel that hasn't fired yet, so cancel it. */
			else {
				
				/* Remove the cancel from run loops and modes */
				_UnscheduleSources(service->_sources, service->_schedules);
				
				/* Invalidate the run loop source */
				_InvalidateSources(service->_sources);
			}
		}
		
		/* Get the raw data for the properties to send down to mdns */
		for (i = 0; i < (sizeof(keys) / sizeof(keys[0])); i++) {
			
			CFStringRef value = (CFStringRef)CFDictionaryGetValue(service->_info, (const void*)(keys[i]));
			if (!value)
				properties[i][0] = '\0';
			else {
				CFIndex used;
				CFStringGetBytes(value,
								 CFRangeMake(0, CFStringGetLength(value)),
								 kCFStringEncodingUTF8,
								 0,
								 FALSE,
								 (UInt8*)properties[i],
								 sizeof(properties[i]) - 1,
								 &used);
				properties[i][used] = '\0';
			}
		}
		
		_ServiceCreateQuery_NoLock(service, ns_t_invalid, properties[0],
								   properties[1],  properties[2], FALSE);
		
		/* If a timeout is set, need to start the timer. */
		if (!service->_error.error && (timeout > 0.0)) {
			
			CFRunLoopTimerContext c = {0, service, NULL, NULL, NULL};
			
			/* Create the timer used for the longest amount of time willing to wait. */
			CFRunLoopTimerRef timer = CFRunLoopTimerCreate(CFGetAllocator(theService),
														   CFAbsoluteTimeGetCurrent() + timeout,
														   0.0,
														   0,
														   0,
														   _LongTimerCallBack,
														   &c);
			
			__CFBitClear(service->_flags, kFlagBitAReceived);
			__CFBitClear(service->_flags, kFlagBitAAAAReceived);
			
			/* Need to add the timer to the list of sources on success */
			if (timer) {
				CFArrayAppendValue(service->_sources, timer);
				CFRelease(timer);
			}
			
			/* Need to clean up if there was a failure. */
			else {
				
				/* Set error to whatever happened. */
				service->_error.error = errno;
				if (!service->_error.error)
					service->_error.error = ENOMEM;
				
				service->_error.domain = kCFStreamErrorDomainPOSIX;
			}
		}
		
		/* Need to bail if it there was an error */
		if (service->_error.error) {
			
			/* Invalidate everything */
			_InvalidateSources(service->_sources);
			
			/* Stop and release the underlying mdns query */
			DNSServiceRefDeallocate(service->_new_service);
			service->_new_service = NULL;
			
			break;
		}
		
		/* Remove any addresses so the resolve fills in new. */
		CFDictionaryRemoveValue(service->_info, (const void*)_kCFNetServiceAddress);
		
		/* Remove any TXT record. */
		CFDictionaryRemoveValue(service->_info, (const void*)_kCFNetServiceTXT);
		
		/* Remove the target host. */
		CFDictionaryRemoveValue(service->_info, (const void*)_kCFNetServiceTargetHost);
		
		/* Don't want a port until it's resolved */
		service->_port = 0;
		
		/* Start with no error. */
		service->_error.error = 0;
		service->_error.domain = 0;
		
		/* Set the flags indicating a new, Panther-type resolve */
		__CFBitClear(service->_flags, kFlagBitLegacyService);
		__CFBitSet(service->_flags, kFlagBitActiveResolve);
		
		/* Clear flags indicating address lookup completion. */
		__CFBitClear(service->_flags, kFlagBitAComplete);
		__CFBitClear(service->_flags, kFlagBitAAAAComplete);

		/* Async mode is complete at this point */
		if (CFArrayGetCount(service->_schedules)) {
			
			/* Schedule the sources on the run loops and modes. */
			_ScheduleSources(service->_sources, service->_schedules);
			
			/* It's now succeeded. */
			result = TRUE;
		}
		
		/* Go into synchronous mode. */
		else {
			
			/* Unlock the service */
			__CFSpinUnlock(&(service->_lock));
			
			/* Wait for synchronous return */
			result = _ServiceBlockUntilComplete(service);
			
			/* Lock down the service */
			__CFSpinLock(&(service->_lock));
		}
		
	} while (0);
	
	/* Copy the error. */
	memmove(error, &service->_error, sizeof(error[0]));
	
	/* Unlock the service */
	__CFSpinUnlock(&(service->_lock));
	
	/* Release the earlier retain. */
	CFRelease(theService);
	
	return result;
}


/* extern */ void
CFNetServiceCancel(CFNetServiceRef theService) {
	
	__CFNetService* service = (__CFNetService*)theService;
	
	/* Lock down the service */
	__CFSpinLock(&(service->_lock));
	
	/* Make sure there is something to cancel. */
	if (CFArrayGetCount(service->_sources)) {
		
		CFRunLoopSourceRef src = NULL;
		CFRunLoopSourceContext ctxt = {
			0,									/* version */
			service,							/* info */
			NULL,								/* retain */
			NULL,								/* release */
			NULL,								/* copyDescription */
			NULL,								/* equal */
			NULL,								/* hash */
			NULL,								/* schedule */
			NULL,								/* cancel */
			(void(*)(void*))(&_ServiceCancel)  	/* perform */
		};
		
		/* Remove the sources from run loops and modes */
		_UnscheduleSources(service->_sources, service->_schedules);
		
		/* Go ahead and invalidate the sources */
		_InvalidateSources(service->_sources);
		
		if (__CFBitIsSet(service->_flags, kFlagBitLegacyService)) {
			
			/* Need to clean up the service discovery stuff if there is */
			if (service->_old_service) {
				
				/* Release the underlying service discovery reference */
				DNSServiceDiscoveryDeallocate_Deprecated(service->_old_service);
				service->_old_service = NULL;
			}
		}
		else {
			
			int i;
			DNSServiceRef* services[] = {&service->_new_service, &service->_a, &service->_aaaa};
			
			/* Need to clean up the service discovery stuff if there is */
			for (i = 0; i < (sizeof(services) / sizeof(services[0])); i++) {
				
				if (*services[i]) {

					/* Release the underlying service discovery reference */
					DNSServiceRefDeallocate(*services[i]);
					*services[i] = NULL;
				}
			}
			
			/* Dump all the records that may have been published */
			CFDictionaryRemoveAllValues(service->_records);
		}
		
		/* Clear any flags related to the sources */
		__CFBitClear(service->_flags, kFlagBitLegacyService);
		__CFBitClear(service->_flags, kFlagBitActiveResolve);
		__CFBitClear(service->_flags, kFlagBitActiveRegister);
		__CFBitClear(service->_flags, kFlagBitCancel);
		
		/* Mark the service as cancelled */
		service->_error.error = kCFNetServicesErrorCancel;
		service->_error.domain = kCFStreamErrorDomainNetServices;
		
		/* Create the cancel source */
		src = CFRunLoopSourceCreate(CFGetAllocator(theService), 0, &ctxt);
		
		/* If the cancel was created, need to schedule and signal it. */
		if (src) {
			
			CFArrayRef schedules = service->_schedules;
			CFIndex i, count = CFArrayGetCount(schedules);
			
			/* Mark it as being a cancel */
			__CFBitSet(service->_flags, kFlagBitCancel);
			
			/* Add the source to the list of sources */
			CFArrayAppendValue(service->_sources, src);
			
			/* Schedule the new cancel */
			_ScheduleSources(service->_sources, service->_schedules);
			
			/* Signal the cancel for immediate attention. */
			CFRunLoopSourceSignal(src);
			
			/* Make sure the signal can make it through */
			for (i = 0; i < count; i += 2) {
				
				/* Grab the run loop for checking */
				CFRunLoopRef runloop = (CFRunLoopRef)CFArrayGetValueAtIndex(schedules, i);
				
				/* If it's sleeping, need to further check it. */
				if (CFRunLoopIsWaiting(runloop)) {
					
					/* Grab the mode for further check */
					CFStringRef mode = CFRunLoopCopyCurrentMode(runloop);
					
					if (mode) {
						
						/* If the cancel source is in the right mode, need to wake up the run loop. */
						if (CFRunLoopContainsSource(runloop, src, mode)) {
							CFRunLoopWakeUp(runloop);
						}
						
						/* Don't need this anymore. */
						CFRelease(mode);
					}
				}
			}
			
			/* No longer need this */
			CFRelease(src);
		}
	}
	
	/* Unlock the service */
	__CFSpinUnlock(&(service->_lock));
}


/* extern */ Boolean
CFNetServiceSetClient(CFNetServiceRef theService, CFNetServiceClientCallBack clientCB, CFNetServiceClientContext* clientContext) {

	__CFNetService* service = (__CFNetService*)theService;

	/* Lock down the service */
	__CFSpinLock(&(service->_lock));

	/* Release the user's context info if there is some and a release method */
	if (service->_client.info && service->_client.release)
		service->_client.release(service->_client.info);
	
	/* NULL callback or context signals to remove the client */
	if (!clientCB || !clientContext) {
		
		/* Cancel the sources if any */
		if (CFArrayGetCount(service->_sources)) {
			
			/* Remove the sources from run loops and modes */
			_UnscheduleSources(service->_sources, service->_schedules);
			
			/* Go ahead and invalidate the sources */
			_InvalidateSources(service->_sources);
			
			if (__CFBitIsSet(service->_flags, kFlagBitLegacyService)) {
				
				/* Need to clean up the service discovery stuff if there is */
				if (service->_old_service) {
					
					/* Release the underlying service discovery reference */
					DNSServiceDiscoveryDeallocate_Deprecated(service->_old_service);
					service->_old_service = NULL;
				}
			}
			
			else {
				
				/* Need to clean up the service discovery stuff if there is */
				if (service->_new_service) {
					
					/* Release the underlying service discovery reference */
					DNSServiceRefDeallocate(service->_new_service);
					service->_new_service = NULL;
					
					/* Dump all the records that may have been published */
					CFDictionaryRemoveAllValues(service->_records);
				}
			}
		}
		
		/* Clear any flags related to the sources */
		__CFBitClear(service->_flags, kFlagBitLegacyService);
		__CFBitClear(service->_flags, kFlagBitActiveResolve);
		__CFBitClear(service->_flags, kFlagBitActiveRegister);
		__CFBitClear(service->_flags, kFlagBitCancel);
		
		/* Zero out the callback and client context. */
		service->_callback = NULL;
		memset(&(service->_client), 0, sizeof(service->_client));
	}

	else {
		
		/*
		** Schedule any sources on the run loops and modes if they haven't been scheduled
		** already.  If there had previously been a callback, the sources will have
		** already been scheduled.
		*/
		if (!service->_callback && CFArrayGetCount(service->_sources))
			_ScheduleSources(service->_sources, service->_schedules);
		
		/* Save the client's new callback */
		service->_callback = clientCB;

		/* Copy the client's context */
		memmove(&(service->_client), clientContext, sizeof(service->_client));

		/* If there is user data and a retain method, call it. */
		if (service->_client.info && service->_client.retain)
			service->_client.info = (void*)(service->_client.retain(service->_client.info));
	}
	
	/* Unlock the service */
	__CFSpinUnlock(&(service->_lock));

	return TRUE;
}



/* extern */ void
CFNetServiceScheduleWithRunLoop(CFNetServiceRef theService, CFRunLoopRef runLoop, CFStringRef runLoopMode) {

	__CFNetService* service = (__CFNetService*)theService;
	
	/* Lock down the service before work */
	__CFSpinLock(&(service->_lock));
	
	if (_SchedulesAddRunLoopAndMode(service->_schedules, runLoop, runLoopMode)) {

		int i, count = CFArrayGetCount(service->_sources);
		
		/* If there are current processes, need to schedule them. */
		for (i = 0; i < count; i++)
			_CFTypeScheduleOnRunLoop(CFArrayGetValueAtIndex(service->_sources, i), runLoop, runLoopMode);
	}
	
	/* Unlock the service */
	__CFSpinUnlock(&(service->_lock));
}


/* extern */ void
CFNetServiceUnscheduleFromRunLoop(CFNetServiceRef theService, CFRunLoopRef runLoop, CFStringRef runLoopMode) {
	
	__CFNetService* service = (__CFNetService*)theService;

	/* Lock down the service before work */
	__CFSpinLock(&(service->_lock));
	
	if (_SchedulesRemoveRunLoopAndMode(service->_schedules, runLoop, runLoopMode)) {
		
		int i, count = CFArrayGetCount(service->_sources);
		
		/* If there are current processes, need to unschedule them. */
		for (i = 0; i < count; i++)
			_CFTypeUnscheduleFromRunLoop(CFArrayGetValueAtIndex(service->_sources, i), runLoop, runLoopMode);
    }

	/* Unlock the service */
	__CFSpinUnlock(&(service->_lock));
}	



/* extern */ CFDictionaryRef
CFNetServiceCreateDictionaryWithTXTData(CFAllocatorRef alloc, CFDataRef txtRecord) {

	CFMutableDictionaryRef result = NULL;

	CFIndex len = CFDataGetLength(txtRecord);
	const void* txt = CFDataGetBytePtr(txtRecord);
	
	if ((len > 0) && (len < 65536)) {
		
		static const CFDictionaryKeyCallBacks kTXTDictionaryKeyCallBacks = {
			0,
			(CFDictionaryRetainCallBack)TXTDictionaryKeyRetain,
			(CFDictionaryReleaseCallBack)TXTDictionaryKeyRelease,
			CFCopyDescription,
			(CFDictionaryEqualCallBack)TXTDictionaryKeyEqual,
			(CFDictionaryHashCallBack)TXTDictionaryKeyHash
		};
		
		/* Get the number of keys */
		uint16_t i, count = TXTRecordGetCount(len, txt);
		result = CFDictionaryCreateMutable(alloc, 0, &kTXTDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		
		if (result) {
			
			/* Iterate over all the keys */
			for (i = 0; i < count; i++) {
				
				char key[256];
				uint8_t valLen = 0;
				const void* value = NULL;
				
				/* Only go through the other stuff if it could cleanly get the key and value */
				if (kDNSServiceErr_NoError == TXTRecordGetItemAtIndex(len, txt, i, sizeof(key), key, &valLen, &value)) {
					
					/* A null value is kCFNull, otherwise it's a CFDataRef of the given length */
					CFTypeRef data = /*(value != NULL) ? CFDataCreate(alloc, value, valLen) :*/ kCFNull;
					CFStringRef str = CFStringCreateWithCString(alloc, key, kCFStringEncodingUTF8);
					
					if (value)
						data = CFDataCreate(alloc, value, valLen);
					
				
					/* Only add if key and value were created and the key doesn't exists already */
					if (data && str && CFStringGetLength(str) && !CFDictionaryGetValue(result, str))
						CFDictionaryAddValue(result, str, data);
					
					if (data)
						CFRelease(data);
					
					if (str)
						CFRelease(str);
				}
			}
		}
	}
	
	return result;
}


/* extern */ CFDataRef 
CFNetServiceCreateTXTDataWithDictionary(CFAllocatorRef alloc, CFDictionaryRef keyValuePairs) {
	
	CFDataRef result = NULL;

	CFStringRef* keys = NULL;
	CFTypeRef* values = NULL;
	CFIndex count = CFDictionaryGetCount(keyValuePairs);
	
	keys = CFAllocatorAllocate(alloc, count * sizeof(keys[0]), 0);
	values = CFAllocatorAllocate(alloc, count * sizeof(values[0]), 0);
	
	if (keys && values) {
		
		CFIndex i;
		TXTRecordRef txt;
		CFTypeID strType = CFStringGetTypeID();
		CFTypeID dataType = CFDataGetTypeID();
		UInt8 key[256];
		
		/* Grab the key/value pairs for iteration. */
		CFDictionaryGetKeysAndValues(keyValuePairs, (const void**)keys, (const void**)values);
		
		/* Create the txt record */
		TXTRecordCreate(&txt, 0, NULL);
		
		/* Iterate over the key/value pairs. */
		for (i = 0; i < count; i++) {
			
			CFIndex length, converted, used = 0;
			CFTypeID type = CFGetTypeID(values[i]);
			DNSServiceErrorType set = kDNSServiceErr_Unknown;
			
			/* Keys must be CFStrings */
			if (CFGetTypeID(keys[i]) != strType)
				break;
			
			length = CFStringGetLength(keys[i]);
			converted = CFStringGetBytes(keys[i], CFRangeMake(0, length), kCFStringEncodingASCII, 0, FALSE, key, sizeof(key), &used);
			
			/* The key has to be cleanly converted must be between 1 and 255 bytes long */
			if (!length || (converted < length) || (used >= sizeof(key)))
				break;
			
			/* Cap the string for dns_sd. */
			key[used] = '\0';
			
			/* String types are converted to raw bytes */
			if (type == strType) {
				
				UInt8 value[256];
				
				/* Convert the string to raw bytes using UTF8 */
				length = CFStringGetLength(values[i]);
				converted = CFStringGetBytes(values[i], CFRangeMake(0, length), kCFStringEncodingUTF8, 0, FALSE, value, sizeof(value), &used);
				
				/* The value has to be cleanly converted and can't be longer than 255 (0 is permitted) */
				if ((converted < length) || (used >= sizeof(key)))
					break;
					
				/* Set the raw bytes */
				set = TXTRecordSetValue(&txt, (const char*)key, used, value);
			}
			
			/* If it's data, it needs to be in the range of 0 and 255, inclusive. */
			else if ((type == dataType) &&
					 (CFDataGetLength((CFDataRef)(values[i])) < 256) &&
					 (CFDataGetLength((CFDataRef)(values[i])) >= 0))
			{
				/* Set the raw bytes from the data */
				set = TXTRecordSetValue(&txt,
										(const char*)key,
										CFDataGetLength((CFDataRef)(values[i])),
										CFDataGetBytePtr((CFDataRef)(values[i])));
			}
			
			/* Allow null for a key with no value */
			else if (values[i] == kCFNull) {
				
				/* Sets the key to no value */
				set = TXTRecordSetValue(&txt, (const char*)key, 0, NULL);
			}
			
			/* Bad type */
			else
				break;
			
			/* If couldn't set, need to fail out. */
			if (set != kDNSServiceErr_NoError)
				break;
		}
		
		/* If all the keys and values were processed, create the data for the txt record. */
		if (i == count)
			result = CFDataCreate(alloc, TXTRecordGetBytesPtr(&txt), TXTRecordGetLength(&txt));
		
		TXTRecordDeallocate(&txt);
	}
	
	if (keys)
		CFAllocatorDeallocate(alloc, keys);
	
	if (values)
		CFAllocatorDeallocate(alloc, values);
	
	return result;
}


#if 0
#pragma mark -
#pragma mark Extern Function Definitions (SPI)
#endif

/* extern */ CFNetServiceRef
_CFNetServiceCreateCommon(CFAllocatorRef alloc, CFStringRef domain, CFStringRef type, CFStringRef name, UInt32 port) {
	
	__CFNetService* result = NULL;
	
	/* Domain, type, and name must be specified */
	if (domain && type && name) {
		
		CFTypeID class_type = CFNetServiceGetTypeID();
		
		if (class_type != _kCFRuntimeNotATypeID) {
			result = (__CFNetService*)_CFRuntimeCreateInstance(alloc,
															   class_type,
															   sizeof(result[0]) - sizeof(CFRuntimeBase),
															   NULL);
		}
		
		if (result) {
			
			CFDictionaryKeyCallBacks keys = {0, NULL, NULL, NULL, NULL, NULL};
			CFDictionaryValueCallBacks values = {0, NULL, NULL, NULL, NULL};
			
			/* Save a copy of the base so it's easier to zero the struct */
			CFRuntimeBase copy = result->_base;
			
			/* Clear everything. */
			memset(result, 0, sizeof(result[0]));
			
			/* Put back the base */
			memmove(&(result->_base), &copy, sizeof(result->_base));
			
			/* Create the dictionary of information */
			result->_info = CFDictionaryCreateMutable(alloc, 0, &keys, &kCFTypeDictionaryValueCallBacks);
			
			/* Create the dictionary for holding the published records */
			result->_records = CFDictionaryCreateMutable(alloc, 0, &keys, &values);
			
			/* Create the list of loops and modes */
			result->_schedules = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
			
			/* Create the list of sources */
			result->_sources = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
			
			/* Save the information if everything is good. */
			if (result->_info && result->_schedules && result->_records && result->_sources) {
				
				CFDictionaryAddValue(result->_info, (const void*)_kCFNetServiceDomain, domain);
				CFDictionaryAddValue(result->_info, (const void*)_kCFNetServiceType, type);
				CFDictionaryAddValue(result->_info, (const void*)_kCFNetServiceName, name);
				
				result->_port = port;
			}
			
			/* Failure needs to release and return null */
			else {
				CFRelease((CFTypeRef)result);
				result = NULL;
			}
		}
	}
	
	return (CFNetServiceRef)result;
}


/* extern */ CFTypeRef 
CFNetServiceGetInfo(CFNetServiceRef theService, UInt32 property) {
	
	CFTypeRef result = NULL;
	__CFNetService* service = (__CFNetService*)theService;
	
	/* Lock the service. */
	__CFSpinLock(&(service->_lock));
	
	/* Get the important bits */
	result = (CFTypeRef)CFDictionaryGetValue(service->_info, (const void*)property);
	
	/* Unlock the service again. */
	__CFSpinUnlock(&(service->_lock));
	
	return result;
}


/* extern */ Boolean 
CFNetServiceSetInfo(CFNetServiceRef theService, UInt32 property, CFTypeRef value) {	
	
	return _ServiceSetInfo((__CFNetService*)theService, property, value, TRUE);
}


/* extern */ Boolean
_CFNetServiceSetInfoNoPublish(CFNetServiceRef theService, UInt32 property, CFTypeRef value) {
	
	return _ServiceSetInfo((__CFNetService*)theService, property, value, FALSE);
}


#if 0
#pragma mark -
#pragma mark Deprecated API
#endif


/* extern */ dns_service_discovery_ref
_CFNetServiceGetDNSServiceDiscovery(CFNetServiceRef theService) {
	
	dns_service_discovery_ref result = NULL;
	__CFNetService* s = (__CFNetService*)theService;
	
	__CFSpinLock(&(s->_lock));
	
	result = s->_old_service;
	
	__CFSpinUnlock(&(s->_lock));
	
	return result;
}


/* extern */ CFStringRef
CFNetServiceGetProtocolSpecificInformation(CFNetServiceRef theService) {
	
	
	CFTypeRef result = CFNetServiceGetInfo(theService, _kCFNetServiceTXT);
	
	/*
	 ** This shouldn't really happen.  This is here in order to protect
	 ** against using the new TXT calls in conjunction with the old
	 ** deprecated calls.
	 */
	if (result && (CFGetTypeID(result) != CFStringGetTypeID()))
		result = NULL;
	
	return (CFStringRef)result;
}


/* extern */ void
CFNetServiceSetProtocolSpecificInformation(CFNetServiceRef theService, CFStringRef theInfo) {
	
	__CFNetService* service = (__CFNetService*)theService;
	
	__CFSpinLock(&(service->_lock));
	
	/* If not registered on the network, simply save the value */
	if (!service->_old_service)
		CFDictionarySetValue(service->_info, (const void*)_kCFNetServiceTXT, theInfo);
	
	else {
		char str[1024];
		CFIndex bytesUsed = 0;
		
		/* NOTE that the behavior here is legacy and should not change. */
		if (theInfo) {
			CFStringGetBytes(theInfo,
							 CFRangeMake(0, CFStringGetLength(theInfo)),
							 kCFStringEncodingUTF8,
							 0,
							 FALSE,
							 (UInt8*)str,
							 sizeof(str) - 1,
							 &bytesUsed);
		}
		str[bytesUsed] = '\0';
		
		/* Send it to the wire */
		DNSServiceRegistrationUpdateRecord_Deprecated(service->_old_service, 0, bytesUsed + 1, str, 0);
	}
	
	__CFSpinUnlock(&(service->_lock));
}


/* extern */ Boolean
CFNetServiceRegister(CFNetServiceRef theService, CFStreamError* error) {
	
	__CFNetService* service = (__CFNetService*)theService;
	
	CFStreamError extra;
	Boolean result = FALSE;
	
	if (!error)
		error = &extra;
	
	memset(error, 0, sizeof(error[0]));
	
	/*
	** Retain so it doesn't go away underneath in the case of a callout.  This is really
	** no worry for async, but makes the memmove for the error more difficult to place
	** for synchronous without it being here.
	*/
	CFRetain(theService);
	
	/* Lock down the service to start */
	__CFSpinLock(&(service->_lock));
	
	do {
		
		int i;
		char properties[4][1024];
		CFMachPortRef prt = NULL;
		CFAllocatorRef alloc = CFGetAllocator(theService);
		CFMachPortContext ctxt = {0, service, CFRetain, CFRelease, NULL};
		UInt32 keys[] = {_kCFNetServiceName, _kCFNetServiceType, _kCFNetServiceDomain, _kCFNetServiceTXT};
		
		/* Check to see if there is an ongoing process already */
		if (CFArrayGetCount(service->_sources)) {
		
			/* If there's already mdns activity, don't allow another. */
			if (!__CFBitIsSet(service->_flags, kFlagBitCancel)) {
				service->_error.error = kCFNetServicesErrorInProgress;
				service->_error.domain = kCFStreamErrorDomainNetServices;
			}
			
			/* It's just the cancel that hasn't fired yet, so cancel it. */
			else {
				
				/* Remove the cancel from run loops and modes */
				_UnscheduleSources(service->_sources, service->_schedules);
				
				/* Invalidate the run loop source */
				_InvalidateSources(service->_sources);
			}
		}
		
		/* Get the raw data for the properties to send down to mdns */
		for (i = 0; i < (sizeof(keys) / sizeof(keys[0])); i++) {
			
			CFStringRef value = (CFStringRef)CFDictionaryGetValue(service->_info, (const void*)(keys[i]));
			if (!value)
				properties[i][0] = '\0';
			else {
				CFIndex used;
				CFStringGetBytes(value,
								 CFRangeMake(0, CFStringGetLength(value)),
								 kCFStringEncodingUTF8,
								 0,
								 FALSE,
								 (UInt8*)properties[i],
								 sizeof(properties[i]) - 1,
								 &used);
				properties[i][used] = '\0';
			}
		}
		
		/* Create the registration */
		service->_old_service = DNSServiceRegistrationCreate_Deprecated(properties[0],
																		 properties[1],
																		 properties[2],
																		 htons((service->_port & 0x0000FFFF)),
																		 properties[3],
																		 _LegacyRegistrationReply,
																		 service);
														 
		if (!service->_old_service) {
		
			/* Set the error to errno if there is one. */
			service->_error.error = errno;
			if (service->_error.error)
				service->_error.domain = kCFStreamErrorDomainPOSIX;
				
			/* Some unknown error occurred. */
			else {
				service->_error.error = kCFNetServicesErrorUnknown;
				service->_error.domain = kCFStreamErrorDomainNetServices;
			}
			
			break;
		}
		
		/* Create a CFMachPort wrapper on the register */
		prt = CFMachPortCreateWithPort(alloc,
									   DNSServiceDiscoveryMachPort_Deprecated(service->_old_service),
									   _MachPortCallBack,
									   &ctxt,
									   NULL);
													 
		/* Need to bail if it failed */
		if (!prt) {
			
			/* Set error to whatever happened. */
			service->_error.error = errno;
			if (!service->_error.error)
				service->_error.error = ENOMEM;
				
			service->_error.domain = kCFStreamErrorDomainPOSIX;
		
			/* Stop right away on failure */
			DNSServiceDiscoveryDeallocate_Deprecated(service->_old_service);
			
			break;
		}
		
		/* Add the mach port to the list of sources */
		CFArrayAppendValue(service->_sources, prt);
		CFRelease(prt);
		
		/* Start with no error. */
		service->_error.error = 0;
		service->_error.domain = 0;
		
		/* Set the flags indicating a legacy registration */
		__CFBitSet(service->_flags, kFlagBitLegacyService);
		__CFBitSet(service->_flags, kFlagBitActiveRegister);
		
		/* Async mode is complete at this point */
		if (CFArrayGetCount(service->_schedules)) {
			
			/* Schedule the sources on the run loops and modes. */
			_ScheduleSources(service->_sources, service->_schedules);
			
			/* It's now succeeded. */
			result = TRUE;
		}
		
		/* Go into synchronous mode. */
		else {
			
			/* Unlock the service */
			__CFSpinUnlock(&(service->_lock));
			
			/* Wait for synchronous return */
			result = _ServiceBlockUntilComplete(service);
			
			/* Lock down the service */
			__CFSpinLock(&(service->_lock));
		}
		
	} while (0);
	
	/* Copy the error. */
	memmove(error, &service->_error, sizeof(error[0]));
	
	/* Unlock the service */
	__CFSpinUnlock(&(service->_lock));
	
	/* Release the earlier retain. */
	CFRelease(theService);
	
	return result;
}


/* extern */ Boolean
CFNetServiceResolve(CFNetServiceRef theService, CFStreamError* error) {
	
	__CFNetService* service = (__CFNetService*)theService;
	
	CFStreamError extra;
	Boolean result = FALSE;
	
	if (!error)
		error = &extra;
	
	memset(error, 0, sizeof(error[0]));
	
	/*
	** Retain so it doesn't go away underneath in the case of a callout.  This is really
	** no worry for async, but makes the memmove for the error more difficult to place
	** for synchronous without it being here.
	*/
	CFRetain(theService);
	
	/* Lock down the service to start */
	__CFSpinLock(&(service->_lock));
	
	do {
		
		int i;
		char properties[3][1024];
		CFMachPortRef prt;
		CFAllocatorRef alloc = CFGetAllocator(theService);
		CFMachPortContext ctxt = {0, service, CFRetain, CFRelease, NULL};
		UInt32 keys[] = {_kCFNetServiceName, _kCFNetServiceType, _kCFNetServiceDomain};
		
		/* Check to see if there is an ongoing process already */
		if (CFArrayGetCount(service->_sources)) {
		
			/* If there's already mdns activity, don't allow another. */
			if (!__CFBitIsSet(service->_flags, kFlagBitCancel)) {
				service->_error.error = kCFNetServicesErrorInProgress;
				service->_error.domain = kCFStreamErrorDomainNetServices;
			}
			
			/* It's just the cancel that hasn't fired yet, so cancel it. */
			else {
				
				/* Remove the cancel from run loops and modes */
				_UnscheduleSources(service->_sources, service->_schedules);
				
				/* Invalidate the run loop source */
				_InvalidateSources(service->_sources);
			}
		}
		
		/* Get the raw data for the properties to send down to mdns */
		for (i = 0; i < (sizeof(keys) / sizeof(keys[0])); i++) {
			
			CFStringRef value = (CFStringRef)CFDictionaryGetValue(service->_info, (const void*)(keys[i]));
			if (!value)
				properties[i][0] = '\0';
			else {
				CFIndex used;
				CFStringGetBytes(value,
								 CFRangeMake(0, CFStringGetLength(value)),
								 kCFStringEncodingUTF8,
								 0,
								 FALSE,
								 (UInt8*)properties[i],
								 sizeof(properties[i]) - 1,
								 &used);
				properties[i][used] = '\0';
			}
		}
		
		/* Create the resolve */
		service->_old_service = DNSServiceResolverResolve_Deprecated(properties[0],
																	 properties[1],
																	 properties[2],
																	 _LegacyResolverReply,
																	 service);
														 
		if (!service->_old_service) {
		
			/* Set the error to errno if there is one. */
			service->_error.error = errno;
			if (service->_error.error)
				service->_error.domain = kCFStreamErrorDomainPOSIX;
				
			/* Some unknown error occurred. */
			else {
				service->_error.error = kCFNetServicesErrorUnknown;
				service->_error.domain = kCFStreamErrorDomainNetServices;
			}
			
			break;
		}
		
		/* Create a CFMachPort wrapper on the register */
		prt = CFMachPortCreateWithPort(alloc,
									   DNSServiceDiscoveryMachPort_Deprecated(service->_old_service),
									   _MachPortCallBack,
									   &ctxt,
									   NULL);
													 
		/* Need to bail if it failed */
		if (!prt) {
			
			/* Set error to whatever happened. */
			service->_error.error = errno;
			if (!service->_error.error)
				service->_error.error = ENOMEM;
				
			service->_error.domain = kCFStreamErrorDomainPOSIX;
		
			/* Stop right away on failure */
			DNSServiceDiscoveryDeallocate_Deprecated(service->_old_service);
			
			break;
		}
		
		/* Remove any addresses so the resolve fills in new. */
		CFDictionaryRemoveValue(service->_info, (const void*)_kCFNetServiceAddress);
		
		/* Add the mach port to the list of sources */
		CFArrayAppendValue(service->_sources, prt);
		CFRelease(prt);
		
		/* Start with no error. */
		service->_error.error = 0;
		service->_error.domain = 0;
		
		/* Set the flags indicating a legacy resolve */
		__CFBitSet(service->_flags, kFlagBitLegacyService);
		__CFBitSet(service->_flags, kFlagBitActiveResolve);
		
		/* Async mode is complete at this point */
		if (CFArrayGetCount(service->_schedules)) {
			
			/* Schedule the sources on the run loops and modes. */
			_ScheduleSources(service->_sources, service->_schedules);
			
			/* It's now succeeded. */
			result = TRUE;
		}
		
		/* Go into synchronous mode. */
		else {
			
			/* Unlock the service */
			__CFSpinUnlock(&(service->_lock));
			
			/* Wait for synchronous return */
			result = _ServiceBlockUntilComplete(service);
			
			/* Lock down the service */
			__CFSpinLock(&(service->_lock));
		}
		
	} while (0);
	
	/* Copy the error. */
	memmove(error, &service->_error, sizeof(error[0]));
	
	/* Unlock the service */
	__CFSpinUnlock(&(service->_lock));
	
	/* Release the earlier retain. */
	CFRelease(theService);
	
	return result;
}
