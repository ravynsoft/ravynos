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

#include <dns_sd.h>  // DNSServiceDiscovery
#include <unistd.h>  // usleep
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <CoreServices/CoreServices.h>
#include <dispatch/dispatch.h>
#include <uuid/uuid.h>
#include "CLog.h"
#include "WorkstationService.h"

#define kWorkstationType   "_workstation._tcp."
#define kWorkstationPort   9
#define kMACAddressLengh   (sizeof(" [00:00:00:00:00:00]") - 1)
#define kMaxComputerName   63-kMACAddressLengh


typedef struct MyDNSServiceState {
    DNSServiceRef service;
    CFSocketRef socket;
} MyDNSServiceState;

DNSServiceErrorType RegisterWorkstationService(MyDNSServiceState *ref, CFStringRef serviceName);
CFStringRef CopyNextWorkstationName(SCDynamicStoreRef store, CFStringRef currentName);


CFStringRef gCurrentWorkstationName = NULL;
MyDNSServiceState *gServiceRegistration = NULL;
CFRunLoopSourceRef gNameMonitorSource = NULL;

static MyDNSServiceState *
MyDNSServiceAlloc(void)
{
    MyDNSServiceState *ref = (MyDNSServiceState *) calloc( 1, sizeof(MyDNSServiceState) );
    assert(ref);
    return ref;
}


static void
MyDNSServiceFree(MyDNSServiceState *ref)
{
    assert(ref);
    
    if (ref->socket) {
        //Invalidate the CFSocket.
        CFSocketInvalidate(ref->socket);
        CFRelease(ref->socket);

        // Workaround that gives time to CFSocket's select thread so it can remove the socket from its
        // FD set before we close the socket by calling DNSServiceRefDeallocate. <rdar://problem/3585273>
        usleep(1000);
    }
    
    if (ref->service) {
        // Terminate the connection with the mDNSResponder daemon, which cancels the operation.
        DNSServiceRefDeallocate(ref->service);
    }
    
    free(ref);
}


static void
MyDNSServiceSocketCallBack(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info)
{
    #pragma unused(s, type, address, data)
    DNSServiceErrorType error;
    MyDNSServiceState *ref = (MyDNSServiceState *)info;  // context passed in to CFSocketCreateWithNative().
    assert(ref);
    
    // Read a reply from the mDNSResponder, which will call the appropriate callback.
    error = DNSServiceProcessResult(ref->service);
    if (kDNSServiceErr_NoError != error) {
        DbgLog( kLogError, "DNSServiceProcessResult returned %d", error );
        
        // Terminate the discovery operation and release everything.
        MyDNSServiceFree(ref);
    }
}


static void
MyDNSServiceAddToRunLoop(MyDNSServiceState *ref)
{
    CFSocketNativeHandle    sock;
    CFOptionFlags           sockFlags;
    CFRunLoopSourceRef      source;
    CFSocketContext         context = { 0, ref, NULL, NULL, NULL };  // Use MyDNSServiceState as context data.

    assert(ref);

    // Access the underlying Unix domain socket to communicate with the mDNSResponder daemon.
    sock = DNSServiceRefSockFD(ref->service);
    assert(sock >= 0);

    // Create a CFSocket using the Unix domain socket.
    ref->socket = CFSocketCreateWithNative(NULL, sock, kCFSocketReadCallBack, MyDNSServiceSocketCallBack, &context);
    assert(ref->socket);

    // Prevent CFSocketInvalidate from closing DNSServiceRef's socket.
    sockFlags = CFSocketGetSocketFlags(ref->socket);
    CFSocketSetSocketFlags(ref->socket, sockFlags & (~kCFSocketCloseOnInvalidate));

    // Create a CFRunLoopSource from the CFSocket.
    source = CFSocketCreateRunLoopSource(NULL, ref->socket, 0);
    assert(source);

    // Add the CFRunLoopSource to the current runloop.
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
    
    // We no longer need our reference to source.  The runloop continues to 
    // hold a reference to it, but we don't care about that.  When we invalidate 
    // the socket, the runloop will drop its reference and the source will get 
    // destroyed.
    
    CFRelease(source);
}


