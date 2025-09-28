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
#import <Onyx2D/O2Paint.h>

@class O2Shading;

typedef struct {
    O2Float offset;
    O2argb32f color32f;
    O2argb8u color8u;
} GradientStop;

@interface O2Paint_ramp : O2Paint {
    O2Point _startPoint;
    O2Point _endPoint;
    BOOL _extendStart;
    BOOL _extendEnd;

    size_t _numberOfColorStops;
    GradientStop *_colorStops;
}

O2argb32f O2PaintIntegrateColorRamp(O2Paint_ramp *self, O2Float gmin, O2Float gmax);
O2argb32f O2PaintColorRamp(O2Paint_ramp *self, O2Float gradient, O2Float rho, int *skip);

- initWithShading:(O2Shading *)shading deviceTransform:(O2AffineTransform)deviceTransform numberOfSamples:(int)numberOfSamples;

@end
