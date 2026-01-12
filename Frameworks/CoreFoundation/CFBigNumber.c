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

/*	CFBigNumber.c
	Copyright (c) 2012-2014, Apple Inc. All rights reserved.
	Responsibility: Christopher Kane
	Original author: Zhi Feng Huang
*/

#include <CoreFoundation/CFBigNumber.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "CFInternal.h"


typedef struct {
    int64_t high;
    uint64_t low;
} CFSInt128Struct;

#define kCFNumberSInt128Type 17

CF_EXPORT CFNumberType _CFNumberGetType2(CFNumberRef number);

#if __LP64__

#ifndef _INT128_T
#define _INT128_T
typedef __int128_t  int128_t;
#endif

#ifndef _UINT128_T
#define _UINT128_T
typedef __uint128_t uint128_t;
#endif

#ifndef INT128_MIN
#define INT128_MIN  ((__int128_t)0 - ((__int128_t)1 << 126) - ((__int128_t)1 << 126))
#endif

#ifndef INT128_MAX
#define INT128_MAX  ((__int128_t)-1 + ((__int128_t)1 << 126) + ((__int128_t)1 << 126))
#endif

#ifndef UINT128_MAX
#define UINT128_MAX (((__uint128_t)1 << 127) - (__uint128_t)1 + ((__uint128_t)1 << 127))
#endif

#endif


#define BIG_DIGITS_LIMIT    1000000000L
#define BIG_DIGITS_LIMIT_2  ((uint64_t)BIG_DIGITS_LIMIT * (uint64_t)BIG_DIGITS_LIMIT)
#define BIG_DIGITS_LIMIT_3  ((__uint128_t)BIG_DIGITS_LIMIT_2 * (__uint128_t)BIG_DIGITS_LIMIT)
#define BIG_DIGITS_LIMIT_4  ((__uint128_t)BIG_DIGITS_LIMIT_3 * (__uint128_t)BIG_DIGITS_LIMIT)

#define GET_FIFTH_DIGIT(A)  (A / BIG_DIGITS_LIMIT_4)
#define GET_FOURTH_DIGIT(A) (A / BIG_DIGITS_LIMIT_3)
#define GET_THIRD_DIGIT(A)  (A / BIG_DIGITS_LIMIT_2)
#define GET_SECOND_DIGIT(A) (A / BIG_DIGITS_LIMIT)

#define GET_REMAINDER_FIFTH_DIGIT(A,B)  (A - B * BIG_DIGITS_LIMIT_4)
#define GET_REMAINDER_FOURTH_DIGIT(A,B) (A - B * BIG_DIGITS_LIMIT_3)
#define GET_REMAINDER_THIRD_DIGIT(A,B)  (A - B * BIG_DIGITS_LIMIT_2)
#define GET_REMAINDER_SECOND_DIGIT(A,B) (A - B * BIG_DIGITS_LIMIT)


void _CFBigNumInitWithInt8(_CFBigNum *r, int8_t inNum) {
    memset(r, 0, sizeof(*r));
    uint8_t unsignInNum = inNum;
    if (inNum < 0) {
        r->sign = -1;
        unsignInNum = -1 * inNum;
    }
    r->digits[0] = unsignInNum;
}

void _CFBigNumInitWithInt16(_CFBigNum *r, int16_t inNum) {
    memset(r, 0, sizeof(*r));
    uint16_t unsignInNum = inNum;
    if (inNum < 0) {
        r->sign = -1;
        unsignInNum = -1 * inNum;
    }
    r->digits[0] = unsignInNum;
}

void _CFBigNumInitWithInt32(_CFBigNum *r, int32_t inNum) {
    memset(r, 0, sizeof(*r));
    uint32_t unsignInNum = inNum;
    if (inNum < 0) {
        r->sign = -1;
        unsignInNum = -1 * inNum;
    }
    uint32_t dig0 = GET_SECOND_DIGIT(unsignInNum);
    r->digits[0] = GET_REMAINDER_SECOND_DIGIT(unsignInNum, dig0);
    r->digits[1] = dig0;
}

