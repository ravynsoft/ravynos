/*
 * Copyright (c) 1999-2008 Apple Computer, Inc.  All Rights Reserved.
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

#ifndef _IOKIT_HID_IOHIDELEMENTEVENT_H
#define _IOKIT_HID_IOHIDELEMENTEVENT_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDBase.h>
#include <IOKit/hid/IOHIDKeys.h>

/*!
	@header IOHIDValue
    IOHIDValue defines a value at a given time from an parsed item 
    (IOHIDElement) contained within a Human Interface Device (HID) object.  It 
    is used to obtain either integer or data element values along with scaled 
    values based on physical or calibrated settings. IOHIDValue is a CFType 
    object and as such conforms to all the conventions expected such object.
    <p>
    This documentation assumes that you have a basic understanding of the material contained in <a href="http://developer.apple.com/documentation/DeviceDrivers/Conceptual/AccessingHardware/index.html"><i>Accessing Hardware From Applications</i></a>
    For definitions of I/O Kit terms used in this documentation, such as matching dictionary, family, and driver, see the overview of I/O Kit terms and concepts 
    in the "Device Access and the I/O Kit" chapter of <i>Accessing Hardware From Applications</i>.

    This documentation also assumes you have read <a href="http://developer.apple.com/documentation/DeviceDrivers/HumanInterfaceDeviceForceFeedback-date.html"><i>Human Interface Device & Force Feedback</i></a>.
    Please review documentation before using this reference.
    <p>
    All of the information described in this document is contained in the header file <font face="Courier New,Courier,Monaco">IOHIDValue.h</font> found at 
    <font face="Courier New,Courier,Monaco">/System/Library/Frameworks/IOKit.framework/Headers/hid/IOHIDValue.h</font>.
*/

__BEGIN_DECLS

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

/*!
	@function   IOHIDValueGetTypeID
	@abstract   Returns the type identifier of all IOHIDValue instances.
*/
CF_EXPORT
CFTypeID IOHIDValueGetTypeID(void)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDValueCreateWithIntegerValue
	@abstract   Creates a new element value using an integer value.
    @discussion IOHIDValueGetTimeStamp should represent OS AbsoluteTime, not CFAbsoluteTime.
                To obtain the OS AbsoluteTime, please reference the APIs declared in <mach/mach_time.h>
    @param      allocator The CFAllocator which should be used to allocate memory for the value.  This 
                parameter may be NULL in which case the current default CFAllocator is used. If this 
                reference is not a valid CFAllocator, the behavior is undefined.
    @param      element IOHIDElementRef associated with this value.
    @param      timeStamp OS absolute time timestamp for this value.
    @param      value Integer value to be copied to this object.
    @result     Returns a reference to a new IOHIDValueRef.
*/
CF_EXPORT
IOHIDValueRef IOHIDValueCreateWithIntegerValue(CFAllocatorRef _Nullable allocator, IOHIDElementRef element, uint64_t timeStamp, CFIndex value)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDValueCreateWithBytes
	@abstract   Creates a new element value using byte data.
    @discussion IOHIDValueGetTimeStamp should represent OS AbsoluteTime, not CFAbsoluteTime.
                To obtain the OS AbsoluteTime, please reference the APIs declared in <mach/mach_time.h>
    @param      allocator The CFAllocator which should be used to allocate memory for the value.  This 
                parameter may be NULL in which case the current default CFAllocator is used. If this 
                reference is not a valid CFAllocator, the behavior is undefined.
    @param      element IOHIDElementRef associated with this value.
    @param      timeStamp OS absolute time timestamp for this value.
    @param      bytes Pointer to a buffer of uint8_t to be copied to this object.
    @param      length Number of bytes in the passed buffer.
    @result     Returns a reference to a new IOHIDValueRef.
