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

/*	CFBasicHash.m
	Copyright (c) 2008-2014, Apple Inc. All rights reserved.
	Responsibility: Christopher Kane
*/

#import "CFBasicHash.h"
#import <CoreFoundation/CFRuntime.h>
#import <CoreFoundation/CFSet.h>
#import <Block.h>
#import <math.h>
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#import <dispatch/dispatch.h>
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#define __SetLastAllocationEventName(A, B) do { if (__CFOASafe && (A)) __CFSetLastAllocationEventName(A, B); } while (0)
#else
#define __SetLastAllocationEventName(A, B) do { } while (0)
#endif

#define __AssignWithWriteBarrier(location, value) objc_assign_strongCast((id)value, (id *)location)

#define ENABLE_DTRACE_PROBES 0
#define ENABLE_MEMORY_COUNTERS 0

#if defined(DTRACE_PROBES_DISABLED) && DTRACE_PROBES_DISABLED
#undef ENABLE_DTRACE_PROBES
#define ENABLE_DTRACE_PROBES 0
#endif

/*
// dtrace -h -s foo.d
// Note: output then changed by casts of the arguments
// dtrace macros last generated 2010-09-08 on 10.7 prerelease (11A259)

provider Cocoa_HashTable {
        probe hash_key(unsigned long table, unsigned long key, unsigned long hash);
        probe test_equal(unsigned long table, unsigned long key1, unsigned long key2);
        probe probing_start(unsigned long table, unsigned long num_buckets);
        probe probe_empty(unsigned long table, unsigned long idx);
        probe probe_deleted(unsigned long table, unsigned long idx);
        probe probe_valid(unsigned long table, unsigned long idx);
        probe probing_end(unsigned long table, unsigned long num_probes);
        probe rehash_start(unsigned long table, unsigned long num_buckets, unsigned long total_size);
        probe rehash_end(unsigned long table, unsigned long num_buckets, unsigned long total_size);
};

#pragma D attributes Unstable/Unstable/Common provider Cocoa_HashTable provider
#pragma D attributes Private/Private/Unknown provider Cocoa_HashTable module
#pragma D attributes Private/Private/Unknown provider Cocoa_HashTable function
#pragma D attributes Unstable/Unstable/Common provider Cocoa_HashTable name
#pragma D attributes Unstable/Unstable/Common provider Cocoa_HashTable args
*/

#if ENABLE_DTRACE_PROBES

#define COCOA_HASHTABLE_STABILITY "___dtrace_stability$Cocoa_HashTable$v1$4_4_5_1_1_0_1_1_0_4_4_5_4_4_5"

#define COCOA_HASHTABLE_TYPEDEFS "___dtrace_typedefs$Cocoa_HashTable$v2"

#define COCOA_HASHTABLE_REHASH_END(arg0, arg1, arg2) \
do { \
        __asm__ volatile(".reference " COCOA_HASHTABLE_TYPEDEFS); \
        __dtrace_probe$Cocoa_HashTable$rehash_end$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67((unsigned long)(arg0), (unsigned long)(arg1), (unsigned long)(arg2)); \
        __asm__ volatile(".reference " COCOA_HASHTABLE_STABILITY); \
} while (0)
#define	COCOA_HASHTABLE_REHASH_END_ENABLED() \
	({ int _r = __dtrace_isenabled$Cocoa_HashTable$rehash_end$v1(); \
		__asm__ volatile(""); \
		_r; })
#define COCOA_HASHTABLE_REHASH_START(arg0, arg1, arg2) \
do { \
        __asm__ volatile(".reference " COCOA_HASHTABLE_TYPEDEFS); \
        __dtrace_probe$Cocoa_HashTable$rehash_start$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67((unsigned long)(arg0), (unsigned long)(arg1), (unsigned long)(arg2)); \
        __asm__ volatile(".reference " COCOA_HASHTABLE_STABILITY); \
} while (0)
#define	COCOA_HASHTABLE_REHASH_START_ENABLED() \
	({ int _r = __dtrace_isenabled$Cocoa_HashTable$rehash_start$v1(); \
		__asm__ volatile(""); \
		_r; })
#define COCOA_HASHTABLE_HASH_KEY(arg0, arg1, arg2) \
do { \
        __asm__ volatile(".reference " COCOA_HASHTABLE_TYPEDEFS); \
        __dtrace_probe$Cocoa_HashTable$hash_key$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67((unsigned long)(arg0), (unsigned long)(arg1), (unsigned long)(arg2)); \
        __asm__ volatile(".reference " COCOA_HASHTABLE_STABILITY); \
} while (0)
#define	COCOA_HASHTABLE_HASH_KEY_ENABLED() \
	({ int _r = __dtrace_isenabled$Cocoa_HashTable$hash_key$v1(); \
		__asm__ volatile(""); \
		_r; })
#define COCOA_HASHTABLE_PROBE_DELETED(arg0, arg1) \
do { \
        __asm__ volatile(".reference " COCOA_HASHTABLE_TYPEDEFS); \
        __dtrace_probe$Cocoa_HashTable$probe_deleted$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67((unsigned long)(arg0), (unsigned long)(arg1)); \
        __asm__ volatile(".reference " COCOA_HASHTABLE_STABILITY); \
} while (0)
#define	COCOA_HASHTABLE_PROBE_DELETED_ENABLED() \
	({ int _r = __dtrace_isenabled$Cocoa_HashTable$probe_deleted$v1(); \
		__asm__ volatile(""); \
		_r; })
#define COCOA_HASHTABLE_PROBE_EMPTY(arg0, arg1) \
do { \
        __asm__ volatile(".reference " COCOA_HASHTABLE_TYPEDEFS); \
        __dtrace_probe$Cocoa_HashTable$probe_empty$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67((unsigned long)(arg0), (unsigned long)(arg1)); \
        __asm__ volatile(".reference " COCOA_HASHTABLE_STABILITY); \
} while (0)
#define	COCOA_HASHTABLE_PROBE_EMPTY_ENABLED() \
	({ int _r = __dtrace_isenabled$Cocoa_HashTable$probe_empty$v1(); \
		__asm__ volatile(""); \
		_r; })
#define COCOA_HASHTABLE_PROBE_VALID(arg0, arg1) \
do { \
        __asm__ volatile(".reference " COCOA_HASHTABLE_TYPEDEFS); \
        __dtrace_probe$Cocoa_HashTable$probe_valid$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67((unsigned long)(arg0), (unsigned long)(arg1)); \
        __asm__ volatile(".reference " COCOA_HASHTABLE_STABILITY); \
} while (0)
#define	COCOA_HASHTABLE_PROBE_VALID_ENABLED() \
	({ int _r = __dtrace_isenabled$Cocoa_HashTable$probe_valid$v1(); \
		__asm__ volatile(""); \
		_r; })
#define COCOA_HASHTABLE_PROBING_END(arg0, arg1) \
do { \
        __asm__ volatile(".reference " COCOA_HASHTABLE_TYPEDEFS); \
        __dtrace_probe$Cocoa_HashTable$probing_end$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67((unsigned long)(arg0), (unsigned long)(arg1)); \
        __asm__ volatile(".reference " COCOA_HASHTABLE_STABILITY); \
} while (0)
#define	COCOA_HASHTABLE_PROBING_END_ENABLED() \
	({ int _r = __dtrace_isenabled$Cocoa_HashTable$probing_end$v1(); \
		__asm__ volatile(""); \
		_r; })
#define COCOA_HASHTABLE_PROBING_START(arg0, arg1) \
do { \
        __asm__ volatile(".reference " COCOA_HASHTABLE_TYPEDEFS); \
        __dtrace_probe$Cocoa_HashTable$probing_start$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67((unsigned long)(arg0), (unsigned long)(arg1)); \
        __asm__ volatile(".reference " COCOA_HASHTABLE_STABILITY); \
} while (0)
#define	COCOA_HASHTABLE_PROBING_START_ENABLED() \
	({ int _r = __dtrace_isenabled$Cocoa_HashTable$probing_start$v1(); \
		__asm__ volatile(""); \
		_r; })
#define COCOA_HASHTABLE_TEST_EQUAL(arg0, arg1, arg2) \
do { \
        __asm__ volatile(".reference " COCOA_HASHTABLE_TYPEDEFS); \
        __dtrace_probe$Cocoa_HashTable$test_equal$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67((unsigned long)(arg0), (unsigned long)(arg1), (unsigned long)(arg2)); \
        __asm__ volatile(".reference " COCOA_HASHTABLE_STABILITY); \
} while (0)
#define	COCOA_HASHTABLE_TEST_EQUAL_ENABLED() \
	({ int _r = __dtrace_isenabled$Cocoa_HashTable$test_equal$v1(); \
		__asm__ volatile(""); \
		_r; })

extern void __dtrace_probe$Cocoa_HashTable$hash_key$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67(unsigned long, unsigned long, unsigned long);
extern int __dtrace_isenabled$Cocoa_HashTable$hash_key$v1(void);
extern void __dtrace_probe$Cocoa_HashTable$probe_deleted$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67(unsigned long, unsigned long);
extern int __dtrace_isenabled$Cocoa_HashTable$probe_deleted$v1(void);
extern void __dtrace_probe$Cocoa_HashTable$probe_empty$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67(unsigned long, unsigned long);
extern int __dtrace_isenabled$Cocoa_HashTable$probe_empty$v1(void);
extern void __dtrace_probe$Cocoa_HashTable$probe_valid$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67(unsigned long, unsigned long);
extern int __dtrace_isenabled$Cocoa_HashTable$probe_valid$v1(void);
extern void __dtrace_probe$Cocoa_HashTable$probing_end$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67(unsigned long, unsigned long);
extern int __dtrace_isenabled$Cocoa_HashTable$probing_end$v1(void);
extern void __dtrace_probe$Cocoa_HashTable$probing_start$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67(unsigned long, unsigned long);
extern int __dtrace_isenabled$Cocoa_HashTable$probing_start$v1(void);
extern void __dtrace_probe$Cocoa_HashTable$rehash_end$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67(unsigned long, unsigned long, unsigned long);
extern int __dtrace_isenabled$Cocoa_HashTable$rehash_end$v1(void);
extern void __dtrace_probe$Cocoa_HashTable$rehash_start$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67(unsigned long, unsigned long, unsigned long);
extern int __dtrace_isenabled$Cocoa_HashTable$rehash_start$v1(void);
extern void __dtrace_probe$Cocoa_HashTable$test_equal$v1$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67$756e7369676e6564206c6f6e67(unsigned long, unsigned long, unsigned long);
extern int __dtrace_isenabled$Cocoa_HashTable$test_equal$v1(void);

#else

#define COCOA_HASHTABLE_REHASH_END(arg0, arg1, arg2) do {} while (0)
#define COCOA_HASHTABLE_REHASH_END_ENABLED() 0
#define COCOA_HASHTABLE_REHASH_START(arg0, arg1, arg2) do {} while (0)
#define COCOA_HASHTABLE_REHASH_START_ENABLED() 0
#define COCOA_HASHTABLE_HASH_KEY(arg0, arg1, arg2) do {} while (0)
#define COCOA_HASHTABLE_HASH_KEY_ENABLED() 0
#define COCOA_HASHTABLE_PROBE_DELETED(arg0, arg1) do {} while (0)
#define COCOA_HASHTABLE_PROBE_DELETED_ENABLED() 0
#define COCOA_HASHTABLE_PROBE_EMPTY(arg0, arg1) do {} while (0)
#define COCOA_HASHTABLE_PROBE_EMPTY_ENABLED() 0
#define COCOA_HASHTABLE_PROBE_VALID(arg0, arg1) do {} while (0)
#define COCOA_HASHTABLE_PROBE_VALID_ENABLED() 0
#define COCOA_HASHTABLE_PROBING_END(arg0, arg1) do {} while (0)
#define COCOA_HASHTABLE_PROBING_END_ENABLED() 0
#define COCOA_HASHTABLE_PROBING_START(arg0, arg1) do {} while (0)
#define COCOA_HASHTABLE_PROBING_START_ENABLED() 0
#define COCOA_HASHTABLE_TEST_EQUAL(arg0, arg1, arg2) do {} while (0)
#define COCOA_HASHTABLE_TEST_EQUAL_ENABLED() 0

