/*
 * This is a tiny driver that attaches to a PCI device and logs information
 * about it. It doesn't alter the device in any way. It also supports a
 * generic IOUserClient subclass that allows driver specific client code to
 * make various kinds of calls into the driver, and map shared memory
 * or portions of hardware memory.
 */

#include "AppleSamplePCI.h"
#include <IOKit/IOLib.h>
#include <IOKit/assert.h>

/* 
 * Define the metaclass information that is used for runtime
 * typechecking of IOKit objects. We're a subclass of IOService,
 * but usually we would subclass from a family class.
 */

#define super IOService
OSDefineMetaClassAndStructors( AppleSamplePCI, IOService );

bool AppleSamplePCI::start( IOService * provider )
{
    IOMemoryDescriptor *        mem;
    IOMemoryMap *               map;

    IOLog("AppleSamplePCI::start\n");

    if( !super::start( provider ))
        return( false );

    /*
     * Our provider class is specified in the driver property table
     * as IOPCIDevice, so the provider must be of that class.
     * The assert is just to make absolutely sure for debugging.
     */

    assert( OSDynamicCast( IOPCIDevice, provider ));
    fPCIDevice = (IOPCIDevice *) provider;

    /*
     * Enable memory response from the card
     */
    fPCIDevice->setMemoryEnable( true );


    /*
     * Log some info about the device
     */

    /* print all the device's memory ranges */
    for( UInt32 index = 0;
         index < fPCIDevice->getDeviceMemoryCount();
         index++ ) {

        mem = fPCIDevice->getDeviceMemoryWithIndex( index );
        assert( mem );
        IOLog("Range[%ld] %08lx:%08lx\n", index,
              mem->getPhysicalAddress(), mem->getLength());
    }

    /* look up a range based on its config space base address register */
    mem = fPCIDevice->getDeviceMemoryWithRegister(
                                  kIOPCIConfigBaseAddress0 );
    if( mem )
        IOLog("Range@0x%x %08lx:%08lx\n", kIOPCIConfigBaseAddress0,
                mem->getPhysicalAddress(), mem->getLength());

    /* map a range based on its config space base address register,
     * this is how the driver gets access to its memory mapped registers
     * the getVirtualAddress() method returns a kernel virtual address
     * for the register mapping */
    
    map = fPCIDevice->mapDeviceMemoryWithRegister(
                                  kIOPCIConfigBaseAddress0 );
    if( map ) {
        IOLog("Range@0x%x (%08lx) mapped to kernel virtual address %08x\n",
                kIOPCIConfigBaseAddress0,
                map->getPhysicalAddress(),
                map->getVirtualAddress());
        /* release the map object, and the mapping itself */
        map->release();
    }

    /* read a config space register */
    IOLog("Config register@0x%x = %08lx\n", kIOPCIConfigCommand,
          fPCIDevice->configRead32(kIOPCIConfigCommand) );

    // construct a memory descriptor for a buffer below the 4Gb line &
    // so addressable by 32 bit DMA. This could be used for a 
    // DMA program buffer for example

    IOBufferMemoryDescriptor * bmd = 
        IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
                                // task to hold the memory
                                kernel_task, 
                                // options
                                kIOMemoryPhysicallyContiguous, 
                                // size
                                64*1024, 
                                // physicalMask - 32 bit addressable and page aligned
                                0x00000000FFFFF000ULL);

    if (bmd) {
        generateDMAAddresses(bmd);
    } else {
        IOLog("IOBufferMemoryDescriptor::inTaskWithPhysicalMask failed\n");
    }
    fLowMemory = bmd;
    
    /* publish ourselves so clients can find us */
    registerService();

    return( true );
}

/*
 * We'll come here when the device goes away, or the driver is unloaded.
 */
 
void AppleSamplePCI::stop( IOService * provider )
{
    IOLog("AppleSamplePCI::stop\n");
    super::stop( provider );
}

/*
 * Method to supply an IOMemoryDescriptor for the user client to map into
 * the client process. This sample just supplies all of the hardware memory
 * associated with the PCI device's Base Address Register 0.
 * In a real driver mapping hardware memory would only ever be used in some
 * limited high performance scenarios where the device range can be safely
 * accessed by client code with compromising system stability.
 */

IOMemoryDescriptor * AppleSamplePCI::copyGlobalMemory( void )
{
    IOMemoryDescriptor * memory;
    
    memory = fPCIDevice->getDeviceMemoryWithRegister( kIOPCIConfigBaseAddress0 );
    if( memory)
        memory->retain();
        
    return( memory );
}

