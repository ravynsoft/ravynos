/*
   Copyright (C) 2002, 2005 Free Software Foundation, Inc.

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

#ifndef XWindowBuffer_h
#define XWindowBuffer_h

#include <config.h>

#ifdef XSHM
#include <X11/extensions/XShm.h>
#else
// FIXME
#define XShmSegmentInfo int
#endif

struct XWindowBuffer_depth_info_s
{
  /* The drawing depth according to X. Usually the number of bits of color
  data in each pixel. */
  int drawing_depth;

  /* The number of bytes used by a pixel. There is generally no
  relationship between this and drawing_depth, except that
  bytes_per_pixel*8>=drawing_depth. (Eg. 32-bit modes usually have a
  drawing depth of 24.) */
  int bytes_per_pixel;

  /* If alpha is to be stored inside the normal data, this should be YES.
  Otherwise, a separate buffer will be allocated for the alpha data (which
  never does any harm, but it wastes memory if there's enough space in a
  pixel, as in 32-bit modes. There needs to be 8 bits available for alpha
  data at a byte boundary for this to work, though. (This could be fixed,
  but in the mean time, just setting inline_alpha to NO is easier.)
  inline_alpha_ofs should be the offset to the byte in each pixel that
  holds the alpha value. */
  BOOL inline_alpha;
  int inline_alpha_ofs;

  /* The byte order used for the buffer. This must be either MSBFirst or
     LSBFirst. */
  int byte_order;
};

/*
XWindowBuffer maintains an XImage for a window. Each ARTGState that
renders to that window uses the same XWindowBuffer (and thus the same
buffer, etc.).

Many states might render to the same window, so we need to make sure
that there's only one XWindowBuffer for each window. */
@interface XWindowBuffer : NSObject
{
@public
  gswindow_device_t *window;

@private
  GC gc;
  Drawable drawable;
  XImage *ximage;
  Display *display;
  Pixmap pixmap;

  int use_shm;
  XShmSegmentInfo shminfo;


  struct XWindowBuffer_depth_info_s DI;


  /* While a XShmPutImage is in progress we don't try to call it
  again. The pending updates are stored here, and when we get the
  ShmCompletion event, we handle them. */
  int pending_put;     /* There are pending updates */
  struct
  {
    int x, y, w, h;
  } pending_rect; /* in this rectangle. */

  int pending_event;   /* We're waiting for the ShmCompletion event. */


  /* This is for the ugly shape-hack */
  unsigned char *old_shape;
  int old_shape_size;

@public
  unsigned char *data;
  int sx, sy;
  int bytes_per_line, bits_per_pixel, bytes_per_pixel;

  /* If has_alpha is 1 and alpha is NULL, the alpha is stored in data
  somehow. The drawing mechanism code should know how to deal with
  it. A separate alpha buffer will always be exactly the right size,
  so each row is sx bytes long.

  If has_alpha is 0, the window is assumed to be completely opaque.
  */
  unsigned char *alpha;
  int has_alpha;
}

/*
Returns a _retained_ XWindowBuffer for the specified gswindow_device_t.

The depth info is only used if a new XWindowBuffer needs to be allocated.
*/
+ windowBufferForWindow: (gswindow_device_t *)awindow
              depthInfo: (struct XWindowBuffer_depth_info_s *)aDI;

/*
Note that alpha is _not_ guaranteed to exist after this has been called;
you still need to check has_alpha. If the call fails, a message will be
logged.

(In ARTGState, I handle failures by simply ignoring the operation that
required alpha.)

The alpha channel is initialized to being completely opaque when first
created.
*/
-(void) needsAlpha;

-(void) _gotShmCompletion;
-(void) _exposeRect: (NSRect)r;
+(void) _gotShmCompletion: (Drawable)d;

@end


#endif

