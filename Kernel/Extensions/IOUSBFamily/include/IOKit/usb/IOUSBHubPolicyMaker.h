/*
 * Copyright © 2007, 2012 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_IOUSBHUBPOLICYMAKER_H
#define _IOKIT_IOUSBHUBPOLICYMAKER_H

#include	<IOKit/IOService.h>
#include	<IOKit/usb/IOUSBController.h>
#include	<IOKit/usb/IOUSBHubDevice.h>


enum {
	kIOUSBHubPowerStateOff		= 0,				// losing power
	kIOUSBHubPowerStateRestart	= 1,				// reseting bus, but may maintain power
	kIOUSBHubPowerStateSleep	= 2,				// upstream port and all downstream ports suspended (from the top)
	kIOUSBHubPowerStateLowPower	= 3,				// upstream port and all downstream ports suspended (from the bottom)
	kIOUSBHubPowerStateOn		= 4,				// upstream port and at least one downstream port on
	kIOUSBHubNumberPowerStates	= 5
};

enum {
	kHubResumeRecoveryTime	=	10,								// 10 ms to recover after I resume myself
	kPortResumeRecoveryTime =	10								// 10 ms to recover another device
};

#define kIOUSBHubPowerStateStable	-1

/*
 class IOUSBHubPolicyMaker
 Super class for Hub drivers to incorporate common Power Management code.
*/
class IOUSBHubPolicyMaker : public IOService
{
	OSDeclareAbstractStructors(IOUSBHubPolicyMaker)

protected:
    IOUSBControllerV2 *					_bus;
    IOUSBHubDevice *					_device;					// our provider
    IOUSBHubDevice *					_parentHubDevice;			// for non root hub drivers, this is the hub device that my hub device is connected to
    bool								_isRootHub;					// this is a root hub
	bool								_dozeEnabled;				// true if the controller has been enabled to go into doze mode
	bool								_dontAllowLowPower;			// If true, we will not allow the hub to go into low power mode.
	bool								_dontAllowSleepPower;		// If true, we will not allow extra sleep power for a self powered hub.
	SInt32								_powerStateChangingTo;		// a power state if we are changing to one, or -1 if we are stable
	unsigned long						_myPowerState;				// my current state (since getPowerState doesn't always change in time)
	UInt32								_extraPower;				// DEPRECATED
	UInt32								_extraPowerRemaining;		// DEPRECATED
	UInt32								_hubResumeRecoveryTime;		// # of ms that we will wait before issuing any transactions on our port (nominally 10ms)
    struct ExpansionData 
	{ 
	};
    ExpansionData			*_expansionData;

public:
	// IOService methods
    virtual bool						start(IOService * provider);
	virtual void						stop( IOService * provider );
	virtual IOReturn					powerStateWillChangeTo ( IOPMPowerFlags capabilities, unsigned long stateNumber, IOService* whatDevice);
	virtual unsigned long				powerStateForDomainState ( IOPMPowerFlags domainState );
	virtual IOReturn					setPowerState ( unsigned long powerStateOrdinal, IOService* whatDevice );
	virtual IOReturn					powerStateDidChangeTo ( IOPMPowerFlags capabilities, unsigned long stateNumber, IOService* whatDevice);
	virtual unsigned long				maxCapabilityForDomainState ( IOPMPowerFlags domainState );
	virtual void						powerChangeDone ( unsigned long fromState );

	// public methods which MAY be implemented in subclass
	virtual IOReturn					EnsureUsability(void);
		
	// Extra Port Power calls
	void								AllocateExtraPower();
	IOReturn							GetExtraPortPower(UInt32 portNum, UInt32 *extraPower);
	IOReturn							ReturnExtraPortPower(UInt32 portNum, UInt32 extraPower);
    
	// virtual methods to be implemented in the controlling driver subclass
	virtual bool						ConfigureHubDriver(void) = 0;
	virtual IOReturn					HubPowerChange(unsigned long powerStateOrdinal) = 0;

    OSMetaClassDeclareReservedUsed(IOUSBHubPolicyMaker,  0);
	virtual	IOReturn					GetPortInformation(UInt32 portNum, UInt32 *info);
	
    OSMetaClassDeclareReservedUsed(IOUSBHubPolicyMaker,  1);
	virtual	IOReturn					ResetPort(UInt32 portNum);
	
    OSMetaClassDeclareReservedUsed(IOUSBHubPolicyMaker,  2);
	virtual	IOReturn					SuspendPort(UInt32 portNum, bool suspend);
	
    OSMetaClassDeclareReservedUsed(IOUSBHubPolicyMaker,  3);
	virtual	IOReturn					ReEnumeratePort(UInt32 portNum, UInt32 options);
	
    OSMetaClassDeclareReservedUsed(IOUSBHubPolicyMaker,  4);
	virtual UInt32						RequestExtraPower(UInt32 portNum, UInt32 type, UInt32 requestedPower);

    OSMetaClassDeclareReservedUsed(IOUSBHubPolicyMaker,  5);
	virtual IOReturn					ReturnExtraPower(UInt32 portNum, UInt32 type, UInt32 returnedPower);

    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  6);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  7);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  8);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  9);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  10);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  11);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  12);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  13);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  14);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  15);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  16);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  17);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  18);
    OSMetaClassDeclareReservedUnused(IOUSBHubPolicyMaker,  19);
};

#endif
