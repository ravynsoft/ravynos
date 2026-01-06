/*
 * Copyright (c) 1998-2001 Apple Computer, Inc. All rights reserved.
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

#ifndef _IOKIT_IOFIREWIREAVCLIB_H_
#define _IOKIT_IOFIREWIREAVCLIB_H_

#include <IOKit/IOCFPlugIn.h>
#include <IOKit/firewire/IOFireWireFamilyCommon.h>
#include <IOKit/avc/IOFireWireAVCConsts.h>

// Unit type UUID
/* 6AAF2EF7-D476-11D5-B57C-0003934B81A0 */
#define kIOFireWireAVCLibUnitTypeID CFUUIDGetConstantUUIDWithBytes(NULL,		\
0x6A, 0xAF, 0x2E, 0xF7, 0xD4, 0x76, 0x11, 0xD5, 0xB5, 0x7C, 0x00, 0x03, 0x93, 0x4B, 0x81, 0xA0)

// Unit Factory UUID
/* 3F4057BC-D479-11D5-9F05-0003934B81A0 */
#define kIOFireWireAVCLibUnitFactoryID CFUUIDGetConstantUUIDWithBytes(NULL, 	\
0x3F, 0x40, 0x57, 0xBC, 0xD4, 0x79, 0x11, 0xD5, 0x9F, 0x05, 0x00, 0x03, 0x93, 0x4B, 0x81, 0xA0)

// IOFireWireAVCUnitInterface UUID
/* FC65C030-D498-11D5-878D-0003934B81A0 */
#define kIOFireWireAVCLibUnitInterfaceID CFUUIDGetConstantUUIDWithBytes(NULL, 	\
0xFC, 0x65, 0xC0, 0x30, 0xD4, 0x98, 0x11, 0xD5, 0x87, 0x8D, 0x00, 0x03, 0x93, 0x4B, 0x81, 0xA0)

// kIOFireWireAVCLibUnitInterfaceID_v2 UUID - No Throttling of AVC Commands
/* 85B5E954-0AEF-11D8-8D19-000393914ABA */
#define kIOFireWireAVCLibUnitInterfaceID_v2  CFUUIDGetConstantUUIDWithBytes(NULL, 	\
0x85, 0xB5, 0xE9, 0x54, 0x0A, 0xEF, 0x11, 0xD8, 0x8D, 0x19, 0x00, 0x03, 0x93, 0x91, 0x4A, 0xBA)

// Protocol type UUID
/* B54BC8F8-D53B-11D5-A1A1-0003934B81A0 */
#define kIOFireWireAVCLibProtocolTypeID CFUUIDGetConstantUUIDWithBytes(NULL,		\
0xB5, 0x4B, 0xC8, 0xF8, 0xD5, 0x3B, 0x11, 0xD5, 0xA1, 0xA1, 0x00, 0x03, 0x93, 0x4B, 0x81, 0xA0)

// Protocol Factory UUID
/* 8E9AD5AC-D55E-11D5-B7D2-0003934B81A0 */
#define kIOFireWireAVCLibProtocolFactoryID CFUUIDGetConstantUUIDWithBytes(NULL,		\
0x8E, 0x9A, 0xD5, 0xAC, 0xD5, 0x5E, 0x11, 0xD5, 0xB7, 0xD2, 0x00, 0x03, 0x93, 0x4B, 0x81, 0xA0)

// IOFireWireAVCProtocolInterface UUID
/* CC85D421-D55E-11D5-8A10-0003934B81A0 */
#define kIOFireWireAVCLibProtocolInterfaceID CFUUIDGetConstantUUIDWithBytes(NULL,		\
0xCC, 0x85, 0xD4, 0x21, 0xD5, 0x5E, 0x11, 0xD5, 0x8A, 0x10, 0x00, 0x03, 0x93, 0x4B, 0x81, 0xA0)

