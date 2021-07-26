/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSSliderCell.h>
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGradient.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSMatrix.h>
#import <AppKit/NSControl.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSGraphicsStyle.h>
#import <AppKit/NSRaise.h>

#define PIXELINSET	8
#define TICKHEIGHT      8

@implementation NSSliderCell

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
	_type = [keyed decodeIntForKey:@"NSSliderType"];
    _minValue=[keyed decodeDoubleForKey:@"NSMinValue"];
    _maxValue=[keyed decodeDoubleForKey:@"NSMaxValue"];
	   if ([keyed containsValueForKey: @"NSValue"]) {
		   // This cell prefers NSValue to NSContents
		   [_objectValue release];
		   _objectValue = [[keyed decodeObjectForKey: @"NSValue"] retain];
	   }
    _numberOfTickMarks=[keyed decodeIntForKey:@"NSNumberOfTickMarks"];
    _tickMarkPosition=[keyed decodeIntForKey:@"NSTickMarkPosition"];
    _allowsTickMarkValuesOnly=[keyed decodeBoolForKey:@"NSAllowsTickMarkValuesOnly"];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }

   return self;
}

-initTextCell:(NSString *)string {
   [super initTextCell:string];
   _isContinuous=YES; // NSCell defaults to NO, NSSliderCell defaults to YES
   _type=NSLinearSlider;
   _minValue=0;
   _maxValue=0;
   _altIncrementValue=0;
   _isVertical=-1;
   _lastRect=NSZeroRect;
   _numberOfTickMarks=0;
   _tickMarkPosition=NSTickMarkBelow;
   _allowsTickMarkValuesOnly=NO;
   return self;
}

-initImageCell:(NSImage *)image {
   [super initImageCell:image];
   _isContinuous=YES; // NSCell defaults to NO, NSSliderCell defaults to YES
   _type=NSLinearSlider;
   _minValue=0;
   _maxValue=0;
   _altIncrementValue=0;
   _isVertical=-1;
   _lastRect=NSZeroRect;
   _numberOfTickMarks=0;
   _tickMarkPosition=NSTickMarkBelow;
   _allowsTickMarkValuesOnly=NO;
   return self;
}

+(BOOL)prefersTrackingUntilMouseUp {
   return YES;
}

-(NSSliderType)sliderType {
   return _type;
}

-(double)minValue {
   return _minValue;
}

-(double)maxValue {
   return _maxValue;
}

-(double)altIncrementValue {
   return _altIncrementValue;
}

-(NSInteger)numberOfTickMarks {
   return _numberOfTickMarks;
}

-(NSTickMarkPosition)tickMarkPosition {
   return _tickMarkPosition;
}

-(BOOL)allowsTickMarkValuesOnly {
   return _allowsTickMarkValuesOnly;
}

-(CGFloat)knobThickness {
   return 0;
}

-(void)setSliderType:(NSSliderType)value {
   _type=value;
}

-(void)setMinValue:(double)minValue {
   _minValue = minValue;
}

-(void)setMaxValue:(double)maxValue {
   _maxValue = maxValue;
}

-(void)setAltIncrementValue:(double)value {
   _altIncrementValue=value;
}

-(void)setNumberOfTickMarks:(NSInteger)number {
   _numberOfTickMarks=number;
}

-(void)setTickMarkPosition:(NSTickMarkPosition)position {
   _tickMarkPosition=position;
}

-(void)setAllowsTickMarkValuesOnly:(BOOL)valuesOnly {
   _allowsTickMarkValuesOnly=valuesOnly;
}

-(void)setKnobThickness:(CGFloat)thickness {
// deprecated, do nothing
}

-(NSInteger)isVertical {
   return _isVertical;
}

-(NSInteger)indexOfTickMarkAtPoint:(NSPoint)point {
   int i;
   
   for(i=0;i<_numberOfTickMarks;i++){
    NSRect check=[self rectOfTickMarkAtIndex:i];
    
    check=NSInsetRect(check,-1,-1);
    
    if(NSPointInRect(point,check))
     return i;
   }
   
   return NSNotFound;
}

