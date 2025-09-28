/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSScrollView.h>
#import <AppKit/NSScroller.h>
#import <AppKit/NSClipView.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSRulerView.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSRaise.h>

@implementation NSScrollView

static Class _rulerViewClass = nil;

+(void)initialize
{
    if (self == [NSScrollView class]) {
        _rulerViewClass = [NSRulerView class];
    }
}

+(NSSize)frameSizeForContentSize:(NSSize)contentSize hasHorizontalScroller:(BOOL)hasHorizontalScroller hasVerticalScroller:(BOOL)hasVerticalScroller borderType:(NSBorderType)borderType {

   if(hasHorizontalScroller)
    contentSize.height+=[NSScroller scrollerWidth];
   if(hasVerticalScroller)
    contentSize.width+=[NSScroller scrollerWidth];

   switch(borderType){
    case NSNoBorder:
     break;

    case NSLineBorder:
     contentSize.height+=1;
     contentSize.width+=1;
     break;

    case NSBezelBorder:
     contentSize.height+=2;
     contentSize.width+=2;
     break;

    case NSGrooveBorder:
     contentSize.height+=2;
     contentSize.width+=2;
     break;
   }

   return contentSize;
}

+(NSSize)contentSizeForFrameSize:(NSSize)frameSize hasHorizontalScroller:(BOOL)hasHorizontalScroller hasVerticalScroller:(BOOL)hasVerticalScroller borderType:(NSBorderType)borderType {

   if(hasHorizontalScroller)
    frameSize.height-=[NSScroller scrollerWidth];
   if(hasVerticalScroller)
    frameSize.width-=[NSScroller scrollerWidth];

   switch(borderType){
    case NSNoBorder:
     break;

    case NSLineBorder:
     frameSize.height-=1;
     frameSize.width-=1;
     break;

    case NSBezelBorder:
     frameSize.height-=2;
     frameSize.width-=2;
     break;

    case NSGrooveBorder:
     frameSize.height-=2;
     frameSize.width-=2;
     break;
   }

   return frameSize;
}

+(void)setRulerViewClass:(Class)class
{
    _rulerViewClass = class;
}

+(Class)rulerViewClass
{
    return _rulerViewClass;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    unsigned           flags=[keyed decodeIntForKey:@"NSsFlags"];
    
    _hasVerticalScroller=(flags&0x10)?YES:NO;
    _hasHorizontalScroller=(flags&0x20)?YES:NO;
    _autohidesScrollers=(flags&0x200)?YES:NO;
    _borderType=flags&0x03;
    _clipView=[[keyed decodeObjectForKey:@"NSContentView"] retain];
    _verticalScroller=[[keyed decodeObjectForKey:@"NSVScroller"] retain];
    _horizontalScroller=[[keyed decodeObjectForKey:@"NSHScroller"] retain];
    [_verticalScroller setTarget:self];
    [_verticalScroller setAction:@selector(_verticalScroll:)];
    [_horizontalScroller setTarget:self];
    [_horizontalScroller setAction:@selector(_horizontalScroll:)];
    _headerClipView=[[keyed decodeObjectForKey:@"NSHeaderClipView"] retain];
    _cornerView=[[keyed decodeObjectForKey:@"NSCornerView"] retain];
    _backgroundColor=[[NSColor controlBackgroundColor] copy];
    
    BOOL copyOnScroll=(flags&0x80)?YES:NO;
    if (copyOnScroll) {
        _drawsBackground = YES;
        [_clipView setDrawsBackground:YES];
        [_clipView setCopiesOnScroll:YES];
    } else {
        _drawsBackground = [_clipView drawsBackground];
    }

    _verticalLineScroll=10.0;  // the default value in IB is 10
    _verticalPageScroll=10.0;
    _horizontalLineScroll=10.0;
    _verticalLineScroll=10.0;
    id scrollAmts = [keyed decodeObjectForKey:@"NSScrollAmts"];
    if ([scrollAmts isKindOfClass:[NSData class]] && [scrollAmts length] == 4*sizeof(NSSwappedFloat)) {
        NSSwappedFloat *amts = (NSSwappedFloat *)[scrollAmts bytes];
        _horizontalPageScroll = NSSwapBigFloatToHost(amts[0]);
        _verticalPageScroll = NSSwapBigFloatToHost(amts[1]);
        _horizontalLineScroll = NSSwapBigFloatToHost(amts[2]);
        _verticalLineScroll = NSSwapBigFloatToHost(amts[3]);
    }
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }

   return self;
}

