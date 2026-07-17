/*
 * Copyright © 1998-2007, 2012 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_IOUSBINTERFACE_H
#define _IOKIT_IOUSBINTERFACE_H

#include <IOKit/IOService.h>
#include <libkern/c++/OSData.h>

#include <IOKit/usb/USB.h>
#include <IOKit/usb/IOUSBNub.h>
#include <IOKit/usb/IOUSBDevice.h>

/*!
    @class IOUSBInterface
    @abstract The object representing an interface of a device on the USB bus.
    @discussion This class provides functionality to find the pipes of an interface and
	to read the descriptors associated with an interface. When an interface is open()ed,
	all its pipes are created
*/
class IOUSBInterface : public IOUSBNub
{
    friend class IOUSBInterfaceUserClientV2;
    friend class IOUSBInterfaceUserClientV3;
    friend class IOUSBDevice;
	friend class IOUSBPipe;
	friend class IOUSBPipeV2;
	
    OSDeclareDefaultStructors(IOUSBInterface)

protected:
    IOUSBPipe * 			_pipeList[kUSBMaxPipes];
    const IOUSBConfigurationDescriptor *_configDesc;
    const IOUSBInterfaceDescriptor *	_interfaceDesc;
    IOUSBDevice *			_device;

    // these variable are the parsed interface descriptor
    UInt8				_bInterfaceNumber;
    UInt8				_bAlternateSetting;
    UInt8				_bNumEndpoints;
    UInt8				_bInterfaceClass;
    UInt8				_bInterfaceSubClass;
    UInt8				_bInterfaceProtocol;
    UInt8				_iInterface;

    struct ExpansionData {
        IOCommandGate		*_gate;
        IOWorkLoop			*_workLoop;
		bool				_needToClose;
		IOLock *			_pipeObjLock;				// Deprecated
		OSSet *				_openClients;
		IOService *			_openClient;
        UInt32              _RememberedStreams[kUSBMaxPipes];
    };
    ExpansionData * _expansionData;

    // private methods
    virtual void		ClosePipes(void);				// close all pipes (except pipe zero)
    virtual IOReturn	CreatePipes(void);				// open all pipes in the current interface/alt interface
    virtual void		SetProperties(void);			// update my property table with the correct properties		

    IOReturn 			ResetPipes(void);				// reset all pipes (except pipe zero) (not virtual)
    IOReturn 			AbortPipesGated(void);			// abort all pipes (except pipe zero) (not virtual)
    IOReturn 			ClosePipesGated(bool close);	// Abort and close or unlink all pipes (except pipe zero) (not virtual)
	IOUSBPipe*			FindNextPipeGated(IOUSBPipe *current, IOUSBFindEndpointRequest *request, bool withRetain);
	IOUSBPipe*			GetPipeObjGated(UInt8 index);
    IOReturn 			ReopenPipesGated();             // relink all pipes (except pipe zero) (not virtual)
	void	 			RememberStreamsGated(void);
	IOReturn	 		RecreateStreamsGated(void);
    
