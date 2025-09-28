/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <CoreFoundation/CFCalendar.h>

@class NSDateComponents, NSTimeZone, NSLocale, NSDate;

enum {
    NSEraCalendarUnit = kCFCalendarUnitEra,
    NSYearCalendarUnit = kCFCalendarUnitYear,
    NSMonthCalendarUnit = kCFCalendarUnitMonth,
    NSDayCalendarUnit = kCFCalendarUnitDay,
    NSHourCalendarUnit = kCFCalendarUnitHour,
    NSMinuteCalendarUnit = kCFCalendarUnitMinute,
    NSSecondCalendarUnit = kCFCalendarUnitSecond,
    NSWeekCalendarUnit = kCFCalendarUnitWeek,
    NSWeekdayCalendarUnit = kCFCalendarUnitWeekday,
    NSWeekdayOrdinalCalendarUnit = kCFCalendarUnitWeekdayOrdinal,
    //   NSQuarterCalendarUnit = kCFCalendarUnitQuarter,
};
typedef NSUInteger NSCalendarUnit;

FOUNDATION_EXPORT NSString *const NSGregorianCalendar;

@interface NSCalendar : NSObject <NSCopying> {
    NSString *_identifier;
    NSUInteger _firstWeekday;
    NSUInteger _minimumDaysInFirstWeek;
    NSTimeZone *_timeZone;
    NSLocale *_locale;
}

+ currentCalendar;

- initWithCalendarIdentifier:(NSString *)identifier;

- (NSString *)calendarIdentifier;
- (NSUInteger)firstWeekday;
- (NSUInteger)minimumDaysInFirstWeek;
- (NSTimeZone *)timeZone;
- (NSLocale *)locale;

- (void)setFirstWeekday:(NSUInteger)weekday;
- (void)setMinimumDaysInFirstWeek:(NSUInteger)days;
- (void)setTimeZone:(NSTimeZone *)timeZone;
- (void)setLocale:(NSLocale *)locale;

- (NSRange)minimumRangeOfUnit:(NSCalendarUnit)unit;
- (NSRange)maximumRangeOfUnit:(NSCalendarUnit)unit;
- (NSRange)rangeOfUnit:(NSCalendarUnit)unit inUnit:(NSCalendarUnit)inUnit forDate:(NSDate *)date;
- (NSUInteger)ordinalityOfUnit:(NSCalendarUnit)unit inUnit:(NSCalendarUnit)inUnit forDate:(NSDate *)date;

- (NSDateComponents *)components:(NSUInteger)flags fromDate:(NSDate *)date;
- (NSDateComponents *)components:(NSUInteger)flags fromDate:(NSDate *)fromDate toDate:(NSDate *)toDate options:(NSUInteger)options;

- (NSDate *)dateByAddingComponents:(NSDateComponents *)components toDate:(NSDate *)date options:(NSUInteger)options;
- (NSDate *)dateFromComponents:(NSDateComponents *)components;

@end
