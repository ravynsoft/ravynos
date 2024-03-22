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

#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "draw/draw_context.h"
#include "draw/draw_gs.h"
#include "draw/draw_tess.h"
#include "draw/draw_vbuf.h"
#include "draw/draw_vertex.h"
#include "draw/draw_pt.h"
#include "draw/draw_prim_assembler.h"
#include "draw/draw_vs.h"
#include "draw/draw_llvm.h"
#include "gallivm/lp_bld_init.h"
#include "gallivm/lp_bld_debug.h"


struct llvm_middle_end {
   struct draw_pt_middle_end base;
   struct draw_context *draw;

   struct pt_emit *emit;
   struct pt_so_emit *so_emit;
   struct pt_fetch *fetch;
   struct pt_post_vs *post_vs;


   unsigned vertex_data_offset;
   unsigned vertex_size;
   enum mesa_prim input_prim;
   unsigned opt;

   struct draw_llvm *llvm;
   struct draw_llvm_variant *current_variant;
};


/** cast wrapper */
static inline struct llvm_middle_end *
llvm_middle_end(struct draw_pt_middle_end *middle)
{
   return (struct llvm_middle_end *) middle;
}


static void
llvm_middle_end_prepare_gs(struct llvm_middle_end *fpme)
{
   struct draw_context *draw = fpme->draw;
   struct draw_llvm *llvm = fpme->llvm;
   struct draw_geometry_shader *gs = draw->gs.geometry_shader;
   struct draw_gs_llvm_variant_list_item *li;
   struct llvm_geometry_shader *shader = llvm_geometry_shader(gs);
   char store[DRAW_GS_LLVM_MAX_VARIANT_KEY_SIZE];
   struct draw_gs_llvm_variant_key *key = draw_gs_llvm_make_variant_key(llvm, store);

   /* Search shader's list of variants for the key */
   struct draw_gs_llvm_variant *variant = NULL;
   LIST_FOR_EACH_ENTRY(li, &shader->variants.list, list) {
      if (memcmp(&li->base->key, key, shader->variant_key_size) == 0) {
         variant = li->base;
         break;
      }
   }

   if (variant) {
      /* found the variant, move to head of global list (for LRU) */
      list_move_to(&variant->list_item_global.list, &llvm->gs_variants_list.list);
   } else {
      /* Need to create new variant */

      /* First check if we've created too many variants.  If so, free
       * 3.125% of the LRU to avoid using too much memory.
       */
      if (llvm->nr_gs_variants >= DRAW_MAX_SHADER_VARIANTS) {
         if (gallivm_debug & GALLIVM_DEBUG_PERF) {
            debug_printf("Evicting GS: %u gs variants,\t%u total variants\n",
                      shader->variants_cached, llvm->nr_gs_variants);
         }

         /*
          * XXX: should we flush here ?
          */
         struct draw_gs_llvm_variant_list_item *item;
         for (unsigned i = 0; i < DRAW_MAX_SHADER_VARIANTS / 32; i++) {
            if (list_is_empty(&llvm->gs_variants_list.list)) {
               break;
            }
            item = list_last_entry(&llvm->gs_variants_list.list,
                                   struct draw_gs_llvm_variant_list_item, list);
            assert(item);
            assert(item->base);
            draw_gs_llvm_destroy_variant(item->base);
         }
      }

      variant = draw_gs_llvm_create_variant(llvm, draw_total_gs_outputs(draw), key);

      if (variant) {
         list_add(&variant->list_item_local.list, &shader->variants.list);
         list_add(&variant->list_item_global.list, &llvm->gs_variants_list.list);
         llvm->nr_gs_variants++;
         shader->variants_cached++;
      }
   }

   gs->current_variant = variant;
}


