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
 *  NetworkDiagnostics.c
 *  NetworkDiagnostics
 *
 *  Created by Louis Gerbarg on Fri Mar 26 2004.
 *  Copyright 2004 Apple Computer, Inc. All rights reserved.
 *
 */

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

#include <CFNetwork/CFNetDiagnostics.h>
#include <CFNetwork/CFNetDiagnosticsPriv.h>

//For mig and mach stuff
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <servers/bootstrap_defs.h>
#include "CFNetDiagnosticsProtocol.h"
#include "CFNetDiagnosticsInternal.h"

//For IN_LINKLOCAL() and inet_addr()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//For mach_absolute_time()
#include <mach/mach_time.h>

//For printf
#include <stdio.h>

extern 
int _CFNetDiagnosticPing(CFStringRef HostToPing, const int NumberOfPacketsToSend, const int PingTimeoutInSeconds);

const char * CFNetDiagnosticNotifyKey = "com.apple.NetworkDiagnostics.notification";

/* 
	**INTERNAL SCHEMA INFO**
	details schema

	_NDNameKey			= <CFString for app that called us>  (example:  "Safari")
	_NDBundleKey			= <CFString for the app that called us>
	_NDRemoteHostKey		= <CFString for remote network host> (example:  "www.apple.com")
	_NDProtocolKey		= <CFString for network protocol>  (example:  "HTTP")
	_NDPortKey			= <CFNumber for network port>  (example:  80)

Usage:
	The details dictionary passed into NDDiagnoseProblem() must contain either a name
	or a BundleIdentifier. Everything else is optional. If the dictionary contains a
	Bundle identifier it will be used to determine the localized name of the caller,
	and potentially to get the icon. If the application wants to override that
	behaviour it may pass a name instead. In that cas the app is responsible for
	localizing the string before it passes it to us.
	
	The rest of the dictionary values can be used to help Network Diagnostics attempt
	to analyze the problem. 
	
*/

const CFStringRef CFNetDiagnosticProtocolHTTP = CFSTR("http");
const CFStringRef CFNetDiagnosticProtocolFTP = CFSTR("ftp");
const CFStringRef CFNetDiagnosticProtocolSMTP = CFSTR("smtp");
const CFStringRef CFNetDiagnosticProtocolIMAP = CFSTR("imap");
const CFStringRef CFNetDiagnosticProtocolOSCAR = CFSTR("oscar");
const CFStringRef CFNetDiagnosticProtocolUnknown = CFSTR("unknown");

#if 0

/*  This is used for debugging. I should also conditionalize it on an environment variable in case any 
	calls are accidentally left in the code.
*/

static
void _CFNetDiagnosticsPrintObject(CFTypeRef object) {
	char buffer[32768];
	Boolean converted;
	CFStringRef desc;
	
	desc = CFCopyDescription(object);
	if(desc) {
		converted = CFStringGetCString(desc, buffer, 32768, kCFStringEncodingASCII);

		if(converted) {
			printf("%s\n", buffer);
		}
		CFRelease(desc);
	}
}

#endif

/*  
	I am a big believer in static functions. Anything you repeated 3 times
	in a row should be refactored into them.
*/

static
void _CFNetDiagnosticSetDictionaryKeyIfNotNull(	CFStringRef key,
													CFTypeRef value,
													CFMutableDictionaryRef dict) {
	if(key != NULL && value != NULL) {
		CFDictionaryAddValue(dict, key, value);
	}
	
}

static
void _CFNetDiagnosticSetDictionaryKeyAndReleaseIfNotNull(	CFStringRef key,
															CFTypeRef value,
															CFMutableDictionaryRef dict) {
	if(key != NULL) {
		if(value != NULL) {
			CFDictionaryAddValue(dict, key, value);
			CFRelease(value);
		}
	}
}

static 
CFTypeRef _CFNetDiagnosticGetValueFromDictionaryAndRetain(CFDictionaryRef dict, CFStringRef key) {
	CFTypeRef s;

	s = (CFTypeRef)CFDictionaryGetValue(dict, key);
	if(s != NULL) {
		CFRetain(s);
	}
	
	return s;
}

