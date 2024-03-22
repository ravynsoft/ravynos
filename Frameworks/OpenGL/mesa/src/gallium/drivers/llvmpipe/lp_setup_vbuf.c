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

/**
 * Interface between 'draw' module's output and the llvmpipe rasterizer/setup
 * code.  When the 'draw' module has finished filling a vertex buffer, the
 * draw_arrays() functions below will be called.  Loop over the vertices and
 * call the point/line/tri setup functions.
 *
 * Authors
 *  Brian Paul
 */


#include "lp_setup_context.h"
#include "lp_context.h"
#include "draw/draw_vbuf.h"
#include "draw/draw_vertex.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "lp_state_fs.h"
#include "lp_perf.h"


/* It should be a multiple of both 6 and 4 (in other words, a multiple of 12)
 * to ensure draw splits between a whole number of rectangles.
 */
#define LP_MAX_VBUF_INDEXES 1020

#define LP_MAX_VBUF_SIZE    4096



/** cast wrapper */
static struct lp_setup_context *
lp_setup_context(struct vbuf_render *vbr)
{
   return (struct lp_setup_context *) vbr;
}



static const struct vertex_info *
lp_setup_get_vertex_info(struct vbuf_render *vbr)
{
   struct lp_setup_context *setup = lp_setup_context(vbr);

   /* Vertex size/info depends on the latest state.
    * The draw module may have issued additional state-change commands.
    */
   lp_setup_update_state(setup, false);

   return setup->vertex_info;
}


static bool
lp_setup_allocate_vertices(struct vbuf_render *vbr,
                          uint16_t vertex_size, uint16_t nr_vertices)
{
   struct lp_setup_context *setup = lp_setup_context(vbr);
   unsigned size = vertex_size * nr_vertices;

   if (setup->vertex_buffer_size < size) {
      align_free(setup->vertex_buffer);
      setup->vertex_buffer = align_malloc(size, 16);
      setup->vertex_buffer_size = size;
   }

   setup->vertex_size = vertex_size;
   setup->nr_vertices = nr_vertices;

   return setup->vertex_buffer != NULL;
}


static void
lp_setup_release_vertices(struct vbuf_render *vbr)
{
   /* keep the old allocation for next time */
}


static void *
lp_setup_map_vertices(struct vbuf_render *vbr)
{
   struct lp_setup_context *setup = lp_setup_context(vbr);
   return setup->vertex_buffer;
}


static void
lp_setup_unmap_vertices(struct vbuf_render *vbr,
                        uint16_t min_index,
                        uint16_t max_index)
{
   ASSERTED struct lp_setup_context *setup = lp_setup_context(vbr);
   assert(setup->vertex_buffer_size >= (max_index+1) * setup->vertex_size);
   /* do nothing */
}


static void
lp_setup_set_primitive(struct vbuf_render *vbr, enum mesa_prim prim)
{
   lp_setup_context(vbr)->prim = prim;
}


static void
lp_setup_set_view_index(struct vbuf_render *vbr, unsigned view_index)
{
   lp_setup_context(vbr)->view_index = view_index;
}


typedef const float (*const_float4_ptr)[4];


static inline const_float4_ptr
get_vert(const void *vertex_buffer, int index, int stride)
{
   return (const_float4_ptr)((char *)vertex_buffer + index * stride);
}


static inline void
rect(struct lp_setup_context *setup,
     const float (*v0)[4],
     const float (*v1)[4],
     const float (*v2)[4],
     const float (*v3)[4],
     const float (*v4)[4],
     const float (*v5)[4])
{
   if (!setup->permit_linear_rasterizer ||
       !setup->rect(setup, v0, v1, v2, v3, v4, v5)) {
      setup->triangle(setup, v0, v1, v2);
      setup->triangle(setup, v3, v4, v5);
   }
}


/**
 * draw elements / indexed primitives
 */
