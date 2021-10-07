/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009-2010 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSView.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSWindow-Private.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSCursorRect.h>
#import <AppKit/NSTrackingArea.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSScrollView.h>
#import <AppKit/NSClipView.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSGraphicsContextFunctions.h>
#import <AppKit/NSDraggingManager.h>
#import <AppKit/NSDragging.h>
#import <AppKit/NSPrintOperation.h>
#import <AppKit/NSPrintInfo.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSPasteboard.h>
#import <AppKit/NSObject+BindingSupport.h>
#import <Onyx2D/O2Context.h>
#import <AppKit/NSRaise.h>
#import <AppKit/NSViewBackingLayer.h>
#import <CoreGraphics/CGWindow.h>
#import <QuartzCore/CALayerContext.h>
#import <QuartzCore/CATransaction.h>

NSString * const NSViewFrameDidChangeNotification=@"NSViewFrameDidChangeNotification";
NSString * const NSViewBoundsDidChangeNotification=@"NSViewBoundsDidChangeNotification";
NSString * const NSViewFocusDidChangeNotification=@"NSViewFocusDidChangeNotification";

@interface NSView(NSView_forward)
-(CGAffineTransform)transformFromWindow;
-(CGAffineTransform)transformToWindow;
-(CGAffineTransform)transformToLayer;
-(void)_trackingAreasChanged;
@end

@implementation NSView

static BOOL NSViewLayersEnabled=NO;

+(void)initialize {
   NSViewLayersEnabled=[[NSUserDefaults standardUserDefaults] boolForKey:@"NSViewLayersEnabled"];
}

+(NSView *)focusView {
   return [NSCurrentFocusStack() lastObject];
}

+(NSMenu *)defaultMenu {
   return nil;
}

+(NSFocusRingType)defaultFocusRingType {
   NSUnimplementedMethod();
   return nil;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    unsigned           vFlags=[keyed decodeIntForKey:@"NSvFlags"];

    _frame=NSZeroRect;
    if([keyed containsValueForKey:@"NSFrame"])
     _frame=[keyed decodeRectForKey:@"NSFrame"];
    else if([keyed containsValueForKey:@"NSFrameSize"])
     _frame.size=[keyed decodeSizeForKey:@"NSFrameSize"];

    _bounds.origin=NSMakePoint(0,0);
    _bounds.size=_frame.size;
    _window=nil;
    _superview=nil;
    _subviews=[NSMutableArray new];
    _postsNotificationOnFrameChange=YES;
    _postsNotificationOnBoundsChange=YES;
    _autoresizingMask=vFlags&0x3F;
    if([keyed containsValueForKey:@"NSvFlags"])
     _autoresizesSubviews=(vFlags&0x100)?YES:NO;
// Despite the fact it appears _autoresizesSubviews is encoded in the flags, it should always be ON
// *** Do not turn off or base on the flags unless you have an alternative for enabling it when it
// should be.
     _autoresizesSubviews=YES;
    _isHidden=(vFlags&0x80000000)?YES:NO;
    _tag= 0; // IB assigns a default tag id of 0 - which is different from the default in the docs.
    if([keyed containsValueForKey:@"NSTag"])
     _tag=[keyed decodeIntForKey:@"NSTag"];
     
	// Subviews come in from the nib in back to front order
    [_subviews addObjectsFromArray:[keyed decodeObjectForKey:@"NSSubviews"]];
	[_subviews makeObjectsPerformSelector:@selector(viewWillMoveToSuperview:) withObject:self];
    [_subviews makeObjectsPerformSelector:@selector(_setSuperview:) withObject:self];
	[_subviews makeObjectsPerformSelector:@selector(viewDidMoveToSuperview)];

    _needsDisplay=YES;
    _invalidRectCount=0;
    _invalidRects=NULL;
    _trackingAreas=[[NSMutableArray alloc] init];
    [self setWantsLayer:[keyed decodeBoolForKey:@"NSViewIsLayerTreeHost"]];
         
    _layerContentsRedrawPolicy=[keyed decodeIntegerForKey:@"NSViewLayerContentsRedrawPolicy"];
    
    _contentFilters=[[keyed decodeObjectForKey:@"NSViewContentFilters"] retain];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
   }

   return self;
}

-init {
   return [self initWithFrame:NSMakeRect(0,0,1,1)];
}

-initWithFrame:(NSRect)frame {
   _frame=frame;
   _bounds.origin=NSMakePoint(0,0);
   _bounds.size=frame.size;
   _window=nil;
   _menu=nil;
   _superview=nil;
   _subviews=[NSMutableArray new];
   _postsNotificationOnFrameChange=YES;
   _postsNotificationOnBoundsChange=YES;
   _autoresizesSubviews=YES;
   _autoresizingMask=NSViewNotSizable;
   _tag=-1; // according to the docs - loading from a nib gets a default of 0.
   _needsDisplay=YES;
   _invalidRectCount=0;
   _invalidRects=NULL;
   _trackingAreas=[[NSMutableArray alloc] init];
   
   _validTransforms=NO;
   _transformFromWindow=CGAffineTransformIdentity;
   _transformToWindow=CGAffineTransformIdentity;
   _transformToLayer=CGAffineTransformIdentity;

   return self;
}

-(void)dealloc {

	// Do this first?
	[self _unbindAllBindings];

   _window=nil;
   [_menu release];

   _superview=nil;
   [_subviews makeObjectsPerformSelector:@selector(_setSuperview:) withObject:nil];

   [_subviews release];
   [_draggedTypes release];
   [_trackingAreas release];
   [_contentFilters release];

   if(_invalidRects!=NULL)
    NSZoneFree(NULL,_invalidRects);
   
	if (_rectsBeingRedrawn!=NULL) {
		NSZoneFree(NULL, _rectsBeingRedrawn);
	}
   [_layer release];
   
   [_layerContext invalidate];
   [_layerContext release];

   [super dealloc];
}

static void invalidateTransform(NSView *self){
   self->_validTransforms=NO;
   self->_validTrackingAreas=NO;
   
   for(NSView *check in self->_subviews)
    invalidateTransform(check);

}

static CGAffineTransform concatViewTransform(CGAffineTransform result,NSView *view,NSView *superview,BOOL doFrame,BOOL flip){
   NSRect bounds=[view bounds];
   NSRect frame=[view frame];

   if(doFrame)
   result=CGAffineTransformTranslate(result,frame.origin.x,frame.origin.y);

	// Apply bounds scaling to fit in the frame
	CGAffineTransform scale = CGAffineTransformMakeScale(NSWidth(frame)/NSWidth(bounds), NSHeight(frame)/NSHeight(bounds));
	result=CGAffineTransformConcat(scale,result);
	
   if(flip){
    CGAffineTransform flip=CGAffineTransformMake(1,0,0,-1,0,bounds.size.height);

    result=CGAffineTransformConcat(flip,result);
   }
   result=CGAffineTransformTranslate(result,-bounds.origin.x,-bounds.origin.y);

   return result;
}

-(CGAffineTransform)createTransformToWindow {
   CGAffineTransform result;
   NSView *superview=[self superview];
   BOOL    doFrame=YES;
   BOOL    flip;
   
   if(superview==nil){
    result=CGAffineTransformIdentity;
    flip=[self isFlipped];
   }
   else {
    result=[superview transformToWindow];
    flip=([self isFlipped]!=[superview isFlipped]);
   }

   result=concatViewTransform(result,self,superview,doFrame,flip);
   
   return result;
}

-(CGAffineTransform)createTransformToLayer {
   CGAffineTransform result=CGAffineTransformIdentity;
   NSRect bounds=[self bounds];
   
   if([self isFlipped]){
    CGAffineTransform flip=CGAffineTransformMake(1,0,0,-1,0,bounds.size.height);

    result=CGAffineTransformConcat(flip,result);
   }
   result=CGAffineTransformTranslate(result,-bounds.origin.x,-bounds.origin.y);

   return result;
}

-(NSRect)calculateVisibleRect {
   if([self isHiddenOrHasHiddenAncestor])
    return NSZeroRect;
    
   if([self superview]==nil)
    return [self bounds];
   else {
    NSRect result=[[self superview] visibleRect];

    result=[self convertRect:result fromView:[self superview]];

    result=NSIntersectionRect(result,[self bounds]);

    return result;
   }
}

static inline void configureLayerGeometry(NSView *self){
   CALayer *layer=self->_layer;
   
   if(layer==nil)
    return;
    
   [CATransaction begin];
   [CATransaction setDisableActions:YES];
   
   [layer setAnchorPoint:CGPointMake(0,0)];
   
   if([layer superlayer]==nil)
    [layer setPosition:CGPointMake(0,0)];
   else
    [layer setPosition:self->_frame.origin];
   
   [layer setBounds:self->_bounds];
   [CATransaction commit];
}

