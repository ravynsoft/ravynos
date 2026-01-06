/*
 * Copyright (c) 2004-2009 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_SCSI_CMDS_REPORT_LUNS_DEFINITIONS_H_
#define _IOKIT_SCSI_CMDS_REPORT_LUNS_DEFINITIONS_H_


#if KERNEL
#include <IOKit/IOTypes.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#endif


/*! @header SCSI Request Sense Definitions
	@discussion
	This file contains all definitions for the data returned from
	the REPORT_LUNS (0xA0) command.
*/


/*!
@struct SCSICmd_REPORT_LUNS_LUN_ENTRY
@discussion
This structure represents a single LUN entry in a LUN list
returned via the REPORT_LUNS command.
*/
typedef struct SCSICmd_REPORT_LUNS_LUN_ENTRY
{
	UInt16		FIRST_LEVEL_ADDRESSING;
	UInt16		SECOND_LEVEL_ADDRESSING;
	UInt16		THIRD_LEVEL_ADDRESSING;
	UInt16		FOURTH_LEVEL_ADDRESSING;
} SCSICmd_REPORT_LUNS_LUN_ENTRY;


/*!
@constant kREPORT_LUNS_HeaderSize
@discussion
Size of the REPORT_LUNS header as defined in the SPC-3 specification.
*/
#define kREPORT_LUNS_HeaderSize		8

/*!
@enum REPORT_LUNS addressing methods.
@discussion
REPORT_LUNS addressing methods described in
SAM-2 documents.
@constant kREPORT_LUNS_ADDRESS_METHOD_PERIPHERAL_DEVICE
Peripheral Device Addressing Method.
@constant kREPORT_LUNS_ADDRESS_DEVICE_TYPE_SPECIFIC
Device Type Specific Addressing Method.
@constant kREPORT_LUNS_ADDRESS_METHOD_LOGICAL_UNIT
Logical Unit Specific Addressing Method.
@constant kREPORT_LUNS_ADDRESS_METHOD_OFFSET
Offset to the address method data.
*/
enum
{
	kREPORT_LUNS_ADDRESS_METHOD_PERIPHERAL_DEVICE	= 0,
	kREPORT_LUNS_ADDRESS_METHOD_FLAT_SPACE			= 1,
	kREPORT_LUNS_ADDRESS_DEVICE_TYPE_SPECIFIC		= kREPORT_LUNS_ADDRESS_METHOD_FLAT_SPACE,
	kREPORT_LUNS_ADDRESS_METHOD_LOGICAL_UNIT 		= 2,
	// Reserved [3]
	kREPORT_LUNS_ADDRESS_METHOD_OFFSET				= 14
};


/*!
@struct REPORT_LUNS_LOGICAL_UNIT_ADDRESSING
@discussion
This structure represents a LUN Addressing scheme.
*/
typedef struct REPORT_LUNS_LOGICAL_UNIT_ADDRESSING
{
#ifdef __LITTLE_ENDIAN__
	UInt16		LUN			: 5;
	UInt16		BUS_NUMBER	: 3;
	UInt16		TARGET		: 6;
	UInt16		reserved2	: 1;
	UInt16		reserved	: 1;
#else /* !__LITTLE_ENDIAN__ */
	UInt16		reserved	: 1;
	UInt16		reserved2	: 1;
	UInt16		TARGET		: 6;
	UInt16		BUS_NUMBER	: 3;
	UInt16		LUN			: 5;
#endif /* !__LITTLE_ENDIAN__ */
} REPORT_LUNS_LOGICAL_UNIT_ADDRESSING;


/*!
@struct REPORT_LUNS_PERIPHERAL_DEVICE_ADDRESSING
@discussion
This structure represents a Peripheral Device Addressing scheme.
*/
typedef struct REPORT_LUNS_PERIPHERAL_DEVICE_ADDRESSING
{
#ifdef __LITTLE_ENDIAN__
	UInt16		TARGET_LUN		: 8;
	UInt16		BUS_IDENTIFIER	: 6;
	UInt16		reserved2		: 1;
	UInt16		reserved		: 1;
#else /* !__LITTLE_ENDIAN__ */
	UInt16		reserved		: 1;
	UInt16		reserved2		: 1;
	UInt16		BUS_IDENTIFIER	: 6;
	UInt16		TARGET_LUN		: 8;
#endif /* !__LITTLE_ENDIAN__ */
} REPORT_LUNS_PERIPHERAL_DEVICE_ADDRESSING;


/*!
@struct SCSICmd_REPORT_LUNS_Header
@discussion
This structure defines the format of the data that is returned for
the REPORT_LUNS command.
*/
typedef struct SCSICmd_REPORT_LUNS_Header
{
	UInt32							LUN_LIST_LENGTH;	// LUN list length in bytes.
	UInt32							RESERVED;
	SCSICmd_REPORT_LUNS_LUN_ENTRY	LUN[1];				// Variable length list. Must have at least LUN 0 if
} SCSICmd_REPORT_LUNS_Header;							// Target supports REPORT_LUNS command.


#endif	/* _IOKIT_SCSI_CMDS_REPORT_LUNS_DEFINITIONS_H_ */
