/*
 * Copyright © 2013 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file brw_vec4_tes.cpp
 *
 * Tessellaton evaluation shader specific code derived from the vec4_visitor class.
 */

#include "brw_vec4_tes.h"
#include "brw_cfg.h"
#include "dev/intel_debug.h"

namespace brw {

vec4_tes_visitor::vec4_tes_visitor(const struct brw_compiler *compiler,
                                   const struct brw_compile_params *params,
                                  const struct brw_tes_prog_key *key,
                                  struct brw_tes_prog_data *prog_data,
                                  const nir_shader *shader,
                                  bool debug_enabled)
   : vec4_visitor(compiler, params, &key->base.tex, &prog_data->base,
                  shader, false, debug_enabled)
{
}

void
vec4_tes_visitor::setup_payload()
{
   int reg = 0;

   /* The payload always contains important data in r0 and r1, which contains
    * the URB handles that are passed on to the URB write at the end
    * of the thread.
    */
   reg += 2;

   reg = setup_uniforms(reg);

   foreach_block_and_inst(block, vec4_instruction, inst, cfg) {
      for (int i = 0; i < 3; i++) {
         if (inst->src[i].file != ATTR)
            continue;

         unsigned slot = inst->src[i].nr + inst->src[i].offset / 16;
         struct brw_reg grf = brw_vec4_grf(reg + slot / 2, 4 * (slot % 2));
         grf = stride(grf, 0, 4, 1);
         grf.swizzle = inst->src[i].swizzle;
         grf.type = inst->src[i].type;
         grf.abs = inst->src[i].abs;
         grf.negate = inst->src[i].negate;
         inst->src[i] = grf;
      }
   }

   reg += 8 * prog_data->urb_read_length;

   this->first_non_payload_grf = reg;
}


void
vec4_tes_visitor::emit_prolog()
{
   input_read_header = src_reg(this, glsl_uvec4_type());
   emit(TES_OPCODE_CREATE_INPUT_READ_HEADER, dst_reg(input_read_header));

   this->current_annotation = NULL;
}


void
vec4_tes_visitor::emit_urb_write_header(int mrf)
{
   /* No need to do anything for DS; an implied write to this MRF will be
    * performed by VEC4_VS_OPCODE_URB_WRITE.
    */
   (void) mrf;
}


vec4_instruction *
vec4_tes_visitor::emit_urb_write_opcode(bool complete)
{
   vec4_instruction *inst = emit(VEC4_VS_OPCODE_URB_WRITE);
   inst->urb_write_flags = complete ?
      BRW_URB_WRITE_EOT_COMPLETE : BRW_URB_WRITE_NO_FLAGS;

   return inst;
}

void
vec4_tes_visitor::nir_emit_intrinsic(nir_intrinsic_instr *instr)
{
   const struct brw_tes_prog_data *tes_prog_data =
      (const struct brw_tes_prog_data *) prog_data;

   switch (instr->intrinsic) {
   case nir_intrinsic_load_tess_coord:
      /* gl_TessCoord is part of the payload in g1 channels 0-2 and 4-6. */
      emit(MOV(get_nir_def(instr->def, BRW_REGISTER_TYPE_F),
               src_reg(brw_vec8_grf(1, 0))));
      break;
   case nir_intrinsic_load_tess_level_outer:
      if (tes_prog_data->domain == BRW_TESS_DOMAIN_ISOLINE) {
         emit(MOV(get_nir_def(instr->def, BRW_REGISTER_TYPE_F),
                  swizzle(src_reg(ATTR, 1, glsl_vec4_type()),
                          BRW_SWIZZLE_ZWZW)));
      } else {
         emit(MOV(get_nir_def(instr->def, BRW_REGISTER_TYPE_F),
                  swizzle(src_reg(ATTR, 1, glsl_vec4_type()),
                          BRW_SWIZZLE_WZYX)));
      }
      break;
   case nir_intrinsic_load_tess_level_inner:
      if (tes_prog_data->domain == BRW_TESS_DOMAIN_QUAD) {
         emit(MOV(get_nir_def(instr->def, BRW_REGISTER_TYPE_F),
                  swizzle(src_reg(ATTR, 0, glsl_vec4_type()),
                          BRW_SWIZZLE_WZYX)));
      } else {
         emit(MOV(get_nir_def(instr->def, BRW_REGISTER_TYPE_F),
                  src_reg(ATTR, 1, glsl_float_type())));
      }
      break;
   case nir_intrinsic_load_primitive_id:
      emit(TES_OPCODE_GET_PRIMITIVE_ID,
           get_nir_def(instr->def, BRW_REGISTER_TYPE_UD));
      break;

   case nir_intrinsic_load_input:
   case nir_intrinsic_load_per_vertex_input: {
      assert(instr->def.bit_size == 32);
      src_reg indirect_offset = get_indirect_offset(instr);
      unsigned imm_offset = instr->const_index[0];
      src_reg header = input_read_header;
      unsigned first_component = nir_intrinsic_component(instr);

      if (indirect_offset.file != BAD_FILE) {
         src_reg clamped_indirect_offset = src_reg(this, glsl_uvec4_type());

         /* Page 190 of "Volume 7: 3D Media GPGPU Engine (Haswell)" says the
          * valid range of the offset is [0, 0FFFFFFFh].
          */
         emit_minmax(BRW_CONDITIONAL_L,
                     dst_reg(clamped_indirect_offset),
                     retype(indirect_offset, BRW_REGISTER_TYPE_UD),
                     brw_imm_ud(0x0fffffffu));

         header = src_reg(this, glsl_uvec4_type());
         emit(TES_OPCODE_ADD_INDIRECT_URB_OFFSET, dst_reg(header),
              input_read_header, clamped_indirect_offset);
      } else {
         /* Arbitrarily only push up to 24 vec4 slots worth of data,
          * which is 12 registers (since each holds 2 vec4 slots).
          */
         const unsigned max_push_slots = 24;
         if (imm_offset < max_push_slots) {
            src_reg src = src_reg(ATTR, imm_offset, glsl_ivec4_type());
            src.swizzle = BRW_SWZ_COMP_INPUT(first_component);

            emit(MOV(get_nir_def(instr->def, BRW_REGISTER_TYPE_D), src));

            prog_data->urb_read_length =
               MAX2(prog_data->urb_read_length,
                    DIV_ROUND_UP(imm_offset + 1, 2));
            break;
         }
      }

      dst_reg temp(this, glsl_ivec4_type());
      vec4_instruction *read =
         emit(VEC4_OPCODE_URB_READ, temp, src_reg(header));
      read->offset = imm_offset;
      read->urb_write_flags = BRW_URB_WRITE_PER_SLOT_OFFSET;

      src_reg src = src_reg(temp);
      src.swizzle = BRW_SWZ_COMP_INPUT(first_component);

      /* Copy to target.  We might end up with some funky writemasks landing
       * in here, but we really don't want them in the above pseudo-ops.
       */
      dst_reg dst = get_nir_def(instr->def, BRW_REGISTER_TYPE_D);
      dst.writemask = brw_writemask_for_size(instr->num_components);
      emit(MOV(dst, src));
      break;
   }
   default:
      vec4_visitor::nir_emit_intrinsic(instr);
   }
}


void
vec4_tes_visitor::emit_thread_end()
{
   /* For DS, we always end the thread by emitting a single vertex.
    * emit_urb_write_opcode() will take care of setting the eot flag on the
    * SEND instruction.
    */
   emit_vertex();
}

} /* namespace brw */
