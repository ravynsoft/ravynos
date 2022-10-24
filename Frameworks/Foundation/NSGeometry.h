/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <CoreGraphics/CGGeometry.h>

typedef CGPoint NSPoint;
typedef CGSize NSSize;
typedef CGRect NSRect;

typedef NSPoint *NSPointArray;
typedef NSPoint *NSPointPointer;

typedef NSSize *NSSizeArray;
typedef NSSize *NSSizePointer;

typedef enum {
    NSMinXEdge,
    NSMinYEdge,
    NSMaxXEdge,
    NSMaxYEdge
} NSRectEdge;

typedef NSRect *NSRectArray;
typedef NSRect *NSRectPointer;

//
FOUNDATION_EXPORT const NSPoint NSZeroPoint;

static inline NSPoint NSMakePoint(CGFloat x, CGFloat y) {
    NSPoint point = {x, y};
    return point;
}

FOUNDATION_EXPORT BOOL NSEqualPoints(NSPoint point0, NSPoint point1);
FOUNDATION_EXPORT NSString *NSStringFromPoint(NSPoint point);
FOUNDATION_EXPORT NSPoint NSPointFromString(NSString *string);

//
FOUNDATION_EXPORT const NSSize NSZeroSize;

static inline NSSize NSMakeSize(CGFloat w, CGFloat h) {
    NSSize size = {w, h};
    return size;
}

FOUNDATION_EXPORT BOOL NSEqualSizes(NSSize size0, NSSize size1);
FOUNDATION_EXPORT NSString *NSStringFromSize(NSSize size);
FOUNDATION_EXPORT NSSize NSSizeFromString(NSString *string);

//
FOUNDATION_EXPORT const NSRect NSZeroRect;

static inline NSRect NSMakeRect(CGFloat x, CGFloat y, CGFloat w, CGFloat h) {
    NSRect rect = {{x, y}, {w, h}};
    return rect;
}

static inline CGFloat NSMinX(NSRect rect) {
    return rect.origin.x;
}

static inline CGFloat NSMinY(NSRect rect) {
    return rect.origin.y;
}

static inline CGFloat NSWidth(NSRect rect) {
    return rect.size.width;
}

static inline CGFloat NSHeight(NSRect rect) {
    return rect.size.height;
}

static inline CGFloat NSMaxX(NSRect rect) {
    return rect.origin.x + rect.size.width;
}

static inline CGFloat NSMaxY(NSRect rect) {
    return rect.origin.y + rect.size.height;
}

static inline CGFloat NSMidX(NSRect rect) {
    return rect.origin.x + rect.size.width / 2;
}

static inline CGFloat NSMidY(NSRect rect) {
    return rect.origin.y + rect.size.height / 2;
}

FOUNDATION_EXPORT BOOL NSEqualRects(NSRect rect0, NSRect rect1);
FOUNDATION_EXPORT BOOL NSIsEmptyRect(NSRect rect);

FOUNDATION_EXPORT NSRect NSInsetRect(NSRect rect, CGFloat dx, CGFloat dy);
FOUNDATION_EXPORT NSRect NSOffsetRect(NSRect rect, CGFloat dx, CGFloat dy);

FOUNDATION_EXPORT NSRect NSIntegralRect(NSRect rect);
FOUNDATION_EXPORT NSRect NSUnionRect(NSRect rect0, NSRect rect1);
FOUNDATION_EXPORT NSRect NSIntersectionRect(NSRect rect0, NSRect rect1);

FOUNDATION_EXPORT void NSDivideRect(NSRect rect, NSRect *parts, NSRect *remainder, CGFloat amount, NSRectEdge edge);

FOUNDATION_EXPORT BOOL NSContainsRect(NSRect rect0, NSRect rect1);
FOUNDATION_EXPORT BOOL NSIntersectsRect(NSRect rect0, NSRect rect1);

FOUNDATION_EXPORT NSString *NSStringFromRect(NSRect rect);
FOUNDATION_EXPORT NSRect NSRectFromString(NSString *string);

//
FOUNDATION_EXPORT BOOL NSPointInRect(NSPoint point, NSRect rect);
FOUNDATION_EXPORT BOOL NSMouseInRect(NSPoint point, NSRect rect, BOOL flipped);

static inline CGRect NSRectToCGRect(NSRect rect) {
    return rect;
}

static inline CGSize NSSizeToCGSize(NSSize size) {
    return size;
}

static inline CGPoint NSPointToCGPoint(NSPoint point) {
    return point;
}
