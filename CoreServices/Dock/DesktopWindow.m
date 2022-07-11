/*
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
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

#import <AppKit/AppKit.h>
#import "Dock.h"
#import "DesktopWindow.h"

@implementation DesktopWindow
- initWithFrame:(NSRect)frame forOutput:(NSDictionary *)outputDict {
    _outputDict = outputDict;
    NSNumber *key = [outputDict objectForKey:@"WLOutputXDGOutput"];
    NSArray *screens = [NSScreen screens];
    _output = nil;

    for(int i = 0; i < [screens count]; ++i) {
        _output = [screens objectAtIndex:i];
        if([_output key] == key)
            break;
    }

    frame = NSZeroRect;
    frame.size = NSSizeFromString([outputDict objectForKey:@"WLOutputSize"]);
    frame.origin = NSPointFromString([outputDict objectForKey:@"WLOutputPosition"]);

    self = [self initWithContentRect:frame
        styleMask:NSBorderlessWindowMask|WLWindowLayerBackground
        backing:NSBackingStoreBuffered defer:NO screen:_output];

    frame.origin = NSZeroPoint;
    view = [[NSImageView alloc] initWithFrame:frame];
    [view setAutoresizingMask:0];
    [view setImageScaling:NSImageScaleAxesIndependently];
    [view setImageAlignment:NSImageAlignCenter];
    [_contentView addSubview:view];
    [self updateBackground];

    return self;
}

- (void)updateBackground {
    NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
    NSDictionary *dict = [prefs objectForKey:INFOKEY_WALLPAPER];
    _wallpaperPath = [dict objectForKey:[_outputDict objectForKey:@"WLOutputModel"]];
    if(!_wallpaperPath)
        _wallpaperPath = @"/System/Library/Desktop Pictures/Mountain.jpg";

    NSImage *image = [[NSImage alloc] initWithContentsOfFile:_wallpaperPath];
    [image setScalesWhenResized:YES];
    [view setImage:image];
    [view setNeedsDisplay:YES];
}

- (NSString *)wallpaperPath {
    return _wallpaperPath;
}

- (NSScreen *)screen {
    return _output;
}

- (NSDictionary *)properties {
    return _outputDict;
}


@end

