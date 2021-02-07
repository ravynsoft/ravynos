#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSLocale.h>
#import <Foundation/NSCalendar.h>
#import "ObjectTesting.h"

#if	defined(GS_USE_ICU)
#define	NSCALENDAR_SUPPORTED	GS_USE_ICU
#else
#define	NSCALENDAR_SUPPORTED	1 /* Assume Apple support */
#endif

int main(void)
{
  START_SET("NSCalendar create")
  NSCalendar *cal;

  if (!NSCALENDAR_SUPPORTED)
    SKIP("NSCalendar not supported\nThe ICU library was not available when GNUstep-base was built")
  
  cal = [NSCalendar currentCalendar];
  PASS (cal != nil, "+currentCalendar returns non-nil");
  TEST_FOR_CLASS(@"NSCalendar", cal, "+currentCalendar return a NSCalendar");
  
  cal = [[NSCalendar alloc] initWithCalendarIdentifier: NSGregorianCalendar];
  PASS (cal != nil, "-initWithCalendarIdentifier: return non-nil");
  TEST_FOR_CLASS(@"NSCalendar", cal,
    "-initWithCalendarIdentifier: return a NSCalendar");
  RELEASE(cal);
  
  END_SET("NSCalendar create")
  return 0;
}
