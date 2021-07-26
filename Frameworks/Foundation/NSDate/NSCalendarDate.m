/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - David Young <daver@geeks.org>
#import <Foundation/NSCalendarDate.h>
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSDateFormatter.h>
#import <Foundation/NSCoder.h>

// given in spec. is this a default someplace?
static NSString * const defaultCalendarDate = @"%Y-%m-%d %H:%M:%S %z";

@implementation NSCalendarDate

+calendarDate {
   return [[[self allocWithZone:NULL] init] autorelease];
}

-initWithTimeIntervalSinceReferenceDate:(NSTimeInterval)seconds {
    _timeInterval=seconds;
    _format=[defaultCalendarDate retain];
    _timeZone=[[NSTimeZone defaultTimeZone] retain];

    return self;
}

-(NSTimeInterval)timeIntervalSinceReferenceDate {
    return _timeInterval;
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

-initWithYear:(NSInteger)year month:(NSUInteger)month day:(NSUInteger)day
         hour:(NSUInteger)hour minute:(NSUInteger)minute second:(NSUInteger)second
timeZone:(NSTimeZone *)aTimeZone; {
	NSTimeInterval interval = NSTimeIntervalWithComponents(year, month, day, hour, minute, 
second, 0);
	NSTimeZone *tz = (aTimeZone == nil ? [NSTimeZone localTimeZone] : aTimeZone);
	interval = interval - [tz secondsFromGMTForDate:[NSDate dateWithTimeIntervalSinceReferenceDate:interval]];
	
	[self initWithTimeIntervalSinceReferenceDate: interval];
    [_timeZone release];
	_timeZone = [tz retain];
    
    return self;
}
    
-initWithString:(NSString *)string calendarFormat:(NSString *)format locale:(NSDictionary *)locale {
   NSMutableString* mu=[[string mutableCopy] autorelease];
   
	if ([mu rangeOfString:@"T"].location != NSNotFound)
 	  [mu replaceCharactersInRange:[mu rangeOfString:@"T"] withString:@" "];
   
    NSDateFormatter *dateFormatter = [[[NSDateFormatter allocWithZone:NULL] initWithDateFormat:format allowNaturalLanguage:YES locale:locale] autorelease];
    NSString *error;
    
    [self autorelease];
    if ([dateFormatter getObjectValue:&self forString:mu errorDescription:&error]) {
        [self retain];	// getObjectValues are autoreleased
        return self;
    }
    
    return nil;
}

-initWithString:(NSString *)string calendarFormat:(NSString *)format {
    return [self initWithString:string calendarFormat:format locale:nil];
}

-initWithString:(NSString *)string {
    return [self initWithString:string calendarFormat:defaultCalendarDate];
}

-(void)dealloc {
    [_format release];
    [_timeZone release];

    [super dealloc];
}

+dateWithYear:(NSInteger)year month:(NSUInteger)month day:(NSUInteger)day
         hour:(NSUInteger)hour minute:(NSUInteger)minute second:(NSUInteger)second
     timeZone:(NSTimeZone *)timeZone {
    return [[[self allocWithZone:NULL] initWithYear:year month:month day:day hour:hour minute:minute second:second timeZone:timeZone] autorelease];;
}

+dateWithString:(NSString *)string calendarFormat:(NSString *)format
         locale:(NSDictionary *)locale {
    return [[[self allocWithZone:NULL] initWithString:string calendarFormat:format locale:locale] autorelease];
}

+dateWithString:(NSString *)string calendarFormat:(NSString *)format {
    return [[[self allocWithZone:NULL] initWithString:string calendarFormat:format] autorelease];
}

-(Class)classForCoder {
   return [NSCalendarDate class];
}

-(void)encodeWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    [coder encodeDouble:_timeInterval forKey:@"NS.time"];
    [coder encodeObject:_timeZone forKey:@"NS.timezone"];
    [coder encodeObject:_format forKey:@"NS.format"];
   }
   else {
    [coder encodeValueOfObjCType:@encode(double) at:&_timeInterval];
    [coder encodeObject:_timeZone];
    [coder encodeObject:_format];
   }
}

