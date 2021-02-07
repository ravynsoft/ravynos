/* Definition of class NSLengthFormatter
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento <greg.casamento@gmail.com>
   Date: Tue Oct  8 13:30:33 EDT 2019

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

#ifndef _NSLengthFormatter_h_GNUSTEP_BASE_INCLUDE
#define _NSLengthFormatter_h_GNUSTEP_BASE_INCLUDE

#import <Foundation/NSFormatter.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

enum {
  NSLengthFormatterUnitMillimeter = 8,
  NSLengthFormatterUnitCentimeter = 9,
  NSLengthFormatterUnitMeter = 11,
  NSLengthFormatterUnitKilometer = 14,
  NSLengthFormatterUnitInch = (5 << 8) + 1,
  NSLengthFormatterUnitFoot = (5 << 8) + 2,
  NSLengthFormatterUnitYard = (5 << 8) + 3,
  NSLengthFormatterUnitMile = (5 << 8) + 4,
}; 
typedef NSInteger NSLengthFormatterUnit;

@class NSNumberFormatter, NSString;

GS_EXPORT_CLASS
@interface NSLengthFormatter : NSFormatter
{
  BOOL _isForPersonHeightUse;
  NSNumberFormatter *_numberFormatter;
  NSFormattingUnitStyle _unitStyle;
}

- (NSNumberFormatter *) numberFormatter;
- (void) setNumberFormatter: (NSNumberFormatter *)formatter;

- (NSFormattingUnitStyle) unitStyle;
- (void) setUnitStyle: (NSFormattingUnitStyle)style;
  
- (BOOL) isForPersonHeightUse;
- (void) setForPersonHeightUse: (BOOL)flag;
  
- (NSString *) stringFromValue: (double)value unit: (NSLengthFormatterUnit)unit;

- (NSString *) stringFromMeters: (double)numberInMeters;

- (NSString *) unitStringFromValue: (double)value unit: (NSLengthFormatterUnit)unit;

- (NSString *) unitStringFromMeters: (double)numberInMeters usedUnit: (NSLengthFormatterUnit *)unit;

- (BOOL) getObjectValue: (id *)obj forString: (NSString *)string errorDescription: (NSString **)error;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSLengthFormatter_h_GNUSTEP_BASE_INCLUDE */

