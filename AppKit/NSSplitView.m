/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSSplitView.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSGraphicsContext.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSCursor.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

NSString * const NSSplitViewDidResizeSubviewsNotification = @"NSSplitViewDidResizeSubviewsNotification";
NSString * const NSSplitViewWillResizeSubviewsNotification = @"NSSplitViewWillResizeSubviewsNotification";

@implementation NSSplitView

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _isVertical=[keyed decodeBoolForKey:@"NSIsVertical"];
// The divider thickness in the nib may not be the same as ours
    [self resizeSubviewsWithOldSize:[self bounds].size];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }

   return self;
}

-(id)delegate {
   return _delegate;
}

-(BOOL)isVertical {
   return _isVertical;
}

-(void)_postNoteWillResize {
    [[NSNotificationCenter defaultCenter] postNotificationName:NSSplitViewWillResizeSubviewsNotification object:self];
}

-(void)_postNoteDidResize {
    [[NSNotificationCenter defaultCenter] postNotificationName:NSSplitViewDidResizeSubviewsNotification object:self];
}

-(void)setDelegate:(id)delegate {
    if ([_delegate respondsToSelector:@selector(splitViewDidResizeSubviews:)])
        [[NSNotificationCenter defaultCenter] removeObserver:_delegate name:NSSplitViewDidResizeSubviewsNotification object:self];
    if ([_delegate respondsToSelector:@selector(splitViewWillResizeSubviews:)])
        [[NSNotificationCenter defaultCenter] removeObserver:_delegate name:NSSplitViewWillResizeSubviewsNotification object:self];
    
   _delegate=delegate;

   if ([_delegate respondsToSelector:@selector(splitViewDidResizeSubviews:)])
       [[NSNotificationCenter defaultCenter] addObserver:_delegate
                                                selector:@selector(splitViewDidResizeSubviews:)
                                                    name:NSSplitViewDidResizeSubviewsNotification
                                                  object:self];
   if ([_delegate respondsToSelector:@selector(splitViewWillResizeSubviews:)])
       [[NSNotificationCenter defaultCenter] addObserver:_delegate
                                                selector:@selector(splitViewWillResizeSubviews:)
                                                    name:NSSplitViewWillResizeSubviewsNotification
                                                  object:self];
}

-(void)setVertical:(BOOL)flag {
    if (_isVertical == flag) {
        // Don't do unneccessary work
        return;
    }
   _isVertical=flag;
    
    // Now get the split axis sorted
   [self adjustSubviews];
}

-(BOOL)isFlipped {
   return YES;
}

-(BOOL)isSubviewCollapsed:(NSView *)subview {
    
    /* From Apple's header comments:
     * Collapsed subviews are hidden but retained by the split view.
     * Collapsing of a subview will not change its bounds, but may set its frame
     * to zero pixels high (in horizontal split views) or zero pixels wide (vertical).
     */
    return [subview isHidden];
}

-(void)setDividerStyle:(NSSplitViewDividerStyle)style {
	_dividerStyle = style;
	[self setNeedsDisplay: YES];
}

- (NSSplitViewDividerStyle)dividerStyle
{
	return _dividerStyle;
}

/** adjust all the non-collapsed subviews so that they are equally spaced horizontally within the splitview */
- (void)_adjustSubviewWidths
{
    // Set all the subview heights to the bounds height of the split view
    float height = NSHeight([self bounds]);

    int     i,count=[_subviews count];

    float totalWidthBefore=0.;
    
    // The available width to the subviews
    float totalWidthAfter=[self bounds].size.width-[self dividerThickness]*(count-1);
    
    for(i = 0; i < count; i++) {
        NSView *subview = [_subviews objectAtIndex: i];
        if ([self isSubviewCollapsed: subview] == NO) {
            totalWidthBefore += NSWidth([subview frame]);
        }
    }
   
    float delta = totalWidthAfter / totalWidthBefore;
    
    NSRect  frame = [self bounds];
    for(i = 0; i < count; i++){
        NSView *subview = [_subviews objectAtIndex: i];
        if ([self isSubviewCollapsed: subview] == NO) {
            frame.size.width= NSWidth([subview frame]) * delta;
            frame.size.width=floor(frame.size.width);
            frame.size.height = height;
            
            NSSize oldSize = [subview frame].size;
            
            [subview setFrame:frame];
            
            frame.origin.x+= NSWidth(frame);
            frame.origin.x+= [self dividerThickness];
        }
    }
}

