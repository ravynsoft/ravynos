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

/*	CFBasicHash.h
	Copyright (c) 2008-2014, Apple Inc. All rights reserved.
*/

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include "CFInternal.h"

CF_EXTERN_C_BEGIN

struct __objcFastEnumerationStateEquivalent2 {
    unsigned long state;
    unsigned long *itemsPtr;
    unsigned long *mutationsPtr;
    unsigned long extra[5];
};

enum {
    __kCFBasicHashLinearHashingValue = 1,
    __kCFBasicHashDoubleHashingValue = 2,
    __kCFBasicHashExponentialHashingValue = 3,
};

enum {
    kCFBasicHashHasKeys = (1UL << 0),
    kCFBasicHashHasCounts = (1UL << 1),
    kCFBasicHashHasHashCache = (1UL << 2),

    kCFBasicHashIntegerValues = (1UL << 6),
    kCFBasicHashIntegerKeys = (1UL << 7),

    kCFBasicHashStrongValues = (1UL << 8),
    kCFBasicHashStrongKeys = (1UL << 9),

    kCFBasicHashWeakValues = (1UL << 10),
    kCFBasicHashWeakKeys = (1UL << 11),

    kCFBasicHashIndirectKeys = (1UL << 12),

    kCFBasicHashLinearHashing = (__kCFBasicHashLinearHashingValue << 13), // bits 13-14
    kCFBasicHashDoubleHashing = (__kCFBasicHashDoubleHashingValue << 13),
    kCFBasicHashExponentialHashing = (__kCFBasicHashExponentialHashingValue << 13),

    kCFBasicHashAggressiveGrowth = (1UL << 15),
};

// Note that for a hash table without keys, the value is treated as the key,
// and the value should be passed in as the key for operations which take a key.

typedef struct {
    CFIndex idx;
    uintptr_t weak_key;
    uintptr_t weak_value;
    uintptr_t count;
} CFBasicHashBucket;

typedef struct __CFBasicHash *CFBasicHashRef;
typedef const struct __CFBasicHash *CFConstBasicHashRef;

// Bit 6 in the CF_INFO_BITS of the CFRuntimeBase inside the CFBasicHashRef is the "is immutable" bit
CF_INLINE Boolean CFBasicHashIsMutable(CFConstBasicHashRef ht) {
    return __CFBitfieldGetValue(((CFRuntimeBase *)ht)->_cfinfo[CF_INFO_BITS], 6, 6) ? false : true;
}

CF_INLINE void CFBasicHashMakeImmutable(CFBasicHashRef ht) {
    __CFBitfieldSetValue(((CFRuntimeBase *)ht)->_cfinfo[CF_INFO_BITS], 6, 6, 1);
}


typedef struct __CFBasicHashCallbacks CFBasicHashCallbacks;

struct __CFBasicHashCallbacks {
    uintptr_t (*retainValue)(CFAllocatorRef alloc, uintptr_t stack_value);	// Return 2nd arg or new value
    uintptr_t (*retainKey)(CFAllocatorRef alloc, uintptr_t stack_key);	// Return 2nd arg or new key
    void (*releaseValue)(CFAllocatorRef alloc, uintptr_t stack_value);
    void (*releaseKey)(CFAllocatorRef alloc, uintptr_t stack_key);
    Boolean (*equateValues)(uintptr_t coll_value1, uintptr_t stack_value2); // 1st arg is in-collection value, 2nd arg is probe parameter OR in-collection value for a second collection
    Boolean (*equateKeys)(uintptr_t coll_key1, uintptr_t stack_key2); // 1st arg is in-collection key, 2nd arg is probe parameter
    CFHashCode (*hashKey)(uintptr_t stack_key);
    uintptr_t (*getIndirectKey)(uintptr_t coll_value);	// Return key; 1st arg is in-collection value
    CFStringRef (*copyValueDescription)(uintptr_t stack_value);
    CFStringRef (*copyKeyDescription)(uintptr_t stack_key);
};

