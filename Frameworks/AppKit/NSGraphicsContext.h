/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <ApplicationServices/ApplicationServices.h>
#import <AppKit/NSGraphics.h>

@class CIContext;
@class NSWindow, NSBitmapImageRep;

typedef enum {
    NSImageInterpolationDefault = kCGInterpolationDefault,
    NSImageInterpolationNone = kCGInterpolationNone,
    NSImageInterpolationLow = kCGInterpolationLow,
    NSImageInterpolationHigh = kCGInterpolationHigh,
} NSImageInterpolation;

typedef enum {
    NSColorRenderingIntentDefault = kCGRenderingIntentDefault,
    NSColorRenderingIntentAbsoluteColorimetric = kCGRenderingIntentAbsoluteColorimetric,
    NSColorRenderingIntentRelativeColorimetric = kCGRenderingIntentRelativeColorimetric,
    NSColorRenderingIntentPerceptual = kCGRenderingIntentPerceptual,
    NSColorRenderingIntentSaturation = kCGRenderingIntentSaturation,
} NSColorRenderingIntent;

@interface NSGraphicsContext : NSObject {
    CGContextRef _graphicsPort;
    CIContext *_ciContext;
    NSMutableArray *_focusStack;
    BOOL _isDrawingToScreen;
    BOOL _isFlipped;
    NSDictionary *_deviceDescription;

    BOOL _shouldAntialias;
    NSColorRenderingIntent _renderingIntent;
    NSCompositingOperation _compOperation;
    NSPoint _patternPhase;
}

+ (NSGraphicsContext *)graphicsContextWithWindow:(NSWindow *)window;
+ (NSGraphicsContext *)graphicsContextWithGraphicsPort:(CGContextRef)context flipped:(BOOL)flipped;
+ (NSGraphicsContext *)graphicsContextWithBitmapImageRep:(NSBitmapImageRep *)imageRep;

+ (NSGraphicsContext *)currentContext;
+ (void)setCurrentContext:(NSGraphicsContext *)context;

+ (void)saveGraphicsState;
+ (void)restoreGraphicsState;

+ (BOOL)currentContextDrawingToScreen;

- (CGContextRef)graphicsPort;
- (NSMutableArray *)focusStack;

- (BOOL)isDrawingToScreen;
- (BOOL)isFlipped;

- (void)setShouldAntialias:(BOOL)value;
- (void)setImageInterpolation:(NSImageInterpolation)value;
- (void)setColorRenderingIntent:(NSColorRenderingIntent)value;
- (void)setCompositingOperation:(NSCompositingOperation)value;
- (void)setPatternPhase:(NSPoint)phase;

- (BOOL)shouldAntialias;
- (NSImageInterpolation)imageInterpolation;
- (NSColorRenderingIntent)colorRenderingIntent;
- (NSCompositingOperation)compositingOperation;
- (NSPoint)patternPhase;

- (CIContext *)CIContext;

- (void)saveGraphicsState;
- (void)restoreGraphicsState;

- (void)flushGraphics;

@end

@interface NSGraphicsContext (QuartzDebugging)

+ (void)setQuartzDebuggingEnabled:(BOOL)enabled;

+ (BOOL)quartzDebuggingIsEnabled;

+ (BOOL)inQuartzDebugMode;

+ (void)setQuartzDebugMode:(BOOL)mode;

@end
