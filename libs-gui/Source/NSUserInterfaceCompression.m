/* Implementation of class NSSharingServicePickerToolbarItem
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Tue Apr  7 08:11:46 EDT 2020

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

#import <Foundation/NSObject.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSString.h>

#import "AppKit/NSUserInterfaceCompression.h"

@implementation NSUserInterfaceCompressionOptions

- (instancetype) initWithIdentifier: (NSString *)identifier
{
  return nil;
}

- (instancetype) initWithCompressionOptions: (NSSet *)opts
{
  return nil;
}

- (BOOL) containsOptions: (NSUserInterfaceCompressionOptions *)opts
{
  return NO;
}

- (BOOL) intersectsOptions: (NSUserInterfaceCompressionOptions *)opts
{
  return NO;
}

- (BOOL) isEmpty
{
  return NO;
}

- (NSUserInterfaceCompressionOptions *) optionsByAddingOptions: (NSUserInterfaceCompressionOptions *)opts
{
  return nil;
}

- (NSUserInterfaceCompressionOptions *) optionsByRemovingOptions: (NSUserInterfaceCompressionOptions *)opts
{
  return nil;
}

+ (NSUserInterfaceCompressionOptions *) hideImagesOption
{
  return nil;
}

+ (NSUserInterfaceCompressionOptions *) hideTextOption
{
  return nil;
}

+ (NSUserInterfaceCompressionOptions *) reduceMetricsOption
{
  return nil;
}

+ (NSUserInterfaceCompressionOptions *) breakEqualWidthsOption
{
  return nil;
}

+ (NSUserInterfaceCompressionOptions *) standardOptions
{
  return nil;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
}

- (id) initWithCoder: (NSCoder *)coder
{
  self = [super init];
  return self;
}

- (id) copyWithZone: (NSZone *)z
{
  return self;
}

@end

