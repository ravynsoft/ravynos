/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
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

#include "svga_cmd.h"

#include "indices/u_indices.h"
#include "util/u_inlines.h"
#include "util/u_prim.h"

#include "svga_hw_reg.h"
#include "svga_draw.h"
#include "svga_draw_private.h"
#include "svga_context.h"
#include "svga_shader.h"


#define DBG 0


static enum pipe_error
generate_indices(struct svga_hwtnl *hwtnl,
                 unsigned nr,
                 unsigned index_size,
                 u_generate_func generate, struct pipe_resource **out_buf)
{
   struct pipe_context *pipe = &hwtnl->svga->pipe;
   struct pipe_transfer *transfer;
   unsigned size = index_size * nr;
   struct pipe_resource *dst = NULL;
   void *dst_map = NULL;

   dst = pipe_buffer_create(pipe->screen, PIPE_BIND_INDEX_BUFFER,
                            PIPE_USAGE_IMMUTABLE, size);
   if (!dst)
      goto fail;

   dst_map = pipe_buffer_map(pipe, dst, PIPE_MAP_WRITE, &transfer);
   if (!dst_map)
      goto fail;

   generate(0, nr, dst_map);

   pipe_buffer_unmap(pipe, transfer);

   *out_buf = dst;
   return PIPE_OK;

fail:
   if (dst_map)
      pipe_buffer_unmap(pipe, transfer);

   if (dst)
      pipe->screen->resource_destroy(pipe->screen, dst);

   return PIPE_ERROR_OUT_OF_MEMORY;
}


static bool
compare(unsigned cached_nr, unsigned nr, unsigned type)
{
   if (type == U_GENERATE_REUSABLE)
      return cached_nr >= nr;
   else
      return cached_nr == nr;
}


static enum pipe_error
retrieve_or_generate_indices(struct svga_hwtnl *hwtnl,
                             enum mesa_prim prim,
                             unsigned gen_type,
                             unsigned gen_nr,
                             unsigned gen_size,
                             u_generate_func generate,
                             struct pipe_resource **out_buf)
{
   enum pipe_error ret = PIPE_OK;
   int i;

   SVGA_STATS_TIME_PUSH(svga_sws(hwtnl->svga), SVGA_STATS_TIME_GENERATEINDICES);

   for (i = 0; i < IDX_CACHE_MAX; i++) {
      if (hwtnl->index_cache[prim][i].buffer != NULL &&
          hwtnl->index_cache[prim][i].generate == generate) {
         if (compare(hwtnl->index_cache[prim][i].gen_nr, gen_nr, gen_type)) {
            pipe_resource_reference(out_buf,
                                    hwtnl->index_cache[prim][i].buffer);

            if (DBG)
               debug_printf("%s retrieve %d/%d\n", __func__, i, gen_nr);

            goto done;
         }
         else if (gen_type == U_GENERATE_REUSABLE) {
            pipe_resource_reference(&hwtnl->index_cache[prim][i].buffer,
                                    NULL);

            if (DBG)
               debug_printf("%s discard %d/%d\n", __func__,
                            i, hwtnl->index_cache[prim][i].gen_nr);

            break;
         }
      }
   }

   if (i == IDX_CACHE_MAX) {
      unsigned smallest = 0;
      unsigned smallest_size = ~0;

      for (i = 0; i < IDX_CACHE_MAX && smallest_size; i++) {
         if (hwtnl->index_cache[prim][i].buffer == NULL) {
            smallest = i;
            smallest_size = 0;
         }
         else if (hwtnl->index_cache[prim][i].gen_nr < smallest) {
            smallest = i;
            smallest_size = hwtnl->index_cache[prim][i].gen_nr;
         }
      }

      assert(smallest != IDX_CACHE_MAX);

      pipe_resource_reference(&hwtnl->index_cache[prim][smallest].buffer,
                              NULL);

      if (DBG)
         debug_printf("%s discard smallest %d/%d\n", __func__,
                      smallest, smallest_size);

      i = smallest;
   }

   ret = generate_indices(hwtnl, gen_nr, gen_size, generate, out_buf);
   if (ret != PIPE_OK)
      goto done;

   hwtnl->index_cache[prim][i].generate = generate;
   hwtnl->index_cache[prim][i].gen_nr = gen_nr;
   pipe_resource_reference(&hwtnl->index_cache[prim][i].buffer, *out_buf);

   if (DBG)
      debug_printf("%s cache %d/%d\n", __func__,
                   i, hwtnl->index_cache[prim][i].gen_nr);

done:
   SVGA_STATS_TIME_POP(svga_sws(hwtnl->svga));
   return ret;
}


