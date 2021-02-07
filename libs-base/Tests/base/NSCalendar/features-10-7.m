#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/NSCalendar.h>
#import <Foundation/NSTimeZone.h>
#import <Foundation/NSLocale.h>
#include <stdio.h>

#if	defined(GS_USE_ICU)
#define	NSCALENDAR_SUPPORTED	GS_USE_ICU
#else
#define	NSCALENDAR_SUPPORTED	1 /* Assume Apple support */
#endif

int main()
{
  NSDateComponents *comps;
  NSCalendar *cal;
  NSDate *date;
  NSDate *date2;
  NSDate *date3;
  NSDate *date4;
  
  START_SET("NSCalendar 10.7 features")
  if (!NSCALENDAR_SUPPORTED)
    SKIP("NSCalendar not supported\nThe ICU library was not available when GNUstep-base was built")
  
  cal = [[NSCalendar alloc] initWithCalendarIdentifier: NSGregorianCalendar];
  [cal setFirstWeekday: 1];
  [cal setMinimumDaysInFirstWeek: 4];
  
  date = [NSDate dateWithString: @"2012-12-31 13:57:00 +0100"];

  comps = [cal components: NSYearForWeekOfYearCalendarUnit
                         | NSWeekOfYearCalendarUnit | NSWeekOfMonthCalendarUnit
                 fromDate: date];
  PASS([comps weekOfMonth] == 5, "-weekOfMonth returns the correct week");
  PASS([comps weekOfYear] == 1, "-weekOfYear returns the correct week");
  PASS([comps yearForWeekOfYear] == 2013,
    "-yearForWeekOfYear returns the correct year");
  
  comps = [[NSDateComponents alloc] init];
  [comps setCalendar: cal];
  [comps setTimeZone: [NSTimeZone timeZoneForSecondsFromGMT: 3600]];
  [comps setHour: 13];
  [comps setMinute: 57];
  [comps setDay: 31];
  [comps setMonth: 12];
  [comps setYear: 2012];
  
  date2 = [comps date];
  
  PASS_EQUAL(date, date2, "-[NSDateComponents date] returns the correct date");

  date3 = [NSDate dateWithString: @"2012-12-31 13:57:00 +0200"];
  [comps setTimeZone: [NSTimeZone timeZoneForSecondsFromGMT: 7200]];
  date4 = [cal dateFromComponents: comps];

  PASS_EQUAL(date3, date4, "-[NSCalendar dateFromComponents:] respects the time zone");

  RELEASE(comps);
  RELEASE(cal);
  
  END_SET("NSCalendar 10.7 features")
  return 0;
}
