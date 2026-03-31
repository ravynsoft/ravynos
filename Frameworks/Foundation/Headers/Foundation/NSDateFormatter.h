/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSFormatter.h>
#import <Foundation/NSDate.h>
#import <CoreFoundation/CFDateFormatter.h>

typedef enum {
    NSDateFormatterBehaviorDefault = 0,
    NSDateFormatterBehavior10_0 = 1000,
    NSDateFormatterBehavior10_4 = 1040,
} NSDateFormatterBehavior;

typedef enum {
    NSDateFormatterNoStyle = kCFDateFormatterNoStyle,
    NSDateFormatterShortStyle = kCFDateFormatterShortStyle,
    NSDateFormatterMediumStyle = kCFDateFormatterMediumStyle,
    NSDateFormatterLongStyle = kCFDateFormatterLongStyle,
    NSDateFormatterFullStyle = kCFDateFormatterFullStyle
} NSDateFormatterStyle;

@interface NSDateFormatter : NSFormatter {
    NSDateFormatterBehavior _behavior;
    NSDateFormatterStyle _dateStyle;
    NSDateFormatterStyle _timeStyle;
    NSString *_dateFormat10_0;
    NSString *_dateFormat;
    BOOL _allowsNaturalLanguage;
    NSDictionary *_locale;
    NSTimeZone *_tz;
}

- initWithDateFormat:(NSString *)format allowNaturalLanguage:(BOOL)flag; // shouldn't this be "allows" ?

// added because NSDateFormatter is the backend for initWithString:calendarFormat:locale
// shouldn't this really exist anyway?
- initWithDateFormat:(NSString *)format allowNaturalLanguage:(BOOL)flag locale:(NSDictionary *)locale;

- (NSString *)dateFormat;
- (BOOL)allowsNaturalLanguage;
- (NSDateFormatterBehavior)formatterBehavior;

- (NSDictionary *)locale;

- (void)setDateFormat:(NSString *)format;

- (NSString *)stringFromDate:(NSDate *)date;
- (NSDate *)dateFromString:(NSString *)string;
- (NSArray *)shortStandaloneWeekdaySymbols;
- (NSArray *)standaloneWeekdaySymbols;

- (void)setLenient:(BOOL)value;
- (void)setFormatterBehavior:(NSDateFormatterBehavior)value;
- (void)setTimeZone:(NSTimeZone *)tz;
- (NSTimeZone *)timeZone;

@end

// internal use

NSTimeInterval NSMoveIntervalFromTimeZoneToGMT(NSTimeInterval interval, NSTimeZone *timeZone);
NSTimeInterval NSMoveIntervalFromGMTToTimeZone(NSTimeInterval interval, NSTimeZone *timeZone);

NSInteger NSNumberOfDaysInMonthOfYear(NSInteger month, NSInteger year);
// interval is not time zone adjusteed
NSTimeInterval NSTimeIntervalWithComponents(NSInteger year, NSInteger month, NSInteger day, NSInteger hour, NSInteger minute, NSInteger second, NSInteger milliseconds);

// interval has already been time zone adjusted
NSInteger NSDayOfCommonEraFromTimeInterval(NSTimeInterval interval);

NSInteger NSYearFromTimeInterval(NSTimeInterval interval);
NSInteger NSDayOfYearFromTimeInterval(NSTimeInterval interval); // 1-366

NSInteger NSMonthFromTimeInterval(NSTimeInterval interval); // 1-12
NSInteger NSDayOfMonthFromTimeInterval(NSTimeInterval interval); // 0-31

NSInteger NSWeekdayFromTimeInterval(NSTimeInterval interval); // 1-7

NSInteger NS24HourFromTimeInterval(NSTimeInterval interval); // 0-23
NSInteger NS12HourFromTimeInterval(NSTimeInterval interval); // 1-12
NSInteger NSAMPMFromTimeInterval(NSTimeInterval interval); // 0-1

NSInteger NSMinuteFromTimeInterval(NSTimeInterval interval); // 0-59

NSInteger NSSecondFromTimeInterval(NSTimeInterval interval); // 0-59

NSInteger NSMillisecondsFromTimeInterval(NSTimeInterval interval); // 0-999

// interval will be time-zone adjusted
NSString *NSStringWithDateFormatLocale(NSTimeInterval interval, NSString *format, NSDictionary *locale, NSTimeZone *timeZone);

NSDate *NSDateWithStringDateFormatLocale(NSString *string, NSString *format, NSDictionary *locale, NSTimeZone *timeZone);
