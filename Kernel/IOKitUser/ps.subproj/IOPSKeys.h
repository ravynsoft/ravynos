/*
 * Copyright (c) 2002-2010 Apple Computer, Inc. All rights reserved.
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

/*!
 *  @header     IOPSKeys.h
 *  
 *  @discussion
 *              IOPSKeys.h defines C strings for use accessing power source data in IOPowerSource
 *              CFDictionaries, as returned by <code>@link //apple_ref/c/func/IOPSGetPowerSourceDescription IOPSGetPowerSourceDescription @/link</code>
 *              Note that all of these C strings must be converted to CFStrings before use. You can wrap
 *              them with the CFSTR() macro, or create a CFStringRef (that you must later CFRelease()) using CFStringCreateWithCString().
 */  

#ifndef _IOPSKEYS_H_
#define _IOPSKEYS_H_

/*!
 * @group       IOPSPowerAdapter Keys
 * 
 * @discussion
 *              Use these kIOPSPowerAdapter keys to decipher the dictionary returned
 *              by @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 */
 
/*!
 * @define      kIOPSPowerAdapterIDKey
 *
 * @abstract    This key refers to the attached external AC power adapter's ID.
 *              The value associated with this key is a CFNumberRef kCFNumberIntType integer.
 *
 * @discussion  This key may be present in the dictionary returned from 
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterIDKey          "AdapterID"

/*!
 * @define      kIOPSPowerAdapterWattsKey
 *
 * @abstract    This key refers to the wattage of the external AC power adapter attached to a portable.
 *              The value associated with this key is a CFNumberRef kCFNumberIntType integer value, in units of watts.
 *
 * @discussion  This key may be present in the dictionary returned from 
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterWattsKey       "Watts"

/*!
 * @define      kIOPSPowerAdapterRevisionKey
 *
 * @abstract    The power adapter's revision.
 *              The value associated with this key is a CFNumberRef kCFNumberIntType integer value
 *
 * @discussion  This key may be present in the dictionary returned from 
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterRevisionKey   "AdapterRevision"

/*!
 * @define      kIOPSPowerAdapterSerialNumberKey
 *
 * @abstract    The power adapter's serial number.
 *              The value associated with this key is a CFNumberRef kCFNumberIntType integer value
 *
 * @discussion  This key may be present in the dictionary returned from 
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */ 
#define kIOPSPowerAdapterSerialNumberKey    "SerialNumber"

/*!
 * @define      kIOPSPowerAdapterFamilyKey
 *
 * @abstract    The power adapter's family code.
 *              The value associated with this key is a CFNumberRef kCFNumberIntType integer value
 *
 * @discussion  This key may be present in the dictionary returned from 
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterFamilyKey          "FamilyCode"

/*!
 * @define      kIOPSPowerAdapterCurrentKey     
 *
 * @abstract    This key refers to the current of the external AC power adapter attached to a portable. 
 *              The value associated with this key is a CFNumberRef kCFNumberIntType integer value, in units of mAmps.
 *
 * @discussion  This key may be present in the dictionary returned from 
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterCurrentKey         "Current"

/*!
 * @define      kIOPSPowerAdapterSourceKey     
 *
 * @abstract    This key refers to the source of the power.
 *              The value associated with this key is a CFNumberRef kCFNumberIntType integer value.
 *
 * @discussion  This key may be present in the dictionary returned from 
 *              @link //apple_ref/c/func/IOPSCopyExternalPowerAdapterDetails IOPSCopyExternalPowerAdapterDetails @/link
 *              This key might not be defined in the adapter details dictionary.
 */
#define kIOPSPowerAdapterSourceKey          "Source"


/*! 
 * @group       Internal Keys
 * 
 */

/*!
 * @define      kIOPSUPSManagementClaimed
 * 
 * @abstract    Claims UPS management for a third-party driver.
 * @discussion  kIOPSUPSManagementClaimed is obsolete. Do not use. 
 * @deprecated  Unsupported in OS X 10.5 and later.
 */
#define kIOPSUPSManagementClaimed       "/IOKit/UPSPowerManagementClaimed"

