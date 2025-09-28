/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSStringDrawer.h>
#import <AppKit/NSRaise.h>

@implementation NSString(NSStringDrawing)

-(void)drawAtPoint:(NSPoint)point withAttributes:(NSDictionary *)attributes {
   [[NSStringDrawer sharedStringDrawer] drawString:self withAttributes:attributes atPoint:point inSize:NSZeroSize];
}

-(void)drawInRect:(NSRect)rect withAttributes:(NSDictionary *)attributes {
   [[NSStringDrawer sharedStringDrawer] drawString:self withAttributes:attributes inRect:rect];
}

-(NSSize)sizeWithAttributes:(NSDictionary *)attributes {
   return [[NSStringDrawer sharedStringDrawer] sizeOfString:self withAttributes:attributes inSize:NSZeroSize];
}

- (NSRect)boundingRectWithSize:(NSSize)size options:(NSStringDrawingOptions)options attributes:(NSDictionary *)attributes {
	NSUnimplementedMethod();
	return NSMakeRect(0,0,0,0);
}

@end

@implementation NSAttributedString(NSStringDrawing)

-(void)drawAtPoint:(NSPoint)point {
   [[NSStringDrawer sharedStringDrawer] drawAttributedString:self atPoint:point inSize:NSZeroSize];
}

-(void)drawInRect:(NSRect)rect {
   [[NSStringDrawer sharedStringDrawer] drawAttributedString:self inRect:rect];
}

-(NSSize)size {
   return [[NSStringDrawer sharedStringDrawer] sizeOfAttributedString:self inSize:NSZeroSize];
}

-(void)drawWithRect:(NSRect)rect options:(NSStringDrawingOptions)options {
   NSUnimplementedMethod();
}

@end
