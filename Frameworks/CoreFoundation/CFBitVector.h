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

/*	CFBitVector.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFBITVECTOR__)
#define __COREFOUNDATION_CFBITVECTOR__ 1

#include <CoreFoundation/CFBase.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef UInt32 CFBit;

typedef const struct __CFBitVector * CFBitVectorRef;
typedef struct __CFBitVector * CFMutableBitVectorRef;

CF_EXPORT CFTypeID	CFBitVectorGetTypeID(void);

CF_EXPORT CFBitVectorRef	CFBitVectorCreate(CFAllocatorRef allocator, const UInt8 *bytes, CFIndex numBits);
CF_EXPORT CFBitVectorRef	CFBitVectorCreateCopy(CFAllocatorRef allocator, CFBitVectorRef bv);
CF_EXPORT CFMutableBitVectorRef	CFBitVectorCreateMutable(CFAllocatorRef allocator, CFIndex capacity);
CF_EXPORT CFMutableBitVectorRef	CFBitVectorCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFBitVectorRef bv);

CF_EXPORT CFIndex	CFBitVectorGetCount(CFBitVectorRef bv);
CF_EXPORT CFIndex	CFBitVectorGetCountOfBit(CFBitVectorRef bv, CFRange range, CFBit value);
CF_EXPORT Boolean	CFBitVectorContainsBit(CFBitVectorRef bv, CFRange range, CFBit value);
CF_EXPORT CFBit		CFBitVectorGetBitAtIndex(CFBitVectorRef bv, CFIndex idx);
CF_EXPORT void		CFBitVectorGetBits(CFBitVectorRef bv, CFRange range, UInt8 *bytes);
CF_EXPORT CFIndex	CFBitVectorGetFirstIndexOfBit(CFBitVectorRef bv, CFRange range, CFBit value);
CF_EXPORT CFIndex	CFBitVectorGetLastIndexOfBit(CFBitVectorRef bv, CFRange range, CFBit value);

CF_EXPORT void		CFBitVectorSetCount(CFMutableBitVectorRef bv, CFIndex count);
CF_EXPORT void		CFBitVectorFlipBitAtIndex(CFMutableBitVectorRef bv, CFIndex idx);
CF_EXPORT void		CFBitVectorFlipBits(CFMutableBitVectorRef bv, CFRange range);
CF_EXPORT void		CFBitVectorSetBitAtIndex(CFMutableBitVectorRef bv, CFIndex idx, CFBit value);
CF_EXPORT void		CFBitVectorSetBits(CFMutableBitVectorRef bv, CFRange range, CFBit value);
CF_EXPORT void		CFBitVectorSetAllBits(CFMutableBitVectorRef bv, CFBit value);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFBITVECTOR__ */