-initWithCoder:(NSCoder *)coder {

   if([coder allowsKeyedCoding]){
    _timeInterval=[coder decodeDoubleForKey:@"NS.time"];
    _timeZone=[[coder decodeObjectForKey:@"NS.timezone"] copy];
    _format=[[coder decodeObjectForKey:@"NS.format"] copy];
   }
   else {
    [coder decodeValueOfObjCType:@encode(double) at:&_timeInterval];
    _timeZone = [[coder decodeObject] retain];
    _format = [[coder decodeObject] retain];
   }
   
   return self;
}

-(NSString *)calendarFormat {
   return _format;
}

-(NSTimeZone *)timeZone {
   return _timeZone;
}


-(void)setCalendarFormat:(NSString *)format {
    if (format != nil) {
   format=[format copy];
   [_format release];
   _format=format;
}
    else {
        [_format release];
        _format=[defaultCalendarDate copy];
    }
}

-(void)setTimeZone:(NSTimeZone *)timeZone {
   [timeZone retain];
   [_timeZone release];
   _timeZone=timeZone;
}

-(NSTimeInterval)timeZoneAdjustedInterval {
   return NSMoveIntervalFromTimeZoneToGMT(_timeInterval,_timeZone);
}

-(NSInteger)secondOfMinute {
   return NSSecondFromTimeInterval([self timeZoneAdjustedInterval]);
}

-(NSInteger)minuteOfHour {
   return NSMinuteFromTimeInterval([self timeZoneAdjustedInterval]);
}

-(NSInteger)hourOfDay {
   return NS24HourFromTimeInterval([self timeZoneAdjustedInterval]);
}

-(NSInteger)dayOfWeek {
   return NSWeekdayFromTimeInterval([self timeZoneAdjustedInterval]);
}

-(NSInteger)dayOfMonth {
   return NSDayOfMonthFromTimeInterval([self timeZoneAdjustedInterval]);
}

-(NSInteger)dayOfYear {
   return NSDayOfYearFromTimeInterval([self timeZoneAdjustedInterval]);
}

-(NSInteger)monthOfYear {
   return NSMonthFromTimeInterval([self timeZoneAdjustedInterval]);
}

-(NSInteger)yearOfCommonEra {
   return NSYearFromTimeInterval([self timeZoneAdjustedInterval]);
}

-(NSInteger)dayOfCommonEra {
    return NSDayOfCommonEraFromTimeInterval([self timeZoneAdjustedInterval]);
}

-(void)years:(NSInteger *)yearsp months:(NSInteger *)monthsp days:(NSInteger *)daysp
  hours:(NSInteger *)hoursp minutes:(NSInteger *)minutesp seconds:(NSInteger *)secondsp
  sinceDate:(NSCalendarDate *)since {
    NSTimeInterval delta;
    BOOL inverted = NO;
    NSInteger carry = 0;
    
    if ([self timeIntervalSinceReferenceDate] > [since timeIntervalSinceReferenceDate]) {
        delta = [self timeIntervalSinceReferenceDate]-[since timeIntervalSinceReferenceDate];
    }
    else {
        delta = [since timeIntervalSinceReferenceDate]-[self timeIntervalSinceReferenceDate];
        inverted  = YES;
    }
    carry = NSYearFromTimeInterval(delta);
    (carry)-=2001;
    if(inverted)
        carry *= -1;
    if (yearsp!=NULL) {
        *yearsp = carry;
        carry = 0;
    }    
    
    carry = (NSMonthFromTimeInterval(delta) -1) * (inverted ? -1 : 1) + carry * 12;
    if (monthsp!=NULL) {
        *monthsp = carry;
    }   
    if(inverted) {
        carry = (NSDayOfCommonEraFromTimeInterval([[since dateByAddingYears:yearsp!=NULL?*yearsp:0 months:monthsp!=NULL?*monthsp:0 days:0 hours:0 minutes:0 seconds:0] timeIntervalSinceReferenceDate]) - NSDayOfCommonEraFromTimeInterval([self timeIntervalSinceReferenceDate]) - 1) * -1;        
    }
    else
    {
        carry = NSDayOfCommonEraFromTimeInterval([[self dateByAddingYears:yearsp!=NULL?*yearsp*-1:0 months:monthsp!=NULL?*monthsp*-1:0 days:0 hours:0 minutes:0 seconds:0] timeIntervalSinceReferenceDate]) - NSDayOfCommonEraFromTimeInterval([since timeIntervalSinceReferenceDate]) -1;        
    }
    
    if (daysp!=NULL) {
        *daysp = carry;
        carry = 0;
    }
    
    if (inverted) {
        carry = carry * 24 - NS24HourFromTimeInterval(delta);
    }
    else {
        carry = carry * 24 + NS24HourFromTimeInterval(delta);
    }
    
    if (hoursp!=NULL) {
        *hoursp = carry;
        carry = 0;
    }
    
    if(inverted) {
        carry = carry * 60 - NSMinuteFromTimeInterval(delta);
    }
    else {
        carry = carry * 60 + NSMinuteFromTimeInterval(delta);
    }
    if (minutesp!=NULL) {
        *minutesp = carry;
        carry = 0;   
    }
    
    if (secondsp!=NULL) {
            if(inverted) {
                *secondsp = carry * 60 - NSSecondFromTimeInterval(delta);
            }
            else {
                *secondsp = carry * 60 + NSSecondFromTimeInterval(delta);
            }
    }
}