void _CFBigNumInitWithInt64(_CFBigNum *r, int64_t inNum) {
    memset(r, 0, sizeof(*r));
    uint64_t unsignInNum = inNum;
    if (inNum < 0) {
        r->sign = -1;
        unsignInNum = -1 * inNum;
    }
    uint32_t dig2 = GET_THIRD_DIGIT(unsignInNum);
    unsignInNum = GET_REMAINDER_THIRD_DIGIT(unsignInNum, (uint64_t)dig2);
    uint32_t dig1 = GET_SECOND_DIGIT(unsignInNum);
    r->digits[0] = GET_REMAINDER_SECOND_DIGIT(unsignInNum, (uint64_t)dig1);
    r->digits[1] = dig1;
    r->digits[2] = dig2;
}

#ifdef __LP64__
void _CFBigNumInitWithInt128(_CFBigNum *r, __int128_t inNum) {
    memset(r, 0, sizeof(*r));
    __uint128_t unsignInNum = inNum;
    if (inNum < 0) {
        r->sign = -1;
        unsignInNum = -1 * inNum;
    }
    uint32_t dig4 = GET_FIFTH_DIGIT(unsignInNum);
    unsignInNum = GET_REMAINDER_FIFTH_DIGIT(unsignInNum, (__uint128_t)dig4);
    uint32_t dig3 = GET_FOURTH_DIGIT(unsignInNum);
    unsignInNum = GET_REMAINDER_FOURTH_DIGIT(unsignInNum, (__uint128_t)dig3);
    uint32_t dig2 = GET_THIRD_DIGIT(unsignInNum);
    unsignInNum = GET_REMAINDER_THIRD_DIGIT(unsignInNum, (__uint128_t)dig2);
    uint32_t dig1 = GET_SECOND_DIGIT(unsignInNum);
    r->digits[0] = GET_REMAINDER_SECOND_DIGIT(unsignInNum, (__uint128_t)dig1);
    r->digits[1] = dig1;
    r->digits[2] = dig2;
    r->digits[3] = dig3;
    r->digits[4] = dig4;
}
#endif

void _CFBigNumInitWithUInt8(_CFBigNum *r, uint8_t inNum) {
    memset(r, 0, sizeof(*r));
    uint8_t unsignInNum = inNum;
    r->digits[0] = unsignInNum;
}

void _CFBigNumInitWithUInt16(_CFBigNum *r, uint16_t inNum) {
    memset(r, 0, sizeof(*r));
    uint16_t unsignInNum = inNum;
    r->digits[0] = unsignInNum;
}

void _CFBigNumInitWithUInt32(_CFBigNum *r, uint32_t inNum) {
    memset(r, 0, sizeof(*r));
    uint32_t unsignInNum = inNum;
    uint32_t dig0 = GET_SECOND_DIGIT(unsignInNum);
    r->digits[0] = GET_REMAINDER_SECOND_DIGIT(unsignInNum, dig0);
    r->digits[1] = dig0;
}

void _CFBigNumInitWithUInt64(_CFBigNum *r, uint64_t inNum) {
    memset(r, 0, sizeof(*r));
    uint64_t unsignInNum = inNum;
    uint32_t dig2 = GET_THIRD_DIGIT(unsignInNum);
    unsignInNum = GET_REMAINDER_THIRD_DIGIT(unsignInNum, (uint64_t)dig2);
    uint32_t dig1 = GET_SECOND_DIGIT(unsignInNum);
    r->digits[0] = GET_REMAINDER_SECOND_DIGIT(unsignInNum, (uint64_t)dig1);
    r->digits[1] = dig1;
    r->digits[2] = dig2;
}

