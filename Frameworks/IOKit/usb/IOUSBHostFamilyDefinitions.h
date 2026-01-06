/*
 * Copyright (c) 1998-2016 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

/*!
   @header     IOUSBHostFamilyDefinitions.h
   @brief      Definitions used for the IOUSBHostFamily
   !*/

#ifndef IOUSBHostFamily_IOUSBHostFamilyDefinitions_h
#define IOUSBHostFamily_IOUSBHostFamilyDefinitions_h

#define IOUSBHostFamilyBit(bit)                     ((uint32_t)(1) << bit)
#define IOUSBHostFamilyBitRange(start, end)         (~(((uint32_t)(1) << start) - 1) & (((uint32_t)(1) << end) | (((uint32_t)(1) << end) - 1)))
#define IOUSBHostFamilyBitRange64(start, end)       (~(((uint64_t)(1) << start) - 1) & (((uint64_t)(1) << end) | (((uint64_t)(1) << end) - 1)))
#define IOUSBHostFamilyBitRangePhase(start, end)    (start)

#define iokit_usb_codemask                          IOUSBHostFamilyBitRange(0, 11)
#define iokit_usbhost_group                         (1 << IOUSBHostFamilyBitRangePhase(12, 13))
#define iokit_usblegacy_group                       (0 << IOUSBHostFamilyBitRangePhase(12, 13))
#define iokit_usbhost_err(message)                  ((IOReturn)(iokit_family_err(sub_iokit_usb, iokit_usbhost_group | (message & iokit_usb_codemask))))
#define iokit_usbhost_msg(message)                  ((uint32_t)(iokit_family_msg(sub_iokit_usb, iokit_usbhost_group | (message & iokit_usb_codemask))))
#define iokit_usblegacy_err_msg(message)            ((uint32_t)(sys_iokit | sub_iokit_usb | message))

/*!
   @definedblock  IOUSBHostFamily message codes
   @discussion    Messages passed between USB services using the <code>IOService::message</code> API.
 */
#define kUSBHostMessageConfigurationSet             iokit_usbhost_msg(0x00) // 0xe0005000  IOUSBHostDevice -> clients upon a setConfiguration call.
#define kUSBHostMessageRenegotiateCurrent           iokit_usbhost_msg(0x01) // 0xe0005001  Request clients to renegotiate bus current allocations

#define kUSBHostReturnPipeStalled                   iokit_usbhost_err(0x0)  // 0xe0005000  Pipe has issued a STALL handshake.  Use clearStall to clear this condition.
#define kUSBHostReturnNoPower                       iokit_usbhost_err(0x1)  // 0xe0005001  A setConfiguration call was not able to succeed because all configurations require more power than is available.

/*!
 * @enum       tIOUSBHostConnectionSpeed
 * @brief      Connection speeds reported in kUSBHostMatchingPropertySpeed
 * @discussion This enumeration matches the default speed ID mappings defined in XHCI 1.0 Table 147.
 * @constant   kIOUSBHostConnectionSpeedNone No device is connected
 * @constant   kIOUSBHostConnectionSpeedFull A full-speed (12 Mb/s) device is connected
 * @constant   kIOUSBHostConnectionSpeedLow A low-speed (1.5 Mb/s) device is connected
 * @constant   kIOUSBHostConnectionSpeedHigh A high-speed (480 Mb/s) device is connected)
 * @constant   kIOUSBHostConnectionSpeedSuper A superspeed (5 Gb/s) device is connected)
 * @constant   kIOUSBHostConnectionSpeedSuperPlus A superspeed (10 Gb/s) device is connected)
 * @constant   kIOUSBHostConnectionSpeedSuperPlusBy2 A superspeed (20 Gb/s) device is connected)
 */
enum tIOUSBHostConnectionSpeed
{
    kIOUSBHostConnectionSpeedNone         = 0,
    kIOUSBHostConnectionSpeedFull         = 1,
    kIOUSBHostConnectionSpeedLow          = 2,
    kIOUSBHostConnectionSpeedHigh         = 3,
    kIOUSBHostConnectionSpeedSuper        = 4,
    kIOUSBHostConnectionSpeedSuperPlus    = 5,
    kIOUSBHostConnectionSpeedSuperPlusBy2 = 6,
    kIOUSBHostConnectionSpeedCount        = 7
};

