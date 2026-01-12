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

/*	CFNumber.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: Ali Ozer
*/

#include <CoreFoundation/CFNumber.h>
#include "CFInternal.h"
#include <CoreFoundation/CFPriv.h>
#include <math.h>
#include <float.h>


#if DEPLOYMENT_TARGET_WINDOWS
#define isnan(A) _isnan(A)
#define isinf(A) !_finite(A)
#define copysign(A, B) _copysign(A, B)
#endif

#define __CFAssertIsBoolean(cf) __CFGenericValidateType(cf, CFBooleanGetTypeID())

struct __CFBoolean {
    CFRuntimeBase _base;
};

static struct __CFBoolean __kCFBooleanTrue = {
    INIT_CFRUNTIME_BASE()
};
const CFBooleanRef kCFBooleanTrue = &__kCFBooleanTrue;

static struct __CFBoolean __kCFBooleanFalse = {
    INIT_CFRUNTIME_BASE()
};
const CFBooleanRef kCFBooleanFalse = &__kCFBooleanFalse;

static CFStringRef __CFBooleanCopyDescription(CFTypeRef cf) {
    CFBooleanRef boolean = (CFBooleanRef)cf;
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("<CFBoolean %p [%p]>{value = %s}"), cf, CFGetAllocator(cf), (boolean == kCFBooleanTrue) ? "true" : "false");
}

CF_PRIVATE CFStringRef __CFBooleanCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions) {
    CFBooleanRef boolean = (CFBooleanRef)cf;
    return (CFStringRef)CFRetain((boolean == kCFBooleanTrue) ? CFSTR("true") : CFSTR("false"));
}

static CFHashCode __CFBooleanHash(CFTypeRef cf) {
    CFBooleanRef boolean = (CFBooleanRef)cf;
    return (boolean == kCFBooleanTrue) ? _CFHashInt(1) : _CFHashInt(0);
}

static void __CFBooleanDeallocate(CFTypeRef cf) {
    CFAssert(false, __kCFLogAssertion, "Deallocated CFBoolean!");
}

static CFTypeID __kCFBooleanTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __CFBooleanClass = {
    0,
    "CFBoolean",
    NULL,      // init
    NULL,      // copy
    __CFBooleanDeallocate,
    NULL,
    __CFBooleanHash,
    __CFBooleanCopyFormattingDescription,
    __CFBooleanCopyDescription
};

CFTypeID CFBooleanGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{
        __kCFBooleanTypeID = _CFRuntimeRegisterClass(&__CFBooleanClass); // initOnce covered
        _CFRuntimeSetInstanceTypeIDAndIsa(&__kCFBooleanTrue, __kCFBooleanTypeID);
        _CFRuntimeSetInstanceTypeIDAndIsa(&__kCFBooleanFalse, __kCFBooleanTypeID);
    });
    return __kCFBooleanTypeID;
}

Boolean CFBooleanGetValue(CFBooleanRef boolean) {
    CF_OBJC_FUNCDISPATCHV(CFBooleanGetTypeID(), Boolean, (NSNumber *)boolean, boolValue);
    return (boolean == kCFBooleanTrue) ? true : false;
}


/*** CFNumber ***/

#define OLD_CRAP_TOO 0

#if OLD_CRAP_TOO

// old implementation, for runtime comparison purposes

typedef union {
    SInt32 valSInt32;
    int64_t valSInt64;
    Float32 valFloat32;
    Float64 valFloat64;
} __CFNumberValue_old;

struct __CFNumber_old {             /* Only as many bytes as necessary are allocated */
    CFRuntimeBase _base;
    __CFNumberValue_old value;
};

static Boolean __CFNumberEqual_old(CFTypeRef cf1, CFTypeRef cf2);
static CFHashCode __CFNumberHash_old(CFTypeRef cf);
static CFStringRef __CFNumberCopyDescription_old(CFTypeRef cf);
CF_PRIVATE CFStringRef __CFNumberCopyFormattingDescriptionAsFloat64_old(CFTypeRef cf);
static CFStringRef __CFNumberCopyFormattingDescription_old(CFTypeRef cf, CFDictionaryRef formatOptions);
static struct __CFNumber_old * CFNumberCreate_old(CFAllocatorRef allocator, CFNumberType type, const void *valuePtr);
static CFNumberType CFNumberGetType_old(struct __CFNumber_old * number);
static CFIndex CFNumberGetByteSize_old(struct __CFNumber_old * number);
static Boolean CFNumberIsFloatType_old(struct __CFNumber_old * number);
static Boolean CFNumberGetValue_old(struct __CFNumber_old * number, CFNumberType type, void *valuePtr);
static CFComparisonResult CFNumberCompare_old(struct __CFNumber_old * number1, struct __CFNumber_old * number2, void *context);

#endif


#define __CFAssertIsNumber(cf) __CFGenericValidateType(cf, CFNumberGetTypeID())
#define __CFAssertIsValidNumberType(type) CFAssert2((0 < type && type <= kCFNumberMaxType) || (type == kCFNumberSInt128Type), __kCFLogAssertion, "%s(): bad CFNumber type %d", __PRETTY_FUNCTION__, type);

/* The IEEE bit patterns... Also have:
0x7f800000		float +Inf
0x7fc00000		float NaN
0xff800000		float -Inf
*/
#define BITSFORDOUBLENAN	((uint64_t)0x7ff8000000000000ULL)
#define BITSFORDOUBLEPOSINF	((uint64_t)0x7ff0000000000000ULL)
#define BITSFORDOUBLENEGINF	((uint64_t)0xfff0000000000000ULL)

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#define FLOAT_POSITIVE_2_TO_THE_64	0x1.0p+64L
#define FLOAT_NEGATIVE_2_TO_THE_127	-0x1.0p+127L
#define FLOAT_POSITIVE_2_TO_THE_127	0x1.0p+127L
#elif DEPLOYMENT_TARGET_WINDOWS
#define FLOAT_POSITIVE_2_TO_THE_64	18446744073709551616.0
#define FLOAT_NEGATIVE_2_TO_THE_127	-170141183460469231731687303715884105728.0
#define FLOAT_POSITIVE_2_TO_THE_127	170141183460469231731687303715884105728.0
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif

typedef struct {	// NOTE WELL: these two fields may switch position someday, do not use '= {high, low}' -style initialization
    int64_t high;
    uint64_t low;
} CFSInt128Struct;

enum {
    kCFNumberSInt128Type = 17
};

static uint8_t isNeg128(const CFSInt128Struct *in) {
    return in->high < 0;
}

static CFComparisonResult cmp128(const CFSInt128Struct *in1, const CFSInt128Struct *in2) {
    if (in1->high < in2->high) return kCFCompareLessThan;
    if (in1->high > in2->high) return kCFCompareGreaterThan;
    if (in1->low < in2->low) return kCFCompareLessThan;
    if (in1->low > in2->low) return kCFCompareGreaterThan;
    return kCFCompareEqualTo;
}

// allows out to be the same as in1 or in2
static void add128(CFSInt128Struct *out, CFSInt128Struct *in1, CFSInt128Struct *in2) {
    CFSInt128Struct tmp;
    tmp.low = in1->low + in2->low;
    tmp.high = in1->high + in2->high;
    if (UINT64_MAX - in1->low < in2->low) {
        tmp.high++;
    }
    *out = tmp;
}

// allows out to be the same as in
static void neg128(CFSInt128Struct *out, CFSInt128Struct *in) {
    uint64_t tmplow = ~in->low;
    out->low = tmplow + 1;
    out->high = ~in->high;
    if (UINT64_MAX == tmplow) {
	out->high++;
    }
}

static const CFSInt128Struct powersOf10[] = {
    { 0x4B3B4CA85A86C47ALL, 0x098A224000000000ULL },
    { 0x0785EE10D5DA46D9LL, 0x00F436A000000000ULL },
    { 0x00C097CE7BC90715LL, 0xB34B9F1000000000ULL },
    { 0x0013426172C74D82LL, 0x2B878FE800000000ULL },
    { 0x0001ED09BEAD87C0LL, 0x378D8E6400000000ULL },
    { 0x0000314DC6448D93LL, 0x38C15B0A00000000ULL },
    { 0x000004EE2D6D415BLL, 0x85ACEF8100000000ULL },
    { 0x0000007E37BE2022LL, 0xC0914B2680000000ULL },
    { 0x0000000C9F2C9CD0LL, 0x4674EDEA40000000ULL },
    { 0x00000001431E0FAELL, 0x6D7217CAA0000000ULL },
    { 0x00000000204FCE5ELL, 0x3E25026110000000ULL },
    { 0x00000000033B2E3CLL, 0x9FD0803CE8000000ULL },
    { 0x000000000052B7D2LL, 0xDCC80CD2E4000000ULL },
    { 0x0000000000084595LL, 0x161401484A000000ULL },
    { 0x000000000000D3C2LL, 0x1BCECCEDA1000000ULL },
    { 0x000000000000152DLL, 0x02C7E14AF6800000ULL },
    { 0x000000000000021ELL, 0x19E0C9BAB2400000ULL },
    { 0x0000000000000036LL, 0x35C9ADC5DEA00000ULL },
    { 0x0000000000000005LL, 0x6BC75E2D63100000ULL },
    { 0x0000000000000000LL, 0x8AC7230489E80000ULL },
    { 0x0000000000000000LL, 0x0DE0B6B3A7640000ULL },
    { 0x0000000000000000LL, 0x016345785D8A0000ULL },
    { 0x0000000000000000LL, 0x002386F26FC10000ULL },
    { 0x0000000000000000LL, 0x00038D7EA4C68000ULL },
    { 0x0000000000000000LL, 0x00005AF3107A4000ULL },
    { 0x0000000000000000LL, 0x000009184E72A000ULL },
    { 0x0000000000000000LL, 0x000000E8D4A51000ULL },
    { 0x0000000000000000LL, 0x000000174876E800ULL },
    { 0x0000000000000000LL, 0x00000002540BE400ULL },
    { 0x0000000000000000LL, 0x000000003B9ACA00ULL },
    { 0x0000000000000000LL, 0x0000000005F5E100ULL },
    { 0x0000000000000000LL, 0x0000000000989680ULL },
    { 0x0000000000000000LL, 0x00000000000F4240ULL },
    { 0x0000000000000000LL, 0x00000000000186A0ULL },
    { 0x0000000000000000LL, 0x0000000000002710ULL },
    { 0x0000000000000000LL, 0x00000000000003E8ULL },
    { 0x0000000000000000LL, 0x0000000000000064ULL },
    { 0x0000000000000000LL, 0x000000000000000AULL },
    { 0x0000000000000000LL, 0x0000000000000001ULL },
};

