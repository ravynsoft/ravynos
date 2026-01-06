/*
 * Copyright (c) 1999-2009 Apple, Inc.  All Rights Reserved.
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

#ifndef _IOKIT_IOETHERNET_CONTROLLER_USER_H
#define _IOKIT_IOETHERNET_CONTROLLER_USER_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

__BEGIN_DECLS

typedef struct __IOEthernetController * IOEthernetControllerRef;

typedef void (*IOEthernetControllerCallback)(IOEthernetControllerRef controller, void * refcon);

extern CFTypeRef kIOEthernetHardwareAddress;
extern CFTypeRef kIOUserEthernetInterfaceRole;

/*!
 * @const kIOUserEthernetInterfaceMergeProperties
 * @abstract
 * The key for a dictionary of properties to merge into the property table
 * of the Ethernet interface.
 * @discussion
 * The properties supplied to <code>IOEthernetControllerCreate</code> may
 * contain a dictionary stored using this key. The contents of the dictionary
 * are merged to the property table of the IOEthernetInterface when it is
 * initialized, before the interface object is registered and attached as
 * a child of the Ethernet controller.
 */
extern CFTypeRef kIOUserEthernetInterfaceMergeProperties;

/*!
	@function   IOEthernetControllerGetTypeID
	@abstract   Returns the type identifier of all IOUserEthernet instances.
*/
CF_EXPORT
CFTypeID IOEthernetControllerGetTypeID(void);

CF_EXPORT
IOEthernetControllerRef IOEthernetControllerCreate(
                                CFAllocatorRef                  allocator, 
                                CFDictionaryRef                 properties);

CF_EXPORT
io_object_t IOEthernetControllerGetIONetworkInterfaceObject(
                                IOEthernetControllerRef         controller);

CF_EXPORT
IOReturn    IOEthernetControllerSetLinkStatus(
                                IOEthernetControllerRef         controller, 
                                Boolean                         state);

CF_EXPORT
IOReturn    IOEthernetControllerSetPowerSavings(
								IOEthernetControllerRef			controller,
								Boolean							state);

CF_EXPORT
CFIndex     IOEthernetControllerReadPacket(
                                IOEthernetControllerRef         controller, 
                                uint8_t *                       buffer,
                                CFIndex                         bufferLength);

CF_EXPORT
IOReturn    IOEthernetControllerWritePacket(
                                IOEthernetControllerRef         controller, 
                                const uint8_t *                 buffer,
                                CFIndex                         bufferLength);

CF_EXPORT
void        IOEthernetControllerScheduleWithRunLoop(
                                IOEthernetControllerRef         controller, 
                                CFRunLoopRef                    runLoop,
                                CFStringRef                     runLoopMode);

CF_EXPORT
void        IOEthernetControllerUnscheduleFromRunLoop(
                                IOEthernetControllerRef         controller, 
                                CFRunLoopRef                    runLoop,
                                CFStringRef                     runLoopMode);

CF_EXPORT
void        IOEthernetControllerSetDispatchQueue(
                                IOEthernetControllerRef         controller, 
                                dispatch_queue_t                queue);

CF_EXPORT
void        IOEthernetControllerRegisterEnableCallback(
                                IOEthernetControllerRef         controller, 
                                IOEthernetControllerCallback    callback, 
                                void *                          refcon);

CF_EXPORT
void        IOEthernetControllerRegisterDisableCallback(
                                IOEthernetControllerRef         controller, 
                                IOEthernetControllerCallback    callback, 
                                void *                          refcon);

CF_EXPORT
void        IOEthernetControllerRegisterPacketAvailableCallback(
                                IOEthernetControllerRef         controller, 
                                IOEthernetControllerCallback    callback, 
                                void *                          refcon);

CF_EXPORT
void        IOEthernetControllerRegisterBSDAttachCallback(
                                IOEthernetControllerRef         controller,
                                IOEthernetControllerCallback    callback, 
                                void *                          refcon);

CF_EXPORT
int         IOEthernetControllerGetBSDSocket(
                                IOEthernetControllerRef         controller);

__END_DECLS

#endif /* _IOKIT_IOETHERNET_CONTROLLER_USER_H */
