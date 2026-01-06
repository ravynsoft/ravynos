/*
 * Copyright © 2006-2018 Apple, Inc. All rights reserved.
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

#ifndef _IOUSBLIB_H
#define _IOUSBLIB_H

#include <IOKit/usb/USB.h>
#include <IOKit/IOKitLib.h>

#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFPlugIn.h>
#if COREFOUNDATION_CFPLUGINCOM_SEPARATE
#include <CoreFoundation/CFPlugInCOM.h>
#endif

#include <sys/cdefs.h>

__BEGIN_DECLS

/*!
    @header IOUSBLib
    This documentation describes the details of the programming interface for accessing USB devices and USB
    interfaces from code running in user space.  This documentation assumes that you have a basic understanding
    of the material contained in <a href="http://developer.apple.com/documentation/DeviceDrivers/Conceptual/AccessingHardware/index.html"><i>Accessing Hardware From Applications</i></a>
    For definitions of I/O Kit terms used in this documentation, such as matching dictionary, family, and driver, see the overview of I/O Kit terms and concepts
    in the "Device Access and the I/O Kit" chapter of <i>Accessing Hardware From Applications</i>.

    This documentation also assumes you have read <a href="http://developer.apple.com/documentation/DeviceDrivers/Conceptual/USBBook/index.html"><i>Working With USB Device Interfaces</i></a>.
    Please review that document before using this reference.

    All of the information described in this document is contained in the header file <font face="Courier New,Courier,Monaco">IOUSBLib.h</font> found at
    <font face="Courier New,Courier,Monaco">/System/Library/Frameworks/IOKit.framework/Headers/usb/IOUSBLib.h</font>.
 */

// 9dc7b780-9ec0-11d4-a54f-000a27052861
/*!
    @defined kIOUSBDeviceUserClientTypeID
    @discussion This UUID constant is used to obtain a device interface corresponding to
   an io_service_t corresponding to an IOUSBDevice in the kernel. Once you have
   obtained the device interface for the service, you must use the QueryInterface
   function to obtain the device interface for the user client itself.

   Example:
   <pre>
   @textblock
   io_service_t            usbDeviceRef;   // obtained earlier

   IOCFPlugInInterface     **iodev;        // fetching this now

   SInt32                  score;          // not used
   IOReturn                err;

   err = IOCreatePlugInInterfaceForService(usbDeviceRef,
                                    kIOUSBDeviceUserClientTypeID,
                                    kIOCFPlugInInterfaceID,
                                    &iodev,
                                    &score);
    @/textblock
    </pre>
 */

#define kIOUSBDeviceUserClientTypeID CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                    0x9d, 0xc7, 0xb7, 0x80, 0x9e, 0xc0, 0x11, 0xD4, \
                                                                    0xa5, 0x4f, 0x00, 0x0a, 0x27, 0x05, 0x28, 0x61)

// 2d9786c6-9ef3-11d4-ad51-000a27052861
/*!
    @defined kIOUSBInterfaceUserClientTypeID
    @discussion This UUID constant is used to obtain a device interface corresponding to
   an io_service_t corresponding to an IOUSBInterface in the kernel. Once you have
   obtained the device interface for the service, you must use the QueryInterface
   function to obtain the device interface for the user client itself.

   Example:
   <pre>
   @textblock
   io_service_t        usbInterfaceRef;	// obtained earlier

   IOCFPlugInInterface	**iodev;                // fetching this now

   SInt32              score;                  // not used
   IOReturn            err;

   err = IOCreatePlugInInterfaceForService(usbInterfaceRef,
                                    kIOUSBInterfaceUserClientTypeID,
                                    kIOCFPlugInInterfaceID,
                                    &iodev,
                                    &score);
    @/textblock
    </pre>
 */

#define kIOUSBInterfaceUserClientTypeID CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                       0x2d, 0x97, 0x86, 0xc6, 0x9e, 0xf3, 0x11, 0xD4, \
                                                                       0xad, 0x51, 0x00, 0x0a, 0x27, 0x05, 0x28, 0x61)

// 4547a8aa-9ef3-11d4-a9bd-000a27052861
/*!
    @defined kIOUSBFactoryID
    @discussion This UUID constant is used internally by the system, and
    should not have to be used by any driver code to access the device interfaces.
 */

#define kIOUSBFactoryID CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                       0x45, 0x47, 0xa8, 0xaa, 0x9e, 0xf3, 0x11, 0xD4, \
                                                       0xa9, 0xbd, 0x00, 0x0a, 0x27, 0x05, 0x28, 0x61)

// 5c8187d0-9ef3-11d4-8b45-000a27052861
/*!
    @defined kIOUSBDeviceInterfaceID100
    @discussion This UUID constant is used to obtain a device interface corresponding
    to an IOUSBDevice user client in the kernel. The type of this device interface is
    IOUSBDeviceInterface. This device interface is obtained after the device interface for
    the service itself has been obtained.

    <b>Note:</b> The IOUSBDeviceInterface is returned by all versions of the IOUSBFamily
    currently shipping. However, there are some functions that are available only in
    IOUSBFamily version 1.8.2 and above. Access to these functions, in addition to the functions
    contained in IOUSBDeviceInterface, can be obtained by using one of the other UUIDs listed in this header.

    Example:
    <pre>
    @textblock
    IOCFPluginInterface		**iodev;        // obtained earlier

    IOUSBDeviceInterface	**dev;		// fetching this now
    IOReturn                    err;

    err = (*iodev)->QueryInterface(iodev,
                                    CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID100),
                                    (LPVoid)&dev);
    @/textblock
    </pre>
 */

#define kIOUSBDeviceInterfaceID100 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                  0x5c, 0x81, 0x87, 0xd0, 0x9e, 0xf3, 0x11, 0xD4, \
                                                                  0x8b, 0x45, 0x00, 0x0a, 0x27, 0x05, 0x28, 0x61)

// 152FC496-4891-11D5-9D52-000A27801E86
/*!
    @defined kIOUSBDeviceInterfaceID182
    @discussion This UUID constant is used to obtain a device interface corresponding to
    an IOUSBDevice user client in the kernel. The type of this device interface is
    IOUSBDeviceInterface182. This device interface is obtained after the device interface
    for the service itself has been obtained.

    <b>Note:</b> The IOUSBDeviceInterface182 is returned only by version 1.8.2
    or above of the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X
    version 10.0.4. If your software is running on an earlier version of Mac OS X,
    you will need to use the UUID kIOUSBDeviceInterfaceID and you will not have access to some functions.

    Example:
    <pre>
    @textblock
    IOCFPluginInterface		**iodev;        // obtained earlier

    IOUSBDeviceInterface182	**dev;		// fetching this now
    IOReturn                    err;

    err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID182),
                                (LPVoid)&dev);
    @/textblock
    </pre>
 */

#define kIOUSBDeviceInterfaceID182 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                  0x15, 0x2f, 0xc4, 0x96, 0x48, 0x91, 0x11, 0xD5, \
                                                                  0x9d, 0x52, 0x00, 0x0a, 0x27, 0x80, 0x1e, 0x86)

// 3C9EE1EB-2402-11B2-8E7E-000A27801E86
/*!
    @defined kIOUSBDeviceInterfaceID187
    @discussion This UUID constant is used to obtain a device interface corresponding
    to an IOUSBDevice user client in the kernel. The type of this device interface is
    IOUSBDeviceInterface187. This device interface is obtained after the device interface
    for the service itself has been obtained (see @link kIOUSBDeviceUserClientTypeID kIOUSBDeviceUserClientTypeID @/link).

    <b>Note:</b> The IOUSBDeviceInterface187 is returned only by version 1.8.7 or above of
    the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.1.2. If your
    software is running on an earlier version of Mac OS X you will need to use UUID kIOUSBDeviceInterfaceID
    or kIOUSBDeviceInterfaceID182 and you will not have access to some functions.

    Example:
    <pre>
    @textblock
    IOCFPluginInterface		**iodev;        // obtained earlier

    IOUSBDeviceInterface187	**dev;		// fetching this now
    IOReturn                    err;

    err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID187),
                                (LPVoid)&dev);
    @/textblock
    </pre>
 */

#define kIOUSBDeviceInterfaceID187 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                  0x3C, 0x9E, 0xE1, 0xEB, 0x24, 0x02, 0x11, 0xB2, \
                                                                  0x8E, 0x7E, 0x00, 0x0A, 0x27, 0x80, 0x1E, 0x86)

// C809B8D8-0884-11D7-BB96-0003933E3E3E
/*!
    @defined kIOUSBDeviceInterfaceID197
    @discussion This UUID constant is used to obtain a device interface corresponding to
    an IOUSBDevice user client in the kernel. The type of this device interface is
    IOUSBDeviceInterface197. This device interface is obtained after the device interface for
    the service itself has been obtained.

    <b>Note:</b> The IOUSBDeviceInterface197 is returned only by version 1.9.7 or above of the
    IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.2.3. If your software
    is running on an earlier version of Mac OS X you will need to use UUID kIOUSBDeviceInterfaceID,
    kIOUSBDeviceInterfaceID182, or kIOUSBDeviceInterfaceID187 and you will not have access to some functions.

    Example:
    <pre>
    @textblock
    IOCFPluginInterface		**iodev;        // obtained earlier

    IOUSBDeviceInterface197	**dev;		// fetching this now
    IOReturn                    err;

    err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID197),
                                (LPVoid)&dev);
    @/textblock
    </pre>
 */

#define kIOUSBDeviceInterfaceID197 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                  0xC8, 0x09, 0xB8, 0xD8, 0x08, 0x84, 0x11, 0xD7, \
                                                                  0xBB, 0x96, 0x00, 0x03, 0x93, 0x3E, 0x3E, 0x3E)

// FE2FD52F-3B5A-473B-978B-AD99001EB3ED
/*!
   @defined kIOUSBDeviceInterfaceID245
   @discussion This UUID constant is used to obtain a device interface corresponding to
   an IOUSBDevice user client in the kernel. The type of this device interface is
   IOUSBDeviceInterface245. This device interface is obtained after the device interface for
   the service itself has been obtained.

   <b>Note:</b> The IOUSBDeviceInterface245 is returned only by version 2.4.5 or above of the
   IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.4.5 (for Intel). This version
   does not add any more functions to the interface.  It is used to allow us to fix an overrelease in our termination without affecting
   any current drivers:  In previous versions, we would end up releasing our IOService, even though we had not retained it.  For
   IOUSBDeviceInterfaceID245 clients we will retain the IOService.

   Example:
   <pre>
   @textblock
   IOCFPluginInterface		**iodev;        // obtained earlier

   IOUSBDeviceInterface245	**dev;		// fetching this now
   IOReturn                    err;

   err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID245),
                                (LPVoid)&dev);
   @/textblock
   </pre>
 */

#define kIOUSBDeviceInterfaceID245 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                  0xFE, 0x2F, 0xD5, 0x2F, 0x3B, 0x5A, 0x47, 0x3B, \
                                                                  0x97, 0x7B, 0xAD, 0x99, 0x00, 0x1E, 0xB3, 0xED)


// 396104F7-943D-4893-90F1-69BD6CF5C2EB
/*!
   @defined kIOUSBDeviceInterfaceID300
   @discussion This UUID constant is used to obtain a device interface corresponding to
   an IOUSBDevice user client in the kernel. The type of this device interface is
   IOUSBDeviceInterface300. This device interface is obtained after the device interface for
   the service itself has been obtained.

   <b>Note:</b> The IOUSBDeviceInterface300 is returned only by version 3.0.0 or above of the
   IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.5. If your software
   is running on an earlier version of Mac OS X you will need to use UUID kIOUSBDeviceInterfaceID,
   kIOUSBDeviceInterfaceID182, kIOUSBDeviceInterfaceID187, kIOUSBDeviceInterfaceID197, or kIOUSBDeviceInterfaceID245
   and you will not have access to some functions.

   Example:
   <pre>
   @textblock
   IOCFPluginInterface		**iodev;        // obtained earlier

   IOUSBDeviceInterface300	**dev;		// fetching this now
   IOReturn                    err;

   err = (*iodev)->QueryInterface(iodev,
   CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID300),
   (LPVoid)&dev);
   @/textblock
   </pre>
 */

#define kIOUSBDeviceInterfaceID300 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                  0x39, 0x61, 0x04, 0xF7, 0x94, 0x3D, 0x48, 0x93, \
                                                                  0x90, 0xF1, 0x69, 0xBD, 0x6C, 0xF5, 0xC2, 0xEB)


// 01A2D0E9-42F6-4A87-8B8B-77057C8CE0CE
/*!
   @defined kIOUSBDeviceInterfaceID320
   @discussion This UUID constant is used to obtain a device interface corresponding to
   an IOUSBDevice user client in the kernel. The type of this device interface is
   IOUSBDeviceInterface320. This device interface is obtained after the device interface for
   the service itself has been obtained.

   <b>Note:</b> The IOUSBDeviceInterface320 is returned only by version 3.2.0 or above of the
   IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.5.4 If your software
   is running on an earlier version of Mac OS X you will need to use UUID kIOUSBDeviceInterfaceID,
   kIOUSBDeviceInterfaceID182, kIOUSBDeviceInterfaceID187, kIOUSBDeviceInterfaceID197, kIOUSBDeviceInterfaceID245,
   or kIOUSBDeviceInterfaceID300  and you will not have access to some functions.

   Example:
   <pre>
   @textblock
   IOCFPluginInterface		**iodev;        // obtained earlier

   IOUSBDeviceInterface320	**dev;		// fetching this now
   IOReturn                    err;

   err = (*iodev)->QueryInterface(iodev,
   CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID320),
   (LPVoid)&dev);
   @/textblock
   </pre>
 */

#define kIOUSBDeviceInterfaceID320 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault,                      \
                                                                  0x01, 0xA2, 0xD0, 0xE9, 0x42, 0xF6, 0x4A, 0x87, \
                                                                  0x8B, 0x8B, 0x77, 0x05, 0x7C, 0x8C, 0xE0, 0xCE)

// 812493DC-9189-4C11-B136-C610086FAE41
/*!
 @defined kIOUSBDeviceInterfaceID400
 @discussion This UUID constant is used to obtain a device interface corresponding to
 an IOUSBDevice user client in the kernel. The type of this device interface is
 IOUSBDeviceInterface400. This device interface is obtained after the device interface for
 the service itself has been obtained.

 First available macOS 10.14
 */
#define kIOUSBDeviceInterfaceID400 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault,                      \
                                                                  0x81, 0x24, 0x93, 0xDC, 0x91, 0x89, 0x4C, 0x11, \
                                                                  0xB1, 0x36, 0xC6, 0x10, 0x08, 0x6F, 0xAE, 0x41)

// A33CF047-4B5B-48E2-B57D-0207FCEAE13B
/*!
 @defined kIOUSBDeviceInterfaceID500
 @discussion This UUID constant is used to obtain a device interface corresponding to
 an IOUSBDevice user client in the kernel. The type of this device interface is
 IOUSBDeviceInterface500. This device interface is obtained after the device interface for
 the service itself has been obtained.

 First available Mac OS X 10.7.3
 */
#define kIOUSBDeviceInterfaceID500 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault,                        \
                                                                  0xA3, 0x3C, 0xF0, 0x47, 0x4B, 0x5B, 0x48, 0xE2,   \
                                                                  0xB5, 0x7D, 0x02, 0x07, 0xFC, 0xEA, 0xE1, 0x3B)

// 4AAC1B2E-24C2-476A-964D-91333534F2CC
/*!
 @defined kIOUSBDeviceInterfaceID650
 @discussion This UUID constant is used to obtain a device interface corresponding to
 an IOUSBDevice user client in the kernel. The type of this device interface is
 IOUSBDeviceInterface650. This device interface is obtained after the device interface for
 the service itself has been obtained.

 First available Mac OS X 10.9
 */
#define kIOUSBDeviceInterfaceID650 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault,                        \
                                                                  0x4A, 0xAC, 0x1B, 0x2E, 0x24, 0xC2, 0x47, 0x6A,   \
                                                                  0x96, 0x4D, 0x91, 0x33, 0x35, 0x34, 0xF2, 0xCC)

// 56AD089D-878D-4BEA-A1F5-2C8DC43E8A98
/*!
 @defined kIOUSBDeviceInterfaceID942
 @discussion This UUID constant is used to obtain a device interface corresponding to
 an IOUSBDevice user client in the kernel. The type of this device interface is
 IOUSBDeviceInterface942. This device interface is obtained after the device interface for
 the service itself has been obtained.

 First available macOS 10.14
 */
#define kIOUSBDeviceInterfaceID942 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault,                        \
                                                                  0x56, 0xAD, 0x08, 0x9D, 0x87, 0x8D, 0x4B, 0xEA,   \
                                                                  0xA1, 0xF5, 0x2C, 0x8D, 0xC4, 0x3E, 0x8A, 0x98)

// 73c97ae8-9ef3-11d4-b1d0-000a27052861
/*!
 @defined kIOUSBInterfaceInterfaceID100
 @discussion This UUID constant is used to obtain an interface interface corresponding
 to an IOUSBInterface user client in the kernel. The type of this device interface
 is IOUSBInterfaceInterface. This device interface is obtained after the device interface
 for the service itself has been obtained.

 <b>Note:</b> The IOUSBInterfaceInterface is returned by all versions of the IOUSBFamily
 currently shipping. However, there are some functions which are available only in
 IOUSBFamily version 1.8.2 and above. Access to these functions, as well as to all of the functions
 contained in IOUSBInterfaceInterface, can be obtained by using one of the other UUIDs listed in this header.

 Example:
 <pre>
 @textblock
 IOCFPluginInterface        **iodev;        // obtained earlier

 IOUSBInterfaceInterface    **intf;        // fetching this now
 IOReturn                    err;

 err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID100),
                                (LPVoid)&intf);
 @/textblock
 </pre>
 */

#define kIOUSBInterfaceInterfaceID100 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                     0x73, 0xc9, 0x7a, 0xe8, 0x9e, 0xf3, 0x11, 0xD4, \
                                                                     0xb1, 0xd0, 0x00, 0x0a, 0x27, 0x05, 0x28, 0x61)

// 4923AC4C-4896-11D5-9208-000A27801E86
/*!
    @defined kIOUSBInterfaceInterfaceID182
    @discussion This UUID constant is used to obtain an interface interface corresponding to
    an IOUSBInterface user client in the kernel. The type of this device interface is
    IOUSBInterfaceInterface182. This device interface is obtained after the device interface
    for the service itself has been obtained.

    <b>Note:</b> The IOUSBInterfaceInterface182 is returned only by version 1.8.2
    or above of the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X
    version 10.0.4. If your software is running on an earlier version of Mac OS X,
    you will need to use the UUID kIOUSBInterfaceInterfaceID and you will not have
    access to some functions.

    Example:
    <pre>
    @textblock
    IOCFPluginInterface         **iodev;        // obtained earlier

    IOUSBInterfaceInterface182	**intf;		// fetching this now
    IOReturn                    err;

    err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID182),
                                (LPVoid)&intf);
    @/textblock
    </pre>
 */

#define kIOUSBInterfaceInterfaceID182 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                     0x49, 0x23, 0xac, 0x4c, 0x48, 0x96, 0x11, 0xD5, \
                                                                     0x92, 0x08, 0x00, 0x0a, 0x27, 0x80, 0x1e, 0x86)

// 1C438356-74C4-11D5-92E6-000A27801E86
/*!
    @defined kIOUSBInterfaceInterfaceID183
    @discussion This UUID constant is used to obtain an interface interface corresponding to
    an IOUSBInterface user client in the kernel. The type of this device interface
    is IOUSBInterfaceInterface183. This device interface is obtained after the device
    interface for the service itself has been obtained.

    <b>Note:</b> The IOUSBInterfaceInterface183 is returned only by version 1.8.3
    or above of the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X
    version 10.1. If your software is running on a version of Mac OS X prior to 10.1
    you will need to use the UUID kIOUSBInterfaceInterfaceID
    or kIOUSBInterfaceInterfaceID182 and you will not have access to some functions.

    Example:
    <pre>
    @textblock
    IOCFPluginInterface             **iodev;    // obtained earlier

    IOUSBInterfaceInterface183      **intf;     // fetching this now
    IOReturn                        err;

    err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID183),
                                (LPVoid)&intf);
    @/textblock
    </pre>
 */

#define kIOUSBInterfaceInterfaceID183 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                     0x1c, 0x43, 0x83, 0x56, 0x74, 0xc4, 0x11, 0xD5, \
                                                                     0x92, 0xe6, 0x00, 0x0a, 0x27, 0x80, 0x1e, 0x86)

// 8FDB8455-74A6-11D6-97B1-003065D3608E
/*!
    @defined kIOUSBInterfaceInterfaceID190
    @discussion This UUID constant is used to obtain an interface interface corresponding
    to an IOUSBInterface user client in the kernel. The type of this device interface
    is IOUSBInterfaceInterface190. This device interface is obtained after the device
    interface for the service itself has been obtained.

    <b>Note:</b> The IOUSBInterfaceInterface190 is returned only by version 1.9 or above
    of the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.2.
    If your software is running on a version of Mac OS X prior to 10.2 you will need to
    use the UUID kIOUSBInterfaceInterfaceID, kIOUSBInterfaceInterfaceID182, or
    kIOUSBInterfaceInterfaceID183 and you will not have access to some functions.

    Example:
    <pre>
    @textblock
    IOCFPluginInterface             **iodev;    // obtained earlier

    IOUSBInterfaceInterface190      **intf;     // fetching this now
    IOReturn                        err;

    err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID190),
                                (LPVoid)&intf);
    @/textblock
    </pre>
 */

#define kIOUSBInterfaceInterfaceID190 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                     0x8f, 0xdb, 0x84, 0x55, 0x74, 0xa6, 0x11, 0xD6, \
                                                                     0x97, 0xb1, 0x00, 0x30, 0x65, 0xd3, 0x60, 0x8e)

// 6C798A6E-D6E9-11D6-ADD6-0003933E3E3E
/*!
    @defined kIOUSBInterfaceInterfaceID192
    @discussion This UUID constant is used to obtain an interface interface corresponding to
    an IOUSBInterface user client in the kernel. The type of this device interface is
    IOUSBInterfaceInterface192. This device interface is obtained after the device interface
    for the service itself has been obtained.

    <b>Note:</b> The IOUSBInterfaceInterface192 is returned only by version 1.9.2 or above of
    the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.2.3. If your
    software is running on a version of Mac OS X prior to 10.2.3 you will need to use the
    UUID kIOUSBInterfaceInterfaceID, kIOUSBInterfaceInterfaceID182, kIOUSBInterfaceInterfaceID183,
    or kIOUSBInterfaceInterfaceID190 and you will not have access to some functions.

    Example:
    <pre>
    @textblock
    IOCFPluginInterface             **iodev;    // obtained earlier

    IOUSBInterfaceInterface192      **intf;     // fetching this now
    IOReturn                        err;

    err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID192),
                                (LPVoid)&intf);
    @/textblock
    </pre>
 */

#define kIOUSBInterfaceInterfaceID192 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                     0x6C, 0x79, 0x8A, 0x6E, 0xD6, 0xE9, 0x11, 0xD6, \
                                                                     0xAD, 0xD6, 0x00, 0x03, 0x93, 0x3E, 0x3E, 0x3E)

// C63D3C92-0884-11D7-9692-0003933E3E3E
/*!
    @defined kIOUSBInterfaceInterfaceID197
    @discussion This UUID constant is used to obtain an interface interface corresponding to
    an IOUSBInterface user client in the kernel. The type of this device interface is
    IOUSBInterfaceInterface197. This device interface is obtained after the device interface
    for the service itself has been obtained.

    <b>Note:</b> The IOUSBInterfaceInterface197 is returned only by version 1.9.7 or above of
    the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.2.5. If your software
    is running on a version of Mac OS X prior to 10.2.5 you will need to use the UUID kIOUSBInterfaceInterfaceID,
    kIOUSBInterfaceInterfaceID182, kIOUSBInterfaceInterfaceID183, kIOUSBInterfaceInterfaceID190, or
    kIOUSBInterfaceInterfaceID192 and you will not have access to some functions.

    Example:
    <pre>
    @textblock
    IOCFPluginInterface             **iodev;    // obtained earlier

    IOUSBInterfaceInterface197      **intf;     // fetching this now
    IOReturn                        err;

    err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID197),
                                (LPVoid)&intf);
    @/textblock
    </pre>
 */

#define kIOUSBInterfaceInterfaceID197 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                     0xC6, 0x3D, 0x3C, 0x92, 0x08, 0x84, 0x11, 0xD7, \
                                                                     0x96, 0x92, 0x00, 0x03, 0x93, 0x3E, 0x3E, 0x3E)

// 770DE60C-2FE8-11D8-A582-000393DCB1D0
/*!
   @defined kIOUSBInterfaceInterfaceID220
   @discussion This UUID constant is used to obtain an interface interface corresponding to
   an IOUSBInterface user client in the kernel. The type of this device interface is
   IOUSBInterfaceInterface197. This device interface is obtained after the device interface
   for the service itself has been obtained.

   <b>Note:</b> The IOUSBInterfaceInterface220 is returned only by version 2.2.0 or above of
   the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.4. If your software
   is running on a version of Mac OS X prior to 10.4 you will need to use the UUID kIOUSBInterfaceInterfaceID,
   kIOUSBInterfaceInterfaceID182, kIOUSBInterfaceInterfaceID183, kIOUSBInterfaceInterfaceID190, kIOUSBInterfaceInterfaceID192,
   or kIOUSBInterfaceInterfaceID197 and you will not have access to some functions.

   Example:
   <pre>
   @textblock
   IOCFPluginInterface             **iodev;     // obtained earlier

   IOUSBInterfaceInterface220      **intf;     // fetching this now
   IOReturn                        err;

   err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID220),
                                (LPVoid)&intf);
   @/textblock
   </pre>
 */

#define kIOUSBInterfaceInterfaceID220 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                     0x77, 0x0D, 0xE6, 0x0C, 0x2F, 0xE8, 0x11, 0xD8, \
                                                                     0xA5, 0x82, 0x00, 0x03, 0x93, 0xDC, 0xB1, 0xD0)

// 64BABDD2-0F6B-4B4F-8E3E-DC36046987AD
/*!
   @defined kIOUSBInterfaceInterfaceID245
   @discussion This UUID constant is used to obtain an interface interface corresponding to
   an IOUSBInterface user client in the kernel. The type of this device interface is
   IOUSBInterfaceInterface245. This device interface is obtained after the device interface
   for the service itself has been obtained.

   <b>Note:</b> The IOUSBInterfaceInterface245 is returned only by version 2.4.5 or above of
   the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.4.5.  This version
   does not add any new functions.  It is used to allow us to fix a leak in our termination without affecting
   any current drivers:  In previous versions, we would not release a reference to the IOUSBDevice.  For
   IOUSBInterfaceInterfaceID245 clients we will now release that reference.

   Example:
   <pre>
   @textblock
   IOCFPluginInterface             **iodev;     // obtained earlier

   IOUSBInterfaceInterface245      **intf;     // fetching this now
   IOReturn                        err;

   err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID245),
                                (LPVoid)&intf);
   @/textblock
   </pre>
 */

#define kIOUSBInterfaceInterfaceID245 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                     0x64, 0xBA, 0xBD, 0xD2, 0x0F, 0x6B, 0x4B, 0x4F, \
                                                                     0x8E, 0x3E, 0xDC, 0x36, 0x04, 0x69, 0x87, 0xAD)


// BCEAADDC-884D-4F27-8340-36D69FAB90F6
/*!
   @defined kIOUSBInterfaceInterfaceID300
   @discussion This UUID constant is used to obtain an interface interface corresponding to
   an IOUSBInterface user client in the kernel. The type of this device interface is
   IOUSBInterfaceInterface300. This device interface is obtained after the device interface
   for the service itself has been obtained.

   <b>Note:</b> The IOUSBInterfaceInterface300 is returned only by version 3.0.0 or above of
   the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.5.  If your software
   is running on a version of Mac OS X prior to 10.5 you will need to use the UUID kIOUSBInterfaceInterfaceID,
   kIOUSBInterfaceInterfaceID182, kIOUSBInterfaceInterfaceID183, kIOUSBInterfaceInterfaceID190, kIOUSBInterfaceInterfaceID192,
   kIOUSBInterfaceInterfaceID197, kIOUSBInterfaceInterfaceID220, or kIOUSBInterfaceInterfaceID245 and you will not have access to some functions.

   Example:
   <pre>
   @textblock
   IOCFPluginInterface             **iodev;     // obtained earlier

   IOUSBInterfaceInterface300      **intf;     // fetching this now
   IOReturn                        err;

   err = (*iodev)->QueryInterface(iodev,
                                CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID300),
                                (LPVoid)&intf);
   @/textblock
   </pre>
 */

