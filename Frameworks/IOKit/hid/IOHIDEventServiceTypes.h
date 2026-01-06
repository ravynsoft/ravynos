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

#ifndef IOHIDEventServiceTypes_h
#define IOHIDEventServiceTypes_h

/*!
 * @typedef IOHIDKeyboardEventOptions
 *
 * @abstract
 * Keyboard event options passed in to dispatchKeyboardEvent function in
 * IOHIDEventService.
 *
 * @field kIOHIDKeyboardEventOptionsNoKeyRepeat
 * Default behavior for keyboard events is to repeat keys if the key has been
 * held down for a certain amount of time defined in system preferences. Pass
 * in this option to not apply key repeat logic to this event.
 */
typedef enum {
    kIOHIDKeyboardEventOptionsNoKeyRepeat   = (1 << 8),
} IOHIDKeyboardEventOptions;

/*!
 * @typedef IOHIDPointerEventOptions
 *
 * @abstract
 * Pointer event options passed in to dispatch(Relative/Absolute)PointerEvent
 * function in IOHIDEventService.
 *
 * @field kIOHIDPointerEventOptionsNoAcceleration
 * Pointer events are subject to an acceleration algorithm. Pass in this option
 * if you do not wish to have acceleration logic applied to the pointer event.
 */
typedef enum {
    kIOHIDPointerEventOptionsNoAcceleration = (1 << 8),
} IOHIDPointerEventOptions;

/*!
 * @typedef IOHIDScrollEventOptions
 *
 * @abstract
 * Scroll event options passed in to dispatchScrollEvent function in
 * IOHIDEventService.
 *
 * @field kIOHIDScrollEventOptionsNoAcceleration
 * Scroll events are subject to an acceleration algorithm. Pass in this option
 * if you do not wish to have acceleration logic applied to the scroll event.
 */
typedef enum {
    kIOHIDScrollEventOptionsNoAcceleration  = (1 << 8),
} IOHIDScrollEventOptions;

#endif /* IOHIDEventServiceTypes_h */
