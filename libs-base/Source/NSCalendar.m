/* NSCalendar.m

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by: Stefan Bidigaray
   Date: December, 2010

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

#import "common.h"
#import "Foundation/NSCalendar.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSException.h"
#import "Foundation/NSLocale.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSString.h"
#import "Foundation/NSTimeZone.h"
#import "Foundation/NSUserDefaults.h"
#import "GNUstepBase/GSLock.h"

#if defined(HAVE_UNICODE_UCAL_H)
#define id ucal_id
#include <unicode/ucal.h>
#include <unicode/uvernum.h>
#undef id
#endif


#if GS_USE_ICU == 1
static UCalendarDateFields _NSCalendarUnitToDateField(NSCalendarUnit unit)
{
  if (unit & NSCalendarUnitEra)
    return UCAL_ERA;
  if (unit & NSCalendarUnitYear)
    return UCAL_YEAR;
  if (unit & NSCalendarUnitMonth)
    return UCAL_MONTH;
  if (unit & NSCalendarUnitDay)
    return UCAL_DAY_OF_MONTH;
  if (unit & NSCalendarUnitHour)
    return UCAL_HOUR_OF_DAY;
  if (unit & NSCalendarUnitMinute)
    return UCAL_MINUTE;
  if (unit & NSCalendarUnitSecond)
    return UCAL_SECOND;
  if (unit & NSCalendarUnitWeekOfYear)
    return UCAL_WEEK_OF_YEAR;
  if (unit & NSCalendarUnitWeekday)
    return UCAL_DAY_OF_WEEK;
  if (unit & NSCalendarUnitWeekdayOrdinal)
    return UCAL_DAY_OF_WEEK_IN_MONTH;
  return (UCalendarDateFields)-1;
}
#endif /* GS_USE_ICU */

typedef struct {
  NSString      *identifier;
  NSString      *localeID;
  NSTimeZone    *tz;
  void          *cal;
  NSInteger     firstWeekday;
  NSInteger     minimumDaysInFirstWeek;
} Calendar;
#define my ((Calendar*)_NSCalendarInternal)

@interface NSCalendar (PrivateMethods)
- (void *) _openCalendarFor: (NSTimeZone *)timeZone;
- (void) _resetCalendar;
- (void *) _UCalendar;
- (NSString *) _localeIDWithLocale: (NSLocale*)locale;
- (NSString *) _localeIdentifier;
- (void) _setLocaleIdentifier: (NSString*)identifier;
@end

static NSCalendar *autoupdatingCalendar = nil;
static NSRecursiveLock *classLock = nil;

#define TZ_NAME_LENGTH 1024
#define SECOND_TO_MILLI 1000.0
#define MILLI_TO_NANO 1000000

@implementation NSCalendar (PrivateMethods)

#if GS_USE_ICU == 1
- (void *) _openCalendarFor: (NSTimeZone *)timeZone
{
  NSString *tzName;
  NSUInteger tzLen;
  unichar cTzId[TZ_NAME_LENGTH];
  const char *cLocaleId;
  UErrorCode err = U_ZERO_ERROR;
  UCalendarType type;

  cLocaleId = [my->localeID UTF8String];
  tzName = [timeZone name];
  tzLen = [tzName length];
  if (tzLen > TZ_NAME_LENGTH)
    {
      tzLen = TZ_NAME_LENGTH;
    }
  [tzName getCharacters: cTzId range: NSMakeRange(0, tzLen)];

  if ([NSGregorianCalendar isEqualToString: my->identifier])
    {
      type = UCAL_GREGORIAN;
    }
  else
    {
#ifndef	UCAL_DEFAULT
/*
 * Older versions of ICU used UCAL_TRADITIONAL rather than UCAL_DEFAULT
 * so if one is not available we use the other.
 */
      type = UCAL_TRADITIONAL;
#else
      type = UCAL_DEFAULT;
#endif
      // We do not need to call uloc_setKeywordValue() here to set the calendar on the locale
      // as the calendar is already encoded in the locale id by _localeIDWithLocale:.
    }

  return ucal_open((const UChar *)cTzId, tzLen, cLocaleId, type, &err);
}
#endif

