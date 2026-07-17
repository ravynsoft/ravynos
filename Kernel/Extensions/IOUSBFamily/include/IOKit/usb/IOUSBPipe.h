/*
 * Copyright © 1998-2012 Apple Inc. All rights reserved.
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
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _IOKIT_IOUSBPIPE_H
#define _IOKIT_IOUSBPIPE_H

#include <IOKit/IOService.h>
#include <IOKit/IOMemoryDescriptor.h>

#include <IOKit/usb/USB.h>
#include <IOKit/usb/IOUSBController.h>
#include <IOKit/usb/IOUSBControllerV2.h>

class IOUSBInterface;

#define	kAppleUSBSSIsocContinuousFrame		0xFFFFFFFFFFFFFFFEull

/*!
    @class IOUSBPipe
    @abstract The object representing an open pipe for a device.
*/
class IOUSBPipe : public OSObject
{
    friend class IOUSBInterface;
    friend class IOUSBDevice;
	
    OSDeclareDefaultStructors(IOUSBPipe)
		
protected:
	
	const IOUSBEndpointDescriptor *	_descriptor;
    IOUSBController::Endpoint		_endpoint;	// tidied up version of descriptor
    IOUSBController *				_controller;
    USBDeviceAddress				_address;
    UInt8							_status;	// was previously used for status.  Now used to detect whether a property exists or not
	
    struct ExpansionData
    {
        IOReturn					_correctStatus;		
		IOUSBDevice *				_device;	// Remember containing device for clearing TTs
		UInt8						_speed;
		IOUSBInterface	*			_interface;
		bool						_crossEndianCompatible;
		UInt32						_locationID;
		UInt8						_uasUsageID;
		UInt8						_usageType;
		UInt8						_syncType;
    };
    ExpansionData * _expansionData;
    
    virtual void free();
    
    IOReturn ClosePipe(void);
	
public:
    
    // The following 4 methods are deprecated (replaced by the new IOUSBPipeV2 class)
    //
    virtual bool InitToEndpoint(const IOUSBEndpointDescriptor *endpoint, UInt8 speed,
                                USBDeviceAddress address, IOUSBController * controller);
	
    static IOUSBPipe *ToEndpoint(const IOUSBEndpointDescriptor *endpoint, UInt8 speed,
                                 USBDeviceAddress address, IOUSBController * controller);
	
    static IOUSBPipe *ToEndpoint(const IOUSBEndpointDescriptor *endpoint,
                                 IOUSBDevice * device, IOUSBController * controller);
	
    static IOUSBPipe *ToEndpoint(const IOUSBEndpointDescriptor *endpoint,
                                 IOUSBDevice * device, IOUSBController * controller, IOUSBInterface *interface);
	
    // Controlling pipe state
    /*!
        @function GetStatus
	 This method does NOT work.  Do not call it. See GetPipeStatus for the correct method.
	 GetStatus will always return 0.
	 */
    virtual UInt8 GetStatus(void);
    /*!
        @function Abort
	 This method causes all outstanding I/O on a pipe to complete with return code kIOReturnAborted. It  clears the halted bit but does NOT clear the
	 toggle bit on the endpoint in the controller.  If you wish to clear the toggle bit, see ClearPipeStall
	 */
    virtual IOReturn Abort(void);
    /*!
        @function Reset
	 This method is identical to ClearPipeStall(false). The use of that API is preferred.
	 */
    virtual IOReturn Reset(void);
    /*!
        @function ClearStall
	 This method is equivalent to ClearPipeStall(false). This method is available before version 1.9.
	 */
    virtual IOReturn ClearStall(void);
	
    //
    // Transferring Data
    //
    
    // deprecated
    virtual IOReturn Read(IOMemoryDescriptor *	buffer,
                          IOUSBCompletion *	completion = 0,
						  IOByteCount *		bytesRead = 0);
	
    // deprecated
    virtual IOReturn Write(IOMemoryDescriptor *	buffer,
                           IOUSBCompletion *	completion = 0);
	
