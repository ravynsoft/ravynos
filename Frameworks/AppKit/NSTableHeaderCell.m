/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/AppKit.h>
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSRaise.h>

@implementation NSTableHeaderCell

- (NSRect)sortIndicatorRectForBounds:(NSRect)bounds {
   NSUnimplementedMethod();
   return bounds;
}

- (void)drawSortIndicatorWithFrame:(NSRect)frame inView:(NSView *)controlView ascending:(BOOL)ascending priority:(NSInteger)priority {
   NSUnimplementedMethod();
}

- (NSRect)titleRectForBounds:(NSRect)rect {
   rect.origin.x   += 3.0;
   rect.origin.y   += 2.5;
   rect.size.width -= 6.0;
   return rect;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView {
   _controlView=controlView;
   [[controlView graphicsStyle] drawTableViewHeaderInRect:cellFrame highlighted:[self isHighlighted]];
   [[self attributedStringValue] _clipAndDrawInRect:[self titleRectForBounds:cellFrame] truncatingTail:(_lineBreakMode > NSLineBreakByClipping)];
}

@end