- (void) _resetCalendar
{
#if GS_USE_ICU == 1
  if (my->cal != NULL)
    {
      ucal_close(my->cal);
    }

  my->cal = [self _openCalendarFor: my->tz];

  if (NSNotFound == my->firstWeekday)
    {
      my->firstWeekday
        = ucal_getAttribute(my->cal, UCAL_FIRST_DAY_OF_WEEK);
    }
  else
    {
      ucal_setAttribute(my->cal, UCAL_FIRST_DAY_OF_WEEK,
        (int32_t)my->firstWeekday);
    }

  if (NSNotFound == my->minimumDaysInFirstWeek)
    {
      my->minimumDaysInFirstWeek
        = ucal_getAttribute(my->cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK);
    }
  else
    {
      ucal_setAttribute(my->cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK,
        (int32_t)my->minimumDaysInFirstWeek);
    }
#endif
}

- (void *) _UCalendar
{
  return my->cal;
}

- (NSString*) _localeIDWithLocale: (NSLocale *)locale
{
  NSString *result;
  NSString *localeId;
  NSMutableDictionary *tmpDict;

  localeId = [locale localeIdentifier];
  if (my->identifier)
    {
      tmpDict = [[NSLocale componentsFromLocaleIdentifier: localeId]
                  mutableCopyWithZone: NULL];
      [tmpDict removeObjectForKey: NSLocaleCalendar];
      [tmpDict setObject: my->identifier forKey: NSLocaleCalendarIdentifier];
      result = [NSLocale localeIdentifierFromComponents: tmpDict];
      RELEASE(tmpDict);
    }
  else
    {
      result = localeId;
    }

  return result;
}

- (NSString*) _localeIdentifier
{
  return my->localeID;
}

- (void) _setLocaleIdentifier: (NSString *) identifier
{
  if ([identifier isEqualToString: my->localeID])
    {
      return;
    }

  ASSIGN(my->localeID, identifier);
  [self _resetCalendar];
}

- (void) _defaultsDidChange: (NSNotification*)n
{
  NSUserDefaults *defs;
  NSString *locale;
  NSString *calendar;
  NSString *tz;

  defs = [NSUserDefaults standardUserDefaults];
  locale = [defs stringForKey: @"Locale"];
  calendar = [defs stringForKey: @"Calendar"];
  tz = [defs stringForKey: @"Local Time Zone"];

  [classLock lock];
  if ([locale isEqual: my->localeID] == NO
    || [calendar isEqual: my->identifier] == NO
    || [tz isEqual: [my->tz name]] == NO)
    {
#if GS_USE_ICU == 1
      ucal_close(my->cal);
#endif

      ASSIGN(my->localeID, locale);
      ASSIGN(my->identifier, calendar);
      RELEASE(my->tz);
      my->tz = [[NSTimeZone alloc] initWithName: tz];

      [self _resetCalendar];
    }
  [classLock unlock];
}
@end

@implementation NSCalendar

+ (void) initialize
{
  if (self == [NSCalendar class])
    {
      classLock = [NSRecursiveLock new];
    }
}

+ (id) currentCalendar
{
  NSCalendar *result;
  NSString *identifier;

  // This identifier may be nil 
  identifier = [[NSLocale currentLocale] objectForKey: NSLocaleCalendarIdentifier];
  result = [[NSCalendar alloc] initWithCalendarIdentifier: identifier];

  return AUTORELEASE(result);
}

+ (id) autoupdatingCurrentCalendar
{
  [classLock lock];
  if (nil == autoupdatingCalendar)
    {
      autoupdatingCalendar = [[self currentCalendar] copy];
      [[NSNotificationCenter defaultCenter]
        addObserver: autoupdatingCalendar
        selector: @selector(_defaultsDidChange:)
        name: NSUserDefaultsDidChangeNotification
        object: nil];
    }

  [classLock unlock];
  return autoupdatingCalendar;
}


+ (id) calendarWithIdentifier: (NSString *) identifier
{
  return AUTORELEASE([[self alloc] initWithCalendarIdentifier: identifier]);
}

- (id) init
{
  return [self initWithCalendarIdentifier: nil];
}

- (id) initWithCalendarIdentifier: (NSString *) identifier
{
  NSAssert(0 == _NSCalendarInternal, NSInvalidArgumentException);
  _NSCalendarInternal = NSZoneCalloc([self zone], sizeof(Calendar), 1);

  my->firstWeekday = NSNotFound;
  my->minimumDaysInFirstWeek = NSNotFound;
  ASSIGN(my->identifier, identifier);
  ASSIGN(my->tz, [NSTimeZone defaultTimeZone]);
  [self setLocale: [NSLocale currentLocale]];

  return self;
}

