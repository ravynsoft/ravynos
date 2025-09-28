/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSDatePickerCell.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSBezierPath.h>
#import <AppKit/NSGraphicsStyle.h>
#import <Foundation/NSLocale.h>

@implementation NSDatePickerCell

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];

   if(![coder allowsKeyedCoding]){
    [NSException raise:NSInvalidArgumentException format:@"%@ can not initWithCoder:%@",isa,[coder class]];
    return self;
   }
   
   _elements=[coder decodeIntegerForKey:@"NSDatePickerElements"];
   _minDate=[[coder decodeObjectForKey:@"NSMinDate"] copy];
   _backgroundColor=[[coder decodeObjectForKey:@"NSBackgroundColor"] copy];
   _textColor=[[coder decodeObjectForKey:@"NSTextColor"] copy];
   _timeInterval=[coder decodeDoubleForKey:@"NSTimeInterval"];
   _calendar=[[NSCalendar currentCalendar] copy];
   // NSDatePicker has an NSEnabled field, 
   [self setEnabled:YES];
   
   return self;
}

-delegate {
   return _delegate;
}

-(NSDatePickerElementFlags)datePickerElements {
   return _elements;
}

-(NSDatePickerMode)datePickerMode {
   return _mode;
}

-(NSDatePickerStyle)datePickerStyle {
   return _style;
}

-(NSCalendar *)calendar {
   return _calendar;
}

-(NSLocale *)locale {
   return _locale;
}

-(NSDate *)minDate {
   return _minDate;
}

-(NSDate *)maxDate {
   return _maxDate;
}

-(NSDate *)dateValue {
   return [self objectValue];
}

-(NSTimeInterval)timeInterval {
   return 0;
}

-(NSTimeZone *)timeZone {
   return _timeZone;
}

-(BOOL)drawsBackground {
   return _drawsBackground;
}

-(NSColor *)backgroundColor {
   return _backgroundColor;
}

-(NSColor *)textColor {
   return _textColor;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
}

-(void)setDatePickerElements:(NSDatePickerElementFlags)elements {
   _elements=elements;
}

-(void)setDatePickerMode:(NSDatePickerMode)mode {
   _mode=mode;
}

-(void)setDatePickerStyle:(NSDatePickerStyle)style {
   _style=style;
}

-(void)setCalendar:(NSCalendar *)calendar {
   calendar=[calendar copy];
   [_calendar release];
   _calendar=calendar;
}

-(void)setLocale:(NSLocale *)locale {
   locale=[locale copy];
   [_locale release];
   _locale=locale;
}

-(void)setMinDate:(NSDate *)date {
   date=[date copy];
   [_minDate release];
   _minDate=date;
}

-(void)setMaxDate:(NSDate *)date {
   date=[date copy];
   [_maxDate release];
   _maxDate=date;
}

-(void)setDateValue:(NSDate *)date {
   [self setObjectValue:date];
}

-(void)setTimeInterval:(NSTimeInterval)interval {
}

-(void)setTimeZone:(NSTimeZone *)timeZone {
   timeZone=[timeZone copy];
   [_timeZone release];
   _timeZone=timeZone;
}

-(void)setDrawsBackground:(BOOL)flag {
   _drawsBackground=flag;
}

-(void)setBackgroundColor:(NSColor *)color {
   color=[color copy];
   [_backgroundColor release];
   _backgroundColor=color;
}

-(void)setTextColor:(NSColor *)color {
   color=[color copy];
   [_textColor release];
   _textColor=color;
}

static NSSize maxDigitSize(NSDictionary *attributes){
   NSString *digits[10]={
    @"0",@"1",@"2",@"3",@"4",@"5",@"6",@"7",@"8",@"9"
   };
   NSSize result={0};
   NSInteger i;

   for(i=0;i<10;i++){
    NSSize check=[digits[i] sizeWithAttributes:attributes];
    
    result.width=MAX(result.width,check.width);
    result.height=MAX(result.height,check.height);
   }
   
   return result;
}

static void drawRoundedSelection(NSRect rect){
   [[NSColor lightGrayColor] set];
   NSBezierPath *path=[NSBezierPath bezierPathWithRoundedRect:rect xRadius:3 yRadius:3];
   [path fill];
}