#endif


#if !defined(__LP64__)
#define __LP64__ 0
#endif

// Prime numbers. Values above 100 have been adjusted up so that the
// malloced block size will be just below a multiple of 512; values
// above 1200 have been adjusted up to just below a multiple of 4096.
static const uintptr_t __CFBasicHashTableSizes[64] = {
    0, 3, 7, 13, 23, 41, 71, 127, 191, 251, 383, 631, 1087, 1723,
    2803, 4523, 7351, 11959, 19447, 31231, 50683, 81919, 132607,
    214519, 346607, 561109, 907759, 1468927, 2376191, 3845119,
    6221311, 10066421, 16287743, 26354171, 42641881, 68996069,
    111638519, 180634607, 292272623, 472907251,
#if __LP64__
    765180413UL, 1238087663UL, 2003267557UL, 3241355263UL, 5244622819UL,
#if 0
    8485977589UL, 13730600407UL, 22216578047UL, 35947178479UL,
    58163756537UL, 94110934997UL, 152274691561UL, 246385626107UL,
    398660317687UL, 645045943807UL, 1043706260983UL, 1688752204787UL,
    2732458465769UL, 4421210670577UL, 7153669136377UL,
    11574879807461UL, 18728548943849UL, 30303428750843UL
#endif
#endif
};

static const uintptr_t __CFBasicHashTableCapacities[64] = {
    0, 3, 6, 11, 19, 32, 52, 85, 118, 155, 237, 390, 672, 1065,
    1732, 2795, 4543, 7391, 12019, 19302, 31324, 50629, 81956,
    132580, 214215, 346784, 561026, 907847, 1468567, 2376414,
    3844982, 6221390, 10066379, 16287773, 26354132, 42641916,
    68996399, 111638327, 180634415, 292272755,
#if __LP64__
    472907503UL, 765180257UL, 1238087439UL, 2003267722UL, 3241355160UL,
#if 0
    5244622578UL, 8485977737UL, 13730600347UL, 22216578100UL,
    35947178453UL, 58163756541UL, 94110935011UL, 152274691274UL,
    246385626296UL, 398660317578UL, 645045943559UL, 1043706261135UL,
    1688752204693UL, 2732458465840UL, 4421210670552UL,
    7153669136706UL, 11574879807265UL, 18728548943682UL
#endif
#endif
};

// Primitive roots for the primes above
static const uintptr_t __CFBasicHashPrimitiveRoots[64] = {
    0, 2, 3, 2, 5, 6, 7, 3, 19, 6, 5, 3, 3, 3,
    2, 5, 6, 3, 3, 6, 2, 3, 3,
    3, 5, 10, 3, 3, 22, 3,
    3, 3, 5, 2, 22, 2,
    11, 5, 5, 2,
#if __LP64__
    3, 10, 2, 3, 10,
    2, 3, 5, 3,
    3, 2, 7, 2,
    3, 3, 3, 2,
    3, 5, 5,
    2, 3, 2
#endif
};

CF_INLINE void *__CFBasicHashAllocateMemory(CFConstBasicHashRef ht, CFIndex count, CFIndex elem_size, Boolean strong, Boolean compactable) {
    CFAllocatorRef allocator = CFGetAllocator(ht);
    void *new_mem = NULL;
    if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
        new_mem = auto_zone_allocate_object(objc_collectableZone(), count * elem_size, strong ? (compactable ? AUTO_POINTERS_ONLY : AUTO_MEMORY_SCANNED) : AUTO_UNSCANNED, false, false);
    } else {
        new_mem = CFAllocatorAllocate(allocator, count * elem_size, 0);
    }
    return new_mem;
}

CF_INLINE void *__CFBasicHashAllocateMemory2(CFAllocatorRef allocator, CFIndex count, CFIndex elem_size, Boolean strong, Boolean compactable) {
    void *new_mem = NULL;
    if (CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
        new_mem = auto_zone_allocate_object(objc_collectableZone(), count * elem_size, strong ? (compactable ? AUTO_POINTERS_ONLY : AUTO_MEMORY_SCANNED) : AUTO_UNSCANNED, false, false);
    } else {
        new_mem = CFAllocatorAllocate(allocator, count * elem_size, 0);
    }
    return new_mem;
}

#define __CFBasicHashSubABZero 0xa7baadb1
#define __CFBasicHashSubABOne 0xa5baadb9

typedef union {
    uintptr_t neutral;
    id strong;
    id weak;
} CFBasicHashValue;

struct __CFBasicHash {
    CFRuntimeBase base;
    struct { // 192 bits
        uint16_t mutations;
        uint8_t hash_style:2;
        uint8_t keys_offset:1;
        uint8_t counts_offset:2;
        uint8_t counts_width:2;
        uint8_t hashes_offset:2;
        uint8_t strong_values:1;
        uint8_t strong_keys:1;
        uint8_t weak_values:1;
        uint8_t weak_keys:1;
        uint8_t int_values:1;
        uint8_t int_keys:1;
        uint8_t indirect_keys:1;
        uint32_t used_buckets;      /* number of used buckets */
        uint64_t deleted:16;
        uint64_t num_buckets_idx:8; /* index to number of buckets */
        uint64_t __kret:10;
        uint64_t __vret:10;
        uint64_t __krel:10;
        uint64_t __vrel:10;
        uint64_t __:1;
        uint64_t null_rc:1;
        uint64_t fast_grow:1;
        uint64_t finalized:1;
        uint64_t __kdes:10;
        uint64_t __vdes:10;
        uint64_t __kequ:10;
        uint64_t __vequ:10;
        uint64_t __khas:10;
        uint64_t __kget:10;
    } bits;
    void *pointers[1];
};

static void *CFBasicHashCallBackPtrs[(1UL << 10)];
static int32_t CFBasicHashCallBackPtrsCount = 0;

static int32_t CFBasicHashGetPtrIndex(void *ptr) {
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        CFBasicHashCallBackPtrs[0] = NULL;
        CFBasicHashCallBackPtrs[1] = (void *)CFCopyDescription;
        CFBasicHashCallBackPtrs[2] = (void *)__CFTypeCollectionRelease;
        CFBasicHashCallBackPtrs[3] = (void *)__CFTypeCollectionRetain;
        CFBasicHashCallBackPtrs[4] = (void *)CFEqual;
        CFBasicHashCallBackPtrs[5] = (void *)CFHash;
        CFBasicHashCallBackPtrs[6] = (void *)__CFStringCollectionCopy;
        CFBasicHashCallBackPtrs[7] = NULL;
        CFBasicHashCallBackPtrsCount = 8;
    });

    // The uniquing here is done locklessly for best performance, and in
    // a way that will keep multiple threads from stomping each other's
    // newly registered values, but may result in multiple slots
    // containing the same pointer value.

    int32_t idx;
    for (idx = 0; idx < CFBasicHashCallBackPtrsCount; idx++) {
        if (CFBasicHashCallBackPtrs[idx] == ptr) return idx;
    }

    if (1000 < CFBasicHashCallBackPtrsCount) HALT;
    idx = OSAtomicIncrement32(&CFBasicHashCallBackPtrsCount); // returns new value
    CFBasicHashCallBackPtrs[idx - 1] = ptr;
    return idx - 1;
}

CF_PRIVATE Boolean CFBasicHashHasStrongValues(CFConstBasicHashRef ht) {
#if DEPLOYMENT_TARGET_MACOSX
    return ht->bits.strong_values ? true : false;
#else
    return false;
#endif
}

CF_PRIVATE Boolean CFBasicHashHasStrongKeys(CFConstBasicHashRef ht) {
#if DEPLOYMENT_TARGET_MACOSX
    return ht->bits.strong_keys ? true : false;
#else
    return false;
#endif
}

CF_INLINE Boolean __CFBasicHashHasHashCache(CFConstBasicHashRef ht) {
#if DEPLOYMENT_TARGET_MACOSX
    return ht->bits.hashes_offset ? true : false;
#else
    return false;
#endif
}

CF_INLINE uintptr_t __CFBasicHashImportValue(CFConstBasicHashRef ht, uintptr_t stack_value) {
    uintptr_t (*func)(CFAllocatorRef, uintptr_t) = (uintptr_t (*)(CFAllocatorRef, uintptr_t))CFBasicHashCallBackPtrs[ht->bits.__vret];
    if (!func || ht->bits.null_rc) return stack_value;
    CFAllocatorRef alloc = CFGetAllocator(ht);
    return func(alloc, stack_value);
}

CF_INLINE uintptr_t __CFBasicHashImportKey(CFConstBasicHashRef ht, uintptr_t stack_key) {
    uintptr_t (*func)(CFAllocatorRef, uintptr_t) = (uintptr_t (*)(CFAllocatorRef, uintptr_t))CFBasicHashCallBackPtrs[ht->bits.__kret];
    if (!func || ht->bits.null_rc) return stack_key;
    CFAllocatorRef alloc = CFGetAllocator(ht);
    return func(alloc, stack_key);
}

CF_INLINE void __CFBasicHashEjectValue(CFConstBasicHashRef ht, uintptr_t stack_value) {
    void (*func)(CFAllocatorRef, uintptr_t) = (void (*)(CFAllocatorRef, uintptr_t))CFBasicHashCallBackPtrs[ht->bits.__vrel];
    if (!func || ht->bits.null_rc) return;
    CFAllocatorRef alloc = CFGetAllocator(ht);
    func(alloc, stack_value);
}

CF_INLINE void __CFBasicHashEjectKey(CFConstBasicHashRef ht, uintptr_t stack_key) {
    void (*func)(CFAllocatorRef, uintptr_t) = (void (*)(CFAllocatorRef, uintptr_t))CFBasicHashCallBackPtrs[ht->bits.__krel];
    if (!func || ht->bits.null_rc) return;
    CFAllocatorRef alloc = CFGetAllocator(ht);
    func(alloc, stack_key);
}

CF_INLINE CFStringRef __CFBasicHashDescValue(CFConstBasicHashRef ht, uintptr_t stack_value) {
    CFStringRef (*func)(uintptr_t) = (CFStringRef (*)(uintptr_t))CFBasicHashCallBackPtrs[ht->bits.__vdes];
    if (!func) return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<%p>"), (void *)stack_value);
    return func(stack_value);
}

CF_INLINE CFStringRef __CFBasicHashDescKey(CFConstBasicHashRef ht, uintptr_t stack_key) {
    CFStringRef (*func)(uintptr_t) = (CFStringRef (*)(uintptr_t))CFBasicHashCallBackPtrs[ht->bits.__kdes];
    if (!func) return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<%p>"), (void *)stack_key);
    return func(stack_key);
}

CF_INLINE Boolean __CFBasicHashTestEqualValue(CFConstBasicHashRef ht, uintptr_t stack_value_a, uintptr_t stack_value_b) {
    Boolean (*func)(uintptr_t, uintptr_t) = (Boolean (*)(uintptr_t, uintptr_t))CFBasicHashCallBackPtrs[ht->bits.__vequ];
    if (!func) return (stack_value_a == stack_value_b);
    return func(stack_value_a, stack_value_b);
}