/*!
 * @define      kIOPSLowWarnLevelKey 
 * @abstract    Specifies the battery capacity percentage at which device is considered to be low on power.
 *              Typically used to show initial warning to the user.
 *
 *              Holds a CFNumber, with possible values between 0-100.
*/
#define kIOPSLowWarnLevelKey           "Low Warn Level"

/*!
 * @define      kIOPSDeadWarnLevelKey 
 * @abstract    Specifies the battery capacity percentage at which device is considered to be very low on power and
 *              soon will not be functional. Typically used to show final warning to the user.
 *
 *              Holds a CFNumber, with possible values between 0-100.
 *
 */
#define kIOPSDeadWarnLevelKey          "Shutdown Level"


/*!
 * @define      kIOPSDynamicStorePath
 *
 * @abstract    This is only used for internal bookkeeping, and should be ignored.
 */
#define kIOPSDynamicStorePath          "/IOKit/PowerSources"


/*!
 * @group       Power Source Commands (UPS)
 *
 */ 

/*!
 * @define      kIOPSCommandDelayedRemovePowerKey
 *
 * @abstract    Command to give a UPS when it should remove power from its AC plugs in a specified amount of time
 * @discussion
 *              <ul>
 *                  <li>The matching argument should be a CFNumber of kCFNumberIntType specifying when the UPS should
 *                  <li>remove power from its AC power ports.
 *              </ul>
 */
#define kIOPSCommandDelayedRemovePowerKey     "Delayed Remove Power"

/*!
 * @define      kIOPSCommandEnableAudibleAlarmKey
 *
 * @abstract    Command to give a UPS when it should either enable or disable the audible alarm.
 * @discussion
 *              <ul>
 *                  <li>The matching argument should be a CFBooleanRef where kCFBooleanTrue enables the alarm and 
 *                  <li>kCFBooleanFalse diables the alarm
 *              </ul>
 */

#define kIOPSCommandEnableAudibleAlarmKey     "Enable Audible Alarm"

/*!
 * @define      kIOPSCommandStartupDelayKey
 *
 * @abstract Tell UPS how long it should wait for 
 * @discussion
 *              <ul>
 *                  <li>The matching argument should be a CFNumber of kCFNumberIntType specifying when the UPS should
 *                  <li>remove power from its AC power ports.
 *              </ul>
 */
#define kIOPSCommandStartupDelayKey           "Startup Delay"

/*!
 * @define      kIOPSCommandSetCurrentLimitKey
 *
 * @abstract Tell the UPS the max current it can draw from an attached power source
 * @discussion
 *              <ul>
 *                  <li>The matching argument should be a CFNumber of kCFNumberIntType specifying the UPS' new current
 *                  <li>limit in mA.
 */
#define kIOPSCommandSetCurrentLimitKey        "Set Current Limit"

/*!
 * @define      kIOPSCommandSetRequiredVoltageKey
 *
 * @abstract Tell the UPS the minimum voltage it needs to provide.
 * @discussion
 *              <ul>
 *                  <li>The matching argument should be a CFNumber of kCFNumberIntType specifying the required voltage
 *                  <li>from a UPS in mV.
 */
#define kIOPSCommandSetRequiredVoltageKey       "Set Required Voltage"

/*!
 * @define      kIOPSCommandSendCurrentStateOfCharge
 *
 * @abstract Tell the UPS the device's current state of charge
 * @discussion
 *              <ul>
 *                  <li>The matching argument should be a CFNumber of kCFNumberIntType specifying the device's
 *                  <li>state of charge as a percentage.
 */
#define kIOPSCommandSendCurrentStateOfCharge       "Send Current State of Charge"

/*!
 * @define      kIOPSCommandSendCurrentTemperature
 *
 * @abstract Tell the UPS the device's current temperature
 * @discussion
 *              <ul>
 *                  <li>The matching argument should be a CFNumber of kCFNumberIntType specifying the device's
 *                  <li>temperature in degrees deciKelvin.
 */
#define kIOPSCommandSendCurrentTemperature       "Send Current Temperature"

