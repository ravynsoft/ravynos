/*	CFByteOrder.h
	Copyright (c) 1995-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFBYTEORDER__)
#define __COREFOUNDATION_CFBYTEORDER__ 1

#include <CoreFoundation/CFBase.h>
#if (TARGET_OS_OSX || TARGET_OS_IPHONE) && !defined(CF_USE_OSBYTEORDER_H)
#include <libkern/OSByteOrder.h>
#define CF_USE_OSBYTEORDER_H 1
#endif

CF_EXTERN_C_BEGIN

enum __CFByteOrder {
    CFByteOrderUnknown,
    CFByteOrderLittleEndian,
    CFByteOrderBigEndian
};
typedef CFIndex CFByteOrder;

CF_INLINE CFByteOrder CFByteOrderGetCurrent(void) {
#if CF_USE_OSBYTEORDER_H
    int32_t byteOrder = OSHostByteOrder();
    switch (byteOrder) {
    case OSLittleEndian: return CFByteOrderLittleEndian;
    case OSBigEndian: return CFByteOrderBigEndian;
    default: break;
    }
    return CFByteOrderUnknown;
#else
#if __LITTLE_ENDIAN__
    return CFByteOrderLittleEndian;
#elif __BIG_ENDIAN__
    return CFByteOrderBigEndian;
#else
    return CFByteOrderUnknown;
#endif
#endif
}

CF_INLINE uint16_t CFSwapInt16(uint16_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapInt16(arg);
#else
    uint16_t result;
    result = (uint16_t)(((arg << 8) & 0xFF00) | ((arg >> 8) & 0xFF));
    return result;
#endif
}

CF_INLINE uint32_t CFSwapInt32(uint32_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapInt32(arg);
#else
    uint32_t result;
    result = ((arg & 0xFF) << 24) | ((arg & 0xFF00) << 8) | ((arg >> 8) & 0xFF00) | ((arg >> 24) & 0xFF);
    return result;
#endif
}

CF_INLINE uint64_t CFSwapInt64(uint64_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapInt64(arg);
#else
    union CFSwap {
        uint64_t sv;
        uint32_t ul[2];
    } tmp, result;
    tmp.sv = arg;
    result.ul[0] = CFSwapInt32(tmp.ul[1]); 
    result.ul[1] = CFSwapInt32(tmp.ul[0]);
    return result.sv;
#endif
}

CF_INLINE uint16_t CFSwapInt16BigToHost(uint16_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapBigToHostInt16(arg);
#elif __BIG_ENDIAN__
    return arg;
#else
    return CFSwapInt16(arg);
#endif
}

CF_INLINE uint32_t CFSwapInt32BigToHost(uint32_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapBigToHostInt32(arg);
#elif __BIG_ENDIAN__
    return arg;
#else
    return CFSwapInt32(arg);
#endif
}

CF_INLINE uint64_t CFSwapInt64BigToHost(uint64_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapBigToHostInt64(arg);
#elif __BIG_ENDIAN__
    return arg;
#else
    return CFSwapInt64(arg);
#endif
}

CF_INLINE uint16_t CFSwapInt16HostToBig(uint16_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapHostToBigInt16(arg);
#elif __BIG_ENDIAN__
    return arg;
#else
    return CFSwapInt16(arg);
#endif
}

CF_INLINE uint32_t CFSwapInt32HostToBig(uint32_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapHostToBigInt32(arg);
#elif __BIG_ENDIAN__
    return arg;
#else
    return CFSwapInt32(arg);
#endif
}

CF_INLINE uint64_t CFSwapInt64HostToBig(uint64_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapHostToBigInt64(arg);
#elif __BIG_ENDIAN__
    return arg;
#else
    return CFSwapInt64(arg);
#endif
}

CF_INLINE uint16_t CFSwapInt16LittleToHost(uint16_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapLittleToHostInt16(arg);
#elif __LITTLE_ENDIAN__
    return arg;
#else
    return CFSwapInt16(arg);
#endif
}

CF_INLINE uint32_t CFSwapInt32LittleToHost(uint32_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapLittleToHostInt32(arg);
#elif __LITTLE_ENDIAN__
    return arg;
#else
    return CFSwapInt32(arg);
#endif
}

CF_INLINE uint64_t CFSwapInt64LittleToHost(uint64_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapLittleToHostInt64(arg);
#elif __LITTLE_ENDIAN__
    return arg;
#else
    return CFSwapInt64(arg);
#endif
}

CF_INLINE uint16_t CFSwapInt16HostToLittle(uint16_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapHostToLittleInt16(arg);
#elif __LITTLE_ENDIAN__
    return arg;
#else
    return CFSwapInt16(arg);
#endif
}

CF_INLINE uint32_t CFSwapInt32HostToLittle(uint32_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapHostToLittleInt32(arg);
#elif __LITTLE_ENDIAN__
    return arg;
#else
    return CFSwapInt32(arg);
#endif
}

CF_INLINE uint64_t CFSwapInt64HostToLittle(uint64_t arg) {
#if CF_USE_OSBYTEORDER_H
    return OSSwapHostToLittleInt64(arg);
#elif __LITTLE_ENDIAN__
    return arg;
#else
    return CFSwapInt64(arg);
#endif
}

typedef struct {uint32_t v;} CFSwappedFloat32;
typedef struct {uint64_t v;} CFSwappedFloat64;

CF_INLINE CFSwappedFloat32 CFConvertFloat32HostToSwapped(Float32 arg) {
    union CFSwap {
	Float32 v;
	CFSwappedFloat32 sv;
    } result;
    result.v = arg;
#if __LITTLE_ENDIAN__
    result.sv.v = CFSwapInt32(result.sv.v);
#endif
    return result.sv;
}

CF_INLINE Float32 CFConvertFloat32SwappedToHost(CFSwappedFloat32 arg) {
    union CFSwap {
	Float32 v;
	CFSwappedFloat32 sv;
    } result;
    result.sv = arg;
#if __LITTLE_ENDIAN__
    result.sv.v = CFSwapInt32(result.sv.v);
#endif
    return result.v;
}

CF_INLINE CFSwappedFloat64 CFConvertFloat64HostToSwapped(Float64 arg) {
    union CFSwap {
	Float64 v;
	CFSwappedFloat64 sv;
    } result;
    result.v = arg;
#if __LITTLE_ENDIAN__
    result.sv.v = CFSwapInt64(result.sv.v);
#endif
    return result.sv;
}

CF_INLINE Float64 CFConvertFloat64SwappedToHost(CFSwappedFloat64 arg) {
    union CFSwap {
	Float64 v;
	CFSwappedFloat64 sv;
    } result;
    result.sv = arg;
#if __LITTLE_ENDIAN__
    result.sv.v = CFSwapInt64(result.sv.v);
#endif
    return result.v;
}

CF_INLINE CFSwappedFloat32 CFConvertFloatHostToSwapped(float arg) {
    union CFSwap {
	float v;
	CFSwappedFloat32 sv;
    } result;
    result.v = arg;
#if __LITTLE_ENDIAN__
    result.sv.v = CFSwapInt32(result.sv.v);
#endif
    return result.sv;
}

CF_INLINE float CFConvertFloatSwappedToHost(CFSwappedFloat32 arg) {
    union CFSwap {
	float v;
	CFSwappedFloat32 sv;
    } result;
    result.sv = arg;
#if __LITTLE_ENDIAN__
    result.sv.v = CFSwapInt32(result.sv.v);
#endif
    return result.v;
}

CF_INLINE CFSwappedFloat64 CFConvertDoubleHostToSwapped(double arg) {
    union CFSwap {
	double v;
	CFSwappedFloat64 sv;
    } result;
    result.v = arg;
#if __LITTLE_ENDIAN__
    result.sv.v = CFSwapInt64(result.sv.v);
#endif
    return result.sv;
}

CF_INLINE double CFConvertDoubleSwappedToHost(CFSwappedFloat64 arg) {
    union CFSwap {
	double v;
	CFSwappedFloat64 sv;
    } result;
    result.sv = arg;
#if __LITTLE_ENDIAN__
    result.sv.v = CFSwapInt64(result.sv.v);
#endif
    return result.v;
}

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFBYTEORDER__ */

