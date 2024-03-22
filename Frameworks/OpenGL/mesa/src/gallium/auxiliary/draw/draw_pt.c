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

 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  */

#include "draw/draw_context.h"
#include "draw/draw_gs.h"
#include "draw/draw_tess.h"
#include "draw/draw_private.h"
#include "draw/draw_pt.h"
#include "draw/draw_vbuf.h"
#include "draw/draw_vs.h"
#include "tgsi/tgsi_dump.h"
#include "util/u_math.h"
#include "util/u_prim.h"
#include "util/format/u_format.h"
#include "util/u_draw.h"


DEBUG_GET_ONCE_BOOL_OPTION(draw_fse, "DRAW_FSE", false)
DEBUG_GET_ONCE_BOOL_OPTION(draw_no_fse, "DRAW_NO_FSE", false)


/* Overall we split things into:
 *     - frontend -- prepare fetch_elts, draw_elts - eg vsplit
 *     - middle   -- fetch, shade, cliptest, viewport
 *     - pipeline -- the prim pipeline: clipping, wide lines, etc
 *     - backend  -- the vbuf_render provided by the driver.
 */
static bool
draw_pt_arrays(struct draw_context *draw,
               enum mesa_prim prim,
               bool index_bias_varies,
               const struct pipe_draw_start_count_bias *draw_info,
               unsigned num_draws)
{
   enum mesa_prim out_prim = prim;

   if (draw->gs.geometry_shader)
      out_prim = draw->gs.geometry_shader->output_primitive;
   else if (draw->tes.tess_eval_shader)
      out_prim = get_tes_output_prim(draw->tes.tess_eval_shader);

   unsigned opt = PT_SHADE;
   if (!draw->render) {
      opt |= PT_PIPELINE;
   }

   if (draw_need_pipeline(draw, draw->rasterizer, out_prim)) {
      opt |= PT_PIPELINE;
   }

   if ((draw->clip_xy ||
         draw->clip_z ||
         draw->clip_user) && !draw->pt.test_fse) {
      opt |= PT_CLIPTEST;
   }

   struct draw_pt_middle_end *middle;
   if (draw->pt.middle.llvm) {
      middle = draw->pt.middle.llvm;
   } else {
      if (opt == PT_SHADE && !draw->pt.no_fse)
         middle = draw->pt.middle.fetch_shade_emit;
      else
         middle = draw->pt.middle.general;
   }

   struct draw_pt_front_end *frontend = draw->pt.frontend;
   if (frontend) {
      if (draw->pt.prim != prim || draw->pt.opt != opt) {
         /* In certain conditions switching primitives requires us to flush
          * and validate the different stages. One example is when smooth
          * lines are active but first drawn with triangles and then with
          * lines.
          */
         draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);
         frontend = NULL;
      } else if (draw->pt.eltSize != draw->pt.user.eltSize || draw->pt.viewid != draw->pt.user.viewid) {
         /* Flush draw state if eltSize or viewid changed.
          * eltSize changes could be improved so only the frontend is flushed since it
          * converts all indices to ushorts and the fetch part of the middle
          * always prepares both linear and indexed.
          */
         frontend->flush(frontend, DRAW_FLUSH_STATE_CHANGE);
         frontend = NULL;
      }
   }

   if (!frontend) {
      frontend = draw->pt.front.vsplit;

      frontend->prepare(frontend, prim, middle, opt);

      draw->pt.frontend = frontend;
      draw->pt.eltSize = draw->pt.user.eltSize;
      draw->pt.viewid = draw->pt.user.viewid;
      draw->pt.prim = prim;
      draw->pt.opt = opt;
   }

   if (draw->pt.rebind_parameters) {
      /* update constants, viewport dims, clip planes, etc */
      middle->bind_parameters(middle);
      draw->pt.rebind_parameters = false;
   }

   for (unsigned i = 0; i < num_draws; i++) {
      /* Sanitize primitive length:
       */
      unsigned first, incr;

      if (prim == MESA_PRIM_PATCHES) {
         first = draw->pt.vertices_per_patch;
         incr = draw->pt.vertices_per_patch;
      } else {
         draw_pt_split_prim(prim, &first, &incr);
      }

      unsigned count = draw_pt_trim_count(draw_info[i].count, first, incr);
      if (draw->pt.user.eltSize) {
         if (index_bias_varies) {
            draw->pt.user.eltBias = draw_info[i].index_bias;
         } else {
            draw->pt.user.eltBias = draw_info[0].index_bias;
         }
      } else {
         draw->pt.user.eltBias = 0;
      }

      draw->start_index = draw_info[i].start;

      if (count >= first)
         frontend->run(frontend, draw_info[i].start, count);

      if (num_draws > 1 && draw->pt.user.increment_draw_id)
         draw->pt.user.drawid++;
   }

   return true;
}


