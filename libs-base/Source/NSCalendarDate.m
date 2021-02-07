/** Implementation for NSCalendarDate for GNUstep
   Copyright (C) 1996, 1998, 1999, 2000, 2002 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: October 1996

   Author: Richard Frith-Macdonald <rfm@gnu.org>
   Date: September 2002

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

   <title>NSCalendarDate class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#define	EXPOSE_NSCalendarDate_IVARS	1
#include <math.h>
#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSCalendarDate.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSException.h"
#import "Foundation/NSTimeZone.h"
#import "Foundation/NSUserDefaults.h"
#import "GNUstepBase/GSObjCRuntime.h"

#import "GSPrivate.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <time.h>
#include <stdio.h>
#include <ctype.h>

@class	GSTimeZone;
@interface	GSTimeZone : NSObject	// Help the compiler
@end
@class	GSAbsTimeZone;
@interface	GSAbsTimeZone : NSObject	// Help the compiler
@end
@class	NSGDate;
@interface	NSGDate : NSObject	// Help the compiler
@end


#define DISTANT_FUTURE	63113990400.0
#define DISTANT_PAST	-63113817600.0

static NSString	*cformat = @"%Y-%m-%d %H:%M:%S %z";

static NSTimeZone	*localTZ = nil;

static Class	NSCalendarDateClass;
static Class	absClass;
static Class	dstClass;

static SEL		offSEL;
static int (*offIMP)(id, SEL, id);
static int (*absOffIMP)(id, SEL, id);
static int (*dstOffIMP)(id, SEL, id);

static SEL		abrSEL;
static NSString* (*abrIMP)(id, SEL, id);
static NSString* (*absAbrIMP)(id, SEL, id);
static NSString* (*dstAbrIMP)(id, SEL, id);

/* Do not fetch the default locale unless we actually need it.
 * Tries to avoid recursion when loading NSUserDefaults containing dates.
 * This is also a little more efficient with many date formats.
 */
#define LOCALE  (nil == locale ? (locale = GSPrivateDefaultLocale()) : locale)


/*
 * Return the offset from GMT for a date in a timezone ...
 * Optimize for the local timezone, and less so for the other
 * base library time zone classes.
 */
static inline int
offset(NSTimeZone *tz, NSDate *d)
{
  if (tz == nil)
    {
      return 0;
    }
  if (tz == localTZ && offIMP != 0)
    {
      return (*offIMP)(tz, offSEL, d);
    }
  else
    {
      Class	c = object_getClass(tz);

      if (c == dstClass && dstOffIMP != 0)
	{
	  return (*dstOffIMP)(tz, offSEL, d);
	}
      if (c == absClass && absOffIMP != 0)
	{
	  return (*absOffIMP)(tz, offSEL, d);
	}
      return [tz secondsFromGMTForDate: d];
    }
}

/*
 * Return the offset from GMT for a date in a timezone ...
 * Optimize for the local timezone, and less so for the other
 * base library time zone classes.
 */
static inline NSString*
abbrev(NSTimeZone *tz, NSDate *d)
{
  if (tz == nil)
    {
      return @"GMT";
    }
  if (tz == localTZ && abrIMP != 0)
    {
      return (*abrIMP)(tz, abrSEL, d);
    }
  else
    {
      Class	c = object_getClass(tz);

      if (c == dstClass && dstAbrIMP != 0)
	{
	  return (*dstAbrIMP)(tz, abrSEL, d);
	}
      if (c == absClass && absAbrIMP != 0)
	{
	  return (*absAbrIMP)(tz, abrSEL, d);
	}
      return [tz abbreviationForDate: d];
    }
}

static inline NSUInteger
lastDayOfGregorianMonth(NSUInteger month, NSUInteger year)
{
  switch (month)
    {
      case 2:
	if ((((year % 4) == 0) && ((year % 100) != 0))
	  || ((year % 400) == 0))
	  return 29;
	else
	  return 28;
      case 4:
      case 6:
      case 9:
      case 11: return 30;
      default: return 31;
    }
}

static inline NSUInteger
absoluteGregorianDay(NSUInteger day, NSUInteger month, NSUInteger year)
{
  if (month > 1)
    {
      while (--month > 0)
	{
	  day = day + lastDayOfGregorianMonth(month, year);
	}
    }
  if (year > 0)
    {
      year--;
    }
  return
    (day            // days this year
     + 365 * year   // days in previous years ignoring leap days
     + year/4       // Julian leap days before this year...
     - year/100     // ...minus prior century years...
     + year/400);   // ...plus prior years divisible by 400
}

static inline NSInteger
dayOfCommonEra(NSTimeInterval when)
{
  NSInteger r;

  // Get reference date in terms of days
  when /= 86400.0;
  // Offset by Gregorian reference
  when += GREGORIAN_REFERENCE;
  r = (NSInteger)when;
  return r;
}

static void
gregorianDateFromAbsolute(NSInteger abs,
  NSInteger *day, NSInteger *month, NSInteger *year)
{
  NSInteger     y;
  NSInteger     m;

  // Search forward year by year from approximate year
  y = abs/366;
  while (abs >= absoluteGregorianDay(1, 1, y+1))
    {
      y++;
    }
  // Search forward month by month from January
  m = 1;
  while (abs > absoluteGregorianDay(lastDayOfGregorianMonth(m, y), m, y))
    {
      m++;
    }
  *year = y;
  *month = m;
  *day = abs - absoluteGregorianDay(1, m, y) + 1;
}

/**
 * Convert a broken out time specification into a time interval
 * since the reference date.
 */
static NSTimeInterval
GSTime(unsigned day, unsigned month, unsigned year, unsigned hour, unsigned minute, unsigned second, unsigned mil)
{
  NSTimeInterval	a;

  a = (NSTimeInterval)absoluteGregorianDay(day, month, year);

  // Calculate date as GMT
  a -= GREGORIAN_REFERENCE;
  a = (NSTimeInterval)a * 86400;
  a += hour * 3600;
  a += minute * 60;
  a += second;
  a += ((NSTimeInterval)mil)/1000.0;
  return a;
}

/**
 * Convert a time interval since the reference date into broken out
 * elements.<br />
 * External - so NSTimeZone  can use it ... but should really be static.
 */
void
GSBreakTime(NSTimeInterval when,
  NSInteger *year, NSInteger *month, NSInteger *day,
  NSInteger *hour, NSInteger *minute, NSInteger *second, NSInteger *mil)
{
  NSInteger h, m, dayOfEra;
  double a, b, c, d;

  /* The 0.1 constant was experimentally derived to cause our behavior
   * to match Mac OS X 10.9.1.
   */
  when = floor(when * 1000.0 + 0.1) / 1000.0;

  // Get reference date in terms of days
  a = when / 86400.0;
  // Offset by Gregorian reference
  a += GREGORIAN_REFERENCE;
  // result is the day of common era.
  dayOfEra = (NSInteger)a;

  // Calculate year, month, and day
  gregorianDateFromAbsolute(dayOfEra, day, month, year);

  // Calculate hour, minute, and seconds
  d = dayOfEra - GREGORIAN_REFERENCE;
  d *= 86400;
  a = fabs(d - when);
  b = a / 3600;
  *hour = (NSInteger)b;
  h = *hour;
  h = h * 3600;
  b = a - h;
  b = b / 60;
  *minute = (NSInteger)b;
  m = *minute;
  m = m * 60;
  c = a - h - m;
  *second = (NSInteger)c;
  *mil = (NSInteger)rint((a - h - m - *second) * 1000.0);
}

/**
 * Returns the current time (seconds since reference date) as an NSTimeInterval.
 */
NSTimeInterval
GSPrivateTimeNow(void)
{
  NSTimeInterval t;
#if !defined(_WIN32)
  struct timeval tp;

  gettimeofday (&tp, NULL);
  t = (NSTimeInterval)tp.tv_sec - NSTimeIntervalSince1970;
  t += (NSTimeInterval)tp.tv_usec / (NSTimeInterval)1000000.0;
#if	1
/* This is a workaround for a bug on some SMP intel systems where the TSC
 * clock information from the processors gets out of sync and causes a
 * leap of 4398 seconds into the future for an instant, and then back.
 * If we detect a time jump back by more than the sort of small interval
 * that ntpd might do (or forwards by a very large amount) we refetch the
 * system time to make sure we don't have a temporary glitch.
 */
{
  static int	old = 0;

  if (old == 0)
    {
      old = tp.tv_sec;
    }
  else
    {
      int	diff = tp.tv_sec - old;

      old = tp.tv_sec;
      if (diff < -1 || diff > 3000)
	{
	  time_t	now = (time_t)tp.tv_sec;

	  fprintf(stderr, "WARNING: system time changed by %d seconds: %s\n",
	    diff, ctime(&now));
	  /* Get time again ... should be OK now.
	   */
	  t = GSPrivateTimeNow();
	}
    }
}
#endif

#else
  SYSTEMTIME sys_time;
  /*
   * Get current GMT time, convert to NSTimeInterval since reference date,
   */
  GetSystemTime(&sys_time);
  t = GSTime(sys_time.wDay, sys_time.wMonth, sys_time.wYear, sys_time.wHour,
    sys_time.wMinute, sys_time.wSecond, sys_time.wMilliseconds);
#endif /* _WIN32 */

  return t;
}

/**
 * An [NSDate] subclass which understands about timezones and provides
 * methods for dealing with date and time information by calendar and
 * with hours minutes and seconds.
 */
@implementation NSCalendarDate

