/*
 * Copyright (c) 2002-2014 Apple Computer, Inc. All rights reserved.
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

#ifndef _IOPSKEYSPRIVATE_H_
#define _IOPSKEYSPRIVATE_H_

/*!
 * @define      kAppleRawCurrentCapacityKey
 * @abstract    CFDictionary key for the current power source's raw capacity, unaltered by any smoothing algorithms.
 *
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key in units of percent or mAh.
 *              <li> The power source's software may specify the units for this key.
 *                   The units must be consistent for all capacities reported by this power source.
 *                   The power source will usually define this number in units of percent, or mAh.
 *              <li> Clients may derive a raw percentage of power source battery remaining by dividing "AppleRawCurrentCapacity" by "Max Capacity"
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */
#ifndef kAppleRawCurrentCapacityKey
#define kAppleRawCurrentCapacityKey "AppleRawCurrentCapacity"
#endif

/*
 * kIOPSVendorIDSourceKey holds CFNumberRef data. Used to differentiate
 * between various vendor id sources.
 */
#define kIOPSVendorIDSourceKey              "Vendor ID Source"

/* Internal values for kIOPSTypeKey */
#define kIOPSAccessoryType                  "Accessory Source"


/* kIOPSRawExternalConnectivityKey specifies if device is receiving power from
 * an external power source. In some cases, kIOPSPowerSourceStateKey may not
 * show the external power source, if that external source is a battery
 */
#define kIOPSRawExternalConnectivityKey     "Raw External Connected"

/* kIOPSShowChargingUIKey specifies whether the UI should treat the device
 * as charging or not.
 */
#define kIOPSShowChargingUIKey     "Show Charging UI"

/* kIOPSPlayChargingChimeKey specifies whether the attached charger is meant
 * to cause UI to make an audible chime in addition to showing the charging
 * UI, as indicated by kIOPSShowChargingUIKey.
 */
#define kIOPSPlayChargingChimeKey     "Play Charging Chime"


/*
 * kIOPSAccessoryIdentifierKey -
 * Accessory Identifier key. This key holds identifier key for each accessory power source.
 * This could be different for each part of an accessory with multiple parts.
 *
 * Holds a CFStringRef
 */
#define kIOPSAccessoryIdentifierKey         "Accessory Identifier"

/*
 * kIOPSGroupIdentifierKey -
 *
 * Accessories with multiple parts report each part as a separate power source to powerd.
 * These individual parts of the accessory will have the same 'Group Identifier'.
 *
 * Holds a CFStringRef
 */
#define kIOPSGroupIdentifierKey    "Group Identifier"

/*
 * kIOPSPartNameKey -
 * User friendly name for each part of an accessory.
 *
 * Accessories with multiple parts report each part as a separate power source to powerd.
 * 'kIOPSPartNameKey' will hold a user friendly name for each of these parts.
 * 'kIOPSNameKey' will be common for all the parts of the accessory
 *
 * Holds a CFStringRef
 */

#define kIOPSPartNameKey           "Part Name"

/*
 * kIOPSPartIdentifierKey -
 * Identity for each part of an accessory.
 *
 * Accessories with multiple parts report each part as a separate power source to powerd.
 * Unlike kIOPSAccessoryIdentifierKey, which carries identification numbers like UUID, serial numbers etc.,
 * kIOPSPartIdentifierKey is used to hold the identity of the part in relation to other parts of the
 * accessory.
 *
 * Holds a CFStringRef, with the possible values defined below.
 */

#define kIOPSPartIdentifierKey           "Part Identifier"

/*
 * Possible values for Part Identifier(kIOPSPartIdentifierKey)
 */
#define kIOPSPartIdentifierLeft         "Left"
#define kIOPSPartIdentifierRight        "Right"
#define kIOPSPartIdentifierCase         "Case"
#define kIOPSPartIdentifierSingle       "Single"
#define kIOPSPartIdentifierOther        "Other"

/* Internal transport types */
#define kIOPSAIDTransportType                   "AID"
#define kIOPSTransportTypeBluetooth             "Bluetooth"
#define kIOPSTransportTypeBluetoothLowEnergy    "Bluetooth LE"
#define kIOPSTransportTypeInductiveInBandComms  "Inductive In-Band"

