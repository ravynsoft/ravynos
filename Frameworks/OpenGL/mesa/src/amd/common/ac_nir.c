/*
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_nir.h"
#include "sid.h"
#include "nir_builder.h"
#include "nir_xfb_info.h"

/* Load argument with index start from arg plus relative_index. */
nir_def *
ac_nir_load_arg_at_offset(nir_builder *b, const struct ac_shader_args *ac_args,
                          struct ac_arg arg, unsigned relative_index)
{
   unsigned arg_index = arg.arg_index + relative_index;
   unsigned num_components = ac_args->args[arg_index].size;

   if (ac_args->args[arg_index].file == AC_ARG_SGPR)
      return nir_load_scalar_arg_amd(b, num_components, .base = arg_index);
   else
      return nir_load_vector_arg_amd(b, num_components, .base = arg_index);
}

void
ac_nir_store_arg(nir_builder *b, const struct ac_shader_args *ac_args, struct ac_arg arg,
                 nir_def *val)
{
   assert(nir_cursor_current_block(b->cursor)->cf_node.parent->type == nir_cf_node_function);

   if (ac_args->args[arg.arg_index].file == AC_ARG_SGPR)
      nir_store_scalar_arg_amd(b, val, .base = arg.arg_index);
   else
      nir_store_vector_arg_amd(b, val, .base = arg.arg_index);
}

nir_def *
ac_nir_unpack_arg(nir_builder *b, const struct ac_shader_args *ac_args, struct ac_arg arg,
                  unsigned rshift, unsigned bitwidth)
{
   nir_def *value = ac_nir_load_arg(b, ac_args, arg);
   if (rshift == 0 && bitwidth == 32)
      return value;
   else if (rshift == 0)
      return nir_iand_imm(b, value, BITFIELD_MASK(bitwidth));
   else if ((32 - rshift) <= bitwidth)
      return nir_ushr_imm(b, value, rshift);
   else
      return nir_ubfe_imm(b, value, rshift, bitwidth);
}

static bool
is_sin_cos(const nir_instr *instr, UNUSED const void *_)
{
   return instr->type == nir_instr_type_alu && (nir_instr_as_alu(instr)->op == nir_op_fsin ||
                                                nir_instr_as_alu(instr)->op == nir_op_fcos);
}

static nir_def *
lower_sin_cos(struct nir_builder *b, nir_instr *instr, UNUSED void *_)
{
   nir_alu_instr *sincos = nir_instr_as_alu(instr);
   nir_def *src = nir_fmul_imm(b, nir_ssa_for_alu_src(b, sincos, 0), 0.15915493667125702);
   return sincos->op == nir_op_fsin ? nir_fsin_amd(b, src) : nir_fcos_amd(b, src);
}

bool
ac_nir_lower_sin_cos(nir_shader *shader)
{
   return nir_shader_lower_instructions(shader, is_sin_cos, lower_sin_cos, NULL);
}

typedef struct {
   const struct ac_shader_args *const args;
   const enum amd_gfx_level gfx_level;
   const enum ac_hw_stage hw_stage;
} lower_intrinsics_to_args_state;

static bool
lower_intrinsic_to_arg(nir_builder *b, nir_instr *instr, void *state)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   lower_intrinsics_to_args_state *s = (lower_intrinsics_to_args_state *)state;
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   nir_def *replacement = NULL;
   b->cursor = nir_after_instr(&intrin->instr);

   switch (intrin->intrinsic) {
   case nir_intrinsic_load_subgroup_id: {
      if (s->hw_stage == AC_HW_COMPUTE_SHADER) {
         assert(s->args->tg_size.used);

         if (s->gfx_level >= GFX10_3) {
            replacement = ac_nir_unpack_arg(b, s->args, s->args->tg_size, 20, 5);
         } else {
            /* GFX6-10 don't actually support a wave id, but we can
             * use the ordered id because ORDERED_APPEND_* is set to
             * zero in the compute dispatch initiatior.
             */
            replacement = ac_nir_unpack_arg(b, s->args, s->args->tg_size, 6, 6);
         }
      } else if (s->hw_stage == AC_HW_HULL_SHADER && s->gfx_level >= GFX11) {
         assert(s->args->tcs_wave_id.used);
         replacement = ac_nir_unpack_arg(b, s->args, s->args->tcs_wave_id, 0, 3);
      } else if (s->hw_stage == AC_HW_LEGACY_GEOMETRY_SHADER ||
                 s->hw_stage == AC_HW_NEXT_GEN_GEOMETRY_SHADER) {
         assert(s->args->merged_wave_info.used);
         replacement = ac_nir_unpack_arg(b, s->args, s->args->merged_wave_info, 24, 4);
      } else {
         replacement = nir_imm_int(b, 0);
      }

      break;
   }
   case nir_intrinsic_load_num_subgroups: {
      if (s->hw_stage == AC_HW_COMPUTE_SHADER) {
         assert(s->args->tg_size.used);
         replacement = ac_nir_unpack_arg(b, s->args, s->args->tg_size, 0, 6);
      } else if (s->hw_stage == AC_HW_LEGACY_GEOMETRY_SHADER ||
                 s->hw_stage == AC_HW_NEXT_GEN_GEOMETRY_SHADER) {
         assert(s->args->merged_wave_info.used);
         replacement = ac_nir_unpack_arg(b, s->args, s->args->merged_wave_info, 28, 4);
      } else {
         replacement = nir_imm_int(b, 1);
      }

      break;
   }
   case nir_intrinsic_load_workgroup_id:
      if (b->shader->info.stage == MESA_SHADER_MESH) {
         /* This lowering is only valid with fast_launch = 2, otherwise we assume that
          * lower_workgroup_id_to_index removed any uses of the workgroup id by this point.
          */
         assert(s->gfx_level >= GFX11);
         nir_def *xy = ac_nir_load_arg(b, s->args, s->args->tess_offchip_offset);
         nir_def *z = ac_nir_load_arg(b, s->args, s->args->gs_attr_offset);
         replacement = nir_vec3(b, nir_extract_u16(b, xy, nir_imm_int(b, 0)),
                                nir_extract_u16(b, xy, nir_imm_int(b, 1)),
                                nir_extract_u16(b, z, nir_imm_int(b, 1)));
      } else {
         return false;
      }

      break;
   default:
      return false;
   }

   assert(replacement);
   nir_def_rewrite_uses(&intrin->def, replacement);
   nir_instr_remove(&intrin->instr);
   return true;
}

