/* NSLocale.m

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by: Stefan Bidigaray
   Date: June, 2010

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#define	EXPOSE_NSLocale_IVARS	1
#import "common.h"
#import "Foundation/NSLocale.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSCalendar.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSNumberFormatter.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSString.h"
#import "GNUstepBase/GSLock.h"

NSString * const NSCurrentLocaleDidChangeNotification =
  @"NSCurrentLocaleDidChangeNotification";

//
// NSLocale Component Keys
//
NSString * const NSLocaleIdentifier = @"NSLocaleIdentifier";
NSString * const NSLocaleLanguageCode = @"NSLocaleLanguageCode";
NSString * const NSLocaleCountryCode = @"NSLocaleCountryCode";
NSString * const NSLocaleScriptCode = @"NSLocaleScriptCode";
NSString * const NSLocaleVariantCode = @"NSLocaleVariantCode";
NSString * const NSLocaleExemplarCharacterSet = @"NSLocaleExemplarCharacterSet";
NSString * const NSLocaleCalendarIdentifier = @"calendar";
NSString * const NSLocaleCalendar = @"NSLocaleCalendar";
NSString * const NSLocaleCollationIdentifier = @"collation";
NSString * const NSLocaleUsesMetricSystem = @"NSLocaleUsesMetricSystem";
NSString * const NSLocaleMeasurementSystem = @"NSLocaleMeasurementSystem";
NSString * const NSLocaleDecimalSeparator = @"NSLocaleDecimalSeparator";
NSString * const NSLocaleGroupingSeparator = @"NSLocaleGroupingSeparator";
NSString * const NSLocaleCurrencySymbol = @"NSLocaleCurrencySymbol";
NSString * const NSLocaleCurrencyCode = @"NSLocaleCurrencyCode";
NSString * const NSLocaleCollatorIdentifier = @"NSLocaleCollatorIdentifier";
NSString * const NSLocaleQuotationBeginDelimiterKey =
  @"NSLocaleQuotationBeginDelimiterKey";
NSString * const NSLocaleQuotationEndDelimiterKey =
  @"NSLocaleQuotationEndDelimiterKey";
NSString * const NSLocaleAlternateQuotationBeginDelimiterKey =
  @"NSLocaleAlternateQuotationBeginDelimiterKey";
NSString * const NSLocaleAlternateQuotationEndDelimiterKey =
  @"NSLocaleAlternateQuotationEndDelimiterKey";

//
// NSLocale Calendar Keys
//
NSString * const NSGregorianCalendar = @"gregorian";
NSString * const NSBuddhistCalendar = @"buddhist";
NSString * const NSChineseCalendar = @"chinese";
NSString * const NSHebrewCalendar = @"hebrew";
NSString * const NSIslamicCalendar = @"islamic";
NSString * const NSIslamicCivilCalendar = @"islamic-civil";
NSString * const NSJapaneseCalendar = @"japanese";
NSString * const NSRepublicOfChinaCalendar = @"roc";
NSString * const NSPersianCalendar = @"persian";
NSString * const NSIndianCalendar = @"indian";
NSString * const NSISO8601Calendar = @"";

//
// NSLocale New Calendar ID Keys
//
NSString * const NSCalendarIdentifierGregorian = @"gregorian";
NSString * const NSCalendarIdentifierBuddhist = @"buddhist";
NSString * const NSCalendarIdentifierChinese = @"chinese";
NSString * const NSCalendarIdentifierCoptic = @"coptic";
NSString * const NSCalendarIdentifierEthiopicAmeteMihret = @"ethiopic-amete-mihret";
NSString * const NSCalendarIdentifierEthiopicAmeteAlem = @"ethiopic-amete-alem";
NSString * const NSCalendarIdentifierHebrew = @"hebrew";
NSString * const NSCalendarIdentifierISO8601 = @"";
NSString * const NSCalendarIdentifierIndian = @"indian";
NSString * const NSCalendarIdentifierIslamic = @"islamic";
NSString * const NSCalendarIdentifierIslamicCivil = @"islamic-civil";
NSString * const NSCalendarIdentifierJapanese = @"japanese";
NSString * const NSCalendarIdentifierPersian = @"persian";
NSString * const NSCalendarIdentifierRepublicOfChina = @"roc";
NSString * const NSCalendarIdentifierIslamicTabular = @"islamic-tabular";
NSString * const NSCalendarIdentifierIslamicUmmAlQura = @"islamic-umm-al-qura";

#if	defined(HAVE_UNICODE_ULOC_H)
# include <unicode/uloc.h>
#endif
#if	defined(HAVE_UNICODE_ULOCDATA_H)
# include <unicode/ulocdata.h>
#endif
#if	defined(HAVE_UNICODE_UCURR_H)
# include <unicode/ucurr.h>
#endif



@interface NSLocale (PrivateMethods)
+ (void) _updateCanonicalLocales;
- (NSString *) _getMeasurementSystem;
- (NSCharacterSet *) _getExemplarCharacterSet;
- (NSString *) _getDelimiterWithType: (NSInteger) delimiterType;
- (NSCalendar *) _getCalendar;
- (NSString *) _getDecimalSeparator;
- (NSString *) _getGroupingSeparator;
- (NSString *) _getCurrencySymbol;
- (NSString *) _getCurrencyCode;
@end

#if	GS_USE_ICU == 1
//
// ICU Component Keywords
//
static const char * ICUCalendarKeyword = "calendar";
static const char * ICUCollationKeyword = "collation";

static NSLocaleLanguageDirection
ICUToNSLocaleOrientation (ULayoutType layout)
{
  switch (layout)
    {
      case ULOC_LAYOUT_LTR:
        return NSLocaleLanguageDirectionLeftToRight;
      case ULOC_LAYOUT_RTL:
        return NSLocaleLanguageDirectionRightToLeft;
      case ULOC_LAYOUT_TTB:
        return NSLocaleLanguageDirectionTopToBottom;
      case ULOC_LAYOUT_BTT:
        return NSLocaleLanguageDirectionBottomToTop;
      default:
        return NSLocaleLanguageDirectionUnknown;
    }
}

static NSArray *_currencyCodesWithType (uint32_t currType)
{
  NSArray *result;
  NSMutableArray *currencies;
  UErrorCode err = U_ZERO_ERROR;
  const char *currCode;
  UEnumeration *codes;

  codes = ucurr_openISOCurrencies (currType, &err);
  if (U_FAILURE(err))
    return nil;

  currencies = [[NSMutableArray alloc] initWithCapacity: 10];

  do
    {
      int strLength;
      
      err = U_ZERO_ERROR;
      currCode = uenum_next (codes, &strLength, &err);
      if (U_FAILURE(err))
        {
          uenum_close (codes);
	  [currencies release];
          return nil;
        }
      if (currCode == NULL)
        break;
      [currencies addObject: [NSString stringWithUTF8String: currCode]];
    } while (NULL != currCode);

  uenum_close (codes);
  result = [NSArray arrayWithArray: currencies];
  [currencies release];
  return result;
}
#endif

@implementation NSLocale

static NSLocale *autoupdatingLocale = nil;
static NSLocale *currentLocale = nil;
static NSLocale *systemLocale = nil;
static NSMutableDictionary *allLocales = nil;
static NSDictionary *canonicalLocales = nil;
static NSRecursiveLock *classLock = nil;

+ (void) initialize
{
  if (self == [NSLocale class])
    {
      classLock = [NSRecursiveLock new];
      [[NSObject leakAt: &classLock] release];
      allLocales = [[NSMutableDictionary alloc] initWithCapacity: 0];
      [[NSObject leakAt: &allLocales] release];
    }
}

+ (void) defaultsDidChange: (NSNotification*)n
{
  NSUserDefaults	*defs;
  NSString		*name;

  defs = [NSUserDefaults standardUserDefaults];
  name = [defs stringForKey: @"Locale"];
  if ([name isEqual: autoupdatingLocale->_localeId] == NO)
    {
      [classLock lock];
      RELEASE(autoupdatingLocale->_localeId);
      RELEASE(autoupdatingLocale->_components);
      
      autoupdatingLocale->_localeId = RETAIN(name);
      autoupdatingLocale->_components = nil;
      
      RELEASE(currentLocale);
      currentLocale = nil;
      [classLock unlock];
      
      [[NSNotificationCenter defaultCenter]
        postNotificationName: NSCurrentLocaleDidChangeNotification
        object: nil];
    }
}

+ (id) autoupdatingCurrentLocale
{
  NSLocale *result;

  [classLock lock];
  if (nil == autoupdatingLocale)
    {
      autoupdatingLocale = [[self currentLocale] copy];
      [[NSNotificationCenter defaultCenter]
        addObserver: self
        selector: @selector(defaultsDidChange:)
        name: NSUserDefaultsDidChangeNotification
        object: nil];
    }

  result = RETAIN(autoupdatingLocale);
  [classLock unlock];
  return AUTORELEASE(result);
}

+ (NSArray *) availableLocaleIdentifiers
{
  static NSArray	*available = nil;

#if	GS_USE_ICU == 1
  if (nil == available)
    {
      [classLock lock];
      if (nil == available)
        {
          NSMutableArray	*array;
          int32_t 		i;
          int32_t 		count = uloc_countAvailable ();

          array = [[NSMutableArray alloc] initWithCapacity: count];

          for (i = 0; i < count; ++i)
            {
              const char *localeID = uloc_getAvailable (i);

              [array addObject: [NSString stringWithUTF8String: localeID]];
            }
          available = [[NSArray alloc] initWithArray: array];
          [array release];
        }
      [classLock unlock];
    }
#endif
  return [[available copy] autorelease];
}

+ (NSString *) canonicalLanguageIdentifierFromString: (NSString *) string
{
  NSString *result;
  NSString *localeId;
  NSArray *localeComps;
  
  /* Can't use the ICU functions here because, according to Apple locale docs,
     the language has a format like "zh-Hant".  ICU, however, uses an
     underscore to separate Scripts "zh_Hant". */
  if (canonicalLocales == nil)
    [self _updateCanonicalLocales];
  
  localeId = [canonicalLocales objectForKey: string];
  if (nil == localeId)
    {
      result = string;
    }
  else
    {
      localeComps = [localeId componentsSeparatedByString: @"_"];
      result = [localeComps objectAtIndex: 0];
    }
  return result;
}

