/*
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
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

#ifndef _IOKIT_HID_IOHIDKEYS_H_
#define _IOKIT_HID_IOHIDKEYS_H_

#include <sys/cdefs.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <IOKit/IOReturn.h>
#include <IOKit/IOMessage.h>
#include <IOKit/hid/IOHIDProperties.h>
#include <IOKit/hid/IOHIDDeviceTypes.h>
#include <IOKit/hid/IOHIDDeviceKeys.h>

__BEGIN_DECLS

/* The following keys are used to search the IORegistry for HID related services
*/

/* This is used to find HID Devices in the IORegistry */
#define kIOHIDDeviceKey                     "IOHIDDevice"

/*!
    @defined HID Device Property Keys
    @abstract Keys that represent properties of a paticular device.
    @discussion Keys that represent properties of a paticular device.  Can be added
        to your matching dictionary when refining searches for HID devices.
        <br><br>
        <b>Please note:</b><br>
        kIOHIDPrimaryUsageKey and kIOHIDPrimaryUsagePageKey are no longer 
        rich enough to describe a device's capabilities.  Take, for example, a
        device that describes both a keyboard and a mouse in the same descriptor.  
        The previous behavior was to only describe the keyboard behavior with the 
        primary usage and usage page.   Needless to say, this would sometimes cause 
        a program interested in mice to skip this device when matching.  
        <br>
        Thus we have added 3 
        additional keys:
        <ul>
            <li>kIOHIDDeviceUsageKey</li>
            <li>kIOHIDDeviceUsagePageKey</li>
            <li>kIOHIDDeviceUsagePairsKey</li>
        </ul>
        kIOHIDDeviceUsagePairsKey is used to represent an array of dictionaries containing 
        key/value pairs referenced by kIOHIDDeviceUsageKey and kIOHIDDeviceUsagePageKey.  
        These usage pairs describe all application type collections (behaviors) defined 
        by the device.
        <br><br>
        An application intersted in only matching on one criteria would only add the 
        kIOHIDDeviceUsageKey and kIOHIDDeviceUsagePageKey keys to the matching dictionary.
        If it is interested in a device that has multiple behaviors, the application would
        instead add an array or dictionaries referenced by kIOHIDDeviceUsagePairsKey to his 
        matching dictionary.
*/
#define kIOHIDVendorIDSourceKey             "VendorIDSource"
#define kIOHIDStandardTypeKey               "StandardType"
#define kIOHIDSampleIntervalKey             "SampleInterval"
#define kIOHIDResetKey                      "Reset"
#define kIOHIDKeyboardLanguageKey           "KeyboardLanguage"
#define kIOHIDAltHandlerIdKey               "alt_handler_id"
#define kIOHIDDisplayIntegratedKey          "DisplayIntegrated"
#define kIOHIDProductIDMaskKey              "ProductIDMask"
#define kIOHIDProductIDArrayKey             "ProductIDArray"
#define kIOHIDPowerOnDelayNSKey             "HIDPowerOnDelayNS"
#define kIOHIDCategoryKey                   "Category"
#define kIOHIDMaxResponseLatencyKey         "MaxResponseLatency"
#define kIOHIDUniqueIDKey                   "UniqueID"
#define kIOHIDModelNumberKey                "ModelNumber"


#define kIOHIDTransportUSBValue                 "USB"
#define kIOHIDTransportBluetoothValue           "Bluetooth"
#define kIOHIDTransportBluetoothLowEnergyValue  "BluetoothLowEnergy"
#define kIOHIDTransportAIDBValue                "AID"
#define kIOHIDTransportI2CValue                 "I2C"
#define kIOHIDTransportSPIValue                 "SPI"
#define kIOHIDTransportSerialValue              "Serial"
#define kIOHIDTransportIAPValue                 "iAP"
#define kIOHIDTransportAirPlayValue             "AirPlay"
#define kIOHIDTransportSPUValue                 "SPU"
#define kIOHIDTransportBTAACPValue              "BT-AACP"


