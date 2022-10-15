/*	CFUUID.c
	Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Ben D. Jones
*/

#include <CoreFoundation/CFUUID.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"

#if __has_include(<os/lock_private.h>)
#include <os/lock_private.h>

static CFMutableDictionaryRef _uniquedUUIDs = NULL;
static os_unfair_lock _uniquedUUIDsLock = OS_UNFAIR_LOCK_INIT;

CF_INLINE void LOCKED(dispatch_block_t work) {
    os_unfair_lock_lock_with_options(&_uniquedUUIDsLock, OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
    work();
    os_unfair_lock_unlock(&_uniquedUUIDsLock);
}

#else
// Platforms without unfair lock
static CFMutableDictionaryRef _uniquedUUIDs = NULL;
static CFLock_t _uniquedUUIDsLock = CFLockInit;

CF_INLINE void LOCKED(void (^work)(void)) {
    __CFLock(&_uniquedUUIDsLock);
    work();
    __CFUnlock(&_uniquedUUIDsLock);
}

#endif

struct __CFUUID {
    CFRuntimeBase _base;
    CFUUIDBytes _bytes;
};

typedef struct __CFUUID __CFUUID_t;

static CFHashCode __CFhashUUIDBytes(const void *ptr) {
    return CFHashBytes((uint8_t *)ptr, 16);
}

static void __CFUUIDAddUniqueUUIDHasLock(CFUUIDRef uuid) {
    CFDictionaryKeyCallBacks __CFUUIDBytesDictionaryKeyCallBacks = {0, NULL, NULL, NULL, __CFisEqualUUIDBytes, __CFhashUUIDBytes};
    CFDictionaryValueCallBacks __CFnonRetainedUUIDDictionaryValueCallBacks = {0, NULL, NULL, CFCopyDescription, CFEqual};

    if (!_uniquedUUIDs) _uniquedUUIDs = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &__CFUUIDBytesDictionaryKeyCallBacks, &__CFnonRetainedUUIDDictionaryValueCallBacks);
    CFDictionarySetValue(_uniquedUUIDs, &(uuid->_bytes), uuid);
}

static void __CFUUIDRemoveUniqueUUIDHasLock(CFUUIDRef uuid) {
    if (_uniquedUUIDs) CFDictionaryRemoveValue(_uniquedUUIDs, &(uuid->_bytes));
}

static CFUUIDRef __CFUUIDGetUniquedUUIDHasLock(const CFUUIDBytes *bytes) {
    CFUUIDRef uuid = NULL;
    if (_uniquedUUIDs) {
        uuid = (CFUUIDRef)CFDictionaryGetValue(_uniquedUUIDs, bytes);
    }
    return uuid;
}

static void __CFUUIDDeallocate(CFTypeRef cf) {    
    __CFUUID_t *uuid = (__CFUUID_t *)cf;
    LOCKED(^{
    __CFUUIDRemoveUniqueUUIDHasLock(uuid);
    });
}

static CFStringRef __CFUUIDCopyDescription(CFTypeRef cf) {
    CFStringRef uuidStr = CFUUIDCreateString(CFGetAllocator(cf), (CFUUIDRef)cf);
    CFStringRef desc = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFUUID %p> %@"), cf, uuidStr);
    CFRelease(uuidStr);
    return desc;
}

static CFStringRef __CFUUIDCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions) {
    return CFUUIDCreateString(CFGetAllocator(cf), (CFUUIDRef)cf);
}

const CFRuntimeClass __CFUUIDClass = {
    0,
    "CFUUID",
    NULL,	// init
    NULL,	// copy
    __CFUUIDDeallocate,
    NULL,	// equal
    NULL,	// hash
    __CFUUIDCopyFormattingDescription,
    __CFUUIDCopyDescription
};

CFTypeID CFUUIDGetTypeID(void) {
    return _kCFRuntimeIDCFUUID;
}