#pragma mark Registry property names

#define kUSBHostMatchingPropertySpeed                           "USBSpeed"
#define kUSBHostMatchingPropertyPortType                        "USBPortType"

#define kUSBHostMatchingPropertyVendorID                        "idVendor"
#define kUSBHostMatchingPropertyProductID                       "idProduct"
#define kUSBHostMatchingPropertyProductIDMask                   "idProductMask"
#define kUSBHostMatchingPropertyProductIDArray                  "idProductArray"
#define kUSBHostMatchingPropertyDeviceClass                     "bDeviceClass"
#define kUSBHostMatchingPropertyDeviceSubClass                  "bDeviceSubClass"
#define kUSBHostMatchingPropertyDeviceProtocol                  "bDeviceProtocol"
#define kUSBHostMatchingPropertyDeviceReleaseNumber             "bcdDevice"
#define kUSBHostMatchingPropertyConfigurationValue              "bConfigurationValue"
#define kUSBHostMatchingPropertyInterfaceClass                  "bInterfaceClass"
#define kUSBHostMatchingPropertyInterfaceSubClass               "bInterfaceSubClass"
#define kUSBHostMatchingPropertyInterfaceProtocol               "bInterfaceProtocol"
#define kUSBHostMatchingPropertyInterfaceNumber                 "bInterfaceNumber"

#define kUSBHostPropertyLocationID                              "locationID"
#define kUSBHostPropertyDebugOptions                            "kUSBDebugOptions"
#define kUSBHostPropertyWakePowerSupply                         "kUSBWakePowerSupply"
#define kUSBHostPropertySleepPowerSupply                        "kUSBSleepPowerSupply"
#define kUSBHostPropertyWakePortCurrentLimit                    "kUSBWakePortCurrentLimit"
#define kUSBHostPropertySleepPortCurrentLimit                   "kUSBSleepPortCurrentLimit"
#define kUSBHostPropertyFailedRemoteWake                        "kUSBFailedRemoteWake"
#define kUSBHostPropertyBusCurrentPoolID                        "UsbBusCurrentPoolID"
#define kUSBHostPropertySmcBusCurrentPoolID                     "UsbSmcBusCurrentPoolID"
#define kUSBHostPropertyUserClientEntitlementRequired           "UsbUserClientEntitlementRequired"
#define kUSBHostPropertyUserClientEnableReset                   "kUSBLegacyForceReEnumerate"    // TODO: Rename this
#define kUSBHostPropertyUserClientOwningTaskName                "UsbUserClientOwningTaskName"
#define kUSBHostPropertyForcePower                              "UsbForcePower"

#define kUSBHostDevicePropertyVendorString                      "kUSBVendorString"
#define kUSBHostDevicePropertySerialNumberString                "kUSBSerialNumberString"
#define kUSBHostDevicePropertyContainerID                       "kUSBContainerID"
#define kUSBHostDevicePropertyFailedRequestedPower              "kUSBFailedRequestedPower"
#define kUSBHostDevicePropertyResumeRecoveryTime                "kUSBResumeRecoveryTime"
#define kUSBHostDevicePropertyPreferredConfiguration            "kUSBPreferredConfiguration"
#define kUSBHostDevicePropertyPreferredRecoveryConfiguration    "kUSBPreferredRecoveryConfiguration"
#define kUSBHostDevicePropertyCurrentConfiguration              "kUSBCurrentConfiguration"
#define kUSBHostDevicePropertyRemoteWakeOverride                "kUSBRemoteWakeOverride"
#define kUSBHostDevicePropertyConfigurationDescriptorOverride   "kUSBConfigurationDescriptorOverride"
#define kUSBHostDevicePropertyDeviceDescriptorOverride          "kUSBDeviceDescriptorOverride"
#define kUSBHostDevicePropertyConfigurationCurrentOverride      "kUSBConfigurationCurrentOverride"
#define kUSBHostDevicePropertyResetDurationOverride             "kUSBResetDurationOverride"
#define kUSBHostDevicePropertyDesiredChargingCurrent            "kUSBDesiredChargingCurrent"
#define kUSBHostDevicePropertyDescriptorOverride                "kUSBDescriptorOverride"
#define kUSBHostDescriptorOverrideVendorStringIndex             "UsbDescriptorOverrideVendorStringIndex"
#define kUSBHostDescriptorOverrideProductStringIndex            "UsbDescriptorOverrideProductStringIndex"
#define kUSBHostDescriptorOverrideSerialNumberStringIndex       "UsbDescriptorOverrideSerialNumberStringIndex"
#define kUSBHostDevicePropertyDeviceECID                        "kUSBDeviceECID"
#define kUSBHostDevicePropertyEnableLPM                         "kUSBHostDeviceEnableLPM"
#define kUSBHostDevicePropertyDisablePortLPM                    "kUSBHostDeviceDisablePortLPM"         // Disable port initiated LPM for this device