#define kIOHIDCategoryAutomotiveValue       "Automotive"

/*!
    @define kIOHIDElementKey
    @abstract Keys that represents an element property.
    @discussion Property for a HID Device or element dictionary.
        Elements can be heirarchical, so they can contain other elements.
*/
#define kIOHIDElementKey                    "Elements"

/*!
    @defined HID Element Dictionary Keys
    @abstract Keys that represent properties of a particular elements.
    @discussion These keys can also be added to a matching dictionary 
        when searching for elements via copyMatchingElements.  
*/
#define kIOHIDElementCookieKey                      "ElementCookie"
#define kIOHIDElementTypeKey                        "Type"
#define kIOHIDElementCollectionTypeKey              "CollectionType"
#define kIOHIDElementUsageKey                       "Usage"
#define kIOHIDElementUsagePageKey                   "UsagePage"
#define kIOHIDElementMinKey                         "Min"
#define kIOHIDElementMaxKey                         "Max"
#define kIOHIDElementScaledMinKey                   "ScaledMin"
#define kIOHIDElementScaledMaxKey                   "ScaledMax"
#define kIOHIDElementSizeKey                        "Size"
#define kIOHIDElementReportSizeKey                  "ReportSize"
#define kIOHIDElementReportCountKey                 "ReportCount"
#define kIOHIDElementReportIDKey                    "ReportID"
#define kIOHIDElementIsArrayKey                     "IsArray"
#define kIOHIDElementIsRelativeKey                  "IsRelative"
#define kIOHIDElementIsWrappingKey                  "IsWrapping"
#define kIOHIDElementIsNonLinearKey                 "IsNonLinear"
#define kIOHIDElementHasPreferredStateKey           "HasPreferredState"
#define kIOHIDElementHasNullStateKey                "HasNullState"
#define kIOHIDElementFlagsKey                       "Flags"
#define kIOHIDElementUnitKey                        "Unit"
#define kIOHIDElementUnitExponentKey                "UnitExponent"
#define kIOHIDElementNameKey                        "Name"
#define kIOHIDElementValueLocationKey               "ValueLocation"
#define kIOHIDElementDuplicateIndexKey              "DuplicateIndex"
#define kIOHIDElementParentCollectionKey            "ParentCollection"
#define kIOHIDElementVariableSizeKey                "VariableSize"

#ifndef __ppc__
    #define kIOHIDElementVendorSpecificKey          "VendorSpecific"
#else
    #define kIOHIDElementVendorSpecificKey          "VendorSpecifc"
#endif

/*!
    @defined HID Element Match Keys
    @abstract Keys used for matching particular elements.
    @discussion These keys should only be used with a matching  
        dictionary when searching for elements via copyMatchingElements.  
*/
#define kIOHIDElementCookieMinKey           "ElementCookieMin"
#define kIOHIDElementCookieMaxKey           "ElementCookieMax"
#define kIOHIDElementUsageMinKey            "UsageMin"
#define kIOHIDElementUsageMaxKey            "UsageMax"

/*!
    @defined kIOHIDElementCalibrationMinKey
    @abstract The minimum bounds for a calibrated value.  
*/
#define kIOHIDElementCalibrationMinKey              "CalibrationMin"

/*!
    @defined kIOHIDElementCalibrationMaxKey
    @abstract The maximum bounds for a calibrated value.  
*/
#define kIOHIDElementCalibrationMaxKey              "CalibrationMax"

/*!
    @defined kIOHIDElementCalibrationSaturationMinKey
    @abstract The mininum tolerance to be used when calibrating a logical element value. 
    @discussion The saturation property is used to allow for slight differences in the minimum and maximum value returned by an element. 
*/
#define kIOHIDElementCalibrationSaturationMinKey    "CalibrationSaturationMin"

