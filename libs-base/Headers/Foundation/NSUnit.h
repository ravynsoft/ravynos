/* Definition of class NSUnit
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

#ifndef _NSUnit_h_GNUSTEP_BASE_INCLUDE
#define _NSUnit_h_GNUSTEP_BASE_INCLUDE

#import <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_12, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

// Unit converter
GS_EXPORT_CLASS
@interface NSUnitConverter : NSObject
- (double) baseUnitValueFromValue: (double)value;
- (double) valueFromBaseUnitValue: (double)baseUnitValue;
@end

// Linea converter... for things like C <-> F conversion...
GS_EXPORT_CLASS
@interface NSUnitConverterLinear : NSUnitConverter <NSCoding>
{
  double _coefficient;
  double _constant;
}
- (instancetype) initWithCoefficient: (double)coefficient;
- (instancetype) initWithCoefficient: (double)coefficient
                            constant: (double)constant;
- (double) coefficient;
- (double) constant;
@end

// Units...  abstract...
GS_EXPORT_CLASS
@interface NSUnit : NSObject <NSCopying, NSCoding>
{
  NSString *_symbol;
}
  
- (instancetype) init;
- (instancetype) initWithSymbol: (NSString *)symbol;
- (NSString *) symbol;

@end

// Dimension using units....
GS_EXPORT_CLASS
@interface NSDimension : NSUnit <NSCoding>
{
    double _value;
    NSUnitConverter *_converter;
}

- (NSUnitConverter *) converter;
- (instancetype) initWithSymbol: (NSString *)symbol converter: (NSUnitConverter *)converter;
+ (instancetype) baseUnit;

@end

// Predefined....
GS_EXPORT_CLASS
@interface NSUnitAcceleration : NSDimension 
/*
 Base unit - metersPerSecondSquared
 */

+ (NSUnitAcceleration *) metersPerSecondSquared;
+ (NSUnitAcceleration *) gravity;

@end

GS_EXPORT_CLASS
@interface NSUnitAngle : NSDimension 
/*
 Base unit - degrees
 */

+ (NSUnitAngle *) degrees;
+ (NSUnitAngle *) arcMinutes;
+ (NSUnitAngle *) arcSeconds;
+ (NSUnitAngle *) radians;
+ (NSUnitAngle *) gradians;
+ (NSUnitAngle *) revolutions;

@end

GS_EXPORT_CLASS
@interface NSUnitArea : NSDimension 
/*
 Base unit - squareMeters
 */

+ (NSUnitArea *) squareMegameters;
+ (NSUnitArea *) squareKilometers;
+ (NSUnitArea *) squareMeters;
+ (NSUnitArea *) squareCentimeters;
+ (NSUnitArea *) squareMillimeters;
+ (NSUnitArea *) squareMicrometers;
+ (NSUnitArea *) squareNanometers;
+ (NSUnitArea *) squareInches;
+ (NSUnitArea *) squareFeet;
+ (NSUnitArea *) squareYards;
+ (NSUnitArea *) squareMiles;
+ (NSUnitArea *) acres;
+ (NSUnitArea *) ares;
+ (NSUnitArea *) hectares;

@end

GS_EXPORT_CLASS
@interface NSUnitConcentrationMass : NSDimension 
/*
 Base unit - gramsPerLiter
 */

+ (NSUnitConcentrationMass *) gramsPerLiter;
+ (NSUnitConcentrationMass *) milligramsPerDeciliter;

+ (NSUnitConcentrationMass *) millimolesPerLiterWithGramsPerMole: (double)gramsPerMole;

@end

GS_EXPORT_CLASS
@interface NSUnitDispersion : NSDimension 
/*
 Base unit - partsPerMillion
 */
+ (NSUnitDispersion *) partsPerMillion;

@end

GS_EXPORT_CLASS
@interface NSUnitDuration : NSDimension   
/*
 Base unit - seconds
 */

+ (NSUnitDuration *) seconds;
+ (NSUnitDuration *) minutes;
+ (NSUnitDuration *) hours;

@end

GS_EXPORT_CLASS
@interface NSUnitElectricCharge : NSDimension 
/*
 Base unit - coulombs
 */

+ (NSUnitElectricCharge *) coulombs;
+ (NSUnitElectricCharge *) megaampereHours;
+ (NSUnitElectricCharge *) kiloampereHours;
+ (NSUnitElectricCharge *) ampereHours;
+ (NSUnitElectricCharge *) milliampereHours;
+ (NSUnitElectricCharge *) microampereHours;