void
draw_pt_flush(struct draw_context *draw, unsigned flags)
{
   assert(flags);

   if (draw->pt.frontend) {
      draw->pt.frontend->flush(draw->pt.frontend, flags);

      /* don't prepare if we only are flushing the backend */
      if (flags & DRAW_FLUSH_STATE_CHANGE)
         draw->pt.frontend = NULL;
   }

   if (flags & DRAW_FLUSH_PARAMETER_CHANGE) {
      draw->pt.rebind_parameters = true;
   }
}


bool
draw_pt_init(struct draw_context *draw)
{
   draw->pt.test_fse = debug_get_option_draw_fse();
   draw->pt.no_fse = debug_get_option_draw_no_fse();

   draw->pt.front.vsplit = draw_pt_vsplit(draw);
   if (!draw->pt.front.vsplit)
      return false;

   draw->pt.middle.fetch_shade_emit = draw_pt_middle_fse(draw);
   if (!draw->pt.middle.fetch_shade_emit)
      return false;

   draw->pt.middle.general = draw_pt_fetch_pipeline_or_emit(draw);
   if (!draw->pt.middle.general)
      return false;

#if DRAW_LLVM_AVAILABLE
   if (draw->llvm) {
      draw->pt.middle.llvm = draw_pt_fetch_pipeline_or_emit_llvm(draw);
      draw->pt.middle.mesh = draw_pt_mesh_pipeline_or_emit(draw);
   }
#endif

   return true;
}


void
draw_pt_destroy(struct draw_context *draw)
{
   if (draw->pt.middle.mesh) {
      draw->pt.middle.mesh->destroy(draw->pt.middle.mesh);
      draw->pt.middle.mesh = NULL;
   }

   if (draw->pt.middle.llvm) {
      draw->pt.middle.llvm->destroy(draw->pt.middle.llvm);
      draw->pt.middle.llvm = NULL;
   }

   if (draw->pt.middle.general) {
      draw->pt.middle.general->destroy(draw->pt.middle.general);
      draw->pt.middle.general = NULL;
   }

   if (draw->pt.middle.fetch_shade_emit) {
      draw->pt.middle.fetch_shade_emit->destroy(draw->pt.middle.fetch_shade_emit);
      draw->pt.middle.fetch_shade_emit = NULL;
   }

   if (draw->pt.front.vsplit) {
      draw->pt.front.vsplit->destroy(draw->pt.front.vsplit);
      draw->pt.front.vsplit = NULL;
   }
}


/**
 * Debug- print the first 'count' vertices.
 */
