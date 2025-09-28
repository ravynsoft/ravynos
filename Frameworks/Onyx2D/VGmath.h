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

#import <math.h>
#import <Onyx2D/O2Geometry.h>
#import <Foundation/Foundation.h>

//#define RI_ASSERT(_) NSCParameterAssert(_)
#define RI_ASSERT(_)

static inline int RI_ISNAN(float a) {
    return (a != a) ? 1 : 0;
}

static inline O2Float RI_MAX(O2Float a, O2Float b) {
    return (a > b) ? a : b;
}
static inline O2Float RI_MIN(O2Float a, O2Float b) {
    return (a < b) ? a : b;
}
static inline O2Float RI_CLAMP(O2Float a, O2Float l, O2Float h) {
    if(RI_ISNAN(a))
        return l;
    RI_ASSERT(l <= h);
    return (a < l) ? l : (a > h) ? h : a;
}
static inline O2Float RI_SQR(O2Float a) {
    return a * a;
}
static inline O2Float RI_MOD(O2Float a, O2Float b) {
    if(RI_ISNAN(a) || RI_ISNAN(b))
        return 0.0f;

    RI_ASSERT(b >= 0.0f);

    if(b == 0.0f)
        return 0.0f;

    O2Float f = (O2Float)fmod(a, b);

    if(f < 0.0f)
        f += b;
    RI_ASSERT(f >= 0.0f && f <= b);
    return f;
}

static inline int RI_INT_MAX(int a, int b) {
    return (a > b) ? a : b;
}
static inline int RI_INT_MIN(int a, int b) {
    return (a < b) ? a : b;
}
static inline uint16_t RI_UINT16_MIN(uint16_t a, uint16_t b) {
    return (a < b) ? a : b;
}
static inline uint32_t RI_UINT32_MIN(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}
static inline int RI_INT_MOD(int a, int b) {
    RI_ASSERT(b >= 0);
    if(!b)
        return 0;
    int i = a % b;
    if(i < 0)
        i += b;
    RI_ASSERT(i >= 0 && i < b);
    return i;
}
static inline int RI_INT_CLAMP(int a, int l, int h) {
    RI_ASSERT(l <= h);
    return (a < l) ? l : (a > h) ? h : a;
}

static inline int RI_FLOORF_TO_INT(float value) {
    if(value < 0)
        return floorf(value);

    return value;
}

static inline int RI_FLOOR_TO_INT(double value) {
    if(value < 0)
        return floor(value);

    return value;
}

static inline O2Point Vector2Subtract(O2Point v1, O2Point v2) {
    return O2PointMake(v1.x - v2.x, v1.y - v2.y);
}

static inline O2Float Vector2Dot(O2Point v1, O2Point v2) {
    return v1.x * v2.x + v1.y * v2.y;
}