@end

GS_EXPORT_CLASS
@interface NSUnitElectricCurrent : NSDimension 
/*
 Base unit - amperes
 */

+ (NSUnitElectricCurrent *) megaamperes;
+ (NSUnitElectricCurrent *) kiloamperes;
+ (NSUnitElectricCurrent *) amperes;
+ (NSUnitElectricCurrent *) milliamperes;
+ (NSUnitElectricCurrent *) microamperes;

@end

GS_EXPORT_CLASS
@interface NSUnitElectricPotentialDifference : NSDimension 
/*
 Base unit - volts
 */

+ (NSUnitElectricPotentialDifference *) megavolts;
+ (NSUnitElectricPotentialDifference *) kilovolts;
+ (NSUnitElectricPotentialDifference *) volts;
+ (NSUnitElectricPotentialDifference *) millivolts;
+ (NSUnitElectricPotentialDifference *) microvolts;

@end

GS_EXPORT_CLASS
@interface NSUnitElectricResistance : NSDimension 
/*
 Base unit - ohms
 */

+ (NSUnitElectricResistance *) megaohms;
+ (NSUnitElectricResistance *) kiloohms;
+ (NSUnitElectricResistance *) ohms;
+ (NSUnitElectricResistance *) milliohms;
+ (NSUnitElectricResistance *) microohms;

@end

GS_EXPORT_CLASS
@interface NSUnitEnergy : NSDimension 
/*
 Base unit - joules
 */

+ (NSUnitEnergy *) kilojoules;
+ (NSUnitEnergy *) joules;
+ (NSUnitEnergy *) kilocalories;
+ (NSUnitEnergy *) calories;
+ (NSUnitEnergy *) kilowattHours;

@end

GS_EXPORT_CLASS
@interface NSUnitFrequency : NSDimension 
/*
 Base unit - hertz
 */

+ (NSUnitFrequency *) terahertz;
+ (NSUnitFrequency *) gigahertz;
+ (NSUnitFrequency *) megahertz;
+ (NSUnitFrequency *) kilohertz;
+ (NSUnitFrequency *) hertz;
+ (NSUnitFrequency *) millihertz;
+ (NSUnitFrequency *) microhertz;
+ (NSUnitFrequency *) nanohertz;

@end

GS_EXPORT_CLASS
@interface NSUnitFuelEfficiency : NSDimension 
/*
 Base unit - litersPer100Kilometers
 */

+ (NSUnitFuelEfficiency *) litersPer100Kilometers;
+ (NSUnitFuelEfficiency *) milesPerImperialGallon;
+ (NSUnitFuelEfficiency *) milesPerGallon;

@end

GS_EXPORT_CLASS
@interface NSUnitLength : NSDimension 
/*
 Base unit - meters
 */

+ (NSUnitLength *) megameters;
+ (NSUnitLength *) kilometers;
+ (NSUnitLength *) hectometers;
+ (NSUnitLength *) decameters;
+ (NSUnitLength *) meters;
+ (NSUnitLength *) decimeters;
+ (NSUnitLength *) centimeters;
+ (NSUnitLength *) millimeters;
+ (NSUnitLength *) micrometers;
+ (NSUnitLength *) nanometers;
+ (NSUnitLength *) picometers;
+ (NSUnitLength *) inches;
+ (NSUnitLength *) feet;
+ (NSUnitLength *) yards;
+ (NSUnitLength *) miles;
+ (NSUnitLength *) scandinavianMiles;
+ (NSUnitLength *) lightyears;
+ (NSUnitLength *) nauticalMiles;
+ (NSUnitLength *) fathoms;
+ (NSUnitLength *) furlongs;
+ (NSUnitLength *) astronomicalUnits;
+ (NSUnitLength *) parsecs;

@end

GS_EXPORT_CLASS
@interface NSUnitIlluminance : NSDimension 
/*
 Base unit - lux
 */

+ (NSUnitIlluminance *) lux;

@end

GS_EXPORT_CLASS
@interface NSUnitMass : NSDimension 
/*
 Base unit - kilograms
 */