IOReturn AppleSamplePCI::generateDMAAddresses( IOMemoryDescriptor * memDesc )
{
    // Get the physical segment list. These could be used to generate a scatter gather
    // list for hardware.

    // This is the old getPhysicalSegment() loop calling IOMemoryDescriptor,
    // it will fail (panic) on new machines with memory above the 4Gb line

    IODMACommand *      cmd;
    IOReturn            err = kIOReturnSuccess;
    IOByteCount         offset = 0;
    IOPhysicalAddress   physicalAddr;
    IOPhysicalLength    segmentLength;
    UInt32              index = 0;

    while( (physicalAddr = memDesc->getPhysicalSegment( offset, &segmentLength ))) {
        IOLog("Physical segment(%ld) %08lx:%08lx\n", index, physicalAddr, segmentLength);
        offset += segmentLength;
        index++;
    }

    // 64 bit physical address generation using IODMACommand
    do
    {
        cmd = IODMACommand::withSpecification(
            // outSegFunc - Host endian since we read the address data with the cpu
            // and 64 bit wide quantities
            kIODMACommandOutputHost64, 
            // numAddressBits
            64, 
            // maxSegmentSize - zero for unrestricted physically contiguous chunks
            0,
            // mappingOptions - kMapped for DMA addresses
            IODMACommand::kMapped,
            // maxTransferSize - no restriction
            0,
            // alignment - no restriction
            1 );
        if (!cmd)
        {
            IOLog("IODMACommand::withSpecification failed\n");
            break;
        }

        // point at the memory descriptor and use the auto prepare option
        // to prepare the entire range
        err = cmd->setMemoryDescriptor(memDesc);
        if (kIOReturnSuccess != err)
        {
            IOLog("setMemoryDescriptor failed (0x%x)\n", err);
            break;
        }

        UInt64 offset = 0;
        while ((kIOReturnSuccess == err) && (offset < memDesc->getLength()))
        {
            // use the 64 bit variant to match outSegFunc
            IODMACommand::Segment64 segments[1];
            UInt32 numSeg = 1;

            // use the 64 bit variant to match outSegFunc
            err = cmd->gen64IOVMSegments(&offset, &segments[0], &numSeg);
            IOLog("gen64IOVMSegments(%x) addr 0x%qx, len 0x%qx, nsegs %ld\n",
                    err, segments[0].fIOVMAddr, segments[0].fLength, numSeg);
        }

        // if we had a DMA controller, kick off the DMA here

        // when the DMA has completed,
        
        // clear the memory descriptor and use the auto complete option
        // to complete the transaction
        err = cmd->clearMemoryDescriptor();
        if (kIOReturnSuccess != err)
        {
            IOLog("clearMemoryDescriptor failed (0x%x)\n", err);
        }
    }
    while (false);
    if (cmd)
        cmd->release();
    // end 64 bit loop


    // 32 bit physical address generation using IODMACommand
    // any memory above 4Gb in the memory descriptor will be buffered
    // to memory below the 4G line, on machines without remapping HW support
    do
    {
        cmd = IODMACommand::withSpecification(
            // outSegFunc - Host endian since we read the address data with the cpu
            // and 32 bit wide quantities
            kIODMACommandOutputHost32, 
            // numAddressBits
            32, 
            // maxSegmentSize - zero for unrestricted physically contiguous chunks
            0,
            // mappingOptions - kMapped for DMA addresses
            IODMACommand::kMapped,
            // maxTransferSize - no restriction
            0,
            // alignment - no restriction
            1 );
        if (!cmd)
        {
            IOLog("IODMACommand::withSpecification failed\n");
            break;
        }

        // point at the memory descriptor and use the auto prepare option
        // to prepare the entire range
        err = cmd->setMemoryDescriptor(memDesc);
        if (kIOReturnSuccess != err)
        {
            IOLog("setMemoryDescriptor failed (0x%x)\n", err);
            break;
        }

        UInt64 offset = 0;
        while ((kIOReturnSuccess == err) && (offset < memDesc->getLength()))
        {
            // use the 32 bit variant to match outSegFunc
            IODMACommand::Segment32 segments[1];
            UInt32 numSeg = 1;

            // use the 32 bit variant to match outSegFunc
            err = cmd->gen32IOVMSegments(&offset, &segments[0], &numSeg);
            IOLog("gen32IOVMSegments(%x) addr 0x%lx, len 0x%lx, nsegs %ld\n",
                    err, segments[0].fIOVMAddr, segments[0].fLength, numSeg);
        }

        // if we had a DMA controller, kick off the DMA here

        // when the DMA has completed,
        
        // clear the memory descriptor and use the auto complete option
        // to complete the transaction
        err = cmd->clearMemoryDescriptor();
        if (kIOReturnSuccess != err)
        {
            IOLog("clearMemoryDescriptor failed (0x%x)\n", err);
        }
    }
    while (false);
    if (cmd)
        cmd->release();
    // end 32 bit loop

    return (err);
}