Boolean CFBasicHashHasStrongValues(CFConstBasicHashRef ht);
Boolean CFBasicHashHasStrongKeys(CFConstBasicHashRef ht);

CFOptionFlags CFBasicHashGetFlags(CFConstBasicHashRef ht);
CFIndex CFBasicHashGetNumBuckets(CFConstBasicHashRef ht);
CFIndex CFBasicHashGetCapacity(CFConstBasicHashRef ht);
void CFBasicHashSetCapacity(CFBasicHashRef ht, CFIndex capacity);

CFIndex CFBasicHashGetCount(CFConstBasicHashRef ht);
CFBasicHashBucket CFBasicHashGetBucket(CFConstBasicHashRef ht, CFIndex idx);
CFBasicHashBucket CFBasicHashFindBucket(CFConstBasicHashRef ht, uintptr_t stack_key);
CFIndex CFBasicHashGetCountOfKey(CFConstBasicHashRef ht, uintptr_t stack_key);
CFIndex CFBasicHashGetCountOfValue(CFConstBasicHashRef ht, uintptr_t stack_value);
Boolean CFBasicHashesAreEqual(CFConstBasicHashRef ht1, CFConstBasicHashRef ht2);
void CFBasicHashApply(CFConstBasicHashRef ht, Boolean (^block)(CFBasicHashBucket));
void CFBasicHashApplyIndexed(CFConstBasicHashRef ht, CFRange range, Boolean (^block)(CFBasicHashBucket));
void CFBasicHashGetElements(CFConstBasicHashRef ht, CFIndex bufferslen, uintptr_t *weak_values, uintptr_t *weak_keys);

Boolean CFBasicHashAddValue(CFBasicHashRef ht, uintptr_t stack_key, uintptr_t stack_value);
void CFBasicHashReplaceValue(CFBasicHashRef ht, uintptr_t stack_key, uintptr_t stack_value);
void CFBasicHashSetValue(CFBasicHashRef ht, uintptr_t stack_key, uintptr_t stack_value);
CFIndex CFBasicHashRemoveValue(CFBasicHashRef ht, uintptr_t stack_key);
CFIndex CFBasicHashRemoveValueAtIndex(CFBasicHashRef ht, CFIndex idx);
void CFBasicHashRemoveAllValues(CFBasicHashRef ht);

Boolean CFBasicHashAddIntValueAndInc(CFBasicHashRef ht, uintptr_t stack_key, uintptr_t int_value);
void CFBasicHashRemoveIntValueAndDec(CFBasicHashRef ht, uintptr_t int_value);

size_t CFBasicHashGetSize(CFConstBasicHashRef ht, Boolean total);
void CFBasicHashSuppressRC(CFBasicHashRef ht);
void CFBasicHashUnsuppressRC(CFBasicHashRef ht);

CFStringRef CFBasicHashCopyDescription(CFConstBasicHashRef ht, Boolean detailed, CFStringRef linePrefix, CFStringRef entryLinePrefix, Boolean describeElements);

CFTypeID CFBasicHashGetTypeID(void);

extern Boolean __CFBasicHashEqual(CFTypeRef cf1, CFTypeRef cf2);
extern CFHashCode __CFBasicHashHash(CFTypeRef cf);
extern CFStringRef __CFBasicHashCopyDescription(CFTypeRef cf);
extern void __CFBasicHashDeallocate(CFTypeRef cf);
extern unsigned long __CFBasicHashFastEnumeration(CFConstBasicHashRef ht, struct __objcFastEnumerationStateEquivalent2 *state, void *stackbuffer, unsigned long count);

// creation functions create mutable CFBasicHashRefs
CFBasicHashRef CFBasicHashCreate(CFAllocatorRef allocator, CFOptionFlags flags, const CFBasicHashCallbacks *cb);
CFBasicHashRef CFBasicHashCreateCopy(CFAllocatorRef allocator, CFConstBasicHashRef ht);


CF_EXTERN_C_END