/*!
 * @group       Power Source data keys
 *
 */

/*              These keys specify the values in a dictionary of PowerSource details.
 *              Use these keys in conjunction with the dictionary returned by 
 *              <code>@link //apple_ref/c/func/IOPSGetPowerSourceDescription IOPSGetPowerSourceDescription @/link</code>
 * 
 *              Clients of <code>@link //apple_ref/c/func/IOPSCreatePowerSource IOPSCreatePowerSource @/link</code>
 *              must specify these keys in their power source dictionaries.
 *              Each key is labelled with one of these labels that indicate what information is REQUIRED. to
 *              represent a power source in OS X.
 *
 *              <ul>
 *                  <li> For power source creators: Providing this key is REQUIRED.
 *                  <li> For power source creators: Providing this key is RECOMMENDED.
 *                  <li> For power source creators: Providing this key is OPTIONAL.
 *                  <li> This key is DEPRECATED, do not provide it. 
 *              </ul>
 *
 */ 

/*!
 * @define      kIOPSPowerSourceIDKey
 *
 * @abstract    CFNumber key uniquely identifying the power source.
 *
 * @discussion
 *              <ul>
 *              <li> Callers should not set this key; OS X power management will publish this key for all power sources
 *              <li> Type CFNumber, kCFNumberIntType, uniquely identifying the power source
 *              </ul>
 */
 
#define kIOPSPowerSourceIDKey          "Power Source ID"

 
/*!
 * @define      kIOPSPowerSourceStateKey
 *
 * @abstract    CFDictionary key for the current source of power.
 *
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For power source creators: Providing this key is REQUIRED.
 *              <li> <code>@link kIOPSBatteryPowerValue @/link</code> indicates power source is drawing internal power; 
 *                   <code>@link kIOPSACPowerValue@/link</code> indicates power source is connected to an external power source.
 *              <li> Type CFString, value is <code>@link kIOPSACPowerValue@/link</code>, <code>@link kIOPSBatteryPowerValue@/link</code>, or <code>@link kIOPSOffLineValue@/link</code>.
 *              </ul>
 */

#define kIOPSPowerSourceStateKey       "Power Source State"

/*!
 * @define      kIOPSCurrentCapacityKey
 * @abstract    CFDictionary key for the current power source's capacity.
 * 
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key in units of percent.
 *              <li> The power source's software may specify the units for this key. 
 *                   The units must be consistent for all capacities reported by this power source.
 *                   The power source will usually define this number in units of percent, or mAh.
 *              <li> Clients may derive a percentage of power source battery remaining by dividing "Current Capacity" by "Max Capacity"
 *              <li> For power source creators: Providing this key is REQUIRED.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */

#define kIOPSCurrentCapacityKey        "Current Capacity"

/*!
 * @define      kIOPSMaxCapacityKey
 * @abstract    CFDictionary key for the current power source's maximum or "Full Charge Capacity"
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key in units of percent. The value is usually 100%.
 *              <li> The power source's software may specify the units for this key. The units must be consistent for all capacities reported by this power source.
 *              <li> For power source creators: Providing this key is REQUIRED.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */

#define kIOPSMaxCapacityKey            "Max Capacity"

/*!
 * @define      kIOPSDesignCapacityKey
 * @abstract    CFDictionary key for the current power source's design capacity
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources might not publish this key.
 *              <li> The power source's software may specify the units for this key. The units must be consistent for all capacities reported by this power source.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */

#define kIOPSDesignCapacityKey          "DesignCapacity"

/*!
 * @define      kIOPSNominalCapacityKey
 * @abstract    CFDictionary key for the current power source's normalized full, or "nominal", charge capacity
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources might not publish this key.
 *              <li> The power source's software may specify the units for this key. The units must be consistent for all capacities reported by this power source.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFNumber kCFNumberIntType (signed integer)
 *              </ul>
 */

#define kIOPSNominalCapacityKey         "Nominal Capacity"

