/*
 * Copyright © 1998-2014 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _USB_H
#define _USB_H

#include <IOKit/IOTypes.h>
#include <libkern/OSByteOrder.h>
#include <IOKit/usb/USBSpec.h>
#include <IOKit/usb/IOUSBHostFamilyDefinitions.h>
#include <IOKit/usb/AppleUSBDefinitions.h>


#if !__OPEN_SOURCE__ && TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
//these headers were previously exported for iOS clients
#include <IOKit/usb/IOUSBHostFamily.h>
#include <IOKit/usb/StandardUSB.h>

// These are for Apple internal use only
#define kIOUSBMessageUpdateIdlePolicy    kUSBHostMessageUpdateIdlePolicy
#define kIOUSBMessageRemoteWake          kUSBHostMessageRemoteWake
#endif

#if KERNEL
#include <IOKit/IOMemoryDescriptor.h>
#endif /* KERNEL */


#ifdef __cplusplus
extern "C" {
#endif

/*!
   @header		USB.h
   @abstract	Public Interfaces to the USB implementation in Mac OS X.
   @discussion	This header file contains definitions and structures that are used in the different USB API's in Mac OS X, both in the kernel and in the user space.
 */

/*!
   @defineblock	Endian conversion definitions
   @discussion The USB API's use a convention of specifying parameters in the host order.  The USB spec specifies that multi-byte items should be
   formatted in little endian order.  The following macros allow one to translate multi-byte values from Host order to USB order and vice versa.  There are separate macros for
   in-kernel use and for user space use.
 */
#define USBToHostWord USBToHost16
#define HostToUSBWord HostToUSB16
#define USBToHostLong USBToHost32
#define HostToUSBLong HostToUSB32
/*! @/defineblock */

#ifndef USBBitRange
#define USBBitRange(start, end) IOUSBBitRange(start, end)
#endif

#ifndef USBBitRangePhase
#define USBBitRangePhase(start, end) IOUSBBitRangePhase(start, end)
#endif

/*!
   @enum Miscellaneous Constants
   @discussion
 */
enum
{
    kUSBDeviceIDShift         = 7,
    kUSBMaxDevices            = 128,
    kUSBMaxDevice             = kUSBMaxDevices - 1,
    kUSBDeviceIDMask          = 0x7f,
    kUSBTooManyDevicesAddress = 0xfffe,

    kUSBPipeIDMask = 0xf,
    kUSBMaxPipes   = 32,        // In and Out pipes can have same pipe number.

    kUSBInterfaceIDShift = 8,
    kUSBMaxInterfaces    = 1 << kUSBInterfaceIDShift,
    kUSBInterfaceIDMask  = kUSBMaxInterfaces - 1,

    kUSBEndPtShift = 7,
    kUSBDeviceMask = ((1 << kUSBEndPtShift) - 1),

    kUSBNoPipeIdx = -1,

#ifndef __OPEN_SOURCE__
    // In order to ameliorate the effects of PCI Pause on drivers that do not subscribe to the USB Notifications for it, we have decided to sleep threads that come into the UIM
    // while we are in PCI Pause.  We will wake those threads up after the root hub drivers are back ON.  However, we needed a way to detect that an I/O was coming from user space.
    // One way to solve that would be to power manage the user client and do the sleep/wake there, but that was a big undertaking.  Another way is for the user client to somehow
    // let the UIM know through some APIs.  We decided on a quick way by using bits that are part of the API to communicate that the request is coming from user space.  For device
    // requests we use the wLenDone field, a 32-bit quantity, but whose maximum value is only 16 bits.  Same goes for the streamID of bulk requests.  Those bits are set by the user
    // client and cleared by the controller after checking them.  If they are set and PCI Pause is active, we will sleep that request.
#endif
    kUSBUCRequestWithoutUSBNotificationMask = (1 << 30),
    kUSBEndpointTransferTypeUCMask          = (1 << 7),

    // Constants for streams
    kUSBStream0                = 0,
    kUSBMaxStream              = 65533,
    kUSBPRimeStream            = 0xfffe,
    kUSBNoStream               = 0xffff,
    kUSBAllStreams             = 0xffffffff,                                            // Obsolete, use kUSBStreamIDAllStreamsMask
    kUSBStreamIDMask           = 0xffff,
    kUSBStreamIDAllStreamsMask = (1 << 31)
};

/*!
   @enum bRequest Shifts and Masks
   @discussion These are used to create the macro to encode the bRequest field of a Device Request
 */
enum
{
    kUSBRqDirnShift = kIOUSBDeviceRequestDirectionPhase,
    kUSBRqDirnMask  = 1,

    kUSBRqTypeShift = kIOUSBDeviceRequestTypePhase,
    kUSBRqTypeMask  = 3,

    kUSBRqRecipientMask = kIOUSBDeviceRequestRecipientMask
};

/*!
   @defined USBmakebmRequestType
   @discussion Macro to encode the bmRequestType field of a Device Request.  It is used when constructing an IOUSBDevRequest.
 */
#define USBmakebmRequestType(direction, type, recipient) \
    (((direction & kUSBRqDirnMask) << kUSBRqDirnShift) | \
     ((type & kUSBRqTypeMask) << kUSBRqTypeShift) |      \
     (recipient & kUSBRqRecipientMask))

/*!
   @enum kUSBMaxIsocFrameReqCount
   @discussion Maximum size in bytes allowed for one Isochronous frame
 */
enum
{
    kUSBMaxFSIsocEndpointReqCount = 1023,       // max size (bytes) of any one Isoc frame for 1 FS endpoint
    kUSBMaxHSIsocEndpointReqCount = 3072,       // max size (bytes) of any one Isoc frame for 1 HS endpoint
    kUSBMaxHSIsocFrameCount       = 7168                // max size (bytes) of all Isoc transfers in a HS frame
};

/*!
   @defined EncodeRequest
   @discussion Macro that encodes the bRequest and bRequestType fields of a IOUSBDevRequest into a single value.  It is useful when one needs
   to know what type of request the IOUSBDevRequest encodes and simplifies comparisons.
 */
#define EncodeRequest(request, direction, type, recipient) \
    (((UInt16)request << 8) +                              \
     ((UInt16)recipient +                                  \
      ((UInt16)type << kUSBRqTypeShift) +                  \
      ((UInt16)direction << kUSBRqDirnShift)))


/*!
   @enum Standard Device Requests
   @discussion Encoding of the standard device requests.
   <tt>
   <pre><b>
   bmRequestType bRequest          wValue        wIndex     wLength Data</b>
   00000000B     CLEAR_FEATURE     Feature       Zero       Zero    None (device)
   00000001B                       Feature       Interface  Zero    None (Interface)
   00000010B                       Feature       Endpoint   Zero    None (Endpoint)

   10000000B     GET_CONFIGURATION Zero          Zero       One     Configuration
   10000000B     GET_DESCRIPTOR    Type          LangID     Length  Descriptor
   10000001B     GET_INTERFACE     Zero          Interface  One     Alternate

   10000000B     GET_STATUS        Zero          Zero       Two     status (device)
   10000001B                       Zero          Interface  Two     status (Interface)
   10000010B                       Zero          Endpoint   Two     status (Endpoint)

   00000000B     SET_ADDRESS       Address       Zero       Zero    None
   00000000B     SET_CONFIGURATION Configuration Zero       Zero    None
   00000000B     SET_DESCRIPTOR    Type          LangID     Length  Descriptor

   00000000B     SET_FEATURE       Feature       Zero       Zero    None (device)
   00000001B                       Feature       Interface  Zero    None (Interface)
   00000010B                       Feature       Endpoint   Zero    None (Endpoint)

   00000001B     SET_INTERFACE     Alternate     Interface  Zero    None
   10000010B     SYNCH_FRAME       Zero          Endpoint   Two     Frame Number
   </pre>
   </tt>
 */
enum
{
    kClearDeviceFeature    = EncodeRequest(kUSBRqClearFeature,  kUSBOut, kUSBStandard, kUSBDevice),
    kClearInterfaceFeature = EncodeRequest(kUSBRqClearFeature,  kUSBOut, kUSBStandard, kUSBInterface),
    kClearEndpointFeature  = EncodeRequest(kUSBRqClearFeature,  kUSBOut, kUSBStandard, kUSBEndpoint),
    kGetConfiguration      = EncodeRequest(kUSBRqGetConfig,     kUSBIn,  kUSBStandard, kUSBDevice),
    kGetDescriptor         = EncodeRequest(kUSBRqGetDescriptor, kUSBIn,  kUSBStandard, kUSBDevice),
    kGetInterface          = EncodeRequest(kUSBRqGetInterface,  kUSBIn,  kUSBStandard, kUSBInterface),
    kGetDeviceStatus       = EncodeRequest(kUSBRqGetStatus,     kUSBIn,  kUSBStandard, kUSBDevice),
    kGetInterfaceStatus    = EncodeRequest(kUSBRqGetStatus,     kUSBIn,  kUSBStandard, kUSBInterface),
    kGetEndpointStatus     = EncodeRequest(kUSBRqGetStatus,     kUSBIn,  kUSBStandard, kUSBEndpoint),
    kSetAddress            = EncodeRequest(kUSBRqSetAddress,    kUSBOut, kUSBStandard, kUSBDevice),
    kSetConfiguration      = EncodeRequest(kUSBRqSetConfig,     kUSBOut, kUSBStandard, kUSBDevice),
    kSetDescriptor         = EncodeRequest(kUSBRqSetDescriptor, kUSBOut, kUSBStandard, kUSBDevice),
    kSetDeviceFeature      = EncodeRequest(kUSBRqSetFeature,    kUSBOut, kUSBStandard, kUSBDevice),
    kSetInterfaceFeature   = EncodeRequest(kUSBRqSetFeature,    kUSBOut, kUSBStandard, kUSBInterface),
    kSetEndpointFeature    = EncodeRequest(kUSBRqSetFeature,    kUSBOut, kUSBStandard, kUSBEndpoint),
    kSetInterface          = EncodeRequest(kUSBRqSetInterface,  kUSBOut, kUSBStandard, kUSBInterface),
    kSyncFrame             = EncodeRequest(kUSBRqSyncFrame,     kUSBIn,  kUSBStandard, kUSBEndpoint),
};

/*!
   @defined kCallInterfaceOpenWithGate
   @discussion If the USB Device has this property, drivers for any of its interfaces will have their handleOpen method called while holding the workloop gate.
 */
#define kCallInterfaceOpenWithGate      "kCallInterfaceOpenWithGate"

// TYPES

typedef UInt16 USBDeviceAddress;

typedef uint32_t USBPhysicalAddress32;

/*!
    @typedef IOUSBIsocFrame
    @discussion Structure used to encode information about each isoc frame.
    @param frStatus Returns status associated with the frame.
    @param frReqCount Input specifiying how many bytes to read or write.
    @param frActCount Actual # of bytes transferred.
 */
typedef struct IOUSBIsocFrame
{
    IOReturn frStatus;
    UInt16   frReqCount;
    UInt16   frActCount;
} IOUSBIsocFrame;


/*!
    @typedef IOUSBLowLatencyIsocFrame
    @discussion    Structure used to encode information about each isoc frame that is processed
    at hardware interrupt time (low latency).
    @param frStatus Returns status associated with the frame.
    @param frReqCount Input specifiying how many bytes to read or write.
    @param frActCount Actual # of bytes transferred.
    @param frTimeStamp Time stamp that indicates time when frame was procesed.
 */
struct IOUSBLowLatencyIsocFrame
{
    IOReturn     frStatus;
    UInt16       frReqCount;
    UInt16       frActCount;
    AbsoluteTime frTimeStamp;
};

typedef struct IOUSBLowLatencyIsocFrame IOUSBLowLatencyIsocFrame;

/*!
   @typedef IOUSBCompletionAction
   @discussion Function called when USB I/O completes.
   @param target The target specified in the IOUSBCompletion struct.
   @param parameter The parameter specified in the IOUSBCompletion struct.
   @param status Completion status.
   @param bufferSizeRemaining Bytes left to be transferred.
 */
typedef void (* IOUSBCompletionAction)(void*    target,
                                       void*    parameter,
                                       IOReturn status,
                                       UInt32   bufferSizeRemaining);

/*!
   @typedef IOUSBCompletionActionWithTimeStamp
   @discussion Function called when USB I/O completes.
   @param target The target specified in the IOUSBCompletion struct.
   @param parameter The parameter specified in the IOUSBCompletion struct.
   @param status Completion status.
   @param bufferSizeRemaining Bytes left to be transferred.
   @param timeStamp Time at which the transaction was processed.
 */
typedef void (* IOUSBCompletionActionWithTimeStamp)(void*        target,
                                                    void*        parameter,
                                                    IOReturn     status,
                                                    UInt32       bufferSizeRemaining,
                                                    AbsoluteTime timeStamp);

/*!
    @typedef IOUSBIsocCompletionAction
    @discussion Function called when Isochronous USB I/O completes.
    @param target The target specified in the IOUSBIsocCompletionn struct.
    @param parameter The parameter specified in the IOUSBIsocCompletion struct.
    @param status Completion status.
    @param pFrames Pointer to the frame list containing the status for each frame transferred.
 */
typedef void (* IOUSBIsocCompletionAction)(void*           target,
                                           void*           parameter,
                                           IOReturn        status,
                                           IOUSBIsocFrame* pFrames);

/*!
    @typedef IOUSBLowLatencyIsocCompletionAction
    @discussion Function called when Low Latency Isochronous USB I/O completes.
    @param target The target specified in the IOUSBLowLatencyIsocCompletion struct.
    @param parameter The parameter specified in the IOUSBLowLatencyIsocCompletion struct.
    @param status Completion status.
    @param pFrames Pointer to the low latency frame list containing the status for each frame transferred.
 */
typedef void (* IOUSBLowLatencyIsocCompletionAction)(void*                     target,
                                                     void*                     parameter,
                                                     IOReturn                  status,
                                                     IOUSBLowLatencyIsocFrame* pFrames);

/*!
   @typedef IOUSBCompletion
   @discussion Struct specifying action to perform when a USB I/O completes.
   @param target The target to pass to the action function.
   @param action The function to call.
   @param parameter The parameter to pass to the action function.
 */
typedef struct IOUSBCompletion
{
    void*                 target;
    IOUSBCompletionAction action;
    void*                 parameter;
} IOUSBCompletion;

/*!
   @typedef IOUSBCompletionWithTimeStamp
   @discussion Struct specifying action to perform when a USB I/O completes.
   @param target The target to pass to the action function.
   @param action The function to call.
   @param parameter The parameter to pass to the action function.
 */
typedef struct IOUSBCompletionWithTimeStamp
{
    void*                              target;
    IOUSBCompletionActionWithTimeStamp action;
    void*                              parameter;
} IOUSBCompletionWithTimeStamp;

/*!
    @typedef IOUSBIsocCompletion
    @discussion Struct specifying action to perform when an Isochronous USB I/O completes.
    @param target The target to pass to the action function.
    @param action The function to call.
    @param parameter The parameter to pass to the action function.
 */
typedef struct IOUSBIsocCompletion
{
    void*                     target;
    IOUSBIsocCompletionAction action;
    void*                     parameter;
} IOUSBIsocCompletion;

/*!
    @typedef IOUSBLowLatencyIsocCompletion
    @discussion Struct specifying action to perform when an Low Latency Isochronous USB I/O completes.
    @param target The target to pass to the action function.
    @param action The function to call.
    @param parameter The parameter to pass to the action function.
 */
typedef struct IOUSBLowLatencyIsocCompletion
{
    void*                               target;
    IOUSBLowLatencyIsocCompletionAction action;
    void*                               parameter;
} IOUSBLowLatencyIsocCompletion;

/*!
   @defineblock IOUSBFamily error codes
   @discussion  Errors specific to the IOUSBFamily.  Note that the iokit_usb_err(x) translates to 0xe0004xxx, where xxx is the value in parenthesis as a hex number.
 */

#define iokit_usb_err(return )                               (IOReturn)(sys_iokit | sub_iokit_usb | return )
#define kIOUSBUnknownPipeErr                                iokit_usb_err(0x61)     // 0xe0004061  Pipe ref not recognized
#define kIOUSBTooManyPipesErr                               iokit_usb_err(0x60)     // 0xe0004060  Too many pipes
#define kIOUSBNoAsyncPortErr                                iokit_usb_err(0x5f)     // 0xe000405f  no async port
#define kIOUSBNotEnoughPipesErr                             iokit_usb_err(0x5e)     // 0xe000405e  not enough pipes in interface
#define kIOUSBNotEnoughPowerErr                             iokit_usb_err(0x5d)     // 0xe000405d  not enough power for selected configuration
#define kIOUSBEndpointNotFound                              iokit_usb_err(0x57)     // 0xe0004057  Endpoint Not found
#define kIOUSBConfigNotFound                                iokit_usb_err(0x56)     // 0xe0004056  Configuration Not found
#define kIOUSBPortWasSuspended                              iokit_usb_err(0x52)     // 0xe0004052  The transaction was returned because the port was suspended
#define kIOUSBPipeStalled                                   iokit_usb_err(0x4f)     // 0xe000404f  Pipe has stalled, error needs to be cleared
#define kIOUSBInterfaceNotFound                             iokit_usb_err(0x4e)     // 0xe000404e  Interface ref not recognized
#define kIOUSBLowLatencyBufferNotPreviouslyAllocated        iokit_usb_err(0x4d)     // 0xe000404d  Attempted to use user land low latency isoc calls w/out calling PrepareBuffer (on the data buffer) first
#define kIOUSBLowLatencyFrameListNotPreviouslyAllocated     iokit_usb_err(0x4c)     // 0xe000404c  Attempted to use user land low latency isoc calls w/out calling PrepareBuffer (on the frame list) first
#define kIOUSBHighSpeedSplitError                           iokit_usb_err(0x4b)     // 0xe000404b  Error to hub on high speed bus trying to do split transaction
#define kIOUSBSyncRequestOnWLThread                         iokit_usb_err(0x4a)     // 0xe000404a  A synchronous USB request was made on the workloop thread (from a callback?).  Only async requests are permitted in that case
#define kIOUSBDeviceNotHighSpeed                            iokit_usb_err(0x49)     // 0xe0004049  Name is deprecated, see below
#define kIOUSBDeviceTransferredToCompanion                  iokit_usb_err(0x49)     // 0xe0004049  The device has been tranferred to another controller for enumeration
#define kIOUSBClearPipeStallNotRecursive                    iokit_usb_err(0x48)     // 0xe0004048  IOUSBPipe::ClearPipeStall should not be called recursively
#define kIOUSBDevicePortWasNotSuspended                     iokit_usb_err(0x47)     // 0xe0004047  Port was not suspended
#define kIOUSBEndpointCountExceeded                         iokit_usb_err(0x46)     // 0xe0004046  The endpoint was not created because the controller cannot support more endpoints
#define kIOUSBDeviceCountExceeded                           iokit_usb_err(0x45)     // 0xe0004045  The device cannot be enumerated because the controller cannot support more devices
#define kIOUSBStreamsNotSupported                           iokit_usb_err(0x44)     // 0xe0004044  The request cannot be completed because the XHCI controller does not support streams
#define kIOUSBInvalidSSEndpoint                             iokit_usb_err(0x43)     // 0xe0004043  An endpoint found in a SuperSpeed device is invalid (usually because there is no Endpoint Companion Descriptor)
#define kIOUSBTooManyTransactionsPending                    iokit_usb_err(0x42)     // 0xe0004042  The transaction cannot be submitted because it would exceed the allowed number of pending transactions

#if !__OPEN_SOURCE__ && TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
#define kIOUSBTransactionReturned                           kIOReturnAborted
#define kIOUSBTransactionTimeout                            kIOReturnTimeout
#else
#define kIOUSBTransactionReturned                           iokit_usb_err(0x50)     // 0xe0004050  The transaction has been returned to the caller
#define kIOUSBTransactionTimeout                            iokit_usb_err(0x51)     // 0xe0004051  Transaction timed out
#endif

/*!
   @definedblock IOUSBFamily hardware error codes
   @discussion These errors are returned by the OHCI controller.  The # in parenthesis (xx) corresponds to the OHCI Completion Code.
   For the following Completion codes, we return a generic IOKit error instead of a USB specific error.
   <tt>
   <pre>
   Completion Code         Error Returned              Description
   9                       kIOReturnUnderrun           (Data Underrun) EP returned less data than max packet size
   8                       kIOReturnOverrun            (Data Overrun) Packet too large or more data than buffer
   5                       kIOReturnNotResponding      Device Not responding
   4                       kIOUSBPipeStalled           Endpoint returned a STALL PID
   </pre>
   </tt>
 */
#define kIOUSBLinkErr           iokit_usb_err(0x10)             // 0xe0004010
#define kIOUSBNotSent2Err       iokit_usb_err(0x0f)             // 0xe000400f Transaction not sent
#define kIOUSBNotSent1Err       iokit_usb_err(0x0e)             // 0xe000400e Transaction not sent
#define kIOUSBBufferUnderrunErr iokit_usb_err(0x0d)             // 0xe000400d Buffer Underrun (Host hardware failure on data out, PCI busy?)
#define kIOUSBBufferOverrunErr  iokit_usb_err(0x0c)             // 0xe000400c Buffer Overrun (Host hardware failure on data out, PCI busy?)
#define kIOUSBReserved2Err      iokit_usb_err(0x0b)             // 0xe000400b Reserved
#define kIOUSBReserved1Err      iokit_usb_err(0x0a)             // 0xe000400a Reserved
#define kIOUSBWrongPIDErr       iokit_usb_err(0x07)             // 0xe0004007 Pipe stall, Bad or wrong PID
#define kIOUSBPIDCheckErr       iokit_usb_err(0x06)             // 0xe0004006 Pipe stall, PID CRC error
#define kIOUSBDataToggleErr     iokit_usb_err(0x03)             // 0xe0004003 Pipe stall, Bad data toggle
#define kIOUSBBitstufErr        iokit_usb_err(0x02)             // 0xe0004002 Pipe stall, bitstuffing
#define kIOUSBCRCErr            iokit_usb_err(0x01)             // 0xe0004001 Pipe stall, bad CRC
/*! @/definedblock */


/*!
   @defineblock IOUSBFamily message codes
   @discussion  Messages specific to the IOUSBFamily.  Note that the iokit_usb_msg(x) translates to 0xe0004xxx, where xxx is the value in parenthesis as a hex number.
 */
#define iokit_usb_msg(message)                          (UInt32)(sys_iokit | sub_iokit_usb | message)
#define kIOUSBMessageHubResetPort                       iokit_usb_msg(0x01)     // 0xe0004001  Message sent to a hub to reset a particular port
#define kIOUSBMessageHubSuspendPort                     iokit_usb_msg(0x02)     // 0xe0004002  Message sent to a hub to suspend a particular port
#define kIOUSBMessageHubResumePort                      iokit_usb_msg(0x03)     // 0xe0004003  Message sent to a hub to resume a particular port
#define kIOUSBMessageHubIsDeviceConnected               iokit_usb_msg(0x04)     // 0xe0004004  Message sent to a hub to inquire whether a particular port has a device connected or not
#define kIOUSBMessageHubIsPortEnabled                   iokit_usb_msg(0x05)     // 0xe0004005  Message sent to a hub to inquire whether a particular port is enabled or not
#define kIOUSBMessageHubReEnumeratePort                 iokit_usb_msg(0x06)     // 0xe0004006  Message sent to a hub to reenumerate the device attached to a particular port
#define kIOUSBMessagePortHasBeenReset                   iokit_usb_msg(0x0a)     // 0xe000400a  Message sent to a device indicating that the port it is attached to has been reset
#define kIOUSBMessagePortHasBeenResumed                 iokit_usb_msg(0x0b)     // 0xe000400b  Message sent to a device indicating that the port it is attached to has been resumed
#define kIOUSBMessageHubPortClearTT                     iokit_usb_msg(0x0c)     // 0xe000400c  Message sent to a hub to clear the transaction translator
#define kIOUSBMessagePortHasBeenSuspended               iokit_usb_msg(0x0d)     // 0xe000400d  Message sent to a device indicating that the port it is attached to has been suspended
#define kIOUSBMessageFromThirdParty                     iokit_usb_msg(0x0e)     // 0xe000400e  Message sent from a third party.  Uses IOUSBThirdPartyParam to encode the sender's ID
#define kIOUSBMessagePortWasNotSuspended                iokit_usb_msg(0x0f)     // 0xe000400f  Message indicating that the hub driver received a resume request for a port that was not suspended
#define kIOUSBMessageExpressCardCantWake                iokit_usb_msg(0x10)     // 0xe0004010  Message from a driver to a bus that an express card will disconnect on sleep and thus shouldn't wake
#define kIOUSBMessageCompositeDriverReconfigured        iokit_usb_msg(0x11)     // 0xe0004011  Message from the composite driver indicating that it has finished re-configuring the device after a reset
#define kIOUSBMessageHubSetPortRecoveryTime             iokit_usb_msg(0x12)     // 0xe0004012  Message sent to a hub to set the # of ms required when resuming a particular port
#define kIOUSBMessageOvercurrentCondition               iokit_usb_msg(0x13)     // 0xe0004013  Message sent to the clients of the device's hub parent, when a device causes an overcurrent condition.  The message argument contains the locationID of the device
#define kIOUSBMessageNotEnoughPower                     iokit_usb_msg(0x14)     // 0xe0004014  Message sent to the clients of the device's hub parent, when a device causes an low power notice to be displayed.  The message argument contains the locationID of the device
#define kIOUSBMessageController                         iokit_usb_msg(0x15)     // 0xe0004015  Generic message sent from controller user client to controllers
#define kIOUSBMessageRootHubWakeEvent                   iokit_usb_msg(0x16)     // 0xe0004016  Message from the HC Wakeup code indicating that a Root Hub port has a wake event
#define kIOUSBMessageReleaseExtraCurrent                iokit_usb_msg(0x17)     // 0xe0004017  Message to ask any clients using extra current to release it if possible
#define kIOUSBMessageReallocateExtraCurrent             iokit_usb_msg(0x18)     // 0xe0004018  Message to ask any clients using extra current to attempt to allocate it some more
#define kIOUSBMessageEndpointCountExceeded              iokit_usb_msg(0x19)     // 0xe0004019  Message sent to a device when endpoints cannot be created because the USB controller ran out of resources
#define kIOUSBMessageDeviceCountExceeded                iokit_usb_msg(0x1a)     // 0xe000401a  Message sent by a hub when a device cannot be enumerated because the USB controller ran out of resources
#define kIOUSBMessageHubPortDeviceDisconnected          iokit_usb_msg(0x1b)     // 0xe000401b  Message sent by a built-in hub when a device was disconnected
#define kIOUSBMessageUnsupportedConfiguration           iokit_usb_msg(0x1c)     // 0xe000401c  Message sent to the clients of the device when a device is not supported in the current configuration.  The message argument contains the locationID of the device
#define kIOUSBMessageHubCountExceeded                   iokit_usb_msg(0x1d)     // 0xe000401d  Message sent when a 6th hub was plugged in and was not enumerated, as the USB spec only support 5 hubs in a chain
#define kIOUSBMessageTDMLowBattery                      iokit_usb_msg(0x1e)     // 0xe000401e  Message sent when when an attached TDM system battery is running low.
#define kIOUSBMessageLegacySuspendDevice                iokit_usb_msg(0x1f)     // 0xe000401f  Message sent to legacy interfaces when SuspedDevice() is called .
#define kIOUSBMessageLegacyResetDevice                  iokit_usb_msg(0x20)     // 0xe0004020  Message sent to legacy interfaces when ResetDevice() is called .
#define kIOUSBMessageLegacyReEnumerateDevice            iokit_usb_msg(0x21)     // 0xe0004021  Message sent to legacy interfaces when ReEnumerateDevice() is called .

#define kIOUSBMessageRenegotiateCurrent                 kUSBHostMessageRenegotiateCurrent

#if !__OPEN_SOURCE__ && TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
#define kIOUSBMessageConfigurationSet               kUSBHostMessageConfigurationSet
#else
#define kIOUSBMessageConfigurationSet               iokit_usb_msg(0x22)     // 0xe0004022  Message sent upon a SetConfiguration call
#endif

// Obsolete
//
struct IOUSBMouseData
{
    UInt16 buttons;
    SInt16 XDelta;
    SInt16 YDelta;
};

typedef struct IOUSBMouseData IOUSBMouseData;
typedef IOUSBMouseData*       IOUSBMouseDataPtr;

// Obsolete
//
struct IOUSBKeyboardData
{
    UInt16 keycount;
    UInt16 usbkeycode[32];
};

typedef struct IOUSBKeyboardData IOUSBKeyboardData;
typedef IOUSBKeyboardData*       IOUSBKeyboardDataPtr;

// Obsolete
//
union IOUSBHIDData
{
    IOUSBKeyboardData kbd;
    IOUSBMouseData    mouse;
};

typedef union IOUSBHIDData IOUSBHIDData;
typedef IOUSBHIDData*      IOUSBHIDDataPtr;


typedef IOUSBDeviceDescriptor*                                     IOUSBDeviceDescriptorPtr;
typedef IOUSBDescriptorHeader*                                     IOUSBDescriptorHeaderPtr;
typedef IOUSBBOSDescriptor*                                        IOUSBBOSDescriptorPtr;
typedef IOUSBDeviceCapabilityDescriptorHeader*                     IOUSBDeviceCapabilityDescriptorHeaderPtr;
typedef IOUSBDeviceCapabilityUSB2Extension*                        IOUSBDeviceCapabilityUSB2ExtensionPtr;
typedef IOUSBDeviceCapabilitySuperSpeedUSB*                        IOUSBDeviceCapabilitySuperSpeedUSBPtr;
typedef IOUSBDeviceCapabilitySuperSpeedPlusUSB*                    IOUSBDeviceCapabilitySuperSpeedPlusUSBPtr;
typedef IOUSBDeviceCapabilityContainerID*                          IOUSBDeviceCapabilityContainerIDPtr;
typedef IOUSBPlatformCapabilityDescriptor*                         IOUSBPlatformCapabilityDescriptorPtr;
typedef IOUSBDeviceCapabilityBillboardAltConfig*                   IOUSBDeviceCapabilityBillboardAltConfigPtr;
typedef IOUSBDeviceCapabilityBillboard*                            IOUSBDeviceCapabilityBillboardPtr;
typedef IOUSBDeviceCapabilityBillboardAltMode*                     IOUSBDeviceCapabilityBillboardAltModePtr;
typedef IOUSBConfigurationDescriptor*                              IOUSBConfigurationDescriptorPtr;
typedef IOUSBConfigurationDescHeader*                              IOUSBConfigurationDescHeaderPtr;
typedef IOUSBInterfaceDescriptor*                                  IOUSBInterfaceDescriptorPtr;
typedef IOUSBEndpointDescriptor*                                   IOUSBEndpointDescriptorPtr;
typedef IOUSBStringDescriptor*                                     IOUSBStringDescriptorPtr;
typedef IOUSBSuperSpeedEndpointCompanionDescriptor*                IOUSBSuperSpeedEndpointCompanionDescriptorPtr;
typedef IOUSBSuperSpeedPlusIsochronousEndpointCompanionDescriptor* IOUSBSuperSpeedPlusIsochronousEndpointCompanionDescriptorPtr;
typedef UASPipeDescriptor*                                         UASPipeDescriptorPtr;
typedef IOUSBHIDDescriptor*                                        IOUSBHIDDescriptorPtr;
typedef IOUSBHIDReportDesc*                                        IOUSBHIDReportDescPtr;
typedef IOUSBDeviceQualifierDescriptor*                            IOUSBDeviceQualifierDescriptorPtr;
typedef IOUSBDFUDescriptor*                                        IOUSBDFUDescriptorPtr;
typedef IOUSBInterfaceAssociationDescriptor*                       IOUSBInterfaceAssociationDescriptorPtr;

enum
{
    kIOUSBDeviceCapabilityDescriptorType      = 16,
    kIOUSBDeviceCapabilityDescriptorLengthMin = 3,
};



enum
{
    kUSB_EPDesc_bmAttributes_TranType_Mask   = USBBitRange(0, 1),
    kUSB_EPDesc_bmAttributes_TranType_Shift  = USBBitRangePhase(0, 1),
    kUSB_EPDesc_bmAttributes_SyncType_Mask   = USBBitRange(2, 3),
    kUSB_EPDesc_bmAttributes_SyncType_Shift  = USBBitRangePhase(2, 3),
    kUSB_EPDesc_bmAttributes_UsageType_Mask  = USBBitRange(4, 5),
    kUSB_EPDesc_bmAttributes_UsageType_Shift = USBBitRangePhase(4, 5),

    kUSB_EPDesc_wMaxPacketSize_MPS_Mask  = USBBitRange(0, 10),
    kUSB_EPDesc_wMaxPacketSize_MPS_Shift = USBBitRangePhase(0, 10),
    kUSB_EPDesc_MaxMPS                   = 1024,                                           // this is the maximum no matter what

    kUSB_HSFSEPDesc_wMaxPacketSize_Mult_Mask  = USBBitRange(11, 12),
    kUSB_HSFSEPDesc_wMaxPacketSize_Mult_Shift = USBBitRangePhase(11, 12)
};

enum
{
    kUSB_SSCompDesc_Bulk_MaxStreams_Mask  = USBBitRange(0, 4),
    kUSB_SSCompDesc_Bulk_MaxStreams_Shift = USBBitRangePhase(0, 4),
    kUSB_SSCompDesc_Isoc_Mult_Mask        = USBBitRange(0, 1),
    kUSB_SSCompDesc_Isoc_Mult_Shift       = USBBitRangePhase(0, 1)
};


#ifndef __OPEN_SOURCE__
// Leave this here because it has existed for many years, but it never really belonged here in the first place
#endif

// these following 2 lines are deprecated and should not be used.
enum {addPacketShift = 11};  // Bits for additional packets in maxPacketField. (Table 9-13)

#define mungeMaxPacketSize(w) ((w > 1024) ? (((w >> (addPacketShift)) + 1) * (w & ((1 << addPacketShift) - 1))) : w)

/*!
    @typedef IOUSBEndpointProperties
    @discussion  Structure used with the IOUSBLib GetEndpointPropertiesV3 and GetPipePropertiesV3 API. Most of the fields are taken directly from corresponding Standard Endpoint Descriptor and SuperSpeed Endpoint Companion Descriptor. wBytesPerInterval will be synthesized for  High Speed High Bandwidth Isochronous endpoints.
    @field bVersion  Version of the structure.  Currently kUSBEndpointPropertiesVersion3.  Need to set this when using this structure
    @field bAlternateSetting Used as an input for GetEndpointPropertiesV3.  Used as an output for GetPipePropertiesV3
    @field bDirection Used as an input for GetEndpointPropertiesV3.  Used as an output for GetPipePropertiesV3. One of kUSBIn or kUSBOut.
    @field bEndpointNumber Used as an input for GetEndpointPropertiesV3.  Used as an output for GetPipePropertiesV3
    @field bTransferType  One of kUSBControl, kUSBBulk, kUSBIsoc, or kUSBInterrupt
    @field bUsageType  For interrupt and isoc endpoints, the usage type.  For Bulk endpoints of the UAS Mass Storage Protocol, the pipe ID.
    @field bSyncType	For isoc endpoints only
    @field bInterval	The bInterval field from the Standard Endpoint descriptor.
    @field wMaxPacketSize  The meaning of this value depends on whether this is called with GetPipePropertiesV3 or GetEndpointPropertiesV3. See the documentation of those calls for more info.
    @field bMaxBurst  For SuperSpeed endpoints, maximum number of packets the endpoint can send or receive as part of a burst
    @field bMaxStreams  For SuperSpeed bulk endpoints, maximum number of streams this endpoint supports.
    @field bMult  For SuperSpeed isoc endpoints, this is the mult value from the SuperSpeed Endpoint Companion Descriptor. For High Speed isoc and interrupt endpoints, this is bits 11 and 12 of the Standard Endpoint Descriptor, which represents a similar value.
    @field wBytesPerInterval  For SuperSpeed interrupt and isoc endpoints, this is the wBytesPerInterval from the SuperSpeed Endpoint Companion Descriptor. For High Speed High Bandwidth isoc endpoints, this will be equal to wMaxPacketSize * (bMult+1).
 */
struct IOUSBEndpointProperties
{
    UInt8  bVersion;
    UInt8  bAlternateSetting;
    UInt8  bDirection;
    UInt8  bEndpointNumber;
    UInt8  bTransferType;
    UInt8  bUsageType;
    UInt8  bSyncType;
    UInt8  bInterval;
    UInt16 wMaxPacketSize;
    UInt8  bMaxBurst;
    UInt8  bMaxStreams;
    UInt8  bMult;
    UInt16 wBytesPerInterval;
} __attribute__((packed));

typedef struct IOUSBEndpointProperties IOUSBEndpointProperties;
typedef IOUSBEndpointProperties*       IOUSBEndpointPropertiesPtr;

/*!
    @enum USBGetEndpointVersion
    @discussion         Version of the IOUSBEndpointProperties structure.
    @constant	kUSBEndpointPropertiesVersion3			Version that has support for USB3 SuperSpeed Endpoint Companion fields.
 */
enum
{
    kUSBEndpointPropertiesVersion3 = 0x03
};

/*!
    @typedef USBStatus
    @discussion Type used to get a DeviceStatus as a single quantity.
 */
typedef UInt16 USBStatus;
typedef USBStatus* USBStatusPtr;

// These constants are obsolete
//
enum
{
    kIOUSBAnyClass    = 0xFFFF,
    kIOUSBAnySubClass = 0xFFFF,
    kIOUSBAnyProtocol = 0xFFFF,
    kIOUSBAnyVendor   = 0xFFFF,
    kIOUSBAnyProduct  = 0xFFFF
};

// This structure are obsolete
//
typedef struct IOUSBMatch
{
    UInt16 usbClass;
    UInt16 usbSubClass;
    UInt16 usbProtocol;
    UInt16 usbVendor;
    UInt16 usbProduct;
} IOUSBMatch;

/*!
    @typedef IOUSBFindEndpointRequest
    @discussion Struct used to find endpoints of an interface
    type and direction are used to match endpoints,
    type, direction, maxPacketSize and interval are updated
    with the properties of the found endpoint.
    @field type Type of endpoint: kUSBControl, kUSBIsoc, kUSBBulk, kUSBInterrupt, kUSBAnyType.  If kUSBAnyType is specified, this field is treated as a don't care.
    @field direction Direction of endpoint: kUSBOut, kUSBIn, kUSBAnyDirn.   If kUSBAnyDirn is specified, this field is treated as a don't care.
    @field maxPacketSize maximum packet size of endpoint.
    @field interval Polling interval in mSec for endpoint.
 */
typedef struct
{
    UInt8  type;
    UInt8  direction;
    UInt16 maxPacketSize;
    UInt8  interval;
} IOUSBFindEndpointRequest;

/*!
    @struct IOUSBDevRequest
    @discussion Parameter block for control requests, using a simple pointer
    for the data to be transferred.
    @field bmRequestType Request type: kUSBStandard, kUSBClass or kUSBVendor
    @field bRequest Request code
    @field wValue 16 bit parameter for request, host endianess
    @field wIndex 16 bit parameter for request, host endianess
    @field wLength Length of data part of request, 16 bits, host endianess
    @field pData Pointer to data for request - data returned in bus endianess
    @field wLenDone Set by standard completion routine to number of data bytes
        actually transferred
 */
typedef struct
{
    UInt8  bmRequestType;
    UInt8  bRequest;
    UInt16 wValue;
    UInt16 wIndex;
    UInt16 wLength;
    void*  pData;
    UInt32 wLenDone;
} IOUSBDevRequest;
typedef IOUSBDevRequest* IOUSBDeviceRequestPtr;

/*!
    @struct IOUSBDevRequestTO
    @discussion Parameter block for control requests with timeouts, using a simple pointer
    for the data to be transferred.  Same as a IOUSBDevRequest except for the two extra timeout fields.
    @field bmRequestType Request type: kUSBStandard, kUSBClass or kUSBVendor
    @field bRequest Request code
    @field wValue 16 bit parameter for request, host endianess
    @field wIndex 16 bit parameter for request, host endianess
    @field wLength Length of data part of request, 16 bits, host endianess
    @field pData Pointer to data for request - data returned in bus endianess
    @field wLenDone Set by standard completion routine to number of data bytes
        actually transferred
    @field noDataTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if no data is transferred in this amount of time, the request will be aborted and returned.
    @field completionTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if the entire request is not completed in this amount of time, the request will be aborted and returned
 */
typedef struct
{
    UInt8  bmRequestType;
    UInt8  bRequest;
    UInt16 wValue;
    UInt16 wIndex;
    UInt16 wLength;
    void*  pData;
    UInt32 wLenDone;
    UInt32 noDataTimeout;
    UInt32 completionTimeout;
} IOUSBDevRequestTO;

/*!
    @enum Default timeout values
    @discussion default values used for data and completion timeouts.
 */
enum
{
    kUSBDefaultControlNoDataTimeoutMS     = 5000,
    kUSBDefaultControlCompletionTimeoutMS = 0
};

// Internal structure to pass parameters between IOUSBLib and UserClient
//
typedef struct
{
    UInt32 pipeRef;
    void*  buf;
    UInt32 size;
    UInt32 noDataTimeout;
    UInt32 completionTimeout;
} IOUSBBulkPipeReq;

#if KERNEL
/*!
    @struct IOUSBDevRequestDesc
    @discussion Parameter block for control requests, using a memory descriptor
    for the data to be transferred.  Only available in the kernel.
    @field bmRequestType Request type: kUSBStandard, kUSBClass or kUSBVendor
    @field bRequest Request code
    @field wValue 16 bit parameter for request, host endianess
    @field wIndex 16 bit parameter for request, host endianess
    @field wLength Length of data part of request, 16 bits, host endianess
    @field pData Pointer to memory descriptor for data for request - data returned in bus endianess
    @field wLenDone Set by standard completion routine to number of data bytes
     actually transferred
 */
typedef struct
{
    UInt8               bmRequestType;
    UInt8               bRequest;
    UInt16              wValue;
    UInt16              wIndex;
    UInt16              wLength;
    IOMemoryDescriptor* pData;
    UInt32              wLenDone;
} IOUSBDevRequestDesc;
// TODO: move IOUSBDevRequestDesc to be shared between kernel and user.

/*!
        @enum IOUSBFamilyIOOptionBit
        @discussion Options used exclusively by the USB Family when calling calling IOService APIs, such as open() and close().
    @constant kIOUSBInterfaceOpenAlt Open the alternate interface specified when creating the interface.
    @constant	kUSBOptionBitOpenExclusivelyBit	Used in open()'ing the IOUSBDevice or IOUSBInterface by the corresponding user client.  Only 1 user client can have exclusive access to those objects
 */
enum
{
    kIOUSBInterfaceOpenAlt                    = 0x00010000,
    kIOUSBInterfaceOpenAlternateInterfaceBit  = 16,
    kUSBOptionBitOpenExclusivelyBit           = 17,
    kIOUSBInterfaceOpenAlternateInterfaceMask = ( 1 << kIOUSBInterfaceOpenAlternateInterfaceBit ),
    kUSBOptionBitOpenExclusivelyMask          = ( 1 << kUSBOptionBitOpenExclusivelyBit )
};

#endif

// Internal structure to pass parameters between IOUSBLib and UserClient
//
// use a structure because there's a limit of 6 total arguments
// to a user client method.
typedef struct
{
    UInt8  bmRequestType;
    UInt8  bRequest;
    UInt16 wValue;
    UInt16 wIndex;
    UInt16 wLength;
    void*  pData;               // data pointer
    UInt32 wLenDone;            // # bytes transferred
    UInt8  pipeRef;
} IOUSBDevReqOOL;

// Internal structure to pass parameters between IOUSBLib and UserClient
//
typedef struct
{
    UInt8  bmRequestType;
    UInt8  bRequest;
    UInt16 wValue;
    UInt16 wIndex;
    UInt16 wLength;
    void*  pData;               // data pointer
    UInt32 wLenDone;            // # bytes transferred
    UInt8  pipeRef;
    UInt32 noDataTimeout;
    UInt32 completionTimeout;
} IOUSBDevReqOOLTO;

// Internal structure to pass parameters between IOUSBLib and UserClient
//
// Structure to request isochronous transfer
//
typedef struct
{
    UInt32          fPipe;
    void*           fBuffer;
    UInt32          fBufSize;
    UInt64          fStartFrame;
    UInt32          fNumFrames;
    IOUSBIsocFrame* fFrameCounts;
} IOUSBIsocStruct;

// Internal structure to pass parameters between IOUSBLib and UserClient
//
// Structure to request low latency isochronous transfer
//
struct IOUSBLowLatencyIsocStruct
{
    UInt32 fPipe;
    UInt32 fBufSize;
    UInt64 fStartFrame;
    UInt32 fNumFrames;
    UInt32 fUpdateFrequency;
    UInt32 fDataBufferCookie;
    UInt32 fDataBufferOffset;
    UInt32 fFrameListBufferCookie;
    UInt32 fFrameListBufferOffset;
};

typedef struct IOUSBLowLatencyIsocStruct IOUSBLowLatencyIsocStruct;


/*!
    @struct IOUSBGetFrameStruct
    @discussion Structure used from user space to return the frame number and a timestamp on when the frame register was read.
    @field frame frame number
    @field timeStamp  AbsoluteTime when the frame was updated
 */
typedef struct
{
    UInt64       frame;
    AbsoluteTime timeStamp;
} IOUSBGetFrameStruct;


/*!
    @struct IOUSBFindInterfaceRequest
    @discussion Structure used with FindNextInterface.
 */
typedef struct
{
    UInt16 bInterfaceClass;                     // requested class
    UInt16 bInterfaceSubClass;                  // requested subclass
    UInt16 bInterfaceProtocol;                  // requested protocol
    UInt16 bAlternateSetting;                   // requested alt setting
} IOUSBFindInterfaceRequest;

/*!
    @enum kIOUSBFindInterfaceDontCare
    @discussion Constant that can be used for the fields of IOUSBFindInterfaceRequest to specify that they should not be matched.
 */
enum
{
    kIOUSBFindInterfaceDontCare = 0xFFFF
};

/*!
    @enum kIOUSBVendorIDApple
    @discussion USB Vendor ID for Apple, Inc.
 */
enum
{
    kIOUSBVendorIDAppleComputer = kIOUSBAppleVendorID,
    kIOUSBVendorIDApple         = kIOUSBAppleVendorID
};

/*!
     @enum USBDeviceSpeed
     @discussion Returns the speed of a particular USB device.
     @constant  kUSBDeviceSpeedLow                 The device is a low speed device.
     @constant  kUSBDeviceSpeedFull                  The device is a full speed device.
     @constant  kUSBDeviceSpeedHigh                The device is a high speed device.
     @constant  kUSBDeviceSpeedSuper              The device is a SuperSpeed (Gen 1) 5Gbps device
     @constant  kUSBDeviceSpeedSuperPlus       The device is a SuperSpeed Plus (Gen 2) 10Gbps device
     @constant  kUSBDeviceSpeedSuperPlusBy2 The device is a SuperSpeed Plus (Gen 2 X 2) 20Gbps device
 */
enum
{
    kUSBDeviceSpeedLow          = 0,
    kUSBDeviceSpeedFull         = 1,
    kUSBDeviceSpeedHigh         = 2,
    kUSBDeviceSpeedSuper        = 3,
    kUSBDeviceSpeedSuperPlus    = 4,
    kUSBDeviceSpeedSuperPlusBy2 = 5
};

/*!
    @enum MicrosecondsInFrame
    @discussion Returns the number of microseconds in a USB frame.
    @constant	kUSBFullSpeedMicrosecondsInFrame	The device is attached to a bus running at full speed (1 ms / frame).
    @constant	kUSBHighSpeedMicrosecondsInFrame	The device is attached to a bus running at high speed (125 microseconds / frame).
 */
enum
{
    kUSBFullSpeedMicrosecondsInFrame = 1000,
    kUSBHighSpeedMicrosecondsInFrame = 125
};

//  During low latency transfers, the stack will set the frStatus for each frame to this value.  A client can check that to see if the transfer has completed.  We set the frStatus to a
//  valid return code when the transfer completes.
//
enum
{
    kUSBLowLatencyIsochTransferKey = 'llit'             // Set frStatus field of first frame in isoch transfer to designate as low latency
};

// This structure is DEPRECATED.  See the LowLatencyUserBufferInfoV2
//
typedef struct LowLatencyUserBufferInfo LowLatencyUserBufferInfo;

struct LowLatencyUserBufferInfo
{
    UInt32                    cookie;
    void*                     bufferAddress;
    IOByteCount               bufferSize;
    UInt32                    bufferType;
    Boolean                   isPrepared;
    LowLatencyUserBufferInfo* nextBuffer;
};

// This structure is DEPRECATED.  See the LowLatencyUserBufferInfoV3

typedef struct LowLatencyUserBufferInfoV2 LowLatencyUserBufferInfoV2;

struct LowLatencyUserBufferInfoV2
{
    UInt32                      cookie;
    void*                       bufferAddress;
    IOByteCount                 bufferSize;
    UInt32                      bufferType;
    Boolean                     isPrepared;
    void*                       mappedUHCIAddress;
    LowLatencyUserBufferInfoV2* nextBuffer;
};


// This structure is used to pass information for the low latency calls between user space and the kernel.
//
typedef struct LowLatencyUserBufferInfoV3 LowLatencyUserBufferInfoV3;

struct LowLatencyUserBufferInfoV3
{
    uint64_t                    cookie;
    mach_vm_address_t           bufferAddress;
    mach_vm_size_t              bufferSize;
    uint64_t                    bufferType;
    uint64_t                    isPrepared;
    mach_vm_address_t           mappedUHCIAddress;
    LowLatencyUserBufferInfoV3* nextBuffer;
};


/*!
   @enum USBLowLatencyBufferType
   @discussion Used to specify what kind of buffer to create when calling LowLatencyCreateBuffer().
   @constant	kUSBLowLatencyWriteBuffer	The buffer will be used to write data out to a device.
   @constant	kUSBLowLatencyReadBuffer	The buffer will be used to read data from a device.
   @constant	kUSBLowLatencyFrameListBuffer	The buffer will be used for a low latency isoch frame list.
 */
typedef enum
{
    kUSBLowLatencyWriteBuffer     = 0,
    kUSBLowLatencyReadBuffer      = 1,
    kUSBLowLatencyFrameListBuffer = 2
} USBLowLatencyBufferType;

// USB User Notification Types
//
enum
{
    kUSBNoUserNotificationType                  = 0,
    kUSBNotEnoughPowerNotificationType          = 1,
    kUSBIndividualOverCurrentNotificationType   = 2,
    kUSBGangOverCurrentNotificationType         = 3,
    kUSBiOSDeviceNotEnoughPowerNotificationType = 4,
    kUSBNotEnoughPowerNoACNotificationType      = 5,
    kUSBDeviceCountExceededNotificationType     = 6,
    kUSBEndpointCountExceededNotificationType   = 7,
    kUSBUnsupportedNotificationType             = 8,
    kUSBHubCountExceededNotificationType        = 9,
    kUSBTDMLowBatteryType                       = 10,
    kUSBCTBNotEnoughPowerNotificationType       = 11,
    kUSBCTBUnsupportedNotificationType          = 12,
    kUSBCUnsupportedTBPortNotificationType      = 13,
    kUSBCUnsupportedTBCableNotificationType     = 14
};

/*!
   @defined Property Definitions
   @discussion Useful property names in USB land.
 */
#define kUSBDevicePropertyBusPowerAvailable         "Bus Power Available"
#define kUSBDevicePropertyLocationID                kUSBHostPropertyLocationID
#define kUSBProductIDMask                           kUSBHostMatchingPropertyProductIDMask
#define kUSBProductIdsArrayName                     kUSBHostMatchingPropertyProductIDArray
#define kUSBSuspendPort                             "kSuspendPort"
#define kUSBExpressCardCantWake                     "ExpressCardCantWake"
#define kUSBDeviceResumeRecoveryTime                kUSBHostDevicePropertyResumeRecoveryTime
#define kUSBOutOfSpecMPSOK                          "Out of spec MPS OK"
#define kOverrideIfAtLocationID                     "OverrideIfAtLocationID"
#define kUSBDeviceCurrentConfiguration              kUSBHostDevicePropertyCurrentConfiguration
#define kUSBDeviceRemoteWakeOverride                kUSBHostDevicePropertyRemoteWakeOverride
#define kUSBDeviceConfigurationCurrentOverride      kUSBHostDevicePropertyConfigurationCurrentOverride
#define kUSBDeviceResetDurationOverride             kUSBHostDevicePropertyResetDurationOverride
#define kUSBDeviceDataToggleResetOverride           kUSBHostPropertyDataToggleResetOverride
#define kUSBDeviceFailedRequestedPower              kUSBHostDevicePropertyFailedRequestedPower
#define kUSBPropertyRemovable                       kUSBHostPortPropertyRemovable
#define kUSBPropertyTestMode                        kUSBHostPortPropertyTestMode
#define kUSBPropertyDebugLevel                      kUSBHostPropertyDebugOptions
#define kUSBHubPropertyPowerSupply                  kUSBHostHubPropertyPowerSupply
#define kUSBControllerSleepSupported                kUSBHostControllerPropertySleepSupported
#define kUSBPortPropertyBusCurrentAllocation        kUSBHostPortPropertyBusCurrentAllocation

// legacy properties
#define kUSBPreferredInterface                     "Preferred Interface"
#define kUSBPreferredInterfacePriority             "priority"
#define kOverrideAllowLowPower                     "kOverrideAllowLowPower"
#define kUSBUserClientEntitlementRequired          "UsbUserClientEntitlementRequired"
#define kUSBDevicePropertySpeed                    "Device Speed"

#if !__OPEN_SOURCE__ && TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
#define kUSBDevicePropertyAddress                   kUSBHostDevicePropertyAddress
#define kUSBPreferredConfiguration                  kUSBHostDevicePropertyPreferredConfiguration
#define kUSBControllerNeedsContiguousMemoryForIsoch kUSBHostControllerPropertyIsochronousRequiresContiguous
#define kUSBHubDontAllowLowPower                    kUSBHostHubPropertyIdlePolicy
#define kConfigurationDescriptorOverride            kUSBHostDevicePropertyConfigurationDescriptorOverride
#else
#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_14
#define kUSBDevicePropertyAddress                   "USB Address"
#define kUSBPreferredConfiguration                  "Preferred Configuration"
#define kUSBControllerNeedsContiguousMemoryForIsoch "Need contiguous memory for isoch"
#define kUSBHubDontAllowLowPower                    "kUSBHubDontAllowLowPower"
#define kConfigurationDescriptorOverride            "ConfigurationDescriptorOverride"
#else
#define kUSBDevicePropertyAddress                   kUSBHostDevicePropertyAddress
#define kUSBPreferredConfiguration                  kUSBHostDevicePropertyPreferredConfiguration
#define kUSBControllerNeedsContiguousMemoryForIsoch kUSBHostControllerPropertyIsochronousRequiresContiguous
#define kUSBHubDontAllowLowPower                    kUSBHostHubPropertyIdlePolicy
#define kConfigurationDescriptorOverride            kUSBHostDevicePropertyConfigurationDescriptorOverride
#endif /* MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_14 */
#endif


/*!
   @enum USBReEnumerateOptions
   @discussion Options used when calling ReEnumerateDevice.
   @constant	kUSBAddExtraResetTimeBit	Setting this bit will cause the Hub driver to wait 100ms before addressing the device after the reset following the re-enumeration.
   @constant	kUSBReEnumerateCaptureDeviceBit	Setting this bit will terminate any drivers attached to an IOUSBInterface for the device and to the IOUSBDevice itself.  It will not terminate
   any drivers attached to a Mass Storage Class IOUSBInterface.  A client needs to have the appropriate permissions in order to specify this bit.  See IOUSBLib.h
   @constant	kUSBReEnumerateReleaseDeviceBit	Setting this bit will return return any device that was captured back to the OS.  The driver for the IOUSBDevice will be loaded.  A client needs to have the appropriate permissions in order to specify this bit.  See IOUSBLib.h
 */
typedef enum
{
    kUSBAddExtraResetTimeBit         = 31,
    kUSBReEnumerateCaptureDeviceBit  = 30,
    kUSBReEnumerateReleaseDeviceBit  = 29,
    kUSBAddExtraResetTimeMask        = ( 1 << kUSBAddExtraResetTimeBit),
    kUSBReEnumerateCaptureDeviceMask = ( 1 << kUSBReEnumerateCaptureDeviceBit),
    kUSBReEnumerateReleaseDeviceMask = ( 1 << kUSBReEnumerateReleaseDeviceBit)
} USBReEnumerateOptions;

/*!
   @enum USBDeviceInformationBits
   @discussion  GetUSBDeviceInformation will return a unit32_t value with bits set indicating that a particular state is present in the USB device.  These bits are described here

   @constant	kUSBInformationDeviceIsCaptiveBit			The USB device is directly attached to its hub and cannot be removed.
   @constant	kUSBInformationDeviceIsAttachedToRootHubBit	The USB device is directly attached to the root hub
   @constant	kUSBInformationDeviceIsInternalBit			The USB device is internal to the enclosure (all the hubs it attaches to are captive)
   @constant	kUSBInformationDeviceIsConnectedBit			The USB device is connected to its hub
   @constant	kUSBInformationDeviceIsEnabledBit			The hub port to which the USB device is attached is enabled
   @constant	kUSBInformationDeviceIsSuspendedBit			The hub port to which the USB device is attached is suspended
   @constant	kUSBInformationDeviceIsInResetBit			The hub port to which the USB device is attached is being reset
   @constant	kUSBInformationDeviceOvercurrentBit			The USB device generated an overcurrent
   @constant	kUSBInformationDevicePortIsInTestModeBit	The hub port to which the USB device is attached is in test mode
   @constant  kUSBInformationDeviceIsRootHub				The device is the root hub simulation
   @constant  kUSBInformationRootHubisBuiltIn				If this is a root hub simulation and it's built into the enclosure, this bit is set.  If it's on an expansion card, it will be cleared
   @constant  kUSBInformationDeviceIsRemote				This device is "attached" to the controller through a remote connection
   @constant  kUSBInformationDeviceIsAttachedToEnclosure	The hub port to which the USB device is connected has a USB connector on the enclosure
   @constant  kUSBInformationDeviceIsOnThunderboltBit		The USB device is downstream of a controller that is attached through Thunderbolt

 */
typedef enum
{
    kUSBInformationDeviceIsCaptiveBit              = 0,
    kUSBInformationDeviceIsAttachedToRootHubBit    = 1,
    kUSBInformationDeviceIsInternalBit             = 2,
    kUSBInformationDeviceIsConnectedBit            = 3,
    kUSBInformationDeviceIsEnabledBit              = 4,
    kUSBInformationDeviceIsSuspendedBit            = 5,
    kUSBInformationDeviceIsInResetBit              = 6,
    kUSBInformationDeviceOvercurrentBit            = 7,
    kUSBInformationDevicePortIsInTestModeBit       = 8,
    kUSBInformationDeviceIsRootHub                 = 9,
    kUSBInformationRootHubisBuiltIn                = 10,
    kUSBInformationRootHubIsBuiltInBit             = 10,
    kUSBInformationDeviceIsRemote                  = 11,
    kUSBInformationDeviceIsAttachedToEnclosure     = 12,
    kUSBInformationDeviceIsOnThunderboltBit        = 13,
    kUSBInformationDeviceIsCaptiveMask             = (1 << kUSBInformationDeviceIsCaptiveBit),
    kUSBInformationDeviceIsAttachedToRootHubMask   = (1 << kUSBInformationDeviceIsAttachedToRootHubBit),
    kUSBInformationDeviceIsInternalMask            = (1 << kUSBInformationDeviceIsInternalBit),
    kUSBInformationDeviceIsConnectedMask           = (1 << kUSBInformationDeviceIsConnectedBit),
    kUSBInformationDeviceIsEnabledMask             = (1 << kUSBInformationDeviceIsEnabledBit),
    kUSBInformationDeviceIsSuspendedMask           = (1 << kUSBInformationDeviceIsSuspendedBit),
    kUSBInformationDeviceIsInResetMask             = (1 << kUSBInformationDeviceIsInResetBit),
    kUSBInformationDeviceOvercurrentMask           = (1 << kUSBInformationDeviceOvercurrentBit),
    kUSBInformationDevicePortIsInTestModeMask      = (1 << kUSBInformationDevicePortIsInTestModeBit),
    kUSBInformationDeviceIsRootHubMask             = (1 << kUSBInformationDeviceIsRootHub),
    kUSBInformationRootHubisBuiltInMask            = (1 << kUSBInformationRootHubisBuiltIn),
    kUSBInformationRootHubIsBuiltInMask            = (1 << kUSBInformationRootHubIsBuiltInBit),
    kUSBInformationDeviceIsRemoteMask              = (1 << kUSBInformationDeviceIsRemote),
    kUSBInformationDeviceIsAttachedToEnclosureMask = (1 << kUSBInformationDeviceIsAttachedToEnclosure),
    kUSBInformationDeviceIsOnThunderboltMask       = (1 << kUSBInformationDeviceIsOnThunderboltBit)
} USBDeviceInformationBits;

/*!
   @enum USBPowerRequestTypes
   @discussion Used to specify what kind of power will be reserved using the IOUSBDevice RequestExtraPower and ReturnExtraPower APIs.
   @constant	kUSBPowerDuringSleep	The power is to be used during sleep.
   @constant	kUSBPowerDuringWake		The power is to be used while the system is awake (i.e not sleeping)
   @constant	kUSBPowerRequestWakeRelease		When used with ReturnExtraPower(), it will send a message to all devices to return any extra wake power if possible.
   @constant	kUSBPowerRequestSleepRelease	When used with ReturnExtraPower(), it will send a message to all devices to return any sleep power if possible.
   @constant	kUSBPowerRequestWakeReallocate		When used with ReturnExtraPower(), it will send a message to all devices indicating that they can ask for more wake power, as some device has released it.
   @constant	kUSBPowerRequestSleepReallocate		When used with ReturnExtraPower(), it will send a message to all devices indicating that they can ask for more sleep power, as some device has released it.
   @constant	kUSBPowerDuringWakeRevocable		The power is to be used while the system is awake (i.e not sleeping), but can be taken away (via the kUSBPowerRequestWakeRelease message).  The system can then allocate that extra power to another device.
   @constant	kUSBPowerDuringWakeUSB3				This is used by the USB stack to allocate the 400mA extra for USB3, above the 500ma allocated by USB2
 */
typedef enum
{
    kUSBPowerDuringSleep            = 0,
    kUSBPowerDuringWake             = 1,
    kUSBPowerRequestWakeRelease     = 2,
    kUSBPowerRequestSleepRelease    = 3,
    kUSBPowerRequestWakeReallocate  = 4,
    kUSBPowerRequestSleepReallocate = 5,
    kUSBPowerDuringWakeRevocable    = 6,
    kUSBPowerDuringWakeUSB3         = 7
} USBPowerRequestTypes;

// these are not for a Public API, but are just the bits used below
enum
{
    kUSBNotificationPreForcedSuspendBit  =    0,
    kUSBNotificationPostForcedSuspendBit =    1,
    kUSBNotificationPreForcedResumeBit   =    2,
    kUSBNotificationPostForcedResumeBit  =    3,
};

/*!
   @enum USBNotificationTypes
   @discussion Used to register for USB notifications. These types may be OR'd together if more than one notification is desired. These notification are expected to be acknowledged before the process (e.g. system sleep or system wake) can be continued. See RegisterForNotification and AcknowledgeNotification in IOUSBDeviceInterface and IOUSBInterfaceInterface.
   @constant	kUSBNotificationPreForcedSuspend	A notification is sent prior to a forced suspend (e.g. system sleep).
   @constant	kUSBNotificationPostForcedSuspend	A notification is sent after a forced suspend has been completed (e.g. system sleep).
   @constant	kUSBNotificationPreForcedResume		A notification is sent before a resume which happens after a forced suspend (e.g. system wake).
   @constant	kUSBNotificationPostForcedResume	A notification is sent after a resume which happens after a forced suspend (e.g. system wake).
 */
typedef enum
{
    kUSBNotificationPreForcedSuspend  =    (1 << kUSBNotificationPreForcedSuspendBit),
    kUSBNotificationPostForcedSuspend =    (1 << kUSBNotificationPostForcedSuspendBit),
    kUSBNotificationPreForcedResume   =    (1 << kUSBNotificationPreForcedResumeBit),
    kUSBNotificationPostForcedResume  =    (1 << kUSBNotificationPostForcedResumeBit),
} USBNotificationTypes;


// Apple specific properties
#define kAppleRevocableExtraCurrent             "AAPL,revocable-extra-current"
#define kAppleExternalSuperSpeedPorts           "AAPL,ExternalSSPorts"
#define kAppleUnconnectedSuperSpeedPorts        "AAPL,UnconnectedSSPorts"
#define kAppleAcpiRootHubDepth                  "AAPL,root-hub-depth"

#define kAppleStandardPortCurrentInSleep        "AAPL,standard-port-current-in-sleep"

#define kAppleInternalUSBDevice                 "AAPL,device-internal"
#define kAppleExternalConnectorBitmap           "AAPL,ExternalConnectorBitmap"
#define kUSBBusID                               "AAPL,bus-id"
#define kApplePowerSupply                       "AAPL,power-supply"

// Deprecated Names and/or values
#define kAppleCurrentAvailable                  "AAPL,current-available"
#define kAppleCurrentInSleep                    "AAPL,current-in-sleep"
#define kApplePortCurrentInSleep                "AAPL,port-current-in-sleep"

#define kOverrideAttachedToCPU                  "kOverrideAttachedToCPU"

// UPC definitions from ACPI Rev 4.0
typedef enum
{
    kUSBPortNotConnectable = 0,                 // Port is not connectable
    kUSBPortConnectable    = 1                  // Port is connectable either user visible or invisible
} kUSBConnectable;

typedef enum
{
    kUSBTypeAConnector        = 0x00,           // Type ÔAÕ connector
    kUSBTypeMiniABConnector   = 0x01,           // Mini-AB connector
    kUSBTypeExpressCard       = 0x02,           // ExpressCard
    kUSB3TypeStdAConnector    = 0x03,           // USB 3 Standard-A connector
    kUSB3TypeStdBConnector    = 0x04,           // USB 3 Standard-B connector
    kUSB3TypeMicroBConnector  = 0x05,           // USB 3 Micro-B connector
    kUSB3TypeMicroABConnector = 0x06,           // USB 3 Micro-AB connector
    kUSB3TypePowerBConnector  = 0x07,           // USB 3 Power-B connector
    kUSBProprietaryConnector  = 0xFF            // Proprietary connector
} kUSBHostConnectorType;

// Root hub definitions
enum
{
    kUSBSpeed_Mask  = USBBitRange(0, 1),
    kUSBSpeed_Shift = USBBitRangePhase(0, 1),

    kUSBAddress_Mask  = USBBitRange(8, 15),
    kUSBAddress_Shift = USBBitRangePhase(8, 15)
};

enum
{
    kXHCISSRootHubAddress   = kUSBMaxDevices,
    kXHCIUSB2RootHubAddress = kUSBMaxDevices + 1,
    kSuperSpeedBusBitMask   = 0x01000000
};

#define ISROOTHUB(a) ((a == kXHCISSRootHubAddress) || (a == kXHCIUSB2RootHubAddress))

// values (in nanoseconds) which are sent to requireMaxBusStall as appropriate
#define kEHCIIsochMaxBusStall            25000
#define kXHCIIsochMaxBusStall            25000
#define kOHCIIsochMaxBusStall            25000
#define kUHCIIsochMaxBusStall            10000
#define kMaxBusStall10uS                 10000
#define kMaxBusStall25uS                 25000

#ifdef __cplusplus
}
#endif

#endif
