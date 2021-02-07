/*
   NSShadow.m

   GUI implementation of a shadow effect.

   Copyright (C) 2009 Free Software Foundation, Inc.

   Author: Eric Wasylishen <ewasylishen@gmail.com>
   Date: Dec 2009

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#import <Foundation/NSString.h>
#import "AppKit/NSShadow.h"
#import "AppKit/NSColor.h"

@implementation NSShadow

- (id) init
{
  if ((self = [super init]))
    {
      _offset = NSMakeSize(0,0);
      _radius = 0;
       ASSIGN(_color, [[NSColor blackColor] colorWithAlphaComponent: 0.333]);
    }
  return self;
}

- (id) copyWithZone: (NSZone*)zone
{
  NSShadow *s = (NSShadow*)NSCopyObject(self, 0, zone);
  RETAIN(s->_color);
  return s;
}

- (void) dealloc
{
  DESTROY(_color);
  [super dealloc];
}

- (NSString *) description
{
  return [NSString stringWithFormat: @"NSShadow {%f, %f} blur = %f color = %@",
	(float)_offset.width, (float)_offset.height, (float)_radius,
	[_color description]];
}

- (NSSize) shadowOffset
{
  return _offset;
}

- (void) setShadowOffset: (NSSize)offset
{
  _offset = offset;
}

- (CGFloat) shadowBlurRadius
{
  return _radius;
}

- (void) setShadowBlurRadius: (CGFloat)radius
{
  _radius = radius;
}

- (NSColor *) shadowColor
{
  return _color;
}

- (void) setShadowColor: (NSColor *)color
{
  ASSIGN(_color, color);
}

- (void) set
{
  // FIXME: Implement
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeFloat: _radius forKey: @"NSShadowBlurRadius"];
      [aCoder encodeFloat: _offset.width forKey: @"NSShadowHoriz"];
      [aCoder encodeFloat: _offset.height forKey: @"NSShadowVert"];
      [aCoder encodeObject: _color forKey: @"NSShadowColor"];
    }
  else
    {
      float radius = _radius;
      [aCoder encodeValueOfObjCType: @encode(float) at: &radius];
      [aCoder encodeSize: _offset];
      [aCoder encodeObject: _color];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super init];
  if (self == nil)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      _radius = [aDecoder decodeFloatForKey: @"NSShadowBlurRadius"];
      _offset = NSMakeSize([aDecoder decodeFloatForKey:  @"NSShadowHoriz"],
                           [aDecoder decodeFloatForKey:  @"NSShadowVert"]);
      _color = [[aDecoder decodeObjectForKey: @"NSShadowColor"] retain];
    }
  else
    {
      float radius;
      [aDecoder decodeValueOfObjCType: @encode(float) at: &radius];
      _radius = radius;
      _offset = [aDecoder decodeSize];
      _color = [[aDecoder decodeObject] retain];
    }
  return self;
}

@end