bool
ac_nir_lower_intrinsics_to_args(nir_shader *shader, const enum amd_gfx_level gfx_level,
                                const enum ac_hw_stage hw_stage,
                                const struct ac_shader_args *ac_args)
{
   lower_intrinsics_to_args_state state = {
      .gfx_level = gfx_level,
      .hw_stage = hw_stage,
      .args = ac_args,
   };

   return nir_shader_instructions_pass(shader, lower_intrinsic_to_arg,
                                       nir_metadata_block_index | nir_metadata_dominance, &state);
}

void
ac_nir_store_var_components(nir_builder *b, nir_variable *var, nir_def *value,
                            unsigned component, unsigned writemask)
{
   /* component store */
   if (value->num_components != 4) {
      nir_def *undef = nir_undef(b, 1, value->bit_size);

      /* add undef component before and after value to form a vec4 */
      nir_def *comp[4];
      for (int i = 0; i < 4; i++) {
         comp[i] = (i >= component && i < component + value->num_components) ?
            nir_channel(b, value, i - component) : undef;
      }

      value = nir_vec(b, comp, 4);
      writemask <<= component;
   } else {
      /* if num_component==4, there should be no component offset */
      assert(component == 0);
   }

   nir_store_var(b, var, value, writemask);
}

static nir_intrinsic_instr *
export(nir_builder *b, nir_def *val, nir_def *row, unsigned base, unsigned flags,
       unsigned write_mask)
{
   if (row) {
      return nir_export_row_amd(b, val, row, .base = base, .flags = flags,
                                .write_mask = write_mask);
   } else {
      return nir_export_amd(b, val, .base = base, .flags = flags,
                            .write_mask = write_mask);
   }
}

void
ac_nir_export_primitive(nir_builder *b, nir_def *prim, nir_def *row)
{
   unsigned write_mask = BITFIELD_MASK(prim->num_components);

   export(b, nir_pad_vec4(b, prim), row, V_008DFC_SQ_EXP_PRIM, AC_EXP_FLAG_DONE,
          write_mask);
}

static nir_def *
get_export_output(nir_builder *b, nir_def **output)
{
   nir_def *vec[4];
   for (int i = 0; i < 4; i++) {
      if (output[i])
         vec[i] = nir_u2uN(b, output[i], 32);
      else
         vec[i] = nir_undef(b, 1, 32);
   }

   return nir_vec(b, vec, 4);
}

static nir_def *
get_pos0_output(nir_builder *b, nir_def **output)
{
   /* Some applications don't write position but expect (0, 0, 0, 1)
    * so use that value instead of undef when it isn't written.
    */

   nir_def *vec[4];

   for (int i = 0; i < 4; i++) {
      if (output[i])
         vec[i] = nir_u2u32(b, output[i]);
     else
         vec[i] = nir_imm_float(b, i == 3 ? 1.0 : 0.0);
   }

   return nir_vec(b, vec, 4);
}