-(double)tickMarkValueAtIndex:(NSInteger)index {
   double scale=(_numberOfTickMarks==1)?0.5:(((double)index)/((double)(_numberOfTickMarks-1)));

   return _minValue+(_maxValue-_minValue)*scale;
}

-(double)closestTickMarkValueToValue:(double)value {
	if (_numberOfTickMarks < 1) {
		return value;
	}
   double closestValue=[self tickMarkValueAtIndex:0];
   NSInteger i;
   
   for(i=1;i<_numberOfTickMarks;i++){
    double check=[self tickMarkValueAtIndex:i];
    
    if(ABS(value-check)<ABS(value-closestValue))
     closestValue=check;
}

   return closestValue;
}

-(NSRect)trackRect {
   NSUnimplementedMethod();
   return NSZeroRect;
}

-(NSRect)rectOfTickMarkAtIndex:(NSInteger)index {
   NSRect result;
   CGFloat  length=(_isVertical?_lastRect.size.height:_lastRect.size.width)-2*PIXELINSET;
   CGFloat  position=floor((_numberOfTickMarks==1)?length/2:index*(length/(_numberOfTickMarks-1)));

   if(_isVertical){
    result.origin.x=_lastRect.origin.x;
    if(_tickMarkPosition==NSTickMarkBelow)
     result.origin.x+=_lastRect.size.width-TICKHEIGHT;
    result.origin.y=floor(_lastRect.origin.y+PIXELINSET+position);
    result.size.width=TICKHEIGHT;
    result.size.height=1;
   }
   else {
    result.origin.x=floor(_lastRect.origin.x+PIXELINSET+position);
    result.origin.y=_lastRect.origin.y;
    if(_tickMarkPosition==NSTickMarkAbove)
     result.origin.y+=_lastRect.size.height-TICKHEIGHT;
    result.size.width=1;
    result.size.height=TICKHEIGHT;
   }

   return result;
}


-(void)drawBarInside:(NSRect)frame flipped:(BOOL)isFlipped {
   [[_controlView graphicsStyle] drawSliderTrackInRect:frame vertical:[self isVertical] hasTickMarks:(_numberOfTickMarks>0)?YES:NO];
}


-(NSRect)_sliderRect {
   NSRect result=_lastRect;

   if(_numberOfTickMarks>0){
    if([self isVertical]){
     result.size.width-=TICKHEIGHT;
     if(_tickMarkPosition==NSTickMarkLeft)
      result.origin.x+=TICKHEIGHT;
    }
    else {
     result.size.height-=TICKHEIGHT;
     if(_tickMarkPosition==NSTickMarkBelow)
      result.origin.y+=TICKHEIGHT;
    }
   }

   return result;
}

-(NSRect)knobRectFlipped:(BOOL)flipped {
   double value=[self doubleValue];
   double percent=(value-_minValue)/(_maxValue-_minValue);
   NSRect sliderRect=[self _sliderRect];
   NSRect knobRect;
   NSSize knobSize=[[_controlView graphicsStyle] sliderKnobSizeForControlSize:[self controlSize]];
   
   if ([self isVertical]) {
    knobRect.size.height=knobSize.width;
    knobRect.size.width=knobSize.height;
    knobRect.origin.x=floor(sliderRect.origin.x+(sliderRect.size.width-knobSize.height)/2);
    knobRect.origin.y=floor(sliderRect.origin.y+PIXELINSET+percent*(sliderRect.size.height-(PIXELINSET*2))-knobSize.width/2);
   }
   else {
    knobRect.size.width=knobSize.width;
    knobRect.size.height=knobSize.height;
    knobRect.origin.x=floor(sliderRect.origin.x+PIXELINSET+percent*(sliderRect.size.width-(PIXELINSET*2))-knobSize.width/2);
    knobRect.origin.y=floor(sliderRect.origin.y+(sliderRect.size.height-knobSize.height)/2);
   }

   return knobRect;
}

