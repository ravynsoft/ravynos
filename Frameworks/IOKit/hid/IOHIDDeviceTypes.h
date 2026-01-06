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

#ifndef IOHIDDeviceTypes_h
#define IOHIDDeviceTypes_h

#include <TargetConditionals.h>

#if TARGET_OS_DRIVERKIT
#include <DriverKit/IOTypes.h>
#include <DriverKit/IOReturn.h>
#else
#include <IOKit/IOReturn.h>
#include <IOKit/IOTypes.h>
#endif // TARGET_OS_DRIVERKIT

/*!
 * @typedef IOHIDReportType
 *
 * @abstract
 * Describes different type of HID reports.
 */
enum IOHIDReportType {
    kIOHIDReportTypeInput = 0,
    kIOHIDReportTypeOutput,
    kIOHIDReportTypeFeature,
    kIOHIDReportTypeCount
};
typedef enum IOHIDReportType IOHIDReportType;

/*!
 * @typedef IOHIDElementCommitDirection
 *
 * @abstract
 * Commit direction passed in to the commit() function of an IOHIDElement.
 *
 * @field kIOHIDElementCommitDirectionIn
 * Passing in kIOHIDElementCommitDirectionIn will issue a getReport call to the
 * device, and the element  will be updated with the value retrieved by the
 * device. The value can be accessed via the getValue() or getDataValue()
 * functions.
 *
 * @field kIOHIDElementCommitDirectionOut
 * Passing in kIOHIDElementCommitDirectionOut will issue a setReport call to the
 * device. Before issuing this call, the desired value should be set on the
 * element with the setValue() or setDataValue() functions.
 */
typedef enum {
    kIOHIDElementCommitDirectionIn,
    kIOHIDElementCommitDirectionOut
} IOHIDElementCommitDirection;

/*!
 * @typedef IOHIDElementCookie
 *
 * @abstract
 * Abstract data type used as a unique identifier for an element.
 */
#ifdef __LP64__
typedef uint32_t IOHIDElementCookie;
#else
typedef void * IOHIDElementCookie;
#endif

/*!
 * @typedef IOHIDElementType
 *
 * @abstract
 * Describes different types of HID elements.
 *
 * @discussion
 * Used by the IOHIDFamily to identify the type of element processed.
 * Represented by the key kIOHIDElementTypeKey in the dictionary describing the
 * element.
 *
 * @field kIOHIDElementTypeInput_Misc
 * Misc input data field or varying size.
 *
 * @field kIOHIDElementTypeInput_Button
 * One bit input data field.
 *
 * @field kIOHIDElementTypeInput_Axis
 * Input data field used to represent an axis.
 *
 * @field kIOHIDElementTypeInput_ScanCodes
 * Input data field used to represent a scan code or usage selector.
 *
 * @field kIOHIDElementTypeInput_NULL
 * Input data field used to represent the end of an input report when receiving
 * input elements.
 *
 * @field kIOHIDElementTypeOutput
 * Used to represent an output data field in a report.
 *
 * @field kIOHIDElementTypeFeature
 * Describes input and output elements not intended for consumption by the end
 * user.
 *
 * @field kIOHIDElementTypeCollection
 * Element used to identify a relationship between two or more elements.
 */
enum IOHIDElementType {
    kIOHIDElementTypeInput_Misc        = 1,
    kIOHIDElementTypeInput_Button      = 2,
    kIOHIDElementTypeInput_Axis        = 3,
    kIOHIDElementTypeInput_ScanCodes   = 4,
    kIOHIDElementTypeInput_NULL        = 5,
    kIOHIDElementTypeOutput            = 129,
    kIOHIDElementTypeFeature           = 257,
    kIOHIDElementTypeCollection        = 513
};
typedef enum IOHIDElementType IOHIDElementType;

enum {
    kIOHIDElementFlagsConstantMask        = 0x0001,
    kIOHIDElementFlagsVariableMask        = 0x0002,
    kIOHIDElementFlagsRelativeMask        = 0x0004,
    kIOHIDElementFlagsWrapMask            = 0x0008,
    kIOHIDElementFlagsNonLinearMask       = 0x0010,
    kIOHIDElementFlagsNoPreferredMask     = 0x0020,
    kIOHIDElementFlagsNullStateMask       = 0x0040,
    kIOHIDElementFlagsVolativeMask        = 0x0080,
    kIOHIDElementFlagsBufferedByteMask    = 0x0100
};
typedef uint32_t IOHIDElementFlags;

