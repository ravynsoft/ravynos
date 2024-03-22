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

#include "pipe/p_defines.h"
#include "util/u_memory.h"
#include "lp_context.h"
#include "lp_state.h"
#include "lp_setup.h"
#include "draw/draw_context.h"


struct lp_rasterizer_state {
   struct pipe_rasterizer_state lp_state;
   struct pipe_rasterizer_state draw_state;
};


/* State which might be handled in either the draw module or locally.
 * This function is used to turn that state off in one of the two
 * places.
 */
static void
clear_flags(struct pipe_rasterizer_state *rast)
{
   rast->light_twoside = 0;
   rast->offset_tri = 0;
   rast->offset_line = 0;
   rast->offset_point = 0;
   rast->offset_units = 0.0f;
   rast->offset_scale = 0.0f;
}


static void *
llvmpipe_create_rasterizer_state(struct pipe_context *pipe,
                                 const struct pipe_rasterizer_state *rast)
{
   bool need_pipeline;

   /* Partition rasterizer state into what we want the draw module to
    * handle, and what we'll look after ourselves.
    */
   struct lp_rasterizer_state *state = MALLOC_STRUCT(lp_rasterizer_state);
   if (!state)
      return NULL;

   memcpy(&state->draw_state, rast, sizeof *rast);
   memcpy(&state->lp_state, rast, sizeof *rast);

   /* We rely on draw module to do unfilled polygons, AA lines and
    * points and stipple.
    *
    * Over time, reduce this list of conditions, and expand the list
    * of flags which get cleared in clear_flags().
    */
   need_pipeline = (rast->fill_front != PIPE_POLYGON_MODE_FILL ||
                    rast->fill_back != PIPE_POLYGON_MODE_FILL ||
                    rast->point_smooth ||
                    rast->line_smooth ||
                    rast->line_stipple_enable ||
                    rast->poly_stipple_enable);

   /* If not using the pipeline, clear out the flags which we can
    * handle ourselves.  If we *are* using the pipeline, do everything
    * on the pipeline and clear those flags on our internal copy of
    * the state.
    */
   if (need_pipeline)
      clear_flags(&state->lp_state);
   else
      clear_flags(&state->draw_state);

   return state;
}


static void
llvmpipe_bind_rasterizer_state(struct pipe_context *pipe, void *handle)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);
   const struct lp_rasterizer_state *state =
      (const struct lp_rasterizer_state *) handle;

   if (state) {
      llvmpipe->rasterizer = &state->lp_state;
      draw_set_rasterizer_state(llvmpipe->draw, &state->draw_state, handle);
      lp_setup_bind_rasterizer(llvmpipe->setup, &state->lp_state);
   } else {
      llvmpipe->rasterizer = NULL;
      draw_set_rasterizer_state(llvmpipe->draw, NULL, handle);
   }

   llvmpipe->dirty |= LP_NEW_RASTERIZER;
}


static void
llvmpipe_delete_rasterizer_state(struct pipe_context *pipe,
                                 void *rasterizer)
{
   FREE(rasterizer);
}


void
llvmpipe_init_rasterizer_funcs(struct llvmpipe_context *llvmpipe)
{
   llvmpipe->pipe.create_rasterizer_state = llvmpipe_create_rasterizer_state;
   llvmpipe->pipe.bind_rasterizer_state   = llvmpipe_bind_rasterizer_state;
   llvmpipe->pipe.delete_rasterizer_state = llvmpipe_delete_rasterizer_state;
}
