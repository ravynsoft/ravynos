/**************************************************************************
 *
 * Copyright 2003 VMware, Inc.
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

#include "i915_context.h"
#include "i915_batch.h"
#include "i915_debug.h"
#include "i915_query.h"
#include "i915_resource.h"
#include "i915_screen.h"
#include "i915_state.h"
#include "i915_surface.h"

#include "draw/draw_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "util/u_draw.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "util/u_upload_mgr.h"

/*
 * Draw functions
 */

static void
i915_draw_vbo(struct pipe_context *pipe, const struct pipe_draw_info *info,
              unsigned drawid_offset,
              const struct pipe_draw_indirect_info *indirect,
              const struct pipe_draw_start_count_bias *draws,
              unsigned num_draws)
{
   if (num_draws > 1) {
      util_draw_multi(pipe, info, drawid_offset, indirect, draws, num_draws);
      return;
   }

   struct i915_context *i915 = i915_context(pipe);
   struct draw_context *draw = i915->draw;
   const void *mapped_indices = NULL;
   unsigned i;

   if (!u_trim_pipe_prim(info->mode, (unsigned *)&draws[0].count))
      return;

   /*
    * Ack vs contants here, helps ipers a lot.
    */
   i915->dirty &= ~I915_NEW_VS_CONSTANTS;

   if (i915->dirty)
      i915_update_derived(i915);

   /*
    * Map vertex buffers
    */
   for (i = 0; i < i915->nr_vertex_buffers; i++) {
      const void *buf = i915->vertex_buffers[i].is_user_buffer
                           ? i915->vertex_buffers[i].buffer.user
                           : NULL;
      if (!buf) {
         if (!i915->vertex_buffers[i].buffer.resource)
            continue;
         buf = i915_buffer(i915->vertex_buffers[i].buffer.resource)->data;
      }
      draw_set_mapped_vertex_buffer(draw, i, buf, ~0);
   }

   /*
    * Map index buffer, if present
    */
   if (info->index_size) {
      mapped_indices = info->has_user_indices ? info->index.user : NULL;
      if (!mapped_indices)
         mapped_indices = i915_buffer(info->index.resource)->data;
      draw_set_indexes(draw, (uint8_t *)mapped_indices, info->index_size, ~0);
   }

   if (i915->constants[PIPE_SHADER_VERTEX])
      draw_set_mapped_constant_buffer(
         draw, PIPE_SHADER_VERTEX, 0,
         i915_buffer(i915->constants[PIPE_SHADER_VERTEX])->data,
         (i915->current.num_user_constants[PIPE_SHADER_VERTEX] * 4 *
          sizeof(float)));
   else
      draw_set_mapped_constant_buffer(draw, PIPE_SHADER_VERTEX, 0, NULL, 0);

   /*
    * Do the drawing
    */
   draw_vbo(i915->draw, info, drawid_offset, NULL, draws, num_draws, 0);

   /*
    * unmap vertex/index buffers
    */
   for (i = 0; i < i915->nr_vertex_buffers; i++) {
      draw_set_mapped_vertex_buffer(i915->draw, i, NULL, 0);
   }
   if (mapped_indices)
      draw_set_indexes(draw, NULL, 0, 0);

   /*
    * Instead of flushing on every state change, we flush once here
    * when we fire the vbo.
    */
   draw_flush(i915->draw);
}

/*
 * Generic context functions
 */

static void
i915_destroy(struct pipe_context *pipe)
{
   struct i915_context *i915 = i915_context(pipe);
   int i;

   if (i915->blitter)
      util_blitter_destroy(i915->blitter);

   draw_destroy(i915->draw);

   if (i915->base.stream_uploader)
      u_upload_destroy(i915->base.stream_uploader);

   if (i915->batch)
      i915->iws->batchbuffer_destroy(i915->batch);

   /* unbind framebuffer */
   util_unreference_framebuffer_state(&i915->framebuffer);

   /* unbind constant buffers */
   for (i = 0; i < PIPE_SHADER_TYPES; i++) {
      pipe_resource_reference(&i915->constants[i], NULL);
   }

   FREE(i915);
}

static void
i915_set_debug_callback(struct pipe_context *pipe,
                        const struct util_debug_callback *cb)
{
   struct i915_context *i915 = i915_context(pipe);

   if (cb)
      i915->debug = *cb;
   else
      memset(&i915->debug, 0, sizeof(i915->debug));
}

struct pipe_context *
i915_create_context(struct pipe_screen *screen, void *priv, unsigned flags)
{
   struct i915_context *i915;

   i915 = CALLOC_STRUCT(i915_context);
   if (!i915)
      return NULL;

   i915->iws = i915_screen(screen)->iws;
   i915->base.screen = screen;
   i915->base.priv = priv;
   i915->base.stream_uploader = u_upload_create_default(&i915->base);
   i915->base.const_uploader = i915->base.stream_uploader;
   i915->base.set_debug_callback = i915_set_debug_callback;

   i915->base.destroy = i915_destroy;

   if (i915_screen(screen)->debug.use_blitter)
      i915->base.clear = i915_clear_blitter;
   else
      i915->base.clear = i915_clear_render;

   i915->base.draw_vbo = i915_draw_vbo;

   /* init this before draw */
   slab_create(&i915->transfer_pool, sizeof(struct pipe_transfer), 16);
   slab_create(&i915->texture_transfer_pool, sizeof(struct i915_transfer), 16);

   /* Batch stream debugging is a bit hacked up at the moment:
    */
   i915->batch = i915->iws->batchbuffer_create(i915->iws);

   /*
    * Create drawing context and plug our rendering stage into it.
    */
   i915->draw = draw_create(&i915->base);
   assert(i915->draw);
   if (i915_debug & DBG_VBUF) {
      draw_set_rasterize_stage(i915->draw, i915_draw_vbuf_stage(i915));
   } else {
      draw_set_rasterize_stage(i915->draw, i915_draw_render_stage(i915));
   }

   i915_init_surface_functions(i915);
   i915_init_state_functions(i915);
   i915_init_flush_functions(i915);
   i915_init_resource_functions(i915);
   i915_init_query_functions(i915);

   /* Create blitter. */
   i915->blitter = util_blitter_create(&i915->base);
   assert(i915->blitter);

   /* must be done before installing Draw stages */
   i915->no_log_program_errors = true;
   util_blitter_cache_all_shaders(i915->blitter);
   i915->no_log_program_errors = false;

   draw_install_aaline_stage(i915->draw, &i915->base);
   draw_install_aapoint_stage(i915->draw, &i915->base, nir_type_float32);
   draw_enable_point_sprites(i915->draw, true);

   i915->dirty = ~0;
   i915->hardware_dirty = ~0;
   i915->immediate_dirty = ~0;
   i915->dynamic_dirty = ~0;
   i915->static_dirty = ~0;
   i915->flush_dirty = 0;

   i915->current.fixup_swizzle = ~0;

   return &i915->base;
}
