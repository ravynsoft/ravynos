/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
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
/*
 * HISTORY
 *
 */

#ifndef _IONETWORKLIB_H
#define _IONETWORKLIB_H

#include <IOKit/IOKitLib.h>
#include <IOKit/network/IONetworkData.h>
#include <IOKit/network/IONetworkMedium.h>
#include <IOKit/network/IONetworkStats.h>
#include <IOKit/network/IOEthernetStats.h>
#include <IOKit/network/IONetworkUserClient.h>

typedef UInt32 IONDHandle;

#ifdef __cplusplus
extern "C" {
#endif

/*! @function IONetworkOpen
    @abstract Open a connection to an IONetworkInterface object.
    An IONetworkUserClient object is created to manage the connection. */

    IOReturn IONetworkOpen(io_object_t obj, io_connect_t * con);

/*! @function IONetworkClose
    @abstract Close the connection to an IONetworkInterface object. */

    IOReturn IONetworkClose(io_connect_t con);

/*! @function IONetworkWriteData
    @abstract Write to the buffer of a network data object.
    @param conObj The connection object.
    @param dataHandle The handle of a network data object.
    @param srcBuf The data to write is taken from this buffer.
    @param inSize The size of the source buffer.
    @result kIOReturnSuccess on success, or an error code otherwise. */

    IOReturn IONetworkWriteData(io_connect_t conObj,
                                IONDHandle   dataHandle,
                                UInt8 *      srcBuf,
                                UInt32       inSize);

/*! @function IONetworkReadData
    @abstract Read the buffer of a network data object.
    @param conObj The connection object.
    @param dataHandle The handle of a network data object.
    @param destBuf The buffer where the data read shall be written to.
    @param inOutSizeP Pointer to an integer that the caller must initialize
           to contain the size of the buffer. This function will overwrite
           it with the actual number of bytes written to the buffer.
    @result kIOReturnSuccess on success, or an error code otherwise. */

    IOReturn IONetworkReadData(io_connect_t conObj,
                                IONDHandle   dataHandle,
                                UInt8 *      destBuf,
                                UInt32 *     inOutSizeP);

/*! @function IONetworkResetData
    @abstract Fill the buffer of a network data object with zeroes.
    @param conObject The connection object.
    @param dataHandle The handle of a network data object.
    @result kIOReturnSuccess on success, or an error code otherwise. */

    IOReturn IONetworkResetData(io_connect_t conObject, IONDHandle dataHandle);

/*! @function IONetworkGetDataCapacity
    @abstract Get the capacity (in bytes) of a network data object.
    @param conObject The connection object.
    @param dataHandle The handle of a network data object.
    @param capacityP Upon success, the capacity is written to this address.
    @result kIOReturnSuccess on success, or an error code otherwise. */

    IOReturn IONetworkGetDataCapacity(io_connect_t conObject,
                                      IONDHandle   dataHandle,
                                      UInt32 *     capacityP);

/*! @function IONetworkGetDataHandle
    @abstract Get the handle of a network data object with the given name.
    @param conObject The connection object.
    @param dataName The name of the network data object.
    @param dataHandleP Upon success, the handle is written to this address.
    @result kIOReturnSuccess on success, or an error code otherwise. */

    IOReturn IONetworkGetDataHandle(io_connect_t conObject,
                                    const char * dataName,
                                    IONDHandle * dataHandleP);

/*! @function IONetworkSetPacketFiltersMask
    @abstract Set the packet filters for a given filter group.
    @discussion A network controller may support a number of packets filters
    that can accept or reject a type of packet seen on the network. A filter
    group identifies a set of related filters, such as all filters that will
    allow a packet to pass upstream based on the destination address encoded
    within the packet. This function allows an user-space program to set the
    filtering performed by a given filter group.
    @param connect The connection object returned from IONetworkOpen(). 
    @param filterGroup The name of the packet filter group.
    @param filtersMask A mask of filters to set.
    @param options No options are currently defined.
    @result An IOReturn error code. */

	IOReturn IONetworkSetPacketFiltersMask( io_connect_t    connect,
                                            const io_name_t filterGroup,
                                            UInt32          filtersMask,
                                            IOOptionBits    options );

/*! @enum IONetworkPacketFilterOptions
    @constant kIONetworkSupportedPacketFilters Indicate the filters that are
    supported by the hardware. */

    enum {
        kIONetworkSupportedPacketFilters = 0x0001
    };

/*! @function IONetworkGetPacketFiltersMask
    @abstract Get the packet filters for a given filter group.
    @discussion A network controller may support a number of packets filters
    that can accept or reject a type of packet seen on the network. A filter
    group identifies a set of related filters, such as all filters that will
    allow a packet to pass upstream based on the destination address encoded
    within the packet. This function allows an user-space program to get the
    filtering performed by a given filter group.
    @param connect The connection object returned from IONetworkOpen(). 
    @param filterGroup The name of the packet filter group.
    @param filtersMask Pointer to the return value containing a mask of
    packet filters.
    @param options kIONetworkSupportedPacketFilters may be set to fetch the
    filters that are supported by the hardware.
    @result An IOReturn error code. */

    IOReturn IONetworkGetPacketFiltersMask( io_connect_t    connect,
                                            const io_name_t filterGroup,
                                            UInt32 *        filtersMask,
                                            IOOptionBits    options );

#ifdef __cplusplus
}
#endif

#endif /* !_IONETWORKLIB_H */
