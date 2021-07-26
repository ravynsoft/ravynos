/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSDate.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSPlatform.h>
#import <Foundation/NSDate_timeInterval.h>
#import <Foundation/NSDateFormatter.h>

const NSTimeInterval NSTimeIntervalSince1970 = (NSTimeInterval)978307200.0;

#define DEFAULT_CALENDAR_FORMAT		@"%Y-%m-%d %H:%M:%S %z"

@implementation NSDate

+allocWithZone:(NSZone *)zone {
   if(self==[NSDate class])
    return NSAllocateObject([NSDate_timeInterval class],0,zone);

   return NSAllocateObject(self,0,zone);
}

+(NSTimeInterval)timeIntervalSinceReferenceDate {
   return NSPlatformTimeIntervalSinceReferenceDate();
}

+distantPast {
   static NSDate *staticInstance=nil;
   if(!staticInstance)
      staticInstance=[[self allocWithZone:NULL] 
                      initWithTimeIntervalSinceReferenceDate:-(2010.0L*365.0*24.0*60.0*60.0)];
   return staticInstance;
}

+distantFuture {
   static NSDate *staticInstance=nil;
   if(!staticInstance)
      staticInstance=[[self allocWithZone:NULL] 
                       initWithTimeIntervalSinceReferenceDate:2010.0L*365.0*24.0*60.0*60.0];
   return staticInstance;
}

-init {
   return [self initWithTimeIntervalSinceReferenceDate:
     NSPlatformTimeIntervalSinceReferenceDate()];
}


-initWithString:(NSString *)string {
    [self dealloc];
    return [[NSCalendarDate allocWithZone:NULL] initWithString:string];
}

-initWithTimeIntervalSinceReferenceDate:(NSTimeInterval)seconds {
   NSInvalidAbstractInvocation();
   return nil;
}

-initWithTimeIntervalSinceNow:(NSTimeInterval)seconds {
   return [self initWithTimeIntervalSinceReferenceDate:
     NSPlatformTimeIntervalSinceReferenceDate()+seconds];
}

-initWithTimeIntervalSince1970:(NSTimeInterval)seconds {
    return [self initWithTimeIntervalSinceReferenceDate:
            -NSTimeIntervalSince1970+seconds];
}

-initWithTimeInterval:(NSTimeInterval)seconds sinceDate:(NSDate *)other {
   return [self initWithTimeIntervalSinceReferenceDate:
     [other timeIntervalSinceReferenceDate]+seconds];
}

+date {
   return [[[self allocWithZone:NULL] initWithTimeIntervalSinceReferenceDate:
     NSPlatformTimeIntervalSinceReferenceDate()] autorelease];
}
    
+dateWithString:(NSString *)string {
   return [[[NSCalendarDate allocWithZone:NULL] initWithString:string] autorelease];
}

+dateWithTimeIntervalSinceReferenceDate:(NSTimeInterval)seconds {
   return [[[self allocWithZone:NULL] initWithTimeIntervalSinceReferenceDate:
     seconds] autorelease];
}

+dateWithTimeIntervalSinceNow:(NSTimeInterval)seconds {
   return [[[self allocWithZone:NULL] initWithTimeIntervalSinceNow:seconds] autorelease];
}

+dateWithTimeIntervalSince1970:(NSTimeInterval)seconds {
   return [[[self allocWithZone:NULL] initWithTimeIntervalSince1970:seconds] autorelease];
}


-copyWithZone:(NSZone *)zone {
    return [self retain];
}

-(Class)classForCoder {
   return objc_lookUpClass("NSDate");
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSTimeInterval interval=[self timeIntervalSinceReferenceDate];

   [coder encodeValueOfObjCType:@encode(NSTimeInterval) at:&interval];
}

-initWithCoder:(NSCoder *)coder {
   NSTimeInterval interval=0;
   
   if(![coder allowsKeyedCoding]){
    [coder decodeValueOfObjCType:@encode(NSTimeInterval) at:&interval];
   }
   else {
    interval=[coder decodeDoubleForKey:@"NS.time"];
   }
   return [self initWithTimeIntervalSinceReferenceDate:interval];
}