    // Return the "Full" MaxPacketSize, given an endpoint descriptor and a pointer to an sscd if there is one
    // This will multiple the MPS in the ep by the mult and the burst which may or may not be present
    UInt16              CalculateFullMaxPacketSize(IOUSBEndpointDescriptor *ed, IOUSBSuperSpeedEndpointCompanionDescriptor *sscd);
	
public:
	// static methods
    static IOUSBInterface *		withDescriptors(const IOUSBConfigurationDescriptor *cfDesc, const IOUSBInterfaceDescriptor *ifDesc);
    static IOReturn				CallSuperOpen(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
    static IOReturn     		CallSuperClose(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	static IOReturn 			_ResetPipes(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	static IOReturn 			_AbortPipes(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	static IOReturn 			_ClosePipes(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	static IOReturn 			_FindNextPipe(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	static IOReturn 			_GetPipeObj(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	static IOReturn 			_ReopenPipes(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	static IOReturn 			_RememberStreams(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);
	static IOReturn 			_RecreateStreams(OSObject *target, void *arg0, void *arg1, void *arg2, void *arg3);

	static UInt8 				hex2char( UInt8 digit );
    
	// IOService methods
    virtual bool		start(IOService * provider);
    virtual bool		handleOpen(IOService *forClient, IOOptionBits options = 0, void *arg = 0 );
 	virtual bool		handleIsOpen(const IOService *forClient) const;
	virtual bool		open(IOService *forClient, IOOptionBits options = 0, void *arg = 0 );
    virtual void		close(IOService *forClient, IOOptionBits options = 0);
    virtual void		handleClose(IOService *forClient, IOOptionBits options = 0);
    virtual IOReturn 	message( UInt32 type, IOService * provider,  void * argument = 0 );
    virtual bool		finalize(IOOptionBits options);
    virtual void		stop(IOService *  provider);
    virtual bool		terminate( IOOptionBits options = 0 );
    virtual void		free();	

	// IOUSBInterface class methods
	virtual bool		init(	const IOUSBConfigurationDescriptor *cfDesc,
							 const IOUSBInterfaceDescriptor *ifDesc);
   /*!
        @function FindNextAltInterface
        return alternate interface descriptor satisfying the requirements specified in request, or NULL if there aren't any.
        request is updated with the properties of the returned interface.
        @param current interface descriptor to start searching from, NULL to start at alternate interface 0.
        @param request specifies what properties an interface must have to match.
        @result Pointer to a matching interface descriptor, or NULL if none match.
    */
    virtual const IOUSBInterfaceDescriptor *FindNextAltInterface(const IOUSBInterfaceDescriptor *current,
                                                                 IOUSBFindInterfaceRequest *request);

    /*!
	@function FindNextPipe
	Find a pipe of the interface that matches the requirements, either
	starting from the beginning of the interface's pipe list or from a specified
	pipe.
	@param current Pipe to start searching from, NULL to start from beginning of list.
	@param request Requirements for pipe to match, updated with the found pipe's
	properties.
	@result Pointer to the pipe, or NULL if no pipe matches the request.
    */
    virtual IOUSBPipe *FindNextPipe(IOUSBPipe *current, IOUSBFindEndpointRequest *request);

    /*!
        @function FindNextAssociatedDescriptor
        Find the next descriptor of the requested type associated with the interface.
	@param current Descriptor to start searching from, NULL to start from beginning of list.
	@param type Descriptor type to search for, or kUSBAnyDesc to return any descriptor type.
	@result Pointer to the descriptor, or NULL if no matching descriptors found.
    */
    virtual const IOUSBDescriptorHeader * FindNextAssociatedDescriptor(const void *current, UInt8 type);

    /*!
        @function SetAlternateInterface
        Select the specified alternate interface.
        @param forClient The client requesting the alternate setting. This client must have the interface open in order to perform the request.
	@param alternateSetting Alternate setting (from the alternate interface's interface descriptor).
        @result exclusive access error if the interface is not open. otherwise the result of the transaction
    */
    virtual IOReturn SetAlternateInterface(IOService *forClient, UInt16 alternateSetting);

     /*!
        @function GetPipeObj
	returns a handle to the pipe at the corresponding index
        @param index value from zero to kUSBMaxPipes-1
	@result The IOUSBPipe object. Note that the client does not own a reference to this pipe, so the client should retain() the IOUSBPipe object if necessary.
    */
    virtual IOUSBPipe *GetPipeObj(UInt8 index);
    /*!
        @function GetConfigValue
	returns the device configuration value for the interface
	@result The device configuration value.
    */
    virtual UInt8 GetConfigValue();
    /*!
        @function GetDevice
        returns the device the interface is part of.
	@result Pointer to the IOUSBDevice object which is the parent of this IOUSBInterface object.
    */
    virtual IOUSBDevice *GetDevice();
    /*!
        @function GetInterfaceNumber
        returns the zero based value identifying the index in the array of concurrent
        interfaces supported by the current configuration
        @result the interface index
    */
    virtual UInt8 GetInterfaceNumber();
    /*!
        @function GetAlternateSetting
        returns the alternate setting for this interface.
        @result the alternate setting
    */
    virtual UInt8 GetAlternateSetting();
    /*!
        @function GetNumEndpoints
        returns the number of endpoints used by this interface (excluding
        device endpoint zero. If the value is zero, this interface only
        uses endpoint zero.
        @result the number of endpoints
    */
    virtual UInt8 GetNumEndpoints();
    /*!
        @function GetInterfaceClass
        returns the class code for this interface (assigned by the USB)
        a value of zero is reserved
        if the value is FFh, the interface class is vendor-specific
        all other values are reserved for assignment by the USB
        @result the interface class
    */
    virtual UInt8 GetInterfaceClass();
    /*!
        @function GetInterfaceSubClass
        returns the subclass code (assigned by the USB).
        These codes are qualified by the value returned by GetInterfaceClass
        @result the interface subclass
    */
    virtual UInt8 GetInterfaceSubClass();
    /*!
        @function GetInterfaceProtocol
        returns the protocol code (assigned by the USB).
        @result the interface index
    */
    virtual UInt8 GetInterfaceProtocol();
    /*!
        @function GetInterfaceStringIndex
        returns the index of the string descriptor describing the interface
        @result the string index
    */
    virtual UInt8 GetInterfaceStringIndex();
    
    /*!
	@function DeviceRequest
        @abstract Sends a control request to the default control pipe in the device (pipe zero)
        @param request The parameter block to send to the device
		@param completion Function to call when request completes. If omitted then
        DeviceRequest() executes synchronously, blocking until the request is complete.  If the request is asynchronous, the client must make sure that
		the IOUSBDevRequest is not released until the callback has occurred.
    */
    virtual IOReturn DeviceRequest(IOUSBDevRequest *request, IOUSBCompletion *completion = 0); 

    /*!
	 @function DeviceRequest
	 @abstract Sends a control request to the default control pipe in the device (pipe zero)
	 @param request The parameter block to send to the device (with the pData as an IOMemoryDesriptor)
	 @param completion Function to call when request completes. If omitted then
	 DeviceRequest() executes synchronously, blocking until the request is complete.  If the request is asynchronous, the client must make sure that
	 the IOUSBDevRequest is not released until the callback has occurred.
	 */
    virtual IOReturn DeviceRequest(IOUSBDevRequestDesc *request, IOUSBCompletion *completion = 0);

    virtual bool matchPropertyTable(OSDictionary * table, SInt32 *score);

    OSMetaClassDeclareReservedUsed(IOUSBInterface,  0);
    /*!
	@function GetEndpointProperties
        @abstract Returns the properties of an endpoint, possibly in an alternate interface.
        @param alternateSetting specifies the desired alternate setting
        @param endpointNumber specifies the endpoint number
        @param direction specifies the direction (kUSBIn, kUSBOut)
        @param transferType a pointer to hold the transfer type (kUSBControl, kUSBBulk, etc.) of the endpoint if found.
        @param maxPacketSize a pointer to hold the maxPacketSize in the endpoint descriptor.
        @param interval a pointer to hold the interval value in the endpoint descriptor.
	@result returns kIOReturnSuccess if the endpoint is found, and kIOUSBEndpointNotFound if it is not.
    */
    virtual IOReturn GetEndpointProperties(UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    
	
	OSMetaClassDeclareReservedUsed(IOUSBInterface,  1);
    /*!
	 @function FindNextPipe
	 Find a pipe of the interface that matches the requirements, either
	 starting from the beginning of the interface's pipe list or from a specified
	 pipe.
	 @param current Pipe to start searching from, NULL to start from beginning of list.
	 @param request Requirements for pipe to match, updated with the found pipe's
	 properties.
	 @param withRetain Pass true to retain and the client should release it later after its use.
	 @result Pointer to the pipe, or NULL if no pipe matches the request.
	 */
	virtual	IOUSBPipe* FindNextPipe(IOUSBPipe *current, IOUSBFindEndpointRequest *request, bool withRetain);

    OSMetaClassDeclareReservedUsed(IOUSBInterface,  2);
    /*!
	 @function RememberStreams.
	 Make a note of which pipes have streams, so they can be recreated later.
	 @result returns kIOReturnSuccess if sucessful.
	 */
	virtual	IOReturn RememberStreams(void);

    OSMetaClassDeclareReservedUsed(IOUSBInterface,  3);
    /*!
	 @function RecreateStreams
	 Recreate the remembered streams after a device has been reset.
	 @result returns kIOReturnSuccess if sucessful.
	 */
	virtual	IOReturn RecreateStreams(void);
    
    OSMetaClassDeclareReservedUsed(IOUSBInterface,  4);
    virtual void		UnlinkPipes(void);				// delete the UIM endpoint from the pipe

    OSMetaClassDeclareReservedUsed(IOUSBInterface,  5);
    virtual void	ReopenPipes(void);				// open all pipes in the current interface/alt interface

    OSMetaClassDeclareReservedUsed(IOUSBInterface,  6);
    /*!
	 @function GetEndpointPropertiesV3
	 @abstract Returns the properties of an endpoint, possibly in an alternate interface, including any information from the SuperSpeed Companion Descriptor
	 @param endpointProperties  Pointer to a IOUSBEndpointProperties structure that upon success will contain the information for the endpoint.  The bVersion field
	 needs to be initialized with the structure version (see USBEndpointVersion enum in USB.h).  The bAlternateSetting, bEndpointNumber, and bDirection (kUSBIn, kUSBOut) MUST also be filled
	 in with the values for the desired descriptor.  Not that the bMaxStreams value for Bulk endpoints is the value specified by the bmAttributes field in the SuperSpeed companion descriptor.
	 @result returns kIOReturnSuccess if the endpoint is found, and kIOUSBEndpointNotFound if it is not.
	 */
    virtual IOReturn GetEndpointPropertiesV3(IOUSBEndpointProperties *endpointProperties);
    
    OSMetaClassDeclareReservedUsed(IOUSBInterface,  7);
    /*!
	 @function GetInterfaceStatus
	 @abstract Returns the result of issuing a GET_STATUS request on the device for this interface.
	 @param  status Pointer to a USBStatus type which will contain the results of the operation.
	 @result returns kIOReturnSuccess if the request is completed successfully, otherwise returns the result of the Device Request.
	 */
	virtual	IOReturn	GetInterfaceStatus(USBStatus *status);
	
    OSMetaClassDeclareReservedUsed(IOUSBInterface,  8);
    /*!
	 @function SetFunctionSuspendFeature
	 @abstract Issues a SET_FEATURE(FUNCTION_SUSPEND) to the interface.
	 @param  options The suspend options passed to the Device Request.
	 @result returns kIOReturnSuccess if the request is completed successfully, otherwise returns the result of the Device Request.
	 */
	virtual IOReturn	SetFunctionSuspendFeature(UInt8 options);

    OSMetaClassDeclareReservedUsed(IOUSBInterface,  9);
    /*!
	 @function EnableRemoteWake
	 @abstract Will enable or disable the USB 3.0 remote wake function for the interface
	 @param  enable Whether we enable (true) or disable (false) the remote wake..
	 @result returns kIOReturnSuccess if the request is completed successfully, otherwise returns the result of the Device Request to set the feature.
	 */
    virtual IOReturn    EnableRemoteWake(bool enable);

    
    OSMetaClassDeclareReservedUnused(IOUSBInterface,  10);
    OSMetaClassDeclareReservedUnused(IOUSBInterface,  11);
    OSMetaClassDeclareReservedUnused(IOUSBInterface,  12);
    OSMetaClassDeclareReservedUnused(IOUSBInterface,  13);
    OSMetaClassDeclareReservedUnused(IOUSBInterface,  14);
    OSMetaClassDeclareReservedUnused(IOUSBInterface,  15);
    OSMetaClassDeclareReservedUnused(IOUSBInterface,  16);
    OSMetaClassDeclareReservedUnused(IOUSBInterface,  17);
    OSMetaClassDeclareReservedUnused(IOUSBInterface,  18);
    OSMetaClassDeclareReservedUnused(IOUSBInterface,  19);

protected:

    
};

#endif
