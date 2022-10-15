/*	CFNumber.c
	Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Ali Ozer
*/

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFNumber.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/CFNumber_Private.h>
#include <math.h>
#include <float.h>
#include <assert.h>


typedef CF_ENUM(uint8_t, _CFNumberCanonicalTypeIndex) {
    kCFNumberSInt8CanonicalTypeIndex   = 0x0,
    kCFNumberSInt16CanonicalTypeIndex  = 0x1,
    kCFNumberSInt32CanonicalTypeIndex  = 0x2,
    kCFNumberSInt64CanonicalTypeIndex  = 0x3,
    kCFNumberFloat32CanonicalTypeIndex = 0x4,
    kCFNumberFloat64CanonicalTypeIndex = 0x5,
    kCFNumberSInt128CanonicalTypeIndex = 0x6,
};

static const CFNumberType __CFNumberCanonicalTypes[] = {
    [kCFNumberSInt8CanonicalTypeIndex] = kCFNumberSInt8Type,
    [kCFNumberSInt16CanonicalTypeIndex] = kCFNumberSInt16Type,
    [kCFNumberSInt32CanonicalTypeIndex] = kCFNumberSInt32Type,
    [kCFNumberSInt64CanonicalTypeIndex] = kCFNumberSInt64Type,
    [kCFNumberFloat32CanonicalTypeIndex] = kCFNumberFloat32Type,
    [kCFNumberFloat64CanonicalTypeIndex] = kCFNumberFloat64Type,
    [kCFNumberSInt128CanonicalTypeIndex] = kCFNumberSInt128Type,
};

static const uint8_t __CFNumberCanonicalTypeIndex[] = {
    [kCFNumberSInt8Type] = kCFNumberSInt8CanonicalTypeIndex,
    [kCFNumberSInt16Type] = kCFNumberSInt16CanonicalTypeIndex,
    [kCFNumberSInt32Type] = kCFNumberSInt32CanonicalTypeIndex,
    [kCFNumberSInt64Type] = kCFNumberSInt64CanonicalTypeIndex,
    [kCFNumberFloat32Type] = kCFNumberFloat32CanonicalTypeIndex,
    [kCFNumberFloat64Type] = kCFNumberFloat64CanonicalTypeIndex,
    [kCFNumberSInt128Type] = kCFNumberSInt128CanonicalTypeIndex,
};

/*
 ____ ____ ____ ____   ____ ____ ____ ____
 63                    47                32
 ____ ____ ____ ____   ____ ____ __pp fttt
 31                    15                0
 
 t: The type index of the canonical type
 f: A flag to tell if the type is a preserved type or a non preserved type (1 means preserved 0 means not preserved)
 p: Bits reserved for reinterpreting both the range of `p` and `t` as a preserved type
 _: Bits reserved for the remainder of the CFRuntime field.
 
*/

// NOTE: Only 6 bits are allowed in the runtime values!
#define __CFRuntimeGetNumberType(num) (__CFNumberCanonicalTypes[__CFRuntimeGetValue((num), 5, 0) & 0x7])
#define __CFRuntimeGetNumberPreservedType(num) (__CFRuntimeGetValue((num), 5, 0) & 0x3f)

#define __CFRuntimeSetNumberType(num, type) (__CFRuntimeSetValue((num), 5, 0, (__CFNumberCanonicalTypeIndex[type] & 0x7)))
#define __CFRuntimeSetNumberPreservedType(num, type) (__CFRuntimeSetValue((num), 5, 0, (type & 0x3f)))

#define __CFRuntimeIsPreservedNumber(num) (__CFRuntimeGetValue((num), 5, 0) & 0x8)



#if TARGET_OS_WIN32
#define isnan(A) _isnan(A)
#define isinf(A) !_finite(A)
#define copysign(A, B) _copysign(A, B)
#endif

#define __CFAssertIsBoolean(cf) __CFGenericValidateType(cf, CFBooleanGetTypeID())

struct __CFBoolean {
    CFRuntimeBase _base;
};

DECLARE_STATIC_CLASS_REF(__NSCFBoolean);

_CF_CONSTANT_OBJECT_BACKING struct __CFBoolean __kCFBooleanTrue = {
    INIT_CFRUNTIME_BASE_WITH_CLASS(__NSCFBoolean, _kCFRuntimeIDCFBoolean)
};
const CFBooleanRef kCFBooleanTrue = &__kCFBooleanTrue;