-(void)postAwakeFromNib {
   if(!_hasVerticalScroller)
    [_verticalScroller removeFromSuperview];
   if(!_hasHorizontalScroller)
    [_horizontalScroller removeFromSuperview];
}

-(NSRect)insetBounds {
   NSRect bounds=[self bounds];

   switch(_borderType){
    case NSNoBorder:
     break;

    case NSLineBorder:
     bounds=NSInsetRect(bounds,1,1);
     break;

    case NSBezelBorder:
     bounds=NSInsetRect(bounds,1,1);
     break;

    case NSGrooveBorder:
     bounds=NSInsetRect(bounds,2,2);
     break;
   }

   return bounds;
}

-(NSView *)_headerView {
   NSView *document=[self documentView];

   if([document respondsToSelector:@selector(headerView)])
    return [document performSelector:@selector(headerView)];

   return nil;
}

-(NSView *)_cornerView {
   id document=[self documentView];

   if([document respondsToSelector:@selector(cornerView)])
    return [document performSelector:@selector(cornerView)];

   return nil;
}

-(NSRect)headerClipViewFrame {
    NSView *headerView=[self _headerView];

    if (headerView == nil)
        return NSZeroRect;

    NSRect  result=[self insetBounds];
    result.size.height=[headerView bounds].size.height;
    result.size.width-=[NSScroller scrollerWidth];

    return result;
}

-(NSRect)cornerViewFrame {
    NSView *headerView = [self _headerView];

    if (headerView == nil)
        return NSZeroRect;

    NSRect bounds = [self insetBounds];
    NSRect frame;
    frame.origin.x = bounds.origin.x+bounds.size.width-[NSScroller scrollerWidth];
    frame.origin.y = bounds.origin.y;
    frame.size.width = [NSScroller scrollerWidth];
    frame.size.height = [headerView bounds].size.height;

    return frame;
}

-(NSRect)horizontalRulerFrame {
    NSRect result=[self insetBounds];
    
    result.size.height = [_horizontalRuler requiredThickness];

    // The horizontal ruler spans the entire width of the scrollview
    
    return result;
}

-(NSRect)verticalRulerFrame {
    NSRect result=[self insetBounds];
    
    result.size.width = [_verticalRuler requiredThickness];
    if ([self rulersVisible] && [self hasHorizontalRuler]) {
        // The vertical ruler is positioned below the horizontal ruler
        result.origin.y += [_horizontalRuler requiredThickness];
        result.size.height -= [_horizontalRuler requiredThickness];
    }
    
    return result;
}

-(NSRect)clipViewFrame {
   NSRect bounds=[self insetBounds];
   NSRect result;

   result.origin.x=bounds.origin.x;
   result.origin.y=bounds.origin.y;

   result.size.width=bounds.size.width;
   if([self hasVerticalScroller] && ![_verticalScroller isHidden])
    result.size.width-=[NSScroller scrollerWidth];
   if([self rulersVisible] && [self hasVerticalRuler]) {
    result.origin.x+=[self verticalRulerFrame].size.width;
    result.size.width-=[self verticalRulerFrame].size.width;
   }

   result.size.height=bounds.size.height;
   if([self hasHorizontalScroller] && ![_horizontalScroller isHidden]){
    result.size.height-=[NSScroller scrollerWidth];
   }
   if([self rulersVisible] && [self hasHorizontalRuler]) {
    result.origin.y+=[self horizontalRulerFrame].size.height;
    result.size.height-=[self horizontalRulerFrame].size.height;
   }

   if ([self _headerView] != nil) {
       result.origin.y+=[self headerClipViewFrame].size.height;
       result.size.height-=[self headerClipViewFrame].size.height;
   }

   return result;
}

