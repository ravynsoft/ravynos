/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Function.h>

@class O2ColorSpace, O2Function, O2Shading;

typedef O2Shading *O2ShadingRef;

@interface O2Shading : NSObject {
    O2ColorSpaceRef _colorSpace;
    O2Point _startPoint;
    O2Point _endPoint;
    O2Function *_function;
    BOOL _extendStart;
    BOOL _extendEnd;
    BOOL _isRadial;
    float _startRadius;
    float _endRadius;
    float _domain[2];
}

- initWithColorSpace:(O2ColorSpaceRef)colorSpace startPoint:(O2Point)startPoint endPoint:(O2Point)endPoint function:(O2Function *)function extendStart:(BOOL)extendStart extendEnd:(BOOL)extendEnd domain:(float[2])domain;

- initWithColorSpace:(O2ColorSpaceRef)colorSpace startPoint:(O2Point)startPoint startRadius:(float)startRadius endPoint:(O2Point)endPoint endRadius:(float)endRadius function:(O2Function *)function extendStart:(BOOL)extendStart extendEnd:(BOOL)extendEnd domain:(float[2])domain;

O2ColorSpaceRef O2ShadingColorSpace(O2Shading *self);

- (O2Point)startPoint;
- (O2Point)endPoint;

- (float)startRadius;
- (float)endRadius;

- (BOOL)extendStart;
- (BOOL)extendEnd;

- (O2Function *)function;

- (BOOL)isAxial;

O2ShadingRef O2ShadingCreateAxial(O2ColorSpaceRef colorSpace, O2Point start, O2Point end, O2FunctionRef function, BOOL extendStart, BOOL extendEnd);
O2ShadingRef O2ShadingCreateRadial(O2ColorSpaceRef colorSpace, O2Point start, O2Float startRadius, O2Point end, O2Float endRadius, O2FunctionRef function, BOOL extendStart, BOOL extendEnd);
O2ShadingRef O2ShadingRetain(O2ShadingRef self);
void O2ShadingRelease(O2ShadingRef self);

@end