- (void) dealloc
{
  if (0 != _NSCalendarInternal)
    {
#if GS_USE_ICU == 1
      ucal_close (my->cal);
#endif
      RELEASE(my->identifier);
      RELEASE(my->localeID);
      RELEASE(my->tz);
      NSZoneFree([self zone], _NSCalendarInternal);
    }
  [super dealloc];
}

- (NSString *) calendarIdentifier
{
  return my->identifier;
}


- (NSDateComponents *) components: (NSUInteger) unitFlags
                         fromDate: (NSDate *) date
{
#if GS_USE_ICU == 1
  NSDateComponents *comps;
  UErrorCode err = U_ZERO_ERROR;
  UDate udate;

  udate = (UDate)floor([date timeIntervalSince1970] * SECOND_TO_MILLI);
  ucal_setMillis(my->cal, udate, &err);
  if (U_FAILURE(err))
    {
      return nil;
    }

  comps = [[NSDateComponents alloc] init];
  if (unitFlags & NSCalendarUnitEra)
    {
      [comps setEra: ucal_get(my->cal, UCAL_ERA, &err)];
    }
  if (unitFlags & NSCalendarUnitYear)
    {
      [comps setYear: ucal_get(my->cal, UCAL_YEAR, &err)];
    }
  if (unitFlags & NSCalendarUnitMonth)
    {
      [comps setMonth: ucal_get(my->cal, UCAL_MONTH, &err) + 1];
    }
  if (unitFlags & NSCalendarUnitDay)
    {
      [comps setDay: ucal_get(my->cal, UCAL_DAY_OF_MONTH, &err)];
    }
  if (unitFlags & NSCalendarUnitHour)
    {
      [comps setHour: ucal_get(my->cal, UCAL_HOUR_OF_DAY, &err)];
    }
  if (unitFlags & NSCalendarUnitMinute)
    {
      [comps setMinute: ucal_get(my->cal, UCAL_MINUTE, &err)];
    }
  if (unitFlags & NSCalendarUnitSecond)
    {
      [comps setSecond: ucal_get(my->cal, UCAL_SECOND, &err)];
    }
  if (unitFlags & (NSWeekCalendarUnit | NSCalendarUnitWeekOfYear))
    {
      [comps setWeek: ucal_get(my->cal, UCAL_WEEK_OF_YEAR, &err)];
    }
  if (unitFlags & NSCalendarUnitWeekday)
    {
      [comps setWeekday: ucal_get(my->cal, UCAL_DAY_OF_WEEK, &err)];
    }
  if (unitFlags & NSCalendarUnitWeekdayOrdinal)
    {
      [comps setWeekdayOrdinal:
               ucal_get(my->cal, UCAL_DAY_OF_WEEK_IN_MONTH, &err)];
    }
  if (unitFlags & NSCalendarUnitQuarter)
    {
      [comps setQuarter: (ucal_get(my->cal, UCAL_MONTH, &err) + 3) / 3];
    }
  if (unitFlags & NSCalendarUnitWeekOfMonth)
    {
      [comps setWeekOfMonth: ucal_get(my->cal, UCAL_WEEK_OF_MONTH, &err)];
    }
  if (unitFlags & NSCalendarUnitYearForWeekOfYear)
    {
      [comps setYearForWeekOfYear: ucal_get(my->cal, UCAL_YEAR_WOY, &err)];
    }
  if (unitFlags & NSCalendarUnitNanosecond)
    {
      [comps setNanosecond: ucal_get(my->cal, UCAL_MILLISECOND, &err) * MILLI_TO_NANO];
    }

  return AUTORELEASE(comps);
#else
  return nil;
#endif
}

/*
 * Convenience macro for field extraction.
 * TODO: We need to implement NSWrapCalendarComponents,
 * but it is unclear how that actually works.
 */
#define COMPONENT_DIFF( \
  cal, units, components, toDate, nsunit, setSel, uunit, err) \
