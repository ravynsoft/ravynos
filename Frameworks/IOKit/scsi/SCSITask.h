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


#ifndef _IOKIT_SCSI_TASK_H_
#define _IOKIT_SCSI_TASK_H_


#if KERNEL
#include <IOKit/IOTypes.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#endif


/*! @header SCSITask
SCSITask typedefs and constants used inside the kernel and user space.

Note that the SCSITaskIdentifier is an opaque object and that directly
casting the SCSITaskIdentifier to any other type is discouraged. The SCSITask
implementation changes when necessary to accomodate architectural changes,
performance improvements, and bug fixes.

Device and protocol layer drivers that need to access information
contained in a SCSITask should use the appropriate accessor methods in
IOSCSIPrimaryCommandsDevice.h or IOSCSIProtocolServices.h
*/


/*! @typedef SCSIDeviceIdentifier
    @abstract 64-bit number to represent a SCSI Device.
	@discussion If the identifier can either be that of an initiator or a target,
    SCSIDeviceIdentifier should be used.
*/

typedef UInt64 					SCSIDeviceIdentifier;

/*! @typedef SCSITargetIdentifier
    @abstract 64-bit number to represent a SCSI Target Device.
	@discussion If the identifier is for a target only and not an initiator, then
	SCSITargetIdentifier should be used.
*/

typedef SCSIDeviceIdentifier 	SCSITargetIdentifier;

/*! @typedef SCSIInitiatorIdentifier
    @abstract 64-bit number to represent a SCSI Initiator Device.
	@discussion If the identifier is for an initiator only and not a target, then
	SCSIInitiatorIdentifier should be used.
*/

typedef SCSIDeviceIdentifier 	SCSIInitiatorIdentifier;

/*! @typedef SCSILogicalUnitBytes[8]
    @abstract 8-Byte array to represent LUN information
    @discussion The SCSI Primary Commands specification treats the 64-bits
	of LUN information as 4 2-byte structures.
	
	Use of the 64-bit SCSILogicalUnitNumber is now deprecated. Since it
	was not defined on Mac OS X how the 64-bits were encoded for hierarchical
	units and all usage was simply as a 64-bit number, changing the encoding
	scheme now would result in non-binary compatible code. New APIs have been
	added to retrieve the LUN bytes from the SCSITask and set them in the SCSITask.
 */
typedef UInt8					SCSILogicalUnitBytes[8];
typedef UInt64					SCSILogicalUnitNumber;          // DEPRECATED


/*! @typedef SCSITaggedTaskIdentifier
    @abstract 64-bit number to represent a unique task identifier.
	@discussion The Tagged Task Identifier is used when a Task has a Task Attribute other
	than SIMPLE. The SCSI Application Layer client that controls the Logical
	Unit for which a Task is intended is required to guarantee that the Task
	Tag Identifier is unique. Zero cannot be used a a Tag value as this is used
	to when a Tagged Task Identifier value is needed for a Task with a SIMPLE 
	attribute.
*/

typedef UInt64 SCSITaggedTaskIdentifier;

/*!
	@enum Untagged Task Identifier
	@discussion The Untagged Task Identifier is used to indicate no unique tag
	is associated with the Task.
	@constant kSCSIUntaggedTaskIdentifier This value means the task is untagged.
 */

enum
{
	kSCSIUntaggedTaskIdentifier = 0
};

/*!
	@typedef SCSITaskAttribute
	@abstract Attributes for task delivery.
	@discussion The Task Attribute defines how this task should be managed
	when determing order for queueing and submission to the 
	appropriate device server. The Task Attribute is set by the SCSI
	Application Layer and cannot be modified by the SCSI Protocol Layer.
	@constant kSCSITask_SIMPLE The task has a simple attribute.
	@constant kSCSITask_ORDERED The task has an ordered attribute.
	@constant kSCSITask_HEAD_OF_QUEUE The task has a head-of-queue attribute.
	@constant kSCSITask_ACA The task has an auto-contingent-allegiance attribute.
 */

