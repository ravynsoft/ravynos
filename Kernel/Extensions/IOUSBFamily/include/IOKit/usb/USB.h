/*
 * USB.h - PureDarwin minimal replacement.
 *
 * The real SDK USB.h (from a much newer macOS release than this kext build
 * targets) gates every descriptor/request struct definition behind
 * `#if TARGET_OS_OSX || TARGET_OS_MACCATALYST`, which never activates in
 * this kernel-private cross-compiled kext build (no TargetConditionals.h
 * feature detection wired up the way a normal app build has it) - the
 * whole header compiled down to nothing. Rather than chase that, this is a
 * small self-contained header with exactly the standard-USB-spec structs
 * and constants IOUSBFamily/RavynXHCIPort actually use. Field layouts are
 * the literal USB spec wire format, not something Apple-specific, so
 * there's no fidelity loss versus the real header for what we use.
 */

#ifndef _IOKIT_USB_USB_H
#define _IOKIT_USB_USB_H

#include <IOKit/usb/USBSpec.h>
#include <IOKit/IOTypes.h>

typedef UInt16 USBDeviceAddress;
typedef UInt16 USBStatus;

#pragma pack(push, 1)

typedef struct {
    UInt8  bLength;
    UInt8  bDescriptorType;
} IOUSBDescriptorHeader;

typedef struct {
    UInt8  bLength;
    UInt8  bDescriptorType;
    UInt16 bcdUSB;
    UInt8  bDeviceClass;
    UInt8  bDeviceSubClass;
    UInt8  bDeviceProtocol;
    UInt8  bMaxPacketSize0;
    UInt16 idVendor;
    UInt16 idProduct;
    UInt16 bcdDevice;
    UInt8  iManufacturer;
    UInt8  iProduct;
    UInt8  iSerialNumber;
    UInt8  bNumConfigurations;
} IOUSBDeviceDescriptor;

typedef struct {
    UInt8  bLength;
    UInt8  bDescriptorType;
    UInt16 wTotalLength;
    UInt8  bNumInterfaces;
    UInt8  bConfigurationValue;
    UInt8  iConfiguration;
    UInt8  bmAttributes;
    UInt8  MaxPower;
} IOUSBConfigurationDescriptor;

typedef struct {
    UInt8  bLength;
    UInt8  bDescriptorType;
    UInt8  bInterfaceNumber;
    UInt8  bAlternateSetting;
    UInt8  bNumEndpoints;
    UInt8  bInterfaceClass;
    UInt8  bInterfaceSubClass;
    UInt8  bInterfaceProtocol;
    UInt8  iInterface;
} IOUSBInterfaceDescriptor;

typedef struct {
    UInt8  bLength;
    UInt8  bDescriptorType;
    UInt8  bEndpointAddress;
    UInt8  bmAttributes;
    UInt16 wMaxPacketSize;
    UInt8  bInterval;
} __attribute__((packed)) IOUSBEndpointDescriptor;

typedef struct {
    UInt8  bLength;
    UInt8  bDescriptorType;
    UInt16 bString[1];
} IOUSBStringDescriptor;

/* Endpoint companion descriptor, referenced by IOUSBInterface.h's
 * CalculateFullMaxPacketSize signature - full SuperSpeed fields not needed
 * for anything we implement. */
typedef struct {
    UInt8  bLength;
    UInt8  bDescriptorType;
    UInt8  bMaxBurst;
    UInt8  bmAttributes;
    UInt16 wBytesPerInterval;
} IOUSBSuperSpeedEndpointCompanionDescriptor;

typedef struct {
    UInt8  bmRequestType;
    UInt8  bRequest;
    UInt16 wValue;
    UInt16 wIndex;
    UInt16 wLength;
    void  *pData;
    UInt32 wLenDone;
} IOUSBDevRequest;

typedef struct {
    UInt8  bmRequestType;
    UInt8  bRequest;
    UInt16 wValue;
    UInt16 wIndex;
    UInt16 wLength;
    class IOMemoryDescriptor *pData;
    UInt32 wLenDone;
} IOUSBDevRequestDesc;

typedef void (*IOUSBCompletionAction)(void *target, void *parameter, IOReturn status, UInt32 bufferSizeRemaining);

typedef struct IOUSBCompletion {
    void *target;
    IOUSBCompletionAction action;
    void *parameter;
} IOUSBCompletion;

