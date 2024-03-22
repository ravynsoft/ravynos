/*
 * Copyright 2012 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_shader.h"
#include "ac_nir.h"
#include "ac_rtld.h"
#include "nir.h"
#include "nir_builder.h"
#include "nir_serialize.h"
#include "nir_xfb_info.h"
#include "si_pipe.h"
#include "si_shader_internal.h"
#include "sid.h"
#include "tgsi/tgsi_from_mesa.h"
#include "util/u_memory.h"
#include "util/mesa-sha1.h"
#include "util/ralloc.h"
#include "util/u_upload_mgr.h"

static const char scratch_rsrc_dword0_symbol[] = "SCRATCH_RSRC_DWORD0";

static const char scratch_rsrc_dword1_symbol[] = "SCRATCH_RSRC_DWORD1";

static void si_dump_shader_key(const struct si_shader *shader, FILE *f);
static void si_fix_resource_usage(struct si_screen *sscreen, struct si_shader *shader);

/* Get the number of all interpolated inputs */
unsigned si_get_ps_num_interp(struct si_shader *ps)
{
   unsigned num_colors = !!(ps->info.ps_colors_read & 0x0f) + !!(ps->info.ps_colors_read & 0xf0);
   unsigned num_interp =
      ps->info.num_ps_inputs + (ps->key.ps.part.prolog.color_two_side ? num_colors : 0);

   assert(num_interp <= 32);
   return MIN2(num_interp, 32);
}

/** Whether the shader runs as a combination of multiple API shaders */
bool si_is_multi_part_shader(struct si_shader *shader)
{
   if (shader->selector->screen->info.gfx_level <= GFX8 ||
       shader->selector->stage > MESA_SHADER_GEOMETRY)
      return false;

   return shader->key.ge.as_ls || shader->key.ge.as_es ||
          shader->selector->stage == MESA_SHADER_TESS_CTRL ||
          shader->selector->stage == MESA_SHADER_GEOMETRY;
}

/** Whether the shader runs on a merged HW stage (LSHS or ESGS) */
bool si_is_merged_shader(struct si_shader *shader)
{
   if (shader->selector->stage > MESA_SHADER_GEOMETRY || shader->is_gs_copy_shader)
      return false;

   return shader->key.ge.as_ngg || si_is_multi_part_shader(shader);
}

/**
 * Returns a unique index for a semantic name and index. The index must be
 * less than 64, so that a 64-bit bitmask of used inputs or outputs can be
 * calculated.
 */
unsigned si_shader_io_get_unique_index(unsigned semantic)
{
   switch (semantic) {
   case VARYING_SLOT_POS:
      return SI_UNIQUE_SLOT_POS;
   default:
      if (semantic >= VARYING_SLOT_VAR0 && semantic <= VARYING_SLOT_VAR31)
         return SI_UNIQUE_SLOT_VAR0 + (semantic - VARYING_SLOT_VAR0);

      if (semantic >= VARYING_SLOT_VAR0_16BIT && semantic <= VARYING_SLOT_VAR15_16BIT)
         return SI_UNIQUE_SLOT_VAR0_16BIT + (semantic - VARYING_SLOT_VAR0_16BIT);

      assert(!"invalid generic index");
      return 0;

   /* Legacy desktop GL varyings. */
   case VARYING_SLOT_FOGC:
      return SI_UNIQUE_SLOT_FOGC;
   case VARYING_SLOT_COL0:
      return SI_UNIQUE_SLOT_COL0;
   case VARYING_SLOT_COL1:
      return SI_UNIQUE_SLOT_COL1;
   case VARYING_SLOT_BFC0:
      return SI_UNIQUE_SLOT_BFC0;
   case VARYING_SLOT_BFC1:
      return SI_UNIQUE_SLOT_BFC1;
   case VARYING_SLOT_TEX0:
   case VARYING_SLOT_TEX1:
   case VARYING_SLOT_TEX2:
   case VARYING_SLOT_TEX3:
   case VARYING_SLOT_TEX4:
   case VARYING_SLOT_TEX5:
   case VARYING_SLOT_TEX6:
   case VARYING_SLOT_TEX7:
      return SI_UNIQUE_SLOT_TEX0 + (semantic - VARYING_SLOT_TEX0);
   case VARYING_SLOT_CLIP_VERTEX:
      return SI_UNIQUE_SLOT_CLIP_VERTEX;

   /* Varyings present in both GLES and desktop GL. */
   case VARYING_SLOT_CLIP_DIST0:
      return SI_UNIQUE_SLOT_CLIP_DIST0;
   case VARYING_SLOT_CLIP_DIST1:
      return SI_UNIQUE_SLOT_CLIP_DIST1;
   case VARYING_SLOT_PSIZ:
      return SI_UNIQUE_SLOT_PSIZ;
   case VARYING_SLOT_LAYER:
      return SI_UNIQUE_SLOT_LAYER;
   case VARYING_SLOT_VIEWPORT:
      return SI_UNIQUE_SLOT_VIEWPORT;
   case VARYING_SLOT_PRIMITIVE_ID:
      return SI_UNIQUE_SLOT_PRIMITIVE_ID;
   }
}

static void declare_streamout_params(struct si_shader_args *args, struct si_shader *shader)
{
   struct si_shader_selector *sel = shader->selector;

   if (shader->selector->screen->info.gfx_level >= GFX11) {
      /* NGG streamout. */
      if (sel->stage == MESA_SHADER_TESS_EVAL)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      return;
   }

   /* Streamout SGPRs. */
   if (si_shader_uses_streamout(shader)) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.streamout_config);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.streamout_write_index);

      /* A streamout buffer offset is loaded if the stride is non-zero. */
      for (int i = 0; i < 4; i++) {
         if (!sel->info.base.xfb_stride[i])
            continue;

         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.streamout_offset[i]);
      }
   } else if (sel->stage == MESA_SHADER_TESS_EVAL) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
   }
}

unsigned si_get_max_workgroup_size(const struct si_shader *shader)
{
   gl_shader_stage stage = shader->is_gs_copy_shader ?
      MESA_SHADER_VERTEX : shader->selector->stage;

   switch (stage) {
   case MESA_SHADER_VERTEX:
   case MESA_SHADER_TESS_EVAL:
      /* Use the largest workgroup size for streamout */
      if (shader->key.ge.as_ngg)
         return si_shader_uses_streamout(shader) ? 256 : 128;

      /* As part of merged shader. */
      return shader->selector->screen->info.gfx_level >= GFX9 &&
         (shader->key.ge.as_ls || shader->key.ge.as_es) ? 128 : 0;

   case MESA_SHADER_TESS_CTRL:
      /* Return this so that LLVM doesn't remove s_barrier
       * instructions on chips where we use s_barrier. */
      return shader->selector->screen->info.gfx_level >= GFX7 ? 128 : 0;

   case MESA_SHADER_GEOMETRY:
      /* GS can always generate up to 256 vertices. */
      return shader->selector->screen->info.gfx_level >= GFX9 ? 256 : 0;

   case MESA_SHADER_COMPUTE:
      break; /* see below */

   default:
      return 0;
   }

   /* Compile a variable block size using the maximum variable size. */
   if (shader->selector->info.base.workgroup_size_variable)
      return SI_MAX_VARIABLE_THREADS_PER_BLOCK;

   uint16_t *local_size = shader->selector->info.base.workgroup_size;
   unsigned max_work_group_size = (uint32_t)local_size[0] *
                                  (uint32_t)local_size[1] *
                                  (uint32_t)local_size[2];
   assert(max_work_group_size);
   return max_work_group_size;
}

static void declare_const_and_shader_buffers(struct si_shader_args *args,
                                             struct si_shader *shader,
                                             bool assign_params)
{
   enum ac_arg_type const_shader_buf_type;

   if (shader->selector->info.base.num_ubos == 1 &&
       shader->selector->info.base.num_ssbos == 0)
      const_shader_buf_type = AC_ARG_CONST_FLOAT_PTR;
   else
      const_shader_buf_type = AC_ARG_CONST_DESC_PTR;

   ac_add_arg(
      &args->ac, AC_ARG_SGPR, 1, const_shader_buf_type,
      assign_params ? &args->const_and_shader_buffers : &args->other_const_and_shader_buffers);
}

static void declare_samplers_and_images(struct si_shader_args *args, bool assign_params)
{
   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_CONST_IMAGE_PTR,
              assign_params ? &args->samplers_and_images : &args->other_samplers_and_images);
}

static void declare_per_stage_desc_pointers(struct si_shader_args *args,
                                            struct si_shader *shader,
                                            bool assign_params)
{
   declare_const_and_shader_buffers(args, shader, assign_params);
   declare_samplers_and_images(args, assign_params);
}

static void declare_global_desc_pointers(struct si_shader_args *args)
{
   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_CONST_DESC_PTR, &args->internal_bindings);
   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_CONST_IMAGE_PTR,
              &args->bindless_samplers_and_images);
}

static void declare_vb_descriptor_input_sgprs(struct si_shader_args *args,
                                              struct si_shader *shader)
{
   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_CONST_DESC_PTR, &args->ac.vertex_buffers);

   unsigned num_vbos_in_user_sgprs = shader->selector->info.num_vbos_in_user_sgprs;
   if (num_vbos_in_user_sgprs) {
      unsigned user_sgprs = args->ac.num_sgprs_used;

      if (si_is_merged_shader(shader))
         user_sgprs -= 8;
      assert(user_sgprs <= SI_SGPR_VS_VB_DESCRIPTOR_FIRST);

      /* Declare unused SGPRs to align VB descriptors to 4 SGPRs (hw requirement). */
      for (unsigned i = user_sgprs; i < SI_SGPR_VS_VB_DESCRIPTOR_FIRST; i++)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* unused */

      assert(num_vbos_in_user_sgprs <= ARRAY_SIZE(args->vb_descriptors));
      for (unsigned i = 0; i < num_vbos_in_user_sgprs; i++)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 4, AC_ARG_INT, &args->vb_descriptors[i]);
   }
}

static void declare_vs_input_vgprs(struct si_shader_args *args, struct si_shader *shader,
                                   unsigned *num_prolog_vgprs)
{
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.vertex_id);
   if (shader->key.ge.as_ls) {
      if (shader->selector->screen->info.gfx_level >= GFX11) {
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* user VGPR */
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* user VGPR */
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.instance_id);
      } else if (shader->selector->screen->info.gfx_level >= GFX10) {
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.vs_rel_patch_id);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* user VGPR */
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.instance_id);
      } else {
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.vs_rel_patch_id);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.instance_id);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* unused */
      }
   } else if (shader->selector->screen->info.gfx_level >= GFX10) {
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* user VGPR */
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT,
                 /* user vgpr or PrimID (legacy) */
                 shader->key.ge.as_ngg ? NULL : &args->ac.vs_prim_id);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.instance_id);
   } else {
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.instance_id);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.vs_prim_id);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* unused */
   }

   if (!shader->is_gs_copy_shader) {
      /* Vertex load indices. */
      if (shader->selector->info.num_inputs) {
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->vertex_index0);
         for (unsigned i = 1; i < shader->selector->info.num_inputs; i++)
            ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL);
      }
      *num_prolog_vgprs += shader->selector->info.num_inputs;
   }
}

static void declare_vs_blit_inputs(struct si_shader *shader, struct si_shader_args *args)
{
   bool has_attribute_ring_address = shader->selector->screen->info.gfx_level >= GFX11;

   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->vs_blit_inputs); /* i16 x1, y1 */
   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);                  /* i16 x1, y1 */
   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, NULL);                /* depth */

   if (shader->selector->info.base.vs.blit_sgprs_amd ==
       SI_VS_BLIT_SGPRS_POS_COLOR + has_attribute_ring_address) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, NULL); /* color0 */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, NULL); /* color1 */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, NULL); /* color2 */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, NULL); /* color3 */
      if (has_attribute_ring_address)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* attribute ring address */
   } else if (shader->selector->info.base.vs.blit_sgprs_amd ==
              SI_VS_BLIT_SGPRS_POS_TEXCOORD + has_attribute_ring_address) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, NULL); /* texcoord.x1 */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, NULL); /* texcoord.y1 */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, NULL); /* texcoord.x2 */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, NULL); /* texcoord.y2 */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, NULL); /* texcoord.z */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, NULL); /* texcoord.w */
      if (has_attribute_ring_address)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* attribute ring address */
   }
}

static void declare_tes_input_vgprs(struct si_shader_args *args)
{
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.tes_u);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.tes_v);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tes_rel_patch_id);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tes_patch_id);
}

enum
{
   /* Convenient merged shader definitions. */
   SI_SHADER_MERGED_VERTEX_TESSCTRL = MESA_ALL_SHADER_STAGES,
   SI_SHADER_MERGED_VERTEX_OR_TESSEVAL_GEOMETRY,
};

void si_add_arg_checked(struct ac_shader_args *args, enum ac_arg_regfile file, unsigned registers,
                        enum ac_arg_type type, struct ac_arg *arg, unsigned idx)
{
   assert(args->arg_count == idx);
   ac_add_arg(args, file, registers, type, arg);
}