static inline void buildTransformsIfNeeded(NSView *self) {
   if(!self->_validTransforms){
    self->_transformToWindow=[self createTransformToWindow];
    self->_transformFromWindow=CGAffineTransformInvert(self->_transformToWindow);
    self->_transformToLayer=[self createTransformToLayer];
    self->_validTransforms=YES;

    self->_visibleRect=[self calculateVisibleRect];
    configureLayerGeometry(self);
   }
}

-(CGAffineTransform)transformFromWindow {
   buildTransformsIfNeeded(self);

   return _transformFromWindow;
}

-(CGAffineTransform)transformToWindow {
   buildTransformsIfNeeded(self);

   return _transformToWindow;
}


-(CGAffineTransform)transformToLayer {
   buildTransformsIfNeeded(self);

   return _transformToLayer;
}

-(NSRect)frame {
   return _frame;
}

-(CGFloat)frameRotation {
   return _frameRotation;
}

-(CGFloat)frameCenterRotation {
   NSUnimplementedMethod();
   return 0.;
}

-(NSRect)bounds {
   return _bounds;
}

-(CGFloat)boundsRotation {
   return _boundsRotation;
}

-(BOOL)isRotatedFromBase {
   NSUnimplementedMethod();
   return NO;
}
  
-(BOOL)isRotatedOrScaledFromBase {
   NSUnimplementedMethod();
   return NO;
}

-(void)translateOriginToPoint:(NSPoint)point {
   NSUnimplementedMethod();
}

-(void)rotateByAngle:(CGFloat)angle {
   [self setBoundsRotation:[self boundsRotation]+angle];
}

-(BOOL)postsFrameChangedNotifications {
   return _postsNotificationOnFrameChange;
}

-(BOOL)postsBoundsChangedNotifications {
   return _postsNotificationOnBoundsChange;
}

-(void)scaleUnitSquareToSize:(NSSize)size {
   NSUnimplementedMethod();
}

-(NSWindow *)window {
   return _window;
}

-(NSView *)superview {
   return _superview;
}

-(BOOL)isDescendantOf:(NSView *)other {
   NSView *check=self;
   
   do {
    if(check==other)
     return YES;
     
    check=[check superview];
   }while(check!=nil);
   
   return NO;
}

-(NSView *)ancestorSharedWithView:(NSView *)view {
   NSUnimplementedMethod();
   return nil;
}

-(NSScrollView *)enclosingScrollView {
   id result=[self superview];

   for(;result!=nil;result=[result superview])
    if([result isKindOfClass:[NSScrollView class]])
     return result;

   return nil;
}

-(NSRect)adjustScroll:(NSRect)toRect {
   NSUnimplementedMethod();
   return NSZeroRect;
}

-(NSArray *)subviews {
   return _subviews;
}

-(BOOL)autoresizesSubviews {
   return _autoresizesSubviews;
}

-(unsigned)autoresizingMask {
   return _autoresizingMask;
}

-(NSFocusRingType)focusRingType {
   return _focusRingType;
}

-(int)tag {
   return _tag;
}

-(BOOL)isFlipped {
   return NO;
}

-(BOOL)isOpaque {
   return NO;
}

-(CGFloat)alphaValue {
   NSUnimplementedMethod();
   return 0.;
}

-(void)setAlphaValue:(CGFloat)alpha {
   NSUnimplementedMethod();
}

-(int)gState {
   return 0;
}

-(NSRect)visibleRect {
   buildTransformsIfNeeded(self);

   return _visibleRect;
}

-(BOOL)wantsDefaultClipping {
   NSUnimplementedMethod();
   return NO;
}

-(NSBitmapImageRep *)bitmapImageRepForCachingDisplayInRect:(NSRect)rect {
   NSUnimplementedMethod();
   return nil;
}

-(void)cacheDisplayInRect:(NSRect)rect toBitmapImageRep:(NSBitmapImageRep *)imageRep {
   NSUnimplementedMethod();
}

-(BOOL)isHidden {
   return _isHidden;
}

-(BOOL)isHiddenOrHasHiddenAncestor {
   return _isHidden || [_superview isHiddenOrHasHiddenAncestor];
}

-(void)setHidden:(BOOL)flag {
    flag=flag?YES:NO;

    if (_isHidden != flag) {
        invalidateTransform(self);
        if ((_isHidden = flag)) {
            id view=[_window firstResponder];
            
            if ([view isKindOfClass:[NSView class]])
                for (; view; view = [view superview]) {
                    if (self==view) {
                        [_window makeFirstResponder:[self nextValidKeyView]];
                        break;
                    }
                }
        }

        [[self superview] setNeedsDisplay:YES];
      
        if(_isHidden)
            [self viewDidHide];
        else
            [self viewDidUnhide];
   }
}

-(void)viewDidHide {
    // do nothing?
}

-(void)viewDidUnhide {
    // do nothing?
}

-(BOOL)canBecomeKeyView {
   return [self acceptsFirstResponder] && ![self isHiddenOrHasHiddenAncestor];
}

// Cocoa does that for views, despite the "The default implementation returns NO" from the documentation
-(BOOL)needsPanelToBecomeKey {
    return [self acceptsFirstResponder];
}

-(NSView *)nextKeyView {
   return _nextKeyView;
}

-(NSView *)nextValidKeyView {
   NSView *result=[self nextKeyView];

    while(result!=nil && ![result canBecomeKeyView]) {
        // prevent an infinite loop
        if(result==self)
            return nil;
        
        result=[result nextKeyView];
    }
    

   return result;
}

-(NSView *)previousKeyView {
   return _previousKeyView;
}

-(NSView *)previousValidKeyView {
    NSView *result=[self previousKeyView];
    
    while(result!=nil && ![result canBecomeKeyView]) {
        // prevent an infinite loop
        if(result==self)
            return nil;
        result=[result previousKeyView];
    }
    
    return result;
}

-(NSMenu *)menu {
   return _menu;
}

-(NSMenu *)menuForEvent:(NSEvent *)event {
   NSMenu *result=[self menu];
   
   if(result==nil) {
    result=[isa defaultMenu];

    if(result) {
     NSArray *itemArray=[result itemArray];
     int i,count=[itemArray count];
     for(i=0;i<count;i++) {
      NSMenuItem *item = [itemArray objectAtIndex:i];
      [item setTarget:self];
     }
    }
   }

   return result;
}

-(NSMenuItem *)enclosingMenuItem {
   NSUnimplementedMethod();
   return nil;
}

-(NSString *)toolTip {
   NSUInteger i,count=[_trackingAreas count];
   NSString *toolTip=nil;

   // In case there's more than one ToolTip the behavior
   // is undocumented. Return the first one.
   for(i=0;i<count;i++){
    NSTrackingArea *area=[_trackingAreas objectAtIndex:i];

    if([area _isToolTip]==YES){
     toolTip=[area owner];
     break;
    }
   }

   return toolTip;
}

-viewWithTag:(int)tag {
   int i,count=[_subviews count];

   if(_tag==tag)
    return self;

   for(i=0;i<count;i++){
    NSView *view=[[_subviews objectAtIndex:i] viewWithTag:tag];

    if(view!=nil)
     return view;
   }

   return nil;
}

-(NSView *)hitTest:(NSPoint)point
{
	if(_isHidden) {
		return nil;
	}
	
	point = [self convertPoint:point fromView:[self superview]];
	
	if(NSMouseInRect(point, [self visibleRect], [self isFlipped]) == NO){
		return nil;
	} else {
		// Subviews are ordered back to front so we need to go
		// front to back in order to hit test correctly.
		NSArray *subviews = [self subviews];
		int      count = [subviews count];
	   
		while (--count >= 0) {
			NSView *check = [subviews objectAtIndex: count];
			NSView *hit = [check hitTest: point];

			if (hit != nil) {
				return hit;
			}
		}
	}
	return self;
}

-(NSPoint)convertPoint:(NSPoint)point fromView:(NSView *)viewOrNil {
   NSView           *fromView=(viewOrNil!=nil)?viewOrNil:[[self window] _backgroundView];
   CGAffineTransform toWindow=[fromView transformToWindow];
   CGAffineTransform fromWindow=[self transformFromWindow];

   return CGPointApplyAffineTransform(CGPointApplyAffineTransform(point,toWindow),fromWindow);
}

-(NSPoint)convertPoint:(NSPoint)point toView:(NSView *)viewOrNil {
   NSView           *toView=(viewOrNil!=nil)?viewOrNil:[[self window] _backgroundView];
   CGAffineTransform toWindow=[self transformToWindow];
   CGAffineTransform fromWindow=[toView transformFromWindow];

   return CGPointApplyAffineTransform(CGPointApplyAffineTransform(point,toWindow),fromWindow);
}

-(NSSize)convertSize:(NSSize)size fromView:(NSView *)viewOrNil {
   NSView           *fromView=(viewOrNil!=nil)?viewOrNil:[[self window] _backgroundView];
   CGAffineTransform toWindow=[fromView transformToWindow];
   CGAffineTransform fromWindow=[self transformFromWindow];

   return CGSizeApplyAffineTransform(CGSizeApplyAffineTransform(size,toWindow),fromWindow);
}

