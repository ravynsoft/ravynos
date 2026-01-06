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

#ifndef _IOKIT_HID_IOHIDMANAGER_H_
#define _IOKIT_HID_IOHIDMANAGER_H_

#include <IOKit/IOTypes.h>
#include <IOKit/IOReturn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <CoreFoundation/CoreFoundation.h>

/*!
	@header IOHIDManager
    IOHIDManager defines an Human Interface Device (HID) managment object.  
    It provides global interaction with managed HID devices such as 
    discovery/removal and receiving input events.  IOHIDManager is also a CFType 
    object and as such conforms to all the conventions expected such object.
    <p>
    This documentation assumes that you have a basic understanding of the 
    material contained in <a href="http://developer.apple.com/documentation/DeviceDrivers/Conceptual/AccessingHardware/index.html"><i>Accessing Hardware From Applications</i></a>
    For definitions of I/O Kit terms used in this documentation, such as 
    matching dictionary, family, and driver, see the overview of I/O Kit terms 
    and concepts n the "Device Access and the I/O Kit" chapter of 
    <i>Accessing Hardware From Applications</i>.
    
    This documentation also assumes you have read <a href="http://developer.apple.com/documentation/DeviceDrivers/HumanInterfaceDeviceForceFeedback-date.html"><i>Human Interface Device & Force Feedback</i></a>.
    Please review documentation before using this reference.
    <p>
    All of the information described in this document is contained in the header 
    file <font face="Courier New,Courier,Monaco">IOHIDManager.h</font> found at 
    <font face="Courier New,Courier,Monaco">/System/Library/Frameworks/IOKit.framework/Headers/hid/IOHIDManager.h</font>.
*/

__BEGIN_DECLS

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

/*!
 @enum      IOHIDManagerOptions
 @abstract  Various options that can be supplied to IOHIDManager functions.
 @const     kIOHIDManagerOptionNone For those times when supplying 0 just isn't
            explicit enough.
 @const     kIOHIDManagerOptionUsePersistentProperties This constant can be
            supplied to @link IOHIDManagerCreate @/link to create and/or use a
            persistent properties store.
 @const     kIOHIDManagerOptionDoNotLoadProperties This constant can be supplied
            to @link IOHIDManagerCreate when you wish to overwrite the
            persistent properties store without loading it first.
 @const     kIOHIDManagerOptionDoNotSaveProperties This constant can be supplied
            to @link IOHIDManagerCreate @/link when you want to use the
            persistent property store but do not want to add to it.
 @const     kIOHIDManagerOptionIndependentDevices Devices maintained by the
            manager will act independently from calls to the manager.
            This allows for devices to be scheduled on separate queues, and
            their lifetime can persist after the manager is gone.
 
            The following calls will not be propagated to the devices:
            IOHIDManagerOpen, IOHIDManagerClose, IOHIDManagerScheduleWithRunLoop,
            IOHIDManagerUnscheduleFromRunLoop, IOHIDManagerSetDispatchQueue,
            IOHIDManagerSetCancelHandler, IOHIDManagerActivate, IOHIDManagerCancel,
            IOHIDManagerRegisterInputReportCallback,
            IOHIDManagerRegisterInputReportWithTimeStampCallback,
            IOHIDManagerRegisterInputValueCallback, IOHIDManagerSetInputValueMatching,
            IOHIDManagerSetInputValueMatchingMultiple,
 
            This also means that the manager will not be able to receive input
            reports or input values, since the devices may or may not be scheduled.
 */
typedef CF_OPTIONS(uint32_t, IOHIDManagerOptions) {
    kIOHIDManagerOptionNone = 0x0,
    kIOHIDManagerOptionUsePersistentProperties = 0x1,
    kIOHIDManagerOptionDoNotLoadProperties = 0x2,
    kIOHIDManagerOptionDoNotSaveProperties = 0x4,
    kIOHIDManagerOptionIndependentDevices = 0x8,
};

/*! @typedef IOHIDManagerRef
	@abstract This is the type of a reference to the IOHIDManager.
*/
typedef struct CF_BRIDGED_TYPE(id) __IOHIDManager * IOHIDManagerRef;