/*  Okay, _CFNetDiagnosticsGetDataFromSCDSAndThowAwayGarbage is tricky. I use it to avoid doing this:
	
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(allocator, arg1, arg2, arg3);
	if(pattern) {
		dict = SCDynamicStoreCopyValue( store, pattern );
		CFRelease(pattern);
	}
	
	in a bunch of places in the code. Afterall, if I am going to generate an SCDS pattern from one of the
	provided functions the only thing I am possibly going to do with it is call SCDynamicStoreCopyValue.
*/

static
CFDictionaryRef _CFNetDiagnosticsGetDataFromSCDSAndThowAwayGarbage(CFAllocatorRef allocator, SCDynamicStoreRef store, 
	CFStringRef(*SCFunc)(CFAllocatorRef, CFStringRef, CFStringRef, CFStringRef), CFStringRef arg1, CFStringRef arg2, CFStringRef arg3) {
	CFStringRef pattern = NULL;
	CFDictionaryRef dict = NULL;

	
	
	pattern = SCFunc(allocator, arg1, arg2, arg3);
	if(pattern) {
		dict = SCDynamicStoreCopyValue( store, pattern );
		CFRelease(pattern);
	}
	
	return dict;
}


CFNetDiagnosticRef CFNetDiagnosticCreateBasic(	CFAllocatorRef allocator,
											CFStringRef remoteHost, 
											CFStringRef protocol, 
											CFNumberRef port) {
	CFMutableDictionaryRef retval = NULL;
	
	retval = CFDictionaryCreateMutable(allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	if(retval != NULL) {
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticNameKey, CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), kCFBundleNameKey), retval);		
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticBundleKey, CFBundleGetIdentifier( CFBundleGetMainBundle() ), retval);
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticRemoteHostKey, remoteHost, retval);
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticProtocolKey, protocol, retval);
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticPortKey, port, retval);
		
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticMethodKey, CFSTR("CFNetDiagnosticCreateBasic"), retval);
	}
	
	return (CFNetDiagnosticRef)retval;
}	
	
CFNetDiagnosticRef CFNetDiagnosticCreateWithURL(CFAllocatorRef allocator, CFURLRef url) {
	CFMutableDictionaryRef retval;
	SInt32 port = 0;
	
	retval = CFDictionaryCreateMutable(allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	if(retval != NULL && CFURLCanBeDecomposed(url)) {
		port = CFURLGetPortNumber(url);
		
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticNameKey, CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), kCFBundleNameKey), retval);		
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticBundleKey, CFBundleGetIdentifier( CFBundleGetMainBundle() ), retval);
		_CFNetDiagnosticSetDictionaryKeyAndReleaseIfNotNull(_CFNetDiagnosticRemoteHostKey, CFURLCopyHostName(url), retval);
		_CFNetDiagnosticSetDictionaryKeyAndReleaseIfNotNull(_CFNetDiagnosticProtocolKey, CFURLCopyScheme(url), retval);
		_CFNetDiagnosticSetDictionaryKeyAndReleaseIfNotNull(_CFNetDiagnosticPortKey, CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &port), retval);
		
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticMethodKey, CFSTR("CFNetDiagnosticCreateWithURL"), retval);
	}
	
	return (CFNetDiagnosticRef)retval;
}

CFNetDiagnosticRef CFNetDiagnosticCreateWithStreams(CFAllocatorRef allocator, CFReadStreamRef readStream, CFWriteStreamRef writeStream) {
	//FIXME deal with read and write streams
	CFMutableDictionaryRef retval;
#if 0
	CFArrayRef hostnames;
	CFHostRef host;
#endif	
	retval = CFDictionaryCreateMutable(allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	if(retval != NULL) {
#if 0
		host = (CFHostRef)CFReadStreamCopyProperty(readStream, kCFStreamPropertySocketRemoteHost);
		if(host) {
			hostnames = CFHostGetAddressing(host, NULL);
			_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticRemoteHostKey, CFArrayGetValueAtIndex(hostnames, 0), retval);
			CFRelease(host);
		}
#endif
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticNameKey, CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), kCFBundleNameKey), retval);		
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticBundleKey, CFBundleGetIdentifier( CFBundleGetMainBundle() ), retval);
	
		_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticMethodKey, CFSTR("CFNetDiagnosticCreateWithStreams"), retval);
	}
	
	return (CFNetDiagnosticRef)retval;
}

