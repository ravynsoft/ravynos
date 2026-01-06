/*
 * Copyright (c) 1998-2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  IOFireWireLibIsoch.h
 *  IOFireWireFamily
 *
 *  Created on Mon Mar 19 2001.
 *  Copyright (c) 2001-2002 Apple Computer, Inc. All rights reserved.
 *
 */

#ifndef __IOFireWireLibIsoch_H__
#define __IOFireWireLibIsoch_H__

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>

#include <IOKit/firewire/IOFireWireFamilyCommon.h>
#include <IOKit/firewire/IOFireWireLib.h>

//
// local isoch port
//

//	uuid string: 541971C6-CE72-11D7-809D-000393C0B9D8
#define kIOFireWireLocalIsochPortInterfaceID_v5 CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault \
											, 0x54, 0x19, 0x71, 0xC6, 0xCE, 0x72, 0x11, 0xD7\
											, 0x80, 0x9D, 0x00, 0x03, 0x93, 0xC0, 0xB9, 0xD8 )

//	uuid string: FECAA2F6-4E84-11D7-B6FD-0003938BEB0A
#define kIOFireWireLocalIsochPortInterfaceID_v4 CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault \
											,0xFE, 0xCA, 0xA2, 0xF6, 0x4E, 0x84, 0x11, 0xD7\
											,0xB6, 0xFD, 0x00, 0x03, 0x93, 0x8B, 0xEB, 0x0A )

//  uuid string: A0AD095E-6D2F-11D6-AC82-0003933F84F0
#define kIOFireWireLocalIsochPortInterfaceID_v3 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0xA0, 0xAD, 0x09, 0x5E, 0x6D, 0x2F, 0x11, 0xD6,\
											0xAC, 0x82, 0x00, 0x03, 0x93, 0x3F, 0x84, 0xF0 )

//	Availability: Mac OS X "Jaguar" and later
//  uuid string: 73C76D09-6D2F-11D6-AF7F-0003933F84F0
#define kIOFireWireLocalIsochPortInterfaceID_v2 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x73, 0xC7, 0x6D, 0x09, 0x6D, 0x2F, 0x11, 0xD6,\
											0xAF, 0x7F, 0x00, 0x03, 0x93, 0x3F, 0x84, 0xF0 )

//  uuid string: 0F5E33C8-1350-11D5-9BE7-003065AF75CC
#define kIOFireWireLocalIsochPortInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x0F, 0x5E, 0x33, 0xC8, 0x13, 0x50, 0x11, 0xD5,\
											0x9B, 0xE7, 0x00, 0x30, 0x65, 0xAF, 0x75, 0xCC)

//
// remote isoch port
//

//	uuid string: AAFDBDB0-489F-11D5-BC9B-003065423456
#define kIOFireWireRemoteIsochPortInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0xAA, 0xFD, 0xBD, 0xB0, 0x48, 0x9F, 0x11, 0xD5,\
											0xBC, 0x9B, 0x00, 0x30, 0x65, 0x42, 0x34, 0x56)


//
// isoch channel
//

//  uuid string: 2EC1E404-1350-11D5-89B5-003065AF75CC
#define kIOFireWireIsochChannelInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x2E, 0xC1, 0xE4, 0x04, 0x13, 0x50, 0x11, 0xD5,\
											0x89, 0xB5, 0x00, 0x30, 0x65, 0xAF, 0x75, 0xCC)

//
// DCL command pool
//

//  uuid string: 4A4B1710-1350-11D5-9B12-003065AF75CC
#define kIOFireWireDCLCommandPoolInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x4A, 0x4B, 0x17, 0x10, 0x13, 0x50, 0x11, 0xD5,\
											0x9B, 0x12, 0x00, 0x30, 0x65, 0xAF, 0x75, 0xCC)

//
// NuDCL pool
//

//	uuid string: D3837670-4463-11D7-B79A-0003938BEB0A
#define kIOFireWireNuDCLPoolInterfaceID CFUUIDGetConstantUUIDWithBytes( kCFAllocatorDefault,\
											0xD3, 0x83, 0x76, 0x70, 0x44, 0x63, 0x11, 0xD7,\
											0xB7, 0x9A, 0x00, 0x03, 0x93, 0x8B, 0xEB, 0x0A)

//  uuid string: 6D1FDE59-50CE-4ED4-880A-9D13A4624038
#define kIOFireWireAsyncStreamListenerInterfaceID  CFUUIDGetConstantUUIDWithBytes(kCFAllocatorDefault,\
											0x6D, 0x1F, 0xDE, 0x59, 0x50, 0xCE, 0x4E, 0xD4,\
											0x88, 0x0A, 0x90, 0x13, 0xA4, 0x62, 0x40, 0x38)


typedef void	(*IOFireWireIsochChannelForceStopHandler)(
	IOFireWireLibIsochChannelRef	interface, 
	UInt32							stopCondition);

typedef IOReturn	(*IOFireWireLibIsochPortCallback)(
	IOFireWireLibIsochPortRef		interface) ;
	
typedef IOReturn	(*IOFireWireLibIsochPortAllocateCallback)(
	IOFireWireLibIsochPortRef		interface,
	IOFWSpeed						maxSpeed,
	UInt32							channel) ;

typedef IOReturn	(*IOFireWireLibIsochPortGetSupportedCallback)(
	IOFireWireLibIsochPortRef		interface,
	IOFWSpeed*						outMaxSpeed,
	UInt64*							outChanSupported) ;

typedef IOReturn	(*IOFireWireLibIsochPortFinalizeCallback)( void* refcon ) ;

// ============================================================
//
// IOFireWireIsochPort
//
// ============================================================

