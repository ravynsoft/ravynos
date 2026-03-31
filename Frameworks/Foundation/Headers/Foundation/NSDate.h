/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSObjCRuntime.h>

@class NSString, NSArray, NSDictionary, NSCalendarDate, NSTimeZone;

typedef double NSTimeInterval;

FOUNDATION_EXPORT const NSTimeInterval NSTimeIntervalSince1970;

@interface NSDate : NSObject <NSCopying, NSCoding>

+ (NSTimeInterval)timeIntervalSinceReferenceDate;

+ distantPast;
+ distantFuture;

- init;
- initWithString:(NSString *)string;
- initWithTimeIntervalSinceReferenceDate:(NSTimeInterval)seconds;
- initWithTimeIntervalSinceNow:(NSTimeInterval)seconds;
- initWithTimeIntervalSince1970:(NSTimeInterval)seconds;
- initWithTimeInterval:(NSTimeInterval)seconds sinceDate:(NSDate *)date;

+ date;
+ dateWithString:(NSString *)string;
+ dateWithTimeIntervalSinceReferenceDate:(NSTimeInterval)seconds;
+ dateWithTimeIntervalSinceNow:(NSTimeInterval)seconds;
+ dateWithTimeIntervalSince1970:(NSTimeInterval)seconds;

- (NSTimeInterval)timeIntervalSinceReferenceDate;

- (NSTimeInterval)timeIntervalSinceDate:(NSDate *)date;
- (NSTimeInterval)timeIntervalSinceNow;
- (NSTimeInterval)timeIntervalSince1970;

- (BOOL)isEqualToDate:(NSDate *)date;

- (NSComparisonResult)compare:(NSDate *)date;
- (NSDate *)earlierDate:(NSDate *)date;
- (NSDate *)laterDate:(NSDate *)date;

- addTimeInterval:(NSTimeInterval)seconds;
- dateByAddingTimeInterval:(NSTimeInterval)seconds;

- (NSCalendarDate *)dateWithCalendarFormat:(NSString *)format
                                  timeZone:(NSTimeZone *)timeZone;

- (NSString *)descriptionWithLocale:(NSDictionary *)locale;
- (NSString *)descriptionWithCalendarFormat:(NSString *)format
                                   timeZone:(NSTimeZone *)timeZone
                                     locale:(NSDictionary *)locale;

@end

#import <Foundation/NSTimeZone.h>
#import <Foundation/NSCalendarDate.h>

@interface NSDate (NSCalendarDateExtras)
+ (id)dateWithNaturalLanguageString:(NSString *)string;
@end