+ (NSString *) canonicalLocaleIdentifierFromString: (NSString *) string
{
  /* The way this works, according to Apple docs, is a mess.  It seems
     that both BCP 47's "-" and ICU's "_" separators are used.  According to
     "Language and Locale Designations" (Apple docs) Taiwan, for example, has
     zh-Hant_TW as it's locale identifier (was zh_TW on 10.3.9 and below).
     Since ICU doesn't use "-" as a separator it will modify that identifier
     to zh_Hant_TW. */
  NSString *result;
  NSMutableString *mStr;
  NSRange range;
  
  if (string == nil)
    return nil;
  
  if (canonicalLocales == nil)
    [self _updateCanonicalLocales];
  
  result = [canonicalLocales objectForKey: string];
  if (result == nil)
    result = string;
  
  // Strip script info from locale
  range = [result rangeOfString: @"-"];
  if (range.location != NSNotFound)
    {
      NSUInteger start = range.location;
      NSUInteger length;
      range = [result rangeOfString: @"_"];
      length = range.location - start;
      
      mStr = [NSMutableString stringWithString: result];
      [mStr deleteCharactersInRange: NSMakeRange (start, length)];
      
      result = [NSString stringWithString: mStr];
    }
  
  return result;
}

+ (NSLocaleLanguageDirection) characterDirectionForLanguage:
    (NSString *)isoLangCode
{
#if	GS_USE_ICU == 1
  ULayoutType result;
  UErrorCode status = U_ZERO_ERROR;

  result = uloc_getCharacterOrientation ([isoLangCode UTF8String], &status);
  if (U_FAILURE(status) || ULOC_LAYOUT_UNKNOWN == result)
    return NSLocaleLanguageDirectionUnknown;

  return ICUToNSLocaleOrientation (result);
#else
  return NSLocaleLanguageDirectionLeftToRight;	// FIXME
#endif
}