-(void)drawKnob {
   [self drawKnob:[self knobRectFlipped:[[self controlView] isFlipped]]];
}

-(void)drawKnob:(NSRect)rect {
   [[_controlView graphicsStyle] drawSliderKnobInRect:rect vertical:[self isVertical] highlighted:[self isHighlighted] hasTickMarks:(_numberOfTickMarks>0)?YES:NO tickMarkPosition:_tickMarkPosition];
}

-(void)drawTickMarks {
   NSGraphicsStyle *style=[_controlView graphicsStyle];
   int i;

   for(i=0;i<_numberOfTickMarks;i++)
    [style drawSliderTickInRect:[self rectOfTickMarkAtIndex:i]];
}

- (void)drawLinearSliderWithFrame:(NSRect)frame inView:(NSView*)controlView
{
    _isVertical = (frame.size.height>frame.size.width)?1:0;
	
    [self drawBarInside:[self _sliderRect] flipped:[controlView isFlipped]];
    [self drawTickMarks];
    [self drawKnob];
}

- (NSGradient*)circularSliderBackgroundGradient
{
	static NSGradient* gradient = nil;
	if (gradient == nil) {
		gradient = [[NSGradient alloc] initWithColorsAndLocations: [NSColor whiteColor], 0,
					[NSColor lightGrayColor], 0.5,
					[NSColor whiteColor], 1, nil];
	}
	return gradient;
}

- (NSGradient*)circularSliderForegroundGradient
{
	static NSGradient* gradient = nil;
	if (gradient == nil) {
		gradient = [[NSGradient alloc] initWithColorsAndLocations: [NSColor grayColor], 0,
					[NSColor clearColor], 0.3,
					[NSColor clearColor], 0.7,
					[NSColor grayColor], 1, nil];
	}
	return gradient;
}

- (NSGradient*)circularSliderKnobGradient
{
	static NSGradient* gradient = nil;
	if (gradient == nil) {
		gradient = [[NSGradient alloc] initWithColorsAndLocations: [NSColor darkGrayColor], 0,
					[NSColor darkGrayColor], 0.49,
					[NSColor lightGrayColor], 0.51,
					[NSColor lightGrayColor], 1, nil];
		
	}
	return gradient;
}

- (void)drawCircularSliderWithFrame:(NSRect)frame inView:(NSView*)controlView
{
	NSRect sliderRect = frame;
	// Square it up
	if (frame.size.width > frame.size.height) {
		sliderRect = NSInsetRect(sliderRect, (frame.size.width - frame.size.height)/2.f, 0);
	} else {
		sliderRect = NSInsetRect(sliderRect, 0, (frame.size.height - frame.size.width)/2.f);
	}
	
	// Get it away from the edges of the frames - to ensure we're not clipped
	sliderRect = NSInsetRect(sliderRect, 3, 3);

	if (NSIsEmptyRect(sliderRect)) {
		return;
	}
	
	NSBezierPath* path = [NSBezierPath bezierPathWithOvalInRect: sliderRect];

	if ([self isEnabled]) {
		[[NSColor whiteColor] set];
	} else {
		[[NSColor controlColor] set];
	}
	[path fill];
		
	NSGradient* backgroundGradient = [self circularSliderBackgroundGradient];
	[backgroundGradient drawInBezierPath: path angle: 90];

	[[NSColor grayColor] set];
	[path stroke];
	
	double percent=0.;
	if (_maxValue != _minValue) {
		double value = [self doubleValue];
		percent = (value-_minValue)/(_maxValue-_minValue);
	}

	double angle = percent * 360;
	
	NSPoint knobOffset = NSMakePoint(0, -(NSHeight(sliderRect)/2.f) + 5);
	
	NSAffineTransform* rotateTransform = [NSAffineTransform transform];
	[rotateTransform rotateByDegrees: angle];
	knobOffset = [rotateTransform transformPoint: knobOffset];
	
	NSPoint knobCenter = NSMakePoint(NSMidX(sliderRect), NSMidY(sliderRect));
	knobCenter.x += knobOffset.x;
	knobCenter.y -= knobOffset.y;
	
	NSRect knobRect = NSMakeRect(knobCenter.x, knobCenter.y, 0, 0);
	knobRect = NSInsetRect(knobRect, -3, -3); // by visual inspection
	
	NSBezierPath* knobPath = [NSBezierPath bezierPathWithOvalInRect: knobRect];
	
	[[NSColor controlColor] set];
	[knobPath fill];

	[[NSColor grayColor] set];
	[knobPath stroke];
}

