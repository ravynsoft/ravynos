/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSDate.h>

@class NSArray, NSDate, NSData, NSDictionary, NSLocale, NSString, NSMutableArray;

typedef NSInteger NSTimeZoneNameStyle;

FOUNDATION_EXPORT NSString *const NSSystemTimeZoneDidChangeNotification;

@interface NSTimeZone : NSObject <NSCoding, NSCopying> {
    NSString *_name;
    NSData *_data;
}

+ (NSTimeZone *)localTimeZone;
+ (NSTimeZone *)systemTimeZone;
+ (NSTimeZone *)defaultTimeZone;

+ (void)resetSystemTimeZone;

+ (void)setDefaultTimeZone:(NSTimeZone *)timeZone;

+ (NSArray *)knownTimeZoneNames;

+ (NSDictionary *)abbreviationDictionary;

- initWithName:(NSString *)name data:(NSData *)data;
- initWithName:(NSString *)name;

+ (NSTimeZone *)timeZoneWithName:(NSString *)name data:(NSData *)data;
+ (NSTimeZone *)timeZoneWithName:(NSString *)name;

+ (NSTimeZone *)timeZoneForSecondsFromGMT:(NSInteger)seconds;
+ (NSTimeZone *)timeZoneWithAbbreviation:(NSString *)abbreviation;

- (NSString *)name;
- (NSData *)data;

- (BOOL)isEqualToTimeZone:(NSTimeZone *)timeZone;

- (NSInteger)secondsFromGMT;
- (NSString *)abbreviation;
- (BOOL)isDaylightSavingTime;
- (NSTimeInterval)daylightSavingTimeOffset;
- (NSDate *)nextDaylightSavingTimeTransition;

- (NSInteger)secondsFromGMTForDate:(NSDate *)date;
- (NSString *)abbreviationForDate:(NSDate *)date;
- (BOOL)isDaylightSavingTimeForDate:(NSDate *)date;
- (NSTimeInterval)daylightSavingTimeOffsetForDate:(NSDate *)date;
- (NSDate *)nextDaylightSavingTimeTransitionAfterDate:(NSDate *)date;

- (NSString *)localizedName:(NSTimeZoneNameStyle)style locale:(NSLocale *)locale;

- (NSString *)description;

@end
