/* Definition of class NSDateComponentsFormatter
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Wed Nov  6 00:24:02 EST 2019

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

#ifndef _NSDateComponentsFormatter_h_GNUSTEP_BASE_INCLUDE
#define _NSDateComponentsFormatter_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSFormatter.h>
#include <Foundation/NSCalendar.h>

#if OS_API_VERSION(MAC_OS_X_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

enum
{
  // "1:10; may fall back to abbreviated units in some cases (such as 3d)"
  NSDateComponentsFormatterUnitsStylePositional = 0,
  // "1h 10m"
  NSDateComponentsFormatterUnitsStyleAbbreviated, 
  // "1hr, 10min"
  NSDateComponentsFormatterUnitsStyleShort,
  // "1 hour, 10 minutes"
  NSDateComponentsFormatterUnitsStyleFull, 
  // "One hour, ten minutes"
  NSDateComponentsFormatterUnitsStyleSpellOut,
  // "1hr 10min" - Brief is shorter than Short
  NSDateComponentsFormatterUnitsStyleBrief, 
};
typedef NSInteger NSDateComponentsFormatterUnitsStyle;
  
enum
{
  //drop none, pad none
  NSDateComponentsFormatterZeroFormattingBehaviorNone = (0),
  //Positional units: drop leading zeros, pad other zeros. All others: drop all zeros.
  NSDateComponentsFormatterZeroFormattingBehaviorDefault = (1 << 0), 
  // Off: "0h 10m", On: "10m"
  NSDateComponentsFormatterZeroFormattingBehaviorDropLeading = (1 << 1),
  // Off: "1h 0m 10s", On: "1h 10s"
  NSDateComponentsFormatterZeroFormattingBehaviorDropMiddle = (1 << 2),
  // Off: "1h 0m", On: "1h"
  NSDateComponentsFormatterZeroFormattingBehaviorDropTrailing = (1 << 3), 
  NSDateComponentsFormatterZeroFormattingBehaviorDropAll = (NSDateComponentsFormatterZeroFormattingBehaviorDropLeading |
                                                            NSDateComponentsFormatterZeroFormattingBehaviorDropMiddle |
                                                            NSDateComponentsFormatterZeroFormattingBehaviorDropTrailing),
  // Off: "1:0:10", On: "01:00:10"
  NSDateComponentsFormatterZeroFormattingBehaviorPad = (1 << 16), 
};
typedef NSUInteger NSDateComponentsFormatterZeroFormattingBehavior;

@class NSString, NSDate;

GS_EXPORT_CLASS
@interface NSDateComponentsFormatter : NSFormatter
{
  NSCalendar *_calendar;
  NSDate *_referenceDate;
  BOOL _allowsFractionalUnits;
  BOOL _collapsesLargestUnit;
  BOOL _includesApproximationPhrase;
  NSFormattingContext _formattingContext;
  NSInteger _maximumUnitCount;
  NSDateComponentsFormatterZeroFormattingBehavior _zeroFormattingBehavior;
  NSCalendarUnit _allowedUnits;
  NSDateComponentsFormatterUnitsStyle _unitsStyle;
}
  
- (NSString *) stringForObjectValue: (id)obj;

- (NSString *) stringFromDateComponents: (NSDateComponents *)components;

- (NSString *) stringFromDate: (NSDate *)startDate toDate: (NSDate *)endDate;

- (NSString *) stringFromTimeInterval: (NSTimeInterval)ti;

- (NSDateComponentsFormatterUnitsStyle) unitsStyle;
- (void) setUnitsStyle: (NSDateComponentsFormatterUnitsStyle)style;
  
- (NSCalendarUnit) allowedUnits;
- (void) setAllowedUnits: (NSCalendarUnit)units;

- (NSDateComponentsFormatterZeroFormattingBehavior) zeroFormattingBehavior;
- (void) setZeroFormattingBehavior: (NSDateComponentsFormatterZeroFormattingBehavior)behavior;

- (NSCalendar *) calendar;
- (void) setCalender: (NSCalendar *)calendar;

- (NSDate *) referenceDate;
- (void) setReferenceDate: (NSDate *)referenceDate;

- (BOOL) allowsFractionalUnits;
- (void) setAllowsFractionalUnits: (BOOL)allowsFractionalUnits;

- (NSInteger) maximumUnitCount;
- (void) setMaximumUnitCount: (NSInteger)maximumUnitCount;

- (BOOL) collapsesLargestUnit;
- (void) setCollapsesLargestUnit: (BOOL)collapsesLargestUnit;

- (BOOL) includesApproximationPhrase;
- (void) setIncludesApproximationPhrase: (BOOL)includesApproximationPhrase;

- (NSFormattingContext) formattingContext;
- (void) setFormattingContext: (NSFormattingContext)formattingContext;

- (BOOL) getObjectValue: (id*)obj forString: (NSString *)string errorDescription: (NSString **)error;

+ (NSString *) localizedStringFromDateComponents: (NSDateComponents *)components
                                      unitsStyle: (NSDateComponentsFormatterUnitsStyle)unitsStyle;
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSDateComponentsFormatter_h_GNUSTEP_BASE_INCLUDE */

