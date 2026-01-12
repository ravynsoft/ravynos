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

/*	CFData.c
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
	Responsibility: Kevin Perry
*/

#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"
#include <string.h>



#if __LP64__
#define CFDATA_MAX_SIZE	    ((1ULL << 42) - 1)
#else
#define CFDATA_MAX_SIZE	    ((1ULL << 31) - 1)
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#import <mach/mach.h>
CF_INLINE unsigned long __CFPageSize() { return vm_page_size; }
#elif DEPLOYMENT_TARGET_WINDOWS
CF_INLINE unsigned long __CFPageSize() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwPageSize;
}
#elif DEPLOYMENT_TARGET_LINUX
#include <unistd.h>
CF_INLINE unsigned long __CFPageSize() {
    return (unsigned long)getpagesize();
}
#endif

#define INLINE_BYTES_THRESHOLD ((4 * __CFPageSize()) - sizeof(struct __CFData) - 15)

struct __CFData {
    CFRuntimeBase _base;
    CFIndex _length;	/* number of bytes */
    CFIndex _capacity;	/* maximum number of bytes */
    CFAllocatorRef _bytesDeallocator;	/* used only for immutable; if NULL, no deallocation */
    uint8_t *_bytes;	/* compaction: direct access to _bytes is only valid when data is not inline */
};

/*  
 Bit 0 = is mutable
 Bit 1 = growable
 Bit 2 = bytes inline
 Bit 3 = use given CFAllocator
 Bit 5 = allocate collectable memory
 
 Bits 1-0 are used for mutability variation
 
 Bit 6 = not all bytes have been zeroed yet (mutable)
 */

enum {
    __kCFMutable = 0x01,
    __kCFGrowable = 0x02,
    __kCFMutableVarietyMask = 0x03,
    __kCFBytesInline = 0x04,
    __kCFUseAllocator = 0x08,
    __kCFAllocatesCollectable = 0x20,
};

enum {
    kCFImmutable = 0x0,		/* unchangable and fixed capacity; default */
    kCFFixedMutable = 0x1,	/* changeable and fixed capacity */
    kCFMutable = 0x3		/* changeable and variable capacity */
};

CF_INLINE void __CFDataSetInfoBits(CFDataRef data, UInt32 v) {__CFBitfieldSetValue(((CFRuntimeBase *)data)->_cfinfo[CF_INFO_BITS], 5, 0, v);}
CF_INLINE Boolean __CFDataGetInfoBit(CFDataRef data, UInt32 b) {return ((((const CFRuntimeBase *)data)->_cfinfo[CF_INFO_BITS] & b) != 0);}
CF_INLINE Boolean __CFDataIsMutable(CFDataRef data) {return __CFDataGetInfoBit(data, __kCFMutable);}
CF_INLINE Boolean __CFDataIsGrowable(CFDataRef data) {return __CFDataGetInfoBit(data, __kCFGrowable);}
CF_INLINE Boolean __CFDataBytesInline(CFDataRef data) {return __CFDataGetInfoBit(data, __kCFBytesInline);}
CF_INLINE Boolean __CFDataUseAllocator(CFDataRef data) {return __CFDataGetInfoBit(data, __kCFUseAllocator);}
CF_INLINE Boolean __CFDataAllocatesCollectable(CFDataRef data) {return __CFDataGetInfoBit(data, __kCFAllocatesCollectable);}

CF_INLINE UInt32 __CFMutableVariety(const void *cf) {
    return __CFBitfieldGetValue(((const CFRuntimeBase *)cf)->_cfinfo[CF_INFO_BITS], 1, 0);
}

CF_INLINE void __CFSetMutableVariety(void *cf, UInt32 v) {
    __CFBitfieldSetValue(((CFRuntimeBase *)cf)->_cfinfo[CF_INFO_BITS], 1, 0, v);
}

CF_INLINE UInt32 __CFMutableVarietyFromFlags(UInt32 flags) {
    return (flags & __kCFMutableVarietyMask);
}

#define __CFGenericValidateMutabilityFlags(flags) \
    CFAssert2(__CFMutableVarietyFromFlags(flags) != 0x2, __kCFLogAssertion, "%s(): flags 0x%x do not correctly specify the mutable variety", __PRETTY_FUNCTION__, flags);
    
CF_INLINE void __CFDataSetInline(CFDataRef data, Boolean flag) {
    __CFBitfieldSetValue(((CFRuntimeBase *)data)->_cfinfo[CF_INFO_BITS], 2, 2, (flag ? 1 : 0));
}

CF_INLINE Boolean __CFDataNeedsToZero(CFDataRef data) {
    return __CFBitfieldGetValue(((CFRuntimeBase *)data)->_cfinfo[CF_INFO_BITS], 6, 6);
}

CF_INLINE void __CFDataSetNeedsToZero(CFDataRef data, Boolean zero) {
    __CFBitfieldSetValue(((CFRuntimeBase *)data)->_cfinfo[CF_INFO_BITS], 6, 6, (zero ? 1 : 0));
}

CF_INLINE CFIndex __CFDataLength(CFDataRef data) {
    return data->_length;
}

CF_INLINE void __CFDataSetLength(CFMutableDataRef data, CFIndex v) {
    /* for a CFData, _bytesUsed == _length */
}

CF_INLINE CFIndex __CFDataCapacity(CFDataRef data) {
    return data->_capacity;
}

CF_INLINE void __CFDataSetCapacity(CFMutableDataRef data, CFIndex v) {
    /* for a CFData, _bytesNum == _capacity */
}

CF_INLINE void __CFDataSetNumBytesUsed(CFMutableDataRef data, CFIndex v) {
    data->_length = v;
}

CF_INLINE CFIndex __CFDataNumBytes(CFDataRef data) {
    return data->_capacity;
}

CF_INLINE void __CFDataSetNumBytes(CFMutableDataRef data, CFIndex v) {
    data->_capacity = v;
}

#if __LP64__
#define CHUNK_SIZE (1ULL << 29)
#define LOW_THRESHOLD (1ULL << 20)
#define HIGH_THRESHOLD (1ULL << 32)
#else
#define CHUNK_SIZE (1ULL << 26)
#define LOW_THRESHOLD (1ULL << 20)
#define HIGH_THRESHOLD (1ULL << 29)
#endif

CF_INLINE CFIndex __CFDataRoundUpCapacity(CFIndex capacity) {
    if (capacity < 16) {
	return 16;
    } else if (capacity < LOW_THRESHOLD) {
	/* Up to 4x */
	long idx = flsl(capacity);
	return (1L << (long)(idx + ((idx % 2 == 0) ? 0 : 1)));
    } else if (capacity < HIGH_THRESHOLD) {
	/* Up to 2x */
	return (1L << (long)flsl(capacity));
    } else {
	/* Round up to next multiple of CHUNK_SIZE */
	unsigned long newCapacity = CHUNK_SIZE * (1+(capacity >> ((long)flsl(CHUNK_SIZE)-1)));
	return __CFMin(newCapacity, CFDATA_MAX_SIZE);
    }
}

CF_INLINE CFIndex __CFDataNumBytesForCapacity(CFIndex capacity) {
    return capacity;
}

static void __CFDataHandleOutOfMemory(CFTypeRef obj, CFIndex numBytes) {
    CFStringRef msg;
    if(0 < numBytes && numBytes <= CFDATA_MAX_SIZE) {
	msg = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("Attempt to allocate %ld bytes for NS/CFData failed"), numBytes);
    } else {
	msg = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("Attempt to allocate %ld bytes for NS/CFData failed. Maximum size: %lld"), numBytes, CFDATA_MAX_SIZE);
    }
    {
        CFLog(kCFLogLevelCritical, CFSTR("%@"), msg);
        HALT;
    }
    CFRelease(msg);
}

#if defined(DEBUG)
CF_INLINE void __CFDataValidateRange(CFDataRef data, CFRange range, const char *func) {
    CFAssert2(0 <= range.location && range.location <= __CFDataLength(data), __kCFLogAssertion, "%s(): range.location index (%d) out of bounds", func, range.location);
    CFAssert2(0 <= range.length, __kCFLogAssertion, "%s(): length (%d) cannot be less than zero", func, range.length);
    CFAssert2(range.location + range.length <= __CFDataLength(data), __kCFLogAssertion, "%s(): ending index (%d) out of bounds", func, range.location + range.length);
}
#else
#define __CFDataValidateRange(a,r,f)
#endif

static Boolean __CFDataEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFDataRef data1 = (CFDataRef)cf1;
    CFDataRef data2 = (CFDataRef)cf2;
    CFIndex length;
    length = __CFDataLength(data1);
    if (length != __CFDataLength(data2)) return false;
    const uint8_t *bytePtr1 = CFDataGetBytePtr(data1);
    const uint8_t *bytePtr2 = CFDataGetBytePtr(data2);
    return 0 == memcmp(bytePtr1, bytePtr2, length);
}

static CFHashCode __CFDataHash(CFTypeRef cf) {
    CFDataRef data = (CFDataRef)cf;
    return CFHashBytes((uint8_t *)CFDataGetBytePtr(data), __CFMin(__CFDataLength(data), 80));
}

static CFStringRef __CFDataCopyDescription(CFTypeRef cf) {
    CFDataRef data = (CFDataRef)cf;
    CFMutableStringRef result;
    CFIndex idx;
    CFIndex len;
    const uint8_t *bytes;
    len = __CFDataLength(data);
    bytes = CFDataGetBytePtr(data);
    result = CFStringCreateMutable(CFGetAllocator(data), 0);
    CFStringAppendFormat(result, NULL, CFSTR("<CFData %p [%p]>{length = %lu, capacity = %lu, bytes = 0x"), cf, CFGetAllocator(data), (unsigned long)len, (unsigned long)__CFDataCapacity(data));
    if (24 < len) {
        for (idx = 0; idx < 16; idx += 4) {
	    CFStringAppendFormat(result, NULL, CFSTR("%02x%02x%02x%02x"), bytes[idx], bytes[idx + 1], bytes[idx + 2], bytes[idx + 3]);
	}
        CFStringAppend(result, CFSTR(" ... "));
        for (idx = len - 8; idx < len; idx += 4) {
	    CFStringAppendFormat(result, NULL, CFSTR("%02x%02x%02x%02x"), bytes[idx], bytes[idx + 1], bytes[idx + 2], bytes[idx + 3]);
	}
    } else {
        for (idx = 0; idx < len; idx++) {
	    CFStringAppendFormat(result, NULL, CFSTR("%02x"), bytes[idx]);
	}
    }
    CFStringAppend(result, CFSTR("}"));
    return result;
}

static void *__CFDataInlineBytesPtr(CFDataRef data) {
    return (void *)((uintptr_t)((int8_t *)data + sizeof(struct __CFData) + 15) & ~0xF);	// 16-byte align
}
    
static Boolean __CFDataShouldAllocateCleared(CFDataRef data, CFIndex size) {
    Boolean result;
    if (__CFDataUseAllocator(data)) {
	result = false;
    } else {
	if (__CFDataAllocatesCollectable(data)) {
#if __LP64__
	    result = false;
#else
	    result = (size > (64 * 1024));
#endif
	} else {
	    result = (size > (128 * 1024));
	}
    }
    return result;
}

    
// Check __CFDataShouldAllocateCleared before passing true.
static void *__CFDataAllocate(CFDataRef data, CFIndex size, Boolean clear) {
    void *bytes = NULL;
    if (__CFDataUseAllocator(data)) {
	CFAllocatorRef allocator = __CFGetAllocator(data);
	bytes = CFAllocatorAllocate(allocator, size, 0);
	if (clear) memset((uint8_t *)bytes, 0, size);
    } else {
	if (__CFDataAllocatesCollectable(data)) {
	    bytes = auto_zone_allocate_object(objc_collectableZone(), size, AUTO_MEMORY_UNSCANNED, 0, clear);
	} else {
	    if (clear) {
		bytes = calloc(1, size);
	    } else {
		bytes = malloc(size);
	    }
	}
    }
    return bytes;
}

static void __CFDataDeallocate(CFTypeRef cf) {
    CFMutableDataRef data = (CFMutableDataRef)cf;
    if (!__CFDataBytesInline(data)) {
	CFAllocatorRef deallocator = data->_bytesDeallocator;
	if (deallocator != NULL) {
	    _CFAllocatorDeallocateGC(deallocator, data->_bytes);
	    CFRelease(deallocator);
	    data->_bytes = NULL;
	} else {
	    if (__CFDataUseAllocator(data)) {
		_CFAllocatorDeallocateGC(__CFGetAllocator(data), data->_bytes);
	    } else if (!__CFDataAllocatesCollectable(data) && data->_bytes) {
		free(data->_bytes);
	    }
	    data->_bytes = NULL;
	}
    }
}

static CFTypeID __kCFDataTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFDataClass = {
    _kCFRuntimeScannedObject,
    "CFData",
    NULL,	// init
    NULL,	// copy
    __CFDataDeallocate,
    __CFDataEqual,
    __CFDataHash,
    NULL,	// 
    __CFDataCopyDescription
};

CFTypeID CFDataGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFDataTypeID = _CFRuntimeRegisterClass(&__CFDataClass); });
    return __kCFDataTypeID;
}


// NULL bytesDeallocator to this function does not mean the default allocator, it means
// that there should be no deallocator, and the bytes should be copied.
static CFMutableDataRef __CFDataInit(CFAllocatorRef allocator, CFOptionFlags flags, CFIndex capacity, const uint8_t *bytes, CFIndex length, CFAllocatorRef bytesDeallocator) {
    CFMutableDataRef memory;
    __CFGenericValidateMutabilityFlags(flags);
    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%d) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFAssert3(kCFFixedMutable != __CFMutableVarietyFromFlags(flags) || length <= capacity, __kCFLogAssertion, "%s(): for kCFFixedMutable type, capacity (%d) must be greater than or equal to number of initial elements (%d)", __PRETTY_FUNCTION__, capacity, length);
    CFAssert2(0 <= length, __kCFLogAssertion, "%s(): length (%d) cannot be less than zero", __PRETTY_FUNCTION__, length);

    Boolean collectableMemory = CF_IS_COLLECTABLE_ALLOCATOR(allocator);
    Boolean noCopy = bytesDeallocator != NULL;
    Boolean isMutable = ((flags & __kCFMutable) != 0);
    Boolean isGrowable = ((flags & __kCFGrowable) != 0);
    Boolean allocateInline = !isGrowable && !noCopy && capacity < INLINE_BYTES_THRESHOLD;
    allocator = (allocator == NULL) ? __CFGetDefaultAllocator() : allocator;
    Boolean useAllocator = (allocator != kCFAllocatorSystemDefault && allocator != kCFAllocatorMalloc && allocator != kCFAllocatorMallocZone);
    
    CFIndex size = sizeof(struct __CFData) - sizeof(CFRuntimeBase);
    if (allocateInline) {
	size += sizeof(uint8_t) * __CFDataNumBytesForCapacity(capacity) + sizeof(uint8_t) * 15;	// for 16-byte alignment fixup
    }
    memory = (CFMutableDataRef)_CFRuntimeCreateInstance(allocator, CFDataGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }
    __CFDataSetNumBytesUsed(memory, 0);
    __CFDataSetLength(memory, 0);
    __CFDataSetInfoBits(memory,
			(allocateInline ? __kCFBytesInline : 0) | 
			(useAllocator ? __kCFUseAllocator : 0) |
			(collectableMemory ? __kCFAllocatesCollectable : 0));
    
    BOOL finalize = YES;
    BOOL scan = YES;
    if (collectableMemory) {
	if (allocateInline) {
	    // We have no pointer to anything that needs to be reclaimed, so don't scan or finalize.
	    scan = NO;
	    finalize = NO;
	} else if (noCopy) {
	    if (CF_IS_COLLECTABLE_ALLOCATOR(bytesDeallocator)) {
		// We're taking responsibility for externally GC-allocated memory, so scan us, but we don't need to finalize.
		finalize = NO;
	    } else if (bytesDeallocator == kCFAllocatorNull) {
		// We don't have responsibility for these bytes, so there's no need to be scanned and we don't need to finalize.
		scan = NO;
		finalize = NO;
	    } else {
		// We have a pointer to non-GC-allocated memory, so don't scan, but do finalize.
		scan = NO;
	    }
	}
	if (!scan) auto_zone_set_unscanned(objc_collectableZone(), memory);
	if (!finalize) auto_zone_set_nofinalize(objc_collectableZone(), memory);
    }
    if (isMutable && isGrowable) {
	__CFDataSetCapacity(memory, __CFDataRoundUpCapacity(1));
	__CFDataSetNumBytes(memory, __CFDataNumBytesForCapacity(__CFDataRoundUpCapacity(1)));
	__CFSetMutableVariety(memory, kCFMutable);
    } else {
	/* Don't round up capacity */
	__CFDataSetCapacity(memory, capacity);
	__CFDataSetNumBytes(memory, __CFDataNumBytesForCapacity(capacity));
	__CFSetMutableVariety(memory, kCFFixedMutable);
    }
    if (noCopy) {
	__CFAssignWithWriteBarrier((void **)&memory->_bytes, (uint8_t *)bytes);
	if (finalize) {
            if ((0)) {
	        memory->_bytesDeallocator = bytesDeallocator;
            } else {
	        memory->_bytesDeallocator = (CFAllocatorRef)CFRetain(bytesDeallocator);
            }
	}
	if (CF_IS_COLLECTABLE_ALLOCATOR(bytesDeallocator) && !(0)) {
	    // we assume that the no-copy memory is GC-allocated with a retain count of (at least) 1 and we should release it now instead of waiting until __CFDataDeallocate.
	    auto_zone_release(objc_collectableZone(), memory->_bytes);
	}
	__CFDataSetNumBytesUsed(memory, length);
	__CFDataSetLength(memory, length);
	// Mutable no-copy datas are not allowed, so don't bother setting needsToZero flag.
    } else {
	Boolean cleared = (isMutable && !isGrowable && !_CFExecutableLinkedOnOrAfter(CFSystemVersionSnowLeopard));
	if (!allocateInline) {
	    // assume that allocators give 16-byte aligned memory back -- it is their responsibility
	    __CFAssignWithWriteBarrier((void **)&memory->_bytes, __CFDataAllocate(memory, __CFDataNumBytes(memory) * sizeof(uint8_t), cleared));
	    if (__CFOASafe) __CFSetLastAllocationEventName(memory->_bytes, "CFData (store)");
	    if (NULL == memory->_bytes) {
		CFRelease(memory);
		return NULL;
	    }
	} else {
	    if (length == 0 && !isMutable) {
                // NSData sets its bytes pointer to NULL when its length is zero. Starting in 10.7 we do the same for CFData.
                memory->_bytes = NULL;
                // It is important to set this data as not inlined, so we do not recalculate a bytes pointer from null.
                __CFDataSetInline(memory, false);
	    }
	    cleared = true;
	}
	__CFDataSetNeedsToZero(memory, !cleared);
	memory->_bytesDeallocator = NULL;
	CFDataReplaceBytes(memory, CFRangeMake(0, 0), bytes, length);
    }
    __CFSetMutableVariety(memory, __CFMutableVarietyFromFlags(flags));
    return memory;
}

CFDataRef CFDataCreate(CFAllocatorRef allocator, const uint8_t *bytes, CFIndex length) {
    return __CFDataInit(allocator, kCFImmutable, length, bytes, length, NULL);
}

CFDataRef CFDataCreateWithBytesNoCopy(CFAllocatorRef allocator, const uint8_t *bytes, CFIndex length, CFAllocatorRef bytesDeallocator) {
    CFAssert1((0 == length || bytes != NULL), __kCFLogAssertion, "%s(): bytes pointer cannot be NULL if length is non-zero", __PRETTY_FUNCTION__);
    if (NULL == bytesDeallocator) bytesDeallocator = __CFGetDefaultAllocator();
    return __CFDataInit(allocator, kCFImmutable, length, bytes, length, bytesDeallocator);
}

CFDataRef CFDataCreateCopy(CFAllocatorRef allocator, CFDataRef data) {
    CFIndex length = CFDataGetLength(data);
    return __CFDataInit(allocator, kCFImmutable, length, CFDataGetBytePtr(data), length, NULL);
}

CFMutableDataRef CFDataCreateMutable(CFAllocatorRef allocator, CFIndex capacity) {
    // Do not allow magic allocator for now for mutable datas, because it
    // isn't remembered for proper handling later when growth of the buffer
    // has to occur.
    Boolean wasMagic = (0);
    CFMutableDataRef r = (CFMutableDataRef)__CFDataInit(allocator, (0 == capacity) ? kCFMutable : kCFFixedMutable, capacity, NULL, 0, NULL);
    if (wasMagic) CFMakeCollectable(r);
    return r;
}

CFMutableDataRef CFDataCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFDataRef data) {
    // Do not allow magic allocator for now for mutable datas, because it
    // isn't remembered for proper handling later when growth of the buffer
    // has to occur.
    Boolean wasMagic = (0);
    CFMutableDataRef r = (CFMutableDataRef) __CFDataInit(allocator, (0 == capacity) ? kCFMutable : kCFFixedMutable, capacity, CFDataGetBytePtr(data), CFDataGetLength(data), NULL);
    if (wasMagic) CFMakeCollectable(r);
    return r;
}

CFIndex CFDataGetLength(CFDataRef data) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), CFIndex, (NSData *)data, length);
    __CFGenericValidateType(data, CFDataGetTypeID());
    return __CFDataLength(data);
}

const uint8_t *CFDataGetBytePtr(CFDataRef data) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), const uint8_t *, (NSData *)data, bytes);
    __CFGenericValidateType(data, CFDataGetTypeID());
    // compaction: if inline, always do the computation.
    return __CFDataBytesInline(data) ? (uint8_t *)__CFDataInlineBytesPtr(data) : data->_bytes;
}

uint8_t *CFDataGetMutableBytePtr(CFMutableDataRef data) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), uint8_t *, (NSMutableData *)data, mutableBytes);
    CFAssert1(__CFDataIsMutable(data), __kCFLogAssertion, "%s(): data is immutable", __PRETTY_FUNCTION__);
    // compaction: if inline, always do the computation.
    return __CFDataBytesInline(data) ? (uint8_t *)__CFDataInlineBytesPtr(data) : data->_bytes;
}

void CFDataGetBytes(CFDataRef data, CFRange range, uint8_t *buffer) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSData *)data, getBytes:(void *)buffer range:NSMakeRange(range.location, range.length));
    __CFDataValidateRange(data, range, __PRETTY_FUNCTION__);
    memmove(buffer, CFDataGetBytePtr(data) + range.location, range.length);
}

/* Allocates new block of data with at least numNewValues more bytes than the current length. If clear is true, the new bytes up to at least the new length with be zeroed. */
static void __CFDataGrow(CFMutableDataRef data, CFIndex numNewValues, Boolean clear) {
    CFIndex oldLength = __CFDataLength(data);
    CFIndex newLength = oldLength + numNewValues;
    if (newLength > CFDATA_MAX_SIZE || newLength < 0) __CFDataHandleOutOfMemory(data, newLength * sizeof(uint8_t));
    CFIndex capacity = __CFDataRoundUpCapacity(newLength);
    CFIndex numBytes = __CFDataNumBytesForCapacity(capacity);
    CFAllocatorRef allocator = CFGetAllocator(data);
    void *bytes = NULL;
    void *oldBytes = data->_bytes;
    Boolean allocateCleared = clear && __CFDataShouldAllocateCleared(data, numBytes);
    if (allocateCleared && !__CFDataUseAllocator(data) && (oldLength == 0 || (newLength / oldLength) > 4)) {
	// If the length that needs to be zeroed is significantly greater than the length of the data, then calloc/memmove is probably more efficient than realloc/memset.
	bytes = __CFDataAllocate(data, numBytes * sizeof(uint8_t), true);
	if (NULL != bytes) {
	    memmove(bytes, oldBytes, oldLength);
	    __CFDataDeallocate(data);
	}
    }
    if (bytes == NULL) {
	// If the calloc/memmove approach either failed or was never attempted, then realloc.
	allocateCleared = false;
	if (__CFDataUseAllocator(data)) {
	    bytes = CFAllocatorReallocate(allocator, oldBytes, numBytes * sizeof(uint8_t), 0);
	} else {
	    bytes = realloc(oldBytes, numBytes * sizeof(uint8_t));
	}
    }
    if (NULL == bytes) __CFDataHandleOutOfMemory(data, numBytes * sizeof(uint8_t));
    __CFDataSetCapacity(data, capacity);
    __CFDataSetNumBytes(data, numBytes);
    if (clear && !allocateCleared && oldLength < newLength) memset((uint8_t *)bytes + oldLength, 0, newLength - oldLength);
    __CFDataSetNeedsToZero(data, !allocateCleared);
    __CFAssignWithWriteBarrier((void **)&data->_bytes, bytes);
    if (__CFOASafe) __CFSetLastAllocationEventName(data->_bytes, "CFData (store)");
}

void CFDataSetLength(CFMutableDataRef data, CFIndex newLength) {
    CFIndex oldLength, capacity;
    Boolean isGrowable;
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSMutableData *)data, setLength:(NSUInteger)newLength);
    CFAssert1(__CFDataIsMutable(data), __kCFLogAssertion, "%s(): data is immutable", __PRETTY_FUNCTION__);
    oldLength = __CFDataLength(data);
    capacity = __CFDataCapacity(data);
    isGrowable = __CFDataIsGrowable(data);
    if (__CFDataIsMutable(data)) {
	if (newLength < 0) {
	    if (isGrowable) {
		__CFDataHandleOutOfMemory(data, newLength);
	    } else {
		HALT;
	    }
	} else if (capacity < newLength) {
	    if (isGrowable) {
		__CFDataGrow(data, newLength - oldLength, true);
	    } else {
		CFAssert1(newLength <= __CFDataCapacity(data), __kCFLogAssertion, "%s(): fixed-capacity data is full", __PRETTY_FUNCTION__);
	    }
	} else if (oldLength < newLength && __CFDataNeedsToZero(data)) {
	    memset(CFDataGetMutableBytePtr(data) + oldLength, 0, newLength - oldLength);
	} else if (newLength < oldLength) {
	    __CFDataSetNeedsToZero(data, true);
	}
    }
    __CFDataSetLength(data, newLength);
    __CFDataSetNumBytesUsed(data, newLength);
}

void CFDataIncreaseLength(CFMutableDataRef data, CFIndex extraLength) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSMutableData *)data, increaseLengthBy:(NSUInteger)extraLength);
    CFAssert1(__CFDataIsMutable(data), __kCFLogAssertion, "%s(): data is immutable", __PRETTY_FUNCTION__);
    if (extraLength < 0) HALT; // Avoid integer overflow.
    CFDataSetLength(data, __CFDataLength(data) + extraLength);
}

void CFDataAppendBytes(CFMutableDataRef data, const uint8_t *bytes, CFIndex length) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSMutableData *)data, appendBytes:(const void *)bytes length:(NSUInteger)length);
    CFAssert1(__CFDataIsMutable(data), __kCFLogAssertion, "%s(): data is immutable", __PRETTY_FUNCTION__);
    CFDataReplaceBytes(data, CFRangeMake(__CFDataLength(data), 0), bytes, length); 
}

void CFDataDeleteBytes(CFMutableDataRef data, CFRange range) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSMutableData *)data, replaceBytesInRange:NSMakeRange(range.location, range.length) withBytes:NULL length:0);
    CFAssert1(__CFDataIsMutable(data), __kCFLogAssertion, "%s(): data is immutable", __PRETTY_FUNCTION__);
    CFDataReplaceBytes(data, range, NULL, 0); 
}

void CFDataReplaceBytes(CFMutableDataRef data, CFRange range, const uint8_t *newBytes, CFIndex newLength) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSMutableData *)data, replaceBytesInRange:NSMakeRange(range.location, range.length) withBytes:(const void *)newBytes length:(NSUInteger)newLength);
    __CFGenericValidateType(data, CFDataGetTypeID());
    __CFDataValidateRange(data, range, __PRETTY_FUNCTION__);
    CFAssert1(__CFDataIsMutable(data), __kCFLogAssertion, "%s(): data is immutable", __PRETTY_FUNCTION__);
    CFAssert2(0 <= newLength, __kCFLogAssertion, "%s(): newLength (%d) cannot be less than zero", __PRETTY_FUNCTION__, newLength);

    CFIndex len = __CFDataLength(data);
    if (len < 0 || range.length < 0 || newLength < 0) HALT;
    CFIndex newCount = len - range.length + newLength;
    if (newCount < 0) HALT;

    uint8_t *bytePtr = (uint8_t *)CFDataGetMutableBytePtr(data);
    uint8_t *srcBuf = (uint8_t *)newBytes;
    switch (__CFMutableVariety(data)) {
    case kCFMutable:
	if (__CFDataNumBytes(data) < newCount) {
            if (bytePtr && newBytes && newBytes < bytePtr + __CFDataCapacity(data) && bytePtr < newBytes + newLength) {
                srcBuf = (uint8_t *)malloc(newLength * sizeof(uint8_t));
                memmove(srcBuf, newBytes, newLength * sizeof(uint8_t));
            }
	    __CFDataGrow(data, newLength - range.length, false);
            bytePtr = (uint8_t *)CFDataGetMutableBytePtr(data);
	}
	break;
    case kCFFixedMutable:
	CFAssert1(newCount <= __CFDataCapacity(data), __kCFLogAssertion, "%s(): fixed-capacity data is full", __PRETTY_FUNCTION__);
	// Continuing after this could cause buffer overruns.
	if (newCount > __CFDataCapacity(data)) HALT;
	break;
    }
    if (newLength != range.length && range.location + range.length < len) {
        memmove(bytePtr + range.location + newLength, bytePtr + range.location + range.length, (len - range.location - range.length) * sizeof(uint8_t));
    }
    if (0 < newLength) {
        memmove(bytePtr + range.location, srcBuf, newLength * sizeof(uint8_t));
    }
    if (srcBuf != newBytes) free(srcBuf);
    __CFDataSetNumBytesUsed(data, newCount);
    __CFDataSetLength(data, newCount);
}

#define REVERSE_BUFFER(type, buf, len) { \
    type tmp; \
    for(int i = 0; i < (len)/2; i++) { \
	tmp = (buf)[i]; \
	(buf)[i] = (buf)[(len) - i - 1]; \
	(buf)[(len) - i - 1] = tmp; \
    } \
}

static void _computeGoodSubstringShift(const uint8_t *needle, int needleLength, unsigned long shift[], unsigned long suff[]) {
    int f, g, i, j;
    
    // Compute suffix lengths
    
    suff[needleLength - 1] = needleLength;
    f = g = needleLength - 1;
    for (i = needleLength - 2; i >= 0; --i) {
        if (i > g && suff[i + needleLength - 1 - f] < i - g)
            suff[i] = suff[i + needleLength - 1 - f];
        else {
            if (i < g)
                g = i;
            f = i;
            while (g >= 0 && needle[g] == needle[g + needleLength - 1 - f])
                --g;
            suff[i] = f - g;
        }
    }
    
    // Compute shift table
    
    for (i = 0; i < needleLength; ++i)
        shift[i] = needleLength;
    j = 0;
    for (i = needleLength - 1; i >= 0; --i)
        if (suff[i] == i + 1)
            for (; j < needleLength - 1 - i; ++j)
                if (shift[j] == needleLength)
                    shift[j] = needleLength - 1 - i;
    // Set the amount of shift necessary to move each of the suffix matches found into a position where it overlaps with the suffix. If there are duplicate matches the latest one is the one that should take effect.
    for (i = 0; i <= needleLength - 2; ++i)
        shift[needleLength - 1 - suff[i]] = needleLength - 1 - i;
    // Since the Boyer-Moore algorithm moves the pointer back while scanning substrings, add the distance to the end of the potential substring.
    for (i = 0; i < needleLength - 1; ++i) {
	shift[i] += (needleLength - 1 - i);
    }
}

static const uint8_t * __CFDataSearchBoyerMoore(const CFDataRef data, const uint8_t *haystack, unsigned long haystackLength, const uint8_t *needle, unsigned long needleLength, Boolean backwards) {
    unsigned long badCharacterShift[UCHAR_MAX + 1] = {0};
    unsigned long *goodSubstringShift = (unsigned long *)malloc(needleLength * sizeof(unsigned long));
    unsigned long *suffixLengths = (unsigned long *)malloc(needleLength * sizeof(unsigned long));
    if (!goodSubstringShift || !suffixLengths) {
	__CFDataHandleOutOfMemory(data, needleLength * sizeof(unsigned long));
    }
    
    if(backwards) {
	for (int i = 0; i < sizeof(badCharacterShift) / sizeof(*badCharacterShift); i++)
	    badCharacterShift[i] = needleLength;
	
	for (int i = needleLength - 1; i >= 0; i--)
	    badCharacterShift[needle[i]] = i;
	
	// To get the correct shift table for backwards search reverse the needle, compute the forwards shift table, and then reverse the result.
	uint8_t *needleCopy = (uint8_t *)malloc(needleLength * sizeof(uint8_t));
	if (!needleCopy) {
	    __CFDataHandleOutOfMemory(data, needleLength * sizeof(uint8_t));
	}
	memmove(needleCopy, needle, needleLength);
	REVERSE_BUFFER(uint8_t, needleCopy, needleLength);
	_computeGoodSubstringShift(needleCopy, needleLength, goodSubstringShift, suffixLengths);
	REVERSE_BUFFER(unsigned long, goodSubstringShift, needleLength);
	free(needleCopy);
    } else {
	for (int i = 0; i < sizeof(badCharacterShift) / sizeof(*badCharacterShift); i++)
	    badCharacterShift[i] = needleLength;
	
	for (int i = 0; i < needleLength; i++)
	    badCharacterShift[needle[i]] = needleLength - i- 1;
	
	_computeGoodSubstringShift(needle, needleLength, goodSubstringShift, suffixLengths);
    }
    
    const uint8_t *scan_needle;
    const uint8_t *scan_haystack;
    const uint8_t *result = NULL;
    if(backwards) {
	const uint8_t *const end_needle = needle + needleLength;
	scan_needle = needle;
	scan_haystack = haystack + haystackLength - needleLength;
	while (scan_haystack >= haystack && scan_needle < end_needle) {
	    if (*scan_haystack == *scan_needle) {
		scan_haystack++;
		scan_needle++;
	    } else {
		scan_haystack -= __CFMax(badCharacterShift[*scan_haystack], goodSubstringShift[scan_needle - needle]);
		scan_needle = needle;
	    }
	}
	if (scan_needle == end_needle) {
	    result = (scan_haystack - needleLength);
	}
    } else {
	const uint8_t *const end_haystack = haystack + haystackLength;
	scan_needle = needle + needleLength - 1;
	scan_haystack = haystack + needleLength - 1;
	while (scan_haystack < end_haystack && scan_needle >= needle) {
	    if (*scan_haystack == *scan_needle) {
		scan_haystack--;
		scan_needle--;
	    } else {
		scan_haystack += __CFMax(badCharacterShift[*scan_haystack], goodSubstringShift[scan_needle - needle]);
		scan_needle = needle + needleLength - 1;
	    }
	}
	if (scan_needle < needle) {
	    result = (scan_haystack + 1);
	}
    }
    
    free(goodSubstringShift);
    free(suffixLengths);
    
    return result;
}

CFRange _CFDataFindBytes(CFDataRef data, CFDataRef dataToFind, CFRange searchRange, CFDataSearchFlags compareOptions) {
    const uint8_t *fullHaystack = CFDataGetBytePtr(data);
    const uint8_t *needle = CFDataGetBytePtr(dataToFind);
    unsigned long fullHaystackLength = CFDataGetLength(data);
    unsigned long needleLength = CFDataGetLength(dataToFind);
    
    if(compareOptions & kCFDataSearchAnchored) {
	if(searchRange.length > needleLength) {
	    if(compareOptions & kCFDataSearchBackwards) {
		searchRange.location += (searchRange.length - needleLength);
	    }
	    searchRange.length = needleLength;
	}
    }
    if(searchRange.length > fullHaystackLength - searchRange.location) {
	searchRange.length = fullHaystackLength - searchRange.location;
    }
    
    if(searchRange.length < needleLength || fullHaystackLength == 0 || needleLength == 0) {
	return CFRangeMake(kCFNotFound, 0);
    }
	
    const uint8_t *haystack = fullHaystack + searchRange.location;
    const uint8_t *searchResult = __CFDataSearchBoyerMoore(data, haystack, searchRange.length, needle, needleLength, (compareOptions & kCFDataSearchBackwards) != 0);
    CFIndex resultLocation = (searchResult == NULL) ? kCFNotFound : searchRange.location + (searchResult - haystack);
    
    return CFRangeMake(resultLocation, resultLocation == kCFNotFound ? 0: needleLength);
}

CFRange CFDataFind(CFDataRef data, CFDataRef dataToFind, CFRange searchRange, CFDataSearchFlags compareOptions) {
    // No objc dispatch
    __CFGenericValidateType(data, CFDataGetTypeID());
    __CFGenericValidateType(dataToFind, CFDataGetTypeID());
    __CFDataValidateRange(data, searchRange, __PRETTY_FUNCTION__);
    
    return _CFDataFindBytes(data, dataToFind, searchRange, compareOptions);
}

#undef __CFDataValidateRange
#undef __CFGenericValidateMutabilityFlags
#undef INLINE_BYTES_THRESHOLD
#undef CFDATA_MAX_SIZE
#undef REVERSE_BUFFER
