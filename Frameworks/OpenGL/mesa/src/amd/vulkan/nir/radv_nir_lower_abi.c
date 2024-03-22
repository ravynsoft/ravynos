/*
 * Copyright © 2022 Valve Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "ac_nir.h"
#include "nir.h"
#include "nir_builder.h"
#include "radv_constants.h"
#include "radv_nir.h"
#include "radv_private.h"
#include "radv_shader.h"
#include "radv_shader_args.h"

#define GET_SGPR_FIELD_NIR(arg, field)                                                                                 \
   ac_nir_unpack_arg(b, &s->args->ac, arg, field##__SHIFT, util_bitcount(field##__MASK))

typedef struct {
   enum amd_gfx_level gfx_level;
   const struct radv_shader_args *args;
   const struct radv_shader_info *info;
   const struct radv_pipeline_key *pl_key;
   uint32_t address32_hi;
   nir_def *gsvs_ring[4];
} lower_abi_state;

static nir_def *
load_ring(nir_builder *b, unsigned ring, lower_abi_state *s)
{
   struct ac_arg arg =
      b->shader->info.stage == MESA_SHADER_TASK ? s->args->task_ring_offsets : s->args->ac.ring_offsets;

   nir_def *ring_offsets = ac_nir_load_arg(b, &s->args->ac, arg);
   ring_offsets = nir_pack_64_2x32_split(b, nir_channel(b, ring_offsets, 0), nir_channel(b, ring_offsets, 1));
   return nir_load_smem_amd(b, 4, ring_offsets, nir_imm_int(b, ring * 16u), .align_mul = 4u);
}

static nir_def *
nggc_bool_setting(nir_builder *b, unsigned mask, lower_abi_state *s)
{
   nir_def *settings = ac_nir_load_arg(b, &s->args->ac, s->args->ngg_culling_settings);
   return nir_test_mask(b, settings, mask);
}

static nir_def *
shader_query_bool_setting(nir_builder *b, unsigned mask, lower_abi_state *s)
{
   nir_def *settings = ac_nir_load_arg(b, &s->args->ac, s->args->shader_query_state);
   return nir_test_mask(b, settings, mask);
}

static bool
lower_abi_instr(nir_builder *b, nir_intrinsic_instr *intrin, void *state)
{
   lower_abi_state *s = (lower_abi_state *)state;
   gl_shader_stage stage = b->shader->info.stage;

   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *replacement = NULL;
   bool progress = true;

   switch (intrin->intrinsic) {
   case nir_intrinsic_load_ring_tess_factors_amd:
      replacement = load_ring(b, RING_HS_TESS_FACTOR, s);
      break;
   case nir_intrinsic_load_ring_tess_factors_offset_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.tcs_factor_offset);
      break;
   case nir_intrinsic_load_ring_tess_offchip_amd:
      replacement = load_ring(b, RING_HS_TESS_OFFCHIP, s);
      break;
   case nir_intrinsic_load_ring_tess_offchip_offset_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.tess_offchip_offset);
      break;
   case nir_intrinsic_load_tcs_num_patches_amd:
      if (s->info->num_tess_patches) {
         replacement = nir_imm_int(b, s->info->num_tess_patches);
      } else {
         if (stage == MESA_SHADER_TESS_CTRL) {
            replacement = GET_SGPR_FIELD_NIR(s->args->tcs_offchip_layout, TCS_OFFCHIP_LAYOUT_NUM_PATCHES);
         } else {
            replacement = GET_SGPR_FIELD_NIR(s->args->tes_state, TES_STATE_NUM_PATCHES);
         }
      }
      break;
   case nir_intrinsic_load_ring_esgs_amd:
      replacement = load_ring(b, stage == MESA_SHADER_GEOMETRY ? RING_ESGS_GS : RING_ESGS_VS, s);
      break;
   case nir_intrinsic_load_ring_gsvs_amd:
      if (stage == MESA_SHADER_VERTEX)
         replacement = load_ring(b, RING_GSVS_VS, s);
      else
         replacement = s->gsvs_ring[nir_intrinsic_stream_id(intrin)];
      break;
   case nir_intrinsic_load_ring_gs2vs_offset_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.gs2vs_offset);
      break;
   case nir_intrinsic_load_ring_es2gs_offset_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.es2gs_offset);
      break;

   case nir_intrinsic_load_ring_attr_amd:
      replacement = load_ring(b, RING_PS_ATTR, s);

      /* Note, the HW always assumes there is at least 1 per-vertex param. */
      const unsigned total_num_params = MAX2(1, s->info->outinfo.param_exports) + s->info->outinfo.prim_param_exports;

      nir_def *dword1 = nir_channel(b, replacement, 1);
      dword1 = nir_ior_imm(b, dword1, S_008F04_STRIDE(16 * total_num_params));
      replacement = nir_vector_insert_imm(b, replacement, dword1, 1);
      break;

   case nir_intrinsic_load_ring_attr_offset_amd: {
      nir_def *ring_attr_offset = ac_nir_load_arg(b, &s->args->ac, s->args->ac.gs_attr_offset);
      replacement = nir_ishl_imm(b, nir_ubfe_imm(b, ring_attr_offset, 0, 15), 9); /* 512b increments. */
      break;
   }

   case nir_intrinsic_load_tess_rel_patch_id_amd:
      if (stage == MESA_SHADER_TESS_CTRL) {
         replacement = nir_extract_u8(b, ac_nir_load_arg(b, &s->args->ac, s->args->ac.tcs_rel_ids), nir_imm_int(b, 0));
      } else if (stage == MESA_SHADER_TESS_EVAL) {
         /* Setting an upper bound like this will actually make it possible
          * to optimize some multiplications (in address calculations) so that
          * constant additions can be added to the const offset in memory load instructions.
          */
         nir_def *arg = ac_nir_load_arg(b, &s->args->ac, s->args->ac.tes_rel_patch_id);

         if (s->info->tes.tcs_vertices_out) {
            nir_intrinsic_instr *load_arg = nir_instr_as_intrinsic(arg->parent_instr);
            nir_intrinsic_set_arg_upper_bound_u32_amd(load_arg, 2048 / MAX2(s->info->tes.tcs_vertices_out, 1));
         }

         replacement = arg;
      } else {
         unreachable("invalid tessellation shader stage");
      }
      break;
   case nir_intrinsic_load_patch_vertices_in:
      if (stage == MESA_SHADER_TESS_CTRL) {
         if (s->pl_key->tcs.tess_input_vertices) {
            replacement = nir_imm_int(b, s->pl_key->tcs.tess_input_vertices);
         } else {
            replacement = GET_SGPR_FIELD_NIR(s->args->tcs_offchip_layout, TCS_OFFCHIP_LAYOUT_PATCH_CONTROL_POINTS);
         }
      } else if (stage == MESA_SHADER_TESS_EVAL) {
         if (s->info->tes.tcs_vertices_out) {
            replacement = nir_imm_int(b, s->info->tes.tcs_vertices_out);
         } else {
            replacement = GET_SGPR_FIELD_NIR(s->args->tes_state, TES_STATE_TCS_VERTICES_OUT);
         }
      } else
         unreachable("invalid tessellation shader stage");
      break;
   case nir_intrinsic_load_gs_vertex_offset_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.gs_vtx_offset[nir_intrinsic_base(intrin)]);
      break;
   case nir_intrinsic_load_workgroup_num_input_vertices_amd:
      replacement = nir_ubfe_imm(b, ac_nir_load_arg(b, &s->args->ac, s->args->ac.gs_tg_info), 12, 9);
      break;
   case nir_intrinsic_load_workgroup_num_input_primitives_amd:
      replacement = nir_ubfe_imm(b, ac_nir_load_arg(b, &s->args->ac, s->args->ac.gs_tg_info), 22, 9);
      break;
   case nir_intrinsic_load_packed_passthrough_primitive_amd:
      /* NGG passthrough mode: the HW already packs the primitive export value to a single register.
       */
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.gs_vtx_offset[0]);
      break;
   case nir_intrinsic_load_pipeline_stat_query_enabled_amd:
      replacement = shader_query_bool_setting(b, radv_shader_query_pipeline_stat, s);
      break;
   case nir_intrinsic_load_prim_gen_query_enabled_amd:
      replacement = shader_query_bool_setting(b, radv_shader_query_prim_gen, s);
      break;
   case nir_intrinsic_load_prim_xfb_query_enabled_amd:
      replacement = shader_query_bool_setting(b, radv_shader_query_prim_xfb, s);
      break;
   case nir_intrinsic_load_merged_wave_info_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.merged_wave_info);
      break;
   case nir_intrinsic_load_cull_any_enabled_amd: {
      nir_def *gs_tg_info = ac_nir_load_arg(b, &s->args->ac, s->args->ac.gs_tg_info);

      /* Consider a workgroup small if it contains less than 16 triangles.
       *
       * The gs_tg_info[30:22] is the number of primitives, which we know is non-zero,
       * so the below is equivalent to: "ult(ubfe(gs_tg_info, 22, 9), 16)", but
       * ACO can optimize out the comparison to zero (see try_optimize_scc_nocompare).
       */
      nir_def *small_workgroup = nir_ieq_imm(b, nir_iand_imm(b, gs_tg_info, BITFIELD_RANGE(22 + 4, 9 - 4)), 0);

      nir_def *mask =
         nir_bcsel(b, small_workgroup, nir_imm_int(b, radv_nggc_none),
                   nir_imm_int(b, radv_nggc_front_face | radv_nggc_back_face | radv_nggc_small_primitives));
      nir_def *settings = ac_nir_load_arg(b, &s->args->ac, s->args->ngg_culling_settings);
      replacement = nir_ine_imm(b, nir_iand(b, settings, mask), 0);
      break;
   }
   case nir_intrinsic_load_cull_front_face_enabled_amd:
      replacement = nggc_bool_setting(b, radv_nggc_front_face, s);
      break;
   case nir_intrinsic_load_cull_back_face_enabled_amd:
      replacement = nggc_bool_setting(b, radv_nggc_back_face, s);
      break;
   case nir_intrinsic_load_cull_ccw_amd:
      replacement = nggc_bool_setting(b, radv_nggc_face_is_ccw, s);
      break;
   case nir_intrinsic_load_cull_small_primitives_enabled_amd:
      replacement = nggc_bool_setting(b, radv_nggc_small_primitives, s);
      break;
   case nir_intrinsic_load_cull_small_prim_precision_amd: {
      /* To save space, only the exponent is stored in the high 8 bits.
       * We calculate the precision from those 8 bits:
       * exponent = nggc_settings >> 24
       * precision = 1.0 * 2 ^ exponent
       */
      nir_def *settings = ac_nir_load_arg(b, &s->args->ac, s->args->ngg_culling_settings);
      nir_def *exponent = nir_ishr_imm(b, settings, 24u);
      replacement = nir_ldexp(b, nir_imm_float(b, 1.0f), exponent);
      break;
   }

   case nir_intrinsic_load_viewport_xy_scale_and_offset: {
      nir_def *comps[] = {
         ac_nir_load_arg(b, &s->args->ac, s->args->ngg_viewport_scale[0]),
         ac_nir_load_arg(b, &s->args->ac, s->args->ngg_viewport_scale[1]),
         ac_nir_load_arg(b, &s->args->ac, s->args->ngg_viewport_translate[0]),
         ac_nir_load_arg(b, &s->args->ac, s->args->ngg_viewport_translate[1]),
      };
      replacement = nir_vec(b, comps, 4);
      break;
   }

   case nir_intrinsic_load_ring_task_draw_amd:
      replacement = load_ring(b, RING_TS_DRAW, s);
      break;
   case nir_intrinsic_load_ring_task_payload_amd:
      replacement = load_ring(b, RING_TS_PAYLOAD, s);
      break;
   case nir_intrinsic_load_ring_mesh_scratch_amd:
      replacement = load_ring(b, RING_MS_SCRATCH, s);
      break;
   case nir_intrinsic_load_ring_mesh_scratch_offset_amd:
      /* gs_tg_info[0:11] is ordered_wave_id. Multiply by the ring entry size. */
      replacement = nir_imul_imm(b, nir_iand_imm(b, ac_nir_load_arg(b, &s->args->ac, s->args->ac.gs_tg_info), 0xfff),
                                 RADV_MESH_SCRATCH_ENTRY_BYTES);
      break;
   case nir_intrinsic_load_task_ring_entry_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.task_ring_entry);
      break;
   case nir_intrinsic_load_lshs_vertex_stride_amd: {
      if (stage == MESA_SHADER_VERTEX) {
         replacement = nir_imm_int(b, get_tcs_input_vertex_stride(s->info->vs.num_linked_outputs));
      } else {
         assert(stage == MESA_SHADER_TESS_CTRL);
         if (s->info->inputs_linked) {
            replacement = nir_imm_int(b, get_tcs_input_vertex_stride(s->info->tcs.num_linked_inputs));
         } else {
            nir_def *lshs_vertex_stride =
               GET_SGPR_FIELD_NIR(s->args->tcs_offchip_layout, TCS_OFFCHIP_LAYOUT_LSHS_VERTEX_STRIDE);
            replacement = nir_ishl_imm(b, lshs_vertex_stride, 2);
         }
      }
      break;
   }
   case nir_intrinsic_load_esgs_vertex_stride_amd: {
      /* Emulate VGT_ESGS_RING_ITEMSIZE on GFX9+ to reduce context register writes. */
      assert(s->gfx_level >= GFX9);
      const unsigned stride =
         s->info->is_ngg ? s->info->ngg_info.vgt_esgs_ring_itemsize : s->info->gs_ring_info.vgt_esgs_ring_itemsize;
      replacement = nir_imm_int(b, stride);
      break;
   }
   case nir_intrinsic_load_hs_out_patch_data_offset_amd: {
      nir_def *num_tcs_outputs, *out_vertices_per_patch;

      if (stage == MESA_SHADER_TESS_CTRL) {
         num_tcs_outputs = nir_imm_int(b, s->info->tcs.num_linked_outputs);
         out_vertices_per_patch = nir_imm_int(b, s->info->tcs.tcs_vertices_out);
      } else {
         if (s->info->inputs_linked) {
            num_tcs_outputs = nir_imm_int(b, s->info->tes.num_linked_inputs);
         } else {
            num_tcs_outputs = GET_SGPR_FIELD_NIR(s->args->tes_state, TES_STATE_NUM_TCS_OUTPUTS);
         }

         if (s->info->tes.tcs_vertices_out) {
            out_vertices_per_patch = nir_imm_int(b, s->info->tes.tcs_vertices_out);
         } else {
            out_vertices_per_patch = GET_SGPR_FIELD_NIR(s->args->tes_state, TES_STATE_TCS_VERTICES_OUT);
         }
      }

      nir_def *per_vertex_output_patch_size =
         nir_imul(b, out_vertices_per_patch, nir_imul_imm(b, num_tcs_outputs, 16u));

      if (s->info->num_tess_patches) {
         unsigned num_patches = s->info->num_tess_patches;
         replacement = nir_imul_imm(b, per_vertex_output_patch_size, num_patches);
      } else {
         nir_def *num_patches;

         if (stage == MESA_SHADER_TESS_CTRL) {
            num_patches = GET_SGPR_FIELD_NIR(s->args->tcs_offchip_layout, TCS_OFFCHIP_LAYOUT_NUM_PATCHES);
         } else {
            num_patches = GET_SGPR_FIELD_NIR(s->args->tes_state, TES_STATE_NUM_PATCHES);
         }
         replacement = nir_imul(b, per_vertex_output_patch_size, num_patches);
      }
      break;
   }
   case nir_intrinsic_load_sample_positions_amd: {
      uint32_t sample_pos_offset = (RING_PS_SAMPLE_POSITIONS * 16) - 8;

      nir_def *ring_offsets = ac_nir_load_arg(b, &s->args->ac, s->args->ac.ring_offsets);
      nir_def *addr = nir_pack_64_2x32(b, ring_offsets);
      nir_def *sample_id = nir_umin(b, intrin->src[0].ssa, nir_imm_int(b, 7));
      nir_def *offset = nir_ishl_imm(b, sample_id, 3); /* 2 floats containing samplepos.xy */

      nir_const_value *const_num_samples = nir_src_as_const_value(intrin->src[1]);
      if (const_num_samples) {
         sample_pos_offset += (const_num_samples->u32 << 3);
      } else {
         offset = nir_iadd(b, offset, nir_ishl_imm(b, intrin->src[1].ssa, 3));
      }

      replacement =
         nir_load_global_amd(b, 2, 32, addr, offset, .base = sample_pos_offset, .access = ACCESS_NON_WRITEABLE);
      break;
   }
   case nir_intrinsic_load_rasterization_samples_amd:
      if (s->pl_key->dynamic_rasterization_samples) {
         replacement = GET_SGPR_FIELD_NIR(s->args->ps_state, PS_STATE_NUM_SAMPLES);
      } else {
         replacement = nir_imm_int(b, s->pl_key->ps.num_samples);
      }
      break;
   case nir_intrinsic_load_provoking_vtx_in_prim_amd: {
      if (s->pl_key->dynamic_provoking_vtx_mode) {
         replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ngg_provoking_vtx);
      } else {
         unsigned provoking_vertex = 0;
         if (s->pl_key->vs.provoking_vtx_last) {
            if (stage == MESA_SHADER_VERTEX) {
               provoking_vertex = radv_get_num_vertices_per_prim(s->pl_key) - 1;
            } else if (stage == MESA_SHADER_GEOMETRY) {
               provoking_vertex = b->shader->info.gs.vertices_in - 1;
            } else {
               /* TES won't use this intrinsic, because it can get primitive id directly
                * instead of using this intrinsic to pass primitive id by LDS.
                */
               unreachable("load_provoking_vtx_in_prim_amd is only supported in VS and GS");
            }
         }

         replacement = nir_imm_int(b, provoking_vertex);
      }
      break;
   }
   case nir_intrinsic_atomic_add_gs_emit_prim_count_amd:
      nir_gds_atomic_add_amd(b, 32, intrin->src[0].ssa, nir_imm_int(b, RADV_SHADER_QUERY_GS_PRIM_EMIT_OFFSET),
                             nir_imm_int(b, 0x100));
      break;
   case nir_intrinsic_atomic_add_gen_prim_count_amd: {
      uint32_t offset = stage == MESA_SHADER_MESH ? RADV_SHADER_QUERY_MS_PRIM_GEN_OFFSET
                                                  : RADV_SHADER_QUERY_PRIM_GEN_OFFSET(nir_intrinsic_stream_id(intrin));

      nir_gds_atomic_add_amd(b, 32, intrin->src[0].ssa, nir_imm_int(b, offset), nir_imm_int(b, 0x100));
      break;
   }
   case nir_intrinsic_atomic_add_xfb_prim_count_amd:
      nir_gds_atomic_add_amd(b, 32, intrin->src[0].ssa,
                             nir_imm_int(b, RADV_SHADER_QUERY_PRIM_XFB_OFFSET(nir_intrinsic_stream_id(intrin))),
                             nir_imm_int(b, 0x100));
      break;
   case nir_intrinsic_atomic_add_shader_invocation_count_amd: {
      uint32_t offset;

      if (stage == MESA_SHADER_MESH) {
         offset = RADV_SHADER_QUERY_MS_INVOCATION_OFFSET;
      } else if (stage == MESA_SHADER_TASK) {
         offset = RADV_SHADER_QUERY_TS_INVOCATION_OFFSET;
      } else {
         offset = RADV_SHADER_QUERY_GS_INVOCATION_OFFSET;
      }

      nir_gds_atomic_add_amd(b, 32, intrin->src[0].ssa, nir_imm_int(b, offset), nir_imm_int(b, 0x100));
      break;
   }
   case nir_intrinsic_load_streamout_config_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.streamout_config);
      break;
   case nir_intrinsic_load_streamout_write_index_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.streamout_write_index);
      break;
   case nir_intrinsic_load_streamout_buffer_amd: {
      nir_def *ptr = nir_pack_64_2x32_split(b, ac_nir_load_arg(b, &s->args->ac, s->args->streamout_buffers),
                                            nir_imm_int(b, s->address32_hi));
      replacement = nir_load_smem_amd(b, 4, ptr, nir_imm_int(b, nir_intrinsic_base(intrin) * 16));
      break;
   }
   case nir_intrinsic_load_streamout_offset_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.streamout_offset[nir_intrinsic_base(intrin)]);
      break;

   case nir_intrinsic_load_lds_ngg_gs_out_vertex_base_amd:
      replacement = nir_imm_int(b, s->info->ngg_info.esgs_ring_size);
      break;
   case nir_intrinsic_load_lds_ngg_scratch_base_amd:
      replacement = nir_imm_int(b, s->info->ngg_info.scratch_lds_base);
      break;
   case nir_intrinsic_load_num_vertices_per_primitive_amd: {
      unsigned num_vertices;

      if (stage == MESA_SHADER_VERTEX) {
         /* For dynamic primitive topology with streamout. */
         if (s->info->vs.dynamic_num_verts_per_prim) {
            replacement = ac_nir_load_arg(b, &s->args->ac, s->args->num_verts_per_prim);
         } else {
            replacement = nir_imm_int(b, radv_get_num_vertices_per_prim(s->pl_key));
         }
      } else if (stage == MESA_SHADER_TESS_EVAL) {
         if (s->info->tes.point_mode) {
            num_vertices = 1;
         } else if (s->info->tes._primitive_mode == TESS_PRIMITIVE_ISOLINES) {
            num_vertices = 2;
         } else {
            num_vertices = 3;
         }
         replacement = nir_imm_int(b, num_vertices);
      } else {
         assert(stage == MESA_SHADER_GEOMETRY);
         switch (s->info->gs.output_prim) {
         case MESA_PRIM_POINTS:
            num_vertices = 1;
            break;
         case MESA_PRIM_LINE_STRIP:
            num_vertices = 2;
            break;
         case MESA_PRIM_TRIANGLE_STRIP:
            num_vertices = 3;
            break;
         default:
            unreachable("invalid GS output primitive");
            break;
         }
         replacement = nir_imm_int(b, num_vertices);
      }
      break;
   }
   case nir_intrinsic_load_ordered_id_amd:
      replacement = ac_nir_unpack_arg(b, &s->args->ac, s->args->ac.gs_tg_info, 0, 12);
      break;
   case nir_intrinsic_load_force_vrs_rates_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.force_vrs_rates);
      break;
   case nir_intrinsic_load_fully_covered: {
      nir_def *sample_coverage = ac_nir_load_arg(b, &s->args->ac, s->args->ac.sample_coverage);
      replacement = nir_ine_imm(b, sample_coverage, 0);
      break;
   }
   case nir_intrinsic_load_barycentric_optimize_amd: {
      nir_def *prim_mask = ac_nir_load_arg(b, &s->args->ac, s->args->ac.prim_mask);
      /* enabled when bit 31 is set */
      replacement = nir_ilt_imm(b, prim_mask, 0);
      break;
   }
   case nir_intrinsic_load_poly_line_smooth_enabled:
      if (s->pl_key->dynamic_line_rast_mode) {
         nir_def *line_rast_mode = GET_SGPR_FIELD_NIR(s->args->ps_state, PS_STATE_LINE_RAST_MODE);
         replacement = nir_ieq_imm(b, line_rast_mode, VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT);
      } else {
         replacement = nir_imm_bool(b, s->pl_key->ps.line_smooth_enabled);
      }
      break;
   case nir_intrinsic_load_initial_edgeflags_amd:
      replacement = nir_imm_int(b, 0);
      break;
   case nir_intrinsic_load_provoking_vtx_amd:
      replacement = ac_nir_load_arg(b, &s->args->ac, s->args->ac.load_provoking_vtx);
      break;
   case nir_intrinsic_load_rasterization_primitive_amd:
      assert(s->pl_key->unknown_rast_prim);
      /* Load the primitive topology from an user SGPR when it's unknown at compile time (GPL). */
      replacement = GET_SGPR_FIELD_NIR(s->args->ps_state, PS_STATE_RAST_PRIM);
      break;
   default:
      progress = false;
      break;
   }

   if (!progress)
      return false;

   if (replacement)
      nir_def_rewrite_uses(&intrin->def, replacement);

   nir_instr_remove(&intrin->instr);
   nir_instr_free(&intrin->instr);

   return true;
}