+ (NSDictionary *) componentsFromLocaleIdentifier: (NSString *) string
{
#if	GS_USE_ICU == 1
  char buffer[ULOC_KEYWORD_AND_VALUES_CAPACITY];
  const char *cLocaleId = [string UTF8String];
  int32_t strLength;
  UEnumeration *enumerator;
  UErrorCode error = U_ZERO_ERROR;
  NSDictionary *result;
  NSMutableDictionary *tmpDict =
    [[NSMutableDictionary alloc] initWithCapacity: 5];

  strLength = uloc_getLanguage (cLocaleId, buffer,
    ULOC_KEYWORD_AND_VALUES_CAPACITY, &error);
  if (U_SUCCESS(error) && strLength)
    {
      [tmpDict setValue: [NSString stringWithUTF8String: buffer]
                 forKey: NSLocaleLanguageCode];
    }
  error = U_ZERO_ERROR;

  strLength = uloc_getCountry (cLocaleId, buffer,
    ULOC_KEYWORD_AND_VALUES_CAPACITY, &error);
  if (U_SUCCESS(error) && strLength)
    {
      [tmpDict setValue: [NSString stringWithUTF8String: buffer]
                  forKey: NSLocaleCountryCode];
    }
  error = U_ZERO_ERROR;

  strLength = uloc_getScript (cLocaleId, buffer,
    ULOC_KEYWORD_AND_VALUES_CAPACITY, &error);
  if (U_SUCCESS(error) && strLength)
    {
      [tmpDict setValue: [NSString stringWithUTF8String: buffer]
                  forKey: NSLocaleScriptCode];
    }
  error = U_ZERO_ERROR;

  strLength = uloc_getVariant (cLocaleId, buffer,
    ULOC_KEYWORD_AND_VALUES_CAPACITY, &error);
  if (U_SUCCESS(error) && strLength)
    {
      [tmpDict setValue: [NSString stringWithUTF8String: buffer]
                  forKey: NSLocaleVariantCode];
    }
  error = U_ZERO_ERROR;
  
  enumerator = uloc_openKeywords (cLocaleId, &error);
  if (U_SUCCESS(error))
    {
      const char *keyword;
      error = U_ZERO_ERROR;
      
      keyword = uenum_next(enumerator, NULL, &error);
      while (keyword && U_SUCCESS(error))
        {
          error = U_ZERO_ERROR;
          strLength = uloc_getKeywordValue (cLocaleId, keyword, buffer,
            ULOC_KEYWORD_AND_VALUES_CAPACITY, &error);
          if (strLength && U_SUCCESS(error))
            {
              // This is OK because NSLocaleCalendarIdentifier = "calendar"
              // and NSLocaleCollationIdentifier = "collation".
              [tmpDict setValue: [NSString stringWithUTF8String: buffer]
                         forKey: [NSString stringWithUTF8String: keyword]];
          
              error = U_ZERO_ERROR;
              keyword = uenum_next (enumerator, NULL, &error);
            }
        }
    }
  uenum_close (enumerator);
  
  result = [NSDictionary dictionaryWithDictionary: tmpDict];
  RELEASE(tmpDict);
  return result;
#else
  return nil;	// FIXME
#endif
}

