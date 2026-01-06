/*
 * Copyright (c) 2001-2009 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_SCSI_COMMAND_DEFINITIONS_H_
#define _IOKIT_SCSI_COMMAND_DEFINITIONS_H_


#if KERNEL
#include <IOKit/IOTypes.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#endif


/*! @header SCSICommandDefinitions
	@discussion
	This file contains all the definitions for types and constants that are
	used by the command set classes for building CDBs. The field type
	definitions are used for the parameters passed to a method that builds and
	sends any SCSI defined command to clearly identify the type of value
	expected for a parameter.
	
	The command methods will then use the appropriate mask to verify that the
	value passed into a parameter is of the specified type.
	
	Currently only types and masks are defined for 8 bytes and smaller fields.
	If a command is defined that uses a larger field, these should be expanded
	to include those sizes.
*/ 

#pragma mark Field Type Definitions
/* These are the type definitions used for the parameters of methods that
 * build and send Command Descriptor Blocks.
 */

/* 1 Byte or smaller fields. */

/*! @typedef SCSICmdField1Bit */
typedef UInt8	SCSICmdField1Bit;

/*! @typedef SCSICmdField2Bit */
typedef UInt8	SCSICmdField2Bit;

/*! @typedef SCSICmdField3Bit */
typedef UInt8	SCSICmdField3Bit;

/*! @typedef SCSICmdField4Bit */
typedef UInt8	SCSICmdField4Bit;

/*! @typedef SCSICmdField5Bit */
typedef UInt8	SCSICmdField5Bit;

/*! @typedef SCSICmdField6Bit */
typedef UInt8	SCSICmdField6Bit;

/*! @typedef SCSICmdField7Bit */
typedef UInt8	SCSICmdField7Bit;

/*! @typedef SCSICmdField1Byte */
typedef UInt8	SCSICmdField1Byte;

/* 2 Bytes or smaller fields. */

/*! @typedef SCSICmdField9Bit */
typedef UInt16	SCSICmdField9Bit;

/*! @typedef SCSICmdField10Bit */
typedef UInt16	SCSICmdField10Bit;

/*! @typedef SCSICmdField11Bit */
typedef UInt16	SCSICmdField11Bit;

/*! @typedef SCSICmdField12Bit */
typedef UInt16	SCSICmdField12Bit;

/*! @typedef SCSICmdField13Bit */
typedef UInt16	SCSICmdField13Bit;

/*! @typedef SCSICmdField14Bit */
typedef UInt16	SCSICmdField14Bit;

/*! @typedef SCSICmdField15Bit */
typedef UInt16	SCSICmdField15Bit;

/*! @typedef SCSICmdField2Byte */
typedef UInt16	SCSICmdField2Byte;

/* 3 Bytes or smaller fields. */

/*! @typedef SCSICmdField17Bit */
typedef UInt32	SCSICmdField17Bit;

/*! @typedef SCSICmdField18Bit */
typedef UInt32	SCSICmdField18Bit;

/*! @typedef SCSICmdField19Bit */
typedef UInt32	SCSICmdField19Bit;

/*! @typedef SCSICmdField20Bit */
typedef UInt32	SCSICmdField20Bit;

/*! @typedef SCSICmdField21Bit */
typedef UInt32	SCSICmdField21Bit;

/*! @typedef SCSICmdField22Bit */
typedef UInt32	SCSICmdField22Bit;

/*! @typedef SCSICmdField23Bit */
typedef UInt32	SCSICmdField23Bit;

/*! @typedef SCSICmdField3Byte */
typedef UInt32	SCSICmdField3Byte;

/* 4 Bytes or smaller fields. */

/*! @typedef SCSICmdField25Bit */
typedef UInt32	SCSICmdField25Bit;

/*! @typedef SCSICmdField26Bit */
typedef UInt32	SCSICmdField26Bit;

/*! @typedef SCSICmdField27Bit */
typedef UInt32	SCSICmdField27Bit;

/*! @typedef SCSICmdField28Bit */
typedef UInt32	SCSICmdField28Bit;

/*! @typedef SCSICmdField29Bit */
typedef UInt32	SCSICmdField29Bit;

