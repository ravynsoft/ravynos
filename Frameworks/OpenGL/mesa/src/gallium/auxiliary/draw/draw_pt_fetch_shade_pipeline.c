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

#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "draw/draw_context.h"
#include "draw/draw_vbuf.h"
#include "draw/draw_vertex.h"
#include "draw/draw_prim_assembler.h"
#include "draw/draw_pt.h"
#include "draw/draw_vs.h"
#include "draw/draw_gs.h"


struct fetch_pipeline_middle_end {
   struct draw_pt_middle_end base;
   struct draw_context *draw;

   struct pt_emit *emit;
   struct pt_so_emit *so_emit;
   struct pt_fetch *fetch;
   struct pt_post_vs *post_vs;

   unsigned vertex_data_offset;
   unsigned vertex_size;
   unsigned input_prim;
   unsigned opt;
};


/** cast wrapper */
static inline struct fetch_pipeline_middle_end *
fetch_pipeline_middle_end(struct draw_pt_middle_end *middle)
{
   return (struct fetch_pipeline_middle_end *) middle;
}


/**
 * Prepare/validate middle part of the vertex pipeline.
 * NOTE: if you change this function, also look at the LLVM
 * function llvm_middle_end_prepare() for similar changes.
 */
static void
fetch_pipeline_prepare(struct draw_pt_middle_end *middle,
                       enum mesa_prim prim,
                       unsigned opt,
                       unsigned *max_vertices)
{
   struct fetch_pipeline_middle_end *fpme = fetch_pipeline_middle_end(middle);
   struct draw_context *draw = fpme->draw;
   struct draw_vertex_shader *vs = draw->vs.vertex_shader;
   struct draw_geometry_shader *gs = draw->gs.geometry_shader;
   unsigned instance_id_index = ~0;
   const unsigned gs_out_prim = (gs ? gs->output_primitive :
                                 u_assembled_prim(prim));
   unsigned nr_vs_outputs = draw_total_vs_outputs(draw);
   unsigned nr = MAX2(vs->info.num_inputs, nr_vs_outputs);
   unsigned point_line_clip = draw->rasterizer->fill_front == PIPE_POLYGON_MODE_POINT ||
                              draw->rasterizer->fill_front == PIPE_POLYGON_MODE_LINE ||
                              gs_out_prim == MESA_PRIM_POINTS ||
                              gs_out_prim == MESA_PRIM_LINE_STRIP;

   if (gs) {
      nr = MAX2(nr, gs->info.num_outputs + 1);
   }

   /* Scan for instanceID system value.
    */
   for (unsigned i = 0; i < vs->info.num_inputs; i++) {
      if (vs->info.input_semantic_name[i] == TGSI_SEMANTIC_INSTANCEID) {
         instance_id_index = i;
         break;
      }
   }

   fpme->input_prim = prim;
   fpme->opt = opt;

   /* Always leave room for the vertex header whether we need it or
    * not.  It's hard to get rid of it in particular because of the
    * viewport code in draw_pt_post_vs.c.
    */
   fpme->vertex_size = sizeof(struct vertex_header) + nr * 4 * sizeof(float);

   draw_pt_fetch_prepare(fpme->fetch,
                         vs->info.num_inputs,
                         fpme->vertex_size,
                         instance_id_index);
   draw_pt_post_vs_prepare(fpme->post_vs,
                           draw->clip_xy,
                           draw->clip_z,
                           draw->clip_user,
                           point_line_clip ? draw->guard_band_points_lines_xy :
                                             draw->guard_band_xy,
                           draw->bypass_viewport,
                           draw->rasterizer->clip_halfz,
                           (draw->vs.edgeflag_output ? true : false));

   draw_pt_so_emit_prepare(fpme->so_emit, false);

   if (!(opt & PT_PIPELINE)) {
      draw_pt_emit_prepare(fpme->emit, gs_out_prim, max_vertices);

      *max_vertices = MAX2(*max_vertices, 4096);
   }
   else {
      /* limit max fetches by limiting max_vertices */
      *max_vertices = 4096;
   }

   /* No need to prepare the shader.
    */
   vs->prepare(vs, draw);

   /* Make sure that the vertex size didn't change at any point above */
   assert(nr_vs_outputs == draw_total_vs_outputs(draw));
}


