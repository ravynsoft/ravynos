#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSPropertyList.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSDecimalNumber.h>

#include <stdlib.h>
#include <limits.h>

#if	!defined(LLONG_MAX)
#  if	defined(__LONG_LONG_MAX__)
#    define LLONG_MAX __LONG_LONG_MAX__
#    define LLONG_MIN	(-LLONG_MAX-1)
#    define ULLONG_MAX	(LLONG_MAX * 2ULL + 1)
#  else
#    error Neither LLONG_MAX nor __LONG_LONG_MAX__ found
#  endif
#endif

int main()
{
  START_SET("NSNumber")
    NSNumber	*n;

    START_SET("not-a-number checks")

      NSNumber	*nan = [NSDecimalNumber notANumber];

      PASS(YES == [nan isEqualToNumber: nan], "NaN is equal to NaN");

      n = [NSNumber numberWithInt: 2];
      PASS(NO == [n isEqualToNumber: nan], "2 is not equal to NaN");
      PASS([n compare: nan] == NSOrderedDescending, "2 is greater than NaN")
      PASS([nan compare: n] == NSOrderedAscending, "NaN is less than 2")

      n = [NSNumber numberWithUnsignedLongLong: 2];
      PASS(NO == [n isEqualToNumber: nan], "2LL is not equal to NaN");
      PASS([n compare: nan] == NSOrderedDescending, "2LL is greater than NaN")
      PASS([nan compare: n] == NSOrderedAscending, "NaN is less than 2LL")

      n = [NSNumber numberWithFloat: 2.0];
      PASS(NO == [n isEqualToNumber: nan], "2.0 is not equal to NaN");
      PASS([n compare: nan] == NSOrderedDescending, "2.0 is greater than NaN")
      PASS([nan compare: n] == NSOrderedAscending, "NaN is less than 2.0")

      n = [NSNumber numberWithDouble: 2.0];
      PASS(NO == [n isEqualToNumber: nan], "2.0dd is not equal to NaN");
      PASS([n compare: nan] == NSOrderedDescending, "2.0dd is greater than NaN")
      PASS([nan compare: n] == NSOrderedAscending, "NaN is less than 2.0dd")

      n = [NSNumber numberWithFloat: 0.0];
      PASS(NO == [n isEqualToNumber: nan], "0.0 is not equal to NaN");
      PASS([n compare: nan] == NSOrderedDescending, "0.0 greater than NaN")
      PASS([nan compare: n] == NSOrderedAscending, "NaN less than 0.0")

      n = [NSNumber numberWithFloat: -1.01];
      PASS(NO == [n isEqualToNumber: nan], "-1.01 is not equal to NaN");
      PASS([n compare: nan] == NSOrderedAscending, "-1.01 less than NaN")
      PASS([nan compare: n] == NSOrderedAscending, "NaN less than -1.01")

      END_SET("not-a-number checks")

    START_SET("zero checks")

      NSNumber	*zero = [NSDecimalNumber zero];

      PASS(YES == [zero isEqualToNumber: zero], "zero is equal to zero");

      n = [NSNumber numberWithInt: 2];
      PASS(NO == [n isEqualToNumber: zero], "2 is not equal to zero");
      PASS([n compare: zero] == NSOrderedDescending, "2 is greater than zero")
      PASS([zero compare: n] == NSOrderedAscending, "zero is less than 2")

      n = [NSNumber numberWithUnsignedLongLong: 2];
      PASS(NO == [n isEqualToNumber: zero], "2LL is not equal to zero");
      PASS([n compare: zero] == NSOrderedDescending, "2LL is greater than zero")
      PASS([zero compare: n] == NSOrderedAscending, "zero is less than 2LL")

      n = [NSNumber numberWithFloat: 2.0];
      PASS(NO == [n isEqualToNumber: zero], "2.0 is not equal to zero");
      PASS([n compare: zero] == NSOrderedDescending, "2.0 is greater than zero")
      PASS([zero compare: n] == NSOrderedAscending, "zero is less than 2.0")

      n = [NSNumber numberWithDouble: 2.0];
      PASS(NO == [n isEqualToNumber: zero], "2.0dd is not equal to zero");
      PASS([n compare: zero] == NSOrderedDescending,
	"2.0dd is greater than zero")
      PASS([zero compare: n] == NSOrderedAscending, "zero is less than 2.0dd")

      n = [NSNumber numberWithFloat: 0.0];
      PASS([n isEqualToNumber: zero], "0.0 is equal to zero");
      PASS([n compare: zero] == NSOrderedSame, "0.0 is zero")
      PASS([zero compare: n] == NSOrderedSame, "zero is 0.0")

      n = [NSNumber numberWithFloat: -1.01];
      PASS(NO == [n isEqualToNumber: zero], "-1.01 is not equal to zero");
      PASS([n compare: zero] == NSOrderedAscending, "-1.01 less than zero")
      PASS([zero compare: n] == NSOrderedDescending, "zero greater than -1.01")

    END_SET("zero checks")
  END_SET("NSNumber")

  return 0;
}