/*!
    @defined kIOHIDElementCalibrationSaturationMaxKey
    @abstract The maximum tolerance to be used when calibrating a logical element value.  
    @discussion The saturation property is used to allow for slight differences in the minimum and maximum value returned by an element. 
*/
#define kIOHIDElementCalibrationSaturationMaxKey    "CalibrationSaturationMax"

/*!
    @defined kIOHIDElementCalibrationDeadZoneMinKey
    @abstract The minimum bounds near the midpoint of a logical value in which the value is ignored.  
    @discussion The dead zone property is used to allow for slight differences in the idle value returned by an element. 
*/
#define kIOHIDElementCalibrationDeadZoneMinKey      "CalibrationDeadZoneMin"

/*!
    @defined kIOHIDElementCalibrationDeadZoneMinKey
    @abstract The maximum bounds near the midpoint of a logical value in which the value is ignored.  
    @discussion The dead zone property is used to allow for slight differences in the idle value returned by an element. 
*/
#define kIOHIDElementCalibrationDeadZoneMaxKey      "CalibrationDeadZoneMax"

/*!
    @defined kIOHIDElementCalibrationGranularityKey
    @abstract The scale or level of detail returned in a calibrated element value.  
    @discussion Values are rounded off such that if granularity=0.1, values after calibration are 0, 0.1, 0.2, 0.3, etc.
*/
#define kIOHIDElementCalibrationGranularityKey      "CalibrationGranularity"

/*!
    @defined kIOHIDKeyboardSupportsEscKey
    @abstract Describe if keyboard device supports esc key.
    @discussion Keyboard devices having full HID keyboard descriptor can specify if esc key is actually supported or not. For new macs with TouchBar this is ideal scenario where keyboard descriptor by defaultspecifies presence of esc key but through given property client can check if key is present or not
 */
#define kIOHIDKeyboardSupportsEscKey                 "HIDKeyboardSupportsEscKey"

/*!
  @typedef IOHIDOptionsType
  @abstract Options for opening a device via IOHIDLib.
  @constant kIOHIDOptionsTypeNone Default option.
  @constant kIOHIDOptionsTypeSeizeDevice Used to open exclusive
    communication with the device.  This will prevent the system
    and other clients from receiving events from the device.
*/
enum {
    kIOHIDOptionsTypeNone     = 0x00,
    kIOHIDOptionsTypeSeizeDevice = 0x01
};
typedef uint32_t IOHIDOptionsType;


/*!
  @typedef IOHIDQueueOptionsType
  @abstract Options for creating a queue via IOHIDLib.
  @constant kIOHIDQueueOptionsTypeNone Default option.
  @constant kIOHIDQueueOptionsTypeEnqueueAll Force the IOHIDQueue
    to enqueue all events, relative or absolute, regardless of change.
*/
enum {
    kIOHIDQueueOptionsTypeNone     = 0x00,
    kIOHIDQueueOptionsTypeEnqueueAll = 0x01
};
typedef uint32_t IOHIDQueueOptionsType;

/*!
  @typedef IOHIDStandardType
  @abstract Type to define what industrial standard the device is referencing.
  @constant kIOHIDStandardTypeANSI ANSI.
  @constant kIOHIDStandardTypeISO ISO.
  @constant kIOHIDStandardTypeJIS JIS.
*/
enum {
    kIOHIDStandardTypeANSI                = 0,
    kIOHIDStandardTypeISO                 = 1,
    kIOHIDStandardTypeJIS                 = 2
};
typedef uint32_t IOHIDStandardType;

#define kIOHIDDigitizerGestureCharacterStateKey "DigitizerCharacterGestureState"

/* 
 * kIOHIDSystemButtonPressedDuringDarkBoot - Used to message that a wake button was pressed during dark boot
 */
#define kIOHIDSystemButtonPressedDuringDarkBoot     iokit_family_msg(sub_iokit_hidsystem, 7)

