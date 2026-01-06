/*
 * Copyright (c) 1998-2007 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_IO_ATA_STORAGE_DEFINES_H_
#define _IOKIT_IO_ATA_STORAGE_DEFINES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
	
/*
 * Important word offsets in device identify data as
 * defined in ATA-5 standard
 */
	
enum
{
	kATAIdentifyConfiguration				= 0,
	kATAIdentifyLogicalCylinderCount 		= 1,
	kATAIdentifyLogicalHeadCount			= 3,
	kATAIdentifySectorsPerTrack				= 6,
	kATAIdentifySerialNumber				= 10,
	kATAIdentifyFirmwareRevision			= 23,
	kATAIdentifyModelNumber					= 27,
	kATAIdentifyMultipleSectorCount			= 47,
	kATAIdentifyDriveCapabilities			= 49,
	kATAIdentifyDriveCapabilitiesExtended	= 50,
	kATAIdentifyPIOTiming					= 51,
	kATAIdentifyExtendedInfoSupport			= 53,
	kATAIdentifyCurrentCylinders			= 54,
	kATAIdentifyCurrentHeads				= 55,
	kATAIdentifyCurrentSectors				= 56,
	kATAIdentifyCurrentCapacity				= 57,
	kATAIdentifyCurrentMultipleSectors		= 59,
	kATAIdentifyLBACapacity					= 60,
	kATAIdentifySingleWordDMA				= 62,
	kATAIdentifyMultiWordDMA				= 63,
	kATAIdentifyAdvancedPIOModes			= 64,
	kATAIdentifyMinMultiWordDMATime			= 65,
	kATAIdentifyRecommendedMultiWordDMATime	= 66,
	kATAIdentifyMinPIOTime					= 67,
	kATAIdentifyMinPIOTimeWithIORDY			= 68,
	kATAIdentifyQueueDepth					= 75,
	kATAIdentifyMajorVersion				= 80,
	kATAIdentifyMinorVersion				= 81,
	kATAIdentifyCommandSetSupported			= 82,
	kATAIdentifyCommandSetSupported2		= 83,
	kATAIdentifyCommandExtension1			= 84,
	kATAIdentifyCommandExtension2			= 85,
	kATAIdentifyCommandsEnabled				= 86,
	kATAIdentifyCommandsDefault				= 87,
	kATAIdentifyUltraDMASupported			= 88,
	kATAIdentifyPhysicalLogicalSectorSize	= 106,
	kATAIdentifyWordsPerLogicalSector1		= 117,
	kATAIdentifyWordsPerLogicalSector2		= 118,
	kATAIdentifyLogicalSectorAlignment		= 209,
	kATAIdentifyIntegrity					= 255
};
	

/* 
 * Important bits in device identify data
 * as defined in ATA-5 standard
 */
 
enum
{
	// Configuration field (word 0)
	kFixedDeviceBit			= 6,							// Fixed disk indicator bit
	kRemoveableMediaBit		= 7,							// Removable media indicator bit
	kNonMagneticDriveBit	= 15,							// Non-magnetic drive indicator bit
	
	kFixedDeviceMask		= (1 << kFixedDeviceBit),		// Mask for fixed disk indicator
	kRemoveableMediaMask	= (1 << kRemoveableMediaBit),	// Mask for removable media indicator
	kNonMagneticDriveMask	= (1 << kNonMagneticDriveBit),	// Mask for non-magnetic drive indicator

	// Capabilities field (word 49)
	kDMABit					= 8,							// DMA supported bit
	kLBABit					= 9,							// LBA supported bit
	kIORDYDisableBit		= 10,							// IORDY can be disabled bit
	kIORDYBit				= 11,							// IORDY supported bit
	kStandbyTimerBit		= 13,							// Standby timer supported bit

	kDMASupportedMask		= (1 << kDMABit),				// Mask for DMA supported
	kLBASupportedMask		= (1 << kLBABit),				// Mask for LBA supported
	kDMADisableMask			= (1 << kIORDYDisableBit),		// Mask for DMA supported
	kIORDYSupportedMask		= (1 << kIORDYBit),				// Mask for IORDY supported
	kStandbySupportedMask	= (1 << kStandbyTimerBit),		// Mask for Standby Timer supported

	// Extensions field (word 53)
	kCurFieldsValidBit		= 0,							// Bit to show words 54-58 are valid
	kExtFieldsValidBit		= 1,							// Bit to show words 64-70 are valid
	kCurFieldsValidMask		= (1 << kCurFieldsValidBit),	// Mask for current fields valid
	kExtFieldsValidMask		= (1 << kExtFieldsValidBit),	// Extension word valid

	// Advanced PIO Transfer Modes field (word 64)
	kMode3Bit				= 0,							// Bit to indicate mode 3 is supported
	kMode3Mask				= (1 << kMode3Bit),				// Mask for mode 3 support
	
	// Integrity of Identify data (word 255)
	kChecksumValidCookie	= 0xA5							// Bits 7:0 if device supports feature
	
};


/* String size constants */
enum
{
	kSizeOfATAModelString 		= 40,
	kSizeOfATARevisionString	= 8
};

/* ATA Command timeout constants ( in milliseconds ) */
enum
{
	kATATimeout10Seconds	= 10000,
	kATATimeout30Seconds	= 30000,
	kATATimeout45Seconds	= 45000,
	kATATimeout1Minute		= 60000,
	kATADefaultTimeout		= kATATimeout30Seconds
};


/* Retry constants */
enum
{
	kATAZeroRetries		= 0,
	kATADefaultRetries	= 4
};

/* max number of blocks supported in ATA transaction */
enum
{
	kIOATASectorCount8Bit	= 8,
	kIOATASectorCount16Bit	= 16
};