/*! @parseOnly */
#define IOFIREWIRELIBISOCHPORT_C_GUTS	\
	/*!	@function GetSupported \
		@abstract The method is called to determine which FireWire isochronous \
			channels and speed this port supports. \
		@discussion This method is called by the channel object to which a port \
			has been added. Subclasses of IOFireWireIsochPortInterface override \
			this method to support specific hardware. Do not call this method \
			directly. \
		@param self The isoch port interface to use. \
		@param maxSpeed A pointer to an IOFWSpeed which should be filled with \
			the maximum speed this port can talk or listen. \
		@param chanSupported A pointer to a UInt64 which should be filled with \
			a bitmask representing the FireWire bus isochonous channels on \
			which the port can talk or listen. Set '1' for supported, '0' for \
			unsupported. \
		@result Return kIOReturnSuccess on success, other return any other \
			IOReturn error code.*/ \
	IOReturn	(*GetSupported)	( IOFireWireLibIsochPortRef self, IOFWSpeed* maxSpeed, UInt64* chanSupported ) ;	\
	/*!	@function AllocatePort \
		@abstract The method is called when the port should configure its \
			associated hardware to prepare to send or receive isochronous data \
			on the channel number and at the speed specified. \
		@discussion This method is called by the channel object to which a port \
			has been added. Subclasses of IOFireWireIsochPortInterface override \
			this method to support specific hardware. Do not call this method \
			directly. \
		@param self The isoch port interface to use. \
		@param speed Channel speed \
		@param chan Channel number (0-63) \
		@result Return kIOReturnSuccess on success, other return any other \
			IOReturn error code.*/ \
	IOReturn	(*AllocatePort)	( IOFireWireLibIsochPortRef self, IOFWSpeed speed, UInt32 chan ) ;	\
	/*!	@function ReleasePort \
		@abstract The method is called to release the hardware after the \
			channel has been stopped. \
		@discussion This method is called by the channel object to which a port \
			has been added. Subclasses of IOFireWireIsochPortInterface override \
			this method to support specific hardware. Do not call this method \
			directly. \
		@param self The isoch port interface to use. \
		@result Return kIOReturnSuccess on success, other return any other IOReturn error code.*/ \
	IOReturn 	(*ReleasePort)	( IOFireWireLibIsochPortRef self ) ;	\
	/*!	@function Start \
		@abstract The method is called when the port is to begin talking or listening. \
		@discussion This method is called by the channel object to which a port \
			has been added. Subclasses of IOFireWireIsochPortInterface override \
			this method to support specific hardware. Do not call this method \
			directly. \
		@param self The isoch port interface to use. \
		@result Return kIOReturnSuccess on success, other return any other IOReturn error code.*/ \
	IOReturn	(*Start)		( IOFireWireLibIsochPortRef self ) ;	\
	/*!	@function Stop \
		@abstract The method is called when the port is to stop talking or listening. \
		@discussion This method is called by the channel object to which a port \
			has been added. Subclasses of IOFireWireIsochPortInterface override \
			this method to support specific hardware. Do not call this method \
			directly. \
		@param self The isoch port interface to use. \
		@result Return kIOReturnSuccess on success, other return any \
			other IOReturn error code.*/ \
	IOReturn	(*Stop)			( IOFireWireLibIsochPortRef self ) ;	\
	/*!	@function SetRefCon \
		@abstract Set reference value associated with this port. \
		@discussion Retrieve the reference value with GetRefCon() \
		@param self The isoch port interface to use. \
		@param inRefCon The new reference value.*/ \
	void 		(*SetRefCon)	( IOFireWireLibIsochPortRef self, void* inRefCon) ;	\
	/*!	@function GetRefCon \
		@abstract Get reference value associated with this port. \
		@discussion Set the reference value with SetRefCon() \
		@param self The isoch port interface to use. \
		@result The port refcon value.*/ \
	void*		(*GetRefCon)	( IOFireWireLibIsochPortRef self)

/*!	@class
	@abstract FireWire user client isochronous port interface
	@discussion Isochronous ports represent talkers or listeners on a
		FireWire isochronous channel. This is a base class containing all
		isochronous port functionality not specific to any type of port.
		Ports are added to channel interfaces
		(IOFireWireIsochChannelInterface) which coordinate the start and
		stop of isochronous traffic on a FireWire bus isochronous channel.
 */
typedef struct IOFireWireIsochPortInterface_t
{
	IUNKNOWN_C_GUTS ;
	/*! Interface revision. */
	UInt32 revision;
	/*! Interface version. */
	UInt32 version;

	IOFIREWIRELIBISOCHPORT_C_GUTS ;

} IOFireWireIsochPortInterface ;

/*! @class
	Description forthcoming
 */
typedef struct IOFireWireRemoteIsochPortInterface_t
{
	IUNKNOWN_C_GUTS ;
	/*! Interface revision. */
	UInt32 revision;
	/*! Interface version. */
	UInt32 version;

	IOFIREWIRELIBISOCHPORT_C_GUTS ;

	/*! Description forthcoming */
	IOFireWireLibIsochPortGetSupportedCallback (*SetGetSupportedHandler) ( IOFireWireLibRemoteIsochPortRef self, IOFireWireLibIsochPortGetSupportedCallback inHandler) ;
	/*! Description forthcoming */
	IOFireWireLibIsochPortAllocateCallback	   (*SetAllocatePortHandler) ( IOFireWireLibRemoteIsochPortRef self, IOFireWireLibIsochPortAllocateCallback inHandler) ;
	/*! Description forthcoming */
	IOFireWireLibIsochPortCallback	(*SetReleasePortHandler)( IOFireWireLibRemoteIsochPortRef self, IOFireWireLibIsochPortCallback inHandler) ;
	/*! Description forthcoming */
	IOFireWireLibIsochPortCallback	(*SetStartHandler)( IOFireWireLibRemoteIsochPortRef self, IOFireWireLibIsochPortCallback inHandler) ;
	/*! Description forthcoming */
	IOFireWireLibIsochPortCallback	(*SetStopHandler)( IOFireWireLibRemoteIsochPortRef self, IOFireWireLibIsochPortCallback inHandler) ;

} IOFireWireRemoteIsochPortInterface ;

/*!	@class
	@abstract FireWire user client local isochronous port object.
	@discussion Represents a FireWire isochronous talker or listener
		within the local machine. Isochronous transfer is controlled by
		an associated DCL (Data Stream Control Language) program, which
		is similar to a hardware DMA program but is hardware agnostic.
		DCL programs can be written using the
		IOFireWireDCLCommandPoolInterface object.

	This interface contains all methods of IOFireWireIsochPortInterface
		and IOFireWireLocalIsochPortInterface. This interface will
		contain all v2 methods of IOFireWireLocalIsochPortInterface when
		instantiated as v2 or newer.
		
	Transfer buffers for the local isoch port must all come from a single allocation
	made with vm_allocate() or mmap(..., MAP_ANON ).
	
	Calling vm_deallocate() on the buffers before deallocating a local isoch port object
	may result in a deadlock.
	
	Note: Calling Release() on the local isoch port may not immediately release the isoch port;
	so it may not be safe to call vm_deallocate() on your transfer buffers. To guarantee
	the port has been release, run the isochronous runloop until the port is finalized (it has
	processed any pending callbacks). The finalize callback will be called when
	the port is finalized. Set the finalize callback using SetFinalizeCallback().
	*/