static const CFSInt128Struct neg_powersOf10[] = {
    { 0xB4C4B357A5793B85LL, 0xF675DDC000000000ULL },
    { 0xF87A11EF2A25B926LL, 0xFF0BC96000000000ULL },
    { 0xFF3F68318436F8EALL, 0x4CB460F000000000ULL },
    { 0xFFECBD9E8D38B27DLL, 0xD478701800000000ULL },
    { 0xFFFE12F64152783FLL, 0xC872719C00000000ULL },
    { 0xFFFFCEB239BB726CLL, 0xC73EA4F600000000ULL },
    { 0xFFFFFB11D292BEA4LL, 0x7A53107F00000000ULL },
    { 0xFFFFFF81C841DFDDLL, 0x3F6EB4D980000000ULL },
    { 0xFFFFFFF360D3632FLL, 0xB98B1215C0000000ULL },
    { 0xFFFFFFFEBCE1F051LL, 0x928DE83560000000ULL },
    { 0xFFFFFFFFDFB031A1LL, 0xC1DAFD9EF0000000ULL },
    { 0xFFFFFFFFFCC4D1C3LL, 0x602F7FC318000000ULL },
    { 0xFFFFFFFFFFAD482DLL, 0x2337F32D1C000000ULL },
    { 0xFFFFFFFFFFF7BA6ALL, 0xE9EBFEB7B6000000ULL },
    { 0xFFFFFFFFFFFF2C3DLL, 0xE43133125F000000ULL },
    { 0xFFFFFFFFFFFFEAD2LL, 0xFD381EB509800000ULL },
    { 0xFFFFFFFFFFFFFDE1LL, 0xE61F36454DC00000ULL },
    { 0xFFFFFFFFFFFFFFC9LL, 0xCA36523A21600000ULL },
    { 0xFFFFFFFFFFFFFFFALL, 0x9438A1D29CF00000ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0x7538DCFB76180000ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xF21F494C589C0000ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFE9CBA87A2760000ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFDC790D903F0000ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFC72815B398000ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFA50CEF85C000ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFF6E7B18D6000ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFF172B5AF000ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFE8B7891800ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFDABF41C00ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFFC4653600ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFFFA0A1F00ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFFFF676980ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFFFFF0BDC0ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFFFFFE7960ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFFFFFFD8F0ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFFFFFFFC18ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFFFFFFFF9CULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFFFFFFFFF6ULL },
    { 0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFFFFFFFFFFULL },
};

static void emit128(char *buffer, const CFSInt128Struct *in, Boolean forcePlus) {
    CFSInt128Struct tmp = *in;
    if (isNeg128(&tmp)) {
	neg128(&tmp, &tmp);
	*buffer++ = '-';
    } else if (forcePlus) {
	*buffer++ = '+';
    }
    Boolean doneOne = false;
    int idx;
    for (idx = 0; idx < sizeof(powersOf10) / sizeof(powersOf10[0]); idx++) {
	int count = 0;
        while (cmp128(&powersOf10[idx], &tmp) <= 0) {
	    add128(&tmp, &tmp, (CFSInt128Struct *)&neg_powersOf10[idx]);
	    count++;
	}
	if (0 != count || doneOne) {
	    *buffer++ = '0' + count;
	    doneOne = true;
	}
    }
    if (!doneOne) {
	*buffer++ = '0';
    }
    *buffer = '\0';
}

static void cvtSInt128ToFloat64(Float64 *out, const CFSInt128Struct *in) {
    // switching to a positive number results in better accuracy
    // for negative numbers close to zero, because the multiply
    // of -1 by 2^64 (scaling the Float64 high) is avoided
    Boolean wasNeg = false;
    CFSInt128Struct tmp = *in;
    if (isNeg128(&tmp)) {
	neg128(&tmp, &tmp);
	wasNeg = true;
    }
    Float64 d = (Float64)tmp.high * FLOAT_POSITIVE_2_TO_THE_64 + (Float64)tmp.low;
    if (wasNeg) d = -d;
    *out = d;
}

static void cvtFloat64ToSInt128(CFSInt128Struct *out, const Float64 *in) {
    CFSInt128Struct i;
    Float64 d = *in;
    if (d < FLOAT_NEGATIVE_2_TO_THE_127) {
	i.high = 0x8000000000000000LL;
	i.low = 0x0000000000000000ULL;
	*out = i;
	return;
    }
    if (FLOAT_POSITIVE_2_TO_THE_127<= d) {
	i.high = 0x7fffffffffffffffLL;
	i.low = 0xffffffffffffffffULL;
	*out = i;
	return;
    }
    Float64 t = floor(d / FLOAT_POSITIVE_2_TO_THE_64);
    i.high = (int64_t)t;
    i.low = (uint64_t)(d - t * FLOAT_POSITIVE_2_TO_THE_64);
    *out = i;
}

struct __CFNumber {
    CFRuntimeBase _base;
#if OLD_CRAP_TOO
    struct __CFNumber_old *__old__;
    void * __dummy__;
#endif
    uint64_t _pad; // need this space here for the constant objects
    /* 0 or 8 more bytes allocated here */
};

/* Seven bits in base:
    Bits 6..5: unused
    Bits 4..0: CFNumber type
*/

static struct __CFNumber __kCFNumberNaN = {
    INIT_CFRUNTIME_BASE(), 0ULL
};
const CFNumberRef kCFNumberNaN = &__kCFNumberNaN;

static struct __CFNumber __kCFNumberNegativeInfinity = {
    INIT_CFRUNTIME_BASE(), 0ULL
};
const CFNumberRef kCFNumberNegativeInfinity = &__kCFNumberNegativeInfinity;

static struct __CFNumber __kCFNumberPositiveInfinity = {
    INIT_CFRUNTIME_BASE(), 0ULL
};
const CFNumberRef kCFNumberPositiveInfinity = &__kCFNumberPositiveInfinity;

static const struct {
    uint16_t canonicalType:5;	// canonical fixed-width type
    uint16_t floatBit:1;	// is float
    uint16_t storageBit:1;	// storage size (0: (float ? 4 : 8), 1: (float ? 8 : 16) bits)
    uint16_t lgByteSize:3;	// base-2 log byte size of public type
    uint16_t unused:6;
} __CFNumberTypeTable[] = {
    /* 0 */			{0, 0, 0, 0},

    /* kCFNumberSInt8Type */	{kCFNumberSInt8Type, 0, 0, 0, 0},
    /* kCFNumberSInt16Type */	{kCFNumberSInt16Type, 0, 0, 1, 0},
    /* kCFNumberSInt32Type */	{kCFNumberSInt32Type, 0, 0, 2, 0},
    /* kCFNumberSInt64Type */	{kCFNumberSInt64Type, 0, 0, 3, 0},
    /* kCFNumberFloat32Type */	{kCFNumberFloat32Type, 1, 0, 2, 0},
    /* kCFNumberFloat64Type */	{kCFNumberFloat64Type, 1, 1, 3, 0},

    /* kCFNumberCharType */	{kCFNumberSInt8Type, 0, 0, 0, 0},
    /* kCFNumberShortType */	{kCFNumberSInt16Type, 0, 0, 1, 0},
    /* kCFNumberIntType */	{kCFNumberSInt32Type, 0, 0, 2, 0},
#if __LP64__
    /* kCFNumberLongType */	{kCFNumberSInt64Type, 0, 0, 3, 0},
#else
    /* kCFNumberLongType */	{kCFNumberSInt32Type, 0, 0, 2, 0},
#endif
    /* kCFNumberLongLongType */	{kCFNumberSInt64Type, 0, 0, 3, 0},
    /* kCFNumberFloatType */	{kCFNumberFloat32Type, 1, 0, 2, 0},
    /* kCFNumberDoubleType */	{kCFNumberFloat64Type, 1, 1, 3, 0},

#if __LP64__
    /* kCFNumberCFIndexType */	{kCFNumberSInt64Type, 0, 0, 3, 0},
    /* kCFNumberNSIntegerType */ {kCFNumberSInt64Type, 0, 0, 3, 0},
    /* kCFNumberCGFloatType */	{kCFNumberFloat64Type, 1, 1, 3, 0},
#else
    /* kCFNumberCFIndexType */	{kCFNumberSInt32Type, 0, 0, 2, 0},
    /* kCFNumberNSIntegerType */ {kCFNumberSInt32Type, 0, 0, 2, 0},
    /* kCFNumberCGFloatType */	{kCFNumberFloat32Type, 1, 0, 2, 0},
#endif

    /* kCFNumberSInt128Type */	{kCFNumberSInt128Type, 0, 1, 4, 0},
};

CF_INLINE CFNumberType __CFNumberGetType(CFNumberRef num) {
    return __CFBitfieldGetValue(num->_base._cfinfo[CF_INFO_BITS], 4, 0);
}