/*!
 * @define      kIOPSTimeToEmptyKey
 * @abstract    CFDictionary key for the current power source's time remaining until empty.
 * @discussion
 *              Only valid if the power source is running off its own power. That's when the 
 *              <code>@link kIOPSPowerSourceStateKey @/link</code> has value <code>@link kIOPSBatteryPowerValue @/link</code> 
 *              and the value of <code>@link kIOPSIsChargingKey @/link</code> is kCFBooleanFalse.
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFNumber kCFNumberIntType (signed integer), units are minutes
 *              <li> A value of -1 indicates "Still Calculating the Time", otherwise estimated minutes left on the battery.
 *              </ul>
 */


#define kIOPSTimeToEmptyKey            "Time to Empty"

/*!
 * @define      kIOPSTimeToFullChargeKey
 * @abstract    CFDictionary key for the current power source's time remaining until empty.
 * @discussion
 *              Only valid if the value of <code>@link kIOPSIsChargingKey @/link</code> is kCFBooleanTrue.
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFNumber kCFNumberIntType (signed integer), units are minutes
 *              <li>A value of -1 indicates "Still Calculating the Time", otherwise estimated minutes until fully charged.
 *              </ul>
 */

#define kIOPSTimeToFullChargeKey       "Time to Full Charge"

/*!
 * @define      kIOPSIsChargingKey
 * @abstract    CFDictionary key for the current power source's charging state
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For power source creators: Providing this key is REQUIRED.
 *              <li> Type CFBoolean - kCFBooleanTrue or kCFBooleanFalse
 *              </ul>
 */

#define kIOPSIsChargingKey             "Is Charging"

/*!
 * @define      kIOPSInternalFailureKey
 * @abstract    CFDictionary key for whether the current power source has experienced a failure
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources might not publish this key.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFBoolean - kCFBooleanTrue or kCFBooleanFalse
 *              </ul>
 */

#define kIOPSInternalFailureKey        "Internal Failure"

/*!
 * @define      kIOPSIsPresentKey
 * @abstract    CFDictionary key for the current power source's presence. 
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For instance, a portable with the capacity for two batteries but 
 *                  with only one present would show two power source dictionaries, 
 *                  but kIOPSIsPresentKey would have the value kCFBooleanFalse in one of them.
 *              <li> For power source creators: Providing this key is REQUIRED.
 *              <li> Type CFBoolean - kCFBooleanTrue or kCFBooleanFalse
 *              </ul>
 */

#define kIOPSIsPresentKey              "Is Present"

/*!
 * @define      kIOPSVoltageKey
 * @abstract    CFDictionary key for the current power source's electrical voltage.
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFNumber kCFNumberIntType (signed integer) - units are mV
 *              </ul>
 */

#define kIOPSVoltageKey                "Voltage"

/*!
 * @define      kIOPSCurrentKey
 * @abstract    CFDictionary key for the current power source's electrical current.
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFNumber kCFNumberIntType (signed integer) - units are mA
 *              </ul>
 */

#define kIOPSCurrentKey                "Current"

/*!
 * @define      kIOPSTemperatureKey
 * @abstract    CFDictionary key for the current power source's temperature.
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFNumber kCFNumberIntType (signed integer) - units are C
 *              </ul>
 */
#define kIOPSTemperatureKey                         "Temperature"

/*!
 * @define      kIOPSNameKey
 * @abstract    CFDictionary key for the current power source's name.
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For power source creators: Providing this key is REQUIRED.
 *              <li> Type CFStringRef
 *              </ul>
 */

#define kIOPSNameKey                   "Name"


/*!
 * @define      kIOPSTypeKey
 * @abstract    CFDictionary key for the type of the power source
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For power source creators: Providing this key is REQUIRED.
 *              <li> Type CFStringRef. Valid transport types are kIOPSUPSType or kIOPSInternalBatteryType.
 *              </ul>
 */

#define kIOPSTypeKey          "Type"


