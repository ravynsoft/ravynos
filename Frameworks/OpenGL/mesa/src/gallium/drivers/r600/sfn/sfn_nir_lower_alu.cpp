/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2022 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "sfn_nir_lower_alu.h"

#include "sfn_nir.h"

namespace r600 {

class Lower2x16 : public NirLowerInstruction {
private:
   bool filter(const nir_instr *instr) const override;
   nir_def *lower(nir_instr *instr) override;
};

bool
Lower2x16::filter(const nir_instr *instr) const
{
   if (instr->type != nir_instr_type_alu)
      return false;
   auto alu = nir_instr_as_alu(instr);
   switch (alu->op) {
   case nir_op_unpack_half_2x16:
   case nir_op_pack_half_2x16:
      return true;
   default:
      return false;
   }
}

nir_def *
Lower2x16::lower(nir_instr *instr)
{
   nir_alu_instr *alu = nir_instr_as_alu(instr);

   switch (alu->op) {
   case nir_op_unpack_half_2x16: {
      nir_def *packed = nir_ssa_for_alu_src(b, alu, 0);
      return nir_vec2(b,
                      nir_unpack_half_2x16_split_x(b, packed),
                      nir_unpack_half_2x16_split_y(b, packed));
   }
   case nir_op_pack_half_2x16: {
      nir_def *src_vec2 = nir_ssa_for_alu_src(b, alu, 0);
      return nir_pack_half_2x16_split(b,
                                      nir_channel(b, src_vec2, 0),
                                      nir_channel(b, src_vec2, 1));
   }
   default:
      unreachable("Lower2x16 filter doesn't filter correctly");
   }
}

class LowerSinCos : public NirLowerInstruction {
public:
   LowerSinCos(amd_gfx_level gxf_level):
       m_gxf_level(gxf_level)
   {
   }

private:
   bool filter(const nir_instr *instr) const override;
   nir_def *lower(nir_instr *instr) override;
   amd_gfx_level m_gxf_level;
};

bool
LowerSinCos::filter(const nir_instr *instr) const
{
   if (instr->type != nir_instr_type_alu)
      return false;

   auto alu = nir_instr_as_alu(instr);
   switch (alu->op) {
   case nir_op_fsin:
   case nir_op_fcos:
      return true;
   default:
      return false;
   }
}

nir_def *
LowerSinCos::lower(nir_instr *instr)
{
   auto alu = nir_instr_as_alu(instr);

   assert(alu->op == nir_op_fsin || alu->op == nir_op_fcos);

   auto fract = nir_ffract(b,
                           nir_ffma_imm12(b,
                                          nir_ssa_for_alu_src(b, alu, 0),
                                          0.15915494,
                                          0.5));

   auto normalized =
      m_gxf_level != R600
         ? nir_fadd_imm(b, fract, -0.5)
         : nir_ffma_imm12(b, fract, 2.0f * M_PI, -M_PI);

   if (alu->op == nir_op_fsin)
      return nir_fsin_amd(b, normalized);
   else
      return nir_fcos_amd(b, normalized);
}

} // namespace r600

bool
r600_nir_lower_pack_unpack_2x16(nir_shader *shader)
{
   return r600::Lower2x16().run(shader);
}

bool
r600_nir_lower_trigen(nir_shader *shader, amd_gfx_level gfx_level)
{
   return r600::LowerSinCos(gfx_level).run(shader);
}