#define CVT(SRC_TYPE, DST_TYPE, DST_MIN, DST_MAX) do { \
	SRC_TYPE sv; memmove(&sv, data, sizeof(SRC_TYPE)); \
	DST_TYPE dv = (sv < DST_MIN) ? (DST_TYPE)DST_MIN : (DST_TYPE)(((DST_MAX < sv) ? DST_MAX : sv)); \
	memmove(valuePtr, &dv, sizeof(DST_TYPE)); \
	SRC_TYPE vv = (SRC_TYPE)dv; return (vv == sv); \
	} while (0)

#define CVT128ToInt(SRC_TYPE, DST_TYPE, DST_MIN, DST_MAX) do { \
        SRC_TYPE sv; memmove(&sv, data, sizeof(SRC_TYPE)); \
	DST_TYPE dv; Boolean noLoss = false; \
	if (0 < sv.high || (0 == sv.high && (int64_t)DST_MAX < sv.low)) { \
	    dv = DST_MAX; \
	} else if (sv.high < -1 || (-1 == sv.high && sv.low < (int64_t)DST_MIN)) { \
	    dv = DST_MIN; \
	} else { \
	    dv = (DST_TYPE)sv.low; \
	    noLoss = true; \
	} \
        memmove(valuePtr, &dv, sizeof(DST_TYPE)); \
        return noLoss; \
        } while (0)

// returns false if the output value is not the same as the number's value, which
// can occur due to accuracy loss and the value not being within the target range
static Boolean __CFNumberGetValue(CFNumberRef number, CFNumberType type, void *valuePtr) {
    type = __CFNumberTypeTable[type].canonicalType;
    CFNumberType ntype = __CFNumberGetType(number);
    const void *data = &(number->_pad);
    switch (type) {
    case kCFNumberSInt8Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		CVT(Float32, int8_t, INT8_MIN, INT8_MAX);
	    } else {
		CVT(Float64, int8_t, INT8_MIN, INT8_MAX);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		CVT(int64_t, int8_t, INT8_MIN, INT8_MAX);
	    } else {
		CVT128ToInt(CFSInt128Struct, int8_t, INT8_MIN, INT8_MAX);
	    }
	}
	return true;
    case kCFNumberSInt16Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
                CVT(Float32, int16_t, INT16_MIN, INT16_MAX);
	    } else {
                CVT(Float64, int16_t, INT16_MIN, INT16_MAX);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
                CVT(int64_t, int16_t, INT16_MIN, INT16_MAX);
	    } else {
		CVT128ToInt(CFSInt128Struct, int16_t, INT16_MIN, INT16_MAX);
	    }
	}
	return true;
    case kCFNumberSInt32Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
                CVT(Float32, int32_t, INT32_MIN, INT32_MAX);
	    } else {
                CVT(Float64, int32_t, INT32_MIN, INT32_MAX);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
                CVT(int64_t, int32_t, INT32_MIN, INT32_MAX);
	    } else {
		CVT128ToInt(CFSInt128Struct, int32_t, INT32_MIN, INT32_MAX);
	    }
	}
	return true;
    case kCFNumberSInt64Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
                CVT(Float32, int64_t, INT64_MIN, INT64_MAX);
	    } else {
                CVT(Float64, int64_t, INT64_MIN, INT64_MAX);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		memmove(valuePtr, data, 8);
	    } else {
		CVT128ToInt(CFSInt128Struct, int64_t, INT64_MIN, INT64_MAX);
	    }
	}
	return true;
    case kCFNumberSInt128Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		Float32 f;
		memmove(&f, data, 4);
		Float64 d = f;
		CFSInt128Struct i;
		cvtFloat64ToSInt128(&i, &d);
		memmove(valuePtr, &i, 16);
		Float64 d2;
		cvtSInt128ToFloat64(&d2, &i);
		Float32 f2 = (Float32)d2;
		return (f2 == f);
	    } else {
		Float64 d;
		memmove(&d, data, 8);
		CFSInt128Struct i;
		cvtFloat64ToSInt128(&i, &d);
		memmove(valuePtr, &i, 16);
		Float64 d2;
		cvtSInt128ToFloat64(&d2, &i);
		return (d2 == d);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		int64_t j;
		memmove(&j, data, 8);
		CFSInt128Struct i;
		i.low = j;
		i.high = (j < 0) ? -1LL : 0LL;
		memmove(valuePtr, &i, 16);
	    } else {
		memmove(valuePtr, data, 16);
	    }
	}
	return true;
    case kCFNumberFloat32Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		memmove(valuePtr, data, 4);
	    } else {
		double d;
		memmove(&d, data, 8);
		if (isnan(d)) {
		    uint32_t l = 0x7fc00000;
		    memmove(valuePtr, &l, 4);
		    return true;
		} else if (isinf(d)) {
		    uint32_t l = 0x7f800000;
		    if (d < 0.0) l += 0x80000000UL;
		    memmove(valuePtr, &l, 4);
		    return true;
		}
		CVT(Float64, Float32, -FLT_MAX, FLT_MAX);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		CVT(int64_t, Float32, -FLT_MAX, FLT_MAX);
	    } else {
		CFSInt128Struct i;
		memmove(&i, data, 16);
		Float64 d;
		cvtSInt128ToFloat64(&d, &i);
		Float32 f = (Float32)d;
		memmove(valuePtr, &f, 4);
		d = f;
		CFSInt128Struct i2;
		cvtFloat64ToSInt128(&i2, &d);
		return cmp128(&i2, &i) == kCFCompareEqualTo;
	    }
	}
	return true;
    case kCFNumberFloat64Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		float f;
		memmove(&f, data, 4);
		if (isnan(f)) {
		    uint64_t l = BITSFORDOUBLENAN;
		    memmove(valuePtr, &l, 8);
		    return true;
		} else if (isinf(f)) {
		    uint64_t l = BITSFORDOUBLEPOSINF;
		    if (f < 0.0) l += 0x8000000000000000ULL;
		    memmove(valuePtr, &l, 8);
		    return true;
		}
		CVT(Float32, Float64, -DBL_MAX, DBL_MAX);
	    } else {
		memmove(valuePtr, data, 8);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		CVT(int64_t, Float64, -DBL_MAX, DBL_MAX);
	    } else {
                CFSInt128Struct i;
                memmove(&i, data, 16);
                Float64 d;
                cvtSInt128ToFloat64(&d, &i);
                memmove(valuePtr, &d, 8);
                CFSInt128Struct i2;
                cvtFloat64ToSInt128(&i2, &d);
                return cmp128(&i2, &i) == kCFCompareEqualTo;
	    }
	}
	return true;
    }
    return false;
}

#define CVT_COMPAT(SRC_TYPE, DST_TYPE, FT) do { \
	SRC_TYPE sv; memmove(&sv, data, sizeof(SRC_TYPE)); \
	DST_TYPE dv = (DST_TYPE)(sv); \
	memmove(valuePtr, &dv, sizeof(DST_TYPE)); \
	SRC_TYPE vv = (SRC_TYPE)dv; return (FT) || (vv == sv); \
	} while (0)

#define CVT128ToInt_COMPAT(SRC_TYPE, DST_TYPE) do { \
        SRC_TYPE sv; memmove(&sv, data, sizeof(SRC_TYPE)); \
	DST_TYPE dv; dv = (DST_TYPE)sv.low; \
        memmove(valuePtr, &dv, sizeof(DST_TYPE)); \
	uint64_t vv = (uint64_t)dv; return (vv == sv.low); \
        } while (0)

