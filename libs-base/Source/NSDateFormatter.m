/** Implementation of NSDateFormatter class
   Copyright (C) 1998 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: December 1998

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.

   <title>NSDateFormatter class reference</title>
   $Date$ $Revision$
   */

#define	GS_NSDateFormatter_IVARS \
  NSUInteger _behavior; \
  NSLocale   *_locale; \
  NSTimeZone *_tz; \
  NSDateFormatterStyle _timeStyle; \
  NSDateFormatterStyle _dateStyle; \
  void      *_formatter

#define	EXPOSE_NSDateFormatter_IVARS	1
#import "common.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSCalendar.h"
#import "Foundation/NSCalendarDate.h"
#import "Foundation/NSLocale.h"
#import "Foundation/NSTimeZone.h"
#import "Foundation/NSFormatter.h"
#import "Foundation/NSDateFormatter.h"
#import "Foundation/NSCoder.h"

#if defined(HAVE_UNICODE_UDAT_H)
#define id id_ucal
#include <unicode/udat.h>
#undef id
#endif
#if defined(HAVE_UNICODE_UDATPG_H)
#include <unicode/udatpg.h>
#endif



// This is defined to be the same as UDAT_RELATIVE
#define FormatterDoesRelativeDateFormatting (1<<16)
#define BUFFER_SIZE 1024

@interface NSDateFormatter (PrivateMethods)
- (void) _resetUDateFormat;
- (void) _setSymbols: (NSArray *)array : (NSInteger)symbol;
- (NSArray *) _getSymbols: (NSInteger)symbol;
@end

static inline NSInteger
NSToUDateFormatStyle (NSDateFormatterStyle style)
{
#if GS_USE_ICU == 1
  NSInteger relative =
    (style & FormatterDoesRelativeDateFormatting) ? UDAT_RELATIVE : 0;
  switch (style)
    {
      case NSDateFormatterNoStyle:
        return (relative | UDAT_NONE);
      case NSDateFormatterShortStyle: 
        return (relative | UDAT_SHORT);
      case NSDateFormatterMediumStyle: 
        return (relative | UDAT_MEDIUM);
      case NSDateFormatterLongStyle:
        return (relative | UDAT_LONG);
      case NSDateFormatterFullStyle: 
        return (relative | UDAT_FULL);
    }
#endif
  return -1;
}


#define	GSInternal		NSDateFormatterInternal
#include	"GSInternal.h"
GS_PRIVATE_INTERNAL(NSDateFormatter)


@implementation NSDateFormatter

static NSDateFormatterBehavior _defaultBehavior = 0;

- (id) init
{
  self = [super init];
  if (self == nil)
    return nil;
  
  GS_CREATE_INTERNAL(NSDateFormatter)

  internal->_behavior = _defaultBehavior;
  internal->_locale = RETAIN([NSLocale currentLocale]);
  internal->_tz = RETAIN([NSTimeZone defaultTimeZone]);
  
  [self _resetUDateFormat];
  
  return self;
}

- (BOOL) allowsNaturalLanguage
{
  return _allowsNaturalLanguage;
}

- (NSAttributedString*) attributedStringForObjectValue: (id)anObject
				 withDefaultAttributes: (NSDictionary*)attr
{
  return nil;
}

- (id) copyWithZone: (NSZone*)zone
{
  NSDateFormatter	*o = (id)NSCopyObject(self, 0, zone);

  IF_NO_GC(RETAIN(o->_dateFormat));
  if (0 != internal)
    {
      GS_COPY_INTERNAL(o, zone)
      IF_NO_GC(RETAIN(GSIVar(o,_locale));)
#if GS_USE_ICU == 1
      {
        UErrorCode err = U_ZERO_ERROR;
        GSIVar(o,_formatter) = udat_clone (internal->_formatter, &err);
      }
#endif
    }
  
  return o;
}

- (NSString*) dateFormat
{
  return _dateFormat;
}