// IOFireWireAVCLibConsumerInterfaceID 
/* 7FB7A454-226F-11D6-B889-000A277E7234 */
#define kIOFireWireAVCLibConsumerInterfaceID CFUUIDGetConstantUUIDWithBytes(NULL,		\
0x7F, 0xB7, 0xA4, 0x54, 0x22, 0x6F, 0x11, 0xD6, 0xB8, 0x89, 0x00, 0x0A, 0x27, 0x7E, 0x72, 0x34)

typedef void (*IOFWAVCMessageCallback)( void * refCon, UInt32 type, void * arg );

/*! @typedef IOFWAVCRequestCallback
	@abstract This Callback has been deprecated. Use installAVCCommandHandler instead.
*/
typedef IOReturn (*IOFWAVCRequestCallback)( void *refCon, UInt32 generation, UInt16 srcNodeID,
                const UInt8 * command, UInt32 cmdLen, UInt8 * response, UInt32 *responseLen);

/*!
    @typedef IOFWAVCPCRCallback
	@abstract Callback called after a successful lock transaction to a CMP plug.
    @param refcon refcon supplied when a client is registered
    @param generation Bus generation command was received in
	@param nodeID is the node originating the request
	@param plug is the plug number
	@param oldVal is the value the plug used to contain
    @param newVal is the quad written into the plug
 */
typedef void (*IOFWAVCPCRCallback)(void *refcon, UInt32 generation, UInt16 nodeID, UInt32 plug,
                                                                    UInt32 oldVal, UInt32 newVal);

/*!
    @typedef IOFWAVCCommandHandlerCallback
    @abstract Callback called when a incoming AVC command matching a registered command handler is received.
    @param refCon The refcon supplied when a client is registered
    @param generation The FireWire bus generation value at the time the command was received
    @param srcNodeID The node ID of the device who sent us this command
    @param speed The speed the AVC command packet
    @param command A pointer to the command bytes
    @param cmdLen The length of the AVC command bytes buffer in bytes
    @result The callback handler should return success if it will send the AVC response, or an error if it doesn't want to handle the command
 */
typedef IOReturn (*IOFWAVCCommandHandlerCallback)( void *refCon, UInt32 generation, UInt16 srcNodeID, IOFWSpeed speed, const UInt8 * command, UInt32 cmdLen);

/*!
    @typedef IOFWAVCSubunitPlugHandlerCallback
    @abstract Callback called when a incoming AVC command matching a registered command handler is received.
    @param refCon The refcon supplied when a client is registered
    @param subunitTypeAndID The subunit type and id of this plug
    @param plugType The type of plug receiving the message
    @param plugNum The number of the plug receiving the message
    @param plugMessage The plug message
    @param messageParams The parameters associated with the plug message
    @result The return value is only pertinent for the kIOFWAVCSubunitPlugMsgSignalFormatModified message. Return an error if not accepting the sig format change.
 */
typedef IOReturn (*IOFWAVCSubunitPlugHandlerCallback)(void *refCon,
												   UInt32 subunitTypeAndID,
												   IOFWAVCPlugTypes plugType,
												   UInt32 plugNum,
												   IOFWAVCSubunitPlugMessages plugMessage,
												   UInt32 messageParams);

typedef struct _IOFireWireAVCLibProtocolInterface IOFireWireAVCLibProtocolInterface;

typedef struct _IOFireWireAVCLibAsynchronousCommand
{
	IOFWAVCAsyncCommandState cmdState;
	void	*pRefCon;
	UInt8	*pCommandBuf;
	UInt32	cmdLen;
	UInt8	*pInterimResponseBuf;
	UInt32	interimResponseLen;
	UInt8	*pFinalResponseBuf;
	UInt32	finalResponseLen;
}IOFireWireAVCLibAsynchronousCommand;

typedef void (*IOFireWireAVCLibAsynchronousCommandCallback)(void *pRefCon, IOFireWireAVCLibAsynchronousCommand *pCommandObject);

/*!
    @class IOFireWireAVCLibUnitInterface
    @abstract Initial interface discovered for all AVC Unit drivers. 
    @discussion The IOFireWireAVCLibUnitInterface is the initial interface discovered by most drivers. It supplies the methods that control the operation of the AVC unit as a whole.
    Finally, the Unit can supply a reference to the IOFireWireUnit.  This can be useful if a driver wishes to access the standard FireWire APIs.  
*/

