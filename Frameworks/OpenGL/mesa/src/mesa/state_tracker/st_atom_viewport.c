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


#include "main/context.h"
#include "main/viewport.h"
#include "main/framebuffer.h"
#include "st_context.h"
#include "st_atom.h"
#include "pipe/p_context.h"
#include "cso_cache/cso_context.h"

static enum pipe_viewport_swizzle
viewport_swizzle_from_glenum(GLenum16 swizzle)
{
   return swizzle - GL_VIEWPORT_SWIZZLE_POSITIVE_X_NV;
}

/**
 * Update the viewport transformation matrix.  Depends on:
 *  - viewport pos/size
 *  - depthrange
 *  - window pos/size or FBO size
 */
void
st_update_viewport( struct st_context *st )
{
   struct gl_context *ctx = st->ctx;
   unsigned i;

   /* _NEW_VIEWPORT 
    */
   for (i = 0; i < st->state.num_viewports; i++) {
      float *scale = st->state.viewport[i].scale;
      float *translate = st->state.viewport[i].translate;

      _mesa_get_viewport_xform(ctx, i, scale, translate);

      /* _NEW_BUFFERS */
      /* Drawing to a window where the coordinate system is upside down. */
      if (st->state.fb_orientation == Y_0_TOP) {
         scale[1] *= -1;
         translate[1] = st->state.fb_height - translate[1];
      }

      st->state.viewport[i].swizzle_x = viewport_swizzle_from_glenum(ctx->ViewportArray[i].SwizzleX);
      st->state.viewport[i].swizzle_y = viewport_swizzle_from_glenum(ctx->ViewportArray[i].SwizzleY);
      st->state.viewport[i].swizzle_z = viewport_swizzle_from_glenum(ctx->ViewportArray[i].SwizzleZ);
      st->state.viewport[i].swizzle_w = viewport_swizzle_from_glenum(ctx->ViewportArray[i].SwizzleW);
   }

   cso_set_viewport(st->cso_context, &st->state.viewport[0]);

   if (st->state.num_viewports > 1) {
      struct pipe_context *pipe = st->pipe;

      pipe->set_viewport_states(pipe, 1, st->state.num_viewports - 1,
                                &st->state.viewport[1]);
   }
}
