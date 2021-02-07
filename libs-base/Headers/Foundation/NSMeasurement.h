/* Definition of class NSMeasurement
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

#ifndef _NSMeasurement_h_GNUSTEP_BASE_INCLUDE
#define _NSMeasurement_h_GNUSTEP_BASE_INCLUDE

#import <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSUnit;

GS_EXPORT_CLASS
@interface NSMeasurement : NSObject <NSCopying, NSCoding>
{
  NSUnit *_unit;
  double _doubleValue;
}
  
// Creating Measurements
- (instancetype) initWithDoubleValue: (double)doubleValue 
                                unit: (NSUnit *)unit;


// Accessing unit and value
- (NSUnit *) unit;

- (double) doubleValue;

// Conversion
- (BOOL) canBeConvertedToUnit: (NSUnit *)unit;

- (NSMeasurement *) measurementByConvertingToUnit: (NSUnit *)unit;

// Operating
- (NSMeasurement *) measurementByAddingMeasurement: (NSMeasurement *)measurement;

- (NSMeasurement *) measurementBySubtractingMeasurement: (NSMeasurement *)measurement;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSMeasurement_h_GNUSTEP_BASE_INCLUDE */

