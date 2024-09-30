/*
 * Copyright (C) 2022-2024 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#import <CoreGraphics/CGWindow.h>
#import <Onyx2D/O2Geometry.h>
#import <Onyx2D/O2Path.h>

@class CAWindowOpenGLContext;

@interface WSWindow : CGWindow {
    int _level;
    int _number;
    CGLContextObj _cglContext;
    CAWindowOpenGLContext *_caContext;
    NSDisplay *_display;
    NSString *_title;

    id _delegate;
    CGSBackingStoreType _backingType;
    O2Context *_context;

    NSMutableDictionary *_deviceDictionary;
    O2Rect _frame;
    unsigned _styleMask;
    BOOL _mapped;
    BOOL _ready;

    NSString *shmPath;
    NSString *bundleID;
    void *buffer;
    int bufsize;
}

- initWithFrame:(NSRect)frame styleMask:(unsigned)styleMask
        isPanel:(BOOL)isPanel backingType:(NSUInteger)backingType
        windowNumber:(int)number;
- (O2Rect)frame;
- (NSPoint)transformPoint:(NSPoint)pos;
- (O2Rect)transformFrame:(O2Rect)frame;
- (BOOL)setProperty:(NSString *)property toValue:(NSString *)value;
- (void)setExclusiveZone:(uint32_t)pixels;
- (O2Context *) createCGContextIfNeeded;
- (void) frameChanged;
- (void) setReady:(BOOL)ready;
- (BOOL) isReady;

@end

void CGNativeBorderFrameWidthsForStyle(unsigned styleMask, CGFloat *top, CGFloat *left,
                                       CGFloat *bottom, CGFloat *right);
