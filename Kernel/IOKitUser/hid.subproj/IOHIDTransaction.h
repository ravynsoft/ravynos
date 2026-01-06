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

#ifndef _IOKIT_HID_IOHIDTRANSACTION_USER_H
#define _IOKIT_HID_IOHIDTRANSACTION_USER_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDBase.h>

/*!
	@header IOHIDTransaction
    IOHIDTransaction defines an object used to manipulate multiple parsed items 
    (IOHIDElement) contained within a Human Interface Device (HID) object.  It
    is used to minimize device communication when interacting with feature and
    output type elements that are grouped by their report IDs.  IOHIDTransaction 
    is a CFType object and as such conforms to all the conventions expected such 
    object.
    <p>
    This documentation assumes that you have a basic understanding of the material contained in <a href="http://developer.apple.com/documentation/DeviceDrivers/Conceptual/AccessingHardware/index.html"><i>Accessing Hardware From Applications</i></a>
    For definitions of I/O Kit terms used in this documentation, such as matching dictionary, family, and driver, see the overview of I/O Kit terms and concepts 
    in the "Device Access and the I/O Kit" chapter of <i>Accessing Hardware From Applications</i>.

    This documentation also assumes you have read <a href="http://developer.apple.com/documentation/DeviceDrivers/HumanInterfaceDeviceForceFeedback-date.html"><i>Human Interface Device & Force Feedback</i></a>.
    Please review documentation before using this reference.
    <p>
    All of the information described in this document is contained in the header file <font face="Courier New,Courier,Monaco">IOHIDTransaction.h</font> found at 
    <font face="Courier New,Courier,Monaco">/System/Library/Frameworks/IOKit.framework/Headers/hid/IOHIDTransaction.h</font>.
*/

__BEGIN_DECLS

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

/*!
 @enum      IOHIDTransactionOptions
 @abstract  Various options that can be supplied to IOHIDTransaction functions.
 @const     kIOHIDTransactionOptionsNone For those times when supplying 0 just isn't
            explicit enough.
 @const     kIOHIDTransactionOptionsWeakDevice specifies the transaction to not retain the
            IOHIDDeviceRef being passed in. The expectation is that transaction will only exist during
            the lifetime of the IOHIDDeviceRef object.
 */
typedef CF_OPTIONS(uint32_t, IOHIDTransactionOptions) {
    kIOHIDTransactionOptionsNone = 0x0,
    kIOHIDTransactionOptionsWeakDevice = 0x1,
};

/*! @typedef IOHIDTransactionRef
	This is the type of a reference to the IOHIDTransaction.
*/
typedef struct CF_BRIDGED_TYPE(id) __IOHIDTransaction * IOHIDTransactionRef;

/*!
	@function   IOHIDTransactionGetTypeID
	@abstract   Returns the type identifier of all IOHIDTransaction instances.
*/
CF_EXPORT
CFTypeID IOHIDTransactionGetTypeID(void)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDTransactionCreate
	@abstract   Creates an IOHIDTransaction object for the specified device.
    @discussion IOHIDTransaction objects can be used to either send or receive
                multiple element values.  As such the direction used should 
                represent they type of objects added to the transaction.
    @param      allocator Allocator to be used during creation.
    @param      device IOHIDDevice object 
    @param      direction The direction, either in or out, for the transaction.
    @param      options Reserved for future use.
    @result     Returns a new IOHIDTransactionRef.
*/
CF_EXPORT
IOHIDTransactionRef _Nullable IOHIDTransactionCreate(
                                CFAllocatorRef _Nullable        allocator,
                                IOHIDDeviceRef                  device,
                                IOHIDTransactionDirectionType   direction,
                                IOOptionBits                    options)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;


/*!
	@function   IOHIDTransactionGetDevice
	@abstract   Obtain the device associated with the transaction.
    @param      transaction IOHIDTransaction to be queried. 
    @result     Returns the a reference to the device.
*/
CF_EXPORT
IOHIDDeviceRef IOHIDTransactionGetDevice(     
                                IOHIDTransactionRef             transaction)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDTransactionGetDirection
	@abstract   Obtain the direction of the transaction.
    @param      transaction IOHIDTransaction to be queried. 
    @result     Returns the transaction direction.
*/
CF_EXPORT
IOHIDTransactionDirectionType IOHIDTransactionGetDirection(     
                                IOHIDTransactionRef             transaction)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDTransactionSetDirection
	@abstract   Sets the direction of the transaction
    @disussion  This method is useful for manipulating bi-direction (feature) 
                elements such that you can set or get element values without
                creating an additional transaction object.
    @param      transaction IOHIDTransaction object to be modified.
    @param      direction The new transaction direction.
*/
CF_EXPORT
void IOHIDTransactionSetDirection(        
                                IOHIDTransactionRef             transaction,
                                IOHIDTransactionDirectionType   direction)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*!
	@function   IOHIDTransactionAddElement
	@abstract   Adds an element to the transaction
    @disussion  To minimize device traffic it is important to add elements that
                share a common report type and report id.
    @param      transaction IOHIDTransaction object to be modified.
    @param      element Element to be added to the transaction.
