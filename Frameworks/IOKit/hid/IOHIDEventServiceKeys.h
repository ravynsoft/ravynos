/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 2019 Apple Computer, Inc.  All Rights Reserved.
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

#ifndef IOHIDEventServiceKeys_h
#define IOHIDEventServiceKeys_h

/*!
 * @define kIOHIDPointerAccelerationKey
 *
 * @abstract
 * Number property that contains the pointer acceleration value.
 */
#define kIOHIDPointerAccelerationKey "HIDPointerAcceleration"

/*!
 * @define kIOHIDPointerAccelerationTypeKey
 *
 * @abstract
 * String property containing the type of acceleration for pointer.
 * Supported types are:
 *      <code>kIOHIDPointerAccelerationKey</code>
 *      <code>kIOHIDMouseScrollAccelerationKey</code>
 *      <code>kIOHIDTrackpadAccelerationType</code>
 */
#define kIOHIDPointerAccelerationTypeKey "HIDPointerAccelerationType"

/*!
 * @define kIOHIDMouseScrollAccelerationKey
 *
 * @abstract
 * Number property that contains the mouse scroll acceleration value.
 */
#define kIOHIDMouseScrollAccelerationKey "HIDMouseScrollAcceleration"

/*!
 * @define kIOHIDMouseAccelerationTypeKey
 *
 * @abstract
 * Number property that contains the mouse acceleration value.
 */
#define kIOHIDMouseAccelerationTypeKey "HIDMouseAcceleration"

/*!
 * @define kIOHIDScrollAccelerationKey
 *
 * @abstract
 * Number property that contains the scroll acceleration value.
 */
#define kIOHIDScrollAccelerationKey "HIDScrollAcceleration"

/*!
 * @define kIOHIDScrollAccelerationTypeKey
 *
 * @abstract
 * Number property containing the type of acceleration for scroll.
 * Supported types are:
 *      <code>kIOHIDMouseScrollAccelerationKey</code>
 *      <code>kIOHIDTrackpadScrollAccelerationKey</code>
 */
#define kIOHIDScrollAccelerationTypeKey "HIDScrollAccelerationType"

#endif /* IOHIDDeviceTypes_h */