void
ac_nir_export_position(nir_builder *b,
                       enum amd_gfx_level gfx_level,
                       uint32_t clip_cull_mask,
                       bool no_param_export,
                       bool force_vrs,
                       bool done,
                       uint64_t outputs_written,
                       nir_def *(*outputs)[4],
                       nir_def *row)
{
   nir_intrinsic_instr *exp[4];
   unsigned exp_num = 0;
   unsigned exp_pos_offset = 0;

   if (outputs_written & VARYING_BIT_POS) {
      /* GFX10 (Navi1x) skip POS0 exports if EXEC=0 and DONE=0, causing a hang.
      * Setting valid_mask=1 prevents it and has no other effect.
      */
      const unsigned pos_flags = gfx_level == GFX10 ? AC_EXP_FLAG_VALID_MASK : 0;
      nir_def *pos = get_pos0_output(b, outputs[VARYING_SLOT_POS]);

      exp[exp_num] = export(b, pos, row, V_008DFC_SQ_EXP_POS + exp_num, pos_flags, 0xf);
      exp_num++;
   } else {
      exp_pos_offset++;
   }

   uint64_t mask =
      VARYING_BIT_PSIZ |
      VARYING_BIT_EDGE |
      VARYING_BIT_LAYER |
      VARYING_BIT_VIEWPORT |
      VARYING_BIT_PRIMITIVE_SHADING_RATE;

   /* clear output mask if no one written */
   if (!outputs[VARYING_SLOT_PSIZ][0])
      outputs_written &= ~VARYING_BIT_PSIZ;
   if (!outputs[VARYING_SLOT_EDGE][0])
      outputs_written &= ~VARYING_BIT_EDGE;
   if (!outputs[VARYING_SLOT_PRIMITIVE_SHADING_RATE][0])
      outputs_written &= ~VARYING_BIT_PRIMITIVE_SHADING_RATE;
   if (!outputs[VARYING_SLOT_LAYER][0])
      outputs_written &= ~VARYING_BIT_LAYER;
   if (!outputs[VARYING_SLOT_VIEWPORT][0])
      outputs_written &= ~VARYING_BIT_VIEWPORT;

   if ((outputs_written & mask) || force_vrs) {
      nir_def *zero = nir_imm_float(b, 0);
      nir_def *vec[4] = { zero, zero, zero, zero };
      unsigned write_mask = 0;

      if (outputs_written & VARYING_BIT_PSIZ) {
         vec[0] = outputs[VARYING_SLOT_PSIZ][0];
         write_mask |= BITFIELD_BIT(0);
      }

      if (outputs_written & VARYING_BIT_EDGE) {
         vec[1] = nir_umin(b, outputs[VARYING_SLOT_EDGE][0], nir_imm_int(b, 1));
         write_mask |= BITFIELD_BIT(1);
      }

      nir_def *rates = NULL;
      if (outputs_written & VARYING_BIT_PRIMITIVE_SHADING_RATE) {
         rates = outputs[VARYING_SLOT_PRIMITIVE_SHADING_RATE][0];
      } else if (force_vrs) {
         /* If Pos.W != 1 (typical for non-GUI elements), use coarse shading. */
         nir_def *pos_w = outputs[VARYING_SLOT_POS][3];
         pos_w = pos_w ? nir_u2u32(b, pos_w) : nir_imm_float(b, 1.0);
         nir_def *cond = nir_fneu_imm(b, pos_w, 1);
         rates = nir_bcsel(b, cond, nir_load_force_vrs_rates_amd(b), nir_imm_int(b, 0));
      }

      if (rates) {
         vec[1] = nir_ior(b, vec[1], rates);
         write_mask |= BITFIELD_BIT(1);
      }

      if (outputs_written & VARYING_BIT_LAYER) {
         vec[2] = outputs[VARYING_SLOT_LAYER][0];
         write_mask |= BITFIELD_BIT(2);
      }

      if (outputs_written & VARYING_BIT_VIEWPORT) {
         if (gfx_level >= GFX9) {
            /* GFX9 has the layer in [10:0] and the viewport index in [19:16]. */
            nir_def *v = nir_ishl_imm(b, outputs[VARYING_SLOT_VIEWPORT][0], 16);
            vec[2] = nir_ior(b, vec[2], v);
            write_mask |= BITFIELD_BIT(2);
         } else {
            vec[3] = outputs[VARYING_SLOT_VIEWPORT][0];
            write_mask |= BITFIELD_BIT(3);
         }
      }

      exp[exp_num] = export(b, nir_vec(b, vec, 4), row,
                            V_008DFC_SQ_EXP_POS + exp_num + exp_pos_offset,
                            0, write_mask);
      exp_num++;
   }

   for (int i = 0; i < 2; i++) {
      if ((outputs_written & (VARYING_BIT_CLIP_DIST0 << i)) &&
          (clip_cull_mask & BITFIELD_RANGE(i * 4, 4))) {
         exp[exp_num] = export(
            b, get_export_output(b, outputs[VARYING_SLOT_CLIP_DIST0 + i]), row,
            V_008DFC_SQ_EXP_POS + exp_num + exp_pos_offset, 0,
            (clip_cull_mask >> (i * 4)) & 0xf);
         exp_num++;
      }
   }

   if (outputs_written & VARYING_BIT_CLIP_VERTEX) {
      nir_def *vtx = get_export_output(b, outputs[VARYING_SLOT_CLIP_VERTEX]);

      /* Clip distance for clip vertex to each user clip plane. */
      nir_def *clip_dist[8] = {0};
      u_foreach_bit (i, clip_cull_mask) {
         nir_def *ucp = nir_load_user_clip_plane(b, .ucp_id = i);
         clip_dist[i] = nir_fdot4(b, vtx, ucp);
      }

      for (int i = 0; i < 2; i++) {
         if (clip_cull_mask & BITFIELD_RANGE(i * 4, 4)) {
            exp[exp_num] = export(
               b, get_export_output(b, clip_dist + i * 4), row,
               V_008DFC_SQ_EXP_POS + exp_num + exp_pos_offset, 0,
               (clip_cull_mask >> (i * 4)) & 0xf);
            exp_num++;
         }
      }
   }

   if (!exp_num)
      return;

   nir_intrinsic_instr *final_exp = exp[exp_num - 1];

   if (done) {
      /* Specify that this is the last export */
      const unsigned final_exp_flags = nir_intrinsic_flags(final_exp);
      nir_intrinsic_set_flags(final_exp, final_exp_flags | AC_EXP_FLAG_DONE);
   }

   /* If a shader has no param exports, rasterization can start before
    * the shader finishes and thus memory stores might not finish before
    * the pixel shader starts.
    */
   if (gfx_level >= GFX10 && no_param_export && b->shader->info.writes_memory) {
      nir_cursor cursor = b->cursor;
      b->cursor = nir_before_instr(&final_exp->instr);
      nir_scoped_memory_barrier(b, SCOPE_DEVICE, NIR_MEMORY_RELEASE,
                                nir_var_mem_ssbo | nir_var_mem_global | nir_var_image);
      b->cursor = cursor;
   }
}

void
ac_nir_export_parameters(nir_builder *b,
                         const uint8_t *param_offsets,
                         uint64_t outputs_written,
                         uint16_t outputs_written_16bit,
                         nir_def *(*outputs)[4],
                         nir_def *(*outputs_16bit_lo)[4],
                         nir_def *(*outputs_16bit_hi)[4])
{
   uint32_t exported_params = 0;

   u_foreach_bit64 (slot, outputs_written) {
      unsigned offset = param_offsets[slot];
      if (offset > AC_EXP_PARAM_OFFSET_31)
         continue;

      uint32_t write_mask = 0;
      for (int i = 0; i < 4; i++) {
         if (outputs[slot][i])
            write_mask |= BITFIELD_BIT(i);
      }

      /* no one set this output slot, we can skip the param export */
      if (!write_mask)
         continue;

      /* Since param_offsets[] can map multiple varying slots to the same
       * param export index (that's radeonsi-specific behavior), we need to
       * do this so as not to emit duplicated exports.
       */
      if (exported_params & BITFIELD_BIT(offset))
         continue;

      nir_export_amd(
         b, get_export_output(b, outputs[slot]),
         .base = V_008DFC_SQ_EXP_PARAM + offset,
         .write_mask = write_mask);
      exported_params |= BITFIELD_BIT(offset);
   }

   u_foreach_bit (slot, outputs_written_16bit) {
      unsigned offset = param_offsets[VARYING_SLOT_VAR0_16BIT + slot];
      if (offset > AC_EXP_PARAM_OFFSET_31)
         continue;

      uint32_t write_mask = 0;
      for (int i = 0; i < 4; i++) {
         if (outputs_16bit_lo[slot][i] || outputs_16bit_hi[slot][i])
            write_mask |= BITFIELD_BIT(i);
      }

      /* no one set this output slot, we can skip the param export */
      if (!write_mask)
         continue;

      /* Since param_offsets[] can map multiple varying slots to the same
       * param export index (that's radeonsi-specific behavior), we need to
       * do this so as not to emit duplicated exports.
       */
      if (exported_params & BITFIELD_BIT(offset))
         continue;

      nir_def *vec[4];
      nir_def *undef = nir_undef(b, 1, 16);
      for (int i = 0; i < 4; i++) {
         nir_def *lo = outputs_16bit_lo[slot][i] ? outputs_16bit_lo[slot][i] : undef;
         nir_def *hi = outputs_16bit_hi[slot][i] ? outputs_16bit_hi[slot][i] : undef;
         vec[i] = nir_pack_32_2x16_split(b, lo, hi);
      }

      nir_export_amd(
         b, nir_vec(b, vec, 4),
         .base = V_008DFC_SQ_EXP_PARAM + offset,
         .write_mask = write_mask);
      exported_params |= BITFIELD_BIT(offset);
   }
}

