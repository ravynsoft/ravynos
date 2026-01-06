/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
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

#ifndef _IOKIT_IOFIREWIRESBP2LIB_H_
#define _IOKIT_IOFIREWIRESBP2LIB_H_

#include <IOKit/firewire/IOFireWireFamilyCommon.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>

/* A45B8156-B51B-11D4-AB4B-000A277E7234 */
#define kIOFireWireSBP2LibTypeID CFUUIDGetConstantUUIDWithBytes(NULL,		\
    0xA4, 0x5B, 0x81, 0x56, 0xB5, 0x1B, 0x11, 0xD4,							\
    0xAB, 0x4B, 0x00, 0x0A, 0x27, 0x7E, 0x72, 0x34 )

/* AE3A2684-B51B-11D4-B516-000A277E7234 */
#define kIOFireWireSBP2LibFactoryID CFUUIDGetConstantUUIDWithBytes(NULL, 	\
    0xAE, 0x3A, 0x26, 0x84, 0xB5, 0x1B, 0x11, 0xD4,							\
    0xB5, 0x16, 0x00, 0x0A, 0x27, 0x7E, 0x72, 0x34 )

/* B63446A8-B51B-11D4-AAB0-000A277E7234 */
#define kIOFireWireSBP2LibLUNInterfaceID CFUUIDGetConstantUUIDWithBytes(NULL,	\
    0xB6, 0x34, 0x46, 0xA8, 0xB5, 0x1B, 0x11, 0xD4,						    	\
    0xAA, 0xB0, 0x00, 0x0A, 0x27, 0x7E, 0x72, 0x34 )

/* BBE32C26-BAD4-11D4-A580-000A277E7234 */
#define kIOFireWireSBP2LibLoginInterfaceID CFUUIDGetConstantUUIDWithBytes(NULL,	\
    0xBB, 0xE3, 0x2C, 0x26, 0xBA, 0xD4, 0x11, 0xD4, 							\
	0xA5, 0x80, 0x00, 0x0A, 0x27, 0x7E, 0x72, 0x34 )

/* 0D818E2E-BC55-11D4-9B72-000A277E7234 */
#define kIOFireWireSBP2LibORBInterfaceID CFUUIDGetConstantUUIDWithBytes(NULL,	\
    0x0D, 0x81, 0x8E, 0x2E, 0xBC, 0x55, 0x11, 0xD4, 							\
	0x9B, 0x72, 0x00, 0x0A, 0x27, 0x7E, 0x72, 0x34 )

/* ECD3E338-BDBC-11D4-A592-000A277E7234 */
#define kIOFireWireSBP2LibMgmtORBInterfaceID CFUUIDGetConstantUUIDWithBytes(NULL,	\
    0xEC, 0xD3, 0xE3, 0x38, 0xBD, 0xBC, 0x11, 0xD4,									\
	0xA5, 0x92, 0x00, 0x0A, 0x27, 0x7E, 0x72, 0x34 )

//////////////////////

#define kIOMessageFWSBP2ReconnectComplete		((UInt32)iokit_fw_err(0x3E8))
#define kIOMessageFWSBP2ReconnectFailed       	((UInt32)iokit_fw_err(0x3E9))

/*!
 * Direction of transfer, with respect to the described memory.
 */
enum IODirection
{
    kIODirectionNone  = 0,
    kIODirectionIn    = 1,	// User land 'read'
    kIODirectionOut   = 2,	// User land 'write'
    kIODirectionOutIn = 3
};

/*! Virtual address range for SBP2. */
typedef struct 
{
	void * address;
	UInt32 length;
} FWSBP2VirtualRange;

//////////////////////

/*! @enum Login Option Flags
    @discussion
	Passed to the setLoginFlags member function.
 */
enum
{
	kFWSBP2DontSynchronizeMgmtAgent = (1 << 0),
    kFWSBP2ExclusiveLogin 			= (1 << 5)
};

/*! @enum ORB Option Flags
    @discussion
	Passed to the setCommandFlags member function.
 */
enum
{
    kFWSBP2CommandCompleteNotify			= (1 << 0),
    kFWSBP2CommandTransferDataFromTarget	= (1 << 1),
    kFWSBP2CommandImmediate					= (1 << 2),
    
    kFWSBP2CommandNormalORB					= (1 << 5),
    kFWSBP2CommandReservedORB				= (1 << 6),
    kFWSBP2CommandVendorORB					= (1 << 7),
    kFWSBP2CommandDummyORB					= (1 << 8),
    kFWSBP2CommandCheckGeneration			= (1 << 9),
	
	kFWSBP2CommandFixedSize					= (1 << 10),
	kFWSBP2CommandVirtualORBs				= (1 << 11)     // handy for debugging
};

/*! @enum SBP2 setCommandFunction values
    @discussion
	Passed to the setCommandFunction member function.
 */
enum
{
    kFWSBP2QueryLogins			= 1,
    kFWSBP2AbortTask			= 0xb,
    kFWSBP2AbortTaskSet			= 0xc,
    kFWSBP2LogicalUnitReset		= 0xe,
    kFWSBP2TargetReset			= 0xf
};

/*! @enum SBP2 Notification Events
    @discussion
	Passed to the setStatusNotifyProc member function.
 */
enum
{
    kFWSBP2NormalCommandStatus	= 6,
    kFWSBP2NormalCommandTimeout	= 7,
    kFWSBP2UnsolicitedStatus	= 8,
    kFWSBP2NormalCommandReset	= 9
};

