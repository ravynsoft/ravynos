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

#include "draw/draw_vbuf.h"
#include "draw/draw_context.h"
#include "draw/draw_vertex.h"

#include "util/u_debug.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "svga_context.h"
#include "svga_state.h"
#include "svga_swtnl.h"

#include "svga_types.h"
#include "svga_reg.h"
#include "svga3d_reg.h"
#include "svga_draw.h"
#include "svga_shader.h"
#include "svga_swtnl_private.h"


static const struct vertex_info *
svga_vbuf_render_get_vertex_info(struct vbuf_render *render)
{
   struct svga_vbuf_render *svga_render = svga_vbuf_render(render);
   struct svga_context *svga = svga_render->svga;

   svga_swtnl_update_vdecl(svga);

   return &svga_render->vertex_info;
}


static bool
svga_vbuf_render_allocate_vertices(struct vbuf_render *render,
                                   uint16_t vertex_size,
                                   uint16_t nr_vertices)
{
   struct svga_vbuf_render *svga_render = svga_vbuf_render(render);
   struct svga_context *svga = svga_render->svga;
   struct pipe_screen *screen = svga->pipe.screen;
   size_t size = (size_t)nr_vertices * (size_t)vertex_size;
   bool new_vbuf = false;
   bool new_ibuf = false;

   SVGA_STATS_TIME_PUSH(svga_sws(svga),
                        SVGA_STATS_TIME_VBUFRENDERALLOCVERT);

   if (svga_render->vertex_size != vertex_size)
      svga->swtnl.new_vdecl = true;
   svga_render->vertex_size = (size_t)vertex_size;

   if (svga->swtnl.new_vbuf)
      new_ibuf = new_vbuf = true;
   svga->swtnl.new_vbuf = false;

   if (svga_render->vbuf_size
       < svga_render->vbuf_offset + svga_render->vbuf_used + size)
      new_vbuf = true;

   if (new_vbuf)
      pipe_resource_reference(&svga_render->vbuf, NULL);
   if (new_ibuf)
      pipe_resource_reference(&svga_render->ibuf, NULL);

   if (!svga_render->vbuf) {
      svga_render->vbuf_size = MAX2(size, svga_render->vbuf_alloc_size);
      svga_render->vbuf = SVGA_TRY_PTR(pipe_buffer_create
                                       (screen, PIPE_BIND_VERTEX_BUFFER,
                                        PIPE_USAGE_STREAM,
                                        svga_render->vbuf_size));
      if (!svga_render->vbuf) {
         svga_retry_enter(svga);
         svga_context_flush(svga, NULL);
         assert(!svga_render->vbuf);
         svga_render->vbuf = pipe_buffer_create(screen,
                                                PIPE_BIND_VERTEX_BUFFER,
                                                PIPE_USAGE_STREAM,
                                                svga_render->vbuf_size);
         /* The buffer allocation may fail if we run out of memory.
          * The draw module's vbuf code should handle that without crashing.
          */
         svga_retry_exit(svga);
      }

      svga->swtnl.new_vdecl = true;
      svga_render->vbuf_offset = 0;
   } else {
      svga_render->vbuf_offset += svga_render->vbuf_used;
   }

   svga_render->vbuf_used = 0;

   if (svga->swtnl.new_vdecl)
      svga_render->vdecl_offset = svga_render->vbuf_offset;

   SVGA_STATS_TIME_POP(svga_sws(svga));

   return true;
}


static void *
svga_vbuf_render_map_vertices(struct vbuf_render *render)
{
   struct svga_vbuf_render *svga_render = svga_vbuf_render(render);
   struct svga_context *svga = svga_render->svga;
   void * retPtr = NULL;

   SVGA_STATS_TIME_PUSH(svga_sws(svga),
                        SVGA_STATS_TIME_VBUFRENDERMAPVERT);

   if (svga_render->vbuf) {
      char *ptr = (char*)pipe_buffer_map(&svga->pipe,
                                         svga_render->vbuf,
                                         PIPE_MAP_WRITE |
                                         PIPE_MAP_FLUSH_EXPLICIT |
                                         PIPE_MAP_DISCARD_RANGE |
                                         PIPE_MAP_UNSYNCHRONIZED,
                                         &svga_render->vbuf_transfer);
      if (ptr) {
         svga_render->vbuf_ptr = ptr;
         retPtr = ptr + svga_render->vbuf_offset;
      }
      else {
         svga_render->vbuf_ptr = NULL;
         svga_render->vbuf_transfer = NULL;
         retPtr = NULL;
      }
   }
   else {
      /* we probably ran out of memory when allocating the vertex buffer */
      retPtr = NULL;
   }

   SVGA_STATS_TIME_POP(svga_sws(svga));
   return retPtr;
}