#define kIOUSBInterfaceInterfaceID300 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                     0xBC, 0xEA, 0xAD, 0xDC, 0x88, 0x4D, 0x4F, 0x27, \
                                                                     0x83, 0x40, 0x36, 0xD6, 0x9F, 0xAB, 0x90, 0xF6)

// 56301DF2-72DE-404B-91ED-ED30BF705121
/*!
   @defined kIOUSBInterfaceInterfaceID398
   @discussion This UUID constant is used to obtain an interface interface corresponding to
   an IOUSBInterface user client in the kernel. The type of this device interface is
   IOUSBInterfaceInterface398. This device interface is obtained after the device interface
   for the service itself has been obtained.
 */

#define kIOUSBInterfaceInterfaceID398 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                     0x56, 0x30, 0x1D, 0xF2, 0x72, 0xDE, 0x40, 0x4B, \
                                                                     0x91, 0xED, 0xED, 0x30, 0xBF, 0x70, 0x51, 0x21)

// 2A415A15-1A7B-4B5B-869C-694FEE0C6C6B
/*!
   @defined kIOUSBInterfaceInterfaceID400
   @discussion This UUID constant is used to obtain an interface interface corresponding to
   an IOUSBInterface user client in the kernel. The type of this device interface is
   IOUSBInterfaceInterface400. This device interface is obtained after the device interface
   for the service itself has been obtained.
 */

#define kIOUSBInterfaceInterfaceID400 CFUUIDGetConstantUUIDWithBytes(NULL,                                           \
                                                                    0x2A, 0x41, 0x5A, 0x15, 0x1A, 0x7B, 0x4B, 0x5B,  \
                                                                    0x86, 0x9C, 0x69, 0x4F, 0xEE, 0x0C, 0x6C, 0x6B)

// 6C0D38C3-B093-4EA7-809B-09FB5DDDAC16
/*!
 @defined kIOUSBInterfaceInterfaceID500
 @discussion This UUID constant is used to obtain an interface interface corresponding to
 an IOUSBInterface user client in the kernel. The type of this device interface is
 IOUSBInterfaceInterface500. This device interface is obtained after the device interface
 for the service itself has been obtained.

 <b>Note:</b> The IOUSBInterfaceInterface500 is returned only by version 5.0.0 or above of
 the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.7.3.  If your software
 is running on a version of Mac OS X prior to 10.7.3 you will need to use the UUID kIOUSBInterfaceInterfaceID,
 kIOUSBInterfaceInterfaceID182, kIOUSBInterfaceInterfaceID183, kIOUSBInterfaceInterfaceID190, kIOUSBInterfaceInterfaceID192,
 kIOUSBInterfaceInterfaceID197, kIOUSBInterfaceInterfaceID220, kIOUSBInterfaceInterfaceID245, or kIOUSBInterfaceInterfaceID300 and you will not have access to some functions.

 Example:
 <pre>
 @textblock
 IOCFPluginInterface             **iodev;     // obtained earlier

 IOUSBInterfaceInterface500      **intf;     // fetching this now
 IOReturn                        err;

 err = (*iodev)->QueryInterface(iodev,
 CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID500),
 (LPVoid)&intf);
 @/textblock
 </pre>
 */
#define kIOUSBInterfaceInterfaceID500 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, \
0x6C, 0x0D, 0x38, 0xC3, 0xB0, 0x93, 0x4E, 0xA7,                                             \
0x80, 0x9B, 0x09, 0xFB, 0x5D, 0xDD, 0xAC, 0x16)





// 6AE44D3F-EB45-487F-8E8E-B93B99F8EA9E
/*!
 @defined kIOUSBInterfaceInterfaceID550
 @discussion This UUID constant is used to obtain an interface interface corresponding to
 an IOUSBInterface user client in the kernel. The type of this device interface is
 kIOUSBInterfaceInterfaceID550. This device interface is obtained after the device interface
 for the service itself has been obtained.

 <b>Note:</b> The kIOUSBInterfaceInterfaceID550 is returned only by version 5.5.0 or above of
 the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.7.3.  If your software
 is running on a version of Mac OS X prior to 10.8.x you will need to use the UUID kIOUSBInterfaceInterfaceID,
 kIOUSBInterfaceInterfaceID182, kIOUSBInterfaceInterfaceID183, kIOUSBInterfaceInterfaceID190, kIOUSBInterfaceInterfaceID192,
 kIOUSBInterfaceInterfaceID197, kIOUSBInterfaceInterfaceID220, kIOUSBInterfaceInterfaceID245, kIOUSBInterfaceInterfaceID300, or kIOUSBInterfaceInterfaceID500 and you will not have access to some functions.

 Example:
 <pre>
 @textblock
 IOCFPluginInterface             **iodev;     // obtained earlier

 IOUSBInterfaceInterface550      **intf;     // fetching this now
 IOReturn                        err;

 err = (*iodev)->QueryInterface(iodev,
 CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID550),
 (LPVoid)&intf);
 @/textblock
 </pre>
 */
#define kIOUSBInterfaceInterfaceID550 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, \
0x6A, 0xE4, 0x4D, 0x3F, 0xEB, 0x45, 0x48, 0x7F,                                             \
0x8E, 0x8E, 0xB9, 0x3B, 0x99, 0xF8, 0xEA, 0x9E)





// 08151A89-8081-4087-8F9E-0AFEDFDB5D9F
/*!
 @defined kIOUSBInterfaceInterfaceID650
 @discussion This UUID constant is used to obtain an interface interface corresponding to
 an IOUSBInterface user client in the kernel. The type of this device interface is
 kIOUSBInterfaceInterfaceID650. This device interface is obtained after the device interface
 for the service itself has been obtained.

 <b>Note:</b> The kIOUSBInterfaceInterfaceID650 is returned only by version 650.4.0 or above of
 the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.9.  If your software
 is running on a version of Mac OS X prior to 10.9 you will need to use the UUID kIOUSBInterfaceInterfaceID,
 kIOUSBInterfaceInterfaceID182, kIOUSBInterfaceInterfaceID183, kIOUSBInterfaceInterfaceID190, kIOUSBInterfaceInterfaceID192,
 kIOUSBInterfaceInterfaceID197, kIOUSBInterfaceInterfaceID220, kIOUSBInterfaceInterfaceID245, kIOUSBInterfaceInterfaceID300,
 kIOUSBInterfaceInterfaceID500 or kIOUSBInterfaceInterfaceID550 and you will not have access to some functions.

 Example:
 <pre>
 @textblock
 IOCFPluginInterface             **iodev;     // obtained earlier

 IOUSBInterfaceInterface650      **intf;     // fetching this now
 IOReturn                        err;

 err = (*iodev)->QueryInterface(iodev,
 CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID650),
 (LPVoid)&intf);
 @/textblock
 </pre>
 */
#define kIOUSBInterfaceInterfaceID650 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, \
0x08, 0x15, 0x1A, 0x89, 0x80, 0x81, 0x40, 0x87,                                             \
0x8F, 0x9E, 0x0A, 0xFE, 0xDF, 0xDB, 0x5D, 0x9F)





// 17F9E59C-B0A1-401D-9AC0-8DE27AC6047E
/*!
 @defined kIOUSBInterfaceInterfaceID700
 @discussion This UUID constant is used to obtain an interface interface corresponding to
 an IOUSBInterface user client in the kernel. The type of this device interface is
 kIOUSBInterfaceInterfaceID700. This device interface is obtained after the device interface
 for the service itself has been obtained.

 <b>Note:</b> The kIOUSBInterfaceInterfaceID700 is returned only by version 700.4.0 or above of
 the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.10.  If your software
 is running on a version of Mac OS X prior to 10.10 you will need to use the UUID kIOUSBInterfaceInterfaceID,
 kIOUSBInterfaceInterfaceID182, kIOUSBInterfaceInterfaceID183, kIOUSBInterfaceInterfaceID190, kIOUSBInterfaceInterfaceID192,
 kIOUSBInterfaceInterfaceID197, kIOUSBInterfaceInterfaceID220, kIOUSBInterfaceInterfaceID245, kIOUSBInterfaceInterfaceID300,
 kIOUSBInterfaceInterfaceID500, kIOUSBInterfaceInterfaceID550 or kIOUSBInterfaceInterfaceID650, and you will not have access to some functions.

 Example:
 <pre>
 @textblock
 IOCFPluginInterface             **iodev;     // obtained earlier

 IOUSBInterfaceInterface700      **intf;     // fetching this now
 IOReturn                        err;

 err = (*iodev)->QueryInterface(iodev,
 CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID700),
 (LPVoid)&intf);
 @/textblock
 </pre>
 */

#define kIOUSBInterfaceInterfaceID700 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, \
0x17, 0xF9, 0xE5, 0x9C, 0xB0, 0xA1, 0x40, 0x1D,                                                 \
0x9A, 0xC0, 0x8D, 0xE2, 0x7A, 0xC6, 0x04, 0x7E)

// 33A85DB0-0C3B-4328-8F02-FDA81B117F4C
/*!
 @defined kIOUSBInterfaceInterfaceID800
 @discussion This UUID constant is used to obtain an interface interface corresponding to
 an IOUSBInterface user client in the kernel. The type of this device interface is
 kIOUSBInterfaceInterfaceID800. This device interface is obtained after the device interface
 for the service itself has been obtained.

 <b>Note:</b> The kIOUSBInterfaceInterfaceID800 is returned only by version 900.4.1 or above of
 the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.12.  If your software
 is running on a version of Mac OS X prior to 10.12 you will need to use the UUID kIOUSBInterfaceInterfaceID,
 kIOUSBInterfaceInterfaceID182, kIOUSBInterfaceInterfaceID183, kIOUSBInterfaceInterfaceID190, kIOUSBInterfaceInterfaceID192,
 kIOUSBInterfaceInterfaceID197, kIOUSBInterfaceInterfaceID220, kIOUSBInterfaceInterfaceID245, kIOUSBInterfaceInterfaceID300,
 kIOUSBInterfaceInterfaceID500, kIOUSBInterfaceInterfaceID550 , kIOUSBInterfaceInterfaceID650 or kIOUSBInterfaceInterfaceID700 and you will not have access to some functions.

 Example:
 <pre>
 @textblock
 IOCFPluginInterface             **iodev;     // obtained earlier

 IOUSBInterfaceInterface800      **intf;     // fetching this now
 IOReturn                        err;

 err = (*iodev)->QueryInterface(iodev,
 CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID800),
 (LPVoid)&intf);
 @/textblock
 </pre>
 */

#define kIOUSBInterfaceInterfaceID800 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, \
0x33, 0xA8, 0x5D, 0xB0, 0x0C, 0x3B, 0x43, 0x28,                                                 \
0x8F, 0x02, 0xFD, 0xA8, 0x1B, 0x11, 0x7F, 0x4C)

// 8752663B-C07B-4BAE-9584-22032FAB9C5A
/*!
 @defined kIOUSBInterfaceInterfaceID942
 @discussion This UUID constant is used to obtain an interface interface corresponding to
 an IOUSBInterface user client in the kernel. The type of this device interface is
 kIOUSBInterfaceInterfaceID942. This device interface is obtained after the device interface
 for the service itself has been obtained.

 <b>Note:</b> The kIOUSBInterfaceInterfaceID942 is returned only by version 900.4.2 or above of
 the IOUSBFamily. This version of IOUSBFamily shipped with Mac OS X version 10.14.  If your software
 is running on a version of Mac OS X prior to 10.14 you will need to use the UUID kIOUSBInterfaceInterfaceID,
 kIOUSBInterfaceInterfaceID182, kIOUSBInterfaceInterfaceID183, kIOUSBInterfaceInterfaceID190, kIOUSBInterfaceInterfaceID192,
 kIOUSBInterfaceInterfaceID197, kIOUSBInterfaceInterfaceID220, kIOUSBInterfaceInterfaceID245, kIOUSBInterfaceInterfaceID300,
 kIOUSBInterfaceInterfaceID500, kIOUSBInterfaceInterfaceID550 , kIOUSBInterfaceInterfaceID650, kIOUSBInterfaceInterfaceID700, or kIOUSBInterfaceInterfaceID800 and you will not have access to some functions.

 Example:
 <pre>
 @textblock
 IOCFPluginInterface             **iodev;     // obtained earlier

 IOUSBInterfaceInterface942      **intf;     // fetching this now
 IOReturn                        err;

 err = (*iodev)->QueryInterface(iodev,
 CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID942),
 (LPVoid)&intf);
 @/textblock
 </pre>
 */

#define kIOUSBInterfaceInterfaceID942 CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault,                         \
                                                                     0x87, 0x52, 0x66, 0x3B, 0xC0, 0x7B, 0x4B, 0xAE,    \
                                                                     0x95, 0x84, 0x22, 0x03, 0x2F, 0xAB, 0x9C, 0x5A)

/*!
   @interface IOUSBDeviceInterface
   @abstract   The object you use to access USB devices from user space, returned by all versions of the IOUSBFamily
        currently shipping.
   @discussion The functions listed here will work with any version of the IOUSBDeviceInterface, including
        the one shipped with Mac OS X version 10.0.
 */

typedef struct IOUSBDeviceStruct100
{
    IUNKNOWN_C_GUTS;

    /*!
       @function CreateDeviceAsyncEventSource
       @abstract   Creates a run loop source for delivery of all asynchronous notifications on this device.
       @discussion The Mac OS X kernel does not spawn a thread to callback to the client. Instead it delivers
                completion notifications (see @link //apple_ref/C/instm/IOUSBInterfaceInterface/CreateInterfaceAsyncPort/ CreateInterfaceAsyncPort @/link). This routine
                wraps that port with the appropriate routing code so that the completion notifications can be
                automatically routed through the client's CFRunLoop.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      source Pointer to a CFRunLoopSourceRef to return the newly created run loop event source.
       @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
     */

    IOReturn (* CreateDeviceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);

    /*!
       @function GetDeviceAsyncEventSource
       @abstract   Returns the CFRunLoopSourceRef for this IOService instance.
       @param      self Pointer to the IOUSBDeviceInterface.
       @result     Returns the run loop source if one has been created, 0 otherwise.
     */

    CFRunLoopSourceRef (* GetDeviceAsyncEventSource)(void* self);

    /*!
       @function CreateDeviceAsyncPort
       @abstract   Creates and registers a mach_port_t for asynchronous communications.
       @discussion The Mac OS X kernel does not spawn a thread to callback to the client. Instead it delivers
                completion notifications on this mach port. After receiving a message on this port the
                client is obliged to call the IOKitLib.h IODispatchCalloutFromMessage() function for
                decoding the notification message.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      port Pointer to a mach_port_t to return the newly created port.
       @result     Returns kIOReturnSuccess if successful or a kern_return_t if unsuccessful.
     */

    IOReturn (* CreateDeviceAsyncPort)(void* self, mach_port_t* port);

    /*!
       @function GetDeviceAsyncPort
       @abstract   Returns the mach_port_t port for this IOService instance.
       @param      self Pointer to the IOUSBDeviceInterface.
       @result     Returns the port if one exists, 0 otherwise.
     */

    mach_port_t (* GetDeviceAsyncPort)(void* self);

    /*!
       @function USBDeviceOpen
       @abstract   Opens the IOUSBDevice for exclusive access.
       @discussion Before the client can issue commands that change the state of the device, it
                must have succeeded in opening the device. This establishes an exclusive link
                between the client's task and the actual device.
       @param      self Pointer to the IOUSBDeviceInterface.
       @result     Returns kIOReturnExclusiveAccess if some other task has the device opened already,
                kIOReturnError if the connection with the kernel cannot be established or kIOReturnSuccess if successful.
     */

    IOReturn (* USBDeviceOpen)(void* self);

    /*!
       @function USBDeviceClose
       @abstract   Closes the task's connection to the IOUSBDevice.
       @discussion Releases the client's exclusive access to the IOUSBDevice.
       @param      self Pointer to the IOUSBDeviceInterface.
       @result     Returns kIOReturnSuccess if successful, some other mach error if the connection is no longer valid.
     */

    IOReturn (* USBDeviceClose)(void* self);

    /*!
       @function GetDeviceClass
       @abstract   Returns the USB Class (bDeviceClass) of the device.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      devClass Pointer to UInt8 to hold the device Class.
       @result      Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceClass)(void* self, UInt8* devClass);

    /*!
       @function GetDeviceSubClass
       @abstract   Returns the USB Subclass (bDeviceSubClass) of the device.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      devSubClass Pointer to UInt8 to hold the device Subclass.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceSubClass)(void* self, UInt8* devSubClass);

    /*!
       @function GetDeviceProtocol
       @abstract   Returns the USB Protocol (bDeviceProtocol) of the interface.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      devProtocol Pointer to UInt8 to hold the device Protocol.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceProtocol)(void* self, UInt8* devProtocol);

    /*!
       @function GetDeviceVendor
       @abstract   Returns the USB Vendor ID (idVendor) of the device.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      devVendor   Pointer to UInt16 to hold the vendorID.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);

    /*!
       @function GetDeviceProduct
       @abstract    Returns the USB Product ID (idProduct) of the device.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      devProduct  Pointer to UInt16 to hold the ProductID.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);

    /*!
       @function GetDeviceReleaseNumber
       @abstract   Returns the Device Release Number (bcdDevice) of the device.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      devRelNum   Pointer to UInt16 to hold the Device Release Number.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);

    /*!
       @function GetDeviceAddress
       @abstract   Returns the address of the device on its bus.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      addr    Pointer to USBDeviceAddress to hold the result.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceAddress)(void* self, USBDeviceAddress* addr);

    /*!
       @function GetDeviceBusPowerAvailable
       @abstract   Returns the power available to the device.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      powerAvailable Pointer to UInt32 to hold the power available (in 2 mA increments).
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceBusPowerAvailable)(void* self, UInt32* powerAvailable);

    /*!
       @function GetDeviceSpeed
       @abstract   Returns the speed of the device.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      devSpeed Pointer to UInt8 to hold the speed (kUSBDeviceSpeedLow, kUSBDeviceSpeedFull, kUSBDeviceSpeedHigh, or kUSBDeviceSpeedSuper).
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceSpeed)(void* self, UInt8* devSpeed);

    /*!
       @function GetNumberOfConfigurations
       @abstract   Returns the number of supported configurations in this device.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      numConfig Pointer to UInt8 to hold the number of configurations.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetNumberOfConfigurations)(void* self, UInt8* numConfig);

    /*!
       @function GetLocationID
       @abstract   Returns the location ID.
       @discussion The location ID is a 32 bit number which is unique among all USB devices in the system, and
                which will not change on a system reboot unless the topology of the bus itself changes. The
                device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      locationID Pointer to UInt32 to hold the location ID.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetLocationID)(void* self, UInt32* locationID);

    /*!
       @function GetConfigurationDescriptorPtr
       @abstract   Returns a pointer to a configuration descriptor for a given index.
       @discussion Note that this will point to the data as received from the USB bus and hence will be in USB bus
                order (i.e. little endian).  The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      configIndex The index (zero based) of the desired config descriptor.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetConfigurationDescriptorPtr)(void* self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr* desc);

    /*!
       @function GetConfiguration
       @abstract   Returns the currently selected configuration in the device.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      configNum Pointer to UInt8 to hold the configuration value.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetConfiguration)(void* self, UInt8* configNum);

    /*!
       @function SetConfiguration
       @abstract   Sets the configuration in the device.
       @discussion Note that setting the configuration causes any existing IOUSBInterface objects attached to the
                IOUSBDevice to be destroyed, and all of the interfaces in the new configuration to be instantiated
                as new IOUSBInterface objects.  The device must be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      configNum The value of the desired configuration (from IOUSBConfigurationDescriptor.bConfigurationValue).
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the device is not open for exclusive access.
     */

    IOReturn (* SetConfiguration)(void* self, UInt8 configNum);

    /*!
       @function GetBusFrameNumber
       @abstract   Gets the current frame number of the bus to which the device is attached.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      frame Pointer to UInt64 to hold the frame number.
       @param      atTime Pointer to a returned AbsoluteTime, which is the system time ("wall time") when the frame number register was read. This
                                system time could be the time at the beginning, middle, or end of the given frame.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);

    /*!
       @function ResetDevice
       @abstract   Tells the IOUSBFamily to issue a reset to the device.
       @discussion It will not reenumerate the device, which means that the cached device descriptor values will not
                be updated after the reset. (If you want the IOUSBFamily to reload the cached values, use the call
                USBDeviceReEnumerate). Prior to version 1.8.5 of the IOUSBFamily, this call also sent a message to
                all clients of the IOUSBDevice (IOUSBInterfaces and their drivers).  The device must be open to use
                this function.

                This behavior was eliminated in version 1.8.5 of the IOUSBFamily.
       @param      self Pointer to the IOUSBDeviceInterface.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the device is not open for exclusive access.
     */

    IOReturn (* ResetDevice)(void* self);

    /*!
       @function DeviceRequest
       @abstract   Sends a USB request on the default control pipe.
       @discussion The device does not have to be open to use this function, but standard requests that change the state of the device
                   may require that the device is opened first.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      req Pointer to an IOUSBDevRequest containing the request.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                                kIOReturnAborted if the thread is interrupted before the call completes,
                or kIOReturnNotOpen if the device is not open for exclusive access.
     */

    IOReturn (* DeviceRequest)(void* self, IOUSBDevRequest* req);

    /*!
       @function DeviceRequestAsync
       @abstract   Sends an asynchronous USB request on the default control pipe.
       @discussion The device does not have to be open to use this function, but standard requests that change the state of the device
                   may require that the device is opened first.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      req Pointer to an IOUSBDevRequest containing the request.
       @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the number of bytes that were actually transferred.  A message addressed to this callback is posted to the Async port upon completion.
       @param      refCon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                kIOReturnNotOpen if the device is not open for exclusive access, or kIOUSBNoAsyncPortErr if no Async
                port has been created for this interface.
     */

    IOReturn (* DeviceRequestAsync)(void* self, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);

    /*!
       @function CreateInterfaceIterator
       @abstract   Creates an iterator to iterate over some or all of the interfaces of a device.
       @discussion The device does not have to be open to use this function.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      req Pointer an IOUSBFindInterfaceRequest structure describing the desired interfaces.
       @param      iter Pointer to a an io_iterator_t to contain the new iterator.
       @result     Returns kIOReturnSuccess if successful or kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* CreateInterfaceIterator)(void* self, IOUSBFindInterfaceRequest* req, io_iterator_t* iter);

} IOUSBDeviceInterface100;

/*!
   @interface IOUSBDeviceInterface182
   @abstract   The object you use to access USB devices from user space, returned by the IOUSBFamily version
        1.8.2 and above.
   @discussion The functions listed here include all of the functions defined for the IOUSBDeviceInterface and
        some new functions that are available on Mac OS X version 10.0.4 and later.
   @super IOUSBDeviceInterface
 */

typedef struct IOUSBDeviceStruct182
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateDeviceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetDeviceAsyncEventSource)(void* self);
    IOReturn (* CreateDeviceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetDeviceAsyncPort)(void* self);
    IOReturn (* USBDeviceOpen)(void* self);
    IOReturn (* USBDeviceClose)(void* self);
    IOReturn (* GetDeviceClass)(void* self, UInt8* devClass);
    IOReturn (* GetDeviceSubClass)(void* self, UInt8* devSubClass);
    IOReturn (* GetDeviceProtocol)(void* self, UInt8* devProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetDeviceAddress)(void* self, USBDeviceAddress* addr);
    IOReturn (* GetDeviceBusPowerAvailable)(void* self, UInt32* powerAvailable);
    IOReturn (* GetDeviceSpeed)(void* self, UInt8* devSpeed);
    IOReturn (* GetNumberOfConfigurations)(void* self, UInt8* numConfig);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetConfigurationDescriptorPtr)(void* self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr* desc);
    IOReturn (* GetConfiguration)(void* self, UInt8* configNum);
    IOReturn (* SetConfiguration)(void* self, UInt8 configNum);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ResetDevice)(void* self);
    IOReturn (* DeviceRequest)(void* self, IOUSBDevRequest* req);
    IOReturn (* DeviceRequestAsync)(void* self, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* CreateInterfaceIterator)(void* self, IOUSBFindInterfaceRequest* req, io_iterator_t* iter);

    /*!
       @function USBDeviceOpenSeize
       @abstract  Opens the IOUSBDevice for exclusive access.
       @discussion This function opens the IOUSBDevice for exclusive access. If another client
                has the device opened, an attempt is made to get that client to close it before
                returning.  Before the client can issue commands that change the state of the device,
                it must have succeeded in opening the device. This establishes an exclusive
                link between the client's task and the actual device.
       @availability This function is only available with IOUSBDeviceInterface182 and above.
       @param      self Pointer to the IOUSBDeviceInterface.
       @result     Returns kIOReturnExclusiveAccess if some other task has the device opened already and refuses
                to close it, kIOReturnError if the connection with the kernel can not be established or kIOReturnSuccess if successful.
     */

    IOReturn (* USBDeviceOpenSeize)(void* self);

    /*!
       @function DeviceRequestTO
       @abstract   Sends a USB request on the default control pipe.
       @discussion The device does not have to be open to use this function, but standard requests that change the state of the device may require that the device is opened first. This function sends a USB request on the default control pipe. The IOUSBDevRequestTO structure allows the client to specify timeout values for this request.

       @availability This function is only available with IOUSBDeviceInterface182 and above.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      req Pointer to an IOUSBDevRequestTO containing the request.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                                kIOReturnAborted if the thread is interrupted before the call completes,
               or kIOReturnNotOpen if the device is not open for exclusive access.
     */

    IOReturn (* DeviceRequestTO)(void* self, IOUSBDevRequestTO* req);

    /*!
       @function DeviceRequestAsyncTO
       @abstract   Sends an asynchronous USB request on the default control pipe.
       @discussion The device does not have to be open to use this function, but standard requests that change the state of the device may require that the device is opened first. This function sends an asynchronous USB request on the default control pipe.  The IOUSBDevRequestTO structure allows the client to specify timeout values for this request.
       @availability This function is only available with IOUSBDeviceInterface182 and above.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      req Pointer to an IOUSBDevRequestTO containing the request.
     @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the number of bytes that were actually transferred
     in the DeviceRequest. A message addressed to this callback is posted to the
                Async port upon completion.
       @param      refCon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                kIOReturnNotOpen if the device is not open for exclusive access, orkIOUSBNoAsyncPortErr if no Async
                port has been created for this interface.
     */

    IOReturn (* DeviceRequestAsyncTO)(void* self, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);

    /*!
       @function USBDeviceSuspend
       @abstract   Tells the USB Family to either suspend or resume the port to which a device is attached.
       @discussion The device must be open to use this function.
       @availability This function is only available with IOUSBDeviceInterface182 and above.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      suspend TRUE to cause the port to be suspended, FALSE to cause it to be resumed.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the device is not open for exclusive access.
     */

    IOReturn (* USBDeviceSuspend)(void* self, Boolean suspend);

    /*!
       @function USBDeviceAbortPipeZero
       @abstract   Aborts a transaction on the default control pipe.
       @discussion The device does not have to be open to use this function.
       @availability This function is only available with IOUSBDeviceInterface182 and above.
       @param      self Pointer to the IOUSBDeviceInterface.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the device is not open for exclusive access.
     */

    IOReturn (* USBDeviceAbortPipeZero)(void* self);

    /*!
       @function USBGetManufacturerStringIndex
       @abstract   Returns the manufacturer string index in the device descriptor.
       @discussion The device does not have to be open to use this function.
       @availability This function is only available with IOUSBDeviceInterface182 and above.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      msi Pointer to UInt8 to hold the string index.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* USBGetManufacturerStringIndex)(void* self, UInt8* msi);

    /*!
       @function USBGetProductStringIndex
       @abstract   Returns the product string index in the device descriptor.
       @discussion The device does not have to be open to use this function.
       @availability This function is only available with IOUSBDeviceInterface182 and above.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      psi Pointer to UInt8 to hold the string index.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* USBGetProductStringIndex)(void* self, UInt8* psi);

    /*!
       @function USBGetSerialNumberStringIndex
       @abstract   Returns the serial number string index in the device descriptor.
       @discussion The device does not have to be open to use this function.
       @availability This function is only available with IOUSBDeviceInterface182 and above.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      snsi Pointer to UInt8 to hold the string index.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* USBGetSerialNumberStringIndex)(void* self, UInt8* snsi);

} IOUSBDeviceInterface182;

/*!
   @interface IOUSBDeviceInterface187
   @abstract   The object you use to access USB devices from user space, returned by the IOUSBFamily version
        10.8.7 and above.
   @discussion The functions listed here include all of the functions defined for the IOUSBDeviceInterface,
        IOUSBDeviceInterface182, and some new functions that are available on Mac OS X version 10.1.2 and later.
   @super IOUSBDeviceInterface182
 */


