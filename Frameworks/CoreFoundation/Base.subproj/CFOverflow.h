/*	CFOverflow.h
	Copyright (c) 2017-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2017-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#ifndef CFOverflow_h
#define CFOverflow_h

#include <CoreFoundation/CFBase.h>

#if __has_include(<os/overflow.h>)
#include <os/overflow.h>
#else
    static _Bool __os_warn_unused(_Bool x) __attribute__((__warn_unused_result__));
    static _Bool __os_warn_unused(_Bool x) { return x; }

    #if __has_builtin(__builtin_add_overflow) && \
    __has_builtin(__builtin_sub_overflow) && \
    __has_builtin(__builtin_mul_overflow)

        #define os_add_overflow(a, b, res) __os_warn_unused(__builtin_add_overflow((a), (b), (res)))
        #define os_sub_overflow(a, b, res) __os_warn_unused(__builtin_sub_overflow((a), (b), (res)))
        #define os_mul_overflow(a, b, res) __os_warn_unused(__builtin_mul_overflow((a), (b), (res)))

    #else
        #error Missing compiler support for overflow checking
    #endif
#endif // __has_include(<os/overflow.h>)

typedef CF_ENUM(uint8_t, _CFOverflowResult) {
    _CFOverflowResultOK = 0,
    _CFOverflowResultNegativeParameters,
    _CFOverflowResultOverflows,
};

// Overflow utilities for positive integers
CF_INLINE _CFOverflowResult _CFPositiveIntegerProductWouldOverflow(CFIndex si_a, CFIndex si_b, CFIndex * /*_Nullable*/ outSum) {
    _CFOverflowResult result = _CFOverflowResultOK;
    CFIndex sum = 0;
    if (si_a < 0 || si_b < 0) {
        // we explicitly only implement a subset of the overflow checking, so report failure if out of domain
        result = _CFOverflowResultNegativeParameters;
    } else {
        if (os_mul_overflow(si_a, si_b, &sum)) {
            result = _CFOverflowResultOverflows;
        }
    }
    if (outSum) {
        *outSum = sum;
    }
    return result;
}

CF_INLINE _CFOverflowResult _CFPointerSumWouldOverflow(void const *p, size_t n, void * /*_Nullable*/ * /*_Nullable*/ outSum) {
    _CFOverflowResult result = _CFOverflowResultOK;
#if TARGET_RT_64_BIT
    uint64_t sum = 0;
    uint64_t const lhs = (uint64_t)p;
    uint64_t const rhs = (uint64_t)n;
#else
    uint32_t sum = 0;
    uint32_t const lhs = (uint32_t)p;
    uint32_t const rhs = (uint32_t)n;
#endif
    if (os_add_overflow(lhs, rhs, &sum)) {
        result = _CFOverflowResultOverflows;
    }
    if (outSum) {
        *outSum = (void *)sum;
    }
    return result;
}

#if TARGET_OS_WIN32
CF_INLINE bool _CFMultiplyBufferSizeWithoutOverflow(size_t a, size_t b, size_t *res) {
    int32_t res32 = 0;
    if (!os_mul_overflow((int32_t)a, (int32_t)b, &res32)) {
        *res = res32;
        return true;
    } else {
        return false;
    }
}
#else
#define _CFMultiplyBufferSizeWithoutOverflow(a, b, res) (os_mul_overflow((a), (b), (res)) == 0)
#endif

#endif /* CFOverflow_h */
