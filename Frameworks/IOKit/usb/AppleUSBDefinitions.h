/*
 * Copyright (c) 1998-2019 Apple Inc. All rights reserved.
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

#ifndef AppleUSBCommon_AppleUSBDefinitions_h
#define AppleUSBCommon_AppleUSBDefinitions_h

#include <TargetConditionals.h>
#if TARGET_OS_DRIVERKIT
#include <stdint.h>
#include <stddef.h>
#include <sys/_types/_uuid_t.h>
#include <DriverKit/IOTypes.h>
#include <DriverKit/IOLib.h>
#else
#include <IOKit/IOTypes.h>
#include <libkern/OSByteOrder.h>
#endif

#ifndef IOUSBBit
#define IOUSBBit(bit)                     ((uint32_t)(1) << bit)
#endif

#ifndef IOUSBBitRange
#define IOUSBBitRange(start, end)         (~(((uint32_t)(1) << start) - 1) & (((uint32_t)(1) << end) | (((uint32_t)(1) << end) - 1)))
#endif

#ifndef IOUSBBitRange64
#define IOUSBBitRange64(start, end)       (~(((uint64_t)(1) << start) - 1) & (((uint64_t)(1) << end) | (((uint64_t)(1) << end) - 1)))
#endif

#ifndef IOUSBBitRangePhase
#define IOUSBBitRangePhase(start, end)    (start)
#endif

#ifndef USBToHost16
#define USBToHost16 OSSwapLittleToHostInt16
#endif

#ifndef HostToUSB16
#define HostToUSB16 OSSwapHostToLittleInt16
#endif

#ifndef USBToHost32
#define USBToHost32 OSSwapLittleToHostInt32
#endif

#ifndef HostToUSB32
#define HostToUSB32 OSSwapHostToLittleInt32
#endif

#ifndef USBToHost64
#define USBToHost64 OSSwapLittleToHostInt64
#endif

#ifndef HostToUSB64
#define HostToUSB64 OSSwapHostToLittleInt64
#endif

#define kIOUSB30Bitrate5Gbps  ( 5 * 1000 * 1000 * 1000ULL)
#define kIOUSB30Bitrate10Gbps (10 * 1000 * 1000 * 1000ULL)
#define kIOUSB32Bitrate20Gbps (20 * 1000 * 1000 * 1000ULL)

/*!
 * @enum       kIOUSBAppleVendorID
 * @discussion Apple's vendor ID, assigned by the USB-IF
 */
enum
{
    kIOUSBAppleVendorID = 0x05AC
};

#pragma mark Descriptor definitions

/*!
 * @enum  tIOUSBDescriptorType
 * @brief Descriptor types defined by USB 2.0 Table 9-5 and USB 3.0 Table 9-6
 */
enum tIOUSBDescriptorType
{
    kIOUSBDescriptorTypeDevice                                     = 1,
    kIOUSBDescriptorTypeConfiguration                              = 2,
    kIOUSBDescriptorTypeString                                     = 3,
    kIOUSBDescriptorTypeInterface                                  = 4,
    kIOUSBDescriptorTypeEndpoint                                   = 5,
    kIOUSBDescriptorTypeDeviceQualifier                            = 6,
    kIOUSBDescriptorTypeOtherSpeedConfiguration                    = 7,
    kIOUSBDescriptorTypeInterfacePower                             = 8,
    kIOUSBDescriptorTypeOTG                                        = 9,
    kIOUSBDescriptorTypeDebug                                      = 10,
    kIOUSBDescriptorTypeInterfaceAssociation                       = 11,
    kIOUSBDescriptorTypeBOS                                        = 15,
    kIOUSBDescriptorTypeDeviceCapability                           = 16,
    kIOUSBDecriptorTypeHID                                         = 33,
    kIOUSBDecriptorTypeReport                                      = 34,
    kIOUSBDescriptorTypePhysical                                   = 35,
    kIOUSBDescriptorTypeHub                                        = 41,
    kIOUSBDescriptorTypeSuperSpeedHub                              = 42,
    kIOUSBDescriptorTypeSuperSpeedUSBEndpointCompanion             = 48,
    kIOUSBDescriptorTypeSuperSpeedPlusIsochronousEndpointCompanion = 49
};

typedef enum tIOUSBDescriptorType tIOUSBDescriptorType;

/*!
 * @enum        tIOUSBDescriptorSize
 * @brief       Size in bytes for descriptor structures
 */
enum tIOUSBDescriptorSize
{
    kIOUSBDescriptorHeaderSize                                     = 2,
    kIOUSBDescriptorSizeDevice                                     = 18,
    kIOUSBDescriptorSizeConfiguration                              = 9,
    kIOUSBDescriptorSizeInterface                                  = 9,
    kIOUSBDescriptorSizeEndpoint                                   = 7,
    kIOUSBDescriptorSizeStringMinimum                              = kIOUSBDescriptorHeaderSize,
    kIOUSBDescriptorSizeStringMaximum                              = 255,
    kIOUSBDescriptorSizeDeviceQualifier                            = 10,
    kIOUSBDescriptorSizeInterfaceAssociation                       = 8,
    kIOUSBDescriptorSizeBOS                                        = 5,
    kIOUSBDescriptorSizeDeviceCapability                           = 3,
    kIOUSBDescriptorSizeUSB20ExtensionCapability                   = 7,
    kIOUSBDescriptorSizeSuperSpeedUSBDeviceCapability              = 10,
    kIOUSBDescriptorSizeContainerIDCapability                      = 20,
    kIOUSBDescriptorSizeHubMinimum                                 = 9,
    kIOUSBDescriptorSizeHubMaximum                                 = 21,
    kIOUSBDescriptorSizeSuperSpeedHub                              = 12,
    kIOUSBDescriptorSizeSuperSpeedUSBEndpointCompanion             = 6,
    kIOUSBDescriptorSizeSuperSpeedPlusIsochronousEndpointCompanion = 8,
    kIOUSBDescriptorSizeBillboardDeviceMinimum                     = 44,
    kIOUSBDescriptorSizeBillboardDeviceMaximum                     = 256,
    kIOUSBDescriptorSizePlatformECIDCapability                     = 28,
    kIOUSBDescriptorSizePlatformCapability                         = 20
};

typedef enum tIOUSBDescriptorSize tIOUSBDescriptorSize;

/*!
 * @struct      IOUSBDescriptorHeader
 * @brief       Base descriptor defined by USB 2.0 9.5
 * @discussion  IOUSBDescriptorDefinitions declares structs to represent a variety of USB standard
 *              descriptors.
 */
