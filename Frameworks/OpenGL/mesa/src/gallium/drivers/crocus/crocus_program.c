/*
 * Copyright © 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file crocus_program.c
 *
 * This file contains the driver interface for compiling shaders.
 *
 * See crocus_program_cache.c for the in-memory program cache where the
 * compiled shaders are stored.
 */

#include <stdio.h>
#include <errno.h>
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/u_atomic.h"
#include "util/u_upload_mgr.h"
#include "util/u_debug.h"
#include "util/u_prim.h"
#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_serialize.h"
#include "intel/compiler/brw_compiler.h"
#include "intel/compiler/brw_nir.h"
#include "intel/compiler/brw_prim.h"
#include "crocus_context.h"
#include "nir/tgsi_to_nir.h"
#include "program/prog_instruction.h"

#define KEY_INIT_NO_ID()                              \
   .base.tex.swizzles[0 ... BRW_MAX_SAMPLERS - 1] = 0x688
#define KEY_INIT()                                                        \
   .base.program_string_id = ish->program_id,                             \
   .base.limit_trig_input_range = screen->driconf.limit_trig_input_range, \
   KEY_INIT_NO_ID()

static void
crocus_sanitize_tex_key(struct brw_sampler_prog_key_data *key)
{
   key->gather_channel_quirk_mask = 0;
   for (unsigned s = 0; s < BRW_MAX_SAMPLERS; s++) {
      key->swizzles[s] = SWIZZLE_NOOP;
      key->gfx6_gather_wa[s] = 0;
   }
}

static uint32_t
crocus_get_texture_swizzle(const struct crocus_context *ice,
                           const struct crocus_sampler_view *t)
{
   uint32_t swiz = 0;

   for (int i = 0; i < 4; i++) {
      swiz |= t->swizzle[i] << (i * 3);
   }
   return swiz;
}

static inline bool can_push_ubo(const struct intel_device_info *devinfo)
{
   /* push works for everyone except SNB at the moment */
   return devinfo->ver != 6;
}

static uint8_t
gfx6_gather_workaround(enum pipe_format pformat)
{
   switch (pformat) {
   case PIPE_FORMAT_R8_SINT: return WA_SIGN | WA_8BIT;
   case PIPE_FORMAT_R8_UINT: return WA_8BIT;
   case PIPE_FORMAT_R16_SINT: return WA_SIGN | WA_16BIT;
   case PIPE_FORMAT_R16_UINT: return WA_16BIT;
   default:
      /* Note that even though PIPE_FORMAT_R32_SINT and
       * PIPE_FORMAT_R32_UINThave format overrides in
       * the surface state, there is no shader w/a required.
       */
      return 0;
   }
}

static const unsigned crocus_gfx6_swizzle_for_offset[4] = {
   BRW_SWIZZLE4(0, 1, 2, 3),
   BRW_SWIZZLE4(1, 2, 3, 3),
   BRW_SWIZZLE4(2, 3, 3, 3),
   BRW_SWIZZLE4(3, 3, 3, 3)
};

static void
gfx6_gs_xfb_setup(const struct pipe_stream_output_info *so_info,
                  struct brw_gs_prog_data *gs_prog_data)
{
   /* Make sure that the VUE slots won't overflow the unsigned chars in
    * prog_data->transform_feedback_bindings[].
    */
   STATIC_ASSERT(BRW_VARYING_SLOT_COUNT <= 256);

   /* Make sure that we don't need more binding table entries than we've
    * set aside for use in transform feedback.  (We shouldn't, since we
    * set aside enough binding table entries to have one per component).
    */
   assert(so_info->num_outputs <= BRW_MAX_SOL_BINDINGS);

   gs_prog_data->num_transform_feedback_bindings = so_info->num_outputs;
   for (unsigned i = 0; i < so_info->num_outputs; i++) {
      gs_prog_data->transform_feedback_bindings[i] =
         so_info->output[i].register_index;
      gs_prog_data->transform_feedback_swizzles[i] =
         crocus_gfx6_swizzle_for_offset[so_info->output[i].start_component];
   }
}

static void
gfx6_ff_gs_xfb_setup(const struct pipe_stream_output_info *so_info,
                     struct brw_ff_gs_prog_key *key)
{
   key->num_transform_feedback_bindings = so_info->num_outputs;
   for (unsigned i = 0; i < so_info->num_outputs; i++) {
      key->transform_feedback_bindings[i] =
         so_info->output[i].register_index;
      key->transform_feedback_swizzles[i] =
         crocus_gfx6_swizzle_for_offset[so_info->output[i].start_component];
   }
}

static void
crocus_populate_sampler_prog_key_data(struct crocus_context *ice,
                                      const struct intel_device_info *devinfo,
                                      gl_shader_stage stage,
                                      struct crocus_uncompiled_shader *ish,
                                      bool uses_texture_gather,
                                      struct brw_sampler_prog_key_data *key)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   uint32_t mask = ish->nir->info.textures_used[0];

   while (mask) {
      const int s = u_bit_scan(&mask);

      struct crocus_sampler_view *texture = ice->state.shaders[stage].textures[s];
      key->swizzles[s] = SWIZZLE_NOOP;

      if (!texture)
         continue;
      if (texture->base.target == PIPE_BUFFER)
         continue;
      if (devinfo->verx10 < 75) {
         key->swizzles[s] = crocus_get_texture_swizzle(ice, texture);
      }

      screen->vtbl.fill_clamp_mask(ice->state.shaders[stage].samplers[s], s, key->gl_clamp_mask);

      /* gather4 for RG32* is broken in multiple ways on Gen7. */
      if (devinfo->ver == 7 && uses_texture_gather) {
         switch (texture->base.format) {
         case PIPE_FORMAT_R32G32_UINT:
         case PIPE_FORMAT_R32G32_SINT: {
            /* We have to override the format to R32G32_FLOAT_LD.
             * This means that SCS_ALPHA and SCS_ONE will return 0x3f8
             * (1.0) rather than integer 1.  This needs shader hacks.
             *
             * On Ivybridge, we whack W (alpha) to ONE in our key's
             * swizzle.  On Haswell, we look at the original texture
             * swizzle, and use XYZW with channels overridden to ONE,
             * leaving normal texture swizzling to SCS.
             */
            unsigned src_swizzle = key->swizzles[s];
            for (int i = 0; i < 4; i++) {
               unsigned src_comp = GET_SWZ(src_swizzle, i);
               if (src_comp == SWIZZLE_ONE || src_comp == SWIZZLE_W) {
                  key->swizzles[i] &= ~(0x7 << (3 * i));
                  key->swizzles[i] |= SWIZZLE_ONE << (3 * i);
               }
            }
         }
         FALLTHROUGH;
         case PIPE_FORMAT_R32G32_FLOAT:
            /* The channel select for green doesn't work - we have to
             * request blue.  Haswell can use SCS for this, but Ivybridge
             * needs a shader workaround.
             */
            if (devinfo->verx10 < 75)
               key->gather_channel_quirk_mask |= 1 << s;
            break;
         default:
            break;
         }
      }
      if (devinfo->ver == 6 && uses_texture_gather) {
         key->gfx6_gather_wa[s] = gfx6_gather_workaround(texture->base.format);
      }
   }
}

static void
crocus_lower_swizzles(struct nir_shader *nir,
                      const struct brw_sampler_prog_key_data *key_tex)
{
   struct nir_lower_tex_options tex_options = {
      .lower_invalid_implicit_lod = true,
   };
   uint32_t mask = nir->info.textures_used[0];

   while (mask) {
      const int s = u_bit_scan(&mask);

      if (key_tex->swizzles[s] == SWIZZLE_NOOP)
         continue;

      tex_options.swizzle_result |= (1 << s);
      for (unsigned c = 0; c < 4; c++)
         tex_options.swizzles[s][c] = GET_SWZ(key_tex->swizzles[s], c);
   }
   if (tex_options.swizzle_result)
      nir_lower_tex(nir, &tex_options);
}

static unsigned
get_new_program_id(struct crocus_screen *screen)
{
   return p_atomic_inc_return(&screen->program_id);
}

static nir_def *
get_aoa_deref_offset(nir_builder *b,
                     nir_deref_instr *deref,
                     unsigned elem_size)
{
   unsigned array_size = elem_size;
   nir_def *offset = nir_imm_int(b, 0);

   while (deref->deref_type != nir_deref_type_var) {
      assert(deref->deref_type == nir_deref_type_array);

      /* This level's element size is the previous level's array size */
      nir_def *index = deref->arr.index.ssa;
      assert(deref->arr.index.ssa);
      offset = nir_iadd(b, offset,
                        nir_imul_imm(b, index, array_size));

      deref = nir_deref_instr_parent(deref);
      assert(glsl_type_is_array(deref->type));
      array_size *= glsl_get_length(deref->type);
   }

   /* Accessing an invalid surface index with the dataport can result in a
    * hang.  According to the spec "if the index used to select an individual
    * element is negative or greater than or equal to the size of the array,
    * the results of the operation are undefined but may not lead to
    * termination" -- which is one of the possible outcomes of the hang.
    * Clamp the index to prevent access outside of the array bounds.
    */
   return nir_umin(b, offset, nir_imm_int(b, array_size - elem_size));
}

static void
crocus_lower_storage_image_derefs(nir_shader *nir)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   nir_builder b = nir_builder_create(impl);

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         switch (intrin->intrinsic) {
         case nir_intrinsic_image_deref_load:
         case nir_intrinsic_image_deref_store:
         case nir_intrinsic_image_deref_atomic:
         case nir_intrinsic_image_deref_atomic_swap:
         case nir_intrinsic_image_deref_size:
         case nir_intrinsic_image_deref_samples:
         case nir_intrinsic_image_deref_load_raw_intel:
         case nir_intrinsic_image_deref_store_raw_intel: {
            nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
            nir_variable *var = nir_deref_instr_get_variable(deref);

            b.cursor = nir_before_instr(&intrin->instr);
            nir_def *index =
               nir_iadd_imm(&b, get_aoa_deref_offset(&b, deref, 1),
                            var->data.driver_location);
            nir_rewrite_image_intrinsic(intrin, index, false);
            break;
         }

         default:
            break;
         }
      }
   }
}

// XXX: need unify_interfaces() at link time...

/**
 * Undo nir_lower_passthrough_edgeflags but keep the inputs_read flag.
 */
static bool
crocus_fix_edge_flags(nir_shader *nir)
{
   if (nir->info.stage != MESA_SHADER_VERTEX) {
      nir_shader_preserve_all_metadata(nir);
      return false;
   }

   nir_variable *var = nir_find_variable_with_location(nir, nir_var_shader_out,
                                                       VARYING_SLOT_EDGE);
   if (!var) {
      nir_shader_preserve_all_metadata(nir);
      return false;
   }

   var->data.mode = nir_var_shader_temp;
   nir->info.outputs_written &= ~VARYING_BIT_EDGE;
   nir->info.inputs_read &= ~VERT_BIT_EDGEFLAG;
   nir_fixup_deref_modes(nir);

   nir_foreach_function_impl(impl, nir) {
      nir_metadata_preserve(impl, nir_metadata_block_index |
                            nir_metadata_dominance |
                            nir_metadata_live_defs |
                            nir_metadata_loop_analysis);
   }

   return true;
}

/**
 * Fix an uncompiled shader's stream output info.
 *
 * Core Gallium stores output->register_index as a "slot" number, where
 * slots are assigned consecutively to all outputs in info->outputs_written.
 * This naive packing of outputs doesn't work for us - we too have slots,
 * but the layout is defined by the VUE map, which we won't have until we
 * compile a specific shader variant.  So, we remap these and simply store
 * VARYING_SLOT_* in our copy's output->register_index fields.
 *
 * We also fix up VARYING_SLOT_{LAYER,VIEWPORT,PSIZ} to select the Y/Z/W
 * components of our VUE header.  See brw_vue_map.c for the layout.
 */
static void
update_so_info(struct pipe_stream_output_info *so_info,
               uint64_t outputs_written)
{
   uint8_t reverse_map[64] = {};
   unsigned slot = 0;
   while (outputs_written) {
      reverse_map[slot++] = u_bit_scan64(&outputs_written);
   }

   for (unsigned i = 0; i < so_info->num_outputs; i++) {
      struct pipe_stream_output *output = &so_info->output[i];

      /* Map Gallium's condensed "slots" back to real VARYING_SLOT_* enums */
      output->register_index = reverse_map[output->register_index];

      /* The VUE header contains three scalar fields packed together:
       * - gl_PointSize is stored in VARYING_SLOT_PSIZ.w
       * - gl_Layer is stored in VARYING_SLOT_PSIZ.y
       * - gl_ViewportIndex is stored in VARYING_SLOT_PSIZ.z
       */
      switch (output->register_index) {
      case VARYING_SLOT_LAYER:
         assert(output->num_components == 1);
         output->register_index = VARYING_SLOT_PSIZ;
         output->start_component = 1;
         break;
      case VARYING_SLOT_VIEWPORT:
         assert(output->num_components == 1);
         output->register_index = VARYING_SLOT_PSIZ;
         output->start_component = 2;
         break;
      case VARYING_SLOT_PSIZ:
         assert(output->num_components == 1);
         output->start_component = 3;
         break;
      }

      //info->outputs_written |= 1ull << output->register_index;
   }
}

static void
setup_vec4_image_sysval(uint32_t *sysvals, uint32_t idx,
                        unsigned offset, unsigned n)
{
   assert(offset % sizeof(uint32_t) == 0);

   for (unsigned i = 0; i < n; ++i)
      sysvals[i] = BRW_PARAM_IMAGE(idx, offset / sizeof(uint32_t) + i);

   for (unsigned i = n; i < 4; ++i)
      sysvals[i] = BRW_PARAM_BUILTIN_ZERO;
}

/**
 * Associate NIR uniform variables with the prog_data->param[] mechanism
 * used by the backend.  Also, decide which UBOs we'd like to push in an
 * ideal situation (though the backend can reduce this).
 */
