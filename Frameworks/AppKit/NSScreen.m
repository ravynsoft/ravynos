/*
 * Copyright (c) 2006-2007 Christopher J. W. Lloyd
 * Copyright (C) 2022-2025 Zoe Knox <zoe@ravynsoft.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/NSScreen.h>
#import <AppKit/NSDisplay.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow.h>


NSString * const NSScreenColorSpaceDidChangeNotification = @"NSScreenColorSpaceDidChangeNotification";

@implementation NSScreen

-initWithFrame:(NSRect)frame visibleFrame:(NSRect)visibleFrame {
    _frame=frame;
    _visibleFrame=visibleFrame;
    _backingScaleFactor = 1.0;
    _maximumPotentialExtendedDynamicRangeColorComponentsValue = 1.0;
    _maximumExtendedDynamicRangeColorComponentValue = 1.0;
    _maximumReferenceExtendedDynamicRangeColorComponentValue = 0;
   return self;
}

// private API for NSDisplay
-(void)_propertiesFromMode:(CGDisplayModeRef)mode colorSpace:(CGColorSpaceRef)cs displayID:(CGDirectDisplayID)displayID {
    _colorSpace = [[[NSColorSpace alloc] initWithCGColorSpace:cs] retain];
    int type = CGColorSpaceGetModel(cs);
    int bps = 8; // I think component size is always 8
    _depth = (type << 8) | bps; // we always use uncalibrated RGBA for now
    unsigned int *p = malloc(sizeof(int)*2);
    p[0] = _depth;
    p[1] = 0;
    _supportedWindowDepths = p;

    _deviceDescription = [[NSDictionary
        dictionaryWithObjects:@[
            [NSNumber numberWithBool:YES], [NSValue valueWithSize:_frame.size],
            [NSValue valueWithSize:NSMakeSize(100, 100)], // FIXME: fixed to 100dpi
            NSColorSpaceFromDepth(_depth), [NSNumber numberWithInt:bps],
            [NSNumber numberWithInt:displayID]
        ] forKeys:@[
            @"NSDeviceIsScreen", @"NSDeviceSize", @"NSDeviceResolution",
            @"NSDeviceColorSpaceName", @"NSDeviceBitsPerSample", @"NSScreenNumber"
        ]] retain];
    _localizedName = @"Display name not available"; // FIXME think we need EDID for this
}

+(NSScreen *)mainScreen {
   NSScreen *result=[[NSApp keyWindow] screen];

   if(result==nil)
    result=[[self screens] objectAtIndex:0];

   return result;
}

+(NSScreen *)deepestScreen {
    NSArray *screens = [self screens];
    
    // This function must always return a screen
    if(screens == nil || [screens count] == 0)
        return [[NSScreen alloc] initWithFrame:NSZeroRect visibleFrame:NSZeroRect];

    NSScreen *result = [screens objectAtIndex:0];
    for(size_t i = 1; i < [screens count]; i++) {
        NSScreen *s = [screens objectAtIndex:i];
        int bpp = NSBitsPerPixelFromDepth([s depth]);
        if(bpp > NSBitsPerPixelFromDepth([result depth]))
            result = s;
    }

    return result;
}

+(NSArray *)screens {
   return [[NSDisplay currentDisplay] screens];
}

+(BOOL)screensHaveSeparateSpaces {
    return NO; // FIXME: implement spaces
}

-(void)dealloc {
    [_deviceDescription release];
    [_colorSpace release];
    free(_supportedWindowDepths);
}

// FIXME: remove - deprecated from 10.6
-(CGFloat)userSpaceScaleFactor {
   return 1.0;
}

-(BOOL)canRepresentDisplayGamut:(NSDisplayGamut)displayGamut {
   return YES; // FIXME
}

-(NSRect)backingAlignedRect:(NSRect)rect options:(NSAlignmentOptions)options {
    return rect; // FIXME
}

-(NSRect)convertRectFromBacking:(NSRect)rect {
    return rect; // FIXME
}

-(NSRect)convertRectToBacking:(NSRect)rect {
    return rect; // FIXME
}

-(id)description {
   return [NSString stringWithFormat:@"< %@ - frame %@, visible %@>", [super description], NSStringFromRect(_frame), NSStringFromRect(_visibleFrame)];
}

@end