void CFNetDiagnosticSetName(CFNetDiagnosticRef details, CFStringRef name) {
	CFMutableDictionaryRef detailsDict = (CFMutableDictionaryRef)details;
	
	_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticNameKey, name, detailsDict);
} 

void CFNetDiagnosticSetProtocol(CFNetDiagnosticRef details, CFStringRef service) {
	CFMutableDictionaryRef detailsDict = (CFMutableDictionaryRef)details;
	
	_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticProtocolKey, service, detailsDict);
} 
	
void CFNetDiagnosticSetServiceID(CFNetDiagnosticRef details, CFStringRef service) {
	CFMutableDictionaryRef detailsDict = (CFMutableDictionaryRef)details;
	
	_CFNetDiagnosticSetDictionaryKeyIfNotNull(_CFNetDiagnosticServiceIDKey, service, detailsDict);
} 

CFNetDiagnosticStatus CFNetDiagnosticDiagnoseProblemInteractively(CFNetDiagnosticRef details) {
	SInt32 retval = 0;
	mach_port_t port = MACH_PORT_NULL;
	CFDataRef msgData = NULL;
	kern_return_t err;
	
	//build message
	CFWriteStreamRef stream = CFWriteStreamCreateWithAllocatedBuffers(kCFAllocatorDefault, kCFAllocatorDefault);
	CFWriteStreamOpen(stream);
	CFIndex len = CFPropertyListWriteToStream(details, stream, kCFPropertyListBinaryFormat_v1_0, NULL);
	CFWriteStreamClose(stream);
	if(len > 0) {
		msgData = CFWriteStreamCopyProperty(stream, kCFStreamPropertyDataWritten);
	}
	CFRelease(stream);
	

	if(msgData) {
		err = bootstrap_look_up(bootstrap_port, *((name_t*)(&_CFNetDiagnosticMachPortName)), &port);
		
		if (err == KERN_SUCCESS) {
			err = _CFNetDiagnosticClient_passDescriptor(	port,
															_CFNetDiagnosticMachProtocolVersion,
															(vm_address_t)CFDataGetBytePtr(msgData),
															CFDataGetLength(msgData));
			if (err == KERN_SUCCESS) {
				//FIXME Yay!!!
			}
			
		}
		
		CFRelease(msgData);
	}
	
	return (CFNetDiagnosticStatus)retval;
}

static 
CFStringRef copyCurrentRouter(void) {
	SCDynamicStoreRef store;
	CFPropertyListRef propList;
	CFStringRef retval = NULL;
	
	store = SCDynamicStoreCreate(kCFAllocatorDefault, CFSTR("Network Diagnostics"), NULL, NULL);

	if(store) {
		propList = SCDynamicStoreCopyValue(store, CFSTR("State:/Network/Global/IPv4"));
		if(propList) {
			retval = CFDictionaryGetValue(propList, CFSTR("Router"));
			if (retval) {
				CFRetain(retval);
			}
			CFRelease(propList);
		}
		CFRelease(store);
	}
	
	return retval;
}

static 
CFStringRef copyCurrentPrimaryService(void) {
	SCDynamicStoreRef store;
	CFPropertyListRef propList;
	CFStringRef retval = NULL;
	
	store = SCDynamicStoreCreate(kCFAllocatorDefault, CFSTR("Network Diagnostics"), NULL, NULL);

	if(store) {
		propList = SCDynamicStoreCopyValue(store, CFSTR("State:/Network/Global/IPv4"));
		if(propList) {
			retval = CFDictionaryGetValue(propList, CFSTR("PrimaryService"));
			if (retval) {
				CFRetain(retval);
			}
			CFRelease(propList);
		}
		CFRelease(store);
	}
	
	return retval;
}