#ifdef __LP64__
void _CFBigNumInitWithUInt128(_CFBigNum *r, __uint128_t inNum) {
    memset(r, 0, sizeof(*r));
    __uint128_t unsignInNum = inNum;
    uint32_t dig4 = GET_FIFTH_DIGIT(unsignInNum);
    unsignInNum = GET_REMAINDER_FIFTH_DIGIT(unsignInNum, (__uint128_t)dig4);
    uint32_t dig3 = GET_FOURTH_DIGIT(unsignInNum);
    unsignInNum = GET_REMAINDER_FOURTH_DIGIT(unsignInNum, (__uint128_t)dig3);
    uint32_t dig2 = GET_THIRD_DIGIT(unsignInNum);
    unsignInNum = GET_REMAINDER_THIRD_DIGIT(unsignInNum, (__uint128_t)dig2);
    uint32_t dig1 = GET_SECOND_DIGIT(unsignInNum);
    r->digits[0] = GET_REMAINDER_SECOND_DIGIT(unsignInNum, (__uint128_t)dig1);
    r->digits[1] = dig1;
    r->digits[2] = dig2;
    r->digits[3] = dig3;
    r->digits[4] = dig4;
}
#endif


int8_t _CFBigNumGetInt8(const _CFBigNum *num) {
    int8_t result = num->digits[0];
    if (num->sign < 0) {
        result = -1 * result;
    }
    return result;
}

int16_t _CFBigNumGetInt16(const _CFBigNum *num) {
    int16_t result = num->digits[0];
    if (num->sign < 0) {
        result = -1 * result;
    }
    return result;
}

int32_t _CFBigNumGetInt32(const _CFBigNum *num) {
    int32_t result = num->digits[0];
    result += num->digits[1] * BIG_DIGITS_LIMIT;
    if (num->sign < 0) {
        result = -1 * result;
    }
    return result;
}

int64_t _CFBigNumGetInt64(const _CFBigNum *num) {
    int64_t result = num->digits[0];
    result += (int64_t)num->digits[1] * BIG_DIGITS_LIMIT;
    result += (int64_t)num->digits[2] * BIG_DIGITS_LIMIT_2;
    if (num->sign < 0) {
        result = -1 * result;
    }
    return result;
}

#if __LP64__
__int128_t _CFBigNumGetInt128(const _CFBigNum *num) {
    __int128_t result = num->digits[0];
    result += (__int128_t)num->digits[1] * BIG_DIGITS_LIMIT;
    result += (__int128_t)num->digits[2] * BIG_DIGITS_LIMIT_2;
    result += (__int128_t)num->digits[3] * BIG_DIGITS_LIMIT_3;
    result += (__int128_t)num->digits[4] * BIG_DIGITS_LIMIT_4;
    if (num->sign < 0) {
        result = -1 * result;
    }
    return result;
}
#endif

uint8_t _CFBigNumGetUInt8(const _CFBigNum *num) {
    uint8_t result = num->digits[0];
    return result;
}

uint16_t _CFBigNumGetUInt16(const _CFBigNum *num) {
    uint16_t result = num->digits[0];
    return result;
}

uint32_t _CFBigNumGetUInt32(const _CFBigNum *num) {
    uint32_t result = num->digits[0];
    result += num->digits[1] * BIG_DIGITS_LIMIT;
    return result;
}

uint64_t _CFBigNumGetUInt64(const _CFBigNum *num) {
    uint64_t result = num->digits[0];
    result += (uint64_t)num->digits[1] * BIG_DIGITS_LIMIT;
    result += (uint64_t)num->digits[2] * BIG_DIGITS_LIMIT_2;
    return result;
}

#if __LP64__
__uint128_t _CFBigNumGetUInt128(const _CFBigNum *num) {
    __uint128_t result = num->digits[0];
    result += (__uint128_t)num->digits[1] * BIG_DIGITS_LIMIT;
    result += (__uint128_t)num->digits[2] * BIG_DIGITS_LIMIT_2;
    result += (__uint128_t)num->digits[3] * BIG_DIGITS_LIMIT_3;
    result += (__uint128_t)num->digits[4] * BIG_DIGITS_LIMIT_4;
    return result;
}
#endif