void si_init_shader_args(struct si_shader *shader, struct si_shader_args *args)
{
   unsigned i, num_returns, num_return_sgprs;
   unsigned num_prolog_vgprs = 0;
   struct si_shader_selector *sel = shader->selector;
   unsigned stage = shader->is_gs_copy_shader ? MESA_SHADER_VERTEX : sel->stage;
   unsigned stage_case = stage;

   memset(args, 0, sizeof(*args));

   /* Set MERGED shaders. */
   if (sel->screen->info.gfx_level >= GFX9 && stage <= MESA_SHADER_GEOMETRY) {
      if (shader->key.ge.as_ls || stage == MESA_SHADER_TESS_CTRL)
         stage_case = SI_SHADER_MERGED_VERTEX_TESSCTRL; /* LS or HS */
      else if (shader->key.ge.as_es || shader->key.ge.as_ngg || stage == MESA_SHADER_GEOMETRY)
         stage_case = SI_SHADER_MERGED_VERTEX_OR_TESSEVAL_GEOMETRY;
   }

   switch (stage_case) {
   case MESA_SHADER_VERTEX:
      declare_global_desc_pointers(args);

      if (sel->info.base.vs.blit_sgprs_amd) {
         declare_vs_blit_inputs(shader, args);
      } else {
         declare_per_stage_desc_pointers(args, shader, true);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->vs_state_bits);

         if (shader->is_gs_copy_shader) {
            declare_streamout_params(args, shader);
         } else {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.base_vertex);
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.draw_id);
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.start_instance);
            declare_vb_descriptor_input_sgprs(args, shader);

            if (shader->key.ge.as_es) {
               ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.es2gs_offset);
            } else if (shader->key.ge.as_ls) {
               /* no extra parameters */
            } else {
               declare_streamout_params(args, shader);
            }
         }
      }

      /* GFX11 set FLAT_SCRATCH directly instead of using this arg. */
      if (sel->screen->use_aco && sel->screen->info.gfx_level < GFX11)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);

      /* VGPRs */
      declare_vs_input_vgprs(args, shader, &num_prolog_vgprs);

      break;

   case MESA_SHADER_TESS_CTRL: /* GFX6-GFX8 */
      declare_global_desc_pointers(args);
      declare_per_stage_desc_pointers(args, shader, true);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tcs_offchip_layout);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tes_offchip_addr);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->vs_state_bits);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tcs_factor_offset);

      /* GFX11 set FLAT_SCRATCH directly instead of using this arg. */
      if (sel->screen->use_aco && sel->screen->info.gfx_level < GFX11)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);

      /* VGPRs */
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tcs_patch_id);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tcs_rel_ids);

      /* For monolithic shaders, the TCS epilog code is generated by
       * ac_nir_lower_hs_outputs_to_mem.
       */
      if (!shader->is_monolithic) {
         /* param_tcs_offchip_offset and param_tcs_factor_offset are
          * placed after the user SGPRs.
          */
         for (i = 0; i < GFX6_TCS_NUM_USER_SGPR + 2; i++)
            ac_add_return(&args->ac, AC_ARG_SGPR);
         for (i = 0; i < 11; i++)
            ac_add_return(&args->ac, AC_ARG_VGPR);
      }
      break;

   case SI_SHADER_MERGED_VERTEX_TESSCTRL:
      /* Merged stages have 8 system SGPRs at the beginning. */
      /* Gfx9-10: SPI_SHADER_USER_DATA_ADDR_LO/HI_HS */
      /* Gfx11+:  SPI_SHADER_PGM_LO/HI_HS */
      declare_per_stage_desc_pointers(args, shader, stage == MESA_SHADER_TESS_CTRL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.merged_wave_info);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tcs_factor_offset);
      if (sel->screen->info.gfx_level >= GFX11)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tcs_wave_id);
      else
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* unused */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* unused */

      declare_global_desc_pointers(args);
      declare_per_stage_desc_pointers(args, shader, stage == MESA_SHADER_VERTEX);

      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->vs_state_bits);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.base_vertex);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.draw_id);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.start_instance);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tcs_offchip_layout);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tes_offchip_addr);

      /* VGPRs (first TCS, then VS) */
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tcs_patch_id);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tcs_rel_ids);

      if (stage == MESA_SHADER_VERTEX) {
         declare_vs_input_vgprs(args, shader, &num_prolog_vgprs);

         /* Need to keep LS/HS arg index same for shared args when ACO,
          * so this is not able to be before shared VGPRs.
          */
         declare_vb_descriptor_input_sgprs(args, shader);

         /* LS return values are inputs to the TCS main shader part. */
         if (!shader->is_monolithic || shader->key.ge.opt.same_patch_vertices) {
            for (i = 0; i < 8 + GFX9_TCS_NUM_USER_SGPR; i++)
               ac_add_return(&args->ac, AC_ARG_SGPR);
            for (i = 0; i < 2; i++)
               ac_add_return(&args->ac, AC_ARG_VGPR);

            /* VS outputs passed via VGPRs to TCS. */
            if (shader->key.ge.opt.same_patch_vertices && !sel->screen->use_aco) {
               unsigned num_outputs = util_last_bit64(shader->selector->info.outputs_written_before_tes_gs);
               for (i = 0; i < num_outputs * 4; i++)
                  ac_add_return(&args->ac, AC_ARG_VGPR);
            }
         }
      } else {
         /* TCS inputs are passed via VGPRs from VS. */
         if (shader->key.ge.opt.same_patch_vertices && !sel->screen->use_aco) {
            unsigned num_inputs = util_last_bit64(shader->previous_stage_sel->info.outputs_written_before_tes_gs);
            for (i = 0; i < num_inputs * 4; i++)
               ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, NULL);
         }

         /* For monolithic shaders, the TCS epilog code is generated by
          * ac_nir_lower_hs_outputs_to_mem.
          */
         if (!shader->is_monolithic) {
            /* TCS return values are inputs to the TCS epilog.
             *
             * param_tcs_offchip_offset, param_tcs_factor_offset,
             * param_tcs_offchip_layout, and internal_bindings
             * should be passed to the epilog.
             */
            for (i = 0; i <= 8 + GFX9_SGPR_TCS_OFFCHIP_ADDR; i++)
               ac_add_return(&args->ac, AC_ARG_SGPR);
            for (i = 0; i < 11; i++)
               ac_add_return(&args->ac, AC_ARG_VGPR);
         }
      }
      break;

   case SI_SHADER_MERGED_VERTEX_OR_TESSEVAL_GEOMETRY:
      /* Merged stages have 8 system SGPRs at the beginning. */
      /* Gfx9-10: SPI_SHADER_USER_DATA_ADDR_LO/HI_GS */
      /* Gfx11+:  SPI_SHADER_PGM_LO/HI_GS */
      declare_per_stage_desc_pointers(args, shader, stage == MESA_SHADER_GEOMETRY);

      if (shader->key.ge.as_ngg)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.gs_tg_info);
      else
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.gs2vs_offset);

      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.merged_wave_info);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);
      if (sel->screen->info.gfx_level >= GFX11)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.gs_attr_offset);
      else
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* unused */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* unused */

      declare_global_desc_pointers(args);
      if (stage != MESA_SHADER_VERTEX || !sel->info.base.vs.blit_sgprs_amd) {
         declare_per_stage_desc_pointers(
            args, shader, (stage == MESA_SHADER_VERTEX || stage == MESA_SHADER_TESS_EVAL));
      }

      if (stage == MESA_SHADER_VERTEX && sel->info.base.vs.blit_sgprs_amd) {
         declare_vs_blit_inputs(shader, args);
      } else {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->vs_state_bits);

         if (stage == MESA_SHADER_VERTEX) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.base_vertex);
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.draw_id);
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.start_instance);
         } else if (stage == MESA_SHADER_TESS_EVAL) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tcs_offchip_layout);
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tes_offchip_addr);
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* unused */
         } else {
            /* GS */
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* unused */
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* unused */
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* unused */
         }

         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_CONST_DESC_PTR, &args->small_prim_cull_info);
         if (sel->screen->info.gfx_level >= GFX11)
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->gs_attr_address);
         else
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* unused */
      }

      /* VGPRs (first GS, then VS/TES) */
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[0]);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[1]);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_prim_id);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_invocation_id);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[2]);

      if (stage == MESA_SHADER_VERTEX) {
         declare_vs_input_vgprs(args, shader, &num_prolog_vgprs);

         /* Need to keep ES/GS arg index same for shared args when ACO,
          * so this is not able to be before shared VGPRs.
          */
         if (!sel->info.base.vs.blit_sgprs_amd)
            declare_vb_descriptor_input_sgprs(args, shader);
      } else if (stage == MESA_SHADER_TESS_EVAL) {
         declare_tes_input_vgprs(args);
      }

      if (shader->key.ge.as_es && !shader->is_monolithic &&
          (stage == MESA_SHADER_VERTEX || stage == MESA_SHADER_TESS_EVAL)) {
         /* ES return values are inputs to GS. */
         for (i = 0; i < 8 + GFX9_GS_NUM_USER_SGPR; i++)
            ac_add_return(&args->ac, AC_ARG_SGPR);
         for (i = 0; i < 5; i++)
            ac_add_return(&args->ac, AC_ARG_VGPR);
      }
      break;

   case MESA_SHADER_TESS_EVAL:
      declare_global_desc_pointers(args);
      declare_per_stage_desc_pointers(args, shader, true);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->vs_state_bits);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tcs_offchip_layout);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tes_offchip_addr);

      if (shader->key.ge.as_es) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.es2gs_offset);
      } else {
         declare_streamout_params(args, shader);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);
      }

      /* GFX11 set FLAT_SCRATCH directly instead of using this arg. */
      if (sel->screen->use_aco && sel->screen->info.gfx_level < GFX11)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);

      /* VGPRs */
      declare_tes_input_vgprs(args);
      break;

   case MESA_SHADER_GEOMETRY:
      declare_global_desc_pointers(args);
      declare_per_stage_desc_pointers(args, shader, true);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.gs2vs_offset);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.gs_wave_id);

      /* GFX11 set FLAT_SCRATCH directly instead of using this arg. */
      if (sel->screen->use_aco && sel->screen->info.gfx_level < GFX11)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);

      /* VGPRs */
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[0]);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[1]);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_prim_id);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[2]);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[3]);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[4]);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[5]);
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_invocation_id);
      break;

   case MESA_SHADER_FRAGMENT:
      declare_global_desc_pointers(args);
      declare_per_stage_desc_pointers(args, shader, true);
      si_add_arg_checked(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->alpha_reference,
                         SI_PARAM_ALPHA_REF);
      si_add_arg_checked(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.prim_mask,
                         SI_PARAM_PRIM_MASK);

      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.persp_sample,
                         SI_PARAM_PERSP_SAMPLE);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.persp_center,
                         SI_PARAM_PERSP_CENTER);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.persp_centroid,
                         SI_PARAM_PERSP_CENTROID);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 3, AC_ARG_INT, NULL, SI_PARAM_PERSP_PULL_MODEL);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.linear_sample,
                         SI_PARAM_LINEAR_SAMPLE);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.linear_center,
                         SI_PARAM_LINEAR_CENTER);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.linear_centroid,
                         SI_PARAM_LINEAR_CENTROID);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, NULL, SI_PARAM_LINE_STIPPLE_TEX);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.frag_pos[0],
                         SI_PARAM_POS_X_FLOAT);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.frag_pos[1],
                         SI_PARAM_POS_Y_FLOAT);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.frag_pos[2],
                         SI_PARAM_POS_Z_FLOAT);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.frag_pos[3],
                         SI_PARAM_POS_W_FLOAT);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.front_face,
                         SI_PARAM_FRONT_FACE);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.ancillary,
                         SI_PARAM_ANCILLARY);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.sample_coverage,
                         SI_PARAM_SAMPLE_COVERAGE);
      si_add_arg_checked(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.pos_fixed_pt,
                         SI_PARAM_POS_FIXED_PT);

      if (sel->screen->use_aco) {
         ac_compact_ps_vgpr_args(&args->ac, shader->config.spi_ps_input_addr);

         /* GFX11 set FLAT_SCRATCH directly instead of using this arg. */
         if (sel->screen->info.gfx_level < GFX11)
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);
      }

      /* Monolithic PS emit prolog and epilog in NIR directly. */
      if (!shader->is_monolithic) {
         /* Color inputs from the prolog. */
         if (shader->selector->info.colors_read) {
            unsigned num_color_elements = util_bitcount(shader->selector->info.colors_read);

            for (i = 0; i < num_color_elements; i++)
               ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, i ? NULL : &args->color_start);

            num_prolog_vgprs += num_color_elements;
         }

         /* Outputs for the epilog. */
         num_return_sgprs = SI_SGPR_ALPHA_REF + 1;
         num_returns =
            num_return_sgprs + util_bitcount(shader->selector->info.colors_written) * 4 +
            shader->selector->info.writes_z + shader->selector->info.writes_stencil +
            shader->ps.writes_samplemask + 1 /* SampleMaskIn */;

         for (i = 0; i < num_return_sgprs; i++)
            ac_add_return(&args->ac, AC_ARG_SGPR);
         for (; i < num_returns; i++)
            ac_add_return(&args->ac, AC_ARG_VGPR);
      }
      break;

   case MESA_SHADER_COMPUTE:
      declare_global_desc_pointers(args);
      declare_per_stage_desc_pointers(args, shader, true);
      if (shader->selector->info.uses_grid_size)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 3, AC_ARG_INT, &args->ac.num_work_groups);
      if (shader->selector->info.uses_variable_block_size)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->block_size);

      unsigned cs_user_data_dwords =
         shader->selector->info.base.cs.user_data_components_amd;
      if (cs_user_data_dwords) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, cs_user_data_dwords, AC_ARG_INT, &args->cs_user_data);
      }

      /* Some descriptors can be in user SGPRs. */
      /* Shader buffers in user SGPRs. */
      for (unsigned i = 0; i < shader->selector->cs_num_shaderbufs_in_user_sgprs; i++) {
         while (args->ac.num_sgprs_used % 4 != 0)
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);

         ac_add_arg(&args->ac, AC_ARG_SGPR, 4, AC_ARG_INT, &args->cs_shaderbuf[i]);
      }
      /* Images in user SGPRs. */
      for (unsigned i = 0; i < shader->selector->cs_num_images_in_user_sgprs; i++) {
         unsigned num_sgprs = BITSET_TEST(shader->selector->info.base.image_buffers, i) ? 4 : 8;

         while (args->ac.num_sgprs_used % num_sgprs != 0)
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);

         ac_add_arg(&args->ac, AC_ARG_SGPR, num_sgprs, AC_ARG_INT, &args->cs_image[i]);
      }

      /* Hardware SGPRs. */
      for (i = 0; i < 3; i++) {
         if (shader->selector->info.uses_block_id[i]) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.workgroup_ids[i]);
         }
      }
      if (shader->selector->info.uses_tg_size)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tg_size);

      /* GFX11 set FLAT_SCRATCH directly instead of using this arg. */
      if (sel->screen->use_aco && sel->screen->info.gfx_level < GFX11)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);

      /* Hardware VGPRs. */
      /* Thread IDs are packed in VGPR0, 10 bits per component or stored in 3 separate VGPRs */
      if (sel->screen->info.gfx_level >= GFX11 ||
          (!sel->screen->info.has_graphics && sel->screen->info.family >= CHIP_MI200))
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.local_invocation_ids);
      else
         ac_add_arg(&args->ac, AC_ARG_VGPR, 3, AC_ARG_INT, &args->ac.local_invocation_ids);
      break;
   default:
      assert(0 && "unimplemented shader");
      return;
   }

   shader->info.num_input_sgprs = args->ac.num_sgprs_used;
   shader->info.num_input_vgprs = args->ac.num_vgprs_used;

   assert(shader->info.num_input_vgprs >= num_prolog_vgprs);
   shader->info.num_input_vgprs -= num_prolog_vgprs;
}

static unsigned get_lds_granularity(struct si_screen *screen, gl_shader_stage stage)
{
   return screen->info.gfx_level >= GFX11 && stage == MESA_SHADER_FRAGMENT ? 1024 :
          screen->info.gfx_level >= GFX7 ? 512 : 256;
}

bool si_shader_binary_open(struct si_screen *screen, struct si_shader *shader,
                           struct ac_rtld_binary *rtld)
{
   const struct si_shader_selector *sel = shader->selector;
   const char *part_elfs[5];
   size_t part_sizes[5];
   unsigned num_parts = 0;

#define add_part(shader_or_part)                                                                   \
   if (shader_or_part) {                                                                           \
      part_elfs[num_parts] = (shader_or_part)->binary.code_buffer;                                 \
      part_sizes[num_parts] = (shader_or_part)->binary.code_size;                                  \
      num_parts++;                                                                                 \
   }

   add_part(shader->prolog);
   add_part(shader->previous_stage);
   add_part(shader);
   add_part(shader->epilog);

#undef add_part

   struct ac_rtld_symbol lds_symbols[2];
   unsigned num_lds_symbols = 0;

   if (sel && screen->info.gfx_level >= GFX9 && !shader->is_gs_copy_shader &&
       (sel->stage == MESA_SHADER_GEOMETRY ||
        (sel->stage <= MESA_SHADER_GEOMETRY && shader->key.ge.as_ngg))) {
      struct ac_rtld_symbol *sym = &lds_symbols[num_lds_symbols++];
      sym->name = "esgs_ring";
      sym->size = shader->gs_info.esgs_ring_size * 4;
      sym->align = 64 * 1024;
   }

   if (sel->stage == MESA_SHADER_GEOMETRY && shader->key.ge.as_ngg) {
      struct ac_rtld_symbol *sym = &lds_symbols[num_lds_symbols++];
      sym->name = "ngg_emit";
      sym->size = shader->ngg.ngg_emit_size * 4;
      sym->align = 4;
   }

   bool ok = ac_rtld_open(
      rtld, (struct ac_rtld_open_info){.info = &screen->info,
                                       .options =
                                          {
                                             .halt_at_entry = screen->options.halt_shaders,
                                          },
                                       .shader_type = sel->stage,
                                       .wave_size = shader->wave_size,
                                       .num_parts = num_parts,
                                       .elf_ptrs = part_elfs,
                                       .elf_sizes = part_sizes,
                                       .num_shared_lds_symbols = num_lds_symbols,
                                       .shared_lds_symbols = lds_symbols});

   if (rtld->lds_size > 0) {
      unsigned alloc_granularity = get_lds_granularity(screen, sel->stage);
      shader->config.lds_size = DIV_ROUND_UP(rtld->lds_size, alloc_granularity);
   }

   return ok;
}

static unsigned get_shader_binaries(struct si_shader *shader, struct si_shader_binary *bin[4])
{
   unsigned num_bin = 0;

   if (shader->prolog)
      bin[num_bin++] = &shader->prolog->binary;

   if (shader->previous_stage)
      bin[num_bin++] = &shader->previous_stage->binary;

   bin[num_bin++] = &shader->binary;

   if (shader->epilog)
      bin[num_bin++] = &shader->epilog->binary;

   return num_bin;
}

static unsigned si_get_shader_binary_size(struct si_screen *screen, struct si_shader *shader)
{
   if (shader->binary.type == SI_SHADER_BINARY_ELF) {
      struct ac_rtld_binary rtld;
      si_shader_binary_open(screen, shader, &rtld);
      uint64_t size = rtld.exec_size;
      ac_rtld_close(&rtld);
      return size;
   } else {
      struct si_shader_binary *bin[4];
      unsigned num_bin = get_shader_binaries(shader, bin);

      unsigned size = 0;
      for (unsigned i = 0; i < num_bin; i++) {
         assert(bin[i]->type == SI_SHADER_BINARY_RAW);
         size += bin[i]->exec_size;
      }
      return size;
   }
}

bool si_get_external_symbol(enum amd_gfx_level gfx_level, void *data, const char *name,
                            uint64_t *value)
{
   uint64_t *scratch_va = data;

   if (!strcmp(scratch_rsrc_dword0_symbol, name)) {
      *value = (uint32_t)*scratch_va;
      return true;
   }
   if (!strcmp(scratch_rsrc_dword1_symbol, name)) {
      /* Enable scratch coalescing. */
      *value = S_008F04_BASE_ADDRESS_HI(*scratch_va >> 32);

      if (gfx_level >= GFX11)
         *value |= S_008F04_SWIZZLE_ENABLE_GFX11(1);
      else
         *value |= S_008F04_SWIZZLE_ENABLE_GFX6(1);
      return true;
   }

   return false;
}

static void *pre_upload_binary(struct si_screen *sscreen, struct si_shader *shader,
                               unsigned binary_size, bool dma_upload,
                               struct si_context **upload_ctx,
                               struct pipe_resource **staging,
                               unsigned *staging_offset)
{
   unsigned aligned_size = ac_align_shader_binary_for_prefetch(&sscreen->info, binary_size);

   si_resource_reference(&shader->bo, NULL);
   shader->bo = si_aligned_buffer_create(
      &sscreen->b,
      SI_RESOURCE_FLAG_DRIVER_INTERNAL | SI_RESOURCE_FLAG_32BIT |
      (dma_upload || sscreen->info.cpdma_prefetch_writes_memory ? 0 : SI_RESOURCE_FLAG_READ_ONLY) |
      (dma_upload ? PIPE_RESOURCE_FLAG_UNMAPPABLE : 0),
      PIPE_USAGE_IMMUTABLE, align(aligned_size, SI_CPDMA_ALIGNMENT), 256);
   if (!shader->bo)
      return NULL;

   shader->gpu_address = shader->bo->gpu_address;

   if (dma_upload) {
      /* First upload into a staging buffer. */
      *upload_ctx = si_get_aux_context(&sscreen->aux_context.shader_upload);

      void *ret;
      u_upload_alloc((*upload_ctx)->b.stream_uploader, 0, binary_size, 256,
                     staging_offset, staging, &ret);
      if (!ret)
         si_put_aux_context_flush(&sscreen->aux_context.shader_upload);

      return ret;
   } else {
      return sscreen->ws->buffer_map(sscreen->ws,
         shader->bo->buf, NULL,
         PIPE_MAP_READ_WRITE | PIPE_MAP_UNSYNCHRONIZED | RADEON_MAP_TEMPORARY);
   }
}

static void post_upload_binary(struct si_screen *sscreen, struct si_shader *shader,
                               void *code, unsigned code_size,
                               unsigned binary_size, bool dma_upload,
                               struct si_context *upload_ctx,
                               struct pipe_resource *staging,
                               unsigned staging_offset)
{
   if (sscreen->debug_flags & DBG(SQTT)) {
      /* Remember the uploaded code */
      shader->binary.uploaded_code_size = code_size;
      shader->binary.uploaded_code = malloc(code_size);
      memcpy(shader->binary.uploaded_code, code, code_size);
   }

   if (dma_upload) {
      /* Then copy from the staging buffer to VRAM.
       *
       * We can't use the upload copy in si_buffer_transfer_unmap because that might use
       * a compute shader, and we can't use shaders in the code that is responsible for making
       * them available.
       */
      si_cp_dma_copy_buffer(upload_ctx, &shader->bo->b.b, staging, 0, staging_offset,
                            binary_size, SI_OP_SYNC_AFTER, SI_COHERENCY_SHADER,
                            sscreen->info.gfx_level >= GFX7 ? L2_LRU : L2_BYPASS);
      upload_ctx->flags |= SI_CONTEXT_INV_ICACHE | SI_CONTEXT_INV_L2;

#if 0 /* debug: validate whether the copy was successful */
      uint32_t *dst_binary = malloc(binary_size);
      uint32_t *src_binary = (uint32_t*)code;
      pipe_buffer_read(&upload_ctx->b, &shader->bo->b.b, 0, binary_size, dst_binary);
      puts("dst_binary == src_binary:");
      for (unsigned i = 0; i < binary_size / 4; i++) {
         printf("   %08x == %08x\n", dst_binary[i], src_binary[i]);
      }
      free(dst_binary);
      exit(0);
#endif

      si_put_aux_context_flush(&sscreen->aux_context.shader_upload);
      pipe_resource_reference(&staging, NULL);
   } else {
      sscreen->ws->buffer_unmap(sscreen->ws, shader->bo->buf);
   }
}

static bool upload_binary_elf(struct si_screen *sscreen, struct si_shader *shader,
                              uint64_t scratch_va, bool dma_upload)
{
   struct ac_rtld_binary binary;
   if (!si_shader_binary_open(sscreen, shader, &binary))
      return false;

   struct si_context *upload_ctx = NULL;
   struct pipe_resource *staging = NULL;
   unsigned staging_offset = 0;

   void *rx_ptr = pre_upload_binary(sscreen, shader, binary.rx_size, dma_upload,
                                    &upload_ctx, &staging, &staging_offset);
   if (!rx_ptr)
      return false;

   /* Upload. */
   struct ac_rtld_upload_info u = {};
   u.binary = &binary;
   u.get_external_symbol = si_get_external_symbol;
   u.cb_data = &scratch_va;
   u.rx_va = shader->bo->gpu_address;
   u.rx_ptr = rx_ptr;

   int size = ac_rtld_upload(&u);

   post_upload_binary(sscreen, shader, rx_ptr, size, binary.rx_size, dma_upload,
                      upload_ctx, staging, staging_offset);

   ac_rtld_close(&binary);

   return size >= 0;
}

static void calculate_needed_lds_size(struct si_screen *sscreen, struct si_shader *shader)
{
   gl_shader_stage stage =
      shader->is_gs_copy_shader ? MESA_SHADER_VERTEX : shader->selector->stage;

   if (sscreen->info.gfx_level >= GFX9 && stage <= MESA_SHADER_GEOMETRY &&
       (stage == MESA_SHADER_GEOMETRY || shader->key.ge.as_ngg)) {
      unsigned size_in_dw = shader->gs_info.esgs_ring_size;

      if (stage == MESA_SHADER_GEOMETRY && shader->key.ge.as_ngg)
         size_in_dw += shader->ngg.ngg_emit_size;

      if (shader->key.ge.as_ngg) {
         unsigned scratch_dw_size = gfx10_ngg_get_scratch_dw_size(shader);
         if (scratch_dw_size) {
            /* scratch base address needs to be 8 byte aligned */
            size_in_dw = ALIGN(size_in_dw, 2);
            size_in_dw += scratch_dw_size;
         }
      }

      shader->config.lds_size =
         DIV_ROUND_UP(size_in_dw * 4, get_lds_granularity(sscreen, stage));
   }
}

