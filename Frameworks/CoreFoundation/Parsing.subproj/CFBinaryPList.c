/*	CFBinaryPList.c
	Copyright (c) 2000-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Tony Parker
*/


#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFSet.h>
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFByteOrder.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFUUID.h>
#include <CoreFoundation/CFNumber_Private.h>
#include "CFBasicHash.h"
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#include "CFPropertyList_Internal.h"

#if !TARGET_OS_WASI
#include <CoreFoundation/CFStream.h>
#endif

enum {
	CF_NO_ERROR = 0,
	CF_OVERFLOW_ERROR = (1 << 0),
};

CF_INLINE uint64_t __check_uint64_add_unsigned_unsigned(uint64_t x, uint64_t y, int32_t* err) {
   if((ULLONG_MAX - y) < x)
        *err = *err | CF_OVERFLOW_ERROR;
   return x + y;
};

CF_INLINE uint64_t __check_uint64_mul_unsigned_unsigned(uint64_t x, uint64_t y, int32_t* err) {
  if(x == 0) return 0;
  if(ULLONG_MAX/x < y)
     *err = *err | CF_OVERFLOW_ERROR;
  return x * y;
};

#if TARGET_RT_64_BIT
#define check_ptr_add(p, a, err)	(const uint8_t *)__check_uint64_add_unsigned_unsigned((uintptr_t)p, (uintptr_t)a, err)
#define check_size_t_mul(b, a, err)	(size_t)__check_uint64_mul_unsigned_unsigned((size_t)b, (size_t)a, err)
#else

CF_INLINE uint32_t __check_uint32_add_unsigned_unsigned(uint32_t x, uint32_t y, int32_t* err) {
    if((UINT_MAX - y) < x)
    *err = *err | CF_OVERFLOW_ERROR;
    return x + y;
};

CF_INLINE uint32_t __check_uint32_mul_unsigned_unsigned(uint32_t x, uint32_t y, int32_t* err) {
    uint64_t tmp = (uint64_t) x * (uint64_t) y;
    /* If any of the upper 32 bits touched, overflow */
    if(tmp & 0xffffffff00000000ULL)
    *err = *err | CF_OVERFLOW_ERROR;
    return (uint32_t) tmp;
};

#define check_ptr_add(p, a, err)	(const uint8_t *)__check_uint32_add_unsigned_unsigned((uintptr_t)p, (uintptr_t)a, err)
#define check_size_t_mul(b, a, err)	(size_t)__check_uint32_mul_unsigned_unsigned((size_t)b, (size_t)a, err)
#endif

CF_INLINE uint64_t _CFBinaryPlistTrailer_objectsRangeEnd(const CFBinaryPlistTrailer *trailer) {
    return trailer->_offsetTableOffset - 1;
}

#pragma mark -
#pragma mark Keyed Archiver UID

struct __CFKeyedArchiverUID {
    CFRuntimeBase _base;
    uint32_t _value;
};

static CFStringRef __CFKeyedArchiverUIDCopyDescription(CFTypeRef cf) {
    CFKeyedArchiverUIDRef uid = (CFKeyedArchiverUIDRef)cf;
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFKeyedArchiverUID %p [%p]>{value = %u}"), cf, CFGetAllocator(cf), uid->_value);
}

static CFStringRef __CFKeyedArchiverUIDCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions) {
    CFKeyedArchiverUIDRef uid = (CFKeyedArchiverUIDRef)cf;
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("@%u@"), uid->_value);
}

const CFRuntimeClass __CFKeyedArchiverUIDClass = {
    0,
    "CFKeyedArchiverUID",
    NULL,	// init
    NULL,	// copy
    NULL,	// finalize
    NULL,	// equal -- pointer equality only
    NULL,	// hash -- pointer hashing only
    __CFKeyedArchiverUIDCopyFormattingDescription,
    __CFKeyedArchiverUIDCopyDescription
};

CFTypeID _CFKeyedArchiverUIDGetTypeID(void) {
    return _kCFRuntimeIDCFKeyedArchiverUID;
}

CFKeyedArchiverUIDRef _CFKeyedArchiverUIDCreate(CFAllocatorRef allocator, uint32_t value) {
    CFKeyedArchiverUIDRef uid;
    uid = (CFKeyedArchiverUIDRef)_CFRuntimeCreateInstance(allocator, _CFKeyedArchiverUIDGetTypeID(), sizeof(struct __CFKeyedArchiverUID) - sizeof(CFRuntimeBase), NULL);
    if (NULL == uid) {
	return NULL;
    }
    ((struct __CFKeyedArchiverUID *)uid)->_value = value;
    return uid;
}


uint32_t _CFKeyedArchiverUIDGetValue(CFKeyedArchiverUIDRef uid) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFKeyedArchiverUID, uid);
    return uid->_value;
}

#pragma mark -
#pragma mark Writing

CF_PRIVATE CFErrorRef __CFPropertyListCreateError(CFIndex code, CFStringRef debugString, ...);

#if !TARGET_OS_WASI
typedef struct {
    CFTypeRef stream;
    void *databytes;
    uint64_t datalen;
    CFErrorRef error;
    uint64_t written;
    int32_t used;
    bool streamIsData;
    uint8_t buffer[8192 - 32];
} __CFBinaryPlistWriteBuffer;

static void writeBytes(__CFBinaryPlistWriteBuffer *buf, const UInt8 *bytes, CFIndex length, Boolean dryRun) {
    if (length <= 0) return;
    if (buf->error) return;
    if (buf->databytes) {
        int32_t err = CF_NO_ERROR;
        uint64_t tmpSum = __check_uint64_add_unsigned_unsigned(buf->written, (uint64_t)length, &err);
        if ((CF_NO_ERROR != err) || buf->datalen < tmpSum) {
            buf->error = __CFPropertyListCreateError(kCFPropertyListWriteStreamError, CFSTR("Binary property list writing could not be completed because databytes is full."));
            return;
        }
        if (!dryRun) memmove((char *)buf->databytes + buf->written, bytes, length);
    }
    if (buf->streamIsData) {
        if (buf->stream && !dryRun) CFDataAppendBytes((CFMutableDataRef)buf->stream, bytes, length);
        buf->written += length;
    } else {
	while (0 < length) {
	    CFIndex ret = (buf->stream && !dryRun) ? CFWriteStreamWrite((CFWriteStreamRef)buf->stream, bytes, length) : length;
            if (ret == 0) {
		buf->error = __CFPropertyListCreateError(kCFPropertyListWriteStreamError, CFSTR("Binary property list writing could not be completed because stream is full."));
                return;
            }
            if (ret < 0) {
                CFErrorRef err = buf->stream ? CFWriteStreamCopyError((CFWriteStreamRef)buf->stream) : NULL;
                buf->error = err ? err : __CFPropertyListCreateError(kCFPropertyListWriteStreamError, CFSTR("Binary property list writing could not be completed because the stream had an unknown error."));
                return;
            }
	    buf->written += ret;
	    length -= ret;
	    bytes += ret;
	}
    }
}

static void bufferFlush(__CFBinaryPlistWriteBuffer *buf, Boolean dryRun) {
    writeBytes(buf, buf->buffer, buf->used, dryRun);
    buf->used = 0;
}

static void bufferWrite(__CFBinaryPlistWriteBuffer *buf, const uint8_t *buffer, CFIndex count, Boolean dryRun) {
    if (0 == count) return;
    if ((CFIndex)sizeof(buf->buffer) <= count) {
	bufferFlush(buf, dryRun);
	writeBytes(buf, buffer, count, dryRun);
	return;
    }
    CFIndex copyLen = __CFMin(count, (CFIndex)sizeof(buf->buffer) - buf->used);
    if (!dryRun && (buf->stream || buf->databytes)) {
        switch (copyLen) {
        case 4: buf->buffer[buf->used + 3] = buffer[3]; /* FALLTHROUGH */
        case 3: buf->buffer[buf->used + 2] = buffer[2]; /* FALLTHROUGH */
        case 2: buf->buffer[buf->used + 1] = buffer[1]; /* FALLTHROUGH */
        case 1: buf->buffer[buf->used] = buffer[0]; break;
        default: memmove(buf->buffer + buf->used, buffer, copyLen);
        }
    }
    buf->used += copyLen;
    if (sizeof(buf->buffer) == buf->used) {
	writeBytes(buf, buf->buffer, sizeof(buf->buffer), dryRun);
        if (!dryRun && (buf->stream || buf->databytes)) {
            memmove(buf->buffer, buffer + copyLen, count - copyLen);
        }
	buf->used = count - copyLen;
    }
}

/*
HEADER
	magic number ("bplist")
	file format version (currently "0?")

OBJECT TABLE
	variable-sized objects

	Object Formats (marker byte followed by additional info in some cases)
	null	0000 0000			// null object [v"1?"+ only]
	bool	0000 1000			// false
	bool	0000 1001			// true
	url	0000 1100	string		// URL with no base URL, recursive encoding of URL string [v"1?"+ only]
	url	0000 1101	base string	// URL with base URL, recursive encoding of base URL, then recursive encoding of URL string [v"1?"+ only]
	uuid	0000 1110			// 16-byte UUID [v"1?"+ only]
	fill	0000 1111			// fill byte
	int	0001 0nnn	...		// # of bytes is 2^nnn, big-endian bytes
	real	0010 0nnn	...		// # of bytes is 2^nnn, big-endian bytes
	date	0011 0011	...		// 8 byte float follows, big-endian bytes
	data	0100 nnnn	[int]	...	// nnnn is number of bytes unless 1111 then int count follows, followed by bytes
	string	0101 nnnn	[int]	...	// ASCII string, nnnn is # of chars, else 1111 then int count, then bytes
	string	0110 nnnn	[int]	...	// Unicode string, nnnn is # of chars, else 1111 then int count, then big-endian 2-byte uint16_t
	string	0111 nnnn	[int]	...	// UTF8 string, nnnn is # of chars, else 1111 then int count, then bytes [v"1?"+ only]
	uid	1000 nnnn	...		// nnnn+1 is # of bytes
		1001 xxxx			// unused
	array	1010 nnnn	[int]	objref*	// nnnn is count, unless '1111', then int count follows
	ordset	1011 nnnn	[int]	objref* // nnnn is count, unless '1111', then int count follows [v"1?"+ only]
	set	1100 nnnn	[int]	objref* // nnnn is count, unless '1111', then int count follows [v"1?"+ only]
	dict	1101 nnnn	[int]	keyref* objref*	// nnnn is count, unless '1111', then int count follows
		1110 xxxx			// unused
		1111 xxxx			// unused

OFFSET TABLE
	list of ints, byte size of which is given in trailer
	-- these are the byte offsets into the file
	-- number of these is in the trailer

TRAILER
	byte size of offset ints in offset table
	byte size of object refs in arrays and dicts
	number of offsets in offset table (also is number of objects)
	element # in offset table which is top level object
	offset table offset

*/


