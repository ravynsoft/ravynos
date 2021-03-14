/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSScroller.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSDisplay.h>
#import <AppKit/NSGraphicsStyle.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSRaise.h>

@implementation NSScroller

+(float)scrollerWidth {
   return [[NSDisplay currentDisplay] scrollerWidth];
}

/* OS X has a global default "AppleScrollBarVariant" with the values: Single, DoubleMin, DoubleMax, and DoubleBoth
 This controls the default position of the scroller. This should be controlling the positioning.
 */

typedef enum {
    NSAppleScrollBarVariantSingle,
    NSAppleScrollBarVariantDoubleMin,
    NSAppleScrollBarVariantDoubleMax,
    NSAppleScrollBarVariantDoubleBoth,
} NSAppleScrollBarVariant;

static NSAppleScrollBarVariant appleScrollBarVariant(NSScroller *self){
    return NSAppleScrollBarVariantSingle;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    sFlags.isHoriz=(_bounds.size.width<_bounds.size.height)?NO:YES;
    _floatValue=0;
    _knobProportion=0;
    [self checkSpaceForParts];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   return self;
}

-initWithFrame:(NSRect)frame {
   [super initWithFrame:frame];
   sFlags.isHoriz=(_bounds.size.width<_bounds.size.height)?NO:YES;
   _floatValue=0;
   _knobProportion=0;
   [self checkSpaceForParts];
   return self;
}

-(BOOL)isFlipped {
   return YES;
}

-(BOOL)refusesFirstResponder {
   return YES;
}

-(BOOL)acceptsFirstResponder {
   return NO;
}

-(void)setFrame:(NSRect)frame {
   [super setFrame:frame];
   [self checkSpaceForParts];
}

-(id)target {
   return _target;
}

-(void)setTarget:(id)target {
   _target=target;
}

-(SEL)action {
   return _action;
}

-(void)setAction:(SEL)action {
   _action=action;
}

-(BOOL)isEnabled {
   return _isEnabled;
}

-(void)setEnabled:(BOOL)flag {
   _isEnabled=flag;
   [self setNeedsDisplay:YES];
}

-(BOOL)isVertical {
   return sFlags.isHoriz?NO:YES;
}

-(float)floatValue {
   return _floatValue;
}

-(float)knobProportion {
   return _knobProportion;
}

-(NSScrollArrowPosition)arrowsPosition {
   return _arrowsPosition;
}

- (NSControlSize)controlSize {
    return _controlSize; }

-(void)setFloatValue:(float)zeroToOneValue knobProportion:(float)zeroToOneKnob {
    if(zeroToOneValue>1)
        zeroToOneValue=1;
    if(zeroToOneValue<0)
        zeroToOneValue=0;
    if(zeroToOneKnob>1)
        zeroToOneKnob=1;
    if(zeroToOneKnob<0)
        zeroToOneKnob=0;

    _floatValue=zeroToOneValue;
   _knobProportion=zeroToOneKnob;
   if(_knobProportion>1)
    _knobProportion=1;

   [self setNeedsDisplay:YES];
}

-(double)doubleValue {
    return _floatValue;
}

-(void)setDoubleValue:(double)zeroToOneValue {
    if(zeroToOneValue>1)
        zeroToOneValue=1;
    if(zeroToOneValue<0)
        zeroToOneValue=0;
    
    _floatValue=zeroToOneValue;
    
   [self setNeedsDisplay:YES];
}

-(void)setArrowsPosition:(NSScrollArrowPosition)position {
   _arrowsPosition=position;
}

-(void)setControlSize:(NSControlSize)value {
   _controlSize=value;
   [self setNeedsDisplay:YES];
}

-(NSRect)frameOfDecrementPage {
   NSRect knobSlot=[self rectForPart:NSScrollerKnobSlot];
   NSRect knob=[self rectForPart:NSScrollerKnob];
   NSRect result=knobSlot;

   if(NSIsEmptyRect(knob))
    return NSZeroRect;

   if([self isVertical]){
    result.size.height=(knob.origin.y-knobSlot.origin.y);
    if(result.size.height<=0)
     result=NSZeroRect;
   }
   else {
    result.size.width=(knob.origin.x-knobSlot.origin.x);
    if(result.size.width<=0)
     result=NSZeroRect;
   }

   return result;
}