-(NSSize)convertSize:(NSSize)size toView:(NSView *)viewOrNil {
   NSView           *toView=(viewOrNil!=nil)?viewOrNil:[[self window] _backgroundView];
   CGAffineTransform toWindow=[self transformToWindow];
   CGAffineTransform fromWindow=[toView transformFromWindow];

   return CGSizeApplyAffineTransform(CGSizeApplyAffineTransform(size,toWindow),fromWindow);
}

-(NSRect)convertRect:(NSRect)rect fromView:(NSView *)viewOrNil {
   NSView           *fromView=(viewOrNil!=nil)?viewOrNil:[[self window] _backgroundView];
   CGAffineTransform toWindow=[fromView transformToWindow];
   CGAffineTransform fromWindow=[self transformFromWindow];
   NSPoint           point1=rect.origin;
   NSPoint           point2=NSMakePoint(NSMaxX(rect),NSMaxY(rect));

   point1= CGPointApplyAffineTransform(CGPointApplyAffineTransform(point1,toWindow),fromWindow);
   point2= CGPointApplyAffineTransform(CGPointApplyAffineTransform(point2,toWindow),fromWindow);
   if(point2.y<point1.y){
    float temp=point2.y;
    point2.y=point1.y;
    point1.y=temp;
   }

   return NSMakeRect(point1.x,point1.y,point2.x-point1.x,point2.y-point1.y);
}

-(NSRect)convertRect:(NSRect)rect toView:(NSView *)viewOrNil {
   NSView           *toView=(viewOrNil!=nil)?viewOrNil:[[self window] _backgroundView];
   CGAffineTransform toWindow=[self transformToWindow];
   CGAffineTransform fromWindow=[toView transformFromWindow];
   NSPoint           point1=rect.origin;
   NSPoint           point2=NSMakePoint(NSMaxX(rect),NSMaxY(rect));

   point1= CGPointApplyAffineTransform(CGPointApplyAffineTransform(point1,toWindow),fromWindow);
   point2= CGPointApplyAffineTransform(CGPointApplyAffineTransform(point2,toWindow),fromWindow);
   if(point2.y<point1.y){
    float temp=point2.y;
    point2.y=point1.y;
    point1.y=temp;
   }

   return NSMakeRect(point1.x,point1.y,point2.x-point1.x,point2.y-point1.y);
}

-(NSRect)centerScanRect:(NSRect)rect {
   float minx=floor(NSMinX(rect)+0.5);
   float miny=floor(NSMinY(rect)+0.5);
   float maxx=floor(NSMaxX(rect)+0.5);
   float maxy=floor(NSMaxY(rect)+0.5);

   return NSMakeRect(minx,miny,maxx-minx,maxy-miny);
}

-(void)setFrame:(NSRect)frame {
  // Cocoa does not post the notification if the frames are equal
   // Possible that resizeSubviewsWithOldSize is not called if the sizes are equal
   if(NSEqualRects(_frame,frame))
    return;

   NSSize oldSize=_bounds.size;

	if (_bounds.size.width == 0 || _bounds.size.height == 0) {
		// No valid current bounds value - just update it to use the frame size
		_bounds.size=frame.size;
	} else {
		// Get the bounds->frame transform
		CGAffineTransform transform=concatViewTransform(CGAffineTransformIdentity,self,nil,YES,NO);
		// ... and invert it so we can get the new bounds size from the new frame size
		transform = CGAffineTransformInvert(transform);
		
		_bounds.size=CGSizeApplyAffineTransform(frame.size, transform);
	}
	_frame=frame;
	
	[_window invalidateCursorRectsForView:self]; // this also invalidates tracking areas
	
   if(_autoresizesSubviews){
    [self resizeSubviewsWithOldSize:oldSize];
   }

    NSRect layerFrame=_frame;
    
    if(_superview!=nil)
        layerFrame=[_superview convertRect:layerFrame toView:nil];
    
    [_layerContext setFrame:layerFrame];
       
   invalidateTransform(self);

   if(_postsNotificationOnFrameChange)
    [[NSNotificationCenter defaultCenter] postNotificationName:NSViewFrameDidChangeNotification object:self];
}

-(void)setFrameSize:(NSSize)size {
   NSRect frame=_frame;

   frame.size=size;
   [self setFrame:frame];
}

-(void)setFrameOrigin:(NSPoint)origin {
   NSRect frame=[self frame];

   frame.origin=origin;
   [self setFrame:frame];
}

-(void)setFrameRotation:(CGFloat)angle {
   NSUnimplementedMethod();
}

-(void)setFrameCenterRotation:(CGFloat)angle {
   NSUnimplementedMethod();
}

-(void)setBounds:(NSRect)bounds {
	if (!NSEqualRects(bounds, _bounds)) {
		_bounds=bounds;
		invalidateTransform(self);
		
		[_window invalidateCursorRectsForView:self]; // this also invalidates tracking areas
		
		if(_postsNotificationOnBoundsChange)
			[[NSNotificationCenter defaultCenter] postNotificationName:NSViewBoundsDidChangeNotification object:self];
	}
}

-(void)setBoundsSize:(NSSize)size {
   NSRect bounds=[self bounds];

   bounds.size=size;

   [self setBounds:bounds];
}

-(void)setBoundsOrigin:(NSPoint)origin {
   NSRect bounds=[self bounds];

   bounds.origin=origin;

   [self setBounds:bounds];
}

-(void)setBoundsRotation:(CGFloat)angle {
   NSUnimplementedMethod();
}

-(void)setPostsFrameChangedNotifications:(BOOL)flag {
   _postsNotificationOnFrameChange=flag;
}

-(void)setPostsBoundsChangedNotifications:(BOOL)flag {
   _postsNotificationOnBoundsChange=flag;
}

-(void)_setWindow:(NSWindow *)window {
    if (_window != window) {
        BOOL windowRecalulatesKeyViewLoop = [window autorecalculatesKeyViewLoop];
        if (windowRecalulatesKeyViewLoop)
            [self setNextKeyView:nil];

        [self viewWillMoveToWindow:window];

        _window=window;

        [_subviews makeObjectsPerformSelector:_cmd withObject:window];
        _validTrackingAreas=NO;
        [_window invalidateCursorRectsForView:self]; // this also invalidates tracking areas

        if (windowRecalulatesKeyViewLoop)
            [_window recalculateKeyViewLoop];

        [self viewDidMoveToWindow];
    }
}

-(void)_setSuperview:superview {
   _superview=superview;
	
   [_window invalidateCursorRectsForView:self]; // this also invalidates tracking areas

   [self setNextResponder:superview];
}

-(void)_insertSubview:(NSView *)view atIndex:(NSUInteger)index {

   [view retain];
   if([view superview]==self)
    [_subviews removeObjectIdenticalTo:view];
   else{
    [view removeFromSuperview];

    [view _setWindow:_window];

    [view viewWillMoveToSuperview:self];
    [view _setSuperview:self];
   }

   if(index==NSNotFound)
    [_subviews addObject:view];
   else
    [_subviews insertObject:view atIndex:index];
   [view release];

   invalidateTransform(view);

   [self setNeedsDisplayInRect:[view frame]];

   [view viewDidMoveToSuperview];
    
   if(_wantsLayer)
    [view setWantsLayer:YES];
}

-(void)addSubview:(NSView *)view {
   if(view==nil) // yes, this is silently ignored
    return;
   
   [self _insertSubview:view atIndex:NSNotFound];
}

-(void)addSubview:(NSView *)view positioned:(NSWindowOrderingMode)ordering relativeTo:(NSView *)relativeTo {
   NSUInteger index=[_subviews indexOfObjectIdenticalTo:relativeTo];
   
   if(index==NSNotFound)
    index=(ordering==NSWindowBelow)?0:NSNotFound;
   else
    index=(ordering==NSWindowBelow)?index:((index+1==[_subviews count])?NSNotFound:index+1);
    
   [self _insertSubview:view atIndex:index];
}

-(void)replaceSubview:(NSView *)oldView with:(NSView *)newView {
   NSUInteger index=[_subviews indexOfObjectIdenticalTo:oldView];

   [oldView retain];
   [oldView removeFromSuperview];
   [self _insertSubview:newView atIndex:index];
   [oldView release];
}

-(void)setSubviews:(NSArray *)array {
// This method marks as needing display per doc.s

   while([_subviews count])
    [[_subviews lastObject] removeFromSuperview];
   
   for(NSView *view in array){
    [self addSubview:view];
    [view setNeedsDisplay:YES];
}
}

-(void)sortSubviewsUsingFunction:(NSComparisonResult (*)(id, id, void *))compareFunction context:(void *)context {
   NSUnimplementedMethod();
}

-(void)didAddSubview:(NSView *)subview {
   NSUnimplementedMethod();
}

-(void)willRemoveSubview:(NSView *)subview {
   NSUnimplementedMethod();
}

-(void)setAutoresizesSubviews:(BOOL)flag {
   _autoresizesSubviews=flag;
}