static void
llvm_middle_end_prepare_tcs(struct llvm_middle_end *fpme)
{
   struct draw_context *draw = fpme->draw;
   struct draw_llvm *llvm = fpme->llvm;
   struct draw_tess_ctrl_shader *tcs = draw->tcs.tess_ctrl_shader;
   struct draw_tcs_llvm_variant_list_item *li;
   struct llvm_tess_ctrl_shader *shader = llvm_tess_ctrl_shader(tcs);
   char store[DRAW_TCS_LLVM_MAX_VARIANT_KEY_SIZE];
   const struct draw_tcs_llvm_variant_key *key =
      draw_tcs_llvm_make_variant_key(llvm, store);

   /* Search shader's list of variants for the key */
   struct draw_tcs_llvm_variant *variant = NULL;
   LIST_FOR_EACH_ENTRY(li, &shader->variants.list, list) {
      if (memcmp(&li->base->key, key, shader->variant_key_size) == 0) {
         variant = li->base;
         break;
      }
   }

   if (variant) {
      /* found the variant, move to head of global list (for LRU) */
      list_move_to(&variant->list_item_global.list,
                   &llvm->tcs_variants_list.list);
   } else {
      /* Need to create new variant */

      /* First check if we've created too many variants.  If so, free
       * 3.125% of the LRU to avoid using too much memory.
       */
      if (llvm->nr_tcs_variants >= DRAW_MAX_SHADER_VARIANTS) {
         if (gallivm_debug & GALLIVM_DEBUG_PERF) {
            debug_printf("Evicting TCS: %u tcs variants,\t%u total variants\n",
                      shader->variants_cached, llvm->nr_tcs_variants);
         }

         /*
          * XXX: should we flush here ?
          */
         for (unsigned i = 0; i < DRAW_MAX_SHADER_VARIANTS / 32; i++) {
            struct draw_tcs_llvm_variant_list_item *item;
            if (list_is_empty(&llvm->tcs_variants_list.list)) {
               break;
            }
            item = list_last_entry(&llvm->tcs_variants_list.list,
                                   struct draw_tcs_llvm_variant_list_item, list);
            assert(item);
            assert(item->base);
            draw_tcs_llvm_destroy_variant(item->base);
         }
      }

      variant = draw_tcs_llvm_create_variant(llvm, 0, key);

      if (variant) {
         list_add(&variant->list_item_local.list, &shader->variants.list);
         list_add(&variant->list_item_global.list, &llvm->tcs_variants_list.list);
         llvm->nr_tcs_variants++;
         shader->variants_cached++;
      }
   }

   tcs->current_variant = variant;
}


static void
llvm_middle_end_prepare_tes(struct llvm_middle_end *fpme)
{
   struct draw_context *draw = fpme->draw;
   struct draw_llvm *llvm = fpme->llvm;
   struct draw_tess_eval_shader *tes = draw->tes.tess_eval_shader;
   struct draw_tes_llvm_variant *variant = NULL;
   struct draw_tes_llvm_variant_list_item *li;
   struct llvm_tess_eval_shader *shader = llvm_tess_eval_shader(tes);
   char store[DRAW_TES_LLVM_MAX_VARIANT_KEY_SIZE];
   const struct draw_tes_llvm_variant_key *key =
      draw_tes_llvm_make_variant_key(llvm, store);

   /* Search shader's list of variants for the key */
   LIST_FOR_EACH_ENTRY(li, &shader->variants.list, list) {
      if (memcmp(&li->base->key, key, shader->variant_key_size) == 0) {
         variant = li->base;
         break;
      }
   }

   if (variant) {
      /* found the variant, move to head of global list (for LRU) */
      list_move_to(&variant->list_item_global.list,
                   &llvm->tes_variants_list.list);
   } else {
      /* Need to create new variant */

      /* First check if we've created too many variants.  If so, free
       * 3.125% of the LRU to avoid using too much memory.
       */
      if (llvm->nr_tes_variants >= DRAW_MAX_SHADER_VARIANTS) {
         if (gallivm_debug & GALLIVM_DEBUG_PERF) {
            debug_printf("Evicting TES: %u tes variants,\t%u total variants\n",
                      shader->variants_cached, llvm->nr_tes_variants);
         }

         /*
          * XXX: should we flush here ?
          */
         for (unsigned i = 0; i < DRAW_MAX_SHADER_VARIANTS / 32; i++) {
            struct draw_tes_llvm_variant_list_item *item;
            if (list_is_empty(&llvm->tes_variants_list.list)) {
               break;
            }
            item = list_last_entry(&llvm->tes_variants_list.list,
                                   struct draw_tes_llvm_variant_list_item, list);
            assert(item);
            assert(item->base);
            draw_tes_llvm_destroy_variant(item->base);
         }
      }

      variant = draw_tes_llvm_create_variant(llvm, draw_total_tes_outputs(draw), key);

      if (variant) {
         list_add(&variant->list_item_local.list, &shader->variants.list);
         list_add(&variant->list_item_global.list, &llvm->tes_variants_list.list);
         llvm->nr_tes_variants++;
         shader->variants_cached++;
      }
   }

   tes->current_variant = variant;
}


