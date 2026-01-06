/*
 * Copyright (c) 1998-2009 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_SCSI_CMDS_MODE_DEFINITIONS_H_
#define _IOKIT_SCSI_CMDS_MODE_DEFINITIONS_H_


//-----------------------------------------------------------------------------
//	Includes
//-----------------------------------------------------------------------------

#if KERNEL
#include <IOKit/IOTypes.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#endif


/*! @header SCSI Request Sense Definitions
	@discussion
	This file contains all definitions for the data returned from
	the MODE_SENSE_6 and MODE_SENSE_10 commands.
*/

#pragma pack(1)

/*!
@struct SPCModeParameterHeader6
@discussion
Mode Parameter Header for the MODE_SENSE_6 command.
*/
typedef struct SPCModeParameterHeader6
{
	UInt8		MODE_DATA_LENGTH;
	UInt8		MEDIUM_TYPE;
	UInt8		DEVICE_SPECIFIC_PARAMETER;
	UInt8		BLOCK_DESCRIPTOR_LENGTH;
} SPCModeParameterHeader6;


/*!
@struct SPCModeParameterHeader10
@discussion
Mode Parameter Header for the MODE_SENSE_10 command.
*/
typedef struct SPCModeParameterHeader10
{
	UInt16		MODE_DATA_LENGTH;
	UInt8		MEDIUM_TYPE;
	UInt8		DEVICE_SPECIFIC_PARAMETER;
	UInt8		LONGLBA;
	UInt8		RESERVED;
	UInt16		BLOCK_DESCRIPTOR_LENGTH;
} SPCModeParameterHeader10;


/*!
@enum Long LBA Bitfield definitions
@discussion
Long LBA Bitfield definitions for Mode Parameter Header
for MODE_SENSE_10 command.
@constant kModeSenseParameterHeader10_LongLBABit
Bit to indicate Long LBA block descriptors follow.
@constant kModeSenseParameterHeader10_LongLBAMask
Mask to test for kModeSenseParameterHeader10_LongLBABit.
*/
enum
{
	kModeSenseParameterHeader10_LongLBABit	=  0,
	kModeSenseParameterHeader10_LongLBAMask	=  (1 << kModeSenseParameterHeader10_LongLBABit),
};


/*!
@enum Device Specific Parameter Bitfield definitions
@discussion
SBC definitions for Device Specific Parameter in the
Mode Sense Header Block.
@constant kModeSenseSBCDeviceSpecific_DPOFUABit
Bit to indicate DPO and FUA bits are accepted by the device server.
@constant kModeSenseSBCDeviceSpecific_WriteProtectBit
Bit to indicate medium is write protected.
@constant kModeSenseSBCDeviceSpecific_DPOFUAMask
Mask to test for kModeSenseSBCDeviceSpecific_DPOFUABit.
@constant kModeSenseSBCDeviceSpecific_WriteProtectMask
Mask to test for kModeSenseSBCDeviceSpecific_WriteProtectBit.
*/
enum
{
	kModeSenseSBCDeviceSpecific_DPOFUABit	 		=  4,
	kModeSenseSBCDeviceSpecific_WriteProtectBit	 	=  7,
	kModeSenseSBCDeviceSpecific_DPOFUAMask 			=  (1 << kModeSenseSBCDeviceSpecific_DPOFUABit),
	kModeSenseSBCDeviceSpecific_WriteProtectMask 	=  (1 << kModeSenseSBCDeviceSpecific_WriteProtectBit)
};


/*!
@struct ModeParameterBlockDescriptor
@discussion
General mode parameter block descriptor.
*/
typedef struct ModeParameterBlockDescriptor
{
	UInt8		DENSITY_CODE;
	UInt8		NUMBER_OF_BLOCKS[3];
	UInt8		RESERVED;
	UInt8		BLOCK_LENGTH[3];
} ModeParameterBlockDescriptor;


/*!
@struct DASDModeParameterBlockDescriptor
@discussion
Direct Access Storage Device mode parameter block descriptor.
*/
typedef struct DASDModeParameterBlockDescriptor
{
	UInt32		NUMBER_OF_BLOCKS;
	UInt8		DENSITY_CODE;
	UInt8		BLOCK_LENGTH[3];
} DASDModeParameterBlockDescriptor;


