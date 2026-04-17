/*
 * Copyright (c) 2018-2019 Apple Inc. All rights reserved.
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

#ifndef IOPCIDefinitions_h
#define IOPCIDefinitions_h

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

// IOPCIDevice matching property names
#define kIOPCIMatchKey                  "IOPCIMatch"
#define kIOPCIPrimaryMatchKey           "IOPCIPrimaryMatch"
#define kIOPCISecondaryMatchKey         "IOPCISecondaryMatch"
#define kIOPCIClassMatchKey             "IOPCIClassMatch"

// pci express capabilities
#define kIOPCIExpressCapabilitiesKey     "IOPCIExpressCapabilities"
// pci express link status
#define kIOPCIExpressLinkStatusKey       "IOPCIExpressLinkStatus"
// pci express link capabilities
#define kIOPCIExpressLinkCapabilitiesKey "IOPCIExpressLinkCapabilities"
// pci express slot status
#define kIOPCIExpressSlotStatusKey       "IOPCIExpressSlotStatus"
// pci express slot capabilities
#define kIOPCIExpressSlotCapabilitiesKey "IOPCIExpressSlotCapabilities"

#define kIOPCIDeviceMemoryArrayKey       "IODeviceMemory"

/* Definitions of PCI Config Registers */
enum
{
    kIOPCIConfigurationOffsetVendorID          = 0x00,
    kIOPCIConfigurationOffsetDeviceID          = 0x02,
    kIOPCIConfigurationOffsetCommand           = 0x04,
    kIOPCIConfigurationOffsetStatus            = 0x06,
    kIOPCIConfigurationOffsetRevisionID        = 0x08,
    kIOPCIConfigurationOffsetClassCode         = 0x09,
    kIOPCIConfigurationOffsetCacheLineSize     = 0x0C,
    kIOPCIConfigurationOffsetLatencyTimer      = 0x0D,
    kIOPCIConfigurationOffsetHeaderType        = 0x0E,
    kIOPCIConfigurationOffsetBIST              = 0x0F,
    kIOPCIConfigurationOffsetBaseAddress0      = 0x10,
    kIOPCIConfigurationOffsetBaseAddress1      = 0x14,
    kIOPCIConfigurationOffsetBaseAddress2      = 0x18,
    kIOPCIConfigurationOffsetBaseAddress3      = 0x1C,
    kIOPCIConfigurationOffsetBaseAddress4      = 0x20,
    kIOPCIConfigurationOffsetBaseAddress5      = 0x24,
    kIOPCIConfigurationOffsetCardBusCISPtr     = 0x28,
    kIOPCIConfigurationOffsetSubSystemVendorID = 0x2C,
    kIOPCIConfigurationOffsetSubSystemID       = 0x2E,
    kIOPCIConfigurationOffsetExpansionROMBase  = 0x30,
    kIOPCIConfigurationOffsetCapabilitiesPtr   = 0x34,
    kIOPCIConfigurationOffsetInterruptLine     = 0x3C,
    kIOPCIConfigurationOffsetInterruptPin      = 0x3D,
    kIOPCIConfigurationOffsetMinimumGrant      = 0x3E,
    kIOPCIConfigurationOffsetMaximumLatency    = 0x3F
};

/* Definitions of Capabilities PCI Config Register */
enum
{
    kIOPCICapabilityIDOffset   = 0x00,
    kIOPCINextCapabilityOffset = 0x01,

    kIOPCICapabilityIDPowerManagement     = 0x01,
    kIOPCICapabilityIDAGP                 = 0x02,
    kIOPCICapabilityIDVitalProductData    = 0x03,
    kIOPCICapabilityIDSlotID              = 0x04,
    kIOPCICapabilityIDMSI                 = 0x05,
    kIOPCICapabilityIDCPCIHotswap         = 0x06,
    kIOPCICapabilityIDPCIX                = 0x07,
    kIOPCICapabilityIDLDT                 = 0x08,
    kIOPCICapabilityIDVendorSpecific      = 0x09,
    kIOPCICapabilityIDDebugPort           = 0x0a,
    kIOPCICapabilityIDCPCIResourceControl = 0x0b,
    kIOPCICapabilityIDHotplug             = 0x0c,
    kIOPCICapabilityIDAGP8                = 0x0e,
    kIOPCICapabilityIDSecure              = 0x0f,
    kIOPCICapabilityIDPCIExpress          = 0x10,
    kIOPCICapabilityIDMSIX                = 0x11,
    kIOPCICapabilityIDFPB                 = 0x15,

    kIOPCIExpressCapabilityIDErrorReporting            = -0x01UL,
    kIOPCIExpressCapabilityIDVirtualChannel            = -0x02UL,
    kIOPCIExpressCapabilityIDDeviceSerialNumber        = -0x03UL,
    kIOPCIExpressCapabilityIDPowerBudget               = -0x04UL,
    kIOPCIExpressCapabilityIDAccessControlServices     = -0x0DUL,
    kIOPCIExpressCapabilityIDLatencyTolerenceReporting = -0x18UL,
    kIOPCIExpressCapabilityIDL1PMSubstates             = -0x1EUL,
};

