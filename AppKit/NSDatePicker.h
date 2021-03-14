/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSControl.h>
#import <AppKit/NSDatePickerCell.h>

@class NSCalendar;

@interface NSDatePicker : NSControl

- (BOOL)isBezeled;
- (BOOL)isBordered;

- delegate;

- (NSDatePickerElementFlags)datePickerElements;
- (NSDatePickerMode)datePickerMode;
- (NSDatePickerStyle)datePickerStyle;

- (NSCalendar *)calendar;
- (NSLocale *)locale;
- (NSDate *)minDate;
- (NSDate *)maxDate;

- (NSDate *)dateValue;
- (NSTimeInterval)timeInterval;
- (NSTimeZone *)timeZone;

- (BOOL)drawsBackground;
- (NSColor *)backgroundColor;
- (NSColor *)textColor;

- (void)setBezeled:(BOOL)flag;
- (void)setBordered:(BOOL)flag;

- (void)setDelegate:delegate;

- (void)setDatePickerElements:(NSDatePickerElementFlags)elements;
- (void)setDatePickerMode:(NSDatePickerMode)mode;
- (void)setDatePickerStyle:(NSDatePickerStyle)style;

- (void)setCalendar:(NSCalendar *)calendar;
- (void)setLocale:(NSLocale *)locale;
- (void)setMinDate:(NSDate *)date;
- (void)setMaxDate:(NSDate *)date;

- (void)setDateValue:(NSDate *)date;
- (void)setTimeInterval:(NSTimeInterval)interval;
- (void)setTimeZone:(NSTimeZone *)timeZone;

- (void)setDrawsBackground:(BOOL)flag;
- (void)setBackgroundColor:(NSColor *)color;
- (void)setTextColor:(NSColor *)color;

@end
