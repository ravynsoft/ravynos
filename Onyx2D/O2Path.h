/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>
#import <Onyx2D/O2AffineTransform.h>

#ifdef __cplusplus
extern "C" {
#endif

@class O2Path, O2MutablePath;

typedef O2Path *O2PathRef;
typedef O2MutablePath *O2MutablePathRef;

typedef enum {
    kO2PathElementMoveToPoint,
    kO2PathElementAddLineToPoint,
    kO2PathElementAddQuadCurveToPoint,
    kO2PathElementAddCurveToPoint,
    kO2PathElementCloseSubpath,
} O2PathElementType;

typedef struct {
    O2PathElementType type;
    O2Point *points;
} O2PathElement;

typedef void (*O2PathApplierFunction)(void *info, const O2PathElement *element);

@interface O2Path : NSObject <NSCopying> {
    unsigned _numberOfElements;
    unsigned char *_elements;
    unsigned _numberOfPoints;
    O2Point *_points;
}

- initWithOperators:(unsigned char *)elements numberOfElements:(unsigned)numberOfElements points:(O2Point *)points numberOfPoints:(unsigned)numberOfPoints;

// internal
id O2PathInitWithOperators(O2Path *self, unsigned char *elements, unsigned numberOfElements, O2Point *points, unsigned numberOfPoints);
unsigned O2PathNumberOfElements(O2PathRef self);
const unsigned char *O2PathElements(O2PathRef self);
unsigned O2PathNumberOfPoints(O2PathRef self);
const O2Point *O2PathPoints(O2PathRef self);

// O2 public
void O2PathRelease(O2PathRef self);
O2PathRef O2PathRetain(O2PathRef self);

BOOL O2PathEqualToPath(O2PathRef self, O2PathRef other);
O2Rect O2PathGetBoundingBox(O2PathRef self);
O2Point O2PathGetCurrentPoint(O2PathRef self);
BOOL O2PathIsEmpty(O2PathRef self);
BOOL O2PathIsRect(O2PathRef self, O2Rect *rect);
void O2PathApply(O2PathRef self, void *info, O2PathApplierFunction function);
O2MutablePathRef O2PathCreateMutableCopy(O2PathRef self);
O2PathRef O2PathCreateCopy(O2PathRef self);
BOOL O2PathContainsPoint(O2PathRef self, const O2AffineTransform *xform, O2Point point, BOOL evenOdd);

@end

#ifdef __cplusplus
}
#endif
