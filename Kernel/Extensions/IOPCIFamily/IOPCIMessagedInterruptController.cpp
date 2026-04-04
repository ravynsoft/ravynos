/*
 * Copyright (c) 1998-2006 Apple Computer, Inc. All rights reserved.
 * Copyright (c) 2007-2021 Apple Inc. All rights reserved.
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


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <IOKit/pci/IOPCIPrivate.h>
#include <IOKit/system.h>
#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/IOMapper.h>

#define kMSIFreeCountKey    "MSIFree"

#ifndef kBaseVectorNumberKey
#define kBaseVectorNumberKey          "Base Vector Number"
#endif

#ifndef kVectorCountKey
#define kVectorCountKey               "Vector Count"
#endif

#ifndef kInterruptControllerNameKey
#define kInterruptControllerNameKey   "InterruptControllerName"
#endif

extern uint32_t gIOPCIFlags;

#if !ACPI_SUPPORT
#define IOPCISetMSIInterrupt(a,b,c)		kIOReturnUnsupported
#endif

#undef  super
#define super IOInterruptController

OSDefineMetaClassAndStructors(IOPCIMessagedInterruptController, IOInterruptController)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define API_ENTRY()																	\
	IOInterruptSource     * interruptSources;										\
	IOInterruptVector     * vector;													\
	IOInterruptVector     * subVectors;												\
	OSData                * vectorData;												\
	IOInterruptVectorNumber vectorNumber;											\
																					\
	interruptSources = nub->_interruptSources;										\
	vectorData = interruptSources[source].vectorData;								\
	vectorNumber = *(IOInterruptVectorNumber *)vectorData->getBytesNoCopy();		\
	vector = &vectors[vectorNumber];												\
	if ((subVectors = (typeof(subVectors)) vector->sharedController)) 				\
	{																				\
		vectorNumber = source - vector->source;				/* now msi index */	    \
		vector       = subVectors + vectorNumber;									\
	}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn IOPCIMessagedInterruptController::registerInterrupt(IOService *nub, int source,
						  void *target,
						  IOInterruptHandler handler,
						  void *refCon)
{
	API_ENTRY();

	// Get the lock for this vector.
	IOLockLock(vector->interruptLock);

	if (vector->interruptRegistered)
	{
		IOLockUnlock(vector->interruptLock);
		return (kIOReturnNoResources);
	}

	// Fill in vector with the client's info.
	vector->handler = handler;
	vector->nub     = nub;
	vector->source  = source;
	vector->target  = target;
	vector->refCon  = refCon;

	// Do any specific initalization for this vector.
	initVector(vectorNumber, vector);

	// Get the vector ready.  It starts hard disabled.
	vector->interruptDisabledHard = 1;
	vector->interruptDisabledSoft = 1;
	vector->interruptRegistered   = 1;

	IOLockUnlock(vector->interruptLock);

    IOPCIDevice * device = OSDynamicCast(IOPCIDevice, nub);
	enableDeviceMSI(device);

	return (kIOReturnSuccess);
}

IOReturn IOPCIMessagedInterruptController::unregisterInterrupt(IOService *nub, int source)
{
	API_ENTRY();

	// Get the lock for this vector.
	IOLockLock(vector->interruptLock);

	// Return success if it is not already registered
	if (!vector->interruptRegistered) 
	{
		IOLockUnlock(vector->interruptLock);
		return (kIOReturnSuccess);
	}

	// Soft disable the source.
	disableInterrupt(nub, source);

	// Turn the source off at hardware. 
	disableVectorHard(vectorNumber, vector);

	// Clear all the storage for the vector except for interruptLock.
	vector->interruptActive = 0;
	vector->interruptDisabledSoft = 0;
	vector->interruptDisabledHard = 0;
	vector->interruptRegistered = 0;
	vector->nub = 0;
	vector->source = 0;
	vector->handler = 0;
	vector->target = 0;
	vector->refCon = 0;

	IOLockUnlock(vector->interruptLock);

	IOPCIDevice * device = OSDynamicCast(IOPCIDevice, nub);
	disableDeviceMSI(device);

	return (kIOReturnSuccess);
}