// this has the old cast-style behavior
static Boolean __CFNumberGetValueCompat(CFNumberRef number, CFNumberType type, void *valuePtr) {
    type = __CFNumberTypeTable[type].canonicalType;
    CFNumberType ntype = __CFNumberGetType(number);
    const void *data = &(number->_pad);
    switch (type) {
    case kCFNumberSInt8Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		CVT_COMPAT(Float32, int8_t, 0);
	    } else {
		CVT_COMPAT(Float64, int8_t, 0);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		// Leopard's implemenation of this always returned true. We should only return true when the conversion is lossless. However, there are some clients who use CFNumber with small unsigned values disguised as signed values. Since there is no CFNumber API yet for unsigned values, we need to accomodate those clients for now. <rdar://problem/6471866>
		// This accomodation should be removed if CFNumber ever provides API for unsigned values. <rdar://problem/6473890>
		int64_t sv; memmove(&sv, data, sizeof(int64_t));
		int8_t dv = (int8_t)(sv);
		memmove(valuePtr, &dv, sizeof(int8_t));
		int64_t vv = (int64_t)dv; return !_CFExecutableLinkedOnOrAfter(CFSystemVersionSnowLeopard) || ((sv >> 8LL) == 0LL) || (vv == sv);
	    } else {
		CVT128ToInt_COMPAT(CFSInt128Struct, int8_t);
	    }
	}
	return true;
    case kCFNumberSInt16Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		CVT_COMPAT(Float32, int16_t, 0);
	    } else {
		CVT_COMPAT(Float64, int16_t, 0);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		// Leopard's implemenation of this always returned true. We should only return true when the conversion is lossless. However, there are some clients who use CFNumber with small unsigned values disguised as signed values. Since there is no CFNumber API yet for unsigned values, we need to accomodate those clients for now. <rdar://problem/6471866>
		// This accomodation should be removed if CFNumber ever provides API for unsigned values. <rdar://problem/6473890>
		int64_t sv; memmove(&sv, data, sizeof(int64_t));
		int16_t dv = (int16_t)(sv);
		memmove(valuePtr, &dv, sizeof(int16_t));
		int64_t vv = (int64_t)dv; return !_CFExecutableLinkedOnOrAfter(CFSystemVersionSnowLeopard) || ((sv >> 16LL) == 0LL) || (vv == sv);
	    } else {
		CVT128ToInt_COMPAT(CFSInt128Struct, int16_t);
	    }
	}
	return true;
    case kCFNumberSInt32Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		CVT_COMPAT(Float32, int32_t, 0);
	    } else {
		CVT_COMPAT(Float64, int32_t, 0);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
                CVT_COMPAT(int64_t, int32_t, 0);
	    } else {
		CVT128ToInt_COMPAT(CFSInt128Struct, int32_t);
	    }
	}
	return true;
    case kCFNumberSInt64Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		CVT_COMPAT(Float32, int64_t, 0);
	    } else {
		CVT_COMPAT(Float64, int64_t, 0);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
                CVT_COMPAT(int64_t, int64_t, 0);
	    } else {
		CVT128ToInt_COMPAT(CFSInt128Struct, int64_t);
	    }
	}
	return true;
    case kCFNumberSInt128Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		Float32 f;
		memmove(&f, data, 4);
		Float64 d = f;
		CFSInt128Struct i;
		cvtFloat64ToSInt128(&i, &d);
		memmove(valuePtr, &i, 16);
		Float64 d2;
		cvtSInt128ToFloat64(&d2, &i);
		Float32 f2 = (Float32)d2;
		return (f2 == f);
	    } else {
		Float64 d;
		memmove(&d, data, 8);
		CFSInt128Struct i;
		cvtFloat64ToSInt128(&i, &d);
		memmove(valuePtr, &i, 16);
		Float64 d2;
		cvtSInt128ToFloat64(&d2, &i);
		return (d2 == d);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		int64_t j;
		memmove(&j, data, 8);
		CFSInt128Struct i;
		i.low = j;
		i.high = (j < 0) ? -1LL : 0LL;
		memmove(valuePtr, &i, 16);
	    } else {
		memmove(valuePtr, data, 16);
	    }
	}
	return true;
    case kCFNumberFloat32Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		memmove(valuePtr, data, 4);
	    } else {
		CVT_COMPAT(Float64, Float32, 0);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		CVT_COMPAT(int64_t, Float32, 0);
	    } else {
		CFSInt128Struct i;
		memmove(&i, data, 16);
		Float64 d;
		cvtSInt128ToFloat64(&d, &i);
		Float32 f = (Float32)d;
		memmove(valuePtr, &f, 4);
		d = f;
		CFSInt128Struct i2;
		cvtFloat64ToSInt128(&i2, &d);
		return cmp128(&i2, &i) == kCFCompareEqualTo;
	    }
	}
	return true;
    case kCFNumberFloat64Type:
	if (__CFNumberTypeTable[ntype].floatBit) {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		CVT_COMPAT(Float32, Float64, 0);
	    } else {
		memmove(valuePtr, data, 8);
	    }
	} else {
	    if (0 == __CFNumberTypeTable[ntype].storageBit) {
		CVT_COMPAT(int64_t, Float64, 0);
	    } else {
		CFSInt128Struct i;
		memmove(&i, data, 16);
		Float64 d;
		cvtSInt128ToFloat64(&d, &i);
		memmove(valuePtr, &d, 8);
		CFSInt128Struct i2;
		cvtFloat64ToSInt128(&i2, &d);
		return cmp128(&i2, &i) == kCFCompareEqualTo;
	    }
	}
	return true;
    }
    return false;
}

#if OLD_CRAP_TOO
static void FAIL(void) {}
#endif

static CFStringRef __CFNumberCopyDescription(CFTypeRef cf) {
    CFNumberRef number = (CFNumberRef)cf;
    CFNumberType type = __CFNumberGetType(number);
    CFMutableStringRef mstr = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    CFStringAppendFormat(mstr, NULL, CFSTR("<CFNumber %p [%p]>{value = "), cf, CFGetAllocator(cf));
    if (__CFNumberTypeTable[type].floatBit) {
	Float64 d;
        __CFNumberGetValue(number, kCFNumberFloat64Type, &d);
	if (isnan(d)) {
	    CFStringAppend(mstr, CFSTR("nan"));
	} else if (isinf(d)) {
	    CFStringAppend(mstr, (0.0 < d) ? CFSTR("+infinity") : CFSTR("-infinity"));
	} else if (0.0 == d) {
	    CFStringAppend(mstr, (copysign(1.0, d) < 0.0) ? CFSTR("-0.0") : CFSTR("+0.0"));
	} else {
	    CFStringAppendFormat(mstr, NULL, CFSTR("%+.*f"), (__CFNumberTypeTable[type].storageBit ? 20 : 10), d);
	}
	const char *typeName = "unknown float";
	switch (type) {
	case kCFNumberFloat32Type: typeName = "kCFNumberFloat32Type"; break;
	case kCFNumberFloat64Type: typeName = "kCFNumberFloat64Type"; break;
	}
	CFStringAppendFormat(mstr, NULL, CFSTR(", type = %s}"), typeName);
    } else {
	CFSInt128Struct i;
	__CFNumberGetValue(number, kCFNumberSInt128Type, &i);
	char buffer[128];
	emit128(buffer, &i, true);
	const char *typeName = "unknown integer";
	switch (type) {
	case kCFNumberSInt8Type: typeName = "kCFNumberSInt8Type"; break;
	case kCFNumberSInt16Type: typeName = "kCFNumberSInt16Type"; break;
	case kCFNumberSInt32Type: typeName = "kCFNumberSInt32Type"; break;
	case kCFNumberSInt64Type: typeName = "kCFNumberSInt64Type"; break;
	case kCFNumberSInt128Type: typeName = "kCFNumberSInt128Type"; break;
	}
	CFStringAppendFormat(mstr, NULL, CFSTR("%s, type = %s}"), buffer, typeName);
    }
#if OLD_CRAP_TOO
if (! number->__old__) {

printf("*** Test skipped in __CFNumberCopyDescription for number %p\n", cf);
} else {
CFStringRef test = __CFNumberCopyDescription_old(number->__old__);
if (!CFEqual(test, mstr)) {
CFLog(kCFLogLevelWarning, CFSTR("*** TEST FAIL in __CFNumberCopyDescription: '%@' '%@'"), test, mstr);  FAIL();
}
}
#endif
    return mstr;
}

// This function separated out from __CFNumberCopyFormattingDescription() so the plist creation can use it as well.

static CFStringRef __CFNumberCreateFormattingDescriptionAsFloat64(CFAllocatorRef allocator, CFTypeRef cf) {
    Float64 d;
    CFNumberGetValue((CFNumberRef)cf, kCFNumberFloat64Type, &d);
    if (isnan(d)) {
	return (CFStringRef)CFRetain(CFSTR("nan"));
    }
    if (isinf(d)) {
	return (CFStringRef)CFRetain((0.0 < d) ? CFSTR("+infinity") : CFSTR("-infinity"));
    }
    if (0.0 == d) {
	return (CFStringRef)CFRetain(CFSTR("0.0"));
    }
    // if %g is used here, need to use DBL_DIG + 2 on Mac OS X, but %f needs +1
    return CFStringCreateWithFormat(allocator, NULL, CFSTR("%.*g"), DBL_DIG + 2, d);
}

CF_PRIVATE CFStringRef __CFNumberCopyFormattingDescriptionAsFloat64(CFTypeRef cf) {
    CFStringRef result = __CFNumberCreateFormattingDescriptionAsFloat64(kCFAllocatorSystemDefault, cf);
#if OLD_CRAP_TOO
CFNumberRef number = (CFNumberRef)cf;
if (! number->__old__) {
printf("*** Test skipped in __CFNumberCopyFormattingDescriptionAsFloat64 for number %p\n", cf);
} else {
CFStringRef test = __CFNumberCopyFormattingDescriptionAsFloat64_old(number->__old__);
if (!CFEqual(test, result)) {
CFLog(kCFLogLevelWarning, CFSTR("*** TEST FAIL in __CFNumberCopyFormattingDescriptionAsFloat64: '%@' '%@'"), test, result);  FAIL();
}
}
#endif
    return result;
}

CF_PRIVATE CFStringRef __CFNumberCreateFormattingDescription(CFAllocatorRef allocator, CFTypeRef cf, CFDictionaryRef formatOptions) {
    CFNumberRef number = (CFNumberRef)cf;
    CFNumberType type = __CFNumberGetType(number);
    if (__CFNumberTypeTable[type].floatBit) {
        return __CFNumberCreateFormattingDescriptionAsFloat64(allocator, number);
    }
    CFSInt128Struct i;
    __CFNumberGetValue(number, kCFNumberSInt128Type, &i);
    char buffer[128];
    emit128(buffer, &i, false);
    return CFStringCreateWithFormat(allocator, NULL, CFSTR("%s"), buffer);
}

static CFStringRef __CFNumberCopyFormattingDescription_new(CFTypeRef cf, CFDictionaryRef formatOptions) {
    CFNumberRef number = (CFNumberRef)cf;
    CFNumberType type = __CFNumberGetType(number);
    if (__CFNumberTypeTable[type].floatBit) {
        return __CFNumberCopyFormattingDescriptionAsFloat64(number);
    }
    CFSInt128Struct i;
    __CFNumberGetValue(number, kCFNumberSInt128Type, &i);
    char buffer[128];
    emit128(buffer, &i, false);
    return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%s"), buffer);
}

CF_PRIVATE CFStringRef __CFNumberCopyFormattingDescription(CFTypeRef cf, CFDictionaryRef formatOptions) {
    CFStringRef result = __CFNumberCopyFormattingDescription_new(cf, formatOptions);
#if OLD_CRAP_TOO
CFNumberRef number = (CFNumberRef)cf;
if (! number->__old__) {
printf("*** Test skipped in __CFNumberCopyFormattingDescription for number %p\n", cf);
} else {
CFStringRef test = __CFNumberCopyFormattingDescription_old(number->__old__, formatOptions);
if (!CFEqual(test, result)) {
CFLog(kCFLogLevelWarning, CFSTR("*** TEST FAIL in __CFNumberCopyFormattingDescription: '%@' '%@'"), test, result);  FAIL();
}
}
#endif
    return result;
}


