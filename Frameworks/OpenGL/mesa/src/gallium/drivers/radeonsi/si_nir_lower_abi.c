/*
 * Copyright 2022 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "nir_builder.h"

#include "ac_nir.h"
#include "si_pipe.h"
#include "si_query.h"
#include "si_state.h"
#include "si_shader_internal.h"

struct lower_abi_state {
   struct si_shader *shader;
   struct si_shader_args *args;

   nir_def *esgs_ring;
   nir_def *tess_offchip_ring;
   nir_def *gsvs_ring[4];
};

#define GET_FIELD_NIR(field) \
   ac_nir_unpack_arg(b, &args->ac, args->vs_state_bits, \
                     field##__SHIFT, util_bitcount(field##__MASK))

nir_def *si_nir_load_internal_binding(nir_builder *b, struct si_shader_args *args,
                                          unsigned slot, unsigned num_components)
{
   nir_def *addr = ac_nir_load_arg(b, &args->ac, args->internal_bindings);
   return nir_load_smem_amd(b, num_components, addr, nir_imm_int(b, slot * 16));
}

static nir_def *get_num_vert_per_prim(nir_builder *b, struct si_shader *shader,
                                          struct si_shader_args *args)
{
   const struct si_shader_info *info = &shader->selector->info;
   gl_shader_stage stage = shader->selector->stage;

   unsigned num_vertices;
   if (stage == MESA_SHADER_GEOMETRY) {
      num_vertices = mesa_vertices_per_prim(info->base.gs.output_primitive);
   } else if (stage == MESA_SHADER_VERTEX) {
      if (info->base.vs.blit_sgprs_amd)
         num_vertices = 3;
      else if (shader->key.ge.opt.ngg_culling & SI_NGG_CULL_LINES)
         num_vertices = 2;
      else {
         /* Extract OUTPRIM field. */
         nir_def *num = GET_FIELD_NIR(GS_STATE_OUTPRIM);
         return nir_iadd_imm(b, num, 1);
      }
   } else {
      assert(stage == MESA_SHADER_TESS_EVAL);

      if (info->base.tess.point_mode)
         num_vertices = 1;
      else if (info->base.tess._primitive_mode == TESS_PRIMITIVE_ISOLINES)
         num_vertices = 2;
      else
         num_vertices = 3;
   }
   return nir_imm_int(b, num_vertices);
}

static nir_def *build_attr_ring_desc(nir_builder *b, struct si_shader *shader,
                                         struct si_shader_args *args)
{
   struct si_shader_selector *sel = shader->selector;

   nir_def *attr_address =
      sel->stage == MESA_SHADER_VERTEX && sel->info.base.vs.blit_sgprs_amd ?
      ac_nir_load_arg_at_offset(b, &args->ac, args->vs_blit_inputs,
                                sel->info.base.vs.blit_sgprs_amd - 1) :
      ac_nir_load_arg(b, &args->ac, args->gs_attr_address);

   unsigned stride = 16 * shader->info.nr_param_exports;
   nir_def *comp[] = {
      attr_address,
      nir_imm_int(b, S_008F04_BASE_ADDRESS_HI(sel->screen->info.address32_hi) |
                  S_008F04_STRIDE(stride) |
                  S_008F04_SWIZZLE_ENABLE_GFX11(3) /* 16B */),
      nir_imm_int(b, 0xffffffff),
      nir_imm_int(b, S_008F0C_DST_SEL_X(V_008F0C_SQ_SEL_X) |
                  S_008F0C_DST_SEL_Y(V_008F0C_SQ_SEL_Y) |
                  S_008F0C_DST_SEL_Z(V_008F0C_SQ_SEL_Z) |
                  S_008F0C_DST_SEL_W(V_008F0C_SQ_SEL_W) |
                  S_008F0C_FORMAT(V_008F0C_GFX11_FORMAT_32_32_32_32_FLOAT) |
                  S_008F0C_INDEX_STRIDE(2) /* 32 elements */),
   };

   return nir_vec(b, comp, 4);
}

static nir_def *
fetch_framebuffer(nir_builder *b, struct si_shader_args *args,
                  struct si_shader_selector *sel, union si_shader_key *key)
{
   /* Load the image descriptor. */
   STATIC_ASSERT(SI_PS_IMAGE_COLORBUF0 % 2 == 0);
   STATIC_ASSERT(SI_PS_IMAGE_COLORBUF0_FMASK % 2 == 0);

   nir_def *zero = nir_imm_zero(b, 1, 32);
   nir_def *undef = nir_undef(b, 1, 32);

   unsigned chan = 0;
   nir_def *vec[4] = {undef, undef, undef, undef};

   vec[chan++] = ac_nir_unpack_arg(b, &args->ac, args->ac.pos_fixed_pt, 0, 16);

   if (!key->ps.mono.fbfetch_is_1D)
      vec[chan++] = ac_nir_unpack_arg(b, &args->ac, args->ac.pos_fixed_pt, 16, 16);