enum
{
	kIOATAMaximumBlockCount8Bit		= (1 << kIOATASectorCount8Bit),
	kIOATAMaximumBlockCount16Bit	= (1 << kIOATASectorCount16Bit),
	
	// For backwards compatibility
	kIOATAMaxBlocksPerXfer			= kIOATAMaximumBlockCount8Bit
};



/* Power Management time constants (in seconds) */
enum
{
	kSecondsInAMinute	= 60,
	k5Minutes			= 5 * kSecondsInAMinute
};

/* Bits for features published in Word 82 of device identify data */
enum
{
	kATASupportsSMARTBit					= 0,
	kATASupportsPowerManagementBit  		= 3,
	kATASupportsWriteCacheBit				= 5
};

/* Masks for features published in Word 82 of device identify data */
enum
{
	kATASupportsSMARTMask					= (1 << kATASupportsSMARTBit),
	kATASupportsPowerManagementMask 		= (1 << kATASupportsPowerManagementBit),
	kATASupportsWriteCacheMask				= (1 << kATASupportsWriteCacheBit)
};

/* Bits for features published in Word 83 of device identify data */
enum
{
	kATASupportsCompactFlashBit				= 2,
	kATASupportsAdvancedPowerManagementBit 	= 3,
	
	kATASupports48BitAddressingBit			= 10,
	
	kATASupportsFlushCacheBit				= 12,
	kATASupportsFlushCacheExtendedBit		= 13
};

/* Masks for features published in Word 83 of device identify data */
enum
{
	kATASupportsCompactFlashMask			= (1 << kATASupportsCompactFlashBit),
	kATASupportsAdvancedPowerManagementMask = (1 << kATASupportsAdvancedPowerManagementBit),
	
	kATASupports48BitAddressingMask			= (1 << kATASupports48BitAddressingBit),
	
	kATASupportsFlushCacheMask				= (1 << kATASupportsFlushCacheBit),
	kATASupportsFlushCacheExtendedMask		= (1 << kATASupportsFlushCacheExtendedBit),
	
	// Mask to ensure data is valid
	kIdentifyWordValidationMask				= 0xC000,
	kIdentifyWordValid						= 0x4000
};

/* Bits for features published in Word 84 of device identify data */
enum
{
	kATAForceUnitAccessFeatureBit			= 6,
};

/* Masks for features published in Word 84 of device identify data */
enum
{
	kATAForceUnitAccessFeatureMask			= (1 << kATAForceUnitAccessFeatureBit),
};

/* Bits for features published in Word 85 of device identify data */
enum
{
	kATAWriteCacheEnabledBit				= 5
};

/* Masks for features published in Word 85 of device identify data */
enum
{
	kATAWriteCacheEnabledMask				= (1 << kATAWriteCacheEnabledBit)
};


/* Bits for features published in Word 106 of device identify data */
enum
{
	kATAPhysicalLogicalEnabledBit0			= 15,
	kATAPhysicalLogicalEnabledBit1			= 14,
	kATAMultipleLogicalSectorsBit			= 13,
	kATAValidLogicalSectorSizeBit			= 12	
};

/* Masks for features published in Word 106 of device identify data */
enum
{
	kATAPhysicalLogicalEnabledMask			= (1 << kATAPhysicalLogicalEnabledBit0) | (1 << kATAPhysicalLogicalEnabledBit1),
	kATAPhysicalLogicalEnabledValue			= (0 << kATAPhysicalLogicalEnabledBit0) | (1 << kATAPhysicalLogicalEnabledBit1),
	kATAMultipleLogicalSectorsMask			= (1 << kATAMultipleLogicalSectorsBit),
	kATAValidLogicalSectorSizeMask			= (1 << kATAValidLogicalSectorSizeBit),
	kATAPhysicalSectorSizeMask				= 0xF,
	kATALogicalSectorAlignmentMask			= 0x3FFF
};

// Property table keys
#define kIOATASupportedFeaturesKey		"ATA Features"

/* ATA supported features */
enum
{
	kIOATAFeaturePowerManagement			= 0x01,		/* OBSOLETE */
	kIOATAFeatureWriteCache					= 0x02,		/* OBSOLETE */
	kIOATAFeatureAdvancedPowerManagement 	= 0x04,
	kIOATAFeatureCompactFlash				= 0x08,
	kIOATAFeature48BitLBA					= 0x10,
	kIOATAFeatureSMART						= 0x20
};

/* ATA Advanced Power Management settings (valid settings range from 1-254),
the settings below are the more common settings */
enum
{
	kIOATAMaxPerformance					= 0xFE,
	kIOATADefaultPerformance			 	= 0x80,
	kIOATAMaxPowerSavings					= 0x01
};

/* ATA Transfer Mode bit masks */
enum
{
	kATAEnableUltraDMAModeMask 		= 0x40,
	kATAEnableMultiWordDMAModeMask	= 0x20,
	kATAEnablePIOModeMask			= 0x08
};


typedef uint32_t	ATAOperationType;
enum
{
	kATAOperationTypeRead				= 0,
	kATAOperationTypeWrite				= 1,
	kATAOperationTypeFlushCache			= 2,
	kATAOperationTypeSMART				= 3,
	kATAOperationTypeConfiguration		= 4,
	kATAOperationTypePowerManagement	= 5,
	kATAOperationTypeSMS				= 6
};

#if defined(KERNEL)

typedef struct __ATAIORequest *	ATARequestIdentifier;

#endif	// defined(KERNEL)
	
#ifdef __cplusplus
}
#endif

#endif	/* _IOKIT_IO_ATA_STORAGE_DEFINES_H_ */
