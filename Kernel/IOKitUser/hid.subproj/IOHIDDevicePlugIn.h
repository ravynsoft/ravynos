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

#ifndef _IOKIT_HID_IOHIDDEVICEPLUGIN_H
#define _IOKIT_HID_IOHIDDEVICEPLUGIN_H


#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#if COREFOUNDATION_CFPLUGINCOM_SEPARATE
#include <CoreFoundation/CFPlugInCOM.h>
#endif

#include <IOKit/IOTypes.h>
#include <IOKit/IOReturn.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDBase.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDLibObsolete.h>

__BEGIN_DECLS
/*! @header IOHIDDevicePlugIn
    This documentation describes the details of the programming interface for accessing Human Interface Devices and
    interfaces from code running in user space.  It is intended that user mode HID drivers properly inplement all
    interfaces described here in order to be visible via the HID Manager.
    <p>
    This documentation assumes that you have a basic understanding
    of the material contained in <a href="http://developer.apple.com/documentation/DeviceDrivers/Conceptual/AccessingHardware/index.html"><i>Accessing Hardware From Applications</i></a>
    For definitions of I/O Kit terms used in this documentation, such as matching dictionary, family, and driver, see the overview of I/O Kit terms and concepts 
    in the "Device Access and the I/O Kit" chapter of <i>Accessing Hardware From Applications</i>.
    
    This documentation also assumes you have read <a href="http://developer.apple.com/documentation/DeviceDrivers/HumanInterfaceDeviceForceFeedback-date.html"><i>Human Interface Device & Force Feedback</i></a>.
    Please review documentation before using this reference.
    <p>
    All of the information described in this document is contained in the header file <font face="Courier New,Courier,Monaco">IOHIDLib.h</font> found at 
    <font face="Courier New,Courier,Monaco">/System/Library/Frameworks/IOKit.framework/Headers/hid/IOHIDDevicePlugIn.h</font>.
*/


/* 13AA9C44-6F1B-11D4-907C-0005028F18D5 */
/*! @defined kIOHIDDeviceFactoryID
    @discussion This UUID constant is used internally by the system, and 
    should not have to be used by any driver code to access the device interfaces.
*/
#define kIOHIDDeviceFactoryID CFUUIDGetConstantUUIDWithBytes(NULL,	\
    0x13, 0xAA, 0x9C, 0x44, 0x6F, 0x1B, 0x11, 0xD4,			\
    0x90, 0x7C, 0x00, 0x05, 0x02, 0x8F, 0x18, 0xD5)

/* 7DDEECA8-A7B4-11DA-8A0E-0014519758EF */
/*! @defined kIOHIDDeviceTypeID
    @discussion This UUID constant is used to obtain a device interface corresponding to 
    an io_service_t corresponding to an IOHIDDevice in the kernel. Once you have 
    obtained the IOCFPlugInInterface for the service, you must use the QueryInterface 
    function to obtain the device interface for the user client itself.

    Example:
    <pre>
    @textblock
    io_service_t            hidDeviceRef;   // obtained earlier
        
    IOCFPlugInInterface     **iodev;        // fetching this now
        
    SInt32                  score;          // not used
    IOReturn                err;
        
    err = IOCreatePlugInInterfaceForService(hidDeviceRef,
                                    kIOHIDDeviceTypeID,
                                    kIOCFPlugInInterfaceID,
                                    &iodev,
                                    &score);
    @/textblock
    </pre>
*/
#define kIOHIDDeviceTypeID CFUUIDGetConstantUUIDWithBytes(NULL, \
    0x7d, 0xde, 0xec, 0xa8, 0xa7, 0xb4, 0x11, 0xda, \
    0x8a, 0x0e, 0x00, 0x14, 0x51, 0x97, 0x58, 0xef)

