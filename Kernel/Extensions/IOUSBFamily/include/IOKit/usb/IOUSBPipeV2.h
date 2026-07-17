/*
 * Copyright © 2012 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_IOUSBPIPEV2_H
#define _IOKIT_IOUSBPIPEV2_H

#include <IOKit/IOService.h>
#include <IOKit/IOMemoryDescriptor.h>

#include <IOKit/usb/USB.h>
#include <IOKit/usb/IOUSBController.h>
#include <IOKit/usb/IOUSBControllerV2.h>
#include <IOKit/usb/IOUSBPipe.h>


/*!
    @class IOUSBPipeV2
    @abstract The object representing an open pipe for a device.
*/
class IOUSBPipeV2 : public IOUSBPipe
{
    friend class IOUSBInterface;
    friend class IOUSBDevice;
	
    OSDeclareDefaultStructors(IOUSBPipeV2)
		
protected:
		
	IOUSBSuperSpeedEndpointCompanionDescriptor *	_sscd;
	UInt32											_maxStream;										// number to be used as an exponent to get the actual maximum
	UInt32                      					_maxBurst;                                      // zero based (i.e. 0=1 burst)
    UInt32                                          _configuredStreams;
	UInt32											_mult;                                          // zero based max value is 2 (meaning 3 packets)
	UInt32											_bytesPerInterval;

    struct V2PipeExpansionData
    {
    };
    V2PipeExpansionData * _v2PipeExpansionData;
    	
    static IOUSBPipeV2 *ToEndpoint(const IOUSBEndpointDescriptor *endpoint, IOUSBSuperSpeedEndpointCompanionDescriptor *sscd,
                                 IOUSBDevice * device, IOUSBController * controller, IOUSBInterface *interface);
	
public:
    using IOUSBPipe::Read;
    using IOUSBPipe::Write;
    using IOUSBPipe::Abort;
    
    // inherited from the IOUSBPipe class
	virtual IOReturn SetPipePolicy(UInt16 maxPacketSize, UInt8 maxInterval);		//this must have the same KPI as IOUSBPipe::SetPipePolicy

	// New to IOUSBPipeV2
	virtual bool InitToEndpoint(const IOUSBEndpointDescriptor *endpoint, IOUSBSuperSpeedEndpointCompanionDescriptor *sscd, UInt8 speed,
								USBDeviceAddress address, IOUSBController * controller, IOUSBDevice * device, IOUSBInterface * interface);
	
    /*!
	 @function Read
     Read from a stream on a bulk endpoint
     @param streamID ID of the stream to read from
     @param buffer place to put the transferred data
     @param noDataTimeout number of milliseconds of no bus activity until transaction times out. Note that if a tranasction times out
     the driver software may have to resynchronize the data toggle. See ClearPipeStall.
     @param completionTimeout number of milliseconds from the time the transaction is placed on the bus until it times out
     @param reqCount requested number of bytes to transfer. must be <= buffer->getLength()
     @param completion describes action to take when buffer has been filled
     @param bytesRead returns total bytes read for synchronous reads
     */
    virtual IOReturn Read(UInt32				streamID,
						  IOMemoryDescriptor *	buffer,
                          UInt32				noDataTimeout,
                          UInt32				completionTimeout,
                          IOByteCount			reqCount,
                          IOUSBCompletion   *	completion = 0,
                          IOByteCount *			bytesRead = 0);
	
    /*!
	 @function Write
	 Write to a stream on a bulk endpoint
     @param streamID ID of the stream to write to
	 @param buffer place to get the transferred data
	 @param noDataTimeout number of milliseconds of no bus activity until transaction times out. Note that if a tranasction times out
	 the driver software may have to resynchronize the data toggle. See ClearPipeStall.
	 @param completionTimeout number of milliseconds from the time the transaction is placed on the bus until it times out
	 @param reqCount requested number of bytes to transfer. must be <= buffer->getLength()
	 @param completion describes action to take when buffer has been emptied
	 */
    virtual IOReturn Write(UInt32				streamID,
						   IOMemoryDescriptor *	buffer,
						   UInt32		noDataTimeout,
						   UInt32		completionTimeout,
						   IOByteCount		reqCount,
                           IOUSBCompletion *	completion = 0);

    /*!
	 @function SupportsStreams
	 Returns non zero if the pipe supports streams. 
	 Return value is the max supported stream.
	 */
	virtual UInt32 SupportsStreams(void);
	
    /*!
	 @function CreateStreams
     @param streamID ID of the highest stream to create (pass 0 to destroy streams)
	 */
	virtual IOReturn CreateStreams(UInt32 maxStreams);
	
	/*!
	 @function Abort(stream)
	 This method causes all outstanding I/O on a stream of a pipe to complete with return code kIOReturnAborted. It  clears the halted bit but does NOT clear the
	 toggle bit on the endpoint in the controller.  If you wish to clear the toggle bit, see ClearPipeStall
     @param streamID ID of the stream to abort
	 */
    virtual IOReturn Abort(UInt32 streamID);
    
	/*!
	 @function GetConfiguredStreams
	 Get the number of streams which have been configured for the endpoint with CreateStreams
	 */
    virtual UInt32 GetConfiguredStreams(void);
	
	/*!
	 @function GetSuperSpeedEndpointCompanionDescriptor
	 returns the SuperSpeed endpoint companion descriptor for the pipe.
	 */
    virtual const IOUSBSuperSpeedEndpointCompanionDescriptor *	GetSuperSpeedEndpointCompanionDescriptor();
  
	/*!
	 @function GetMaxBurst
	 returns the value of bMaxBurst from the SuperSpeed endpoint companion descriptor
	 */
    virtual UInt8 GetMaxBurst();
	
    /*!
	 @function GetMaxBurst
	 returns the value of bMaxBurst from the SuperSpeed endpoint companion descriptor
	 */
    virtual UInt8 GetMult();
	
    /*!
	 @function GetMaxBurst
	 returns the value of bMaxBurst from the SuperSpeed endpoint companion descriptor
	 */
    virtual UInt16 GetBytesPerInterval();
	
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  0);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  1);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  2);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  3);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  4);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  5);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  6);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  7);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  8);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  9);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  10);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  11);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  12);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  13);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  14);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  15);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  16);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  17);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  18);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  19);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  20);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  21);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  22);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  23);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  24);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  25);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  26);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  27);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  28);
	OSMetaClassDeclareReservedUnused(IOUSBPipeV2,  29);
};

#endif	
