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

#ifndef _IOETHERNETINTERFACE_H
#define _IOETHERNETINTERFACE_H

/*! @defined kIOEthernetInterfaceClass
    @abstract The name of the
        IOEthernetInterface class. 
*/

#define kIOEthernetInterfaceClass     "IOEthernetInterface"

/*! @defined kIOActivePacketFilters
    @abstract A property of IOEthernetInterface objects.
    @discussion The kIOActivePacketFilters property has an OSDictionary value that describes the current
        set of packet filters that have been successfully activated. Each
        entry in the dictionary is a key/value pair consisting of the filter
        group name, and an OSNumber describing the set of active filters for
        that group. Entries in this dictionary will mirror those in
        kIORequiredPacketFilters if the controller has reported success for
        all filter change requests from the IOEthernetInterface object. 
*/

#define kIOActivePacketFilters        "IOActivePacketFilters"

/*! @defined kIORequiredPacketFilters
    @abstract A property of IOEthernetInterface objects.
    @discussion The kIORequiredPacketFilters property has an OSDictionary value that describes the current
        set of required packet filters. Each entry in the dictionary is a
        key/value pair consisting of the filter group name, and an OSNumber
        describing the set of required filters for that group. 
*/

#define kIORequiredPacketFilters      "IORequiredPacketFilters"

/*! @defined kIOMulticastAddressList
    @abstract A property of IOEthernetInterface objects.
    @discussion The kIOMulticastAddressList property is an OSData object that describes the
        list of multicast addresses that are being used by the
        controller to match against the destination address of an
        incoming frame. 
*/

#define kIOMulticastAddressList       "IOMulticastAddressList"
#define kIOMulticastFilterData        kIOMulticastAddressList    

#endif /* !_IOETHERNETINTERFACE_H */
