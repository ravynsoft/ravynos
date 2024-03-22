/*
 * Copyright © 2016 Intel Corporation
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

#include "compiler/nir/nir_builder.h"
#include "brw_nir.h"

/**
 * Prior to Haswell, the hardware can't natively support GL_FIXED or
 * 2_10_10_10_REV vertex formats.  This pass inserts extra shader code
 * to produce the correct values.
 */

static bool
apply_attr_wa_instr(nir_builder *b, nir_instr *instr, void *cb_data)
{
   const uint8_t *attrib_wa_flags = cb_data;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   if (intrin->intrinsic != nir_intrinsic_load_input)
      return false;

   uint8_t wa_flags = attrib_wa_flags[nir_intrinsic_base(intrin)];
   if (wa_flags == 0)
      return false;

   b->cursor = nir_after_instr(instr);

   nir_def *val = &intrin->def;

   /* Do GL_FIXED rescaling for GLES2.0.  Our GL_FIXED attributes
    * come in as floating point conversions of the integer values.
    */
   if (wa_flags & BRW_ATTRIB_WA_COMPONENT_MASK) {
      nir_def *scaled =
         nir_fmul_imm(b, val, 1.0f / 65536.0f);
      nir_def *comps[4];
      for (int i = 0; i < val->num_components; i++) {
         bool rescale = i < (wa_flags & BRW_ATTRIB_WA_COMPONENT_MASK);
         comps[i] = nir_channel(b, rescale ? scaled : val, i);
      }
      val = nir_vec(b, comps, val->num_components);
   }

   /* Do sign recovery for 2101010 formats if required. */
   if (wa_flags & BRW_ATTRIB_WA_SIGN) {
      /* sign recovery shift: <22, 22, 22, 30> */
      nir_def *shift = nir_imm_ivec4(b, 22, 22, 22, 30);
      val = nir_ishr(b, nir_ishl(b, val, shift), shift);
   }

   /* Apply BGRA swizzle if required. */
   if (wa_flags & BRW_ATTRIB_WA_BGRA) {
      val = nir_swizzle(b, val, (unsigned[4]){2,1,0,3}, 4);
   }

   if (wa_flags & BRW_ATTRIB_WA_NORMALIZE) {
      /* ES 3.0 has different rules for converting signed normalized
       * fixed-point numbers than desktop GL.
       */
      if (wa_flags & BRW_ATTRIB_WA_SIGN) {
         /* According to equation 2.2 of the ES 3.0 specification,
          * signed normalization conversion is done by:
          *
          * f = c / (2^(b-1)-1)
          *
          * OpenGL 4.2+ uses this equation as well.  Since most contexts
          * promote to the new higher version, and this is what Haswell+
          * hardware does anyway, we just always use this formula.
          */
         nir_def *es3_normalize_factor =
            nir_imm_vec4(b, 1.0f / ((1 << 9) - 1), 1.0f / ((1 << 9) - 1),
                            1.0f / ((1 << 9) - 1), 1.0f / ((1 << 1) - 1));
         val = nir_fmax(b,
                        nir_fmul(b, nir_i2f32(b, val), es3_normalize_factor),
                        nir_imm_float(b, -1.0f));
      } else {
         /* The following equation is from the OpenGL 3.2 specification:
          *
          * 2.1 unsigned normalization
          * f = c/(2^n-1)
          */
         nir_def *normalize_factor =
            nir_imm_vec4(b, 1.0f / ((1 << 10) - 1), 1.0f / ((1 << 10) - 1),
                            1.0f / ((1 << 10) - 1), 1.0f / ((1 << 2)  - 1));

         val = nir_fmul(b, nir_u2f32(b, val), normalize_factor);
      }
   }

   if (wa_flags & BRW_ATTRIB_WA_SCALE) {
      val = (wa_flags & BRW_ATTRIB_WA_SIGN) ? nir_i2f32(b, val)
                                            : nir_u2f32(b, val);
   }

   nir_def_rewrite_uses_after(&intrin->def, val,
                                  val->parent_instr);

   return true;
}

bool
brw_nir_apply_attribute_workarounds(nir_shader *shader,
                                    const uint8_t *attrib_wa_flags)
{
   return nir_shader_instructions_pass(shader, apply_attr_wa_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       (void *)attrib_wa_flags);
}
