#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSCalendarDate.h>
#import <Foundation/NSString.h>
#import <Foundation/NSTimeZone.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *val;
  NSDate *date1;
  NSCalendarDate *date2;

  val = @"2000-10-19 00:00:00 +0000";
  date1 = [NSDate date];
  PASS(date1 != nil && [date1 isKindOfClass:[NSDate class]],
       "+date works");
  date1 = [NSDate dateWithString:val];
  PASS(date1 != nil && [date1 isKindOfClass:[NSDate class]],
       "+dateWithString works");

  date2 = [NSCalendarDate dateWithTimeIntervalSinceReferenceDate: 
   			    [date1 timeIntervalSinceReferenceDate]];
  PASS(date2 != nil && [date2 isKindOfClass:[NSDate class]],
       "+dateWithTimeIntervalSinceReferenceDate: works");
  // Make sure we get day in correct zone.
  [date2 setTimeZone: [NSTimeZone timeZoneForSecondsFromGMT: 0]];
  PASS([date2 dayOfMonth] == 19, "+dateWithString makes correct day");
  PASS([date2 monthOfYear] == 10, "+dateWithString makes correct month");
  PASS([date2 yearOfCommonEra] == 2000, "+dateWithString makes correct year");
  
  date1 = [NSDate dateWithTimeIntervalSinceNow:0];
  PASS(date1 != nil && [date1 isKindOfClass:[NSDate class]],
        "+dateWithTimeIntervalSinceNow: works");
  
  date1 = [NSDate dateWithTimeIntervalSince1970:0];
  PASS(date1 != nil && [date1 isKindOfClass:[NSDate class]],
        "+dateWithTimeIntervalSince1970: works");
  
  date1 = [NSDate dateWithTimeIntervalSinceReferenceDate:0];
  PASS(date1 != nil && [date1 isKindOfClass:[NSDate class]],
        "+dateWithTimeIntervalSinceReferenceDate: works");
  
  date1 = [NSDate distantFuture];
  PASS(date1 != nil && [date1 isKindOfClass:[NSDate class]],
       "+distantFuture works");
  
  date1 = [NSDate distantPast];
  PASS(date1 != nil && [date1 isKindOfClass:[NSDate class]],
       "+distantPast works");
  
  [arp release]; arp = nil;
  return 0;
}
