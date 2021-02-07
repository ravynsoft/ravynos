/* Implementation of class NSDateIntervalFormatter
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento <greg.casamento@gmail.com>
   Date: Wed Oct  9 16:23:55 EDT 2019

   This file is part of the GNUstep Library.
   
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

#include <Foundation/NSDateIntervalFormatter.h>
#include <Foundation/NSLocale.h>
#include <Foundation/NSCalendar.h>
#include <Foundation/NSTimeZone.h>
#include <Foundation/NSString.h>
#include <Foundation/NSDate.h>
#include <Foundation/NSDateInterval.h>

@implementation NSDateIntervalFormatter
// Properties
- (NSLocale *) locale
{
  return _locale;
}

- (void) setLocale: (NSLocale *)locale
{
  ASSIGNCOPY(_locale, locale);
}

- (NSCalendar *) calendar
{
  return _calendar;
}

- (void) setCalendar: (NSCalendar *)calendar
{
  ASSIGNCOPY(_calendar, calendar);
}

- (NSTimeZone *) timeZone
{
  return _timeZone;
}

- (void) setTimeZone: (NSTimeZone *)timeZone
{
  ASSIGNCOPY(_timeZone, timeZone);
}

- (NSString *) dateTemplate
{
  return _dateTemplate;
}

- (void) setDateTemplate: (NSString *)dateTemplate
{
  ASSIGNCOPY(_dateTemplate, dateTemplate);
}

- (NSDateIntervalFormatterStyle) dateStyle
{
  return _dateStyle;
}

- (void) setDateStyle: (NSDateIntervalFormatterStyle)dateStyle
{
  _dateStyle = dateStyle;
}
  
- (NSDateIntervalFormatterStyle) timeStyle
{
  return _timeStyle;
}

- (void) setTimeStyle: (NSDateIntervalFormatterStyle)timeStyle
{
  _timeStyle = timeStyle;
}

// Create strings
- (NSString *) stringFromDate: (NSDate *)fromDate toDate: (NSDate *)toDate
{
  NSDateInterval *interval = [[NSDateInterval alloc] initWithStartDate: fromDate
                                                               endDate: toDate];
  AUTORELEASE(interval);
  return [self stringFromDateInterval: interval];
}

- (NSString *) stringFromDateInterval: (NSDateInterval *)dateInterval
{
  NSDate *fromDate = [dateInterval startDate];
  NSDate *toDate = [dateInterval endDate];

  // Add formatting of NSDate here.
  
  return [NSString stringWithFormat: @"%@ - %@", fromDate, toDate];
}

@end