/*! @typedef SCSICmdField30Bit */
typedef UInt32	SCSICmdField30Bit;

/*! @typedef SCSICmdField31Bit */
typedef UInt32	SCSICmdField31Bit;

/*! @typedef SCSICmdField4Byte */
typedef UInt32	SCSICmdField4Byte;

/* 5 Bytes or smaller fields. */

/*! @typedef SCSICmdField33Bit */
typedef UInt64	SCSICmdField33Bit;

/*! @typedef SCSICmdField34Bit */
typedef UInt64	SCSICmdField34Bit;

/*! @typedef SCSICmdField35Bit */
typedef UInt64	SCSICmdField35Bit;

/*! @typedef SCSICmdField36Bit */
typedef UInt64	SCSICmdField36Bit;

/*! @typedef SCSICmdField37Bit */
typedef UInt64	SCSICmdField37Bit;

/*! @typedef SCSICmdField38Bit */
typedef UInt64	SCSICmdField38Bit;

/*! @typedef SCSICmdField39Bit */
typedef UInt64	SCSICmdField39Bit;

/*! @typedef SCSICmdField5Byte */
typedef UInt64	SCSICmdField5Byte;

/* 6 Bytes or smaller fields. */

/*! @typedef SCSICmdField41Bit */
typedef UInt64	SCSICmdField41Bit;

/*! @typedef SCSICmdField42Bit */
typedef UInt64	SCSICmdField42Bit;

/*! @typedef SCSICmdField43Bit */
typedef UInt64	SCSICmdField43Bit;

/*! @typedef SCSICmdField44Bit */
typedef UInt64	SCSICmdField44Bit;

/*! @typedef SCSICmdField45Bit */
typedef UInt64	SCSICmdField45Bit;

/*! @typedef SCSICmdField46Bit */
typedef UInt64	SCSICmdField46Bit;

/*! @typedef SCSICmdField47Bit */
typedef UInt64	SCSICmdField47Bit;

/*! @typedef SCSICmdField6Byte */
typedef UInt64	SCSICmdField6Byte;

/* 7 Bytes or smaller fields. */

/*! @typedef SCSICmdField49Bit */
typedef UInt64	SCSICmdField49Bit;

/*! @typedef SCSICmdField50Bit */
typedef UInt64	SCSICmdField50Bit;

/*! @typedef SCSICmdField51Bit */
typedef UInt64	SCSICmdField51Bit;

/*! @typedef SCSICmdField52Bit */
typedef UInt64	SCSICmdField52Bit;

/*! @typedef SCSICmdField53Bit */
typedef UInt64	SCSICmdField53Bit;

/*! @typedef SCSICmdField54Bit */
typedef UInt64	SCSICmdField54Bit;

/*! @typedef SCSICmdField55Bit */
typedef UInt64	SCSICmdField55Bit;

/*! @typedef SCSICmdField7Byte */
typedef UInt64	SCSICmdField7Byte;

/* 8 Bytes or smaller fields. */

/*! @typedef SCSICmdField57Bit */
typedef UInt64	SCSICmdField57Bit;

/*! @typedef SCSICmdField58Bit */
typedef UInt64	SCSICmdField58Bit;

/*! @typedef SCSICmdField59Bit */
typedef UInt64	SCSICmdField59Bit;

/*! @typedef SCSICmdField60Bit */
typedef UInt64	SCSICmdField60Bit;

/*! @typedef SCSICmdField61Bit */
typedef UInt64	SCSICmdField61Bit;

/*! @typedef SCSICmdField62Bit */
typedef UInt64	SCSICmdField62Bit;

/*! @typedef SCSICmdField63Bit */
typedef UInt64	SCSICmdField63Bit;

/*! @typedef SCSICmdField8Byte */
typedef UInt64	SCSICmdField8Byte;


#pragma mark Field Mask Definitions
/* These are masks that are used to verify that the values passed into the
 * parameters for the fields are not larger than the field size.
 *
 * NB: These have changed from enums to #define since enums greater than
 * 32 bits in size are not well-defined in C99.
 */

/* 1 Byte or smaller fields. */