static void
crocus_setup_uniforms(ASSERTED const struct intel_device_info *devinfo,
                      void *mem_ctx,
                      nir_shader *nir,
                      struct brw_stage_prog_data *prog_data,
                      enum brw_param_builtin **out_system_values,
                      unsigned *out_num_system_values,
                      unsigned *out_num_cbufs)
{
   const unsigned CROCUS_MAX_SYSTEM_VALUES =
      PIPE_MAX_SHADER_IMAGES * BRW_IMAGE_PARAM_SIZE;
   enum brw_param_builtin *system_values =
      rzalloc_array(mem_ctx, enum brw_param_builtin, CROCUS_MAX_SYSTEM_VALUES);
   unsigned num_system_values = 0;

   unsigned patch_vert_idx = -1;
   unsigned tess_outer_default_idx = -1;
   unsigned tess_inner_default_idx = -1;
   unsigned ucp_idx[CROCUS_MAX_CLIP_PLANES];
   unsigned img_idx[PIPE_MAX_SHADER_IMAGES];
   unsigned variable_group_size_idx = -1;
   memset(ucp_idx, -1, sizeof(ucp_idx));
   memset(img_idx, -1, sizeof(img_idx));

   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   nir_builder b = nir_builder_at(nir_before_impl(impl));

   nir_def *temp_ubo_name = nir_undef(&b, 1, 32);
   nir_def *temp_const_ubo_name = NULL;

   /* Turn system value intrinsics into uniforms */
   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         nir_def *offset;

         switch (intrin->intrinsic) {
         case nir_intrinsic_load_base_workgroup_id: {
            /* GL doesn't have a concept of base workgroup */
            b.cursor = nir_instr_remove(&intrin->instr);
            nir_def_rewrite_uses(&intrin->def,
                                     nir_imm_zero(&b, 3, 32));
            continue;
         }
         case nir_intrinsic_load_constant: {
            /* This one is special because it reads from the shader constant
             * data and not cbuf0 which gallium uploads for us.
             */
            b.cursor = nir_before_instr(instr);
            nir_def *offset =
               nir_iadd_imm(&b, intrin->src[0].ssa,
                            nir_intrinsic_base(intrin));

            if (temp_const_ubo_name == NULL)
               temp_const_ubo_name = nir_imm_int(&b, 0);

            nir_intrinsic_instr *load_ubo =
               nir_intrinsic_instr_create(b.shader, nir_intrinsic_load_ubo);
            load_ubo->num_components = intrin->num_components;
            load_ubo->src[0] = nir_src_for_ssa(temp_const_ubo_name);
            load_ubo->src[1] = nir_src_for_ssa(offset);
            nir_intrinsic_set_align(load_ubo, 4, 0);
            nir_intrinsic_set_range_base(load_ubo, 0);
            nir_intrinsic_set_range(load_ubo, ~0);
            nir_def_init(&load_ubo->instr, &load_ubo->def,
                         intrin->def.num_components,
                         intrin->def.bit_size);
            nir_builder_instr_insert(&b, &load_ubo->instr);

            nir_def_rewrite_uses(&intrin->def,
                                     &load_ubo->def);
            nir_instr_remove(&intrin->instr);
            continue;
         }
         case nir_intrinsic_load_user_clip_plane: {
            unsigned ucp = nir_intrinsic_ucp_id(intrin);

            if (ucp_idx[ucp] == -1) {
               ucp_idx[ucp] = num_system_values;
               num_system_values += 4;
            }

            for (int i = 0; i < 4; i++) {
               system_values[ucp_idx[ucp] + i] =
                  BRW_PARAM_BUILTIN_CLIP_PLANE(ucp, i);
            }

            b.cursor = nir_before_instr(instr);
            offset = nir_imm_int(&b, ucp_idx[ucp] * sizeof(uint32_t));
            break;
         }
         case nir_intrinsic_load_patch_vertices_in:
            if (patch_vert_idx == -1)
               patch_vert_idx = num_system_values++;

            system_values[patch_vert_idx] =
               BRW_PARAM_BUILTIN_PATCH_VERTICES_IN;

            b.cursor = nir_before_instr(instr);
            offset = nir_imm_int(&b, patch_vert_idx * sizeof(uint32_t));
            break;
         case nir_intrinsic_load_tess_level_outer_default:
            if (tess_outer_default_idx == -1) {
               tess_outer_default_idx = num_system_values;
               num_system_values += 4;
            }

            for (int i = 0; i < 4; i++) {
               system_values[tess_outer_default_idx + i] =
                  BRW_PARAM_BUILTIN_TESS_LEVEL_OUTER_X + i;
            }

            b.cursor = nir_before_instr(instr);
            offset =
               nir_imm_int(&b, tess_outer_default_idx * sizeof(uint32_t));
            break;
         case nir_intrinsic_load_tess_level_inner_default:
            if (tess_inner_default_idx == -1) {
               tess_inner_default_idx = num_system_values;
               num_system_values += 2;
            }

            for (int i = 0; i < 2; i++) {
               system_values[tess_inner_default_idx + i] =
                  BRW_PARAM_BUILTIN_TESS_LEVEL_INNER_X + i;
            }

            b.cursor = nir_before_instr(instr);
            offset =
               nir_imm_int(&b, tess_inner_default_idx * sizeof(uint32_t));
            break;
         case nir_intrinsic_image_deref_load_param_intel: {
            assert(devinfo->ver < 9);
            nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
            nir_variable *var = nir_deref_instr_get_variable(deref);

            if (img_idx[var->data.binding] == -1) {
               /* GL only allows arrays of arrays of images. */
               assert(glsl_type_is_image(glsl_without_array(var->type)));
               unsigned num_images = MAX2(1, glsl_get_aoa_size(var->type));

               for (int i = 0; i < num_images; i++) {
                  const unsigned img = var->data.binding + i;

                  img_idx[img] = num_system_values;
                  num_system_values += BRW_IMAGE_PARAM_SIZE;

                  uint32_t *img_sv = &system_values[img_idx[img]];

                  setup_vec4_image_sysval(
                     img_sv + BRW_IMAGE_PARAM_OFFSET_OFFSET, img,
                     offsetof(struct brw_image_param, offset), 2);
                  setup_vec4_image_sysval(
                     img_sv + BRW_IMAGE_PARAM_SIZE_OFFSET, img,
                     offsetof(struct brw_image_param, size), 3);
                  setup_vec4_image_sysval(
                     img_sv + BRW_IMAGE_PARAM_STRIDE_OFFSET, img,
                     offsetof(struct brw_image_param, stride), 4);
                  setup_vec4_image_sysval(
                     img_sv + BRW_IMAGE_PARAM_TILING_OFFSET, img,
                     offsetof(struct brw_image_param, tiling), 3);
                  setup_vec4_image_sysval(
                     img_sv + BRW_IMAGE_PARAM_SWIZZLING_OFFSET, img,
                     offsetof(struct brw_image_param, swizzling), 2);
               }
            }

            b.cursor = nir_before_instr(instr);
            offset = nir_iadd_imm(&b,
                                  get_aoa_deref_offset(&b, deref, BRW_IMAGE_PARAM_SIZE * 4),
                                  img_idx[var->data.binding] * 4 +
                                  nir_intrinsic_base(intrin) * 16);
            break;
         }
         case nir_intrinsic_load_workgroup_size: {
            assert(nir->info.workgroup_size_variable);
            if (variable_group_size_idx == -1) {
               variable_group_size_idx = num_system_values;
               num_system_values += 3;
               for (int i = 0; i < 3; i++) {
                  system_values[variable_group_size_idx + i] =
                     BRW_PARAM_BUILTIN_WORK_GROUP_SIZE_X + i;
               }
            }

            b.cursor = nir_before_instr(instr);
            offset = nir_imm_int(&b,
                                 variable_group_size_idx * sizeof(uint32_t));
            break;
         }
         default:
            continue;
         }

         unsigned comps = nir_intrinsic_dest_components(intrin);

         nir_intrinsic_instr *load =
            nir_intrinsic_instr_create(nir, nir_intrinsic_load_ubo);
         load->num_components = comps;
         load->src[0] = nir_src_for_ssa(temp_ubo_name);
         load->src[1] = nir_src_for_ssa(offset);
         nir_intrinsic_set_align(load, 4, 0);
         nir_intrinsic_set_range_base(load, 0);
         nir_intrinsic_set_range(load, ~0);
         nir_def_init(&load->instr, &load->def, comps, 32);
         nir_builder_instr_insert(&b, &load->instr);
         nir_def_rewrite_uses(&intrin->def,
                                  &load->def);
         nir_instr_remove(instr);
      }
   }

   nir_validate_shader(nir, "before remapping");

   /* Uniforms are stored in constant buffer 0, the
    * user-facing UBOs are indexed by one.  So if any constant buffer is
    * needed, the constant buffer 0 will be needed, so account for it.
    */
   unsigned num_cbufs = nir->info.num_ubos;
   if (num_cbufs || nir->num_uniforms)
      num_cbufs++;

   /* Place the new params in a new cbuf. */
   if (num_system_values > 0) {
      unsigned sysval_cbuf_index = num_cbufs;
      num_cbufs++;

      system_values = reralloc(mem_ctx, system_values, enum brw_param_builtin,
                               num_system_values);

      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *load = nir_instr_as_intrinsic(instr);

            if (load->intrinsic != nir_intrinsic_load_ubo)
               continue;

            b.cursor = nir_before_instr(instr);

            if (load->src[0].ssa == temp_ubo_name) {
               nir_def *imm = nir_imm_int(&b, sysval_cbuf_index);
               nir_src_rewrite(&load->src[0], imm);
            }
         }
      }

      /* We need to fold the new iadds for brw_nir_analyze_ubo_ranges */
      nir_opt_constant_folding(nir);
   } else {
      ralloc_free(system_values);
      system_values = NULL;
   }

   assert(num_cbufs < PIPE_MAX_CONSTANT_BUFFERS);
   nir_validate_shader(nir, "after remap");

   /* We don't use params[] but gallium leaves num_uniforms set.  We use this
    * to detect when cbuf0 exists but we don't need it anymore when we get
    * here.  Instead, zero it out so that the back-end doesn't get confused
    * when nr_params * 4 != num_uniforms != nr_params * 4.
    */
   nir->num_uniforms = 0;

   /* Constant loads (if any) need to go at the end of the constant buffers so
    * we need to know num_cbufs before we can lower to them.
    */
   if (temp_const_ubo_name != NULL) {
      nir_load_const_instr *const_ubo_index =
         nir_instr_as_load_const(temp_const_ubo_name->parent_instr);
      assert(const_ubo_index->def.bit_size == 32);
      const_ubo_index->value[0].u32 = num_cbufs;
   }

   *out_system_values = system_values;
   *out_num_system_values = num_system_values;
   *out_num_cbufs = num_cbufs;
}

static const char *surface_group_names[] = {
   [CROCUS_SURFACE_GROUP_RENDER_TARGET]      = "render target",
   [CROCUS_SURFACE_GROUP_RENDER_TARGET_READ] = "non-coherent render target read",
   [CROCUS_SURFACE_GROUP_SOL]                = "streamout",
   [CROCUS_SURFACE_GROUP_CS_WORK_GROUPS]     = "CS work groups",
   [CROCUS_SURFACE_GROUP_TEXTURE]            = "texture",
   [CROCUS_SURFACE_GROUP_TEXTURE_GATHER]     = "texture gather",
   [CROCUS_SURFACE_GROUP_UBO]                = "ubo",
   [CROCUS_SURFACE_GROUP_SSBO]               = "ssbo",
   [CROCUS_SURFACE_GROUP_IMAGE]              = "image",
};

static void
crocus_print_binding_table(FILE *fp, const char *name,
                           const struct crocus_binding_table *bt)
{
   STATIC_ASSERT(ARRAY_SIZE(surface_group_names) == CROCUS_SURFACE_GROUP_COUNT);

   uint32_t total = 0;
   uint32_t compacted = 0;

   for (int i = 0; i < CROCUS_SURFACE_GROUP_COUNT; i++) {
      uint32_t size = bt->sizes[i];
      total += size;
      if (size)
         compacted += util_bitcount64(bt->used_mask[i]);
   }

   if (total == 0) {
      fprintf(fp, "Binding table for %s is empty\n\n", name);
      return;
   }

   if (total != compacted) {
      fprintf(fp, "Binding table for %s "
              "(compacted to %u entries from %u entries)\n",
              name, compacted, total);
   } else {
      fprintf(fp, "Binding table for %s (%u entries)\n", name, total);
   }

   uint32_t entry = 0;
   for (int i = 0; i < CROCUS_SURFACE_GROUP_COUNT; i++) {
      uint64_t mask = bt->used_mask[i];
      while (mask) {
         int index = u_bit_scan64(&mask);
         fprintf(fp, "  [%u] %s #%d\n", entry++, surface_group_names[i], index);
      }
   }
   fprintf(fp, "\n");
}

enum {
   /* Max elements in a surface group. */
   SURFACE_GROUP_MAX_ELEMENTS = 64,
};

static void
rewrite_src_with_bti(nir_builder *b, struct crocus_binding_table *bt,
                     nir_instr *instr, nir_src *src,
                     enum crocus_surface_group group)
{
   assert(bt->sizes[group] > 0);

   b->cursor = nir_before_instr(instr);
   nir_def *bti;
   if (nir_src_is_const(*src)) {
      uint32_t index = nir_src_as_uint(*src);
      bti = nir_imm_intN_t(b, crocus_group_index_to_bti(bt, group, index),
                           src->ssa->bit_size);
   } else {
      /* Indirect usage makes all the surfaces of the group to be available,
       * so we can just add the base.
       */
      assert(bt->used_mask[group] == BITFIELD64_MASK(bt->sizes[group]));
      bti = nir_iadd_imm(b, src->ssa, bt->offsets[group]);
   }
   nir_src_rewrite(src, bti);
}

static void
mark_used_with_src(struct crocus_binding_table *bt, nir_src *src,
                   enum crocus_surface_group group)
{
   assert(bt->sizes[group] > 0);

   if (nir_src_is_const(*src)) {
      uint64_t index = nir_src_as_uint(*src);
      assert(index < bt->sizes[group]);
      bt->used_mask[group] |= 1ull << index;
   } else {
      /* There's an indirect usage, we need all the surfaces. */
      bt->used_mask[group] = BITFIELD64_MASK(bt->sizes[group]);
   }
}

static bool
skip_compacting_binding_tables(void)
{
   static int skip = -1;
   if (skip < 0)
      skip = debug_get_bool_option("INTEL_DISABLE_COMPACT_BINDING_TABLE", false);
   return skip;
}

/**
 * Set up the binding table indices and apply to the shader.
 */