- (void) dealloc
{
  RELEASE(_dateFormat);
  if (internal != 0)
    {
      RELEASE(internal->_locale);
      RELEASE(internal->_tz);
#if GS_USE_ICU == 1
      udat_close (internal->_formatter);
#endif
      GS_DESTROY_INTERNAL(NSDateFormatter)
    }
  [super dealloc];
}

- (NSString*) editingStringForObjectValue: (id)anObject
{
  return [self stringForObjectValue: anObject];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [aCoder encodeValuesOfObjCTypes: "@C", &_dateFormat, &_allowsNaturalLanguage];
}

- (BOOL) getObjectValue: (id*)anObject
	      forString: (NSString*)string
       errorDescription: (NSString**)error
{
  NSCalendarDate	*d;

  if ([string length] == 0)
    {
      d = nil;
    }
  else
    {
      d = [NSCalendarDate dateWithString: string calendarFormat: _dateFormat];
    }
  if (d == nil)
    {
      if (_allowsNaturalLanguage)
	{
	  d = [NSCalendarDate dateWithNaturalLanguageString: string];
	}
      if (d == nil)
	{
	  if (error)
	    {
	      *error = @"Couldn't convert to date";
	    }
	  return NO;
	}
    }
  if (anObject)
    {
      *anObject = d;
    }
  return YES;
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  GS_CREATE_INTERNAL(NSDateFormatter)

  [aCoder decodeValuesOfObjCTypes: "@C", &_dateFormat, &_allowsNaturalLanguage];
  return self;
}

- (id) initWithDateFormat: (NSString *)format
     allowNaturalLanguage: (BOOL)flag
{
  self = [self init];
  if (self == nil)
    return nil;

  [self setDateFormat: format];
  _allowsNaturalLanguage = flag;
  internal->_behavior = NSDateFormatterBehavior10_0;
  return self;
}

- (BOOL) isPartialStringValid: (NSString*)partialString
	     newEditingString: (NSString**)newString
	     errorDescription: (NSString**)error
{
  if (newString)
    {
      *newString = nil;
    }
  if (error)
    {
      *error = nil;
    }
  return YES;
}

- (NSString*) stringForObjectValue: (id)anObject
{
  if ([anObject isKindOfClass: [NSDate class]] == NO)
    {
      return nil;
    }
  return [anObject descriptionWithCalendarFormat: _dateFormat
					timeZone: [NSTimeZone defaultTimeZone]
					  locale: nil];
}



+ (NSDateFormatterBehavior) defaultFormatterBehavior
{
  return _defaultBehavior;
}

+ (void) setDefaultFormatterBehavior: (NSDateFormatterBehavior)behavior
{
  _defaultBehavior = behavior;
}

- (NSDateFormatterBehavior) formatterBehavior
{
  return internal->_behavior;
}

- (void) setFormatterBehavior: (NSDateFormatterBehavior)behavior
{
  internal->_behavior = behavior;
}

- (BOOL) generatesCalendarDates
{
  return NO; // FIXME
}

- (void) setGeneratesCalendarDates: (BOOL)flag
{
  return; // FIXME
}

- (BOOL) isLenient
{
#if GS_USE_ICU == 1
  return (BOOL)udat_isLenient (internal->_formatter);
#else
  return NO;
#endif
}

- (void) setLenient: (BOOL)flag
{
#if GS_USE_ICU == 1
  udat_setLenient (internal->_formatter, flag);
#else
  return;
#endif
}


- (NSDate *) dateFromString: (NSString *) string
{
#if GS_USE_ICU == 1
  NSDate *result = nil;
  UDate date;
  UChar *text;
  int32_t textLength;
  UErrorCode err = U_ZERO_ERROR;
  int32_t pPos = 0;
  
  textLength = [string length];
  text = malloc(sizeof(UChar) * textLength);
  if (text == NULL)
    return nil;
  
  [string getCharacters: text range: NSMakeRange (0, textLength)];
  
  date = udat_parse (internal->_formatter, text, textLength, &pPos, &err);
  if (U_SUCCESS(err))
    result =
      [NSDate dateWithTimeIntervalSince1970: (NSTimeInterval)(date / 1000.0)];
  
  free(text);
  return result;
#else
  return nil;
#endif
}