struct IOUSBDescriptorHeader
{
    uint8_t bLength;
    uint8_t bDescriptorType;
} __attribute__((packed));

typedef struct IOUSBDescriptorHeader IOUSBDescriptorHeader;

/*!
 * @brief       Base descriptor defined by USB 2.0 9.5
 * @discussion  Used to represent generic descriptor definitions
 */
typedef IOUSBDescriptorHeader IOUSBDescriptor;

/*!
 * @typedef    IOUSBDeviceDescriptor
 * @discussion Descriptor for a USB Device.
 *             See the USB Specification at <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 */
struct IOUSBDeviceDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
} __attribute__((packed));

typedef struct IOUSBDeviceDescriptor IOUSBDeviceDescriptor;

/*!
 * @typedef    IOUSBDeviceQualifierDescriptor
 * @discussion USB Device Qualifier Descriptor.
 *             See the USB Specification at <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 *             USB 2.0 9.6.2: Device Qualifier
 */
struct IOUSBDeviceQualifierDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint8_t  bNumConfigurations;
    uint8_t  bReserved;
} __attribute__((packed));

typedef struct IOUSBDeviceQualifierDescriptor IOUSBDeviceQualifierDescriptor;

/*!
 * @typedef    IOUSBConfigurationDescHeader
 * @discussion Header of a IOUSBConfigurationDescriptor.  Used to get the total length of the descriptor.
 *             USB 2.0 9.6.3: Configuration
 */
struct IOUSBConfigurationDescHeader
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
} __attribute__((packed));

typedef struct IOUSBConfigurationDescHeader IOUSBConfigurationDescHeader;

/*!
 * @typedef    IOUSBConfigurationDescriptor
 * @discussion Standard USB Configuration Descriptor.  It is variable length, so this only specifies
 *             the known fields.  We use the wTotalLength field to read the whole descriptor.
 *             See the USB Specification at <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 *             USB 2.0 9.6.3: Configuration
 */
struct IOUSBConfigurationDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  MaxPower;
} __attribute__((packed));

typedef struct IOUSBConfigurationDescriptor IOUSBConfigurationDescriptor;


enum
{
    kIOUSBConfigurationDescriptorAttributeRemoteWakeCapable = IOUSBBit(5),
    kIOUSBConfigurationDescriptorAttributeSelfPowered       = IOUSBBit(6)
};


/*!
 * @typedef    IOUSBInterfaceDescriptor
 * @discussion Descriptor for a USB Interface.  See the USB Specification at
 *             <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 *             USB 2.0 9.6.5: Interface
 */
struct IOUSBInterfaceDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} __attribute__((packed));

typedef struct IOUSBInterfaceDescriptor IOUSBInterfaceDescriptor;


/*!
 * @typedef    IOUSBEndpointDescriptor
 * @discussion Descriptor for a USB Endpoint.  See the USB Specification at
 *             <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 *             USB 2.0 9.6.6: Endpoint
 */
struct IOUSBEndpointDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t  bInterval;
} __attribute__((packed));

typedef struct IOUSBEndpointDescriptor IOUSBEndpointDescriptor;

enum
{
    kIOUSBEndpointDescriptorNumber                  = IOUSBBitRange(0, 3),
    kIOUSBEndpointDescriptorEndpointAddressReserved = IOUSBBitRange(4, 6),
    kIOUSBEndpointDescriptorDirection               = IOUSBBit(7),
    kIOUSBEndpointDescriptorDirectionPhase          = IOUSBBitRangePhase(7, 7),
    kIOUSBEndpointDescriptorDirectionOut            = 0,
    kIOUSBEndpointDescriptorDirectionIn             = IOUSBBit(7),

    kIOUSBEndpointDescriptorTransferType                    = IOUSBBitRange(0, 1),
    kIOUSBEndpointDescriptorTransferTypePhase               = IOUSBBitRangePhase(0, 1),
    kIOUSBEndpointDescriptorTransferTypeControl             = (0 << IOUSBBitRangePhase(0, 1)),
    kIOUSBEndpointDescriptorTransferTypeIsochronous         = (1 << IOUSBBitRangePhase(0, 1)),
    kIOUSBEndpointDescriptorTransferTypeBulk                = (2 << IOUSBBitRangePhase(0, 1)),
    kIOUSBEndpointDescriptorTransferTypeInterrupt           = (3 << IOUSBBitRangePhase(0, 1)),
    kIOUSBEndpointDescriptorSynchronizationType             = IOUSBBitRange(2, 3),
    kIOUSBEndpointDescriptorSynchronizationTypePhase        = IOUSBBitRangePhase(2, 3),
    kIOUSBEndpointDescriptorSynchronizationTypeNone         = (0 << IOUSBBitRangePhase(2, 3)),
    kIOUSBEndpointDescriptorSynchronizationTypeAsynchronous = (1 << IOUSBBitRangePhase(2, 3)),
    kIOUSBEndpointDescriptorSynchronizationTypeAdaptive     = (2 << IOUSBBitRangePhase(2, 3)),
    kIOUSBEndpointDescriptorSynchronizationTypeSynchronous  = (3 << IOUSBBitRangePhase(2, 3)),
    kIOUSBEndpointDescriptorUsageType                       = IOUSBBitRange(4, 5),
    kIOUSBEndpointDescriptorUsageTypePhase                  = IOUSBBitRangePhase(4, 5),
    kIOUSBEndpointDescriptorUsageTypeInterruptPeriodic      = (0 << IOUSBBitRangePhase(4, 5)),
    kIOUSBEndpointDescriptorUsageTypeInterruptNotification  = (1 << IOUSBBitRangePhase(4, 5)),
    kIOUSBEndpointDescriptorUsageTypeInterruptReserved1     = (2 << IOUSBBitRangePhase(4, 5)),
    kIOUSBEndpointDescriptorUsageTypeInterruptReserved2     = (3 << IOUSBBitRangePhase(4, 5)),
    kIOUSBEndpointDescriptorUsageTypeIsocData               = (0 << IOUSBBitRangePhase(4, 5)),
    kIOUSBEndpointDescriptorUsageTypeIsocFeedback           = (1 << IOUSBBitRangePhase(4, 5)),
    kIOUSBEndpointDescriptorUsageTypeIsocImplicit           = (2 << IOUSBBitRangePhase(4, 5)),
    kIOUSBEndpointDescriptorUsageTypeIsocReserved           = (3 << IOUSBBitRangePhase(4, 5)),

