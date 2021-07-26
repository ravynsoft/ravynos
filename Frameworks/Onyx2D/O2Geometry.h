/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ONYX2D_EXPORT extern
#define ONYX2D_STATIC static
#define ONYX2D_STATIC_INLINE static inline

typedef float O2Float;

typedef NSPoint O2Point;
typedef NSSize O2Size;
typedef NSRect O2Rect;

extern const O2Rect O2RectZero;
extern const O2Point O2PointZero;
extern const O2Size O2SizeZero;

static inline O2Rect O2RectMake(O2Float x, O2Float y, O2Float width, O2Float height) {
    return NSMakeRect(x, y, width, height);
}

static inline O2Point O2PointMake(O2Float x, O2Float y) {
    return NSMakePoint(x, y);
}

static inline O2Size O2SizeMake(O2Float x, O2Float y) {
    O2Size result = {x, y};
    return result;
}

static inline O2Float O2RectGetMinX(O2Rect rect) {
    return rect.origin.x;
}

static inline O2Float O2RectGetMaxX(O2Rect rect) {
    return rect.origin.x + rect.size.width;
}

static inline O2Float O2RectGetMinY(O2Rect rect) {
    return rect.origin.y;
}

static inline O2Float O2RectGetMaxY(O2Rect rect) {
    return rect.origin.y + rect.size.height;
}

static inline BOOL O2RectContainsPoint(O2Rect rect, O2Point point) {
    return (point.x >= NSMinX(rect) && point.x <= NSMaxX(rect)) && (point.y >= NSMinY(rect) && point.y <= NSMaxY(rect));
}

static inline BOOL O2PointEqualToPoint(O2Point a, O2Point b) {
    return ((a.x == b.x) && (a.y == b.y)) ? YES : NO;
}

static inline O2Rect O2RectIntersection(O2Rect rect0, O2Rect rect1) {
    // FIX: embed code
    return NSIntersectionRect(rect0, rect1);
}

static inline O2Rect O2RectIntegral(O2Rect rect) {
    float minx = floor(rect.origin.x);
    float miny = floor(rect.origin.y);
    float maxx = ceil(O2RectGetMaxX(rect));
    float maxy = ceil(O2RectGetMaxY(rect));

    rect.origin.x = minx;
    rect.origin.y = miny;
    rect.size.width = maxx - minx;
    rect.size.height = maxy - miny;

    return rect;
}

#ifdef __cplusplus
}
#endif
