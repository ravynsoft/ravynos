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

#ifndef __SCSI_TASK_LIB_H__
#define __SCSI_TASK_LIB_H__

#include <IOKit/scsi/SCSITask.h>
#include <IOKit/scsi/SCSICommandDefinitions.h>
#include <IOKit/scsi/SCSICmds_INQUIRY_Definitions.h>
#include <IOKit/scsi/SCSICmds_REQUEST_SENSE_Defs.h>

#if !KERNEL
	#include <CoreFoundation/CFPlugIn.h>
	#if COREFOUNDATION_CFPLUGINCOM_SEPARATE
		#include <CoreFoundation/CFPlugInCOM.h>
	#endif
	
	#include <IOKit/IOReturn.h>
	#include <IOKit/IOTypes.h>
	#include <IOKit/IOCFPlugIn.h>
	
#ifdef __cplusplus
extern "C" {
#endif


/*! @header SCSITaskLib
SCSITaskLib implements non-kernel task access to specific IOKit object types, namely
any SCSI Peripheral Device for which there isn't an in-kernel driver and for authoring
devices such as CD-R/W and DVD-R/W drives.
*/


// 7D66678E-08A2-11D5-A1B8-0030657D052A
/*! @defined kIOSCSITaskDeviceUserClientTypeID
    @discussion Factory ID for creating an SCSITask Device User Client. */

#define kIOSCSITaskDeviceUserClientTypeID	CFUUIDGetConstantUUIDWithBytes(NULL,			\
										0x7D, 0x66, 0x67, 0x8E, 0x08, 0xA2, 0x11, 0xD5,		\
										0xA1, 0xB8, 0x00, 0x30, 0x65, 0x7D, 0x05, 0x2A)

// 97ABCF2C-23CC-11D5-A0E8-003065704866
/*! @defined kIOMMCDeviceUserClientTypeID
    @discussion Factory ID for creating an MMC Device User Client. */

#define kIOMMCDeviceUserClientTypeID 														\
										CFUUIDGetConstantUUIDWithBytes(NULL,				\
										0x97, 0xAB, 0xCF, 0x2C, 0x23, 0xCC, 0x11, 0xD5,		\
										0xA0, 0xE8, 0x00, 0x30, 0x65, 0x70, 0x48, 0x66)

// 63326D72-08A2-11D5-865F-0030657D052A
/*! @defined kIOSCSITaskLibFactoryID
    @discussion UUID for the SCSITaskLib Factory. */

#define kIOSCSITaskLibFactoryID			CFUUIDGetConstantUUIDWithBytes(NULL,				\
										0x63, 0x32, 0x6D, 0x72, 0x08, 0xA2, 0x11, 0xD5,		\
										0x86, 0x5F, 0x00, 0x30, 0x65, 0x7D, 0x05, 0x2A)


//0B85B63C-462E-11D5-A9D6-003065704866
/*! @defined kIOSCSITaskInterfaceID
    @discussion InterfaceID for SCSITaskInterface. */
#define kIOSCSITaskInterfaceID			CFUUIDGetConstantUUIDWithBytes(NULL,				\
										0x0B, 0x85, 0xB6, 0x3C, 0x46, 0x2E, 0x11, 0xD5,		\
										0xA9, 0xD6, 0x00, 0x30, 0x65, 0x70, 0x48, 0x66)


// 1BBC4132-08A5-11D5-90ED-0030657D052A
/*! @defined kIOSCSITaskDeviceInterfaceID
    @discussion InterfaceID for SCSITaskDeviceInterface. */
#define kIOSCSITaskDeviceInterfaceID 														\
                                        CFUUIDGetConstantUUIDWithBytes(NULL,				\
										0x1B, 0xBC, 0x41, 0x32, 0x08, 0xA5, 0x11, 0xD5,		\
										0x90, 0xED, 0x00, 0x30, 0x65, 0x7D, 0x05, 0x2A)

// 1F651106-23CC-11D5-BBDB-003065704866
/*! @defined kIOMMCDeviceInterfaceID
    @discussion InterfaceID for MMCDeviceInterface. */
#define kIOMMCDeviceInterfaceID 															\
                                        CFUUIDGetConstantUUIDWithBytes(NULL,				\
										0x1F, 0x65, 0x11, 0x06, 0x23, 0xCC, 0x11, 0xD5,		\
										0xBB, 0xDB, 0x00, 0x30, 0x65, 0x70, 0x48, 0x66)


// The following UUID constants are deprecated because of possible name-space collisions
// Eventually this switch will be set to 0 and then removed entirely.
#define OLD_UUIDS 1
#if OLD_UUIDS

#define kSCSITaskDeviceInterfaceID		kIOSCSITaskDeviceInterfaceID
#define kMMCDeviceInterfaceID			kIOMMCDeviceInterfaceID
#define kSCSITaskInterfaceID			kIOSCSITaskInterfaceID
#define kSCSITaskDeviceUserClientTypeID	kIOSCSITaskDeviceUserClientTypeID
#define kMMCDeviceUserClientTypeID 		kIOMMCDeviceUserClientTypeID
#define kSCSITaskLibFactoryID 			kIOSCSITaskLibFactoryID

#endif /* OLD_UUIDS */


#endif /* !KERNEL */

/*! @defined kIOPropertySCSITaskUserClientInstanceGUID
    @discussion IORegistry property for the SCSITaskUserClient GUID.
	This GUID helps uniquely identify and track SCSITask enabled devices */

#define kIOPropertySCSITaskUserClientInstanceGUID	"SCSITaskUserClient GUID"

/*! @defined kIOPropertySCSITaskDeviceCategory
    @discussion IORegistry property for the SCSITaskUserClient.
	This category identifies which type of device and interface
	to the device is used in conjunction with the SCSITaskUserClient. */

#define kIOPropertySCSITaskDeviceCategory			"SCSITaskDeviceCategory"

/*! @defined kIOPropertySCSITaskUserClientDevice
    @discussion IORegistry property for the SCSI Task User Client.
	This property identifies an SCSITask enabled device. */

#define kIOPropertySCSITaskUserClientDevice			"SCSITaskUserClientDevice"

