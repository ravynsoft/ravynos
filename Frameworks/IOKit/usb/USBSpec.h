/*
 * Copyright � 1998-2012 Apple Inc. All rights reserved.
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

/*
 * Constants that both OS9 and OSX want to define, and whose values are
 * specified by the USB Standard.
 * Put in a seperate file so they can be included if the OS9 include file isn't already
 * included.
 */
#ifndef _USBSPEC_H
#define _USBSPEC_H

#include <IOKit/usb/AppleUSBDefinitions.h>
#include <IOKit/usb/IOUSBHostFamilyDefinitions.h>

#if !__OPEN_SOURCE__ && TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
// these headers were previously exported for iOS clients
#include <IOKit/usb/IOUSBHostFamily.h>
#include <IOKit/usb/StandardUSB.h>
#endif


#ifdef __cplusplus

#if !__OPEN_SOURCE__ && TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
using namespace StandardUSB;
#endif

extern "C" {
#endif

#if !__OPEN_SOURCE__ && TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
/* @enum Feature constants
   @discussion Used with SET_FEATURE requests
 */
enum
{
    kUSBFunctionSuspendLowPower         = kIOUSBInterfaceSuspendLowPower,
    kUSBFunctionSuspendEnableRemoteWake = kIOUSBInterfaceSuspendRemoteWakeEnable
};

/*!
   @enum Device Status
   @discussion Specified bits returned by a get status request
 */
enum
{
    kUSBDeviceStatusSelfPowered = kIOUSBDeviceStatusSelfPowered,
    kUSBDeviceStatusRemoteWake  = kIOUSBDeviceStatusRemoteWakeEnable,
    kUSBDeviceStatusU1Enable    = kIOUSBDeviceStatusU1Enable,
    kUSBDeviceStatusU2Enable    = kIOUSBDeviceStatusU2Enable,
    kUSBDeviceStatusLTMEnable   = kIOUSBDeviceStatusLTMEnable,

    kUSBInterfaceStatusFunctionRemoteWakeCapable = kIOUSBInterfaceStatusRemoteWakeCapable,
    kUSBInterfaceStatusFunctionRemoteWake        = kIOUSBInterfaceStatusRemoteWakeEnable,

    kUSBPipeStatusHalt = IOUSBEndpointStatusHalt
};

#endif

/*!
   @header     USBSpec.h
   @abstract  Constants and definitions of parameters that are used in communcating with USB devices and interfaces.
   @discussion
 */

/*!
   @enum Endpoint type
   @discussion Used in IOUSBFindEndpointRequest's type field
 */
enum
{
    kUSBControl   = kIOUSBEndpointTypeControl,
    kUSBIsoc      = kIOUSBEndpointTypeIsochronous,
    kUSBBulk      = kIOUSBEndpointTypeBulk,
    kUSBInterrupt = kIOUSBEndpointTypeInterrupt,
    kUSBAnyType   = 0xFF
};

/*!
   @enum Endpoint direction
   @discussion Used in IOUSBFindEndpointRequest's direction field
 */
enum
{
    kUSBOut     = kIOUSBEndpointDirectionOut,
    kUSBIn      = kIOUSBEndpointDirectionIn,
    kUSBNone    = kIOUSBEndpointDirectionUnknown,
    kUSBAnyDirn = 3
};

/*!
   @enum Device Request Type
   @discussion This type is encoded in the bmRequestType field of a Device Request.  It specifies the type of request: standard, class or vendor specific.
 */
enum
{
    kUSBStandard = kIOUSBDeviceRequestTypeValueStandard,
    kUSBClass    = kIOUSBDeviceRequestTypeValueClass,
    kUSBVendor   = kIOUSBDeviceRequestTypeValueVendor
};

/*!
   @enum Device Request Recipient
   @discussion This recipient is encoded in the bmRequestType field of a Device Request.  It specifies the type of recipient for a request:  the device, the interface, or an endpoint.
 */
enum
{
    kUSBDevice    = kIOUSBDeviceRequestRecipientValueDevice,
    kUSBInterface = kIOUSBDeviceRequestRecipientValueInterface,
    kUSBEndpoint  = kIOUSBDeviceRequestRecipientValueEndpoint,
    kUSBOther     = kIOUSBDeviceRequestRecipientValueOther
};

/*!
   @enum Device Request
   @discussion Specifies values for the bRequest field of a Device Request.
 */
enum
{
    kUSBRqGetStatus     = kIOUSBDeviceRequestGetStatus,
    kUSBRqClearFeature  = kIOUSBDeviceRequestClearFeature,
    kUSBRqGetState      = kIOUSBDeviceRequestGetState,
    kUSBRqSetFeature    = kIOUSBDeviceRequestSetFeature,
    kUSBRqReserved2     = 4,
    kUSBRqSetAddress    = kIOUSBDeviceRequestSetAddress,
    kUSBRqGetDescriptor = kIOUSBDeviceRequestGetDescriptor,
    kUSBRqSetDescriptor = kIOUSBDeviceRequestSetDescriptor,
    kUSBRqGetConfig     = kIOUSBDeviceRequestGetConfiguration,
    kUSBRqSetConfig     = kIOUSBDeviceRequestSetConfiguration,
    kUSBRqGetInterface  = kIOUSBDeviceRequestGetInterface,
    kUSBRqSetInterface  = kIOUSBDeviceRequestSetInterface,
    kUSBRqSyncFrame     = kIOUSBDeviceRequestSynchFrame,
    kUSBSetSel          = kIOUSBDeviceRequestSetSel,
    kUSBSetIsochDelay   = kIOUSBDeviceRequestSetIsochronousDelay
};

/*!
   @enum USB Descriptors
   @discussion Specifies values for diffent descriptor types.
 */
enum
{
    kUSBAnyDesc                     = 0, // Wildcard for searches
    kUSBDeviceDesc                  = kIOUSBDescriptorTypeDevice,
    kUSBConfDesc                    = kIOUSBDescriptorTypeConfiguration,
    kUSBStringDesc                  = kIOUSBDescriptorTypeString,
    kUSBInterfaceDesc               = kIOUSBDescriptorTypeInterface,
    kUSBEndpointDesc                = kIOUSBDescriptorTypeEndpoint,
    kUSBDeviceQualifierDesc         = kIOUSBDescriptorTypeDeviceQualifier,
    kUSBOtherSpeedConfDesc          = kIOUSBDescriptorTypeOtherSpeedConfiguration,
    kUSBInterfacePowerDesc          = kIOUSBDescriptorTypeInterfacePower,
    kUSBOnTheGoDesc                 = kIOUSBDescriptorTypeOTG,
    kUSDebugDesc                    = kIOUSBDescriptorTypeDebug,
    kUSBInterfaceAssociationDesc    = kIOUSBDescriptorTypeInterfaceAssociation,
    kUSBBOSDescriptor               = kIOUSBDescriptorTypeBOS,
    kUSBDeviceCapability            = kIOUSBDescriptorTypeDeviceCapability,
    kUSBSuperSpeedEndpointCompanion = kIOUSBDescriptorTypeSuperSpeedUSBEndpointCompanion,
    kUSB3HUBDesc                    = kIOUSBDescriptorTypeSuperSpeedHub,
    kUSBHIDDesc                     = kIOUSBDecriptorTypeHID,
    kUSBReportDesc                  = kIOUSBDecriptorTypeReport,
    kUSBPhysicalDesc                = 0x23,
    kUSBHUBDesc                     = kIOUSBDescriptorTypeHub,
};


/*!
   @enum Device Capability Types
   @discussion Used with decoding the Device Capability descriptor
 */
enum
{
    kUSBDeviceCapabilityWirelessUSB      = kIOUSBDeviceCapabilityTypeWireless,
    kUSBDeviceCapabilityUSB20Extension   = kIOUSBDeviceCapabilityTypeUSB20Extension,
    kUSBDeviceCapabilitySuperSpeedUSB    = kIOUSBDeviceCapabilityTypeSuperSpeed,
    kUSBDeviceCapabilityContainerID      = kIOUSBDeviceCapabilityTypeContainerID,
    kUSBDeviceCapabilityBillboard        = kIOUSBDeviceCapabilityTypeBillboard,
    kUSBDeviceCapabilityBillboardAltMode = kIOUSBDeviceCapabilityTypeBillboardAltMode
};

/*!
   @enum Feature Selectors
   @discussion Used with SET/CLEAR_FEATURE requests.
 */
enum
{
    kUSBFeatureEndpointStall      = IOUSBEndpointFeatureSelectorStall,      // Endpoint
    kUSBFeatureDeviceRemoteWakeup = kIOUSBDeviceFeatureSelectorRemoteWakeup,      // Device
    kUSBFeatureTestMode           = kIOUSBDeviceFeatureSelectorTestMode,      // Device
    kUSBFeatureFunctionSuspend    = kIOUSBInterfaceFeatureSelectorSuspend,      // Interface
    kUSBFeatureU1Enable           = kIOUSBDeviceFeatureSelectorU1Enable,     // Device
    kUSBFeatureU2Enable           = kIOUSBDeviceFeatureSelectorU2Enable,     // Device
    kUSBFeatureLTMEnable          = kIOUSBDeviceFeatureSelectorLTMEnable      // Device
};

/*!
   @enum Miscellaneous bits and masks
 */
enum
{
    kUSBFunctionRemoteWakeCapableBit = IOUSBBit(0),   //  GET_STATUS
    kUSBFunctionRemoteWakeupBit      = IOUSBBit(1),   //  GET_STATUS
    kUSBLowPowerSuspendStateBit      = IOUSBBit(0),   //  SET_FEATURE(FUNCTION_SUSPEND)
    kUSBFunctionRemoteWakeEnableBit  = IOUSBBit(1)    //  SET_FEATURE(FUNCTION_SUSPEND)
};

/*!
   @enum USB Power constants
   @discussion Constants relating to USB Power.
 */
enum
{
    kUSB100mAAvailable   = 50,
    kUSB500mAAvailable   = 250,
    kUSB100mA            = 50,
    kUSBAtrBusPowered    = 0x80,
    kUSBAtrSelfPowered   = 0x40,
    kUSBAtrRemoteWakeup  = 0x20,
    kUSB2MaxPowerPerPort = kIOUSB20BusCurrentDefault,
    kUSB150mAAvailable   = 75,
    kUSB900mAAvailable   = 450,
    kUSB150mA            = 75,
    kUSB3MaxPowerPerPort = kIOUSB30BusCurrentDefault
};

/*!
   @enum USB Release constants
   @discussion Constants relating to USB releases as found in the bcdUSB field of the Device Descriptor.
 */
enum
{
    kUSBRel10 = 0x0100,
    kUSBRel11 = 0x0110,
    kUSBRel20 = 0x0200,
    kUSBRel30 = 0x0300
};


/*!
   @enum HID requests
   @discussion Constants for HID requests.
 */
enum
{
    kHIDRqGetReport   = 1,
    kHIDRqGetIdle     = 2,
    kHIDRqGetProtocol = 3,
    kHIDRqSetReport   = 9,
    kHIDRqSetIdle     = 10,
    kHIDRqSetProtocol = 11
};

/*!
   @enum HID report types
   @discussion Constants for the three kinds of HID reports.
 */
enum
{
    kHIDRtInputReport   = 1,
    kHIDRtOutputReport  = 2,
    kHIDRtFeatureReport = 3
};


/*!
   @enum HID Protocol
   @discussion  Used in the SET_PROTOCOL device request
 */
enum
{
    kHIDBootProtocolValue   = 0,
    kHIDReportProtocolValue = 1
};



enum
{
    kUSBCapsLockKey   = 0x39,
    kUSBNumLockKey    = 0x53,
    kUSBScrollLockKey = 0x47
};

/*!
   @enum Device Class Codes
   @discussion Constants for USB Device classes (bDeviceClass).
 */
enum
{
    kUSBCompositeClass           = 0,
    kUSBCommClass                = 2,   // Deprecated
    kUSBCommunicationClass       = 2,
    kUSBHubClass                 = 9,
    kUSBDataClass                = 10,
    kUSBPersonalHealthcareClass  = 15,
    kUSBBillBoardClass           = 17,
    kUSBDiagnosticClass          = 220,
    kUSBWirelessControllerClass  = 224,
    kUSBMiscellaneousClass       = 239,
    kUSBApplicationSpecificClass = 254,
    kUSBVendorSpecificClass      = 255
};

/*!
   @enum Interface Class
   @discussion Constants for Interface classes (bInterfaceClass).
 */
enum
{
    kUSBAudioClass          = 1,    // Deprecated
    kUSBAudioInterfaceClass = 1,

    kUSBCommunicationControlInterfaceClass = 2,
    kUSBCommunicationDataInterfaceClass    = 10,

    kUSBHIDClass          = 3,
    kUSBHIDInterfaceClass = 3,

    kUSBPhysicalInterfaceClass = 5,

    kUSBImageInterfaceClass = 6,

    kUSBPrintingClass          = 7, // Deprecated
    kUSBPrintingInterfaceClass = 7,

    kUSBMassStorageClass          = 8,  // Deprecated
    kUSBMassStorageInterfaceClass = 8,

    kUSBChipSmartCardInterfaceClass = 11,

    kUSBContentSecurityInterfaceClass = 13,

    kUSBVideoInterfaceClass = 14,

    kUSBPersonalHealthcareInterfaceClass = 15,

    kUSBDiagnosticDeviceInterfaceClass = 220,

    kUSBWirelessControllerInterfaceClass = 224,

    kUSBApplicationSpecificInterfaceClass = 254,

    kUSBVendorSpecificInterfaceClass = 255
};

// Obsolete
enum
{
    kUSBDisplayClass = 4   // Obsolete
};

/*!
    @enum Interface SubClass
    @discussion Constants for USB Interface SubClasses (bInterfaceSubClass).
 */
enum
{
    kUSBCompositeSubClass = 0,

    kUSBHubSubClass = 0,

    // For the kUSBAudioInterfaceClass
    //
    kUSBAudioControlSubClass   = 0x01,
    kUSBAudioStreamingSubClass = 0x02,
    kUSBMIDIStreamingSubClass  = 0x03,

    // For the kUSBApplicationSpecificInterfaceClass
    //
    kUSBDFUSubClass             = 0x01,
    kUSBIrDABridgeSubClass      = 0x02,
    kUSBTestMeasurementSubClass = 0x03,

    // For the kUSBMassStorageInterfaceClass
    //
    kUSBMassStorageRBCSubClass      = 0x01,
    kUSBMassStorageATAPISubClass    = 0x02,
    kUSBMassStorageQIC157SubClass   = 0x03,
    kUSBMassStorageUFISubClass      = 0x04,
    kUSBMassStorageSFF8070iSubClass = 0x05,
    kUSBMassStorageSCSISubClass     = 0x06,

    // For the kUSBHIDInterfaceClass
    //
    kUSBHIDBootInterfaceSubClass = 0x01,

    // For the kUSBCommunicationDataInterfaceClass
    //
    kUSBCommDirectLineSubClass         = 0x01,
    kUSBCommAbstractSubClass           = 0x02,
    kUSBCommTelephoneSubClass          = 0x03,
    kUSBCommMultiChannelSubClass       = 0x04,
    kUSBCommCAPISubClass               = 0x05,
    kUSBCommEthernetNetworkingSubClass = 0x06,
    kUSBATMNetworkingSubClass          = 0x07,

    // For the kUSBDiagnosticDeviceInterfaceClass
    //
    kUSBReprogrammableDiagnosticSubClass = 0x01,

    // For the kUSBWirelessControllerInterfaceClass
    //
    kUSBRFControllerSubClass = 0x01,

    // For the kUSBMiscellaneousClass
    //
    kUSBCommonClassSubClass = 0x02,

    // For the kUSBVideoInterfaceClass
    //
    kUSBVideoControlSubClass             = 0x01,
    kUSBVideoStreamingSubClass           = 0x02,
    kUSBVideoInterfaceCollectionSubClass = 0x03

};

enum USBClassSpecificDesc
{
    kUSBClassSpecificDescriptor = 0x24
};


/*!
   @enum	Interface Protocol
   @discussion Reported in the bInterfaceProtocol field of the Interface Descriptor.
 */
enum
{

    // For kUSBHubClass
    kHubSuperSpeedProtocol = 3,

    // For kUSBHIDInterfaceClass
    //
    kHIDNoInterfaceProtocol       = 0,
    kHIDKeyboardInterfaceProtocol = 1,
    kHIDMouseInterfaceProtocol    = 2,
    kUSBVendorSpecificProtocol    = 0xff,

    // For kUSBDiagnosticDeviceInterfaceClass
    //
    kUSB2ComplianceDeviceProtocol = 0x01,

    // For kUSBWirelessControllerInterfaceClass
    //
    kUSBBluetoothProgrammingInterfaceProtocol = 0x01,

    // For kUSBMiscellaneousClass
    //
    KUSBInterfaceAssociationDescriptorProtocol = 0x01,

    // For Mass Storage
    //
    kMSCProtocolControlBulkInterrupt = 0x00,
    kMSCProtocolControlBulk          = 0x01,
    kMSCProtocolBulkOnly             = 0x50,
    kMSCProtocolUSBAttachedSCSI      = 0x62
};


/*!
    @enum DFU Class Attributes
    @discussion
 */
enum
{
    kUSBDFUAttributesMask           = 0x07,
    kUSBDFUCanDownloadBit           = 0,
    kUSBDFUCanUploadBit             = 1,
    kUSBDFUManifestationTolerantBit = 2
};

/*!
   @enum Printer Class Requests
   @discussion The bRequest parameter for Printing Class Sepcific Requests
 */
enum
{
    kUSPrintingClassGetDeviceID  = 0,
    kUSPrintingClassGePortStatus = 1,
    kUSPrintingClassSoftReset    = 2
};

/*!
   @enum Endpoint Descriptor bits
   @discussion Bit definitions for endpoint descriptor fields
 */
enum
{
    kUSBbEndpointAddressMask   = 0x0f,
    kUSBbEndpointDirectionBit  = kIOUSBEndpointDescriptorDirectionPhase,
    kUSBbEndpointDirectionMask = kIOUSBEndpointDescriptorDirection,
    kUSBEndpointDirectionOut   = kIOUSBEndpointDescriptorDirectionOut,
    kUSBEndpointDirectionIn    = kIOUSBEndpointDescriptorDirectionIn,

    kUSBEndpointbmAttributesTransferTypeMask         = kIOUSBEndpointDescriptorTransferType,
    kUSBEndpointbmAttributesSynchronizationTypeMask  = kIOUSBEndpointDescriptorSynchronizationType,
    kUSBEndpointbmAttributesSynchronizationTypeShift = 2,
    kUSBEndpointbmAttributesUsageTypeMask            = kIOUSBEndpointDescriptorUsageType,
    kUSBEndpointbmAttributesUsageTypeShift           = 4,

    kUSBPeriodicInterruptUsageType        = 0,
    kUSBNotificationInterruptUsageType    = 1,
    kUSBNoSynchronizationIsocSyncType     = 0,
    kUSBAsynchronousIsocSyncType          = 1,
    kUSBAdaptiveIsocSyncType              = 2,
    kUSBSynchronousIsocSyncType           = 3,
    kUSBDataIsocUsageType                 = 0,
    kUSBFeedbackIsocUsageType             = 1,
    kUSBImplicitFeedbackDataIsocUsageType = 2
};

/*!
   @enum USB Device Capability Type constants
   @discussion Bit definitions and constants for different values of USB Device Capability types
 */
enum
{
    kUSB20ExtensionLPMSupported =       1,  // Bit 1 of bmAttributes of USB 2.0 Extension Device Capability
    kUSBSuperSpeedLTMCapable    =       1,  // Bit 1 of bmAttributes of SuperSpeed USB Device Capability
    kUSBSuperSpeedSupportsLS    =       0,  // Value of wSpeedSupported indicating that the device supports low speed
    kUSBSuperSpeedSupportsFS    =       1,  // Value of wSpeedSupported indicating that the device supports full speed
    kUSBSuperSpeedSupportsHS    =       2,  // Value of wSpeedSupported indicating that the device supports high speed
    kUSBSuperSpeedSupportsSS    =       3,  // Value of wSpeedSupported indicating that the device supports 5 Gbps
};

/*!
   @enum USB Device Billboard Capability Vconn Power constants
   @discussion Power needed by the adapter for full functionality
 */
enum
{
    kUSBBillboardVConn1Watt    =   0,
    kUSBBillboardVConn1P5Watt  =   1,
    kUSBBillboardVConn2Watt    =   2,
    kUSBBillboardVConn3Watt    =   3,
    kUSBBillboardVConn4Watt    =   4,
    kUSBBillboardVConn5Watt    =   5,
    kUSBBillboardVConn6Watt    =   6,
    kUSBBillboardVConnReserved =   7
};

/*!
   @defineblock USB Billboard Capability Descriptor Constant
   @define	kUSBBillboardVConnNoPowerReq	bit position for No Power Required, VConn b2:0 are ignored when this bit is set
 */

#define kUSBBillboardVConnNoPowerReq    15

/*! @/defineblock */

/*!
   @enum USB Device Billboard Capability bmConfigured constants
   @ A bit pair signifying the state of Alternate Modes
 */
enum
{
    kUSBBillboardUnspecifiedError     =   0,
    kUSBBillboardConfigNotAttempted   =   1,
    kUSBBillboardConfigUnsuccessful   =   2,
    kUSBBillboardAltModeConfigSuccess =   3
};

/*!
   @enum USB Device Billboard Capability AdditionalFailureInfo constants
   @ A bit field signifying additional failure information
 */
enum
{
    kUSBBillboardAdditinalInfoNoPower =   1,
    kUSBBillboardAdditinalInfoNoUSBPD =   2
};

/*!
   @defineblock USB Descriptor and IORegistry constants
   @discussion  Various constants used to describe the fields in the various USB Device Descriptors and IORegistry names used for some of those fields

   @define	kUSBDeviceClass				The field in the USB Device Descriptor corresponding to the device class
   @define	kUSBDeviceSubClass			The field in the USB Device Descriptor corresponding to the device sub class
   @define	kUSBDeviceProtocol			The field in the USB Device Descriptor corresponding to the device protocol
   @define	kUSBDeviceMaxPacketSize		The field in the USB Device Descriptor corresponding to the maximum packet size for endpoint 0
   @define	kUSBVendorID				The field in the USB Device Descriptor corresponding to the device USB Vendor ID
   @define	kUSBVendorName				Deprecated.  Use kUSBVendorID
   @define	kUSBProductID				The field in the USB Device Descriptor corresponding to the device USB Product ID
   @define	kUSBProductName				Deprecated.  Use kUSBProductID
   @define	kUSBDeviceReleaseNumber		The field in the USB Device Descriptor corresponding to the device release version
   @define	kUSBManufacturerStringIndex	The field in the USB Device Descriptor corresponding to the index for the manufacturer's string
   @define	kUSBProductStringIndex		The field in the USB Device Descriptor corresponding to the index for the product name's string
   @define	kUSBSerialNumberStringIndex	The field in the USB Device Descriptor corresponding to the index for the serial number's string
   @define	kUSBDeviceNumConfigs		The field in the USB Configuration Descriptor corresponding to the number of configurations
   @define	kUSBInterfaceNumber			The field in the USB Configuration Descriptor corresponding to the number of configurations
   @define	kUSBAlternateSetting		The field in the USB Configuration Descriptor corresponding to the number of configurations
   @define	kUSBNumEndpoints			The field in the USB Configuration Descriptor corresponding to the number of configurations
   @define	kUSBInterfaceClass			The field in the USB Interface Descriptor corresponding to the interface class
   @define	kUSBInterfaceSubClass		The field in the USB Interface Descriptor corresponding to the interface sub class
   @define	kUSBInterfaceProtocol		The field in the USB Interface Descriptor corresponding to the interface protocol
   @define	kUSBInterfaceStringIndex	The field in the USB Interface Descriptor corresponding to the index for the interface name's string
   @define	kUSBConfigurationValue		The field in the USB Interface Descriptor corresponding to the configuration
   @define	kUSBProductString			IORegistry key for the device's USB Product string
   @define	kUSBVendorString			IORegistry key for the device's USB manufacturer string
   @define	kUSBSerialNumberString		IORegistry key for the device's USB serial number string
   @define	kUSB1284DeviceID			IORegistry key for the 1284 Device ID of a printer

 */
#define kUSBDeviceClass             kUSBHostMatchingPropertyDeviceClass
#define kUSBDeviceSubClass          kUSBHostMatchingPropertyDeviceSubClass
#define kUSBDeviceProtocol          kUSBHostMatchingPropertyDeviceProtocol
#define kUSBDeviceMaxPacketSize     kUSBHostDevicePropertyMaxPacketSize
#define kUSBVendorID                kUSBHostMatchingPropertyVendorID
#define kUSBVendorName              kUSBVendorID // bad name - keep for backward compatibility
#define kUSBProductID               kUSBHostMatchingPropertyProductID
#define kUSBProductName             kUSBProductID // bad name - keep for backward compatibility
#define kUSBDeviceReleaseNumber     kUSBHostMatchingPropertyDeviceReleaseNumber
#define kUSBManufacturerStringIndex kUSBHostDevicePropertyManufacturerStringIndex
#define kUSBProductStringIndex      kUSBHostDevicePropertyProductStringIndex
#define kUSBSerialNumberStringIndex kUSBHostDevicePropertySerialNumberStringIndex
#define kUSBDeviceNumConfigs        kUSBHostDevicePropertyNumConfigs
#define kUSBInterfaceNumber         kUSBHostMatchingPropertyInterfaceNumber
#define kUSBAlternateSetting        kUSBHostInterfacePropertyAlternateSetting
#define kUSBNumEndpoints            kUSBHostInterfacePropertyNumEndpoints
#define kUSBInterfaceClass          kUSBHostMatchingPropertyInterfaceClass
#define kUSBInterfaceSubClass       kUSBHostMatchingPropertyInterfaceSubClass
#define kUSBInterfaceProtocol       kUSBHostMatchingPropertyInterfaceProtocol
#define kUSBInterfaceStringIndex    kUSBHostInterfacePropertyStringIndex
#define kUSBConfigurationValue      kUSBHostMatchingPropertyConfigurationValue
#define kUSBInterfaceString         kUSBHostInterfacePropertyString
#define kUSB1284DeviceID            "1284 Device ID"

#define kUSBCompatibilityMatch      "USBCompatibilityMatch"
#define kUSBStandardVersion         kUSBHostDevicePropertyStandardVersion
#define kUSBSpecReleaseNumber       kUSBStandardVersion
#define kUSBDeviceContainerID       kUSBContainerID
#define kUSBContainerID             kUSBHostDevicePropertyContainerID

#if !__OPEN_SOURCE__ && TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
#define kUSBProductString           kUSBHostDevicePropertyProductString
#define kUSBVendorString            kUSBHostDevicePropertyVendorString
#define kUSBSerialNumberString      kUSBHostDevicePropertySerialNumberString
#else
#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_14
#define kUSBProductString           "USB Product Name"
#define kUSBVendorString            "USB Vendor Name"
#define kUSBSerialNumberString      "USB Serial Number"
#else
#define kUSBProductString           kUSBHostDevicePropertyProductString
#define kUSBVendorString            kUSBHostDevicePropertyVendorString
#define kUSBSerialNumberString      kUSBHostDevicePropertySerialNumberString
#endif /* MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_14 */
#endif

/*!
   @enum Apple USB Vendor ID
   @discussion Apple's vendor ID, assigned by the USB-IF
 */
enum
{
    kAppleVendorID = kIOUSBAppleVendorID
};

#ifdef __cplusplus
}
#endif

#endif