/**
 * This function takes an I/O intrinsic like load/store_input,
 * and emits a sequence that calculates the full offset of that instruction,
 * including a stride to the base and component offsets.
 */
nir_def *
ac_nir_calc_io_offset(nir_builder *b,
                      nir_intrinsic_instr *intrin,
                      nir_def *base_stride,
                      unsigned component_stride,
                      ac_nir_map_io_driver_location map_io)
{
   unsigned base = nir_intrinsic_base(intrin);
   unsigned semantic = nir_intrinsic_io_semantics(intrin).location;
   unsigned mapped_driver_location = map_io ? map_io(semantic) : base;

   /* base is the driver_location, which is in slots (1 slot = 4x4 bytes) */
   nir_def *base_op = nir_imul_imm(b, base_stride, mapped_driver_location);

   /* offset should be interpreted in relation to the base,
    * so the instruction effectively reads/writes another input/output
    * when it has an offset
    */
   nir_def *offset_op = nir_imul(b, base_stride,
                                 nir_get_io_offset_src(intrin)->ssa);

   /* component is in bytes */
   unsigned const_op = nir_intrinsic_component(intrin) * component_stride;

   return nir_iadd_imm_nuw(b, nir_iadd_nuw(b, base_op, offset_op), const_op);
}

bool
ac_nir_lower_indirect_derefs(nir_shader *shader,
                             enum amd_gfx_level gfx_level)
{
   bool progress = false;

   /* Lower large variables to scratch first so that we won't bloat the
    * shader by generating large if ladders for them. We later lower
    * scratch to alloca's, assuming LLVM won't generate VGPR indexing.
    */
   NIR_PASS(progress, shader, nir_lower_vars_to_scratch, nir_var_function_temp, 256,
            glsl_get_natural_size_align_bytes);

   /* LLVM doesn't support VGPR indexing on GFX9. */
   bool llvm_has_working_vgpr_indexing = gfx_level != GFX9;

   /* TODO: Indirect indexing of GS inputs is unimplemented.
    *
    * TCS and TES load inputs directly from LDS or offchip memory, so
    * indirect indexing is trivial.
    */
   nir_variable_mode indirect_mask = 0;
   if (shader->info.stage == MESA_SHADER_GEOMETRY ||
       (shader->info.stage != MESA_SHADER_TESS_CTRL && shader->info.stage != MESA_SHADER_TESS_EVAL &&
        !llvm_has_working_vgpr_indexing)) {
      indirect_mask |= nir_var_shader_in;
   }
   if (!llvm_has_working_vgpr_indexing && shader->info.stage != MESA_SHADER_TESS_CTRL)
      indirect_mask |= nir_var_shader_out;

   /* TODO: We shouldn't need to do this, however LLVM isn't currently
    * smart enough to handle indirects without causing excess spilling
    * causing the gpu to hang.
    *
    * See the following thread for more details of the problem:
    * https://lists.freedesktop.org/archives/mesa-dev/2017-July/162106.html
    */
   indirect_mask |= nir_var_function_temp;

   NIR_PASS(progress, shader, nir_lower_indirect_derefs, indirect_mask, UINT32_MAX);
   return progress;
}

struct shader_outputs {
   nir_def *data[VARYING_SLOT_MAX][4];
   nir_def *data_16bit_lo[16][4];
   nir_def *data_16bit_hi[16][4];

   nir_alu_type (*type_16bit_lo)[4];
   nir_alu_type (*type_16bit_hi)[4];
};

static nir_def **
get_output_and_type(struct shader_outputs *outputs, unsigned slot, bool high_16bits,
                    nir_alu_type **types)
{
   nir_def **data;
   nir_alu_type *type;

   /* Only VARYING_SLOT_VARn_16BIT slots need output type to convert 16bit output
    * to 32bit. Vulkan is not allowed to streamout output less than 32bit.
    */
   if (slot < VARYING_SLOT_VAR0_16BIT) {
      data = outputs->data[slot];
      type = NULL;
   } else {
      unsigned index = slot - VARYING_SLOT_VAR0_16BIT;

      if (high_16bits) {
         data = outputs->data_16bit_hi[index];
         type = outputs->type_16bit_hi[index];
      } else {
         data = outputs->data_16bit_lo[index];
         type = outputs->type_16bit_lo[index];
      }
   }

   *types = type;
   return data;
}

static void
emit_streamout(nir_builder *b, unsigned stream, nir_xfb_info *info,
               struct shader_outputs *outputs)
{
   nir_def *so_vtx_count = nir_ubfe_imm(b, nir_load_streamout_config_amd(b), 16, 7);
   nir_def *tid = nir_load_subgroup_invocation(b);

   nir_push_if(b, nir_ilt(b, tid, so_vtx_count));
   nir_def *so_write_index = nir_load_streamout_write_index_amd(b);

   nir_def *so_buffers[NIR_MAX_XFB_BUFFERS];
   nir_def *so_write_offset[NIR_MAX_XFB_BUFFERS];
   u_foreach_bit(i, info->buffers_written) {
      so_buffers[i] = nir_load_streamout_buffer_amd(b, i);

      unsigned stride = info->buffers[i].stride;
      nir_def *offset = nir_load_streamout_offset_amd(b, i);
      offset = nir_iadd(b, nir_imul_imm(b, nir_iadd(b, so_write_index, tid), stride),
                        nir_imul_imm(b, offset, 4));
      so_write_offset[i] = offset;
   }

