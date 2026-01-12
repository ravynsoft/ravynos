/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFUUID.h
	Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFUUID__)
#define __COREFOUNDATION_CFUUID__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef const struct __CFUUID * CFUUIDRef;

typedef struct {
    UInt8 byte0;
    UInt8 byte1;
    UInt8 byte2;
    UInt8 byte3;
    UInt8 byte4;
    UInt8 byte5;
    UInt8 byte6;
    UInt8 byte7;
    UInt8 byte8;
    UInt8 byte9;
    UInt8 byte10;
    UInt8 byte11;
    UInt8 byte12;
    UInt8 byte13;
    UInt8 byte14;
    UInt8 byte15;
} CFUUIDBytes;
/* The CFUUIDBytes struct is a 128-bit struct that contains the
raw UUID.  A CFUUIDRef can provide such a struct from the
CFUUIDGetUUIDBytes() function.  This struct is suitable for
passing to APIs that expect a raw UUID.
*/
        
CF_EXPORT
CFTypeID CFUUIDGetTypeID(void);

CF_EXPORT
CFUUIDRef CFUUIDCreate(CFAllocatorRef alloc);
    /* Create and return a brand new unique identifier */

CF_EXPORT
CFUUIDRef CFUUIDCreateWithBytes(CFAllocatorRef alloc, UInt8 byte0, UInt8 byte1, UInt8 byte2, UInt8 byte3, UInt8 byte4, UInt8 byte5, UInt8 byte6, UInt8 byte7, UInt8 byte8, UInt8 byte9, UInt8 byte10, UInt8 byte11, UInt8 byte12, UInt8 byte13, UInt8 byte14, UInt8 byte15);
    /* Create and return an identifier with the given contents.  This may return an existing instance with its ref count bumped because of uniquing. */

CF_EXPORT
CFUUIDRef CFUUIDCreateFromString(CFAllocatorRef alloc, CFStringRef uuidStr);
    /* Converts from a string representation to the UUID.  This may return an existing instance with its ref count bumped because of uniquing. */

CF_EXPORT
CFStringRef CFUUIDCreateString(CFAllocatorRef alloc, CFUUIDRef uuid);
    /* Converts from a UUID to its string representation. */

CF_EXPORT
CFUUIDRef CFUUIDGetConstantUUIDWithBytes(CFAllocatorRef alloc, UInt8 byte0, UInt8 byte1, UInt8 byte2, UInt8 byte3, UInt8 byte4, UInt8 byte5, UInt8 byte6, UInt8 byte7, UInt8 byte8, UInt8 byte9, UInt8 byte10, UInt8 byte11, UInt8 byte12, UInt8 byte13, UInt8 byte14, UInt8 byte15);
    /* This returns an immortal CFUUIDRef that should not be released.  It can be used in headers to declare UUID constants with #define. */

CF_EXPORT
CFUUIDBytes CFUUIDGetUUIDBytes(CFUUIDRef uuid);

CF_EXPORT
CFUUIDRef CFUUIDCreateFromUUIDBytes(CFAllocatorRef alloc, CFUUIDBytes bytes);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFUUID__ */

