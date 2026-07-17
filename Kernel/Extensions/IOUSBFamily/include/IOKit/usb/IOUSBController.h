/*
 * IOUSBController.h - PureDarwin reconstruction.
 *
 * Apple's real IOUSBController.h/.cpp were 0 bytes in every open-source
 * IOUSBFamily drop we could find - only the class's *callers* survived
 * (IOUSBPipe.cpp, IOUSBController_Pipes.cpp, IOUSBDevice.h's declarations,
 * etc). This header is reconstructed from those call sites: it declares
 * exactly the surface those files need, with real Apple-original method
 * *bodies* where they survived (IOUSBController_Pipes.cpp) sitting on top
 * of a synchronous virtual UIM interface instead of Apple's command-gate/
 * DMA-command machinery. Functionality over ABI fidelity - nothing here
 * needs to binary-match a real macOS kernel.
 */

#ifndef _IOKIT_IOUSBCONTROLLER_H
#define _IOKIT_IOUSBCONTROLLER_H

#include <IOKit/usb/IOUSBBus.h>
#include <IOKit/usb/USB.h>
#include <IOKit/IOLocks.h>

class IOUSBDevice;
class IOUSBCommand;

class IOUSBController : public IOUSBBus
{
    OSDeclareAbstractStructors(IOUSBController)

public:
    /* Tidied-up version of an endpoint descriptor, passed between
     * IOUSBPipe/IOUSBDevice and the controller. Field set mined from
     * IOUSBPipe.cpp's _endpoint.* usage. */
    struct Endpoint
    {
        UInt8  number;
        UInt8  direction;      /* kUSBIn / kUSBOut */
        UInt8  transferType;   /* kUSBControl / kUSBBulk / kUSBInterrupt / kUSBIsoc */
        UInt16 maxPacketSize;
        UInt8  interval;
    };

    virtual bool init(OSDictionary *propTable) APPLE_KEXT_OVERRIDE;
    virtual bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;

    /* Pipe lifecycle - real bodies survive in IOUSBController_Pipes.cpp in
     * the original drop (command-gate dispatch there); here they call the
     * synchronous UIM* virtuals directly. */
    virtual IOReturn OpenPipe(USBDeviceAddress address, UInt8 speed, Endpoint *endpoint);
    virtual IOReturn ClosePipe(USBDeviceAddress address, Endpoint *endpoint);
    virtual IOReturn AbortPipe(USBDeviceAddress address, Endpoint *endpoint);
    virtual IOReturn ResetPipe(USBDeviceAddress address, Endpoint *endpoint);
    virtual IOReturn ClearPipeStall(USBDeviceAddress address, Endpoint *endpoint);

    virtual IOReturn DeviceRequest(IOUSBDevRequest *request, USBDeviceAddress address,
                                    IOUSBCompletion *completion = 0);
    virtual IOReturn DeviceRequest(IOUSBDevRequest *request, IOUSBCompletion *completion,
                                    USBDeviceAddress address, UInt32 endpointNumber,
                                    UInt32 noDataTimeout, UInt32 completionTimeout);
    virtual IOReturn DeviceRequest(IOUSBDevRequestDesc *request, USBDeviceAddress address,
                                    IOUSBCompletion *completion = 0);
    virtual IOReturn DeviceRequest(IOUSBDevRequestDesc *request, IOUSBCompletion *completion,
                                    USBDeviceAddress address, UInt32 endpointNumber,
                                    UInt32 noDataTimeout, UInt32 completionTimeout);

    virtual IOReturn Read(IOMemoryDescriptor *buffer, USBDeviceAddress address,
                           Endpoint *endpoint, IOUSBCompletion *completion = 0);
    virtual IOReturn Read(IOMemoryDescriptor *buffer, USBDeviceAddress address,
                           Endpoint *endpoint, IOUSBCompletion *completion,
                           UInt32 noDataTimeout, UInt32 completionTimeout, IOByteCount reqCount = 0);
    virtual IOReturn Write(IOMemoryDescriptor *buffer, USBDeviceAddress address,
                            Endpoint *endpoint, IOUSBCompletion *completion = 0);
    virtual IOReturn Write(IOMemoryDescriptor *buffer, USBDeviceAddress address,
                            Endpoint *endpoint, IOUSBCompletion *completion,
                            UInt32 noDataTimeout, UInt32 completionTimeout, IOByteCount reqCount = 0);

    virtual IOReturn IsocIO(IOMemoryDescriptor *buffer, UInt64 frameStart, UInt32 numFrames,
                             void *frameList, USBDeviceAddress address, Endpoint *endpoint,
                             IOUSBCompletion *completion = 0);
    virtual IOReturn IsocIO(IOMemoryDescriptor *buffer, UInt64 frameStart, UInt32 numFrames,
                             void *frameList, USBDeviceAddress address, Endpoint *endpoint,
                             IOUSBCompletion *completion, UInt32 updateFrequency);
    virtual IOReturn IsocIO(IOMemoryDescriptor *buffer, UInt64 frameStart, UInt32 numFrames,
                             void *frameList, USBDeviceAddress address, Endpoint *endpoint,
                             IOUSBLowLatencyIsocCompletion *completion, UInt32 updateFrequency);

    virtual UInt32 GetBandwidthAvailableForDevice(USBDeviceAddress address);

    /* Fetch the device + every configuration descriptor over the control
     * pipe (address must already be addressed/ready for control transfers)
     * and return a populated, unattached IOUSBDevice. Caller attaches/
     * registers it. NULL on any descriptor-fetch failure. A plain
     * (non-virtual) method on the base class rather than the concrete
     * subclass: C++ friendship ("friend class IOUSBController" in
     * IOUSBDevice.h) doesn't extend to subclasses, so this has to live
     * here to get at IOUSBDevice's protected fields. */
    IOUSBDevice *CreateAndConfigureDevice(USBDeviceAddress address, UInt8 speed, UInt8 maxPacketSize0);

    /* UIM interface: the concrete controller (RavynXHCIUSBBus, etc)
     * implements these against whatever ring/slot logic it already has.
     * Named UIM* to match Apple's naming convention, but the signatures
     * here are ours - synchronous, no DMA-command/command-gate plumbing. */
    virtual IOReturn UIMOpenPipe(USBDeviceAddress address, UInt8 speed, Endpoint *endpoint) = 0;
    virtual IOReturn UIMClosePipe(USBDeviceAddress address, Endpoint *endpoint) = 0;
    virtual IOReturn UIMAbortPipe(USBDeviceAddress address, Endpoint *endpoint) = 0;
    virtual IOReturn UIMClearPipeStall(USBDeviceAddress address, Endpoint *endpoint) = 0;

    virtual IOReturn UIMDeviceRequest(IOUSBDevRequest *request, USBDeviceAddress address) = 0;

    virtual IOReturn UIMReadWrite(IOMemoryDescriptor *buffer, USBDeviceAddress address,
                                   Endpoint *endpoint, bool isWrite) = 0;

protected:
    IOLock *_reqLock;
};

#endif /* !_IOKIT_IOUSBCONTROLLER_H */
