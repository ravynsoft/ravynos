/*
 * Copyright © 2022 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "nak_private.h"
#include "nir_builder.h"
#include "nir_xfb_info.h"

#include "util/u_math.h"

#define OPT(nir, pass, ...) ({                           \
   bool this_progress = false;                           \
   NIR_PASS(this_progress, nir, pass, ##__VA_ARGS__);    \
   if (this_progress)                                    \
      progress = true;                                   \
   this_progress;                                        \
})

#define OPT_V(nir, pass, ...) NIR_PASS_V(nir, pass, ##__VA_ARGS__)

bool
nak_nir_workgroup_has_one_subgroup(const nir_shader *nir)
{
   switch (nir->info.stage) {
   case MESA_SHADER_VERTEX:
   case MESA_SHADER_TESS_EVAL:
   case MESA_SHADER_GEOMETRY:
   case MESA_SHADER_FRAGMENT:
      unreachable("Shader stage does not have workgroups");
      break;

   case MESA_SHADER_TESS_CTRL:
      /* Tessellation only ever has one subgroup per workgroup.  The Vulkan
       * limit on the number of tessellation invocations is 32 to allow for
       * this.
       */
      return true;

   case MESA_SHADER_COMPUTE:
   case MESA_SHADER_KERNEL: {
      if (nir->info.workgroup_size_variable)
         return false;

      uint16_t wg_sz = nir->info.workgroup_size[0] *
                       nir->info.workgroup_size[1] *
                       nir->info.workgroup_size[2];

      return wg_sz <= 32;
   }

   default:
      unreachable("Unknown shader stage");
   }
}

static void
optimize_nir(nir_shader *nir, const struct nak_compiler *nak, bool allow_copies)
{
   bool progress;

   unsigned lower_flrp =
      (nir->options->lower_flrp16 ? 16 : 0) |
      (nir->options->lower_flrp32 ? 32 : 0) |
      (nir->options->lower_flrp64 ? 64 : 0);

   do {
      progress = false;

      /* This pass is causing problems with types used by OpenCL :
       *    https://gitlab.freedesktop.org/mesa/mesa/-/merge_requests/13955
       *
       * Running with it disabled made no difference in the resulting assembly
       * code.
       */
      if (nir->info.stage != MESA_SHADER_KERNEL)
         OPT(nir, nir_split_array_vars, nir_var_function_temp);

      OPT(nir, nir_shrink_vec_array_vars, nir_var_function_temp);
      OPT(nir, nir_opt_deref);
      if (OPT(nir, nir_opt_memcpy))
         OPT(nir, nir_split_var_copies);

      OPT(nir, nir_lower_vars_to_ssa);

      if (allow_copies) {
         /* Only run this pass in the first call to brw_nir_optimize.  Later
          * calls assume that we've lowered away any copy_deref instructions
          * and we don't want to introduce any more.
          */
         OPT(nir, nir_opt_find_array_copies);
      }
      OPT(nir, nir_opt_copy_prop_vars);
      OPT(nir, nir_opt_dead_write_vars);
      OPT(nir, nir_opt_combine_stores, nir_var_all);

      OPT(nir, nir_lower_alu_to_scalar, NULL, NULL);
      OPT(nir, nir_lower_phis_to_scalar, false);
      OPT(nir, nir_lower_frexp);
      OPT(nir, nir_copy_prop);
      OPT(nir, nir_opt_dce);
      OPT(nir, nir_opt_cse);

      OPT(nir, nir_opt_peephole_select, 0, false, false);
      OPT(nir, nir_opt_intrinsics);
      OPT(nir, nir_opt_idiv_const, 32);
      OPT(nir, nir_opt_algebraic);
      OPT(nir, nir_lower_constant_convert_alu_types);
      OPT(nir, nir_opt_constant_folding);

      if (lower_flrp != 0) {
         if (OPT(nir, nir_lower_flrp, lower_flrp, false /* always_precise */))
            OPT(nir, nir_opt_constant_folding);
         /* Nothing should rematerialize any flrps */
         lower_flrp = 0;
      }

      OPT(nir, nir_opt_dead_cf);
      if (OPT(nir, nir_opt_loop)) {
         /* If nir_opt_loop makes progress, then we need to clean things up
          * if we want any hope of nir_opt_if or nir_opt_loop_unroll to make
          * progress.
          */
         OPT(nir, nir_copy_prop);
         OPT(nir, nir_opt_dce);
      }
      OPT(nir, nir_opt_if, nir_opt_if_optimize_phi_true_false);
      OPT(nir, nir_opt_conditional_discard);
      if (nir->options->max_unroll_iterations != 0) {
         OPT(nir, nir_opt_loop_unroll);
      }
      OPT(nir, nir_opt_remove_phis);
      OPT(nir, nir_opt_gcm, false);
      OPT(nir, nir_opt_undef);
      OPT(nir, nir_lower_pack);
   } while (progress);

   OPT(nir, nir_remove_dead_variables, nir_var_function_temp, NULL);
}

void
nak_optimize_nir(nir_shader *nir, const struct nak_compiler *nak)
{
   optimize_nir(nir, nak, false);
}

