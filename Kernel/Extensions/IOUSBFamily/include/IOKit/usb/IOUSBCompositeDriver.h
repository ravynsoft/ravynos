/*
 * Copyright © 1998-2012 Apple Inc.  All rights reserved.
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

#ifndef _IOKIT_IOUSBCompositeDriver_H
#define _IOKIT_IOUSBCompositeDriver_H

//================================================================================================
//
//   Headers
//
//================================================================================================
//
#include <libkern/OSByteOrder.h>
#include <IOKit/IOLib.h>
#include <IOKit/IONotifier.h>
#include <IOKit/IOService.h>
#include <IOKit/IOMessage.h>
#include <IOKit/usb/IOUSBBus.h>
#include <IOKit/usb/IOUSBDevice.h>
#include <IOKit/usb/IOUSBInterface.h>
#include <IOKit/usb/IOUSBLog.h>
#include <IOKit/usb/USB.h>


//================================================================================================
//
//   Class Declaration for IOUSBCompositeDriver
//
//================================================================================================
//
/*!
 @class IOUSBCompositeDriver
 @abstract Driver that matches to USB composite devices.
 @discussion This class can be overriden to provide for specific behaviors.  The driver itself essentially
 just calls SetConfiguration().
 */
class IOUSBCompositeDriver : public IOService
{
    OSDeclareDefaultStructors(IOUSBCompositeDriver)
    
    IOUSBDevice	*       fDevice;
    IONotifier *        fNotifier;
    bool                fExpectingClose;
    UInt8               fConfigValue;
    UInt8               fConfigbmAttributes;
    
    struct IOUSBCompositeDriverExpansionData 
    {
		bool			fIssueRemoteWakeup;
		bool			fRemoteWakeupIssued;
		bool			fDoNotReconfigure;
    };
    
    IOUSBCompositeDriverExpansionData * fIOUSBCompositeExpansionData;
    
    static IOReturn         CompositeDriverInterestHandler(  void * target, void * refCon, UInt32 messageType, IOService * provider,  void * messageArgument, vm_size_t argSize );
    
public:
        
		// IOService Methods
		//
	virtual bool			init( OSDictionary *  propTable );
	virtual	void			free();
    virtual bool            start(IOService * provider);
    virtual IOReturn        message( UInt32 type, IOService * provider,  void * argument = 0 );
    virtual bool            willTerminate( IOService * provider, IOOptionBits options );
    virtual bool            didTerminate( IOService * provider, IOOptionBits options, bool * defer );
    
    // IOUSBCompositeDriver Methods
    //
    virtual bool            ConfigureDevice();
    virtual IOReturn        ReConfigureDevice();
    /*!
		@function SetConfiguration
     @abstract Call IOUSBDevice to do a SetConfiguration call to the device.
     @param configValue The desired configuration value.
     @param startInterfaceMatching A boolean specifying whether IOKit should begin the process of finding
     matching drivers for the new IOUSBInterface objects.
     */
    virtual IOReturn SetConfiguration(UInt8 configValue, bool startInterfaceMatching=true);
    
    // Getters
    //
    bool                                GetExpectingClose()         { return fExpectingClose; }
    UInt8                               GetConfigValue()            { return fConfigValue; }
    UInt8                               GetConfigbmAttributes()     { return fConfigbmAttributes; }
    IONotifier *                        GetNotifier()               { return fNotifier; }
	
    /*!
     @function ConfigureDriverPowerManagement
     @abstract To be used by a subclass of IOUSBCompositeDriver which wants to participare in the IOPower tree. This is called as part of IOUSBCompositeDriver::start. The default implementation is a NOP
     @param provider The provider as passed into the start method.
     */
    OSMetaClassDeclareReservedUsed(IOUSBCompositeDriver,  0);
    virtual IOReturn ConfigureDevicePowerManagement( IOService * provider );

    
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver,  1);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver,  2);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver,  3);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver,  4);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver,  5);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver,  6);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver,  7);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver,  8);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver,  9);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver, 10);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver, 11);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver, 12);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver, 13);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver, 14);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver, 15);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver, 16);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver, 17);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver, 18);
    OSMetaClassDeclareReservedUnused(IOUSBCompositeDriver, 19);
};

#endif
