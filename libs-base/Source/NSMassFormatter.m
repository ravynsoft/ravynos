
/* Implementation of class NSMassFormatter
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

#import "Foundation/NSMassFormatter.h"
#import "Foundation/NSMeasurement.h"
#import "Foundation/NSMeasurementFormatter.h"
#import "Foundation/NSNumberFormatter.h"
#import "Foundation/NSUnit.h"

@implementation NSMassFormatter

- (instancetype) init
{
  self = [super init];
  if (self != nil)
    {
      _numberFormatter = nil;
      _unitStyle = NSFormattingUnitStyleMedium;
      _isForPersonMassUse = NO;
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

- (void) setUnitStyle: (NSFormattingUnitStyle)style;
{
  _unitStyle = style;
}

- (BOOL) isForPersonMassUse;
{
  return _isForPersonMassUse;
}

- (void) setForPersonMassUse: (BOOL)flag;
{
  _isForPersonMassUse = flag;
}

- (NSString *) stringFromValue: (double)value unit: (NSMassFormatterUnit)unit;
{
  NSUnit *u = nil;
  NSMeasurement *m = nil;
  NSMeasurementFormatter *mf = nil;

  switch(unit)
    {
    case NSMassFormatterUnitGram:
      u = [NSUnitMass grams];
      break;
    case NSMassFormatterUnitKilogram:
      u = [NSUnitMass kilograms];
      break;
    case NSMassFormatterUnitOunce:
      u = [NSUnitMass ounces];
      break;
    case NSMassFormatterUnitPound:
      u = [NSUnitMass pounds];
      break;
    case NSMassFormatterUnitStone:
      u = [NSUnitMass stones];
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

- (NSString *) stringFromKilograms: (double)numberInKilograms;
{
  return [self stringFromValue: numberInKilograms unit: NSMassFormatterUnitKilogram];
}

- (NSString *) unitStringFromValue: (double)value unit: (NSMassFormatterUnit)unit;
{
  return [self stringFromValue: value unit: unit];
}

- (NSString *) unitStringFromKilograms: (double)numberInKilograms usedUnit: (NSMassFormatterUnit *)unit
{
  *unit = NSMassFormatterUnitKilogram;
  return [self stringFromValue: numberInKilograms unit: *unit];
}

- (BOOL) getObjectValue: (id*)obj forString: (NSString *)string errorDescription: (NSString **)error
{
  return NO;
}

@end
