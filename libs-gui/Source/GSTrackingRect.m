/* 
   GSTrackingRect.m

   Tracking rectangle class

   Copyright (C) 1996,1999 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
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

#include "config.h"
#import "GNUstepGUI/GSTrackingRect.h"

@implementation GSTrackingRect

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [GSTrackingRect class])
    {
      [self setVersion: 1];
    }
}

- (id) initWithRect: (NSRect)aRect
		tag: (NSTrackingRectTag)aTag
	      owner: (id)anObject
	   userData: (void *)theData
	     inside: (BOOL)flag
{
  rectangle = aRect;
  tag = aTag;
  owner = anObject;
  user_data = theData;
  flags.inside = flag;
  flags.isValid = YES;
  return self;
}

- (void) dealloc
{
  [super dealloc];
}

- (NSRect) rectangle
{
  return rectangle;
}

- (void) reset: (NSRect)aRect inside: (BOOL)flag
{
  rectangle = aRect;
  flags.inside = flag;
  flags.isValid = YES;
  flags.checked = NO;
}

- (NSTrackingRectTag) tag
{
  return tag;
}

- (id) owner
{
  return owner;
}

- (void *) userData
{
  return user_data;
}

- (BOOL) inside
{
  return flags.inside;
}

- (BOOL) isValid
{
  return flags.isValid;
}

- (void) invalidate
{
  if (flags.isValid)
    {
      flags.isValid = NO;
      flags.checked = NO;
    }
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding] == NO)
    {
      BOOL	inside = flags.inside;
      
      [aCoder encodeRect: rectangle];
      [aCoder encodeValueOfObjCType: @encode(NSTrackingRectTag) at: &tag];
      [aCoder encodeObject: owner];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &inside];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding] == NO)
    {
      BOOL	inside;
      
      rectangle = [aDecoder decodeRect];
      [aDecoder decodeValueOfObjCType: @encode(NSTrackingRectTag) at: &tag];
      owner = [aDecoder decodeObject];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &inside];
      flags.inside = inside;
    }
  return self;
}

@end