static unsigned
lower_bit_size_cb(const nir_instr *instr, void *_data)
{
   switch (instr->type) {
   case nir_instr_type_alu: {
      nir_alu_instr *alu = nir_instr_as_alu(instr);
      if (nir_op_infos[alu->op].is_conversion)
         return 0;

      switch (alu->op) {
      case nir_op_bit_count:
      case nir_op_ufind_msb:
      case nir_op_ifind_msb:
      case nir_op_find_lsb:
         /* These are handled specially because the destination is always
          * 32-bit and so the bit size of the instruction is given by the
          * source.
          */
         return alu->src[0].src.ssa->bit_size == 32 ? 0 : 32;
      default:
         break;
      }

      const unsigned bit_size = nir_alu_instr_is_comparison(alu)
                                ? alu->src[0].src.ssa->bit_size
                                : alu->def.bit_size;
      if (bit_size >= 32)
         return 0;

      /* TODO: Some hardware has native 16-bit support */
      if (bit_size & (8 | 16))
         return 32;

      return 0;
   }

   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      switch (intrin->intrinsic) {
      case nir_intrinsic_vote_ieq:
         if (intrin->src[0].ssa->bit_size != 1 &&
             intrin->src[0].ssa->bit_size < 32)
            return 32;
         return 0;

      case nir_intrinsic_vote_feq:
      case nir_intrinsic_read_invocation:
      case nir_intrinsic_read_first_invocation:
      case nir_intrinsic_shuffle:
      case nir_intrinsic_shuffle_xor:
      case nir_intrinsic_shuffle_up:
      case nir_intrinsic_shuffle_down:
      case nir_intrinsic_quad_broadcast:
      case nir_intrinsic_quad_swap_horizontal:
      case nir_intrinsic_quad_swap_vertical:
      case nir_intrinsic_quad_swap_diagonal:
      case nir_intrinsic_reduce:
      case nir_intrinsic_inclusive_scan:
      case nir_intrinsic_exclusive_scan:
         if (intrin->src[0].ssa->bit_size < 32)
            return 32;
         return 0;

      default:
         return 0;
      }
   }

   case nir_instr_type_phi: {
      nir_phi_instr *phi = nir_instr_as_phi(instr);
      if (phi->def.bit_size < 32 && phi->def.bit_size != 1)
         return 32;
      return 0;
   }

   default:
      return 0;
   }
}

static nir_def *
nir_udiv_round_up(nir_builder *b, nir_def *n, nir_def *d)
{
   return nir_udiv(b, nir_iadd(b, n, nir_iadd_imm(b, d, -1)), d);
}

static bool
nak_nir_lower_subgroup_id_intrin(nir_builder *b, nir_intrinsic_instr *intrin,
                                 void *data)
{
   switch (intrin->intrinsic) {
   case nir_intrinsic_load_num_subgroups: {
      b->cursor = nir_instr_remove(&intrin->instr);

      nir_def *num_subgroups;
      if (nak_nir_workgroup_has_one_subgroup(b->shader)) {
         num_subgroups = nir_imm_int(b, 1);
      } else {
         assert(b->shader->info.cs.derivative_group == DERIVATIVE_GROUP_NONE);

         nir_def *workgroup_size = nir_load_workgroup_size(b);
         workgroup_size =
            nir_imul(b, nir_imul(b, nir_channel(b, workgroup_size, 0),
                                    nir_channel(b, workgroup_size, 1)),
                        nir_channel(b, workgroup_size, 2));
         nir_def *subgroup_size = nir_load_subgroup_size(b);
         num_subgroups = nir_udiv_round_up(b, workgroup_size, subgroup_size);
      }
      nir_def_rewrite_uses(&intrin->def, num_subgroups);

      return true;
   }
   case nir_intrinsic_load_subgroup_id: {
      b->cursor = nir_instr_remove(&intrin->instr);

      nir_def *subgroup_id;
      if (nak_nir_workgroup_has_one_subgroup(b->shader)) {
         subgroup_id = nir_imm_int(b, 0);
      } else {
         assert(b->shader->info.cs.derivative_group == DERIVATIVE_GROUP_NONE);

         nir_def *invocation_index = nir_load_local_invocation_index(b);
         nir_def *subgroup_size = nir_load_subgroup_size(b);
         subgroup_id = nir_udiv(b, invocation_index, subgroup_size);
      }
      nir_def_rewrite_uses(&intrin->def, subgroup_id);

      return true;
   }
   default:
      return false;
   }
}

static bool
nak_nir_lower_subgroup_id(nir_shader *nir)
{
   return nir_shader_intrinsics_pass(nir, nak_nir_lower_subgroup_id_intrin,
                                     nir_metadata_block_index |
                                     nir_metadata_dominance,
                                     NULL);
}

void
nak_preprocess_nir(nir_shader *nir, const struct nak_compiler *nak)
{
   UNUSED bool progress = false;

   nir_validate_ssa_dominance(nir, "before nak_preprocess_nir");

   const nir_lower_tex_options tex_options = {
      .lower_txd_3d = true,
      .lower_txd_cube_map = true,
      .lower_txd_clamp = true,
      .lower_txd_shadow = true,
      .lower_txp = ~0,
      /* TODO: More lowering */
   };
   OPT(nir, nir_lower_tex, &tex_options);
   OPT(nir, nir_normalize_cubemap_coords);

   nir_lower_image_options image_options = {
      .lower_cube_size = true,
   };
   OPT(nir, nir_lower_image, &image_options);

   OPT(nir, nir_lower_global_vars_to_local);

   OPT(nir, nir_split_var_copies);
   OPT(nir, nir_split_struct_vars, nir_var_function_temp);

   /* Optimize but allow copies because we haven't lowered them yet */
   optimize_nir(nir, nak, true /* allow_copies */);

   OPT(nir, nir_lower_load_const_to_scalar);
   OPT(nir, nir_lower_var_copies);
   OPT(nir, nir_lower_system_values);
   OPT(nir, nak_nir_lower_subgroup_id);
   OPT(nir, nir_lower_compute_system_values, NULL);
}

static uint16_t
nak_attribute_attr_addr(gl_vert_attrib attrib)
{
   assert(attrib >= VERT_ATTRIB_GENERIC0);
   return NAK_ATTR_GENERIC_START + (attrib - VERT_ATTRIB_GENERIC0) * 0x10;
}

static int
type_size_vec4_bytes(const struct glsl_type *type, bool bindless)
{
   return glsl_count_vec4_slots(type, false, bindless) * 16;
}

static bool
nak_nir_lower_vs_inputs(nir_shader *nir)
{
   bool progress = false;

   nir_foreach_shader_in_variable(var, nir) {
      var->data.driver_location =
         nak_attribute_attr_addr(var->data.location);
   }

   progress |= OPT(nir, nir_lower_io, nir_var_shader_in, type_size_vec4_bytes,
                        nir_lower_io_lower_64bit_to_32);

   return progress;
}