static void
CFStringTruncateToUTF8Length(CFMutableStringRef str, ssize_t utf8LengthLimit)
    // Truncates a CFString such that it's UTF-8 representation will have 
    // utf8LengthLimit or less characters (not including the terminating 
    // null character).  Handles UTF-16 surrogates and trims whitespace 
    // from the end of the resulting string.
{
    CFIndex             shortLen;
    CFIndex             convertedLen;
    CFIndex             originalLen;
    CFIndex             utf8Length;
    CFCharacterSetRef   whiteCharSet;
    UniChar             thisChar;
    CFIndex             trailingCharsToDelete;
    
    // Keep converting successively smaller strings until the UTF-8 string is suitably 
    // short.  Note that utf8LengthLimit must be non-negative, so this loop will 
    // always terminate before we run off the front of the string.
    
    originalLen = CFStringGetLength(str);
    shortLen = originalLen;
    do {
        // Because the buffer parameter is NULL, CFStringGetBytes returns the size of 
        // buffer required for the range of characters.  This doesn't include the 
        // trailing null byte traditionally associated with UTF-8 C strings, which 
        // is cool because that's what our caller is expecting.
        
        convertedLen = CFStringGetBytes(str, CFRangeMake(0, shortLen), kCFStringEncodingUTF8, 0, false, NULL, 0, &utf8Length);
        assert( (convertedLen == shortLen) || (convertedLen == (shortLen - 1)) );
        shortLen = convertedLen;
        
        if (utf8Length <= utf8LengthLimit) {
            break;
        }
        shortLen -= 1;
    } while (true);
    
    whiteCharSet = CFCharacterSetGetPredefined(kCFCharacterSetWhitespaceAndNewline);
    assert(whiteCharSet != NULL);
    
    do {
        if ( shortLen == 0 ) {
            break;
        }
        thisChar = CFStringGetCharacterAtIndex(str, shortLen - 1);
        if ( ! CFCharacterSetIsCharacterMember(whiteCharSet, thisChar) ) {
            break;
        }
        shortLen -= 1;
    } while (true);    
    
    trailingCharsToDelete = originalLen - shortLen;
    CFStringDelete(str, CFRangeMake(originalLen - trailingCharsToDelete, trailingCharsToDelete));
}


static kern_return_t
FindEthernetInterfaces(io_iterator_t *matchingServices)
{
    kern_return_t    kernResult; 
    CFMutableDictionaryRef  matchingDict;
    CFMutableDictionaryRef  propertyMatchDict;
    
    // Ethernet interfaces are instances of class kIOEthernetInterfaceClass. 
    // IOServiceMatching is a convenience function to create a dictionary with the key kIOProviderClassKey and 
    // the specified value.
    matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);

    // Note that another option here would be:
    // matchingDict = IOBSDMatching("en0");
        
    if (NULL == matchingDict) {
        DbgLog( kLogError, "IOServiceMatching returned a NULL dictionary." );
    } else {
        // Each IONetworkInterface object has a Boolean property with the key kIOPrimaryInterface. Only the
        // primary (built-in) interface has this property set to TRUE.
        
        // IOServiceGetMatchingServices uses the default matching criteria defined by IOService. This considers
        // only the following properties plus any family-specific matching in this order of precedence 
        // (see IOService::passiveMatch):
        //
        // kIOProviderClassKey (IOServiceMatching)
        // kIONameMatchKey (IOServiceNameMatching)
        // kIOPropertyMatchKey
        // kIOPathMatchKey
        // kIOMatchedServiceCountKey
        // family-specific matching
        // kIOBSDNameKey (IOBSDNameMatching)
        // kIOLocationMatchKey
        
        // The IONetworkingFamily does not define any family-specific matching. This means that in            
        // order to have IOServiceGetMatchingServices consider the kIOPrimaryInterface property, we must
        // add that property to a separate dictionary and then add that to our matching dictionary
        // specifying kIOPropertyMatchKey.
            
        propertyMatchDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                            &kCFTypeDictionaryKeyCallBacks,
                            &kCFTypeDictionaryValueCallBacks);
    
        if (NULL == propertyMatchDict) {
            DbgLog( kLogError, "CFDictionaryCreateMutable returned a NULL dictionary." );
        } else {
            // Set the value in the dictionary of the property with the given key, or add the key 
            // to the dictionary if it doesn't exist. This call retains the value object passed in.
            CFDictionarySetValue(propertyMatchDict, CFSTR(kIOPrimaryInterface), kCFBooleanTrue); 
            
            // Now add the dictionary containing the matching value for kIOPrimaryInterface to our main
            // matching dictionary. This call will retain propertyMatchDict, so we can release our reference 
            // on propertyMatchDict after adding it to matchingDict.
            CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey), propertyMatchDict);
            CFRelease(propertyMatchDict);
        }
    }
    
    // IOServiceGetMatchingServices retains the returned iterator, so release the iterator when we're done with it.
    // IOServiceGetMatchingServices also consumes a reference on the matching dictionary so we don't need to release
    // the dictionary explicitly.
    kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, matchingServices);    
    if (KERN_SUCCESS != kernResult) {
        DbgLog( kLogError, "IOServiceGetMatchingServices returned 0x%08x", kernResult );
    }
        
    return kernResult;
}


