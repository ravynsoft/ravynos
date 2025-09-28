/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Path.h>
#import <Onyx2D/O2Geometry.h>

#ifdef __cplusplus
extern "C" {
#endif

@interface O2MutablePath : O2Path <NSCopying> {
    unsigned _capacityOfElements;
    unsigned _capacityOfPoints;
}

void O2PathReset(O2MutablePathRef self);

O2MutablePathRef O2PathCreateMutable(void);
void O2PathMoveToPoint(O2MutablePathRef self, const O2AffineTransform *matrix, O2Float x, O2Float y);
void O2PathAddLineToPoint(O2MutablePathRef self, const O2AffineTransform *matrix, O2Float x, O2Float y);
void O2PathAddCurveToPoint(O2MutablePathRef self, const O2AffineTransform *matrix, O2Float cp1x, O2Float cp1y, O2Float cp2x, O2Float cp2y, O2Float x, O2Float y);
void O2PathAddQuadCurveToPoint(O2MutablePathRef self, const O2AffineTransform *matrix, O2Float cpx, O2Float cpy, O2Float x, O2Float y);
void O2PathCloseSubpath(O2MutablePathRef self);
void O2PathAddLines(O2MutablePathRef self, const O2AffineTransform *matrix, const O2Point *points, size_t count);
void O2PathAddRect(O2MutablePathRef self, const O2AffineTransform *matrix, O2Rect rect);
void O2PathAddRects(O2MutablePathRef self, const O2AffineTransform *matrix, const O2Rect *rects, size_t count);
void O2PathAddArc(O2MutablePathRef self, const O2AffineTransform *matrix, O2Float x, O2Float y, O2Float radius, O2Float startRadian, O2Float endRadian, BOOL clockwise);
void O2PathAddArcToPoint(O2MutablePathRef self, const O2AffineTransform *matrix, O2Float tx1, O2Float ty1, O2Float tx2, O2Float ty2, O2Float radius);
void O2PathAddEllipseInRect(O2MutablePathRef self, const O2AffineTransform *matrix, O2Rect rect);
void O2PathAddPath(O2MutablePathRef self, const O2AffineTransform *matrix, O2PathRef other);

void O2PathApplyTransform(O2MutablePathRef self, const O2AffineTransform matrix);
void O2MutablePathEllipseToBezier(O2Point *cp, float x, float y, float xrad, float yrad);

@end

#ifdef __cplusplus
}
#endif