/*!
	@function   IOHIDManagerGetTypeID
	@abstract   Returns the type identifier of all IOHIDManager instances.
*/
CF_EXPORT
CFTypeID IOHIDManagerGetTypeID(void) 
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDManagerCreate
	@abstract   Creates an IOHIDManager object.
    @discussion The IOHIDManager object is meant as a global management system
                for communicating with HID devices.
    @param      allocator Allocator to be used during creation.
    @param      options Supply @link kIOHIDManagerOptionUsePersistentProperties @/link to load
                properties from the default persistent property store. Otherwise supply
                @link kIOHIDManagerOptionNone @/link (or 0).                
    @result     Returns a new IOHIDManagerRef.
*/
CF_EXPORT 
IOHIDManagerRef IOHIDManagerCreate(     
                                CFAllocatorRef _Nullable        allocator,
                                IOOptionBits                    options)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*!
	@function   IOHIDManagerOpen
	@abstract   Opens the IOHIDManager.
    @discussion This will open both current and future devices that are 
                enumerated. To establish an exclusive link use the 
                kIOHIDOptionsTypeSeizeDevice option. 
    @param      manager Reference to an IOHIDManager.
    @param      options Option bits to be sent down to the manager and device.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDManagerOpen(
                                IOHIDManagerRef                 manager,
                                IOOptionBits                    options)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*!
	@function   IOHIDManagerClose
	@abstract   Closes the IOHIDManager.
    @discussion This will also close all devices that are currently enumerated.
    @param      manager Reference to an IOHIDManager.
    @param      options Option bits to be sent down to the manager and device.
    @result     Returns kIOReturnSuccess if successful.
*/
CF_EXPORT
IOReturn IOHIDManagerClose(
                                IOHIDManagerRef                 manager,
                                IOOptionBits                    options)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDManagerGetProperty
	@abstract   Obtains a property of an IOHIDManager.
    @discussion Property keys are prefixed by kIOHIDDevice and declared in 
                <IOKit/hid/IOHIDKeys.h>.
    @param      manager Reference to an IOHIDManager.
    @param      key CFStringRef containing key to be used when querying the 
                manager.
    @result     Returns CFTypeRef containing the property.
*/
CF_EXPORT
CFTypeRef _Nullable IOHIDManagerGetProperty(
                                IOHIDManagerRef                 manager,
                                CFStringRef                     key)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*!
	@function   IOHIDManagerSetProperty
	@abstract   Sets a property for an IOHIDManager.
    @discussion Property keys are prefixed by kIOHIDDevice and kIOHIDManager and
                declared in <IOKit/hid/IOHIDKeys.h>. This method will propagate 
                any relevent properties to current and future devices that are 
                enumerated.
    @param      manager Reference to an IOHIDManager.
    @param      key CFStringRef containing key to be used when modifiying the 
                device property.
    @param      value CFTypeRef containing the property value to be set.
    @result     Returns TRUE if successful.
*/
CF_EXPORT
Boolean IOHIDManagerSetProperty(
                                IOHIDManagerRef                 manager,
                                CFStringRef                     key,
                                CFTypeRef                       value)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                        
/*! @function   IOHIDManagerScheduleWithRunLoop
    @abstract   Schedules HID manager with run loop.
    @discussion Formally associates manager with client's run loop. Scheduling
                this device with the run loop is necessary before making use of
                any asynchronous APIs.  This will propagate to current and 
                future devices that are enumerated.
    @param      manager Reference to an IOHIDManager.
    @param      runLoop RunLoop to be used when scheduling any asynchronous 
                activity.
    @param      runLoopMode Run loop mode to be used when scheduling any 
                asynchronous activity.
*/
CF_EXPORT
void IOHIDManagerScheduleWithRunLoop(
                                IOHIDManagerRef                 manager,
                                CFRunLoopRef                    runLoop, 
                                CFStringRef                     runLoopMode)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDManagerUnscheduleFromRunLoop
    @abstract   Unschedules HID manager with run loop.
    @discussion Formally disassociates device with client's run loop. This will 
                propagate to current devices that are enumerated.
    @param      manager Reference to an IOHIDManager.
    @param      runLoop RunLoop to be used when unscheduling any asynchronous 
                activity.
    @param      runLoopMode Run loop mode to be used when unscheduling any 
                asynchronous activity.