static bool upload_binary_raw(struct si_screen *sscreen, struct si_shader *shader,
                              uint64_t scratch_va, bool dma_upload)
{
   struct si_shader_binary *bin[4];
   unsigned num_bin = get_shader_binaries(shader, bin);

   unsigned code_size = 0, exec_size = 0;
   for (unsigned i = 0; i < num_bin; i++) {
      assert(bin[i]->type == SI_SHADER_BINARY_RAW);
      code_size += bin[i]->code_size;
      exec_size += bin[i]->exec_size;
   }

   struct si_context *upload_ctx = NULL;
   struct pipe_resource *staging = NULL;
   unsigned staging_offset = 0;

   void *rx_ptr = pre_upload_binary(sscreen, shader, code_size, dma_upload,
                                    &upload_ctx, &staging, &staging_offset);
   if (!rx_ptr)
      return false;

   unsigned exec_offset = 0, data_offset = exec_size;
   for (unsigned i = 0; i < num_bin; i++) {
      memcpy(rx_ptr + exec_offset, bin[i]->code_buffer, bin[i]->exec_size);

      if (bin[i]->num_symbols) {
         /* Offset needed to add to const data symbol because of inserting other
          * shader part between exec code and const data.
          */
         unsigned const_offset = data_offset - exec_offset - bin[i]->exec_size;

         /* Prolog and epilog have no symbols. */
         struct si_shader *sh = bin[i] == &shader->binary ? shader : shader->previous_stage;
         assert(sh && bin[i] == &sh->binary);

         si_aco_resolve_symbols(sh, rx_ptr + exec_offset, (const uint32_t *)bin[i]->code_buffer,
                                scratch_va, const_offset);
      }

      exec_offset += bin[i]->exec_size;

      unsigned data_size = bin[i]->code_size - bin[i]->exec_size;
      if (data_size) {
         memcpy(rx_ptr + data_offset, bin[i]->code_buffer + bin[i]->exec_size, data_size);
         data_offset += data_size;
      }
   }

   post_upload_binary(sscreen, shader, rx_ptr, code_size, code_size, dma_upload,
                      upload_ctx, staging, staging_offset);

   calculate_needed_lds_size(sscreen, shader);
   return true;
}

bool si_shader_binary_upload(struct si_screen *sscreen, struct si_shader *shader,
                             uint64_t scratch_va)
{
   bool dma_upload = !(sscreen->debug_flags & DBG(NO_DMA_SHADERS)) &&
                     sscreen->info.has_dedicated_vram;

   if (shader->binary.type == SI_SHADER_BINARY_ELF) {
      return upload_binary_elf(sscreen, shader, scratch_va, dma_upload);
   } else {
      assert(shader->binary.type == SI_SHADER_BINARY_RAW);
      return upload_binary_raw(sscreen, shader, scratch_va, dma_upload);
   }
}

static void print_disassembly(const char *disasm, size_t nbytes,
                              const char *name, FILE *file,
                              struct util_debug_callback *debug)
{
   if (debug && debug->debug_message) {
      /* Very long debug messages are cut off, so send the
       * disassembly one line at a time. This causes more
       * overhead, but on the plus side it simplifies
       * parsing of resulting logs.
       */
      util_debug_message(debug, SHADER_INFO, "Shader Disassembly Begin");

      uint64_t line = 0;
      while (line < nbytes) {
         int count = nbytes - line;
         const char *nl = memchr(disasm + line, '\n', nbytes - line);
         if (nl)
            count = nl - (disasm + line);

         if (count) {
            util_debug_message(debug, SHADER_INFO, "%.*s", count, disasm + line);
         }

         line += count + 1;
      }

      util_debug_message(debug, SHADER_INFO, "Shader Disassembly End");
   }

   if (file) {
      fprintf(file, "Shader %s disassembly:\n", name);
      fprintf(file, "%*s", (int)nbytes, disasm);
   }
}

static void si_shader_dump_disassembly(struct si_screen *screen,
                                       const struct si_shader_binary *binary,
                                       gl_shader_stage stage, unsigned wave_size,
                                       struct util_debug_callback *debug, const char *name,
                                       FILE *file)
{
   if (binary->type == SI_SHADER_BINARY_RAW) {
      print_disassembly(binary->disasm_string, binary->disasm_size, name, file, debug);
      return;
   }

   struct ac_rtld_binary rtld_binary;

   if (!ac_rtld_open(&rtld_binary, (struct ac_rtld_open_info){
                                      .info = &screen->info,
                                      .shader_type = stage,
                                      .wave_size = wave_size,
                                      .num_parts = 1,
                                      .elf_ptrs = &binary->code_buffer,
                                      .elf_sizes = &binary->code_size}))
      return;

   const char *disasm;
   size_t nbytes;

   if (!ac_rtld_get_section_by_name(&rtld_binary, ".AMDGPU.disasm", &disasm, &nbytes))
      goto out;

   if (nbytes > INT_MAX)
      goto out;

   print_disassembly(disasm, nbytes, name, file, debug);

out:
   ac_rtld_close(&rtld_binary);
}

static void si_calculate_max_simd_waves(struct si_shader *shader)
{
   struct si_screen *sscreen = shader->selector->screen;
   struct ac_shader_config *conf = &shader->config;
   unsigned lds_increment = get_lds_granularity(sscreen, shader->selector->stage);
   unsigned lds_per_wave = 0;
   unsigned max_simd_waves;

   max_simd_waves = sscreen->info.max_waves_per_simd;

   /* Compute LDS usage for PS. */
   switch (shader->selector->stage) {
   case MESA_SHADER_FRAGMENT:
      /* The minimum usage per wave is (num_inputs * 48). The maximum
       * usage is (num_inputs * 48 * 16).
       * We can get anything in between and it varies between waves.
       *
       * The 48 bytes per input for a single primitive is equal to
       * 4 bytes/component * 4 components/input * 3 points.
       *
       * Other stages don't know the size at compile time or don't
       * allocate LDS per wave, but instead they do it per thread group.
       */
      lds_per_wave = conf->lds_size * lds_increment +
                     align(shader->info.num_ps_inputs * 48, lds_increment);
      break;
   case MESA_SHADER_COMPUTE: {
         unsigned max_workgroup_size = si_get_max_workgroup_size(shader);
         lds_per_wave = (conf->lds_size * lds_increment) /
                        DIV_ROUND_UP(max_workgroup_size, shader->wave_size);
      }
      break;
   default:;
   }

   /* Compute the per-SIMD wave counts. */
   if (conf->num_sgprs) {
      max_simd_waves =
         MIN2(max_simd_waves, sscreen->info.num_physical_sgprs_per_simd / conf->num_sgprs);
   }

   if (conf->num_vgprs) {
      /* GFX 10.3 internally:
       * - aligns VGPRS to 16 for Wave32 and 8 for Wave64
       * - aligns LDS to 1024
       *
       * For shader-db stats, set num_vgprs that the hw actually uses.
       */
      unsigned num_vgprs = conf->num_vgprs;
      if (sscreen->info.gfx_level >= GFX10_3) {
         unsigned real_vgpr_gran = sscreen->info.num_physical_wave64_vgprs_per_simd / 64;
         num_vgprs = util_align_npot(num_vgprs, real_vgpr_gran * (shader->wave_size == 32 ? 2 : 1));
      } else {
         num_vgprs = align(num_vgprs, shader->wave_size == 32 ? 8 : 4);
      }

      /* Always print wave limits as Wave64, so that we can compare
       * Wave32 and Wave64 with shader-db fairly. */
      unsigned max_vgprs = sscreen->info.num_physical_wave64_vgprs_per_simd;
      max_simd_waves = MIN2(max_simd_waves, max_vgprs / num_vgprs);
   }

   unsigned max_lds_per_simd = sscreen->info.lds_size_per_workgroup / 4;
   if (lds_per_wave)
      max_simd_waves = MIN2(max_simd_waves, max_lds_per_simd / lds_per_wave);

   shader->info.max_simd_waves = max_simd_waves;
}

void si_shader_dump_stats_for_shader_db(struct si_screen *screen, struct si_shader *shader,
                                        struct util_debug_callback *debug)
{
   const struct ac_shader_config *conf = &shader->config;
   static const char *stages[] = {"VS", "TCS", "TES", "GS", "PS", "CS"};

   if (screen->options.debug_disassembly)
      si_shader_dump_disassembly(screen, &shader->binary, shader->selector->stage,
                                 shader->wave_size, debug, "main", NULL);

   unsigned num_outputs = 0;

   if (shader->selector->stage <= MESA_SHADER_GEOMETRY) {
      /* This doesn't include pos exports because only param exports are interesting
       * for performance and can be optimized.
       */
      if (shader->gs_copy_shader)
         num_outputs = shader->gs_copy_shader->info.nr_param_exports;
      else if (shader->key.ge.as_es)
         num_outputs = shader->selector->info.esgs_vertex_stride / 16;
      else if (shader->key.ge.as_ls)
         num_outputs = shader->selector->info.lshs_vertex_stride / 16;
      else if (shader->selector->stage == MESA_SHADER_VERTEX ||
               shader->selector->stage == MESA_SHADER_TESS_EVAL ||
               shader->key.ge.as_ngg)
         num_outputs = shader->info.nr_param_exports;
      else if (shader->selector->stage == MESA_SHADER_TESS_CTRL)
         num_outputs = util_last_bit64(shader->selector->info.outputs_written_before_tes_gs);
      else
         unreachable("invalid shader key");
   } else if (shader->selector->stage == MESA_SHADER_FRAGMENT) {
      num_outputs = util_bitcount(shader->selector->info.colors_written) +
                    (shader->selector->info.writes_z ||
                     shader->selector->info.writes_stencil ||
                     shader->ps.writes_samplemask);
   }

   util_debug_message(debug, SHADER_INFO,
                      "Shader Stats: SGPRS: %d VGPRS: %d Code Size: %d "
                      "LDS: %d Scratch: %d Max Waves: %d Spilled SGPRs: %d "
                      "Spilled VGPRs: %d PrivMem VGPRs: %d Outputs: %u PatchOutputs: %u DivergentLoop: %d "
                      "InlineUniforms: %d (%s, W%u)",
                      conf->num_sgprs, conf->num_vgprs, si_get_shader_binary_size(screen, shader),
                      conf->lds_size, conf->scratch_bytes_per_wave, shader->info.max_simd_waves,
                      conf->spilled_sgprs, conf->spilled_vgprs, shader->info.private_mem_vgprs,
                      num_outputs,
                      util_last_bit64(shader->selector->info.patch_outputs_written),
                      shader->selector->info.has_divergent_loop,
                      shader->selector->info.base.num_inlinable_uniforms,
                      stages[shader->selector->stage], shader->wave_size);
}

bool si_can_dump_shader(struct si_screen *sscreen, gl_shader_stage stage,
                        enum si_shader_dump_type dump_type)
{
   static uint64_t filter[] = {
      [SI_DUMP_SHADER_KEY] = DBG(NIR) | DBG(INIT_LLVM) | DBG(LLVM) | DBG(INIT_ACO) | DBG(ACO) | DBG(ASM),
      [SI_DUMP_INIT_NIR] = DBG(INIT_NIR),
      [SI_DUMP_NIR] = DBG(NIR),
      [SI_DUMP_INIT_LLVM_IR] = DBG(INIT_LLVM),
      [SI_DUMP_LLVM_IR] = DBG(LLVM),
      [SI_DUMP_INIT_ACO_IR] = DBG(INIT_ACO),
      [SI_DUMP_ACO_IR] = DBG(ACO),
      [SI_DUMP_ASM] = DBG(ASM),
      [SI_DUMP_STATS] = DBG(STATS),
      [SI_DUMP_ALWAYS] = DBG(VS) | DBG(TCS) | DBG(TES) | DBG(GS) | DBG(PS) | DBG(CS),
   };
   assert(dump_type < ARRAY_SIZE(filter));

   return sscreen->debug_flags & (1 << stage) &&
          sscreen->debug_flags & filter[dump_type];
}

static void si_shader_dump_stats(struct si_screen *sscreen, struct si_shader *shader, FILE *file,
                                 bool check_debug_option)
{
   const struct ac_shader_config *conf = &shader->config;

   if (shader->selector->stage == MESA_SHADER_FRAGMENT) {
      fprintf(file,
              "*** SHADER CONFIG ***\n"
              "SPI_PS_INPUT_ADDR = 0x%04x\n"
              "SPI_PS_INPUT_ENA  = 0x%04x\n",
              conf->spi_ps_input_addr, conf->spi_ps_input_ena);
   }

   fprintf(file,
           "*** SHADER STATS ***\n"
           "SGPRS: %d\n"
           "VGPRS: %d\n"
           "Spilled SGPRs: %d\n"
           "Spilled VGPRs: %d\n"
           "Private memory VGPRs: %d\n"
           "Code Size: %d bytes\n"
           "LDS: %d bytes\n"
           "Scratch: %d bytes per wave\n"
           "Max Waves: %d\n"
           "********************\n\n\n",
           conf->num_sgprs, conf->num_vgprs, conf->spilled_sgprs, conf->spilled_vgprs,
           shader->info.private_mem_vgprs, si_get_shader_binary_size(sscreen, shader),
           conf->lds_size * get_lds_granularity(sscreen, shader->selector->stage),
           conf->scratch_bytes_per_wave, shader->info.max_simd_waves);
}

const char *si_get_shader_name(const struct si_shader *shader)
{
   switch (shader->selector->stage) {
   case MESA_SHADER_VERTEX:
      if (shader->key.ge.as_es)
         return "Vertex Shader as ES";
      else if (shader->key.ge.as_ls)
         return "Vertex Shader as LS";
      else if (shader->key.ge.as_ngg)
         return "Vertex Shader as ESGS";
      else
         return "Vertex Shader as VS";
   case MESA_SHADER_TESS_CTRL:
      return "Tessellation Control Shader";
   case MESA_SHADER_TESS_EVAL:
      if (shader->key.ge.as_es)
         return "Tessellation Evaluation Shader as ES";
      else if (shader->key.ge.as_ngg)
         return "Tessellation Evaluation Shader as ESGS";
      else
         return "Tessellation Evaluation Shader as VS";
   case MESA_SHADER_GEOMETRY:
      if (shader->is_gs_copy_shader)
         return "GS Copy Shader as VS";
      else
         return "Geometry Shader";
   case MESA_SHADER_FRAGMENT:
      return "Pixel Shader";
   case MESA_SHADER_COMPUTE:
      return "Compute Shader";
   default:
      return "Unknown Shader";
   }
}

void si_shader_dump(struct si_screen *sscreen, struct si_shader *shader,
                    struct util_debug_callback *debug, FILE *file, bool check_debug_option)
{
   gl_shader_stage stage = shader->selector->stage;

   if (!check_debug_option || si_can_dump_shader(sscreen, stage, SI_DUMP_SHADER_KEY))
      si_dump_shader_key(shader, file);

   if (!check_debug_option && shader->binary.llvm_ir_string) {
      /* This is only used with ddebug. */
      if (shader->previous_stage && shader->previous_stage->binary.llvm_ir_string) {
         fprintf(file, "\n%s - previous stage - LLVM IR:\n\n", si_get_shader_name(shader));
         fprintf(file, "%s\n", shader->previous_stage->binary.llvm_ir_string);
      }

      fprintf(file, "\n%s - main shader part - LLVM IR:\n\n", si_get_shader_name(shader));
      fprintf(file, "%s\n", shader->binary.llvm_ir_string);
   }

   if (!check_debug_option || (si_can_dump_shader(sscreen, stage, SI_DUMP_ASM))) {
      fprintf(file, "\n%s:\n", si_get_shader_name(shader));

      if (shader->prolog)
         si_shader_dump_disassembly(sscreen, &shader->prolog->binary, stage, shader->wave_size, debug,
                                    "prolog", file);
      if (shader->previous_stage)
         si_shader_dump_disassembly(sscreen, &shader->previous_stage->binary, stage,
                                    shader->wave_size, debug, "previous stage", file);
      si_shader_dump_disassembly(sscreen, &shader->binary, stage, shader->wave_size, debug, "main",
                                 file);

      if (shader->epilog)
         si_shader_dump_disassembly(sscreen, &shader->epilog->binary, stage, shader->wave_size, debug,
                                    "epilog", file);
      fprintf(file, "\n");

      si_shader_dump_stats(sscreen, shader, file, check_debug_option);
   }
}

static void si_dump_shader_key_vs(const union si_shader_key *key,
                                  const struct si_vs_prolog_bits *prolog, const char *prefix,
                                  FILE *f)
{
   fprintf(f, "  %s.instance_divisor_is_one = %u\n", prefix, prolog->instance_divisor_is_one);
   fprintf(f, "  %s.instance_divisor_is_fetched = %u\n", prefix,
           prolog->instance_divisor_is_fetched);
   fprintf(f, "  %s.ls_vgpr_fix = %u\n", prefix, prolog->ls_vgpr_fix);

   fprintf(f, "  mono.vs.fetch_opencode = %x\n", key->ge.mono.vs_fetch_opencode);
   fprintf(f, "  mono.vs.fix_fetch = {");
   for (int i = 0; i < SI_MAX_ATTRIBS; i++) {
      union si_vs_fix_fetch fix = key->ge.mono.vs_fix_fetch[i];
      if (i)
         fprintf(f, ", ");
      if (!fix.bits)
         fprintf(f, "0");
      else
         fprintf(f, "%u.%u.%u.%u", fix.u.reverse, fix.u.log_size, fix.u.num_channels_m1,
                 fix.u.format);
   }
   fprintf(f, "}\n");
}

