/*
   Win32CairoGState.m

   Copyright (C) 2003 Free Software Foundation, Inc.

   August 8, 2012
 
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

#include "cairo/Win32CairoGState.h"
#include "cairo/CairoSurface.h"
#include <cairo-win32.h>

@implementation Win32CairoGState 

+ (void) initialize
{
  if (self == [Win32CairoGState class])
    {
    }
}

- (void) dealloc
{
  DESTROY(_lastPath);
  [super dealloc];
}

- (id)copyWithZone: (NSZone *)zone
{
  Win32CairoGState *object = [super copyWithZone: zone];
  object->_lastPath = [_lastPath copy];
  return object;
}

- (HDC) getHDC
{
  if (_surface)
    {
      cairo_surface_flush([_surface surface]);
      NSDebugLLog(@"CairoGState",
                  @"%s:_surface: %p hdc: %p\n", __PRETTY_FUNCTION__,
                  _surface, cairo_win32_surface_get_dc([_surface surface]));
      return cairo_win32_surface_get_dc([_surface surface]);
    }
  NSLog(@"%s:_surface is NULL\n", __PRETTY_FUNCTION__);
  return NULL;
}

- (void) releaseHDC: (HDC)hdc
{
  if (hdc && _surface)
    cairo_surface_mark_dirty([_surface surface]);
}

@end
