/*
 * Copyright (c) 1999-2008 Apple Computer, Inc.  All Rights Reserved.
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

#ifndef _IOKIT_HID_IOHIDDEVICE_USER_H
#define _IOKIT_HID_IOHIDDEVICE_USER_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDBase.h>

/*!
	@header IOHIDDevice
    IOHIDDevice defines a Human Interface Device (HID) object, which interacts 
    with an IOHIDDevicePlugIn object that typically maps to an object in the 
    kernel.  IOHIDDevice is used to communicate with a single HID device in 
    order to obtain or set device properties, element values, and reports.
    IOHIDDevice is also a CFType object and as such conforms to all the 
    conventions expected such object.
    <p>
    All of the information described in this document is contained in the header 
    file <font face="Courier New,Courier,Monaco">IOHIDDevice.h</font> found at 
    <font face="Courier New,Courier,Monaco">/System/Library/Frameworks/IOKit.framework/Headers/hid/IOHIDDevice.h</font>.
*/

__BEGIN_DECLS

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

/*!
	@function   IOHIDDeviceGetTypeID
	@abstract   Returns the type identifier of all IOHIDDevice instances.
*/
CF_EXPORT
CFTypeID IOHIDDeviceGetTypeID(void)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDDeviceCreate
	@abstract   Creates an element from an io_service_t.
    @discussion The io_service_t passed in this method must reference an object 
                in the kernel of type IOHIDDevice.
    @param      allocator Allocator to be used during creation.
    @param      service Reference to service object in the kernel.
    @result     Returns a new IOHIDDeviceRef.
*/
CF_EXPORT
IOHIDDeviceRef _Nullable IOHIDDeviceCreate(
                                CFAllocatorRef _Nullable        allocator,
                                io_service_t                    service)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
    @function   IOHIDDeviceGetService
    @abstract   Returns the io_service_t for an IOHIDDevice, if it has one.
    @discussion If the IOHIDDevice references an object in the kernel, this is
                used to get the io_service_t for that object.
    @param      device Reference to an IOHIDDevice.
    @result     Returns the io_service_t if the IOHIDDevice has one, or 
                MACH_PORT_NULL if it does not.
 */
CF_EXPORT
io_service_t IOHIDDeviceGetService(
                                 IOHIDDeviceRef                  device)
AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
	@function   IOHIDDeviceOpen
	@abstract   Opens a HID device for communication.
    @discussion Before the client can issue commands that change the state of 
                the device, it must have succeeded in opening the device. This 
                establishes a link between the client's task and the actual 
                device.  To establish an exclusive link use the 
                kIOHIDOptionsTypeSeizeDevice option. 
    @param      device Reference to an IOHIDDevice.
    @param      options Option bits to be sent down to the device.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceOpen(          
                                IOHIDDeviceRef                  device, 
                                IOOptionBits                    options)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDDeviceClose
	@abstract   Closes communication with a HID device.
    @discussion This closes a link between the client's task and the actual 
                device.
    @param      device Reference to an IOHIDDevice.
    @param      options Option bits to be sent down to the device.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceClose(
                                IOHIDDeviceRef                  device, 
                                IOOptionBits                    options)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDDeviceConformsTo
	@abstract   Convenience function that scans the Application Collection 
                elements to see if it conforms to the provided usagePage 
                and usage.
    @discussion Examples of Application Collection usages pairs are:
                <br>
                    usagePage = kHIDPage_GenericDesktop  <br>
                    usage = kHIDUsage_GD_Mouse
                <br>
                <b>or</b>
                <br>
                    usagePage = kHIDPage_GenericDesktop  <br>
                    usage = kHIDUsage_GD_Keyboard
    @param      device Reference to an IOHIDDevice.
    @param      usagePage Device usage page
    @param      usage Device usage
    @result     Returns TRUE if device conforms to provided usage.
*/
CF_EXPORT
Boolean IOHIDDeviceConformsTo(          
                                IOHIDDeviceRef                  device, 
                                uint32_t                        usagePage,
                                uint32_t                        usage)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;