static Boolean __CFNumberEqual(CFTypeRef cf1, CFTypeRef cf2) {
    Boolean b = CFNumberCompare((CFNumberRef)cf1, (CFNumberRef)cf2, 0) == kCFCompareEqualTo;
#if OLD_CRAP_TOO
CFNumberRef number1 = (CFNumberRef)cf1;
CFNumberRef number2 = (CFNumberRef)cf2;
if (! number1->__old__ || !number2->__old__) {
printf("*** Test skipped in __CFNumberEqual for numbers %p %p\n", cf1, cf2);
} else {
Boolean b2 = __CFNumberEqual_old(number1->__old__, number2->__old__);
if (b2 != b) {
CFLog(kCFLogLevelWarning, CFSTR("*** TEST FAIL in __CFNumberEqual: '%d' '%d'"), b2, b);  FAIL();
}
}
#endif
    return b;
}

static CFHashCode __CFNumberHash(CFTypeRef cf) {
    CFHashCode h;
    CFNumberRef number = (CFNumberRef)cf;
    switch (__CFNumberGetType(number)) {
	case kCFNumberSInt8Type:
	case kCFNumberSInt16Type:
	case kCFNumberSInt32Type: {
	    SInt32 i;
	    __CFNumberGetValue(number, kCFNumberSInt32Type, &i);
	    h = _CFHashInt(i);
	    break;
	}
	default: {
	    Float64 d;
	    __CFNumberGetValue(number, kCFNumberFloat64Type, &d);
	    h = _CFHashDouble((double)d);
	    break;
	}
    }
#if OLD_CRAP_TOO
CFNumberRef number1 = (CFNumberRef)cf;
if (! number1->__old__) {
printf("*** Test skipped in __CFNumberHash for number %p\n", cf);
} else {
CFHashCode h2 = __CFNumberHash_old(number1->__old__);
if (h2 != h) {
CFLog(kCFLogLevelWarning, CFSTR("*** TEST FAIL in __CFNumberHash: '%d' '%d'"), h2, h);  FAIL();
}
}
#endif
    return h;
}

static CFTypeID __kCFNumberTypeID = _kCFRuntimeNotATypeID;

enum {
  kCFNumberCachingEnabled = 0,
  kCFNumberCachingDisabled = 1,
  kCFNumberCachingFullyDisabled = 2
};
static char __CFNumberCaching = kCFNumberCachingEnabled;

static const CFRuntimeClass __CFNumberClass = {
    0,
    "CFNumber",
    NULL,      // init
    NULL,      // copy
    NULL,
    __CFNumberEqual,
    __CFNumberHash,
    __CFNumberCopyFormattingDescription,
    __CFNumberCopyDescription
};


CFTypeID CFNumberGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{
        __kCFNumberTypeID = _CFRuntimeRegisterClass(&__CFNumberClass); // initOnce covered

        _CFRuntimeSetInstanceTypeIDAndIsa(&__kCFNumberNaN, __kCFNumberTypeID);
        __CFBitfieldSetValue(__kCFNumberNaN._base._cfinfo[CF_INFO_BITS], 4, 0, kCFNumberFloat64Type);
        __kCFNumberNaN._pad = BITSFORDOUBLENAN;

        _CFRuntimeSetInstanceTypeIDAndIsa(& __kCFNumberNegativeInfinity, __kCFNumberTypeID);
        __CFBitfieldSetValue(__kCFNumberNegativeInfinity._base._cfinfo[CF_INFO_BITS], 4, 0, kCFNumberFloat64Type);
        __kCFNumberNegativeInfinity._pad = BITSFORDOUBLENEGINF;

        _CFRuntimeSetInstanceTypeIDAndIsa(& __kCFNumberPositiveInfinity, __kCFNumberTypeID);
        __CFBitfieldSetValue(__kCFNumberPositiveInfinity._base._cfinfo[CF_INFO_BITS], 4, 0, kCFNumberFloat64Type);
        __kCFNumberPositiveInfinity._pad = BITSFORDOUBLEPOSINF;


        const char *caching = __CFgetenv("CFNumberDisableCache");	// "all" to disable caching and tagging; anything else to disable caching; nothing to leave both enabled
        if (caching) __CFNumberCaching = (!strcmp(caching, "all")) ? kCFNumberCachingFullyDisabled : kCFNumberCachingDisabled;	// initial state above is kCFNumberCachingEnabled
    });
    return __kCFNumberTypeID;
}

#define MinCachedInt (-1)
#define MaxCachedInt (12)
#define NotToBeCached (MinCachedInt - 1)
static CFNumberRef __CFNumberCache[MaxCachedInt - MinCachedInt + 1] = {NULL};	// Storing CFNumberRefs for range MinCachedInt..MaxCachedInt

CFNumberRef CFNumberCreate(CFAllocatorRef allocator, CFNumberType type, const void *valuePtr) {
    __CFAssertIsValidNumberType(type);
//printf("+ [%p] CFNumberCreate(%p, %d, %p)\n", pthread_self(), allocator, type, valuePtr);

    if (!allocator) allocator = __CFGetDefaultAllocator();


    // Look for cases where we can return a cached instance.
    // We only use cached objects if the allocator is the system
    // default allocator, except for the special floating point
    // constant objects, where we return the cached object
    // regardless of allocator, since that is what has always
    // been done (and now must for compatibility).
    int64_t valToBeCached = NotToBeCached;

    if (__CFNumberTypeTable[type].floatBit) {
	CFNumberRef cached = NULL;
	if (0 == __CFNumberTypeTable[type].storageBit) {
	    Float32 f = *(Float32 *)valuePtr;
	    if (isnan(f)) cached = kCFNumberNaN;
	    if (isinf(f)) cached = (f < 0.0) ? kCFNumberNegativeInfinity : kCFNumberPositiveInfinity;
	} else {
            Float64 d = *(Float64 *)valuePtr;
	    if (isnan(d)) cached = kCFNumberNaN;
	    if (isinf(d)) cached = (d < 0.0) ? kCFNumberNegativeInfinity : kCFNumberPositiveInfinity;
	}
	if (cached) return (CFNumberRef)CFRetain(cached);
    } else if (_CFAllocatorIsSystemDefault(allocator) && (__CFNumberCaching == kCFNumberCachingEnabled)) {
	switch (__CFNumberTypeTable[type].canonicalType) {
	case kCFNumberSInt8Type:   {int8_t  val = *(int8_t *)valuePtr;  if (MinCachedInt <= val && val <= MaxCachedInt) valToBeCached = (int64_t)val; break;}
	case kCFNumberSInt16Type:  {int16_t val = *(int16_t *)valuePtr; if (MinCachedInt <= val && val <= MaxCachedInt) valToBeCached = (int64_t)val; break;}
	case kCFNumberSInt32Type:  {int32_t val = *(int32_t *)valuePtr; if (MinCachedInt <= val && val <= MaxCachedInt) valToBeCached = (int64_t)val; break;}
	case kCFNumberSInt64Type:  {int64_t val = *(int64_t *)valuePtr; if (MinCachedInt <= val && val <= MaxCachedInt) valToBeCached = (int64_t)val; break;}
	}
	if (NotToBeCached != valToBeCached) {
	    CFNumberRef cached = __CFNumberCache[valToBeCached - MinCachedInt];	    // Atomic to access the value in the cache
	    if (NULL != cached) return (CFNumberRef)CFRetain(cached);
	}
    }

    CFIndex size = 8 + ((!__CFNumberTypeTable[type].floatBit && __CFNumberTypeTable[type].storageBit) ? 8 : 0);
#if OLD_CRAP_TOO
    size += 2 * sizeof(void *);
#endif
    CFNumberRef result = (CFNumberRef)_CFRuntimeCreateInstance(allocator, CFNumberGetTypeID(), size, NULL);
    if (NULL == result) {
	return NULL;
    }
    __CFBitfieldSetValue(((struct __CFNumber *)result)->_base._cfinfo[CF_INFO_BITS], 4, 0, (uint8_t)__CFNumberTypeTable[type].canonicalType);

#if OLD_CRAP_TOO
    ((struct __CFNumber *)result)->__old__ = CFNumberCreate_old(allocator, type, valuePtr);
CFLog(kCFLogLevelWarning, CFSTR("+++ Create old number '%@'"), __CFNumberCopyDescription_old(result->__old__));

#endif

    // for a value to be cached, we already have the value handy
    if (NotToBeCached != valToBeCached) {
	memmove((void *)&result->_pad, &valToBeCached, 8);
	// Put this in the cache unless the cache is already filled (by another thread).  If we do put it in the cache, retain it an extra time for the cache.
	// Note that we don't bother freeing this result and returning the cached value if the cache was filled, since cached CFNumbers are not guaranteed unique.
	// Barrier assures that the number that is placed in the cache is properly formed.
	CFNumberType origType = __CFNumberGetType(result);
	// Force all cached numbers to have the same type, so that the type does not
	// depend on the order and original type in/with which the numbers are created.
	// Forcing the type AFTER it was cached would cause a race condition with other
	// threads pulling the number object out of the cache and using it.
	__CFBitfieldSetValue(((struct __CFNumber *)result)->_base._cfinfo[CF_INFO_BITS], 4, 0, (uint8_t)kCFNumberSInt32Type);
	if (OSAtomicCompareAndSwapPtrBarrier(NULL, (void *)result, (void *volatile *)&__CFNumberCache[valToBeCached - MinCachedInt])) {
	    CFRetain(result);
	} else {
	    // Did not cache the number object, put original type back.
	    __CFBitfieldSetValue(((struct __CFNumber *)result)->_base._cfinfo[CF_INFO_BITS], 4, 0, (uint8_t)origType);
	}
	return result;
    }

    uint64_t value;
    switch (__CFNumberTypeTable[type].canonicalType) {
    case kCFNumberSInt8Type:   value = (uint64_t)(int64_t)*(int8_t *)valuePtr; goto smallVal;
    case kCFNumberSInt16Type:  value = (uint64_t)(int64_t)*(int16_t *)valuePtr; goto smallVal;
    case kCFNumberSInt32Type:  value = (uint64_t)(int64_t)*(int32_t *)valuePtr; goto smallVal;
			smallVal: memmove((void *)&result->_pad, &value, 8); break;
    case kCFNumberSInt64Type:  memmove((void *)&result->_pad, valuePtr, 8); break;
    case kCFNumberSInt128Type: memmove((void *)&result->_pad, valuePtr, 16); break;
    case kCFNumberFloat32Type: memmove((void *)&result->_pad, valuePtr, 4); break;
    case kCFNumberFloat64Type: memmove((void *)&result->_pad, valuePtr, 8); break;
    }
//printf("  => %p\n", result);
    return result;
}

