/* Implementation of class NSMeasurementFormatter
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento <greg.casamento@gmail.com>
   Date: Mon Sep 30 15:58:21 EDT 2019

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

#import "Foundation/NSLocale.h"
#import "Foundation/NSMeasurement.h"
#import "Foundation/NSMeasurementFormatter.h"
#import "Foundation/NSNumberFormatter.h"
#import "Foundation/NSUnit.h"

@implementation NSMeasurementFormatter

- (instancetype) init
{
  self = [super init];
  if (self != nil)
    {
      _unitOptions = NSMeasurementFormatterUnitOptionsProvidedUnit;
      _unitStyle = NSFormattingUnitStyleMedium;
      _locale = RETAIN([NSLocale currentLocale]);
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_locale);
  [super dealloc];
}

- (NSMeasurementFormatterUnitOptions) unitOptions
{
  return _unitOptions;
}

- (void) setUnitOptions: (NSMeasurementFormatterUnitOptions) unitOptions
{
  _unitOptions = unitOptions;
}
  
- (NSFormattingUnitStyle) unitStyle
{
  return _unitStyle;
}

- (void) setUnitStyle: (NSFormattingUnitStyle)style
{
  _unitStyle = style;
}

- (NSLocale *) locale
{
  return _locale;
}

- (void) setLocale: (NSLocale *)locale
{
  ASSIGNCOPY(_locale, locale);
}

- (NSNumberFormatter *) numberFormatter
{
  return _numberFormatter;
}

- (void) setNumberFormatter: (NSNumberFormatter *)numberFormatter
{
  ASSIGNCOPY(_numberFormatter, numberFormatter);
}
  
- (NSString *) stringFromMeasurement: (NSMeasurement *)measurement
{
  NSString *result = nil;
  NSNumber *num = [NSNumber numberWithDouble: [measurement doubleValue]];
  NSUnit *u = [measurement unit];
  
  result = [_numberFormatter stringForObjectValue: num];
  switch (_unitStyle)
    {
    case NSFormattingUnitStyleShort:
    case NSFormattingUnitStyleMedium:
    case NSFormattingUnitStyleLong:
      result = [result stringByAppendingString: [self stringFromUnit: u]];
      break;
    }

  return result;
}

- (NSString *) stringFromUnit: (NSUnit *)unit
{
  return [unit symbol];
}

- (NSString *) stringForObjectValue: (id)obj
{
  NSString *result = nil;
  if ([obj isKindOfClass: [NSMeasurement class]])
    {
      result = [self stringFromMeasurement: obj];
    }
  else if ([obj isKindOfClass: [NSUnit class]])
    {
      result = [self stringFromUnit: obj];
    }
  return result;
}

- (id) initWithCoder: (NSCoder*)decoder
{
  self = [super initWithCoder: decoder];
  return self;
}

- (void) encodeWithCoder: (NSCoder*)encoder
{
  [super encodeWithCoder: encoder];
}
@end

