/*
 * Copyright (c) 2008-2014 Apple Inc. All rights reserved.
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

#include <CoreFoundation/CFMachPort.h>

#if !__has_feature(objc_arc)
struct IONotificationPort
{
    mach_port_t		masterPort;
    mach_port_t		wakePort;
    CFMachPortRef	cfmachPort;
    CFRunLoopSourceRef	source;
    dispatch_source_t	dispatchSource;
    int32_t			refcount;
};
typedef struct IONotificationPort IONotificationPort;
#endif

void
IODispatchCalloutFromCFMessage(
        CFMachPortRef port,
        void *msg,
        CFIndex size,
        void *info );

kern_return_t
iokit_user_client_trap(
                       io_connect_t	connect,
                       unsigned int	index,
                       uintptr_t p1,
                       uintptr_t p2,
                       uintptr_t p3,
                       uintptr_t p4,
                       uintptr_t p5,
                       uintptr_t p6 );

kern_return_t
IOServiceGetState(
	io_service_t    service,
	uint64_t *	state );

kern_return_t
IOServiceGetBusyStateAndTime(
	io_service_t    service,
	uint64_t *	state,
	uint32_t *	busy_state,
	uint64_t *	accumulated_busy_time);

// masks for getState()
enum {
    kIOServiceInactiveState	= 0x00000001,
    kIOServiceRegisteredState	= 0x00000002,
    kIOServiceMatchedState	= 0x00000004,
    kIOServiceFirstPublishState	= 0x00000008,
    kIOServiceFirstMatchState	= 0x00000010
};

kern_return_t
_IOServiceGetAuthorizationID(
	io_service_t    service,
	uint64_t *	authorizationID );

kern_return_t
_IOServiceSetAuthorizationID(
	io_service_t    service,
	uint64_t	authorizationID );

boolean_t
_IOObjectConformsTo(
	io_object_t	object,
	const io_name_t	className,
	uint64_t        options);

kern_return_t
_IOObjectGetClass(
	io_object_t	object,
	uint64_t        options,
	io_name_t       className);

CFStringRef
_IOObjectCopyClass(
	io_object_t     object,
	uint64_t        options);


