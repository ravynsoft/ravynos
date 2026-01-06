/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 2019 Apple Computer, Inc.  All Rights Reserved.
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

#ifndef _IOKIT_HID_IOHIDUSERDEVICE_H
#define _IOKIT_HID_IOHIDUSERDEVICE_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDKeys.h>

__BEGIN_DECLS

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

typedef struct CF_BRIDGED_TYPE(id) __IOHIDUserDevice * IOHIDUserDeviceRef;

/*!
 * @typedef IOHIDUserDeviceSetReportBlock
 *
 * @abstract
 * The type block used for IOHIDUserDevice set report calls.
 *
 * @param type
 * The report type.
 *
 * @param reportID
 * The report ID.
 *
 * @param report
 * The report bytes.
 *
 * @param reportLength
 * The length of the report being passed in.
 */
typedef IOReturn (^IOHIDUserDeviceSetReportBlock)(IOHIDReportType type,
                                                  uint32_t reportID,
                                                  const uint8_t *report,
                                                  CFIndex reportLength);

/*!
 * @typedef IOHIDUserDeviceGetReportBlock
 *
 * @abstract
 * The type block used for IOHIDUserDevice get report calls.
 *
 * @param type
 * The report type.
 *
 * @param reportID
 * The report ID.
 *
 * @param report
 * A buffer to be filled in by the implementor with the report.
 *
 * @param reportLength
 * The length of the report buffer being passed in. The implementor of this
 * block may update the reportLength variable to reflect the actual length of
 * the returned report.
 */
typedef IOReturn (^IOHIDUserDeviceGetReportBlock)(IOHIDReportType type,
                                                  uint32_t reportID,
                                                  uint8_t *report,
                                                  CFIndex *reportLength);

/*!
 * @enum IOHIDUserDeviceOptions
 *
 * @abstract
 * Enumerator of IOHIDUserDeviceOptions to be passed in to
 * IOHIDUserDeviceCreateWithOptions.
 *
 * @field IOHIDUserDeviceOptionsCreateOnActivate
 * Specifies that the kernel HID device should not be created until the call
 * to IOHIDUserDeviceActivate. This may be useful for preventing dropped get/set
 * report calls to the user device.
 */
typedef CF_ENUM(IOOptionBits, IOHIDUserDeviceOptions) {
    IOHIDUserDeviceOptionsCreateOnActivate = 1 << 0,
};

/*!
 * @function IOHIDUserDeviceGetTypeID
 *
 * @abstract
 * Returns the type identifier of all IOHIDUserDevice instances.
 */
CF_EXPORT
CFTypeID IOHIDUserDeviceGetTypeID(void);

/*!
 * @function IOHIDUserDeviceCreateWithProperties
 *
 * @abstract
 * Creates a virtual IOHIDDevice in the kernel.
 *
 * @discussion
 * The IOHIDUserDeviceRef represents a virtual IOHIDDevice. In order to create
 * the device, the entitlement "com.apple.developer.hid.virtual.device" is
 * required to validate the source of the device.
 *
 * @param allocator
 * Allocator to be used during creation.
 *
 * @param properties
 * Dictionary containing device properties indexed by keys defined in
 * IOHIDKeys.h. At the bare minimum, the kIOHIDReportDescriptorKey key must be
 * provided, where the value represents a CFData representation of the device's
 * report descriptor.
 *
 * @param options
 * Options to be used when creating the device.
 *
 * @result
 * Returns a IOHIDUserDeviceRef on success.
 */
CF_EXPORT
IOHIDUserDeviceRef _Nullable IOHIDUserDeviceCreateWithProperties(
                                            CFAllocatorRef _Nullable allocator,
                                            CFDictionaryRef properties,
                                            IOOptionBits options)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDUserDeviceRegisterGetReportBlock
 *
 * @abstract
 * Registers a block to receive get report requests.
 *
 * @discussion
 * The call to IOHIDUserDeviceRegisterGetReportBlock should be made before the
 * device is activated. The device must be activated in order to receive
 * get report requests.
 *
 * @param device
 * Reference to a IOHIDUserDeviceRef
 *
 * @param block
 * The block to be invoked for get report calls.
 */
CF_EXPORT
void IOHIDUserDeviceRegisterGetReportBlock(IOHIDUserDeviceRef device,
                                           IOHIDUserDeviceGetReportBlock block)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDUserDeviceRegisterSetReportBlock
 *
 * @abstract
 * Registers a block to receive set report requests.
 *
 * @discussion
 * The call to IOHIDUserDeviceRegisterSetReportBlock should be made before the
 * device is activated. The device must be activated in order to receive set
 * report requests.
 *
 * @param device
 * Reference to a IOHIDUserDeviceRef
 *
 * @param block
 * The block to be invoked for set report calls.
 */
CF_EXPORT
void IOHIDUserDeviceRegisterSetReportBlock(IOHIDUserDeviceRef device,
                                           IOHIDUserDeviceSetReportBlock block)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDUserDeviceSetDispatchQueue
 *
 * @abstract
 * Sets the dispatch queue to be associated with the IOHIDUserDevice.
 * This is necessary in order to receive asynchronous events from the kernel.
 *
 * @discussion
 * A call to IOHIDUserDeviceSetDispatchQueue should only be made once.
 *
 * After a dispatch queue is set, the IOHIDUserDevice must make a call to
 * activate via IOHIDUserDeviceActivate and cancel via IOHIDUserDeviceCancel.
 * All calls to "Register" functions should be done before activation and not
 * after cancellation.
 *
 * @param device
 * Reference to an IOHIDUserDevice
 *
 * @param queue
 * The dispatch queue to which the event handler block will be submitted.
 */