/*!
 @defined IOHIDKeyboard Keys
 @abstract Keys that represent parameters of keyboards.
 @discussion Legacy IOHIDKeyboard keys, formerly in IOHIDPrivateKeys. See IOHIDServiceKeys.h for the new keys.
 */
#define kIOHIDKeyboardCapsLockDelay         "CapsLockDelay"
#define kIOHIDKeyboardEjectDelay            "EjectDelay"

/*!
    @defined kFnFunctionUsageMapKey
    @abstract top row key remapping for consumer usages
    @discussion string of comma separated uint64_t value representing (usagePage<<32) | usage pairs
 
 */
#define kFnFunctionUsageMapKey      "FnFunctionUsageMap"

/*!
    @defined kFnKeyboardUsageMapKey
    @abstract top row key reampping for consumer usages
    @discussion string of comma separated uint64_t value representing (usagePage<<32) | usage pairs
 
 */
#define kFnKeyboardUsageMapKey      "FnKeyboardUsageMap"

#define kNumLockKeyboardUsageMapKey "NumLockKeyboardUsageMap"

#define kKeyboardUsageMapKey        "KeyboardUsageMap"

/*!
 @defined kIOHIDDeviceOpenedByEventSystemKey
 @abstract Property set when corresponding event service object opened by HID event system
 @discussion boolean value
 
 */
#define  kIOHIDDeviceOpenedByEventSystemKey "DeviceOpenedByEventSystem"

/*!
 * @define kIOHIDDeviceSuspendKey
 *
 * @abstract
 * Boolean property set on a user space IOHIDDeviceRef to suspend report delivery
 * to registered callbacks.
 *
 * @discussion
 * When set to true, the callbacks registered via the following API will not be invoked:
 *     IOHIDDeviceRegisterInputReportCallback
 *     IOHIDDeviceRegisterInputReportWithTimeStampCallback
 *     IOHIDDeviceRegisterInputValueCallback
 * To resume report delivery, this property should be set to false.
 */
#define kIOHIDDeviceSuspendKey              "IOHIDDeviceSuspend"



/*!
    @defined    kIOHIDSensorPropertyReportIntervalKey
    @abstract   Property to get or set the Report Interval in us of supported sensor devices
    @discussion Corresponds to kHIDUsage_Snsr_Property_ReportInterval in a sensor device's
                descriptor.
 */
#define kIOHIDSensorPropertyReportIntervalKey   kIOHIDReportIntervalKey

/*!
    @defined    kIOHIDSensorPropertySampleIntervalKey
    @abstract   Property to get or set the Sample Interval in us of supported sensor devices
    @discussion Corresponds to kHIDUsage_Snsr_Property_SamplingRate in a sensor device's
                descriptor.
 */
#define kIOHIDSensorPropertySampleIntervalKey   kIOHIDSampleIntervalKey

/*!
    @defined    kIOHIDSensorPropertyBatchIntervalKey
    @abstract   Property to get or set the Batch Interval / Report Latency in us of supported sensor devices
    @discussion Corresponds to kHIDUsage_Snsr_Property_ReportLatency in a sensor device's
                descriptor.
 */
#define kIOHIDSensorPropertyBatchIntervalKey    kIOHIDBatchIntervalKey

/*!
    @defined    kIOHIDSensorPropertyReportLatencyKey
    @abstract   Alias of kIOHIDSensorPropertyBatchIntervalKey
 */
#define kIOHIDSensorPropertyReportLatencyKey    kIOHIDSensorPropertyBatchIntervalKey

/*!
    @defined    kIOHIDSensorPropertyMaxFIFOEventsKey
    @abstract   Property to get or set the maximum FIFO event queue size of supported sensor devices
    @discussion Corresponds to kHIDUsage_Snsr_Property_MaxFIFOEvents in a sensor device's
                descriptor.
 */
#define kIOHIDSensorPropertyMaxFIFOEventsKey    "MaxFIFOEvents"

__END_DECLS

#endif /* !_IOKIT_HID_IOHIDKEYS_H_ */
