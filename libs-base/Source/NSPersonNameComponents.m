/* Definition of class NSPersonNameComponents
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   Implemented by: Gregory Casamento <greg.casamento@gmail.com>
   Date: Sep 2019

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

#import "common.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSString.h"
#import "Foundation/NSPersonNameComponents.h"

@implementation NSPersonNameComponents 

- (instancetype) init
{
  if((self = [super init]) != nil)
    {
      _namePrefix = nil;
      _givenName = nil;
      _middleName = nil;
      _familyName = nil;
      _nameSuffix = nil;
      _nickname = nil;
      _phoneticRepresentation = nil;
    }
  return self;
}

- (instancetype) initWithCoder: (NSCoder *)aCoder
{
  if((self = [self init]) != nil)
    {
      if([aCoder allowsKeyedCoding])
        {
          _namePrefix = [aCoder decodeObjectForKey: @"namePrefix"];
          _givenName =  [aCoder decodeObjectForKey: @"givenName"];
          _middleName = [aCoder decodeObjectForKey: @"middleName"];
          _familyName = [aCoder decodeObjectForKey: @"familyName"];
          _nameSuffix = [aCoder decodeObjectForKey: @"nameSuffix"];
          _nickname =   [aCoder decodeObjectForKey: @"nickname"];
          _phoneticRepresentation = [aCoder decodeObjectForKey: @"phoneticRepresentation"];
        }
      else
        {
          _namePrefix = [aCoder decodeObject];
          _givenName =  [aCoder decodeObject];
          _middleName = [aCoder decodeObject];
          _familyName = [aCoder decodeObject];
          _nameSuffix = [aCoder decodeObject];
          _nickname =   [aCoder decodeObject];
          _phoneticRepresentation = [aCoder decodeObject];
        }
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)aCoder
{
  if([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: _namePrefix
                    forKey: @"namePrefix"];
      [aCoder encodeObject: _givenName
                    forKey: @"givenName"];
      [aCoder encodeObject: _middleName
                    forKey: @"middleName"];
      [aCoder encodeObject: _familyName
                    forKey: @"familyName"];
      [aCoder encodeObject: _nameSuffix
                    forKey: @"nameSuffix"];
      [aCoder encodeObject: _nameSuffix
                    forKey: @"nameSuffix"];
      [aCoder encodeObject: _phoneticRepresentation
                    forKey: @"phoneticRepresentation"];
    }
  else
    {
      [aCoder encodeObject: _namePrefix];
      [aCoder encodeObject: _givenName];
      [aCoder encodeObject: _middleName];
      [aCoder encodeObject: _familyName];
      [aCoder encodeObject: _nameSuffix];
      [aCoder encodeObject: _nameSuffix];
      [aCoder encodeObject: _phoneticRepresentation];
    }
}

- (instancetype) copyWithZone: (NSZone *)zone
{
  NSPersonNameComponents *copy =
    [[NSPersonNameComponents allocWithZone: zone] init];
  
  [copy setNamePrefix: [self namePrefix]];
  [copy setGivenName: [self givenName]];
  [copy setMiddleName: [self middleName]];
  [copy setFamilyName: [self familyName]];
  [copy setNameSuffix: [self nameSuffix]];
  [copy setNickname: [self nickname]];
  [copy setPhoneticRepresentation: [self phoneticRepresentation]];

  return copy;
}

- (void) dealloc
{
  RELEASE(_namePrefix);
  RELEASE(_givenName);
  RELEASE(_middleName);
  RELEASE(_familyName);
  RELEASE(_nameSuffix);
  RELEASE(_nickname);
  RELEASE(_phoneticRepresentation);
  [super dealloc];
}

- (NSString *) namePrefix
{
  return _namePrefix;
}

- (void) setNamePrefix: (NSString *)namePrefix
{
  ASSIGNCOPY(_namePrefix, namePrefix);
}

- (NSString *) givenName
{
  return _givenName;
}

- (void) setGivenName: (NSString *)givenName
{
  ASSIGNCOPY(_givenName, givenName);
}

- (NSString *) middleName
{
  return _middleName;
}

- (void) setMiddleName: (NSString *)middleName
{
  ASSIGNCOPY(_middleName, middleName);
}

- (NSString *) familyName
{
  return _familyName;  
}

- (void) setFamilyName: (NSString *)familyName
{
  ASSIGNCOPY(_familyName, familyName);
}

- (NSString *) nameSuffix
{
  return _nameSuffix;  
}

- (void) setNameSuffix: (NSString *)nameSuffix
{
  ASSIGNCOPY(_nameSuffix, nameSuffix);
}

- (NSString *) nickname
{
  return _nickname;  
}

- (void) setNickname: (NSString *)nickname
{
  ASSIGNCOPY(_nickname, nickname);
}

- (NSPersonNameComponents *) phoneticRepresentation
{
  return _phoneticRepresentation;
}

- (void) setPhoneticRepresentation: (NSPersonNameComponents *)pr
{
  ASSIGNCOPY(_phoneticRepresentation, pr);
}

@end