typedef struct IOUSBDeviceStruct187
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateDeviceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetDeviceAsyncEventSource)(void* self);
    IOReturn (* CreateDeviceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetDeviceAsyncPort)(void* self);
    IOReturn (* USBDeviceOpen)(void* self);
    IOReturn (* USBDeviceClose)(void* self);
    IOReturn (* GetDeviceClass)(void* self, UInt8* devClass);
    IOReturn (* GetDeviceSubClass)(void* self, UInt8* devSubClass);
    IOReturn (* GetDeviceProtocol)(void* self, UInt8* devProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetDeviceAddress)(void* self, USBDeviceAddress* addr);
    IOReturn (* GetDeviceBusPowerAvailable)(void* self, UInt32* powerAvailable);
    IOReturn (* GetDeviceSpeed)(void* self, UInt8* devSpeed);
    IOReturn (* GetNumberOfConfigurations)(void* self, UInt8* numConfig);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetConfigurationDescriptorPtr)(void* self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr* desc);
    IOReturn (* GetConfiguration)(void* self, UInt8* configNum);
    IOReturn (* SetConfiguration)(void* self, UInt8 configNum);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ResetDevice)(void* self);
    IOReturn (* DeviceRequest)(void* self, IOUSBDevRequest* req);
    IOReturn (* DeviceRequestAsync)(void* self, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* CreateInterfaceIterator)(void* self, IOUSBFindInterfaceRequest* req, io_iterator_t* iter);
    IOReturn (* USBDeviceOpenSeize)(void* self);
    IOReturn (* DeviceRequestTO)(void* self, IOUSBDevRequestTO* req);
    IOReturn (* DeviceRequestAsyncTO)(void* self, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* USBDeviceSuspend)(void* self, Boolean suspend);
    IOReturn (* USBDeviceAbortPipeZero)(void* self);
    IOReturn (* USBGetManufacturerStringIndex)(void* self, UInt8* msi);
    IOReturn (* USBGetProductStringIndex)(void* self, UInt8* psi);
    IOReturn (* USBGetSerialNumberStringIndex)(void* self, UInt8* snsi);

    /*!
     @function USBDeviceReEnumerate
     @abstract   Tells the IOUSBFamily to reenumerate the device.
     @discussion This function will send a terminate message to all clients of the IOUSBDevice (such as
     IOUSBInterfaces and their drivers, as well as the current User Client), emulating an unplug
     of the device. The IOUSBFamily will then enumerate the device as if it had just
     been plugged in. This call should be used by clients wishing to take advantage
     of the Device Firmware Update Class specification.  The device must be open to use this function, except when you are passing the kUSBReEnumerateCaptureDeviceBit or
     kUSBReEnumerateReleaseDeviceBit options.  In those cases you either need to (1) have the "com.apple.vm.device-access" entitlement set and the IOUSBDevice needs to have successfully been authorized by
     the IOKit's IOServiceAuthorize() APIs or (2) run with root privileges.
     @availability This function is only available with IOUSBDeviceInterface187 and above.
     @param      self Pointer to the IOUSBDeviceInterface.
     @param      options A UInt32 with a bit mask of options.  See USB.h and the USBReEnumerateOptions enum.  If the kUSBReEnumerateCaptureDeviceBit is used
     the client needs to either (1) have the "com.apple.vm.device-access" entitlement set and the IOUSBDevice needs to have successfully been authorized by
     the IOKit's IOServiceAuthorize() APIs or (2) run with root privileges.  Using that bit will terminate any kernel drivers for all non-mass storage interfaces
     attached to the device, as well as for any kernel driver that is attached to the device.  Specifying the kUSBReEnumerateReleaseDeviceBit will cause the IOUSBDevice to
     be returned to the OS and the driver for that device to be reloaded.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
     or kIOReturnNotOpen if the device is not open for exclusive access.
     */

    IOReturn (* USBDeviceReEnumerate)(void* self, UInt32 options);
} IOUSBDeviceInterface187;

/*!
   @interface IOUSBDeviceInterface197
   @abstract   The object you use to access USB devices from user space, returned by the IOUSBFamily version
        1.9.7 and above.
   @discussion The functions listed here include all of the functions defined for the IOUSBDeviceInterface,
        IOUSBDeviceInterface182, IOUSBDeviceInterface187, and some new functions that are available
        on Mac OS X version 10.2.3 and later.
   @super IOUSBDeviceInterface187
 */


typedef struct IOUSBDeviceStruct197
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateDeviceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetDeviceAsyncEventSource)(void* self);
    IOReturn (* CreateDeviceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetDeviceAsyncPort)(void* self);
    IOReturn (* USBDeviceOpen)(void* self);
    IOReturn (* USBDeviceClose)(void* self);
    IOReturn (* GetDeviceClass)(void* self, UInt8* devClass);
    IOReturn (* GetDeviceSubClass)(void* self, UInt8* devSubClass);
    IOReturn (* GetDeviceProtocol)(void* self, UInt8* devProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetDeviceAddress)(void* self, USBDeviceAddress* addr);
    IOReturn (* GetDeviceBusPowerAvailable)(void* self, UInt32* powerAvailable);
    IOReturn (* GetDeviceSpeed)(void* self, UInt8* devSpeed);
    IOReturn (* GetNumberOfConfigurations)(void* self, UInt8* numConfig);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetConfigurationDescriptorPtr)(void* self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr* desc);
    IOReturn (* GetConfiguration)(void* self, UInt8* configNum);
    IOReturn (* SetConfiguration)(void* self, UInt8 configNum);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ResetDevice)(void* self);
    IOReturn (* DeviceRequest)(void* self, IOUSBDevRequest* req);
    IOReturn (* DeviceRequestAsync)(void* self, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* CreateInterfaceIterator)(void* self, IOUSBFindInterfaceRequest* req, io_iterator_t* iter);
    IOReturn (* USBDeviceOpenSeize)(void* self);
    IOReturn (* DeviceRequestTO)(void* self, IOUSBDevRequestTO* req);
    IOReturn (* DeviceRequestAsyncTO)(void* self, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* USBDeviceSuspend)(void* self, Boolean suspend);
    IOReturn (* USBDeviceAbortPipeZero)(void* self);
    IOReturn (* USBGetManufacturerStringIndex)(void* self, UInt8* msi);
    IOReturn (* USBGetProductStringIndex)(void* self, UInt8* psi);
    IOReturn (* USBGetSerialNumberStringIndex)(void* self, UInt8* snsi);
    IOReturn (* USBDeviceReEnumerate)(void* self, UInt32 options);

    /*!
       @function GetBusMicroFrameNumber
       @abstract   Gets the current micro frame number of the bus to which the device is attached.
       @discussion The device does not have to be open to use this function.
       @availability This function is only available with IOUSBDeviceInterface197 and above.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      microFrame Pointer to UInt64 to hold the microframe number.
       @param      atTime Pointer to an AbsoluteTime, which should be within 1ms of the time when the bus
                frame number was acquired.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetBusMicroFrameNumber)(void* self, UInt64* microFrame, AbsoluteTime* atTime);

    /*!
       @function GetIOUSBLibVersion
       @abstract   Returns the version of the IOUSBLib and the version of the IOUSBFamily.
       @discussion The device does not have to be open to use this function.
       @availability This function is only available with IOUSBDeviceInterface197 and above.
       @param      self Pointer to the IOUSBDeviceInterface.
       @param      ioUSBLibVersion Pointer to a NumVersion structure that on return will contain the version of
                the IOUSBLib.
       @param      usbFamilyVersion Pointer to a NumVersion structure that on return will contain the version of
                the IOUSBFamily.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetIOUSBLibVersion)(void* self, NumVersion* ioUSBLibVersion, NumVersion* usbFamilyVersion);
} IOUSBDeviceInterface197;


/*!
   @interface IOUSBDeviceInterface245
   @abstract   The object you use to access USB devices from user space, returned by the IOUSBFamily version
   2.4.5 and above.
   @discussion The functions listed here include all of the functions defined for the IOUSBDeviceInterface,
   IOUSBDeviceInterface182, IOUSBDeviceInterface187, IOUSBDeviceInterface197, and some new functions that are available
   on Mac OS X version 10.2.3 and later.
   @super IOUSBDeviceInterface197
 */


typedef struct IOUSBDeviceStruct245
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateDeviceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetDeviceAsyncEventSource)(void* self);
    IOReturn (* CreateDeviceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetDeviceAsyncPort)(void* self);
    IOReturn (* USBDeviceOpen)(void* self);
    IOReturn (* USBDeviceClose)(void* self);
    IOReturn (* GetDeviceClass)(void* self, UInt8* devClass);
    IOReturn (* GetDeviceSubClass)(void* self, UInt8* devSubClass);
    IOReturn (* GetDeviceProtocol)(void* self, UInt8* devProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetDeviceAddress)(void* self, USBDeviceAddress* addr);
    IOReturn (* GetDeviceBusPowerAvailable)(void* self, UInt32* powerAvailable);
    IOReturn (* GetDeviceSpeed)(void* self, UInt8* devSpeed);
    IOReturn (* GetNumberOfConfigurations)(void* self, UInt8* numConfig);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetConfigurationDescriptorPtr)(void* self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr* desc);
    IOReturn (* GetConfiguration)(void* self, UInt8* configNum);
    IOReturn (* SetConfiguration)(void* self, UInt8 configNum);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ResetDevice)(void* self);
    IOReturn (* DeviceRequest)(void* self, IOUSBDevRequest* req);
    IOReturn (* DeviceRequestAsync)(void* self, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* CreateInterfaceIterator)(void* self, IOUSBFindInterfaceRequest* req, io_iterator_t* iter);
    IOReturn (* USBDeviceOpenSeize)(void* self);
    IOReturn (* DeviceRequestTO)(void* self, IOUSBDevRequestTO* req);
    IOReturn (* DeviceRequestAsyncTO)(void* self, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* USBDeviceSuspend)(void* self, Boolean suspend);
    IOReturn (* USBDeviceAbortPipeZero)(void* self);
    IOReturn (* USBGetManufacturerStringIndex)(void* self, UInt8* msi);
    IOReturn (* USBGetProductStringIndex)(void* self, UInt8* psi);
    IOReturn (* USBGetSerialNumberStringIndex)(void* self, UInt8* snsi);
    IOReturn (* USBDeviceReEnumerate)(void* self, UInt32 options);
    IOReturn (* GetBusMicroFrameNumber)(void* self, UInt64* microFrame, AbsoluteTime* atTime);
    IOReturn (* GetIOUSBLibVersion)(void* self, NumVersion* ioUSBLibVersion, NumVersion* usbFamilyVersion);
} IOUSBDeviceInterface245;



/*!
   @interface IOUSBDeviceInterface300
   @abstract   The object you use to access USB devices from user space, returned by the IOUSBFamily version 3.0.0 and above.
   @discussion The functions listed here include all of the functions defined for the IOUSBDeviceInterface,
   IOUSBDeviceInterface182, IOUSBDeviceInterface187, IOUSBDeviceInterface197, IOUSBDeviceInterface245,
   and some new functions that are available on Mac OS X version 10.5 and later.
   @super IOUSBDeviceInterface245
 */


typedef struct IOUSBDeviceStruct300
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateDeviceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetDeviceAsyncEventSource)(void* self);
    IOReturn (* CreateDeviceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetDeviceAsyncPort)(void* self);
    IOReturn (* USBDeviceOpen)(void* self);
    IOReturn (* USBDeviceClose)(void* self);
    IOReturn (* GetDeviceClass)(void* self, UInt8* devClass);
    IOReturn (* GetDeviceSubClass)(void* self, UInt8* devSubClass);
    IOReturn (* GetDeviceProtocol)(void* self, UInt8* devProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetDeviceAddress)(void* self, USBDeviceAddress* addr);
    IOReturn (* GetDeviceBusPowerAvailable)(void* self, UInt32* powerAvailable);
    IOReturn (* GetDeviceSpeed)(void* self, UInt8* devSpeed);
    IOReturn (* GetNumberOfConfigurations)(void* self, UInt8* numConfig);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetConfigurationDescriptorPtr)(void* self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr* desc);
    IOReturn (* GetConfiguration)(void* self, UInt8* configNum);
    IOReturn (* SetConfiguration)(void* self, UInt8 configNum);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ResetDevice)(void* self);
    IOReturn (* DeviceRequest)(void* self, IOUSBDevRequest* req);
    IOReturn (* DeviceRequestAsync)(void* self, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* CreateInterfaceIterator)(void* self, IOUSBFindInterfaceRequest* req, io_iterator_t* iter);
    IOReturn (* USBDeviceOpenSeize)(void* self);
    IOReturn (* DeviceRequestTO)(void* self, IOUSBDevRequestTO* req);
    IOReturn (* DeviceRequestAsyncTO)(void* self, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* USBDeviceSuspend)(void* self, Boolean suspend);
    IOReturn (* USBDeviceAbortPipeZero)(void* self);
    IOReturn (* USBGetManufacturerStringIndex)(void* self, UInt8* msi);
    IOReturn (* USBGetProductStringIndex)(void* self, UInt8* psi);
    IOReturn (* USBGetSerialNumberStringIndex)(void* self, UInt8* snsi);
    IOReturn (* USBDeviceReEnumerate)(void* self, UInt32 options);
    IOReturn (* GetBusMicroFrameNumber)(void* self, UInt64* microFrame, AbsoluteTime* atTime);
    IOReturn (* GetIOUSBLibVersion)(void* self, NumVersion* ioUSBLibVersion, NumVersion* usbFamilyVersion);

    /*!
         @function GetBusFrameNumberWithTime
         @abstract   Gets a recent frame number of the bus to which the device is attached, along with a system time corresponding to the start of that frame
         @discussion The device does not have to be open to use this function.
         @availability This function is only available with IOUSBDeviceInterface300 and above.
         @param      self Pointer to the IOUSBDeviceInterface.
         @param      frame Pointer to UInt64 to hold the frame number.
         @param      atTime Pointer to a returned AbsoluteTime, which is the system time ("wall time") as close as possible to the beginning of that USB frame. The jitter on this value may be as much as 200 microseconds.
         @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or kIOReturnUnsupported is the bus doesn't support this function.
     */

    IOReturn (* GetBusFrameNumberWithTime)(void* self, UInt64* frame, AbsoluteTime* atTime);
} IOUSBDeviceInterface300;

/*!
   @interface IOUSBDeviceInterface320
   @abstract   The object you use to access USB devices from user space, returned by the IOUSBFamily version 3.2.0 and above.
   @discussion The functions listed here include all of the functions defined for the IOUSBDeviceInterface,
   IOUSBDeviceInterface182, IOUSBDeviceInterface187, IOUSBDeviceInterface197, IOUSBDeviceInterface245, or IOUSBDeviceInterface300
   and some new functions that are available on Mac OS X version 10.5.4 and later.
   @super IOUSBDeviceInterface300
 */


typedef struct IOUSBDeviceStruct320
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateDeviceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetDeviceAsyncEventSource)(void* self);
    IOReturn (* CreateDeviceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetDeviceAsyncPort)(void* self);
    IOReturn (* USBDeviceOpen)(void* self);
    IOReturn (* USBDeviceClose)(void* self);
    IOReturn (* GetDeviceClass)(void* self, UInt8* devClass);
    IOReturn (* GetDeviceSubClass)(void* self, UInt8* devSubClass);
    IOReturn (* GetDeviceProtocol)(void* self, UInt8* devProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetDeviceAddress)(void* self, USBDeviceAddress* addr);
    IOReturn (* GetDeviceBusPowerAvailable)(void* self, UInt32* powerAvailable);
    IOReturn (* GetDeviceSpeed)(void* self, UInt8* devSpeed);
    IOReturn (* GetNumberOfConfigurations)(void* self, UInt8* numConfig);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetConfigurationDescriptorPtr)(void* self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr* desc);
    IOReturn (* GetConfiguration)(void* self, UInt8* configNum);
    IOReturn (* SetConfiguration)(void* self, UInt8 configNum);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ResetDevice)(void* self);
    IOReturn (* DeviceRequest)(void* self, IOUSBDevRequest* req);
    IOReturn (* DeviceRequestAsync)(void* self, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* CreateInterfaceIterator)(void* self, IOUSBFindInterfaceRequest* req, io_iterator_t* iter);
    IOReturn (* USBDeviceOpenSeize)(void* self);
    IOReturn (* DeviceRequestTO)(void* self, IOUSBDevRequestTO* req);
    IOReturn (* DeviceRequestAsyncTO)(void* self, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* USBDeviceSuspend)(void* self, Boolean suspend);
    IOReturn (* USBDeviceAbortPipeZero)(void* self);
    IOReturn (* USBGetManufacturerStringIndex)(void* self, UInt8* msi);
    IOReturn (* USBGetProductStringIndex)(void* self, UInt8* psi);
    IOReturn (* USBGetSerialNumberStringIndex)(void* self, UInt8* snsi);
    IOReturn (* USBDeviceReEnumerate)(void* self, UInt32 options);
    IOReturn (* GetBusMicroFrameNumber)(void* self, UInt64* microFrame, AbsoluteTime* atTime);
    IOReturn (* GetIOUSBLibVersion)(void* self, NumVersion* ioUSBLibVersion, NumVersion* usbFamilyVersion);
    IOReturn (* GetBusFrameNumberWithTime)(void* self, UInt64* frame, AbsoluteTime* atTime);

    /*!
         @function GetUSBDeviceInformation
         @abstract      Returns status information about the USB device, such as whether the device is captive or whether it is in the suspended state.
         @discussion The device does not have to be open to use this function.
         @availability This function is only available with IOUSBDeviceInterface320 and above.
         @param      self Pointer to the IOUSBDeviceInterface.
         @param      info Pointer to a buffer that returns a bit field of information on the device (see the USBDeviceInformationBits in USB.h).
         @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or kIOReturnUnsupported is the bus doesn't support this function.
     */
    IOReturn (* GetUSBDeviceInformation)(void* self, UInt32* info);

    /*!
       @function RequestExtraPower
       @abstract				Clients can use this API to reserve extra power for use by this device while the machine is asleep or while it is awake.  Units are milliAmps (mA).
       @discussion			The device has to be open to use this function.
       @availability			This function is only available with IOUSBDeviceInterface320 and above.
       @param self			Pointer to the IOUSBDeviceInterface.
       @param type			Indicates whether the power is to be used during wake or sleep (One of kUSBPowerDuringSleep or kUSBPowerDuringWake)
       @param requestedPower    Amount of power desired, in mA
       @param powerAvailable    Amount of power that was reserved, in mA
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or kIOReturnUnsupported is the bus doesn't support this function.
     */
    IOReturn (* RequestExtraPower)(void* self, UInt32 type, UInt32 requestedPower, UInt32* powerAvailable);

    /*!
       @function ReturnExtraPower
       @abstract				Clients can use this API to tell the system that they will not use power that was previously reserved by using the RequestExtraPower API.
       @discussion			The device has to be open to use this function.
       @availability			This function is only available with IOUSBDeviceInterface320 and above.
       @param      self		Pointer to the IOUSBDeviceInterface.
       @param type			Indicates whether the power is to be used during wake or sleep (One of kUSBPowerDuringSleep or kUSBPowerDuringWake)
       @param powerReturned     Amount of power to be returned, in mA.
       @result				If the returnedPower was not previously allocated, an error will be returned.  This will include the case for power that was requested for sleep but was returned for wake. Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */
    IOReturn (* ReturnExtraPower)(void* self, UInt32 type, UInt32 powerReturned);

    /*!
       @function GetExtraPowerAllocated
       @abstract				Clients can use this API to ask how much extra power has already been reserved by this device.  Units are milliAmps (mA).
       @discussion			The device has to be open to use this function.
       @availability			This function is only available with IOUSBDeviceInterface320 and above.
       @param      self		Pointer to the IOUSBDeviceInterface.
       @param type			Indicates whether the allocated power was to be used during wake or sleep (One of kUSBPowerDuringSleep or kUSBPowerDuringWake)
       @param powerAllocated    Amount of power to be returned, in mA.
       @result				Value returned can be 0 if no power has been allocated. Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */
    IOReturn (* GetExtraPowerAllocated)(void* self, UInt32 type, UInt32* powerAllocated);

} IOUSBDeviceInterface320;

/*!
 @interface IOUSBDeviceInterface400
 @abstract   The object you use to access USB devices from user space, returned by the IOUSBFamily version 4.0.0 and above.
 @discussion This object is functionally identical to IOUSBDeviceInterface320 on macOS, and includes some new functions that are only available on iOS.
 @super IOUSBDeviceInterface320
 */
typedef struct IOUSBDeviceStruct400
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateDeviceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetDeviceAsyncEventSource)(void* self);
    IOReturn (* CreateDeviceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetDeviceAsyncPort)(void* self);
    IOReturn (* USBDeviceOpen)(void* self);
    IOReturn (* USBDeviceClose)(void* self);
    IOReturn (* GetDeviceClass)(void* self, UInt8* devClass);
    IOReturn (* GetDeviceSubClass)(void* self, UInt8* devSubClass);
    IOReturn (* GetDeviceProtocol)(void* self, UInt8* devProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetDeviceAddress)(void* self, USBDeviceAddress* addr);
    IOReturn (* GetDeviceBusPowerAvailable)(void* self, UInt32* powerAvailable);
    IOReturn (* GetDeviceSpeed)(void* self, UInt8* devSpeed);
    IOReturn (* GetNumberOfConfigurations)(void* self, UInt8* numConfig);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetConfigurationDescriptorPtr)(void* self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr* desc);
    IOReturn (* GetConfiguration)(void* self, UInt8* configNum);
    IOReturn (* SetConfiguration)(void* self, UInt8 configNum);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ResetDevice)(void* self);
    IOReturn (* DeviceRequest)(void* self, IOUSBDevRequest* req);
    IOReturn (* DeviceRequestAsync)(void* self, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* CreateInterfaceIterator)(void* self, IOUSBFindInterfaceRequest* req, io_iterator_t* iter);
    IOReturn (* USBDeviceOpenSeize)(void* self);
    IOReturn (* DeviceRequestTO)(void* self, IOUSBDevRequestTO* req);
    IOReturn (* DeviceRequestAsyncTO)(void* self, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* USBDeviceSuspend)(void* self, Boolean suspend);
    IOReturn (* USBDeviceAbortPipeZero)(void* self);
    IOReturn (* USBGetManufacturerStringIndex)(void* self, UInt8* msi);
    IOReturn (* USBGetProductStringIndex)(void* self, UInt8* psi);
    IOReturn (* USBGetSerialNumberStringIndex)(void* self, UInt8* snsi);
    IOReturn (* USBDeviceReEnumerate)(void* self, UInt32 options);
    IOReturn (* GetBusMicroFrameNumber)(void* self, UInt64* microFrame, AbsoluteTime* atTime);
    IOReturn (* GetIOUSBLibVersion)(void* self, NumVersion* ioUSBLibVersion, NumVersion* usbFamilyVersion);
    IOReturn (* GetBusFrameNumberWithTime)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* GetUSBDeviceInformation)(void* self, UInt32* info);
    IOReturn (* RequestExtraPower)(void* self, UInt32 type, UInt32 requestedPower, UInt32* powerAvailable);
    IOReturn (* ReturnExtraPower)(void* self, UInt32 type, UInt32 powerReturned);
    IOReturn (* GetExtraPowerAllocated)(void* self, UInt32 type, UInt32* powerAllocated);

#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    /*!
     @function   GetDeviceAsyncNotificationPort
     @abstract   Returns the IONotificationPort for this IOService instance.
     @param      self Pointer to the IOUSBDeviceInterface.
     @result     Returns the IONotificationPortRef if one exists, MACH_PORT_NULL otherwise.
     */
    IONotificationPortRef (* GetDeviceAsyncNotificationPort)(void* self);
#endif
#endif

} IOUSBDeviceInterface400;

/*!
 @interface IOUSBDeviceInterface500
 @abstract   The object you use to access USB devices from user space, returned by the IOUSBFamily version 5.0.0 and above.
 @discussion The functions listed here include all of the functions defined for IOUSBDeviceInterface400, and some new functions that are available on Mac OS X version 10.7.3 and later.
 @super IOUSBDeviceInterface400
 */
typedef struct IOUSBDeviceStruct500 {
    IUNKNOWN_C_GUTS;
    IOReturn (*CreateDeviceAsyncEventSource)(void *self, CFRunLoopSourceRef *source);
    CFRunLoopSourceRef (*GetDeviceAsyncEventSource)(void *self);
    IOReturn (*CreateDeviceAsyncPort)(void *self, mach_port_t *port);
    mach_port_t (*GetDeviceAsyncPort)(void *self);
    IOReturn (*USBDeviceOpen)(void *self);
    IOReturn (*USBDeviceClose)(void *self);
    IOReturn (*GetDeviceClass)(void *self, UInt8 *devClass);
    IOReturn (*GetDeviceSubClass)(void *self, UInt8 *devSubClass);
    IOReturn (*GetDeviceProtocol)(void *self, UInt8 *devProtocol);
    IOReturn (*GetDeviceVendor)(void *self, UInt16 *devVendor);
    IOReturn (*GetDeviceProduct)(void *self, UInt16 *devProduct);
    IOReturn (*GetDeviceReleaseNumber)(void *self, UInt16 *devRelNum);
    IOReturn (*GetDeviceAddress)(void *self, USBDeviceAddress *addr);
    IOReturn (*GetDeviceBusPowerAvailable)(void *self, UInt32 *powerAvailable);
    IOReturn (*GetDeviceSpeed)(void *self, UInt8 *devSpeed);
    IOReturn (*GetNumberOfConfigurations)(void *self, UInt8 *numConfig);
    IOReturn (*GetLocationID)(void *self, UInt32 *locationID);
    IOReturn (*GetConfigurationDescriptorPtr)(void *self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr *desc);
    IOReturn (*GetConfiguration)(void *self, UInt8 *configNum);
    IOReturn (*SetConfiguration)(void *self, UInt8 configNum);
    IOReturn (*GetBusFrameNumber)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*ResetDevice)(void *self);
    IOReturn (*DeviceRequest)(void *self, IOUSBDevRequest *req);
    IOReturn (*DeviceRequestAsync)(void *self, IOUSBDevRequest *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*CreateInterfaceIterator)(void *self, IOUSBFindInterfaceRequest *req, io_iterator_t *iter);
    IOReturn (*USBDeviceOpenSeize)(void *self);
    IOReturn (*DeviceRequestTO)(void *self, IOUSBDevRequestTO *req);
    IOReturn (*DeviceRequestAsyncTO)(void *self, IOUSBDevRequestTO *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*USBDeviceSuspend)(void *self, Boolean suspend);
    IOReturn (*USBDeviceAbortPipeZero)(void *self);
    IOReturn (*USBGetManufacturerStringIndex)(void *self, UInt8 *msi);
    IOReturn (*USBGetProductStringIndex)(void *self, UInt8 *psi);
    IOReturn (*USBGetSerialNumberStringIndex)(void *self, UInt8 *snsi);
    IOReturn (*USBDeviceReEnumerate)(void *self, UInt32 options);
    IOReturn (*GetBusMicroFrameNumber)(void *self, UInt64 *microFrame, AbsoluteTime *atTime);
    IOReturn (*GetIOUSBLibVersion)(void *self, NumVersion *ioUSBLibVersion, NumVersion *usbFamilyVersion);
    IOReturn (*GetBusFrameNumberWithTime)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*GetUSBDeviceInformation)(void *self, UInt32 *info);
    IOReturn (*RequestExtraPower)(void *self, UInt32 type, UInt32 requestedPower, UInt32 *powerAvailable);
    IOReturn (*ReturnExtraPower)(void *self, UInt32 type, UInt32 powerReturned);
    IOReturn (*GetExtraPowerAllocated)(void *self, UInt32 type, UInt32 *powerAllocated);
