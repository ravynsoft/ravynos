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


#ifndef __ATA_SMART_LIB_H__
#define __ATA_SMART_LIB_H__


/*! @defined kIOPropertySMARTCapableKey
	@discussion Property to search for in IORegistry to find SMART capable devices
	without hardcoding the search to a particular device class. */
#define kIOPropertySMARTCapableKey		"SMART Capable"


#include <IOKit/IOReturn.h>
#include <IOKit/IOTypes.h>


#if !KERNEL
	#include <CoreFoundation/CFPlugIn.h>
	#if COREFOUNDATION_CFPLUGINCOM_SEPARATE
		#include <CoreFoundation/CFPlugInCOM.h>
	#endif
	
	#include <IOKit/IOCFPlugIn.h>
	#include <IOKit/storage/ata/IOATAStorageDefines.h>
	
#ifdef __cplusplus
extern "C" {
#endif


/*! @header ATASMARTLib
 ATASMARTLib implements non-kernel task access to ATA SMART data.
*/


/* 5E659F92-20D3-11D6-BDB5-0003935A76B2 */
/*! @defined kIOATASMARTLibFactoryID
    @discussion UUID for the IOATASMARTInterface Factory. */

#define kIOATASMARTLibFactoryID			CFUUIDGetConstantUUIDWithBytes(NULL,				\
										0x5E, 0x65, 0x9F, 0x92, 0x20, 0xD3, 0x11, 0xD6,		\
										0xBD, 0xB5, 0x00, 0x03, 0x93, 0x5A, 0x76, 0xB2)


/* 24514B7A-2804-11D6-8A02-003065704866 */
/*! @defined kIOATASMARTUserClientTypeID
    @discussion Factory ID for creating an ATA SMART user client. */
#define kIOATASMARTUserClientTypeID		CFUUIDGetConstantUUIDWithBytes(NULL,				\
										0x24, 0x51, 0x4B, 0x7A, 0x28, 0x04, 0x11, 0xD6,		\
										0x8A, 0x02, 0x00, 0x30, 0x65, 0x70, 0x48, 0x66)

/* 08ABE21C-20D4-11D6-8DF6-0003935A76B2 */
/*! @defined kIOATASMARTInterfaceID
    @discussion InterfaceID for IOATASMARTInterface. */
#define kIOATASMARTInterfaceID			CFUUIDGetConstantUUIDWithBytes(NULL,				\
										0x08, 0xAB, 0xE2, 0x1C, 0x20, 0xD4, 0x11, 0xD6,		\
										0x8D, 0xF6, 0x00, 0x03, 0x93, 0x5A, 0x76, 0xB2)


#endif	/* !KERNEL */


// Off-line data collection status byte values ( offset 362 into 
// device SMART data structure ) See section 8.54.5.8.1 of ATA/ATAPI-6.
enum
{
	kATASMARTOffLineCollectionNeverStarted		= 0x00,
	kATASMARTOffLineCollectionNoError			= 0x02,
	kATASMARTOffLineCollectionSuspendedByHost	= 0x04,
	kATASMARTOffLineCollectionAbortedByHost		= 0x05,
	kATASMARTOffLineCollectionFatalError		= 0x06,
	
	kATASMARTOffLineCollectionMask				= 0x7F
};

// Self-test execution status values ( offset 363 into 
// Device SMART data structure ) See section 8.54.5.8.2 of ATA/ATAPI-6.
enum
{
	kATASMARTSelfTestStatusNoError 							= 0x00,
	kATASMARTSelfTestStatusAbortedByHost 					= 0x01,
	kATASMARTSelfTestStatusInterruptedByReset				= 0x02,
	kATASMARTSelfTestStatusFatalError 						= 0x03,
	kATASMARTSelfTestStatusPreviousTestUnknownFailure 		= 0x04,
	kATASMARTSelfTestStatusPreviousTestElectricalFailure	= 0x05,
	kATASMARTSelfTestStatusPreviousTestServoFailure			= 0x06,
	kATASMARTSelfTestStatusPreviousTestReadFailure			= 0x07,
	kATASMARTSelfTestStatusInProgress						= 0x0F
};


/*
 *	512 byte Device SMART data structure - 
 *	See section 8.54.5.8 of T13:1410D (ATA/ATAPI-6).
 */
