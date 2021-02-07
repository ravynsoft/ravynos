#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"

#if	defined(GS_USE_ICU)
#define	NSLOCALE_SUPPORTED	GS_USE_ICU
#else
#define	NSLOCALE_SUPPORTED	1 /* Assume Apple support */
#endif

int main()
{
  NSNumberFormatter *fmt;
  NSNumber *num;
  NSString *str;

  START_SET("NSNumberFormatter")

    PASS(NSNumberFormatterBehavior10_4
      == [NSNumberFormatter defaultFormatterBehavior],
     "default behavior is NSNumberFormatterBehavior10_4")

    [NSNumberFormatter
      setDefaultFormatterBehavior: NSNumberFormatterBehavior10_0];
    PASS(NSNumberFormatterBehavior10_0
      == [NSNumberFormatter defaultFormatterBehavior],
     "default behavior can be changed to NSNumberFormatterBehavior10_0")

    [NSNumberFormatter
      setDefaultFormatterBehavior: NSNumberFormatterBehaviorDefault];
    PASS(NSNumberFormatterBehavior10_4
      == [NSNumberFormatter defaultFormatterBehavior],
     "NSNumberFormatterBehaviorDefault gives NSNumberFormatterBehavior10_4")

    [NSNumberFormatter
      setDefaultFormatterBehavior: NSNumberFormatterBehavior10_0];
    [NSNumberFormatter setDefaultFormatterBehavior: 1234];
    PASS(1234 == [NSNumberFormatter defaultFormatterBehavior],
     "unknown behavior is accepted")

    [NSNumberFormatter
      setDefaultFormatterBehavior: NSNumberFormatterBehavior10_4];
    PASS(NSNumberFormatterBehavior10_4
      == [NSNumberFormatter defaultFormatterBehavior],
     "default behavior can be changed to NSNumberFormatterBehavior10_4")

    fmt = [[[NSNumberFormatter alloc] init] autorelease];

    PASS(NSNumberFormatterBehavior10_4 == [fmt formatterBehavior],
     "a new formatter gets the current default behavior")
    [fmt setFormatterBehavior: NSNumberFormatterBehaviorDefault];
    PASS(NSNumberFormatterBehaviorDefault == [fmt formatterBehavior],
     "a new formatter can have the default behavior set")

    str = [fmt stringFromNumber: [NSDecimalNumber notANumber]];
    PASS_EQUAL(str, @"NaN", "notANumber special case")

    START_SET("NSLocale")
      NSLocale  *sys;
      NSLocale  *en;
      const unichar uspc[1] = {0x00a0};
      NSString *spc = [NSString stringWithCharacters: uspc length: 1];
      if (!NSLOCALE_SUPPORTED)
        SKIP("NSLocale not supported\nThe ICU library was not available when GNUstep-base was built")
      
      sys = [NSLocale systemLocale];
      [fmt setLocale: sys];

      PASS([fmt getObjectValue: &num forString: @"0.00" errorDescription: 0]
        && num != nil, "formatting suceeded")
      if (testPassed)
        {
          PASS(NO == [num isEqual: [NSDecimalNumber notANumber]],
            "is not equal to NaN")
          PASS(YES == [num isEqual: [NSDecimalNumber zero]],
            "is equal to zero")
        }
      
      num = [[[NSNumber alloc] initWithFloat: 1234.567] autorelease];

      str = [fmt stringFromNumber: num];
      PASS_EQUAL(str, @"1235", "default 10.4 format same as Cocoa")

      en = [[[NSLocale alloc] initWithLocaleIdentifier: @"en"] autorelease];
      PASS_EQUAL([en localeIdentifier], @"en", "have locale 'en'");

      [fmt setLocale: en];

      [fmt setPaddingCharacter: @"+"];
      PASS_EQUAL([fmt paddingCharacter], @"+", "padding character se to '+'")

      [fmt setPaddingCharacter: @"*"]; // Subsequent tests use '*'

      [fmt setMaximumFractionDigits: 2];
      str = [fmt stringFromNumber: num];

      PASS_EQUAL(str, @"1234.57", "round up for fractional part >0.5")

      num = [[[NSNumber alloc] initWithFloat: 1234.432] autorelease];
      str = [fmt stringFromNumber: num];

      PASS_EQUAL(str, @"1234.43", "round down for fractional part <0.5")
      
      num = [[[NSNumber alloc] initWithFloat: -1234.43] autorelease];
      [fmt setMaximumFractionDigits: 1];
      [fmt setMinusSign: @"_"];
      PASS_EQUAL([fmt stringFromNumber: num], @"_1234.4",
        "minus sign assigned correctly");
      
      [fmt setPercentSymbol: @"##"];
      
      [fmt setNumberStyle: NSNumberFormatterPercentStyle];
      testHopeful = YES;
      PASS_EQUAL([fmt stringFromNumber: num], @"_123,443##",
        "Negative percentage correct");
      testHopeful = NO;
      
      
      num = [[[NSNumber alloc] initWithFloat: 1234.432] autorelease];
      
      [fmt setNumberStyle: NSNumberFormatterNoStyle];
      [fmt setMaximumFractionDigits: 0];
      [fmt setFormatWidth: 6];
      
      str = [fmt stringFromNumber: num];
      PASS([str isEqual: @"**1234"] || [str isEqual: @"  1234"],
        "format width set correctly");
      
      [fmt setPositivePrefix: @"+"];
      str = [fmt stringFromNumber: num];
      PASS([str isEqual: @"*+1234"] || [str isEqual: @" +1234"],
        "positive prefix set correctly");
      
      [fmt setPaddingCharacter: @"0"];
      str = [fmt stringFromNumber: num];
      PASS_EQUAL(str, @"0+1234", "default padding position is before prefix");
      
      [fmt setPaddingPosition: NSNumberFormatterPadAfterPrefix];
      str = [fmt stringFromNumber: num];
    
      PASS_EQUAL(str, @"+01234", "numeric and space padding OK")

      num = [[[NSNumber alloc] initWithFloat: 1234.56] autorelease];
      [fmt setNumberStyle: NSNumberFormatterCurrencyStyle];
      [fmt setLocale: [[NSLocale alloc] initWithLocaleIdentifier: @"pt_BR"]];
      
      testHopeful = YES;
      PASS_EQUAL([fmt stringFromNumber: num], @"+1.235",
        "currency style does not include currency string");
      testHopeful = NO;
      
      [fmt setPositivePrefix: @"+"];
      PASS_EQUAL([fmt stringFromNumber: num], @"+1.235",
        "positive prefix is set correctly for currency style");
      
      [fmt setPositiveSuffix: @"c"];
      PASS_EQUAL([fmt stringFromNumber: num], @"+1.235c",
        "prefix and suffix used properly");

      num = [[[NSNumber alloc] initWithFloat: -1234.56] autorelease];
      str = [fmt stringFromNumber: num];
      PASS(([str isEqual: @"(R$1.235)"] || [str isEqual: @"_R$1.235"]
        || [str isEqual: [NSString stringWithFormat: @"_R$%@%@",
        spc, @"1.235"]]),
        "negativeFormat used for -ve number");

      [fmt setNumberStyle: NSNumberFormatterNoStyle];
      [fmt setMinusSign: @"_"];
      
      testHopeful = YES;
      PASS_EQUAL([fmt stringFromNumber: num], @"_01235",
	"format string of length 1")
      testHopeful = NO;

    END_SET("NSLocale")

  END_SET("NSNumberFormatter")

  return 0;
}

