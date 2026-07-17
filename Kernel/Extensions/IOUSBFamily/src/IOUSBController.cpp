/*
 * IOUSBController.cpp - PureDarwin reconstruction (see IOUSBController.h).
 * Thin synchronous dispatch onto the UIM* virtuals a concrete controller
 * (e.g. RavynXHCIUSBBus) implements directly against its own ring/slot
 * logic. No command gates, no DMA-command machinery - functionality over
 * ABI fidelity to Apple's original (which never shipped its source anyway).
 */

#include <IOKit/usb/IOUSBController.h>
#include <IOKit/usb/IOUSBDevice.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOBufferMemoryDescriptor.h>

#define super IOUSBBus

/* See USB.h - only ever referenced by address, never actually called. */
void IOUSBSyncCompletion(void *target, void *parameter, IOReturn status, UInt32 bufferSizeRemaining) { }
void IOUSBSyncIsoCompletion(void *target, void *parameter, IOReturn status, UInt32 bufferSizeRemaining) { }

OSDefineMetaClassAndAbstractStructors(IOUSBController, IOUSBBus)

bool IOUSBController::init(OSDictionary *propTable)
{
    if (!super::init(propTable))
        return false;
    _reqLock = IOLockAlloc();
    return _reqLock != NULL;
}

bool IOUSBController::start(IOService *provider)
{
    return super::start(provider);
}

void IOUSBController::free()
{
    if (_reqLock) { IOLockFree(_reqLock); _reqLock = NULL; }
    super::free();
}

IOReturn IOUSBController::OpenPipe(USBDeviceAddress address, UInt8 speed, Endpoint *endpoint)
{
    return UIMOpenPipe(address, speed, endpoint);
}

IOReturn IOUSBController::ClosePipe(USBDeviceAddress address, Endpoint *endpoint)
{
    return UIMClosePipe(address, endpoint);
}

IOReturn IOUSBController::AbortPipe(USBDeviceAddress address, Endpoint *endpoint)
{
    return UIMAbortPipe(address, endpoint);
}

IOReturn IOUSBController::ResetPipe(USBDeviceAddress address, Endpoint *endpoint)
{
    return UIMClearPipeStall(address, endpoint);
}

IOReturn IOUSBController::ClearPipeStall(USBDeviceAddress address, Endpoint *endpoint)
{
    return UIMClearPipeStall(address, endpoint);
}

IOReturn IOUSBController::DeviceRequest(IOUSBDevRequest *request, USBDeviceAddress address,
                                         IOUSBCompletion *completion)
{
    IOLockLock(_reqLock);
    IOReturn ret = UIMDeviceRequest(request, address);
    IOLockUnlock(_reqLock);

    if (completion && completion->action)
        (*completion->action)(completion->target, completion->parameter, ret,
                               request->wLenDone);
    return ret;
}

IOReturn IOUSBController::DeviceRequest(IOUSBDevRequest *request, IOUSBCompletion *completion,
                                         USBDeviceAddress address, UInt32 endpointNumber,
                                         UInt32 noDataTimeout, UInt32 completionTimeout)
{
    /* Our UIM is fully synchronous - endpointNumber/timeout refinements
     * this overload exists for in real Apple source aren't supported. */
    return DeviceRequest(request, address, completion);
}

IOReturn IOUSBController::DeviceRequest(IOUSBDevRequestDesc *request, IOUSBCompletion *completion,
                                         USBDeviceAddress address, UInt32 endpointNumber,
                                         UInt32 noDataTimeout, UInt32 completionTimeout)
{
    return DeviceRequest(request, address, completion);
}

IOReturn IOUSBController::DeviceRequest(IOUSBDevRequestDesc *request, USBDeviceAddress address,
                                         IOUSBCompletion *completion)
{
    /* IOUSBDevRequestDesc carries an IOMemoryDescriptor* instead of a raw
     * pointer for pData; bounce it through a flat buffer since every UIM
     * we have works in terms of flat buffers, not IOMemoryDescriptors, for
     * control transfers. */
    IOReturn ret;
    UInt32 len = request->wLength;
    void *buf = len ? IOMalloc(len) : NULL;

    if (len && !buf)
        return kIOReturnNoMemory;

    if (len && (request->bmRequestType & 0x80) == 0 && request->pData)
        request->pData->readBytes(0, buf, len);

    IOUSBDevRequest flat;
    flat.bmRequestType = request->bmRequestType;
    flat.bRequest      = request->bRequest;
    flat.wValue        = request->wValue;
    flat.wIndex        = request->wIndex;
    flat.wLength       = request->wLength;
    flat.pData         = buf;
    flat.wLenDone      = 0;

    IOLockLock(_reqLock);
    ret = UIMDeviceRequest(&flat, address);
    IOLockUnlock(_reqLock);

    if (len && (request->bmRequestType & 0x80) && request->pData && ret == kIOReturnSuccess)
        request->pData->writeBytes(0, buf, flat.wLenDone);

    request->wLenDone = flat.wLenDone;
    if (buf) IOFree(buf, len);

    if (completion && completion->action)
        (*completion->action)(completion->target, completion->parameter, ret, flat.wLenDone);
    return ret;
}

IOReturn IOUSBController::Read(IOMemoryDescriptor *buffer, USBDeviceAddress address,
                                Endpoint *endpoint, IOUSBCompletion *completion)
{
    IOReturn ret = UIMReadWrite(buffer, address, endpoint, false);
    if (completion && completion->action)
        (*completion->action)(completion->target, completion->parameter, ret,
                               buffer ? (UInt32)buffer->getLength() : 0);
    return ret;
}

