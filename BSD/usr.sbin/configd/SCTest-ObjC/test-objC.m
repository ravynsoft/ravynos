/*
 * Copyright (c) 2015, 2018, 2020 Apple Inc. All rights reserved.
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
 * Modification History
 *
 * April 21, 2015		Sushant Chavan
 * - initial revision
 */

/*
 *  A Objective-C test target to test SC APIs
 */

#import <TargetConditionals.h>

#if	!defined(USING_PUBLIC_SDK)
@import Foundation;
@import SystemConfiguration;
@import SystemConfiguration_Private;
#else	// !defined(USING_PUBLIC_SDK)
#include <Foundation/Foundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#endif	// !defined(USING_PUBLIC_SDK)

#if	TARGET_OS_MACCATALYST
#pragma message "Building for IOS_MAC"
#endif

#define MY_APP_NAME	CFSTR("SCTestObjC")
#define TARGET_HOST	"www.apple.com"


#if	!TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)
static void
test_SCDynamicStore()
{
	NSLog(@"\n\n*** SCDynamicStore ***\n\n");
	CFDictionaryRef		dict;
	CFStringRef		intf;
	CFStringRef		key;
	SCDynamicStoreRef	store;

	store = SCDynamicStoreCreate(NULL, MY_APP_NAME, NULL, NULL);
	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetIPv4);
	dict = SCDynamicStoreCopyValue(store, key);
	intf = CFDictionaryGetValue(dict, kSCDynamicStorePropNetPrimaryInterface);
	NSLog(@"- Primary Interface is %@\n", intf);

	CFRelease(store);
	CFRelease(dict);
	CFRelease(key);
}
#endif	// !TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)

#if	!TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)
static void
test_SCNetworkConfiguration()
{
	NSLog(@"\n\n*** SCNetworkConfiguration ***\n\n");
	CFIndex			count;
	CFIndex			idx;
	CFArrayRef		interfaces;
	
	interfaces = SCNetworkInterfaceCopyAll();
	count = CFArrayGetCount(interfaces);
	NSLog(@"Network Interfaces:\n");
	for (idx=0; idx < count; idx++) {
		SCNetworkInterfaceRef intf;
		CFStringRef bsdName;
		
		intf = CFArrayGetValueAtIndex(interfaces, idx);
		bsdName = SCNetworkInterfaceGetBSDName(intf);
		NSLog(@"- %@", bsdName);
	}
	
	CFRelease(interfaces);
}
#endif	// !TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)

void
test_SCNetworkReachability()
{
	NSLog(@"\n\n*** SCNetworkReachability ***\n\n");
	SCNetworkReachabilityFlags	flags;
	SCNetworkReachabilityRef	target;

	target = SCNetworkReachabilityCreateWithName(NULL, TARGET_HOST);
	(void)SCNetworkReachabilityGetFlags(target, &flags);
	NSLog(@"- Reachability flags for "TARGET_HOST": %#x", flags);

	CFRelease(target);
}

#if	!TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)
static void
test_SCPreferences()
{
	NSLog(@"\n\n*** SCPreferences ***\n\n");
	CFIndex			count;
	CFIndex			idx;
	CFStringRef		model = NULL;
	SCPreferencesRef	prefs;
	CFArrayRef		services;
	
	prefs = SCPreferencesCreate(NULL, MY_APP_NAME, NULL);
	model = SCPreferencesGetValue(prefs, CFSTR("Model"));
	if (model != NULL) {
		NSLog(@"Current model is %@", model);
	}
	
	services = SCNetworkServiceCopyAll(prefs);
	count = CFArrayGetCount(services);
	NSLog(@"Network Services:\n");
	for (idx = 0; idx < count; idx++) {
		SCNetworkServiceRef serv;
		CFStringRef servName;
		
		serv = CFArrayGetValueAtIndex(services, idx);
		servName = SCNetworkServiceGetName(serv);
		NSLog(@"- %@\n", servName);
	}
	
	CFRelease(prefs);
	CFRelease(services);
}
#endif	// !TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)

void
SCTest()
{

#if	!TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)
	test_SCDynamicStore();
#endif	// !TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)

#if	!TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)
	test_SCNetworkConfiguration();
#endif	// !TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)

	test_SCNetworkReachability();

#if	!TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)
	test_SCPreferences();
#endif	// !TARGET_OS_SIMULATOR && !defined(USING_PUBLIC_SDK)

}

int
main(int argc, const char * argv[]) {
#pragma unused(argc, argv)

#if	TARGET_OS_MACCATALYST
#if	!defined(USING_PUBLIC_SDK)
#include <CoreFoundation/CFPriv.h>
#else	// !defined(USING_PUBLIC_SDK)
extern Boolean _CFMZEnabled(void);
#endif	// !defined(USING_PUBLIC_SDK)
	if (_CFMZEnabled()) {
		NSLog(@"*** IOS_MAC ***\n");
	}
#endif

	@autoreleasepool {
		SCTest();
	}
	return 0;
}