static void _appendInt(__CFBinaryPlistWriteBuffer *buf, uint64_t bigint, Boolean dryRun) {
    uint8_t marker;
    uint8_t *bytes;
    CFIndex nbytes;
    if (bigint <= (uint64_t)0xff) {
	nbytes = 1;
	marker = kCFBinaryPlistMarkerInt | 0;
    } else if (bigint <= (uint64_t)0xffff) {
	nbytes = 2;
	marker = kCFBinaryPlistMarkerInt | 1;
    } else if (bigint <= (uint64_t)0xffffffff) {
	nbytes = 4;
	marker = kCFBinaryPlistMarkerInt | 2;
    } else {
	nbytes = 8;
	marker = kCFBinaryPlistMarkerInt | 3;
    }
    bigint = CFSwapInt64HostToBig(bigint);
    bytes = (uint8_t *)&bigint + sizeof(bigint) - nbytes;
    bufferWrite(buf, &marker, 1, dryRun);
    bufferWrite(buf, bytes, nbytes, dryRun);
}

static void _appendUID(__CFBinaryPlistWriteBuffer *buf, CFKeyedArchiverUIDRef uid, Boolean dryRun) {
    uint8_t marker;
    uint8_t *bytes;
    CFIndex nbytes;
    uint64_t bigint = _CFKeyedArchiverUIDGetValue(uid);
    if (bigint <= (uint64_t)0xff) {
	nbytes = 1;
    } else if (bigint <= (uint64_t)0xffff) {
	nbytes = 2;
    } else if (bigint <= (uint64_t)0xffffffff) {
	nbytes = 4;
    } else {
	nbytes = 8;
    }
    marker = kCFBinaryPlistMarkerUID | (uint8_t)(nbytes - 1);
    bigint = CFSwapInt64HostToBig(bigint);
    bytes = (uint8_t *)&bigint + sizeof(bigint) - nbytes;
    bufferWrite(buf, &marker, 1, dryRun);
    bufferWrite(buf, bytes, nbytes, dryRun);
}

static void _appendString(__CFBinaryPlistWriteBuffer *buf, CFStringRef str, Boolean dryRun) {
    CFIndex ret, count = CFStringGetLength(str);
    CFIndex needed, idx2;
    uint8_t *bytes, buffer[1024];
    bytes = (count <= 1024) ? buffer : (uint8_t *)CFAllocatorAllocate(kCFAllocatorSystemDefault, count, 0);
    // presumption, believed to be true, is that ASCII encoding may need
    // less bytes, but will not need greater, than the # of unichars
    ret = CFStringGetBytes(str, CFRangeMake(0, count), kCFStringEncodingASCII, 0, false, bytes, count, &needed);
    if (ret == count) {
        uint8_t marker = (uint8_t)(kCFBinaryPlistMarkerASCIIString | (needed < 15 ? needed : 0xf));
        bufferWrite(buf, &marker, 1, dryRun);
        if (15 <= needed) {
	    _appendInt(buf, (uint64_t)needed, dryRun);
        }
        bufferWrite(buf, bytes, needed, dryRun);
    } else {
        UniChar *chars;
        uint8_t marker = (uint8_t)(kCFBinaryPlistMarkerUnicode16String | (count < 15 ? count : 0xf));
        bufferWrite(buf, &marker, 1, dryRun);
        if (15 <= count) {
	    _appendInt(buf, (uint64_t)count, dryRun);
        }
        chars = (UniChar *)CFAllocatorAllocate(kCFAllocatorSystemDefault, count * sizeof(UniChar), 0);
        CFStringGetCharacters(str, CFRangeMake(0, count), chars);
        for (idx2 = 0; idx2 < count; idx2++) {
	    chars[idx2] = CFSwapInt16HostToBig(chars[idx2]);
        }
        bufferWrite(buf, (uint8_t *)chars, count * sizeof(UniChar), dryRun);
        CFAllocatorDeallocate(kCFAllocatorSystemDefault, chars);
    }
    if (bytes != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, bytes);
}

static void _appendNumber(__CFBinaryPlistWriteBuffer *buf, CFNumberRef num, Boolean dryRun) {
    uint8_t marker;
    uint64_t bigint;
    uint8_t *bytes;
    CFIndex nbytes;
    if (CFNumberIsFloatType(num)) {
        CFSwappedFloat64 swapped64;
        CFSwappedFloat32 swapped32;
        if (CFNumberGetByteSize(num) <= (CFIndex)sizeof(float)) {
	    float v;
	    CFNumberGetValue(num, kCFNumberFloat32Type, &v);
	    swapped32 = CFConvertFloat32HostToSwapped(v);
	    bytes = (uint8_t *)&swapped32;
	    nbytes = sizeof(float);
	    marker = kCFBinaryPlistMarkerReal | 2;
        } else {
	    double v;
	    CFNumberGetValue(num, kCFNumberFloat64Type, &v);
	    swapped64 = CFConvertFloat64HostToSwapped(v);
	    bytes = (uint8_t *)&swapped64;
	    nbytes = sizeof(double);
	    marker = kCFBinaryPlistMarkerReal | 3;
        }
        bufferWrite(buf, &marker, 1, dryRun);
        bufferWrite(buf, bytes, nbytes, dryRun);
    } else {
        CFNumberType type = _CFNumberGetType2(num);
        if (kCFNumberSInt128Type == type) {
	    CFSInt128Struct s;
	    CFNumberGetValue(num, kCFNumberSInt128Type, &s);
	    struct {
        	int64_t high;
        	uint64_t low;
	    } storage;
	    storage.high = CFSwapInt64HostToBig(s.high);
	    storage.low = CFSwapInt64HostToBig(s.low);
	    uint8_t *bytes = (uint8_t *)&storage;
	    uint8_t marker = kCFBinaryPlistMarkerInt | 4;
	    CFIndex nbytes = 16;
	    bufferWrite(buf, &marker, 1, dryRun);
	    bufferWrite(buf, bytes, nbytes, dryRun);
        } else {
	    CFNumberGetValue(num, kCFNumberSInt64Type, &bigint);
	    _appendInt(buf, bigint, dryRun);
        }
    }
}

static Boolean _appendObject(__CFBinaryPlistWriteBuffer *buf, CFTypeRef obj, CFDictionaryRef objtable, uint32_t objRefSize, Boolean dryRun) {
    uint64_t refnum;
    CFIndex idx2;
    CFTypeID type = CFGetTypeID(obj);
	if (_kCFRuntimeIDCFString == type) {
	    _appendString(buf, (CFStringRef)obj, dryRun);
	} else if (_kCFRuntimeIDCFNumber == type) {
	    _appendNumber(buf, (CFNumberRef)obj, dryRun);
	} else if (_kCFRuntimeIDCFBoolean == type) {
	    uint8_t marker = CFBooleanGetValue((CFBooleanRef)obj) ? kCFBinaryPlistMarkerTrue : kCFBinaryPlistMarkerFalse;
	    bufferWrite(buf, &marker, 1, dryRun);
	} else if (_kCFRuntimeIDCFData == type) {
	    CFIndex count = CFDataGetLength((CFDataRef)obj);
	    uint8_t marker = (uint8_t)(kCFBinaryPlistMarkerData | (count < 15 ? count : 0xf));
	    bufferWrite(buf, &marker, 1, dryRun);
	    if (15 <= count) {
		_appendInt(buf, (uint64_t)count, dryRun);
	    }
	    bufferWrite(buf, CFDataGetBytePtr((CFDataRef)obj), count, dryRun);
	} else if (_kCFRuntimeIDCFDate == type) {
	    CFSwappedFloat64 swapped;
	    uint8_t marker = kCFBinaryPlistMarkerDate;
	    bufferWrite(buf, &marker, 1, dryRun);
	    swapped = CFConvertFloat64HostToSwapped(CFDateGetAbsoluteTime((CFDateRef)obj));
	    bufferWrite(buf, (uint8_t *)&swapped, sizeof(swapped), dryRun);
	} else if (_kCFRuntimeIDCFDictionary == type) {
            CFIndex count = CFDictionaryGetCount((CFDictionaryRef)obj);
            uint8_t marker = (uint8_t)(kCFBinaryPlistMarkerDict | (count < 15 ? count : 0xf));
            bufferWrite(buf, &marker, 1, dryRun);
            if (15 <= count) {
                _appendInt(buf, (uint64_t)count, dryRun);
            }
            CFPropertyListRef *list, buffer[512];
            list = (count <= 256) ? buffer : (CFPropertyListRef *)CFAllocatorAllocate(kCFAllocatorSystemDefault, 2 * count * sizeof(CFTypeRef), 0);
            CFDictionaryGetKeysAndValues((CFDictionaryRef)obj, list, list + count);
            for (idx2 = 0; idx2 < 2 * count; idx2++) {
		CFPropertyListRef value = list[idx2];
		if (objtable) {
		    uint32_t swapped = 0;
		    uint8_t *source = (uint8_t *)&swapped;
		    refnum = (uint32_t)(uintptr_t)CFDictionaryGetValue(objtable, value);
		    swapped = CFSwapInt32HostToBig((uint32_t)refnum);
		    bufferWrite(buf, source + sizeof(swapped) - objRefSize, objRefSize, dryRun);
		} else {
		    Boolean ret = _appendObject(buf, value, objtable, objRefSize, dryRun);
		    if (!ret) {
			if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
			return false;
		    }
		}
            }
            if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
	} else if (_kCFRuntimeIDCFArray == type) {
	    CFIndex count = CFArrayGetCount((CFArrayRef)obj);
	    CFPropertyListRef *list, buffer[256];
	    uint8_t marker = (uint8_t)(kCFBinaryPlistMarkerArray | (count < 15 ? count : 0xf));
	    bufferWrite(buf, &marker, 1, dryRun);
	    if (15 <= count) {
		_appendInt(buf, (uint64_t)count, dryRun);
	    }
	    list = (count <= 256) ? buffer : (CFPropertyListRef *)CFAllocatorAllocate(kCFAllocatorSystemDefault, count * sizeof(CFTypeRef), 0);
	    CFArrayGetValues((CFArrayRef)obj, CFRangeMake(0, count), list);
	    for (idx2 = 0; idx2 < count; idx2++) {
		CFPropertyListRef value = list[idx2];
		if (objtable) {
		    uint32_t swapped = 0;
		    uint8_t *source = (uint8_t *)&swapped;
		    refnum = (uint32_t)(uintptr_t)CFDictionaryGetValue(objtable, value);
		    swapped = CFSwapInt32HostToBig((uint32_t)refnum);
		    bufferWrite(buf, source + sizeof(swapped) - objRefSize, objRefSize, dryRun);
		} else {
		    Boolean ret = _appendObject(buf, value, objtable, objRefSize, dryRun);
		    if (!ret) {
			if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
			return false;
		    }
		}
	    }
	    if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
	} else if (_CFKeyedArchiverUIDGetTypeID() == type) {
	    _appendUID(buf, (CFKeyedArchiverUIDRef)obj, dryRun);
	} else {
	    return false;
	}
    return true;
}

