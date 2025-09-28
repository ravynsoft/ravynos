/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSClipView.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSScrollView.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSCursor.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

@implementation NSClipView

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    unsigned           flags=[keyed decodeIntForKey:@"NScvFlags"];
    
    _drawsBackground=(flags&0x04)?YES:NO;
    _backgroundColor=[[keyed decodeObjectForKey:@"NSBGColor"] retain];
    _docView=[[keyed decodeObjectForKey:@"NSDocView"] retain];
    
    if(_docView!=nil)
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(viewFrameChanged:)
													 name:NSViewFrameDidChangeNotification object:_docView];
	   [[NSNotificationCenter defaultCenter] addObserver:self
												selector:@selector(viewBoundsChanged:)
													name: NSViewBoundsDidChangeNotification object:_docView];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   return self;
}

-initWithFrame:(NSRect)frame {
   [super initWithFrame:frame];
   _backgroundColor=[[NSColor controlBackgroundColor] retain];
   _drawsBackground=YES;
   return self;
}

-(void)dealloc {
   [[NSNotificationCenter defaultCenter] removeObserver:self];
   [_backgroundColor release];
   [_docView release];
   [super dealloc];
}

-(BOOL)drawsBackground {
   return _drawsBackground;
}

-(BOOL)copiesOnScroll {
   return _copiesOnScroll;
}

-(NSColor *)backgroundColor {
   return _backgroundColor;
}

-(NSCursor *)documentCursor {
   return _documentCursor;
}

-(id)documentView {
   return _docView;
}

-(void)setDrawsBackground:(BOOL)value {
   _drawsBackground=value;
   [self setNeedsDisplay:YES];
}

-(void)setCopiesOnScroll:(BOOL)value {
   _copiesOnScroll=value;
}

-(void)setBackgroundColor:(NSColor *)color {
   color=[color copy];
   [_backgroundColor release];
   _backgroundColor=color;
}

-(void)setDocumentCursor:(NSCursor *)value {
   value=[value retain];
   [_documentCursor release];
   _documentCursor=value;
   NSUnimplementedMethod();
}

-(void)setDocumentView:(NSView *)view {
   if(_docView!=nil){
    [[NSNotificationCenter defaultCenter] removeObserver:self
      name:NSViewFrameDidChangeNotification object:_docView];
    [[NSNotificationCenter defaultCenter] removeObserver:self
      name:NSViewBoundsDidChangeNotification object:_docView];
   }

   [_docView removeFromSuperview];

   view=[view retain];
   [_docView release];
   _docView=view;   

   [self addSubview:view];

   if(_docView!=nil){
    [[NSNotificationCenter defaultCenter] addObserver:self
       selector:@selector(viewFrameChanged:)
           name:NSViewFrameDidChangeNotification object:_docView];
    [[NSNotificationCenter defaultCenter] addObserver:self
       selector:@selector(viewBoundsChanged:)
           name: NSViewBoundsDidChangeNotification object:_docView];
   }
}

-(NSRect)documentRect {
   NSUnimplementedMethod();
   return NSMakeRect(0,0,0,0);
}

-(NSRect)documentVisibleRect {
   return [self convertRect:[self bounds] toView:_docView];
}

-(NSPoint)constrainScrollPoint:(NSPoint)point {
   NSRect bounds=[self bounds];
   NSRect docFrame=[[self documentView] frame];

   if(point.y<docFrame.origin.y)
    point.y=docFrame.origin.y;

   if(point.x<docFrame.origin.x)
    point.x=docFrame.origin.x;

   if(docFrame.size.height<bounds.size.height)
    point.y=docFrame.origin.y;
   else if(point.y+bounds.size.height>NSMaxY(docFrame))
    point.y=NSMaxY(docFrame)-bounds.size.height;

   if(docFrame.size.width<bounds.size.width)
    point.x=docFrame.origin.x;
   else if(point.x+bounds.size.width>NSMaxX(docFrame))
    point.x=NSMaxX(docFrame)-bounds.size.width;

   return point;
}