+ (id) currentLocale
{
  NSLocale *result;

  [classLock lock];
  if (nil == currentLocale)
    {
      NSString *localeId;
      [classLock unlock];
      
      localeId =
        [[NSUserDefaults standardUserDefaults] objectForKey: @"Locale"];
      
      [classLock lock];
      if (currentLocale == nil)
        currentLocale = [[NSLocale alloc] initWithLocaleIdentifier: localeId];
    }
  result = RETAIN(currentLocale);
  [classLock unlock];
  return AUTORELEASE(result);
}

+ (NSArray *) commonISOCurrencyCodes
{
#if	GS_USE_ICU == 1
  return _currencyCodesWithType (UCURR_COMMON | UCURR_NON_DEPRECATED);
#else
  return nil;	// FIXME
#endif
}

+ (NSArray *) ISOCurrencyCodes
{
#if	GS_USE_ICU == 1
  return _currencyCodesWithType (UCURR_ALL);
#else
  return nil;	// FIXME
#endif
}

+ (NSArray *) ISOCountryCodes
{
  static NSArray	*countries = nil;

  if (nil == countries)
    {
#if	GS_USE_ICU == 1
      [classLock lock];
      if (nil == countries)
        {
          NSMutableArray *array = [[NSMutableArray alloc] initWithCapacity: 10];
          const char *const *codes = uloc_getISOCountries ();

          while (*codes != NULL)
            {
              [array addObject: [NSString stringWithUTF8String: *codes]];
              ++codes;
            }
          countries = [[NSArray alloc] initWithArray: array];
          [array release];
        }
      [classLock unlock];
#endif
    }
  return [[countries copy] autorelease];
}

+ (NSArray *) ISOLanguageCodes
{
  static NSArray	*languages = nil;

  if (nil == languages)
    {
#if	GS_USE_ICU == 1
      [classLock lock];
      if (nil == languages)
        {
          NSMutableArray *array = [[NSMutableArray alloc] initWithCapacity: 10];
          const char *const *codes = uloc_getISOLanguages ();

          while (*codes != NULL)
            {
              [array addObject: [NSString stringWithUTF8String: *codes]];
              ++codes;
            }
          languages = [[NSArray alloc] initWithArray: array];
          [array release];
        }
      [classLock unlock];
#endif
    }
  return [[languages copy] autorelease];
}

+ (NSLocaleLanguageDirection) lineDirectionForLanguage: (NSString *) isoLangCode
{
#if	GS_USE_ICU == 1
  ULayoutType result;
  UErrorCode status = U_ZERO_ERROR;

  result = uloc_getLineOrientation ([isoLangCode UTF8String], &status);
  if (U_FAILURE(status) || ULOC_LAYOUT_UNKNOWN == result)
    return NSLocaleLanguageDirectionUnknown;

  return ICUToNSLocaleOrientation (result);
#else
  return NSLocaleLanguageDirectionTopToBottom;	// FIXME
#endif
}