static void
crocus_setup_binding_table(const struct intel_device_info *devinfo,
                           struct nir_shader *nir,
                           struct crocus_binding_table *bt,
                           unsigned num_render_targets,
                           unsigned num_system_values,
                           unsigned num_cbufs,
                           const struct brw_sampler_prog_key_data *key)
{
   const struct shader_info *info = &nir->info;

   memset(bt, 0, sizeof(*bt));

   /* Set the sizes for each surface group.  For some groups, we already know
    * upfront how many will be used, so mark them.
    */
   if (info->stage == MESA_SHADER_FRAGMENT) {
      bt->sizes[CROCUS_SURFACE_GROUP_RENDER_TARGET] = num_render_targets;
      /* All render targets used. */
      bt->used_mask[CROCUS_SURFACE_GROUP_RENDER_TARGET] =
         BITFIELD64_MASK(num_render_targets);

      /* Setup render target read surface group in order to support non-coherent
       * framebuffer fetch on Gfx7
       */
      if (devinfo->ver >= 6 && info->outputs_read) {
         bt->sizes[CROCUS_SURFACE_GROUP_RENDER_TARGET_READ] = num_render_targets;
         bt->used_mask[CROCUS_SURFACE_GROUP_RENDER_TARGET_READ] =
            BITFIELD64_MASK(num_render_targets);
      }
   } else if (info->stage == MESA_SHADER_COMPUTE) {
      bt->sizes[CROCUS_SURFACE_GROUP_CS_WORK_GROUPS] = 1;
   } else if (info->stage == MESA_SHADER_GEOMETRY) {
      /* In gfx6 we reserve the first BRW_MAX_SOL_BINDINGS entries for transform
       * feedback surfaces.
       */
      if (devinfo->ver == 6) {
         bt->sizes[CROCUS_SURFACE_GROUP_SOL] = BRW_MAX_SOL_BINDINGS;
         bt->used_mask[CROCUS_SURFACE_GROUP_SOL] = (uint64_t)-1;
      }
   }

   bt->sizes[CROCUS_SURFACE_GROUP_TEXTURE] = BITSET_LAST_BIT(info->textures_used);
   bt->used_mask[CROCUS_SURFACE_GROUP_TEXTURE] = info->textures_used[0];

   if (info->uses_texture_gather && devinfo->ver < 8) {
      bt->sizes[CROCUS_SURFACE_GROUP_TEXTURE_GATHER] = BITSET_LAST_BIT(info->textures_used);
      bt->used_mask[CROCUS_SURFACE_GROUP_TEXTURE_GATHER] = info->textures_used[0];
   }

   bt->sizes[CROCUS_SURFACE_GROUP_IMAGE] = info->num_images;

   /* Allocate an extra slot in the UBO section for NIR constants.
    * Binding table compaction will remove it if unnecessary.
    *
    * We don't include them in crocus_compiled_shader::num_cbufs because
    * they are uploaded separately from shs->constbufs[], but from a shader
    * point of view, they're another UBO (at the end of the section).
    */
   bt->sizes[CROCUS_SURFACE_GROUP_UBO] = num_cbufs + 1;

   bt->sizes[CROCUS_SURFACE_GROUP_SSBO] = info->num_ssbos;

   for (int i = 0; i < CROCUS_SURFACE_GROUP_COUNT; i++)
      assert(bt->sizes[i] <= SURFACE_GROUP_MAX_ELEMENTS);

   /* Mark surfaces used for the cases we don't have the information available
    * upfront.
    */
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   nir_foreach_block (block, impl) {
      nir_foreach_instr (instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         switch (intrin->intrinsic) {
         case nir_intrinsic_load_num_workgroups:
            bt->used_mask[CROCUS_SURFACE_GROUP_CS_WORK_GROUPS] = 1;
            break;

         case nir_intrinsic_load_output:
            if (devinfo->ver >= 6) {
               mark_used_with_src(bt, &intrin->src[0],
                                  CROCUS_SURFACE_GROUP_RENDER_TARGET_READ);
            }
            break;

         case nir_intrinsic_image_size:
         case nir_intrinsic_image_load:
         case nir_intrinsic_image_store:
         case nir_intrinsic_image_atomic:
         case nir_intrinsic_image_atomic_swap:
         case nir_intrinsic_image_load_raw_intel:
         case nir_intrinsic_image_store_raw_intel:
            mark_used_with_src(bt, &intrin->src[0], CROCUS_SURFACE_GROUP_IMAGE);
            break;

         case nir_intrinsic_load_ubo:
            mark_used_with_src(bt, &intrin->src[0], CROCUS_SURFACE_GROUP_UBO);
            break;

         case nir_intrinsic_store_ssbo:
            mark_used_with_src(bt, &intrin->src[1], CROCUS_SURFACE_GROUP_SSBO);
            break;

         case nir_intrinsic_get_ssbo_size:
         case nir_intrinsic_ssbo_atomic:
         case nir_intrinsic_ssbo_atomic_swap:
         case nir_intrinsic_load_ssbo:
            mark_used_with_src(bt, &intrin->src[0], CROCUS_SURFACE_GROUP_SSBO);
            break;

         default:
            break;
         }
      }
   }

   /* When disable we just mark everything as used. */
   if (unlikely(skip_compacting_binding_tables())) {
      for (int i = 0; i < CROCUS_SURFACE_GROUP_COUNT; i++)
         bt->used_mask[i] = BITFIELD64_MASK(bt->sizes[i]);
   }

   /* Calculate the offsets and the binding table size based on the used
    * surfaces.  After this point, the functions to go between "group indices"
    * and binding table indices can be used.
    */
   uint32_t next = 0;
   for (int i = 0; i < CROCUS_SURFACE_GROUP_COUNT; i++) {
      if (bt->used_mask[i] != 0) {
         bt->offsets[i] = next;
         next += util_bitcount64(bt->used_mask[i]);
      }
   }
   bt->size_bytes = next * 4;

   if (INTEL_DEBUG(DEBUG_BT)) {
      crocus_print_binding_table(stderr, gl_shader_stage_name(info->stage), bt);
   }

   /* Apply the binding table indices.  The backend compiler is not expected
    * to change those, as we haven't set any of the *_start entries in brw
    * binding_table.
    */
   nir_builder b = nir_builder_create(impl);

   nir_foreach_block (block, impl) {
      nir_foreach_instr (instr, block) {
         if (instr->type == nir_instr_type_tex) {
            nir_tex_instr *tex = nir_instr_as_tex(instr);
            bool is_gather = devinfo->ver < 8 && tex->op == nir_texop_tg4;

            /* rewrite the tg4 component from green to blue before replacing the
               texture index */
            if (devinfo->verx10 == 70) {
               if (tex->component == 1)
                  if (key->gather_channel_quirk_mask & (1 << tex->texture_index))
                     tex->component = 2;
            }

            if (is_gather && devinfo->ver == 6 && key->gfx6_gather_wa[tex->texture_index]) {
               b.cursor = nir_after_instr(instr);
               enum gfx6_gather_sampler_wa wa = key->gfx6_gather_wa[tex->texture_index];
               int width = (wa & WA_8BIT) ? 8 : 16;

               nir_def *val = nir_fmul_imm(&b, &tex->def, (1 << width) - 1);
               val = nir_f2u32(&b, val);
               if (wa & WA_SIGN) {
                  val = nir_ishl_imm(&b, val, 32 - width);
                  val = nir_ishr_imm(&b, val, 32 - width);
               }
               nir_def_rewrite_uses_after(&tex->def, val, val->parent_instr);
            }

            tex->texture_index =
               crocus_group_index_to_bti(bt, is_gather ? CROCUS_SURFACE_GROUP_TEXTURE_GATHER : CROCUS_SURFACE_GROUP_TEXTURE,
                                         tex->texture_index);
            continue;
         }

         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         switch (intrin->intrinsic) {
         case nir_intrinsic_image_size:
         case nir_intrinsic_image_load:
         case nir_intrinsic_image_store:
         case nir_intrinsic_image_atomic:
         case nir_intrinsic_image_atomic_swap:
         case nir_intrinsic_image_load_raw_intel:
         case nir_intrinsic_image_store_raw_intel:
            rewrite_src_with_bti(&b, bt, instr, &intrin->src[0],
                                 CROCUS_SURFACE_GROUP_IMAGE);
            break;

         case nir_intrinsic_load_ubo:
            rewrite_src_with_bti(&b, bt, instr, &intrin->src[0],
                                 CROCUS_SURFACE_GROUP_UBO);
            break;

         case nir_intrinsic_store_ssbo:
            rewrite_src_with_bti(&b, bt, instr, &intrin->src[1],
                                 CROCUS_SURFACE_GROUP_SSBO);
            break;

         case nir_intrinsic_load_output:
            if (devinfo->ver >= 6) {
               rewrite_src_with_bti(&b, bt, instr, &intrin->src[0],
                                    CROCUS_SURFACE_GROUP_RENDER_TARGET_READ);
            }
            break;

         case nir_intrinsic_get_ssbo_size:
         case nir_intrinsic_ssbo_atomic:
         case nir_intrinsic_ssbo_atomic_swap:
         case nir_intrinsic_load_ssbo:
            rewrite_src_with_bti(&b, bt, instr, &intrin->src[0],
                                 CROCUS_SURFACE_GROUP_SSBO);
            break;

         default:
            break;
         }
      }
   }
}

static void
crocus_debug_recompile(struct crocus_context *ice,
                       struct shader_info *info,
                       const struct brw_base_prog_key *key)
{
   struct crocus_screen *screen = (struct crocus_screen *) ice->ctx.screen;
   const struct brw_compiler *c = screen->compiler;

   if (!info)
      return;

   brw_shader_perf_log(c, &ice->dbg, "Recompiling %s shader for program %s: %s\n",
                       _mesa_shader_stage_to_string(info->stage),
                       info->name ? info->name : "(no identifier)",
                       info->label ? info->label : "");

   const void *old_key =
      crocus_find_previous_compile(ice, info->stage, key->program_string_id);

   brw_debug_key_recompile(c, &ice->dbg, info->stage, old_key, key);
}

/**
 * Get the shader for the last enabled geometry stage.
 *
 * This stage is the one which will feed stream output and the rasterizer.
 */
static gl_shader_stage
last_vue_stage(struct crocus_context *ice)
{
   if (ice->shaders.uncompiled[MESA_SHADER_GEOMETRY])
      return MESA_SHADER_GEOMETRY;

   if (ice->shaders.uncompiled[MESA_SHADER_TESS_EVAL])
      return MESA_SHADER_TESS_EVAL;

   return MESA_SHADER_VERTEX;
}

static GLbitfield64
crocus_vs_outputs_written(struct crocus_context *ice,
                          const struct brw_vs_prog_key *key,
                          GLbitfield64 user_varyings)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   GLbitfield64 outputs_written = user_varyings;

   if (devinfo->ver < 6) {

      if (key->copy_edgeflag)
         outputs_written |= BITFIELD64_BIT(VARYING_SLOT_EDGE);

      /* Put dummy slots into the VUE for the SF to put the replaced
       * point sprite coords in.  We shouldn't need these dummy slots,
       * which take up precious URB space, but it would mean that the SF
       * doesn't get nice aligned pairs of input coords into output
       * coords, which would be a pain to handle.
       */
      for (unsigned i = 0; i < 8; i++) {
         if (key->point_coord_replace & (1 << i))
            outputs_written |= BITFIELD64_BIT(VARYING_SLOT_TEX0 + i);
      }

      /* if back colors are written, allocate slots for front colors too */
      if (outputs_written & BITFIELD64_BIT(VARYING_SLOT_BFC0))
         outputs_written |= BITFIELD64_BIT(VARYING_SLOT_COL0);
      if (outputs_written & BITFIELD64_BIT(VARYING_SLOT_BFC1))
         outputs_written |= BITFIELD64_BIT(VARYING_SLOT_COL1);
   }

   /* In order for legacy clipping to work, we need to populate the clip
    * distance varying slots whenever clipping is enabled, even if the vertex
    * shader doesn't write to gl_ClipDistance.
    */
   if (key->nr_userclip_plane_consts > 0) {
      outputs_written |= BITFIELD64_BIT(VARYING_SLOT_CLIP_DIST0);
      outputs_written |= BITFIELD64_BIT(VARYING_SLOT_CLIP_DIST1);
   }

   return outputs_written;
}

/*
 * If no edgeflags come from the user, gen4/5
 * require giving the clip shader a default edgeflag.
 *
 * This will always be 1.0.
 */
static void
crocus_lower_default_edgeflags(struct nir_shader *nir)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   nir_builder b = nir_builder_at(nir_after_impl(impl));

   nir_variable *var = nir_variable_create(nir, nir_var_shader_out,
                                           glsl_float_type(),
                                           "edgeflag");
   var->data.location = VARYING_SLOT_EDGE;
   nir_store_var(&b, var, nir_imm_float(&b, 1.0), 0x1);
}

/**
 * Compile a vertex shader, and upload the assembly.
 */
static struct crocus_compiled_shader *
crocus_compile_vs(struct crocus_context *ice,
                  struct crocus_uncompiled_shader *ish,
                  const struct brw_vs_prog_key *key)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct brw_compiler *compiler = screen->compiler;
   const struct intel_device_info *devinfo = &screen->devinfo;
   void *mem_ctx = ralloc_context(NULL);
   struct brw_vs_prog_data *vs_prog_data =
      rzalloc(mem_ctx, struct brw_vs_prog_data);
   struct brw_vue_prog_data *vue_prog_data = &vs_prog_data->base;
   struct brw_stage_prog_data *prog_data = &vue_prog_data->base;
   enum brw_param_builtin *system_values;
   unsigned num_system_values;
   unsigned num_cbufs;

   nir_shader *nir = nir_shader_clone(mem_ctx, ish->nir);

   if (key->nr_userclip_plane_consts) {
      nir_function_impl *impl = nir_shader_get_entrypoint(nir);
      /* Check if variables were found. */
      if (nir_lower_clip_vs(nir, (1 << key->nr_userclip_plane_consts) - 1,
                            true, false, NULL)) {
         nir_lower_io_to_temporaries(nir, impl, true, false);
         nir_lower_global_vars_to_local(nir);
         nir_lower_vars_to_ssa(nir);
         nir_shader_gather_info(nir, impl);
      }
   }

   if (key->clamp_pointsize)
      nir_lower_point_size(nir, 1.0, 255.0);

   prog_data->use_alt_mode = nir->info.use_legacy_math_rules;

   crocus_setup_uniforms(devinfo, mem_ctx, nir, prog_data, &system_values,
                         &num_system_values, &num_cbufs);

   crocus_lower_swizzles(nir, &key->base.tex);

   if (devinfo->ver <= 5 &&
       !(nir->info.inputs_read & BITFIELD64_BIT(VERT_ATTRIB_EDGEFLAG)))
      crocus_lower_default_edgeflags(nir);

   struct crocus_binding_table bt;
   crocus_setup_binding_table(devinfo, nir, &bt, /* num_render_targets */ 0,
                              num_system_values, num_cbufs, &key->base.tex);

   if (can_push_ubo(devinfo))
      brw_nir_analyze_ubo_ranges(compiler, nir, prog_data->ubo_ranges);

   uint64_t outputs_written =
      crocus_vs_outputs_written(ice, key, nir->info.outputs_written);
   brw_compute_vue_map(devinfo,
                       &vue_prog_data->vue_map, outputs_written,
                       nir->info.separate_shader, /* pos slots */ 1);

   /* Don't tell the backend about our clip plane constants, we've already
    * lowered them in NIR and we don't want it doing it again.
    */
   struct brw_vs_prog_key key_no_ucp = *key;
   key_no_ucp.nr_userclip_plane_consts = 0;
   key_no_ucp.copy_edgeflag = false;
   crocus_sanitize_tex_key(&key_no_ucp.base.tex);

   struct brw_compile_vs_params params = {
      .base = {
         .mem_ctx = mem_ctx,
         .nir = nir,
         .log_data = &ice->dbg,
      },
      .key = &key_no_ucp,
      .prog_data = vs_prog_data,
      .edgeflag_is_last = devinfo->ver < 6,
   };
   const unsigned *program =
      brw_compile_vs(compiler, &params);
   if (program == NULL) {
      dbg_printf("Failed to compile vertex shader: %s\n", params.base.error_str);
      ralloc_free(mem_ctx);
      return false;
   }

   if (ish->compiled_once) {
      crocus_debug_recompile(ice, &nir->info, &key->base);
   } else {
      ish->compiled_once = true;
   }

   uint32_t *so_decls = NULL;
   if (devinfo->ver > 6)
      so_decls = screen->vtbl.create_so_decl_list(&ish->stream_output,
                                                  &vue_prog_data->vue_map);

   struct crocus_compiled_shader *shader =
      crocus_upload_shader(ice, CROCUS_CACHE_VS, sizeof(*key), key, program,
                           prog_data->program_size,
                           prog_data, sizeof(*vs_prog_data), so_decls,
                           system_values, num_system_values,
                           num_cbufs, &bt);

   crocus_disk_cache_store(screen->disk_cache, ish, shader,
                           ice->shaders.cache_bo_map,
                           key, sizeof(*key));

   ralloc_free(mem_ctx);
   return shader;
}

