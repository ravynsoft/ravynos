/**********************************************************
 * Copyright 2008-2023 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/


#include "util/u_draw.h"
#include "util/format/u_format.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_prim.h"
#include "util/u_prim_restart.h"

#include "svga_context.h"
#include "svga_draw_private.h"
#include "svga_screen.h"
#include "svga_draw.h"
#include "svga_shader.h"
#include "svga_surface.h"
#include "svga_swtnl.h"
#include "svga_debug.h"
#include "svga_resource_buffer.h"


static enum pipe_error
retry_draw_range_elements(struct svga_context *svga,
                          const struct pipe_draw_info *info,
                          const struct pipe_draw_start_count_bias *draw,
                          unsigned count)
{
   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_DRAWELEMENTS);

   SVGA_RETRY(svga, svga_hwtnl_draw_range_elements(svga->hwtnl, info, draw, count));

   SVGA_STATS_TIME_POP(svga_sws(svga));
   return PIPE_OK;
}


static enum pipe_error
retry_draw_arrays( struct svga_context *svga,
                   enum mesa_prim prim, unsigned start, unsigned count,
                   unsigned start_instance, unsigned instance_count,
                   uint8_t vertices_per_patch)
{
   enum pipe_error ret;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_DRAWARRAYS);

   SVGA_RETRY_OOM(svga, ret, svga_hwtnl_draw_arrays(svga->hwtnl, prim, start,
                                                    count, start_instance,
                                                    instance_count,
                                                    vertices_per_patch));
   SVGA_STATS_TIME_POP(svga_sws(svga));
   return ret;
}


/**
 * Auto draw (get vertex count from a transform feedback result).
 */
static enum pipe_error
retry_draw_auto(struct svga_context *svga,
                const struct pipe_draw_info *info,
                const struct pipe_draw_indirect_info *indirect)
{
   assert(svga_have_sm5(svga));
   assert(indirect->count_from_stream_output);
   assert(info->instance_count == 1);
   /* SO drawing implies core profile and none of these prim types */
   assert(info->mode != MESA_PRIM_QUADS &&
          info->mode != MESA_PRIM_QUAD_STRIP &&
          info->mode != MESA_PRIM_POLYGON);

   if (info->mode == MESA_PRIM_LINE_LOOP) {
      /* XXX need to do a fallback */
      assert(!"draw auto fallback not supported yet");
      return PIPE_OK;
   }
   else {
      SVGA3dPrimitiveRange range;
      unsigned hw_count;

      range.primType = svga_translate_prim(info->mode, 12, &hw_count,
                                           svga->patch_vertices);
      range.primitiveCount = 0;
      range.indexArray.surfaceId = SVGA3D_INVALID_ID;
      range.indexArray.offset = 0;
      range.indexArray.stride = 0;
      range.indexWidth = 0;
      range.indexBias = 0;

      SVGA_RETRY(svga, svga_hwtnl_prim
                 (svga->hwtnl, &range,
                  0,    /* vertex count comes from SO buffer */
                  0,    /* don't know min index */
                  ~0u,  /* don't know max index */
                  NULL, /* no index buffer */
                  0,    /* start instance */
                  1,    /* only 1 instance supported */
                  NULL, /* indirect drawing info */
                  indirect->count_from_stream_output));

      return PIPE_OK;
   }
}


/**
 * Indirect draw (get vertex count, start index, etc. from a buffer object.
 */