    kIOUSBEndpointDescriptorPacketSize          = IOUSBBitRange(0, 10),
    kIOUSBEndpointDescriptorPacketSizePhase     = IOUSBBitRangePhase(0, 10),
    kIOUSBEndpointDescriptorPacketSizeMult      = IOUSBBitRange(11, 12),
    kIOUSBEndpointDescriptorPacketSizeMultPhase = IOUSBBitRangePhase(11, 12),
    kIOUSBEndpointDescriptorReserved            = IOUSBBitRange(13, 15),
    kIOUSBEndpointDescriptorReservedPhase       = IOUSBBitRangePhase(13, 15)
};

/*!
 * @enum        tIOUSBEndpointDirection
 * @brief       IO Direction of an endpoint
 * @constant    kIOUSBEndpointDirectionOut endpoint direction is host-to-device
 * @constant    kIOUSBEndpointDirectionIn  endpoint direction is device-to-host
 * @constant    kIOUSBEndpointDirectionUnknown the direction is unknown
 */
enum tIOUSBEndpointDirection
{
    kIOUSBEndpointDirectionOut     = (kIOUSBEndpointDescriptorDirectionOut >> kIOUSBEndpointDescriptorDirectionPhase),
    kIOUSBEndpointDirectionIn      = (kIOUSBEndpointDescriptorDirectionIn >> kIOUSBEndpointDescriptorDirectionPhase),
    kIOUSBEndpointDirectionUnknown = 2
};

typedef enum tIOUSBEndpointDirection tIOUSBEndpointDirection;

/*!
 * @enum        tIOUSBEndpointType
 * @brief       Describes the type of endpoint
 * @constant    kIOUSBEndpointTypeControl endpoint is a control endpoint
 * @constant    kIOUSBEndpointTypeIsochronous endpoint is an isochronous endpoint
 * @constant    kIOUSBEndpointTypeBulk is a bulk endpoint
 * @constant    kIOUSBEndpointTypeInterrupt is an interrupt endpoint
 */
enum tIOUSBEndpointType
{
    kIOUSBEndpointTypeControl     = (kIOUSBEndpointDescriptorTransferTypeControl >> kIOUSBEndpointDescriptorTransferTypePhase),
    kIOUSBEndpointTypeIsochronous = (kIOUSBEndpointDescriptorTransferTypeIsochronous >> kIOUSBEndpointDescriptorTransferTypePhase),
    kIOUSBEndpointTypeBulk        = (kIOUSBEndpointDescriptorTransferTypeBulk >> kIOUSBEndpointDescriptorTransferTypePhase),
    kIOUSBEndpointTypeInterrupt   = (kIOUSBEndpointDescriptorTransferTypeInterrupt >> kIOUSBEndpointDescriptorTransferTypePhase)
};

typedef enum tIOUSBEndpointType tIOUSBEndpointType;


/*!
 * @enum  tIOUSBLanguageID
 * @brief USB Language Identifiers 1.0
 */
enum tIOUSBLanguageID
{
    kIOUSBLanguageIDEnglishUS = 0x0409
};

typedef enum tIOUSBLanguageID tIOUSBLanguageID;

/*!
 * @typedef    IOUSBStringDescriptor
 * @discussion Descriptor for a USB Strings.  See the USB Specification at
 *             <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 *             USB 2.0 9.6.7: String
 */
struct IOUSBStringDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bString[1];
} __attribute__((packed));

typedef struct IOUSBStringDescriptor IOUSBStringDescriptor;

enum tIOUSBDeviceCapabilityType
{
    // USB 3.0 Table 9-13
    kIOUSBDeviceCapabilityTypeWireless       = 1,
    kIOUSBDeviceCapabilityTypeUSB20Extension = 2,
    kIOUSBDeviceCapabilityTypeSuperSpeed     = 3,
    kIOUSBDeviceCapabilityTypeContainerID    = 4,

    // USB 3.1 Table 9-14
    kIOUSBDeviceCapabilityTypePlatform             = 5,
    kIOUSBDeviceCapabilityTypePowerDelivery        = 6,
    kIOUSBDeviceCapabilityTypeBatteryInfo          = 7,
    kIOUSBDeviceCapabilityTypePdConsumerPort       = 8,
    kIOUSBDeviceCapabilityTypePdProviderPort       = 9,
    kIOUSBDeviceCapabilityTypeSuperSpeedPlus       = 10,
    kIOUSBDeviceCapabilityTypePrecisionMeasurement = 11,
    kIOUSBDeviceCapabilityTypeWirelessExt          = 12,
    kIOUSBDeviceCapabilityTypeBillboard            = 13,
    kIOUSBDeviceCapabilityTypeBillboardAltMode     = 15
};

typedef enum tIOUSBDeviceCapabilityType tIOUSBDeviceCapabilityType;

/*!
 * @typedef    IOUSBBOSDescriptor
 * @discussion USB BOS descriptor. See the USB Specification at
 *             <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 *             USB 3.0 9.6.2: Binary Device Object Store (BOS)
 */
struct IOUSBBOSDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumDeviceCaps;
} __attribute__((packed));

typedef struct IOUSBBOSDescriptor IOUSBBOSDescriptor;


/*!
 * @typedef    IOUSBDeviceCapabilityDescriptorHeader
 * @discussion USB BOS descriptor.  See the USB Specification at
 *             <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 *             USB 3.0 9.6.2: Binary Device Object Store (BOS)
 */
struct IOUSBDeviceCapabilityDescriptorHeader
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDevCapabilityType;
    uint8_t bNumDeviceCaps;
} __attribute__((packed));

typedef struct IOUSBDeviceCapabilityDescriptorHeader IOUSBDeviceCapabilityDescriptorHeader;

/*!
 * @typedef    IOUSBDeviceCapabilityUSB2Extension
 * @discussion Device Capability USB 2.0 Extension.
 *             USB 3.0 9.6.2.1: USB 2.0 Extension
 */
struct IOUSBDeviceCapabilityUSB2Extension
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDevCapabilityType;
    uint32_t bmAttributes;
} __attribute__((packed));

typedef struct IOUSBDeviceCapabilityUSB2Extension IOUSBDeviceCapabilityUSB2Extension;


enum
{
    kIOUSBUSB20ExtensionCapabilityLPM = IOUSBBit(1),

    // From USB 2.0 ECN Errata for Link Power Management.
    kIOUSBUSB20ExtensionCapabilityBESLSupport = IOUSBBit(2),
    kIOUSBUSB20ExtensionCapabilityBESLValid   = IOUSBBit(3),
    kIOUSBUSB20ExtensionCapabilityBESLDValid  = IOUSBBit(4),
    kIOUSBUSB20ExtensionCapabilityBESL        = IOUSBBitRange(8, 11),
    kIOUSBUSB20ExtensionCapabilityBESLPhase   = IOUSBBitRangePhase(8, 11),
    kIOUSBUSB20ExtensionCapabilityBESLD       = IOUSBBitRange(12, 15),
    kIOUSBUSB20ExtensionCapabilityBESLDPhase  = IOUSBBitRangePhase(12, 15)
};


