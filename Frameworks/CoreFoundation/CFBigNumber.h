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

/*	CFBigNumber.h
	Copyright (c) 2012-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFBIGNUMBER__)
#define __COREFOUNDATION_CFBIGNUMBER__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFNumber.h>


// Base 1 billion number: each digit represents 0 to 999999999
typedef struct {
    uint32_t digits[5];
    int32_t sign:8;
    uint32_t __:24;
} _CFBigNum;

void _CFBigNumInitWithInt8(_CFBigNum *r, int8_t inNum);
void _CFBigNumInitWithInt16(_CFBigNum *r, int16_t inNum);
void _CFBigNumInitWithInt32(_CFBigNum *r, int32_t inNum);
void _CFBigNumInitWithInt64(_CFBigNum *r, int64_t inNum);
#ifdef __LP64__
void _CFBigNumInitWithInt128(_CFBigNum *r, __int128_t inNum);
#endif

void _CFBigNumInitWithUInt8(_CFBigNum *r, uint8_t inNum);
void _CFBigNumInitWithUInt16(_CFBigNum *r, uint16_t inNum);
void _CFBigNumInitWithUInt32(_CFBigNum *r, uint32_t inNum);
void _CFBigNumInitWithUInt64(_CFBigNum *r, uint64_t inNum);
#ifdef __LP64__
void _CFBigNumInitWithUInt128(_CFBigNum *r, __uint128_t inNum);
#endif

int8_t  _CFBigNumGetInt8(const _CFBigNum *num);
int16_t _CFBigNumGetInt16(const _CFBigNum *num);
int32_t _CFBigNumGetInt32(const _CFBigNum *num);
int64_t _CFBigNumGetInt64(const _CFBigNum *num);
#ifdef __LP64__
__int128_t _CFBigNumGetInt128(const _CFBigNum *num);
#endif

uint8_t  _CFBigNumGetUInt8(const _CFBigNum *num);
uint16_t _CFBigNumGetUInt16(const _CFBigNum *num);
uint32_t _CFBigNumGetUInt32(const _CFBigNum *num);
uint64_t _CFBigNumGetUInt64(const _CFBigNum *num);
#ifdef __LP64__
__uint128_t _CFBigNumGetUInt128(const _CFBigNum *num);
#endif

void _CFBigNumInitWithCFNumber(_CFBigNum *r, CFNumberRef input);
void _CFBigNumInitWithBytes(_CFBigNum *r, const void *bytes, CFNumberType type);
CFNumberRef _CFNumberCreateWithBigNum(const _CFBigNum *input);


CFComparisonResult _CFBigNumCompare(const _CFBigNum *a, const _CFBigNum *b);

void _CFBigNumNeg(_CFBigNum *r, const _CFBigNum *a);
uint8_t _CFBigNumAdd(_CFBigNum *r, const _CFBigNum *a, const _CFBigNum *b);
uint8_t _CFBigNumSub(_CFBigNum *r, const _CFBigNum *a, const _CFBigNum *b);


void _CFBigNumToCString(const _CFBigNum *vp, Boolean leading_zeros, Boolean leading_plus, char *buffer, size_t buflen);
void _CFBigNumFromCString(_CFBigNum *r, const char *string);

char *_CFBigNumCopyDescription(const _CFBigNum *num); // caller must free() returned ptr


#endif /* ! __COREFOUNDATION_CFBIGNUMBER__ */

