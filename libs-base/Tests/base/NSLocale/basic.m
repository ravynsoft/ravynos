#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/NSLocale.h>

#if	defined(GS_USE_ICU)
#define	NSLOCALE_SUPPORTED	GS_USE_ICU
#else
#define	NSLOCALE_SUPPORTED	1 /* Assume Apple support */
#endif

int main()
{  
  START_SET("NSLocale")
  id testObj;

  if (!NSLOCALE_SUPPORTED)
    SKIP("NSLocale not supported\nThe ICU library was not available when GNUstep-base was built")

  testObj = [NSLocale currentLocale];
  test_NSObject(@"NSLocale", [NSArray arrayWithObject: testObj]);
  test_keyed_NSCoding([NSArray arrayWithObject: testObj]);
  test_NSCopying(@"NSLocale", @"NSLocale",
    [NSArray arrayWithObject: testObj], NO, NO);
  
  END_SET("NSLocale")
  return 0;
}