static CFUUIDRef __CFUUIDCreateWithBytesPrimitive(CFAllocatorRef allocator, CFUUIDBytes bytes, Boolean isConst) {
    __block __CFUUID_t *uuid = NULL;
    LOCKED(^{
        uuid = (__CFUUID_t *)__CFUUIDGetUniquedUUIDHasLock(&bytes);
        if (!uuid) {
            size_t size;
            size = sizeof(__CFUUID_t) - sizeof(CFRuntimeBase);
            uuid = (__CFUUID_t *)_CFRuntimeCreateInstance(allocator, CFUUIDGetTypeID(), size, NULL);
            
            if (!uuid) return;
            
            uuid->_bytes = bytes;
            __CFUUIDAddUniqueUUIDHasLock(uuid);
        } else if (!isConst) {
            CFRetain(uuid);
        }
        
        if (isConst) {
#if !DEPLOYMENT_RUNTIME_SWIFT
            __CFRuntimeSetRC(uuid, 0); // constant CFUUIDs should be immortal. This applies even to equivalent UUIDs created earlier that were *not* constant.
#else
            CFRetain(uuid); // Swift doesn't support meddling with the retain count. Just ensure there is one retain here.
#endif
        }
    });

    return (CFUUIDRef)uuid;
}

#if TARGET_OS_WIN32
#include <Rpc.h>
#else
#if DEPLOYMENT_RUNTIME_SWIFT
#include "uuid/uuid.h"
#else
#include <uuid/uuid.h>
#endif
#endif

CFUUIDRef CFUUIDCreate(CFAllocatorRef alloc) {
    /* Create a new bytes struct and then call the primitive. */
    __block CFUUIDBytes bytes;
    __block uint32_t retval = 0;
    
    LOCKED(^{
#if TARGET_OS_WIN32
        UUID u;
        long rStatus = UuidCreate(&u);
        if (RPC_S_OK != rStatus && RPC_S_UUID_LOCAL_ONLY != rStatus) retval = 1;
        memmove(&bytes, &u, sizeof(bytes));
#elif TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD
        static int8_t useV1UUIDs = -1;
        uuid_t uuid;
        if (useV1UUIDs == -1) {
            const char *value = __CFgetenv("CFUUIDVersionNumber");
            if (value) {
                useV1UUIDs = (1 == strtoul_l(value, NULL, 0, NULL)) ? 1 : 0;
            }
        }
        if (useV1UUIDs == 1) uuid_generate_time(uuid); else uuid_generate_random(uuid);
        memcpy((void *)&bytes, uuid, sizeof(uuid));
#else
        //This bzero works around <rdar://problem/23381916>. It isn't actually needed, since the function will simply return NULL on this deployment target, anyway.
        bzero(&bytes, sizeof(bytes));
        retval = 1;
#endif
    });

    return (retval == 0) ? __CFUUIDCreateWithBytesPrimitive(alloc, bytes, false) : NULL;
}

CFUUIDRef CFUUIDCreateWithBytes(CFAllocatorRef alloc, uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8, uint8_t byte9, uint8_t byte10, uint8_t byte11, uint8_t byte12, uint8_t byte13, uint8_t byte14, uint8_t byte15) {
    CFUUIDBytes bytes;
    // CodeWarrior can't handle the structure assignment of bytes, so we must explode this - REW, 10/8/99
    bytes.byte0 = byte0;
    bytes.byte1 = byte1;
    bytes.byte2 = byte2;
    bytes.byte3 = byte3;
    bytes.byte4 = byte4;
    bytes.byte5 = byte5;
    bytes.byte6 = byte6;
    bytes.byte7 = byte7;
    bytes.byte8 = byte8;
    bytes.byte9 = byte9;
    bytes.byte10 = byte10;
    bytes.byte11 = byte11;
    bytes.byte12 = byte12;
    bytes.byte13 = byte13;
    bytes.byte14 = byte14;
    bytes.byte15 = byte15;

    return __CFUUIDCreateWithBytesPrimitive(alloc, bytes, false);
}

static void _intToHexChars(UInt32 in, UniChar *out, int digits) {
    int shift;
    UInt32 d;

    while (--digits >= 0) {
        shift = digits << 2;
        d = 0x0FL & (in >> shift);
        if (d <= 9) {
            *out++ = (UniChar)'0' + d;
        } else {
            *out++ = (UniChar)'A' + (d - 10);
        }
    }
}

