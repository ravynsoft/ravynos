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

 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  */
 

#include "main/macros.h"
#include "main/framebuffer.h"
#include "st_context.h"
#include "pipe/p_context.h"
#include "st_atom.h"
#include "st_util.h"


/**
 * Scissor depends on the scissor box, and the framebuffer dimensions.
 */
void
st_update_scissor( struct st_context *st )
{
   struct pipe_scissor_state scissor[PIPE_MAX_VIEWPORTS];
   const struct gl_context *ctx = st->ctx;
   const struct gl_framebuffer *fb = ctx->DrawBuffer;
   const unsigned int fb_width = _mesa_geometric_width(fb);
   const unsigned int fb_height = _mesa_geometric_height(fb);
   GLint miny, maxy;
   unsigned i;
   bool changed = false;

   if (!ctx->Scissor.EnableFlags)
      return;

   for (i = 0 ; i < st->state.num_viewports; i++) {
      scissor[i].minx = 0;
      scissor[i].miny = 0;
      scissor[i].maxx = fb_width;
      scissor[i].maxy = fb_height;

      if (ctx->Scissor.EnableFlags & (1 << i)) {
         /* need to be careful here with xmax or ymax < 0 */
         GLint xmax = MAX2(0, ctx->Scissor.ScissorArray[i].X + ctx->Scissor.ScissorArray[i].Width);
         GLint ymax = MAX2(0, ctx->Scissor.ScissorArray[i].Y + ctx->Scissor.ScissorArray[i].Height);

         if (ctx->Scissor.ScissorArray[i].X > (GLint)scissor[i].minx)
            scissor[i].minx = ctx->Scissor.ScissorArray[i].X;
         if (ctx->Scissor.ScissorArray[i].Y > (GLint)scissor[i].miny)
            scissor[i].miny = ctx->Scissor.ScissorArray[i].Y;

         if (xmax < (GLint) scissor[i].maxx)
            scissor[i].maxx = xmax;
         if (ymax < (GLint) scissor[i].maxy)
            scissor[i].maxy = ymax;
         
         /* check for null space */
         if (scissor[i].minx >= scissor[i].maxx || scissor[i].miny >= scissor[i].maxy)
            scissor[i].minx = scissor[i].miny = scissor[i].maxx = scissor[i].maxy = 0;
      }

      /* Now invert Y if needed.
       * Gallium drivers use the convention Y=0=top for surfaces.
       */
      if (st->state.fb_orientation == Y_0_TOP) {
         miny = fb->Height - scissor[i].maxy;
         maxy = fb->Height - scissor[i].miny;
         scissor[i].miny = miny;
         scissor[i].maxy = maxy;
      }

      if (memcmp(&scissor[i], &st->state.scissor[i], sizeof(scissor[0])) != 0) {
         /* state has changed */
         st->state.scissor[i] = scissor[i];  /* struct copy */
         changed = true;
      }
   }

   if (changed) {
      struct pipe_context *pipe = st->pipe;

      pipe->set_scissor_states(pipe, 0, st->state.num_viewports, scissor);
   }
}

void
st_update_window_rectangles(struct st_context *st)
{
   struct pipe_scissor_state new_rects[PIPE_MAX_WINDOW_RECTANGLES];
   const struct gl_context *ctx = st->ctx;
   const struct gl_scissor_attrib *scissor = &ctx->Scissor;
   unsigned i;
   bool changed = false;
   unsigned num_rects = scissor->NumWindowRects;
   bool include = scissor->WindowRectMode == GL_INCLUSIVE_EXT;

   if (ctx->DrawBuffer == ctx->WinSysDrawBuffer) {
      num_rects = 0;
      include = false;
   }
   for (i = 0; i < num_rects; i++) {
      const struct gl_scissor_rect *rect = &scissor->WindowRects[i];
      new_rects[i].minx = MAX2(rect->X, 0);
      new_rects[i].miny = MAX2(rect->Y, 0);
      new_rects[i].maxx = MAX2(rect->X + rect->Width, 0);
      new_rects[i].maxy = MAX2(rect->Y + rect->Height, 0);
   }
   if (num_rects > 0 && memcmp(new_rects, st->state.window_rects.rects,
                               num_rects * sizeof(struct pipe_scissor_state))) {
      memcpy(st->state.window_rects.rects, new_rects,
             num_rects * sizeof(struct pipe_scissor_state));
      changed = true;
   }
   if (st->state.window_rects.num != num_rects) {
      st->state.window_rects.num = num_rects;
      changed = true;
   }
   if (st->state.window_rects.include != include) {
      st->state.window_rects.include = include;
      changed = true;
   }
   if (changed)
      st->pipe->set_window_rectangles(
            st->pipe, include, num_rects, new_rects);
}