static void _flattenPlist(CFPropertyListRef plist, CFMutableArrayRef objlist, CFMutableDictionaryRef objtable, CFMutableSetRef uniquingset) {
    uint32_t refnum;
    CFTypeID type = CFGetTypeID(plist);

    // Do not unique dictionaries or arrays, because: they
    // are slow to compare, and have poor hash codes.
    // Uniquing bools is unnecessary.
    if (_kCFRuntimeIDCFString == type || _kCFRuntimeIDCFNumber == type || _kCFRuntimeIDCFDate == type || _kCFRuntimeIDCFData == type) {
	CFIndex before = CFSetGetCount(uniquingset);
	CFSetAddValue(uniquingset, plist);
	CFIndex after = CFSetGetCount(uniquingset);
	if (after == before) {	// already in set
	    CFPropertyListRef unique = CFSetGetValue(uniquingset, plist);
	    if (unique != plist) {
		refnum = (uint32_t)(uintptr_t)CFDictionaryGetValue(objtable, unique);
		CFDictionaryAddValue(objtable, plist, (const void *)(uintptr_t)refnum);
	    }
	    return;
	}
    }
    refnum = CFArrayGetCount(objlist);
    CFArrayAppendValue(objlist, plist);
    CFDictionaryAddValue(objtable, plist, (const void *)(uintptr_t)refnum);
    if (_kCFRuntimeIDCFDictionary == type) {
        CFIndex count = CFDictionaryGetCount((CFDictionaryRef)plist);
        STACK_BUFFER_DECL(CFPropertyListRef, buffer, (count > 0 && count <= 128) ? count * 2 : 1);
        CFPropertyListRef *list = (count <= 128) ? buffer : (CFPropertyListRef *)CFAllocatorAllocate(kCFAllocatorSystemDefault, 2 * count * sizeof(CFTypeRef), 0);
        CFDictionaryGetKeysAndValues((CFDictionaryRef)plist, list, list + count);
        for (CFIndex idx = 0; idx < 2 * count; idx++) {
            _flattenPlist(list[idx], objlist, objtable, uniquingset);
        }
        if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
    } else if (_kCFRuntimeIDCFArray == type) {
        CFIndex count = CFArrayGetCount((CFArrayRef)plist);
        STACK_BUFFER_DECL(CFPropertyListRef, buffer, (count > 0 && count <= 256) ? count : 1);
        CFPropertyListRef *list = (count <= 256) ? buffer : (CFPropertyListRef *)CFAllocatorAllocate(kCFAllocatorSystemDefault, count * sizeof(CFTypeRef), 0);
        CFArrayGetValues((CFArrayRef)plist, CFRangeMake(0, count), list);
        for (CFIndex idx = 0; idx < count; idx++) {
            _flattenPlist(list[idx], objlist, objtable, uniquingset);
        }
        if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
    }
}

/* Get the number of bytes required to hold the value in 'count'. Will return a power of 2 value big enough to hold 'count'.
 */
CF_INLINE uint8_t _byteCount(uint64_t count) {
    uint64_t mask = ~(uint64_t)0;
    uint8_t size = 0;

    // Find something big enough to hold 'count'
    while (count & mask) {
        size++;
        mask = mask << 8;
    }

    // Ensure that 'count' is a power of 2
    // For sizes bigger than 8, just use the required count
    while ((size != 1 && size != 2 && size != 4 && size != 8) && size <= 8) {
        size++;
    }

    return size;
}

// stream can be a CFWriteStreamRef (on supported platforms) or a CFMutableDataRef
/* Write a property list to a stream, in binary format. plist is the property list to write (one of the basic property list types), stream is the destination of the property list, and estimate is a best-guess at the total number of objects in the property list. The estimate parameter is for efficiency in pre-allocating memory for the uniquing step. Pass in a 0 if no estimate is available. The options flag specifies sort options. If sizeOnly is true, then no actual buffer allocations will be done, but the necessary buffer size will be calculated and return. If the error parameter is non-NULL and an error occurs, it will be used to return a CFError explaining the problem. It is the callers responsibility to release the error. */
CF_PRIVATE CFIndex __CFBinaryPlistWriteOrPresize(CFPropertyListRef plist, CFTypeRef stream, uint64_t estimate, CFOptionFlags options, Boolean sizeOnly, CFErrorRef *error) {
    CFMutableDictionaryRef objtable = NULL;
    CFMutableArrayRef objlist = NULL;
    CFMutableSetRef uniquingset = NULL;
    CFBinaryPlistTrailer trailer;
    uint64_t *offsets, length_so_far;
    int64_t idx, cnt;
    __CFBinaryPlistWriteBuffer *buf;

    //If we're actually serializing, rather than just pre-sizing, we have to have something to serialize into.
    CFAssert(stream || sizeOnly, __kCFLogAssertion, "Passing NULL for the stream argument to __CFBinaryPlistWriteOrPresize is only valid if sizeOnly is true");

    /*
     This is exactly the same as a CFDictionary with NULL callbacks, except that it has the "aggressive growth" flag set, since we're not keeping it around. Radar 21883482
     */
    CFBasicHashCallbacks callbacks = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    objtable = 	(CFMutableDictionaryRef)CFBasicHashCreate(kCFAllocatorSystemDefault, kCFBasicHashHasKeys | kCFBasicHashLinearHashing | kCFBasicHashAggressiveGrowth, &callbacks);
    _CFRuntimeSetInstanceTypeIDAndIsa(objtable, _kCFRuntimeIDCFDictionary);
    
    const CFArrayCallBacks arrayCallbacks = {0, __CFTypeCollectionRetain, __CFTypeCollectionRelease, 0, 0};
    objlist = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &arrayCallbacks);
    
    const CFSetCallBacks setCallbacks = {0, __CFTypeCollectionRetain, __CFTypeCollectionRelease, 0, 0, 0};
    uniquingset = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &setCallbacks);

#if TARGET_OS_OSX
    _CFDictionarySetCapacity(objtable, estimate ? estimate : 650);
    _CFArraySetCapacity(objlist, estimate ? estimate : 650);
    _CFSetSetCapacity(uniquingset, estimate ? estimate : 1000);
#endif

    _flattenPlist(plist, objlist, objtable, uniquingset);

    CFRelease(uniquingset);
    
    cnt = CFArrayGetCount(objlist);
    offsets = (uint64_t *)CFAllocatorAllocate(kCFAllocatorSystemDefault, (CFIndex)(cnt * sizeof(*offsets)), 0);

    buf = (__CFBinaryPlistWriteBuffer *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(__CFBinaryPlistWriteBuffer), 0);
    buf->stream = stream;
    buf->databytes = NULL;
    buf->datalen = 0;
    buf->error = NULL;
    buf->streamIsData = !stream || (CFGetTypeID(stream) == CFDataGetTypeID());
    buf->written = 0;
    buf->used = 0;
    bufferWrite(buf, (uint8_t *)"bplist00", 8, sizeOnly);	// header

    memset(&trailer, 0, sizeof(trailer));
    trailer._numObjects = CFSwapInt64HostToBig(cnt);
    trailer._topObject = 0;	// true for this implementation
    trailer._objectRefSize = _byteCount(cnt);    
    for (idx = 0; idx < cnt; idx++) {
	offsets[idx] = buf->written + buf->used;
	CFPropertyListRef obj = CFArrayGetValueAtIndex(objlist, (CFIndex)idx);
	Boolean success = _appendObject(buf, obj, objtable, trailer._objectRefSize, sizeOnly);
	if (!success) {
	    CFRelease(objtable);
	    CFRelease(objlist);
	    if (error && buf->error) {
		// caller will release error
		*error = buf->error;
	    } else if (buf->error) {
		// caller is not interested in error, release it here
		CFRelease(buf->error);
	    }
	    CFAllocatorDeallocate(kCFAllocatorSystemDefault, buf);
            CFAllocatorDeallocate(kCFAllocatorSystemDefault, offsets);
	    return 0;
	}
    }
    CFRelease(objtable);
    CFRelease(objlist);
    
    length_so_far = buf->written + buf->used;
    trailer._offsetTableOffset = CFSwapInt64HostToBig(length_so_far);
    trailer._offsetIntSize = _byteCount(length_so_far);
    
    for (idx = 0; idx < cnt; idx++) {
	uint64_t swapped = CFSwapInt64HostToBig(offsets[idx]);
	uint8_t *source = (uint8_t *)&swapped;
	bufferWrite(buf, source + sizeof(*offsets) - trailer._offsetIntSize, trailer._offsetIntSize, sizeOnly);
    }
    length_so_far += cnt * trailer._offsetIntSize;
    CFAllocatorDeallocate(kCFAllocatorSystemDefault, offsets);

    bufferWrite(buf, (uint8_t *)&trailer, sizeof(trailer), sizeOnly);
    bufferFlush(buf, sizeOnly);
    length_so_far += sizeof(trailer);
    if (buf->error) {
	if (error) {
	    // caller will release error
	    *error = buf->error;
	} else {
	    CFRelease(buf->error);
	}
        CFAllocatorDeallocate(kCFAllocatorSystemDefault, buf);
	return 0;
    }
    CFAllocatorDeallocate(kCFAllocatorSystemDefault, buf);
    return (CFIndex)length_so_far;
}

CFIndex __CFBinaryPlistWrite(CFPropertyListRef plist, CFTypeRef stream, uint64_t estimate, CFOptionFlags options, CFErrorRef *error) {
    return __CFBinaryPlistWriteOrPresize(plist, stream, estimate, options, false, error);
}

CFIndex __CFBinaryPlistWriteToStream(CFPropertyListRef plist, CFTypeRef stream) {
    return __CFBinaryPlistWriteOrPresize(plist, stream, 0, 0, false, NULL);
}

// to be removed soon
CFIndex __CFBinaryPlistWriteToStreamWithEstimate(CFPropertyListRef plist, CFTypeRef stream, uint64_t estimate) {
    return __CFBinaryPlistWriteOrPresize(plist, stream, estimate, 0, false, NULL);
}

// to be removed soon
CFIndex __CFBinaryPlistWriteToStreamWithOptions(CFPropertyListRef plist, CFTypeRef stream, uint64_t estimate, CFOptionFlags options) {
    return __CFBinaryPlistWriteOrPresize(plist, stream, estimate, options, false, NULL);
}

CF_PRIVATE CFMutableDataRef _CFDataCreateFixedMutableWithBuffer(CFAllocatorRef allocator, CFIndex capacity, const uint8_t *bytes, CFAllocatorRef bytesDeallocator);

