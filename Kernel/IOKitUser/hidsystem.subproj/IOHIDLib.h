/*
 * Copyright (c) 1999-2009 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1999 Apple Computer, Inc.  All rights reserved.
 *
 * HISTORY
 *
 */

#ifndef _IOKIT_IOHIDLIB_H
#define _IOKIT_IOHIDLIB_H

#include <IOKit/hidsystem/IOHIDShared.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

extern kern_return_t
IOHIDCreateSharedMemory( io_connect_t connect,
	unsigned int version );

extern kern_return_t
IOHIDSetEventsEnable( io_connect_t connect,
	boolean_t enable );

extern kern_return_t
IOHIDSetCursorEnable( io_connect_t connect,
	boolean_t enable );

enum {
    // Options for IOHIDPostEvent()
    kIOHIDSetGlobalEventFlags       = 0x00000001,
    kIOHIDSetCursorPosition         = 0x00000002,
    kIOHIDSetRelativeCursorPosition = 0x00000004,
    kIOHIDPostHIDManagerEvent       = 0x00000008
};

extern kern_return_t
IOHIDPostEvent( io_connect_t        connect,
                UInt32              eventType,
                IOGPoint            location,
                const NXEventData * eventData,
                UInt32              eventDataVersion,
                IOOptionBits        eventFlags,
                IOOptionBits        options );

extern kern_return_t
IOHIDSetMouseLocation( io_connect_t connect, int x, int y);

extern kern_return_t
IOHIDGetButtonEventNum( io_connect_t connect,
	NXMouseButton button, int * eventNum ) __deprecated;

extern kern_return_t
IOHIDGetScrollAcceleration( io_connect_t handle, double * acceleration ) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_12, __IPHONE_NA, __IPHONE_NA);

extern kern_return_t
IOHIDSetScrollAcceleration( io_connect_t handle, double acceleration ) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_12, __IPHONE_NA, __IPHONE_NA);

extern kern_return_t
IOHIDGetMouseAcceleration( io_connect_t handle, double * acceleration ) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_12, __IPHONE_NA, __IPHONE_NA);

extern kern_return_t
IOHIDSetMouseAcceleration( io_connect_t handle, double acceleration ) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_12, __IPHONE_NA, __IPHONE_NA);

extern kern_return_t
IOHIDGetMouseButtonMode( io_connect_t handle, int * mode );

extern kern_return_t
IOHIDSetMouseButtonMode( io_connect_t handle, int mode ) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_12, __IPHONE_NA, __IPHONE_NA);

extern kern_return_t
IOHIDGetAccelerationWithKey( io_connect_t handle, CFStringRef key, double * acceleration ) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_12, __IPHONE_NA, __IPHONE_NA);

extern kern_return_t
IOHIDSetAccelerationWithKey( io_connect_t handle, CFStringRef key, double acceleration ) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_12, __IPHONE_NA, __IPHONE_NA);

extern kern_return_t
IOHIDGetParameter( io_connect_t handle, CFStringRef key, IOByteCount maxSize, 
		void * bytes, IOByteCount * actualSize ) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_12, __IPHONE_2_0, __IPHONE_10_0);

extern kern_return_t
IOHIDSetParameter( io_connect_t handle, CFStringRef key, 
		const void * bytes, IOByteCount size ) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_12, __IPHONE_NA, __IPHONE_NA);

extern kern_return_t
IOHIDCopyCFTypeParameter( io_connect_t handle, CFStringRef key,
                CFTypeRef * parameter );

extern kern_return_t
IOHIDSetCFTypeParameter( io_connect_t handle, CFStringRef key,
                CFTypeRef parameter );

// selectors are found in IOHIDParameter.h
extern kern_return_t
IOHIDGetModifierLockState( io_connect_t handle, int selector, bool *state );

extern kern_return_t
IOHIDSetModifierLockState( io_connect_t handle, int selector, bool state );

extern kern_return_t
IOHIDGetStateForSelector( io_connect_t handle, int selector, UInt32 *state );

extern kern_return_t
IOHIDSetStateForSelector( io_connect_t handle, int selector, UInt32 state );

// Used by Window Server only
extern kern_return_t
IOHIDRegisterVirtualDisplay( io_connect_t handle, UInt32 *display_token );

extern kern_return_t
IOHIDUnregisterVirtualDisplay( io_connect_t handle, UInt32 display_token );

extern kern_return_t
IOHIDSetVirtualDisplayBounds( io_connect_t handle, UInt32 display_token, const IOGBounds * bounds );

extern kern_return_t
IOHIDGetActivityState( io_connect_t handle, bool *hidActivityIdle );

/*
 * @typedef IOHIDRequestType
 *
 * @abstract
 * Request type passed in to IOHIDCheckAccess/IOHIDRequestAccess.
 *
 * @field kIOHIDRequestTypePostEvent
 * Request to post event through IOHIDPostEvent API. Access must be granted
 * by the user to use this API. If you do not request access through the
 * IOHIDRequestAccess call, the request will be made on the process's behalf
 * in the IOHIDPostEvent call.
 *
 * @field kIOHIDRequestTypeListenEvent
 * Request to listen to event through IOHIDManager/IOHIDDevice API. Access must
 * be granted by the user to use this API. If you do not request access through
 * the IOHIDRequestAccess call, the request will be made on the process's behalf
 * in IOHIDManagerOpen/IOHIDDeviceOpen calls.
 *
 */
typedef enum {
    kIOHIDRequestTypePostEvent,
    kIOHIDRequestTypeListenEvent
} IOHIDRequestType;

/*
 * @typedef IOHIDAccessType
 *
 * @abstract
 * Enumerator of access types returned from IOHIDCheckAccess.
 */
typedef enum {
    kIOHIDAccessTypeGranted,
    kIOHIDAccessTypeDenied,
    kIOHIDAccessTypeUnknown
} IOHIDAccessType;

/*!
 * @function IOHIDCheckAccess
 *
 * @abstract
 * Checks if the process has access to a specific IOHIDRequestType. A process
 * may request access by calling the IOHIDRequestAccess function.
 *
 * @param requestType
 * The request type defined in the IOHIDRequestType enumerator.
 *
 * @result
 * Returns an access type defined in the IOHIDAccessType enumerator.
 */
IOHIDAccessType IOHIDCheckAccess(IOHIDRequestType requestType)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDRequestAccess
 *
 * @abstract
 * Requests access from the user for a specific IOHIDRequestType.
 *
 * @discussion
 * Processes that wish to post events through the IOHIDPostEvent API, or receive
 * reports through the IOHIDManager/IOHIDDevice API must be granted access first
 * by the user. If you do not call this API, it will be called on your behalf
 * when the API are used.
 *
 * @param requestType
 * The request type defined in the IOHIDRequestType enumerator.
 *
 * @result
 * Returns true if access was granted.
 */
bool IOHIDRequestAccess(IOHIDRequestType requestType)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

__END_DECLS

#endif /* ! _IOKIT_IOHIDLIB_H */