   nir_def *undef = nir_undef(b, 1, 32);
   for (unsigned i = 0; i < info->output_count; i++) {
      const nir_xfb_output_info *output = info->outputs + i;
      if (stream != info->buffer_to_stream[output->buffer])
         continue;

      nir_alu_type *output_type;
      nir_def **output_data =
         get_output_and_type(outputs, output->location, output->high_16bits, &output_type);

      nir_def *vec[4] = {undef, undef, undef, undef};
      uint8_t mask = 0;
      u_foreach_bit(j, output->component_mask) {
         nir_def *data = output_data[j];

         if (data) {
            if (data->bit_size < 32) {
               /* we need output type to convert non-32bit output to 32bit */
               assert(output_type);

               nir_alu_type base_type = nir_alu_type_get_base_type(output_type[j]);
               data = nir_convert_to_bit_size(b, data, base_type, 32);
            }

            unsigned comp = j - output->component_offset;
            vec[comp] = data;
            mask |= 1 << comp;
         }
      }

      if (!mask)
         continue;

      unsigned buffer = output->buffer;
      nir_def *data = nir_vec(b, vec, util_last_bit(mask));
      nir_def *zero = nir_imm_int(b, 0);
      nir_store_buffer_amd(b, data, so_buffers[buffer], so_write_offset[buffer], zero, zero,
                           .base = output->offset, .write_mask = mask,
                           .access = ACCESS_COHERENT | ACCESS_NON_TEMPORAL);
   }

   nir_pop_if(b, NULL);
}

nir_shader *
ac_nir_create_gs_copy_shader(const nir_shader *gs_nir,
                             enum amd_gfx_level gfx_level,
                             uint32_t clip_cull_mask,
                             const uint8_t *param_offsets,
                             bool has_param_exports,
                             bool disable_streamout,
                             bool kill_pointsize,
                             bool kill_layer,
                             bool force_vrs,
                             ac_nir_gs_output_info *output_info)
{
   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_VERTEX, gs_nir->options, "gs_copy");

   nir_foreach_shader_out_variable(var, gs_nir)
      nir_shader_add_variable(b.shader, nir_variable_clone(var, b.shader));

   b.shader->info.outputs_written = gs_nir->info.outputs_written;
   b.shader->info.outputs_written_16bit = gs_nir->info.outputs_written_16bit;

   nir_def *gsvs_ring = nir_load_ring_gsvs_amd(&b);

   nir_xfb_info *info = gs_nir->xfb_info;
   nir_def *stream_id = NULL;
   if (!disable_streamout && info)
      stream_id = nir_ubfe_imm(&b, nir_load_streamout_config_amd(&b), 24, 2);

   nir_def *vtx_offset = nir_imul_imm(&b, nir_load_vertex_id_zero_base(&b), 4);
   nir_def *zero = nir_imm_zero(&b, 1, 32);

   for (unsigned stream = 0; stream < 4; stream++) {
      if (stream > 0 && (!stream_id || !(info->streams_written & BITFIELD_BIT(stream))))
         continue;

      if (stream_id)
         nir_push_if(&b, nir_ieq_imm(&b, stream_id, stream));

      uint32_t offset = 0;
      struct shader_outputs outputs = {
         .type_16bit_lo = output_info->types_16bit_lo,
         .type_16bit_hi = output_info->types_16bit_hi,
      };

      u_foreach_bit64 (i, gs_nir->info.outputs_written) {
         u_foreach_bit (j, output_info->usage_mask[i]) {
            if (((output_info->streams[i] >> (j * 2)) & 0x3) != stream)
               continue;

            outputs.data[i][j] =
               nir_load_buffer_amd(&b, 1, 32, gsvs_ring, vtx_offset, zero, zero,
                                   .base = offset,
                                   .access = ACCESS_COHERENT | ACCESS_NON_TEMPORAL);

            /* clamp legacy color output */
            if (i == VARYING_SLOT_COL0 || i == VARYING_SLOT_COL1 ||
                i == VARYING_SLOT_BFC0 || i == VARYING_SLOT_BFC1) {
               nir_def *color = outputs.data[i][j];
               nir_def *clamp = nir_load_clamp_vertex_color_amd(&b);
               outputs.data[i][j] = nir_bcsel(&b, clamp, nir_fsat(&b, color), color);
            }

            offset += gs_nir->info.gs.vertices_out * 16 * 4;
         }
      }

      u_foreach_bit (i, gs_nir->info.outputs_written_16bit) {
         for (unsigned j = 0; j < 4; j++) {
            bool has_lo_16bit = (output_info->usage_mask_16bit_lo[i] & (1 << j)) &&
               ((output_info->streams_16bit_lo[i] >> (j * 2)) & 0x3) == stream;
            bool has_hi_16bit = (output_info->usage_mask_16bit_hi[i] & (1 << j)) &&
               ((output_info->streams_16bit_hi[i] >> (j * 2)) & 0x3) == stream;
            if (!has_lo_16bit && !has_hi_16bit)
               continue;

            nir_def *data =
               nir_load_buffer_amd(&b, 1, 32, gsvs_ring, vtx_offset, zero, zero,
                                   .base = offset,
                                   .access = ACCESS_COHERENT | ACCESS_NON_TEMPORAL);

            if (has_lo_16bit)
               outputs.data_16bit_lo[i][j] = nir_unpack_32_2x16_split_x(&b, data);

            if (has_hi_16bit)
               outputs.data_16bit_hi[i][j] = nir_unpack_32_2x16_split_y(&b, data);

            offset += gs_nir->info.gs.vertices_out * 16 * 4;
         }
      }

      if (stream_id)
         emit_streamout(&b, stream, info, &outputs);

      if (stream == 0) {
         uint64_t export_outputs = b.shader->info.outputs_written | VARYING_BIT_POS;
         if (kill_pointsize)
            export_outputs &= ~VARYING_BIT_PSIZ;
         if (kill_layer)
            export_outputs &= ~VARYING_BIT_LAYER;

         ac_nir_export_position(&b, gfx_level, clip_cull_mask, !has_param_exports,
                                force_vrs, true, export_outputs, outputs.data, NULL);

         if (has_param_exports) {
            ac_nir_export_parameters(&b, param_offsets,
                                     b.shader->info.outputs_written,
                                     b.shader->info.outputs_written_16bit,
                                     outputs.data,
                                     outputs.data_16bit_lo,
                                     outputs.data_16bit_hi);
         }
      }

      if (stream_id)
         nir_push_else(&b, NULL);
   }

   b.shader->info.clip_distance_array_size = gs_nir->info.clip_distance_array_size;
   b.shader->info.cull_distance_array_size = gs_nir->info.cull_distance_array_size;

   return b.shader;
}

