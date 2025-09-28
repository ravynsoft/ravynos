/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSProgressIndicator.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSGraphicsStyle.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSGraphicsContextFunctions.h>
#import <AppKit/NSObject+BindingSupport.h>
#import <ApplicationServices/ApplicationServices.h>
#import <AppKit/NSRaise.h>

@implementation NSProgressIndicator

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    unsigned           flags=[keyed decodeIntForKey:@"NSpiFlags"];

    _minValue=[keyed decodeDoubleForKey:@"NSMinValue"];
    _maxValue=[keyed decodeDoubleForKey:@"NSMaxValue"];
    _value=_minValue;
    _animationDelay=5.0/60.0;
    _animationTimer=nil;
    _animationValue=0;
    _style=(flags&0x1000)?NSProgressIndicatorSpinningStyle:NSProgressIndicatorBarStyle;
    _size=(flags&0x100)?NSSmallControlSize:NSRegularControlSize;
    _displayWhenStopped=(flags&0x2000)?NO:YES; // inverted 
    _isBezeled=YES;    
    _isIndeterminate=(flags&0x02)?YES:NO;
    _usesThreadedAnimation=NO;
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   return self;
}


-initWithFrame:(NSRect)frame {
   [super initWithFrame:frame];
   _value=0;
   _minValue=0;
   _maxValue=100;
   _animationDelay=5.0/60.0;
   _animationTimer=nil;
   _animationValue=0;
   _style=NSProgressIndicatorBarStyle;
   _size=NSRegularControlSize;
   _tint=0;
   _displayWhenStopped=NO;
   _isBezeled=YES;
   _isIndeterminate=YES;
   _usesThreadedAnimation=NO;
   
   return self;
}

-(void)_invalidateTimer {
   if(_animationTimer!=nil){
    [self willChangeValueForKey:@"animate"];
     [_animationTimer invalidate];
     [_animationTimer release];
     _animationTimer=nil;
    [self didChangeValueForKey:@"animate"];
   }
}

-(void)_buildTimer {
   if(_animationTimer==nil){
    [self willChangeValueForKey:@"animate"];
    _animationTimer = [[NSTimer timerWithTimeInterval:_animationDelay
                                                        target:self
                                                      selector:@selector(animate:)
                                                      userInfo:nil
                                                       repeats:YES] retain];

// FIXME: does it do this? Or does it add it to the current mode?
// Apple's does work in a modal panel (right?)_                                                 
    [[NSRunLoop currentRunLoop] addTimer:_animationTimer forMode:NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addTimer:_animationTimer forMode:NSModalPanelRunLoopMode];
    
    [self didChangeValueForKey:@"animate"];
   }
}

-(void)dealloc {
   [self _invalidateTimer];
   
   [super dealloc];
}

-(BOOL)isFlipped {
    return YES;
}

-(void)drawIndeterminateCircular {
   CGContextRef context=[[NSGraphicsContext currentContext] graphicsPort];
   int     i,numberOfRays=12;
   int     offset=roundf(_animationValue*(numberOfRays-1));
   NSRect bounds=[self bounds];
   CGPoint center=CGPointMake(bounds.origin.x+bounds.size.width/2,bounds.origin.y+bounds.size.height/2);
   CGFloat angle=0;//MI_PI*2.0/4.0;
   CGFloat arc=M_PI*2.0/numberOfRays;
   CGFloat minAxis=MIN(bounds.size.width,bounds.size.height);
   CGFloat lineWidth=minAxis/numberOfRays/2;
   
   lineWidth+=lineWidth*0.50;
   CGContextSaveGState(context);
   CGContextSetLineWidth(context,lineWidth);
   CGContextSetLineCap(context,kCGLineCapRound);
   for(i=0;i<numberOfRays;i++){
    CGFloat startGray=0;
    CGFloat endGray=0.90;
    int     place=(i-offset<0)?numberOfRays+(i-offset):i-offset;
    CGFloat gray=startGray+((endGray-startGray)/numberOfRays)*place;
    CGAffineTransform rotate=CGAffineTransformMakeRotation(angle+arc*i);
    CGPoint ray;
    
    CGContextSetGrayStrokeColor(context,gray,1.0);
    ray.x=0;
    ray.y=minAxis/2-lineWidth;
    ray=CGPointApplyAffineTransform(ray,rotate);
    CGContextMoveToPoint(context,center.x+ray.x,center.y+ray.y);

    ray.x=0;
    ray.y=minAxis/4;
    ray=CGPointApplyAffineTransform(ray,rotate);
    CGContextAddLineToPoint(context,center.x+ray.x,center.y+ray.y);
    CGContextStrokePath(context);
   }
   CGContextRestoreGState(context);
}

