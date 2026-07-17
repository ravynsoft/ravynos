/*
 * IOUSBDevice.cpp - PureDarwin reconstruction (Apple's original was 0
 * bytes in every open-source drop found; only IOUSBDevice.h survived).
 * Implements the subset IOUSBCompositeDriver/IOUSBPipe/IOUSBInterface
 * actually call (mined from their surviving source): GetDeviceDescriptor,
 * GetFullConfigurationDescriptor, GetConfigurationDescriptor,
 * FindNextInterfaceDescriptor, SetConfiguration (builds+registers
 * IOUSBInterface nubs so normal IOKit matching finds class drivers),
 * GetBus/GetAddress/GetSpeed/GetBusPowerAvailable/GetVendor-Product-etc,
 * GetPipeZero/MakePipe, DeviceRequest passthrough. Everything else gets a
 * safe no-op/unsupported default rather than Apple's real (lost) behavior -
 * functionality over completeness.
 */

#include <IOKit/usb/IOUSBDevice.h>
#include <IOKit/usb/IOUSBInterface.h>
#include <IOKit/usb/IOUSBPipe.h>
#include <IOKit/usb/IOUSBController.h>
#include <IOKit/usb/IOUSBLog.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOBufferMemoryDescriptor.h>

#define super IOUSBNub

OSDefineMetaClassAndStructors(IOUSBDevice, IOUSBNub)

static bool
matchUSBNumber(OSDictionary *table, const char *key, UInt32 value)
{
    OSObject *obj = table->getObject(key);
    if (!obj)
        return true;
    OSNumber *num = OSDynamicCast(OSNumber, obj);
    if (!num)
        return false;
    return num->unsigned32BitValue() == value;
}

IOUSBDevice *IOUSBDevice::NewDevice(void)
{
    return OSTypeAlloc(IOUSBDevice);
}

bool IOUSBDevice::init(void)
{
    return super::init();
}

bool IOUSBDevice::init(USBDeviceAddress deviceAddress, UInt32 powerAvailable,
                        UInt8 speed, UInt8 maxPacketSize)
{
    if (!super::init())
        return false;

    _expansionData = (ExpansionData *)IOMalloc(sizeof(ExpansionData));
    if (!_expansionData)
        return false;
    bzero(_expansionData, sizeof(ExpansionData));

    _address = deviceAddress;
    _busPowerAvailable = powerAvailable;
    _speed = speed;
    bzero(&_descriptor, sizeof(_descriptor));
    bzero(&_endpointZero, sizeof(_endpointZero));
    _endpointZero.bLength = sizeof(_endpointZero);
    _endpointZero.bDescriptorType = kUSBEndpointDesc;
    _endpointZero.bmAttributes = kUSBControl;
    _endpointZero.wMaxPacketSize = maxPacketSize;
    _controller = NULL;
    _pipeZero = NULL;
    _configList = NULL;
    _interfaceList = NULL;
    _currentConfigValue = 0;
    _numInterfaces = 0;
    return true;
}

bool IOUSBDevice::start(IOService *provider)
{
    if (!super::start(provider))
        return false;
    SetProperties();
    return true;
}

void IOUSBDevice::stop(IOService *provider)
{
    super::stop(provider);
}

bool IOUSBDevice::finalize(IOOptionBits options)
{
    return super::finalize(options);
}

bool IOUSBDevice::terminate(IOOptionBits options)
{
    return super::terminate(options);
}

bool IOUSBDevice::requestTerminate(IOService *provider, IOOptionBits options)
{
    return super::requestTerminate(provider, options);
}

void IOUSBDevice::free(void)
{
    if (_pipeZero) { _pipeZero->release(); _pipeZero = NULL; }

    if (_configList) {
        UInt8 n = _descriptor.bNumConfigurations;
        for (UInt8 i = 0; i < n; i++)
            if (_configList[i]) _configList[i]->release();
        IOFree(_configList, n * sizeof(IOBufferMemoryDescriptor *));
        _configList = NULL;
    }

    if (_expansionData) { IOFree(_expansionData, sizeof(ExpansionData)); _expansionData = NULL; }
    super::free();
}

