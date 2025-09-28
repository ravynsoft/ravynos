/* Copyright (c) 2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSDateComponents.h>

@implementation NSDateComponents

-init {
   _era=NSUndefinedDateComponent;
   _year=NSUndefinedDateComponent;
   _quarter=NSUndefinedDateComponent;
   _month=NSUndefinedDateComponent;
   _week=NSUndefinedDateComponent;
   _weekday=NSUndefinedDateComponent;
   _weekdayOrdinal=NSUndefinedDateComponent;
   _day=NSUndefinedDateComponent;
   _hour=NSUndefinedDateComponent;
   _minute=NSUndefinedDateComponent;
   _second=NSUndefinedDateComponent;
   return self;
}

-(NSInteger)era {
   return _era;
}

-(NSInteger)year {
   return _year;
}

-(NSInteger)quarter {
   return _quarter;
}

-(NSInteger)month {
   return _month;
}

-(NSInteger)week {
   return _week;
}

-(NSInteger)weekday {
   return _weekday;
}

-(NSInteger)weekdayOrdinal {
   return _weekdayOrdinal;
}

-(NSInteger)day {
   return _day;
}

-(NSInteger)hour {
   return _hour;
}

-(NSInteger)minute {
   return _minute;
}

-(NSInteger)second {
   return _second;
}

-(void)setEra:(NSInteger)value {
   _era=value;
}

-(void)setYear:(NSInteger)value {
   _year=value;
}

-(void)setQuarter:(NSInteger)value {
   _quarter=value;
}

-(void)setMonth:(NSInteger)value {
   _month=value;
}

-(void)setWeek:(NSInteger)value {
   _week=value;
}

-(void)setWeekday:(NSInteger)value {
   _weekday=value;
}

-(void)setWeekdayOrdinal:(NSInteger)value {
   _weekdayOrdinal=value;
}

-(void)setDay:(NSInteger)value {
   _day=value;
}

-(void)setHour:(NSInteger)value {
   _hour=value;
}

-(void)setMinute:(NSInteger)value {
   _minute=value;
}

-(void)setSecond:(NSInteger)value {
   _second=value;
}

@end
