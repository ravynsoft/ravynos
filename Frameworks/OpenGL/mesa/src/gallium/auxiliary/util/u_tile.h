/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

#ifndef P_TILE_H
#define P_TILE_H

#include "util/compiler.h"
#include "util/format/u_formats.h"
#include "pipe/p_state.h"

struct pipe_context;
struct pipe_transfer;

/**
 * Clip tile against transfer dims.
 *
 * XXX: this only clips width and height!
 *
 * \return TRUE if tile is totally clipped, FALSE otherwise
 */
static inline bool
u_clip_tile(unsigned x, unsigned y, unsigned *w, unsigned *h,
            const struct pipe_box *box)
{
   if ((int) x >= box->width)
      return true;
   if ((int) y >= box->height)
      return true;
   if ((int) (x + *w) > box->width)
      *w = box->width - x;
   if ((int) (y + *h) > box->height)
      *h = box->height - y;
   return false;
}

#ifdef __cplusplus
extern "C" {
#endif

void
pipe_get_tile_raw(struct pipe_transfer *pt,
                  const void *src,
                  unsigned x, unsigned y,
                  unsigned w, unsigned h,
                  void *p, int dst_stride);

void
pipe_put_tile_raw(struct pipe_transfer *pt,
                  void *dst,
                  unsigned x, unsigned y,
                  unsigned w, unsigned h,
                  const void *p, int src_stride);


void
pipe_get_tile_rgba(struct pipe_transfer *pt,
                   const void *src,
                   unsigned x, unsigned y,
                   unsigned w, unsigned h,
                   enum pipe_format format,
                   void *dst);

void
pipe_put_tile_rgba(struct pipe_transfer *pt,
                   void *dst,
                   unsigned x, unsigned y,
                   unsigned w, unsigned h,
                   enum pipe_format format,
                   const void *src);

#ifdef __cplusplus
}
#endif

#endif
