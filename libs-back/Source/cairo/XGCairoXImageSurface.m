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

#include "config.h"
#include "x11/XGServer.h"
#include "x11/XGServerWindow.h"
#include "x11/XWindowBuffer.h"
#include "cairo/XGCairoXImageSurface.h"
#include "cairo/XGCairoSurface.h"

#define GSWINDEVICE ((gswindow_device_t *)gsDevice)

@implementation XGCairoXImageSurface

- (id) initWithDevice: (void *)device
{
  struct XWindowBuffer_depth_info_s di;

  gsDevice = device;
  if (GSWINDEVICE->type == NSBackingStoreNonretained)
    {
      // FIXME: This is a hack to get non-reatined backing store working.
      // I see no reason, why it isn't working, as the code is identical
      // to the one in the art backend.
      RELEASE(self);
      return (XGCairoXImageSurface*)[[XGCairoSurface alloc] initWithDevice: device];
    }

  di.drawing_depth = GSWINDEVICE->depth;
  // FIXME: The next lines may be wrong for depth <> 32.
  // But then art uses a depth of 24 for 32 bit modes. Strange!
  di.bytes_per_pixel = 4;
  di.inline_alpha = YES;
  di.inline_alpha_ofs = 0;
  /* The cairo image surface uses the client's byte order (cf. bug #28590). */
#if GS_WORDS_BIGENDIAN
  di.byte_order = MSBFirst;
#else
  di.byte_order = LSBFirst;
#endif

  ASSIGN(wi, [XWindowBuffer windowBufferForWindow: GSWINDEVICE depthInfo: &di]);

  _surface = cairo_image_surface_create_for_data((unsigned char*)wi->data, 
                                                 CAIRO_FORMAT_ARGB32, 
                                                 wi->sx, wi->sy, 
                                                 wi->bytes_per_line);
  
  return self;
}

- (void) dealloc
{
  DESTROY(wi);
  [super dealloc];
}

- (NSSize) size
{
  return GSWINDEVICE->xframe.size;
}

@end

