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

#ifndef _IONETWORKCONTROLLER_H
#define _IONETWORKCONTROLLER_H

/*! @defined kIONetworkControllerClass
    @abstract The name of the IONetworkController class. */

#define kIONetworkControllerClass   "IONetworkController"

/*! @defined kIOVendor
    @abstract A property of IONetworkController objects.
    @discussion The kIOVendor property is a property of IONetworkController objects.  It has an OSString value 	that describes the vendor of the network controller. */

#define kIOVendor                "IOVendor"

/*! @defined kIOModel
    @abstract A property of IONetworkController objects.
    @discussion The kIOModel property is a property of IONetworkController objects.  It has an OSString value that 	describes the model of the network controller. */

#define kIOModel                 "IOModel"

/*! @defined kIORevision
    @abstract A property of IONetworkController objects.
    @discussion The kIORevision property is a property of IONetworkController objects.  It has an OSString value 	that describes the revision level of the network controller. */

#define kIORevision              "IORevision"

/*! @defined kIOFeatures
    @abstract A property of IONetworkController objects.
    @discussion The kIOFeatures property is a property of IONetworkController objects. It has an OSNumber value 	that describes generic features defined by IONetworkController that are supported by the
        network controller. */

#define kIOFeatures              "IOFeatures"

/*! @defined kIOMediumDictionary
    @abstract A property of IONetworkController objects.
    @discussion The kIOMediumDictionary property is a property of IONetworkController
        objects.  It has an OSDictionary value that is a container for the
        collection of IONetworkMedium objects that represent the media
        types supported by the network controller.
        Each entry in the dictionary is a key/value pair consisting of
        the medium name, and a dictionary value that contains the
        properties for that medium entry. */

#define kIOMediumDictionary      "IOMediumDictionary"

/*! @defined kIODefaultMedium
    @abstract A property of IONetworkController objects.
    @discussion The kIODefaultMedium property is a property of IONetworkController
        objects.  It has an OSString value that describes the name of the
        default medium. This definition may change or disappear in the
        future. */

#define kIODefaultMedium         "IODefaultMedium"

/*! @defined kIOSelectedMedium
    @abstract A property of IONetworkController objects.
    @discussion The kIOSelectedMedium property is a property of IONetworkController
        objects.  It has an OSSymbol value that describes the name of the
        current selected medium. This name can be used as a key into the
        medium dictionary to gather additional information about the
        selected medium. */

#define kIOSelectedMedium         "IOSelectedMedium"

/*! @defined kIOActiveMedium
    @abstract A property of IONetworkController objects.
    @discussion The kIOActiveMedium property is a property of IONetworkController
        objects.  It has an OSSymbol value that describes the name of the
        active medium. This is the name of the medium where an active
        link has been established. This name can be used as a key into
        the medium dictionary to gather additional information about the
        active medium. */

#define kIOActiveMedium          "IOActiveMedium"

/*! @defined kIOLinkSpeed
    @abstract A property of IONetworkController objects.
    @discussion The kIOLinkSpeed property is a property of IONetworkController
        objects. It has an OSNumber value that describes the speed of the
    	link established over the active medium in bits per second. */

#define kIOLinkSpeed             "IOLinkSpeed"

/*! @defined kIOLinkStatus
    @abstract A property of IONetworkController objects.
    @discussion The kIOLinkStatus property is a property of IONetworkController
        objects. It has an OSNumber value that describes the current network
        link status. See IONetworkMedium for the definition of the link
        status bits. */

#define kIOLinkStatus            "IOLinkStatus"

/*! @defined kIOLinkData
    @abstract A property of IONetworkController objects.
    @discussion The kIOLinkData property is a property of IONetworkController
        objects. It has an OSData value that contains additional information
        describing the active link that was established.
        Its interpretation is not defined. */

#define kIOLinkData              "IOLinkData"

/*! @defined kIOPacketFilters
    @abstract A property of IONetworkController objects.
    @discussion The kIOPacketFilters property is a property of IONetworkController
        objects. It has an OSDictionary value that describes the entire
        set of packet filters supported by the controller. Each entry
        in the dictionary is a key/value pair consisting of the filter
        group name, and an OSNumber describing the set of supported
        filters for that group. */

#define kIOPacketFilters         "IOPacketFilters"

/*! @defined kIOMACAddress
    @abstract A property of IONetworkController objects.
    @discussion The kIOMACAddress property is a property of IONetworkController
        objects. It has an OSData value that describes the hardware
        MAC (media access controller) address, or station address,
        of the network controller. */

#define kIOMACAddress            "IOMACAddress"

