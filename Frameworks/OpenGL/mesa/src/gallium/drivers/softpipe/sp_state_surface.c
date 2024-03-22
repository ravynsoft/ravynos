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

/* Authors:  Keith Whitwell <keithw@vmware.com>
 */

#include "sp_context.h"
#include "sp_state.h"
#include "sp_tile_cache.h"

#include "draw/draw_context.h"

#include "util/format/u_format.h"
#include "util/u_inlines.h"


/**
 * XXX this might get moved someday
 * Set the framebuffer surface info: color buffers, zbuffer, stencil buffer.
 * Here, we flush the old surfaces and update the tile cache to point to the new
 * surfaces.
 */
void
softpipe_set_framebuffer_state(struct pipe_context *pipe,
                               const struct pipe_framebuffer_state *fb)
{
   struct softpipe_context *sp = softpipe_context(pipe);
   uint i;

   draw_flush(sp->draw);

   for (i = 0; i < PIPE_MAX_COLOR_BUFS; i++) {
      struct pipe_surface *cb = i < fb->nr_cbufs ? fb->cbufs[i] : NULL;

      /* check if changing cbuf */
      if (sp->framebuffer.cbufs[i] != cb) {
         /* flush old */
         sp_flush_tile_cache(sp->cbuf_cache[i]);

         /* assign new */
         pipe_surface_reference(&sp->framebuffer.cbufs[i], cb);

         /* update cache */
         sp_tile_cache_set_surface(sp->cbuf_cache[i], cb);
      }
   }

   sp->framebuffer.nr_cbufs = fb->nr_cbufs;

   /* zbuf changing? */
   if (sp->framebuffer.zsbuf != fb->zsbuf) {
      /* flush old */
      sp_flush_tile_cache(sp->zsbuf_cache);

      /* assign new */
      pipe_surface_reference(&sp->framebuffer.zsbuf, fb->zsbuf);

      /* update cache */
      sp_tile_cache_set_surface(sp->zsbuf_cache, fb->zsbuf);

      /* Tell draw module how deep the Z/depth buffer is
       *
       * If no depth buffer is bound, send the utility function the
       * format for no bound depth (PIPE_FORMAT_NONE).
       */
      draw_set_zs_format(sp->draw,
                         (sp->framebuffer.zsbuf) ?
                            sp->framebuffer.zsbuf->format : PIPE_FORMAT_NONE);
   }

   sp->framebuffer.width = fb->width;
   sp->framebuffer.height = fb->height;
   sp->framebuffer.samples = fb->samples;
   sp->framebuffer.layers = fb->layers;

   sp->dirty |= SP_NEW_FRAMEBUFFER | SP_NEW_TEXTURE;
}