-(void)setAutoresizingMask:(unsigned int)mask {
   _autoresizingMask=mask;
}

-(void)setFocusRingType:(NSFocusRingType)value {
   _focusRingType=value;
   [self setNeedsDisplay:YES];
}

-(void)setTag:(int)tag {
   _tag=tag;
}

-(void)_setPreviousKeyView:(NSView *)previous {
    _previousKeyView = previous;
}

-(void)setNextKeyView:(NSView *)next {
    if (next)
        [next _setPreviousKeyView:self];
    else
        [_nextKeyView _setPreviousKeyView:nil];

    _nextKeyView = next;
}

-(BOOL)acceptsFirstMouse:(NSEvent *)event {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)acceptsTouchEvents {
   NSUnimplementedMethod();
   return NO;
}

-(void)setAcceptsTouchEvents:(BOOL)accepts {
   NSUnimplementedMethod();
}

-(BOOL)wantsRestingTouches {
   NSUnimplementedMethod();
   return NO;
}

-(void)setWantsRestingTouches:(BOOL)wants {
   NSUnimplementedMethod();
}
 
-(void)setToolTip:(NSString *)string {
   [self removeAllToolTips];
   if(string!=nil && ![string isEqualToString:@""])
    [self addToolTipRect:[self bounds] owner:string userData:NULL];
}

-(NSToolTipTag)addToolTipRect:(NSRect)rect owner:object userData:(void *)userData {
   NSTrackingArea *area=nil;

   area=[[NSTrackingArea alloc] _initWithRect:rect options:0 owner:object userData:userData retainUserData:NO isToolTip:YES isLegacy:NO];
   [_trackingAreas addObject:area];
   [area release];

   [self _trackingAreasChanged];

   return area;
}

-(void)removeToolTip:(NSToolTipTag)tag {
   [self removeTrackingArea:tag];
}

-(void)removeAllToolTips {
   NSInteger count=[_trackingAreas count];

   while(--count>=0){
    if([[_trackingAreas objectAtIndex:count] _isToolTip]==YES)
     [_trackingAreas removeObjectAtIndex:count];
   }

   [self _trackingAreasChanged];
}

-(void)addCursorRect:(NSRect)rect cursor:(NSCursor *)cursor {
   NSCursorRect *cursorRect=[[NSCursorRect alloc] initWithCursor:cursor];
   NSTrackingArea *area=nil;

   area=[[NSTrackingArea alloc] _initWithRect:rect options:NSTrackingCursorUpdate|NSTrackingActiveInKeyWindow owner:cursorRect userData:NULL retainUserData:NO isToolTip:NO isLegacy:YES];
   [_trackingAreas addObject:area];
   [area release];
   [cursorRect release];

   [self _trackingAreasChanged];
}

-(void)removeCursorRect:(NSRect)rect cursor:(NSCursor *)cursor {
   NSInteger count=[_trackingAreas count];

   while(--count>=0){
    NSTrackingArea *area=[_trackingAreas objectAtIndex:count];
    NSObject *candidate=[area owner];

    if([area _isLegacy]==YES &&
       [candidate isKindOfClass:[NSCursorRect class]]==YES &&
       [(NSCursorRect *)candidate cursor]==cursor){
     [_trackingAreas removeObjectAtIndex:count];
     break;
    }
   }

   [self _trackingAreasChanged];
}

-(void)discardCursorRects {
   NSInteger count=[_trackingAreas count];

   while(--count>=0){
    NSTrackingArea *area=[_trackingAreas objectAtIndex:count];

    if([area _isLegacy]==YES && ([area options]&NSTrackingCursorUpdate)){
     [_trackingAreas removeObjectAtIndex:count];
    }
   }

   [[self subviews] makeObjectsPerformSelector:_cmd];

   [self _trackingAreasChanged];
}

-(void)resetCursorRects {
   // do nothing
}

-(void)_collectTrackingAreasForWindowInto:(NSMutableArray *)collector {
   if(_isHidden==NO){
    NSUInteger  i,count;

    if(!_validTrackingAreas){
     /* We don't clear the tracking areas, they are managed by the view with add/remove
      */
     [self updateTrackingAreas];
     _validTrackingAreas=YES;
    }
	   
    count=[_trackingAreas count];
    for(i=0;i<count;i++){
     NSTrackingArea *area=[_trackingAreas objectAtIndex:i];
     NSRect          rectOfInterest;

     rectOfInterest=NSIntersectionRect([area rect], [self bounds]);

     if(rectOfInterest.size.width>0. && rectOfInterest.size.height>0.){
      [area _setView:self];

      [area _setRectInWindow:[self convertRect:rectOfInterest toView:nil]];

      [collector addObject:[_trackingAreas objectAtIndex:i]];
     }
    }

    NSArray *subviews=[self subviews];
    // Collect subviews' areas _after_ collecting our own.
    count=[subviews count];
    for(i=0;i<count;i++)
     [(NSView *)[subviews objectAtIndex:i] _collectTrackingAreasForWindowInto:collector];
   }
}

-(NSArray *)trackingAreas {
   return _trackingAreas;
}

-(void)_trackingAreasChanged {
   [[self window] _invalidateTrackingAreas];
}

-(void)addTrackingArea:(NSTrackingArea *)trackingArea {
   [_trackingAreas addObject:trackingArea];
   [self _trackingAreasChanged];
}

-(void)removeTrackingArea:(NSTrackingArea *)trackingArea {
   [_trackingAreas removeObjectIdenticalTo:trackingArea];
   [self _trackingAreasChanged];
}

-(void)updateTrackingAreas {
   [self _trackingAreasChanged];
}

-(NSTrackingRectTag)addTrackingRect:(NSRect)rect owner:owner userData:(void *)userData assumeInside:(BOOL)assumeInside {
   NSTrackingAreaOptions options=NSTrackingMouseEnteredAndExited|NSTrackingActiveAlways;
   NSTrackingArea *area=nil;

   if(assumeInside==YES)
    options|=NSTrackingAssumeInside;

   area=[[NSTrackingArea alloc] _initWithRect:rect options:options owner:owner userData:NULL retainUserData:NO isToolTip:NO isLegacy:NO];
   [_trackingAreas addObject:area];
   [area release];

   [self _trackingAreasChanged];

   return area;
}

-(void)removeTrackingRect:(NSTrackingRectTag)tag {
   [self removeTrackingArea:tag];
}

-(NSTextInputContext *)inputContext {
   NSUnimplementedMethod();
   return nil;
}

-(void)registerForDraggedTypes:(NSArray *)types {
   types=[types copy];
   [_draggedTypes release];
   _draggedTypes=types;
}

-(void)unregisterDraggedTypes {
   [_draggedTypes release];
   _draggedTypes=nil;
}

-(NSArray *)registeredDraggedTypes {
   return _draggedTypes;
}

-(void)_deepResignFirstResponder {
   if([_window firstResponder]==self)
    [_window makeFirstResponder:nil];
   
   [[self subviews] makeObjectsPerformSelector:_cmd];
}

-(void)removeFromSuperview {
   [_superview setNeedsDisplayInRect:[self frame]];
   [self removeFromSuperviewWithoutNeedingDisplay];
}

-(void)_removeViewWithoutDisplay:(NSView *)view  {
   [_subviews removeObjectIdenticalTo:view];
}

-(void)removeFromSuperviewWithoutNeedingDisplay {
   NSView *removeFrom=_superview;
   NSWindow *window=[self window];

   [self _deepResignFirstResponder];
   [self _setSuperview:nil];
   [self _setWindow:nil];

   [removeFrom _removeViewWithoutDisplay:self];
   [window _invalidateTrackingAreas];
}

-(void)viewWillMoveToSuperview:(NSView *)view {
   // Intentionally empty.
}

-(void)viewDidMoveToSuperview {
   // Intentionally empty.
}

-(void)viewWillMoveToWindow:(NSWindow *)window {
   // Intentionally empty.
}

-(void)viewDidMoveToWindow {
   // Default implementation does nothing
}

-(BOOL)shouldDelayWindowOrderingForEvent:(NSEvent *)event {
   NSUnimplementedMethod();
   return NO;
}

-(void)resizeSubviewsWithOldSize:(NSSize)oldSize {
   int i,count=[_subviews count];

   for(i=0;i<count;i++)
    [[_subviews objectAtIndex:i] resizeWithOldSuperviewSize:oldSize];
}