static uint16_t
nak_varying_attr_addr(gl_varying_slot slot)
{
   if (slot >= VARYING_SLOT_PATCH0) {
      return NAK_ATTR_PATCH_START + (slot - VARYING_SLOT_PATCH0) * 0x10;
   } else if (slot >= VARYING_SLOT_VAR0) {
      return NAK_ATTR_GENERIC_START + (slot - VARYING_SLOT_VAR0) * 0x10;
   } else {
      switch (slot) {
      case VARYING_SLOT_TESS_LEVEL_OUTER: return NAK_ATTR_TESS_LOD;
      case VARYING_SLOT_TESS_LEVEL_INNER: return NAK_ATTR_TESS_INTERRIOR;
      case VARYING_SLOT_PRIMITIVE_ID:     return NAK_ATTR_PRIMITIVE_ID;
      case VARYING_SLOT_LAYER:            return NAK_ATTR_RT_ARRAY_INDEX;
      case VARYING_SLOT_VIEWPORT:         return NAK_ATTR_VIEWPORT_INDEX;
      case VARYING_SLOT_PSIZ:             return NAK_ATTR_POINT_SIZE;
      case VARYING_SLOT_POS:              return NAK_ATTR_POSITION;
      case VARYING_SLOT_CLIP_DIST0:       return NAK_ATTR_CLIP_CULL_DIST_0;
      case VARYING_SLOT_CLIP_DIST1:       return NAK_ATTR_CLIP_CULL_DIST_4;
      default: unreachable("Invalid varying slot");
      }
   }
}

static uint16_t
nak_sysval_attr_addr(gl_system_value sysval)
{
   switch (sysval) {
   case SYSTEM_VALUE_PRIMITIVE_ID:  return NAK_ATTR_PRIMITIVE_ID;
   case SYSTEM_VALUE_FRAG_COORD:    return NAK_ATTR_POSITION;
   case SYSTEM_VALUE_POINT_COORD:   return NAK_ATTR_POINT_SPRITE;
   case SYSTEM_VALUE_TESS_COORD:    return NAK_ATTR_TESS_COORD;
   case SYSTEM_VALUE_INSTANCE_ID:   return NAK_ATTR_INSTANCE_ID;
   case SYSTEM_VALUE_VERTEX_ID:     return NAK_ATTR_VERTEX_ID;
   case SYSTEM_VALUE_FRONT_FACE:    return NAK_ATTR_FRONT_FACE;
   default: unreachable("Invalid system value");
   }
}

static uint8_t
nak_sysval_sysval_idx(gl_system_value sysval)
{
   switch (sysval) {
   case SYSTEM_VALUE_SUBGROUP_INVOCATION:    return NAK_SV_LANE_ID;
   case SYSTEM_VALUE_VERTICES_IN:            return NAK_SV_VERTEX_COUNT;
   case SYSTEM_VALUE_INVOCATION_ID:          return NAK_SV_INVOCATION_ID;
   case SYSTEM_VALUE_HELPER_INVOCATION:      return NAK_SV_THREAD_KILL;
   case SYSTEM_VALUE_LOCAL_INVOCATION_INDEX: return NAK_SV_COMBINED_TID;
   case SYSTEM_VALUE_LOCAL_INVOCATION_ID:    return NAK_SV_TID;
   case SYSTEM_VALUE_WORKGROUP_ID:           return NAK_SV_CTAID;
   case SYSTEM_VALUE_SUBGROUP_EQ_MASK:       return NAK_SV_LANEMASK_EQ;
   case SYSTEM_VALUE_SUBGROUP_LT_MASK:       return NAK_SV_LANEMASK_LT;
   case SYSTEM_VALUE_SUBGROUP_LE_MASK:       return NAK_SV_LANEMASK_LE;
   case SYSTEM_VALUE_SUBGROUP_GT_MASK:       return NAK_SV_LANEMASK_GT;
   case SYSTEM_VALUE_SUBGROUP_GE_MASK:       return NAK_SV_LANEMASK_GE;
   default: unreachable("Invalid system value");
   }
}

static bool
nak_nir_lower_system_value_intrin(nir_builder *b, nir_intrinsic_instr *intrin,
                                  void *data)
{
   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *val;
   switch (intrin->intrinsic) {
   case nir_intrinsic_load_layer_id: {
      const uint32_t addr = nak_varying_attr_addr(VARYING_SLOT_LAYER);
      val = nir_load_input(b, intrin->def.num_components, 32,
                           nir_imm_int(b, 0), .base = addr,
                           .dest_type = nir_type_int32);
      break;
   }

   case nir_intrinsic_load_primitive_id: {
      assert(b->shader->info.stage == MESA_SHADER_TESS_CTRL ||
             b->shader->info.stage == MESA_SHADER_TESS_EVAL ||
             b->shader->info.stage == MESA_SHADER_GEOMETRY);
      val = nir_load_per_vertex_input(b, 1, 32, nir_imm_int(b, 0),
                                      nir_imm_int(b, 0),
                                      .base = NAK_ATTR_PRIMITIVE_ID,
                                      .dest_type = nir_type_int32);
      break;
   }

   case nir_intrinsic_load_front_face:
   case nir_intrinsic_load_instance_id:
   case nir_intrinsic_load_vertex_id: {
      const gl_system_value sysval =
         nir_system_value_from_intrinsic(intrin->intrinsic);
      const uint32_t addr = nak_sysval_attr_addr(sysval);
      val = nir_load_input(b, intrin->def.num_components, 32,
                           nir_imm_int(b, 0), .base = addr,
                           .dest_type = nir_type_int32);
      break;
   }

   case nir_intrinsic_load_patch_vertices_in: {
      val = nir_load_sysval_nv(b, 32, .base = NAK_SV_VERTEX_COUNT,
                               .access = ACCESS_CAN_REORDER);
      val = nir_extract_u8(b, val, nir_imm_int(b, 1));
      break;
   }

   case nir_intrinsic_load_subgroup_eq_mask:
   case nir_intrinsic_load_subgroup_lt_mask:
   case nir_intrinsic_load_subgroup_le_mask:
   case nir_intrinsic_load_subgroup_gt_mask:
   case nir_intrinsic_load_subgroup_ge_mask: {
      const gl_system_value sysval =
         nir_system_value_from_intrinsic(intrin->intrinsic);
      const uint32_t idx = nak_sysval_sysval_idx(sysval);
      val = nir_load_sysval_nv(b, 32, .base = idx,
                               .access = ACCESS_CAN_REORDER);

      /* Pad with 0 because all invocations above 31 are off */
      if (intrin->def.bit_size == 64) {
         val = nir_u2u32(b, val);
      } else {
         assert(intrin->def.bit_size == 32);
         val = nir_pad_vector_imm_int(b, val, 0, intrin->def.num_components);
      }
      break;
   }

   case nir_intrinsic_load_subgroup_invocation:
   case nir_intrinsic_load_helper_invocation:
   case nir_intrinsic_load_invocation_id:
   case nir_intrinsic_load_local_invocation_index:
   case nir_intrinsic_load_local_invocation_id:
   case nir_intrinsic_load_workgroup_id:
   case nir_intrinsic_load_workgroup_id_zero_base: {
      const gl_system_value sysval =
         intrin->intrinsic == nir_intrinsic_load_workgroup_id_zero_base ?
         SYSTEM_VALUE_WORKGROUP_ID :
         nir_system_value_from_intrinsic(intrin->intrinsic);
      const uint32_t idx = nak_sysval_sysval_idx(sysval);
      nir_def *comps[3];
      assert(intrin->def.num_components <= 3);
      for (unsigned c = 0; c < intrin->def.num_components; c++) {
         comps[c] = nir_load_sysval_nv(b, 32, .base = idx + c,
                                       .access = ACCESS_CAN_REORDER);
      }
      val = nir_vec(b, comps, intrin->def.num_components);
      break;
   }

   case nir_intrinsic_is_helper_invocation: {
      /* Unlike load_helper_invocation, this one isn't re-orderable */
      val = nir_load_sysval_nv(b, 32, .base = NAK_SV_THREAD_KILL);
      break;
   }

   case nir_intrinsic_shader_clock:
      val = nir_load_sysval_nv(b, 64, .base = NAK_SV_CLOCK);
      val = nir_unpack_64_2x32(b, val);
      break;

   default:
      return false;
   }

   if (intrin->def.bit_size == 1)
      val = nir_i2b(b, val);

   nir_def_rewrite_uses(&intrin->def, val);

   return true;
}