/*!
 * @typedef    IOUSBDeviceCapabilitySuperSpeedUSB
 * @discussion Device Capability SuperSpeed USB. USB 3.0 9.6.2.2: SuperSpeed USB Device Capability
 */
struct IOUSBDeviceCapabilitySuperSpeedUSB
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDevCapabilityType;
    uint8_t  bmAttributes;
    uint16_t wSpeedsSupported;
    uint8_t  bFunctionalitySupport;
    uint8_t  bU1DevExitLat;
    uint16_t wU2DevExitLat;
} __attribute__((packed));

typedef struct IOUSBDeviceCapabilitySuperSpeedUSB IOUSBDeviceCapabilitySuperSpeedUSB;


enum
{
    kIOUSBSuperSpeedDeviceCapabilityLTM = IOUSBBit(1),

    kIOUSBSuperSpeedDeviceCapabilityLowSpeed  = IOUSBBit(0),
    kIOUSBSuperSpeedDeviceCapabilityFullSpeed = IOUSBBit(1),
    kIOUSBSuperSpeedDeviceCapabilityHighSpeed = IOUSBBit(2),
    kIOUSBSuperSpeedDeviceCapability5Gb       = IOUSBBit(3),

    kIOUSBSuperSpeedDeviceCapabilitySupportLowSpeed  = 0,
    kIOUSBSuperSpeedDeviceCapabilitySupportFullSpeed = 1,
    kIOUSBSuperSpeedDeviceCapabilitySupportHighSpeed = 2,
    kIOUSBSuperSpeedDeviceCapabilitySupport5Gb       = 3,

    kIOUSBSuperSpeedDeviceCapabilityU1DevExitLatMax = 0xa,
    kIOUSBSuperSpeedDeviceCapabilityU2DevExitLatMax = 0x7ff
};


/*!
 * @typedef    IOUSBDeviceCapabilitySuperSpeedPlusUSB
 * @discussion Device Capability SuperSpeedPlus USB.
 *             USB 3.1 9.6.2.5: SuperSpeedPlus USB Device Capability
 */
struct IOUSBDeviceCapabilitySuperSpeedPlusUSB
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDevCapabilityType;
    uint8_t  bReserved;
    uint32_t bmAttributes;
    uint16_t wFunctionalitySupport;
    uint16_t wReserved;
    uint32_t bmSublinkSpeedAttr[1];
} __attribute__((packed));

typedef struct IOUSBDeviceCapabilitySuperSpeedPlusUSB IOUSBDeviceCapabilitySuperSpeedPlusUSB;

enum
{
    //bmAttributes
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSpeedAttrCount      = IOUSBBitRange(0, 4),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSpeedAttrCountPhase = IOUSBBitRangePhase(0, 4),

    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSpeedIdCount      = IOUSBBitRange(5, 8),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSpeedIdCountPhase = IOUSBBitRangePhase(5, 8),

    //wFunctionalitySupport
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkMinSpeedId      = IOUSBBitRange(0, 3),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkMinSpeedIdPhase = IOUSBBitRangePhase(0, 3),

    kIOUSBSuperSpeedPlusDeviceCapabilityReserved      = IOUSBBitRange(4, 7),
    kIOUSBSuperSpeedPlusDeviceCapabilityReservedPhase = IOUSBBitRangePhase(4, 7),

    kIOUSBSuperSpeedPlusDeviceCapabilityMinRxLaneCount      = IOUSBBitRange(8, 11),
    kIOUSBSuperSpeedPlusDeviceCapabilityMinRxLaneCountPhase = IOUSBBitRangePhase(8, 11),

    kIOUSBSuperSpeedPlusDeviceCapabilityMinTxLaneCount      = IOUSBBitRange(12, 15),
    kIOUSBSuperSpeedPlusDeviceCapabilityMinTxLaneCountPhase = IOUSBBitRangePhase(12, 15),

    //bmSublinkSpeedAttr
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSpeedId      = IOUSBBitRange(0, 3),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSpeedIdPhase = IOUSBBitRangePhase(0, 3),

    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkLSE      = IOUSBBitRange(4, 5),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkLSEPhase = IOUSBBitRangePhase(4, 5),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkLSEBits  = (0 << kIOUSBSuperSpeedPlusDeviceCapabilitySublinkLSEPhase),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkLSEKbits = (1 << kIOUSBSuperSpeedPlusDeviceCapabilitySublinkLSEPhase),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkLSEMbits = (2 << kIOUSBSuperSpeedPlusDeviceCapabilitySublinkLSEPhase),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkLSEGbits = (3 << kIOUSBSuperSpeedPlusDeviceCapabilitySublinkLSEPhase),

    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkType      = IOUSBBitRange(6, 7),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkTypePhase = IOUSBBitRangePhase(6, 7),

    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSymmetry      = IOUSBBit(6),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSymmetryPhase = IOUSBBitRangePhase(6, 6),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSymmetric     = (0 << kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSymmetryPhase),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkAsymmetric    = (1 << kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSymmetryPhase),

    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkDirection      = IOUSBBit(7),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkDirectionPhase = IOUSBBitRangePhase(7, 7),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkDirectionRx    = (0 << kIOUSBSuperSpeedPlusDeviceCapabilitySublinkDirectionPhase),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkDirectionTx    = (1 << kIOUSBSuperSpeedPlusDeviceCapabilitySublinkDirectionPhase),

    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkReserved      = IOUSBBitRange(8, 13),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkReservedPhase = IOUSBBitRangePhase(8, 13),

    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkProtocol      = IOUSBBitRange(14, 15),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkProtocolPhase = IOUSBBitRangePhase(14, 15),

    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSpeedMantissa      = IOUSBBitRange(16, 31),
    kIOUSBSuperSpeedPlusDeviceCapabilitySublinkSpeedMantissaPhase = IOUSBBitRangePhase(16, 31),
};


/*!
 * @typedef    IOUSBDeviceCapabilityContainerID
 * @discussion Device Capability Container ID.
 *             USB 3.0 9.6.2.3: Container ID
 */
struct IOUSBDeviceCapabilityContainerID
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDevCapabilityType;
    uint8_t bReservedID;
    uint8_t containerID[16];
} __attribute__((packed));

typedef struct IOUSBDeviceCapabilityContainerID IOUSBDeviceCapabilityContainerID;


/*!
 * @typedef    IOUSBPlatformCapabilityDescriptor
 * @discussion Device Capability Platform Descriptor.
 *             USB 3.1 9.6.2.4: Platform Descriptor
 */
struct IOUSBPlatformCapabilityDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDevCapabilityType;
    uint8_t bReserved;
    uuid_t  PlatformCapabilityUUID;
} __attribute__((packed));

typedef struct IOUSBPlatformCapabilityDescriptor IOUSBPlatformCapabilityDescriptor;


/*!
 * @typedef    IOUSBDeviceCapabilityBillboardAltConfig
 * @discussion Device Capability Billboard Alternate Setting Info.
 *             USB Billboard 3.1.6.2: Billboard Capability Descriptor V1.2
 */
struct IOUSBDeviceCapabilityBillboardAltConfigCompatibility
{
    uint16_t wSVID;
    uint32_t dwAlternateMode;
    uint8_t  iAlternateModeString;
} __attribute__((packed));

typedef struct IOUSBDeviceCapabilityBillboardAltConfigCompatibility IOUSBDeviceCapabilityBillboardAltConfigCompatibility;

/*!
 * @typedef    IOUSBDeviceCapabilityBillboardAltConfig
 * @discussion Device Capability Billboard Alternate Setting Info.
 *             USB Billboard 3.1.6.2: Billboard Capability Descriptor V1.1 and 1.21+
 */
struct IOUSBDeviceCapabilityBillboardAltConfig
{
    uint16_t wSVID;
    uint8_t  bAltenateMode;
    uint8_t  iAlternateModeString;
} __attribute__((packed));

typedef struct IOUSBDeviceCapabilityBillboardAltConfig IOUSBDeviceCapabilityBillboardAltConfig;

/*!
 * @typedef    IOUSBDeviceCapabilityBillboard
 * @discussion Device Capability Billboard.
 *             USB Billboard 3.1.6.2: Billboard Capability Descriptor
 */
struct IOUSBDeviceCapabilityBillboard
{
    uint8_t                                 bLength;
    uint8_t                                 bDescriptorType;
    uint8_t                                 bDevCapabilityType;
    uint8_t                                 iAdditionalInfoURL;
    uint8_t                                 bNumberOfAlternateModes;
    uint8_t                                 bPreferredAlternateMode;
    uint16_t                                vCONNPower;
    uint8_t                                 bmConfigured[32];
    uint16_t                                bcdVersion;
    uint8_t                                 bAdditionalFailureInfo;
    uint8_t                                 bReserved;
    IOUSBDeviceCapabilityBillboardAltConfig pAltConfigurations[];
} __attribute__((packed));

typedef struct IOUSBDeviceCapabilityBillboard IOUSBDeviceCapabilityBillboard;

/*!
 * @typedef    IOUSBDeviceCapabilityBillboardAltMode
 * @discussion Device Capability Billboard Alternate mode.
 *             USB Billboard 3.1.6.3: Billboard Capability Descriptor V1.21
 */
struct IOUSBDeviceCapabilityBillboardAltMode
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDevCapabilityType;
    uint8_t  bIndex;
    uint16_t dwAlternateModeVdo;
} __attribute__((packed));

typedef struct IOUSBDeviceCapabilityBillboardAltMode IOUSBDeviceCapabilityBillboardAltMode;

/*!
 * @typedef    IOUSBInterfaceAssociationDescriptor
 * @discussion USB Inerface Association Descriptor.  ECN to the USB 2.0 Spec.
 *             See the USB Specification at <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 *             USB 3.0 9.6.4: Interface Association
 */
struct IOUSBInterfaceAssociationDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bFirstInterface;
    uint8_t bInterfaceCount;
    uint8_t bFunctionClass;
    uint8_t bFunctionSubClass;
    uint8_t bFunctionProtocol;
    uint8_t iFunction;
} __attribute__((packed));

typedef struct IOUSBInterfaceAssociationDescriptor IOUSBInterfaceAssociationDescriptor;


/*!
 * @typedef    IOUSBSuperSpeedEndpointCompanionDescriptor
 * @discussion Descriptor for a SuperSpeed USB Endpoint Companion.
 *             See the USB Specification at <a href="http://www.usb.org"TARGET="_blank">http://www.usb.org</a>.
 *             USB 3.1 9.6.7: SuperSpeed Endpoint Companion
 */
struct IOUSBSuperSpeedEndpointCompanionDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bMaxBurst;
    uint8_t  bmAttributes;
    uint16_t wBytesPerInterval;
} __attribute__((packed));

typedef struct IOUSBSuperSpeedEndpointCompanionDescriptor IOUSBSuperSpeedEndpointCompanionDescriptor;


enum
{
    kIOUSBSuperSpeedEndpointCompanionDescriptorMaxBurst      = IOUSBBitRange(0, 4),
    kIOUSBSuperSpeedEndpointCompanionDescriptorMaxBurstPhase = IOUSBBitRangePhase(0, 4),

    kIOUSBSuperSpeedEndpointCompanionDescriptorBulkMaxStreams      = IOUSBBitRange(0, 4),
    kIOUSBSuperSpeedEndpointCompanionDescriptorBulkMaxStreamsPhase = IOUSBBitRangePhase(0, 4),
    kIOUSBSuperSpeedEndpointCompanionDescriptorBulkReserved        = IOUSBBitRange(5, 7),
    kIOUSBSuperSpeedEndpointCompanionDescriptorBulkReservedPhase   = IOUSBBitRangePhase(5, 7),
    kIOUSBSuperSpeedEndpointCompanionDescriptorIsocMult            = IOUSBBitRange(0, 1),
    kIOUSBSuperSpeedEndpointCompanionDescriptorIsocMultPhase       = IOUSBBitRangePhase(0, 1),
    kIOUSBSuperSpeedEndpointCompanionDescriptorIsocReserved        = IOUSBBitRange(2, 6),
    kIOUSBSuperSpeedEndpointCompanionDescriptorIsocReservedPhase   = IOUSBBitRangePhase(2, 6),
    kIOUSBSuperSpeedEndpointCompanionDescriptorSSPIsocCompanion    = IOUSBBit(7)
};

/*!
 * @typedef    IOUSBSuperSpeedPlusIsochronousEndpointCompanionDescriptor
 * @discussion Descriptor for a SuperSpeedPlus Isochronout USB Endpoint Companion.
 *             See the USB Specification at <a href="http://www.usb.org"TARGET="_blank">http://www.usb.org</a>.
 *             USB 3.1 9.6.8: SuperSpeedPlus Isochronous Endpoint Companion
 */
struct IOUSBSuperSpeedPlusIsochronousEndpointCompanionDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wReserved;
    uint32_t dwBytesPerInterval;
} __attribute__((packed));