/*!
@struct LongLBAModeParameterBlockDescriptor
@discussion
Long LBA mode parameter block descriptor.
*/
typedef struct LongLBAModeParameterBlockDescriptor
{
	UInt64		NUMBER_OF_BLOCKS;
	UInt8		DENSITY_CODE;
	UInt8		RESERVED[3];
	UInt32		BLOCK_LENGTH;
} LongLBAModeParameterBlockDescriptor;


/*!
@struct ModePageFormatHeader
@discussion
Mode Page format header.
*/
typedef struct ModePageFormatHeader
{
	UInt8		PS_PAGE_CODE;
	UInt8		PAGE_LENGTH;
} ModePageFormatHeader;


/*!
@enum Mode Page Format bit definitions
@discussion
Mode Page Format bit definitions.
@constant kModePageFormat_PS_Bit
Bit to indicate Parameters Saveable.
@constant kModePageFormat_PAGE_CODE_Mask
Mask to obtain the PAGE_CODE from the PS_PAGE_CODE field.
@constant kModePageFormat_PS_Mask
Mask to test for kModePageFormat_PS_Bit.
*/
enum
{
	kModePageFormat_PS_Bit			= 7,
	
	kModePageFormat_PAGE_CODE_Mask	= 0x3F,
	kModePageFormat_PS_Mask			= (1 << kModePageFormat_PS_Bit)
};


#if 0
#pragma mark -
#pragma mark SPC Mode Pages
#pragma mark -
#endif


/*!
@enum SPC Mode Pages
@discussion
SPC Mode Page definitions.
@constant kSPCModePagePowerConditionCode
Power Conditions Mode Page value.
@constant kSPCModePageAllPagesCode
All Mode Pages value.
*/
enum
{
	kSPCModePagePowerConditionCode		= 0x1A,
	kSPCModePageAllPagesCode			= 0x3F
};

/*!
@struct SPCModePagePowerCondition
@discussion
Power Conditions Mode Page (PAGE CODE 0x1A) format.
*/
typedef struct SPCModePagePowerCondition
{
	ModePageFormatHeader	header;
	UInt8					RESERVED;
	UInt8					IDLE_STANDBY;
	UInt32					IDLE_CONDITION_TIMER;
	UInt32					STANDBY_CONDITION_TIMER;
} SPCModePagePowerCondition;


#if 0
#pragma mark -
#pragma mark 0x00 SBC Direct Access Mode Pages
#pragma mark -
#endif


/*!
@enum SBC Mode Pages
@discussion
SBC Mode Page definitions.
@constant kSBCModePageFormatDeviceCode
Format Device Mode Page value.
@constant kSBCModePageRigidDiskGeometryCode
Rigid Disk Geometry Page value.
@constant kSBCModePageFlexibleDiskCode
Flexible Disk Page value.
@constant kSBCModePageCachingCode
Caching Page value.
*/
enum
{
	kSBCModePageFormatDeviceCode		= 0x03,
	kSBCModePageRigidDiskGeometryCode	= 0x04,
	kSBCModePageFlexibleDiskCode		= 0x05,
	kSBCModePageCachingCode				= 0x08
};


/*!
@struct SBCModePageFormatDevice
@discussion
Format Device Mode Page (PAGE CODE 0x03) format.
*/
typedef struct SBCModePageFormatDevice
{
	ModePageFormatHeader	header;
	UInt16					TRACKS_PER_ZONE;
	UInt16					ALTERNATE_SECTORS_PER_ZONE;
	UInt16					ALTERNATE_TRACKS_PER_ZONE;
	UInt16					ALTERNATE_TRACKS_PER_LOGICAL_UNIT;
	UInt16					SECTORS_PER_TRACK;
	UInt16					DATA_BYTES_PER_PHYSICAL_SECTOR;
	UInt16					INTERLEAVE;
	UInt16					TRACK_SKEW_FACTOR;
	UInt16					CYLINDER_SKEW_FACTOR;
	UInt8					SSEC_HSEC_RMB_SURF;
	UInt8					RESERVED[3];
} SBCModePageFormatDevice;


