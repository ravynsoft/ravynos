/*
   Copyright (C) 2004 Free Software Foundation, Inc.

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

#include "ARTGState.h"

#ifndef RDS
#include "x11/XWindowBuffer.h"
#endif
#include "blit.h"

#include <math.h>

#include <Foundation/NSData.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSValue.h>
#include <AppKit/NSAffineTransform.h>
#include <AppKit/NSGraphics.h>


@implementation ARTGState (ReadRect)

-(NSDictionary *) GSReadRect: (NSRect)r
{
  NSMutableDictionary *md = [[NSMutableDictionary alloc] init];
  NSAffineTransform *matrix;

  int x0, y0, x1, y1, w, h, ox, oy;
  NSPoint p;

  /* Get the bounding rect in pixel coordinates. */
  p = r.origin;
  p = [ctm transformPoint: p];
  x0 = floor(p.x);  x1 = ceil(p.x);
  y0 = floor(p.y);  y1 = ceil(p.y);

#define CHECK do { \
  if (floor(p.x)<x0) x0=floor(p.x); \
  if (floor(p.y)<y0) y0=floor(p.y); \
  if (ceil(p.x)>x1) x1=ceil(p.x); \
  if (ceil(p.y)>y1) y1=ceil(p.y); } while (0)

  p = r.origin; p.x += r.size.width;
  p = [ctm transformPoint: p];
  CHECK;

  p = r.origin; p.x += r.size.width; p.y += r.size.height;
  p = [ctm transformPoint: p];
  CHECK;

  p = r.origin; p.y += r.size.height;
  p = [ctm transformPoint: p];
  CHECK;
#undef CHECK

  /* Clip to the window. */
  if (x0 < 0) x0 = 0;
  if (x1 < 0) x1 = 0;
  if (x0 > wi->sx) x0 = wi->sx;
  if (x1 > wi->sx) x1 = wi->sx;

  if (y0 < 0) y0 = 0;
  if (y1 < 0) y1 = 0;
  if (y0 > wi->sy) y0 = wi->sy;
  if (y1 > wi->sy) y1 = wi->sy;

  w = x1 - x0;
  h = y1 - y0;
  if (w <= 0 || h <= 0)
    w = h = 0;


  /* The matrix is the transform from user space to image space, and image
  space has its origin in the lower left corner if the image. Thus, we need
  to translate the ctm according to the position of the lower left corner
  in the window. */
  matrix=[ctm copy];
  [matrix translateXBy: -x0 yBy: -y0];

  ox = [matrix transformPoint: NSMakePoint(0, 0)].x - offset.x;
  oy = offset.y - [matrix transformPoint: NSMakePoint(0, 0)].y;

  [md setObject: NSDeviceRGBColorSpace  forKey: @"ColorSpace"];
  [md setObject: [NSNumber numberWithUnsignedInt: 1]  forKey: @"HasAlpha"];
  [md setObject: [NSNumber numberWithUnsignedInt: 8]  forKey: @"BitsPerSample"];
  [md setObject: [NSNumber numberWithUnsignedInt: 4]  forKey: @"SamplesPerPixel"];
  [md setObject: [NSValue valueWithSize: NSMakeSize(w, h)]  forKey: @"Size"];
  [md setObject: matrix  forKey: @"Matrix"];
  [matrix release];

  if (!w || !h)
    return [[md autorelease] makeImmutableCopyOnFail: YES];

  /* The rectangle isn't degenerate, so we need to actually copy some data. */
  {
    NSMutableData *d;
    int y;
    composite_run_t c;

    d = [[NSMutableData alloc] initWithLength: w * h * 4];

    c.dst = [d mutableBytes];

    c.src = wi->data + (oy - y1) * wi->bytes_per_line + (x0 + ox) * DI.bytes_per_pixel;
    c.srca = wi->alpha + (oy - y1) * wi->sx + (x0 + ox);

    for (y = 0; y < h; y++)
      {
	if (wi->has_alpha)
	  DI.read_pixels_a(&c, w);
	else
	  DI.read_pixels_o(&c, w);
	c.src += wi->bytes_per_line;
	c.srca += wi->sx;
	c.dst += w * 4;
      }

    [md setObject: [[d autorelease] makeImmutableCopyOnFail: YES]  forKey: @"Data"];
  }

  return [[md autorelease] makeImmutableCopyOnFail: YES];
}

@end