+ (NSUnitMass *) kilograms;
+ (NSUnitMass *) grams;
+ (NSUnitMass *) decigrams;
+ (NSUnitMass *) centigrams;
+ (NSUnitMass *) milligrams;
+ (NSUnitMass *) micrograms;
+ (NSUnitMass *) nanograms;
+ (NSUnitMass *) picograms;
+ (NSUnitMass *) ounces;
+ (NSUnitMass *) pounds;
+ (NSUnitMass *) stones;
+ (NSUnitMass *) metricTons;
+ (NSUnitMass *) shortTons;
+ (NSUnitMass *) carats;
+ (NSUnitMass *) ouncesTroy;
+ (NSUnitMass *) slugs;

@end

GS_EXPORT_CLASS
@interface NSUnitPower : NSDimension 
/*
 Base unit - watts
 */

+ (NSUnitPower *) terawatts;
+ (NSUnitPower *) gigawatts;
+ (NSUnitPower *) megawatts;
+ (NSUnitPower *) kilowatts;
+ (NSUnitPower *) watts;
+ (NSUnitPower *) milliwatts;
+ (NSUnitPower *) microwatts;
+ (NSUnitPower *) nanowatts;
+ (NSUnitPower *) picowatts;
+ (NSUnitPower *) femtowatts;
+ (NSUnitPower *) horsepower;

@end

GS_EXPORT_CLASS
@interface NSUnitPressure : NSDimension 
/*
 Base unit - newtonsPerMetersSquared (equivalent to 1 pascal)
 */

+ (NSUnitPressure *) newtonsPerMetersSquared;
+ (NSUnitPressure *) gigapascals;
+ (NSUnitPressure *) megapascals;
+ (NSUnitPressure *) kilopascals;
+ (NSUnitPressure *) hectopascals;
+ (NSUnitPressure *) inchesOfMercury;
+ (NSUnitPressure *) bars;
+ (NSUnitPressure *) millibars;
+ (NSUnitPressure *) millimetersOfMercury;
+ (NSUnitPressure *) poundsForcePerSquareInch;

@end

GS_EXPORT_CLASS
@interface NSUnitSpeed : NSDimension 
/*
 Base unit - metersPerSecond
 */

+ (NSUnitSpeed *) metersPerSecond;
+ (NSUnitSpeed *) kilometersPerHour;
+ (NSUnitSpeed *) milesPerHour;
+ (NSUnitSpeed *) knots;

@end

GS_EXPORT_CLASS
@interface NSUnitTemperature : NSDimension 
/*
 Base unit - kelvin
 */
+ (NSUnitTemperature *) kelvin;
+ (NSUnitTemperature *) celsius; 
+ (NSUnitTemperature *) fahrenheit;


@end

GS_EXPORT_CLASS
@interface NSUnitVolume : NSDimension 
/*
 Base unit - liters
 */

+ (NSUnitVolume *) megaliters;
+ (NSUnitVolume *) kiloliters;
+ (NSUnitVolume *) liters;
+ (NSUnitVolume *) deciliters;
+ (NSUnitVolume *) centiliters;
+ (NSUnitVolume *) milliliters;
+ (NSUnitVolume *) cubicKilometers;
+ (NSUnitVolume *) cubicMeters;
+ (NSUnitVolume *) cubicDecimeters;
+ (NSUnitVolume *) cubicCentimeters;
+ (NSUnitVolume *) cubicMillimeters;
+ (NSUnitVolume *) cubicInches;
+ (NSUnitVolume *) cubicFeet;
+ (NSUnitVolume *) cubicYards;
+ (NSUnitVolume *) cubicMiles;
+ (NSUnitVolume *) acreFeet;
+ (NSUnitVolume *) bushels;
+ (NSUnitVolume *) teaspoons;
+ (NSUnitVolume *) tablespoons;
+ (NSUnitVolume *) fluidOunces;
+ (NSUnitVolume *) cups;
+ (NSUnitVolume *) pints;
+ (NSUnitVolume *) quarts;
+ (NSUnitVolume *) gallons;
+ (NSUnitVolume *) imperialTeaspoons;
+ (NSUnitVolume *) imperialTablespoons;
+ (NSUnitVolume *) imperialFluidOunces;
+ (NSUnitVolume *) imperialPints;
+ (NSUnitVolume *) imperialQuarts;
+ (NSUnitVolume *) imperialGallons;
+ (NSUnitVolume *) metricCups;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSUnit_h_GNUSTEP_BASE_INCLUDE */

