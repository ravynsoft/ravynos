/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
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

#include "draw/draw_private.h"
#include "draw/draw_vs.h"
#include "draw/draw_gs.h"
#include "draw/draw_tess.h"
#include "draw/draw_context.h"
#include "draw/draw_vbuf.h"
#include "draw/draw_vertex.h"
#include "draw/draw_pt.h"

#include "pipe/p_state.h"

#include "util/u_math.h"
#include "util/u_prim.h"
#include "util/u_memory.h"


struct pt_so_emit {
   struct draw_context *draw;

   unsigned input_vertex_stride;
   const float (*inputs)[4];
   const float *pre_clip_pos;
   bool has_so;
   bool use_pre_clip_pos;
   int pos_idx;
   unsigned emitted_primitives;
   unsigned generated_primitives;
   unsigned stream;
};


static const struct pipe_stream_output_info *
draw_so_info(const struct draw_context *draw)
{
   const struct pipe_stream_output_info *state = NULL;

   if (draw->ms.mesh_shader)
      return state;
   if (draw->gs.geometry_shader) {
      state = &draw->gs.geometry_shader->state.stream_output;
   } else if (draw->tes.tess_eval_shader) {
      state = &draw->tes.tess_eval_shader->state.stream_output;
   } else {
      state = &draw->vs.vertex_shader->state.stream_output;
   }

   return state;
}

static inline bool
draw_has_so(const struct draw_context *draw)
{
   const struct pipe_stream_output_info *state = draw_so_info(draw);

   if (state && state->num_outputs > 0)
      return true;

   return false;
}

void
draw_pt_so_emit_prepare(struct pt_so_emit *emit, bool use_pre_clip_pos)
{
   struct draw_context *draw = emit->draw;

   emit->use_pre_clip_pos = use_pre_clip_pos;
   emit->has_so = draw_has_so(draw);
   if (use_pre_clip_pos)
      emit->pos_idx = draw_current_shader_position_output(draw);

   /* if we have a state with outputs make sure we have
    * buffers to output to */
   if (emit->has_so) {
      bool has_valid_buffer = false;
      for (unsigned i = 0; i < draw->so.num_targets; ++i) {
         if (draw->so.targets[i]) {
            has_valid_buffer = true;
            break;
         }
      }
      emit->has_so = has_valid_buffer;
   }

   if (!emit->has_so)
      return;

   /* XXX: need to flush to get prim_vbuf.c to release its allocation??
    */
   draw_do_flush(draw, DRAW_FLUSH_BACKEND);
}


static void
so_emit_prim(struct pt_so_emit *so,
             unsigned *indices,
             unsigned num_vertices)
{
   unsigned input_vertex_stride = so->input_vertex_stride;
   struct draw_context *draw = so->draw;
   const float (*input_ptr)[4];
   const float *pcp_ptr = NULL;
   const struct pipe_stream_output_info *state = draw_so_info(draw);
   int buffer_total_bytes[PIPE_MAX_SO_BUFFERS];
   bool buffer_written[PIPE_MAX_SO_BUFFERS] = {0};

   input_ptr = so->inputs;
   if (so->use_pre_clip_pos)
      pcp_ptr = so->pre_clip_pos;

   ++so->generated_primitives;

   for (unsigned i = 0; i < draw->so.num_targets; i++) {
      struct draw_so_target *target = draw->so.targets[i];
      if (target) {
         buffer_total_bytes[i] = target->internal_offset;
      } else {
         buffer_total_bytes[i] = 0;
      }
   }

   /* check have we space to emit prim first - if not don't do anything */
   for (unsigned i = 0; i < num_vertices; ++i) {
      for (unsigned slot = 0; slot < state->num_outputs; ++slot) {
         unsigned num_comps = state->output[slot].num_components;
         int ob = state->output[slot].output_buffer;
         unsigned dst_offset = state->output[slot].dst_offset * sizeof(float);
         unsigned write_size = num_comps * sizeof(float);

         if (state->output[slot].stream != so->stream)
            continue;
         /* If a buffer is missing then that's equivalent to
          * an overflow */
         if (!draw->so.targets[ob]) {
            return;
         }
         if ((buffer_total_bytes[ob] + write_size + dst_offset) >
             draw->so.targets[ob]->target.buffer_size) {
            return;
         }
      }
      for (unsigned ob = 0; ob < draw->so.num_targets; ++ob) {
         buffer_total_bytes[ob] += state->stride[ob] * sizeof(float);
      }
   }

   for (unsigned i = 0; i < num_vertices; ++i) {
      const float (*input)[4];
      const float *pre_clip_pos = NULL;

      input = (const float (*)[4])(
         (const char *)input_ptr + (indices[i] * input_vertex_stride));

      if (pcp_ptr)
         pre_clip_pos = (const float *)(
         (const char *)pcp_ptr + (indices[i] * input_vertex_stride));

      for (unsigned slot = 0; slot < state->num_outputs; ++slot) {
         unsigned idx = state->output[slot].register_index;
         unsigned start_comp = state->output[slot].start_component;
         unsigned num_comps = state->output[slot].num_components;
         unsigned stream = state->output[slot].stream;

         if (stream != so->stream)
            continue;

         unsigned ob = state->output[slot].output_buffer;
         buffer_written[ob] = true;

         float *buffer = (float *)((char *)draw->so.targets[ob]->mapping +
                            draw->so.targets[ob]->target.buffer_offset +
                            draw->so.targets[ob]->internal_offset) +
            state->output[slot].dst_offset;

         if (idx == so->pos_idx && pcp_ptr && so->stream == 0) {
            memcpy(buffer, &pre_clip_pos[start_comp],
                   num_comps * sizeof(float));
         } else {
            memcpy(buffer, &input[idx][start_comp],
                   num_comps * sizeof(float));
         }

#if 0
         {
            debug_printf("VERT[%d], stream = %d, offset = %d, slot[%d] sc = %d, num_c = %d, idx = %d = [",
                         i, stream,
                         draw->so.targets[ob]->internal_offset + (4 * state->output[slot].dst_offset),
                         slot, start_comp, num_comps, idx);
            for (unsigned j = 0; j < num_comps; ++j) {
               unsigned *ubuffer = (unsigned*)buffer;
               debug_printf("%d (0x%x), ", ubuffer[j], ubuffer[j]);
            }
            debug_printf("]\n");
         }
#endif
      }

      for (unsigned ob = 0; ob < draw->so.num_targets; ++ob) {
         struct draw_so_target *target = draw->so.targets[ob];
         if (target && buffer_written[ob]) {
            target->internal_offset += state->stride[ob] * sizeof(float);
         }
      }
   }
   ++so->emitted_primitives;
}