+ (NSArray *) preferredLanguages
{
  NSArray *result;
  NSMutableArray *mArray;
  NSUInteger cnt;
  NSUInteger idx = 0;
  NSArray *languages;
  
  languages = [[NSUserDefaults standardUserDefaults]
    stringArrayForKey: @"NSLanguages"];
  if (languages == nil)
    return [NSArray arrayWithObject: @"en"];
  
  mArray = [NSMutableArray array];
  cnt = [languages count];
  while (idx < cnt)
    {
      NSString *lang = [self canonicalLanguageIdentifierFromString:
        [languages objectAtIndex: idx]];
      if (![mArray containsObject: lang])
        [mArray addObject: lang];
      
      ++idx;
    }
  
  result = [NSArray arrayWithArray: mArray];
  return result;
}

+ (id) systemLocale
{
  NSLocale *result;

  [classLock lock];
  if (nil == systemLocale)
    {
#if	GS_USE_ICU == 1
      systemLocale = [[NSLocale alloc] initWithLocaleIdentifier: @""];
#endif
    }

  result = RETAIN(systemLocale);
  [classLock unlock];
  return AUTORELEASE(result);
}

+ (id) localeWithLocaleIdentifier:(NSString *)string
{
  return AUTORELEASE([[NSLocale alloc] initWithLocaleIdentifier: string]);
}

+ (NSString *) localeIdentifierFromComponents: (NSDictionary *) dict
{
  NSString *result;
  NSMutableString *string;
  const char *language = [[dict objectForKey: NSLocaleLanguageCode] UTF8String];
  const char *script = [[dict objectForKey: NSLocaleScriptCode] UTF8String];
  const char *country = [[dict objectForKey: NSLocaleCountryCode] UTF8String];
  const char *variant = [[dict objectForKey: NSLocaleVariantCode] UTF8String];
  const char *calendar =
    [[[dict objectForKey: NSLocaleCalendar] calendarIdentifier] UTF8String];
  const char *collation =
    [[dict objectForKey: NSLocaleCollationIdentifier] UTF8String];
  const char *currency = [[dict objectForKey: NSLocaleCurrencyCode] UTF8String];
  
  if (!calendar)
    {
      calendar = [[dict objectForKey: NSLocaleCalendarIdentifier] UTF8String];
    }

  // A locale cannot be constructed without a language.
  if (language == NULL)
    return nil;
#define TEST_CODE(x) (x ? "_" : ""), (x ? x : "")
  string = [[NSMutableString alloc] initWithFormat: @"%s%s%s%s%s%s%s",
    language, TEST_CODE(script), TEST_CODE(country), TEST_CODE(variant)];
#undef TEST_CODE
  
  // I'm not using uloc_setKeywordValue() here because the format is easy
  // enough to reproduce and has the added advatange that we doesn't need ICU.
  if (calendar || collation || currency)
    [string appendString: @"@"];
  if (calendar)
    [string appendFormat: @"calendar=%s", calendar];
  if (collation)
    {
      if (calendar)
        [string appendString: @";"];
      [string appendFormat: @"collation=%s", collation];
    }
  if (currency)
    {
      if (calendar || currency)
        [string appendString: @";"];
      [string appendFormat: @"currency=%s", currency];
    }
  
  result =  [NSString stringWithString: string];
  RELEASE(string);
  return result;
}

+ (NSString *) localeIdentifierFromWindowsLocaleCode: (uint32_t) lcid
{
#if	GS_USE_ICU == 1
  char buffer[ULOC_FULLNAME_CAPACITY];
  UErrorCode status = U_ZERO_ERROR;

  uloc_getLocaleForLCID (lcid, buffer, ULOC_FULLNAME_CAPACITY, &status);
  if (U_FAILURE(status))
    return nil;

  return [NSString stringWithUTF8String: buffer];
#else
  return nil;	// FIXME Check
              // msdn.microsoft.com/en-us/library/0h88fahh%28v=vs.85%29.aspx
#endif
}

+ (uint32_t) windowsLocaleCodeFromLocaleIdentifier: (NSString *)localeIdentifier
{
#if	GS_USE_ICU == 1
  return uloc_getLCID ([localeIdentifier UTF8String]);
#else
  return 0;	// FIXME: Check
            // msdn.microsoft.com/en-us/library/0h88fahh%28v=vs.85%29.aspx
#endif
}