CF_INLINE Boolean __CFBasicHashTestEqualKey(CFConstBasicHashRef ht, uintptr_t in_coll_key, uintptr_t stack_key) {
    COCOA_HASHTABLE_TEST_EQUAL(ht, in_coll_key, stack_key);
    Boolean (*func)(uintptr_t, uintptr_t) = (Boolean (*)(uintptr_t, uintptr_t))CFBasicHashCallBackPtrs[ht->bits.__kequ];
    if (!func) return (in_coll_key == stack_key);
    return func(in_coll_key, stack_key);
}

CF_INLINE CFHashCode __CFBasicHashHashKey(CFConstBasicHashRef ht, uintptr_t stack_key) {
    CFHashCode (*func)(uintptr_t) = (CFHashCode (*)(uintptr_t))CFBasicHashCallBackPtrs[ht->bits.__khas];
    CFHashCode hash_code = func ? func(stack_key) : stack_key;
    COCOA_HASHTABLE_HASH_KEY(ht, stack_key, hash_code);
    return hash_code;
}

CF_INLINE uintptr_t __CFBasicHashGetIndirectKey(CFConstBasicHashRef ht, uintptr_t coll_key) {
    uintptr_t (*func)(uintptr_t) = (uintptr_t (*)(uintptr_t))CFBasicHashCallBackPtrs[ht->bits.__kget];
    if (!func) return coll_key;
    return func(coll_key);
}

CF_INLINE CFBasicHashValue *__CFBasicHashGetValues(CFConstBasicHashRef ht) {
    return (CFBasicHashValue *)ht->pointers[0];
}

CF_INLINE void __CFBasicHashSetValues(CFBasicHashRef ht, CFBasicHashValue *ptr) {
    __AssignWithWriteBarrier(&ht->pointers[0], ptr);
}

CF_INLINE CFBasicHashValue *__CFBasicHashGetKeys(CFConstBasicHashRef ht) {
    return (CFBasicHashValue *)ht->pointers[ht->bits.keys_offset];
}

CF_INLINE void __CFBasicHashSetKeys(CFBasicHashRef ht, CFBasicHashValue *ptr) {
    __AssignWithWriteBarrier(&ht->pointers[ht->bits.keys_offset], ptr);
}

CF_INLINE void *__CFBasicHashGetCounts(CFConstBasicHashRef ht) {
    return (void *)ht->pointers[ht->bits.counts_offset];
}

CF_INLINE void __CFBasicHashSetCounts(CFBasicHashRef ht, void *ptr) {
    __AssignWithWriteBarrier(&ht->pointers[ht->bits.counts_offset], ptr);
}

CF_INLINE uintptr_t __CFBasicHashGetValue(CFConstBasicHashRef ht, CFIndex idx) {
    uintptr_t val = __CFBasicHashGetValues(ht)[idx].neutral;
    if (__CFBasicHashSubABZero == val) return 0UL;
    if (__CFBasicHashSubABOne == val) return ~0UL;
    return val;
}

CF_INLINE void __CFBasicHashSetValue(CFBasicHashRef ht, CFIndex idx, uintptr_t stack_value, Boolean ignoreOld, Boolean literal) {
    CFBasicHashValue *valuep = &(__CFBasicHashGetValues(ht)[idx]);
    uintptr_t old_value = ignoreOld ? 0 : valuep->neutral;
    if (!literal) {
        if (0UL == stack_value) stack_value = __CFBasicHashSubABZero;
        if (~0UL == stack_value) stack_value = __CFBasicHashSubABOne;
    }
    if (CFBasicHashHasStrongValues(ht)) valuep->strong = (id)stack_value; else valuep->neutral = stack_value;
    if (!ignoreOld) {
        if (!(old_value == 0UL || old_value == ~0UL)) {
            if (__CFBasicHashSubABZero == old_value) old_value = 0UL;
            if (__CFBasicHashSubABOne == old_value) old_value = ~0UL;
            __CFBasicHashEjectValue(ht, old_value);
        }
    }
}

CF_INLINE uintptr_t __CFBasicHashGetKey(CFConstBasicHashRef ht, CFIndex idx) {
    if (ht->bits.keys_offset) {
        uintptr_t key = __CFBasicHashGetKeys(ht)[idx].neutral;
        if (__CFBasicHashSubABZero == key) return 0UL;
        if (__CFBasicHashSubABOne == key) return ~0UL;
        return key;
    }
    if (ht->bits.indirect_keys) {
        uintptr_t stack_value = __CFBasicHashGetValue(ht, idx);
        return __CFBasicHashGetIndirectKey(ht, stack_value);
    }
    return __CFBasicHashGetValue(ht, idx);
}

CF_INLINE void __CFBasicHashSetKey(CFBasicHashRef ht, CFIndex idx, uintptr_t stack_key, Boolean ignoreOld, Boolean literal) {
    if (0 == ht->bits.keys_offset) HALT;
    CFBasicHashValue *keyp = &(__CFBasicHashGetKeys(ht)[idx]);
    uintptr_t old_key = ignoreOld ? 0 : keyp->neutral;
    if (!literal) {
        if (0UL == stack_key) stack_key = __CFBasicHashSubABZero;
        if (~0UL == stack_key) stack_key = __CFBasicHashSubABOne;
    }
    if (CFBasicHashHasStrongKeys(ht)) keyp->strong = (id)stack_key; else keyp->neutral = stack_key;
    if (!ignoreOld) {
        if (!(old_key == 0UL || old_key == ~0UL)) {
            if (__CFBasicHashSubABZero == old_key) old_key = 0UL;
            if (__CFBasicHashSubABOne == old_key) old_key = ~0UL;
            __CFBasicHashEjectKey(ht, old_key);
        }
    }
}

CF_INLINE uintptr_t __CFBasicHashIsEmptyOrDeleted(CFConstBasicHashRef ht, CFIndex idx) {
    uintptr_t stack_value = __CFBasicHashGetValues(ht)[idx].neutral;
    return (0UL == stack_value || ~0UL == stack_value);
}

CF_INLINE uintptr_t __CFBasicHashIsDeleted(CFConstBasicHashRef ht, CFIndex idx) {
    uintptr_t stack_value = __CFBasicHashGetValues(ht)[idx].neutral;
    return (~0UL == stack_value);
}

CF_INLINE uintptr_t __CFBasicHashGetSlotCount(CFConstBasicHashRef ht, CFIndex idx) {
    void *counts = __CFBasicHashGetCounts(ht);
    switch (ht->bits.counts_width) {
    case 0: return ((uint8_t *)counts)[idx];
    case 1: return ((uint16_t *)counts)[idx];
    case 2: return ((uint32_t *)counts)[idx];
    case 3: return ((uint64_t *)counts)[idx];
    }
    return 0;
}

CF_INLINE void __CFBasicHashBumpCounts(CFBasicHashRef ht) {
    void *counts = __CFBasicHashGetCounts(ht);
    CFAllocatorRef allocator = CFGetAllocator(ht);
    switch (ht->bits.counts_width) {
    case 0: {
        uint8_t *counts08 = (uint8_t *)counts;
        ht->bits.counts_width = 1;
        CFIndex num_buckets = __CFBasicHashTableSizes[ht->bits.num_buckets_idx];
        uint16_t *counts16 = (uint16_t *)__CFBasicHashAllocateMemory(ht, num_buckets, 2, false, false);
        if (!counts16) HALT;
        __SetLastAllocationEventName(counts16, "CFBasicHash (count-store)");
        for (CFIndex idx2 = 0; idx2 < num_buckets; idx2++) {
            counts16[idx2] = counts08[idx2];
        }
        __CFBasicHashSetCounts(ht, counts16);
        if (!CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
            CFAllocatorDeallocate(allocator, counts08);
        }
        break;
    }
    case 1: {
        uint16_t *counts16 = (uint16_t *)counts;
        ht->bits.counts_width = 2;
        CFIndex num_buckets = __CFBasicHashTableSizes[ht->bits.num_buckets_idx];
        uint32_t *counts32 = (uint32_t *)__CFBasicHashAllocateMemory(ht, num_buckets, 4, false, false);
        if (!counts32) HALT;
        __SetLastAllocationEventName(counts32, "CFBasicHash (count-store)");
        for (CFIndex idx2 = 0; idx2 < num_buckets; idx2++) {
            counts32[idx2] = counts16[idx2];
        }
        __CFBasicHashSetCounts(ht, counts32);
         if (!CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
            CFAllocatorDeallocate(allocator, counts16);
        }
        break;
    }
    case 2: {
        uint32_t *counts32 = (uint32_t *)counts;
        ht->bits.counts_width = 3;
        CFIndex num_buckets = __CFBasicHashTableSizes[ht->bits.num_buckets_idx];
        uint64_t *counts64 = (uint64_t *)__CFBasicHashAllocateMemory(ht, num_buckets, 8, false, false);
        if (!counts64) HALT;
        __SetLastAllocationEventName(counts64, "CFBasicHash (count-store)");
        for (CFIndex idx2 = 0; idx2 < num_buckets; idx2++) {
            counts64[idx2] = counts32[idx2];
        }
        __CFBasicHashSetCounts(ht, counts64);
         if (!CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
            CFAllocatorDeallocate(allocator, counts32);
        }
        break;
    }
    case 3: {
        HALT;
        break;
    }
    }
}

static void __CFBasicHashIncSlotCount(CFBasicHashRef ht, CFIndex idx) {
    void *counts = __CFBasicHashGetCounts(ht);
    switch (ht->bits.counts_width) {
    case 0: {
        uint8_t *counts08 = (uint8_t *)counts;
        uint8_t val = counts08[idx];
        if (val < INT8_MAX) {
            counts08[idx] = val + 1;
            return;
        }
        __CFBasicHashBumpCounts(ht);
        __CFBasicHashIncSlotCount(ht, idx);
        break;
    }
    case 1: {
        uint16_t *counts16 = (uint16_t *)counts;
        uint16_t val = counts16[idx];
        if (val < INT16_MAX) {
            counts16[idx] = val + 1;
            return;
        }
        __CFBasicHashBumpCounts(ht);
        __CFBasicHashIncSlotCount(ht, idx);
        break;
    }
    case 2: {
        uint32_t *counts32 = (uint32_t *)counts;
        uint32_t val = counts32[idx];
        if (val < INT32_MAX) {
            counts32[idx] = val + 1;
            return;
        }
        __CFBasicHashBumpCounts(ht);
        __CFBasicHashIncSlotCount(ht, idx);
        break;
    }
    case 3: {
        uint64_t *counts64 = (uint64_t *)counts;
        uint64_t val = counts64[idx];
        if (val < INT64_MAX) {
            counts64[idx] = val + 1;
            return;
        }
        __CFBasicHashBumpCounts(ht);
        __CFBasicHashIncSlotCount(ht, idx);
        break;
    }
    }
}

CF_INLINE void __CFBasicHashDecSlotCount(CFBasicHashRef ht, CFIndex idx) {
    void *counts = __CFBasicHashGetCounts(ht);
    switch (ht->bits.counts_width) {
    case 0: ((uint8_t  *)counts)[idx]--; return;
    case 1: ((uint16_t *)counts)[idx]--; return;
    case 2: ((uint32_t *)counts)[idx]--; return;
    case 3: ((uint64_t *)counts)[idx]--; return;
    }
}

CF_INLINE uintptr_t *__CFBasicHashGetHashes(CFConstBasicHashRef ht) {
    return (uintptr_t *)ht->pointers[ht->bits.hashes_offset];
}

CF_INLINE void __CFBasicHashSetHashes(CFBasicHashRef ht, uintptr_t *ptr) {
    __AssignWithWriteBarrier(&ht->pointers[ht->bits.hashes_offset], ptr);
}


