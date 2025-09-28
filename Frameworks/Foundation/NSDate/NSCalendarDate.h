/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSDate.h>

@interface NSCalendarDate : NSDate {
    NSTimeInterval _timeInterval;
    NSString *_format;
    NSTimeZone *_timeZone;
}

+ calendarDate;

- initWithYear:(NSInteger)year month:(NSUInteger)month day:(NSUInteger)day
            hour:(NSUInteger)hour
          minute:(NSUInteger)minute
          second:(NSUInteger)second
        timeZone:(NSTimeZone *)timeZone;

- initWithString:(NSString *)string calendarFormat:(NSString *)format
            locale:(NSDictionary *)locale;
- initWithString:(NSString *)string calendarFormat:(NSString *)format;
- initWithString:(NSString *)string;

+ dateWithYear:(NSInteger)year month:(NSUInteger)month day:(NSUInteger)day
            hour:(NSUInteger)hour
          minute:(NSUInteger)minute
          second:(NSUInteger)second
        timeZone:(NSTimeZone *)timeZone;

+ dateWithString:(NSString *)string calendarFormat:(NSString *)format
            locale:(NSDictionary *)locale;

+ dateWithString:(NSString *)string calendarFormat:(NSString *)format;

- (NSString *)calendarFormat;
- (NSTimeZone *)timeZone;

- (void)setCalendarFormat:(NSString *)format;
- (void)setTimeZone:(NSTimeZone *)timeZone;

- (NSInteger)secondOfMinute; // 0-59
- (NSInteger)minuteOfHour; // 0-59
- (NSInteger)hourOfDay; // 0-23
- (NSInteger)dayOfWeek; // 0 through 6. how consistent
- (NSInteger)dayOfMonth; // 1 through 31
- (NSInteger)dayOfYear; // 1 through 366. also consistent
- (NSInteger)monthOfYear; // 1 through 12 says spec
- (NSInteger)yearOfCommonEra; // 1 through armageddon
- (NSInteger)dayOfCommonEra;

- (void)years:(NSInteger *)yearsp months:(NSInteger *)monthsp days:(NSInteger *)daysp
        hours:(NSInteger *)hoursp
      minutes:(NSInteger *)minutesp
      seconds:(NSInteger *)secondsp
    sinceDate:(NSCalendarDate *)date;

- (NSCalendarDate *)dateByAddingYears:(NSInteger)years months:(NSInteger)months
                                 days:(NSInteger)days
                                hours:(NSInteger)hours
                              minutes:(NSInteger)minutes
                              seconds:(NSInteger)seconds;

- (NSString *)descriptionWithCalendarFormat:(NSString *)format
                                     locale:(NSDictionary *)locale;
- (NSString *)descriptionWithCalendarFormat:(NSString *)format;
- (NSString *)descriptionWithLocale:(NSDictionary *)locale;

@end
