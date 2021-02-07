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
  
  START_SET("NSCalendar date component differences")
  if (!NSCALENDAR_SUPPORTED)
    SKIP("NSCalendar not supported\nThe ICU library was not available when GNUstep-base was built")
  
  cal = [[NSCalendar alloc] initWithCalendarIdentifier: NSGregorianCalendar];
  [cal setFirstWeekday: 1];
  
  date = [NSDate dateWithString: @"2015-01-01 01:01:01 +0100"];
  date2 = [NSDate dateWithString: @"2015-02-03 04:05:06 +0100"];

  comps = [cal components:
    NSYearCalendarUnit | NSMonthCalendarUnit | NSDayCalendarUnit
    | NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit
                 fromDate: date
                   toDate: date2
                  options: 0];
  if (nil == comps)
    {
      SKIP("-components:fromDate:toDate:options: not implementaed. The ICU library was not available (or too old) when GNUstep-base was built")
    }
  PASS([comps year] == 0, "year difference correct");
  PASS([comps month] == 1, "month difference correct");
  PASS([comps day] == 2, "day difference correct");
  PASS([comps hour] == 3, "hour difference correct");
  PASS([comps minute] == 4, "minute difference correct");
  PASS([comps second] == 5, "second difference correct");
  
  comps = [cal components: NSDayCalendarUnit
                 fromDate: date
                   toDate: date2
                  options: 0];
  PASS([comps month] == NSNotFound, "no month returned if not requested");
  PASS([comps day] == 33, "day difference without larger unit correct");
  
  
  RELEASE(cal);
  
  END_SET("NSCalendar date component differences")
  return 0;
}