static void
lp_setup_draw_elements(struct vbuf_render *vbr, const uint16_t *indices, uint nr)
{
   struct lp_setup_context *setup = lp_setup_context(vbr);
   const unsigned stride = setup->vertex_info->size * sizeof(float);
   const void *vertex_buffer = setup->vertex_buffer;
   const bool flatshade_first = setup->flatshade_first;
   unsigned i;

   assert(setup->setup.variant);

   if (!lp_setup_update_state(setup, true))
      return;

   const bool uses_constant_interp =
      setup->setup.variant->key.uses_constant_interp;

   switch (setup->prim) {
   case MESA_PRIM_POINTS:
      for (i = 0; i < nr; i++) {
         setup->point(setup,
                      get_vert(vertex_buffer, indices[i-0], stride));
      }
      break;

   case MESA_PRIM_LINES:
      for (i = 1; i < nr; i += 2) {
         setup->line(setup,
                     get_vert(vertex_buffer, indices[i-1], stride),
                     get_vert(vertex_buffer, indices[i-0], stride));
      }
      break;

   case MESA_PRIM_LINE_STRIP:
      for (i = 1; i < nr; i ++) {
         setup->line(setup,
                     get_vert(vertex_buffer, indices[i-1], stride),
                     get_vert(vertex_buffer, indices[i-0], stride));
      }
      break;

   case MESA_PRIM_LINE_LOOP:
      for (i = 1; i < nr; i ++) {
         setup->line(setup,
                     get_vert(vertex_buffer, indices[i-1], stride),
                     get_vert(vertex_buffer, indices[i-0], stride));
      }
      if (nr) {
         setup->line(setup,
                     get_vert(vertex_buffer, indices[nr-1], stride),
                     get_vert(vertex_buffer, indices[0], stride));
      }
      break;

   case MESA_PRIM_TRIANGLES:
      if (nr % 6 == 0 && !uses_constant_interp) {
         for (i = 5; i < nr; i += 6) {
            rect(setup,
                 get_vert(vertex_buffer, indices[i-5], stride),
                 get_vert(vertex_buffer, indices[i-4], stride),
                 get_vert(vertex_buffer, indices[i-3], stride),
                 get_vert(vertex_buffer, indices[i-2], stride),
                 get_vert(vertex_buffer, indices[i-1], stride),
                 get_vert(vertex_buffer, indices[i-0], stride));
         }
      } else {
         for (i = 2; i < nr; i += 3) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-2], stride),
                            get_vert(vertex_buffer, indices[i-1], stride),
                            get_vert(vertex_buffer, indices[i-0], stride));
         }
      }
      break;

   case MESA_PRIM_TRIANGLE_STRIP:
      if (flatshade_first) {
         for (i = 2; i < nr; i += 1) {
            /* emit first triangle vertex as first triangle vertex */
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-2], stride),
                            get_vert(vertex_buffer, indices[i+(i&1)-1], stride),
                            get_vert(vertex_buffer, indices[i-(i&1)], stride));

         }
      } else {
         for (i = 2; i < nr; i += 1) {
            /* emit last triangle vertex as last triangle vertex */
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i+(i&1)-2], stride),
                            get_vert(vertex_buffer, indices[i-(i&1)-1], stride),
                            get_vert(vertex_buffer, indices[i-0], stride));
         }
      }
      break;

   case MESA_PRIM_TRIANGLE_FAN:
      if (flatshade_first) {
         for (i = 2; i < nr; i += 1) {
            /* emit first non-spoke vertex as first vertex */
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-1], stride),
                            get_vert(vertex_buffer, indices[i-0], stride),
                            get_vert(vertex_buffer, indices[0], stride));
         }
      } else {
         for (i = 2; i < nr; i += 1) {
            /* emit last non-spoke vertex as last vertex */
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[0], stride),
                            get_vert(vertex_buffer, indices[i-1], stride),
                            get_vert(vertex_buffer, indices[i-0], stride));
         }
      }
      break;

   case MESA_PRIM_QUADS:
      /* GL quads don't follow provoking vertex convention */
      if (flatshade_first) {
         /* emit last quad vertex as first triangle vertex */
         for (i = 3; i < nr; i += 4) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-0], stride),
                            get_vert(vertex_buffer, indices[i-3], stride),
                            get_vert(vertex_buffer, indices[i-2], stride));

            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-0], stride),
                            get_vert(vertex_buffer, indices[i-2], stride),
                            get_vert(vertex_buffer, indices[i-1], stride));
         }
      } else {
         /* emit last quad vertex as last triangle vertex */
         for (i = 3; i < nr; i += 4) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-3], stride),
                            get_vert(vertex_buffer, indices[i-2], stride),
                            get_vert(vertex_buffer, indices[i-0], stride));

            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-2], stride),
                            get_vert(vertex_buffer, indices[i-1], stride),
                            get_vert(vertex_buffer, indices[i-0], stride));
         }
      }
      break;

   case MESA_PRIM_QUAD_STRIP:
      /* GL quad strips don't follow provoking vertex convention */
      if (flatshade_first) {
         /* emit last quad vertex as first triangle vertex */
         for (i = 3; i < nr; i += 2) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-0], stride),
                            get_vert(vertex_buffer, indices[i-3], stride),
                            get_vert(vertex_buffer, indices[i-2], stride));
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-0], stride),
                            get_vert(vertex_buffer, indices[i-1], stride),
                            get_vert(vertex_buffer, indices[i-3], stride));
         }
      } else {
         /* emit last quad vertex as last triangle vertex */
         for (i = 3; i < nr; i += 2) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-3], stride),
                            get_vert(vertex_buffer, indices[i-2], stride),
                            get_vert(vertex_buffer, indices[i-0], stride));
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-1], stride),
                            get_vert(vertex_buffer, indices[i-3], stride),
                            get_vert(vertex_buffer, indices[i-0], stride));
         }
      }
      break;

   case MESA_PRIM_POLYGON:
      /* Almost same as tri fan but the _first_ vertex specifies the flat
       * shading color.
       */
      if (flatshade_first) {
         /* emit first polygon  vertex as first triangle vertex */
         for (i = 2; i < nr; i += 1) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[0], stride),
                            get_vert(vertex_buffer, indices[i-1], stride),
                            get_vert(vertex_buffer, indices[i-0], stride));
         }
      } else {
         /* emit first polygon  vertex as last triangle vertex */
         for (i = 2; i < nr; i += 1) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, indices[i-1], stride),
                            get_vert(vertex_buffer, indices[i-0], stride),
                            get_vert(vertex_buffer, indices[0], stride));
         }
      }
      break;

   default:
      assert(0);
   }
}