typedef struct IOFireWireLocalIsochPortInterface_t {

	IUNKNOWN_C_GUTS ;
	/*! Interface revision */
	UInt32 revision;
	/*! Interface revision */
	UInt32 version;

	IOFIREWIRELIBISOCHPORT_C_GUTS ;
	
	/*!	@function ModifyJumpDCL
		@abstract Change the jump target label of a jump DCL.
		@discussion Use this function to change the flow of a DCL
			program. Works whether the DCL program is currently running
			or not.
		@param self The local isoch port interface to use.
		@param inJump The jump DCL to modify.
		@param inLabel The label to jump to.
		@result kIOReturnSuccess on success. Will return an error if 'inJump'
			does not point to a valid jump DCL or 'inLabel' does not point to a
			valid label DCL.*/
	IOReturn 	(*ModifyJumpDCL)( IOFireWireLibLocalIsochPortRef self, DCLJump* inJump, DCLLabel* inLabel) ;
	
	// --- utility functions ----------
	/*!	@function PrintDCLProgram
		@abstract Display the contents of a DCL program.
		@param self The local isoch port interface to use.
		@param inProgram A pointer to the first DCL of the program to display.
		@param inLength The length (in DCLs) of the program.*/
	void 		(*PrintDCLProgram)( IOFireWireLibLocalIsochPortRef self, const DCLCommand* inProgram, UInt32 inLength) ;

	//
	// --- v2
	//
	
	/*!	@function ModifyTransferPacketDCLSize
		@abstract Modify the transfer size of a transfer packet DCL (send or
			receive)
		@discussion Allows you to modify transfer packet DCLs after they have
			been compiled and while the DCL program is still running. The
			transfer size can be set to any size less than or equal to the size
			set when the DCL program was compiled (including 0).

		Availability: IOFireWireLocalIsochPortInterface_v2 and newer.

		@param self The local isoch port interface to use.
		@param inDCL A pointer to the DCL to modify.
		@param size The new size of data to be transferred.
		@result Returns kIOReturnSuccess on success. Will return an
			error if 'size' is too large for this program.*/
	IOReturn	(*ModifyTransferPacketDCLSize)( IOFireWireLibLocalIsochPortRef self, DCLTransferPacket* inDCL, IOByteCount size ) ;

	//
	// --- v3
	//
	
	/*!	@function ModifyTransferPacketDCLBuffer
		@abstract NOT IMPLEMENTED. Modify the transfer size of a
			transfer packet DCL (send or receive)
		@discussion NOT IMPLEMENTED. Allows you to modify transfer packet DCLs
			after they have been compiled and while the DCL program is still
			running. The buffer can be set to be any location within the range
			of buffers specified when the DCL program was compiled (including
			0).

		Availability: IOFireWireLocalIsochPortInterface_v3 and newer.

		@param self The local isoch port interface to use.
		@param inDCL A pointer to the DCL to modify.
		@param buffer The new buffer to or from data will be transferred.
		@result Returns kIOReturnSuccess on success. Will return an
			error if the range specified by [buffer, buffer+size] is not
			in the range of memory locked down for this program.*/
	IOReturn	(*ModifyTransferPacketDCLBuffer)( IOFireWireLibLocalIsochPortRef self, DCLTransferPacket* inDCL, void* buffer ) ;

	/*!	@function ModifyTransferPacketDCL
		@abstract Modify the transfer size of a transfer packet DCL (send or receive)
		@discussion Allows you to modify transfer packet DCLs after they
			have been compiled and while the DCL program is still
			running. The transfer size can be set to any size less than
			or equal to the size set when the DCL program was compiled
			(including 0).
			
			Availability: IOFireWireLocalIsochPortInterface_v3 and newer.
			
		@param self The local isoch port interface to use.
		@param inDCL A pointer to the DCL to modify.
		@param buffer The new buffer to or from data will be transferred.
		@param size The new size of data to be transferred.
		@result Returns kIOReturnSuccess on success. Will return an
			error if 'size' is too large or 'inDCL' does not point to a
			valid transfer packet DCL, or the range specified by
			[buffer, buffer+size] is not in the range of memory locked
			down for this program.*/
	IOReturn	(*ModifyTransferPacketDCL)( IOFireWireLibLocalIsochPortRef self, DCLTransferPacket* inDCL, void* buffer, IOByteCount size ) ;

	//
	// v4
	// 
	
	/*!	@function SetFinalizeCallback
		@abstract Set the finalize callback for a local isoch port
		@discussion When Stop() is called on a LocalIsochPortInterface, there may or
			may not be isoch callbacks still pending for this isoch port. The port must be allowed
			to handle any pending callbacks, so the isoch runloop should not be stopped until a port 
			has handled all pending callbacks. The finalize callback is called after the final 
			callback has been made on the isoch runloop. After this callback is sent, it is safe
			to stop the isoch runloop.
			
			You should not access the isoch port after the finalize callback has been made; it may
			be released immediately after this callback is sent.
						
			Availability: IOFireWireLocalIsochPortInterface_v4 and newer.
			
		@param self The local isoch port interface to use.
		@param finalizeCallback The finalize callback.
		@result Returns true if this isoch port has no more pending callbacks and does not
			need any more runloop time.*/
	IOReturn		(*SetFinalizeCallback)( IOFireWireLibLocalIsochPortRef self, IOFireWireLibIsochPortFinalizeCallback finalizeCallback ) ;

	//
	// v5
	//
	
	IOReturn		(*SetResourceUsageFlags)( IOFireWireLibLocalIsochPortRef self, IOFWIsochResourceFlags flags ) ;
	IOReturn		(*Notify)( IOFireWireLibLocalIsochPortRef self, IOFWDCLNotificationType notificationType, void ** inDCLList, UInt32 numDCLs ) ;

} IOFireWireLocalIsochPortInterface ;

// ============================================================
//
// IOFireWireIsochChannelInterface
//
// ============================================================

	/*!	@class
		@abstract FireWire user client isochronous channel object.
		@discussion IOFireWireIsochChannelInterface is an abstract
			representataion of a FireWire bus isochronous channel. This
			interface coordinates starting and stopping traffic on a
			FireWire bus isochronous channel and can optionally
			communicate with the IRM to automatically allocate bandwidth
			and channel numbers. When using automatic IRM allocation,
			the channel interface reallocates its bandwidth and channel
			reservation after each bus reset.

		Isochronous port interfaces representing FireWire isochronous talkers
			and listeners must be added to the channel using SetTalker() and
			AddListener()
	*/
