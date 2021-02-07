/* Implementation of class NSDataAsset
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Fri Jan 17 10:25:34 EST 2020

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

#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>
#import <Foundation/NSData.h>
#import <AppKit/NSDataAsset.h>

@implementation NSDataAsset

// Initializing the Data Asset
- (instancetype) initWithName: (NSDataAssetName)name
{
  return [self initWithName: name bundle: nil];
}

- (instancetype) initWithName: (NSDataAssetName)name bundle: (NSBundle *)bundle
{
  self = [super init];
  if (self != nil)
    {
      ASSIGNCOPY(_name, name);
      ASSIGN(_bundle, bundle);
      _data = nil;
      _typeIdentifier = nil;
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_name);
  RELEASE(_bundle);
  RELEASE(_data);
  RELEASE(_typeIdentifier);
  [super dealloc];
}

- (id) copyWithZone: (NSZone *)zone
{
  NSDataAsset *copy = [[NSDataAsset allocWithZone: zone] initWithName: _name bundle: _bundle];
  ASSIGNCOPY(copy->_data, _data);
  ASSIGNCOPY(copy->_typeIdentifier, _typeIdentifier);
  return copy;
}

// Accessing data...
- (NSData *) data
{
  return _data;
}

// Getting data asset information
- (NSDataAssetName) name
{
  return _name;
}

- (NSString *) typeIdentifier
{
  return _typeIdentifier;
}
@end