typedef struct ATASMARTData
{
	UInt8		vendorSpecific1[362];
	UInt8		offLineDataCollectionStatus;
	UInt8		selfTestExecutionStatus;
	UInt8		secondsToCompleteOffLineActivity[2];
	UInt8		vendorSpecific2;
	UInt8		offLineDataCollectionCapability;
	UInt8		SMARTCapability[2];
	UInt8		errorLoggingCapability;
	UInt8		vendorSpecific3;
	UInt8		shortTestPollingInterval;		/* expressed in minutes */
	UInt8		extendedTestPollingInterval; 	/* expressed in minutes */
	UInt8		reserved[12];
	UInt8		vendorSpecific4[125];
	UInt8		checksum;
} ATASMARTData;


/*
 *	512 byte Device SMART data thresholds structure. Not
 *	strictly part of ATA/ATAPI-6.
 */
typedef struct ATASMARTDataThresholds
{
	UInt8		vendorSpecific1[362];
	UInt8		vendorSpecific2[149];
	UInt8		checksum;
} ATASMARTDataThresholds;


typedef struct ATASMARTLogEntry
{
	UInt8	numberOfSectors;
	UInt8	reserved;
} ATASMARTLogEntry;


typedef struct ATASMARTLogDirectory
{
	UInt8				SMARTLoggingVersion[2];
	ATASMARTLogEntry	entries[255];
} ATASMARTLogDirectory;


#if !KERNEL

/*! 
 *	@interface IOATASMARTInterface
 *	@abstract Self-Monitoring, Analysis, and Reporting
 *	Technology Interface.  
 *	@discussion See section 6.14 and section 8.54 of T13:1410D ATA/ATAPI-6
 *	for details on Self-Monitoring, Analysis, and Reporting Technology
 *	feature set.
 */