    // Transfer data over Isochronous pipes
    /*!
        @function Read
	 Read from an isochronous endpoint
	 @param buffer place to put the transferred data
	 @param frameStart USB frame number of the frame to start transfer
	 @param numFrames Number of frames to transfer
	 @param frameList Bytes to transfer and result for each frame
	 @param completion describes action to take when buffer has been filled
	 */
    virtual IOReturn Read(IOMemoryDescriptor *	buffer,
                          UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                          IOUSBIsocCompletion *	completion = 0);
    /*!
        @function Write
	 Write to an isochronous endpoint
	 @param buffer place to get the transferred data
	 @param frameStart USB frame number of the frame to start transfer
	 @param numFrames Number of frames to transfer
	 @param frameList Bytes to transfer and result for each frame
	 @param completion describes action to take when buffer has been emptied
	 */
    virtual IOReturn Write(IOMemoryDescriptor *	buffer,
                           UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                           IOUSBIsocCompletion * completion = 0);
	
    // Do a control request over a Control pipe, using a memory descriptor
    /*!
        @function ControlRequest
	 Make a control request
	 There are two versions of this method, one uses a simple void *
	 to point to the data portion of the transfer, the other uses an
	 IOMemoryDescriptor to point to the data.
	 @param request parameter block for the control request
	 @param completion describes action to take when the request has been executed
	 */
    virtual IOReturn ControlRequest(IOUSBDevRequestDesc	*request,
                                    IOUSBCompletion	*completion = 0);
	
    // Do a control request over a Control pipe, using a simple buffer
    virtual IOReturn ControlRequest(IOUSBDevRequest	*request,
                                    IOUSBCompletion	*completion = 0);
	
	/*
	 *  Accessors
	 */
    /*!
        @function GetEndpoint
	 returns a pointer to the Endpoint structure for the pipe.
	 */
    virtual const IOUSBController::Endpoint *	GetEndpoint();
    /*!
        @function GetEndpointDescriptor
	 returns the endpoint descriptor for the pipe.
	 */
    virtual const IOUSBEndpointDescriptor *	GetEndpointDescriptor();
    /*!
        @function GetDirection
	 returns the direction of the pipe:kUSBOut/kUSBIn for a bulk or interrupt pipe,
	 kUSBAnyDirn for a control pipe.
	 */
    virtual UInt8 GetDirection();
    /*!
        @function GetType
	 returns the pipe type: kUSBControl, kUSBBulk or kUSBInterrupt.
	 */
    virtual UInt8 GetType();
    /*!
        @function GetEndpointNumber
	 returns the endpoint number in the device that the pipe is connected to.
	 */
    virtual UInt8 GetEndpointNumber();
    virtual USBDeviceAddress GetAddress();
    virtual UInt16 GetMaxPacketSize();
    virtual UInt8 GetInterval();
	