static void si_dump_shader_key(const struct si_shader *shader, FILE *f)
{
   const union si_shader_key *key = &shader->key;
   gl_shader_stage stage = shader->selector->stage;

   fprintf(f, "SHADER KEY\n");
   fprintf(f, "  source_sha1 = {");
   _mesa_sha1_print(f, shader->selector->info.base.source_sha1);
   fprintf(f, "}\n");

   switch (stage) {
   case MESA_SHADER_VERTEX:
      si_dump_shader_key_vs(key, &key->ge.part.vs.prolog, "part.vs.prolog", f);
      fprintf(f, "  as_es = %u\n", key->ge.as_es);
      fprintf(f, "  as_ls = %u\n", key->ge.as_ls);
      fprintf(f, "  as_ngg = %u\n", key->ge.as_ngg);
      fprintf(f, "  mono.u.vs_export_prim_id = %u\n", key->ge.mono.u.vs_export_prim_id);
      break;

   case MESA_SHADER_TESS_CTRL:
      if (shader->selector->screen->info.gfx_level >= GFX9) {
         si_dump_shader_key_vs(key, &key->ge.part.tcs.ls_prolog, "part.tcs.ls_prolog", f);
      }
      fprintf(f, "  part.tcs.epilog.prim_mode = %u\n", key->ge.part.tcs.epilog.prim_mode);
      fprintf(f, "  opt.prefer_mono = %u\n", key->ge.opt.prefer_mono);
      fprintf(f, "  opt.same_patch_vertices = %u\n", key->ge.opt.same_patch_vertices);
      break;

   case MESA_SHADER_TESS_EVAL:
      fprintf(f, "  as_es = %u\n", key->ge.as_es);
      fprintf(f, "  as_ngg = %u\n", key->ge.as_ngg);
      fprintf(f, "  mono.u.vs_export_prim_id = %u\n", key->ge.mono.u.vs_export_prim_id);
      break;

   case MESA_SHADER_GEOMETRY:
      if (shader->is_gs_copy_shader)
         break;

      if (shader->selector->screen->info.gfx_level >= GFX9 &&
          key->ge.part.gs.es->stage == MESA_SHADER_VERTEX) {
         si_dump_shader_key_vs(key, &key->ge.part.gs.vs_prolog, "part.gs.vs_prolog", f);
      }
      fprintf(f, "  mono.u.gs_tri_strip_adj_fix = %u\n", key->ge.mono.u.gs_tri_strip_adj_fix);
      fprintf(f, "  as_ngg = %u\n", key->ge.as_ngg);
      break;

   case MESA_SHADER_COMPUTE:
      break;

   case MESA_SHADER_FRAGMENT:
      fprintf(f, "  prolog.color_two_side = %u\n", key->ps.part.prolog.color_two_side);
      fprintf(f, "  prolog.flatshade_colors = %u\n", key->ps.part.prolog.flatshade_colors);
      fprintf(f, "  prolog.poly_stipple = %u\n", key->ps.part.prolog.poly_stipple);
      fprintf(f, "  prolog.force_persp_sample_interp = %u\n",
              key->ps.part.prolog.force_persp_sample_interp);
      fprintf(f, "  prolog.force_linear_sample_interp = %u\n",
              key->ps.part.prolog.force_linear_sample_interp);
      fprintf(f, "  prolog.force_persp_center_interp = %u\n",
              key->ps.part.prolog.force_persp_center_interp);
      fprintf(f, "  prolog.force_linear_center_interp = %u\n",
              key->ps.part.prolog.force_linear_center_interp);
      fprintf(f, "  prolog.bc_optimize_for_persp = %u\n",
              key->ps.part.prolog.bc_optimize_for_persp);
      fprintf(f, "  prolog.bc_optimize_for_linear = %u\n",
              key->ps.part.prolog.bc_optimize_for_linear);
      fprintf(f, "  prolog.samplemask_log_ps_iter = %u\n",
              key->ps.part.prolog.samplemask_log_ps_iter);
      fprintf(f, "  epilog.spi_shader_col_format = 0x%x\n",
              key->ps.part.epilog.spi_shader_col_format);
      fprintf(f, "  epilog.color_is_int8 = 0x%X\n", key->ps.part.epilog.color_is_int8);
      fprintf(f, "  epilog.color_is_int10 = 0x%X\n", key->ps.part.epilog.color_is_int10);
      fprintf(f, "  epilog.last_cbuf = %u\n", key->ps.part.epilog.last_cbuf);
      fprintf(f, "  epilog.alpha_func = %u\n", key->ps.part.epilog.alpha_func);
      fprintf(f, "  epilog.alpha_to_one = %u\n", key->ps.part.epilog.alpha_to_one);
      fprintf(f, "  epilog.alpha_to_coverage_via_mrtz = %u\n", key->ps.part.epilog.alpha_to_coverage_via_mrtz);
      fprintf(f, "  epilog.clamp_color = %u\n", key->ps.part.epilog.clamp_color);
      fprintf(f, "  epilog.dual_src_blend_swizzle = %u\n", key->ps.part.epilog.dual_src_blend_swizzle);
      fprintf(f, "  epilog.rbplus_depth_only_opt = %u\n", key->ps.part.epilog.rbplus_depth_only_opt);
      fprintf(f, "  epilog.kill_samplemask = %u\n", key->ps.part.epilog.kill_samplemask);
      fprintf(f, "  mono.poly_line_smoothing = %u\n", key->ps.mono.poly_line_smoothing);
      fprintf(f, "  mono.point_smoothing = %u\n", key->ps.mono.point_smoothing);
      fprintf(f, "  mono.interpolate_at_sample_force_center = %u\n",
              key->ps.mono.interpolate_at_sample_force_center);
      fprintf(f, "  mono.fbfetch_msaa = %u\n", key->ps.mono.fbfetch_msaa);
      fprintf(f, "  mono.fbfetch_is_1D = %u\n", key->ps.mono.fbfetch_is_1D);
      fprintf(f, "  mono.fbfetch_layered = %u\n", key->ps.mono.fbfetch_layered);
      break;

   default:
      assert(0);
   }

   if ((stage == MESA_SHADER_GEOMETRY || stage == MESA_SHADER_TESS_EVAL ||
        stage == MESA_SHADER_VERTEX) &&
       !key->ge.as_es && !key->ge.as_ls) {
      fprintf(f, "  opt.kill_outputs = 0x%" PRIx64 "\n", key->ge.opt.kill_outputs);
      fprintf(f, "  opt.kill_pointsize = 0x%x\n", key->ge.opt.kill_pointsize);
      fprintf(f, "  opt.kill_layer = 0x%x\n", key->ge.opt.kill_layer);
      fprintf(f, "  opt.kill_clip_distances = 0x%x\n", key->ge.opt.kill_clip_distances);
      fprintf(f, "  opt.ngg_culling = 0x%x\n", key->ge.opt.ngg_culling);
      fprintf(f, "  opt.remove_streamout = 0x%x\n", key->ge.opt.remove_streamout);
   }

   if (stage <= MESA_SHADER_GEOMETRY)
      fprintf(f, "  opt.prefer_mono = %u\n", key->ge.opt.prefer_mono);
   else
      fprintf(f, "  opt.prefer_mono = %u\n", key->ps.opt.prefer_mono);

   if (stage <= MESA_SHADER_GEOMETRY) {
      if (key->ge.opt.inline_uniforms) {
         fprintf(f, "  opt.inline_uniforms = %u (0x%x, 0x%x, 0x%x, 0x%x)\n",
                 key->ge.opt.inline_uniforms,
                 key->ge.opt.inlined_uniform_values[0],
                 key->ge.opt.inlined_uniform_values[1],
                 key->ge.opt.inlined_uniform_values[2],
                 key->ge.opt.inlined_uniform_values[3]);
      } else {
         fprintf(f, "  opt.inline_uniforms = 0\n");
      }
   } else {
      if (key->ps.opt.inline_uniforms) {
         fprintf(f, "  opt.inline_uniforms = %u (0x%x, 0x%x, 0x%x, 0x%x)\n",
                 key->ps.opt.inline_uniforms,
                 key->ps.opt.inlined_uniform_values[0],
                 key->ps.opt.inlined_uniform_values[1],
                 key->ps.opt.inlined_uniform_values[2],
                 key->ps.opt.inlined_uniform_values[3]);
      } else {
         fprintf(f, "  opt.inline_uniforms = 0\n");
      }
   }
}

bool si_vs_needs_prolog(const struct si_shader_selector *sel,
                        const struct si_vs_prolog_bits *prolog_key)
{
   assert(sel->stage == MESA_SHADER_VERTEX);

   /* VGPR initialization fixup for Vega10 and Raven is always done in the
    * VS prolog. */
   return sel->info.vs_needs_prolog || prolog_key->ls_vgpr_fix;
}

/**
 * Compute the VS prolog key, which contains all the information needed to
 * build the VS prolog function, and set shader->info bits where needed.
 *
 * \param info             Shader info of the vertex shader.
 * \param num_input_sgprs  Number of input SGPRs for the vertex shader.
 * \param prolog_key       Key of the VS prolog
 * \param shader_out       The vertex shader, or the next shader if merging LS+HS or ES+GS.
 * \param key              Output shader part key.
 */
void si_get_vs_prolog_key(const struct si_shader_info *info, unsigned num_input_sgprs,
                          const struct si_vs_prolog_bits *prolog_key,
                          struct si_shader *shader_out, union si_shader_part_key *key)
{
   memset(key, 0, sizeof(*key));
   key->vs_prolog.states = *prolog_key;
   key->vs_prolog.wave32 = shader_out->wave_size == 32;
   key->vs_prolog.num_input_sgprs = num_input_sgprs;
   key->vs_prolog.num_inputs = info->num_inputs;
   key->vs_prolog.as_ls = shader_out->key.ge.as_ls;
   key->vs_prolog.as_es = shader_out->key.ge.as_es;
   key->vs_prolog.as_ngg = shader_out->key.ge.as_ngg;

   if (shader_out->selector->stage == MESA_SHADER_TESS_CTRL) {
      key->vs_prolog.as_ls = 1;
      key->vs_prolog.num_merged_next_stage_vgprs = 2;
   } else if (shader_out->selector->stage == MESA_SHADER_GEOMETRY) {
      key->vs_prolog.as_es = 1;
      key->vs_prolog.num_merged_next_stage_vgprs = 5;
   } else if (shader_out->key.ge.as_ngg) {
      key->vs_prolog.num_merged_next_stage_vgprs = 5;
   }

   /* Only one of these combinations can be set. as_ngg can be set with as_es. */
   assert(key->vs_prolog.as_ls + key->vs_prolog.as_ngg +
          (key->vs_prolog.as_es && !key->vs_prolog.as_ngg) <= 1);

   /* Enable loading the InstanceID VGPR. */
   uint16_t input_mask = u_bit_consecutive(0, info->num_inputs);

   if ((key->vs_prolog.states.instance_divisor_is_one |
        key->vs_prolog.states.instance_divisor_is_fetched) &
       input_mask)
      shader_out->info.uses_instanceid = true;
}

/* TODO: convert to nir_shader_instructions_pass */
static bool si_nir_kill_outputs(nir_shader *nir, const union si_shader_key *key)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   assert(impl);
   assert(nir->info.stage <= MESA_SHADER_GEOMETRY);

   if (!key->ge.opt.kill_outputs &&
       !key->ge.opt.kill_pointsize &&
       !key->ge.opt.kill_layer &&
       !key->ge.opt.kill_clip_distances &&
       !(nir->info.outputs_written & BITFIELD64_BIT(VARYING_SLOT_LAYER))) {
      nir_metadata_preserve(impl, nir_metadata_all);
      return false;
   }

   bool progress = false;

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
         if (intr->intrinsic != nir_intrinsic_store_output)
            continue;

         /* No indirect indexing allowed. */
         ASSERTED nir_src offset = *nir_get_io_offset_src(intr);
         assert(nir_src_is_const(offset) && nir_src_as_uint(offset) == 0);

         assert(intr->num_components == 1); /* only scalar stores expected */
         nir_io_semantics sem = nir_intrinsic_io_semantics(intr);

         if (nir_slot_is_varying(sem.location) &&
             key->ge.opt.kill_outputs &
             (1ull << si_shader_io_get_unique_index(sem.location)))
            progress |= nir_remove_varying(intr, MESA_SHADER_FRAGMENT);

         switch (sem.location) {
         case VARYING_SLOT_PSIZ:
            if (key->ge.opt.kill_pointsize)
               progress |= nir_remove_sysval_output(intr);
            break;

         case VARYING_SLOT_CLIP_VERTEX:
            /* TODO: We should only kill specific clip planes as required by kill_clip_distance,
             * not whole gl_ClipVertex. Lower ClipVertex in NIR.
             */
            if ((key->ge.opt.kill_clip_distances & SI_USER_CLIP_PLANE_MASK) ==
                SI_USER_CLIP_PLANE_MASK)
               progress |= nir_remove_sysval_output(intr);
            break;

         case VARYING_SLOT_CLIP_DIST0:
         case VARYING_SLOT_CLIP_DIST1:
            if (key->ge.opt.kill_clip_distances) {
               assert(nir_intrinsic_src_type(intr) == nir_type_float32);
               unsigned index = (sem.location - VARYING_SLOT_CLIP_DIST0) * 4 +
                                nir_intrinsic_component(intr);

               if (key->ge.opt.kill_clip_distances & BITFIELD_BIT(index))
                  progress |= nir_remove_sysval_output(intr);
            }
            break;

         case VARYING_SLOT_LAYER:
            /* LAYER is never passed to FS. Instead, we load it there as a system value. */
            progress |= nir_remove_varying(intr, MESA_SHADER_FRAGMENT);

            if (key->ge.opt.kill_layer)
               progress |= nir_remove_sysval_output(intr);
            break;
         }
      }
   }

   if (progress) {
      nir_metadata_preserve(impl, nir_metadata_dominance |
                                  nir_metadata_block_index);
   } else {
      nir_metadata_preserve(impl, nir_metadata_all);
   }

   return progress;
}

/* Remove PS output components from NIR if they are disabled by spi_shader_col_format. */
static bool kill_ps_outputs_cb(struct nir_builder *b, nir_instr *instr, void *_key)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   /* No indirect indexing allowed. */
   ASSERTED nir_src offset = *nir_get_io_offset_src(intr);
   assert(nir_src_is_const(offset) && nir_src_as_uint(offset) == 0);

   unsigned location = nir_intrinsic_io_semantics(intr).location;
   const union si_shader_key *key = _key;

   switch (location) {
   case FRAG_RESULT_DEPTH:
   case FRAG_RESULT_STENCIL:
      return false;

   case FRAG_RESULT_SAMPLE_MASK:
      if (key->ps.part.epilog.kill_samplemask) {
         nir_instr_remove(instr);
         return true;
      }
      return false;
   }

   /* Color outputs. */
   unsigned comp_mask = BITFIELD_MASK(intr->num_components);
   assert(nir_intrinsic_component(intr) == 0);
   unsigned cb_shader_mask = ac_get_cb_shader_mask(key->ps.part.epilog.spi_shader_col_format);

   /* If COLOR is broadcasted to multiple color buffers, combine their masks. */
   if (location == FRAG_RESULT_COLOR) {
      for (unsigned i = 1; i <= key->ps.part.epilog.last_cbuf; i++)
         cb_shader_mask |= (cb_shader_mask >> (i * 4)) & 0xf;
   }

   unsigned index = location == FRAG_RESULT_COLOR ? 0 : location - FRAG_RESULT_DATA0;
   unsigned output_mask = (cb_shader_mask >> (index * 4)) & 0xf;

   if ((output_mask & comp_mask) == comp_mask)
      return false;

   if (!(output_mask & comp_mask)) {
      nir_instr_remove(instr);
      return true;
   }

   /* Fill disabled components with undef. */
   b->cursor = nir_before_instr(instr);
   nir_def *new_value = intr->src[0].ssa;
   nir_def *undef = nir_undef(b, 1, new_value->bit_size);

   unsigned kill_mask = ~output_mask & comp_mask;
   u_foreach_bit(i, kill_mask) {
      new_value = nir_vector_insert_imm(b, new_value, undef, i);
   }

   nir_src_rewrite(&intr->src[0], new_value);
   return true;
}

static bool si_nir_kill_ps_outputs(nir_shader *nir, const union si_shader_key *key)
{
   assert(nir->info.stage == MESA_SHADER_FRAGMENT);
   return nir_shader_instructions_pass(nir, kill_ps_outputs_cb,
                                       nir_metadata_dominance |
                                       nir_metadata_block_index, (void*)key);
}

static bool clamp_vertex_color_instr(nir_builder *b,
                                     nir_intrinsic_instr *intrin, void *state)
{
   if (intrin->intrinsic != nir_intrinsic_store_output)
      return false;

   unsigned location = nir_intrinsic_io_semantics(intrin).location;
   if (location != VARYING_SLOT_COL0 && location != VARYING_SLOT_COL1 &&
       location != VARYING_SLOT_BFC0 && location != VARYING_SLOT_BFC1)
      return false;

   /* no indirect output */
   assert(nir_src_is_const(intrin->src[1]) && !nir_src_as_uint(intrin->src[1]));
   /* only scalar output */
   assert(intrin->src[0].ssa->num_components == 1);

   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *color = intrin->src[0].ssa;
   nir_def *clamp = nir_load_clamp_vertex_color_amd(b);
   nir_def *new_color = nir_bcsel(b, clamp, nir_fsat(b, color), color);
   nir_src_rewrite(&intrin->src[0], new_color);

   return true;
}

static bool si_nir_clamp_vertex_color(nir_shader *nir)
{
   uint64_t mask = VARYING_BIT_COL0 | VARYING_BIT_COL1 | VARYING_BIT_BFC0 | VARYING_BIT_BFC1;
   if (!(nir->info.outputs_written & mask))
      return false;

   return nir_shader_intrinsics_pass(nir, clamp_vertex_color_instr,
                                       nir_metadata_dominance | nir_metadata_block_index,
                                       NULL);
}

static unsigned si_map_io_driver_location(unsigned semantic)
{
   if ((semantic >= VARYING_SLOT_PATCH0 && semantic < VARYING_SLOT_TESS_MAX) ||
       semantic == VARYING_SLOT_TESS_LEVEL_INNER ||
       semantic == VARYING_SLOT_TESS_LEVEL_OUTER)
      return ac_shader_io_get_unique_index_patch(semantic);

   return si_shader_io_get_unique_index(semantic);
}

static bool si_lower_io_to_mem(struct si_shader *shader, nir_shader *nir,
                               uint64_t tcs_vgpr_only_inputs)
{
   struct si_shader_selector *sel = shader->selector;
   const union si_shader_key *key = &shader->key;

   if (nir->info.stage == MESA_SHADER_VERTEX) {
      if (key->ge.as_ls) {
         NIR_PASS_V(nir, ac_nir_lower_ls_outputs_to_mem, si_map_io_driver_location,
                    key->ge.opt.same_patch_vertices, tcs_vgpr_only_inputs);
         return true;
      } else if (key->ge.as_es) {
         NIR_PASS_V(nir, ac_nir_lower_es_outputs_to_mem, si_map_io_driver_location,
                    sel->screen->info.gfx_level, sel->info.esgs_vertex_stride);
         return true;
      }
   } else if (nir->info.stage == MESA_SHADER_TESS_CTRL) {
      NIR_PASS_V(nir, ac_nir_lower_hs_inputs_to_mem, si_map_io_driver_location,
                 key->ge.opt.same_patch_vertices);

      /* Used by hs_emit_write_tess_factors() when monolithic shader. */
      nir->info.tess._primitive_mode = key->ge.part.tcs.epilog.prim_mode;

      NIR_PASS_V(nir, ac_nir_lower_hs_outputs_to_mem, si_map_io_driver_location,
                 sel->screen->info.gfx_level,
                 /* Used by hs_emit_write_tess_factors() when monolithic shader. */
                 key->ge.part.tcs.epilog.tes_reads_tess_factors,
                 ~0ULL, ~0ULL, /* no TES inputs filter */
                 util_last_bit64(sel->info.outputs_written_before_tes_gs),
                 util_last_bit64(sel->info.patch_outputs_written),
                 shader->wave_size,
                 /* ALL TCS inputs are passed by register. */
                 key->ge.opt.same_patch_vertices &&
                 !(sel->info.base.inputs_read & ~sel->info.tcs_vgpr_only_inputs),
                 sel->info.tessfactors_are_def_in_all_invocs,
                 /* Emit epilog only when monolithic shader. */
                 shader->is_monolithic);
      return true;
   } else if (nir->info.stage == MESA_SHADER_TESS_EVAL) {
      NIR_PASS_V(nir, ac_nir_lower_tes_inputs_to_mem, si_map_io_driver_location);

      if (key->ge.as_es) {
         NIR_PASS_V(nir, ac_nir_lower_es_outputs_to_mem, si_map_io_driver_location,
                    sel->screen->info.gfx_level, sel->info.esgs_vertex_stride);
      }

      return true;
   } else if (nir->info.stage == MESA_SHADER_GEOMETRY) {
      NIR_PASS_V(nir, ac_nir_lower_gs_inputs_to_mem, si_map_io_driver_location,
                 sel->screen->info.gfx_level, key->ge.mono.u.gs_tri_strip_adj_fix);
      return true;
   }

   return false;
}

