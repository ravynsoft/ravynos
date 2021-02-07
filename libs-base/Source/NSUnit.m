/* Implementation of class NSUnit
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

#import "Foundation/NSArchiver.h"
#import "Foundation/NSKeyedArchiver.h"
#import "Foundation/NSUnit.h"

// Private methods...
@interface NSDimension (Private)
- (instancetype) initWithSymbol: (NSString *)symbol
                    coefficient: (double)coefficient
                       constant: (double)constant;
@end

// Abstract conversion...
@implementation NSUnitConverter
- (double) baseUnitValueFromValue: (double)value
{
  return 0.0;
}

- (double) valueFromBaseUnitValue: (double)baseUnitValue
{
  return 0.0;
}
@end

// Linear conversion...
@implementation NSUnitConverterLinear
- (instancetype) initWithCoefficient: (double)coefficient
{
  return [self initWithCoefficient: coefficient constant: 0.0];
}

- (instancetype) initWithCoefficient: (double)coefficient
                            constant: (double)constant
{
  self = [super init];
  if (self != nil)
    {
      _coefficient = coefficient;
      _constant = constant;
    }
  return self;
}

- (id) initWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      _coefficient = [coder decodeDoubleForKey: @"NS.coefficient"];
      _constant = [coder decodeDoubleForKey: @"NS.constant"];
    }
  else
    {
      [coder decodeValueOfObjCType: @encode(double) at: &_coefficient];
      [coder decodeValueOfObjCType: @encode(double) at: &_constant];
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if([coder allowsKeyedCoding])
    {
      [coder encodeDouble: _coefficient forKey: @"NS.coefficient"];
      [coder encodeDouble: _constant forKey: @"NS.constant"];
    }
  else
    {
      [coder encodeValueOfObjCType: @encode(double) at: &_coefficient];
      [coder encodeValueOfObjCType: @encode(double) at: &_constant];
    }
}

- (double) coefficient
{
  return _coefficient;
}

- (double) constant
{
  return _constant;
}

- (double) baseUnitValueFromValue: (double)value
{
  return (_coefficient * value) + _constant;
}

- (double) valueFromBaseUnitValue: (double)baseUnitValue
{
  return (baseUnitValue - _constant) / _coefficient;
}
@end

// Abstract unit...
@implementation NSUnit
- (instancetype) init
{
  return [self initWithSymbol: @""];
}

- (instancetype) initWithSymbol: (NSString *)symbol
{
  self = [super init];
  if (self != nil)
    {
      ASSIGNCOPY(_symbol, symbol);
    }
  return self;
}

- (id) initWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      _symbol = [coder decodeObjectForKey: @"NS.symbol"];
    }
  else
    {
      _symbol = [coder decodeObject];
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: _symbol forKey: @"NS.symbol"];
    }
  else
    {
      [coder encodeObject: _symbol];
    }
}

- (instancetype) copyWithZone: (NSZone *)zone
{
  return [[NSUnit allocWithZone: zone] initWithSymbol: [self symbol]];
}

- (NSString *) symbol
{
  return _symbol;
}
@end


// Dimension using units....
@implementation NSDimension
- (NSUnitConverter *) converter
{
  return _converter;
}

- (instancetype) initWithSymbol: (NSString *)symbol converter: (NSUnitConverter *) converter
{
  self = [super initWithSymbol: symbol];
  if (self != nil)
    {
      ASSIGN(_converter, converter);
    }
  return self;
}

- (instancetype) initWithSymbol: (NSString *)symbol
                    coefficient: (double)coefficient
                       constant: (double)constant
{
  NSUnitConverterLinear *converter = [[NSUnitConverterLinear alloc] initWithCoefficient: coefficient
                                                                               constant: constant];
  self = [self initWithSymbol: symbol
                    converter: converter];

  RELEASE(converter);
  return self;
}

+ (instancetype) baseUnit
{
  return nil;
}

- (id) initWithCoder: (NSCoder *)coder
{
  self = [super initWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      _converter = [coder decodeObjectForKey: @"NS.converter"];
    }
  else
    {
      _symbol = [coder decodeObject];
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: _converter forKey: @"NS.converter"];
    }
  else
    {
      [coder encodeObject: _symbol];
    }
}

@end


// Predefined....
@implementation NSUnitAcceleration

+ (instancetype) baseUnit
{
  return [self metersPerSecondSquared];
}

// Base unit - metersPerSecondSquared
+ (NSUnitAcceleration *) metersPerSecondSquared
{
  return AUTORELEASE([[NSUnitAcceleration alloc] initWithSymbol: @"m/s^2"
                                                    coefficient: 1.0
                                                       constant: 0.0]);
}

+ (NSUnitAcceleration *) gravity
{
  return AUTORELEASE([[NSUnitAcceleration alloc] initWithSymbol: @"g"
                                                    coefficient: 9.81
                                                       constant: 0]);
}

@end

@implementation NSUnitAngle

+ (instancetype) baseUnit
{
  return [self degrees];
}

// Base unit - degrees
+ (NSUnitAngle *) degrees
{
  return AUTORELEASE([[NSUnitAngle alloc] initWithSymbol: @"deg"
                                             coefficient: 1.0
                                                constant: 0.0]);
}

+ (NSUnitAngle *) arcMinutes
{
  return AUTORELEASE([[NSUnitAngle alloc] initWithSymbol: @"'"
                                             coefficient: 0.016667
                                                constant: 0.0]);
}

+ (NSUnitAngle *) arcSeconds
{
  return AUTORELEASE([[NSUnitAngle alloc] initWithSymbol: @"\""
                                             coefficient: 0.00027778
                                                constant: 0.0]);
}

+ (NSUnitAngle *) radians
{
  return AUTORELEASE([[NSUnitAngle alloc] initWithSymbol: @"rad"
                                             coefficient: 57.2958
                                                constant: 0.0]);
}

+ (NSUnitAngle *) gradians
{
  return AUTORELEASE([[NSUnitAngle alloc] initWithSymbol: @"grad"
                                             coefficient: 0.9
                                                constant: 0.0]);
}

+ (NSUnitAngle *) revolutions
{
  return AUTORELEASE([[NSUnitAngle alloc] initWithSymbol: @"rev"
                                             coefficient: 360.0
                                                constant: 0.0]);
}

@end

@implementation NSUnitArea

+ (instancetype) baseUnit
{
  return [self squareMeters];
}

// Base unit - squareMeters
+ (NSUnitArea *) squareMegameters
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"Mm^2"
                                            coefficient: 1e12
                                               constant: 0.0]);
}

+ (NSUnitArea *) squareKilometers
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"km^2"
                                            coefficient: 1000000.0
                                               constant: 0.0]);
}

+ (NSUnitArea *) squareMeters
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"m^2"
                                            coefficient: 1.0
                                               constant: 0.0]);
}

+ (NSUnitArea *) squareCentimeters
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"cm^2"
                                            coefficient: 0.0001
                                               constant: 0.0]);
}

+ (NSUnitArea *) squareMillimeters
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"mm^2"
                                            coefficient: 0.000001
                                               constant: 0.0]);
}

+ (NSUnitArea *) squareMicrometers
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"um^2"
                                            coefficient: 1e-12
                                               constant: 0.0]);
}

+ (NSUnitArea *) squareNanometers
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"nm^2"
                                            coefficient: 1e-18
                                               constant: 0.0]);
}

+ (NSUnitArea *) squareInches
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"in^2"
                                            coefficient: 0.00064516
                                               constant: 0.0]);
}

+ (NSUnitArea *) squareFeet
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"ft^2"
                                            coefficient: 0.092903
                                               constant: 0.0]);
}

+ (NSUnitArea *) squareYards
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"yd^2"
                                            coefficient: 0.836127
                                               constant: 0.0]);
}

+ (NSUnitArea *) squareMiles
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"mi^2"
                                            coefficient: 2.59e+6
                                               constant: 0.0]);
}

+ (NSUnitArea *) acres
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"acres"
                                            coefficient: 4046.86
                                               constant: 0.0]);
}

+ (NSUnitArea *) ares
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"ares"
                                            coefficient: 100.0
                                               constant: 0.0]);
}

+ (NSUnitArea *) hectares
{
  return AUTORELEASE([[NSUnitArea alloc] initWithSymbol: @"hectares"
                                            coefficient: 10000.0
                                               constant: 0.0]);
}

@end

@implementation NSUnitConcentrationMass

+ (instancetype) baseUnit
{
  return [self gramsPerLiter];
}

// Base unit - gramsPerLiter
+ (NSUnitConcentrationMass *) gramsPerLiter
{
  return AUTORELEASE([[NSUnitConcentrationMass alloc] initWithSymbol: @"g/L"
                                                         coefficient: 1.0
                                                            constant: 0.0]);
}

+ (NSUnitConcentrationMass *) milligramsPerDeciliter
{
  return AUTORELEASE([[NSUnitConcentrationMass alloc] initWithSymbol: @"mg/dL"
                                                         coefficient: 0.01
                                                            constant: 0.0]);
}

+ (NSUnitConcentrationMass *) millimolesPerLiterWithGramsPerMole:(double)gramsPerMole
{
  return AUTORELEASE([[NSUnitConcentrationMass alloc] initWithSymbol: @"mmol/L"
                                                         coefficient: 18.0 * gramsPerMole
                                                            constant: 0.0]);
}

@end

@implementation NSUnitDispersion

+ (instancetype) baseUnit
{
  return [self partsPerMillion];
}

// Base unit - partsPerMillion
+ (NSUnitDispersion *) partsPerMillion
{
  return AUTORELEASE([[NSUnitDispersion alloc] initWithSymbol: @"ppm"
                                                  coefficient: 1.0
                                                     constant: 0.0]);
}

@end

@implementation NSUnitDuration

+ (instancetype) baseUnit
{
  return [self seconds];
}

// Base unit - seconds
+ (NSUnitDuration *) seconds
{
  return AUTORELEASE([[NSUnitDuration alloc] initWithSymbol: @"sec"
                                                coefficient: 1.0
                                                   constant: 0.0]);
}

+ (NSUnitDuration *) minutes
{
  return AUTORELEASE([[NSUnitDuration alloc] initWithSymbol: @"min"
                                                coefficient: 60.0
                                                   constant: 0.0]);
}

+ (NSUnitDuration *) hours
{
  return AUTORELEASE([[NSUnitDuration alloc] initWithSymbol: @"hr"
                                                coefficient: 3600.0
                                                   constant: 0.0]);
}

@end

@implementation NSUnitElectricCharge

+ (instancetype) baseUnit
{
  return [self coulombs];
}

// Base unit - coulombs
+ (NSUnitElectricCharge *) coulombs
{
  return AUTORELEASE([[NSUnitElectricCharge alloc] initWithSymbol: @"C"
                                                      coefficient: 1.0
                                                         constant: 0.0]);
}

+ (NSUnitElectricCharge *) megaampereHours
{
  return AUTORELEASE([[NSUnitElectricCharge alloc] initWithSymbol: @"MAh"
                                                      coefficient: 3.6e9
                                                         constant: 0.0]);
}

+ (NSUnitElectricCharge *) kiloampereHours
{
  return AUTORELEASE([[NSUnitElectricCharge alloc] initWithSymbol: @"kAh"
                                                      coefficient: 3600000.0
                                                         constant: 0.0]);
}

+ (NSUnitElectricCharge *) ampereHours
{
  return AUTORELEASE([[NSUnitElectricCharge alloc] initWithSymbol: @"mAh"
                                                      coefficient: 3600.0
                                                         constant: 0.0]);
}

+ (NSUnitElectricCharge *) milliampereHours
{
  return AUTORELEASE([[NSUnitElectricCharge alloc] initWithSymbol: @"hr"
                                                      coefficient: 3.6
                                                         constant: 0.0]);
}

+ (NSUnitElectricCharge *) microampereHours
{
  return AUTORELEASE([[NSUnitElectricCharge alloc] initWithSymbol: @"uAh"
                                                      coefficient: 0.0036
                                                         constant: 0.0]);
}

@end

@implementation NSUnitElectricCurrent

+ (instancetype) baseUnit
{
  return [self amperes];
}

// Base unit - amperes
+ (NSUnitElectricCurrent *) megaamperes
{
  return AUTORELEASE([[NSUnitElectricCurrent alloc] initWithSymbol: @"MA"
                                                       coefficient: 1000000.0
                                                          constant: 0.0]);
}

+ (NSUnitElectricCurrent *) kiloamperes
{
  return AUTORELEASE([[NSUnitElectricCurrent alloc] initWithSymbol: @"kA"
                                                       coefficient: 1000.0
                                                          constant: 0.0]);
}

+ (NSUnitElectricCurrent *) amperes
{
  return AUTORELEASE([[NSUnitElectricCurrent alloc] initWithSymbol: @"A"
                                                       coefficient: 1.0
                                                          constant: 0.0]);
}

+ (NSUnitElectricCurrent *) milliamperes
{
  return AUTORELEASE([[NSUnitElectricCurrent alloc] initWithSymbol: @"mA"
                                                       coefficient: 0.001
                                                          constant: 0.0]);
}

+ (NSUnitElectricCurrent *) microamperes
{
  return AUTORELEASE([[NSUnitElectricCurrent alloc] initWithSymbol: @"uA"
                                                       coefficient: 0.000001
                                                          constant: 0.0]);
}

@end

@implementation NSUnitElectricPotentialDifference

+ (instancetype) baseUnit
{
  return [self volts];
}

// Base unit - volts
+ (NSUnitElectricPotentialDifference *) megavolts
{
  return AUTORELEASE([[NSUnitElectricPotentialDifference alloc] initWithSymbol: @"MV"
                                                                   coefficient: 1000000.0
                                                                      constant: 0.0]);
}

+ (NSUnitElectricPotentialDifference *) kilovolts
{
  return AUTORELEASE([[NSUnitElectricPotentialDifference alloc] initWithSymbol: @"kV"
                                                                   coefficient: 1000.0
                                                                      constant: 0.0]);
}

+ (NSUnitElectricPotentialDifference *) volts
{
  return AUTORELEASE([[NSUnitElectricPotentialDifference alloc] initWithSymbol: @"V"
                                                                   coefficient: 1.0
                                                                      constant: 0.0]);
}

+ (NSUnitElectricPotentialDifference *) millivolts
{
  return AUTORELEASE([[NSUnitElectricPotentialDifference alloc] initWithSymbol: @"mV"
                                                                   coefficient: 0.001
                                                                      constant: 0.0]);
}

+ (NSUnitElectricPotentialDifference *) microvolts
{
  return AUTORELEASE([[NSUnitElectricPotentialDifference alloc] initWithSymbol: @"uV"
                                                                   coefficient: 0.000001
                                                                      constant: 0.0]);
}

@end

@implementation NSUnitElectricResistance

+ (instancetype) baseUnit
{
  return [self ohms];
}

// Base unit - ohms
+ (NSUnitElectricResistance *) megaohms
{
  return AUTORELEASE([[NSUnitElectricResistance alloc] initWithSymbol: @"MOhm"
                                                          coefficient: 100000.0
                                                             constant: 0.0]);
}

+ (NSUnitElectricResistance *) kiloohms
{
  return AUTORELEASE([[NSUnitElectricResistance alloc] initWithSymbol: @"kOhm"
                                                          coefficient: 1000.0
                                                             constant: 0.0]);
}

+ (NSUnitElectricResistance *) ohms
{
  return AUTORELEASE([[NSUnitElectricResistance alloc] initWithSymbol: @"Ohm"
                                                          coefficient: 1.0
                                                             constant: 0.0]);
}

+ (NSUnitElectricResistance *) milliohms
{
  return AUTORELEASE([[NSUnitElectricResistance alloc] initWithSymbol: @"mOhm"
                                                          coefficient: 0.001
                                                             constant: 0.0]);
}

+ (NSUnitElectricResistance *) microohms
{
  return AUTORELEASE([[NSUnitElectricResistance alloc] initWithSymbol: @"uOhm"
                                                          coefficient: 0.000001
                                                             constant: 0.0]);
}


@end

@implementation NSUnitEnergy

+ (instancetype) baseUnit
{
  return [self joules];
}

// Base unit - joules
+ (NSUnitEnergy *) kilojoules
{
  return AUTORELEASE([[NSUnitEnergy alloc] initWithSymbol: @"kJ"
                                              coefficient: 1000.0
                                                 constant: 0.0]);
}

+ (NSUnitEnergy *) joules
{
  return AUTORELEASE([[NSUnitEnergy alloc] initWithSymbol: @"J"
                                              coefficient: 1.0
                                                 constant: 0.0]);
}

+ (NSUnitEnergy *) kilocalories
{
  return AUTORELEASE([[NSUnitEnergy alloc] initWithSymbol: @"kCal"
                                              coefficient: 4184.0
                                                 constant: 0.0]);
}

+ (NSUnitEnergy *) calories
{
  return AUTORELEASE([[NSUnitEnergy alloc] initWithSymbol: @"cal"
                                              coefficient: 4.184
                                                 constant: 0.0]);
}

+ (NSUnitEnergy *) kilowattHours
{
  return AUTORELEASE([[NSUnitEnergy alloc] initWithSymbol: @"kWh"
                                              coefficient: 3600000.0
                                                 constant: 0.0]);
}

@end

@implementation NSUnitFrequency

+ (instancetype) baseUnit
{
  return [self hertz];
}

// Base unit - hertz

+ (NSUnitFrequency *) terahertz
{
  return AUTORELEASE([[NSUnitFrequency alloc] initWithSymbol: @"thz"
                                                 coefficient: 1e12
                                                    constant: 0.0]);
}

+ (NSUnitFrequency *) gigahertz
{
  return AUTORELEASE([[NSUnitFrequency alloc] initWithSymbol: @"ghz"
                                                 coefficient: 1e9
                                                    constant: 0.0]);
}

+ (NSUnitFrequency *) megahertz
{
  return AUTORELEASE([[NSUnitFrequency alloc] initWithSymbol: @"GHz"
                                                 coefficient: 1000000.0
                                                    constant: 0.0]);
}

+ (NSUnitFrequency *) kilohertz
{
  return AUTORELEASE([[NSUnitFrequency alloc] initWithSymbol: @"KHz"
                                                 coefficient: 1000.0
                                                    constant: 0.0]);
}

+ (NSUnitFrequency *) hertz
{
  return AUTORELEASE([[NSUnitFrequency alloc] initWithSymbol: @"Hz"
                                                 coefficient: 1.0
                                                    constant: 0.0]);
}

+ (NSUnitFrequency *) millihertz
{
  return AUTORELEASE([[NSUnitFrequency alloc] initWithSymbol: @"mHz"
						 coefficient: 0.001
						    constant: 0.0]);
}

+ (NSUnitFrequency *) microhertz
{
  return AUTORELEASE([[NSUnitFrequency alloc] initWithSymbol: @"uHz"
                                                 coefficient: 0.000001
                                                    constant: 0.0]);
}

+ (NSUnitFrequency *) nanohertz
{
  return AUTORELEASE([[NSUnitFrequency alloc] initWithSymbol: @"nHz"
                                                 coefficient: 1e-9
                                                    constant: 0.0]);
}

@end

@implementation NSUnitFuelEfficiency

+ (instancetype) baseUnit
{
  return [self litersPer100Kilometers];
}

// Base unit - litersPer100Kilometers

+ (NSUnitFuelEfficiency *) litersPer100Kilometers
{
  return AUTORELEASE([[NSUnitFuelEfficiency alloc] initWithSymbol: @"L/100km"
                                                      coefficient: 1.0
                                                         constant: 0.0]);
}

+ (NSUnitFuelEfficiency *) milesPerImperialGallon
{
  // FIXME
  return AUTORELEASE([[NSUnitFuelEfficiency alloc] initWithSymbol: @"mpg"
                                                      coefficient: 0.0
                                                         constant: 0.0]);
}

+ (NSUnitFuelEfficiency *) milesPerGallon
{
  // FIXME
  return AUTORELEASE([[NSUnitFuelEfficiency alloc] initWithSymbol: @"mpg"
                                                      coefficient: 0.0
                                                         constant: 0.0]);
}

@end

@implementation NSUnitLength

+ (instancetype) baseUnit
{
  return [self meters];
}

// Base unit - meters

+ (NSUnitLength *) megameters
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"Mm"
                                              coefficient: 1000000.0
                                                 constant: 0.0]);
}

+ (NSUnitLength *) kilometers
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"kM"
                                              coefficient: 1000.0
                                                 constant: 0.0]);
}

+ (NSUnitLength *) hectometers
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"hm"
                                              coefficient: 100.0
                                                 constant: 0.0]);
}

+ (NSUnitLength *) decameters
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"dam"
                                              coefficient: 10
                                                 constant: 0.0]);
}

+ (NSUnitLength *) meters
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"meters"
                                              coefficient: 1.0
                                                 constant: 0.0]);
}

+ (NSUnitLength *) decimeters
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"dm"
                                              coefficient: 0.1
                                                 constant: 0.0]);
}

+ (NSUnitLength *) centimeters
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"cm"
                                              coefficient: 0.01
                                                 constant: 0.0]);
}

+ (NSUnitLength *) millimeters
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"mm"
                                              coefficient: 0.001
                                                 constant: 0.0]);
}

+ (NSUnitLength *) micrometers
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"um"
                                              coefficient: 0.000001
                                                 constant: 0.0]);
}

+ (NSUnitLength *) nanometers
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"nm"
                                              coefficient: 1e-9
                                                 constant: 0.0]);
}

+ (NSUnitLength *) picometers
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"pm"
                                              coefficient: 1e-12
                                                 constant: 0.0]);
}

+ (NSUnitLength *) inches
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"in"
                                              coefficient: 0.254
                                                 constant: 0.0]);
}

+ (NSUnitLength *) feet
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"ft"
                                              coefficient: 0.3048
                                                 constant: 0.0]);
}

+ (NSUnitLength *) yards
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"yd"
                                              coefficient: 0.9144
                                                 constant: 0.0]);
}

+ (NSUnitLength *) miles
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"mi"
                                              coefficient: 1609.34
                                                 constant: 0.0]);
}

+ (NSUnitLength *) scandinavianMiles
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"smi"
                                              coefficient: 10000
                                                 constant: 0.0]);
}

+ (NSUnitLength *) lightyears
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"ly"
                                              coefficient: 9.461e+15
                                                 constant: 0.0]);
}

+ (NSUnitLength *) nauticalMiles
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"NM"
                                              coefficient: 1852.0
                                                 constant: 0.0]);
}

+ (NSUnitLength *) fathoms
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"ftm"
                                              coefficient: 1.8288
                                                 constant: 0.0]);
}

+ (NSUnitLength *) furlongs
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"fur"
                                              coefficient: 201.168
                                                 constant: 0.0]);
}

+ (NSUnitLength *) astronomicalUnits
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"ua"
                                              coefficient: 1.496e+11
                                                 constant: 0.0]);
}

+ (NSUnitLength *) parsecs
{
  return AUTORELEASE([[NSUnitLength alloc] initWithSymbol: @"pc"
                                              coefficient: 3.086e+16
                                                 constant: 0.0]);
}

@end

@implementation NSUnitIlluminance

+ (instancetype) baseUnit
{
  return [self lux];
}

// Base unit - lux
+ (NSUnitIlluminance *) lux
{
  return AUTORELEASE([[NSUnitIlluminance alloc] initWithSymbol: @"lux"
                                                   coefficient: 1.0
                                                      constant: 0.0]);
}

@end

@implementation NSUnitMass

+ (instancetype) baseUnit
{
  return [self kilograms];
}

// Base unit - kilograms

+ (NSUnitMass *) kilograms
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"kg"
                                            coefficient: 1.0
                                               constant: 0.0]);
}

+ (NSUnitMass *) grams
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"g"
                                            coefficient: 0.001
                                               constant: 0.0]);
}

+ (NSUnitMass *) decigrams
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"dg"
                                            coefficient: 0.0001
                                               constant: 0.0]);
}

+ (NSUnitMass *) centigrams
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"cg"
                                            coefficient: 0.00001
                                               constant: 0.0]);
}

+ (NSUnitMass *) milligrams
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"mg"
                                            coefficient: 0.000001
                                               constant: 0.0]);
}

+ (NSUnitMass *) micrograms
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"ug"
                                            coefficient: 1e-9
                                               constant: 0.0]);
}

+ (NSUnitMass *) nanograms
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"ng"
                                            coefficient: 1e-12
                                               constant: 0.0]);
}

+ (NSUnitMass *) picograms
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"pg"
                                            coefficient: 1e-15
                                               constant: 0.0]);
}

+ (NSUnitMass *) ounces
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"oz"
                                            coefficient: 0.0283495
                                               constant: 0.0]);
}

+ (NSUnitMass *) pounds
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"lb"
                                            coefficient: 0.453592
                                               constant: 0.0]);
}

+ (NSUnitMass *) stones
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"st"
                                            coefficient: 6.35029
                                               constant: 0.0]);
}

+ (NSUnitMass *) metricTons
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"t"
                                            coefficient: 1000
                                               constant: 0.0]);
}

+ (NSUnitMass *) shortTons
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"ton"
                                            coefficient: 907.185
                                               constant: 0.0]);
}

+ (NSUnitMass *) carats
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"ct"
                                            coefficient: 0.0002
                                               constant: 0.0]);
}

+ (NSUnitMass *) ouncesTroy
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"oz t"
                                            coefficient: 0.03110348
                                               constant: 0.0]);
}

+ (NSUnitMass *) slugs
{
  return AUTORELEASE([[NSUnitMass alloc] initWithSymbol: @"slug"
                                            coefficient: 14.5939
                                               constant: 0.0]);
}

@end

@implementation NSUnitPower

+ (instancetype) baseUnit
{
  return [self watts];
}

// Base unit - watts

+ (NSUnitPower *) terawatts
{
  return AUTORELEASE([[NSUnitPower alloc] initWithSymbol: @"TW"
                                             coefficient: 1e12
                                                constant: 0.0]);
}

+ (NSUnitPower *) gigawatts
{
  return AUTORELEASE([[NSUnitPower alloc] initWithSymbol: @"GW"
                                             coefficient: 1e9
                                                constant: 0.0]);
}

+ (NSUnitPower *) megawatts
{
  return AUTORELEASE([[NSUnitPower alloc] initWithSymbol: @"MW"
                                             coefficient: 1000000.0
                                                constant: 0.0]);
}

+ (NSUnitPower *) kilowatts
{
  return AUTORELEASE([[NSUnitPower alloc] initWithSymbol: @"kW"
                                             coefficient: 1000.0
                                                constant: 0.0]);
}

+ (NSUnitPower *) watts
{
  return AUTORELEASE([[NSUnitPower alloc] initWithSymbol: @"W"
                                             coefficient: 1.0
                                                constant: 0.0]);
}

+ (NSUnitPower *) milliwatts
{
  return AUTORELEASE([[NSUnitPower alloc] initWithSymbol: @"mW"
                                             coefficient: 0.001
                                                constant: 0.0]);
}

+ (NSUnitPower *) microwatts
{
  return AUTORELEASE([[NSUnitPower alloc] initWithSymbol: @"uW"
                                             coefficient: 0.000001
                                                constant: 0.0]);
}

+ (NSUnitPower *) nanowatts
{
  return AUTORELEASE([[NSUnitPower alloc] initWithSymbol: @"nW"
                                             coefficient: 1e-9
                                                constant: 0.0]);
}

+ (NSUnitPower *) picowatts
{
  return AUTORELEASE([[NSUnitPower alloc] initWithSymbol: @"pW"
                                             coefficient: 1e-12
                                                constant: 0.0]);
}

+ (NSUnitPower *) femtowatts
{
  return AUTORELEASE([[NSUnitPower alloc] initWithSymbol: @"fW"
                                             coefficient: 1e-15
                                                constant: 0.0]);
}

+ (NSUnitPower *) horsepower
{
  return AUTORELEASE([[NSUnitPower alloc] initWithSymbol: @"hp"
                                             coefficient: 745.7
                                                constant: 0.0]);
}

@end

@implementation NSUnitPressure

+ (instancetype) baseUnit
{
  return [self newtonsPerMetersSquared];
}

// Base unit - newtonsPerMetersSquared (equivalent to 1 pascal)

+ (NSUnitPressure *) newtonsPerMetersSquared
{
  return AUTORELEASE([[NSUnitPressure alloc] initWithSymbol: @"N/m^2"
                                                coefficient: 1.0
                                                   constant: 0.0]);
}

+ (NSUnitPressure *) gigapascals
{
  return AUTORELEASE([[NSUnitPressure alloc] initWithSymbol: @"GPa"
                                                coefficient: 1e9
                                                   constant: 0.0]);
}

+ (NSUnitPressure *) megapascals
{
  return AUTORELEASE([[NSUnitPressure alloc] initWithSymbol: @"MPa"
                                                coefficient: 1000000.0
                                                   constant: 0.0]);
}

+ (NSUnitPressure *) kilopascals
{
  return AUTORELEASE([[NSUnitPressure alloc] initWithSymbol: @"kPa"
                                                coefficient: 1000.0
                                                   constant: 0.0]);
}

+ (NSUnitPressure *) hectopascals
{
  return AUTORELEASE([[NSUnitPressure alloc] initWithSymbol: @"hPa"
                                                coefficient: 100.0
                                                   constant: 0.0]);
}

+ (NSUnitPressure *) inchesOfMercury
{
  return AUTORELEASE([[NSUnitPressure alloc] initWithSymbol: @"inHg"
                                                coefficient: 3386.0
                                                   constant: 0.0]);
}

+ (NSUnitPressure *) bars
{
  return AUTORELEASE([[NSUnitPressure alloc] initWithSymbol: @"bars"
                                                coefficient: 100000.0
                                                   constant: 0.0]);
}

+ (NSUnitPressure *) millibars
{
  return AUTORELEASE([[NSUnitPressure alloc] initWithSymbol: @"mbars"
                                                coefficient: 100.0
                                                   constant: 0.0]);
}

+ (NSUnitPressure *) millimetersOfMercury
{
  return AUTORELEASE([[NSUnitPressure alloc] initWithSymbol: @"mmHg"
                                                coefficient: 133.322
                                                   constant: 0.0]);
}

+ (NSUnitPressure *) poundsForcePerSquareInch
{
  return AUTORELEASE([[NSUnitPressure alloc] initWithSymbol: @"psi"
                                                coefficient: 6894.76
                                                   constant: 0.0]);
}


@end

@implementation NSUnitSpeed
+ (instancetype) baseUnit
{
  return [self metersPerSecond];
}

// Base unit - metersPerSecond
+ (NSUnitSpeed *) metersPerSecond
{
  return AUTORELEASE([[NSUnitSpeed alloc] initWithSymbol: @"m/s"
                                             coefficient: 1.0
                                                constant: 0.0]);
}

+ (NSUnitSpeed *) kilometersPerHour
{
  return AUTORELEASE([[NSUnitSpeed alloc] initWithSymbol: @"km/h"
                                             coefficient: 0.277778
                                                constant: 0.0]);
}

+ (NSUnitSpeed *) milesPerHour
{
  return AUTORELEASE([[NSUnitSpeed alloc] initWithSymbol: @"mph"
                                             coefficient: 0.44704
                                                constant: 0.0]);
}

+ (NSUnitSpeed *) knots
{
  return AUTORELEASE([[NSUnitSpeed alloc] initWithSymbol: @"kn"
                                             coefficient: 0.51444
                                                constant: 0.0]);
}

@end

@implementation NSUnitTemperature
+ (instancetype) baseUnit
{
  return [self kelvin];
}

// Base unit - kelvin
+ (NSUnitTemperature *) kelvin
{
  return AUTORELEASE([[NSUnitTemperature alloc] initWithSymbol: @"K"
                                                   coefficient: 1.0
                                                      constant: 0.0]);
}

+ (NSUnitTemperature *) celsius
{
  return AUTORELEASE([[NSUnitTemperature alloc] initWithSymbol: @"C"
                                                   coefficient: 1.0
                                                      constant: 273.15]);
}

+ (NSUnitTemperature *) fahrenheit
{
  return AUTORELEASE([[NSUnitTemperature alloc] initWithSymbol: @"F"
                                                   coefficient: 0.55555555555556
                                                      constant: 255.37222222222427]);
}
@end

@implementation NSUnitVolume
+ (instancetype) baseUnit
{
  return [self liters];
}

// Base unit - liters
+ (NSUnitVolume *) megaliters
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"ML"
                                              coefficient: 1000000.0
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) kiloliters
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"kL"
                                              coefficient: 1000.0
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) liters
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"L"
                                              coefficient: 1.0
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) deciliters
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"dL"
                                              coefficient: 0.1
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) centiliters
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"cL"
                                              coefficient: 0.01
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) milliliters
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"mL"
                                              coefficient: 0.001
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) cubicKilometers
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"km^3"
                                              coefficient: 1e12
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) cubicMeters
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"m^3"
                                              coefficient: 1000.0
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) cubicDecimeters
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"dm^3"
                                              coefficient: 1.0
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) cubicCentimeters
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"cm^3"
                                              coefficient: 0.0001
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) cubicMillimeters
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"mm^3"
                                              coefficient: 0.000001
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) cubicInches
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"in^3"
					      coefficient: 0.0163871
						 constant: 0.0]);
}

+ (NSUnitVolume *) cubicFeet
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"ft^3"
                                              coefficient: 28.3168
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) cubicYards
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"yd^3"
                                              coefficient: 764.555
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) cubicMiles
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"mi^3"
                                              coefficient: 4.168e+12
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) acreFeet
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"af"
                                              coefficient: 1.233e+6
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) bushels
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"bsh"
                                              coefficient: 32.2391
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) teaspoons
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"tsp"
                                              coefficient: 0.00492892
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) tablespoons
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"tbsp"
                                              coefficient: 0.0147868
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) fluidOunces
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"fl oz"
                                              coefficient: 0.295735
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) cups
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"cups"
                                              coefficient: 0.24
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) pints
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"pt"
                                              coefficient: 0.473176
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) quarts
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"qt"
                                              coefficient: 0.946353
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) gallons
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"gal"
                                              coefficient: 3.78541
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) imperialTeaspoons
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"tsp"
                                              coefficient: 0.00591939
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) imperialTablespoons
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"tbsp"
                                              coefficient: 0.0177582
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) imperialFluidOunces
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"fl oz"
                                              coefficient: 0.0284131
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) imperialPints
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"pt"
                                              coefficient: 0.568261
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) imperialQuarts
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"qt"
                                              coefficient: 1.13652
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) imperialGallons
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"gal"
                                              coefficient: 4.54609
                                                 constant: 0.0]);
}

+ (NSUnitVolume *) metricCups
{
  return AUTORELEASE([[NSUnitVolume alloc] initWithSymbol: @"metric cup"
                                              coefficient: 0.25
                                                 constant: 0.0]);
}

@end