void _CFBigNumInitWithCFNumber(_CFBigNum *r, CFNumberRef inNum) {
    uint8_t bytes[128 + 16];
    memset(bytes, 0, sizeof(bytes));
    // round bytes up to next multiple of 16; compiler attributes won't always guarantee big alignment
    void *bytesa = (uint8_t *)(((uintptr_t)bytes / 16) * 16 + 16);
    CFNumberType type = _CFNumberGetType2(inNum);
    CFNumberGetValue(inNum, type, bytesa);
    _CFBigNumInitWithBytes(r, bytesa, type);
}

void _CFBigNumInitWithBytes(_CFBigNum *r, const void *bytes, CFNumberType type) {
    switch (type) {
    case kCFNumberSInt8Type:
        _CFBigNumInitWithInt8(r, *(int8_t *)bytes);
        return;
    case kCFNumberSInt16Type:
        _CFBigNumInitWithInt16(r, *(int16_t *)bytes);
        return;
    case kCFNumberSInt32Type:
        _CFBigNumInitWithInt32(r, *(int32_t *)bytes);
        return;
    case kCFNumberSInt64Type:
        _CFBigNumInitWithInt64(r, *(int64_t *)bytes);
        return;
    case kCFNumberCharType:
        _CFBigNumInitWithInt8(r, *(int8_t *)bytes);
        return;
    case kCFNumberShortType:
        _CFBigNumInitWithInt16(r, *(int16_t *)bytes);
        return;
    case kCFNumberIntType:
        _CFBigNumInitWithInt32(r, *(int32_t *)bytes);
        return;
    case kCFNumberLongType:
        if (sizeof(long) == 8) {
            _CFBigNumInitWithInt64(r, *(int64_t *)bytes);
        } else {
            _CFBigNumInitWithInt32(r, *(int32_t *)bytes);
        }
        return;
    case kCFNumberLongLongType:
        _CFBigNumInitWithInt64(r, *(int64_t *)bytes);
        return;
    case kCFNumberCFIndexType:
        if (sizeof(CFIndex) == 8) {
            _CFBigNumInitWithInt64(r, *(int64_t *)bytes);
        } else {
            _CFBigNumInitWithInt32(r, *(int32_t *)bytes);
        }
        return;
    case kCFNumberNSIntegerType:
        if (sizeof(long) == 8) { // NSInteger follows long
            _CFBigNumInitWithInt64(r, *(int64_t *)bytes);
        } else {
            _CFBigNumInitWithInt32(r, *(int32_t *)bytes);
        }
        return;
#if __LP64__
    case kCFNumberSInt128Type: {
        CFSInt128Struct s;
        memmove(&s, bytes, sizeof(CFSInt128Struct)); // the hard way because bytes might not be aligned
        __int128_t val = (__int128_t)s.low + ((__int128_t)s.high << 64);
        _CFBigNumInitWithInt128(r, val);
        return;
    }
#endif
    default:
        return;
    }
}

