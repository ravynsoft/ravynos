/*
   OpalContext.m

   Copyright (C) 2013 Free Software Foundation, Inc.

   Author: Ivan Vucica <ivan@vucica.net>
   Date: June 2013

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

#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSGraphics.h>
#import "opal/OpalContext.h"
#import "opal/OpalFontInfo.h"
#import "opal/OpalFontEnumerator.h"
#import "opal/OpalSurface.h"
#import "opal/OpalGState.h"
#include "config.h"

#define OGSTATE ((OpalGState *)gstate)

#if BUILD_SERVER == SERVER_x11
#  import "x11/XGServerWindow.h"
#  import "x11/XWindowBuffer.h"
#endif

@implementation OpalContext

+ (void) initializeBackend
{
  [NSGraphicsContext setDefaultContextClass: self];

  [GSFontEnumerator setDefaultClass: [OpalFontEnumerator class]];
  [GSFontInfo setDefaultClass: [OpalFontInfo class]];
}

+ (Class) GStateClass
{
  return [OpalGState class];
}

- (BOOL) supportsDrawGState
{
  return YES;
}

- (BOOL) isDrawingToScreen
{
#warning isDrawingToScreen returning NO to fix DPSimage
  return NO;

  // NOTE: This was returning NO because it was not looking at the
  // return value of GSCurrentSurface. Now it returns YES, which
  // seems to have broken image drawing (yellow rectangles are drawn instead)
  OpalSurface *surface;

  [OGSTATE GSCurrentSurface: &surface : NULL : NULL];

  return [surface isDrawingToScreen];
}

- (void) flushGraphics
{
  NSDebugLLog(@"OpalContext", @"%p (%@): %s", self, [self class], __PRETTY_FUNCTION__);
  OpalSurface *surface;

  [OGSTATE GSCurrentSurface: &surface : NULL : NULL];

  CGContextFlush([surface CGContext]);
  //[surface handleExposeRect: [surface size]];
}

/* Private backend methods */
/**
  This handles 'expose' event notifications that arrive from
  X11.
 */
+ (void) handleExposeRect: (NSRect)rect forDriver: (void *)driver
{
  if ([(id)driver isKindOfClass: [OpalSurface class]])
    {
      [(OpalSurface *)driver handleExposeRect: rect];
    }
}


#if BUILD_SERVER == SERVER_x11
#ifdef XSHM
+ (void) _gotShmCompletion: (Drawable)d
{
  [XWindowBuffer _gotShmCompletion: d];
}

- (void) gotShmCompletion: (Drawable)d
{
  [XWindowBuffer _gotShmCompletion: d];
}
#endif // XSHM
#endif // BUILD_SERVER = SERVER_x11

- (id) initWithGraphicsPort: (void *)port
                    flipped: (BOOL)flag;
{
  self = [super initWithGraphicsPort: port
                             flipped: flag];
  if (self != nil)
    {
      [self GSSetDevice: NULL : -1 : -1];
    }
  return self;
}

@end

@implementation OpalContext (Ops)

- (BOOL) isCompatibleBitmap: (NSBitmapImageRep*)bitmap
{
  NSString *colorSpaceName;

  if ([bitmap bitmapFormat] != 0)
    {
      return NO;
    }

  if ([bitmap isPlanar])
    {
      return NO;
    }

  if ([bitmap bitsPerSample] != 8)
    {
      return NO;
    }

  // FIXME: Allow more image types as soon as the Opal backend handles them correctly
  colorSpaceName = [bitmap colorSpaceName];
  if (![colorSpaceName isEqualToString: NSDeviceRGBColorSpace] &&
      ![colorSpaceName isEqualToString: NSCalibratedRGBColorSpace])
    {
      return NO;
    }
  else
    {
      return YES;
    }
}

- (void) GSCurrentDevice: (void **)device : (int *)x : (int *)y
{
  OpalSurface *surface;

  [OGSTATE GSCurrentSurface: &surface : x : y];
  if (device)
    {
      *device = [surface device];
    }
}

- (void) GSSetDevice: (void *)device
                    : (int)x
                    : (int)y
{
  OpalSurface *surface;

  /*
   * The "graphics port" associated to an OpalContext is necessarily a
   * CGContextRef supplied by the client to back the OpalContext, instead
   * of having us create the CGContextRef ourselves.
   *
   * Since -graphicsPort is overriden from NSGraphicsContext to compute the
   * CGContextRef for an OpalSurface (which is not initialized yet), we
   * get the _graphicsPort ivar directly to obtain the supplied CGContextRef
   * on initialization, and use that to init our surface.
   */
  CGContextRef suppliedContext = self->_graphicsPort;
  surface = [[OpalSurface alloc] initWithDevice: device
                                        context: suppliedContext];
  if (x == -1 && y == -1)
    {
      NSSize size = [surface size];
      x = 0;
      y = size.height;
    }

  [OGSTATE GSSetSurface: surface
                       : x
                       : y];

  [surface release];
}

- (void) DPSgsave
{
  [OGSTATE DPSgsave];
  [super DPSgsave];
}

- (void) DPSgrestore
{
  [super DPSgrestore];
  [OGSTATE DPSgrestore];
}

/** For information about this method, please see description of
    i-var '_opGState' in OpalGState.h.
 **/
- (void) DPSsetgstate: (int)gstateID
{
  OPGStateRef previousGState = OPContextCopyGState([OGSTATE CGContext]);
  [OGSTATE setOPGState: previousGState];
  [previousGState release]; // FIXME

  [super DPSsetgstate: gstateID];

  OPGStateRef newGState = [OGSTATE OPGState];
  if (newGState)
    {
      OPContextSetGState([OGSTATE CGContext], newGState);
      [OGSTATE setOPGState: nil];
    }
}

- (NSInteger) GSDefineGState
{
  // FIXME
  return [super GSDefineGState];
}

- (void *) graphicsPort
{
  OpalSurface * surface;

  [OGSTATE GSCurrentSurface: &surface : NULL : NULL];
  return [surface CGContext];
}

@end
