/*
 * Copyright © 1998-2013 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_IOUSBCONTROLLERV2_H
#define _IOKIT_IOUSBCONTROLLERV2_H

#include <IOKit/IODMACommand.h>

#include <IOKit/usb/IOUSBControllerListElement.h>
#include <IOKit/usb/IOUSBController.h>



/*!
    @class IOUSBControllerV2
    @abstract subclass of the IOUSBController to provide support for high speed 
                devices and split transactions.
    @discussion The IOUSBController class provide sufficient functionality to 
                work with full (12Mb/s) and low (1.5Mb/s) devices. IOUSBControllerV2
                introduces the support for high (480Mb/s) speed devices from the
                USB 2.0 spec. In particular IOUSBControllerV2 indicates the high
                speed hub a full or low speed device is attached to so that split
                transactions can be directed to the hub at high speed to be forwarded
                to the full or low speed device by the hub.
                It also gives support for bulk endoints of greater than 256 bytes.
*/

class IOUSBControllerV2 : public IOUSBController
{
    OSDeclareAbstractStructors(IOUSBControllerV2)

protected:
    
    // These for keeping track of high speed ancestor to allow controller to do splits.
    //
    UInt8 _highSpeedHub[128];
    UInt8 _highSpeedPort[128];

    struct V2ExpansionData { 
		UInt8										_multiTT[128];
		IOUSBCommand								*ClearTTCommand;
		IOUSBControllerIsochEndpoint				*_isochEPList;						// linked list of active Isoch "endpoints"
		IOUSBControllerIsochEndpoint				*_freeIsochEPList;					// linked list of freed Isoch EP data structures
		thread_call_t								_returnIsochDoneQueueThread;
	};
    V2ExpansionData *_v2ExpansionData;

    // Super's expansion data
    #define _freeUSBCommandPool					_expansionData->freeUSBCommandPool
    #define _freeUSBIsocCommandPool				_expansionData->freeUSBIsocCommandPool
    #define _watchdogUSBTimer					_expansionData->watchdogUSBTimer
    #define _controllerTerminating				_expansionData->_terminating
    #define _watchdogTimerActive				_expansionData->_watchdogTimerActive
    #define _busNumber							_expansionData->_busNumber
    #define _currentSizeOfCommandPool			_expansionData->_currentSizeOfCommandPool
    #define _currentSizeOfIsocCommandPool		_expansionData->_currentSizeOfIsocCommandPool
    #define _controllerSpeed					_expansionData->_controllerSpeed
	#define _activeIsochTransfers				_expansionData->_activeIsochTransfers
	#define _activeInterruptTransfers			_expansionData->_activeInterruptTransfers
	#define _rootHubDeviceSS					_expansionData->_rootHubDeviceSS

	// this class's expansion data
	#define _isochEPList						_v2ExpansionData->_isochEPList
	#define _freeIsochEPList					_v2ExpansionData->_freeIsochEPList
	#define _returnIsochDoneQueueThread			_v2ExpansionData->_returnIsochDoneQueueThread
	
    virtual bool 		init( OSDictionary *  propTable );
    virtual bool 		start( IOService *  provider );
    virtual void		free();

    static IOReturn  DoCreateEP(OSObject *owner,
                           void *arg0, void *arg1,
                           void *arg2, void *arg3);

    static void		clearTTHandler( 
							OSObject *	target,
                            void *	parameter,
                            IOReturn	status,
                            UInt32	bufferSizeRemaining );
    
    void UpdateTopology(USBDeviceAddress deviceAddress,
                              UInt8 speed,
                              USBDeviceAddress hubAddress,
                              int port);

public:

       /*!
        @function openPipe
         Open a pipe to the specified device endpoint
         @param address Address of the device on the USB bus
         @param speed of the device: kUSBDeviceSpeedLow, kUSBDeviceSpeedFull, kUSBDeviceSpeedHigh or kUSBDeviceSpeedSuper
         @param endpoint description of endpoint to connect to
         */
        virtual IOReturn 		OpenPipe(   USBDeviceAddress 	address,
                                       UInt8 		speed,
                                       Endpoint *		endpoint );