// Might be a little off with daylight savings, etc., needs to be verified
-(NSCalendarDate *)dateByAddingYears:(NSInteger)yearDelta months:(NSInteger)monthDelta
  days:(NSInteger)dayDelta hours:(NSInteger)hourDelta minutes:(NSInteger)minuteDelta seconds:(NSInteger)secondDelta {
    NSInteger year=NSYearFromTimeInterval(_timeInterval);
    NSInteger month=NSMonthFromTimeInterval(_timeInterval);
    NSInteger day=NSDayOfMonthFromTimeInterval(_timeInterval);
    NSInteger hour=NS24HourFromTimeInterval(_timeInterval);
    NSInteger minute=NSMinuteFromTimeInterval(_timeInterval);
    NSInteger second=NSSecondFromTimeInterval(_timeInterval);
    
    second+=secondDelta;
    minute+=second/60;
    second %= 60;
    if (second < 0) {
        second+=60;
        minute--;
    }
    
    minute+=minuteDelta;
    hour+=minute/60;
    minute %= 60;
    if (minute < 0) {
        minute+=60;
        hour--;
    }
    
    hour+= hourDelta;
    day+=hour/24;
    hour%=24;
    if (hour < 0) {
        day--;
        hour+=24;
    }
    
    day+=dayDelta;
    if (day > 28) {
        int i = NSNumberOfDaysInMonthOfYear(month, year);
        while (day > i) {
            day -= i;
            if (month < 12) {
                month++;
            }
            else {
                year++;
                month = 1;
            }
            i = NSNumberOfDaysInMonthOfYear(month, year);
        }
    }
    else {
        while (day <= 0) {
            if (month == 1) {
                month=12;
                year--;
            }
            else {
                month--;
            }
            day += NSNumberOfDaysInMonthOfYear(month, year);
        }
    }
    
    month+=monthDelta;
    
    while (month > 12) {
        month-=12;
        year++;
    }
    
    while (month < 1) {
        month+=12;
        year--;
    }
    
    year += yearDelta;
    
    NSCalendarDate *resultDate = [NSCalendarDate dateWithTimeIntervalSinceReferenceDate:NSTimeIntervalWithComponents(year,month,day,hour,minute,second,0)];
    
    [resultDate setTimeZone:_timeZone];
    
    return resultDate;
}

-(NSString *)descriptionWithCalendarFormat:(NSString *)format locale:(NSDictionary *)locale {
   return NSStringWithDateFormatLocale(_timeInterval, format,locale,_timeZone);
}

-(NSString *)descriptionWithCalendarFormat:(NSString *)format {
   return NSStringWithDateFormatLocale(_timeInterval,format,nil,_timeZone);
}

-(NSString *)descriptionWithLocale:(NSDictionary *)locale {
   return NSStringWithDateFormatLocale(_timeInterval,_format,locale,_timeZone);
}

-(NSString *)description {
   return NSStringWithDateFormatLocale(_timeInterval,_format,nil,_timeZone);
}

@end