/* 474BDC8E-9F4A-11DA-B366-000D936D06D2 */
/*! @defined kIOHIDDeviceDeviceInterfaceID
    @discussion This UUID constant is used to obtain a device interface corresponding 
    to an IOHIDDevice service in the kernel. The type of this device interface is IOHIDDeviceDeviceInterface. 
    This device interface is obtained after the IOCFPlugInInterface for the service itself has been obtained.
    
    <b>Note:</b> Please note that subsequent calls to QueryInterface with the UUID 
    kIOHIDDeviceDeviceInterfaceID, will return a retained instance of an existing IOHIDDeviceDeviceInterface.

    Example:
    <pre>
    @textblock
    IOCFPluginInterface **          iodev;  // obtained earlier
    
    IOHIDDeviceDeviceInterface **   dev;    // fetching this now
    IOReturn                        err;
    
    err = (*iodev)->QueryInterface(iodev,
                                    CFUUIDGetUUIDBytes(kIOHIDDeviceDeviceInterfaceID),
                                    (LPVoid)&dev);
    @/textblock
    </pre>
*/
#define kIOHIDDeviceDeviceInterfaceID CFUUIDGetConstantUUIDWithBytes(NULL, \
    0x47, 0x4b, 0xdc, 0x8e, 0x9f, 0x4a, 0x11, 0xda, \
    0xb3, 0x66, 0x00, 0x0d, 0x93, 0x6d, 0x06, 0xd2 )
    
/* B473256C-6A72-4E04-B694-C4001D202020 */
/*! @defined kIOHIDDeviceDeviceInterfaceID2
    @discussion This UUID constant is used to obtain a device interface corresponding
    to an IOHIDDevice service in the kernel, but only for timestamped report callbacks.
    The type of this device interface is IOHIDDeviceTimeStampedDeviceInterface.
    This device interface is obtained after the IOCFPlugInInterface for the service itself has been obtained.

    <b>Note:</b> Please note that subsequent calls to QueryInterface with the UUID 
    kIOHIDDeviceDeviceInterfaceID2, will return a retained instance of an existing IOHIDDeviceTimeStampedDeviceInterface.

    Example:
    <pre>
    @textblock
    IOCFPluginInterface                     **iodev;  // obtained earlier

    IOHIDDeviceTimeStampedDeviceInterface   **devTime;// fetching this now
    IOReturn                                err;

    err = (*iodev)->QueryInterface(iodev,
                                   CFUUIDGetUUIDBytes(kIOHIDDeviceDeviceInterfaceID2),
                                   (LPVoid)&devTime);
    @/textblock
    </pre>
*/
#define kIOHIDDeviceDeviceInterfaceID2 CFUUIDGetConstantUUIDWithBytes(NULL, \
   0xB4, 0x73, 0x25, 0x6C, 0x6A, 0x72, 0x4E, 0x04, \
   0xB6, 0x94, 0xC4, 0x00, 0x1D, 0x20, 0x20, 0x20)

/* 2EC78BDB-9F4E-11DA-B65C-000D936D06D2 */
/*! @defined kIOHIDDeviceQueueInterfaceID
    @discussion This UUID constant is used to obtain a queue interface corresponding 
    to an IOHIDDevice service in the kernel. The type of this queue interface 
    is IOHIDDeviceQueueInterface. This device interface is obtained after the device interface 
    for the service itself has been obtained.
    
    <b>Note:</b> Please note that subsequent calls to QueryInterface with the UUID 
    kIOHIDDeviceQueueInterfaceID, will return a retained instance of a new IOHIDDeviceQueueInterface.
    
    Example:
    <pre>
    @textblock
    IOCFPluginInterface **          iodev; 	// obtained earlier
    
    IOHIDDeviceQueueInterface **    intf;   // fetching this now
    IOReturn                        err;
    
    err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOHIDDeviceQueueInterfaceID),
                                (LPVoid)&intf);
    @/textblock
    </pre>
*/
#define kIOHIDDeviceQueueInterfaceID CFUUIDGetConstantUUIDWithBytes(NULL, 	\
    0x2e, 0xc7, 0x8b, 0xdb, 0x9f, 0x4e, 0x11, 0xda, \
    0xb6, 0x5c, 0x00, 0x0d, 0x93, 0x6d, 0x06, 0xd2)

