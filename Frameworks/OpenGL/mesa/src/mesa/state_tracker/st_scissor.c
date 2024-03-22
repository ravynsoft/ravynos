/**************************************************************************
 *
 * Copyright 2016 Ilia Mirkin
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

#include "main/macros.h"
#include "main/mtypes.h"
#include "pipe/p_state.h"

#include "st_scissor.h"

void
st_window_rectangles_to_blit(const struct gl_context *ctx,
                             struct pipe_blit_info *blit)
{
   unsigned i;

   blit->num_window_rectangles = ctx->Scissor.NumWindowRects;
   blit->window_rectangle_include =
      ctx->Scissor.WindowRectMode == GL_INCLUSIVE_EXT;
   for (i = 0; i < blit->num_window_rectangles; i++) {
      const struct gl_scissor_rect *src_rect = &ctx->Scissor.WindowRects[i];
      struct pipe_scissor_state *dst_rect = &blit->window_rectangles[i];
      dst_rect->minx = MAX2(src_rect->X, 0);
      dst_rect->miny = MAX2(src_rect->Y, 0);
      dst_rect->maxx = MAX2(src_rect->X + src_rect->Width, 0);
      dst_rect->maxy = MAX2(src_rect->Y + src_rect->Height, 0);
   }
}