/*!
@struct SBCModePageRigidDiskGeometry
@discussion
Rigid Disk Geometry Mode Page (PAGE CODE 0x04) format.
*/
typedef struct SBCModePageRigidDiskGeometry
{
	ModePageFormatHeader	header;
	UInt8					NUMBER_OF_CYLINDERS[3];
	UInt8					NUMBER_OF_HEADS;
	UInt8					STARTING_CYLINDER_WRITE_PRECOMPENSATION[3];
	UInt8					STARTING_CYLINDER_REDUCED_WRITE_CURRENT[3];
	UInt16					DEVICE_STEP_RATE;
	UInt8					LANDING_ZONE_CYLINDER[3];
	UInt8					RPL;
	UInt8					ROTATIONAL_OFFSET;
	UInt8					RESERVED;
	UInt16					MEDIUM_ROTATION_RATE;
	UInt8					RESERVED1[2];
} SBCModePageRigidDiskGeometry;


/*!
@enum Rigid Disk Geometry bitfields
@discussion
Bit field masks for Rigid Disk Geometry structure fields.
@constant kSBCModePageRigidDiskGeometry_RPL_Mask
Mask for use with the RPL field.
*/
enum
{
	kSBCModePageRigidDiskGeometry_RPL_Mask	= 0x03
};


/*!
@struct SBCModePageFlexibleDisk
@discussion
Flexible Disk Mode Page (PAGE CODE 0x05) format.
*/
typedef struct SBCModePageFlexibleDisk
{
	ModePageFormatHeader	header;
	UInt16					TRANSFER_RATE;
	UInt8					NUMBER_OF_HEADS;
	UInt8					SECTORS_PER_TRACK;
	UInt16					DATA_BYTES_PER_SECTOR;
	UInt16					NUMBER_OF_CYLINDERS;
	UInt16					STARTING_CYLINDER_WRITE_PRECOMPENSATION;
	UInt16					STARTING_CYLINDER_REDUCED_WRITE_CURRENT;
	UInt16					DEVICE_STEP_RATE;
	UInt8					DEVICE_STEP_PULSE_WIDTH;
	UInt16					HEAD_SETTLE_DELAY;
	UInt8					MOTOR_ON_DELAY;
	UInt8					MOTOR_OFF_DELAY;
	UInt8					TRDY_SSN_MO;
	UInt8					SPC;
	UInt8					WRITE_COMPENSATION;
	UInt8					HEAD_LOAD_DELAY;
	UInt8					HEAD_UNLOAD_DELAY;
	UInt8					PIN_34_PIN_2;
	UInt8					PIN_4_PIN_1;
	UInt16					MEDIUM_ROTATION_RATE;
	UInt8					RESERVED[2];
} SBCModePageFlexibleDisk;


/*!
@enum TRDY_SSN_MO bitfields
@discussion
Bit field definitions and masks for Flexible Disk TRDY_SSN_MO field.
@constant kSBCModePageFlexibleDisk_MO_Bit
MO Bit definition.
@constant kSBCModePageFlexibleDisk_SSN_Bit
SSN Bit definition.
@constant kSBCModePageFlexibleDisk_TRDY_Bit
TRDY Bit definition.
@constant kSBCModePageFlexibleDisk_MO_Mask
Mask for use with TRDY_SSN_MO field.
@constant kSBCModePageFlexibleDisk_SSN_Mask
Mask for use with TRDY_SSN_MO field.
@constant kSBCModePageFlexibleDisk_TRDY_Mask
Mask for use with TRDY_SSN_MO field.
*/
enum
{
	// Bits 0:4 Reserved
	kSBCModePageFlexibleDisk_MO_Bit		= 5,
	kSBCModePageFlexibleDisk_SSN_Bit	= 6,
	kSBCModePageFlexibleDisk_TRDY_Bit	= 7,
	