// to expose the load factor, expose this function to customization
CF_INLINE CFIndex __CFBasicHashGetCapacityForNumBuckets(CFConstBasicHashRef ht, CFIndex num_buckets_idx) {
    return __CFBasicHashTableCapacities[num_buckets_idx];
}

CF_INLINE CFIndex __CFBasicHashGetNumBucketsIndexForCapacity(CFConstBasicHashRef ht, CFIndex capacity) {
    for (CFIndex idx = 0; idx < 64; idx++) {
        if (capacity <= __CFBasicHashGetCapacityForNumBuckets(ht, idx)) return idx;
    }
    HALT;
    return 0;
}

CF_PRIVATE CFIndex CFBasicHashGetNumBuckets(CFConstBasicHashRef ht) {
    return __CFBasicHashTableSizes[ht->bits.num_buckets_idx];
}

CF_PRIVATE CFIndex CFBasicHashGetCapacity(CFConstBasicHashRef ht) {
    return __CFBasicHashGetCapacityForNumBuckets(ht, ht->bits.num_buckets_idx);
}

// In returned struct, .count is zero if the bucket is empty or deleted,
// and the .weak_key field indicates which. .idx is either the index of
// the found bucket or the index of the bucket which should be filled by
// an add operation. For a set or multiset, the .weak_key and .weak_value
// are the same.
CF_PRIVATE CFBasicHashBucket CFBasicHashGetBucket(CFConstBasicHashRef ht, CFIndex idx) {
    CFBasicHashBucket result;
    result.idx = idx;
    if (__CFBasicHashIsEmptyOrDeleted(ht, idx)) {
        result.count = 0;
        result.weak_value = 0;
        result.weak_key = 0;
    } else {
        result.count = (ht->bits.counts_offset) ? __CFBasicHashGetSlotCount(ht, idx) : 1;
        result.weak_value = __CFBasicHashGetValue(ht, idx);
        result.weak_key = __CFBasicHashGetKey(ht, idx);
    }
    return result;
}

#if defined(__arm__)
static uintptr_t __CFBasicHashFold(uintptr_t dividend, uint8_t idx) {
    switch (idx) {
    case 1: return dividend % 3;
    case 2: return dividend % 7;
    case 3: return dividend % 13;
    case 4: return dividend % 23;
    case 5: return dividend % 41;
    case 6: return dividend % 71;
    case 7: return dividend % 127;
    case 8: return dividend % 191;
    case 9: return dividend % 251;
    case 10: return dividend % 383;
    case 11: return dividend % 631;
    case 12: return dividend % 1087;
    case 13: return dividend % 1723;
    case 14: return dividend % 2803;
    case 15: return dividend % 4523;
    case 16: return dividend % 7351;
    case 17: return dividend % 11959;
    case 18: return dividend % 19447;
    case 19: return dividend % 31231;
    case 20: return dividend % 50683;
    case 21: return dividend % 81919;
    case 22: return dividend % 132607;
    case 23: return dividend % 214519;
    case 24: return dividend % 346607;
    case 25: return dividend % 561109;
    case 26: return dividend % 907759;
    case 27: return dividend % 1468927;
    case 28: return dividend % 2376191;
    case 29: return dividend % 3845119;
    case 30: return dividend % 6221311;
    case 31: return dividend % 10066421;
    case 32: return dividend % 16287743;
    case 33: return dividend % 26354171;
    case 34: return dividend % 42641881;
    case 35: return dividend % 68996069;
    case 36: return dividend % 111638519;
    case 37: return dividend % 180634607;
    case 38: return dividend % 292272623;
    case 39: return dividend % 472907251;
    }
    HALT;
    return ~0;
}
#endif


#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Linear
#define FIND_BUCKET_HASH_STYLE		1
#define FIND_BUCKET_FOR_REHASH		0
#define FIND_BUCKET_FOR_INDIRECT_KEY	0
#include "CFBasicHashFindBucket.m"

#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Linear_NoCollision
#define FIND_BUCKET_HASH_STYLE		1
#define FIND_BUCKET_FOR_REHASH		1
#define FIND_BUCKET_FOR_INDIRECT_KEY	0
#include "CFBasicHashFindBucket.m"

#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Linear_Indirect
#define FIND_BUCKET_HASH_STYLE		1
#define FIND_BUCKET_FOR_REHASH		0
#define FIND_BUCKET_FOR_INDIRECT_KEY	1
#include "CFBasicHashFindBucket.m"

#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Linear_Indirect_NoCollision
#define FIND_BUCKET_HASH_STYLE		1
#define FIND_BUCKET_FOR_REHASH		1
#define FIND_BUCKET_FOR_INDIRECT_KEY	1
#include "CFBasicHashFindBucket.m"

#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Double
#define FIND_BUCKET_HASH_STYLE		2
#define FIND_BUCKET_FOR_REHASH		0
#define FIND_BUCKET_FOR_INDIRECT_KEY	0
#include "CFBasicHashFindBucket.m"

#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Double_NoCollision
#define FIND_BUCKET_HASH_STYLE		2
#define FIND_BUCKET_FOR_REHASH		1
#define FIND_BUCKET_FOR_INDIRECT_KEY	0
#include "CFBasicHashFindBucket.m"

#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Double_Indirect
#define FIND_BUCKET_HASH_STYLE		2
#define FIND_BUCKET_FOR_REHASH		0
#define FIND_BUCKET_FOR_INDIRECT_KEY	1
#include "CFBasicHashFindBucket.m"

#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Double_Indirect_NoCollision
#define FIND_BUCKET_HASH_STYLE		2
#define FIND_BUCKET_FOR_REHASH		1
#define FIND_BUCKET_FOR_INDIRECT_KEY	1
#include "CFBasicHashFindBucket.m"

#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Exponential
#define FIND_BUCKET_HASH_STYLE		3
#define FIND_BUCKET_FOR_REHASH		0
#define FIND_BUCKET_FOR_INDIRECT_KEY	0
#include "CFBasicHashFindBucket.m"

#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Exponential_NoCollision
#define FIND_BUCKET_HASH_STYLE		3
#define FIND_BUCKET_FOR_REHASH		1
#define FIND_BUCKET_FOR_INDIRECT_KEY	0
#include "CFBasicHashFindBucket.m"

#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Exponential_Indirect
#define FIND_BUCKET_HASH_STYLE		3
#define FIND_BUCKET_FOR_REHASH		0
#define FIND_BUCKET_FOR_INDIRECT_KEY	1
#include "CFBasicHashFindBucket.m"

#define FIND_BUCKET_NAME		___CFBasicHashFindBucket_Exponential_Indirect_NoCollision
#define FIND_BUCKET_HASH_STYLE		3
#define FIND_BUCKET_FOR_REHASH		1
#define FIND_BUCKET_FOR_INDIRECT_KEY	1
#include "CFBasicHashFindBucket.m"


CF_INLINE CFBasicHashBucket __CFBasicHashFindBucket(CFConstBasicHashRef ht, uintptr_t stack_key) {
    if (0 == ht->bits.num_buckets_idx) {
        CFBasicHashBucket result = {kCFNotFound, 0UL, 0UL, 0};
        return result;
    }
    if (ht->bits.indirect_keys) {
        switch (ht->bits.hash_style) {
        case __kCFBasicHashLinearHashingValue: return ___CFBasicHashFindBucket_Linear_Indirect(ht, stack_key);
        case __kCFBasicHashDoubleHashingValue: return ___CFBasicHashFindBucket_Double_Indirect(ht, stack_key);
        case __kCFBasicHashExponentialHashingValue: return ___CFBasicHashFindBucket_Exponential_Indirect(ht, stack_key);
        }
    } else {
        switch (ht->bits.hash_style) {
        case __kCFBasicHashLinearHashingValue: return ___CFBasicHashFindBucket_Linear(ht, stack_key);
        case __kCFBasicHashDoubleHashingValue: return ___CFBasicHashFindBucket_Double(ht, stack_key);
        case __kCFBasicHashExponentialHashingValue: return ___CFBasicHashFindBucket_Exponential(ht, stack_key);
        }
    }
    HALT;
    CFBasicHashBucket result = {kCFNotFound, 0UL, 0UL, 0};
    return result;
}

CF_INLINE CFIndex __CFBasicHashFindBucket_NoCollision(CFConstBasicHashRef ht, uintptr_t stack_key, uintptr_t key_hash) {
    if (0 == ht->bits.num_buckets_idx) {
        return kCFNotFound;
    }
    if (ht->bits.indirect_keys) {
        switch (ht->bits.hash_style) {
        case __kCFBasicHashLinearHashingValue: return ___CFBasicHashFindBucket_Linear_Indirect_NoCollision(ht, stack_key, key_hash);
        case __kCFBasicHashDoubleHashingValue: return ___CFBasicHashFindBucket_Double_Indirect_NoCollision(ht, stack_key, key_hash);
        case __kCFBasicHashExponentialHashingValue: return ___CFBasicHashFindBucket_Exponential_Indirect_NoCollision(ht, stack_key, key_hash);
        }
    } else {
        switch (ht->bits.hash_style) {
        case __kCFBasicHashLinearHashingValue: return ___CFBasicHashFindBucket_Linear_NoCollision(ht, stack_key, key_hash);
        case __kCFBasicHashDoubleHashingValue: return ___CFBasicHashFindBucket_Double_NoCollision(ht, stack_key, key_hash);
        case __kCFBasicHashExponentialHashingValue: return ___CFBasicHashFindBucket_Exponential_NoCollision(ht, stack_key, key_hash);
        }
    }
    HALT;
    return kCFNotFound;
}

CF_PRIVATE CFBasicHashBucket CFBasicHashFindBucket(CFConstBasicHashRef ht, uintptr_t stack_key) {
    if (__CFBasicHashSubABZero == stack_key || __CFBasicHashSubABOne == stack_key) {
        CFBasicHashBucket result = {kCFNotFound, 0UL, 0UL, 0};
        return result;
    }
    return __CFBasicHashFindBucket(ht, stack_key);
}

CF_PRIVATE void CFBasicHashSuppressRC(CFBasicHashRef ht) {
    ht->bits.null_rc = 1;
}

CF_PRIVATE void CFBasicHashUnsuppressRC(CFBasicHashRef ht) {
    ht->bits.null_rc = 0;
}

CF_PRIVATE CFOptionFlags CFBasicHashGetFlags(CFConstBasicHashRef ht) {
    CFOptionFlags flags = (ht->bits.hash_style << 13);
    if (CFBasicHashHasStrongValues(ht)) flags |= kCFBasicHashStrongValues;
    if (CFBasicHashHasStrongKeys(ht)) flags |= kCFBasicHashStrongKeys;
    if (ht->bits.fast_grow) flags |= kCFBasicHashAggressiveGrowth;
    if (ht->bits.keys_offset) flags |= kCFBasicHashHasKeys;
    if (ht->bits.counts_offset) flags |= kCFBasicHashHasCounts;
    if (__CFBasicHashHasHashCache(ht)) flags |= kCFBasicHashHasHashCache;
    return flags;
}

CF_PRIVATE CFIndex CFBasicHashGetCount(CFConstBasicHashRef ht) {
    if (ht->bits.counts_offset) {
        CFIndex total = 0L;
        CFIndex cnt = (CFIndex)__CFBasicHashTableSizes[ht->bits.num_buckets_idx];
        for (CFIndex idx = 0; idx < cnt; idx++) {
            total += __CFBasicHashGetSlotCount(ht, idx);
        }
        return total;
    }
    return (CFIndex)ht->bits.used_buckets;
}