/*! @defined kIOPropertySCSITaskAuthoringDevice
    @discussion IORegistry property for the SCSI Task User Client.
	This property identifies an SCSITask enabled device capable of authoring. */

#define kIOPropertySCSITaskAuthoringDevice			"SCSITaskAuthoringDevice"

/*!
	@enum MMCDeviceTrayState
	@abstract Used to identify the state of an MMCDevice's tray (if applicable).
	@discussion Used to identify the state of an MMCDevice's tray (if applicable).
	@constant kMMCDeviceTrayClosed This value means the tray is closed.
	@constant kMMCDeviceTrayOpen This value means the tray is open.
 */

enum
{
	kMMCDeviceTrayClosed 	= 0,
	kMMCDeviceTrayOpen		= 1,
	kMMCDeviceTrayMask		= 0x1
};


#if defined(__LP64__)
typedef IOAddressRange	SCSITaskSGElement;
#else
typedef IOVirtualRange	SCSITaskSGElement;
#endif


#if !KERNEL

/*! @typedef SCSITaskCallbackFunction
    @abstract Asynchronous callback routine definition.
	@discussion Asynchronous callback routine definition. Any function which is used
	as a callback routine for SCSITasks must conform to this function definition.
	@param serviceResponse An SCSIServiceResponse returned by the protocol transport.
	@param taskStatus An SCSITaskStatus to indicate the task's status
	@param bytesTransferred A total byte count of bytes transferred.
    @param refCon The refCon passed when the task was executed.
*/

typedef void ( *SCSITaskCallbackFunction ) ( SCSIServiceResponse 	serviceResponse,
											 SCSITaskStatus 		taskStatus,
											 UInt64 				bytesTransferred,
											 void *					refCon );

/*! 
	@class SCSITaskInterface
    @abstract Basic interface for a SCSITask.  
    @discussion After rendezvous with a SCSITask Device in the IORegistry you can create
    an instance of this interface using the CreateSCSITask method in the
    SCSITaskDeviceInterface. Once you have this interface, or one of its subclasses,
    you can manipulate SCSITasks to send to the device.
*/

