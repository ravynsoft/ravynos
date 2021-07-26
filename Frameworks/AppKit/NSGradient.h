/* Copyright (c) 2008 Sijmen Mulder

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSColor, NSMutableArray, NSArray, NSColorSpace, NSBezierPath;

typedef enum {
    NSGradientDrawsBeforeStartingLocation = (1 << 0),
    NSGradientDrawsAfterEndingLocation = (1 << 1),
} NSGradientDrawingOptions;

@interface NSGradient : NSObject {
    NSColorSpace *_colorSpace;
    NSInteger _numberOfColors;
    NSInteger _numberOfComponents;
    CGFloat **_components;
    CGFloat *_locations;
}

- initWithColors:(NSArray *)colors atLocations:(const CGFloat *)locations colorSpace:(NSColorSpace *)colorSpace;
- initWithStartingColor:(NSColor *)startingColor endingColor:(NSColor *)endingColor;
- initWithColors:(NSArray *)colors;
- initWithColorsAndLocations:(NSColor *)firstColor, ...;

- (void)drawFromPoint:(NSPoint)startingPoint toPoint:(NSPoint)endingPoint options:(NSGradientDrawingOptions)options;
- (void)drawFromCenter:(NSPoint)startCenter radius:(CGFloat)startRadius toCenter:(NSPoint)endCenter radius:(CGFloat)endRadius options:(NSGradientDrawingOptions)options;

- (void)drawInRect:(NSRect)rect angle:(CGFloat)angle;
- (void)drawInBezierPath:(NSBezierPath *)path angle:(CGFloat)angle;

- (void)drawInRect:(NSRect)rect relativeCenterPosition:(NSPoint)center;
- (void)drawInBezierPath:(NSBezierPath *)path relativeCenterPosition:(NSPoint)center;

- (NSColorSpace *)colorSpace;
- (NSInteger)numberOfColorStops;
- (void)getColor:(NSColor **)color location:(CGFloat *)location atIndex:(NSInteger)index;

- (NSColor *)interpolatedColorAtLocation:(CGFloat)location;

@end
