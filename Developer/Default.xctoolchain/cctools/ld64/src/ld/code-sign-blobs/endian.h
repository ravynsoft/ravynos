/*
 * Copyright (c) 2002-2004 Apple Computer, Inc. All Rights Reserved.
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
 * cssm utilities
 */
#ifndef _H_ENDIAN
#define _H_ENDIAN

#include <machine/endian.h>
#include <libkern/OSByteOrder.h>
//#include <security_utilities/utilities.h>
#include "memutils.h"

namespace Security {


//
// Encode/decode operations by type, overloaded.
// You can use these functions directly, but consider using
// the higher-level constructs below instead.
//
#ifdef __LP64__
static inline unsigned long h2n(unsigned long v) { return OSSwapHostToBigInt64(v); }
static inline unsigned long n2h(unsigned long v) { return OSSwapBigToHostInt64(v); }
static inline unsigned long flip(unsigned long v) { return OSSwapInt64(v); }
static inline signed long h2n(signed long v) { return OSSwapHostToBigInt64(v); }
static inline signed long n2h(signed long v) { return OSSwapBigToHostInt64(v); }
static inline signed long flip(signed long v) { return OSSwapInt64(v); }
#else
static inline unsigned long h2n(unsigned long v)	{ return htonl(v); }
static inline unsigned long n2h(unsigned long v)	{ return ntohl(v); }
static inline unsigned long flip(unsigned long v)	{ return OSSwapInt32(v); }
static inline signed long h2n(signed long v)		{ return htonl(v); }
static inline signed long n2h(signed long v)		{ return ntohl(v); }
static inline signed long flip(signed long v)		{ return OSSwapInt32(v); }
#endif

static inline unsigned long long h2n(unsigned long long v) { return OSSwapHostToBigInt64(v); }
static inline unsigned long long n2h(unsigned long long v) { return OSSwapBigToHostInt64(v); }
static inline unsigned long long flip(unsigned long long v) { return OSSwapInt64(v); }
static inline long long h2n(long long v)			{ return OSSwapHostToBigInt64(v); }
static inline long long n2h(long long v)			{ return OSSwapBigToHostInt64(v); }
static inline long long flip(long long v)			{ return OSSwapInt64(v); }

static inline unsigned int h2n(unsigned int v)		{ return htonl(v); }
static inline unsigned int n2h(unsigned int v)		{ return ntohl(v); }
static inline unsigned int flip(unsigned int v)		{ return OSSwapInt32(v); }
static inline signed int h2n(int v)					{ return htonl(v); }
static inline signed int n2h(int v)					{ return ntohl(v); }
static inline signed int flip(int v)				{ return OSSwapInt32(v); }

static inline unsigned short h2n(unsigned short v)	{ return htons(v); }
static inline unsigned short n2h(unsigned short v)	{ return ntohs(v); }
static inline unsigned short flip(unsigned short v)	{ return OSSwapInt16(v); }
static inline signed short h2n(signed short v)		{ return htons(v); }
static inline signed short n2h(signed short v)		{ return ntohs(v); }
static inline signed short flip(signed short v)		{ return OSSwapInt16(v); }

static inline unsigned char h2n(unsigned char v)	{ return v; }
static inline unsigned char n2h(unsigned char v)	{ return v; }
static inline unsigned char flip(unsigned char v)	{ return v; }
static inline signed char h2n(signed char v)		{ return v; }
static inline signed char n2h(signed char v)		{ return v; }
static inline signed char flip(signed char v)		{ return v; }


//
// Flip pointers
//
template <class Base>
static inline Base *h2n(Base *p)	{ return (Base *)h2n(uintptr_t(p)); }

template <class Base>
static inline Base *n2h(Base *p)	{ return (Base *)n2h(uintptr_t(p)); }


//
// In-place fix operations
//
template <class Type>
static inline void h2ni(Type &v)	{ v = h2n(v); }

template <class Type>
static inline void n2hi(Type &v)	{ v = n2h(v); }

//
// Endian<SomeType> keeps NBO values in memory and converts
// during loads and stores. This presumes that you are using
// memory blocks thare are read/written/mapped as amorphous byte
// streams, but want to be byte-order clean using them.
//
// The generic definition uses h2n/n2h to flip bytes. Feel free
// to declare specializations of Endian<T> as appropriate.
//
// Note well that the address of an Endian<T> is not an address-of-T,
// and there is no conversion available.
//
template <class Type>
class Endian {
public:
    typedef Type Value;
    Endian() : mValue(Type(0)) { }
    Endian(Value v) : mValue(h2n(v)) { }
    
    operator Value () const		{ return n2h(mValue); }
    Endian &operator = (Value v)	{ mValue = h2n(v); return *this; }
    
private:
    Value mValue;
};


}	// end namespace Security


#endif //_H_ENDIAN
