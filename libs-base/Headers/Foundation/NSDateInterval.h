/* Definition of class NSDateInterval
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Wed Oct  9 16:24:13 EDT 2019

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

#ifndef _NSDateInterval_h_GNUSTEP_BASE_INCLUDE
#define _NSDateInterval_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSObject.h>
#include <Foundation/NSDate.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_12, GS_API_LATEST)

GS_EXPORT_CLASS
@interface NSDateInterval : NSObject <NSCoding, NSCopying>
{
  NSTimeInterval _duration;
  NSDate *_startDate;
}
  
// Init
- (instancetype) init;

- (instancetype) initWithStartDate: (NSDate *)startDate 
                          duration: (NSTimeInterval)duration;

- (instancetype) initWithStartDate: (NSDate *)startDate
                           endDate: (NSDate *)endDate;

// Access
- (NSDate *) startDate;
- (void) setStartDate: (NSDate *)startDate;

- (NSDate *) endDate;
- (void) setEndDate: (NSDate *)endDate;

- (NSTimeInterval)duration;
- (void) setDuration: (NSTimeInterval)duration;

// Compare
- (NSComparisonResult) compare: (NSDateInterval *)dateInterval;

- (BOOL) isEqualToDateInterval: (NSDateInterval *)dateInterval;

// Determine
- (BOOL) intersectsDateInterval: (NSDateInterval *)dateInterval;

- (NSDateInterval *) intersectionWithDateInterval: (NSDateInterval *)dateInterval;

// Contain
- (BOOL) containsDate: (NSDate *)date;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSDateInterval_h_GNUSTEP_BASE_INCLUDE */