CF_PRIVATE CFDataRef __CFBinaryPlistCreateDataUsingExternalBufferAllocator(CFPropertyListRef plist, uint64_t estimate, CFOptionFlags options, CFAllocatorRef (^allocatorCreator)(CFIndex bufferSize), CFErrorRef *error) {
    CFIndex size = __CFBinaryPlistWriteOrPresize(plist, NULL, estimate, options, true, error);
    CFDataRef result = NULL;
    if (size > 0) {
        CFAllocatorRef allocator = allocatorCreator(size);
        if (allocator) {
            void *buffer = CFAllocatorAllocate(allocator, size, 0);
            if (buffer) {
                CFMutableDataRef data = _CFDataCreateFixedMutableWithBuffer(kCFAllocatorSystemDefault, size, buffer, allocator);
                if (data) {
                    CFRelease(allocator);
                    if (size == __CFBinaryPlistWriteOrPresize(plist, data, estimate, options, false, error)) {
                        result = data;
                    } else {
                        CFRelease(data);
                    }
                } else {
                    CFAllocatorDeallocate(allocator, buffer);
                    if (error) {
                        *error = __CFPropertyListCreateError(kCFPropertyListWriteStreamError, CFSTR("Binary property list writing could not be completed because a CFMutableDataRef using the external buffer could not be allocated."));
                    }
                }
            } else {
                CFRelease(allocator);
                if (error) {
                    *error = __CFPropertyListCreateError(kCFPropertyListWriteStreamError, CFSTR("Binary property list writing could not be completed because an external buffer could not be allocated."));
                }
            }
        } else if (error) {
            *error = __CFPropertyListCreateError(kCFPropertyListWriteStreamError, CFSTR("Binary property list writing could not be completed because an allocator could not be created."));
        }
    }
    return result;
}
#endif

#pragma mark -
#pragma mark Reading

#define FAIL_FALSE	do { return false; } while (0)
#define FAIL_NULL	do { return NULL; } while (0)

CF_PRIVATE bool __CFBinaryPlistCreateObjectFiltered(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFAllocatorRef allocator, CFOptionFlags mutabilityOption, CFMutableDictionaryRef objects, CFMutableSetRef set, CFIndex curDepth, CFSetRef keyPaths, CFPropertyListRef *plist, CFTypeID *outPlistTypeID);

/* Grab a valSize-bytes integer out of the buffer pointed at by data and return it.
 */
CF_INLINE uint64_t _getSizedInt(const uint8_t *data, uint8_t valSize) {

    if (valSize == 1) {
        return (uint64_t)*data;
    } else if (valSize == 2) {
        return (uint64_t)_CFUnalignedLoad16BE(data);
    } else if (valSize == 4) {
        return (uint64_t)_CFUnalignedLoad32BE(data);
    } else if (valSize == 8) {
        return _CFUnalignedLoad64BE(data);
    }

    // Compatibility with existing archives, including anything with a non-power-of-2
    // size and 16-byte values
    uint64_t res = 0;
    for (CFIndex idx = 0; idx < valSize; idx++) {
        res = (res << 8) + data[idx];
    }
    return res;
}

bool __CFBinaryPlistGetTopLevelInfo(const uint8_t *databytes, uint64_t datalen, uint8_t *marker, uint64_t *offset, CFBinaryPlistTrailer *trailer) {
    CFBinaryPlistTrailer trail;

    if (!databytes || datalen < sizeof(trail) + 8 + 1) FAIL_FALSE;
    // Tiger and earlier will parse "bplist00"
    // Leopard will parse "bplist00" or "bplist01"
    // SnowLeopard will parse "bplist0?" where ? is any one character
    if (0 != memcmp("bplist0", databytes, 7)) {
	FAIL_FALSE;
    }
    memmove(&trail, databytes + datalen - sizeof(trail), sizeof(trail));
    // In Leopard, the unused bytes in the trailer must be 0 or the parse will fail
    // This check is not present in Tiger and earlier or after Leopard
    trail._numObjects = CFSwapInt64BigToHost(trail._numObjects);
    trail._topObject = CFSwapInt64BigToHost(trail._topObject);
    trail._offsetTableOffset = CFSwapInt64BigToHost(trail._offsetTableOffset);
    
    // Don't overflow on the number of objects or offset of the table
    if (LONG_MAX < trail._numObjects) FAIL_FALSE;
    if (LONG_MAX < trail._offsetTableOffset) FAIL_FALSE;
    
    //  Must be a minimum of 1 object
    if (trail._numObjects < 1) FAIL_FALSE;
    
    // The ref to the top object must be a value in the range of 1 to the total number of objects
    if (trail._numObjects <= trail._topObject) FAIL_FALSE;
    
    // The offset table must be after at least 9 bytes of other data ('bplist??' + 1 byte of object table data).
    if (trail._offsetTableOffset < 9) FAIL_FALSE;
    
    // The trailer must point to a value before itself in the data.
    if (datalen - sizeof(trail) <= trail._offsetTableOffset) FAIL_FALSE;
    
    // Minimum of 1 byte for the size of integers and references in the data
    if (trail._offsetIntSize < 1) FAIL_FALSE;
    if (trail._objectRefSize < 1) FAIL_FALSE;
    
    int32_t err = CF_NO_ERROR;
    
    // The total size of the offset table (number of objects * size of each int in the table) must not overflow
    uint64_t offsetIntSize = trail._offsetIntSize;
    uint64_t offsetTableSize = __check_uint64_mul_unsigned_unsigned(trail._numObjects, offsetIntSize, &err);
    if (CF_NO_ERROR!= err) FAIL_FALSE;
    
    // The offset table must have at least 1 entry
    if (offsetTableSize < 1) FAIL_FALSE;
    
    // Make sure the size of the offset table and data sections do not overflow
    uint64_t objectDataSize = trail._offsetTableOffset - 8;
    uint64_t tmpSum = __check_uint64_add_unsigned_unsigned(8, objectDataSize, &err);
    tmpSum = __check_uint64_add_unsigned_unsigned(tmpSum, offsetTableSize, &err);
    tmpSum = __check_uint64_add_unsigned_unsigned(tmpSum, sizeof(trail), &err);
    if (CF_NO_ERROR != err) FAIL_FALSE;
    
    // The total size of the data should be equal to the sum of offsetTableOffset + sizeof(trailer)
    if (datalen != tmpSum) FAIL_FALSE;
    
    // The object refs must be the right size to point into the offset table. That is, if the count of objects is 260, but only 1 byte is used to store references (max value 255), something is wrong.
    if (trail._objectRefSize < 8 && (1ULL << (8 * trail._objectRefSize)) <= trail._numObjects) FAIL_FALSE;
    
    // The integers used for pointers in the offset table must be able to reach as far as the start of the offset table.
    if (trail._offsetIntSize < 8 && (1ULL << (8 * trail._offsetIntSize)) <= trail._offsetTableOffset) FAIL_FALSE;
    
    
    (void)check_ptr_add(databytes, 8, &err);
    if (CF_NO_ERROR != err) FAIL_FALSE;
    const uint8_t *offsetsFirstByte = check_ptr_add(databytes, trail._offsetTableOffset, &err);
    if (CF_NO_ERROR != err) FAIL_FALSE;
    (void)check_ptr_add(offsetsFirstByte, offsetTableSize - 1, &err);
    if (CF_NO_ERROR != err) FAIL_FALSE;

    const uint8_t *bytesptr = databytes + trail._offsetTableOffset;
    uint64_t maxOffset = trail._offsetTableOffset - 1;
    for (CFIndex idx = 0; idx < trail._numObjects; idx++) {
	uint64_t off = _getSizedInt(bytesptr, trail._offsetIntSize);
	if (maxOffset < off) FAIL_FALSE;
	bytesptr += trail._offsetIntSize;
    }

    bytesptr = databytes + trail._offsetTableOffset + trail._topObject * trail._offsetIntSize;
    uint64_t off = _getSizedInt(bytesptr, trail._offsetIntSize);
    if (off < 8 || trail._offsetTableOffset <= off) FAIL_FALSE;
    if (trailer) *trailer = trail;
    if (offset) *offset = off;
    if (marker) *marker = *(databytes + off);
    return true;
}

CF_INLINE Boolean _typeIsPlistPrimitive(CFTypeID type) {
    if (_kCFRuntimeIDCFDictionary == type || _kCFRuntimeIDCFArray == type || _kCFRuntimeIDCFSet == type || _kCFRuntimeNotATypeID == type) FAIL_FALSE;
    return true;
}

CF_INLINE bool _readInt(const uint8_t *ptr, const uint8_t *end_byte_ptr, uint64_t *bigint, const uint8_t **newptr) {
    if (end_byte_ptr < ptr) FAIL_FALSE;
    uint8_t marker = *ptr++;
    if ((marker & 0xf0) != kCFBinaryPlistMarkerInt) FAIL_FALSE;
    uint64_t cnt = 1 << (marker & 0x0f);
    int32_t err = CF_NO_ERROR;
    const uint8_t *extent = check_ptr_add(ptr, cnt, &err) - 1;
    if (CF_NO_ERROR != err) FAIL_FALSE;
    if (end_byte_ptr < extent) FAIL_FALSE;
    // integers are not required to be in the most compact possible representation, but only the last 64 bits are significant currently
    *bigint = _getSizedInt(ptr, cnt);
    ptr += cnt;
    if (newptr) *newptr = ptr;
    return true;
}

// bytesptr points at a ref
CF_INLINE Boolean _getOffsetOfRefAt(const uint8_t *databytes, const uint8_t *bytesptr, const CFBinaryPlistTrailer *trailer, uint64_t *outOffset) {
    // *trailer contents are trusted, even for overflows -- was checked when the trailer was parsed;
    // this pointer arithmetic and the multiplication was also already done once and checked,
    // and the offsetTable was already validated.
    const uint8_t *objectsFirstByte = databytes + 8;
    const uint8_t *offsetsFirstByte = databytes + trailer->_offsetTableOffset;
    if (bytesptr < objectsFirstByte || offsetsFirstByte - trailer->_objectRefSize < bytesptr) {
        FAIL_FALSE;
    }
    
    const uint64_t ref = _getSizedInt(bytesptr, trailer->_objectRefSize);
    if (trailer->_numObjects <= ref) {
        FAIL_FALSE;
    }
    
    bytesptr = databytes + trailer->_offsetTableOffset + ref * trailer->_offsetIntSize;
    if (outOffset) {
        *outOffset = _getSizedInt(bytesptr, trailer->_offsetIntSize);
    }
    return true;
}

CF_INLINE bool __CFBinaryPlist_beginArrayParse(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, const uint8_t **outPtr, uint8_t *outMarker, uint64_t *outObjectsRangeEnd) {
    uint64_t objectsRangeStart = 8;
    const uint64_t objectsRangeEnd = _CFBinaryPlistTrailer_objectsRangeEnd(trailer);
    if (startOffset < objectsRangeStart || objectsRangeEnd < startOffset) FAIL_FALSE;
    const uint8_t *ptr = databytes + startOffset;
    uint8_t marker = *ptr;
    if ((marker & 0xf0) != kCFBinaryPlistMarkerArray) FAIL_FALSE;
    
    if (outPtr) { *outPtr = ptr; }
    if (outMarker) { *outMarker = marker; }
    if (outObjectsRangeEnd) { *outObjectsRangeEnd = objectsRangeEnd; }
    return true;
}

bool __CFBinaryPlistIsArray(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer) {
    const bool result = __CFBinaryPlist_beginArrayParse(databytes, datalen, startOffset, trailer, NULL, NULL, NULL);
    return result;
}