- (void)_adjustSubviewHeights
{
    // Set all the subview widths to the bounds width of the split view
    float width = NSWidth([self bounds]);

    int     i,count=[_subviews count];

    // We've got to figure out how much the delta is between the old and new heights and multiply
    // all the heights to the new delta to get them to fit (or something like that...) Apple says
    // They resize proportionally
    float totalHeightBefore=0.;
    float totalHeightAfter=[self bounds].size.height-[self dividerThickness]*(count-1);
    
    for(i=0;i<count;i++) {
        NSView *subview = [_subviews objectAtIndex: i];
        if ([self isSubviewCollapsed: subview] == NO) {
            NSRect subviewFrame = [subview frame];
            totalHeightBefore += NSHeight(subviewFrame);
        }
    }

    float delta = totalHeightAfter / totalHeightBefore;
    

    NSRect  frame=[self bounds];
    for(i=0;i<count;i++){
        NSView *subview = [_subviews objectAtIndex: i];
        if ([self isSubviewCollapsed: subview] == NO) {
            frame.size.height= NSHeight([subview frame]) * delta;
            frame.size.height=floor(frame.size.height);
            frame.size.width = width;
            
            NSSize oldSize = [subview frame].size;
            
            [subview setFrame:frame];
            
            frame.origin.y+= NSHeight(frame);
            frame.origin.y+=[self dividerThickness];
        }
    }
}

/**
 * Apple says this method sets the frames of the split view's subviews so that they, plus the dividers,
 * fill the split view. The default implementation of this method resizes all of the subviews
 * proportionally so that the ratio of heights (in the horizontal split view case) or widths
 * (in the vertical split view case) doesn't change, even though the absolute sizes of the
 * subviews do change. This message should be sent to split views from which subviews have been
 * added or removed, to reestablish the consistency of subview placement.
 */
-(void)adjustSubviews {

    if ([_subviews count] < 2) {
        return;
    }
    
   [self _postNoteWillResize];

   if ([self isVertical]){
        [self _adjustSubviewWidths];
    }  else {
        [self _adjustSubviewHeights];
    }
    [self setNeedsDisplay: YES];
    [self _postNoteDidResize];
}

-(float)dividerThickness {
	if (_dividerStyle == NSSplitViewDividerStyleThick) {
		return 10;
	} else {
		return 5;
	}
}

-(NSImage *)dimpleImage {
   if([self isVertical])
    return [NSImage imageNamed:@"NSSplitViewVDimple"];
   else
    return [NSImage imageNamed:@"NSSplitViewHDimple"];
}

-(void)drawDividerInRect:(NSRect)rect {

	if (_dividerStyle != NSSplitViewDividerStylePaneSplitter) {
		// Fill in the view - pane splitter means just draw the dimple
		[[NSColor controlColor] setFill];
		NSRectFill(rect);
	}

	
	NSImage *image=[self dimpleImage];
	NSSize imageSize = [image size];

	NSPoint point = rect.origin;

	if([self isVertical]){
		point.x += floor((NSWidth(rect) - imageSize.width)/2);
		point.y += floor((NSHeight(rect) - imageSize.height)/2);
   } else {
		point.x += floor((NSWidth(rect) - imageSize.width)/2);
		point.y += floor((NSHeight(rect) - imageSize.height)/2);
   }

   [image drawAtPoint: point fromRect: NSZeroRect operation: NSCompositeSourceOver fraction: 1.0];
}

-(void)addSubview:(NSView *)view {
   [super addSubview:view];
   [self adjustSubviews];
}

-(void)resizeSubviewsWithOldSize:(NSSize)oldSize {
   NSSize  size=[self bounds].size;
   NSPoint origin=[self bounds].origin;
   int     i,count=[_subviews count];

   if(size.width<1) size.width=1;
   if(size.height<1) size.height=1;
   if(oldSize.width<1) oldSize.width=1;
   if(oldSize.height<1) oldSize.height=1;

   if([_delegate respondsToSelector:@selector(splitView:resizeSubviewsWithOldSize:)]) {
       [_delegate splitView:self resizeSubviewsWithOldSize:oldSize];
   } else {

       // Apple docs say just call adjustSubviews
       [self adjustSubviews];
   }
}

-(NSRect)dividerRectAtIndex:(unsigned)index {
   NSRect rect=[[_subviews objectAtIndex:index] frame];

   if([self isVertical]){
    rect.origin.x=rect.origin.x+rect.size.width;
    rect.size.width=[self dividerThickness];
   }
   else {
    rect.origin.y=rect.origin.y+rect.size.height;
    rect.size.height=[self dividerThickness];
   }

   return rect;
}

-(void)drawRect:(NSRect)rect {
   int i,count=[_subviews count];

   for(i=0;i<count-1;i++){
    if ([self dividerThickness] > 0)
     [self drawDividerInRect:[self dividerRectAtIndex:i]];
   }
}

