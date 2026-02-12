/*
 * Copyright (c) 2020 Apple Inc. All rights reserved.
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

#import "SCTest.h"
#import "SCTestUtils.h"

#include <net/ethernet.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/network/IONetworkController.h>
#include <IOKit/network/IOUserEthernetController.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>
#include <IOKit/usb/USB.h>

#define	INTERFACES_KEY			@"State:/Network/Interface"
#define	PLUGIN_INTERFACE_NAMER_KEY	@"Plugin:InterfaceNamer"

#define	WAIT_TIME			(30 * NSEC_PER_SEC)


@interface SCTestInterfaceNamer : SCTest
@property NSArray		*interfaces_all;
@property NSArray		*interfaces_preconfigured;
@property dispatch_queue_t	queue;
@property dispatch_semaphore_t	sem_interfaces_all;
@property dispatch_semaphore_t	sem_interfaces_preconfigured;
@property SCDynamicStoreRef	store;
@end

@implementation SCTestInterfaceNamer

+ (NSString *)command
{
	return @"InterfaceNamer";
}

+ (NSString *)commandDescription
{
	return @"Tests the InterfaceNamer.bundle code paths";
}

static void
storeCallback(SCDynamicStoreRef store, CFArrayRef changedKeys, void *info)
{
	@autoreleasepool {
		NSDictionary		*dict;
		NSArray			*interfaces;
		SCTestInterfaceNamer	*test	= (__bridge SCTestInterfaceNamer *)info;

		if ([(__bridge NSArray *)changedKeys containsObject:INTERFACES_KEY]) {
			// copy list of interfaces
			dict = (__bridge_transfer NSDictionary *)SCDynamicStoreCopyValue(store, (CFStringRef)INTERFACES_KEY);
			interfaces = [dict objectForKey:(__bridge NSString *)kSCPropNetInterfaces];
			if (!_SC_CFEqual((__bridge CFArrayRef)interfaces, (__bridge CFArrayRef)test.interfaces_all)) {
				test.interfaces_all = interfaces;
				dispatch_semaphore_signal(test.sem_interfaces_all);
			}
		}

		if ([(__bridge NSArray *)changedKeys containsObject:PLUGIN_INTERFACE_NAMER_KEY]) {
			// copy list of [pre-configured] interfaces
			dict = (__bridge_transfer NSDictionary *)SCDynamicStoreCopyValue(store, (CFStringRef)PLUGIN_INTERFACE_NAMER_KEY);
			interfaces = [dict objectForKey:@"_PreConfigured_"];
			if (!_SC_CFEqual((__bridge CFArrayRef)interfaces, (__bridge CFArrayRef)test.interfaces_preconfigured)) {
				test.interfaces_preconfigured = interfaces;
				dispatch_semaphore_signal(test.sem_interfaces_preconfigured);
			}
		}
	}

	return;
}

- (instancetype)initWithOptions:(NSDictionary *)options
{
	self = [super initWithOptions:options];
//	if (self) {
//	}
	return self;
}

- (void)dealloc
{
	[self watchForChanges:FALSE];
}

- (void)start
{
	[self cleanupAndExitWithErrorCode:0];
}

- (void)cleanupAndExitWithErrorCode:(int)error
{
	[super cleanupAndExitWithErrorCode:error];
}

- (BOOL)setup
{
	return YES;
}

- (BOOL)unitTest
{
	if(![self setup]) {
		return NO;
	}

	BOOL allUnitTestsPassed = YES;
	allUnitTestsPassed &= [self unitTestInsertRemoveOneInterface];

	if(![self tearDown]) {
		return NO;
	}

	return allUnitTestsPassed;
}

- (BOOL)tearDown
{
	return YES;
}

- (void)watchForChanges:(BOOL)enable
{
	if (enable) {
		SCDynamicStoreContext	context	= {0, NULL, CFRetain, CFRelease, NULL};
		Boolean			ok;

		self.queue = dispatch_queue_create("SCTestInterfaceNamer callback queue", NULL);

		self.sem_interfaces_all = dispatch_semaphore_create(0);

		self.sem_interfaces_preconfigured = dispatch_semaphore_create(0);

		context.info = (__bridge void * _Nullable)self;
		self.store = SCDynamicStoreCreate(kCFAllocatorDefault, CFSTR("SCTest"), storeCallback, &context);

		ok = SCDynamicStoreSetNotificationKeys(self.store,
						       (__bridge CFArrayRef)@[INTERFACES_KEY,
									      PLUGIN_INTERFACE_NAMER_KEY],
						       NULL);
		assert(ok);

		ok = SCDynamicStoreSetDispatchQueue(self.store, self.queue);
		assert(ok);
	} else {
		if (self.store != NULL) {
			(void)SCDynamicStoreSetDispatchQueue(self.store, NULL);
			CFRelease(self.store);
			self.store = NULL;
		}

		self.queue = NULL;
		self.sem_interfaces_all = NULL;
		self.sem_interfaces_preconfigured = NULL;
	}
}

- (BOOL)isPreconfiguredInterface:(NSString *)bsdName
{
	return [self.interfaces_preconfigured containsObject:bsdName];;
}

#pragma mark -
#pragma mark IOEthernetController support

static CFStringRef
copy_interface_name(IOEthernetControllerRef controller)
{
	CFStringRef 	bsdName;
	io_object_t 	interface;

	interface = IOEthernetControllerGetIONetworkInterfaceObject(controller);
	if (interface == MACH_PORT_NULL) {
		SCTestLog("*** could not get interface for controller");
		return NULL;
	}

	bsdName = IORegistryEntryCreateCFProperty(interface, CFSTR(kIOBSDNameKey), NULL, kNilOptions);
	if (bsdName == NULL) {
		SCTestLog("*** IOEthernetController with no BSD interface name");
		return NULL;
	}

	return bsdName;
}

static CFDataRef
copy_interface_mac(IOEthernetControllerRef controller)
{
	CFDataRef 	macAddress;
	io_object_t 	interface;

	interface = IOEthernetControllerGetIONetworkInterfaceObject(controller);
	if (interface == MACH_PORT_NULL) {
		SCTestLog("*** could not get interface for controller");
		return NULL;
	}

	macAddress = IORegistryEntrySearchCFProperty(interface,
						     kIOServicePlane,
						     CFSTR(kIOMACAddress),
						     NULL,
						     kIORegistryIterateRecursively | kIORegistryIterateParents);
	if (macAddress == NULL) {
		SCTestLog("*** IOEthernetController with no BSD interface name");
		return NULL;
	}

	return macAddress;
}

static IOEthernetControllerRef
create_hidden_interface(u_char ea_unique)
{
	IOEthernetControllerRef	controller;
	CFDataRef 		data;
	struct ether_addr	ea	= { .octet = { 0x02, 'F', 'A', 'K', 'E', ea_unique } };
	CFMutableDictionaryRef 	merge;
	CFNumberRef		num;
	CFMutableDictionaryRef 	props;
	const int		usb_vid_apple	= kIOUSBAppleVendorID;

	props = CFDictionaryCreateMutable(NULL, 0,
					  &kCFTypeDictionaryKeyCallBacks,
					  &kCFTypeDictionaryValueCallBacks);

	data = CFDataCreate(NULL, ea.octet, ETHER_ADDR_LEN);
	CFDictionarySetValue(props, kIOEthernetHardwareAddress, data);
	CFRelease(data);

	merge = CFDictionaryCreateMutable(NULL, 0,
					  &kCFTypeDictionaryKeyCallBacks,
					  &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(merge, CFSTR(kIOPropertyProductNameKey),		CFSTR("Hidden Ethernet"));
	CFDictionarySetValue(merge, kIOUserEthernetInterfaceRole,		CFSTR("hidden-ethernet"));
	CFDictionarySetValue(merge, kSCNetworkInterfaceHiddenConfigurationKey,	kCFBooleanTrue);
	num = CFNumberCreate(NULL, kCFNumberIntType, &usb_vid_apple);
	CFDictionarySetValue(merge, CFSTR(kUSBVendorID),			num);
	CFRelease(num);
	CFDictionarySetValue(props, kIOUserEthernetInterfaceMergeProperties,	merge);
	CFRelease(merge);

	controller = IOEthernetControllerCreate(NULL, props);
	CFRelease(props);
	if (controller == NULL) {
		SCTestLog("*** could not create ethernet controller for \"%s\"", ether_ntoa(&ea));
		return NULL;
	}

	return controller;
}

- (BOOL)interfaceAdd:(u_char)ea_unique controller:(IOEthernetControllerRef *)newController
{
	BOOL			ok	= FALSE;

	do {
		NSString	*bsdName;
		NSData		*macAddress;
		long		status;

		// add an interface
		*newController = create_hidden_interface(ea_unique);
		if (*newController == NULL) {
			SCTestLog("*** could not create controller");
			break;
		}

		// wait for the [BSD] interface to show up
		status = dispatch_semaphore_wait(self.sem_interfaces_all, dispatch_time(DISPATCH_TIME_NOW, WAIT_TIME));
		if (status != 0) {
			// if timeout
			SCTestLog("*** no KernelEventMonitor change posted, interface not? created");
			break;
		}

		bsdName = (__bridge_transfer NSString *)copy_interface_name(*newController);
		if (bsdName == NULL) {
			break;
		}

		macAddress = (__bridge_transfer NSData *)copy_interface_mac(*newController);
		if (macAddress == NULL) {
			break;
		}

		SCTestLog("  Interface \"%@\" added, mac=%s",
			  bsdName,
			  ether_ntoa((struct ether_addr *)macAddress.bytes));

		// check if pre-configured
		status = dispatch_semaphore_wait(self.sem_interfaces_preconfigured, dispatch_time(DISPATCH_TIME_NOW, WAIT_TIME));
		if (status != 0) {
			// if timeout
			SCTestLog("*** no InterfaceNamer change posted, not? preconfigured");
			break;
		}

		if (![self isPreconfiguredInterface:bsdName]) {
			SCTestLog("*** Interface \"%@\" is not pre-configured", bsdName);
			break;
		}

		SCTestLog("  Interface \"%@\" is pre-configured", bsdName);

		ok = TRUE;
	} while (0);

	if (!ok) {
		if (*newController != NULL) {
			CFRelease(*newController);
			*newController = NULL;
		}
	}

	return ok;
}

- (BOOL)interfaceRemove:(IOEthernetControllerRef)controller
{
	BOOL	ok	= FALSE;

	do {
		NSString	*bsdName	= (__bridge_transfer NSString *)copy_interface_name(controller);
		long		status;

		// remove the interface
		CFRelease(controller);

		// wait for the [BSD] interface to go away
		status = dispatch_semaphore_wait(self.sem_interfaces_all, dispatch_time(DISPATCH_TIME_NOW, WAIT_TIME));
		if (status != 0) {
			// if timeout
			SCTestLog("*** no KernelEventMonitor change posted, interface not? removed");
			break;
		}

		SCTestLog("  Interface \"%@\" removed", bsdName);

		// check if [still] pre-configured
		status = dispatch_semaphore_wait(self.sem_interfaces_preconfigured, dispatch_time(DISPATCH_TIME_NOW, WAIT_TIME));
		if (status != 0) {
			// if timeout
			SCTestLog("*** no InterfaceNamer change posted, still? preconfigured");
			break;
		}

		if ([self isPreconfiguredInterface:bsdName]) {
			SCTestLog("*** interface \"%@\" is still pre-configured", bsdName);
			break;
		}

		ok = TRUE;
	} while (0);

	return ok;
}

#pragma mark -
#pragma mark unitTestInsertRemoveOneInterface

#define N_INTERFACES		5

- (BOOL)unitTestInsertRemoveOneInterface
{
	NSString		*bsdName1	= nil;
	BOOL			ok;
	SCTestInterfaceNamer	*test;

	test = [[SCTestInterfaceNamer alloc] initWithOptions:self.options];
	[test watchForChanges:TRUE];

	for (size_t i = 0; i < N_INTERFACES; i++) {
		NSString		*bsdName;
		IOEthernetControllerRef	controller;

		SCTestLog("Interface #%zd", i + 1);

		// add an interface
		ok = [test interfaceAdd:i controller:&controller];
		if (!ok) {
			break;
		}

		// check the assigned interface name
		bsdName = (__bridge_transfer NSString *)copy_interface_name(controller);
		if (i == 0) {
			bsdName1 = bsdName;
		} else if ([bsdName isNotEqualTo:bsdName1]) {
			SCTestLog("*** interface name not re-assigned, expected \"%@\", assigned \"%@\"", bsdName1, bsdName);
			break;
		}

		// remove the interface
		ok = [test interfaceRemove:controller];
		if (!ok) {
			break;
		}
	};

	if (ok) {
		SCTestLog("Successfully completed unitTestInsertRemoveOneInterface unit test");
	}

	return ok;
}

#pragma mark -
#pragma mark unitTestInsertRemoveMultipleInterfaces

#define N_INTERFACES_SLOT_X	(N_INTERFACES / 2)

- (BOOL)unitTestInsertRemoveMultipleInterfaces
{
	NSString			*bsdName;
	IOEthernetControllerRef		controller;
	struct {
		NSString		*bsdName;
		IOEthernetControllerRef	controller;
	} interfaces[N_INTERFACES]			= { { } };
	BOOL				ok;
	SCTestInterfaceNamer		*test;

	test = [[SCTestInterfaceNamer alloc] initWithOptions:self.options];
	[test watchForChanges:TRUE];

	SCTestLog("Adding %d interfaces", N_INTERFACES);

	for (size_t i = 0; i < N_INTERFACES; i++) {
		// add an interface
		ok = [test interfaceAdd:i controller:&controller];
		if (!ok) {
			break;
		}

		interfaces[i].controller = controller;
		interfaces[i].bsdName = (__bridge_transfer NSString *)copy_interface_name(controller);
	}

	if (ok) {
		SCTestLog("Removing interfaces");

		for (size_t i = 0; i < N_INTERFACES; i++) {
			// remove the interface
			ok = [test interfaceRemove:interfaces[i].controller];
			interfaces[i].controller = NULL;
			if (!ok) {
				break;
			}
		}
	}

	if (ok) {
		SCTestLog("Re-adding %d interfaces", N_INTERFACES);

		for (size_t i = 0; i < N_INTERFACES; i++) {
			// add an interface
			ok = [test interfaceAdd:i controller:&controller];
			if (!ok) {
				break;
			}

			interfaces[i].controller = controller;

			bsdName = (__bridge_transfer NSString *)copy_interface_name(controller);
			ok = [bsdName isEqualTo:interfaces[i].bsdName];
			if (!ok) {
				SCTestLog("*** interface %zd not assigned the same name, expected \"%@\", assigned \"%@\"",
					  i,
					  interfaces[i].bsdName,
					  bsdName);
				break;
			}

			interfaces[i].bsdName = bsdName;
		}
	}

	if (ok) {
		SCTestLog("Removing one interface");

		ok = [test interfaceRemove:interfaces[N_INTERFACES_SLOT_X].controller];
		interfaces[N_INTERFACES_SLOT_X].controller = NULL;
	}

	if (ok) {
		SCTestLog("Adding a new interface");

		do {
			// add a new interface (should re-use the first available name)
			ok = [test interfaceAdd:N_INTERFACES controller:&controller];
			if (!ok) {
				break;
			}

			interfaces[N_INTERFACES_SLOT_X].controller = controller;

			bsdName = (__bridge_transfer NSString *)copy_interface_name(controller);
			ok = [bsdName isEqualTo:interfaces[N_INTERFACES_SLOT_X].bsdName];
			if (!ok) {
				SCTestLog("*** interface %d not assigned an old name, expected \"%@\", assigned \"%@\"",
					  N_INTERFACES,
					  interfaces[N_INTERFACES_SLOT_X].bsdName,
					  bsdName);
				break;
			}

			interfaces[N_INTERFACES_SLOT_X].bsdName = bsdName;
		} while (0);
	}

	if (ok) {
		SCTestLog("Removing interfaces (again)");

		for (size_t i = 0; i < N_INTERFACES; i++) {
			// remove the interface
			ok = [test interfaceRemove:interfaces[i].controller];
			interfaces[i].controller = NULL;
			if (!ok) {
				break;
			}
		}
	}

	// cleanup
	for (size_t i = 0; i < N_INTERFACES; i++) {
		if (interfaces[i].controller != NULL) {
			CFRelease(interfaces[i].controller);
		}
	}

	return ok;
}

@end