/**
 * Update the current vertex shader variant.
 *
 * Fill out the key, look in the cache, compile and bind if needed.
 */
static void
crocus_update_compiled_vs(struct crocus_context *ice)
{
   struct crocus_shader_state *shs = &ice->state.shaders[MESA_SHADER_VERTEX];
   struct crocus_uncompiled_shader *ish =
      ice->shaders.uncompiled[MESA_SHADER_VERTEX];
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct brw_vs_prog_key key = { KEY_INIT() };

   if (ish->nos & (1ull << CROCUS_NOS_TEXTURES))
      crocus_populate_sampler_prog_key_data(ice, devinfo, MESA_SHADER_VERTEX, ish,
                                            ish->nir->info.uses_texture_gather, &key.base.tex);
   screen->vtbl.populate_vs_key(ice, &ish->nir->info, last_vue_stage(ice), &key);

   struct crocus_compiled_shader *old = ice->shaders.prog[CROCUS_CACHE_VS];
   struct crocus_compiled_shader *shader =
      crocus_find_cached_shader(ice, CROCUS_CACHE_VS, sizeof(key), &key);

   if (!shader)
      shader = crocus_disk_cache_retrieve(ice, ish, &key, sizeof(key));

   if (!shader)
      shader = crocus_compile_vs(ice, ish, &key);

   if (old != shader) {
      ice->shaders.prog[CROCUS_CACHE_VS] = shader;
      if (devinfo->ver == 8)
         ice->state.dirty |= CROCUS_DIRTY_GEN8_VF_SGVS;
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_VS |
                                CROCUS_STAGE_DIRTY_BINDINGS_VS |
                                CROCUS_STAGE_DIRTY_CONSTANTS_VS;
      shs->sysvals_need_upload = true;

      const struct brw_vs_prog_data *vs_prog_data =
         (void *) shader->prog_data;
      const bool uses_draw_params = vs_prog_data->uses_firstvertex ||
                                    vs_prog_data->uses_baseinstance;
      const bool uses_derived_draw_params = vs_prog_data->uses_drawid ||
                                            vs_prog_data->uses_is_indexed_draw;
      const bool needs_sgvs_element = uses_draw_params ||
                                      vs_prog_data->uses_instanceid ||
                                      vs_prog_data->uses_vertexid;

      if (ice->state.vs_uses_draw_params != uses_draw_params ||
          ice->state.vs_uses_derived_draw_params != uses_derived_draw_params ||
          ice->state.vs_needs_edge_flag != ish->needs_edge_flag ||
          ice->state.vs_uses_vertexid != vs_prog_data->uses_vertexid ||
          ice->state.vs_uses_instanceid != vs_prog_data->uses_instanceid) {
         ice->state.dirty |= CROCUS_DIRTY_VERTEX_BUFFERS |
                             CROCUS_DIRTY_VERTEX_ELEMENTS;
      }
      ice->state.vs_uses_draw_params = uses_draw_params;
      ice->state.vs_uses_derived_draw_params = uses_derived_draw_params;
      ice->state.vs_needs_sgvs_element = needs_sgvs_element;
      ice->state.vs_needs_edge_flag = ish->needs_edge_flag;
      ice->state.vs_uses_vertexid = vs_prog_data->uses_vertexid;
      ice->state.vs_uses_instanceid = vs_prog_data->uses_instanceid;
   }
}

/**
 * Get the shader_info for a given stage, or NULL if the stage is disabled.
 */
const struct shader_info *
crocus_get_shader_info(const struct crocus_context *ice, gl_shader_stage stage)
{
   const struct crocus_uncompiled_shader *ish = ice->shaders.uncompiled[stage];

   if (!ish)
      return NULL;

   const nir_shader *nir = ish->nir;
   return &nir->info;
}

/**
 * Get the union of TCS output and TES input slots.
 *
 * TCS and TES need to agree on a common URB entry layout.  In particular,
 * the data for all patch vertices is stored in a single URB entry (unlike
 * GS which has one entry per input vertex).  This means that per-vertex
 * array indexing needs a stride.
 *
 * SSO requires locations to match, but doesn't require the number of
 * outputs/inputs to match (in fact, the TCS often has extra outputs).
 * So, we need to take the extra step of unifying these on the fly.
 */
static void
get_unified_tess_slots(const struct crocus_context *ice,
                       uint64_t *per_vertex_slots,
                       uint32_t *per_patch_slots)
{
   const struct shader_info *tcs =
      crocus_get_shader_info(ice, MESA_SHADER_TESS_CTRL);
   const struct shader_info *tes =
      crocus_get_shader_info(ice, MESA_SHADER_TESS_EVAL);

   *per_vertex_slots = tes->inputs_read;
   *per_patch_slots = tes->patch_inputs_read;

   if (tcs) {
      *per_vertex_slots |= tcs->outputs_written;
      *per_patch_slots |= tcs->patch_outputs_written;
   }
}

/**
 * Compile a tessellation control shader, and upload the assembly.
 */
static struct crocus_compiled_shader *
crocus_compile_tcs(struct crocus_context *ice,
                   struct crocus_uncompiled_shader *ish,
                   const struct brw_tcs_prog_key *key)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct brw_compiler *compiler = screen->compiler;
   void *mem_ctx = ralloc_context(NULL);
   struct brw_tcs_prog_data *tcs_prog_data =
      rzalloc(mem_ctx, struct brw_tcs_prog_data);
   struct brw_vue_prog_data *vue_prog_data = &tcs_prog_data->base;
   struct brw_stage_prog_data *prog_data = &vue_prog_data->base;
   const struct intel_device_info *devinfo = &screen->devinfo;
   enum brw_param_builtin *system_values = NULL;
   unsigned num_system_values = 0;
   unsigned num_cbufs = 0;

   nir_shader *nir;

   struct crocus_binding_table bt;

   if (ish) {
      nir = nir_shader_clone(mem_ctx, ish->nir);
   } else {
      nir = brw_nir_create_passthrough_tcs(mem_ctx, compiler, key);
   }

   crocus_setup_uniforms(devinfo, mem_ctx, nir, prog_data, &system_values,
                         &num_system_values, &num_cbufs);

   crocus_lower_swizzles(nir, &key->base.tex);
   crocus_setup_binding_table(devinfo, nir, &bt, /* num_render_targets */ 0,
                              num_system_values, num_cbufs, &key->base.tex);
   if (can_push_ubo(devinfo))
      brw_nir_analyze_ubo_ranges(compiler, nir, prog_data->ubo_ranges);

   struct brw_tcs_prog_key key_clean = *key;
   crocus_sanitize_tex_key(&key_clean.base.tex);

   struct brw_compile_tcs_params params = {
      .base = {
         .mem_ctx = mem_ctx,
         .nir = nir,
         .log_data = &ice->dbg,
      },
      .key = &key_clean,
      .prog_data = tcs_prog_data,
   };

   const unsigned *program = brw_compile_tcs(compiler, &params);
   if (program == NULL) {
      dbg_printf("Failed to compile control shader: %s\n", params.base.error_str);
      ralloc_free(mem_ctx);
      return false;
   }

   if (ish) {
      if (ish->compiled_once) {
         crocus_debug_recompile(ice, &nir->info, &key->base);
      } else {
         ish->compiled_once = true;
      }
   }

   struct crocus_compiled_shader *shader =
      crocus_upload_shader(ice, CROCUS_CACHE_TCS, sizeof(*key), key, program,
                           prog_data->program_size,
                           prog_data, sizeof(*tcs_prog_data), NULL,
                           system_values, num_system_values,
                           num_cbufs, &bt);

   if (ish)
      crocus_disk_cache_store(screen->disk_cache, ish, shader,
                              ice->shaders.cache_bo_map,
                              key, sizeof(*key));

   ralloc_free(mem_ctx);
   return shader;
}

/**
 * Update the current tessellation control shader variant.
 *
 * Fill out the key, look in the cache, compile and bind if needed.
 */
static void
crocus_update_compiled_tcs(struct crocus_context *ice)
{
   struct crocus_shader_state *shs = &ice->state.shaders[MESA_SHADER_TESS_CTRL];
   struct crocus_uncompiled_shader *tcs =
      ice->shaders.uncompiled[MESA_SHADER_TESS_CTRL];
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   const struct shader_info *tes_info =
      crocus_get_shader_info(ice, MESA_SHADER_TESS_EVAL);
   struct brw_tcs_prog_key key = {
      KEY_INIT_NO_ID(),
      .base.program_string_id = tcs ? tcs->program_id : 0,
      ._tes_primitive_mode = tes_info->tess._primitive_mode,
      .input_vertices = ice->state.vertices_per_patch,
      .quads_workaround = tes_info->tess._primitive_mode == TESS_PRIMITIVE_QUADS &&
                          tes_info->tess.spacing == TESS_SPACING_EQUAL,
   };

   if (tcs && tcs->nos & (1ull << CROCUS_NOS_TEXTURES))
      crocus_populate_sampler_prog_key_data(ice, devinfo, MESA_SHADER_TESS_CTRL, tcs,
                                            tcs->nir->info.uses_texture_gather, &key.base.tex);
   get_unified_tess_slots(ice, &key.outputs_written,
                          &key.patch_outputs_written);
   screen->vtbl.populate_tcs_key(ice, &key);

   struct crocus_compiled_shader *old = ice->shaders.prog[CROCUS_CACHE_TCS];
   struct crocus_compiled_shader *shader =
      crocus_find_cached_shader(ice, CROCUS_CACHE_TCS, sizeof(key), &key);

   if (tcs && !shader)
      shader = crocus_disk_cache_retrieve(ice, tcs, &key, sizeof(key));

   if (!shader)
      shader = crocus_compile_tcs(ice, tcs, &key);

   if (old != shader) {
      ice->shaders.prog[CROCUS_CACHE_TCS] = shader;
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_TCS |
                                CROCUS_STAGE_DIRTY_BINDINGS_TCS |
                                CROCUS_STAGE_DIRTY_CONSTANTS_TCS;
      shs->sysvals_need_upload = true;
   }
}

/**
 * Compile a tessellation evaluation shader, and upload the assembly.
 */
static struct crocus_compiled_shader *
crocus_compile_tes(struct crocus_context *ice,
                   struct crocus_uncompiled_shader *ish,
                   const struct brw_tes_prog_key *key)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct brw_compiler *compiler = screen->compiler;
   void *mem_ctx = ralloc_context(NULL);
   struct brw_tes_prog_data *tes_prog_data =
      rzalloc(mem_ctx, struct brw_tes_prog_data);
   struct brw_vue_prog_data *vue_prog_data = &tes_prog_data->base;
   struct brw_stage_prog_data *prog_data = &vue_prog_data->base;
   enum brw_param_builtin *system_values;
   const struct intel_device_info *devinfo = &screen->devinfo;
   unsigned num_system_values;
   unsigned num_cbufs;

   nir_shader *nir = nir_shader_clone(mem_ctx, ish->nir);

   if (key->nr_userclip_plane_consts) {
      nir_function_impl *impl = nir_shader_get_entrypoint(nir);
      nir_lower_clip_vs(nir, (1 << key->nr_userclip_plane_consts) - 1, true,
                        false, NULL);
      nir_lower_io_to_temporaries(nir, impl, true, false);
      nir_lower_global_vars_to_local(nir);
      nir_lower_vars_to_ssa(nir);
      nir_shader_gather_info(nir, impl);
   }

   if (key->clamp_pointsize)
      nir_lower_point_size(nir, 1.0, 255.0);

   crocus_setup_uniforms(devinfo, mem_ctx, nir, prog_data, &system_values,
                         &num_system_values, &num_cbufs);
   crocus_lower_swizzles(nir, &key->base.tex);
   struct crocus_binding_table bt;
   crocus_setup_binding_table(devinfo, nir, &bt, /* num_render_targets */ 0,
                              num_system_values, num_cbufs, &key->base.tex);

   if (can_push_ubo(devinfo))
      brw_nir_analyze_ubo_ranges(compiler, nir, prog_data->ubo_ranges);

   struct brw_vue_map input_vue_map;
   brw_compute_tess_vue_map(&input_vue_map, key->inputs_read,
                            key->patch_inputs_read);

   struct brw_tes_prog_key key_clean = *key;
   crocus_sanitize_tex_key(&key_clean.base.tex);

   struct brw_compile_tes_params params = {
      .base = {
         .mem_ctx = mem_ctx,
         .nir = nir,
         .log_data = &ice->dbg,
      },
      .key = &key_clean,
      .prog_data = tes_prog_data,
      .input_vue_map = &input_vue_map,
   };

   const unsigned *program = brw_compile_tes(compiler, &params);
   if (program == NULL) {
      dbg_printf("Failed to compile evaluation shader: %s\n", params.base.error_str);
      ralloc_free(mem_ctx);
      return false;
   }

   if (ish->compiled_once) {
      crocus_debug_recompile(ice, &nir->info, &key->base);
   } else {
      ish->compiled_once = true;
   }

   uint32_t *so_decls = NULL;
   if (devinfo->ver > 6)
      so_decls = screen->vtbl.create_so_decl_list(&ish->stream_output,
                                                  &vue_prog_data->vue_map);

   struct crocus_compiled_shader *shader =
      crocus_upload_shader(ice, CROCUS_CACHE_TES, sizeof(*key), key, program,
                           prog_data->program_size,
                           prog_data, sizeof(*tes_prog_data), so_decls,
                           system_values, num_system_values,
                           num_cbufs, &bt);

   crocus_disk_cache_store(screen->disk_cache, ish, shader,
                           ice->shaders.cache_bo_map,
                           key, sizeof(*key));

   ralloc_free(mem_ctx);
   return shader;
}