_CF_CONSTANT_OBJECT_BACKING struct __CFBoolean __kCFBooleanFalse = {
    INIT_CFRUNTIME_BASE_WITH_CLASS(__NSCFBoolean, _kCFRuntimeIDCFBoolean)
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

const CFRuntimeClass __CFBooleanClass = {
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
    return _kCFRuntimeIDCFBoolean;
}

Boolean CFBooleanGetValue(CFBooleanRef boolean) {
    CF_OBJC_FUNCDISPATCHV(CFBooleanGetTypeID(), Boolean, (NSNumber *)boolean, boolValue);
    return (boolean == kCFBooleanTrue) ? true : false;
}


/*** CFNumber ***/

#define __CFAssertIsNumber(cf) __CFGenericValidateType(cf, CFNumberGetTypeID())
#define __CFAssertIsValidNumberType(type) CFAssert2((0 < type && type <= kCFNumberMaxType) || (type == kCFNumberSInt128Type), __kCFLogAssertion, "%s(): bad CFNumber type %ld", __PRETTY_FUNCTION__, type);

/* The IEEE bit patterns... Also have:
0x7f800000		float +Inf
0x7fc00000		float NaN
0xff800000		float -Inf
*/
#define BITSFORDOUBLENAN	((uint64_t)0x7ff8000000000000ULL)
#define BITSFORDOUBLEPOSINF	((uint64_t)0x7ff0000000000000ULL)
#define BITSFORDOUBLENEGINF	((uint64_t)0xfff0000000000000ULL)

// The union dance is to avoid needing hard coded values for different floating point ABIs, technically would be safe to hardcode values, but this way is future proofed
typedef union {
    Float32 floatValue;
    int32_t bits;
} Float32Bits;

typedef union {
    Float64 floatValue;
    int64_t bits;
} Float64Bits;

typedef union {
    Float32Bits _32;
    Float64Bits _64;
} FloatBits;

static const Float32Bits floatZeroBits = {.floatValue = 0.0f};
static const Float32Bits floatOneBits = {.floatValue = 1.0f};

static const Float64Bits doubleZeroBits = {.floatValue = 0.0f};
static const Float64Bits doubleOneBits = {.floatValue = 1.0f};

#define BITSFORFLOATZERO floatZeroBits.bits
#define BITSFORFLOATONE  floatOneBits.bits

#define BITSFORDOUBLEZERO doubleZeroBits.bits
#define BITSFORDOUBLEONE  doubleOneBits.bits

#if TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD || TARGET_OS_WASI
#define FLOAT_POSITIVE_2_TO_THE_64	0x1.0p+64L
#define FLOAT_NEGATIVE_2_TO_THE_127	-0x1.0p+127L
#define FLOAT_POSITIVE_2_TO_THE_127	0x1.0p+127L
#elif TARGET_OS_WIN32
#define FLOAT_POSITIVE_2_TO_THE_64	18446744073709551616.0
#define FLOAT_NEGATIVE_2_TO_THE_127	-170141183460469231731687303715884105728.0
#define FLOAT_POSITIVE_2_TO_THE_127	170141183460469231731687303715884105728.0
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif

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
    FloatBits _bits; // need this space here for the constant objects
    /* 0 or 8 more bytes allocated here */
};

/* Seven bits in base:
    Bits 6..5: unused
    Bits 4..0: CFNumber type
*/

DECLARE_STATIC_CLASS_REF(__NSCFNumber);

// Note: The isa for these things is fixed up later
static _CF_CONSTANT_OBJECT_BACKING struct __CFNumber __kCFNumberNaN = {
    INIT_CFRUNTIME_BASE_WITH_CLASS_AND_FLAGS(__NSCFNumber, _kCFRuntimeIDCFNumber, kCFNumberFloat64CanonicalTypeIndex),
    { ._64.bits = BITSFORDOUBLENAN }
};
const CFNumberRef kCFNumberNaN = &__kCFNumberNaN;

static _CF_CONSTANT_OBJECT_BACKING struct __CFNumber __kCFNumberNegativeInfinity = {
    INIT_CFRUNTIME_BASE_WITH_CLASS_AND_FLAGS(__NSCFNumber, _kCFRuntimeIDCFNumber, kCFNumberFloat64CanonicalTypeIndex),
    { ._64.bits = BITSFORDOUBLENEGINF }
};
const CFNumberRef kCFNumberNegativeInfinity = &__kCFNumberNegativeInfinity;