typedef struct IOUSBSuperSpeedPlusIsochronousEndpointCompanionDescriptor IOUSBSuperSpeedPlusIsochronousEndpointCompanionDescriptor;



/*!
 * @typedef    IOUSB20HubDescriptor
 * @discussion Descriptor for a USB hub.
 *             See the USB Specification at <a href="http://www.usb.org"TARGET="_blank">http://www.usb.org</a>.
 *             USB 2.0 11.23.2.1: Hub Descriptor
 */
struct IOUSB20HubDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bNumberPorts;
    uint16_t wHubCharacteristics;
    uint8_t  bPowerOnToPowerGood;
    uint8_t  bHubControllerCurrent;
    uint8_t  deviceRemovable[2];    // Technically variable size
    uint8_t  reserved[2];           // Unused
} __attribute__((packed));

typedef struct IOUSB20HubDescriptor IOUSB20HubDescriptor;


/*!
 * @typedef    IOUSBSuperSpeedHubDescriptor
 * @discussion Descriptor for a Super Speed USB hub.
 *             See the USB Specification at <a href="http://www.usb.org"TARGET="_blank">http://www.usb.org</a>.
 *             USB 3.0 10.13.2.1: SuperSpeed Hub Descriptor
 */
struct IOUSBSuperSpeedHubDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bNumberPorts;
    uint16_t wHubCharacteristics;
    uint8_t  bPowerOnToPowerGood;
    uint8_t  bHubControllerCurrent;
    uint8_t  bHubDecodeLatency;
    uint16_t wHubDelay;
    uint16_t deviceRemovable;
} __attribute__((packed));

typedef struct IOUSBSuperSpeedHubDescriptor IOUSBSuperSpeedHubDescriptor;


enum
{
    kIOUSBSuperSpeedHubCharacteristicsPowerSwitchingMask       = IOUSBBitRange(0, 1),
    kIOUSBSuperSpeedHubCharacteristicsPowerSwitchingGanged     = (0 << IOUSBBitRangePhase(0, 1)),
    kIOUSBSuperSpeedHubCharacteristicsPowerSwitchingIndividual = (1 << IOUSBBitRangePhase(0, 1)),
    kIOUSBSuperSpeedHubCharacteristicsCompoundDevice           = IOUSBBit(2),
    kIOUSBSuperSpeedHubCharacteristicsOverCurrentMask          = IOUSBBitRange(3, 4),
    kIOUSBSuperSpeedHubCharacteristicsOverCurrentGlobal        = (0 << IOUSBBitRangePhase(3, 4)),
    kIOUSBSuperSpeedHubCharacteristicsOverCurrentIndividual    = (1 << IOUSBBitRangePhase(3, 4)),
    kIOUSBSuperSpeedHubCharacteristicsReserved                 = IOUSBBitRange(5, 15),

    kIOUSBSuperSpeedHubDecodeLatencyMax = 10,
    kIOUSBSuperSpeedHubDelayMax         = 400
};

/*!
 * @typedef     UASPipeDescriptor
 * @discussion  Structure used to specify the Mass Storage Specific UAS pipe usage descriptor
 */
struct UASPipeDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bPipeID;
    uint8_t bReserved;
} __attribute__((packed));

typedef struct  UASPipeDescriptor UASPipeDescriptor;

/*!
 * @typedef    IOUSBHIDDescriptor
 * @discussion USB HID Descriptor.  See the USB HID Specification at
 *             <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 */
struct IOUSBHIDDescriptor
{
    uint8_t  descLen;
    uint8_t  descType;
    uint16_t descVersNum;
    uint8_t  hidCountryCode;
    uint8_t  hidNumDescriptors;
    uint8_t  hidDescriptorType;
    uint8_t  hidDescriptorLengthLo;
    uint8_t  hidDescriptorLengthHi;
} __attribute__((packed));

typedef struct IOUSBHIDDescriptor IOUSBHIDDescriptor;

/*!
 * @typedef    IOUSBHIDReportDesc
 * @discussion USB HID Report Descriptor header.  See the USB HID Specification at
 *             <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 */
struct IOUSBHIDReportDesc
{
    uint8_t hidDescriptorType;
    uint8_t hidDescriptorLengthLo;
    uint8_t hidDescriptorLengthHi;
} __attribute__((packed));

typedef struct IOUSBHIDReportDesc IOUSBHIDReportDesc;

/*!
 * @typedef    IOUSBDFUDescriptor
 * @discussion USB Device Firmware Update Descriptor. See the USB Device Firmware Update
 *             Specification at <a href="http://www.usb.org" target="_blank">http://www.usb.org</a>.
 */
struct IOUSBDFUDescriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bmAttributes;
    uint16_t wDetachTimeout;
    uint16_t wTransferSize;
} __attribute__((packed));

typedef struct  IOUSBDFUDescriptor IOUSBDFUDescriptor;
#pragma mark Device requests
/*!
 * @typedef    IOUSBDeviceRequest
 * @discussion Standard device request.
 *             See the USB Specification at <a href="http://www.usb.org"TARGET="_blank">http://www.usb.org</a>.
 *             USB 2.0 9.3: USB Device Requests
 */
