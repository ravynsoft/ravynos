/*
 * Copyright (c) 2016 Apple Computer, Inc. All rights reserved.
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

#ifndef IOHIDProperties_h
#define IOHIDProperties_h

#include <IOKit/hid/IOHIDEventServiceKeys.h>

/*!
 * @define      kIOHIDMouseAccelerationType
 *
 * @abstract    CFNumber that contains the mouse acceleration value.
 */
#define kIOHIDMouseAccelerationType                 "HIDMouseAcceleration"

/*!
 * @define      kIOHIDPointerButtonMode
 *
 * @abstract    CFNumber containing the current pointer button mode.
 *              See IOHIDButtonModes enumerator for possible modes.
 */
#define kIOHIDPointerButtonMode                     "HIDPointerButtonMode"
#define kIOHIDPointerButtonModeKey                  kIOHIDPointerButtonMode

/*!
 * @define      kIOHIDUserUsageMapKey
 *
 * @abstract    CFArray of dictionaries that contain user defined key mappings.
 */
#define kIOHIDUserKeyUsageMapKey                     "UserKeyMapping"

/*!
 * @define      kIOHIDKeyboardCapsLockDelayOverride
 *
 * @abstract    CFNumber containing the delay (in ms) before the caps lock key is activated.
 */
#define kIOHIDKeyboardCapsLockDelayOverride         "CapsLockDelayOverride"
#define kIOHIDKeyboardCapsLockDelayOverrideKey      kIOHIDKeyboardCapsLockDelayOverride

/*!
 * @define      kIOHIDServiceEjectDelayKey
 *
 * @abstract    CFNumber containing the delay (in ms) before the eject key is activated.
 */
#define kIOHIDServiceEjectDelayKey                  "EjectDelay"

/*!
 * @define      kIOHIDServiceInitialKeyRepeatDelayKey
 *
 * @abstract    CFNumber containing the delay (in ns) before the initial key repeat.
 *              If value is 0, there are no repeats.
 */
#define kIOHIDServiceInitialKeyRepeatDelayKey       "HIDInitialKeyRepeat"

/*!
 * @define      kIOHIDServiceKeyRepeatDelayKey
 *
 * @abstract    CFNumber containing the delay (in ns) for subsequent key repeats.
 *              If value is 0, there are no repeats (including initial).
 */
#define kIOHIDServiceKeyRepeatDelayKey              "HIDKeyRepeat"

/*!
 * @define      kIOHIDIdleTimeMicrosecondsKey
 *
 * @abstract    CFNumber containing the HID idle time in microseconds.
 */
#define kIOHIDIdleTimeMicrosecondsKey               "HIDIdleTimeMicroseconds"

/*!
 * @define      kIOHIDServiceCapsLockStateKey
 *
 * @abstract    CFBoolean for setting/getting the caps lock state of the
 *              service. The caps lock LED will be updated to reflect the state.
 */
#define kIOHIDServiceCapsLockStateKey                "HIDCapsLockState"

#endif /* IOHIDProperties_h */