typedef struct
 {

	IUNKNOWN_C_GUTS;

	UInt16	version;						
    UInt16	revision;

    /*!
		@function open
		@abstract Exclusively opens a connection to the in-kernel device.
		@discussion Exclusively opens a connection to the in-kernel device.  As long as the in-kernel 
        device object is open, no other drivers will be able to open a connection to the device. When 
        open, the device on the bus may disappear, but the in-kernel object representing it will stay
        instantiated and can begin communicating with the device again if it ever reappears. 
        @param self Pointer to IOFireWireAVCLibUnitInterface.
        @result Returns kIOReturnSuccess on success.
    */
    
	IOReturn (*open)( void * self );
    
    /*!
		@function openWithSessionRef
		@abstract Opens a connection to a device that is not already open.
		@discussion Sometimes it is desirable to open multiple user clients on a device.  In the case 
        of FireWire sometimes we wish to have both the FireWire User Client and the AVC User Client 
        open at the same time.  The technique to arbitrate this is as follows:<br>First open normally 
        the device furthest from the root in the I/O Registry.<br>Second, get its sessionRef with the 
        getSessionRef call.<br>Third, open the device further up the chain by calling this method and 
        passing the sessionRef returned from the call in step 2.
        @param sessionRef SessionRef returned from getSessionRef call. 
        @param self Pointer to IOFireWireAVCLibUnitInterface.
        @result Returns kIOReturnSuccess on success.
    */
    
	IOReturn (*openWithSessionRef)( void * self, IOFireWireSessionRef sessionRef );

    /*!
		@function getSessionRef
		@abstract Get the session reference.
		@discussion Gets the sessionRef to be used with openWithSessionRef.
        @param self Pointer to IOFireWireAVCLibUnitInterface.
        @result Returns a sessionRef on success.
    */

	IOFireWireSessionRef (*getSessionRef)(void * self);

    /*!
		@function close
		@abstract Closes an exclusive access to the device.
		@discussion Closes an exclusive access to the device.  When a device is closed it may be 
        unloaded by the kernel.  If it is unloaded and then later reappears it will be represented 
        by a different object.  You won't be able to use this user client on the new object.  The 
        new object will have to be looked up in the I/O Registry and a new user client will have to 
        be opened on it. 
        @param self Pointer to IOFireWireAVCLibUnitInterface.
    */

	void (*close)( void * self );

    /*!
		@function addCallbackDispatcherToRunLoop
		@abstract Adds a dispatcher for kernel callbacks to the specified runloop.
		@discussion The user space portions of the AVC API communicate with the in-kernel services by 
        messaging the kernel.  Similarly, the kernel messages the user space services in response.  
        These responses need to be picked up by a piece of code.  This call adds that code to the specified
        run loop.  Most drivers will call this method on the run loop that was created when your task was 
        created.  To avoid deadlock you must avoid sleeping (or spin waiting) the run loop to wait for 
        AVC response.  If you do this the dispatcher will never get to run and you will wait forever.
        @param self Pointer to IOFireWireAVCLibUnitInterface.
        @param cfRunLoopRef Reference to a run loop.
        @result Returns kIOReturnSuccess on success.
    */
    	
	IOReturn (*addCallbackDispatcherToRunLoop)( void *self, CFRunLoopRef cfRunLoopRef );
	
    /*!
		@function removeCallbackDispatcherFromRunLoop
		@abstract Removes a dispatcher for kernel callbacks to the specified run loop.
		@discussion Undoes the work of addCallbackDispatcherToRunLoop.
        @param self Pointer to IOFireWireAVCLibUnitInterface.
    */
    
    void (*removeCallbackDispatcherFromRunLoop)( void * self );

    /*!
		@function setMessageCallback
		@abstract Sets callback for user space message routine.
		@discussion In FireWire and AVC, bus status messages are delivered via IOKit's message routine.  
        This routine is emulated in user space for AVC and FireWire messages via this callback.  You should
        register here for bus reset and reconnect messages.
        @param self Pointer to IOFireWireAVCLibUnitInterface.
        @param refCon RefCon to be returned as first argument of completion routine.
        @param callback Address of completion routine.
    */
    
	void (*setMessageCallback)( void *self, void * refCon, IOFWAVCMessageCallback callback);
    
    /*!
		@function AVCCommand
		@abstract Sends an AVC command to the device and returns the response.
		@discussion This function will block until the device returns a response or the kernel driver times out. 
        @param self Pointer to IOFireWireAVCLibUnitInterface.
        @param command Pointer to command to send.
        @param cmdLen Length (in bytes) of command.
        @param response Pointer to place to store the response sent by the device.
        @param responseLen Pointer to place to store the length of the response.
    */

	IOReturn (*AVCCommand)( void * self,
        const UInt8 * command, UInt32 cmdLen, UInt8 * response, UInt32 *responseLen);
    
    /*!
		@function AVCCommandInGeneration
		@abstract Sends an AVC command to the device and returns the response.
		@discussion Sends an AVC command to the device and returns the response.  The command must complete in the specified bus generation.  This function is only available if the interface version is > 1 (MacOSX 10.2.0 or later?).  This function will block until the device returns a response or the kernel driver times out. 
        @param self Pointer to IOFireWireAVCLibUnitInterface.
        @param busGeneration FireWire bus generation that the command is valid in.
        @param command Pointer to command to send.
        @param cmdLen Length (in bytes) of command.
        @param response Pointer to place to store the response sent by the device.
        @param responseLen Pointer to place to store the length of the response.
    */

	IOReturn (*AVCCommandInGeneration)( void * self, UInt32 busGeneration,
        const UInt8 * command, UInt32 cmdLen, UInt8 * response, UInt32 *responseLen);
        
	/*!	
        @function getAncestorInterface
        @abstract Creates a plug-in object for an ancestor (in the I/O Registry) of the AVC unit and returns an interface to it.
        @discussion This function is only available if the interface version is > 1 (MacOSX 10.2.0 or later?).
        @param self Pointer to IOFireWireAVCLibUnitInterface.
        @param object_class Class name of ancestor of the device to get an interface for.
        @param pluginType An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the type of plug-in service to be returned for the ancestor.
        @param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the type of interface to be returned for the created plug-in object.
        @result Returns a COM-style interface pointer. Returns 0 upon failure.
    */
	void * (*getAncestorInterface)( void * self, char * object_class, REFIID pluginType, REFIID iid) ;

	/*!	
        @function getProtocolInterface
        @abstract Creates a plug-in object for a protocol driver for the FireWire bus the AVC unit
        is connected to and returns an interface to it.
        @discussion This function is only available if the interface version is > 1 (MacOSX 10.2.0 or later?).
        @param self Pointer to IOFireWireAVCLibUnitInterface.
        @param pluginType An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the type of plug-in service to be returned for the created protocol object.
        @param iid An ID number, of type CFUUIDBytes (see CFUUID.h), identifying the type of interface to be returned for the created protocol device object.
        @result Returns a COM-style interface pointer. Returns 0 upon failure.
    */
	void * (*getProtocolInterface)( void * self, REFIID pluginType, REFIID iid) ;
	
	/*
	*/
	IOReturn (*getAsyncConnectionPlugCounts)
                        ( void *self, UInt8 * inputPlugCount, UInt8 * outputPlugCount );

	/*
	*/
    IUnknownVTbl ** (*createConsumerPlug)( void *self, UInt8 plugNumber, REFIID iid );

    /*!
        @function updateAVCCommandTimeout
        @abstract Updates an AVCCommand's timeout back to 10 seconds.
        @discussion AVCCommands will time out after 10 seconds unless this function is called (from another thread) to update the command's timeout back to 10 seconds.
        This function is only available if the interface version is > 2.
    */
    IOReturn (*updateAVCCommandTimeout)(void * self);
    
    /*!
        @function makeP2PInputConnection
        @abstract Increments the point-to-point connection count of a unit input plug.
        @discussion This function is only available if the interface version is > 3.
    */
    IOReturn (*makeP2PInputConnection)(void * self, UInt32 inputPlug, UInt32 chan);
    
    /*!
        @function breakP2PInputConnection
        @abstract Decrements the point-to-point connection count of a unit input plug.
        @discussion This function is only available if the interface version is > 3.
    */
    IOReturn (*breakP2PInputConnection)(void * self, UInt32 inputPlug);

    /*!
        @function makeP2POutputConnection
        @abstract Increments the point-to-point connection count of a unit output plug.
        @discussion This function is only available if the interface version is > 3.
    */
    IOReturn (*makeP2POutputConnection)(void * self, UInt32 outputPlug, UInt32 chan, IOFWSpeed speed);
    
    /*!
        @function breakP2POutputConnection
        @abstract Decrements the point-to-point connection count of a unit output plug.
        @discussion This function is only available if the interface version is > 3.
    */
    IOReturn (*breakP2POutputConnection)(void * self, UInt32 outputPlug);

    /*!
		@function createAVCAsynchronousCommand
	 */

	IOReturn (*createAVCAsynchronousCommand)(void * self,
										  const UInt8 * command,
										  UInt32 cmdLen,
										  IOFireWireAVCLibAsynchronousCommandCallback completionCallback,
										  void *pRefCon,
										  IOFireWireAVCLibAsynchronousCommand **ppCommandObject);

    /*!
		@function AVCAsynchronousCommandSubmit
	 */

	IOReturn (*AVCAsynchronousCommandSubmit)(void * self, IOFireWireAVCLibAsynchronousCommand *pCommandObject);

    /*!
		@function AVCAsynchronousCommandReinit
	 */

	IOReturn (*AVCAsynchronousCommandReinit)(void * self, IOFireWireAVCLibAsynchronousCommand *pCommandObject);

	/*!
		@function AVCAsynchronousCommandCancel
	 */

	IOReturn (*AVCAsynchronousCommandCancel)(void * self, IOFireWireAVCLibAsynchronousCommand *pCommandObject);

    /*!
		@function AVCAsynchronousCommandRelease
	 */

	IOReturn (*AVCAsynchronousCommandRelease)(void * self, IOFireWireAVCLibAsynchronousCommand *pCommandObject);

    /*!
		@function AVCAsynchronousCommandReinitWithCommandBytes
	 */

	IOReturn (*AVCAsynchronousCommandReinitWithCommandBytes)(void * self, 
															 IOFireWireAVCLibAsynchronousCommand *pCommandObject, 
															 const UInt8 * command,
															 UInt32 cmdLen);
 } IOFireWireAVCLibUnitInterface;