+ (void) initialize
{
  if (self == [NSCalendarDate class] && nil == NSCalendarDateClass)
    {
      NSAutoreleasePool *pool = [NSAutoreleasePool new];

      NSCalendarDateClass = self;
      [self setVersion: 1];
      localTZ = RETAIN([NSTimeZone localTimeZone]);

      dstClass = [GSTimeZone class];
      absClass = [GSAbsTimeZone class];

      offSEL = @selector(secondsFromGMTForDate:);
      offIMP = (int (*)(id,SEL,id))
	[localTZ methodForSelector: offSEL];
      dstOffIMP = (int (*)(id,SEL,id))
	[dstClass instanceMethodForSelector: offSEL];
      absOffIMP = (int (*)(id,SEL,id))
	[absClass instanceMethodForSelector: offSEL];

      abrSEL = @selector(abbreviationForDate:);
      abrIMP = (NSString* (*)(id,SEL,id))
	[localTZ methodForSelector: abrSEL];
      dstAbrIMP = (NSString* (*)(id,SEL,id))
	[dstClass instanceMethodForSelector: abrSEL];
      absAbrIMP = (NSString* (*)(id,SEL,id))
	[absClass instanceMethodForSelector: abrSEL];

      GSObjCAddClassBehavior(self, [NSGDate class]);
      [pool release];
    }
}

/**
 * Return an NSCalendarDate for the current date and time using the
 * default timezone.
 */
+ (id) calendarDate
{
  id	d = [[self alloc] init];

  return AUTORELEASE(d);
}

/**
 * Return an NSCalendarDate generated from the supplied description
 * using the format specified for parsing that string.<br />
 * Calls -initWithString:calendarFormat: to create the date.
 */
+ (id) dateWithString: (NSString *)description
       calendarFormat: (NSString *)format
{
  NSCalendarDate *d = [[self alloc] initWithString: description
				    calendarFormat: format];
  return AUTORELEASE(d);
}

/**
 * Return an NSCalendarDate generated from the supplied description
 * using the format specified for parsing that string and interpreting
 * it according to the dictionary specified.<br />
 * Calls -initWithString:calendarFormat:locale: to create the date.
 */
+ (id) dateWithString: (NSString *)description
       calendarFormat: (NSString *)format
	       locale: (NSDictionary *)dictionary
{
  NSCalendarDate *d = [[self alloc] initWithString: description
				    calendarFormat: format
				    locale: dictionary];
  return AUTORELEASE(d);
}

/**
 * Creates and returns an NSCalendarDate from the specified values
 * by calling -initWithYear:month:day:hour:minute:second:timeZone:
 */
+ (id) dateWithYear: (NSInteger)year
	      month: (NSUInteger)month
	        day: (NSUInteger)day
	       hour: (NSUInteger)hour
	     minute: (NSUInteger)minute
	     second: (NSUInteger)second
	   timeZone: (NSTimeZone *)aTimeZone
{
  NSCalendarDate *d = [[self alloc] initWithYear: year
					   month: month
					     day: day
					    hour: hour
					  minute: minute
					  second: second
					timeZone: aTimeZone];
  return AUTORELEASE(d);
}

/**
 * Creates and returns a new NSCalendarDate object by taking the
 * value of the receiver and adding the interval in seconds specified.
 */
- (id) addTimeInterval: (NSTimeInterval)seconds
{
  return [self dateByAddingTimeInterval: seconds];
}

- (Class) classForCoder
{
  return [self class];
}

/**
 * Creates and returns a new NSCalendarDate object by taking the
 * value of the receiver and adding the interval specified.
 */
- (id) dateByAddingTimeInterval: (NSTimeInterval)seconds
{
  id newObj = [[self class] dateWithTimeIntervalSinceReferenceDate:
     [self timeIntervalSinceReferenceDate] + seconds];
	
  [newObj setTimeZone: [self timeZoneDetail]];
  [newObj setCalendarFormat: [self calendarFormat]];

  return newObj;
}

- (id) replacementObjectForPortCoder: (NSPortCoder*)aRmc
{
  return self;
}

- (void) encodeWithCoder: (NSCoder*)coder
{
  [coder encodeValueOfObjCType: @encode(NSTimeInterval)
			    at: &_seconds_since_ref];
  [coder encodeObject: _calendar_format];
  [coder encodeObject: _time_zone];
}

- (id) initWithCoder: (NSCoder*)coder
{
  [coder decodeValueOfObjCType: @encode(NSTimeInterval)
			    at: &_seconds_since_ref];
  [coder decodeValueOfObjCType: @encode(id) at: &_calendar_format];
  [coder decodeValueOfObjCType: @encode(id) at: &_time_zone];
  return self;
}

- (void) dealloc
{
  RELEASE(_calendar_format);
  RELEASE(_time_zone);
  [super dealloc];
}

/**
 * Initializes an NSCalendarDate using the specified description and the
 * default calendar format and locale.<br />
 * Calls -initWithString:calendarFormat:locale:
 */
- (id) initWithString: (NSString *)description
{
  return [self initWithString: description
	       calendarFormat: cformat
		       locale: nil];
}

/**
 * Initializes an NSCalendarDate using the specified description and format
 * string interpreted in the default locale.<br />
 * Calls -initWithString:calendarFormat:locale:
 */
- (id) initWithString: (NSString *)description
       calendarFormat: (NSString *)format
{
  return [self initWithString: description
	       calendarFormat: format
		       locale: nil];
}

/*
 * read up to the specified number of characters, terminating at a non-digit
 * except for leading whitespace characters.
 */
static inline int getDigits(const char *from, char *to, int limit, BOOL *error)
{
  int	i = 0;
  int	j = 0;

  BOOL	foundDigit = NO;

  while (i < limit)
    {
      if (isdigit(from[i]))
	{
	  to[j++] = from[i];
	  foundDigit = YES;
	}
      else if (isspace(from[i]))
	{
	  if (foundDigit == YES)
	    {
	      break;
	    }
	}
      else
	{
	  break;
	}
      i++;
    }
  to[j] = '\0';
  if (j == 0)
    {
      *error = YES;	// No digits read
    }
  return i;
}

#define	hadY	1
#define	hadM	2
#define	hadD	4
#define	hadh	8
#define	hadm	16
#define	hads	32
#define	hadw	64

/**
 * Initializes an NSCalendarDate using the specified description and format
 * string interpreted in the given locale.<br />
 * If description does not match fmt exactly, this method returns nil.<br />
 * Excess characters in the description (after the format is matched)
 * are ignored.<br />
 * Format specifiers are -
 * <list>
 *   <item>
 *     %%   literal % character
 *   </item>
 *   <item>
 *     %a   abbreviated weekday name according to locale
 *   </item>
 *   <item>
 *     %A   full weekday name according to locale
 *   </item>
 *   <item>
 *     %b   abbreviated month name according to locale
 *   </item>
 *   <item>
 *     %B   full month name according to locale
 *   </item>
 *   <item>
 *     %c   same as '%X %x'
 *   </item>
 *   <item>
 *     %d   day of month as a two digit decimal number
 *   </item>
 *   <item>
 *     %e   same as %d without leading zero
 *   </item>
 *   <item>
 *     %F   milliseconds as a decimal number
 *   </item>
 *   <item>
 *     %H   hour as a decimal number using 24-hour clock
 *   </item>
 *   <item>
 *     %I   hour as a decimal number using 12-hour clock
 *   </item>
 *   <item>
 *     %j   day of year as a decimal number
 *   </item>
 *   <item>
 *     %k   same as %H without leading zero (leading space is used instead)
 *   </item>
 *   <item>
 *     %m   month as decimal number
 *   </item>
 *   <item>
 *     %M   minute as decimal number
 *   </item>
 *   <item>
 *     %p   'am' or 'pm'
 *   </item>
 *   <item>
 *     %S   second as decimal number
 *   </item>
 *   <item>
 *     %U   week of the current year as decimal number (Sunday first day)
 *   </item>
 *   <item>
 *     %W   week of the current year as decimal number (Monday first day)
 *   </item>
 *   <item>
 *     %w   day of the week as decimal number (Sunday = 0)
 *   </item>
 *   <item>
 *     %x   date with date representation for locale
 *   </item>
 *   <item>
 *     %X   time with time representation for locale
 *   </item>
 *   <item>
 *     %y   year as a decimal number without century
 *   </item>
 *   <item>
 *     %Y   year as a decimal number with century
 *   </item>
 *   <item>
 *     %z   time zone offset in hours and minutes from GMT (HHMM)
 *   </item>
 *   <item>
 *     %Z   time zone abbreviation
 *   </item>
 * </list>
 * If no year is specified in the format, the current year is assumed.<br />
 * If no month is specified in the format, January is assumed.<br />
 * If no day is specified in the format, 1 is assumed.<br />
 * If no hour is specified in the format, 0 is assumed.<br />
 * If no minute is specified in the format, 0 is assumed.<br />
 * If no second is specified in the format, 0 is assumed.<br />
 * If no millisecond is specified in the format, 0 is assumed.<br />
 * If no timezone is specified in the format, the local timezone is assumed.
 * <p>If GSMacOSXCompatible is YES, the %k specifier is not recognized.</p>
 * <p>NB. Where the format calls for a numeric value and the string contains
 * fewer digits than expected, the value will be accepted and left padded
 * with zeros to the expected size.<br />
 * For instance, the '%z' format implies four digits (two for the hour
 * offset and two for the digit offset) and if the string contains '01'
 * it will be treated as '0001' ie. a timezone offset of 1 minute.<br />
 * Similarly, the '%F' format implies three digits, so a value of '1'
 * would be treated as '001' or 1 millisecond, not a tenth of a second
 * (as you might assume as '%F' is usually used after a decimal separator).
 * </p>
 */
