/*
   <title>NSNibConnector</title>

   <abstract>Implementation of NSNibConnector</abstract>

   Copyright (C) 1999, 2015 Free Software Foundation, Inc.

   Author:  Richard Frith-Macdonald <richard@branstorm.co.uk>
   Date: 1999
   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: August 2015

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

#import <Foundation/NSCoder.h>
#import <Foundation/NSString.h>
#import "AppKit/NSNibConnector.h"

@implementation	NSNibConnector

- (void) dealloc
{
  RELEASE(_src);
  RELEASE(_dst);
  RELEASE(_tag);
  [super dealloc];
}

- (id) destination
{
  return _dst;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      if (_src != nil)
	{
	  [aCoder encodeObject: _src forKey: @"NSSource"];
	}
      if (_dst != nil)
	{
	  [aCoder encodeObject: _dst forKey: @"NSDestination"];
	}
      if (_tag != nil)
	{
	  [aCoder encodeObject: _tag forKey: @"NSLabel"];
	}
    }
  else
    {
      [aCoder encodeObject: _src];
      [aCoder encodeObject: _dst];
      [aCoder encodeObject: _tag];
    }
}

- (void) establishConnection
{
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      if ([aDecoder containsValueForKey: @"NSDestination"])
	{
	  ASSIGN(_dst, [aDecoder decodeObjectForKey: @"NSDestination"]);
	}
      if ([aDecoder containsValueForKey: @"NSSource"])
	{
	  ASSIGN(_src, [aDecoder decodeObjectForKey: @"NSSource"]);
	}
      if ([aDecoder containsValueForKey: @"NSLabel"])
	{
	  ASSIGN(_tag, [aDecoder decodeObjectForKey: @"NSLabel"]);
	}
    }
  else
    {
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_src];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_dst];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_tag];
    }
  return self;
}

- (BOOL) isEqual: (id)object
{
  BOOL result = NO;

  if([object isKindOfClass: [NSNibConnector class]] == NO)
    {
      return NO;
    }

  if(self == object)
    {
      result = YES;
    }
  else if([[self source] isEqual: [object source]] &&
	  [[self destination] isEqual: [object destination]] &&
	  [[self label] isEqual: [object label]] &&
	  ([self class] == [object class]))
    {
      result = YES;
    }
  return result;
}

- (NSString*) label
{
  return _tag;
}

- (void) replaceObject: (id)anObject withObject: (id)anotherObject
{
  if (_src == anObject)
    {
      ASSIGN(_src, anotherObject);
    }
  if (_dst == anObject)
    {
      ASSIGN(_dst, anotherObject);
    }
  if (_tag == anObject)
    {
      ASSIGN(_tag, anotherObject);
    }
}

- (id) source
{
  return _src;
}

- (void) setDestination: (id)anObject
{
  ASSIGN(_dst, anObject);
}

- (void) setLabel: (NSString*)label
{
  ASSIGN(_tag, label);
}

- (void) setSource: (id)anObject
{
  ASSIGN(_src, anObject);
}

- (NSString *)description
{
  return [NSString stringWithFormat: @"<%@ src=%@ dst=%@ label=%@>",
                   [super description],
                   [self source],
                   [self destination],
                   [self label]];
}

@end