static bool
nak_nir_lower_system_values(nir_shader *nir)
{
   return nir_shader_intrinsics_pass(nir, nak_nir_lower_system_value_intrin,
                                     nir_metadata_block_index |
                                     nir_metadata_dominance,
                                     NULL);
}

static bool
nak_nir_lower_varyings(nir_shader *nir, nir_variable_mode modes)
{
   bool progress = false;

   assert(!(modes & ~(nir_var_shader_in | nir_var_shader_out)));

   nir_foreach_variable_with_modes(var, nir, modes)
      var->data.driver_location = nak_varying_attr_addr(var->data.location);

   OPT(nir, nir_lower_io, modes, type_size_vec4_bytes,
       nir_lower_io_lower_64bit_to_32);

   return progress;
}

struct nak_xfb_info
nak_xfb_from_nir(const struct nir_xfb_info *nir_xfb)
{
   if (nir_xfb == NULL)
      return (struct nak_xfb_info) { };

   struct nak_xfb_info nak_xfb = { };

   u_foreach_bit(b, nir_xfb->buffers_written) {
      nak_xfb.stride[b] = nir_xfb->buffers[b].stride;
      nak_xfb.stream[b] = nir_xfb->buffer_to_stream[b];
   }
   memset(nak_xfb.attr_index, 0xff, sizeof(nak_xfb.attr_index)); /* = skip */

   for (unsigned o = 0; o < nir_xfb->output_count; o++) {
      const nir_xfb_output_info *out = &nir_xfb->outputs[o];
      const uint8_t b = out->buffer;
      assert(nir_xfb->buffers_written & BITFIELD_BIT(b));

      const uint16_t attr_addr = nak_varying_attr_addr(out->location);
      assert(attr_addr % 4 == 0);
      const uint16_t attr_idx = attr_addr / 4;

      assert(out->offset % 4 == 0);
      uint8_t out_idx = out->offset / 4;

      u_foreach_bit(c, out->component_mask)
         nak_xfb.attr_index[b][out_idx++] = attr_idx + c;

      nak_xfb.attr_count[b] = MAX2(nak_xfb.attr_count[b], out_idx);
   }

   return nak_xfb;
}

static nir_def *
load_frag_w(nir_builder *b, enum nak_interp_loc interp_loc, nir_def *offset)
{
   if (offset == NULL)
      offset = nir_imm_int(b, 0);

   const uint16_t w_addr =
      nak_sysval_attr_addr(SYSTEM_VALUE_FRAG_COORD) + 12;

   const struct nak_nir_ipa_flags flags = {
      .interp_mode = NAK_INTERP_MODE_SCREEN_LINEAR,
      .interp_freq = NAK_INTERP_FREQ_PASS,
      .interp_loc = interp_loc,
   };
   uint32_t flags_u32;
   memcpy(&flags_u32, &flags, sizeof(flags_u32));

   return nir_ipa_nv(b, nir_imm_float(b, 0), offset,
                     .base = w_addr, .flags = flags_u32);
}

static nir_def *
load_interpolated_input(nir_builder *b, unsigned num_components, uint32_t addr,
                        enum nak_interp_mode interp_mode,
                        enum nak_interp_loc interp_loc,
                        nir_def *inv_w, nir_def *offset,
                        const struct nak_compiler *nak)
{
   if (offset == NULL)
      offset = nir_imm_int(b, 0);

   if (nak->sm >= 70) {
      const struct nak_nir_ipa_flags flags = {
         .interp_mode = interp_mode,
         .interp_freq = NAK_INTERP_FREQ_PASS,
         .interp_loc = interp_loc,
      };
      uint32_t flags_u32;
      memcpy(&flags_u32, &flags, sizeof(flags_u32));

      nir_def *comps[NIR_MAX_VEC_COMPONENTS];
      for (unsigned c = 0; c < num_components; c++) {
         comps[c] = nir_ipa_nv(b, nir_imm_float(b, 0), offset,
                               .base = addr + c * 4,
                               .flags = flags_u32);
         if (interp_mode == NAK_INTERP_MODE_PERSPECTIVE)
            comps[c] = nir_fmul(b, comps[c], inv_w);
      }
      return nir_vec(b, comps, num_components);
   } else if (nak->sm >= 50) {
      struct nak_nir_ipa_flags flags = {
         .interp_mode = interp_mode,
         .interp_freq = NAK_INTERP_FREQ_PASS,
         .interp_loc = interp_loc,
      };

      if (interp_mode == NAK_INTERP_MODE_PERSPECTIVE)
         flags.interp_freq = NAK_INTERP_FREQ_PASS_MUL_W;
      else
         inv_w = nir_imm_float(b, 0);

      uint32_t flags_u32;
      memcpy(&flags_u32, &flags, sizeof(flags_u32));

      nir_def *comps[NIR_MAX_VEC_COMPONENTS];
      for (unsigned c = 0; c < num_components; c++) {
         comps[c] = nir_ipa_nv(b, inv_w, offset,
                               .base = addr + c * 4,
                               .flags = flags_u32);
      }
      return nir_vec(b, comps, num_components);
   } else {
      unreachable("Figure out input interpolation on Kepler");
   }
}

