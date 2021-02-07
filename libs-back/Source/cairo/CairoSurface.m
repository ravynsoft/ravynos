/*
   Copyright (C) 2004 Free Software Foundation, Inc.

   Author: Banlu Kemiyatorn <object at gmail dot com>

   This file is part of GNUstep.

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

#include "cairo/CairoSurface.h"

@implementation CairoSurface 

- (id) initWithDevice: (void *) device
{
  /* TODO FIXME make a more abstract struct for the device */
  [self subclassResponsibility:_cmd];

  return self;
}

- (void) dealloc
{
  //NSLog(@"CairoSurface dealloc");
  if (_surface != NULL)
    {
      cairo_surface_destroy(_surface);
    }
  [super dealloc];
}

- (NSString *) description
{
  return [NSString stringWithFormat:@"<%@ %p xr:%p>", [self class], self, _surface];
}

-(NSSize) size
{
  [self subclassResponsibility:_cmd];
  return NSMakeSize(0, 0);
}

- (void) setSize: (NSSize)newSize
{
  [self subclassResponsibility:_cmd];
}

- (cairo_surface_t *) surface
{
  return _surface;
}

- (void) handleExposeRect: (NSRect)rect
{
}

- (BOOL) isDrawingToScreen
{
  return YES;
}

@end
