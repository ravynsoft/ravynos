/* Definition of class NSISO8601DateFormatter
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento <greg.casamento@gmail.com>
   Date: Tue Oct 29 04:43:13 EDT 2019

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

#ifndef _NSISO8601DateFormatter_h_GNUSTEP_BASE_INCLUDE
#define _NSISO8601DateFormatter_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSFormatter.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif
  
enum
{
  NSISO8601DateFormatWithYear = (1UL << 0),
  NSISO8601DateFormatWithMonth  = (1UL << 1),
  NSISO8601DateFormatWithWeekOfYear = (1UL << 2),
  NSISO8601DateFormatWithDay  = (1UL << 4),
  NSISO8601DateFormatWithTime  = (1UL << 5),
  NSISO8601DateFormatWithTimeZone  = (1UL << 6),
  NSISO8601DateFormatWithSpaceBetweenDateAndTime = (1UL << 7), 
  NSISO8601DateFormatWithDashSeparatorInDate  = (1UL << 8),
  NSISO8601DateFormatWithColonSeparatorInTime   = (1UL << 9),
  NSISO8601DateFormatWithColonSeparatorInTimeZone = (1UL << 10), 
  NSISO8601DateFormatWithFractionalSeconds  = (1UL << 11),
  NSISO8601DateFormatWithFullDate = NSISO8601DateFormatWithYear |
                                    NSISO8601DateFormatWithMonth |
                                    NSISO8601DateFormatWithDay |
                                    NSISO8601DateFormatWithDashSeparatorInDate,
  NSISO8601DateFormatWithFullTime = NSISO8601DateFormatWithTime |
                                    NSISO8601DateFormatWithColonSeparatorInTime |
                                    NSISO8601DateFormatWithTimeZone |
                                    NSISO8601DateFormatWithColonSeparatorInTimeZone,
  NSISO8601DateFormatWithInternetDateTime = (NSISO8601DateFormatWithFullDate | NSISO8601DateFormatWithFullTime),
};
typedef NSUInteger NSISO8601DateFormatOptions;

@class NSTimeZone, NSString, NSDate, NSDateFormatter;

GS_EXPORT_CLASS
@interface NSISO8601DateFormatter : NSFormatter <NSCoding>
{
  NSTimeZone *_timeZone;
  NSISO8601DateFormatOptions _formatOptions;
  NSDateFormatter *_formatter; 
}
  
- (NSTimeZone *) timeZone;
- (void) setTimeZone: (NSTimeZone *)tz;

- (NSISO8601DateFormatOptions) formatOptions;
- (void) setFormatOptions: (NSISO8601DateFormatOptions)options;
  
- (NSString *) stringFromDate: (NSDate *)date;
- (NSDate *) dateFromString: (NSString *)string;

+ (NSString *) stringFromDate: (NSDate *)date
                     timeZone: (NSTimeZone *)timeZone
                formatOptions: (NSISO8601DateFormatOptions)formatOptions;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSISO8601DateFormatter_h_GNUSTEP_BASE_INCLUDE */

