
/* Definition of class NSMassFormatter
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

#ifndef _NSMassFormatter_h_GNUSTEP_BASE_INCLUDE
#define _NSMassFormatter_h_GNUSTEP_BASE_INCLUDE

#import <Foundation/NSFormatter.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

enum {
    NSMassFormatterUnitGram = 11,
    NSMassFormatterUnitKilogram = 14,
    NSMassFormatterUnitOunce = (6 << 8) + 1,
    NSMassFormatterUnitPound = (6 << 8) + 2,
    NSMassFormatterUnitStone = (6 << 8) + 3,
};  
typedef NSInteger NSMassFormatterUnit;  

@class NSNumberFormatter;

GS_EXPORT_CLASS
@interface NSMassFormatter : NSObject
{
  NSNumberFormatter *_numberFormatter;
  BOOL _isForPersonMassUse;
  NSFormattingUnitStyle _unitStyle;
}

- (NSNumberFormatter *) numberFormatter;
- (void) setNumberFormatter: (NSNumberFormatter *)formatter;
  
- (NSFormattingUnitStyle) unitStyle;
- (void) setUnitStyle: (NSFormattingUnitStyle)style;

- (BOOL) isForPersonMassUse;
- (void) setForPersonMassUse: (BOOL)flag;
  
- (NSString *)stringFromValue: (double)value unit: (NSMassFormatterUnit)unit;

- (NSString *)stringFromKilograms: (double)numberInKilograms;

- (NSString *)unitStringFromValue: (double)value unit: (NSMassFormatterUnit)unit;

- (NSString *)unitStringFromKilograms: (double)numberInKilograms usedUnit: (NSMassFormatterUnit *)unitp;

- (BOOL) getObjectValue: (id*)obj forString: (NSString *)string errorDescription: (NSString **)error;
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSMassFormatter_h_GNUSTEP_BASE_INCLUDE */