do \
  { \
    if (nsunit == (units & nsunit)) \
      { \
        int32_t uunit ## Diff \
          = ucal_getFieldDifference(cal, toDate, uunit, &err); \
        if (U_FAILURE(err)) \
          { \
            RELEASE(components); \
            return nil; \
          } \
        [components setSel uunit ## Diff]; \
      } \
  } while (0)


- (NSDateComponents *) components: (NSUInteger) unitFlags
                         fromDate: (NSDate *) startingDate
                           toDate: (NSDate *) resultDate
                          options: (NSUInteger) opts
{
#if GS_USE_ICU == 1 && (U_ICU_VERSION_MAJOR_NUM > 4 \
  || (U_ICU_VERSION_MAJOR_NUM == 4 && U_ICU_VERSION_MINOR_NUM >= 8))

  NSDateComponents *comps = nil;
  UErrorCode err = U_ZERO_ERROR;
  UDate udateFrom = (UDate)floor([startingDate timeIntervalSince1970] * SECOND_TO_MILLI);
  UDate udateTo = (UDate)floor([resultDate timeIntervalSince1970] * SECOND_TO_MILLI);

  ucal_setMillis(my->cal, udateFrom, &err);
  if (U_FAILURE(err))
    {
      return nil;
    }
  comps = [[NSDateComponents alloc] init];
  /*
   * Since the ICU field difference function automatically advances
   * the calendar as appropriate, we need to process the units from
   * the largest to the smallest.
   */
  COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
    NSCalendarUnitEra, setEra:, UCAL_ERA, err);
  COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
    NSCalendarUnitYear, setYear:, UCAL_YEAR, err);
  COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
    NSCalendarUnitMonth, setMonth:, UCAL_MONTH, err);
  COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
    NSCalendarUnitWeekOfYear, setWeek:, UCAL_WEEK_OF_YEAR, err);
  if (!(unitFlags & NSCalendarUnitWeekOfYear))
    {
      /* We must avoid setting the same unit twice (it would be zero because
       * of the automatic advancement.
       */
      COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
        NSWeekCalendarUnit, setWeek:, UCAL_WEEK_OF_YEAR, err);
    }
  COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
    NSCalendarUnitWeekOfMonth, setWeekOfMonth:, UCAL_WEEK_OF_MONTH, err);
  COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
    NSCalendarUnitDay, setDay:, UCAL_DAY_OF_MONTH, err);
  COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
    NSCalendarUnitWeekdayOrdinal, setWeekdayOrdinal:,
    UCAL_DAY_OF_WEEK_IN_MONTH, err);
  COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
    NSCalendarUnitWeekday, setWeekday:, UCAL_DAY_OF_WEEK, err);
  COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
    NSCalendarUnitHour, setHour:, UCAL_HOUR_OF_DAY, err);
  COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
    NSCalendarUnitMinute, setMinute:, UCAL_MINUTE, err);
  COMPONENT_DIFF(my->cal, unitFlags, comps, udateTo,
    NSCalendarUnitSecond, setSecond:, UCAL_SECOND, err);
  if (unitFlags & NSCalendarUnitNanosecond)
    {
      int32_t ms;

      ms = ucal_getFieldDifference(my->cal, udateTo, UCAL_MILLISECOND, &err);
      if (U_FAILURE(err))
        {
          RELEASE(comps);
          return nil;
        }
      [comps setNanosecond: ms * MILLI_TO_NANO];
    }

  return AUTORELEASE(comps);

#else
  return nil;
#endif
}

#undef COMPONENT_DIFF

#define _ADD_COMPONENT(c, n)           \
  if (opts & NSWrapCalendarComponents) \
    ucal_roll(my->cal, c, n, &err); \
  else \
    ucal_add(my->cal, c, n, &err); \
  if (U_FAILURE(err)) \
    { \
      return nil; \
    }