-(NSRect)frameOfIncrementPage {
   NSRect knobSlot=[self rectForPart:NSScrollerKnobSlot];
   NSRect knob=[self rectForPart:NSScrollerKnob];
   NSRect result=knobSlot;

   if(NSIsEmptyRect(knob))
    return NSZeroRect;

   if([self isVertical]){
    result.origin.y=knob.origin.y+knob.size.height;
    result.size.height=((knobSlot.origin.y+knobSlot.size.height)
        -result.origin.y);
    if(result.size.height<=0)
     result=NSZeroRect;
   }
   else {
    result.origin.x=knob.origin.x+knob.size.width;
    result.size.width=((knobSlot.origin.x+knobSlot.size.width)
        -result.origin.x);
    if(result.size.width<=0)
     result=NSZeroRect;
   }

   return result;
}

static inline float roundFloat(float value){
   value+=0.5;

   return (int)value;
}

-(NSRect)rectForPart:(NSScrollerPart)part {
   NSRect bounds=[self bounds];
   NSRect decLine=bounds;
   NSRect incLine;
   NSRect knobSlot=bounds;
   NSRect knob;
   NSRect result=NSZeroRect;

   if([self isVertical]){
    if(_arrowsPosition==NSScrollerArrowsNone){
     decLine=NSZeroRect;
     incLine=NSZeroRect;
    }
    else {
     decLine.size.height=decLine.size.width;
     if(decLine.size.height*2>bounds.size.height)
      decLine.size.height=floor(bounds.size.height/2);

     incLine=decLine;

     if(appleScrollBarVariant(self)==NSAppleScrollBarVariantSingle){
     incLine.origin.y=bounds.size.height-incLine.size.height;
    knobSlot.origin.y+=decLine.size.height;
    knobSlot.size.height-=decLine.size.height+incLine.size.height;
     }
     else {
      knobSlot.size.height-=decLine.size.height+incLine.size.height;

      if(_arrowsPosition==NSScrollerArrowsMaxEnd){
       incLine.origin.y=bounds.size.height-incLine.size.height;
       decLine.origin.y=incLine.origin.y-decLine.size.height;
      }
      else if(_arrowsPosition==NSScrollerArrowsMinEnd){
       incLine.origin.y=NSMaxY(decLine);
       knobSlot.origin.y=NSMaxY(incLine);
      }
     }
    }


    knob=knobSlot;
    knob.size.height=roundFloat(knobSlot.size.height*_knobProportion);
    if(knob.size.height<knob.size.width)
     knob.size.height=knob.size.width;
    knob.origin.y+=floor((knobSlot.size.height-knob.size.height)*_floatValue);

    if(floor(knob.size.height)>=floor(knobSlot.size.height))
     knob=NSZeroRect;
   }
   else {
    if(_arrowsPosition==NSScrollerArrowsNone){
     decLine=NSZeroRect;
     incLine=NSZeroRect;
    }
    else {
     decLine.size.width=decLine.size.height;
     if(decLine.size.width*2>bounds.size.width)
      decLine.size.width=floor(bounds.size.width/2);

     incLine=decLine;
     incLine.origin.x=bounds.size.width-incLine.size.width;
    }

    knobSlot.origin.x+=decLine.size.width;
    knobSlot.size.width-=decLine.size.width+incLine.size.width;

    knob=knobSlot;
    knob.size.width=roundFloat(knobSlot.size.width*_knobProportion);
    if(knob.size.width<knob.size.height)
     knob.size.width=knob.size.height;
    knob.origin.x+=floor((knobSlot.size.width-knob.size.width)*_floatValue);
    if(floor(knob.size.width)>=floor(knobSlot.size.width))
     knob=NSZeroRect;
   }

   switch(part){
    case NSScrollerNoPart:
     result=NSZeroRect;
     break;

    case NSScrollerKnob:
     result=[self isEnabled]?knob:NSZeroRect;
     break;

    case NSScrollerKnobSlot:
     result=knobSlot;
     break;

    case NSScrollerIncrementLine:
     result=incLine;
     break;

    case NSScrollerDecrementLine:
     result=decLine;
     break;
     
    case NSScrollerIncrementPage:
     result=[self frameOfIncrementPage];
     break;
     
    case NSScrollerDecrementPage:
     result=[self frameOfDecrementPage];
     break;
   }

   result=[self centerScanRect:result];

   return result;
}