- (NSString *) stringFromDate: (NSDate *) date
{
#if GS_USE_ICU == 1
  NSString *result;
  int32_t length;
  unichar *string;
  UDate udate = [date timeIntervalSince1970] * 1000.0;
  UErrorCode err = U_ZERO_ERROR;
  
  length = udat_format (internal->_formatter, udate, NULL, 0, NULL, &err);
  string = malloc(sizeof(UChar) * (length + 1));
  err = U_ZERO_ERROR;
  udat_format (internal->_formatter, udate, string, length, NULL, &err);
  if (U_SUCCESS(err))
    {
      result = AUTORELEASE([[NSString allocWithZone: NSDefaultMallocZone()]
        initWithBytesNoCopy: string
        length: length * sizeof(UChar)
        encoding: NSUnicodeStringEncoding
        freeWhenDone: YES]);
      return result;
    }
  
  free(string);
  return nil;
#else
  return nil;
#endif
}

- (BOOL) getObjectValue: (out id *) obj
              forString: (NSString *) string
                  range: (inout NSRange *) range
                  error: (out NSError **) error
{
  return NO; // FIXME
}

- (void) setDateFormat: (NSString *)string
{
  ASSIGNCOPY(_dateFormat, string);
  [self _resetUDateFormat];
}

- (NSDateFormatterStyle) dateStyle
{
  return internal->_dateStyle;
}

- (void) setDateStyle: (NSDateFormatterStyle)style
{
  internal->_dateStyle = style;
  [self _resetUDateFormat];
}

- (NSDateFormatterStyle) timeStyle
{
  return internal->_timeStyle;
}

- (void) setTimeStyle: (NSDateFormatterStyle)style
{
  internal->_timeStyle = style;
  [self _resetUDateFormat];
}

- (NSCalendar *) calendar
{
  return [internal->_locale objectForKey: NSLocaleCalendar];
}

- (void) setCalendar: (NSCalendar *)calendar
{
  NSMutableDictionary *dict;
  NSLocale *locale;
  
  dict = [[NSLocale componentsFromLocaleIdentifier:
    [internal->_locale localeIdentifier]] mutableCopy];
  [dict setValue: calendar forKey: NSLocaleCalendar];
  locale = [[NSLocale alloc] initWithLocaleIdentifier:
    [NSLocale localeIdentifierFromComponents: dict]];
  [self setLocale: locale];
  /* Don't have to use udat_setCalendar here because -setLocale: will take care
     of setting the calendar when it resets the formatter. */
  RELEASE(locale);
  RELEASE(dict);
}

- (NSDate *) defaultDate
{
  return nil;  // FIXME
}

- (void) setDefaultDate: (NSDate *)date
{
  return; // FIXME
}

- (NSLocale *) locale
{
  return internal->_locale;
}

- (void) setLocale: (NSLocale *)locale
{
  if (locale == internal->_locale)
    return;
  RELEASE(internal->_locale);
  
  internal->_locale = RETAIN(locale);
  [self _resetUDateFormat];
}

- (NSTimeZone *) timeZone
{
  return internal->_tz;
}

- (void) setTimeZone: (NSTimeZone *)tz
{
  if (tz == internal->_tz)
    return;
  RELEASE(internal->_tz);
  
  internal->_tz = RETAIN(tz);
  [self _resetUDateFormat];
}

- (NSDate *) twoDigitStartDate
{
#if GS_USE_ICU == 1
  UErrorCode err = U_ZERO_ERROR;
  return [NSDate dateWithTimeIntervalSince1970:
    (udat_get2DigitYearStart (internal->_formatter, &err) / 1000.0)];
#else
  return nil;
#endif
}

