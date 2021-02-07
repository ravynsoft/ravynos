#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSLocale.h>
#import <Foundation/NSValue.h>
#import "ObjectTesting.h"

#if	defined(GS_USE_ICU)
#define	NSLOCALE_SUPPORTED	GS_USE_ICU
#else
#define	NSLOCALE_SUPPORTED	1 /* Assume Apple support */
#endif

int main(void)
{
  START_SET("NSLocale")

  NSLocale *locale;
  id            o;
  unichar       u;
  
  if (!NSLOCALE_SUPPORTED)
    SKIP("NSLocale not supported\nThe ICU library was not available when GNUstep-base was built")

  // These tests don't really work all that well.  I need to come up with
  // something better.  Most of the ones that fail are because nil is returned.
  testHopeful = YES;
  locale = [[NSLocale alloc] initWithLocaleIdentifier: @"es_ES_PREEURO"];
  PASS_EQUAL([locale objectForKey: NSLocaleIdentifier],
    @"es_ES@currency=ESP",
    "NSLocaleIdentifier key returns 'es_ES@currency=ESP'");
  PASS_EQUAL([locale objectForKey: NSLocaleLanguageCode],
    @"es",
    "NSLocaleLanguageCode key returns 'es'");
  PASS_EQUAL([locale objectForKey: NSLocaleCountryCode],
    @"ES",
    "NSLocaleCountryCode key returns 'ES'");

  PASS_EQUAL([locale objectForKey: NSLocaleScriptCode], nil,
    "NSLocaleScriptCode key returns nil");
  PASS_EQUAL([locale objectForKey: NSLocaleVariantCode], nil,
    "NSLocaleVariantCode key returns nil");
  PASS_EQUAL([locale objectForKey: NSLocaleCollationIdentifier], nil,
    "NSLocaleCollationIdentifier key returns nil");
  TEST_FOR_CLASS(@"NSCharacterSet",
    [locale objectForKey: NSLocaleExemplarCharacterSet],
    "NSLocaleExemplarCharacterSet key returns a NSCharacterSet");
  TEST_FOR_CLASS(@"NSCalendar", [locale objectForKey: NSLocaleCalendar],
    "NSLocaleCalendar key returns a NSCalendar");
  o = [locale objectForKey: NSLocaleUsesMetricSystem];
  TEST_FOR_CLASS(@"NSNumber", o,
    "NSLocaleUsesMetricSystem key returns a NSNumber");
  PASS_EQUAL(o, [NSNumber numberWithBool: YES],
    "NSLocaleUsesMetricSystem key returns YES");
  PASS_EQUAL([locale objectForKey: NSLocaleMeasurementSystem],
    @"Metric",
    "NSLocaleMeasurementSystem key returns 'Metric'");
  PASS_EQUAL([locale objectForKey: NSLocaleDecimalSeparator],
    @",",
    "NSLocaleDecimalSeparator key returns ','");
  u = 8359;
  PASS_EQUAL([locale objectForKey: NSLocaleCurrencySymbol],
    [NSString stringWithCharacters: &u length: 1],
    "NSLocaleCurrencySymbol key returns 'xx3'");
  PASS_EQUAL([locale objectForKey: NSLocaleCurrencyCode],
    @"ESP",
    "NSLocaleCurrencyCode key returns 'ESP'");
  PASS_EQUAL([locale objectForKey: NSLocaleCollatorIdentifier],
    @"es_ES@currency=ESP", "NSLocaleCollatorIdentifier for Spain");
  PASS_EQUAL([locale objectForKey: NSLocaleGroupingSeparator],
    @".",
    "NSLocaleGroupingSeparator key returns '.'");
  u = 8216;
  PASS_EQUAL([locale objectForKey: NSLocaleQuotationBeginDelimiterKey],
    [NSString stringWithCharacters: &u length: 1],
    "NSLocaleQuotationBeginDelimiterKey key works");
  u = 8217;
  PASS_EQUAL([locale objectForKey: NSLocaleQuotationEndDelimiterKey],
    [NSString stringWithCharacters: &u length: 1],
    "NSLocaleQuotationEndDelimiterKey key returns 'xx6'");
  u = 8220;
  PASS_EQUAL([locale objectForKey: NSLocaleAlternateQuotationBeginDelimiterKey],
    [NSString stringWithCharacters: &u length: 1],
    "NSLocaleAlternateQuotationBeginDelimiterKey key returns 'xx7'");
  u = 8221;
  PASS_EQUAL([locale objectForKey: NSLocaleAlternateQuotationEndDelimiterKey],
    [NSString stringWithCharacters: &u length: 1],
    "NSLocaleAlternateQuotationEndDelimiterKey key returns 'xx8'");
  testHopeful = NO;
  RELEASE(locale);
  
  locale = [[NSLocale alloc] initWithLocaleIdentifier: @"en_US"];
  PASS_EQUAL([locale localeIdentifier], @"en_US",
    "'en_US' is stored as 'en_US'.");
  PASS_EQUAL([locale objectForKey: NSLocaleScriptCode], nil,
    "en_US does not have script code");
  PASS_EQUAL([locale objectForKey: NSLocaleVariantCode], nil,
    "en_US does not have variant code");
  PASS_EQUAL([locale objectForKey: NSLocaleCollationIdentifier], nil,
    "en_US does not have a collation identifier");
  PASS ([[locale objectForKey: NSLocaleUsesMetricSystem] boolValue] == NO,
    "en_US does not use the metric system");
  RELEASE(locale);
  
  locale = [[NSLocale alloc] initWithLocaleIdentifier: @"zh-Hant_TW"];
  PASS_EQUAL([locale objectForKey: NSLocaleCountryCode], @"TW",
    "zh-Hant_TW country code is zh");
  PASS_EQUAL([locale objectForKey: NSLocaleLanguageCode], @"zh",
    "zh-Hant_TW language code is zh");
  PASS_EQUAL([locale localeIdentifier], @"zh_TW",
    "'zh-Hant_TW' is stored as 'zh_TW'");
  PASS_EQUAL([locale objectForKey: NSLocaleScriptCode], nil,
    "zh-Hant_TW has no script code");
  RELEASE(locale);
  
  PASS_EQUAL([NSLocale canonicalLocaleIdentifierFromString: nil], nil,
    "Canonical identifier for nil is nil");
  PASS_EQUAL([NSLocale canonicalLocaleIdentifierFromString: @""], @"",
    "Canonical identifier for an empty string is an empty string");
  PASS_EQUAL([NSLocale canonicalLocaleIdentifierFromString: @"some rubbish"],
    @"some rubbish",
    "Canonical identifier for 'some rubbish is 'some rubbish'");

  /* Let's just hope the next two PASS.
   */
  testHopeful = YES;
  PASS_EQUAL([NSLocale canonicalLocaleIdentifierFromString: @"AmericanEnglish"],
    @"americanenglish",
    "Canonical identifier for 'AmericanEnglish is americanenglish");
  PASS_EQUAL([NSLocale canonicalLanguageIdentifierFromString: @"AmericanEnglish"],
    @"americanenglish",
    "Canonical language identifier for 'AmericanEnglish is americanenglish");
  
  END_SET("NSLocale")

  return 0;
}
