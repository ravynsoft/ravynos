/*
 * IOUSBInterface.cpp - PureDarwin reconstruction (Apple's original was 0
 * bytes; only IOUSBInterface.h survived). Implements withDescriptors/init,
 * property publishing (so IOKit personality matching on interface class/
 * subclass/protocol/vendor/product works), and FindNextPipe/GetPipeObj
 * (synchronous, no command gate - matches IOUSBController's approach here).
 */

#include <IOKit/usb/IOUSBInterface.h>
#include <IOKit/usb/IOUSBDevice.h>
#include <IOKit/usb/IOUSBPipe.h>
#include <IOKit/usb/IOUSBController.h>
#include <IOKit/usb/IOUSBLog.h>
#include <IOKit/IOLib.h>

#define super IOUSBNub

OSDefineMetaClassAndStructors(IOUSBInterface, IOUSBNub)

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

IOUSBInterface *IOUSBInterface::withDescriptors(const IOUSBConfigurationDescriptor *cfDesc,
                                                 const IOUSBInterfaceDescriptor *ifDesc)
{
    IOUSBInterface *me = OSTypeAlloc(IOUSBInterface);
    if (me && !me->init(cfDesc, ifDesc)) {
        me->release();
        return NULL;
    }
    return me;
}

bool IOUSBInterface::init(const IOUSBConfigurationDescriptor *cfDesc, const IOUSBInterfaceDescriptor *ifDesc)
{
    if (!super::init())
        return false;

    _expansionData = (ExpansionData *)IOMalloc(sizeof(ExpansionData));
    if (!_expansionData)
        return false;
    bzero(_expansionData, sizeof(ExpansionData));

    bzero(_pipeList, sizeof(_pipeList));
    _configDesc = cfDesc;
    _interfaceDesc = ifDesc;
    _device = NULL;

    _bInterfaceNumber   = ifDesc->bInterfaceNumber;
    _bAlternateSetting  = ifDesc->bAlternateSetting;
    _bNumEndpoints      = ifDesc->bNumEndpoints;
    _bInterfaceClass    = ifDesc->bInterfaceClass;
    _bInterfaceSubClass = ifDesc->bInterfaceSubClass;
    _bInterfaceProtocol = ifDesc->bInterfaceProtocol;
    _iInterface         = ifDesc->iInterface;
    return true;
}

bool IOUSBInterface::start(IOService *provider)
{
    _device = OSDynamicCast(IOUSBDevice, provider);
    if (!super::start(provider))
        return false;
    if (!_device)
        return false;
    SetProperties();
    CreatePipes();
    return true;
}

void IOUSBInterface::stop(IOService *provider)
{
    ClosePipes();
    super::stop(provider);
}

bool IOUSBInterface::finalize(IOOptionBits options) { return super::finalize(options); }
bool IOUSBInterface::terminate(IOOptionBits options) { return super::terminate(options); }

bool IOUSBInterface::handleIsOpen(const IOService *forClient) const { return super::handleIsOpen(forClient); }
bool IOUSBInterface::handleOpen(IOService *forClient, IOOptionBits options, void *arg)
{
    if (!_expansionData || !forClient)
        return false;
    if (_expansionData->_openClient && _expansionData->_openClient != forClient)
        return false;
    if (!super::handleOpen(forClient, options, arg))
        return false;
    _expansionData->_openClient = forClient;
    return true;
}
void IOUSBInterface::handleClose(IOService *forClient, IOOptionBits options)
{
    if (_expansionData && _expansionData->_openClient == forClient)
        _expansionData->_openClient = NULL;
    super::handleClose(forClient, options);
}
bool IOUSBInterface::open(IOService *forClient, IOOptionBits options, void *arg) { return super::open(forClient, options, arg); }
void IOUSBInterface::close(IOService *forClient, IOOptionBits options) { super::close(forClient, options); }

IOReturn IOUSBInterface::message(UInt32 type, IOService *provider, void *argument)
{
    return super::message(type, provider, argument);
}

IOReturn IOUSBInterface::DeviceRequest(IOUSBDevRequest *request, IOUSBCompletion *completion)
{
    if (!_device)
        return kIOReturnNotAttached;
    return _device->DeviceRequest(request, completion);
}

IOReturn IOUSBInterface::DeviceRequest(IOUSBDevRequestDesc *request, IOUSBCompletion *completion)
{
    if (!_device)
        return kIOReturnNotAttached;
    return _device->DeviceRequest(request, completion);
}

bool IOUSBInterface::matchPropertyTable(OSDictionary *table, SInt32 *score)
{
    if (!super::matchPropertyTable(table, score))
        return false;
    if (!matchUSBNumber(table, kUSBInterfaceNumber, _bInterfaceNumber))
        return false;
    if (!matchUSBNumber(table, kUSBInterfaceClass, _bInterfaceClass))
        return false;
    if (!matchUSBNumber(table, kUSBInterfaceSubClass, _bInterfaceSubClass))
        return false;
    if (!matchUSBNumber(table, kUSBInterfaceProtocol, _bInterfaceProtocol))
        return false;
    if (_device) {
        if (!matchUSBNumber(table, kUSBVendorID, _device->GetVendorID()))
            return false;
        if (!matchUSBNumber(table, kUSBProductID, _device->GetProductID()))
            return false;
        if (!matchUSBNumber(table, kUSBDeviceReleaseNumber, _device->GetDeviceRelease()))
            return false;
    }
    return true;
}

void IOUSBInterface::free()
{
    ClosePipes();
    if (_expansionData) { IOFree(_expansionData, sizeof(ExpansionData)); _expansionData = NULL; }
    super::free();
}