bool __CFBinaryPlistGetOffsetForValueFromArray2(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFIndex idx, uint64_t *offset, CFMutableDictionaryRef _Nullable unused) {
    const uint8_t *ptr;
    uint8_t marker;
    uint64_t objectsRangeEnd;
    if (!__CFBinaryPlist_beginArrayParse(databytes, datalen, startOffset, trailer, &ptr, &marker, &objectsRangeEnd)) FAIL_FALSE;

    int32_t err = CF_NO_ERROR;
    ptr = check_ptr_add(ptr, 1, &err);
    if (CF_NO_ERROR != err) FAIL_FALSE;
    uint64_t cnt = (marker & 0x0f);
    if (0xf == cnt) {
	uint64_t bigint;
	if (!_readInt(ptr, databytes + objectsRangeEnd, &bigint, &ptr)) FAIL_FALSE;
	if (LONG_MAX < bigint) FAIL_FALSE;
	cnt = bigint;
    }
    if (cnt <= idx) FAIL_FALSE;
    size_t byte_cnt = check_size_t_mul(cnt, trailer->_objectRefSize, &err);
    if (CF_NO_ERROR != err) FAIL_FALSE;
    const uint8_t *extent = check_ptr_add(ptr, byte_cnt, &err) - 1;
    if (CF_NO_ERROR != err) FAIL_FALSE;
    if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
    if (!_getOffsetOfRefAt(databytes, ptr + idx * trailer->_objectRefSize, trailer, offset)) {
        FAIL_FALSE;
    }
    return true;
}

CF_INLINE bool __CFBinaryPList_beginDictionaryParse(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, uint64_t *outEntryCount, const uint8_t **outPtr, uint8_t *outMarker, const uint8_t **outExtent) {
    
    // Require that startOffset is in the range of the object table
    uint64_t objectsRangeStart = 8;
    const uint64_t objectsRangeEnd = _CFBinaryPlistTrailer_objectsRangeEnd(trailer);
    
    if (startOffset < objectsRangeStart || objectsRangeEnd < startOffset) FAIL_FALSE;
    
    // ptr is the start of the dictionary we are reading
    const uint8_t *ptr = databytes + startOffset;
    
    // Check that the data pointer actually points to a dictionary
    uint8_t marker = *ptr;
    if ((marker & 0xf0) != kCFBinaryPlistMarkerDict) FAIL_FALSE;
    
    // Get the number of objects in this dictionary
    int32_t err = CF_NO_ERROR;
    ptr = check_ptr_add(ptr, 1, &err);
    if (CF_NO_ERROR != err) FAIL_FALSE;
    uint64_t cnt = (marker & 0x0f);
    if (0xf == cnt) {
        uint64_t bigint = 0;
        if (!_readInt(ptr, databytes + objectsRangeEnd, &bigint, &ptr)) FAIL_FALSE;
        if (LONG_MAX < bigint) FAIL_FALSE;
        cnt = bigint;
    }
    
    // Total number of objects (keys + values) is cnt * 2
    cnt = check_size_t_mul(cnt, 2, &err);
    if (CF_NO_ERROR != err) FAIL_FALSE;
    size_t byte_cnt = check_size_t_mul(cnt, trailer->_objectRefSize, &err);
    if (CF_NO_ERROR != err) FAIL_FALSE;
    
    // Find the end of the dictionary
    const uint8_t *extent = check_ptr_add(ptr, byte_cnt, &err) - 1;
    if (CF_NO_ERROR != err) FAIL_FALSE;
    
    // Check that we didn't overflow the size of the dictionary
    if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
    
    if (outEntryCount) { *outEntryCount = cnt; }
    if (outPtr) { *outPtr = ptr; }
    if (outMarker) { *outMarker = marker; }
    if (outExtent) { *outExtent = extent; }
    
    return true;
}

CF_PRIVATE bool __CFBinaryPlistIsDictionary(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer) {
    const bool result = __CFBinaryPList_beginDictionaryParse(databytes, datalen, startOffset, trailer, NULL, NULL, NULL, NULL);
    return result;
}

CFSetRef __CFBinaryPlistCopyTopLevelKeys(CFAllocatorRef allocator, const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer) {
    uint64_t cnt = 0;
    const uint8_t *ptr = NULL;
    uint8_t marker = 0;
    const uint8_t *extent = NULL;
    if (!__CFBinaryPList_beginDictionaryParse(databytes, datalen, startOffset, trailer, &cnt, &ptr, &marker, &extent)) {
        FAIL_NULL;
    }
    

    // Find the object in the dictionary with this key
    cnt = cnt / 2;
    uint64_t off;

   
    // Perform linear accumulation of the keys
    size_t buffer_idx = 0;
    size_t capacity = 16;
    CFStringRef *buffer = malloc(sizeof(CFStringRef) * capacity);
    if (buffer == NULL) {
        FAIL_NULL;
    }
    bool bad = false;
    for (CFIndex idx = 0; !bad && idx < cnt; idx++) {
        if (!_getOffsetOfRefAt(databytes, ptr, trailer, &off)) {
            bad = true;
            break;
        }
        marker = *(databytes + off);

        // Unlike in __CFBinaryPlistGetOffsetForValueFromDictionary3, we're accumulating keys, so we go through the CFObjectRef case always.
        CFPropertyListRef keyInData = NULL;
        CFTypeID typeID = _kCFRuntimeNotATypeID;
        if (!(__CFBinaryPlistCreateObjectFiltered(databytes, datalen, off, trailer, allocator, kCFPropertyListImmutable, NULL, NULL, 0, NULL, &keyInData, &typeID) && typeID == _kCFRuntimeIDCFString)) {
            bad = true;
            if (keyInData) {
                // we're not storing keyInData in the buffer, so we need to free it now; buffered keys are cleaned below
                CFRelease(keyInData);
            }
            break;
        }
        
        buffer[buffer_idx] = keyInData;
        ++buffer_idx;
        if (buffer_idx >= capacity) {
            const size_t newCapacity = capacity * 3 / 2;
            // NOTE: this code doesn't use __CFSafelyReallocate as it handles its own recovery
            CFStringRef *reallocated = realloc(buffer, sizeof(CFStringRef) * newCapacity);
            if (reallocated == NULL) {
                bad = true;
                break;
            } else {
                buffer = reallocated;
                capacity = newCapacity;
            }
        }
        ptr += trailer->_objectRefSize;
    }
    
    CFSetRef result = NULL;
    if (!bad) {
        result = CFSetCreate(allocator, (const void **)buffer, buffer_idx, &kCFTypeSetCallBacks);
    }
    
    // cleanup any keys stored in the local buffer
    for (size_t i = 0; i < buffer_idx; ++i) {
        CFStringRef s = buffer[i];
        if (s) {
            CFRelease(s);
        }
    }
    free(buffer);
    
    return result;
}

/* Get the offset for a value in a dictionary in a binary property list.
 @param databytes A pointer to the start of the binary property list data.
 @param datalen The length of the data.
 @param startOffset The offset at which the dictionary starts.
 @param trailer A pointer to a filled out trailer structure (use __CFBinaryPlistGetTopLevelInfo).
 @param key A string key in the dictionary that should be searched for.
 @param koffset Will be filled out with the offset to the key in the data bytes.
 @param voffset Will be filled out with the offset to the value in the data bytes.
 @param unused Unused parameter.
 @param objects Used for caching objects. Should be a valid CFMutableDictionaryRef.
 @return True if the key was found, false otherwise.
*/
bool __CFBinaryPlistGetOffsetForValueFromDictionary3(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFTypeRef key, uint64_t *koffset, uint64_t *voffset, Boolean unused, CFMutableDictionaryRef _Nullable unused2) {
    
    // Require a key that is a plist primitive
    CFTypeID const keyTypeID = key ? CFGetTypeID(key) : _kCFRuntimeNotATypeID;
    if (!_typeIsPlistPrimitive(keyTypeID)) FAIL_FALSE;
    
    uint64_t cnt = 0;
    const uint8_t *ptr = NULL;
    uint8_t marker = 0;
    const uint8_t *extent = NULL;
    if (!__CFBinaryPList_beginDictionaryParse(databytes, datalen, startOffset, trailer, &cnt, &ptr, &marker, &extent)) FAIL_NULL;
    
    // For short keys (15 bytes or less) in ASCII form, we can do a quick comparison check
    // We get the pointer or copy the buffer here, outside of the loop
    CFIndex stringKeyLen = -1;
    if (keyTypeID == _kCFRuntimeIDCFString) {
	stringKeyLen = CFStringGetLength((CFStringRef)key);
    }
    
    // Find the object in the dictionary with this key
    cnt = cnt / 2;
    uint64_t totalKeySize = cnt * trailer->_objectRefSize;
    uint64_t off;
    Boolean match = false;
    CFPropertyListRef keyInData = NULL;
    
#define KEY_BUFF_SIZE 16    
    char keyBuffer[KEY_BUFF_SIZE];
    const char *keyBufferPtr = NULL;
    
    // If we have a string for the key, then we will grab the ASCII encoded version of it, if possible, and do a memcmp on it
    if (stringKeyLen != -1) {
	// Since we will only be comparing ASCII strings, we can attempt to get a pointer using MacRoman encoding
	// (this is cheaper than a copy)
	if (!(keyBufferPtr = CFStringGetCStringPtr((CFStringRef)key, kCFStringEncodingMacRoman)) && stringKeyLen < KEY_BUFF_SIZE) {
	    const Boolean converted = CFStringGetCString((CFStringRef)key, keyBuffer, KEY_BUFF_SIZE, kCFStringEncodingMacRoman);
            if (converted && strnlen(keyBuffer, KEY_BUFF_SIZE) == stringKeyLen) {
                // The pointer should now point to our keyBuffer instead of the original string buffer, since we've copied it
                keyBufferPtr = keyBuffer;
            }
	}
    }
    
    // Perform linear search of the keys
    int32_t err = CF_NO_ERROR;
    const uint64_t objectsRangeEnd = _CFBinaryPlistTrailer_objectsRangeEnd(trailer);
    for (CFIndex idx = 0; idx < cnt; idx++) {
        if (!_getOffsetOfRefAt(databytes, ptr, trailer, &off)) {
            FAIL_FALSE;
	}
	marker = *(databytes + off);
	// if it is an ASCII string in the data, then we do a memcmp. If the key isn't ASCII, then it won't pass the compare, unless it hits some odd edge case of the ASCII string actually containing the unicode escape sequence.
	if (keyBufferPtr && (marker & 0xf0) == kCFBinaryPlistMarkerASCIIString) {
	    CFIndex len = marker & 0x0f;
	    // move past the marker
	    err = CF_NO_ERROR;
	    const uint8_t *ptr2 = check_ptr_add(databytes, off, &err);
	    if (CF_NO_ERROR != err) FAIL_FALSE;
	    ptr2 = check_ptr_add(ptr2, 1, &err);
	    if (CF_NO_ERROR != err) FAIL_FALSE;
	    
	    // If the key's length is large, and the length we are querying is also large, then we have to read it in. If stringKeyLen is less than 0xf, then len will never be equal to it if it was encoded as large.
	    if (0xf == len && stringKeyLen >= 0xf) {
		uint64_t bigint = 0;
		if (!_readInt(ptr2, databytes + objectsRangeEnd, &bigint, &ptr2)) FAIL_FALSE;
		if (LONG_MAX < bigint) FAIL_FALSE;
		len = (CFIndex)bigint;
	    }
	    
	    if (len == stringKeyLen) {                
		err = CF_NO_ERROR;
		extent = check_ptr_add(ptr2, len, &err);
		if (CF_NO_ERROR != err) FAIL_FALSE;
		
		if (databytes + trailer->_offsetTableOffset <= extent) FAIL_FALSE;
		
		// Compare the key to this potential match
		if (memcmp(ptr2, keyBufferPtr, stringKeyLen) == 0) {
		    match = true;
		}
	    }
	} else {
            // temp object not saved in 'objects', because we don't know what allocator to use
            // (what allocator __CFBinaryPlistCreateObjectFiltered() or __CFBinaryPlistCreateObject()
            //  will eventually be called with which results in that object)
	    keyInData = NULL;
            CFTypeID typeID = _kCFRuntimeNotATypeID;
	    if (!__CFBinaryPlistCreateObjectFiltered(databytes, datalen, off, trailer, kCFAllocatorSystemDefault, kCFPropertyListImmutable, NULL /*objects*/, NULL, 0, NULL, &keyInData, &typeID) || !_typeIsPlistPrimitive(typeID)) {
		if (keyInData) CFRelease(keyInData);
		FAIL_FALSE;
	    }
	    
	    match = CFEqual(key, keyInData);            
            CFRelease(keyInData);
	}            
	
	if (match) {
            if (!_getOffsetOfRefAt(databytes, ptr + totalKeySize, trailer, voffset)) {
                FAIL_FALSE;
            }
            if (koffset) { *koffset = off; }
            return true;
	}
	
	ptr += trailer->_objectRefSize;
    }
    
    FAIL_FALSE;
}