static enum pipe_error
simple_draw_arrays(struct svga_hwtnl *hwtnl,
                   enum mesa_prim prim, unsigned start, unsigned count,
                   unsigned start_instance, unsigned instance_count,
                   uint8_t vertices_per_patch)
{
   SVGA3dPrimitiveRange range;
   unsigned hw_prim;
   unsigned hw_count;

   hw_prim = svga_translate_prim(prim, count, &hw_count, vertices_per_patch);
   if (hw_count == 0)
      return PIPE_ERROR_BAD_INPUT;

   range.primType = hw_prim;
   range.primitiveCount = hw_count;
   range.indexArray.surfaceId = SVGA3D_INVALID_ID;
   range.indexArray.offset = 0;
   range.indexArray.stride = 0;
   range.indexWidth = 0;
   range.indexBias = start;

   /* Min/max index should be calculated prior to applying bias, so we
    * end up with min_index = 0, max_index = count - 1 and everybody
    * looking at those numbers knows to adjust them by
    * range.indexBias.
    */
   return svga_hwtnl_prim(hwtnl, &range, count,
                          0, count - 1, NULL,
                          start_instance, instance_count,
                          NULL, NULL);
}


enum pipe_error
svga_hwtnl_draw_arrays(struct svga_hwtnl *hwtnl,
                       enum mesa_prim prim, unsigned start, unsigned count,
                       unsigned start_instance, unsigned instance_count,
                       uint8_t vertices_per_patch)
{
   enum mesa_prim gen_prim;
   unsigned gen_size, gen_nr;
   enum indices_mode gen_type;
   u_generate_func gen_func;
   enum pipe_error ret = PIPE_OK;
   unsigned api_pv = hwtnl->api_pv;
   struct svga_context *svga = hwtnl->svga;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_HWTNLDRAWARRAYS);

   if (svga->curr.rast->templ.fill_front !=
       svga->curr.rast->templ.fill_back) {
      assert(hwtnl->api_fillmode == PIPE_POLYGON_MODE_FILL);
   }

   if (svga->curr.rast->templ.flatshade &&
         svga_fs_variant(svga->state.hw_draw.fs)->constant_color_output) {
      /* The fragment color is a constant, not per-vertex so the whole
       * primitive will be the same color (except for possible blending).
       * We can ignore the current provoking vertex state and use whatever
       * the hardware wants.
       */
      api_pv = hwtnl->hw_pv;

      if (hwtnl->api_fillmode == PIPE_POLYGON_MODE_FILL) {
         /* Do some simple primitive conversions to avoid index buffer
          * generation below.  Note that polygons and quads are not directly
          * supported by the svga device.  Also note, we can only do this
          * for flat/constant-colored rendering because of provoking vertex.
          */
         if (prim == MESA_PRIM_POLYGON) {
            prim = MESA_PRIM_TRIANGLE_FAN;
         }
         else if (prim == MESA_PRIM_QUADS && count == 4) {
            prim = MESA_PRIM_TRIANGLE_FAN;
         }
      }
   }

   if (svga_need_unfilled_fallback(hwtnl, prim)) {
      /* Convert unfilled polygons into points, lines, triangles */
      gen_type = u_unfilled_generator(prim,
                                      start,
                                      count,
                                      hwtnl->api_fillmode,
                                      &gen_prim,
                                      &gen_size, &gen_nr, &gen_func);
   }
   else {
      /* Convert MESA_PRIM_LINE_LOOP to MESA_PRIM_LINESTRIP,
       * convert MESA_PRIM_POLYGON to MESA_PRIM_TRIANGLE_FAN,
       * etc, if needed (as determined by svga_hw_prims mask).
       */
      gen_type = u_index_generator(svga_hw_prims,
                                   prim,
                                   start,
                                   count,
                                   api_pv,
                                   hwtnl->hw_pv,
                                   &gen_prim, &gen_size, &gen_nr, &gen_func);
   }

   if (gen_type == U_GENERATE_LINEAR) {
      ret = simple_draw_arrays(hwtnl, gen_prim, start, count,
                               start_instance, instance_count,
                               vertices_per_patch);
   }
   else {
      struct pipe_resource *gen_buf = NULL;

      /* Need to draw as indexed primitive.
       * Potentially need to run the gen func to build an index buffer.
       */
      ret = retrieve_or_generate_indices(hwtnl,
                                         prim,
                                         gen_type,
                                         gen_nr,
                                         gen_size, gen_func, &gen_buf);
      if (ret == PIPE_OK) {
         util_debug_message(&svga->debug.callback, PERF_INFO,
                            "generating temporary index buffer for drawing %s",
                            u_prim_name(prim));

         ret = svga_hwtnl_simple_draw_range_elements(hwtnl,
                                                     gen_buf,
                                                     gen_size,
                                                     start,
                                                     0,
                                                     count - 1,
                                                     gen_prim, 0, gen_nr,
                                                     start_instance,
                                                     instance_count,
                                                     vertices_per_patch);
      }

      if (gen_buf) {
         pipe_resource_reference(&gen_buf, NULL);
      }
   }

   SVGA_STATS_TIME_POP(svga_sws(svga));
   return ret;
}