/*! 
    @typedef FWSBP2LoginResponse
    @param length Length of login response.
    @param loginID Unique id representing this login. 
    @param commandBlockAgentAddressHi High 32 bits of command block agent address.
    @param commandBlockAgentAddressLo Low 32 bits of command block agent address.
    @param reserved Reserved.
    @param reconnectHold Reconnect timeout encoded as 2^reconnectHold seconds.
*/

typedef struct
{
    UInt16		length;
    UInt16		loginID;
    UInt32		commandBlockAgentAddressHi;
    UInt32		commandBlockAgentAddressLo;
    UInt16		reserved;
    UInt16		reconnectHold;
} FWSBP2LoginResponse;

/*! 
    @typedef FWSBP2StatusBlock
    @param details Src, Resp, D, Len fields of status block format
    @param sbpStatus SBP2 specific status
    @param orbOffsetHi High 32 bits of address of orb status is for.
    @param orbOffsetLo Low 32 bits of address of orb status is for.
    @param status Up to 48 bytes of additional data. Length is determined by len field.
*/

typedef struct
{
    UInt8		details;
    UInt8		sbpStatus;
    UInt16		orbOffsetHi;
    UInt32		orbOffsetLo;
    UInt32		status[6];
} FWSBP2StatusBlock;

// struct sent to login complete handler

/*! 
    @typedef FWSBP2LoginCompleteParams
    @param refCon refCon set on login object.
    @param generation FireWire generation value.
    @param status Status of login attempt.
    @param loginResponse Pointer to login response struct.
    @param statusBlock Pointer to status block buffer.
    @param statusBlockLength Length of entire status block.
*/

typedef struct
{
    void * 								refCon;			// refCon from login object
    UInt32								generation;		// generation this login was attempted in 
    IOReturn							status;			// status of login attempt
    FWSBP2LoginResponse *				loginResponse;		// pointer to loginResponse buffer
    FWSBP2StatusBlock *					statusBlock;		// pointer to statusBlock buffer
    UInt32								statusBlockLength;	// size of statusBlock buffer
} FWSBP2LoginCompleteParams;

// struct sent to logout complete handler

/*! 
    @typedef FWSBP2LogoutCompleteParams
    @param refCon refCon set on login object.
    @param generation FireWire generation value.
    @param status Status of login attempt.
    @param statusBlock Pointer to status block buffer.
    @param statusBlockLength Length of entire status block.
*/

typedef struct
{
    void * 								refCon;			// refCon from login object
    UInt32								generation;		// generation this login was attempted in
    IOReturn							status;				// status of login attempt
    FWSBP2StatusBlock *					statusBlock;		// pointer to statusBlock buffer
    UInt32								statusBlockLength;	// size of statusBlock buffer
} FWSBP2LogoutCompleteParams;

// struct sent with reconnect notification

/*! 
    @typedef FWSBP2ReconnectParams
    @param refCon refCon set on LUN object.
    @param generation FireWire generation value.
    @param status Status of reconnect attempt.
    @param reconnectStatusBlock Pointer to status block buffer.
    @param reconnectStatusBlockLength Length of entire status block.
*/

typedef struct
{
    void *				 	refCon;			// refCon from lun object
    UInt32					generation;		// generation this login was attempted in

    IOReturn				status;			// status of reconnect attempt

    FWSBP2StatusBlock *		reconnectStatusBlock;		// pointer to statusBlock buffer
    UInt32					reconnectStatusBlockLength;	// size of statusBlock buffer
} FWSBP2ReconnectParams;

/*! 
    @typedef FWSBP2NotifyParams
    @param refCon refCon set on Login object for unsolicited status or refCon set ORB for normal status.
    @param notificationEvent Type of event we are being notified of.
    @param message buffer containing message.
    @param length length of message field.
    @param generation FireWire generation value.
*/

typedef struct
{
    void *			refCon; 				// refCon from ORB object
    UInt32 			notificationEvent;
    const void * 	message;
    UInt32			length;
    UInt32			generation;
} FWSBP2NotifyParams;

//////////////////////

typedef void (*IOFWSBP2MessageCallback)( void * refCon, UInt32 type, void * arg );

/*! 
    @typedef IOFWSBP2LoginCallback
    @param refCon Reference constant supplied when the notification was registered.
    @param params Structure containing additional information about the status of the login. 
*/

typedef void (*IOFWSBP2LoginCallback)( void * refCon, FWSBP2LoginCompleteParams * params );

/*! 
    @typedef IOFWSBP2LogoutCallback
    @param refCon Reference constant supplied when the notification was registered.
    @param params Structure containing additional information about the status of the logout. 
*/

typedef void (*IOFWSBP2LogoutCallback)( void * refCon, FWSBP2LogoutCompleteParams * params );

/*! 
    @typedef IOFWSBP2ORBAppendCallback
    @param refCon Reference constant supplied when the notification was registered.
    @param status Indicates success or failure of operation. 
    @param orb refCon set on management orb.
*/

typedef void (*IOFWSBP2ORBAppendCallback)( void * refCon, IOReturn status, void * orb );

/*! 
    @typedef IOFWSBP2ORBCompleteCallback
    @param refCon Reference constant supplied when the notification was registered.
    @param status Indicates success or failure of operation. 
    @param orb refCon set on management orb.
*/

typedef void (*IOFWSBP2ORBCompleteCallback)( void * refCon, IOReturn status, void * orb );

/*! 
    @typedef IOFWSBP2NotifyCallback
    @param refCon Reference constant supplied when the notification was registered.
    @param params FWSBP2NotifyParams containing notification information.
*/

typedef void (*IOFWSBP2NotifyCallback)(void * refCon, FWSBP2NotifyParams * params);

/*! 
    @typedef IOFWSBP2StatusCallback
    @param refCon Reference constant supplied when the notification was registered.
    @param status Indicates success or failure of operation. 
*/