/*!
 * @define      kIOPSTransportTypeKey
 * @abstract    CFDictionary key for the current power source's data transport type (e.g. the means that the power source conveys power source data to the OS X machine). 
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> A value of <code>@link kIOPSInternalType @/link</code> describes an internal power source.
 *              <li> <code>@link kIOPSUSBTransportType @/link</code>, <code>@link kIOPSNetworkTransportType @/link</code>, and <code>@link kIOPSSerialTransportType @/link</code> usually describe UPS's.
 *              <li> For power source creators: Providing this key is REQUIRED.
 *              <li> Type CFStringRef. Valid transport types are kIOPSSerialTransportType, 
 *                  kIOPSUSBTransportType, kIOPSNetworkTransportType, kIOPSInternalType
 *              </ul>
 */

#define kIOPSTransportTypeKey          "Transport Type"

/*!
 * @define      kIOPSVendorIDKey
 * @abstract    CFDictionary key for the current power source's vendor ID.
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFNumberRef
 *              </ul>
 */

#define kIOPSVendorIDKey          "Vendor ID"

/*!
 * @define      kIOPSProductIDKey
 * @abstract    CFDictionary key for the current power source's product ID.
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFNumberRef
 *              </ul>
 */

#define kIOPSProductIDKey          "Product ID"

/*!
 * @define      kIOPSVendorDataKey
 * @abstract    CFDictionary key for arbitrary vendor data.
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources are not required to publish this key.
 *              <li> For power source creators: Providing this key is OPTIONAL.
 *              <li>CFDictionary; contents determined by the power source software. OS X will not look at this data.
 *              </ul>
 */

#define kIOPSVendorDataKey          "Vendor Specific Data"

/*!
 * @define      kIOPSBatteryHealthKey
 * @abstract    CFDictionary key for the current power source's "health" estimate.
 * @discussion
 *              <ul>
 *              <li> Apple-defined battery power sources will publish this key.
 *              <li> Use value <code>@link kIOPSGoodValue @/link</code> to describe a well-performing power source, 
 *              <li> Use <code>@link kIOPSFairValue @/link</code> to describe a functional power source with limited capacity
 *              <li> And use  <code>@link kIOPSPoorValue @/link</code> to describe a power source that's not capable of Providing power.
 *              <li> For power source creators: Providing this key is OPTIONAL.
 *              <li> Type CFStringRef
 *              </ul>
 */

#define kIOPSBatteryHealthKey       "BatteryHealth"

/*!
 * @define      kIOPSBatteryHealthConditionKey
 * @abstract    kIOPSBatteryHealthConditionKey broadly describes the battery's health.
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will publish this key.
 *              <li> Value is one of the "Battery Health Condition Values" strings described in this file.
 *              <li> For power source creators: Providing this key is OPTIONAL - these keys have values only used by Apple power sources.
 *              <li> Type CFStringRef
 *              </ul>
 */

#define kIOPSBatteryHealthConditionKey       "BatteryHealthCondition"

/*!
 * @define      kIOPSBatteryFailureModesKey
 * @abstract    Enumerates a battery's failures and error conditions. 
 * @discussion 
 *              Various battery failures will be listed here. A battery may suffer from more than one 
 *              type of failure simultaneously, so this key has a CFArray value.
 *
 *              If BatteryFailureModesKey is not defined (or is set to an empty dictionary), 
 *                  then the battery has no detectable failures.
 *
 *              Each entry in the array should be a short descriptive string describing the error.
 *              <li> Apple-defined power sources will publish this key if any battery errors exist.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFArrayRef
 *              </ul>
 */
#define kIOPSBatteryFailureModesKey          "BatteryFailureModes"

/*!
 * @define      kIOPSHealthConfidenceKey    
 * @abstract    CFDictionary key for our confidence in the accuracy of our 
 *              power source's "health" estimate. 
 * @deprecated  In OS X 10.6 and later.
 * @discussion
 *              <ul>
 *              <li> Apple-defined power sources will no longer publish this key.
 *              <li> Power source creators should not publish this key.
 *              <li> For power source creators: This key is DEPRECATED, do not implement it. 
 *              <li> Type CFStringRef
 *              </ul>
 */

#define kIOPSHealthConfidenceKey    "HealthConfidence"


