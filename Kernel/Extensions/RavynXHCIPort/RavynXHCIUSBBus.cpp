#include "RavynXHCIUSBBus.h"
#include "RavynXHCIPort.h"
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOLib.h>

#define super IOUSBController

OSDefineMetaClassAndStructors(RavynXHCIUSBBus, IOUSBController)

bool RavynXHCIUSBBus::initWithPort(RavynXHCIPort *port)
{
    if (!init(NULL)) return false;
    fPort = port;
    bzero(fBulkConfigured, sizeof(fBulkConfigured));
    bzero(fIntrConfigured, sizeof(fIntrConfigured));
    return true;
}

IOReturn RavynXHCIUSBBus::UIMOpenPipe(USBDeviceAddress address, UInt8 speed, Endpoint *endpoint)
{
    UInt32 slotId = address;
    if (slotId >= 64 || !endpoint) return kIOReturnBadArgument;

    switch (endpoint->transferType) {
        case kUSBControl:
            /* Endpoint 0 is already set up by addressDevice() at slot
             * address time - nothing to do. */
            return kIOReturnSuccess;

        case kUSBBulk:
            if (fBulkConfigured[slotId])
                return kIOReturnSuccess; /* single ring pair per slot, see header note */
            {
                UInt8 inEp  = (endpoint->direction == kUSBIn)  ? endpoint->number : 0;
                UInt8 outEp = (endpoint->direction == kUSBOut) ? endpoint->number : 0;
                /* We only ever see one direction per OpenPipe call, but
                 * configureBulkEndpoints() wants both at once - the other
                 * side just won't be usable until/unless it's also opened.
                 * Real composite devices with only one bulk direction are
                 * the common case this covers. */
                if (!fPort->configureBulkEndpoints(slotId, inEp, endpoint->maxPacketSize,
                                                    outEp, endpoint->maxPacketSize))
                    return kIOReturnError;
                fBulkConfigured[slotId] = true;
            }
            return kIOReturnSuccess;

        case kUSBInterrupt:
            if (endpoint->direction != kUSBIn)
                return kIOReturnUnsupported; /* no interrupt-OUT support in the UIM */
            if (fIntrConfigured[slotId])
                return kIOReturnSuccess;
            if (!fPort->configureInterruptInEndpoint(slotId, endpoint->number,
                                                       endpoint->maxPacketSize, endpoint->interval))
                return kIOReturnError;
            fIntrConfigured[slotId] = true;
            return kIOReturnSuccess;

        default:
            return kIOReturnUnsupported; /* isoc: no UIM support */
    }
}

IOReturn RavynXHCIUSBBus::UIMClosePipe(USBDeviceAddress address, Endpoint *endpoint)
{
    /* No per-endpoint teardown primitive in RavynXHCIPort short of tearing
     * down the whole slot - treat as a no-op rather than disturb sibling
     * endpoints sharing the slot's single bulk/interrupt ring pair. */
    return kIOReturnSuccess;
}

IOReturn RavynXHCIUSBBus::UIMAbortPipe(USBDeviceAddress address, Endpoint *endpoint)
{
    return kIOReturnSuccess;
}

IOReturn RavynXHCIUSBBus::UIMClearPipeStall(USBDeviceAddress address, Endpoint *endpoint)
{
    return kIOReturnSuccess;
}

IOReturn RavynXHCIUSBBus::UIMDeviceRequest(IOUSBDevRequest *request, USBDeviceAddress address)
{
    UInt32 slotId = address;
    if (slotId >= 64 || !request) return kIOReturnBadArgument;

    USBSetupPacket setup;
    setup.bmRequestType = request->bmRequestType;
    setup.bRequest      = request->bRequest;
    setup.wValue        = request->wValue;
    setup.wIndex        = request->wIndex;
    setup.wLength        = request->wLength;

    bool in = (request->bmRequestType & 0x80) != 0;
    bool ok = fPort->controlTransfer(slotId, setup, request->pData, request->wLength, in);
    if (ok) request->wLenDone = request->wLength;
    return ok ? kIOReturnSuccess : kIOReturnError;
}

IOReturn RavynXHCIUSBBus::UIMReadWrite(IOMemoryDescriptor *buffer, USBDeviceAddress address,
                                       Endpoint *endpoint, bool isWrite)
{
    UInt32 slotId = address;
    if (slotId >= 64 || !endpoint || !buffer) return kIOReturnBadArgument;
    if (endpoint->transferType != kUSBBulk && endpoint->transferType != kUSBInterrupt)
        return kIOReturnUnsupported;

    UInt32 len = (UInt32)buffer->getLength();
    if (!len) return kIOReturnSuccess;

    if (endpoint->transferType == kUSBInterrupt) {
        if (isWrite)
            return kIOReturnUnsupported;
        return fPort->interruptTransfer(slotId, endpoint->number, buffer, len, 20);
    }

    IOBufferMemoryDescriptor *bounce =
        IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, kIODirectionInOut, len);
    if (!bounce) return kIOReturnNoMemory;

    if (isWrite)
        buffer->readBytes(0, bounce->getBytesNoCopy(), len);

    bool ok = fPort->bulkTransfer(slotId, endpoint->number, !isWrite, bounce, len, 5000);

    if (ok && !isWrite)
        buffer->writeBytes(0, bounce->getBytesNoCopy(), len);

    bounce->release();
    return ok ? kIOReturnSuccess : kIOReturnError;
}