typedef struct SCSITaskInterface
{

	IUNKNOWN_C_GUTS;
	
	/*! Interface version */
	UInt16	version;
	
	/*! Interface revision */
	UInt16	revision;
	
	/*! @function IsTaskActive
    @abstract Method to find out if the task is active or not.
    @discussion Method to find out if the task is active or not. The task is
    considered "active" if the SCSITaskState is not kSCSITaskState_NEW
    nor kSCSITaskState_ENDED.
    @param task Pointer to an instance of an SCSITaskInterface.
    @result Returns 0 if the task is not active, non-zero if it is active.
	*/

	Boolean	( *IsTaskActive ) ( void * task );

	/*! @function SetTaskAttribute
    @abstract Method to set the task's attribute.
    @discussion This method can be used to set the SCSITask's SCSITaskAttribute field.
    Valid values are defined in SCSITask.h
    @param task Pointer to an instance of an SCSITaskInterface.
	@param inAttribute The new attribute value to be stored in the SCSITask.
    @result Returns kIOReturnSuccess or kIOReturnError.
	*/

	IOReturn	( *SetTaskAttribute ) ( void * task, SCSITaskAttribute inAttribute );

	/*! @function GetTaskAttribute
    @abstract Method to get the task's attribute.
    @discussion This method can be used to get the SCSITasks' SCSITaskAttribute field.
    Valid values are defined in SCSITask.h
    @param task Pointer to an instance of an SCSITaskInterface.
	@param outAttribute Pointer to the attribute value stored in the SCSITask.
    @result Returns kIOReturnSuccess or kIOReturnError.
	*/
	
	IOReturn	( *GetTaskAttribute ) ( void * task, SCSITaskAttribute * outAttribute );

	/*! @function SetCommandDescriptorBlock
    @abstract Method to set the task's SCSICommandDescriptorBlock.
    @discussion This method can be used to set the SCSITasks' SCSICommandDescriptorBlock.
    @param task Pointer to an instance of an SCSITaskInterface.
	@param inCDB Pointer to an array of values to be stored in the SCSITask's
	SCSICommandDescriptorBlock.
	@param inSize The size of the array inCDB. Valid values are 6, 10, 12, and 16
	which have enums defined in SCSITask.h.
    @result Returns kIOReturnSuccess or kIOReturnError.
	*/
	
	IOReturn	( *SetCommandDescriptorBlock ) ( void * task, UInt8 * inCDB, UInt8 inSize );

	/*! @function GetCommandDescriptorBlockSize
    @abstract Method to get the task's SCSICommandDescriptorBlock size.
    @discussion This method can be used to get the size of the SCSITask's
    SCSICommandDescriptorBlock.
    @param task Pointer to an instance of an SCSITaskInterface.
    @result UInt8 which is the size of the SCSICommandDescriptorBlock. Valid values
    are 6, 10, 12, and 16 which have enums defined in SCSITask.h 
	*/

	UInt8	( *GetCommandDescriptorBlockSize ) ( void * task );

	/*! @function GetCommandDescriptorBlock
    @abstract Method to get the task's SCSICommandDescriptorBlock.
    @discussion This method can be used to get the SCSITasks' SCSICommandDescriptorBlock.
    @param task Pointer to an instance of an SCSITaskInterface.
	@param outCDB Pointer to an array the size of the SCSICommandDescriptorBlock in
	question. Clients should call GetCommandDescriptorBlockSize first to find out
	how large an array should be passed in.
    @result Returns kIOReturnSucces or kIOReturnError.  
	*/

	IOReturn	( *GetCommandDescriptorBlock ) ( void * task, UInt8 * outCDB );

	/*! @function SetScatterGatherEntries
    @abstract Method to set the task's scatter-gather list entries.
    @discussion This method can be used to set the SCSITask's scatter-gather
    list entries. Scatter-gather lists are represented as an array of SCSITaskSGElements.
    The SCSITaskSGElement structure has two elements, the address of the buffer and the
    length of the buffer.
    @param task Pointer to an instance of an SCSITaskInterface.
	@param inScatterGatherList Pointer to an array of SCSITaskSGElements.
	@param inScatterGatherEntries The size of the inScatterGatherList array.
	@param inTransferCount The TOTAL amount of data to transfer. The length of all the
	entries in the scatter-gather list should at least add up to the amount
	in inTransferCount.
	@param inTransferDirection The transfer direction as defined in
	SCSITask.h. Valid values are kSCSIDataTransfer_NoDataTransfer,
	kSCSIDataTransfer_FromTargetToInitiator, and kSCSIDataTransfer_FromInitiatorToTarget.
    @result Returns kIOReturnSucces or kIOReturnError.  
	*/

	IOReturn	( *SetScatterGatherEntries ) ( void *				task,
											   SCSITaskSGElement *	inScatterGatherList,
											   UInt8				inScatterGatherEntries,
											   UInt64				inTransferCount,
											   UInt8				inTransferDirection );

	/*! @function SetTimeoutDuration
    @abstract Method to set the timeout duration for the SCSITask.
    @discussion This method can be used to set the timeout duration for the SCSITask.
    The timeout duration is counted in milliseconds. A value of zero is equivalent
    to "Wait Forever", but on some buses, this isn't possible, so ULONG_MAX is used.
    @param task Pointer to an instance of an SCSITaskInterface.
	@param inTimeoutDurationMS UInt32 representing the timeout in milliseconds.
    @result Returns kIOReturnSucces or kIOReturnError.  
	*/

	IOReturn	( *SetTimeoutDuration ) ( void * task, UInt32 inTimeoutDurationMS );

	/*! @function GetTimeoutDuration
    @abstract Method to get the timeout duration for the SCSITask.
    @discussion This method can be used to get the timeout duration for the SCSITask.
    The timeout duration is counted in milliseconds.
    @param task Pointer to an instance of an SCSITaskInterface.
    @result Returns a value between zero and ULONG_MAX.  
	*/
	
	UInt32	( *GetTimeoutDuration ) ( void * task );

	/*! @function SetTaskCompletionCallback
    @abstract Method to set the asynchronous completion routine for the SCSITask.
    @discussion This method can be used to set the asynchronous completion routine
    for the SCSITask.
    @param task Pointer to an instance of an SCSITaskInterface.
	@param callback SCSITaskCallbackFunction to be called upon completion of the SCSITask.
	@param refCon A value to be returned to the caller upon completion of the routine.
	This field is not used by the SCSITaskInterface.
    @result Returns kIOReturnSuccess, kIOReturnError, or kIOReturnNotPermitted if the
    client has not called AddCallbackDispatcherToRunLoop on the SCSITaskDeviceInterface.
	*/
	
	IOReturn	( *SetTaskCompletionCallback ) ( void *						task,
												 SCSITaskCallbackFunction	callback,
												 void *						refCon );
	/*! @function ExecuteTaskAsync
    @abstract Method to execute the SCSITask asynchronously.
    @discussion This method can be used to execute the SCSITask asynchronously.
    @param task Pointer to an instance of an SCSITaskInterface.
	@result Returns a valid IOReturn code such as kIOReturnSuccess, kIOReturnError, kIOReturnVMError, kIOReturnCannotWire, etc.
	It will return kIOReturnNotPermitted if the client has not called AddCallbackDispatcherToRunLoop on the SCSITaskDeviceInterface.
    NOTE: IOReturn is defined as kern_return_t and as such, you may get errors back that do not fall under the IOKit subsystem
    error domain (sys_iokit) defined in IOReturn.h.
	*/
	
	IOReturn	( *ExecuteTaskAsync ) ( void * task );

	/*! @function ExecuteTaskSync
    @abstract Method to execute the SCSITask synchronously.
    @discussion This method can be used to execute the SCSITask synchronously.
    @param task Pointer to an instance of an SCSITaskInterface.
	@param senseDataBuffer Pointer to a buffer for REQUEST_SENSE data. May
	be NULL if caller does not wish to have sense data returned. If caller has
	previously called SetAutoSenseDataBuffer(), this parameter is ignored.
	@param outStatus Pointer to an SCSITaskStatus. May
	be NULL if caller does not wish to have task status returned.
	@param realizedTransferCount Pointer to an UInt64 which reflects how much data was
	actually transferred. May be NULL if caller does not wish to know
	how many bytes were transferred.
    @result Returns a valid IOReturn code such as kIOReturnSuccess, kIOReturnError, kIOReturnVMError, kIOReturnCannotWire, etc.
    NOTE: IOReturn is defined as kern_return_t and as such, you may get errors back that do not fall under the IOKit subsystem
    error domain (sys_iokit) defined in IOReturn.h.
	*/

	IOReturn	( *ExecuteTaskSync ) ( void *				task,
									   SCSI_Sense_Data *	senseDataBuffer,
									   SCSITaskStatus *		outStatus,
									   UInt64 *				realizedTransferCount );

	/*! @function AbortTask
    @abstract Method to abort the SCSITask.
    @discussion This method can be used to abort an SCSITask which is already in progress.
    @param task Pointer to an instance of an SCSITaskInterface.
    @result Returns kIOReturnSuccess, kIOReturnUnsupported or kIOReturnError.
	*/

	IOReturn	( *AbortTask ) ( void * task );

	/*! @function GetSCSIServiceResponse
    @abstract Method to get the SCSIServiceResponse from the SCSITask.
    @discussion This method can be used to get the SCSIServiceResponse from the SCSITask.
    @param task Pointer to an instance of an SCSITaskInterface.
	@param outServiceResponse Pointer to an SCSIServiceResponse.
    @result Returns kIOReturnSuccess or kIOReturnError.
	*/

	IOReturn	( *GetSCSIServiceResponse ) ( void * 				task,
											  SCSIServiceResponse * outServiceResponse );

	/*! @function GetTaskState
    @abstract Method to get the SCSITaskState from the SCSITask.
    @discussion This method can be used to get the SCSITaskState from the SCSITask.
    @param task Pointer to an instance of an SCSITaskInterface.
	@param outState Pointer to an SCSITaskState.
    @result Returns kIOReturnSuccess or kIOReturnError.
	*/

	IOReturn	( *GetTaskState ) ( void * task, SCSITaskState * outState );

	/*! @function GetTaskStatus
    @abstract Method to get the SCSITaskStatus from the SCSITask.
    @discussion This method can be used to get the SCSITaskStatus from the SCSITask.
    @param task Pointer to an instance of an SCSITaskInterface.
	@param outStatus Pointer to an SCSITaskStatus.
    @result Returns kIOReturnSuccess or kIOReturnError.
	*/

	IOReturn	( *GetTaskStatus ) ( void * task, SCSITaskStatus * outStatus );

	/*! @function GetRealizedDataTransferCount
    @abstract Method to get the actual transfer count in bytes from the SCSITask.
    @discussion This method can be used to get the actual transfer count in bytes
    from the SCSITask.
    @param task Pointer to an instance of an SCSITaskInterface.
    @result Returns a UInt64 value of bytes transferred.
	*/

	UInt64	( *GetRealizedDataTransferCount ) ( void * task );
	
	/*! @function GetAutoSenseData
    @abstract Method to get the auto-sense data from the SCSITask.
    @discussion This method can be used to get the auto-sense data from the SCSITask.
    @param task Pointer to an instance of an SCSITaskInterface.
	@param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data structure.
	If caller has previously called SetAutoSenseDataBuffer(), this routine will return
	an error.
    @result Returns kIOReturnSuccess if sense data is valid, otherwise kIOReturnError.
	*/

	IOReturn	( *GetAutoSenseData ) ( void * task, SCSI_Sense_Data * senseDataBuffer );
	
	
	/* Added in 10.2 */
	
	
	/*! @function SetAutoSenseDataBuffer
    @abstract Method to set the auto-sense data for the SCSITask.
    @discussion This method can be used to set the auto-sense data buffer for the SCSITask.
    @param task Pointer to an instance of an SCSITaskInterface.
	@param senseDataBuffer Pointer to a buffer. May be be NULL if the caller wants to
	restrict the size to be less than the normal 18 bytes of sense data.
	@param senseDataLength Amount of sense data to retrieve. Zero is not a valid value.
    @result Returns kIOReturnSuccess if sense data buffer was set, otherwise kIOReturnError.
	*/
	
	IOReturn	( *SetAutoSenseDataBuffer ) ( void *			task,
											  SCSI_Sense_Data * senseDataBuffer,
											  UInt8				senseDataLength );
	
	/* Added in 10.6 */
	
	/*! @function ResetForNewTask
    @abstract Method to reset the SCSITask to defaults.
    @discussion This method can be used to reset the SCSITask to defaults.
    @param task Pointer to an instance of an SCSITaskInterface.
    @result Returns kIOReturnSuccess if reset was successful, otherwise kIOReturnError.
	*/
	
	IOReturn	( *ResetForNewTask ) ( void * task );
	
} SCSITaskInterface;