/**
 * Update the current tessellation evaluation shader variant.
 *
 * Fill out the key, look in the cache, compile and bind if needed.
 */
static void
crocus_update_compiled_tes(struct crocus_context *ice)
{
   struct crocus_shader_state *shs = &ice->state.shaders[MESA_SHADER_TESS_EVAL];
   struct crocus_uncompiled_shader *ish =
      ice->shaders.uncompiled[MESA_SHADER_TESS_EVAL];
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   struct brw_tes_prog_key key = { KEY_INIT() };
   const struct intel_device_info *devinfo = &screen->devinfo;

   if (ish->nos & (1ull << CROCUS_NOS_TEXTURES))
      crocus_populate_sampler_prog_key_data(ice, devinfo, MESA_SHADER_TESS_EVAL, ish,
                                            ish->nir->info.uses_texture_gather, &key.base.tex);
   get_unified_tess_slots(ice, &key.inputs_read, &key.patch_inputs_read);
   screen->vtbl.populate_tes_key(ice, &ish->nir->info, last_vue_stage(ice), &key);

   struct crocus_compiled_shader *old = ice->shaders.prog[CROCUS_CACHE_TES];
   struct crocus_compiled_shader *shader =
      crocus_find_cached_shader(ice, CROCUS_CACHE_TES, sizeof(key), &key);

   if (!shader)
      shader = crocus_disk_cache_retrieve(ice, ish, &key, sizeof(key));

   if (!shader)
      shader = crocus_compile_tes(ice, ish, &key);

   if (old != shader) {
      ice->shaders.prog[CROCUS_CACHE_TES] = shader;
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_TES |
                                CROCUS_STAGE_DIRTY_BINDINGS_TES |
                                CROCUS_STAGE_DIRTY_CONSTANTS_TES;
      shs->sysvals_need_upload = true;
   }

   /* TODO: Could compare and avoid flagging this. */
   const struct shader_info *tes_info = &ish->nir->info;
   if (BITSET_TEST(tes_info->system_values_read, SYSTEM_VALUE_VERTICES_IN)) {
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_TES;
      ice->state.shaders[MESA_SHADER_TESS_EVAL].sysvals_need_upload = true;
   }
}

/**
 * Compile a geometry shader, and upload the assembly.
 */
static struct crocus_compiled_shader *
crocus_compile_gs(struct crocus_context *ice,
                  struct crocus_uncompiled_shader *ish,
                  const struct brw_gs_prog_key *key)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct brw_compiler *compiler = screen->compiler;
   const struct intel_device_info *devinfo = &screen->devinfo;
   void *mem_ctx = ralloc_context(NULL);
   struct brw_gs_prog_data *gs_prog_data =
      rzalloc(mem_ctx, struct brw_gs_prog_data);
   struct brw_vue_prog_data *vue_prog_data = &gs_prog_data->base;
   struct brw_stage_prog_data *prog_data = &vue_prog_data->base;
   enum brw_param_builtin *system_values;
   unsigned num_system_values;
   unsigned num_cbufs;

   nir_shader *nir = nir_shader_clone(mem_ctx, ish->nir);

   if (key->nr_userclip_plane_consts) {
      nir_function_impl *impl = nir_shader_get_entrypoint(nir);
      nir_lower_clip_gs(nir, (1 << key->nr_userclip_plane_consts) - 1, false,
                        NULL);
      nir_lower_io_to_temporaries(nir, impl, true, false);
      nir_lower_global_vars_to_local(nir);
      nir_lower_vars_to_ssa(nir);
      nir_shader_gather_info(nir, impl);
   }

   if (key->clamp_pointsize)
      nir_lower_point_size(nir, 1.0, 255.0);

   crocus_setup_uniforms(devinfo, mem_ctx, nir, prog_data, &system_values,
                         &num_system_values, &num_cbufs);
   crocus_lower_swizzles(nir, &key->base.tex);
   struct crocus_binding_table bt;
   crocus_setup_binding_table(devinfo, nir, &bt, /* num_render_targets */ 0,
                              num_system_values, num_cbufs, &key->base.tex);

   if (can_push_ubo(devinfo))
      brw_nir_analyze_ubo_ranges(compiler, nir, prog_data->ubo_ranges);

   brw_compute_vue_map(devinfo,
                       &vue_prog_data->vue_map, nir->info.outputs_written,
                       nir->info.separate_shader, /* pos slots */ 1);

   if (devinfo->ver == 6)
      gfx6_gs_xfb_setup(&ish->stream_output, gs_prog_data);
   struct brw_gs_prog_key key_clean = *key;
   crocus_sanitize_tex_key(&key_clean.base.tex);

   struct brw_compile_gs_params params = {
      .base = {
         .mem_ctx = mem_ctx,
         .nir = nir,
         .log_data = &ice->dbg,
      },
      .key = &key_clean,
      .prog_data = gs_prog_data,
   };

   const unsigned *program = brw_compile_gs(compiler, &params);
   if (program == NULL) {
      dbg_printf("Failed to compile geometry shader: %s\n", params.base.error_str);
      ralloc_free(mem_ctx);
      return false;
   }

   if (ish->compiled_once) {
      crocus_debug_recompile(ice, &nir->info, &key->base);
   } else {
      ish->compiled_once = true;
   }

   uint32_t *so_decls = NULL;
   if (devinfo->ver > 6)
      so_decls = screen->vtbl.create_so_decl_list(&ish->stream_output,
                                                  &vue_prog_data->vue_map);

   struct crocus_compiled_shader *shader =
      crocus_upload_shader(ice, CROCUS_CACHE_GS, sizeof(*key), key, program,
                           prog_data->program_size,
                           prog_data, sizeof(*gs_prog_data), so_decls,
                           system_values, num_system_values,
                           num_cbufs, &bt);

   crocus_disk_cache_store(screen->disk_cache, ish, shader,
                           ice->shaders.cache_bo_map,
                           key, sizeof(*key));

   ralloc_free(mem_ctx);
   return shader;
}

/**
 * Update the current geometry shader variant.
 *
 * Fill out the key, look in the cache, compile and bind if needed.
 */
static void
crocus_update_compiled_gs(struct crocus_context *ice)
{
   struct crocus_shader_state *shs = &ice->state.shaders[MESA_SHADER_GEOMETRY];
   struct crocus_uncompiled_shader *ish =
      ice->shaders.uncompiled[MESA_SHADER_GEOMETRY];
   struct crocus_compiled_shader *old = ice->shaders.prog[CROCUS_CACHE_GS];
   struct crocus_compiled_shader *shader = NULL;

   if (ish) {
      struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
      const struct intel_device_info *devinfo = &screen->devinfo;
      struct brw_gs_prog_key key = { KEY_INIT() };

      if (ish->nos & (1ull << CROCUS_NOS_TEXTURES))
         crocus_populate_sampler_prog_key_data(ice, devinfo, MESA_SHADER_GEOMETRY, ish,
                                               ish->nir->info.uses_texture_gather, &key.base.tex);
      screen->vtbl.populate_gs_key(ice, &ish->nir->info, last_vue_stage(ice), &key);

      shader =
         crocus_find_cached_shader(ice, CROCUS_CACHE_GS, sizeof(key), &key);

      if (!shader)
         shader = crocus_disk_cache_retrieve(ice, ish, &key, sizeof(key));

      if (!shader)
         shader = crocus_compile_gs(ice, ish, &key);
   }

   if (old != shader) {
      ice->shaders.prog[CROCUS_CACHE_GS] = shader;
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_GS |
                                CROCUS_STAGE_DIRTY_BINDINGS_GS |
                                CROCUS_STAGE_DIRTY_CONSTANTS_GS;
      shs->sysvals_need_upload = true;
   }
}

/**
 * Compile a fragment (pixel) shader, and upload the assembly.
 */
static struct crocus_compiled_shader *
crocus_compile_fs(struct crocus_context *ice,
                  struct crocus_uncompiled_shader *ish,
                  const struct brw_wm_prog_key *key,
                  struct brw_vue_map *vue_map)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct brw_compiler *compiler = screen->compiler;
   void *mem_ctx = ralloc_context(NULL);
   struct brw_wm_prog_data *fs_prog_data =
      rzalloc(mem_ctx, struct brw_wm_prog_data);
   struct brw_stage_prog_data *prog_data = &fs_prog_data->base;
   enum brw_param_builtin *system_values;
   const struct intel_device_info *devinfo = &screen->devinfo;
   unsigned num_system_values;
   unsigned num_cbufs;

   nir_shader *nir = nir_shader_clone(mem_ctx, ish->nir);

   prog_data->use_alt_mode = nir->info.use_legacy_math_rules;

   crocus_setup_uniforms(devinfo, mem_ctx, nir, prog_data, &system_values,
                         &num_system_values, &num_cbufs);

   /* Lower output variables to load_output intrinsics before setting up
    * binding tables, so crocus_setup_binding_table can map any load_output
    * intrinsics to CROCUS_SURFACE_GROUP_RENDER_TARGET_READ on Gen8 for
    * non-coherent framebuffer fetches.
    */
   brw_nir_lower_fs_outputs(nir);

   /* lower swizzles before binding table */
   crocus_lower_swizzles(nir, &key->base.tex);
   int null_rts = 1;

   struct crocus_binding_table bt;
   crocus_setup_binding_table(devinfo, nir, &bt,
                              MAX2(key->nr_color_regions, null_rts),
                              num_system_values, num_cbufs,
                              &key->base.tex);

   if (can_push_ubo(devinfo))
      brw_nir_analyze_ubo_ranges(compiler, nir, prog_data->ubo_ranges);

   struct brw_wm_prog_key key_clean = *key;
   crocus_sanitize_tex_key(&key_clean.base.tex);

   struct brw_compile_fs_params params = {
      .base = {
         .mem_ctx = mem_ctx,
         .nir = nir,
         .log_data = &ice->dbg,
      },
      .key = &key_clean,
      .prog_data = fs_prog_data,

      .allow_spilling = true,
      .max_polygons = 1,
      .vue_map = vue_map,
   };
   const unsigned *program =
      brw_compile_fs(compiler, &params);
   if (program == NULL) {
      dbg_printf("Failed to compile fragment shader: %s\n", params.base.error_str);
      ralloc_free(mem_ctx);
      return false;
   }

   if (ish->compiled_once) {
      crocus_debug_recompile(ice, &nir->info, &key->base);
   } else {
      ish->compiled_once = true;
   }

   struct crocus_compiled_shader *shader =
      crocus_upload_shader(ice, CROCUS_CACHE_FS, sizeof(*key), key, program,
                           prog_data->program_size,
                           prog_data, sizeof(*fs_prog_data), NULL,
                           system_values, num_system_values,
                           num_cbufs, &bt);

   crocus_disk_cache_store(screen->disk_cache, ish, shader,
                           ice->shaders.cache_bo_map,
                           key, sizeof(*key));

   ralloc_free(mem_ctx);
   return shader;
}

/**
 * Update the current fragment shader variant.
 *
 * Fill out the key, look in the cache, compile and bind if needed.
 */
static void
crocus_update_compiled_fs(struct crocus_context *ice)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct crocus_shader_state *shs = &ice->state.shaders[MESA_SHADER_FRAGMENT];
   struct crocus_uncompiled_shader *ish =
      ice->shaders.uncompiled[MESA_SHADER_FRAGMENT];
   struct brw_wm_prog_key key = { KEY_INIT() };

   if (ish->nos & (1ull << CROCUS_NOS_TEXTURES))
      crocus_populate_sampler_prog_key_data(ice, devinfo, MESA_SHADER_FRAGMENT, ish,
                                            ish->nir->info.uses_texture_gather, &key.base.tex);
   screen->vtbl.populate_fs_key(ice, &ish->nir->info, &key);

   if (ish->nos & (1ull << CROCUS_NOS_LAST_VUE_MAP))
      key.input_slots_valid = ice->shaders.last_vue_map->slots_valid;

   struct crocus_compiled_shader *old = ice->shaders.prog[CROCUS_CACHE_FS];
   struct crocus_compiled_shader *shader =
      crocus_find_cached_shader(ice, CROCUS_CACHE_FS, sizeof(key), &key);

   if (!shader)
      shader = crocus_disk_cache_retrieve(ice, ish, &key, sizeof(key));

   if (!shader)
      shader = crocus_compile_fs(ice, ish, &key, ice->shaders.last_vue_map);

   if (old != shader) {
      // XXX: only need to flag CLIP if barycentric has NONPERSPECTIVE
      // toggles.  might be able to avoid flagging SBE too.
      ice->shaders.prog[CROCUS_CACHE_FS] = shader;
      ice->state.dirty |= CROCUS_DIRTY_WM;
      /* gen4 clip/sf rely on fs prog_data */
      if (devinfo->ver < 6)
         ice->state.dirty |= CROCUS_DIRTY_GEN4_CLIP_PROG | CROCUS_DIRTY_GEN4_SF_PROG;
      else
         ice->state.dirty |= CROCUS_DIRTY_CLIP | CROCUS_DIRTY_GEN6_BLEND_STATE;
      if (devinfo->ver == 6)
         ice->state.dirty |= CROCUS_DIRTY_RASTER;
      if (devinfo->ver >= 7)
         ice->state.dirty |= CROCUS_DIRTY_GEN7_SBE;
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_FS |
                                CROCUS_STAGE_DIRTY_BINDINGS_FS |
                                CROCUS_STAGE_DIRTY_CONSTANTS_FS;
      shs->sysvals_need_upload = true;
   }
}

/**
 * Update the last enabled stage's VUE map.
 *
 * When the shader feeding the rasterizer's output interface changes, we
 * need to re-emit various packets.
 */
static void
update_last_vue_map(struct crocus_context *ice,
                    struct brw_stage_prog_data *prog_data)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct brw_vue_prog_data *vue_prog_data = (void *) prog_data;
   struct brw_vue_map *vue_map = &vue_prog_data->vue_map;
   struct brw_vue_map *old_map = ice->shaders.last_vue_map;
   const uint64_t changed_slots =
      (old_map ? old_map->slots_valid : 0ull) ^ vue_map->slots_valid;

   if (changed_slots & VARYING_BIT_VIEWPORT) {
      ice->state.num_viewports =
         (vue_map->slots_valid & VARYING_BIT_VIEWPORT) ? CROCUS_MAX_VIEWPORTS : 1;
      ice->state.dirty |= CROCUS_DIRTY_SF_CL_VIEWPORT |
                          CROCUS_DIRTY_CC_VIEWPORT;
      if (devinfo->ver < 6)
         ice->state.dirty |= CROCUS_DIRTY_GEN4_CLIP_PROG | CROCUS_DIRTY_GEN4_SF_PROG;

      if (devinfo->ver <= 6)
         ice->state.dirty |= CROCUS_DIRTY_GEN4_FF_GS_PROG;

      if (devinfo->ver >= 6)
         ice->state.dirty |= CROCUS_DIRTY_CLIP |
                             CROCUS_DIRTY_GEN6_SCISSOR_RECT;;
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_UNCOMPILED_FS |
         ice->state.stage_dirty_for_nos[CROCUS_NOS_LAST_VUE_MAP];
   }

   if (changed_slots || (old_map && old_map->separate != vue_map->separate)) {
      ice->state.dirty |= CROCUS_DIRTY_GEN7_SBE;
      if (devinfo->ver < 6)
         ice->state.dirty |= CROCUS_DIRTY_GEN4_FF_GS_PROG;
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_UNCOMPILED_FS;
   }

   ice->shaders.last_vue_map = &vue_prog_data->vue_map;
}