CFNumberType CFNumberGetType(CFNumberRef number) {
//printf("+ [%p] CFNumberGetType(%p)\n", pthread_self(), number);
    CF_OBJC_FUNCDISPATCHV(CFNumberGetTypeID(), CFNumberType, (NSNumber *)number, _cfNumberType);
    __CFAssertIsNumber(number);
    CFNumberType type = __CFNumberGetType(number);
    if (kCFNumberSInt128Type == type) type = kCFNumberSInt64Type; // must hide this type, since it is not public
//printf("  => %d\n", type);
#if OLD_CRAP_TOO
if (! number->__old__) {
printf("*** Test skipped in CFNumberGetType for number %p\n", number);
} else {
CFNumberType t2 = CFNumberGetType_old(number->__old__);
if (t2 != type) {
CFLog(kCFLogLevelWarning, CFSTR("*** TEST FAIL in CFNumberGetType: '%d' '%d'"), t2, type);  FAIL();
}
}
#endif
    return type;
}

CF_EXPORT CFNumberType _CFNumberGetType2(CFNumberRef number) {
    CF_OBJC_FUNCDISPATCHV(CFNumberGetTypeID(), CFNumberType, (NSNumber *)number, _cfNumberType);
    __CFAssertIsNumber(number);
    return __CFNumberGetType(number);
}

CFIndex CFNumberGetByteSize(CFNumberRef number) {
//printf("+ [%p] CFNumberGetByteSize(%p)\n", pthread_self(), number);
    __CFAssertIsNumber(number);
    CFIndex r = 1 << __CFNumberTypeTable[CFNumberGetType(number)].lgByteSize;
//printf("  => %d\n", r);
#if OLD_CRAP_TOO
if (! number->__old__) {
printf("*** Test skipped in CFNumberGetByteSize for number %p\n", number);
} else {
CFIndex r2 = CFNumberGetByteSize_old(number->__old__);
if (r2 != r) {
CFLog(kCFLogLevelWarning, CFSTR("*** TEST FAIL in CFNumberGetByteSize: '%d' '%d'"), r2, r);  FAIL();
}
}
#endif
    return r;
}

Boolean CFNumberIsFloatType(CFNumberRef number) {
//printf("+ [%p] CFNumberIsFloatType(%p)\n", pthread_self(), number);
    __CFAssertIsNumber(number);
    Boolean r = __CFNumberTypeTable[CFNumberGetType(number)].floatBit;
//printf("  => %d\n", r);
#if OLD_CRAP_TOO
if (! number->__old__) {
printf("*** Test skipped in CFNumberIsFloatType for number %p\n", number);
} else {
Boolean r2 = CFNumberIsFloatType_old(number->__old__);
if (r2 != r) {
CFLog(kCFLogLevelWarning, CFSTR("*** TEST FAIL in CFNumberIsFloatType: '%d' '%d'"), r2, r);  FAIL();
}
}
#endif
    return r;
}

Boolean CFNumberGetValue(CFNumberRef number, CFNumberType type, void *valuePtr) {
//printf("+ [%p] CFNumberGetValue(%p, %d, %p)\n", pthread_self(), number, type, valuePtr);

    CF_OBJC_FUNCDISPATCHV(CFNumberGetTypeID(), Boolean, (NSNumber *)number, _getValue:(void *)valuePtr forType:(CFNumberType)__CFNumberTypeTable[type].canonicalType);
    __CFAssertIsNumber(number);
    __CFAssertIsValidNumberType(type);
    uint8_t localMemory[128];
    Boolean r = __CFNumberGetValueCompat(number, type, valuePtr ? valuePtr : localMemory);
//printf("  => %d\n", r);
#if OLD_CRAP_TOO
if (! number->__old__) {
printf("*** Test skipped in CFNumberGetValue for number %p\n", number);
} else {
    uint8_t localMemory2[128];
Boolean r2 = CFNumberGetValue_old(number->__old__, type, localMemory2);
if (r2 != r) {
CFLog(kCFLogLevelWarning, CFSTR("*** TEST FAIL 1 in CFNumberGetValue: '%d' '%d'"), r2, r);  FAIL();
}
if (0 != memcmp(localMemory2, valuePtr, CFNumberGetByteSize(number))) {
CFLog(kCFLogLevelWarning, CFSTR("*** TEST FAIL 2 in CFNumberGetValue: BYTES NOT SAME"));  FAIL();
}
}
#endif
    return r;
}

static CFComparisonResult CFNumberCompare_new(CFNumberRef number1, CFNumberRef number2, void *context) {
    CF_OBJC_FUNCDISPATCHV(CFNumberGetTypeID(), CFComparisonResult, (NSNumber *)number1, compare:(NSNumber *)number2);
    CF_OBJC_FUNCDISPATCHV(CFNumberGetTypeID(), CFComparisonResult, (NSNumber *)number2, _reverseCompare:(NSNumber *)number1);
    __CFAssertIsNumber(number1);
    __CFAssertIsNumber(number2);

    CFNumberType type1 = __CFNumberGetType(number1);
    CFNumberType type2 = __CFNumberGetType(number2);
    // Both numbers are integers
    if (!__CFNumberTypeTable[type1].floatBit && !__CFNumberTypeTable[type2].floatBit) {
        CFSInt128Struct i1, i2;
        __CFNumberGetValue(number1, kCFNumberSInt128Type, &i1);
        __CFNumberGetValue(number2, kCFNumberSInt128Type, &i2);
        return cmp128(&i1, &i2);
    }
    // Both numbers are floats
    if (__CFNumberTypeTable[type1].floatBit && __CFNumberTypeTable[type2].floatBit) {
	Float64 d1, d2;
        __CFNumberGetValue(number1, kCFNumberFloat64Type, &d1);
        __CFNumberGetValue(number2, kCFNumberFloat64Type, &d2);
	double s1 = copysign(1.0, d1);
	double s2 = copysign(1.0, d2);
	if (isnan(d1) && isnan(d2)) return kCFCompareEqualTo;
	if (isnan(d1)) return (s2 < 0.0) ? kCFCompareGreaterThan : kCFCompareLessThan;
	if (isnan(d2)) return (s1 < 0.0) ? kCFCompareLessThan : kCFCompareGreaterThan;
	// at this point, we know we don't have any NaNs
	if (s1 < s2) return kCFCompareLessThan;
	if (s2 < s1) return kCFCompareGreaterThan;
	// at this point, we know the signs are the same; do not combine these tests
	if (d1 < d2) return kCFCompareLessThan;
	if (d2 < d1) return kCFCompareGreaterThan;
        return kCFCompareEqualTo;
    }
    // One float, one integer; swap if necessary so number1 is the float
    Boolean swapResult = false;
    if (__CFNumberTypeTable[type2].floatBit) {
        CFNumberRef tmp = number1;
	number1 = number2;
	number2 = tmp;
	swapResult = true;
    }
    // At large integer values, the precision of double is quite low
    // e.g. all values roughly 2^127 +- 2^73 are represented by 1 double, 2^127.
    // If we just used double compare, that would make the 2^73 largest 128-bit
    // integers look equal, so we have to use integer comparison when possible.
    Float64 d1, d2;
    __CFNumberGetValue(number1, kCFNumberFloat64Type, &d1);
    // if the double value is really big, cannot be equal to integer
    // nan d1 will not compare true here
    if (d1 < FLOAT_NEGATIVE_2_TO_THE_127) {
	return !swapResult ? kCFCompareLessThan : kCFCompareGreaterThan;
    }
    if (FLOAT_POSITIVE_2_TO_THE_127 <= d1) {
	return !swapResult ? kCFCompareGreaterThan : kCFCompareLessThan;
    }
    CFSInt128Struct i1, i2;
    __CFNumberGetValue(number1, kCFNumberSInt128Type, &i1);
    __CFNumberGetValue(number2, kCFNumberSInt128Type, &i2);
    CFComparisonResult res = cmp128(&i1, &i2);
    if (kCFCompareEqualTo != res) {
	return !swapResult ? res : -res;
    }
    // now things are equal, but perhaps due to rounding or nan
    if (isnan(d1)) {
	if (isNeg128(&i2)) {
	    return !swapResult ? kCFCompareGreaterThan : kCFCompareLessThan;
	}
	// nan compares less than positive 0 too
	return !swapResult ? kCFCompareLessThan : kCFCompareGreaterThan;
    }
    // at this point, we know we don't have NaN
    double s1 = copysign(1.0, d1);
    double s2 = isNeg128(&i2) ? -1.0 : 1.0;
    if (s1 < s2) return !swapResult ? kCFCompareLessThan : kCFCompareGreaterThan;
    if (s2 < s1) return !swapResult ? kCFCompareGreaterThan : kCFCompareLessThan;
    // at this point, we know the signs are the same; do not combine these tests
    __CFNumberGetValue(number2, kCFNumberFloat64Type, &d2);
    if (d1 < d2) return !swapResult ? kCFCompareLessThan : kCFCompareGreaterThan;
    if (d2 < d1) return !swapResult ? kCFCompareGreaterThan : kCFCompareLessThan;
    return kCFCompareEqualTo;
}

