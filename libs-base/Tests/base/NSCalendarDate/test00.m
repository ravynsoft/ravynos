#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSCalendarDate.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSTimeZone.h>

#include "./western.h"

@interface NSCalendarDate(TestAdditions)
-(BOOL) testDateValues: (int)y : (int)m : (int)d : (int)h : (int)i : (int)s;
@end
@implementation NSCalendarDate(TestAdditions)
-(BOOL) testDateValues: (int)y : (int)m : (int)d : (int)h : (int)i : (int)s
{
  return (y == [self yearOfCommonEra] && m == [self monthOfYear]
    && d == [self dayOfMonth] && h == [self hourOfDay]
    && i == [self minuteOfHour] && s == [self secondOfMinute]);
}
@end
int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *val1, *val2;
  NSCalendarDate *date1, *date2;
  NSDictionary	*locale;
  NSTimeZone	*tz;

  val1 = @"1999-12-31 23:59:59";
  val2 = @"%Y-%m-%d %H:%M:%S";

  /* Y2K checks */
  date1 = [NSCalendarDate calendarDate];
  PASS(date1 != nil && [date1 isKindOfClass: [NSCalendarDate class]],
    "+calendarDate works");

  date1 = [NSCalendarDate dateWithString: val1 calendarFormat: val2];
  PASS(date1 != nil, "+dateWithString:calendarFormat: works");

  locale = westernLocale();
  date1 = [NSCalendarDate dateWithString: @"Fri Oct 27 08:41:14GMT 2000"
			  calendarFormat: nil
				  locale: locale];
  PASS(date1 != nil,
    "+dateWithString:calendarFormat:locale: with nil format works");

  date1 = [NSCalendarDate dateWithString: @"1999-12-31 23:59:"
                          calendarFormat: val2
				  locale: locale];
  PASS(date1 == nil,
    "+dateWithString:calendarFormat:locale: objects to missing seconds");

  date1 = [NSCalendarDate dateWithString: @"1999-12-31 23::00"
                          calendarFormat: val2
				  locale: locale];
  PASS(date1 == nil,
    "+dateWithString:calendarFormat:locale: objects to missing minutes");

  date1 = [NSCalendarDate dateWithString: @"1999-12-31 :00:00"
                          calendarFormat: val2
				  locale: locale];
  PASS(date1 == nil,
    "+dateWithString:calendarFormat:locale: objects to missing hours");

  date1 = [NSCalendarDate dateWithString: @"1999-12-00 00:00:00"
                          calendarFormat: val2
				  locale: locale];
  PASS(date1 == nil,
    "+dateWithString:calendarFormat:locale: objects to zero day");

  date1 = [NSCalendarDate dateWithString: @"1999-00-01 00:00:00"
                          calendarFormat: val2
				  locale: locale];
  PASS(date1 == nil,
    "+dateWithString:calendarFormat:locale: objects to zero month");

  date1 = [NSCalendarDate dateWithString: @"1999-12-31 00:00:00"
                          calendarFormat: @"%Y-%m-%d %H:%M:%S %Z"
				  locale: locale];
  PASS(date1 == nil,
    "+dateWithString:calendarFormat:locale: objects to missing timezone");

  date1 = [NSCalendarDate dateWithString: @"1999-12-31 00:00:00 this_is_a_ridiculously_long_timezone_name_and_is_in_fact_designed_to_exceed_the_one_hundred_and_twenty_bytes_temporary_data_buffer_size_used_within_the_gnustep_base_method_which_parses_it"
                          calendarFormat: @"%Y-%m-%d %H:%M:%S %Z"
				  locale: locale];
  PASS(date1 == nil,
    "+dateWithString:calendarFormat:locale: objects to long timezone");

  date1 = [NSCalendarDate dateWithString: @"1999-12-31 00:00:00 GMT+0100"
                          calendarFormat: @"%Y-%m-%d %H:%M:%S %Z"
				  locale: locale];
  PASS(date1 != nil,
    "+dateWithString:calendarFormat:locale: handles GMT+0100 timezone");

  date1 = [NSCalendarDate dateWithString: @"1999-12-31 00:00:00 GMT-0100"
                          calendarFormat: @"%Y-%m-%d %H:%M:%S %Z"
				  locale: locale];
  PASS(date1 != nil,
    "+dateWithString:calendarFormat:locale: handles GMT-0100 timezone");

  date1 = [NSCalendarDate dateWithString: @"1970-01-01 00:00:00 GMT+1000"
                          calendarFormat: @"%Y-%m-%d %H:%M:%S %Z"
				  locale: locale];
  [date1 setTimeZone: [NSTimeZone timeZoneWithName: @"GMT"]];
  PASS_EQUAL([date1 description], @"1969-12-31 14:00:00 GMT+0000",
    "+dateWithString:calendarFormat:locale: handles GMT+1000 timezone");

  date1 = [NSCalendarDate dateWithString:
    @"1999-12-31 00:00:00 Africa/Addis_Ababa"
                          calendarFormat: @"%Y-%m-%d %H:%M:%S %Z"
				  locale: locale];
  PASS(date1 != nil,
    "+dateWithString:calendarFormat:locale: handles Africa/Addis_Ababa");

  date1 = [NSCalendarDate dateWithString: @"1999-12-31 23:59:59"
                          calendarFormat: val2
				  locale: locale];
  PASS([date1 testDateValues: 1999 : 12 : 31 : 23 : 59 : 59],
    "date check with %s", [[date1 description] UTF8String]);
  date2 = [date1 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 1];
  PASS([date2 testDateValues: 2000 : 01 : 01 : 00 : 00 : 00],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 1];
  PASS([date2 testDateValues: 2000 : 01 : 01 : 00 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 1 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2000 : 01 : 01 : 01 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: -2 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 1999 : 12 : 31 : 23 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  /* Y2K is a leap year checks */
  date2 = [NSCalendarDate dateWithString: @"2000-2-28 23:59:59"
                          calendarFormat: val2
				  locale: locale];

  PASS([date2 testDateValues: 2000 : 02 : 28 : 23 : 59 : 59],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 1];
  PASS([date2 testDateValues: 2000 : 02 : 29 : 00 : 00 : 00],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 1];
  PASS([date2 testDateValues: 2000 : 02 : 29 : 00 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 1 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2000 : 02 : 29 : 01 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: -2 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2000 : 02 : 28 : 23 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 5 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2000 : 02 : 29 : 04 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 1
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2001 : 03 : 01 : 04 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: -1 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2001 : 02 : 28 : 04 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 1
    months: 0 days: 1 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 03 : 01 : 04 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  /* 2004 is a year leap check */
  date2 = [NSCalendarDate dateWithString: @"2004-2-28 23:59:59"
                          calendarFormat: val2
				  locale: locale];
  PASS([date2 testDateValues: 2004 : 02 : 28 : 23 : 59 : 59],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 1];
  PASS([date2 testDateValues: 2004 : 02 : 29 : 00 : 00 : 00],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 1];
  PASS([date2 testDateValues: 2004 : 02 : 29 : 00 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);
  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 1 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2004 : 02 : 29 : 01 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: -2 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2004 : 02 : 28 : 23 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  /* 2100 is not a leap year */

  date2 = [NSCalendarDate dateWithString: @"2100-2-28 23:59:59"
			  calendarFormat: val2
				  locale: locale];
  PASS([date2 testDateValues: 2100 : 02 : 28 : 23 : 59 : 59],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 1];
  PASS([date2 testDateValues: 2100 : 03 : 01 : 00 : 00 : 00],
    "date check with %s", [[date2 description] UTF8String]);
  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 1];
  PASS([date2 testDateValues: 2100 : 03 : 01 : 00 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);
  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 1 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2100 : 03 : 01 : 01 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: -2 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2100 : 02 : 28 : 23 : 00 : 01],
    "date check with %s", [[date2 description] UTF8String]);

  /* daylight savings time checks */
  [NSTimeZone setDefaultTimeZone: [NSTimeZone timeZoneWithName: @"GB"]];

  date2 = [NSCalendarDate dateWithString: @"2002-3-31 00:30:00"
			  calendarFormat: val2
				  locale: locale];
  PASS([date2 testDateValues: 2002 : 03 : 31 : 00 : 30 : 00],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 1 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 03 : 31 : 02 : 30 : 00],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: -1 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 03 : 31 : 00 : 30 : 00],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 2 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 03 : 31 : 02 : 30 : 00],
    "date check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: -1 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 03 : 31 : 00 : 30 : 00],
    "date check with %s", [[date2 description] UTF8String]);
  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: -1 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 03 : 30 : 23 : 30 : 00],
    "date check with %s", [[date2 description] UTF8String]);
  /* End daylight savings checks */

  /* Seconds calculation checks */
  date2 = [NSCalendarDate dateWithString: @"2002-10-27 00:30:00"
			  calendarFormat: val2
				  locale: locale];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date second calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: -1];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 29 : 59],
    "date second calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 1];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date second calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 2];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 02],
    "date second calculation check with %s", [[date2 description] UTF8String]);

  /* Minutes calculation checks */
  date2 = [NSCalendarDate dateWithString: @"2002-10-27 00:30:00"
			  calendarFormat: val2
				  locale: locale];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date minute calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: -1 seconds: 0];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 29 : 00],
    "date minute calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 1 seconds: 0];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date minute calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 1 seconds: 0];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 31 : 00],
    "date minute calculation check with %s", [[date2 description] UTF8String]);

  /* Hour calculation checks */
  date2 = [NSCalendarDate dateWithString: @"2002-10-27 00:30:00"
			  calendarFormat: val2
				  locale: locale];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date hour calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: -1 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 10 : 26 : 23 : 30 : 00],
    "date hour calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 1 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date hour calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 2 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 02 : 30 : 00],
    "date hour calculation check with %s", [[date2 description] UTF8String]);

  /* Days calculation checks */
  date2 = [NSCalendarDate dateWithString: @"2002-10-27 00:30:00"
			  calendarFormat: val2
				  locale: locale];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date day calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: -1 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 10 : 26 : 00 : 30 : 00],
    "date day calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 1 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date day calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 2 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 10 : 29 : 00 : 30 : 00],
    "date day calculation check with %s", [[date2 description] UTF8String]);

  date2 = [NSCalendarDate dateWithString: @"2002-10-27 00:00:00"
			  calendarFormat: val2
				  locale: locale];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 00 : 00],
    "date day calculation check with %s", [[date2 description] UTF8String]);

  date2 = [NSCalendarDate dateWithString: @"2002-10-26 24:00:00"
			  calendarFormat: val2
				  locale: locale];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 00 : 00],
    "date day calculation check with 2002-10-26 24:00:00");

  /* Months calculation checks */
  date2 = [NSCalendarDate dateWithString: @"2002-10-27 00:30:00"
			  calendarFormat: val2
				  locale: locale];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date month calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: -1 days: 0 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 9 : 27 : 00 : 30 : 00],
    "date month calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 1 days: 0 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date month calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 0
    months: 2 days: 0 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 12 : 27 : 00 : 30 : 00],
    "date month calculation check with %s", [[date2 description] UTF8String]);

  /* Years calculation checks */
  date2 = [NSCalendarDate dateWithString: @"2002-10-27 00:30:00"
			  calendarFormat: val2
				  locale: locale];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date year calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: -1
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2001 : 10 : 27 : 00 : 30 : 00],
    "date year calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 1
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2002 : 10 : 27 : 00 : 30 : 00],
    "date year calculation check with %s", [[date2 description] UTF8String]);

  date2 = [date2 dateByAddingYears: 2
    months: 0 days: 0 hours: 0 minutes: 0 seconds: 0];
  PASS([date2 testDateValues: 2004 : 10 : 27 : 00 : 30 : 00],
    "date year calculation check with %s", [[date2 description] UTF8String]);

  [NSTimeZone setDefaultTimeZone: [NSTimeZone timeZoneWithName: @"GMT"]];

  tz = [NSTimeZone timeZoneWithName: @"GB"];
  date2 = [NSCalendarDate dateWithYear: 2006
    month: 10 day: 1 hour: 0 minute: 0 second: 0 timeZone: tz];
  date2 = [date2 dateByAddingYears: 0
    months: 1 days: 0 hours: 2 minutes: 10 seconds: 0];
  PASS([date2 testDateValues: 2006 : 11 : 1 : 2 : 10 : 00],
    "date year calculation check with %s", [[date2 description] UTF8String]);
  PASS([[date2 timeZone] isEqual: tz],
    "date year calculation preserves timezone");

  tz = [NSTimeZone timeZoneForSecondsFromGMT: 3600];
  date2 = [NSCalendarDate dateWithYear: 2016
    month: 1 day: 6 hour: 1 minute: 0 second: 0 timeZone: tz];
  date2 = [date2 dateByAddingYears: 0
    months: 0 days: 0 hours: 0 minutes: 0 seconds: -3600];
  PASS([date2 testDateValues: 2016 : 1 : 6 : 00 : 00 : 00],
    "date year calculation check with %s", [[date2 description] UTF8String]);

  [arp release]; arp = nil;
  return 0;
}