/*
 * Invalid ProductId & VendorId values are used in cases when there are no
 * product/vendor ids assigned. In such cases, kIOPSNameKey can be used to
 * identify the power source.
 */
#define kIOPSInvalidProductID                   0xffff
#define kIOPSInvalidVendorID                    0xffff

/*
 * kIOPSAccessoryCategoryKey -
 * Classifies the accessory in to one of the pre-defined categories
 *
 * Holds a CFStringRef, with the possible values defined below.
 */
#define kIOPSAccessoryCategoryKey       "Accessory Category"

/*
 * Possible Categories of accessories(kIOPSAccessoryCategoryKey)
 */
#define kIOPSAccessoryCategoryAudioBatteryCase   "Audio Battery Case"
#define kIOPSAccessoryCategoryGameController     "Game Controller"
#define kIOPSAccessoryCategorySpeaker       "Speaker"
#define kIOPSAccessoryCategoryHeadphone     "Headphone"
#define kIOPSAccessoryCategoryHeadset       "Headset"
#define kIOPSAccessoryCategoryWatch         "Watch"
#define kIOPSAccessoryCategoryBatteryCase   "Battery Case"
#define kIOPSAccessoryCategoryKeyboard      "Keyboard"
#define kIOPSAccessoryCategoryTrackpad      "Trackpad"
#define kIOPSAccessoryCategoryMouse         "Mouse"
#define kIOPSAccessoryCategoryPencil        "Pencil"
#define kIOPSAccessoryCategoryUnknown       "Unknown"

/*
 * Power adapter related internal keys
 */

/*!
 * @define      kIOPSPowerAdapterSerialStringKey
 *
 * @abstract    The power adapter's serial string.
 *              The value associated with this key is a CFString value
 *
 * @discussion  This key may be present in the dictionary returned from
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterSerialStringKey    "SerialString"

/*!
 * @define      kIOPSPowerAdapterNameKey
 *
 * @abstract    The power adapter's name.
 *              The value associated with this key is a CFString value
 *
 * @discussion  This key may be present in the dictionary returned from
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */

#define kIOPSPowerAdapterNameKey            "Name"

/*!
 * @define      kIOPSPowerAdapterNameKey
 *
 * @abstract    The power adapter's manufacturer's id.
 *              The value associated with this key is a CFNumber kCFNumberIntType integer value
 *
 * @discussion  This key may be present in the dictionary returned from
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterManufacturerIDKey  "Manufacturer"

/*!
 * @define      kIOPSPowerAdapterHardwareVersionKey
 *
 * @abstract    The power adapter's hardware version.
 *              The value associated with this key is a CFString value
 *
 * @discussion  This key may be present in the dictionary returned from
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterHardwareVersionKey       "HwVersion"

/*!
 * @define      kIOPSPowerAdapterFirmwareVersionKey
 *
 * @abstract    The power adapter's firmware version.
 *              The value associated with this key is a CFString value
 *
 * @discussion  This key may be present in the dictionary returned from
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterFirmwareVersionKey       "FwVersion"

/*!
 * @define      kIOPSPowerAdapterVoltageKey
 *
 * @abstract    This key refers to the voltage of the external AC power adapter attached to a portable.
 *              The value associated with this key is a CFNumberRef kCFNumberIntType integer value, in units of mVolts.
 *
 * @discussion  This key may be present in the dictionary returned from
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterVoltageKey                 "Voltage"

/*!
 * @define      kIOPSPowerAdapterDescriptionKey
 *
 * @abstract    This key refers to provides a description of the external AC power adapter attached to a portable.
 *              The value associated with this key is a CFString value.
 *
 * @discussion  This key may be present in the dictionary returned from
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterDescriptionKey             "Description"


/*
 * Battery case related internal keys
 */

/*!
 * @define      kIOPSAppleBatteryCaseCumulativeCurrentKey
 * @abstract    CFDictionary key for a battery case's cumulative current flow
 *              through its battery since its last reset.
 *
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources may publish this key in units amp-seconds.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */
#define kIOPSAppleBatteryCaseCumulativeCurrentKey "Battery Case Cumulative Current"