typedef struct IOFireWireIsochChannelInterface_t
{
	IUNKNOWN_C_GUTS ;
	/*! Interface revision. */
	UInt32 revision;
	/*! Interface version. */
	UInt32 version;

	/*!	@function SetTalker
		@abstract Set the talker port for this channel.
		@param self The isoch channel interface to use.
		@param talker The new talker.
		@result Returns an IOReturn error code. */
	IOReturn 			(*SetTalker)		( IOFireWireLibIsochChannelRef self, IOFireWireLibIsochPortRef talker ) ;

	/*!	@function AddListener
		@abstract Modify the transfer size of a transfer packet DCL (send or receive)
		@discussion Allows you to modify transfer packet DCLs after they have
			been compiled and while the DCL program is still running. The
			transfer size can be set to any size less than or equal to the size
			set when the DCL program was compiled (including 0).

		Availability: IOFireWireLocalIsochPortInterface_v3 and newer.

		@param self The isoch channel interface to use.
		@param listener The listener to add.
		@result Returns an IOReturn error code. */
	IOReturn			(*AddListener)		( IOFireWireLibIsochChannelRef self, IOFireWireLibIsochPortRef listener ) ;
	
	/*!	@function AllocateChannel
		@abstract Prepare all hardware to begin sending or receiving isochronous data.
		@discussion Calling this function will result in all listener and talker ports on this 
			isochronous channel having their AllocatePort method called.
		@param self The isoch channel interface to use.
		@result Returns an IOReturn error code. */
	IOReturn			(*AllocateChannel)	( IOFireWireLibIsochChannelRef self ) ;

	/*!	@function ReleaseChannel
		@abstract Release all hardware after stopping the isochronous channel.
		@discussion Calling this function will result in all listener and talker ports on this 
			isochronous channel having their ReleasePort method called.
		@param self The isoch channel interface to use.
		@result Returns an IOReturn error code. */
	IOReturn			(*ReleaseChannel)	( IOFireWireLibIsochChannelRef self ) ;

	/*!	@function Start
		@abstract Start the channel.
		@discussion Calling this function will result in all listener and talker ports on this 
			isochronous channel having their Start method called.
		@param self The isoch channel interface to use.
		@result Returns an IOReturn error code. */
	IOReturn			(*Start)			( IOFireWireLibIsochChannelRef self ) ;

	/*!	@function Stop
		@abstract Stop the channel.
		@discussion Calling this function will result in all listener and talker ports on this 
			isochronous channel having their Stop method called.
		@param self The isoch channel interface to use.
		@result Returns an IOReturn error code. */
	IOReturn			(*Stop)				( IOFireWireLibIsochChannelRef self ) ;
	
	// --- notification
	/*!	@function SetChannelForceStopHandler
		@abstract Set the channel force stop handler.
		@discussion The specified callback is called when the channel is stopped and cannot be 
			restarted automatically.
		@param self The isoch channel interface to use.
		@param stopProc The handler to set.
		@result Returns the previously set handler or NULL is no handler was set.*/
	IOFireWireIsochChannelForceStopHandler (*SetChannelForceStopHandler)
													( IOFireWireLibIsochChannelRef self, IOFireWireIsochChannelForceStopHandler stopProc) ;

	/*!	@function SetRefCon
		@abstract Set reference value associated with this channel.
		@discussion Retrieve the reference value with GetRefCon()
		@param self The isoch channel interface to use.
		@param stopProcRefCon The new reference value.*/
	void 				(*SetRefCon)				( IOFireWireLibIsochChannelRef self, void* stopProcRefCon) ;

	/*!	@function GetRefCon
		@abstract Set reference value associated with this channel.
		@discussion Retrieve the reference value with SetRefCon()
		@param self The isoch channel interface to use.*/
	void*				(*GetRefCon)				( IOFireWireLibIsochChannelRef self) ;

	/*! Description forthcoming */
	Boolean				(*NotificationIsOn) 		( IOFireWireLibIsochChannelRef self) ;
	/*! Description forthcoming */
	Boolean				(*TurnOnNotification) 		( IOFireWireLibIsochChannelRef self) ;
	/*! Description forthcoming */
	void				(*TurnOffNotification) 		( IOFireWireLibIsochChannelRef self) ;	
	/*! Description forthcoming */
	void				(*ClientCommandIsComplete)	( IOFireWireLibIsochChannelRef self, FWClientCommandID commandID, IOReturn status) ;

} IOFireWireIsochChannelInterface ;

// ============================================================
//
// IOFireWireDCLCommandPoolInterface
//
// ============================================================

/*!	@class IOFireWireDCLCommandPoolInterface
	Description forthcoming.
*/
typedef struct IOFireWireDCLCommandPoolInterface_t
{
	IUNKNOWN_C_GUTS ;
	/*! Interface revision. */
	UInt32 revision;
	/*! Interface version. */
	UInt32 version;

	/*! Description forthcoming */
	DCLCommand*			(*Allocate)						( IOFireWireLibDCLCommandPoolRef self, IOByteCount inSize ) ;
	/*! Description forthcoming */
	IOReturn			(*AllocateWithOpcode)			( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, DCLCommand** outDCL, UInt32 opcode, ... ) ;
	
	/*! Description forthcoming */
	DCLCommand*			(*AllocateTransferPacketDCL)	( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, UInt32 inOpcode, void* inBuffer, IOByteCount inSize) ;
	/*! Description forthcoming */
	DCLCommand*			(*AllocateTransferBufferDCL)	( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, UInt32 inOpcode, void* inBuffer, IOByteCount inSize, IOByteCount inPacketSize, UInt32 inBufferOffset) ;

	/*! Description forthcoming */
	DCLCommand*			(*AllocateSendPacketStartDCL)	( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, void* inBuffer, IOByteCount inSize) ;
	
	// AllocateSendPacketWithHeaderStartDCL has been deprecated! If you need this functionality, you should be using NuDCL!
	/*! Description forthcoming */
	DCLCommand*			(*AllocateSendPacketWithHeaderStartDCL)( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, void* inBuffer, IOByteCount inSize) ;
	
	/*! Description forthcoming */
	DCLCommand*			(*AllocateSendBufferDCL)		( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, void* inBuffer, IOByteCount inSize, IOByteCount inPacketSize, UInt32 inBufferOffset) ;
	/*! Description forthcoming */
	DCLCommand*			(*AllocateSendPacketDCL)		( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, void* inBuffer, IOByteCount inSize) ;

	/*! Description forthcoming */
	DCLCommand*			(*AllocateReceivePacketStartDCL)( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, void* inBuffer, IOByteCount inSize) ;
	/*! Description forthcoming */
	DCLCommand*			(*AllocateReceivePacketDCL)		( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, void* inBuffer, IOByteCount inSize) ;
	/*! Description forthcoming */
	DCLCommand*			(*AllocateReceiveBufferDCL)		( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, void* inBuffer, IOByteCount inSize, IOByteCount inPacketSize, UInt32 inBufferOffset) ;

	/*! Description forthcoming */
	DCLCommand*			(*AllocateCallProcDCL)			( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, DCLCallCommandProc* inProc, DCLCallProcDataType inProcData) ;
	/*! Description forthcoming */
	DCLCommand*			(*AllocateLabelDCL)				( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL) ;
	/*! Description forthcoming */
	DCLCommand*			(*AllocateJumpDCL)				( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, DCLLabel* pInJumpDCLLabel) ;
	/*! Description forthcoming */
	DCLCommand*			(*AllocateSetTagSyncBitsDCL)	( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, UInt16 inTagBits, UInt16 inSyncBits) ;
	/*! Description forthcoming */
	DCLCommand*			(*AllocateUpdateDCLListDCL)		( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, DCLCommand** inDCLCommandList, UInt32 inNumCommands) ;
	/*! Description forthcoming */
	DCLCommand*			(*AllocatePtrTimeStampDCL)		( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL, UInt32* inTimeStampPtr) ;

	/*! Description forthcoming */
	void 				(*Free)							( IOFireWireLibDCLCommandPoolRef self, DCLCommand* inDCL ) ;
	
	/*! Description forthcoming */
	IOByteCount			(*GetSize)						( IOFireWireLibDCLCommandPoolRef self ) ;
	/*! Description forthcoming */
	Boolean				(*SetSize)						( IOFireWireLibDCLCommandPoolRef self, IOByteCount inSize ) ;
	/*! Description forthcoming */
	IOByteCount			(*GetBytesRemaining)			( IOFireWireLibDCLCommandPoolRef self ) ;
} IOFireWireDCLCommandPoolInterface ;