IOReturn IOPCIMessagedInterruptController::getInterruptType(IOService *nub, int source,
						 int *interruptType)
{
	if (interruptType == 0) return (kIOReturnBadArgument);

	API_ENTRY();

	*interruptType = getVectorType(vectorNumber, vector);

	return (kIOReturnSuccess);
}

IOReturn IOPCIMessagedInterruptController::enableInterrupt(IOService *nub, int source)
{
	API_ENTRY();

	if (vector->interruptDisabledSoft) 
	{
		vector->interruptDisabledSoft = 0;
#if !defined(__i386__) && !defined(__x86_64__)
		OSMemoryBarrier();
#endif
		if (!getPlatform()->atInterruptLevel())
		{
			while (vector->interruptActive) {}
		}
		if (vector->interruptDisabledHard) 
		{
			vector->interruptDisabledHard = 0;
			enableVector(vectorNumber, vector);
		}
	}

	return (kIOReturnSuccess);
}

IOReturn IOPCIMessagedInterruptController::disableInterrupt(IOService *nub, int source)
{
	API_ENTRY();

	vector->interruptDisabledSoft = 1;
#if !defined(__i386__) && !defined(__x86_64__)
	OSMemoryBarrier();
#endif

	if (!getPlatform()->atInterruptLevel())
	{
		while (vector->interruptActive)	{}
	}

	return (kIOReturnSuccess);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOInterruptVector * IOPCIMessagedInterruptController::allocVectors(uint32_t count)
{
    IOInterruptVector * vectors;

    vectors = IONew(IOInterruptVector, count);
    if (!vectors) return (0);
    bzero(vectors, sizeof(IOInterruptVector) * count);

    // Allocate locks for the vectors.
    for (uint32_t i = 0; i < count; i++)
    {
        vectors[i].interruptLock = IOLockAlloc();
        if (!vectors[i].interruptLock)
        {
            goto fail;
        }
    }

    return (vectors);

fail:
    for (uint32_t i = 0; i < count; i++)
    {
        if (vectors[i].interruptLock)
        {
            IOLockFree(vectors[i].interruptLock);
            vectors[i].interruptLock = NULL;
        }
    }
    IOSafeDeleteNULL(vectors, IOInterruptVector, count);
    return (0);
}

bool IOPCIMessagedInterruptController::init(UInt32 numVectors, UInt32 baseVector)
{
    OSNumber * num;
    const OSSymbol * sym = 0;

    if (!super::init())
        return (false);

    _vectorCount = numVectors;
    setProperty(kVectorCountKey, _vectorCount, 32);
	if (-1 != baseVector) setProperty(kBaseVectorNumberKey, baseVector, 32);

    // Allocate the memory for the vectors shared with the superclass.
	vectors = allocVectors(_vectorCount);
    if (!vectors) return (false);

    attach(getPlatform());
    sym = copyName();
    setProperty(kInterruptControllerNameKey, (OSObject *) sym);
    getPlatform()->registerInterruptController( (OSSymbol *) sym, this );
    sym->release();

    num = OSDynamicCast(OSNumber, getProperty(kBaseVectorNumberKey));
    if (num) _vectorBase = num->unsigned32BitValue();

    _messagedInterruptsAllocator = IORangeAllocator::withRange(0, 0, 4, IORangeAllocator::kLocking);
    _messagedInterruptsAllocator->deallocate(0, _vectorCount);
    setProperty(kMSIFreeCountKey, _messagedInterruptsAllocator->getFreeCount(), 32);

    registerService();

    return (true);
}

bool IOPCIMessagedInterruptController::init(UInt32 numVectors)
{
    return (init(numVectors, -1));
}

bool IOPCIMessagedInterruptController::addDeviceInterruptProperties(
                                IORegistryEntry * device,
                                UInt32            controllerIndex,
                                UInt32            interruptFlags,
                                SInt32 *          deviceIndex)
{
    OSArray *        controllers;
    OSArray *        specifiers;
    OSArray *        liveCtrls;
    OSArray *        liveSpecs;
    const OSSymbol * symName;
    OSData *         specData;
    bool             success = false;

    if (!device) return false;
    
    liveCtrls = OSDynamicCast(OSArray,
        device->getProperty(gIOInterruptControllersKey));

    liveSpecs = OSDynamicCast(OSArray,
        device->getProperty(gIOInterruptSpecifiersKey));

    if (liveCtrls && liveSpecs)
    {
        // reserve space for new interrupt vector
        controllers = OSArray::withArray(liveCtrls, liveCtrls->getCount() + 1);
        specifiers  = OSArray::withArray(liveSpecs, liveSpecs->getCount() + 1);
    }
    else
    {
        controllers = OSArray::withCapacity(1);
        specifiers  = OSArray::withCapacity(1);
    }

    specData = OSData::withCapacity(2 * sizeof(UInt32));
    symName = copyName();

    if (!controllers || !specifiers || !specData || !symName) 			return (false);

    // Specifier data will be 64-bits long, containing:
    //    data[0] = interrupt number
    //    data[1] = interrupt flags
    // This must agree with interrupt controller drivers.
    //
    // << Warning >>
    // IOInterruptController::registerInterrupt() assumes that
    // the vectorNumber is the first long in the specifier.

    specData->appendBytes(&controllerIndex, sizeof(controllerIndex));
    specData->appendBytes(&interruptFlags,  sizeof(interruptFlags));

    if (deviceIndex) *deviceIndex = specifiers->getCount();

    success = specifiers->setObject(specData)
                && controllers->setObject(symName);

    if (success)
    {
        device->setProperty(gIOInterruptControllersKey, controllers);
        device->setProperty(gIOInterruptSpecifiersKey, specifiers);
    }

    specifiers->release();
    controllers->release();
    symName->release();
    specData->release();

    return (success);
}

IOReturn IOPCIMessagedInterruptController::allocateDeviceInterrupts(
                                IOService * entry, uint32_t numVectors, uint32_t msiCapability,
                                uint64_t * msiAddress, uint32_t * msiData)
{
    IOReturn      ret;
    IOPCIDevice * device;
    OSData      * data;
    uint32_t      vector, firstVector = _vectorBase;
    IORangeScalar rangeStart;
    uint32_t      message[3];
    uint32_t      msiPhysVectors;
    uint16_t      control = 0;
    bool          allocated;
    uint32_t      msiFlags;

    msiFlags = 0;
    device = OSDynamicCast(IOPCIDevice, entry);
    if (!device) msiCapability = 0;
    msiPhysVectors = 0;

    if (msiCapability)
    {
        uint32_t    vendorProd;
        uint32_t    revIDClass;

        msiCapability  = device->reserved->msiCapability;
        control    = device->configRead16(msiCapability + 2);
        if (kMSIX & device->reserved->msiMode)
            msiPhysVectors = 1 + (0x7ff & control);
        else
            msiPhysVectors = 1 << (0x7 & (control >> 1));

	    numVectors = msiPhysVectors;
        vendorProd = device->savedConfig[kIOPCIConfigVendorID >> 2];
        revIDClass = device->savedConfig[kIOPCIConfigRevisionID >> 2];

        // pci2pci bridges get none or one for hotplug
        if (0x0604 == (revIDClass >> 16))
        {
            bool tunnelLink = (0 != device->getProperty(kIOPCITunnelLinkChangeKey));
            if (tunnelLink 
                || device->getProperty(kIOPCIHotPlugKey)
                || device->getProperty(kIOPCILinkChangeKey))
            {
                // hot plug bridge, but use legacy if avail
                uint8_t line = device->configRead8(kIOPCIConfigInterruptLine);
                if (tunnelLink)
                {
                    tunnelLink = (0x15138086 != vendorProd)
                              && (0x151a8086 != vendorProd)
                              && (0x151b8086 != vendorProd)
                              && (0x15498086 != vendorProd)
                              && ((0x15478086 != vendorProd) || ((revIDClass & 0xff) > 1));
                }
                if (tunnelLink || (line == 0) || (line == 0xFF))
                {
                    // no legacy ints, need one MSI
                    numVectors = 1;
                }
                else numVectors = 0;
            }
            else
            {
                // no hot plug
                numVectors = 0;
            }
        }
#if !defined(SUPPORT_MULTIPLE_MSI)
        else if (numVectors)
        {
            if ((data = OSDynamicCast(OSData, entry->getProperty(kIOPCIMSIFlagsKey)))
              && (data->getLength() >= sizeof(uint32_t)))
            {
                msiFlags = ((uint32_t *)data->getBytesNoCopy())[0];
            }
            if (!(kIOPCIMSIFlagRespect & msiFlags))
            {
                // max per function is one
                numVectors = 1;
            } else if ((data = OSDynamicCast(OSData, entry->getProperty(kIOPCIMSILimitKey)))
              && (data->getLength() >= sizeof(uint32_t)))
            {
                numVectors = msiPhysVectors = min(msiPhysVectors, ((uint32_t *)data->getBytesNoCopy())[0]);
            }
        }
#endif
    }

    allocated  = false;
    rangeStart = 0;
    while (!allocated && numVectors > 0)
    {
        allocated = allocateInterruptVectors(entry, numVectors, &rangeStart);
        if (!allocated) numVectors >>= 1;
    }
    if (!allocated) return (kIOReturnNoSpace);

	firstVector = rangeStart;
	ret = entry->callPlatformFunction(gIOPlatformGetMessagedInterruptAddressKey,
				/* waitForFunction */ false,
				/* nub             */ entry,
				/* options         */ (void *) 0,
				/* vector          */ (void *) (uintptr_t) (firstVector + _vectorBase),
				/* message         */ (void *) &message[0]);

    if (kIOReturnSuccess == ret)
    {
        if (device)
        {
            IOPCISetMSIInterrupt(firstVector + _vectorBase, numVectors, &message[0]);
        }

        if (msiAddress) *msiAddress = message[0] | (((uint64_t)message[1]) << 32);
        if (msiData)    *msiData    = message[2];

        if (!msiCapability)
        {
			for (vector = firstVector; vector < (firstVector + numVectors); vector++)
			{
				addDeviceInterruptProperties(entry, vector,
						kIOInterruptTypeEdge | kIOInterruptTypePCIMessaged, NULL);
			}
		}
		else
        {
			IOPCIConfigShadow * shadow;
			IOPCIConfigSave   * saved;

			shadow = configShadow(device);
			saved  = &shadow->configSave;
			if ((kMSIX & device->reserved->msiMode)
			  && (numVectors < msiPhysVectors))
			{
				device->reserved->msiVectors = allocVectors(msiPhysVectors);
				IOInterruptVector * ivector = &vectors[firstVector];
				// Fill in vector with the IOPCIMessagedInterruptController info
				ivector->handler = OSMemberFunctionCast(IOInterruptHandler,
						this, &IOPCIMessagedInterruptController::handleInterrupt);
				ivector->nub     = device;
				ivector->target  = this;
				ivector->refCon  = (void *)(uintptr_t) msiPhysVectors;
				initVector(firstVector, ivector);
				ivector->interruptDisabledSoft = 0;
				ivector->interruptDisabledHard = 0;
				ivector->interruptRegistered   = 0;
				ivector->sharedController = (IOSharedInterruptController *) device->reserved->msiVectors;

				for (vector = 0; vector < msiPhysVectors; vector++)
				{
					SInt32 deviceIndex;
					addDeviceInterruptProperties(entry, firstVector,
							kIOInterruptTypeEdge | kIOInterruptTypePCIMessaged, &deviceIndex);
					if (!vector) ivector->source = deviceIndex;
				}
			}
			else
			{
				device->reserved->msiVectors = &vectors[firstVector];
				for (vector = firstVector; vector < (firstVector + numVectors); vector++)
				{
					addDeviceInterruptProperties(entry, vector,
							kIOInterruptTypeEdge | kIOInterruptTypePCIMessaged, NULL);
				}
			}

			saved->savedMSIAddress0 = message[0];
			saved->savedMSIAddress1 = message[1];
			saved->savedMSIData     = message[2];
			device->reserved->msiPhysVectorCount = msiPhysVectors;
			device->reserved->msiVectorCount     = numVectors;

            if (kMSIX & device->reserved->msiMode)
            {
                IOMemoryMap * map;
                uint32_t      table;
                uint8_t       bar;

                table = device->configRead32(msiCapability + 8);
                bar = kIOPCIConfigBaseAddress0 + ((table & 7) << 2);
                table &= ~7;
                map = device->mapDeviceMemoryWithRegister(bar);
                if (map) device->reserved->msiPBA = map->getAddress() + table;

                table  = device->configRead32(msiCapability + 4);
                bar    = (kIOPCIConfigBaseAddress0 + ((table & 7) << 2));
                table &= ~7;
                map = device->mapDeviceMemoryWithRegister(bar);
                if (map) device->reserved->msiTable = map->getAddress() + table;
            }
            else
            {
				if (numVectors) numVectors = (31 - __builtin_clz(numVectors)); // log2
				control |= (numVectors << 4);
			}
			control &= ~((1 << 15) | 1); 			// disabled
			device->reserved->msiControl = control;
			initDevice(device, saved);
        }
    }

    return (ret);
}

void IOPCIMessagedInterruptController::initDevice(IOPCIDevice * device, IOPCIConfigSave * saved)
{
    IOInterruptVector * vectors;
	uint32_t            numVectors, msiPhysVectors, vector, data, dummy;
	uint16_t            control, msiCapability, cmd;

	msiCapability  = device->reserved->msiCapability;
	control        = device->reserved->msiControl;
	numVectors     = device->reserved->msiVectorCount;
	msiPhysVectors = device->reserved->msiPhysVectorCount;

	if (kMSIX & device->reserved->msiMode)
	{
		if (device->reserved->msiTable)
		{
			vectors = device->reserved->msiVectors;
			cmd = device->configRead16(kIOPCIConfigCommand);
			device->configWrite16(kIOPCIConfigCommand, cmd | kIOPCICommandMemorySpace);
			dummy = device->configRead16(kIOPCIConfigCommand);	// "pci barrier"
			(void)dummy;
			for (vector = 0; vector < msiPhysVectors; vector++)
			{
				data = saved->savedMSIData;
				if (vector < numVectors) data += vector;
				((volatile uint32_t *) device->reserved->msiTable)[vector*4 + 0] = saved->savedMSIAddress0;
				((volatile uint32_t *) device->reserved->msiTable)[vector*4 + 1] = saved->savedMSIAddress1;
				((volatile uint32_t *) device->reserved->msiTable)[vector*4 + 2] = data;
				((volatile uint32_t *) device->reserved->msiTable)[vector*4 + 3] = vectors[vector].interruptDisabledHard;
			}
			dummy = ((volatile uint32_t *) device->reserved->msiTable)[0]; 	// "pci barrier"
			(void)dummy;
			device->configWrite16(kIOPCIConfigCommand, cmd);
		}
	}
	else
	{
		device->configWrite32(msiCapability + 4, saved->savedMSIAddress0);
		if (0x0080 & control)
		{
			// 64b
			device->configWrite32(msiCapability + 8,  saved->savedMSIAddress1);
			device->configWrite16(msiCapability + 12, saved->savedMSIData);
		}
		else
		{
			device->configWrite16(msiCapability + 8,  saved->savedMSIData);
		}
//		if (0x0100 & control) msiBlockSize += 2;
	}
	device->configWrite16(msiCapability + 2, control);
}

void IOPCIMessagedInterruptController::enableDeviceMSI(IOPCIDevice *device)
{
    if (device && device->reserved && !device->isInactive())
    {
        if (!device->reserved->msiEnable)
        {
            IOByteCount msi = device->reserved->msiCapability;
            uint16_t control;

            control = device->reserved->msiControl;
            if (kMSIX & device->reserved->msiMode)
            {
                control |= (1 << 15);
            }
            else
            {
                control |= 1;
            }
			device->reserved->msiControl = control;
            device->configWrite16(msi + 2, control);

            control = device->configRead16(kIOPCIConfigCommand);
            control |= kIOPCICommandInterruptDisable | kIOPCICommandBusMaster;
            device->configWrite16(kIOPCIConfigCommand, control);
            device->setProperty("IOPCIMSIMode", kOSBooleanTrue);
        }
        device->reserved->msiEnable++;
    }
}

void IOPCIMessagedInterruptController::disableDeviceMSI(IOPCIDevice *device)
{
    if (device && device->reserved 
        && device->reserved->msiEnable 
        && !(--device->reserved->msiEnable)
        && !device->isInactive())
    {
        IOByteCount msi = device->reserved->msiCapability;
        uint16_t control;

        control = device->reserved->msiControl;
        control &= ~((1 << 15) | 1);
		device->reserved->msiControl = control;
        device->configWrite16(msi + 2, control);

		control = device->configRead16(kIOPCIConfigCommand);
		control &= ~kIOPCICommandInterruptDisable;
        device->configWrite16(kIOPCIConfigCommand, control);

        device->removeProperty("IOPCIMSIMode");
    }
}

void IOPCIMessagedInterruptController::saveDeviceState(IOPCIDevice * device, IOPCIConfigSave * save)
{
    if (!device->reserved->msiCapability) return;
}

void IOPCIMessagedInterruptController::restoreDeviceState(IOPCIDevice * device, IOPCIConfigSave * save)
{
    if (!device->reserved->msiCapability) return;
    initDevice(device, save);
}

bool IOPCIMessagedInterruptController::reserveVectors(UInt32 vector, UInt32 count)
{
    bool result;
   
    result = _messagedInterruptsAllocator->allocateRange(vector, count);
    if (result) setProperty(kMSIFreeCountKey, _messagedInterruptsAllocator->getFreeCount(), 32);

    return (result);
}

bool IOPCIMessagedInterruptController::allocateInterruptVectors( IOService *device,
                                                                 uint32_t numVectors,
                                                                 IORangeScalar *rangeStartOut)
{
    bool result;
   
    result = _messagedInterruptsAllocator->allocate(numVectors, rangeStartOut, numVectors);
    if (result) setProperty(kMSIFreeCountKey, _messagedInterruptsAllocator->getFreeCount(), 32);

    return (result);
}

IOReturn IOPCIMessagedInterruptController::deallocateDeviceInterrupts(IOService * device)
{
    const OSSymbol * myName;
    OSArray *        controllers;
    OSObject *       controller;
    OSArray *        specs;
    OSData *         spec;
    uint32_t         index = 0;
    uint32_t         firstVector;

    myName = copyName();

    controllers = OSDynamicCast(OSArray, device->getProperty(gIOInterruptControllersKey));
    specs = OSDynamicCast(OSArray, device->getProperty(gIOInterruptSpecifiersKey));

    if (!myName || !controllers || !specs) return (kIOReturnBadArgument);

	for (firstVector = -1U;
        (spec = OSDynamicCast(OSData, specs->getObject(index)))
		  && (controller = controllers->getObject(index));
		index++)
    {
        if (!controller->isEqualTo(myName)) continue;

		uint32_t vector = *((uint32_t *) spec->getBytesNoCopy());
		if (vector == firstVector) continue;
		if (-1U == firstVector)    firstVector = vector;
		deallocateInterrupt(vector);
    }
    myName->release();

    return (kIOReturnSuccess);
}

void IOPCIMessagedInterruptController::deallocateInterrupt(UInt32 vector)
{
    IOInterruptVector * subVectors;
    IORangeScalar       rangeStart;
    uint32_t            count;

	if ((subVectors = (IOInterruptVector *) vectors[vector].sharedController))
	{
		count = (typeof(count))(uintptr_t) vectors[vector].refCon;
		vectors[vector].sharedController = 0;
	    IODelete(subVectors, IOInterruptVector, count);
	}

	rangeStart = vector;
    _messagedInterruptsAllocator->deallocate(rangeStart, 1);
    IOPCISetMSIInterrupt(rangeStart + _vectorBase, 1, NULL);
    setProperty(kMSIFreeCountKey, _messagedInterruptsAllocator->getFreeCount(), 32);
}

IOReturn
IOPCIMessagedInterruptController::handleInterrupt( void *      state,
                                                   IOService * nub,
                                                   int         source)
{
    IOInterruptVector * vector;
	IOInterruptVector * subVectors;
	uint64_t            bits;
	uint32_t            count, bit;

    source -= _vectorBase;
    if ((source < 0) || (source > (int) _vectorCount)) return (kIOReturnSuccess);

    vector = &vectors[source];
	if ((subVectors = (IOInterruptVector *) vector->sharedController))
	{
		IOPCIDevice * device;

		device = (IOPCIDevice *) vector->nub;
		assert(OSTypeID(IOPCIDevice) == device->getMetaClass());

//      if (!(kIOPCICommandMemorySpace & device->configRead16(kIOPCIConfigCommand))) return (kIOReturnSuccess);

		count = (typeof(count))(uintptr_t) vector->refCon;
		bits = 0;
		for (source = 0; source < count; source++)
		{
            if (device->reserved->msiEnable == 1) vector = &subVectors[0];
            else
            {
                bit = (source & 63);
                if (!bit) bits = ((volatile uint64_t *) device->reserved->msiPBA)[source >> 6];
                if (!(bits & (1ULL << bit))) continue;

                vector = &subVectors[source];
//                if (!vector->interruptRegistered && source) vector = &subVectors[0];
            }
			vector->interruptActive = 1;
			if (vector->interruptRegistered)
			{
				if (vector->interruptDisabledHard) vector->interruptRegistered = 3;
				else
				{
					vector->handler(vector->target, vector->refCon, vector->nub, vector->source);
				}
			}
			vector->interruptActive = 0;
		}
	}
	else
    {
		vector->interruptActive = 1;
		if (vector->interruptRegistered)
		{
			if (vector->interruptDisabledHard) vector->interruptRegistered = 3;
			else
			{
				vector->handler(vector->target, vector->refCon, vector->nub, vector->source);
			}
		}
		vector->interruptActive = 0;
    }

    return (kIOReturnSuccess);
}

bool IOPCIMessagedInterruptController::vectorCanBeShared(IOInterruptVectorNumber vectorNumber,
                                       					 IOInterruptVector * vector)
{
    return (false);
}

void IOPCIMessagedInterruptController::initVector(IOInterruptVectorNumber vectorNumber,
                                       			  IOInterruptVector * vector)
{
}

int IOPCIMessagedInterruptController::getVectorType(IOInterruptVectorNumber vectorNumber,
                                       				IOInterruptVector * vector)
{
    return (kIOInterruptTypeEdge | kIOInterruptTypePCIMessaged);
}

void IOPCIMessagedInterruptController::disableVectorHard(IOInterruptVectorNumber vectorNumber,
                                       					 IOInterruptVector * vector)
{
	IOPCIDevice * device;
	device = (IOPCIDevice *) vector->nub;

	if ((OSTypeID(IOPCIDevice) == device->getMetaClass()) && device->reserved->msiTable)
	{
		// masked
		((volatile uint32_t *) device->reserved->msiTable)[vectorNumber * 4 + 3] = 1;
	}
}

void IOPCIMessagedInterruptController::enableVector(IOInterruptVectorNumber vectorNumber,
                                       				IOInterruptVector * vector)
{
	IOPCIDevice * device;
	device = (IOPCIDevice *) vector->nub;

    if (3 == vector->interruptRegistered)
    {
        vector->interruptRegistered = 1;
        vector->handler(vector->target, vector->refCon,
                        vector->nub, vector->source);
    }
	if ((OSTypeID(IOPCIDevice) == device->getMetaClass()) && device->reserved->msiTable)
	{
		// enabled
		((volatile uint32_t *) device->reserved->msiTable)[vectorNumber * 4 + 3] = 0;
	}
}