typedef enum SCSITaskAttribute
{
	kSCSITask_SIMPLE			= 0,
	kSCSITask_ORDERED			= 1,
	kSCSITask_HEAD_OF_QUEUE		= 2,
	kSCSITask_ACA				= 3
} SCSITaskAttribute;

/*!
	@typedef SCSITaskState
	@abstract Attributes for task state.
	@discussion The Task State represents the current state of the task.
	The state is set to NEW_TASK when the task is created.  The SCSI Protocol
	Layer will then adjust the state as the task is queued and during
	execution. The SCSI Application Layer can examine the state to monitor
	the progress of a task. The Task State can only be modified by the SCSI
	Protocol Layer.  The SCSI Application Layer can only read the state.
	@constant kSCSITaskState_NEW_TASK The task state is new task.
	@constant kSCSITaskState_ENABLED The task is enabled and queued.
	@constant kSCSITaskState_BLOCKED The task is blocked.
	@constant kSCSITaskState_DORMANT The task is dormant.
	@constant kSCSITaskState_ENDED The task is complete.
 */

typedef enum SCSITaskState
{
	kSCSITaskState_NEW_TASK		= 0,
	kSCSITaskState_ENABLED		= 1,
	kSCSITaskState_BLOCKED		= 2,
	kSCSITaskState_DORMANT		= 3,
	kSCSITaskState_ENDED		= 4
} SCSITaskState;


/*!
	@typedef SCSIServiceResponse
	@abstract Attributes for task service response.
	@discussion The Service Response represents the execution status of
	a service request made to a Protocol Services Driver. The Service
	Response can only be modified by the SCSI Protocol Layer. The SCSI
	Application Layer can only read the state.
 */

typedef enum SCSIServiceResponse
{
	/*!
	@constant kSCSIServiceResponse_Request_In_Process
	Not defined in SAM specification, but is a service response used
	for asynchronous commands that are not yet completed.
	*/
	kSCSIServiceResponse_Request_In_Process					= 0,

	/*!
	@constant kSCSIServiceResponse_SERVICE_DELIVERY_OR_TARGET_FAILURE
	The service request failed because of a delivery or target failure.
	*/
	kSCSIServiceResponse_SERVICE_DELIVERY_OR_TARGET_FAILURE	= 1,

	/*!
	@constant kSCSIServiceResponse_TASK_COMPLETE
	The task completed.
	*/
	kSCSIServiceResponse_TASK_COMPLETE 						= 2,
	
	/*!
	@constant kSCSIServiceResponse_LINK_COMMAND_COMPLETE
	The linked command completed.
	*/
	kSCSIServiceResponse_LINK_COMMAND_COMPLETE				= 3,
	
	/*!
	@constant kSCSIServiceResponse_FUNCTION_COMPLETE
	The task management function completed.
	*/
	kSCSIServiceResponse_FUNCTION_COMPLETE					= 4,
	
	/*!
	@constant kSCSIServiceResponse_FUNCTION_REJECTED
	The task management function was rejected.
	*/
	kSCSIServiceResponse_FUNCTION_REJECTED					= 5
} SCSIServiceResponse;

