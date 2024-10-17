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

#import <sys/types.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <Onyx2D/O2Context_builtin.h>
#import <CoreGraphics/CGColorSpace.h>

@interface WSDisplay : NSObject {
    uint32_t _ID;               // CGDirectDisplayID
    uint32_t _flags;            // status flags
    uint32_t _openGLMask;       // CGDisplayOpenGLDisplayMask
    pid_t _captured;            // set to owning pid when captured
    unsigned int shmid;         // ID of shared mem for context
    unsigned int shmSize;

    int width;
    int height;
    int depth;
    CGColorSpaceRef cs;
    O2Context *activeCtx;
    O2Context *captureCtx;

    WSDisplay *_mirrorOf;       // display this display mirrors
    float _rotation;
    WSDisplay *_primaryDisplay; // primary of HW mirror set

    struct CGDisplayMode *_currentMode;
    CFMutableArrayRef _allModes;
}

-(uint32_t)getDisplayID;
-(BOOL)isActive;
-(BOOL)isOnline;
-(BOOL)isSleeping;
-(BOOL)isMain;
-(uint32_t)flags;
-(uint32_t)openGLMask;
-(pid_t)captured;
-(BOOL)capture:(pid_t)pid withOptions:(uint32_t)options;
-(void)releaseCapture;

// to implement in subclasses
-(CGRect)geometry;
-(void)clear;
-(void)draw;
-(void)drawWithCursor:(O2Image *)cursor inRect:(O2Rect)rect;
-(int)getDepth;
-(int)format;
-(O2Context *)context;
-(CGColorSpaceRef)colorSpace;
-(O2Context *)getCapturedContext;
-(uint32_t)getCapturedContextID;
-(O2ImageRef)imageForRect:(O2Rect)rect;

-(float)rotation;
-(BOOL)rotate:(float)degrees;
-(WSDisplay *)mirrorOf;
-(uint32_t)vendorNumber;
-(uint32_t)modelNumber;
-(uint32_t)serialNumber;
-(WSDisplay *)primaryDisplay;
-(CGSize)screenSizeMM;
-(BOOL)mirror:(WSDisplay *)display;
-(BOOL)setMode:(struct CGDisplayMode *)mode;
-(BOOL)setOriginX:(int32_t)x Y:(int32_t)y;

-(void)savePermanentConfig;
-(void)saveSessionConfig;
-(void)savePermanentConfig;
-(void)restorePermanentConfig;
-(struct CGDisplayMode *)currentMode;
-(CFArrayRef)allModes;
-(BOOL)setMode:(struct CGDisplayMode *)mode;

-(BOOL)loadGammaTable:(float *)redTable green:(float *)greenTable blue:(float *)blueTable;
-(BOOL)load8BitGammaTable:(uint8_t *)redTable green:(uint8_t *)greenTable blue:(uint8_t *)blueTable;
-(BOOL)getGammaTablesWithCapacity:(size_t)capacity red:(float *)redTable green:(float *)greenTable blue:(float *)blueTable;
-(void)loadDefaultGamma;
-(size_t)gammaTableSize;
-(void)getGammaCoefficientRed:(float *)red green:(float *)green blue:(float *)blue;

@end