*/
CF_EXPORT
void IOHIDTransactionAddElement(      
                                IOHIDTransactionRef             transaction,
                                IOHIDElementRef                 element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*!
	@function   IOHIDTransactionRemoveElement
	@abstract   Removes an element to the transaction
    @param      transaction IOHIDTransaction object to be modified.
    @param      element Element to be removed to the transaction.
*/
CF_EXPORT
void IOHIDTransactionRemoveElement(
                                IOHIDTransactionRef             transaction,
                                IOHIDElementRef                 element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*!
	@function   IOHIDTransactionContainsElement
	@abstract   Queries the transaction to determine if elemement has been added.
    @param      transaction IOHIDTransaction object to be queried.
    @param      element Element to be queried.
    @result     Returns true or false depending if element is present.
*/
CF_EXPORT
Boolean IOHIDTransactionContainsElement(
                                IOHIDTransactionRef             transaction,
                                IOHIDElementRef                 element)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDTransactionScheduleWithRunLoop
	@abstract   Schedules transaction with run loop.
    @discussion Formally associates transaction with client's run loop. 
                Scheduling this transaction with the run loop is necessary 
                before making use of any asynchronous APIs.
    @param      transaction IOHIDTransaction object to be modified.
    @param      runLoop RunLoop to be used when scheduling any asynchronous 
                activity.
    @param      runLoopMode Run loop mode to be used when scheduling any 
                asynchronous activity.
*/
CF_EXPORT
void IOHIDTransactionScheduleWithRunLoop(
                                IOHIDTransactionRef             transaction, 
                                CFRunLoopRef                    runLoop, 
                                CFStringRef                     runLoopMode)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDTransactionUnscheduleFromRunLoop
	@abstract   Unschedules transaction with run loop.
    @discussion Formally disassociates transaction with client's run loop.
    @param      transaction IOHIDTransaction object to be modified.
    @param      runLoop RunLoop to be used when scheduling any asynchronous 
                activity.
    @param      runLoopMode Run loop mode to be used when scheduling any 
                asynchronous activity.
*/
CF_EXPORT
void IOHIDTransactionUnscheduleFromRunLoop(  
                                IOHIDTransactionRef             transaction, 
                                CFRunLoopRef                    runLoop, 
                                CFStringRef                     runLoopMode)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDTransactionSetValue
	@abstract   Sets the value for a transaction element.
    @discussion The value set is pended until the transaction is committed and
                is only used if the transaction direction is 
                kIOHIDTransactionDirectionTypeOutput.  Use the 
                kIOHIDTransactionOptionDefaultOutputValue option to set the 
                default element value.
    @param      transaction IOHIDTransaction object to be modified.
    @param      element Element to be modified after a commit.
    @param      value Value to be set for the given element.
    @param      options See IOHIDTransactionOption.
*/
CF_EXPORT
void IOHIDTransactionSetValue(
                                IOHIDTransactionRef             transaction,
                                IOHIDElementRef                 element, 
                                IOHIDValueRef                   value,
                                IOOptionBits                    options)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*!
	@function   IOHIDTransactionGetValue
	@abstract   Obtains the value for a transaction element.
    @discussion If the transaction direction is 
                kIOHIDTransactionDirectionTypeInput the value represents what
                was obtained from the device from the transaction.  Otherwise, 
                if the transaction direction is 
                kIOHIDTransactionDirectionTypeOutput the value represents the 
                pending value to be sent to the device.  Use the 
                kIOHIDTransactionOptionDefaultOutputValue option to get the 
                default element value.
    @param      transaction IOHIDTransaction object to be queried.
    @param      element Element to be queried.
    @param      options See IOHIDTransactionOption.
    @result     Returns IOHIDValueRef for the given element.
*/
CF_EXPORT
IOHIDValueRef _Nullable IOHIDTransactionGetValue(
                                IOHIDTransactionRef             transaction,
                                IOHIDElementRef                 element,
                                IOOptionBits                    options)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
       
/*!
	@function   IOHIDTransactionCommit
	@abstract   Synchronously commits element transaction to the device.
    @discussion In regards to kIOHIDTransactionDirectionTypeOutput direction, 
                default element values will be used if element values are not 
                set.  If neither are set, that element will be omitted from the 
                commit. After a transaction is committed, transaction element 
                values will be cleared and default values preserved.
    @param      transaction IOHIDTransaction object to be modified.
    @result     Returns kIOReturnSuccess if successful or a kern_return_t if 
                unsuccessful.
*/
CF_EXPORT
IOReturn IOHIDTransactionCommit(
                                IOHIDTransactionRef             transaction)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;
                                
/*!
	@function   IOHIDTransactionCommitWithCallback
	@abstract   Commits element transaction to the device.
    @discussion In regards to kIOHIDTransactionDirectionTypeOutput direction, 
                default element values will be used if element values are not 
                set.  If neither are set, that element will be omitted from the 
                commit. After a transaction is committed, transaction element 
                values will be cleared and default values preserved.
                <br>
                <b>Note:</b> It is possible for elements from different reports
                to be present in a given transaction causing a commit to
                transcend multiple reports. Keep this in mind when setting a 
                appropriate timeout.
    @param      transaction IOHIDTransaction object to be modified.
    @param      timeout Timeout for issuing the transaction.
    @param      callback Callback of type IOHIDCallback to be used when 
                transaction has been completed.  If null, this method will 
                behave synchronously.
    @param      context Pointer to data to be passed to the callback.
    @result     Returns kIOReturnSuccess if successful or a kern_return_t if 
                unsuccessful.
*/
CF_EXPORT
IOReturn IOHIDTransactionCommitWithCallback(
                                IOHIDTransactionRef             transaction,
                                CFTimeInterval                  timeout, 
                                IOHIDCallback _Nullable         callback,
                                void * _Nullable                context)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;

/*! 
    @function   IOHIDTransactionClear
    @abstract   Clears element transaction values.
    @discussion In regards to kIOHIDTransactionDirectionTypeOutput direction, 
                default element values will be preserved.
    @param      transaction IOHIDTransaction object to be modified.
*/
CF_EXPORT
void IOHIDTransactionClear(
                                IOHIDTransactionRef             transaction)
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER;


CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

__END_DECLS

#endif /* _IOKIT_HID_IOHIDTRANSACTION_USER_H */