-(void)resizeWithOldSuperviewSize:(NSSize)oldSize {
   NSRect superFrame=[_superview frame];
   NSRect frame=[self frame];
   BOOL   originChanged=NO,sizeChanged=NO;

   if(_autoresizingMask&NSViewMinXMargin){
    if(_autoresizingMask&NSViewWidthSizable){
     if(_autoresizingMask&NSViewMaxXMargin){
      frame.origin.x+=((superFrame.size.width-oldSize.width)/3);
      frame.size.width+=((superFrame.size.width-oldSize.width)/3);
     }
     else {
      frame.origin.x+=((superFrame.size.width-oldSize.width)/2);
      frame.size.width+=((superFrame.size.width-oldSize.width)/2);
     }
     originChanged=YES;
     sizeChanged=YES;
    }
    else if(_autoresizingMask&NSViewMaxXMargin){
     frame.origin.x+=((superFrame.size.width-oldSize.width)/2);
     originChanged=YES;
    }
    else{
     frame.origin.x+=superFrame.size.width-oldSize.width;
     originChanged=YES;
    }
   }
   else if(_autoresizingMask&NSViewWidthSizable){
    if(_autoresizingMask&NSViewMaxXMargin)
     frame.size.width+=((superFrame.size.width-oldSize.width)/2);
    else
     frame.size.width+=superFrame.size.width-oldSize.width;
    sizeChanged=YES;
   }
   else if(_autoresizingMask&NSViewMaxXMargin){
    // don't move or resize
   }


   if(_autoresizingMask& NSViewMinYMargin){
    if(_autoresizingMask& NSViewHeightSizable){
     if(_autoresizingMask& NSViewMaxYMargin){
      frame.origin.y+=((superFrame.size.height-oldSize.height)/3);
      frame.size.height+=((superFrame.size.height-oldSize.height)/3);
     }
     else {
      frame.origin.y+=((superFrame.size.height-oldSize.height)/2);
      frame.size.height+=((superFrame.size.height-oldSize.height)/2);
     }
     originChanged=YES;
     sizeChanged=YES;
    }
    else if(_autoresizingMask& NSViewMaxYMargin){
     frame.origin.y+=((superFrame.size.height-oldSize.height)/2);
     originChanged=YES;
    }
    else {
     frame.origin.y+=superFrame.size.height-oldSize.height;
     originChanged=YES;
    }
   }
   else if(_autoresizingMask&NSViewHeightSizable){
    if(_autoresizingMask& NSViewMaxYMargin)
     frame.size.height+=((superFrame.size.height-oldSize.height)/2);
    else
     frame.size.height+=superFrame.size.height-oldSize.height;
    sizeChanged=YES;
   }

   if(originChanged || sizeChanged)
    [self setFrame:frame];
}

-(BOOL)inLiveResize {
   return _inLiveResize;
}

-(BOOL)preservesContentDuringLiveResize {
   NSUnimplementedMethod();
   return NO;
}

-(NSRect)rectPreservedDuringLiveResize {
   NSUnimplementedMethod();
   return NSZeroRect;
}

-(void)viewWillStartLiveResize {
	_inLiveResize=YES;
   [_subviews makeObjectsPerformSelector:_cmd];
}

-(void)viewDidEndLiveResize {
   _inLiveResize=NO;
   [_subviews makeObjectsPerformSelector:_cmd];
}

-(BOOL)enterFullScreenMode:(NSScreen *)screen withOptions:(NSDictionary *)options {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)isInFullScreenMode {
// dont issue warning
   return NO;
}

-(void)exitFullScreenModeWithOptions:(NSDictionary *)options {
   NSUnimplementedMethod();
}

-(NSClipView *)_enclosingClipView {
   id result=[self superview];

   for(;result!=nil;result=[result superview])
    if([result isKindOfClass:[NSClipView class]])
     return result;

   return nil;
}

-(void)scrollPoint:(NSPoint)point {
   NSClipView *clipView=[self _enclosingClipView];

   if(clipView!=nil){
    NSPoint origin=[self convertPoint:point toView:clipView];

    [clipView scrollToPoint:origin];
   }
}

-(BOOL)scrollRectToVisible:(NSRect)rect {
    NSClipView *clipView = [self _enclosingClipView];
    NSView *documentView = [clipView documentView];
    // Current the document view visible rect in document view space
    NSRect vRect = [clipView documentVisibleRect];
    // Convert what we want in the document view space 
    rect = [documentView convertRect:rect fromView:self];
    
    // Do the minimal amount of scrolling to show the rect
    
    // Missing amount on the four directions
    float missingLeft = NSMinX(vRect) - NSMinX(rect);
    float missingRight = NSMaxX(rect) - NSMaxX(vRect);

    float missingTop = NSMinY(vRect) - NSMinY(rect);
    float missingBottom = NSMaxY(rect) - NSMaxY(vRect);
    
    float dx = 0.;
    float dy = 0.;
    
    if (missingLeft * missingRight < 0) {
        // We need to scroll in one direction - no need to scroll if we're missing bits both ways or
        // if everything is visible
        
        // Let's do the minimal amount of scrolling
        if (fabs(missingLeft) < fabs(missingRight)) {
            dx = -missingLeft;
        } else {
            dx = missingRight;
        }
    }

    if (missingTop * missingBottom < 0) {
        // We need to scroll in one direction - no need to scroll if we're missing bits both ways or
        // if everything is visible
        
        // Let's do the minimal amount of scrolling
        if (fabs(missingTop) < fabs(missingBottom)) {
            dy = -missingTop;
        } else {
            dy = missingBottom;
        }
    }
    if (dx != 0 || dy != 0) {
        NSPoint pt = vRect.origin;
        pt.x += dx;
        pt.y += dy;
        pt = [documentView convertPoint:pt toView:clipView];
        [clipView scrollToPoint:pt];
        return YES;
    }
    return NO;
}

-(void)scrollClipView:(NSClipView *)clipView toPoint:(NSPoint)newOrigin {
   NSUnimplementedMethod();
}

-(BOOL)mouse:(NSPoint)point inRect:(NSRect)rect {
   return NSMouseInRect(point, rect, [self isFlipped]);
}

-(void)reflectScrolledClipView:(NSClipView *)view {
   NSUnimplementedMethod();
}

-(void)allocateGState {
   // unimplemented
}

-(void)releaseGState {
   // unimplemented
}

-(void)setUpGState {
   // do nothing
}

-(void)renewGState {
   // do nothing
}

-(BOOL)wantsLayer {
   return _wantsLayer;
}

-(CALayer *)layer {
   return _layer;
}

-(CALayer *)makeBackingLayer {
   return [NSViewBackingLayer layer];
}

-(void)_removeLayerFromSuperlayer {
   [_layer removeFromSuperlayer];    
   [_layerContext invalidate];
   [_layerContext release];
   _layerContext=nil;
}

-(void)_createLayerContextIfNeeded {
   if([_superview layer]==nil){
    _layerContext=[[CALayerContext alloc] initWithFrame:[self frame]];
    [_layerContext setLayer:_layer];
   }
}

-(void)_addLayerToSuperlayer {
   [[_superview layer] addSublayer:_layer];
   [self _createLayerContextIfNeeded];
}

/*
  When turning off layering, we only turn off layers below us which did not want a layer
  Layers which did want a layer are not touched, nor are their children.
 */
-(void)_removeLayerBackedViewsFromTree {
   if(_wantsLayer)
    [self _createLayerContextIfNeeded];
   else {
    [self _removeLayerFromSuperlayer];

// A backing layer is removed regardless of whether it was set implicitly or explicitly
// The distinction appears to be based on the layers class, not how it was set (host vs. backing)
    if([_layer isKindOfClass:[NSViewBackingLayer class]]){
     [_layer release];
     _layer=nil;
    }

    [_subviews makeObjectsPerformSelector:_cmd];
   }
}

-(void)_createLayersInTreeIfNeeded {
   if(!NSViewLayersEnabled)
    return;
    
   if(_layer==nil){
    _layer=[[self makeBackingLayer] retain];
    configureLayerGeometry(self);
   }
   
   [self _addLayerToSuperlayer];

   [_subviews makeObjectsPerformSelector:_cmd];
}

-(void)setWantsLayer:(BOOL)value {   
   if(!value){
    if(_wantsLayer){
     _wantsLayer=NO;
     [self _removeLayerBackedViewsFromTree];
    }
   }
   else {
    if(!_wantsLayer){
     _wantsLayer=YES;
     [self _createLayersInTreeIfNeeded];
    }
   }
}

-(void)setLayer:(CALayer *)value {
   if(!NSViewLayersEnabled)
    return;

   if(value!=_layer){
    [_subviews makeObjectsPerformSelector:@selector(_removeLayerFromSuperlayer)];
    
    value=[value retain];  
    
    if(_layer==nil){
     if(value!=nil){
      _layer=value;
      [self _addLayerToSuperlayer];
     }
    }
    else if(value==nil){
     [self _removeLayerFromSuperlayer];
     [_layer release];
     _layer=nil;
    }
    else {
     [[_superview layer] replaceSublayer:_layer with:value];
     [_layer release];
      _layer=value;
    }
    
    [_subviews makeObjectsPerformSelector:@selector(_addLayerToSuperlayer)];
   }
}

-(NSViewLayerContentsPlacement)layerContentsPlacement {
   return _layerContentsPlacement;
}

-(void)setLayerContentsPlacement:(NSViewLayerContentsPlacement)value {
   _layerContentsPlacement=value;
}