#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    IONotificationPortRef (* GetDeviceAsyncNotificationPort)(void* self);
#endif
#endif
    /*!
     @function GetBandwidthAvailableForDevice
     @abstract   Returns the amount of bandwidth available on the bus for allocation to
     periodic pipes.  If the device is a high or super speed device, it will be the number of bytes per microframe (125 µsecs). If it is a full
     speed device, it will be the number of bytes per frame (1ms)
     @discussion This function is useful for determining the correct AltInterface setting as well as for using
     SetPipePolicy. The interface does not have to be open to use this function.
     @availability This function is only available with IOUSBDeviceInterface500 and above.
     @param      self Pointer to the IOUSBDeviceInterface.
     @param      bandwidth Pointer to UInt32 to hold the amount of bandwidth available (in bytes per frame or microframe).
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (*GetBandwidthAvailableForDevice)(void *self, UInt32 *bandwidth);

} IOUSBDeviceInterface500;

/*!
 @interface IOUSBDeviceInterface650
 @abstract   The object you use to access USB devices from user space, returned by the IOUSBFamily version 650.4.0 and above.
 @discussion The functions listed here include all of the functions defined for IOUSBDeviceInterface500, and some new functions that are available on Mac OS X version 10.9 and later.
 @super IOUSBDeviceInterface500
 */
typedef struct IOUSBDeviceStruct650 {
    IUNKNOWN_C_GUTS;
    IOReturn (*CreateDeviceAsyncEventSource)(void *self, CFRunLoopSourceRef *source);
    CFRunLoopSourceRef (*GetDeviceAsyncEventSource)(void *self);
    IOReturn (*CreateDeviceAsyncPort)(void *self, mach_port_t *port);
    mach_port_t (*GetDeviceAsyncPort)(void *self);
    IOReturn (*USBDeviceOpen)(void *self);
    IOReturn (*USBDeviceClose)(void *self);
    IOReturn (*GetDeviceClass)(void *self, UInt8 *devClass);
    IOReturn (*GetDeviceSubClass)(void *self, UInt8 *devSubClass);
    IOReturn (*GetDeviceProtocol)(void *self, UInt8 *devProtocol);
    IOReturn (*GetDeviceVendor)(void *self, UInt16 *devVendor);
    IOReturn (*GetDeviceProduct)(void *self, UInt16 *devProduct);
    IOReturn (*GetDeviceReleaseNumber)(void *self, UInt16 *devRelNum);
    IOReturn (*GetDeviceAddress)(void *self, USBDeviceAddress *addr);
    IOReturn (*GetDeviceBusPowerAvailable)(void *self, UInt32 *powerAvailable);
    IOReturn (*GetDeviceSpeed)(void *self, UInt8 *devSpeed);
    IOReturn (*GetNumberOfConfigurations)(void *self, UInt8 *numConfig);
    IOReturn (*GetLocationID)(void *self, UInt32 *locationID);
    IOReturn (*GetConfigurationDescriptorPtr)(void *self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr *desc);
    IOReturn (*GetConfiguration)(void *self, UInt8 *configNum);
    IOReturn (*SetConfiguration)(void *self, UInt8 configNum);
    IOReturn (*GetBusFrameNumber)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*ResetDevice)(void *self);
    IOReturn (*DeviceRequest)(void *self, IOUSBDevRequest *req);
    IOReturn (*DeviceRequestAsync)(void *self, IOUSBDevRequest *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*CreateInterfaceIterator)(void *self, IOUSBFindInterfaceRequest *req, io_iterator_t *iter);
    IOReturn (*USBDeviceOpenSeize)(void *self);
    IOReturn (*DeviceRequestTO)(void *self, IOUSBDevRequestTO *req);
    IOReturn (*DeviceRequestAsyncTO)(void *self, IOUSBDevRequestTO *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*USBDeviceSuspend)(void *self, Boolean suspend);
    IOReturn (*USBDeviceAbortPipeZero)(void *self);
    IOReturn (*USBGetManufacturerStringIndex)(void *self, UInt8 *msi);
    IOReturn (*USBGetProductStringIndex)(void *self, UInt8 *psi);
    IOReturn (*USBGetSerialNumberStringIndex)(void *self, UInt8 *snsi);
    IOReturn (*USBDeviceReEnumerate)(void *self, UInt32 options);
    IOReturn (*GetBusMicroFrameNumber)(void *self, UInt64 *microFrame, AbsoluteTime *atTime);
    IOReturn (*GetIOUSBLibVersion)(void *self, NumVersion *ioUSBLibVersion, NumVersion *usbFamilyVersion);
    IOReturn (*GetBusFrameNumberWithTime)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*GetUSBDeviceInformation)(void *self, UInt32 *info);
    IOReturn (*RequestExtraPower)(void *self, UInt32 type, UInt32 requestedPower, UInt32 *powerAvailable);
    IOReturn (*ReturnExtraPower)(void *self, UInt32 type, UInt32 powerReturned);
    IOReturn (*GetExtraPowerAllocated)(void *self, UInt32 type, UInt32 *powerAllocated);
#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    IONotificationPortRef (* GetDeviceAsyncNotificationPort)(void* self);
#endif
#endif
    IOReturn (*GetBandwidthAvailableForDevice)(void *self, UInt32 *bandwidth);

    /*!
     @function SetConfigurationV2
     @abstract   Sets the configuration in the device.
     @discussion Note that setting the configuration causes any existing IOUSBInterface objects attached to the
     IOUSBDevice to be destroyed, and all of the interfaces in the new configuration to be instantiated
     as new IOUSBInterface objects.  The device must be open to use this function.
     @param      self Pointer to the IOUSBDeviceInterface.
     @param      configNum The value of the desired configuration (from IOUSBConfigurationDescriptor.bConfigurationValue)
     @param      startInterfaceMatching true if IOUSBFamily should call IOKit to match the IOUSBInterface nubs.  If false is set, the client
     needs to either (1) have the "com.apple.vm.device-access" entitlement and have the IOUSBDevice authorized
     via the IOKit's IOServiceAuthorize() API or (1) have root privileges.
     @param      issueRemoteWakeup true if IOUSBFamily should send the command to enable remote wakeup in the device
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
     or kIOReturnNotOpen if the device is not open for exclusive access.
     */

    IOReturn (*SetConfigurationV2)(void *self, UInt8 configNum, bool startInterfaceMatching, bool issueRemoteWakeup);


    /*!
     @function RegisterForNotification
     @abstract   Registers a callback routine to be invoked when certain events occur in the kernel.
     @discussion The callback function will be called, for example when the underlying IOUSBDevice is
     going to be suspended due to some kind of kernel activity. It will also be called when
     the underlying IOUSBDevice is resumed.
     @availability This function is only available with IOUSBDeviceInterface650 and above.
     @param      self Pointer to the IOUSBDeviceInterface.
     @param      notificationMask Specifies the desired type of notification
     @param      callback An IOAsyncCallback2 method. Upon completion, the arg0 argument of the AsyncCallback2 will contain the
     notification type, and arg1 will contain a notificationToken which should be used when calling AcknowledgeNotification
     @param      refCon Arbitrary pointer which is passed as a parameter to the callback routine.
     @param      pRegistrationToken A pointer to a UInt64 which will contain a registration token if the function is
     successful. This registration token can then later be used to call UnregisterNotification.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */
    IOReturn (*RegisterForNotification)(void * self, UInt64 notificationMask, IOAsyncCallback2 callback, void *refCon, UInt64 *pRegistrationToken);



    /*!
     @function UnregisterNotification
     @abstract   Unregisters a previously registered callback routine
     @discussion The callback routine will no longer be invoked when events occur
     @availability This function is only available with IOUSBDeviceInterface650 and above.
     @param      self Pointer to the IOUSBDeviceInterface.
     @param      registrationToken The registration token which was obtained in the call to RegisterForNotification
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */
    IOReturn (*UnregisterNotification)(void *self, UInt64 registrationToken);


    /*!
     @function AcknowledgeNotification
     @abstract   Acknowledges a notification event
     @discussion Some events in the kernel will wait for an acknowledgement from all interested parties before proceeding.
     For example, if an IOUSBDevice is about to be suspended, any User Code which has registered to receive
     that event will be notified and should acknowledge the notification when it is ready for the IOUSBDevice
     to be suspended.
     @availability This function is only available with IOUSBDeviceInterface650 and above.
     @param      self Pointer to the IOUSBDeviceInterface.
     @param      notificationToken The notification token which was passed in to the callback routine
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */
    IOReturn (*AcknowledgeNotification)(void *self, UInt64 notificationToken);

} IOUSBDeviceInterface650;

/*!
 @interface IOUSBDeviceInterface942
 @abstract   The object you use to access USB devices from user space, returned by the IOUSBFamily version 900.4.2 and above.
 @discussion The functions listed here include all of the functions defined for IOUSBDeviceInterface650, and some new functions that are available on macOS 10.14 and later.
 @super IOUSBDeviceInterface650
 */
typedef struct IOUSBDeviceStruct942 {
    IUNKNOWN_C_GUTS;
    IOReturn (*CreateDeviceAsyncEventSource)(void *self, CFRunLoopSourceRef *source);
    CFRunLoopSourceRef (*GetDeviceAsyncEventSource)(void *self);
    IOReturn (*CreateDeviceAsyncPort)(void *self, mach_port_t *port);
    mach_port_t (*GetDeviceAsyncPort)(void *self);
    IOReturn (*USBDeviceOpen)(void *self);
    IOReturn (*USBDeviceClose)(void *self);
    IOReturn (*GetDeviceClass)(void *self, UInt8 *devClass);
    IOReturn (*GetDeviceSubClass)(void *self, UInt8 *devSubClass);
    IOReturn (*GetDeviceProtocol)(void *self, UInt8 *devProtocol);
    IOReturn (*GetDeviceVendor)(void *self, UInt16 *devVendor);
    IOReturn (*GetDeviceProduct)(void *self, UInt16 *devProduct);
    IOReturn (*GetDeviceReleaseNumber)(void *self, UInt16 *devRelNum);
    IOReturn (*GetDeviceAddress)(void *self, USBDeviceAddress *addr);
    IOReturn (*GetDeviceBusPowerAvailable)(void *self, UInt32 *powerAvailable);
    IOReturn (*GetDeviceSpeed)(void *self, UInt8 *devSpeed);
    IOReturn (*GetNumberOfConfigurations)(void *self, UInt8 *numConfig);
    IOReturn (*GetLocationID)(void *self, UInt32 *locationID);
    IOReturn (*GetConfigurationDescriptorPtr)(void *self, UInt8 configIndex, IOUSBConfigurationDescriptorPtr *desc);
    IOReturn (*GetConfiguration)(void *self, UInt8 *configNum);
    IOReturn (*SetConfiguration)(void *self, UInt8 configNum);
    IOReturn (*GetBusFrameNumber)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*ResetDevice)(void *self);
    IOReturn (*DeviceRequest)(void *self, IOUSBDevRequest *req);
    IOReturn (*DeviceRequestAsync)(void *self, IOUSBDevRequest *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*CreateInterfaceIterator)(void *self, IOUSBFindInterfaceRequest *req, io_iterator_t *iter);
    IOReturn (*USBDeviceOpenSeize)(void *self);
    IOReturn (*DeviceRequestTO)(void *self, IOUSBDevRequestTO *req);
    IOReturn (*DeviceRequestAsyncTO)(void *self, IOUSBDevRequestTO *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*USBDeviceSuspend)(void *self, Boolean suspend);
    IOReturn (*USBDeviceAbortPipeZero)(void *self);
    IOReturn (*USBGetManufacturerStringIndex)(void *self, UInt8 *msi);
    IOReturn (*USBGetProductStringIndex)(void *self, UInt8 *psi);
    IOReturn (*USBGetSerialNumberStringIndex)(void *self, UInt8 *snsi);
    IOReturn (*USBDeviceReEnumerate)(void *self, UInt32 options);
    IOReturn (*GetBusMicroFrameNumber)(void *self, UInt64 *microFrame, AbsoluteTime *atTime);
    IOReturn (*GetIOUSBLibVersion)(void *self, NumVersion *ioUSBLibVersion, NumVersion *usbFamilyVersion);
    IOReturn (*GetBusFrameNumberWithTime)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*GetUSBDeviceInformation)(void *self, UInt32 *info);
    IOReturn (*RequestExtraPower)(void *self, UInt32 type, UInt32 requestedPower, UInt32 *powerAvailable);
    IOReturn (*ReturnExtraPower)(void *self, UInt32 type, UInt32 powerReturned);
    IOReturn (*GetExtraPowerAllocated)(void *self, UInt32 type, UInt32 *powerAllocated);
#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    IONotificationPortRef (* GetDeviceAsyncNotificationPort)(void* self);
#endif
#endif
    IOReturn (*GetBandwidthAvailableForDevice)(void *self, UInt32 *bandwidth);
    IOReturn (*SetConfigurationV2)(void *self, UInt8 configNum, bool startInterfaceMatching, bool issueRemoteWakeup);
    IOReturn (*RegisterForNotification)(void * self, UInt64 notificationMask, IOAsyncCallback2 callback, void *refCon, UInt64 *pRegistrationToken);
    IOReturn (*UnregisterNotification)(void *self, UInt64 registrationToken);
    IOReturn (*AcknowledgeNotification)(void *self, UInt64 notificationToken);

#ifndef __OPEN_SOURCE__
#if !TARGET_OS_IPHONE || TARGET_OS_MACCATALYST
    /*!
     @function   GetDeviceAsyncNotificationPort
     @abstract   Returns the IONotificationPort for this IOService instance.
     @availability This function is only available with IOUSBDeviceInterface942 and above.
     @param      self Pointer to the IOUSBDeviceInterface.
     @result     Returns the IONotificationPortRef if one exists, MACH_PORT_NULL otherwise.
     */
    IONotificationPortRef (* GetDeviceAsyncNotificationPort)(void* self);
#endif
#endif
} IOUSBDeviceInterface942;

typedef IOUSBDeviceInterface942 IOUSBDeviceInterface;

// The undecorated interface ID is the oldest version
#define kIOUSBDeviceInterfaceID kIOUSBDeviceInterfaceID100

/*!
   @interface IOUSBInterfaceInterface
   @abstract   The object you use to access a USB device interface from user space, returned by all versions
        of the IOUSBFamily currently shipping.
   @discussion The functions listed here will work with any version of the IOUSBInterfaceInterface, including
        the one shipped with Mac OS X version 10.0.
 */
typedef struct IOUSBInterfaceStruct100
{
    IUNKNOWN_C_GUTS;

    /*!
       @function CreateInterfaceAsyncEventSource
       @abstract   Creates a run loop source for delivery of all asynchronous notifications on this device.
       @discussion The Mac OS X kernel does not spawn a thread to callback to the client. Instead
                it delivers completion notifications on a Mach port (see {@link CreateInterfaceAsyncPort}). This
                routine wraps that port with the appropriate routing code so that
                the completion notifications can be automatically routed through the client's
                CFRunLoop.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      source Pointer to a CFRunLoopSourceRef to return the newly created run loop event source.
       @result     Returns kIOReturnSuccess if successful or a kern_return_t if failed.
     */

    IOReturn (* CreateInterfaceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);

    /*!
       @function GetInterfaceAsyncEventSource
       @abstract   Returns the CFRunLoopSourceRef for this IOService instance.
       @discussion (description)
       @param      self Pointer to the IOUSBInterfaceInterface.
       @result     Returns the run loop source if one has been created, 0 otherwise.
     */

    CFRunLoopSourceRef (* GetInterfaceAsyncEventSource)(void* self);

    /*!
       @function CreateInterfaceAsyncPort
       @abstract   Creates and registers a mach_port_t for asynchronous communications.
       @discussion The Mac OS X kernel does not spawn a thread to callback to the client. Instead
                it delivers completion notifications on this Mach port. After receiving a message
                on this port the client is obliged to call the IOKitLib.h: IODispatchCalloutFromMessage()
                function for decoding the notification message.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @result     Returns kIOReturnSuccess if successful or a kern_return_t if failed.
     */

    IOReturn (* CreateInterfaceAsyncPort)(void* self, mach_port_t* port);

    /*!
       @function GetInterfaceAsyncPort
       @abstract   Returns the mach_port_t port for this IOService instance.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @result     Returns the port if one exists, 0 otherwise.
     */

    mach_port_t (* GetInterfaceAsyncPort)(void* self);

    /*!
       @function USBInterfaceOpen
       @abstract   Opensthe IOUSBInterface for exclusive access.
       @discussion Before the client can transfer data to and from the interface, it must have
                succeeded in opening the interface. This establishes an exclusive link between
                the client's task and the actual interface device. Opening the interface causes
                pipes to be created on each endpoint contained in the interface. If the interface
                contains isochronous endpoints, an attempt is made to allocate bandwidth on
                the bus for each of those pipes. If there is not enough bandwidth available,
                an isochronous pipe may be created with a bandwidth of zero. The software must
                then call SetPipePolicy to change the size of that pipe before it can be used
                for I/O.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @result     Returns kIOReturnExclusiveAccess if some other task has the device opened already,
                kIOReturnError if the connection with the kernel cannot be established or
                kIOReturnSuccess if successful.
     */

    IOReturn (* USBInterfaceOpen)(void* self);

    /*!
       @function USBInterfaceClose
       @abstract   Closes the task's connection to the IOUSBInterface.
       @discussion Releases the client's exclusive access to the IOUSBInterface.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* USBInterfaceClose)(void* self);

    /*!
       @function GetInterfaceClass
       @abstract   Returns the USB Class of the interface  (bInterfaceClass).
       @discussion The interface does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      intfClass Pointer to UInt8 to hold the interface Class.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetInterfaceClass)(void* self, UInt8* intfClass);

    /*!
       @function GetInterfaceSubClass
       @abstract   Returns the USB Subclass of the interface (bInterfaceSubClass).
       @discussion The interface does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      intfSubClass Pointer to UInt8 to hold the interface Subclass.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetInterfaceSubClass)(void* self, UInt8* intfSubClass);

    /*!
       @function GetInterfaceProtocol
       @abstract   Returns the USB Protocol of the interface (bInterfaceProtocol).
       @discussion The interface does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      intfProtocol Pointer to UInt8 to hold the interface Protocol.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetInterfaceProtocol)(void* self, UInt8* intfProtocol);

    /*!
       @function GetDeviceVendor
       @abstract   Returns the USB Vendor ID (idVendor) of the device of which this interface is a part.
       @discussion The interface does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      devVendor Pointer to UInt16 to hold the vendorID.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);

    /*!
       @function GetDeviceProduct
       @abstract   Returns the USB Product ID (idProduct) of the device of which this interface is a part.
       @discussion The interface does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      devProduct Pointer to UInt16 to hold the ProductID.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);

    /*!
       @function GetDeviceReleaseNumber
       @abstract   Returns the Device Release Number (bcdDevice) of the device of which this interface is a part.
       @discussion The interface does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      devRelNum Pointer to UInt16 to hold the Release Number.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);

    /*!
       @function GetConfigurationValue
       @abstract   Returns the current configuration value set in the device (the interface will be part of that configuration.)
       @discussion The interface does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      configVal Pointer to UInt8 to hold the configuration value.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetConfigurationValue)(void* self, UInt8* configVal);

    /*!
       @function GetInterfaceNumber
       @abstract   Returns the interface number (zero-based index) of this interface within the current configuration of the device.
       @discussion The interface does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      intfNumber Pointer to UInt8 to hold the interface number.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetInterfaceNumber)(void* self, UInt8* intfNumber);

    /*!
       @function GetAlternateSetting
       @abstract   Returns the alternate setting currently selected in this interface.
       @discussion The interface does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      intfAltSetting Pointer to UInt8 to hold the alternate setting value.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetAlternateSetting)(void* self, UInt8* intfAltSetting);

    /*!
       @function GetNumEndpoints
       @abstract   Returns the number of endpoints in this interface.
       @discussion The interface does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      intfNumEndpoints Pointer to UInt8 to hold the number of endpoints.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetNumEndpoints)(void* self, UInt8* intfNumEndpoints);

    /*!
       @function GetLocationID
       @abstract   Returns the location ID.
       @discussion The location ID is a 32 bit number which is unique among all USB devices in the system, and which
                will not change on a system reboot unless the topology of the bus itself changes.  The interface
                does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      locationID Pointer to UInt32 to hold the location ID.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetLocationID)(void* self, UInt32* locationID);

    /*!
       @function GetDevice
       @abstract   Returns the device of which this interface is part.
       @discussion The interface does not have to be open to use this function. The returned device can be used to
                create a CFPlugin to talk to the device.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      device Pointer to io_service_t to hold the result.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetDevice)(void* self, io_service_t* device);

    /*!
       @function SetAlternateInterface
       @abstract   Changes the AltInterface setting.
       @discussion The interface must be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      alternateSetting The new alternate setting for the interface.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* SetAlternateInterface)(void* self, UInt8 alternateSetting);

    /*!
       @function GetBusFrameNumber
       @abstract   Gets the current frame number of the bus to which the interface and its device are attached.
       @discussion The interface does not have to be open to use this function.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      frame Pointer to UInt64 to hold the frame number.
       @param      atTime Pointer to an AbsoluteTime, which should be within 1ms of the time when the bus frame
                number was attained.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);

    /*!
       @function ControlRequest
       @abstract   Sends a USB request on a control pipe.
       @discussion If the request is a standard request which will change the state of the device, the device must
                be open, which means you should be using the IOUSBDeviceInterface for this command.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index of the control pipe to use. Use zero for the default control pipe on the device.
       @param      req Pointer to an IOUSBDevRequest containing the request.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                                kIOReturnAborted if the thread is interrupted before the call completes,
                or kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* ControlRequest)(void* self, UInt8 pipeRef, IOUSBDevRequest* req);

    /*!
       @function ControlRequestAsync
       @abstract   Sends an asynchronous USB request on a control pipe.
       @discussion Use pipeRef=0 for the default device control pipe.  If the request is a standard request which will
                change the state of the device, the device must be open, which means you should be using the
                IOUSBDeviceInterface for this command.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index of the control pipe to use. Use zero for the default control pipe on the device.
       @param      req Pointer to an IOUSBDevRequest containing the request.
       @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the number of bytes that were actually transferred. A message addressed to this callback is posted to the Async port upon completion.
       @param      refCon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                kIOReturnNotOpen if the interface is not open for exclusive access, or kIOUSBNoAsyncPortErr if no
                Async port has been created for this interface.
     */

    IOReturn (* ControlRequestAsync)(void* self, UInt8 pipeRef, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);

    /*!
       @function GetPipeProperties
       @abstract   Gets the properties for a pipe.
       @discussion Once an interface is opened, all of the pipes in that interface get created by the kernel. The number
                of pipes can be retrieved by GetNumEndpoints. The client can then get the properties of any pipe
                using an index of 1 to GetNumEndpoints. Pipe 0 is the default control pipe in the device.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      direction Pointer to an UInt8 to get the direction of the pipe.
       @param      number Pointer to an UInt8 to get the pipe number.
       @param      transferType Pointer to an UInt8 to get the transfer type of the pipe.
       @param      maxPacketSize Pointer to an UInt16 to get the maxPacketSize of the pipe. This maxPacketSize is the FULL maxPacketSize, which takes into account the multipler for HS Isoc pipes
     and the burst and the multiplier for SS Isoc pipes. It could also have been adjusted by SetPipePolicy.
       @param      interval Pointer to an UInt8 to get the interval for polling the pipe for data (in milliseconds).
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* GetPipeProperties)(void* self, UInt8 pipeRef, UInt8* direction, UInt8* number, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);

    /*!
       @function GetPipeStatus
       @abstract   Gets the current status of a pipe.
       @discussion The interface must be open for the pipe to exist.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @result     Returns kIOReturnNoDevice if there is no connection to an IOService, or kIOReturnNotOpen
                if the interface is not open for exclusive access. Otherwise, the status of the pipe is returned.
                Returns kIOUSBPipeStalled if the pipe is stalled. See @link //apple_ref/C/instm/IOUSBInterfaceInterface/ClearPipeStall/ ClearPipeStall @/link
                or @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link for
                more information.
     */

    IOReturn (* GetPipeStatus)(void* self, UInt8 pipeRef);

    /*!
       @function AbortPipe
       @abstract   Aborts any outstanding transactions on the pipe with status kIOReturnAborted.
       @discussion If there are outstanding asynchronous transactions on the pipe, the callbacks will happen.
                Note that this command will also clear the halted bit on the endpoint
                in the controller, but will NOT clear the data toggle bit.  If you want to clear the data toggle bit as well, see @link //apple_ref/C/instm/IOUSBInterfaceInterface/ClearPipeStall/ ClearPipeStall @/link or
                @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link for more information.  The interface must be open for the pipe to exist.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the interface is not open for exclusive access.

     */

    IOReturn (* AbortPipe)(void* self, UInt8 pipeRef);

    /*!
       @function ResetPipe
       @abstract   Equivalent to ClearPipeStall.
       @discussion The interface must be open for the pipe to exist.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* ResetPipe)(void* self, UInt8 pipeRef);

    /*!
       @function ClearPipeStall
       @abstract   Clears the halted bit and the data toggle bit on the pipe's endpoint in the controller.
       @discussion This function also returns any outstanding transactions on the pipe with status kIOUSBTransactionReturned.
                If there are outstanding asynchronous transactions on the pipe, the callbacks will happen. The data
                toggle may need to be resynchronized. The driver may handle this by sending a ClearFeature(ENDPOINT_HALT)
                to the default control pipe, specifying the device's endpoint for this pipe. See also
                @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* ClearPipeStall)(void* self, UInt8 pipeRef);

    /*!
       @function ReadPipe
       @abstract   Reads data on a <b>BULK IN</b> or an <b>INTERRUPT</b> pipe.
       @discussion The interface must be open for the pipe to exist.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data.
       @param      size On entry: a pointer to the size of the buffer pointed to by buf.
                On exit: a pointer to the number of bytes actually read from the device.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* ReadPipe)(void* self, UInt8 pipeRef, void* buf, UInt32* size);

    /*!
       @function WritePipe
       @abstract   Writes data on a <b>BULK OUT</b> or <b>INTERRUPT OUT</b> pipe.
       @discussion The interface must be open for the pipe to exist.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data.
       @param      size The size of the data buffer.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                                kIOReturnAborted if the thread is interrupted before the call completes,
                or kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* WritePipe)(void* self, UInt8 pipeRef, void* buf, UInt32 size);

    /*!
       @function ReadPipeAsync
       @abstract   Performs an asynchronous read on a <b>BULK IN</b> or an <b>INTERRUPT</b> pipe.
       @discussion The interface must be open for the pipe to exist.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data.
       @param      size The size of the buffer pointed to by buf.
       @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the number of bytes that were actually read. A message addressed to this callback is posted to the Async
                port upon completion.
       @param      refcon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                                kIOReturnAborted if the thread is interrupted before the call completes,
                or kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* ReadPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);

    /*!
       @function WritePipeAsync
       @abstract   Performs an asynchronous write on a <b>BULK OUT</b> or <b>INTERRUPT OUT</b> pipe.
       @discussion The interface must be open for the pipe to exist.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data.
       @param      size The size of the buffer pointed to by buf.
       @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the number of bytes that were actually written. A message addressed to this callback is posted to the Async
                port upon completion.
       @param      refcon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* WritePipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);

    /*!
       @function ReadIsochPipeAsync
       @abstract   Performs a read on an <b>ISOCHRONOUS</b> pipe.
       @discussion The interface must be open for the pipe to exist.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data.
       @param      frameStart The bus frame number on which to start the read (obtained from GetBusFrameNumber).
       @param      numFrames The number of frames for which to transfer data.
       @param      frameList A pointer to an array of IOUSBIsocFrame structures describing the frames.
       @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the framelist pointer, which can be used to associate the completion with a particular request. A message addressed to this callback is posted to the Async
                port upon completion.
       @param      refcon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* ReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                    IOAsyncCallback1 callback, void* refcon);

    /*!
       @function WriteIsochPipeAsync
       @abstract   Performs an asynchronous write on an <b>ISOCHRONOUS</b> pipe.
       @discussion The interface must be open for the pipe to exist.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data.
       @param      frameStart The bus frame number on which to start the write (obtained from GetBusFrameNumber).
       @param      numFrames The number of frames for which to transfer data.
       @param      frameList A pointer to an array of IOUSBIsocFrame structures describing the frames.
       @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the framelist pointer, which can be used to associate the completion with a particular request. A message addressed to this callback is posted to the Async
                port upon completion.
       @param      refcon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                or kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* WriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                     IOAsyncCallback1 callback, void* refcon);
} IOUSBInterfaceInterface100;

/*!
   @interface IOUSBInterfaceInterface182
   @abstract   The object you use to access a USB device interface from user space, returned by the IOUSBFamily
        version 1.8.2 and above.
   @discussion The functions listed here include all of the functions defined for the IOUSBInterfaceInterface and
        some new functions that are available on Mac OS X version 10.0.4 and later.
   @super  IOUSBInterfaceInterface
 */

typedef struct IOUSBInterfaceStruct182
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateInterfaceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetInterfaceAsyncEventSource)(void* self);
    IOReturn (* CreateInterfaceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetInterfaceAsyncPort)(void* self);
    IOReturn (* USBInterfaceOpen)(void* self);
    IOReturn (* USBInterfaceClose)(void* self);
    IOReturn (* GetInterfaceClass)(void* self, UInt8* intfClass);
    IOReturn (* GetInterfaceSubClass)(void* self, UInt8* intfSubClass);
    IOReturn (* GetInterfaceProtocol)(void* self, UInt8* intfProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetConfigurationValue)(void* self, UInt8* configVal);
    IOReturn (* GetInterfaceNumber)(void* self, UInt8* intfNumber);
    IOReturn (* GetAlternateSetting)(void* self, UInt8* intfAltSetting);
    IOReturn (* GetNumEndpoints)(void* self, UInt8* intfNumEndpoints);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetDevice)(void* self, io_service_t* device);
    IOReturn (* SetAlternateInterface)(void* self, UInt8 alternateSetting);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ControlRequest)(void* self, UInt8 pipeRef, IOUSBDevRequest* req);
    IOReturn (* ControlRequestAsync)(void* self, UInt8 pipeRef, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* GetPipeProperties)(void* self, UInt8 pipeRef, UInt8* direction, UInt8* number, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* GetPipeStatus)(void* self, UInt8 pipeRef);
    IOReturn (* AbortPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ResetPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ClearPipeStall)(void* self, UInt8 pipeRef);
    IOReturn (* ReadPipe)(void* self, UInt8 pipeRef, void* buf, UInt32* size);
    IOReturn (* WritePipe)(void* self, UInt8 pipeRef, void* buf, UInt32 size);
    IOReturn (* ReadPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                    IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                     IOAsyncCallback1 callback, void* refcon);

    /*!
       @function ControlRequestTO
       @abstract   Sends a USB request on a control pipe.
       @discussion The IOUSBDevRequestTO structure allows the client to specify timeout values for this request.  If
                the request is a standard request which will change the state of the device, the device must be open,
                which means you should be using the IOUSBDeviceInterface for this command.
       @availability This function is only available with IOUSBInterfaceInterface182 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index of the control pipe to use. Use zero for the default control pipe on the device.
       @param      req Pointer to an IOUSBDevRequestTO containing the request.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
                                kIOReturnAborted if the thread is interrupted before the call completes,
                kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* ControlRequestTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req);

    /*!
       @function ControlRequestAsyncTO
       @abstract   Sends an asynchronous USB request on a control pipe.
       @discussion The IOUSBDevRequestTO structure allows the client to specify timeout values for this request. Use
                pipeRef=0 for the default device control pipe.  If the request is a standard request which will
                change the state of the device, the device must be open, which means you should be using the
                IOUSBDeviceInterface for this command.
       @availability This function is only available with IOUSBInterfaceInterface182 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index of the control pipe to use. Use zero for the default control pipe on the device.
       @param      req Pointer to an IOUSBDevRequestTO containing the request.
       @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the number of bytes that were actually transferred. A message addressed to this callback is posted to the Async
                port upon completion.
       @param      refCon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
                kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* ControlRequestAsyncTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);

    /*!
       @function ReadPipeTO
       @abstract   Performs a read on a <b>BULK IN</b> pipe, specifying timeout values.
       @discussion The interface must be open for the pipe to exist.

                If a timeout is specified and the request times out, the driver may need to resynchronize the data
                toggle. See @link //apple_ref/C/instm/IOUSBInterfaceInterface/ClearPipeStall/ ClearPipeStall @/link
                or @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link.


                Timeouts do not apply to interrupt pipes, so you should use the ReadPipe API to perform a read from
                an interrupt pipe.
       @availability This function is only available with IOUSBInterfaceInterface182 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data.
       @param      size Pointer to the size of the buffer pointed to by buf.
       @param      noDataTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if no
                data is transferred in this amount of time, the request will be aborted and returned.
       @param      completionTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if
                the entire request is not completed in this amount of time, the request will be aborted and returned.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                                kIOReturnAborted if the thread is interrupted before the call completes, or
                kIOReturnNotOpen if the interface is not open for exclusive access.  Returns kIOReturnBadArgument if timeout
                values are specified for an interrupt pipe.  If an error is returned, the size parameter is not updated and the buffer will
                                NOT contain any valid data.
     */

    IOReturn (* ReadPipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32* size, UInt32 noDataTimeout, UInt32 completionTimeout);

    /*!
       @function WritePipeTO
       @abstract   Performs a write on a <b>BULK OUT</b> pipe, with specified timeout values.
       @discussion The interface must be open for the pipe to exist.

                If a timeout is specified and the request times out, the driver may need to resynchronize the data
                toggle. See @link //apple_ref/C/instm/IOUSBInterfaceInterface/ClearPipeStall/ ClearPipeStall @/link
                or @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link.
       @availability This function is only available with IOUSBInterfaceInterface182 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data.
       @param      size The size of the buffer pointed to by buf.
       @param      noDataTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if no
                data is transferred in this amount of time, the request will be aborted and returned.
       @param      completionTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if
                the entire request is not completed in this amount of time, the request will be aborted and returned.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
                                kIOReturnAborted if the thread is interrupted before the call completes, or
                kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* WritePipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);

    /*!
       @function ReadPipeAsyncTO
       @abstract   Performs an asynchronous read on a <b>BULK IN </b>pipe, with specified timeout values.
       @discussion The interface must be open for the pipe to exist.

                If a timeout is specified and the request times out, the driver may need to resynchronize the data
                toggle. See @link //apple_ref/C/instm/IOUSBInterfaceInterface/ClearPipeStall/ ClearPipeStall @/link
                or @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link.

                Timeouts do not apply to interrupt pipes, so you should use the ReadPipeAsync API to perform an
                asynchronous read from an interrupt pipe.
       @availability This function is only available with IOUSBInterfaceInterface182 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data.
       @param      size The size of the buffer pointed to by buf.
       @param      noDataTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if no
                data is transferred in this amount of time, the request will be aborted and returned.
       @param      completionTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if
                the entire request is not completed in this amount of time, the request will be aborted and returned.
       @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the number of bytes that were actually read. A message addressed to this callback is posted to the Async port
                upon completion.
       @param      refcon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
                kIOReturnNotOpen if the interface is not open for exclusive access.  Returns kIOReturnBadArgument if timeout
         values are specified for an interrupt pipe.  If an error is returned, the size parameter is not updated and the buffer will
         NOT contain any valid data.
     */

    IOReturn (* ReadPipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);

    /*!
       @function WritePipeAsyncTO
       @abstract   Performs an asynchronous write on a <b>BULK OUT</b> pipe, with specified timeout values.
       @discussion The interface must be open for the pipe to exist.

                If a timeout is specified and the request times out, the driver may need to resynchronize the data
                toggle. See @link //apple_ref/C/instm/IOUSBInterfaceInterface/ClearPipeStall/ ClearPipeStall @/link
                or @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link.
       @availability This function is only available with IOUSBInterfaceInterface182 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data.
       @param      size The size of the buffer pointed to by buf.
       @param      noDataTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if no
                data is transferred in this amount of time, the request will be aborted and returned.
       @param      completionTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if
                the entire request is not completed in this amount of time, the request will be aborted and returned.
       @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the number of bytes that were actually written. A message addressed to this callback is posted to the Async port
                upon completion.
       @param      refcon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
                kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* WritePipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);

    /*!
       @function USBInterfaceGetStringIndex
       @abstract   Returns the string index in the interface descriptor.
       @discussion The interface does not have to be open to use this function.
       @availability This function is only available with IOUSBInterfaceInterface182 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      si Pointer to UInt8 to hold the string index.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* USBInterfaceGetStringIndex)(void* self, UInt8* si);
} IOUSBInterfaceInterface182;

/*!
   @interface IOUSBInterfaceInterface183
   @abstract   The object you use to access a USB device interface from user space, returned by the IOUSBFamily
        version 1.8.3 and above.
   @discussion The functions listed here include all of the functions defined for the IOUSBInterfaceInterface,
        IOUSBInterfaceInterface182, and some new functions that are available on Mac OS X version 10.1
        and later.
   @super IOUSBInterfaceInterface182
 */

