/*
 * Reconstructed IOUSBControllerV2.
 *
 * Apple's open-source drop shipped IOUSBControllerV2.cpp as a 0-byte file, so
 * the class had no compiled metaclass. Any concrete controller deriving from it
 * (AppleUSBOHCI/EHCI/UHCI all do, via the IOUSBControllerV3 typedef) would then
 * GP-fault in OSMetaClass::postModLoad at kext-load time, because the superclass
 * metaclass symbol resolved to garbage under -undefined dynamic_lookup.
 *
 * This provides a real metaclass plus the full concrete-virtual surface so the
 * vtable is complete. The high-speed-split / isoch-queue machinery is stubbed
 * (returns kIOReturnUnsupported / NULL); concrete controllers override the pure
 * virtual UIMCreate*Endpoint entry points themselves. Functionality over form.
 */

#include <IOKit/IOLib.h>
#include <IOKit/usb/IOUSBControllerV2.h>
#include <IOKit/usb/IOUSBControllerListElement.h>

#define super IOUSBController

OSDefineMetaClassAndAbstractStructors(IOUSBControllerV2, IOUSBController)

OSMetaClassDefineReservedUnused(IOUSBControllerV2, 27);
OSMetaClassDefineReservedUnused(IOUSBControllerV2, 28);
OSMetaClassDefineReservedUnused(IOUSBControllerV2, 29);

bool
IOUSBControllerV2::init(OSDictionary *propTable)
{
    if (!super::init(propTable))
        return false;

    _v2ExpansionData = (V2ExpansionData *)IOMalloc(sizeof(V2ExpansionData));
    if (!_v2ExpansionData)
        return false;
    bzero(_v2ExpansionData, sizeof(V2ExpansionData));

    bzero(_highSpeedHub, sizeof(_highSpeedHub));
    bzero(_highSpeedPort, sizeof(_highSpeedPort));
    return true;
}

bool
IOUSBControllerV2::start(IOService *provider)
{
    return super::start(provider);
}

void
IOUSBControllerV2::free()
{
    if (_v2ExpansionData) {
        IOFree(_v2ExpansionData, sizeof(V2ExpansionData));
        _v2ExpansionData = NULL;
    }
    super::free();
}

void
IOUSBControllerV2::UpdateTopology(USBDeviceAddress deviceAddress, UInt8 speed,
                                  USBDeviceAddress hubAddress, int port)
{
    if (deviceAddress < 128) {
        _highSpeedHub[deviceAddress] = 0;
        _highSpeedPort[deviceAddress] = 0;
    }
}

IOReturn
IOUSBControllerV2::OpenPipe(USBDeviceAddress address, UInt8 speed, Endpoint *endpoint)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::CreateDevice(IOUSBDevice *newDevice, USBDeviceAddress deviceAddress,
                                UInt8 maxPacketSize, UInt8 speed, UInt32 powerAvailable,
                                USBDeviceAddress hub, int port)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::ConfigureDeviceZero(UInt8 maxPacketSize, UInt8 speed,
                                       USBDeviceAddress hub, int port)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::AddHSHub(USBDeviceAddress highSpeedHub, UInt32 flags)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::UIMHubMaintenance(USBDeviceAddress highSpeedHub, UInt32 highSpeedPort,
                                     UInt32 command, UInt32 flags)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::RemoveHSHub(USBDeviceAddress highSpeedHub)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::SetTestMode(UInt32 mode, UInt32 port)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::UIMSetTestMode(UInt32 mode, UInt32 port)
{
    return kIOReturnUnsupported;
}

UInt64
IOUSBControllerV2::GetMicroFrameNumber(void)
{
    return 0;
}

void
IOUSBControllerV2::ClearTT(USBDeviceAddress addr, UInt8 endpt, Boolean IN)
{
}