/*!
 * @define      kIOPSMaxErrKey
 * @abstract    CFDictionary key for the current power source's percentage error in capacity reporting. 
 * @discussion
 *              In internal batteries, this refers to the battery pack's estimated percentage error.
 *              <ul>
 *              <li> Apple-defined battery power sources will publish this key, but only if it's defined for the battery.
 *              <li> For power source creators: Providing this key is OPTIONAL.
 *              <li> Type CFNumberRef kCFNumberIntType, non-negative integer
 *              </ul>
 */

#define kIOPSMaxErrKey              "MaxErr"

/*!
 * @define      kIOPSIsChargedKey
 * @abstract    CFDictionary key indicates whether the battery is charged. 
 * @discussion
 *              A battery must be plugged in to an external power source in order to be fully charged.
 *              Note that a battery may validly be plugged in, not charging, and <100% charge.
 *              e.g. A battery with capacity >= 95% and not charging, is defined as charged.
 *              <ul>
 *                  <li> Apple-defined power sources will publish this key.
 *                  <li> For power source creators: Providing this key is REQUIRED.
 *                  <li> Type CFBoolean - kCFBooleanTrue or kCFBooleanFalse
 *              </ul>
 */

#define kIOPSIsChargedKey                   "Is Charged"

/*!
 * @define      kIOPSIsFinishingChargeKey
 * @abstract    CFDictionary key indicates whether the battery is finishing off its charge.
 * @discussion
 *              When this is true, the system UI should indicate that the battery is "Finishing Charge."
 *              Some batteries may continue charging after they report 100% capacity.
 *              <ul>
 *              <li> Apple-defined battery power sources will publish this key.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFBoolean - kCFBooleanTrue or kCFBooleanFalse
 *              </ul>
 */

#define kIOPSIsFinishingChargeKey              "Is Finishing Charge"

/*!
 * @define      kIOPSHardwareSerialNumberKey
 * @abstract    A unique serial number that identifies the power source.
 * @discussion  For Apple-manufactured batteries, this is an alphanumeric string generated
 *                  during the battery manufacturing process.
 *              <ul>
 *              <li> Apple-defined power sources will publish this key if the hardware provides the serial number.
 *              <li> For power source creators: Providing this key is RECOMMENDED.
 *              <li> Type CFStringRef
 *              </ul>
 */


#define kIOPSHardwareSerialNumberKey            "Hardware Serial Number"



/*
 * @group       Transport types
 * @abstract    Possible values for <code>@link kIOPSTransportTypeKey @/link</code>
 */
/*!
 * @define      kIOPSSerialTransportType
 * @abstract    Value for key <code>@link kIOPSTransportTypeKey @/link</code>. 
 * @discussion  Indicates the power source is a UPS attached over a serial connection.
 */
#define kIOPSSerialTransportType       "Serial"

/*!
 * @define      kIOPSUSBTransportType
 * @abstract    Value for key <code>@link kIOPSTransportTypeKey @/link</code>. 
 * @discussion  Indicates the power source is a UPS attached over a USB connection.
 */
#define kIOPSUSBTransportType          "USB"

/*!
 * @define      kIOPSNetworkTransportType
 * @abstract    Value for key <code>@link kIOPSTransportTypeKey @/link</code>. 
 * @discussion  Indicates the power source is a UPS attached over a network connection (and it may be managing several computers).
 */
#define kIOPSNetworkTransportType      "Ethernet"

/*!
 * @define      kIOPSInternalType
 * @abstract    Value for key <code>@link kIOPSTransportTypeKey @/link</code>. Indicates the power source is an internal battery.
*/
#define kIOPSInternalType              "Internal"


/*
 * @group       PowerSource Types
 * @discussion
 * A string that broadly describes the type of power source. One of these strings must be passed
 * as an argument to IOPSCreatePowerSource() when defining a new system power source.
 */

/*!
 * @define      kIOPSInternalBatteryType 
 *
 * @abstract    Represents a battery residing inside a Mac.
 */
#define kIOPSInternalBatteryType    "InternalBattery"

/*!
 * @define      kIOPSUPSType 
 *
 * @abstract    Represents an external attached UPS.
 */
#define kIOPSUPSType                "UPS"