- (NSString *) displayNameForKey: (NSString *) key value: (id) value
{
#if	GS_USE_ICU == 1
  int32_t length = 0;
  unichar buffer[ULOC_FULLNAME_CAPACITY];
  UErrorCode status = 0;
  const char *keyword = NULL;
  const char *locale = [_localeId UTF8String];

  if ([key isEqualToString: NSLocaleIdentifier])
    {
      length = uloc_getDisplayName([value UTF8String], locale,
        (UChar *)buffer, sizeof(buffer)/sizeof(unichar),
        &status);
    }
  else if ([key isEqualToString: NSLocaleLanguageCode])
    {
      length = uloc_getDisplayLanguage([value UTF8String], locale,
        (UChar *)buffer, sizeof(buffer)/sizeof(unichar),
        &status);
    }
  else if ([key isEqualToString: NSLocaleCountryCode])
    {
      length = uloc_getDisplayCountry([value UTF8String], locale,
        (UChar *)buffer, sizeof(buffer)/sizeof(unichar),
        &status);
    }
  else if ([key isEqualToString: NSLocaleScriptCode])
    {
      length = uloc_getDisplayCountry([value UTF8String], locale,
        (UChar *)buffer, sizeof(buffer)/sizeof(unichar),
        &status);
    }
  else if ([key isEqualToString: NSLocaleVariantCode])
    {
      length = uloc_getDisplayVariant([value UTF8String], locale,
        (UChar *)buffer, sizeof(buffer)/sizeof(unichar),
        &status);
    }
  else if ([key isEqualToString: NSLocaleCalendar])
    {
      keyword = ICUCalendarKeyword;
    }
  else if ([key isEqualToString: NSLocaleCollationIdentifier])
    {
      keyword = ICUCollationKeyword;
    }
  else
    {
	    return nil;
    }

  /*
   * TODO: Implement handling of the other locale component constants.
   */
  if (NULL != keyword)
  {
    length = uloc_getDisplayKeywordValue ([value UTF8String], keyword,
      locale, (UChar *)buffer, sizeof(buffer)/sizeof(unichar),
      &status);
  }
  if (U_FAILURE(status))
    return nil;

  return [NSString stringWithCharacters: buffer length: (NSUInteger)length];
#else
  return nil;	// FIXME
#endif
}

- (id) initWithLocaleIdentifier: (NSString*)string
{
  NSLocale	*newLocale;
  NSString	*localeId;
#if	GS_USE_ICU == 1
  char cLocaleId[ULOC_FULLNAME_CAPACITY];
  UErrorCode error = U_ZERO_ERROR;
  
  localeId = [NSLocale canonicalLocaleIdentifierFromString: string];
  // Normalize locale ID
  uloc_canonicalize ([localeId UTF8String], cLocaleId,
    ULOC_FULLNAME_CAPACITY, &error);
  if (U_FAILURE(error))
    {
      [self release];
      return nil;
    }
  
  localeId = [NSString stringWithUTF8String: cLocaleId];
#else
  localeId = [NSLocale canonicalLocaleIdentifierFromString: string];
#endif
  if (nil == localeId)
    {
      [self release];
      return nil;
    }

  [classLock lock];
  newLocale = [allLocales objectForKey: localeId];
  if (nil == newLocale)
    {
      _localeId = [localeId copy];
      _components = [[NSMutableDictionary alloc] initWithCapacity: 0];
      [allLocales setObject: self forKey: localeId];
    }
  else
    {
      [self release];
      self = [newLocale retain];
    }
  [classLock unlock];

  return self;
}

- (NSString *) localeIdentifier
{
  return _localeId;
}

- (id) objectForKey: (id) key
{
  id result = nil;
#if GS_USE_ICU == 1
  if (key == NSLocaleIdentifier || key == NSLocaleCollatorIdentifier)
    return _localeId;

  if ((result = [_components objectForKey: key]))
    return result;

  if ([_components count] == 0)
    {
      [_components addEntriesFromDictionary:
        [NSLocale componentsFromLocaleIdentifier: _localeId]];
      if ((result = [_components objectForKey: key]))
        return result;
    }
  
  if ([key isEqualToString: NSLocaleUsesMetricSystem])
    {
      NSString *mSys = [_components objectForKey: key];
      mSys = (mSys == nil) ? [self _getMeasurementSystem] : mSys;
      if (mSys != nil)
        {
          [_components setValue: mSys forKey: NSLocaleMeasurementSystem];
          if ([mSys isEqualToString: @"Metric"])
            result = [NSNumber numberWithBool: YES];
          else
            result = [NSNumber numberWithBool: NO];
        }
    }
  else if ([key isEqualToString: NSLocaleMeasurementSystem])
    result = [self _getMeasurementSystem];
  else if ([key isEqualToString: NSLocaleExemplarCharacterSet])
    result = [self _getExemplarCharacterSet];
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
  else if ([key isEqualToString: NSLocaleQuotationBeginDelimiterKey])
    result = [self _getDelimiterWithType: ULOCDATA_QUOTATION_START];
  else if ([key isEqualToString: NSLocaleQuotationEndDelimiterKey])
    result = [self _getDelimiterWithType: ULOCDATA_QUOTATION_END];
  else if ([key isEqualToString: NSLocaleAlternateQuotationBeginDelimiterKey])
    result = [self _getDelimiterWithType: ULOCDATA_ALT_QUOTATION_START];
  else if ([key isEqualToString: NSLocaleAlternateQuotationEndDelimiterKey])
    result = [self _getDelimiterWithType: ULOCDATA_ALT_QUOTATION_END];
#endif
  else if ([key isEqualToString: NSLocaleCalendar])
    result = [self _getCalendar];
  else if ([key isEqualToString: NSLocaleDecimalSeparator])
    result = [self _getDecimalSeparator];
  else if ([key isEqualToString: NSLocaleGroupingSeparator])
    result = [self _getGroupingSeparator];
  else if ([key isEqualToString: NSLocaleCurrencySymbol])
    result = [self _getCurrencySymbol];
  else if ([key isEqualToString: NSLocaleCurrencyCode])
    result = [self _getCurrencyCode];
  
  [_components setValue: result forKey: key];
#endif
  return result;
}