/*!	@class IOFireWireNuDCLPoolInterface
	@discussion Use this interface to build NuDCL-based DCL programs.
*/
typedef struct IOFireWireNuDCLPoolInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version */
	UInt32 revision;
	/*! Interface version */
	UInt32 version;

	// Command pool management:

	/*!	@function GetProgram
		@abstract Finds the first DCL in the pool not preceeded by any other DCL.
		@discussion Returns a backwards-compatible DCL program pointer. This can be passed
			to IOFireWireLibDeviceRef::CreateLocalIsochPort.
		@param self The NuDCL pool to use.
		@result A DCLCommand pointer.*/
	DCLCommand*				(*GetProgram)( IOFireWireLibNuDCLPoolRef self ) ;
	
	/*!	@function GetDCLs
		@abstract Returns the pool's DCL program as a CFArray of NuDCLRef's. 
		@param self The NuDCL pool to use.
		@result A CFArrayRef.*/
	CFArrayRef				(*GetDCLs)( IOFireWireLibNuDCLPoolRef self ) ;

	/*! Description forthcoming */
	void					(*PrintProgram)( IOFireWireLibNuDCLPoolRef self ) ;
	/*! Description forthcoming */
	void					(*PrintDCL)( NuDCLRef dcl ) ;
	
	// Allocating transmit NuDCLs:

	/*!	@function SetCurrentTagAndSync
		@abstract Set current tag and sync bits
		@discussion Sets the DCL pool's current tag and sync bits. All send DCLs allocated after calling
			this function will transmit the specified tag and sync values. These fields can also be
			set on each DCL using SetDCLTagBits() and SetDCLSyncBits().
		@param self The NuDCL pool to use.
		@param tag Tag field value for subsequently allocated send DCLs
		@param sync Sync field value for subsequently allocated send DCLs */
	void					(*SetCurrentTagAndSync)( IOFireWireLibNuDCLPoolRef self, UInt8 tag, UInt8 sync ) ;

	/*!	@function AllocateSendPacket
		@abstract Allocate a SendPacket NuDCL and append it to the program.
		@discussion The SendPacket DCL sends an isochronous packet on the bus. One DCL runs per bus cycle.
			The isochronous header is automatically generated, but can be overriden. An update must be run to
			regenerate the isochronous header. The sync and tag fields of allocated DCLs default to 0, unless
			If SetCurrentTagAndSync has been called.
			
			Send DCLs can be modified using other functions of IOFireWireLibNuDCLPool.
		@param self The NuDCL pool to use.
		@param saveBag The allocated DCL can be added to a CFBag for easily setting DCL update lists. Pass a CFMutableSetRef to add the allocated
			DCL to a CFBag; pass NULL to ignore. SaveBag is unmodified on failure.
		@param numBuffers The number of virtual ranges in 'buffers'.
		@param buffers An array of virtual memory ranges containing the packet contents. The array is copied
			into the DCL.
		@result Returns an NuDCLSendPacketRef on success or 0 on failure. */
	NuDCLSendPacketRef		(*AllocateSendPacket)( IOFireWireLibNuDCLPoolRef self, CFMutableSetRef saveBag, UInt32 numBuffers, IOVirtualRange* buffers ) ;	

	/*!	@function AllocateSendPacket_v
		@abstract Allocate a SendPacket NuDCL and append it to the program.
		@discussion Same as AllocateSendPacket but ranges are passed as a NULL-terminated vector of IOVirtualRange's
		@param self The NuDCL pool to use.
		@param saveBag The allocated DCL can be added to a CFBag for easily setting DCL update lists. Pass a CFMutableSetRef to add the allocated
			DCL to a CFBag; pass NULL to ignore. SaveBag is unmodified on failure.
		@param firstRange The first buffer to be transmitted. Follow with additional ranges; terminate with NULL.
		@result Returns an NuDCLSendPacketRef on success or 0 on failure. */
	NuDCLSendPacketRef		(*AllocateSendPacket_v)( IOFireWireLibNuDCLPoolRef self, CFMutableSetRef saveBag, IOVirtualRange* firstRange, ... ) ;	

	/*!	@function AllocateSkipCycle
		@abstract Allocate a SkipCycle NuDCL and append it to the program.
		@discussion The SkipCycle DCL causes the DCL program to "sends" an empty cycle.
		@param self The NuDCL pool to use.
		@result Returns an NuDCLSkipCycleRef on success or 0 on failure. */
	NuDCLSkipCycleRef		(*AllocateSkipCycle)( IOFireWireLibNuDCLPoolRef self ) ;
	
	// Allocating receive NuDCLs:

	/*!	@function AllocateReceivePacket
		@abstract Allocate a ReceivePacket NuDCL and append it to the program
		@discussion The ReceivePacket DCL receives an isochronous packet from the bus. One DCL runs per bus cycle.
			If receiving isochronous headers, an update must be run before the isochronous header is valid.
			
			Receive DCLs can be modified using other functions of IOFireWireLibNuDCLPool.

		@param self The NuDCL pool to use.
		@param headerBytes Number of bytes of isochronous header to receive with the data. Valid values are 0, 4, and 8.
		@param saveBag The allocated DCL can be added to a CFBag for easily setting DCL update lists. Pass a CFMutableSetRef to add the allocated
			DCL to a CFBag; pass NULL to ignore. SaveBag is unmodified on failure.
		@param numBuffers The number of virtual ranges in 'buffers'.
		@param buffers An array of virtual memory ranges containing the packet contents. The array is copied
			into the DCL.
		@result Returns an NuDCLReceivePacketRef on success or 0 on failure. */
	NuDCLReceivePacketRef	(*AllocateReceivePacket)( IOFireWireLibNuDCLPoolRef self, CFMutableSetRef saveBag, UInt8 headerBytes, UInt32 numBuffers, IOVirtualRange* buffers ) ;

	/*!	@function AllocateReceivePacket_v
		@abstract Allocate a ReceivePacket NuDCL and append it to the program
		@discussion Same as AllocateReceivePacket but ranges are passed as a NULL-terminated vector of IOVirtualRange's
		@param self The NuDCL pool to use.
		@param saveBag The allocated DCL can be added to a CFBag for easily setting DCL update lists. Pass a CFMutableSetRef to add the allocated
			DCL to a CFBag; pass NULL to ignore. SaveBag is unmodified on failure.
		@param headerBytes Number of bytes of isochronous header to receive with the data. Valid values are 0, 4, and 8.
		@param firstRange The first buffer to be transmitted. Follow with additional ranges; terminate with NULL.
		@result Returns an NuDCLReceivePacketRef on success or 0 on failure. */
	NuDCLReceivePacketRef	(*AllocateReceivePacket_v)( IOFireWireLibNuDCLPoolRef self, CFMutableSetRef saveBag, UInt8 headerBytes, IOVirtualRange* firstRange, ... ) ;

	// NuDCL configuration

	/*!	@function FindDCLNextDCL
		@abstract Get the next pointer for a NuDCL
		@discussion Applies: Any NuDCLRef
		@param dcl The dcl whose next pointer will be returned
		@result Returns the DCL immediately following this DCL in program order (ignoring branches) or 0 for none.*/
	NuDCLRef			(*FindDCLNextDCL)( IOFireWireLibNuDCLPoolRef self, NuDCLRef dcl ) ;

	/*!	@function SetDCLBranch
		@abstract Set the branch pointer for a NuDCL
		@discussion Program execution will jump to the DCL pointed to by 'branchDCL', after the DCL is executed. If set to 0, 
			execution will stop after this DCL.
		
			This change will apply immediately to a non-running DCL program. To apply the change to a running program
			use IOFireWireLocalIsochPortInterface::Notify()

			Applies: Any NuDCLRef.
		@result Returns an IOReturn error code.*/
	IOReturn			(*SetDCLBranch)( NuDCLRef dcl, NuDCLRef branchDCL ) ;

	/*!	@function GetDCLBranch
		@abstract Get the branch pointer for a NuDCL
		@discussion Applies: Any NuDCLRef.
		@param dcl The dcl whose branch pointer will be returned.
		@result Returns the branch pointer of 'dcl' or 0 for none is set.*/
	NuDCLRef			(*GetDCLBranch)( NuDCLRef dcl ) ;

	/*!	@function SetDCLTimeStampPtr
		@abstract Set the time stamp pointer for a NuDCL
		@discussion Setting a the time stamp pointer for a NuDCL causes a time stamp to be recorded when a DCL executes. 
			This DCL must be updated after it has executed for the timestamp to be valid.

			This change will apply immediately to a non-running DCL program. To apply the change to a running program
			use IOFireWireLocalIsochPortInterface::Notify()

			Applies: Any NuDCLRef.

		@param dcl The DCL for which time stamp pointer will be set
		@param timeStampPtr A pointer to a quadlet which will hold the timestamp after 'dcl' is updated.
		@result Returns an IOReturn error code.*/
	IOReturn			(*SetDCLTimeStampPtr)( NuDCLRef dcl, UInt32* timeStampPtr ) ;

	/*!	@function GetDCLTimeStampPtr
		@abstract Get the time stamp pointer for a NuDCL.
		@discussion Applies: Any NuDCLRef.

		@param dcl The DCL whose timestamp pointer will be returned.
		@result Returns a UInt32 time stamp pointer.*/
	UInt32*				(*GetDCLTimeStampPtr)( NuDCLRef dcl ) ;

	/*!	@function SetDCLStatusPtr
		@abstract Set the status pointer for a NuDCL
		@discussion Setting a the status pointer for a NuDCL causes the packet transmit/receive hardware status to be recorded when the DCL executes. 
			This DCL must be updated after it has executed for the status to be valid. 
			
			This change will apply immediately to a non-running DCL program. To apply the change to a running program
			use IOFireWireLocalIsochPortInterface::Notify()

			Status values are as follows: (from the OHCI spec, section 3.1.1)
			
			<dfn><table bgcolor="#EBEBEB">
				<tr>
					<td><b>5'h00</b></td> <td>No event status. </td>
				</tr>
				<tr>
					<td><b>5'h02</b></td> <td>evt_long_packet (receive)</td> <td>The received data length was greater than the buffer's data_length. </td>
				</tr>
				<tr>
					<td><b>5'h05</b></td> <td>evt_overrun (receive)</td> <td>A receive FIFO overflowed during the reception of an isochronous packet. </td>
				</tr>
				<tr>
					<td><b>5'h06</b></td> <td>evt_descriptor_read (receive/transmit)</td> <td>An unrecoverable error occurred while the Host Controller was reading a descriptor  block. </td>
				</tr>
				<tr>
					<td><b>5'h07</b></td> <td>evt_data_read (transmit)</td> <td>An error occurred while the Host Controller was attempting to read from host memory  in the data stage of descriptor processing. </td>
				</tr>
				<tr>
					<td><b>5'h08</b></td> <td>evt_data_write (receive/transmit)</td> <td>An error occurred while the Host Controller was attempting to write to host memory  either in the data stage of descriptor processing (AR, IR), or when processing a single  16-bit host memory write (IT). </td>
				</tr>
				<tr>
					<td><b>5'h0A</b></td> <td>evt_timeout (transmit)</td> <td>Indicates that the asynchronous transmit response packet expired and was not  transmitted, or that an IT DMA context experienced a skip processing overflow (See  section9.3.4). </td>
				</tr>
				<tr>
					<td><b>5'h0B</b></td> <td>evt_tcode_err (transmit)</td> <td>A bad tCode is associated with this packet. The packet was flushed. </td>
				</tr>
				<tr>
					<td><b>5'h0E</b></td> <td>evt_unknown (receive/transmit)</td> <td>An error condition has occurred that cannot be represented by any other event codes defined herein. </td>
				</tr>
				<tr>
					<td><b>5'h11</b></td> <td>ack_complete (receive/transmit)</td> <td>No event occurred. (Success)</td>
				</tr>
				<tr>
					<td><b>5'h1D</b></td> <td>ack_data_error (receive)</td> <td>A data field CRC or data_length error.</td>
				</tr>
			</table>
			</dfn>
			
			Applies: Any NuDCLRef.

		@param dcl The DCL for which status pointer will be set
		@param statusPtr A pointer to a quadlet which will hold the status after 'dcl' is updated.
		@result Returns an IOReturn error code.*/
	IOReturn			(*SetDCLStatusPtr)( NuDCLRef dcl, UInt32* statusPtr ) ;

	/*!	@function GetDCLStatusPtr
		@abstract Get the status pointer for a NuDCL.
		@discussion Applies: Any NuDCLRef.
		@param dcl The DCL whose status pointer will be returned.
		@result Returns a UInt32 status pointer.*/
	UInt32*				(*GetDCLStatusPtr)( NuDCLRef dcl ) ;
	

	/*!	@function AppendDCLRanges
		@abstract Add a memory range to the scatter gather list of a NuDCL
		@discussion This change will apply immediately to a non-running DCL program. To apply the change to a running program
			use IOFireWireLocalIsochPortInterface::Notify()
		
			Applies: NuDCLSendPacketRef, NuDCLReceivePacketRef

		@param dcl The DCL to modify
		@param range A IOVirtualRange to add to this DCL buffer list. Do not pass NULL.
		@result Returns an IOReturn error code.*/
	IOReturn			(*AppendDCLRanges)			( NuDCLRef dcl, UInt32 numRanges, IOVirtualRange* range ) ;

	/*!	@function SetDCLRanges
		@abstract Set the scatter gather list for a NuDCL
		@discussion Set the list of data buffers for a DCL. Setting too many ranges may result in a memory region
			with too many discontinous physical segments for the hardware to send or receive in a single packet.
			This will result in an error when the program is compiled.

			This change will apply immediately to a non-running DCL program. To apply the change to a running program
			use IOFireWireLocalIsochPortInterface::Notify()

			Applies: NuDCLSendPacketRef, NuDCLReceivePacketRef

		@param dcl The DCL to modify
		@param numRanges number of ranges in 'ranges'.
		@param ranges An array of virtual ranges
		@result Returns an IOReturn error code.*/
	IOReturn			(*SetDCLRanges)				( NuDCLRef dcl, UInt32 numRanges, IOVirtualRange* ranges ) ;

	IOReturn			(*SetDCLRanges_v)			( NuDCLRef dcl, IOVirtualRange* firstRange, ... ) ;
	
	/*!	@function GetDCLRanges
		@abstract Get the scatter-gather list for a NuDCL
		@discussion 

			Applies: NuDCLSendPacketRef, NuDCLReceivePacketRef

		@param dcl The DCL to query
		@param maxRanges Description forthcoming.
		@param outRanges Description forthcoming.
		@result Returns the previously set handler or NULL is no handler was set.*/
	UInt32				(*GetDCLRanges)				( NuDCLRef dcl, UInt32 maxRanges, IOVirtualRange* outRanges ) ;

	/*!	@function CountDCLRanges
		@abstract Returns number of buffers for a NuDCL
		@discussion 

			Applies: NuDCLSendPacketRef, NuDCLReceivePacket

		@param dcl The DCL to query
		@result Returns number of ranges in DCLs scatter-gather list*/
	UInt32				(*CountDCLRanges)			( NuDCLRef dcl ) ;

	/*!	@function GetDCLSpan
		@abstract Returns a virtual range spanning lowest referenced buffer address to highest
		@discussion 

			Applies: NuDCLSendPacketRef, NuDCLReceivePacket

		@param dcl The DCL to query
		@result Returns an IOVirtualRange.*/
	IOReturn			(*GetDCLSpan)				( NuDCLRef dcl, IOVirtualRange* spanRange ) ;

	/*!	@function GetDCLSize
		@abstract Returns number of bytes to be transferred by a NuDCL
		@discussion 

			Applies: NuDCLSendPacketRef, NuDCLReceivePacket

		@param dcl The DCL to query
		@result Returns an IOByteCount.*/
	IOByteCount			(*GetDCLSize)				( NuDCLRef dcl ) ;

	/*!	@function SetDCLCallback
		@abstract Set the callback for a NuDCL
		@discussion A callback can be called each time a NuDCL is run. Use SetDCLCallback() to set the
			callback for a NuDCL. If the update option is also set, the callback will be called after the update
			has run.

			This change will apply immediately to a non-running DCL program. To apply the change to a running program
			use IOFireWireLocalIsochPortInterface::Notify()

			Applies: Any NuDCLRef
		
		@param dcl The DCL to modify
		@param callback The callback function.
		@result Returns an IOReturn error code.*/
	IOReturn			(*SetDCLCallback)				( NuDCLRef dcl, NuDCLCallback callback ) ;

	/*!	@function GetDCLCallback
		@abstract Get callback for a NuDCL
		@discussion Returns the callback function for a DCL

			Applies: Any NuDCLRef

		@param dcl The DCL to query
		@result Returns the DCLs callback function or NULL if none is set.*/
	NuDCLCallback		(*GetDCLCallback)( NuDCLRef dcl ) ;

	/*!	@function SetDCLUserHeaderPtr
		@abstract Set a user specified header for a send NuDCL
		@discussion Allows the client to create a custom header for a transmitted isochronous packet. The header is masked with 'mask', 
			and the FireWire system software fills in the masked out bits.
			
			This change will apply immediately to a non-running DCL program. 
			An update must be run on the DCL for changes to take effect in a running program.

			Applies: NuDCLSendPacketRef
		
		@param dcl The DCL to modify
		@param headerPtr A pointer to a two-quadlet header. See section 9.6 of the the OHCI specification. 
		@param mask A pointer to a two-quadlet mask. The quadlets in headerPtr are masked with 'mask' and the masked-out bits
			are replaced by the FireWire system software.
		@result Returns an IOReturn error code.*/
	IOReturn			(*SetDCLUserHeaderPtr)( NuDCLRef dcl, UInt32 * headerPtr, UInt32 * mask ) ;

	/*! Description forthcoming */
	UInt32 *			(*GetDCLUserHeaderPtr)( NuDCLRef dcl ) ;

	/*! Description forthcoming */
	UInt32 *			(*GetUserHeaderMaskPtr)( NuDCLRef dcl ) ;
	
	/*! Description forthcoming */
	void				(*SetDCLRefcon)( NuDCLRef dcl, void* refcon ) ;
	/*! Description forthcoming */
	void*				(*GetDCLRefcon)( NuDCLRef dcl ) ;
	
	/*! Description forthcoming */
	IOReturn			(*AppendDCLUpdateList)( NuDCLRef dcl, NuDCLRef updateDCL ) ;

	// consumes a reference on dclList..
	/*! Description forthcoming */
	IOReturn			(*SetDCLUpdateList)( NuDCLRef dcl, CFSetRef dclList ) ;
	/*! Description forthcoming */
	CFSetRef			(*CopyDCLUpdateList)( NuDCLRef dcl ) ;
	/*! Description forthcoming */
	IOReturn			(*RemoveDCLUpdateList)( NuDCLRef dcl ) ;
	
	/*! Description forthcoming */
	IOReturn			(*SetDCLWaitControl)( NuDCLRef dcl, Boolean wait ) ;
	
	/*! Description forthcoming */
	void				(*SetDCLFlags)( NuDCLRef dcl, UInt32 flags ) ;
	/*! Description forthcoming */
	UInt32				(*GetDCLFlags)( NuDCLRef dcl ) ;
	/*! Description forthcoming */
	IOReturn			(*SetDCLSkipBranch)( NuDCLRef dcl, NuDCLRef skipCycleDCL ) ;
	/*! Description forthcoming */
	NuDCLRef			(*GetDCLSkipBranch)( NuDCLRef dcl ) ;
	/*! Description forthcoming */
	IOReturn			(*SetDCLSkipCallback)( NuDCLRef dcl, NuDCLCallback callback ) ;
	/*! Description forthcoming */
	NuDCLCallback		(*GetDCLSkipCallback)( NuDCLRef dcl ) ;
	/*! Description forthcoming */
	IOReturn			(*SetDCLSkipRefcon)( NuDCLRef dcl, void * refcon ) ;
	/*! Description forthcoming */
	void *				(*GetDCLSkipRefcon)( NuDCLRef dcl ) ;
	/*! Description forthcoming */
	IOReturn			(*SetDCLSyncBits)( NuDCLRef dcl, UInt8 syncBits ) ;
	/*! Description forthcoming */
	UInt8				(*GetDCLSyncBits)( NuDCLRef dcl ) ;
	/*! Description forthcoming */
	IOReturn			(*SetDCLTagBits)( NuDCLRef dcl, UInt8 tagBits ) ;
	/*! Description forthcoming */
	UInt8				(*GetDCLTagBits)( NuDCLRef dcl ) ;

} IOFireWireNuDCLPoolInterface ;

