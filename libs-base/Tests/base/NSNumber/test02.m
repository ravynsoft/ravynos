#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSPropertyList.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSDecimalNumber.h>

#include <stdlib.h>
#include <limits.h>

int main()
{
  START_SET("NSDecimalNumber")
    NSDecimalNumberHandler      *handler;
    NSDecimalNumber             *n1;
    NSDecimalNumber             *n2;
    NSString                    *s1;
    NSString                    *s2;
    NSLocale                    *locale;

    locale = [NSLocale systemLocale];
    handler = [NSDecimalNumberHandler alloc];
    handler = [[handler initWithRoundingMode: NSRoundPlain
                                       scale: 2
                            raiseOnExactness: NO
                             raiseOnOverflow: NO
                            raiseOnUnderflow: NO
                         raiseOnDivideByZero: NO] autorelease];

    s1 = [NSString stringWithFormat: @"%0.2f", 0.009];
    n1 = [NSDecimalNumber decimalNumberWithString: @"0.009" locale: locale];
    n2 = [n1 decimalNumberByRoundingAccordingToBehavior: handler];
    s2 = [n2 descriptionWithLocale: locale];
    PASS_EQUAL(s2, s1, "rounding 0.009");

    s1 = [NSString stringWithFormat: @"%0.2f", 0.019];
    n1 = [NSDecimalNumber decimalNumberWithString: @"0.019" locale: locale];
    n2 = [n1 decimalNumberByRoundingAccordingToBehavior: handler];
    s2 = [n2 descriptionWithLocale: locale];
    PASS_EQUAL(s2, s1, "rounding 0.019");

    handler = [NSDecimalNumberHandler alloc];
    handler = [[handler initWithRoundingMode: NSRoundPlain
                                       scale: 3
                            raiseOnExactness: NO
                             raiseOnOverflow: NO
                            raiseOnUnderflow: NO
                         raiseOnDivideByZero: NO] autorelease];

    s1 = [NSString stringWithFormat: @"%0.3f", 0.0009];
    n1 = [NSDecimalNumber decimalNumberWithString: @"0.0009" locale: locale];
    n2 = [n1 decimalNumberByRoundingAccordingToBehavior: handler];
    s2 = [n2 descriptionWithLocale: locale];
    PASS_EQUAL(s2, s1, "rounding 0.0009");

    s1 = [NSString stringWithFormat: @"%0.3f", 0.0019];
    n1 = [NSDecimalNumber decimalNumberWithString: @"0.0019" locale: locale];
    /* Try working with NSDecimal directly
     */
    {
      NSDecimal result;
      NSDecimal d1 = [n1 decimalValue];
      NSString  *s2;

      NSLog(@"NSDecimal before rounding %g", NSDecimalDouble(&d1));
      NSDecimalRound(&result, &d1, [handler scale], [handler roundingMode]);
      NSLog(@"NSDecimal after rounding %g", NSDecimalDouble(&result));
      s2 = NSDecimalString(&result, nil);
      PASS_EQUAL(s2, s1, "NSDecimal rounding 0.0019 to 0.02");
      n2 = [NSDecimalNumber decimalNumberWithDecimal: result];
    }
    s2 = [n2 descriptionWithLocale: locale];
    PASS_EQUAL(s2, s1, "NSDecimalNumber rounding 0.0019");

  END_SET("NSDecimalNumber")

  return 0;
}