static kern_return_t
GetMACAddress(io_iterator_t intfIterator, UInt8 *MACAddress, UInt8 bufferSize)
{
    io_object_t    intfService;
    io_object_t    controllerService;
    kern_return_t  kernResult = KERN_FAILURE;
    
    // Make sure the caller provided enough buffer space. Protect against buffer overflow problems.
	if (bufferSize < kIOEthernetAddressSize) {
		return kernResult;
	}
  
	// Initialize the returned address
    bzero(MACAddress, bufferSize);
    
    // IOIteratorNext retains the returned object, so release it when we're done with it.
    while ((intfService = IOIteratorNext(intfIterator))) {
        CFTypeRef  MACAddressAsCFData;        

        // IONetworkControllers can't be found directly by the IOServiceGetMatchingServices call, 
        // since they are hardware nubs and do not participate in driver matching. In other words,
        // registerService() is never called on them. So we've found the IONetworkInterface and will 
        // get its parent controller by asking for it specifically.
        
        // IORegistryEntryGetParentEntry retains the returned object, so release it when we're done with it.
        kernResult = IORegistryEntryGetParentEntry(intfService, kIOServicePlane, &controllerService);
    
        if (KERN_SUCCESS != kernResult) {
            DbgLog( kLogError, "IORegistryEntryGetParentEntry returned 0x%08x", kernResult );
        } else {
            // Retrieve the MAC address property from the I/O Registry in the form of a CFData
            MACAddressAsCFData = IORegistryEntryCreateCFProperty(controllerService, CFSTR(kIOMACAddress), kCFAllocatorDefault, 0);
            if (MACAddressAsCFData) {
                //CFShow(MACAddressAsCFData); // for display purposes only; output goes to stderr
                
                // Get the raw bytes of the MAC address from the CFData
                CFDataGetBytes((CFDataRef)MACAddressAsCFData, CFRangeMake(0, kIOEthernetAddressSize), MACAddress);
                CFRelease(MACAddressAsCFData);
            }
                
            // Done with the parent Ethernet controller object so we release it.
            (void)IOObjectRelease(controllerService);
        }
        
        // Done with the Ethernet interface object so we release it.
        (void)IOObjectRelease(intfService);
    }
        
    return kernResult;
}