static void
svga_vbuf_render_unmap_vertices(struct vbuf_render *render,
                                uint16_t min_index,
                                uint16_t max_index)
{
   struct svga_vbuf_render *svga_render = svga_vbuf_render(render);
   struct svga_context *svga = svga_render->svga;
   unsigned offset, length;
   size_t used = svga_render->vertex_size * ((size_t)max_index + 1);

   SVGA_STATS_TIME_PUSH(svga_sws(svga),
                        SVGA_STATS_TIME_VBUFRENDERUNMAPVERT);

   offset = svga_render->vbuf_offset + svga_render->vertex_size * min_index;
   length = svga_render->vertex_size * (max_index + 1 - min_index);

   if (0) {
      /* dump vertex data */
      const float *f = (const float *) ((char *) svga_render->vbuf_ptr +
                                        svga_render->vbuf_offset);
      unsigned i;
      debug_printf("swtnl vertex data:\n");
      for (i = 0; i < length / 4; i += 4) {
         debug_printf("%u: %f %f %f %f\n", i, f[i], f[i+1], f[i+2], f[i+3]);
      }
   }

   pipe_buffer_flush_mapped_range(&svga->pipe,
                                  svga_render->vbuf_transfer,
                                  offset, length);
   pipe_buffer_unmap(&svga->pipe, svga_render->vbuf_transfer);
   svga_render->min_index = min_index;
   svga_render->max_index = max_index;
   svga_render->vbuf_used = MAX2(svga_render->vbuf_used, used);

   SVGA_STATS_TIME_POP(svga_sws(svga));
}


static void
svga_vbuf_render_set_primitive(struct vbuf_render *render,
                               enum mesa_prim prim)
{
   struct svga_vbuf_render *svga_render = svga_vbuf_render(render);
   svga_render->prim = prim;
}


static void
svga_vbuf_submit_state(struct svga_vbuf_render *svga_render)
{
   struct svga_context *svga = svga_render->svga;
   SVGA3dVertexDecl vdecl[PIPE_MAX_ATTRIBS];
   unsigned i;
   static const unsigned zero[PIPE_MAX_ATTRIBS] = {0};
   bool retried;

   /* if the vdecl or vbuf hasn't changed do nothing */
   if (!svga->swtnl.new_vdecl)
      return;

   SVGA_STATS_TIME_PUSH(svga_sws(svga),
                        SVGA_STATS_TIME_VBUFSUBMITSTATE);

   memcpy(vdecl, svga_render->vdecl, sizeof(vdecl));

   /* flush the hw state */
   SVGA_RETRY_CHECK(svga, svga_hwtnl_flush(svga->hwtnl), retried);
   if (retried) {
      /* if we hit this path we might become synced with hw */
      svga->swtnl.new_vbuf = true;
   }

   for (i = 0; i < svga_render->vdecl_count; i++) {
      vdecl[i].array.offset += svga_render->vdecl_offset;
   }

   svga_hwtnl_vertex_decls(svga->hwtnl,
                           svga_render->vdecl_count,
                           vdecl,
                           zero,
                           svga_render->layout_id);

   /* Specify the vertex buffer (there's only ever one) */
   {
      struct pipe_vertex_buffer vb;
      vb.is_user_buffer = false;
      vb.buffer.resource = svga_render->vbuf;
      vb.buffer_offset = svga_render->vdecl_offset;
      svga_hwtnl_vertex_buffers(svga->hwtnl, 1, &vb);
   }

   /* We have already taken care of flatshading, so let the hwtnl
    * module use whatever is most convenient:
    */
   if (svga->state.sw.need_pipeline) {
      svga_hwtnl_set_flatshade(svga->hwtnl, false, false);
      svga_hwtnl_set_fillmode(svga->hwtnl, PIPE_POLYGON_MODE_FILL);
   }
   else {
      svga_hwtnl_set_flatshade(svga->hwtnl,
                                svga->curr.rast->templ.flatshade ||
                                svga_is_using_flat_shading(svga),
                                svga->curr.rast->templ.flatshade_first);

      svga_hwtnl_set_fillmode(svga->hwtnl, svga->curr.rast->hw_fillmode);
   }

   svga->swtnl.new_vdecl = false;
   SVGA_STATS_TIME_POP(svga_sws(svga));
}