-(NSTimeInterval)timeIntervalSinceReferenceDate {
    NSInvalidAbstractInvocation();
    return 0;
}


-(NSTimeInterval)timeIntervalSinceDate:(NSDate *)other {
   return [self timeIntervalSinceReferenceDate]-
     [other timeIntervalSinceReferenceDate];
}


-(NSTimeInterval)timeIntervalSinceNow {
   return [self timeIntervalSinceReferenceDate]-
     [NSDate timeIntervalSinceReferenceDate];
}


-(NSTimeInterval)timeIntervalSince1970 {
   return [self timeIntervalSinceReferenceDate]+NSTimeIntervalSince1970;
}

-(NSUInteger)hash {
	return (NSUInteger)[self timeIntervalSinceReferenceDate];
}

-(BOOL)isEqual:other {
   if(self==other)
    return YES;

   if(![other isKindOfClass:[NSDate class]])
    return NO;

   return [self isEqualToDate:other];
}


-(BOOL)isEqualToDate:(NSDate *)other {
   if(self==other)
    return YES;

   return [self timeIntervalSinceReferenceDate]==
          [other timeIntervalSinceReferenceDate];
}

-(NSComparisonResult)compare:(NSDate *)other {
   NSTimeInterval interval=[self timeIntervalSinceDate:other];

   if(interval<0)
    return NSOrderedAscending;

   if(interval>0)
    return NSOrderedDescending;

   return NSOrderedSame;
}

-(NSDate *)earlierDate:(NSDate *)other {
   NSTimeInterval interval=[self timeIntervalSinceDate:other];
   return (interval<0)?self:other;
}

-(NSDate *)laterDate:(NSDate *)other {
   NSTimeInterval interval=[self timeIntervalSinceDate:other];
   return (interval>0)?self:other;
}

-addTimeInterval:(NSTimeInterval)seconds {
   return [[self class] dateWithTimeIntervalSinceReferenceDate:
     [self timeIntervalSinceReferenceDate]+seconds];
}

-dateByAddingTimeInterval:(NSTimeInterval)seconds {
    return [[self class] dateWithTimeIntervalSinceReferenceDate:
            [self timeIntervalSinceReferenceDate]+seconds];
}

-(NSCalendarDate *)dateWithCalendarFormat:(NSString *)format timeZone:(NSTimeZone *)timeZone {
    NSCalendarDate *date = [NSCalendarDate dateWithTimeIntervalSinceReferenceDate:[self timeIntervalSinceReferenceDate]];

    if (format == nil)
        format = DEFAULT_CALENDAR_FORMAT;
    if (timeZone == nil)
        timeZone = [NSTimeZone defaultTimeZone];

    [date setCalendarFormat:format];
    [date setTimeZone:timeZone];

    return date;
}

-(NSString *)description {
   return [self descriptionWithLocale:nil];
}

-(NSString *)descriptionWithLocale:(NSDictionary *)locale {
    return NSStringWithDateFormatLocale([self timeIntervalSinceReferenceDate], DEFAULT_CALENDAR_FORMAT, locale, [NSTimeZone defaultTimeZone]);
}

-(NSString *)descriptionWithCalendarFormat:(NSString *)format timeZone:(NSTimeZone *)timeZone locale:(NSDictionary *)locale {
    if (format == nil)
        format = DEFAULT_CALENDAR_FORMAT;
    if (timeZone == nil)
        timeZone = [NSTimeZone defaultTimeZone];

    // nil locale is handled within this function
    return NSStringWithDateFormatLocale([self timeIntervalSinceReferenceDate], format,locale,timeZone);
}

@end

@implementation NSDate(NSCalendarDateExtras)

+ (id)dateWithNaturalLanguageString:(NSString *)string {
   NSUnimplementedMethod();
}

@end