static CFStringRef
CopyPrimaryMacAddress(void)
{
	io_iterator_t intfIterator;
    UInt8 MAC[kIOEthernetAddressSize];
	CFStringRef macAddress = NULL;
 
    kern_return_t kernResult = FindEthernetInterfaces(&intfIterator);
    if (KERN_SUCCESS != kernResult) {
        DbgLog( kLogError, "FindEthernetInterfaces returned 0x%08x", kernResult );
    } else {
        kernResult = GetMACAddress(intfIterator, MAC, sizeof(MAC));
        
        if (KERN_SUCCESS != kernResult) {
            DbgLog( kLogError, "GetMACAddress returned 0x%08x", kernResult );
        } else {
			macAddress = CFStringCreateWithFormat(NULL, NULL, CFSTR("[%02x:%02x:%02x:%02x:%02x:%02x]"), MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
		}
    }
    
    (void)IOObjectRelease(intfIterator);

	return macAddress;
}


static void
RegisterWorkstationCallBack(DNSServiceRef service, DNSServiceFlags flags, DNSServiceErrorType errorCode,
    const char *name, const char *type, const char *domain, void *context)
{
    #pragma unused(flags, service, context)
       
    if (errorCode == kDNSServiceErr_NoError) {
		DbgLog( kLogInfo, "Registration callback for service %s.%s%s", name, kWorkstationType, domain );
	}
    else if (errorCode == kDNSServiceErr_NameConflict) {
        DbgLog( kLogError, "Workstation name conflict occured so something weird is happening" );

		if (gServiceRegistration) {
			MyDNSServiceFree(gServiceRegistration);
			gServiceRegistration = NULL;
		}
	
		CFStringRef newWorkstationName = CopyNextWorkstationName(NULL, gCurrentWorkstationName);
		
		if (gCurrentWorkstationName) CFRelease(gCurrentWorkstationName);
		gCurrentWorkstationName = newWorkstationName;

		if (gCurrentWorkstationName) {
			gServiceRegistration = MyDNSServiceAlloc();
			RegisterWorkstationService(gServiceRegistration, gCurrentWorkstationName);
		}
    } else {
        DbgLog( kLogError, "Workstation name registration failed with error (%d)", errorCode );

		if (gServiceRegistration) {
			MyDNSServiceFree(gServiceRegistration);
			gServiceRegistration = NULL;
		}
	}
}


DNSServiceErrorType
RegisterWorkstationService(MyDNSServiceState *ref, CFStringRef serviceName)
{
	char name[64];
	DNSServiceErrorType error = kDNSServiceErr_BadParam;
	
	if (CFStringGetCString(serviceName, name, sizeof(name), kCFStringEncodingUTF8)) {

		uuid_t compUUID;
		CFDataRef cfTXTRecord = NULL;
		struct timespec waitTime = { 0 };
		
		if ( gethostuuid(compUUID, &waitTime) == 0 )
		{
			CFMutableDictionaryRef txtRecordDict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
																			  &kCFTypeDictionaryValueCallBacks );
			uuid_string_t uuidstr;
			
			uuid_unparse_upper( compUUID, uuidstr );
			CFStringRef cfUUID = CFStringCreateWithCString( kCFAllocatorDefault, uuidstr, kCFStringEncodingUTF8 );
			CFDictionarySetValue( txtRecordDict, CFSTR("uuid"), cfUUID );
			DbgLog( kLogInfo, "Workstation service - uuid = %s", uuidstr );
			DSCFRelease( cfUUID );
			
			cfTXTRecord = CFNetServiceCreateTXTDataWithDictionary( kCFAllocatorDefault, txtRecordDict );
			
			DSCFRelease( txtRecordDict );
		}
		else
		{
			DbgLog( kLogError, "Failed to get host UUID from gethostuuid()" );
		}
		
		error = DNSServiceRegister(&ref->service,
								   kDNSServiceFlagsNoAutoRename,
								   kDNSServiceInterfaceIndexAny,
								   name,
								   kWorkstationType,
								   NULL,
								   NULL,
								   htons(kWorkstationPort),
								   (cfTXTRecord ? CFDataGetLength(cfTXTRecord) : 0),
								   (cfTXTRecord ? CFDataGetBytePtr(cfTXTRecord) : NULL),
								   RegisterWorkstationCallBack,
								   (void *)ref);
		
		DSCFRelease( cfTXTRecord );
	}

    if (kDNSServiceErr_NoError == error) {
        DbgLog( kLogNotice, "Registered Workstation service - %s.%s", name, kWorkstationType );
        MyDNSServiceAddToRunLoop(ref);
	}
	
    return error;
}