   /* Get the current render target layer index. */
   if (key->ps.mono.fbfetch_layered)
      vec[chan++] = ac_nir_unpack_arg(b, &args->ac, args->ac.ancillary, 16, 11);

   nir_def *coords = nir_vec(b, vec, 4);

   enum glsl_sampler_dim dim;
   if (key->ps.mono.fbfetch_msaa)
      dim = GLSL_SAMPLER_DIM_MS;
   else if (key->ps.mono.fbfetch_is_1D)
      dim = GLSL_SAMPLER_DIM_1D;
   else
      dim = GLSL_SAMPLER_DIM_2D;

   nir_def *sample_id;
   if (key->ps.mono.fbfetch_msaa) {
      sample_id = ac_nir_unpack_arg(b, &args->ac, args->ac.ancillary, 8, 4);

      if (sel->screen->info.gfx_level < GFX11 &&
          !(sel->screen->debug_flags & DBG(NO_FMASK))) {
         nir_def *desc =
            si_nir_load_internal_binding(b, args, SI_PS_IMAGE_COLORBUF0_FMASK, 8);

         nir_def *fmask =
            nir_bindless_image_fragment_mask_load_amd(
               b, desc, coords,
               .image_dim = dim,
               .image_array = key->ps.mono.fbfetch_layered,
               .access = ACCESS_CAN_REORDER);

         nir_def *offset = nir_ishl_imm(b, sample_id, 2);
         /* 3 for EQAA handling, see lower_image_to_fragment_mask_load() */
         nir_def *width = nir_imm_int(b, 3);
         sample_id = nir_ubfe(b, fmask, offset, width);
      }
   } else {
      sample_id = zero;
   }

   nir_def *desc = si_nir_load_internal_binding(b, args, SI_PS_IMAGE_COLORBUF0, 8);

   return nir_bindless_image_load(b, 4, 32, desc, coords, sample_id, zero,
                                  .image_dim = dim,
                                  .image_array = key->ps.mono.fbfetch_layered,
                                  .access = ACCESS_CAN_REORDER);
}

static nir_def *build_tess_ring_desc(nir_builder *b, struct si_screen *screen,
                                         struct si_shader_args *args)
{
   nir_def *addr = ac_nir_load_arg(b, &args->ac, args->tes_offchip_addr);

   uint32_t rsrc3 =
      S_008F0C_DST_SEL_X(V_008F0C_SQ_SEL_X) |
      S_008F0C_DST_SEL_Y(V_008F0C_SQ_SEL_Y) |
      S_008F0C_DST_SEL_Z(V_008F0C_SQ_SEL_Z) |
      S_008F0C_DST_SEL_W(V_008F0C_SQ_SEL_W);

   if (screen->info.gfx_level >= GFX11) {
      rsrc3 |= S_008F0C_FORMAT(V_008F0C_GFX11_FORMAT_32_FLOAT) |
               S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW);
   } else if (screen->info.gfx_level >= GFX10) {
      rsrc3 |= S_008F0C_FORMAT(V_008F0C_GFX10_FORMAT_32_FLOAT) |
               S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_RAW) |
               S_008F0C_RESOURCE_LEVEL(1);
   } else {
      rsrc3 |= S_008F0C_NUM_FORMAT(V_008F0C_BUF_NUM_FORMAT_FLOAT) |
               S_008F0C_DATA_FORMAT(V_008F0C_BUF_DATA_FORMAT_32);
   }

   nir_def *comp[4] = {
      addr,
      nir_imm_int(b, S_008F04_BASE_ADDRESS_HI(screen->info.address32_hi)),
      nir_imm_int(b, 0xffffffff),
      nir_imm_int(b, rsrc3),
   };

   return nir_vec(b, comp, 4);
}

static nir_def *build_esgs_ring_desc(nir_builder *b, enum amd_gfx_level gfx_level,
                                         struct si_shader_args *args)
{
   nir_def *desc = si_nir_load_internal_binding(b, args, SI_RING_ESGS, 4);

   if (b->shader->info.stage == MESA_SHADER_GEOMETRY)
      return desc;

   nir_def *vec[4];
   for (int i = 0; i < 4; i++)
      vec[i] = nir_channel(b, desc, i);

   vec[1] = nir_ior_imm(b, vec[1], S_008F04_SWIZZLE_ENABLE_GFX6(1));
   vec[3] = nir_ior_imm(b, vec[3],
                        S_008F0C_ELEMENT_SIZE(1) |
                        S_008F0C_INDEX_STRIDE(3) |
                        S_008F0C_ADD_TID_ENABLE(1));

   /* If MUBUF && ADD_TID_ENABLE, DATA_FORMAT means STRIDE[14:17] on gfx8-9, so set 0. */
   if (gfx_level == GFX8)
      vec[3] = nir_iand_imm(b, vec[3], C_008F0C_DATA_FORMAT);