typedef void (*IOFWSBP2StatusCallback)(void * refCon, IOReturn status);

/*! 
    @typedef IOFWSBP2FetchAgentWriteCallback
    @param refCon Reference constant supplied when the notification was registered.
    @param status Indicates success or failure of operation. 
    @param orbRefCon refCon from last orb in chain.
*/

typedef void (*IOFWSBP2FetchAgentWriteCallback)(void * refCon, IOReturn status, void * orbRefCon );

//////////////////////


/*!
    @class IOFireWireSBP2LibLUNInterface
    @abstract Initial interface disovered for all drivers. 
    @discussion The IOFireWireSBP2LibLUNInterface is the initial interface discovered by most drivers. It supplies the methods that control the operation of the LUN as a whole.  Methods that control the behavior and execution of an SBP2 login session are supplied in a separate IOFireWireSBP2LibLoginInterface object. The LUN can be used to create one of these login objects.
    The LUN can also create IOFireWireSBP2LibManagementORBInterfaces for configuring and appending non-login related management functions.  Login related management functions (ie. Login, Logout, Reconnect) are supplied by the IOFireWireSBP2LibLoginInterface.
    Finally the LUN can supply a reference to the IOFireWireUnit.  This can be useful if a driver wishes to access the standard FireWire APIs.  
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
        open the device on the bus may disappear, but the in-kernel object representing it will stay
        instantiated and can begin communicating with the device again if it ever reappears. 
        @param self Pointer to IOFireWireSBP2LibLUNInterface.
        @result Returns kIOReturnSuccess on success.
    */
    
	IOReturn (*open)( void * self );
    
    /*!
		@function openWithSessionRef
		@abstract Opens a connection to a device that is already open.
		@discussion Sometimes it is desirable to open multiple user clients on a device.  In the case 
        of FireWire sometimes we wish to have both the FireWire User Client and the SBP2 User Client 
        open at the same time. 
		<p>The technique to arbitrate this is as follows :</p>
		<p>First open normally the device furthest from the root in the IORegistry.</p>
		<p>Second, get its sessionRef with the getSessionRef call.</p>
		<p>Third open the device further up the chain by calling this method and passing the sessionRef returned from the call in step 2.</p>
		@param sessionRef SessionRef returned from getSessionRef call. 
        @param self Pointer to IOFireWireSBP2LibLUNInterface.
        @result Returns kIOReturnSuccess on success.
    */
    
	IOReturn (*openWithSessionRef)( void * self, IOFireWireSessionRef sessionRef );

    /*!
		@function getSessionRef
		@abstract Returns the session reference to an already open device.
		@discussion Sometimes it is desirable to open multiple user clients on a device.  In the case 
        of FireWire sometimes we wish to have both the FireWire User Client and the SBP2 User Client 
        open at the same time.  
		<p>The technique to arbitrate this is as follows:</p> 
		<p>First open normally the device furthest from the root in the IORegistry.</p>  
		<p>Second, get its sessionRef with a call to this method.</p>
		<p>Third open the device further up the chain by calling openWithSessionRef and passing the sessionRef returned from this call.</p>
        @param self Pointer to IOFireWireSBP2LibLUNInterface.
        @result Returns a sessionRef on success.
    */

	IOFireWireSessionRef (*getSessionRef)(void * self);

    /*!
		@function close
		@abstract Opens a connection to a device that is not already open.
		@discussion Closes an exclusive access to the device.  When a device is closed it may be 
        unloaded by the kernel.  If it is unloaded and then later reappears it will be represented 
        by a different object.  You won't be able to use this user client on the new object.  The 
        new object will have to be looked up in the IORegistry and a new user client will have to 
        be opened on it. 
        @param self Pointer to IOFireWireSBP2LibLUNInterface.
    */

	void (*close)( void * self );

    /*!
		@function addCallbackDispatcherToRunLoop
		@abstract Adds a dispatcher for kernel callbacks to the specified runloop.
		@discussion The user space portions of the SBP2 api communicate with the in-kernel services by 
        messaging the kernel.  Similarly, the kernel messages the user space services in response.  
        These responses need to be picked up by a piece of code.  This call adds that code to the specified
        runloop.  Most drivers will call this method on the runloop that was created when your task was 
        created.  To avoid deadlock you must avoid sleeping (or spin waiting) the runloop to wait for 
        SBP2 response.  If you do this the dispatcher will never get to run and you will wait forever.
        @param self Pointer to IOFireWireSBP2LibLUNInterface.
        @param cfRunLoopRef Reference to a runloop
        @result Returns kIOReturnSuccess on success.
    */
    	
	IOReturn (*addCallbackDispatcherToRunLoop)( void *self, CFRunLoopRef cfRunLoopRef );
	
    /*!
		@function removeCallbackDispatcherFromRunLoop
		@abstract Removes a dispatcher for kernel callbacks from the specified runloop.
		@discussion Undoes the work of addCallbackDispatcherToRunLoop.
        @param self Pointer to IOFireWireSBP2LibLUNInterface.
    */
    
    void (*removeCallbackDispatcherFromRunLoop)( void * self );
    
    /*!
		@function setMessageCallback
		@abstract Set callback for user space message routine.
		@discussion In FireWire & SBP2 bus status messages are delivered via IOKit's message routine.  
        This routine is emulated in user space for SBP2 & FireWire messages via this callback.  You should
        register here for bus reset, and reconnect messages.
        @param self Pointer to IOFireWireSBP2LibLUNInterface.
        @param refCon RefCon to be returned as first argument of completion routine
        @param callback Address of completion routine.
    */
    
	void (*setMessageCallback)( void *self, void * refCon, IOFWSBP2MessageCallback callback);

    /*!
		@function setRefCon
		@abstract Sets the ORB refCon.
		@discussion Sets a user defined value on the ORB that can be retrieved later with the 
        method getRefCon.
        @param self Pointer to IOFireWireSBP2LibLUNInterface.
        @param refCon a user defined value.
    */
    
	void (*setRefCon)( void * self, void * refCon );
    
    /*!
		@function getRefCon
		@abstract Returns the refCon set with setRefCon.
		@discussion Returns the user defined value previously stored in the ORB with setRefCon.
        @param self Pointer to IOFireWireSBP2LibLUNInterface.
        @result Returns the previously stored user defined value.
	*/

	void * (*getRefCon)( void * self);
    
    /*!
		@function createLogin
		@abstract Creates a new IOFireWireSBP2LibLoginInterface object.
		@discussion	Creates a new IOFireWireSBP2LibLoginInterface object for the LUN.  Login 
        objects supply most of the SBP2 APIs related to login maintenance and Normal 
        Command ORB execution.
        @param self Pointer to IOFireWireSBP2LibLUNInterface.
        @param iid UUID for desired type of IOFireWireSBP2LibLoginInterface.
        @result Returns a pointer to a new IOFireWireSBP2LibLoginInterface.
	*/

	IUnknownVTbl** (*createLogin)( void * self, REFIID iid );
    
    /*!
		@function createMgmtORB
		@abstract Creates a new IOFireWireSBP2LibMgmntORBInterface object.
		@discussion	Creates a new IOFireWireSBP2LibMgmtORBInterface object.  Management objects let you 
        execute commands like QueryLogins, LogicalUnitReset, and AbortTask.  These commands are 
        configured after they are created here.  When they are done executing (after a call to submit) 
        the supplied completion routine will be called with the supplied refcon.  Usually this refCon
        is the "this" pointer of completion method's object.
        @param self Pointer to IOFireWireSBP2LibLUNInterface.
        @param iid UUID for desired type of IOFireWireSBP2LibMgmtORBInterface.
        @result Returns a pointer to a new IOFireWireSBP2Login.
	*/

	IUnknownVTbl** (*createMgmtORB)( void * self, REFIID iid );

} IOFireWireSBP2LibLUNInterface;



