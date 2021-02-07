/* Implementation of extension methods to base additions

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

*/
#import "common.h"
#import "Foundation/NSAutoreleasePool.h"
#import "GNUstepBase/NSCalendarDate+GNUstepBase.h"

/**
 * Extension methods for the NSCalendarDate class
 */
@implementation NSCalendarDate (GNUstepBase)

- (NSUInteger) isoYear
{
  NSUInteger year = [self yearOfCommonEra];
  NSUInteger week = [self weekOfYear];
  NSUInteger month = [self monthOfYear];

  if (week == 1 && month == 12)
    {
      year++;
    }
  else if (week >= 52 && month == 1)
    {
      year--;
    }
  return year;
}

- (NSInteger) weekOfYear
{
  NSInteger	dayOfWeek = [self dayOfWeek];
  NSInteger	dayOfYear;

  /*
   * Whether a week is considered to be in a year or not depends on its
   * thursday ... so find thursday for the receivers week.
   * NB. this may result in a date which is not in the same year as the
   * receiver.
   */
  if (dayOfWeek != 4)
    {
      NSAutoreleasePool	*arp = [NSAutoreleasePool new];
      NSCalendarDate	*thursday;

      /*
       * A week starts on monday ... so adjust from 0 to 7 so that a
       * sunday is counted as the last day of the week.
       */
      if (dayOfWeek == 0)
	{
	  dayOfWeek = 7;
	}
      thursday = [self dateByAddingYears: 0
				  months: 0
				    days: 4 - dayOfWeek
				   hours: 0
				 minutes: 0
				 seconds: 0];
      dayOfYear = [thursday dayOfYear];
      [arp drain];
    }
  else
    {
      dayOfYear = [self dayOfYear];
    }

  /*
   * Round up to a week boundary, so that when we divide by seven we
   * get a result in the range 1 to 53 as mandated by the ISO standard.
   * Note that dayOfYear starts at 1, too, and hence we must be careful
   * to not round up an exact multiple of 7.
   */
  dayOfYear += (7 - (dayOfYear - 1) % 7);
  return dayOfYear / 7;
}

@end