*/
CF_EXPORT
void IOHIDManagerUnscheduleFromRunLoop(
                                IOHIDManagerRef                 manager,
                                CFRunLoopRef                    runLoop, 
                                CFStringRef                     runLoopMode)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
 * @function IOHIDManagerSetDispatchQueue
 *
 * @abstract
 * Sets the dispatch queue to be associated with the IOHIDManager.
 * This is necessary in order to receive asynchronous events from the kernel.
 *
 * @discussion
 * An IOHIDManager should not be associated with both a runloop and
 * dispatch queue. A call to IOHIDManagerSetDispatchQueue should only be made once.
 *
 * If a dispatch queue is set but never used, a call to IOHIDManagerCancel followed
 * by IOHIDManagerActivate should be performed in that order.
 *
 * After a dispatch queue is set, the IOHIDManager must make a call to activate
 * via IOHIDManagerActivate and cancel via IOHIDManagerCancel. All calls to "Register"
 * functions should be done before activation and not after cancellation.
 *
 * @param manager
 * Reference to an IOHIDManager
 *
 * @param queue
 * The dispatch queue to which the event handler block will be submitted.
 */
CF_EXPORT
void IOHIDManagerSetDispatchQueue(
                                IOHIDManagerRef                 manager,
                                dispatch_queue_t                queue)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDManagerSetCancelHandler
 *
 * @abstract
 * Sets a cancellation handler for the dispatch queue associated with
 * IOHIDManagerSetDispatchQueue.
 *
 * @discussion
 * The cancellation handler (if specified) will be will be submitted to the
 * manager's dispatch queue in response to a call to IOHIDManagerCancel after
 * all the events have been handled.
 *
 * IOHIDManagerSetCancelHandler should not be used when scheduling with
 * a run loop.
 *
 * The IOHIDManagerRef should only be released after the manager has been
 * cancelled, and the cancel handler has been called. This is to ensure all
 * asynchronous objects are released. For example:
 *
 *     dispatch_block_t cancelHandler = dispatch_block_create(0, ^{
 *         CFRelease(manager);
 *     });
 *     IOHIDManagerSetCancelHandler(manager, cancelHandler);
 *     IOHIDManagerActivate(manager);
 *     IOHIDManageCancel(manager);
 *
 * @param manager
 * Reference to an IOHIDManager.
 *
 * @param handler
 * The cancellation handler block to be associated with the dispatch queue.
 */
CF_EXPORT
void IOHIDManagerSetCancelHandler(
                                IOHIDManagerRef                 manager,
                                dispatch_block_t                handler)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDManagerActivate
 *
 * @abstract
 * Activates the IOHIDManager object.
 *
 * @discussion
 * An IOHIDManager object associated with a dispatch queue is created
 * in an inactive state. The object must be activated in order to
 * receive asynchronous events from the kernel.
 *
 * A dispatch queue must be set via IOHIDManagerSetDispatchQueue before
 * activation.
 *
 * An activated manager must be cancelled via IOHIDManagerCancel. All calls
 * to "Register" functions should be done before activation
 * and not after cancellation.
 *
 * Calling IOHIDManagerActivate on an active IOHIDManager has no effect.
 *
 * @param manager
 * Reference to an IOHIDManager
 */
CF_EXPORT
void IOHIDManagerActivate(      IOHIDManagerRef                 manager)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDManagerCancel
 *
 * @abstract
 * Cancels the IOHIDManager preventing any further invocation
 * of its event handler block.
 *
 * @discussion
 * Cancelling prevents any further invocation of the event handler block for
 * the specified dispatch queue, but does not interrupt an event handler
 * block that is already in progress.
 *
 * Explicit cancellation of the IOHIDManager is required, no implicit
 * cancellation takes place.
 *
 * Calling IOHIDManagerCancel on an already cancelled queue has no effect.
 *
 * The IOHIDManagerRef should only be released after the manager has been
 * cancelled, and the cancel handler has been called. This is to ensure all
 * asynchronous objects are released. For example:
 *
 *     dispatch_block_t cancelHandler = dispatch_block_create(0, ^{
 *         CFRelease(manager);
 *     });
 *     IOHIDManagerSetCancelHandler(manager, cancelHandler);
 *     IOHIDManagerActivate(manager);
 *     IOHIDManageCancel(manager);
 *
 * @param manager
 * Reference to an IOHIDManager
 */
