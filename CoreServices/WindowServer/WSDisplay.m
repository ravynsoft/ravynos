/*
 * Copyright (C) 2024 Zoe Knox <zoe@pixin.net>
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

#import <Foundation/NSRaise.h>
#import <CoreGraphics/CGImage.h>
#import "WSDisplay.h"
#import "rpc.h" // for flags constants

@implementation WSDisplay
-init {
    self = [super init];
    _flags = 0;
    _openGLMask = 0x1;
    _captured = 0;
    width = 0;
    height = 0;
    depth = 0;
    cs = NULL;
    captureCtx = NULL;
    activeCtx = NULL;
    shmid = 0;
    shmSize = 0;
    _mirrorOf = nil;
    _rotation = 0;
    _primaryDisplay = nil;
    return self;
}

-(uint32_t)getDisplayID {
    return _ID;
}

-(BOOL)isActive {
    return (_flags & kWSDisplayActive);
}

-(BOOL)isOnline {
    return (_flags & kWSDisplayOnline);
}

-(BOOL)isSleeping {
    return (_flags & kWSDisplaySleeping);
}

-(BOOL)isMain {
    return (_flags & kWSDisplayMain);
}

-(uint32_t)flags {
    return _flags;
}

-(uint32_t)openGLMask {
    return _openGLMask;
}

-(CGRect)geometry {
    return NSMakeRect(0, 0, width, height);
}

- (int)format {
    switch(depth) {
        case 32:
            return kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedFirst;
        case 24:
            return kCGBitmapByteOrderDefault | kCGImageAlphaNone;
        default:
            return kCGBitmapByteOrderDefault | kCGImageAlphaNone;
    }
}

- (int)getDepth {
    return depth;
}

- (CGColorSpaceRef)colorSpace {
    return cs;
}

// NOTE: implement in backend subclass
-(void)clear {
    NSUnimplementedMethod();
}

-(void)draw {
    NSUnimplementedMethod();
}

-(void)drawWithCursor:(O2Image *)cursor inRect:(O2Rect)rect {
    NSUnimplementedMethod();
}

-(O2Context *)context {
    if(_captured != 0)
        return captureCtx;
    return activeCtx;
}

-(pid_t)captured {
    return _captured;
}

-(BOOL)capture:(pid_t)pid withOptions:(uint32_t)options {
    if(_captured != 0)
        return NO;
    _captured = pid;
    // we ignore the deprecated options and always fill with black
    [self clear];
    return YES;
}

-(void)releaseCapture {
    _captured = 0;
    [self draw];
}

-(O2Context *)getCapturedContext {
    if(!_captured)
        return nil;
    return [self context];
}

-(uint32_t)getCapturedContextID {
    if(!_captured)
        return 0;
    return shmid;
}

-(O2ImageRef)imageForRect:(O2Rect)rect {
    return O2ImageCreateWithImageInRect([[self context] surface], rect);
}

-(float)rotation {
    return _rotation;
}

// positive is clockwise, negative is counter-clockwise
-(BOOL)rotate:(float)degrees {
    _rotation += degrees;
}

-(WSDisplay *)mirrorOf {
    return _mirrorOf;
}

// pass in nil to turn off mirroring
-(BOOL)mirror:(WSDisplay *)display {
    if(display)
        _flags |= kWSDisplayMirrored;
    else
        _flags &= ~kWSDisplayMirrored;
    _mirrorOf = display;
    return (_mirrorOf == nil) ? NO : YES;
}

-(uint32_t)modelNumber {
    return 0; // implement in backend class
}

-(uint32_t)vendorNumber {
    return 0; // implement in backend class
}

-(uint32_t)serialNumber {
    return 0; // implement in backend class
}

-(WSDisplay *)primaryDisplay {
    if(_flags & kWSDisplayHWMirror)
        if(!(_flags & kWSDisplayPrimary))
            return _primaryDisplay;
    return self;
}

// override in backend class with EDID if available
-(CGSize)screenSizeMM {
   return NSMakeSize(2.835*width, 2.835*height);
   // https://developer.apple.com/documentation/coregraphics/1456599-cgdisplayscreensize?language=objc
}

// override in backend class
-(void)restorePermanentConfig {
    return;
}

-(BOOL)setMode:(struct CGDisplayMode *)mode {
    // FIXME: implement this
    return YES;
}

-(BOOL)setOriginX:(int32_t)x Y:(int32_t)y {
    // FIXME: implement this
}

-(void)saveAppConfig {
    NSLog(@"%@ save app config", self);
}

-(void)saveSessionConfig {
    NSLog(@"%@ save session config", self);
}

-(void)savePermanentConfig {
    NSLog(@"%@ save permanent config", self);
}

@end