static uint8_t _byteFromHexChars(UniChar *in) {
    uint8_t result = 0;
    UniChar c;
    uint8_t d;
    CFIndex i;

    for (i=0; i<2; i++) {
        c = in[i];
        if ((c >= (UniChar)'0') && (c <= (UniChar)'9')) {
            d = c - (UniChar)'0';
        } else if ((c >= (UniChar)'a') && (c <= (UniChar)'f')) {
            d = c - ((UniChar)'a' - 10);
        } else if ((c >= (UniChar)'A') && (c <= (UniChar)'F')) {
            d = c - ((UniChar)'A' - 10);
        } else {
            return 0;
        }
        result = (result << 4) | d;
    }
    
    return result;
}

CF_INLINE Boolean _isHexChar(UniChar c) {
    return ((((c >= (UniChar)'0') && (c <= (UniChar)'9')) || ((c >= (UniChar)'a') && (c <= (UniChar)'f')) || ((c >= (UniChar)'A') && (c <= (UniChar)'F'))) ? true : false);
}

#define READ_A_BYTE(into) if (i+1 < len) { \
    (into) = _byteFromHexChars(&(chars[i])); \
        i+=2; \
}

CFUUIDRef CFUUIDCreateFromString(CFAllocatorRef alloc, CFStringRef uuidStr) {
    /* Parse the string into a bytes struct and then call the primitive. */
    CFUUIDBytes bytes;
    UniChar chars[100];
    CFIndex len;
    CFIndex i = 0;
    
    if (uuidStr == NULL) return NULL;

    len = CFStringGetLength(uuidStr);
    if (len > 100) {
        len = 100;
    } else if (len == 0) {
        return NULL;
    }
    CFStringGetCharacters(uuidStr, CFRangeMake(0, len), chars);
    memset((void *)&bytes, 0, sizeof(bytes));

    /* Skip initial random stuff */
    while (!_isHexChar(chars[i]) && i < len) i++;

    READ_A_BYTE(bytes.byte0);
    READ_A_BYTE(bytes.byte1);
    READ_A_BYTE(bytes.byte2);
    READ_A_BYTE(bytes.byte3);
    i++;

    READ_A_BYTE(bytes.byte4);
    READ_A_BYTE(bytes.byte5);
    i++;

    READ_A_BYTE(bytes.byte6);
    READ_A_BYTE(bytes.byte7);
    i++;

    READ_A_BYTE(bytes.byte8);
    READ_A_BYTE(bytes.byte9);
    i++;

    READ_A_BYTE(bytes.byte10);
    READ_A_BYTE(bytes.byte11);
    READ_A_BYTE(bytes.byte12);
    READ_A_BYTE(bytes.byte13);
    READ_A_BYTE(bytes.byte14);
    READ_A_BYTE(bytes.byte15);

    return __CFUUIDCreateWithBytesPrimitive(alloc, bytes, false);
}

CFStringRef CFUUIDCreateString(CFAllocatorRef alloc, CFUUIDRef uuid) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFUUID, uuid);
    
    CFMutableStringRef str = CFStringCreateMutable(alloc, 0);
    UniChar buff[12];

    // First segment (4 bytes, 8 digits + 1 dash)
    _intToHexChars(uuid->_bytes.byte0, buff, 2);
    _intToHexChars(uuid->_bytes.byte1, &(buff[2]), 2);
    _intToHexChars(uuid->_bytes.byte2, &(buff[4]), 2);
    _intToHexChars(uuid->_bytes.byte3, &(buff[6]), 2);
    buff[8] = (UniChar)'-';
    CFStringAppendCharacters(str, buff, 9);

    // Second segment (2 bytes, 4 digits + 1 dash)
    _intToHexChars(uuid->_bytes.byte4, buff, 2);
    _intToHexChars(uuid->_bytes.byte5, &(buff[2]), 2);
    buff[4] = (UniChar)'-';
    CFStringAppendCharacters(str, buff, 5);

    // Third segment (2 bytes, 4 digits + 1 dash)
    _intToHexChars(uuid->_bytes.byte6, buff, 2);
    _intToHexChars(uuid->_bytes.byte7, &(buff[2]), 2);
    buff[4] = (UniChar)'-';
    CFStringAppendCharacters(str, buff, 5);

    // Fourth segment (2 bytes, 4 digits + 1 dash)
    _intToHexChars(uuid->_bytes.byte8, buff, 2);
    _intToHexChars(uuid->_bytes.byte9, &(buff[2]), 2);
    buff[4] = (UniChar)'-';
    CFStringAppendCharacters(str, buff, 5);

    // Fifth segment (6 bytes, 12 digits)
    _intToHexChars(uuid->_bytes.byte10, buff, 2);
    _intToHexChars(uuid->_bytes.byte11, &(buff[2]), 2);
    _intToHexChars(uuid->_bytes.byte12, &(buff[4]), 2);
    _intToHexChars(uuid->_bytes.byte13, &(buff[6]), 2);
    _intToHexChars(uuid->_bytes.byte14, &(buff[8]), 2);
    _intToHexChars(uuid->_bytes.byte15, &(buff[10]), 2);
    CFStringAppendCharacters(str, buff, 12);

    return str;
}