/*! @constant kSCSICmdFieldMask1Bit */
#define 	kSCSICmdFieldMask1Bit		0x01

/*! @constant kSCSICmdFieldMask2Bit */
#define 	kSCSICmdFieldMask2Bit		0x03

/*! @constant kSCSICmdFieldMask3Bit */
#define 	kSCSICmdFieldMask3Bit		0x07

/*! @constant kSCSICmdFieldMask4Bit */
#define 	kSCSICmdFieldMask4Bit		0x0F

/*! @constant kSCSICmdFieldMask5Bit */
#define 	kSCSICmdFieldMask5Bit		0x1F

/*! @constant kSCSICmdFieldMask6Bit */
#define 	kSCSICmdFieldMask6Bit		0x3F

/*! @constant kSCSICmdFieldMask7Bit */
#define 	kSCSICmdFieldMask7Bit		0x7F

#define 	kSCSICmdFieldMask1Byte		0xFF

/* 2 Bytes or smaller fields. */

/*! @constant kSCSICmdFieldMask9Bit */
#define 	kSCSICmdFieldMask9Bit		0x01FF

/*! @constant kSCSICmdFieldMask10Bit */
#define 	kSCSICmdFieldMask10Bit		0x03FF

/*! @constant kSCSICmdFieldMask11Bit */
#define 	kSCSICmdFieldMask11Bit		0x07FF

/*! @constant kSCSICmdFieldMask12Bit */
#define 	kSCSICmdFieldMask12Bit		0x0FFF

/*! @constant kSCSICmdFieldMask13Bit */
#define 	kSCSICmdFieldMask13Bit		0x1FFF

/*! @constant kSCSICmdFieldMask14Bit */
#define 	kSCSICmdFieldMask14Bit		0x3FFF

/*! @constant kSCSICmdFieldMask15Bit */
#define 	kSCSICmdFieldMask15Bit		0x7FFF

/*! @constant kSCSICmdFieldMask2Byte */
#define 	kSCSICmdFieldMask2Byte		0xFFFF

/* 3 Bytes or smaller fields. */

/*! @constant kSCSICmdFieldMask17Bit */
#define 	kSCSICmdFieldMask17Bit		0x01FFFF

/*! @constant kSCSICmdFieldMask18Bit */
#define 	kSCSICmdFieldMask18Bit		0x03FFFF

/*! @constant kSCSICmdFieldMask19Bit */
#define 	kSCSICmdFieldMask19Bit		0x07FFFF

/*! @constant kSCSICmdFieldMask20Bit */
#define 	kSCSICmdFieldMask20Bit		0x0FFFFF

/*! @constant kSCSICmdFieldMask21Bit */
#define 	kSCSICmdFieldMask21Bit		0x1FFFFF

/*! @constant kSCSICmdFieldMask22Bit */
#define 	kSCSICmdFieldMask22Bit		0x3FFFFF

/*! @constant kSCSICmdFieldMask23Bit */
#define 	kSCSICmdFieldMask23Bit		0x7FFFFF

/*! @constant kSCSICmdFieldMask3Byte */
#define 	kSCSICmdFieldMask3Byte		0xFFFFFF

/* 4 Bytes or smaller fields. */
/*! @constant kSCSICmdFieldMask25Bit */
#define 	kSCSICmdFieldMask25Bit		0x01FFFFFFUL

/*! @constant kSCSICmdFieldMask26Bit */
#define 	kSCSICmdFieldMask26Bit		0x03FFFFFFUL

/*! @constant kSCSICmdFieldMask27Bit */
#define 	kSCSICmdFieldMask27Bit		0x07FFFFFFUL

/*! @constant kSCSICmdFieldMask28Bit */
#define 	kSCSICmdFieldMask28Bit		0x0FFFFFFFUL

/*! @constant kSCSICmdFieldMask29Bit */
#define 	kSCSICmdFieldMask29Bit		0x1FFFFFFFUL

/*! @constant kSCSICmdFieldMask30Bit */
#define 	kSCSICmdFieldMask30Bit		0x3FFFFFFFUL

/*! @constant kSCSICmdFieldMask31Bit */
#define 	kSCSICmdFieldMask31Bit		0x7FFFFFFFUL