/**
 * Prepare/validate middle part of the vertex pipeline.
 * NOTE: if you change this function, also look at the non-LLVM
 * function fetch_pipeline_prepare() for similar changes.
 */
static void
llvm_middle_end_prepare(struct draw_pt_middle_end *middle,
                        enum mesa_prim in_prim,
                        unsigned opt,
                        unsigned *max_vertices)
{
   struct llvm_middle_end *fpme = llvm_middle_end(middle);
   struct draw_context *draw = fpme->draw;
   struct draw_llvm *llvm = fpme->llvm;
   struct draw_vertex_shader *vs = draw->vs.vertex_shader;
   struct draw_geometry_shader *gs = draw->gs.geometry_shader;
   struct draw_tess_ctrl_shader *tcs = draw->tcs.tess_ctrl_shader;
   struct draw_tess_eval_shader *tes = draw->tes.tess_eval_shader;
   const enum mesa_prim out_prim =
      gs ? gs->output_primitive : tes ? get_tes_output_prim(tes) :
      u_assembled_prim(in_prim);
   unsigned point_line_clip = draw->rasterizer->fill_front == PIPE_POLYGON_MODE_POINT ||
                              draw->rasterizer->fill_front == PIPE_POLYGON_MODE_LINE ||
                              out_prim == MESA_PRIM_POINTS ||
                              u_reduced_prim(out_prim) == MESA_PRIM_LINES;

   fpme->input_prim = in_prim;
   fpme->opt = opt;

   draw_pt_post_vs_prepare(fpme->post_vs,
                           draw->clip_xy,
                           draw->clip_z,
                           draw->clip_user,
                           point_line_clip ? draw->guard_band_points_lines_xy :
                                             draw->guard_band_xy,
                           draw->bypass_viewport,
                           draw->rasterizer->clip_halfz,
                           (draw->vs.edgeflag_output ? true : false));

   draw_pt_so_emit_prepare(fpme->so_emit, (gs == NULL && tes == NULL));

   if (!(opt & PT_PIPELINE)) {
      draw_pt_emit_prepare(fpme->emit, out_prim, max_vertices);

      *max_vertices = MAX2(*max_vertices, 4096);
   } else {
      /* limit max fetches by limiting max_vertices */
      *max_vertices = 4096;
   }

   /* Get the number of float[4] attributes per vertex.
    * Note: this must be done after draw_pt_emit_prepare() since that
    * can effect the vertex size.
    */
   unsigned nr = MAX2(vs->info.num_inputs, draw_total_vs_outputs(draw));

   /* Always leave room for the vertex header whether we need it or
    * not.  It's hard to get rid of it in particular because of the
    * viewport code in draw_pt_post_vs.c.
    */
   fpme->vertex_size = sizeof(struct vertex_header) + nr * 4 * sizeof(float);

   /* return even number */
   *max_vertices = *max_vertices & ~1;

   /* Find/create the vertex shader variant */
   {
      struct draw_llvm_variant *variant = NULL;
      struct draw_llvm_variant_list_item *li;
      struct llvm_vertex_shader *shader = llvm_vertex_shader(vs);
      char store[DRAW_LLVM_MAX_VARIANT_KEY_SIZE];
      struct draw_llvm_variant_key *key = draw_llvm_make_variant_key(llvm, store);

      /* Search shader's list of variants for the key */
      LIST_FOR_EACH_ENTRY(li, &shader->variants.list, list) {
         if (memcmp(&li->base->key, key, shader->variant_key_size) == 0) {
            variant = li->base;
            break;
         }
      }

      if (variant) {
         /* found the variant, move to head of global list (for LRU) */
         list_move_to(&variant->list_item_global.list, &llvm->vs_variants_list.list);
      } else {
         /* Need to create new variant */

         /* First check if we've created too many variants.  If so, free
          * 3.125% of the LRU to avoid using too much memory.
          */
         if (llvm->nr_variants >= DRAW_MAX_SHADER_VARIANTS) {
            if (gallivm_debug & GALLIVM_DEBUG_PERF) {
               debug_printf("Evicting VS: %u vs variants,\t%u total variants\n",
                         shader->variants_cached, llvm->nr_variants);
            }

            /*
             * XXX: should we flush here ?
             */
            for (unsigned i = 0; i < DRAW_MAX_SHADER_VARIANTS / 32; i++) {
               struct draw_llvm_variant_list_item *item;
               if (list_is_empty(&llvm->vs_variants_list.list)) {
                  break;
               }
               item = list_last_entry(&llvm->vs_variants_list.list,
                                    struct draw_llvm_variant_list_item, list);
               assert(item);
               assert(item->base);
               draw_llvm_destroy_variant(item->base);
            }
         }

         variant = draw_llvm_create_variant(llvm, nr, key);

         if (variant) {
            list_add(&variant->list_item_local.list, &shader->variants.list);
            list_add(&variant->list_item_global.list, &llvm->vs_variants_list.list);
            llvm->nr_variants++;
            shader->variants_cached++;
         }
      }

      fpme->current_variant = variant;
   }

   if (gs) {
      llvm_middle_end_prepare_gs(fpme);
   }
   if (tcs) {
      llvm_middle_end_prepare_tcs(fpme);
   }
   if (tes) {
      llvm_middle_end_prepare_tes(fpme);
   }
}


static unsigned
get_num_consts_robust(struct draw_context *draw, struct draw_buffer_info *bufs, unsigned idx)
{
   uint64_t const_bytes = bufs[idx].size;

   if (const_bytes < sizeof(float))
      return 0;

   return DIV_ROUND_UP(const_bytes, draw->constant_buffer_stride);
}

/**
 * Bind/update constant buffer pointers, clip planes and viewport dims.
 * These are "light weight" parameters which aren't baked into the
 * generated code.  Updating these items is much cheaper than revalidating
 * and rebuilding the generated pipeline code.
 */
static void
llvm_middle_end_bind_parameters(struct draw_pt_middle_end *middle)
{
   static const float fake_const_buf[4];
   struct llvm_middle_end *fpme = llvm_middle_end(middle);
   struct draw_context *draw = fpme->draw;
   struct draw_llvm *llvm = fpme->llvm;
   unsigned i;

   for (enum pipe_shader_type shader_type = PIPE_SHADER_VERTEX; shader_type <= PIPE_SHADER_GEOMETRY; shader_type++) {
      for (i = 0; i < ARRAY_SIZE(llvm->jit_resources[shader_type].constants); ++i) {
         /*
          * There could be a potential issue with rounding this up, as the
          * shader expects 16-byte allocations, the fix is likely to move
          * to LOAD intrinsic in the future and remove the vec4 constraint.
          */
         int num_consts = get_num_consts_robust(draw, draw->pt.user.constants[shader_type], i);
         llvm->jit_resources[shader_type].constants[i].f = draw->pt.user.constants[shader_type][i].ptr;
         llvm->jit_resources[shader_type].constants[i].num_elements = num_consts;
         if (num_consts == 0) {
            llvm->jit_resources[shader_type].constants[i].f = fake_const_buf;
         }
      }
      for (i = 0; i < ARRAY_SIZE(llvm->jit_resources[shader_type].ssbos); ++i) {
         int num_ssbos = draw->pt.user.ssbos[shader_type][i].size;
         llvm->jit_resources[shader_type].ssbos[i].u = draw->pt.user.ssbos[shader_type][i].ptr;
         llvm->jit_resources[shader_type].ssbos[i].num_elements = num_ssbos;
         if (num_ssbos == 0) {
            llvm->jit_resources[shader_type].ssbos[i].u = (const uint32_t *)fake_const_buf;
         }
      }

      llvm->jit_resources[shader_type].aniso_filter_table = lp_build_sample_aniso_filter_table();
   }

   llvm->vs_jit_context.planes =
      (float (*)[DRAW_TOTAL_CLIP_PLANES][4]) draw->pt.user.planes[0];
   llvm->gs_jit_context.planes =
      (float (*)[DRAW_TOTAL_CLIP_PLANES][4]) draw->pt.user.planes[0];

   llvm->vs_jit_context.viewports = draw->viewports;
   llvm->gs_jit_context.viewports = draw->viewports;
}