/*!
 * @define      kAppleBatteryCaseAvailableCurrentKey
 * @abstract    CFDictionary key for a battery case's available current for host.
 *
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key in units mA.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */
#define kIOPSAppleBatteryCaseAvailableCurrentKey "Battery Case Available Current"

/*!
 * @define      kAppleBatteryCaseChargingVoltageKey
 * @abstract    CFDictionary key for a the voltage a battery case is charging from.
 *
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key in units mV.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */
#define kAppleBatteryCaseChargingVoltageKey "Battery Case Charging Voltage"

/*!
 * @define      kIOPSAppleBatteryCaseChemIDKey
 * @abstract    CFDictionary key for a battery case's battery's Chem ID.
 *
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources may publish this key.
 *              <li> Type CFNumber kCFNumberIntType (integer)
 *              <li> Note that this value does not have any physical unit.
 *              </ul>
 */
#define kIOPSAppleBatteryCaseChemIDKey "Battery Case Chem ID"

/*!
 * @define      kIOPSAppleBatteryCaseAddress
 *
 * @abstract    CFDictionary key for a battery case's advertised resolvable public address,
 *              as assigned by the last device it was attached to.
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources may publish this key.
 *              <li> Type CFData
 *              <li> A 6-byte address wrapped in an CFData object.
 *              </ul>
 */
#define kIOPSAppleBatteryCaseAddress "Address"

/*!
 * @define      kIOPSAppleBatteryCaseIncomingVoltage
 *
 * @abstract    CFDictionary key for a battery case's incoming voltage
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources may publish this key in units mV.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */
#define kIOPSAppleBatteryCaseIncomingVoltage "Incoming Voltage"

/*!
 * @define      kIOPSAppleBatteryCaseIncomingCurrent
 *
 * @abstract    CFDictionary key for a battery case's incoming current
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources may publish this key in units mA.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */
#define kIOPSAppleBatteryCaseIncomingCurrent "Incoming Current"

/*!
 * @define      kIOPSAppleBatteryCaseCell0Voltage
 *
 * @abstract    CFDictionary key for a battery case's first cell's voltage
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources may publish this key in units mV.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */
#define kIOPSAppleBatteryCaseCell0Voltage "Cell 0 Voltage"

/*!
 * @define      kIOPSAppleBatteryCaseCell1Voltage
 *
 * @abstract    CFDictionary key for a battery case's second cell's voltage
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources may publish this key in units mV.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */
#define kIOPSAppleBatteryCaseCell1Voltage "Cell 1 Voltage"


/*
 * Battery case related internal commands
 */

/*!
 * @define      kIOPSAppleBatteryCaseCommandSetCurrentLimitBackOffKey
 *
 * @abstract    Tell the battery case of a PMU imposed back off in current limit.
 * @discussion
 *              <ul>
 *              <li> The matching argument should be a CFNumber of kCFNumberIntType
 *              <li> specifying the amount the PMU has reduced incoming current limit in mA.
 *              </ul>
 */
#define kIOPSAppleBatteryCaseCommandSetCurrentLimitBackOffKey "Current Limit Back Off"

/*!
 * @define      kIOPSCommandSendAverageChargingCurrent
 *
 * @abstract Tell the UPS the device's average current drawn from the charger
 * @discussion
 *              <ul>
 *                  <li>The matching argument should be a CFNumber of kCFNumberIntType specifying the device's
 *                  <li>average current draw from the charger in mA.
 */
#define kIOPSCommandSendAverageChargingCurrent       "Send Average Charging Current"

/*!
 * @define      kIOPSAppleBatteryCaseCommandEnableChargingKey
 *
 * @abstract    Tell the battery case of whether it should enable its boost to enable charging.
 * @discussion
 *              <ul>
 *              <li>The matching argument should be a CFBooleanRef where kCFBooleanTrue enables charging and
 *              <li>kCFBooleanFalse diables it.
 *              </ul>
 */
#define kIOPSAppleBatteryCaseCommandEnableChargingKey "Enable Charging"