CFUUIDRef CFUUIDGetConstantUUIDWithBytes(CFAllocatorRef alloc, uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8, uint8_t byte9, uint8_t byte10, uint8_t byte11, uint8_t byte12, uint8_t byte13, uint8_t byte14, uint8_t byte15) {
    CFUUIDBytes bytes;
    // CodeWarrior can't handle the structure assignment of bytes, so we must explode this - REW, 10/8/99
    bytes.byte0 = byte0;
    bytes.byte1 = byte1;
    bytes.byte2 = byte2;
    bytes.byte3 = byte3;
    bytes.byte4 = byte4;
    bytes.byte5 = byte5;
    bytes.byte6 = byte6;
    bytes.byte7 = byte7;
    bytes.byte8 = byte8;
    bytes.byte9 = byte9;
    bytes.byte10 = byte10;
    bytes.byte11 = byte11;
    bytes.byte12 = byte12;
    bytes.byte13 = byte13;
    bytes.byte14 = byte14;
    bytes.byte15 = byte15;

    // The analyzer can't understand functions like __CFUUIDCreateWithBytesPrimitive which return retained objects based on a parameter.
#ifdef __clang_analyzer__
    return NULL;
#else
    return __CFUUIDCreateWithBytesPrimitive(alloc, bytes, true);
#endif
}

CFUUIDBytes CFUUIDGetUUIDBytes(CFUUIDRef uuid) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFUUID, uuid);
    return uuid->_bytes;
}

CF_EXPORT CFUUIDRef CFUUIDCreateFromUUIDBytes(CFAllocatorRef alloc, CFUUIDBytes bytes) {
    return __CFUUIDCreateWithBytesPrimitive(alloc, bytes, false);
}

#undef READ_A_BYTE

#if DEPLOYMENT_RUNTIME_SWIFT

void _cf_uuid_clear(_cf_uuid_t uu) { uuid_clear(uu); }
int _cf_uuid_compare(const _cf_uuid_t uu1, const _cf_uuid_t uu2) { return uuid_compare(uu1, uu2); }
void _cf_uuid_copy(_cf_uuid_t dst, const _cf_uuid_t src) { uuid_copy(dst, src); }
void _cf_uuid_generate(_cf_uuid_t out) { uuid_generate(out); }
void _cf_uuid_generate_random(_cf_uuid_t out) { uuid_generate_random(out); }
void _cf_uuid_generate_time(_cf_uuid_t out) { uuid_generate_time(out); }
int _cf_uuid_is_null(const _cf_uuid_t uu) { return uuid_is_null(uu); }
int _cf_uuid_parse(const _cf_uuid_string_t in, _cf_uuid_t uu) { return uuid_parse(in, uu); }
void _cf_uuid_unparse(const _cf_uuid_t uu, _cf_uuid_string_t out) { uuid_unparse(uu, out); }
void _cf_uuid_unparse_lower(const _cf_uuid_t uu, _cf_uuid_string_t out) { uuid_unparse_lower(uu, out); }
void _cf_uuid_unparse_upper(const _cf_uuid_t uu, _cf_uuid_string_t out) { uuid_unparse_upper(uu, out); }

#endif