/*!
    @class IOFireWireSBP2LibORBInterface
    @abstract Represents an SBP2 normal command ORB.  Supplies the APIs for configuring normal
    command ORBs.  This includes setting the command block and writing the page tables for I/O.
    The ORBs are executed using the submitORB method in IOFireWireSBP2LibLoginInterface.
*/
typedef struct
{

	IUNKNOWN_C_GUTS;

	UInt16	version;						
    UInt16	revision;

    /*!
		@function setRefCon
		@abstract Sets the ORB refCon.
		@discussion Sets a user defined value on the ORB that can be retrieved later with the 
        method getRefCon.
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @param refCon a user defined value.
    */
    
	void (*setRefCon)( void * self, void * refCon );

    /*!
		@function getRefCon
		@abstract Returns the refCon set with setRefCon.
		@discussion Returns the user defined value previously stored in the ORB with setRefCon.
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @result Returns the previously stored user defined value.
	*/

	void * (*getRefCon)( void * self );	
    
    /*! 
        @function setCommandFlags
        @abstract Sets configuration flags for the ORB.
        @discussion Sets the configuration flags for the ORB.  These can be any of the following:
		<p>kFWSBP2CommandCompleteNotify - Set the notify bit as specified in SBP2 standard. Set to receive completion/timeout notification on this ORB.  You almost always want to set this.</p>
		<p>kFWSBP2CommandTransferDataFromTarget - Transfer direction as specified in SBP2 standard.  Set if data is to be written by the device into the host's memory.</p>
		<p>kFWSBP2CommandImmediate - Immediate Append.  ORB address will be written to fetch agent and not chained.  It is only legal to have one immediate ORB in progress at a time.</p>
		<p>kFWSBP2CommandNormalORB - ORB format 0 - Format specified by SBP2 standard.  Set this for most ORBs.</p>
		<p>kFWSBP2CommandReservedORB - ORB format 1 - Format reserved by SBP2 standard for future standardization.</p>
		<p>kFWSBP2CommandVendorORB - ORB format 2 - Format specified by SBP2 standard for vendor dependent ORBs.</p>
		<p>kFWSBP2CommandDummyORB - ORB format 3 - Format specified by SBP2 standard for dummy ORBs.</p>
		<p>kFWSBP2CommandCheckGeneration - If set upon submitORB, the ORB will only be appended if generation set with setCommandGeneration() matches the current generation.  Pretty much all SBP2 drivers need sophisticated logic to track login state, so this is generally not used. </p>
		<p>kFWSBP2CommandFixedSize - Do not allocate more memory for page table if needed.  If there is not enough space in the currently allocated page table, the setCommandBuffers call will fail.  This is important to set if your device is the backing store, as we don't want to cause memory allocations on the paging path. </p>
		<p>kFWSBP2CommandVirtualORBs - Normally ORBs are backed by physical address spaces.  Setting this flag makes this ORB backed by a pseudo address space.  This can make ORBs easier to see in a bus trace.  Virtual ORBs will have an address in the form of ffcX.XXXX.0000.0000.  Pseudo address space backed ORBs are slower, so you won't want to set for deployment builds.</p>
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @param flags The flags to be set.
    */
    
	void (*setCommandFlags)( void * self, UInt32 flags );

    /*! 
        @function setMaxORBPayloadSize
        @abstract Sets max payload size for the ORB.
        @discussion This sets the maximum payload size for this ORB only.  This size is clipped by 
        the global max payload size set in the login object.
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @param size The maximum payload size in bytes.
    */
    
	void (*setMaxORBPayloadSize)( void * self, UInt32 size );

    /*! 
        @function setCommandTimeout
        @abstract Sets the timeout of the ORB.
        @discussion This sets the timeout for the ORB in milliseconds. Note that ORBs without timeouts 
        can be "lost."  You will obviously not recieve timeout notification for timeouts of zero.  But 
        perhaps less obviously you will not recieve orb reset notification, which is really a sort of
        accelerated timeout notification for bus reset situations.  
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @param timeout The timeout duration in milliseconds.
    */
    
	void (*setCommandTimeout)( void * self, UInt32 timeout );

    /*! 
        @function setCommandGeneration
        @abstract Sets the command generation.
        @discussion This sets the bus generation this ORB should be appended in.  It is only meaningful 
        when combined with the kFWSBP2CommandCheckGeneration flags above.
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @param generation The bus generation for command execution.
    */

	void (*setCommandGeneration)( void * self, UInt32 generation );

    /*! 
        @function setCommandBuffersAsRanges
        @abstract Creates a page table from a list of ranges.
        @discussion Creates a page table with the given parameters. Any addresses mapped by this method 
        must remain valid until setCommandBuffers is called again or releaseCommandBuffers is called.  
        The SBP2 services do not release references to the command buffers just because the command 
        has completed.
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @param ranges An array of ranges representing the data to be transfered.
        @param withCount The number of ranges in the ranges array.
        @param withDirection An IODirection indicating the direction of data transfer.
        @param offset Offset in bytes into data to begin writing table at.
        @param length Number of bytes of data to map from offset.
        @result Returns KIOReturnSuccess if the page table was written successfully. 
    */

    IOReturn (*setCommandBuffersAsRanges)( void * self, FWSBP2VirtualRange * ranges, UInt32 withCount,
											UInt32 withDirection, UInt32 offset, 
											UInt32 length );

    /*! 
        @function releaseCommandBuffers
        @abstract Releases SBP2's reference to the command buffers.
        @discussion When you create a page table with one of the variants of setCommandBuffers.  
        SBP2 holds on to a reference to the buffers until this method is called.  This means that 
        if a command completed and you released the buffers 
        without calling this method you could leave FW in an inconsistent state.  
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @result Returns KIOReturnSuccess if the page table was cleared successfully. 
    */
    
    IOReturn (*releaseCommandBuffers)( void * self );
    
    /*! 
        @function setCommandBlock
        @abstract Sets the command block portion of the ORB.
        @discussion Copys the data provided in the buffer to the command block portion of the ORB.
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @param buffer Pointer to buffer to copy command block from.
        @param length Number of bytes of data to copy.
        @result Returns KIOReturnSuccess if the command block was updated successfully. 
    */

    IOReturn (*setCommandBlock)( void * self, void * buffer, UInt32 length );
    
    /*! 
        @function LSIWorkaroundSetCommandBuffersAsRanges
        @abstract Creates a page table with the LSI workaround from a list of ranges.
        @discussion Creates an LSI workaround page table with the given parameters. Any addresses 
        mapped by this method routine must remain valid until setCommandBuffers is called again
        or releaseCommandBuffers is called. The SBP2 services do not release references to the 
        command buffers just because the command has completed.
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @param ranges An array of ranges representing the data to be transfered.
        @param withCount The number of ranges in the ranges array.
        @param withDirection An IODirection indicating the direction of data transfer.
        @param offset Offset in bytes into data to begin writing table at.
        @param length Number of bytes of data to map from offset.
        @result Returns KIOReturnSuccess if the page table was written successfully. 
    */
    
    IOReturn (*LSIWorkaroundSetCommandBuffersAsRanges)( void * self, FWSBP2VirtualRange * ranges, UInt32 withCount,
											UInt32 withDirection, UInt32 offset, 
											UInt32 length );

    /*!
		@function LSIWorkaroundSyncBuffersForOutput
		@abstract Synchronize the buffers for output.
		@discussion	Since double buffering may be invovled in the workaround.  The driver needs to 
        indicate when these buffers should be syncronized with the original descriptor.  For data 
        that will be output LSIWorkaroundSyncBuffersForOutput should be called before submiting the ORB.
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @result Returns kIOReturnSuccess if sync was successful.
	*/
    
    IOReturn (*LSIWorkaroundSyncBuffersForOutput)( void * self );

    /*!
		@function LSIWorkaroundSyncBuffersForInput
		@abstract Synchronize the buffers for input.
		@discussion	Since double buffering may be invovled in the workaround.  The driver needs to 
        indicate when these buffers should be syncronized with the original descriptor.  For data 
        that will be input LSIWorkaroundSyncBuffersForInput should be called after receiving completion status 
        for the ORB.
        @param self Pointer to IOFireWireSBP2LibORBInterface.
        @result Returns kIOReturnSuccess if sync was successful.
	*/

    IOReturn (*LSIWorkaroundSyncBuffersForInput)( void * self );
		
} IOFireWireSBP2LibORBInterface;

 /*!
    @class IOFireWireSBP2LibLoginInterface
    @abstract Supplies the login maintenance and Normal Command ORB execution portions of the API.
    @discussion Supplies APIs for login maintenance and command execution.  Drivers can use this 
    object to create IOFireWireSBP2LibORBInterface objects and execute them.  Solicited and unsolicited status 
    callback routines can be registered and the SBP2 services will notify the driver when the 
    appropriate status arrives.
    This class also handles login maintenance.  Supplies APIs for logging in and logging out and 
    attempts to reconnect to the LUN after bus resets.  The base FireWire services deliver bus 
    reset notification via the IOKit message routine.  The SBP2 services build on this behavior 
    and deliver reconnectFailed and reconnectComplete through the message routine as well.
*/
typedef struct
 {

	IUNKNOWN_C_GUTS;

	UInt16	version;						
    UInt16	revision;

    /*!
		@function submitLogin
		@abstract Attempts to login to the LUN.		
        @discussion This call begins the login process.  The login object should be configured prior 
        to this call. If kIOReturnSuccess is returned from this call then the loginCompletion routine 
        will be called when the login completes (successfully or unsuccesfully). 
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @result Returns kIOReturnSuccess login has successlly begun.
	*/

	IOReturn (*submitLogin)( void * self );

    /*!
		@function submitLogout
		@abstract Attempts to logout of the LUN.		
        @discussion This call begins the logout process.  If kIOReturnSuccess is returned from this call then
        the logoutCompletion routine will be called when the logout completes (successfully or unsuccesfully). 
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @result Returns kIOReturnSuccess if logout has successfully begun.
	*/

	IOReturn (*submitLogout)( void * self );

    /*!
		@function setLoginFlags
		@abstract Sets login configuration flags.
		@discussion Configures the login behavior according to the provided flags.  Currently two 
        flags are defined for this API.  kFWSBP2ExclusiveLogin sets the exclusive login bit in the 
        login ORB. kFWSBP2DontSynchronizeMgmtAgent allows simultaneous logins or reconnects to LUNs
        with a common management agent (ie LUNs in the same unit directory).
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param flags the login configuration flags.
	*/
    
	void (*setLoginFlags)( void * self, UInt32 flags );

    /*!
		@function setLoginCallback
		@abstract Sets the callback to be called when a login attempt is complete.
		@discussion The supplied callback is called when a login attempt has completed. "status" in the
        callback's params should be checked to determine the success or failure of the login attempt.
        The "refCon" field in the params will return the refcon set with setRefCon.   
        If "statusBlock" is non-null then login status was written and it has been supplied here.  If 
        the login attempt was successful then the login response will be supplied in the "loginResponse" 
        buffer.  Note: all buffers supplied to callbacks are only valid for the duration of the callback.  
        Also, you are not to modify the contents of any supplied buffer.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param refCon refCon passed to callback.	
        @param callback address of callback method of type FWSBP2LoginCallback.
	*/

	void (*setLoginCallback)( void * self, void * refCon, IOFWSBP2LoginCallback callback );

    /*!
		@function setLogoutCallback
		@abstract Sets the callback to be called when a logout attempt is complete.
		@discussion The supplied callback is called when a logout attempt has completed. "status" in the
        callback's params should be checked to determine the success or failure of the logout attempt.
        The "refCon" field in the params will return the refcon set with setRefCon.  
        If "statusBlock" is non-null then logout status was written and it has been supplied here. 
        Note: all buffers supplied to callbacks are only valid for the duration of the callback.  
        Also, you are not to modify the contents of any supplied buffer.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param refCon refCon passed to callback.	
        @param callback address of callback method of type FWSBP2LogoutCallback.
	*/

	void (*setLogoutCallback)( void * self, void * refCon, IOFWSBP2LogoutCallback callback );

    /*!
		@function setRefCon
		@abstract Sets the login refCon.
		@discussion Sets a user defined value on the login that can be retrieved later with the 
        method getRefCon.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param refCon a user defined value.
    */

	void (*setRefCon)( void * self, void * refCon );

    /*!
		@function getRefCon
		@abstract Returns the refCon set with setRefCon.
		@discussion Returns the user defined value previously stored in the login with setRefCon.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @result Returns the previously stored user defined value.
	*/

	void * (*getRefCon)( void * self);

    /*!
		@function getMaxCommandBlockSize
		@abstract Returns the maximum command block size.
		@discussion The device publishes an upper limit on the size of command block that it can 
        accept.  That value can be accessed via this method.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @result Returns a UInt32 containing the maximum command block size.
	*/

	UInt32 (*getMaxCommandBlockSize)( void * self);

    /*!
		@function getLoginID
		@abstract Returns the current login ID.
		@discussion When we successfully login to a device.  The device gives us a unique login id.  
        This is used internally for reconnecting to the device after bus resets and for certain other 
        management ORBs.  Most drivers are probably not interested in this value.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @result Returns a UInt32 containing the current login ID.
	*/
    
	UInt32 (*getLoginID)( void * self);

    /*!
		@function setMaxPayloadSize
		@abstract Sets the maximum data transfer length for a normal command ORB.
		@discussion Sets the maximum data transfer length for a normal command ORB.  This value is 
        the maximum for all ORBs sent to this LUN.  This can be trimmed further on an ORB by ORB basis, 
        by a similar call in the IOFireWireSBP2ORB itself.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param size The desired maximum payload size in bytes.
	*/

	void (*setMaxPayloadSize)( void * self, UInt32 size );

    /*!
		@function setReconnectTime
		@abstract Sets the desired reconnect duration.
		@discussion The target and initiator arbitrate the duration of the reconnect timeout.  Here 
        the initiator specifies its desired timeout time in 2^reconnectTime seconds.  After a 
        successful login the device returns the actual timeout value it wishes to use.  This value 
        may be less than the reconnect timeout that the intiator specified if this is all that the 
        device can support.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param time The desired reconnect timeout encoded as 2^reconnectTime seconds.
	*/

	void (*setReconnectTime)( void * self, UInt32 time );

    /*!
		@function createORB
		@abstract Creates a new IOFireWireSBP2ORB for this login.
		@discussion	Create a new IOFireWireSBP2ORB for this login.  It can be configured 
        with it's accessors and executed with submitORB below.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param iid UUID for the desired type of IOFireWireSBP2LibLoginInterface.
        @result Returns a pointer to the new ORB object.
	*/
    
	IUnknownVTbl** (*createORB)( void * self, REFIID iid );

    /*!
		@function submitORB
		@abstract Submits the given orb
		@discussion	Starts execution of the given ORB.  If the ORB is an immediate ORB it's 
        addresss is written to the fetch agent.  If it is a non immediate orb its address 
        is appended to the last orb of the currently processing chain.  The doorbell is not 
        rung automatically it must be run manually with the ringDoorbell command described below.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param orb The orb to be executed.	
        @result Returns kIOReturnSuccess if the ORB has been started successfully.
	*/

	IOReturn (*submitORB)( void * self, IOFireWireSBP2LibORBInterface ** orb );

    /*!
		@function setUnsolicitedStatusNotify
		@abstract Sets the callback to be called on normal command status.
		@discussion The supplied callback is called when unsolicited status is recieved.
        "notificationEvent" in the callback's params will indicate what happened.  In this 
        case it will be set to kFWSBP2UnsolicitedStatus. If "len" is 
        non-zero then "message" contains the data written to the status block. Note: any buffers 
        returned by callbacks are only valid for the duration of the login and should not have 
        their contents modified. The "refCon" field in the callback's
        params will return the refcon set with setRefCon. 
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param refCon refCon passed to callback.	
        @param callback address of callback method of type FWSBP2NotifyCallback.
	*/

	void (*setUnsolicitedStatusNotify)( void * self, void * refCon, IOFWSBP2NotifyCallback callback );
    
    /*!
		@function setStatusNotify
		@abstract Sets the callback to be called on normal command status.
		@discussion The supplied callback is called when normal command status is recieved, when 
        a normal command times out, or when a normal command is aborted. 
        "notificationEvent" in the callback's params will indicate what happened.
        It will be set to one of the following values: kFWSBP2NormalCommandReset, kFWSBP2NormalCommandStatus, 
        or kFWSBP2NormalCommandTimeout.  If the event type is kFWSBP2NormalCommandTimeout and "len" is 
        non-zero then "message" contains the data written to the status block.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param refCon refCon passed to callback.	
        @param callback Address of callback method of type FWSBP2NotifyCallback.
	*/

	void (*setStatusNotify)( void * self, void * refCon, IOFWSBP2NotifyCallback callback );

    /*!
		@function setFetchAgentResetCallback
		@abstract Sets the callback to be called when a fetch agent reset completes.
		@discussion The fetch agent state machine on the device can be reset by a write to a specific 
        register.  The SBP2 services offer a utility method to reset the fetch agent.  You can register 
        a callback routine here to be notified when this rest write completes.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param refCon refCon passed to callback.	
        @param callback address of callback method of type FWSBP2FetchAgentWriteCallback.
	*/

	void (*setFetchAgentResetCallback)( void * self, void * refCon, IOFWSBP2StatusCallback callback );

    /*!
		@function submitFetchAgentReset
		@abstract Resets the LUN's fetch agent.
		@discussion The fetch agent state machine on the device can be reset by a write to a specific 
        register.  This reset can be intiated by a call to this method.  Notification of the completion 
        of this write can be had by registering a callback with the setFetchAgentResetCallback method.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @result Returns kIOReturnSuccess if the reset started successfully.
	*/

	IOReturn (*submitFetchAgentReset)( void * self );

    /*!
		@function setFetchAgentWriteCallback
		@abstract Sets the callback to be called when the fetch agent write completes.
		@discussion When an immediate orb is executed with submitORB, it's address is written to a 
        specific address on the device.  This address is called the fetch agent.  The device the 
        reads that orb from the Mac's memory and executes it.  With this call you can register to 
        be called back when this write to the fetch agent completes.  The SBP2 services guarantee 
        that the fetch agent write will be complete before status is reported for an ORB, so for 
        most drivers this notification is not required.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param refCon refCon passed to callback.	
        @param callback address of callback method of type FWSBP2FetchAgentWriteCallback.
	*/

	void (*setFetchAgentWriteCallback)( void * self, void * refCon, IOFWSBP2FetchAgentWriteCallback callback );

    /*!
		@function ringDoorbell
		@abstract Rings the doorbell on the LUN.
		@discussion Non-immediate appends to the ORB chain may require the fetch agent state machine 
        to be notified of the new ORB's presence.  This is accomplished by writing to the so called 
        doorbell register.  This method begins one of those writes.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @result Returns kIOReturnSuccess if the ring started successfully.
	*/

	IOReturn (*ringDoorbell)( void * self );

    /*!
		@function enableUnsolicitedStatus
		@abstract Enables unsolicited status.
		@discussion After unsolicited is sent the device will not send any additional unsolicited status 
        until a specific register is written.  This serves as a sort of flow-control for unsolicited status.  
        After unsolicited status is recieved and processed drivers will want to reenable the delivery 
        of unsolicted status by a call to this method.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @result Returns kIOReturnSuccess if the status enable write started successfully.
	*/

	IOReturn (*enableUnsolicitedStatus)( void * self );

    /*!
		@function setBusyTimeoutRegisterValue
		@abstract Sets the value to be written to the BUSY_TIMEOUT register.
		@discussion 1394-1995 defines a register known as the BUSY_TIMEOUT register. This register 
        controls the busy retry behavior of your device.  The initial value for this register is 
        0x00000000.  Which means busied transactions will not be retried.  Since most devices want 
        their transactions retired on busy acks, the SBP2 service automatically updates the 
        BUSY_TIMEOUT register with the value specified here whenever necessary.  Most drivers should 
        set this value to 0x0000000f.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param timeout desired value of the BUSY_TIMEOUT register.
    */
    
	IOReturn (*setBusyTimeoutRegisterValue)( void * self, UInt32 timeout );

    /*!
		@function setPassword
		@abstract Sets the login password.
		@discussion Sets the login password using a buffer and a length.  An alternate version exists 
        that accepts an IOMemoryDescriptor.  If the password length is 8 or less the password is copied 
        directly into the login orb.  If the length is greater than 8 the buffer is referenced by address 
        in the login ORB.  In this case the buffer is not copied and should remain allocated for the 
        duration of the login attempt.
        @param self Pointer to IOFireWireSBP2LibLoginInterface object.
        @param buffer a pointer to the password buffer.
        @param length the length in bytes of the password buffer.
        @result Returns kIOReturnSuccess on success.
	*/
    	
    IOReturn (*setPassword)( void * self, void * buffer, UInt32 length );
	
} IOFireWireSBP2LibLoginInterface;