-(NSViewLayerContentsRedrawPolicy)layerContentsRedrawPolicy {
   return _layerContentsRedrawPolicy;
}

-(void)setLayerContentsRedrawPolicy:(NSViewLayerContentsRedrawPolicy)value {
   _layerContentsRedrawPolicy=value;
}


-(NSArray *)backgroundFilters {
   NSUnimplementedMethod();
   return nil;
}

-(void)setBackgroundFilters:(NSArray *)filters {
// FIXME: implement but dont warn
//   NSUnimplementedMethod();
}

-(NSArray *)contentFilters {
   return _contentFilters;
}

-(void)setContentFilters:(NSArray *)filters {
   filters=[filters copy];
   [_contentFilters release];
   _contentFilters=filters;
}

-(CIFilter *)compositingFilter {
   return _compositingFilter;
}

-(void)setCompositingFilter:(CIFilter *)filter {
   filter=[filter copy];
   [_compositingFilter release];
   _compositingFilter=filter;
}

-(NSShadow *)shadow {
   return _shadow;
}

-(void)setShadow:(NSShadow *)shadow {
   shadow=[shadow copy];
   [_shadow release];
   _shadow=shadow;
}

-(BOOL)needsDisplay {
   return _needsDisplay;
}

/*
   If _needsDisplay is YES and there are no _invalidRects, invalid rect is bounds
   If _needsDisplay is YES and there are _invalidRects, invalid rect is union
   You can't just keep a running invalid rect because setting YES then changing the
   bounds should redraw the new bounds, but changing the bounds should not alter the
   invalidated rects.
 */
 
 static NSRect unionOfInvalidRects(NSView *self){
   NSRect result;

   if(self->_invalidRectCount==0)
    result=[self visibleRect];
   else {
    int i;
    
    result=self->_invalidRects[0];
    
    for(i=1;i<self->_invalidRectCount;i++)
     result=NSUnionRect(result,self->_invalidRects[i]);
   }
   
   return result;
}

static void removeRectFromInvalidInVisibleRect(NSView *self,NSRect rect,NSRect visibleRect) {
   int count=self->_invalidRectCount;
	
   while(--count>=0){
    self->_invalidRects[count]=NSIntersectionRect(self->_invalidRects[count],visibleRect);
    
    if(NSContainsRect(rect,self->_invalidRects[count])){
     int i;
     
     self->_invalidRectCount--;
     for(i=count;i<self->_invalidRectCount;i++)
      self->_invalidRects[i]=self->_invalidRects[i+1];
    }
   }
	if(self->_invalidRectCount==0){
		if(self->_invalidRects!=NULL) {
			NSZoneFree(NULL,self->_invalidRects);
			self->_invalidRects=NULL;
			// We killed the last invalidRect - we're clean now
			self->_needsDisplay=NO;
		} else if (NSContainsRect(rect, visibleRect)) {
			// We had no invalidRect, which means the full visibleRect was dirty
			// We're now clean
			self->_needsDisplay=NO;
		}
	}
}

static void clearRectsBeingRedrawn(NSView *self){
	if (self->_rectsBeingRedrawn) {
		NSZoneFree(NULL, self->_rectsBeingRedrawn);
		self->_rectsBeingRedrawn = NULL;
		self->_rectsBeingRedrawnCount = 0;
	}
}

static void clearInvalidRects(NSView *self){
   if(self->_invalidRects!=NULL)
    NSZoneFree(NULL,self->_invalidRects);
   self->_invalidRects=NULL;
   self->_invalidRectCount=0;
	clearRectsBeingRedrawn(self);

}

static void clearNeedsDisplay(NSView *self){
	if ([NSGraphicsContext inQuartzDebugMode]) {
		return;
	}
   clearInvalidRects(self);
   self->_needsDisplay=NO;
}

-(void)setNeedsDisplay:(BOOL)flag {
   _needsDisplay=flag;

// We removed them for YES to indicate entire view, and NO for obvious reasons
   clearInvalidRects(self);
   
   if(_needsDisplay)
    [[self window] setViewsNeedDisplay:YES];
}

-(void)setNeedsDisplayInRect:(NSRect)rect {
// We only add rects if its not the entire view
   if(!_needsDisplay || _invalidRects!=NULL){
       NSRect visibleRect = [self visibleRect];
       rect = NSIntersectionRect(visibleRect, rect);
       if (NSContainsRect(rect, visibleRect)) {
           clearInvalidRects(self); // Everything is dirty
       } else {
           // All of our clipping done by the context is rounded - so we need to do the same
           // here else we might get some artifact on clipping borders
           rect = [self convertRect:rect toView:nil];
           rect = NSIntegralRect(rect);
           rect = [self convertRect:rect fromView:nil];
           _invalidRectCount++;
           _invalidRects=NSZoneRealloc(NULL,_invalidRects,sizeof(NSRect)*_invalidRectCount);
           _invalidRects[_invalidRectCount-1]=rect;
	   }
       clearRectsBeingRedrawn(self);
       
	   // We also needs to be sure all of our superviews will properly redraw this area,
	   // even if they are smart about what to redraw (using needsDisplayInRect:)
	   NSView *opaqueAncestor = [self opaqueAncestor];
	   if (opaqueAncestor != self) {
			NSRect dirtyRect = [self convertRect:rect toView:opaqueAncestor];
			[opaqueAncestor setNeedsDisplayInRect:dirtyRect];
       }
   }
   
   _needsDisplay=YES;
   [[self window] setViewsNeedDisplay:YES];
}

-(void)setKeyboardFocusRingNeedsDisplayInRect:(NSRect)rect {
   NSUnimplementedMethod();
}

-(void)translateRectsNeedingDisplayInRect:(NSRect)rect by:(NSSize)delta {
   NSUnimplementedMethod();
}

-(BOOL)canDraw {
   return _window!=nil && ![self isHiddenOrHasHiddenAncestor];
}

-(BOOL)canDrawConcurrently {
   NSUnimplementedMethod();
   return NO;
}

-(void)viewWillDraw {
    [_subviews makeObjectsPerformSelector:_cmd];
}

-(void)setCanDrawConcurrently:(BOOL)canDraw {
   NSUnimplementedMethod();
}

static NSGraphicsContext *graphicsContextForView(NSView *view){
   if(view->_layer!=nil){
    NSRect             frame=[view frame];
    size_t             width=frame.size.width;
    size_t             height=frame.size.height;
    CGColorSpaceRef    colorSpace=CGColorSpaceCreateDeviceRGB();
    CGContextRef       context=CGBitmapContextCreate(NULL,width,height,8,0,colorSpace,kCGImageAlphaPremultipliedFirst|kCGBitmapByteOrder32Host);
    NSGraphicsContext *result=[NSGraphicsContext graphicsContextWithGraphicsPort:context flipped:NO];
    
    CGColorSpaceRelease(colorSpace);
    CGContextRelease(context);
    
    return result;
   }
   
   return [[view window] graphicsContext];
}

-(void)_lockFocusInContext:(NSGraphicsContext *)context {
    CGContextRef graphicsPort=[context graphicsPort];

    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:context];
   
    [[context focusStack] addObject:self];

    CGContextSaveGState(graphicsPort);
    CGContextResetClip(graphicsPort);
    
    if(_layer!=nil)
     CGContextSetCTM(graphicsPort,[self transformToLayer]);
    else
     CGContextSetCTM(graphicsPort,[self transformToWindow]);
     
    CGContextClipToRect(graphicsPort,[self visibleRect]);

    [self setUpGState];
}

-(void)lockFocus {
   [self _lockFocusInContext:graphicsContextForView(self)];
}

-(BOOL)lockFocusIfCanDraw {
   if([self canDraw]){
    [self lockFocus];
    return YES;
   }
   return NO;
}

-(BOOL)lockFocusIfCanDrawInContext:(NSGraphicsContext *)context {
   if(context!=nil){
    [self _lockFocusInContext:context];
    return YES;
   }
   return NO;
}


-(void)unlockFocus {
   NSGraphicsContext *graphicsContext=[NSGraphicsContext currentContext];
   CGContextRef       context=[graphicsContext graphicsPort];
   
   if(_layer!=nil){
    CGImageRef image=CGBitmapContextCreateImage(context);
    
    [_layer setContents:image];
   }
   
   CGContextRestoreGState(context);

   [[graphicsContext focusStack] removeLastObject];
   [NSGraphicsContext restoreGraphicsState];
}

-(BOOL)needsToDrawRect:(NSRect)rect {
	BOOL needsToDrawRect = NO;

	if (NSIntersectsRect(rect, _visibleRect)) {
		const NSRect *rects;
		NSUInteger count;
		[self getRectsBeingDrawn:&rects count:&count];
		if (count) {
			for(int i=0; i<count && needsToDrawRect == NO;i++) {
				needsToDrawRect = NSIntersectsRect(rect, rects[i]);
			}		
		}
	}
	return needsToDrawRect;
}

