/*
 * Copyright (c) 1998-2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _IONETWORKINTERFACE_H
#define _IONETWORKINTERFACE_H

/*! @defined kIONetworkInterfaceClass
    @abstract The name of the IONetworkInterface class. 
*/

#define kIONetworkInterfaceClass  "IONetworkInterface"

/*! @defined kIONetworkData
    @abstract A property of IONetworkInterface objects. 
    @discussion The kIONetworkData property has an OSDictionary value and is a
        container for the set of IONetworkData objects managed by the interface.
        Each entry in the dictionary is a key/value pair consisting of
        the network data name, and an OSDictionary describing the
        contents of the network data. 
*/

#define kIONetworkData            "IONetworkData"

/*! @defined kIOInterfaceType
    @abstract A property of IONetworkInterface objects.
    @discussion The kIOInterfaceType property has an OSNumber value that
        specifies the type of network interface that this interface represents.
        The type constants are defined in bsd/net/if_types.h. 
*/

#define kIOInterfaceType          "IOInterfaceType"

/*! @defined kIOMaxTransferUnit
    @abstract A property of IONetworkInterface objects.
    @discussion The kIOMaxTransferUnit property has an OSNumber value that
        specifies the maximum transfer unit for the interface in bytes.
*/

#define kIOMaxTransferUnit        "IOMaxTransferUnit"

/*! @defined kIOMediaAddressLength
    @abstract A property of IONetworkInterface objects.
    @discussion The kIOMediaAddressLength property has an OSNumber value that
        specifies the size of the media address in bytes. 
*/

#define kIOMediaAddressLength     "IOMediaAddressLength"

/*! @defined kIOMediaHeaderLength
    @abstract A property of IONetworkInterface objects.
    @discussion The kIOMediaHeaderLength property has an OSNumber value that
        specifies the size of the media header in bytes. 
*/

#define kIOMediaHeaderLength      "IOMediaHeaderLength"

/*! @defined kIOInterfaceFlags
    @abstract A property of IONetworkInterface objects.
    @discussion The kIOInterfaceFlags property has an OSNumber value that
        specifies the current value of the interface flags. The flag constants
        are defined in bsd/net/if.h. 
*/

#define kIOInterfaceFlags         "IOInterfaceFlags"

/*! @defined kIOInterfaceExtraFlags
    @abstract A property of IONetworkInterface objects.
    @discussion The kIOInterfaceExtraFlags property has an OSNumber value that
        specifies the current value of the interface eflags. The eflag constants
        are defined in bsd/net/if.h. 
*/

#define kIOInterfaceExtraFlags    "IOInterfaceExtraFlags"

/*! @defined kIOInterfaceUnit
    @abstract A property of IONetworkInterface objects.
    @discussion The kIOInterfaceUnit property has an OSNumber value that
        describes the unit number assigned to the interface object. 
*/

#define kIOInterfaceUnit          "IOInterfaceUnit"

/*! @defined kIOInterfaceState
    @abstract A property of IONetworkInterface objects.
    @discussion The kIOInterfaceState property has an OSNumber value that
        describes the current state of the interface object. This property is
        not exported to BSD via the ifnet structure. 
*/

#define kIOInterfaceState         "IOInterfaceState"

/*! @defined kIOInterfaceNamePrefix
    @abstract A property of IONetworkInterface objects.
    @discussion The kIOInterfaceNamePrefix property has an OSString value that
        describes the string prefix for the BSD name assigned to the interface. 
*/

#define kIOInterfaceNamePrefix    "IOInterfaceNamePrefix"

/*! @defined kIOPrimaryInterface
    @abstract A property of IONetworkInterface objects.
    @discussion The kIOInterfaceNamePrefix property has an OSBoolean value that
        describes whether the interface is the primary or the built-in network
        interface. 
*/

#define kIOPrimaryInterface       "IOPrimaryInterface"

/*! @defined kIOBuiltin
    @abstract kIOBuiltin is a property of IONetworkInterface
        objects. It has an OSBoolean value.
    @discussion The kIOBuiltin property describes whether the
        interface is built-in. 
*/

#define kIOBuiltin                "IOBuiltin"

/*! @defined kIOLocation
    @abstract kIOLocation is a property of IONetworkInterface
        objects. It has an OSString value.
    @discussion The kIOLocation property describes the physical 
        location of built-in interfaces. 
*/

#define kIOLocation               "IOLocation"

/*! @defined kIONetworkNoBSDAttachKey
    @abstract kIONetworkNoBSDAttachKey is a property of IONetworkInterface
        objects. It has an OSBoolean value.
    @discussion Adding a property with this key and the value kOSBooleanTrue
        before the interface is published will hold off the BSD attach.
        When the interface is ready to attach to BSD, remove the property
        and then re-publish the interface by calling registerService().
*/

#define kIONetworkNoBSDAttachKey  "IONetworkNoBSDAttach"

/*! @enum InterfaceObjectStates
    @discussion Constants used to encode the state of the interface object.
   @constant kIONetworkInterfaceRegisteredState The interface object has
        registered with the data link layer.
    @constant kIONetworkInterfaceOpenedState One or more clients have an
        open on the interface object.
    @constant kIONetworkInterfaceDisabledState The interface is temporarily
        unable to service its clients. This will occur when the network
        controller that is servicing the interface has entered a low power
        state that renders it unusable. 
*/

enum {
    kIONetworkInterfaceRegisteredState  = 0x1,
    kIONetworkInterfaceOpenedState      = 0x2,
    kIONetworkInterfaceDisabledState    = 0x4
};

#endif /* !_IONETWORKINTERFACE_H */