*/
CF_EXPORT
IOHIDValueRef _Nullable IOHIDValueCreateWithBytes(CFAllocatorRef _Nullable allocator, IOHIDElementRef element, uint64_t timeStamp, const uint8_t * bytes, CFIndex length)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDValueCreateWithBytesNoCopy
	@abstract   Creates a new element value using byte data without performing a copy.
    @discussion The timestamp value passed should represent OS AbsoluteTime, not CFAbsoluteTime.
                To obtain the OS AbsoluteTime, please reference the APIs declared in <mach/mach_time.h>
    @param      allocator The CFAllocator which should be used to allocate memory for the value.  This 
                parameter may be NULL in which case the current default CFAllocator is used. If this 
                reference is not a valid CFAllocator, the behavior is undefined.
    @param      element IOHIDElementRef associated with this value.
    @param      timeStamp OS absolute time timestamp for this value.
    @param      bytes Pointer to a buffer of uint8_t to be referenced by this object.
    @param      length Number of bytes in the passed buffer.
    @result     Returns a reference to a new IOHIDValueRef.
*/
CF_EXPORT
IOHIDValueRef _Nullable IOHIDValueCreateWithBytesNoCopy(CFAllocatorRef _Nullable allocator, IOHIDElementRef element, uint64_t timeStamp, const uint8_t * bytes, CFIndex length)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDValueGetElement
	@abstract   Returns the element value associated with this IOHIDValueRef.
    @param      value The value to be queried. If this parameter is not a valid IOHIDValueRef, the behavior is undefined.
    @result     Returns a IOHIDElementRef referenced by this value.
*/
CF_EXPORT
IOHIDElementRef IOHIDValueGetElement(IOHIDValueRef value)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDValueGetTimeStamp
	@abstract   Returns the timestamp value contained in this IOHIDValueRef.
    @discussion The timestamp value returned represents OS AbsoluteTime, not CFAbsoluteTime.
    @param      value The value to be queried. If this parameter is not a valid IOHIDValueRef, the behavior is undefined.
    @result     Returns a uint64_t representing the timestamp of this value.
*/
CF_EXPORT
uint64_t IOHIDValueGetTimeStamp(IOHIDValueRef value)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDValueGetLength
	@abstract   Returns the size, in bytes, of the value contained in this IOHIDValueRef.
    @param      value The value to be queried. If this parameter is not a valid IOHIDValueRef, the behavior is undefined.
    @result     Returns length of the value.
*/
CF_EXPORT
CFIndex IOHIDValueGetLength(IOHIDValueRef value)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDValueGetBytePtr
	@abstract   Returns a byte pointer to the value contained in this IOHIDValueRef.
    @param      value The value to be queried. If this parameter is not a valid IOHIDValueRef, the behavior is undefined.
    @result     Returns a pointer to the value.
*/
CF_EXPORT
const uint8_t * IOHIDValueGetBytePtr(IOHIDValueRef value)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDValueGetIntegerValue
	@abstract   Returns an integer representaion of the value contained in this IOHIDValueRef.
    @discussion The value is based on the logical element value contained in the report returned by the device.
    @param      value The value to be queried. If this parameter is not a valid IOHIDValueRef, the behavior is undefined.
    @result     Returns an integer representation of the value.
*/
CF_EXPORT
CFIndex IOHIDValueGetIntegerValue(IOHIDValueRef value)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDValueGetScaledValue
	@abstract   Returns an scaled representaion of the value contained in this IOHIDValueRef based on the scale type.
    @discussion The scaled value is based on the range described by the scale type's min and max, such that:
        <br>
        scaledValue = ((value - min) * (scaledMax - scaledMin) / (max - min)) + scaledMin
        <br>
        <b>Note:</b>
        <br>
        There are currently two types of scaling that can be applied:  
        <ul>
        <li><b>kIOHIDValueScaleTypePhysical</b>: Scales element value using the physical bounds of the device such that <b>scaledMin = physicalMin</b> and <b>scaledMax = physicalMax</b>.
        <li><b>kIOHIDValueScaleTypeCalibrated</b>: Scales element value such that <b>scaledMin = -1</b> and <b>scaledMax = 1</b>.  This value will also take into account the calibration properties associated with this element.
        </ul>
    @param      value The value to be queried. If this parameter is not a valid IOHIDValueRef, the behavior is undefined.
    @param      type The type of scaling to be performed.
    @result     Returns an scaled floating point representation of the value.
*/
CF_EXPORT
double_t IOHIDValueGetScaledValue(IOHIDValueRef value, IOHIDValueScaleType type)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

__END_DECLS

#endif /* _IOKIT_HID_IOHIDELEMENTEVENT_H */