/*!
    @class IOFireWireAVCLibProtocolInterface
    @abstract Initial interface discovered for all AVC protocol drivers. 
    @discussion The IOFireWireAVCLibProtocolInterface is used to set up local plug control registers and to receive AVC requests.
*/

typedef struct _IOFireWireAVCLibProtocolInterface
 {
	IUNKNOWN_C_GUTS;

	UInt16	version;						
    UInt16	revision;
    /*!
		@function addCallbackDispatcherToRunLoop
		@abstract Adds a dispatcher for kernel callbacks to the specified run loop.
		@discussion The user space portions of the AVC API communicate with the in-kernel services by 
        messaging the kernel.  Similarly, the kernel messages the user space services in response.  
        These responses need to be picked up by a piece of code.  This call adds that code to the specified
        run loop.  Most drivers will call this method on the run loop that was created when your task was 
        created.  To avoid deadlock you must avoid sleeping (or spin waiting) the run loop to wait for 
        AVC response.  If you do this the dispatcher will never get to run and you will wait forever.
        @param self Pointer to IOFireWireAVCLibProtocolInterface.
        @param cfRunLoopRef Reference to a run loop.
        @result Returns kIOReturnSuccess on success.
    */
    	
	IOReturn (*addCallbackDispatcherToRunLoop)( void *self, CFRunLoopRef cfRunLoopRef );
	
    /*!
        @function removeCallbackDispatcherFromRunLoop
        @abstract Removes a dispatcher for kernel callbacks to the specified run loop.
        @discussion Undoes the work of addCallbackDispatcherToRunLoop.
        @param self Pointer to IOFireWireAVCLibProtocolInterface.
    */
    
    void (*removeCallbackDispatcherFromRunLoop)( void * self );
    
    /*!
		@function setMessageCallback
		@abstract Sets callback for user space message routine.
		@discussion In FireWire and AVC, bus status messages are delivered via IOKit's message routine.  
        This routine is emulated in user space for AVC and FireWire messages via this callback.  You should
        register here for bus reset and reconnect messages.
        @param self Pointer to IOFireWireAVCLibProtocolInterface.
        @param refCon RefCon to be returned as first argument of completion routine.
        @param callback Address of completion routine.
    */
    
	void (*setMessageCallback)( void *self, void * refCon, IOFWAVCMessageCallback callback);
    
    /*!
		@function setAVCRequestCallback
		@abstract This function has been deprecated. Use installAVCCommandHandler instead.
    */
    
    IOReturn (*setAVCRequestCallback)( void *self, UInt32 subUnitType, UInt32 subUnitID,
                                                void *refCon, IOFWAVCRequestCallback callback);

/*!
    @function allocateInputPlug
    @abstract Allocates an input plug.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param refcon Arbitrary value passed back as first argument of callback.
    @param func Callback function when a successful lock transaction to the plug has been performed.
    @param plug Set to the plug number if a plug is successfully allocated.
*/
    IOReturn (*allocateInputPlug)( void *self, void *refcon, IOFWAVCPCRCallback func, UInt32 *plug);
/*!
    @function freeInputPlug
    @abstract Deallocates an input plug.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param plug Value returned by allocateInputPlug.
*/
    void (*freeInputPlug)( void *self, UInt32 plug);
/*!
    @function readInputPlug
    @abstract Returns the current value of an input plug.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param plug Value returned by allocateInputPlug.
*/
    UInt32 (*readInputPlug)( void *self, UInt32 plug);
/*!
    @function updateInputPlug
    @abstract Updates the value of an input plug (simulating a lock transaction).
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param plug Value returned by allocateInputPlug.
    @param oldVal Value returned by readInputPlug.
    @param newVal New value to store in plug if its current value is oldVal.
*/
    IOReturn (*updateInputPlug)( void *self, UInt32 plug, UInt32 oldVal, UInt32 newVal);
/*!
    @function allocateOutputPlug
    @abstract Allocates an output plug.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param refcon Arbitrary value passed back as first argument of callback.
    @param func Callback function when a successful lock transaction to the plug has been performed.
    @param plug Set to the plug number if a plug is successfully allocated.
*/
    IOReturn (*allocateOutputPlug)( void *self, void *refcon, IOFWAVCPCRCallback func, UInt32 *plug);
/*!
    @function freeOutputPlug
    @abstract Deallocates an output plug.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param plug Value returned by allocateOutputPlug.
*/
    void (*freeOutputPlug)( void *self, UInt32 plug);
/*!
    @function readOutputPlug
    @abstract Returns the current value of an output plug.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param plug Value returned by allocateOutputPlug.
*/
    UInt32 (*readOutputPlug)( void *self, UInt32 plug);
/*!
    @function updateOutputPlug
    @abstract Updates the value of an output plug (simulating a lock transaction).
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param plug Value returned by allocateOutputPlug.
    @param oldVal Value returned by readOutputPlug.
    @param newVal New value to store in plug if its current value is oldVal.
*/
   IOReturn (*updateOutputPlug)( void *self, UInt32 plug, UInt32 oldVal, UInt32 newVal);
/*!
    @function readOutputMasterPlug
    @abstract Returns the current value of the output master plug.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
*/
    UInt32 (*readOutputMasterPlug)( void *self);
/*!
    @function updateOutputMasterPlug
    @abstract Updates the value of the master output plug (simulating a lock transaction).
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param oldVal Value returned by readOutputMasterPlug.
    @param newVal New value to store in plug if its current value is oldVal.
*/
    IOReturn (*updateOutputMasterPlug)( void *self, UInt32 oldVal, UInt32 newVal);
/*!
    @function readInputMasterPlug
    @abstract Returns the current value of the input master plug.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    
*/
    UInt32 (*readInputMasterPlug)( void *self);
/*!
    @function updateInputMasterPlug
    @abstract Updates the value of the master input plug (simulating a lock transaction).
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param oldVal Value returned by readInputMasterPlug.
    @param newVal New value to store in plug if its current value is oldVal.
*/
    IOReturn (*updateInputMasterPlug)( void *self, UInt32 oldVal, UInt32 newVal);

/*!
    @function publishAVCUnitDirectory
    @abstract Publishes an AVC unit directory in the config ROM.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
*/
	IOReturn (*publishAVCUnitDirectory)(void *self);

/*!
    @function installAVCCommandHandler
    @abstract Installs a command handler for handling specific incoming AVC commands.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param subUnitTypeAndID The subunit type and ID for this command handler.
    @param opCode The opcode for this command handler.
    @param refCon Arbitrary value passed back as first argument of callback.
    @param callback A pointer to the callback function
*/
	IOReturn (*installAVCCommandHandler)(void *self,
									  UInt32 subUnitTypeAndID,
									  UInt32 opCode,
									  void *refCon,
									  IOFWAVCCommandHandlerCallback callback);

/*!
    @function sendAVCResponse
    @abstract Sends an AVC response packet.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param generation The Firewire bus generation that this response should be sent in.
    @param nodeID The node ID of the device we are sending this response to.
    @param response A pointer to the response bytes.
    @param responseLen The number of response bytes.
*/
	IOReturn (*sendAVCResponse)(void *self,
							 UInt32 generation,
							 UInt16 nodeID,
							 const char *response,
							 UInt32 responseLen);

/*!
    @function addSubunit
    @abstract Installs a virtual AVC subunit.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param subunitType The type of subunit to create.
    @param numSourcePlugs The number of source plugs for this subunit.
    @param numDestPlugs The number of destination plugs for this subunit.
    @param refCon Arbitrary value passed back as first argument of callback.
    @param callback A pointer to the callback to receive plug management messages.
    @param pSubunitTypeAndID A pointer to a byte to hold the returned subunit address for the new subunit.
 */
	IOReturn (*addSubunit)(void *self,
						UInt32 subunitType,
						UInt32 numSourcePlugs,
						UInt32 numDestPlugs,
						void *refCon,
						IOFWAVCSubunitPlugHandlerCallback callback,
						UInt32 *pSubunitTypeAndID);

/*!
    @function setSubunitPlugSignalFormat
    @abstract Sets the signal format of the specifed plug.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param subunitTypeAndID The subunit type and ID of the plug.
    @param plugType The plug type.
    @param plugNum The plug number.
    @param signalFormat The 32-bit signal format value.
*/
	IOReturn (*setSubunitPlugSignalFormat)(void *self,
										UInt32 subunitTypeAndID,
										IOFWAVCPlugTypes plugType,
										UInt32 plugNum,
										UInt32 signalFormat);

/*!
    @function getSubunitPlugSignalFormat
    @abstract Gets the signal format of the specifed plug.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param subunitTypeAndID The subunit type and ID of the plug.
    @param plugType The plug type.
    @param plugNum The plug number.
    @param pSignalFormat A pointer to the location to return the signal format value.
*/
	IOReturn (*getSubunitPlugSignalFormat)(void *self,
										UInt32 subunitTypeAndID,
										IOFWAVCPlugTypes plugType,
										UInt32 plugNum,
										UInt32 *pSignalFormat);

/*!
    @function connectTargetPlugs
    @abstract Establishes an internal AVC plug connection between subunit/unit plugs.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param sourceSubunitTypeAndID The subunit type and ID for the source plug
    @param sourcePlugType The source plug type.
    @param pSourcePlugNum A pointer to the source plug num. Will return the actual source plug num here.
    @param destSubunitTypeAndID The subunit type and ID for the destination plug.
    @param destPlugType The dest plug type.
    @param pDestPlugNum A pointer to the dest plug num. Will return the actual dest plug num here.
    @param lockConnection A flag to specify if this connection should be locked.
    @param permConnection A flag to specify if this connection is permanent.
*/
	IOReturn (*connectTargetPlugs)(void *self,
								UInt32 sourceSubunitTypeAndID,
								IOFWAVCPlugTypes sourcePlugType,
								UInt32 *pSourcePlugNum,
								UInt32 destSubunitTypeAndID,
								IOFWAVCPlugTypes destPlugType,
								UInt32 *pDestPlugNum,
								bool lockConnection,
								bool permConnection);

/*!
    @function disconnectTargetPlugs
    @abstract Breaks an internal AVC plug connection between subunit/unit plugs.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param sourceSubunitTypeAndID The subunit type and ID for the source plug.
    @param sourcePlugType The source plug type.
    @param sourcePlugNum The source plug num.
    @param destSubunitTypeAndID The subunit type and ID for the destination plug.
    @param destPlugType The dest plug type.
    @param destPlugNum The dest plug num.
*/
	IOReturn (*disconnectTargetPlugs)(void *self,
								   UInt32 sourceSubunitTypeAndID,
								   IOFWAVCPlugTypes sourcePlugType,
								   UInt32 sourcePlugNum,
								   UInt32 destSubunitTypeAndID,
								   IOFWAVCPlugTypes destPlugType,
								   UInt32 destPlugNum);

/*!
    @function getTargetPlugConnection
    @abstract Gets the connection details for a specific plug.
    @param self Pointer to IOFireWireAVCLibProtocolInterface.
    @param subunitTypeAndID The subunit type and ID of the plug.
    @param plugType The plug type.
    @param plugNum The plug number.
    @param pConnectedSubunitTypeAndID The subunit type and ID of the connected plug.
    @param pConnectedPlugType The type of the connected plug.
    @param pConnectedPlugNum The number of the connected plug.
    @param pLockConnection A pointer for returning the lock status of the connection.
    @param pPermConnection A pointer for returning the perm status of the connection.
*/
	IOReturn (*getTargetPlugConnection)(void *self,
									 UInt32 subunitTypeAndID,
									 IOFWAVCPlugTypes plugType,
									 UInt32 plugNum,
									 UInt32 *pConnectedSubunitTypeAndID,
									 IOFWAVCPlugTypes *pConnectedPlugType,
									 UInt32 *pConnectedPlugNum,
									 bool *pLockConnection,
									 bool *pPermConnection);
} IOFireWireAVCLibProtocolInterface;