-(NSRect)verticalScrollerFrame {
   NSRect bounds=[self insetBounds];
   NSRect result;

   result.origin.x=bounds.origin.x+bounds.size.width-[NSScroller scrollerWidth];
   result.origin.y=bounds.origin.y;
   result.size.width=[NSScroller scrollerWidth];
   result.size.height=bounds.size.height;
    if([self hasHorizontalScroller] && ![_horizontalScroller isHidden])
        result.size.height-=[NSScroller scrollerWidth];

    if ([self _headerView] != nil) {
       result.origin.y+=[self headerClipViewFrame].size.height;
       result.size.height-=[self headerClipViewFrame].size.height;
   }
    if([self rulersVisible] && [self hasHorizontalRuler]) {
       result.origin.y+=[_horizontalRuler requiredThickness];
       result.size.height-=[_horizontalRuler requiredThickness];
   }

   return result;
}

-(NSRect)horizontalScrollerFrame {
   NSRect bounds=[self insetBounds];
   NSRect result;

   result.origin.x=bounds.origin.x;
   result.origin.y=(bounds.origin.y+bounds.size.height)-[NSScroller scrollerWidth];
   result.size.width=bounds.size.width;
   if([self hasVerticalScroller] && ![_verticalScroller isHidden])
    result.size.width-=[NSScroller scrollerWidth];

   result.size.height=[NSScroller scrollerWidth];

    if([self rulersVisible] && [self hasVerticalRuler]) {
       result.origin.x+=[_verticalRuler requiredThickness];
       result.size.width-=[_verticalRuler requiredThickness];
   }

   return result;
}

-(void)createVerticalScrollerIfNeeded {
   if(_verticalScroller==nil){
    _verticalScroller=[[NSScroller alloc] initWithFrame:[self verticalScrollerFrame]];
    [_verticalScroller setAutoresizingMask: NSViewMinXMargin |NSViewHeightSizable];
    [_verticalScroller setTarget:self];
    [_verticalScroller setAction:@selector(_verticalScroll:)];
   }
}

-(void)createHorizontalScrollerIfNeeded {
   if(_horizontalScroller==nil){
    _horizontalScroller=[[NSScroller alloc] initWithFrame:[self horizontalScrollerFrame]];
    [_horizontalScroller setAutoresizingMask: NSViewMaxYMargin|NSViewWidthSizable];
    [_horizontalScroller setTarget:self];
    [_horizontalScroller setAction:@selector(_horizontalScroll:)];
   }
}

-initWithFrame:(NSRect)frame {
   [super initWithFrame:frame];

   _clipView=[[NSClipView alloc] initWithFrame:[self clipViewFrame]];
   [_clipView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];

   _hasVerticalScroller=NO;
   _hasHorizontalScroller=NO;
   _drawsBackground=YES;
   _borderType=NSNoBorder;
   _backgroundColor=[[NSColor controlBackgroundColor] copy];

   [self setLineScroll:1.0];
   [self setPageScroll:10.0];		//entirely arbitrary
   [self setAutoresizesSubviews:YES];

   [self addSubview:_clipView];

   return self;
}

-(void)dealloc {
   [_clipView release];
   [_verticalScroller release];
   [_horizontalScroller release];
   [_headerClipView release];
   [_cornerView release];
   [_horizontalRuler release];
   [_verticalRuler release];
   [_backgroundColor release];
   [super dealloc];
}

-(BOOL)isOpaque {
   return _drawsBackground;
}

-(BOOL)isFlipped {
   return YES;
}

-(NSSize)contentSize {
   return [_clipView frame].size;
}

