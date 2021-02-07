/* Implementation of class NSAccessibilityCustomRotor
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Mon 15 Jun 2020 03:18:59 AM EDT

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import "AppKit/NSAccessibilityCustomRotor.h"

@implementation NSAccessibilityCustomRotor
  
- (instancetype) initWithLabel: (NSString *)label
            itemSearchDelegate: (id<NSAccessibilityCustomRotorItemSearchDelegate>)delegate
{
  return nil;
}

- (instancetype) initWithRotorType: (NSAccessibilityCustomRotorType)rotorType
                itemSearchDelegate: (id<NSAccessibilityCustomRotorItemSearchDelegate>)delegate
{
  return nil;
}

- (NSAccessibilityCustomRotorType) type
{
  return 0;
}

- (void) setType: (NSAccessibilityCustomRotorType)type
{
}

- (NSString *) label
{
  return nil;
}

- (void) setLabel: (NSString *)label
{
}

- (id<NSAccessibilityCustomRotorItemSearchDelegate>) itemSearchDelegate
{
  return nil;
}

- (void) setItemSearchDelegate: (id<NSAccessibilityCustomRotorItemSearchDelegate>) delegate
{
}

- (id<NSAccessibilityElementLoading>) itemLoadingDelegate
{
  return nil;
}

- (void) setItemLoadingDelegate: (id<NSAccessibilityElementLoading>) delegate
{
}
  
@end

// Results...
@implementation NSAccessibilityCustomRotorItemResult : NSObject

- (instancetype)initWithTargetElement:(id<NSAccessibilityElement>)targetElement
{
  return nil;
}

- (instancetype)initWithItemLoadingToken: (id<NSAccessibilityLoadingToken>)token
                             customLabel: (NSString *)customLabel
{
  return nil;
}

- (id<NSAccessibilityElement>) targetElement;
{
  return nil;
}

- (id<NSAccessibilityLoadingToken>) itemLoadingToken
{
  return nil;
}

- (NSRange) targetRange
{
  return NSMakeRange(0,NSNotFound);
}

- (NSString *) customLabel
{
  return nil;
}

@end