static void
gather_outputs(nir_builder *b, nir_function_impl *impl, struct shader_outputs *outputs)
{
   /* Assume:
    * - the shader used nir_lower_io_to_temporaries
    * - 64-bit outputs are lowered
    * - no indirect indexing is present
    */
   nir_foreach_block (block, impl) {
      nir_foreach_instr_safe (instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic != nir_intrinsic_store_output)
            continue;

         assert(nir_src_is_const(intrin->src[1]) && !nir_src_as_uint(intrin->src[1]));

         nir_alu_type type = nir_intrinsic_src_type(intrin);
         nir_io_semantics sem = nir_intrinsic_io_semantics(intrin);

         nir_alu_type *output_type;
         nir_def **output_data =
            get_output_and_type(outputs, sem.location, sem.high_16bits, &output_type);

         u_foreach_bit (i, nir_intrinsic_write_mask(intrin)) {
            unsigned comp = nir_intrinsic_component(intrin) + i;
            output_data[comp] = nir_channel(b, intrin->src[0].ssa, i);

            if (output_type)
               output_type[comp] = type;
         }

         /* remove all store output instruction */
         nir_instr_remove(instr);
      }
   }
}

void
ac_nir_lower_legacy_vs(nir_shader *nir,
                       enum amd_gfx_level gfx_level,
                       uint32_t clip_cull_mask,
                       const uint8_t *param_offsets,
                       bool has_param_exports,
                       bool export_primitive_id,
                       bool disable_streamout,
                       bool kill_pointsize,
                       bool kill_layer,
                       bool force_vrs)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   nir_metadata preserved = nir_metadata_block_index | nir_metadata_dominance;

   nir_builder b = nir_builder_at(nir_after_impl(impl));

   nir_alu_type output_types_16bit_lo[16][4];
   nir_alu_type output_types_16bit_hi[16][4];
   struct shader_outputs outputs = {
      .type_16bit_lo = output_types_16bit_lo,
      .type_16bit_hi = output_types_16bit_hi,
   };
   gather_outputs(&b, impl, &outputs);

   if (export_primitive_id) {
      /* When the primitive ID is read by FS, we must ensure that it's exported by the previous
       * vertex stage because it's implicit for VS or TES (but required by the Vulkan spec for GS
       * or MS).
       */
      outputs.data[VARYING_SLOT_PRIMITIVE_ID][0] = nir_load_primitive_id(&b);

      /* Update outputs_written to reflect that the pass added a new output. */
      nir->info.outputs_written |= BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_ID);
   }

   if (!disable_streamout && nir->xfb_info) {
      emit_streamout(&b, 0, nir->xfb_info, &outputs);
      preserved = nir_metadata_none;
   }

   uint64_t export_outputs = nir->info.outputs_written | VARYING_BIT_POS;
   if (kill_pointsize)
      export_outputs &= ~VARYING_BIT_PSIZ;
   if (kill_layer)
      export_outputs &= ~VARYING_BIT_LAYER;

   ac_nir_export_position(&b, gfx_level, clip_cull_mask, !has_param_exports,
                          force_vrs, true, export_outputs, outputs.data, NULL);

   if (has_param_exports) {
      ac_nir_export_parameters(&b, param_offsets,
                               nir->info.outputs_written,
                               nir->info.outputs_written_16bit,
                               outputs.data,
                               outputs.data_16bit_lo,
                               outputs.data_16bit_hi);
   }

   nir_metadata_preserve(impl, preserved);
}

