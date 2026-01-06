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

#ifndef _IOKIT_HID_IOHIDELEMENT_USER_H
#define _IOKIT_HID_IOHIDELEMENT_USER_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDBase.h>

/*!
	@header IOHIDElement
    IOHIDElement defines a parsed item contained within a Human Interface Device
    (HID) object.  It is used to obtain properties of the parsed.  It can also
    be used to set properties such as calibration settings.  IOHIDElement is a 
    CFType object and as such conforms to all the conventions expected such 
    object.
    <p>
    This documentation assumes that you have a basic understanding of the material contained in <a href="http://developer.apple.com/documentation/DeviceDrivers/Conceptual/AccessingHardware/index.html"><i>Accessing Hardware From Applications</i></a>
    For definitions of I/O Kit terms used in this documentation, such as matching dictionary, family, and driver, see the overview of I/O Kit terms and concepts 
    in the "Device Access and the I/O Kit" chapter of <i>Accessing Hardware From Applications</i>.
    This documentation also assumes you have read <a href="http://developer.apple.com/documentation/DeviceDrivers/HumanInterfaceDeviceForceFeedback-date.html"><i>Human Interface Device & Force Feedback</i></a>.
    Please review documentation before using this reference.
    <p>
    All of the information described in this document is contained in the header file <font face="Courier New,Courier,Monaco">IOHIDElement.h</font> found at 
    <font face="Courier New,Courier,Monaco">/System/Library/Frameworks/IOKit.framework/Headers/hid/IOHIDElement.h</font>.
*/

__BEGIN_DECLS

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED


/*!
	@function   IOHIDElementGetTypeID
	@abstract   Returns the type identifier of all IOHIDElement instances.
*/
CF_EXPORT
CFTypeID IOHIDElementGetTypeID(void)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementCreateWithDictionary
	@abstract   Creates an element from a dictionary.
    @discussion The dictionary should contain keys defined in IOHIDKeys.h and start with kIOHIDElement.  This call is meant be used by a IOHIDDeviceDeviceInterface object.
    @param      allocator Allocator to be used during creation.
    @param      dictionary dictionary containing values in which to create element.
    @result     Returns a new IOHIDElementRef.
*/
CF_EXPORT
IOHIDElementRef IOHIDElementCreateWithDictionary(CFAllocatorRef _Nullable allocator, CFDictionaryRef dictionary)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetDevice
	@abstract   Obtain the device associated with the element.
    @param      element IOHIDElement to be queried. 
    @result     Returns the a reference to the device.
