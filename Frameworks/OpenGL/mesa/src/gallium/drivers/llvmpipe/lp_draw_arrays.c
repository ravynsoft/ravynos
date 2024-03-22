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

/* Author:
 *    Brian Paul
 *    Keith Whitwell
 */


#include "pipe/p_defines.h"
#include "pipe/p_context.h"
#include "util/u_draw.h"
#include "util/u_prim.h"

#include "lp_context.h"
#include "lp_state.h"
#include "lp_query.h"

#include "draw/draw_context.h"



/**
 * Draw vertex arrays, with optional indexing, optional instancing.
 * All the other drawing functions are implemented in terms of this function.
 * Basically, map the vertex buffers (and drawing surfaces), then hand off
 * the drawing to the 'draw' module.
 */
static void
llvmpipe_draw_vbo(struct pipe_context *pipe, const struct pipe_draw_info *info,
                  unsigned drawid_offset,
                  const struct pipe_draw_indirect_info *indirect,
                  const struct pipe_draw_start_count_bias *draws,
                  unsigned num_draws)
{
   if (!indirect && (!draws[0].count || !info->instance_count))
      return;

   struct llvmpipe_context *lp = llvmpipe_context(pipe);
   struct draw_context *draw = lp->draw;
   const void *mapped_indices = NULL;
   unsigned i;

   if (!llvmpipe_check_render_cond(lp))
      return;

   if (indirect && indirect->buffer) {
      util_draw_indirect(pipe, info, indirect);
      return;
   }

   if (lp->dirty)
      llvmpipe_update_derived(lp);

   /*
    * Map vertex buffers
    */
   for (i = 0; i < lp->num_vertex_buffers; i++) {
      const void *buf = lp->vertex_buffer[i].is_user_buffer ?
                           lp->vertex_buffer[i].buffer.user : NULL;
      size_t size = ~0;
      if (!buf) {
         if (!lp->vertex_buffer[i].buffer.resource) {
            continue;
         }
         buf = llvmpipe_resource_data(lp->vertex_buffer[i].buffer.resource);
         size = lp->vertex_buffer[i].buffer.resource->width0;
      }
      draw_set_mapped_vertex_buffer(draw, i, buf, size);
   }

   /* Map index buffer, if present */
   if (info->index_size) {
      unsigned available_space = ~0;
      mapped_indices = info->has_user_indices ? info->index.user : NULL;
      if (!mapped_indices) {
         mapped_indices = llvmpipe_resource_data(info->index.resource);
         available_space = info->index.resource->width0;
      }
      draw_set_indexes(draw,
                       (uint8_t *) mapped_indices,
                       info->index_size, available_space);
   }

   llvmpipe_prepare_vertex_sampling(lp,
                                    lp->num_sampler_views[PIPE_SHADER_VERTEX],
                                    lp->sampler_views[PIPE_SHADER_VERTEX]);
   llvmpipe_prepare_geometry_sampling(lp,
                                      lp->num_sampler_views[PIPE_SHADER_GEOMETRY],
                                      lp->sampler_views[PIPE_SHADER_GEOMETRY]);
   llvmpipe_prepare_tess_ctrl_sampling(lp,
                                       lp->num_sampler_views[PIPE_SHADER_TESS_CTRL],
                                       lp->sampler_views[PIPE_SHADER_TESS_CTRL]);
   llvmpipe_prepare_tess_eval_sampling(lp,
                                       lp->num_sampler_views[PIPE_SHADER_TESS_EVAL],
                                       lp->sampler_views[PIPE_SHADER_TESS_EVAL]);

   llvmpipe_prepare_vertex_images(lp,
                                  lp->num_images[PIPE_SHADER_VERTEX],
                                  lp->images[PIPE_SHADER_VERTEX]);
   llvmpipe_prepare_geometry_images(lp,
                                    lp->num_images[PIPE_SHADER_GEOMETRY],
                                    lp->images[PIPE_SHADER_GEOMETRY]);
   llvmpipe_prepare_tess_ctrl_images(lp,
                                     lp->num_images[PIPE_SHADER_TESS_CTRL],
                                     lp->images[PIPE_SHADER_TESS_CTRL]);
   llvmpipe_prepare_tess_eval_images(lp,
                                     lp->num_images[PIPE_SHADER_TESS_EVAL],
                                     lp->images[PIPE_SHADER_TESS_EVAL]);
   if (lp->gs && lp->gs->no_tokens) {
      /* we have an empty geometry shader with stream output, so
         attach the stream output info to the current vertex shader */
      if (lp->vs) {
         draw_vs_attach_so(lp->vs, &lp->gs->stream_output);
      }
   }
   draw_collect_pipeline_statistics(draw,
                                    lp->active_statistics_queries > 0 &&
                                    !lp->queries_disabled);

   draw_collect_primitives_generated(draw,
                                     lp->active_primgen_queries &&
                                     !lp->queries_disabled);

   /* draw! */
   draw_vbo(draw, info, drawid_offset, indirect, draws, num_draws,
            lp->patch_vertices);

   /*
    * unmap vertex/index buffers
    */
   for (i = 0; i < lp->num_vertex_buffers; i++) {
      draw_set_mapped_vertex_buffer(draw, i, NULL, 0);
   }
   if (mapped_indices) {
      draw_set_indexes(draw, NULL, 0, 0);
   }

   if (lp->gs && lp->gs->no_tokens) {
      /* we have attached stream output to the vs for rendering,
         now lets reset it */
      if (lp->vs) {
         draw_vs_reset_so(lp->vs);
      }
   }

   llvmpipe_cleanup_stage_sampling(lp, PIPE_SHADER_VERTEX);
   llvmpipe_cleanup_stage_sampling(lp, PIPE_SHADER_GEOMETRY);
   llvmpipe_cleanup_stage_sampling(lp, PIPE_SHADER_TESS_CTRL);
   llvmpipe_cleanup_stage_sampling(lp, PIPE_SHADER_TESS_EVAL);

   llvmpipe_cleanup_stage_images(lp, PIPE_SHADER_VERTEX);
   llvmpipe_cleanup_stage_images(lp, PIPE_SHADER_GEOMETRY);
   llvmpipe_cleanup_stage_images(lp, PIPE_SHADER_TESS_CTRL);
   llvmpipe_cleanup_stage_images(lp, PIPE_SHADER_TESS_EVAL);

   /*
    * TODO: Flush only when a user vertex/index buffer is present
    * (or even better, modify draw module to do this
    * internally when this condition is seen?)
    */
   draw_flush(draw);
}


void
llvmpipe_init_draw_funcs(struct llvmpipe_context *llvmpipe)
{
   llvmpipe->pipe.draw_vbo = llvmpipe_draw_vbo;
}
