/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - David Young <daver@geeks.org>
#import <AppKit/AppKit.h>
#import <AppKit/NSBrowserCellColorList.h>

@implementation NSBrowserCellColorList

- (NSColor *)color { return _color; }
- (void)setColor:(NSColor *)color { color = [color retain]; [_color release]; _color = color; }

- (void)dealloc
{
    [_color release];
    [super dealloc];
}

- (void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)control
{
   NSRect   colorRect, remainderRect;
   
   NSDivideRect(frame, &colorRect, &remainderRect, NSHeight(frame), NSMinXEdge);
   [_color drawSwatchInRect:colorRect];

   [super drawInteriorWithFrame:remainderRect inView:control];
}

@end