#define kUSBHostBillboardDevicePropertyNumberOfAlternateModes   "bNumberOfAlternateModes"
#define kUSBHostBillboardDevicePropertyPreferredAlternateMode   "bPreferredAlternateMode"
#define kUSBHostBillboardDevicePropertyVCONNPower               "VCONNPower"
#define kUSBHostBillboardDevicePropertyConfigured               "bmConfigured"
#define kUSBHostBillboardDevicePropertyAdditionalFailureInfo    "bAdditonalFailureInfo"
#define kUSBHostBillboardDevicePropertyBcdVersion               "BcdVersion"
#define kUSBHostBillboardDevicePropertySVID                     "wSVID"
#define kUSBHostBillboardDevicePropertyAlternateMode            "bAlternateMode"
#define kUSBHostBillboardDevicePropertyAlternateModeStringIndex "iAlternateModeString"
#define kUSBHostBillboardDevicePropertyAlternateModeString      "AlternateModeString"
#define kUSBHostBillboardDevicePropertyAddtionalInfoURLIndex    "iAddtionalInfoURL"
#define kUSBHostBillboardDevicePropertyAddtionalInfoURL         "AddtionalInfoURL"
#define kUSBHostBillboardDevicePropertydwAlternateModeVdo       "dwAlternateModeVdo"

#define kUSBHostInterfacePropertyAlternateSetting               "bAlternateSetting"

#define kUSBHostPortPropertyOvercurrent                         "UsbHostPortOvercurrent"
#define kUSBHostPortPropertyPortNumber                          "port"
#define kUSBHostPortPropertyRemovable                           "removable"
#define kUSBHostPortPropertyTestMode                            "kUSBTestMode"
#define kUSBHostPortPropertySimulateInterrupt                   "kUSBSimulateInterrupt"
#define kUSBHostPortPropertyBusCurrentAllocation                "kUSBBusCurrentAllocation"
#define kUSBHostPortPropertyBusCurrentSleepAllocation           "kUSBBusCurrentSleepAllocation"
#define kUSBHostPortPropertyConnectable                         "UsbConnectable"
#define kUSBHostPortPropertyConnectorType                       "UsbConnector"
#define kUSBHostPortPropertyMux                                 "UsbMux"
#define kUSBHostPortPropertyCompanionIndex                      "kUSBCompanionIndex"
#define kUSBHostPortPropertyDisconnectInterval                  "kUSBDisconnectInterval"
#define kUSBHostPortPropertyUsbCPortNumber                      "UsbCPortNumber"
#define kUSBHostPortPropertyCompanionPortNumber                 "UsbCompanionPortNumber"                // OSData  key to set/get the port number of the companion port
#define kUSBHostPortPropertyPowerSource                         "UsbPowerSource"

#define kUSBHostHubPropertyPowerSupply                          "kUSBHubPowerSupply"                    // OSNumber mA available for downstream ports, 0 for bus-powered
#define kUSBHostHubPropertyIdlePolicy                           "kUSBHubIdlePolicy"                     // OSNumber ms to be used as device idle policy
#define kUSBHostHubPropertyStartupDelay                         "kUSBHubStartupDelay"                   // OSNumber ms delay before creating downstream ports
#define kUSBHostHubPropertyPortSequenceDelay                    "kUSBHubPortSequenceDelay"              // OSNumber ms delay between port creation
#define kUSBHostHubPropertyHubPowerSupplyType                   "kUSBHubPowerSupplyType"                // OSNumber for tPowerSupply hub is, 2 for bus-powered, 1 for self