static void
draw_print_arrays(struct draw_context *draw, enum mesa_prim prim,
                  int start, unsigned count, int index_bias)
{
   debug_printf("Draw arrays(prim = %u, start = %u, count = %u)\n",
                prim, start, count);

   for (unsigned i = 0; i < count; i++) {
      unsigned ii = 0;

      if (draw->pt.user.eltSize) {
         /* indexed arrays */

         switch (draw->pt.user.eltSize) {
         case 1:
            {
               const uint8_t *elem = (const uint8_t *) draw->pt.user.elts;
               ii = elem[start + i];
            }
            break;
         case 2:
            {
               const uint16_t *elem = (const uint16_t *) draw->pt.user.elts;
               ii = elem[start + i];
            }
            break;
         case 4:
            {
               const uint32_t *elem = (const uint32_t *) draw->pt.user.elts;
               ii = elem[start + i];
            }
            break;
         default:
            assert(0);
            return;
         }
         ii += index_bias;
         debug_printf("Element[%u + %u] + %i -> Vertex %u:\n", start, i,
                      index_bias, ii);
      }
      else {
         /* non-indexed arrays */
         ii = start + i;
         debug_printf("Vertex %u:\n", ii);
      }

      for (unsigned j = 0; j < draw->pt.nr_vertex_elements; j++) {
         unsigned buf = draw->pt.vertex_element[j].vertex_buffer_index;
         uint8_t *ptr = (uint8_t *) draw->pt.user.vbuffer[buf].map;

         if (draw->pt.vertex_element[j].instance_divisor) {
            ii = draw->instance_id / draw->pt.vertex_element[j].instance_divisor;
         }

         ptr += draw->pt.vertex_buffer[buf].buffer_offset;
         ptr += draw->pt.vertex_element[j].src_stride * ii;
         ptr += draw->pt.vertex_element[j].src_offset;

         debug_printf("  Attr %u: ", j);
         switch (draw->pt.vertex_element[j].src_format) {
         case PIPE_FORMAT_R32_FLOAT:
            {
               float *v = (float *) ptr;
               debug_printf("R %f  @ %p\n", v[0], (void *) v);
            }
            break;
         case PIPE_FORMAT_R32G32_FLOAT:
            {
               float *v = (float *) ptr;
               debug_printf("RG %f %f  @ %p\n", v[0], v[1], (void *) v);
            }
            break;
         case PIPE_FORMAT_R32G32B32_FLOAT:
            {
               float *v = (float *) ptr;
               debug_printf("RGB %f %f %f  @ %p\n", v[0], v[1], v[2], (void *) v);
            }
            break;
         case PIPE_FORMAT_R32G32B32A32_FLOAT:
            {
               float *v = (float *) ptr;
               debug_printf("RGBA %f %f %f %f  @ %p\n", v[0], v[1], v[2], v[3],
                            (void *) v);
            }
            break;
         case PIPE_FORMAT_B8G8R8A8_UNORM:
            {
               uint8_t *u = (uint8_t *) ptr;
               debug_printf("BGRA %d %d %d %d  @ %p\n", u[0], u[1], u[2], u[3],
                            (void *) u);
            }
            break;
         case PIPE_FORMAT_A8R8G8B8_UNORM:
            {
               uint8_t *u = (uint8_t *) ptr;
               debug_printf("ARGB %d %d %d %d  @ %p\n", u[0], u[1], u[2], u[3],
                            (void *) u);
            }
            break;
         default:
            debug_printf("other format %s (fix me)\n",
                     util_format_name(draw->pt.vertex_element[j].src_format));
         }
      }
   }
}


/** Helper code for below */
static inline void
prim_restart_loop(struct draw_context *draw,
                  const struct pipe_draw_info *info,
                  const struct pipe_draw_start_count_bias *draw_info,
                  const void *elements)
{
   const unsigned elt_max = draw->pt.user.eltMax;
   struct pipe_draw_start_count_bias cur = *draw_info;
   cur.count = 0;

   for (unsigned j = 0; j < draw_info->count; j++) {
      unsigned index = 0;
      unsigned i = util_clamped_uadd(draw_info->start, j);
      if (i < elt_max) {
         switch (draw->pt.user.eltSize) {
         case 1:
            index = ((const uint8_t*)elements)[i];
            break;
         case 2:
            index = ((const uint16_t*)elements)[i];
            break;
         case 4:
            index = ((const uint32_t*)elements)[i];
            break;
         default:
            assert(0 && "bad eltSize in draw_arrays()");
         }
      }

      if (index == info->restart_index) {
         if (cur.count > 0) {
            /* draw elts up to prev pos */
            draw_pt_arrays(draw, info->mode, info->index_bias_varies, &cur, 1);
         }
         /* begin new prim at next elt */
         cur.start = i + 1;
         cur.count = 0;
      }
      else {
         cur.count++;
      }
   }
   if (cur.count > 0) {
      draw_pt_arrays(draw, info->mode, info->index_bias_varies, &cur, 1);
   }
}


/**
 * For drawing prims with primitive restart enabled.
 * Scan for restart indexes and draw the runs of elements/vertices between
 * the restarts.
 */