- (NSString *) languageCode
{
  return [self objectForKey: NSLocaleLanguageCode];
}

- (NSString *) countryCode
{
  return [self objectForKey: NSLocaleLanguageCode];
}

- (NSString *) scriptCode
{
  return [self objectForKey: NSLocaleScriptCode];
}

- (NSString *) variantCode
{
  return [self objectForKey: NSLocaleVariantCode];
}

- (NSCharacterSet *) exemplarCharacterSet
{
  return [self objectForKey: NSLocaleExemplarCharacterSet];
}

- (NSString *) collationIdentifier
{
  return [self objectForKey: NSLocaleCollationIdentifier];
}

- (NSString *) collatorIdentifier
{
  return [self objectForKey: NSLocaleCollatorIdentifier];
}

- (NSString *) description
{
  return _localeId;
}

- (BOOL) isEqual: (id)obj
{
  if ([obj isKindOfClass: [self class]])
    {
      return [_localeId isEqual: [obj localeIdentifier]];
    }
  return NO;
}

- (void) dealloc
{
  RELEASE(_localeId);
  RELEASE(_components);
  [super dealloc];
}

- (void) encodeWithCoder: (NSCoder*)encoder
{
  [encoder encodeObject: _localeId];
}

- (id) initWithCoder: (NSCoder*)decoder
{
  NSString	*s = [decoder decodeObject];

  return [self initWithLocaleIdentifier: s];
}

- (id) copyWithZone: (NSZone *) zone
{
  NSLocale *result;
  
  if (NSShouldRetainWithZone(self, zone))
    result = RETAIN(self);
  else
    {
       result = (NSLocale *)NSCopyObject(self, 0, zone);
       result->_localeId = [_localeId copyWithZone: zone];
    }
  
  return result;
}

@end



@implementation NSLocale (PrimateMethods)
+ (void) _updateCanonicalLocales
{
  NSBundle *gbundle = [NSBundle bundleForLibrary: @"gnustep-base"];
  NSString *file = [gbundle pathForResource: @"Locale"
                                     ofType: @"canonical"
                                inDirectory: @"Languages"];
  if (file != nil)
    canonicalLocales = [[NSDictionary alloc] initWithContentsOfFile: file];
}

- (NSString *) _getMeasurementSystem
{
#if GS_USE_ICU == 1
  const char *cLocaleId;
  ULocaleData *localeData;
  UMeasurementSystem msystem;
  UErrorCode err = U_ZERO_ERROR;
  NSString *result = nil;
  
  cLocaleId = [_localeId UTF8String];
  localeData = ulocdata_open (cLocaleId, &err);
  if (U_FAILURE(err))
    return nil;
  
  msystem = ulocdata_getMeasurementSystem (cLocaleId, &err);
  if (U_SUCCESS(err))
    {
      if (msystem == UMS_SI)
        result = @"Metric";
      else
        result = @"U.S.";
    }
  ulocdata_close (localeData);
  return result;
#else
  return nil;
#endif
}