static nir_def *
load_sample_pos_at(nir_builder *b, nir_def *sample_id,
                   const struct nak_fs_key *fs_key)
{
   nir_def *loc = nir_load_ubo(b, 1, 64,
                               nir_imm_int(b, fs_key->sample_locations_cb),
                               nir_imm_int(b, fs_key->sample_locations_offset),
                               .align_mul = 8,
                               .align_offset = 0,
                               .range = fs_key->sample_locations_offset + 8);

   /* Yay little endian */
   loc = nir_ushr(b, loc, nir_imul_imm(b, sample_id, 8));
   nir_def *loc_x_u4 = nir_iand_imm(b, loc, 0xf);
   nir_def *loc_y_u4 = nir_iand_imm(b, nir_ushr_imm(b, loc, 4), 0xf);
   nir_def *loc_u4 = nir_vec2(b, loc_x_u4, loc_y_u4);
   nir_def *result = nir_fmul_imm(b, nir_i2f32(b, loc_u4), 1.0 / 16.0);

   return result;
}

static nir_def *
load_barycentric_offset(nir_builder *b, nir_intrinsic_instr *bary,
                        const struct nak_fs_key *fs_key)
{
   nir_def *offset_f;

   if (bary->intrinsic == nir_intrinsic_load_barycentric_coord_at_sample ||
       bary->intrinsic == nir_intrinsic_load_barycentric_at_sample) {
      nir_def *sample_id = bary->src[0].ssa;
      nir_def *sample_pos = load_sample_pos_at(b, sample_id, fs_key);
      offset_f = nir_fadd_imm(b, sample_pos, -0.5);
   } else {
      offset_f = bary->src[0].ssa;
   }

   offset_f = nir_fclamp(b, offset_f, nir_imm_float(b, -0.5),
                         nir_imm_float(b, 0.437500));
   nir_def *offset_fixed =
      nir_f2i32(b, nir_fmul_imm(b, offset_f, 4096.0));
   nir_def *offset = nir_ior(b, nir_ishl_imm(b, nir_channel(b, offset_fixed, 1), 16),
                             nir_iand_imm(b, nir_channel(b, offset_fixed, 0),
                                          0xffff));

   return offset;
}

struct lower_fs_input_ctx {
   const struct nak_compiler *nak;
   const struct nak_fs_key *fs_key;
};