bool IOUSBDevice::handleIsOpen(const IOService *forClient) const { return super::handleIsOpen(forClient); }
bool IOUSBDevice::handleOpen(IOService *forClient, IOOptionBits options, void *arg) { return super::handleOpen(forClient, options, arg); }
void IOUSBDevice::handleClose(IOService *forClient, IOOptionBits options) { super::handleClose(forClient, options); }

IOReturn IOUSBDevice::message(UInt32 type, IOService *provider, void *argument)
{
    return super::message(type, provider, argument);
}

bool IOUSBDevice::matchPropertyTable(OSDictionary *table, SInt32 *score)
{
    if (!super::matchPropertyTable(table, score))
        return false;
    if (!matchUSBNumber(table, kUSBVendorID, _descriptor.idVendor))
        return false;
    if (!matchUSBNumber(table, kUSBProductID, _descriptor.idProduct))
        return false;
    if (!matchUSBNumber(table, kUSBDeviceReleaseNumber, _descriptor.bcdDevice))
        return false;
    if (!matchUSBNumber(table, kUSBDeviceClass, _descriptor.bDeviceClass))
        return false;
    if (!matchUSBNumber(table, kUSBDeviceSubClass, _descriptor.bDeviceSubClass))
        return false;
    if (!matchUSBNumber(table, kUSBDeviceProtocol, _descriptor.bDeviceProtocol))
        return false;
    return true;
}

void IOUSBDevice::SetPort(void *port) { /* obsolete, no-op */ }

void IOUSBDevice::SetProperties()
{
    setProperty(kUSBVendorID, (unsigned long long)_descriptor.idVendor, 16);
    setProperty(kUSBProductID, (unsigned long long)_descriptor.idProduct, 16);
    setProperty(kUSBDeviceReleaseNumber, (unsigned long long)_descriptor.bcdDevice, 16);
    setProperty(kUSBDeviceClass, (unsigned long long)_descriptor.bDeviceClass, 8);
    setProperty(kUSBDeviceSubClass, (unsigned long long)_descriptor.bDeviceSubClass, 8);
    setProperty(kUSBDeviceProtocol, (unsigned long long)_descriptor.bDeviceProtocol, 8);
}

IOReturn IOUSBDevice::GetDeviceDescriptor(IOUSBDeviceDescriptor *desc, UInt32 size)
{
    if (!desc) return kIOReturnBadArgument;
    bcopy(&_descriptor, desc, size < sizeof(_descriptor) ? size : sizeof(_descriptor));
    return kIOReturnSuccess;
}

const IOUSBConfigurationDescriptor *IOUSBDevice::GetFullConfigurationDescriptor(UInt8 configIndex)
{
    if (!_configList || configIndex >= _descriptor.bNumConfigurations || !_configList[configIndex])
        return NULL;
    return (const IOUSBConfigurationDescriptor *)_configList[configIndex]->getBytesNoCopy();
}

const IOUSBConfigurationDescriptor *IOUSBDevice::FindConfig(UInt8 configValue, UInt8 *configIndexOut)
{
    for (UInt8 i = 0; i < _descriptor.bNumConfigurations; i++) {
        const IOUSBConfigurationDescriptor *cd = GetFullConfigurationDescriptor(i);
        if (cd && cd->bConfigurationValue == configValue) {
            if (configIndexOut) *configIndexOut = i;
            return cd;
        }
    }
    return NULL;
}

IOReturn IOUSBDevice::GetConfigDescriptor(UInt8 configIndex, void *data, UInt32 len)
{
    const IOUSBConfigurationDescriptor *cd = GetFullConfigurationDescriptor(configIndex);
    if (!cd) return kIOReturnBadArgument;
    bcopy(cd, data, len < cd->wTotalLength ? len : cd->wTotalLength);
    return kIOReturnSuccess;
}

IOReturn IOUSBDevice::GetConfigurationDescriptor(UInt8 configValue, void *data, UInt32 len)
{
    const IOUSBConfigurationDescriptor *cd = FindConfig(configValue);
    if (!cd) return kIOReturnBadArgument;
    bcopy(cd, data, len < cd->wTotalLength ? len : cd->wTotalLength);
    return kIOReturnSuccess;
}

