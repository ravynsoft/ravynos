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

/*	CFUUID.c
	Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
	Responsibility: David Smith
*/

#include <CoreFoundation/CFUUID.h>
#include "CFInternal.h"

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_WINDOWS
#include <dispatch/dispatch.h>

static CFMutableDictionaryRef _uniquedUUIDs = NULL;
CF_INLINE void LOCKED(dispatch_block_t work) {
    static dispatch_once_t guard;
    static dispatch_queue_t CFUUIDGlobalDataLock;
    dispatch_once(&guard, ^{
        dispatch_queue_attr_t dqattr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, qos_class_main(), 0);
        CFUUIDGlobalDataLock = dispatch_queue_create("com.apple.CFUUID", dqattr);
    });
    dispatch_sync(CFUUIDGlobalDataLock, work);
}

#else
// Platforms without dispatch

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

static Boolean __CFisEqualUUIDBytes(const void *ptr1, const void *ptr2) {
    CFUUIDBytes *p1 = (CFUUIDBytes *)ptr1;
    CFUUIDBytes *p2 = (CFUUIDBytes *)ptr2;

    return (((p1->byte0 == p2->byte0) && (p1->byte1 == p2->byte1) && (p1->byte2 == p2->byte2) && (p1->byte3 == p2->byte3) && (p1->byte4 == p2->byte4) && (p1->byte5 == p2->byte5) && (p1->byte6 == p2->byte6) && (p1->byte7 == p2->byte7) && (p1->byte8 == p2->byte8) && (p1->byte9 == p2->byte9) && (p1->byte10 == p2->byte10) && (p1->byte11 == p2->byte11) && (p1->byte12 == p2->byte12) && (p1->byte13 == p2->byte13) && (p1->byte14 == p2->byte14) && (p1->byte15 == p2->byte15)) ? true : false);
}

static CFHashCode __CFhashUUIDBytes(const void *ptr) {
    return CFHashBytes((uint8_t *)ptr, 16);
}

/*
 * GC implementation of a weak set specifically designed for UUID
 */

#define MALLOC(x) CFAllocatorAllocate(kCFAllocatorSystemDefault, x, 0)
#define FREE(x) CFAllocatorDeallocate(kCFAllocatorSystemDefault, x)
#define HASH(x) CFHashBytes((uint8_t *)x, 16)

#define READWEAK(location) objc_read_weak((id *)location)
#define WRITEWEAK(location, value) objc_assign_weak((id)value, (id *)location)

typedef struct {
    unsigned long count, size;
    __CFUUID_t **weakPtrs;
} _UUIDWeakSet_t;

static _UUIDWeakSet_t _UUIDWeakSet;

static void grow_has_lock(void);

// enter if not already present
static void enter_has_lock(__CFUUID_t *candidate) {
    if (!candidate) return;
    _UUIDWeakSet_t *table = &_UUIDWeakSet;
    if (!table->size) grow_has_lock();
    unsigned long int hashValue = HASH(&candidate->_bytes) & (table->size-1);
    __CFUUID_t *result = table->weakPtrs[hashValue];
    while (1) {
        if (result == (void *)0x1 || result == NULL) {
            table->weakPtrs[hashValue] = NULL;  // so that we don't try to unregister 0x1
            WRITEWEAK(&table->weakPtrs[hashValue], (void *)candidate);
            ++table->count;
            break;
        }
        if (result) result = (__CFUUID_t *)READWEAK(&table->weakPtrs[hashValue]);
        if (result) {
            // see if it is equal to candidate
            if (__CFisEqualUUIDBytes(&result->_bytes, &candidate->_bytes)) {
                // keep first one.  There is a race if two threads both fail to find
                // a candidate uuid then both try decide to create and enter one.
                // Under non-GC one of them simply leaks.
                break;
            }
        } else {
            // was zeroed by collector.  Use this slot.
            continue;
        }
        // move on
        if (++hashValue >= table->size) hashValue = 0;
        result = table->weakPtrs[hashValue];
    }
}

static void *find_has_lock(const CFUUIDBytes *bytes) {
    if (!bytes) return NULL;
    _UUIDWeakSet_t *table = &_UUIDWeakSet;
    if (!table->size) return NULL;  // no entries
    unsigned long int hashValue = HASH(bytes) & (table->size-1);
    __CFUUID_t *result = table->weakPtrs[hashValue];
    while (1) {
        if (result == (void *)0x1) break;
        if (result) result = (__CFUUID_t *)READWEAK(&table->weakPtrs[hashValue]);
        if (result) {
            // see if it is equal to bytes
            if (__CFisEqualUUIDBytes(&result->_bytes, bytes)) return result;
        }
        // move on
        if (++hashValue >= table->size) hashValue = 0;
        result = table->weakPtrs[hashValue];
    }
    return NULL;
}