- (void) setTwoDigitStartDate: (NSDate *)date
{
#if GS_USE_ICU == 1
  UErrorCode err = U_ZERO_ERROR;
  udat_set2DigitYearStart (internal->_formatter,
                           ([date timeIntervalSince1970] * 1000.0),
                           &err);
#else
  return;
#endif
}


- (NSString *) AMSymbol
{
#if GS_USE_ICU == 1
  NSArray *array = [self _getSymbols: UDAT_AM_PMS];
  
  return [array objectAtIndex: 0];
#else
  return nil;
#endif
}

- (void) setAMSymbol: (NSString *) string
{
  return;
}

- (NSString *) PMSymbol
{
#if GS_USE_ICU == 1
  NSArray *array = [self _getSymbols: UDAT_AM_PMS];
  
  return [array objectAtIndex: 1];
#else
  return nil;
#endif
}

- (void) setPMSymbol: (NSString *)string
{
  return;
}

- (NSArray *) weekdaySymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_WEEKDAYS];
#else
  return nil;
#endif
}

- (void) setWeekdaySymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_WEEKDAYS];
#else
  return;
#endif
}

- (NSArray *) shortWeekdaySymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_SHORT_WEEKDAYS];
#else
  return nil;
#endif
}

- (void) setShortWeekdaySymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_SHORT_WEEKDAYS];
#else
  return;
#endif
}

- (NSArray *) monthSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_MONTHS];
#else
  return nil;
#endif
}

- (void) setMonthSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_MONTHS];
#else
  return;
#endif
}

- (NSArray *) shortMonthSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_SHORT_MONTHS];
#else
  return nil;
#endif
}

- (void) setShortMonthSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_SHORT_MONTHS];
#else
  return;
#endif
}

- (NSArray *) eraSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_ERAS];
#else
  return nil;
#endif
}

- (void) setEraSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_ERAS];
#else
  return;
#endif
}

- (NSDate *) gregorianStartDate
{
  return nil;
}

- (void) setGregorianStartDate: (NSDate *)date
{
  return;
}

- (NSArray *) longEraSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_ERA_NAMES];
#else
  return nil;
#endif
}

- (void) setLongEraSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_ERA_NAMES];
#else
  return;
#endif
}


- (NSArray *) quarterSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_QUARTERS];
#else
  return nil;
#endif
}

- (void) setQuarterSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_QUARTERS];
#else
  return;
#endif
}

- (NSArray *) shortQuarterSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_SHORT_QUARTERS];
#else
  return nil;
#endif
}

- (void) setShortQuarterSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_SHORT_QUARTERS];
#else
  return;
#endif
}

- (NSArray *) standaloneQuarterSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_STANDALONE_QUARTERS];
#else
  return nil;
#endif
}

- (void) setStandaloneQuarterSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_STANDALONE_QUARTERS];
#else
  return;
#endif
}

- (NSArray *) shortStandaloneQuarterSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_STANDALONE_SHORT_QUARTERS];
#else
  return nil;
#endif
}

- (void) setShortStandaloneQuarterSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_STANDALONE_SHORT_QUARTERS];
#else
  return;
#endif
}

- (NSArray *) shortStandaloneMonthSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_STANDALONE_SHORT_MONTHS];
#else
  return nil;
#endif
}

- (void) setShortStandaloneMonthSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_STANDALONE_SHORT_MONTHS];
#else
  return;
#endif
}

- (NSArray *) standaloneMonthSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_STANDALONE_MONTHS];
#else
  return nil;
#endif
}

- (void) setStandaloneMonthSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_STANDALONE_MONTHS];
#else
  return;
#endif
}

- (NSArray *) veryShortMonthSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_NARROW_MONTHS];
#else
  return nil;
#endif
}

- (void) setVeryShortMonthSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_NARROW_MONTHS];
#else
  return;
#endif
}

- (NSArray *) veryShortStandaloneMonthSymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_STANDALONE_NARROW_MONTHS];
#else
  return nil;
#endif
}

- (void) setVeryShortStandaloneMonthSymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_STANDALONE_NARROW_MONTHS];
#else
  return;
#endif
}

- (NSArray *) shortStandaloneWeekdaySymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_STANDALONE_SHORT_WEEKDAYS];
#else
  return nil;
#endif
}

- (void) setShortStandaloneWeekdaySymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_STANDALONE_SHORT_WEEKDAYS];
#else
  return;
#endif
}

- (NSArray *) standaloneWeekdaySymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_STANDALONE_WEEKDAYS];
#else
  return nil;
#endif
}

- (void) setStandaloneWeekdaySymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_STANDALONE_WEEKDAYS];
#else
  return;
#endif
}

- (NSArray *) veryShortWeekdaySymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_SHORT_WEEKDAYS];
#else
  return nil;
#endif
}

- (void) setVeryShortWeekdaySymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_SHORT_WEEKDAYS];
#else
  return;
#endif
}

- (NSArray *) veryShortStandaloneWeekdaySymbols
{
#if GS_USE_ICU == 1
  return [self _getSymbols: UDAT_STANDALONE_NARROW_WEEKDAYS];
#else
  return nil;
#endif
}

- (void) setVeryShortStandaloneWeekdaySymbols: (NSArray *)array
{
#if GS_USE_ICU == 1
  [self _setSymbols: array : UDAT_STANDALONE_NARROW_WEEKDAYS];
#else
  return;
#endif
}

+ (NSString *) localizedStringFromDate: (NSDate *) date
                             dateStyle: (NSDateFormatterStyle) dateStyle
                             timeStyle: (NSDateFormatterStyle) timeStyle
{
  NSString *result;
  NSDateFormatter *fmt = [[self alloc] init];
  
  [fmt setDateStyle: dateStyle];
  [fmt setTimeStyle: timeStyle];
  
  result = [fmt stringFromDate: date];
  RELEASE(fmt);
  
  return result;
}

+ (NSString *) dateFormatFromTemplate: (NSString *) aTemplate
                              options: (NSUInteger) opts
                               locale: (NSLocale *) locale
{
#if GS_USE_ICU == 1
  unichar pat[BUFFER_SIZE];
  unichar skel[BUFFER_SIZE];
  int32_t patLen;
  int32_t skelLen;
  UDateTimePatternGenerator *datpg;
  UErrorCode err = U_ZERO_ERROR;
  
  datpg = udatpg_open ([[locale localeIdentifier] UTF8String], &err);
  if (U_FAILURE(err))
    return nil;
  
  if ((patLen = [aTemplate length]) > BUFFER_SIZE)
    patLen = BUFFER_SIZE;
  [aTemplate getCharacters: pat range: NSMakeRange(0, patLen)];
  
  skelLen = udatpg_getSkeleton (datpg, pat, patLen, skel, BUFFER_SIZE, &err);
  if (U_FAILURE(err))
    return nil;
  
  patLen =
    udatpg_getBestPattern (datpg, skel, skelLen, pat, BUFFER_SIZE, &err);
  
  udatpg_close (datpg);
  return [NSString stringWithCharacters: pat length: patLen];
#else
  return nil;
#endif
}

- (BOOL) doesRelativeDateFormatting
{
  return (internal->_dateStyle & FormatterDoesRelativeDateFormatting) ? YES : NO;
}

- (void) setDoesRelativeDateFormatting: (BOOL)flag
{
  internal->_dateStyle |= FormatterDoesRelativeDateFormatting;
}
@end