*/
CF_EXPORT
IOHIDDeviceRef IOHIDElementGetDevice(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetParent
	@abstract   Returns the parent for the element.
    @discussion The parent element can be an element of type kIOHIDElementTypeCollection.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns an IOHIDElementRef referencing the parent element.
*/
CF_EXPORT
IOHIDElementRef _Nullable IOHIDElementGetParent(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetChildren
	@abstract   Returns the children for the element.
    @discussion An element of type kIOHIDElementTypeCollection usually contains children.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns an CFArrayRef containing element objects of type IOHIDElementRef.
*/
CF_EXPORT
CFArrayRef _Nullable IOHIDElementGetChildren(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementAttach
	@abstract   Establish a relationship between one or more elements.
    @discussion This is useful for grouping HID elements with related functionality.
    @param      element The element to be modified. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @param      toAttach The element to be attached. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
*/
CF_EXPORT
void IOHIDElementAttach(IOHIDElementRef element, IOHIDElementRef toAttach)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementDetach
	@abstract   Remove a relationship between one or more elements.
    @discussion This is useful for grouping HID elements with related functionality.
    @param      element The element to be modified. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @param      toDetach The element to be detached. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
*/
CF_EXPORT
void IOHIDElementDetach(IOHIDElementRef element, IOHIDElementRef toDetach)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementCopyAttached
	@abstract   Obtain attached elements.
    @discussion Attached elements are those that have been grouped via IOHIDElementAttach.
    @param      element The element to be modified. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns a copy of the current attached elements.
*/
CF_EXPORT
CFArrayRef _Nullable IOHIDElementCopyAttached(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetCookie
	@abstract   Retrieves the cookie for the element.
    @discussion The IOHIDElementCookie represent a unique identifier for an element within a device.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the IOHIDElementCookie for the element.
*/
CF_EXPORT
IOHIDElementCookie IOHIDElementGetCookie(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetType
	@abstract   Retrieves the type for the element.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the IOHIDElementType for the element.
*/
CF_EXPORT
IOHIDElementType IOHIDElementGetType(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetCollectionType
	@abstract   Retrieves the collection type for the element.
    @discussion The value returned by this method only makes sense if the element type is kIOHIDElementTypeCollection.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the IOHIDElementCollectionType for the element.
*/
CF_EXPORT
IOHIDElementCollectionType IOHIDElementGetCollectionType(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetUsagePage
	@abstract   Retrieves the usage page for an element.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the usage page for the element.
*/
CF_EXPORT
uint32_t IOHIDElementGetUsagePage(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetUsage
	@abstract   Retrieves the usage for an element.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the usage for the element.
*/
CF_EXPORT
uint32_t IOHIDElementGetUsage(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementIsVirtual
	@abstract   Returns the virtual property for the element.
    @discussion Indicates whether the element is a virtual element.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the TRUE if virtual or FALSE if not.
*/
CF_EXPORT
Boolean IOHIDElementIsVirtual(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementIsRelative
	@abstract   Returns the relative property for the element.
    @discussion Indicates whether the data is relative (indicating the change in value from the last report) or absolute 
                (based on a fixed origin).
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns TRUE if relative or FALSE if absolute.
*/
CF_EXPORT
Boolean IOHIDElementIsRelative(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementIsWrapping
	@abstract   Returns the wrap property for the element.
    @discussion Wrap indicates whether the data "rolls over" when reaching either the extreme high or low value.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns TRUE if wrapping or FALSE if non-wrapping.
*/
CF_EXPORT
Boolean IOHIDElementIsWrapping(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementIsArray
	@abstract   Returns the array property for the element.
    @discussion Indicates whether the element represents variable or array data values. Variable values represent data from a 
                physical control.  An array returns an index in each field that corresponds to the pressed button 
                (like keyboard scan codes).
                <br>
                <b>Note:</b> The HID Manager will represent most elements as "variable" including the possible usages of an array.  
                Array indices will remain as "array" elements with a usage of 0xffffffff.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns TRUE if array or FALSE if variable.
*/
CF_EXPORT
Boolean IOHIDElementIsArray(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementIsNonLinear
	@abstract   Returns the linear property for the element.
    @discussion Indicates whether the value for the element has been processed in some way, and no longer represents a linear 
                relationship between what is measured and the value that is reported.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns TRUE if non linear or FALSE if linear.
*/
CF_EXPORT
Boolean IOHIDElementIsNonLinear(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementHasPreferredState
	@abstract   Returns the preferred state property for the element.
    @discussion Indicates whether the element has a preferred state to which it will return when the user is not physically 
                interacting with the control.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns TRUE if preferred state or FALSE if no preferred state.
*/
CF_EXPORT
Boolean IOHIDElementHasPreferredState(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementHasNullState
	@abstract   Returns the null state property for the element.
    @discussion Indicates whether the element has a state in which it is not sending meaningful data. 
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns TRUE if null state or FALSE if no null state.
*/
CF_EXPORT
Boolean IOHIDElementHasNullState(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetName
	@abstract   Returns the name for the element.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns CFStringRef containing the element name.
*/
CF_EXPORT
CFStringRef IOHIDElementGetName(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetReportID
	@abstract   Returns the report ID for the element.
    @discussion The report ID represents what report this particular element belongs to.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the report ID.
*/
CF_EXPORT
uint32_t IOHIDElementGetReportID(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetReportSize
	@abstract   Returns the report size in bits for the element.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the report size.
*/
CF_EXPORT
uint32_t IOHIDElementGetReportSize(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetReportCount
	@abstract   Returns the report count for the element.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the report count.
*/
CF_EXPORT
uint32_t IOHIDElementGetReportCount(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetUnit
	@abstract   Returns the unit property for the element.
    @discussion The unit property is described in more detail in Section 6.2.2.7 of the 
                "Device Class Definition for Human Interface Devices(HID)" Specification, Version 1.11.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the unit.
*/
CF_EXPORT
uint32_t IOHIDElementGetUnit(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetUnitExponent
	@abstract   Returns the unit exponenet in base 10 for the element.
    @discussion The unit exponent property is described in more detail in Section 6.2.2.7 of the 
                "Device Class Definition for Human Interface Devices(HID)" Specification, Version 1.11.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the unit exponent.
*/
CF_EXPORT
uint32_t IOHIDElementGetUnitExponent(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetLogicalMin
	@abstract   Returns the minimum value possible for the element.
    @discussion This corresponds to the logical minimun, which indicates the lower bounds of a variable element.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the logical minimum.
*/
CF_EXPORT
CFIndex IOHIDElementGetLogicalMin(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetLogicalMax
	@abstract   Returns the maximum value possible for the element.
    @discussion This corresponds to the logical maximum, which indicates the upper bounds of a variable element.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the logical maximum.
*/
CF_EXPORT
CFIndex IOHIDElementGetLogicalMax(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetPhysicalMin
	@abstract   Returns the scaled minimum value possible for the element.
    @discussion Minimum value for the physical extent of a variable element. This represents the value for the logical minimum with units applied to it.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the physical minimum.
*/
CF_EXPORT
CFIndex IOHIDElementGetPhysicalMin(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetPhysicalMax
	@abstract   Returns the scaled maximum value possible for the element.
    @discussion Maximum value for the physical extent of a variable element.  This represents the value for the logical maximum with units applied to it.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @result     Returns the physical maximum.
*/
CF_EXPORT
CFIndex IOHIDElementGetPhysicalMax(IOHIDElementRef element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementGetProperty
	@abstract   Returns the an element property.
    @discussion Property keys are prefixed by kIOHIDElement and declared in IOHIDKeys.h.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @param      key The key to be used when querying the element.
    @result     Returns the property.
*/
CF_EXPORT
CFTypeRef _Nullable IOHIDElementGetProperty(IOHIDElementRef element, CFStringRef key)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDElementSetProperty
	@abstract   Sets an element property.
    @discussion This method can be used to set arbitrary element properties, such as application specific references.
    @param      element The element to be queried. If this parameter is not a valid IOHIDElementRef, the behavior is undefined.
    @param      key The key to be used when querying the element.
    @result     Returns TRUE if successful.
*/
CF_EXPORT 
Boolean IOHIDElementSetProperty(IOHIDElementRef element, CFStringRef key, CFTypeRef property)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

__END_DECLS

#endif /* _IOKIT_HID_IOHIDELEMENT_USER_H */
