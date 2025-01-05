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

/*
 * Private API used by NSDisplay to set up the properties of the screen.
 * Experiments show that 'depth' is the Bits Per Sample (component) in the lowest byte
 * and a colorspace number in the higher 24 bits. The only valid values found through
 * enumerating this 24 bits are:
 * 2025-01-05 12:14:13.821 foo[7386:982302] [0x8] bpp 0 bps 8 cs NSCalibratedBlackColorSpace
 * 2025-01-05 12:14:13.821 foo[7386:982302] [0x108] bpp 8 bps 8 cs NSCalibratedWhiteColorSpace
 * 2025-01-05 12:14:13.821 foo[7386:982302] [0x208] bpp 24 bps 8 cs NSCalibratedRGBColorSpace
 * 2025-01-05 12:14:13.821 foo[7386:982302] [0x508] bpp 0 bps 8 cs NSDeviceCMYKColorSpace
 * 2025-01-05 12:14:13.821 foo[7386:982302] [0x608] bpp 0 bps 8 cs NSDeviceRGBColorSpace
 * 2025-01-05 12:21:34.925 foo[7386:982302] [0xffffff08] bpp 0 bps 8 cs NSCustomColorSpace
 */
-(void)_propertiesFromMode:(CGDisplayModeRef)mode colorSpace:(CGColorSpaceRef)cs displayID:(CGDirectDisplayID)displayID {
    _colorSpace = [[[NSColorSpace alloc] initWithCGColorSpace:cs] retain];
    _depth = 8; // I think component size is always 8 bits
    switch(CGColorSpaceGetModel(cs)) {
        case kCGColorSpaceModelMonochrome:
            // This could be either CalibratedWhite or CalibratedBlack. Prefer
            // spaces where 1.0 = white since that's how RGB behaves.
            _depth |= 0x0100; break; // NSCalibratedWhiteColorSpace
        case kCGColorSpaceModelRGB:
            if(O2ColorSpaceIsPlatformRGB(cs))
                _depth |= 0x0200; // NSCalibratedRGBColorSpace
            else
                _depth |= 0x0600; // NSDeviceRGBColorSpace
            break;
        case kCGColorSpaceModelCMYK:
            _depth |= 0x0508; break; // NSDeviceCMYKColorSpace
        default:
            _depth |= 0xffffff00; break; // NSCustomColorSpace
    }

    unsigned int *p = malloc(sizeof(int)*2);
    p[0] = _depth;
    p[1] = 0;
    _supportedWindowDepths = p;

    _deviceDescription = [[NSDictionary
        dictionaryWithObjects:@[
            [NSNumber numberWithBool:YES], [NSValue valueWithSize:_frame.size],
            [NSValue valueWithSize:NSMakeSize(100, 100)], // FIXME: fixed to 100dpi
            NSColorSpaceFromDepth(_depth), [NSNumber numberWithInt:(_depth & 0xFF)],
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