/*!
	@class IOFireWireSBP2LibMgmtORBInterface
    @abstract Supplies non login related management ORBs.  Management ORBs can be executed independent 
    of a login, if necessary.  Management ORBs are created using the IOFireWireSBP2LibLUNInterface.
*/
typedef struct
{

	IUNKNOWN_C_GUTS;

	UInt16	version;						
    UInt16	revision;

    /*!
		@function submitORB
		@abstract Submits this ORB for execution.
		@discussion Submits this ORB for execution
        @param self Pointer to a IOFireWireSBP2LibMgmtORBInterface.
    */
    
	IOReturn (*submitORB)( void * self );
    
    /*!
		@function setORBCompleteCallback
		@abstract Sets the ORB completion routine.
		@discussion Sets the completion routine to be called when the ORB finishes execution. The refCon 
        set with setRefCon will also be passed as the third argument to the completion handler.
        @param self Pointer to a IOFireWireSBP2LibMgmtORBInterface.
        @param refCon refCon passed as first argument to completion routine
    */
    
	void (*setORBCompleteCallback)( void * self, void * refCon, 
													IOFWSBP2ORBCompleteCallback callback );
    /*!
		@function setRefCon
		@abstract Sets the login refCon.
		@discussion Sets a user defined value on the login that can be retrieved later with the 
        method getRefCon.
        @param self Pointer to a IOFireWireSBP2LibMgmtORBInterface.
        @param refCon a user defined value.
    */
    
	void (*setRefCon)( void * self, void * refCon );

   /*!
		@function getRefCon
		@abstract Returns the current function of the management ORB.
		@discussion Returns the function of the management ORB.  This is the same value that was 
        set with setCommandFunction.
        @param self Pointer to a IOFireWireSBP2LibMgmtORBInterface.
        @result Returns the function of the management ORB.
	*/

	void * (*getRefCon)( void * self);	

    /*!
		@function setCommandFunction
		@abstract Sets the function of the management ORB.
		@discussion Sets the the function of the management ORB.  Legal values are kFWSBP2QueryLogins,
        kFWSBP2AbortTask, kFWSBP2AbortTaskSet,  kFWSBP2LogicalUnitReset, and kFWSBP2TargetReset.
        @param self Pointer to a IOFireWireSBP2LibMgmtORBInterface.
        @param function a value indicating the desired management function.
        @result Returns kIOReturnSuccess if function was a legal function.
	*/
    	
    IOReturn (*setCommandFunction)( void * self, UInt32 function );

    /*!
		@function setManageeORB
		@abstract Sets the command to be managed by the management ORB.
		@discussion All management functions except kFWSBP2QueryLogins require a reference to an ORB of 
        some sort.  kFWSBP2AbortTask requires a reference to the ORB to be aborted.  
        This method allows you to set the Normal Command ORB to be managed.
        @param self Pointer to a IOFireWireSBP2LibMgmtORBInterface.
        @param command a reference to an IOFireWireSBP2Login or an IOFireWireSBP2ORB.
        @result Returns kIOReturnSuccess on a success.
 	*/

	IOReturn (*setManageeORB)( void * self, void * command );

    /*!
		@function setManageeLogin
		@abstract Sets the command to be managed by the management ORB.
		@discussion All management functions except kFWSBP2QueryLogins require a reference to an ORB of 
        some sort.  kFWSBP2AbortTaskSet,  kFWSBP2LogicalUnitReset, and kFWSBP2TargetReset require a 
        reference to the login ORB.    
        This method allows you to set the login ORB to be managed.
        @param self Pointer to a IOFireWireSBP2LibMgmtORBInterface.
        @param command a reference to an IOFireWireSBP2Login or an IOFireWireSBP2ORB.
        @result Returns kIOReturnSuccess on a success.
 	*/
    
	IOReturn (*setManageeLogin)( void * self, void * command );

    /*!
		@function setResponseBuffer
		@abstract Sets the response buffer for the management ORB.
		@discussion Sets the response buffer for the management ORB.  kFWSBP2QueryLogins returns 
        a response to its query and needs to write it somewhere.  This routine allows you to 
        specify the location.        
        @param self Pointer to a IOFireWireSBP2LibMgmtORBInterface.
        @param buf backing store for buffer
        @param len length of buffer.
        @result Returns kIOReturnSuccess on a success.
	*/

	IOReturn (*setResponseBuffer)( void * self, void * buf, UInt32 len );
		
} IOFireWireSBP2LibMgmtORBInterface;

#endif