   return nir_vec(b, vec, 4);
}

static void build_gsvs_ring_desc(nir_builder *b, struct lower_abi_state *s)
{
   const struct si_shader_selector *sel = s->shader->selector;
   const union si_shader_key *key = &s->shader->key;

   if (s->shader->is_gs_copy_shader) {
      s->gsvs_ring[0] = si_nir_load_internal_binding(b, s->args, SI_RING_GSVS, 4);
   } else if (sel->stage == MESA_SHADER_GEOMETRY && !key->ge.as_ngg) {
      nir_def *base_addr = si_nir_load_internal_binding(b, s->args, SI_RING_GSVS, 2);
      base_addr = nir_pack_64_2x32(b, base_addr);

      /* The conceptual layout of the GSVS ring is
       *   v0c0 .. vLv0 v0c1 .. vLc1 ..
       * but the real memory layout is swizzled across
       * threads:
       *   t0v0c0 .. t15v0c0 t0v1c0 .. t15v1c0 ... t15vLcL
       *   t16v0c0 ..
       * Override the buffer descriptor accordingly.
       */

      for (unsigned stream = 0; stream < 4; stream++) {
         unsigned num_components = sel->info.num_stream_output_components[stream];
         if (!num_components)
            continue;

         nir_def *desc[4];
         desc[0] = nir_unpack_64_2x32_split_x(b, base_addr);
         desc[1] = nir_unpack_64_2x32_split_y(b, base_addr);

         unsigned stride = 4 * num_components * sel->info.base.gs.vertices_out;
         /* Limit on the stride field for <= GFX7. */
         assert(stride < (1 << 14));

         desc[1] = nir_ior_imm(
            b, desc[1], S_008F04_STRIDE(stride) | S_008F04_SWIZZLE_ENABLE_GFX6(1));

         unsigned num_records = s->shader->wave_size;
         desc[2] = nir_imm_int(b, num_records);

         uint32_t rsrc3 =
            S_008F0C_DST_SEL_X(V_008F0C_SQ_SEL_X) |
            S_008F0C_DST_SEL_Y(V_008F0C_SQ_SEL_Y) |
            S_008F0C_DST_SEL_Z(V_008F0C_SQ_SEL_Z) |
            S_008F0C_DST_SEL_W(V_008F0C_SQ_SEL_W) |
            S_008F0C_INDEX_STRIDE(1) | /* index_stride = 16 (elements) */
            S_008F0C_ADD_TID_ENABLE(1);

         if (sel->screen->info.gfx_level >= GFX10) {
            rsrc3 |=
               S_008F0C_FORMAT(V_008F0C_GFX10_FORMAT_32_FLOAT) |
               S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_DISABLED) |
               S_008F0C_RESOURCE_LEVEL(1);
         } else {
            /* If MUBUF && ADD_TID_ENABLE, DATA_FORMAT means STRIDE[14:17] on gfx8-9, so set 0. */
            unsigned data_format =
               sel->screen->info.gfx_level == GFX8 || sel->screen->info.gfx_level == GFX9 ?
               0 : V_008F0C_BUF_DATA_FORMAT_32;

            rsrc3 |=
               S_008F0C_NUM_FORMAT(V_008F0C_BUF_NUM_FORMAT_FLOAT) |
               S_008F0C_DATA_FORMAT(data_format) |
               S_008F0C_ELEMENT_SIZE(1); /* element_size = 4 (bytes) */
         }

         desc[3] = nir_imm_int(b, rsrc3);

         s->gsvs_ring[stream] = nir_vec(b, desc, 4);

         /* next stream's desc addr */
         base_addr = nir_iadd_imm(b, base_addr, stride * num_records);
      }
   }
}

static void preload_reusable_variables(nir_builder *b, struct lower_abi_state *s)
{
   const struct si_shader_selector *sel = s->shader->selector;
   const union si_shader_key *key = &s->shader->key;

   b->cursor = nir_before_impl(b->impl);

   if (sel->screen->info.gfx_level <= GFX8 && sel->stage <= MESA_SHADER_GEOMETRY &&
       (key->ge.as_es || sel->stage == MESA_SHADER_GEOMETRY)) {
      s->esgs_ring = build_esgs_ring_desc(b, sel->screen->info.gfx_level, s->args);
   }

   if (sel->stage == MESA_SHADER_TESS_CTRL || sel->stage == MESA_SHADER_TESS_EVAL)
      s->tess_offchip_ring = build_tess_ring_desc(b, sel->screen, s->args);

   build_gsvs_ring_desc(b, s);
}