-(void)drawRect:(NSRect)clipRect {
   if(_style==NSProgressIndicatorBarStyle){
    if(_isIndeterminate){
     if([self isDisplayedWhenStopped] || (_animationTimer!=nil)){
      [[self graphicsStyle] drawProgressIndicatorIndeterminate:_bounds clipRect:clipRect bezeled:_isBezeled animation:_animationValue];
     }
    }
    else {
     double value=(_value-_minValue)/(_maxValue-_minValue);

     [[self graphicsStyle] drawProgressIndicatorDeterminate:_bounds clipRect:clipRect bezeled:_isBezeled value:value];
    }
   }
   else {
    if([self isIndeterminate]){
     if([self isDisplayedWhenStopped] || (_animationTimer!=nil)){
      [self drawIndeterminateCircular];
     }
    }
    else {
     // FIXME
     [[NSColor redColor] set];
     NSRectFill([self bounds]);
    }
   }   
}

-(NSProgressIndicatorStyle)style {
   return _style;
}

-(NSControlSize)controlSize {
   return _size;
}

-(NSControlTint)controlTint {
   return _tint;
}

-(BOOL)isDisplayedWhenStopped {
   return _displayWhenStopped;
}

-(BOOL)usesThreadedAnimation {
   return _usesThreadedAnimation;
}

-(double)minValue {
    return _minValue;
}

-(double)maxValue {
    return _maxValue;
}

-(double)doubleValue {
    return _value;
}

-(NSTimeInterval)animationDelay {
    return _animationDelay;
}

-(BOOL)isIndeterminate {
    return _isIndeterminate;
}

-(BOOL)isBezeled {
    return _isBezeled;
}

-(void)setStyle:(NSProgressIndicatorStyle)value {
   _style=value;
   [self sizeToFit];
   [self setNeedsDisplay:YES];
}

-(void)setControlSize:(NSControlSize)value {
   _size=value;
   [self sizeToFit];
   [self setNeedsDisplay:YES];
}

-(void)setControlTint:(NSControlTint)value {
   _tint=value;
   [self setNeedsDisplay:YES];
}

-(void)setDisplayedWhenStopped:(BOOL)value {
   _displayWhenStopped=value;
   [self setNeedsDisplay:YES];
}

-(void)setUsesThreadedAnimation:(BOOL)value {
	// This currently has no effect on the indicator - but no need to complain about it
   _usesThreadedAnimation=value;
}

-(void)setMinValue:(double)value {
    _minValue = value;
    [self setNeedsDisplay:YES];
}

-(void)setMaxValue:(double)value {
    _maxValue = value;
    [self setNeedsDisplay:YES];
}

-(void)setDoubleValue:(double)value {
   if(value<_minValue)
    value=_minValue;
   if(value>_maxValue)
    value=_maxValue;
     
   _value = value;
   [self setNeedsDisplay:YES];
}

-(void)setAnimationDelay:(double)delay {
    _animationDelay = delay;
   [self _invalidateTimer];
   [self _buildTimer];
}

-(void)setIndeterminate:(BOOL)flag {
   _isIndeterminate = flag;
   [self setNeedsDisplay:YES];
}

-(void)setBezeled:(BOOL)flag {
   _isBezeled = flag;
   [self setNeedsDisplay:YES];
}

-(void)incrementBy:(double)value {
   [self setDoubleValue:_value+value];
}

-(void)sizeToFit {
}

- (void)_runThreadedAnimation:(id)arg
{
	_endThreadedAnimation = NO;
	
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	[self _buildTimer];
	[self setNeedsDisplay:YES];
	
	BOOL isRunning = YES;
	do {
		isRunning = [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
								 beforeDate:[NSDate distantFuture]];
	} while (isRunning && _endThreadedAnimation == NO);
	
	[self _invalidateTimer];
	
	[pool drain];
}

- (void)_stopThreadedAnimation
{
	_endThreadedAnimation = YES;
}

-(void)startAnimation:sender {
	if (!_isIndeterminate)
		return;

	if (_usesThreadedAnimation) {
		[NSThread detachNewThreadSelector: @selector(_runThreadedAnimation:) toTarget: self withObject: nil];
	} else {
		[self _buildTimer];
		[self setNeedsDisplay:YES];
	}
}

-(void)stopAnimation:sender {

	if (!_isIndeterminate)
		return;

	if (_usesThreadedAnimation) {
		[self _stopThreadedAnimation];
	} else {

		[self _invalidateTimer];
		[self setNeedsDisplay:YES];
	}
}

-(void)animate:sender {
   _animationValue+=1.0/[self bounds].size.width;
    
   if(_animationValue>1)
    _animationValue=0;
    
   [self setNeedsDisplay:YES];
}

@end


@implementation NSProgressIndicator (Bindings)

-(BOOL)_animate {
   return _animationTimer!=nil;
}

-(void)_setAnimate:(BOOL)animate {
   if(animate){
    [self startAnimation:nil];
   }
   else {
    [self stopAnimation:nil];
   }
}

-_replacementKeyPathForBinding:(id)binding {
	if([binding isEqual:@"value"])
      return @"doubleValue";
   return [super _replacementKeyPathForBinding:binding];
}

@end