/**
 * This function is hit when the draw module is working in pass-through mode.
 * It's up to us to convert the vertex array into point/line/tri prims.
 */
static void
lp_setup_draw_arrays(struct vbuf_render *vbr, uint start, uint nr)
{
   struct lp_setup_context *setup = lp_setup_context(vbr);
   const unsigned stride = setup->vertex_info->size * sizeof(float);
   const void *vertex_buffer =
      (void *) get_vert(setup->vertex_buffer, start, stride);
   const bool flatshade_first = setup->flatshade_first;
   unsigned i;

   if (!lp_setup_update_state(setup, true))
      return;

   const bool uses_constant_interp =
      setup->setup.variant->key.uses_constant_interp;

   switch (setup->prim) {
   case MESA_PRIM_POINTS:
      for (i = 0; i < nr; i++) {
         setup->point(setup,
                      get_vert(vertex_buffer, i-0, stride));
      }
      break;

   case MESA_PRIM_LINES:
      for (i = 1; i < nr; i += 2) {
         setup->line(setup,
                     get_vert(vertex_buffer, i-1, stride),
                     get_vert(vertex_buffer, i-0, stride));
      }
      break;

   case MESA_PRIM_LINE_STRIP:
      for (i = 1; i < nr; i ++) {
         setup->line(setup,
                     get_vert(vertex_buffer, i-1, stride),
                     get_vert(vertex_buffer, i-0, stride));
      }
      break;

   case MESA_PRIM_LINE_LOOP:
      for (i = 1; i < nr; i ++) {
         setup->line(setup,
                     get_vert(vertex_buffer, i-1, stride),
                     get_vert(vertex_buffer, i-0, stride));
      }
      if (nr) {
         setup->line(setup,
                     get_vert(vertex_buffer, nr-1, stride),
                     get_vert(vertex_buffer, 0, stride));
      }
      break;

   case MESA_PRIM_TRIANGLES:
      if (nr % 6 == 0 && !uses_constant_interp) {
         for (i = 5; i < nr; i += 6) {
            rect(setup,
                 get_vert(vertex_buffer, i-5, stride),
                 get_vert(vertex_buffer, i-4, stride),
                 get_vert(vertex_buffer, i-3, stride),
                 get_vert(vertex_buffer, i-2, stride),
                 get_vert(vertex_buffer, i-1, stride),
                 get_vert(vertex_buffer, i-0, stride));
         }
      } else if (!uses_constant_interp &&
               lp_setup_analyse_triangles(setup, vertex_buffer, stride, nr)) {
         /* If lp_setup_analyse_triangles() returned true, it also
          * emitted (setup) the rect or triangles.
          */
      } else {
         for (i = 2; i < nr; i += 3) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, i-2, stride),
                            get_vert(vertex_buffer, i-1, stride),
                            get_vert(vertex_buffer, i-0, stride));
         }
      }
      break;

   case MESA_PRIM_TRIANGLE_STRIP:
      if (flatshade_first) {
         if (!uses_constant_interp) {
            int j;
            i = 2;
            j = 3;
            while (j < nr) {
               /* emit first triangle vertex as first triangle vertex */
               const float (*v0)[4] = get_vert(vertex_buffer, i-2, stride);
               const float (*v1)[4] = get_vert(vertex_buffer, i+(i&1)-1, stride);
               const float (*v2)[4] = get_vert(vertex_buffer, i-(i&1), stride);
               const float (*v3)[4] = get_vert(vertex_buffer, j-2, stride);
               const float (*v4)[4] = get_vert(vertex_buffer, j+(j&1)-1, stride);
               const float (*v5)[4] = get_vert(vertex_buffer, j-(j&1), stride);
               if (setup->permit_linear_rasterizer &&
                   setup->rect(setup, v0, v1, v2, v3, v4, v5)) {
                  i += 2;
                  j += 2;
               } else {
                  /* emit one triangle, and retry rectangle in the next one */
                  setup->triangle(setup, v0, v1, v2);
                  i += 1;
                  j += 1;
               }
            }
            if (i < nr) {
               /* emit last triangle */
               setup->triangle(setup,
                               get_vert(vertex_buffer, i-2, stride),
                               get_vert(vertex_buffer, i+(i&1)-1, stride),
                               get_vert(vertex_buffer, i-(i&1), stride));
            }
         } else {
            for (i = 2; i < nr; i++) {
               /* emit first triangle vertex as first triangle vertex */
               setup->triangle(setup,
                               get_vert(vertex_buffer, i-2, stride),
                               get_vert(vertex_buffer, i+(i&1)-1, stride),
                               get_vert(vertex_buffer, i-(i&1), stride));
            }
         }
      } else {
         for (i = 2; i < nr; i++) {
            /* emit last triangle vertex as last triangle vertex */
            setup->triangle(setup,
                            get_vert(vertex_buffer, i+(i&1)-2, stride),
                            get_vert(vertex_buffer, i-(i&1)-1, stride),
                            get_vert(vertex_buffer, i-0, stride));
         }
      }
      break;

   case MESA_PRIM_TRIANGLE_FAN:
      if (nr == 4 && !uses_constant_interp) {
         rect(setup,
                      get_vert(vertex_buffer, 0, stride),
                      get_vert(vertex_buffer, 1, stride),
                      get_vert(vertex_buffer, 2, stride),
                      get_vert(vertex_buffer, 0, stride),
                      get_vert(vertex_buffer, 2, stride),
                      get_vert(vertex_buffer, 3, stride));
      } else if (flatshade_first) {
         for (i = 2; i < nr; i += 1) {
            /* emit first non-spoke vertex as first vertex */
            setup->triangle(setup,
                            get_vert(vertex_buffer, i-1, stride),
                            get_vert(vertex_buffer, i-0, stride),
                            get_vert(vertex_buffer, 0, stride));
         }
      } else {
         for (i = 2; i < nr; i += 1) {
            /* emit last non-spoke vertex as last vertex */
            setup->triangle(setup,
                            get_vert(vertex_buffer, 0, stride),
                            get_vert(vertex_buffer, i-1, stride),
                            get_vert(vertex_buffer, i-0, stride));
         }
      }
      break;

   case MESA_PRIM_QUADS:
      /* GL quads don't follow provoking vertex convention */
      if (flatshade_first) {
         /* emit last quad vertex as first triangle vertex */
         for (i = 3; i < nr; i += 4) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, i-0, stride),
                            get_vert(vertex_buffer, i-3, stride),
                            get_vert(vertex_buffer, i-2, stride));
            setup->triangle(setup,
                            get_vert(vertex_buffer, i-0, stride),
                            get_vert(vertex_buffer, i-2, stride),
                            get_vert(vertex_buffer, i-1, stride));
         }
      } else {
         /* emit last quad vertex as last triangle vertex */
         if (!uses_constant_interp) {
            for (i = 3; i < nr; i += 4) {
               rect(setup,
                    get_vert(vertex_buffer, i-3, stride),
                    get_vert(vertex_buffer, i-2, stride),
                    get_vert(vertex_buffer, i-1, stride),
                    get_vert(vertex_buffer, i-3, stride),
                    get_vert(vertex_buffer, i-1, stride),
                    get_vert(vertex_buffer, i-0, stride));
            }
         } else {
            for (i = 3; i < nr; i += 4) {
               setup->triangle(setup,
                               get_vert(vertex_buffer, i-3, stride),
                               get_vert(vertex_buffer, i-2, stride),
                               get_vert(vertex_buffer, i-0, stride));
               setup->triangle(setup,
                               get_vert(vertex_buffer, i-2, stride),
                               get_vert(vertex_buffer, i-1, stride),
                               get_vert(vertex_buffer, i-0, stride));
            }
         }
      }
      break;

   case MESA_PRIM_QUAD_STRIP:
      /* GL quad strips don't follow provoking vertex convention */
      if (flatshade_first) {
         /* emit last quad vertex as first triangle vertex */
         for (i = 3; i < nr; i += 2) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, i-0, stride),
                            get_vert(vertex_buffer, i-3, stride),
                            get_vert(vertex_buffer, i-2, stride));
            setup->triangle(setup,
                            get_vert(vertex_buffer, i-0, stride),
                            get_vert(vertex_buffer, i-1, stride),
                            get_vert(vertex_buffer, i-3, stride));
         }
      } else {
         /* emit last quad vertex as last triangle vertex */
         for (i = 3; i < nr; i += 2) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, i-3, stride),
                            get_vert(vertex_buffer, i-2, stride),
                            get_vert(vertex_buffer, i-0, stride));
            setup->triangle(setup,
                            get_vert(vertex_buffer, i-1, stride),
                            get_vert(vertex_buffer, i-3, stride),
                            get_vert(vertex_buffer, i-0, stride));
         }
      }
      break;

   case MESA_PRIM_POLYGON:
      /* Almost same as tri fan but the _first_ vertex specifies the flat
       * shading color.
       */
      if (flatshade_first) {
         /* emit first polygon  vertex as first triangle vertex */
         for (i = 2; i < nr; i += 1) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, 0, stride),
                            get_vert(vertex_buffer, i-1, stride),
                            get_vert(vertex_buffer, i-0, stride));
         }
      } else {
         /* emit first polygon  vertex as last triangle vertex */
         for (i = 2; i < nr; i += 1) {
            setup->triangle(setup,
                            get_vert(vertex_buffer, i-1, stride),
                            get_vert(vertex_buffer, i-0, stride),
                            get_vert(vertex_buffer, 0, stride));
         }
      }
      break;

   default:
      assert(0);
   }
}