void IOUSBInterface::SetProperties(void)
{
    setProperty(kUSBInterfaceNumber, (unsigned long long)_bInterfaceNumber, 8);
    setProperty(kUSBInterfaceClass, (unsigned long long)_bInterfaceClass, 8);
    setProperty(kUSBInterfaceSubClass, (unsigned long long)_bInterfaceSubClass, 8);
    setProperty(kUSBInterfaceProtocol, (unsigned long long)_bInterfaceProtocol, 8);
    if (_device) {
        setProperty(kUSBVendorID, (unsigned long long)_device->GetVendorID(), 16);
        setProperty(kUSBProductID, (unsigned long long)_device->GetProductID(), 16);
        setProperty(kUSBDeviceReleaseNumber, (unsigned long long)_device->GetDeviceRelease(), 16);
    }
}

IOReturn IOUSBInterface::CreatePipes(void)
{
    if (!_device || !_configDesc || !_interfaceDesc) return kIOReturnNotAttached;

    const UInt8 *base = (const UInt8 *)_configDesc;
    UInt16 total = _configDesc->wTotalLength;
    UInt32 off = (UInt32)((const UInt8 *)_interfaceDesc - base) + _interfaceDesc->bLength;
    UInt8 found = 0;

    while (off + 2 <= total && found < _bNumEndpoints) {
        UInt8 len = base[off];
        UInt8 type = base[off + 1];
        if (len < 2 || off + len > total) break;

        if (type == kUSBInterfaceDesc) {
            /* Ran into the next interface (or alt setting) - endpoints for
             * this interface/altsetting are exhausted. */
            break;
        }
        if (type == kUSBEndpointDesc && len >= 7) {
            const IOUSBEndpointDescriptor *epd = (const IOUSBEndpointDescriptor *)(base + off);
            IOUSBPipe *pipe = _device->MakePipe(epd, this);
            if (pipe) {
                UInt8 idx = (epd->bEndpointAddress & 0x0F) +
                            ((epd->bEndpointAddress & 0x80) ? (kUSBMaxPipes / 2) : 0);
                if (idx < kUSBMaxPipes) _pipeList[idx] = pipe;
            }
            found++;
        }
        off += len;
    }
    return kIOReturnSuccess;
}

void IOUSBInterface::ClosePipes(void)
{
    for (int i = 0; i < kUSBMaxPipes; i++) {
        if (_pipeList[i]) {
            _pipeList[i]->Abort();
            _pipeList[i]->release();
            _pipeList[i] = NULL;
        }
    }
}

IOReturn IOUSBInterface::ResetPipes(void) { return kIOReturnSuccess; }
IOReturn IOUSBInterface::AbortPipesGated(void) { return kIOReturnSuccess; }
IOReturn IOUSBInterface::ClosePipesGated(bool close) { ClosePipes(); return kIOReturnSuccess; }
IOReturn IOUSBInterface::ReopenPipesGated() { return CreatePipes(); }
void IOUSBInterface::RememberStreamsGated(void) { }
IOReturn IOUSBInterface::RecreateStreamsGated(void) { return kIOReturnSuccess; }

IOUSBPipe *IOUSBInterface::GetPipeObj(UInt8 index)
{
    if (index >= kUSBMaxPipes) return NULL;
    return _pipeList[index];
}

IOUSBPipe *IOUSBInterface::FindNextPipe(IOUSBPipe *current, IOUSBFindEndpointRequest *request)
{
    return FindNextPipe(current, request, false);
}

IOUSBPipe *IOUSBInterface::FindNextPipe(IOUSBPipe *current, IOUSBFindEndpointRequest *request, bool withRetain)
{
    int start = 0;
    if (current) {
        for (int i = 0; i < kUSBMaxPipes; i++)
            if (_pipeList[i] == current) { start = i + 1; break; }
    }
    for (int i = start; i < kUSBMaxPipes; i++) {
        IOUSBPipe *p = _pipeList[i];
        if (!p) continue;
        if (request) {
            if (request->type != kUSBAnyType && request->type != p->GetType()) continue;
            if (request->direction != kUSBAnyDirn && request->direction != p->GetDirection()) continue;
        }
        if (withRetain) p->retain();
        return p;
    }
    return NULL;
}

const IOUSBInterfaceDescriptor *IOUSBInterface::FindNextAltInterface(const IOUSBInterfaceDescriptor *current,
                                                                      IOUSBFindInterfaceRequest *request)
{
    if (!_configDesc) return NULL;
    const UInt8 *base = (const UInt8 *)_configDesc;
    UInt16 total = _configDesc->wTotalLength;
    UInt32 off = current ? (UInt32)((const UInt8 *)current - base) + current->bLength : 0;

    while (off + 2 <= total) {
        UInt8 len = base[off];
        UInt8 type = base[off + 1];
        if (len < 2 || off + len > total) break;
        if (type == kUSBInterfaceDesc && len >= sizeof(IOUSBInterfaceDescriptor)) {
            const IOUSBInterfaceDescriptor *ifd = (const IOUSBInterfaceDescriptor *)(base + off);
            if (ifd->bInterfaceNumber == _bInterfaceNumber)
                return ifd;
        }
        off += len;
    }
    return NULL;
}

UInt8 IOUSBInterface::hex2char(UInt8 digit)
{
    return (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
}

UInt16 IOUSBInterface::CalculateFullMaxPacketSize(IOUSBEndpointDescriptor *ed,
                                                   IOUSBSuperSpeedEndpointCompanionDescriptor *sscd)
{
    return ed ? ed->wMaxPacketSize : 0;
}
