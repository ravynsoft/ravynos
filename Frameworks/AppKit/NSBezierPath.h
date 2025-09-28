/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>
#import <AppKit/NSFont.h>

@class NSAffineTransform;

typedef enum {
    NSMoveToBezierPathElement,
    NSLineToBezierPathElement,
    NSCurveToBezierPathElement,
    NSClosePathBezierPathElement,
} NSBezierPathElement;

typedef enum {
    NSMiterLineJoinStyle,
    NSRoundLineJoinStyle,
    NSBevelLineJoinStyle,
} NSLineJoinStyle;

typedef enum {
    NSButtLineCapStyle,
    NSRoundLineCapStyle,
    NSSquareLineCapStyle,
} NSLineCapStyle;

typedef enum {
    NSNonZeroWindingRule,
    NSEvenOddWindingRule,
} NSWindingRule;

@interface NSBezierPath : NSObject <NSCopying> {
    NSUInteger _capacityOfElements;
    NSUInteger _numberOfElements;
    NSUInteger _capacityOfPoints;
    NSUInteger _numberOfPoints;
    uint8_t *_elements;
    CGPoint *_points;

    float _lineWidth;
    float _miterLimit;
    float _flatness;
    NSWindingRule _windingRule;
    NSLineCapStyle _lineCapStyle;
    NSLineJoinStyle _lineJoinStyle;
    int _dashCount;
    float *_dashes;
    float _dashPhase;
    unsigned _cachesPath : 1;
    unsigned _lineWidthIsDefault : 1;
    unsigned _miterLimitIsDefault : 1;
    unsigned _flatnessIsDefault : 1;
    unsigned _windingRuleIsDefault : 1;
    unsigned _lineCapStyleIsDefault : 1;
    unsigned _lineJoinStyleIsDefault : 1;
}

+ (NSBezierPath *)bezierPath;
+ (NSBezierPath *)bezierPathWithOvalInRect:(NSRect)rect;
+ (NSBezierPath *)bezierPathWithRect:(NSRect)rect;
+ (NSBezierPath *)bezierPathWithRoundedRect:(NSRect)rect xRadius:(CGFloat)xRadius yRadius:(CGFloat)yRadius;

+ (float)defaultLineWidth;
+ (float)defaultMiterLimit;
+ (float)defaultFlatness;
+ (NSWindingRule)defaultWindingRule;
+ (NSLineCapStyle)defaultLineCapStyle;
+ (NSLineJoinStyle)defaultLineJoinStyle;

+ (void)setDefaultLineWidth:(float)width;
+ (void)setDefaultMiterLimit:(float)limit;
+ (void)setDefaultFlatness:(float)flatness;
+ (void)setDefaultWindingRule:(NSWindingRule)rule;
+ (void)setDefaultLineCapStyle:(NSLineCapStyle)style;
+ (void)setDefaultLineJoinStyle:(NSLineJoinStyle)style;

+ (void)fillRect:(NSRect)rect;
+ (void)strokeRect:(NSRect)rect;
+ (void)strokeLineFromPoint:(NSPoint)point toPoint:(NSPoint)point;
+ (void)drawPackedGlyphs:(const char *)packed atPoint:(NSPoint)point;
+ (void)clipRect:(NSRect)rect;

- (float)lineWidth;
- (float)miterLimit;
- (float)flatness;
- (NSWindingRule)windingRule;
- (NSLineCapStyle)lineCapStyle;
- (NSLineJoinStyle)lineJoinStyle;

- (void)getLineDash:(float *)dashes count:(int *)count phase:(float *)phase;

- (BOOL)cachesBezierPath;

- (int)elementCount;
- (NSBezierPathElement)elementAtIndex:(int)index;
- (NSBezierPathElement)elementAtIndex:(int)index associatedPoints:(NSPoint *)points;

- (void)setLineWidth:(float)width;
- (void)setMiterLimit:(float)limit;
- (void)setFlatness:(float)flatness;
- (void)setWindingRule:(NSWindingRule)rule;
- (void)setLineCapStyle:(NSLineCapStyle)style;
- (void)setLineJoinStyle:(NSLineJoinStyle)style;
- (void)setLineDash:(const float *)dashes count:(int)count phase:(float)phase;
- (void)setCachesBezierPath:(BOOL)flag;

- (BOOL)isEmpty;
- (NSRect)bounds;
- (NSRect)controlPointBounds;
- (BOOL)containsPoint:(NSPoint)point;
- (NSPoint)currentPoint;

- (void)moveToPoint:(NSPoint)point;
- (void)lineToPoint:(NSPoint)point;
- (void)curveToPoint:(NSPoint)point controlPoint1:(NSPoint)cp1 controlPoint2:(NSPoint)cp2;
- (void)closePath;
- (void)relativeMoveToPoint:(NSPoint)point;
- (void)relativeLineToPoint:(NSPoint)point;
- (void)relativeCurveToPoint:(NSPoint)point controlPoint1:(NSPoint)cp1 controlPoint2:(NSPoint)cp2;

- (void)appendBezierPathWithPoints:(NSPoint *)points count:(unsigned)count;
- (void)appendBezierPathWithRect:(NSRect)rect;
- (void)appendBezierPathWithOvalInRect:(NSRect)rect;
- (void)appendBezierPathWithArcFromPoint:(NSPoint)point toPoint:(NSPoint)toPoint radius:(float)radius;
- (void)appendBezierPathWithRoundedRect:(NSRect)rect xRadius:(CGFloat)radius yRadius:(CGFloat)yRadius;
- (void)appendBezierPathWithArcWithCenter:(NSPoint)center radius:(float)radius startAngle:(float)startAngle endAngle:(float)endAngle;
- (void)appendBezierPathWithArcWithCenter:(NSPoint)center radius:(float)radius startAngle:(float)startAngle endAngle:(float)endAngle clockwise:(BOOL)clockwise;
- (void)appendBezierPathWithGlyph:(NSGlyph)glyph inFont:(NSFont *)font;
- (void)appendBezierPathWithGlyphs:(NSGlyph *)glyphs count:(unsigned)count inFont:(NSFont *)font;
- (void)appendBezierPathWithPackedGlyphs:(const char *)packed;
- (void)appendBezierPath:(NSBezierPath *)other;

- (void)transformUsingAffineTransform:(NSAffineTransform *)matrix;
- (void)removeAllPoints;
- (void)setAssociatedPoints:(NSPoint *)points atIndex:(int)index;

- (NSBezierPath *)bezierPathByFlatteningPath;
- (NSBezierPath *)bezierPathByReversingPath;

- (void)stroke;
- (void)fill;

- (void)addClip;
- (void)setClip;

@end