static void
so_point(struct pt_so_emit *so, int idx)
{
   unsigned indices[1];
   indices[0] = idx;
   so_emit_prim(so, indices, 1);
}


static void
so_line(struct pt_so_emit *so, int i0, int i1)
{
   unsigned indices[2];
   indices[0] = i0;
   indices[1] = i1;
   so_emit_prim(so, indices, 2);
}


static void
so_tri(struct pt_so_emit *so, int i0, int i1, int i2)
{
   unsigned indices[3];
   indices[0] = i0;
   indices[1] = i1;
   indices[2] = i2;
   so_emit_prim(so, indices, 3);
}


#define FUNC         so_run_linear
#define GET_ELT(idx) (start + (idx))
#include "draw_so_emit_tmp.h"


#define FUNC         so_run_elts
#define LOCAL_VARS   const uint16_t *elts = input_prims->elts;
#define GET_ELT(idx) (elts[start + (idx)])
#include "draw_so_emit_tmp.h"


void
draw_pt_so_emit(struct pt_so_emit *emit,
                int num_vertex_streams,
                const struct draw_vertex_info *input_verts,
                const struct draw_prim_info *input_prims)
{
   struct draw_context *draw = emit->draw;
   struct vbuf_render *render = draw->render;

   if (!emit->has_so && num_vertex_streams == 1) {
      if (draw->collect_primgen) {
         unsigned total = 0;
         for (unsigned i = 0; i < input_prims->primitive_count; i++) {
            total +=
               u_decomposed_prims_for_vertices(input_prims->prim,
                                               input_prims->primitive_lengths[i]);
         }
         render->set_stream_output_info(render, 0, 0, total);
      }
      return;
   }

   if (!emit->has_so && !draw->collect_primgen)
      return;

   /* XXX: need to flush to get prim_vbuf.c to release its allocation??*/
   draw_do_flush(draw, DRAW_FLUSH_BACKEND);

   for (unsigned stream = 0; stream < num_vertex_streams; stream++) {
      emit->emitted_primitives = 0;
      emit->generated_primitives = 0;
      if (emit->use_pre_clip_pos)
         emit->pre_clip_pos = input_verts[stream].verts->clip_pos;

      emit->input_vertex_stride = input_verts[stream].stride;
      emit->inputs = (const float (*)[4])input_verts[stream].verts->data;
      emit->stream = stream;

      unsigned start, i;
      for (start = i = 0; i < input_prims[stream].primitive_count;
           start += input_prims[stream].primitive_lengths[i], i++) {
         unsigned count = input_prims[stream].primitive_lengths[i];

         if (input_prims->linear) {
            so_run_linear(emit, &input_prims[stream], &input_verts[stream],
                          start, count);
         } else {
            so_run_elts(emit, &input_prims[stream], &input_verts[stream],
                        start, count);
         }
      }
      render->set_stream_output_info(render,
                                     stream,
                                     emit->has_so ? emit->emitted_primitives : 0,
                                     emit->generated_primitives);
   }
}


struct pt_so_emit *
draw_pt_so_emit_create(struct draw_context *draw)
{
   struct pt_so_emit *emit = CALLOC_STRUCT(pt_so_emit);
   if (!emit)
      return NULL;

   emit->draw = draw;

   return emit;
}


void
draw_pt_so_emit_destroy(struct pt_so_emit *emit)
{
   FREE(emit);
}