IOReturn IOUSBController::Read(IOMemoryDescriptor *buffer, USBDeviceAddress address,
                                Endpoint *endpoint, IOUSBCompletion *completion,
                                UInt32 noDataTimeout, UInt32 completionTimeout, IOByteCount reqCount)
{
    /* Our UIM is fully synchronous - the timeout/reqCount refinements this
     * overload exists for in real Apple source (per-call timeout tuning,
     * partial-length reads) aren't things any UIM we have supports. */
    return Read(buffer, address, endpoint, completion);
}

IOReturn IOUSBController::Write(IOMemoryDescriptor *buffer, USBDeviceAddress address,
                                 Endpoint *endpoint, IOUSBCompletion *completion)
{
    IOReturn ret = UIMReadWrite(buffer, address, endpoint, true);
    if (completion && completion->action)
        (*completion->action)(completion->target, completion->parameter, ret,
                               buffer ? (UInt32)buffer->getLength() : 0);
    return ret;
}

IOReturn IOUSBController::Write(IOMemoryDescriptor *buffer, USBDeviceAddress address,
                                 Endpoint *endpoint, IOUSBCompletion *completion,
                                 UInt32 noDataTimeout, UInt32 completionTimeout, IOByteCount reqCount)
{
    return Write(buffer, address, endpoint, completion);
}

IOReturn IOUSBController::IsocIO(IOMemoryDescriptor *buffer, UInt64 frameStart, UInt32 numFrames,
                                  void *frameList, USBDeviceAddress address, Endpoint *endpoint,
                                  IOUSBCompletion *completion)
{
    /* No isoc transport in any UIM we have (keyboard/mouse/mass-storage all
     * use control/bulk/interrupt) - not implemented. */
    return kIOReturnUnsupported;
}

IOReturn IOUSBController::IsocIO(IOMemoryDescriptor *buffer, UInt64 frameStart, UInt32 numFrames,
                                  void *frameList, USBDeviceAddress address, Endpoint *endpoint,
                                  IOUSBCompletion *completion, UInt32 updateFrequency)
{
    return kIOReturnUnsupported;
}

IOReturn IOUSBController::IsocIO(IOMemoryDescriptor *buffer, UInt64 frameStart, UInt32 numFrames,
                                  void *frameList, USBDeviceAddress address, Endpoint *endpoint,
                                  IOUSBLowLatencyIsocCompletion *completion, UInt32 updateFrequency)
{
    return kIOReturnUnsupported;
}

UInt32 IOUSBController::GetBandwidthAvailableForDevice(USBDeviceAddress address)
{
    /* We don't do real USB bandwidth accounting (no isoc support); report a
     * generous fixed budget so bandwidth-aware clients don't refuse to
     * attach. */
    return 90 * 1024 * 1024 / 8;
}

IOUSBDevice *IOUSBController::CreateAndConfigureDevice(USBDeviceAddress address, UInt8 speed,
                                                        UInt8 maxPacketSize0)
{
    IOUSBDevRequest req;

    IOUSBDevice *dev = IOUSBDevice::NewDevice();
    if (!dev) return NULL;
    if (!dev->init(address, 0 /* busPowerAvailable, unknown */, speed, maxPacketSize0)) {
        dev->release();
        return NULL;
    }
    dev->_controller = this;

    req.bmRequestType = 0x80;
    req.bRequest = kUSBRqGetDescriptor;
    req.wValue = (kUSBDeviceDesc << 8);
    req.wIndex = 0;
    req.wLength = sizeof(IOUSBDeviceDescriptor);
    req.pData = &dev->_descriptor;
    if (UIMDeviceRequest(&req, address) != kIOReturnSuccess || dev->_descriptor.bNumConfigurations == 0) {
        dev->release();
        return NULL;
    }

    UInt8 numConfigs = dev->_descriptor.bNumConfigurations;
    dev->_configList = (IOBufferMemoryDescriptor **)IOMalloc(numConfigs * sizeof(IOBufferMemoryDescriptor *));
    if (!dev->_configList) { dev->release(); return NULL; }
    bzero(dev->_configList, numConfigs * sizeof(IOBufferMemoryDescriptor *));

    for (UInt8 i = 0; i < numConfigs; i++) {
        IOUSBConfigurationDescriptor cfgHdr;
        bzero(&cfgHdr, sizeof(cfgHdr));

        req.bmRequestType = 0x80;
        req.bRequest = kUSBRqGetDescriptor;
        req.wValue = (kUSBConfDesc << 8) | i;
        req.wIndex = 0;
        req.wLength = sizeof(cfgHdr);
        req.pData = &cfgHdr;
        if (UIMDeviceRequest(&req, address) != kIOReturnSuccess || cfgHdr.wTotalLength < sizeof(cfgHdr)) {
            dev->release();
            return NULL;
        }

        IOBufferMemoryDescriptor *buf =
            IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, kIODirectionInOut, cfgHdr.wTotalLength);
        if (!buf) { dev->release(); return NULL; }

        req.wLength = cfgHdr.wTotalLength;
        req.pData = buf->getBytesNoCopy();
        if (UIMDeviceRequest(&req, address) != kIOReturnSuccess) {
            buf->release();
            dev->release();
            return NULL;
        }
        dev->_configList[i] = buf;
    }

    return dev;
}
