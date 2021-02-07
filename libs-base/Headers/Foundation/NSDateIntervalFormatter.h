
/* Definition of class NSDateIntervalFormatter
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: heron
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

#ifndef _NSDateIntervalFormatter_h_GNUSTEP_BASE_INCLUDE
#define _NSDateIntervalFormatter_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSFormatter.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

enum {
  NSDateIntervalFormatterNoStyle = 0,
  NSDateIntervalFormatterShortStyle = 1,
  NSDateIntervalFormatterMediumStyle = 2,
  NSDateIntervalFormatterLongStyle = 3,
  NSDateIntervalFormatterFullStyle = 4
};
typedef NSUInteger NSDateIntervalFormatterStyle;

@class NSCalendar, NSLocale, NSDateInterval;

GS_EXPORT_CLASS
@interface NSDateIntervalFormatter : NSFormatter
{
    NSLocale *_locale;
    NSCalendar *_calendar;
    NSTimeZone *_timeZone;
    NSString *_dateTemplate;
    NSDateIntervalFormatterStyle _dateStyle;
    NSDateIntervalFormatterStyle _timeStyle;
}

// Properties
- (NSLocale *) locale;
- (void) setLocale: (NSLocale *)locale;

- (NSCalendar *) calendar;
- (void) setCalendar: (NSCalendar *)calendar;

- (NSTimeZone *) timeZone;
- (void) setTimeZone: (NSTimeZone *)timeZone;

- (NSString *) dateTemplate;
- (void) setDateTemplate: (NSString *)dateTemplate;

- (NSDateIntervalFormatterStyle) dateStyle;
- (void) setDateStyle: (NSDateIntervalFormatterStyle)dateStyle;
  
- (NSDateIntervalFormatterStyle) timeStyle;
- (void) setTimeStyle: (NSDateIntervalFormatterStyle)timeStyle;

// Create strings
- (NSString *)stringFromDate:(NSDate *)fromDate toDate:(NSDate *)toDate;

- (NSString *)stringFromDateInterval:(NSDateInterval *)dateInterval;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSDateIntervalFormatter_h_GNUSTEP_BASE_INCLUDE */