	kSBCModePageFlexibleDisk_MO_Mask	= (1 << kSBCModePageFlexibleDisk_MO_Bit),
	kSBCModePageFlexibleDisk_SSN_Mask	= (1 << kSBCModePageFlexibleDisk_SSN_Bit),
	kSBCModePageFlexibleDisk_TRDY_Mask	= (1 << kSBCModePageFlexibleDisk_TRDY_Bit)
};


/*!
@enum SPC bitfields
@discussion
Bit field definitions and masks for Flexible Disk SPC field.
@constant kSBCModePageFlexibleDisk_SPC_Mask
Mask for use with SPC field.
*/
enum
{
	kSBCModePageFlexibleDisk_SPC_Mask	= 0x0F
};


/*!
@enum PIN_34_PIN_2 bitfields
@discussion
Bit field definitions and masks for Flexible Disk PIN_34_PIN_2 field.
@constant kSBCModePageFlexibleDisk_PIN_2_Mask
Mask for use with PIN_34_PIN_2 field.
@constant kSBCModePageFlexibleDisk_PIN_34_Mask
Mask for use with PIN_34_PIN_2 field.
*/
enum
{
	kSBCModePageFlexibleDisk_PIN_2_Mask		= 0x0F,
	kSBCModePageFlexibleDisk_PIN_34_Mask	= 0xF0
};


/*!
@enum PIN_4_PIN_1 bitfields
@discussion
Bit field definitions and masks for Flexible Disk PIN_4_PIN_1 field.
@constant kSBCModePageFlexibleDisk_PIN_1_Mask
Mask for use with PIN_4_PIN_1 field.
@constant kSBCModePageFlexibleDisk_PIN_4_Mask
Mask for use with PIN_4_PIN_1 field.
*/
enum
{
	kSBCModePageFlexibleDisk_PIN_1_Mask		= 0x0F,
	kSBCModePageFlexibleDisk_PIN_4_Mask		= 0xF0
};


/*!
@struct SBCModePageCaching
@discussion
Caching Mode Page (PAGE CODE 0x08) format.
*/
typedef struct SBCModePageCaching
{
	ModePageFormatHeader	header;
	UInt8					flags;
	UInt8					DEMAND_READ_WRITE_RETENTION_PRIORITY;
	UInt16					DISABLE_PREFETCH_TRANSFER_LENGTH;
	UInt16					MINIMUM_PREFETCH;
	UInt16					MAXIMUM_PREFETCH;
	UInt16					MAXIMUM_PREFETCH_CEILING;
	UInt8					flags2;
	UInt8					NUMBER_OF_CACHE_SEGMENTS;
	UInt16					CACHE_SEGMENT_SIZE;
	UInt8					RESERVED;
	UInt8					NON_CACHE_SEGMENT_SIZE[3];
} SBCModePageCaching;


/*!
@enum Caching flags bitfields
@discussion
Bit field definitions and masks for Caching flags field.
@constant kSBCModePageCaching_RCD_Bit
RCD Bit definition.
@constant kSBCModePageCaching_MF_Bit
MF Bit definition.
@constant kSBCModePageCaching_WCE_Bit
WCE Bit definition.
@constant kSBCModePageCaching_SIZE_Bit
SIZE Bit definition.
@constant kSBCModePageCaching_DISC_Bit
DISC Bit definition.
@constant kSBCModePageCaching_CAP_Bit
CAP Bit definition.
@constant kSBCModePageCaching_ABPF_Bit
ABPF Bit definition.
@constant kSBCModePageCaching_IC_Bit
IC Bit definition.
@constant kSBCModePageCaching_RCD_Mask
Mask for use with flags field.
@constant kSBCModePageCaching_MF_Mask
Mask for use with flags field.
@constant kSBCModePageCaching_WCE_Mask
Mask for use with flags field.
@constant kSBCModePageCaching_SIZE_Mask
Mask for use with flags field.
@constant kSBCModePageCaching_DISC_Mask
Mask for use with flags field.
@constant kSBCModePageCaching_CAP_Mask
Mask for use with flags field.
@constant kSBCModePageCaching_ABPF_Mask
Mask for use with flags field.
@constant kSBCModePageCaching_IC_Mask
Mask for use with flags field.
*/
enum
{
	kSBCModePageCaching_RCD_Bit		= 0,
	kSBCModePageCaching_MF_Bit		= 1,
	kSBCModePageCaching_WCE_Bit		= 2,
	kSBCModePageCaching_SIZE_Bit	= 3,
	kSBCModePageCaching_DISC_Bit	= 4,
	kSBCModePageCaching_CAP_Bit		= 5,
	kSBCModePageCaching_ABPF_Bit	= 6,
	kSBCModePageCaching_IC_Bit		= 7,