- (id) initWithString: (NSString*)description
       calendarFormat: (NSString*)fmt
               locale: (NSDictionary*)locale
{
  int		milliseconds = 0;
  int		year = 1;
  int		month = 1;
  int		day = 1;
  int		hour = 0;
  int		min = 0;
  int		sec = 0;
  NSTimeZone	*tz = nil;
  BOOL		ampm = NO;
  BOOL		isPM = NO;
  BOOL		twelveHrClock = NO;
  int		julianWeeks = -1, weekStartsMonday = 0, dayOfWeek = -1;
  const char	*source;
  unsigned	sourceLen;
  unichar	*format;
  unsigned	formatLen;
  unsigned	formatIdx = 0;
  unsigned	sourceIdx = 0;
  char		tmpStr[120];
  unsigned int	tmpIdx;
  unsigned int	tmpEnd;
  unsigned	had = 0;
  unsigned int	pos;
  BOOL		hadPercent = NO;
  NSMutableData	*fd;
  BOOL		changedFormat = NO;
  BOOL		error = NO;

  if (description == nil)
    {
      description = @"";
    }
  source = [description cString];
  sourceLen = strlen(source);
  if (fmt == nil)
    {
      fmt = [LOCALE objectForKey: NSTimeDateFormatString];
      if (fmt == nil)
	{
	  fmt = @"";
	}
    }

  /*
   * Get format into a buffer, leaving room for expansion in case it has
   * escapes that need to be converted.
   */
  formatLen = [fmt length];
  fd = [[NSMutableData alloc]
    initWithLength: (formatLen + 32) * sizeof(unichar)];
  format = (unichar*)[fd mutableBytes];
  [fmt getCharacters: format];

  /*
   * Expand any sequences to their basic components.
   */
  for (pos = 0; pos < formatLen; pos++)
    {
      unichar	c = format[pos];

      if (c == '%')
	{
	  if (hadPercent == YES)
	    {
	      hadPercent = NO;
	    }
	  else
	    {
	      hadPercent = YES;
	    }
	}
      else
	{
	  if (hadPercent == YES)
	    {
	      NSString	*sub = nil;

	      if (c == 'c')
		{
		  sub = [LOCALE objectForKey: NSTimeDateFormatString];
		  if (sub == nil)
		    {
		      sub = @"%X %x";
		    }
		}
	      else if (c == 'R')
		{
		  sub = @"%H:%M";
		}
	      else if (c == 'r')
		{
		  sub = @"%I:%M:%S %p";
		}
	      else if (c == 'T')
		{
		  sub = @"%H:%M:%S";
		}
	      else if (c == 't')
	        {
		  sub = @"\t";
	        }
	      else if (c == 'X')
		{
		  sub = [LOCALE objectForKey: NSTimeFormatString];
		  if (sub == nil)
		    {
		      sub = @"%H-%M-%S";
		    }
		}
	      else if (c == 'x')
		{
		  sub = [LOCALE objectForKey: NSShortDateFormatString];
		  if (sub == nil)
		    {
		      sub = @"%y-%m-%d";
		    }
		}

	      if (sub != nil)
		{
		  unsigned	sLen = [sub length];
		  int	i;

		  if (sLen > 2)
		    {
		      [fd setLength:
			(formatLen + sLen - 2) * sizeof(unichar)];
		      format = (unichar*)[fd mutableBytes];
		      for (i = formatLen-1; i > (NSInteger)pos; i--)
			{
			  format[i+sLen-2] = format[i];
			}
		    }
		  else
		    {
		      for (i = pos+1; i < (NSInteger)formatLen; i++)
			{
			  format[i+sLen-2] = format[i];
			}
		      [fd setLength:
			(formatLen + sLen - 2) * sizeof(unichar)];
		      format = (unichar*)[fd mutableBytes];
		    }
		  [sub getCharacters: &format[pos-1]];
		  formatLen += sLen - 2;
		  changedFormat = YES;
		  pos -= 2;	// Re-parse the newly substituted data.
		}
	    }
	  hadPercent = NO;
	}
    }

  /*
   * Set up calendar format.
   */
  if (changedFormat == YES)
    {
      fmt = [NSString stringWithCharacters: format length: formatLen];
    }
  ASSIGN(_calendar_format, fmt);

  //
  // WARNING:
  //   -Most locale stuff is dubious at best.
  //   -Long day and month names depend on a non-alpha character after the
  //    last digit to work.
  //

  while (error == NO && formatIdx < formatLen)
    {
      if (format[formatIdx] != '%')
	{
	  // If it's not a format specifier, ignore it.
	  if (isspace(format[formatIdx]))
	    {
	      // Skip any amount of white space.
	      while (source[sourceIdx] != 0 && isspace(source[sourceIdx]))
		{
		  sourceIdx++;
		}
	    }
	  else
	    {
	      if (sourceIdx < sourceLen)
		{
		  if (source[sourceIdx] != format[formatIdx])
		    {
		      error = YES;
		      NSDebugMLog(
			@"Expected literal '%c' but got '%c' parsing"
			@"'%@' using '%@'", format[formatIdx],
			source[sourceIdx], description, fmt);
		    }
		  sourceIdx++;
		}
	    }
	}
      else
	{
	  // Skip '%'
	  formatIdx++;
	  while (formatIdx < formatLen && isdigit(format[formatIdx]))
	    {
	      formatIdx++; // skip field width
	    }
	  if (formatIdx < formatLen)
	    {
	      switch (format[formatIdx])
		{
		  case '%':
		    // skip literal %
		    if (sourceIdx < sourceLen)
		      {
			if (source[sourceIdx] != '%')
			  {
			    error = YES;
			    NSDebugMLog(
			      @"Expected literal '%%' but got '%c' parsing"
			      @"'%@' using '%@'", source[sourceIdx],
			      description, fmt);
			  }
			sourceIdx++;
		      }
		    else
		      {
			error = YES;
			NSDebugMLog(
			  @"Expected literal '%%' but got end of string parsing"
			  @"'%@' using '%@'", description, fmt);
		      }
		    break;

		  case 'a':
		    /* FIXME ... Should look for all values from the locale,
		     * matching for longest values first, rather than (wrongly)
		     * assuming a fixed length of three characters.
		     */
		    tmpStr[0] = toupper(source[sourceIdx]);
		    if (sourceIdx < sourceLen)
		      sourceIdx++;
		    tmpStr[1] = tolower(source[sourceIdx]);
		    if (sourceIdx < sourceLen)
		      sourceIdx++;
		    tmpStr[2] = tolower(source[sourceIdx]);
		    if (sourceIdx < sourceLen)
		      sourceIdx++;
		    tmpStr[3] = '\0';
		    {
		      NSString	*currDay;
		      NSArray	*dayNames;

		      currDay = [[NSString alloc] initWithCString: tmpStr];
		      dayNames = [LOCALE objectForKey: NSShortWeekDayNameArray];
		      for (tmpIdx = 0; tmpIdx < 7; tmpIdx++)
			{
			  if ([[dayNames objectAtIndex: tmpIdx] isEqual:
			    currDay] == YES)
			    {
			      break;
			    }
			}
		      if (tmpIdx == 7)
			{
			  error = YES;
			  NSDebugMLog(@"Day of week '%@' not found in locale",
			    currDay);
			}
		      else
			{
			  dayOfWeek = tmpIdx;
			  had |= hadw;
			}
		      RELEASE(currDay);
		    }
		    break;

		  case 'A':
		    /* FIXME ... Should look for all values from the locale,
		     * matching for longest values first, rather than (wrongly)
		     * assuming the name contains only western letters.
		     */
		    tmpEnd = sizeof(tmpStr) - 1;
		    if (sourceLen - sourceIdx < tmpEnd)
		      {
			tmpEnd = sourceLen - sourceIdx;
		      }
		    for (tmpIdx = 0; tmpIdx < tmpEnd; tmpIdx++)
		      {
			if (isalpha(source[sourceIdx + tmpIdx]))
			  {
			    tmpStr[tmpIdx] = source[sourceIdx + tmpIdx];
			  }
			else
			  {
			    break;
			  }
		      }
		    tmpStr[tmpIdx] = '\0';
		    sourceIdx += tmpIdx;
		    {
		      NSString	*currDay;
		      NSArray	*dayNames;

		      currDay = [[NSString alloc] initWithCString: tmpStr];
		      dayNames = [LOCALE objectForKey: NSWeekDayNameArray];
		      for (tmpIdx = 0; tmpIdx < 7; tmpIdx++)
			{
			  if ([[dayNames objectAtIndex: tmpIdx] isEqual:
			    currDay] == YES)
			    {
			      break;
			    }
			}
		      if (tmpIdx == 7)
			{
			  error = YES;
			  NSDebugMLog(@"Day of week '%@' not found in locale",
			    currDay);
			}
		      else
			{
			  dayOfWeek = tmpIdx;
			  had |= hadw;
			}
		      RELEASE(currDay);
		    }
		    break;

		  case 'b':
		    /* FIXME ... Should look for all values from the locale,
		     * matching for longest values first, rather than (wrongly)
		     * assuming a fixed length of three characters.
		     */
		    tmpStr[0] = toupper(source[sourceIdx]);
		    if (sourceIdx < sourceLen)
		      sourceIdx++;
		    tmpStr[1] = tolower(source[sourceIdx]);
		    if (sourceIdx < sourceLen)
		      sourceIdx++;
		    tmpStr[2] = tolower(source[sourceIdx]);
		    if (sourceIdx < sourceLen)
		      sourceIdx++;
		    tmpStr[3] = '\0';
		    {
		      NSString	*currMonth;
		      NSArray	*monthNames;

		      currMonth = [[NSString alloc] initWithCString: tmpStr];
		      monthNames = [LOCALE objectForKey: NSShortMonthNameArray];

		      for (tmpIdx = 0; tmpIdx < 12; tmpIdx++)
			{
			  if ([[monthNames objectAtIndex: tmpIdx]
				    isEqual: currMonth] == YES)
			    {
			      break;
			    }
			}
		      if (tmpIdx == 12)
			{
			  error = YES;
			  NSDebugMLog(@"Month of year '%@' not found in locale",
			    currMonth);
			}
		      else
			{
			  month = tmpIdx+1;
			  had |= hadM;
			}
		      RELEASE(currMonth);
		    }
		    break;

		  case 'B':
		    /* FIXME ... Should look for all values from the locale,
		     * matching for longest values first, rather than (wrongly)
		     * assuming the name contains only western letters.
		     */
		    tmpEnd = sizeof(tmpStr) - 1;
		    if (sourceLen - sourceIdx < tmpEnd)
		      {
			tmpEnd = sourceLen - sourceIdx;
		      }
		    for (tmpIdx = 0; tmpIdx < tmpEnd; tmpIdx++)
		      {
			if (isalpha(source[sourceIdx + tmpIdx]))
			  {
			    tmpStr[tmpIdx] = source[sourceIdx + tmpIdx];
			  }
			else
			  {
			    break;
			  }
		      }
		    tmpStr[tmpIdx] = '\0';
		    sourceIdx += tmpIdx;
		    {
		      NSString	*currMonth;
		      NSArray	*monthNames;

		      currMonth = [[NSString alloc] initWithCString: tmpStr];
		      monthNames = [LOCALE objectForKey: NSMonthNameArray];

		      for (tmpIdx = 0; tmpIdx < 12; tmpIdx++)
			{
			  if ([[monthNames objectAtIndex: tmpIdx]
				    isEqual: currMonth] == YES)
			    {
			      break;
			    }
			}
		      if (tmpIdx == 12)
			{
			  error = YES;
			  NSDebugMLog(@"Month of year '%@' not found in locale",
			    currMonth);
			}
		      else
			{
			  month = tmpIdx+1;
			  had |= hadM;
			}
		      RELEASE(currMonth);
		    }
		    break;

		  case 'd': // fall through
		  case 'e':
		    sourceIdx
		      += getDigits(&source[sourceIdx], tmpStr, 2, &error);
		    day = atoi(tmpStr);
		    had |= hadD;
		    if (error == NO && day < 1)
		      {
			error = YES;
			NSDebugMLog(@"Day of month is zero");
		      }
		    break;

		  case 'F':
		    sourceIdx
		      += getDigits(&source[sourceIdx], tmpStr, 3, &error);
		    milliseconds = atoi(tmpStr);
		    break;

		  case 'k':
		    // GNUstep extension, not available in Cocoa
		    if (GSPrivateDefaultsFlag(GSMacOSXCompatible))
		      {
			error = YES;
			NSLog(@"Invalid NSCalendar date, "
			      @"specifier %c not recognized in format %@",
			      format[formatIdx], fmt);
		      }
		  case 'I': // fall through
		    twelveHrClock = YES;
		  case 'H':
		    sourceIdx
		      += getDigits(&source[sourceIdx], tmpStr, 2, &error);
		    hour = atoi(tmpStr);
		    had |= hadh;
		    break;

		  case 'j':
		    sourceIdx
		      += getDigits(&source[sourceIdx], tmpStr, 3, &error);
		    day = atoi(tmpStr);
		    had |= hadD;
		    break;

		  case 'm':
		    sourceIdx
		      += getDigits(&source[sourceIdx], tmpStr, 2, &error);
		    month = atoi(tmpStr);
		    had |= hadM;
		    if (error == NO && month < 1)
		      {
			error = YES;
			NSDebugMLog(@"Month of year is zero");
		      }
		    break;

		  case 'M':
		    sourceIdx
		      += getDigits(&source[sourceIdx], tmpStr, 2, &error);
		    min = atoi(tmpStr);
		    had |= hadm;
		    break;

		  case 'p':
		    /* FIXME ... Should look for all values from the locale,
		     * matching for longest values first, rather than (wrongly)
		     * assuming the name is always two uppercase letters.
		     */
		    twelveHrClock = YES;
		    tmpStr[0] = toupper(source[sourceIdx]);
		    if (sourceIdx < sourceLen)
		      sourceIdx++;
		    tmpStr[1] = toupper(source[sourceIdx]);
		    if (sourceIdx < sourceLen)
		      sourceIdx++;
		    tmpStr[2] = '\0';
		    {
		      NSString	*currAMPM;
		      NSArray	*amPMNames;

		      currAMPM = [NSString stringWithUTF8String: tmpStr];
		      amPMNames = [LOCALE objectForKey: NSAMPMDesignation];

		      /*
		       * The time addition is handled below because this
		       * indicator only modifies the time on a 12hour clock.
		       */
                      if ([currAMPM caseInsensitiveCompare:
			[amPMNames objectAtIndex: 0]] == NSOrderedSame)
                        {
                          ampm = YES;
                          isPM = NO;
                        }
                      else if ([currAMPM caseInsensitiveCompare:
			[amPMNames objectAtIndex: 1]] == NSOrderedSame)
                        {
                          ampm = YES;
                          isPM = YES;
                        }
		    }
		    break;

		  case 'S':
		    sourceIdx
		      += getDigits(&source[sourceIdx], tmpStr, 2, &error);
		    sec = atoi(tmpStr);
		    had |= hads;
		    break;

		  case 'w':
		    sourceIdx
		      += getDigits(&source[sourceIdx], tmpStr, 1, &error);
		    dayOfWeek = atoi(tmpStr);
		    had |= hadw;
		    break;

		  case 'W': // Fall through
		    weekStartsMonday = 1;
		  case 'U':
		    sourceIdx
		      += getDigits(&source[sourceIdx], tmpStr, 1, &error);
		    julianWeeks = atoi(tmpStr);
		    break;

		    //	case 'x':
		    //	break;

		    //	case 'X':
		    //	break;

		  case 'y':
		    sourceIdx
		      += getDigits(&source[sourceIdx], tmpStr, 2, &error);
		    year = atoi(tmpStr);
		    if (year >= 70)
		      {
			year += 1900;
		      }
		    else
		      {
			year += 2000;
		      }
		    had |= hadY;
		    break;

		  case 'Y':
		    sourceIdx
		      += getDigits(&source[sourceIdx], tmpStr, 4, &error);
		    year = atoi(tmpStr);
		    had |= hadY;
		    break;

		  case 'z':
		    {
		      int	sign = 1;
		      int	zone;
		      int	found;

		      if (source[sourceIdx] == '+')
			{
			  sourceIdx++;
			}
		      else if (source[sourceIdx] == '-')
			{
			  sign = -1;
			  sourceIdx++;
			}
		      found = getDigits(&source[sourceIdx], tmpStr, 4, &error);
		      if (found > 0)
			{
			  sourceIdx += found;
			  zone = atoi(tmpStr);
			  if (found == 2)
			    {
			      zone *= 100;	// Convert 2 digits to 4
			    }
			  tz = [NSTimeZone timeZoneForSecondsFromGMT:
			    sign * ((zone / 100) * 60 + (zone % 100)) * 60];
			}
		    }
		    break;

		  case 'Z':
		    /* Can we assume a timezone name is always space terminated?
		     */
		    tmpEnd = sizeof(tmpStr) - 1;
		    if (sourceLen - sourceIdx < tmpEnd)
		      {
			tmpEnd = sourceLen - sourceIdx;
		      }
		    for (tmpIdx = 0; tmpIdx < tmpEnd; tmpIdx++)
		      {
			if (!isspace(source[sourceIdx + tmpIdx]))
			  {
			    tmpStr[tmpIdx] = source[sourceIdx + tmpIdx];
			  }
			else
			  {
			    break;
			  }
		      }
		    tmpStr[tmpIdx] = '\0';
		    sourceIdx += tmpIdx;
		    {
		      NSString	*z = [NSString stringWithUTF8String: tmpStr];

		      /* Abbreviations aren't one-to-one with time zone names
			 so just look for the zone named after the abbreviation,
			 then look up the abbreviation as a last resort */
		      if ([z length] > 0)
		        {
		          tz = [NSTimeZone timeZoneWithName: z];
		          if (tz == nil)
			    {
			      tz = [NSTimeZone timeZoneWithAbbreviation: z];
			      if (tz == nil)
			        {
			          error = YES;
			          NSDebugMLog(@"Time zone '%@' not found", z);
			        }
			    }
		        }
                      else
                        {
                           error = YES;
                           NSDebugMLog(@"Time zone not given");
                        }
		    }
		    break;

		  default:
		    error = YES;
		    NSLog(@"Invalid NSCalendar date, "
		      @"specifier %c not recognized in format %@",
		      format[formatIdx], fmt);
		    break;
		}
	    }
	}
      formatIdx++;
    }
  RELEASE(fd);

  if (error == NO)
    {
      if (tz == nil)
	{
	  tz = localTZ;
	}

      if (twelveHrClock == YES)
        {
          if (ampm == YES && isPM == YES && hour != 12)
            {
              hour += 12;
            }
          else if (ampm == YES && isPM == NO && hour == 12)
            {
              hour = 0; // 12 AM
            }
        }

      if (julianWeeks != -1)
	{
	  NSTimeZone		*gmtZone;
	  NSCalendarDate	*d;
	  int			currDay;

	  gmtZone = [NSTimeZone timeZoneForSecondsFromGMT: 0];

	  if ((had & (hadY|hadw)) != (hadY|hadw))
	    {
	      NSCalendarDate	*now = [[NSCalendarDateClass alloc] init];

	      [now setTimeZone: gmtZone];
	      if ((had & hadY) == 0)
		{
		  year = [now yearOfCommonEra];
		  had |= hadY;
		}
	      if ((had & hadw) == 0)
		{
		  dayOfWeek = [now dayOfWeek];
		  had |= hadw;
		}
	      RELEASE(now);
	    }

	  d = [[NSCalendarDateClass alloc] initWithYear: year
						  month: 1
						    day: 1
						   hour: 0
						 minute: 0
						 second: 0
					       timeZone: gmtZone];
	  currDay = [d dayOfWeek];
	  RELEASE(d);

	  /*
	   * The julian weeks are either sunday relative or monday relative
	   * but all of the day of week specifiers are sunday relative.
	   * This means that if no day of week specifier was used the week
	   * starts on monday.
	   */
	  if (dayOfWeek == -1)
	    {
	      if (weekStartsMonday)
		{
		  dayOfWeek = 1;
		}
	      else
		{
		  dayOfWeek = 0;
		}
	    }
	  day = dayOfWeek + (julianWeeks * 7 - (currDay - 1));
	  had |= hadD;
	}

      /*
       * If the year has not been set ... use this year ... as on MacOS-X
       */
      if ((had & hadY) == 0)
	{
	  NSCalendarDate	*now = [[NSCalendarDateClass alloc] init];

	  year = [now yearOfCommonEra];
	  RELEASE(now);
	}

      self = [self initWithYear: year
			  month: month
			    day: day
			   hour: hour
			 minute: min
			 second: sec
		       timeZone: tz];
      if (self != nil)
	{
	  _seconds_since_ref += ((NSTimeInterval)milliseconds) / 1000.0;
	}
    }

  if (error == YES)
    {
      DESTROY(self);
    }
  return self;
}