/* Isoc/low-latency-isoc/timestamped completions: referenced in IOUSBPipe.h/
 * IOUSBControllerListElement.h/IOUSBControllerV2.h signatures we don't
 * actually call (no isoc support in any UIM we have) - defined only so
 * those headers parse. */
typedef void (*IOUSBCompletionActionWithTimeStamp)(void *target, void *parameter, IOReturn status,
                                                     UInt32 bufferSizeRemaining, UInt64 timeStamp);
typedef struct IOUSBCompletionWithTimeStamp {
    void *target;
    IOUSBCompletionActionWithTimeStamp action;
    void *parameter;
} IOUSBCompletionWithTimeStamp;

typedef struct {
    UInt16   frStatus;
    UInt16   frReqCount;
    UInt16   frActCount;
} IOUSBIsocFrame;

typedef struct {
    UInt64   frTimeStamp;
    UInt16   frStatus;
    UInt16   frReqCount;
    UInt16   frActCount;
} IOUSBLowLatencyIsocFrame;

typedef IOUSBCompletion IOUSBIsocCompletion;

typedef void (*IOUSBLowLatencyIsocCompletionAction)(void *target, void *parameter,
                                                     IOReturn status, IOUSBLowLatencyIsocFrame *pFrames);
typedef struct IOUSBLowLatencyIsocCompletion {
    void *target;
    IOUSBLowLatencyIsocCompletionAction action;
    void *parameter;
} IOUSBLowLatencyIsocCompletion;

/* IOUSBSyncIsoCompletion: same address-sentinel idea as IOUSBSyncCompletion,
 * for the plain-isoc completion path (matches IOUSBCompletionAction's
 * signature - the low-latency call sites in IOUSBPipe.cpp explicitly
 * C-style-cast this same function to IOUSBLowLatencyIsocCompletionAction,
 * which is fine since it's never actually invoked by any UIM we have; no
 * isoc support anywhere in this tree). Defined (empty) in
 * IOUSBController.cpp. */
void IOUSBSyncIsoCompletion(void *target, void *parameter, IOReturn status,
                             UInt32 bufferSizeRemaining);

typedef struct {
    UInt16 bInterfaceClass;
    UInt16 bInterfaceSubClass;
    UInt16 bInterfaceProtocol;
    UInt16 bAlternateSetting;
} IOUSBFindInterfaceRequest;

typedef struct {
    UInt8  type;
    UInt8  direction;
    UInt16 maxPacketSize;
    UInt8  interval;
} IOUSBFindEndpointRequest;

#pragma pack(pop)

enum {
    kIOUSBFindInterfaceDontCare = 0xFFFF
};

#ifndef kIOUSBInterfaceNotFound
#define kIOUSBInterfaceNotFound iokit_usb_err(0x4e)
#endif
#ifndef iokit_usb_err
#define iokit_usb_err(x) (0xe0004000 | (x))
#endif

enum {
    kUSBMaxPipes = 32
};

/* Referenced by IOUSBInterface::GetEndpointPropertiesV3 (never called by
 * anything we implement - no isoc/SuperSpeed-companion support - defined
 * only so the header parses). */
#pragma pack(push, 1)
typedef struct {
    UInt8  bVersion;
    UInt8  bAlternateSetting;
    UInt8  bEndpointNumber;
    UInt8  bDirection;
    UInt8  bTransferType;
    UInt8  bSyncType;
    UInt8  bUsageType;
    UInt16 wMaxPacketSize;
    UInt8  bInterval;
    UInt8  bMaxBurst;
    UInt8  bMaxStreams;
    UInt8  bMult;
    UInt16 wBytesPerInterval;
} IOUSBEndpointProperties;
#pragma pack(pop)

/* EncodeRequest: packs a standard USB control-request signature (request,
 * direction, type, recipient) the way USBHub.h's kXxxFeature-style
 * constants expect, matching the encoding implied by
 * kIOUSBDeviceRequestDirectionPhase/TypePhase/RecipientMask naming - real
 * value doesn't matter for anything we implement (we build bmRequestType
 * directly, byte-at-a-time, everywhere we issue a real control transfer),
 * just needs to produce distinct compile-time constants. */
#define EncodeRequest(request, direction, type, recipient) \
    (((request) << 16) | ((direction) << 8) | ((type) << 4) | (recipient))

