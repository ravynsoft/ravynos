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

#ifndef _IOKIT_HID_IOHIDQUEUE_USER_H
#define _IOKIT_HID_IOHIDQUEUE_USER_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDBase.h>

/*!
	@header IOHIDQueue
    IOHIDQueue defines an object used to queue values from input parsed items 
    (IOHIDElement) contained within a Human Interface Device (HID) object.  This
    object is useful when you need to keep track of all values of an input 
    element, rather than just the most recent one.  IOHIDQueue is a CFType 
    object and as such conforms to all the conventions expected such object.
    <p>
    IOHIDQueue should be considered optional and is only useful for working with
    complex input elements.  These elements include those whose length are 
    greater than sizeof(CFIndex) or elements that are duplicate items. 
    Whenever possible please defer to using 
    IOHIDManagerRegisterInputValueCallback or 
    IOHIDDeviceRegisterInputValueCallback.
    <p>
    <b>Note:</b>Absolute element values (based on a fixed origin) will only be 
    placed on a queue if there is a change in value. 
    <p>
    This documentation assumes that you have a basic understanding of the material contained in <a href="http://developer.apple.com/documentation/DeviceDrivers/Conceptual/AccessingHardware/index.html"><i>Accessing Hardware From Applications</i></a>
    For definitions of I/O Kit terms used in this documentation, such as matching dictionary, family, and driver, see the overview of I/O Kit terms and concepts 
    in the "Device Access and the I/O Kit" chapter of <i>Accessing Hardware From Applications</i>.

    This documentation also assumes you have read <a href="http://developer.apple.com/documentation/DeviceDrivers/HumanInterfaceDeviceForceFeedback-date.html"><i>Human Interface Device & Force Feedback</i></a>.
    Please review documentation before using this reference.
    <p>
    All of the information described in this document is contained in the header file <font face="Courier New,Courier,Monaco">IOHIDQueue.h</font> found at 
    <font face="Courier New,Courier,Monaco">/System/Library/Frameworks/IOKit.framework/Headers/hid/IOHIDQueue.h</font>.
*/

__BEGIN_DECLS

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

/*! @typedef IOHIDQueueRef
	This is the type of a reference to the IOHIDQueue.
*/
typedef struct CF_BRIDGED_TYPE(id) __IOHIDQueue * IOHIDQueueRef;

/*!
	@function   IOHIDQueueGetTypeID
	@abstract   Returns the type identifier of all IOHIDQueue instances.
*/
CF_EXPORT
CFTypeID IOHIDQueueGetTypeID(void)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDQueueCreate
	@abstract   Creates an IOHIDQueue object for the specified device.
    @discussion Take care in specifying an appropriate depth to prevent dropping
                events.
    @param      allocator Allocator to be used during creation.
    @param      device IOHIDDevice object 
    @param      depth The number of values that can be handled by the queue.
    @param      options Reserved for future use.
    @result     Returns a new IOHIDQueueRef.
*/
CF_EXPORT
IOHIDQueueRef _Nullable IOHIDQueueCreate(
                                CFAllocatorRef _Nullable        allocator,
                                IOHIDDeviceRef                  device,
                                CFIndex                         depth,
                                IOOptionBits                    options)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDQueueGetDevice
	@abstract   Obtain the device associated with the queue.
    @param      queue IOHIDQueue to be queried. 
    @result     Returns the a reference to the device.
*/
CF_EXPORT
IOHIDDeviceRef IOHIDQueueGetDevice(     
                                IOHIDQueueRef                   queue)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;


/*!
	@function   IOHIDQueueGetDepth
	@abstract   Obtain the depth of the queue.
    @param      queue IOHIDQueue to be queried. 
    @result     Returns the queue depth.
*/
CF_EXPORT
CFIndex IOHIDQueueGetDepth(     
                                IOHIDQueueRef                   queue)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDQueueSetDepth
	@abstract   Sets the depth of the queue.
    @discussion Set the appropriate depth value based on the number of elements
                contained in a queue.
    @param      queue IOHIDQueue object to be modified.
    @param      depth The new queue depth.
*/
CF_EXPORT
void IOHIDQueueSetDepth(        
                                IOHIDQueueRef                   queue,
                                CFIndex                         depth)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*!
	@function   IOHIDQueueAddElement
	@abstract   Adds an element to the queue
    @param      queue IOHIDQueue object to be modified.
    @param      element Element to be added to the queue.