-(void)getRectsBeingDrawn:(const NSRect **)rects count:(NSInteger *)count {
	// This method returns all the rects being drawn concerning the view
	// That's all of the dirty rects from the view, but also all the ones
	// from the superview that might have caused the redraw.
	// Since invalidating a rect also invalidates the first opaque superview,
	// only the opaque views need to be checked
	*rects = _rectsBeingRedrawn;
	*count = _rectsBeingRedrawnCount;
	
	if  (_rectsBeingRedrawn == NULL) {
		NSView *opaqueAncestor = [self opaqueAncestor];
		if (opaqueAncestor != self) {
			// Ask our opaque ancestor what to draw
			const NSRect *ancestorRects;
			[opaqueAncestor getRectsBeingDrawn:&ancestorRects count:&_rectsBeingRedrawnCount];
			if (_rectsBeingRedrawnCount) {
				_rectsBeingRedrawn = NSZoneCalloc(NULL, _rectsBeingRedrawnCount, sizeof(NSRect));
				int rectsCount = 0;
				for (int i = 0; i < _rectsBeingRedrawnCount; ++i) {
					NSRect r = [opaqueAncestor convertRect:ancestorRects[i] toView:self];
					// No need for the rects that are outside of the visibleRect
					if (NSIntersectsRect(r, _visibleRect)) {
						_rectsBeingRedrawn[rectsCount++] = r;
					}
				}
				*rects = _rectsBeingRedrawn;
				*count = rectsCount;
			}
		} else {
			// We're opaque - concatenate our invalid rect with the one from the previous opaque view
			NSView *view = [self superview];
			if (view) {
				NSView *opaqueAncestor = [view opaqueAncestor];
				const NSRect *ancestorRects;
				NSUInteger ancestorRectsCount;
				[opaqueAncestor getRectsBeingDrawn:&ancestorRects count:&ancestorRectsCount];
				if (ancestorRectsCount || _invalidRectCount) {
					_rectsBeingRedrawn = NSZoneCalloc(NULL, _invalidRectCount + ancestorRectsCount, sizeof(NSRect));
					int rectsCount = 0;
					for (int i = 0; i < ancestorRectsCount; ++i) {
						NSRect r = [opaqueAncestor convertRect:ancestorRects[i] toView:self];
						// No need for the rects that are outside of the visibleRect
						if (NSIntersectsRect(r, _visibleRect)) {
							_rectsBeingRedrawn[rectsCount++] = r;
						}
					}
					for (int i = 0; i < _invalidRectCount; ++i) {
						_rectsBeingRedrawn[rectsCount++] = _invalidRects[i];
					}
					_rectsBeingRedrawnCount = rectsCount;
					*rects = _rectsBeingRedrawn;
					*count = _rectsBeingRedrawnCount;
				}
			}
		}
	}
	// We had no info and no opaque ancestor gave us any useful rect - just use our invalid rects
	if (*rects == NULL) {
		if (_invalidRects == NULL) {
			if (_needsDisplay) {
				*rects = &_visibleRect;
				*count = 1;
			}
		} else {
			*rects = _invalidRects;
			*count = _invalidRectCount;
		}
	}		
}

-(void)getRectsExposedDuringLiveResize:(NSRect)rects count:(NSInteger *)count {
   NSUnimplementedMethod();
}

-(BOOL)shouldDrawColor {
   NSUnimplementedMethod();
   return YES;
}

-(NSView *)opaqueAncestor {
   if([self isOpaque])
    return self;

   return [[self superview] opaqueAncestor];
}

-(void)display {
   [self displayRect:[self visibleRect]];
}

- (NSEnumerator*)_subviewsInDisplayOrderEnumerator
{
	// Subviews are ordered back to front - 
	return [_subviews objectEnumerator];
}

-(void)_displayIfNeededWithoutViewWillDraw {
   if([self needsDisplay]){
    [self displayRect:unionOfInvalidRects(self)];
    clearNeedsDisplay(self);
   }

	NSEnumerator* viewEnumerator = [self _subviewsInDisplayOrderEnumerator];

	NSView* subView = nil;
	while ((subView = [viewEnumerator nextObject])) {
		[subView _displayIfNeededWithoutViewWillDraw];
	}
}

-(void)displayIfNeeded {
   [self viewWillDraw];
   [self _displayIfNeededWithoutViewWillDraw];
}

-(void)displayIfNeededInRect:(NSRect)rect {
   
   rect=NSIntersectionRect(unionOfInvalidRects(self), rect);

   if([self needsDisplay])
    [self displayRect:rect];

	NSEnumerator* viewEnumerator = [self _subviewsInDisplayOrderEnumerator];
	
	NSView* child = nil;
	while ((child = [viewEnumerator nextObject])) {
		NSRect converted=NSIntersectionRect([self convertRect:rect toView:child],[child bounds]);   
		if(!NSIsEmptyRect(converted)) {
		 [child displayIfNeededInRect:converted];
		}
	}
}

-(void)displayIfNeededInRectIgnoringOpacity:(NSRect)rect {
   
   rect=NSIntersectionRect(unionOfInvalidRects(self), rect);

   if([self needsDisplay])
    [self displayRectIgnoringOpacity:rect];

	NSEnumerator* viewEnumerator = [self _subviewsInDisplayOrderEnumerator];
	
	NSView* child = nil;
	while ((child = [viewEnumerator nextObject])) {
		NSRect  converted=NSIntersectionRect([self convertRect:rect toView:child],[child bounds]);
   
		if(!NSIsEmptyRect(converted)) {
			[child displayIfNeededInRectIgnoringOpacity:converted];
		}
   }
}

-(void)displayIfNeededIgnoringOpacity {

   if([self needsDisplay])
    [self displayRectIgnoringOpacity:unionOfInvalidRects(self)];

	NSEnumerator* viewEnumerator = [self _subviewsInDisplayOrderEnumerator];
	
	NSView* child = nil;
	while ((child = [viewEnumerator nextObject])) {
		[child displayIfNeededIgnoringOpacity];
	}
}

-(void)displayRect:(NSRect)rect {
   NSView *opaque=[self opaqueAncestor];

   if(opaque!=self)
    rect=[self convertRect:rect toView:opaque];
   [opaque displayRectIgnoringOpacity:rect];
}

-(void)displayRectIgnoringOpacity:(NSRect)rect {   
   NSRect visibleRect=[self visibleRect];

   rect=NSIntersectionRect(rect,visibleRect);

   if(NSIsEmptyRect(rect))
    return;
    
   if ([self canDraw]) {
      // This view must be locked/unlocked prior to drawing subviews otherwise gState changes may affect subviews.
      [self lockFocus];

      NSGraphicsContext *context=[NSGraphicsContext currentContext];
      CGContextRef       graphicsPort=[context graphicsPort];

	   CGContextClipToRect(graphicsPort,rect);
	   
	   const NSRect *rects;
	   NSUInteger rectsCount;
	   [self getRectsBeingDrawn:&rects count:&rectsCount];
	   // If there is only one rect, it's the visible rect - it's already clipped
	   if (rectsCount > 1)
 		   CGContextClipToRects(graphicsPort, rects, rectsCount);

      // [_window dirtyRect:[self convertRect:rect toView:nil]];
	   if ([NSGraphicsContext inQuartzDebugMode]) {
		   [[NSColor yellowColor] set];
		   NSRectFill(rect);
	   }
      else
		   [self drawRect:rect];

      [self unlockFocus];

	   NSEnumerator *viewEnumerator = [self _subviewsInDisplayOrderEnumerator];
	   
	   NSView *child = nil;
	   while ((child = [viewEnumerator nextObject])) {
		   NSRect check=[self convertRect:rect toView:child];

		   check=NSIntersectionRect(check,[child bounds]);
		   
		   if(!NSIsEmptyRect(check))
			   [child displayRectIgnoringOpacity:check];
	   }
   }

   [_layerContext render];

	// Don't do anything to interfere with what will be drawn in non-debug mode
	if ([NSGraphicsContext inQuartzDebugMode] == NO) {
		removeRectFromInvalidInVisibleRect(self,rect,visibleRect);

		// Rects being drawn are only valid while we redraw
		clearRectsBeingRedrawn(self);
	}

   // We do the flushWindow here. If any of the display* methods are being used, you want it to update on screen immediately.
   // If the view hierarchy is being displayed as needed at the end of an event, flushing will be disabled and this will just
   // mark the window as needing flushing which will happen when all the views have finished being displayed
   [[self window] flushWindow];
}

-(void)displayRectIgnoringOpacity:(NSRect)rect inContext:(NSGraphicsContext *)context {   
   NSUnimplementedMethod();
}

-(void)drawRect:(NSRect)rect {
   // do nothing
}

-(BOOL)autoscroll:(NSEvent *)event {
   return [[self superview] autoscroll:event];
}

-(void)scrollRect:(NSRect)rect by:(NSSize)delta {
   NSPoint point=rect.origin;

   point.x+=delta.width;
   point.y+=delta.height;

   if([self lockFocusIfCanDraw]){
    NSCopyBits([self gState],rect,point);
    [self unlockFocus];
   }
}