CFComparisonResult CFNumberCompare(CFNumberRef number1, CFNumberRef number2, void *context) {
//printf("+ [%p] CFNumberCompare(%p, %p, %p)\n", pthread_self(), number1, number2, context);
    CFComparisonResult r = CFNumberCompare_new(number1, number2, context);
//printf("  => %d\n", r);
#if OLD_CRAP_TOO
if (! number1->__old__ || !number2->__old__) {
printf("*** Test skipped in CFNumberCompare for numbers %p %p\n", number1, number2);
} else {
CFComparisonResult r2 = CFNumberCompare_old(number1->__old__, number2->__old__, context);
if (r2 != r) {
CFLog(kCFLogLevelWarning, CFSTR("*** TEST FAIL in CFNumberCompare: '%d' '%d'"), r2, r);  FAIL();
}
}
#endif
    return r;
}

#if OLD_CRAP_TOO

static const unsigned char __CFNumberCanonicalType[kCFNumberMaxType + 1] = {
    0, kCFNumberSInt8Type, kCFNumberSInt16Type, kCFNumberSInt32Type, kCFNumberSInt64Type, kCFNumberFloat32Type, kCFNumberFloat64Type,
    kCFNumberSInt8Type, kCFNumberSInt16Type, kCFNumberSInt32Type, kCFNumberSInt32Type, kCFNumberSInt64Type, kCFNumberFloat32Type, kCFNumberFloat64Type,
    kCFNumberSInt32Type, kCFNumberSInt32Type, kCFNumberFloat32Type
};

static const unsigned char __CFNumberStorageType[kCFNumberMaxType + 1] = {
    0, kCFNumberSInt32Type, kCFNumberSInt32Type, kCFNumberSInt32Type, kCFNumberSInt64Type, kCFNumberFloat32Type, kCFNumberFloat64Type,
    kCFNumberSInt32Type, kCFNumberSInt32Type, kCFNumberSInt32Type, kCFNumberSInt32Type, kCFNumberSInt64Type, kCFNumberFloat32Type, kCFNumberFloat64Type,
    kCFNumberSInt32Type, kCFNumberSInt32Type, kCFNumberFloat32Type
};



// Returns the type that is used to store the specified type
static CFNumberType __CFNumberGetStorageTypeForType_old(CFNumberType type) {
    return __CFNumberStorageType[type];
}

// Returns the canonical type used to represent the specified type
static CFNumberType __CFNumberGetCanonicalTypeForType_old(CFNumberType type) {
    return __CFNumberCanonicalType[type];
}

// Extracts and returns the type out of the CFNumber
static CFNumberType __CFNumberGetType_old(struct __CFNumber_old * num) {
    return __CFBitfieldGetValue(num->_base._cfinfo[CF_INFO_BITS], 4, 0);
}

// Returns true if the argument type is float or double
static Boolean __CFNumberTypeIsFloat_old(CFNumberType type) {
    return (type == kCFNumberFloat64Type) || (type == kCFNumberFloat32Type) || (type == kCFNumberDoubleType) || (type == kCFNumberFloatType);
}

// Returns the number of bytes necessary to store the specified type
// Needs to handle all canonical types
static CFIndex __CFNumberSizeOfType_old(CFNumberType type) {
    switch (type) {
        case kCFNumberSInt8Type:        return sizeof(int8_t);
        case kCFNumberSInt16Type:       return sizeof(int16_t);
        case kCFNumberSInt32Type:       return sizeof(SInt32);
        case kCFNumberSInt64Type:       return sizeof(int64_t);
        case kCFNumberFloat32Type:      return sizeof(Float32);
        case kCFNumberFloat64Type:      return sizeof(Float64);
        default:                        printf("*** WARNING: 0 size from __CFNumberSizeOfType_old \n"); return 0;
    }
}

// Copies an external value of a given type into the appropriate slot in the union (does no type conversion)
// Needs to handle all canonical types
#define SET_VALUE(valueUnion, type, valuePtr)   \
    switch (type) {                             \
        case kCFNumberSInt8Type:        (valueUnion)->valSInt32 = *(int8_t *)(valuePtr); break; \
        case kCFNumberSInt16Type:       (valueUnion)->valSInt32 = *(int16_t *)(valuePtr); break;        \
        case kCFNumberSInt32Type:       (valueUnion)->valSInt32 = *(SInt32 *)(valuePtr); break; \
        case kCFNumberSInt64Type:       (valueUnion)->valSInt64 = *(int64_t *)(valuePtr); break;        \
        case kCFNumberFloat32Type:      (valueUnion)->valFloat32 = *(Float32 *)(valuePtr); break;       \
        case kCFNumberFloat64Type:      (valueUnion)->valFloat64 = *(Float64 *)(valuePtr); break;       \
        default: printf("*** WARNING: default case in SET_VALUE \n"); break; \
    }

// Casts the specified value into the specified type and copies it into the provided memory
// Needs to handle all canonical types
#define GET_VALUE(value, type, resultPtr)       \
    switch (type) {                             \
        case kCFNumberSInt8Type:        *(int8_t *)(resultPtr) = (int8_t)value; break;  \
        case kCFNumberSInt16Type:       *(int16_t *)(resultPtr) = (int16_t)value; break;        \
        case kCFNumberSInt32Type:       *(SInt32 *)(resultPtr) = (SInt32)value; break;  \
        case kCFNumberSInt64Type:       *(int64_t *)(resultPtr) = (int64_t)value; break;        \
        case kCFNumberFloat32Type:      *(Float32 *)(resultPtr) = (Float32)value; break;        \
        case kCFNumberFloat64Type:      *(Float64 *)(resultPtr) = (Float64)value; break;        \
        default: printf("*** WARNING: default case in GET_VALUE \n"); break; \
    }

// Extracts the stored type out of the union and copies it in the desired type into the provided memory
// Needs to handle all storage types
static void __CFNumberGetValue_old(const __CFNumberValue_old *value, CFNumberType numberType, CFNumberType typeToGet, void *valuePtr) {
    switch (numberType) {
        case kCFNumberSInt32Type:       GET_VALUE(value->valSInt32, typeToGet, valuePtr); break;
        case kCFNumberSInt64Type:       GET_VALUE(value->valSInt64, typeToGet, valuePtr); break;
        case kCFNumberFloat32Type:      GET_VALUE(value->valFloat32, typeToGet, valuePtr); break;
        case kCFNumberFloat64Type:      GET_VALUE(value->valFloat64, typeToGet, valuePtr); break;
        default: printf("*** WARNING: default case in __CFNumberGetValue_old \n"); break; \
    }
}

// Sees if two value union structs have the same value (will do type conversion)
static Boolean __CFNumberEqualValue_old(const __CFNumberValue_old *value1, CFNumberType type1, const __CFNumberValue_old *value2, CFNumberType type2) {
    if (__CFNumberTypeIsFloat_old(type1) || __CFNumberTypeIsFloat_old(type2)) {
        Float64 d1, d2;
        __CFNumberGetValue_old(value1, type1, kCFNumberFloat64Type, &d1);
        __CFNumberGetValue_old(value2, type2, kCFNumberFloat64Type, &d2);
            if (isnan(d1) && isnan(d2)) return true;    // Not mathematically sound, but required
        return d1 == d2;
    } else {
        int64_t i1, i2;
        __CFNumberGetValue_old(value1, type1, kCFNumberSInt64Type, &i1);
        __CFNumberGetValue_old(value2, type2, kCFNumberSInt64Type, &i2);
        return i1 == i2;
    }
}

static Boolean __CFNumberEqual_old(CFTypeRef cf1, CFTypeRef cf2) {
    struct __CFNumber_old * number1 = (struct __CFNumber_old *)cf1;
    struct __CFNumber_old * number2 = (struct __CFNumber_old *)cf2;
    return __CFNumberEqualValue_old(&(number1->value), __CFNumberGetType_old(number1), &(number2->value), __CFNumberGetType_old(number2));
}

static CFHashCode __CFNumberHash_old(CFTypeRef cf) {
    struct __CFNumber_old * number = (struct __CFNumber_old *)cf;
    switch (__CFNumberGetType_old((struct __CFNumber_old *)cf)) {
        case kCFNumberSInt32Type: return _CFHashInt(number->value.valSInt32);
        case kCFNumberSInt64Type: return _CFHashDouble((double)(number->value.valSInt64));
        case kCFNumberFloat32Type: return _CFHashDouble((double)(number->value.valFloat32));
        case kCFNumberFloat64Type: return _CFHashDouble((double)(number->value.valFloat64));
        default: printf("*** WARNING default case in __CFNumberHash_old\n");
            return 0;
    }
}

#define BUFFER_SIZE 100
#define emitChar(ch) \
    {if (buf - stackBuf == BUFFER_SIZE) {CFStringAppendCharacters(mstr, stackBuf, BUFFER_SIZE); buf = stackBuf;} *buf++ = ch;}
                 
static void __CFNumberEmitInt64_old(CFMutableStringRef mstr, int64_t value, int32_t width, UniChar pad, bool explicitPlus) {
    UniChar stackBuf[BUFFER_SIZE], *buf = stackBuf;
    uint64_t uvalue, factor, tmp;
    int32_t w;
    bool neg;

    neg = (value < 0) ? true : false;
    uvalue = (neg) ? -value : value;
    if (neg || explicitPlus) width--;
    width--;
    factor = 1;
    tmp = uvalue;
    while (9 < tmp) {
        width--;
        factor *= 10;
        tmp /= 10;
    }
    for (w = 0; w < width; w++) emitChar(pad);
    if (neg) {
        emitChar('-');
    } else if (explicitPlus) {
        emitChar('+');
    }
    while (0 < factor) {
        UniChar ch = '0' + (UniChar)(uvalue / factor);
        uvalue %= factor;
        emitChar(ch);
        factor /= 10;
    }
    if (buf > stackBuf) CFStringAppendCharacters(mstr, stackBuf, buf - stackBuf);
}