-(unsigned)dividerIndexAtPoint:(NSPoint)point {
   int i,count=[[self subviews] count];

   for(i=0;i<count-1;i++){
    NSRect rect=[self dividerRectAtIndex:i];

    if(NSMouseInRect(point,rect,[self isFlipped]))
     return i;
   }

   return NSNotFound;
}

static float constrainTo(float value,float min,float max){
   if(value<min)
    value=min;
   if(value>max)
    value=max;
   return value;
}

-(void)mouseDown:(NSEvent *)event {

    NSPoint  firstPoint=[self convertPoint:[event locationInWindow] fromView:nil];
    unsigned divider=[self dividerIndexAtPoint:firstPoint];

    if (divider == NSNotFound) {
        return;
    }
    
    NSEventType eventType;

   [self _postNoteWillResize];

   do{
    NSAutoreleasePool *pool=[NSAutoreleasePool new];
    NSPoint point;

    event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|
                          NSLeftMouseDraggedMask];
    eventType=[event type];

    point=[self convertPoint:[event locationInWindow] fromView:nil];

       if([self isVertical]){
           [self setPosition: point.x ofDividerAtIndex: divider];
       } else {
           [self setPosition: point.y ofDividerAtIndex: divider];
       }
       
    [pool release];
   }while(eventType!=NSLeftMouseUp);

	if ([self dividerThickness] > 0)
		[[self window] invalidateCursorRectsForView:self];

   [self _postNoteDidResize];
}


-(void)resetCursorRects {
	
   if ([self dividerThickness] <= 0)
	   return;

	int       i,count=[_subviews count];
   NSCursor *cursor;

   if([self isVertical])
    cursor=[NSCursor resizeLeftRightCursor];
   else
    cursor=[NSCursor resizeUpDownCursor];

   for(i=0;i<count-1;i++){
    NSRect rect=[self dividerRectAtIndex:i];

// FIX
// The cursor is activated one pixel past NSMaxY(rect) if we don't do
// this, not sure where the problem is. 
    rect.origin.y-=1.;

    [self addCursorRect:rect cursor:cursor];
   }
}

- (float)minPossiblePositionOfDividerAtIndex:(int)index
{
    NSUnimplementedMethod();
    return 0;
}

- (float)maxPossiblePositionOfDividerAtIndex:(int)index
{
    NSUnimplementedMethod();
    return 0;
}

/** adjusts the subviews on either side of the divider while
 * honoring the constraints imposed by the delegate (if any)
 */