/* 1F2E78FA-9FFA-11DA-90B4-000D936D06D2 */
/*! @defined kIOHIDDeviceTransactionInterfaceID
    @discussion This UUID constant is used to obtain a transaction interface corresponding 
    to an IOHIDDevice service in the kernel. The type of this queue interface 
    is IOHIDDeviceTransactionInterface. This device interface is obtained after the device interface 
    for the service itself has been obtained.
    
    <b>Note:</b> Please note that subsequent calls to QueryInterface with the UUID 
    kIOHIDDeviceTransactionInterfaceID, will return a retained instance of a new IOHIDDeviceTransactionInterface.
    
    Example:
    <pre>
    @textblock
    IOCFPluginInterface **              iodev;  // obtained earlier
    
    IOHIDDeviceTransactionInterface	**  intf;   // fetching this now
    IOReturn                            err;
    
    err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOHIDDeviceTransactionInterfaceID),
                                (LPVoid)&intf);
    @/textblock
    </pre>
*/
#define kIOHIDDeviceTransactionInterfaceID CFUUIDGetConstantUUIDWithBytes(NULL, 	\
    0x1f, 0x2e, 0x78, 0xfa, 0x9f, 0xfa, 0x11, 0xda, \
    0x90, 0xb4, 0x00, 0x0d, 0x93, 0x6d, 0x06, 0xd2)


#define IOHID_DEVICE_DEVICE_FUNCS_V1                                                                    \
        IOReturn (*open)(void * self, IOOptionBits options);                                            \
        IOReturn (*close)(void * self, IOOptionBits options);                                           \
        IOReturn (*getProperty)(void * self, CFStringRef key, CFTypeRef * pProperty);                   \
        IOReturn (*setProperty)(void * self, CFStringRef key, CFTypeRef property);                      \
        IOReturn (*getAsyncEventSource)(void * self, CFTypeRef * pSource);                              \
        IOReturn (*copyMatchingElements)(void * self, CFDictionaryRef matchingDict, CFArrayRef * pElements, IOOptionBits options); \
        IOReturn (*setValue)(void * self, IOHIDElementRef element, IOHIDValueRef value, uint32_t timeout, IOHIDValueCallback callback, void * context, IOOptionBits options); \
        IOReturn (*getValue)(void * self, IOHIDElementRef element, IOHIDValueRef * pValue, uint32_t timeout, IOHIDValueCallback callback, void * context, IOOptionBits options); \
        IOReturn (*setInputReportCallback)(void * self, uint8_t * report, CFIndex reportLength, IOHIDReportCallback callback, void * context, IOOptionBits options); \
        IOReturn (*setReport)(void * self, IOHIDReportType reportType, uint32_t reportID, const uint8_t * report, CFIndex reportLength, uint32_t timeout, IOHIDReportCallback callback, void * context, IOOptionBits options); \
        IOReturn (*getReport)(void * self, IOHIDReportType reportType, uint32_t reportID, uint8_t * report, CFIndex * pReportLength, uint32_t timeout, IOHIDReportCallback callback, void * context, IOOptionBits options)