static void si_lower_ngg(struct si_shader *shader, nir_shader *nir)
{
   struct si_shader_selector *sel = shader->selector;
   const union si_shader_key *key = &shader->key;
   assert(key->ge.as_ngg);

   unsigned clipdist_mask =
      (sel->info.clipdist_mask & ~key->ge.opt.kill_clip_distances) |
      sel->info.culldist_mask;

   ac_nir_lower_ngg_options options = {
      .family = sel->screen->info.family,
      .gfx_level = sel->screen->info.gfx_level,
      .max_workgroup_size = si_get_max_workgroup_size(shader),
      .wave_size = shader->wave_size,
      .can_cull = !!key->ge.opt.ngg_culling,
      .disable_streamout = !si_shader_uses_streamout(shader),
      .vs_output_param_offset = shader->info.vs_output_param_offset,
      .has_param_exports = shader->info.nr_param_exports,
      .clipdist_enable_mask = clipdist_mask,
      .kill_pointsize = key->ge.opt.kill_pointsize,
      .kill_layer = key->ge.opt.kill_layer,
      .force_vrs = sel->screen->options.vrs2x2,
   };

   if (nir->info.stage == MESA_SHADER_VERTEX ||
       nir->info.stage == MESA_SHADER_TESS_EVAL) {
      /* Per instance inputs, used to remove instance load after culling. */
      unsigned instance_rate_inputs = 0;

      if (nir->info.stage == MESA_SHADER_VERTEX) {
         instance_rate_inputs =
            key->ge.part.vs.prolog.instance_divisor_is_one |
            key->ge.part.vs.prolog.instance_divisor_is_fetched;

         /* Manually mark the instance ID used, so the shader can repack it. */
         if (instance_rate_inputs)
            BITSET_SET(nir->info.system_values_read, SYSTEM_VALUE_INSTANCE_ID);
      } else {
         /* Manually mark the primitive ID used, so the shader can repack it. */
         if (key->ge.mono.u.vs_export_prim_id)
            BITSET_SET(nir->info.system_values_read, SYSTEM_VALUE_PRIMITIVE_ID);
      }

      unsigned clip_plane_enable =
         SI_NGG_CULL_GET_CLIP_PLANE_ENABLE(key->ge.opt.ngg_culling);

      options.num_vertices_per_primitive = gfx10_ngg_get_vertices_per_prim(shader);
      options.early_prim_export = gfx10_ngg_export_prim_early(shader);
      options.passthrough = gfx10_is_ngg_passthrough(shader);
      options.use_edgeflags = gfx10_edgeflags_have_effect(shader);
      options.has_gen_prim_query = options.has_xfb_prim_query =
         sel->screen->info.gfx_level >= GFX11 && !sel->info.base.vs.blit_sgprs_amd;
      options.export_primitive_id = key->ge.mono.u.vs_export_prim_id;
      options.instance_rate_inputs = instance_rate_inputs;
      options.user_clip_plane_enable_mask = clip_plane_enable;

      NIR_PASS_V(nir, ac_nir_lower_ngg_nogs, &options);
   } else {
      assert(nir->info.stage == MESA_SHADER_GEOMETRY);

      options.gs_out_vtx_bytes = sel->info.gsvs_vertex_size;
      options.has_gen_prim_query = options.has_xfb_prim_query =
         sel->screen->info.gfx_level >= GFX11;

      /* For monolithic ES/GS to add vscnt wait when GS export pos0. */
      if (key->ge.part.gs.es)
         nir->info.writes_memory |= key->ge.part.gs.es->info.base.writes_memory;

      NIR_PASS_V(nir, ac_nir_lower_ngg_gs, &options);
   }

   /* may generate some subgroup op like ballot */
   NIR_PASS_V(nir, nir_lower_subgroups, &si_nir_subgroups_options);

   /* may generate some vector output store */
   NIR_PASS_V(nir, nir_lower_io_to_scalar, nir_var_shader_out, NULL, NULL);
}

struct nir_shader *si_deserialize_shader(struct si_shader_selector *sel)
{
   struct pipe_screen *screen = &sel->screen->b;
   const void *options = screen->get_compiler_options(screen, PIPE_SHADER_IR_NIR,
                                                      pipe_shader_type_from_mesa(sel->stage));

   struct blob_reader blob_reader;
   blob_reader_init(&blob_reader, sel->nir_binary, sel->nir_size);
   return nir_deserialize(NULL, options, &blob_reader);
}

static void si_nir_assign_param_offsets(nir_shader *nir, struct si_shader *shader,
                                        int8_t slot_remap[NUM_TOTAL_VARYING_SLOTS])
{
   struct si_shader_selector *sel = shader->selector;
   struct si_shader_binary_info *info = &shader->info;

   uint64_t outputs_written = 0;
   uint32_t outputs_written_16bit = 0;

   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   assert(impl);

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
         if (intr->intrinsic != nir_intrinsic_store_output)
            continue;

         /* No indirect indexing allowed. */
         ASSERTED nir_src offset = *nir_get_io_offset_src(intr);
         assert(nir_src_is_const(offset) && nir_src_as_uint(offset) == 0);

         assert(intr->num_components == 1); /* only scalar stores expected */
         nir_io_semantics sem = nir_intrinsic_io_semantics(intr);

         if (sem.location >= VARYING_SLOT_VAR0_16BIT)
            outputs_written_16bit |= BITFIELD_BIT(sem.location - VARYING_SLOT_VAR0_16BIT);
         else
            outputs_written |= BITFIELD64_BIT(sem.location);

         /* Assign the param index if it's unassigned. */
         if (nir_slot_is_varying(sem.location) && !sem.no_varying &&
             (sem.gs_streams & 0x3) == 0 &&
             info->vs_output_param_offset[sem.location] == AC_EXP_PARAM_DEFAULT_VAL_0000) {
            /* The semantic and the base should be the same as in si_shader_info. */
            assert(sem.location == sel->info.output_semantic[nir_intrinsic_base(intr)]);
            /* It must not be remapped (duplicated). */
            assert(slot_remap[sem.location] == -1);

            info->vs_output_param_offset[sem.location] = info->nr_param_exports++;
         }
      }
   }

   /* Duplicated outputs are redirected here. */
   for (unsigned i = 0; i < NUM_TOTAL_VARYING_SLOTS; i++) {
      if (slot_remap[i] >= 0)
         info->vs_output_param_offset[i] = info->vs_output_param_offset[slot_remap[i]];
   }

   if (shader->key.ge.mono.u.vs_export_prim_id) {
      info->vs_output_param_offset[VARYING_SLOT_PRIMITIVE_ID] = info->nr_param_exports++;
   }

   /* Update outputs written info, we may remove some outputs before. */
   nir->info.outputs_written = outputs_written;
   nir->info.outputs_written_16bit = outputs_written_16bit;
}

static void si_assign_param_offsets(nir_shader *nir, struct si_shader *shader)
{
   /* Initialize this first. */
   shader->info.nr_param_exports = 0;

   STATIC_ASSERT(sizeof(shader->info.vs_output_param_offset[0]) == 1);
   memset(shader->info.vs_output_param_offset, AC_EXP_PARAM_DEFAULT_VAL_0000,
          sizeof(shader->info.vs_output_param_offset));

   /* A slot remapping table for duplicated outputs, so that 1 vertex shader output can be
    * mapped to multiple fragment shader inputs.
    */
   int8_t slot_remap[NUM_TOTAL_VARYING_SLOTS];
   memset(slot_remap, -1, NUM_TOTAL_VARYING_SLOTS);

   /* This sets DEFAULT_VAL for constant outputs in vs_output_param_offset. */
   /* TODO: This doesn't affect GS. */
   NIR_PASS_V(nir, ac_nir_optimize_outputs, false, slot_remap,
              shader->info.vs_output_param_offset);

   /* Assign the non-constant outputs. */
   /* TODO: Use this for the GS copy shader too. */
   si_nir_assign_param_offsets(nir, shader, slot_remap);
}

static unsigned si_get_nr_pos_exports(const struct si_shader_selector *sel,
                                      const union si_shader_key *key)
{
   const struct si_shader_info *info = &sel->info;

   /* Must have a position export. */
   unsigned nr_pos_exports = 1;

   if ((info->writes_psize && !key->ge.opt.kill_pointsize) ||
       (info->writes_edgeflag && !key->ge.as_ngg) ||
       (info->writes_layer && !key->ge.opt.kill_layer) ||
       info->writes_viewport_index || sel->screen->options.vrs2x2) {
      nr_pos_exports++;
   }

   unsigned clipdist_mask =
      (info->clipdist_mask & ~key->ge.opt.kill_clip_distances) | info->culldist_mask;

   for (int i = 0; i < 2; i++) {
      if (clipdist_mask & BITFIELD_RANGE(i * 4, 4))
         nr_pos_exports++;
   }

   return nr_pos_exports;
}

static bool lower_ps_load_color_intrinsic(nir_builder *b, nir_instr *instr, void *state)
{
   nir_def **colors = (nir_def **)state;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   if (intrin->intrinsic != nir_intrinsic_load_color0 &&
       intrin->intrinsic != nir_intrinsic_load_color1)
      return false;

   unsigned index = intrin->intrinsic == nir_intrinsic_load_color0 ? 0 : 1;
   assert(colors[index]);

   nir_def_rewrite_uses(&intrin->def, colors[index]);

   nir_instr_remove(&intrin->instr);
   return true;
}

static bool si_nir_lower_ps_color_input(nir_shader *nir, const union si_shader_key *key,
                                        const struct si_shader_info *info)
{
   bool progress = false;
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   nir_builder builder = nir_builder_at(nir_before_impl(impl));
   nir_builder *b = &builder;

   /* Build ready to be used colors at the beginning of the shader. */
   nir_def *colors[2] = {0};
   for (int i = 0; i < 2; i++) {
      if (!(info->colors_read & (0xf << (i * 4))))
         continue;

      unsigned color_base = info->color_attr_index[i];
      /* If BCOLOR0 is used, BCOLOR1 is at offset "num_inputs + 1",
       * otherwise it's at offset "num_inputs".
       */
      unsigned back_color_base = info->num_inputs;
      if (i == 1 && (info->colors_read & 0xf))
         back_color_base += 1;

      enum glsl_interp_mode interp_mode = info->color_interpolate[i];
      if (interp_mode == INTERP_MODE_COLOR) {
         interp_mode = key->ps.part.prolog.flatshade_colors ?
            INTERP_MODE_FLAT : INTERP_MODE_SMOOTH;
      }

      nir_def *back_color = NULL;
      if (interp_mode == INTERP_MODE_FLAT) {
         colors[i] = nir_load_input(b, 4, 32, nir_imm_int(b, 0),
                                   .base = color_base,
                                   .io_semantics.location = VARYING_SLOT_COL0 + i,
                                   .io_semantics.num_slots = 1);

         if (key->ps.part.prolog.color_two_side) {
            back_color = nir_load_input(b, 4, 32, nir_imm_int(b, 0),
                                        .base = back_color_base,
                                        .io_semantics.location = VARYING_SLOT_BFC0 + i,
                                        .io_semantics.num_slots = 1);
         }
      } else {
         nir_intrinsic_op op = 0;
         switch (info->color_interpolate_loc[i]) {
         case TGSI_INTERPOLATE_LOC_CENTER:
            op = nir_intrinsic_load_barycentric_pixel;
            break;
         case TGSI_INTERPOLATE_LOC_CENTROID:
            op = nir_intrinsic_load_barycentric_centroid;
            break;
         case TGSI_INTERPOLATE_LOC_SAMPLE:
            op = nir_intrinsic_load_barycentric_sample;
            break;
         default:
            unreachable("invalid color interpolate location");
            break;
         }

         nir_def *barycentric = nir_load_barycentric(b, op, interp_mode);

         colors[i] =
            nir_load_interpolated_input(b, 4, 32, barycentric, nir_imm_int(b, 0),
                                        .base = color_base,
                                        .io_semantics.location = VARYING_SLOT_COL0 + i,
                                        .io_semantics.num_slots = 1);

         if (key->ps.part.prolog.color_two_side) {
            back_color =
               nir_load_interpolated_input(b, 4, 32, barycentric, nir_imm_int(b, 0),
                                           .base = back_color_base,
                                           .io_semantics.location = VARYING_SLOT_BFC0 + i,
                                           .io_semantics.num_slots = 1);
         }
      }

      if (back_color) {
         nir_def *is_front_face = nir_load_front_face(b, 1);
         colors[i] = nir_bcsel(b, is_front_face, colors[i], back_color);
      }

      progress = true;
   }

   /* lower nir_load_color0/1 to use the color value. */
   return nir_shader_instructions_pass(nir, lower_ps_load_color_intrinsic,
                                       nir_metadata_block_index | nir_metadata_dominance,
                                       colors) || progress;
}

static void si_nir_emit_polygon_stipple(nir_shader *nir, struct si_shader_args *args)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   nir_builder builder = nir_builder_at(nir_before_impl(impl));
   nir_builder *b = &builder;

   /* Load the buffer descriptor. */
   nir_def *desc =
      si_nir_load_internal_binding(b, args, SI_PS_CONST_POLY_STIPPLE, 4);

   /* Use the fixed-point gl_FragCoord input.
    * Since the stipple pattern is 32x32 and it repeats, just get 5 bits
    * per coordinate to get the repeating effect.
    */
   nir_def *pos_x = ac_nir_unpack_arg(b, &args->ac, args->ac.pos_fixed_pt, 0, 5);
   nir_def *pos_y = ac_nir_unpack_arg(b, &args->ac, args->ac.pos_fixed_pt, 16, 5);

   nir_def *zero = nir_imm_int(b, 0);
   /* The stipple pattern is 32x32, each row has 32 bits. */
   nir_def *offset = nir_ishl_imm(b, pos_y, 2);
   nir_def *row = nir_load_buffer_amd(b, 1, 32, desc, offset, zero, zero);
   nir_def *bit = nir_ubfe(b, row, pos_x, nir_imm_int(b, 1));

   nir_def *pass = nir_i2b(b, bit);
   nir_discard_if(b, nir_inot(b, pass));
}

bool si_should_clear_lds(struct si_screen *sscreen, const struct nir_shader *shader)
{
   return shader->info.stage == MESA_SHADER_COMPUTE && shader->info.shared_size > 0 && sscreen->options.clear_lds;
}

struct nir_shader *si_get_nir_shader(struct si_shader *shader,
                                     struct si_shader_args *args,
                                     bool *free_nir,
                                     uint64_t tcs_vgpr_only_inputs,
                                     ac_nir_gs_output_info *output_info)
{
   struct si_shader_selector *sel = shader->selector;
   const union si_shader_key *key = &shader->key;

   nir_shader *nir;
   *free_nir = false;

   if (sel->nir) {
      nir = sel->nir;
   } else if (sel->nir_binary) {
      nir = si_deserialize_shader(sel);
      *free_nir = true;
   } else {
      return NULL;
   }

   bool progress = false;
   bool late_opts = false;

   const char *original_name = NULL;
   if (unlikely(should_print_nir(nir))) {
      /* Modify the shader's name so that each variant gets its own name. */
      original_name = ralloc_strdup(nir, nir->info.name);
      ralloc_asprintf_append((char **)&nir->info.name, "-%08x", _mesa_hash_data(key, sizeof(*key)));

      /* Dummy pass to get the starting point. */
      printf("nir_dummy_pass\n");
      nir_print_shader(nir, stdout);
   }

   /* Kill outputs according to the shader key. */
   if (sel->stage <= MESA_SHADER_GEOMETRY)
      NIR_PASS(progress, nir, si_nir_kill_outputs, key);

   NIR_PASS(progress, nir, ac_nir_lower_tex,
            &(ac_nir_lower_tex_options){
               .gfx_level = sel->screen->info.gfx_level,
               .lower_array_layer_round_even = !sel->screen->info.conformant_trunc_coord,
            });

   if (nir->info.uses_resource_info_query)
      NIR_PASS(progress, nir, ac_nir_lower_resinfo, sel->screen->info.gfx_level);

   bool inline_uniforms = false;
   uint32_t *inlined_uniform_values;
   si_get_inline_uniform_state((union si_shader_key*)key, sel->pipe_shader_type,
                               &inline_uniforms, &inlined_uniform_values);

   if (inline_uniforms) {
      assert(*free_nir);

      /* Most places use shader information from the default variant, not
       * the optimized variant. These are the things that the driver looks at
       * in optimized variants and the list of things that we need to do.
       *
       * The driver takes into account these things if they suddenly disappear
       * from the shader code:
       * - Register usage and code size decrease (obvious)
       * - Eliminated PS system values are disabled by LLVM
       *   (FragCoord, FrontFace, barycentrics)
       * - VS/TES/GS param exports are eliminated if they are undef.
       *   The param space for eliminated outputs is also not allocated.
       * - VS/TCS/TES/GS/PS input loads are eliminated (VS relies on DCE in LLVM)
       * - TCS output stores are eliminated
       * - Eliminated PS inputs are removed from PS.NUM_INTERP.
       *
       * TODO: These are things the driver ignores in the final shader code
       * and relies on the default shader info.
       * - System values in VS, TCS, TES, GS are not eliminated
       * - uses_discard - if it changed to false
       * - writes_memory - if it changed to false
       * - VS->TCS, VS->GS, TES->GS output stores for the former stage are not
       *   eliminated
       * - Eliminated VS/TCS/TES outputs are still allocated. (except when feeding PS)
       *   GS outputs are eliminated except for the temporary LDS.
       *   Clip distances, gl_PointSize, gl_Layer and PS outputs are eliminated based
       *   on current states, so we don't care about the shader code.
       *
       * TODO: Merged shaders don't inline uniforms for the first stage.
       * VS-GS: only GS inlines uniforms; VS-TCS: only TCS; TES-GS: only GS.
       * (key == NULL for the first stage here)
       *
       * TODO: Compute shaders don't support inlinable uniforms, because they
       * don't have shader variants.
       *
       * TODO: The driver uses a linear search to find a shader variant. This
       * can be really slow if we get too many variants due to uniform inlining.
       */
      NIR_PASS_V(nir, nir_inline_uniforms, nir->info.num_inlinable_uniforms,
                 inlined_uniform_values, nir->info.inlinable_uniform_dw_offsets);
      progress = true;
   }

   if (sel->stage == MESA_SHADER_FRAGMENT) {
      /* This uses the epilog key, so only monolithic shaders can call this. */
      if (shader->is_monolithic)
         NIR_PASS(progress, nir, si_nir_kill_ps_outputs, key);

      if (key->ps.mono.poly_line_smoothing)
         NIR_PASS(progress, nir, nir_lower_poly_line_smooth, SI_NUM_SMOOTH_AA_SAMPLES);

      if (key->ps.mono.point_smoothing)
         NIR_PASS(progress, nir, nir_lower_point_smooth);
   }

   /* This must be before si_nir_lower_resource. */
   if (!sel->screen->info.has_image_opcodes)
      NIR_PASS(progress, nir, ac_nir_lower_image_opcodes);

   /* LLVM does not work well with this, so is handled in llvm backend waterfall. */
   if (sel->screen->use_aco && sel->info.has_non_uniform_tex_access) {
      nir_lower_non_uniform_access_options options = {
         .types = nir_lower_non_uniform_texture_access,
      };
      NIR_PASS(progress, nir, nir_lower_non_uniform_access, &options);
   }

   NIR_PASS(progress, nir, si_nir_lower_resource, shader, args);

   bool is_last_vgt_stage =
      (sel->stage == MESA_SHADER_VERTEX ||
       sel->stage == MESA_SHADER_TESS_EVAL ||
       (sel->stage == MESA_SHADER_GEOMETRY && shader->key.ge.as_ngg)) &&
      !shader->key.ge.as_ls && !shader->key.ge.as_es;

   /* Legacy GS is not last VGT stage because it has GS copy shader. */
   bool is_legacy_gs = sel->stage == MESA_SHADER_GEOMETRY && !key->ge.as_ngg;

   if (is_last_vgt_stage || is_legacy_gs)
      NIR_PASS(progress, nir, si_nir_clamp_vertex_color);

   if (progress) {
      si_nir_opts(sel->screen, nir, true);
      late_opts = true;
      progress = false;
   }

   /* Lower large variables that are always constant with load_constant intrinsics, which
    * get turned into PC-relative loads from a data section next to the shader.
    *
    * Loop unrolling caused by uniform inlining can help eliminate indirect indexing, so
    * this should be done after that.
    *
    * The pass crashes if there are dead temps of lowered IO interface types, so remove
    * them first.
    */
   NIR_PASS_V(nir, nir_remove_dead_variables, nir_var_function_temp, NULL);
   NIR_PASS(progress, nir, nir_opt_large_constants, glsl_get_natural_size_align_bytes, 16);

   /* Loop unrolling caused by uniform inlining can help eliminate indirect indexing, so
    * this should be done after that.
    */
   progress |= ac_nir_lower_indirect_derefs(nir, sel->screen->info.gfx_level);

   if (sel->stage == MESA_SHADER_VERTEX)
      NIR_PASS(progress, nir, si_nir_lower_vs_inputs, shader, args);

   progress |= si_lower_io_to_mem(shader, nir, tcs_vgpr_only_inputs);

   if (is_last_vgt_stage) {
      /* Assign param export indices. */
      si_assign_param_offsets(nir, shader);

      /* Assign num of position exports. */
      shader->info.nr_pos_exports = si_get_nr_pos_exports(sel, key);

      if (key->ge.as_ngg) {
         /* Lower last VGT NGG shader stage. */
         si_lower_ngg(shader, nir);
      } else if (sel->stage == MESA_SHADER_VERTEX || sel->stage == MESA_SHADER_TESS_EVAL) {
         /* Lower last VGT none-NGG VS/TES shader stage. */
         unsigned clip_cull_mask =
            (sel->info.clipdist_mask & ~key->ge.opt.kill_clip_distances) |
            sel->info.culldist_mask;

         NIR_PASS_V(nir, ac_nir_lower_legacy_vs,
                    sel->screen->info.gfx_level,
                    clip_cull_mask,
                    shader->info.vs_output_param_offset,
                    shader->info.nr_param_exports,
                    shader->key.ge.mono.u.vs_export_prim_id,
                    !si_shader_uses_streamout(shader),
                    key->ge.opt.kill_pointsize,
                    key->ge.opt.kill_layer,
                    sel->screen->options.vrs2x2);
      }
      progress = true;
   } else if (is_legacy_gs) {
      NIR_PASS_V(nir, ac_nir_lower_legacy_gs, false, sel->screen->use_ngg, output_info);
      progress = true;
   } else if (sel->stage == MESA_SHADER_FRAGMENT && shader->is_monolithic) {
      /* Uniform inlining can eliminate PS inputs, and colormask can remove PS outputs,
       * which can also cause the elimination of PS inputs. Remove holes after removed PS inputs
       * by renumbering them. This can only happen with monolithic PS. Colors are unaffected
       * because they are still represented by nir_intrinsic_load_color0/1.
       */
      NIR_PASS_V(nir, nir_recompute_io_bases, nir_var_shader_in);

      /* Two-side color selection and interpolation: Get the latest shader info because
       * uniform inlining and colormask can fully eliminate color inputs.
       */
      struct si_shader_info info;
      si_nir_scan_shader(sel->screen, nir, &info);

      if (info.colors_read)
         NIR_PASS(progress, nir, si_nir_lower_ps_color_input, &shader->key, &info);

      /* We need to set this early for lowering nir_intrinsic_load_point_coord_maybe_flipped,
       * which can only occur with monolithic PS.
       */
      shader->info.num_ps_inputs = info.num_inputs;
      shader->info.ps_colors_read = info.colors_read;

      ac_nir_lower_ps_options options = {
         .gfx_level = sel->screen->info.gfx_level,
         .family = sel->screen->info.family,
         .use_aco = sel->screen->use_aco,
         .uses_discard = si_shader_uses_discard(shader),
         .alpha_to_coverage_via_mrtz = key->ps.part.epilog.alpha_to_coverage_via_mrtz,
         .dual_src_blend_swizzle = key->ps.part.epilog.dual_src_blend_swizzle,
         .spi_shader_col_format = key->ps.part.epilog.spi_shader_col_format,
         .color_is_int8 = key->ps.part.epilog.color_is_int8,
         .color_is_int10 = key->ps.part.epilog.color_is_int10,
         .clamp_color = key->ps.part.epilog.clamp_color,
         .alpha_to_one = key->ps.part.epilog.alpha_to_one,
         .alpha_func = key->ps.part.epilog.alpha_func,
         .broadcast_last_cbuf = key->ps.part.epilog.last_cbuf,
         .kill_samplemask = key->ps.part.epilog.kill_samplemask,

         .bc_optimize_for_persp = key->ps.part.prolog.bc_optimize_for_persp,
         .bc_optimize_for_linear = key->ps.part.prolog.bc_optimize_for_linear,
         .force_persp_sample_interp = key->ps.part.prolog.force_persp_sample_interp,
         .force_linear_sample_interp = key->ps.part.prolog.force_linear_sample_interp,
         .force_persp_center_interp = key->ps.part.prolog.force_persp_center_interp,
         .force_linear_center_interp = key->ps.part.prolog.force_linear_center_interp,
         .ps_iter_samples = 1 << key->ps.part.prolog.samplemask_log_ps_iter,
      };

      NIR_PASS_V(nir, ac_nir_lower_ps, &options);

      if (key->ps.part.prolog.poly_stipple)
         NIR_PASS_V(nir, si_nir_emit_polygon_stipple, args);

      progress = true;
   }

   NIR_PASS(progress, nir, nir_opt_idiv_const, 8);
   NIR_PASS(progress, nir, nir_lower_idiv,
            &(nir_lower_idiv_options){
               .allow_fp16 = sel->screen->info.gfx_level >= GFX9,
            });

   if (si_should_clear_lds(sel->screen, nir)) {
      const unsigned chunk_size = 16; /* max single store size */
      const unsigned shared_size = ALIGN(nir->info.shared_size, chunk_size);
      NIR_PASS_V(nir, nir_clear_shared_memory, shared_size, chunk_size);
   }

   NIR_PASS(progress, nir, ac_nir_lower_intrinsics_to_args, sel->screen->info.gfx_level,
            si_select_hw_stage(nir->info.stage, key, sel->screen->info.gfx_level),
            &args->ac);
   NIR_PASS(progress, nir, si_nir_lower_abi, shader, args);

   if (progress) {
      si_nir_opts(sel->screen, nir, false);
      progress = false;
      late_opts = true;
   }

   static const nir_opt_offsets_options offset_options = {
      .uniform_max = 0,
      .buffer_max = ~0,
      .shared_max = ~0,
   };
   NIR_PASS_V(nir, nir_opt_offsets, &offset_options);

   if (late_opts)
      si_nir_late_opts(nir);

   /* aco only accept scalar const, must be done after si_nir_late_opts()
    * which may generate vec const.
    */
   if (sel->screen->use_aco)
      NIR_PASS_V(nir, nir_lower_load_const_to_scalar);

   /* This helps LLVM form VMEM clauses and thus get more GPU cache hits.
    * 200 is tuned for Viewperf. It should be done last.
    */
   NIR_PASS_V(nir, nir_group_loads, nir_group_same_resource_only, 200);

   if (unlikely(original_name)) {
      ralloc_free((void*)nir->info.name);
      nir->info.name = original_name;
   }

   return nir;
}