-(BOOL)mouseDownCanMoveWindow {
   NSUnimplementedMethod();
   return NO;
}

-(void)print:sender {
   [[NSPrintOperation printOperationWithView:self] runOperation];
}

-(void)beginDocument {
}

-(void)endDocument {
}

-(void)beginPageInRect:(NSRect)rect atPlacement:(NSPoint)placement {
   CGContextRef      graphicsPort=NSCurrentGraphicsPort();
   CGRect            mediaBox=NSMakeRect(0,0,rect.size.width,rect.size.height);
   NSPrintInfo      *printInfo=[[NSPrintOperation currentOperation] printInfo];
   NSRect            imageableRect=[printInfo imageablePageBounds];
   CGAffineTransform transform=CGAffineTransformIdentity;

   [NSCurrentFocusStack() addObject:self];

	CGContextBeginPage(graphicsPort,&mediaBox);
	CGContextSaveGState(graphicsPort);

   transform=CGAffineTransformIdentity;
   if([self isFlipped]){
	   transform=CGAffineTransformMake(1,0,0,-1,0,[printInfo paperSize].height);
   }

   transform=CGAffineTransformTranslate(transform,-rect.origin.x,-rect.origin.y);

   CGContextConcatCTM(graphicsPort,transform);

   [self setUpGState];
}

-(void)endPage {
   CGContextRef graphicsPort=NSCurrentGraphicsPort();

   CGContextRestoreGState(graphicsPort);
   CGContextEndPage(graphicsPort);
   [NSCurrentFocusStack() removeLastObject];
}

-(NSAttributedString *)pageHeader {
   NSUnimplementedMethod();
   return nil;
}

-(NSAttributedString *)pageFooter {
   NSUnimplementedMethod();
   return nil;
}

-(NSString *)printJobTitle {
   NSUnimplementedMethod();
   return nil;
}

-(void)drawSheetBorderWithSize:(NSSize)size {
   NSUnimplementedMethod();
}

-(void)drawPageBorderWithSize:(NSSize)size {
   NSUnimplementedMethod();
}


-(float)widthAdjustLimit {
   return 0.5;
}

-(float)heightAdjustLimit {
   return 0.5;
}

-(void)adjustPageWidthNew:(float *)adjusted left:(float)left right:(float)right limit:(float)limit {
// FIX, give subviews a chance
}

-(void)adjustPageHeightNew:(float *)adjust top:(float)top bottom:(float)bottom limit:(float)limit {
// FIX, give subviews a chance
}

-(BOOL)knowsPageRange:(NSRange *)range {
   return NO;
}

-(NSPoint)locationOfPrintRect:(NSRect)rect {
   return NSZeroPoint;
}

-(NSRect)rectForPage:(int)page {
   return NSZeroRect;
}

-(NSData *)dataWithEPSInsideRect:(NSRect)rect {
   NSMutableData    *result=[NSMutableData data];
   NSPrintOperation *operation=[NSPrintOperation EPSOperationWithView:self insideRect:rect toData:result];
   
   [operation runOperation];
   
   return result;
}

-(NSData *)dataWithPDFInsideRect:(NSRect)rect {
   NSMutableData    *result=[NSMutableData data];
   NSPrintOperation *operation=[NSPrintOperation PDFOperationWithView:self insideRect:rect toData:result];
   
   [operation runOperation];
   
   return result;
}

-(void)writeEPSInsideRect:(NSRect)rect toPasteboard:(NSPasteboard *)pasteboard {
   NSData *data=[self dataWithEPSInsideRect:rect];
   
   [pasteboard declareTypes:[NSArray arrayWithObject:NSPostScriptPboardType] owner:nil];
   [pasteboard setData:data forType:NSPostScriptPboardType];
}

-(void)writePDFInsideRect:(NSRect)rect toPasteboard:(NSPasteboard *)pasteboard {
   NSData *data=[self dataWithPDFInsideRect:rect];

   [pasteboard declareTypes:[NSArray arrayWithObject:NSPDFPboardType] owner:nil];
   [pasteboard setData:data forType:NSPDFPboardType];
}

-(void)dragImage:(NSImage *)image at:(NSPoint)location offset:(NSSize)offset event:(NSEvent *)event pasteboard:(NSPasteboard *)pasteboard source:source slideBack:(BOOL)slideBack {
	location = [self convertPoint:location toView:nil];
   [[NSDraggingManager draggingManager] dragImage:image at:location offset:offset event:event pasteboard:pasteboard source:source slideBack:slideBack];
}

-(BOOL)dragFile:(NSString *)path fromRect:(NSRect)rect slideBack:(BOOL)slideBack event:(NSEvent *)event {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)acceptsFirstResponder {
   return NO;
}

-(void)scrollWheel:(NSEvent *)event {
    NSScrollView *scrollView=[self enclosingScrollView];
    
    if(scrollView==nil) {
        /* If we can't handle it, pass up responder chain, yep, it does this. */
        [super scrollWheel:event];
    }
    else {
        NSView *documentView=[scrollView documentView];
        NSRect bounds=[documentView bounds];
        NSRect visible=[documentView visibleRect];
        float  direction=[documentView isFlipped]?-1:1;
        
        visible.origin.x+=[event deltaX]*[scrollView horizontalLineScroll]*3;
        visible.origin.y+=[event deltaY]*direction*[scrollView verticalLineScroll]*3;
        
        // Something equivalent to this should be in scrollRectToVisible:
        if(visible.origin.y<bounds.origin.y)
            visible.origin.y=bounds.origin.y;
        if(visible.origin.x<bounds.origin.x)
            visible.origin.x=bounds.origin.x;
        if(NSMaxY(visible)>NSMaxY(bounds))
            visible.origin.y=NSMaxY(bounds)-visible.size.height;
        if(NSMaxX(visible)>NSMaxX(bounds))
            visible.origin.x=NSMaxX(bounds)-visible.size.width;
        
        [documentView scrollRectToVisible:visible];
    }
}

-(BOOL)performKeyEquivalent:(NSEvent *)event {
   int i,count=[_subviews count];

   for(i=0;i<count;i++){
    NSView *check=[_subviews objectAtIndex:i];

    if([check performKeyEquivalent:event])
     return YES;
   }
   return NO;
}

-(BOOL)performMnemonic:(NSString *)string {
   NSUnimplementedMethod();
   return NO;
}

-(void)setMenu:(NSMenu *)menu {
   menu=[menu retain];
   [_menu release];
   _menu=menu;
}

-(void)rightMouseDown:(NSEvent *)event {
   [NSMenu popUpContextMenu:[self menuForEvent:event] withEvent:event forView:self];
}


// default NSDraggingDestination
-(NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
   return NSDragOperationNone;
}

-(NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender {
   return [sender draggingSourceOperationMask];
}

-(void)draggingExited:(id <NSDraggingInfo>)sender {
   // do nothing
}

-(BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender {
   return NO;
}

-(BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
   return NO;
}

-(void)concludeDragOperation:(id <NSDraggingInfo>)sender {
   // do nothing
}

-(NSArray *)_draggedTypes {
   return _draggedTypes;
}

- (BOOL)dragPromisedFilesOfTypes:(NSArray *)types fromRect:(NSRect)rect source:(id)source slideBack:(BOOL)slideBack event:(NSEvent *)event {
   NSUnimplementedMethod();
   return NO;
}

-(NSPoint)convertPointFromBase:(NSPoint)aPoint; {
   return aPoint;
}

-(NSPoint)convertPointToBase:(NSPoint)aPoint; {
   return aPoint;
}

-(NSSize)convertSizeFromBase:(NSSize)aSize {
   return aSize;
}

-(NSSize)convertSizeToBase:(NSSize)aSize {
   return aSize;
}

-(NSRect)convertRectFromBase:(NSRect)aRect {
   return aRect;
}

-(NSRect)convertRectToBase:(NSRect)aRect {
   return aRect;
}

-(void)showDefinitionForAttributedString:(NSAttributedString *)string atPoint:(NSPoint)origin {
   NSUnimplementedMethod();
}

+defaultAnimationForKey:(NSString *)key {
   NSUnimplementedMethod();
   return nil;
}

-animator {
   NSUnimplementedMethod();
    // should return animating proxy. returning self does not animate.
   return self;
}

-(NSDictionary *)animations {
   return _animations;
}

-animationForKey:(NSString *)key {
   return [_animations objectForKey:key];
}

-(void)setAnimations:(NSDictionary *)dictionary {
   dictionary=[dictionary copy];
   [_animations release];
   _animations=dictionary;
}

// Blocks aren't supported by the compiler yet.
//-(void)showDefinitionForAttributedString:(NSAttributedString *)string range:(NSRange)range options:(NSDictionary *)options baselineOriginProvider:(NSPoint (^)(NSRange adjustedRange))originProvider {
//   NSUnimplementedMethod();
//}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@[0x%lx] frame: %@>", [self class], self, NSStringFromRect(_frame)];
}

@end