static _CF_CONSTANT_OBJECT_BACKING struct __CFNumber __kCFNumberPositiveInfinity = {
    INIT_CFRUNTIME_BASE_WITH_CLASS_AND_FLAGS(__NSCFNumber, _kCFRuntimeIDCFNumber, kCFNumberFloat64CanonicalTypeIndex),
    { ._64.bits = BITSFORDOUBLEPOSINF }
};
const CFNumberRef kCFNumberPositiveInfinity = &__kCFNumberPositiveInfinity;

static _CF_CONSTANT_OBJECT_BACKING struct __CFNumber __kCFNumberFloat32Zero = {
    INIT_CFRUNTIME_BASE_WITH_CLASS_AND_FLAGS(__NSCFNumber, _kCFRuntimeIDCFNumber, kCFNumberFloat32CanonicalTypeIndex),
    { ._32.floatValue = 0.0f }
};
static const CFNumberRef kCFNumberFloat32Zero = &__kCFNumberFloat32Zero;

static _CF_CONSTANT_OBJECT_BACKING struct __CFNumber __kCFNumberFloat32One = {
    INIT_CFRUNTIME_BASE_WITH_CLASS_AND_FLAGS(__NSCFNumber, _kCFRuntimeIDCFNumber, kCFNumberFloat32CanonicalTypeIndex),
    { ._32.floatValue = 1.0f }
};
static const CFNumberRef kCFNumberFloat32One = &__kCFNumberFloat32One;

static _CF_CONSTANT_OBJECT_BACKING struct __CFNumber __kCFNumberFloat64Zero = {
    INIT_CFRUNTIME_BASE_WITH_CLASS_AND_FLAGS(__NSCFNumber, _kCFRuntimeIDCFNumber, kCFNumberFloat64CanonicalTypeIndex),
    { ._64.floatValue = 0.0f }
};
static const CFNumberRef kCFNumberFloat64Zero = &__kCFNumberFloat64Zero;

static _CF_CONSTANT_OBJECT_BACKING struct __CFNumber __kCFNumberFloat64One = {
    INIT_CFRUNTIME_BASE_WITH_CLASS_AND_FLAGS(__NSCFNumber, _kCFRuntimeIDCFNumber, kCFNumberFloat64CanonicalTypeIndex),
    { ._64.floatValue = 1.0f }
};
static const CFNumberRef kCFNumberFloat64One = &__kCFNumberFloat64One;

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
#if TARGET_RT_64_BIT
    /* kCFNumberLongType */	{kCFNumberSInt64Type, 0, 0, 3, 0},
#else
    /* kCFNumberLongType */	{kCFNumberSInt32Type, 0, 0, 2, 0},
#endif
    /* kCFNumberLongLongType */	{kCFNumberSInt64Type, 0, 0, 3, 0},
    /* kCFNumberFloatType */	{kCFNumberFloat32Type, 1, 0, 2, 0},
    /* kCFNumberDoubleType */	{kCFNumberFloat64Type, 1, 1, 3, 0},