typedef struct IOATASMARTInterface
{
	
	IUNKNOWN_C_GUTS;
	
	UInt16		version;
	UInt16		revision;
	
	/* 
	 * MANDATORY API support. If the device claims SMART feature set compliance, it
	 * must implement the following functions.
	 */
	
	
    /*!
		@function SMARTEnableDisableOperations
		@abstract toggle SMART Operations.
		@discussion See section 8.54.1 and 8.54.3 of ATA/ATAPI-6.
		@param enable Passing true will ENABLE SMART operations,
		false will DISABLE SMART operations.
		@result Returns kIOReturnSuccess if successful, kIOReturnNoDevice
		if there is no connection to an IOService, kIOReturnExclusiveAccess
		if it is already opened by another client.
	*/
	
	IOReturn ( *SMARTEnableDisableOperations ) ( void *		interface,
												 Boolean	enable );
	
    /*!
		@function SMARTEnableDisableAutosave
		@abstract toggle SMART Autosave.
		@discussion	See section 8.54.2 of ATA/ATAPI-6.
		@param enable Passing true will ENABLE SMART Autosave, false will
		DISABLE SMART Autosave.
		@result Returns kIOReturnSuccess if successful, kIOReturnNoDevice
		if there is no connection to an IOService, kIOReturnExclusiveAccess
		if it is already opened by another client.
	*/
	
	IOReturn ( *SMARTEnableDisableAutosave ) ( void *	interface,
											   Boolean	enable );
	
    /*!
		@function SMARTReturnStatus
		@abstract see if device has detected a threshold exceeded condition.
		@discussion	The caller will poll this function and if
		exceededCondition is non-zero and we returned kIOReturnSuccess the
		device threshold exceeded condition. This would prompt the caller
		to call ATASMARTReadData to get more information. See section
		8.54.7 of ATA/ATAPI-6.
		@param exceededCondition
		if exceededCondition is non-zero the device threshold
		exceeded condition.
		@result Returns kIOReturnSuccess if successful, kIOReturnNoDevice
		if there is no connection to an IOService, kIOReturnExclusiveAccess
		if it is already opened by another client.	
	*/
	
	IOReturn ( *SMARTReturnStatus ) ( void *		interface,
									  Boolean * 	exceededCondition );
	

	/* 
	 * OPTIONAL API support. If the device claims SMART feature set compliance, it
	 * may implement one or more of the following functions. Please consult the
	 * technical manual for the device to see what functions are supported.
	 */


    /*!
		@function SMARTExecuteOffLineImmediate
		@abstract immediately initiate collection of SMART data.
		@discussion	See section 8.54.4 of ATA/ATAPI-6.
		@param extendedTest passing true will collect "off-line" extended test,
		false short test.
		@result Returns kIOReturnSuccess if successful,
		kIOReturnNoDevice if there is no connection to an IOService,
		kIOReturnExclusiveAccess if it is already opened by another client.	
	*/
	
	IOReturn ( *SMARTExecuteOffLineImmediate ) ( void *		interface,
												 Boolean	extendedTest );
	
    /*!
		@function SMARTReadData
		@abstract Retrieves 512 byte device SMART data structure.
		@discussion	See section 8.54.5 of ATA/ATAPI-6.
		Will return an appropiate error if command can not be completed.
	*/
	
	IOReturn ( *SMARTReadData ) ( void * interface, ATASMARTData * data );	
	
    /*!
		@function SMARTValidateReadData
		@abstract Test the integrity of the device SMART data structure.
		@discussion	The data structure checksum is the two's complement
		of the sum of the first 511 bytes in the data structure. The sum
		of all 512 bytes will be zero when the checksum is correct. See
		section 8.54.5.8.7 of ATA/ATAPI-6. Will return an error if
		checksum fails.
	*/
	
	IOReturn ( *SMARTValidateReadData ) ( void *				interface,
										  const ATASMARTData *	data );
	
	
    /*!
		@function SMARTReadDataThresholds
		@abstract Retrieves 512 byte device SMART data thresholds structure.
		@discussion	Retrieves 512 byte device SMART data thresholds structure.
		This command is not defined as part of ATA/ATAPI-6, but is implemented
		by a large variety of manufacturers. Will return an appropiate error if
		command can not be completed.
	*/
	
	IOReturn ( *SMARTReadDataThresholds ) ( void *						interface,
											ATASMARTDataThresholds *	dataThresholds );
	
    /*!
		@function SMARTReadLogDirectory
		@abstract Reads the 512-byte log directory.
		@discussion	The log directory is a directory of all possible
		SMART logs available from the drive.
	*/
	
	IOReturn ( *SMARTReadLogDirectory ) ( void *				 interface,
										  ATASMARTLogDirectory * logData );
	
    /*!
		@function SMARTReadLogAtAddress
		@abstract Reads the 512-byte log at the specified logOffset in the log.
		@discussion	Reads the 512-byte log at the specified logOffset in the log.
		See section 8.54.6.4 of ATA/ATAPI-6.
	*/
	
	IOReturn ( *SMARTReadLogAtAddress ) ( void *		interface,
										  UInt32		logOffset,
										  void *		buffer,
										  UInt32		size );
	
    /*!
		@function SMARTWriteLogAtAddress
		@abstract Writes to the 512-byte log at the specified logOffset in the log.
		@discussion	Writes to the 512-byte log at the specified logOffset in the log.
		See section 8.54.8.4 of ATA/ATAPI-6.
	*/
	
	IOReturn ( *SMARTWriteLogAtAddress ) ( void *			interface,
										   UInt32			logOffset,
										   const void *		buffer,
										   UInt32			size );
	
	/*!
		@function GetATAIdentifyData
		@abstract Reads the 512-byte data provided by the drive in response
		to the ATA IDENTIFY DEVICE command.
		@discussion Reads the 512-byte data provided by the drive in response
		to the ATA IDENTIFY DEVICE command.
		See section 8.15 of ATA/ATAPI-6.
		The data placed in buffer is guaranteed to be in native endian form on return.
		(i.e. it will be byte swapped on big endian platforms, so the caller need not
		do anything)
		@param interface A valid IOATASMARTInterface**.
		@param buffer A valid buffer.
		@param inSize The number of bytes to place in the buffer.
		@param outSize The number of bytes placed in the buffer. Can be NULL if the information
		is not required by the caller.
		@return An IOReturn result code. If inSize is greater than 512 or less than 1,
				kIOReturnBadArgument is returned.
	*/
	
	IOReturn ( *GetATAIdentifyData ) ( void *				interface,
									   void *				buffer,
									   UInt32				inSize,
									   UInt32 *				outSize );
	
} IOATASMARTInterface;


#ifdef __cplusplus
}
#endif

#endif	/* !KERNEL */

#endif	/* __ATA_SMART_LIB_H__ */