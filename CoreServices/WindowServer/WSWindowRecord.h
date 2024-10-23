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

#import <CoreFoundation/CFString.h>
#import <CoreGraphics/CGFont.h>

@interface WSWindowRecord : NSObject {
    CGGlyph _titleGlyphs[128];          // AppKit can't pass anything longer
}

@property int number;                   // internal window ID
@property void *surfaceBuf;             // mmaped shared graphics memory
@property size_t bufSize;               // size of surfaceBuf (bytes)
@property O2Surface *surface;           // rendering surface
@property enum WindowState state;       // state
@property NSRect geometry;              // position and size of client window
@property NSRect frame;                 // position and size with decorations
@property NSString *title;              // titlebar string
@property NSSize titleSize;             // bounding box of glyphs
@property BOOL titleSet;                // YES if ready to display title
@property NSImage *icon;                // window icon
@property NSString *shmPath;
@property int styleMask;                // NSWindow style flags
@property(readonly) NSRect closeButtonRect;
@property(readonly) NSRect miniButtonRect;
@property(readonly) NSRect zoomButtonRect;

-(void)dealloc;
-(void)setOrigin:(NSPoint)pos;
-(void)drawFrame:(O2Context *)_context pointer:(NSPoint)pointer;
-(void)moveByX:(double)x Y:(double)y;
-(void)setGlyphs:(CGGlyph *)glyphs;

@end