CF_EXPORT
void IOHIDManagerCancel(        IOHIDManagerRef                 manager)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);
                                
/*! @function   IOHIDManagerSetDeviceMatching
    @abstract   Sets matching criteria for device enumeration.
    @discussion Matching keys are prefixed by kIOHIDDevice and declared in 
                <IOKit/hid/IOHIDKeys.h>.  Passing a NULL dictionary will result
                in all devices being enumerated. Any subsequent calls will cause
                the hid manager to release previously enumerated devices and 
                restart the enuerate process using the revised criteria.  If 
                interested in multiple, specific device classes, please defer to
                using IOHIDManagerSetDeviceMatchingMultiple.
                If a dispatch queue is set, this call must occur before activation.
    @param      manager Reference to an IOHIDManager.
    @param      matching CFDictionaryRef containg device matching criteria.
*/
CF_EXPORT
void IOHIDManagerSetDeviceMatching(
                                IOHIDManagerRef                 manager,
                                CFDictionaryRef _Nullable       matching)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*! @function   IOHIDManagerSetDeviceMatchingMultiple
    @abstract   Sets multiple matching criteria for device enumeration.
    @discussion Matching keys are prefixed by kIOHIDDevice and declared in 
                <IOKit/hid/IOHIDKeys.h>.  This method is useful if interested 
                in multiple, specific device classes.
                If a dispatch queue is set, this call must occur before activation.
    @param      manager Reference to an IOHIDManager.
    @param      multiple CFArrayRef containing multiple CFDictionaryRef objects
                containg device matching criteria.
*/
CF_EXPORT
void IOHIDManagerSetDeviceMatchingMultiple(
                                IOHIDManagerRef                 manager,
                                CFArrayRef _Nullable            multiple)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*! @function   IOHIDManagerCopyDevices
    @abstract   Obtains currently enumerated devices.
    @param      manager Reference to an IOHIDManager.
    @result     CFSetRef containing IOHIDDeviceRefs.
*/
CF_EXPORT
CFSetRef _Nullable IOHIDManagerCopyDevices(
                                IOHIDManagerRef                 manager)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDManagerRegisterDeviceMatchingCallback
    @abstract   Registers a callback to be used a device is enumerated.
    @discussion Only device matching the set criteria will be enumerated.
                If a dispatch queue is set, this call must occur before activation.
                Devices provided in the callback will be scheduled with the same
                runloop/dispatch queue as the IOHIDManagerRef, and should not be
                rescheduled.
    @param      manager Reference to an IOHIDManagerRef.
    @param      callback Pointer to a callback method of type 
                IOHIDDeviceCallback.
    @param      context Pointer to data to be passed to the callback.