-(NSArray *)_attributedStrings {
   NSMutableArray          *result=[NSMutableArray array];
   NSMutableParagraphStyle *style=[[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];
   NSMutableDictionary     *attributes=[NSMutableDictionary dictionary];
   NSMutableDictionary     *separatorAttributes;
   
   [style setAlignment:NSRightTextAlignment];
   [attributes setObject:style forKey:NSParagraphStyleAttributeName];
   if([self font]!=nil)
    [attributes setObject:[self font] forKey:NSFontAttributeName];
   [attributes setObject:[self textColor] forKey:NSForegroundColorAttributeName];

   NSSize digitSize=maxDigitSize(attributes);
   CGFloat digitWidth=digitSize.width;
   
   separatorAttributes=[[attributes mutableCopy] autorelease];
   
   if((_elements&NSYearMonthDayDatePickerElementFlag)==NSYearMonthDayDatePickerElementFlag){
    NSUInteger        flags=NSYearCalendarUnit|NSMonthCalendarUnit|NSDayCalendarUnit;
    NSDateComponents *components=[_calendar components:flags fromDate:[self dateValue]];

    NSString *month=[NSString stringWithFormat:@"%d",[components month]];
    NSString *day=[NSString stringWithFormat:@"%d",[components day]];
    NSString *year=[NSString stringWithFormat:@"%d",[components year]];
    
    [attributes setObject:[NSNumber numberWithUnsignedInteger:[components month]] forKey:@"_value"];
    [attributes setObject:[NSNumber numberWithUnsignedInteger:1] forKey:@"_min"];
    [attributes setObject:[NSNumber numberWithUnsignedInteger:12] forKey:@"_max"];
    [attributes setObject:[NSNumber numberWithUnsignedInteger:NSMonthCalendarUnit] forKey:@"_unit"];
    [attributes setObject:[NSNumber numberWithFloat:digitWidth*2] forKey:@"_width"];
    [result addObject:[[[NSAttributedString alloc] initWithString:month attributes:attributes] autorelease]];
    
    [result addObject:[[[NSAttributedString alloc] initWithString:@"/" attributes:separatorAttributes] autorelease]];
    
    [attributes setObject:[NSNumber numberWithUnsignedInteger:[components day]] forKey:@"_value"];
    [attributes setObject:[NSNumber numberWithUnsignedInteger:1] forKey:@"_min"];
    [attributes setObject:[NSNumber numberWithUnsignedInteger:31] forKey:@"_max"];
    [attributes setObject:[NSNumber numberWithUnsignedInteger:NSDayCalendarUnit] forKey:@"_unit"];
    [attributes setObject:[NSNumber numberWithFloat:digitWidth*2] forKey:@"_width"];
    [result addObject:[[[NSAttributedString alloc] initWithString:day attributes:attributes] autorelease]];
    
    [result addObject:[[[NSAttributedString alloc] initWithString:@"/" attributes:separatorAttributes] autorelease]];

    [attributes setObject:[NSNumber numberWithUnsignedInteger:[components year]] forKey:@"_value"];
    [attributes setObject:[NSNumber numberWithUnsignedInteger:0] forKey:@"_min"];
    [attributes setObject:[NSNumber numberWithUnsignedInteger:9999] forKey:@"_max"];
    [attributes setObject:[NSNumber numberWithUnsignedInteger:NSYearCalendarUnit] forKey:@"_unit"];
    [attributes setObject:[NSNumber numberWithFloat:digitWidth*4] forKey:@"_width"];
    [result addObject:[[[NSAttributedString alloc] initWithString:year attributes:attributes] autorelease]];
   }
  
   return result;
}

static void getRectsInFrameForAttributesStrings(NSRect *rects,NSRect frame,NSArray *array){
   NSInteger i,count=[array count];
   CGFloat   maxHeight=0;
   
   for(i=0;i<count;i++){
    NSAttributedString *check=[array objectAtIndex:i];
    NSSize              size=[check size];
    NSDictionary       *attributes=[check attributesAtIndex:0 effectiveRange:NULL];
    NSNumber           *width=[attributes objectForKey:@"_width"];
    
    if(width==nil)
     rects[i].size=size;
    else {
     rects[i].size.width=[width floatValue];
     rects[i].size.height=size.height;
    }
    maxHeight=MAX(maxHeight,rects[i].size.height);
   }
   
   NSPoint origin=frame.origin;
   
   origin.y+=floor((frame.size.height-maxHeight)/2);
   
   for(i=0;i<count;i++){
    rects[i].origin=origin;
    origin.x+=rects[i].size.width;
   }
}

-(NSRect)_stepperFrameForFrame:(NSRect)frame {
   NSRect stepper=frame;
   stepper.origin.x=NSMaxX(stepper)-15;
   stepper.size.width=15;
   stepper=NSInsetRect(stepper,0,3);
   return stepper;
}

-(NSRect)_bezelFrameForFrame:(NSRect)frame {
   NSRect stepper=[self _stepperFrameForFrame:frame];
   NSRect bezel=frame;
   
   bezel.size.width-=stepper.size.width;
   bezel=NSInsetRect(bezel,1,3);
   return bezel;
}

-(void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)view {
   NSArray  *array=[self _attributedStrings];
   NSInteger i,count=[array count];
   NSRect    rects[count];
   
   frame=[self _bezelFrameForFrame:[view bounds]];

   getRectsInFrameForAttributesStrings(rects,NSInsetRect(frame,2,0),array);
   
   for(i=0;i<count;i++){
    NSAttributedString *string=[array objectAtIndex:i];
    NSDictionary       *attributes=[string attributesAtIndex:0 effectiveRange:NULL];
    NSNumber           *unit=[attributes objectForKey:@"_unit"];
    
    if([unit unsignedIntegerValue]&_selectedUnit){
     drawRoundedSelection(rects[i]);
    }
     
    [string drawInRect:rects[i]];
   }
}

-(void)_multiplyCurrentUnitBy:(NSInteger)multiply add:(NSInteger)addValue {
   NSArray          *array=[self _attributedStrings];
   NSInteger         i,count=[array count];
   NSDateComponents *components=[[[NSDateComponents alloc] init] autorelease];
   
   for(i=0;i<count;i++){
    NSAttributedString *string=[array objectAtIndex:i];
    NSDictionary       *attributes=[string attributesAtIndex:0 effectiveRange:NULL];
    NSUInteger          unit=[[attributes objectForKey:@"_unit"] unsignedIntegerValue];
    
    if(unit==0)
     continue;
     
    NSUInteger          value=[[attributes objectForKey:@"_value"] unsignedIntegerValue];
    NSUInteger          min=[[attributes objectForKey:@"_min"] unsignedIntegerValue];
    NSUInteger          max=[[attributes objectForKey:@"_max"] unsignedIntegerValue];
    
    if(unit==_selectedUnit){
     value*=multiply;
     value+=addValue;
     if(value>max){
      value=addValue;
     }
     if(value<min)
      value=min;
    }
    
    switch(unit){
     case NSYearCalendarUnit:
      [components setYear:value];
      break;
      
     case NSMonthCalendarUnit:
      [components setMonth:value];
      break;
      
     case NSDayCalendarUnit:
      [components setDay:value];
      break;
    }
    
   }
   
   NSDate *date=[_calendar dateFromComponents:components];

   [self setDateValue:date];
}



-(NSRect)_upArrowFrameForStepperFrame:(NSRect)frame {
    int half = frame.size.height/2;
    if (![_controlView isFlipped])
        frame.origin.y += half;
    frame.size.height = half;
    return frame;
}

-(NSRect)_downArrowFrameForStepperFrame:(NSRect)frame {
    int half = frame.size.height/2;
    frame.size.height = half;
    if ([_controlView isFlipped])
        frame.origin.y += half;
    return frame;
}

-(void)drawWithFrame:(NSRect)frame inView:(NSView *)view {
   NSRect stepper=[self _stepperFrameForFrame:frame];
   NSRect bezel=[self _bezelFrameForFrame:frame];
      
   NSDrawWhiteBezel(bezel,bezel);
   
   [self drawInteriorWithFrame:frame inView:view];
   
   [[_controlView graphicsStyle] drawStepperButtonInRect:[self _upArrowFrameForStepperFrame:stepper] clipRect:stepper enabled:[self isEnabled] highlighted:_isUpHighlighted upNotDown:YES];
   [[_controlView graphicsStyle] drawStepperButtonInRect:[self _downArrowFrameForStepperFrame:stepper] clipRect:stepper enabled:[self isEnabled] highlighted:_isDownHighlighted upNotDown:NO];
}

-(void)selectComponentUnderPoint:(NSPoint)point inView:(NSView *)view {
   NSArray  *array=[self _attributedStrings];
   NSInteger i,count=[array count];
   NSRect    rects[count];
   NSRect    frame=[self _bezelFrameForFrame:[view bounds]];

   getRectsInFrameForAttributesStrings(rects,NSInsetRect(frame,2,0),array);
   
   for(i=0;i<count;i++){
    
    if(NSMouseInRect(point,rects[i],[view isFlipped])){
     NSAttributedString *string=[array objectAtIndex:i];
     NSDictionary       *attributes=[string attributesAtIndex:0 effectiveRange:NULL];
     NSUInteger          unit=[[attributes objectForKey:@"_unit"] unsignedIntegerValue];

     if(unit!=0){
      _selectedUnit=unit;
      break;
     }
    }
   }
}

-(void)incrementStepperUnderPoint:(NSPoint)point inView:(NSView *)view {
   NSRect stepper=[self _stepperFrameForFrame:[view bounds]];
   NSRect checkUp=[self _upArrowFrameForStepperFrame:stepper];
   NSRect checkDown=[self _downArrowFrameForStepperFrame:stepper];
 
   if(NSMouseInRect(point,checkUp,[view isFlipped]))
    [self _multiplyCurrentUnitBy:1 add:1];  
    
   if(NSMouseInRect(point,checkDown,[view isFlipped]))
    [self _multiplyCurrentUnitBy:1 add:-1];  
}

-(BOOL)startTrackingAt:(NSPoint)startPoint inView:(NSView *)view {
   startPoint=[view convertPoint:startPoint fromView:nil];
   [self selectComponentUnderPoint:startPoint inView:view];
   [self incrementStepperUnderPoint:startPoint inView:view];
   return YES;
}

-(BOOL)continueTracking:(NSPoint)lastPoint at:(NSPoint)currentPoint inView:(NSView *)view {
   currentPoint=[view convertPoint:currentPoint fromView:nil];
   [self selectComponentUnderPoint:currentPoint inView:view];
   return YES;
}

-(void)insertText:(NSString *)text {
   NSInteger i,length=[text length];
   unichar   buffer[length];
   
   [text getCharacters:buffer];
   
   for(i=0;i<length;i++){
    unichar check=buffer[i];
    
    if(check>='0' && check<='9')
     [self _multiplyCurrentUnitBy:10 add:check-'0'];
   }
}

@end