/**
 * Returns an NSCalendarDate instance with the given year, month, day,
 * hour, minute, and second, using aTimeZone.<br />
 * The year includes the century (ie you can't just say '02' when you
 * mean '2002').<br />
 * The month is in the range 1 to 12,<br />
 * The day is in the range 1 to 31,<br />
 * The hour is in the range 0 to 23,<br />
 * The minute is in the range 0 to 59,<br />
 * The second is in the range 0 to 59.<br />
 * If aTimeZone is nil, the [NSTimeZone+localTimeZone] value is used.
 * <p>
 *   GNUstep checks the validity of the method arguments, and unless
 *   the base library was built with 'warn=no' it generates a warning
 *   for bad values.  It tries to use those bad values to generate a
 *   date anyway though, rather than failing (this also appears to be
 *   the behavior of MacOS-X).
 * </p>
 * The algorithm GNUstep uses to create the date is this ...<br />
 * <list>
 *   <item>
 *     Convert the broken out date values into a time interval since
 *     the reference date, as if those values represent a GMT date/time.
 *   </item>
 *   <item>
 *     Ask the time zone for the offset from GMT at the resulting date,
 *     and apply that offset to the time interval ... so get the value
 *     for the specified timezone.
 *   </item>
 *   <item>
 *     Ask the time zone for the offset from GMT at the new date ...
 *     in case the new date is in a different daylight savings time
 *     band from the original date.  If this offset differs from the
 *     previous one, apply the difference so that the result is
 *     corrected for daylight savings.  This is the final result used.
 *   </item>
 *   <item>
 *     After establishing the time interval we will use and completing
 *     initialisation, we ask the time zone for the offset from GMT again.
 *     If it is not the same as the last time, then the time specified by
 *     the broken out date does not really exist ... since it's in the
 *     period lost by the transition to daylight savings.  The resulting
 *     date is therefore not the date that was actually asked for, but is
 *     the best approximation we can do.  If the base library was not
 *     built with 'warn=no' then a warning message is logged for this
 *     condition.
 *   </item>
 * </list>
 */