static bool lower_intrinsic(nir_builder *b, nir_instr *instr, struct lower_abi_state *s)
{
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   struct si_shader *shader = s->shader;
   struct si_shader_args *args = s->args;
   struct si_shader_selector *sel = shader->selector;
   union si_shader_key *key = &shader->key;
   gl_shader_stage stage = sel->stage;

   b->cursor = nir_before_instr(instr);

   nir_def *replacement = NULL;

   switch (intrin->intrinsic) {
   case nir_intrinsic_load_first_vertex:
      replacement = ac_nir_load_arg(b, &args->ac, args->ac.base_vertex);
      break;
   case nir_intrinsic_load_base_vertex: {
      nir_def *indexed = GET_FIELD_NIR(VS_STATE_INDEXED);
      indexed = nir_i2b(b, indexed);

      nir_def *base_vertex = ac_nir_load_arg(b, &args->ac, args->ac.base_vertex);
      replacement = nir_bcsel(b, indexed, base_vertex, nir_imm_int(b, 0));
      break;
   }
   case nir_intrinsic_load_workgroup_size: {
      assert(sel->info.base.workgroup_size_variable && sel->info.uses_variable_block_size);

      nir_def *block_size = ac_nir_load_arg(b, &args->ac, args->block_size);
      nir_def *comp[] = {
         nir_ubfe_imm(b, block_size, 0, 10),
         nir_ubfe_imm(b, block_size, 10, 10),
         nir_ubfe_imm(b, block_size, 20, 10),
      };
      replacement = nir_vec(b, comp, 3);
      break;
   }
   case nir_intrinsic_load_tess_level_outer_default:
   case nir_intrinsic_load_tess_level_inner_default: {
      nir_def *buf = si_nir_load_internal_binding(b, args, SI_HS_CONST_DEFAULT_TESS_LEVELS, 4);
      unsigned num_components = intrin->def.num_components;
      unsigned offset =
         intrin->intrinsic == nir_intrinsic_load_tess_level_inner_default ? 16 : 0;
      replacement = nir_load_ubo(b, num_components, 32, buf, nir_imm_int(b, offset),
                                 .range = ~0);
      break;
   }
   case nir_intrinsic_load_patch_vertices_in:
      if (stage == MESA_SHADER_TESS_CTRL)
         replacement = ac_nir_unpack_arg(b, &args->ac, args->tcs_offchip_layout, 11, 5);
      else if (stage == MESA_SHADER_TESS_EVAL) {
         replacement = ac_nir_unpack_arg(b, &args->ac, args->tcs_offchip_layout, 6, 5);
      } else
         unreachable("no nir_load_patch_vertices_in");
      replacement = nir_iadd_imm(b, replacement, 1);
      break;
   case nir_intrinsic_load_sample_mask_in:
      replacement = ac_nir_load_arg(b, &args->ac, args->ac.sample_coverage);
      break;
   case nir_intrinsic_load_lshs_vertex_stride_amd:
      if (stage == MESA_SHADER_VERTEX)
         replacement = nir_imm_int(b, sel->info.lshs_vertex_stride);
      else if (stage == MESA_SHADER_TESS_CTRL)
         replacement = sel->screen->info.gfx_level >= GFX9 && shader->is_monolithic ?
            nir_imm_int(b, key->ge.part.tcs.ls->info.lshs_vertex_stride) :
            nir_ishl_imm(b, GET_FIELD_NIR(VS_STATE_LS_OUT_VERTEX_SIZE), 2);
      else
         unreachable("no nir_load_lshs_vertex_stride_amd");
      break;
   case nir_intrinsic_load_esgs_vertex_stride_amd:
      assert(sel->screen->info.gfx_level >= GFX9);
      replacement = shader->is_monolithic ?
         nir_imm_int(b, key->ge.part.gs.es->info.esgs_vertex_stride / 4) :
         GET_FIELD_NIR(GS_STATE_ESGS_VERTEX_STRIDE);
      break;
   case nir_intrinsic_load_tcs_num_patches_amd: {
      nir_def *tmp = ac_nir_unpack_arg(b, &args->ac, args->tcs_offchip_layout, 0, 6);
      replacement = nir_iadd_imm(b, tmp, 1);
      break;
   }
   case nir_intrinsic_load_hs_out_patch_data_offset_amd:
      replacement = ac_nir_unpack_arg(b, &args->ac, args->tcs_offchip_layout, 16, 16);
      break;
   case nir_intrinsic_load_ring_tess_offchip_offset_amd:
      replacement = ac_nir_load_arg(b, &args->ac, args->ac.tess_offchip_offset);
      break;
   case nir_intrinsic_load_ring_es2gs_offset_amd:
      replacement = ac_nir_load_arg(b, &args->ac, args->ac.es2gs_offset);
      break;
   case nir_intrinsic_load_clip_half_line_width_amd: {
      nir_def *addr = ac_nir_load_arg(b, &args->ac, args->small_prim_cull_info);
      replacement = nir_load_smem_amd(b, 2, addr, nir_imm_int(b, 32));
      break;
   }
   case nir_intrinsic_load_viewport_xy_scale_and_offset: {
      bool prim_is_lines = key->ge.opt.ngg_culling & SI_NGG_CULL_LINES;
      nir_def *addr = ac_nir_load_arg(b, &args->ac, args->small_prim_cull_info);
      unsigned offset = prim_is_lines ? 16 : 0;
      replacement = nir_load_smem_amd(b, 4, addr, nir_imm_int(b, offset));
      break;
   }
   case nir_intrinsic_load_num_vertices_per_primitive_amd:
      replacement = get_num_vert_per_prim(b, shader, args);
      break;
   case nir_intrinsic_load_cull_ccw_amd:
      /* radeonsi embed cw/ccw info into front/back face enabled */
      replacement = nir_imm_false(b);
      break;
   case nir_intrinsic_load_cull_any_enabled_amd:
      replacement = nir_imm_bool(b, !!key->ge.opt.ngg_culling);
      break;
   case nir_intrinsic_load_cull_back_face_enabled_amd:
      replacement = nir_imm_bool(b, key->ge.opt.ngg_culling & SI_NGG_CULL_BACK_FACE);
      break;
   case nir_intrinsic_load_cull_front_face_enabled_amd:
      replacement = nir_imm_bool(b, key->ge.opt.ngg_culling & SI_NGG_CULL_FRONT_FACE);
      break;
   case nir_intrinsic_load_cull_small_prim_precision_amd: {
      nir_def *small_prim_precision =
         key->ge.opt.ngg_culling & SI_NGG_CULL_LINES ?
         GET_FIELD_NIR(GS_STATE_SMALL_PRIM_PRECISION_NO_AA) :
         GET_FIELD_NIR(GS_STATE_SMALL_PRIM_PRECISION);

      /* Extract the small prim precision. */
      small_prim_precision = nir_ior_imm(b, small_prim_precision, 0x70);
      replacement = nir_ishl_imm(b, small_prim_precision, 23);
      break;
   }
   case nir_intrinsic_load_cull_small_primitives_enabled_amd: {
      unsigned mask = SI_NGG_CULL_LINES | SI_NGG_CULL_SMALL_LINES_DIAMOND_EXIT;
      replacement = nir_imm_bool(b, (key->ge.opt.ngg_culling & mask) != SI_NGG_CULL_LINES);
      break;
   }
   case nir_intrinsic_load_provoking_vtx_in_prim_amd:
      replacement = GET_FIELD_NIR(GS_STATE_PROVOKING_VTX_INDEX);
      break;
   case nir_intrinsic_load_pipeline_stat_query_enabled_amd:
      replacement = nir_i2b(b, GET_FIELD_NIR(GS_STATE_PIPELINE_STATS_EMU));
      break;
   case nir_intrinsic_load_prim_gen_query_enabled_amd:
   case nir_intrinsic_load_prim_xfb_query_enabled_amd:
      replacement = nir_i2b(b, GET_FIELD_NIR(GS_STATE_STREAMOUT_QUERY_ENABLED));
      break;
   case nir_intrinsic_load_clamp_vertex_color_amd:
      replacement = nir_i2b(b, GET_FIELD_NIR(VS_STATE_CLAMP_VERTEX_COLOR));
      break;
   case nir_intrinsic_load_user_clip_plane: {
      nir_def *buf = si_nir_load_internal_binding(b, args, SI_VS_CONST_CLIP_PLANES, 4);
      unsigned offset = nir_intrinsic_ucp_id(intrin) * 16;
      replacement = nir_load_ubo(b, 4, 32, buf, nir_imm_int(b, offset),
                                 .range = ~0);
      break;
   }
   case nir_intrinsic_load_streamout_buffer_amd: {
      unsigned slot = SI_VS_STREAMOUT_BUF0 + nir_intrinsic_base(intrin);
      replacement = si_nir_load_internal_binding(b, args, slot, 4);
      break;
   }
   case nir_intrinsic_atomic_add_gs_emit_prim_count_amd:
   case nir_intrinsic_atomic_add_shader_invocation_count_amd: {
      nir_def *buf =
         si_nir_load_internal_binding(b, args, SI_GS_QUERY_EMULATED_COUNTERS_BUF, 4);

      enum pipe_statistics_query_index index =
         intrin->intrinsic == nir_intrinsic_atomic_add_gs_emit_prim_count_amd ?
         PIPE_STAT_QUERY_GS_PRIMITIVES : PIPE_STAT_QUERY_GS_INVOCATIONS;
      unsigned offset = si_query_pipestat_end_dw_offset(sel->screen, index) * 4;

      nir_def *count = intrin->src[0].ssa;
      nir_ssbo_atomic(b, 32, buf, nir_imm_int(b, offset), count,
                      .atomic_op = nir_atomic_op_iadd);
      break;
   }
   case nir_intrinsic_atomic_add_gen_prim_count_amd:
   case nir_intrinsic_atomic_add_xfb_prim_count_amd: {
      nir_def *buf = si_nir_load_internal_binding(b, args, SI_GS_QUERY_BUF, 4);

      unsigned stream = nir_intrinsic_stream_id(intrin);
      unsigned offset = intrin->intrinsic == nir_intrinsic_atomic_add_gen_prim_count_amd ?
         offsetof(struct gfx11_sh_query_buffer_mem, stream[stream].generated_primitives) :
         offsetof(struct gfx11_sh_query_buffer_mem, stream[stream].emitted_primitives);

      nir_def *prim_count = intrin->src[0].ssa;
      nir_ssbo_atomic(b, 32, buf, nir_imm_int(b, offset), prim_count,
                      .atomic_op = nir_atomic_op_iadd);
      break;
   }
   case nir_intrinsic_load_ring_attr_amd:
      replacement = build_attr_ring_desc(b, shader, args);
      break;
   case nir_intrinsic_load_ring_attr_offset_amd: {
      nir_def *offset = ac_nir_unpack_arg(b, &args->ac, args->ac.gs_attr_offset, 0, 15);
      replacement = nir_ishl_imm(b, offset, 9);
      break;
   }
   case nir_intrinsic_load_ring_gs2vs_offset_amd:
      replacement = ac_nir_load_arg(b, &args->ac, args->ac.gs2vs_offset);
      break;
   case nir_intrinsic_load_streamout_config_amd:
      replacement = ac_nir_load_arg(b, &args->ac, args->ac.streamout_config);
      break;
   case nir_intrinsic_load_streamout_write_index_amd:
      replacement = ac_nir_load_arg(b, &args->ac, args->ac.streamout_write_index);
      break;
   case nir_intrinsic_load_streamout_offset_amd:
      replacement =
         ac_nir_load_arg(b, &args->ac, args->ac.streamout_offset[nir_intrinsic_base(intrin)]);
      break;
   case nir_intrinsic_load_force_vrs_rates_amd:
      if (sel->screen->info.gfx_level >= GFX11) {
         /* Bits [2:5] = VRS rate
          *
          * The range is [0, 15].
          *
          * If the hw doesn't support VRS 4x4, it will silently use 2x2 instead.
          */
         replacement = nir_imm_int(b, V_0283D0_VRS_SHADING_RATE_4X4 << 2);
      } else {
         /* Bits [2:3] = VRS rate X
          * Bits [4:5] = VRS rate Y
          *
          * The range is [-2, 1]. Values:
          *   1: 2x coarser shading rate in that direction.
          *   0: normal shading rate
          *  -1: 2x finer shading rate (sample shading, not directional)
          *  -2: 4x finer shading rate (sample shading, not directional)
          *
          * Sample shading can't go above 8 samples, so both numbers can't be -2
          * at the same time.
          */
         replacement = nir_imm_int(b, (1 << 2) | (1 << 4));
      }
      break;
   case nir_intrinsic_load_barycentric_at_sample: {
      unsigned mode = nir_intrinsic_interp_mode(intrin);

      if (key->ps.mono.interpolate_at_sample_force_center) {
         replacement = nir_load_barycentric_pixel(b, 32, .interp_mode = mode);
      } else {
         nir_def *sample_id = intrin->src[0].ssa;
         /* offset = sample_id * 8  (8 = 2 floats containing samplepos.xy) */
         nir_def *offset = nir_ishl_imm(b, sample_id, 3);

         nir_def *buf = si_nir_load_internal_binding(b, args, SI_PS_CONST_SAMPLE_POSITIONS, 4);
         nir_def *sample_pos = nir_load_ubo(b, 2, 32, buf, offset, .range = ~0);

         sample_pos = nir_fadd_imm(b, sample_pos, -0.5);

         replacement = nir_load_barycentric_at_offset(b, 32, sample_pos, .interp_mode = mode);
      }
      break;
   }
   case nir_intrinsic_load_output: {
      nir_io_semantics sem = nir_intrinsic_io_semantics(intrin);

      /* not fbfetch */
      if (!(stage == MESA_SHADER_FRAGMENT && sem.fb_fetch_output))
         return false;

      /* Ignore src0, because KHR_blend_func_extended disallows multiple render targets. */

      replacement = fetch_framebuffer(b, args, sel, key);
      break;
   }
   case nir_intrinsic_load_ring_tess_factors_amd: {
      assert(s->tess_offchip_ring);
      nir_def *addr = nir_channel(b, s->tess_offchip_ring, 0);
      addr = nir_iadd_imm(b, addr, sel->screen->hs.tess_offchip_ring_size);
      replacement = nir_vector_insert_imm(b, s->tess_offchip_ring, addr, 0);
      break;
   }
   case nir_intrinsic_load_ring_tess_factors_offset_amd:
      replacement = ac_nir_load_arg(b, &args->ac, args->ac.tcs_factor_offset);
      break;
   case nir_intrinsic_load_alpha_reference_amd:
      replacement = ac_nir_load_arg(b, &args->ac, args->alpha_reference);
      break;
   case nir_intrinsic_load_front_face:
      if (!key->ps.opt.force_front_face_input)
         return false;
      replacement = nir_imm_bool(b, key->ps.opt.force_front_face_input == 1);
      break;
   case nir_intrinsic_load_barycentric_optimize_amd: {
      nir_def *prim_mask = ac_nir_load_arg(b, &args->ac, args->ac.prim_mask);
      /* enabled when bit 31 is set */
      replacement = nir_ilt_imm(b, prim_mask, 0);
      break;
   }
   case nir_intrinsic_load_layer_id:
      replacement = ac_nir_unpack_arg(b, &args->ac, args->ac.ancillary, 16, 13);
      break;
   case nir_intrinsic_load_color0:
   case nir_intrinsic_load_color1: {
      uint32_t colors_read = sel->info.colors_read;

      int start, offset;
      if (intrin->intrinsic == nir_intrinsic_load_color0) {
         start = 0;
         offset = 0;
      } else {
         start = 4;
         offset = util_bitcount(colors_read & 0xf);
      }

      nir_def *color[4];
      for (int i = 0; i < 4; i++) {
         if (colors_read & BITFIELD_BIT(start + i)) {
            color[i] = ac_nir_load_arg_at_offset(b, &args->ac, args->color_start, offset++);

            nir_intrinsic_set_flags(nir_instr_as_intrinsic(color[i]->parent_instr),
                                    SI_VECTOR_ARG_IS_COLOR |
                                    SI_VECTOR_ARG_COLOR_COMPONENT(start + i));
         } else {
            color[i] = nir_undef(b, 1, 32);
         }
      }

      replacement = nir_vec(b, color, 4);
      break;
   }
   case nir_intrinsic_load_point_coord_maybe_flipped: {
      nir_def *interp_param =
         nir_load_barycentric_pixel(b, 32, .interp_mode = INTERP_MODE_NONE);

      /* Load point coordinates (x, y) which are written by the hw after the interpolated inputs */
      replacement = nir_load_interpolated_input(b, 2, 32, interp_param, nir_imm_int(b, 0),
                                                .base = si_get_ps_num_interp(shader),
                                                .component = 2,
                                                /* This tells si_nir_scan_shader that it's PARAM_GEN */
                                                .io_semantics.no_varying = 1);
      break;
   }
   case nir_intrinsic_load_poly_line_smooth_enabled:
      replacement = nir_imm_bool(b, key->ps.mono.poly_line_smoothing);
      break;
   case nir_intrinsic_load_gs_vertex_offset_amd: {
      unsigned base = nir_intrinsic_base(intrin);
      replacement = ac_nir_load_arg(b, &args->ac, args->ac.gs_vtx_offset[base]);
      break;
   }
   case nir_intrinsic_load_merged_wave_info_amd:
      replacement = ac_nir_load_arg(b, &args->ac, args->ac.merged_wave_info);
      break;
   case nir_intrinsic_load_workgroup_num_input_vertices_amd:
      replacement = ac_nir_unpack_arg(b, &args->ac, args->ac.gs_tg_info, 12, 9);
      break;
   case nir_intrinsic_load_workgroup_num_input_primitives_amd:
      replacement = ac_nir_unpack_arg(b, &args->ac, args->ac.gs_tg_info, 22, 9);
      break;
   case nir_intrinsic_load_initial_edgeflags_amd:
      if (shader->key.ge.opt.ngg_culling & SI_NGG_CULL_LINES ||
          (shader->selector->stage == MESA_SHADER_VERTEX &&
           shader->selector->info.base.vs.blit_sgprs_amd)) {
         /* Line primitives and blits don't need edge flags. */
         replacement = nir_imm_int(b, 0);
      } else if (shader->selector->stage == MESA_SHADER_VERTEX) {
         /* Use the following trick to extract the edge flags:
          *   extracted = v_and_b32 gs_invocation_id, 0x700 ; get edge flags at bits 8, 9, 10
          *   shifted = v_mul_u32_u24 extracted, 0x80402u   ; shift the bits: 8->9, 9->19, 10->29
          *   result = v_and_b32 shifted, 0x20080200        ; remove garbage
          */
         nir_def *tmp = ac_nir_load_arg(b, &args->ac, args->ac.gs_invocation_id);
         tmp = nir_iand_imm(b, tmp, 0x700);
         tmp = nir_imul_imm(b, tmp, 0x80402);
         replacement = nir_iand_imm(b, tmp, 0x20080200);
      } else {
         /* Edge flags are always enabled when polygon mode is enabled, so we always have to
          * return valid edge flags if the primitive type is not lines and if we are not blitting
          * because the shader doesn't know when polygon mode is enabled.
          */
         replacement = nir_imm_int(b, ac_get_all_edge_flag_bits());
      }
      break;
   case nir_intrinsic_load_packed_passthrough_primitive_amd:
      replacement = ac_nir_load_arg(b, &args->ac, args->ac.gs_vtx_offset[0]);
      break;
   case nir_intrinsic_load_ordered_id_amd:
      replacement = ac_nir_unpack_arg(b, &args->ac, args->ac.gs_tg_info, 0, 12);
      break;
   case nir_intrinsic_load_ring_esgs_amd:
      assert(s->esgs_ring);
      replacement = s->esgs_ring;
      break;
   case nir_intrinsic_load_tess_rel_patch_id_amd:
      /* LLVM need to replace patch id arg, so have to be done in LLVM backend. */
      if (!sel->screen->use_aco)
         return false;

      if (stage == MESA_SHADER_TESS_CTRL) {
         replacement = ac_nir_unpack_arg(b, &args->ac, args->ac.tcs_rel_ids, 0, 8);
      } else {
         assert(stage == MESA_SHADER_TESS_EVAL);
         replacement = ac_nir_load_arg(b, &args->ac, args->ac.tes_rel_patch_id);
      }
      break;
   case nir_intrinsic_load_ring_tess_offchip_amd:
      assert(s->tess_offchip_ring);
      replacement = s->tess_offchip_ring;
      break;
   case nir_intrinsic_load_ring_gsvs_amd: {
      unsigned stream_id = nir_intrinsic_stream_id(intrin);
      /* Unused nir_load_ring_gsvs_amd may not be eliminated yet. */
      replacement = s->gsvs_ring[stream_id] ?
         s->gsvs_ring[stream_id] : nir_undef(b, 4, 32);
      break;
   }
   case nir_intrinsic_load_user_data_amd:
      replacement = ac_nir_load_arg(b, &args->ac, args->cs_user_data);
      replacement = nir_pad_vec4(b, replacement);
      break;
   default:
      return false;
   }

   if (replacement)
      nir_def_rewrite_uses(&intrin->def, replacement);

   nir_instr_remove(instr);
   nir_instr_free(instr);

   return true;
}