#pragma mark -
#pragma mark ASYNCSTREAM LISTENER INTERFACE
// ============================================================
// IOFWAsyncStreamListener Interface
// ============================================================

/*!	@class
	@discussion Represents and provides management functions for a asyn stream listener object.
*/
typedef struct IOFWAsyncStreamListenerInterface_t
{
	IUNKNOWN_C_GUTS ;

	/*! Interface version. */
	UInt16 version;
	/*! Interface revision. */
	UInt16 revision;

	/*!	@function SetListenerHandler
		@abstract Set the callback that should be called to handle incoming async stream packets
		@param self The async stream interface to use.
		@param inReceiver The callback to set.
		@result Returns the callback that was previously set or nil for none.*/
	const IOFWAsyncStreamListenerHandler (*SetListenerHandler)( IOFWAsyncStreamListenerInterfaceRef self, IOFWAsyncStreamListenerHandler inReceiver) ;

	/*!	@function SetSkippedPacketHandler
		@abstract Set the callback that should be called when incoming packets are
			dropped by the address space.
		@param self The address space interface to use.
		@param inHandler The callback to set.
		@result Returns the callback that was previously set or nil for none.*/
	const IOFWAsyncStreamListenerSkippedPacketHandler (*SetSkippedPacketHandler)( IOFWAsyncStreamListenerInterfaceRef self, IOFWAsyncStreamListenerSkippedPacketHandler inHandler) ;

	/*!	@function NotificationIsOn
		@abstract Is notification on?
		@param self The async stream interface to use.
		@result Returns true if packet notifications for this channel are active */
	Boolean (*NotificationIsOn)(IOFWAsyncStreamListenerInterfaceRef self) ;

	/*!	@function TurnOnNotification
		@abstract Try to turn on packet notifications for this channel.
		@param self The async stream interface to use.
		@result Returns true upon success */
	Boolean (*TurnOnNotification)(IOFWAsyncStreamListenerInterfaceRef self) ;

	/*!	@function TurnOffNotification
		@abstract Force packet notification off.
		@param self The async stream interface to use. */
	void (*TurnOffNotification)(IOFWAsyncStreamListenerInterfaceRef self) ;	

	/*!	@function ClientCommandIsComplete
		@abstract Notify the async stream object that a packet notification handler has completed.
		@discussion Packet notifications are received one at a time, in order. This function
			must be called after a packet handler has completed its work.
		@param self The async stream interface to use.
		@param commandID The ID of the packet notification being completed. This is the same
			ID that was passed when a packet notification handler is called.
		@param status The completion status of the packet handler */
	void (*ClientCommandIsComplete)(IOFWAsyncStreamListenerInterfaceRef self, FWClientCommandID commandID, IOReturn status) ;

	// --- accessors ----------
	
	/*!	@function GetRefCon
		@abstract Returns the user refCon value for this async stream interface.
		@param self The async stream interface to use.
		@result returns the callback object.*/
	void* (*GetRefCon)(IOFWAsyncStreamListenerInterfaceRef self) ;

	/*!	@function SetFlags
		@abstract set flags for the listener.
		@param self The async stream interface to use.
		@param flags indicate performance metrics.
		@result none.	*/	
	void (*SetFlags)( IOFWAsyncStreamListenerInterfaceRef self, UInt32 flags );
		
	/*!	@function GetFlags
		@abstract get the flags of listener.
		@param self The async stream interface to use.
		@result flags.	*/	
	UInt32 (*GetFlags)(IOFWAsyncStreamListenerInterfaceRef self);
	
	
	/*!	@function GetOverrunCounter
		@abstract get overrun counter from the DCL program.
		@param self The async stream interface to use.
		@result returns the counter value.	*/	
	UInt32 (*GetOverrunCounter)(IOFWAsyncStreamListenerInterfaceRef self);

} IOFWAsyncStreamListenerInterface ;

#endif //__IOFireWireLibIsoch_H__
