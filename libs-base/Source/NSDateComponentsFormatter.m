/* Implementation of class NSDateComponentsFormatter
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

#include <Foundation/NSDateComponentsFormatter.h>
#include <Foundation/NSDate.h>
#include <Foundation/NSString.h>
#include <Foundation/NSValue.h>
#include <Foundation/NSException.h>
#include <Foundation/NSNumberFormatter.h>

@implementation NSDateComponentsFormatter

- (instancetype) init
{
  self = [super init];
  if (self != nil)
    {
      _calendar = nil;
      _referenceDate = nil;
      _allowsFractionalUnits = NO;
      _collapsesLargestUnit = NO;
      _includesApproximationPhrase = NO;
      _formattingContext = NSFormattingContextUnknown;
      _maximumUnitCount = 0;
      _zeroFormattingBehavior
	= NSDateComponentsFormatterZeroFormattingBehaviorDefault;
      _allowedUnits = NSCalendarUnitYear
	| NSCalendarUnitMonth
	| NSCalendarUnitDay
	| NSCalendarUnitHour
	| NSCalendarUnitMinute
	| NSCalendarUnitSecond;
      _unitsStyle = NSDateComponentsFormatterUnitsStylePositional;
    }
  return self;
}

- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super initWithCoder: coder];
  if (self != nil)
    {
      // TODO: Implement coding...
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];
  // TODO: Implement coding...
}

- (void) dealloc
{
  RELEASE(_calendar);
  RELEASE(_referenceDate);
  [super dealloc];
}

- (NSString *) stringForObjectValue: (id)obj
{
  NSString *result = nil;
  
  if ([obj isKindOfClass: [NSDateComponents class]])
    {
      result = [self stringFromDateComponents: obj];
    }
  else if ([obj isKindOfClass: [NSNumber class]])
    {
      NSTimeInterval ti = [obj longLongValue];
      result = [self stringFromTimeInterval: ti];
    }
    
  return result;
}

- (NSString *) stringFromDateComponents: (NSDateComponents *)components
{
  NSString *result = @"";

  if (_allowedUnits & NSCalendarUnitYear)
    {
      if (_unitsStyle == NSDateComponentsFormatterUnitsStyleSpellOut)
        {
          NSNumberFormatter *fmt = [[NSNumberFormatter alloc] init];
          NSNumber *num = [NSNumber numberWithInteger: [components year]];
          AUTORELEASE(fmt);
          [fmt setNumberStyle: NSNumberFormatterSpellOutStyle];
          result
	    = [result stringByAppendingString: [fmt stringFromNumber: num]];
          result = [result stringByAppendingString: @" years"];  
        }
      else
        {
          if (_zeroFormattingBehavior
	    & NSDateComponentsFormatterZeroFormattingBehaviorDefault)
            {
              NSString *s;

              s = [NSString stringWithFormat: @"%4ld", [components year]];
              result = [result stringByAppendingString: s];
            }
          
          if (_unitsStyle == NSDateComponentsFormatterUnitsStylePositional)
            {
              result = [result stringByAppendingString: @" yr "];
            }
          else if (_unitsStyle
	    == NSDateComponentsFormatterUnitsStyleAbbreviated)
            {
              result = [result stringByAppendingString: @" yr "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleShort)
            {
              result = [result stringByAppendingString: @" yr "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleFull)
            {
              result = [result stringByAppendingString: @" years "];
            }
        }
    }
  if (_allowedUnits & NSCalendarUnitMonth)
    {
      if (_unitsStyle == NSDateComponentsFormatterUnitsStyleSpellOut)
        {
          NSNumberFormatter *fmt = [[NSNumberFormatter alloc] init];
          NSNumber *num = [NSNumber numberWithInteger: [components month]];
          AUTORELEASE(fmt);
          [fmt setNumberStyle: NSNumberFormatterSpellOutStyle];
          result
	    = [result stringByAppendingString: [fmt stringFromNumber: num]];
          result = [result stringByAppendingString: @" months "];  
        }
      else
        {
          if (_zeroFormattingBehavior
	    & NSDateComponentsFormatterZeroFormattingBehaviorDefault)
            {
              NSString *s;

              s = [NSString stringWithFormat: @"%2ld", [components month]];
              result = [result stringByAppendingString: s];
            }

          if (_unitsStyle == NSDateComponentsFormatterUnitsStylePositional)
            {
              result = [result stringByAppendingString: @" "];
            }
          else if (_unitsStyle
	    == NSDateComponentsFormatterUnitsStyleAbbreviated)
            {
              result = [result stringByAppendingString: @" mn "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleShort)
            {
              result = [result stringByAppendingString: @" mon "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleFull)
            {
              result = [result stringByAppendingString: @" months "];
            }
        }
    }
  if (_allowedUnits & NSCalendarUnitDay)
    {
      if (_unitsStyle == NSDateComponentsFormatterUnitsStyleSpellOut)
        {
          NSNumberFormatter *fmt = [[NSNumberFormatter alloc] init];
          NSNumber *num = [NSNumber numberWithInteger: [components day]];
          AUTORELEASE(fmt);
          [fmt setNumberStyle: NSNumberFormatterSpellOutStyle];
          result
	    = [result stringByAppendingString: [fmt stringFromNumber: num]];
          result = [result stringByAppendingString: @" days "];  
        }
      else
        {
          if (_zeroFormattingBehavior
	    & NSDateComponentsFormatterZeroFormattingBehaviorDefault)
            {
              NSString *s;

              s = [NSString stringWithFormat: @"%2ld", [components day]];
              result = [result stringByAppendingString: s];
            }

          if (_unitsStyle == NSDateComponentsFormatterUnitsStylePositional)
            {
              result = [result stringByAppendingString: @" "];
            }
          else if (_unitsStyle
	    == NSDateComponentsFormatterUnitsStyleAbbreviated)
            {
              result = [result stringByAppendingString: @" d "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleShort)
            {
              result = [result stringByAppendingString: @" day "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleFull)
            {
              result = [result stringByAppendingString: @" days "];
            }
        }
    }
  if (_allowedUnits & NSCalendarUnitHour)
    {
      if (_unitsStyle == NSDateComponentsFormatterUnitsStyleSpellOut)
        {
          NSNumberFormatter *fmt = [[NSNumberFormatter alloc] init];
          NSNumber *num = [NSNumber numberWithInteger: [components hour]];
          AUTORELEASE(fmt);
          [fmt setNumberStyle: NSNumberFormatterSpellOutStyle];
          result
	    = [result stringByAppendingString: [fmt stringFromNumber: num]];
          result = [result stringByAppendingString: @" hours "];  
        }
      else
        {
          if (_zeroFormattingBehavior
	    & NSDateComponentsFormatterZeroFormattingBehaviorDefault)
            {
              NSString *s;

              s = [NSString stringWithFormat: @"%2ld", [components hour]];
              result = [result stringByAppendingString: s];
            }

          if (_unitsStyle == NSDateComponentsFormatterUnitsStylePositional)
            {
              result = [result stringByAppendingString: @" "];
            }
          else if (_unitsStyle
	    == NSDateComponentsFormatterUnitsStyleAbbreviated)
            {
              result = [result stringByAppendingString: @" h "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleShort)
            {
              result = [result stringByAppendingString: @" hrs "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleFull)
            {
              result = [result stringByAppendingString: @" hours "];
            }
        }
    }
  if (_allowedUnits & NSCalendarUnitMinute)
    {
      if (_unitsStyle == NSDateComponentsFormatterUnitsStyleSpellOut)
        {
          NSNumberFormatter *fmt = [[NSNumberFormatter alloc] init];
          NSNumber *num = [NSNumber numberWithInteger: [components minute]];
          AUTORELEASE(fmt);
          [fmt setNumberStyle: NSNumberFormatterSpellOutStyle];
          result
	    = [result stringByAppendingString: [fmt stringFromNumber: num]];
          result = [result stringByAppendingString: @" minutes "];  
        }
      else
        {
          if (_zeroFormattingBehavior
	    & NSDateComponentsFormatterZeroFormattingBehaviorDefault)
            {
              NSString *s;

              s = [NSString stringWithFormat: @"%2ld", [components minute]];
              result = [result stringByAppendingString: s];
            }

          if (_unitsStyle == NSDateComponentsFormatterUnitsStylePositional)
            {
              result = [result stringByAppendingString: @" "];
            }
          else if (_unitsStyle
	    == NSDateComponentsFormatterUnitsStyleAbbreviated)
            {
              result = [result stringByAppendingString: @" min "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleShort)
            {
              result = [result stringByAppendingString: @" mins "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleFull)
            {
              result = [result stringByAppendingString: @" minutes "];
            }
        }
    }
  if (_allowedUnits & NSCalendarUnitSecond)
    {
      if (_unitsStyle == NSDateComponentsFormatterUnitsStyleSpellOut)
        {
          NSNumberFormatter *fmt = [[NSNumberFormatter alloc] init];
          NSNumber *num = [NSNumber numberWithInteger: [components second]];
          AUTORELEASE(fmt);
          [fmt setNumberStyle: NSNumberFormatterSpellOutStyle];
          result
	    = [result stringByAppendingString: [fmt stringFromNumber: num]];
          result = [result stringByAppendingString: @" seconds "];  
        }
      else
        {
          if (_zeroFormattingBehavior
	    & NSDateComponentsFormatterZeroFormattingBehaviorDefault)
            {
              NSString *s;

              s = [NSString stringWithFormat: @"%2ld", [components second]];
              result = [result stringByAppendingString: s];
            }

          if (_unitsStyle == NSDateComponentsFormatterUnitsStylePositional)
            {
              result = [result stringByAppendingString: @" "];
            }
          else if (_unitsStyle
	    == NSDateComponentsFormatterUnitsStyleAbbreviated)
            {
              result = [result stringByAppendingString: @" s "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleShort)
            {
              result = [result stringByAppendingString: @" secs "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleFull)
            {
              result = [result stringByAppendingString: @" seconds "];
            }
        }
    }
  if (_allowedUnits & NSCalendarUnitWeekOfMonth)
    {
      if (_unitsStyle == NSDateComponentsFormatterUnitsStyleSpellOut)
        {
          NSNumberFormatter *fmt;
          NSNumber *num;

          fmt = AUTORELEASE([[NSNumberFormatter alloc] init]);
          num = [NSNumber numberWithInteger: [components weekOfMonth]];
          [fmt setNumberStyle: NSNumberFormatterSpellOutStyle];
          result
	    = [result stringByAppendingString: [fmt stringFromNumber: num]];
          result = [result stringByAppendingString: @" days "];  
        }
      else
        {
          if (_zeroFormattingBehavior
	    & NSDateComponentsFormatterZeroFormattingBehaviorDefault)
            {
	      long	wom = (long)[components weekOfMonth];
              NSString *s = [NSString stringWithFormat: @"%2ld", wom];
              result = [result stringByAppendingString: s];
            }

          if (_unitsStyle == NSDateComponentsFormatterUnitsStylePositional)
            {
              result = [result stringByAppendingString: @" "];
            }
          else if (_unitsStyle
	    == NSDateComponentsFormatterUnitsStyleAbbreviated)
            {
              result = [result stringByAppendingString: @" wm "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleShort)
            {
              result = [result stringByAppendingString: @" wom "];
            }
          else if (_unitsStyle == NSDateComponentsFormatterUnitsStyleFull)
            {
              result = [result stringByAppendingString: @" week of month "];
            }
        }
    }
  
  return result;
}

- (NSString *) stringFromDate: (NSDate *)startDate
                       toDate: (NSDate *)endDate
{
  NSDateComponents	*dc;
  NSCalendar		*calendar;

  calendar = ( _calendar != nil ) ? _calendar : [NSCalendar currentCalendar];
  dc = [calendar components: _allowedUnits
                   fromDate: startDate
                     toDate: endDate
                    options: NSCalendarMatchStrictly];
  return [self stringFromDateComponents: dc];
}

- (NSString *) stringFromTimeInterval: (NSTimeInterval)ti
{
  NSDate *startDate = [NSDate date];
  NSDate *endDate = [startDate dateByAddingTimeInterval: (ti > 0) ? ti : -ti];
  return [self stringFromDate: startDate toDate: endDate];
}

- (NSDateComponentsFormatterUnitsStyle) unitsStyle
{
  return _unitsStyle;
}

- (void) setUnitsStyle: (NSDateComponentsFormatterUnitsStyle)style
{
  _unitsStyle = style;
}
  
- (NSCalendarUnit) allowedUnits
{
  return _allowedUnits;
}

- (void) setAllowedUnits: (NSCalendarUnit)units
{
  if (units & NSCalendarUnitYear
    && units & NSCalendarUnitMonth
    && units & NSCalendarUnitDay
    && units & NSCalendarUnitHour
    && units & NSCalendarUnitMinute
    && units & NSCalendarUnitSecond
    && units & NSCalendarUnitWeekOfMonth)
    {
      [NSException raise: NSInvalidArgumentException
                  format: @"Passed invalid unit into allowedUnits"];
    }
  _allowedUnits = units;
}

- (NSDateComponentsFormatterZeroFormattingBehavior) zeroFormattingBehavior
{
  return _zeroFormattingBehavior;
}

- (void) setZeroFormattingBehavior: (NSDateComponentsFormatterZeroFormattingBehavior)behavior;
{
  _zeroFormattingBehavior = behavior;
}

- (NSCalendar *) calendar
{
  return _calendar;
}

- (void) setCalender: (NSCalendar *)calendar
{
  ASSIGNCOPY(_calendar, calendar);
}

- (NSDate *) referenceDate
{
  return _referenceDate;
}

- (void) setReferenceDate: (NSDate *)referenceDate
{
  ASSIGNCOPY(_referenceDate, referenceDate);
}

- (BOOL) allowsFractionalUnits
{
  return _allowsFractionalUnits;
}

- (void) setAllowsFractionalUnits: (BOOL)allowsFractionalUnits
{
  _allowsFractionalUnits = allowsFractionalUnits;
}

- (NSInteger) maximumUnitCount
{
  return _maximumUnitCount;
}

- (void) setMaximumUnitCount: (NSInteger)maximumUnitCount
{
  _maximumUnitCount = maximumUnitCount;
}

- (BOOL) collapsesLargestUnit
{
  return _collapsesLargestUnit;
}

- (void) setCollapsesLargestUnit: (BOOL)collapsesLargestUnit
{
  _collapsesLargestUnit = collapsesLargestUnit;
}

- (BOOL) includesApproximationPhrase
{
  return _includesApproximationPhrase;
}

- (void) setIncludesApproximationPhrase: (BOOL)includesApproximationPhrase
{
  _includesApproximationPhrase = includesApproximationPhrase;
}

- (NSFormattingContext) formattingContext
{
  return _formattingContext;
}

- (void) setFormattingContext: (NSFormattingContext)formattingContext
{
  _formattingContext = formattingContext;
}

- (BOOL) getObjectValue: (id*)obj
	      forString: (NSString *)string
       errorDescription: (NSString **)error
{
  return NO;
}

+ (NSString *) localizedStringFromDateComponents: (NSDateComponents *)components
                                      unitsStyle: (NSDateComponentsFormatterUnitsStyle)unitsStyle
{
  NSDateComponentsFormatter *fmt = [[NSDateComponentsFormatter alloc] init];
  [fmt setUnitsStyle: unitsStyle];
  AUTORELEASE(fmt);
  return [fmt stringFromDateComponents: components];
}
  
@end