IOReturn
IOUSBControllerV2::ReadV2(IOMemoryDescriptor *buffer, USBDeviceAddress address,
                          Endpoint *endpoint, IOUSBCompletionWithTimeStamp *completion,
                          UInt32 noDataTimeout, UInt32 completionTimeout, IOByteCount reqCount)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::UIMCreateIsochEndpoint(short functionAddress, short endpointNumber,
                                          UInt32 maxPacketSize, UInt8 direction,
                                          USBDeviceAddress highSpeedHub, int highSpeedPort,
                                          UInt8 interval)
{
    return kIOReturnUnsupported;
}

IOUSBControllerIsochEndpoint *
IOUSBControllerV2::AllocateIsochEP(void)
{
    return NULL;
}

IOReturn
IOUSBControllerV2::DeallocateIsochEP(IOUSBControllerIsochEndpoint *pEP)
{
    return kIOReturnUnsupported;
}

IOUSBControllerIsochEndpoint *
IOUSBControllerV2::FindIsochronousEndpoint(short functionNumber, short endpointNumber,
                                           short direction, IOUSBControllerIsochEndpoint **pEDBack)
{
    if (pEDBack)
        *pEDBack = NULL;
    return NULL;
}

IOUSBControllerIsochEndpoint *
IOUSBControllerV2::CreateIsochronousEndpoint(short functionNumber, short endpointNumber,
                                             short direction)
{
    return NULL;
}

void
IOUSBControllerV2::PutTDonToDoList(IOUSBControllerIsochEndpoint *pED,
                                   IOUSBControllerIsochListElement *pTD)
{
}

IOUSBControllerIsochListElement *
IOUSBControllerV2::GetTDfromToDoList(IOUSBControllerIsochEndpoint *pED)
{
    return NULL;
}

void
IOUSBControllerV2::PutTDonDeferredQueue(IOUSBControllerIsochEndpoint *pED,
                                        IOUSBControllerIsochListElement *pTD)
{
}

IOUSBControllerIsochListElement *
IOUSBControllerV2::GetTDfromDeferredQueue(IOUSBControllerIsochEndpoint *pED)
{
    return NULL;
}

void
IOUSBControllerV2::PutTDonDoneQueue(IOUSBControllerIsochEndpoint *pED,
                                    IOUSBControllerIsochListElement *pTD, bool checkDeferred)
{
}

IOUSBControllerIsochListElement *
IOUSBControllerV2::GetTDfromDoneQueue(IOUSBControllerIsochEndpoint *pED)
{
    return NULL;
}

void
IOUSBControllerV2::ReturnIsochDoneQueue(IOUSBControllerIsochEndpoint *pED)
{
}

IODMACommand *
IOUSBControllerV2::GetNewDMACommand(void)
{
    return IODMACommand::withSpecification(kIODMACommandOutputHost64, 64, 0);
}

IOReturn
IOUSBControllerV2::GetLowLatencyOptionsAndPhysicalMask(IOOptionBits *optionBits,
                                                       mach_vm_address_t *physicalMask)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::GetFrameNumberWithTime(UInt64 *frameNumber, AbsoluteTime *theTime)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::ReadStream(UInt32 streamID, IOMemoryDescriptor *buffer,
                              USBDeviceAddress address, Endpoint *endpoint,
                              IOUSBCompletion *completion, UInt32 noDataTimeout,
                              UInt32 completionTimeout, IOByteCount reqCount)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::WriteStream(UInt32 streamID, IOMemoryDescriptor *buffer,
                               USBDeviceAddress address, Endpoint *endpoint,
                               IOUSBCompletion *completion, UInt32 noDataTimeout,
                               UInt32 completionTimeout, IOByteCount reqCount)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::OpenSSPipe(USBDeviceAddress address, UInt8 speed, Endpoint *endpoint,
                              UInt32 maxStreams, UInt32 maxBurstAndMult)
{
    return kIOReturnUnsupported;
}

IOReturn
IOUSBControllerV2::UpdateDeviceAddress(USBDeviceAddress oldDeviceAddress,
                                       USBDeviceAddress newDeviceAddress, UInt8 speed,
                                       USBDeviceAddress hubAddress, int port)
{
    return kIOReturnUnsupported;
}