static bool
lower_fs_input_intrin(nir_builder *b, nir_intrinsic_instr *intrin, void *data)
{
   const struct lower_fs_input_ctx *ctx = data;

   switch (intrin->intrinsic) {
   case nir_intrinsic_load_barycentric_pixel: {
      if (!(ctx->fs_key && ctx->fs_key->force_sample_shading))
         return false;

      intrin->intrinsic = nir_intrinsic_load_barycentric_sample;
      return true;
   }

   case nir_intrinsic_load_frag_coord:
   case nir_intrinsic_load_point_coord: {
      b->cursor = nir_before_instr(&intrin->instr);

      const enum nak_interp_loc interp_loc =
         b->shader->info.fs.uses_sample_shading ? NAK_INTERP_LOC_CENTROID
                                                : NAK_INTERP_LOC_DEFAULT;
      const uint32_t addr =
         intrin->intrinsic == nir_intrinsic_load_point_coord ?
         nak_sysval_attr_addr(SYSTEM_VALUE_POINT_COORD) :
         nak_sysval_attr_addr(SYSTEM_VALUE_FRAG_COORD);

      nir_def *coord = load_interpolated_input(b, intrin->def.num_components,
                                               addr,
                                               NAK_INTERP_MODE_SCREEN_LINEAR,
                                               interp_loc, NULL, NULL,
                                               ctx->nak);

      nir_def_rewrite_uses(&intrin->def, coord);
      nir_instr_remove(&intrin->instr);

      return true;
   }

   case nir_intrinsic_load_input: {
      b->cursor = nir_before_instr(&intrin->instr);

      uint16_t addr = nir_intrinsic_base(intrin) +
                      nir_src_as_uint(intrin->src[0]) +
                      nir_intrinsic_component(intrin) * 4;

      const struct nak_nir_ipa_flags flags = {
         .interp_mode = NAK_INTERP_MODE_CONSTANT,
         .interp_freq = NAK_INTERP_FREQ_CONSTANT,
         .interp_loc = NAK_INTERP_LOC_DEFAULT,
      };
      uint32_t flags_u32;
      memcpy(&flags_u32, &flags, sizeof(flags_u32));

      nir_def *comps[NIR_MAX_VEC_COMPONENTS];
      for (unsigned c = 0; c < intrin->def.num_components; c++) {
         comps[c] = nir_ipa_nv(b, nir_imm_float(b, 0), nir_imm_int(b, 0),
                               .base = addr + c * 4, .flags = flags_u32);
      }
      nir_def *res = nir_vec(b, comps, intrin->def.num_components);

      nir_def_rewrite_uses(&intrin->def, res);
      nir_instr_remove(&intrin->instr);

      return true;
   }

   case nir_intrinsic_load_barycentric_coord_pixel:
   case nir_intrinsic_load_barycentric_coord_centroid:
   case nir_intrinsic_load_barycentric_coord_sample:
   case nir_intrinsic_load_barycentric_coord_at_sample:
   case nir_intrinsic_load_barycentric_coord_at_offset: {
      b->cursor = nir_before_instr(&intrin->instr);

      uint32_t addr;
      enum nak_interp_mode interp_mode;
      if (nir_intrinsic_interp_mode(intrin) == INTERP_MODE_NOPERSPECTIVE) {
         addr = NAK_ATTR_BARY_COORD_NO_PERSP;
         interp_mode = NAK_INTERP_MODE_SCREEN_LINEAR;
      } else {
         addr = NAK_ATTR_BARY_COORD;
         interp_mode = NAK_INTERP_MODE_PERSPECTIVE;
      }

      nir_def *offset = NULL;
      enum nak_interp_loc interp_loc;
      switch (intrin->intrinsic) {
      case nir_intrinsic_load_barycentric_coord_at_sample:
      case nir_intrinsic_load_barycentric_coord_at_offset:
         interp_loc = NAK_INTERP_LOC_OFFSET;
         offset = load_barycentric_offset(b, intrin, ctx->fs_key);
         break;
      case nir_intrinsic_load_barycentric_coord_centroid:
      case nir_intrinsic_load_barycentric_coord_sample:
         interp_loc = NAK_INTERP_LOC_CENTROID;
         break;
      case nir_intrinsic_load_barycentric_coord_pixel:
         interp_loc = NAK_INTERP_LOC_DEFAULT;
         break;
      default:
         unreachable("Unknown intrinsic");
      }

      nir_def *inv_w = NULL;
      if (interp_mode == NAK_INTERP_MODE_PERSPECTIVE)
         inv_w = nir_frcp(b, load_frag_w(b, interp_loc, offset));

      nir_def *res = load_interpolated_input(b, intrin->def.num_components,
                                             addr, interp_mode, interp_loc,
                                             inv_w, offset, ctx->nak);

      nir_def_rewrite_uses(&intrin->def, res);
      nir_instr_remove(&intrin->instr);

      return true;
   }

   case nir_intrinsic_load_interpolated_input: {
      b->cursor = nir_before_instr(&intrin->instr);

      const uint16_t addr = nir_intrinsic_base(intrin) +
                            nir_src_as_uint(intrin->src[1]) +
                            nir_intrinsic_component(intrin) * 4;

      nir_intrinsic_instr *bary = nir_src_as_intrinsic(intrin->src[0]);

      enum nak_interp_mode interp_mode;
      if (nir_intrinsic_interp_mode(bary) == INTERP_MODE_SMOOTH ||
          nir_intrinsic_interp_mode(bary) == INTERP_MODE_NONE)
         interp_mode = NAK_INTERP_MODE_PERSPECTIVE;
      else
         interp_mode = NAK_INTERP_MODE_SCREEN_LINEAR;

      nir_def *offset = NULL;
      enum nak_interp_loc interp_loc;
      switch (bary->intrinsic) {
      case nir_intrinsic_load_barycentric_at_offset:
      case nir_intrinsic_load_barycentric_at_sample: {
         interp_loc = NAK_INTERP_LOC_OFFSET;
         offset = load_barycentric_offset(b, bary, ctx->fs_key);
         break;
      }

      case nir_intrinsic_load_barycentric_centroid:
      case nir_intrinsic_load_barycentric_sample:
         interp_loc = NAK_INTERP_LOC_CENTROID;
         break;

      case nir_intrinsic_load_barycentric_pixel:
         interp_loc = NAK_INTERP_LOC_DEFAULT;
         break;

      default:
         unreachable("Unsupported barycentric");
      }

      nir_def *inv_w = NULL;
      if (interp_mode == NAK_INTERP_MODE_PERSPECTIVE)
         inv_w = nir_frcp(b, load_frag_w(b, interp_loc, offset));

      nir_def *res = load_interpolated_input(b, intrin->def.num_components,
                                             addr, interp_mode, interp_loc,
                                             inv_w, offset, ctx->nak);

      nir_def_rewrite_uses(&intrin->def, res);
      nir_instr_remove(&intrin->instr);

      return true;
   }

   case nir_intrinsic_load_sample_mask_in: {
      if (!b->shader->info.fs.uses_sample_shading &&
          !(ctx->fs_key && ctx->fs_key->force_sample_shading))
         return false;

      b->cursor = nir_after_instr(&intrin->instr);

      /* Mask off just the current sample */
      nir_def *sample = nir_load_sample_id(b);
      nir_def *mask = nir_ishl(b, nir_imm_int(b, 1), sample);
      mask = nir_iand(b, &intrin->def, mask);
      nir_def_rewrite_uses_after(&intrin->def, mask, mask->parent_instr);

      return true;
   }

   case nir_intrinsic_load_sample_pos: {
      b->cursor = nir_before_instr(&intrin->instr);

      nir_def *sample_id = nir_load_sample_id(b);
      nir_def *sample_pos = load_sample_pos_at(b, sample_id, ctx->fs_key);

      nir_def_rewrite_uses(&intrin->def, sample_pos);
      nir_instr_remove(&intrin->instr);

      return true;
   }

   case nir_intrinsic_load_input_vertex: {
      b->cursor = nir_before_instr(&intrin->instr);

      unsigned vertex_id = nir_src_as_uint(intrin->src[0]);
      assert(vertex_id < 3);

      const uint16_t addr = nir_intrinsic_base(intrin) +
                            nir_src_as_uint(intrin->src[1]) +
                            nir_intrinsic_component(intrin) * 4;

      nir_def *comps[NIR_MAX_VEC_COMPONENTS];
      for (unsigned c = 0; c < intrin->def.num_components; c++) {
         nir_def *data = nir_ldtram_nv(b, .base = addr + c * 4,
                                       .flags = vertex_id == 2);
         comps[c] = nir_channel(b, data, vertex_id & 1);
      }
      nir_def *res = nir_vec(b, comps, intrin->num_components);

      nir_def_rewrite_uses(&intrin->def, res);
      nir_instr_remove(&intrin->instr);

      return true;
   }

   default:
      return false;
   }
}

