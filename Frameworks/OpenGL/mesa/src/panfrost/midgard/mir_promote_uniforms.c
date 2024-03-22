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
 *
 * Authors (Collabora):
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "util/u_math.h"
#include "util/u_memory.h"
#include "compiler.h"

/* This pass promotes reads from UBOs to register-mapped uniforms.  This saves
 * both instructions and work register pressure, but it reduces the work
 * registers available, requiring a balance.
 *
 * We use a heuristic to determine the ideal count, implemented by
 * mir_work_heuristic, which returns the ideal number of work registers.
 */

static bool
mir_is_ubo(midgard_instruction *ins)
{
   return (ins->type == TAG_LOAD_STORE_4) && (OP_IS_UBO_READ(ins->op));
}

static bool
mir_is_direct_aligned_ubo(midgard_instruction *ins)
{
   return mir_is_ubo(ins) && !(ins->constants.u32[0] & 0xF) &&
          (ins->src[1] == ~0) && (ins->src[2] == ~0);
}

/* Represents use data for a single UBO */

#define MAX_UBO_QWORDS (65536 / 16)

struct mir_ubo_block {
   BITSET_DECLARE(uses, MAX_UBO_QWORDS);
   BITSET_DECLARE(pushed, MAX_UBO_QWORDS);
};

struct mir_ubo_analysis {
   /* Per block analysis */
   unsigned nr_blocks;
   struct mir_ubo_block *blocks;
};

static struct mir_ubo_analysis
mir_analyze_ranges(compiler_context *ctx)
{
   struct mir_ubo_analysis res = {
      .nr_blocks = ctx->nir->info.num_ubos + 1,
   };

   res.blocks = calloc(res.nr_blocks, sizeof(struct mir_ubo_block));

   mir_foreach_instr_global(ctx, ins) {
      if (!mir_is_direct_aligned_ubo(ins))
         continue;

      unsigned ubo = midgard_unpack_ubo_index_imm(ins->load_store);
      unsigned offset = ins->constants.u32[0] / 16;

      assert(ubo < res.nr_blocks);

      if (offset < MAX_UBO_QWORDS)
         BITSET_SET(res.blocks[ubo].uses, offset);
   }

   return res;
}

/* Select UBO words to push. A sophisticated implementation would consider the
 * number of uses and perhaps the control flow to estimate benefit. This is not
 * sophisticated. Select from the last UBO first to prioritize sysvals. */

static void
mir_pick_ubo(struct panfrost_ubo_push *push, struct mir_ubo_analysis *analysis,
             unsigned max_qwords)
{
   unsigned max_words = MIN2(PAN_MAX_PUSH, max_qwords * 4);

   for (signed ubo = analysis->nr_blocks - 1; ubo >= 0; --ubo) {
      struct mir_ubo_block *block = &analysis->blocks[ubo];

      unsigned vec4;
      BITSET_FOREACH_SET(vec4, block->uses, MAX_UBO_QWORDS) {
         /* Don't push more than possible */
         if (push->count > max_words - 4)
            return;

         for (unsigned offs = 0; offs < 4; ++offs) {
            struct panfrost_ubo_word word = {
               .ubo = ubo,
               .offset = (vec4 * 16) + (offs * 4),
            };

            push->words[push->count++] = word;
         }

         /* Mark it as pushed so we can rewrite */
         BITSET_SET(block->pushed, vec4);
      }
   }
}

#if 0
static void
mir_dump_ubo_analysis(struct mir_ubo_analysis *res)
{
        printf("%u blocks\n", res->nr_blocks);

        for (unsigned i = 0; i < res->nr_blocks; ++i) {
                BITSET_WORD *uses = res->blocks[i].uses;
                BITSET_WORD *push = res->blocks[i].pushed;

                unsigned last = BITSET_LAST_BIT_SIZED(uses, BITSET_WORDS(MAX_UBO_QWORDS));

                printf("\t");

                for (unsigned j = 0; j < last; ++j) {
                        bool used = BITSET_TEST(uses, j);
                        bool pushed = BITSET_TEST(push, j);
                        assert(used || !pushed);

                        putchar(pushed ? '*' : used ? '-' : '_');
                }

                printf("\n");
        }
}
#endif

static unsigned
mir_promoteable_uniform_count(struct mir_ubo_analysis *analysis)
{
   unsigned count = 0;

   for (unsigned i = 0; i < analysis->nr_blocks; ++i) {
      BITSET_WORD *uses = analysis->blocks[i].uses;

      for (unsigned w = 0; w < BITSET_WORDS(MAX_UBO_QWORDS); ++w)
         count += util_bitcount(uses[w]);
   }

   return count;
}

static unsigned
mir_count_live(uint16_t *live, unsigned temp_count)
{
   unsigned count = 0;

   for (unsigned i = 0; i < temp_count; ++i)
      count += util_bitcount(live[i]);

   return count;
}

static unsigned
mir_estimate_pressure(compiler_context *ctx)
{
   mir_invalidate_liveness(ctx);
   mir_compute_liveness(ctx);

   unsigned max_live = 0;

   mir_foreach_block(ctx, _block) {
      midgard_block *block = (midgard_block *)_block;
      uint16_t *live =
         mem_dup(block->base.live_out, ctx->temp_count * sizeof(uint16_t));

      mir_foreach_instr_in_block_rev(block, ins) {
         unsigned count = mir_count_live(live, ctx->temp_count);
         max_live = MAX2(max_live, count);
         mir_liveness_ins_update(live, ins, ctx->temp_count);
      }

      free(live);
   }

   return DIV_ROUND_UP(max_live, 16);
}