    // Transfer data over Bulk pipes with timeouts.
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  0);
	
    // deprecated
    virtual IOReturn Read(IOMemoryDescriptor *	buffer,
						  UInt32		noDataTimeout,
						  UInt32		completionTimeout,
                          IOUSBCompletion *	completion = 0,
						  IOByteCount *		bytesRead = 0);
	
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  1);
	
    // deprecated
    virtual IOReturn Write(IOMemoryDescriptor *	buffer,
						   UInt32		noDataTimeout,
						   UInt32		completionTimeout,
                           IOUSBCompletion *	completion = 0);
	
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  2);
    // Do a control request over a Control pipe, using a memory descriptor
    /*!
        @function ControlRequest
	 Make a control request. 
	 There are two versions of this method, one uses a simple void *
	 to point to the data portion of the transfer, the other uses an
	 IOMemoryDescriptor to point to the data.
	 @param request parameter block for the control request
	 @param noDataTimeout Specifies an amount of time (in ms) after which the command will be aborted
	 if no data has been transferred on the bus.
	 @param completionTimeout Specifies an amount of time (in ms) after which the command will be aborted if the entire command has
	 not been completed.
	 @param completion describes action to take when the request has been executed
	 */
    virtual IOReturn ControlRequest(IOUSBDevRequestDesc	*request,
									UInt32		noDataTimeout,
									UInt32		completionTimeout,
                                    IOUSBCompletion	*completion = 0);
	
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  3);
    // Do a control request over a Control pipe, using a simple buffer
    virtual IOReturn ControlRequest(IOUSBDevRequest	*request,
									UInt32		noDataTimeout,
									UInt32		completionTimeout,
                                    IOUSBCompletion	*completion = 0);
	
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  4);
    /*!
        @function Read
	 Read from an interrupt or bulk endpoint
	 @param buffer place to put the transferred data
	 @param noDataTimeout number of milliseconds of no bus activity until transaction times out. Note that if a tranasction times out
	 the driver software may have to resynchronize the data toggle. See ClearPipeStall.
	 @param completionTimeout number of milliseconds from the time the transaction is placed on the bus until it times out
	 @param reqCount requested number of bytes to transfer. must be <= buffer->getLength()
	 @param completion describes action to take when buffer has been filled
	 @param bytesRead returns total bytes read for synchronous reads
	 */
    virtual IOReturn Read(IOMemoryDescriptor *	buffer,
						  UInt32		noDataTimeout,
						  UInt32		completionTimeout,
						  IOByteCount		reqCount,
						  IOUSBCompletion *	completion = 0,
						  IOByteCount *		bytesRead = 0);
	
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  5);
    /*!
        @function Write
	 Write to an interrupt or bulk endpoint
	 @param buffer place to get the transferred data
	 @param noDataTimeout number of milliseconds of no bus activity until transaction times out. Note that if a tranasction times out
	 the driver software may have to resynchronize the data toggle. See ClearPipeStall.
	 @param completionTimeout number of milliseconds from the time the transaction is placed on the bus until it times out
	 @param reqCount requested number of bytes to transfer. must be <= buffer->getLength()
	 @param completion describes action to take when buffer has been emptied
	 */
    virtual IOReturn Write(IOMemoryDescriptor *	buffer,
						   UInt32		noDataTimeout,
						   UInt32		completionTimeout,
						   IOByteCount		reqCount,
                           IOUSBCompletion *	completion = 0);
    
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  6);
    /*!
        @function GetPipeStatus
	 Returns the status of the pipe (kIOUSBPipeStalled of the pipe is stalled, else kIOReturnSuccess)
	 */
    virtual IOReturn GetPipeStatus(void);
    
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  7);
    /*!
		@function ClearPipeStall
	 AVAILABLE ONLY IN VERSION 1.9 AND ABOVE
	 This method causes all outstanding I/O on a pipe to complete with return code kIOUSBTransactionReturned. It also clears both the halted bit and the
	 toggle bit on the endpoint in the controller. The driver may need to reset the data toggle within the device to avoid losing any data. If the
	 device correctly handles the ClearFeature(ENDPOINT_HALT) device request, then this API will handle that by sending the correct request to the
	 device.
	 @param withDeviceRequest if true, a ClearFeature(ENDPOINT_HALT) is sent to the appropriate endpoint on the device after the transactions on the 
	 controllers endpoint are returned and the toggle bit on the controllers endpoint is cleared. if this parameter is false, then this is equivalent
	 to the pre-1.9 API. This means that the endpoint on the controller is cleared, but no DeviceRequest is sent to the device's endpoint.
	 */
    virtual IOReturn ClearPipeStall(bool withDeviceRequest);
	
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  8);
    /*!
		@function SetPipePolicy
	 AVAILABLE ONLY IN VERSION 1.9 AND ABOVE
	 This method allows a driver to change the maxPacketSize of an Isochronous pipe, or the polling interval for an interrupt pipe. There is a limited
	 amount of bandwidth on any single bus, and isochronous pipes tend to use much of this bandwidth. The driver may know, however, that there
	 will never be as much bandwidth used as is specified in the pipe's endpoint descriptor. Therefore, the driver may return some of this 
	 bandwidth to the system by using this method. Additionally, if on an open of an IOUSBInterface any of the Isochronous pipes is unable to be 
	 opened because of a lack of bandwidth, the pipe will be created with a bandwidth of zero, and the driver may get some of the limited bandwidth
	 remaining by using this call.
	 This method returns kIOReturnBadArgument if the pipe is a bulk on control pipe, or if the maxPacketSize parameter is larger than the amount specified
	 in the endpoint descriptor. It returns kIOReturnNoBandwidth if the bandwidth requested cannot be allocated. Otherwise it returns kIOReturnSuccess.
	 @param maxPacketSize specifies the maximum number of bytes to be used in any one millisecond frame by this pipe. The value must be less than or 
	 equal to the maxPacketSize specified in the endpoint descriptor.
	 @param maxInterval not currently used. reserved for future expansion
	 
	 */
    virtual IOReturn SetPipePolicy(UInt16 maxPacketSize, UInt8 maxInterval);
	
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  9);
	
    // Transfer data over Isochronous pipes and process the frame list at hardware interrupt time
    /*!
        @function Read
	 AVAILABLE ONLY IN VERSION 1.9.2 AND ABOVE
	 Read from an isochronous endpoint and process the IOUSBLowLatencyIsocFrame fields at 
	 hardware interrupt time
	 @param buffer place to put the transferred data
	 @param frameStart USB frame number of the frame to start transfer. For SuperSpeed Isoc devices, if the frameStart is kAppleUSBSSIsocContinuousFrame, then just continue after the last transfer which was called.
	 @param numFrames Number of frames to transfer
	 @param frameList Bytes to transfer, result, and time stamp for each frame
	 @param completion describes action to take when buffer has been filled
	 @param updateFrequency describes how often (in milliseconds) should the frame list be processed
	 */
    virtual IOReturn Read(IOMemoryDescriptor *	buffer,
                          UInt64 frameStart, UInt32 numFrames, IOUSBLowLatencyIsocFrame *frameList,
                          IOUSBLowLatencyIsocCompletion *	completion = 0, UInt32 updateFrequency = 0);
	
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  10);
    /*!
        @function Write
	 AVAILABLE ONLY IN VERSION 1.9.2 AND ABOVE
	 Write to an isochronous endpoint
	 @param buffer place to get the data to transfer
	 @param frameStart USB frame number of the frame to start transfer. For SuperSpeed Isoc devices, if the frameStart is kAppleUSBSSIsocContinuousFrame, then just continue after the last transfer which was called.
	 @param numFrames Number of frames to transfer
	 @param frameList Pointer to list of frames indicating bytes to transfer and result for each frame
	 @param completion describes action to take when buffer has been emptied
	 @param updateFrequency describes how often (in milliseconds) should the frame list be processed
	 */
    virtual IOReturn Write(IOMemoryDescriptor *	buffer,
                           UInt64 frameStart, UInt32 numFrames, IOUSBLowLatencyIsocFrame *frameList,
                           IOUSBLowLatencyIsocCompletion * completion = 0, UInt32 updateFrequency = 0);
	
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  11);
    /*!
        @function Read
     Read from an interrupt or bulk endpoint
     @param buffer place to put the transferred data
     @param noDataTimeout number of milliseconds of no bus activity until transaction times out. Note that if a tranasction times out
     the driver software may have to resynchronize the data toggle. See ClearPipeStall.
     @param completionTimeout number of milliseconds from the time the transaction is placed on the bus until it times out
     @param reqCount requested number of bytes to transfer. must be <= buffer->getLength()
     @param completion describes action to take when buffer has been filled
     @param bytesRead returns total bytes read for synchronous reads
     */
    virtual IOReturn Read(IOMemoryDescriptor *			buffer,
                          UInt32				noDataTimeout,
                          UInt32				completionTimeout,
                          IOByteCount				reqCount,
                          IOUSBCompletionWithTimeStamp *	completion = 0,
                          IOByteCount *				bytesRead = 0);
	
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  12);
	virtual bool InitToEndpoint(const IOUSBEndpointDescriptor *endpoint, UInt8 speed,
								USBDeviceAddress address, IOUSBController * controller, IOUSBDevice * device, IOUSBInterface * interface);

    OSMetaClassDeclareReservedUsed(IOUSBPipe,  13);
	virtual	UInt8	GetUsageType(void);
	
    OSMetaClassDeclareReservedUsed(IOUSBPipe,  14);
	virtual UInt8	GetSyncType(void);
	
    OSMetaClassDeclareReservedUnused(IOUSBPipe,  15);
    OSMetaClassDeclareReservedUnused(IOUSBPipe,  16);
    OSMetaClassDeclareReservedUnused(IOUSBPipe,  17);
	OSMetaClassDeclareReservedUnused(IOUSBPipe,  18);
    OSMetaClassDeclareReservedUnused(IOUSBPipe,  19);
	
};

#endif