IOReturn IOUSBDevice::FindNextInterfaceDescriptor(const IOUSBConfigurationDescriptor *configDescIn,
                                                   const IOUSBInterfaceDescriptor *intfDesc,
                                                   const IOUSBFindInterfaceRequest *request,
                                                   IOUSBInterfaceDescriptor **descOut)
{
    const IOUSBConfigurationDescriptor *configDesc = configDescIn;
    if (!configDesc) configDesc = FindConfig(_currentConfigValue);
    if (!configDesc) return kIOReturnBadArgument;

    const UInt8 *base = (const UInt8 *)configDesc;
    UInt16 total = configDesc->wTotalLength;
    UInt32 off = 0;

    if (intfDesc) {
        const UInt8 *p = (const UInt8 *)intfDesc;
        if (p < base || p >= base + total) return kIOReturnBadArgument;
        off = (UInt32)(p - base) + intfDesc->bLength;
    } else {
        off = configDesc->bLength;
    }

    while (off + 2 <= total) {
        const UInt8 *hdr = base + off;
        UInt8 len = hdr[0];
        UInt8 type = hdr[1];
        if (len < 2 || off + len > total) break;

        if (type == kUSBInterfaceDesc) {
            const IOUSBInterfaceDescriptor *ifd = (const IOUSBInterfaceDescriptor *)hdr;
            bool match = true;
            if (request) {
                if (request->bInterfaceClass != kIOUSBFindInterfaceDontCare &&
                    request->bInterfaceClass != ifd->bInterfaceClass) match = false;
                if (match && request->bInterfaceSubClass != kIOUSBFindInterfaceDontCare &&
                    request->bInterfaceSubClass != ifd->bInterfaceSubClass) match = false;
                if (match && request->bInterfaceProtocol != kIOUSBFindInterfaceDontCare &&
                    request->bInterfaceProtocol != ifd->bInterfaceProtocol) match = false;
                if (match && request->bAlternateSetting != kIOUSBFindInterfaceDontCare &&
                    request->bAlternateSetting != ifd->bAlternateSetting) match = false;
            }
            if (match) {
                if (descOut) *descOut = (IOUSBInterfaceDescriptor *)ifd;
                return kIOReturnSuccess;
            }
        }
        off += len;
    }
    return kIOUSBInterfaceNotFound;
}

IOUSBInterface *IOUSBDevice::GetInterface(const IOUSBInterfaceDescriptor *interface)
{
    /* Not tracking a live interface list (Obsolete per the header comment
     * on _interfaceList) - interfaces are found via the registry once
     * SetConfiguration() has published them, not through this call. */
    return NULL;
}

IOUSBInterface *IOUSBDevice::FindNextInterface(IOUSBInterface *current, IOUSBFindInterfaceRequest *request)
{
    return NULL;
}

OSIterator *IOUSBDevice::CreateInterfaceIterator(IOUSBFindInterfaceRequest *request)
{
    return NULL;
}

IOReturn IOUSBDevice::SetConfiguration(IOService *forClient, UInt8 configValue, bool startInterfaceMatching)
{
    UInt8 configIndex = 0;
    const IOUSBConfigurationDescriptor *cd = FindConfig(configValue, &configIndex);
    if (!cd) return kIOReturnBadArgument;

    IOUSBDevRequest req;
    req.bmRequestType = 0x00;
    req.bRequest = kUSBRqSetConfig;
    req.wValue = configValue;
    req.wIndex = 0;
    req.wLength = 0;
    req.pData = NULL;
    IOReturn ret = _controller->DeviceRequest(&req, _address);
    if (ret != kIOReturnSuccess) return ret;

    _currentConfigValue = configValue;

    /* Walk every interface descriptor in this configuration and publish an
     * IOUSBInterface nub for each - IOKit's normal registry matching then
     * finds IOUSBCompositeDriver / class-specific drivers on its own. */
    const UInt8 *base = (const UInt8 *)cd;
    UInt16 total = cd->wTotalLength;
    UInt32 off = cd->bLength;
    UInt8 seenAltZeroFor[256];
    UInt8 seenCount = 0;
    bzero(seenAltZeroFor, sizeof(seenAltZeroFor));

    while (off + 2 <= total) {
        UInt8 len = base[off];
        UInt8 type = base[off + 1];
        if (len < 2 || off + len > total) break;

        if (type == kUSBInterfaceDesc && len >= sizeof(IOUSBInterfaceDescriptor)) {
            const IOUSBInterfaceDescriptor *ifd = (const IOUSBInterfaceDescriptor *)(base + off);

            /* Only publish the default (alternate setting 0) interface for
             * each interface number - alternate settings are selected later
             * via SetInterface on the published IOUSBInterface, not here. */
            if (ifd->bAlternateSetting == 0) {
                bool already = false;
                for (UInt8 i = 0; i < seenCount; i++)
                    if (seenAltZeroFor[i] == ifd->bInterfaceNumber) already = true;
                if (!already && seenCount < 256) {
                    seenAltZeroFor[seenCount++] = ifd->bInterfaceNumber;

                    IOUSBInterface *intf = IOUSBInterface::withDescriptors(cd, ifd);
                    if (intf) {
                        if (intf->attach(this)) {
                            if (intf->start(this)) {
                                if (startInterfaceMatching)
                                    intf->registerService();
                            } else {
                                intf->detach(this);
                                intf->release();
                            }
                        } else {
                            intf->release();
                        }
                    }
                }
            }
        }
        off += len;
    }

    return kIOReturnSuccess;
}

