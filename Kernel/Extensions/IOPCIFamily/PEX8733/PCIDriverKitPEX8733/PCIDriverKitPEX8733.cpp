//
//  PCIDriverKitPEX8733.cpp
//  PCIDriverKitPEX8733
//
//  Created by Kevin Strasberg on 10/29/19.
//
#include <stdio.h>
#include <os/log.h>
#include <DriverKit/DriverKit.h>
#include <PCIDriverKit/PCIDriverKit.h>

#include "PCIDriverKitPEX8733.h"
#include "PEX8733Definitions.h"
#define debugLog(fmt, args...)  os_log(OS_LOG_DEFAULT, "PCIDriverKitPEX8733::%s:  " fmt,  __FUNCTION__,##args)

struct PCIDriverKitPEX8733_IVars
{
    IOPCIDevice* pciDevice;
};

bool
PCIDriverKitPEX8733::init()
{
    if(super::init() != true)
    {
        return false;
    }

    ivars = IONewZero(PCIDriverKitPEX8733_IVars, 1);

    if(ivars == NULL)
    {
        return false;
    }

    return true;
}

void
PCIDriverKitPEX8733::free()
{
    IOSafeDeleteNULL(ivars, PCIDriverKitPEX8733_IVars, 1);
    super::free();
}

kern_return_t
IMPL(PCIDriverKitPEX8733, Start)
{
    kern_return_t result = Start(provider, SUPERDISPATCH);

    if(result != kIOReturnSuccess)
    {
        return result;
    }

    ivars->pciDevice = OSDynamicCast(IOPCIDevice, provider);
    if(ivars->pciDevice == NULL)
    {
        Stop(provider);
        return kIOReturnNoDevice;
    }
    ivars->pciDevice->retain();

	if(ivars->pciDevice->Open(this, 0) != kIOReturnSuccess)
	{
		Stop(provider);
        return kIOReturnNoDevice;
	}
    debugLog("enabling bus master and memory space");
    uint16_t command;
    ivars->pciDevice->ConfigurationRead16(kIOPCIConfigurationOffsetCommand, &command);
    ivars->pciDevice->ConfigurationWrite16(kIOPCIConfigurationOffsetCommand,
                                           command | kIOPCICommandBusMaster | kIOPCICommandMemorySpace);

    // set bus mastering and memory space enabled for the channel
    ivars->pciDevice->MemoryRead16(kPEX8733MemoryIndex,
                                   configurationSpaceOffset(0) + kIOPCIConfigurationOffsetCommand,
                                   &command);

    ivars->pciDevice->MemoryWrite16(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(0) + kIOPCIConfigurationOffsetCommand,
                                    command | kIOPCICommandBusMaster | kIOPCICommandMemorySpace);

    // for now only allow 1 channel
    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(0) + kPEX8733DMAChannelGlobalControlOffset,
                                    kPEX8733DMAChannelGlobalControlModeOne);

    dmaBlockTransferTest();
    dmaDescriptorTransferTest();
    return result;
}

kern_return_t
IMPL(PCIDriverKitPEX8733, Stop)
{
    debugLog("disabling bus master and memory space");
    if(ivars->pciDevice != NULL)
    {
        uint16_t command;
        ivars->pciDevice->ConfigurationRead16(kIOPCIConfigurationOffsetCommand, &command);
        ivars->pciDevice->ConfigurationWrite16(kIOPCIConfigurationOffsetCommand,
                                               command & ~(kIOPCICommandBusMaster | kIOPCICommandMemorySpace));

        ivars->pciDevice->MemoryRead16(kPEX8733MemoryIndex,
                                       configurationSpaceOffset(0) + kIOPCIConfigurationOffsetCommand,
                                       &command);

        ivars->pciDevice->MemoryWrite16(kPEX8733MemoryIndex,
                                        configurationSpaceOffset(0) + kIOPCIConfigurationOffsetCommand,
                                        command & ~(kIOPCICommandBusMaster | kIOPCICommandMemorySpace));
    }
    OSSafeReleaseNULL(ivars->pciDevice);
    return Stop(provider, SUPERDISPATCH);
}