/*! 
	@class SCSITaskDeviceInterface
    @abstract Basic interface for a SCSITask Device.  
    @discussion After rendezvous with a SCSITask Device in the IORegistry you can create
    an instance of this interface as a proxy to the IOService. Once you have this interface,
    or one of its subclasses, you can create SCSITasks to send to the device. Use the
    CreateSCSITask method to create new SCSITask instances for this device.
*/

typedef struct SCSITaskDeviceInterface
{

	IUNKNOWN_C_GUTS;

	/*! Interface version */
	UInt16	version;
	
	/*! Interface revision */
	UInt16	revision;
	
	/*! @function IsExclusiveAccessAvailable
    @abstract Method to find out if the device can be opened exclusively by the caller.
    @discussion Method to find out if the device can be opened exclusively by the caller.
    @param self Pointer to an instance of an SCSITaskDeviceInterface.
    @result Returns false if the device has been opened for exclusive access, otherwise true.
	*/
	
	Boolean ( *IsExclusiveAccessAvailable ) ( void * self );
	
	/*! @function AddCallbackDispatcherToRunLoop
    @abstract Convenience method to add asynchronous callback mechanisms to the CFRunLoop
    of choice.
    @discussion Once a SCSITaskDeviceInterface is opened, the client may make synchronous
    or asynchronous requests to the device. This method creates and initializes a
    mach_port_t for receiving asynchronous callback notifications via the CFRunLoop mechanism.
	@param self Pointer to a SCSITaskDeviceInterface instance.
    @param cfRunLoopRef The CFRunLoop to which asynchronous callback notifications should
    be attached.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, or kIOReturnNoMemory if a mach port could not be
    allocated and initialized properly.
	*/

	IOReturn ( *AddCallbackDispatcherToRunLoop ) ( void * self, CFRunLoopRef cfRunLoopRef );
	
	/*! @function RemoveCallbackDispatcherFromRunLoop
    @abstract Convenience method to remove asynchronous callback mechanisms from the CFRunLoop.
    @discussion Once a SCSITaskDeviceInterface is opened, the client may make synchronous
    or asynchronous requests to the device.
	This method removes the asynchronous notifications delivered via the CFRunLoop. This
	should be called only after calling AddCallbackDispatcherToRunLoop.
	@param self Pointer to a SCSITaskDeviceInterface instance.
	*/
	
	void ( *RemoveCallbackDispatcherFromRunLoop ) ( void * self );
	
	/*! @function ObtainExclusiveAccess
    @abstract Method to obtain exclusive access to the device so that SCSITasks can be sent
    to it.
    @discussion Once a SCSITaskDeviceInterface is opened, the client may request exclusive
    access to the device. Once the client has successfully gained exclusive access, it
    becomes the Logical Unit Driver and all in-kernel Logical Unit Drivers are quiesced.
	@param self Pointer to a SCSITaskDeviceInterface instance.
	@result Returns kIOReturnSuccess if exclusive access was granted, else if media is
	still mounted it returns kIOReturnBusy. If another client already has exclusive
	access, kIOReturnExclusiveAccess is returned.
	*/
	
	IOReturn ( *ObtainExclusiveAccess ) ( void * self );

	/*! @function ReleaseExclusiveAccess
    @abstract Method to release exclusive access to the device so that other clients
    can send commands to it.
    @discussion Once a SCSITaskDeviceInterface is opened, the client may request
    exclusive access to the device.
	Once the client has successfully gained exclusive access, it becomes the Logical
	Unit Driver and all in-kernel Logical Unit Drivers (if any are matched on the device)
	are quiesced. This method releases this access and unquiesces the in-kernel
	drivers (if any).	
	@param self Pointer to a SCSITaskDeviceInterface instance.
	@result Returns kIOReturnSuccess if exclusive access was released, else some
	appropriate error.
	*/
	
	IOReturn ( *ReleaseExclusiveAccess ) ( void * self );
	
	/*! @function CreateSCSITask
    @abstract Method to create SCSITasks.
    @discussion Once a SCSITaskDeviceInterface is opened, the client may request
    exclusive access to the device.
	Once the client has successfully gained exclusive access, it becomes the Logical
	Unit Driver. It then can use this method to allocate SCSITasks to be sent to the device.
	@param self Pointer to a SCSITaskDeviceInterface instance.
	@result Returns a handle to an instance of a SCSITaskInterface or NULL if one
	could not be allocated.
	*/

	SCSITaskInterface ** ( *CreateSCSITask )( void * self );
	
} SCSITaskDeviceInterface;