-(void)createHeaderAndCornerViewsIfNeeded {

   if([self _headerView]==nil){
    [_headerClipView removeFromSuperview];
    [_headerClipView autorelease];
    _headerClipView=nil;
   }
   else if([self _headerView]!=nil && _headerClipView==nil) {
    _headerClipView = [[NSClipView alloc] initWithFrame:[self headerClipViewFrame]];
    [_headerClipView setDocumentView:[self _headerView]];

    [self addSubview:_headerClipView];
    [_headerClipView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
    [_headerClipView setAutoresizesSubviews:YES];
   }

    if([self _cornerView]==nil){
        [_cornerView removeFromSuperview];
        [_cornerView release];
        _cornerView = nil;
    }
    else if ([self _cornerView] != nil && _cornerView == nil) {
        // Use the document corner view - and it's a retained property
        _cornerView = [[self _cornerView] retain];
        [self addSubview:_cornerView];
    }
}    

-(void)createRulerViewsIfNeeded {
    if ([_horizontalRuler superview] != nil)
        [_horizontalRuler removeFromSuperview];
    if ([_verticalRuler superview] != nil)
        [_verticalRuler removeFromSuperview];

    if (_rulersVisible) {
        if (_hasHorizontalRuler) {
            if (_horizontalRuler == nil)
                _horizontalRuler = [[[[self class] rulerViewClass] alloc] initWithScrollView:self orientation:NSHorizontalRuler];

            [self addSubview:_horizontalRuler];
        }
        if (_hasVerticalRuler) {
            if (_verticalRuler == nil)
                _verticalRuler = [[[[self class] rulerViewClass] alloc] initWithScrollView:self orientation:NSVerticalRuler];
            
            [self addSubview:_verticalRuler];
        }
    }
}

-documentView {
   return [_clipView documentView];
}

-(NSClipView *)contentView {
   return _clipView;
}

-(NSRect)documentVisibleRect {
    return [_clipView documentVisibleRect];
}

-(BOOL)drawsBackground {
   return [_clipView drawsBackground];
}

-(NSColor *)backgroundColor {
   return [_clipView backgroundColor];
}

-(NSBorderType)borderType {
   return _borderType;
}

-(NSScroller *)verticalScroller {
   return _verticalScroller;
}

-(NSScroller *)horizontalScroller {
   return _horizontalScroller;
}

-(NSRulerView *)verticalRulerView
{
    return _verticalRuler;
}

- (void)setVerticalRulerView:(NSRulerView *)ruler
{
    ruler = [ruler retain];
    if (_verticalRuler) {
        [_verticalRuler removeFromSuperview];
        [_verticalRuler release];
    }
    _verticalRuler = ruler;
    if (_verticalRuler) {
        [_verticalRuler setScrollView:self];
        [_verticalRuler setOrientation:NSVerticalRuler];
        [self addSubview:_verticalRuler];
    }
    _hasVerticalRuler = _verticalRuler != nil;
    [self tile];
}

- (void)setHorizontalRulerView:(NSRulerView *)ruler
{
    ruler = [ruler retain];
    if (_horizontalRuler) {
        [_horizontalRuler removeFromSuperview];
        [_horizontalRuler release];
    }
    _horizontalRuler = ruler;
    if (_horizontalRuler) {
        [_horizontalRuler setScrollView:self];
        [_horizontalRuler setOrientation:NSHorizontalRuler];
        [self addSubview:_horizontalRuler];
    }
    _hasHorizontalRuler = _horizontalRuler != nil;
    [self tile];
}

-(NSRulerView *)horizontalRulerView
{
    return _horizontalRuler;
}

-(BOOL)hasVerticalScroller {
   return _hasVerticalScroller;
}

-(BOOL)hasHorizontalScroller {
   return _hasHorizontalScroller;
}

-(BOOL)hasVerticalRuler
{
    return _hasVerticalRuler;
}

-(BOOL)hasHorizontalRuler
{
    return _hasHorizontalRuler;
}

-(BOOL)rulersVisible
{
    return _rulersVisible;
}

-(float)verticalLineScroll {
    return _verticalLineScroll;
}

-(float)horizontalLineScroll {
    return _horizontalLineScroll;
}

-(float)verticalPageScroll {
    return _verticalPageScroll;
}

-(float)horizontalPageScroll {
    return _horizontalPageScroll;
}

-(float)lineScroll {
    return [self verticalLineScroll];
}

-(float)pageScroll {
    return [self verticalPageScroll];
}

-(BOOL)scrollsDynamically {
    return _scrollsDynamically;
}

-(BOOL)autohidesScrollers {
   return _autohidesScrollers;
}

-(NSCursor *)documentCursor {
   return _documentCursor;
}

-(void)setDocumentView:(NSView *)view {
   [_clipView setDocumentView:view];
   [self reflectScrolledClipView:_clipView];
}

-(void)setContentView:(NSClipView *)clipView {
   [_clipView removeFromSuperview];
   [_clipView autorelease];

   _clipView=[clipView retain];
   [self addSubview:_clipView];
   [_clipView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
   [_clipView setAutoresizesSubviews:YES];
   [self tile];
}

-(void)setDrawsBackground:(BOOL)value {
   _drawsBackground=value;
   if(!_drawsBackground)
    [_clipView setCopiesOnScroll:NO];
}

-(void)setBackgroundColor:(NSColor *)color {
   [_clipView setBackgroundColor:color];
}

-(void)setBorderType:(NSBorderType)borderType {
   if(_borderType!=borderType){
    _borderType=borderType;
    [self tile];
   }
}

-(void)setVerticalScroller:(NSScroller *)scroller {
   scroller=[scroller retain];
   [_verticalScroller removeFromSuperview];
   [_verticalScroller release];
   _verticalScroller=scroller;
   [_verticalScroller setTarget:self];
   [_verticalScroller setAction:@selector(_verticalScroll:)];

   if(_hasVerticalScroller)
    [self addSubview:_verticalScroller];
   
   [self tile];
}

-(void)setHorizontalScroller:(NSScroller *)scroller {
   scroller=[scroller retain];
   [_horizontalScroller removeFromSuperview];
   [_horizontalScroller release];
   _horizontalScroller=scroller;
   [_horizontalScroller setTarget:self];
   [_horizontalScroller setAction:@selector(_horizontalScroll:)];

   if(_hasHorizontalScroller)
    [self addSubview:_horizontalScroller];
    
   [self tile];
}

-(void)setHasVerticalScroller:(BOOL)flag {
   if(flag){
    if(!_hasVerticalScroller){
     _hasVerticalScroller=flag;
     [self createVerticalScrollerIfNeeded];
     [self addSubview:_verticalScroller];
     [self tile];
    }
   }
   else {
    if(_hasVerticalScroller){
     _hasVerticalScroller=flag;
     [_verticalScroller removeFromSuperview];
     [self tile];
    }
   }
}

-(void)setHasHorizontalScroller:(BOOL)flag {
   if(flag){
    if(!_hasHorizontalScroller){
     _hasHorizontalScroller=flag;
     [self createHorizontalScrollerIfNeeded];
     [self addSubview:_horizontalScroller];
     [self tile];
    }
   }
   else {
    if(_hasHorizontalScroller){
     _hasHorizontalScroller=flag;
     [_horizontalScroller removeFromSuperview];
     [self tile];
    }
   }
}

-(void)setHasVerticalRuler:(BOOL)flag
{
    if (_hasVerticalRuler != flag) {
        _hasVerticalRuler = flag;
        [self tile];
        [_verticalRuler setNeedsDisplay:flag];
    }
}

-(void)setHasHorizontalRuler:(BOOL)flag
{
    if (_hasHorizontalRuler != flag) {
        _hasHorizontalRuler = flag;
        [self tile];
        [_horizontalRuler setNeedsDisplay:flag];
    }
}

-(void)setRulersVisible:(BOOL)flag
{
    if (_rulersVisible != flag) {
        _rulersVisible = flag;
        [self tile];
    }
}

-(void)setVerticalLineScroll:(float)value {
    if (value > 0.0)
        _verticalLineScroll = value;
}

-(void)setHorizontalLineScroll:(float)value {
    if (value > 0.0)
        _horizontalLineScroll = value;
}

-(void)setVerticalPageScroll:(float)value {
    if (value > 0.0)
        _verticalPageScroll = value;
}

-(void)setHorizontalPageScroll:(float)value {
    if (value > 0.0)
        _horizontalPageScroll = value;
}

-(void)setLineScroll:(float)value {
    [self setHorizontalLineScroll:value];
    [self setVerticalLineScroll:value];
}

-(void)setPageScroll:(float)value {
    [self setHorizontalPageScroll:value];
    [self setVerticalPageScroll:value];
}

-(void)setScrollsDynamically:(BOOL)flag {
    _scrollsDynamically = flag;
}

-(void)setDocumentCursor:(NSCursor *)cursor {
   [_clipView discardCursorRects];
   [_documentCursor release];
   _documentCursor=[cursor retain];
   if(_documentCursor!=nil)
    [_clipView addCursorRect:[_clipView bounds] cursor:_documentCursor];
}

-(void)setAutohidesScrollers:(BOOL)value {
   _autohidesScrollers=value;
// FIXME: tile or hide/show scrollers?
}

-(void)tile {
   NSRect frame;

   [self createHeaderAndCornerViewsIfNeeded];
   [self createRulerViewsIfNeeded];

   frame=[self headerClipViewFrame];
   [_headerClipView setFrame:frame];

   frame=[self cornerViewFrame];
   [_cornerView setFrame:frame];

    frame = [self verticalScrollerFrame];
   [_verticalScroller setFrame:frame];

    frame = [self horizontalScrollerFrame];
   [_horizontalScroller setFrame:frame];
    
    frame = [self clipViewFrame];
   [_clipView setFrame:frame];

   frame=[self horizontalRulerFrame];
   [_horizontalRuler setFrame:frame];
   
   frame=[self verticalRulerFrame];
   [_verticalRuler setFrame:frame];

   frame=[_clipView bounds];
   if(![self hasVerticalScroller])
    frame.origin.y=0;
   if(![self hasHorizontalScroller])
    frame.origin.x=0;
   [_clipView setBoundsOrigin:frame.origin];

   [self reflectScrolledClipView:_clipView];
   [_clipView discardCursorRects];
   if(_documentCursor!=nil)
    [_clipView addCursorRect:[_clipView bounds] cursor:_documentCursor];
}

-(void)reflectScrolledClipView:(NSClipView *)clipView {

   if(_clipView==clipView){
    NSView *docView=[self documentView];
    NSRect headerClipRect=[_headerClipView frame];

    if(docView==nil){
     [_verticalScroller setEnabled:NO];
     [_verticalScroller setHidden:_autohidesScrollers];
     [_horizontalScroller setEnabled:NO];
     [_horizontalScroller setHidden:_autohidesScrollers];
    }
    else {
     NSRect docRect=[docView frame];
     NSRect clipRect=[_clipView bounds];
     float  heightDiff=docRect.size.height-clipRect.size.height;
     float  widthDiff=docRect.size.width-clipRect.size.width;

     if(heightDiff<=0){
      [_verticalScroller setEnabled:NO];
      [_verticalScroller setHidden:_autohidesScrollers];
     }
     else {
      float  value=(heightDiff<=0)?0:(clipRect.origin.y-docRect.origin.y)/heightDiff;

      if(![docView isFlipped])
       value=1.0-value;

      [_verticalScroller setEnabled:YES];
      [_verticalScroller setHidden:NO];
      [_verticalScroller setFloatValue:value knobProportion:clipRect.size.height/docRect.size.height];
     }

     if(widthDiff<=0){
      [_horizontalScroller setEnabled:NO];
      [_horizontalScroller setHidden:_autohidesScrollers];
     }
     else {
      float value=(widthDiff<=0)?0:(clipRect.origin.x-docRect.origin.x)/widthDiff;

      [_horizontalScroller setEnabled:YES];
      [_horizontalScroller setHidden:NO];
      [_horizontalScroller setFloatValue:value knobProportion:clipRect.size.width/docRect.size.width];
     }
    }

    // Can't do sublayout in here because it messes with the tile method
       
    [_horizontalRuler invalidateHashMarks];
    [_verticalRuler invalidateHashMarks];
       
    // keep the header in line with the document
    // using scrollToPoint: ran into some ordering issues, since scrollToPoint calls
    // constrainScrollPoint *and* this method.
    headerClipRect.origin.x = [_clipView frame].origin.x;
    headerClipRect.size.width = [_clipView frame].size.width;
    [_headerClipView setFrame:headerClipRect];
    [_headerClipView setNeedsDisplay:YES];
   }
}

-(void)drawRect:(NSRect)rect {

   if(_drawsBackground){
    [[self backgroundColor] setFill];
    NSRectFill(rect);
   }
   
   switch(_borderType){
    case NSNoBorder:
     break;

    case NSLineBorder:
     [[NSColor blackColor] set];
     NSFrameRect(_bounds);
     break;

    case NSBezelBorder:
     NSDrawGrayBezel(_bounds,rect);
     break;

    case NSGrooveBorder:
     NSDrawGroove(_bounds,rect);
     break;
   }
}

-(void)_verticalScroll:(NSScroller *)scroller {
   float   value=[scroller floatValue];
   NSView *docView=[self documentView];
   NSRect  docRect=[docView frame];
   NSRect  clipRect=[_clipView bounds];
   float lineScroll=_verticalLineScroll;
   float pageScroll=_verticalPageScroll;

   if (![docView isFlipped]) {
       lineScroll=0.0-lineScroll;
       pageScroll=0.0-pageScroll;
   }

   switch([scroller hitPart]) {
       case NSScrollerIncrementLine:
           clipRect.origin.y+=lineScroll;
           break;
           
       case NSScrollerDecrementLine:
           clipRect.origin.y-=lineScroll;
           break;

       case NSScrollerIncrementPage:
           clipRect.origin.y+=pageScroll;
           break;
           
       case NSScrollerDecrementPage:
           clipRect.origin.y-=pageScroll;           
           break;

       case NSScrollerKnobSlot:
       case NSScrollerKnob:
       default:
           if(![docView isFlipped])
               value=1.0-value;

           value*=(docRect.size.height-clipRect.size.height);
           clipRect.origin.y=docRect.origin.y+floor(value);
           break;
   }

   [_clipView scrollToPoint:clipRect.origin];
   [[self superview] setNeedsDisplay:YES];
}

-(void)_horizontalScroll:(NSScroller *)scroller {
   float   value=[scroller floatValue];
   NSView *docView=[self documentView];
   NSRect  docRect=[docView frame];
   NSRect  clipRect=[_clipView bounds];
   NSRect  headerClipRect=[_headerClipView bounds];

   switch([scroller hitPart]) {
       case NSScrollerIncrementLine:
           clipRect.origin.x += _horizontalLineScroll;
           break;
           
       case NSScrollerDecrementLine:
           clipRect.origin.x -= _horizontalLineScroll;
          break;
           
       case NSScrollerIncrementPage:
           clipRect.origin.x += _horizontalPageScroll;
           break;
           
       case NSScrollerDecrementPage:
           clipRect.origin.x -= _horizontalPageScroll;
           break;

       case NSScrollerKnobSlot:
       case NSScrollerKnob:
       default:
           value*=(docRect.size.width-clipRect.size.width);
           clipRect.origin.x=docRect.origin.x+floor(value);
           break;
   }

   headerClipRect.origin.x = clipRect.origin.x;
   [_clipView scrollToPoint:clipRect.origin];
   [_headerClipView scrollToPoint:headerClipRect.origin];
   [[self superview] setNeedsDisplay:YES];
}

-(void)resizeSubviewsWithOldSize:(NSSize)oldSize {
   [self tile];

   if([self hasVerticalScroller])
    [self _verticalScroll:_verticalScroller];
   if([self hasHorizontalScroller])
    [self _horizontalScroll:_horizontalScroller];
}

@end