- (void)drawWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    _controlView=controlView;
    _lastRect = frame;

	switch ([self sliderType]) {
		case NSLinearSlider:
			[self drawLinearSliderWithFrame: frame inView: controlView];
			break;
		case NSCircularSlider:
			[self drawCircularSliderWithFrame: frame inView: controlView];
			break;
	}

    BOOL drawDottedRect=NO;
	
    // would be nice to put this code in some superclass
    if([[controlView window] firstResponder]==controlView){
        if([controlView isKindOfClass:[NSMatrix class]]){
            NSMatrix *matrix=(NSMatrix *)controlView;

            drawDottedRect=([matrix keyCell]==self)?YES:NO;
        }
        else if([controlView isKindOfClass:[NSControl class]]){
            NSControl *control=(NSControl *)controlView;

            drawDottedRect=([control selectedCell]==self)?YES:NO;
        }
    }

    if(drawDottedRect)
        NSDottedFrameRect(NSInsetRect(frame,1,1)); // fugly
}

-(void)setClosestDoubleValue:(double)value {
   if([self allowsTickMarkValuesOnly])
    value=[self closestTickMarkValueToValue:value];
    
   [self setDoubleValue:value];
}


// just a guess, but we'll try moving it 10% of its total range of values...
- (void)_incrementByPercentageAndConstrain:(CGFloat)percentage decrement:(BOOL)decrement {
    double originalValue = [self doubleValue];

    if (decrement)
        originalValue -= (_maxValue - _minValue) * percentage;
    else
        originalValue += (_maxValue - _minValue) * percentage;        
    
    if (originalValue > _maxValue)
        originalValue = _maxValue;
    else if (originalValue < _minValue)
        originalValue = _minValue;

    [self setClosestDoubleValue:originalValue];
}

- (void)moveUp:(id)sender {
    if ([self isVertical])
        [self _incrementByPercentageAndConstrain:0.10 decrement:NO];
}

- (void)moveDown:(id)sender {
    if ([self isVertical])
        [self _incrementByPercentageAndConstrain:0.10 decrement:YES];
}

- (void)moveLeft:(id)sender {
    if ([self isVertical] == NO)
        [self _incrementByPercentageAndConstrain:0.10 decrement:YES];
}

- (void)moveRight:(id)sender {
    if ([self isVertical] == NO)
        [self _incrementByPercentageAndConstrain:0.10 decrement:NO];
}

// linear sliderCell behavior:
// 	1. test hit in knob.. if so, go on to 3
//	2. test hit in bar. if hit, move knob to click location
//	3. track knob.
-(void)_setLinearDoubleValueFromPoint:(NSPoint)point
{
    // ((pointX-cellX)/cellW)*((max-min)+min)!
    double length,position;
    double percentPixels;

    if(_isVertical){
     length=(_lastRect.size.height-(2*PIXELINSET));
     position=point.y-(_lastRect.origin.y+PIXELINSET);
    }
    else {
     length=(_lastRect.size.width-(2*PIXELINSET));
     position=point.x-(_lastRect.origin.x+PIXELINSET);
    }

    percentPixels=position/length;

#if 0
    NSLog(@"percentPixels is %g; max-min is %g", percentPixels, _maxValue - _minValue);
    NSLog(@"doubleValue should be %g", (percentPixels*(_maxValue-_minValue))+_minValue);
#endif
    if (percentPixels > 1.0)
        percentPixels = 1.0;
    if (percentPixels < 0.0)
        percentPixels = 0.0;

    [self setClosestDoubleValue:(percentPixels*(_maxValue-_minValue))+_minValue];
}

