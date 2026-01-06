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

#ifndef _IONETWORKDATA_H
#define _IONETWORKDATA_H

#define IONetworkParameter IONetworkData  // FIXME

/*! @enum NetworkDataAccessTypes
    @abstract Constants that describe access types.
    @constant kIONetworkDataAccessTypeRead  Read access.
    @constant kIONetworkDataAccessTypeWrite Write access (deprecated).
    @constant kIONetworkDataAccessTypeReset Reset access (deprecated).
    @constant kIONetworkDataAccessTypeSerialize Serialization access. 
*/

enum {
    kIONetworkDataAccessTypeRead        = 0x01,
    kIONetworkDataAccessTypeWrite       = 0x02,
    kIONetworkDataAccessTypeReset       = 0x04,
    kIONetworkDataAccessTypeSerialize   = 0x08,
    kIONetworkDataAccessTypeMask        = 0xff
};

/*! @define kIONetworkDataBasicAccessTypes
    @discussion The default access types supported by an IONetworkData
    object. Allow read() and serialize(). */

#define kIONetworkDataBasicAccessTypes \
       (kIONetworkDataAccessTypeRead | kIONetworkDataAccessTypeSerialize)

/*! @enum NetworkDataBufferTypes
    @abstract The types of data buffers that can be managed by an IONetworkData object.
    @constant kIONetworkDataBufferTypeInternal An internal data buffer
              allocated by the init() method.
    @constant kIONetworkDataBufferTypeExternal An external (persistent) data
              buffer.
    @constant kIONetworkDataBufferTypeNone No data buffer. The only useful 
              action perfomed by an IONetworkData object with this buffer type 
              is to call the access notification handler. 
*/

enum {
    kIONetworkDataBufferTypeInternal = 0,
    kIONetworkDataBufferTypeExternal,
    kIONetworkDataBufferTypeNone
};

/*! @defined kIONetworkDataBytes
    @abstract A property of IONetworkData objects.
    @discussion The kIONetworkDataBytes property is an OSData that describes
        the data buffer of an IONetworkData object. This property is present
        only if kIONetworkDataAccessTypeSerialize access is supported. 
*/

#define kIONetworkDataBytes             "Data"

/*! @defined kIONetworkDataAccessTypes
    @abstract A property of IONetworkData objects.
    @discussion The kIONetworkDataAccessTypes property is an OSNumber that
        describes the supported access types of an IONetworkData object. 
*/

#define kIONetworkDataAccessTypes       "Access Types"

/*! @defined kIONetworkDataSize
    @abstract A property of IONetworkData objects.
    @discussion The kIONetworkDataSize property is an OSNumber that
        describes the size of the data buffer of an IONetworkData object. 
*/

#define kIONetworkDataSize              "Size"


#endif /* !_IONETWORKDATA_H */