typedef struct IOUSBInterfaceStruct183
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateInterfaceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetInterfaceAsyncEventSource)(void* self);
    IOReturn (* CreateInterfaceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetInterfaceAsyncPort)(void* self);
    IOReturn (* USBInterfaceOpen)(void* self);
    IOReturn (* USBInterfaceClose)(void* self);
    IOReturn (* GetInterfaceClass)(void* self, UInt8* intfClass);
    IOReturn (* GetInterfaceSubClass)(void* self, UInt8* intfSubClass);
    IOReturn (* GetInterfaceProtocol)(void* self, UInt8* intfProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetConfigurationValue)(void* self, UInt8* configVal);
    IOReturn (* GetInterfaceNumber)(void* self, UInt8* intfNumber);
    IOReturn (* GetAlternateSetting)(void* self, UInt8* intfAltSetting);
    IOReturn (* GetNumEndpoints)(void* self, UInt8* intfNumEndpoints);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetDevice)(void* self, io_service_t* device);
    IOReturn (* SetAlternateInterface)(void* self, UInt8 alternateSetting);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ControlRequest)(void* self, UInt8 pipeRef, IOUSBDevRequest* req);
    IOReturn (* ControlRequestAsync)(void* self, UInt8 pipeRef, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* GetPipeProperties)(void* self, UInt8 pipeRef, UInt8* direction, UInt8* number, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* GetPipeStatus)(void* self, UInt8 pipeRef);
    IOReturn (* AbortPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ResetPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ClearPipeStall)(void* self, UInt8 pipeRef);
    IOReturn (* ReadPipe)(void* self, UInt8 pipeRef, void* buf, UInt32* size);
    IOReturn (* WritePipe)(void* self, UInt8 pipeRef, void* buf, UInt32 size);
    IOReturn (* ReadPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                    IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                     IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ControlRequestTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req);
    IOReturn (* ControlRequestAsyncTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* ReadPipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32* size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* WritePipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* ReadPipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* USBInterfaceGetStringIndex)(void* self, UInt8* si);

    /*!
       @function USBInterfaceOpenSeize
       @abstract   Opens the IOUSBInterface for exclusive access.
       @discussion If another client has the device open, an attempt is made to get that client to close it before
                returning.

                Before the client can issue commands that change the state of the device, it must have succeeded
                in opening the device. This establishes an exclusive link between the clients task and the actual
                device.
       @availability This function is only available with IOUSBInterfaceInterface183 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @result     Returns kIOReturnExclusiveAccess if some other task has the interface open already and refuses to
                close it, kIOReturnError if the connection with the kernel cannot be established or kIOReturnSuccess
                if successful.
     */

    IOReturn (* USBInterfaceOpenSeize)(void* self);
} IOUSBInterfaceInterface183;

/*!
   @interface IOUSBInterfaceInterface190
   @abstract   The object you use to access a USB device interface from user space, returned by the IOUSBFamily
        version 1.9 and above.
   @discussion The functions listed here include all of the functions defined for the IOUSBInterfaceInterface,
        IOUSBInterfaceInterface182, IOUSBInterfaceInterface183, and some new functions that are available
        on Mac OS X version 10.2 and later.
   @super IOUSBInterfaceInterface183
 */

typedef struct IOUSBInterfaceStruct190
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateInterfaceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetInterfaceAsyncEventSource)(void* self);
    IOReturn (* CreateInterfaceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetInterfaceAsyncPort)(void* self);
    IOReturn (* USBInterfaceOpen)(void* self);
    IOReturn (* USBInterfaceClose)(void* self);
    IOReturn (* GetInterfaceClass)(void* self, UInt8* intfClass);
    IOReturn (* GetInterfaceSubClass)(void* self, UInt8* intfSubClass);
    IOReturn (* GetInterfaceProtocol)(void* self, UInt8* intfProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetConfigurationValue)(void* self, UInt8* configVal);
    IOReturn (* GetInterfaceNumber)(void* self, UInt8* intfNumber);
    IOReturn (* GetAlternateSetting)(void* self, UInt8* intfAltSetting);
    IOReturn (* GetNumEndpoints)(void* self, UInt8* intfNumEndpoints);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetDevice)(void* self, io_service_t* device);
    IOReturn (* SetAlternateInterface)(void* self, UInt8 alternateSetting);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ControlRequest)(void* self, UInt8 pipeRef, IOUSBDevRequest* req);
    IOReturn (* ControlRequestAsync)(void* self, UInt8 pipeRef, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* GetPipeProperties)(void* self, UInt8 pipeRef, UInt8* direction, UInt8* number, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* GetPipeStatus)(void* self, UInt8 pipeRef);
    IOReturn (* AbortPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ResetPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ClearPipeStall)(void* self, UInt8 pipeRef);
    IOReturn (* ReadPipe)(void* self, UInt8 pipeRef, void* buf, UInt32* size);
    IOReturn (* WritePipe)(void* self, UInt8 pipeRef, void* buf, UInt32 size);
    IOReturn (* ReadPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                    IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                     IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ControlRequestTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req);
    IOReturn (* ControlRequestAsyncTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* ReadPipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32* size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* WritePipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* ReadPipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* USBInterfaceGetStringIndex)(void* self, UInt8* si);
    IOReturn (* USBInterfaceOpenSeize)(void* self);

    /*!
       @function ClearPipeStallBothEnds
       @abstract   Equivalent to ClearPipeStall.
       @discussion This function is equivalent to ClearPipeStall except that it also attempts to clear the halt and
                toggle bits on the device's endpoint for the pipe by sending a ClearFeature(ENDPOINT_HALT) to the
                default control pipe in the device, specifying the endpoint for the pipe represented by pipeRef. For
                most devices, this resynchronizes the data toggle between the two endpoints to ensure that there is
                no loss of data.
       @availability This function is only available with IOUSBInterfaceInterface190 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
                kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (* ClearPipeStallBothEnds)(void* self, UInt8 pipeRef);

    /*!
       @function SetPipePolicy
       @abstract   Changes the amount of bandwidth of an isochronous pipe or interrupt pipe, or the polling interval of an interrupt pipe.
       @discussion A pipe may be made smaller or larger (up to the maxPacketSize specified in the endpoint descriptor).
                When an interface is first opened, all pipes are created with their descriptor-supplied maxPacketSize.
                For isochronous or interrupt pipes, if there is not enough bandwidth on the bus to allocate to the pipe, the pipe
                is created with a reserved bandwidth of zero. Any attempts to transfer data on a pipe with zero
                bandwidth will result in a kIOReturnNoBandwidth error. The pipe must first be given some bandwidth
                using this call.  This can also be used to return bandwidth for an isochronous or an interrupt pipe.  If the driver
                                knows that the device will not be sending the maxPacketSize data, it can use this call to return that unused bandwidth to the
                                system.  If an interrupt pipe wants to change the polling interval, it can do so with this call.

                The interface must be open for the pipe to exist.
       @availability This function is only available with IOUSBInterfaceInterface190 and above.
       @param   self Pointer to the IOUSBInterfaceInterface.
       @param   pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param   maxPacketSize The desired size for the isochronous or interrupt pipe. For Full Speed endpoints and High Speed endpoints which
                are not High Bandwidth (i.e. only a single packet is transferred in a microframe), valid values are 0 through the maxPacketSize
                defined in the endpoint descriptor. For High Speed High Bandwidth (i.e. 2 or 3 packets transferred in a microframe) and Super Speed endpoints,
                valid values are 0 and the maxPacketSize of the endpoint. The maxPacketSize of the endpoint is calculated as the base maxPacketSize
                multiplies by the burst size and the multiplier. See the USB 2.0 and the USB 3.0 specifications for more detail. Using a value of 0 for maxPacketSize
                maintains the handle for the pipe but unreserves any bandwidth on the bus.
       @param   maxInterval  applies only to interrupt endpoints. maxInterval has the same value as the bInterval field of the endpoint.
                For Low Speed and Full Speed interrupt endpoints, it is the desired polling interval in milliseconds, up to a maximum of 128 ms.
                The system can only poll devices powers of 2 (1, 2, 4, 8, 16, 32, 64, or 128 ms).
                For High Speed and Super Speed endpoints, maxInterval is used as an exponent for a (2 ^ (maxInterval-1)) value representing the number
                of 125uS microframes between service intervals. See the USB 2.0 and the USB 3.0 specifications for more info. The lower layers may
                schedule the endpoint for service at an interval value equal to or less than the value specified. A value of 0 is illegal.
       @result  Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
                kIOReturnNotOpen if the interface is not open for exclusive access.  May also return kIOReturnNoBandwidth
                if there is not enough bandwidth available on the bus, or kIOReturnBadArgument if the desired
                maxPacketSize is outside of the allowed range.
     */

    IOReturn (* SetPipePolicy)(void* self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);

    /*!
       @function GetBandwidthAvailable
       @abstract   Returns the amount of bandwidth available on the bus for allocation to
                isochronous pipes.  If the device is a high speed device, it will be the number of bytes per microframe (125 µsecs). If it is a full
                                speed device, it will be the number of bytes per frame (1ms)
       @discussion This function is useful for determining the correct AltInterface setting as well as for using
                SetPipePolicy.

                The interface does not have to be open to use this function.
       @availability This function is only available with IOUSBInterfaceInterface190 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      bandwidth Pointer to UInt32 to hold the amount of bandwidth available (in bytes per 1ms frame).
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetBandwidthAvailable)(void* self, UInt32* bandwidth);

    /*!
       @function GetEndpointProperties
       @abstract   Returns the transfer type, max packet size, and interval of a specified endpoint, whether or not
                the endpoint has a pipe currently established.
       @discussion This function may be useful for determining which alternate interface to select when trying to
                balance bandwidth allocations among isochronous pipes.

                The interface does not have to be open to use this function.
       @availability This function is only available with IOUSBInterfaceInterface190 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      alternateSetting Specifies the alternate setting within the current interface.
       @param      endpointNumber Specifies the desired endpoint number.
       @param      direction Specifies the desired direction.
       @param      transferType Pointer to UInt8 to hold the endpoint's transfer type (kUSBControl, kUSBIsoc, etc).
       @param      maxPacketSize Pointer to UInt16 to hold the maxPacketSize of the endpoint.
       @param      interval Pointer to UInt8 to hold the polling interval for interrupt endpoints.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetEndpointProperties)(void* self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
} IOUSBInterfaceInterface190;

/*!
   @interface IOUSBInterfaceInterface192
   @abstract   The object you use to access a USB device interface from user space, returned by the IOUSBFamily
        version 1.9.2 and above.
   @discussion The functions listed here include all of the functions defined for the IOUSBInterfaceInterface,
        IOUSBInterfaceInterface182, IOUSBInterfaceInterface183, IOUSBInterfaceInterface190, and some new
        functions that are available on Mac OS X version 10.2.3 and later.
   @super IOUSBInterfaceInterface190
 */

typedef struct IOUSBInterfaceStruct192
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateInterfaceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetInterfaceAsyncEventSource)(void* self);
    IOReturn (* CreateInterfaceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetInterfaceAsyncPort)(void* self);
    IOReturn (* USBInterfaceOpen)(void* self);
    IOReturn (* USBInterfaceClose)(void* self);
    IOReturn (* GetInterfaceClass)(void* self, UInt8* intfClass);
    IOReturn (* GetInterfaceSubClass)(void* self, UInt8* intfSubClass);
    IOReturn (* GetInterfaceProtocol)(void* self, UInt8* intfProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetConfigurationValue)(void* self, UInt8* configVal);
    IOReturn (* GetInterfaceNumber)(void* self, UInt8* intfNumber);
    IOReturn (* GetAlternateSetting)(void* self, UInt8* intfAltSetting);
    IOReturn (* GetNumEndpoints)(void* self, UInt8* intfNumEndpoints);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetDevice)(void* self, io_service_t* device);
    IOReturn (* SetAlternateInterface)(void* self, UInt8 alternateSetting);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ControlRequest)(void* self, UInt8 pipeRef, IOUSBDevRequest* req);
    IOReturn (* ControlRequestAsync)(void* self, UInt8 pipeRef, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* GetPipeProperties)(void* self, UInt8 pipeRef, UInt8* direction, UInt8* number, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* GetPipeStatus)(void* self, UInt8 pipeRef);
    IOReturn (* AbortPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ResetPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ClearPipeStall)(void* self, UInt8 pipeRef);
    IOReturn (* ReadPipe)(void* self, UInt8 pipeRef, void* buf, UInt32* size);
    IOReturn (* WritePipe)(void* self, UInt8 pipeRef, void* buf, UInt32 size);
    IOReturn (* ReadPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                    IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                     IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ControlRequestTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req);
    IOReturn (* ControlRequestAsyncTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* ReadPipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32* size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* WritePipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* ReadPipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* USBInterfaceGetStringIndex)(void* self, UInt8* si);
    IOReturn (* USBInterfaceOpenSeize)(void* self);
    IOReturn (* ClearPipeStallBothEnds)(void* self, UInt8 pipeRef);
    IOReturn (* SetPipePolicy)(void* self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (* GetBandwidthAvailable)(void* self, UInt32* bandwidth);
    IOReturn (* GetEndpointProperties)(void* self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);

    /*!
       @function LowLatencyReadIsochPipeAsync
       @abstract   Performs an asynchronous read on a isochronous pipe and updates the frame list at primary interrupt time.
       @discussion The LowLatencyReadIsochPipeAsync() and LowLatencyWriteIsochPipeAsync()
                calls are analogous to ReadIsochPipeAsync() and WriteIsochPipeAsync(). They differ in that the frame
                list data is updated at <em>primary interrupt time</em>. This means that the client can inspect the
                frStatus and frActCount fields as soon as the transaction completes, without having to wait for the
                callback to happen (depending on the value of updateFrequency). The callback will still happen when
                all the frames have been received.

                The client can specify how often the USB stack should update the frame list data by specifying the
                updateFrequency: this value can range from 0 - 8. If the value is between 1 and 8, the
                frame list will be updated every updateFrequency milliseconds. If the value is 0, the
                frame list will only be updated once all the frames in the transfer have been received. For example,
                consider a transfer with numFrames equal to 64. If the update frequency is 4, the frame
                list data will be updated every 4 milliseconds. If the update frequency is 0, the frame list will
                only be updated at the end of the transfer, after the 64 frames have been sent or received. The
                difference between using an update frequency of 0 and using the non-low latency isoch calls is that
                in the former case, the frame list will be updated at primary interrupt time, while in the latter,
                it will be updated at secondary interrupt time. Regardless of the value of updateFrequency,
                the frame list will <em>always</em> be updated on the last frame of a transfer.

                The rationale for adding this call is that because completion routines run on the USB Workloop, they
                can be scheduled to run a number of milliseconds after the USB transfer has finished. This latency
                is variable and depends on what other higher priority threads are running on the system. This latency
                presents a problem for applications, such as audio processing, that depend on receiving data,
                processing it, and sending it back out, and need to do this as fast as possible. Since applications
                that use isochronous data know when the data should be available, they can look at the frame list at
                the expected time and note the frActCount and frStatus (and frTimeStamp
                if needed) and determine how many valid bytes are in their data buffer and whether there was an
                error. They can then access their data buffer and process the actual data.

                In order to update the frame list at primary interrupt time and to allow the client to see that
                update, the frame list buffer needs to be shared between the kernel and user space. The same thing
                applies to the data buffer. This is a difference between the low latency isoch calls and the regular
                isoch calls. The LowLatencyCreateBuffer() call is used to pre-allocate the buffers. The
                client <em>must</em> use that call to allocate the data and the frame list buffers. The client can
                pass a portion of the buffer that was previously allocated. The USB stack will range-check the data
                and frame list buffers to make sure they are within the ranges of the buffers previously allocated.
                This allows the client, if it so desires, to allocate a large data buffer and pass portions of it to
                the read or write calls. The same applies to the frame list buffers. Of course, the client can also
                pre-allocate several data buffers and several frame list buffers and use those for each transfer.
                Once the transfer completes, the buffers can be reused in subsequent calls. When all transfers are
                finished, the client needs to call LowLatencyDestroyBuffer() for each buffer that was
                created with LowLatencyCreateBuffer().

                The interface must be open for the pipe to exist. The buf pointer and the frameList
                pointer need to be pre-allocated using LowLatencyCreateBuffer().
                After using them, they should be freed using LowLatencyDestroyBuffer().
       @availability This function is only available with IOUSBInterfaceInterface192 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data, previously allocated with LowLatencyCreateBuffer()
                using a kUSBLowLatencyReadBuffer type.
       @param      frameStart The bus frame number on which to start the read (obtained from GetBusFrameNumber).
       @param      numFrames The number of frames for which to transfer data.
       @param      updateFrequency Specifies how often, in milliseconds, the frame list data should be updated. Valid
                range is 0 - 8. If 0, it means that the framelist should be updated at the end of the transfer.
       @param      frameList A pointer to an array of IOUSBLowLatencyIsocFrame structures describing the frames.
       @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the framelist pointer, which can be used to associate the completion with a particular request. A message addressed to this callback is posted to
                the Async port upon completion.
       @param      refcon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
                kIOReturnNotOpen if the interface is not open for exclusive access.  Will return kIOUSBLowLatencyBufferNotPreviouslyAllocated
                or kIOUSBLowLatencyFrameListNotPreviouslyAllocated if the buffer or the frameList were
                not previously allocated using LowLatencyCreateBuffer().
     */

    IOReturn (* LowLatencyReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                              IOAsyncCallback1 callback, void* refcon);

    /*!
       @function LowLatencyWriteIsochPipeAsync
       @abstract   Performs an asynchronous write on an isochronous pipe and updates the frame list at primary interrupt time.
       @discussion The LowLatencyReadIsochPipeAsync() and LowLatencyWriteIsochPipeAsync()
            calls are analogous to ReadIsochPipeAsync() and WriteIsochPipeAsync().
            They differ in that the frame list data is updated at <em>primary interrupt time</em>. This means that
            the client can inspect the frStatus and frActCount fields as soon as the
            transaction completes, without having to wait for the callback to happen (depending on the value of
            updateFrequency). The callback will still happen when the all the frames have been received.

            The client can specify how often the USB stack should update the frame list data by specifying the
            updateFrequency: this value can range from 0 - 8. If the value is between 1 and 8, the
            frame list will be updated every updateFrequency milliseconds. If the value is 0, the
            frame list will only be updated once all the frames in the transfer have been received. For example,
            consider a transfer with numFrames equal to 64. If the update frequency is 4, the frame
            list data will be updated every 4 milliseconds. If the update frequency is 0, the frame list will
            only be updated at the end of the transfer, after the 64 frames have been sent or received. The
            difference between using an update frequency of 0 and using the non-low latency isoch calls is that
            in the former case, the frame list will be updated at primary interrupt time, while in the latter,
            it will be updated at secondary interrupt time. Regardless of the value of updateFrequency,
            the frame list will <em>always</em> be updated on the last frame of a transfer.

            The rationale for adding this call is that because completion routines run on the USB Workloop,
            they can be scheduled to run a number of milliseconds after the USB transfer has finished. This
            latency is variable and depends on what other higher priority threads are running on the system.
            This latency presents a problem for applications, such as audio processing, that depend on receiving
            data, processing it, and sending it back out, and need to do this as fast as possible. Since applications
            that use isochronous data know when the data should be available, they can look at the frame list at
            the expected time and note the frActCount and frStatus (and frTimeStamp
            if needed) and determine how many valid bytes are in their data buffer and whether there was an error.
            They can then access their data buffer and process the actual data.

            In order to update the frame list at primary interrupt time and to allow the client to see that
            update, the frame list buffer needs to be shared between the kernel and user space. The same thing
            applies to the data buffer. This is a difference between the low latency isoch calls and the regular
            isoch calls. The LowLatencyCreateBuffer() call is used to pre-allocate the buffers. The
            <em>client</em> must use that call to allocate the data and the frame list buffers. The client can
            pass a portion of the buffer that was previously allocated. The USB stack will range-check the data
            and frame list buffers to make sure they are within the ranges of the buffers previously allocated.
            This allows the client, if it so desires, to allocate a large data buffer and pass portions of it to
            the read or write calls. The same applies to the frame list buffers. Of course, the client can also
            pre-allocate several data buffers and several frame list buffers and use those for each transfer.
            Once the transfer completes, the buffers can be reused in subsequent calls. When all transfers are
            finished, the client needs to call LowLatencyDestroyBuffer() for each buffer that was
            created with LowLatencyCreateBuffer().

             The interface must be open for the pipe to exist. The buf pointer and the frameList
            pointer need to be pre-allocated using LowLatencyCreateBuffer(). After using them, they
            should be freed using LowLatencyDestroyBuffer().
       @availability This function is only available with IOUSBInterfaceInterface192 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
       @param      buf Buffer to hold the data, previously allocated with LowLatencyCreateBuffer()
            using a kUSBLowLatencyWriteBuffer type.
       @param      frameStart The bus frame number on which to start the write (obtained from GetBusFrameNumber).
       @param      numFrames The number of frames for which to transfer data.
       @param      updateFrequency Specifies how often, in milliseconds, should the frame list data be updated. Valid
            range is 0 - 8. If 0, it means that the framelist should be updated at the end of the transfer.
       @param      frameList A pointer to an array of IOUSBLowLatencyIsocFrame structures describing the frames.
       @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the framelist pointer, which can be used to associate the completion with a particular request. A message addressed to this callback is posted to
            the Async port upon completion.
       @param      refcon Arbitrary pointer which is passed as a parameter to the callback routine.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
            kIOReturnNotOpen if the interface is not open for exclusive access.  Will return kIOUSBLowLatencyBufferNotPreviouslyAllocated
            or kIOUSBLowLatencyFrameListNotPreviouslyAllocated if the buffer or the frameList were
            not previously allocated using LowLatencyCreateBuffer().
     */

    IOReturn (* LowLatencyWriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                               IOAsyncCallback1 callback, void* refcon);

    /*!
       @function LowLatencyCreateBuffer
       @abstract   Allocates a buffer of type bufferType.
       @discussion This function allocates a buffer of type bufferType. The buffer can then be used with
            the LowLatencyIsochReadPipeAsync() or LowLatencyIsochWritePipeAsync() calls.

            The LowLatencyIsochReadPipeAsync() or LowLatencyIsochWritePipeAsync() calls
            require the clients to pre-allocate the data buffer and the frame list buffer parameters. This call
            is used to allocate those buffers. After the client is done using the buffers, they need to be
            released through the LowLatencyDestroyBuffer() call.

            If the buffer is to be used for reading data, the type passed in should be kUSBLowLatencyReadBuffer.
            If the buffer is to be used for writing data, the type should be kUSBLowLatencyWriteBuffer. For
            frame list data, the type should be kUSBLowLatencyFrameListBuffer.

            The client can create multiple data and frame list buffers, or it can allocate a large buffer and
            then use only a portion of the buffer in calls to LowLatencyReadIsochPipeAsync()
            or LowLatencyWriteIsochPipeAsync().

            The interface must be open for the pipe to exist.
       @availability This function is only available with IOUSBInterfaceInterface192 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      buffer Pointer to a pointer that will receive the pointer to the buffer created by this call.
       @param      size The size of the buffer to be created in bytes.
       @param      bufferType Type of buffer: one of kUSBLowLatencyWriteBuffer, kUSBLowLatencyReadBuffer,
            or kUSBLowLatencyFrameListBuffer. See the documentation for USB.h.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
            kIOReturnNotOpen if the interface is not open for exclusive access.  If the buffer can't be allocated,
            it will return kIOReturnNoMemory.
     */

    IOReturn (* LowLatencyCreateBuffer)(void* self, void** buffer, IOByteCount size, UInt32 bufferType);

    /*!
       @function LowLatencyDestroyBuffer
       @abstract   Releases a buffer that was previously allocated using LowLatencyCreateBuffer().
       @discussion The interface must be open for the pipe to exist.
       @availability This function is only available with IOUSBInterfaceInterface192 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      buffer Pointer to the buffer previously allocated using LowLatencyCreateBuffer().
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
                kIOReturnNotOpen if the interface is not open for exclusive access.  If the buffer was not previously
                allocated using LowLatencyCreateBuffer() it will return kIOReturnBadArgument.
     */

    IOReturn (* LowLatencyDestroyBuffer)(void* self, void* buffer);
} IOUSBInterfaceInterface192;