typedef void (*IOFireWireAVCPortStateHandler)( void * refcon, UInt32 state );
typedef void (*IOFireWireAVCFrameStatusHandler)( void * refcon, UInt32 mode, UInt32 count );



/*!
    @class IOFireWireAVCLibConsumerInterface
    @abstract   Interface for an asynchronous connection consumer.
    @discussion Used to receive data from an asynchronous connection producer.
*/

typedef struct
{

	IUNKNOWN_C_GUTS;

	UInt16	version;						
    UInt16	revision;
   
    void (*setSubunit)( void * self, UInt8 subunit );
    void (*setRemotePlug)( void * self, UInt8 plugNumber );

    IOReturn (*connectToRemotePlug)( void * self );
    IOReturn (*disconnectFromRemotePlug)( void * self );

    void (*setFrameStatusHandler)( void * self, void * refcon, IOFireWireAVCFrameStatusHandler handler );
    void (*frameProcessed)( void * self, UInt32 mode );
 
    void (*setMaxPayloadSize)( void * self, UInt32 size );

    IOReturn (*setSegmentSize)( void * self, UInt32 size );
    UInt32 (*getSegmentSize)( void * self );
    char * (*getSegmentBuffer)( void * self );
 
    void (*setPortStateHandler)( void * self, void * refcon, IOFireWireAVCPortStateHandler handler );

       
    void (*setPortFlags)( void * self, UInt32 flags );
    void (*clearPortFlags)( void * self, UInt32 flags );
    UInt32 (*getPortFlags)( void * self );

} IOFireWireAVCLibConsumerInterface;

#endif