static void
pipeline(struct llvm_middle_end *llvm,
         const struct draw_vertex_info *vert_info,
         const struct draw_prim_info *prim_info)
{
   if (prim_info->linear)
      draw_pipeline_run_linear(llvm->draw, vert_info, prim_info);
   else
      draw_pipeline_run(llvm->draw, vert_info, prim_info);
}


static void
emit(struct pt_emit *emit,
     const struct draw_vertex_info *vert_info,
     const struct draw_prim_info *prim_info)
{
   if (prim_info->linear)
      draw_pt_emit_linear(emit, vert_info, prim_info);
   else
      draw_pt_emit(emit, vert_info, prim_info);
}


static void
llvm_pipeline_generic(struct draw_pt_middle_end *middle,
                      const struct draw_fetch_info *fetch_info,
                      const struct draw_prim_info *in_prim_info)
{
   struct llvm_middle_end *fpme = llvm_middle_end(middle);
   struct draw_context *draw = fpme->draw;
   struct draw_geometry_shader *gshader = draw->gs.geometry_shader;
   struct draw_tess_ctrl_shader *tcs_shader = draw->tcs.tess_ctrl_shader;
   struct draw_tess_eval_shader *tes_shader = draw->tes.tess_eval_shader;
   struct draw_prim_info tcs_prim_info;
   struct draw_prim_info tes_prim_info;
   struct draw_prim_info gs_prim_info[TGSI_MAX_VERTEX_STREAMS];
   struct draw_vertex_info llvm_vert_info;
   struct draw_vertex_info tcs_vert_info;
   struct draw_vertex_info tes_vert_info;
   struct draw_vertex_info *vert_info;
   struct draw_prim_info ia_prim_info;
   struct draw_vertex_info ia_vert_info;
   const struct draw_prim_info *prim_info = in_prim_info;
   bool free_prim_info = false;
   unsigned opt = fpme->opt;
   bool clipped = 0;
   uint16_t *tes_elts_out = NULL;

   assert(fetch_info->count > 0);

   llvm_vert_info.count = fetch_info->count;
   llvm_vert_info.vertex_size = fpme->vertex_size;
   llvm_vert_info.stride = fpme->vertex_size;
   llvm_vert_info.verts = (struct vertex_header *)
      MALLOC(fpme->vertex_size *
             align(fetch_info->count, lp_native_vector_width / 32) +
             DRAW_EXTRA_VERTICES_PADDING);
   if (!llvm_vert_info.verts) {
      assert(0);
      return;
   }

   if (draw->collect_statistics) {
      draw->statistics.ia_vertices += prim_info->count;
      if (prim_info->prim == MESA_PRIM_PATCHES)
         draw->statistics.ia_primitives +=
            prim_info->count / draw->pt.vertices_per_patch;
      else
         draw->statistics.ia_primitives +=
            u_decomposed_prims_for_vertices(prim_info->prim, prim_info->count);
      draw->statistics.vs_invocations += fetch_info->count;
   }

   {
      unsigned start, vertex_id_offset;
      const unsigned *elts;

      if (fetch_info->linear) {
         start = fetch_info->start;
         vertex_id_offset = draw->start_index;
         elts = NULL;
      } else {
         start = draw->pt.user.eltMax;
         vertex_id_offset = draw->pt.user.eltBias;
         elts = fetch_info->elts;
      }
      /* Run vertex fetch shader */
      clipped = fpme->current_variant->jit_func(&fpme->llvm->vs_jit_context,
                                                &fpme->llvm->jit_resources[PIPE_SHADER_VERTEX],
                                                llvm_vert_info.verts,
                                                draw->pt.user.vbuffer,
                                                fetch_info->count,
                                                start,
                                                fpme->vertex_size,
                                                draw->pt.vertex_buffer,
                                                draw->instance_id,
                                                vertex_id_offset,
                                                draw->start_instance,
                                                elts,
                                                draw->pt.user.drawid,
                                                draw->pt.user.viewid);

      /* Finished with fetch and vs */
      fetch_info = NULL;
      vert_info = &llvm_vert_info;
   }

   if (opt & PT_SHADE) {
      struct draw_vertex_shader *vshader = draw->vs.vertex_shader;
      if (tcs_shader) {
         draw_tess_ctrl_shader_run(tcs_shader,
                                   vert_info,
                                   prim_info,
                                   &vshader->info,
                                   &tcs_vert_info,
                                   &tcs_prim_info);
         FREE(vert_info->verts);
         vert_info = &tcs_vert_info;
         prim_info = &tcs_prim_info;

      } else if (tes_shader) {
         unsigned num_prims = prim_info->count / draw->pt.vertices_per_patch;
         tcs_prim_info = *prim_info;
         tcs_prim_info.primitive_count = num_prims;
         prim_info = &tcs_prim_info;
      }

      if (tes_shader) {
         draw_tess_eval_shader_run(tes_shader,
                                   tcs_shader ? tcs_shader->vertices_out : draw->pt.vertices_per_patch,
                                   vert_info,
                                   prim_info,
                                   tcs_shader ? &tcs_shader->info : &vshader->info,
                                   &tes_vert_info,
                                   &tes_prim_info, &tes_elts_out);

         FREE(vert_info->verts);
         vert_info = &tes_vert_info;
         prim_info = &tes_prim_info;
         free_prim_info = true;

         /*
          * pt emit can only handle ushort number of vertices (see
          * render->allocate_vertices).
          * vsplit guarantees there's never more than 4096, however GS can
          * easily blow this up (by a factor of 256 (or even 1024) max).
          */
         if (vert_info->count > 65535) {
            opt |= PT_PIPELINE;
         }
      }
   }

   struct draw_vertex_info gs_vert_info[TGSI_MAX_VERTEX_STREAMS];
   memset(&gs_vert_info, 0, sizeof(gs_vert_info));

   if ((opt & PT_SHADE) && gshader) {
      struct draw_vertex_shader *vshader = draw->vs.vertex_shader;
      draw_geometry_shader_run(gshader,
                               draw->pt.user.constants[PIPE_SHADER_GEOMETRY],
                               vert_info,
                               prim_info,
                               tes_shader ? &tes_shader->info : &vshader->info,
                               gs_vert_info,
                               gs_prim_info);

      FREE(vert_info->verts);
      if (free_prim_info) {
         FREE(prim_info->primitive_lengths);
         FREE(tes_elts_out);
      }
      vert_info = &gs_vert_info[0];
      prim_info = &gs_prim_info[0];
      free_prim_info = false;
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
      if (!tes_shader &&
          draw_prim_assembler_is_required(draw, prim_info, vert_info)) {
         draw_prim_assembler_run(draw, prim_info, vert_info,
                                 &ia_prim_info, &ia_vert_info);

         if (ia_vert_info.count) {
            FREE(vert_info->verts);
            if (free_prim_info) {
               FREE(prim_info->primitive_lengths);
               FREE(tes_elts_out);
               tes_elts_out = NULL;
            }
            vert_info = &ia_vert_info;
            prim_info = &ia_prim_info;
            free_prim_info = true;
         }
      }
   }

   /* stream output needs to be done before clipping */
   draw_pt_so_emit(fpme->so_emit,
                   gshader ? gshader->num_vertex_streams : 1,
                   vert_info, prim_info);

   if (prim_info->count == 0) {
      debug_printf("GS/IA didn't emit any vertices!\n");
   } else {
      draw_stats_clipper_primitives(draw, prim_info);

      /*
       * if there's no position, need to stop now, or the latter stages
       * will try to access non-existent position output.
       */
      if (draw_current_shader_position_output(draw) != -1) {
         if ((opt & PT_SHADE) &&
             (gshader || tes_shader ||
              draw->vs.vertex_shader->info.writes_viewport_index)) {
            clipped = draw_pt_post_vs_run(fpme->post_vs, vert_info, prim_info);
         }
         /* "clipped" also includes non-one edgeflag */
         if (clipped) {
            opt |= PT_PIPELINE;
         }

         /* Do we need to run the pipeline? Now will come here if clipped */
         if (opt & PT_PIPELINE) {
            pipeline(fpme, vert_info, prim_info);
         } else {
            emit(fpme->emit, vert_info, prim_info);
         }
      }
   }

   FREE(vert_info->verts);
   if (gshader && gshader->num_vertex_streams > 1)
     for (unsigned i = 1; i < gshader->num_vertex_streams; i++)
       FREE(gs_vert_info[i].verts);

   if (free_prim_info) {
      FREE(tes_elts_out);
      FREE(prim_info->primitive_lengths);
   }
}


