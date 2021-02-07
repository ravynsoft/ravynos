/*
   Copyright (C) 2002 Free Software Foundation, Inc.

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

#ifndef CairoSurface_h
#define CairoSurface_h

#include <cairo.h>
#include <Foundation/Foundation.h>

@interface CairoSurface : NSObject
{
@public
  void *gsDevice;
  cairo_surface_t *_surface;
}

- (id) initWithDevice: (void *)device;

- (NSSize) size;
- (void) setSize: (NSSize)newSize;

- (cairo_surface_t *) surface;

- (void) handleExposeRect: (NSRect)rect;

- (BOOL) isDrawingToScreen;

@end

#endif