*/
CF_EXPORT
void IOHIDQueueAddElement(      
                                IOHIDQueueRef                   queue,
                                IOHIDElementRef                 element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*!
	@function   IOHIDQueueRemoveElement
	@abstract   Removes an element from the queue
    @param      queue IOHIDQueue object to be modified.
    @param      element Element to be removed from the queue.
*/
CF_EXPORT
void IOHIDQueueRemoveElement(
                                IOHIDQueueRef                   queue,
                                IOHIDElementRef                 element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*!
	@function   IOHIDQueueContainsElement
	@abstract   Queries the queue to determine if elemement has been added.
    @param      queue IOHIDQueue object to be queried.
    @param      element Element to be queried.
    @result     Returns true or false depending if element is present.
*/
CF_EXPORT
Boolean IOHIDQueueContainsElement(
                                IOHIDQueueRef                   queue,
                                IOHIDElementRef                 element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*! 
    @function   IOHIDQueueStart
    @abstract   Starts element value delivery to the queue.
    @discussion When a dispatch queue is assocaited with the IOHIDQueue
                via IOHIDQueueSetDispatchQueue, the queue does not need
                to be explicity started, this will be done during activation
                when IOHIDQueueActivate is called.
    @param      queue IOHIDQueue object to be started.
*/
CF_EXPORT
void IOHIDQueueStart(           IOHIDQueueRef                   queue)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! 
    @function   IOHIDQueueStop
    @abstract   Stops element value delivery to the queue.
    @discussion When a dispatch queue is assocaited with the IOHIDQueue
                via IOHIDQueueSetDispatchQueue, the queue does not need
                to be explicity stopped, this will be done during cancellation
                when IOHIDQueueCancel is called.
    @param      queue IOHIDQueue object to be stopped.
*/
CF_EXPORT
void IOHIDQueueStop(            IOHIDQueueRef                   queue)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDQueueScheduleWithRunLoop
	@abstract   Schedules queue with run loop.
    @discussion Formally associates queue with client's run loop. Scheduling
                this queue with the run loop is necessary before making 
                use of any asynchronous APIs.
    @param      queue IOHIDQueue object to be modified.
    @param      runLoop RunLoop to be used when scheduling any asynchronous 
                activity.
    @param      runLoopMode Run loop mode to be used when scheduling any 
                asynchronous activity.
*/
CF_EXPORT
void IOHIDQueueScheduleWithRunLoop(
                                IOHIDQueueRef                   queue, 
                                CFRunLoopRef                    runLoop, 
                                CFStringRef                     runLoopMode)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDQueueUnscheduleFromRunLoop
	@abstract   Unschedules queue with run loop.
    @discussion Formally disassociates queue with client's run loop.
    @param      queue IOHIDQueue object to be modified.
    @param      runLoop RunLoop to be used when scheduling any asynchronous 
                activity.
    @param      runLoopMode Run loop mode to be used when scheduling any 
                asynchronous activity.
*/
CF_EXPORT
void IOHIDQueueUnscheduleFromRunLoop(  
                                IOHIDQueueRef                   queue, 
                                CFRunLoopRef                    runLoop, 
                                CFStringRef                     runLoopMode)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
 * @function IOHIDQueueSetDispatchQueue
 *
 * @abstract
 * Sets the dispatch queue to be associated with the IOHIDQueue.
 * This is necessary in order to receive asynchronous events from the kernel.
 *
 * @discussion
 * An IOHIDQueue should not be associated with both a runloop and
 * dispatch queue. A call to IOHIDQueueSetDispatchQueue should only be made once.
 *
 * If a dispatch queue is set but never used, a call to IOHIDQueueCancel followed
 * by IOHIDQueueActivate should be performed in that order.
 *
 * After a dispatch queue is set, the IOHIDQueue must make a call to activate
 * via IOHIDQueueActivate and cancel via IOHIDQueueCancel. All calls to "Register"
 * functions should be done before activation and not after cancellation.
 *
 * @param queue
 * Reference to an IOHIDQueue
 *
 * @param dispatchQueue
 * The dispatch queue to which the event handler block will be submitted.
 */
CF_EXPORT
void IOHIDQueueSetDispatchQueue(
                                IOHIDQueueRef                   queue,
                                dispatch_queue_t                dispatchQueue)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDQueueSetCancelHandler
 *
 * @abstract
 * Sets a cancellation handler for the dispatch queue associated with
 * IOHIDQueueSetDispatchQueue.
 *
 * @discussion
 * The cancellation handler (if specified) will be will be submitted to the
 * queue's dispatch queue in response to a call to IOHIDQueueCancel after all
 * the events have been handled.
 *
 * IOHIDQueueSetCancelHandler should not be used when scheduling with
 * a run loop.
 *
 * The IOHIDQueueRef should only be released after the queue has been
 * cancelled, and the cancel handler has been called. This is to ensure all
 * asynchronous objects are released. For example:
 *
 *     dispatch_block_t cancelHandler = dispatch_block_create(0, ^{
 *         CFRelease(queue);
 *     });
 *     IOHIDQueueSetCancelHandler(queue, cancelHandler);
 *     IOHIDQueueActivate(queue);
 *     IOHIDQueueCancel(queue);
 *
 * @param queue
 * Reference to an IOHIDQueue.
 *
 * @param handler
 * The cancellation handler block to be associated with the dispatch queue.
 */
CF_EXPORT
void IOHIDQueueSetCancelHandler(
                                IOHIDQueueRef                   queue,
                                dispatch_block_t                handler)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDQueueActivate
 *
 * @abstract
 * Activates the IOHIDQueue object.
 *
 * @discussion
 * An IOHIDQueue object associated with a dispatch queue is created
 * in an inactive state. The object must be activated in order to
 * receive asynchronous events from the kernel.
 *
 * A dispatch queue must be set via IOHIDQueueSetDispatchQueue before
 * activation.
 *
 * An activated queue must be cancelled via IOHIDQueueCancel. All calls
 * to "Register" functions should be done before activation
 * and not after cancellation.
 *
 * Calling IOHIDQueueActivate on an active IOHIDQueue has no effect.
 *
 * @param queue
 * Reference to an IOHIDQueue
 */
CF_EXPORT
void IOHIDQueueActivate(        IOHIDQueueRef                   queue)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

/*!
 * @function IOHIDQueueCancel
 *
 * @abstract
 * Cancels the IOHIDQueue preventing any further invocation
 * of its event handler block.
 *
 * @discussion
 * Cancelling prevents any further invocation of the event handler block for
 * the specified dispatch queue, but does not interrupt an event handler
 * block that is already in progress.
 *
 * Explicit cancellation of the IOHIDQueue is required, no implicit
 * cancellation takes place.
 *
 * Calling IOHIDQueueCancel on an already cancelled queue has no effect.
 *
 * The IOHIDQueueRef should only be released after the queue has been
 * cancelled, and the cancel handler has been called. This is to ensure all
 * asynchronous objects are released. For example:
 *
 *     dispatch_block_t cancelHandler = dispatch_block_create(0, ^{
 *         CFRelease(queue);
 *     });
 *     IOHIDQueueSetCancelHandler(queue, cancelHandler);
 *     IOHIDQueueActivate(queue);
 *     IOHIDQueueCancel(queue);
 *
 * @param queue
 * Reference to an IOHIDQueue
 */
CF_EXPORT
void IOHIDQueueCancel(          IOHIDQueueRef                   queue)
__OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);
                                
/*!
	@function   IOHIDQueueRegisterValueAvailableCallback
	@abstract   Sets callback to be used when the queue transitions to non-empty.
    @discussion In order to make use of asynchronous behavior, the queue needs
                to be scheduled with the run loop or dispatch queue.
                If a dispatch queue is set, this call must occur before activation.
    @param      queue IOHIDQueue object to be modified.
    @param      callback Callback of type IOHIDCallback to be used when data is 
                placed on the queue.
    @param      context Pointer to data to be passed to the callback.
*/
CF_EXPORT
void IOHIDQueueRegisterValueAvailableCallback(
                                IOHIDQueueRef                   queue,
                                IOHIDCallback                   callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! 
    @function   IOHIDQueueCopyNextValue
    @abstract   Dequeues a retained copy of an element value from the head of an 
                IOHIDQueue.
    @discussion Because the value is a retained copy, it is up to the caller to 
                release the value using CFRelease. Use with setValueCallback to 
                avoid polling the queue for data.
    @param      queue IOHIDQueue object to be queried.
    @result     Returns valid IOHIDValueRef if data is available.
*/
CF_EXPORT
IOHIDValueRef _Nullable IOHIDQueueCopyNextValue(
                                IOHIDQueueRef                   queue)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! 
    @function   IOHIDQueueCopyNextValueWithTimeout
    @abstract   Dequeues a retained copy of an element value from the head of an 
                IOHIDQueue.  This method will block until either a value is
                available or it times out.
    @discussion Because the value is a retained copy, it is up to the caller to 
                release the value using CFRelease. Use with setValueCallback to 
                avoid polling the queue for data.
    @param      queue IOHIDQueue object to be queried.
    @param      timeout Timeout before aborting an attempt to dequeue a value 
                from the head of a queue.
    @result     Returns valid IOHIDValueRef if data is available.
*/
CF_EXPORT
IOHIDValueRef _Nullable IOHIDQueueCopyNextValueWithTimeout(
                                IOHIDQueueRef                   queue,
                                CFTimeInterval                  timeout)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

__END_DECLS

#endif /* _IOKIT_HID_IOHIDQUEUE_USER_H */