CF_EXPORT
void IOHIDUserDeviceSetDispatchQueue(IOHIDUserDeviceRef device,
                                     dispatch_queue_t queue)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDUserDeviceSetCancelHandler
 *
 * @abstract
 * Sets a cancellation handler for the dispatch queue associated with
 * IOHIDUserDeviceScheduleWithDispatchQueue.
 *
 * @discussion
 * The cancellation handler (if specified) will be submitted to the device's
 * dispatch queue in response to a call to to IOHIDUserDeviceCancel
 * after all the events have been handled.
 *
 * The IOHIDUserDeviceRef should only be released after the device has been
 * cancelled, and the cancel handler has been called. This is to ensure all
 * asynchronous objects are released. For example:
 *
 *     dispatch_block_t cancelHandler = dispatch_block_create(0, ^{
 *         CFRelease(device);
 *     });
 *     IOHIDUserDeviceSetCancelHandler(device, cancelHandler);
 *     IOHIDUserDeviceActivate(device);
 *     IOHIDUserDeviceCancel(device);
 *
 * @param device
 * Reference to an IOHIDUserDevice.
 *
 * @param handler
 * The cancellation handler block to be associated with the dispatch queue.
 */
CF_EXPORT
void IOHIDUserDeviceSetCancelHandler(IOHIDUserDeviceRef device,
                                     dispatch_block_t handler)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDUserDeviceActivate
 *
 * @abstract
 * Activates the IOHIDUserDevice object.
 *
 * @discussion
 * An IOHIDUserDevice object associated with a dispatch queue is created
 * in an inactive state. The object must be activated in order to
 * receive asynchronous events from the kernel.
 *
 * A dispatch queue must be set via IOHIDUserDeviceSetDispatchQueue before
 * activation.
 *
 * An activated device must be cancelled via IOHIDUserDeviceCancel. All calls
 * to "Register" functions should be done before activation and not after
 * cancellation.
 *
 * Calling IOHIDUserDeviceActivate on an active IOHIDUserDevice has no effect.
 *
 * @param device
 * Reference to an IOHIDUserDevice.
 */
CF_EXPORT
void IOHIDUserDeviceActivate(IOHIDUserDeviceRef device)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDUserDeviceCancel
 *
 * @abstract
 * Cancels the IOHIDUserDevice preventing any further invocation of its event
 * handler block.
 *
 * @discussion
 * Cancelling prevents any further invocation of the event handler block for
 * the specified dispatch queue, but does not interrupt an event handler block
 * that is already in progress.
 *
 * Explicit cancellation of the IOHIDUserDevice is required, no implicit
 * cancellation takes place.
 *
 * Calling IOHIDUserDeviceCancel on an already cancelled queue has no effect.
 *
 * The IOHIDUserDeviceRef should only be released after the device has been
 * cancelled, and the cancel handler has been called. This is to ensure all
 * asynchronous objects are released. For example:
 *
 *     dispatch_block_t cancelHandler = dispatch_block_create(0, ^{
 *         CFRelease(device);
 *     });
 *     IOHIDUserDeviceSetCancelHandler(device, cancelHandler);
 *     IOHIDUserDeviceActivate(device);
 *     IOHIDUserDeviceCancel(device);
 *
 * @param device
 * Reference to an IOHIDUserDevice
 */
CF_EXPORT
void IOHIDUserDeviceCancel(IOHIDUserDeviceRef device)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDUserDeviceCopyProperty
 *
 * @abstract
 * Obtains a property from the device.
 *
 * @param key
 * The property key.
 *
 * @result
 * Returns the property on success.
 */
CF_EXPORT
CFTypeRef _Nullable IOHIDUserDeviceCopyProperty(IOHIDUserDeviceRef device,
                                                CFStringRef key)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDUserDeviceSetProperty
 *
 * @abstract
 * Sets a property on the device.
 *
 * @param key
 * The property key.
 *
 * @param value
 * The value of the property.
 *
 * @result
 * Returns true on success.
 */
CF_EXPORT
Boolean IOHIDUserDeviceSetProperty(IOHIDUserDeviceRef device,
                                   CFStringRef key,
                                   CFTypeRef property)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDUserDeviceHandleReportWithTimeStamp
 *
 * @abstract
 * Dispatches a report on behalf of the device.
 *
 * @param device
 * Reference to a IOHIDUserDeviceRef.
 *
 * @param timestamp
 * mach_absolute_time() based timestamp.
 *
 * @param report
 * Buffer containing a HID report.
 *
 * @param reportLength
 * The report buffer length.
 *
 * @result
 * Returns kIOReturnSuccess on success.
 */
CF_EXPORT
IOReturn IOHIDUserDeviceHandleReportWithTimeStamp(IOHIDUserDeviceRef device,
                                                  uint64_t timestamp,
                                                  const uint8_t *report,
                                                  CFIndex reportLength)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

__END_DECLS

#endif /* _IOKIT_HID_IOHIDUSERDEVICE_H */
