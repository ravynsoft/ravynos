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

#ifndef _IOKIT_SCSI_CMDS_READ_CAPACITY_H_
#define _IOKIT_SCSI_CMDS_READ_CAPACITY_H_


#if KERNEL
#include <IOKit/IOTypes.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#endif


/*! @header SCSI Request Sense Definitions
	@discussion
	This file contains all definitions for the data returned from
	the READ CAPACITY 10 (0x25) and READ CAPACITY 16 (0x9E) commands.
*/

/*!
@enum READ CAPACITY Payload Sizes
@discussion
Sizes of the payload for the READ CAPACITY 10 and
READ CAPACITY 16 commands.
@constant kREPORT_CAPACITY_DataSize
Data size for a READ_CAPACITY command.
@constant kREPORT_CAPACITY_16_DataSize
Data size for a READ_CAPACITY_16 command.
*/
enum
{
	kREPORT_CAPACITY_DataSize		= 8,
	kREPORT_CAPACITY_16_DataSize	= 32
};


/*!
@constant kREPORT_CAPACITY_MaximumLBA
@discussion
Maximum LBA supported via READ CAPACITY 10 command.
*/
#define 	kREPORT_CAPACITY_MaximumLBA			0xFFFFFFFFUL


/*!
@constant kREPORT_CAPACITY_16_MaximumLBA
@discussion
Maximum LBA supported via READ CAPACITY 16 command.
*/
#define 	kREPORT_CAPACITY_16_MaximumLBA		0xFFFFFFFFFFFFFFFFULL


/*!
@struct SCSI_Capacity_Data
@discussion
Capacity return structure for READ CAPACITY 10 command.
*/
typedef struct SCSI_Capacity_Data
{
	UInt32		RETURNED_LOGICAL_BLOCK_ADDRESS;
	UInt32		BLOCK_LENGTH_IN_BYTES;
} SCSI_Capacity_Data;


/*!
@struct SCSI_Capacity_Data_Long
@discussion
Capacity return structure for READ CAPACITY 16 command.
*/
typedef struct SCSI_Capacity_Data_Long
{
	UInt64		RETURNED_LOGICAL_BLOCK_ADDRESS;
	UInt32		BLOCK_LENGTH_IN_BYTES;
	UInt8		RTO_EN_PROT_EN;
	UInt8		Reserved[19];
} SCSI_Capacity_Data_Long;


/*!
@enum RTO_EN definitions
@discussion
Values for the REFERENCE TAG OWN (RTO_EN) bit in the
READ CAPACITY Long Data structure.
@constant kREAD_CAPACITY_RTO_Enabled
Reference Tag Own enabled.
@constant kREAD_CAPACITY_RTO_Disabled
Reference Tag Own disabled.
@constant kREAD_CAPACITY_RTO_Mask
Mask to use when checking the RTO_EN_PROT_EN field.
*/
enum
{
	kREAD_CAPACITY_RTO_Enabled								= 0x02,
	kREAD_CAPACITY_RTO_Disabled								= 0x00,
	kREAD_CAPACITY_RTO_Mask									= 0x02
};


/*!
@enum PROTECTION INFORMATION definitions
@discussion
Values for the PROTECTION INFORMATION (PROT_EN) bit in the
READ CAPACITY Long Data structure.
@constant kREAD_CAPACITY_PROT_Enabled
Protection Information enabled.
@constant kREAD_CAPACITY_PROT_Disabled
Protection Information disabled.
@constant kREAD_CAPACITY_PROT_Mask
Mask to use when checking the RTO_EN_PROT_EN field.
*/

enum
{
	kREAD_CAPACITY_PROT_Enabled								= 0x01,
	kREAD_CAPACITY_PROT_Disabled							= 0x00,
	kREAD_CAPACITY_PROT_Mask								= 0x01
};


#endif	/* _IOKIT_SCSI_CMDS_READ_CAPACITY_H_ */