/* Standard bmRequestType byte: bit7 direction, bits6:5 type, bits4:0 recipient. */
#define USBmakebmRequestType(direction, type, recipient) \
    ((UInt8)(((direction) << 7) | ((type) << 5) | (recipient)))

/* Device speed (bDeviceProtocol-independent "logical speed" used throughout
 * IOUSBFamily/IOUSBCompositeDriver) - values match the real USB spec
 * encoding used everywhere else in this tree (RavynXHCIPort's
 * XHCI_PORTSC_SPEED, etc): 1=low/full-is-implicit-elsewhere, 2=high, 3=super. */
enum {
    kUSBDeviceSpeedLow  = 0,
    kUSBDeviceSpeedFull = 1,
    kUSBDeviceSpeedHigh = 2,
    kUSBDeviceSpeedSuper = 3
};

/* Endpoint descriptor field masks/shifts IOUSBPipe.cpp needs for wMaxPacketSize
 * high-bit multiplier decoding (HS isoc mult, FS/HS/SS all share this MPS mask). */
enum {
    kUSBPipeIDMask                              = 0x0F,
    kUSB_EPDesc_wMaxPacketSize_MPS_Mask         = 0x07FF,
    kUSB_EPDesc_wMaxPacketSize_MPS_Shift        = 0,
    kUSB_HSFSEPDesc_wMaxPacketSize_Mult_Mask    = 0x1800,
    kUSB_HSFSEPDesc_wMaxPacketSize_Mult_Shift   = 11,
    kUSB_EPDesc_bmAttributes_UsageType_Mask     = kUSBEndpointbmAttributesUsageTypeMask,
    kUSB_EPDesc_bmAttributes_UsageType_Shift    = kUSBEndpointbmAttributesUsageTypeShift,
    kUSB_EPDesc_bmAttributes_SyncType_Mask      = kUSBEndpointbmAttributesSynchronizationTypeMask,
    kUSB_EPDesc_bmAttributes_SyncType_Shift     = kUSBEndpointbmAttributesSynchronizationTypeShift,
    kUSBAllStreams                              = 0xFFFFFFFF
};

#define kUSBDevicePropertyLocationID "locationID"
#define kUSBOutOfSpecMPSOK           "kUSBOutOfSpecMPSOK"
#define kUSBSuspendPort              "kUSBSuspendPort"
#define kUSBPreferredConfiguration   "kUSBPreferredConfiguration"
#define kUSBExpressCardCantWake      "kUSBExpressCardCantWake"

#define kIOUSBClearPipeStallNotRecursive ((UInt32)1)

/* Notification/message constants (kIOUSBMsg* families) - purely informational
 * message-type tags passed through IOService::messageClients/message(), no
 * fixed values required by anything we implement other than distinctness. */
#define iokit_usb_msg(x) ((UInt32)(0xe0004000 | 0x100 | (x)))
enum {
    kIOUSBMessagePortHasBeenReset            = iokit_usb_msg(0x01),
    kIOUSBMessageCompositeDriverReconfigured = iokit_usb_msg(0x03),
    kIOUSBMessageExpressCardCantWake         = iokit_usb_msg(0x04),
    kIOUSBMessageHubPortClearTT              = iokit_usb_msg(0x05),
};

#define kIOUSBConfigNotFound          iokit_usb_err(0x4d)
#define kIOUSBNotEnoughPowerErr       iokit_usb_err(0x4c)
#define kIOUSBPipeStalled             iokit_usb_err(0x4f)
#define kIOUSBTransactionTimeout      iokit_usb_err(0x21)
#define kUSBNotEnoughPowerNotificationType 1

enum {
    kUSBDefaultControlNoDataTimeoutMS   = 5000,
    kUSBDefaultControlCompletionTimeoutMS = 5000,
};

/* IOUSBSyncCompletion: IOUSBPipe.cpp's synchronous Read/Write/control-request
 * wrappers take &IOUSBSyncCompletion's own ADDRESS as a completion.action
 * sentinel (the real UIM below is expected to recognize that address and
 * block/signal internally rather than call it as a real async callback).
 * None of our UIMs (RavynXHCIUSBBus) implement real async transfers, so we
 * never dereference/call it - it just needs to exist as a distinct address.
 * Defined (empty body) in IOUSBController.cpp. */
void IOUSBSyncCompletion(void *target, void *parameter, IOReturn status, UInt32 bufferSizeRemaining);

#endif /* !_IOKIT_USB_USB_H */