/*!
   @interface IOUSBInterfaceInterface197
   @abstract   The object you use to access a USB device interface from user space, returned by the IOUSBFamily
        version 1.9.7 and above.
   @discussion The functions listed here include all of the functions defined for the IOUSBInterfaceInterface,
        IOUSBInterfaceInterface182, IOUSBInterfaceInterface183, IOUSBInterfaceInterface190, IOUSBInterfaceInterface192,
        and some new functions that are available on Mac OS X version 10.2.5 and later.
   @super IOUSBInterfaceInterface192
 */

typedef struct IOUSBInterfaceStruct197
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateInterfaceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetInterfaceAsyncEventSource)(void* self);
    IOReturn (* CreateInterfaceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetInterfaceAsyncPort)(void* self);
    IOReturn (* USBInterfaceOpen)(void* self);
    IOReturn (* USBInterfaceClose)(void* self);
    IOReturn (* GetInterfaceClass)(void* self, UInt8* intfClass);
    IOReturn (* GetInterfaceSubClass)(void* self, UInt8* intfSubClass);
    IOReturn (* GetInterfaceProtocol)(void* self, UInt8* intfProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetConfigurationValue)(void* self, UInt8* configVal);
    IOReturn (* GetInterfaceNumber)(void* self, UInt8* intfNumber);
    IOReturn (* GetAlternateSetting)(void* self, UInt8* intfAltSetting);
    IOReturn (* GetNumEndpoints)(void* self, UInt8* intfNumEndpoints);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetDevice)(void* self, io_service_t* device);
    IOReturn (* SetAlternateInterface)(void* self, UInt8 alternateSetting);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ControlRequest)(void* self, UInt8 pipeRef, IOUSBDevRequest* req);
    IOReturn (* ControlRequestAsync)(void* self, UInt8 pipeRef, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* GetPipeProperties)(void* self, UInt8 pipeRef, UInt8* direction, UInt8* number, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* GetPipeStatus)(void* self, UInt8 pipeRef);
    IOReturn (* AbortPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ResetPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ClearPipeStall)(void* self, UInt8 pipeRef);
    IOReturn (* ReadPipe)(void* self, UInt8 pipeRef, void* buf, UInt32* size);
    IOReturn (* WritePipe)(void* self, UInt8 pipeRef, void* buf, UInt32 size);
    IOReturn (* ReadPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                    IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                     IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ControlRequestTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req);
    IOReturn (* ControlRequestAsyncTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* ReadPipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32* size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* WritePipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* ReadPipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* USBInterfaceGetStringIndex)(void* self, UInt8* si);
    IOReturn (* USBInterfaceOpenSeize)(void* self);
    IOReturn (* ClearPipeStallBothEnds)(void* self, UInt8 pipeRef);
    IOReturn (* SetPipePolicy)(void* self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (* GetBandwidthAvailable)(void* self, UInt32* bandwidth);
    IOReturn (* GetEndpointProperties)(void* self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* LowLatencyReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                              IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyWriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                               IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyCreateBuffer)(void* self, void** buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (* LowLatencyDestroyBuffer)(void* self, void* buffer);

    /*!
       @function GetBusMicroFrameNumber
       @abstract   Gets the current micro frame number of the bus to which the interface and its device are attached.
       @discussion The interface does not have to be open to use this function.
       @availability This function is only available with IOUSBInterfaceInterface197 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      microFrame Pointer to UInt64 to hold the microrame number.
       @param      atTime Pointer to an AbsoluteTime, which should be within 1ms of the time when the bus frame number
                was attained.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetBusMicroFrameNumber)(void* self, UInt64* microFrame, AbsoluteTime* atTime);

    /*!
       @function GetFrameListTime
       @abstract   Returns the number of microseconds in each USB Frame.
       @discussion This function can be used to determine whether the device is functioning in full speed or a high
                speed. In the case of a full speed device, the returned value will be kUSBFullSpeedMicrosecondsInFrame.
                In the case of a high speed device, the return value will be kUSBHighSpeedMicrosecondsInFrame.
                (This API should really be called GetUSBFrameTime).

                The interface does not have to be open to use this function.
       @availability This function is only available with IOUSBInterfaceInterface197 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      microsecondsInFrame Pointer to UInt32 to hold the number of microseconds in each USB frame.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetFrameListTime)(void* self, UInt32* microsecondsInFrame);

    /*!
       @function GetIOUSBLibVersion
       @abstract   Returns the version of the IOUSBLib and the version of the IOUSBFamily.
       @discussion The interface does not have to be open to use this function.
       @availability This function is only available with IOUSBInterfaceInterface197 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      ioUSBLibVersion Pointer to a NumVersion structure that on return will contain
                the version of the IOUSBLib.
       @param      usbFamilyVersion Pointer to a NumVersion structure that on return will contain
                the version of the IOUSBFamily.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (* GetIOUSBLibVersion)(void* self, NumVersion* ioUSBLibVersion, NumVersion* usbFamilyVersion);
} IOUSBInterfaceInterface197;

/*!
 @interface IOUSBInterfaceInterface220
 @abstract   The object you use to access a USB device interface from user space, returned by the IOUSBFamily
 version 2.2.0 and above.
 @discussion The functions listed here include all of the functions defined for the IOUSBInterfaceInterface,
 IOUSBInterfaceInterface182, IOUSBInterfaceInterface183, IOUSBInterfaceInterface190, IOUSBInterfaceInterface192
 and IOUSBInterfaceInterface197, as well as some new functions that are available on Mac OS X version 10.4 and later.
 @super IOUSBInterfaceInterface197
 */
typedef struct IOUSBInterfaceStruct220
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateInterfaceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetInterfaceAsyncEventSource)(void* self);
    IOReturn (* CreateInterfaceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetInterfaceAsyncPort)(void* self);
    IOReturn (* USBInterfaceOpen)(void* self);
    IOReturn (* USBInterfaceClose)(void* self);
    IOReturn (* GetInterfaceClass)(void* self, UInt8* intfClass);
    IOReturn (* GetInterfaceSubClass)(void* self, UInt8* intfSubClass);
    IOReturn (* GetInterfaceProtocol)(void* self, UInt8* intfProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetConfigurationValue)(void* self, UInt8* configVal);
    IOReturn (* GetInterfaceNumber)(void* self, UInt8* intfNumber);
    IOReturn (* GetAlternateSetting)(void* self, UInt8* intfAltSetting);
    IOReturn (* GetNumEndpoints)(void* self, UInt8* intfNumEndpoints);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetDevice)(void* self, io_service_t* device);
    IOReturn (* SetAlternateInterface)(void* self, UInt8 alternateSetting);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ControlRequest)(void* self, UInt8 pipeRef, IOUSBDevRequest* req);
    IOReturn (* ControlRequestAsync)(void* self, UInt8 pipeRef, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* GetPipeProperties)(void* self, UInt8 pipeRef, UInt8* direction, UInt8* number, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* GetPipeStatus)(void* self, UInt8 pipeRef);
    IOReturn (* AbortPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ResetPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ClearPipeStall)(void* self, UInt8 pipeRef);
    IOReturn (* ReadPipe)(void* self, UInt8 pipeRef, void* buf, UInt32* size);
    IOReturn (* WritePipe)(void* self, UInt8 pipeRef, void* buf, UInt32 size);
    IOReturn (* ReadPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                    IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                     IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ControlRequestTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req);
    IOReturn (* ControlRequestAsyncTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* ReadPipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32* size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* WritePipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* ReadPipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* USBInterfaceGetStringIndex)(void* self, UInt8* si);
    IOReturn (* USBInterfaceOpenSeize)(void* self);
    IOReturn (* ClearPipeStallBothEnds)(void* self, UInt8 pipeRef);
    IOReturn (* SetPipePolicy)(void* self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (* GetBandwidthAvailable)(void* self, UInt32* bandwidth);
    IOReturn (* GetEndpointProperties)(void* self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* LowLatencyReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                              IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyWriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                               IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyCreateBuffer)(void* self, void** buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (* LowLatencyDestroyBuffer)(void* self, void* buffer);
    IOReturn (* GetBusMicroFrameNumber)(void* self, UInt64* microFrame, AbsoluteTime* atTime);
    IOReturn (* GetFrameListTime)(void* self, UInt32* microsecondsInFrame);
    IOReturn (* GetIOUSBLibVersion)(void* self, NumVersion* ioUSBLibVersion, NumVersion* usbFamilyVersion);

    /*!
        @function FindNextAssociatedDescriptor
       @abstract   Find the next descriptor of the requested type associated with the interface.
       @discussion The interface does not have to be open to use this function.
       @availability This function is only available with IOUSBInterfaceInterface220 and above.
       @param self Pointer to the IOUSBInterfaceInterface.
       @param currentDescriptor Descriptor to start searching from, NULL to start from beginning of list.
       @param descriptorType Descriptor type to search for, or kUSBAnyDesc to return any descriptor type.
       @result Pointer to the descriptor, or NULL if no matching descriptors found.

     */

    IOUSBDescriptorHeader* (*FindNextAssociatedDescriptor)(void* self, const void* currentDescriptor, UInt8 descriptorType);

    /*!
        @function FindNextAltInterface
       @discussion return alternate interface descriptor satisfying the requirements specified in request, or NULL if there aren't any.
       discussion request is updated with the properties of the returned interface.
       @param self Pointer to the IOUSBInterfaceInterface.
       @param current interface descriptor to start searching from, NULL to start at alternate interface 0.
       @param request specifies what properties an interface must have to match.
       @result Pointer to a matching interface descriptor, or NULL if none match.
     */

    IOUSBDescriptorHeader* (*FindNextAltInterface)(void* self, const void* current,
                                                   IOUSBFindInterfaceRequest * request);
} IOUSBInterfaceInterface220;

/*!
 @interface IOUSBInterfaceInterface245
 @abstract   The object you use to access a USB device interface from user space, returned by the IOUSBFamily
 version 2.4.5 and above.
 @discussion The functions listed here include all of the functions defined for the IOUSBInterfaceInterface,
 IOUSBInterfaceInterface182, IOUSBInterfaceInterface183, IOUSBInterfaceInterface190, IOUSBInterfaceInterface192,
 IOUSBInterfaceInterface197 and IOUSBInterfaceInterface220, as well as some new functions that are available on
 Mac OS X version 10.4.6 and later.
 @super IOUSBInterfaceInterface220
 */
typedef struct IOUSBInterfaceStruct245
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateInterfaceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetInterfaceAsyncEventSource)(void* self);
    IOReturn (* CreateInterfaceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetInterfaceAsyncPort)(void* self);
    IOReturn (* USBInterfaceOpen)(void* self);
    IOReturn (* USBInterfaceClose)(void* self);
    IOReturn (* GetInterfaceClass)(void* self, UInt8* intfClass);
    IOReturn (* GetInterfaceSubClass)(void* self, UInt8* intfSubClass);
    IOReturn (* GetInterfaceProtocol)(void* self, UInt8* intfProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetConfigurationValue)(void* self, UInt8* configVal);
    IOReturn (* GetInterfaceNumber)(void* self, UInt8* intfNumber);
    IOReturn (* GetAlternateSetting)(void* self, UInt8* intfAltSetting);
    IOReturn (* GetNumEndpoints)(void* self, UInt8* intfNumEndpoints);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetDevice)(void* self, io_service_t* device);
    IOReturn (* SetAlternateInterface)(void* self, UInt8 alternateSetting);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ControlRequest)(void* self, UInt8 pipeRef, IOUSBDevRequest* req);
    IOReturn (* ControlRequestAsync)(void* self, UInt8 pipeRef, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* GetPipeProperties)(void* self, UInt8 pipeRef, UInt8* direction, UInt8* number, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* GetPipeStatus)(void* self, UInt8 pipeRef);
    IOReturn (* AbortPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ResetPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ClearPipeStall)(void* self, UInt8 pipeRef);
    IOReturn (* ReadPipe)(void* self, UInt8 pipeRef, void* buf, UInt32* size);
    IOReturn (* WritePipe)(void* self, UInt8 pipeRef, void* buf, UInt32 size);
    IOReturn (* ReadPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                    IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                     IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ControlRequestTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req);
    IOReturn (* ControlRequestAsyncTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* ReadPipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32* size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* WritePipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* ReadPipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* USBInterfaceGetStringIndex)(void* self, UInt8* si);
    IOReturn (* USBInterfaceOpenSeize)(void* self);
    IOReturn (* ClearPipeStallBothEnds)(void* self, UInt8 pipeRef);
    IOReturn (* SetPipePolicy)(void* self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (* GetBandwidthAvailable)(void* self, UInt32* bandwidth);
    IOReturn (* GetEndpointProperties)(void* self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* LowLatencyReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                              IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyWriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                               IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyCreateBuffer)(void* self, void** buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (* LowLatencyDestroyBuffer)(void* self, void* buffer);
    IOReturn (* GetBusMicroFrameNumber)(void* self, UInt64* microFrame, AbsoluteTime* atTime);
    IOReturn (* GetFrameListTime)(void* self, UInt32* microsecondsInFrame);
    IOReturn (* GetIOUSBLibVersion)(void* self, NumVersion* ioUSBLibVersion, NumVersion* usbFamilyVersion);
    IOUSBDescriptorHeader* (*FindNextAssociatedDescriptor)(void* self, const void* currentDescriptor, UInt8 descriptorType);
    IOUSBDescriptorHeader* (*FindNextAltInterface)(void* self, const void* current, IOUSBFindInterfaceRequest * request);
} IOUSBInterfaceInterface245;

/*!
 @interface IOUSBInterfaceInterface300
 @abstract   The object you use to access a USB device interface from user space, returned by the IOUSBFamily
 version 3.0.0 and above.
 @discussion The functions listed here include all of the functions defined for the IOUSBInterfaceInterface,
 IOUSBInterfaceInterface182, IOUSBInterfaceInterface183, IOUSBInterfaceInterface190, IOUSBInterfaceInterface192,
 IOUSBInterfaceInterface197, IOUSBInterfaceInterface220 and IOUSBInterfaceInterface245, as well as some new functions
 that are available on Mac OS X version 10.5 and later.
 @super IOUSBInterfaceInterface245
 */
typedef struct IOUSBInterfaceStruct300
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateInterfaceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetInterfaceAsyncEventSource)(void* self);
    IOReturn (* CreateInterfaceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetInterfaceAsyncPort)(void* self);
    IOReturn (* USBInterfaceOpen)(void* self);
    IOReturn (* USBInterfaceClose)(void* self);
    IOReturn (* GetInterfaceClass)(void* self, UInt8* intfClass);
    IOReturn (* GetInterfaceSubClass)(void* self, UInt8* intfSubClass);
    IOReturn (* GetInterfaceProtocol)(void* self, UInt8* intfProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetConfigurationValue)(void* self, UInt8* configVal);
    IOReturn (* GetInterfaceNumber)(void* self, UInt8* intfNumber);
    IOReturn (* GetAlternateSetting)(void* self, UInt8* intfAltSetting);
    IOReturn (* GetNumEndpoints)(void* self, UInt8* intfNumEndpoints);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetDevice)(void* self, io_service_t* device);
    IOReturn (* SetAlternateInterface)(void* self, UInt8 alternateSetting);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ControlRequest)(void* self, UInt8 pipeRef, IOUSBDevRequest* req);
    IOReturn (* ControlRequestAsync)(void* self, UInt8 pipeRef, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* GetPipeProperties)(void* self, UInt8 pipeRef, UInt8* direction, UInt8* number, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* GetPipeStatus)(void* self, UInt8 pipeRef);
    IOReturn (* AbortPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ResetPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ClearPipeStall)(void* self, UInt8 pipeRef);
    IOReturn (* ReadPipe)(void* self, UInt8 pipeRef, void* buf, UInt32* size);
    IOReturn (* WritePipe)(void* self, UInt8 pipeRef, void* buf, UInt32 size);
    IOReturn (* ReadPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                    IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                     IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ControlRequestTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req);
    IOReturn (* ControlRequestAsyncTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* ReadPipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32* size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* WritePipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* ReadPipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* USBInterfaceGetStringIndex)(void* self, UInt8* si);
    IOReturn (* USBInterfaceOpenSeize)(void* self);
    IOReturn (* ClearPipeStallBothEnds)(void* self, UInt8 pipeRef);
    IOReturn (* SetPipePolicy)(void* self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (* GetBandwidthAvailable)(void* self, UInt32* bandwidth);
    IOReturn (* GetEndpointProperties)(void* self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* LowLatencyReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                              IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyWriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                               IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyCreateBuffer)(void* self, void** buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (* LowLatencyDestroyBuffer)(void* self, void* buffer);
    IOReturn (* GetBusMicroFrameNumber)(void* self, UInt64* microFrame, AbsoluteTime* atTime);
    IOReturn (* GetFrameListTime)(void* self, UInt32* microsecondsInFrame);
    IOReturn (* GetIOUSBLibVersion)(void* self, NumVersion* ioUSBLibVersion, NumVersion* usbFamilyVersion);
    IOUSBDescriptorHeader* (*FindNextAssociatedDescriptor)(void* self, const void* currentDescriptor, UInt8 descriptorType);
    IOUSBDescriptorHeader* (*FindNextAltInterface)(void* self, const void* current, IOUSBFindInterfaceRequest * request);

    /*!
       @function GetBusFrameNumberWithTime
       @abstract   Gets a recent frame number of the bus to which the device is attached, along with a system time corresponding to the start of that frame
       @discussion The device does not have to be open to use this function.
       @availability This function is only available with IOUSBInterfaceInterface300 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      frame Pointer to UInt64 to hold the frame number.
       @param      atTime Pointer to a returned AbsoluteTime, which is the system time ("wall time") as close as possible to the beginning of that USB frame. The jitter on this value may be as much as 200 microseconds.
        @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or kIOReturnUnsupported is the bus doesn't support this function.
     */

    IOReturn (* GetBusFrameNumberWithTime)(void* self, UInt64* frame, AbsoluteTime* atTime);
} IOUSBInterfaceInterface300;

/*!
 @interface IOUSBInterfaceInterface398
 @abstract   The object you use to access a USB interface from user space, returned by the IOUSBFamily version 3.9.8 and above.
 @discussion This object is functionally identical to IOUSBInterfaceInterface300 on macOS, and includes some new functions that are only available on iOS.
 @super IOUSBInterfaceInterface300
 */
typedef struct IOUSBInterfaceStruct398
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateInterfaceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetInterfaceAsyncEventSource)(void* self);
    IOReturn (* CreateInterfaceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetInterfaceAsyncPort)(void* self);
    IOReturn (* USBInterfaceOpen)(void* self);
    IOReturn (* USBInterfaceClose)(void* self);
    IOReturn (* GetInterfaceClass)(void* self, UInt8* intfClass);
    IOReturn (* GetInterfaceSubClass)(void* self, UInt8* intfSubClass);
    IOReturn (* GetInterfaceProtocol)(void* self, UInt8* intfProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetConfigurationValue)(void* self, UInt8* configVal);
    IOReturn (* GetInterfaceNumber)(void* self, UInt8* intfNumber);
    IOReturn (* GetAlternateSetting)(void* self, UInt8* intfAltSetting);
    IOReturn (* GetNumEndpoints)(void* self, UInt8* intfNumEndpoints);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetDevice)(void* self, io_service_t* device);
    IOReturn (* SetAlternateInterface)(void* self, UInt8 alternateSetting);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ControlRequest)(void* self, UInt8 pipeRef, IOUSBDevRequest* req);
    IOReturn (* ControlRequestAsync)(void* self, UInt8 pipeRef, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* GetPipeProperties)(void* self, UInt8 pipeRef, UInt8* direction, UInt8* number, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* GetPipeStatus)(void* self, UInt8 pipeRef);
    IOReturn (* AbortPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ResetPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ClearPipeStall)(void* self, UInt8 pipeRef);
    IOReturn (* ReadPipe)(void* self, UInt8 pipeRef, void* buf, UInt32* size);
    IOReturn (* WritePipe)(void* self, UInt8 pipeRef, void* buf, UInt32 size);
    IOReturn (* ReadPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                    IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                     IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ControlRequestTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req);
    IOReturn (* ControlRequestAsyncTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* ReadPipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32* size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* WritePipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* ReadPipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* USBInterfaceGetStringIndex)(void* self, UInt8* si);
    IOReturn (* USBInterfaceOpenSeize)(void* self);
    IOReturn (* ClearPipeStallBothEnds)(void* self, UInt8 pipeRef);
    IOReturn (* SetPipePolicy)(void* self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (* GetBandwidthAvailable)(void* self, UInt32* bandwidth);
    IOReturn (* GetEndpointProperties)(void* self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* LowLatencyReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                              IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyWriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                               IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyCreateBuffer)(void* self, void** buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (* LowLatencyDestroyBuffer)(void* self, void* buffer);
    IOReturn (* GetBusMicroFrameNumber)(void* self, UInt64* microFrame, AbsoluteTime* atTime);
    IOReturn (* GetFrameListTime)(void* self, UInt32* microsecondsInFrame);
    IOReturn (* GetIOUSBLibVersion)(void* self, NumVersion* ioUSBLibVersion, NumVersion* usbFamilyVersion);
    IOUSBDescriptorHeader* (*FindNextAssociatedDescriptor)(void* self, const void* currentDescriptor, UInt8 descriptorType);
    IOUSBDescriptorHeader* (*FindNextAltInterface)(void* self, const void* current, IOUSBFindInterfaceRequest * request);
    IOReturn (* GetBusFrameNumberWithTime)(void* self, UInt64* frame, AbsoluteTime* atTime);

#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    /*!
       @function SetIdlePolicy
       @abstract   Define an idling policy for the interface
       @discussion The device does not have to be open to use this function.
       @availability This function is only available with IOUSBInterfaceInterface398 and above.
       @param      self Pointer to the IOUSBInterfaceInterface.
       @param      deviceIdleTimeout The amount of time in ms a device is idle (no IO, or all pending IOs have had the ioIdleTimeout period elapse) before it can be suspended.  Use 0 to prevent suspending an idle device.
       @param      ioIdleTimeout The amount of time in ms an IO is pending before it allows the device to be considered idle.  Use 0 to prevent idling the device while the IO is in progress, regardless of how long it takes.
       @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or kIOReturnUnsupported is the bus doesn't support this function.
     */
    IOReturn (* SetIdlePolicy)(void* self, UInt32 deviceIdleTimeout, UInt32 ioIdleTimeout);

    IOReturn (* OverrideIdlePolicy)(void* self, UInt8 pipeRef, bool override, UInt32 ioIdleTimeout);
#endif
#endif

} IOUSBInterfaceInterface398;

/*!
 @interface IOUSBInterfaceInterface400
 @abstract   The object you use to access a USB interface from user space, returned by the IOUSBFamily version 4.0.0 and above.
 @discussion This object is functionally identical to IOUSBInterfaceInterface300 on macOS, and includes some new functions that are only available on iOS.
 @super IOUSBInterfaceInterface398
 */
typedef struct IOUSBInterfaceStruct400
{
    IUNKNOWN_C_GUTS;
    IOReturn (* CreateInterfaceAsyncEventSource)(void* self, CFRunLoopSourceRef* source);
    CFRunLoopSourceRef (* GetInterfaceAsyncEventSource)(void* self);
    IOReturn (* CreateInterfaceAsyncPort)(void* self, mach_port_t* port);
    mach_port_t (* GetInterfaceAsyncPort)(void* self);
    IOReturn (* USBInterfaceOpen)(void* self);
    IOReturn (* USBInterfaceClose)(void* self);
    IOReturn (* GetInterfaceClass)(void* self, UInt8* intfClass);
    IOReturn (* GetInterfaceSubClass)(void* self, UInt8* intfSubClass);
    IOReturn (* GetInterfaceProtocol)(void* self, UInt8* intfProtocol);
    IOReturn (* GetDeviceVendor)(void* self, UInt16* devVendor);
    IOReturn (* GetDeviceProduct)(void* self, UInt16* devProduct);
    IOReturn (* GetDeviceReleaseNumber)(void* self, UInt16* devRelNum);
    IOReturn (* GetConfigurationValue)(void* self, UInt8* configVal);
    IOReturn (* GetInterfaceNumber)(void* self, UInt8* intfNumber);
    IOReturn (* GetAlternateSetting)(void* self, UInt8* intfAltSetting);
    IOReturn (* GetNumEndpoints)(void* self, UInt8* intfNumEndpoints);
    IOReturn (* GetLocationID)(void* self, UInt32* locationID);
    IOReturn (* GetDevice)(void* self, io_service_t* device);
    IOReturn (* SetAlternateInterface)(void* self, UInt8 alternateSetting);
    IOReturn (* GetBusFrameNumber)(void* self, UInt64* frame, AbsoluteTime* atTime);
    IOReturn (* ControlRequest)(void* self, UInt8 pipeRef, IOUSBDevRequest* req);
    IOReturn (* ControlRequestAsync)(void* self, UInt8 pipeRef, IOUSBDevRequest* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* GetPipeProperties)(void* self, UInt8 pipeRef, UInt8* direction, UInt8* number, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* GetPipeStatus)(void* self, UInt8 pipeRef);
    IOReturn (* AbortPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ResetPipe)(void* self, UInt8 pipeRef);
    IOReturn (* ClearPipeStall)(void* self, UInt8 pipeRef);
    IOReturn (* ReadPipe)(void* self, UInt8 pipeRef, void* buf, UInt32* size);
    IOReturn (* WritePipe)(void* self, UInt8 pipeRef, void* buf, UInt32 size);
    IOReturn (* ReadPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt32 size, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                    IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame* frameList,
                                     IOAsyncCallback1 callback, void* refcon);
    IOReturn (* ControlRequestTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req);
    IOReturn (* ControlRequestAsyncTO)(void* self, UInt8 pipeRef, IOUSBDevRequestTO* req, IOAsyncCallback1 callback, void* refCon);
    IOReturn (* ReadPipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32* size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* WritePipeTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (* ReadPipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* WritePipeAsyncTO)(void* self, UInt8 pipeRef, void* buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void* refcon);
    IOReturn (* USBInterfaceGetStringIndex)(void* self, UInt8* si);
    IOReturn (* USBInterfaceOpenSeize)(void* self);
    IOReturn (* ClearPipeStallBothEnds)(void* self, UInt8 pipeRef);
    IOReturn (* SetPipePolicy)(void* self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (* GetBandwidthAvailable)(void* self, UInt32* bandwidth);
    IOReturn (* GetEndpointProperties)(void* self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8* transferType, UInt16* maxPacketSize, UInt8* interval);
    IOReturn (* LowLatencyReadIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                              IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyWriteIsochPipeAsync)(void* self, UInt8 pipeRef, void* buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame* frameList,
                                               IOAsyncCallback1 callback, void* refcon);
    IOReturn (* LowLatencyCreateBuffer)(void* self, void** buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (* LowLatencyDestroyBuffer)(void* self, void* buffer);
    IOReturn (* GetBusMicroFrameNumber)(void* self, UInt64* microFrame, AbsoluteTime* atTime);
    IOReturn (* GetFrameListTime)(void* self, UInt32* microsecondsInFrame);
    IOReturn (* GetIOUSBLibVersion)(void* self, NumVersion* ioUSBLibVersion, NumVersion* usbFamilyVersion);
    IOUSBDescriptorHeader* (*FindNextAssociatedDescriptor)(void* self, const void* currentDescriptor, UInt8 descriptorType);
    IOUSBDescriptorHeader* (*FindNextAltInterface)(void* self, const void* current, IOUSBFindInterfaceRequest * request);
    IOReturn (* GetBusFrameNumberWithTime)(void* self, UInt64* frame, AbsoluteTime* atTime);
#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    IOReturn (* SetIdlePolicy)(void* self, UInt32 deviceIdleTimeout, UInt32 ioIdleTimeout);
    IOReturn (* OverrideIdlePolicy)(void* self, UInt8 pipeRef, bool override, UInt32 ioIdleTimeout);

    /*!
     @function   GetInterfaceAsyncNotificationPort
     @abstract   Returns the IONotificationPort for this IOService instance.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @result     Returns the IONotificationPortRef if one exists, MACH_PORT_NULL otherwise.
     */
    IONotificationPortRef (* GetInterfaceAsyncNotificationPort)(void* self);
#endif
#endif
} IOUSBInterfaceInterface400;

/*!
 @interface IOUSBInterfaceInterface500
 @abstract   The object you use to access a USB device interface from user space, returned by the IOUSBFamily
 version 5.0.0 and above.
 @discussion The functions listed here include all of the functions defined for IOUSBInterfaceInterface400, as well as some new functions that are available on Mac OS X version 10.7.3 and later.
 @super IOUSBInterfaceInterface400
 */
typedef struct IOUSBInterfaceStruct500 {
    IUNKNOWN_C_GUTS;
    IOReturn (*CreateInterfaceAsyncEventSource)(void *self, CFRunLoopSourceRef *source);
    CFRunLoopSourceRef (*GetInterfaceAsyncEventSource)(void *self);
    IOReturn (*CreateInterfaceAsyncPort)(void *self, mach_port_t *port);
    mach_port_t (*GetInterfaceAsyncPort)(void *self);
    IOReturn (*USBInterfaceOpen)(void *self);
    IOReturn (*USBInterfaceClose)(void *self);
    IOReturn (*GetInterfaceClass)(void *self, UInt8 *intfClass);
    IOReturn (*GetInterfaceSubClass)(void *self, UInt8 *intfSubClass);
    IOReturn (*GetInterfaceProtocol)(void *self, UInt8 *intfProtocol);
    IOReturn (*GetDeviceVendor)(void *self, UInt16 *devVendor);
    IOReturn (*GetDeviceProduct)(void *self, UInt16 *devProduct);
    IOReturn (*GetDeviceReleaseNumber)(void *self, UInt16 *devRelNum);
    IOReturn (*GetConfigurationValue)(void *self, UInt8 *configVal);
    IOReturn (*GetInterfaceNumber)(void *self, UInt8 *intfNumber);
    IOReturn (*GetAlternateSetting)(void *self, UInt8 *intfAltSetting);
    IOReturn (*GetNumEndpoints)(void *self, UInt8 *intfNumEndpoints);
    IOReturn (*GetLocationID)(void *self, UInt32 *locationID);
    IOReturn (*GetDevice)(void *self, io_service_t *device);
    IOReturn (*SetAlternateInterface)(void *self, UInt8 alternateSetting);
    IOReturn (*GetBusFrameNumber)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*ControlRequest)(void *self, UInt8 pipeRef, IOUSBDevRequest *req);
    IOReturn (*ControlRequestAsync)(void *self, UInt8 pipeRef, IOUSBDevRequest *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*GetPipeProperties)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*GetPipeStatus)(void *self, UInt8 pipeRef);
    IOReturn (*AbortPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ResetPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ClearPipeStall)(void *self, UInt8 pipeRef);
    IOReturn (*ReadPipe)(void *self, UInt8 pipeRef, void *buf, UInt32 *size);
    IOReturn (*WritePipe)(void *self, UInt8 pipeRef, void *buf, UInt32 size);
    IOReturn (*ReadPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                   IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                    IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ControlRequestTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req);
    IOReturn (*ControlRequestAsyncTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*ReadPipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 *size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*WritePipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*ReadPipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*USBInterfaceGetStringIndex)(void *self, UInt8 *si);
    IOReturn (*USBInterfaceOpenSeize)(void *self);
    IOReturn (*ClearPipeStallBothEnds)(void *self, UInt8 pipeRef);
    IOReturn (*SetPipePolicy)(void *self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (*GetBandwidthAvailable)(void *self, UInt32 *bandwidth);
    IOReturn (*GetEndpointProperties)(void *self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*LowLatencyReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                             IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyWriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                              IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyCreateBuffer)(void * self, void **buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (*LowLatencyDestroyBuffer) (void * self, void * buffer );
    IOReturn (*GetBusMicroFrameNumber)(void *self, UInt64 *microFrame, AbsoluteTime *atTime);
    IOReturn (*GetFrameListTime)(void *self, UInt32 *microsecondsInFrame);
    IOReturn (*GetIOUSBLibVersion)(void *self, NumVersion *ioUSBLibVersion, NumVersion *usbFamilyVersion);
    IOUSBDescriptorHeader * (*FindNextAssociatedDescriptor)(void *self, const void *currentDescriptor, UInt8 descriptorType);
    IOUSBDescriptorHeader * (*FindNextAltInterface)(void *self, const void *current, IOUSBFindInterfaceRequest *request);
    IOReturn (*GetBusFrameNumberWithTime)(void *self, UInt64 *frame, AbsoluteTime *atTime);
#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    IOReturn (* SetIdlePolicy)(void* self, UInt32 deviceIdleTimeout, UInt32 ioIdleTimeout);
    IOReturn (* OverrideIdlePolicy)(void* self, UInt8 pipeRef, bool override, UInt32 ioIdleTimeout);
    IONotificationPortRef (* GetInterfaceAsyncNotificationPort)(void* self);
#endif
#endif

    /*!
     @function GetPipePropertiesV2
     @abstract   This function is deprecated. See GetPipePropertiesV3
     */
    IOReturn (*GetPipePropertiesV2)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval, UInt8 *maxBurst, UInt8 *mult, UInt16 *bytesPerInterval);
} IOUSBInterfaceInterface500;





/*!
 @interface IOUSBInterfaceInterface550
 @abstract   The object you use to access a USB device interface from user space, returned by the IOUSBFamily
 version 5.5.0 and above.
 @discussion The functions listed here include all of the functions defined for IOUSBInterfaceInterface500, as well as some new functions that are available on Mac OS X version 10.8.2 and later.
 @super IOUSBInterfaceInterface500
 */

typedef struct IOUSBInterfaceStruct550 {
    IUNKNOWN_C_GUTS;
    IOReturn (*CreateInterfaceAsyncEventSource)(void *self, CFRunLoopSourceRef *source);
    CFRunLoopSourceRef (*GetInterfaceAsyncEventSource)(void *self);
    IOReturn (*CreateInterfaceAsyncPort)(void *self, mach_port_t *port);
    mach_port_t (*GetInterfaceAsyncPort)(void *self);
    IOReturn (*USBInterfaceOpen)(void *self);
    IOReturn (*USBInterfaceClose)(void *self);
    IOReturn (*GetInterfaceClass)(void *self, UInt8 *intfClass);
    IOReturn (*GetInterfaceSubClass)(void *self, UInt8 *intfSubClass);
    IOReturn (*GetInterfaceProtocol)(void *self, UInt8 *intfProtocol);
    IOReturn (*GetDeviceVendor)(void *self, UInt16 *devVendor);
    IOReturn (*GetDeviceProduct)(void *self, UInt16 *devProduct);
    IOReturn (*GetDeviceReleaseNumber)(void *self, UInt16 *devRelNum);
    IOReturn (*GetConfigurationValue)(void *self, UInt8 *configVal);
    IOReturn (*GetInterfaceNumber)(void *self, UInt8 *intfNumber);
    IOReturn (*GetAlternateSetting)(void *self, UInt8 *intfAltSetting);
    IOReturn (*GetNumEndpoints)(void *self, UInt8 *intfNumEndpoints);
    IOReturn (*GetLocationID)(void *self, UInt32 *locationID);
    IOReturn (*GetDevice)(void *self, io_service_t *device);
    IOReturn (*SetAlternateInterface)(void *self, UInt8 alternateSetting);
    IOReturn (*GetBusFrameNumber)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*ControlRequest)(void *self, UInt8 pipeRef, IOUSBDevRequest *req);
    IOReturn (*ControlRequestAsync)(void *self, UInt8 pipeRef, IOUSBDevRequest *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*GetPipeProperties)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*GetPipeStatus)(void *self, UInt8 pipeRef);
    IOReturn (*AbortPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ResetPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ClearPipeStall)(void *self, UInt8 pipeRef);
    IOReturn (*ReadPipe)(void *self, UInt8 pipeRef, void *buf, UInt32 *size);
    IOReturn (*WritePipe)(void *self, UInt8 pipeRef, void *buf, UInt32 size);
    IOReturn (*ReadPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                   IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                    IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ControlRequestTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req);
    IOReturn (*ControlRequestAsyncTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*ReadPipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 *size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*WritePipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*ReadPipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*USBInterfaceGetStringIndex)(void *self, UInt8 *si);
    IOReturn (*USBInterfaceOpenSeize)(void *self);
    IOReturn (*ClearPipeStallBothEnds)(void *self, UInt8 pipeRef);
    IOReturn (*SetPipePolicy)(void *self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (*GetBandwidthAvailable)(void *self, UInt32 *bandwidth);
    IOReturn (*GetEndpointProperties)(void *self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*LowLatencyReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                             IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyWriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                              IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyCreateBuffer)(void * self, void **buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (*LowLatencyDestroyBuffer) (void * self, void * buffer );
    IOReturn (*GetBusMicroFrameNumber)(void *self, UInt64 *microFrame, AbsoluteTime *atTime);
    IOReturn (*GetFrameListTime)(void *self, UInt32 *microsecondsInFrame);
    IOReturn (*GetIOUSBLibVersion)(void *self, NumVersion *ioUSBLibVersion, NumVersion *usbFamilyVersion);
    IOUSBDescriptorHeader * (*FindNextAssociatedDescriptor)(void *self, const void *currentDescriptor, UInt8 descriptorType);
    IOUSBDescriptorHeader * (*FindNextAltInterface)(void *self, const void *current, IOUSBFindInterfaceRequest *request);
    IOReturn (*GetBusFrameNumberWithTime)(void *self, UInt64 *frame, AbsoluteTime *atTime);
#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    IOReturn (* SetIdlePolicy)(void* self, UInt32 deviceIdleTimeout, UInt32 ioIdleTimeout);
    IOReturn (* OverrideIdlePolicy)(void* self, UInt8 pipeRef, bool override, UInt32 ioIdleTimeout);
    IONotificationPortRef (* GetInterfaceAsyncNotificationPort)(void* self);
#endif
#endif
    IOReturn (*GetPipePropertiesV2)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval, UInt8 *maxBurst, UInt8 *mult, UInt16 *bytesPerInterval);

    /*!
     @function GetPipePropertiesV3
     @abstract   Gets the different properties for a pipe.  This API uses a pointer to IOUSBEndpointProperties to return all the different properties.
     @discussion Once an interface is opened, all of the pipes in that interface get created by the kernel. The number
     of pipes can be retrieved by GetNumEndpoints. The client can then get the properties of any pipe
     using an index of 1 to GetNumEndpoints. Pipe 0 is the default control pipe in the device.
     @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
     @param properties  pointer to a IOUSBEndpointProperties that will contain all the endpoint parameters.  Initialize the bVersion field with the appropriate version.  (See USBGetEndpointVersion in USB.h).  The bMaxStreams
     field, if valid, is the actual number of streams that are supported for this pipe (e.g. it takes into account what the USB controller supports, as well as what the endpoint supports). The wMaxPacketSize is the
     current FULL maxPacketSize for this pipe, which includes both the mult and the burst. It may have been changed by SetPipePolicy, or it could be 0 as a result of a lack of bandwidth.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
     or kIOReturnNotOpen if the interface is not open for exclusive access.
     */
    IOReturn (*GetPipePropertiesV3)(void *self, UInt8 pipeRef, IOUSBEndpointProperties *properties);

    /*!
     @function GetEndpointPropertiesV3
     @abstract Returns the properties of an endpoint, possibly in an alternate interface, including any information from the SuperSpeed Companion Descriptor
     @param properties  pointer to a IOUSBEndpointProperties that will contain all the endpoint parameters.  Initialize the bVersion field with the appropriate version.  (See USBGetEndpointVersion in USB.h).  Initialize
     the bAlternateSetting, bEndpointNumber, and bDirection fields of the structure with the desired values for the endpoint.
     @param properties  pointer to a IOUSBEndpointProperties that will contain all the endpoint parameters.  Initialize the bVersion field with the appropriate version.  (See USBGetEndpointVersion in USB.h).  You also NEED
     to initialize the bAlternateSetting, the bDirection (kUSBIn or kUSBOut), and the bEndPointNumber with the desired values for the endpoint. The bMaxStreams field, if valid, is the number of streams found in the Super Speed
     Companion Descriptor. The wMaxPacketSize is the BASE maxPacketSize as found in the endpoint descriptor, and has not been multiplied to take into account burst and mult.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
     or kIOReturnNotOpen if the interface is not open for exclusive access.
     */
    IOReturn (*GetEndpointPropertiesV3)(void *self, IOUSBEndpointProperties *properties);

    /*!
     @function SupportsStreams
     @abstract   Returns non zero if the pipe supports streams, nonZero for the maximum supported streams
     @discussion The interface does not have to be open to use this function.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
     @param         supportsStreams 0 if streams not supported, non-zero value indicates maximum stream number
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (*SupportsStreams)(void *self, UInt8 pipeRef, UInt32 *supportsStreams);

    /*!
     @function CreateStreams
     @abstract   Creates the streams for the pipe
     @discussion The interface does not have to be open to use this function.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
     @param         streamID pass 0 if you want to destroy all streams, non-zero value indicates streamID of the highest stream to create
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (*CreateStreams)(void *self, UInt8 pipeRef, UInt32 streamID);

    /*!
     @function GetConfiguredStreams
     @abstract   Get the number of streams which have been configured for the endpoint with CreateStreams
     @discussion The interface does not have to be open to use this function.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
     @param         configuredStreams  Number of streams that have been configured with CreateStreams
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */

    IOReturn (*GetConfiguredStreams)(void *self, UInt8 pipeRef, UInt32 *configuredStreams);

    /*!
     @function ReadStreamsPipeTO
     @abstract   Performs a read on a stream in a <b>BULK IN</b> pipe, specifying timeout values.
     @discussion The interface must be open for the pipe to exist.

     If a timeout is specified and the request times out, the driver may need to resynchronize the data
     toggle. See @link //apple_ref/C/instm/IOUSBInterfaceInterface/ClearPipeStall/ ClearPipeStall @/link
     or @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link.


     Timeouts do not apply to interrupt pipes, so you should use the ReadPipe API to perform a read from
     an interrupt pipe.
     @availability This function is only available with IOUSBInterfaceInterface182 and above.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
     @param      streamID ID of the stream to read from
     @param      buf Buffer to hold the data.
     @param      size Pointer to the size of the buffer pointed to by buf.
     @param      noDataTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if no
     data is transferred in this amount of time, the request will be aborted and returned.
     @param      completionTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if
     the entire request is not completed in this amount of time, the request will be aborted and returned.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
     kIOReturnAborted if the thread is interrupted before the call completes, or
     kIOReturnNotOpen if the interface is not open for exclusive access.  Returns kIOReturnBadArgument if timeout
     values are specified for an interrupt pipe.  If an error is returned, the size parameter is not updated and the buffer will
     NOT contain any valid data.
     */
    IOReturn (*ReadStreamsPipeTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 *size, UInt32 noDataTimeout, UInt32 completionTimeout);

    /*!
     @function WriteStreamsPipeTO
     @abstract   Performs an write on a stream on a<b>BULK OUT</b> pipe, with specified timeout values.
     @discussion The interface must be open for the pipe to exist.

     If a timeout is specified and the request times out, the driver may need to resynchronize the data
     toggle. See @link //apple_ref/C/instm/IOUSBInterfaceInterface/ClearPipeStall/ ClearPipeStall @/link
     or @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link.
     @availability This function is only available with IOUSBInterfaceInterface182 and above.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
     @param      streamID ID of the stream to write to
     @param      buf Buffer to hold the data.
     @param      size The size of the buffer pointed to by buf.
     @param      noDataTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if no
     data is transferred in this amount of time, the request will be aborted and returned.
     @param      completionTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if
     the entire request is not completed in this amount of time, the request will be aborted and returned.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
     kIOReturnAborted if the thread is interrupted before the call completes, or
     kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (*WriteStreamsPipeTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);

    /*!
     @function ReadStreamsPipeAsyncTO
     @abstract   Performs an asynchronous read on a stream on a <b>BULK IN </b>pipe, with specified timeout values.
     @discussion The interface must be open for the pipe to exist.

     If a timeout is specified and the request times out, the driver may need to resynchronize the data
     toggle. See @link //apple_ref/C/instm/IOUSBInterfaceInterface/ClearPipeStall/ ClearPipeStall @/link
     or @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link.

     Timeouts do not apply to interrupt pipes, so you should use the ReadPipeAsync API to perform an
     asynchronous read from an interrupt pipe.
     @availability This function is only available with IOUSBInterfaceInterface182 and above.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
     @param      streamID ID of the stream to read from
     @param      buf Buffer to hold the data.
     @param      size The size of the buffer pointed to by buf.
     @param      noDataTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if no
     data is transferred in this amount of time, the request will be aborted and returned.
     @param      completionTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if
     the entire request is not completed in this amount of time, the request will be aborted and returned.
     @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the number of bytes that were actually read.
     A message addressed to this callback is posted to the Async port
     upon completion.
     @param      refcon Arbitrary pointer which is passed as a parameter to the callback routine.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
     kIOReturnNotOpen if the interface is not open for exclusive access.  Returns kIOReturnBadArgument if timeout
     values are specified for an interrupt pipe.  If an error is returned, the size parameter is not updated and the buffer will
     NOT contain any valid data.
     */

    IOReturn (*ReadStreamsPipeAsyncTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);

    /*!
     @function WriteStreamsPipeAsyncTO
     @abstract   Performs an asynchronous write on a stream on a <b>BULK OUT</b> pipe, with specified timeout values.
     @discussion The interface must be open for the pipe to exist.

     If a timeout is specified and the request times out, the driver may need to resynchronize the data
     toggle. See @link //apple_ref/C/instm/IOUSBInterfaceInterface/ClearPipeStall/ ClearPipeStall @/link
     or @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link.
     @availability This function is only available with IOUSBInterfaceInterface182 and above.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
     @param      streamID ID of the stream to write to
     @param      buf Buffer to hold the data.
     @param      size The size of the buffer pointed to by buf.
     @param      noDataTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if no
     data is transferred in this amount of time, the request will be aborted and returned.
     @param      completionTimeout Specifies a time value in milliseconds. Once the request is queued on the bus, if
     the entire request is not completed in this amount of time, the request will be aborted and returned.
     @param      callback An IOAsyncCallback1 method. Upon completion, the arg0 argument of the AsyncCallback1 will contain the number of bytes that were actually written.
     A message addressed to this callback is posted to the Async port
     upon completion.
     @param      refcon Arbitrary pointer which is passed as a parameter to the callback routine.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, or
     kIOReturnNotOpen if the interface is not open for exclusive access.
     */

    IOReturn (*WriteStreamsPipeAsyncTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);

    /*!
     @function AbortStreamsPipe
     @abstract   This method causes all outstanding I/O on a stream of a pipe to complete with return code kIOReturnAborted.
     @discussion If there are outstanding asynchronous transactions on the pipe, the callbacks will happen.
     Note that this command will also clear the halted bit on the endpoint
     in the controller, but will NOT clear the data toggle bit.  If you want to clear the data toggle bit as well, see @link //apple_ref/C/instm/IOUSBInterfaceInterface/ClearPipeStall/ ClearPipeStall @/link or
     @link //apple_ref/C/instm/IOUSBInterfaceInterface190/ClearPipeStallBothEnds/ ClearPipeStallBothEnds @/link for more information.  The interface must be open for the pipe to exist.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
     @param streamID ID of the stream to abort
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService,
     or kIOReturnNotOpen if the interface is not open for exclusive access.

     */

    IOReturn (*AbortStreamsPipe)(void *self, UInt8 pipeRef, UInt32 streamID);

} IOUSBInterfaceInterface550;





/*!
 @interface IOUSBInterfaceInterface650
 @abstract   The object you use to access a USB device interface from user space, returned by the IOUSBFamily
 version 6.5.0 and above.
 @discussion The functions listed here include all of the functions defined for IOUSBInterfaceInterface550, as well as some new functions that are available on
 Mac OS X version 10.9 and later.
 @super IOUSBInterfaceInterface550
 */
typedef struct IOUSBInterfaceStruct650 {
    IUNKNOWN_C_GUTS;
    IOReturn (*CreateInterfaceAsyncEventSource)(void *self, CFRunLoopSourceRef *source);
    CFRunLoopSourceRef (*GetInterfaceAsyncEventSource)(void *self);
    IOReturn (*CreateInterfaceAsyncPort)(void *self, mach_port_t *port);
    mach_port_t (*GetInterfaceAsyncPort)(void *self);
    IOReturn (*USBInterfaceOpen)(void *self);
    IOReturn (*USBInterfaceClose)(void *self);
    IOReturn (*GetInterfaceClass)(void *self, UInt8 *intfClass);
    IOReturn (*GetInterfaceSubClass)(void *self, UInt8 *intfSubClass);
    IOReturn (*GetInterfaceProtocol)(void *self, UInt8 *intfProtocol);
    IOReturn (*GetDeviceVendor)(void *self, UInt16 *devVendor);
    IOReturn (*GetDeviceProduct)(void *self, UInt16 *devProduct);
    IOReturn (*GetDeviceReleaseNumber)(void *self, UInt16 *devRelNum);
    IOReturn (*GetConfigurationValue)(void *self, UInt8 *configVal);
    IOReturn (*GetInterfaceNumber)(void *self, UInt8 *intfNumber);
    IOReturn (*GetAlternateSetting)(void *self, UInt8 *intfAltSetting);
    IOReturn (*GetNumEndpoints)(void *self, UInt8 *intfNumEndpoints);
    IOReturn (*GetLocationID)(void *self, UInt32 *locationID);
    IOReturn (*GetDevice)(void *self, io_service_t *device);
    IOReturn (*SetAlternateInterface)(void *self, UInt8 alternateSetting);
    IOReturn (*GetBusFrameNumber)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*ControlRequest)(void *self, UInt8 pipeRef, IOUSBDevRequest *req);
    IOReturn (*ControlRequestAsync)(void *self, UInt8 pipeRef, IOUSBDevRequest *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*GetPipeProperties)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*GetPipeStatus)(void *self, UInt8 pipeRef);
    IOReturn (*AbortPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ResetPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ClearPipeStall)(void *self, UInt8 pipeRef);
    IOReturn (*ReadPipe)(void *self, UInt8 pipeRef, void *buf, UInt32 *size);
    IOReturn (*WritePipe)(void *self, UInt8 pipeRef, void *buf, UInt32 size);
    IOReturn (*ReadPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                   IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                    IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ControlRequestTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req);
    IOReturn (*ControlRequestAsyncTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*ReadPipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 *size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*WritePipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*ReadPipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*USBInterfaceGetStringIndex)(void *self, UInt8 *si);
    IOReturn (*USBInterfaceOpenSeize)(void *self);
    IOReturn (*ClearPipeStallBothEnds)(void *self, UInt8 pipeRef);
    IOReturn (*SetPipePolicy)(void *self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (*GetBandwidthAvailable)(void *self, UInt32 *bandwidth);
    IOReturn (*GetEndpointProperties)(void *self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*LowLatencyReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                             IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyWriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                              IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyCreateBuffer)(void * self, void **buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (*LowLatencyDestroyBuffer) (void * self, void * buffer );
    IOReturn (*GetBusMicroFrameNumber)(void *self, UInt64 *microFrame, AbsoluteTime *atTime);
    IOReturn (*GetFrameListTime)(void *self, UInt32 *microsecondsInFrame);
    IOReturn (*GetIOUSBLibVersion)(void *self, NumVersion *ioUSBLibVersion, NumVersion *usbFamilyVersion);
    IOUSBDescriptorHeader * (*FindNextAssociatedDescriptor)(void *self, const void *currentDescriptor, UInt8 descriptorType);
    IOUSBDescriptorHeader * (*FindNextAltInterface)(void *self, const void *current, IOUSBFindInterfaceRequest *request);
    IOReturn (*GetBusFrameNumberWithTime)(void *self, UInt64 *frame, AbsoluteTime *atTime);
#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    IOReturn (* SetIdlePolicy)(void* self, UInt32 deviceIdleTimeout, UInt32 ioIdleTimeout);
    IOReturn (* OverrideIdlePolicy)(void* self, UInt8 pipeRef, bool override, UInt32 ioIdleTimeout);
    IONotificationPortRef (* GetInterfaceAsyncNotificationPort)(void* self);
#endif
#endif
    IOReturn (*GetPipePropertiesV2)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval, UInt8 *maxBurst, UInt8 *mult, UInt16 *bytesPerInterval);
    IOReturn (*GetPipePropertiesV3)(void *self, UInt8 pipeRef, IOUSBEndpointProperties *properties);
    IOReturn (*GetEndpointPropertiesV3)(void *self, IOUSBEndpointProperties *properties);
    IOReturn (*SupportsStreams)(void *self, UInt8 pipeRef, UInt32 *supportsStreams);
    IOReturn (*CreateStreams)(void *self, UInt8 pipeRef, UInt32 streamID);
    IOReturn (*GetConfiguredStreams)(void *self, UInt8 pipeRef, UInt32 *configuredStreams);
    IOReturn (*ReadStreamsPipeTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 *size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*WriteStreamsPipeTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*ReadStreamsPipeAsyncTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WriteStreamsPipeAsyncTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*AbortStreamsPipe)(void *self, UInt8 pipeRef, UInt32 streamID);

    /*!
     @function RegisterForNotification
     @abstract   Registers a callback routine to be invoked when certain events occur in the kernel.
     @discussion The callback function will be called, for example when the underlying IOUSBInterface is
     going to be suspended due to some kind of kernel activity. It will also be called when
     the underlying IOUSBInterface is resumed.
     @availability This function is only available with IOUSBInterfaceInterface650 and above.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      notificationMask Specifies the desired type of notification (e.g. suspend/resume)
     @param      callback An IOAsyncCallback2 method. Upon completion, the arg0 argument of the AsyncCallback2 will contain the
     notification type, and arg1 will contain a notificationToken which should be used when calling AcknowledgeNotification
     @param      refCon Arbitrary pointer which is passed as a parameter to the callback routine.
     @param      pRegistrationToken A pointer to a UInt64 which will contain a registration token if the function is
     successful. This registration token can then later be used to call UnregisterNotification.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */
    IOReturn (*RegisterForNotification)(void * self, UInt64 notificationMask, IOAsyncCallback2 callback, void *refCon, UInt64 *pRegistrationToken);



    /*!
     @function UnregisterNotification
     @abstract   Unregisters a previously registered callback routine
     @discussion The callback routine will no longer be invoked when events occur
     @availability This function is only available with IOUSBInterfaceInterface650 and above.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      registrationToken The registration token which was obtained in the call to RegisterForNotification
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */
    IOReturn (*UnregisterNotification)(void *self, UInt64 registrationToken);


    /*!
     @function AcknowledgeNotification
     @abstract   Acknowledges a notification event
     @discussion Some events in the kernel will wait for an acknowledgement from all interested parties before proceeding.
     For example, if an IOUSBInterface is about to be suspended, any User Code which has registered to receive
     that event will be notified and should acknowledge the notification when it is ready for the IOUSBInterface
     to be suspended.
     @availability This function is only available with IOUSBInterfaceInterface650 and above.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      notificationToken The notification token which was passed in to the callback routine
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService.
     */
    IOReturn (*AcknowledgeNotification)(void *self, UInt64 notificationToken);

} IOUSBInterfaceInterface650;

/*!
 @interface IOUSBInterfaceInterface700
 @abstract   The object you use to access a USB interface interface from user space, returned by the IOUSBFamily
 version 7.0.0 and above.
 @discussion The functions listed here include all of the functions defined for IOUSBInterfaceInterface650, as well as some new functions that are available on
 Mac OS X version 10.9 and later.
 @super IOUSBInterfaceInterface650
 */
typedef struct IOUSBInterfaceStruct700 {
    IUNKNOWN_C_GUTS;
    IOReturn (*CreateInterfaceAsyncEventSource)(void *self, CFRunLoopSourceRef *source);
    CFRunLoopSourceRef (*GetInterfaceAsyncEventSource)(void *self);
    IOReturn (*CreateInterfaceAsyncPort)(void *self, mach_port_t *port);
    mach_port_t (*GetInterfaceAsyncPort)(void *self);
    IOReturn (*USBInterfaceOpen)(void *self);
    IOReturn (*USBInterfaceClose)(void *self);
    IOReturn (*GetInterfaceClass)(void *self, UInt8 *intfClass);
    IOReturn (*GetInterfaceSubClass)(void *self, UInt8 *intfSubClass);
    IOReturn (*GetInterfaceProtocol)(void *self, UInt8 *intfProtocol);
    IOReturn (*GetDeviceVendor)(void *self, UInt16 *devVendor);
    IOReturn (*GetDeviceProduct)(void *self, UInt16 *devProduct);
    IOReturn (*GetDeviceReleaseNumber)(void *self, UInt16 *devRelNum);
    IOReturn (*GetConfigurationValue)(void *self, UInt8 *configVal);
    IOReturn (*GetInterfaceNumber)(void *self, UInt8 *intfNumber);
    IOReturn (*GetAlternateSetting)(void *self, UInt8 *intfAltSetting);
    IOReturn (*GetNumEndpoints)(void *self, UInt8 *intfNumEndpoints);
    IOReturn (*GetLocationID)(void *self, UInt32 *locationID);
    IOReturn (*GetDevice)(void *self, io_service_t *device);
    IOReturn (*SetAlternateInterface)(void *self, UInt8 alternateSetting);
    IOReturn (*GetBusFrameNumber)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*ControlRequest)(void *self, UInt8 pipeRef, IOUSBDevRequest *req);
    IOReturn (*ControlRequestAsync)(void *self, UInt8 pipeRef, IOUSBDevRequest *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*GetPipeProperties)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*GetPipeStatus)(void *self, UInt8 pipeRef);
    IOReturn (*AbortPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ResetPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ClearPipeStall)(void *self, UInt8 pipeRef);
    IOReturn (*ReadPipe)(void *self, UInt8 pipeRef, void *buf, UInt32 *size);
    IOReturn (*WritePipe)(void *self, UInt8 pipeRef, void *buf, UInt32 size);
    IOReturn (*ReadPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                   IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                    IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ControlRequestTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req);
    IOReturn (*ControlRequestAsyncTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*ReadPipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 *size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*WritePipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*ReadPipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*USBInterfaceGetStringIndex)(void *self, UInt8 *si);
    IOReturn (*USBInterfaceOpenSeize)(void *self);
    IOReturn (*ClearPipeStallBothEnds)(void *self, UInt8 pipeRef);
    IOReturn (*SetPipePolicy)(void *self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (*GetBandwidthAvailable)(void *self, UInt32 *bandwidth);
    IOReturn (*GetEndpointProperties)(void *self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*LowLatencyReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                             IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyWriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                              IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyCreateBuffer)(void * self, void **buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (*LowLatencyDestroyBuffer) (void * self, void * buffer );
    IOReturn (*GetBusMicroFrameNumber)(void *self, UInt64 *microFrame, AbsoluteTime *atTime);
    IOReturn (*GetFrameListTime)(void *self, UInt32 *microsecondsInFrame);
    IOReturn (*GetIOUSBLibVersion)(void *self, NumVersion *ioUSBLibVersion, NumVersion *usbFamilyVersion);
    IOUSBDescriptorHeader * (*FindNextAssociatedDescriptor)(void *self, const void *currentDescriptor, UInt8 descriptorType);
    IOUSBDescriptorHeader * (*FindNextAltInterface)(void *self, const void *current, IOUSBFindInterfaceRequest *request);
    IOReturn (*GetBusFrameNumberWithTime)(void *self, UInt64 *frame, AbsoluteTime *atTime);
#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    IOReturn (* SetIdlePolicy)(void* self, UInt32 deviceIdleTimeout, UInt32 ioIdleTimeout);
    IOReturn (* OverrideIdlePolicy)(void* self, UInt8 pipeRef, bool override, UInt32 ioIdleTimeout);
    IONotificationPortRef (* GetInterfaceAsyncNotificationPort)(void* self);
#endif
#endif
    IOReturn (*GetPipePropertiesV2)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval, UInt8 *maxBurst, UInt8 *mult, UInt16 *bytesPerInterval);
    IOReturn (*GetPipePropertiesV3)(void *self, UInt8 pipeRef, IOUSBEndpointProperties *properties);
    IOReturn (*GetEndpointPropertiesV3)(void *self, IOUSBEndpointProperties *properties);
    IOReturn (*SupportsStreams)(void *self, UInt8 pipeRef, UInt32 *supportsStreams);
    IOReturn (*CreateStreams)(void *self, UInt8 pipeRef, UInt32 streamID);
    IOReturn (*GetConfiguredStreams)(void *self, UInt8 pipeRef, UInt32 *configuredStreams);
    IOReturn (*ReadStreamsPipeTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 *size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*WriteStreamsPipeTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*ReadStreamsPipeAsyncTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WriteStreamsPipeAsyncTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*AbortStreamsPipe)(void *self, UInt8 pipeRef, UInt32 streamID);
    IOReturn (*RegisterForNotification)(void * self, UInt64 notificationMask, IOAsyncCallback2 callback, void *refCon, UInt64 *pRegistrationToken);
    IOReturn (*UnregisterNotification)(void *self, UInt64 registrationToken);
    IOReturn (*AcknowledgeNotification)(void *self, UInt64 notificationToken);

    /*!
     @function RegisterDriver
     @abstract   Triggers IOKit driver matching for this interface
     @discussion    This function will call IOKit's registerService on the IOUSBInterface, which will load any kernel drivers for the interface. The SetConfigurationV2 API can be told to not load
     drivers for ALL IOUSBInterfaces.  This call will then allow a client to load a driver for just one of the interfaces.  The client must have the "com.apple.vm.device-access" entitlement or run with root privileges to call this API.
     @availability This function is only available with IOUSBInterfaceInterface700 and above.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, kIOReturnNotPermitted if the client does not have the "com.apple.vm.device-access" entitlement or does not have root privileges.
     */
    IOReturn (*RegisterDriver)(void *self);

} IOUSBInterfaceInterface700;