static void
svga_vbuf_render_draw_arrays(struct vbuf_render *render,
                              unsigned start, uint nr)
{
   struct svga_vbuf_render *svga_render = svga_vbuf_render(render);
   struct svga_context *svga = svga_render->svga;
   unsigned bias = (svga_render->vbuf_offset - svga_render->vdecl_offset)
      / svga_render->vertex_size;
   /* instancing will already have been resolved at this point by 'draw' */
   const unsigned start_instance = 0;
   const unsigned instance_count = 1;
   bool retried;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_VBUFDRAWARRAYS);

   /* off to hardware */
   svga_vbuf_submit_state(svga_render);

   /* Need to call update_state() again as the draw module may have
    * altered some of our state behind our backs.  Testcase:
    * redbook/polys.c
    */
   svga_update_state_retry(svga, SVGA_STATE_HW_DRAW);
   SVGA_RETRY_CHECK(svga, svga_hwtnl_draw_arrays
                    (svga->hwtnl, svga_render->prim, start + bias,
                     nr, start_instance, instance_count, 0), retried);
   if (retried) {
      svga->swtnl.new_vbuf = true;
   }

   SVGA_STATS_TIME_POP(svga_sws(svga));
}


static void
svga_vbuf_render_draw_elements(struct vbuf_render *render,
                                const uint16_t *indices,
                                uint nr_indices)
{
   struct svga_vbuf_render *svga_render = svga_vbuf_render(render);
   struct svga_context *svga = svga_render->svga;
   int bias = (svga_render->vbuf_offset - svga_render->vdecl_offset)
      / svga_render->vertex_size;
   bool retried;
   /* instancing will already have been resolved at this point by 'draw' */
   const struct pipe_draw_info info = {
      .index_size = 2,
      .mode = svga_render->prim,
      .has_user_indices = 1,
      .index.user = indices,
      .start_instance = 0,
      .instance_count = 1,
      .index_bounds_valid = true,
      .min_index = svga_render->min_index,
      .max_index = svga_render->max_index,
   };
   const struct pipe_draw_start_count_bias draw = {
      .start = 0,
      .count = nr_indices,
      .index_bias = bias,
   };

   assert((svga_render->vbuf_offset - svga_render->vdecl_offset)
          % svga_render->vertex_size == 0);

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_VBUFDRAWELEMENTS);

   /* off to hardware */
   svga_vbuf_submit_state(svga_render);

   /* Need to call update_state() again as the draw module may have
    * altered some of our state behind our backs.  Testcase:
    * redbook/polys.c
    */
   svga_update_state_retry(svga, SVGA_STATE_HW_DRAW);
   SVGA_RETRY_CHECK(svga, svga_hwtnl_draw_range_elements(svga->hwtnl, &info,
                                                         &draw,
                                                         nr_indices), retried);
   if (retried) {
      svga->swtnl.new_vbuf = true;
   }

   SVGA_STATS_TIME_POP(svga_sws(svga));
}


static void
svga_vbuf_render_release_vertices(struct vbuf_render *render)
{

}


static void
svga_vbuf_render_destroy(struct vbuf_render *render)
{
   struct svga_vbuf_render *svga_render = svga_vbuf_render(render);

   pipe_resource_reference(&svga_render->vbuf, NULL);
   pipe_resource_reference(&svga_render->ibuf, NULL);
   FREE(svga_render);
}


/**
 * Create a new primitive render.
 */
struct vbuf_render *
svga_vbuf_render_create(struct svga_context *svga)
{
   struct svga_vbuf_render *svga_render = CALLOC_STRUCT(svga_vbuf_render);

   svga_render->svga = svga;
   svga_render->ibuf_size = 0;
   svga_render->vbuf_size = 0;
   svga_render->ibuf_alloc_size = 4*1024;
   svga_render->vbuf_alloc_size = 64*1024;
   svga_render->layout_id = SVGA3D_INVALID_ID;
   svga_render->base.max_vertex_buffer_bytes = 64*1024/10;
   svga_render->base.max_indices = 65536;
   svga_render->base.get_vertex_info = svga_vbuf_render_get_vertex_info;
   svga_render->base.allocate_vertices = svga_vbuf_render_allocate_vertices;
   svga_render->base.map_vertices = svga_vbuf_render_map_vertices;
   svga_render->base.unmap_vertices = svga_vbuf_render_unmap_vertices;
   svga_render->base.set_primitive = svga_vbuf_render_set_primitive;
   svga_render->base.draw_elements = svga_vbuf_render_draw_elements;
   svga_render->base.draw_arrays = svga_vbuf_render_draw_arrays;
   svga_render->base.release_vertices = svga_vbuf_render_release_vertices;
   svga_render->base.destroy = svga_vbuf_render_destroy;

   return &svga_render->base;
}