/*! @constant kSCSICmdFieldMask4Byte */
#define 	kSCSICmdFieldMask4Byte		0xFFFFFFFFUL

/* 5 Bytes or smaller fields. */

/*! @constant kSCSICmdFieldMask33Bit */
#define 	kSCSICmdFieldMask33Bit		0x01FFFFFFFFULL

/*! @constant kSCSICmdFieldMask34Bit */
#define 	kSCSICmdFieldMask34Bit		0x03FFFFFFFFULL

/*! @constant kSCSICmdFieldMask35Bit */
#define 	kSCSICmdFieldMask35Bit		0x07FFFFFFFFULL

/*! @constant kSCSICmdFieldMask36Bit */
#define 	kSCSICmdFieldMask36Bit		0x0FFFFFFFFFULL

/*! @constant kSCSICmdFieldMask37Bit */
#define 	kSCSICmdFieldMask37Bit		0x1FFFFFFFFFULL

/*! @constant kSCSICmdFieldMask38Bit */
#define 	kSCSICmdFieldMask38Bit		0x3FFFFFFFFFULL

/*! @constant kSCSICmdFieldMask39Bit */
#define 	kSCSICmdFieldMask39Bit		0x7FFFFFFFFFULL

/*! @constant kSCSICmdFieldMask5Byte */
#define 	kSCSICmdFieldMask5Byte		0xFFFFFFFFFFULL

/* 6 Bytes or smaller fields. */

/*! @constant kSCSICmdFieldMask41Bit */
#define 	kSCSICmdFieldMask41Bit		0x01FFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask42Bit */
#define 	kSCSICmdFieldMask42Bit		0x03FFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask43Bit */
#define 	kSCSICmdFieldMask43Bit		0x07FFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask44Bit */
#define 	kSCSICmdFieldMask44Bit		0x0FFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask45Bit */
#define 	kSCSICmdFieldMask45Bit		0x1FFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask46Bit */
#define 	kSCSICmdFieldMask46Bit		0x3FFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask47Bit */
#define 	kSCSICmdFieldMask47Bit		0x7FFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask6Byte */
#define 	kSCSICmdFieldMask6Byte		0xFFFFFFFFFFFFULL

/* 7 Bytes or smaller fields. */

/*! @constant kSCSICmdFieldMask49Bit */
#define 	kSCSICmdFieldMask49Bit		0x01FFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask50Bit */
#define 	kSCSICmdFieldMask50Bit		0x03FFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask51Bit */
#define 	kSCSICmdFieldMask51Bit		0x07FFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask52Bit */
#define 	kSCSICmdFieldMask52Bit		0x0FFFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask53Bit */
#define 	kSCSICmdFieldMask53Bit		0x1FFFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask54Bit */
#define 	kSCSICmdFieldMask54Bit		0x3FFFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask55Bit */
#define 	kSCSICmdFieldMask55Bit		0x7FFFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask7Byte */
#define 	kSCSICmdFieldMask7Byte		0xFFFFFFFFFFFFFFULL

/* 8 Bytes or smaller fields. */

/*! @constant kSCSICmdFieldMask57Bit */
#define 	kSCSICmdFieldMask57Bit		0x01FFFFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask58Bit */
#define 	kSCSICmdFieldMask58Bit		0x03FFFFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask59Bit */
#define 	kSCSICmdFieldMask59Bit		0x07FFFFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask60Bit */
#define 	kSCSICmdFieldMask60Bit		0x0FFFFFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask61Bit */
#define 	kSCSICmdFieldMask61Bit		0x1FFFFFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask62Bit */
#define 	kSCSICmdFieldMask62Bit		0x3FFFFFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask63Bit */
#define 	kSCSICmdFieldMask63Bit		0x7FFFFFFFFFFFFFFFULL

/*! @constant kSCSICmdFieldMask8Byte */
#define 	kSCSICmdFieldMask8Byte		0xFFFFFFFFFFFFFFFFULL

#endif	/* _IOKIT_SCSI_COMMAND_DEFINITIONS_H_ */
