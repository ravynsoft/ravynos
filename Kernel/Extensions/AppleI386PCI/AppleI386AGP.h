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
/*
 * Copyright (c) 1998 Apple Computer, Inc.  All rights reserved. 
 *
 * HISTORY
 *
 */


#ifndef _IOKIT_APPLEI386AGP_H
#define _IOKIT_APPLEI386AGP_H

#include <IOKit/pci/IOPCIBridge.h>

class AppleI386AGP : public IOPCI2PCIBridge
{
    OSDeclareDefaultStructors(AppleI386AGP)

protected:
    IORangeAllocator *	agpRange;
    UInt32		agpBaseIndex;
    IOPhysicalAddress	systemBase;
    IOPhysicalLength	systemLength;
    volatile UInt32 *	gartArray;
    IOByteCount		gartLength;
    IOPhysicalAddress gartPhys;
    UInt32		gartCtrl;
    UInt32		agpCommandMask;
    UInt8		targetAGPRegisters;

private:
    virtual IOReturn setAGPEnable( IOAGPDevice * master, bool enable,
					IOOptionBits options = 0 );

public:
    virtual bool start(	IOService * provider );

    virtual bool configure( IOService * provider );

    virtual IOPCIAddressSpace getBridgeSpace( void );

    virtual IOPCIDevice * createNub( OSDictionary * from );

    virtual IOReturn createAGPSpace( IOAGPDevice * master, 
				     IOOptionBits options,
				     IOPhysicalAddress * address, 
				     IOPhysicalLength * length );

    virtual IOReturn destroyAGPSpace( IOAGPDevice * master );

    virtual IORangeAllocator * getAGPRangeAllocator( IOAGPDevice * master );

    virtual IOOptionBits getAGPStatus( IOAGPDevice * master,
				      IOOptionBits options = 0 );
    virtual IOReturn resetAGPDevice( IOAGPDevice * master,
                                     IOOptionBits options = 0 );

    virtual IOReturn getAGPSpace( IOAGPDevice * master,
                                  IOPhysicalAddress * address, 
				  IOPhysicalLength * length );

    virtual IOReturn commitAGPMemory( IOAGPDevice * master, 
				      IOMemoryDescriptor * memory,
				      IOByteCount agpOffset,
				      IOOptionBits options = 0 );

    virtual IOReturn releaseAGPMemory( IOAGPDevice * master, 
				       IOMemoryDescriptor * memory,
				       IOByteCount agpOffset,
				       IOOptionBits options = 0 );
};

#endif /* ! _IOKIT_APPLEI386AGP_H */