static CFStringRef __CFNumberCopyDescription_old(CFTypeRef cf) {
    struct __CFNumber_old * number = (struct __CFNumber_old *)cf;
    CFMutableStringRef mstr = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    CFStringAppendFormat(mstr, NULL, CFSTR("<CFNumber %p [%p]>{value = "), cf, CFGetAllocator(cf));
    switch (__CFNumberGetType_old(number)) {
    case kCFNumberSInt32Type:
        __CFNumberEmitInt64_old(mstr, number->value.valSInt32, 0, ' ', true);
        CFStringAppendFormat(mstr, NULL, CFSTR(", type = kCFNumberSInt32Type}"));
        break;
    case kCFNumberSInt64Type:
        __CFNumberEmitInt64_old(mstr, number->value.valSInt64, 0, ' ', true);
        CFStringAppendFormat(mstr, NULL, CFSTR(", type = kCFNumberSInt64Type}"));
        break;
    case kCFNumberFloat32Type:
        // debugging formatting is intentionally more verbose and explicit about the value of the number
        if (isnan(number->value.valFloat32)) {
            CFStringAppend(mstr, CFSTR("nan"));
        } else if (isinf(number->value.valFloat32)) {
            CFStringAppend(mstr, (0.0f < number->value.valFloat32) ? CFSTR("+infinity") : CFSTR("-infinity"));
        } else if (0.0f == number->value.valFloat32) {
            CFStringAppend(mstr, (copysign(1.0, number->value.valFloat32) < 0.0) ? CFSTR("-0.0") : CFSTR("+0.0"));
        } else {
            CFStringAppendFormat(mstr, NULL, CFSTR("%+.10f"), number->value.valFloat32);
        }
        CFStringAppend(mstr, CFSTR(", type = kCFNumberFloat32Type}"));
        break;
    case kCFNumberFloat64Type:
        // debugging formatting is intentionally more verbose and explicit about the value of the number
        if (isnan(number->value.valFloat64)) {
            CFStringAppend(mstr, CFSTR("nan"));
        } else if (isinf(number->value.valFloat64)) {
            CFStringAppend(mstr, (0.0 < number->value.valFloat64) ? CFSTR("+infinity") : CFSTR("-infinity"));
        } else if (0.0 == number->value.valFloat64) {
            CFStringAppend(mstr, (copysign(1.0, number->value.valFloat64) < 0.0) ? CFSTR("-0.0") : CFSTR("+0.0"));
        } else {
            CFStringAppendFormat(mstr, NULL, CFSTR("%+.20f"), number->value.valFloat64);
        }
        CFStringAppend(mstr, CFSTR(", type = kCFNumberFloat64Type}"));
        break;
    default:
        CFRelease(mstr);
        return NULL;
    }
    return mstr;
}

// This function separated out from __CFNumberCopyFormattingDescription() so the plist creation can use it as well.

CF_PRIVATE CFStringRef __CFNumberCopyFormattingDescriptionAsFloat64_old(CFTypeRef cf) {
    double d;
    CFNumberGetValue_old((struct __CFNumber_old *)cf, kCFNumberFloat64Type, &d);
        if (isnan(d)) {
            return (CFStringRef)CFRetain(CFSTR("nan"));
        }
        if (isinf(d)) {
            return (CFStringRef)CFRetain((0.0 < d) ? CFSTR("+infinity") : CFSTR("-infinity"));
        }
        if (0.0 == d) {
            return (CFStringRef)CFRetain(CFSTR("0.0"));
        }
        // if %g is used here, need to use DBL_DIG + 2 on Mac OS X, but %f needs +1
        return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%.*g"), DBL_DIG + 2, d);
}

static CFStringRef __CFNumberCopyFormattingDescription_old(CFTypeRef cf, CFDictionaryRef formatOptions) {
    struct __CFNumber_old * number = (struct __CFNumber_old *)cf;
    CFMutableStringRef mstr;
    int64_t value;
    switch (__CFNumberGetType_old(number)) {
    case kCFNumberSInt32Type:
    case kCFNumberSInt64Type: 
        value = (__CFNumberGetType_old(number) == kCFNumberSInt32Type) ? number->value.valSInt32 : number->value.valSInt64;
        mstr = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
        __CFNumberEmitInt64_old(mstr, value, 0, ' ', false);
        return mstr;
    case kCFNumberFloat32Type:
            if (isnan(number->value.valFloat32)) {
                return (CFStringRef)CFRetain(CFSTR("nan"));
            }
            if (isinf(number->value.valFloat32)) {
                return (CFStringRef)CFRetain((0.0f < number->value.valFloat32) ? CFSTR("+infinity") : CFSTR("-infinity"));
            }
            if (0.0f == number->value.valFloat32) {
                return (CFStringRef)CFRetain(CFSTR("0.0"));
            }
            // if %g is used here, need to use FLT_DIG + 2 on Mac OS X, but %f needs +1
            return CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%.*g"), FLT_DIG + 2, number->value.valFloat32);
    case kCFNumberFloat64Type:
        return __CFNumberCopyFormattingDescriptionAsFloat64_old(number);
        break;
    default:
        return NULL;
    }
}



static struct __CFNumber_old * CFNumberCreate_old(CFAllocatorRef allocator, CFNumberType type, const void *valuePtr) {
    struct __CFNumber_old * num;
    CFNumberType equivType, storageType;

if (type == 17) {
CFSInt128Struct *s = valuePtr;
s->high = (int64_t)s->low;
type = kCFNumberSInt64Type;
}

    
    equivType = __CFNumberGetCanonicalTypeForType_old(type);

    storageType = __CFNumberGetStorageTypeForType_old(type);

    num = (struct __CFNumber_old *)_CFRuntimeCreateInstance(allocator, CFNumberGetTypeID(), __CFNumberSizeOfType_old(storageType), NULL);
    if (NULL == num) {
        return NULL;
    }
    SET_VALUE((__CFNumberValue_old *)&(num->value), equivType, valuePtr);
    __CFBitfieldSetValue(((struct __CFNumber_old *)num)->_base._cfinfo[CF_INFO_BITS], 6, 0, (uint8_t)storageType);
    
if (__CFNumberGetType_old(num) == 0) printf("*** ERROR: new number %p type is 0 (%d)\n", num, storageType);
    return num;
}

static CFNumberType CFNumberGetType_old(struct __CFNumber_old * number) {

    return __CFNumberGetType_old(number);
}

static CFIndex CFNumberGetByteSize_old(struct __CFNumber_old * number) {
    return __CFNumberSizeOfType_old(CFNumberGetType_old(number));
}

static Boolean CFNumberIsFloatType_old(struct __CFNumber_old * number) {
    return __CFNumberTypeIsFloat_old(CFNumberGetType_old(number));
}

static Boolean CFNumberGetValue_old(struct __CFNumber_old * number, CFNumberType type, void *valuePtr) {
    uint8_t localMemory[sizeof(__CFNumberValue_old)];
    __CFNumberValue_old localValue;
    CFNumberType numType;
    CFNumberType storageTypeForType;

if (type == 17) type = kCFNumberSInt64Type;

    storageTypeForType = __CFNumberGetStorageTypeForType_old(type);
    type = __CFNumberGetCanonicalTypeForType_old(type);
    if (!valuePtr) valuePtr = &localMemory;

    numType = __CFNumberGetType_old(number);
    __CFNumberGetValue_old((__CFNumberValue_old *)&(number->value), numType, type, valuePtr);

    // If the types match, then we're fine!
    if (numType == storageTypeForType) return true;

    // Test to see if the returned value is intact...
    SET_VALUE(&localValue, type, valuePtr);
    return __CFNumberEqualValue_old(&localValue, storageTypeForType, &(number->value), numType);
}

static CFComparisonResult CFNumberCompare_old(struct __CFNumber_old * number1, struct __CFNumber_old * number2, void *context) {
    CFNumberType type1, type2;


    type1 = __CFNumberGetType_old(number1);
    type2 = __CFNumberGetType_old(number2);

    if (__CFNumberTypeIsFloat_old(type1) || __CFNumberTypeIsFloat_old(type2)) {
        Float64 d1, d2;
        double s1, s2;
        __CFNumberGetValue_old(&(number1->value), type1, kCFNumberFloat64Type, &d1);
        __CFNumberGetValue_old(&(number2->value), type2, kCFNumberFloat64Type, &d2);
        s1 = copysign(1.0, d1);
        s2 = copysign(1.0, d2);
        if (isnan(d1) && isnan(d2)) return kCFCompareEqualTo;
        if (isnan(d1)) return (s2 < 0.0) ? kCFCompareGreaterThan : kCFCompareLessThan;
        if (isnan(d2)) return (s1 < 0.0) ? kCFCompareLessThan : kCFCompareGreaterThan;
        // at this point, we know we don't have any NaNs
        if (s1 < s2) return kCFCompareLessThan;
        if (s2 < s1) return kCFCompareGreaterThan;
        // at this point, we know the signs are the same; do not combine these tests
        if (d1 < d2) return kCFCompareLessThan;
        if (d2 < d1) return kCFCompareGreaterThan;
        return kCFCompareEqualTo;
    } else {
        int64_t i1, i2;
        __CFNumberGetValue_old(&(number1->value), type1, kCFNumberSInt64Type, &i1);
        __CFNumberGetValue_old(&(number2->value), type2, kCFNumberSInt64Type, &i2);
        return (i1 > i2) ? kCFCompareGreaterThan : ((i1 < i2) ? kCFCompareLessThan : kCFCompareEqualTo);
    }
}

#endif


#undef __CFAssertIsBoolean
#undef __CFAssertIsNumber
#undef __CFAssertIsValidNumberType
#undef BITSFORDOUBLENAN
#undef BITSFORDOUBLEPOSINF
#undef BITSFORDOUBLENEGINF
#undef MinCachedInt
#undef MaxCachedInt
#undef NotToBeCached

