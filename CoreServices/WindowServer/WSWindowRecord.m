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

#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2Surface.h>
#import <Onyx2D/O2Image.h>
#import <Onyx2D/O2ImageSource.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSWindow.h>
#import "common.h"
#import "WindowServer.h"

// all measurements in pixels
const float WSWindowTitleHeight = 32;
const float WSWindowEdgePad = 2;
const float WSWindowCornerRadius = 8;
const float WSWindowControlDiameter = 14;
const float WSWindowControlSpacing = 10;

static O2Image *wsClose, *wsCloseHover, *wsMini, *wsMiniHover;
static O2Image *wsZoom, *wsZoomUp, *wsZoomDown;

NSRect WSOutsetFrame(NSRect rect, int style) {
    if(style & NSBorderlessWindowMask)
        return rect;

    NSRect _frame = rect;
    _frame.size.height += WSWindowTitleHeight;
    _frame.size.width += 2*WSWindowEdgePad;
    _frame.origin.x -= WSWindowEdgePad;
    _frame.origin.y -= WSWindowEdgePad;
    return _frame;
}

@implementation WSWindowRecord
-init {
    self = [super init];
    // use singleton traffic light images
    if(wsClose == nil) {
        NSBundle *bundle = [NSBundle mainBundle];
        NSString *path = [bundle pathForResource:@"close" ofType:@"png"];
        wsClose = [[[NSBitmapImageRep alloc] initWithContentsOfFile:path] CGImage];
        path = [bundle pathForResource:@"closeHover" ofType:@"png"];
        wsCloseHover = [[[NSBitmapImageRep alloc] initWithContentsOfFile:path] CGImage];
        path = [bundle pathForResource:@"mini" ofType:@"png"];
        wsMini = [[[NSBitmapImageRep alloc] initWithContentsOfFile:path] CGImage];
        path = [bundle pathForResource:@"miniHover" ofType:@"png"];
        wsMiniHover = [[[NSBitmapImageRep alloc] initWithContentsOfFile:path] CGImage];
        path = [bundle pathForResource:@"zoom" ofType:@"png"];
        wsZoom = [[[NSBitmapImageRep alloc] initWithContentsOfFile:path] CGImage];
        path = [bundle pathForResource:@"zoomUpHover" ofType:@"png"];
        wsZoomUp = [[[NSBitmapImageRep alloc] initWithContentsOfFile:path] CGImage];
        path = [bundle pathForResource:@"zoomDownHover" ofType:@"png"];
        wsZoomDown = [[[NSBitmapImageRep alloc] initWithContentsOfFile:path] CGImage];
    }
    return self;
}

-(void)dealloc {
    if(_surfaceBuf != NULL)
        munmap(_surfaceBuf, _bufSize);
    if(shm_unlink([_shmPath cString]) != 0)
        perror("shm_unlink");
}

-(void)setOrigin:(NSPoint)pos {
    _geometry.origin = pos;
    _frame = WSOutsetFrame(_geometry, _styleMask);
}

-(void)drawFrame:(O2Context *)_context pointer:(NSPoint)pointer {
    if((_styleMask & 0x0FFF) == NSBorderlessWindowMask)
        return;
    
    O2ContextSetGrayStrokeColor(_context, 0.8, 1);
    O2ContextSetGrayFillColor(_context, 0.8, 1);

    // let's round these corners
    float radius = WSWindowCornerRadius;
    O2ContextBeginPath(_context);
    O2ContextMoveToPoint(_context, _frame.origin.x+radius, NSMaxY(_frame));
    O2ContextAddArc(_context, _frame.origin.x + _frame.size.width - radius,
        _frame.origin.y + _frame.size.height - radius, radius, 1.5708 /*radians*/,
        0 /*radians*/, YES);
    O2ContextAddLineToPoint(_context, _frame.origin.x + _frame.size.width,
        _frame.origin.y);
    O2ContextAddArc(_context, _frame.origin.x + _frame.size.width - radius,
        _frame.origin.y + radius, radius, 6.28319 /*radians*/, 4.71239 /*radians*/,
        YES);
    O2ContextAddLineToPoint(_context, _frame.origin.x, _frame.origin.y);
    O2ContextAddArc(_context, _frame.origin.x + radius, _frame.origin.y + radius,
        radius, 4.71239, 3.14159, YES);
    O2ContextAddLineToPoint(_context, _frame.origin.x,
        _frame.origin.y + _frame.size.height);
    O2ContextAddArc(_context, _frame.origin.x + radius, _frame.origin.y +
        _frame.size.height - radius, radius, 3.14159, 1.5708, YES);
    O2ContextAddLineToPoint(_context, _frame.origin.x, NSMaxY(_frame));
    O2ContextClosePath(_context);
    O2ContextFillPath(_context);

    // window controls
    CGRect button = NSMakeRect(_frame.origin.x + WSWindowControlSpacing,
        _frame.origin.y + _frame.size.height - WSWindowTitleHeight / 2 - WSWindowControlDiameter / 2,
        WSWindowControlDiameter, WSWindowControlDiameter);
    _closeButtonRect = button;
    button.origin.x += WSWindowControlSpacing + WSWindowControlDiameter;
    _miniButtonRect = button;
    button.origin.x += WSWindowControlSpacing + WSWindowControlDiameter;
    _zoomButtonRect = button;

    NSRect controls = NSMakeRect(_closeButtonRect.origin.x, _closeButtonRect.origin.y,
            3*WSWindowControlSpacing + 3*WSWindowControlDiameter, WSWindowControlDiameter);

    if(NSPointInRect(pointer, controls)) {
        [_context drawImage:wsCloseHover inRect:_closeButtonRect];
        [_context drawImage:wsMiniHover inRect:_miniButtonRect];
        [_context drawImage:wsZoomUp inRect:_zoomButtonRect];
    } else {
        [_context drawImage:wsClose inRect:_closeButtonRect];
        [_context drawImage:wsMini inRect:_miniButtonRect];
        [_context drawImage:wsZoom inRect:_zoomButtonRect];
    }

    // title
    if(_title) {
        O2ContextSetRGBFillColor(_context, 0, 0, 0, 1);
        O2ContextSetRGBStrokeColor(_context, 0, 0, 0, 1);
        float x = _frame.origin.x + _frame.size.width / 2 - _titleSize.width / 2;
        float y = _frame.origin.y + _frame.size.height - WSWindowTitleHeight / 3 - _titleSize.height / 2;
        O2ContextShowGlyphsAtPoint(_context, x, y, _titleGlyphs, [_title length]);
    }
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@ 0x%x> %@ %@ title:%@ state:%u style:0x%x",
           [self class], (uint32_t)self, self.shmPath, NSStringFromRect(self.geometry),
           self.title, self.state, self.styleMask];
}

-(void)moveByX:(double)x Y:(double)y {
    _geometry.origin.x += x;
    _geometry.origin.y += y;
    _frame = WSOutsetFrame(_geometry, _styleMask);
}

-(void)setGlyphs:(CGGlyph *)glyphs {
    memset(_titleGlyphs, 0, sizeof(_titleGlyphs));
    for(int i = 0; i < [_title length]; ++i)
        _titleGlyphs[i] = glyphs[i];
}

@end