static bool
nak_nir_lower_fs_inputs(nir_shader *nir,
                        const struct nak_compiler *nak,
                        const struct nak_fs_key *fs_key)
{
   NIR_PASS_V(nir, nir_lower_indirect_derefs, nir_var_shader_in, UINT32_MAX);
   NIR_PASS_V(nir, nak_nir_lower_varyings, nir_var_shader_in);
   NIR_PASS_V(nir, nir_opt_constant_folding);

   const struct lower_fs_input_ctx fs_in_ctx = {
      .nak = nak,
      .fs_key = fs_key,
   };
   NIR_PASS_V(nir, nir_shader_intrinsics_pass, lower_fs_input_intrin,
              nir_metadata_block_index | nir_metadata_dominance,
              (void *)&fs_in_ctx);

   return true;
}

static int
fs_out_size(const struct glsl_type *type, bool bindless)
{
   assert(glsl_type_is_vector_or_scalar(type));
   return 16;
}

static bool
nak_nir_lower_fs_outputs(nir_shader *nir)
{
   if (nir->info.outputs_written == 0)
      return false;

   NIR_PASS_V(nir, nir_lower_io_arrays_to_elements_no_indirects, true);

   nir->num_outputs = 0;
   nir_foreach_shader_out_variable(var, nir) {
      assert(nir->info.outputs_written & BITFIELD_BIT(var->data.location));
      switch (var->data.location) {
      case FRAG_RESULT_DEPTH:
         assert(var->data.index == 0);
         assert(var->data.location_frac == 0);
         var->data.driver_location = NAK_FS_OUT_DEPTH;
         break;
      case FRAG_RESULT_STENCIL:
         unreachable("EXT_shader_stencil_export not supported");
         break;
      case FRAG_RESULT_COLOR:
         assert(var->data.index == 0);
         var->data.driver_location = NAK_FS_OUT_COLOR0;
         break;
      case FRAG_RESULT_SAMPLE_MASK:
         assert(var->data.index == 0);
         assert(var->data.location_frac == 0);
         var->data.driver_location = NAK_FS_OUT_SAMPLE_MASK;
         break;
      default: {
         assert(var->data.location >= FRAG_RESULT_DATA0);
         assert(var->data.index < 2);
         const unsigned out =
            (var->data.location - FRAG_RESULT_DATA0) + var->data.index;
         var->data.driver_location = NAK_FS_OUT_COLOR(out);
         break;
      }
      }
   }

   NIR_PASS_V(nir, nir_lower_io, nir_var_shader_out, fs_out_size, 0);

   return true;
}

static bool
nak_mem_vectorize_cb(unsigned align_mul, unsigned align_offset,
                     unsigned bit_size, unsigned num_components,
                     nir_intrinsic_instr *low, nir_intrinsic_instr *high,
                     void *cb_data)
{
   /*
    * Since we legalize these later with nir_lower_mem_access_bit_sizes,
    * we can optimistically combine anything that might be profitable
    */
   assert(util_is_power_of_two_nonzero(align_mul));

   unsigned max_bytes = 128u / 8u;
   if (low->intrinsic == nir_intrinsic_load_ubo)
      max_bytes = 64u / 8u;

   align_mul = MIN2(align_mul, max_bytes);
   align_offset = align_offset % align_mul;
   return align_offset + num_components * (bit_size / 8) <= align_mul;
}

static nir_mem_access_size_align
nak_mem_access_size_align(nir_intrinsic_op intrin,
                          uint8_t bytes, uint8_t bit_size,
                          uint32_t align_mul, uint32_t align_offset,
                          bool offset_is_const, const void *cb_data)
{
   const uint32_t align = nir_combined_align(align_mul, align_offset);
   assert(util_is_power_of_two_nonzero(align));

   unsigned bytes_pow2;
   if (nir_intrinsic_infos[intrin].has_dest) {
      /* Reads can over-fetch a bit if the alignment is okay. */
      bytes_pow2 = util_next_power_of_two(bytes);
   } else {
      bytes_pow2 = 1 << (util_last_bit(bytes) - 1);
   }

   unsigned chunk_bytes = MIN3(bytes_pow2, align, 16);
   assert(util_is_power_of_two_nonzero(chunk_bytes));
   if (intrin == nir_intrinsic_load_ubo)
      chunk_bytes = MIN2(chunk_bytes, 8);

   if (intrin == nir_intrinsic_load_ubo && align < 4) {
      /* CBufs require 4B alignment unless we're doing a ldc.u8 or ldc.i8.
       * In particular, this applies to ldc.u16 which means we either have to
       * fall back to two ldc.u8 or use ldc.u32 and shift stuff around to get
       * the 16bit value out.  Fortunately, nir_lower_mem_access_bit_sizes()
       * can handle over-alignment for reads.
       */
      if (align == 2 || offset_is_const) {
         return (nir_mem_access_size_align) {
            .bit_size = 32,
            .num_components = 1,
            .align = 4,
         };
      } else {
         assert(align == 1);
         return (nir_mem_access_size_align) {
            .bit_size = 8,
            .num_components = 1,
            .align = 1,
         };
      }
   } else if (chunk_bytes < 4) {
      return (nir_mem_access_size_align) {
         .bit_size = chunk_bytes * 8,
         .num_components = 1,
         .align = chunk_bytes,
      };
   } else {
      return (nir_mem_access_size_align) {
         .bit_size = 32,
         .num_components = chunk_bytes / 4,
         .align = chunk_bytes,
      };
   }
}

static bool
nir_shader_has_local_variables(const nir_shader *nir)
{
   nir_foreach_function(func, nir) {
      if (func->impl && !exec_list_is_empty(&func->impl->locals))
         return true;
   }

   return false;
}