- (id) initWithYear: (NSInteger)year
	      month: (NSUInteger)month
	        day: (NSUInteger)day
	       hour: (NSUInteger)hour
	     minute: (NSUInteger)minute
	     second: (NSUInteger)second
	   timeZone: (NSTimeZone *)aTimeZone
{
  unsigned int		c;
  NSTimeInterval	s;
  NSTimeInterval	oldOffset;
  NSTimeInterval	newOffset;

  if (month < 1 || month > 12)
    {
      NSWarnMLog(@"invalid month given - %"PRIuPTR, month);
    }
  c = lastDayOfGregorianMonth(month, year);
  if (day < 1 || day > c)
    {
      NSWarnMLog(@"invalid day given - %"PRIuPTR, day);
    }
  if (hour > 23)
    {
      NSWarnMLog(@"invalid hour given - %"PRIuPTR, hour);
    }
  if (minute > 59)
    {
      NSWarnMLog(@"invalid minute given - %"PRIuPTR, minute);
    }
  if (second > 59)
    {
      NSWarnMLog(@"invalid second given - %"PRIuPTR, second);
    }

  // Calculate date as GMT
  s = GSTime(day, month, year, hour, minute, second, 0);

  // Assign time zone detail
  if (aTimeZone == nil)
    {
      _time_zone = localTZ;	// retain is a no-op for the local timezone.
    }
  else
    {
      _time_zone = RETAIN(aTimeZone);
    }
  if (_calendar_format == nil)
    {
      _calendar_format = cformat;
    }
  _seconds_since_ref = s;

  /*
   * Adjust date so it is correct for time zone.
   */
  oldOffset = offset(_time_zone, self);
  s -= oldOffset;
  _seconds_since_ref = s;

  /*
   * See if we need to adjust for daylight savings time
   */
  newOffset = offset(_time_zone, self);
  if (oldOffset != newOffset)
    {
      s -= (newOffset - oldOffset);
      _seconds_since_ref = s;
      oldOffset = offset(_time_zone, self);
      /*
       * If the adjustment puts us in another offset, we must be in the
       * non-existent period at the start of daylight savings time.
       */
      if (oldOffset != newOffset)
	{
	  NSWarnMLog(@"init non-existent time at start of daylight savings");
	}
    }

  return self;
}

/**
 * Initialises the receiver with the specified interval since the
 * reference date.  Uses th standard format string "%Y-%m-%d %H:%M:%S %z"
 * and the default time zone.
 */