static void
fetch_pipeline_bind_parameters(struct draw_pt_middle_end *middle)
{
   /* No-op since the vertex shader executor and drawing pipeline
    * just grab the constants, viewport, etc. from the draw context state.
    */
}


static void
fetch(struct pt_fetch *fetch,
      const struct draw_fetch_info *fetch_info,
      char *output)
{
   if (fetch_info->linear) {
      draw_pt_fetch_run_linear(fetch, fetch_info->start,
                               fetch_info->count, output);
   }
   else {
      draw_pt_fetch_run(fetch, fetch_info->elts, fetch_info->count, output);
   }
}


static void
pipeline(struct fetch_pipeline_middle_end *fpme,
         const struct draw_vertex_info *vert_info,
         const struct draw_prim_info *prim_info)
{
   if (prim_info->linear)
      draw_pipeline_run_linear(fpme->draw, vert_info, prim_info);
   else
      draw_pipeline_run(fpme->draw, vert_info, prim_info);
}


static void
emit(struct pt_emit *emit,
     const struct draw_vertex_info *vert_info,
     const struct draw_prim_info *prim_info)
{
   if (prim_info->linear) {
      draw_pt_emit_linear(emit, vert_info, prim_info);
   }
   else {
      draw_pt_emit(emit, vert_info, prim_info);
   }
}


static void
draw_vertex_shader_run(struct draw_vertex_shader *vshader,
                       const struct draw_buffer_info *constants,
                       const struct draw_fetch_info *fetch_info,
                       const struct draw_vertex_info *input_verts,
                       struct draw_vertex_info *output_verts)
{
   output_verts->vertex_size = input_verts->vertex_size;
   output_verts->stride = input_verts->vertex_size;
   output_verts->count = input_verts->count;
   output_verts->verts =
      (struct vertex_header *)MALLOC(output_verts->vertex_size *
                                     align(output_verts->count, 4) +
                                     DRAW_EXTRA_VERTICES_PADDING);

   vshader->run_linear(vshader,
                       (const float (*)[4])input_verts->verts->data,
                       (      float (*)[4])output_verts->verts->data,
                       constants,
                       input_verts->count,
                       input_verts->vertex_size,
                       input_verts->vertex_size,
                       fetch_info->elts);
}