// circular sliderCell behavior:
// 	1. set value based on angle of click
//	2. track mouse to continue calc'ing angles.
-(void)_setCircularDoubleValueFromPoint:(NSPoint)point flipped:(BOOL)flipped
{
	NSPoint center = NSMakePoint(NSMidX(_lastRect), NSMidY(_lastRect));

	if (flipped == NO) {
		point.y = center.y - (point.y - center.y);
	}
	// Get the angle and ensure it's in 0..2*PI - 0˙ is top center
	double angle = fmod(atan2(center.y - point.y, center.x - point.x) - M_PI_2 + M_PI * 2.f, M_PI * 2.f);

	// Convert to degrees
	angle *= 180.f / M_PI;
	
	double percentAngle = angle/360.f;
	
	double value = (percentAngle*(_maxValue-_minValue))+_minValue;
	
    [self setDoubleValue: value];
}

- (BOOL)startTrackingLinearSliderAt:(NSPoint)startPoint inView:(NSView*)controlView
{
    NSPoint localPoint = [controlView convertPoint:startPoint fromView:nil];
	
    if (NSMouseInRect(localPoint,[self knobRectFlipped:[controlView isFlipped]],[controlView isFlipped])) {
        [self highlight:YES withFrame:_lastRect inView:controlView];
    }

	[self _setLinearDoubleValueFromPoint:localPoint];
	return YES;
}

- (BOOL)startTrackingCircularSliderAt:(NSPoint)startPoint inView:(NSView*)controlView
{
    NSPoint localPoint = [controlView convertPoint:startPoint fromView:nil];
	
	[self _setCircularDoubleValueFromPoint: localPoint flipped: [controlView isFlipped]];
	return YES;
}

-(BOOL)startTrackingAt:(NSPoint)startPoint inView:(NSView *)controlView
{
	switch ([self sliderType]) {
		case NSLinearSlider:
			return [self startTrackingLinearSliderAt: startPoint inView: controlView];
			break;
		case NSCircularSlider:
			return [self startTrackingCircularSliderAt: startPoint inView: controlView];
			break;
	}
    return YES;		// what happened here?    
}

- (BOOL)continueTrackingLinearSliderAtPoint:(NSPoint)lastPoint at: (NSPoint)currentPoint inView:(NSView*)controlView
{
    NSPoint localPoint = [controlView convertPoint:currentPoint fromView:nil];
    [self _setLinearDoubleValueFromPoint:localPoint];
    [controlView setNeedsDisplayInRect:_lastRect];
	
	return YES;
}

- (BOOL)continueTrackingCircularSliderAtPoint:(NSPoint)lastPoint at: (NSPoint)currentPoint inView:(NSView*)controlView
{
    NSPoint localPoint = [controlView convertPoint:currentPoint fromView:nil];
    [self _setCircularDoubleValueFromPoint:localPoint flipped: [controlView isFlipped]];
    [controlView setNeedsDisplayInRect:_lastRect];
	return YES;
}

-(BOOL)continueTracking:(NSPoint)lastPoint at:(NSPoint)currentPoint inView:(NSView *)controlView
{
	switch ([self sliderType]) {
		case NSLinearSlider:
			return [self continueTrackingLinearSliderAtPoint: lastPoint at: currentPoint inView: controlView];
			break;
		case NSCircularSlider:
			return [self continueTrackingCircularSliderAtPoint: lastPoint at: currentPoint inView: controlView];
			break;
	}			
    return YES;
}

-(void)stopTracking:(NSPoint)lastPoint at:(NSPoint)stopPoint inView:(NSView *)controlView mouseIsUp:(BOOL)flag
{
    [self highlight:NO withFrame:_lastRect inView:controlView];
}

@end
