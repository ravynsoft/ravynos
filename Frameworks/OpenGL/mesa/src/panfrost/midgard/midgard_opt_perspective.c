/*
 * Copyright (C) 2019 Collabora, Ltd.
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
 */

/* Midgard has some accelerated support for perspective projection on the
 * load/store pipes. So the first perspective projection pass looks for
 * lowered/open-coded perspective projection of the form "fmul (A.xyz,
 * frcp(A.w))" or "fmul (A.xy, frcp(A.z))" and rewrite with a native
 * perspective division opcode (on the load/store pipe). Caveats apply: the
 * frcp should be used only once to make this optimization worthwhile. And the
 * source of the frcp ought to be a varying to make it worthwhile...
 *
 * The second pass in this file is a step #2 of sorts: fusing that load/store
 * projection into a varying load instruction (they can be done together
 * implicitly). This depends on the combination pass. Again caveat: the vary
 * should only be used once to make this worthwhile.
 */

#include "compiler.h"

static bool
is_swizzle_0(unsigned *swizzle)
{
   for (unsigned c = 0; c < MIR_VEC_COMPONENTS; ++c)
      if (swizzle[c])
         return false;

   return true;
}

bool
midgard_opt_combine_projection(compiler_context *ctx, midgard_block *block)
{
   bool progress = false;

   mir_foreach_instr_in_block_safe(block, ins) {
      /* First search for fmul */
      if (ins->type != TAG_ALU_4)
         continue;
      if (ins->op != midgard_alu_op_fmul)
         continue;

      /* TODO: Flip */

      /* Check the swizzles */

      if (!mir_is_simple_swizzle(ins->swizzle[0], ins->mask))
         continue;
      if (!is_swizzle_0(ins->swizzle[1]))
         continue;

      /* Awesome, we're the right form. Now check where src2 is from */
      unsigned frcp = ins->src[1];
      unsigned to = ins->dest;

      if (frcp & PAN_IS_REG)
         continue;
      if (to & PAN_IS_REG)
         continue;

      bool frcp_found = false;
      unsigned frcp_component = 0;
      unsigned frcp_from = 0;

      mir_foreach_instr_in_block_safe(block, sub) {
         if (sub->dest != frcp)
            continue;

         frcp_component = sub->swizzle[0][0];
         frcp_from = sub->src[0];

         frcp_found =
            (sub->type == TAG_ALU_4) && (sub->op == midgard_alu_op_frcp);
         break;
      }

      if (!frcp_found)
         continue;
      if (frcp_from != ins->src[0])
         continue;
      if (frcp_component != COMPONENT_W && frcp_component != COMPONENT_Z)
         continue;
      if (!mir_single_use(ctx, frcp))
         continue;

      /* Heuristic: check if the frcp is from a single-use varying */

      bool ok = false;

      /* One for frcp and one for fmul */
      if (mir_use_count(ctx, frcp_from) > 2)
         continue;

      mir_foreach_instr_in_block_safe(block, v) {
         if (v->dest != frcp_from)
            continue;
         if (v->type != TAG_LOAD_STORE_4)
            break;
         if (!OP_IS_LOAD_VARY_F(v->op))
            break;

         ok = true;
         break;
      }

      if (!ok)
         continue;

      /* Nice, we got the form spot on. Let's convert! */

      midgard_instruction accel = {
         .type = TAG_LOAD_STORE_4,
         .mask = ins->mask,
         .dest = to,
         .dest_type = nir_type_float32,
         .src =
            {
               frcp_from,
               ~0,
               ~0,
               ~0,
            },
         .src_types =
            {
               nir_type_float32,
            },
         .swizzle = SWIZZLE_IDENTITY_4,
         .op = frcp_component == COMPONENT_W
                  ? midgard_op_ldst_perspective_div_w
                  : midgard_op_ldst_perspective_div_z,
         .load_store =
            {
               .bitsize_toggle = true,
            },
      };

      mir_insert_instruction_before(ctx, ins, accel);
      mir_remove_instruction(ins);

      progress |= true;
   }

   return progress;
}

bool
midgard_opt_varying_projection(compiler_context *ctx, midgard_block *block)
{
   bool progress = false;

   mir_foreach_instr_in_block_safe(block, ins) {
      /* Search for a projection */
      if (ins->type != TAG_LOAD_STORE_4)
         continue;
      if (!OP_IS_PROJECTION(ins->op))
         continue;

      unsigned vary = ins->src[0];
      unsigned to = ins->dest;

      if (vary & PAN_IS_REG)
         continue;
      if (to & PAN_IS_REG)
         continue;
      if (!mir_single_use(ctx, vary))
         continue;

      /* Check for a varying source. If we find it, we rewrite */

      bool rewritten = false;

      mir_foreach_instr_in_block_safe(block, v) {
         if (v->dest != vary)
            continue;
         if (v->type != TAG_LOAD_STORE_4)
            break;
         if (!OP_IS_LOAD_VARY_F(v->op))
            break;

         /* We found it, so rewrite it to project. Grab the
          * modifier */

         midgard_varying_params p =
            midgard_unpack_varying_params(v->load_store);

         if (p.modifier != midgard_varying_mod_none)
            break;

         bool projects_w = ins->op == midgard_op_ldst_perspective_div_w;

         p.modifier = projects_w ? midgard_varying_mod_perspective_w
                                 : midgard_varying_mod_perspective_z;

         midgard_pack_varying_params(&v->load_store, p);

         /* Use the new destination */
         v->dest = to;

         rewritten = true;
         break;
      }

      if (rewritten)
         mir_remove_instruction(ins);

      progress |= rewritten;
   }

   return progress;
}