CFNumberRef _CFNumberCreateWithBigNum(const _CFBigNum *input) {
    if (0 == input->digits[4] && 0 == input->digits[3] && 0 == input->digits[2] && 0 == input->digits[1]) {
        // This bumps up the size of the most negative n-bit value to the next larger size; oh well
        if (input->digits[0] <= INT8_MAX) {
            int8_t num = _CFBigNumGetInt8(input);
            CFNumberRef result = CFNumberCreate(kCFAllocatorSystemDefault, kCFNumberSInt8Type, (const void *)&num);
            return result;
        } else if (input->digits[0] <= INT16_MAX) {
            int16_t num = _CFBigNumGetInt16(input);
            CFNumberRef result = CFNumberCreate(kCFAllocatorSystemDefault, kCFNumberSInt16Type, (const void *)&num);
            return result;
        }
    }    
    _CFBigNum maxlimit, minlimit;
    if (0 == input->digits[4] && 0 == input->digits[3] && 0 == input->digits[2]) {
        _CFBigNumInitWithInt32(&maxlimit, INT32_MAX);
        _CFBigNumInitWithInt32(&minlimit, INT32_MIN);
        CFComparisonResult cr = _CFBigNumCompare(input, &maxlimit);
        CFComparisonResult crn = _CFBigNumCompare(input, &minlimit);
        if ((kCFCompareLessThan == cr || kCFCompareEqualTo == cr) && (kCFCompareLessThan != crn)) {
            int32_t num = _CFBigNumGetInt32(input);
            CFNumberRef result = CFNumberCreate(kCFAllocatorSystemDefault, kCFNumberSInt32Type, (const void *)&num);
            return result;
        }
    }
    if (0 == input->digits[4] && 0 == input->digits[3]) {
        _CFBigNumInitWithInt64(&maxlimit, INT64_MAX);
        _CFBigNumInitWithInt64(&minlimit, INT64_MIN);
        CFComparisonResult cr = _CFBigNumCompare(input, &maxlimit);
        CFComparisonResult crn = _CFBigNumCompare(input, &minlimit);
        if ((kCFCompareLessThan == cr || kCFCompareEqualTo == cr) && (kCFCompareLessThan != crn)) {
            int64_t num = _CFBigNumGetInt64(input);
            CFNumberRef result = CFNumberCreate(kCFAllocatorSystemDefault, kCFNumberSInt64Type, (const void *)&num);
            return result;
        }
    }
#if __LP64__
    _CFBigNumInitWithInt128(&maxlimit, INT128_MAX);
    _CFBigNumInitWithInt128(&minlimit, INT128_MIN);
    CFComparisonResult cr = _CFBigNumCompare(input, &maxlimit);
    CFComparisonResult crn = _CFBigNumCompare(input, &minlimit);
    if ((kCFCompareLessThan == cr || kCFCompareEqualTo == cr) && (kCFCompareLessThan != crn)) {
        __int128_t num = _CFBigNumGetInt128(input);
        CFNumberRef result = CFNumberCreate(kCFAllocatorSystemDefault, kCFNumberSInt128Type, (const void *)&num);
        return result;
    }
#endif
    return NULL;
}

CFComparisonResult _CFBigNumCompare(const _CFBigNum *a, const _CFBigNum *b) {
    Boolean sameSign = a->sign == b->sign;
    if (sameSign) {
        Boolean negative = a->sign < 0;
        for (CFIndex i = sizeof(a->digits) / sizeof(a->digits[0]); i--;) {
            if (a->digits[i] < b->digits[i]) {
                return !negative ? kCFCompareLessThan : kCFCompareGreaterThan;
            }
            if (a->digits[i] > b->digits[i]) {
                return negative ? kCFCompareLessThan : kCFCompareGreaterThan;
            }
        }
        return kCFCompareEqualTo;
    }
    return (a->sign < b->sign) ? kCFCompareLessThan : kCFCompareGreaterThan;
}

// abs(a) < abs(b)
// do not care about the sign
static Boolean _CFBigNumAbsoluteLessThan(const _CFBigNum *a, const _CFBigNum *b) {
    for (CFIndex i = sizeof(a->digits) / sizeof(a->digits[0]); i--;) {
        if (a->digits[i] < b->digits[i]) {
            return true;
        }
        if (a->digits[i] > b->digits[i]) {
            return false;
        }
    }
    return false;
}

// r = a * -1
void _CFBigNumNeg(_CFBigNum *r, const _CFBigNum *a) {
    memmove(r, a, sizeof(*a));
    Boolean aIsZero = true;
    for (CFIndex i = 0; i < sizeof(a->digits) / sizeof(a->digits[0]); i++) {
        if (a->digits[i] != 0) {
            aIsZero = false;
            break;
        }
    }
    // if a is zero, we do not flip the sign
    if (!aIsZero) {
        // 0  -> -1
        // -1 -> 0
        r->sign = r->sign * r->sign - 1;
    }
}