kern_return_t
PCIDriverKitPEX8733::dmaBlockTransferTest()
{
    debugLog("starting dmaBlockTransferTest\n");
    kern_return_t result = kIOReturnSuccess;

    IOBufferMemoryDescriptor* transferBuffer         = NULL;
    uint64_t                  bufferCapacity         = IOVMPageSize * kPEX8733DefaultTestTransferSizeNumPages;
    uint64_t                  bufferAlignment        = IOVMPageSize;
    IOAddressSegment          virtualAddressSegment  = { 0 };
    IOAddressSegment          physicalAddressSegment = { 0 };
    uint64_t                  dmaFlags               = 0;
    uint64_t                  dmaLength              = 0;
    uint32_t                  dmaSegmentCount        = 1;

    IOBufferMemoryDescriptor::Create(kIOMemoryDirectionOutIn,
                                     bufferCapacity,
                                     bufferAlignment,
                                     &transferBuffer);
    transferBuffer->SetLength(bufferCapacity);
    transferBuffer->GetAddressRange(&virtualAddressSegment);

    // Populate the first half of the buffer with data to be written to the 2nd half
    uint64_t transferSize = virtualAddressSegment.length / 2;
    for(unsigned int i = 0; i < transferSize; i++)
    {
        reinterpret_cast<uint8_t*>(virtualAddressSegment.address)[i] = i % 0xFF;
    }


    transferBuffer->PrepareForDMA(0,
                                  ivars->pciDevice,
                                  0,
                                  0,
                                  &dmaFlags,
                                  &dmaLength,
                                  &dmaSegmentCount,
                                  &physicalAddressSegment);

    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelSourceAddressLowerOffset,
                                    static_cast<uint32_t>(physicalAddressSegment.address));
    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelSourceAddressUpperOffset,
                                    static_cast<uint32_t>(physicalAddressSegment.address >> 32));

    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelDestinationAddressLowerOffset,
                                    static_cast<uint32_t>(physicalAddressSegment.address + transferSize));
    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex, configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelDestinationAddressUpperOffset,
                                    static_cast<uint32_t>((physicalAddressSegment.address + transferSize) >> 32));


    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelTransferSizeOffset,
                                    kPEX8733DMAChannelTransferSizeRegisterDMAValid
                                    | kPEX8733DMAChannelTransferSizeRegisterDoneInterruptEnable
                                    | (transferSize & kPEX8733DMAChannelTransferSizeRegisterTransferSizeRange));

    uint32_t interruptControlStatus = 0;
    ivars->pciDevice->MemoryRead32(kPEX8733MemoryIndex,
                                   configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelInterruptControlStatusRegisterOffset,
                                   &interruptControlStatus);

    // clear interrupts (clear on write)
    interruptControlStatus |= kPEX8733DMAChannelInterruptControlStatusInterruptStatusRange;

    // enable interrupts
    interruptControlStatus |= kPEX8733DMAChannelInterruptControlStatusErrorInterruptEnable
                              | kPEX8733DMAChannelInterruptControlStatusInvalidDescriptorInterruptEnable
                              | kPEX8733DMAChannelInterruptControlStatusGracefulPauseDoneInterruptEnable
                              | kPEX8733DMAChannelInterruptControlStatusImmediatePauseDoneInterruptEnable;

    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelInterruptControlStatusRegisterOffset,
                                    interruptControlStatus);

    // Enable DMA Block Mode, disable descriptor completion status writeback, and clear any active status bits
    uint32_t controlStatus = 0;
    ivars->pciDevice->MemoryRead32(kPEX8733MemoryIndex,
                                   configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelControlStatusRegisterOffset,
                                   &controlStatus);

    controlStatus &= (~kPEX8733DMAChannelControlStatusDescriptorModeSelectRange);
    controlStatus &= ~kPEX8733DMAChannelControlStatusCompletionStatusWriteBackEnable;
    controlStatus |= kPEX8733DMAChannelControlStatusStart | kPEX8733DMAChannelControlStatusDescriptorModeSelectBlockMode;

    // clear any active status bits
    controlStatus |= kPEX8733DMAChannelControlStatusRange | kPEX8733DMAChannelControlStatusHeaderLoggingValid;

    // start DMA transfer
    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelControlStatusRegisterOffset,
                                    controlStatus);


    // Wait for transfer to complete
    ivars->pciDevice->MemoryRead32(kPEX8733MemoryIndex,
                                   configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelInterruptControlStatusRegisterOffset,
                                   &interruptControlStatus);

    while(((interruptControlStatus & (kPEX8733DMAChannelInterruptControlStatusDescriptorDoneInterruptStatus
                                      | kPEX8733DMAChannelInterruptControlStatusErrorInterruptStatus
                                      | kPEX8733DMAChannelInterruptControlStatusInvalidDescriptorInterruptStatus
                                      | kPEX8733DMAChannelInterruptControlStatusGracefulPauseDoneInterruptStatus
                                      | kPEX8733DMAChannelInterruptControlStatusImmediatePauseDoneInterruptStatus)) == 0))
    {
        debugLog("interrupt control status 0x%x\n", interruptControlStatus);

        IOSleep(50);

        ivars->pciDevice->MemoryRead32(kPEX8733MemoryIndex,
                                       configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelInterruptControlStatusRegisterOffset,
                                       &interruptControlStatus);
    }

    if(memcmp(reinterpret_cast<void*>(virtualAddressSegment.address),
              reinterpret_cast<void*>(virtualAddressSegment.address + transferSize),
              transferSize) != 0)
    {
        debugLog("source and destination do not match\n");
    }
    else
    {
        debugLog("dma successful\n");
    }

    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelSourceAddressLowerOffset,
                                    0);
    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelSourceAddressUpperOffset,
                                    0);

    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelDestinationAddressLowerOffset,
                                    0);
    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelDestinationAddressUpperOffset,
                                    0);


    OSSafeReleaseNULL(transferBuffer);
    return result;
}