- (NSDate *) dateByAddingComponents: (NSDateComponents *) comps
                             toDate: (NSDate *) date
                            options: (NSUInteger) opts
{
#if GS_USE_ICU == 1
  NSInteger amount;
  UErrorCode err = U_ZERO_ERROR;
  UDate udate;

  [self _resetCalendar];
  udate = (UDate)([date timeIntervalSince1970] * SECOND_TO_MILLI);
  ucal_setMillis(my->cal, udate, &err);

  if ((amount = [comps era]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_ERA, (int32_t)amount);
    }
  if ((amount = [comps year]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_YEAR, (int32_t)amount);
    }
  if ((amount = [comps month]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_MONTH, (int32_t)amount);
    }
  if ((amount = [comps day]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_DAY_OF_MONTH, (int32_t)amount);
    }
  if ((amount = [comps hour]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_HOUR_OF_DAY, (int32_t)amount);
    }
  if ((amount = [comps minute]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_MINUTE, (int32_t)amount);
    }
  if ((amount = [comps second]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_SECOND, (int32_t)amount);
    }
  if ((amount = [comps week]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_WEEK_OF_YEAR, (int32_t)amount);
    }
  if ((amount = [comps weekday]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_DAY_OF_WEEK, (int32_t)amount);
    }
  if ((amount = [comps weekOfMonth]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_WEEK_OF_MONTH, (int32_t)amount);
    }
  if ((amount = [comps yearForWeekOfYear]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_YEAR_WOY, (int32_t)amount);
    }
  if ((amount = [comps nanosecond]) != NSDateComponentUndefined)
    {
      _ADD_COMPONENT(UCAL_MILLISECOND, (int32_t)(amount / MILLI_TO_NANO));
    }

  udate = ucal_getMillis(my->cal, &err);
  if (U_FAILURE(err))
    {
      return nil;
    }

  return [NSDate dateWithTimeIntervalSince1970: (udate / SECOND_TO_MILLI)];
#else
  return nil;
#endif
}

#undef _ADD_COMPONENT

- (NSDate *) dateFromComponents: (NSDateComponents *) comps
{
#if GS_USE_ICU == 1
  NSInteger amount;
  UDate udate;
  UErrorCode err = U_ZERO_ERROR;
  void *cal;
  NSTimeZone *timeZone;

  timeZone = [comps timeZone];
  if (timeZone == nil)
    {
      timeZone = [self timeZone];
    }
  cal = [self _openCalendarFor: timeZone];
  if (!cal)
    {
      return nil;
    }
  ucal_clear(cal);

  if ((amount = [comps era]) != NSDateComponentUndefined)
    {
      ucal_set(cal, UCAL_ERA, (int32_t)amount);
    }
  if ((amount = [comps year]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_YEAR, (int32_t)amount);
    }
  if ((amount = [comps month]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_MONTH, amount - 1);
    }
  if ((amount = [comps day]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_DAY_OF_MONTH, (int32_t)amount);
    }
  if ((amount = [comps hour]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_HOUR_OF_DAY, (int32_t)amount);
    }
  if ((amount = [comps minute]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_MINUTE, (int32_t)amount);
    }
  if ((amount = [comps second]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_SECOND, (int32_t)amount);
    }
  if ((amount = [comps week]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_WEEK_OF_YEAR, (int32_t)amount);
    }
  if ((amount = [comps weekday]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_DAY_OF_WEEK, (int32_t)amount);
    }
  if ((amount = [comps weekdayOrdinal]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_DAY_OF_WEEK_IN_MONTH, (int32_t)amount);
    }
  if ((amount = [comps weekOfMonth]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_WEEK_OF_MONTH, (int32_t)amount);
    }
  if ((amount = [comps yearForWeekOfYear]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_YEAR_WOY, (int32_t)amount);
    }
  if ((amount = [comps nanosecond]) != NSDateComponentUndefined)
    {
      ucal_set (cal, UCAL_MILLISECOND, (int32_t)(amount / MILLI_TO_NANO));
    }

  udate = ucal_getMillis(cal, &err);
  ucal_close(cal);
  if (U_FAILURE(err))
    {
      return nil;
    }

  return [NSDate dateWithTimeIntervalSince1970: (udate / SECOND_TO_MILLI)];
#else
  return nil;
#endif
}

- (NSLocale *) locale
{
  return AUTORELEASE([[NSLocale alloc] initWithLocaleIdentifier: my->localeID]);
}

- (void) setLocale: (NSLocale *) locale
{
  // It's much easier to keep a copy of the NSLocale's string representation
  // than to have to build it everytime we have to open a UCalendar.
  [self _setLocaleIdentifier: [self _localeIDWithLocale: locale]];
}

- (NSUInteger) firstWeekday
{
  return my->firstWeekday;
}

- (void) setFirstWeekday: (NSUInteger)weekday
{
  my->firstWeekday = weekday;
#if GS_USE_ICU == 1
  ucal_setAttribute(my->cal, UCAL_FIRST_DAY_OF_WEEK, my->firstWeekday);
#endif
}

