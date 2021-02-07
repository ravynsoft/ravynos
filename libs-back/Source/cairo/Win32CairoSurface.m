/*
   Win32CairoSurface.m

   Copyright (C) 2008 Free Software Foundation, Inc.

   Author: Xavier Glattard <xavier.glattard@online.fr>
   Based on the work of:
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

#include "cairo/Win32CairoSurface.h"
#include "win32/WIN32Geometry.h"
#include "win32/WIN32Server.h"
#include <cairo-win32.h>

#define GSWINDEVICE ((HWND)gsDevice)

@implementation Win32CairoSurface

- (id) initWithDevice: (void *)device
{
  if (self)
    {
      WIN_INTERN *win = (WIN_INTERN *)GetWindowLong((HWND)device, GWL_USERDATA);
      HDC         hDC = GetDC((HWND)device);

      // Save/set initial state...
      gsDevice = device;
      _surface = NULL;
  
      if (hDC == NULL)
	{
	  NSWarnMLog(@"Win32CairoSurface line: %d : no device context", __LINE__);
      
	  // And deallocate ourselves...
	  DESTROY(self);
	}
      else
	{
	  // Create the cairo surfaces...
	  // NSBackingStoreRetained works like Buffered since 10.5 (See apple docs)...
	  // NOTE: According to Apple docs NSBackingStoreBuffered should be the only one
	  //       ever used anymore....others are NOT recommended...
	  if (win && (win->type == NSBackingStoreNonretained))
	    {
	      // This is the raw DC surface...
	      _surface = cairo_win32_surface_create(hDC);

	      // Check for error...
	      if (cairo_surface_status(_surface) != CAIRO_STATUS_SUCCESS)
		{
		  // Output the surface create error...
		  cairo_status_t status = cairo_surface_status(_surface);
		  NSWarnMLog(@"surface create FAILED - status: %s\n", cairo_status_to_string(status));
              
		  // Destroy the initial surface created...
		  cairo_surface_destroy(_surface);
              
		  // And deallocate ourselves...
		  DESTROY(self);

		  // Release the device context...
                  ReleaseDC(device, hDC);
		}
	    }
	  else
	    {
	      NSSize csize = [self size];
          
	      // This is the raw DC surface...
	      cairo_surface_t *window = cairo_win32_surface_create(hDC);

	      // Check for error...
	      if (cairo_surface_status(window) != CAIRO_STATUS_SUCCESS)
		{
		  // Output the surface create error...
		  cairo_status_t status = cairo_surface_status(window);
		  NSWarnMLog(@"surface create FAILED - status: %s\n",  cairo_status_to_string(status));
                  
		  // And deallocate ourselves...
		  DESTROY(self);
		}
	      else
		{
		  // and this is the in-memory DC surface...surface owns its DC...
		  // NOTE: For some reason we get an init sequence with zero width/height,
		  //       which creates problems in the cairo layer.  It tries to clear
		  //       the 'similar' surface it's creating, and with a zero width/height
		  //       it incorrectly thinks the clear failed...so we will init with
		  //       a minimum size of 1 for width/height...
		  _surface = cairo_surface_create_similar(window, CAIRO_CONTENT_COLOR_ALPHA,
							  MAX(1, csize.width),
							  MAX(1, csize.height));
		  cairo_status_t status = cairo_surface_status(_surface);

		  // Check for error...
		  if (status != CAIRO_STATUS_SUCCESS)
		    {
		      // Output the surface create error...
		      NSWarnMLog(@"surface create FAILED - status: %s\n",  cairo_status_to_string(status));
                  
		      // Destroy the surface created...
		      cairo_surface_destroy(_surface);
                  
		      // And deallocate ourselves...
		      DESTROY(self);
		    }
		}
          
	      // Destroy the initial surface created...
	      cairo_surface_destroy(window);

	      // Release the device context...
	      ReleaseDC((HWND)device, hDC);
	    }
          
	  if (win && self)
	    {
	      // We need this for handleExposeEvent in WIN32Server...
	      win->surface = (void*)self;
	    }
	}
    }

  return self;
}

- (void) dealloc
{
  // After further testing and monitoring USER/GDI object counts found
  // that releasing the HDC is redundant and unnecessary...
  [super dealloc];
}

- (NSString*) description
{
  NSMutableString *description = AUTORELEASE([[super description] mutableCopy]);
  HDC shdc = NULL;

  if (_surface)
    {
      shdc = cairo_win32_surface_get_dc(_surface);
    }
  [description appendFormat: @" size: %@",NSStringFromSize([self size])];
  [description appendFormat: @" _surface: %p",_surface];
  [description appendFormat: @" surfDC: %p",shdc];
  return AUTORELEASE(description);
}

- (NSSize) size
{
  RECT csize;

  GetClientRect(GSWINDEVICE, &csize);
  return NSMakeSize(csize.right - csize.left, csize.bottom - csize.top);
}

- (void) setSize: (NSSize)newSize
{
  NSDebugMLLog(@"Win32CairoSurface", @"size: %@\n", NSStringFromSize(newSize));
}

- (void) handleExposeRect: (NSRect)rect
{
  // If the surface is buffered then we will have been invoked...
  HDC hdc = GetDC(GSWINDEVICE);
  
  // Make sure we got a HDC...
  if (hdc == NULL)
    {
      NSWarnMLog(@"ERROR: cannot get a HDC for surface: %@\n", self);
    }
  else
    {
      // Create a cairo surface for the hDC...
      cairo_surface_t *window = cairo_win32_surface_create(hdc);
      
      // If the surface is buffered then...
      if (window == NULL)
        {
          NSWarnMLog(@"ERROR: cannot create cairo surface for: %@\n", self);
        }
      else
        {
          // First check the current status of the foreground surface...
          if (cairo_surface_status(window) != CAIRO_STATUS_SUCCESS)
            {
              NSWarnMLog(@"cairo initial window error status: %s\n",
                         cairo_status_to_string(cairo_surface_status(window)));
            }
          else
            {
              cairo_t *context = cairo_create(window);

              if (cairo_status(context) != CAIRO_STATUS_SUCCESS)
                {
                  NSWarnMLog(@"cairo context create error - status: _surface: %s window: %s windowCtxt: %s (%d)",
                             cairo_status_to_string(cairo_surface_status(_surface)),
                             cairo_status_to_string(cairo_surface_status(window)),
                             cairo_status_to_string(cairo_status(context)), cairo_get_reference_count(context));
                }
              else
                {
                  double  backupOffsetX = 0;
                  double  backupOffsetY = 0;
                  RECT    msRect        = GSWindowRectToMS((WIN32Server*)GSCurrentServer(), GSWINDEVICE, rect);

                  // Need to save the device offset context...
                  cairo_surface_get_device_offset(_surface, &backupOffsetX, &backupOffsetY);
                  cairo_surface_set_device_offset(_surface, 0, 0);

                  cairo_rectangle(context, msRect.left, msRect.top, rect.size.width, rect.size.height);
                  cairo_clip(context);
                  cairo_set_source_surface(context, _surface, 0, 0);
                  cairo_set_operator(context, CAIRO_OPERATOR_SOURCE);
                  cairo_paint(context);
                  
                  if (cairo_status(context) != CAIRO_STATUS_SUCCESS)
                    {
                      NSWarnMLog(@"cairo expose error - status: _surface: %s window: %s windowCtxt: %s",
                                 cairo_status_to_string(cairo_surface_status(_surface)),
                                 cairo_status_to_string(cairo_surface_status(window)),
                                 cairo_status_to_string(cairo_status(context)));
                    }

                  // Cleanup...
                  cairo_destroy(context);

                  // Restore device offset
                  cairo_surface_set_device_offset(_surface, backupOffsetX, backupOffsetY);
                }
            }
          
          // Destroy the surface...
          cairo_surface_destroy(window);
        }

      // Release the aquired HDC...
      ReleaseDC(GSWINDEVICE, hdc);
    }
}

@end

@implementation WIN32Server (ScreenCapture)

- (NSImage *) contentsOfScreen: (int)screen inRect: (NSRect)rect
{
  NSImage *result = nil;
  HDC hdc = [self createHdcForScreen: screen];

  // We need a screen device context for this to work...
  if (hdc == NULL)
    {
      NSWarnMLog(@"invalid screen request: %d", screen);
    }
  else
    {
      // Convert rect to flipped coordinates
      NSRect boundsForScreen = [self boundsForScreen: screen];
      NSInteger width = rect.size.width;
      NSInteger height = rect.size.height;
      NSBitmapImageRep *bmp;
      cairo_surface_t *src, *dst;

      rect.origin.y = boundsForScreen.size.height - NSMaxY(rect);
      
      // Create a bitmap representation for capturing the screen area...
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
	
      // Create the required surfaces...
      src = cairo_win32_surface_create(hdc);
      dst = cairo_image_surface_create_for_data([bmp bitmapData],
                                                CAIRO_FORMAT_ARGB32,
                                                width, height,
                                                [bmp bytesPerRow]);

      // Ensure we were able to generate the required surfaces...
      if (cairo_surface_status(src) != CAIRO_STATUS_SUCCESS)
 	{
          NSWarnMLog(@"cairo screen surface error status: %s\n",
                     cairo_status_to_string(cairo_surface_status(src)));
 	}
      else if (cairo_surface_status(dst) != CAIRO_STATUS_SUCCESS)
 	{
          NSWarnMLog(@"cairo screen surface error status: %s\n",
                     cairo_status_to_string(cairo_surface_status(dst)));
          cairo_surface_destroy(src);
 	}
      else
 	{
          // Capture the requested screen rectangle...
          cairo_t *cr = cairo_create(dst);
          cairo_set_source_surface(cr, src, -1 * rect.origin.x, -1 * rect.origin.y);
          cairo_paint(cr);
          cairo_destroy(cr);

          // Cleanup the cairo surfaces...
          cairo_surface_destroy(src);
          cairo_surface_destroy(dst);
          [self deleteScreenHdc: hdc];
	
          // Convert BGRA to RGBA
          // Original code located in XGCairSurface.m
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
                    cdata[i + 0] = cdata[i + 1];
                    cdata[i + 1] = cdata[i + 2];
                    cdata[i + 2] = cdata[i + 3];
                    cdata[i + 3] = d;
#else
                    cdata[i + 0] = cdata[i + 2];
                    //cdata[i + 1] = cdata[i + 1];
                    cdata[i + 2] = d;
                    //cdata[i + 3] = cdata[i + 3];
#endif
                  }
              }
          }

          // Create the image and add the bitmap representation...
          result = [[[NSImage alloc] initWithSize: NSMakeSize(width, height)] autorelease];
          [result addRepresentation: bmp];
 	}
    }

  // Return whatever we got...
  return result;
}

@end