/*! 
	@class MMCDeviceInterface
    @abstract Basic interface for an MMC-2 Compliant Device.  
    @discussion After rendezvous with a MMC-2 Compliant Device in the IORegistry
    you can create an instance of this interface as a proxy to the IOService. Once
    you have this interface, or one of its subclasses, you can issue some select
    MMC-2 calls to the device without getting exclusive access first.
*/

typedef struct MMCDeviceInterface
{

	IUNKNOWN_C_GUTS;

	/*! Interface version */
	UInt16	version;
	
	/*! Interface revision */
	UInt16	revision;
	
	/*! @function Inquiry
    @abstract Issues an INQUIRY command to the device as defined in SPC-2.
    @discussion Once an MMCDeviceInterface is opened, the client may send this command
    to get inquiry data from the drive.
    @param self Pointer to an MMCDeviceInterface for one IOService.
    @param inquiryBuffer A pointer to a buffer the size of the SCSICmd_INQUIRY_StandardData
    struct found in SCSICmds_INQUIRY_Definitions.h.
    @param inqBufferSize The amount of INQUIRY data to ask the device for (some devices
    return less INQUIRY data than the size of SCSICmd_INQUIRY_StandardData and will need
    to be reset if more than that amount is specified). This value must be less than the
    size of SCSICmd_INQUIRY_StandardData.
    @param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask which
    was executed. Valid SCSITaskStatus values are defined in SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct found
    in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/

	IOReturn ( *Inquiry )(	void *							self,
							SCSICmd_INQUIRY_StandardData *	inquiryBuffer,
							UInt32							inqBufferSize,
							SCSITaskStatus *				taskStatus,
							SCSI_Sense_Data *				senseDataBuffer );
	
	/*! @function TestUnitReady
    @abstract Issues a TEST_UNIT_READY command to the device as defined in SPC-2.
    @discussion Once an MMCDeviceInterface is opened, the client may send this command
    to test if the drive is ready.
    @param self Pointer to an MMCDeviceInterface for one IOService.
    @param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask
    which was executed. Valid SCSITaskStatus values are defined in
    SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct
    found in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/

 	IOReturn ( *TestUnitReady )( 	void *				self,
 									SCSITaskStatus *	taskStatus,
 									SCSI_Sense_Data *	senseDataBuffer  );
	
	
	
	/* This version of GetPerformance is OBSOLETED by Mt. Fuji 5. Please use the newly
	 * introduced API at the end of this struct
	 */
	
	/*! @function GetPerformance
    @abstract Issues a GET_PERFORMANCE command to the device as defined in MMC-2.
    @discussion Once an MMCDeviceInterface is opened, the client may send this command
    to get performance information from the device.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param TOLERANCE The TOLERANCE field as described for the GET_PERFORMANCE command in MMC-2.
	@param WRITE The WRITE bit as described in MMC-2 for the GET_PERFORMANCE command.
	@param EXCEPT The EXCEPT field as described in MMC-2 for the GET_PERFORMANCE command.
	@param STARTING_LBA The STARTING_LBA field as described in MMC-2 for the GET_PERFORMANCE command.
	@param MAXIMUM_NUMBER_OF_DESCRIPTORS The MAXIMUM_NUMBER_OF_DESCRIPTORS field as
	described in MMC-2 for the GET_PERFORMANCE command.
	@param buffer Pointer to the buffer where the mode sense data should be placed.
	@param bufferSize Size of the buffer.
    @param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask
    which was executed. Valid SCSITaskStatus values are defined in
    SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct
    found in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/
	
	IOReturn ( *GetPerformance )(	void * 				self,
									SCSICmdField2Bit	TOLERANCE,
									SCSICmdField1Bit	WRITE,
									SCSICmdField2Bit	EXCEPT,
									SCSICmdField4Byte	STARTING_LBA,
									SCSICmdField2Byte	MAXIMUM_NUMBER_OF_DESCRIPTORS,
									void *				buffer,
									SCSICmdField2Byte	bufferSize,
									SCSITaskStatus * 	taskStatus,
									SCSI_Sense_Data *	senseDataBuffer );
	
	/*! @function GetConfiguration
    @abstract Issues a GET_CONFIGURATION command to the device as defined in MMC-2.
    @discussion Once an MMCDeviceInterface is opened, the client may send this command
    to get configuration information from the device.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param RT The RT field as described for the GET_CONFIGURATION command in MMC-2.
	@param STARTING_FEATURE_NUMBER The STARTING_FEATURE_NUMBER field as described in MMC-2
	for the GET_CONFIGURATION command.
	@param buffer Pointer to the buffer where the mode sense data should be placed.
	@param bufferSize Size of the buffer.
    @param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask
    which was executed. Valid SCSITaskStatus values are defined in
    SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct
    found in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/

	IOReturn ( *GetConfiguration )(	void *				self,
									SCSICmdField1Byte	RT,
									SCSICmdField2Byte	STARTING_FEATURE_NUMBER,
									void *				buffer,
									SCSICmdField2Byte	bufferSize,
									SCSITaskStatus *	taskStatus,
									SCSI_Sense_Data *	senseDataBuffer );

	/*! @function ModeSense10
    @abstract Issues a MODE_SENSE_10 command to the device as defined in SPC-2.
    @discussion Once an MMCDeviceInterface is opened, the client may send this command
    to get mode page information from the device.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param LLBAA The LLBAA bit as defined in SPC-2 for the MODE_SENSE_10 command.
	@param DBD The DBD bit as defined in SPC-2 for the MODE_SENSE_10 command.
	@param PC The PC bits as defined in SPC-2 for the MODE_SENSE_10 command.
	@param PAGE_CODE The PAGE_CODE bits as defined in SPC-2 for the MODE_SENSE_10 command.
	@param buffer Pointer to the buffer where the mode sense data should be placed.
	@param bufferSize Size of the buffer.
    @param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask
    which was executed. Valid SCSITaskStatus values are defined in
    SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct
    found in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/

	IOReturn ( *ModeSense10 )(	void *				self,
								SCSICmdField1Bit	LLBAA,
								SCSICmdField1Bit	DBD,
								SCSICmdField2Bit	PC,
								SCSICmdField6Bit	PAGE_CODE,
								void *				buffer,
								SCSICmdField2Byte	bufferSize,
								SCSITaskStatus * 	taskStatus,
								SCSI_Sense_Data * 	senseDataBuffer );
	
	/*! @function SetWriteParametersModePage
	@abstract Issues a MODE_SELECT command to the device as defined in SPC-2 with the
	Write Parameters Mode Page Code as defined in MMC-2.
	@discussion Once an MMCDeviceInterface is opened, the client may send this command
    to set the default values returned in a READ_DISC_INFORMATION call.
	@param buffer Pointer to buffer (including mode parameter header).
    @param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask
    which was executed. Valid SCSITaskStatus values are defined in
    SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct
    found in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/
	
	IOReturn ( *SetWriteParametersModePage )( 	void * 				self,
												void * 				buffer,
												SCSICmdField1Byte 	bufferSize,
												SCSITaskStatus *	taskStatus,
												SCSI_Sense_Data *	senseDataBuffer );


	/*! @function GetTrayState
    @abstract Issues a GET_EVENT_STATUS_NOTIFICATION command to the device as defined in MMC-2.
    @discussion Once an MMCDeviceInterface is opened, the client may send this command to
    find out if the device's medium tray is open.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param trayState Pointer to a UInt8 which will hold the tray state on completion of
	the routine. The tray state can be one of two values,
	kMMCDeviceTrayClosed or kMMCDeviceTrayOpen.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, or kIOReturnExclusiveAccess if the device is already
	opened for exclusive access by another client.
	*/
	
	IOReturn ( *GetTrayState )( void * self, UInt8 * trayState );
	
	/*! @function SetTrayState
    @abstract Issues a START_STOP_UNIT command to the device as defined in SBC-3.
    @discussion Once an MMCDeviceInterface is opened and all volumes associated
    with that device's media have been unmounted, the client may send this command
    to eject the tray.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param trayState A UInt8 describing which tray state is desired. The tray state
	can be one of two values, kMMCDeviceTrayClosed or kMMCDeviceTrayOpen.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNotPermitted if media is inserted,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/
	
	IOReturn ( *SetTrayState )( void * self, UInt8 trayState );
 	
	/*! @function ReadTableOfContents
    @abstract Issues a READ_TOC_PMA_ATIP command to the device as defined in MMC-2/SFF-8020i.
    @discussion Once an MMCDeviceInterface is opened the client may send this command
    to read the table of contents from the media.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param MSF The MSF bit as defined in MMC-2/SFF-8020i.
    @param FORMAT The FORMAT field as defined in MMC-2/SFF-8020i.
	@param TRACK_SESSION_NUMBER The TRACK_SESSION_NUMBER field as defined in MMC-2/SFF-8020i.
	@param buffer Pointer to the buffer to be used for this function.
	@param bufferSize The size of the data transfer requested.
	@param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask
	which was executed. Valid SCSITaskStatus values are defined in
	SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct
    found in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/

	IOReturn ( *ReadTableOfContents )(	void *				self,
										SCSICmdField1Bit 	MSF,
										SCSICmdField4Bit 	FORMAT,
										SCSICmdField1Byte	TRACK_SESSION_NUMBER,
										void *				buffer,
										SCSICmdField2Byte	bufferSize,
										SCSITaskStatus *	taskStatus,
										SCSI_Sense_Data *	senseDataBuffer );
	
	/*! @function ReadDiscInformation
    @abstract Issues a READ_DISC_INFORMATION command to the device as defined in MMC-2.
    @discussion Once an MMCDeviceInterface is opened the client may send this command
    to read information about the disc (CD-R/RW, (un)finalized, etc..
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param buffer Pointer to the buffer to be used for this function.
	@param bufferSize The size of the data transfer requested.
	@param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask which
	was executed. Valid SCSITaskStatus values are defined in SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct found
    in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection
    to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created, or
    kIOReturnExclusiveAccess if the device is already opened for exclusive access
    by another client.
	*/

	IOReturn ( *ReadDiscInformation ) (	void *				self,
										void *				buffer,
										SCSICmdField2Byte	bufferSize,
										SCSITaskStatus *	taskStatus,
										SCSI_Sense_Data *	senseDataBuffer );
	
	/*! @function ReadTrackInformation
    @abstract Issues a READ_TRACK_INFORMATION command to the device as defined in MMC-2.
    @discussion Once an MMCDeviceInterface is opened the client may send this command to
    read information about selected tracks on the disc.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param ADDRESS_NUMBER_TYPE The ADDRESS/NUMBER_TYPE field as defined in MMC-2.
    @param LOGICAL_BLOCK_ADDRESS_TRACK_SESSION_NUMBER The LOGICAL_BLOCK_ADDRESS/SESSION_NUMBER
    field as defined in MMC-2.
	@param buffer Pointer to the buffer to be used for this function.
	@param bufferSize The size of the data transfer requested.
	@param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask which
	was executed. Valid SCSITaskStatus values are defined in SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct found
    in SCSICmds_REQUEST_SENSE_Defs.h. The sense data is only valid
    if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/

	IOReturn ( *ReadTrackInformation ) (	void *				self,
											SCSICmdField2Bit	ADDRESS_NUMBER_TYPE,
											SCSICmdField4Byte	LOGICAL_BLOCK_ADDRESS_TRACK_SESSION_NUMBER,
											void *				buffer,
											SCSICmdField2Byte	bufferSize,
											SCSITaskStatus *	taskStatus,
											SCSI_Sense_Data *	senseDataBuffer );
	
	/*! @function ReadDVDStructure
    @abstract Issues a READ_DVD_STRUCTURE command to the device as defined in MMC-2.
    @discussion Once an MMCDeviceInterface is opened the client may send this command to
    read information about DVD specific structures on the disc.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param ADDRESS The ADDRESS field as defined in MMC-2.
    @param LAYER_NUMBER The LAYER_NUMBER field as defined in MMC-2.
    @param FORMAT The FORMAT field as defined in MMC-2.
	@param buffer Pointer to the buffer to be used for this function.
	@param bufferSize The size of the data transfer requested.
	@param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask
	which was executed. Valid SCSITaskStatus values are defined in
	SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct
    found in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/

	IOReturn ( *ReadDVDStructure ) (	void *				self,
										SCSICmdField4Byte	ADDRESS,
										SCSICmdField1Byte	LAYER_NUMBER,
										SCSICmdField1Byte	FORMAT,
										void *				buffer,
										SCSICmdField2Byte	bufferSize,
										SCSITaskStatus *	taskStatus,
										SCSI_Sense_Data *	senseDataBuffer );
	
	/*! @function GetSCSITaskDeviceInterface
    @abstract Gets a handle to the SCSITaskDeviceInterface without closing the user
    client connection which was initiated by IOCreateCFPlugInForService.
    @discussion Once an MMCDeviceInterface is opened the client may use this function
    to get a handle to the interface used to create and send SCSITasks directly to the device.
    @param self Pointer to an MMCDeviceInterface for one IOService.
    @result Returns a handle to a SCSITaskDeviceInterface if successful, otherwise NULL.
	*/
	
	SCSITaskDeviceInterface ** 	( *GetSCSITaskDeviceInterface )( void * self );
	
	/* Added in Mac OS X 10.2 */
	
	/*! @function GetPerformanceV2
    @abstract Issues a GET_PERFORMANCE command to the device as defined in Mt. Fuji 5.
    @discussion Once an MMCDeviceInterface is opened, the client may send this command
    to get performance information from the device.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param DATA_TYPE The DATA_TYPE field as described for the GET_PERFORMANCE command in Mt. Fuji 5.
	@param STARTING_LBA The STARTING_LBA field as described in Mt. Fuji 5 for the GET_PERFORMANCE command.
	@param MAXIMUM_NUMBER_OF_DESCRIPTORS The MAXIMUM_NUMBER_OF_DESCRIPTORS field as
	described in Mt. Fuji 5 for the GET_PERFORMANCE command.
	@param TYPE The TYPE field as described for the GET_PERFORMANCE command in Mt. Fuji 5.
	@param buffer Pointer to the buffer where the mode sense data should be placed.
	@param bufferSize Size of the buffer.
    @param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask
    which was executed. Valid SCSITaskStatus values are defined in
    SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct
    found in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/
	
	IOReturn ( *GetPerformanceV2 )( void * 				self,
									SCSICmdField5Bit 	DATA_TYPE,
									SCSICmdField4Byte	STARTING_LBA,
									SCSICmdField2Byte	MAXIMUM_NUMBER_OF_DESCRIPTORS,
									SCSICmdField1Byte	TYPE,
									void *				buffer,
									SCSICmdField2Byte	bufferSize,
									SCSITaskStatus *	taskStatus,
									SCSI_Sense_Data *	senseDataBuffer );
	
	/* Added in Mac OS X 10.3 */
	
	/*! @function SetCDSpeed
    @abstract Issues a SET_CD_SPEED command to the device as defined in MMC-2.
    @discussion Once an MMCDeviceInterface is opened the client may send this command to
				change the read and/or write CD speed of the drive.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param LOGICAL_UNIT_READ_SPEED The LOGICAL_UNIT_READ_SPEED field as defined in MMC-2.
    @param LOGICAL_UNIT_WRITE_SPEED The LOGICAL_UNIT_WRITE_SPEED field as defined in MMC-2.
	@param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask
		   which was executed. Valid SCSITaskStatus values are defined in SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct
		   found in SCSICmds_REQUEST_SENSE_Defs.h.  The sense data is only valid if the
		   SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
			connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
			or kIOReturnExclusiveAccess if the device is already opened for exclusive access
			by another client.
	*/

	IOReturn ( *SetCDSpeed ) (	void *				self,
								SCSICmdField2Byte	LOGICAL_UNIT_READ_SPEED,
								SCSICmdField2Byte	LOGICAL_UNIT_WRITE_SPEED,
								SCSITaskStatus *	taskStatus,
								SCSI_Sense_Data *	senseDataBuffer );
	
	
	/* Added in Mac OS X 10.3 */
	
	/*! @function ReadFormatCapacities
    @abstract Issues a READ_FORMAT_CAPACITIES command to the device as defined in MMC-2.
    @discussion Once an MMCDeviceInterface is opened the client may send this command to
				get format capacity information from the media.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param buffer Pointer to the buffer where the mode sense data should be placed.
	@param bufferSize Size of the buffer.
	@param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask
		   which was executed. Valid SCSITaskStatus values are defined in SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct
		   found in SCSICmds_REQUEST_SENSE_Defs.h.  The sense data is only valid if the
		   SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
			connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
			or kIOReturnExclusiveAccess if the device is already opened for exclusive access
			by another client.
	*/
	
	IOReturn ( *ReadFormatCapacities ) ( void *					self,
										 void *					buffer,
										 SCSICmdField2Byte		bufferSize,
										 SCSITaskStatus *		taskStatus,
										 SCSI_Sense_Data *		senseDataBuffer );
	
	/*! @function ReadDiscStructure
    @abstract Issues a READ_DISC_STRUCTURE command to the device as defined in MMC-5.
    @discussion Once an MMCDeviceInterface is opened the client may send this command to
    read information about Disc specific structures on the disc.
    @param self Pointer to an MMCDeviceInterface for one IOService.
	@param MEDIA_TYPE The MEDIA_TYPE field as defined in MMC-5.
	@param ADDRESS The ADDRESS field as defined in MMC-5.
    @param LAYER_NUMBER The LAYER_NUMBER field as defined in MMC-5.
    @param FORMAT The FORMAT field as defined in MMC-5.
	@param buffer Pointer to the buffer to be used for this function.
	@param bufferSize The size of the data transfer requested.
	@param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask
	which was executed. Valid SCSITaskStatus values are defined in
	SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct
    found in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/

	IOReturn ( *ReadDiscStructure ) (	void *				self,
										SCSICmdField4Bit	MEDIA_TYPE,
										SCSICmdField4Byte	ADDRESS,
										SCSICmdField1Byte	LAYER_NUMBER,
										SCSICmdField1Byte	FORMAT,
										void *				buffer,
										SCSICmdField2Byte	bufferSize,
										SCSITaskStatus *	taskStatus,
										SCSI_Sense_Data *	senseDataBuffer );
	
	/*! @function ReadDiscInformationV2
    @abstract Issues a READ_DISC_INFORMATION command to the device as defined in MMC-5.
    @discussion Once an MMCDeviceInterface is opened the client may send this command
    to read information about the disc (CD-R/RW, (un)finalized, etc..
    @param self Pointer to an MMCDeviceInterface for one IOService.
    @param DATA_TYPE The DATA_TYPE field as defined in MMC-5.
	@param buffer Pointer to the buffer to be used for this function.
	@param bufferSize The size of the data transfer requested.
	@param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask which
	was executed. Valid SCSITaskStatus values are defined in SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct found
    in SCSICmds_REQUEST_SENSE_Defs.h.
	The sense data is only valid if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection
    to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created, or
    kIOReturnExclusiveAccess if the device is already opened for exclusive access
    by another client.
	*/

	IOReturn ( *ReadDiscInformationV2 ) (	void *				self,
											SCSICmdField3Bit	DATA_TYPE,
											void *				buffer,
											SCSICmdField2Byte	bufferSize,
											SCSITaskStatus *	taskStatus,
											SCSI_Sense_Data *	senseDataBuffer );
	
	/*! @function ReadTrackInformationV2
    @abstract Issues a READ_TRACK_INFORMATION command to the device as defined in Mt. Fuji 5.
    @discussion Once an MMCDeviceInterface is opened the client may send this command to
    read information about selected tracks on the disc.
    @param self Pointer to an MMCDeviceInterface for one IOService.
    @param OPEN The OPEN field as defined in Mt. Fuji 5.
	@param ADDRESS_NUMBER_TYPE The ADDRESS/NUMBER_TYPE field as defined in Mt. Fuji 5.
    @param LOGICAL_BLOCK_ADDRESS_TRACK_SESSION_NUMBER The LOGICAL_BLOCK_ADDRESS/SESSION_NUMBER
    field as defined in Mt. Fuji 5.
	@param buffer Pointer to the buffer to be used for this function.
	@param bufferSize The size of the data transfer requested.
	@param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask which
	was executed. Valid SCSITaskStatus values are defined in SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct found
    in SCSICmds_REQUEST_SENSE_Defs.h. The sense data is only valid
    if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/

	IOReturn ( *ReadTrackInformationV2 ) (	void *				self,
											SCSICmdField1Bit	OPEN,
											SCSICmdField2Bit	ADDRESS_NUMBER_TYPE,
											SCSICmdField4Byte	LOGICAL_BLOCK_ADDRESS_TRACK_SESSION_NUMBER,
											void *				buffer,
											SCSICmdField2Byte	bufferSize,
											SCSITaskStatus *	taskStatus,
											SCSI_Sense_Data *	senseDataBuffer );
	
	/* Added in 10.6 */
	
	/*! @function SetStreaming
    @abstract Issues a SET_STREAMING command to the device as defined in MMC-5.
    @discussion Once an MMCDeviceInterface is opened the client may send this command to
    change streaming attributes. Clients should check for the Real-time Streaming Feature
    (107h) before using this command.
    @param self Pointer to an MMCDeviceInterface for one IOService.
    @param TYPE The TYPE field as defined in MMC-5.
	@param buffer Pointer to the buffer to be used for this function.
	@param bufferSize The size of the data transfer requested.
	@param taskStatus Pointer to a SCSITaskStatus to get the status of the SCSITask which
	was executed. Valid SCSITaskStatus values are defined in SCSITask.h
    @param senseDataBuffer Pointer to a buffer the size of the SCSI_Sense_Data struct found
    in SCSICmds_REQUEST_SENSE_Defs.h. The sense data is only valid
    if the SCSITaskStatus is kSCSITaskStatus_CHECK_CONDITION.
    @result Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no
    connection to an IOService, kIOReturnNoMemory if a SCSITask couldn't be created,
	or kIOReturnExclusiveAccess if the device is already opened for exclusive access
	by another client.
	*/
	
	IOReturn ( *SetStreaming ) (	void *				self,
									SCSICmdField1Byte	TYPE,
									void *				buffer,
									SCSICmdField2Byte	bufferSize,
									SCSITaskStatus *	taskStatus,
									SCSI_Sense_Data *	senseDataBuffer );
	
} MMCDeviceInterface;
	
#endif

#if !KERNEL

#ifdef __cplusplus
}
#endif

#endif /* !KERNEL */

#endif /* __SCSI_TASK_LIB_H__ */