- (id) initWithTimeIntervalSinceReferenceDate: (NSTimeInterval)seconds
{
  if (isnan(seconds))
    {
      [NSException raise: NSInvalidArgumentException
	          format: @"[%@-%@] interval is not a number",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
#if	GS_SIZEOF_VOIDP == 4
  if (seconds <= DISTANT_PAST)
    {
      seconds = DISTANT_PAST;
    }
  else if (seconds >= DISTANT_FUTURE)
    {
      seconds = DISTANT_FUTURE;
    }
#endif
  _seconds_since_ref = seconds;
  if (_calendar_format == nil)
    {
      _calendar_format = cformat;
    }
  if (_time_zone == nil)
    {
      _time_zone = localTZ;	// retain is a no-op for the local timezone.
    }
  return self;
}

/**
 * Return the day number (ie number of days since the start of) in the
 * 'common' era of the receiving date.  The era starts at 1 A.D.
 */
- (NSInteger) dayOfCommonEra
{
  NSTimeInterval	when;

  when = _seconds_since_ref + offset(_time_zone, self);
  return dayOfCommonEra(when);
}

/**
 * Return the month (1 to 31) of the receiving date.
 */
- (NSInteger) dayOfMonth
{
  NSInteger m, d, y;
  NSTimeInterval	when;

  when = _seconds_since_ref + offset(_time_zone, self);
  gregorianDateFromAbsolute(dayOfCommonEra(when), &d, &m, &y);

  return d;
}

/**
 * Return the day of the week (0 to 6) of the receiving date.
 * <list>
 *   <item>0 is sunday</item>
 *   <item>1 is monday</item>
 *   <item>2 is tuesday</item>
 *   <item>3 is wednesday</item>
 *   <item>4 is thursday</item>
 *   <item>5 is friday</item>
 *   <item>6 is saturday</item>
 * </list>
 */
- (NSInteger) dayOfWeek
{
  NSInteger	        d;
  NSTimeInterval	when;

  when = _seconds_since_ref + offset(_time_zone, self);
  d = dayOfCommonEra(when);

  /* The era started on a sunday.
     Did we always have a seven day week?
     Did we lose week days changing from Julian to Gregorian?
     AFAIK seven days a week is ok for all reasonable dates.  */
  d = d % 7;
  if (d < 0)
    d += 7;
  return d;
}

/**
 * Return the day of the year (1 to 366) of the receiving date.
 */
- (NSInteger) dayOfYear
{
  NSInteger m, d, y, days, i;
  NSTimeInterval	when;

  when = _seconds_since_ref + offset(_time_zone, self);
  gregorianDateFromAbsolute(dayOfCommonEra(when), &d, &m, &y);
  days = d;
  for (i = m - 1;  i > 0; i--) // days in prior months this year
    days = days + lastDayOfGregorianMonth(i, y);

  return days;
}

/**
 * Return the hour of the day (0 to 23) of the receiving date.
 */
- (NSInteger) hourOfDay
{
  NSInteger h;
  double a, d;
  NSTimeInterval	when;

  when = _seconds_since_ref + offset(_time_zone, self);
  d = dayOfCommonEra(when);
  d -= GREGORIAN_REFERENCE;
  d *= 86400;
  a = fabs(d - (_seconds_since_ref + offset(_time_zone, self)));
  a = a / 3600;
  h = (NSInteger)a;

  // There is a small chance of getting
  // it right at the stroke of midnight
  if (h == 24)
    h = 0;

  return h;
}

/**
 * Return the minute of the hour (0 to 59) of the receiving date.
 */
- (NSInteger) minuteOfHour
{
  NSInteger h, m;
  double a, b, d;
  NSTimeInterval	when;

  when = _seconds_since_ref + offset(_time_zone, self);
  d = dayOfCommonEra(when);
  d -= GREGORIAN_REFERENCE;
  d *= 86400;
  a = fabs(d - (_seconds_since_ref + offset(_time_zone, self)));
  b = a / 3600;
  h = (NSInteger)b;
  h = h * 3600;
  b = a - h;
  b = b / 60;
  m = (NSInteger)b;

  return m;
}

/**
 * Return the month of the year (1 to 12) of the receiving date.
 */
- (NSInteger) monthOfYear
{
  NSInteger m, d, y;
  NSTimeInterval	when;

  when = _seconds_since_ref + offset(_time_zone, self);
  gregorianDateFromAbsolute(dayOfCommonEra(when), &d, &m, &y);

  return m;
}

/**
 * Return the second of the minute (0 to 59) of the receiving date.
 */
- (NSInteger) secondOfMinute
{
  NSInteger h, m, s;
  double a, b, c, d;
  NSTimeInterval	when;

  when = _seconds_since_ref + offset(_time_zone, self);
  d = dayOfCommonEra(when);
  d -= GREGORIAN_REFERENCE;
  d *= 86400;
  a = fabs(d - (_seconds_since_ref + offset(_time_zone, self)));
  b = a / 3600;
  h = (NSInteger)b;
  h = h * 3600;
  b = a - h;
  b = b / 60;
  m = (NSInteger)b;
  m = m * 60;
  c = a - h - m;
  s = (NSInteger)c;

  return s;
}

/**
 * Return the year of the 'common' era of the receiving date.
 * The era starts at 1 A.D.
 */
- (NSInteger) yearOfCommonEra
{
  NSInteger m, d, y;
  NSTimeInterval	when;

  when = _seconds_since_ref + offset(_time_zone, self);
  gregorianDateFromAbsolute(dayOfCommonEra(when), &d, &m, &y);

  return y;
}

/**
 * This method exists solely for conformance to the OpenStep spec.
 * Its use is deprecated ... it simply calls
 * -dateByAddingYears:months:days:hours:minutes:seconds:
 */
- (NSCalendarDate*) addYear: (NSInteger)year
		      month: (NSInteger)month
			day: (NSInteger)day
		       hour: (NSInteger)hour
		     minute: (NSInteger)minute
		     second: (NSInteger)second
{
  return [self dateByAddingYears: year
		          months: month
			    days: day
			   hours: hour
		         minutes: minute
		         seconds: second];
}

/**
 * Calls -descriptionWithCalendarFormat:locale: passing the receiver's
 * calendar format and a nil locale.
 */
- (NSString*) description
{
  return [self descriptionWithCalendarFormat: _calendar_format locale: nil];
}

/**
 * Returns a string representation of the receiver using the specified
 * format string.<br />
 * Calls -descriptionWithCalendarFormat:locale: with a nil locale.
 */
- (NSString*) descriptionWithCalendarFormat: (NSString *)format
{
  return [self descriptionWithCalendarFormat: format locale: nil];
}

#define UNIX_REFERENCE_INTERVAL -978307200.0

typedef struct {
  unichar	*base;
  unichar	*t;
  unsigned	length;
  unsigned	offset;
  NSInteger	yd;
  NSInteger	md;
  NSInteger	dom;
  NSInteger	hd;
  NSInteger	mnd;
  NSInteger	sd;
  NSInteger	mil;
} DescriptionInfo;

static void Grow(DescriptionInfo *info, unsigned size)
{
  if (info->offset + size >= info->length)
    {
      if (info->t == info->base)
	{
	  unichar	*old = info->t;

	  info->t = NSZoneMalloc(NSDefaultMallocZone(),
	    (info->length + 512) * sizeof(unichar));
	  memcpy(info->t, old, info->length*sizeof(unichar));
	}
      else
	{
	  info->t = NSZoneRealloc(NSDefaultMallocZone(), info->t,
	    (info->length + 512) * sizeof(unichar));
	}
      info->length += 512;
    }
}

#define MAX_FLD_WIDTH 99

static void outputValueWithFormat(int v, char *fldfmt, DescriptionInfo *info)
{
  char	cbuf[MAX_FLD_WIDTH + 1];
  int	idx = 0;

  snprintf((char*)cbuf, sizeof(cbuf), fldfmt, v);
  Grow(info, strlen((char*)cbuf));
  while (cbuf[idx] != '\0')
    {
      info->t[info->offset++] = cbuf[idx++];
    }
}

- (void) _format: (NSString*)fmt
	  locale: (NSDictionary*)locale
	    info: (DescriptionInfo*)info
{
  unichar	fbuf[512];
  unichar	*f = fbuf;
  unsigned	lf = [fmt length];
  unsigned	i = 0;
  int		v;

  if (lf == 0)
    {
      return;	// Nothing to do.
    }
  if (lf >= sizeof(fbuf)/sizeof(unichar))
    {
      /*
       * Make temporary buffer to hold format string as unicode.
       */
      f = (unichar*)NSZoneMalloc(NSDefaultMallocZone(), lf*sizeof(unichar));
    }
  [fmt getCharacters: f];

  while (i < lf)
    {
      NSString  *str;
      BOOL	mtag = NO;
      BOOL	dtag = NO;
      BOOL	ycent = NO;
      BOOL	mname = NO;
      BOOL	dname = NO;
      BOOL	twelve = NO;
      BOOL	hspc = NO;
      char	fldfmt[8];
      int	fmtlen = 0;
      int	width = 0;

      // Only care about a format specifier
      if (f[i] == '%')
	{
          i++;
	  fldfmt[fmtlen++] = '%';
          // field width specified
          while (fmtlen < 5 && f[i] >= '0' && f[i] <= '9')
	    {
	      fldfmt[fmtlen++] = f[i];
	      width = 10 * width + f[i] - '0';
	      i++;
	    }
	  if (fmtlen >= 5 || width > MAX_FLD_WIDTH)
	    {
	      /* ignore formats that specify field width
               * greater than the max allowed.
               * set i back so all ignored characters will
               * be copied
               */
	      i -= fmtlen;
	    }

	  // check the character that comes after
	  switch (f[i++])
	    {
		// literal %
	      case '%':
		Grow(info, 1);
		info->t[info->offset++] = f[i-1];
		break;

	      case 'R':
		[self _format: @"%H:%M" locale: locale info: info];
		break;

	      case 'r':
		[self _format: @"%I:%M:%S %p" locale: locale info: info];
		break;

	      case 'T':
		[self _format: @"%H:%M:%S" locale: locale info: info];
		break;

	      case 't':
		Grow(info, 1);
		info->t[info->offset++] = '\t';
		break;

	      case 'c':
		str = (NSString*)[LOCALE objectForKey: NSTimeFormatString];
		[self _format: str locale: locale info: info];
		Grow(info, 1);
		info->t[info->offset++] = ' ';
		str = (NSString*)[LOCALE objectForKey: NSDateFormatString];
		[self _format: str locale: locale info: info];
		break;

	      case 'X':
		str = (NSString*)[LOCALE objectForKey: NSTimeFormatString];
		[self _format: str locale: locale info: info];
		break;

	      case 'x':
		str = (NSString*)[LOCALE objectForKey: NSDateFormatString];
		[self _format: str locale: locale info: info];
		break;

		// is it the year
	      case 'Y':
		ycent = YES;
	      case 'y':
		v = info->yd;
		if (ycent)
		  {
		    if (fmtlen == 1)
		      {
			// no format width specified; supply default
			fldfmt[fmtlen++] = '0';
			fldfmt[fmtlen++] = '4';
		      }
		  }
		else
		  {
		    if (v < 0) v = -v;
		    v = v % 100;
		    if (fmtlen == 1)
		      {
			// no format width specified; supply default
			fldfmt[fmtlen++] = '0';
			fldfmt[fmtlen++] = '2';
		      }
		  }
		  fldfmt[fmtlen++] = 'd';
		  fldfmt[fmtlen] = 0;
		  outputValueWithFormat(v, fldfmt, info);
		break;

		// is it the month
	      case 'b':
		mname = YES;
	      case 'B':
		mtag = YES;    // Month is character string
	      case 'm':
		if (mtag == YES)
		  {
		    NSArray	*months;

		    if (mname)
		      months = [LOCALE objectForKey: NSShortMonthNameArray];
		    else
		      months = [LOCALE objectForKey: NSMonthNameArray];
		    if (info->md > [months count])
		      {
			mtag = NO;
		      }
		    else
		      {
			NSString	*name;
		
			name = [months objectAtIndex: info->md-1];
			v = [name length];
			Grow(info, v);
			[name getCharacters: info->t + info->offset];
			info->offset += v;
		      }
		  }
		if (mtag == NO)
		  {
		    v = info->md;
		    v = v % 100;
		    if (fmtlen == 1)
		      {
			// no format width specified; supply default
			fldfmt[fmtlen++] = '0';
			fldfmt[fmtlen++] = '2';
		      }
		    fldfmt[fmtlen++] = 'd';
		    fldfmt[fmtlen] = 0;
		    outputValueWithFormat(v, fldfmt, info);
		  }
		break;

	      case 'd': 	// day of month with leading zero
		v = info->dom;
		v = v % 100;
		if (fmtlen == 1) // no format width specified; supply default
		  {
		    fldfmt[fmtlen++] = '0';
		    fldfmt[fmtlen++] = '2';
		  }
		fldfmt[fmtlen++] = 'd';
		fldfmt[fmtlen] = 0;
		outputValueWithFormat(v, fldfmt, info);
		break;

	      case 'e': 	// day of month with leading space
		v = info->dom;
		v = v % 100;
		if (fmtlen == 1) // no format width specified; supply default
		  {
		    fldfmt[fmtlen++] = '1'; // no leading space, just like Cocoa
		  }
		fldfmt[fmtlen++] = 'd';
		fldfmt[fmtlen] = 0;
		outputValueWithFormat(v, fldfmt, info);
		break;

	      case 'F': 	// milliseconds
                v = info->mil;
		if (fmtlen == 1) // no format width specified; supply default
		  {
		    fldfmt[fmtlen++] = '0';
		    fldfmt[fmtlen++] = '3';
		  }
		fldfmt[fmtlen++] = 'd';
		fldfmt[fmtlen] = 0;
		outputValueWithFormat(v, fldfmt, info);
		break;

	      case 'j': 	// day of year
		v = [self dayOfYear];
		if (fmtlen == 1) // no format width specified; supply default
		  {
		    fldfmt[fmtlen++] = '0';
		    fldfmt[fmtlen++] = '3';
		  }
		fldfmt[fmtlen++] = 'd';
		fldfmt[fmtlen] = 0;
		outputValueWithFormat(v, fldfmt, info);
		break;

		// is it the week-day
	      case 'a':
		dname = YES;
	      case 'A':
		dtag = YES;   // Day is character string
	      case 'w':
		{
		  v = [self dayOfWeek];
		  if (dtag == YES)
		    {
		      NSArray	*days;

		      if (dname)
			days = [LOCALE objectForKey: NSShortWeekDayNameArray];
		      else
			days = [LOCALE objectForKey: NSWeekDayNameArray];
		      if (v < [days count])
			{
			  NSString	*name;

			  name = [days objectAtIndex: v];
			  v = [name length];
			  Grow(info, v);
			  [name getCharacters: info->t + info->offset];
			  info->offset += v;
			}
		      else
			{
			  dtag = NO;
			}
		    }
		  if (dtag == NO)
		    {
		      if (fmtlen == 1)
			{
			  // no format width specified; supply default
			  fldfmt[fmtlen++] = '1';
			}
		      fldfmt[fmtlen++] = 'd';
		      fldfmt[fmtlen] = 0;
		      outputValueWithFormat(v, fldfmt, info);
		    }
		}
		break;

		// is it the hour
	      case 'I':
		twelve = YES;
	      case 'k':
		if (twelve == NO)
		  hspc = YES;
	      case 'H':
		v = info->hd;
		if (twelve == YES)
		  {
		    if (info->hd == 12 || info->hd == 0)
		      {
			v = 12;
		      }
		    else
		      {
			v = v % 12;
		      }
		  }
		if (fmtlen == 1) // no format width specified; supply default
		  {
		    if (hspc == YES)
		      fldfmt[fmtlen++] = '2'; // ensure a leading space
		    else
		      {
			fldfmt[fmtlen++] = '0';
			fldfmt[fmtlen++] = '2';
		      }

		  }
		fldfmt[fmtlen++] = 'd';
		fldfmt[fmtlen] = 0;
		if (GSPrivateDefaultsFlag(GSMacOSXCompatible)
		    && hspc == YES)
		  {
		    Grow(info, 2);
		    info->t[info->offset++] = '%';
		    info->t[info->offset++] = f[i-1];
		    break;
		  }
		else
		  outputValueWithFormat(v, fldfmt, info);
		break;

		// is it the minute
	      case 'M':
		v = info->mnd;
		if (fmtlen == 1) // no format width specified; supply default
		  {
		    fldfmt[fmtlen++] = '0';
		    fldfmt[fmtlen++] = '2';
		  }
		fldfmt[fmtlen++] = 'd';
		fldfmt[fmtlen] = 0;
		outputValueWithFormat(v, fldfmt, info);
		break;

		// is it the second
	      case 'S':
		v = info->sd;
		if (fmtlen == 1) // no format width specified; supply default
		  {
		    fldfmt[fmtlen++] = '0';
		    fldfmt[fmtlen++] = '2';
		  }
		fldfmt[fmtlen++] = 'd';
		fldfmt[fmtlen] = 0;
		outputValueWithFormat(v, fldfmt, info);
		break;

		// Is it the am/pm indicator
	      case 'p':
		{
		  NSArray	*a = [LOCALE objectForKey: NSAMPMDesignation];
		  NSString	*ampm;

		  if (info->hd >= 12)
		    {
		      if ([a count] > 1)
			ampm = [a objectAtIndex: 1];
		      else
			ampm = @"pm";
		    }
		  else
		    {
		      if ([a count] > 0)
			ampm = [a objectAtIndex: 0];
		      else
			ampm = @"am";
		    }
		  v = [ampm length];
		  Grow(info, v);
		  [ampm getCharacters: info->t + info->offset];
		  info->offset += v;
		}
		break;

		// is it the zone name
	      case 'Z':
		{
		  NSString	*s;

		  s = abbrev(_time_zone, self);
		  v = [s length];
		  Grow(info, v);
		  [s getCharacters: info->t + info->offset];
		  info->offset += v;
		}
		break;

	      case 'z':
		{
		  int	z;

		  Grow(info, 5);
		  z = offset(_time_zone, self);
		  if (z < 0)
		    {
		      z = -z;
		      info->t[info->offset++] = '-';
		    }
		  else
		    {
		      info->t[info->offset++] = '+';
		    }
		  z /= 60;	// Convert seconds to minutes.
		  v = z / 60;
		  info->t[info->offset+1] = (v%10) + '0';
		  v /= 10;
		  info->t[info->offset+0] = (v%10) + '0';
		  info->offset += 2;
		  v = z % 60;
		  info->t[info->offset+1] = (v%10) + '0';
		  v /= 10;
		  info->t[info->offset+0] = (v%10) + '0';
		  info->offset += 2;
		}
		break;

		// Anything else is unknown so just copy
	      default:
		Grow(info, 2);
		info->t[info->offset++] = '%';
		info->t[info->offset++] = f[i-1];
		break;
	    }
	}
      else
	{
	  Grow(info, 1);
	  info->t[info->offset++] = f[i++];
	}
    }

  if (f != fbuf)
    {
      NSZoneFree(NSDefaultMallocZone(), f);
    }
}

/**
 * Returns a string representation of the receiver using the specified
 * format string and locale dictionary.<br />
 * Format specifiers are -
 * <list>
 *   <item>
 *     %a   abbreviated weekday name according to locale
 *   </item>
 *   <item>
 *     %A   full weekday name according to locale
 *   </item>
 *   <item>
 *     %b   abbreviated month name according to locale
 *   </item>
 *   <item>
 *     %c   this is the same as %X %x
 *   </item>
 *   <item>
 *     %B   full month name according to locale
 *   </item>
 *   <item>
 *     %d   day of month as two digit decimal number (leading zero)
 *   </item>
 *   <item>
 *     %e   day of month as decimal number (without leading zero)
 *   </item>
 *   <item>
 *     %F   milliseconds (000 to 999)
 *   </item>
 *   <item>
 *     %H   hour as a decimal number using 24-hour clock
 *   </item>
 *   <item>
 *     %I   hour as a decimal number using 12-hour clock
 *   </item>
 *   <item>
 *     %j   day of year as a decimal number
 *   </item>
 *   <item>
 *     %k   same as %H with leading space instead of zero
 *   </item>
 *   <item>
 *     %m   month as decimal number
 *   </item>
 *   <item>
 *     %M   minute as decimal number
 *   </item>
 *   <item>
 *     %p   'am' or 'pm'
 *   </item>
 *   <item>
 *     %S   second as decimal number
 *   </item>
 *   <item>
 *     %U   week of the current year as decimal number (Sunday first day)
 *   </item>
 *   <item>
 *     %W   week of the current year as decimal number (Monday first day)
 *   </item>
 *   <item>
 *     %w   day of the week as decimal number (Sunday = 0)
 *   </item>
 *   <item>
 *     %x   date formatted according to the locale
 *   </item>
 *   <item>
 *     %X   time formatted according to the locale
 *   </item>
 *   <item>
 *     %y   year as a decimal number without century (minimum 0)
 *   </item>
 *   <item>
 *     %Y   year as a decimal number with century, minimum 0, maximum 9999
 *   </item>
 *   <item>
 *     %z   time zone offset (HHMM)
 *   </item>
 *   <item>
 *     %Z   time zone
 *   </item>
 *   <item>
 *     %%   literal % character
 *   </item>
 * </list>
 *
 * <p>NB.  If GSMacOSCompatible is set to YES, the %k specifier is not
 * recognized.</p>
 */
- (NSString*) descriptionWithCalendarFormat: (NSString*)format
				     locale: (NSDictionary*)locale
{
  unichar		tbuf[512];
  NSString		*result;
  DescriptionInfo	info;

  if (format == nil)
    format = [LOCALE objectForKey: NSTimeDateFormatString];

  GSBreakTime(_seconds_since_ref + offset(_time_zone, self),
    &info.yd, &info.md, &info.dom, &info.hd, &info.mnd, &info.sd, &info.mil);

  info.base = tbuf;
  info.t = tbuf;
  info.length = sizeof(tbuf)/sizeof(unichar);
  info.offset = 0;

  [self _format: format locale: locale info: &info];

  result = [NSString stringWithCharacters: info.t length: info.offset];

  if (info.t != tbuf)
    {
      NSZoneFree(NSDefaultMallocZone(), info.t);
    }

  return result;
}

- (id) copyWithZone: (NSZone*)zone
{
  NSCalendarDate	*newDate;

  if (NSShouldRetainWithZone(self, zone))
    {
      newDate = RETAIN(self);
    }
  else
    {
      newDate = (NSCalendarDate*)NSCopyObject(self, 0, zone);

      if (newDate != nil)
	{
	  if (_calendar_format != cformat)
	    {
	      newDate->_calendar_format = [_calendar_format copyWithZone: zone];
	    }
	  if (_time_zone != localTZ)
	    {
	      newDate->_time_zone = RETAIN(_time_zone);
	    }
	}
    }
  return newDate;
}

/**
 * Returns a description of the receiver using its normal format but with
 * the specified locale dictionary.<br />
 * Calls -descriptionWithCalendarFormat:locale: to do this.
 */
- (NSString*) descriptionWithLocale: (id)locale
{
  return [self descriptionWithCalendarFormat: _calendar_format locale: locale];
}

/**
 * Returns the format string associated with the receiver.<br />
 * See -descriptionWithCalendarFormat:locale: for details.
 */
- (NSString*) calendarFormat
{
  return _calendar_format;
}

/**
 * Sets the format string associated with the receiver.<br />
 * Providing a nil argument sets the default calendar format.<br />
 * See -descriptionWithCalendarFormat:locale: for details.
 */
- (void) setCalendarFormat: (NSString *)format
{
  if (format == nil)
    {
      format = cformat;
    }
  ASSIGNCOPY(_calendar_format, format);
}

/**
 * Sets the time zone associated with the receiver.<br />
 * Providing a nil argument sets the local time zone.
 */
- (void) setTimeZone: (NSTimeZone *)aTimeZone
{
  if (aTimeZone == nil)
    {
      aTimeZone = localTZ;
    }
  ASSIGN(_time_zone, aTimeZone);
}

/**
 * Returns the time zone associated with the receiver.
 */
- (NSTimeZone*) timeZone
{
  return _time_zone;
}

/**
 * Returns the time zone detail associated with the receiver.
 */
- (NSTimeZoneDetail*) timeZoneDetail
{
  NSTimeZoneDetail	*detail = [_time_zone timeZoneDetailForDate: self];
  return detail;
}

@end

/**
 * Routines for manipulating Gregorian dates.
 */
// The following code is based upon the source code in
// ``Calendrical Calculations'' by Nachum Dershowitz and Edward M. Reingold,
// Software---Practice & Experience, vol. 20, no. 9 (September, 1990),
// pp. 899--928.

@implementation NSCalendarDate (GregorianDate)

/**
 * Returns the number of the last day of the month in the specified year.
 */
- (NSInteger) lastDayOfGregorianMonth: (NSInteger)month year: (NSInteger)year
{
  return lastDayOfGregorianMonth(month, year);
}

/**
 * Returns the number of days since the start of the era for the specified
 * day, month, and year.
 */
- (NSInteger) absoluteGregorianDay: (NSInteger)day
			     month: (NSInteger)month
			      year: (NSInteger)year
{
  return absoluteGregorianDay(day, month, year);
}

/**
 * Given a day number since the start of the era, returns the date as a
 * day, month, and year.
 */
- (void) gregorianDateFromAbsolute: (NSInteger)d
			       day: (NSInteger *)day
			     month: (NSInteger *)month
			      year: (NSInteger *)year
{
  gregorianDateFromAbsolute(d, day, month, year);
}

@end


/**
 * Methods present in OpenStep but later removed from MacOS-X.
 */
@implementation NSCalendarDate (OPENSTEP)

- (NSCalendarDate*) dateByAddingYears: (NSInteger)years
			       months: (NSInteger)months
				 days: (NSInteger)days
			        hours: (NSInteger)hours
			      minutes: (NSInteger)minutes
			      seconds: (NSInteger)seconds
{
  NSCalendarDate	*c;
  NSTimeInterval	s;
  NSTimeInterval	oldOffset;
  NSTimeInterval	newOffset;
  NSInteger		i, year, month, day, hour, minute, second, mil;

  /* Apply timezone offset to _seconds_since_ref from GMT to local time,
   * then break into components in local time zone.
   */
  oldOffset = offset(_time_zone, self);
  s = _seconds_since_ref + oldOffset;
  GSBreakTime(s, &year, &month, &day, &hour, &minute, &second, &mil);

  /* Apply required offsets to get new local time.
   */
  while (years != 0 || months != 0 || days != 0
    || hours != 0 || minutes != 0 || seconds != 0)
    {
      year += years;
      years = 0;

      month += months;
      months = 0;
      while (month > 12)
	{
	  year++;
	  month -= 12;
	}
      while (month < 1)
	{
	  year--;
	  month += 12;
	}

      day += days;
      days = 0;
      if (day > 28)
	{
	  i = lastDayOfGregorianMonth(month, year);
	  while (day > i)
	    {
	      day -= i;
	      if (month < 12)
		{
		  month++;
		}
	      else
		{
		  month = 1;
		  year++;
		}
	      i = lastDayOfGregorianMonth(month, year);
	    }
	}
      else
	{
	  while (day < 1)
	    {
	      if (month == 1)
		{
		  year--;
		  month = 12;
		}
	      else
		{
		  month--;
		}
	      day += lastDayOfGregorianMonth(month, year);
	    }
	}

      hour += hours;
      hours = 0;
      days += hour/24;
      hour %= 24;
      if (hour < 0)
	{
	  days--;
	  hour += 24;
	}

      minute += minutes;
      minutes = 0;
      hours += minute/60;
      minute %= 60;
      if (minute < 0)
	{
	  hours--;
	  minute += 60;
	}

      second += seconds;
      seconds = 0;
      minutes += second/60;
      second %= 60;
      if (second < 0)
	{
	  minutes--;
	  second += 60;
	}
    }

  /*
   * Reassemble and apply original timezone offset to get
   * _seconds_since_ref back to GMT.
   */
  s = GSTime(day, month, year, hour, minute, second, mil);
  s -= oldOffset;
  c = [NSCalendarDateClass alloc];
  c->_calendar_format = [_calendar_format copy];
  c->_time_zone = [_time_zone copy];
  c->_seconds_since_ref = s;

  /*
   * Adjust date to try to maintain the time of day over
   * a daylight savings time boundary if necessary.
   */
  newOffset = offset(_time_zone, c);
  if (newOffset != oldOffset)
    {
      NSTimeInterval	tmpOffset = newOffset;

      s -= (newOffset - oldOffset);
      c->_seconds_since_ref = s;
      /*
       * If the date we have lies within a missing hour at a
       * daylight savings time transition, we use the original
       * date rather than the adjusted one.
       */
      newOffset = offset(_time_zone, c);
      if (newOffset == oldOffset)
	{
	  s += (tmpOffset - oldOffset);
	  c->_seconds_since_ref = s;
	}
    }
  return AUTORELEASE(c);
}

/**
 * Returns the number of years, months, days, hours, minutes, and seconds
 * between the receiver and the given date.<br />
 * If date is in the future of the receiver, the returned values will
 * be negative (or zero), otherwise they are all positive.<br />
 * If any of the pointers to return value in is null, the corresponding
 * value will not be returned, and other return values will be adjusted
 * accordingly. eg. If a difference of 1 hour was to be returned but
 * hours is null, then the value returned in minutes will be increased
 * by 60.
 */
- (void) years: (NSInteger*)years
	months: (NSInteger*)months
          days: (NSInteger*)days
         hours: (NSInteger*)hours
       minutes: (NSInteger*)minutes
       seconds: (NSInteger*)seconds
     sinceDate: (NSDate*)date
{
  NSCalendarDate	*start;
  NSCalendarDate	*end;
  NSCalendarDate	*tmp;
  int			diff;
  int			extra;
  int			sign;
  NSInteger		mil;
  NSInteger		syear, smonth, sday, shour, sminute, ssecond;
  NSInteger		eyear, emonth, eday, ehour, eminute, esecond;

  /* FIXME What if the two dates are in different time zones?
    How about daylight savings time?
   */
  if ([date isKindOfClass: NSCalendarDateClass])
    {
      tmp = (NSCalendarDate*)RETAIN(date);
    }
  else if ([date isKindOfClass: [NSDate class]])
    {
      tmp = [[NSCalendarDateClass alloc] initWithTimeIntervalSinceReferenceDate:
	[date timeIntervalSinceReferenceDate]];
    }
  else
    {
      tmp = nil;	// Avoid compiler warning
      [NSException raise: NSInvalidArgumentException
	format: @"%@ invalid date given - %@",
	NSStringFromSelector(_cmd), date];
    }

  end = (NSCalendarDate*)[self laterDate: tmp];
  if (end == self)
    {
      start = tmp;
      sign = 1;
    }
  else
    {
      start = self;
      sign = -1;
    }

  GSBreakTime(start->_seconds_since_ref + offset(start->_time_zone, start),
    &syear, &smonth, &sday, &shour, &sminute, &ssecond, &mil);

  GSBreakTime(end->_seconds_since_ref + offset(end->_time_zone, end),
    &eyear, &emonth, &eday, &ehour, &eminute, &esecond, &mil);

  if (esecond < ssecond)
    {
      eminute -= 1;
      esecond += 60;
    }
  if (eminute < sminute)
    {
      ehour -= 1;
      eminute += 60;
    }
  if (ehour < shour)
    {
      eday -= 1;
      ehour += 24;
    }
  if (eday < sday)
    {
      emonth -= 1;
      if (emonth >= 0)
	{
	  eday += [end lastDayOfGregorianMonth: emonth year: eyear];
	}
      else
	{
	  eday += 31;
	}
    }
  if (emonth < smonth || (emonth == smonth && eday < sday))
    {
      eyear -= 1;
      emonth += 12;
    }

  /* Calculate year difference and leave any remaining months in 'extra' */
  diff = eyear - syear;
  extra = 0;
  if (years != 0)
    {
      *years = sign*diff;
    }
  else
    {
      extra += diff*12;
    }

  /* Calculate month difference and leave any remaining days in 'extra' */
  diff = emonth - smonth + extra;
  extra = 0;
  if (months != 0)
    {
      *months = sign*diff;
    }
  else
    {
      while (diff-- > 0)
	{
	  int tmpmonth = emonth - diff - 1;
	  int tmpyear = eyear;

          while (tmpmonth < 1)
	    {
	      tmpmonth += 12;
	      tmpyear--;
	    }
          extra += lastDayOfGregorianMonth(tmpmonth, tmpyear);
        }
    }

  /* Calculate day difference and leave any remaining hours in 'extra' */
  diff = eday - sday + extra;
  extra = 0;
  if (days != 0)
    {
      *days = sign*diff;
    }
  else
    {
      extra += diff*24;
    }

  /* Calculate hour difference and leave any remaining minutes in 'extra' */
  diff = ehour - shour + extra;
  extra = 0;
  if (hours != 0)
    {
      *hours = sign*diff;
    }
  else
    {
      extra += diff*60;
    }

  /* Calculate minute difference and leave any remaining seconds in 'extra' */
  diff = eminute - sminute + extra;
  extra = 0;
  if (minutes != 0)
    {
      *minutes = sign*diff;
    }
  else
    {
      extra += diff*60;
    }

  diff = esecond - ssecond + extra;
  if (seconds != 0)
    {
      *seconds = sign*diff;
    }

  RELEASE(tmp);
}

@end