static
CFArrayRef copyCurrentDNSServers(void) {
	SCDynamicStoreRef store;
	CFPropertyListRef propList;
	CFStringRef scdsRegexp;
	CFArrayRef retval = NULL;
	CFStringRef serviceID;
	
	serviceID = copyCurrentPrimaryService();
	if(serviceID) {

		scdsRegexp = SCDynamicStoreKeyCreateNetworkServiceEntity (
						kCFAllocatorDefault,
						kSCDynamicStoreDomainState,
						serviceID,
						kSCEntNetDNS);
		
		if(scdsRegexp) {

			store = SCDynamicStoreCreate(kCFAllocatorDefault, CFSTR("Network Diagnostics"), NULL, NULL);

			if(store) {
				propList = SCDynamicStoreCopyValue(store, scdsRegexp);
				if(propList) {
					retval = CFDictionaryGetValue(propList, CFSTR("ServerAddresses"));
					if (retval) {
						CFRetain(retval);
					}
					CFRelease(propList);
				}
				
				CFRelease(store);
			}

			CFRelease(scdsRegexp);
		}
		
		CFRelease(serviceID);
	}
	
	return retval;
}


CFNetDiagnosticStatus CFNetDiagnosticCopyNetworkStatusActively(CFNetDiagnosticRef details, CFNumberRef timeout, CFStringRef *description) {
	uint64_t timestamp;
	uint32_t timeout_value;
	uint32_t running_timeout;
	struct mach_timebase_info timebase;
	CFNetDiagnosticStatus retval = kCFNetDiagnosticConnectionDown;
	kern_return_t err;
	double conversion_factor;
	CFStringRef pingTarget;
	CFArrayRef nameServers;
	CFIndex nameServerCount;
	CFIndex i;
	bool nameServerResponded;

	//Get a timestamp	
	timestamp = mach_absolute_time();
	
	err = mach_timebase_info(&timebase);

	if(err == KERN_SUCCESS) {
		conversion_factor = 1e-9 * (double)(timebase.numer) / (double)(timebase.denom);
		retval = CFNetDiagnosticCopyNetworkStatusPassively(details, description);
	
		if (retval != kCFNetDiagnosticConnectionUp) {
			if (CFNumberGetValue(timeout, kCFNumberIntType, &timeout_value)) {
			
				//Get current time remaining
				running_timeout = timeout_value - conversion_factor * (mach_absolute_time() - timestamp);
			
				pingTarget = copyCurrentRouter();
				if (pingTarget) {
					if(_CFNetDiagnosticPing(pingTarget, 1, running_timeout)) {
						CFRelease(pingTarget);
						retval = kCFNetDiagnosticConnectionDown;
						if (description) {
							*description = CFCopyLocalizedStringFromTableInBundle( CFSTR("ROUTER_DOWN"),
																		NULL,
																		CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CFNetwork")),
																		"This computer's router is not responding.");
						}
					} else {
						CFRelease(pingTarget);
						
						nameServers = copyCurrentDNSServers();
						
						if (nameServers) {
							nameServerCount = CFArrayGetCount(nameServers);
							
							//Get current time remaining
							running_timeout = ((timeout_value - conversion_factor * (mach_absolute_time() - timestamp)) / nameServerCount);
			
							//ping a nameserver
							nameServerResponded = false;
							for (i=0; i < nameServerCount; i++) {
								pingTarget = CFArrayGetValueAtIndex(nameServers, i);
							
								if (!nameServerResponded) {
									if(!_CFNetDiagnosticPing(pingTarget, 1, running_timeout)) {
										nameServerResponded = true;
									}
								}
							}
							CFRelease(nameServers);
							
							if (!nameServerResponded) {
								retval = kCFNetDiagnosticConnectionDown;
								if (description) {
									*description = CFCopyLocalizedStringFromTableInBundle( CFSTR("NAMESERVER_DOWN"),
																				NULL,
																				CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CFNetwork")),
																				"This computer's DNS server is not responding.");
									}
							} else {
								//Get current time remaining
								running_timeout = timeout_value - conversion_factor * (mach_absolute_time() - timestamp);
			
								//Server router
								pingTarget = CFDictionaryGetValue((CFDictionaryRef)details, _CFNetDiagnosticRemoteHostKey);
								if(pingTarget) {
									if (_CFNetDiagnosticPing(pingTarget, 1, running_timeout)) {
										retval = kCFNetDiagnosticConnectionDown;
										if (description) {
											*description = CFCopyLocalizedStringFromTableInBundle( CFSTR("SERVER_DOWN"),
																						NULL,
																						CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CFNetwork")),
																						"The server this computer is attempting to connect to is not responding.");
										}
									} else {
										retval = kCFNetDiagnosticConnectionUp;
										if (description) {
											*description = CFCopyLocalizedStringFromTableInBundle( CFSTR("SERVER_UP"),
																						NULL,
																						CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CFNetwork")),
																						"This computer's Internet connection appears ot be online.");
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	//FIXME
	return retval;
}

static
Boolean _CFNetDiagnosticIsLinkLocal (CFStringRef s)
{
	char buffer[16];
	Boolean converted;
	Boolean retval = 0;
	uint32_t addr;

	converted = CFStringGetCString(s, buffer, 16, kCFStringEncodingASCII);
	if(converted) {
		addr = inet_addr(buffer);
		if(addr == INADDR_NONE) {
			retval = 0;
		} else {
			retval = IN_LINKLOCAL(addr);
		} 
	}

	 return retval;
}

//_CFNetDiagnosticCopyNetworkStatusPassivelyInterfaceSpecific is where the meat of CFNetDiagnosticCopyNetworkStatusPassively is implemented

static
CFNetDiagnosticStatus _CFNetDiagnosticCopyNetworkStatusPassivelyInterfaceSpecific(SCDynamicStoreRef store, CFStringRef serviceID, CFStringRef *description) {
	CFDictionaryRef dict = NULL;
	CFNetDiagnosticStatus retval;
//	CFStringRef name;
	CFStringRef device = NULL;
//	CFStringRef hardware;
//	Boolean isDialup = 0;
	CFArrayRef addresses = NULL;
	CFTypeRef address = NULL;
	Boolean isLinkActive = 0;
	Boolean isConnected = 0;
	Boolean isLinkLocal = 0;
	

	//First we get basic Information about the device
	dict = _CFNetDiagnosticsGetDataFromSCDSAndThowAwayGarbage(NULL, store, SCDynamicStoreKeyCreateNetworkServiceEntity, 
		kSCDynamicStoreDomainSetup, serviceID, kSCEntNetInterface);

	if(dict) {
//		name = _CFNetDiagnosticGetValueFromDictionaryAndRetain(dict, kSCPropUserDefinedName);
		device = (CFStringRef)_CFNetDiagnosticGetValueFromDictionaryAndRetain(dict, kSCPropNetInterfaceDeviceName);
//		hardware = _CFNetDiagnosticGetValueFromDictionaryAndRetain(dict, kSCPropNetInterfaceHardware);
//		isDialup = CFEqual(kSCValNetInterfaceTypePPP, CFDictionaryGetValue(dict, kSCPropNetInterfaceType));
	
		CFRelease(dict);
	}

	//Now we find out if the link is active
	if(device) {
		dict = _CFNetDiagnosticsGetDataFromSCDSAndThowAwayGarbage(NULL, store, SCDynamicStoreKeyCreateNetworkInterfaceEntity, 
			kSCDynamicStoreDomainState, device, kSCEntNetLink);
		CFRelease(device);
		
		if(dict) {
			isLinkActive = CFEqual(kCFBooleanTrue, CFDictionaryGetValue(dict, kSCPropNetLinkActive)) ? 1 : 0;

			CFRelease( (CFDictionaryRef) dict );
		}
	}
	
	//Now we find out if the link is connected
	dict = _CFNetDiagnosticsGetDataFromSCDSAndThowAwayGarbage(NULL, store, SCDynamicStoreKeyCreateNetworkServiceEntity, 
		kSCDynamicStoreDomainState, serviceID, kSCEntNetIPv4);

	if(dict) {
		isConnected = 1;
    
		addresses = CFDictionaryGetValue(dict, kSCPropNetIPv4Addresses);
		if(CFArrayGetCount(addresses) > 0) {
			address = CFArrayGetValueAtIndex(addresses, 0);
		}
    
		if(address) {
			isLinkLocal = _CFNetDiagnosticIsLinkLocal(address);
		}
		
		CFRelease( (CFDictionaryRef) dict );
	}
	

	
	if(isLinkActive && isConnected) {
		if(isLinkLocal) {
			retval = kCFNetDiagnosticConnectionIndeterminate;
		} else {
			retval = kCFNetDiagnosticConnectionUp;
		}
	} else {
		retval = kCFNetDiagnosticConnectionDown;
	}
	
	return retval;
}


CFNetDiagnosticStatus CFNetDiagnosticCopyNetworkStatusPassively(CFNetDiagnosticRef details, CFStringRef *description) {
	CFMutableDictionaryRef detailsDict = (CFMutableDictionaryRef)details;
	CFNetDiagnosticStatus retval = kCFNetDiagnosticConnectionIndeterminate;
	CFStringRef serviceID;
	SCDynamicStoreRef store;

	
	store = SCDynamicStoreCreate(kCFAllocatorDefault, CFSTR("CFNetDiagnostics"), NULL, NULL);
	
	if(store) {
	
		
		serviceID = CFDictionaryGetValue(detailsDict, _CFNetDiagnosticServiceIDKey);
		if(serviceID) {
			//If there is a specific ServiceID we only scan on it. We can only get in this position through SPIs
			retval = _CFNetDiagnosticCopyNetworkStatusPassivelyInterfaceSpecific(store, serviceID, description);
		} else {
			//Iterate through all serviceIDs. If any are good, then we return it
			CFStringRef pattern = NULL;
			CFDictionaryRef dict = NULL;
			CFArrayRef serviceOrder = NULL;
			CFIndex i, count;
			CFNetDiagnosticStatus serviceState = kCFNetDiagnosticConnectionDown;
			
			pattern = SCDynamicStoreKeyCreateNetworkGlobalEntity( NULL,
                                        (CFStringRef) kSCDynamicStoreDomainSetup,
                                        (CFStringRef) kSCEntNetIPv4 );
			
			if(pattern) {
				dict =  SCDynamicStoreCopyValue( store, pattern );
				CFRelease( pattern );
			}
			
			if(dict) {
				serviceOrder = CFDictionaryGetValue(dict, CFSTR("ServiceOrder"));
				CFRetain(serviceOrder);
				CFRelease(dict);
			}
			
			if(serviceOrder) {
				count = CFArrayGetCount(serviceOrder);
				retval = kCFNetDiagnosticConnectionDown;
				
				for ( i = 0; i < count; i++ ) {
					serviceID = CFArrayGetValueAtIndex(serviceOrder, i);
					serviceState = _CFNetDiagnosticCopyNetworkStatusPassivelyInterfaceSpecific(store, serviceID, description);
				
					if(serviceState == kCFNetDiagnosticConnectionDown) {
						retval = kCFNetDiagnosticConnectionDown;
						if (description) {
							*description = CFCopyLocalizedStringFromTableInBundle( CFSTR("CONNECTION_DOWN"),
																		NULL,
																		CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CFNetwork")),
																		"This computer's Internet connect appears to be offline.");
						}
					} else if (serviceState == kCFNetDiagnosticConnectionIndeterminate) {
						retval = kCFNetDiagnosticConnectionIndeterminate;
						if (description) {
							*description = CFCopyLocalizedStringFromTableInBundle( CFSTR("CONNECTION_INDETERMINATE"),
																		NULL,
																		CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CFNetwork")),
																		"This computer's Internet may be offline.");
						}
					} else if (serviceState == kCFNetDiagnosticConnectionUp) {
						retval = kCFNetDiagnosticConnectionUp;
						if (description) {
							*description = CFCopyLocalizedStringFromTableInBundle( CFSTR("CONNECTION_UP"),
																		NULL,
																		CFBundleGetBundleWithIdentifier(CFSTR("com.apple.CFNetwork")),
																		"This computer's Internet may be online.");
						}
						break;
					} else {
						//FIXME
						//NOT REACHED log an error
					}
				}
				CFRelease(serviceOrder);
			}
		}
	
		CFRelease(store);
	}
	
	return retval;
}