- (NSUInteger) minimumDaysInFirstWeek
{
  return my->minimumDaysInFirstWeek;
}

- (void) setMinimumDaysInFirstWeek: (NSUInteger)mdw
{
  my->minimumDaysInFirstWeek = (int32_t)mdw;
#if GS_USE_ICU == 1
  ucal_setAttribute(my->cal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK,
    my->minimumDaysInFirstWeek);
#endif
}

- (NSTimeZone *) timeZone
{
  return my->tz;
}

- (void) setTimeZone: (NSTimeZone *) tz
{
  if ([tz isEqual: my->tz])
    {
      return;
    }

  ASSIGN(my->tz, tz);
  [self _resetCalendar];
}

- (NSRange) maximumRangeOfUnit: (NSCalendarUnit) unit
{
  NSRange result = NSMakeRange (0, 0);
#if GS_USE_ICU == 1
  UCalendarDateFields dateField;
  UErrorCode err = U_ZERO_ERROR;

  [self _resetCalendar];
  dateField = _NSCalendarUnitToDateField(unit);
  if (dateField != (UCalendarDateFields)-1)
    {
      // We really don't care if there are any errors...
      result.location =
        (NSUInteger)ucal_getLimit(my->cal, dateField, UCAL_MINIMUM, &err);
      result.length =
        (NSUInteger)ucal_getLimit(my->cal, dateField, UCAL_MAXIMUM, &err)
        - result.location + 1;
      // ICU's month is 0-based, while NSCalendar is 1-based
      if (dateField == UCAL_MONTH)
        {
          result.location += 1;
        }
    }
#endif

  return result;
}

- (NSRange) minimumRangeofUnit: (NSCalendarUnit) unit
{
  NSRange result = NSMakeRange (0, 0);
#if GS_USE_ICU == 1
  UCalendarDateFields dateField;
  UErrorCode err = U_ZERO_ERROR;

  [self _resetCalendar];
  dateField = _NSCalendarUnitToDateField(unit);
  if (dateField != (UCalendarDateFields)-1)
    {
      // We really don't care if there are any errors...
      result.location =
        (NSUInteger)ucal_getLimit(my->cal, dateField, UCAL_GREATEST_MINIMUM, &err);
      result.length =
        (NSUInteger)ucal_getLimit(my->cal, dateField, UCAL_LEAST_MAXIMUM, &err)
        - result.location + 1;
      // ICU's month is 0-based, while NSCalendar is 1-based
      if (dateField == UCAL_MONTH)
        {
          result.location += 1;
        }
    }
#endif
  return result;
}

- (NSUInteger) ordinalityOfUnit: (NSCalendarUnit) smaller
                         inUnit: (NSCalendarUnit) larger
                        forDate: (NSDate *) date
{
  return 0;
}

- (NSRange) rangeOfUnit: (NSCalendarUnit) smaller
                 inUnit: (NSCalendarUnit) larger
                forDate: (NSDate *) date
{
  return NSMakeRange (0, 0);
}

- (BOOL) rangeOfUnit: (NSCalendarUnit) unit
           startDate: (NSDate **) datep
            interval: (NSTimeInterval *)tip
             forDate: (NSDate *)date
{
  return NO;
}

- (BOOL) isEqual: (id) obj
{
#if GS_USE_ICU == 1
  return (BOOL)ucal_equivalentTo(my->cal, [obj _UCalendar]);
#else
  if ([obj isKindOfClass: [self class]])
    {
      if (![my->identifier isEqual: [obj calendarIdentifier]])
        return NO;
      if (![my->localeID isEqual: [obj localeIdentifier]])
        return NO;
      if (![my->tz isEqual: [obj timeZone]])
        return NO;
      if (my->firstWeekday != [obj firstWeekday])
        return NO;
      if (my->minimumDaysInFirstWeek != [obj minimumDaysInFirstWeek])
        return NO;
      return YES;
    }

  return NO;
#endif
}

- (void) encodeWithCoder: (NSCoder*)encoder
{
  [encoder encodeObject: my->identifier];
  [encoder encodeObject: my->localeID];
  [encoder encodeObject: my->tz];
}

