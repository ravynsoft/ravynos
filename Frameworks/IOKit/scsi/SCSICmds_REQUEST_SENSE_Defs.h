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

#ifndef _IOKIT_SCSI_CMDS_REQUEST_SENSE_H_
#define _IOKIT_SCSI_CMDS_REQUEST_SENSE_H_


#if KERNEL
#include <IOKit/IOTypes.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#endif


/*! @header SCSI Request Sense Definitions
	@discussion
	This file contains all definitions for the data returned from
	the REQUEST SENSE (0x03) command and from auto sense on protocols
	that support it.
*/


/*!
@enum kSenseDefaultSize
@discussion
The default size for SCSI Request Sense data.
*/
enum
{
	kSenseDefaultSize	= 18
};


/*!
@struct SCSI_Sense_Data
@discussion
The basic SCSI Request Sense data structure.
*/
typedef struct SCSI_Sense_Data
{
	UInt8		VALID_RESPONSE_CODE;				// 7 = Valid. 6-0 = Response Code.
	UInt8		SEGMENT_NUMBER;						// Segment number
	UInt8		SENSE_KEY;							// 7 = FILEMARK, 6 = EOM, 5 = ILI, 3-0 = SENSE KEY.
	UInt8		INFORMATION_1;						// INFORMATION.
	UInt8		INFORMATION_2;						// INFORMATION.
	UInt8		INFORMATION_3;						// INFORMATION.
	UInt8		INFORMATION_4;						// INFORMATION.
	UInt8		ADDITIONAL_SENSE_LENGTH;			// Number of additional bytes available in sense data
	UInt8		COMMAND_SPECIFIC_INFORMATION_1;		// Command Specific Information
	UInt8		COMMAND_SPECIFIC_INFORMATION_2;		// Command Specific Information
	UInt8		COMMAND_SPECIFIC_INFORMATION_3;		// Command Specific Information
	UInt8		COMMAND_SPECIFIC_INFORMATION_4;		// Command Specific Information
	UInt8		ADDITIONAL_SENSE_CODE;				// Additional Sense Code
	UInt8		ADDITIONAL_SENSE_CODE_QUALIFIER;	// Additional Sense Code Qualifier
	UInt8		FIELD_REPLACEABLE_UNIT_CODE;		// Field Replaceable Unit Code
	UInt8		SKSV_SENSE_KEY_SPECIFIC_MSB;		// 7 = Sense Key Specific Valid bit, 6-0 Sense Key Specific MSB
	UInt8		SENSE_KEY_SPECIFIC_MID;				// Sense Key Specific Middle
	UInt8		SENSE_KEY_SPECIFIC_LSB;				// Sense Key Specific LSB
} SCSI_Sense_Data;


/*!
@enum Sense Valid
@discussion
Masks to use to determine if sense data is valid or not.
@constant kSENSE_DATA_VALID
Sense data is valid.
@constant kSENSE_NOT_DATA_VALID
Sense data is not valid.
@constant kSENSE_DATA_VALID_Mask
Validity mask to use when checking the VALID_RESPONSE_CODE field.
*/
enum
{
	kSENSE_DATA_VALID										= 0x80,
	kSENSE_NOT_DATA_VALID									= 0x00,
	kSENSE_DATA_VALID_Mask									= 0x80
};


/*!
@enum Sense Response Codes
@discussion
Masks and values to determine the Response Code.
@constant kSENSE_RESPONSE_CODE_Current_Errors
Response code indicating current errors are reported.
@constant kSENSE_RESPONSE_CODE_Deferred_Errors
Response code indicating deferred errors are reported.
@constant kSENSE_RESPONSE_CODE_Mask
Mask to use when checking the VALID_RESPONSE_CODE field.
*/
enum
{
	kSENSE_RESPONSE_CODE_Current_Errors						= 0x70,
	kSENSE_RESPONSE_CODE_Deferred_Errors					= 0x71,
	kSENSE_RESPONSE_CODE_Mask								= 0x7F
};


/*!
@enum FILEMARK bit field definitions
@discussion
Masks and values to determine the FileMark bit field.
@constant kSENSE_FILEMARK_Set
Filemark bit is set.
@constant kSENSE_FILEMARK_Not_Set
Filemark bit is not set.
@constant kSENSE_FILEMARK_Mask
Mask to use when checking the SENSE_KEY field for the FILEMARK bit.
*/
enum
{
	kSENSE_FILEMARK_Set										= 0x80,
	kSENSE_FILEMARK_Not_Set 								= 0x00,
	kSENSE_FILEMARK_Mask 									= 0x80
};


