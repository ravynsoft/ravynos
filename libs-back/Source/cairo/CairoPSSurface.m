/*
   Copyright (C) 2007 Free Software Foundation, Inc.

   Author: Fred Kiefer <fredkiefer@gmx.de>

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

#include "cairo/CairoPSSurface.h"
#include <cairo-ps.h>

@implementation CairoPSSurface

- (id) initWithDevice: (void *)device
{
  NSDictionary *info;
  NSString *path;

  info = (NSDictionary*)device;
  path = [info objectForKey: @"NSOutputFile"];

  // FIXME: Hard coded size in points
  size = NSMakeSize(400, 400);
  _surface = cairo_ps_surface_create([path fileSystemRepresentation], size.width, size.height);
  if (cairo_surface_status(_surface))
    {
      DESTROY(self);
    }

  return self;
}

- (NSSize) size
{
  return size;
}

- (void) setSize: (NSSize)newSize
{
  size = newSize;
  cairo_ps_surface_set_size(_surface, size.width, size.height);
}

- (void) writeComment: (NSString *)comment
{
  cairo_ps_surface_dsc_comment(_surface, [comment UTF8String]);
}

- (BOOL) isDrawingToScreen
{
  return NO;
}

@end
