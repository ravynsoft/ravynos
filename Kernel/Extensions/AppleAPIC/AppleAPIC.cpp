/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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

#include <IOKit/IOLib.h>
#include <IOKit/IOPlatformExpert.h>
#include "AppleAPIC.h"
#include "Apple8259PIC.h"
#include "PICShared.h"

#define super IOInterruptController

OSDefineMetaClassAndStructors( AppleAPICInterruptController,
                               IOInterruptController )

#define GET_FIELD(v, f) \
        (((v) & (f ## Mask)) >> (f ## Shift))

#define PIC_TO_SYS_VECTOR(pv) \
        ((pv) + _vectorBase)

#define SYS_TO_PIC_VECTOR(sv) \
        ((sv) - _vectorBase);

//---------------------------------------------------------------------------

bool AppleAPIC::start( IOService * provider )
{
    IOLog("AppleAPIC::start()\n");
    OSNumber *        num;
    const OSSymbol *  sym;

    _handleSleepWakeFunction = OSSymbol::withCString(
                               kHandleSleepWakeFunction );

	_setVectorPhysicalDestination = OSSymbol::withCString(
									kSetVectorPhysicalDestination );

    if (!_handleSleepWakeFunction || !_setVectorPhysicalDestination)
        goto fail;

    // Get the base vector number assigned to this I/O APIC. When multiple
    // I/O APIC are present, each will be assigned a continuous range of
    // interrupt vectors, starting from the base. Keep in mind that the
    // vector number in IOInterruptSpecifier is an offset into this base,
    // which is equivalent to the interrupt pin number.

    num = OSDynamicCast( OSNumber,
                         provider->getProperty( kBaseVectorNumberKey ) );
    if ( num ) _vectorBase = num->unsigned32BitValue();

    // Get the APIC ID of the local APIC that will handle our interrupt
    // messages. Currently this is the local APIC ID of the boot CPU.
    // I/O APIC will be configured for physical destination mode.

    num = OSDynamicCast( OSNumber,
                         provider->getProperty( kDestinationAPICIDKey ) );
    if ( 0 == num )
    {
        APIC_LOG("IOAPIC-%ld: no destination APIC ID\n", _vectorBase);
        goto fail;
    }
    _destinationAddress = num->unsigned32BitValue();

    // Protect access to the indirect APIC registers.

    _apicLock = IOSimpleLockAlloc();
    if ( 0 == _apicLock )
    {
        APIC_LOG("IOAPIC-%ld: IOSimpleLockAlloc failed\n", _vectorBase);
        goto fail;
    }

    // Get the physical location of the I/O APIC registers.

    num = OSDynamicCast( OSNumber,
                         provider->getProperty( kPhysicalAddressKey ) );
    if ( 0 == num )
    {
        APIC_LOG("IOAPIC-%ld: no physical address\n", _vectorBase);
        goto fail;
    }

    // Describe the I/O APIC registers using a memory descriptor.

    _apicMemory = IOMemoryDescriptor::withPhysicalAddress(
                                      num->unsigned32BitValue(),
                                      256,
                                      kIODirectionInOut );
    if ( 0 == _apicMemory )
    {
        APIC_LOG("IOAPIC-%ld: no memory for apicMemory\n", _vectorBase);
        goto fail;
    }

    // Map in the memory-mapped registers.

    _apicMemory->prepare();
    _apicMemoryMap = _apicMemory->map( kIOMapInhibitCache );    
    if ( 0 == _apicMemoryMap )
    {
        APIC_LOG("IOAPIC-%ld: memory mapping failed\n", _vectorBase);
        goto fail;
    }

    _apicBaseAddr = _apicMemoryMap->getVirtualAddress();
    APIC_LOG("IOAPIC-%ld: phys = %lx virt = %lx\n", _vectorBase,
              num->unsigned32BitValue(), _apicBaseAddr);

    // Cache the ID register, restored on system wake. We trust the BIOS
    // to assign an unique APIC ID for each I/O APIC. Can we?

    _apicIDRegister = indexRead( kIndexID );

    // With the registers mapped in, find out how many interrupt table
    // entries are supported.

    _vectorCount = GET_FIELD( indexRead( kIndexVER ), kVERMaxEntries );
    if (_vectorCount >= 0xFF)
    {
        APIC_LOG("IOAPIC-%ld: excessive vector count (%ld)\n",
            _vectorBase, _vectorCount);
        goto fail;
    }

    APIC_LOG("IOAPIC-%ld: vector range = %ld:%ld\n",
             _vectorBase, _vectorBase, _vectorBase + _vectorCount);
    _vectorCount++;

    // Allocate the memory for the vectors shared with the superclass.

    vectors = IONew( IOInterruptVector, _vectorCount );
    if ( 0 == vectors )
    {
        APIC_LOG("IOAPIC-%ld: no memory for shared vectors\n", _vectorBase);
        goto fail;
    }
    bzero( vectors, sizeof(IOInterruptVector) * _vectorCount );

    // Allocate locks for the vectors.

    for ( int i = 0; i < _vectorCount; i++ )
    {
        vectors[i].interruptLock = IOLockAlloc();
        if ( vectors[i].interruptLock == 0 )
        {
            APIC_LOG("IOAPIC-%ld: no memory for %dth vector lock\n",
                     _vectorBase, i);
            goto fail;
        }
    }

    // Allocate memory for the vector entry table.

    _vectorTable = IONew( VectorEntry, _vectorCount );
    if ( 0 == _vectorTable )
    {
        APIC_LOG("IOAPIC-%ld: no memory for vector table\n", _vectorBase);
        goto fail;
    }

    resetVectorTable();

    // Register our vectors with the top-level interrupt dispatcher.

    setProperty(kBaseVectorNumberKey, _vectorBase, 32);
    setProperty(kVectorCountKey,      _vectorCount, 32);

    // Register this interrupt controller so clients can register with us
    // by name. Grab the interrupt controller name from the provider.
    // This name is assigned by the platform driver, the same entity that
    // recorded our name in the IOInterruptControllers property in nubs.
    // Name assigned to each APIC must be unique system-wide.

    sym = OSSymbol::withString( (OSString *)
                    provider->getProperty( kInterruptControllerNameKey ) );
    if ( 0 == sym )
    {
        APIC_LOG("IOAPIC-%ld: no interrupt controller name\n", _vectorBase);
        goto fail;
    }

    IOLog("IOAPIC: Version 0x%02x Vectors %d:%d\n",
          (uint32_t) GET_FIELD( indexRead( kIndexVER ), kVERVersion ),
          (uint32_t) _vectorBase, (uint32_t) (_vectorBase + _vectorCount - 1));

    getPlatform()->registerInterruptController( (OSSymbol *) sym, this );
    sym->release();

    registerService();

    APIC_LOG("IOAPIC-%ld: start success\n", _vectorBase);
    return true;

fail:
    /* probably fatal */
    return false;
}

//---------------------------------------------------------------------------

void AppleAPIC::free( void )
{
    APIC_LOG("IOAPIC-%ld: %s\n", _vectorBase, __FUNCTION__);

    if ( _handleSleepWakeFunction )
    {
        _handleSleepWakeFunction->release();
        _handleSleepWakeFunction = 0;
    }

    if ( _setVectorPhysicalDestination )
    {
        _setVectorPhysicalDestination->release();
        _setVectorPhysicalDestination = 0;
    }

    if ( vectors )
    {
        for ( int i = 0; i < _vectorCount; i++ )
        {
            if (vectors[i].interruptLock)
                IOLockFree(vectors[i].interruptLock);
        }
        IODelete( vectors, IOInterruptVector, _vectorCount );
        vectors = 0;
    }

    if ( _vectorTable )
    {
        IODelete( _vectorTable, VectorEntry, _vectorCount );
        _vectorTable = 0;
    }

    if ( _apicMemoryMap )
    {
        _apicMemoryMap->release();
        _apicMemoryMap = 0;
    }

    if ( _apicMemory )
    {
        _apicMemory->complete();
        _apicMemory->release();
        _apicMemory = 0;
    }

    if ( _apicLock )
    {
        IOSimpleLockFree( _apicLock );
        _apicLock = 0;
    }

    super::free();
}

//---------------------------------------------------------------------------

void AppleAPIC::dumpRegisters( void )
{
    for ( int i = 0x0; i < 0x10; i++ )
    {
        kprintf("IOAPIC-%d: reg %02x = %08x\n", (uint32_t) _vectorBase, i,
                (uint32_t) indexRead(i));
    }
    for ( int i = 0x10; i < 0x40; i+=2 )
    {
        kprintf("IOAPIC-%d: reg %02x = %08x %08x\n", (uint32_t) _vectorBase, i,
                (uint32_t) indexRead(i + 1), (uint32_t) indexRead(i));
    }
}

//---------------------------------------------------------------------------

void AppleAPIC::resetVectorTable( void )
{
    VectorEntry * entry;

    for ( int vectorNumber = 0; vectorNumber < _vectorCount; vectorNumber++ )
    {
        entry = &_vectorTable[ vectorNumber ];

        // A vector number can be easily mapped to an input pin number, and
        // vice-versa. Is this an issue for P6 platforms? There is a note in
        // the PPro manual about a 2 interrupt per priority level limitation.
        // Need to investigate more...

        entry->l32 = ( PIC_TO_SYS_VECTOR(vectorNumber) & kRTLOVectorNumberMask )
                     | kRTLODeliveryModeFixed
                     | kRTLODestinationModePhysical
                     | kRTLOMaskDisabled;

        entry->h32 = ( _destinationAddress << kRTHIDestinationShift ) &
                       kRTHIDestinationMask;

        writeVectorEntry( vectorNumber );
    }
}

//---------------------------------------------------------------------------

void AppleAPIC::writeVectorEntry( IOInterruptVectorNumber vectorNumber )
{
    IOInterruptState state;

    APIC_LOG("IOAPIC-%ld: %s %02ld = %08lx %08lx\n",
             _vectorBase, __FUNCTION__, vectorNumber,
             _vectorTable[vectorNumber].h32,
             _vectorTable[vectorNumber].l32);

    state = IOSimpleLockLockDisableInterrupt( _apicLock );
        
    indexWrite( kIndexRTLO + vectorNumber * 2,
                _vectorTable[vectorNumber].l32 );

    indexWrite( kIndexRTHI + vectorNumber * 2,
                _vectorTable[vectorNumber].h32 );

    IOSimpleLockUnlockEnableInterrupt( _apicLock, state );
}

//---------------------------------------------------------------------------

void AppleAPIC::writeVectorEntry( IOInterruptVectorNumber vectorNumber, VectorEntry entry )
{
    IOInterruptState state;

    APIC_LOG("IOAPIC-%ld: %s %02ld = %08lx %08lx\n",
             _vectorBase, __FUNCTION__, vectorNumber, entry.h32, entry.l32);

    state = IOSimpleLockLockDisableInterrupt( _apicLock );

    indexWrite( kIndexRTLO + vectorNumber * 2, entry.l32 );
    indexWrite( kIndexRTHI + vectorNumber * 2, entry.h32 );

    IOSimpleLockUnlockEnableInterrupt( _apicLock, state );
}

//---------------------------------------------------------------------------
// Report if the interrupt trigger type is edge or level.

IOReturn AppleAPIC::getInterruptType( IOService * nub,
                                      int         source,
                                      int *       interruptType )
{
    IOInterruptSource * interruptSources;
    OSData            * vectorData;
    UInt32              vectorFlags;
  
    if ( 0 == nub || 0 == interruptType )
        return kIOReturnBadArgument;
  
    interruptSources = nub->_interruptSources;
    vectorData = interruptSources[source].vectorData;
    if (vectorData->getLength() < sizeof(UInt64))
        return kIOReturnNotFound;

    vectorFlags = DATA_TO_FLAGS( vectorData );

    if ((vectorFlags & kInterruptTriggerModeMask) == kInterruptTriggerModeEdge)
        *interruptType = kIOInterruptTypeEdge;
    else
        *interruptType = kIOInterruptTypeLevel;

    APIC_LOG("IOAPIC-%ld: %s( %s, %d ) = %s (vector %ld)\n",
             _vectorBase, __FUNCTION__,
             nub->getName(), source,
             *interruptType == kIOInterruptTypeLevel ? "level" : "edge",
             DATA_TO_VECTOR(vectorData));

    return kIOReturnSuccess;
}

//---------------------------------------------------------------------------

IOReturn AppleAPIC::registerInterrupt( IOService *        nub,
                                       int                source,
                                       void *             target,
                                       IOInterruptHandler handler,
                                       void *             refCon )
{
    IOInterruptSource * interruptSources;
    UInt32              vectorNumber;
    OSData *            vectorData;
  
    interruptSources = nub->_interruptSources;
    vectorData       = interruptSources[source].vectorData;
    vectorNumber     = DATA_TO_VECTOR( vectorData );

    // Check that the vectorNumber is within bounds.
    // Proceed to the superclass method if valid.
    if ( vectorNumber >= (UInt32) _vectorCount )
        return kIOReturnBadArgument;

    return super::registerInterrupt( nub, source, target, handler, refCon );
}

//---------------------------------------------------------------------------

void AppleAPIC::initVector( IOInterruptVectorNumber vectorNumber, IOInterruptVector * vector )
{
    IOInterruptSource * interruptSources;
    UInt32              vectorFlags;
    OSData *            vectorData;

    // Get the vector flags assigned by the platform driver

    interruptSources = vector->nub->_interruptSources;
    vectorData = interruptSources[vector->source].vectorData;
    if (vectorData->getLength() < sizeof(UInt64))
        return;  // expect trouble soon...

    vectorFlags = DATA_TO_FLAGS( vectorData );

    // This interrupt vector should be disabled, so no locking is needed
    // while modifying the table entry for this particular vector.

    // Set trigger mode

    _vectorTable[vectorNumber].l32 &= ~kRTLOTriggerModeMask;
    if ((vectorFlags & kInterruptTriggerModeMask) == kInterruptTriggerModeEdge)
        _vectorTable[vectorNumber].l32 |= kRTLOTriggerModeEdge;
    else
        _vectorTable[vectorNumber].l32 |= kRTLOTriggerModeLevel;

    // Set input pin polarity

    _vectorTable[vectorNumber].l32 &= ~kRTLOInputPolarityMask;
    if ((vectorFlags & kInterruptPolarityMask) == kInterruptPolarityHigh)
        _vectorTable[vectorNumber].l32 |= kRTLOInputPolarityHigh;
    else
        _vectorTable[vectorNumber].l32 |= kRTLOInputPolarityLow;

    writeVectorEntry( vectorNumber );

    APIC_LOG("IOAPIC-%ld: %s %ld to %s trigger, active %s\n",
             _vectorBase, __FUNCTION__, vectorNumber,
             (_vectorTable[vectorNumber].l32 & kRTLOTriggerModeLevel) ?
                "level" : "edge",
             (_vectorTable[vectorNumber].l32 & kRTLOInputPolarityLow) ?
                "low" : "high");
}

//---------------------------------------------------------------------------

bool AppleAPIC::vectorCanBeShared( IOInterruptVectorNumber vectorNumber,
                                   IOInterruptVector * vector)
{
    APIC_LOG("IOAPIC-%ld: %s( %ld )\n", _vectorBase, __FUNCTION__, vectorNumber);

    // Trust the ACPI platform driver to manage interrupt allocations
    // and not assign unshareable interrupts to multiple devices.
    // Drivers must never bypass the platform and wire up interrupts.
    //
    // - FIXME -
    // If access to the 'nub' and 'source' were provided, then we
    // could be extra safe and check the shareable interrupt flag.

    return true;
}

//---------------------------------------------------------------------------

IOInterruptAction AppleAPIC::getInterruptHandlerAddress( void )
{

    return OSMemberFunctionCast(IOInterruptAction,
					this, &AppleAPIC::handleInterrupt);

}

//---------------------------------------------------------------------------

void AppleAPIC::disableVectorHard( IOInterruptVectorNumber vectorNumber,
                                   IOInterruptVector * vector )
{
    APIC_LOG("IOAPIC-%ld: %s %ld\n", _vectorBase, __FUNCTION__, vectorNumber);
    disableVectorEntry( vectorNumber );
}

//---------------------------------------------------------------------------

void AppleAPIC::enableVector( IOInterruptVectorNumber vectorNumber,
                              IOInterruptVector * vector )
{
    APIC_LOG("IOAPIC-%ld: %s %ld\n", _vectorBase, __FUNCTION__, vectorNumber);
    enableVectorEntry( vectorNumber );
}

//---------------------------------------------------------------------------

extern "C" void lapic_end_of_interrupt( void );
IOReturn AppleAPIC::handleInterrupt( void *      savedState,
                                     IOService * nub,
                                     int         source )
{
    IOInterruptVector *     vector;
    IOInterruptVectorNumber vectorNumber;

    // Convert the system interrupt to a vector table entry offset.

    vectorNumber = SYS_TO_PIC_VECTOR(source);
    assert( vectorNumber >= 0 );
    assert( vectorNumber < _vectorCount );

    vector = &vectors[ vectorNumber ];

    vector->interruptActive = 1;

    if ( !vector->interruptDisabledSoft && vector->interruptRegistered )
    {
        vector->handler( vector->target, vector->refCon,
                         vector->nub, vector->source );

        // interruptDisabledSoft flag may be set by the
        // vector handler to indicate that the interrupt
        // should now be disabled. Might as well do it
        // now rather than take another interrupt.

        if ( vector->interruptDisabledSoft )
        {
            vector->interruptDisabledHard = 1;
            disableVectorEntry( vectorNumber );
        }
    }
    else
    {
        vector->interruptDisabledHard = 1;
        disableVectorEntry( vectorNumber );
    }

    vector->interruptActive = 0;


    return kIOReturnSuccess;
}

//---------------------------------------------------------------------------

void AppleAPIC::prepareForSleep( void )
{
    // Mask all interrupts before platform sleep

    for ( int vectorNumber = 0; vectorNumber < _vectorCount; vectorNumber++ )
    {
        VectorEntry entry = _vectorTable[ vectorNumber ];
        entry.l32 |= kRTLOMaskDisabled;
        writeVectorEntry( vectorNumber, entry );
    }
}

//---------------------------------------------------------------------------

void AppleAPIC::resumeFromSleep( void )
{
    // [3550539]
    // Some systems wake up with the PIC interrupt line asserted.
    // This is bad since we program the LINT0 input on the Local
    // APIC to ExtINT mode, and unmask the LINT0 vector. And any
    // unexpected PIC interrupt requests will be serviced. Result
    // is a hard hang the moment the platform driver enables CPU
    // interrupt on wake. Avoid this by masking all PIC vectors.

    outb( kPIC_OCW1(kPIC2BasePort), 0xFF );
    outb( kPIC_OCW1(kPIC1BasePort), 0xFF );

    // Update the identification register containing our APIC ID

    indexWrite( kIndexID, _apicIDRegister );

    for ( int vectorNumber = 0; vectorNumber < _vectorCount; vectorNumber++ )
    {
        // Force a de-assertion on the interrupt line.

        VectorEntry entry = _vectorTable[ vectorNumber ];
        entry.l32 |= kRTLOMaskDisabled;
        writeVectorEntry( vectorNumber, entry );

        // Restore vector entry to its pre-sleep state.

        writeVectorEntry( vectorNumber );
    }
}

//---------------------------------------------------------------------------

IOReturn AppleAPIC::setVectorPhysicalDestination( UInt32 vectorNumber,
												  UInt32 apicID )
{
	VectorEntry * entry;

    //kprintf("IOAPIC-%d: %s( %d, %d )\n", (uint32_t) _vectorBase, __FUNCTION__,
	//	(uint32_t) vectorNumber, (uint32_t) apicID);

	if (vectorNumber >= (UInt32)_vectorCount)
		return kIOReturnBadArgument;

	if (apicID > 255)
		return kIOReturnBadArgument;

	disableVectorEntry( vectorNumber );

	entry = &_vectorTable[ vectorNumber ];
	entry->h32 = (apicID << kRTHIDestinationShift) & kRTHIDestinationMask;

	writeVectorEntry( vectorNumber );
	
	return kIOReturnSuccess;
}

//---------------------------------------------------------------------------

IOReturn AppleAPIC::callPlatformFunction( const OSSymbol * function,
                                          bool waitForFunction,
                                          void * param1, void * param2,
                                          void * param3, void * param4 )
{
    if ( function == _handleSleepWakeFunction )
    {
        if ( param1 )
            prepareForSleep();   /* prior to system sleep */
        else
            resumeFromSleep();   /* after system wake */

        return kIOReturnSuccess;
    }
	else if ( function == _setVectorPhysicalDestination )
	{
		// param1 - vector number
		// param2 - APIC ID
		
		return setVectorPhysicalDestination( (UInt32) (uintptr_t) param1, (UInt32) (uintptr_t) param2 );
	}

    return super::callPlatformFunction( function, waitForFunction,
                                        param1, param2, param3, param4 );
}