static nir_def *
load_gsvs_ring(nir_builder *b, lower_abi_state *s, unsigned stream_id)
{
   nir_def *ring = load_ring(b, RING_GSVS_GS, s);
   unsigned stream_offset = 0;
   unsigned stride = 0;
   for (unsigned i = 0; i <= stream_id; i++) {
      stride = 4 * s->info->gs.num_stream_output_components[i] * s->info->gs.vertices_out;
      if (i < stream_id)
         stream_offset += stride * s->info->wave_size;
   }

   /* Limit on the stride field for <= GFX7. */
   assert(stride < (1 << 14));

   if (stream_offset) {
      nir_def *addr = nir_pack_64_2x32_split(b, nir_channel(b, ring, 0), nir_channel(b, ring, 1));
      addr = nir_iadd_imm(b, addr, stream_offset);
      ring = nir_vector_insert_imm(b, ring, nir_unpack_64_2x32_split_x(b, addr), 0);
      ring = nir_vector_insert_imm(b, ring, nir_unpack_64_2x32_split_y(b, addr), 1);
   }

   ring = nir_vector_insert_imm(b, ring, nir_ior_imm(b, nir_channel(b, ring, 1), S_008F04_STRIDE(stride)), 1);
   return nir_vector_insert_imm(b, ring, nir_imm_int(b, s->info->wave_size), 2);
}

void
radv_nir_lower_abi(nir_shader *shader, enum amd_gfx_level gfx_level, const struct radv_shader_info *info,
                   const struct radv_shader_args *args, const struct radv_pipeline_key *pl_key, uint32_t address32_hi)
{
   lower_abi_state state = {
      .gfx_level = gfx_level,
      .info = info,
      .args = args,
      .pl_key = pl_key,
      .address32_hi = address32_hi,
   };

   if (shader->info.stage == MESA_SHADER_GEOMETRY && !info->is_ngg) {
      nir_function_impl *impl = nir_shader_get_entrypoint(shader);

      nir_builder b = nir_builder_at(nir_before_impl(impl));

      u_foreach_bit (i, shader->info.gs.active_stream_mask)
         state.gsvs_ring[i] = load_gsvs_ring(&b, &state, i);
   }

   nir_shader_intrinsics_pass(shader, lower_abi_instr, nir_metadata_dominance | nir_metadata_block_index, &state);
}
