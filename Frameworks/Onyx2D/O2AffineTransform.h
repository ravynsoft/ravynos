/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Geometry.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    O2Float a;
    O2Float b;
    O2Float c;
    O2Float d;
    O2Float tx;
    O2Float ty;
} O2AffineTransform;

ONYX2D_EXPORT const O2AffineTransform O2AffineTransformIdentity;

ONYX2D_EXPORT O2AffineTransform O2AffineTransformMake(O2Float a, O2Float b, O2Float c, O2Float d, O2Float tx, O2Float ty);
ONYX2D_EXPORT O2AffineTransform O2AffineTransformMakeRotation(O2Float radians);
ONYX2D_EXPORT O2AffineTransform O2AffineTransformMakeScale(O2Float scalex, O2Float scaley);
ONYX2D_EXPORT O2AffineTransform O2AffineTransformMakeTranslation(O2Float tx, O2Float ty);

ONYX2D_EXPORT O2AffineTransform O2AffineTransformConcat(O2AffineTransform xform, O2AffineTransform append);
ONYX2D_EXPORT O2AffineTransform O2AffineTransformInvert(O2AffineTransform xform);

ONYX2D_EXPORT O2AffineTransform O2AffineTransformRotate(O2AffineTransform xform, O2Float radians);
ONYX2D_EXPORT O2AffineTransform O2AffineTransformScale(O2AffineTransform xform, O2Float scalex, O2Float scaley);
ONYX2D_EXPORT O2AffineTransform O2AffineTransformTranslate(O2AffineTransform xform, O2Float tx, O2Float ty);

ONYX2D_EXPORT O2Point O2PointApplyAffineTransform(O2Point point, O2AffineTransform xform);
ONYX2D_EXPORT O2Size O2SizeApplyAffineTransform(O2Size size, O2AffineTransform xform);

#ifdef __cplusplus
}
#endif