bool
ac_nir_gs_shader_query(nir_builder *b,
                       bool has_gen_prim_query,
                       bool has_pipeline_stats_query,
                       unsigned num_vertices_per_primitive,
                       unsigned wave_size,
                       nir_def *vertex_count[4],
                       nir_def *primitive_count[4])
{
   nir_def *pipeline_query_enabled = NULL;
   nir_def *prim_gen_query_enabled = NULL;
   nir_def *shader_query_enabled = NULL;
   if (has_gen_prim_query) {
      prim_gen_query_enabled = nir_load_prim_gen_query_enabled_amd(b);
      if (has_pipeline_stats_query) {
         pipeline_query_enabled = nir_load_pipeline_stat_query_enabled_amd(b);
         shader_query_enabled = nir_ior(b, pipeline_query_enabled, prim_gen_query_enabled);
      } else {
         shader_query_enabled = prim_gen_query_enabled;
      }
   } else if (has_pipeline_stats_query) {
      pipeline_query_enabled = nir_load_pipeline_stat_query_enabled_amd(b);
      shader_query_enabled = pipeline_query_enabled;
   } else {
      /* has no query */
      return false;
   }

   nir_if *if_shader_query = nir_push_if(b, shader_query_enabled);

   nir_def *active_threads_mask = nir_ballot(b, 1, wave_size, nir_imm_true(b));
   nir_def *num_active_threads = nir_bit_count(b, active_threads_mask);

   /* Calculate the "real" number of emitted primitives from the emitted GS vertices and primitives.
    * GS emits points, line strips or triangle strips.
    * Real primitives are points, lines or triangles.
    */
   nir_def *num_prims_in_wave[4] = {0};
   u_foreach_bit (i, b->shader->info.gs.active_stream_mask) {
      assert(vertex_count[i] && primitive_count[i]);

      nir_scalar vtx_cnt = nir_get_scalar(vertex_count[i], 0);
      nir_scalar prm_cnt = nir_get_scalar(primitive_count[i], 0);

      if (nir_scalar_is_const(vtx_cnt) && nir_scalar_is_const(prm_cnt)) {
         unsigned gs_vtx_cnt = nir_scalar_as_uint(vtx_cnt);
         unsigned gs_prm_cnt = nir_scalar_as_uint(prm_cnt);
         unsigned total_prm_cnt = gs_vtx_cnt - gs_prm_cnt * (num_vertices_per_primitive - 1u);
         if (total_prm_cnt == 0)
            continue;

         num_prims_in_wave[i] = nir_imul_imm(b, num_active_threads, total_prm_cnt);
      } else {
         nir_def *gs_vtx_cnt = vtx_cnt.def;
         nir_def *gs_prm_cnt = prm_cnt.def;
         if (num_vertices_per_primitive > 1)
            gs_prm_cnt = nir_iadd(b, nir_imul_imm(b, gs_prm_cnt, -1u * (num_vertices_per_primitive - 1)), gs_vtx_cnt);
         num_prims_in_wave[i] = nir_reduce(b, gs_prm_cnt, .reduction_op = nir_op_iadd);
      }
   }

   /* Store the query result to query result using an atomic add. */
   nir_if *if_first_lane = nir_push_if(b, nir_elect(b, 1));
   {
      if (has_pipeline_stats_query) {
         nir_if *if_pipeline_query = nir_push_if(b, pipeline_query_enabled);
         {
            nir_def *count = NULL;

            /* Add all streams' number to the same counter. */
            for (int i = 0; i < 4; i++) {
               if (num_prims_in_wave[i]) {
                  if (count)
                     count = nir_iadd(b, count, num_prims_in_wave[i]);
                  else
                     count = num_prims_in_wave[i];
               }
            }

            if (count)
               nir_atomic_add_gs_emit_prim_count_amd(b, count);

            nir_atomic_add_shader_invocation_count_amd(b, num_active_threads);
         }
         nir_pop_if(b, if_pipeline_query);
      }

      if (has_gen_prim_query) {
         nir_if *if_prim_gen_query = nir_push_if(b, prim_gen_query_enabled);
         {
            /* Add to the counter for this stream. */
            for (int i = 0; i < 4; i++) {
               if (num_prims_in_wave[i])
                  nir_atomic_add_gen_prim_count_amd(b, num_prims_in_wave[i], .stream_id = i);
            }
         }
         nir_pop_if(b, if_prim_gen_query);
      }
   }
   nir_pop_if(b, if_first_lane);

   nir_pop_if(b, if_shader_query);
   return true;
}

typedef struct {
   nir_def *outputs[64][4];
   nir_def *outputs_16bit_lo[16][4];
   nir_def *outputs_16bit_hi[16][4];

   ac_nir_gs_output_info *info;

   nir_def *vertex_count[4];
   nir_def *primitive_count[4];
} lower_legacy_gs_state;

static bool
lower_legacy_gs_store_output(nir_builder *b, nir_intrinsic_instr *intrin,
                             lower_legacy_gs_state *s)
{
   /* Assume:
    * - the shader used nir_lower_io_to_temporaries
    * - 64-bit outputs are lowered
    * - no indirect indexing is present
    */
   assert(nir_src_is_const(intrin->src[1]) && !nir_src_as_uint(intrin->src[1]));

   b->cursor = nir_before_instr(&intrin->instr);

   unsigned component = nir_intrinsic_component(intrin);
   unsigned write_mask = nir_intrinsic_write_mask(intrin);
   nir_io_semantics sem = nir_intrinsic_io_semantics(intrin);

   nir_def **outputs;
   if (sem.location < VARYING_SLOT_VAR0_16BIT) {
      outputs = s->outputs[sem.location];
   } else {
      unsigned index = sem.location - VARYING_SLOT_VAR0_16BIT;
      if (sem.high_16bits)
         outputs = s->outputs_16bit_hi[index];
      else
         outputs = s->outputs_16bit_lo[index];
   }

   nir_def *store_val = intrin->src[0].ssa;
   /* 64bit output has been lowered to 32bit */
   assert(store_val->bit_size <= 32);

   u_foreach_bit (i, write_mask) {
      unsigned comp = component + i;
      outputs[comp] = nir_channel(b, store_val, i);
   }

   nir_instr_remove(&intrin->instr);
   return true;
}

