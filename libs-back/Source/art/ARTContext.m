/*
   Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   Author:  Alexander Malmberg <alexander@malmberg.org>

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


#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSUserDefaults.h>
#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSGraphics.h>

#import "ARTGState.h"
#import "blit.h"
#import "ftfont.h"

#ifndef RDS
#import "x11/XWindowBuffer.h"
#endif

@implementation ARTContext

+ (void)initializeBackend
{
  float gamma;

  NSDebugLLog(@"back-art",@"Initializing libart/freetype backend");

  [NSGraphicsContext setDefaultContextClass: [ARTContext class]];
  [FTFontInfo initializeBackend];

  gamma = [[NSUserDefaults standardUserDefaults]
              floatForKey: @"back-art-text-gamma"];
  artcontext_setup_gamma(gamma);
}

+ (Class) GStateClass
{
  return [ARTGState class];
}

- (void) setupDrawInfo: (void*)device
{
  int bpp;
  int red_mask, green_mask, blue_mask;
#ifdef RDS
  RDSServer *s = (RDSServer *)server;
  
  [s getPixelFormat: &bpp masks: &red_mask : &green_mask : &blue_mask];
#else
  gswindow_device_t *gs_win;

  gs_win = device;
  [(XGServer *)server getForScreen: gs_win->screen_id pixelFormat: &bpp 
                masks: &red_mask : &green_mask : &blue_mask];
#endif
  artcontext_setup_draw_info(&DI, red_mask, green_mask, blue_mask, bpp);
}

- (void) flushGraphics
{ 
  /* TODO: _really_ flush? (ie. force updates and wait for shm completion?) */
#ifndef RDS
  XFlush([(XGServer *)server xDisplay]);
#endif
}

#ifndef RDS
+ (void) _gotShmCompletion: (Drawable)d
{
  [XWindowBuffer _gotShmCompletion: d];
}

- (void) gotShmCompletion: (Drawable)d
{
  [XWindowBuffer _gotShmCompletion: d];
}
#endif

/* Private backend methods */
+ (void) handleExposeRect: (NSRect)rect forDriver: (void *)driver
{
  [(XWindowBuffer *)driver _exposeRect: rect];
}

- (BOOL) isCompatibleBitmap: (NSBitmapImageRep*)bitmap
{
  NSString *colorSpaceName;
  int numColors;

  if ([bitmap bitmapFormat] != 0)
    {
      return NO;
    }

  if (([bitmap bitsPerSample] > 8) &&
      ([bitmap bitsPerSample] != 16))
    {
      return NO;
    }

  numColors = [bitmap samplesPerPixel] - ([bitmap hasAlpha] ? 1 : 0);
  colorSpaceName = [bitmap colorSpaceName];
  if ([colorSpaceName isEqualToString: NSDeviceRGBColorSpace] ||
      [colorSpaceName isEqualToString: NSCalibratedRGBColorSpace])
    {
      return (numColors == 3);
    }
  else if ([colorSpaceName isEqualToString: NSDeviceCMYKColorSpace])
    {
      return (numColors == 4);
    }
  else if ([colorSpaceName isEqualToString: NSDeviceWhiteColorSpace] ||
      [colorSpaceName isEqualToString: NSCalibratedWhiteColorSpace])
    {
      return (numColors == 1);
    }
  else if ([colorSpaceName isEqualToString: NSDeviceBlackColorSpace] ||
      [colorSpaceName isEqualToString: NSCalibratedBlackColorSpace])
    {
      return (numColors == 1);
    }
  else
    {
      return NO;
    }
}

@end

@implementation ARTContext (ops)

- (void) GSSetDevice: (void*)device : (int)x : (int)y
{
  // Currently all windows share the same drawing info.
  // It is enough to initialize it once.
  // This will fail when different screen use different visuals.
  static BOOL serverInitialized = NO;

  if (!serverInitialized)
    {
      [self setupDrawInfo: device];
      serverInitialized = YES;
    }
  [(ARTGState *)gstate GSSetDevice: device : x : y];
}

- (void) GSCurrentDevice: (void **)device : (int *)x : (int *)y
{
  [(ARTGState *)gstate GSCurrentDevice: device : x : y];
}
@end