static void
fetch_pipeline_generic(struct draw_pt_middle_end *middle,
                       const struct draw_fetch_info *fetch_info,
                       const struct draw_prim_info *in_prim_info)
{
   struct fetch_pipeline_middle_end *fpme = fetch_pipeline_middle_end(middle);
   struct draw_context *draw = fpme->draw;
   struct draw_vertex_shader *vshader = draw->vs.vertex_shader;
   struct draw_geometry_shader *gshader = draw->gs.geometry_shader;
   struct draw_prim_info gs_prim_info[TGSI_MAX_VERTEX_STREAMS];
   struct draw_vertex_info fetched_vert_info;
   struct draw_vertex_info vs_vert_info;
   struct draw_vertex_info gs_vert_info[TGSI_MAX_VERTEX_STREAMS];
   struct draw_vertex_info *vert_info;
   struct draw_prim_info ia_prim_info;
   struct draw_vertex_info ia_vert_info;
   const struct draw_prim_info *prim_info = in_prim_info;
   bool free_prim_info = false;
   unsigned opt = fpme->opt;
   int num_vertex_streams = 1;

   fetched_vert_info.count = fetch_info->count;
   fetched_vert_info.vertex_size = fpme->vertex_size;
   fetched_vert_info.stride = fpme->vertex_size;
   fetched_vert_info.verts =
      (struct vertex_header *)MALLOC(fpme->vertex_size *
                                     align(fetch_info->count,  4) +
                                     DRAW_EXTRA_VERTICES_PADDING);
   if (!fetched_vert_info.verts) {
      assert(0);
      return;
   }
   if (draw->collect_statistics) {
      draw->statistics.ia_vertices += prim_info->count;
      draw->statistics.ia_primitives +=
         u_decomposed_prims_for_vertices(prim_info->prim, fetch_info->count);
      draw->statistics.vs_invocations += fetch_info->count;
   }

   /* Fetch into our vertex buffer.
    */
   fetch(fpme->fetch, fetch_info, (char *)fetched_vert_info.verts);

   vert_info = &fetched_vert_info;

   /* Run the shader, note that this overwrites the data[] parts of
    * the pipeline verts.
    * Need fetch info to get vertex id correct.
    */
   if (fpme->opt & PT_SHADE) {
      draw_vertex_shader_run(vshader,
                             draw->pt.user.constants[PIPE_SHADER_VERTEX],
                             fetch_info,
                             vert_info,
                             &vs_vert_info);

      FREE(vert_info->verts);
      vert_info = &vs_vert_info;
   }

   /* Finished with fetch:
    */
   fetch_info = NULL;

   if ((fpme->opt & PT_SHADE) && gshader) {
      draw_geometry_shader_run(gshader,
                               draw->pt.user.constants[PIPE_SHADER_GEOMETRY],
                               vert_info,
                               prim_info,
                               &vshader->info,
                               gs_vert_info,
                               gs_prim_info);

      FREE(vert_info->verts);
      vert_info = &gs_vert_info[0];
      prim_info = &gs_prim_info[0];
      num_vertex_streams = gshader->num_vertex_streams;

      /*
       * pt emit can only handle ushort number of vertices (see
       * render->allocate_vertices).
       * vsplit guarantees there's never more than 4096, however GS can
       * easily blow this up (by a factor of 256 (or even 1024) max).
       */
      if (vert_info->count > 65535) {
         opt |= PT_PIPELINE;
      }
   } else {
      if (draw_prim_assembler_is_required(draw, prim_info, vert_info)) {
         draw_prim_assembler_run(draw, prim_info, vert_info,
                                 &ia_prim_info, &ia_vert_info);

         if (ia_vert_info.count) {
            FREE(vert_info->verts);
            vert_info = &ia_vert_info;
            prim_info = &ia_prim_info;
            free_prim_info = true;
         }
      }
   }
   if (prim_info->count == 0) {
      debug_printf("GS/IA didn't emit any vertices!\n");

      FREE(vert_info->verts);
      if (free_prim_info) {
         FREE(prim_info->primitive_lengths);
      }
      return;
   }


   /* Stream output needs to be done before clipping.
    *
    * XXX: Stream output surely needs to respect the prim_info->elt
    *      lists.
    */
   draw_pt_so_emit(fpme->so_emit, num_vertex_streams, vert_info, prim_info);

   draw_stats_clipper_primitives(draw, prim_info);

   /*
    * if there's no position, need to stop now, or the latter stages
    * will try to access non-existent position output.
    */
   if (draw_current_shader_position_output(draw) != -1) {
      if (draw_pt_post_vs_run(fpme->post_vs, vert_info, prim_info)) {
         opt |= PT_PIPELINE;
      }

      /* Do we need to run the pipeline?
       */
      if (opt & PT_PIPELINE) {
         pipeline(fpme, vert_info, prim_info);
      }
      else {
         emit(fpme->emit, vert_info, prim_info);
      }
   }
   FREE(vert_info->verts);
   if (free_prim_info) {
      FREE(prim_info->primitive_lengths);
   }
}


static inline unsigned
prim_type(unsigned prim, unsigned flags)
{
   if (flags & DRAW_LINE_LOOP_AS_STRIP)
      return MESA_PRIM_LINE_STRIP;
   else
      return prim;
}


static void
fetch_pipeline_run(struct draw_pt_middle_end *middle,
                   const unsigned *fetch_elts,
                   unsigned fetch_count,
                   const uint16_t *draw_elts,
                   unsigned draw_count,
                   unsigned prim_flags)
{
   struct fetch_pipeline_middle_end *fpme = fetch_pipeline_middle_end(middle);
   struct draw_fetch_info fetch_info;
   struct draw_prim_info prim_info;

   fetch_info.linear = false;
   fetch_info.start = 0;
   fetch_info.elts = fetch_elts;
   fetch_info.count = fetch_count;

   prim_info.linear = false;
   prim_info.start = 0;
   prim_info.count = draw_count;
   prim_info.elts = draw_elts;
   prim_info.prim = prim_type(fpme->input_prim, prim_flags);
   prim_info.flags = prim_flags;
   prim_info.primitive_count = 1;
   prim_info.primitive_lengths = &draw_count;

   fetch_pipeline_generic(middle, &fetch_info, &prim_info);
}