/* Command register definitions */
enum
{
    kIOPCICommandIOSpace          = 0x0001,
    kIOPCICommandMemorySpace      = 0x0002,
    kIOPCICommandBusMaster        = 0x0004,
    kIOPCICommandSpecialCycles    = 0x0008,
    kIOPCICommandMemWrInvalidate  = 0x0010,
    kIOPCICommandPaletteSnoop     = 0x0020,
    kIOPCICommandParityError      = 0x0040,
    kIOPCICommandAddressStepping  = 0x0080,
    kIOPCICommandSERR             = 0x0100,
    kIOPCICommandFastBack2Back    = 0x0200,
    kIOPCICommandInterruptDisable = 0x0400
};

/* Status register definitions */
enum
{
    kIOPCIStatusInterrupt          = 0x0008,
    kIOPCIStatusCapabilities       = 0x0010,
    kIOPCIStatusPCI66              = 0x0020,
    kIOPCIStatusUDF                = 0x0040,
    kIOPCIStatusFastBack2Back      = 0x0080,
    kIOPCIStatusDevSel0            = 0x0000,
    kIOPCIStatusDevSel1            = 0x0200,
    kIOPCIStatusDevSel2            = 0x0400,
    kIOPCIStatusDevSel3            = 0x0600,
    kIOPCIStatusTargetAbortCapable = 0x0800,
    kIOPCIStatusTargetAbortActive  = 0x1000,
    kIOPCIStatusMasterAbortActive  = 0x2000,
    kIOPCIStatusSERRActive         = 0x4000,
    kIOPCIStatusParityErrActive    = 0x8000
};

enum
{
    kPCI2PCIOffsetPrimaryBus          = 0x18,
    kPCI2PCIOffsetSecondaryBus        = 0x19,
    kPCI2PCIOffsetSubordinateBus      = 0x1a,
    kPCI2PCIOffsetSecondaryLT         = 0x1b,
    kPCI2PCIOffsetIORange             = 0x1c,
    kPCI2PCIOffsetMemoryRange         = 0x20,
    kPCI2PCIOffsetPrefetchMemoryRange = 0x24,
    kPCI2PCIOffsetPrefetchUpperBase   = 0x28,
    kPCI2PCIOffsetPrefetchUpperLimit  = 0x2c,
    kPCI2PCIOffsetUpperIORange        = 0x30,
    kPCI2PCIOffsetBridgeControl       = 0x3e
};

// constants which are part of the PCI Bus Power Management Spec.
enum
{
    // capabilities bits in the 16 bit capabilities register
    kPCIPMCPMESupportFromD3Cold = 0x8000,
    kPCIPMCPMESupportFromD3Hot  = 0x4000,
    kPCIPMCPMESupportFromD2     = 0x2000,
    kPCIPMCPMESupportFromD1     = 0x1000,
    kPCIPMCPMESupportFromD0     = 0x0800,
    kPCIPMCD2Support            = 0x0400,
    kPCIPMCD1Support            = 0x0200,

    kPCIPMCD3Support = 0x0001
};

enum
{
    // bits in the power management control/status register
    kPCIPMCSPMEStatus      = 0x8000,
    kPCIPMCSPMEEnable      = 0x0100,
    kPCIPMCSPowerStateMask = 0x0003,
    kPCIPMCSPowerStateD3   = 0x0003,
    kPCIPMCSPowerStateD2   = 0x0002,
    kPCIPMCSPowerStateD1   = 0x0001,
    kPCIPMCSPowerStateD0   = 0x0000,

    kPCIPMCSDefaultEnableBits = (~(IOOptionBits)0),

    kPCIPMCSPMEDisableInS3 = 0x00010000,
    kPCIPMCSPMEWakeReason  = 0x00020000
};

// PCIe error bits
enum
{
    kIOPCIUncorrectableErrorBitDataLinkProtocol     = 4,
    kIOPCIUncorrectableErrorBitSurpriseDown         = 5,
    kIOPCIUncorrectableErrorBitPoisonedTLP          = 12,
    kIOPCIUncorrectableErrorBitFlowControlProtocol  = 13,
    kIOPCIUncorrectableErrorBitCompletionTimeout    = 14,
    kIOPCIUncorrectableErrorBitCompleterAbort       = 15,
    kIOPCIUncorrectableErrorBitUnexpectedCompletion = 16,
    kIOPCIUncorrectableErrorBitReceiverOverflow     = 17,
    kIOPCIUncorrectableErrorBitMalformedTLP         = 18,
    kIOPCIUncorrectableErrorBitECRC                 = 19,
    kIOPCIUncorrectableErrorBitUnsupportedRequest   = 20,

    kIOPCIUncorrectableErrorBitACSViolation          = 21,
    kIOPCIUncorrectableErrorBitInternal              = 22,
    kIOPCIUncorrectableErrorBitMCBlockedTLP          = 23,
    kIOPCIUncorrectableErrorBitAtomicOpEgressBlocked = 24,
    kIOPCIUncorrectableErrorBitTLPPrefixBlocked      = 25
};

enum
{
    kIOPCICorrectableErrorBitReceiver           = 0,
    kIOPCICorrectableErrorBitBadTLP             = 6,
    kIOPCICorrectableErrorBitBadDLLP            = 7,
    kIOPCICorrectableErrorBitReplayNumRollover  = 8,
    kIOPCICorrectableErrorBitReplayTimerTimeout = 12,
    kIOPCICorrectableErrorBitAdvisoryNonFatal   = 13,
    kIOPCICorrectableErrorBitCorrectedInternal  = 14,
    kIOPCICorrectableErrorBitHeaderLogOverflow  = 15,
};


#endif /* IOPCIDefinitions_h */