IOReturn IOUSBDevice::SetConfiguration(IOService *forClient, UInt8 configValue,
                                        bool startInterfaceMatching, bool issueRemoteWakeup)
{
    /* No remote-wakeup signaling support in any UIM we have - same
     * SET_CONFIGURATION either way. */
    return SetConfiguration(forClient, configValue, startInterfaceMatching);
}

IOReturn IOUSBDevice::ResetDevice()
{
    if (!_controller) return kIOReturnNotAttached;
    return _controller->UIMOpenPipe(_address, _speed, NULL) == kIOReturnSuccess
           ? kIOReturnSuccess : kIOReturnUnsupported;
}

USBDeviceAddress IOUSBDevice::GetAddress(void) { return _address; }
UInt8 IOUSBDevice::GetSpeed(void) { return _speed; }
IOUSBController *IOUSBDevice::GetBus(void) { return _controller; }
UInt32 IOUSBDevice::GetBusPowerAvailable(void) { return _busPowerAvailable; }
UInt8 IOUSBDevice::GetMaxPacketSize(void) { return (UInt8)_endpointZero.wMaxPacketSize; }
UInt16 IOUSBDevice::GetVendorID(void) { return _descriptor.idVendor; }
UInt16 IOUSBDevice::GetProductID(void) { return _descriptor.idProduct; }
UInt16 IOUSBDevice::GetDeviceRelease(void) { return _descriptor.bcdDevice; }
UInt8 IOUSBDevice::GetNumConfigurations(void) { return _descriptor.bNumConfigurations; }
UInt8 IOUSBDevice::GetManufacturerStringIndex(void) { return _descriptor.iManufacturer; }
UInt8 IOUSBDevice::GetProductStringIndex(void) { return _descriptor.iProduct; }
UInt8 IOUSBDevice::GetSerialNumberStringIndex(void) { return _descriptor.iSerialNumber; }

IOUSBPipe *IOUSBDevice::GetPipeZero(void)
{
    if (!_pipeZero && _controller) {
        _pipeZero = IOUSBPipe::ToEndpoint(&_endpointZero, _speed, (USBDeviceAddress)_address, _controller);
    }
    return _pipeZero;
}

IOUSBPipe *IOUSBDevice::MakePipe(const IOUSBEndpointDescriptor *ep)
{
    return MakePipe(ep, NULL);
}

IOUSBPipe *IOUSBDevice::MakePipe(const IOUSBEndpointDescriptor *ep, IOUSBInterface *interface)
{
    if (!ep || !_controller) return NULL;
    return IOUSBPipe::ToEndpoint(ep, this, _controller, interface);
}

IOReturn IOUSBDevice::DeviceRequest(IOUSBDevRequest *request, IOUSBCompletion *completion)
{
    if (!_controller) return kIOReturnNotAttached;
    return _controller->DeviceRequest(request, _address, completion);
}

IOReturn IOUSBDevice::DeviceRequest(IOUSBDevRequest *request, UInt32 noDataTimeout,
                                     UInt32 completionTimeout, IOUSBCompletion *completion)
{
    return DeviceRequest(request, completion);
}

IOReturn IOUSBDevice::DeviceRequest(IOUSBDevRequestDesc *request, IOUSBCompletion *completion)
{
    if (!_controller) return kIOReturnNotAttached;
    return _controller->DeviceRequest(request, _address, completion);
}