/*!
	@typedef SCSITaskStatus
	@abstract Attributes for task status.
	@discussion The Task Status represents the completion status
	of the task which provides the  SCSI Application Layer with
	additional information about how to procede in handling a
	completed task.
	
	The SCSI Architecture Model specification only defines task
	status values for when a task completes with a service response
	of either TASK_COMPLETED or LINK_COMMAND_COMPLETE.
	
	Since additional information will aid in error recovery when
	a task fails to be completed by a device due to a service
	response of kSCSIServiceResponse_SERVICE_DELIVERY_OR_TARGET_FAILURE,
	additional values have been defined that can be returned by the
	SCSI Protocol Layer to inform the SCSI Application Layer of the
	cause of the delivery failure.
	
	The Task Status can only be modified by the SCSI Protocol Layer.
	The SCSI Application Layer can only read the status
*/
typedef enum SCSITaskStatus
{
	
	/*!
	@constant kSCSITaskStatus_GOOD
	The task completed with a status of GOOD.
	*/
	
	kSCSITaskStatus_GOOD						= 0x00,

	/*!
	@constant kSCSITaskStatus_CHECK_CONDITION
	The task completed with a status of CHECK_CONDITION. Additional
	information about the condition should be available in the sense data.
	*/
	
	kSCSITaskStatus_CHECK_CONDITION				= 0x02,
	
	/*!
	@constant kSCSITaskStatus_CONDITION_MET
	The task completed with a status of CONDITION_MET.
	*/
	
	kSCSITaskStatus_CONDITION_MET				= 0x04,
	
	/*!
	@constant kSCSITaskStatus_BUSY
	The task completed with a status of BUSY. The device server might need
	time to process a request and a delay may be required.
	*/
	kSCSITaskStatus_BUSY						= 0x08,

	/*!
	@constant kSCSITaskStatus_INTERMEDIATE
	The task completed with a status of INTERMEDIATE.
	*/
	kSCSITaskStatus_INTERMEDIATE				= 0x10,

	/*!
	@constant kSCSITaskStatus_INTERMEDIATE_CONDITION_MET
	The task completed with a status of INTERMEDIATE_CONDITION_MET.
	*/
	kSCSITaskStatus_INTERMEDIATE_CONDITION_MET	= 0x14,

	/*!
	@constant kSCSITaskStatus_RESERVATION_CONFLICT
	The task completed with a status of RESERVATION_CONFLICT.
	*/
	kSCSITaskStatus_RESERVATION_CONFLICT		= 0x18,

	/*!
	@constant kSCSITaskStatus_TASK_SET_FULL
	The task completed with a status of TASK_SET_FULL. The device server
	may need to complete a task before the initiator sends another.
	*/
	kSCSITaskStatus_TASK_SET_FULL				= 0x28,
	
	/*!
	@constant kSCSITaskStatus_ACA_ACTIVE
	The task completed with a status of ACA_ACTIVE. The device server may
	need the initiator to clear the Auto-Contingent Allegiance condition
	before it will respond to new commands.
	*/
	kSCSITaskStatus_ACA_ACTIVE					= 0x30,
	
	/*!
	@constant kSCSITaskStatus_TaskTimeoutOccurred
	If a task is aborted by the SCSI Protocol Layer due to it exceeding
	the timeout value specified by the task, the task status shall be
	set to kSCSITaskStatus_TaskTimeoutOccurred.
	*/
	
	kSCSITaskStatus_TaskTimeoutOccurred			= 0x01,
	
	/*!
	@constant kSCSITaskStatus_ProtocolTimeoutOccurred
	If a task is aborted by the SCSI Protocol Layer due to it exceeding a
	timeout value specified by the support for the protocol or a related
	specification, the task status shall be set to
	kSCSITaskStatus_ProtocolTimeoutOccurred.
	*/
	
	kSCSITaskStatus_ProtocolTimeoutOccurred		= 0x02,
	
	/*!
	@constant kSCSITaskStatus_DeviceNotResponding
	If a task is unable to be delivered due to a failure of the device not
	accepting the task or the device acknowledging the attempt to send it the
	device the task status shall be set to kSCSITaskStatus_DeviceNotResponding.
	This will allow the SCSI Application driver to perform the necessary steps
	to try to recover the device. This shall only be reported after the SCSI
	Protocol Layer driver has attempted all protocol specific attempts to recover
	the device.
	*/
	
	kSCSITaskStatus_DeviceNotResponding			= 0x03,
	
	/*!
	@constant kSCSITaskStatus_DeviceNotPresent
	If the task is unable to be delivered because the device has been
	detached, the task status shall be set to kSCSITaskStatus_DeviceNotPresent.
	This will allow the SCSI Application Layer to halt the sending of tasks
	to the device and, if supported, perform any device failover or system
	cleanup.
	*/
	kSCSITaskStatus_DeviceNotPresent			= 0x04,
	
	/*!
	@constant kSCSITaskStatus_DeliveryFailure
	If the task is unable to be
	delivered to the device due to a failure in the SCSI Protocol Layer,
	such as a bus reset or communications error, but the device is is
	known to be functioning properly, the task status shall be set to 
	kSCSITaskStatus_DeliveryFailure. This can also be reported if the
	task could not be delivered due to a protocol error that has since
	been corrected.
	 */
	kSCSITaskStatus_DeliveryFailure				= 0x05,
	
	/*!
	@constant kSCSITaskStatus_No_Status
	This status is not defined by
	the SCSI specifications, but is here to provide a status that can
	be returned in cases where there is not status available from the
	device or protocol, for example, when the service response is
	neither TASK_COMPLETED nor LINK_COMMAND_COMPLETE or when the
	service response is SERVICE_DELIVERY_OR_TARGET_FAILURE and the
	reason for failure could not be determined.
	*/
	kSCSITaskStatus_No_Status					= 0xFF
} SCSITaskStatus;