static inline enum mesa_prim
prim_type(enum mesa_prim prim, unsigned flags)
{
   if (flags & DRAW_LINE_LOOP_AS_STRIP)
      return MESA_PRIM_LINE_STRIP;
   else
      return prim;
}


static void
llvm_middle_end_run(struct draw_pt_middle_end *middle,
                    const unsigned *fetch_elts,
                    unsigned fetch_count,
                    const uint16_t *draw_elts,
                    unsigned draw_count,
                    unsigned prim_flags)
{
   struct llvm_middle_end *fpme = llvm_middle_end(middle);
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

   llvm_pipeline_generic(middle, &fetch_info, &prim_info);
}


static void
llvm_middle_end_linear_run(struct draw_pt_middle_end *middle,
                           unsigned start,
                           unsigned count,
                           unsigned prim_flags)
{
   struct llvm_middle_end *fpme = llvm_middle_end(middle);
   struct draw_fetch_info fetch_info;
   struct draw_prim_info prim_info;

   fetch_info.linear = true;
   fetch_info.start = start;
   fetch_info.count = count;
   fetch_info.elts = NULL;

   prim_info.linear = true;
   prim_info.start = start;
   prim_info.count = count;
   prim_info.elts = NULL;
   prim_info.prim = prim_type(fpme->input_prim, prim_flags);
   prim_info.flags = prim_flags;
   prim_info.primitive_count = 1;
   prim_info.primitive_lengths = &count;

   llvm_pipeline_generic(middle, &fetch_info, &prim_info);
}