CFStringRef
CopyNextWorkstationName(SCDynamicStoreRef store, CFStringRef currentName)
{
	CFMutableStringRef computerName = NULL;
	CFStringRef macAddress = NULL;
		
	if (currentName) {
	
		/* Sanity check to make sure the current Workstation name is longer than the length of a MAC address string */
		CFIndex totalLengh = CFStringGetLength(currentName);
		if (totalLengh >= (CFIndex)kMACAddressLengh) {
		
			/* Create a substring that chops off the MAC addres, giving us the Computer Name */
			CFRange range = CFRangeMake(0, totalLengh - kMACAddressLengh);
			CFStringRef oldComputerName = CFStringCreateWithSubstring(NULL, currentName, range);
			
			/* If the Computer Name contains a trailing close paren it means that this name may have
			already experienced a name conflict which means it could have a trailing digit to increment */
			if (CFStringHasSuffix(oldComputerName, CFSTR(")"))) {
				CFRange result;
				
				/* Search for the first open paren, starting the search from the end of the Computer Name */
				if (CFStringFindWithOptions(oldComputerName, CFSTR("("), range, kCFCompareBackwards, &result)) {
				
					/* Create a substring which contains the contents between the open and close paren */
					range = CFRangeMake(result.location + 1, CFStringGetLength(oldComputerName) - result.location - 2);
					CFStringRef countString = CFStringCreateWithSubstring(NULL, currentName, range);
					
					/* Try converting the substring to an integer */
					SInt32 conflictCount = CFStringGetIntValue(countString);
					if (conflictCount) {
					
						/* Create a substring of just the Computer Name without the trailing open paren, conflict integer, close paren */
						range = CFRangeMake(0, result.location);
						CFStringRef tempComputerName = CFStringCreateWithSubstring(NULL, oldComputerName, range);
						
						/* Create a mutable copy of the Computer Name base substring */
						computerName = CFStringCreateMutableCopy(NULL, 0, tempComputerName);
						
						/* Create a string containing a space, open paren, previous conflict digit incremented by one, close paren */
						CFStringRef numberString = CFStringCreateWithFormat(NULL, NULL, CFSTR(" (%d)"), ++conflictCount);

						/* Truncate the Computer Name base as neccessary to ensure we can append the conflict digits */
						CFStringTruncateToUTF8Length(computerName, kMaxComputerName - CFStringGetLength(numberString));
						
						/* Append the incremented conflict digit to the Computer Name base string */
						CFStringAppend(computerName, numberString);
					}
				}
			}
			
			/* If computerName is NULL it means that the previous Computer Name didn't contain any conflict digits so append a " (2)" */
			if (!computerName) {
			
				/* Create mutable copy of previous Computer Name */
				computerName = CFStringCreateMutableCopy(NULL, 0, oldComputerName);
				
				/* Make sure we have enough room to append 4 characters to the name by truncating the Computer Name if neccessary */
				CFStringTruncateToUTF8Length(computerName, kMaxComputerName - 4);
				
				/* Append the name conflict digits */
				CFStringAppend(computerName, CFSTR(" (2)"));
			}
			
			CFRelease(oldComputerName);
		} else {
			DbgLog( kLogError, "Workstation name is shorter than a MAC address which shouldn't be possible" );
		}
	
	} else {
		
		/* There's currently no registered Workstation name so get the Computer Name from the dynamic store */
		CFStringRef tempName = SCDynamicStoreCopyComputerName(store, NULL);
		if (tempName) {
			/* Create a mutable copy of the Computer Name */
			computerName = CFStringCreateMutableCopy(NULL, 0, tempName);
			CFRelease(tempName);
			
			/* Truncate the Computer Name to ensure we can append the MAC address */
			CFStringTruncateToUTF8Length(computerName, kMaxComputerName);
		} else {
			return NULL;
		}
	}
	
	/* Copy the primary MAC address string */
	macAddress = CopyPrimaryMacAddress();
	if (!macAddress) {
		if (computerName) {
			CFRelease(computerName);
		}
		return NULL;
	}
	
	/* Append a space */
	CFStringAppend(computerName, CFSTR(" "));
	
	/* Append the MAC address string */
	CFStringAppend(computerName, macAddress);
	CFRelease(macAddress);

	return computerName;
}