CF_PRIVATE CFIndex CFBasicHashGetCountOfKey(CFConstBasicHashRef ht, uintptr_t stack_key) {
    if (__CFBasicHashSubABZero == stack_key || __CFBasicHashSubABOne == stack_key) {
        return 0L;
    }
    if (0L == ht->bits.used_buckets) {
        return 0L;
    }
    return __CFBasicHashFindBucket(ht, stack_key).count;
}

CF_PRIVATE CFIndex CFBasicHashGetCountOfValue(CFConstBasicHashRef ht, uintptr_t stack_value) {
    if (__CFBasicHashSubABZero == stack_value) {
        return 0L;
    }
    if (0L == ht->bits.used_buckets) {
        return 0L;
    }
    if (!(ht->bits.keys_offset)) {
        return __CFBasicHashFindBucket(ht, stack_value).count;
    }
    __block CFIndex total = 0L;
    CFBasicHashApply(ht, ^(CFBasicHashBucket bkt) {
            if ((stack_value == bkt.weak_value) || __CFBasicHashTestEqualValue(ht, bkt.weak_value, stack_value)) total += bkt.count;
            return (Boolean)true;
        });
    return total;
}

CF_PRIVATE Boolean CFBasicHashesAreEqual(CFConstBasicHashRef ht1, CFConstBasicHashRef ht2) {
    CFIndex cnt1 = CFBasicHashGetCount(ht1);
    if (cnt1 != CFBasicHashGetCount(ht2)) return false;
    if (0 == cnt1) return true;
    __block Boolean equal = true;
    CFBasicHashApply(ht1, ^(CFBasicHashBucket bkt1) {
            CFBasicHashBucket bkt2 = __CFBasicHashFindBucket(ht2, bkt1.weak_key);
            if (bkt1.count != bkt2.count) {
                equal = false;
                return (Boolean)false;
            }
            if ((ht1->bits.keys_offset) && (bkt1.weak_value != bkt2.weak_value) && !__CFBasicHashTestEqualValue(ht1, bkt1.weak_value, bkt2.weak_value)) {
                equal = false;
                return (Boolean)false;
            }
            return (Boolean)true;
        });
    return equal;
}

CF_PRIVATE void CFBasicHashApply(CFConstBasicHashRef ht, Boolean (^block)(CFBasicHashBucket)) {
    CFIndex used = (CFIndex)ht->bits.used_buckets, cnt = (CFIndex)__CFBasicHashTableSizes[ht->bits.num_buckets_idx];
    for (CFIndex idx = 0; 0 < used && idx < cnt; idx++) {
        CFBasicHashBucket bkt = CFBasicHashGetBucket(ht, idx);
        if (0 < bkt.count) {
            if (!block(bkt)) {
                return;
            }
            used--;
        }
    }
}

CF_PRIVATE void CFBasicHashApplyIndexed(CFConstBasicHashRef ht, CFRange range, Boolean (^block)(CFBasicHashBucket)) {
    if (range.length < 0) HALT;
    if (range.length == 0) return;
    CFIndex cnt = (CFIndex)__CFBasicHashTableSizes[ht->bits.num_buckets_idx];
    if (cnt < range.location + range.length) HALT;
    for (CFIndex idx = 0; idx < range.length; idx++) {
        CFBasicHashBucket bkt = CFBasicHashGetBucket(ht, range.location + idx);
        if (0 < bkt.count) {
            if (!block(bkt)) {
                return;
            }
        }
    }
}

CF_PRIVATE void CFBasicHashGetElements(CFConstBasicHashRef ht, CFIndex bufferslen, uintptr_t *weak_values, uintptr_t *weak_keys) {
    CFIndex used = (CFIndex)ht->bits.used_buckets, cnt = (CFIndex)__CFBasicHashTableSizes[ht->bits.num_buckets_idx];
    CFIndex offset = 0;
    for (CFIndex idx = 0; 0 < used && idx < cnt && offset < bufferslen; idx++) {
        CFBasicHashBucket bkt = CFBasicHashGetBucket(ht, idx);
        if (0 < bkt.count) {
            used--;
            for (CFIndex cnt = bkt.count; cnt-- && offset < bufferslen;) {
                if (weak_values) { weak_values[offset] = bkt.weak_value; }
                if (weak_keys) { weak_keys[offset] = bkt.weak_key; }
                offset++;
            }
        }
    }
}

CF_PRIVATE unsigned long __CFBasicHashFastEnumeration(CFConstBasicHashRef ht, struct __objcFastEnumerationStateEquivalent2 *state, void *stackbuffer, unsigned long count) {
    /* copy as many as count items over */
    if (0 == state->state) {        /* first time */
        state->mutationsPtr = (unsigned long *)&ht->bits;
    }
    state->itemsPtr = (unsigned long *)stackbuffer;
    CFIndex cntx = 0;
    CFIndex used = (CFIndex)ht->bits.used_buckets, cnt = (CFIndex)__CFBasicHashTableSizes[ht->bits.num_buckets_idx];
    for (CFIndex idx = (CFIndex)state->state; 0 < used && idx < cnt && cntx < (CFIndex)count; idx++) {
        CFBasicHashBucket bkt = CFBasicHashGetBucket(ht, idx);
        if (0 < bkt.count) {
            state->itemsPtr[cntx++] = (unsigned long)bkt.weak_key;
            used--;
        }
        state->state++;
    }
    return cntx;
}

#if ENABLE_MEMORY_COUNTERS
static volatile int64_t __CFBasicHashTotalCount = 0ULL;
static volatile int64_t __CFBasicHashTotalSize = 0ULL;
static volatile int64_t __CFBasicHashPeakCount = 0ULL;
static volatile int64_t __CFBasicHashPeakSize = 0ULL;
static volatile int32_t __CFBasicHashSizes[64] = {0};
#endif

static void __CFBasicHashDrain(CFBasicHashRef ht, Boolean forFinalization) {
#if ENABLE_MEMORY_COUNTERS
    OSAtomicAdd64Barrier(-1 * (int64_t) CFBasicHashGetSize(ht, true), & __CFBasicHashTotalSize);
#endif

    CFIndex old_num_buckets = __CFBasicHashTableSizes[ht->bits.num_buckets_idx];

    CFAllocatorRef allocator = CFGetAllocator(ht);
    Boolean nullify = (!forFinalization || !CF_IS_COLLECTABLE_ALLOCATOR(allocator));

    CFBasicHashValue *old_values = NULL, *old_keys = NULL;
    void *old_counts = NULL;
    uintptr_t *old_hashes = NULL;

    old_values = __CFBasicHashGetValues(ht);
    if (nullify) __CFBasicHashSetValues(ht, NULL);
    if (ht->bits.keys_offset) {
        old_keys = __CFBasicHashGetKeys(ht);
        if (nullify) __CFBasicHashSetKeys(ht, NULL);
    }
    if (ht->bits.counts_offset) {
        old_counts = __CFBasicHashGetCounts(ht);
        if (nullify) __CFBasicHashSetCounts(ht, NULL);
    }
    if (__CFBasicHashHasHashCache(ht)) {
        old_hashes = __CFBasicHashGetHashes(ht);
        if (nullify) __CFBasicHashSetHashes(ht, NULL);
    }

    if (nullify) {
        ht->bits.mutations++;
        ht->bits.num_buckets_idx = 0;
        ht->bits.used_buckets = 0;
        ht->bits.deleted = 0;
    }
    
        for (CFIndex idx = 0; idx < old_num_buckets; idx++) {
            uintptr_t stack_value = old_values[idx].neutral;
            if (stack_value != 0UL && stack_value != ~0UL) {
                uintptr_t old_value = stack_value;
                if (__CFBasicHashSubABZero == old_value) old_value = 0UL;
                if (__CFBasicHashSubABOne == old_value) old_value = ~0UL;
                __CFBasicHashEjectValue(ht, old_value);
                if (old_keys) {
                    uintptr_t old_key = old_keys[idx].neutral;
                    if (__CFBasicHashSubABZero == old_key) old_key = 0UL;
                    if (__CFBasicHashSubABOne == old_key) old_key = ~0UL;
                    __CFBasicHashEjectKey(ht, old_key);
                }
            }
        }

    if (!CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
        CFAllocatorDeallocate(allocator, old_values);
        CFAllocatorDeallocate(allocator, old_keys);
        CFAllocatorDeallocate(allocator, old_counts);
        CFAllocatorDeallocate(allocator, old_hashes);
    }

#if ENABLE_MEMORY_COUNTERS
    int64_t size_now = OSAtomicAdd64Barrier((int64_t) CFBasicHashGetSize(ht, true), & __CFBasicHashTotalSize);
    while (__CFBasicHashPeakSize < size_now && !OSAtomicCompareAndSwap64Barrier(__CFBasicHashPeakSize, size_now, & __CFBasicHashPeakSize));
#endif
}