/*!
@enum EOM bit field definitions
@discussion
Masks and values to determine the End Of Medium bit field.
@constant kSENSE_EOM_Set
End Of Medium bit is set.
@constant kSENSE_EOM_Not_Set
End Of Medium bit is not set.
@constant kSENSE_EOM_Mask
Mask to use when checking the SENSE_KEY field for the EOM bit.
*/
enum
{
	kSENSE_EOM_Set											= 0x40,
	kSENSE_EOM_Not_Set 										= 0x00,
	kSENSE_EOM_Mask 										= 0x40
};


/*!
@enum ILI bit field definitions
@discussion
Masks and values to determine the Incorrect Length Indicator bit field.
@constant kSENSE_ILI_Set
Incorrect Length Indicator bit is set.
@constant kSENSE_ILI_Not_Set
Incorrect Length Indicator bit is not set.
@constant kSENSE_ILI_Mask
Mask to use when checking the SENSE_KEY field for the ILI bit.
*/
enum
{
	kSENSE_ILI_Set											= 0x20,
	kSENSE_ILI_Not_Set 										= 0x00,
	kSENSE_ILI_Mask 										= 0x20
};


/*!
@enum Sense Key definitions
@discussion
Masks and values to determine the SENSE_KEY.
@constant kSENSE_KEY_NO_SENSE
No sense data is present.
@constant kSENSE_KEY_RECOVERED_ERROR
A recovered error has occurred.
@constant kSENSE_KEY_NOT_READY
Device server is not ready.
@constant kSENSE_KEY_MEDIUM_ERROR
Device server detected a medium error.
@constant kSENSE_KEY_HARDWARE_ERROR
Device server detected a hardware error.
@constant kSENSE_KEY_ILLEGAL_REQUEST
Device server detected an illegal request.
@constant kSENSE_KEY_UNIT_ATTENTION
Device server indicates a unit attention condition.
@constant kSENSE_KEY_DATA_PROTECT
Device server indicates a data protect condition.
@constant kSENSE_KEY_BLANK_CHECK
Device server indicates a blank check condition.
@constant kSENSE_KEY_VENDOR_SPECIFIC
Device server indicates a vendor specific condition.
@constant kSENSE_KEY_COPY_ABORTED
Device server indicates a copy aborted condition.
@constant kSENSE_KEY_ABORTED_COMMAND
Device server indicates an aborted command condition.
@constant kSENSE_KEY_VOLUME_OVERFLOW
Device server indicates a volume overflow condition.
@constant kSENSE_KEY_MISCOMPARE
Device server indicates a miscompare condition.
@constant kSENSE_KEY_Mask
Mask to use when checking the SENSE_KEY field for the SENSE_KEY value.
*/
enum
{
	kSENSE_KEY_NO_SENSE										= 0x00,
	kSENSE_KEY_RECOVERED_ERROR								= 0x01,
	kSENSE_KEY_NOT_READY									= 0x02,
	kSENSE_KEY_MEDIUM_ERROR									= 0x03,
	kSENSE_KEY_HARDWARE_ERROR								= 0x04,
	kSENSE_KEY_ILLEGAL_REQUEST								= 0x05,
	kSENSE_KEY_UNIT_ATTENTION								= 0x06,
	kSENSE_KEY_DATA_PROTECT									= 0x07,
	kSENSE_KEY_BLANK_CHECK									= 0x08,
	kSENSE_KEY_VENDOR_SPECIFIC								= 0x09,
	kSENSE_KEY_COPY_ABORTED									= 0x0A,
	kSENSE_KEY_ABORTED_COMMAND								= 0x0B,
	/* SENSE KEY 0x0C is obsoleted */
	kSENSE_KEY_VOLUME_OVERFLOW								= 0x0D,
	kSENSE_KEY_MISCOMPARE									= 0x0E,
	/* SENSE KEY 0x0F is reserved */
	kSENSE_KEY_Mask 										= 0x0F
};


#endif	/* _IOKIT_SCSI_CMDS_REQUEST_SENSE_H_ */