/*! @defined kIOMaxPacketSize
    @abstract A property of IONetworkController objects.
    @discussion The kIOMaxPacketSize property is a property of IONetworkController
        objects. It has an OSNumber value that describes the maximum
        packet size supported by the controller. */

#define kIOMaxPacketSize         "IOMaxPacketSize"

/*! @defined kIOMinPacketSize
    @abstract A property of IONetworkController objects.
    @discussion The kIOMinPacketSize property is a property of IONetworkController
        objects. It has an OSNumber value that describes the minimum
        packet size supported by the controller. */

#define kIOMinPacketSize         "IOMinPacketSize"

/*! @defined kIONetworkFilterGroup
    @abstract The name assigned to the standard network filter group. */

#define kIONetworkFilterGroup    "IONetworkFilterGroup"

/*! @enum StandardPacketFilters
    @abstract All standard packet filters. 
    @discussion Each filter will allow the reception of certain class of packets
        depending on its destination MAC address.
    @constant kIOPacketFilterUnicast Reception of unicast packets.
    @constant kIOPacketFilterBroadcast Reception of broadcast packets.
    @constant kIOPacketFilterMulticast Reception of multicast packets
        addressed to a set of multicast addresses.
    @constant kIOPacketFilterMulticastAll Reception of all multicast
        packets.
    @constant kIOPacketFilterPromiscuous Reception of all packets.
    @constant kIOPacketFilterPromiscuousAll Reception of all packets,
        including bad packets. */

enum {
    kIOPacketFilterUnicast         = 0x1,
    kIOPacketFilterBroadcast       = 0x2,
    kIOPacketFilterMulticast       = 0x10,
    kIOPacketFilterMulticastAll    = 0x20,
    kIOPacketFilterPromiscuous     = 0x100,
    kIOPacketFilterPromiscuousAll  = 0x200
};

/*! @enum Network Feature Flags
    @abstract Feature flags returned by the getFeatures() method.
    @constant kIONetworkFeatureNoBSDWait Set this bit in the value
        returned by getFeatures() to disable the automatic wait for
        "IOBSD" resource by the IONetworkController::start() method. 
    @constant kIONetworkFeatureHardwareVlan Set this bit in the value
        returned by getFeatures() to indicate the controller supports hardware
        stripping and stuffing of 802.1q vlan tags.  If the controller supports
        this feature it must enable it when initializing so that all received
        packets delivered to higher layers have the tag stripped.  The controller
        should use setVlanTag() to provide the tag information out of band.
    @constant kIONetworkFeatureSoftwareVlan Set this bit in the value
        returned by getFeatures() to indicate that the controller can support software
        based vlan by transmitting and receiving packets 4 bytes longer that normal. 
    @constant kIONetworkFeatureMultiPages Set this bit if the driver is
	capable of handling packets coming down from the network stack that
	reside in virtually, but not in physically contiguous span of the
	external mbuf clusters.  In this case, the data area of a packet in
	the external mbuf cluster might cross one or more physical pages that
	are disjoint, depending on the interface MTU and the packet size.
	Such a use of larger than system page size clusters by the network
	stack is done for better system efficiency.  Drivers that utilize the
	IOMbufNaturalMemoryCursor with the getPhysicalSegmentsWithCoalesce
	interfaces and enumerate the list of vectors should set this flag
	for possible gain in performance during bulk data transfer.
    @constant kIONetworkFeatureTSOIPv4 Set this bit to advertise support
        for TCP/IPv4 segmentation offload.
    @constant kIONetworkFeatureTSOIPv6 Set this bit to advertise support
        for TCP/IPv6 segmentation offload.
    @constant kIONetworkFeatureTransmitCompletionStatus Set this bit to
        advertise the capability to report per-packet transmit completion status.
        See <code>IONetworkInterface::reportTransmitCompletionStatus</code>.
    @constant kIONetworkFeatureHWTimeStamp Set this bit to advertise
         the capability to compute per-packet timestamp in hardware.
    @constant kIONetworkFeatureHWTimeStamp Set this bit to advertise
        the capability to compute per-packet timestamp in software.
*/

enum {
    kIONetworkFeatureNoBSDWait                  = 0x001,
    kIONetworkFeatureHardwareVlan               = 0x002,
    kIONetworkFeatureSoftwareVlan               = 0x004,
    kIONetworkFeatureMultiPages                 = 0x008,
    kIONetworkFeatureTSOIPv4                    = 0x010,
    kIONetworkFeatureTSOIPv6                    = 0x020,
    kIONetworkFeatureTransmitCompletionStatus   = 0x040,
    kIONetworkFeatureHWTimeStamp                = 0x080,
    kIONetworkFeatureSWTimeStamp                = 0x100
};

#endif /* !_IONETWORKCONTROLLER_H */