static void
draw_pt_arrays_restart(struct draw_context *draw,
                       const struct pipe_draw_info *info,
                       const struct pipe_draw_start_count_bias *draw_info,
                       unsigned num_draws)
{
   assert(info->primitive_restart);

   if (draw->pt.user.eltSize) {
      /* indexed prims (draw_elements) */
      for (unsigned i = 0; i < num_draws; i++)
         prim_restart_loop(draw, info, &draw_info[i], draw->pt.user.elts);
   } else {
      /* Non-indexed prims (draw_arrays).
       * Primitive restart should have been handled in gallium frontends.
       */
      draw_pt_arrays(draw, info->mode, info->index_bias_varies, draw_info, num_draws);
   }
}


/**
 * Resolve true values within pipe_draw_info.
 * If we're rendering from transform feedback/stream output
 * buffers both the count and max_index need to be computed
 * from the attached stream output target.
 */
static void
resolve_draw_info(const struct pipe_draw_info *raw_info,
                  const struct pipe_draw_indirect_info *indirect,
                  const struct pipe_draw_start_count_bias *raw_draw,
                  struct pipe_draw_info *info,
                  struct pipe_draw_start_count_bias *draw,
                  struct pipe_vertex_buffer *vertex_buffer,
                  struct pipe_vertex_element *vertex_element)
{
   *info = *raw_info;
   *draw = *raw_draw;

   struct draw_so_target *target =
      (struct draw_so_target *)indirect->count_from_stream_output;
   assert(vertex_buffer != NULL);
   draw->count = vertex_element->src_stride == 0 ? 0 :
                    target->internal_offset / vertex_element->src_stride;

   /* Stream output draw can not be indexed */
   assert(!info->index_size);
   info->max_index = draw->count - 1;
}


/*
 * Loop over all instances and execute draws for them.
 */
static void
draw_instances(struct draw_context *draw,
               unsigned drawid_offset,
               const struct pipe_draw_info *info,
               const struct pipe_draw_start_count_bias *draws,
               unsigned num_draws)
{
   draw->start_instance = info->start_instance;

   for (unsigned instance = 0; instance < info->instance_count; instance++) {
      unsigned instance_idx = instance + info->start_instance;
      draw->instance_id = instance;
      /* check for overflow */
      if (instance_idx < instance ||
          instance_idx < draw->start_instance) {
         /* if we overflown just set the instance id to the max */
         draw->instance_id = 0xffffffff;
      }

      draw->pt.user.drawid = drawid_offset;
      draw_new_instance(draw);

      if (info->primitive_restart) {
         draw_pt_arrays_restart(draw, info, draws, num_draws);
      } else {
         draw_pt_arrays(draw, info->mode, info->index_bias_varies,
                        draws, num_draws);
      }
   }
}


/**
 * Draw vertex arrays.
 * This is the main entrypoint into the drawing module.  If drawing an indexed
 * primitive, the draw_set_indexes() function should have already been called
 * to specify the element/index buffer information.
 */