-(NSPoint)_scrollPoint {
   return [self bounds].origin;
}

-(void)viewBoundsChanged:(NSNotification *)note {
   [self scrollToPoint:[self _scrollPoint]];

	// Be sure our scrollbars are in sync with the new docview bounds
	if([[self superview] isKindOfClass:[NSScrollView class]]) {
        NSScrollView *sv = (NSScrollView *)[self superview];
		[sv tile]; // tiling might be needed if autohide scrollers is enabled
		[sv reflectScrolledClipView:self];
	}
}

-(void)viewFrameChanged:(NSNotification *)note {
   [self scrollToPoint:[self _scrollPoint]];

	// Be sure our scrollbars are in sync with the new docview frame
	if([[self superview] isKindOfClass:[NSScrollView class]]) {
        NSScrollView *sv = (NSScrollView *)[self superview];
		[sv tile]; // tiling might be needed if autohide scrollers is enabled
		[sv reflectScrolledClipView:self];
	}
    
    // if the docview doesn't completely fill the clip view, we need a redraw
	// because some of our content has been revealed
    NSRect visibleRect=[self visibleRect];
    NSRect frame=[_docView frame];
    if(NSContainsRect(frame, visibleRect) == NO) {
		[self setNeedsDisplay:YES];
	}
}

-(BOOL)isOpaque {
    return _drawsBackground && [_backgroundColor alphaComponent] >= 1.;
}

-(BOOL)isFlipped {
   return [_docView isFlipped];
}

-(void)drawRect:(NSRect)rect {
   if([_docView isOpaque]){
    NSRect frame=[_docView frame];

    // if the docview completely fills the drawing rect, don't draw the background
    if(NSContainsRect(frame, rect))
     return;
   }

   if([self drawsBackground]){
    [_backgroundColor setFill];
    NSRectFill(rect);
   }

}

-(BOOL)autoscroll:(NSEvent *)event {
   NSRect  bounds=[self bounds];
   NSPoint point=[self convertPoint:[event locationInWindow] fromView:nil];
   int     deltax=0,deltay=0;
   NSView *superview=[self superview];

   if(NSMouseInRect(point,bounds,[self isFlipped]))
    return NO;

   if(![superview isKindOfClass:[NSScrollView class]] || 
       [(NSScrollView *)[self superview] hasVerticalScroller]){
    if(point.y<NSMinY(bounds))
     deltay=NSMinY(bounds)-point.y;
    else if(point.y>NSMaxY(bounds))
     deltay=NSMaxY(bounds)-point.y;
    if(deltay<-bounds.size.height)
     deltay=-bounds.size.height;
    if(deltay>bounds.size.height)
     deltay=bounds.size.height;
   }
   if(![superview isKindOfClass:[NSScrollView class]] || 
       [(NSScrollView *)[self superview] hasHorizontalScroller]){
    if(point.x<NSMinX(bounds))
     deltax=NSMinX(bounds)-point.x;
    else if(point.x>NSMaxX(bounds))
     deltax=NSMaxX(bounds)-point.x;
    if(deltax<-bounds.size.width)
     deltax=-bounds.size.width;
    if(deltax>bounds.size.width)
     deltax=bounds.size.width;
   }

  // "Returns YES if any scrolling is performed; otherwise returns NO." - AppKit documentation
	if (deltax != 0.f || deltay != 0.f) {
		bounds.origin.y-=deltay;
		bounds.origin.x-=deltax;
		[self scrollToPoint:bounds.origin];
		// Return YES only if some scrolling really happened
		return NSEqualPoints(bounds.origin, _bounds.origin) == NO;
	} else {
		return NO;
	}
}

-(void)scrollToPoint:(NSPoint)point {   
   point=[self constrainScrollPoint:point];
	// Not need for more work and a full redislay if we don't really scroll
	if (!NSEqualPoints(point, _bounds.origin)) {
		[self setBoundsOrigin:point];
		[self setNeedsDisplay:YES];

		if([[self superview] isKindOfClass:[NSScrollView class]])
			[[self superview] reflectScrolledClipView:self];
	}
}

@end