-(void)checkSpaceForParts {
   sFlags.partsUsable=NSAllScrollerParts;
}

-(NSUsableScrollerParts)usableParts {
   return sFlags.partsUsable;
}

-(void)highlight:(BOOL)flag {
   if(_isHighlighted!=flag){
    _isHighlighted=flag;
    [self setNeedsDisplay:YES];
   }
}

-(void)drawKnobSlotInRect:(NSRect)rect highlight:(BOOL)flag {
}

-(void)drawParts {
   // do nothing
}

-(void)drawKnob {
   NSRect knob=[self rectForPart:NSScrollerKnob];

   if(!NSIsEmptyRect(knob)) 
    [[self graphicsStyle] drawScrollerKnobInRect:knob vertical:[self isVertical] highlight:_isHighlighted];
}

-(void)drawArrow:(NSScrollerArrow)arrow highlight:(BOOL)highlight {
   NSRect rect=(arrow==NSScrollerIncrementArrow)?[self rectForPart:NSScrollerIncrementLine]:[self rectForPart:NSScrollerDecrementLine];
   
   [[self graphicsStyle] drawScrollerButtonInRect:rect enabled:[self isEnabled] pressed:highlight vertical:[self isVertical] upOrLeft:(arrow!=NSScrollerIncrementArrow)];
 }


-(void)drawRect:(NSRect)rect {
   NSRect decPage=[self rectForPart:NSScrollerDecrementPage];
   NSRect incPage=[self rectForPart:NSScrollerIncrementPage];
   NSRect slot=[self rectForPart:NSScrollerKnobSlot];
   NSRect knob=[self rectForPart:NSScrollerKnob];
   BOOL   high;

   high=(_hitPart==NSScrollerIncrementLine) && _isHighlighted;
   [self drawArrow:NSScrollerIncrementArrow highlight:high];
   high=(_hitPart==NSScrollerDecrementLine) && _isHighlighted;
   [self drawArrow:NSScrollerDecrementArrow highlight:high];

   if(!NSIsEmptyRect(decPage))
    [[self graphicsStyle] drawScrollerTrackInRect:decPage vertical:[self isVertical] upOrLeft:YES];

   if(!NSIsEmptyRect(incPage))
    [[self graphicsStyle] drawScrollerTrackInRect:incPage vertical:[self isVertical] upOrLeft:NO];

   if(NSIsEmptyRect(knob) && !NSIsEmptyRect(slot))
    [[self graphicsStyle] drawScrollerTrackInRect:slot vertical:[self isVertical]];

   [self drawKnob];
}

-(NSScrollerPart)hitPart {
   return _hitPart;
}

-(NSScrollerPart)testPart:(NSPoint)point {
   int part;

   _hitPart=NSScrollerNoPart;

   for(part=NSScrollerIncrementLine;part<=NSScrollerKnobSlot;part++){
    NSRect rect=[self rectForPart:part];

    if(NSMouseInRect(point,rect,[self isFlipped])){
     _hitPart=part;
     break;
    }
   }

   return _hitPart;
}

-(void)trackKnob:(NSEvent *)event {
   NSPoint firstPoint=[self convertPoint:[event locationInWindow] fromView:nil];
   NSRect  slotRect=[self rectForPart:NSScrollerKnobSlot];
   NSRect  knobRect=[self rectForPart:NSScrollerKnob];
   float   totalSize;
   float   startFloatValue=_floatValue;

   if([self isVertical])
    totalSize=slotRect.size.height-knobRect.size.height;
   else
    totalSize=slotRect.size.width-knobRect.size.width;

   do{
    NSPoint point;
    float   delta;

    event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|NSLeftMouseDraggedMask];

    point=[self convertPoint:[event locationInWindow] fromView:nil];

    if([self isVertical])
     delta=point.y-firstPoint.y;
    else
     delta=point.x-firstPoint.x;

    if(totalSize==0)
     _floatValue=0;
    else
     _floatValue=startFloatValue+(delta/totalSize);
    if(_floatValue<0)
     _floatValue=0;
    else if(_floatValue>1.0)
     _floatValue=1.0;

    [self setNeedsDisplay:YES];

    [self sendAction:_action to:_target];

   }while([event type]!=NSLeftMouseUp);
}