/*!
 @interface IOUSBInterfaceInterface800
 @abstract   The object you use to access a USB interface interface from user space, returned by the IOUSBFamily
 version 8.0.0 and above.
 @discussion The functions listed here include all of the functions defined for IOUSBInterfaceInterface700, as well as some new functions that are available on
 Mac OS X version 10.11 and later.
 @super IOUSBInterfaceInterface700
 */

typedef struct IOUSBInterfaceStruct800 {
    IUNKNOWN_C_GUTS;
    IOReturn (*CreateInterfaceAsyncEventSource)(void *self, CFRunLoopSourceRef *source);
    CFRunLoopSourceRef (*GetInterfaceAsyncEventSource)(void *self);
    IOReturn (*CreateInterfaceAsyncPort)(void *self, mach_port_t *port);
    mach_port_t (*GetInterfaceAsyncPort)(void *self);
    IOReturn (*USBInterfaceOpen)(void *self);
    IOReturn (*USBInterfaceClose)(void *self);
    IOReturn (*GetInterfaceClass)(void *self, UInt8 *intfClass);
    IOReturn (*GetInterfaceSubClass)(void *self, UInt8 *intfSubClass);
    IOReturn (*GetInterfaceProtocol)(void *self, UInt8 *intfProtocol);
    IOReturn (*GetDeviceVendor)(void *self, UInt16 *devVendor);
    IOReturn (*GetDeviceProduct)(void *self, UInt16 *devProduct);
    IOReturn (*GetDeviceReleaseNumber)(void *self, UInt16 *devRelNum);
    IOReturn (*GetConfigurationValue)(void *self, UInt8 *configVal);
    IOReturn (*GetInterfaceNumber)(void *self, UInt8 *intfNumber);
    IOReturn (*GetAlternateSetting)(void *self, UInt8 *intfAltSetting);
    IOReturn (*GetNumEndpoints)(void *self, UInt8 *intfNumEndpoints);
    IOReturn (*GetLocationID)(void *self, UInt32 *locationID);
    IOReturn (*GetDevice)(void *self, io_service_t *device);
    IOReturn (*SetAlternateInterface)(void *self, UInt8 alternateSetting);
    IOReturn (*GetBusFrameNumber)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*ControlRequest)(void *self, UInt8 pipeRef, IOUSBDevRequest *req);
    IOReturn (*ControlRequestAsync)(void *self, UInt8 pipeRef, IOUSBDevRequest *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*GetPipeProperties)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*GetPipeStatus)(void *self, UInt8 pipeRef);
    IOReturn (*AbortPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ResetPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ClearPipeStall)(void *self, UInt8 pipeRef);
    IOReturn (*ReadPipe)(void *self, UInt8 pipeRef, void *buf, UInt32 *size);
    IOReturn (*WritePipe)(void *self, UInt8 pipeRef, void *buf, UInt32 size);
    IOReturn (*ReadPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                   IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                    IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ControlRequestTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req);
    IOReturn (*ControlRequestAsyncTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*ReadPipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 *size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*WritePipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*ReadPipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*USBInterfaceGetStringIndex)(void *self, UInt8 *si);
    IOReturn (*USBInterfaceOpenSeize)(void *self);
    IOReturn (*ClearPipeStallBothEnds)(void *self, UInt8 pipeRef);
    IOReturn (*SetPipePolicy)(void *self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (*GetBandwidthAvailable)(void *self, UInt32 *bandwidth);
    IOReturn (*GetEndpointProperties)(void *self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*LowLatencyReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                             IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyWriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                              IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyCreateBuffer)(void * self, void **buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (*LowLatencyDestroyBuffer) (void * self, void * buffer );
    IOReturn (*GetBusMicroFrameNumber)(void *self, UInt64 *microFrame, AbsoluteTime *atTime);
    IOReturn (*GetFrameListTime)(void *self, UInt32 *microsecondsInFrame);
    IOReturn (*GetIOUSBLibVersion)(void *self, NumVersion *ioUSBLibVersion, NumVersion *usbFamilyVersion);
    IOUSBDescriptorHeader * (*FindNextAssociatedDescriptor)(void *self, const void *currentDescriptor, UInt8 descriptorType);
    IOUSBDescriptorHeader * (*FindNextAltInterface)(void *self, const void *current, IOUSBFindInterfaceRequest *request);
    IOReturn (*GetBusFrameNumberWithTime)(void *self, UInt64 *frame, AbsoluteTime *atTime);
#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    IOReturn (* SetIdlePolicy)(void* self, UInt32 deviceIdleTimeout, UInt32 ioIdleTimeout);
    IOReturn (* OverrideIdlePolicy)(void* self, UInt8 pipeRef, bool override, UInt32 ioIdleTimeout);
    IONotificationPortRef (* GetInterfaceAsyncNotificationPort)(void* self);
#endif
#endif
    IOReturn (*GetPipePropertiesV2)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval, UInt8 *maxBurst, UInt8 *mult, UInt16 *bytesPerInterval);
    IOReturn (*GetPipePropertiesV3)(void *self, UInt8 pipeRef, IOUSBEndpointProperties *properties);
    IOReturn (*GetEndpointPropertiesV3)(void *self, IOUSBEndpointProperties *properties);
    IOReturn (*SupportsStreams)(void *self, UInt8 pipeRef, UInt32 *supportsStreams);
    IOReturn (*CreateStreams)(void *self, UInt8 pipeRef, UInt32 streamID);
    IOReturn (*GetConfiguredStreams)(void *self, UInt8 pipeRef, UInt32 *configuredStreams);
    IOReturn (*ReadStreamsPipeTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 *size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*WriteStreamsPipeTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*ReadStreamsPipeAsyncTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WriteStreamsPipeAsyncTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*AbortStreamsPipe)(void *self, UInt8 pipeRef, UInt32 streamID);
    IOReturn (*RegisterForNotification)(void * self, UInt64 notificationMask, IOAsyncCallback2 callback, void *refCon, UInt64 *pRegistrationToken);
    IOReturn (*UnregisterNotification)(void *self, UInt64 registrationToken);
    IOReturn (*AcknowledgeNotification)(void *self, UInt64 notificationToken);
    IOReturn (*RegisterDriver)(void *self);

    /*!
     @function SetDeviceIdlePolicy
     @abstract   Define an idling policy for the interface.
     @discussion This method is called to enforce an an idle policy for the device.  Note, the idle policy is only active while the interface is open and is reset when closed.
     @availability This function is only available with IOUSBInterfaceInterface800 and above.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      deviceIdleTimeout The amount of time in ms a device is idle before it can be suspended.  Use 0 to prevent suspending an idle device.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, kIOReturnUnsupported is the bus doesn't support this function.
     */
    IOReturn (*SetDeviceIdlePolicy)(void* self, UInt32 deviceIdleTimeout);

    /*!
     @function SetPipeIdlePolicy
     @abstract   Define an idling policy for the interface.
     @discussion This method is called to enforce an an idle policy for the pipes. Note, the idle policy is only active while the interface is open and is reset when closed.
     @availability This function is only available with IOUSBInterfaceInterface800 and above.
     @param      self Pointer to the IOUSBInterfaceInterface.
     @param      pipeRef Index for the desired pipe (1 - GetNumEndpoints).
     @param      ioIdleTimeout The amount of time in ms an IO is pending before it allows the device to be considered idle.  Use 0 to prevent idling the device while the IO is in progress, regardless of how long it takes.
     @result     Returns kIOReturnSuccess if successful, kIOReturnNoDevice if there is no connection to an IOService, kIOReturnUnsupported is the bus doesn't support this function.
     */
    IOReturn (*SetPipeIdlePolicy)(void* self, UInt8 pipeRef, UInt32 ioIdleTimeout);

} IOUSBInterfaceInterface800;