/*!
	@function   IOHIDDeviceGetProperty
	@abstract   Obtains a property from an IOHIDDevice.
    @discussion Property keys are prefixed by kIOHIDDevice and declared in 
                <IOKit/hid/IOHIDKeys.h>.
    @param      device Reference to an IOHIDDevice.
    @param      key CFStringRef containing key to be used when querying the 
                device.
    @result     Returns CFTypeRef containing the property.
*/
CF_EXPORT
CFTypeRef _Nullable IOHIDDeviceGetProperty(
                                IOHIDDeviceRef                  device, 
                                CFStringRef                     key)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*!
	@function   IOHIDDeviceSetProperty
	@abstract   Sets a property for an IOHIDDevice.
    @discussion Property keys are prefixed by kIOHIDDevice and declared in 
                <IOKit/hid/IOHIDKeys.h>.
    @param      device Reference to an IOHIDDevice.
    @param      key CFStringRef containing key to be used when modifiying the 
                device property.
    @param      property CFTypeRef containg the property to be set.
    @result     Returns TRUE if successful.
*/
CF_EXPORT
Boolean IOHIDDeviceSetProperty(
                                IOHIDDeviceRef                  device,
                                CFStringRef                     key,
                                CFTypeRef                       property)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDDeviceCopyMatchingElements
    @abstract   Obtains HID elements that match the criteria contained in the 
                matching dictionary.
    @discussion Matching keys are prefixed by kIOHIDElement and declared in 
                <IOKit/hid/IOHIDKeys.h>.  Passing a NULL dictionary will result
                in all device elements being returned.
    @param      device Reference to an IOHIDDevice.
    @param      matching CFDictionaryRef containg element matching criteria.
    @param      options Reserved for future use.
    @result     Returns CFArrayRef containing multiple IOHIDElement object.
*/
CF_EXPORT 
CFArrayRef _Nullable IOHIDDeviceCopyMatchingElements(
                                IOHIDDeviceRef                  device, 
                                CFDictionaryRef _Nullable       matching,
                                IOOptionBits                    options)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceScheduleWithRunLoop
    @abstract   Schedules HID device with run loop.
    @discussion Formally associates device with client's run loop. Scheduling
                this device with the run loop is necessary before making use of
                any asynchronous APIs.
    @param      device Reference to an IOHIDDevice.
    @param      runLoop RunLoop to be used when scheduling any asynchronous 
                activity.
    @param      runLoopMode Run loop mode to be used when scheduling any 
                asynchronous activity.
*/
CF_EXPORT
void IOHIDDeviceScheduleWithRunLoop(
                                IOHIDDeviceRef                  device, 
                                CFRunLoopRef                    runLoop, 
                                CFStringRef                     runLoopMode)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceUnscheduleFromRunLoop
    @abstract   Unschedules HID device with run loop.
    @discussion Formally disassociates device with client's run loop.
    @param      device Reference to an IOHIDDevice.
    @param      runLoop RunLoop to be used when unscheduling any asynchronous 
                activity.
    @param      runLoopMode Run loop mode to be used when unscheduling any 
                asynchronous activity.
*/
CF_EXPORT
void IOHIDDeviceUnscheduleFromRunLoop(  
                                IOHIDDeviceRef                  device, 
                                CFRunLoopRef                    runLoop, 
                                CFStringRef                     runLoopMode)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
 * @function IOHIDDeviceSetDispatchQueue
 *
 * @abstract
 * Sets the dispatch queue to be associated with the IOHIDDevice.
 * This is necessary in order to receive asynchronous events from the kernel.
 *
 * @discussion
 * An IOHIDDevice should not be associated with both a runloop and
 * dispatch queue. A call to IOHIDDeviceSetDispatchQueue should only be made once.
 *
 * If a dispatch queue is set but never used, a call to IOHIDDeviceCancel followed
 * by IOHIDDeviceActivate should be performed in that order.
 *
 * After a dispatch queue is set, the IOHIDDevice must make a call to activate
 * via IOHIDDeviceActivate and cancel via IOHIDDeviceCancel. All calls to "Register"
 * functions should be done before activation and not after cancellation.
 *
 * @param device
 * Reference to an IOHIDDevice
 *
 * @param queue
 * The dispatch queue to which the event handler block will be submitted.
 */
