/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSDatePicker.h>
#import <AppKit/NSDatePickerCell.h>
#import <AppKit/NSObject+BindingSupport.h>

@implementation NSDatePicker

-(BOOL)acceptsFirstResponder {
   return YES;
}

-(BOOL)needsPanelToBecomeKey {
    return YES;
}

-(NSDatePickerCell *)_pickerCell {
   return [self cell];
}

-(BOOL)isBezeled {
   return [[self _pickerCell] isBezeled];
}

-(BOOL)isBordered {
   return [[self _pickerCell] isBordered];
}

-delegate {
   return [[self _pickerCell] delegate];
}


-(NSDatePickerElementFlags)datePickerElements {
   return [[self _pickerCell] datePickerElements];
}

-(NSDatePickerMode)datePickerMode {
   return [[self _pickerCell] datePickerMode];
}

-(NSDatePickerStyle)datePickerStyle {
   return [[self _pickerCell] datePickerStyle];
}


-(NSCalendar *)calendar {
   return [[self _pickerCell] calendar];
}

-(NSLocale *)locale {
   return [[self _pickerCell] locale];
}

-(NSDate *)minDate {
   return [[self _pickerCell] minDate];
}

-(NSDate *)maxDate {
   return [[self _pickerCell] maxDate];
}


-(NSDate *)dateValue {
   return [[self _pickerCell] dateValue];
}

-(NSTimeInterval)timeInterval {
   return [[self _pickerCell] timeInterval];
}

-(NSTimeZone *)timeZone {
   return [[self _pickerCell] timeZone];
}


-(BOOL)drawsBackground {
   return [[self _pickerCell] drawsBackground];
}

-(NSColor *)backgroundColor {
   return [[self _pickerCell] backgroundColor];
}

-(NSColor *)textColor {
   return [[self _pickerCell] textColor];
}

-(void)setBezeled:(BOOL)flag {
   [[self _pickerCell] setBezeled:flag];
   [self setNeedsDisplay:YES];
}

-(void)setBordered:(BOOL)flag {
   [[self _pickerCell] setBordered:flag];
   [self setNeedsDisplay:YES];
   [self setNeedsDisplay:YES];
}

-(void)setDelegate:delegate {
   [[self _pickerCell] setDelegate:delegate];
}

-(void)setDatePickerElements:(NSDatePickerElementFlags)elements {
   [[self _pickerCell] setDatePickerElements:elements];
   [self setNeedsDisplay:YES];
}

-(void)setDatePickerMode:(NSDatePickerMode)mode {
   [[self _pickerCell] setDatePickerMode:mode];
   [self setNeedsDisplay:YES];
}

-(void)setDatePickerStyle:(NSDatePickerStyle)style {
   [[self _pickerCell] setDatePickerStyle:style];
   [self setNeedsDisplay:YES];
}

-(void)setCalendar:(NSCalendar *)calendar {
   [[self _pickerCell] setCalendar:calendar];
   [self setNeedsDisplay:YES];
}

-(void)setLocale:(NSLocale *)locale {
   [[self _pickerCell] setLocale:locale];
   [self setNeedsDisplay:YES];
}

-(void)setMinDate:(NSDate *)date {
   [[self _pickerCell] setMinDate:date];
   [self setNeedsDisplay:YES];
}

-(void)setMaxDate:(NSDate *)date {
   [[self _pickerCell] setMaxDate:date];
   [self setNeedsDisplay:YES];
}

-(void)setDateValue:(NSDate *)date {
   [[self _pickerCell] setDateValue:date];
   [self setNeedsDisplay:YES];
}

-(void)setTimeInterval:(NSTimeInterval)interval {
   [[self _pickerCell] setTimeInterval:interval];
   [self setNeedsDisplay:YES];
}

-(void)setTimeZone:(NSTimeZone *)timeZone {
   [[self _pickerCell] setTimeZone:timeZone];
   [self setNeedsDisplay:YES];
}


-(void)setDrawsBackground:(BOOL)flag {
   [[self _pickerCell] setDrawsBackground:flag];
   [self setNeedsDisplay:YES];
}

-(void)setBackgroundColor:(NSColor *)color {
   [[self _pickerCell] setBackgroundColor:color];
   [self setNeedsDisplay:YES];
}

-(void)setTextColor:(NSColor *)color {
   [[self _pickerCell] setTextColor:color];
   [self setNeedsDisplay:YES];
}

-(void)insertText:(NSString *)string {
   [[self cell] insertText:string];
   [self setNeedsDisplay:YES];
}

-(void)keyDown:(NSEvent *)event {
   if([event type]==NSKeyDown) {
    [self interpretKeyEvents:[NSArray arrayWithObject:event]];
   }
}


-(id)_replacementKeyPathForBinding:(id)binding
{
    if([binding isEqual:@"minValue"])
        return @"minDate";
	if([binding isEqual:@"maxValue"])
        return @"maxDate";

	return [super _replacementKeyPathForBinding:binding];
}

@end