-(void)trackScrollButtons:(NSEvent *)event {
   NSRect  rect=[self rectForPart:[self hitPart]];
   NSPoint point=[self convertPoint:[event locationInWindow] fromView:nil];

   // fixup to make paging behavior available via the Alt key, like NEXTSTEP
   if (([event modifierFlags] & NSAlternateKeyMask) && (_hitPart == NSScrollerIncrementLine))
       _hitPart=NSScrollerIncrementPage;
   else if (([event modifierFlags] & NSAlternateKeyMask) && (_hitPart == NSScrollerDecrementLine))
       _hitPart=NSScrollerDecrementPage;

   // scroll every 1/2 second...
   [NSEvent startPeriodicEventsAfterDelay:0.0 withPeriod:0.05];

   do{
       if([event type] != NSPeriodic)	// periodic events have location of 0,0
           point=[self convertPoint:[event locationInWindow] fromView:nil];

       [self highlight:NSMouseInRect(point,rect,[self isFlipped])];
       if (NSMouseInRect(point,rect,[self isFlipped]))
           [self sendAction:_action to:_target];

       event=[[self window] nextEventMatchingMask:NSPeriodicMask|NSLeftMouseUpMask|NSLeftMouseDraggedMask];
   }while([event type]!=NSLeftMouseUp);

   [NSEvent stopPeriodicEvents];

   [self highlight:NO];
}

-(void)trackPageSlots:(NSEvent *)event {
   do{
    NSPoint point=[self convertPoint:[event locationInWindow] fromView:nil];
    NSRect  knobRect=[self rectForPart:NSScrollerKnob];
    NSRect knobSlotRect=[self rectForPart:NSScrollerKnobSlot];
    float   roundingThreshold;

    // rounding to edges when distance from edge < size of knob
    if ([self isVertical])
        roundingThreshold=knobRect.size.height/knobSlotRect.size.height;
    else
        roundingThreshold=knobRect.size.width/knobSlotRect.size.width;

    if (NSMouseInRect(point,knobSlotRect,[self isFlipped]) && !NSMouseInRect(point,knobRect,[self isFlipped])) {
        // correct for knobSlot origin
        point.x -= knobSlotRect.origin.x;
        point.y -= knobSlotRect.origin.y;
        
        if ([self isVertical])
            _floatValue = point.y/knobSlotRect.size.height;
        else
            _floatValue = point.x/knobSlotRect.size.width;

        if (_floatValue < roundingThreshold)
            _floatValue = 0;
        else if (_floatValue > 1.0 - roundingThreshold)
            _floatValue = 1.0;

        // knobRect may now be different
        knobRect=[self rectForPart:NSScrollerKnob];
        _hitPart=NSScrollerKnobSlot;			// for scroll-to-click

        [self highlight:YES];
        [self sendAction:_action to:_target];
    }

    event=[[self window] nextEventMatchingMask:NSLeftMouseUpMask|NSLeftMouseDraggedMask];

   }while([event type]!=NSLeftMouseUp);

   [self highlight:NO];
}

-(void)mouseDown:(NSEvent *)event {
   NSPoint        point=[self convertPoint:[event locationInWindow] fromView:nil];
   NSScrollerPart part=[self testPart:point];

   if(![self isEnabled])
    return;

   switch(part){
    case NSScrollerNoPart:
     return;

    case NSScrollerKnob:
     [self trackKnob:event];
     break;

    case NSScrollerKnobSlot:
     break;

    case NSScrollerIncrementLine:
    case NSScrollerDecrementLine:
     [self trackScrollButtons:event];
     break;
     
    case NSScrollerIncrementPage:
    case NSScrollerDecrementPage:
     [self trackPageSlots:event];
     break;
   }

}

-(void)scrollWheel:(NSEvent *)event {
    NSRect  slotRect=[self rectForPart:NSScrollerKnobSlot];
    NSRect  knobRect=[self rectForPart:NSScrollerKnob];
        
    if([self isVertical]) {
        float delta=[event deltaY];
        float totalSize=slotRect.size.height-knobRect.size.height;
        
        if(totalSize==0)
            _floatValue=0;
        else
            _floatValue=_floatValue-(delta/totalSize);
        
        if(_floatValue<0)
            _floatValue=0;
        else if(_floatValue>1.0)
            _floatValue=1.0;

        [self setNeedsDisplay:YES];
        [self sendAction:_action to:_target];
    }
}

@end
