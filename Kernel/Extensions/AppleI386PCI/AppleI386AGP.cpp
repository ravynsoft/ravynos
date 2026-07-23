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

#include <IOKit/system.h>

#include <libkern/c++/OSContainers.h>
#include <libkern/OSByteOrder.h>
#include <IOKit/IODeviceMemory.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOLib.h>

#include "AppleI386AGP.h"

#ifndef kIOAGPCommandValueKey
#define kIOAGPCommandValueKey	"IOAGPCommandValue"
#endif

#if 0
#warning **LOGS**
#define DEBG(fmt, args...)  	\
    IOLog(fmt, ## args);
#else
#define DEBG(idx, fmt, args...)  {}
#endif

enum {
    kiMCHCFG	= 0x50,		// 16 bits
    kiAPBASE	= 0x10,
    kiAGPCTRL	= 0xb0,
    kiAPSIZE	= 0xb4,		// 8 bit
    kiATTBASE	= 0xb8,
    kiAMTT	= 0xbc,		// 8 bit
    kiLPTT	= 0xbd,		// 8 bit
};

extern "C" kern_return_t IOUnmapPages(vm_map_t map, vm_offset_t va, vm_size_t length);

extern "C" kern_return_t IOMapPages(vm_map_t map, vm_offset_t va, vm_offset_t pa,
			vm_size_t length, unsigned int options);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define super IOPCI2PCIBridge

OSDefineMetaClassAndStructors(AppleI386AGP, IOPCI2PCIBridge)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool AppleI386AGP::start( IOService * provider )
{
    return( super::start( provider));
}

bool AppleI386AGP::configure( IOService * provider )
{
    if( !findPCICapability( getBridgeSpace(), kIOPCIAGPCapability, &targetAGPRegisters ))
	return( false );

    return( super::configure( provider ));
}

IOPCIAddressSpace AppleI386AGP::getBridgeSpace( void )
{
    IOPCIAddressSpace	space;

    space = super::getBridgeSpace();

    // how do we know this?
    space.s.deviceNum = 0;

    return( space );
}

IOPCIDevice * AppleI386AGP::createNub( OSDictionary * from )
{
    IOPCIDevice *	nub;
    IOPCIAddressSpace	space;
    bool		isAGP;
    UInt8		masterAGPRegisters;

    spaceFromProperties( from, &space);

    isAGP = ( /* (space.s.deviceNum != getBridgeSpace().s.deviceNum)
	    && */ findPCICapability( space, kIOPCIAGPCapability, &masterAGPRegisters ));

    if( isAGP) {
	nub = new IOAGPDevice;
        if( nub)
            ((IOAGPDevice *)nub)->masterAGPRegisters = masterAGPRegisters;
	from->setObject( kIOAGPBusFlagsKey, getProperty(kIOAGPBusFlagsKey));
    } else
        nub = super::createNub( from );

    return( nub );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn AppleI386AGP::createAGPSpace( IOAGPDevice * master,
				      IOOptionBits options,
				      IOPhysicalAddress * address,
				      IOPhysicalLength * length )
{
    IOReturn		err;
    IOPCIAddressSpace 	target = getBridgeSpace();
    IOPhysicalLength	agpLength;
    UInt32		agpCtrl;

    enum { agpSpacePerPage = 4 * 1024 * 1024 };
    enum { agpBytesPerGartByte = 1024 };
    enum { alignLen = 4 * 1024 * 1024 - 1 };

    destroyAGPSpace( master );

    agpCommandMask = 0xffffffff;
    agpCommandMask &= ~kIOAGPFastWrite;
//  agpCommandMask &= ~kIOAGPSideBandAddresssing;

    {
	// There's an nVidia NV11 ROM (revision 1017) that says that it can do fast writes,
	// but can't, and can often lock the machine up when fast writes are enabled.

	#define kNVIDIANV11EntryName	"NVDA,NVMac"
	#define kNVROMRevPropertyName 	"rom-revision"
	#define kNVBadRevision			'1017'

	const UInt32    badRev = kNVBadRevision;
	OSData *	data;

	if( (0 == strcmp( kNVIDIANV11EntryName, master->getName()))
	 && (data = OSDynamicCast(OSData, master->getProperty(kNVROMRevPropertyName)))
	 && (data->isEqualTo( &badRev, sizeof(badRev) )))

	    agpCommandMask &= ~kIOAGPFastWrite;
    }

    agpLength = *length;
    if( !agpLength)
	agpLength = 32 * 1024 * 1024;

    agpLength = (agpLength + alignLen) & ~alignLen;

    err = kIOReturnVMError;
    do {

	gartLength = agpLength / agpBytesPerGartByte;
	gartArray = (volatile UInt32 *) IOMallocContiguous(
				gartLength, 4096, &gartPhys );
	if( !gartArray)
	    continue;
	IOSetProcessorCacheMode(kernel_task, (vm_address_t) gartArray, gartLength, kIOInhibitCache);
        bzero( (void *) gartArray, gartLength);

//	IOUnmapPages( kernel_map, (vm_address_t) gartArray, gartLength );
	// is this std?
        systemBase	= configRead32( target, kiAPBASE ) & 0xfffffff0;
	DEBG("APSIZE: %08lx\n", (UInt32)configRead8(target, kiAPSIZE));
        systemLength	= (((configRead8( target, kiAPSIZE ) & 0x3f) ^ 0x3f) + 1) << 22;

	DEBG("sysB %08lx, sysL %08lx\n", systemBase, systemLength);

	if( !systemLength)
	    continue;

if (systemLength > agpLength)
    systemLength = agpLength;

	DEBG("sysB %08lx, sysL %08lx\n", systemBase, systemLength);

	agpRange = IORangeAllocator::withRange( agpLength, 4096 );
	if( !agpRange)
	    continue;

        *address = systemBase;
        *length = systemLength;

	agpCtrl = configRead32(target, kiAGPCTRL);
	agpCtrl &= ~(1 << 7);
	configWrite32( target, kiAGPCTRL, agpCtrl ); 		// b7 gtlb ena

//        configWrite32( target, kiAGPCTRL, 0 << 7 ); 		// b7 gtlb ena

//        assert( 0 == (gartPhys & 0xfff));

        configWrite32( target, kiATTBASE, gartPhys );

	agpCtrl = configRead32(target, kiAGPCTRL);
	//agpCtrl |= (1 << 7);
	configWrite32( target, kiAGPCTRL, agpCtrl ); 		// b7 gtlb ena

	DEBG("kiAGPCTRL %08lx, kiATTBASE %08lx\n",
	    configRead32( target, kiAGPCTRL ),
	    configRead32( target, kiATTBASE ));

        err = kIOReturnSuccess;

    } while( false );

    if( kIOReturnSuccess == err)
        setAGPEnable( master, true, 0 );
    else
	destroyAGPSpace( master );

    return( err );
}

IOReturn AppleI386AGP::getAGPSpace( IOAGPDevice * master,
                                  IOPhysicalAddress * address,
				  IOPhysicalLength * length )
{
    if( systemLength) {

        if( address)
            *address = systemBase;
        if( length)
            *length  = systemLength;
        return( kIOReturnSuccess );

    } else
        return( kIOReturnNotReady );
}

IOReturn AppleI386AGP::destroyAGPSpace( IOAGPDevice * master )
{

    setAGPEnable( master, false, 0 );

    if( gartArray) {
//	IOMapPages( kernel_map, (vm_address_t) gartArray, gartPhys, gartLength, kIOMapDefaultCache );
	IOFreeContiguous( (void *) gartArray, gartLength);
	gartArray = 0;
    }
    if( agpRange) {
	agpRange->release();
	agpRange = 0;
    }
    if( systemLength)
	systemLength = 0;


    return( kIOReturnSuccess );
}

IORangeAllocator * AppleI386AGP::getAGPRangeAllocator(
					IOAGPDevice * master )
{
//    if( agpRange)	agpRange->retain();
    return( agpRange );
}

IOOptionBits AppleI386AGP::getAGPStatus( IOAGPDevice * master,
					    IOOptionBits options )
{
    IOPCIAddressSpace 	target = getBridgeSpace();

#if XXX
    return( configRead32( target, kUniNINTERNAL_STATUS ) );
#else
    return( 0 );
#endif
}

IOReturn AppleI386AGP::commitAGPMemory( IOAGPDevice * master,
				      IOMemoryDescriptor * memory,
				      IOByteCount agpOffset,
				      IOOptionBits options )
{
    IOPCIAddressSpace 	target = getBridgeSpace();
    IOReturn		err = kIOReturnSuccess;
    UInt32		offset = 0, tmp, agpCtrl;
    IOPhysicalAddress	physAddr;
    IOByteCount		len;

//    ok = agpRange->allocate( memory->getLength(), &agpOffset );

    agpCtrl = configRead32(target, kiAGPCTRL);
    agpCtrl &= ~(1 << 7);
    configWrite32( target, kiAGPCTRL, agpCtrl ); 		// b7 gtlb ena

    if(memory)
    {
	assert( agpOffset < systemLength );
	agpOffset /= (page_size / 4);
	while( (physAddr = memory->getPhysicalSegment( offset, &len ))) {

	    offset += len;
	    len = (len + 0xfff) & ~0xfff;
	    while( len > 0) {
		OSWriteLittleInt32( gartArray, agpOffset,
					((physAddr & ~0xfff) | 1));
		agpOffset += 4;
		physAddr += page_size;
		len -= page_size;
	    }
	}

	// Read back from last entry written to flush entry to main memory.
	tmp = OSReadLittleInt32(gartArray, agpOffset-4);

	#if 1
	// Deal with stupid Pentium 4 8-deeps store queue crap.
	for(offset = 0; offset < 64*16; offset += 64)
	{
		tmp = OSReadLittleInt32(gartArray, offset);
		OSWriteLittleInt32(gartArray, offset, tmp);
	}
	#endif
    }

    agpCtrl = configRead32(target, kiAGPCTRL);
    agpCtrl |= (1 << 7);
    configWrite32( target, kiAGPCTRL, agpCtrl ); 		// b7 gtlb ena

    return( err );
}

IOReturn AppleI386AGP::releaseAGPMemory( IOAGPDevice * master,
                                            IOMemoryDescriptor * memory,
                                            IOByteCount agpOffset,
                                            IOOptionBits options )
{
    IOPCIAddressSpace 	target = getBridgeSpace();
    IOReturn		err = kIOReturnSuccess;
    IOByteCount		length;
    UInt32 agpCtrl;

    if( !memory)
	return( kIOReturnBadArgument );

    length = memory->getLength();

    if( (agpOffset + length) > systemLength)
	return( kIOReturnBadArgument );

    agpCtrl = configRead32(target, kiAGPCTRL);
    agpCtrl &= ~(1 << 7);
    configWrite32( target, kiAGPCTRL, agpCtrl ); 		// b7 gtlb ena

//    agpRange->deallocate( agpOffset, length );

    length = (length + 0xfff) & ~0xfff;
    agpOffset /= page_size;

    while( length > 0) {
	gartArray[ agpOffset++ ] = 0;
	length -= page_size;
    }

    agpCtrl = configRead32(target, kiAGPCTRL);
    agpCtrl |= (1 << 7);
    configWrite32( target, kiAGPCTRL, agpCtrl ); 		// b7 gtlb ena

    return( err );
}

IOReturn AppleI386AGP::setAGPEnable( IOAGPDevice * _master,
					bool enable, IOOptionBits options )
{
    IOReturn		err = kIOReturnSuccess;
    IOPCIAddressSpace 	target = getBridgeSpace();
    IOPCIAddressSpace 	master = _master->space;
    UInt32		command;
    UInt32		targetStatus, masterStatus;
    UInt8		masterAGPRegisters = _master->masterAGPRegisters;

    if( enable) {

#if XX
        configWrite32( target, kUniNGART_CTRL, gartCtrl | 0);
#endif

	targetStatus = configRead32( target,
                                     targetAGPRegisters + kIOPCIConfigAGPStatusOffset );
	masterStatus = configRead32( master,
                                     masterAGPRegisters + kIOPCIConfigAGPStatusOffset );

	DEBG("target %08lx, master %08lx\n", targetStatus, masterStatus);

	command = kIOAGPSideBandAddresssing
                | kIOAGPFastWrite
		| kIOAGP4xDataRate | kIOAGP2xDataRate | kIOAGP1xDataRate;
	command &= targetStatus;
	command &= masterStatus;

	if( command & kIOAGP4xDataRate)
	    command &= ~(kIOAGP2xDataRate | kIOAGP1xDataRate);
	else if( command & kIOAGP2xDataRate)
	    command &= ~(kIOAGP1xDataRate);
	else if( 0 == (command & kIOAGP1xDataRate))
            return( kIOReturnUnsupported );

	command |= kIOAGPEnable;
        command &= agpCommandMask;

	if( targetStatus > masterStatus)
	    targetStatus = masterStatus;
	command |= (targetStatus & kIOAGPRequestQueueMask);

        _master->setProperty(kIOAGPCommandValueKey, &command, sizeof(command));

	DEBG("AGPCMD %08lx\n", command);

#if XX
        configWrite32( target, kUniNGART_CTRL, gartCtrl | kGART_EN | kGART_INV );
#endif

#if 1
	configWrite32( target, targetAGPRegisters + kIOPCIConfigAGPCommandOffset, command );

	DEBG("target command %08lx\n", configRead32( target, targetAGPRegisters + kIOPCIConfigAGPCommandOffset));

	configWrite32( master, masterAGPRegisters + kIOPCIConfigAGPCommandOffset, command );

	DEBG("master command %08lx\n", configRead32( master, masterAGPRegisters + kIOPCIConfigAGPCommandOffset));

#endif

	UInt32 value = configRead16( target, kiMCHCFG );

	DEBG("kiMCHCFG %08lx\n", value);

	value |= (1 << 9) | (0 << 10);
	configWrite16( target, kiMCHCFG, value );

	DEBG("kiMCHCFG %02x\n", configRead16( target, kiMCHCFG ));

#if 0
        configWrite32( target, kUniNGART_CTRL, gartCtrl | kGART_EN | kGART_INV );
        configWrite32( target, kUniNGART_CTRL, gartCtrl | kGART_EN );
        configWrite32( target, kUniNGART_CTRL, gartCtrl | kGART_EN | kGART_2xRESET);
        configWrite32( target, kUniNGART_CTRL, gartCtrl | kGART_EN );
#endif

        _master->masterState |= kIOAGPStateEnabled;

    } else {

#if XX
	while( 0 == (kIOAGPIdle & configRead32( getBridgeSpace(),
					kUniNINTERNAL_STATUS )))
		{}
#endif

        configWrite32( master, masterAGPRegisters + kIOPCIConfigAGPCommandOffset, 0 );
        configWrite32( target, targetAGPRegisters + kIOPCIConfigAGPCommandOffset, 0 );
#if 0
        configWrite32( target, kUniNGART_CTRL, gartCtrl | kGART_EN | kGART_INV );
        configWrite32( target, kUniNGART_CTRL, gartCtrl | 0 );
        configWrite32( target, kUniNGART_CTRL, gartCtrl | kGART_2xRESET);
        configWrite32( target, kUniNGART_CTRL, gartCtrl | 0 );
#endif

	UInt32 value = configRead16( target, kiMCHCFG );

	DEBG("kiMCHCFG %08lx\n", value);

	value &= ~((1 << 9)|(1 << 10));
	configWrite16( target, kiMCHCFG, value );

        _master->masterState &= ~kIOAGPStateEnabled;
    }

    return( err );
}

IOReturn AppleI386AGP::resetAGPDevice( IOAGPDevice * master,
                                          IOOptionBits options )
{
    IOReturn ret = kIOReturnSuccess;

    return( ret );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */