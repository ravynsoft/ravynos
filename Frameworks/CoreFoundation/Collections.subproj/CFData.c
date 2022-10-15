/*	CFData.c
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Kevin Perry
*/

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFPriv.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#include <string.h>



#if DEPLOYMENT_RUNTIME_SWIFT
DECLARE_STATIC_CLASS_REF(NSMutableData);
static const void *_NSMutableData = STATIC_CLASS_REF(NSMutableData);
static Boolean _CFDataShouldBridgeToSwift(CFTypeID type, CFDataRef data);

#define CF_SWIFT_NSDATA_FUNCDISPATCHV(type, ret, obj, fn, ...) CF_SWIFT_FUNCDISPATCHV_CHECK(_CFDataShouldBridgeToSwift, type, ret, obj, fn, ## __VA_ARGS__)

#else
#define CF_SWIFT_NSDATA_FUNCDISPATCHV(...)
#endif


#if TARGET_RT_64_BIT
#define CFDATA_MAX_SIZE	    ((1ULL << 42) - 1)
#else
#define CFDATA_MAX_SIZE	    ((1ULL << 31) - 1)
#endif

#if TARGET_OS_MAC
#include <mach/mach.h>
CF_INLINE unsigned long __CFPageSize() { return vm_page_size; }
#elif TARGET_OS_WIN32
CF_INLINE unsigned long __CFPageSize() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwPageSize;
}
#elif TARGET_OS_LINUX || TARGET_OS_BSD
#include <unistd.h>
CF_INLINE unsigned long __CFPageSize() {
    return (unsigned long)getpagesize();
}
#elif TARGET_OS_WASI
CF_INLINE unsigned long __CFPageSize() {
    // WebAssembly linear memory pages are always 64KiB in size
    return 65536;
}
#endif

#define INLINE_BYTES_THRESHOLD ((4 * __CFPageSize()) - sizeof(struct __CFData) - 15)

struct __CFData {
    CFRuntimeBase _base;
    CFIndex _length;	/* number of bytes */
    CFIndex _capacity;	/* maximum number of bytes */
    CFAllocatorRef _bytesDeallocator;	/* used only for immutable; if NULL, no deallocation */
#if DEPLOYMENT_RUNTIME_SWIFT
    void *_deallocHandler; // for swift
#endif
    uint8_t *_bytes;	/* compaction: direct access to _bytes is only valid when data is not inline */
};

#if DEPLOYMENT_RUNTIME_SWIFT
static Boolean _CFDataShouldBridgeToSwift(CFTypeID type, CFDataRef data) {
    return CF_IS_SWIFT(type, data) && data->_base._cfisa != (uintptr_t)_NSMutableData;
}
#endif

/*  
 Bit 0 = is mutable
 Bit 1 = growable
 Bit 2 = bytes inline
 Bit 3 = use given CFAllocator
 Bit 4 = don't deallocate
 
 Bits 1-0 are used for mutability variation
 
 Bit 6 = not all bytes have been zeroed yet (mutable)
 */

enum {
    __kCFMutableMask = 0x01,
    __kCFGrowableMask = 0x02,
};

enum {
    // These bits are within the mutable mask and growable mask
    __kCFMutable = 0,
    __kCFGrowable = 1,
    
    // These bits are stand-alone
    __kCFBytesInline = 2,
    __kCFUseAllocator = 3,
    __kCFDontDeallocate = 4,
    __kCFNeedsZero = 6,
};

typedef enum {
    kCFImmutable = 0x0,		/* unchangable and fixed capacity; default */
    kCFFixedMutable = 0x1,	/* changeable and fixed capacity */
    kCFMutable = 0x3		/* changeable and variable capacity */
} _CFDataMutableVariety;

CF_INLINE Boolean __CFDataIsMutable(CFDataRef data) {
    return __CFRuntimeGetFlag(data, __kCFMutable);
}

CF_INLINE Boolean __CFDataIsGrowable(CFDataRef data) {
    return __CFRuntimeGetFlag(data, __kCFGrowable);
}

CF_INLINE Boolean __CFDataBytesInline(CFDataRef data) {
    return __CFRuntimeGetFlag(data, __kCFBytesInline);
}
CF_INLINE void __CFDataSetInline(CFDataRef data, Boolean flag) {
    __CFRuntimeSetFlag(data, __kCFBytesInline, flag);
}

CF_INLINE Boolean __CFDataUseAllocator(CFDataRef data) {
    return __CFRuntimeGetFlag(data, __kCFUseAllocator);
}
CF_INLINE void __CFDataSetUseAllocator(CFDataRef data, Boolean flag) {
    __CFRuntimeSetFlag(data, __kCFUseAllocator, flag);
}

CF_INLINE Boolean __CFDataDontDeallocate(CFDataRef data) {
    return __CFRuntimeGetFlag(data, __kCFDontDeallocate);
}
CF_INLINE void __CFDataSetDontDeallocate(CFDataRef data, Boolean flag) {
    __CFRuntimeSetFlag(data, __kCFDontDeallocate, flag);
}

CF_INLINE _CFDataMutableVariety __CFMutableVariety(const void *cf) {
    return __CFRuntimeGetValue(cf, 1, 0);
}
CF_INLINE void __CFSetMutableVariety(void *cf, _CFDataMutableVariety v) {
    __CFRuntimeSetValue(cf, 1, 0, v);
}

CF_INLINE Boolean __CFDataNeedsToZero(CFDataRef data) {
    return __CFRuntimeGetFlag(data, __kCFNeedsZero);
}
CF_INLINE void __CFDataSetNeedsToZero(CFDataRef data, Boolean zero) {
    __CFRuntimeSetFlag(data, __kCFNeedsZero, zero);
}

CF_INLINE CFIndex __CFDataLength(CFDataRef data) {
    return data->_length;
}

CF_INLINE void __CFDataSetLength(CFMutableDataRef data, CFIndex v) {
    data->_length = v;
}

CF_INLINE CFIndex __CFDataCapacity(CFDataRef data) {
    return data->_capacity;
}

CF_INLINE void __CFDataSetCapacity(CFMutableDataRef data, CFIndex v) {
    data->_capacity = v;
}

#if TARGET_RT_64_BIT
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
	unsigned long long newCapacity = CHUNK_SIZE * (1+(capacity >> ((long)flsl(CHUNK_SIZE)-1)));
	return __CFMin(newCapacity, CFDATA_MAX_SIZE);
    }
}

__attribute__((cold))
static void __CFDataHandleOutOfMemory(CFTypeRef obj, CFIndex numBytes) CLANG_ANALYZER_NORETURN {
    CFStringRef msg;
    if(0 < numBytes && numBytes <= CFDATA_MAX_SIZE) {
	msg = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("Attempt to allocate %ld bytes for NS/CFData failed"), numBytes);
    } else {
	msg = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("Attempt to allocate %ld bytes for NS/CFData failed. Maximum size: %lld"), numBytes, CFDATA_MAX_SIZE);
    }
    CFLog(kCFLogLevelCritical, CFSTR("%@"), msg);
    HALT;
}

#define FAUX_HALT_MSG(msg) fprintf(stderr, "%s\n", msg)

CF_INLINE void __CFDataValidateRange(CFDataRef data, CFRange range) {
    if (range.location < 0) FAUX_HALT_MSG("range.location out of range (<0)");
    if (range.location > __CFDataLength(data)) FAUX_HALT_MSG("range.location out of range (>len)");
    if (range.length < 0) FAUX_HALT_MSG("length cannot be less than zero");
    if (range.location + range.length > __CFDataLength(data)) FAUX_HALT_MSG("ending index out of bounds");
}

static Boolean __CFDataEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFDataRef data1 = (CFDataRef)cf1;
    CFDataRef data2 = (CFDataRef)cf2;
    CFIndex length;
    length = __CFDataLength(data1);
    if (length != __CFDataLength(data2)) return false;
    const uint8_t *bytePtr1 = _CFDataGetBytePtrNonObjC(data1);
    const uint8_t *bytePtr2 = _CFDataGetBytePtrNonObjC(data2);
    if (bytePtr1 == bytePtr2) return true;
    return 0 == memcmp(bytePtr1, bytePtr2, length);
}

static CFHashCode __CFDataHash(CFTypeRef cf) {
    CFDataRef data = (CFDataRef)cf;
    return CFHashBytes((uint8_t *)_CFDataGetBytePtrNonObjC(data), __CFMin(__CFDataLength(data), 80));
}

static CFStringRef __CFDataCopyDescription(CFTypeRef cf) {
    CFDataRef data = (CFDataRef)cf;
    CFMutableStringRef result;
    CFIndex idx;
    CFIndex len;
    const uint8_t *bytes;
    len = __CFDataLength(data);
    bytes = _CFDataGetBytePtrNonObjC(data);
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
        result = (size > (128 * 1024));
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
        if (clear) {
            bytes = calloc(1, size);
        } else {
            bytes = malloc(size);
        }
    }
    return bytes;
}

static void __CFDataDeallocate(CFTypeRef cf) {
    CFMutableDataRef data = (CFMutableDataRef)cf;
    if (!__CFDataBytesInline(data) && !__CFDataDontDeallocate(data)) {
	CFAllocatorRef deallocator = data->_bytesDeallocator;
	if (deallocator != NULL) {
	    CFAllocatorDeallocate(deallocator, data->_bytes);
	    CFRelease(deallocator);
	    data->_bytes = NULL;
	} else {
	    if (__CFDataUseAllocator(data)) {
		CFAllocatorDeallocate(__CFGetAllocator(data), data->_bytes);
	    } else if (data->_bytes) {
		free(data->_bytes);
	    }
	    data->_bytes = NULL;
	}
    }
}

const CFRuntimeClass __CFDataClass = {
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
    return _kCFRuntimeIDCFData;
}

void _CFDataInit(CFMutableDataRef memory, CFOptionFlags variety, CFIndex capacity, const uint8_t *bytes, CFIndex length, Boolean noCopy) {
    Boolean isMutable = ((variety & __kCFMutableMask) != 0);
    Boolean isGrowable = ((variety & __kCFGrowableMask) != 0);
    Boolean isDontDeallocate = ((variety & __kCFDontDeallocate) != 0);
    
    __CFDataSetLength(memory, 0);
    __CFDataSetDontDeallocate(memory, isDontDeallocate);
    
    if (isMutable && isGrowable) {
        __CFDataSetCapacity(memory, __CFDataRoundUpCapacity(1));
        __CFSetMutableVariety(memory, kCFMutable);
    } else {
        /* Don't round up capacity */
        __CFDataSetCapacity(memory, capacity);
        __CFSetMutableVariety(memory, kCFFixedMutable);
    }
    if (noCopy) {
        memory->_bytes = (uint8_t *)bytes;
        __CFDataSetLength(memory, length);
        // Mutable no-copy datas are not allowed, so don't bother setting needsToZero flag.
    } else {
        Boolean cleared = (isMutable && !isGrowable && !_CFExecutableLinkedOnOrAfter(CFSystemVersionSnowLeopard));
        // assume that allocators give 16-byte aligned memory back -- it is their responsibility
        memory->_bytes = __CFDataAllocate(memory, __CFDataCapacity(memory) * sizeof(uint8_t), cleared);
        if (__CFOASafe) __CFSetLastAllocationEventName(memory->_bytes, "CFData (store)");
        if (NULL == memory->_bytes) {
            return;
        }
        
        __CFDataSetNeedsToZero(memory, !cleared);
        CFDataReplaceBytes(memory, CFRangeMake(0, 0), bytes, length);
    }
    __CFSetMutableVariety(memory, variety);
}


static Boolean __CFDataShouldUseAllocator(CFAllocatorRef allocator) {
    CFAllocatorRef const effectiveAllocator = (allocator == kCFAllocatorDefault) ? __CFGetDefaultAllocator() : allocator;
    return effectiveAllocator != kCFAllocatorSystemDefault && effectiveAllocator != kCFAllocatorMalloc && effectiveAllocator != kCFAllocatorMallocZone;
}

// NULL bytesDeallocator to this function does not mean the default allocator, it means
// that there should be no deallocator, and the bytes should be copied.
static CFMutableDataRef __CFDataInit(CFAllocatorRef allocator, _CFDataMutableVariety variety, CFIndex capacity, const uint8_t *bytes, CFIndex length, CFAllocatorRef bytesDeallocator) CF_RETURNS_RETAINED {
    CFMutableDataRef memory;
    CFAssert2(0 <= capacity, __kCFLogAssertion, "%s(): capacity (%ld) cannot be less than zero", __PRETTY_FUNCTION__, capacity);
    CFAssert3(kCFFixedMutable != variety || length <= capacity, __kCFLogAssertion, "%s(): for kCFFixedMutable type, capacity (%ld) must be greater than or equal to number of initial elements (%ld)", __PRETTY_FUNCTION__, capacity, length);
    CFAssert2(0 <= length, __kCFLogAssertion, "%s(): length (%ld) cannot be less than zero", __PRETTY_FUNCTION__, length);

    Boolean noCopy = bytesDeallocator != NULL;
    Boolean isMutable = ((variety & __kCFMutableMask) != 0);
    Boolean isGrowable = ((variety & __kCFGrowableMask) != 0);
    Boolean allocateInline = !isGrowable && !noCopy && capacity < INLINE_BYTES_THRESHOLD;
    allocator = (allocator == NULL) ? __CFGetDefaultAllocator() : allocator;
    Boolean useAllocator = __CFDataShouldUseAllocator(allocator);
    
    CFIndex size = sizeof(struct __CFData) - sizeof(CFRuntimeBase);
    if (allocateInline) {
	size += sizeof(uint8_t) * capacity + sizeof(uint8_t) * 15;	// for 16-byte alignment fixup
    }
    memory = (CFMutableDataRef)_CFRuntimeCreateInstance(allocator, CFDataGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }
#if DEPLOYMENT_RUNTIME_SWIFT
    memory->_deallocHandler = NULL;
#endif
    __CFDataSetLength(memory, 0);
    __CFDataSetInline(memory, allocateInline);
    __CFDataSetUseAllocator(memory, useAllocator);

    if (isMutable && isGrowable) {
	__CFDataSetCapacity(memory, __CFDataRoundUpCapacity(1));
        __CFSetMutableVariety(memory, kCFMutable);
    } else {
	// Don't round up capacity
	__CFDataSetCapacity(memory, capacity);
        
        // Immutable datas are temporarily set to "fixed mutable" since we pass it to CFDataReplaceBytes() to copy in the initial bytes.
        __CFSetMutableVariety(memory, kCFFixedMutable);
    }

    if (noCopy) {
	memory->_bytes = (uint8_t *)bytes;
	memory->_bytesDeallocator = (CFAllocatorRef)CFRetain(bytesDeallocator);
	__CFDataSetLength(memory, length);
	// Mutable no-copy datas are not allowed, so don't bother setting needsToZero flag.
    } else {
	Boolean cleared = (isMutable && !isGrowable && !_CFExecutableLinkedOnOrAfter(CFSystemVersionSnowLeopard));
	if (!allocateInline) {
	    // assume that allocators give 16-byte aligned memory back -- it is their responsibility
	    memory->_bytes = __CFDataAllocate(memory, __CFDataCapacity(memory) * sizeof(uint8_t), cleared);
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

    // Ensure immutable datas get marked as such now.
    if (!isMutable) {
        __CFSetMutableVariety(memory, kCFImmutable);
    }
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
    Boolean allowRetain = true;
    if (allowRetain) {
        CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), CFDataRef, (NSData *)data, copy);
        CF_SWIFT_NSDATA_FUNCDISPATCHV(CFDataGetTypeID(), CFDataRef, data, NSData.copy);

        // If the data isn't mutable...
        if (!__CFDataIsMutable(data)) {

            // ... and the requested allocator is the same as the data's ...
            CFAllocatorRef const effectiveCopyAllocator = __CFDataShouldUseAllocator(allocator) ? allocator : NULL;
            CFAllocatorRef const effectiveDataAllocator = __CFDataUseAllocator(data) ? __CFGetAllocator(data) : NULL;
            if (effectiveCopyAllocator == effectiveDataAllocator) {
                // ... and the buffer is owned by the CFData.
                if (__CFDataBytesInline(data) || (data->_bytesDeallocator == NULL)) {

                    // Then just retain instead of making a true copy
                    return CFRetain(data);

                }
            }
            
        }
    }
    
    CFIndex length = CFDataGetLength(data);
    return __CFDataInit(allocator, kCFImmutable, length, CFDataGetBytePtr(data), length, NULL);
}

CF_PRIVATE CFMutableDataRef _CFDataCreateFixedMutableWithBuffer(CFAllocatorRef allocator, CFIndex capacity, const uint8_t *bytes, CFAllocatorRef bytesDeallocator) {
    return (CFMutableDataRef)__CFDataInit(allocator, kCFFixedMutable, capacity, bytes, 0, bytesDeallocator);
}

CFMutableDataRef CFDataCreateMutable(CFAllocatorRef allocator, CFIndex capacity) {
    CFMutableDataRef r = (CFMutableDataRef)__CFDataInit(allocator, (0 == capacity) ? kCFMutable : kCFFixedMutable, capacity, NULL, 0, NULL);
    return r;
}

CFMutableDataRef CFDataCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFDataRef data) {
    CFMutableDataRef r = (CFMutableDataRef) __CFDataInit(allocator, (0 == capacity) ? kCFMutable : kCFFixedMutable, capacity, CFDataGetBytePtr(data), CFDataGetLength(data), NULL);
    return r;
}

CFIndex CFDataGetLength(CFDataRef data) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), CFIndex, (NSData *)data, length);
    CF_SWIFT_NSDATA_FUNCDISPATCHV(_kCFRuntimeIDCFData, CFIndex, data, NSData.length);
    __CFGenericValidateType(data, CFDataGetTypeID());
    return __CFDataLength(data);
}

CF_PRIVATE uint8_t *_CFDataGetBytePtrNonObjC(CFDataRef data) {
    __CFGenericValidateType(data, CFDataGetTypeID());
    return __CFDataBytesInline(data) ? (uint8_t *)__CFDataInlineBytesPtr(data) : data->_bytes;
}

const uint8_t *CFDataGetBytePtr(CFDataRef data) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), const uint8_t *, (NSData *)data, bytes);
    CF_SWIFT_NSDATA_FUNCDISPATCHV(_kCFRuntimeIDCFData, const uint8_t *, data, NSData.bytes);
    return _CFDataGetBytePtrNonObjC(data);
}

uint8_t *CFDataGetMutableBytePtr(CFMutableDataRef data) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), uint8_t *, (NSMutableData *)data, mutableBytes);
    CF_SWIFT_NSDATA_FUNCDISPATCHV(_kCFRuntimeIDCFData, uint8_t *, data, NSData.mutableBytes);
    CFAssert1(__CFDataIsMutable(data), __kCFLogAssertion, "%s(): data is immutable", __PRETTY_FUNCTION__);
    return _CFDataGetBytePtrNonObjC(data);
}

void CFDataGetBytes(CFDataRef data, CFRange range, uint8_t *buffer) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSData *)data, getBytes:(void *)buffer range:NSMakeRange(range.location, range.length));
    CF_SWIFT_NSDATA_FUNCDISPATCHV(_kCFRuntimeIDCFData, void, data, NSData.getBytes, range, buffer);
    __CFDataValidateRange(data, range);
    memmove(buffer, _CFDataGetBytePtrNonObjC(data) + range.location, range.length);
}

/* Allocates new block of data with at least numNewValues more bytes than the current length. If clear is true, the new bytes up to at least the new length with be zeroed. */
static void __CFDataGrow(CFMutableDataRef data, CFIndex numNewValues, Boolean clear) {
    CFIndex oldLength = __CFDataLength(data);
    CFIndex newLength = oldLength + numNewValues;
    if (newLength > CFDATA_MAX_SIZE || newLength < 0) __CFDataHandleOutOfMemory(data, newLength * sizeof(uint8_t));
    CFIndex capacity = __CFDataRoundUpCapacity(newLength);
    CFAllocatorRef allocator = CFGetAllocator(data);
    void *bytes = NULL;
    void *oldBytes = data->_bytes;
    Boolean allocateCleared = clear && __CFDataShouldAllocateCleared(data, capacity);
    if (allocateCleared && !__CFDataUseAllocator(data) && (oldLength == 0 || (newLength / oldLength) > 4)) {
	// If the length that needs to be zeroed is significantly greater than the length of the data, then calloc/memmove is probably more efficient than realloc/memset.
	bytes = __CFDataAllocate(data, capacity * sizeof(uint8_t), true);
	if (NULL != bytes) {
	    memmove(bytes, oldBytes, oldLength);
	    __CFDataDeallocate(data);
	}
    }
    if (bytes == NULL) {
	// If the calloc/memmove approach either failed or was never attempted, then realloc.
	allocateCleared = false;
	if (__CFDataUseAllocator(data)) {
	    bytes = __CFSafelyReallocateWithAllocator(allocator, oldBytes, capacity * sizeof(uint8_t), 0, NULL);
        } else {
	    bytes = __CFSafelyReallocate(oldBytes, capacity * sizeof(uint8_t), NULL);
	}
    }
    if (NULL == bytes) __CFDataHandleOutOfMemory(data, capacity * sizeof(uint8_t));
    if (clear && !allocateCleared && oldLength < newLength) {
        memset((uint8_t *)bytes + oldLength, 0, newLength - oldLength);
    }
    __CFDataSetCapacity(data, capacity);
    __CFDataSetNeedsToZero(data, !allocateCleared);
    data->_bytes = bytes;
    if (__CFOASafe) __CFSetLastAllocationEventName(data->_bytes, "CFData (store)");
}

void CFDataSetLength(CFMutableDataRef data, CFIndex newLength) {
    CFIndex oldLength, capacity;
    Boolean isGrowable;
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSMutableData *)data, setLength:(NSUInteger)newLength);
    CF_SWIFT_NSDATA_FUNCDISPATCHV(_kCFRuntimeIDCFData, void, data, NSData.setLength, newLength);
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
                // Continuing after this could cause buffer overruns.
                HALT_MSG("fixed-capacity CFMutableData is full");
	    }
	} else if (oldLength < newLength && __CFDataNeedsToZero(data)) {
	    memset(_CFDataGetBytePtrNonObjC(data) + oldLength, 0, newLength - oldLength);
	} else if (newLength < oldLength) {
	    __CFDataSetNeedsToZero(data, true);
	}
    }
    __CFDataSetLength(data, newLength);
}

void CFDataIncreaseLength(CFMutableDataRef data, CFIndex extraLength) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSMutableData *)data, increaseLengthBy:(NSUInteger)extraLength);
    CF_SWIFT_NSDATA_FUNCDISPATCHV(_kCFRuntimeIDCFData, void, data, NSData.increaseLengthBy, extraLength);
    CFAssert1(__CFDataIsMutable(data), __kCFLogAssertion, "%s(): data is immutable", __PRETTY_FUNCTION__);
    if (extraLength < 0) HALT; // Avoid integer overflow.
    CFDataSetLength(data, __CFDataLength(data) + extraLength);
}

void CFDataAppendBytes(CFMutableDataRef data, const uint8_t *bytes, CFIndex length) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSMutableData *)data, appendBytes:(const void *)bytes length:(NSUInteger)length);
    CF_SWIFT_NSDATA_FUNCDISPATCHV(_kCFRuntimeIDCFData, void, data, NSData.appendBytes, bytes, length);
    CFAssert1(__CFDataIsMutable(data), __kCFLogAssertion, "%s(): data is immutable", __PRETTY_FUNCTION__);
    CFDataReplaceBytes(data, CFRangeMake(__CFDataLength(data), 0), bytes, length); 
}

void CFDataDeleteBytes(CFMutableDataRef data, CFRange range) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSMutableData *)data, replaceBytesInRange:NSMakeRange(range.location, range.length) withBytes:NULL length:0);
    CF_SWIFT_NSDATA_FUNCDISPATCHV(_kCFRuntimeIDCFData, void, data, NSData.replaceBytes, range, NULL, 0);
    CFAssert1(__CFDataIsMutable(data), __kCFLogAssertion, "%s(): data is immutable", __PRETTY_FUNCTION__);
    CFDataReplaceBytes(data, range, NULL, 0); 
}

void CFDataReplaceBytes(CFMutableDataRef data, CFRange range, const uint8_t *newBytes, CFIndex newBytesLength) {
    CF_OBJC_FUNCDISPATCHV(CFDataGetTypeID(), void, (NSMutableData *)data, replaceBytesInRange:NSMakeRange(range.location, range.length) withBytes:(const void *)newBytes length:(NSUInteger)newBytesLength);
    CF_SWIFT_NSDATA_FUNCDISPATCHV(_kCFRuntimeIDCFData, void, data, NSData.replaceBytes, range, newBytes, newBytesLength);
    __CFGenericValidateType(data, CFDataGetTypeID());
    __CFDataValidateRange(data, range);
    CFAssert1(__CFDataIsMutable(data), __kCFLogAssertion, "%s(): data is immutable", __PRETTY_FUNCTION__);
    CFAssert2(0 <= newBytesLength, __kCFLogAssertion, "%s(): newLength (%ld) cannot be less than zero", __PRETTY_FUNCTION__, newBytesLength);

    CFIndex const originalCapacity = __CFDataCapacity(data);
    CFIndex const originalLength = __CFDataLength(data);
    if (range.length < 0) HALT_MSG("Negative range.length passed to CFDataReplaceBytes");
    if (newBytesLength < 0) HALT_MSG("Negative buffer length passed to CFDataReplaceBytes");
    CFIndex const newLength = originalLength - range.length + newBytesLength;
    if (newLength < 0) HALT_MSG("Invalid range passed to CFDataReplaceBytes");
    if (newBytesLength > 0 && newBytes == NULL) HALT_MSG("Invalid length passed to CFDataReplaceBytes when newBytes == NULL");

    uint8_t * dstBuf = _CFDataGetBytePtrNonObjC(data);
    uint8_t * srcBuf = (uint8_t *)newBytes;
    switch (__CFMutableVariety(data)) {
        case kCFMutable:
            if (originalCapacity < newLength) {

                // We need to grow the CFData, but if the buffer we're inserting overlaps with the current buffer, we need to copy it out so it can survive a realloc, which may change the pointers, making newBytes invalid.
                Boolean const buffersOverlap = dstBuf && srcBuf && (srcBuf < dstBuf + originalCapacity) && (srcBuf < newBytes + newBytesLength);
                if (buffersOverlap) {
                    size_t const allocationSize = newBytesLength * sizeof(uint8_t);
                    srcBuf = (uint8_t *)malloc(allocationSize);
                    if (srcBuf == NULL) {
                        __CFDataHandleOutOfMemory(data, allocationSize);
                    }
                    memmove(srcBuf, newBytes, allocationSize);
                }

                __CFDataGrow(data, newLength - originalLength, false);
                dstBuf = _CFDataGetBytePtrNonObjC(data);
            }
            break;
        case kCFFixedMutable:
            CFAssert1(newLength <= originalCapacity, __kCFLogAssertion, "%s(): fixed-capacity data is full", __PRETTY_FUNCTION__);
            // Continuing after this could cause buffer overruns.
            if (newLength > originalCapacity) HALT_MSG("fixed-capacity CFMutableData is full");
            break;
    }

    Boolean const hasBytesToInsert = srcBuf != NULL && newBytesLength > 0;
    CFIndex const distanceToShift = newBytesLength - range.length;
    CFIndex const amountToShift = originalLength - (range.location + range.length);
    Boolean const hasBytesToShift = distanceToShift != 0 && amountToShift > 0;
    if (hasBytesToShift) {
        uint8_t * const shiftSrc = dstBuf + range.location + range.length;
        uint8_t * const shiftDst = shiftSrc + distanceToShift;
        memmove(shiftDst, shiftSrc, amountToShift * sizeof(uint8_t));
    }

    if (hasBytesToInsert) {
        memmove(dstBuf + range.location, srcBuf, newBytesLength * sizeof(uint8_t));
    }
    if (srcBuf != newBytes) free(srcBuf);
    __CFDataSetLength(data, newLength);
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

#define new_ulong_array(N, C) \
    size_t N ## _count__ = (C); \
    if (N ## _count__ > LONG_MAX / sizeof(unsigned long)) { \
        __CFDataHandleOutOfMemory(data, (N ## _count__) * sizeof(unsigned long)); \
    } \
    Boolean N ## _is_stack__ = (N ## _count__ <= 256); \
    if (N ## _count__ == 0) N ## _count__ = 1; \
    STACK_BUFFER_DECL(unsigned long, N ## _buffer__, N ## _is_stack__ ? N ## _count__ : 1); \
    unsigned long * N = N ## _is_stack__ ? N ## _buffer__ : (unsigned long *)malloc((N ## _count__) * sizeof(unsigned long)); \
    if (! N) { \
        __CFDataHandleOutOfMemory(data, (N ## _count__) * sizeof(unsigned long)); \
    } \
    do {} while (0)

#define free_ulong_array(N) \
    if (! N ## _is_stack__) { \
        free(N); \
    } \
    do {} while (0)

static const uint8_t * __CFDataSearchBoyerMoore(const CFDataRef data, const uint8_t *haystack, unsigned long haystackLength, const uint8_t *needle, unsigned long needleLength, Boolean backwards) {
    unsigned long badCharacterShift[UCHAR_MAX + 1] = {0};
    new_ulong_array(goodSubstringShift, needleLength);
    new_ulong_array(suffixLengths, needleLength);
    
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
    
    free_ulong_array(goodSubstringShift);
    free_ulong_array(suffixLengths);
    
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
    __CFDataValidateRange(data, searchRange);
    
    return _CFDataFindBytes(data, dataToFind, searchRange, compareOptions);
}

#undef INLINE_BYTES_THRESHOLD
#undef CFDATA_MAX_SIZE
#undef REVERSE_BUFFER