#if TARGET_RT_64_BIT
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
    return __CFRuntimeGetNumberType(num);
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
    const void *data = &(number->_bits._64.bits);
    Boolean floatBit = __CFNumberTypeTable[ntype].floatBit;
    Boolean storageBit = __CFNumberTypeTable[ntype].storageBit;
    switch (type) {
    case kCFNumberSInt8Type:
	if (floatBit) {
	    if (!storageBit) {
		CVT(Float32, int8_t, INT8_MIN, INT8_MAX);
	    } else {
		CVT(Float64, int8_t, INT8_MIN, INT8_MAX);
	    }
	} else {
	    if (!storageBit) {
		CVT(int64_t, int8_t, INT8_MIN, INT8_MAX);
	    } else {
		CVT128ToInt(CFSInt128Struct, int8_t, INT8_MIN, INT8_MAX);
	    }
	}
	return true;
    case kCFNumberSInt16Type:
	if (floatBit) {
	    if (!storageBit) {
                CVT(Float32, int16_t, INT16_MIN, INT16_MAX);
	    } else {
                CVT(Float64, int16_t, INT16_MIN, INT16_MAX);
	    }
	} else {
	    if (!storageBit) {
                CVT(int64_t, int16_t, INT16_MIN, INT16_MAX);
	    } else {
		CVT128ToInt(CFSInt128Struct, int16_t, INT16_MIN, INT16_MAX);
	    }
	}
	return true;
    case kCFNumberSInt32Type:
	if (floatBit) {
	    if (!storageBit) {
                CVT(Float32, int32_t, INT32_MIN, INT32_MAX);
	    } else {
                CVT(Float64, int32_t, INT32_MIN, INT32_MAX);
	    }
	} else {
	    if (!storageBit) {
                CVT(int64_t, int32_t, INT32_MIN, INT32_MAX);
	    } else {
		CVT128ToInt(CFSInt128Struct, int32_t, INT32_MIN, INT32_MAX);
	    }
	}
	return true;
    case kCFNumberSInt64Type:
	if (floatBit) {
	    if (!storageBit) {
                CVT(Float32, int64_t, INT64_MIN, INT64_MAX);
	    } else {
                CVT(Float64, int64_t, INT64_MIN, INT64_MAX);
	    }
	} else {
	    if (!storageBit) {
		memmove(valuePtr, data, 8);
	    } else {
		CVT128ToInt(CFSInt128Struct, int64_t, INT64_MIN, INT64_MAX);
	    }
	}
	return true;
    case kCFNumberSInt128Type:
	if (floatBit) {
	    if (!storageBit) {
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
	    if (!storageBit) {
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
	if (floatBit) {
	    if (!storageBit) {
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
	    if (!storageBit) {
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
	if (floatBit) {
	    if (!storageBit) {
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
	    if (!storageBit) {
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
    const void *data = &(number->_bits._64.bits);
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
		// Leopard's implementation of this always returned true. We should only return true when the conversion is lossless. However, there are some clients who use CFNumber with small unsigned values disguised as signed values. Since there is no CFNumber API yet for unsigned values, we need to accommodate those clients for now. <rdar://problem/6471866>
		// This accommodation should be removed if CFNumber ever provides API for unsigned values. <rdar://problem/6473890>
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
		// Leopard's implementation of this always returned true. We should only return true when the conversion is lossless. However, there are some clients who use CFNumber with small unsigned values disguised as signed values. Since there is no CFNumber API yet for unsigned values, we need to accommodate those clients for now. <rdar://problem/6471866>
		// This accommodation should be removed if CFNumber ever provides API for unsigned values. <rdar://problem/6473890>
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
    return result;
}


static Boolean __CFNumberEqual(CFTypeRef cf1, CFTypeRef cf2) {
    Boolean b = CFNumberCompare((CFNumberRef)cf1, (CFNumberRef)cf2, 0) == kCFCompareEqualTo;
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
    return h;
}


enum {
  kCFNumberCachingEnabled = 0,
  kCFNumberCachingDisabled = 1,
  kCFNumberCachingFullyDisabled = 2
};
static char __CFNumberCaching = kCFNumberCachingEnabled;

const CFRuntimeClass __CFNumberClass = {
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
    // TODO: Move other work out of here
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{
        

        const char *caching = getenv("CFNumberDisableCache");	// "all" to disable caching and tagging; anything else to disable caching; nothing to leave both enabled
        if (caching) __CFNumberCaching = (!strcmp(caching, "all")) ? kCFNumberCachingFullyDisabled : kCFNumberCachingDisabled;	// initial state above is kCFNumberCachingEnabled
    });
    return _kCFRuntimeIDCFNumber;
}

#define MinCachedInt (-1)
#define MaxCachedInt (12)
#define NotToBeCached (MinCachedInt - 1)
static CFNumberRef __CFNumberCache[MaxCachedInt - MinCachedInt + 1] = {NULL};	// Storing CFNumberRefs for range MinCachedInt..MaxCachedInt

static inline void __CFNumberInit(CFNumberRef result, CFNumberType type, const void *valuePtr) {
    __CFAssertIsValidNumberType(type);
    
    uint64_t value;
    switch (__CFNumberTypeTable[type].canonicalType) {
        case kCFNumberSInt8Type:   value = (uint64_t)(int64_t)*(int8_t *)valuePtr; goto smallVal;
        case kCFNumberSInt16Type:  value = (uint64_t)(int64_t)*(int16_t *)valuePtr; goto smallVal;
        case kCFNumberSInt32Type:  value = (uint64_t)(int64_t)*(int32_t *)valuePtr; goto smallVal;
        smallVal: memmove((void *)&result->_bits._64.bits, &value, 8); break;
        case kCFNumberSInt64Type:  memmove((void *)&result->_bits._64.bits, valuePtr, 8); break;
        case kCFNumberSInt128Type: memmove((void *)&result->_bits._64.bits, valuePtr, 16); break;
        case kCFNumberFloat32Type: memmove((void *)&result->_bits._64.bits, valuePtr, 4); break;
        case kCFNumberFloat64Type: memmove((void *)&result->_bits._64.bits, valuePtr, 8); break;
    }
}

static inline void _CFNumberInit(CFNumberRef result, CFNumberType type, const void *valuePtr) {
    __CFNumberInit(result, type, valuePtr);
}

void _CFNumberInitBool(CFNumberRef result, Boolean value) {
    _CFNumberInit(result, kCFNumberCharType, &value);
}

void _CFNumberInitInt8(CFNumberRef result, int8_t value) {
    _CFNumberInit(result, kCFNumberCharType, &value);
}

void _CFNumberInitUInt8(CFNumberRef result, uint8_t value) {
    _CFNumberInit(result, kCFNumberCharType, &value);
}

void _CFNumberInitInt16(CFNumberRef result, int16_t value) {
    _CFNumberInit(result, kCFNumberShortType, &value);
}

void _CFNumberInitUInt16(CFNumberRef result, uint16_t value) {
    _CFNumberInit(result, kCFNumberShortType, &value);
}

void _CFNumberInitInt32(CFNumberRef result, int32_t value) {
    _CFNumberInit(result, kCFNumberIntType, &value);
}

void _CFNumberInitUInt32(CFNumberRef result, uint32_t value) {
    _CFNumberInit(result, kCFNumberIntType, &value);
}

void _CFNumberInitInt(CFNumberRef result, long value) {
    _CFNumberInit(result, kCFNumberLongType, &value);
}

void _CFNumberInitUInt(CFNumberRef result, unsigned long value) {
    _CFNumberInit(result, kCFNumberLongType, &value);
}

void _CFNumberInitInt64(CFNumberRef result, int64_t value) {
    _CFNumberInit(result, kCFNumberLongLongType, &value);
}

void _CFNumberInitUInt64(CFNumberRef result, uint64_t value) {
    _CFNumberInit(result, kCFNumberLongLongType, &value);
}

void _CFNumberInitFloat(CFNumberRef result, float value) {
    _CFNumberInit(result, kCFNumberFloatType, &value);
}

void _CFNumberInitDouble(CFNumberRef result, double value) {
    _CFNumberInit(result, kCFNumberDoubleType, &value);
}

static CFNumberRef _CFNumberCreate(CFAllocatorRef allocator, CFNumberType type, const void *valuePtr) {
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
            Float32Bits f = *(Float32Bits *)valuePtr;
            if (f.bits == BITSFORFLOATZERO) cached = kCFNumberFloat32Zero;
            else if (f.bits == BITSFORFLOATONE) cached = kCFNumberFloat32One;
            else if (isnan(f.floatValue)) cached = kCFNumberNaN;
            else if (isinf(f.floatValue)) cached = (f.floatValue < 0.0) ? kCFNumberNegativeInfinity : kCFNumberPositiveInfinity;


        } else {
            Float64Bits d = *(Float64Bits *)valuePtr;
            if (d.bits == BITSFORDOUBLEZERO) cached = kCFNumberFloat64Zero;
            else if (d.bits == BITSFORDOUBLEONE) cached = kCFNumberFloat64One;
            else if (isnan(d.floatValue)) cached = kCFNumberNaN;
            else if (isinf(d.floatValue)) cached = (d.floatValue < 0.0) ? kCFNumberNegativeInfinity : kCFNumberPositiveInfinity;
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
            CFNumberRef cached = __CFNumberCache[valToBeCached - MinCachedInt];        // Atomic to access the value in the cache
            if (NULL != cached) return (CFNumberRef)CFRetain(cached);
        }
    }

    CFIndex size = 8 + ((!__CFNumberTypeTable[type].floatBit && __CFNumberTypeTable[type].storageBit) ? 8 : 0);
    CFNumberRef result = (CFNumberRef)_CFRuntimeCreateInstance(allocator, CFNumberGetTypeID(), size, NULL);
    if (NULL == result) {
	return NULL;
    }
    
    __CFRuntimeSetNumberType(result, (uint8_t)__CFNumberTypeTable[type].canonicalType);
    
    
    // should be initialized BEFORE ever caching it!
    __CFNumberInit(result, type, valuePtr);

    // for a value to be cached, we already have the value handy
    if (NotToBeCached != valToBeCached) {
	memmove((void *)&result->_bits._64.bits, &valToBeCached, 8);
	// Put this in the cache unless the cache is already filled (by another thread).  If we do put it in the cache, retain it an extra time for the cache.
	// Note that we don't bother freeing this result and returning the cached value if the cache was filled, since cached CFNumbers are not guaranteed unique.
	// Barrier assures that the number that is placed in the cache is properly formed.
	CFNumberType origType = __CFNumberGetType(result);
	// Force all cached numbers to have the same type, so that the type does not
	// depend on the order and original type in/with which the numbers are created.
	// Forcing the type AFTER it was cached would cause a race condition with other
	// threads pulling the number object out of the cache and using it.
        __CFRuntimeSetNumberType(result, (uint8_t)kCFNumberSInt32Type);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
	if (OSAtomicCompareAndSwapPtrBarrier(NULL, (void *)result, (void *volatile *)&__CFNumberCache[valToBeCached - MinCachedInt])) {
#pragma GCC diagnostic pop
	    CFRetain(result);
	} else {
	    // Did not cache the number object, put original type back.
            __CFRuntimeSetNumberType(result, origType);
	}
	return result;
    }

    return result;
}

CFNumberRef CFNumberCreate(CFAllocatorRef allocator, CFNumberType type, const void *valuePtr) {
    return _CFNumberCreate(allocator, type, valuePtr);
}

CFNumberType CFNumberGetType(CFNumberRef number) {
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFNumber, CFNumberType, (CFSwiftRef)number, NSNumber._cfNumberGetType);
    CFNumberType type;
#if DEPLOYMENT_RUNTIME_OBJC
    if (CF_IS_OBJC(_kCFRuntimeIDCFNumber, (const void *)number)) {
        type = (CFNumberType)[(NSNumber *)number _cfNumberType];
    } else {
#endif
         __CFAssertIsNumber(number);
        type = __CFNumberGetType(number);
#if DEPLOYMENT_RUNTIME_OBJC
    }
#endif
    if (kCFNumberSInt128Type == type) type = kCFNumberSInt64Type; // must hide this type, since it is not public
    return type;
}

CF_EXPORT CFNumberType _CFNumberGetType2(CFNumberRef number) {
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFNumber, CFNumberType, (NSNumber *)number, _cfNumberType);
    __CFAssertIsNumber(number);
    return __CFNumberGetType(number);
}

CFIndex CFNumberGetByteSize(CFNumberRef number) {
    __CFAssertIsNumber(number);
    CFIndex r = 1 << __CFNumberTypeTable[CFNumberGetType(number)].lgByteSize;
    return r;
}

Boolean CFNumberIsFloatType(CFNumberRef number) {
    __CFAssertIsNumber(number);
    Boolean r = __CFNumberTypeTable[CFNumberGetType(number)].floatBit;
    return r;
}

Boolean CFNumberGetValue(CFNumberRef number, CFNumberType type, void *valuePtr) {
//printf("+ [%p] CFNumberGetValue(%p, %d, %p)\n", pthread_self(), number, type, valuePtr);

    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFNumber, Boolean, (NSNumber *)number, _getValue:(void *)valuePtr forType:(CFNumberType)__CFNumberTypeTable[type].canonicalType);
    CF_SWIFT_FUNCDISPATCHV(_kCFRuntimeIDCFNumber, Boolean, (CFSwiftRef)number, NSNumber._getValue, valuePtr, (CFNumberType)__CFNumberTypeTable[type].canonicalType);
    __CFAssertIsNumber(number);
    __CFAssertIsValidNumberType(type);
    uint8_t localMemory[128];
    Boolean r = __CFNumberGetValueCompat(number, type, valuePtr ? valuePtr : localMemory);
    return r;
}

static CFComparisonResult CFNumberCompare_new(CFNumberRef number1, CFNumberRef number2, void *context) {
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFNumber, CFComparisonResult, (NSNumber *)number1, compare:(NSNumber *)number2);
    CF_OBJC_FUNCDISPATCHV(_kCFRuntimeIDCFNumber, CFComparisonResult, (NSNumber *)number2, _reverseCompare:(NSNumber *)number1);
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
    CFComparisonResult r = CFNumberCompare_new(number1, number2, context);
    return r;
}



#undef __CFAssertIsBoolean
#undef __CFAssertIsNumber
#undef __CFAssertIsValidNumberType
#undef BITSFORDOUBLENAN
#undef BITSFORDOUBLEPOSINF
#undef BITSFORDOUBLENEGINF
#undef MinCachedInt
#undef MaxCachedInt
#undef NotToBeCached