static void __CFBasicHashRehash(CFBasicHashRef ht, CFIndex newItemCount) {
#if ENABLE_MEMORY_COUNTERS
    OSAtomicAdd64Barrier(-1 * (int64_t) CFBasicHashGetSize(ht, true), & __CFBasicHashTotalSize);
    OSAtomicAdd32Barrier(-1, &__CFBasicHashSizes[ht->bits.num_buckets_idx]);
#endif

    if (COCOA_HASHTABLE_REHASH_START_ENABLED()) COCOA_HASHTABLE_REHASH_START(ht, CFBasicHashGetNumBuckets(ht), CFBasicHashGetSize(ht, true));

    CFIndex new_num_buckets_idx = ht->bits.num_buckets_idx;
    if (0 != newItemCount) {
        if (newItemCount < 0) newItemCount = 0;
        CFIndex new_capacity_req = ht->bits.used_buckets + newItemCount;
        new_num_buckets_idx = __CFBasicHashGetNumBucketsIndexForCapacity(ht, new_capacity_req);
        if (1 == newItemCount && ht->bits.fast_grow) {
            new_num_buckets_idx++;
        }
    }

    CFIndex new_num_buckets = __CFBasicHashTableSizes[new_num_buckets_idx];
    CFIndex old_num_buckets = __CFBasicHashTableSizes[ht->bits.num_buckets_idx];

    CFBasicHashValue *new_values = NULL, *new_keys = NULL;
    void *new_counts = NULL;
    uintptr_t *new_hashes = NULL;

    if (0 < new_num_buckets) {
        new_values = (CFBasicHashValue *)__CFBasicHashAllocateMemory(ht, new_num_buckets, sizeof(CFBasicHashValue), CFBasicHashHasStrongValues(ht), 0);
        if (!new_values) HALT;
        __SetLastAllocationEventName(new_values, "CFBasicHash (value-store)");
        memset(new_values, 0, new_num_buckets * sizeof(CFBasicHashValue));
        if (ht->bits.keys_offset) {
            new_keys = (CFBasicHashValue *)__CFBasicHashAllocateMemory(ht, new_num_buckets, sizeof(CFBasicHashValue), CFBasicHashHasStrongKeys(ht), 0);
            if (!new_keys) HALT;
            __SetLastAllocationEventName(new_keys, "CFBasicHash (key-store)");
            memset(new_keys, 0, new_num_buckets * sizeof(CFBasicHashValue));
        }
        if (ht->bits.counts_offset) {
            new_counts = (uintptr_t *)__CFBasicHashAllocateMemory(ht, new_num_buckets, (1 << ht->bits.counts_width), false, false);
            if (!new_counts) HALT;
            __SetLastAllocationEventName(new_counts, "CFBasicHash (count-store)");
            memset(new_counts, 0, new_num_buckets * (1 << ht->bits.counts_width));
        }
        if (__CFBasicHashHasHashCache(ht)) {
            new_hashes = (uintptr_t *)__CFBasicHashAllocateMemory(ht, new_num_buckets, sizeof(uintptr_t), false, false);
            if (!new_hashes) HALT;
            __SetLastAllocationEventName(new_hashes, "CFBasicHash (hash-store)");
            memset(new_hashes, 0, new_num_buckets * sizeof(uintptr_t));
        }
    }

    ht->bits.num_buckets_idx = new_num_buckets_idx;
    ht->bits.deleted = 0;

    CFBasicHashValue *old_values = NULL, *old_keys = NULL;
    void *old_counts = NULL;
    uintptr_t *old_hashes = NULL;

    old_values = __CFBasicHashGetValues(ht);
    __CFBasicHashSetValues(ht, new_values);
    if (ht->bits.keys_offset) {
        old_keys = __CFBasicHashGetKeys(ht);
        __CFBasicHashSetKeys(ht, new_keys);
    }
    if (ht->bits.counts_offset) {
        old_counts = __CFBasicHashGetCounts(ht);
        __CFBasicHashSetCounts(ht, new_counts);
    }
    if (__CFBasicHashHasHashCache(ht)) {
        old_hashes = __CFBasicHashGetHashes(ht);
        __CFBasicHashSetHashes(ht, new_hashes);
    }

    if (0 < old_num_buckets) {
        for (CFIndex idx = 0; idx < old_num_buckets; idx++) {
            uintptr_t stack_value = old_values[idx].neutral;
            if (stack_value != 0UL && stack_value != ~0UL) {
                if (__CFBasicHashSubABZero == stack_value) stack_value = 0UL;
                if (__CFBasicHashSubABOne == stack_value) stack_value = ~0UL;
                uintptr_t stack_key = stack_value;
                if (ht->bits.keys_offset) {
                    stack_key = old_keys[idx].neutral;
                    if (__CFBasicHashSubABZero == stack_key) stack_key = 0UL;
                    if (__CFBasicHashSubABOne == stack_key) stack_key = ~0UL;
                }
                if (ht->bits.indirect_keys) {
                    stack_key = __CFBasicHashGetIndirectKey(ht, stack_value);
                }
                CFIndex bkt_idx = __CFBasicHashFindBucket_NoCollision(ht, stack_key, old_hashes ? old_hashes[idx] : 0UL);
                __CFBasicHashSetValue(ht, bkt_idx, stack_value, false, false);
                if (old_keys) {
                    __CFBasicHashSetKey(ht, bkt_idx, stack_key, false, false);
                }
                if (old_counts) {
                    switch (ht->bits.counts_width) {
                    case 0: ((uint8_t *)new_counts)[bkt_idx] = ((uint8_t *)old_counts)[idx]; break;
                    case 1: ((uint16_t *)new_counts)[bkt_idx] = ((uint16_t *)old_counts)[idx]; break;
                    case 2: ((uint32_t *)new_counts)[bkt_idx] = ((uint32_t *)old_counts)[idx]; break;
                    case 3: ((uint64_t *)new_counts)[bkt_idx] = ((uint64_t *)old_counts)[idx]; break;
                    }
                }
                if (old_hashes) {
                    new_hashes[bkt_idx] = old_hashes[idx];
                }
            }
        }
    }

    CFAllocatorRef allocator = CFGetAllocator(ht);
    if (!CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
        CFAllocatorDeallocate(allocator, old_values);
        CFAllocatorDeallocate(allocator, old_keys);
        CFAllocatorDeallocate(allocator, old_counts);
        CFAllocatorDeallocate(allocator, old_hashes);
    }

    if (COCOA_HASHTABLE_REHASH_END_ENABLED()) COCOA_HASHTABLE_REHASH_END(ht, CFBasicHashGetNumBuckets(ht), CFBasicHashGetSize(ht, true));

#if ENABLE_MEMORY_COUNTERS
    int64_t size_now = OSAtomicAdd64Barrier((int64_t) CFBasicHashGetSize(ht, true), &__CFBasicHashTotalSize);
    while (__CFBasicHashPeakSize < size_now && !OSAtomicCompareAndSwap64Barrier(__CFBasicHashPeakSize, size_now, & __CFBasicHashPeakSize));
    OSAtomicAdd32Barrier(1, &__CFBasicHashSizes[ht->bits.num_buckets_idx]);
#endif
}

CF_PRIVATE void CFBasicHashSetCapacity(CFBasicHashRef ht, CFIndex capacity) {
    if (!CFBasicHashIsMutable(ht)) HALT;
    if (ht->bits.used_buckets < capacity) {
        ht->bits.mutations++;
        __CFBasicHashRehash(ht, capacity - ht->bits.used_buckets);
    }
}

static void __CFBasicHashAddValue(CFBasicHashRef ht, CFIndex bkt_idx, uintptr_t stack_key, uintptr_t stack_value) {
    ht->bits.mutations++;
    if (CFBasicHashGetCapacity(ht) < ht->bits.used_buckets + 1) {
        __CFBasicHashRehash(ht, 1);
        bkt_idx = __CFBasicHashFindBucket_NoCollision(ht, stack_key, 0);
    } else if (__CFBasicHashIsDeleted(ht, bkt_idx)) {
        ht->bits.deleted--;
    }
    uintptr_t key_hash = 0;
    if (__CFBasicHashHasHashCache(ht)) {
        key_hash = __CFBasicHashHashKey(ht, stack_key);
    }
    stack_value = __CFBasicHashImportValue(ht, stack_value);
    if (ht->bits.keys_offset) {
        stack_key = __CFBasicHashImportKey(ht, stack_key);
    }
    __CFBasicHashSetValue(ht, bkt_idx, stack_value, false, false);
    if (ht->bits.keys_offset) {
        __CFBasicHashSetKey(ht, bkt_idx, stack_key, false, false);
    }
    if (ht->bits.counts_offset) {
        __CFBasicHashIncSlotCount(ht, bkt_idx);
    }
    if (__CFBasicHashHasHashCache(ht)) {
        __CFBasicHashGetHashes(ht)[bkt_idx] = key_hash;
    }
    ht->bits.used_buckets++;
}

static void __CFBasicHashReplaceValue(CFBasicHashRef ht, CFIndex bkt_idx, uintptr_t stack_key, uintptr_t stack_value) {
    ht->bits.mutations++;
    stack_value = __CFBasicHashImportValue(ht, stack_value);
    if (ht->bits.keys_offset) {
        stack_key = __CFBasicHashImportKey(ht, stack_key);
    }
    __CFBasicHashSetValue(ht, bkt_idx, stack_value, false, false);
    if (ht->bits.keys_offset) {
        __CFBasicHashSetKey(ht, bkt_idx, stack_key, false, false);
    }
}

static void __CFBasicHashRemoveValue(CFBasicHashRef ht, CFIndex bkt_idx) {
    ht->bits.mutations++;
    __CFBasicHashSetValue(ht, bkt_idx, ~0UL, false, true);
    if (ht->bits.keys_offset) {
        __CFBasicHashSetKey(ht, bkt_idx, ~0UL, false, true);
    }
    if (ht->bits.counts_offset) {
        __CFBasicHashDecSlotCount(ht, bkt_idx);
    }
    if (__CFBasicHashHasHashCache(ht)) {
        __CFBasicHashGetHashes(ht)[bkt_idx] = 0;
    }
    ht->bits.used_buckets--;
    ht->bits.deleted++;
    Boolean do_shrink = false;
    if (ht->bits.fast_grow) { // == slow shrink
        do_shrink = (5 < ht->bits.num_buckets_idx && ht->bits.used_buckets < __CFBasicHashGetCapacityForNumBuckets(ht, ht->bits.num_buckets_idx - 5));
    } else {
        do_shrink = (2 < ht->bits.num_buckets_idx && ht->bits.used_buckets < __CFBasicHashGetCapacityForNumBuckets(ht, ht->bits.num_buckets_idx - 2));
    }
    if (do_shrink) {
        __CFBasicHashRehash(ht, -1);
        return;
    }
    do_shrink = (0 == ht->bits.deleted); // .deleted roll-over
    CFIndex num_buckets = __CFBasicHashTableSizes[ht->bits.num_buckets_idx];
    do_shrink = do_shrink || ((20 <= num_buckets) && (num_buckets / 4 <= ht->bits.deleted));
    if (do_shrink) {
        __CFBasicHashRehash(ht, 0);
    }
}

CF_PRIVATE Boolean CFBasicHashAddValue(CFBasicHashRef ht, uintptr_t stack_key, uintptr_t stack_value) {
    if (!CFBasicHashIsMutable(ht)) HALT;
    if (__CFBasicHashSubABZero == stack_key) HALT;
    if (__CFBasicHashSubABOne == stack_key) HALT;
    if (__CFBasicHashSubABZero == stack_value) HALT;
    if (__CFBasicHashSubABOne == stack_value) HALT;
    CFBasicHashBucket bkt = __CFBasicHashFindBucket(ht, stack_key);
    if (0 < bkt.count) {
        ht->bits.mutations++;
        if (ht->bits.counts_offset && bkt.count < LONG_MAX) { // if not yet as large as a CFIndex can be... otherwise clamp and do nothing
            __CFBasicHashIncSlotCount(ht, bkt.idx);
            return true;
        }
    } else {
        __CFBasicHashAddValue(ht, bkt.idx, stack_key, stack_value);
        return true;
    }
    return false;
}

CF_PRIVATE void CFBasicHashReplaceValue(CFBasicHashRef ht, uintptr_t stack_key, uintptr_t stack_value) {
    if (!CFBasicHashIsMutable(ht)) HALT;
    if (__CFBasicHashSubABZero == stack_key) HALT;
    if (__CFBasicHashSubABOne == stack_key) HALT;
    if (__CFBasicHashSubABZero == stack_value) HALT;
    if (__CFBasicHashSubABOne == stack_value) HALT;
    CFBasicHashBucket bkt = __CFBasicHashFindBucket(ht, stack_key);
    if (0 < bkt.count) {
        __CFBasicHashReplaceValue(ht, bkt.idx, stack_key, stack_value);
    }
}

CF_PRIVATE void CFBasicHashSetValue(CFBasicHashRef ht, uintptr_t stack_key, uintptr_t stack_value) {
    if (!CFBasicHashIsMutable(ht)) HALT;
    if (__CFBasicHashSubABZero == stack_key) HALT;
    if (__CFBasicHashSubABOne == stack_key) HALT;
    if (__CFBasicHashSubABZero == stack_value) HALT;
    if (__CFBasicHashSubABOne == stack_value) HALT;
    CFBasicHashBucket bkt = __CFBasicHashFindBucket(ht, stack_key);
    if (0 < bkt.count) {
        __CFBasicHashReplaceValue(ht, bkt.idx, stack_key, stack_value);
    } else {
        __CFBasicHashAddValue(ht, bkt.idx, stack_key, stack_value);
    }
}

CF_PRIVATE CFIndex CFBasicHashRemoveValue(CFBasicHashRef ht, uintptr_t stack_key) {
    if (!CFBasicHashIsMutable(ht)) HALT;
    if (__CFBasicHashSubABZero == stack_key || __CFBasicHashSubABOne == stack_key) return 0;
    CFBasicHashBucket bkt = __CFBasicHashFindBucket(ht, stack_key);
    if (1 < bkt.count) {
        ht->bits.mutations++;
        if (ht->bits.counts_offset && bkt.count < LONG_MAX) { // if not as large as a CFIndex can be... otherwise clamp and do nothing
            __CFBasicHashDecSlotCount(ht, bkt.idx);
        }
    } else if (0 < bkt.count) {
        __CFBasicHashRemoveValue(ht, bkt.idx);
    }
    return bkt.count;
}