*/
CF_EXPORT
void IOHIDManagerRegisterDeviceMatchingCallback(
                                IOHIDManagerRef                 manager,
                                IOHIDDeviceCallback _Nullable   callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDManagerRegisterDeviceRemovalCallback
    @abstract   Registers a callback to be used when any enumerated device is 
                removed.
    @discussion In most cases this occurs when a device is unplugged.
                If a dispatch queue is set, this call must occur before activation.
    @param      manager Reference to an IOHIDManagerRef.
    @param      callback Pointer to a callback method of type 
                IOHIDDeviceCallback.
    @param      context Pointer to data to be passed to the callback.
*/
CF_EXPORT
void IOHIDManagerRegisterDeviceRemovalCallback(
                                IOHIDManagerRef                 manager,
                                IOHIDDeviceCallback _Nullable   callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDManagerRegisterInputReportCallback
    @abstract   Registers a callback to be used when an input report is issued by 
                any enumerated device.
    @discussion An input report is an interrupt driver report issued by a device.
                If a dispatch queue is set, this call must occur before activation.
    @param      manager Reference to an IOHIDManagerRef.
    @param      callback Pointer to a callback method of type IOHIDReportCallback.
    @param      context Pointer to data to be passed to the callback.
*/
CF_EXPORT
void IOHIDManagerRegisterInputReportCallback( 
                                IOHIDManagerRef                 manager,
                                IOHIDReportCallback _Nullable   callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDManagerRegisterInputReportWithTimeStampCallback
    @abstract   Registers a callback to be used when an input report is issued by
                any enumerated device.
    @discussion An input report is an interrupt driver report issued by a device.
                If a dispatch queue is set, this call must occur before activation.
    @param      manager Reference to an IOHIDManagerRef.
    @param      callback Pointer to a callback method of type
                IOHIDReportWithTimeStampCallback.
    @param      context Pointer to data to be passed to the callback.
 */
void IOHIDManagerRegisterInputReportWithTimeStampCallback(
                                IOHIDManagerRef                    manager,
                                IOHIDReportWithTimeStampCallback   callback,
                                void * _Nullable                   context)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);
                                                                    
/*! @function   IOHIDManagerRegisterInputValueCallback
    @abstract   Registers a callback to be used when an input value is issued by 
                any enumerated device.
    @discussion An input element refers to any element of type 
                kIOHIDElementTypeInput and is usually issued by interrupt driven
                reports.
                If a dispatch queue is set, this call must occur before activation.
    @param      manager Reference to an IOHIDManagerRef.
    @param      callback Pointer to a callback method of type IOHIDValueCallback.
    @param      context Pointer to data to be passed to the callback.
*/
CF_EXPORT
void IOHIDManagerRegisterInputValueCallback( 
                                IOHIDManagerRef                 manager,
                                IOHIDValueCallback _Nullable    callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDManagerSetInputValueMatching
    @abstract   Sets matching criteria for input values received via 
                IOHIDManagerRegisterInputValueCallback.
    @discussion Matching keys are prefixed by kIOHIDElement and declared in 
                <IOKit/hid/IOHIDKeys.h>.  Passing a NULL dictionary will result
                in all devices being enumerated. Any subsequent calls will cause
                the hid manager to release previously matched input elements and 
                restart the matching process using the revised criteria.  If 
                interested in multiple, specific device elements, please defer to
                using IOHIDManagerSetInputValueMatchingMultiple.
                If a dispatch queue is set, this call must occur before activation.
    @param      manager Reference to an IOHIDManager.
    @param      matching CFDictionaryRef containg device matching criteria.
*/
CF_EXPORT
void IOHIDManagerSetInputValueMatching(
                                IOHIDManagerRef                 manager,
                                CFDictionaryRef _Nullable       matching)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! @function   IOHIDManagerSetInputValueMatchingMultiple
    @abstract   Sets multiple matching criteria for input values received via 
                IOHIDManagerRegisterInputValueCallback.
    @discussion Matching keys are prefixed by kIOHIDElement and declared in 
                <IOKit/hid/IOHIDKeys.h>.  This method is useful if interested 
                in multiple, specific elements.
                If a dispatch queue is set, this call must occur before activation.
    @param      manager Reference to an IOHIDManager.
    @param      multiple CFArrayRef containing multiple CFDictionaryRef objects
                containing input element matching criteria.
*/

CF_EXPORT
void IOHIDManagerSetInputValueMatchingMultiple(
                                               IOHIDManagerRef                 manager,
                                               CFArrayRef _Nullable            multiple)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
 @abstract   Used to write out the current properties to a specific domain.
 @discussion Using this function will cause the persistent properties to be saved out
 replacing any properties that already existed in the specified domain. All arguments 
 must be non-NULL except options.
 @param     manager Reference to an IOHIDManager.
 @param     applicationID Reference to a CFPreferences applicationID.
 @param     userName Reference to a CFPreferences userName.
 @param     hostName Reference to a CFPreferences hostName.
 @param     options Reserved for future use.
 */
CF_EXPORT
void IOHIDManagerSaveToPropertyDomain(IOHIDManagerRef                 manager,
                                      CFStringRef                     applicationID,
                                      CFStringRef                     userName,
                                      CFStringRef                     hostName,
                                      IOOptionBits                    options)
AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER;


CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

__END_DECLS

#endif /* _IOKIT_HID_IOHIDMANAGER_H_ */

