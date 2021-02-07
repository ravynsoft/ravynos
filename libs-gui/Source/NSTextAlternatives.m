/*
   NSTextAlternatives.m

   Select an alternative for a given text.

   Copyright (C) 2017 Free Software Foundation, Inc.

   Author: Daniel Ferreira <dtf@stanford.edu>
   Date: 2017

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#import <AppKit/NSTextAlternatives.h>
#import <Foundation/Foundation.h>

NSString *NSTextAlternativesSelectedAlternativeStringNotification =
  @"NSTextAlternativesSelectedAlternativeStringNotification";

@implementation NSTextAlternatives
- (id)initWithPrimaryString:(NSString *)primaryString
         alternativeStrings:(NSArray *)alternativeStrings
{
  if ((self = [super init]))
    {
      _primaryString = RETAIN(primaryString);
      _alternativeStrings = RETAIN(alternativeStrings);
    }

  return self;
}

- (NSString *)primaryString
{
  return [_primaryString copy];
}

- (NSArray *)alternativeStrings
{
  return [_alternativeStrings copy];
}

- (void)noteSelectedAlternativeString:(NSString *)alternativeString
{
  NSDictionary *dict =
    [NSDictionary dictionaryWithObject: alternativeString
                                forKey: @"NSAlternativeString"];

  [[NSNotificationCenter defaultCenter]
  postNotificationName: NSTextAlternativesSelectedAlternativeStringNotification
                object: self
              userInfo: dict];
}
- (void)dealloc
{
  RELEASE(_primaryString);
  RELEASE(_alternativeStrings);

  [super dealloc];
}
@end