static void
crocus_update_pull_constant_descriptors(struct crocus_context *ice,
                                        gl_shader_stage stage)
{
   struct crocus_compiled_shader *shader = ice->shaders.prog[stage];

   if (!shader || !shader->prog_data->has_ubo_pull)
      return;

   struct crocus_shader_state *shs = &ice->state.shaders[stage];
   bool any_new_descriptors =
      shader->num_system_values > 0 && shs->sysvals_need_upload;

   unsigned bound_cbufs = shs->bound_cbufs;

   while (bound_cbufs) {
      const int i = u_bit_scan(&bound_cbufs);
      struct pipe_constant_buffer *cbuf = &shs->constbufs[i];
      if (cbuf->buffer) {
         any_new_descriptors = true;
      }
   }

   if (any_new_descriptors)
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_BINDINGS_VS << stage;
}

/**
 * Get the prog_data for a given stage, or NULL if the stage is disabled.
 */
static struct brw_vue_prog_data *
get_vue_prog_data(struct crocus_context *ice, gl_shader_stage stage)
{
   if (!ice->shaders.prog[stage])
      return NULL;

   return (void *) ice->shaders.prog[stage]->prog_data;
}

static struct crocus_compiled_shader *
crocus_compile_clip(struct crocus_context *ice, struct brw_clip_prog_key *key)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct brw_compiler *compiler = screen->compiler;
   void *mem_ctx;
   unsigned program_size;
   mem_ctx = ralloc_context(NULL);

   struct brw_clip_prog_data *clip_prog_data =
      rzalloc(mem_ctx, struct brw_clip_prog_data);

   const unsigned *program = brw_compile_clip(compiler, mem_ctx, key, clip_prog_data,
                                              ice->shaders.last_vue_map, &program_size);

   if (program == NULL) {
      dbg_printf("failed to compile clip shader\n");
      ralloc_free(mem_ctx);
      return false;
   }
   struct crocus_binding_table bt;
   memset(&bt, 0, sizeof(bt));

   struct crocus_compiled_shader *shader =
      crocus_upload_shader(ice, CROCUS_CACHE_CLIP, sizeof(*key), key, program,
                           program_size,
                           (struct brw_stage_prog_data *)clip_prog_data, sizeof(*clip_prog_data),
                           NULL, NULL, 0, 0, &bt);
   ralloc_free(mem_ctx);
   return shader;
}
static void
crocus_update_compiled_clip(struct crocus_context *ice)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   struct brw_clip_prog_key key;
   struct crocus_compiled_shader *old = ice->shaders.clip_prog;
   memset(&key, 0, sizeof(key));

   const struct brw_wm_prog_data *wm_prog_data = brw_wm_prog_data(ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data);
   if (wm_prog_data) {
      key.contains_flat_varying = wm_prog_data->contains_flat_varying;
      key.contains_noperspective_varying =
         wm_prog_data->contains_noperspective_varying;
      memcpy(key.interp_mode, wm_prog_data->interp_mode, sizeof(key.interp_mode));
   }

   key.primitive = ice->state.reduced_prim_mode;
   key.attrs = ice->shaders.last_vue_map->slots_valid;

   struct pipe_rasterizer_state *rs_state = crocus_get_rast_state(ice);
   key.pv_first = rs_state->flatshade_first;

   if (rs_state->clip_plane_enable)
      key.nr_userclip = util_logbase2(rs_state->clip_plane_enable) + 1;

   if (screen->devinfo.ver == 5)
      key.clip_mode = BRW_CLIP_MODE_KERNEL_CLIP;
   else
      key.clip_mode = BRW_CLIP_MODE_NORMAL;

   if (key.primitive == MESA_PRIM_TRIANGLES) {
      if (rs_state->cull_face == PIPE_FACE_FRONT_AND_BACK)
         key.clip_mode = BRW_CLIP_MODE_REJECT_ALL;
      else {
         uint32_t fill_front = BRW_CLIP_FILL_MODE_CULL;
         uint32_t fill_back = BRW_CLIP_FILL_MODE_CULL;
         uint32_t offset_front = 0;
         uint32_t offset_back = 0;

         if (!(rs_state->cull_face & PIPE_FACE_FRONT)) {
            switch (rs_state->fill_front) {
            case PIPE_POLYGON_MODE_FILL:
               fill_front = BRW_CLIP_FILL_MODE_FILL;
               offset_front = 0;
               break;
            case PIPE_POLYGON_MODE_LINE:
               fill_front = BRW_CLIP_FILL_MODE_LINE;
               offset_front = rs_state->offset_line;
               break;
            case PIPE_POLYGON_MODE_POINT:
               fill_front = BRW_CLIP_FILL_MODE_POINT;
               offset_front = rs_state->offset_point;
               break;
            }
         }

         if (!(rs_state->cull_face & PIPE_FACE_BACK)) {
            switch (rs_state->fill_back) {
            case PIPE_POLYGON_MODE_FILL:
               fill_back = BRW_CLIP_FILL_MODE_FILL;
               offset_back = 0;
               break;
            case PIPE_POLYGON_MODE_LINE:
               fill_back = BRW_CLIP_FILL_MODE_LINE;
               offset_back = rs_state->offset_line;
               break;
            case PIPE_POLYGON_MODE_POINT:
               fill_back = BRW_CLIP_FILL_MODE_POINT;
               offset_back = rs_state->offset_point;
               break;
            }
         }

         if (rs_state->fill_back != PIPE_POLYGON_MODE_FILL ||
             rs_state->fill_front != PIPE_POLYGON_MODE_FILL) {
            key.do_unfilled = 1;

            /* Most cases the fixed function units will handle.  Cases where
             * one or more polygon faces are unfilled will require help:
             */
            key.clip_mode = BRW_CLIP_MODE_CLIP_NON_REJECTED;

            if (offset_back || offset_front) {
               double mrd = 0.0;
               if (ice->state.framebuffer.zsbuf)
                  mrd = util_get_depth_format_mrd(util_format_description(ice->state.framebuffer.zsbuf->format));
               key.offset_units = rs_state->offset_units * mrd * 2;
               key.offset_factor = rs_state->offset_scale * mrd;
               key.offset_clamp = rs_state->offset_clamp * mrd;
            }

            if (!(rs_state->front_ccw ^ rs_state->bottom_edge_rule)) {
               key.fill_ccw = fill_front;
               key.fill_cw = fill_back;
               key.offset_ccw = offset_front;
               key.offset_cw = offset_back;
               if (rs_state->light_twoside &&
                   key.fill_cw != BRW_CLIP_FILL_MODE_CULL)
                  key.copy_bfc_cw = 1;
            } else {
               key.fill_cw = fill_front;
               key.fill_ccw = fill_back;
               key.offset_cw = offset_front;
               key.offset_ccw = offset_back;
               if (rs_state->light_twoside &&
                   key.fill_ccw != BRW_CLIP_FILL_MODE_CULL)
                  key.copy_bfc_ccw = 1;
            }
         }
      }
   }
   struct crocus_compiled_shader *shader =
      crocus_find_cached_shader(ice, CROCUS_CACHE_CLIP, sizeof(key), &key);

   if (!shader)
      shader = crocus_compile_clip(ice, &key);

   if (old != shader) {
      ice->state.dirty |= CROCUS_DIRTY_CLIP;
      ice->shaders.clip_prog = shader;
   }
}

static struct crocus_compiled_shader *
crocus_compile_sf(struct crocus_context *ice, struct brw_sf_prog_key *key)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct brw_compiler *compiler = screen->compiler;
   void *mem_ctx;
   unsigned program_size;
   mem_ctx = ralloc_context(NULL);

   struct brw_sf_prog_data *sf_prog_data =
      rzalloc(mem_ctx, struct brw_sf_prog_data);

   const unsigned *program = brw_compile_sf(compiler, mem_ctx, key, sf_prog_data,
                                            ice->shaders.last_vue_map, &program_size);

   if (program == NULL) {
      dbg_printf("failed to compile sf shader\n");
      ralloc_free(mem_ctx);
      return false;
   }

   struct crocus_binding_table bt;
   memset(&bt, 0, sizeof(bt));
   struct crocus_compiled_shader *shader =
      crocus_upload_shader(ice, CROCUS_CACHE_SF, sizeof(*key), key, program,
                           program_size,
                           (struct brw_stage_prog_data *)sf_prog_data, sizeof(*sf_prog_data),
                           NULL, NULL, 0, 0, &bt);
   ralloc_free(mem_ctx);
   return shader;
}

static void
crocus_update_compiled_sf(struct crocus_context *ice)
{
   struct brw_sf_prog_key key;
   struct crocus_compiled_shader *old = ice->shaders.sf_prog;
   memset(&key, 0, sizeof(key));

   key.attrs = ice->shaders.last_vue_map->slots_valid;

   switch (ice->state.reduced_prim_mode) {
   case MESA_PRIM_TRIANGLES:
   default:
      if (key.attrs & BITFIELD64_BIT(VARYING_SLOT_EDGE))
         key.primitive = BRW_SF_PRIM_UNFILLED_TRIS;
      else
         key.primitive = BRW_SF_PRIM_TRIANGLES;
      break;
   case MESA_PRIM_LINES:
      key.primitive = BRW_SF_PRIM_LINES;
      break;
   case MESA_PRIM_POINTS:
      key.primitive = BRW_SF_PRIM_POINTS;
      break;
   }

   struct pipe_rasterizer_state *rs_state = crocus_get_rast_state(ice);
   key.userclip_active = rs_state->clip_plane_enable != 0;
   const struct brw_wm_prog_data *wm_prog_data = brw_wm_prog_data(ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data);
   if (wm_prog_data) {
      key.contains_flat_varying = wm_prog_data->contains_flat_varying;
      memcpy(key.interp_mode, wm_prog_data->interp_mode, sizeof(key.interp_mode));
   }

   key.do_twoside_color = rs_state->light_twoside;

   key.do_point_sprite = rs_state->point_quad_rasterization;
   if (key.do_point_sprite) {
      key.point_sprite_coord_replace = rs_state->sprite_coord_enable & 0xff;
      if (rs_state->sprite_coord_enable & (1 << 8))
         key.do_point_coord = 1;
      if (wm_prog_data && wm_prog_data->urb_setup[VARYING_SLOT_PNTC] != -1)
         key.do_point_coord = 1;
   }

   key.sprite_origin_lower_left = rs_state->sprite_coord_mode == PIPE_SPRITE_COORD_LOWER_LEFT;

   if (key.do_twoside_color) {
      key.frontface_ccw = rs_state->front_ccw;
   }
   struct crocus_compiled_shader *shader =
      crocus_find_cached_shader(ice, CROCUS_CACHE_SF, sizeof(key), &key);

   if (!shader)
      shader = crocus_compile_sf(ice, &key);

   if (old != shader) {
      ice->state.dirty |= CROCUS_DIRTY_RASTER;
      ice->shaders.sf_prog = shader;
   }
}

static struct crocus_compiled_shader *
crocus_compile_ff_gs(struct crocus_context *ice, struct brw_ff_gs_prog_key *key)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   struct brw_compiler *compiler = screen->compiler;
   void *mem_ctx;
   unsigned program_size;
   mem_ctx = ralloc_context(NULL);

   struct brw_ff_gs_prog_data *ff_gs_prog_data =
      rzalloc(mem_ctx, struct brw_ff_gs_prog_data);

   const unsigned *program = brw_compile_ff_gs_prog(compiler, mem_ctx, key, ff_gs_prog_data,
                                                    ice->shaders.last_vue_map, &program_size);

   if (program == NULL) {
      dbg_printf("failed to compile sf shader\n");
      ralloc_free(mem_ctx);
      return false;
   }

   struct crocus_binding_table bt;
   memset(&bt, 0, sizeof(bt));

   if (screen->devinfo.ver == 6) {
      bt.sizes[CROCUS_SURFACE_GROUP_SOL] = BRW_MAX_SOL_BINDINGS;
      bt.used_mask[CROCUS_SURFACE_GROUP_SOL] = (uint64_t)-1;

      bt.size_bytes = BRW_MAX_SOL_BINDINGS * 4;
   }

   struct crocus_compiled_shader *shader =
      crocus_upload_shader(ice, CROCUS_CACHE_FF_GS, sizeof(*key), key, program,
                           program_size,
                           (struct brw_stage_prog_data *)ff_gs_prog_data, sizeof(*ff_gs_prog_data),
                           NULL, NULL, 0, 0, &bt);
   ralloc_free(mem_ctx);
   return shader;
}

static void
crocus_update_compiled_ff_gs(struct crocus_context *ice)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct brw_ff_gs_prog_key key;
   struct crocus_compiled_shader *old = ice->shaders.ff_gs_prog;
   memset(&key, 0, sizeof(key));

   assert(devinfo->ver < 7);

   key.attrs = ice->shaders.last_vue_map->slots_valid;

   key.primitive = screen->vtbl.translate_prim_type(ice->state.prim_mode, 0);

   struct pipe_rasterizer_state *rs_state = crocus_get_rast_state(ice);
   key.pv_first = rs_state->flatshade_first;

   if (key.primitive == _3DPRIM_QUADLIST && !rs_state->flatshade) {
      /* Provide consistenbbbbbt primitive order with brw_set_prim's
       * optimization of single quads to trifans.
       */
      key.pv_first = true;
   }

   if (devinfo->ver >= 6) {
      key.need_gs_prog = ice->state.streamout_active;
      if (key.need_gs_prog) {
         struct crocus_uncompiled_shader *vs =
            ice->shaders.uncompiled[MESA_SHADER_VERTEX];
         gfx6_ff_gs_xfb_setup(&vs->stream_output,
                              &key);
      }
   } else {
      key.need_gs_prog = (key.primitive == _3DPRIM_QUADLIST ||
                          key.primitive == _3DPRIM_QUADSTRIP ||
                          key.primitive == _3DPRIM_LINELOOP);
   }

   struct crocus_compiled_shader *shader = NULL;
   if (key.need_gs_prog) {
      shader = crocus_find_cached_shader(ice, CROCUS_CACHE_FF_GS,
                                         sizeof(key), &key);
      if (!shader)
         shader = crocus_compile_ff_gs(ice, &key);
   }
   if (old != shader) {
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_GS;
      if (!!old != !!shader)
         ice->state.dirty |= CROCUS_DIRTY_GEN6_URB;
      ice->shaders.ff_gs_prog = shader;
      if (shader) {
         const struct brw_ff_gs_prog_data *gs_prog_data = (struct brw_ff_gs_prog_data *)ice->shaders.ff_gs_prog->prog_data;
         ice->state.last_xfb_verts_per_prim = gs_prog_data->svbi_postincrement_value;
      }
   }
}

