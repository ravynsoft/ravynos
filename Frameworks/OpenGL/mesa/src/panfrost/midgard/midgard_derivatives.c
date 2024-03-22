/*
 * Copyright (C) 2019 Collabora, Ltd.
 * Copyright (C) 2019-2020 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors (Collabora):
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "compiler.h"

/* Derivatives in Midgard are implemented on the texture pipe, rather than the
 * ALU pipe as suggested by NIR. The rationale is that normal texture
 * instructions require (implicit) derivatives to be calculated anyway, so it
 * makes sense to reuse the derivative logic. Thus, in addition to the usual
 * texturing ops that calculate derivatives, there are two explicit texture ops
 * dFdx/dFdy that perform differencing across helper invocations in either
 * horizontal or vertical directions.
 *
 * One major caveat is that derivatives can only be calculated on up to a vec2
 * at a time. This restriction presumably is to save some silicon, as 99% of
 * derivatives will be vec2 (autocalculating mip levels of 2D texture
 * coordinates). Admittedly I'm not sure why 3D textures can have their levels
 * calculated automatically, umm... Pressing on.
 *
 * This caveat is handled in two steps. During the first pass (code
 * generation), we generate texture ops 1:1 to the incoming NIR derivatives.
 * This works for float/vec2 but not for vec3/vec4. A later lowering pass will
 * scan for vec3/vec4 derivatives and lower (split) to multiple instructions.
 * This pass is separated as we'll have to rewrite th e destination into a
 * register (rather than SSA) and we'd rather do this after we have the whole
 * IR in front of us to do it at once.
 */

static unsigned
mir_derivative_mode(nir_op op)
{
   switch (op) {
   case nir_op_fddx:
   case nir_op_fddx_fine:
   case nir_op_fddx_coarse:
      return TEXTURE_DFDX;

   case nir_op_fddy:
   case nir_op_fddy_fine:
   case nir_op_fddy_coarse:
      return TEXTURE_DFDY;

   default:
      unreachable("Invalid derivative op");
   }
}

/* Returns true if a texturing op computes derivatives either explicitly or
 * implicitly */

bool
mir_op_computes_derivatives(gl_shader_stage stage, unsigned op)
{
   /* Only fragment shaders may compute derivatives, but the sense of
    * "normal" changes in vertex shaders on certain GPUs */

   if (op == midgard_tex_op_normal && stage != MESA_SHADER_FRAGMENT)
      return false;

   switch (op) {
   case midgard_tex_op_normal:
   case midgard_tex_op_derivative:
      assert(stage == MESA_SHADER_FRAGMENT);
      return true;
   default:
      return false;
   }
}

void
midgard_emit_derivatives(compiler_context *ctx, nir_alu_instr *instr)
{
   /* Create texture instructions */
   midgard_instruction ins = {
      .type = TAG_TEXTURE_4,
      .dest_type = nir_type_float32,
      .src =
         {
            ~0,
            nir_src_index(ctx, &instr->src[0].src),
            ~0,
            ~0,
         },
      .swizzle = SWIZZLE_IDENTITY_4,
      .src_types =
         {
            nir_type_float32,
            nir_type_float32,
         },
      .op = midgard_tex_op_derivative,
      .texture =
         {
            .mode = mir_derivative_mode(instr->op),
            .format = 2,
            .in_reg_full = 1,
            .out_full = 1,
            .sampler_type = MALI_SAMPLER_FLOAT,
         },
   };

   ins.dest = nir_def_index_with_mask(&instr->def, &ins.mask);
   emit_mir_instruction(ctx, ins);
}

void
midgard_lower_derivatives(compiler_context *ctx, midgard_block *block)
{
   mir_foreach_instr_in_block_safe(block, ins) {
      if (ins->type != TAG_TEXTURE_4)
         continue;
      if (ins->op != midgard_tex_op_derivative)
         continue;

      /* Check if we need to split */

      bool upper = ins->mask & 0b1100;
      bool lower = ins->mask & 0b0011;

      if (!(upper && lower))
         continue;

      /* Duplicate for dedicated upper instruction */

      midgard_instruction dup;
      memcpy(&dup, ins, sizeof(dup));

      /* Fixup masks. Make original just lower and dupe just upper */

      ins->mask &= 0b0011;
      dup.mask &= 0b1100;

      /* Fixup swizzles */
      dup.swizzle[0][0] = dup.swizzle[0][1] = dup.swizzle[0][2] = COMPONENT_X;
      dup.swizzle[0][3] = COMPONENT_Y;

      dup.swizzle[1][0] = COMPONENT_Z;
      dup.swizzle[1][1] = dup.swizzle[1][2] = dup.swizzle[1][3] = COMPONENT_W;

      /* Insert the new instruction */
      mir_insert_instruction_before(ctx, mir_next_op(ins), dup);

      /* We'll need both instructions to write to the same index, so
       * rewrite to use a register */

      unsigned new = make_compiler_temp_reg(ctx);
      mir_rewrite_index(ctx, ins->dest, new);
   }
}
