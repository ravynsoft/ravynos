/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "compiler.h"
#include "midgard.h"
#include "midgard_ops.h"
#include "nir.h"

static bool
is_inot(midgard_instruction *I)
{
   return I->type == TAG_ALU_4 && I->op == midgard_alu_op_inor &&
          I->has_inline_constant && I->inline_constant == 0;
}

static bool
try_fold_fmov_src(midgard_instruction *use, unsigned src_idx,
                  midgard_instruction *fmov)
{
   if (use->type != TAG_ALU_4)
      return false;
   if (fmov->has_constants || fmov->has_inline_constant)
      return false;
   if (mir_nontrivial_outmod(fmov))
      return false;

   /* Don't propagate into non-float instructions */
   if (nir_alu_type_get_base_type(use->src_types[src_idx]) != nir_type_float)
      return false;

   /* TODO: Size conversions not handled yet */
   if (use->src_types[src_idx] != fmov->src_types[1])
      return false;

   if (use->src_abs[src_idx]) {
      /* abs(abs(x)) = abs(x) and abs(-x) = abs(x) */
   } else {
      /* -(-(abs(x))) = abs(x) */
      use->src_abs[src_idx] = fmov->src_abs[1];
      use->src_neg[src_idx] ^= fmov->src_neg[1];
   }

   use->src[src_idx] = fmov->src[1];
   mir_compose_swizzle(use->swizzle[src_idx], fmov->swizzle[1],
                       use->swizzle[src_idx]);
   return true;
}

static bool
try_fold_inot(midgard_instruction *use, unsigned src_idx,
              midgard_instruction *inot)
{
   /* TODO: Size conversions not handled yet */
   if (nir_alu_type_get_type_size(use->src_types[src_idx]) !=
       nir_alu_type_get_type_size(inot->src_types[0]))
      return false;

   if (use->compact_branch) {
      use->branch.invert_conditional ^= true;
   } else if (use->type == TAG_ALU_4) {
      switch (use->op) {
      case midgard_alu_op_iand:
      case midgard_alu_op_ior:
      case midgard_alu_op_ixor:
         break;
      default:
         return false;
      }

      use->src_invert[src_idx] ^= true;
      mir_compose_swizzle(use->swizzle[src_idx], inot->swizzle[0],
                          use->swizzle[src_idx]);
   } else {
      return false;
   }

   use->src[src_idx] = inot->src[0];
   return true;
}

static bool
midgard_opt_prop_forward(compiler_context *ctx)
{
   bool progress = false;

   midgard_instruction **defs =
      calloc(ctx->temp_count, sizeof(midgard_instruction *));

   mir_foreach_block(ctx, block_) {
      midgard_block *block = (midgard_block *)block_;

      mir_foreach_instr_in_block(block, I) {
         /* Record SSA defs */
         if (mir_is_ssa(I->dest)) {
            assert(I->dest < ctx->temp_count);
            defs[I->dest] = I;
         }

         mir_foreach_src(I, s) {
            unsigned src = I->src[s];
            if (!mir_is_ssa(src))
               continue;

            /* Try to fold a source mod in */
            assert(src < ctx->temp_count);
            midgard_instruction *def = defs[src];
            if (def == NULL)
               continue;

            if (def->type == TAG_ALU_4 && def->op == midgard_alu_op_fmov) {
               progress |= try_fold_fmov_src(I, s, def);
            } else if (is_inot(def)) {
               progress |= try_fold_inot(I, s, def);
            }
         }
      }
   }

   free(defs);
   return progress;
}

enum outmod_state {
   outmod_unknown = 0,
   outmod_clamp_0_1,
   outmod_clamp_m1_1,
   outmod_clamp_0_inf,
   outmod_incompatible,
};

static enum outmod_state
outmod_to_state(unsigned outmod)
{
   switch (outmod) {
   case midgard_outmod_clamp_0_1:
      return outmod_clamp_0_1;
   case midgard_outmod_clamp_m1_1:
      return outmod_clamp_m1_1;
   case midgard_outmod_clamp_0_inf:
      return outmod_clamp_0_inf;
   default:
      return outmod_incompatible;
   }
}

static enum outmod_state
union_outmod_state(enum outmod_state a, enum outmod_state b)
{
   if (a == outmod_unknown)
      return b;
   else if (b == outmod_unknown)
      return a;
   else if (a == b)
      return a /* b */;
   else
      return outmod_incompatible;
}

static bool
midgard_opt_prop_backward(compiler_context *ctx)
{
   bool progress = false;
   enum outmod_state *state = calloc(ctx->temp_count, sizeof(*state));
   BITSET_WORD *folded = calloc(BITSET_WORDS(ctx->temp_count), sizeof(*folded));

   /* Scan for outmod states */
   mir_foreach_instr_global(ctx, I) {
      if (I->type == TAG_ALU_4 && I->op == midgard_alu_op_fmov &&
          !I->src_neg[1] && !I->src_abs[1] && mir_is_ssa(I->src[1])) {

         enum outmod_state outmod = outmod_to_state(I->outmod);
         state[I->src[1]] = union_outmod_state(state[I->src[1]], outmod);
      } else {
         /* Anything used as any other source cannot have an outmod folded in */
         mir_foreach_src(I, s) {
            if (mir_is_ssa(I->src[s]))
               state[I->src[s]] = outmod_incompatible;
         }
      }
   }

   /* Apply outmods */
   mir_foreach_instr_global(ctx, I) {
      if (!mir_is_ssa(I->dest))
         continue;

      if (I->type != TAG_ALU_4 && I->type != TAG_TEXTURE_4)
         continue;

      if (nir_alu_type_get_base_type(I->dest_type) != nir_type_float)
         continue;

      if (I->outmod != midgard_outmod_none)
         continue;

      switch (state[I->dest]) {
      case outmod_clamp_0_1:
         I->outmod = midgard_outmod_clamp_0_1;
         break;
      case outmod_clamp_m1_1:
         I->outmod = midgard_outmod_clamp_m1_1;
         break;
      case outmod_clamp_0_inf:
         I->outmod = midgard_outmod_clamp_0_inf;
         break;
      default:
         break;
      }

      if (I->outmod != midgard_outmod_none) {
         BITSET_SET(folded, I->dest);
      }
   }

   /* Strip outmods from FMOVs to let copyprop go ahead */
   mir_foreach_instr_global(ctx, I) {
      if (I->type == TAG_ALU_4 && I->op == midgard_alu_op_fmov &&
          mir_is_ssa(I->src[1]) && BITSET_TEST(folded, I->src[1])) {

         I->outmod = midgard_outmod_none;
      }
   }

   free(state);
   free(folded);
   return progress;
}

bool
midgard_opt_prop(compiler_context *ctx)
{
   bool progress = false;
   mir_compute_temp_count(ctx);
   progress |= midgard_opt_prop_forward(ctx);
   progress |= midgard_opt_prop_backward(ctx);
   return progress;
}