	kSBCModePageCaching_RCD_Mask	= (1 << kSBCModePageCaching_RCD_Bit),
	kSBCModePageCaching_MF_Mask		= (1 << kSBCModePageCaching_MF_Bit),
	kSBCModePageCaching_WCE_Mask	= (1 << kSBCModePageCaching_WCE_Bit),
	kSBCModePageCaching_SIZE_Mask	= (1 << kSBCModePageCaching_SIZE_Bit),
	kSBCModePageCaching_DISC_Mask	= (1 << kSBCModePageCaching_DISC_Bit),
	kSBCModePageCaching_CAP_Mask	= (1 << kSBCModePageCaching_CAP_Bit),
	kSBCModePageCaching_ABPF_Mask	= (1 << kSBCModePageCaching_ABPF_Bit),
	kSBCModePageCaching_IC_Mask		= (1 << kSBCModePageCaching_IC_Bit)
};


/*!
@enum Demand Read/Write Retention masks
@discussion
Demand Read/Write Retention masks.
@constant kSBCModePageCaching_DEMAND_WRITE_Mask
Mask for the DEMAND_READ_WRITE_RETENTION_PRIORITY field.
@constant kSBCModePageCaching_DEMAND_READ_Mask
Mask for the DEMAND_READ_WRITE_RETENTION_PRIORITY field.
*/
enum
{
	kSBCModePageCaching_DEMAND_WRITE_Mask	= 0x00FF,
	kSBCModePageCaching_DEMAND_READ_Mask 	= 0xFF00
};

/*!
@enum Caching flags2 bitfields
@discussion
Bit field definitions and masks for Caching flags2 field.
@constant kSBCModePageCaching_VS1_Bit
VS1 Bit definition.
@constant kSBCModePageCaching_VS2_Bit
VS2 Bit definition.
@constant kSBCModePageCaching_DRA_Bit
DRA Bit definition.
@constant kSBCModePageCaching_LBCSS_Bit
LBCSS Bit definition.
@constant kSBCModePageCaching_FSW_Bit
FSW Bit definition.
@constant kSBCModePageCaching_VS1_Mask
Mask for use with flags2 field.
@constant kSBCModePageCaching_VS2_Mask
Mask for use with flags2 field.
@constant kSBCModePageCaching_DRA_Mask
Mask for use with flags2 field.
@constant kSBCModePageCaching_LBCSS_Mask
Mask for use with flags2 field.
@constant kSBCModePageCaching_FSW_Mask
Mask for use with flags2 field.
*/
enum
{
	// Bits 0:2 Reserved
	kSBCModePageCaching_VS1_Bit		= 3,
	kSBCModePageCaching_VS2_Bit		= 4,
	kSBCModePageCaching_DRA_Bit		= 5,
	kSBCModePageCaching_LBCSS_Bit	= 6,
	kSBCModePageCaching_FSW_Bit		= 7,

	kSBCModePageCaching_VS1_Mask	= (1 << kSBCModePageCaching_VS1_Bit),
	kSBCModePageCaching_VS2_Mask	= (1 << kSBCModePageCaching_VS2_Bit),
	kSBCModePageCaching_DRA_Mask	= (1 << kSBCModePageCaching_DRA_Bit),
	kSBCModePageCaching_LBCSS_Mask	= (1 << kSBCModePageCaching_LBCSS_Bit),
	kSBCModePageCaching_FSW_Mask	= (1 << kSBCModePageCaching_FSW_Bit)
};

#pragma options align=reset

#endif	/* _IOKIT_SCSI_CMDS_MODE_DEFINITIONS_H_ */
