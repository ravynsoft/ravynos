/** GSLocale - various functions for localization

   Copyright (C) 2000 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Created: Oct 2000

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
*/
#import "common.h"
#import "GNUstepBase/GSLocale.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSLock.h"

static NSString *
privateSetLocale(int category, NSString *locale);

const char*
GSSetLocaleC(int category, const char *loc)
{
  NSWarnLog(@"GSSetLocaleC is deprecated and has no effect");
  return NULL;
}

NSString *
GSSetLocale(int category, NSString *locale)
{
  NSWarnLog(@"GSSetLocale is deprecated and has no effect");
  return nil;
}

#if defined(HAVE_LOCALE_H) && defined(HAVE_CURRENCY_SYMBOL_IN_LCONV)
/* There is little point in using locale.h if no useful information
   is exposed through struct lconv. An example platform is Android. */

#include <locale.h>
#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif
#import "Foundation/NSUserDefaults.h"

#import "GSPrivate.h"

#define ToString(value) [NSString stringWithCString: (value) \
encoding: GSPrivateNativeCStringEncoding()]

static NSString *
privateSetLocale(int category, NSString *locale)
{
  const char *clocale = NULL;
  /* Need to get the encoding first as the function call invalidates 
   * the return value of setlocale()
   */
  NSStringEncoding enc = GSPrivateNativeCStringEncoding();
  if (locale != nil)
    {
      clocale = [locale cString];
    }
  clocale = setlocale(category, clocale);

  if (clocale != NULL)
    {
      return [NSString stringWithCString: clocale encoding: enc];
    }
  return nil;
}

#define GSLanginfo(value) ToString(nl_langinfo (value))


/* Creates a locale dictionary from information provided by i18n functions.
   Many, but not all, of the keys are filled in or inferred from the
   available information */
NSDictionary *
GSDomainFromDefaultLocale(void)
{
  static NSDictionary	*saved = nil;
  struct lconv		*lconv;
  NSMutableDictionary	*dict;
  NSString		*str1;
  NSString		*str2;
#ifdef HAVE_LANGINFO_H
  int			i;
  NSMutableArray	*arr;
#endif
  NSString              *backupLocale;

  if (saved != nil)
    return saved;

  dict = [NSMutableDictionary dictionary];

  /* Protect locale access with locks to prevent multiple threads using
   * it and interfering with the buffer.
   */
  [gnustep_global_lock lock];

  /**
   * Set the current locale to the system default, and backup
   * what it was previously (should have been @"C").
   */
  backupLocale = privateSetLocale(LC_ALL, nil);
  privateSetLocale(LC_ALL, @"");

#ifdef HAVE_LANGINFO_H
  /* Time/Date Information */
  arr = [NSMutableArray arrayWithCapacity: 7];
  for (i = 0; i < 7; i++)
    {
      [arr addObject: GSLanginfo(DAY_1+i)];
    }
  [dict setObject: arr forKey: NSWeekDayNameArray];

  arr = [NSMutableArray arrayWithCapacity: 7];
  for (i = 0; i < 7; i++)
    {
      [arr addObject: GSLanginfo(ABDAY_1+i)];
    }
  [dict setObject: arr forKey: NSShortWeekDayNameArray];

  arr = [NSMutableArray arrayWithCapacity: 12];
  for (i = 0; i < 12; i++)
    {
      [arr addObject: GSLanginfo(MON_1+i)];
    }
  [dict setObject: arr forKey: NSMonthNameArray];

  arr = [NSMutableArray arrayWithCapacity: 12];
  for (i = 0; i < 12; i++)
    {
      [arr addObject: GSLanginfo(ABMON_1+i)];
    }
  [dict setObject: arr forKey: NSShortMonthNameArray];

  str1 = GSLanginfo(AM_STR);
  str2 = GSLanginfo(PM_STR);
  if (str1 != nil && str2 != nil)
    {
      [dict setObject: [NSArray arrayWithObjects: str1, str2, nil]
	       forKey: NSAMPMDesignation];
    }

  [dict setObject: GSLanginfo(D_T_FMT)
	   forKey: NSTimeDateFormatString];
  [dict setObject: GSLanginfo(D_FMT)
	   forKey: NSShortDateFormatString];
  [dict setObject: GSLanginfo(T_FMT)
	   forKey: NSTimeFormatString];
#endif /* HAVE_LANGINFO_H */

  lconv = localeconv();

  /* Currency Information */
  if (lconv->currency_symbol)
    {
      [dict setObject: ToString(lconv->currency_symbol)
	       forKey: NSCurrencySymbol];
    }
  if (lconv->int_curr_symbol)
    {
      [dict setObject: ToString(lconv->int_curr_symbol)
	       forKey: NSInternationalCurrencyString];
    }
  if (lconv->mon_decimal_point)
    {
      [dict setObject: ToString(lconv->mon_decimal_point)
	       forKey: NSInternationalCurrencyString];
    }
  if (lconv->mon_thousands_sep)
    {
      [dict setObject: ToString(lconv->mon_thousands_sep)
	       forKey: NSInternationalCurrencyString];
    }

  if (lconv->decimal_point)
    {
      [dict setObject: ToString(lconv->decimal_point)
	       forKey: NSDecimalSeparator];
    }
  if (lconv->thousands_sep)
    {
      [dict setObject: ToString(lconv->thousands_sep)
	       forKey: NSThousandsSeparator];
    }

  /* FIXME: Get currency format from localeconv */

#ifdef	LC_MESSAGES
  str1 = privateSetLocale(LC_MESSAGES, nil);
#else
  str1 = nil;
#endif
  if (str1 != nil)
    {
      [dict setObject: str1 forKey: GSLocale];
    }
  str2 = GSLanguageFromLocale(str1);
  if (str2 != nil)
    {
      [dict setObject: str2 forKey: NSLanguageName];
    }

  /*
   * Another thread might have been faster in setting the static variable.
   * If so, we just drop our dict.
   */
  if (saved == nil)
    {
      saved = [NSObject leak: dict];
    }

  /**
   * Restore the current locale to what we backed up (again, should
   * be restored to @"C")
   */
  privateSetLocale(LC_ALL, backupLocale);

  [gnustep_global_lock unlock];
  return saved;
}