/*!
	@enum Command Descriptor Block Size
	@discussion Command Descriptor Block Size constants.
*/
enum
{
	/*!
	@constant kSCSICDBSize_Maximum This is the largest size a Command Descriptor
	Block can be as specified in SPC-2.
	*/
	kSCSICDBSize_Maximum 	= 16,

	/*!
	@constant kSCSICDBSize_6Byte Use this for a 6-byte CDB.
	*/
	kSCSICDBSize_6Byte 		= 6,

	/*!
	@constant kSCSICDBSize_10Byte Use this for a 10-byte CDB.
	*/
	kSCSICDBSize_10Byte 	= 10,

	/*!
	@constant kSCSICDBSize_12Byte Use this for a 12-byte CDB.
	*/
	kSCSICDBSize_12Byte 	= 12,

	/*!
	@constant kSCSICDBSize_16Byte Use this for a 16-byte CDB.
	*/
	kSCSICDBSize_16Byte 	= 16
};

typedef UInt8 SCSICommandDescriptorBlock[kSCSICDBSize_Maximum];

/*!
	@enum Data Transfer Direction
	@discussion DataTransferDirection constants.
*/
enum
{
	/*!
	@constant kSCSIDataTransfer_NoDataTransfer Use this for tasks
	that transfer no data.
	*/
	kSCSIDataTransfer_NoDataTransfer		= 0x00,

	/*!
	@constant kSCSIDataTransfer_FromInitiatorToTarget Use this for tasks that transfer
	data from the initiator to the target.
	*/
	kSCSIDataTransfer_FromInitiatorToTarget	= 0x01,

	/*!
	@constant kSCSIDataTransfer_FromTargetToInitiator Use this for tasks that transfer
	data from the target to the initiator.
	*/
	kSCSIDataTransfer_FromTargetToInitiator	= 0x02
};


#if defined(KERNEL) && defined(__cplusplus)

/* Libkern includes */
#include <libkern/c++/OSObject.h>


/*!
	@enum SCSITaskMode
	@discussion The SCSI Task mode is used by the SCSI
	Protocol Layer to indicate what mode the task is executing.
*/
typedef enum SCSITaskMode
{
	kSCSITaskMode_CommandExecution	= 1,
	kSCSITaskMode_Autosense			= 2
} SCSITaskMode;

/*!
	@typedef SCSITaskIdentifier
	@discussion This is an opaque object that represents a task.
	This is used so that drivers for both the SCSI Protocol Layer
	and the SCSI  Application Layer cannot modify the SCSITask object
	directly but must instead use the inherited methods to do so. This
	allows the implementation of SCSITask to change without directly
	impacting device and protocol layer drivers. In addition, it
	prevents changing of properties that are not allowed to be
	changed by a given layer.
*/
typedef OSObject *	SCSITaskIdentifier;


/*!
	@typedef SCSITaskCompletion
	@discussion This is the typedef for completion routines that
	work with SCSITaskIdentifiers.
*/
typedef void ( *SCSITaskCompletion )( SCSITaskIdentifier completedTask );


#endif	/* defined(KERNEL) && defined(__cplusplus) */

#endif /* _IOKIT_SCSI_TASK_H_ */