/*!
 @interface IOUSBInterfaceInterface942
 @abstract   The object you use to access a USB interface interface from user space, returned by the IOUSBFamily
 version 900.4.2 and above.
 @discussion The functions listed here include all of the functions defined for IOUSBInterfaceInterface800, as well as some new functions that are available on
 macOS 10.14 and later.
 @super IOUSBInterfaceInterface800
 */

typedef struct IOUSBInterfaceStruct942 {
    IUNKNOWN_C_GUTS;
    IOReturn (*CreateInterfaceAsyncEventSource)(void *self, CFRunLoopSourceRef *source);
    CFRunLoopSourceRef (*GetInterfaceAsyncEventSource)(void *self);
    IOReturn (*CreateInterfaceAsyncPort)(void *self, mach_port_t *port);
    mach_port_t (*GetInterfaceAsyncPort)(void *self);
    IOReturn (*USBInterfaceOpen)(void *self);
    IOReturn (*USBInterfaceClose)(void *self);
    IOReturn (*GetInterfaceClass)(void *self, UInt8 *intfClass);
    IOReturn (*GetInterfaceSubClass)(void *self, UInt8 *intfSubClass);
    IOReturn (*GetInterfaceProtocol)(void *self, UInt8 *intfProtocol);
    IOReturn (*GetDeviceVendor)(void *self, UInt16 *devVendor);
    IOReturn (*GetDeviceProduct)(void *self, UInt16 *devProduct);
    IOReturn (*GetDeviceReleaseNumber)(void *self, UInt16 *devRelNum);
    IOReturn (*GetConfigurationValue)(void *self, UInt8 *configVal);
    IOReturn (*GetInterfaceNumber)(void *self, UInt8 *intfNumber);
    IOReturn (*GetAlternateSetting)(void *self, UInt8 *intfAltSetting);
    IOReturn (*GetNumEndpoints)(void *self, UInt8 *intfNumEndpoints);
    IOReturn (*GetLocationID)(void *self, UInt32 *locationID);
    IOReturn (*GetDevice)(void *self, io_service_t *device);
    IOReturn (*SetAlternateInterface)(void *self, UInt8 alternateSetting);
    IOReturn (*GetBusFrameNumber)(void *self, UInt64 *frame, AbsoluteTime *atTime);
    IOReturn (*ControlRequest)(void *self, UInt8 pipeRef, IOUSBDevRequest *req);
    IOReturn (*ControlRequestAsync)(void *self, UInt8 pipeRef, IOUSBDevRequest *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*GetPipeProperties)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*GetPipeStatus)(void *self, UInt8 pipeRef);
    IOReturn (*AbortPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ResetPipe)(void *self, UInt8 pipeRef);
    IOReturn (*ClearPipeStall)(void *self, UInt8 pipeRef);
    IOReturn (*ReadPipe)(void *self, UInt8 pipeRef, void *buf, UInt32 *size);
    IOReturn (*WritePipe)(void *self, UInt8 pipeRef, void *buf, UInt32 size);
    IOReturn (*ReadPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt32 size, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                   IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, IOUSBIsocFrame *frameList,
                                    IOAsyncCallback1 callback, void *refcon);
    IOReturn (*ControlRequestTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req);
    IOReturn (*ControlRequestAsyncTO)(void *self, UInt8 pipeRef, IOUSBDevRequestTO *req, IOAsyncCallback1 callback, void *refCon);
    IOReturn (*ReadPipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 *size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*WritePipeTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*ReadPipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WritePipeAsyncTO)(void *self, UInt8 pipeRef, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*USBInterfaceGetStringIndex)(void *self, UInt8 *si);
    IOReturn (*USBInterfaceOpenSeize)(void *self);
    IOReturn (*ClearPipeStallBothEnds)(void *self, UInt8 pipeRef);
    IOReturn (*SetPipePolicy)(void *self, UInt8 pipeRef, UInt16 maxPacketSize, UInt8 maxInterval);
    IOReturn (*GetBandwidthAvailable)(void *self, UInt32 *bandwidth);
    IOReturn (*GetEndpointProperties)(void *self, UInt8 alternateSetting, UInt8 endpointNumber, UInt8 direction, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval);
    IOReturn (*LowLatencyReadIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                             IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyWriteIsochPipeAsync)(void *self, UInt8 pipeRef, void *buf, UInt64 frameStart, UInt32 numFrames, UInt32 updateFrequency, IOUSBLowLatencyIsocFrame *frameList,
                                              IOAsyncCallback1 callback, void *refcon);
    IOReturn (*LowLatencyCreateBuffer)(void * self, void **buffer, IOByteCount size, UInt32 bufferType);
    IOReturn (*LowLatencyDestroyBuffer) (void * self, void * buffer );
    IOReturn (*GetBusMicroFrameNumber)(void *self, UInt64 *microFrame, AbsoluteTime *atTime);
    IOReturn (*GetFrameListTime)(void *self, UInt32 *microsecondsInFrame);
    IOReturn (*GetIOUSBLibVersion)(void *self, NumVersion *ioUSBLibVersion, NumVersion *usbFamilyVersion);
    IOUSBDescriptorHeader * (*FindNextAssociatedDescriptor)(void *self, const void *currentDescriptor, UInt8 descriptorType);
    IOUSBDescriptorHeader * (*FindNextAltInterface)(void *self, const void *current, IOUSBFindInterfaceRequest *request);
    IOReturn (*GetBusFrameNumberWithTime)(void *self, UInt64 *frame, AbsoluteTime *atTime);
#ifndef __OPEN_SOURCE__
#if TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
    IOReturn (* SetIdlePolicy)(void* self, UInt32 deviceIdleTimeout, UInt32 ioIdleTimeout);
    IOReturn (* OverrideIdlePolicy)(void* self, UInt8 pipeRef, bool override, UInt32 ioIdleTimeout);
    IONotificationPortRef (* GetInterfaceAsyncNotificationPort)(void* self);
#endif
#endif
    IOReturn (*GetPipePropertiesV2)(void *self, UInt8 pipeRef, UInt8 *direction, UInt8 *number, UInt8 *transferType, UInt16 *maxPacketSize, UInt8 *interval, UInt8 *maxBurst, UInt8 *mult, UInt16 *bytesPerInterval);
    IOReturn (*GetPipePropertiesV3)(void *self, UInt8 pipeRef, IOUSBEndpointProperties *properties);
    IOReturn (*GetEndpointPropertiesV3)(void *self, IOUSBEndpointProperties *properties);
    IOReturn (*SupportsStreams)(void *self, UInt8 pipeRef, UInt32 *supportsStreams);
    IOReturn (*CreateStreams)(void *self, UInt8 pipeRef, UInt32 streamID);
    IOReturn (*GetConfiguredStreams)(void *self, UInt8 pipeRef, UInt32 *configuredStreams);
    IOReturn (*ReadStreamsPipeTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 *size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*WriteStreamsPipeTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout);
    IOReturn (*ReadStreamsPipeAsyncTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*WriteStreamsPipeAsyncTO)(void *self, UInt8 pipeRef, UInt32 streamID, void *buf, UInt32 size, UInt32 noDataTimeout, UInt32 completionTimeout, IOAsyncCallback1 callback, void *refcon);
    IOReturn (*AbortStreamsPipe)(void *self, UInt8 pipeRef, UInt32 streamID);
    IOReturn (*RegisterForNotification)(void * self, UInt64 notificationMask, IOAsyncCallback2 callback, void *refCon, UInt64 *pRegistrationToken);
    IOReturn (*UnregisterNotification)(void *self, UInt64 registrationToken);
    IOReturn (*AcknowledgeNotification)(void *self, UInt64 notificationToken);
    IOReturn (*RegisterDriver)(void *self);
    IOReturn (*SetDeviceIdlePolicy)(void* self, UInt32 deviceIdleTimeout);
    IOReturn (*SetPipeIdlePolicy)(void* self, UInt8 pipeRef, UInt32 ioIdleTimeout);

#ifndef __OPEN_SOURCE__
#if !TARGET_OS_IPHONE || TARGET_OS_MACCATALYST
    /*!
     @function     GetInterfaceAsyncNotificationPort
     @abstract     Returns the IONotificationPort for this IOService instance.
     @availability This function is only available with IOUSBInterfaceInterface942 and above.
     @param        self Pointer to the IOUSBInterfaceInterface.
     @result       Returns the IONotificationPortRef if one exists, MACH_PORT_NULL otherwise.
     */
    IONotificationPortRef (* GetInterfaceAsyncNotificationPort)(void* self);
#endif
#endif

} IOUSBInterfaceInterface942;

typedef IOUSBInterfaceInterface942 IOUSBInterfaceInterface;

// The undecorated interface ID is the oldest version
#define kIOUSBInterfaceInterfaceID kIOUSBInterfaceInterfaceID100

#if !__OPEN_SOURCE__ && TARGET_OS_IPHONE && !TARGET_OS_MACCATALYST
#define kIOUSBDeviceClassName           kIOUSBHostDeviceClassName
#define kIOUSBInterfaceClassName        kIOUSBHostInterfaceClassName
#else
#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_14
#define kIOUSBDeviceClassName           "IOUSBDevice"
#define kIOUSBInterfaceClassName        "IOUSBInterface"
#else
#define kIOUSBDeviceClassName           kIOUSBHostDeviceClassName
#define kIOUSBInterfaceClassName        kIOUSBHostInterfaceClassName
#endif /* MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_14 */
#endif
__END_DECLS

#endif /* ! _IOUSBLIB_H */