void
nak_postprocess_nir(nir_shader *nir,
                    const struct nak_compiler *nak,
                    nir_variable_mode robust2_modes,
                    const struct nak_fs_key *fs_key)
{
   UNUSED bool progress = false;

   nak_optimize_nir(nir, nak);

   const nir_lower_subgroups_options subgroups_options = {
      .subgroup_size = 32,
      .ballot_bit_size = 32,
      .ballot_components = 1,
      .lower_to_scalar = true,
      .lower_vote_eq = true,
      .lower_first_invocation_to_ballot = true,
      .lower_read_first_invocation = true,
      .lower_elect = true,
      .lower_inverse_ballot = true,
   };
   OPT(nir, nir_lower_subgroups, &subgroups_options);
   OPT(nir, nak_nir_lower_scan_reduce);

   if (nir_shader_has_local_variables(nir)) {
      OPT(nir, nir_lower_vars_to_explicit_types, nir_var_function_temp,
          glsl_get_natural_size_align_bytes);
      OPT(nir, nir_lower_explicit_io, nir_var_function_temp,
          nir_address_format_32bit_offset);
      nak_optimize_nir(nir, nak);
   }

   OPT(nir, nir_opt_shrink_vectors);

   nir_load_store_vectorize_options vectorize_opts = {};
   vectorize_opts.modes = nir_var_mem_global |
                          nir_var_mem_ssbo |
                          nir_var_mem_shared |
                          nir_var_shader_temp;
   vectorize_opts.callback = nak_mem_vectorize_cb;
   vectorize_opts.robust_modes = robust2_modes;
   OPT(nir, nir_opt_load_store_vectorize, &vectorize_opts);

   nir_lower_mem_access_bit_sizes_options mem_bit_size_options = {
      .modes = nir_var_mem_constant | nir_var_mem_ubo | nir_var_mem_generic,
      .callback = nak_mem_access_size_align,
   };
   OPT(nir, nir_lower_mem_access_bit_sizes, &mem_bit_size_options);
   OPT(nir, nir_lower_bit_size, lower_bit_size_cb, (void *)nak);

   OPT(nir, nir_opt_combine_barriers, NULL, NULL);

   nak_optimize_nir(nir, nak);

   OPT(nir, nak_nir_lower_tex, nak);
   OPT(nir, nir_lower_idiv, NULL);

   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));

   OPT(nir, nir_lower_indirect_derefs, 0, UINT32_MAX);

   if (nir->info.stage == MESA_SHADER_TESS_EVAL) {
      OPT(nir, nir_lower_tess_coord_z,
          nir->info.tess._primitive_mode == TESS_PRIMITIVE_TRIANGLES);
   }

   OPT(nir, nak_nir_lower_system_values);

   switch (nir->info.stage) {
   case MESA_SHADER_VERTEX:
      OPT(nir, nak_nir_lower_vs_inputs);
      OPT(nir, nak_nir_lower_varyings, nir_var_shader_out);
      OPT(nir, nir_opt_constant_folding);
      OPT(nir, nak_nir_lower_vtg_io, nak);
      break;

   case MESA_SHADER_TESS_CTRL:
   case MESA_SHADER_TESS_EVAL:
      OPT(nir, nak_nir_lower_varyings, nir_var_shader_in | nir_var_shader_out);
      OPT(nir, nir_opt_constant_folding);
      OPT(nir, nak_nir_lower_vtg_io, nak);
      break;

   case MESA_SHADER_FRAGMENT:
      OPT(nir, nak_nir_lower_fs_inputs, nak, fs_key);
      OPT(nir, nak_nir_lower_fs_outputs);
      break;

   case MESA_SHADER_GEOMETRY:
      OPT(nir, nak_nir_lower_varyings, nir_var_shader_in | nir_var_shader_out);
      OPT(nir, nir_opt_constant_folding);
      OPT(nir, nak_nir_lower_vtg_io, nak);
      OPT(nir, nak_nir_lower_gs_intrinsics);
      break;

   case MESA_SHADER_COMPUTE:
   case MESA_SHADER_KERNEL:
      break;

   default:
      unreachable("Unsupported shader stage");
   }

   OPT(nir, nir_lower_doubles, NULL, nak->nir_options.lower_doubles_options);
   OPT(nir, nir_lower_int64);

   nak_optimize_nir(nir, nak);

   do {
      progress = false;
      OPT(nir, nir_opt_algebraic_late);
      OPT(nir, nak_nir_lower_algebraic_late, nak);

      /* If we're lowering fp64 sat but not min/max, the sat lowering may have
       * been undone by nir_opt_algebraic.  Lower sat again just to be sure.
       */
      if ((nak->nir_options.lower_doubles_options & nir_lower_dsat) &&
          !(nak->nir_options.lower_doubles_options & nir_lower_dminmax))
         OPT(nir, nir_lower_doubles, NULL, nir_lower_dsat);

      if (progress) {
         OPT(nir, nir_opt_constant_folding);
         OPT(nir, nir_copy_prop);
         OPT(nir, nir_opt_dce);
         OPT(nir, nir_opt_cse);
      }
   } while (progress);

   nir_divergence_analysis(nir);

   OPT(nir, nak_nir_add_barriers, nak);

   /* Re-index blocks and compact SSA defs because we'll use them to index
    * arrays
    */
   nir_foreach_function(func, nir) {
      if (func->impl) {
         nir_index_blocks(func->impl);
         nir_index_ssa_defs(func->impl);
      }
   }

   if (nak_should_print_nir())
      nir_print_shader(nir, stderr);
}

static bool
scalar_is_imm_int(nir_scalar x, unsigned bits)
{
   if (!nir_scalar_is_const(x))
      return false;

   int64_t imm = nir_scalar_as_int(x);
   return u_intN_min(bits) <= imm && imm <= u_intN_max(bits);
}

struct nak_io_addr_offset
nak_get_io_addr_offset(nir_def *addr, uint8_t imm_bits)
{
   nir_scalar addr_s = {
      .def = addr,
      .comp = 0,
   };
   if (scalar_is_imm_int(addr_s, imm_bits)) {
      /* Base is a dumb name for this.  It should be offset */
      return (struct nak_io_addr_offset) {
         .offset = nir_scalar_as_int(addr_s),
      };
   }

   addr_s = nir_scalar_chase_movs(addr_s);
   if (!nir_scalar_is_alu(addr_s) ||
       nir_scalar_alu_op(addr_s) != nir_op_iadd) {
      return (struct nak_io_addr_offset) {
         .base = addr_s,
      };
   }

   for (unsigned i = 0; i < 2; i++) {
      nir_scalar off_s = nir_scalar_chase_alu_src(addr_s, i);
      off_s = nir_scalar_chase_movs(off_s);
      if (scalar_is_imm_int(off_s, imm_bits)) {
         return (struct nak_io_addr_offset) {
            .base = nir_scalar_chase_alu_src(addr_s, 1 - i),
            .offset = nir_scalar_as_int(off_s),
         };
      }
   }

   return (struct nak_io_addr_offset) {
      .base = addr_s,
   };
}
