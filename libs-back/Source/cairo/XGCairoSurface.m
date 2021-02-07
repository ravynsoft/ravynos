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

#include <AppKit/NSImage.h>
#include <AppKit/NSGraphics.h>
#include "x11/XGServer.h"
#include "x11/XGServerWindow.h"
#include "cairo/XGCairoSurface.h"
#include <cairo-xlib.h>

#define GSWINDEVICE ((gswindow_device_t *)gsDevice)

@implementation XGCairoSurface

- (id) initWithDevice: (void *)device
{
  Display *dpy;
  Drawable drawable;
  Visual* visual;
  XWindowAttributes attributes;

  gsDevice = device;

  dpy = GSWINDEVICE->display;
  if (GSWINDEVICE->type != NSBackingStoreNonretained)
    {
      drawable = GSWINDEVICE->buffer;
    }
  else
    {
      drawable = GSWINDEVICE->ident;
    }

  /*
    if (GSWINDEVICE->type != NSBackingStoreNonretained)
    {
    GSWINDEVICE->gdriverProtocol |= GDriverHandlesExpose;
    XSetWindowBackgroundPixmap(GSWINDEVICE->display,
    GSWINDEVICE->ident,
    GSWINDEVICE->buffer);
    }
  */

  if (!XGetWindowAttributes (dpy, GSWINDEVICE->ident, &attributes))
    {
      visual = DefaultVisual (dpy, DefaultScreen (dpy));
    }
  else
    {
      visual = attributes.visual;
    }

  _surface = cairo_xlib_surface_create(dpy,
			       drawable,
			       visual,
			       GSWINDEVICE->xframe.size.width,
			       GSWINDEVICE->xframe.size.height);
  
  return self;
}

- (NSSize) size
{
  return GSWINDEVICE->xframe.size;
}

@end


@implementation XGServer (ScreenCapture)

- (NSImage *) contentsOfScreen: (int)screen inRect: (NSRect)rect
{
  Window win;
  XWindowAttributes attrs;

  win = [self xDisplayRootWindow];
 
  if (XGetWindowAttributes(dpy, win, &attrs))
    {
      NSInteger width = rect.size.width;
      NSInteger height = rect.size.height;
      NSImage *result;
      NSBitmapImageRep *bmp;
      cairo_surface_t *src, *dest;

      // Convert rect to flipped coordinates
      rect.origin.y = attrs.height - NSMaxY(rect);

      bmp = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL
						     pixelsWide: width
						     pixelsHigh: height
						  bitsPerSample: 8
						samplesPerPixel: 4
						       hasAlpha: YES
						       isPlanar: NO
						 colorSpaceName: NSDeviceRGBColorSpace
						   bitmapFormat: 0
						    bytesPerRow: 0
						   bitsPerPixel: 32] autorelease];

      src = cairo_xlib_surface_create(dpy, win, attrs.visual, attrs.width, attrs.height);
      dest = cairo_image_surface_create_for_data([bmp bitmapData], CAIRO_FORMAT_ARGB32, width, height, [bmp bytesPerRow]);
      
      {
	cairo_t *cr = cairo_create(dest);
	cairo_set_source_surface(cr, src, -1 * rect.origin.x, -1 * rect.origin.y);
	cairo_paint(cr);
	cairo_destroy(cr);
      }

      cairo_surface_destroy(src);
      cairo_surface_destroy(dest);

      // Convert BGRA to RGBA
      {
	NSInteger stride;
	NSInteger x, y;
	unsigned char *cdata;

	stride = [bmp bytesPerRow];
	cdata = [bmp bitmapData];

	for (y = 0; y < height; y++)
	  {
	    for (x = 0; x < width; x++)
	      {
		NSInteger i = (y * stride) + (x * 4);
		unsigned char d = cdata[i];
	    
#if GS_WORDS_BIGENDIAN
		cdata[i] = cdata[i + 1];
		cdata[i + 1] = cdata[i + 2];
		cdata[i + 2] = cdata[i + 3];
		cdata[i + 3] = d;
#else
		cdata[i] = cdata[i + 2];
		//cdata[i + 1] = cdata[i + 1];
		cdata[i + 2] = d;
		//cdata[i + 3] = cdata[i + 3];
#endif 
	      }
	  }
      }

      result = [[[NSImage alloc] initWithSize: NSMakeSize(width, height)] autorelease];
      [result addRepresentation: bmp];
      return result;
    }
  
  return nil;
}

@end
