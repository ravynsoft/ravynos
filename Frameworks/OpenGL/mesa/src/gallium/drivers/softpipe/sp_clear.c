/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 * Copyright 2009 VMware, Inc.  All Rights Reserved.
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

/* Author:
 *    Brian Paul
 *    Michel Dänzer
 */


#include "pipe/p_defines.h"
#include "util/u_pack_color.h"
#include "util/u_surface.h"
#include "sp_clear.h"
#include "sp_context.h"
#include "sp_screen.h"
#include "sp_query.h"
#include "sp_tile_cache.h"


/**
 * Clear the given buffers to the specified values.
 * No masking, no scissor (clear entire buffer).
 */
void
softpipe_clear(struct pipe_context *pipe, unsigned buffers,
               const struct pipe_scissor_state *scissor_state,
               const union pipe_color_union *color,
               double depth, unsigned stencil)
{
   struct softpipe_context *softpipe = softpipe_context(pipe);
   struct pipe_surface *zsbuf = softpipe->framebuffer.zsbuf;
   unsigned zs_buffers = buffers & PIPE_CLEAR_DEPTHSTENCIL;
   uint64_t cv;
   uint i;

   if (unlikely(sp_debug & SP_DBG_NO_RAST))
      return;

   if (!softpipe_check_render_cond(softpipe))
      return;

#if 0
   softpipe_update_derived(softpipe, MESA_PRIM_TRIANGLES); /* not needed?? */
#endif

   if (buffers & PIPE_CLEAR_COLOR) {
      for (i = 0; i < softpipe->framebuffer.nr_cbufs; i++) {
         if (buffers & (PIPE_CLEAR_COLOR0 << i))
            sp_tile_cache_clear(softpipe->cbuf_cache[i], color, 0);
      }
   }

   if (zs_buffers &&
       util_format_is_depth_and_stencil(zsbuf->texture->format) &&
       zs_buffers != PIPE_CLEAR_DEPTHSTENCIL) {
      /* Clearing only depth or stencil in a combined depth-stencil buffer. */
      util_clear_depth_stencil(pipe, zsbuf, zs_buffers, depth, stencil,
                               0, 0, zsbuf->width, zsbuf->height);
   }
   else if (zs_buffers) {
      static const union pipe_color_union zero;

      cv = util_pack64_z_stencil(zsbuf->format, depth, stencil);
      sp_tile_cache_clear(softpipe->zsbuf_cache, &zero, cv);
   }

   softpipe->dirty_render_cache = true;
}
