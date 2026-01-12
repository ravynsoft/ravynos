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

/*	CFData.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFDATA__)
#define __COREFOUNDATION_CFDATA__ 1

#include <CoreFoundation/CFBase.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN
    
typedef const struct CF_BRIDGED_TYPE(NSData) __CFData * CFDataRef;
typedef struct CF_BRIDGED_MUTABLE_TYPE(NSMutableData) __CFData * CFMutableDataRef;

CF_EXPORT
CFTypeID CFDataGetTypeID(void);

CF_EXPORT
CFDataRef CFDataCreate(CFAllocatorRef allocator, const UInt8 *bytes, CFIndex length);

CF_EXPORT
CFDataRef CFDataCreateWithBytesNoCopy(CFAllocatorRef allocator, const UInt8 *bytes, CFIndex length, CFAllocatorRef bytesDeallocator);
    /* Pass kCFAllocatorNull as bytesDeallocator to assure the bytes aren't freed */

CF_EXPORT
CFDataRef CFDataCreateCopy(CFAllocatorRef allocator, CFDataRef theData);

CF_EXPORT
CFMutableDataRef CFDataCreateMutable(CFAllocatorRef allocator, CFIndex capacity);

CF_EXPORT
CFMutableDataRef CFDataCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFDataRef theData);

CF_EXPORT
CFIndex CFDataGetLength(CFDataRef theData);

CF_EXPORT
const UInt8 *CFDataGetBytePtr(CFDataRef theData);

CF_EXPORT
UInt8 *CFDataGetMutableBytePtr(CFMutableDataRef theData);

CF_EXPORT
void CFDataGetBytes(CFDataRef theData, CFRange range, UInt8 *buffer); 

CF_EXPORT
void CFDataSetLength(CFMutableDataRef theData, CFIndex length);

CF_EXPORT
void CFDataIncreaseLength(CFMutableDataRef theData, CFIndex extraLength);

CF_EXPORT
void CFDataAppendBytes(CFMutableDataRef theData, const UInt8 *bytes, CFIndex length);

CF_EXPORT
void CFDataReplaceBytes(CFMutableDataRef theData, CFRange range, const UInt8 *newBytes, CFIndex newLength);

CF_EXPORT
void CFDataDeleteBytes(CFMutableDataRef theData, CFRange range);

typedef CF_OPTIONS(CFOptionFlags, CFDataSearchFlags) {
    kCFDataSearchBackwards = 1UL << 0,
    kCFDataSearchAnchored = 1UL << 1
} CF_ENUM_AVAILABLE(10_6, 4_0);

CF_EXPORT
CFRange CFDataFind(CFDataRef theData, CFDataRef dataToFind, CFRange searchRange, CFDataSearchFlags compareOptions) CF_AVAILABLE(10_6, 4_0);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFDATA__ */

