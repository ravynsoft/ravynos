/* Implementation of class NSEnergyFormatter
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento <greg.casamento@gmail.com>
   Date: Tue Oct  8 13:30:10 EDT 2019

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

#import "Foundation/NSEnergyFormatter.h"
#import "Foundation/NSMeasurement.h"
#import "Foundation/NSMeasurementFormatter.h"
#import "Foundation/NSNumberFormatter.h"
#import "Foundation/NSUnit.h"

@implementation NSEnergyFormatter

- (instancetype) init
{
  self = [super init];
  if (self != nil)
    {
      _numberFormatter = nil;
      _unitStyle = NSFormattingUnitStyleMedium;
      _isForFoodEnergyUse = NO;
    }
  return self;
}

- (NSNumberFormatter *) numberFormatter
{
  return _numberFormatter;
}

- (void) setNumberFormatter: (NSNumberFormatter *)formatter
{
  ASSIGN(_numberFormatter, formatter);
}
  
- (NSFormattingUnitStyle) unitStyle
{
  return _unitStyle;
}

- (void) setUnitStyle: (NSFormattingUnitStyle)style
{
  _unitStyle = style;
}
  
- (BOOL) isForFoodEnergyUse
{
  return _isForFoodEnergyUse;
}

- (void) setForFoodEnergyUse: (BOOL)flag
{
  _isForFoodEnergyUse = flag;
}

- (NSString *) stringFromValue: (double)value unit: (NSEnergyFormatterUnit)unit
{
  NSUnit *u = nil;
  NSMeasurement *m = nil;
  NSMeasurementFormatter *mf = nil;

  switch (unit)
    {
    case NSEnergyFormatterUnitJoule:
      u = [NSUnitEnergy joules];
      break;
    case NSEnergyFormatterUnitKilojoule:
      u = [NSUnitEnergy kilojoules];
      break;
    case NSEnergyFormatterUnitCalorie:
      u = [NSUnitEnergy calories];
      break;
    case NSEnergyFormatterUnitKilocalorie:
      u = [NSUnitEnergy kilocalories];
      break;
    }

  m = [[NSMeasurement alloc] initWithDoubleValue: value
                                            unit: u];
  AUTORELEASE(m);
  mf = [[NSMeasurementFormatter alloc] init];
  AUTORELEASE(mf);
  [mf setUnitStyle: _unitStyle];
  [mf setNumberFormatter: _numberFormatter];
  
  return [mf stringFromMeasurement: m];
}

- (NSString *) stringFromJoules: (double)numberInJoules
{
  return [self stringFromValue: numberInJoules unit: NSEnergyFormatterUnitJoule];
}

- (NSString *) unitStringFromValue: (double)value unit: (NSEnergyFormatterUnit)unit
{
  return [self stringFromValue: value unit: unit];
}

- (NSString *) unitStringFromJoules: (double)numberInJoules usedUnit: (NSEnergyFormatterUnit *)unit
{
  *unit = NSEnergyFormatterUnitJoule;
  return [self stringFromValue: numberInJoules unit: *unit];
}

- (BOOL) getObjectValue: (id *)obj forString: (NSString *)string errorDescription: (NSString **)error
{
  return NO;
}

@end

