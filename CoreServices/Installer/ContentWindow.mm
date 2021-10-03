/*
 * ContentWindow: utility class for drawing a titled window with a
 * logo image centered vertically on the left side and some standard
 * controls
 *
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
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

#import "ContentWindow.h"

@implementation ContentWindow
-init {
    [super init];
    _cview = nil;
    _iview = nil;
    _tview = nil;
    return self;
}

-initWithSize:(NSSize)sz bgColor:(NSColor *)color {
    NSRect rect = NSMakeRect(0,0,sz.width,sz.height);
    [[self initWithContentRect:rect
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO]
        autorelease];
    [self cascadeTopLeftFromPoint:NSMakePoint(10,10)];
    [self setBackgroundColor:color];

    [self makeKeyAndOrderFront:nil];

    rect = NSMakeRect(2,2,sz.width-2,sz.height-2);
    if(_cview)
        [_cview release];

    _cview = [[[NSView alloc] initWithFrame:rect] autorelease];
    [self setContentView:_cview];
    [_cview setNeedsDisplay:YES];
    [self display];
    return self;
}

-(void)setSideImage:(NSString *)path size:(NSSize)sz {
    NSImage *image;

    image = [[[NSImage alloc] initWithContentsOfFile:path] autorelease];
    NSRect bounds = [_cview bounds];
    int y = (bounds.size.height / 2) - (sz.height / 2);
    if(y < 0) y = 0;

    if(_iview)
        [_iview release];

    _iview = [[[NSImageView alloc]
        initWithFrame:NSMakeRect(12,y,12+(sz.width),y+(sz.height / 2))]
        autorelease];
    [_iview setImage:image];
    [_cview addSubview:_iview];
    [_iview setNeedsDisplay:YES];
    [_cview setNeedsDisplay:YES];
}

-(void)setText:(NSAttributedString *)text {
    if(_tview)
        [_tview release];

    NSRect bounds = [_cview bounds];
    NSRect imgBounds = [_iview bounds];

    int x = 32+imgBounds.size.width;
    int width = bounds.size.width - imgBounds.size.width - 54;
    int height = bounds.size.height - 64;

    NSRect rect = NSMakeRect(x,52,width,height);
    _tview = [[[NSTextView alloc] initWithFrame:rect] autorelease];

    [_tview setBackgroundColor:[NSColor whiteColor]];
    [[_tview textStorage] setAttributedString:text];

    [_cview addSubview:_tview];
    [_tview setNeedsDisplay:YES];
    [_cview setNeedsDisplay:YES];
}

@end
