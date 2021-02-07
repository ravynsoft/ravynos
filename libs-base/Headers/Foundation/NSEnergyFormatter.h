/* Definition of class NSEnergyFormatter
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

#ifndef _NSEnergyFormatter_h_GNUSTEP_BASE_INCLUDE
#define _NSEnergyFormatter_h_GNUSTEP_BASE_INCLUDE

#import <Foundation/NSFormatter.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSNumberFormatter, NSString;  
  
enum {
    NSEnergyFormatterUnitJoule = 11,
    NSEnergyFormatterUnitKilojoule = 14,
    NSEnergyFormatterUnitCalorie = (7 << 8) + 1,       
    NSEnergyFormatterUnitKilocalorie = (7 << 8) + 2,   
};
typedef NSInteger NSEnergyFormatterUnit;

GS_EXPORT_CLASS
@interface NSEnergyFormatter : NSFormatter
{
  BOOL _isForFoodEnergyUse;
  NSNumberFormatter *_numberFormatter;
  NSFormattingUnitStyle _unitStyle;
}

- (NSNumberFormatter *) numberFormatter;
- (void) setNumberFormatter: (NSNumberFormatter *)formatter;
  
- (NSFormattingUnitStyle) unitStyle;
- (void) setUnitStyle: (NSFormattingUnitStyle)style;
  
- (BOOL) isForFoodEnergyUse;
- (void) setForFoodEnergyUse: (BOOL)flag;
  
- (NSString *) stringFromValue: (double)value unit: (NSEnergyFormatterUnit)unit;

- (NSString *) stringFromJoules: (double)numberInJoules;

- (NSString *) unitStringFromValue: (double)value unit: (NSEnergyFormatterUnit)unit;

- (NSString *) unitStringFromJoules: (double)numberInJoules usedUnit: (NSEnergyFormatterUnit *)unitp;

- (BOOL) getObjectValue:(id *)obj forString: (NSString *)string errorDescription: (NSString **)error;
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSEnergyFormatter_h_GNUSTEP_BASE_INCLUDE */

