/*
   Copyright (C) 2011 Free Software Foundation, Inc.

   Author: Eric Wasylishen <ewasylishen@gmail.com>

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

#include "config.h"
#include "x11/XGServer.h"
#include "x11/XGServerWindow.h"
#include "cairo/XGCairoModernSurface.h"
#include <cairo-xlib.h>

#define GSWINDEVICE ((gswindow_device_t *)gsDevice)

@implementation XGCairoModernSurface

- (id) initWithDevice: (void *)device
{
  cairo_surface_t *windowsurface;

  gsDevice = device;

  // Create a cairo xlib surface for the window
  {
    Display *dpy = GSWINDEVICE->display;
    Visual *visual;
    XWindowAttributes attributes;
    
    if (!XGetWindowAttributes(dpy, GSWINDEVICE->ident, &attributes))
      {
	visual = DefaultVisual(dpy, DefaultScreen(dpy));
      }
    else
      {
	visual = attributes.visual;
      }
    
    windowsurface = cairo_xlib_surface_create(GSWINDEVICE->display,
					      GSWINDEVICE->ident,
					      visual,
					      GSWINDEVICE->xframe.size.width,
					      GSWINDEVICE->xframe.size.height);
  }


  if (GSWINDEVICE->type == NSBackingStoreNonretained)
    {
      // Don't double-buffer:
      // use the window surface as the drawing destination.

      _surface = windowsurface;
    }
  else
    {
      // Do double-buffer:
      // Create a similar surface to the window which supports alpha

      // Ask XGServerWindow to call +[CairoContext handleExposeRect:forDriver:]
      // to let us handle the back buffer -> front buffer copy using cairo.
      GSWINDEVICE->gdriverProtocol |= GDriverHandlesExpose | GDriverHandlesBacking;
      GSWINDEVICE->gdriver = self;

      _windowSurface = windowsurface;
      _surface = cairo_surface_create_similar(windowsurface,
					      CAIRO_CONTENT_COLOR_ALPHA,
					      GSWINDEVICE->xframe.size.width,
					      GSWINDEVICE->xframe.size.height);

      /*NSLog(@"Window surface type %d, back buffer type %d, w %d h %d",
	(int) cairo_surface_get_type(_windowSurface),
	(int) cairo_surface_get_type(_surface),
	(int) GSWINDEVICE->xframe.size.width,
	(int) GSWINDEVICE->xframe.size.height);
      */
    }
  
  return self;
}

- (void) dealloc
{
  if (_windowSurface != NULL)
    {
      cairo_surface_destroy(_windowSurface);
    }
  [super dealloc];
}

- (NSSize) size
{
  return GSWINDEVICE->xframe.size;
}

- (void) handleExposeRect: (NSRect)rect
{
  cairo_t *windowCtx = cairo_create(_windowSurface);

  double backupOffsetX, backupOffsetY;

  // Temporairly cancel the device offset on the back buffer since
  // we want to work with raw X11 pixel coordinates

  cairo_surface_get_device_offset(_surface, &backupOffsetX, &backupOffsetY);
  cairo_surface_set_device_offset(_surface, 0, 0);

  // Copy the desired rectangle from the back buffer to the
  // front buffer

  // FIXME: should round
  cairo_rectangle(windowCtx, rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
  cairo_clip(windowCtx);
  cairo_set_source_surface(windowCtx, _surface, 0, 0);
  cairo_set_operator(windowCtx, CAIRO_OPERATOR_SOURCE);
  cairo_paint(windowCtx);

  // Debugging
  // cairo_reset_clip(windowCtx);
  // cairo_set_source_rgba(windowCtx, 1.0, 0.0, 0.0, 0.5);
  // cairo_rectangle(windowCtx, rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
  // cairo_stroke(windowCtx);
  //  NSLog(@"Exposing rect %@ with cairo. Status: '%s'", NSStringFromRect(rect), cairo_status_to_string(cairo_status(windowCtx)));

  cairo_destroy(windowCtx);
  
  // Restore device offset
  cairo_surface_set_device_offset(_surface, backupOffsetX, backupOffsetY);
}

@end