void si_update_shader_binary_info(struct si_shader *shader, nir_shader *nir)
{
   struct si_shader_info info;
   si_nir_scan_shader(shader->selector->screen, nir, &info);

   shader->info.uses_vmem_load_other |= info.uses_vmem_load_other;
   shader->info.uses_vmem_sampler_or_bvh |= info.uses_vmem_sampler_or_bvh;

   if (nir->info.stage == MESA_SHADER_FRAGMENT) {
      /* Since uniform inlining can remove PS inputs, set the latest info about PS inputs here. */
      shader->info.num_ps_inputs = info.num_inputs;
      shader->info.ps_colors_read = info.colors_read;

      /* A non-monolithic PS doesn't know if back colors are enabled, so copy 2 more. */
      unsigned max_interp = MIN2(info.num_inputs + 2, SI_NUM_INTERP);
      memcpy(shader->info.ps_inputs, info.input, max_interp * sizeof(info.input[0]));
   }
}

/* Generate code for the hardware VS shader stage to go with a geometry shader */
static struct si_shader *
si_nir_generate_gs_copy_shader(struct si_screen *sscreen,
                               struct ac_llvm_compiler *compiler,
                               struct si_shader *gs_shader,
                               nir_shader *gs_nir,
                               struct util_debug_callback *debug,
                               ac_nir_gs_output_info *output_info)
{
   struct si_shader *shader;
   struct si_shader_selector *gs_selector = gs_shader->selector;
   struct si_shader_info *gsinfo = &gs_selector->info;
   union si_shader_key *gskey = &gs_shader->key;

   shader = CALLOC_STRUCT(si_shader);
   if (!shader)
      return NULL;

   /* We can leave the fence as permanently signaled because the GS copy
    * shader only becomes visible globally after it has been compiled. */
   util_queue_fence_init(&shader->ready);

   shader->selector = gs_selector;
   shader->is_gs_copy_shader = true;
   shader->wave_size = si_determine_wave_size(sscreen, shader);

   STATIC_ASSERT(sizeof(shader->info.vs_output_param_offset[0]) == 1);
   memset(shader->info.vs_output_param_offset, AC_EXP_PARAM_DEFAULT_VAL_0000,
          sizeof(shader->info.vs_output_param_offset));

   for (unsigned i = 0; i < gsinfo->num_outputs; i++) {
      unsigned semantic = gsinfo->output_semantic[i];

      /* Skip if no channel writes to stream 0. */
      if (!nir_slot_is_varying(semantic) ||
          (gsinfo->output_streams[i] & 0x03 &&
           gsinfo->output_streams[i] & 0x0c &&
           gsinfo->output_streams[i] & 0x30 &&
           gsinfo->output_streams[i] & 0xc0))
         continue;

      shader->info.vs_output_param_offset[semantic] = shader->info.nr_param_exports++;
   }

   shader->info.nr_pos_exports = si_get_nr_pos_exports(gs_selector, gskey);

   unsigned clip_cull_mask =
      (gsinfo->clipdist_mask & ~gskey->ge.opt.kill_clip_distances) | gsinfo->culldist_mask;

   nir_shader *nir =
      ac_nir_create_gs_copy_shader(gs_nir,
                                   sscreen->info.gfx_level,
                                   clip_cull_mask,
                                   shader->info.vs_output_param_offset,
                                   shader->info.nr_param_exports,
                                   !si_shader_uses_streamout(gs_shader),
                                   gskey->ge.opt.kill_pointsize,
                                   gskey->ge.opt.kill_layer,
                                   sscreen->options.vrs2x2,
                                   output_info);

   struct si_shader_args args;
   si_init_shader_args(shader, &args);

   NIR_PASS_V(nir, ac_nir_lower_intrinsics_to_args, sscreen->info.gfx_level, AC_HW_VERTEX_SHADER, &args.ac);
   NIR_PASS_V(nir, si_nir_lower_abi, shader, &args);

   si_nir_opts(gs_selector->screen, nir, false);

   /* aco only accept scalar const */
   if (sscreen->use_aco)
      NIR_PASS_V(nir, nir_lower_load_const_to_scalar);

   if (si_can_dump_shader(sscreen, MESA_SHADER_GEOMETRY, SI_DUMP_NIR)) {
      fprintf(stderr, "GS Copy Shader:\n");
      nir_print_shader(nir, stderr);
   }

   bool ok =
#if LLVM_AVAILABLE
      !sscreen->use_aco ? si_llvm_compile_shader(sscreen, compiler, shader, &args, debug, nir) :
#endif
      si_aco_compile_shader(shader, &args, nir, debug);


   if (ok) {
      assert(!shader->config.scratch_bytes_per_wave);
      ok = si_shader_binary_upload(sscreen, shader, 0);
      si_shader_dump(sscreen, shader, debug, stderr, true);
   }
   ralloc_free(nir);

   if (!ok) {
      FREE(shader);
      shader = NULL;
   } else {
      si_fix_resource_usage(sscreen, shader);
   }
   return shader;
}

struct si_gs_output_info {
   uint8_t streams[64];
   uint8_t streams_16bit_lo[16];
   uint8_t streams_16bit_hi[16];

   uint8_t usage_mask[64];
   uint8_t usage_mask_16bit_lo[16];
   uint8_t usage_mask_16bit_hi[16];

   ac_nir_gs_output_info info;
};

static void
si_init_gs_output_info(struct si_shader_info *info, struct si_gs_output_info *out_info)
{
   for (int i = 0; i < info->num_outputs; i++) {
      unsigned slot = info->output_semantic[i];
      if (slot < VARYING_SLOT_VAR0_16BIT) {
         out_info->streams[slot] = info->output_streams[i];
         out_info->usage_mask[slot] = info->output_usagemask[i];
      } else {
         unsigned index = slot - VARYING_SLOT_VAR0_16BIT;
         /* TODO: 16bit need separated fields for lo/hi part. */
         out_info->streams_16bit_lo[index] = info->output_streams[i];
         out_info->streams_16bit_hi[index] = info->output_streams[i];
         out_info->usage_mask_16bit_lo[index] = info->output_usagemask[i];
         out_info->usage_mask_16bit_hi[index] = info->output_usagemask[i];
      }
   }

   ac_nir_gs_output_info *ac_info = &out_info->info;

   ac_info->streams = out_info->streams;
   ac_info->streams_16bit_lo = out_info->streams_16bit_lo;
   ac_info->streams_16bit_hi = out_info->streams_16bit_hi;

   ac_info->usage_mask = out_info->usage_mask;
   ac_info->usage_mask_16bit_lo = out_info->usage_mask_16bit_lo;
   ac_info->usage_mask_16bit_hi = out_info->usage_mask_16bit_hi;

   /* TODO: construct 16bit slot per component store type. */
   ac_info->types_16bit_lo = ac_info->types_16bit_hi = NULL;
}

static void si_fixup_spi_ps_input_config(struct si_shader *shader)
{
   const union si_shader_key *key = &shader->key;

   /* Enable POS_FIXED_PT if polygon stippling is enabled. */
   if (key->ps.part.prolog.poly_stipple)
      shader->config.spi_ps_input_ena |= S_0286CC_POS_FIXED_PT_ENA(1);

   /* Set up the enable bits for per-sample shading if needed. */
   if (key->ps.part.prolog.force_persp_sample_interp &&
       (G_0286CC_PERSP_CENTER_ENA(shader->config.spi_ps_input_ena) ||
        G_0286CC_PERSP_CENTROID_ENA(shader->config.spi_ps_input_ena))) {
      shader->config.spi_ps_input_ena &= C_0286CC_PERSP_CENTER_ENA;
      shader->config.spi_ps_input_ena &= C_0286CC_PERSP_CENTROID_ENA;
      shader->config.spi_ps_input_ena |= S_0286CC_PERSP_SAMPLE_ENA(1);
   }
   if (key->ps.part.prolog.force_linear_sample_interp &&
       (G_0286CC_LINEAR_CENTER_ENA(shader->config.spi_ps_input_ena) ||
        G_0286CC_LINEAR_CENTROID_ENA(shader->config.spi_ps_input_ena))) {
      shader->config.spi_ps_input_ena &= C_0286CC_LINEAR_CENTER_ENA;
      shader->config.spi_ps_input_ena &= C_0286CC_LINEAR_CENTROID_ENA;
      shader->config.spi_ps_input_ena |= S_0286CC_LINEAR_SAMPLE_ENA(1);
   }
   if (key->ps.part.prolog.force_persp_center_interp &&
       (G_0286CC_PERSP_SAMPLE_ENA(shader->config.spi_ps_input_ena) ||
        G_0286CC_PERSP_CENTROID_ENA(shader->config.spi_ps_input_ena))) {
      shader->config.spi_ps_input_ena &= C_0286CC_PERSP_SAMPLE_ENA;
      shader->config.spi_ps_input_ena &= C_0286CC_PERSP_CENTROID_ENA;
      shader->config.spi_ps_input_ena |= S_0286CC_PERSP_CENTER_ENA(1);
   }
   if (key->ps.part.prolog.force_linear_center_interp &&
       (G_0286CC_LINEAR_SAMPLE_ENA(shader->config.spi_ps_input_ena) ||
        G_0286CC_LINEAR_CENTROID_ENA(shader->config.spi_ps_input_ena))) {
      shader->config.spi_ps_input_ena &= C_0286CC_LINEAR_SAMPLE_ENA;
      shader->config.spi_ps_input_ena &= C_0286CC_LINEAR_CENTROID_ENA;
      shader->config.spi_ps_input_ena |= S_0286CC_LINEAR_CENTER_ENA(1);
   }

   /* POW_W_FLOAT requires that one of the perspective weights is enabled. */
   if (G_0286CC_POS_W_FLOAT_ENA(shader->config.spi_ps_input_ena) &&
       !(shader->config.spi_ps_input_ena & 0xf)) {
      shader->config.spi_ps_input_ena |= S_0286CC_PERSP_CENTER_ENA(1);
   }

   /* At least one pair of interpolation weights must be enabled. */
   if (!(shader->config.spi_ps_input_ena & 0x7f))
      shader->config.spi_ps_input_ena |= S_0286CC_LINEAR_CENTER_ENA(1);

   /* Samplemask fixup requires the sample ID. */
   if (key->ps.part.prolog.samplemask_log_ps_iter)
      shader->config.spi_ps_input_ena |= S_0286CC_ANCILLARY_ENA(1);
}

static void
si_set_spi_ps_input_config(struct si_shader *shader)
{
   const struct si_shader_selector *sel = shader->selector;
   const struct si_shader_info *info = &sel->info;
   const union si_shader_key *key = &shader->key;

   /* TODO: This should be determined from the final NIR instead of the input NIR,
    * otherwise LLVM will have a performance advantage here because it determines
    * VGPR inputs for each shader variant after LLVM optimizations.
    */
   shader->config.spi_ps_input_ena =
      S_0286CC_PERSP_CENTER_ENA(info->uses_persp_center) |
      S_0286CC_PERSP_CENTROID_ENA(info->uses_persp_centroid) |
      S_0286CC_PERSP_SAMPLE_ENA(info->uses_persp_sample) |
      S_0286CC_LINEAR_CENTER_ENA(info->uses_linear_center) |
      S_0286CC_LINEAR_CENTROID_ENA(info->uses_linear_centroid) |
      S_0286CC_LINEAR_SAMPLE_ENA(info->uses_linear_sample) |
      S_0286CC_FRONT_FACE_ENA(info->uses_frontface && !key->ps.opt.force_front_face_input) |
      S_0286CC_SAMPLE_COVERAGE_ENA(info->reads_samplemask) |
      S_0286CC_ANCILLARY_ENA(info->uses_sampleid || info->uses_layer_id);

   uint8_t mask = info->reads_frag_coord_mask | info->reads_sample_pos_mask;
   u_foreach_bit(i, mask) {
      shader->config.spi_ps_input_ena |= S_0286CC_POS_X_FLOAT_ENA(1) << i;
   }

   if (key->ps.part.prolog.color_two_side)
      shader->config.spi_ps_input_ena |= S_0286CC_FRONT_FACE_ENA(1);

   /* INTERP_MODE_COLOR, same as SMOOTH if flat shading is disabled. */
   if (info->uses_interp_color && !key->ps.part.prolog.flatshade_colors) {
      shader->config.spi_ps_input_ena |=
         S_0286CC_PERSP_SAMPLE_ENA(info->uses_persp_sample_color) |
         S_0286CC_PERSP_CENTER_ENA(info->uses_persp_center_color) |
         S_0286CC_PERSP_CENTROID_ENA(info->uses_persp_centroid_color);
   }

   /* nir_lower_poly_line_smooth use nir_load_sample_mask_in */
   if (key->ps.mono.poly_line_smoothing)
      shader->config.spi_ps_input_ena |= S_0286CC_SAMPLE_COVERAGE_ENA(1);

   /* nir_lower_point_smooth use nir_load_point_coord_maybe_flipped which is lowered
    * to nir_load_barycentric_pixel and nir_load_interpolated_input.
    */
   if (key->ps.mono.point_smoothing)
      shader->config.spi_ps_input_ena |= S_0286CC_PERSP_CENTER_ENA(1);

   /* See fetch_framebuffer() for used args when fbfetch output. */
   if (info->base.fs.uses_fbfetch_output) {
      shader->config.spi_ps_input_ena |= S_0286CC_POS_FIXED_PT_ENA(1);

      if (key->ps.mono.fbfetch_layered || key->ps.mono.fbfetch_msaa)
         shader->config.spi_ps_input_ena |= S_0286CC_ANCILLARY_ENA(1);
   }

   if (shader->is_monolithic) {
      si_fixup_spi_ps_input_config(shader);
      shader->config.spi_ps_input_addr = shader->config.spi_ps_input_ena;
   } else {
      /* Part mode will call si_fixup_spi_ps_input_config() when combining multi
       * shader part in si_shader_select_ps_parts().
       *
       * Reserve register locations for VGPR inputs the PS prolog may need.
       */
      shader->config.spi_ps_input_addr =
         shader->config.spi_ps_input_ena |
         SI_SPI_PS_INPUT_ADDR_FOR_PROLOG;
   }
}

static void
debug_message_stderr(void *data, unsigned *id, enum util_debug_type ptype,
                      const char *fmt, va_list args)
{
   vfprintf(stderr, fmt, args);
   fprintf(stderr, "\n");
}

bool si_compile_shader(struct si_screen *sscreen, struct ac_llvm_compiler *compiler,
                       struct si_shader *shader, struct util_debug_callback *debug)
{
   bool ret = true;
   struct si_shader_selector *sel = shader->selector;