static void
lp_setup_vbuf_destroy(struct vbuf_render *vbr)
{
   struct lp_setup_context *setup = lp_setup_context(vbr);
   if (setup->vertex_buffer) {
      align_free(setup->vertex_buffer);
      setup->vertex_buffer = NULL;
   }
   lp_setup_destroy(setup);
}


/*
 * FIXME: it is unclear if primitives_storage_needed (which is generally
 * the same as pipe query num_primitives_generated) should increase
 * if SO is disabled for d3d10, but for GL we definitely need to
 * increase num_primitives_generated and this is only called for active
 * SO. If it must not increase for d3d10 need to disambiguate the counters
 * in the driver and do some work for getting correct values, if it should
 * increase too should call this from outside streamout code.
 */
static void
lp_setup_so_info(struct vbuf_render *vbr, uint stream,
                 uint primitives, uint prim_generated)
{
   struct lp_setup_context *setup = lp_setup_context(vbr);
   struct llvmpipe_context *lp = llvmpipe_context(setup->pipe);

   lp->so_stats[stream].num_primitives_written += primitives;
   lp->so_stats[stream].primitives_storage_needed += prim_generated;
}


static void
lp_setup_pipeline_statistics(
   struct vbuf_render *vbr,
   const struct pipe_query_data_pipeline_statistics *stats)
{
   struct lp_setup_context *setup = lp_setup_context(vbr);
   struct llvmpipe_context *llvmpipe = llvmpipe_context(setup->pipe);

   llvmpipe->pipeline_statistics.ia_vertices +=
      stats->ia_vertices;
   llvmpipe->pipeline_statistics.ia_primitives +=
      stats->ia_primitives;
   llvmpipe->pipeline_statistics.vs_invocations +=
      stats->vs_invocations;
   llvmpipe->pipeline_statistics.gs_invocations +=
      stats->gs_invocations;
   llvmpipe->pipeline_statistics.gs_primitives +=
      stats->gs_primitives;
   llvmpipe->pipeline_statistics.hs_invocations +=
      stats->hs_invocations;
   llvmpipe->pipeline_statistics.ds_invocations +=
      stats->ds_invocations;
   if (!setup->rasterizer_discard) {
      llvmpipe->pipeline_statistics.c_invocations +=
         stats->c_invocations;
   } else {
      llvmpipe->pipeline_statistics.c_invocations = 0;
   }
}


/**
 * Create the post-transform vertex handler for the given context.
 */
void
lp_setup_init_vbuf(struct lp_setup_context *setup)
{
   setup->base.max_indices = LP_MAX_VBUF_INDEXES;
   setup->base.max_vertex_buffer_bytes = LP_MAX_VBUF_SIZE;

   setup->base.get_vertex_info = lp_setup_get_vertex_info;
   setup->base.allocate_vertices = lp_setup_allocate_vertices;
   setup->base.map_vertices = lp_setup_map_vertices;
   setup->base.unmap_vertices = lp_setup_unmap_vertices;
   setup->base.set_primitive = lp_setup_set_primitive;
   setup->base.set_view_index = lp_setup_set_view_index;
   setup->base.draw_elements = lp_setup_draw_elements;
   setup->base.draw_arrays = lp_setup_draw_arrays;
   setup->base.release_vertices = lp_setup_release_vertices;
   setup->base.destroy = lp_setup_vbuf_destroy;
   setup->base.set_stream_output_info = lp_setup_so_info;
   setup->base.pipeline_statistics = lp_setup_pipeline_statistics;
}