kern_return_t
PCIDriverKitPEX8733::dmaDescriptorTransferTest()
{
    debugLog("starting dmaDescriptorTransferTest\n");

    kern_return_t              result                           = kIOReturnSuccess;
    IOBufferMemoryDescriptor*  ringBuffer                       = NULL;
    IOBufferMemoryDescriptor** descriptors                      = NULL;
    unsigned int               numDescriptors                   = kPEX8733DefaultNumDMADescriptors;
    uint64_t                   transferBufferSize               = IOVMPageSize * kPEX8733DefaultTestTransferSizeNumPages;
    uint64_t                   transferBufferAlignment          = IOVMPageSize;
    IOAddressSegment           ringBufferVirtualAddressSegment  = { 0 };
    IOAddressSegment           ringBufferPhysicalAddressSegment = { 0 };
    uint64_t                   ringDmaFlags                     = 0;
    uint64_t                   ringDmaLength                    = 0;
    uint32_t                   ringDmaSegmentCount              = 1;

    descriptors = reinterpret_cast<IOBufferMemoryDescriptor**>(IOMalloc(sizeof(IOBufferMemoryDescriptor*) * numDescriptors));

    IOBufferMemoryDescriptor::Create(kIOMemoryDirectionOut,
                                     numDescriptors * sizeof(DMAStandardDescriptor),
                                     transferBufferAlignment,
                                     &ringBuffer);

    ringBuffer->SetLength(numDescriptors * sizeof(DMAStandardDescriptor));
    ringBuffer->GetAddressRange(&ringBufferVirtualAddressSegment);
    ringBuffer->PrepareForDMA(0,
                              ivars->pciDevice,
                              0,
                              0,
                              &ringDmaFlags,
                              &ringDmaLength,
                              &ringDmaSegmentCount,
                              &ringBufferPhysicalAddressSegment);

    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelDescriptorRingAddressLowerOffset,
                                    static_cast<uint32_t>(ringBufferPhysicalAddressSegment.address));
    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelDescriptorRingAddressUpperOffset,
                                    static_cast<uint32_t>(ringBufferPhysicalAddressSegment.address >> 32));

    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelDescriptorRingNextDescriptorAddressLowerOffset,
                                    static_cast<uint32_t>(ringBufferPhysicalAddressSegment.address));

    DMAStandardDescriptor* descriptorRing = reinterpret_cast<DMAStandardDescriptor*>(ringBufferVirtualAddressSegment.address);
    for(unsigned int i = 0; i < numDescriptors; i++)
    {
        IOAddressSegment descriptorBufferVirtualAddressSegment  = { 0 };
        IOAddressSegment descriptorBufferPhysicalAddressSegment = { 0 };
        uint64_t         dmaFlags                               = 0;
        uint64_t         dmaLength                              = 0;
        uint32_t         dmaSegmentCount                        = 1;

        IOBufferMemoryDescriptor::Create(kIOMemoryDirectionOutIn,
                                         transferBufferSize,
                                         transferBufferAlignment,
                                         &descriptors[i]);

        descriptors[i]->SetLength(transferBufferSize);
        descriptors[i]->GetAddressRange(&descriptorBufferVirtualAddressSegment);
        descriptors[i]->PrepareForDMA(0,
                                      ivars->pciDevice,
                                      0,
                                      0,
                                      &dmaFlags,
                                      &dmaLength,
                                      &dmaSegmentCount,
                                      &descriptorBufferPhysicalAddressSegment);

        // Populate the first half of the buffer with data to be written to the 2nd half
        uint64_t transferSize = descriptorBufferVirtualAddressSegment.length / 2;
        for(unsigned int j = 0; j < transferSize; j++)
        {
            reinterpret_cast<uint8_t*>(descriptorBufferVirtualAddressSegment.address)[j] = j % 0xFF;
        }

        // setup the descriptor in the ring
        descriptorRing[i]                         = { 0 };
        descriptorRing[i].transferSize            = transferSize & 0x7FFFFFF;
        descriptorRing[i].descriptorValid         = 1;
        descriptorRing[i].lowerSourceAddress      = static_cast<uint32_t>(descriptorBufferPhysicalAddressSegment.address);
        descriptorRing[i].upperSourceAddress      = ((descriptorBufferPhysicalAddressSegment.address >> 32) & 0xFFFF);
        descriptorRing[i].lowerDestinationAddress = static_cast<uint32_t>(descriptorBufferPhysicalAddressSegment.address + transferSize);
        descriptorRing[i].upperDestinationAddress = (((descriptorBufferPhysicalAddressSegment.address + transferSize) >> 32) & 0xFFFF);

        if(i == (numDescriptors - 1))
        {
            // interrupt when the last descriptor is done
            descriptorRing[i].interruptWhenDoneWithDescriptor = 1;
        }
    }

    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelDescriptorRingRingSizeOffset,
                                    numDescriptors);
    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelMaximumPrefetchLimitOffset,
                                    kPEX8733DefaultDMADescriptorPrefetchLimit);

    uint32_t interruptControlStatus = 0;
    ivars->pciDevice->MemoryRead32(kPEX8733MemoryIndex,
                                   configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelInterruptControlStatusRegisterOffset,
                                   &interruptControlStatus);

    // clear interrupts (clear on write)
    interruptControlStatus |= kPEX8733DMAChannelInterruptControlStatusInterruptStatusRange;

    // enable interrupts
    interruptControlStatus |= kPEX8733DMAChannelInterruptControlStatusEnableRange;

    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelInterruptControlStatusRegisterOffset,
                                    interruptControlStatus);

    // Enable DMA Block Mode, disable descriptor completion status writeback, and clear any active status bits
    uint32_t controlStatus = 0;
    ivars->pciDevice->MemoryRead32(kPEX8733MemoryIndex,
                                   configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelControlStatusRegisterOffset,
                                   &controlStatus);

    controlStatus &= (~kPEX8733DMAChannelControlStatusDescriptorModeSelectRange);
    controlStatus &= ~kPEX8733DMAChannelControlStatusCompletionStatusWriteBackEnable;
    controlStatus |= kPEX8733DMAChannelControlStatusStart | kPEX8733DMAChannelControlStatusRingStopMode | kPEX8733DMAChannelControlStatusDescriptorModeSelectOffChip;

    // clear any active status bits
    controlStatus |= kPEX8733DMAChannelControlStatusRange;

    // start DMA transfer
    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelControlStatusRegisterOffset,
                                    controlStatus);

    // Wait for transfer to complete
    ivars->pciDevice->MemoryRead32(kPEX8733MemoryIndex,
                                   configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelInterruptControlStatusRegisterOffset,
                                   &interruptControlStatus);
    while(((interruptControlStatus & (kPEX8733DMAChannelInterruptControlStatusDescriptorDoneInterruptStatus
                                      | kPEX8733DMAChannelInterruptControlStatusErrorInterruptStatus
                                      | kPEX8733DMAChannelInterruptControlStatusInvalidDescriptorInterruptStatus
                                      | kPEX8733DMAChannelInterruptControlStatusGracefulPauseDoneInterruptStatus
                                      | kPEX8733DMAChannelInterruptControlStatusImmediatePauseDoneInterruptStatus)) == 0))
    {
        debugLog("interrupt control status 0x%x\n", interruptControlStatus);

        IOSleep(50);

        ivars->pciDevice->MemoryRead32(kPEX8733MemoryIndex,
                                       configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelInterruptControlStatusRegisterOffset,
                                       &interruptControlStatus);
    }

    if((interruptControlStatus & kPEX8733DMAChannelInterruptControlStatusDescriptorDoneInterruptStatus) == 0)
    {
        debugLog("interrupt control status 0x%x, failed to complete transfer\n", interruptControlStatus);
    }
    else
    {
        for(int j = 0; j < numDescriptors; j++)
        {
            IOAddressSegment descriptorBufferVirtualAddressSegment = { 0 };
            descriptors[j]->GetAddressRange(&descriptorBufferVirtualAddressSegment);

            if(memcmp(reinterpret_cast<void*>(descriptorBufferVirtualAddressSegment.address),
                      reinterpret_cast<void*>(descriptorBufferVirtualAddressSegment.address + (descriptorBufferVirtualAddressSegment.length / 2)),
                      (descriptorBufferVirtualAddressSegment.length / 2)) != 0)
            {
                debugLog("descriptor [%u]: source and destination do not match\n", j);
            }
            else
            {
                debugLog("dma[%u] successful\n", j);
            }
        }

    }

    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelDescriptorRingAddressLowerOffset,
                                    0);
    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelDescriptorRingAddressUpperOffset,
                                    0);
    ivars->pciDevice->MemoryWrite32(kPEX8733MemoryIndex,
                                    configurationSpaceOffset(kPEX8733DefaultChannel) + kPEX8733DMAChannelDescriptorRingNextDescriptorAddressLowerOffset,
                                    0);

    OSSafeReleaseNULL(ringBuffer);
    for(unsigned int i = 0; i < numDescriptors; i++)
    {
        OSSafeReleaseNULL(descriptors[i]);
    }
    IOFree(descriptors, sizeof(IOBufferMemoryDescriptor*) * numDescriptors);

    return result;
}