// XXX: crocus_compiled_shaders are space-leaking :(
// XXX: do remember to unbind them if deleting them.

/**
 * Update the current shader variants for the given state.
 *
 * This should be called on every draw call to ensure that the correct
 * shaders are bound.  It will also flag any dirty state triggered by
 * swapping out those shaders.
 */
bool
crocus_update_compiled_shaders(struct crocus_context *ice)
{
   struct crocus_screen *screen = (void *) ice->ctx.screen;
   const uint64_t stage_dirty = ice->state.stage_dirty;

   struct brw_vue_prog_data *old_prog_datas[4];
   if (!(ice->state.dirty & CROCUS_DIRTY_GEN6_URB)) {
      for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++)
         old_prog_datas[i] = get_vue_prog_data(ice, i);
   }

   if (stage_dirty & (CROCUS_STAGE_DIRTY_UNCOMPILED_TCS |
                      CROCUS_STAGE_DIRTY_UNCOMPILED_TES)) {
      struct crocus_uncompiled_shader *tes =
         ice->shaders.uncompiled[MESA_SHADER_TESS_EVAL];
      if (tes) {
         crocus_update_compiled_tcs(ice);
         crocus_update_compiled_tes(ice);
      } else {
         ice->shaders.prog[CROCUS_CACHE_TCS] = NULL;
         ice->shaders.prog[CROCUS_CACHE_TES] = NULL;
         ice->state.stage_dirty |=
            CROCUS_STAGE_DIRTY_TCS | CROCUS_STAGE_DIRTY_TES |
            CROCUS_STAGE_DIRTY_BINDINGS_TCS | CROCUS_STAGE_DIRTY_BINDINGS_TES |
            CROCUS_STAGE_DIRTY_CONSTANTS_TCS | CROCUS_STAGE_DIRTY_CONSTANTS_TES;
      }
   }

   if (stage_dirty & CROCUS_STAGE_DIRTY_UNCOMPILED_VS)
      crocus_update_compiled_vs(ice);
   if (stage_dirty & CROCUS_STAGE_DIRTY_UNCOMPILED_GS)
      crocus_update_compiled_gs(ice);

   if (stage_dirty & (CROCUS_STAGE_DIRTY_UNCOMPILED_GS |
                      CROCUS_STAGE_DIRTY_UNCOMPILED_TES)) {
      const struct crocus_compiled_shader *gs =
         ice->shaders.prog[MESA_SHADER_GEOMETRY];
      const struct crocus_compiled_shader *tes =
         ice->shaders.prog[MESA_SHADER_TESS_EVAL];

      bool points_or_lines = false;

      if (gs) {
         const struct brw_gs_prog_data *gs_prog_data = (void *) gs->prog_data;
         points_or_lines =
            gs_prog_data->output_topology == _3DPRIM_POINTLIST ||
            gs_prog_data->output_topology == _3DPRIM_LINESTRIP;
      } else if (tes) {
         const struct brw_tes_prog_data *tes_data = (void *) tes->prog_data;
         points_or_lines =
            tes_data->output_topology == BRW_TESS_OUTPUT_TOPOLOGY_LINE ||
            tes_data->output_topology == BRW_TESS_OUTPUT_TOPOLOGY_POINT;
      }

      if (ice->shaders.output_topology_is_points_or_lines != points_or_lines) {
         /* Outbound to XY Clip enables */
         ice->shaders.output_topology_is_points_or_lines = points_or_lines;
         ice->state.dirty |= CROCUS_DIRTY_CLIP;
      }
   }

   if (!ice->shaders.prog[MESA_SHADER_VERTEX])
      return false;

   gl_shader_stage last_stage = last_vue_stage(ice);
   struct crocus_compiled_shader *shader = ice->shaders.prog[last_stage];
   struct crocus_uncompiled_shader *ish = ice->shaders.uncompiled[last_stage];
   update_last_vue_map(ice, shader->prog_data);
   if (ice->state.streamout != shader->streamout) {
      ice->state.streamout = shader->streamout;
      ice->state.dirty |= CROCUS_DIRTY_SO_DECL_LIST | CROCUS_DIRTY_STREAMOUT;
   }

   if (ice->state.streamout_active) {
      screen->vtbl.update_so_strides(ice, ish->stream_output.stride);
   }

   /* use ice->state version as last_vue_map can dirty this bit */
   if (ice->state.stage_dirty & CROCUS_STAGE_DIRTY_UNCOMPILED_FS)
      crocus_update_compiled_fs(ice);

   if (screen->devinfo.ver <= 6) {
      if (ice->state.dirty & CROCUS_DIRTY_GEN4_FF_GS_PROG &&
          !ice->shaders.prog[MESA_SHADER_GEOMETRY])
         crocus_update_compiled_ff_gs(ice);
   }

   if (screen->devinfo.ver < 6) {
      if (ice->state.dirty & CROCUS_DIRTY_GEN4_CLIP_PROG)
         crocus_update_compiled_clip(ice);
      if (ice->state.dirty & CROCUS_DIRTY_GEN4_SF_PROG)
         crocus_update_compiled_sf(ice);
   }


   /* Changing shader interfaces may require a URB configuration. */
   if (!(ice->state.dirty & CROCUS_DIRTY_GEN6_URB)) {
      for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
         struct brw_vue_prog_data *old = old_prog_datas[i];
         struct brw_vue_prog_data *new = get_vue_prog_data(ice, i);
         if (!!old != !!new ||
             (new && new->urb_entry_size != old->urb_entry_size)) {
            ice->state.dirty |= CROCUS_DIRTY_GEN6_URB;
            break;
         }
      }
   }

   if (ice->state.stage_dirty & CROCUS_RENDER_STAGE_DIRTY_CONSTANTS) {
      for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_FRAGMENT; i++) {
         if (ice->state.stage_dirty & (CROCUS_STAGE_DIRTY_CONSTANTS_VS << i))
            crocus_update_pull_constant_descriptors(ice, i);
      }
   }
   return true;
}

static struct crocus_compiled_shader *
crocus_compile_cs(struct crocus_context *ice,
                  struct crocus_uncompiled_shader *ish,
                  const struct brw_cs_prog_key *key)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct brw_compiler *compiler = screen->compiler;
   void *mem_ctx = ralloc_context(NULL);
   struct brw_cs_prog_data *cs_prog_data =
      rzalloc(mem_ctx, struct brw_cs_prog_data);
   struct brw_stage_prog_data *prog_data = &cs_prog_data->base;
   enum brw_param_builtin *system_values;
   const struct intel_device_info *devinfo = &screen->devinfo;
   unsigned num_system_values;
   unsigned num_cbufs;

   nir_shader *nir = nir_shader_clone(mem_ctx, ish->nir);

   NIR_PASS_V(nir, brw_nir_lower_cs_intrinsics);

   crocus_setup_uniforms(devinfo, mem_ctx, nir, prog_data, &system_values,
                         &num_system_values, &num_cbufs);
   crocus_lower_swizzles(nir, &key->base.tex);
   struct crocus_binding_table bt;
   crocus_setup_binding_table(devinfo, nir, &bt, /* num_render_targets */ 0,
                              num_system_values, num_cbufs, &key->base.tex);

   struct brw_compile_cs_params params = {
      .base = {
         .mem_ctx = mem_ctx,
         .nir = nir,
         .log_data = &ice->dbg,
      },
      .key = key,
      .prog_data = cs_prog_data,
   };

   const unsigned *program =
      brw_compile_cs(compiler, &params);
   if (program == NULL) {
      dbg_printf("Failed to compile compute shader: %s\n", params.base.error_str);
      ralloc_free(mem_ctx);
      return false;
   }

   if (ish->compiled_once) {
      crocus_debug_recompile(ice, &nir->info, &key->base);
   } else {
      ish->compiled_once = true;
   }

   struct crocus_compiled_shader *shader =
      crocus_upload_shader(ice, CROCUS_CACHE_CS, sizeof(*key), key, program,
                           prog_data->program_size,
                           prog_data, sizeof(*cs_prog_data), NULL,
                           system_values, num_system_values,
                           num_cbufs, &bt);

   crocus_disk_cache_store(screen->disk_cache, ish, shader,
                           ice->shaders.cache_bo_map,
                           key, sizeof(*key));

   ralloc_free(mem_ctx);
   return shader;
}

static void
crocus_update_compiled_cs(struct crocus_context *ice)
{
   struct crocus_shader_state *shs = &ice->state.shaders[MESA_SHADER_COMPUTE];
   struct crocus_uncompiled_shader *ish =
      ice->shaders.uncompiled[MESA_SHADER_COMPUTE];
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct brw_cs_prog_key key = { KEY_INIT() };

   if (ish->nos & (1ull << CROCUS_NOS_TEXTURES))
      crocus_populate_sampler_prog_key_data(ice, devinfo, MESA_SHADER_COMPUTE, ish,
                                            ish->nir->info.uses_texture_gather, &key.base.tex);
   screen->vtbl.populate_cs_key(ice, &key);

   struct crocus_compiled_shader *old = ice->shaders.prog[CROCUS_CACHE_CS];
   struct crocus_compiled_shader *shader =
      crocus_find_cached_shader(ice, CROCUS_CACHE_CS, sizeof(key), &key);

   if (!shader)
      shader = crocus_disk_cache_retrieve(ice, ish, &key, sizeof(key));

   if (!shader)
      shader = crocus_compile_cs(ice, ish, &key);

   if (old != shader) {
      ice->shaders.prog[CROCUS_CACHE_CS] = shader;
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CS |
                          CROCUS_STAGE_DIRTY_BINDINGS_CS |
                          CROCUS_STAGE_DIRTY_CONSTANTS_CS;
      shs->sysvals_need_upload = true;
   }
}

void
crocus_update_compiled_compute_shader(struct crocus_context *ice)
{
   if (ice->state.stage_dirty & CROCUS_STAGE_DIRTY_UNCOMPILED_CS)
      crocus_update_compiled_cs(ice);

   if (ice->state.stage_dirty & CROCUS_STAGE_DIRTY_CONSTANTS_CS)
      crocus_update_pull_constant_descriptors(ice, MESA_SHADER_COMPUTE);
}

void
crocus_fill_cs_push_const_buffer(struct brw_cs_prog_data *cs_prog_data,
                                 unsigned threads,
                                 uint32_t *dst)
{
   assert(brw_cs_push_const_total_size(cs_prog_data, threads) > 0);
   assert(cs_prog_data->push.cross_thread.size == 0);
   assert(cs_prog_data->push.per_thread.dwords == 1);
   assert(cs_prog_data->base.param[0] == BRW_PARAM_BUILTIN_SUBGROUP_ID);
   for (unsigned t = 0; t < threads; t++)
      dst[8 * t] = t;
}

/**
 * Allocate scratch BOs as needed for the given per-thread size and stage.
 */
struct crocus_bo *
crocus_get_scratch_space(struct crocus_context *ice,
                         unsigned per_thread_scratch,
                         gl_shader_stage stage)
{
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   struct crocus_bufmgr *bufmgr = screen->bufmgr;
   const struct intel_device_info *devinfo = &screen->devinfo;

   unsigned encoded_size = ffs(per_thread_scratch) - 11;
   assert(encoded_size < (1 << 16));

   struct crocus_bo **bop = &ice->shaders.scratch_bos[encoded_size][stage];

   if (!*bop) {
      assert(stage < ARRAY_SIZE(devinfo->max_scratch_ids));
      uint32_t size = per_thread_scratch * devinfo->max_scratch_ids[stage];
      *bop = crocus_bo_alloc(bufmgr, "scratch", size);
   }

   return *bop;
}

/* ------------------------------------------------------------------- */

/**
 * The pipe->create_[stage]_state() driver hooks.
 *
 * Performs basic NIR preprocessing, records any state dependencies, and
 * returns an crocus_uncompiled_shader as the Gallium CSO.
 *
 * Actual shader compilation to assembly happens later, at first use.
 */
static void *
crocus_create_uncompiled_shader(struct pipe_context *ctx,
                                nir_shader *nir,
                                const struct pipe_stream_output_info *so_info)
{
   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct crocus_uncompiled_shader *ish =
      calloc(1, sizeof(struct crocus_uncompiled_shader));
   if (!ish)
      return NULL;

   if (devinfo->ver >= 6)
     NIR_PASS(ish->needs_edge_flag, nir, crocus_fix_edge_flags);
   else
     ish->needs_edge_flag = false;

   struct brw_nir_compiler_opts opts = {};
   brw_preprocess_nir(screen->compiler, nir, &opts);

   NIR_PASS_V(nir, brw_nir_lower_storage_image,
              &(struct brw_nir_lower_storage_image_opts) {
                 .devinfo = devinfo,
                 .lower_loads = true,
                 .lower_stores = true,
                 .lower_atomics = true,
                 .lower_get_size = true,
              });
   NIR_PASS_V(nir, crocus_lower_storage_image_derefs);

   nir_sweep(nir);

   ish->program_id = get_new_program_id(screen);
   ish->nir = nir;
   if (so_info) {
      memcpy(&ish->stream_output, so_info, sizeof(*so_info));
      update_so_info(&ish->stream_output, nir->info.outputs_written);
   }

   if (screen->disk_cache) {
      /* Serialize the NIR to a binary blob that we can hash for the disk
       * cache.  Drop unnecessary information (like variable names)
       * so the serialized NIR is smaller, and also to let us detect more
       * isomorphic shaders when hashing, increasing cache hits.
       */
      struct blob blob;
      blob_init(&blob);
      nir_serialize(&blob, nir, true);
      _mesa_sha1_compute(blob.data, blob.size, ish->nir_sha1);
      blob_finish(&blob);
   }

   return ish;
}

static struct crocus_uncompiled_shader *
crocus_create_shader_state(struct pipe_context *ctx,
                           const struct pipe_shader_state *state)
{
   struct nir_shader *nir;

   if (state->type == PIPE_SHADER_IR_TGSI)
      nir = tgsi_to_nir(state->tokens, ctx->screen, false);
   else
      nir = state->ir.nir;

   return crocus_create_uncompiled_shader(ctx, nir, &state->stream_output);
}