- (NSCharacterSet *) _getExemplarCharacterSet
{
#if GS_USE_ICU == 1
  const char *cLocaleId;
  int idx;
  int count;
  UChar buffer[1024];
    // This is an arbitrary size, increase it if it's not enough.
  ULocaleData *localeData;
  USet *charSet;
  UErrorCode err = U_ZERO_ERROR;
  NSCharacterSet *result;
  NSMutableCharacterSet *mSet;
  
  cLocaleId = [_localeId UTF8String];
  localeData = ulocdata_open(cLocaleId, &err);
  if (U_FAILURE(err))
    {
      return nil;
    }
  
  charSet = ulocdata_getExemplarSet(localeData, NULL,
    USET_ADD_CASE_MAPPINGS, ULOCDATA_ES_STANDARD, &err);
  if (U_FAILURE(err))
    {
      ulocdata_close(localeData);
      return nil;
    }
  ulocdata_close(localeData);
  
  mSet = [[NSMutableCharacterSet alloc] init];
  if (mSet == nil)
    {
      uset_close(charSet);
      return nil;
    }
  
  count = uset_getItemCount(charSet);
  for (idx = 0 ; idx < count ; ++idx)
    {
      UChar32 start, end;
      int strLen;
      
      err = U_ZERO_ERROR;
      strLen = uset_getItem(charSet, idx, &start, &end, buffer, 1024, &err);
      if (U_FAILURE(err))
        {
	  uset_close(charSet);
          RELEASE(mSet);
          return nil;
        }
      if (strLen == 0)
        {
          [mSet addCharactersInRange: NSMakeRange(start, (end - start) + 1)];
        }
      else if (strLen >= 2)
        {
          NSString *str = [NSString stringWithCharacters: buffer
                                                  length: strLen];
          [mSet addCharactersInString: str];
        }
      // FIXME: The icu docs are a bit iffy and don't explain what len == 1
      // means.  So, if it is encountered, we simply skip it.
    }
  uset_close(charSet);
  
  result = [mSet copyWithZone: NULL];
  RELEASE(mSet);

  return AUTORELEASE(result);
#else
  return nil;
#endif
}

- (NSString *) _getDelimiterWithType: (NSInteger) delimiterType
{
#if GS_USE_ICU == 1
  const char *cLocaleId;
  int strLen;
  UErrorCode err = U_ZERO_ERROR;
  ULocaleData *localeData;
  UChar result[32]; // Arbritrary size
  
  cLocaleId = [_localeId UTF8String];
  localeData = ulocdata_open (cLocaleId, &err);
  strLen = ulocdata_getDelimiter (localeData, delimiterType, result, 32, &err);
  ulocdata_close (localeData);
  if (U_SUCCESS(err))
    return [NSString stringWithCharacters: (unichar *)result length: strLen];
#endif
  
  return nil;
}

- (NSCalendar *) _getCalendar
{
#if GS_USE_ICU == 1
  NSCalendar *result;
  NSString *calId;
  int strLen;
  char buffer[ULOC_KEYWORDS_CAPACITY];
  UErrorCode err = U_ZERO_ERROR;
  
  strLen = uloc_getKeywordValue ([_localeId UTF8String], ICUCalendarKeyword,
    buffer, ULOC_KEYWORDS_CAPACITY, &err);
  if (U_SUCCESS(err) && strLen > 0)
    calId = [NSString stringWithUTF8String: buffer];
  else
    calId = NSGregorianCalendar;
  
  result = [[NSCalendar alloc] initWithCalendarIdentifier: calId];
  
  return AUTORELEASE(result);
#else
  return nil;
#endif
}

- (NSString *) _getDecimalSeparator
{
  NSNumberFormatter    *nFor;
  NSString              *result;
  
  nFor = [[NSNumberFormatter alloc] init];
  [nFor setLocale: self];
  [nFor setNumberStyle: NSNumberFormatterDecimalStyle];
  result = [nFor decimalSeparator];
  RELEASE(nFor);
  return result;
}

- (NSString *) _getGroupingSeparator
{
  NSNumberFormatter     *nFor;
  NSString              *result;
  
  nFor = [[NSNumberFormatter alloc] init];
  [nFor setLocale: self];
  [nFor setNumberStyle: NSNumberFormatterDecimalStyle];
  result = [nFor groupingSeparator];
  RELEASE(nFor);
  return result;
}

- (NSString *) _getCurrencySymbol
{
  NSNumberFormatter *nFor;
  NSString *result;
  
  nFor = [[NSNumberFormatter alloc] init];
  [nFor setLocale: self];
  [nFor setNumberStyle: NSNumberFormatterCurrencyStyle];
  result = [nFor currencySymbol];
  
  RELEASE(nFor);
  return result;
}

- (NSString *) _getCurrencyCode
{
  NSNumberFormatter *nFor;
  NSString *result;
  
  nFor = [[NSNumberFormatter alloc] init];
  [nFor setLocale: self];
  [nFor setNumberStyle: NSNumberFormatterCurrencyStyle];
  result = [nFor currencyCode];
  
  RELEASE(nFor);
  return result;
}

@end