static bool
lower_legacy_gs_emit_vertex_with_counter(nir_builder *b, nir_intrinsic_instr *intrin,
                                         lower_legacy_gs_state *s)
{
   b->cursor = nir_before_instr(&intrin->instr);

   unsigned stream = nir_intrinsic_stream_id(intrin);
   nir_def *vtxidx = intrin->src[0].ssa;

   nir_def *gsvs_ring = nir_load_ring_gsvs_amd(b, .stream_id = stream);
   nir_def *soffset = nir_load_ring_gs2vs_offset_amd(b);

   unsigned offset = 0;
   u_foreach_bit64 (i, b->shader->info.outputs_written) {
      for (unsigned j = 0; j < 4; j++) {
         nir_def *output = s->outputs[i][j];
         /* Next vertex emit need a new value, reset all outputs. */
         s->outputs[i][j] = NULL;

         if (!(s->info->usage_mask[i] & (1 << j)) ||
             ((s->info->streams[i] >> (j * 2)) & 0x3) != stream)
            continue;

         unsigned base = offset * b->shader->info.gs.vertices_out * 4;
         offset++;

         /* no one set this output, skip the buffer store */
         if (!output)
            continue;

         nir_def *voffset = nir_ishl_imm(b, vtxidx, 2);

         /* extend 8/16 bit to 32 bit, 64 bit has been lowered */
         nir_def *data = nir_u2uN(b, output, 32);

         nir_store_buffer_amd(b, data, gsvs_ring, voffset, soffset, nir_imm_int(b, 0),
                              .access = ACCESS_COHERENT | ACCESS_NON_TEMPORAL |
                                        ACCESS_IS_SWIZZLED_AMD,
                              .base = base,
                              /* For ACO to not reorder this store around EmitVertex/EndPrimitve */
                              .memory_modes = nir_var_shader_out);
      }
   }

   u_foreach_bit (i, b->shader->info.outputs_written_16bit) {
      for (unsigned j = 0; j < 4; j++) {
         nir_def *output_lo = s->outputs_16bit_lo[i][j];
         nir_def *output_hi = s->outputs_16bit_hi[i][j];
         /* Next vertex emit need a new value, reset all outputs. */
         s->outputs_16bit_lo[i][j] = NULL;
         s->outputs_16bit_hi[i][j] = NULL;

         bool has_lo_16bit = (s->info->usage_mask_16bit_lo[i] & (1 << j)) &&
            ((s->info->streams_16bit_lo[i] >> (j * 2)) & 0x3) == stream;
         bool has_hi_16bit = (s->info->usage_mask_16bit_hi[i] & (1 << j)) &&
            ((s->info->streams_16bit_hi[i] >> (j * 2)) & 0x3) == stream;
         if (!has_lo_16bit && !has_hi_16bit)
            continue;

         unsigned base = offset * b->shader->info.gs.vertices_out;
         offset++;

         bool has_lo_16bit_out = has_lo_16bit && output_lo;
         bool has_hi_16bit_out = has_hi_16bit && output_hi;

         /* no one set needed output, skip the buffer store */
         if (!has_lo_16bit_out && !has_hi_16bit_out)
            continue;

         if (!has_lo_16bit_out)
            output_lo = nir_undef(b, 1, 16);

         if (!has_hi_16bit_out)
            output_hi = nir_undef(b, 1, 16);

         nir_def *voffset = nir_iadd_imm(b, vtxidx, base);
         voffset = nir_ishl_imm(b, voffset, 2);

         nir_store_buffer_amd(b, nir_pack_32_2x16_split(b, output_lo, output_hi),
                              gsvs_ring, voffset, soffset, nir_imm_int(b, 0),
                              .access = ACCESS_COHERENT | ACCESS_NON_TEMPORAL |
                                        ACCESS_IS_SWIZZLED_AMD,
                              /* For ACO to not reorder this store around EmitVertex/EndPrimitve */
                              .memory_modes = nir_var_shader_out);
      }
   }

   /* Signal vertex emission. */
   nir_sendmsg_amd(b, nir_load_gs_wave_id_amd(b),
                   .base = AC_SENDMSG_GS_OP_EMIT | AC_SENDMSG_GS | (stream << 8));

   nir_instr_remove(&intrin->instr);
   return true;
}

static bool
lower_legacy_gs_set_vertex_and_primitive_count(nir_builder *b, nir_intrinsic_instr *intrin,
                                               lower_legacy_gs_state *s)
{
   b->cursor = nir_before_instr(&intrin->instr);

   unsigned stream = nir_intrinsic_stream_id(intrin);

   s->vertex_count[stream] = intrin->src[0].ssa;
   s->primitive_count[stream] = intrin->src[1].ssa;

   nir_instr_remove(&intrin->instr);
   return true;
}

static bool
lower_legacy_gs_end_primitive_with_counter(nir_builder *b, nir_intrinsic_instr *intrin,
                                               lower_legacy_gs_state *s)
{
   b->cursor = nir_before_instr(&intrin->instr);
   const unsigned stream = nir_intrinsic_stream_id(intrin);

   /* Signal primitive emission. */
   nir_sendmsg_amd(b, nir_load_gs_wave_id_amd(b),
                   .base = AC_SENDMSG_GS_OP_CUT | AC_SENDMSG_GS | (stream << 8));

   nir_instr_remove(&intrin->instr);
   return true;
}

static bool
lower_legacy_gs_intrinsic(nir_builder *b, nir_instr *instr, void *state)
{
   lower_legacy_gs_state *s = (lower_legacy_gs_state *) state;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   if (intrin->intrinsic == nir_intrinsic_store_output)
      return lower_legacy_gs_store_output(b, intrin, s);
   else if (intrin->intrinsic == nir_intrinsic_emit_vertex_with_counter)
      return lower_legacy_gs_emit_vertex_with_counter(b, intrin, s);
   else if (intrin->intrinsic == nir_intrinsic_end_primitive_with_counter)
      return lower_legacy_gs_end_primitive_with_counter(b, intrin, s);
   else if (intrin->intrinsic == nir_intrinsic_set_vertex_and_primitive_count)
      return lower_legacy_gs_set_vertex_and_primitive_count(b, intrin, s);

   return false;
}

void
ac_nir_lower_legacy_gs(nir_shader *nir,
                       bool has_gen_prim_query,
                       bool has_pipeline_stats_query,
                       ac_nir_gs_output_info *output_info)
{
   lower_legacy_gs_state s = {
      .info = output_info,
   };

   unsigned num_vertices_per_primitive = 0;
   switch (nir->info.gs.output_primitive) {
   case MESA_PRIM_POINTS:
      num_vertices_per_primitive = 1;
      break;
   case MESA_PRIM_LINE_STRIP:
      num_vertices_per_primitive = 2;
      break;
   case MESA_PRIM_TRIANGLE_STRIP:
      num_vertices_per_primitive = 3;
      break;
   default:
      unreachable("Invalid GS output primitive.");
      break;
   }

   nir_shader_instructions_pass(nir, lower_legacy_gs_intrinsic,
                                nir_metadata_block_index | nir_metadata_dominance, &s);

   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   nir_builder builder = nir_builder_at(nir_after_impl(impl));
   nir_builder *b = &builder;

   /* Emit shader query for mix use legacy/NGG GS */
   bool progress = ac_nir_gs_shader_query(b,
                                          has_gen_prim_query,
                                          has_pipeline_stats_query,
                                          num_vertices_per_primitive,
                                          64,
                                          s.vertex_count,
                                          s.primitive_count);

   /* Wait for all stores to finish. */
   nir_barrier(b, .execution_scope = SCOPE_INVOCATION,
                      .memory_scope = SCOPE_DEVICE,
                      .memory_semantics = NIR_MEMORY_RELEASE,
                      .memory_modes = nir_var_shader_out | nir_var_mem_ssbo |
                                      nir_var_mem_global | nir_var_image);

   /* Signal that the GS is done. */
   nir_sendmsg_amd(b, nir_load_gs_wave_id_amd(b),
                   .base = AC_SENDMSG_GS_OP_NOP | AC_SENDMSG_GS_DONE);

   if (progress)
      nir_metadata_preserve(impl, nir_metadata_none);
}