- (id) initWithCoder: (NSCoder*)decoder
{
  NSString	*s = [decoder decodeObject];

  [self initWithCalendarIdentifier: s];
  [self _setLocaleIdentifier: [decoder decodeObject]];
  [self setTimeZone: [decoder decodeObject]];

  return self;
}

- (id) copyWithZone: (NSZone*)zone
{
  NSCalendar *result;

  if (NSShouldRetainWithZone(self, zone))
    {
      return RETAIN(self);
    }
  else
    {
      result = [[[self class] allocWithZone: zone]
        initWithCalendarIdentifier: my->identifier];
      [result _setLocaleIdentifier: my->localeID];
      [result setTimeZone: my->tz];
    }

  return result;
}

@end

#undef  my


@implementation NSDateComponents

typedef struct {
  NSInteger era;
  NSInteger year;
  NSInteger month;
  NSInteger day;
  NSInteger hour;
  NSInteger minute;
  NSInteger second;
  NSInteger week;
  NSInteger weekday;
  NSInteger weekdayOrdinal;
  NSInteger quarter;
  NSInteger weekOfMonth;
  NSInteger yearForWeekOfYear;
  BOOL leapMonth;
  NSInteger nanosecond;
  NSCalendar *cal;
  NSTimeZone *tz;
} DateComp;

#define my ((DateComp*)_NSDateComponentsInternal)

- (void) dealloc
{
  if (0 != _NSDateComponentsInternal)
    {
      RELEASE(my->cal);
      RELEASE(my->tz);
      NSZoneFree([self zone], _NSDateComponentsInternal);
    }
  [super dealloc];
}

- (id) init
{
  if (nil != (self = [super init]))
    {
      _NSDateComponentsInternal =
        NSZoneCalloc([self zone], sizeof(DateComp), 1);

      my->era = NSDateComponentUndefined;
      my->year = NSDateComponentUndefined;
      my->month = NSDateComponentUndefined;
      my->day = NSDateComponentUndefined;
      my->hour = NSDateComponentUndefined;
      my->minute = NSDateComponentUndefined;
      my->second = NSDateComponentUndefined;
      my->week = NSDateComponentUndefined;
      my->weekday = NSDateComponentUndefined;
      my->weekdayOrdinal = NSDateComponentUndefined;
      my->quarter = NSDateComponentUndefined;
      my->weekOfMonth = NSDateComponentUndefined;
      my->yearForWeekOfYear = NSDateComponentUndefined;
      my->leapMonth = NO;
      my->nanosecond = NSDateComponentUndefined;
      my->cal = NULL;
      my->tz = NULL;
     }
  return self;
}

- (NSInteger) day
{
  return my->day;
}

- (NSInteger) era
{
  return my->era;
}

- (NSInteger) hour
{
  return my->hour;
}

- (NSInteger) minute
{
  return my->minute;
}

- (NSInteger) month
{
  return my->month;
}

- (NSInteger) quarter
{
  return my->quarter;
}

- (NSInteger) second
{
  return my->second;
}

- (NSInteger) nanosecond
{
  return my->nanosecond;
}

- (NSInteger) week
{
  return my->week;
}

- (NSInteger) weekday
{
  return my->weekday;
}

- (NSInteger) weekdayOrdinal
{
  return my->weekdayOrdinal;
}

- (NSInteger) year
{
  return my->year;
}

- (NSInteger) weekOfMonth
{
  return my->weekOfMonth;
}

- (NSInteger) weekOfYear
{
  return my->week;
}

- (NSInteger) yearForWeekOfYear
{
  return my->yearForWeekOfYear;
}

- (BOOL) leapMonth
{
  return my->leapMonth;
}

- (NSCalendar *) calendar
{
  return my->cal;
}

- (NSTimeZone *) timeZone
{
  return my->tz;
}

- (NSDate *) date
{
  NSCalendar* cal = [self calendar];

  return [cal dateFromComponents: self];
}


- (void) setDay: (NSInteger) v
{
  my->day = v;
}

- (void) setEra: (NSInteger) v
{
  my->era = v;
}

- (void) setHour: (NSInteger) v
{
  my->hour = v;
}

- (void) setMinute: (NSInteger) v
{
  my->minute = v;
}

- (void) setMonth: (NSInteger) v
{
  my->month = v;
}

- (void) setQuarter: (NSInteger) v
{
  my->quarter = v;
}

- (void) setSecond: (NSInteger) v
{
  my->second = v;
}

