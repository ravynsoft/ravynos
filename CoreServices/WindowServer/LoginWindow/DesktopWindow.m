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
#import "desktop.h"

@implementation DesktopWindow
- initWithFrame:(NSRect)frame forOutput:(NSNumber *)outputKey {
    NSArray *screens = [NSScreen screens];
    NSScreen *output = nil;
    BOOL priDisplay = NO;

    for(int i = 0; i < [screens count]; ++i) {
        NSScreen *s = [screens objectAtIndex:i];
        if([s key] == outputKey) {
            output = s;
            if(i == 0)
                priDisplay = YES;
            break;
        }
    }

    frame.origin = NSZeroPoint;
    self = [super initWithContentRect:frame
        styleMask:NSBorderlessWindowMask|WLWindowLayerOverlay
        backing:NSBackingStoreBuffered defer:NO screen:output];
    _priDisplay = priDisplay;

    view = [[NSImageView alloc] initWithFrame:frame];
    [view setImageScaling:NSImageScaleAxesIndependently];
    [view setImageAlignment:NSImageAlignCenter];
    [_contentView addSubview:view];
    [self updateBackground];
    [view setAutoresizingMask:0];
    [view setNeedsDisplay:YES];

    if(_priDisplay) {
        LoginBox *box = [[LoginBox alloc] initWithDesktopWindow:self];
        [[self platformWindow] setKeyboardInteractivity:WLWindowLayerKeyboardExclusive];
        [view addSubview:box];
        [view setNextKeyView:box];
        [self makeKeyAndOrderFront:nil];
    }

    return self;
}

- (BOOL)isPrimaryDisplay {
    return _priDisplay;
}

- (void)updateBackground {
    NSString *wallpaper = [[NSBundle mainBundle] pathForResource:@"splash-1" ofType:@"png"];
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:wallpaper];
    [view setImage:image];
}

@end