- (void)setPosition:(float)position ofDividerAtIndex:(int)index
{
    NSAssert(index >= 0 && index < [[self subviews] count] - 1, @"divider index out of range");
    
    NSView *subview0 = [[self subviews] objectAtIndex: index];
    NSView *subview1 = [[self subviews] objectAtIndex: index + 1];

    BOOL subview0Expanded = [self isSubviewCollapsed: subview0] == NO;
    BOOL subview1Expanded = [self isSubviewCollapsed: subview1] == NO;
    
    float    minPosition = 0;
    float    maxPosition = 0;
    
    NSRect frame0 = NSZeroRect;
    NSRect frame1 = NSZeroRect;

    
    if (subview0Expanded) {
        frame0 = [subview0 frame];
    }
    
    if (subview1Expanded) {
        frame1 = [subview1 frame];
    }
    
    // Determine the minimum position
    if (subview0Expanded) {
        if ([self isVertical]) {
            minPosition = NSMinX(frame0);
        } else {
            minPosition = NSMinY(frame0);
        }
    } else {
        NSAssert(subview1Expanded, @"both are collapsed??");
        if ([self isVertical]) {
            minPosition = NSMinX(frame1);
        } else {
            minPosition = NSMinY(frame1);
        }
    }
    
    // Determine the maximum position
    if (subview1Expanded) {
        if ([self isVertical]) {
            maxPosition = NSMaxX(frame1);
        } else {
            maxPosition = NSMaxY(frame1);
        }
    } else {
        NSAssert(subview0Expanded, @"both are collapsed??");
        if ([self isVertical]) {
            maxPosition = NSMaxX(frame0);
        } else {
            maxPosition = NSMaxY(frame0);
        }
    }

    // Check in with the delegate and see if it wants to tweak the min and max
    
    if ([_delegate respondsToSelector: @selector(splitView:constrainMinCoordinate:ofSubviewAt:)] ||
        [_delegate respondsToSelector: @selector(splitView:constrainMaxCoordinate:ofSubviewAt:)]) {
        // Use the modern API
        if ([_delegate respondsToSelector: @selector(splitView:constrainMinCoordinate:ofSubviewAt:)]) {
            minPosition = [_delegate splitView: self constrainMinCoordinate: minPosition ofSubviewAt: index];
        }
        if ([_delegate respondsToSelector: @selector(splitView:constrainMaxCoordinate:ofSubviewAt:)]) {
            maxPosition = [_delegate splitView: self constrainMaxCoordinate: maxPosition ofSubviewAt: index];
        }
    }
    else if([_delegate respondsToSelector:@selector(splitView:constrainMinCoordinate:maxCoordinate:ofSubviewAt:)]) {
        // Use the deprecated API
        
        [_delegate splitView:self constrainMinCoordinate:&minPosition maxCoordinate:&maxPosition ofSubviewAt: index];
    }
    
    // And if it wants to constrain the divider position
    BOOL delegateWantsTrackConstraining = [_delegate respondsToSelector:@selector(splitView:constrainSplitPosition:ofSubviewAt:)];

    if(delegateWantsTrackConstraining) {
        position = [_delegate splitView:self constrainSplitPosition: position ofSubviewAt: index];
    }
    

    // OK we're ready to figure out where the divider can be positioned
    NSRect  resize0=frame0;
    NSRect  resize1=frame1;
    
    BOOL subviewsWereCollapsedOrExpanded = NO;
    BOOL checkWithDelegateAboutCollapsingViews = [_delegate respondsToSelector:@selector(splitView:canCollapseSubview:)];
    if([self isVertical]){
        
        float lastPosition = NSMaxX(resize0);
        
        float delta = floor(position - lastPosition);
        
        resize0.size.width += delta;
        
        resize1.size.width -= delta;
        
        if (checkWithDelegateAboutCollapsingViews) {
            if (position < minPosition) {
                if ([_delegate splitView: self canCollapseSubview: subview0]) {
                    [subview0 setHidden: YES];
                    subviewsWereCollapsedOrExpanded = YES;
                }
            } else if (position > maxPosition) {
                if ([_delegate splitView: self canCollapseSubview: subview1]) {
                    [subview1 setHidden: YES];
                    subviewsWereCollapsedOrExpanded = YES;
                }
            }
        }
        
        // But make sure collapsed views can reappear
        if (position > minPosition && [subview0 isHidden]) {
            [subview0 setHidden: NO];
            subviewsWereCollapsedOrExpanded = YES;
        } else if (position < maxPosition && [subview1 isHidden]) {
            [subview1 setHidden: NO];
            subviewsWereCollapsedOrExpanded = YES;
        }

        // Figure out the adjusted widths
        resize0.size.width = constrainTo(NSWidth(resize0), minPosition, maxPosition);
        resize1.size.width = constrainTo(NSWidth(resize1), minPosition, maxPosition);
        resize1.origin.x = (NSMinX(frame1) + NSWidth(frame1)) - NSWidth(resize1);
    }
    else {
        
        float lastPosition = NSMaxY(resize0);
        float delta = floor(position - lastPosition);
        
        resize0.size.height += delta;

        resize1.size.height -= delta;
        
        if (checkWithDelegateAboutCollapsingViews) {
            if (position < minPosition) {
                if ([_delegate splitView: self canCollapseSubview: subview0]) {
                    [subview0 setHidden: YES];
                    subviewsWereCollapsedOrExpanded = YES;
                }
            } else if (position > maxPosition) {
                if ([_delegate splitView: self canCollapseSubview: subview1]) {
                    [subview1 setHidden: YES];
                    subviewsWereCollapsedOrExpanded = YES;
                }
            }
        }
        
        // But make sure collapsed views can reappear
        if (position > minPosition && [subview0 isHidden]) {
            [subview0 setHidden: NO];
            subviewsWereCollapsedOrExpanded = YES;
        } else if (position < maxPosition && [subview1 isHidden]) {
            [subview1 setHidden: NO];
            subviewsWereCollapsedOrExpanded = YES;
        }

        // Figure out the adjusted heights
        resize0.size.height = constrainTo(NSHeight(resize0), minPosition, maxPosition);
        resize1.size.height = constrainTo(NSHeight(resize1), minPosition, maxPosition);
        resize1.origin.y = (NSMinY(frame1) + NSHeight(frame1)) - NSHeight(resize1);
    }

    if (subviewsWereCollapsedOrExpanded) {
        // It doesn't really matter what happened with the divider because we need
        // to get the views re-laid out - so fall back to adjustSubviews and bail
        [self adjustSubviews];
        return;
    }

    // Nothing special happened so just resize the subviews as expected
    if ([subview0 isHidden] == NO) {
        [subview0 setFrame: resize0];
        // Tell the view to redisplay otherwise there are drawing artifacts
        [subview0  setNeedsDisplay: YES];
    }
    
    if ([subview1 isHidden] == NO) {
        [subview1 setFrame: resize1];
        // Tell the view to redisplay otherwise there are drawing artifacts
        [subview1  setNeedsDisplay: YES];
    }
    
    [self setNeedsDisplay:YES];
}

@end