uint8_t _CFBigNumAdd(_CFBigNum *r, const _CFBigNum *a, const _CFBigNum *b) {
    uint8_t carry = 0;
    Boolean sameSign = a->sign == b->sign;
    if (sameSign) {
        for (CFIndex i = 0; i < sizeof(a->digits) / sizeof(a->digits[0]); i++) {
            uint32_t result = a->digits[i] + b->digits[i] + carry;
            if (result > BIG_DIGITS_LIMIT) {
                carry = 1;
                result -= BIG_DIGITS_LIMIT;
            } else {
                carry = 0;
            }
            r->digits[i] = result;
        }
        r->sign = a->sign;
        return carry;
    } else {
        // the algorithm here is to find the larger one and do the subtraction and then neg the result if necessary
        const _CFBigNum *bigNum = NULL;
        const _CFBigNum *smallNum = NULL;
        if (_CFBigNumAbsoluteLessThan(a, b)) {
            bigNum = b;
            smallNum = a;
        } else {
            bigNum = a;
            smallNum = b;
        }
        for (int i = 0; i < sizeof(a->digits) / sizeof(a->digits[0]); i++) {
            int64_t result = (int64_t)bigNum->digits[i] - (int64_t)smallNum->digits[i] - carry;
            if (result < 0) {
                carry = 1;
                result += BIG_DIGITS_LIMIT;
            } else {
                carry = 0;
            }
            r->digits[i] = result;
        }
        if (bigNum->sign < 0) {
            r->sign = -1;
        } else {
            r->sign = 0;
        }
        return carry;
    }
}

uint8_t _CFBigNumSub(_CFBigNum *r, const _CFBigNum *a, const _CFBigNum *b) {
    _CFBigNum nb;
    _CFBigNumNeg(&nb, b);
    return _CFBigNumAdd(r, a, &nb);
}


void _CFBigNumToCString(const _CFBigNum *vp, Boolean leading_zeros, Boolean leading_plus, char *buffer, size_t buflen) {
    if (vp->sign < 0) {
        *buffer++ = '-';
        buflen--;
    } else if (leading_plus) {
        *buffer++ = '+';
        buflen--;
    }
    char tmp[46];
    snprintf(tmp, sizeof(tmp), "%09u%09u%09u%09u%09u", vp->digits[4], vp->digits[3], vp->digits[2], vp->digits[1], vp->digits[0]);
    if (leading_zeros) {
        memset(buffer, '0', buflen);
        uint32_t tocopy = __CFMin(sizeof(tmp), buflen);
        memmove(buffer + buflen - tocopy, tmp + sizeof(tmp) - tocopy, tocopy); // copies trailing nul from tmp to nul-terminate
    } else {
        char *s = tmp;
        while (*s == '0') s++;
        if (*s == 0) s--; // if tmp is all zeros, copy out at least one zero
        strlcpy(buffer, s, buflen);
    }
}

void _CFBigNumFromCString(_CFBigNum *r, const char *string) {
    memset(r, 0, sizeof(*r));
    char *copy = (char *)calloc(strlen(string)+1, sizeof(char));
    memcpy(copy, string, strlen(string)+1);
    char *working = copy;
    if (*working == '-') {
        r->sign = -1;
        working++;
    } else if (*working == '+') {
        working++;
    }
    while (*working == '0') {
        working++;
    }

    size_t length = strlen(working);
    if (length == 0) { // the number is zero
        free(copy);
        return;
    }
    int curDigit = 0;
    while (curDigit + 1 < sizeof(r->digits) / sizeof(r->digits[0]) && 9 < length) {
        uint32_t digit = atol(working+length-9);
        r->digits[curDigit] = digit;
        *(working+length-9) = 0;
        length -= 9;
        curDigit++;
    }
    uint32_t digit = atol(working);
    r->digits[curDigit] = digit;
    free(copy);
}

char *_CFBigNumCopyDescription(const _CFBigNum *num) {
    char *result = (char *)calloc(1024, sizeof(char));
    sprintf(result, "sign:%s 1st:%u 2nd:%u 3rd:%u 4th:%u 5th:%u", num->sign < 0 ? "-" : "+", num->digits[0], num->digits[1], num->digits[2], num->digits[3], num->digits[4]);
    return result;
}