IOReturn IOUSBDevice::DeviceRequest(IOUSBDevRequestDesc *request, UInt32 noDataTimeout,
                                     UInt32 completionTimeout, IOUSBCompletion *completion)
{
    return DeviceRequest(request, completion);
}

IOReturn IOUSBDevice::GetConfiguration(UInt8 *configNumber)
{
    if (!configNumber) return kIOReturnBadArgument;
    *configNumber = _currentConfigValue;
    return kIOReturnSuccess;
}

IOReturn IOUSBDevice::GetDeviceStatus(USBStatus *status)
{
    if (!status) return kIOReturnBadArgument;
    *status = 0;
    return kIOReturnSuccess;
}

IOReturn IOUSBDevice::GetStringDescriptor(UInt8 index, char *buf, int maxLen, UInt16 lang)
{
    if (!buf || maxLen <= 0) return kIOReturnBadArgument;
    buf[0] = 0;
    if (!index || !_controller) return kIOReturnSuccess;

    UInt8 raw[256];
    bzero(raw, sizeof(raw));
    IOUSBDevRequest req;
    req.bmRequestType = 0x80;
    req.bRequest = kUSBRqGetDescriptor;
    req.wValue = (kUSBStringDesc << 8) | index;
    req.wIndex = lang;
    req.wLength = sizeof(raw);
    req.pData = raw;
    if (_controller->DeviceRequest(&req, _address) != kIOReturnSuccess || raw[0] < 2)
        return kIOReturnSuccess;

    /* UTF-16LE -> naive ASCII (high byte dropped), matches this driver's
     * "functionality over form" scope. */
    int n = (raw[0] - 2) / 2;
    int outLen = (n < maxLen - 1) ? n : maxLen - 1;
    for (int i = 0; i < outLen; i++) buf[i] = (char)raw[2 + i * 2];
    buf[outLen] = 0;
    return kIOReturnSuccess;
}

UInt32 IOUSBDevice::GetChildLocationID(UInt32 parentLocationID, int port)
{
    return parentLocationID | ((UInt32)(port & 0xF) << (4 * 1));
}

const IOUSBDescriptorHeader *IOUSBDevice::FindNextDescriptor(const void *cur, UInt8 descType)
{
    const IOUSBConfigurationDescriptor *cd = FindConfig(_currentConfigValue);
    if (!cd) return NULL;
    const UInt8 *base = (const UInt8 *)cd;
    UInt16 total = cd->wTotalLength;
    UInt32 off = cur ? (UInt32)((const UInt8 *)cur - base) + ((const IOUSBDescriptorHeader *)cur)->bLength
                      : 0;
    while (off + 2 <= total) {
        UInt8 len = base[off];
        UInt8 type = base[off + 1];
        if (len < 2 || off + len > total) break;
        if (descType == 0 || type == descType)
            return (const IOUSBDescriptorHeader *)(base + off);
        off += len;
    }
    return NULL;
}

void IOUSBDevice::DisplayNotEnoughPowerNotice() { /* no console notification stack here */ }
void IOUSBDevice::DisplayUserNotification(UInt32 notificationType) { /* no-op */ }

IOReturn IOUSBDevice::SetFeature(UInt8 feature)
{
    if (!_controller) return kIOReturnNotAttached;
    IOUSBDevRequest req;
    req.bmRequestType = 0x00;
    req.bRequest = kUSBRqSetFeature;
    req.wValue = feature;
    req.wIndex = 0;
    req.wLength = 0;
    req.pData = NULL;
    return _controller->DeviceRequest(&req, _address);
}

IOReturn IOUSBDevice::SuspendDevice(bool suspend)
{
    /* No suspend/resume support in any UIM we have. */
    return kIOReturnUnsupported;
}

IOReturn IOUSBDevice::ReEnumerateDevice(UInt32 options)
{
    return kIOReturnUnsupported;
}

void IOUSBDevice::SetHubParent(IOUSBHubPolicyMaker *hubParent)
{
    _expansionData->_hubPolicyMaker = hubParent;
}

IOUSBHubPolicyMaker *IOUSBDevice::GetHubParent()
{
    return _expansionData ? _expansionData->_hubPolicyMaker : NULL;
}