    /*!
     @function CreateDevice
     @abstract Create a new device as IOUSBController, making a note of the
     high speed hub device ID and port number the full/low speed
     device is attached to.
     @param newDevice       new device object to work with
     @param deviceAddress   USB device ID
     @param maxPacketSize   max packet size of endpoint zero
     @param speed           speed of the device kUSBDeviceSpeedLow, kUSBDeviceSpeedFull, kUSBDeviceSpeedHigh, kUSBDeviceSpeedSuper
     @param powerAvailable  power available to the device 
     @param hub             USB ID of hub the device is immediatly attached to. (Not necessarily high speed)
     @param port            port number of port the device is attached to.
     */
    virtual  IOReturn CreateDevice(	IOUSBDevice 		*newDevice,
                                    USBDeviceAddress	deviceAddress,
                                    UInt8		 	maxPacketSize,
                                    UInt8			speed,
                                    UInt32			powerAvailable,
                                    USBDeviceAddress		hub,
                                    int      port);

    /*!
     @function ConfigureDeviceZero
     @abstract configure pipe zero of device zero, as IOUSBController, but also keeping 
     note of high speed hub device is attached to.
     @param maxPacketSize  max packet size for the pipe
     @param speed           speed of the device kUSBDeviceSpeedLow, kUSBDeviceSpeedFull, kUSBDeviceSpeedHigh, kUSBDeviceSpeedSuper
     @param hub            USB ID of hub the device is immediatly attached to.  (Not necessarily high speed)
     @param port           port number of port the device is attached to.
     */
    virtual  IOReturn ConfigureDeviceZero(UInt8 maxPacketSize, UInt8 speed, USBDeviceAddress hub, int port);

/*!
	@function UIMCreateControlEndpoint
    @abstract Create an endpoint in the controller to do control transactions.
    @param functionNumber  USB device ID of device
    @param endpointNumber  endpoint address of the endpoint in the device
    @param maxPacketSize   maximum packet size of this endpoint
    @param speed           speed of the device kUSBDeviceSpeedLow, kUSBDeviceSpeedFull, kUSBDeviceSpeedHigh
    @param highSpeedHub    If speed is not kUSBDeviceSpeedHigh, the address of the high speed hub to
                           address split transactions to.
    @param highSpeedPort   If speed is not kUSBDeviceSpeedHigh, the hub port to address split transactions to
*/
    virtual IOReturn 		UIMCreateControlEndpoint(
                                                            UInt8	functionNumber,
                                                            UInt8	endpointNumber,
                                                            UInt16	maxPacketSize,
                                                            UInt8	speed,
                                                            USBDeviceAddress highSpeedHub,
                                                            int      highSpeedPort) = 0;

/*!
	@function UIMCreateBulkEndpoint
    @abstract Create an endpoint in the controller to do bulk transactions.
    @param functionNumber  USB device ID of device
    @param endpointNumber  endpoint address of the endpoint in the device
    @param direction       Direction of data flow. kUSBIn or kUSBOut
    @param maxPacketSize   maximum packet size of this endpoint
    @param speed           speed of the device kUSBDeviceSpeedFull, kUSBDeviceSpeedHigh
    @param highSpeedHub    If speed is not kUSBDeviceSpeedHigh, the address of the high speed hub to
                           address split transactions to.
    @param highSpeedPort   If speed is not kUSBDeviceSpeedHigh, the hub port to address split transactions to
*/
    virtual IOReturn 		UIMCreateBulkEndpoint(
                                                        UInt8		functionNumber,
                                                        UInt8		endpointNumber,
                                                        UInt8		direction,
                                                        UInt8		speed,
                                                        UInt16		maxPacketSize,
                                                        USBDeviceAddress highSpeedHub,
                                                        int      highSpeedPort) = 0;

/*!
	@function UIMCreateInterruptEndpoint
    @abstract Create an endpoint in the controller to do interrupt transactions.
    @param functionAddress USB device ID of device
    @param endpointNumber  endpoint address of the endpoint in the device
    @param direction       Direction of data flow. kUSBIn or kUSBOut
    @param speed           speed of the device kUSBDeviceSpeedLow, kUSBDeviceSpeedFull, kUSBDeviceSpeedHigh
    @param maxPacketSize   maximum packet size of this endpoint
    @param pollingRate     The maximum polling interval from the endpoint descriptor.
    @param highSpeedHub    If speed is not kUSBDeviceSpeedHigh, the address of the high speed hub to
                           address split transactions to.
    @param highSpeedPort   If speed is not kUSBDeviceSpeedHigh, the hub port to address split transactions to
*/
    virtual IOReturn		UIMCreateInterruptEndpoint(
                                                                short		functionAddress,
                                                                short		endpointNumber,
                                                                UInt8		direction,
                                                                short		speed,
                                                                UInt16		maxPacketSize,
                                                                short		pollingRate,
                                                                USBDeviceAddress highSpeedHub,
                                                                int      highSpeedPort) = 0;

/*!
	@function UIMCreateIsochEndpoint
    @abstract Create an endpoint in the controller to do Isochronous transactions.
    @param functionAddress USB device ID of device
    @param endpointNumber  endpoint address of the endpoint in the device
    @param maxPacketSize   maximum packet size of this endpoint
    @param direction       Specifies direction for the endpoint. kUSBIn or KUSBOut.
    @param highSpeedHub    If non zero, this is a full speed device, the address of the high speed hub to
                           address split transactions to.
    @param highSpeedPort   If highSpeedHub is non zero, the hub port to address split transactions to
*/
    virtual IOReturn 		UIMCreateIsochEndpoint(
                                                        short				functionAddress,
                                                        short				endpointNumber,
                                                        UInt32				maxPacketSize,
                                                        UInt8				direction,
                                                        USBDeviceAddress	highSpeedHub,
                                                        int					highSpeedPort) = 0;