CF_EXPORT
void IOHIDDeviceSetDispatchQueue(
                                IOHIDDeviceRef                  device,
                                dispatch_queue_t                queue)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDDeviceSetCancelHandler
 *
 * @abstract
 * Sets a cancellation handler for the dispatch queue associated with
 * IOHIDDeviceSetDispatchQueue.
 *
 * @discussion
 * The cancellation handler (if specified) will be will be submitted to the
 * device's dispatch queue in response to a call to IOHIDDeviceCancel after
 * all the events have been handled.
 *
 * IOHIDDeviceSetCancelHandler should not be used when scheduling with
 * a run loop.
 *
 * The IOHIDDeviceRef should only be released after the device has been
 * cancelled, and the cancel handler has been called. This is to ensure all
 * asynchronous objects are released. For example:
 *
 *     dispatch_block_t cancelHandler = dispatch_block_create(0, ^{
 *         CFRelease(device);
 *     });
 *     IOHIDDeviceSetCancelHandler(device, cancelHandler);
 *     IOHIDDeviceActivate(device);
 *     IOHIDDeviceCancel(device);
 *
 * @param device
 * Reference to an IOHIDDevice.
 *
 * @param handler
 * The cancellation handler block to be associated with the dispatch queue.
 */
CF_EXPORT
void IOHIDDeviceSetCancelHandler(
                                 IOHIDDeviceRef                 device,
                                 dispatch_block_t               handler)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDDeviceActivate
 *
 * @abstract
 * Activates the IOHIDDevice object.
 *
 * @discussion
 * An IOHIDDevice object associated with a dispatch queue is created
 * in an inactive state. The object must be activated in order to
 * receive asynchronous events from the kernel.
 *
 * A dispatch queue must be set via IOHIDDeviceSetDispatchQueue before
 * activation.
 *
 * An activated device must be cancelled via IOHIDDeviceCancel. All calls
 * to "Register" functions should be done before activation
 * and not after cancellation.
 *
 * Calling IOHIDDeviceActivate on an active IOHIDDevice has no effect.
 *
 * @param device
 * Reference to an IOHIDDevice
 */
CF_EXPORT
void IOHIDDeviceActivate(
                                 IOHIDDeviceRef                 device)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDDeviceCancel
 *
 * @abstract
 * Cancels the IOHIDDevice preventing any further invocation
 * of its event handler block.
 *
 * @discussion
 * Cancelling prevents any further invocation of the event handler block for
 * the specified dispatch queue, but does not interrupt an event handler
 * block that is already in progress.
 *
 * Explicit cancellation of the IOHIDDevice is required, no implicit
 * cancellation takes place.
 *
 * Calling IOHIDDeviceCancel on an already cancelled queue has no effect.
 *
 * The IOHIDDeviceRef should only be released after the device has been
 * cancelled, and the cancel handler has been called. This is to ensure all
 * asynchronous objects are released. For example:
 *
 *     dispatch_block_t cancelHandler = dispatch_block_create(0, ^{
 *         CFRelease(device);
 *     });
 *     IOHIDDeviceSetCancelHandler(device, cancelHandler);
 *     IOHIDDeviceActivate(device);
 *     IOHIDDeviceCancel(device);
 *
 * @param device
 * Reference to an IOHIDDevice
 */
CF_EXPORT
void IOHIDDeviceCancel(
                                 IOHIDDeviceRef                 device)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*! @function   IOHIDDeviceRegisterRemovalCallback
    @abstract   Registers a callback to be used when a IOHIDDevice is removed.
    @discussion In most cases this occurs when a device is unplugged.
                If a dispatch queue is set, this call must occur before activation.
    @param      device Reference to an IOHIDDevice.
    @param      callback Pointer to a callback method of type IOHIDCallback.
    @param      context Pointer to data to be passed to the callback.