static void
ComputerNameChangedCallBack(SCDynamicStoreRef store, CFArrayRef changedKeys, void *info)
{
    #pragma unused(changedKeys, info)

	if (gServiceRegistration) {
		MyDNSServiceFree(gServiceRegistration);
		gServiceRegistration = NULL;
	}
	
	if (gCurrentWorkstationName) {
		CFRelease(gCurrentWorkstationName);
	}
	
	gCurrentWorkstationName = CopyNextWorkstationName(store, NULL);
	if (gCurrentWorkstationName) {
		gServiceRegistration = MyDNSServiceAlloc();
		RegisterWorkstationService(gServiceRegistration, gCurrentWorkstationName);
	}
}


static CFRunLoopSourceRef
InstallComputerNameMonitor(void)
{
	CFRunLoopSourceRef runLoopSource = NULL;
    SCDynamicStoreContext context = { 0, NULL, NULL, NULL, NULL };
    SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("DirectoryService"), ComputerNameChangedCallBack, &context);
    if (store) {
        CFStringRef computerNameKey = SCDynamicStoreKeyCreateComputerName(NULL);
		CFMutableArrayRef keys = CFArrayCreateMutable(NULL, 1, &kCFTypeArrayCallBacks);
		CFArrayAppendValue(keys, computerNameKey);
		
        if (SCDynamicStoreSetNotificationKeys(store, keys, NULL)) {
            runLoopSource = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
            if (runLoopSource) {
                CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
            }
        }
		CFRelease(computerNameKey);
		CFRelease(keys);
		CFRelease(store);
    }
	return runLoopSource;
}


static void
DisposeRegistration(void *context)
{
	if (gNameMonitorSource) {
		CFRunLoopSourceInvalidate(gNameMonitorSource);
		CFRelease(gNameMonitorSource);
		gNameMonitorSource = NULL;
	}

	if (gServiceRegistration) {
		MyDNSServiceFree(gServiceRegistration);
		gServiceRegistration = NULL;
	}
	
	if (gCurrentWorkstationName) {
		CFRelease(gCurrentWorkstationName);
		gCurrentWorkstationName = NULL;
	}
}


void
WorkstationServiceRegister( void )
{
	dispatch_async( dispatch_get_main_queue(), 
				    ^(void) {
						DNSServiceErrorType error = kDNSServiceErr_NoError;
						CFStringRef workstationName = NULL;
						
						workstationName = CopyNextWorkstationName(NULL, NULL);
						if ( workstationName == NULL )
							return;
						
						if ( gCurrentWorkstationName != NULL )
						{
							// if this name is registered, there's nothing to do
							if ( CFStringCompare(workstationName, gCurrentWorkstationName, 0) == kCFCompareEqualTo )
							{
								CFRelease( workstationName );
								return;
							}
							
							// there is a new name to register; remove the old registration
							DisposeRegistration( NULL );
						}
						
						// register
						gCurrentWorkstationName = workstationName;
						gNameMonitorSource = InstallComputerNameMonitor();
						gServiceRegistration = MyDNSServiceAlloc();
						error = RegisterWorkstationService(gServiceRegistration, gCurrentWorkstationName);
						if (error != kDNSServiceErr_NoError)
						{
							DbgLog( kLogError, "WorkstationServiceRegister(): received error from RegisterWorkstationService: %l", error );
							DisposeRegistration( NULL );
						}
					} );

}

void
WorkstationServiceUnregister(void)
{
	dispatch_async_f( dispatch_get_main_queue(), 
					  NULL,
					  DisposeRegistration );
}

