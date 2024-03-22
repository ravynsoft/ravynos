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
#include "util/u_inlines.h"
#include "util/u_draw.h"
#include "util/u_prim.h"

#include "sp_context.h"
#include "sp_query.h"
#include "sp_state.h"
#include "sp_texture.h"
#include "sp_screen.h"

#include "draw/draw_context.h"

/**
 * This function handles drawing indexed and non-indexed prims,
 * instanced and non-instanced drawing, with or without min/max element
 * indexes.
 * All the other drawing functions are expressed in terms of this
 * function.
 *
 * For non-indexed prims, indexBuffer should be NULL.
 * For non-instanced drawing, instanceCount should be 1.
 * When the min/max element indexes aren't known, minIndex should be 0
 * and maxIndex should be ~0.
 */
void
softpipe_draw_vbo(struct pipe_context *pipe,
                  const struct pipe_draw_info *info,
                  unsigned drawid_offset,
                  const struct pipe_draw_indirect_info *indirect,
                  const struct pipe_draw_start_count_bias *draws,
                  unsigned num_draws)
{
   if (num_draws > 1) {
      util_draw_multi(pipe, info, drawid_offset, indirect, draws, num_draws);
      return;
   }

   if (!indirect && (!draws[0].count || !info->instance_count))
      return;

   struct softpipe_context *sp = softpipe_context(pipe);
   struct draw_context *draw = sp->draw;
   const void *mapped_indices = NULL;
   unsigned i;

   if (!softpipe_check_render_cond(sp))
      return;

   if (indirect && indirect->buffer) {
      util_draw_indirect(pipe, info, indirect);
      return;
   }

   sp->reduced_api_prim = u_reduced_prim(info->mode);

   if (sp->dirty) {
      softpipe_update_derived(sp, sp->reduced_api_prim);
   }

   /* Map vertex buffers */
   for (i = 0; i < sp->num_vertex_buffers; i++) {
      const void *buf = sp->vertex_buffer[i].is_user_buffer ?
                           sp->vertex_buffer[i].buffer.user : NULL;
      size_t size = ~0;
      if (!buf) {
         if (!sp->vertex_buffer[i].buffer.resource) {
            continue;
         }
         buf = softpipe_resource_data(sp->vertex_buffer[i].buffer.resource);
         size = sp->vertex_buffer[i].buffer.resource->width0;
      }
      draw_set_mapped_vertex_buffer(draw, i, buf, size);
   }

   /* Map index buffer, if present */
   if (info->index_size) {
      unsigned available_space = ~0;
      mapped_indices = info->has_user_indices ? info->index.user : NULL;
      if (!mapped_indices) {
         mapped_indices = softpipe_resource_data(info->index.resource);
         available_space = info->index.resource->width0;
      }

      draw_set_indexes(draw,
                       (uint8_t *) mapped_indices,
                       info->index_size, available_space);
   }

   if (softpipe_screen(sp->pipe.screen)->use_llvm) {
      softpipe_prepare_vertex_sampling(sp,
                                       sp->num_sampler_views[PIPE_SHADER_VERTEX],
                                       sp->sampler_views[PIPE_SHADER_VERTEX]);
      softpipe_prepare_geometry_sampling(sp,
                                         sp->num_sampler_views[PIPE_SHADER_GEOMETRY],
                                         sp->sampler_views[PIPE_SHADER_GEOMETRY]);
   }

   if (sp->gs && !sp->gs->shader.tokens) {
      /* we have an empty geometry shader with stream output, so
         attach the stream output info to the current vertex shader */
      if (sp->vs) {
         draw_vs_attach_so(sp->vs->draw_data, &sp->gs->shader.stream_output);
      }
   }
   draw_collect_pipeline_statistics(draw,
                                    sp->active_statistics_queries > 0);

   /* draw! */
   draw_vbo(draw, info, drawid_offset, indirect, draws, num_draws, 0);

   /* unmap vertex/index buffers - will cause draw module to flush */
   for (i = 0; i < sp->num_vertex_buffers; i++) {
      draw_set_mapped_vertex_buffer(draw, i, NULL, 0);
   }
   if (mapped_indices) {
      draw_set_indexes(draw, NULL, 0, 0);
   }

   if (softpipe_screen(sp->pipe.screen)->use_llvm) {
      softpipe_cleanup_vertex_sampling(sp);
      softpipe_cleanup_geometry_sampling(sp);
   }

   /*
    * TODO: Flush only when a user vertex/index buffer is present
    * (or even better, modify draw module to do this
    * internally when this condition is seen?)
    */
   draw_flush(draw);

   /* Note: leave drawing surfaces mapped */
   sp->dirty_render_cache = true;
}