static enum pipe_error
retry_draw_indirect(struct svga_context *svga,
                    const struct pipe_draw_info *info,
                    const struct pipe_draw_indirect_info *indirect)
{
   assert(svga_have_sm5(svga));
   assert(indirect && indirect->buffer);
   /* indirect drawing implies core profile and none of these prim types */
   assert(info->mode != MESA_PRIM_QUADS &&
          info->mode != MESA_PRIM_QUAD_STRIP &&
          info->mode != MESA_PRIM_POLYGON);

   if (info->mode == MESA_PRIM_LINE_LOOP) {
      /* need to do a fallback */
      util_draw_indirect(&svga->pipe, info, indirect);
      return PIPE_OK;
   }
   else {
      SVGA3dPrimitiveRange range;
      unsigned hw_count;

      range.primType = svga_translate_prim(info->mode, 12, &hw_count,
                                           svga->patch_vertices);
      range.primitiveCount = 0;  /* specified in indirect buffer */
      range.indexArray.surfaceId = SVGA3D_INVALID_ID;
      range.indexArray.offset = 0;
      range.indexArray.stride = 0;
      range.indexWidth = info->index_size;
      range.indexBias = 0; /* specified in indirect buffer */

      SVGA_RETRY(svga, svga_hwtnl_prim
                 (svga->hwtnl, &range,
                  0,   /* vertex count is in indirect buffer */
                  0,   /* don't know min index */
                  ~0u, /* don't know max index */
                  info->index.resource,
                  info->start_instance,
                  0,   /* don't know instance count */
                  indirect,
                  NULL)); /* SO vertex count */

      return PIPE_OK;
   }
}


/**
 * Determine if we need to implement primitive restart with a fallback
 * path which breaks the original primitive into sub-primitive at the
 * restart indexes.
 */
static bool
need_fallback_prim_restart(const struct svga_context *svga,
                           const struct pipe_draw_info *info)
{
   if (info->primitive_restart && info->index_size) {
      if (!svga_have_vgpu10(svga))
         return true;
      else if (!svga->state.sw.need_swtnl) {
         if (info->index_size == 1)
            return true; /* no device support for 1-byte indexes */
         else if (info->index_size == 2)
            return info->restart_index != 0xffff;
         else
            return info->restart_index != 0xffffffff;
      }
   }

   return false;
}


/**
 * A helper function to return the vertex count from the primitive count
 * returned from the stream output statistics query for the specified stream.
 */
static unsigned
get_vcount_from_stream_output(struct svga_context *svga,
                              const struct pipe_draw_info *info,
                              unsigned stream)
{
   unsigned primcount;
   primcount = svga_get_primcount_from_stream_output(svga, stream);
   return u_vertices_for_prims(info->mode, primcount);
}