   /* ACO need spi_ps_input in advance to init args and used in compiler. */
   if (sel->stage == MESA_SHADER_FRAGMENT && sscreen->use_aco)
      si_set_spi_ps_input_config(shader);

   /* We need this info only when legacy GS. */
   struct si_gs_output_info legacy_gs_output_info;
   if (sel->stage == MESA_SHADER_GEOMETRY && !shader->key.ge.as_ngg) {
      memset(&legacy_gs_output_info, 0, sizeof(legacy_gs_output_info));
      si_init_gs_output_info(&sel->info, &legacy_gs_output_info);
   }

   struct si_shader_args args;
   si_init_shader_args(shader, &args);

   bool free_nir;
   struct nir_shader *nir =
      si_get_nir_shader(shader, &args, &free_nir, 0, &legacy_gs_output_info.info);

   /* Dump NIR before doing NIR->LLVM conversion in case the
    * conversion fails. */
   if (si_can_dump_shader(sscreen, sel->stage, SI_DUMP_NIR)) {
      nir_print_shader(nir, stderr);

      if (nir->xfb_info)
         nir_print_xfb_info(nir->xfb_info, stderr);
   }

   /* Initialize vs_output_ps_input_cntl to default. */
   for (unsigned i = 0; i < ARRAY_SIZE(shader->info.vs_output_ps_input_cntl); i++)
      shader->info.vs_output_ps_input_cntl[i] = SI_PS_INPUT_CNTL_UNUSED;
   shader->info.vs_output_ps_input_cntl[VARYING_SLOT_COL0] = SI_PS_INPUT_CNTL_UNUSED_COLOR0;

   si_update_shader_binary_info(shader, nir);

   /* uses_instanceid may be set by si_nir_lower_vs_inputs(). */
   shader->info.uses_instanceid |= sel->info.uses_instanceid;
   shader->info.private_mem_vgprs = DIV_ROUND_UP(nir->scratch_size, 4);

   /* Set the FP ALU behavior. */
   /* By default, we disable denormals for FP32 and enable them for FP16 and FP64
    * for performance and correctness reasons. FP32 denormals can't be enabled because
    * they break output modifiers and v_mad_f32 and are very slow on GFX6-7.
    *
    * float_controls_execution_mode defines the set of valid behaviors. Contradicting flags
    * can be set simultaneously, which means we are allowed to choose, but not really because
    * some options cause GLCTS failures.
    */
   unsigned float_mode = V_00B028_FP_16_64_DENORMS;

   if (!(nir->info.float_controls_execution_mode & FLOAT_CONTROLS_ROUNDING_MODE_RTE_FP32) &&
       nir->info.float_controls_execution_mode & FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP32)
      float_mode |= V_00B028_FP_32_ROUND_TOWARDS_ZERO;

   if (!(nir->info.float_controls_execution_mode & (FLOAT_CONTROLS_ROUNDING_MODE_RTE_FP16 |
                                                    FLOAT_CONTROLS_ROUNDING_MODE_RTE_FP64)) &&
       nir->info.float_controls_execution_mode & (FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP16 |
                                                  FLOAT_CONTROLS_ROUNDING_MODE_RTZ_FP64))
      float_mode |= V_00B028_FP_16_64_ROUND_TOWARDS_ZERO;

   if (!(nir->info.float_controls_execution_mode & (FLOAT_CONTROLS_DENORM_PRESERVE_FP16 |
                                                    FLOAT_CONTROLS_DENORM_PRESERVE_FP64)) &&
       nir->info.float_controls_execution_mode & (FLOAT_CONTROLS_DENORM_FLUSH_TO_ZERO_FP16 |
                                                  FLOAT_CONTROLS_DENORM_FLUSH_TO_ZERO_FP64))
      float_mode &= ~V_00B028_FP_16_64_DENORMS;

   ret =
#if LLVM_AVAILABLE
      !sscreen->use_aco ? si_llvm_compile_shader(sscreen, compiler, shader, &args, debug, nir) :
#endif
      si_aco_compile_shader(shader, &args, nir, debug);

   if (!ret)
      goto out;

   shader->config.float_mode = float_mode;

   /* The GS copy shader is compiled next. */
   if (sel->stage == MESA_SHADER_GEOMETRY && !shader->key.ge.as_ngg) {
      shader->gs_copy_shader =
         si_nir_generate_gs_copy_shader(sscreen, compiler, shader, nir, debug,
                                        &legacy_gs_output_info.info);
      if (!shader->gs_copy_shader) {
         fprintf(stderr, "radeonsi: can't create GS copy shader\n");
         ret = false;
         goto out;
      }
   }

   /* Compute vs_output_ps_input_cntl. */
   if ((sel->stage == MESA_SHADER_VERTEX ||
        sel->stage == MESA_SHADER_TESS_EVAL ||
        sel->stage == MESA_SHADER_GEOMETRY) &&
       !shader->key.ge.as_ls && !shader->key.ge.as_es) {
      uint8_t *vs_output_param_offset = shader->info.vs_output_param_offset;

      if (sel->stage == MESA_SHADER_GEOMETRY && !shader->key.ge.as_ngg)
         vs_output_param_offset = shader->gs_copy_shader->info.vs_output_param_offset;

      /* We must use the original shader info before the removal of duplicated shader outputs. */
      /* VS and TES should also set primitive ID output if it's used. */
      unsigned num_outputs_with_prim_id = sel->info.num_outputs +
                                          shader->key.ge.mono.u.vs_export_prim_id;

      for (unsigned i = 0; i < num_outputs_with_prim_id; i++) {
         unsigned semantic = sel->info.output_semantic[i];
         unsigned offset = vs_output_param_offset[semantic];
         unsigned ps_input_cntl;

         if (offset <= AC_EXP_PARAM_OFFSET_31) {
            /* The input is loaded from parameter memory. */
            ps_input_cntl = S_028644_OFFSET(offset);
         } else {
            /* The input is a DEFAULT_VAL constant. */
            assert(offset >= AC_EXP_PARAM_DEFAULT_VAL_0000 &&
                   offset <= AC_EXP_PARAM_DEFAULT_VAL_1111);
            offset -= AC_EXP_PARAM_DEFAULT_VAL_0000;

            /* OFFSET=0x20 means that DEFAULT_VAL is used. */
            ps_input_cntl = S_028644_OFFSET(0x20) |
                            S_028644_DEFAULT_VAL(offset);
         }

         shader->info.vs_output_ps_input_cntl[semantic] = ps_input_cntl;
      }
   }

   /* Validate SGPR and VGPR usage for compute to detect compiler bugs. */
   if (sel->stage == MESA_SHADER_COMPUTE) {
      unsigned max_vgprs =
         sscreen->info.num_physical_wave64_vgprs_per_simd * (shader->wave_size == 32 ? 2 : 1);
      unsigned max_sgprs = sscreen->info.num_physical_sgprs_per_simd;
      unsigned max_sgprs_per_wave = 128;
      unsigned simds_per_tg = 4; /* assuming WGP mode on gfx10 */
      unsigned threads_per_tg = si_get_max_workgroup_size(shader);
      unsigned waves_per_tg = DIV_ROUND_UP(threads_per_tg, shader->wave_size);
      unsigned waves_per_simd = DIV_ROUND_UP(waves_per_tg, simds_per_tg);

      max_vgprs = max_vgprs / waves_per_simd;
      max_sgprs = MIN2(max_sgprs / waves_per_simd, max_sgprs_per_wave);

      if (shader->config.num_sgprs > max_sgprs || shader->config.num_vgprs > max_vgprs) {
         fprintf(stderr,
                 "LLVM failed to compile a shader correctly: "
                 "SGPR:VGPR usage is %u:%u, but the hw limit is %u:%u\n",
                 shader->config.num_sgprs, shader->config.num_vgprs, max_sgprs, max_vgprs);

         /* Just terminate the process, because dependent
          * shaders can hang due to bad input data, but use
          * the env var to allow shader-db to work.
          */
         if (!debug_get_bool_option("SI_PASS_BAD_SHADERS", false))
            abort();
      }
   }

   /* Add/remove the scratch offset to/from input SGPRs. */
   if (sel->screen->info.gfx_level < GFX11 &&
       (sel->screen->info.family < CHIP_GFX940 || sel->screen->info.has_graphics) &&
       !si_is_merged_shader(shader)) {
      if (sscreen->use_aco) {
         /* When aco scratch_offset arg is added explicitly at the beginning.
          * After compile if no scratch used, reduce the input sgpr count.
          */
         if (!shader->config.scratch_bytes_per_wave)
            shader->info.num_input_sgprs--;
      } else {
         /* scratch_offset arg is added by llvm implicitly */
         if (shader->info.num_input_sgprs)
            shader->info.num_input_sgprs++;
      }
   }

   /* Calculate the number of fragment input VGPRs. */
   if (sel->stage == MESA_SHADER_FRAGMENT) {
      shader->info.num_input_vgprs = ac_get_fs_input_vgpr_cnt(
         &shader->config, &shader->info.num_fragcoord_components);
   }

   si_calculate_max_simd_waves(shader);

   if (si_can_dump_shader(sscreen, sel->stage, SI_DUMP_STATS)) {
      struct util_debug_callback out_stderr = {
         .debug_message = debug_message_stderr,
      };

      si_shader_dump_stats_for_shader_db(sscreen, shader, &out_stderr);
   } else {
      si_shader_dump_stats_for_shader_db(sscreen, shader, debug);
   }

out:
   if (free_nir)
      ralloc_free(nir);

   return ret;
}

/**
 * Create, compile and return a shader part (prolog or epilog).
 *
 * \param sscreen  screen
 * \param list     list of shader parts of the same category
 * \param type     shader type
 * \param key      shader part key
 * \param prolog   whether the part being requested is a prolog
 * \param tm       LLVM target machine
 * \param debug    debug callback
 * \return         non-NULL on success
 */
static struct si_shader_part *
si_get_shader_part(struct si_screen *sscreen, struct si_shader_part **list,
                   gl_shader_stage stage, bool prolog, union si_shader_part_key *key,
                   struct ac_llvm_compiler *compiler, struct util_debug_callback *debug,
                   const char *name)
{
   struct si_shader_part *result;

   simple_mtx_lock(&sscreen->shader_parts_mutex);

   /* Find existing. */
   for (result = *list; result; result = result->next) {
      if (memcmp(&result->key, key, sizeof(*key)) == 0) {
         simple_mtx_unlock(&sscreen->shader_parts_mutex);
         return result;
      }
   }

   /* Compile a new one. */
   result = CALLOC_STRUCT(si_shader_part);
   result->key = *key;

   bool ok =
#if LLVM_AVAILABLE
      !sscreen->use_aco ? si_llvm_build_shader_part(sscreen, stage, prolog, compiler, debug, name, result) :
#endif
      si_aco_build_shader_part(sscreen, stage, prolog, debug, name, result);

   if (ok) {
      result->next = *list;
      *list = result;
   } else {
      FREE(result);
      result = NULL;
   }

   simple_mtx_unlock(&sscreen->shader_parts_mutex);
   return result;
}

static bool si_get_vs_prolog(struct si_screen *sscreen, struct ac_llvm_compiler *compiler,
                             struct si_shader *shader, struct util_debug_callback *debug,
                             struct si_shader *main_part, const struct si_vs_prolog_bits *key)
{
   struct si_shader_selector *vs = main_part->selector;

   if (!si_vs_needs_prolog(vs, key))
      return true;

   /* Get the prolog. */
   union si_shader_part_key prolog_key;
   si_get_vs_prolog_key(&vs->info, main_part->info.num_input_sgprs, key, shader,
                        &prolog_key);

   shader->prolog =
      si_get_shader_part(sscreen, &sscreen->vs_prologs, MESA_SHADER_VERTEX, true, &prolog_key,
                         compiler, debug, "Vertex Shader Prolog");
   return shader->prolog != NULL;
}

/**
 * Select and compile (or reuse) vertex shader parts (prolog & epilog).
 */
static bool si_shader_select_vs_parts(struct si_screen *sscreen, struct ac_llvm_compiler *compiler,
                                      struct si_shader *shader, struct util_debug_callback *debug)
{
   return si_get_vs_prolog(sscreen, compiler, shader, debug, shader, &shader->key.ge.part.vs.prolog);
}

void si_get_tcs_epilog_key(struct si_shader *shader, union si_shader_part_key *key)
{
   memset(key, 0, sizeof(*key));
   key->tcs_epilog.wave32 = shader->wave_size == 32;
   key->tcs_epilog.states = shader->key.ge.part.tcs.epilog;

   /* If output patches are wholly in one wave, we don't need a barrier. */
   key->tcs_epilog.noop_s_barrier =
      shader->wave_size % shader->selector->info.base.tess.tcs_vertices_out == 0;
}

/**
 * Select and compile (or reuse) TCS parts (epilog).
 */
static bool si_shader_select_tcs_parts(struct si_screen *sscreen, struct ac_llvm_compiler *compiler,
                                       struct si_shader *shader, struct util_debug_callback *debug)
{
   if (sscreen->info.gfx_level >= GFX9) {
      struct si_shader *ls_main_part = shader->key.ge.part.tcs.ls->main_shader_part_ls;

      if (!si_get_vs_prolog(sscreen, compiler, shader, debug, ls_main_part,
                            &shader->key.ge.part.tcs.ls_prolog))
         return false;

      shader->previous_stage = ls_main_part;
   }

   /* Get the epilog. */
   union si_shader_part_key epilog_key;
   si_get_tcs_epilog_key(shader, &epilog_key);

   shader->epilog = si_get_shader_part(sscreen, &sscreen->tcs_epilogs, MESA_SHADER_TESS_CTRL, false,
                                       &epilog_key, compiler, debug,
                                       "Tessellation Control Shader Epilog");
   return shader->epilog != NULL;
}

/**
 * Select and compile (or reuse) GS parts (prolog).
 */
static bool si_shader_select_gs_parts(struct si_screen *sscreen, struct ac_llvm_compiler *compiler,
                                      struct si_shader *shader, struct util_debug_callback *debug)
{
   if (sscreen->info.gfx_level >= GFX9) {
      struct si_shader *es_main_part;

      if (shader->key.ge.as_ngg)
         es_main_part = shader->key.ge.part.gs.es->main_shader_part_ngg_es;
      else
         es_main_part = shader->key.ge.part.gs.es->main_shader_part_es;

      if (shader->key.ge.part.gs.es->stage == MESA_SHADER_VERTEX &&
          !si_get_vs_prolog(sscreen, compiler, shader, debug, es_main_part,
                            &shader->key.ge.part.gs.vs_prolog))
         return false;

      shader->previous_stage = es_main_part;
   }

   return true;
}

/**
 * Compute the PS prolog key, which contains all the information needed to
 * build the PS prolog function, and set related bits in shader->config.
 */
void si_get_ps_prolog_key(struct si_shader *shader, union si_shader_part_key *key)
{
   struct si_shader_info *info = &shader->selector->info;

   memset(key, 0, sizeof(*key));
   key->ps_prolog.states = shader->key.ps.part.prolog;
   key->ps_prolog.wave32 = shader->wave_size == 32;
   key->ps_prolog.colors_read = shader->info.ps_colors_read;
   key->ps_prolog.num_input_sgprs = shader->info.num_input_sgprs;
   key->ps_prolog.wqm =
      info->base.fs.needs_quad_helper_invocations &&
      (key->ps_prolog.colors_read || key->ps_prolog.states.force_persp_sample_interp ||
       key->ps_prolog.states.force_linear_sample_interp ||
       key->ps_prolog.states.force_persp_center_interp ||
       key->ps_prolog.states.force_linear_center_interp ||
       key->ps_prolog.states.bc_optimize_for_persp || key->ps_prolog.states.bc_optimize_for_linear);
   key->ps_prolog.num_fragcoord_components = shader->info.num_fragcoord_components;

   if (shader->key.ps.part.prolog.poly_stipple)
      shader->info.uses_vmem_load_other = true;

   if (shader->info.ps_colors_read) {
      uint8_t *color = shader->selector->info.color_attr_index;

      if (shader->key.ps.part.prolog.color_two_side) {
         /* BCOLORs are stored after the last input. */
         key->ps_prolog.num_interp_inputs = shader->info.num_ps_inputs;
         shader->config.spi_ps_input_ena |= S_0286CC_FRONT_FACE_ENA(1);
      }

      for (unsigned i = 0; i < 2; i++) {
         unsigned interp = info->color_interpolate[i];
         unsigned location = info->color_interpolate_loc[i];

         if (!(shader->info.ps_colors_read & (0xf << i * 4)))
            continue;

         key->ps_prolog.color_attr_index[i] = color[i];

         if (shader->key.ps.part.prolog.flatshade_colors && interp == INTERP_MODE_COLOR)
            interp = INTERP_MODE_FLAT;

         switch (interp) {
         case INTERP_MODE_FLAT:
            key->ps_prolog.color_interp_vgpr_index[i] = -1;
            break;
         case INTERP_MODE_SMOOTH:
         case INTERP_MODE_COLOR:
            /* Force the interpolation location for colors here. */
            if (shader->key.ps.part.prolog.force_persp_sample_interp)
               location = TGSI_INTERPOLATE_LOC_SAMPLE;
            if (shader->key.ps.part.prolog.force_persp_center_interp)
               location = TGSI_INTERPOLATE_LOC_CENTER;

            switch (location) {
            case TGSI_INTERPOLATE_LOC_SAMPLE:
               key->ps_prolog.color_interp_vgpr_index[i] = 0;
               shader->config.spi_ps_input_ena |= S_0286CC_PERSP_SAMPLE_ENA(1);
               break;
            case TGSI_INTERPOLATE_LOC_CENTER:
               key->ps_prolog.color_interp_vgpr_index[i] = 2;
               shader->config.spi_ps_input_ena |= S_0286CC_PERSP_CENTER_ENA(1);
               break;
            case TGSI_INTERPOLATE_LOC_CENTROID:
               key->ps_prolog.color_interp_vgpr_index[i] = 4;
               shader->config.spi_ps_input_ena |= S_0286CC_PERSP_CENTROID_ENA(1);
               break;
            default:
               assert(0);
            }
            break;
         case INTERP_MODE_NOPERSPECTIVE:
            /* Force the interpolation location for colors here. */
            if (shader->key.ps.part.prolog.force_linear_sample_interp)
               location = TGSI_INTERPOLATE_LOC_SAMPLE;
            if (shader->key.ps.part.prolog.force_linear_center_interp)
               location = TGSI_INTERPOLATE_LOC_CENTER;

            /* The VGPR assignment for non-monolithic shaders
             * works because InitialPSInputAddr is set on the
             * main shader and PERSP_PULL_MODEL is never used.
             */
            switch (location) {
            case TGSI_INTERPOLATE_LOC_SAMPLE:
               key->ps_prolog.color_interp_vgpr_index[i] = 6;
               shader->config.spi_ps_input_ena |= S_0286CC_LINEAR_SAMPLE_ENA(1);
               break;
            case TGSI_INTERPOLATE_LOC_CENTER:
               key->ps_prolog.color_interp_vgpr_index[i] = 8;
               shader->config.spi_ps_input_ena |= S_0286CC_LINEAR_CENTER_ENA(1);
               break;
            case TGSI_INTERPOLATE_LOC_CENTROID:
               key->ps_prolog.color_interp_vgpr_index[i] = 10;
               shader->config.spi_ps_input_ena |= S_0286CC_LINEAR_CENTROID_ENA(1);
               break;
            default:
               assert(0);
            }
            break;
         default:
            assert(0);
         }
      }
   }
}

/**
 * Check whether a PS prolog is required based on the key.
 */
bool si_need_ps_prolog(const union si_shader_part_key *key)
{
   return key->ps_prolog.colors_read || key->ps_prolog.states.force_persp_sample_interp ||
          key->ps_prolog.states.force_linear_sample_interp ||
          key->ps_prolog.states.force_persp_center_interp ||
          key->ps_prolog.states.force_linear_center_interp ||
          key->ps_prolog.states.bc_optimize_for_persp ||
          key->ps_prolog.states.bc_optimize_for_linear || key->ps_prolog.states.poly_stipple ||
          key->ps_prolog.states.samplemask_log_ps_iter;
}