struct IOUSBDeviceRequest
{
    uint8_t  bmRequestType;
    uint8_t  bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __attribute__((packed));

typedef struct IOUSBDeviceRequest IOUSBDeviceRequest;

/*!
 * @typedef    IOUSBDeviceRequestSetSELData
 * @discussion See the USB Specification at <a href="http://www.usb.org"TARGET="_blank">http://www.usb.org</a>.
 *             USB 3.0 9.4.12: Set SEL Standard Device Request
 */
struct IOUSBDeviceRequestSetSELData
{
    uint8_t  u1Sel;                 // Time in μs for U1 System Exit Latency
    uint8_t  u1Pel;                 // Time in μs for U1 Device to Host Exit Latency
    uint16_t u2Sel;                 // Time in μs for U2 System Exit Latency
    uint16_t u2Pel;                 // Time in μs for U2 Device to Host Exit Latency
} __attribute__((packed));

typedef struct IOUSBDeviceRequestSetSELData IOUSBDeviceRequestSetSELData;

/*!
 * @enum        tIOUSBDeviceRequestDirectionValue
 * @brief       Values of 'direction' field of a device request
 * @constant    kIOUSBDeviceRequestDirectionValueOut the device request data phase direction is host-to-device
 * @constant    kIOUSBDeviceRequestDirectionValueIn the device request data phase direction is device-to-host
 */
enum tIOUSBDeviceRequestDirectionValue
{
    kIOUSBDeviceRequestDirectionValueOut = 0,
    kIOUSBDeviceRequestDirectionValueIn  = 1
};

typedef enum tIOUSBDeviceRequestDirectionValue tIOUSBDeviceRequestDirectionValue;

/*!
 * @enum        tIOUSBDeviceRequestType
 * @brief       Values of 'type' field of a device request
 * @constant    kIOUSBDeviceRequestTypeValueStandard the device request is a standard device request
 * @constant    kIOUSBDeviceRequestTypeValueClass the device request is a class specified device request
 * @constant    kIOUSBDeviceRequestTypeValueVendor the device request is a vendor specified device request
 */
enum tIOUSBDeviceRequestTypeValue
{
    kIOUSBDeviceRequestTypeValueStandard = 0,
    kIOUSBDeviceRequestTypeValueClass    = 1,
    kIOUSBDeviceRequestTypeValueVendor   = 2
};

typedef enum tIOUSBDeviceRequestTypeValue tIOUSBDeviceRequestTypeValue;

/*!
 * @enum        tIOUSBDeviceRequestRecipientValue
 * @brief       Values of 'recipient' field of a device request
 * @constant    kIOUSBDeviceRequestRecipientValueDevice the device request's recipient is a device
 * @constant    kIOUSBDeviceRequestRecipientValueInterface the device request's recipient is an interface
 * @constant    kIOUSBDeviceRequestRecipientValueEndpoint the device request's recipient is an endpoint
 * @constant    kIOUSBDeviceRequestRecipientValueOther the device request's recipient is an other type
 */
enum tIOUSBDeviceRequestRecipientValue
{
    kIOUSBDeviceRequestRecipientValueDevice    = 0,
    kIOUSBDeviceRequestRecipientValueInterface = 1,
    kIOUSBDeviceRequestRecipientValueEndpoint  = 2,
    kIOUSBDeviceRequestRecipientValueOther     = 3
};

typedef enum tIOUSBDeviceRequestRecipientValue tIOUSBDeviceRequestRecipientValue;

enum tIOUSBDeviceRequest
{
    kIOUSBDeviceRequestSize               = 8,
    kIOUSBDeviceRequestDirectionMask      = IOUSBBit(7),
    kIOUSBDeviceRequestDirectionPhase     = IOUSBBitRangePhase(7, 7),
    kIOUSBDeviceRequestDirectionOut       = (kIOUSBDeviceRequestDirectionValueOut << kIOUSBDeviceRequestDirectionPhase),
    kIOUSBDeviceRequestDirectionIn        = (kIOUSBDeviceRequestDirectionValueIn << kIOUSBDeviceRequestDirectionPhase),
    kIOUSBDeviceRequestTypeMask           = IOUSBBitRange(5, 6),
    kIOUSBDeviceRequestTypePhase          = IOUSBBitRangePhase(5, 6),
    kIOUSBDeviceRequestTypeStandard       = (kIOUSBDeviceRequestTypeValueStandard << kIOUSBDeviceRequestTypePhase),
    kIOUSBDeviceRequestTypeClass          = (kIOUSBDeviceRequestTypeValueClass << kIOUSBDeviceRequestTypePhase),
    kIOUSBDeviceRequestTypeVendor         = (kIOUSBDeviceRequestTypeValueVendor << kIOUSBDeviceRequestTypePhase),
    kIOUSBDeviceRequestRecipientMask      = IOUSBBitRange(0, 4),
    kIOUSBDeviceRequestRecipientPhase     = IOUSBBitRangePhase(0, 4),
    kIOUSBDeviceRequestRecipientDevice    = (kIOUSBDeviceRequestRecipientValueDevice << kIOUSBDeviceRequestRecipientPhase),
    kIOUSBDeviceRequestRecipientInterface = (kIOUSBDeviceRequestRecipientValueInterface << kIOUSBDeviceRequestRecipientPhase),
    kIOUSBDeviceRequestRecipientEndpoint  = (kIOUSBDeviceRequestRecipientValueEndpoint << kIOUSBDeviceRequestRecipientPhase),
    kIOUSBDeviceRequestRecipientOther     = (kIOUSBDeviceRequestRecipientValueOther << kIOUSBDeviceRequestRecipientPhase),
};

// USB 2.0 9.4: Standard Device Requests
// USB 3.0 9.4: Standard Device Requests
enum
{
    kIOUSBDeviceRequestGetStatus           = 0,
    kIOUSBDeviceRequestClearFeature        = 1,
    kIOUSBDeviceRequestGetState            = 2,
    kIOUSBDeviceRequestSetFeature          = 3,
    kIOUSBDeviceRequestSetAddress          = 5,
    kIOUSBDeviceRequestGetDescriptor       = 6,
    kIOUSBDeviceRequestSetDescriptor       = 7,
    kIOUSBDeviceRequestGetConfiguration    = 8,
    kIOUSBDeviceRequestSetConfiguration    = 9,
    kIOUSBDeviceRequestGetInterface        = 10,
    kIOUSBDeviceRequestSetInterface        = 11,
    kIOUSBDeviceRequestSynchFrame          = 12,
    kIOUSBDeviceRequestSetSel              = 48,
    kIOUSBDeviceRequestSetIsochronousDelay = 49
};

// USB 2.0 9.4.5: Get Status
// USB 3.0 9.4.5: Get Status
enum
{
    kIOUSBDeviceStatusSelfPowered      = IOUSBBit(0),
    kIOUSBDeviceStatusRemoteWakeEnable = IOUSBBit(1),
    kIOUSBDeviceStatusU1Enable         = IOUSBBit(2),
    kIOUSBDeviceStatusU2Enable         = IOUSBBit(3),
    kIOUSBDeviceStatusLTMEnable        = IOUSBBit(4),

    kIOUSBInterfaceStatusRemoteWakeCapable = IOUSBBit(0),
    kIOUSBInterfaceStatusRemoteWakeEnable  = IOUSBBit(1),

    IOUSBEndpointStatusHalt = IOUSBBit(0)
};

// USB 2.0 Table 9-6: Standard Feature Selectors
// USB 3.0 Table 9-7: Standard Feature Selectors
enum
{
    kIOUSBDeviceFeatureSelectorRemoteWakeup = 1,
    kIOUSBDeviceFeatureSelectorTestMode     = 2,
    kIOUSBDeviceFeatureSelectorU1Enable     = 48,
    kIOUSBDeviceFeatureSelectorU2Enable     = 49,
    kIOUSBDeviceFeatureSelectorLTMEnable    = 50,

    kIOUSBInterfaceFeatureSelectorSuspend = 0,