static void
svga_draw_vbo(struct pipe_context *pipe, const struct pipe_draw_info *info,
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

   struct svga_context *svga = svga_context(pipe);
   enum mesa_prim reduced_prim = u_reduced_prim(info->mode);
   unsigned count = draws[0].count;
   enum pipe_error ret = 0;
   bool needed_swtnl;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_DRAWVBO);

   svga->hud.num_draw_calls++;  /* for SVGA_QUERY_NUM_DRAW_CALLS */

   if (u_reduced_prim(info->mode) == MESA_PRIM_TRIANGLES &&
       svga->curr.rast->templ.cull_face == PIPE_FACE_FRONT_AND_BACK)
      goto done;

   if (svga->curr.reduced_prim != reduced_prim) {
      svga->curr.reduced_prim = reduced_prim;
      svga->dirty |= SVGA_NEW_REDUCED_PRIMITIVE;
   }

   /* We need to adjust the vertexID in the vertex shader since SV_VertexID
    * always start from 0 for DrawArrays and does not include baseVertex for
    * DrawIndexed.
    */
   unsigned index_bias = info->index_size ? draws->index_bias : 0;
   if (svga->curr.vertex_id_bias != (draws[0].start + index_bias)) {
      svga->curr.vertex_id_bias = draws[0].start + index_bias;
      svga->dirty |= SVGA_NEW_VS_CONSTS;
   }

   if (svga->curr.vertices_per_patch != svga->patch_vertices) {
      svga->curr.vertices_per_patch = svga->patch_vertices;

      /* If input patch size changes, we need to notifiy the TCS
       * code to reevaluate the shader variant since the
       * vertices per patch count is a constant in the control
       * point count declaration.
       */
      if (svga->curr.tcs || svga->curr.tes)
         svga->dirty |= SVGA_NEW_TCS_PARAM;
   }

   if (need_fallback_prim_restart(svga, info)) {
      enum pipe_error r;
      r = util_draw_vbo_without_prim_restart(pipe, info, drawid_offset, indirect, &draws[0]);
      assert(r == PIPE_OK);
      (void) r;
      goto done;
   }

   if (!indirect && !u_trim_pipe_prim(info->mode, &count))
      goto done;

   needed_swtnl = svga->state.sw.need_swtnl;

   svga_update_state_retry(svga, SVGA_STATE_NEED_SWTNL);

   if (svga->state.sw.need_swtnl) {
      svga->hud.num_fallbacks++;  /* for SVGA_QUERY_NUM_FALLBACKS */
      if (!needed_swtnl) {
         /*
          * We're switching from HW to SW TNL.  SW TNL will require mapping all
          * currently bound vertex buffers, some of which may already be
          * referenced in the current command buffer as result of previous HW
          * TNL. So flush now, to prevent the context to flush while a referred
          * vertex buffer is mapped.
          */

         svga_context_flush(svga, NULL);
      }

      /* Avoid leaking the previous hwtnl bias to swtnl */
      svga_hwtnl_set_index_bias(svga->hwtnl, 0);
      ret = svga_swtnl_draw_vbo(svga, info, drawid_offset, indirect, &draws[0]);
   }
   else {
      if (!svga_update_state_retry(svga, SVGA_STATE_HW_DRAW)) {
         static const char *msg = "State update failed, skipping draw call";
         debug_printf("%s\n", msg);
         util_debug_message(&svga->debug.callback, INFO, "%s", msg);
         goto done;
      }
      svga_hwtnl_set_fillmode(svga->hwtnl, svga->curr.rast->hw_fillmode);

      svga_update_state_retry(svga, SVGA_STATE_HW_DRAW);

      /** determine if flatshade is to be used after svga_update_state()
       *  in case the fragment shader is changed.
       */
      svga_hwtnl_set_flatshade(svga->hwtnl,
                               svga->curr.rast->templ.flatshade ||
                               svga_is_using_flat_shading(svga),
                               svga->curr.rast->templ.flatshade_first);

      if (indirect && indirect->count_from_stream_output) {
         unsigned stream = 0;
         assert(count == 0);

         /* If the vertex count is from the stream output of a non-zero stream
          * or the draw info specifies instancing, we will need a workaround
          * since the draw_auto command does not support stream instancing.
          * The workaround requires querying the vertex count from the
          * stream output statistics query for the specified stream and then
          * fallback to the regular draw function.
          */

         /* Check the stream index of the specified stream output target */
         for (unsigned i = 0; i < ARRAY_SIZE(svga->so_targets); i++) {
            if (svga->vcount_so_targets[i] == indirect->count_from_stream_output) {
               stream = (svga->vcount_buffer_stream >> (i * 4)) & 0xf;
               break;
            }
         }
         if (info->instance_count > 1 || stream > 0) {
            count = get_vcount_from_stream_output(svga, info, stream);
         }
      }

      if (indirect && indirect->count_from_stream_output && count == 0) {
         ret = retry_draw_auto(svga, info, indirect);
      }
      else if (indirect && indirect->buffer) {
         ret = retry_draw_indirect(svga, info, indirect);
      }
      else if (info->index_size) {
         ret = retry_draw_range_elements(svga, info, &draws[0], count);
      }
      else {
         ret = retry_draw_arrays(svga, info->mode, draws[0].start, count,
                                 info->start_instance, info->instance_count,
                                 svga->patch_vertices);
      }
   }

   /*
    * Mark currently bound target surfaces as dirty after draw is completed.
    */
   svga_mark_surfaces_dirty(svga_context(pipe));

   /* XXX: Silence warnings, do something sensible here? */
   (void)ret;

   if (SVGA_DEBUG & DEBUG_FLUSH) {
      svga_hwtnl_flush_retry(svga);
      svga_context_flush(svga, NULL);
   }

done:
   SVGA_STATS_TIME_POP(svga_sws(svga));
}


void
svga_init_draw_functions(struct svga_context *svga)
{
   svga->pipe.draw_vbo = svga_draw_vbo;
}