/**
 * Compute the PS epilog key, which contains all the information needed to
 * build the PS epilog function.
 */
void si_get_ps_epilog_key(struct si_shader *shader, union si_shader_part_key *key)
{
   struct si_shader_info *info = &shader->selector->info;
   memset(key, 0, sizeof(*key));
   key->ps_epilog.wave32 = shader->wave_size == 32;
   key->ps_epilog.uses_discard = si_shader_uses_discard(shader);
   key->ps_epilog.colors_written = info->colors_written;
   key->ps_epilog.color_types = info->output_color_types;
   key->ps_epilog.writes_z = info->writes_z;
   key->ps_epilog.writes_stencil = info->writes_stencil;
   key->ps_epilog.writes_samplemask = info->writes_samplemask &&
                                      !shader->key.ps.part.epilog.kill_samplemask;
   key->ps_epilog.states = shader->key.ps.part.epilog;
}

/**
 * Select and compile (or reuse) pixel shader parts (prolog & epilog).
 */
static bool si_shader_select_ps_parts(struct si_screen *sscreen, struct ac_llvm_compiler *compiler,
                                      struct si_shader *shader, struct util_debug_callback *debug)
{
   union si_shader_part_key prolog_key;
   union si_shader_part_key epilog_key;

   /* Get the prolog. */
   si_get_ps_prolog_key(shader, &prolog_key);

   /* The prolog is a no-op if these aren't set. */
   if (si_need_ps_prolog(&prolog_key)) {
      shader->prolog =
         si_get_shader_part(sscreen, &sscreen->ps_prologs, MESA_SHADER_FRAGMENT, true, &prolog_key,
                            compiler, debug, "Fragment Shader Prolog");
      if (!shader->prolog)
         return false;
   }

   /* Get the epilog. */
   si_get_ps_epilog_key(shader, &epilog_key);

   shader->epilog =
      si_get_shader_part(sscreen, &sscreen->ps_epilogs, MESA_SHADER_FRAGMENT, false, &epilog_key,
                         compiler, debug, "Fragment Shader Epilog");
   if (!shader->epilog)
      return false;

   si_fixup_spi_ps_input_config(shader);

   /* Make sure spi_ps_input_addr bits is superset of spi_ps_input_ena. */
   unsigned spi_ps_input_ena = shader->config.spi_ps_input_ena;
   unsigned spi_ps_input_addr = shader->config.spi_ps_input_addr;
   assert((spi_ps_input_ena & spi_ps_input_addr) == spi_ps_input_ena);

   return true;
}

void si_multiwave_lds_size_workaround(struct si_screen *sscreen, unsigned *lds_size)
{
   /* If tessellation is all offchip and on-chip GS isn't used, this
    * workaround is not needed.
    */
   return;

   /* SPI barrier management bug:
    *   Make sure we have at least 4k of LDS in use to avoid the bug.
    *   It applies to workgroup sizes of more than one wavefront.
    */
   if (sscreen->info.family == CHIP_BONAIRE || sscreen->info.family == CHIP_KABINI)
      *lds_size = MAX2(*lds_size, 8);
}

static void si_fix_resource_usage(struct si_screen *sscreen, struct si_shader *shader)
{
   unsigned min_sgprs = shader->info.num_input_sgprs + 2; /* VCC */

   shader->config.num_sgprs = MAX2(shader->config.num_sgprs, min_sgprs);

   if (shader->selector->stage == MESA_SHADER_COMPUTE &&
       si_get_max_workgroup_size(shader) > shader->wave_size) {
      si_multiwave_lds_size_workaround(sscreen, &shader->config.lds_size);
   }
}

bool si_create_shader_variant(struct si_screen *sscreen, struct ac_llvm_compiler *compiler,
                              struct si_shader *shader, struct util_debug_callback *debug)
{
   struct si_shader_selector *sel = shader->selector;
   struct si_shader *mainp = *si_get_main_shader_part(sel, &shader->key);

   if (sel->stage == MESA_SHADER_FRAGMENT) {
      shader->ps.writes_samplemask = sel->info.writes_samplemask &&
                                     !shader->key.ps.part.epilog.kill_samplemask;
   }

   /* LS, ES, VS are compiled on demand if the main part hasn't been
    * compiled for that stage.
    *
    * GS are compiled on demand if the main part hasn't been compiled
    * for the chosen NGG-ness.
    *
    * Vertex shaders are compiled on demand when a vertex fetch
    * workaround must be applied.
    */
   if (shader->is_monolithic) {
      /* Monolithic shader (compiled as a whole, has many variants,
       * may take a long time to compile).
       */
      if (!si_compile_shader(sscreen, compiler, shader, debug))
         return false;
   } else {
      /* The shader consists of several parts:
       *
       * - the middle part is the user shader, it has 1 variant only
       *   and it was compiled during the creation of the shader
       *   selector
       * - the prolog part is inserted at the beginning
       * - the epilog part is inserted at the end
       *
       * The prolog and epilog have many (but simple) variants.
       *
       * Starting with gfx9, geometry and tessellation control
       * shaders also contain the prolog and user shader parts of
       * the previous shader stage.
       */

      if (!mainp)
         return false;

      /* Copy the compiled shader data over. */
      shader->is_binary_shared = true;
      shader->binary = mainp->binary;
      shader->config = mainp->config;
      shader->info = mainp->info;

      /* Select prologs and/or epilogs. */
      switch (sel->stage) {
      case MESA_SHADER_VERTEX:
         if (!si_shader_select_vs_parts(sscreen, compiler, shader, debug))
            return false;
         break;
      case MESA_SHADER_TESS_CTRL:
         if (!si_shader_select_tcs_parts(sscreen, compiler, shader, debug))
            return false;
         break;
      case MESA_SHADER_TESS_EVAL:
         break;
      case MESA_SHADER_GEOMETRY:
         if (!si_shader_select_gs_parts(sscreen, compiler, shader, debug))
            return false;

         /* Clone the GS copy shader for the shader variant.
          * We can't just copy the pointer because we change the pm4 state and
          * si_shader_selector::gs_copy_shader must be immutable because it's shared
          * by multiple contexts.
          */
         if (!shader->key.ge.as_ngg) {
            assert(sel->main_shader_part == mainp);
            assert(sel->main_shader_part->gs_copy_shader);
            assert(sel->main_shader_part->gs_copy_shader->bo);
            assert(!sel->main_shader_part->gs_copy_shader->previous_stage_sel);
            assert(!sel->main_shader_part->gs_copy_shader->scratch_bo);

            shader->gs_copy_shader = CALLOC_STRUCT(si_shader);
            memcpy(shader->gs_copy_shader, sel->main_shader_part->gs_copy_shader,
                   sizeof(*shader->gs_copy_shader));
            /* Increase the reference count. */
            pipe_reference(NULL, &shader->gs_copy_shader->bo->b.b.reference);
            /* Initialize some fields differently. */
            shader->gs_copy_shader->shader_log = NULL;
            shader->gs_copy_shader->is_binary_shared = true;
            util_queue_fence_init(&shader->gs_copy_shader->ready);
         }
         break;
      case MESA_SHADER_FRAGMENT:
         if (!si_shader_select_ps_parts(sscreen, compiler, shader, debug))
            return false;

         /* Make sure we have at least as many VGPRs as there
          * are allocated inputs.
          */
         shader->config.num_vgprs = MAX2(shader->config.num_vgprs, shader->info.num_input_vgprs);
         break;
      default:;
      }

      assert(shader->wave_size == mainp->wave_size);
      assert(!shader->previous_stage || shader->wave_size == shader->previous_stage->wave_size);

      /* Update SGPR and VGPR counts. */
      if (shader->prolog) {
         shader->config.num_sgprs =
            MAX2(shader->config.num_sgprs, shader->prolog->config.num_sgprs);
         shader->config.num_vgprs =
            MAX2(shader->config.num_vgprs, shader->prolog->config.num_vgprs);
      }
      if (shader->previous_stage) {
         shader->config.num_sgprs =
            MAX2(shader->config.num_sgprs, shader->previous_stage->config.num_sgprs);
         shader->config.num_vgprs =
            MAX2(shader->config.num_vgprs, shader->previous_stage->config.num_vgprs);
         shader->config.spilled_sgprs =
            MAX2(shader->config.spilled_sgprs, shader->previous_stage->config.spilled_sgprs);
         shader->config.spilled_vgprs =
            MAX2(shader->config.spilled_vgprs, shader->previous_stage->config.spilled_vgprs);
         shader->info.private_mem_vgprs =
            MAX2(shader->info.private_mem_vgprs, shader->previous_stage->info.private_mem_vgprs);
         shader->config.scratch_bytes_per_wave =
            MAX2(shader->config.scratch_bytes_per_wave,
                 shader->previous_stage->config.scratch_bytes_per_wave);
         shader->info.uses_instanceid |= shader->previous_stage->info.uses_instanceid;
         shader->info.uses_vmem_load_other |= shader->previous_stage->info.uses_vmem_load_other;
         shader->info.uses_vmem_sampler_or_bvh |= shader->previous_stage->info.uses_vmem_sampler_or_bvh;
      }
      if (shader->epilog) {
         shader->config.num_sgprs =
            MAX2(shader->config.num_sgprs, shader->epilog->config.num_sgprs);
         shader->config.num_vgprs =
            MAX2(shader->config.num_vgprs, shader->epilog->config.num_vgprs);
      }
      si_calculate_max_simd_waves(shader);
   }

   if (sel->stage <= MESA_SHADER_GEOMETRY && shader->key.ge.as_ngg) {
      assert(!shader->key.ge.as_es && !shader->key.ge.as_ls);
      if (!gfx10_ngg_calculate_subgroup_info(shader)) {
         fprintf(stderr, "Failed to compute subgroup info\n");
         return false;
      }
   } else if (sscreen->info.gfx_level >= GFX9 && sel->stage == MESA_SHADER_GEOMETRY) {
      gfx9_get_gs_info(shader->previous_stage_sel, sel, &shader->gs_info);
   }

   shader->uses_vs_state_provoking_vertex =
      sscreen->use_ngg &&
      /* Used to convert triangle strips from GS to triangles. */
      ((sel->stage == MESA_SHADER_GEOMETRY &&
        util_rast_prim_is_triangles(sel->info.base.gs.output_primitive)) ||
       (sel->stage == MESA_SHADER_VERTEX &&
        /* Used to export PrimitiveID from the correct vertex. */
        shader->key.ge.mono.u.vs_export_prim_id));

   shader->uses_gs_state_outprim = sscreen->use_ngg &&
                                   /* Only used by streamout in vertex shaders. */
                                   sel->stage == MESA_SHADER_VERTEX &&
                                   si_shader_uses_streamout(shader);

   if (sel->stage == MESA_SHADER_VERTEX) {
      shader->uses_base_instance = sel->info.uses_base_instance ||
                                   shader->key.ge.part.vs.prolog.instance_divisor_is_one ||
                                   shader->key.ge.part.vs.prolog.instance_divisor_is_fetched;
   } else if (sel->stage == MESA_SHADER_TESS_CTRL) {
      shader->uses_base_instance = shader->previous_stage_sel &&
                                   (shader->previous_stage_sel->info.uses_base_instance ||
                                    shader->key.ge.part.tcs.ls_prolog.instance_divisor_is_one ||
                                    shader->key.ge.part.tcs.ls_prolog.instance_divisor_is_fetched);
   } else if (sel->stage == MESA_SHADER_GEOMETRY) {
      shader->uses_base_instance = shader->previous_stage_sel &&
                                   (shader->previous_stage_sel->info.uses_base_instance ||
                                    shader->key.ge.part.gs.vs_prolog.instance_divisor_is_one ||
                                    shader->key.ge.part.gs.vs_prolog.instance_divisor_is_fetched);
   }

   si_fix_resource_usage(sscreen, shader);

   /* Upload. */
   bool ok = si_shader_binary_upload(sscreen, shader, 0);
   si_shader_dump(sscreen, shader, debug, stderr, true);

   if (!ok)
      fprintf(stderr, "LLVM failed to upload shader\n");
   return ok;
}

void si_shader_binary_clean(struct si_shader_binary *binary)
{
   free((void *)binary->code_buffer);
   binary->code_buffer = NULL;

   free(binary->llvm_ir_string);
   binary->llvm_ir_string = NULL;

   free((void *)binary->symbols);
   binary->symbols = NULL;

   free(binary->uploaded_code);
   binary->uploaded_code = NULL;
   binary->uploaded_code_size = 0;
}

void si_shader_destroy(struct si_shader *shader)
{
   if (shader->scratch_bo)
      si_resource_reference(&shader->scratch_bo, NULL);

   si_resource_reference(&shader->bo, NULL);

   if (!shader->is_binary_shared)
      si_shader_binary_clean(&shader->binary);

   free(shader->shader_log);
}

nir_shader *si_get_prev_stage_nir_shader(struct si_shader *shader,
                                         struct si_shader *prev_shader,
                                         struct si_shader_args *args,
                                         bool *free_nir)
{
   const struct si_shader_selector *sel = shader->selector;
   const union si_shader_key *key = &shader->key;

   if (sel->stage == MESA_SHADER_TESS_CTRL) {
      struct si_shader_selector *ls = key->ge.part.tcs.ls;

      prev_shader->selector = ls;
      prev_shader->key.ge.part.vs.prolog = key->ge.part.tcs.ls_prolog;
      prev_shader->key.ge.as_ls = 1;
   } else {
      struct si_shader_selector *es = key->ge.part.gs.es;

      prev_shader->selector = es;
      prev_shader->key.ge.part.vs.prolog = key->ge.part.gs.vs_prolog;
      prev_shader->key.ge.as_es = 1;
      prev_shader->key.ge.as_ngg = key->ge.as_ngg;
   }

   prev_shader->key.ge.mono = key->ge.mono;
   prev_shader->key.ge.opt = key->ge.opt;
   prev_shader->key.ge.opt.inline_uniforms = false; /* only TCS/GS can inline uniforms */
   /* kill_outputs was computed based on second shader's outputs so we can't use it to
    * kill first shader's outputs.
    */
   prev_shader->key.ge.opt.kill_outputs = 0;
   prev_shader->is_monolithic = true;

   si_init_shader_args(prev_shader, args);

   nir_shader *nir = si_get_nir_shader(prev_shader, args, free_nir,
                                       sel->info.tcs_vgpr_only_inputs, NULL);

   si_update_shader_binary_info(shader, nir);

   shader->info.uses_instanceid |=
      prev_shader->selector->info.uses_instanceid || prev_shader->info.uses_instanceid;

   return nir;
}

unsigned si_get_tcs_out_patch_stride(const struct si_shader_info *info)
{
   unsigned tcs_out_vertices = info->base.tess.tcs_vertices_out;
   unsigned vertex_stride = util_last_bit64(info->outputs_written_before_tes_gs) * 4;
   unsigned num_patch_outputs = util_last_bit64(info->patch_outputs_written);

   return tcs_out_vertices * vertex_stride + num_patch_outputs * 4;
}

void si_get_tcs_epilog_args(enum amd_gfx_level gfx_level,
                            struct si_shader_args *args,
                            struct ac_arg *rel_patch_id,
                            struct ac_arg *invocation_id,
                            struct ac_arg *tf_lds_offset,
                            struct ac_arg tess_factors[6])
{
   memset(args, 0, sizeof(*args));

   if (gfx_level >= GFX9) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); /* wave info */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tcs_factor_offset);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tcs_offchip_layout);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tes_offchip_addr);
   } else {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tcs_offchip_layout);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->tes_offchip_addr);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tcs_factor_offset);
   }

   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* VGPR gap */
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* VGPR gap */
   /* patch index within the wave (REL_PATCH_ID) */
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, rel_patch_id);
   /* invocation ID within the patch */
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, invocation_id);
   /* LDS offset where tess factors should be loaded from */
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, tf_lds_offset);

   for (unsigned i = 0; i < 6; i++)
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &tess_factors[i]);
}

void si_get_vs_prolog_args(enum amd_gfx_level gfx_level,
                           struct si_shader_args *args,
                           const union si_shader_part_key *key)
{
   memset(args, 0, sizeof(*args));

   unsigned num_input_sgprs = key->vs_prolog.num_input_sgprs;
   unsigned num_input_vgprs = key->vs_prolog.num_merged_next_stage_vgprs + 4;

   struct ac_arg input_sgprs[num_input_sgprs];
   for (unsigned i = 0; i < num_input_sgprs; i++)
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, input_sgprs + i);

   struct ac_arg input_vgprs[num_input_vgprs];
   for (unsigned i = 0; i < num_input_vgprs; i++)
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, input_vgprs + i);

   if (key->vs_prolog.num_merged_next_stage_vgprs)
      args->ac.merged_wave_info = input_sgprs[3];

   unsigned first_vs_vgpr = key->vs_prolog.num_merged_next_stage_vgprs;
   unsigned vertex_id_vgpr = first_vs_vgpr;
   unsigned instance_id_vgpr = gfx_level >= GFX10 ?
      first_vs_vgpr + 3 : first_vs_vgpr + (key->vs_prolog.as_ls ? 2 : 1);

   args->ac.vertex_id = input_vgprs[vertex_id_vgpr];
   args->ac.instance_id = input_vgprs[instance_id_vgpr];

   if (key->vs_prolog.as_ls) {
      if (gfx_level < GFX11)
         args->ac.vs_rel_patch_id = input_vgprs[first_vs_vgpr + 1];

      if (gfx_level >= GFX9) {
         args->ac.tcs_patch_id = input_vgprs[0];
         args->ac.tcs_rel_ids = input_vgprs[1];
      }
   }

   unsigned user_sgpr_base = key->vs_prolog.num_merged_next_stage_vgprs ? 8 : 0;
   args->internal_bindings = input_sgprs[user_sgpr_base + SI_SGPR_INTERNAL_BINDINGS];
   args->ac.start_instance = input_sgprs[user_sgpr_base + SI_SGPR_START_INSTANCE];
   args->ac.base_vertex = input_sgprs[user_sgpr_base + SI_SGPR_BASE_VERTEX];
}

void si_get_ps_prolog_args(struct si_shader_args *args,
                           const union si_shader_part_key *key)
{
   memset(args, 0, sizeof(*args));

   const unsigned num_input_sgprs = key->ps_prolog.num_input_sgprs;

   struct ac_arg input_sgprs[num_input_sgprs];
   for (unsigned i = 0; i < num_input_sgprs; i++)
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, input_sgprs + i);

   args->internal_bindings = input_sgprs[SI_SGPR_INTERNAL_BINDINGS];
   /* Use the absolute location of the input. */
   args->ac.prim_mask = input_sgprs[SI_PS_NUM_USER_SGPR];

   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_FLOAT, &args->ac.persp_sample);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_FLOAT, &args->ac.persp_center);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_FLOAT, &args->ac.persp_centroid);
   /* skip PERSP_PULL_MODEL */
   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_FLOAT, &args->ac.linear_sample);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_FLOAT, &args->ac.linear_center);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_FLOAT, &args->ac.linear_centroid);
   /* skip LINE_STIPPLE_TEX */

   /* POS_X|Y|Z|W_FLOAT */
   for (unsigned i = 0; i < key->ps_prolog.num_fragcoord_components; i++)
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, NULL);

   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.front_face);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.ancillary);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.sample_coverage);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.pos_fixed_pt);
}

void si_get_ps_epilog_args(struct si_shader_args *args,
                           const union si_shader_part_key *key,
                           struct ac_arg colors[MAX_DRAW_BUFFERS],
                           struct ac_arg *depth, struct ac_arg *stencil,
                           struct ac_arg *sample_mask)
{
   memset(args, 0, sizeof(*args));

   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_FLOAT, &args->alpha_reference);

   u_foreach_bit (i, key->ps_epilog.colors_written) {
      ac_add_arg(&args->ac, AC_ARG_VGPR, 4, AC_ARG_FLOAT, colors + i);
   }

   if (key->ps_epilog.writes_z)
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, depth);

   if (key->ps_epilog.writes_stencil)
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, stencil);

   if (key->ps_epilog.writes_samplemask)
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, sample_mask);
}
