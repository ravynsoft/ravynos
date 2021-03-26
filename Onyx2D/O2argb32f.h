/*------------------------------------------------------------------------
 *
 * Derivative of the OpenVG 1.0.1 Reference Implementation
 * -------------------------------------
 *
 * Copyright (c) 2007 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and /or associated documentation files
 * (the "Materials "), to deal in the Materials without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Materials,
 * and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Materials. 
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR
 * THE USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 *-------------------------------------------------------------------*/

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>
#import <Onyx2D/VGmath.h>
#import <Onyx2D/O2argb8u.h>

typedef float O2Float32;

typedef struct {
    O2Float32 a;
    O2Float32 r;
    O2Float32 g;
    O2Float32 b;
} O2argb32f;

static inline O2argb32f O2argb32fInit(O2Float r, O2Float g, O2Float b, O2Float a) {
    O2argb32f result;
    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a;
    return result;
}

static inline O2argb32f O2argb32fMultiplyByFloat(O2argb32f result, O2Float value) {
    result.r *= value;
    result.g *= value;
    result.b *= value;
    result.a *= value;
    return result;
}

static inline O2argb32f O2argb32fAdd(O2argb32f result, O2argb32f other) {
    result.r += other.r;
    result.g += other.g;
    result.b += other.b;
    result.a += other.a;
    return result;
}

static inline O2argb32f O2argb32fSubtract(O2argb32f result, O2argb32f other) {
    result.r -= other.r;
    result.g -= other.g;
    result.b -= other.b;
    result.a -= other.a;
    return result;
}

static inline O2argb32f O2argb32fPremultiply(O2argb32f result) {
    result.r *= result.a;
    result.g *= result.a;
    result.b *= result.a;
    return result;
}
static inline O2argb32f O2argb32fClamp(O2argb32f result) {
    result.r = RI_CLAMP(result.r, 0.0f, 1.0f);
    result.g = RI_CLAMP(result.g, 0.0f, 1.0f);
    result.b = RI_CLAMP(result.b, 0.0f, 1.0f);
    result.a = RI_CLAMP(result.a, 0.0f, 1.0f);
    return result;
}

static inline O2Float O2Float32FromByte(uint8_t i) {
    return (O2Float)(i) / (O2Float)0xFF;
}

static inline uint8_t O2ByteFromFloat(O2Float c) {
    return RI_INT_MIN(RI_INT_MAX(RI_FLOOR_TO_INT(c * (O2Float)0xFF + 0.5f), 0), 0xFF);
}

static inline O2argb8u O2argb8uFromO2argb32f(O2argb32f rgba) {
    O2argb8u result;
    result.r = O2ByteFromFloat(rgba.r);
    result.g = O2ByteFromFloat(rgba.g);
    result.b = O2ByteFromFloat(rgba.b);
    result.a = O2ByteFromFloat(rgba.a);
    return result;
}

#define COVERAGE_MULTIPLIER_FLOAT 256.0f

static inline O2Float zeroToOneFromCoverage(unsigned coverage) {
    return (O2Float)coverage / COVERAGE_MULTIPLIER_FLOAT;
}

static inline uint32_t coverageFromZeroToOne(O2Float value) {
    return value * COVERAGE_MULTIPLIER_FLOAT;
}

void O2BlendSpanNormal_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanMultiply_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanScreen_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanOverlay_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanDarken_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanLighten_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanColorDodge_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanColorBurn_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanHardLight_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanSoftLight_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanDifference_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanExclusion_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanHue_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanSaturation_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanColor_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanLuminosity_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanClear_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanCopy_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanSourceIn_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanSourceOut_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanSourceAtop_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanDestinationOver_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanDestinationIn_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanDestinationOut_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanDestinationAtop_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanXOR_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanPlusDarker_ffff(O2argb32f *src, O2argb32f *dst, int length);
void O2BlendSpanPlusLighter_ffff(O2argb32f *src, O2argb32f *dst, int length);