*/
CF_EXPORT
void IOHIDDeviceRegisterRemovalCallback( 
                                IOHIDDeviceRef                  device, 
                                IOHIDCallback _Nullable         callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceRegisterInputValueCallback
    @abstract   Registers a callback to be used when an input value is issued by 
                the device.
    @discussion An input element refers to any element of type 
                kIOHIDElementTypeInput and is usually issued by interrupt driven
                reports.  If more specific element values are desired, you can 
                specify matching criteria via IOHIDDeviceSetInputValueMatching
                and IOHIDDeviceSetInputValueMatchingMultiple.
                If a dispatch queue is set, this call must occur before activation.
    @param      device Reference to an IOHIDDevice.
    @param      callback Pointer to a callback method of type IOHIDValueCallback.
    @param      context Pointer to data to be passed to the callback.
*/
CF_EXPORT
void IOHIDDeviceRegisterInputValueCallback(
                                IOHIDDeviceRef                  device,
                                IOHIDValueCallback _Nullable    callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceRegisterInputReportCallback
    @abstract   Registers a callback to be used when an input report is issued 
                by the device.
    @discussion An input report is an interrupt driver report issued by the 
                device.
                If a dispatch queue is set, this call must occur before activation.
    @param      device Reference to an IOHIDDevice.
    @param      report Pointer to preallocated buffer in which to copy inbound
                report data.
    @param      reportLength Length of preallocated buffer.
    @param      callback Pointer to a callback method of type 
                IOHIDReportCallback.
    @param      context Pointer to data to be passed to the callback.
*/
CF_EXPORT
void IOHIDDeviceRegisterInputReportCallback( 
                                IOHIDDeviceRef                  device, 
                                uint8_t *                       report, 
                                CFIndex                         reportLength,
                                IOHIDReportCallback _Nullable   callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceRegisterInputReportWithTimeStampCallback
    @abstract   Registers a timestamped callback to be used when an input report is issued 
                by the device.
    @discussion An input report is an interrupt driver report issued by the 
                device.
                If a dispatch queue is set, this call must occur before activation.
    @param      device Reference to an IOHIDDevice.
    @param      report Pointer to preallocated buffer in which to copy inbound
                report data.
    @param      reportLength Length of preallocated buffer.
    @param      callback Pointer to a callback method of type 
                IOHIDReportWithTimeStampCallback.
    @param      context Pointer to data to be passed to the callback.
*/
CF_EXPORT
void IOHIDDeviceRegisterInputReportWithTimeStampCallback(
                                IOHIDDeviceRef                      device, 
                                uint8_t *                           report, 
                                CFIndex                             reportLength,
                                IOHIDReportWithTimeStampCallback _Nullable  callback,
                                void * _Nullable                    context)
AVAILABLE_MAC_OS_X_VERSION_10_10_AND_LATER;

/*! @function   IOHIDDeviceSetInputValueMatching
    @abstract   Sets matching criteria for input values received via 
                IOHIDDeviceRegisterInputValueCallback.
    @discussion Matching keys are prefixed by kIOHIDElement and declared in 
                <IOKit/hid/IOHIDKeys.h>.  Passing a NULL dictionary will result
                in all devices being enumerated. Any subsequent calls will cause
                the hid manager to release previously matched input elements and 
                restart the matching process using the revised criteria.  If 
                interested in multiple, specific device elements, please defer to
                using IOHIDDeviceSetInputValueMatchingMultiple.
                If a dispatch queue is set, this call must occur before activation.
    @param      device Reference to an IOHIDDevice.
    @param      matching CFDictionaryRef containg device matching criteria.
*/
CF_EXPORT
void IOHIDDeviceSetInputValueMatching(
                                IOHIDDeviceRef                  device, 
                                CFDictionaryRef _Nullable       matching)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*! @function   IOHIDDeviceSetInputValueMatchingMultiple
    @abstract   Sets multiple matching criteria for input values received via 
                IOHIDDeviceRegisterInputValueCallback.
    @discussion Matching keys are prefixed by kIOHIDElement and declared in 
                <IOKit/hid/IOHIDKeys.h>.  This method is useful if interested 
                in multiple, specific elements.
                If a dispatch queue is set, this call must occur before activation.
    @param      device Reference to an IOHIDDevice.
    @param      multiple CFArrayRef containing multiple CFDictionaryRef objects
                containg input element matching criteria.
*/
CF_EXPORT
void IOHIDDeviceSetInputValueMatchingMultiple(
                                IOHIDDeviceRef                  device, 
                                CFArrayRef _Nullable            multiple)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*! @function   IOHIDDeviceSetValue
    @abstract   Sets a value for an element.
    @discussion This method behaves synchronously and will block until the
                report has been issued to the device.  It is only relevent for 
                either output or feature type elements.  If setting values for 
                multiple elements you may want to consider using 
                IOHIDDeviceSetValueMultiple or IOHIDTransaction.
    @param      device Reference to an IOHIDDevice.
    @param      element IOHIDElementRef whose value is to be modified.
    @param      value IOHIDValueRef containing value to be set.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceSetValue(
                                IOHIDDeviceRef                  device, 
                                IOHIDElementRef                 element, 
                                IOHIDValueRef                   value)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceSetValueMultiple
    @abstract   Sets multiple values for multiple elements.
    @discussion This method behaves synchronously and will block until the
                report has been issued to the device.  It is only relevent for 
                either output or feature type elements.
    @param      device Reference to an IOHIDDevice.
    @param      multiple CFDictionaryRef where key is IOHIDElementRef and
                value is IOHIDValueRef.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceSetValueMultiple(
                                IOHIDDeviceRef                  device, 
                                CFDictionaryRef                 multiple)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceSetValueWithCallback
    @abstract   Sets a value for an element and returns status via a completion
                callback.
    @discussion This method behaves asynchronously and will invoke the callback
                once the report has been issued to the device.  It is only 
                relevent for either output or feature type elements.  
                If setting values for multiple elements you may want to 
                consider using IOHIDDeviceSetValueWithCallback or 
                IOHIDTransaction.
    @param      device Reference to an IOHIDDevice.
    @param      element IOHIDElementRef whose value is to be modified.
    @param      value IOHIDValueRef containing value to be set.
    @param      timeout CFTimeInterval containing the timeout.
    @param      callback Pointer to a callback method of type 
                IOHIDValueCallback.
    @param      context Pointer to data to be passed to the callback.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceSetValueWithCallback(
                                IOHIDDeviceRef                  device, 
                                IOHIDElementRef                 element, 
                                IOHIDValueRef                   value, 
                                CFTimeInterval                  timeout,
                                IOHIDValueCallback _Nullable    callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceSetValueMultipleWithCallback
    @abstract   Sets multiple values for multiple elements and returns status 
                via a completion callback.
    @discussion This method behaves asynchronously and will invoke the callback
                once the report has been issued to the device.  It is only 
                relevent for either output or feature type elements.  
    @param      device Reference to an IOHIDDevice.
    @param      multiple CFDictionaryRef where key is IOHIDElementRef and
                value is IOHIDValueRef.
    @param      timeout CFTimeInterval containing the timeout.
    @param      callback Pointer to a callback method of type 
                IOHIDValueMultipleCallback.
    @param      context Pointer to data to be passed to the callback.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceSetValueMultipleWithCallback(
                                IOHIDDeviceRef                  device, 
                                CFDictionaryRef                 multiple,
                                CFTimeInterval                  timeout,
                                IOHIDValueMultipleCallback _Nullable    callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceGetValue
    @abstract   Gets a value for an element.
    @discussion This method behaves synchronously and return back immediately
                for input type element.  If requesting a value for a feature
                element, this will block until the report has been issued to the
                device.  If obtaining values for multiple elements you may want 
                to consider using IOHIDDeviceCopyValueMultiple or IOHIDTransaction.
    @param      device Reference to an IOHIDDevice.
    @param      element IOHIDElementRef whose value is to be obtained.
    @param      pValue Pointer to IOHIDValueRef to be obtained.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceGetValue(
                                IOHIDDeviceRef                  device, 
                                IOHIDElementRef                 element, 
                                IOHIDValueRef _Nonnull * _Nonnull   pValue)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;


typedef CF_ENUM(uint32_t, IOHIDDeviceGetValueOptions) {
    kIOHIDDeviceGetValueWithUpdate        = 0x00020000,   //  force value update (get report will be send to the device)
    kIOHIDDeviceGetValueWithoutUpdate     = 0x00040000    //  get last value
};


/*! @function   IOHIDDeviceGetValueWithOptions
 @abstract   Gets a value for an element.
 @discussion This method behaves synchronously and return back immediately
 for input type element.  If requesting a value for a feature
 element, this will block until the report has been issued to the
 device.  If obtaining values for multiple elements you may want
 to consider using IOHIDDeviceCopyValueMultiple or IOHIDTransaction.
 @param      device Reference to an IOHIDDevice.
 @param      element IOHIDElementRef whose value is to be obtained.
 @param      pValue Pointer to IOHIDValueRef to be obtained.
 @param      options (see IOHIDDeviceGetValueOptions).
 @result     Returns kIOReturnSuccess if successful.
 */
IOReturn IOHIDDeviceGetValueWithOptions (
                                        IOHIDDeviceRef                      device,
                                        IOHIDElementRef                     element,
                                        IOHIDValueRef  _Nonnull * _Nonnull  pValue,
                                        uint32_t                            options)
AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER;

/*! @function   IOHIDDeviceCopyValueMultiple
    @abstract   Copies a values for multiple elements.
    @discussion This method behaves synchronously and return back immediately
                for input type element.  If requesting a value for a feature
                element, this will block until the report has been issued to the
                device.
    @param      device Reference to an IOHIDDevice.
    @param      elements CFArrayRef containing multiple IOHIDElementRefs whose 
                values are to be obtained.
    @param      pMultiple Pointer to CFDictionaryRef where the keys are the 
                provided elements and the values are the requested values.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceCopyValueMultiple(
                                IOHIDDeviceRef                  device, 
                                CFArrayRef                      elements, 
                                CFDictionaryRef _Nullable * _Nullable   pMultiple)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceGetValueWithCallback
    @abstract   Gets a value for an element and returns status via a completion
                callback.
    @discussion This method behaves asynchronusly and is only relevent for 
                either output or feature type elements. If obtaining values for 
                multiple elements you may want to consider using 
                IOHIDDeviceCopyValueMultipleWithCallback or IOHIDTransaction.
    @param      device Reference to an IOHIDDevice.
    @param      element IOHIDElementRef whose value is to be obtained.
    @param      pValue Pointer to IOHIDValueRef to be passedback.
    @param      timeout CFTimeInterval containing the timeout.
    @param      callback Pointer to a callback method of type 
                IOHIDValueCallback.
    @param      context Pointer to data to be passed to the callback.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceGetValueWithCallback(
                                IOHIDDeviceRef                  device, 
                                IOHIDElementRef                 element, 
                                IOHIDValueRef _Nonnull * _Nonnull    pValue,
                                CFTimeInterval                  timeout,
                                IOHIDValueCallback _Nullable    callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;


/*! @function   IOHIDDeviceCopyValueMultipleWithCallback
    @abstract   Copies a values for multiple elements and returns status via a 
                completion callback.
    @discussion This method behaves asynchronusly and is only relevent for 
                either output or feature type elements.
    @param      device Reference to an IOHIDDevice.
    @param      elements CFArrayRef containing multiple IOHIDElementRefs whose 
                values are to be obtained.
    @param      pMultiple Pointer to CFDictionaryRef where the keys are the 
                provided elements and the values are the requested values.
    @param      timeout CFTimeInterval containing the timeout.
    @param      callback Pointer to a callback method of type 
                IOHIDValueMultipleCallback.
    @param      context Pointer to data to be passed to the callback.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT

IOReturn IOHIDDeviceCopyValueMultipleWithCallback(
                                IOHIDDeviceRef                  device, 
                                CFArrayRef                      elements, 
                                CFDictionaryRef _Nullable * _Nullable   pMultiple,
                                CFTimeInterval                  timeout,
                                IOHIDValueMultipleCallback _Nullable    callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*! @function   IOHIDDeviceSetReport
    @abstract   Sends a report to the device.
    @discussion This method behaves synchronously and will block until the
                report has been issued to the device.  It is only relevent for 
                either output or feature type reports.
    @param      device Reference to an IOHIDDevice.
    @param      reportType Type of report being sent.
    @param      reportID ID of the report being sent.  If the device supports
                multiple reports, this should also be set in the first byte of
                the report.
    @param      report The report bytes to be sent to the device.
    @param      reportLength The length of the report to be sent to the device.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceSetReport(
                                IOHIDDeviceRef                  device,
                                IOHIDReportType                 reportType,
                                CFIndex                         reportID,
                                const uint8_t *                 report,
                                CFIndex                         reportLength)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*! @function   IOHIDDeviceSetReportWithCallback
    @abstract   Sends a report to the device.
    @discussion This method behaves asynchronously and will block until the
                report has been issued to the device.  It is only relevent for 
                either output or feature type reports.
    @param      device Reference to an IOHIDDevice.
    @param      reportType Type of report being sent.
    @param      reportID ID of the report being sent.  If the device supports
                multiple reports, this should also be set in the first byte of
                the report.
    @param      report The report bytes to be sent to the device.
    @param      reportLength The length of the report to be sent to the device.
    @param      timeout CFTimeInterval containing the timeout.
    @param      callback Pointer to a callback method of type 
                IOHIDReportCallback.
    @param      context Pointer to data to be passed to the callback.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceSetReportWithCallback(
                                IOHIDDeviceRef                  device,
                                IOHIDReportType                 reportType,
                                CFIndex                         reportID,
                                const uint8_t *                 report,
                                CFIndex                         reportLength,
                                CFTimeInterval                  timeout,
                                IOHIDReportCallback _Nullable   callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceGetReport
    @abstract   Obtains a report from the device.
    @discussion This method behaves synchronously and will block until the
                report has been received from the device.  This is only intended 
                for feature reports because of sporadic devicesupport for 
                polling input reports.  Please defer to using 
                IOHIDDeviceRegisterInputReportCallback for obtaining input 
                reports.
    @param      device Reference to an IOHIDDevice.
    @param      reportType Type of report being requested.
    @param      reportID ID of the report being requested.
    @param      report Pointer to preallocated buffer in which to copy inbound
                report data.
    @param      pReportLength Pointer to length of preallocated buffer.  This
                value will be modified to refect the length of the returned 
                report.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceGetReport(
                                IOHIDDeviceRef                  device,
                                IOHIDReportType                 reportType,
                                CFIndex                         reportID,
                                uint8_t *                       report,
                                CFIndex *                       pReportLength)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDDeviceGetReportWithCallback
    @abstract   Obtains a report from the device.
    @discussion This method behaves asynchronously and will block until the
                report has been received from the device.  This is only intended 
                for feature reports because of sporadic devicesupport for 
                polling input reports.  Please defer to using 
                IOHIDDeviceRegisterInputReportCallback for obtaining input 
                reports.
    @param      device Reference to an IOHIDDevice.
    @param      reportType Type of report being requested.
    @param      reportID ID of the report being requested.
    @param      report Pointer to preallocated buffer in which to copy inbound
                report data.
    @param      pReportLength Pointer to length of preallocated buffer.
    @param      pReportLength Pointer to length of preallocated buffer.  This
                value will be modified to refect the length of the returned 
                report.
    @param      callback Pointer to a callback method of type 
                IOHIDReportCallback.
    @param      context Pointer to data to be passed to the callback.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDDeviceGetReportWithCallback(
                                IOHIDDeviceRef                  device,
                                IOHIDReportType                 reportType,
                                CFIndex                         reportID,
                                uint8_t *                       report,
                                CFIndex *                       pReportLength,
                                CFTimeInterval                  timeout,
                                IOHIDReportCallback             callback,
                                void *                          context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

__END_DECLS

#endif /* _IOKIT_HID_IOHIDDEVICE_USER_H */