/*!
 * @typedef IOHIDElementCollectionType
 *
 * @abstract
 * Describes different types of HID collections.
 *
 * @discussion
 * Collections identify a relationship between two or more elements.
 *
 * @field kIOHIDElementCollectionTypePhysical
 * Used for a set of data items that represent data points collected at one
 * geometric point.
 *
 * @field kIOHIDElementCollectionTypeApplication
 * Identifies item groups serving different purposes in a single device.
 *
 * @field kIOHIDElementCollectionTypeLogical
 * Used when a set of data items form a composite data structure.
 *
 * @field kIOHIDElementCollectionTypeReport
 * Wraps all the fields in a report.
 *
 * @field kIOHIDElementCollectionTypeNamedArray
 * Contains an array of selector usages.
 *
 * @field kIOHIDElementCollectionTypeUsageSwitch
 * Modifies the meaning of the usage it contains.
 *
 * @field kIOHIDElementCollectionTypeUsageModifier
 * Modifies the meaning of the usage attached to the encompassing collection.
 */
enum IOHIDElementCollectionType{
    kIOHIDElementCollectionTypePhysical    = 0x00,
    kIOHIDElementCollectionTypeApplication,
    kIOHIDElementCollectionTypeLogical,
    kIOHIDElementCollectionTypeReport,
    kIOHIDElementCollectionTypeNamedArray,
    kIOHIDElementCollectionTypeUsageSwitch,
    kIOHIDElementCollectionTypeUsageModifier
};
typedef enum IOHIDElementCollectionType IOHIDElementCollectionType;

/*!
 * @typedef IOHIDValueScaleType
 *
 * @abstract
 * Describes different types of scaling that can be performed on element values.
 *
 * @field kIOHIDValueScaleTypeCalibrated
 * Type for value that is scaled with respect to the calibration properties.
 *
 * @field kIOHIDValueScaleTypePhysical
 * Type for value that is scaled with respect to the physical min and physical
 * max of the element.
 *
 * @field kIOHIDValueScaleTypeExponent
 * Type for value that is scaled with respect to the element's unit exponent.
 */
enum {
    kIOHIDValueScaleTypeCalibrated,
    kIOHIDValueScaleTypePhysical,
    kIOHIDValueScaleTypeExponent
};
typedef uint32_t IOHIDValueScaleType;

/*!
 * @typedef IOHIDValueOptions
 *
 * @abstract
 * Describes options for gathering element values.
 *
 * @field kIOHIDValueOptionsFlagRelativeSimple
 * Compares against previous value
 */
enum {
    kIOHIDValueOptionsFlagRelativeSimple    = (1<<0),
    kIOHIDValueOptionsFlagPrevious          = (1<<1),
    kIOHIDValueOptionsUpdateElementValues   = (1<<2)
};
typedef uint32_t IOHIDValueOptions;

/*!
 * @typedef IOHIDCompletionAction
 *
 * @abstract Function called when set/get report completes
 *
 * @param target
 * The target specified in the IOHIDCompletion struct.
 *
 * @param parameter
 * The parameter specified in the IOHIDCompletion struct.
 *
 * @param status
 * Completion status
 */
typedef void (*IOHIDCompletionAction)(void *target,
                                      void *parameter,
                                      IOReturn status,
                                      uint32_t bufferSizeRemaining);

/*!
 * @typedef IOHIDCompletion
 *
 * @abstract
 * Struct spefifying action to perform when set/get report completes.
 *
 * @var target
 * The target to pass to the action function.
 *
 * @var action
 * The function to call.
 *
 * @var parameter
 * The parameter to pass to the action function.
 */
typedef struct IOHIDCompletion {
    void *target;
    IOHIDCompletionAction action;
    void *parameter;
} IOHIDCompletion;

/*!
 * @abstract
 * Option bits for IOHIDDevice::handleReport, IOHIDDevice::getReport, and
 * IOHIDDevice::setReport
 *
 * @field kIOHIDReportOptionNotInterrupt
 * Tells method that the report passed was not interrupt driven.
 */
enum
{
    kIOHIDReportOptionNotInterrupt      = 0x100,
    kIOHIDReportOptionVariableSize      = 0x200
};

/*!
 * @typedef HIDReportCommandType
 *
 * @abstract
 * Type of the report command for DriverKit driver
 */
typedef enum {
    kIOHIDReportCommandSetReport,
    kIOHIDReportCommandGetReport
} HIDReportCommandType;

#endif /* IOHIDDeviceTypes_h */
