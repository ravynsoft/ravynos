/*
   Win32CairoGlitzSurface.m

   Copyright (C) 2008 Free Software Foundation, Inc.

   Author: Xavier Glattard <xavier.glattard@online.fr>
   Based on the work of:
     Alexander Malmberg <alexander@malmberg.org>
     Banlu Kemiyatorn <object at gmail dot com>

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

#include "cairo/Win32CairoGlitzSurface.h"
#include <cairo-glitz.h>
#include <glitz-wgl.h>

#define GSWINDEVICE ((HWND)gsDevice)

@implementation Win32CairoGlitzSurface

+ (void) initialize
{
  NSLog(@"Win32CairoGlitzSurface : %d : glitz_wgl_init", __LINE__);
  glitz_wgl_init(NULL);
}

- (id) initWithDevice: (void *)device
{
  WIN_INTERN *win;

  unsigned long mask = 0;
  glitz_drawable_format_t dtempl;

  //static
  glitz_drawable_format_t *dformat = NULL;
  glitz_drawable_t *drawable = NULL;
  glitz_format_t *format = NULL;
  glitz_surface_t *surface = NULL;
  RECT sz;

  gsDevice = device;
  GetClientRect(GSWINDEVICE, &sz);
  win = (WIN_INTERN *)GetWindowLong(GSWINDEVICE, GWL_USERDATA);
  if (win && win->useHDC)
    {
      HGDIOBJ old;

      old = SelectObject(win->hdc, win->old);
      DeleteObject(old);
      DeleteDC(win->hdc);
      win->hdc = NULL;
      win->old = NULL;
      win->useHDC = NO;
    }

  //NSLog(@"Win32CairoGlitzSurface : init window %d (%d,%d-%d,%d)",GSWINDEVICE,sz.left,sz.top,sz.right,sz.bottom);

  //if(!dformat)
  {
  dtempl.samples = 1;
  mask |= GLITZ_FORMAT_SAMPLES_MASK;
  dtempl.color.red_size = 8;
  dtempl.color.green_size = 8;
  dtempl.color.blue_size = 8;
  dtempl.color.alpha_size = 8;
  dtempl.color.fourcc = GLITZ_FOURCC_RGB;
  mask |= GLITZ_FORMAT_FOURCC_MASK
        | GLITZ_FORMAT_RED_SIZE_MASK
        | GLITZ_FORMAT_GREEN_SIZE_MASK
        | GLITZ_FORMAT_BLUE_SIZE_MASK
        | GLITZ_FORMAT_ALPHA_SIZE_MASK;

  dtempl.doublebuffer = 0;
  mask |= GLITZ_FORMAT_DOUBLEBUFFER_MASK;

  dformat = glitz_wgl_find_window_format(mask, &dtempl, 0);
  if (!dformat)
    {
      NSLog(@"Win32CairoGlitzSurface : %d : no format for drawable",__LINE__);
      exit(1);
    }

/*	NSLog (@"ID:%d RGBA:%d/%d/%d/%d D:%d St:%d DB:%d S:%d",
	    (int) dformat->id,
		dformat->color.red_size,
		dformat->color.green_size,
		dformat->color.blue_size,
		dformat->color.alpha_size,
		dformat->depth_size,
		dformat->stencil_size,
		dformat->doublebuffer,
		dformat->samples
	);
*/
  }
  
  drawable = glitz_wgl_create_drawable_for_window(
		 dformat,
		 GSWINDEVICE,
		 sz.right - sz.left,
		 sz.bottom - sz.top);
  if (!drawable)
    {
      NSLog(@"Win32CairoGlitzSurface : %d : no glitz drawable",__LINE__);
      exit(1);
    }
    
  format = glitz_find_standard_format(drawable, GLITZ_STANDARD_ARGB32);
  if (!format)
  {
    NSLog(@"Win32CairoGlitzSurface : %d (%d) : couldn't find ARGB32 surface format",__LINE__,GSWINDEVICE);
    exit(1);
  }

  surface = glitz_surface_create(
		drawable, format,
		sz.right - sz.left,
		sz.bottom - sz.top,
		0, NULL);
  if (!surface)
    {
      NSLog(@"Win32CairoGlitzSurface : %d : couldn't create glitz surface",__LINE__);
      exit(1);
    }

  glitz_surface_attach(surface, drawable, GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);
  _surface = cairo_glitz_surface_create(surface);
  if (cairo_surface_status(_surface))
    {
      DESTROY(self);
    }

  return self;
}

- (NSSize) size
{
  RECT sz;

  GetClientRect(GSWINDEVICE, &sz);
  return NSMakeSize(sz.right - sz.left, sz.bottom - sz.top);
}

@end

