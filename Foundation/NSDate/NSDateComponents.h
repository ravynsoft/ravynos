/* Copyright (c) 2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>

enum {
    NSUndefinedDateComponent = 0x7fffffff
};

@interface NSDateComponents : NSObject {
    NSInteger _era;
    NSInteger _year;
    NSInteger _quarter;
    NSInteger _month;
    NSInteger _week;
    NSInteger _weekday;
    NSInteger _weekdayOrdinal;
    NSInteger _day;
    NSInteger _hour;
    NSInteger _minute;
    NSInteger _second;
}

- (NSInteger)era;
- (NSInteger)year;
- (NSInteger)quarter;
- (NSInteger)month;
- (NSInteger)week;
- (NSInteger)weekday;
- (NSInteger)weekdayOrdinal;
- (NSInteger)day;
- (NSInteger)hour;
- (NSInteger)minute;
- (NSInteger)second;

- (void)setEra:(NSInteger)value;
- (void)setYear:(NSInteger)value;
- (void)setQuarter:(NSInteger)value;
- (void)setMonth:(NSInteger)value;
- (void)setWeek:(NSInteger)value;
- (void)setWeekday:(NSInteger)value;
- (void)setWeekdayOrdinal:(NSInteger)value;
- (void)setDay:(NSInteger)value;
- (void)setHour:(NSInteger)value;
- (void)setMinute:(NSInteger)value;
- (void)setSecond:(NSInteger)value;

@end
