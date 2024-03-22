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
#include "lp_context.h"
#include "lp_state.h"
#include "draw/draw_context.h"


static void
llvmpipe_set_clip_state(struct pipe_context *pipe,
                        const struct pipe_clip_state *clip)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   /* pass the clip state to the draw module */
   draw_set_clip_state(llvmpipe->draw, clip);
}


static void
llvmpipe_set_viewport_states(struct pipe_context *pipe,
                             unsigned start_slot,
                             unsigned num_viewports,
                             const struct pipe_viewport_state *viewports)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   /* pass the viewport info to the draw module */
   draw_set_viewport_states(llvmpipe->draw, start_slot, num_viewports,
                            viewports);

   memcpy(llvmpipe->viewports + start_slot, viewports,
          sizeof(struct pipe_viewport_state) * num_viewports);
   llvmpipe->dirty |= LP_NEW_VIEWPORT;
}


static void
llvmpipe_set_scissor_states(struct pipe_context *pipe,
                            unsigned start_slot,
                            unsigned num_scissors,
                            const struct pipe_scissor_state *scissors)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   draw_flush(llvmpipe->draw);

   assert(start_slot < PIPE_MAX_VIEWPORTS);
   assert((start_slot + num_scissors) <= PIPE_MAX_VIEWPORTS);

   memcpy(llvmpipe->scissors + start_slot, scissors,
          sizeof(struct pipe_scissor_state) * num_scissors);

   llvmpipe->dirty |= LP_NEW_SCISSOR;
}


static void
llvmpipe_set_polygon_stipple(struct pipe_context *pipe,
                             const struct pipe_poly_stipple *stipple)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   draw_flush(llvmpipe->draw);

   llvmpipe->poly_stipple = *stipple; /* struct copy */
   llvmpipe->dirty |= LP_NEW_STIPPLE;
}


void
llvmpipe_init_clip_funcs(struct llvmpipe_context *llvmpipe)
{
   llvmpipe->pipe.set_clip_state = llvmpipe_set_clip_state;
   llvmpipe->pipe.set_polygon_stipple = llvmpipe_set_polygon_stipple;
   llvmpipe->pipe.set_scissor_states = llvmpipe_set_scissor_states;
   llvmpipe->pipe.set_viewport_states = llvmpipe_set_viewport_states;
}