extern CFDictionaryRef __CFDictionaryCreateTransfer(CFAllocatorRef allocator, const void * *klist, const void * *vlist, CFIndex numValues);
extern CFSetRef __CFSetCreateTransfer(CFAllocatorRef allocator, const void * *klist, CFIndex numValues);
extern CFArrayRef __CFArrayCreateTransfer(CFAllocatorRef allocator, const void * *klist, CFIndex numValues);
CF_PRIVATE void __CFPropertyListCreateSplitKeypaths(CFAllocatorRef allocator, CFSetRef currentKeys, CFSetRef *theseKeys, CFSetRef *nextKeys);

CF_PRIVATE bool __CFBinaryPlistCreateObjectFiltered(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFAllocatorRef allocator, CFOptionFlags mutabilityOption, CFMutableDictionaryRef objects, CFMutableSetRef set, CFIndex curDepth, CFSetRef keyPaths, CFPropertyListRef *outPlist, CFTypeID *outPlistTypeID) {
    
    // NOTE: Bailing out here will cause us to attempt to parse
    //       as XML (which will fail) then as a OpenSTEP plist
    //       the final error string is less than helpful:
    //       "Unexpected character b at line 1".
    //       It would be nice to actually be more descriptive but that
    //       would require a more scaffolding.
    if (curDepth > _CFPropertyListMaxRecursionDepth()) {
        // Bail before we get so far into the stack that we run out of space.
        // Emit an `os_log_fault` to relay the issue to the debugger and to track how common this case may be.
        os_log_fault(_CFOSLog(), "Too many nested arrays or dictionaries");
        FAIL_FALSE;
    }

    if (objects && outPlist) {
        *outPlist = CFDictionaryGetValue(objects, (const void *)(uintptr_t)startOffset);
        if (*outPlist) {
            // have to assume that '*plist' was previously created with same allocator that is now being passed in
            CFRetain(*outPlist);
            if (outPlistTypeID) *outPlistTypeID = CFGetTypeID(*outPlist);
            return true;
        }
    }

    // at any one invocation of this function, set should contain the offsets in the "path" down to this object
    if (set && CFSetContainsValue(set, (const void *)(uintptr_t)startOffset)) FAIL_FALSE;

    // databytes is trusted to be at least datalen bytes long
    // *trailer contents are trusted, even for overflows -- was checked when the trailer was parsed
    uint64_t objectsRangeStart = 8;
    const uint64_t objectsRangeEnd = _CFBinaryPlistTrailer_objectsRangeEnd(trailer);
    if (startOffset < objectsRangeStart || objectsRangeEnd < startOffset) FAIL_FALSE;

    uint64_t off;
    CFPropertyListRef *list = NULL;

    uint8_t marker = *(databytes + startOffset);
    switch (marker & 0xf0) {
    case kCFBinaryPlistMarkerNull:
	switch (marker) {
	case kCFBinaryPlistMarkerNull:
	    if (outPlist) *outPlist = kCFNull;
            if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFNull;
	    return true;
	case kCFBinaryPlistMarkerFalse:
	    if (outPlist) *outPlist = kCFBooleanFalse;
            if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFBoolean;
	    return true;
	case kCFBinaryPlistMarkerTrue:
	    if (outPlist) *outPlist = kCFBooleanTrue;
            if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFBoolean;
	    return true;
	}
	FAIL_FALSE;
    case kCFBinaryPlistMarkerInt:
    {
	const uint8_t *ptr = (databytes + startOffset);
	int32_t err = CF_NO_ERROR;
	ptr = check_ptr_add(ptr, 1, &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
	uint64_t cnt = 1 << (marker & 0x0f);
	const uint8_t *extent = check_ptr_add(ptr, cnt, &err) - 1;
	if (CF_NO_ERROR != err) FAIL_FALSE;
	if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
	if (16 < cnt) FAIL_FALSE;
	// in format version '00', 1, 2, and 4-byte integers have to be interpreted as unsigned,
	// whereas 8-byte integers are signed (and 16-byte when available)
	// negative 1, 2, 4-byte integers are always emitted as 8 bytes in format '00'
	// integers are not required to be in the most compact possible representation, but only the last 64 bits are significant currently
	uint64_t bigint = _getSizedInt(ptr, cnt);
        if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFNumber;
        if (outPlist) {
            CFNumberRef number = NULL;
            if (8 < cnt) {
                CFSInt128Struct val;
                val.high = 0;
                val.low = bigint;
                number = CFNumberCreate(allocator, kCFNumberSInt128Type, &val);
            } else {
                number = CFNumberCreate(allocator, kCFNumberSInt64Type, &bigint);
            }
            // these are always immutable
            if (objects && number) {
                CFDictionarySetValue(objects, (const void *)(uintptr_t)startOffset, number);
            }
            *outPlist = number;
            return number ? true : false;
        } else {
            // Assume CFNumber creation would always succeed.
            return true;
        }
    }
    case kCFBinaryPlistMarkerReal:
	switch (marker & 0x0f) {
	case 2: {
	    const uint8_t *ptr = (databytes + startOffset);
	    int32_t err = CF_NO_ERROR;
	    ptr = check_ptr_add(ptr, 1, &err);
	    if (CF_NO_ERROR != err) FAIL_FALSE;
	    const uint8_t *extent = check_ptr_add(ptr, 4, &err) - 1;
	    if (CF_NO_ERROR != err) FAIL_FALSE;
	    if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
	    CFSwappedFloat32 swapped32;
	    memmove(&swapped32, ptr, 4);
	    float f = CFConvertFloat32SwappedToHost(swapped32);
            if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFNumber;
            if (outPlist) {
                CFNumberRef number = CFNumberCreate(allocator, kCFNumberFloat32Type, &f);
                // these are always immutable
                if (objects && number) {
                    CFDictionarySetValue(objects, (const void *)(uintptr_t)startOffset, number);
                }
                *outPlist = number;
                return number ? true : false;
            } else {
                // Assume CFNumber creation would always succeed.
                return true;
            }
	}
	case 3: {
	    const uint8_t *ptr = (databytes + startOffset);
	    int32_t err = CF_NO_ERROR;
	    ptr = check_ptr_add(ptr, 1, &err);
	    if (CF_NO_ERROR != err) FAIL_FALSE;
	    const uint8_t *extent = check_ptr_add(ptr, 8, &err) - 1;
	    if (CF_NO_ERROR != err) FAIL_FALSE;
	    if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
	    CFSwappedFloat64 swapped64;
	    memmove(&swapped64, ptr, 8);
	    double d = CFConvertFloat64SwappedToHost(swapped64);
            if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFNumber;
            if (outPlist) {
                CFNumberRef number = CFNumberCreate(allocator, kCFNumberFloat64Type, &d);
                // these are always immutable
                if (objects && number) {
                    CFDictionarySetValue(objects, (const void *)(uintptr_t)startOffset, number);
                }
                *outPlist = number;
                return number ? true : false;
            } else {
                // Assume CFNumber creation would always succeed.
                return true;
            }
	}
	}
	FAIL_FALSE;
    case kCFBinaryPlistMarkerDate & 0xf0:
	switch (marker) {
	case kCFBinaryPlistMarkerDate: {
	    const uint8_t *ptr = (databytes + startOffset);
	    int32_t err = CF_NO_ERROR;
	    ptr = check_ptr_add(ptr, 1, &err);
	    if (CF_NO_ERROR != err) FAIL_FALSE;
	    const uint8_t *extent = check_ptr_add(ptr, 8, &err) - 1;
	    if (CF_NO_ERROR != err) FAIL_FALSE;
	    if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
	    CFSwappedFloat64 swapped64;
	    memmove(&swapped64, ptr, 8);
	    double d = CFConvertFloat64SwappedToHost(swapped64);
            if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFDate;
            if (outPlist) {
                CFDateRef date = CFDateCreate(allocator, d);
                // these are always immutable
                if (objects && date) {
                    CFDictionarySetValue(objects, (const void *)(uintptr_t)startOffset, date);
                }
                *outPlist = date;
                return date ? true : false;
            } else {
                // Assume CFDate creation would always succeed.
                return true;
            }
	}
	}
	FAIL_FALSE;
    case kCFBinaryPlistMarkerData: {
	const uint8_t *ptr = databytes + startOffset;
	int32_t err = CF_NO_ERROR;
	ptr = check_ptr_add(ptr, 1, &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
	CFIndex cnt = marker & 0x0f;
	if (0xf == cnt) {
	    uint64_t bigint = 0;
	    if (!_readInt(ptr, databytes + objectsRangeEnd, &bigint, &ptr)) FAIL_FALSE;
	    if (LONG_MAX < bigint) FAIL_FALSE;
	    cnt = (CFIndex)bigint;
	}
	const uint8_t *extent = check_ptr_add(ptr, cnt, &err) - 1;
	if (CF_NO_ERROR != err) FAIL_FALSE;
	if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
        if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFData;
        if (outPlist) {
            CFDataRef data = NULL;
            if (mutabilityOption == kCFPropertyListMutableContainersAndLeaves) {
                data = CFDataCreateMutable(allocator, 0);
                if (data) CFDataAppendBytes((CFMutableDataRef)data, ptr, cnt);
            } else {
                data = CFDataCreate(allocator, ptr, cnt);
            }
            if (objects && data && (mutabilityOption != kCFPropertyListMutableContainersAndLeaves)) {
                CFDictionarySetValue(objects, (const void *)(uintptr_t)startOffset, data);
            }
            *outPlist = data;
            return data ? true : false;
        } else {
            // Assume CFData creation would always succeed
            return true;
        }
	}
    case kCFBinaryPlistMarkerASCIIString: {
	const uint8_t *ptr = databytes + startOffset;
	int32_t err = CF_NO_ERROR;
	ptr = check_ptr_add(ptr, 1, &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
	CFIndex cnt = marker & 0x0f;
	if (0xf == cnt) {
            uint64_t bigint = 0;
	    if (!_readInt(ptr, databytes + objectsRangeEnd, &bigint, &ptr)) FAIL_FALSE;
	    if (LONG_MAX < bigint) FAIL_FALSE;
	    cnt = (CFIndex)bigint;
	}
	const uint8_t *extent = check_ptr_add(ptr, cnt, &err) - 1;
	if (CF_NO_ERROR != err) FAIL_FALSE;
	if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
        if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFString;
        if (outPlist) {
            CFStringRef string = CFStringCreateWithBytes(allocator, ptr, cnt, kCFStringEncodingASCII, false);
            if (mutabilityOption == kCFPropertyListMutableContainersAndLeaves) {
                if (string) {
                    CFStringRef tmp = string;
                    string = CFStringCreateMutableCopy(allocator, 0, string);
                    CFRelease(tmp);
                }
            }
            if (objects && string && (mutabilityOption != kCFPropertyListMutableContainersAndLeaves)) {
                CFDictionarySetValue(objects, (const void *)(uintptr_t)startOffset, string);
            }
            *outPlist = string;
            return string ? true : false;
        } else {
            // Assume CFString creation with kCFStringEncodingASCII would always succeed.
            return true;
        }
	}
    case kCFBinaryPlistMarkerUnicode16String: {
	const uint8_t *ptr = databytes + startOffset;
	int32_t err = CF_NO_ERROR;
	ptr = check_ptr_add(ptr, 1, &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
	CFIndex cnt = marker & 0x0f;
	if (0xf == cnt) {
            uint64_t bigint = 0;
	    if (!_readInt(ptr, databytes + objectsRangeEnd, &bigint, &ptr)) FAIL_FALSE;
	    if (LONG_MAX < bigint) FAIL_FALSE;
	    cnt = (CFIndex)bigint;
	}
	const uint8_t *extent = check_ptr_add(ptr, cnt, &err) - 1;
	extent = check_ptr_add(extent, cnt, &err);	// 2 bytes per character
	if (CF_NO_ERROR != err) FAIL_FALSE;
	if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
	size_t byte_cnt = check_size_t_mul(cnt, sizeof(UniChar), &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
        if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFString;
        if (outPlist) {
            UniChar *chars = (UniChar *)CFAllocatorAllocate(kCFAllocatorSystemDefault, byte_cnt, 0);
            if (!chars) FAIL_FALSE;
            memmove(chars, ptr, byte_cnt);
            for (CFIndex idx = 0; idx < cnt; idx++) {
                chars[idx] = CFSwapInt16BigToHost(chars[idx]);
            }
            CFStringRef string = CFStringCreateWithCharacters(allocator, chars, cnt);
            if (mutabilityOption == kCFPropertyListMutableContainersAndLeaves) {
                if (string) {
                    CFStringRef tmp = string;
                    string = CFStringCreateMutableCopy(allocator, 0, string);
                    CFRelease(tmp);
                }
            }
            CFAllocatorDeallocate(kCFAllocatorSystemDefault, chars);
            if (objects && string && (mutabilityOption != kCFPropertyListMutableContainersAndLeaves)) {
                CFDictionarySetValue(objects, (const void *)(uintptr_t)startOffset, string);
            }
            *outPlist = string;
            return string ? true : false;
        } else {
            // Assume CFStringCreateWithCharacters would always succeed.
            return true;
        }
	}
    case kCFBinaryPlistMarkerUID: {
	const uint8_t *ptr = databytes + startOffset;
	int32_t err = CF_NO_ERROR;
	ptr = check_ptr_add(ptr, 1, &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
	CFIndex cnt = (marker & 0x0f) + 1;
	const uint8_t *extent = check_ptr_add(ptr, cnt, &err) - 1;
	if (CF_NO_ERROR != err) FAIL_FALSE;
	if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
	// uids are not required to be in the most compact possible representation, but only the last 64 bits are significant currently
	uint64_t bigint = _getSizedInt(ptr, cnt);
	if (UINT32_MAX < bigint) FAIL_FALSE;
        if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFKeyedArchiverUID;
        if (outPlist) {
            CFKeyedArchiverUIDRef uid = _CFKeyedArchiverUIDCreate(allocator, (uint32_t)bigint);
            // these are always immutable
            if (objects && uid) {
                CFDictionarySetValue(objects, (const void *)(uintptr_t)startOffset, uid);
            }
            *outPlist = uid;
            return (uid) ? true : false;
        } else {
            // Assume CFKeyedArchiverUID creation would always succeed.
            return true;
        }
	}
    case kCFBinaryPlistMarkerArray:
    case kCFBinaryPlistMarkerSet: {
	const uint8_t *ptr = databytes + startOffset;
	int32_t err = CF_NO_ERROR;
	ptr = check_ptr_add(ptr, 1, &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
	CFIndex arrayCount = marker & 0x0f;
	if (0xf == arrayCount) {
	    uint64_t bigint = 0;
	    if (!_readInt(ptr, databytes + objectsRangeEnd, &bigint, &ptr)) FAIL_FALSE;
	    if (LONG_MAX < bigint) FAIL_FALSE;
	    arrayCount = (CFIndex)bigint;
	}
	size_t byte_cnt = check_size_t_mul(arrayCount, trailer->_objectRefSize, &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
	const uint8_t *extent = check_ptr_add(ptr, byte_cnt, &err) - 1;
	if (CF_NO_ERROR != err) FAIL_FALSE;
	if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
	byte_cnt = check_size_t_mul(arrayCount, sizeof(CFPropertyListRef), &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
        STACK_BUFFER_DECL(CFPropertyListRef, buffer, (arrayCount > 0 && arrayCount <= 256) ? arrayCount : 1);
        if (outPlist) {
            list = (arrayCount <= 256) ? buffer : (CFPropertyListRef *)CFAllocatorAllocate(kCFAllocatorSystemDefault, byte_cnt, 0);
            if (!list) FAIL_FALSE;
        }
        _CFReleaseDeferred CFMutableSetRef madeSet = NULL;
	if (!set && 15 < curDepth) {
	    madeSet = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, NULL);
            set = madeSet;
	}
        
        Boolean success = true;
        CFTypeID typeID = _kCFRuntimeNotATypeID;
        if (set) CFSetAddValue(set, (const void *)(uintptr_t)startOffset);
        if ((marker & 0xf0) == kCFBinaryPlistMarkerArray && keyPaths) {
            // Only get a subset of this array
            CFSetRef theseKeys, nextKeys;
            __CFPropertyListCreateSplitKeypaths(kCFAllocatorSystemDefault, keyPaths, &theseKeys, &nextKeys);
            
            CFMutableArrayRef array = CFArrayCreateMutable(allocator, CFSetGetCount(theseKeys), &kCFTypeArrayCallBacks);
            if (theseKeys) {
                CFTypeRef *keys = (CFTypeRef *)malloc(CFSetGetCount(theseKeys) * sizeof(CFTypeRef));
                CFSetGetValues(theseKeys, keys);
                CFIndex theseKeysCount = CFSetGetCount(theseKeys);
                for (CFIndex i = 0; i < theseKeysCount; i++) {
                    CFStringRef key = (CFStringRef)keys[i];
                    SInt32 intValue = CFStringGetIntValue(key);
                    if ((intValue == 0 && CFStringCompare(CFSTR("0"), key, 0) != kCFCompareEqualTo) || intValue == INT_MAX || intValue == INT_MIN || intValue < 0) {
                        // skip, doesn't appear to be a proper integer
                    } else {
                        uint64_t valueOffset;
                        Boolean found = __CFBinaryPlistGetOffsetForValueFromArray2(databytes, datalen, startOffset, trailer, (CFIndex)intValue, &valueOffset, objects);
                        if (found) {
                            CFPropertyListRef result = NULL;
                            success = __CFBinaryPlistCreateObjectFiltered(databytes, datalen, valueOffset, trailer, allocator, mutabilityOption, objects, set, curDepth + 1, nextKeys, outPlist ? &result : NULL, NULL);
                            if (success) {
                                if (result) {
                                    CFArrayAppendValue(array, result);
                                    CFRelease(result);
                                }
                            } else {
                                break;
                            }
                        }
                    }
                }
                
                free(keys);
                CFRelease(theseKeys);
            }
            if (nextKeys) CFRelease(nextKeys);
            
            if (success && outPlist) {
                if (!(mutabilityOption == kCFPropertyListMutableContainers || mutabilityOption == kCFPropertyListMutableContainersAndLeaves)) {
                    // make immutable
                    *outPlist = CFArrayCreateCopy(allocator, array);
                    CFRelease(array);
                } else {
                    *outPlist = array;
                }
            } else if (array) {
                CFRelease(array);
            }
            typeID = _kCFRuntimeIDCFArray;
        } else {            
            for (CFIndex idx = 0; idx < arrayCount; idx++) {
                if (!_getOffsetOfRefAt(databytes, ptr, trailer, &off)) {
                    if (list) {
                        while (idx--) {
                            CFRelease(list[idx]);
                        }
                        if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
                    }
                    FAIL_FALSE;
                }
                CFPropertyListRef pl = NULL;
                if (!__CFBinaryPlistCreateObjectFiltered(databytes, datalen, off, trailer, allocator, mutabilityOption, objects, set, curDepth + 1, NULL, outPlist ? &pl : NULL, NULL)) {
                    if (list) {
                        while (idx--) {
                            CFRelease(list[idx]);
                        }
                        if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
                    }
                    FAIL_FALSE;
                }
                if (list) {
                    *((void **)list + idx) = (void *)pl;
                }
                ptr += trailer->_objectRefSize;
            }
            success = true;
            if ((marker & 0xf0) == kCFBinaryPlistMarkerArray) {
                if (outPlist) {
                    if (mutabilityOption != kCFPropertyListImmutable) {
                        *outPlist = CFArrayCreateMutable(allocator, 0, &kCFTypeArrayCallBacks);
                        CFArrayReplaceValues((CFMutableArrayRef)*outPlist, CFRangeMake(0, 0), list, arrayCount);
                        for (CFIndex idx = 0; idx < arrayCount; idx++) {
                            CFRelease(list[idx]);
                        }
                    } else {
                        *outPlist = __CFArrayCreateTransfer(allocator, list, arrayCount);
                    }
                }
                typeID = _kCFRuntimeIDCFArray;
            } else {
                if (outPlist) {
                    if (mutabilityOption != kCFPropertyListImmutable) {
                        *outPlist = CFSetCreateMutable(allocator, 0, &kCFTypeSetCallBacks);
                        for (CFIndex idx = 0; idx < arrayCount; idx++) {
                            CFSetAddValue((CFMutableSetRef)*outPlist, list[idx]);
                        }
                        for (CFIndex idx = 0; idx < arrayCount; idx++) {
                            CFRelease(list[idx]);
                        }
                    } else {
                        *outPlist = __CFSetCreateTransfer(allocator, list, arrayCount);
                    }
                }
                typeID = _kCFRuntimeIDCFSet;
            }
        }
        if (outPlistTypeID) *outPlistTypeID = typeID;
        if (set) CFSetRemoveValue(set, (const void *)(uintptr_t)startOffset);
        if (objects && success && outPlist && (mutabilityOption == kCFPropertyListImmutable)) {
            CFDictionarySetValue(objects, (const void *)(uintptr_t)startOffset, *outPlist);
	}
	if (list && list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
	return success;
	}
    case kCFBinaryPlistMarkerDict: {
	const uint8_t *ptr = databytes + startOffset;
	int32_t err = CF_NO_ERROR;
	ptr = check_ptr_add(ptr, 1, &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
	CFIndex dictionaryCount = marker & 0x0f;
	if (0xf == dictionaryCount) {
	    uint64_t bigint = 0;
	    if (!_readInt(ptr, databytes + objectsRangeEnd, &bigint, &ptr)) FAIL_FALSE;
	    if (LONG_MAX < bigint) FAIL_FALSE;
	    dictionaryCount = (CFIndex)bigint;
	}
	dictionaryCount = check_size_t_mul(dictionaryCount, 2, &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
	size_t byte_cnt = check_size_t_mul(dictionaryCount, trailer->_objectRefSize, &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
	const uint8_t *extent = check_ptr_add(ptr, byte_cnt, &err) - 1;
	if (CF_NO_ERROR != err) FAIL_FALSE;
	if (databytes + objectsRangeEnd < extent) FAIL_FALSE;
	byte_cnt = check_size_t_mul(dictionaryCount, sizeof(CFPropertyListRef), &err);
	if (CF_NO_ERROR != err) FAIL_FALSE;
        STACK_BUFFER_DECL(CFPropertyListRef, buffer, 0 < dictionaryCount && dictionaryCount <= 256 ? dictionaryCount : 1);
        if (outPlist) {
            list = (dictionaryCount <= 256) ? buffer : (CFPropertyListRef *)CFAllocatorAllocate(kCFAllocatorSystemDefault, byte_cnt, 0);
            if (!list) FAIL_FALSE;
        }
        _CFReleaseDeferred CFMutableSetRef madeSet = NULL;
	if (!set && 15 < curDepth) {
	    madeSet = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, NULL);
            set = madeSet;
	}
        
        Boolean success = true;
        if (set) CFSetAddValue(set, (const void *)(uintptr_t)startOffset);
        if (keyPaths) {
            // Only get a subset of this dictionary
            CFSetRef theseKeys, nextKeys;
            __CFPropertyListCreateSplitKeypaths(kCFAllocatorSystemDefault, keyPaths, &theseKeys, &nextKeys);
            
            CFMutableDictionaryRef dict = CFDictionaryCreateMutable(allocator, CFSetGetCount(theseKeys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            if (theseKeys) {
                CFTypeRef *keys = (CFTypeRef *)malloc(CFSetGetCount(theseKeys) * sizeof(CFTypeRef));
                CFSetGetValues(theseKeys, keys);
                for (CFIndex i = 0; i < CFSetGetCount(theseKeys); i++) {
                    CFStringRef key = (CFStringRef)keys[i];
                    uint64_t keyOffset, valueOffset;
                    Boolean found = __CFBinaryPlistGetOffsetForValueFromDictionary3(databytes, datalen, startOffset, trailer, key, &keyOffset, &valueOffset, false, objects);
                    if (found) {
                        CFPropertyListRef result = NULL;
                        success = __CFBinaryPlistCreateObjectFiltered(databytes, datalen, valueOffset, trailer, allocator, mutabilityOption, objects, set, curDepth + 1, nextKeys, outPlist ? &result : NULL, NULL);
                        if (success) {
                            if (result) {
                                CFDictionarySetValue(dict, key, result);
                                CFRelease(result);
                            }
                        } else {
                            break;
                        }
                    }
                }
                
                free(keys);
                CFRelease(theseKeys);
            }
            if (nextKeys) CFRelease(nextKeys);
            
            if (success && outPlist) {
                if (!(mutabilityOption == kCFPropertyListMutableContainers || mutabilityOption == kCFPropertyListMutableContainersAndLeaves)) {
                    // make immutable
                    *outPlist = CFDictionaryCreateCopy(allocator, dict);
                    CFRelease(dict);
                } else {
                    *outPlist = dict;
                }
            } else if (dict) {
                CFRelease(dict);
            }
        } else {
            CFIndex const halfDictionaryCount = dictionaryCount / 2;
            for (CFIndex idx = 0; idx < dictionaryCount; idx++) {
                if (!_getOffsetOfRefAt(databytes, ptr, trailer, &off)) {
                    if (list) {
                        while (idx--) {
                            CFRelease(list[idx]);
                        }
                        if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
                    }
                    FAIL_FALSE;
                }
                CFPropertyListRef pl = NULL;
                CFTypeID typeID = _kCFRuntimeNotATypeID;
                if (!__CFBinaryPlistCreateObjectFiltered(databytes, datalen, off, trailer, allocator, mutabilityOption, objects, set, curDepth + 1, NULL, (outPlist ? &pl : NULL), &typeID) || (idx < halfDictionaryCount && !_typeIsPlistPrimitive(typeID))) {
                    if (pl) CFRelease(pl);
                    if (list) {
                        while (idx--) {
                            CFRelease(list[idx]);
                        }
                        if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
                    }
                    FAIL_FALSE;
                }
                if (list) {
                    *((void **)list + idx) = (void *)pl;
#if __clang_analyzer__
                    // The static analyzer can't reason that we're always looping through this an even number of times. It thinks list[idx + halfDictionaryCount] below will be uninitialized.
                    if (idx % 2 == 0) {
                        *((void **)list + halfDictionaryCount) = NULL;
                    }
#endif
                }
                ptr += trailer->_objectRefSize;
            }
            if (outPlist) {
                if (mutabilityOption != kCFPropertyListImmutable) {
                    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(allocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                    for (CFIndex idx = 0; idx < halfDictionaryCount; idx++) {
                        CFDictionaryAddValue((CFMutableDictionaryRef)dict, list[idx], list[idx + halfDictionaryCount]);
                    }
                    for (CFIndex idx = 0; idx < dictionaryCount; idx++) {
                        CFRelease(list[idx]);
                    }
                    *outPlist = dict;
                } else {
                    *outPlist = __CFDictionaryCreateTransfer(allocator, list, list + halfDictionaryCount, halfDictionaryCount);
                }
            }
            if (outPlistTypeID) *outPlistTypeID = _kCFRuntimeIDCFDictionary;
        }
        if (set) CFSetRemoveValue(set, (const void *)(uintptr_t)startOffset);
        if (objects && success && outPlist && (mutabilityOption == kCFPropertyListImmutable)) {
            CFDictionarySetValue(objects, (const void *)(uintptr_t)startOffset, *outPlist);
	}
	if (list && list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list);
	return success;
	}
    }
    FAIL_FALSE;
}

bool __CFBinaryPlistCreateObject(const uint8_t *databytes, uint64_t datalen, uint64_t startOffset, const CFBinaryPlistTrailer *trailer, CFAllocatorRef allocator, CFOptionFlags mutabilityOption, CFMutableDictionaryRef objects, CFPropertyListRef *plist) {
	// for compatibility with Foundation's use, need to leave this here
    return __CFBinaryPlistCreateObjectFiltered(databytes, datalen, startOffset, trailer, allocator, mutabilityOption, objects, NULL, 0, NULL, plist, NULL);
}

CF_PRIVATE bool __CFTryParseBinaryPlist(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags option, CFPropertyListRef *plist, CFStringRef *errorString) {
    uint8_t marker;    
    CFBinaryPlistTrailer trailer;
    uint64_t offset;
    const uint8_t *databytes = CFDataGetBytePtr(data);
    uint64_t datalen = CFDataGetLength(data);

    if (8 <= datalen && __CFBinaryPlistGetTopLevelInfo(databytes, datalen, &marker, &offset, &trailer)) {
	// FALSE: We know for binary plist parsing that the result objects will be retained
	// by their containing collections as the parsing proceeds, so we do not need
	// to use retaining callbacks for the objects map in this case. WHY: the file might
	// be malformed and contain hash-equal keys for the same dictionary (for example)
	// and the later key will cause the previous one to be released when we set the second
	// in the dictionary.
	CFMutableDictionaryRef objects = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, NULL, &kCFTypeDictionaryValueCallBacks);
	_CFDictionarySetCapacity(objects, trailer._numObjects);
	CFPropertyListRef pl = NULL;
        bool result = true;
        if (__CFBinaryPlistCreateObjectFiltered(databytes, datalen, offset, &trailer, allocator, option, objects, NULL, 0, NULL, &pl, NULL)) {
	    if (plist) *plist = pl;
#if 0
// code to check the 1.5 version code against any binary plist successfully parsed above
extern size_t __CFBinaryPlistWrite15(CFPropertyListRef plist, CFMutableDataRef data, CFErrorRef *error);
extern CFPropertyListRef __CFBinaryPlistCreate15(const uint8_t *databytes, uint64_t datalen, CFErrorRef *error);

CFMutableDataRef mdata = CFDataCreateMutable(0, 0);
size_t s = __CFBinaryPlistWrite15(pl, mdata, NULL);
//double ratio = (double)s / (double)datalen;
//if (ratio < 0.75 || ratio > 4.0) CFLog(4, CFSTR("@@@ note: Binary plist of %ld bytes is %ld bytes (%f) in version 1.5"), datalen, s, ratio);
if (s != CFDataGetLength(mdata)) CFLog(3, CFSTR("### error: returned length not equal to data length (%ld != %ld)"), s, CFDataGetLength(mdata));
CFPropertyListRef pl2 = __CFBinaryPlistCreate15((const uint8_t *)CFDataGetBytePtr(mdata), CFDataGetLength(mdata), NULL);
if (!CFEqual(pl, pl2)) CFLog(3, CFSTR("*** error: plists before and after are not equal\n--------\n%@\n--------\n%@\n--------"), pl, pl2);
#endif
        } else {
	    if (plist) *plist = NULL;
            if (errorString) *errorString = (CFStringRef)CFRetain(CFSTR("binary data is corrupt"));
            result = false;
	}
        CFRelease(objects);
        return result;
    }
    FAIL_FALSE;
}

