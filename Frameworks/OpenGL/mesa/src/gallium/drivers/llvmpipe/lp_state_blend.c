/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
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

/**
 * @author Jose Fonseca <jfonseca@vmware.com>
 * @author Keith Whitwell <keithw@vmware.com>
 */

#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_dump.h"
#include "draw/draw_context.h"
#include "lp_screen.h"
#include "lp_context.h"
#include "lp_state.h"
#include "lp_debug.h"


static void *
llvmpipe_create_blend_state(struct pipe_context *pipe,
                            const struct pipe_blend_state *blend)
{
   struct pipe_blend_state *state = mem_dup(blend, sizeof *blend);

   if (LP_PERF & PERF_NO_BLEND) {
      state->independent_blend_enable = 0;
      for (unsigned i = 0; i < PIPE_MAX_COLOR_BUFS; i++)
         state->rt[i].blend_enable = 0;
   }

   return state;
}


static void
llvmpipe_bind_blend_state(struct pipe_context *pipe, void *blend)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   if (llvmpipe->blend == blend)
      return;

   draw_flush(llvmpipe->draw);

   llvmpipe->blend = blend;

   llvmpipe->dirty |= LP_NEW_BLEND;
}


static void
llvmpipe_delete_blend_state(struct pipe_context *pipe, void *blend)
{
   FREE(blend);
}


static void
llvmpipe_set_blend_color(struct pipe_context *pipe,
                         const struct pipe_blend_color *blend_color)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   if (!blend_color)
      return;

   if (memcmp(&llvmpipe->blend_color, blend_color, sizeof *blend_color) == 0)
      return;

   draw_flush(llvmpipe->draw);

   memcpy(&llvmpipe->blend_color, blend_color, sizeof *blend_color);

   llvmpipe->dirty |= LP_NEW_BLEND_COLOR;
}


/** XXX move someday?  Or consolidate all these simple state setters
 * into one file.
 */


static void *
llvmpipe_create_depth_stencil_state(struct pipe_context *pipe,
                                    const struct pipe_depth_stencil_alpha_state *depth_stencil)
{
   struct pipe_depth_stencil_alpha_state *state;

   state = mem_dup(depth_stencil, sizeof *depth_stencil);

   if (LP_PERF & PERF_NO_DEPTH) {
      state->depth_enabled = 0;
      state->depth_writemask = 0;
      state->stencil[0].enabled = 0;
      state->stencil[1].enabled = 0;
   }

   if (LP_PERF & PERF_NO_ALPHATEST) {
      state->alpha_enabled = 0;
   }

   return state;
}


static void
llvmpipe_bind_depth_stencil_state(struct pipe_context *pipe,
                                  void *depth_stencil)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   if (llvmpipe->depth_stencil == depth_stencil)
      return;

   draw_flush(llvmpipe->draw);

   llvmpipe->depth_stencil = depth_stencil;

   llvmpipe->dirty |= LP_NEW_DEPTH_STENCIL_ALPHA;
}


static void
llvmpipe_delete_depth_stencil_state(struct pipe_context *pipe, void *depth)
{
   FREE(depth);
}


static void
llvmpipe_set_stencil_ref(struct pipe_context *pipe,
                         const struct pipe_stencil_ref stencil_ref)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   if (memcmp(&llvmpipe->stencil_ref, &stencil_ref, sizeof stencil_ref) == 0)
      return;

   draw_flush(llvmpipe->draw);

   memcpy(&llvmpipe->stencil_ref, &stencil_ref, sizeof stencil_ref);

   /* not sure. want new flag? */
   llvmpipe->dirty |= LP_NEW_DEPTH_STENCIL_ALPHA;
}

static void
llvmpipe_set_sample_mask(struct pipe_context *pipe,
                         unsigned sample_mask)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   if (sample_mask != llvmpipe->sample_mask) {
      draw_flush(llvmpipe->draw);

      llvmpipe->sample_mask = sample_mask;

      llvmpipe->dirty |= LP_NEW_SAMPLE_MASK;
   }
}

static void
llvmpipe_set_min_samples(struct pipe_context *pipe,
                         unsigned min_samples)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   if (min_samples != llvmpipe->min_samples) {
      llvmpipe->min_samples = min_samples;

      llvmpipe->dirty |= LP_NEW_FS;
   }
}

void
llvmpipe_init_blend_funcs(struct llvmpipe_context *llvmpipe)
{
   llvmpipe->pipe.create_blend_state = llvmpipe_create_blend_state;
   llvmpipe->pipe.bind_blend_state   = llvmpipe_bind_blend_state;
   llvmpipe->pipe.delete_blend_state = llvmpipe_delete_blend_state;

   llvmpipe->pipe.create_depth_stencil_alpha_state = llvmpipe_create_depth_stencil_state;
   llvmpipe->pipe.bind_depth_stencil_alpha_state   = llvmpipe_bind_depth_stencil_state;
   llvmpipe->pipe.delete_depth_stencil_alpha_state = llvmpipe_delete_depth_stencil_state;

   llvmpipe->pipe.set_blend_color = llvmpipe_set_blend_color;

   llvmpipe->pipe.set_stencil_ref = llvmpipe_set_stencil_ref;
   llvmpipe->pipe.set_sample_mask = llvmpipe_set_sample_mask;
   llvmpipe->pipe.set_min_samples = llvmpipe_set_min_samples;

   llvmpipe->dirty |= LP_NEW_SAMPLE_MASK;
   llvmpipe->sample_mask = ~0;
}