- (void) setNanosecond: (NSInteger) v
{
  my->nanosecond = v;
}

- (void) setWeek: (NSInteger) v
{
  my->week = v;
}

- (void) setWeekday: (NSInteger) v
{
  my->weekday = v;
}

- (void) setWeekdayOrdinal: (NSInteger) v
{
  my->weekdayOrdinal = v;
}

- (void) setYear: (NSInteger) v
{
  my->year = v;
}

- (void) setWeekOfYear: (NSInteger) v
{
  my->week = v;
}

- (void) setWeekOfMonth: (NSInteger) v
{
  my->weekOfMonth = v;
}

- (void) setYearForWeekOfYear: (NSInteger) v
{
  my->yearForWeekOfYear = v;
}

- (void) setLeapMonth: (BOOL) v
{
  my->leapMonth = v;
}

- (void) setCalendar: (NSCalendar *) cal
{
  ASSIGN(my->cal, cal);
}

- (void) setTimeZone: (NSTimeZone *) tz
{
  ASSIGN(my->tz, tz);
}

- (BOOL) isValidDate
{
  if (my->cal == nil)
    {
      return NO;
    }
  return [self isValidDateInCalendar: my->cal];
}

- (BOOL) isValidDateInCalendar: (NSCalendar *) calendar
{
  return [calendar dateFromComponents: self] != nil;
}

- (NSInteger) valueForComponent: (NSCalendarUnit) unit
{
  switch (unit)
    {
      case NSCalendarUnitEra: return my->era;
      case NSCalendarUnitYear: return my->year;
      case NSCalendarUnitMonth: return my->month;
      case NSCalendarUnitDay: return my->day;
      case NSCalendarUnitHour: return my->hour;
      case NSCalendarUnitMinute: return my->minute;
      case NSCalendarUnitSecond: return my->second;
      case NSCalendarUnitWeekday: return my->weekday;
      case NSCalendarUnitWeekdayOrdinal: return my->weekdayOrdinal;
      case NSCalendarUnitQuarter: return my->quarter;
      case NSCalendarUnitWeekOfMonth: return my->weekOfMonth;
      case NSCalendarUnitWeekOfYear: return my->week;
      case NSWeekCalendarUnit: return my->week;
      case NSCalendarUnitYearForWeekOfYear: return my->yearForWeekOfYear;
      case NSCalendarUnitNanosecond: return my->nanosecond;
      default: return 0;
    }
}

- (void) setValue: (NSInteger) value
     forComponent: (NSCalendarUnit) unit
{
  switch (unit)
    {
      case NSCalendarUnitEra:
        my->era = value;
        break;
      case NSCalendarUnitYear:
        my->year = value;
        break;
      case NSCalendarUnitMonth:
        my->month = value;
        break;
      case NSCalendarUnitDay:
          my->day = value;
        break;
      case NSCalendarUnitHour:
        my->hour = value;
        break;
      case NSCalendarUnitMinute:
        my->minute = value;
        break;
      case NSCalendarUnitSecond:
        my->second = value;
        break;
      case NSCalendarUnitWeekday:
        my->weekday = value;
        break;
      case NSCalendarUnitWeekdayOrdinal:
        my->weekdayOrdinal = value;
        break;
      case NSCalendarUnitQuarter:
        my->quarter = value;
        break;
      case NSCalendarUnitWeekOfMonth:
        my->weekOfMonth = value;
        break;
      case NSCalendarUnitWeekOfYear:
        my->week = value;
        break;
      case NSWeekCalendarUnit:
        my->week = value;
        break;
      case NSCalendarUnitYearForWeekOfYear:
        my->yearForWeekOfYear = value;
        break;
      case NSCalendarUnitNanosecond:
        my->nanosecond = value;
        break;
      default:
        break;
    }
}

- (id) copyWithZone: (NSZone*)zone
{
  if (NSShouldRetainWithZone(self, zone))
    {
      return RETAIN(self);
    }
  else
    {
      NSDateComponents  *c = [[NSDateComponents allocWithZone: zone] init];

      memcpy(c->_NSDateComponentsInternal, _NSDateComponentsInternal,
        sizeof(DateComp));
      /* We gave objects to the copy, so we need to retain them too.
       */
      RETAIN(my->cal);
      RETAIN(my->tz);
      return c;
    }
}

@end