/*
 * PS state 
 */
/*!
 * @define      kIOPSOffLineValue
 * @abstract    Value for key kIOPSPowerSourceStateKey. Power source is off-line or no longer connected.
*/
#define kIOPSOffLineValue              "Off Line"

/*!
 * @define      kIOPSACPowerValue
 * @abstract    Value for key kIOPSPowerSourceStateKey. Power source is connected to external or AC power, and is not draining the internal battery.
*/
#define kIOPSACPowerValue              "AC Power"

/*!
 * @define      kIOPSBatteryPowerValue
 * @abstract    Value for key kIOPSPowerSourceStateKey. Power source is currently using the internal battery.
*/
#define kIOPSBatteryPowerValue         "Battery Power"



/*!
 * @group Battery Health values
 */
/*!
 * @define      kIOPSPoorValue
 * @abstract    Value for key <code>@link kIOPSBatteryHealthKey @/link</code>.
*/
#define kIOPSPoorValue                  "Poor"

/*!
 * @define      kIOPSFairValue
 * @abstract    Value for key <code>@link kIOPSBatteryHealthKey @/link</code>.
*/
#define kIOPSFairValue                  "Fair"

/*!
 * @define      kIOPSGoodValue
 * @abstract    Value for key <code>@link kIOPSBatteryHealthKey @/link</code>.
*/
#define kIOPSGoodValue                  "Good"



/*
 * @group       Battery Health Condition values
 */
/*!
 * @define      kIOPSCheckBatteryValue
 *
 * @abstract    Value for key <code>@link kIOPSBatteryHealthConditionKey @/link</code>
 *
 * @discussion  This value indicates that the battery should be checked out by a licensed Mac repair service.
 */
#define kIOPSCheckBatteryValue                      "Check Battery"

/*!
 * @define      kIOPSPermanentFailureValue
 *
 * @abstract    Value for key <code>@link kIOPSBatteryHealthConditionKey @/link</code>
 *
 * @discussion  Indicates the battery needs replacement.
 */
#define kIOPSPermanentFailureValue                  "Permanent Battery Failure"


/*!
 * @group       Battery Failure Mode values
 */

/*!
 * @define      kIOPSFailureExternalInput
 *
 * @abstract    Value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureExternalInput                   "Externally Indicated Failure"
/*!
 *  @define     kIOPSFailureSafetyOverVoltage
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureSafetyOverVoltage               "Safety Over-Voltage"
/*!
 *  @define     kIOPSFailureChargeOverTemp
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureChargeOverTemp                  "Charge Over-Temperature"
/*!
 *  @define     kIOPSFailureDischargeOverTemp
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureDischargeOverTemp               "Discharge Over-Temperature"
/*!
 *  @define     kIOPSFailureCellImbalance
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureCellImbalance                   "Cell Imbalance"
/*!
 *  @define     kIOPSFailureChargeFET
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureChargeFET                       "Charge FET"
/*!
 *  @define     kIOPSFailureDischargeFET
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureDischargeFET                    "Discharge FET"
/*!
 *  @define     kIOPSFailureDataFlushFault
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureDataFlushFault                  "Data Flush Fault"
/*!
 *  @define     kIOPSFailurePermanentAFEComms
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailurePermanentAFEComms               "Permanent AFE Comms"
/*!
 *  @define     kIOPSFailurePeriodicAFEComms
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailurePeriodicAFEComms                "Periodic AFE Comms"
/*!
 *  @define     kIOPSFailureChargeOverCurrent
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureChargeOverCurrent               "Charge Over-Current"
/*!
 *  @define     kIOPSFailureDischargeOverCurrent
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureDischargeOverCurrent            "Discharge Over-Current"
/*!
 *  @define     kIOPSFailureOpenThermistor
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureOpenThermistor                  "Open Thermistor"
/*!
 *  @define     kIOPSFailureFuseBlown
 *
 *  @abstract   Potential value for key <code>@link kIOPSBatteryFailureModesKey@/link</code>
 */
#define kIOPSFailureFuseBlown                       "Fuse Blown"

#endif