	static void 				ReturnIsochDoneQueueEntry(OSObject *target, thread_call_param_t endpointPtr);

	
	OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  0);
    virtual IOReturn		AddHSHub(USBDeviceAddress highSpeedHub, UInt32 flags);
    
    static IOReturn DOHSHubMaintenance(OSObject *owner, void *arg0, void *arg1, void *arg2, void *arg3);
    
    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  1);
    virtual IOReturn 		UIMHubMaintenance(USBDeviceAddress highSpeedHub, UInt32 highSpeedPort, UInt32 command, UInt32 flags);
    
    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  2);
    virtual IOReturn		RemoveHSHub(USBDeviceAddress highSpeedHub);
    
    static IOReturn DOSetTestMode(OSObject *owner, void *arg0, void *arg1, void *arg2, void *arg3);

    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  3);
    virtual IOReturn		SetTestMode(UInt32 mode, UInt32 port);
    
    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  4);
    virtual IOReturn		UIMSetTestMode(UInt32 mode, UInt32 port);
    
    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  5);
    virtual UInt64		GetMicroFrameNumber( void );
    
    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  6);
    virtual void ClearTT(USBDeviceAddress addr, UInt8 endpt, Boolean IN);

    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  7);
    /*!
        @function Read
     Read from an interrupt or bulk endpoint
     @param buffer place to put the transferred data
     @param address Address of the device on the USB bus
     @param endpoint description of endpoint
     @param completion describes action to take when buffer has been filled
     @param noDataTimeout number of milliseconds of no data movement before the request is aborted
     @param completionTimeout number of milliseonds after the command is on the bus in which it must complete
     @param reqCount number of bytes requested for the transfer (must not be greater than the length of the buffer)
     */
    virtual IOReturn ReadV2( IOMemoryDescriptor *			buffer,
                           USBDeviceAddress					address,
                           Endpoint *						endpoint,
                           IOUSBCompletionWithTimeStamp *	completion,
                           UInt32							noDataTimeout,
                           UInt32							completionTimeout,
                           IOByteCount						reqCount );
    
    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  8);