static bool lower_tex(nir_builder *b, nir_instr *instr, struct lower_abi_state *s)
{
   nir_tex_instr *tex = nir_instr_as_tex(instr);
   const struct si_shader_selector *sel = s->shader->selector;
   enum amd_gfx_level gfx_level = sel->screen->info.gfx_level;

   b->cursor = nir_before_instr(instr);

   /* Section 8.23.1 (Depth Texture Comparison Mode) of the
    * OpenGL 4.5 spec says:
    *
    *    "If the texture’s internal format indicates a fixed-point
    *     depth texture, then D_t and D_ref are clamped to the
    *     range [0, 1]; otherwise no clamping is performed."
    *
    * TC-compatible HTILE promotes Z16 and Z24 to Z32_FLOAT,
    * so the depth comparison value isn't clamped for Z16 and
    * Z24 anymore. Do it manually here for GFX8-9; GFX10 has
    * an explicitly clamped 32-bit float format.
    */

   /* LLVM keep non-uniform sampler as index, so can't do this in NIR. */
   if (tex->is_shadow && gfx_level >= GFX8 && gfx_level <= GFX9 && sel->screen->use_aco) {
      int samp_index = nir_tex_instr_src_index(tex, nir_tex_src_sampler_handle);
      int comp_index = nir_tex_instr_src_index(tex, nir_tex_src_comparator);
      assert(samp_index >= 0 && comp_index >= 0);

      nir_def *sampler = tex->src[samp_index].src.ssa;
      nir_def *compare = tex->src[comp_index].src.ssa;
      /* Must have been lowered to descriptor. */
      assert(sampler->num_components > 1);

      nir_def *upgraded = nir_channel(b, sampler, 3);
      upgraded = nir_i2b(b, nir_ubfe_imm(b, upgraded, 29, 1));

      nir_def *clamped = nir_fsat(b, compare);
      compare = nir_bcsel(b, upgraded, clamped, compare);

      nir_src_rewrite(&tex->src[comp_index].src, compare);
      return true;
   }

   return false;
}

bool si_nir_lower_abi(nir_shader *nir, struct si_shader *shader, struct si_shader_args *args)
{
   struct lower_abi_state state = {
      .shader = shader,
      .args = args,
   };

   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   nir_builder b = nir_builder_create(impl);

   preload_reusable_variables(&b, &state);

   bool progress = false;
   nir_foreach_block_safe(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type == nir_instr_type_intrinsic)
            progress |= lower_intrinsic(&b, instr, &state);
         else if (instr->type == nir_instr_type_tex)
            progress |= lower_tex(&b, instr, &state);
      }
   }

   nir_metadata preserved = progress ?
      nir_metadata_dominance | nir_metadata_block_index :
      nir_metadata_all;
   nir_metadata_preserve(impl, preserved);

   return progress;
}
