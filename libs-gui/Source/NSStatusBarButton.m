/* Implementation of class NSStatusBarButton
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: 31-07-2020

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

#import "AppKit/NSStatusBarButton.h"

@implementation NSStatusBarButton

- (BOOL) appearsDisabled
{
  return _appearsDisabled;
}

- (void) setAppearsDisabled: (BOOL)flag
{
  _appearsDisabled = flag;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      [coder encodeBool: _appearsDisabled
                 forKey: @"NSAppearsDisabled"];
    }
  else
    {
      [coder encodeValueOfObjCType: @encode(BOOL)
                                at: &_appearsDisabled];
    }
}

- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super initWithCoder: coder];
  if (self != nil)
    {
      if ([coder allowsKeyedCoding])
        {
          if ([coder containsValueForKey: @"NSAppearsDisabled"])
            {
              _appearsDisabled = [coder decodeBoolForKey: @"NSAppearsDisabled"];
            }
        }
      else
        {
          [coder decodeValueOfObjCType: @encode(BOOL)
                                    at: &_appearsDisabled];
        }
    }
  return self;
}
@end