static unsigned
mir_work_heuristic(compiler_context *ctx, struct mir_ubo_analysis *analysis)
{
   unsigned uniform_count = mir_promoteable_uniform_count(analysis);

   /* If there are 8 or fewer uniforms, it doesn't matter what we do, so
    * allow as many work registers as needed */

   if (uniform_count <= 8)
      return 16;

   /* Otherwise, estimate the register pressure */

   unsigned pressure = mir_estimate_pressure(ctx);

   /* Prioritize not spilling above all else. The relation between the
    * pressure estimate and the actual register pressure is a little
    * murkier than we might like (due to scheduling, pipeline registers,
    * failure to pack vector registers, load/store registers, texture
    * registers...), hence why this is a heuristic parameter */

   if (pressure > 6)
      return 16;

   /* If there's no chance of spilling, prioritize UBOs and thread count */

   return 8;
}

/* Bitset of indices that will be used as a special register -- inputs to a
 * non-ALU op. We precompute this set so that testing is efficient, otherwise
 * we end up O(mn) behaviour for n instructions and m uniform reads */

static BITSET_WORD *
mir_special_indices(compiler_context *ctx)
{
   mir_compute_temp_count(ctx);
   BITSET_WORD *bset =
      calloc(BITSET_WORDS(ctx->temp_count), sizeof(BITSET_WORD));

   mir_foreach_instr_global(ctx, ins) {
      /* Look for special instructions */
      bool is_ldst = ins->type == TAG_LOAD_STORE_4;
      bool is_tex = ins->type == TAG_TEXTURE_4;
      bool is_writeout = ins->compact_branch && ins->writeout;

      if (!(is_ldst || is_tex || is_writeout))
         continue;

      /* Anything read by a special instruction is itself special */
      mir_foreach_src(ins, i) {
         unsigned idx = ins->src[i];

         if (idx < ctx->temp_count)
            BITSET_SET(bset, idx);
      }
   }

   return bset;
}

void
midgard_promote_uniforms(compiler_context *ctx)
{
   if (ctx->inputs->no_ubo_to_push) {
      /* If nothing is pushed, all UBOs need to be uploaded
       * conventionally */
      ctx->ubo_mask = ~0;
      return;
   }

   struct mir_ubo_analysis analysis = mir_analyze_ranges(ctx);

   unsigned work_count = mir_work_heuristic(ctx, &analysis);
   unsigned promoted_count = 24 - work_count;

   /* Ensure we are 16 byte aligned to avoid underallocations */
   mir_pick_ubo(&ctx->info->push, &analysis, promoted_count);
   ctx->info->push.count = ALIGN_POT(ctx->info->push.count, 4);

   /* First, figure out special indices a priori so we don't recompute a lot */
   BITSET_WORD *special = mir_special_indices(ctx);

   ctx->ubo_mask = 0;

   mir_foreach_instr_global_safe(ctx, ins) {
      if (!mir_is_ubo(ins))
         continue;

      unsigned ubo = midgard_unpack_ubo_index_imm(ins->load_store);
      unsigned qword = ins->constants.u32[0] / 16;

      if (!mir_is_direct_aligned_ubo(ins)) {
         if (ins->src[1] == ~0)
            ctx->ubo_mask |= BITSET_BIT(ubo);
         else
            ctx->ubo_mask = ~0;

         continue;
      }

      /* Check if we decided to push this */
      assert(ubo < analysis.nr_blocks);
      if (!BITSET_TEST(analysis.blocks[ubo].pushed, qword)) {
         ctx->ubo_mask |= BITSET_BIT(ubo);
         continue;
      }

      /* Find where we pushed to, TODO: unaligned pushes to pack */
      unsigned base = pan_lookup_pushed_ubo(&ctx->info->push, ubo, qword * 16);
      assert((base & 0x3) == 0);

      unsigned address = base / 4;
      unsigned uniform_reg = 23 - address;

      /* Should've taken into account when pushing */
      assert(address < promoted_count);
      unsigned promoted = SSA_FIXED_REGISTER(uniform_reg);

      /* We do need the move for safety for a non-SSA dest, or if
       * we're being fed into a special class */

      bool needs_move = ins->dest & PAN_IS_REG || ins->dest == ctx->blend_src1;

      if (ins->dest < ctx->temp_count)
         needs_move |= BITSET_TEST(special, ins->dest);

      if (needs_move) {
         unsigned type_size = nir_alu_type_get_type_size(ins->dest_type);
         midgard_instruction mov = v_mov(promoted, ins->dest);
         mov.dest_type = nir_type_uint | type_size;
         mov.src_types[1] = mov.dest_type;

         uint16_t rounded = mir_round_bytemask_up(mir_bytemask(ins), type_size);
         mir_set_bytemask(&mov, rounded);
         mir_insert_instruction_before(ctx, ins, mov);
      } else {
         mir_rewrite_index_src(ctx, ins->dest, promoted);
      }

      mir_remove_instruction(ins);
   }

   free(special);
   free(analysis.blocks);
}