CF_PRIVATE CFIndex CFBasicHashRemoveValueAtIndex(CFBasicHashRef ht, CFIndex idx) {
    if (!CFBasicHashIsMutable(ht)) HALT;
    CFBasicHashBucket bkt = CFBasicHashGetBucket(ht, idx);
    if (1 < bkt.count) {
        ht->bits.mutations++;
        if (ht->bits.counts_offset && bkt.count < LONG_MAX) { // if not as large as a CFIndex can be... otherwise clamp and do nothing
            __CFBasicHashDecSlotCount(ht, bkt.idx);
        }
    } else if (0 < bkt.count) {
        __CFBasicHashRemoveValue(ht, bkt.idx);
    }
    return bkt.count;
}

CF_PRIVATE void CFBasicHashRemoveAllValues(CFBasicHashRef ht) {
    if (!CFBasicHashIsMutable(ht)) HALT;
    if (0 == ht->bits.num_buckets_idx) return;
    __CFBasicHashDrain(ht, false);
}

CF_PRIVATE Boolean CFBasicHashAddIntValueAndInc(CFBasicHashRef ht, uintptr_t stack_key, uintptr_t int_value) {
    if (!CFBasicHashIsMutable(ht)) HALT;
    if (__CFBasicHashSubABZero == stack_key) HALT;
    if (__CFBasicHashSubABOne == stack_key) HALT;
    if (__CFBasicHashSubABZero == int_value) HALT;
    if (__CFBasicHashSubABOne == int_value) HALT;
    CFBasicHashBucket bkt = __CFBasicHashFindBucket(ht, stack_key);
    if (0 < bkt.count) {
        ht->bits.mutations++;
    } else {
        // must rehash before renumbering
        if (CFBasicHashGetCapacity(ht) < ht->bits.used_buckets + 1) {
            __CFBasicHashRehash(ht, 1);
            bkt.idx = __CFBasicHashFindBucket_NoCollision(ht, stack_key, 0);
        }
        CFIndex cnt = (CFIndex)__CFBasicHashTableSizes[ht->bits.num_buckets_idx];
        for (CFIndex idx = 0; idx < cnt; idx++) {
            if (!__CFBasicHashIsEmptyOrDeleted(ht, idx)) {
                uintptr_t stack_value = __CFBasicHashGetValue(ht, idx);
                if (int_value <= stack_value) {
                    stack_value++;
                    __CFBasicHashSetValue(ht, idx, stack_value, true, false);
                    ht->bits.mutations++;
                }
            }
        }
        __CFBasicHashAddValue(ht, bkt.idx, stack_key, int_value);
        return true;
    }
    return false;
}

CF_PRIVATE void CFBasicHashRemoveIntValueAndDec(CFBasicHashRef ht, uintptr_t int_value) {
    if (!CFBasicHashIsMutable(ht)) HALT;
    if (__CFBasicHashSubABZero == int_value) HALT;
    if (__CFBasicHashSubABOne == int_value) HALT;
    uintptr_t bkt_idx = ~0UL;
    CFIndex cnt = (CFIndex)__CFBasicHashTableSizes[ht->bits.num_buckets_idx];
    for (CFIndex idx = 0; idx < cnt; idx++) {
        if (!__CFBasicHashIsEmptyOrDeleted(ht, idx)) {
            uintptr_t stack_value = __CFBasicHashGetValue(ht, idx);
            if (int_value == stack_value) {
                bkt_idx = idx;
            }
            if (int_value < stack_value) {
                stack_value--;
                __CFBasicHashSetValue(ht, idx, stack_value, true, false);
                ht->bits.mutations++;
            }
        }
    }
    __CFBasicHashRemoveValue(ht, bkt_idx);
}

CF_PRIVATE size_t CFBasicHashGetSize(CFConstBasicHashRef ht, Boolean total) {
    size_t size = sizeof(struct __CFBasicHash);
    if (ht->bits.keys_offset) size += sizeof(CFBasicHashValue *);
    if (ht->bits.counts_offset) size += sizeof(void *);
    if (__CFBasicHashHasHashCache(ht)) size += sizeof(uintptr_t *);
    if (total) {
        CFIndex num_buckets = __CFBasicHashTableSizes[ht->bits.num_buckets_idx];
        if (0 < num_buckets) {
            size += malloc_size(__CFBasicHashGetValues(ht));
            if (ht->bits.keys_offset) size += malloc_size(__CFBasicHashGetKeys(ht));
            if (ht->bits.counts_offset) size += malloc_size(__CFBasicHashGetCounts(ht));
            if (__CFBasicHashHasHashCache(ht)) size += malloc_size(__CFBasicHashGetHashes(ht));
        }
    }
    return size;
}

CF_PRIVATE CFStringRef CFBasicHashCopyDescription(CFConstBasicHashRef ht, Boolean detailed, CFStringRef prefix, CFStringRef entryPrefix, Boolean describeElements) {
    CFMutableStringRef result = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    CFStringAppendFormat(result, NULL, CFSTR("%@{type = %s %s%s, count = %ld,\n"), prefix, (CFBasicHashIsMutable(ht) ? "mutable" : "immutable"), ((ht->bits.counts_offset) ? "multi" : ""), ((ht->bits.keys_offset) ? "dict" : "set"), CFBasicHashGetCount(ht));
    if (detailed) {
        const char *cb_type = "custom";
        CFStringAppendFormat(result, NULL, CFSTR("%@hash cache = %s, strong values = %s, strong keys = %s, cb = %s,\n"), prefix, (__CFBasicHashHasHashCache(ht) ? "yes" : "no"), (CFBasicHashHasStrongValues(ht) ? "yes" : "no"), (CFBasicHashHasStrongKeys(ht) ? "yes" : "no"), cb_type);
        CFStringAppendFormat(result, NULL, CFSTR("%@num bucket index = %d, num buckets = %ld, capacity = %ld, num buckets used = %u,\n"), prefix, ht->bits.num_buckets_idx, CFBasicHashGetNumBuckets(ht), (long)CFBasicHashGetCapacity(ht), ht->bits.used_buckets);
        CFStringAppendFormat(result, NULL, CFSTR("%@counts width = %d, finalized = %s,\n"), prefix,((ht->bits.counts_offset) ? (1 << ht->bits.counts_width) : 0), (ht->bits.finalized ? "yes" : "no"));
        CFStringAppendFormat(result, NULL, CFSTR("%@num mutations = %ld, num deleted = %ld, size = %ld, total size = %ld,\n"), prefix, (long)ht->bits.mutations, (long)ht->bits.deleted, CFBasicHashGetSize(ht, false), CFBasicHashGetSize(ht, true));
        CFStringAppendFormat(result, NULL, CFSTR("%@values ptr = %p, keys ptr = %p, counts ptr = %p, hashes ptr = %p,\n"), prefix, __CFBasicHashGetValues(ht), ((ht->bits.keys_offset) ? __CFBasicHashGetKeys(ht) : NULL), ((ht->bits.counts_offset) ? __CFBasicHashGetCounts(ht) : NULL), (__CFBasicHashHasHashCache(ht) ? __CFBasicHashGetHashes(ht) : NULL));
    }
    CFStringAppendFormat(result, NULL, CFSTR("%@entries =>\n"), prefix);
    CFBasicHashApply(ht, ^(CFBasicHashBucket bkt) {
            CFStringRef vDesc = NULL, kDesc = NULL;
            if (!describeElements) {
                vDesc = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<%p>"), (void *)bkt.weak_value);
                if (ht->bits.keys_offset) {
                    kDesc = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<%p>"), (void *)bkt.weak_key);
                }
            } else {
                vDesc = __CFBasicHashDescValue(ht, bkt.weak_value);
                if (ht->bits.keys_offset) {
                    kDesc = __CFBasicHashDescKey(ht, bkt.weak_key);
                }
            }
            if (ht->bits.keys_offset && ht->bits.counts_offset) {
                CFStringAppendFormat(result, NULL, CFSTR("%@%ld : %@ = %@ (%ld)\n"), entryPrefix, bkt.idx, kDesc, vDesc, bkt.count);
            } else if (ht->bits.keys_offset) {
                CFStringAppendFormat(result, NULL, CFSTR("%@%ld : %@ = %@\n"), entryPrefix, bkt.idx, kDesc, vDesc);
            } else if (ht->bits.counts_offset) {
                CFStringAppendFormat(result, NULL, CFSTR("%@%ld : %@ (%ld)\n"), entryPrefix, bkt.idx, vDesc, bkt.count);
            } else {
                CFStringAppendFormat(result, NULL, CFSTR("%@%ld : %@\n"), entryPrefix, bkt.idx, vDesc);
            }
            if (kDesc) CFRelease(kDesc);
            if (vDesc) CFRelease(vDesc);
            return (Boolean)true;
        });
    CFStringAppendFormat(result, NULL, CFSTR("%@}\n"), prefix);
    return result;
}

CF_PRIVATE void CFBasicHashShow(CFConstBasicHashRef ht) {
    CFStringRef str = CFBasicHashCopyDescription(ht, true, CFSTR(""), CFSTR("\t"), false);
    CFShow(str);
    CFRelease(str);
}

CF_PRIVATE Boolean __CFBasicHashEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFBasicHashRef ht1 = (CFBasicHashRef)cf1;
    CFBasicHashRef ht2 = (CFBasicHashRef)cf2;
//#warning this used to require that the key and value equal callbacks were pointer identical
    return CFBasicHashesAreEqual(ht1, ht2);
}

CF_PRIVATE CFHashCode __CFBasicHashHash(CFTypeRef cf) {
    CFBasicHashRef ht = (CFBasicHashRef)cf;
    return CFBasicHashGetCount(ht);
}

CF_PRIVATE CFStringRef __CFBasicHashCopyDescription(CFTypeRef cf) {
    CFBasicHashRef ht = (CFBasicHashRef)cf;
    CFStringRef desc = CFBasicHashCopyDescription(ht, false, CFSTR(""), CFSTR("\t"), true);
    CFStringRef result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFBasicHash %p [%p]>%@"), cf, CFGetAllocator(cf), desc);
    CFRelease(desc);
    return result;
}

CF_PRIVATE void __CFBasicHashDeallocate(CFTypeRef cf) {
    CFBasicHashRef ht = (CFBasicHashRef)cf;
    if (ht->bits.finalized) HALT;
    ht->bits.finalized = 1;
    __CFBasicHashDrain(ht, true);
#if ENABLE_MEMORY_COUNTERS
    OSAtomicAdd64Barrier(-1, &__CFBasicHashTotalCount);
    OSAtomicAdd32Barrier(-1, &__CFBasicHashSizes[ht->bits.num_buckets_idx]);
#endif
}

static CFTypeID __kCFBasicHashTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFBasicHashClass = {
    _kCFRuntimeScannedObject,
    "CFBasicHash",
    NULL,        // init
    NULL,        // copy
    __CFBasicHashDeallocate,
    __CFBasicHashEqual,
    __CFBasicHashHash,
    NULL,        //
    __CFBasicHashCopyDescription
};

CF_PRIVATE CFTypeID CFBasicHashGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ __kCFBasicHashTypeID = _CFRuntimeRegisterClass(&__CFBasicHashClass); });
    return __kCFBasicHashTypeID;
}