#define kUSBHostControllerPropertyIsochronousRequiresContiguous "kUSBIsochronousRequiresContiguous"
#define kUSBHostControllerPropertySleepSupported                "kUSBSleepSupported"
#define kUSBHostControllerPropertyRTD3Supported                 "UsbRTD3Supported"
#define kUSBHostControllerPropertyMuxEnabled                    "kUSBMuxEnabled"
#define kUSBHostControllerPropertyCompanion                     "kUSBCompanion"                         // OSBoolean false to disable all companion controllers
#define kUSBHostControllerPropertyLowSpeedCompanion             "kUSBLowSpeedCompanion"                 // OSBoolean false to disable low-speed companion controller
#define kUSBHostControllerPropertyFullSpeedCompanion            "kUSBFullSpeedCompanion"                // OSBoolean false to disable full-speed companion controller
#define kUSBHostControllerPropertyHighSpeedCompanion            "kUSBHighSpeedCompanion"                // OSBoolean false to disable high-speed companion controller
#define kUSBHostControllerPropertySuperSpeedCompanion           "kUSBSuperSpeedCompanion"               // OSBoolean false to disable superspeed companion controller
#define kUSBHostControllerPropertyRevision                      "Revision"                              // OSData    Major/minor revision number of controller
#define kUSBHostControllerPropertyCompanionControllerName       "UsbCompanionControllerName"            // OSString  key to set/get the name of the service, i.e. companion controller dictionary.
#define kUSBHostControllerPropertyDisableUSB3LPM                "kUSBHostControllerDisableUSB3LPM"      // OSBoolean true to disable USB3 LPM on a given controller
#define kUSBHostControllerPropertyDisableUSB2LPM                "kUSBHostControllerDisableUSB2LPM"      // OSBoolean true to disable USB2 LPM on a given controller
#define kUSBHostControllerPropertyDisableWakeSources            "UsbHostControllerDisableWakeSources"   // OSBoolean true to disable connect/disconnect/overcurrent wake sources
#define kUSBHostControllerPropertyPersistFullSpeedIsochronous   "UsbHostControllerPersistFullSpeedIsochronous"  // OSBoolean true to reduce commands related to full-speed isochronous endpoints

#define kUSBHostPortPropertyExternalDeviceResetController       "kUSBHostPortExternalDeviceResetController"
#define kUSBHostPortPropertyExternalDevicePowerController       "kUSBHostPortExternalDevicePowerController"

#define kUSBHostPortPropertyCardReader                          "kUSBHostPortPropertyCardReader"
#define kUSBHostPortPropertyCardReaderValidateDescriptors       "kUSBHostPortPropertyCardReaderValidateDescriptors"

#define kUSBHostPortPropertyOffset                              "kUSBHostPortPropertyOffset"

#define kIOUSBHostDeviceClassName                               "IOUSBHostDevice"
#define kIOUSBHostInterfaceClassName                            "IOUSBHostInterface"

// for IOUSBLib compatibility
#define kUSBHostDevicePropertyAddress                           "kUSBAddress"
#define kUSBHostDevicePropertyManufacturerStringIndex           "iManufacturer"
#define kUSBHostDevicePropertySerialNumberStringIndex           "iSerialNumber"
#define kUSBHostDevicePropertyProductStringIndex                "iProduct"
#define kUSBHostDevicePropertyProductString                     "kUSBProductString"
#define kUSBHostDevicePropertyNumConfigs                        "bNumConfigurations"
#define kUSBHostDevicePropertyMaxPacketSize                     "bMaxPacketSize0"
#define kUSBHostDevicePropertyStandardVersion                   "bcdUSB"

#define kUSBHostInterfacePropertyStringIndex                    "iInterface"
#define kUSBHostInterfacePropertyString                         "kUSBString"
#define kUSBHostInterfacePropertyNumEndpoints                   "bNumEndpoints"

// Legacy power properties
#define kAppleMaxPortCurrent                                    "AAPL,current-available"
#define kAppleCurrentExtra                                      "AAPL,current-extra"
#define kAppleMaxPortCurrentInSleep                             "AAPL,max-port-current-in-sleep"
#define kAppleCurrentExtraInSleep                               "AAPL,current-extra-in-sleep"
#define kAppleExternalConnectorBitmap                           "AAPL,ExternalConnectorBitmap"

#endif /* IOUSBHostFamily_IOUSBHostFamilyDefinitions_h */