/*!
	@function UIMCreateIsochEndpoint
    @abstract Create an endpoint in the controller to do Isochronous transactions.
    @param functionAddress USB device ID of device
    @param endpointNumber  endpoint address of the endpoint in the device
    @param maxPacketSize   maximum packet size of this endpoint
    @param direction       Specifies direction for the endpoint. kUSBIn or KUSBOut.
    @param highSpeedHub    If non zero, this is a full speed device, the address of the high speed hub to
                           address split transactions to.
    @param highSpeedPort   If highSpeedHub is non zero, the hub port to address split transactions to
	@param interval		   The encoded interval value from the endpoint descriptor
*/
    virtual IOReturn 		UIMCreateIsochEndpoint(		short				functionAddress,
                                                        short				endpointNumber,
                                                        UInt32				maxPacketSize,
                                                        UInt8				direction,
                                                        USBDeviceAddress	highSpeedHub,
                                                        int					highSpeedPort,
														UInt8				interval);


    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  9);
	virtual IOUSBControllerIsochEndpoint*		AllocateIsochEP(void);	
	
    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  10);
	virtual IOReturn							DeallocateIsochEP(IOUSBControllerIsochEndpoint *pEP);

	OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  11);
    virtual IOUSBControllerIsochEndpoint* 	FindIsochronousEndpoint(short functionNumber, short endpointNumber, short direction, IOUSBControllerIsochEndpoint* *pEDBack);

    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  12);
    virtual IOUSBControllerIsochEndpoint*	CreateIsochronousEndpoint(short functionNumber, short endpointNumber, short direction);
	
    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  13);
    virtual void								PutTDonToDoList(IOUSBControllerIsochEndpoint* pED, IOUSBControllerIsochListElement *pTD);

    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  14);
    virtual IOUSBControllerIsochListElement		*GetTDfromToDoList(IOUSBControllerIsochEndpoint* pED);

    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  15);
    virtual void								PutTDonDeferredQueue(IOUSBControllerIsochEndpoint* pED, IOUSBControllerIsochListElement *pTD);

    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  16);
	virtual IOUSBControllerIsochListElement		*GetTDfromDeferredQueue(IOUSBControllerIsochEndpoint* pED);

    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  17);
    virtual void								PutTDonDoneQueue(IOUSBControllerIsochEndpoint* pED, IOUSBControllerIsochListElement *pTD, bool checkDeferred);

    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  18);
    virtual IOUSBControllerIsochListElement		*GetTDfromDoneQueue(IOUSBControllerIsochEndpoint* pED);
	
	// 7185026 - this is to make this call from behind the gate
    static IOReturn		GatedGetTDfromDoneQueue(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	
    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  19);
    virtual void								ReturnIsochDoneQueue(IOUSBControllerIsochEndpoint*);

    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  20);
	virtual IODMACommand						*GetNewDMACommand();
	
    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  21);
	/*!
	 @function GetLowLatencyOptionsAndPhysicalMask
	 @abstract Low Latency transfers require that the client have access to the memory after the Isochronous I/O request has already been scheduled. This might be used, for example to fill in outgoing data "just in time." Some controllers, however, may have requirements which need to be followed in order to make sure that the memory buffer isn't moved after the call is made. This call will return an IOOptionBits and mach_vm_address_t which can be used in a call to IOBufferMemoryDescriptor::inTaskWithPhysicalMask which will help meet these requirements.
	 @param optionBits Pointer to an an IOOptionBits. The only bit which may be returned is kIOMemoryPhysicallyContiguous. Other bits, e.g. direction bits, must be ORd in by the client as needed. This call replaces the old property based method of obtaining this information.
	 @param physicalMask  Pointer to a mach_vm_address_t which should be used in the call to IOBufferMemoryDescriptor::inTaskWithPhysicalMask and will guarantee that when the memory is wired down it will be accessible by both the client and the USB controller at the same time.
	 @result returns kIOReturnSuccess if the method is implemented by the controller, otherwise kIOReturnUnsupported
	 */
    virtual IOReturn 		GetLowLatencyOptionsAndPhysicalMask(IOOptionBits *optionBits, mach_vm_address_t *physicalMask);
	
	OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  22);
	/*!
	 @function GetFrameNumberWithTime
	 @abstract Real Time A/V applications send and receive Iscohronous data scheduled on certain USB frame numbers. The clock for these frame numbers is independent of the system clock, and drivers need to synchronize these two clocks. This routine will return a system time which corresponds to the beginning of a USB frame number. It is not necessarily the currrent frame, but it will be a frame in the recent past (within the past minute). The jitter between the start of the USB frame and the system time will be as low as possible, but due to hardware interrupt latencies could be as high as 200 microseconds.
	 @param frameNumber A pointer to a UInt64 in which to hold the USB frame number corresponding to the given system time.
	 @param theTime A pointer to an AbsoluteTime corresponding to the system time at the beginning of the given USB frame number.
	 @result returns kIOReturnSuccess if the method is implemented by the controller, otherwise kIOReturnUnsupported
	 */
	virtual IOReturn		GetFrameNumberWithTime(UInt64* frameNumber, AbsoluteTime *theTime);
	
	
	OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  23);
    /*!
	 @function Read
     Read from a stream on a bulk endpoint
     @param streamID stream ID of the stream to read from
     @param buffer place to put the transferred data
     @param address Address of the device on the USB bus
     @param endpoint description of endpoint
     @param completion describes action to take when buffer has been filled
     @param noDataTimeout number of milliseconds of no data movement before the request is aborted
     @param completionTimeout number of milliseonds after the command is on the bus in which it must complete
     @param reqCount number of bytes requested for the transfer (must not be greater than the length of the buffer)
     */
    virtual IOReturn ReadStream(UInt32							streamID,
							IOMemoryDescriptor *			buffer,
							USBDeviceAddress					address,
							Endpoint *						endpoint,
							IOUSBCompletion *	completion,
							UInt32							noDataTimeout,
							UInt32							completionTimeout,
							IOByteCount						reqCount );

    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  24);
    /*!
	 @function Write
	 Write to a stream on a bulk endpoint
     @param streamID stream ID of the stream to write to
	 @param buffer place to get the transferred data
	 @param address Address of the device on the USB bus
	 @param endpoint description of endpoint
	 @param completion describes action to take when buffer has been emptied
	 @param noDataTimeout number of milliseconds of no data movement before the request is aborted
	 @param completionTimeout number of milliseonds after the command is on the bus in which it must complete
	 @param reqCount number of bytes requested for the transfer (must not be greater than the length of the buffer)
	 */
    virtual IOReturn WriteStream(UInt32				streamID,
								  IOMemoryDescriptor *	buffer,
								  USBDeviceAddress 	address,
								  Endpoint *		endpoint,
								  IOUSBCompletion *	completion,
								  UInt32			noDataTimeout,
								  UInt32			completionTimeout,
								  IOByteCount		reqCount );

    OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  25);
	/*!
	 @function openPipe
	 Open a pipe to the specified device endpoint
	 @param address Address of the device on the USB bus
	 @param speed of the device: kUSBDeviceSpeedLow, kUSBDeviceSpeedFull, kUSBDeviceSpeedHigh or kUSBDeviceSpeedSuper
	 @param endpoint description of endpoint to connect to
	 @param maxStreams maximum number of streams pipe supports
	 @param maxBurstAndMult maximum number of bursts and burst multiplier
	 */
	virtual IOReturn 		OpenSSPipe(USBDeviceAddress 	address,
									   UInt8				speed,
									   Endpoint *			endpoint,
									   UInt32				maxStreams,
									   UInt32				maxBurstAndMult);
    
	OSMetaClassDeclareReservedUsed(IOUSBControllerV2,  26);
	/*!
	 @function UpdateDeviceAddress
	 Tell the controller about the new address of a device. Used when a device has been reset
	 @param deviceAddress Address of the device on the USB bus
	 @param speed of the device: kUSBDeviceSpeedLow, kUSBDeviceSpeedFull, kUSBDeviceSpeedHigh or kUSBDeviceSpeedSuper
     @param highSpeedHub    If non zero, this is a full speed device, the address of the high speed hub to address split transactions to.
     @param highSpeedPort   If highSpeedHub is non zero, the hub port to address split transactions to
	 */
    virtual IOReturn 		UpdateDeviceAddress(USBDeviceAddress oldDeviceAddress,
                              USBDeviceAddress newDeviceAddress,
                              UInt8 speed,
                              USBDeviceAddress hubAddress,
                              int port);

    
    
	OSMetaClassDeclareReservedUnused(IOUSBControllerV2,  27);
    OSMetaClassDeclareReservedUnused(IOUSBControllerV2,  28);
    OSMetaClassDeclareReservedUnused(IOUSBControllerV2,  29);
    
};


#endif