static void *
crocus_create_vs_state(struct pipe_context *ctx,
                       const struct pipe_shader_state *state)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_screen *screen = (void *) ctx->screen;
   struct crocus_uncompiled_shader *ish = crocus_create_shader_state(ctx, state);

   ish->nos |= (1ull << CROCUS_NOS_TEXTURES);
   /* User clip planes or gen5 sprite coord enable */
   if (ish->nir->info.clip_distance_array_size == 0 ||
       screen->devinfo.ver <= 5)
      ish->nos |= (1ull << CROCUS_NOS_RASTERIZER);

   if (screen->devinfo.verx10 < 75)
      ish->nos |= (1ull << CROCUS_NOS_VERTEX_ELEMENTS);

   if (screen->precompile) {
      struct brw_vs_prog_key key = { KEY_INIT() };

      if (!crocus_disk_cache_retrieve(ice, ish, &key, sizeof(key)))
         crocus_compile_vs(ice, ish, &key);
   }

   return ish;
}

static void *
crocus_create_tcs_state(struct pipe_context *ctx,
                        const struct pipe_shader_state *state)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_screen *screen = (void *) ctx->screen;
   struct crocus_uncompiled_shader *ish = crocus_create_shader_state(ctx, state);
   struct shader_info *info = &ish->nir->info;

   ish->nos |= (1ull << CROCUS_NOS_TEXTURES);
   if (screen->precompile) {
      struct brw_tcs_prog_key key = {
         KEY_INIT(),
         // XXX: make sure the linker fills this out from the TES...
         ._tes_primitive_mode =
            info->tess._primitive_mode ? info->tess._primitive_mode
                                      : TESS_PRIMITIVE_TRIANGLES,
         .outputs_written = info->outputs_written,
         .patch_outputs_written = info->patch_outputs_written,
      };

      key.input_vertices = info->tess.tcs_vertices_out;

      if (!crocus_disk_cache_retrieve(ice, ish, &key, sizeof(key)))
         crocus_compile_tcs(ice, ish, &key);
   }

   return ish;
}

static void *
crocus_create_tes_state(struct pipe_context *ctx,
                        const struct pipe_shader_state *state)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_screen *screen = (void *) ctx->screen;
   struct crocus_uncompiled_shader *ish = crocus_create_shader_state(ctx, state);
   struct shader_info *info = &ish->nir->info;

   ish->nos |= (1ull << CROCUS_NOS_TEXTURES);
   /* User clip planes */
   if (ish->nir->info.clip_distance_array_size == 0)
      ish->nos |= (1ull << CROCUS_NOS_RASTERIZER);

   if (screen->precompile) {
      struct brw_tes_prog_key key = {
         KEY_INIT(),
         // XXX: not ideal, need TCS output/TES input unification
         .inputs_read = info->inputs_read,
         .patch_inputs_read = info->patch_inputs_read,
      };

      if (!crocus_disk_cache_retrieve(ice, ish, &key, sizeof(key)))
         crocus_compile_tes(ice, ish, &key);
   }

   return ish;
}

static void *
crocus_create_gs_state(struct pipe_context *ctx,
                       const struct pipe_shader_state *state)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_screen *screen = (void *) ctx->screen;
   struct crocus_uncompiled_shader *ish = crocus_create_shader_state(ctx, state);

   ish->nos |= (1ull << CROCUS_NOS_TEXTURES);
   /* User clip planes */
   if (ish->nir->info.clip_distance_array_size == 0)
      ish->nos |= (1ull << CROCUS_NOS_RASTERIZER);

   if (screen->precompile) {
      struct brw_gs_prog_key key = { KEY_INIT() };

      if (!crocus_disk_cache_retrieve(ice, ish, &key, sizeof(key)))
         crocus_compile_gs(ice, ish, &key);
   }

   return ish;
}

static void *
crocus_create_fs_state(struct pipe_context *ctx,
                       const struct pipe_shader_state *state)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_screen *screen = (void *) ctx->screen;
   struct crocus_uncompiled_shader *ish = crocus_create_shader_state(ctx, state);
   struct shader_info *info = &ish->nir->info;

   ish->nos |= (1ull << CROCUS_NOS_FRAMEBUFFER) |
               (1ull << CROCUS_NOS_DEPTH_STENCIL_ALPHA) |
               (1ull << CROCUS_NOS_RASTERIZER) |
               (1ull << CROCUS_NOS_TEXTURES) |
               (1ull << CROCUS_NOS_BLEND);

   /* The program key needs the VUE map if there are > 16 inputs or gen4/5 */
   if (screen->devinfo.ver < 6 || util_bitcount64(ish->nir->info.inputs_read &
                                                  BRW_FS_VARYING_INPUT_MASK) > 16) {
      ish->nos |= (1ull << CROCUS_NOS_LAST_VUE_MAP);
   }

   if (screen->precompile) {
      const uint64_t color_outputs = info->outputs_written &
         ~(BITFIELD64_BIT(FRAG_RESULT_DEPTH) |
           BITFIELD64_BIT(FRAG_RESULT_STENCIL) |
           BITFIELD64_BIT(FRAG_RESULT_SAMPLE_MASK));

      bool can_rearrange_varyings =
         screen->devinfo.ver > 6 && util_bitcount64(info->inputs_read & BRW_FS_VARYING_INPUT_MASK) <= 16;

      const struct intel_device_info *devinfo = &screen->devinfo;
      struct brw_wm_prog_key key = {
         KEY_INIT(),
         .nr_color_regions = util_bitcount(color_outputs),
         .coherent_fb_fetch = false,
         .ignore_sample_mask_out = screen->devinfo.ver < 6 ? 1 : 0,
         .input_slots_valid =
         can_rearrange_varyings ? 0 : info->inputs_read | VARYING_BIT_POS,
      };

      struct brw_vue_map vue_map;
      if (devinfo->ver < 6) {
         brw_compute_vue_map(devinfo, &vue_map,
                             info->inputs_read | VARYING_BIT_POS,
                             false, /* pos slots */ 1);
      }
      if (!crocus_disk_cache_retrieve(ice, ish, &key, sizeof(key)))
         crocus_compile_fs(ice, ish, &key, &vue_map);
   }

   return ish;
}

static void *
crocus_create_compute_state(struct pipe_context *ctx,
                            const struct pipe_compute_state *state)
{
   assert(state->ir_type == PIPE_SHADER_IR_NIR);

   struct crocus_context *ice = (void *) ctx;
   struct crocus_screen *screen = (void *) ctx->screen;
   struct crocus_uncompiled_shader *ish =
      crocus_create_uncompiled_shader(ctx, (void *) state->prog, NULL);

   ish->nos |= (1ull << CROCUS_NOS_TEXTURES);
   // XXX: disallow more than 64KB of shared variables

   if (screen->precompile) {
      struct brw_cs_prog_key key = { KEY_INIT() };

      if (!crocus_disk_cache_retrieve(ice, ish, &key, sizeof(key)))
         crocus_compile_cs(ice, ish, &key);
   }

   return ish;
}

/**
 * The pipe->delete_[stage]_state() driver hooks.
 *
 * Frees the crocus_uncompiled_shader.
 */
static void
crocus_delete_shader_state(struct pipe_context *ctx, void *state, gl_shader_stage stage)
{
   struct crocus_uncompiled_shader *ish = state;
   struct crocus_context *ice = (void *) ctx;

   if (ice->shaders.uncompiled[stage] == ish) {
      ice->shaders.uncompiled[stage] = NULL;
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_UNCOMPILED_VS << stage;
   }

   if (ish->const_data) {
      pipe_resource_reference(&ish->const_data, NULL);
      pipe_resource_reference(&ish->const_data_state.res, NULL);
   }

   ralloc_free(ish->nir);
   free(ish);
}

static void
crocus_delete_vs_state(struct pipe_context *ctx, void *state)
{
   crocus_delete_shader_state(ctx, state, MESA_SHADER_VERTEX);
}

static void
crocus_delete_tcs_state(struct pipe_context *ctx, void *state)
{
   crocus_delete_shader_state(ctx, state, MESA_SHADER_TESS_CTRL);
}

static void
crocus_delete_tes_state(struct pipe_context *ctx, void *state)
{
   crocus_delete_shader_state(ctx, state, MESA_SHADER_TESS_EVAL);
}

static void
crocus_delete_gs_state(struct pipe_context *ctx, void *state)
{
   crocus_delete_shader_state(ctx, state, MESA_SHADER_GEOMETRY);
}

static void
crocus_delete_fs_state(struct pipe_context *ctx, void *state)
{
   crocus_delete_shader_state(ctx, state, MESA_SHADER_FRAGMENT);
}

static void
crocus_delete_cs_state(struct pipe_context *ctx, void *state)
{
   crocus_delete_shader_state(ctx, state, MESA_SHADER_COMPUTE);
}

/**
 * The pipe->bind_[stage]_state() driver hook.
 *
 * Binds an uncompiled shader as the current one for a particular stage.
 * Updates dirty tracking to account for the shader's NOS.
 */
static void
bind_shader_state(struct crocus_context *ice,
                  struct crocus_uncompiled_shader *ish,
                  gl_shader_stage stage)
{
   uint64_t dirty_bit = CROCUS_STAGE_DIRTY_UNCOMPILED_VS << stage;
   const uint64_t nos = ish ? ish->nos : 0;

   const struct shader_info *old_info = crocus_get_shader_info(ice, stage);
   const struct shader_info *new_info = ish ? &ish->nir->info : NULL;

   if ((old_info ? BITSET_LAST_BIT(old_info->textures_used) : 0) !=
       (new_info ? BITSET_LAST_BIT(new_info->textures_used) : 0)) {
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_SAMPLER_STATES_VS << stage;
   }

   ice->shaders.uncompiled[stage] = ish;
   ice->state.stage_dirty |= dirty_bit;

   /* Record that CSOs need to mark CROCUS_DIRTY_UNCOMPILED_XS when they change
    * (or that they no longer need to do so).
    */
   for (int i = 0; i < CROCUS_NOS_COUNT; i++) {
      if (nos & (1 << i))
         ice->state.stage_dirty_for_nos[i] |= dirty_bit;
      else
         ice->state.stage_dirty_for_nos[i] &= ~dirty_bit;
   }
}

static void
crocus_bind_vs_state(struct pipe_context *ctx, void *state)
{
   struct crocus_context *ice = (struct crocus_context *)ctx;
   struct crocus_uncompiled_shader *new_ish = state;
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   if (new_ish &&
       ice->state.window_space_position !=
       new_ish->nir->info.vs.window_space_position) {
      ice->state.window_space_position =
         new_ish->nir->info.vs.window_space_position;

      ice->state.dirty |= CROCUS_DIRTY_CLIP |
                          CROCUS_DIRTY_RASTER |
                          CROCUS_DIRTY_CC_VIEWPORT;
   }

   if (devinfo->ver == 6) {
      ice->state.stage_dirty |= CROCUS_DIRTY_GEN4_FF_GS_PROG;
   }

   bind_shader_state((void *) ctx, state, MESA_SHADER_VERTEX);
}

static void
crocus_bind_tcs_state(struct pipe_context *ctx, void *state)
{
   bind_shader_state((void *) ctx, state, MESA_SHADER_TESS_CTRL);
}

static void
crocus_bind_tes_state(struct pipe_context *ctx, void *state)
{
   struct crocus_context *ice = (struct crocus_context *)ctx;

   /* Enabling/disabling optional stages requires a URB reconfiguration. */
   if (!!state != !!ice->shaders.uncompiled[MESA_SHADER_TESS_EVAL])
      ice->state.dirty |= CROCUS_DIRTY_GEN6_URB;

   bind_shader_state((void *) ctx, state, MESA_SHADER_TESS_EVAL);
}

static void
crocus_bind_gs_state(struct pipe_context *ctx, void *state)
{
   struct crocus_context *ice = (struct crocus_context *)ctx;

   /* Enabling/disabling optional stages requires a URB reconfiguration. */
   if (!!state != !!ice->shaders.uncompiled[MESA_SHADER_GEOMETRY])
      ice->state.dirty |= CROCUS_DIRTY_GEN6_URB;

   bind_shader_state((void *) ctx, state, MESA_SHADER_GEOMETRY);
}

static void
crocus_bind_fs_state(struct pipe_context *ctx, void *state)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_screen *screen = (struct crocus_screen *) ctx->screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct crocus_uncompiled_shader *old_ish =
      ice->shaders.uncompiled[MESA_SHADER_FRAGMENT];
   struct crocus_uncompiled_shader *new_ish = state;

   const unsigned color_bits =
      BITFIELD64_BIT(FRAG_RESULT_COLOR) |
      BITFIELD64_RANGE(FRAG_RESULT_DATA0, BRW_MAX_DRAW_BUFFERS);

   /* Fragment shader outputs influence HasWriteableRT */
   if (!old_ish || !new_ish ||
       (old_ish->nir->info.outputs_written & color_bits) !=
       (new_ish->nir->info.outputs_written & color_bits)) {
      if (devinfo->ver == 8)
         ice->state.dirty |= CROCUS_DIRTY_GEN8_PS_BLEND;
      else
         ice->state.dirty |= CROCUS_DIRTY_WM;
   }

   if (devinfo->ver == 8)
      ice->state.dirty |= CROCUS_DIRTY_GEN8_PMA_FIX;
   bind_shader_state((void *) ctx, state, MESA_SHADER_FRAGMENT);
}

static void
crocus_bind_cs_state(struct pipe_context *ctx, void *state)
{
   bind_shader_state((void *) ctx, state, MESA_SHADER_COMPUTE);
}

void
crocus_init_program_functions(struct pipe_context *ctx)
{
   ctx->create_vs_state  = crocus_create_vs_state;
   ctx->create_tcs_state = crocus_create_tcs_state;
   ctx->create_tes_state = crocus_create_tes_state;
   ctx->create_gs_state  = crocus_create_gs_state;
   ctx->create_fs_state  = crocus_create_fs_state;
   ctx->create_compute_state = crocus_create_compute_state;

   ctx->delete_vs_state  = crocus_delete_vs_state;
   ctx->delete_tcs_state = crocus_delete_tcs_state;
   ctx->delete_tes_state = crocus_delete_tes_state;
   ctx->delete_gs_state  = crocus_delete_gs_state;
   ctx->delete_fs_state  = crocus_delete_fs_state;
   ctx->delete_compute_state = crocus_delete_cs_state;

   ctx->bind_vs_state  = crocus_bind_vs_state;
   ctx->bind_tcs_state = crocus_bind_tcs_state;
   ctx->bind_tes_state = crocus_bind_tes_state;
   ctx->bind_gs_state  = crocus_bind_gs_state;
   ctx->bind_fs_state  = crocus_bind_fs_state;
   ctx->bind_compute_state = crocus_bind_cs_state;
}
