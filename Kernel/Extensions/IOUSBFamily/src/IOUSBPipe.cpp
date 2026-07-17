/*
 * Copyright � 1998-2013 Apple Inc.  All rights reserved.
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

//================================================================================================
//
//   Headers
//
//================================================================================================
//
#include <libkern/OSByteOrder.h>

#include <IOKit/IOService.h>
#include <IOKit/IOKitKeys.h>

#include <IOKit/usb/IOUSBController.h>
#include <IOKit/usb/IOUSBControllerV2.h>
#include <IOKit/usb/USBHub.h>
#include <IOKit/usb/IOUSBDevice.h>
#include <IOKit/usb/IOUSBPipe.h>
#include <IOKit/usb/IOUSBInterface.h>
#include <IOKit/usb/IOUSBNub.h>
#include <IOKit/usb/IOUSBLog.h>

#include "IOUSBInterfaceUserClient.h"

#include "USBTracepoints.h"

//================================================================================================
//
//   Local Definitions
//
//================================================================================================
//
#define super OSObject

#ifndef kIOUserClientCrossEndianCompatibleKey
#define kIOUserClientCrossEndianCompatibleKey "IOUserClientCrossEndianCompatible"
#endif

#define	_DEVICE							_expansionData->_device
#define	_CORRECTSTATUS					_expansionData->_correctStatus
#define	_SPEED							_expansionData->_speed
#define	_INTERFACE						_expansionData->_interface
#define	_CROSSENDIANCOMPATIBLE			_expansionData->_crossEndianCompatible
#define	_LOCATIONID						_expansionData->_locationID
#define	_UASUSAGEID						_expansionData->_uasUsageID
#define	_USAGETYPE						_expansionData->_usageType
#define	_SYNCTYPE						_expansionData->_syncType


// Note:  We are overloading the use of the _status iVar -- was obsoleted, but now use it to signify that
// we should accept an illegal MPS.  We did not create a new ivar in the expansion data because we need
// to check the property before the expansion data is allocated and we did not want to modify the params
// and create another method.  We just reused an unused ivar, but we don't change the name so as not break
// binary compatibility
#define	_OUTOFSPECMPSOK				_status				// if non-zero, then we should ignore an out of spec MPS

//================================================================================================
#ifndef IOUSBPIPE_USE_KPRINTF
	#define IOUSBPIPE_USE_KPRINTF 0
#endif

#if IOUSBPIPE_USE_KPRINTF
	#undef USBLog
	#undef USBError
	void kprintf(const char *format, ...) __attribute__((format(printf, 1, 2)));
	#define USBLog( LEVEL, FORMAT, ARGS... )  if ((LEVEL) <= IOUSBPIPE_USE_KPRINTF) { kprintf( FORMAT "\n", ## ARGS ) ; }
	#define USBError( LEVEL, FORMAT, ARGS... )  { kprintf( FORMAT "\n", ## ARGS ) ; }
#endif

//================================================================================================
//
//  IOUSBPipe Methods
//
//================================================================================================
OSDefineMetaClassAndStructors(IOUSBPipe, OSObject)


#pragma mark Intializers

//================================================================================================
//
//   InitToEndpoint
//
//================================================================================================
//
bool 
IOUSBPipe::InitToEndpoint(const IOUSBEndpointDescriptor *ed, UInt8 speed, USBDeviceAddress address, IOUSBController * controller, IOUSBDevice * device, IOUSBInterface * interface)
{
    IOReturn		err;
	UInt16			baseMPS = 0;				// maximum of 1024
	UInt8			mult = 0;					// only changed with HS Isoch
    
	USBTrace_Start( kUSBTPipe, kTPPipeInitToEndpoint, (uintptr_t)this, (uintptr_t)ed, speed, address);
	
    USBLog(1, "IOUSBPipe::ToEndpoint, obsolete method called");
    if ( !super::init() || ed == 0)
        return (false);
	
    // allocate our expansion data
    if (!_expansionData)
    {
        _expansionData = (ExpansionData *)IOMalloc(sizeof(ExpansionData));
        if (!_expansionData)
            return false;
        bzero(_expansionData, sizeof(ExpansionData));
    }
	
    _controller = controller;
    _descriptor = ed;
    _endpoint.number = ed->bEndpointAddress & kUSBPipeIDMask;
    _endpoint.transferType = ed->bmAttributes & 0x03;
    if (_endpoint.transferType == kUSBControl)
        _endpoint.direction = kUSBAnyDirn;
    else
		_endpoint.direction = (ed->bEndpointAddress & 0x80) ? kUSBIn : kUSBOut;
	
	// HS Isoch may have a multiplier in the higher bits of the wMaxPacketSize field. no other speeds should have any bits
	// at all in the higher bits. SS devices have a _different_ multiplier which is found in a different field
	
	baseMPS = (ed->wMaxPacketSize & kUSB_EPDesc_wMaxPacketSize_MPS_Mask) >> kUSB_EPDesc_wMaxPacketSize_MPS_Shift;
	
	// this is safe to do even for SS devices because the actual mult field moved
	mult    = (ed->wMaxPacketSize & kUSB_HSFSEPDesc_wMaxPacketSize_Mult_Mask) >> kUSB_HSFSEPDesc_wMaxPacketSize_Mult_Shift;
	if (mult > 2)
	{
		mult = 2;
		USBLog(1,"IOUSBPipe[%p]::InitToEndpoint (%s, EP: 0x%x) -- HS mult for wMaxPacketSize is illegal (3), setting to 2", this, device ? device->getName() : "Unnamed Device", (uint32_t)ed->bEndpointAddress);
	}
	_endpoint.maxPacketSize = baseMPS * (mult + 1);
	
    _endpoint.interval = ed->bInterval;
	
	if ( _endpoint.transferType == kUSBInterrupt)
	{
		_USAGETYPE = (ed->bmAttributes & kUSB_EPDesc_bmAttributes_UsageType_Mask) >> kUSB_EPDesc_bmAttributes_UsageType_Shift;
		_SYNCTYPE = 0;
	}
	else if ( _endpoint.transferType == kUSBIsoc)
	{
		_USAGETYPE = (ed->bmAttributes & kUSB_EPDesc_bmAttributes_UsageType_Mask) >> kUSB_EPDesc_bmAttributes_UsageType_Shift;
		_SYNCTYPE = (ed->bmAttributes & kUSB_EPDesc_bmAttributes_SyncType_Mask) >> kUSB_EPDesc_bmAttributes_SyncType_Shift;
	}
	else
	{
		_USAGETYPE = 0;
		_SYNCTYPE = 0;
	}

    _address = address;
    _SPEED = speed;
	_DEVICE = device;
	_INTERFACE = interface;
	
	
	if ( _DEVICE )
	{
		OSNumber *	location = OSDynamicCast(OSNumber, _DEVICE->getProperty(kUSBDevicePropertyLocationID));
		if (location)
		{
			_LOCATIONID = location->unsigned32BitValue();
		}
	}

	USBTrace(kUSBTPipe,  kTPPipeInitToEndpoint, (uintptr_t)this, (uintptr_t)_controller, _endpoint.transferType, _endpoint.maxPacketSize);

	// Bring in workaround from the EHCI UIM for Bulk pipes that are high speed but report a MPS of 64:
    if ( (_SPEED == kUSBDeviceSpeedHigh) && (_endpoint.transferType == kUSBBulk) && (_endpoint.maxPacketSize != 512) ) 
    {	
		if ( _OUTOFSPECMPSOK == 0 )
		{
			// This shouldn't happen any more, this has been fixed.
			USBError(1, "Endpoint 0x%x of the USB device \"%s\" at location 0x%x:  converting Bulk MPS from %d to 512 (USB 2.0 Spec section 5.8.3)", ed->bEndpointAddress, _DEVICE ? _DEVICE->getName() : "Unnamed", (uint32_t)_LOCATIONID, _endpoint.maxPacketSize);
			_endpoint.maxPacketSize = 512;
		}
		else
		{
			USBLog(5, "IOUSBPipe[%p]::InitToEndpoint: High Speed Bulk pipe with %d MPS, but property says we should still use it.", this, _endpoint.maxPacketSize);
		}
    }
	
    err = _controller->OpenPipe(_address, speed, &_endpoint);
    
    if ((err == kIOReturnNoBandwidth) && (_endpoint.transferType == kUSBIsoc))
    {
		USBError(1,"There is not enough USB isochronous bandwidth to allow the device \"%s\" at location 0x%x to function in its current configuration (requested %d bytes for endpoint 0x%x )", _DEVICE ? _DEVICE->getName() : "Unnamed", (uint32_t)_LOCATIONID, _endpoint.maxPacketSize, ed->bEndpointAddress);
		
		_endpoint.maxPacketSize = 0;
		USBLog(6, "IOUSBPipe[%p]::InitToEndpoint  can't get bandwidth for isoc pipe - creating 0 bandwidth pipe", this);
		err = _controller->OpenPipe(_address, speed, &_endpoint);
    }
    
    if ( err != kIOReturnSuccess)
    {
        USBLog(3,"IOUSBPipe[%p]::InitToEndpoint Could not create pipe for endpoint (Addr: %d, numb: %d, type: %d).  Error 0x%x", this, _address, _endpoint.number, _endpoint.transferType, err);
        return false;
    }
    
	USBTrace_End( kUSBTPipe, kTPPipeInitToEndpoint, (uintptr_t)this, 0, 0, 0);
    
    return true;
}



//================================================================================================
//
//   ToEndpoint
//
//================================================================================================
//
IOUSBPipe *
IOUSBPipe::ToEndpoint(const IOUSBEndpointDescriptor *ed, IOUSBDevice * device, IOUSBController *controller, IOUSBInterface * interface)
{
	IOUSBPipe *	me = new IOUSBPipe;
   
    USBLog(1, "IOUSBPipe::ToEndpoint, obsolete method 3 called");
	USBLog(6, "IOUSBPipe[%p]::ToEndpoint device %p(%s), interface %p, ep: 0x%x,0x%x,%d,%d", me, device, device->getName(), interface, ed->bEndpointAddress, ed->bmAttributes, ed->wMaxPacketSize, ed->bInterval);
	
	if ( me && interface)
	{
		// If we allow out of spec MPS (with a property) set a flag
		me->_OUTOFSPECMPSOK = ( interface->getProperty(kUSBOutOfSpecMPSOK) == kOSBooleanTrue );
		if (me->_OUTOFSPECMPSOK)
		{
			USBLog(6,"IOUSBPipe[%p]::ToEndpoint Device reports Out of spec MPS property and is TRUE", me);
		}
	}

	if ( !me->InitToEndpoint(ed, device->GetSpeed(), device->GetAddress(), controller, device, interface) ) 
    {
        me->release();
        return NULL;
    }

	if ( me && me->_expansionData && me->_INTERFACE )
    {
        // If our interface has the CrossEndianCompatible property, set our boolean
        me->_CROSSENDIANCOMPATIBLE = ( me->_INTERFACE->getProperty(kIOUserClientCrossEndianCompatibleKey) == kOSBooleanTrue);
    }
	
    return me;
}



//================================================================================================
//
//   free
//
//================================================================================================
//
void 
IOUSBPipe::free()
{

    //  This needs to be the LAST thing we do, as it disposes of our "fake" member
    //  variables.
    //
    if (_expansionData)
    {
        IOFree(_expansionData, sizeof(ExpansionData));
        _expansionData = NULL;
    }

    super::free();
}


#pragma mark IOUSBPipe State

//================================================================================================
//
//   Abort
//
//================================================================================================
//
IOReturn 
IOUSBPipe::Abort(void)
{
	IOUSBPipeV2		*pipev2 = OSDynamicCast(IOUSBPipeV2, this);
	
	if ( pipev2 )
	{
		USBLog(5,"IOUSBPipe[%p]::AbortPipe  calling through to IOUSBPipeV2::Abort(kUSBAllStreams)",this);
		return pipev2->Abort(kUSBAllStreams);
	}
	else
	{
		USBLog(5,"IOUSBPipe[%p]::AbortPipe",this);
		if (_CORRECTSTATUS != 0)
		{
			USBLog(2, "IOUSBPipe[%p]::Abort setting status to 0", this);
		}
		_CORRECTSTATUS = 0;
		return _controller->AbortPipe(_address, &_endpoint);
	}
}


//================================================================================================
//
//   Reset
//
//================================================================================================
//
IOReturn 
IOUSBPipe::Reset(void)
{
    USBLog(7,"IOUSBPipe[%p]::Reset",this);
    if (_CORRECTSTATUS != 0)
	{
        USBLog(2, "IOUSBPipe[%p]::Reset setting status to 0", this);
	}
    _CORRECTSTATUS = 0;
    return _controller->ResetPipe(_address, &_endpoint);
}


//================================================================================================
//
//   ClosePipe
//
//================================================================================================
//
IOReturn 
IOUSBPipe::ClosePipe(void)
{
    // This method should be called by the provider of the IOUSBPipe (IOUSBInterface) right before it
    // releases the pipe.  We do it here instead of in the free method (as was previosuly done) because
    // _controller->ClosePipe() will end up deleting the endpoint, which is necessary.  If we left this
    // call in the free method then this wouldn't get called if someone had an extra retain on the pipe object.

    return _controller->ClosePipe(_address, &_endpoint);
}


//================================================================================================
//
//   ClearPipeStall
//
//================================================================================================
//
IOReturn 
IOUSBPipe::ClearStall(void)
{
    return ClearPipeStall(false);
}


IOReturn 
IOUSBPipe::ClearPipeStall(bool withDeviceRequest)
{
    IOReturn	err;
    
    USBLog(5,"IOUSBPipe[%p]::ClearPipeStall",this);
	
	if (_expansionData == NULL)
		return kIOReturnNoDevice;
	
    USBTrace_Start( kUSBTPipe, kTPPipeClearPipeStall, (uintptr_t)this, _address, _endpoint.number , _endpoint.transferType );

    if (_CORRECTSTATUS != 0)
	{
        USBLog(2, "IOUSBPipe[%p]::ClearPipeStall setting status to 0", this);
        USBTrace( kUSBTPipe, kTPPipeClearPipeStall, (uintptr_t)this, _address, _endpoint.number , 1 );
	}
	
    _CORRECTSTATUS = 0;
    
    if ( _endpoint.transferType == kUSBIsoc )
    {
        USBLog(2, "IOUSBPipe[%p]::ClearPipeStall Isoch pipes never stall.  Returning success", this);
        USBTrace( kUSBTPipe, kTPPipeClearPipeStall, (uintptr_t)this, _address, _endpoint.number , 2 );
        return kIOReturnSuccess;
    }
    
	if ( !_DEVICE )
	{
        USBLog(2, "IOUSBPipe[%p]::ClearPipeStall  no _DEVICE", this);
        USBTrace( kUSBTPipe, kTPPipeClearPipeStall, (uintptr_t)this, _address, _endpoint.number , 3 );
        return kIOReturnNotPermitted;
	}
	
	if ( _DEVICE->isInactive() )
	{
        USBLog(6, "IOUSBPipe[%p]::ClearPipeStall  _DEVICE is inActive(), so returning success", this);
        USBTrace( kUSBTPipe, kTPPipeClearPipeStall, (uintptr_t)this, _address, _endpoint.number , 4 );
        return kIOReturnSuccess;
	}
	
	// Make sure we don't go away and our device doesn't go away
	retain();
	_DEVICE->retain();
	
    err = _controller->ClearPipeStall(_address, &_endpoint);
    USBTrace( kUSBTPipe, kTPPipeClearPipeStall, (uintptr_t)this, _address, err , 5 );
	
	if (err == kIOUSBClearPipeStallNotRecursive)
	{
		USBLog(1,"IOUSBPipe[%p]::ClearPipeStall - tried to call recursively, err = %p", this, (void*)err);
        USBTrace( kUSBTPipe, kTPPipeClearPipeStall, (uintptr_t)this, _address, err , 6 );
	}
	
	
    if ((_DEVICE->GetSpeed() == kUSBDeviceSpeedHigh) || (_DEVICE->GetSpeed() == kUSBDeviceSpeedSuper))
    {
		USBLog(6,"IOUSBPipe[%p]::ClearPipeStall - HiSpeed or SuperSpeedDevice, no clear TT needed",this);
        USBTrace( kUSBTPipe, kTPPipeClearPipeStall, (uintptr_t)this, _address, _endpoint.number , 7 );
    }
    else if (!err && ( (_endpoint.transferType == kUSBBulk) || (_endpoint.transferType == kUSBControl) ) )
    {
		USBLog(5,"IOUSBPipe[%p]::ClearPipeStall Bulk or Control endpoint, clear TT",this);
		// Now, we need to tell our parent to issue a port reset to our port.  Our parent is an IOUSBDevice
		// that has a hub driver attached to it.  However, we don't know what kind of driver that is, so we
		// just send a message to all the clients of our parent.  The hub driver will be the only one that
		// should do anything with that message.
		//
		if ( _expansionData && _DEVICE && _DEVICE->_expansionData && _DEVICE->_expansionData->_usbPlaneParent )
		{
			IOUSBHubPortClearTTParam        params;
			UInt8						deviceAddress;		//<<0
			UInt8						endpointNum;		//<<8
			UInt8						endpointType;		//<<16 // As split transaction. 00 Control, 10 Bulk
			UInt8						in;					//<<24 // Direction, 1 = IN, 0 = OUT};
			IOUSBDevice *				usbParent = _DEVICE->_expansionData->_usbPlaneParent;
			
			if (usbParent)
			{
				usbParent->retain();
				params.portNumber = _DEVICE->_expansionData->_portNumber;
				deviceAddress = _DEVICE->GetAddress();
				endpointNum = _endpoint.number;
				if (_endpoint.transferType == kUSBControl)
				{
					endpointType = 0;	// As split transaction. 00 Control, 10 Bulk
					in = 0;				// Direction, 1 = IN, 0 = OUT, not used for control
				}
				else
				{
					endpointType = 2;		// As split transaction. 00 Control, 10 Bulk
					if (_endpoint.direction == kUSBIn)
					{
						in = 1;			// Direction, 1 = IN, 0 = OUT, not used for control
					}
					else
					{
						in = 0;
					}
				}
				params.options = deviceAddress + (endpointNum <<8) + (endpointType << 16) + (in << 24);
				
				
				USBLog(6, "IOUSBPipe[%p]::ClearPipeStall  calling device messageClients (kIOUSBMessageHubPortClearTT) with options: 0x%x", this, (uint32_t)params.options);
                USBTrace( kUSBTPipe, kTPPipeClearPipeStall, (uintptr_t)this, params.portNumber, params.options , 8 );
				(void) usbParent->messageClients(kIOUSBMessageHubPortClearTT, &params, sizeof(params));
				usbParent->release();
			}
		}
        else
        {
            USBTrace( kUSBTPipe, kTPPipeClearPipeStall, (uintptr_t)this, _address, _endpoint.number , 9 );

        }
    }
    else
    {
		USBLog(5,"IOUSBPipe[%p]::ClearPipeStall Int or Isoc endpoint, don't clear TT (or err: 0x%x)",this, err);
    }
    
    if (!err && withDeviceRequest)
    {
		IOUSBDevRequest	request;
        IOUSBCompletion	tap;
		
		// The action of IOUSBSyncCompletion will tell the USL that this is a sync transfer
		//
        tap.target = NULL;
        tap.action = &IOUSBSyncCompletion;
        tap.parameter = &request.wLenDone;
		
        USBLog(7,"IOUSBPipe[%p]::ClearPipeStall - sending request to the device", this);
		request.bmRequestType = USBmakebmRequestType(kUSBOut, kUSBStandard, kUSBEndpoint);
		request.bRequest = kUSBRqClearFeature;
		request.wValue = kUSBFeatureEndpointStall;
		request.wIndex = _endpoint.number | ((_endpoint.direction == kUSBIn) ? 0x80 : 0);
		request.wLenDone = request.wLength = 0;
		request.pData = NULL;
		
		// send the request to pipe zero
		err = _controller->DeviceRequest(&request, &tap, _address, 0, kUSBDefaultControlNoDataTimeoutMS, kUSBDefaultControlCompletionTimeoutMS);
        USBTrace( kUSBTPipe, kTPPipeClearPipeStall, (uintptr_t)this, _address, err , 10 );
		
    }
	
	if (_expansionData && _DEVICE)
		_DEVICE->release();
	
	release();

    USBTrace_End( kUSBTPipe, kTPPipeClearPipeStall, _address, _endpoint.number , _endpoint.transferType, err );
    return err;
}


//================================================================================================
//
//   SetPipePolicy
//
//================================================================================================
//
IOReturn
IOUSBPipe::SetPipePolicy(UInt16 maxPacketSize, UInt8 maxInterval)
{
	UInt8		oldInterval = _endpoint.interval;
	UInt16		oldsize = _endpoint.maxPacketSize;
    IOReturn 	err = kIOReturnSuccess;
	bool		change = false;
    
	USBLog(6, "IOUSBPipe[%p]::SetPipePolicy (addr %d:%d dir: %d, type: %d) - maxPacketSize: %d, maxInterval: %d", this, _address, _endpoint.number, _endpoint.direction, _endpoint.transferType, maxPacketSize, maxInterval);
	
    switch (_endpoint.transferType)
    {
		case kUSBIsoc:
			if (maxPacketSize <= _INTERFACE->CalculateFullMaxPacketSize((IOUSBEndpointDescriptor*)_descriptor, NULL))
			{
				USBLog(6, "IOUSBPipe[%p]::SetPipePolicy - trying to change isoch pipe from %d to %d bytes", this, oldsize, maxPacketSize);
				_endpoint.maxPacketSize = maxPacketSize;
				
				// OpenPipe with Isoch pipes which already exist will try to change the maxpacketSize in the pipe.
				// the speed param below is actually unused for Isoc pipes
				err = _controller->OpenPipe(_address, _SPEED, &_endpoint);
				if (err)
				{
					USBLog(2, "IOUSBPipe[%p]::SetPipePolicy - new OpenPipe failed with 0x%x (%s) - returning old settings", this, err, USBStringFromReturn(err));
					_endpoint.maxPacketSize = oldsize;
				}
			}
			else
			{
				USBLog(2, "IOUSBPipe[%p]::SetPipePolicy - requested size (%d) larger than maxPacketSize in descriptor (%d)", this, maxPacketSize, _endpoint.maxPacketSize);
				err = kIOReturnBadArgument;
			}
			break;
			
		case kUSBInterrupt:
			if ( maxInterval == 0 )
			{
				USBLog(2, "IOUSBPipe[%p]::SetPipePolicy - requested a maxInterval of 0, which is not legal", this);
				err = kIOReturnBadArgument;
			}
			else
			{
				if ( maxInterval != _endpoint.interval )
				{    
					USBLog(3, "IOUSBPipe[%p]::SetPipePolicy - trying to change interrupt pollingRate from %d to %d", this, oldInterval, maxInterval);
					_endpoint.interval = maxInterval;
					change = true;
				}
				else
				{
					USBLog(6, "IOUSBPipe[%p]::SetPipePolicy - requested maxInterval size is the same as before", this);
				}
				
				if (maxPacketSize <= _INTERFACE->CalculateFullMaxPacketSize((IOUSBEndpointDescriptor*)_descriptor, NULL))
				{
					USBLog(3, "IOUSBPipe[%p]::SetPipePolicy - trying to change interrupt maxPacketSize from %d to %d bytes", this, oldsize, maxPacketSize);
					_endpoint.maxPacketSize = maxPacketSize;
					change = true;
				}
				else
				{
					USBLog(2, "IOUSBPipe[%p]::SetPipePolicy - requested size (%d) larger than maxPacketSize in descriptor (%d)", this, maxPacketSize, _endpoint.maxPacketSize);
					err = kIOReturnBadArgument;
					change = false;
				}
				
				if ( change  )
				{
					err = _controller->OpenPipe(_address, _SPEED, &_endpoint);
					if (err)
					{
						USBLog(2, "IOUSBPipe[%p]::SetPipePolicy - changing parameters failed with err = 0x%x (%s), trying to restore old settings", this, err, USBStringFromReturn(err));
						_endpoint.interval = oldInterval;
						_endpoint.maxPacketSize = oldsize;
						
						err = _controller->OpenPipe(_address, _SPEED, &_endpoint);
						if (err)
						{
							USBLog(2, "IOUSBPipe[%p]::SetPipePolicy - changing parameters failed second time with error 0x%x (%s)", this, err, USBStringFromReturn(err));
						}
					}
				}
			}
			break;
			
		default:
			USBLog(2, "IOUSBPipe[%p]::SetPipePolicy - wrong type of pipe - returning kIOReturnBadArgument", this);
			err = kIOReturnBadArgument;
    }
	
    return err;
}


#pragma mark Bulk Read

//================================================================================================
//
//   Read (Bulk)
//
//================================================================================================
//
IOReturn 
IOUSBPipe::Read(IOMemoryDescriptor *buffer, IOUSBCompletion *completion, IOByteCount *bytesRead)
{
    USBLog(7, "IOUSBPipe[%p]::Read #1", this);
    return Read(buffer, 0, 0, completion, bytesRead);
}


IOReturn 
IOUSBPipe::Read(IOMemoryDescriptor *buffer, UInt32 noDataTimeout, UInt32 completionTimeout, IOUSBCompletion *completion, IOByteCount *bytesRead)
{
    USBLog(7, "IOUSBPipe[%p]::Read #2", this);
	
    // Validate that there is a buffer so that we can call getLength on it
    if (!buffer)
    {
        USBLog(5, "IOUSBPipe[%p]::Read - NULL buffer!", this);
		return kIOReturnBadArgument;
    }
    
    return Read(buffer, noDataTimeout, completionTimeout, buffer->getLength(), completion, bytesRead);
}


IOReturn 
IOUSBPipe::Read(IOMemoryDescriptor *buffer, UInt32 noDataTimeout, UInt32 completionTimeout, IOByteCount reqCount, IOUSBCompletion *completion, IOByteCount *bytesRead)
{
	IOUSBPipeV2		*pipev2 = OSDynamicCast(IOUSBPipeV2, this);
	
	if ( pipev2 )
	{
		return pipev2->Read(0, buffer, noDataTimeout, completionTimeout, reqCount, completion, bytesRead);
	}
	else
	{
		IOReturn	err = kIOReturnSuccess;
		
		USBLog(7, "IOUSBPipe[%p]::Read #3 (addr %d:%d type %d) - reqCount = %qd", this, _address, _endpoint.number , _endpoint.transferType, (uint64_t)reqCount);
		USBTrace_Start( kUSBTPipe, kTPBulkPipeRead, _address, _endpoint.number , _endpoint.transferType, reqCount );
		
		if ((_endpoint.transferType != kUSBBulk) && (noDataTimeout || completionTimeout))
		{
			USBLog(5, "IOUSBPipe[%p]::Read - bad arguments:  (EP type: %d != kUSBBulk(%d)) && ( dataTimeout: %d || completionTimeout: %d)", this, _endpoint.transferType, kUSBBulk, (uint32_t)noDataTimeout, (uint32_t)completionTimeout);
			return kIOReturnBadArgument;
		}
		
		if (!buffer || (buffer->getLength() < reqCount))
		{
			USBLog(5, "IOUSBPipe[%p]::Write - bad buffer: (buffer %p) || ( length %qd < reqCount %qd)", this, buffer, buffer ? (uint64_t)buffer->getLength() : 0, (uint64_t)reqCount);
			return kIOReturnBadArgument;
		}
		
		if (_CORRECTSTATUS == kIOUSBPipeStalled)
		{
			USBLog(2, "IOUSBPipe[%p]::Read - invalid read on a stalled pipe", this);
			return kIOUSBPipeStalled;
		}
		
		if (completion == NULL)
		{
			// put in our own completion routine if none was specified to
			// fake synchronous operation
			IOUSBCompletion	tap;
			
			if (bytesRead)
				*bytesRead = reqCount;
			
			// The action of IOUSBSyncCompletion will tell the USL that this is a sync transfer
			//
			tap.target = NULL;
			tap.action = &IOUSBSyncCompletion;
			tap.parameter = bytesRead;
			
			err = _controller->Read(buffer, _address, &_endpoint, &tap, noDataTimeout, completionTimeout, reqCount);
			if (err != kIOReturnSuccess)
			{
				// any err coming back in the callback indicates a stalled pipe
            if (err && (err != kIOUSBTransactionTimeout) && (err != kIOReturnAborted))
				{
					USBLog(2, "IOUSBPipe[%p]::Read(sync)  returned 0x%x (%s) - stalling pipe", this, err, USBStringFromReturn(err));
					_CORRECTSTATUS = kIOUSBPipeStalled;
				}
			}
		}
		else
		{
			if (completion->action == NULL)
			{
				USBLog(1, "IOUSBPipe[%p]::Read - completion has NULL action - returning kIOReturnBadArgument(%p)", this, (void*)kIOReturnBadArgument);
				USBTrace(kUSBTPipe,  kTPBulkPipeRead, (uintptr_t)this, kIOReturnBadArgument, 0, 1 );
				return kIOReturnBadArgument;
			}
			err = _controller->Read(buffer, _address, &_endpoint, completion, noDataTimeout, completionTimeout, reqCount);
		}
		
		if (err == kIOUSBPipeStalled)
		{
			USBLog(2, "IOUSBPipe[%p]::Read  - controller returned stalled pipe, changing status", this);
			_CORRECTSTATUS = kIOUSBPipeStalled;
		}
		
 		if ( err == kIOUSBTransactionTimeout && _endpoint.transferType != kUSBInterrupt )
		{
			USBLog(1, "IOUSBPipe[%p]::Read - Location: 0x%x returned a kIOUSBTransactionTimeout", this, (uint32_t)_DEVICE->_expansionData->_locationID);
		}
		
		USBTrace_End( kUSBTPipe, kTPBulkPipeRead, (uintptr_t)this, _address, _endpoint.number, err);
		
		return(err);
	}
}

IOReturn
IOUSBPipe::Read(IOMemoryDescriptor *buffer, UInt32 noDataTimeout, UInt32 completionTimeout, IOByteCount reqCount, IOUSBCompletionWithTimeStamp *completionWithTimeStamp, IOByteCount *bytesRead)
{
    IOReturn                err = kIOReturnSuccess;
    IOUSBControllerV2  *    controllerV2;
    
    controllerV2 = OSDynamicCast(IOUSBControllerV2, _controller);
    if ( controllerV2 == NULL )
    {
        USBLog(2,"IOUSBPipe[%p]:Read #4 -- Requested a Read with time stamp, but this IOUSBController does not support it", this);
        return kIOReturnUnsupported;
    }
    
    USBLog(7, "IOUSBPipe[%p]::Read #4 (addr %d:%d type %d) - reqCount = %qd", this, _address, _endpoint.number , _endpoint.transferType, (uint64_t)reqCount);
	USBTrace_Start( kUSBTPipe, kTPIBulkReadTS, _address, _endpoint.number , _endpoint.transferType, reqCount );
	
    if ((_endpoint.transferType != kUSBBulk) && (noDataTimeout || completionTimeout))
    {
        USBLog(5, "IOUSBPipe[%p]::Read #4 - bad arguments:  (EP type: %d != kUSBBulk(%d)) && ( dataTimeout: %d || completionTimeout: %d)", this, _endpoint.transferType, kUSBBulk, (uint32_t)noDataTimeout, (uint32_t)completionTimeout);
        return kIOReturnBadArgument;
    }
    
    if (!buffer || (buffer->getLength() < reqCount))
    {
        USBLog(5, "IOUSBPipe[%p]::Read #4- bad buffer: (buffer %p) || ( length %qd < reqCount %qd)", this, buffer, buffer ? (uint64_t)buffer->getLength() : 0, (uint64_t)reqCount);
        return kIOReturnBadArgument;
    }
	
    if (_CORRECTSTATUS == kIOUSBPipeStalled)
    {
        USBLog(2, "IOUSBPipe[%p]::Read #4 - invalid read on a stalled pipe", this);
        return kIOUSBPipeStalled;
    }
	
    if (completionWithTimeStamp == NULL)
    {
        // put in our own completion routine if none was specified to
        // fake synchronous operation
        IOUSBCompletion	tap;
		
        if (bytesRead)
            *bytesRead = reqCount;
		
        // The action of IOUSBSyncCompletion will tell the USL that this is a sync transfer
        //
        tap.target = NULL;
        tap.action = &IOUSBSyncCompletion;
        tap.parameter = bytesRead;
		
        err = controllerV2->ReadV2(buffer, _address, &_endpoint, (IOUSBCompletionWithTimeStamp *) &tap, noDataTimeout, completionTimeout, reqCount);
        if (err != kIOReturnSuccess)
        {
            // any err coming back in the callback indicates a stalled pipe
            if (err && (err != kIOUSBTransactionTimeout) && (err != kIOReturnAborted))
            {
                USBLog(2, "IOUSBPipe[%p]::Read  - i/o err (0x%x) on sync call - stalling pipe", this, err);
                _CORRECTSTATUS = kIOUSBPipeStalled;
            }
        }
    }
    else
    {
		if (completionWithTimeStamp->action == NULL)
		{
			USBLog(1, "IOUSBPipe[%p]::Read - completionWithTimeStamp has NULL action - returning kIOReturnBadArgument(%p)", this, (void*)kIOReturnBadArgument);
			USBTrace(kUSBTPipe,  kTPIBulkReadTS, (uintptr_t)this, kIOReturnBadArgument, 0, 0);
			return kIOReturnBadArgument;
		}
        err = controllerV2->ReadV2(buffer, _address, &_endpoint, completionWithTimeStamp, noDataTimeout, completionTimeout, reqCount);
    }
	
    if (err == kIOUSBPipeStalled)
    {
        USBLog(2, "IOUSBPipe[%p]::Read  - controller returned stalled pipe, changing status", this);
        _CORRECTSTATUS = kIOUSBPipeStalled;
    }
	
	if ( err == kIOUSBTransactionTimeout )
	{
		USBLog(1, "IOUSBPipe[%p]::Read #4 - Location: 0x%x returned a kIOUSBTransactionTimeout", this, (uint32_t)_DEVICE->_expansionData->_locationID);
	}
	USBTrace_End( kUSBTPipe, kTPIBulkReadTS, (uintptr_t)this, _address, _endpoint.number, err);
	
    return(err);
}


#pragma mark Bulk Write
//================================================================================================
//
//   Write (Bulk)
//
//================================================================================================
//
IOReturn 
IOUSBPipe::Write(IOMemoryDescriptor *buffer, IOUSBCompletion *completion)
{
    USBLog(7, "IOUSBPipe[%p]::Write #1", this);
    return Write(buffer, 0, 0, completion);
}


IOReturn 
IOUSBPipe::Write(IOMemoryDescriptor *buffer, UInt32 noDataTimeout, UInt32 completionTimeout, IOUSBCompletion *completion)
{
    USBLog(7, "IOUSBPipe[%p]::Write #2", this);
    // Validate that there is a buffer so that we can call getLength on it
    if (!buffer)
    {
        USBLog(5, "IOUSBPipe[%p]::Write - NULL buffer!", this);
        return kIOReturnBadArgument;
    }
    
    return Write(buffer, noDataTimeout, completionTimeout, buffer->getLength(), completion);
}

IOReturn 
IOUSBPipe::Write(IOMemoryDescriptor *buffer, UInt32 noDataTimeout, UInt32 completionTimeout, IOByteCount reqCount, IOUSBCompletion *completion)
{
	IOUSBPipeV2		*pipev2 = OSDynamicCast(IOUSBPipeV2, this);
	
	if ( pipev2 )
	{
		return pipev2->Write(0, buffer, noDataTimeout, completionTimeout, reqCount, completion);
	}
	else
	{
		IOReturn	err = kIOReturnSuccess;
		
		USBLog(7, "IOUSBPipe[%p]::Write #3 (addr %d:%d type %d) - reqCount = %qd", this, _address, _endpoint.number , _endpoint.transferType, (uint64_t)reqCount);
		USBTrace_Start( kUSBTPipe, kTPBulkPipeWrite, _address, _endpoint.number , _endpoint.transferType, reqCount );
		
		if ((_endpoint.transferType != kUSBBulk) && (noDataTimeout || completionTimeout))
		{
			USBLog(5, "IOUSBPipe[%p]::Write - bad arguments:  (EP type: %d != kUSBBulk(%d)) && ( dataTimeout: %d || completionTimeout: %d)", this, _endpoint.transferType, kUSBBulk, (uint32_t)noDataTimeout, (uint32_t)completionTimeout);
			return kIOReturnBadArgument;
		}
		
		if (!buffer || (buffer->getLength() < reqCount))
		{
			USBLog(5, "IOUSBPipe[%p]::Write - bad buffer: (buffer %p) || ( length %qd < reqCount %qd)", this, buffer, buffer ? (uint64_t)buffer->getLength() : 0, (uint64_t)reqCount);
			return kIOReturnBadArgument;
		}
		
		if (_CORRECTSTATUS == kIOUSBPipeStalled)
		{
			USBLog(2, "IOUSBPipe[%p]::Write - invalid write on a stalled pipe", this);
			return kIOUSBPipeStalled;
		}
		
		if (completion == NULL)
		{
			// put in our own completion routine if none was specified to
			// fake synchronous operation
			IOUSBCompletion	tap;
			
			// The action of IOUSBSyncCompletion will tell the USL that this is a sync transfer
			//
			tap.target = NULL;
			tap.action = &IOUSBSyncCompletion;
			tap.parameter = NULL;
			
			err = _controller->Write(buffer, _address, &_endpoint, &tap, noDataTimeout, completionTimeout, reqCount);
			if (err != kIOReturnSuccess)
			{
				// any err coming back in the callback indicates a stalled pipe
            if (err && (err != kIOUSBTransactionTimeout) && (err != kIOReturnAborted))
				{
					USBLog(2, "IOUSBPipe[%p]::Write(sync)  returned 0x%x (%s) - stalling pipe", this, err, USBStringFromReturn(err));
					_CORRECTSTATUS = kIOUSBPipeStalled;
				}
			}
		}
		else
		{
			if (completion->action == NULL)
			{
				USBLog(1, "IOUSBPipe[%p]::Write - completion has NULL action - returning kIOReturnBadArgument(%p)", this, (void*)kIOReturnBadArgument);
				USBTrace(kUSBTPipe,  kTPBulkPipeWrite, (uintptr_t)this, kIOReturnBadArgument, 0, 0 );
				return kIOReturnBadArgument;
			}
			err = _controller->Write(buffer, _address, &_endpoint, completion, noDataTimeout, completionTimeout, reqCount);
		}
		
		if (err == kIOUSBPipeStalled)
		{
			USBLog(2, "IOUSBPipe[%p]::Write - controller returned stalled pipe, changing status", this);
			_CORRECTSTATUS = kIOUSBPipeStalled;
		}
		
 		if ( err == kIOUSBTransactionTimeout )
		{
			USBLog(1, "IOUSBPipe[%p]::Write - Location: 0x%x returned a kIOUSBTransactionTimeout", this, (uint32_t)_DEVICE->_expansionData->_locationID);
		}
		USBTrace_End( kUSBTPipe, kTPBulkPipeWrite, (uintptr_t)this, _address, _endpoint.number, err);
		
		return err;
	}
}


#pragma mark Isochronous 
//================================================================================================
//
//   Read (Isoch)
//
//================================================================================================
//
IOReturn 
IOUSBPipe::Read(IOMemoryDescriptor * buffer, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *pFrames, IOUSBIsocCompletion *completion)
{
    IOReturn	err = kIOReturnSuccess;
	
	// USBTrace_Start( kUSBTPipe, kTPIsocPipeRead, (uintptr_t)this, (uintptr_t)buffer, frameStart, numFrames );
	
    if (_CORRECTSTATUS == kIOUSBPipeStalled)
    {
        USBLog(2, "IOUSBPipe[%p]::Read - invalid read on a stalled isoch pipe", this);
        return kIOUSBPipeStalled;
    }
	
    if (_endpoint.maxPacketSize == 0)
    {
        USBLog(2, "IOUSBPipe[%p]::Read - no bandwidth on an isoc pipe", this);
        return kIOReturnNoBandwidth;
    }
	
	// The following is a hack to tell the UIM that this request is coming from a Rosetta client.  We set the high bit of the endpoint transfer type here
	// and in IsocIO we will clear it and set a flag in the IOUSBCommand
	if ( _CROSSENDIANCOMPATIBLE )
		_endpoint.direction |= 0x80;
	
    if (completion == NULL)
    {
        // put in our own completion routine if none was specified to
        // fake synchronous operation
        IOUSBIsocCompletion	tap;
		
        // The action of IOUSBSyncCompletion will tell the USL that this is a sync transfer
        //
        tap.target = NULL;
        tap.action = &IOUSBSyncIsoCompletion;
        tap.parameter = NULL;
		
		USBLog(3, "IOUSBPipe[%p]::Read Sync (Isoc) completion: %p", this, tap.action);
		err = _controller->IsocIO(buffer, frameStart, numFrames, pFrames, _address, &_endpoint, &tap);
		
    }
    else
    {
		if (completion->action == NULL)
		{
			USBLog(1, "IOUSBPipe[%p]::Read - completion has NULL action - returning kIOReturnBadArgument(%p)", this, (void*)kIOReturnBadArgument);
			USBTrace(kUSBTPipe,  kTPIsocPipeRead, (uintptr_t)this, kIOReturnBadArgument, 0, 1 );
			return kIOReturnBadArgument;
		}
		err = _controller->IsocIO(buffer, frameStart, numFrames, pFrames, _address, &_endpoint, completion);
    }
	
	// USBTrace_End( kUSBTPipe, kTPIsocPipeRead, (uintptr_t)this, err);
	
    return err;
}


//================================================================================================
//
//   Write (Isoch)
//
//================================================================================================
//
IOReturn 
IOUSBPipe::Write(IOMemoryDescriptor * buffer, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *pFrames, IOUSBIsocCompletion *completion)
{
    IOReturn	err = kIOReturnSuccess;
	
	// USBTrace_Start( kUSBTPipe, kTPIsocPipeWrite, (uintptr_t)this, (uintptr_t)buffer, frameStart, numFrames );
	
    if (_CORRECTSTATUS == kIOUSBPipeStalled)
    {
        USBLog(2, "IOUSBPipe[%p]::Write - invalid write on a stalled isoch pipe", this);
        return kIOUSBPipeStalled;
    }
	
    if (_endpoint.maxPacketSize == 0)
    {
        USBLog(2, "IOUSBPipe[%p]::Write - no bandwidth on an isoc pipe", this);
        return kIOReturnNoBandwidth;
    }
	
	// The following is a hack to tell the UIM that this request is coming from a Rosetta client.  We set the high bit of the endpoint transfer type here
	// and in IsocIO we will clear it and set a flag in the IOUSBCommand
	if ( _CROSSENDIANCOMPATIBLE )
		_endpoint.direction |= 0x80;
	
    if (completion == NULL)
    {
        // put in our own completion routine if none was specified to
        // fake synchronous operation
        IOUSBIsocCompletion	tap;
		
        // The action of IOUSBSyncCompletion will tell the USL that this is a sync transfer
        //
        tap.target = NULL;
        tap.action = &IOUSBSyncIsoCompletion;
        tap.parameter = NULL;
		
        USBLog(3, "IOUSBPipe[%p]::Write Sync (Isoc) completion: %p", this, tap.action);
        err = _controller->IsocIO(buffer, frameStart, numFrames, pFrames, _address, &_endpoint, &tap);
		
    }
    else
    {
		if (completion->action == NULL)
		{
			USBLog(1, "IOUSBPipe[%p]::Write - completion has NULL action - returning kIOReturnBadArgument(%p)", this, (void*)kIOReturnBadArgument);
			USBTrace(kUSBTPipe,  kTPIsocPipeWrite, (uintptr_t)this, kIOReturnBadArgument, 0, 1 );
			return kIOReturnBadArgument;
		}
        err = _controller->IsocIO(buffer, frameStart, numFrames, pFrames, _address, &_endpoint, completion);
    }
	
	// USBTrace_End( kUSBTPipe, kTPIsocPipeWrite, (uintptr_t)this, err);
	
    return(err);
}


#pragma mark Low Latency Isochronous
//================================================================================================
//
//   Read 
//
//	 Isochronous read with frame list updated at hardware interrupt time
//
//================================================================================================
//
IOReturn 
IOUSBPipe::Read(IOMemoryDescriptor *	buffer,
				UInt64 frameStart, UInt32 numFrames, IOUSBLowLatencyIsocFrame *pFrames,
				IOUSBLowLatencyIsocCompletion *	completion, UInt32 updateFrequency)
{
    IOReturn	err = kIOReturnSuccess;
	
    USBLog(7, "IOUSBPipe[%p]::Read (Low Latency Isoc) buffer: %p, completion: %p, numFrames: %d, update: %d", this, buffer, completion, (uint32_t)numFrames, (uint32_t)updateFrequency);
	// USBTrace_Start( kUSBTPipe, kTPIsocPipeReadLL, (uintptr_t)buffer, (uintptr_t)completion, (uint32_t)numFrames, (uint32_t)updateFrequency );
	
    if (_CORRECTSTATUS == kIOUSBPipeStalled)
    {
        USBLog(2, "IOUSBPipe[%p]::Read (Low Latency Isoc) - invalid read on a stalled low latency isoch pipe", this);
        return kIOUSBPipeStalled;
    }
	
    if (_endpoint.maxPacketSize == 0)
    {
        USBLog(2, "IOUSBPipe[%p]::Read (Low Latency Isoc) - no bandwidth on an low latency isoc pipe", this);
        return kIOReturnNoBandwidth;
    }
	
	// The following is a hack to tell the UIM that this request is coming from a Rosetta client.  We set the high bit of the endpoint transfer type here
	// and in IsocIO we will clear it and set a flag in the IOUSBCommand
	if ( _CROSSENDIANCOMPATIBLE )
		_endpoint.direction |= 0x80;
	
    if (completion == NULL)
    {
        // put in our own completion routine if none was specified to
        // fake synchronous operation
        //
        IOUSBLowLatencyIsocCompletion	tap;
		
        // The action of IOUSBSyncCompletion will tell the USL that this is a sync transfer
        //
        tap.target = NULL;
        tap.action = (IOUSBLowLatencyIsocCompletionAction) &IOUSBSyncIsoCompletion;
        tap.parameter = NULL;
		
        USBLog(3, "IOUSBPipe[%p]::Read Sync (Low Latency Isoc) completion: %p", this, tap.action);
        err = _controller->IsocIO(buffer, frameStart, numFrames, pFrames, _address, &_endpoint, &tap, updateFrequency);
    }
    else
    {
		if (completion->action == NULL)
		{
			USBLog(1, "IOUSBPipe[%p]::Read - completion has NULL action - returning kIOReturnBadArgument(%p)", this, (void*)kIOReturnBadArgument);
			USBTrace(kUSBTPipe,  kTPIsocPipeReadLL, (uintptr_t)this, kIOReturnBadArgument, 0, 0 );
			return kIOReturnBadArgument;
		}
        err = _controller->IsocIO(buffer, frameStart, numFrames, pFrames, _address, &_endpoint, completion, updateFrequency);
    }
	
	// USBTrace_End( kUSBTPipe, kTPIsocPipeReadLL, (uintptr_t)this, err);
	
    return err;
}                          


//================================================================================================
//
//   Write 
//
//	 Isochronous read with frame list updated at hardware interrupt time
//
//================================================================================================
//
IOReturn 
IOUSBPipe::Write(IOMemoryDescriptor * buffer, UInt64 frameStart, UInt32 numFrames, IOUSBLowLatencyIsocFrame *pFrames, IOUSBLowLatencyIsocCompletion *completion, UInt32 updateFrequency)
{
    IOReturn	err = kIOReturnSuccess;
	
    USBLog(7, "IOUSBPipe[%p]::Write (Low Latency Isoc) buffer: %p, completion: %p, numFrames: %d, update: %d", this, buffer, completion, (uint32_t)numFrames, (uint32_t)updateFrequency);
	//USBTrace_Start( kUSBTPipe, kTPIsocPipeWriteLL, (uintptr_t)buffer, (uintptr_t)completion, (uint32_t)numFrames, (uint32_t)updateFrequency);
	
    if (_CORRECTSTATUS == kIOUSBPipeStalled)
    {
        USBLog(2, "IOUSBPipe[%p]::Write (Low Latency Isoc) - invalid write on a stalled isoch pipe", this);
        return kIOUSBPipeStalled;
    }
	
    if (_endpoint.maxPacketSize == 0)
    {
        USBLog(2, "IOUSBPipe[%p]::Write (Low Latency Isoc) - no bandwidth on an isoc pipe", this);
        return kIOReturnNoBandwidth;
    }
	
	// The following is a hack to tell the UIM that this request is coming from a Rosetta client.  We set the high bit of the endpoint transfer type here
	// and in IsocIO we will clear it and set a flag in the IOUSBCommand
	if ( _CROSSENDIANCOMPATIBLE )
		_endpoint.direction |= 0x80;
	
    if (completion == NULL)
    {
        // put in our own completion routine if none was specified to
        // fake synchronous operation
        IOUSBLowLatencyIsocCompletion	tap;
		
		// The action of IOUSBSyncCompletion will tell the USL that this is a sync transfer
		//
        tap.target = NULL;
        tap.action = (IOUSBLowLatencyIsocCompletionAction)&IOUSBSyncIsoCompletion;
        tap.parameter = NULL;
		
        USBLog(3, "IOUSBPipe[%p]::Write Sync (Low Latency Isoc) completion: %p", this, tap.action);
        err = _controller->IsocIO(buffer, frameStart, numFrames, pFrames, _address, &_endpoint, &tap, updateFrequency);
    }
    else
    {
		if (completion->action == NULL)
		{
			USBLog(1, "IOUSBPipe[%p]::Write - completion has NULL action - returning kIOReturnBadArgument(%p)", this, (void*)kIOReturnBadArgument);
			USBTrace(kUSBTPipe,  kTPIsocPipeWriteLL, (uintptr_t)this, kIOReturnBadArgument, 0, 0);
			return kIOReturnBadArgument;
		}
        err = _controller->IsocIO(buffer, frameStart, numFrames, pFrames, _address, &_endpoint, completion, updateFrequency);
    }
	
	// USBTrace_End( kUSBTPipe, kTPIsocPipeWriteLL, (uintptr_t)this);
	
    return err;
}




#pragma mark Control Requests

//================================================================================================
//
//   ControlRequest 
//
//================================================================================================
//
IOReturn 
IOUSBPipe::ControlRequest(IOUSBDevRequest *request, IOUSBCompletion *completion)
{
    return ControlRequest(request, _endpoint.number ? 0 : kUSBDefaultControlNoDataTimeoutMS, _endpoint.number ? 0 : kUSBDefaultControlCompletionTimeoutMS, completion);
}

IOReturn 
IOUSBPipe::ControlRequest(IOUSBDevRequest *request, UInt32 noDataTimeout, UInt32 completionTimeout, IOUSBCompletion *completion)
{
    IOReturn	err = kIOReturnSuccess;

	// USBTrace_Start( kUSBTPipe, kTPPipeControlRequest, request->bmRequestType,  request->bRequest, request->wValue, request->wIndex );

	if (request == NULL)
		return kIOReturnBadArgument;
	
	if ( _DEVICE && request && (_DEVICE->GetVendorID() == kAppleVendorID) && (request->bmRequestType == USBmakebmRequestType(kUSBOut, kUSBVendor, kUSBDevice)) && (request->bRequest == 0x40) && (request->wLength == 0) )
	{
		USBLog(1, "IOUSBPipe[%p]::ControlRequest  Possible charging command sent to %s @ 0x%x, operating: %d extra, sleep: %d", this, _DEVICE->getName(), (uint32_t)_DEVICE->_expansionData->_locationID, request->wIndex, request->wValue );
		USBTrace(kUSBTPipe,  kTPPipeControlRequest, (uintptr_t)this, (uintptr_t)_DEVICE->_expansionData->_locationID, (uintptr_t)(request->wIndex<< 16 | request->wValue), 1 );
	}

    if (completion == NULL)
    {
        // put in our own completion routine if none was specified to
        // fake synchronous operation
        IOUSBCompletion	tap;

        request->wLenDone = request->wLength;

		// The action of IOUSBSyncCompletion will tell the USL that this is a sync transfer
		//
        tap.target = NULL;
        tap.action = &IOUSBSyncCompletion;
        tap.parameter = &request->wLenDone;

        err = _controller->DeviceRequest(request, &tap, _address, _endpoint.number, noDataTimeout, completionTimeout);

    }
    else
    {
		if (completion->action == NULL)
		{
			USBLog(1, "IOUSBPipe[%p]::ControlRequest - completion has NULL action - returning kIOReturnBadArgument(%p)", this, (void*)kIOReturnBadArgument);
			USBTrace(kUSBTPipe,  kTPPipeControlRequestMemDesc, (uintptr_t)this, kIOReturnBadArgument, 0, 2 );
			return kIOReturnBadArgument;
		}
        err = _controller->DeviceRequest(request, completion, _address, _endpoint.number, noDataTimeout, completionTimeout);
    }
	// USBTrace_End( kUSBTPipe, kTPPipeControlRequest, (uintptr_t)this, err);

    return(err);
}


IOReturn 
IOUSBPipe::ControlRequest(IOUSBDevRequestDesc *request, IOUSBCompletion	*completion)
{
    return ControlRequest(request, _endpoint.number ? 0 : kUSBDefaultControlNoDataTimeoutMS, _endpoint.number ? 0 : kUSBDefaultControlCompletionTimeoutMS, completion);
}


IOReturn 
IOUSBPipe::ControlRequest(IOUSBDevRequestDesc *request, UInt32 noDataTimeout, UInt32 completionTimeout, IOUSBCompletion	*completion)
{
    IOReturn	err = kIOReturnSuccess;
	
	// USBTrace_Start( kUSBTPipe, kTPPipeControlRequestMemDesc, request->bmRequestType,  request->bRequest, request->wValue, request->wIndex );
	
	if (request == NULL)
		return kIOReturnBadArgument;
	
	if ( _DEVICE && request && (_DEVICE->GetVendorID() == kAppleVendorID) && (request->bmRequestType == USBmakebmRequestType(kUSBOut, kUSBVendor, kUSBDevice)) && (request->bRequest == 0x40) && (request->wLength == 0) )
	{
		USBLog(1, "IOUSBPipe[%p]::ControlRequest  Possible charging command sent to %s @ 0x%x, operating: %d extra, sleep: %d", this, _DEVICE->getName(), (uint32_t)_DEVICE->_expansionData->_locationID, request->wIndex, request->wValue );
		USBTrace(kUSBTPipe,  kTPPipeControlRequest, (uintptr_t)this, (uintptr_t)_DEVICE->_expansionData->_locationID, (uintptr_t)(request->wIndex<< 16 | request->wValue), 2 );
	}
	
	
    if (completion == NULL)
    {
        // put in our own completion routine if none was specified to
        // fake synchronous operation
        IOUSBCompletion	tap;
		
        request->wLenDone = request->wLength;
		
		// The action of IOUSBSyncCompletion will tell the USL that this is a sync transfer
		//
        tap.target = NULL;
        tap.action = &IOUSBSyncCompletion;
        tap.parameter = &request->wLenDone;
		
        err = _controller->DeviceRequest(request, &tap, _address, _endpoint.number, noDataTimeout, completionTimeout);
		
	}
    else
    {
		if (completion->action == NULL)
		{
			USBLog(1, "IOUSBPipe[%p]::ControlRequest - completion has NULL action - returning kIOReturnBadArgument(%p)", this, (void*)kIOReturnBadArgument);
			USBTrace(kUSBTPipe,  kTPPipeControlRequestMemDesc, (uintptr_t)this, kIOReturnBadArgument, 0, 1 );
			return kIOReturnBadArgument;
		}
        err = _controller->DeviceRequest(request, completion, _address, _endpoint.number, noDataTimeout, completionTimeout);
    }
	
	// USBTrace_End( kUSBTPipe, kTPPipeControlRequestMemDesc, (uintptr_t)this, err);
	
    return(err);
}

#pragma mark Accessors

//================================================================================================
//
//   Accessors 
//
//================================================================================================
//
const IOUSBController::Endpoint *	 
IOUSBPipe::GetEndpoint() 
{ 
    return(&_endpoint); 
}

const IOUSBEndpointDescriptor *	 
IOUSBPipe::GetEndpointDescriptor() 
{ 
    return(_descriptor); 
}

UInt8  
IOUSBPipe::GetDirection() 
{ 
    return(_endpoint.direction); 
}

UInt8  
IOUSBPipe::GetType() 
{ 
    return(_endpoint.transferType); 
}

UInt8  
IOUSBPipe::GetEndpointNumber() 
{ 
    return(_endpoint.number); 
}

USBDeviceAddress  
IOUSBPipe::GetAddress() 
{ 
    return(_address); 
}

UInt16  
IOUSBPipe::GetMaxPacketSize() 
{ 
    return _endpoint.maxPacketSize; 
}

UInt8  
IOUSBPipe::GetInterval() 
{ 
    return (_endpoint.interval); 
}

// 5-16-02 JRH
// This is the "original" GetStatus call, which returned the wrong sized return code, and which always returned zero
// anyway, making it rather stupid. I am leaving in the zero return for backwards compatibility
UInt8 
IOUSBPipe::GetStatus(void)
{ 
    return(0);
}


// 5-16-02 JRH
// This is the "new" GetPipeStatus call which will attempt to return the correct status, i.e. whether the pipe is
// currently stalled or is active
IOReturn
IOUSBPipe::GetPipeStatus(void)
{
    return _CORRECTSTATUS;
}

UInt8
IOUSBPipe::GetUsageType(void)
{
	if ( _endpoint.transferType == kUSBBulk)
		return _UASUSAGEID;
	else
		return _USAGETYPE;
}

UInt8
IOUSBPipe::GetSyncType(void)
{
	return _SYNCTYPE;
}



#pragma mark Obsolete Methods
bool 
IOUSBPipe::InitToEndpoint(const IOUSBEndpointDescriptor *ed, UInt8 speed, USBDeviceAddress address, IOUSBController * controller)
{
#pragma unused (ed, speed, address, controller)
	// Deprecated method
    USBLog(1, "IOUSBPipe::InitToEndpoint, obsolete method 1 called");
    return false;
}



IOUSBPipe *
IOUSBPipe::ToEndpoint(const IOUSBEndpointDescriptor *ed, UInt8 speed, USBDeviceAddress address, IOUSBController *controller)
{
#pragma unused (ed, speed, address, controller)
	// Deprecated method
    USBLog(1, "IOUSBPipe::ToEndpoint, obsolete method 1 called");
    return NULL;
}



IOUSBPipe *
IOUSBPipe::ToEndpoint(const IOUSBEndpointDescriptor *ed, IOUSBDevice * device, IOUSBController *controller)
{
#pragma unused (ed, device, controller)
	// Deprecated method
    USBLog(1, "IOUSBPipe::ToEndpoint, obsolete method 2 called");
    return NULL;
}



#pragma mark Padding Slots

OSMetaClassDefineReservedUsed(IOUSBPipe,  0);
OSMetaClassDefineReservedUsed(IOUSBPipe,  1);
OSMetaClassDefineReservedUsed(IOUSBPipe,  2);
OSMetaClassDefineReservedUsed(IOUSBPipe,  3);
OSMetaClassDefineReservedUsed(IOUSBPipe,  4);
OSMetaClassDefineReservedUsed(IOUSBPipe,  5);
OSMetaClassDefineReservedUsed(IOUSBPipe,  6);
OSMetaClassDefineReservedUsed(IOUSBPipe,  7);
OSMetaClassDefineReservedUsed(IOUSBPipe,  8);
OSMetaClassDefineReservedUsed(IOUSBPipe,  9);
OSMetaClassDefineReservedUsed(IOUSBPipe,  10);
OSMetaClassDefineReservedUsed(IOUSBPipe,  11);
OSMetaClassDefineReservedUsed(IOUSBPipe,  12);
OSMetaClassDefineReservedUsed(IOUSBPipe,  13);
OSMetaClassDefineReservedUsed(IOUSBPipe,  14);

OSMetaClassDefineReservedUnused(IOUSBPipe,  15);
OSMetaClassDefineReservedUnused(IOUSBPipe,  16);
OSMetaClassDefineReservedUnused(IOUSBPipe,  17);
OSMetaClassDefineReservedUnused(IOUSBPipe,  18);
OSMetaClassDefineReservedUnused(IOUSBPipe,  19);