@implementation NSDateFormatter (PrivateMethods)
- (void) _resetUDateFormat
{
#if GS_USE_ICU == 1
  UChar *pat = NULL;
  UChar *tzID;
  int32_t patLength = 0;
  int32_t tzIDLength;
  UDateFormatStyle timeStyle;
  UDateFormatStyle dateStyle;
  UErrorCode err = U_ZERO_ERROR;
  
  if (internal->_formatter)
    udat_close (internal->_formatter);
  
  tzIDLength = [[internal->_tz name] length];
  tzID = malloc(sizeof(UChar) * tzIDLength);
  [[internal->_tz name] getCharacters: tzID];
  
  if (self->_dateFormat)
    {
      patLength = [self->_dateFormat length];
      pat = malloc(sizeof(UChar) * patLength);
      [self->_dateFormat getCharacters: pat];
    }
#if U_ICU_VERSION_MAJOR_NUM >= 50
  timeStyle = pat ? UDAT_PATTERN : NSToUDateFormatStyle (internal->_timeStyle);
  dateStyle = pat ? UDAT_PATTERN : NSToUDateFormatStyle (internal->_dateStyle);
#else
  timeStyle = NSToUDateFormatStyle (internal->_timeStyle);
  dateStyle = NSToUDateFormatStyle (internal->_dateStyle);
#endif
  internal->_formatter = udat_open (timeStyle, dateStyle,
                          [[internal->_locale localeIdentifier] UTF8String],
                          tzID, tzIDLength, pat, patLength, &err);
  if (U_FAILURE(err))
    internal->_formatter = NULL;
  if (pat)
    free(pat);
  free(tzID);
#else
  return;
#endif
}

#if GS_USE_ICU == 1
static inline void
symbolRange(NSInteger symbol, int *from)
{
  switch (symbol)
    {
      case UDAT_SHORT_WEEKDAYS:
      case UDAT_STANDALONE_NARROW_WEEKDAYS:
      case UDAT_STANDALONE_SHORT_WEEKDAYS:
      case UDAT_STANDALONE_WEEKDAYS:
      case UDAT_WEEKDAYS:
        /* In ICU days of the week number from 1 rather than zero.
         */
        *from = 1;
        break;

      default:
        *from = 0;
        break;
    }
}
#endif

- (void) _setSymbols: (NSArray*)array : (NSInteger)symbol
{
#if GS_USE_ICU == 1
  int idx;
  int count = udat_countSymbols (internal->_formatter, symbol);
  
  symbolRange(symbol, &idx);
  if ([array count] == count - idx)
    {
      while (idx < count)
        {
          int           length;
          UChar         *value;
          UErrorCode    err = U_ZERO_ERROR;
          NSString      *string = [array objectAtIndex: idx];
          
          length = [string length];
          value = malloc(sizeof(unichar) * length);
          [string getCharacters: value range: NSMakeRange(0, length)];
          udat_setSymbols(internal->_formatter, symbol, idx,
            value, length, &err);
          free(value);
          ++idx;
        }
    }
#endif
  return;
}

- (NSArray *) _getSymbols: (NSInteger)symbol
{
#if GS_USE_ICU == 1
  NSMutableArray        *mArray;
  int                   idx;
  int                   count;
  
  count = udat_countSymbols(internal->_formatter, symbol);
  symbolRange(symbol, &idx);
  mArray = [NSMutableArray arrayWithCapacity: count - idx];
  while (idx < count)
    {
      int               length;
      unichar           *value;
      NSString          *str;
      NSZone            *z = [self zone];
      UErrorCode        err = U_ERROR_LIMIT;
      
      length
        = udat_getSymbols(internal->_formatter, symbol, idx, NULL, 0, &err);
      value = NSZoneMalloc(z, sizeof(unichar) * (length + 1));
      err = U_ZERO_ERROR;
      udat_getSymbols(internal->_formatter, symbol, idx, value, length, &err);
      if (U_SUCCESS(err))
        {
          str = [[NSString allocWithZone: z]
            initWithBytesNoCopy: value
            length: length * sizeof(unichar)
            encoding: NSUnicodeStringEncoding
            freeWhenDone: YES];
          [mArray addObject: str];
          RELEASE(str);
        }
      else
        {
          NSZoneFree (z, value);
        }
      
      ++idx;
    }
  
  return [NSArray arrayWithArray: mArray];
#else
  return nil;
#endif
}
@end