#else /* HAVE_LOCALE_H */
static NSString *
privateSetLocale(int category, NSString *locale)
{
  return nil;
}

NSDictionary *
GSDomainFromDefaultLocale(void)
{
  return nil;
}

#endif /* !HAVE_LOCALE_H */

NSString *
GSLanguageFromLocale(NSString *locale)
{
  NSString	*language = nil;
  NSString	*aliases = nil;
  NSBundle      *gbundle;

  if (locale == nil || [locale isEqual: @"C"] || [locale isEqual: @"POSIX"]
      || [locale length] < 2)
    return @"English";

  gbundle = [NSBundle bundleForLibrary: @"gnustep-base"];
  aliases = [gbundle pathForResource: @"Locale"
		              ofType: @"aliases"
		         inDirectory: @"Languages"];
  if (aliases != nil)
    {
      NSDictionary	*dict;

      dict = [NSDictionary dictionaryWithContentsOfFile: aliases];
      language = [dict objectForKey: locale];
      if (language == nil && [locale pathExtension] != nil)
	{
	  locale = [locale stringByDeletingPathExtension];
          if ([locale isEqual: @"C"] || [locale isEqual: @"POSIX"])
            return @"English";
	  language = [dict objectForKey: locale];
	}
      if (language == nil)
	{
	  locale = [locale substringWithRange: NSMakeRange(0, 2)];
	  language = [dict objectForKey: locale];
	}
    }

  return language;
}

NSArray *
GSLocaleVariants(NSString *locale)
{
  NSRange under = [locale rangeOfString: @"_"];
  if (under.location != NSNotFound)
    {
      return [NSArray arrayWithObjects:
			locale,
		      [locale substringToIndex: under.location],
		      nil];
    }
  return [NSArray arrayWithObject: locale];
}

NSArray *
GSLanguagesFromLocale(NSString *locale)
{
  NSArray *variants = GSLocaleVariants(locale);
  NSMutableArray *result = [NSMutableArray arrayWithCapacity: [variants count]];

  NSEnumerator *enumerator = [variants objectEnumerator];
  NSString *variant;
  while ((variant = [enumerator nextObject]) != nil)
    {
      NSString *language = GSLanguageFromLocale(variant);
      if (language != nil)
	{
	  [result addObject: language];
	}
    }
  return result;
}

NSString *GSDefaultLanguageLocale()
{
  NSString *locale = nil;

#ifdef HAVE_LOCALE_H
#ifdef LC_MESSAGES
  NSString *backup;

  [gnustep_global_lock lock];

  backup = privateSetLocale(LC_ALL, nil);
  privateSetLocale(LC_ALL, @"");
  locale = privateSetLocale(LC_MESSAGES, nil);  
  privateSetLocale(LC_ALL, backup);

  [gnustep_global_lock unlock];
#endif
#endif

  return locale;  
}
