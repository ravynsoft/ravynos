#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/NSCalendar.h>

#if	defined(GS_USE_ICU)
#define	NSCALENDAR_SUPPORTED	GS_USE_ICU
#else
#define	NSCALENDAR_SUPPORTED	1 /* Assume Apple support */
#endif

int main()
{  
  START_SET("NSCalendar basic")
  if (!NSCALENDAR_SUPPORTED)
    SKIP("NSCalendar not supported\nThe ICU library was not available when GNUstep-base was built")
  id testObj = [NSCalendar currentCalendar];

  test_NSObject(@"NSCalendar", [NSArray arrayWithObject: testObj]);

#if     __APPLE__ 
  test_keyed_NSCoding([NSArray arrayWithObject: testObj]);
#else
  test_NSCoding([NSArray arrayWithObject: testObj]);
#endif

  test_NSCopying(@"NSCalendar", @"NSCalendar",
    [NSArray arrayWithObject: testObj], NO, NO);
  
/* Test first weekday
 */
  [testObj setFirstWeekday: 3];
  PASS([testObj firstWeekday] == 3, "-firstWeekday returns the correct day");
  [testObj setFirstWeekday: 1];
  PASS([testObj firstWeekday] == 1, "-setFirstWeekday: works");

  [testObj setFirstWeekday: 0];
  PASS([testObj firstWeekday] == 0, "-setFirstWeekday: can set zero");
  [testObj setFirstWeekday: 10];
  PASS([testObj firstWeekday] == 10, "-setFirstWeekday: can set ten");

/* Test minimum days in first week
 */
  [testObj setMinimumDaysInFirstWeek: 1];
  PASS([testObj minimumDaysInFirstWeek] == 1,
    "-minimumDaysInFirstWeek returns the correct count");
  [testObj setMinimumDaysInFirstWeek: 4];
  PASS([testObj minimumDaysInFirstWeek] == 4,
    "-setMinimumDaysInFirstWeek: works");

  [testObj setMinimumDaysInFirstWeek: 0];
  PASS([testObj minimumDaysInFirstWeek] == 0,
    "-setMinimumDaysInFirstWeek: can set zero");
  [testObj setMinimumDaysInFirstWeek: 10];
  PASS([testObj minimumDaysInFirstWeek] == 10,
    "-setMinimumDaysInFirstWeek: can set ten");


  END_SET("NSCalendar basic")
  return 0;
}