static bool
llvm_middle_end_linear_run_elts(struct draw_pt_middle_end *middle,
                                unsigned start,
                                unsigned count,
                                const uint16_t *draw_elts,
                                unsigned draw_count,
                                unsigned prim_flags)
{
   struct llvm_middle_end *fpme = llvm_middle_end(middle);
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

   llvm_pipeline_generic(middle, &fetch_info, &prim_info);

   return true;
}


static void
llvm_middle_end_finish(struct draw_pt_middle_end *middle)
{
   /* nothing to do */
}


static void
llvm_middle_end_destroy(struct draw_pt_middle_end *middle)
{
   struct llvm_middle_end *fpme = llvm_middle_end(middle);

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
draw_pt_fetch_pipeline_or_emit_llvm(struct draw_context *draw)
{
   struct llvm_middle_end *fpme = 0;

   if (!draw->llvm)
      return NULL;

   fpme = CALLOC_STRUCT(llvm_middle_end);
   if (!fpme)
      goto fail;

   fpme->base.prepare         = llvm_middle_end_prepare;
   fpme->base.bind_parameters = llvm_middle_end_bind_parameters;
   fpme->base.run             = llvm_middle_end_run;
   fpme->base.run_linear      = llvm_middle_end_linear_run;
   fpme->base.run_linear_elts = llvm_middle_end_linear_run_elts;
   fpme->base.finish          = llvm_middle_end_finish;
   fpme->base.destroy         = llvm_middle_end_destroy;

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

   fpme->llvm = draw->llvm;
   if (!fpme->llvm)
      goto fail;

   fpme->current_variant = NULL;

   return &fpme->base;

 fail:
   if (fpme)
      llvm_middle_end_destroy(&fpme->base);

   return NULL;
}