void
draw_vbo(struct draw_context *draw,
         const struct pipe_draw_info *info,
         unsigned drawid_offset,
         const struct pipe_draw_indirect_info *indirect,
         const struct pipe_draw_start_count_bias *draws,
         unsigned num_draws,
         uint8_t patch_vertices)
{
   unsigned fpstate = util_fpstate_get();
   struct pipe_draw_info resolved_info;
   struct pipe_draw_start_count_bias resolved_draw;
   const struct pipe_draw_info *use_info;
   const struct pipe_draw_start_count_bias *use_draws;

   if (info->instance_count == 0)
      return;

   /* Make sure that denorms are treated like zeros. This is
    * the behavior required by D3D10. OpenGL doesn't care.
    */
   util_fpstate_set_denorms_to_zero(fpstate);

   if (indirect && indirect->count_from_stream_output) {
      resolve_draw_info(info, indirect, &draws[0], &resolved_info,
                        &resolved_draw, &(draw->pt.vertex_buffer[0]),
                        &(draw->pt.vertex_element[0]));
      use_info = &resolved_info;
      use_draws = &resolved_draw;
      num_draws = 1;
   } else {
      use_info = info;
      use_draws = draws;
   }

   if (info->index_size) {
      assert(draw->pt.user.elts);
      draw->pt.user.min_index = use_info->index_bounds_valid ? use_info->min_index : 0;
      draw->pt.user.max_index = use_info->index_bounds_valid ? use_info->max_index : ~0;
   } else {
      draw->pt.user.min_index = 0;
      draw->pt.user.max_index = ~0;
   }
   draw->pt.user.eltSize = use_info->index_size ? draw->pt.user.eltSizeIB : 0;
   draw->pt.user.drawid = drawid_offset;
   draw->pt.user.increment_draw_id = use_info->increment_draw_id;
   draw->pt.user.viewid = 0;
   draw->pt.vertices_per_patch = patch_vertices;

   if (0) {
      for (unsigned i = 0; i < num_draws; i++)
         debug_printf("draw_vbo(mode=%u start=%u count=%u):\n",
                      use_info->mode, use_draws[i].start, use_draws[i].count);
   }

   if (0)
      tgsi_dump(draw->vs.vertex_shader->state.tokens, 0);

   if (0) {
      debug_printf("Elements:\n");
      for (unsigned i = 0; i < draw->pt.nr_vertex_elements; i++) {
         debug_printf("  %u: src_offset=%u src_stride=%u inst_div=%u   vbuf=%u  format=%s\n",
                      i,
                      draw->pt.vertex_element[i].src_offset,
                      draw->pt.vertex_element[i].src_stride,
                      draw->pt.vertex_element[i].instance_divisor,
                      draw->pt.vertex_element[i].vertex_buffer_index,
                      util_format_name(draw->pt.vertex_element[i].src_format));
      }
      debug_printf("Buffers:\n");
      for (unsigned i = 0; i < draw->pt.nr_vertex_buffers; i++) {
         debug_printf("  %u: offset=%u size=%d ptr=%p\n",
                      i,
                      draw->pt.vertex_buffer[i].buffer_offset,
                      (int) draw->pt.user.vbuffer[i].size,
                      draw->pt.user.vbuffer[i].map);
      }
   }

   if (0) {
      for (unsigned i = 0; i < num_draws; i++)
         draw_print_arrays(draw, use_info->mode, use_draws[i].start,
                           MIN2(use_draws[i].count, 20),
                           use_info->index_bias_varies
                           ? use_draws[i].index_bias
                           : use_draws[0].index_bias);
   }

   unsigned index_limit = util_draw_max_index(draw->pt.vertex_buffer,
                                              draw->pt.vertex_element,
                                              draw->pt.nr_vertex_elements,
                                              use_info);
#if DRAW_LLVM_AVAILABLE
   if (!draw->llvm)
#endif
   {
      if (index_limit == 0) {
         /* one of the buffers is too small to do any valid drawing */
         debug_warning("draw: VBO too small to draw anything\n");
         util_fpstate_set(fpstate);
         return;
      }
   }

   /* If we're collecting stats then make sure we start from scratch */
   if (draw->collect_statistics) {
      memset(&draw->statistics, 0, sizeof(draw->statistics));
   }

   draw->pt.max_index = index_limit - 1;

   /*
    * TODO: We could use draw->pt.max_index to further narrow
    * the min_index/max_index hints given by gallium frontends.
    */

   if (use_info->view_mask) {
      u_foreach_bit(i, use_info->view_mask) {
         draw->pt.user.viewid = i;
         draw_instances(draw, drawid_offset, use_info, use_draws, num_draws);
      }
   } else {
      draw_instances(draw, drawid_offset, use_info, use_draws, num_draws);
   }

   /* If requested emit the pipeline statistics for this run */
   if (draw->collect_statistics) {
      draw->render->pipeline_statistics(draw->render, &draw->statistics);
   }
   util_fpstate_set(fpstate);
}

/* to be called after a mesh shader is run */
void
draw_mesh(struct draw_context *draw,
          struct draw_vertex_info *vert_info,
          struct draw_prim_info *prim_info)
{
   struct draw_pt_middle_end *middle = draw->pt.middle.mesh;

   draw->pt.user.eltSize = 0;
   draw->pt.user.viewid = 0;
   draw->pt.user.drawid = 0;
   draw->pt.user.increment_draw_id = false;
   draw->pt.vertices_per_patch = 0;

   middle->prepare(middle, 0, 0, NULL);

   draw_mesh_middle_end_run(middle, vert_info, prim_info);
}