CF_PRIVATE CFBasicHashRef CFBasicHashCreate(CFAllocatorRef allocator, CFOptionFlags flags, const CFBasicHashCallbacks *cb) {
    size_t size = sizeof(struct __CFBasicHash) - sizeof(CFRuntimeBase);
    if (flags & kCFBasicHashHasKeys) size += sizeof(CFBasicHashValue *); // keys
    if (flags & kCFBasicHashHasCounts) size += sizeof(void *); // counts
    if (flags & kCFBasicHashHasHashCache) size += sizeof(uintptr_t *); // hashes
    CFBasicHashRef ht = (CFBasicHashRef)_CFRuntimeCreateInstance(allocator, CFBasicHashGetTypeID(), size, NULL);
    if (NULL == ht) return NULL;

    ht->bits.finalized = 0;
    ht->bits.hash_style = (flags >> 13) & 0x3;
    ht->bits.fast_grow = (flags & kCFBasicHashAggressiveGrowth) ? 1 : 0;
    ht->bits.counts_width = 0;
    ht->bits.strong_values = (flags & kCFBasicHashStrongValues) ? 1 : 0;
    ht->bits.strong_keys = (flags & kCFBasicHashStrongKeys) ? 1 : 0;
    ht->bits.weak_values = (flags & kCFBasicHashWeakValues) ? 1 : 0;
    ht->bits.weak_keys = (flags & kCFBasicHashWeakKeys) ? 1 : 0;
    ht->bits.int_values = (flags & kCFBasicHashIntegerValues) ? 1 : 0;
    ht->bits.int_keys = (flags & kCFBasicHashIntegerKeys) ? 1 : 0;
    ht->bits.indirect_keys = (flags & kCFBasicHashIndirectKeys) ? 1 : 0;
    ht->bits.num_buckets_idx = 0;
    ht->bits.used_buckets = 0;
    ht->bits.deleted = 0;
    ht->bits.mutations = 1;

    if (ht->bits.strong_values && ht->bits.weak_values) HALT;
    if (ht->bits.strong_values && ht->bits.int_values) HALT;
    if (ht->bits.strong_keys && ht->bits.weak_keys) HALT;
    if (ht->bits.strong_keys && ht->bits.int_keys) HALT;
    if (ht->bits.weak_values && ht->bits.int_values) HALT;
    if (ht->bits.weak_keys && ht->bits.int_keys) HALT;
    if (ht->bits.indirect_keys && ht->bits.strong_keys) HALT;
    if (ht->bits.indirect_keys && ht->bits.weak_keys) HALT;
    if (ht->bits.indirect_keys && ht->bits.int_keys) HALT;

    uint64_t offset = 1;
    ht->bits.keys_offset = (flags & kCFBasicHashHasKeys) ? offset++ : 0;
    ht->bits.counts_offset = (flags & kCFBasicHashHasCounts) ? offset++ : 0;
    ht->bits.hashes_offset = (flags & kCFBasicHashHasHashCache) ? offset++ : 0;

#if DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    ht->bits.hashes_offset = 0;
    ht->bits.strong_values = 0;
    ht->bits.strong_keys = 0;
    ht->bits.weak_values = 0;
    ht->bits.weak_keys = 0;
#endif

    ht->bits.__kret = CFBasicHashGetPtrIndex((void *)cb->retainKey);
    ht->bits.__vret = CFBasicHashGetPtrIndex((void *)cb->retainValue);
    ht->bits.__krel = CFBasicHashGetPtrIndex((void *)cb->releaseKey);
    ht->bits.__vrel = CFBasicHashGetPtrIndex((void *)cb->releaseValue);
    ht->bits.__kdes = CFBasicHashGetPtrIndex((void *)cb->copyKeyDescription);
    ht->bits.__vdes = CFBasicHashGetPtrIndex((void *)cb->copyValueDescription);
    ht->bits.__kequ = CFBasicHashGetPtrIndex((void *)cb->equateKeys);
    ht->bits.__vequ = CFBasicHashGetPtrIndex((void *)cb->equateValues);
    ht->bits.__khas = CFBasicHashGetPtrIndex((void *)cb->hashKey);
    ht->bits.__kget = CFBasicHashGetPtrIndex((void *)cb->getIndirectKey);

    for (CFIndex idx = 0; idx < offset; idx++) {
        ht->pointers[idx] = NULL;
    }

#if ENABLE_MEMORY_COUNTERS
    int64_t size_now = OSAtomicAdd64Barrier((int64_t) CFBasicHashGetSize(ht, true), & __CFBasicHashTotalSize);
    while (__CFBasicHashPeakSize < size_now && !OSAtomicCompareAndSwap64Barrier(__CFBasicHashPeakSize, size_now, & __CFBasicHashPeakSize));
    int64_t count_now = OSAtomicAdd64Barrier(1, & __CFBasicHashTotalCount);
    while (__CFBasicHashPeakCount < count_now && !OSAtomicCompareAndSwap64Barrier(__CFBasicHashPeakCount, count_now, & __CFBasicHashPeakCount));
    OSAtomicAdd32Barrier(1, &__CFBasicHashSizes[ht->bits.num_buckets_idx]);
#endif

    return ht;
}

CF_PRIVATE CFBasicHashRef CFBasicHashCreateCopy(CFAllocatorRef allocator, CFConstBasicHashRef src_ht) {
    size_t size = CFBasicHashGetSize(src_ht, false) - sizeof(CFRuntimeBase);
    CFIndex new_num_buckets = __CFBasicHashTableSizes[src_ht->bits.num_buckets_idx];
    CFBasicHashValue *new_values = NULL, *new_keys = NULL;
    void *new_counts = NULL;
    uintptr_t *new_hashes = NULL;

    if (0 < new_num_buckets) {
        Boolean strongValues = CFBasicHashHasStrongValues(src_ht) && !(kCFUseCollectableAllocator && !CF_IS_COLLECTABLE_ALLOCATOR(allocator));
        Boolean strongKeys = CFBasicHashHasStrongKeys(src_ht) && !(kCFUseCollectableAllocator && !CF_IS_COLLECTABLE_ALLOCATOR(allocator));
        new_values = (CFBasicHashValue *)__CFBasicHashAllocateMemory2(allocator, new_num_buckets, sizeof(CFBasicHashValue), strongValues, 0);
        if (!new_values) return NULL; // in this unusual circumstance, leak previously allocated blocks for now
        __SetLastAllocationEventName(new_values, "CFBasicHash (value-store)");
        if (src_ht->bits.keys_offset) {
            new_keys = (CFBasicHashValue *)__CFBasicHashAllocateMemory2(allocator, new_num_buckets, sizeof(CFBasicHashValue), strongKeys, false);
            if (!new_keys) return NULL; // in this unusual circumstance, leak previously allocated blocks for now
            __SetLastAllocationEventName(new_keys, "CFBasicHash (key-store)");
        }
        if (src_ht->bits.counts_offset) {
            new_counts = (uintptr_t *)__CFBasicHashAllocateMemory2(allocator, new_num_buckets, (1 << src_ht->bits.counts_width), false, false);
            if (!new_counts) return NULL; // in this unusual circumstance, leak previously allocated blocks for now
            __SetLastAllocationEventName(new_counts, "CFBasicHash (count-store)");
        }
        if (__CFBasicHashHasHashCache(src_ht)) {
            new_hashes = (uintptr_t *)__CFBasicHashAllocateMemory2(allocator, new_num_buckets, sizeof(uintptr_t), false, false);
            if (!new_hashes) return NULL; // in this unusual circumstance, leak previously allocated blocks for now
            __SetLastAllocationEventName(new_hashes, "CFBasicHash (hash-store)");
        }
    }

    CFBasicHashRef ht = (CFBasicHashRef)_CFRuntimeCreateInstance(allocator, CFBasicHashGetTypeID(), size, NULL);
    if (NULL == ht) return NULL; // in this unusual circumstance, leak previously allocated blocks for now

    memmove((uint8_t *)ht + sizeof(CFRuntimeBase), (uint8_t *)src_ht + sizeof(CFRuntimeBase), sizeof(ht->bits));
    if (kCFUseCollectableAllocator && !CF_IS_COLLECTABLE_ALLOCATOR(allocator)) {
        ht->bits.strong_values = 0;
        ht->bits.strong_keys = 0;
        ht->bits.weak_values = 0;
        ht->bits.weak_keys = 0;
    }
    ht->bits.finalized = 0;
    ht->bits.mutations = 1;

    if (0 == new_num_buckets) {
#if ENABLE_MEMORY_COUNTERS
        int64_t size_now = OSAtomicAdd64Barrier((int64_t) CFBasicHashGetSize(ht, true), & __CFBasicHashTotalSize);
        while (__CFBasicHashPeakSize < size_now && !OSAtomicCompareAndSwap64Barrier(__CFBasicHashPeakSize, size_now, & __CFBasicHashPeakSize));
        int64_t count_now = OSAtomicAdd64Barrier(1, & __CFBasicHashTotalCount);
        while (__CFBasicHashPeakCount < count_now && !OSAtomicCompareAndSwap64Barrier(__CFBasicHashPeakCount, count_now, & __CFBasicHashPeakCount));
        OSAtomicAdd32Barrier(1, &__CFBasicHashSizes[ht->bits.num_buckets_idx]);
#endif
        return ht;
    }

    CFBasicHashValue *old_values = NULL, *old_keys = NULL;
    void *old_counts = NULL;
    uintptr_t *old_hashes = NULL;

    old_values = __CFBasicHashGetValues(src_ht);
    if (src_ht->bits.keys_offset) {
        old_keys = __CFBasicHashGetKeys(src_ht);
    }
    if (src_ht->bits.counts_offset) {
        old_counts = __CFBasicHashGetCounts(src_ht);
    }
    if (__CFBasicHashHasHashCache(src_ht)) {
        old_hashes = __CFBasicHashGetHashes(src_ht);
    }

    __CFBasicHashSetValues(ht, new_values);
    if (new_keys) {
        __CFBasicHashSetKeys(ht, new_keys);
    }
    if (new_counts) {
        __CFBasicHashSetCounts(ht, new_counts);
    }
    if (new_hashes) {
        __CFBasicHashSetHashes(ht, new_hashes);
    }

    for (CFIndex idx = 0; idx < new_num_buckets; idx++) {
        uintptr_t stack_value = old_values[idx].neutral;
        if (stack_value != 0UL && stack_value != ~0UL) {
            uintptr_t old_value = stack_value;
            if (__CFBasicHashSubABZero == old_value) old_value = 0UL;
            if (__CFBasicHashSubABOne == old_value) old_value = ~0UL;
            __CFBasicHashSetValue(ht, idx, __CFBasicHashImportValue(ht, old_value), true, false);
            if (new_keys) {
                uintptr_t old_key = old_keys[idx].neutral;
                if (__CFBasicHashSubABZero == old_key) old_key = 0UL;
                if (__CFBasicHashSubABOne == old_key) old_key = ~0UL;
                __CFBasicHashSetKey(ht, idx, __CFBasicHashImportKey(ht, old_key), true, false);
            }
        } else {
            __CFBasicHashSetValue(ht, idx, stack_value, true, true);
            if (new_keys) {
                __CFBasicHashSetKey(ht, idx, stack_value, true, true);
            }
        }
    }
    if (new_counts) memmove(new_counts, old_counts, new_num_buckets * (1 << ht->bits.counts_width));
    if (new_hashes) memmove(new_hashes, old_hashes, new_num_buckets * sizeof(uintptr_t));

#if ENABLE_MEMORY_COUNTERS
    int64_t size_now = OSAtomicAdd64Barrier((int64_t) CFBasicHashGetSize(ht, true), & __CFBasicHashTotalSize);
    while (__CFBasicHashPeakSize < size_now && !OSAtomicCompareAndSwap64Barrier(__CFBasicHashPeakSize, size_now, & __CFBasicHashPeakSize));
    int64_t count_now = OSAtomicAdd64Barrier(1, & __CFBasicHashTotalCount);
    while (__CFBasicHashPeakCount < count_now && !OSAtomicCompareAndSwap64Barrier(__CFBasicHashPeakCount, count_now, & __CFBasicHashPeakCount));
    OSAtomicAdd32Barrier(1, &__CFBasicHashSizes[ht->bits.num_buckets_idx]);
#endif

    return ht;
}