/*! @interface  IOHIDDeviceDeviceInterface
    @abstract   The object you use to access HID devices from user space, returned by version 1.5 of the IOHIDFamily.
    @discussion The functions listed here will work with any version of the IOHIDDeviceDeviceInterface. 
    
    <b>Note:</b> Please note that methods declared in this interface follow the copy/get/set conventions.
*/
typedef struct IOHIDDeviceDeviceInterface {
    IUNKNOWN_C_GUTS;
#ifdef IOHID_DEVICE_DEVICE_FUNCS_V1 // {
    IOHID_DEVICE_DEVICE_FUNCS_V1;
#else // } {
    /*! @function   open
        @abstract   Opens the IOHIDDevice.
        @discussion Before the client can issue commands that change the state of the device, it must have succeeded in 
                    opening the device. This establishes a link between the client's task and the actual device.  To 
                    establish an exclusive link use the kIOHIDOptionsTypeSeizeDevice option.
        @param      self Pointer to the IOHIDDeviceDeviceInterface.
        @param      options Option bits to be passed down to the user client.
        @result     Returns kIOReturnSuccess if successful, some other mach error if the connection is no longer valid.
    */
    IOReturn (*open)(void * self, IOOptionBits options);

    /*! @function   close
        @abstract   Closes the task's connection to the IOHIDDevice.
        @discussion Releases the client's access to the IOHIDDevice.
        @param      self Pointer to the IOHIDDeviceDeviceInterface.
        @param      options Option bits to be passed down to the user client.
        @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
    */    
    IOReturn (*close)(void * self, IOOptionBits options);

    /*! @function   getProperty
        @abstract   Obtains a property related to the IOHIDDevice.
        @discussion Property keys are prefixed by kIOHIDDevice and declared in IOHIDKeys.h.
        @param      self Pointer to the IOHIDDeviceDeviceInterface.
        @param      key CFStringRef key
        @param      pProperty Pointer to a CFTypeRef property.
        @result     Returns kIOReturnSuccess if successful.
    */    
    IOReturn (*getProperty)(void * self, CFStringRef key, CFTypeRef * pProperty);

    /*! @function   setProperty
        @abstract   Sets a property related to the IOHIDDevice.
        @discussion Property keys are prefixed by kIOHIDDevice and declared in IOHIDKeys.h.
        @param      self Pointer to the IOHIDDeviceDeviceInterface.
        @param      key CFStringRef key
        @param      property CFTypeRef property.
        @result     Returns kIOReturnSuccess if successful.
    */    
    IOReturn (*setProperty)(void * self, CFStringRef key, CFTypeRef property);
    
    /*! @function   getAsyncEventSource
        @abstract   Obtains the event source for this IOHIDDeviceDeviceInterface instance.
        @discussion The returned event source can be of type CFRunLoopSourceRef or CFRunLoopTimerRef.
        @param      self Pointer to the IOHIDDeviceDeviceInterface.
        @param      pSource Pointer to a CFType to return the run loop event source.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*getAsyncEventSource)(void * self, CFTypeRef * pSource);

    /*! @function   copyMatchingElements
        @abstract   Obtains a CFArrayRef containing the IOHIDDeviceDeviceInterface elements that match 
                    the passed matching dictionary.
        @discussion Objects contained in the returned array are of type IOHIDElementRef.  Please see 
                    IOHIDElement.h for additional API information.  Elemenet properties are prefixed by 
                    kIOHIDElement and declared in IOHIDKeys.h.
        @param      self Pointer to the IOHIDDeviceDeviceInterface.
        @param      matchingDict CFDictionaryRef containing the element properties to match on.
        @param      pElements CFArrayRef containing matched elements.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*copyMatchingElements)(void * self, CFDictionaryRef matchingDict, CFArrayRef * pElements, IOOptionBits options);
    
    /*! @function   setValue
        @abstract   Sets the value for an element.
        @discussion If setting multiple element values, please consider using an IOHIDDeviceTransactionInterface
                    with the kIOHIDTransactionDirectionTypeOutput direction.
                    <br>
                    <b>Note:</b> In order to make use of asynchronous behavior, the event source obtained using getAsyncEventSource
                    must be added to a run loop. 
        @param      self Pointer to the IOHIDDeviceDeviceInterface.
        @param      element IOHIDElementRef referencing the element of interest.
        @param      value IOHIDValueRef containing element value to be set.
        @param      timeout Time in milliseconds to wait before aborting request.
        @param      callback Callback of type IOHIDValueCallback to be used after report data has been sent to the device.
                    If null, this method will behave synchronously.
        @param      context Pointer to data to be passed to the callback.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*setValue)(void * self, IOHIDElementRef element, IOHIDValueRef value,
                    uint32_t timeout, IOHIDValueCallback callback, void * context, IOOptionBits options);

    /*! @function   getValue
        @abstract   Obtains the current value for an element.
        @discussion If an element of type kIOHIDElementTypeFeature is passed, this method will issue a request to the IOHIDDevice.  
                    Otherwise, this will return the last value reported by the IOHIDDevice.  If requesting multiple feature element 
                    values, please consider using an IOHIDDeviceTransactionInterface with the kIOHIDTransactionDirectionTypeInput direction.
                    <br>
                    <b>Note:</b> In order to make use of asynchronous behavior, the event source obtained using getAsyncEventSource
                    must be added to a run loop. 
        @param      self Pointer to the IOHIDDeviceDeviceInterface.
        @param      element IOHIDElementRef referencing the element of interest.
        @param      pValue Pointer to a IOHIDValueRef to return the element value.
        @param      timeout Time in milliseconds to wait before aborting request.
        @param      callback Callback of type IOHIDReportCallback to be used when element value has been received from the device.
                    If null, this method will behave synchronously.
        @param      context Pointer to data to be passed to the callback.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*getValue)(void * self, IOHIDElementRef element, IOHIDValueRef * pValue,
                    uint32_t timeout, IOHIDValueCallback callback, void * context, IOOptionBits options);

    /*!
        @function   setInputReportCallback
        @abstract   Sets the input report callback to be used when data is received from the Input pipe.
        @discussion In order to function properly, the event source obtained using getAsyncEventSource must be added to a run loop.
        @param      self Pointer to the IOHIDDeviceDeviceInterface.
        @param      report Pointer to a pre-allocated buffer to be filled and passed back via the callback.
        @param      reportLength Length of the report buffer.
        @param      callback Callback of type IOHIDReportCallback to be used when report data has been receieved by the IOHIDDevice.
        @param      context Pointer to data to be passed to the callback.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*setInputReportCallback)(void * self, uint8_t * report, CFIndex reportLength,
                    IOHIDReportCallback callback, void * context, IOOptionBits options);

    /*! @function   setReport
        @abstract   Sends a report of type kIOHIDReportTypeOutput or kIOHIDReportTypeFeature to the IOHIDDevice.
        @discussion This method is useful if specific knowledge of the unparsed report is known to the caller.  Otherwise, using
                    an IOHIDDeviceTransactionInterface with the kIOHIDTransactionDirectionTypeOutput direction is recommended.
                    <br>
                    <b>Note:</b> In order to make use of asynchronous behavior, the event source obtained using getAsyncEventSource
                    must be added to a run loop. 
        @param      self Pointer to the IOHIDDeviceDeviceInterface.
        @param      reportType The report type.
        @param      reportID The report id.
        @param      report Pointer to a buffer containing the report data to be sent.
        @param      reportLength Length of the report buffer.
        @param      timeout Timeout in milliseconds for issuing the setReport.
        @param      callback Callback of type IOHIDReportCallback to be used after report data has been sent to the device.
                    If null, this method will behave synchronously.
        @param      context Pointer to data to be passed to the callback.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*setReport)(void * self, IOHIDReportType reportType, uint32_t reportID, const uint8_t * report, CFIndex reportLength,
                    uint32_t timeout, IOHIDReportCallback callback, void * context, IOOptionBits options);

    /*! @function   getReport
        @abstract   Obtains a report of type kIOHIDReportTypeInput or kIOHIDReportTypeFeature from the IOHIDDevice.
        @discussion This method is useful if specific knowledge of the unparsed report is known to the caller.  Otherwise, using
                    an IOHIDDeviceTransactionInterface with the kIOHIDTransactionDirectionTypeInput direction is recommended.
                    <br>
                    <b>Note:</b> In order to make use of asynchronous behavior, the event source obtained using getAsyncEventSource
                    must be added to a run loop. 
        @param      self Pointer to the IOHIDDeviceDeviceInterface.
        @param      reportType The report type.
        @param      reportID The report id.
        @param      report Pointer to a pre-allocated buffer to be filled.
        @param      pReportLength Length of the report buffer.  When finished, this will contain the actual length of the report.
        @param      timeout Timeout in milliseconds for issuing the getReport.
        @param      callback Callback of type IOHIDReportCallback to be used when report data has been received from the device.
                    If null, this method will behave synchronously.
        @param      context Pointer to data to be passed to the callback.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*getReport)(void * self, IOHIDReportType reportType, uint32_t reportID, uint8_t * report, CFIndex * pReportLength,
                    uint32_t timeout, IOHIDReportCallback callback, void * context, IOOptionBits options);
#endif // }
} IOHIDDeviceDeviceInterface;

#define IOHID_DEVICE_DEVICE_FUNCS_V2                                                        \
        IOReturn (*setInputReportWithTimeStampCallback)(void * self, uint8_t * report, CFIndex reportLength, IOHIDReportWithTimeStampCallback callback, void * context, IOOptionBits options)
/*! @interface  IOHIDDeviceTimeStampedDeviceInterface
    @abstract   The object you use to access HID devices from user space, returned by version 2.1 of the IOHIDFamily.
    @discussion The functions listed here include all of the functions from the IOHIDDeviceDeviceInterface.
    
    <b>Note:</b> Please note that methods declared in this interface follow the copy/get/set conventions.
*/
typedef struct IOHIDDeviceTimeStampedDeviceInterface {
    IUNKNOWN_C_GUTS;
    IOHID_DEVICE_DEVICE_FUNCS_V1;
#ifdef IOHID_DEVICE_DEVICE_FUNCS_V2 // {
    IOHID_DEVICE_DEVICE_FUNCS_V2;
#else // } {
/*!
    @function   setInputReportWithTimeStampCallback
    @abstract   Sets the input report callback to be used when data is received from the Input pipe.
    @discussion In order to function properly, the event source obtained using getAsyncEventSource must be added to a run loop.
    @param      self Pointer to the IOHIDDeviceDeviceInterface.
    @param      report Pointer to a pre-allocated buffer to be filled and passed back via the callback.
    @param      reportLength Length of the report buffer.
    @param      callback Callback of type IOHIDReportWithTimeStampCallback to be used when report data has been receieved by the IOHIDDevice.
    @param      context Pointer to data to be passed to the callback.
    @param      options Reserved for future use. Ignored in current implementation. Set to zero.
    @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
*/
IOReturn (*setInputReportWithTimeStampCallback)(void * self, uint8_t * report, CFIndex reportLength,
                IOHIDReportWithTimeStampCallback callback, void * context, IOOptionBits options);

#endif // }
} IOHIDDeviceTimeStampedDeviceInterface;

/*!
    @interface  IOHIDDeviceQueueInterface
    @abstract   The object you use to access a HID queue from user space, returned by version 1.5 of the IOHIDFamily.
    @discussion The functions listed here will work with any version of the IOHIDDeviceQueueInterface.  This behavior is useful when you 
                need to keep track of all values of an input element, rather than just the most recent one.
                <br>
                <b>Note:</b>Absolute element values (based on a fixed origin) will only be placed on a queue if there is a change in value. 
*/
typedef struct IOHIDDeviceQueueInterface {
    IUNKNOWN_C_GUTS;

    /*! @function   getAsyncEventSource
        @abstract   Obtains the event source for this IOHIDDeviceQueueInterface instance.
        @discussion The returned event source can be of type CFRunLoopSourceRef or CFRunLoopTimerRef.
        @param      self Pointer to the IOHIDDeviceQueueInterface.
        @param      pSource Pointer to a CFType to return the run loop event source.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*getAsyncEventSource)(void * self, CFTypeRef * pSource);
    
    /*! @function   setDepth
        @abstract   Sets the depth for this IOHIDDeviceQueueInterface instance.
        @discussion Regardless of element value size, queue will guarantee n=depth elements will be serviced.
        @param      self Pointer to the IOHIDDeviceTransactionInterface.
        @param      depth The maximum number of elements in the queue before the oldest elements in the queue begin to be lost.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*setDepth)(void *self, uint32_t depth, IOOptionBits options);

    /*! @function   getDepth
        @abstract   Obtains the queue depth for this IOHIDDeviceQueueInterface instance.
        @param      self Pointer to the IOHIDDeviceQueueInterface.
        @param      pDepth Pointer to a uint32_t to obtain the number of elements that can be serviced by the queue.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*getDepth)(void *self, uint32_t * pDepth);

    /*! @function   addElement
        @abstract   Adds an element to this IOHIDDeviceQueueInterface instance.
        @param      self Pointer to the IOHIDDeviceQueueInterface.
        @param      element IOHIDElementRef referencing the element to be added to the queue.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*addElement)(void * self, IOHIDElementRef element, IOOptionBits options);

    /*! @function   removeElement
        @abstract   Removes an element from this IOHIDDeviceQueueInterface instance.
        @param      self Pointer to the IOHIDDeviceQueueInterface.
        @param      element IOHIDElementRef referencing the element to be removed from the queue.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*removeElement)(void * self, IOHIDElementRef element, IOOptionBits options);

    /*! @function   containsElement
        @abstract   Determines whether an element has been added to this IOHIDDeviceQueueInterface instance.
        @param      self Pointer to the IOHIDDeviceQueueInterface.
        @param      element IOHIDElementRef referencing the element to be be found in the queue.
        @param      pValue Pointer to a Boolean to return whether or not the element was found in the queue.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*containsElement)(void * self, IOHIDElementRef element, Boolean * pValue, IOOptionBits options);

    /*! @function   start
        @abstract   Starts element value delivery to the queue.
        @param      self Pointer to the IOHIDDeviceQueueInterface.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*start)(void * self, IOOptionBits options);

    /*! @function   stop
        @abstract   Stops element value delivery to the queue.
        @param      self Pointer to the IOHIDDeviceQueueInterface.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*stop)(void * self, IOOptionBits options);
    
    /*! @function   setValueAvailableCallback
        @abstract   Sets callback to be used when the queue transitions to non-empty.
        @discussion In order to make use of asynchronous behavior, the event source obtained using getAsyncEventSource
                    must be added to a run loop. 
        @param      self Pointer to the IOHIDDeviceQueueInterface.
        @param      callback Callback of type IOHIDCallback to be used when data is placed on the queue.
        @param      context Pointer to data to be passed to the callback.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*setValueAvailableCallback)(void * self, IOHIDCallback callback, void * context);
    
    /*! @function   copyNextValue
        @abstract   Dequeues a retained copy of an element value from the head of an IOHIDDeviceQueueInterface.
        @discussion Because the value is a retained copy, it is up to the caller to release the value using CFRelease. 
                    Use with setValueCallback to avoid polling the queue for data.
        @param      self Pointer to the IOHIDDeviceQueueInterface.
        @param      pValue Pointer to a IOHIDValueRef to return the value at the head of the queue.
        @param      timeout Timeout in milliseconds before aborting an attempt to dequeue a value from the head of a queue.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful, kIOReturnUnderrun if data is unavailble, or a kern_return_t if unsuccessful.
    */
    IOReturn (*copyNextValue)(void * self, IOHIDValueRef * pValue, uint32_t timeout, IOOptionBits options);
} IOHIDDeviceQueueInterface;


/*!
    @interface  IOHIDDeviceTransactionInterface
    @abstract   The object you use to access a HID transaction from user space, returned by version 1.5 of the IOHIDFamily.
    @discussion The functions listed here will work with any version of the IOHIDDeviceTransactionInterface. This functionality
                is useful when either setting or getting the values for multiple parsed elements.
*/
typedef struct IOHIDDeviceTransactionInterface {
    IUNKNOWN_C_GUTS;

    /*! @function   getAsyncEventSource
        @abstract   Obtains the event source for this IOHIDDeviceTransactionInterface instance.
        @discussion The returned event source can be of type CFRunLoopSourceRef or CFRunLoopTimerRef.
        @param      self Pointer to the IOHIDDeviceTransactionInterface.
        @param      pSource Pointer to a CFType to return the run loop event source.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*getAsyncEventSource)(void * self, CFTypeRef * pSource);

    /*! @function   setDirection
        @abstract   Sets the direction for this IOHIDDeviceTransactionInterface instance.
        @discussion Direction constants are declared in IOHIDTransactionDirectionType.  Changing directions
                    is useful when dealing with elements of type kIOHIDElementTypeFeature as you use the
                    transaction to both set and get element values.
        @param      self Pointer to the IOHIDDeviceTransactionInterface.
        @param      direction Transaction direction of type IOHIDTransactionDirectionType.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*setDirection)(void * self, IOHIDTransactionDirectionType direction, IOOptionBits options);

    /*! @function   getDirection
        @abstract   Obtains the direction for this IOHIDDeviceTransactionInterface instance.
        @discussion Direction constants are declared in IOHIDTransactionDirectionType.
        @param      self Pointer to the IOHIDDeviceTransactionInterface.
        @param      pDirection Pointer to a IOHIDTransactionDirectionType to obtain transaction direction.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*getDirection)(void * self, IOHIDTransactionDirectionType * pDirection);
    
    /*! @function   addElement
        @abstract   Adds an element to this IOHIDDeviceTransactionInterface instance.
        @param      self Pointer to the IOHIDDeviceTransactionInterface.
        @param      element IOHIDElementRef referencing the element to be added to the transaction.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*addElement)(void * self, IOHIDElementRef element, IOOptionBits options);
    
    /*! @function   removeElement
        @abstract   Removes an element from this IOHIDDeviceTransactionInterface instance.
        @param      self Pointer to the IOHIDDeviceTransactionInterface.
        @param      element IOHIDElementRef referencing the element to be removed from the transaction.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*removeElement)(void * self, IOHIDElementRef element, IOOptionBits options);

    /*! @function   containsElement
        @abstract   Checks whether an element has been added to this IOHIDDeviceTransactionInterface instance.
        @param      self Pointer to the IOHIDDeviceTransactionInterface.
        @param      element IOHIDElementRef referencing the element to be be found in the transaction.
        @param      pValue Pointer to a Boolean to return whether or not the element was found in the transaction.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*containsElement)(void * self, IOHIDElementRef element, Boolean * pValue, IOOptionBits options);

    /*! @function   setValue
        @abstract   Sets the transaction value for an element in this IOHIDDeviceTransactionInterface instance.
        @discussion This method is intended for use with transaction of direction kIOHIDTransactionDirectionTypeOutput.
                    Use the kIOHIDTransactionOptionDefaultOutputValue option to set the default element value.
        @param      self Pointer to the IOHIDDeviceTransactionInterface.
        @param      element IOHIDElementRef referencing the element of interest.
        @param      value IOHIDValueRef referencing element value to be used in the transaction.
        @param      options See IOHIDTransactionOption.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*setValue)(void * self, IOHIDElementRef element, IOHIDValueRef value, IOOptionBits options);

    /*! @function   getValue
        @abstract   Obtains the transaction value for an element in this IOHIDDeviceTransactionInterface instance.
        @discussion Use the kIOHIDTransactionOptionDefaultOutputValue option to get the default element value.
        @param      self Pointer to the IOHIDDeviceTransactionInterface.
        @param      element IOHIDElementRef referencing the element of interest.
        @param      pValue Pointer to an IOHIDValueRef to return the element value of the transaction.
        @param      options See IOHIDTransactionOption.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*getValue)(void * self, IOHIDElementRef element, IOHIDValueRef *pValue, IOOptionBits options);
    
    /*! @function   commit
        @abstract   Commits element transaction to an IOHIDDevice in this IOHIDDeviceTransactionInterface instance.
        @discussion In regards to kIOHIDTransactionDirectionTypeOutput direction, default element values will be used if
                    element values are not set.  If neither are set, that element will be omitted from the commit. After 
                    a transaction is committed, transaction element values will be cleared and default values preserved.
                    <br>
                    <b>Note:</b> It is possible for elements from different reports to be present in a given transaction 
                    causing a commit to transcend multiple reports. Keep this in mind when setting a timeout.
        @param      self Pointer to the IOHIDDeviceTransactionInterface.
        @param      timeout Timeout in milliseconds for issuing the transaction.
        @param      callback Callback of type IOHIDCallback to be used when transaction has been completed.  If null, 
                    this method will behave synchronously.
        @param      context Pointer to data to be passed to the callback.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*commit)(void * self, uint32_t timeout, IOHIDCallback callback, void * context, IOOptionBits options);

    /*! @function   clear
        @abstract   Clears element transaction values for an IOHIDDeviceTransactionInterface.
        @discussion In regards to kIOHIDTransactionDirectionTypeOutput direction, default element values will be preserved.
        @param      self Pointer to the IOHIDDeviceTransactionInterface.
        @param      options Reserved for future use. Ignored in current implementation. Set to zero.
        @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
    */
    IOReturn (*clear)(void * self, IOOptionBits options);
} IOHIDDeviceTransactionInterface;

__END_DECLS

#endif /* _IOKIT_HID_IOHIDDEVICEPLUGIN_H */