/*!
 * @define      kIOPSAppleBatteryCaseCommandSetAddress
 *
 * @abstract    Tell the battery case what resolvable public address it should advertise.
 * @discussion
 *              <ul>
 *              <li>The matching argument should be a CFString of a 6-byte address in the format "XX:XX:XX:XX:XX:XX".
 *              </ul>
 */
#define kIOPSAppleBatteryCaseCommandSetAddress "Set Address"


/*
 * Debug Information Dictionary/Keys
 */

/*!
 * @define      kIOPSDebugInformationKey
 *
 * @abstract    Dictionary containing CFString keys mapped to 32-bit CFNumber values.
 * @discussion  Key/Value pairs are device specific. Some keys are defined here, but no gurantees are made
 *              in regards to the formatting of their values.
 *              <ul>
 *              <li> Type CFDictionaryRef.
 *              </ul>
 */
#define kIOPSDebugInformationKey "Debug Information"

// Keys for use in the kIOPSDebugInformationKey dictionary
#define kIOPSDebugInformation_PowerStatusKey       "PowerStatus"
#define kIOPSDebugInformation_ChargingStatusKey    "ChargingStatus"
#define kIOPSDebugInformation_InductiveStatusKey   "InductiveStatus"



/*!
 * @define      kIOPSDeviceColor
 *
 * @abstract    CFDictionary key for a power source's physical device color.
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources may publish this key.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */
#define kIOPSDeviceColor "Device Color"

/*!
 * @define      kIOPSLEDsKey
 * @abstract    An array describing the state of each LED associated with the power source.
 * @discussion  Power sources that feature LEDs can publish the state of their LEDs. Each
 *              LED is described through an CFDictionary.
 *              <ul>
 *              <li> Apple-defined power sources will publish this key if the hardware features LEDs.
 *              <li> Type CFArrayRef
 *              </ul>
 */
#define kIOPSLEDsKey            "LEDs"

/*!
 * @group       LED Description
 * @abstract    Possible keys for <code>@link kIOPSLEDsKey @/link</code>
 */
/*!
 * @define      kIOPSLedStateKey
 * @abstract    Describtion of the state of the LED.
 * @discussion  <ul>
 *              <li> Type CFStringRef
 *              </ul>
 */
#define kIOPSLedStateKey       "State"

/*!
 * @define      kIOPSLedColorKey
 * @abstract    Description of the color of the LED.
 * @discussion  <ul>
 *              <li> Type CFStringRef
 *              </ul>
 */
#define kIOPSLedColorKey       "Color"

/*!
 * @group       LED States
 * @abstract    Possible keys for <code>@link kIOPSLedStateKey @/link</code>
 */
/*!
 * @define      kIOPSLedStateSolid
 * @abstract    Potential value for key <code>@link kIOPSLeddStateKey@/link</code>
 */
#define kIOPSLedStateSolid    "Solid"

/*!
 * @define      kIOPSLedStateBlinking
 * @abstract    Potential value for key <code>@link kIOPSLeddStateKey@/link</code>
 */
#define kIOPSLedStateBlinking "Blinking"

/*!
 * @define      kIOPSLedStateOff
 * @abstract    Potential value for key <code>@link kIOPSLeddStateKey@/link</code>
 */
#define kIOPSLedStateOff      "Off"

/*!
 * @group       LED Colors
 * @abstract    Possible keys for <code>@link kIOPSLedColorKey @/link</code>
 */
/*!
 * @define      kIOPSLedColorWhite
 * @abstract    Potential value for key <code>@link kIOPSLeddColorKey@/link</code>
 */
#define kIOPSLedColorWhite    "White"

/*!
 * @define      kIOPSLedColorGreen
 * @abstract    Potential value for key <code>@link kIOPSLeddColorKey@/link</code>
 */
#define kIOPSLedColorGreen    "Green"

/*!
 * @define      kIOPSLedColorOrange
 * @abstract    Potential value for key <code>@link kIOPSLeddColorKey@/link</code>
 */
#define kIOPSLedColorOrange   "Orange"

#endif /* defined(_IOPSKEYSPRIVATE_H_) */