static void grow_has_lock() {
    _UUIDWeakSet_t *table = &_UUIDWeakSet;
    if (table->size == 0) {
        table->size = 16;
        table->weakPtrs = (__CFUUID_t **)MALLOC(sizeof(__CFUUID_t *)*table->size);
        for (int i = 0; i < table->size; ++i) table->weakPtrs[i] = (__CFUUID_t *)0x1;
        table->count = 0;
        return;
    }
    table->count = 0;
    table->size = table->size*2;
    __CFUUID_t **oldPtrs = table->weakPtrs;
    table->weakPtrs = (__CFUUID_t **)MALLOC(sizeof(__CFUUID_t *)*table->size);
    for (int i = 0; i < table->size; ++i) table->weakPtrs[i] = (__CFUUID_t *)0x1;
    for (int i = 0; i < table->size / 2; ++i) {
        if (oldPtrs[i] == (__CFUUID_t *)0x1) continue; // available field, ignore
        if (oldPtrs[i] == NULL) continue;  // zero'ed by collector, ignore
        enter_has_lock((__CFUUID_t *)READWEAK(&oldPtrs[i]));  // read, then enter (but enter must check for NULL)
        WRITEWEAK(&oldPtrs[i], NULL);  // unregister
    }
    FREE(oldPtrs);
}

/***** end of weak set */

static void __CFUUIDAddUniqueUUIDHasLock(CFUUIDRef uuid) {
    CFDictionaryKeyCallBacks __CFUUIDBytesDictionaryKeyCallBacks = {0, NULL, NULL, NULL, __CFisEqualUUIDBytes, __CFhashUUIDBytes};
    CFDictionaryValueCallBacks __CFnonRetainedUUIDDictionaryValueCallBacks = {0, NULL, NULL, CFCopyDescription, CFEqual};

    if (kCFUseCollectableAllocator) {
        enter_has_lock((__CFUUID_t *)uuid);
        if (_UUIDWeakSet.count > (3 * _UUIDWeakSet.size / 4)) grow_has_lock();
    } else {
        if (!_uniquedUUIDs) _uniquedUUIDs = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &__CFUUIDBytesDictionaryKeyCallBacks, &__CFnonRetainedUUIDDictionaryValueCallBacks);
        CFDictionarySetValue(_uniquedUUIDs, &(uuid->_bytes), uuid);
    }
}

static void __CFUUIDRemoveUniqueUUIDHasLock(CFUUIDRef uuid) {
    if (_uniquedUUIDs) CFDictionaryRemoveValue(_uniquedUUIDs, &(uuid->_bytes));
}

static CFUUIDRef __CFUUIDGetUniquedUUIDHasLock(const CFUUIDBytes *bytes) {
    CFUUIDRef uuid = NULL;
    if (kCFUseCollectableAllocator) {
        uuid = (CFUUIDRef)find_has_lock(bytes);
    } else if (_uniquedUUIDs) {
        uuid = (CFUUIDRef)CFDictionaryGetValue(_uniquedUUIDs, bytes);
    }
    return uuid;
}

static void __CFUUIDDeallocate(CFTypeRef cf) {
    if (kCFUseCollectableAllocator) return;
    
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

static CFTypeID __kCFUUIDTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFUUIDClass = {
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
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFUUIDTypeID = _CFRuntimeRegisterClass(&__CFUUIDClass); });
    return __kCFUUIDTypeID;
}

static CFUUIDRef __CFUUIDCreateWithBytesPrimitive(CFAllocatorRef allocator, CFUUIDBytes bytes, Boolean isConst) {
    __block __CFUUID_t *uuid = NULL;
    LOCKED(^{
        uuid = (__CFUUID_t *)__CFUUIDGetUniquedUUIDHasLock(&bytes);
        if (!uuid) {
            size_t size;
            size = sizeof(__CFUUID_t) - sizeof(CFRuntimeBase);
            uuid = (__CFUUID_t *)_CFRuntimeCreateInstance(kCFUseCollectableAllocator ? kCFAllocatorSystemDefault : allocator, CFUUIDGetTypeID(), size, NULL);
            
            if (!uuid) return;
            
            uuid->_bytes = bytes;
            __CFUUIDAddUniqueUUIDHasLock(uuid);
        } else if (!isConst) {
            CFRetain(uuid);
        }
    });

    return (CFUUIDRef)uuid;
}

#if DEPLOYMENT_TARGET_WINDOWS
#include <Rpc.h>
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#include <uuid/uuid.h>
#endif

CFUUIDRef CFUUIDCreate(CFAllocatorRef alloc) {
    /* Create a new bytes struct and then call the primitive. */
    __block CFUUIDBytes bytes;
    __block uint32_t retval = 0;
    
    LOCKED(^{
#if DEPLOYMENT_TARGET_WINDOWS
        UUID u;
        long rStatus = UuidCreate(&u);
        if (RPC_S_OK != rStatus && RPC_S_UUID_LOCAL_ONLY != rStatus) retval = 1;
        memmove(&bytes, &u, sizeof(bytes));
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
        static Boolean useV1UUIDs = false, checked = false;
        uuid_t uuid;
        if (!checked) {
            const char *value = __CFgetenv("CFUUIDVersionNumber");
            if (value) {
                if (1 == strtoul_l(value, NULL, 0, NULL)) useV1UUIDs = true;
            }
            checked = true;
        }
        if (useV1UUIDs) uuid_generate_time(uuid); else uuid_generate_random(uuid);
        memcpy((void *)&bytes, uuid, sizeof(uuid));
#else
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

    return __CFUUIDCreateWithBytesPrimitive(alloc, bytes, true);
}

CFUUIDBytes CFUUIDGetUUIDBytes(CFUUIDRef uuid) {
    return uuid->_bytes;
}

CF_EXPORT CFUUIDRef CFUUIDCreateFromUUIDBytes(CFAllocatorRef alloc, CFUUIDBytes bytes) {
    return __CFUUIDCreateWithBytesPrimitive(alloc, bytes, false);
}

#undef READ_A_BYTE