    IOUSBEndpointFeatureSelectorStall = 0,
};

// USB 3.0 Table 9-8: Suspend Options
enum
{
    kIOUSBInterfaceSuspendLowPower         = IOUSBBit(0),
    kIOUSBInterfaceSuspendRemoteWakeEnable = IOUSBBit(1)
};

// USB 3.0 Table 10-16: Hub Parameters
enum
{
    kIOUSBHubPort2PortExitLatencyNs = 1000,
    kIOUSBHubDelayNs                = 400
};

// USB 3.0 Table 8-33: Timing Parameters
enum
{
    kIOUSBPingResponseTimeNs = 400
};

#pragma mark Power Constants
enum tIOUSBBusVoltage
{
    kIOUSBBusVoltageDefault = 5
};

enum tIOUSB20BusCurrent
{
    kIOUSB20BusCurrentMinimum       = 100,
    kIOUSB20BusCurrentDefault       = 500,
    kIOUSB20BusCurrentMaxPowerUnits = 2
};

enum tIOUSB30BusCurrent
{
    kIOUSB30BusCurrentMinimum       = 150,
    kIOUSB30BusCurrentDefault       = 900,
    kIOUSB30BusCurrentMaxPowerUnits = 8
};

#pragma mark USB 3.0 constants
// USB 3.0 Table 6-21
enum tIOUSB30ResetTimeout
{
    kIOUSB30ResetMinimumTimeout           = 80,
    kIOUSB30ResetTypicalTimeout           = 100,
    kIOUSB30ResetMaximumTimeout           = 120,
    kIOUSB30ResetMaximumWithMarginTimeout = 150
};

// USB 3.0 Table 7-12
enum tIOUSB30LinkStateTimeout
{
    kIOUSB30LinkStateSSInactiveQuietTimeout       = 12,
    kIOUSB30LinkStateRxDetectQuietTimeout         = 12,
    kIOUSB30LinkStatePollingLFPSTimeout           = 360,
    kIOUSB30LinkStatePollingActiveTimeout         = 12,
    kIOUSB30LinkStatePollingConfigurationTimeout  = 12,
    kIOUSB30LinkStatePollingIdleTimeout           = 2,
    kIOUSB30LinkStateU0RecoveryTimeout            = 1,
    kIOUSB30LinkStateU0LTimeout                   = 0,         // 10 microseconds
    kIOUSB30LinkStateU1NoLFPSResponseTimeout      = 2,
    kIOUSB30LinkStateU1PingTimeout                = 300,
    kIOUSB30LinkStateU2NoLFPSResponseTimeout      = 2,
    kIOUSB30LinKStateU2RxDetectDelay              = 100,
    kIOUSB30LinkStateU3NoLFPSResponseTimeout      = 10,
    kIOUSB30LinkStateU3WakeupRetryDelay           = 100,
    kIOUSB30LinkStateU3RxDetectDelay              = 100,
    kIOUSB30LinkStateRecoveryActiveTimeout        = 12,
    kIOUSB30LinkStateRecoveryConfigurationTimeout = 6,
    kIOUSB30LinkStateRecoveryIdleTimeout          = 2,
    kIOUSB30LinkStateLoopbackExitTimeout          = 2,
    kIOUSB30LinkStateHotResetActiveTimeout        = 12,
    kIOUSB30LinkStateHotResetExistTimeout         = 2,

    // USB 3.0 7.5.4
    kIOUSB30LinkStatePollingDeadline = (kIOUSB30LinkStatePollingLFPSTimeout + 1 + kIOUSB30LinkStatePollingActiveTimeout + kIOUSB30LinkStatePollingConfigurationTimeout + kIOUSB30LinkStatePollingIdleTimeout),

    // USB 3.0 7.5.9 and 7.5.10
    kIOUSB30LinkStateSSResumeDeadline = (kIOUSB30LinkStateU3NoLFPSResponseTimeout + kIOUSB30LinkStateRecoveryActiveTimeout + kIOUSB30LinkStateRecoveryConfigurationTimeout + kIOUSB30LinkStateRecoveryIdleTimeout),
};

// USB 3.1 Table 8-18
enum tIOUSB30DeviceNotificationType
{
    kIOUSB30DeviceNotificationTypeFunctionWake          = 1,
    kIOUSB30DeviceNotificationTypeLatencyTolerance      = 2,
    kIOUSB30DeviceNotificationTypeBusIntervalAdjustment = 3,
    kIOUSB30DeviceNotificationTypeHostRoleRequest       = 4,
    kIOUSB30DeviceNotificationTypeSublinkSpeed          = 5
};

// USB 3.1 Table 8-36
enum tIOUSB30TimingParameters
{
    kIOUSB30TimingParameterBELTDefaultNs = 1 * 1000 * 1000,
    kIOUSB30TimingParameterBELTMinNs     =      125 * 1000
};

// USB 3.1 Table 10-12 Port Status Type Codes
enum tIOUSB30HubPortStatusCode
{
    kIOUSB30HubPortStatusCodeStandard = 0,
    kIOUSB30HubPortStatusCodePD       = 1,
    kIOUSB30HubPortStatusCodeExt      = 2,
    kIOUSB30HubPortStatusCodeCount    = 3
};

/*!
 * @typedef    IOUSB30HubPortStatusExt
 * @discussion See the USB Specification at <a href="http://www.usb.org"TARGET="_blank">http://www.usb.org</a>.
 *             USB 3.1 10.16.2.6 Get Port Status
 */
struct IOUSB30HubPortStatusExt
{
    uint16_t wPortStatus;
    uint16_t wPortChange;
    uint32_t dwExtPortStatus;
} __attribute__((packed));

// USB 3.1 Table 10-11.  Note, offsets are specific to dwExtPortStatus
enum tIOUSB30HubExtStatus
{
    kIOUSB30HubExtStatusRxSublinkSpeedID      = IOUSBBitRange(0, 3),
    kIOUSB30HubExtStatusRxSublinkSpeedIDPhase = IOUSBBitRangePhase(0, 3),

    kIOUSB30HubExtStatusTxSublinkSpeedID      = IOUSBBitRange(4, 7),
    kIOUSB30HubExtStatusTxSublinkSpeedIDPhase = IOUSBBitRangePhase(4, 7),

    kIOUSB30HubExtStatusRxLaneCount      = IOUSBBitRange(8, 11),
    kIOUSB30HubExtStatusRxLaneCountPhase = IOUSBBitRangePhase(8, 11),

    kIOUSB30HubExtStatusTxLaneCount      = IOUSBBitRange(12, 15),
    kIOUSB30HubExtStatusTxLaneCountPhase = IOUSBBitRangePhase(12, 15)
};

#endif /* AppleUSBCommon_AppleUSBDefinitions_h */