static void
fetch_pipeline_linear_run(struct draw_pt_middle_end *middle,
                          unsigned start,
                          unsigned count,
                          unsigned prim_flags)
{
   struct fetch_pipeline_middle_end *fpme = fetch_pipeline_middle_end(middle);
   struct draw_fetch_info fetch_info;
   struct draw_prim_info prim_info;

   fetch_info.linear = true;
   fetch_info.start = start;
   fetch_info.count = count;
   fetch_info.elts = NULL;

   prim_info.linear = true;
   prim_info.start = 0;
   prim_info.count = count;
   prim_info.elts = NULL;
   prim_info.prim = prim_type(fpme->input_prim, prim_flags);
   prim_info.flags = prim_flags;
   prim_info.primitive_count = 1;
   prim_info.primitive_lengths = &count;

   fetch_pipeline_generic(middle, &fetch_info, &prim_info);
}


static bool
fetch_pipeline_linear_run_elts(struct draw_pt_middle_end *middle,
                               unsigned start,
                               unsigned count,
                               const uint16_t *draw_elts,
                               unsigned draw_count,
                               unsigned prim_flags)
{
   struct fetch_pipeline_middle_end *fpme = fetch_pipeline_middle_end(middle);
   struct draw_fetch_info fetch_info;
   struct draw_prim_info prim_info;

   fetch_info.linear = true;
   fetch_info.start = start;
   fetch_info.count = count;
   fetch_info.elts = NULL;

   prim_info.linear = false;
   prim_info.start = 0;
   prim_info.count = draw_count;
   prim_info.elts = draw_elts;
   prim_info.prim = prim_type(fpme->input_prim, prim_flags);
   prim_info.flags = prim_flags;
   prim_info.primitive_count = 1;
   prim_info.primitive_lengths = &draw_count;

   fetch_pipeline_generic(middle, &fetch_info, &prim_info);

   return true;
}


static void
fetch_pipeline_finish(struct draw_pt_middle_end *middle)
{
   /* nothing to do */
}


static void
fetch_pipeline_destroy(struct draw_pt_middle_end *middle)
{
   struct fetch_pipeline_middle_end *fpme = fetch_pipeline_middle_end(middle);

   if (fpme->fetch)
      draw_pt_fetch_destroy(fpme->fetch);

   if (fpme->emit)
      draw_pt_emit_destroy(fpme->emit);

   if (fpme->so_emit)
      draw_pt_so_emit_destroy(fpme->so_emit);

   if (fpme->post_vs)
      draw_pt_post_vs_destroy(fpme->post_vs);

   FREE(middle);
}


struct draw_pt_middle_end *
draw_pt_fetch_pipeline_or_emit(struct draw_context *draw)
{
   struct fetch_pipeline_middle_end *fpme =
      CALLOC_STRUCT(fetch_pipeline_middle_end);
   if (!fpme)
      goto fail;

   fpme->base.prepare        = fetch_pipeline_prepare;
   fpme->base.bind_parameters  = fetch_pipeline_bind_parameters;
   fpme->base.run            = fetch_pipeline_run;
   fpme->base.run_linear     = fetch_pipeline_linear_run;
   fpme->base.run_linear_elts = fetch_pipeline_linear_run_elts;
   fpme->base.finish         = fetch_pipeline_finish;
   fpme->base.destroy        = fetch_pipeline_destroy;

   fpme->draw = draw;

   fpme->fetch = draw_pt_fetch_create(draw);
   if (!fpme->fetch)
      goto fail;

   fpme->post_vs = draw_pt_post_vs_create(draw);
   if (!fpme->post_vs)
      goto fail;

   fpme->emit = draw_pt_emit_create(draw);
   if (!fpme->emit)
      goto fail;

   fpme->so_emit = draw_pt_so_emit_create(draw);
   if (!fpme->so_emit)
      goto fail;

   return &fpme->base;

 fail:
   if (fpme)
      fetch_pipeline_destroy(&fpme->base);

   return NULL;
}
