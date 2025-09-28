/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSCalendar.h>
#import <Foundation/NSDateComponents.h>
#import <Foundation/NSTimeZone.h>
#import <Foundation/NSLocale.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSDateFormatter.h>

NSString * const NSGregorianCalendar=@"NSGregorianCalendar";

@implementation NSCalendar 

-copyWithZone:(NSZone *)zone {
   NSCalendar *result=NSCopyObject(self,0,zone);
   
   result->_identifier=[_identifier copy];
   result->_timeZone=[_timeZone copy];
   result->_locale=[_locale copy];
   
   return result;
}

+currentCalendar {
   return [[[self alloc] initWithCalendarIdentifier:NSGregorianCalendar] autorelease];
}

-initWithCalendarIdentifier:(NSString *)identifier {
   _identifier=[identifier copy];
   _timeZone=[[NSTimeZone defaultTimeZone] copy];
   return self;
}

-(void)dealloc {
   [_identifier release];
   [_timeZone release];
   [super dealloc];
}

-(NSString *)calendarIdentifier {
   return _identifier;
}

-(NSUInteger)firstWeekday {
   return _firstWeekday;
}

-(NSUInteger)minimumDaysInFirstWeek {
   return _minimumDaysInFirstWeek;
}

-(NSTimeZone *)timeZone {
   return _timeZone;
}

-(NSLocale *)locale {
   return _locale;
}

-(void)setFirstWeekday:(NSUInteger)weekday {
   _firstWeekday=weekday;
}

-(void)setMinimumDaysInFirstWeek:(NSUInteger)days {
   _minimumDaysInFirstWeek=days;
}

-(void)setTimeZone:(NSTimeZone *)timeZone {
   timeZone=[timeZone retain];
   [_timeZone release];
   _timeZone=timeZone;
}

-(void)setLocale:(NSLocale *)locale {
   locale=[locale retain];
   [_locale release];
   _locale=locale;
}

-(NSRange)minimumRangeOfUnit:(NSCalendarUnit)unit {
   NSUnimplementedMethod();
   return NSMakeRange(0,0);
}

-(NSRange)maximumRangeOfUnit:(NSCalendarUnit)unit {
   NSUnimplementedMethod();
   return NSMakeRange(0,0);
}

-(NSRange)rangeOfUnit:(NSCalendarUnit)unit inUnit:(NSCalendarUnit)inUnit forDate:(NSDate *)date {
   NSUnimplementedMethod();
   return NSMakeRange(0,0);
}

-(NSUInteger)ordinalityOfUnit:(NSCalendarUnit)unit inUnit:(NSCalendarUnit)inUnit forDate:(NSDate *)date {
   NSUnimplementedMethod();
   return 0;
}

-(NSDateComponents *)components:(NSUInteger)flags fromDate:(NSDate *)date {
   NSDateComponents *result=[[[NSDateComponents alloc] init] autorelease];
   NSTimeInterval interval=[date timeIntervalSinceReferenceDate];
   
  interval=NSMoveIntervalFromTimeZoneToGMT(interval,[NSTimeZone localTimeZone]);
   
   if(flags&NSEraCalendarUnit)
    NSUnimplementedMethod();
   if(flags&NSYearCalendarUnit)
    [result setYear:NSYearFromTimeInterval(interval)];
   if(flags&NSMonthCalendarUnit)
    [result setMonth:NSMonthFromTimeInterval(interval)];
   if(flags&NSDayCalendarUnit)
    [result setDay:NSDayOfMonthFromTimeInterval(interval)];
   if(flags&NSHourCalendarUnit)
    [result setHour:NS24HourFromTimeInterval(interval)];
   if(flags&NSMinuteCalendarUnit)
    [result setMinute:NSMinuteFromTimeInterval(interval)];
   if(flags&NSSecondCalendarUnit)
    [result setSecond:NSSecondFromTimeInterval(interval)];
   if(flags&NSWeekCalendarUnit)
    NSUnimplementedMethod();
   if(flags&NSWeekdayCalendarUnit)
    [result setWeekday:NSWeekdayFromTimeInterval(interval)];
   if(flags&NSWeekdayOrdinalCalendarUnit)
    NSUnimplementedMethod();
#if 0
   if(flags&NSQuarterCalendarUnit)
    NSUnimplementedMethod();
#endif
    
   return result;
}

-(NSDateComponents *)components:(NSUInteger)flags fromDate:(NSDate *)fromDate toDate:(NSDate *)toDate options:(NSUInteger)options {
   NSUnimplementedMethod();
   return nil;
}

-(NSDate *)dateByAddingComponents:(NSDateComponents *)components toDate:(NSDate *)date options:(NSUInteger)options {
   NSUnimplementedMethod();
   return nil;
}

-(NSDate *)dateFromComponents:(NSDateComponents *)components {
   NSInteger year=0;
   NSInteger month=0;
   NSInteger day=0;
   NSInteger hour=0;
   NSInteger minute=0;
   NSInteger second=0;
   NSInteger milliseconds=0;
   NSInteger check;

// FIXME: doesn't handle all components
   
   if((check=[components year])!=NSUndefinedDateComponent)
    year=check;
   if((check=[components month])!=NSUndefinedDateComponent)
    month=check;
   if((check=[components day])!=NSUndefinedDateComponent)
    day=check;
   if((check=[components hour])!=NSUndefinedDateComponent)
    hour=check;
   if((check=[components minute])!=NSUndefinedDateComponent)
    minute=check;
   if((check=[components second])!=NSUndefinedDateComponent)
    second=check;
    
   NSTimeInterval interval=NSTimeIntervalWithComponents(year,month,day,hour,minute,second,milliseconds);

   interval=NSMoveIntervalFromGMTToTimeZone(interval,[NSTimeZone localTimeZone]);

   return [NSDate dateWithTimeIntervalSinceReferenceDate:interval];
}

@end
