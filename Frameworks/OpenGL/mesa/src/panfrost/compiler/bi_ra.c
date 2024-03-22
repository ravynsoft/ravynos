/*
 * Copyright (C) 2020 Collabora Ltd.
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
 *      Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "util/u_memory.h"
#include "bi_builder.h"
#include "compiler.h"
#include "nodearray.h"

struct lcra_state {
   unsigned node_count;
   uint64_t *affinity;

   /* Linear constraints imposed. For each node there there is a
    * 'nodearray' structure, which changes between a sparse and dense
    * array depending on the number of elements.
    *
    * Each element is itself a bit field denoting whether (c_j - c_i) bias
    * is present or not, including negative biases.
    *
    * We support up to 8 components so the bias is in range
    * [-7, 7] encoded by a 16-bit field
    */
   nodearray *linear;

   /* Before solving, forced registers; after solving, solutions. */
   unsigned *solutions;

   /** Node which caused register allocation to fail */
   unsigned spill_node;
};

/* This module is an implementation of "Linearly Constrained
 * Register Allocation". The paper is available in PDF form
 * (https://people.collabora.com/~alyssa/LCRA.pdf) as well as Markdown+LaTeX
 * (https://gitlab.freedesktop.org/alyssa/lcra/blob/master/LCRA.md)
 */

static struct lcra_state *
lcra_alloc_equations(unsigned node_count)
{
   struct lcra_state *l = calloc(1, sizeof(*l));

   l->node_count = node_count;

   l->linear = calloc(sizeof(l->linear[0]), node_count);
   l->solutions = calloc(sizeof(l->solutions[0]), node_count);
   l->affinity = calloc(sizeof(l->affinity[0]), node_count);

   memset(l->solutions, ~0, sizeof(l->solutions[0]) * node_count);

   return l;
}

static void
lcra_free(struct lcra_state *l)
{
   for (unsigned i = 0; i < l->node_count; ++i)
      nodearray_reset(&l->linear[i]);

   free(l->linear);
   free(l->affinity);
   free(l->solutions);
   free(l);
}

static void
lcra_add_node_interference(struct lcra_state *l, unsigned i, unsigned cmask_i,
                           unsigned j, unsigned cmask_j)
{
   if (i == j)
      return;

   nodearray_value constraint_fw = 0;
   nodearray_value constraint_bw = 0;

   /* The constraint bits are reversed from lcra.c so that register
    * allocation can be done in parallel for every possible solution,
    * with lower-order bits representing smaller registers. */

   for (unsigned D = 0; D < 8; ++D) {
      if (cmask_i & (cmask_j << D)) {
         constraint_fw |= (1 << (7 + D));
         constraint_bw |= (1 << (7 - D));
      }

      if (cmask_i & (cmask_j >> D)) {
         constraint_bw |= (1 << (7 + D));
         constraint_fw |= (1 << (7 - D));
      }
   }

   /* Use dense arrays after adding 256 elements */
   nodearray_orr(&l->linear[j], i, constraint_fw, 256, l->node_count);
   nodearray_orr(&l->linear[i], j, constraint_bw, 256, l->node_count);
}

static bool
lcra_test_linear(struct lcra_state *l, unsigned *solutions, unsigned i)
{
   signed constant = solutions[i];

   if (nodearray_is_sparse(&l->linear[i])) {
      nodearray_sparse_foreach(&l->linear[i], elem) {
         unsigned j = nodearray_sparse_key(elem);
         nodearray_value constraint = nodearray_sparse_value(elem);

         if (solutions[j] == ~0)
            continue;

         signed lhs = constant - solutions[j];

         if (lhs < -7 || lhs > 7)
            continue;

         if (constraint & (1 << (lhs + 7)))
            return false;
      }

      return true;
   }

   nodearray_value *row = l->linear[i].dense;

   for (unsigned j = 0; j < l->node_count; ++j) {
      if (solutions[j] == ~0)
         continue;

      signed lhs = constant - solutions[j];

      if (lhs < -7 || lhs > 7)
         continue;

      if (row[j] & (1 << (lhs + 7)))
         return false;
   }

   return true;
}

static bool
lcra_solve(struct lcra_state *l)
{
   for (unsigned step = 0; step < l->node_count; ++step) {
      if (l->solutions[step] != ~0)
         continue;
      if (l->affinity[step] == 0)
         continue;

      bool succ = false;

      u_foreach_bit64(r, l->affinity[step]) {
         l->solutions[step] = r;

         if (lcra_test_linear(l, l->solutions, step)) {
            succ = true;
            break;
         }
      }

      /* Out of registers - prepare to spill */
      if (!succ) {
         l->spill_node = step;
         return false;
      }
   }

   return true;
}

/* Register spilling is implemented with a cost-benefit system. Costs are set
 * by the user. Benefits are calculated from the constraints. */

static unsigned
lcra_count_constraints(struct lcra_state *l, unsigned i)
{
   unsigned count = 0;
   nodearray *constraints = &l->linear[i];

   if (nodearray_is_sparse(constraints)) {
      nodearray_sparse_foreach(constraints, elem)
         count += util_bitcount(nodearray_sparse_value(elem));
   } else {
      nodearray_dense_foreach_64(constraints, elem)
         count += util_bitcount64(*elem);
   }

   return count;
}

/* Liveness analysis is a backwards-may dataflow analysis pass. Within a block,
 * we compute live_out from live_in. The intrablock pass is linear-time. It
 * returns whether progress was made. */

static void
bi_liveness_ins_update_ra(uint8_t *live, bi_instr *ins)
{
   /* live_in[s] = GEN[s] + (live_out[s] - KILL[s]) */

   bi_foreach_dest(ins, d) {
      live[ins->dest[d].value] &= ~bi_writemask(ins, d);
   }

   bi_foreach_ssa_src(ins, src) {
      unsigned count = bi_count_read_registers(ins, src);
      unsigned rmask = BITFIELD_MASK(count);

      live[ins->src[src].value] |= (rmask << ins->src[src].offset);
   }
}

static bool
liveness_block_update(bi_block *blk, unsigned temp_count)
{
   bool progress = false;

   /* live_out[s] = sum { p in succ[s] } ( live_in[p] ) */
   bi_foreach_successor(blk, succ) {
      for (unsigned i = 0; i < temp_count; ++i)
         blk->live_out[i] |= succ->live_in[i];
   }

   uint8_t *live = ralloc_array(blk, uint8_t, temp_count);
   memcpy(live, blk->live_out, temp_count);

   bi_foreach_instr_in_block_rev(blk, ins)
      bi_liveness_ins_update_ra(live, ins);

   /* To figure out progress, diff live_in */

   for (unsigned i = 0; (i < temp_count) && !progress; ++i)
      progress |= (blk->live_in[i] != live[i]);

   ralloc_free(blk->live_in);
   blk->live_in = live;

   return progress;
}

/* Globally, liveness analysis uses a fixed-point algorithm based on a
 * worklist. We initialize a work list with the exit block. We iterate the work
 * list to compute live_in from live_out for each block on the work list,
 * adding the predecessors of the block to the work list if we made progress.
 */

static void
bi_compute_liveness_ra(bi_context *ctx)
{
   u_worklist worklist;
   bi_worklist_init(ctx, &worklist);

   bi_foreach_block(ctx, block) {
      if (block->live_in)
         ralloc_free(block->live_in);

      if (block->live_out)
         ralloc_free(block->live_out);

      block->live_in = rzalloc_array(block, uint8_t, ctx->ssa_alloc);
      block->live_out = rzalloc_array(block, uint8_t, ctx->ssa_alloc);

      bi_worklist_push_tail(&worklist, block);
   }

   while (!u_worklist_is_empty(&worklist)) {
      /* Pop off in reverse order since liveness is backwards */
      bi_block *blk = bi_worklist_pop_tail(&worklist);

      /* Update liveness information. If we made progress, we need to
       * reprocess the predecessors
       */
      if (liveness_block_update(blk, ctx->ssa_alloc)) {
         bi_foreach_predecessor(blk, pred)
            bi_worklist_push_head(&worklist, *pred);
      }
   }

   u_worklist_fini(&worklist);
}

/* Construct an affinity mask such that the vector with `count` elements does
 * not intersect any of the registers in the bitset `clobber`. In other words,
 * an allocated register r needs to satisfy for each i < count: a + i != b.
 * Equivalently that's a != b - i, so we need a \ne { b - i : i < n }. For the
 * entire clobber set B, we need a \ne union b \in B { b - i : i < n }, where
 * that union is the desired clobber set. That may be written equivalently as
 * the union over i < n of (B - i), where subtraction is defined elementwise
 * and corresponds to a shift of the entire bitset.
 *
 * EVEN_BITS_MASK is an affinity mask for aligned register pairs. Interpreted
 * as a bit set, it is { x : 0 <= x < 64 if x is even }
 */

#define EVEN_BITS_MASK (0x5555555555555555ull)

static uint64_t
bi_make_affinity(uint64_t clobber, unsigned count, bool split_file)
{
   uint64_t clobbered = 0;

   for (unsigned i = 0; i < count; ++i)
      clobbered |= (clobber >> i);

   /* Don't allocate past the end of the register file */
   if (count > 1) {
      unsigned excess = count - 1;
      uint64_t mask = BITFIELD_MASK(excess);
      clobbered |= mask << (64 - excess);

      if (split_file)
         clobbered |= mask << (16 - excess);
   }

   /* Don't allocate the middle if we split out the middle */
   if (split_file)
      clobbered |= BITFIELD64_MASK(32) << 16;

   /* We can use a register iff it's not clobberred */
   return ~clobbered;
}

static void
bi_mark_interference(bi_block *block, struct lcra_state *l, uint8_t *live,
                     uint64_t preload_live, unsigned node_count, bool is_blend,
                     bool split_file, bool aligned_sr)
{
   bi_foreach_instr_in_block_rev(block, ins) {
      /* Mark all registers live after the instruction as
       * interfering with the destination */

      bi_foreach_dest(ins, d) {
         unsigned node = ins->dest[d].value;

         /* Don't allocate to anything that's read later as a
          * preloaded register. The affinity is the intersection
          * of affinity masks for each write. Since writes have
          * offsets, but the affinity is for the whole node, we
          * need to offset the affinity opposite the write
          * offset, so we shift right. */
         unsigned count = bi_count_write_registers(ins, d);
         unsigned offset = ins->dest[d].offset;
         uint64_t affinity =
            bi_make_affinity(preload_live, count, split_file) >> offset;
         /* Valhall needs >= 64-bit staging writes to be pair-aligned */
         if (aligned_sr && (count >= 2 || offset))
            affinity &= EVEN_BITS_MASK;

         l->affinity[node] &= affinity;

         for (unsigned i = 0; i < node_count; ++i) {
            uint8_t r = live[i];

            /* Nodes only interfere if they occupy
             * /different values/ at the same time
             * (Boissinot). In particular, sources of
             * moves do not interfere with their
             * destinations. This enables a limited form of
             * coalescing.
             */
            if (ins->op == BI_OPCODE_MOV_I32 && bi_is_ssa(ins->src[0]) &&
                i == ins->src[0].value) {

               r &= ~BITFIELD_BIT(ins->src[0].offset);
            }

            if (r) {
               lcra_add_node_interference(l, node, bi_writemask(ins, d), i, r);
            }
         }

         unsigned node_first = ins->dest[0].value;
         if (d == 1) {
            lcra_add_node_interference(l, node, bi_writemask(ins, 1),
                                       node_first, bi_writemask(ins, 0));
         }
      }

      /* Valhall needs >= 64-bit reads to be pair-aligned */
      if (aligned_sr) {
         bi_foreach_ssa_src(ins, s) {
            if (bi_count_read_registers(ins, s) >= 2)
               l->affinity[ins->src[s].value] &= EVEN_BITS_MASK;
         }
      }

      if (!is_blend && ins->op == BI_OPCODE_BLEND) {
         /* Blend shaders might clobber r0-r15, r48. */
         uint64_t clobber = BITFIELD64_MASK(16) | BITFIELD64_BIT(48);

         for (unsigned i = 0; i < node_count; ++i) {
            if (live[i])
               l->affinity[i] &= ~clobber;
         }
      }

      /* Update live_in */
      preload_live = bi_postra_liveness_ins(preload_live, ins);
      bi_liveness_ins_update_ra(live, ins);
   }

   block->reg_live_in = preload_live;
}

static void
bi_compute_interference(bi_context *ctx, struct lcra_state *l, bool full_regs)
{
   bi_compute_liveness_ra(ctx);
   bi_postra_liveness(ctx);

   bi_foreach_block_rev(ctx, blk) {
      uint8_t *live = mem_dup(blk->live_out, ctx->ssa_alloc);

      bi_mark_interference(blk, l, live, blk->reg_live_out, ctx->ssa_alloc,
                           ctx->inputs->is_blend, !full_regs, ctx->arch >= 9);

      free(live);
   }
}

static struct lcra_state *
bi_allocate_registers(bi_context *ctx, bool *success, bool full_regs)
{
   struct lcra_state *l = lcra_alloc_equations(ctx->ssa_alloc);

   /* Blend shaders are restricted to R0-R15. Other shaders at full
    * occupancy also can access R48-R63. At half occupancy they can access
    * the whole file. */

   uint64_t default_affinity =
      ctx->inputs->is_blend ? BITFIELD64_MASK(16)
      : full_regs           ? BITFIELD64_MASK(64)
                  : (BITFIELD64_MASK(16) | (BITFIELD64_MASK(16) << 48));

   /* To test spilling, mimic a small register file */
   if (bifrost_debug & BIFROST_DBG_SPILL && !ctx->inputs->is_blend)
      default_affinity &= BITFIELD64_MASK(48) << 8;

   bi_foreach_instr_global(ctx, ins) {
      bi_foreach_dest(ins, d)
         l->affinity[ins->dest[d].value] = default_affinity;

      /* Blend shaders expect the src colour to be in r0-r3 */
      if (ins->op == BI_OPCODE_BLEND && !ctx->inputs->is_blend) {
         assert(bi_is_ssa(ins->src[0]));
         l->solutions[ins->src[0].value] = 0;

         /* Dual source blend input in r4-r7 */
         if (bi_is_ssa(ins->src[4]))
            l->solutions[ins->src[4].value] = 4;

         /* Writes to R48 */
         if (!bi_is_null(ins->dest[0]))
            l->solutions[ins->dest[0].value] = 48;
      }

      /* Coverage mask writes stay in R60 */
      if ((ins->op == BI_OPCODE_ATEST || ins->op == BI_OPCODE_ZS_EMIT) &&
          !bi_is_null(ins->dest[0])) {
         l->solutions[ins->dest[0].value] = 60;
      }

      /* Experimentally, it seems coverage masks inputs to ATEST must
       * be in R60. Otherwise coverage mask writes do not work with
       * early-ZS with pixel-frequency-shading (this combination of
       * settings is legal if depth/stencil writes are disabled).
       */
      if (ins->op == BI_OPCODE_ATEST) {
         assert(bi_is_ssa(ins->src[0]));
         l->solutions[ins->src[0].value] = 60;
      }
   }

   bi_compute_interference(ctx, l, full_regs);

   /* Coalesce register moves if we're allowed. We need to be careful due
    * to the restricted affinity induced by the blend shader ABI.
    */
   bi_foreach_instr_global(ctx, I) {
      if (I->op != BI_OPCODE_MOV_I32)
         continue;
      if (I->src[0].type != BI_INDEX_REGISTER)
         continue;

      unsigned reg = I->src[0].value;
      unsigned node = I->dest[0].value;

      if (l->solutions[node] != ~0)
         continue;

      uint64_t affinity = l->affinity[node];

      if (ctx->inputs->is_blend) {
         /* We're allowed to coalesce the moves to these */
         affinity |= BITFIELD64_BIT(48);
         affinity |= BITFIELD64_BIT(60);
      }

      /* Try to coalesce */
      if (affinity & BITFIELD64_BIT(reg)) {
         l->solutions[node] = reg;

         if (!lcra_test_linear(l, l->solutions, node))
            l->solutions[node] = ~0;
      }
   }

   *success = lcra_solve(l);

   return l;
}

static bi_index
bi_reg_from_index(bi_context *ctx, struct lcra_state *l, bi_index index)
{
   /* Offsets can only be applied when we register allocated an index, or
    * alternatively for FAU's encoding */

   ASSERTED bool is_offset = (index.offset > 0) && (index.type != BI_INDEX_FAU);

   /* Did we run RA for this index at all */
   if (!bi_is_ssa(index)) {
      assert(!is_offset);
      return index;
   }

   /* LCRA didn't bother solving this index (how lazy!) */
   signed solution = l->solutions[index.value];
   if (solution < 0) {
      assert(!is_offset);
      return index;
   }

   /* todo: do we want to compose with the subword swizzle? */
   bi_index new_index = bi_register(solution + index.offset);
   new_index.swizzle = index.swizzle;
   new_index.abs = index.abs;
   new_index.neg = index.neg;
   return new_index;
}

/* Dual texture instructions write to two sets of staging registers, modeled as
 * two destinations in the IR. The first set is communicated with the usual
 * staging register mechanism. The second set is encoded in the texture
 * operation descriptor. This is quite unusual, and requires the following late
 * fixup.
 */
static void
bi_fixup_dual_tex_register(bi_instr *I)
{
   assert(I->dest[1].type == BI_INDEX_REGISTER);
   assert(I->src[3].type == BI_INDEX_CONSTANT);

   struct bifrost_dual_texture_operation desc = {
      .secondary_register = I->dest[1].value,
   };

   I->src[3].value |= bi_dual_tex_as_u32(desc);
}

static void
bi_install_registers(bi_context *ctx, struct lcra_state *l)
{
   bi_foreach_instr_global(ctx, ins) {
      bi_foreach_dest(ins, d)
         ins->dest[d] = bi_reg_from_index(ctx, l, ins->dest[d]);

      bi_foreach_src(ins, s)
         ins->src[s] = bi_reg_from_index(ctx, l, ins->src[s]);

      if (ins->op == BI_OPCODE_TEXC_DUAL)
         bi_fixup_dual_tex_register(ins);
   }
}

static void
bi_rewrite_index_src_single(bi_instr *ins, bi_index old, bi_index new)
{
   bi_foreach_src(ins, i) {
      if (bi_is_equiv(ins->src[i], old)) {
         ins->src[i].type = new.type;
         ins->src[i].value = new.value;
      }
   }
}

/* If register allocation fails, find the best spill node */

static signed
bi_choose_spill_node(bi_context *ctx, struct lcra_state *l)
{
   /* Pick a node satisfying bi_spill_register's preconditions */
   BITSET_WORD *no_spill =
      calloc(sizeof(BITSET_WORD), BITSET_WORDS(l->node_count));

   bi_foreach_instr_global(ctx, ins) {
      bi_foreach_dest(ins, d) {
         /* Don't allow spilling coverage mask writes because the
          * register preload logic assumes it will stay in R60.
          * This could be optimized.
          */
         if (ins->no_spill || ins->op == BI_OPCODE_ATEST ||
             ins->op == BI_OPCODE_ZS_EMIT ||
             (ins->op == BI_OPCODE_MOV_I32 &&
              ins->src[0].type == BI_INDEX_REGISTER &&
              ins->src[0].value == 60)) {
            BITSET_SET(no_spill, ins->dest[d].value);
         }
      }
   }

   unsigned best_benefit = 0.0;
   signed best_node = -1;

   if (nodearray_is_sparse(&l->linear[l->spill_node])) {
      nodearray_sparse_foreach(&l->linear[l->spill_node], elem) {
         unsigned i = nodearray_sparse_key(elem);
         unsigned constraint = nodearray_sparse_value(elem);

         /* Only spill nodes that interfere with the node failing
          * register allocation. It's pointless to spill anything else */
         if (!constraint)
            continue;

         if (BITSET_TEST(no_spill, i))
            continue;

         unsigned benefit = lcra_count_constraints(l, i);

         if (benefit > best_benefit) {
            best_benefit = benefit;
            best_node = i;
         }
      }
   } else {
      nodearray_value *row = l->linear[l->spill_node].dense;

      for (unsigned i = 0; i < l->node_count; ++i) {
         /* Only spill nodes that interfere with the node failing
          * register allocation. It's pointless to spill anything else */
         if (!row[i])
            continue;

         if (BITSET_TEST(no_spill, i))
            continue;

         unsigned benefit = lcra_count_constraints(l, i);

         if (benefit > best_benefit) {
            best_benefit = benefit;
            best_node = i;
         }
      }
   }

   free(no_spill);
   return best_node;
}

static unsigned
bi_count_read_index(bi_instr *I, bi_index index)
{
   unsigned max = 0;

   bi_foreach_src(I, s) {
      if (bi_is_equiv(I->src[s], index)) {
         unsigned count = bi_count_read_registers(I, s);
         max = MAX2(max, count + I->src[s].offset);
      }
   }

   return max;
}

/*
 * Wrappers to emit loads/stores to thread-local storage in an appropriate way
 * for the target, so the spill/fill code becomes architecture-independent.
 */

static bi_index
bi_tls_ptr(bool hi)
{
   return bi_fau(BIR_FAU_TLS_PTR, hi);
}

static bi_instr *
bi_load_tl(bi_builder *b, unsigned bits, bi_index src, unsigned offset)
{
   if (b->shader->arch >= 9) {
      return bi_load_to(b, bits, src, bi_tls_ptr(false), bi_tls_ptr(true),
                        BI_SEG_TL, offset);
   } else {
      return bi_load_to(b, bits, src, bi_imm_u32(offset), bi_zero(), BI_SEG_TL,
                        0);
   }
}

static void
bi_store_tl(bi_builder *b, unsigned bits, bi_index src, unsigned offset)
{
   if (b->shader->arch >= 9) {
      bi_store(b, bits, src, bi_tls_ptr(false), bi_tls_ptr(true), BI_SEG_TL,
               offset);
   } else {
      bi_store(b, bits, src, bi_imm_u32(offset), bi_zero(), BI_SEG_TL, 0);
   }
}

/* Once we've chosen a spill node, spill it and returns bytes spilled */

static unsigned
bi_spill_register(bi_context *ctx, bi_index index, uint32_t offset)
{
   bi_builder b = {.shader = ctx};
   unsigned channels = 0;

   /* Spill after every store, fill before every load */
   bi_foreach_instr_global_safe(ctx, I) {
      bi_foreach_dest(I, d) {
         if (!bi_is_equiv(I->dest[d], index))
            continue;

         unsigned extra = I->dest[d].offset;
         bi_index tmp = bi_temp(ctx);

         I->dest[d] = bi_replace_index(I->dest[d], tmp);
         I->no_spill = true;

         unsigned count = bi_count_write_registers(I, d);
         unsigned bits = count * 32;

         b.cursor = bi_after_instr(I);
         bi_store_tl(&b, bits, tmp, offset + 4 * extra);

         ctx->spills++;
         channels = MAX2(channels, extra + count);
      }

      if (bi_has_arg(I, index)) {
         b.cursor = bi_before_instr(I);
         bi_index tmp = bi_temp(ctx);

         unsigned bits = bi_count_read_index(I, index) * 32;
         bi_rewrite_index_src_single(I, index, tmp);

         bi_instr *ld = bi_load_tl(&b, bits, tmp, offset);
         ld->no_spill = true;
         ctx->fills++;
      }
   }

   return (channels * 4);
}

/*
 * For transition, lower collects and splits before RA, rather than after RA.
 * LCRA knows how to deal with offsets (broken SSA), but not how to coalesce
 * these vector moves.
 */
static void
bi_lower_vector(bi_context *ctx, unsigned first_reg)
{
   bi_index *remap = calloc(ctx->ssa_alloc, sizeof(bi_index));

   bi_foreach_instr_global_safe(ctx, I) {
      bi_builder b = bi_init_builder(ctx, bi_after_instr(I));

      if (I->op == BI_OPCODE_SPLIT_I32) {
         bi_index src = I->src[0];
         assert(src.offset == 0);

         bi_foreach_dest(I, i) {
            src.offset = i;
            bi_mov_i32_to(&b, I->dest[i], src);

            if (I->dest[i].value < first_reg)
               remap[I->dest[i].value] = src;
         }

         bi_remove_instruction(I);
      } else if (I->op == BI_OPCODE_COLLECT_I32) {
         bi_index dest = I->dest[0];
         assert(dest.offset == 0);
         assert(((dest.value < first_reg) || I->nr_srcs == 1) &&
                "nir_lower_phis_to_scalar");

         bi_foreach_src(I, i) {
            if (bi_is_null(I->src[i]))
               continue;

            dest.offset = i;
            bi_mov_i32_to(&b, dest, I->src[i]);
         }

         bi_remove_instruction(I);
      }
   }

   bi_foreach_instr_global(ctx, I) {
      bi_foreach_ssa_src(I, s) {
         if (I->src[s].value < first_reg && !bi_is_null(remap[I->src[s].value]))
            bi_replace_src(I, s, remap[I->src[s].value]);
      }
   }

   free(remap);

   /* After generating a pile of moves, clean up */
   bi_compute_liveness_ra(ctx);

   bi_foreach_block_rev(ctx, block) {
      uint8_t *live = rzalloc_array(block, uint8_t, ctx->ssa_alloc);

      bi_foreach_successor(block, succ) {
         for (unsigned i = 0; i < ctx->ssa_alloc; ++i)
            live[i] |= succ->live_in[i];
      }

      bi_foreach_instr_in_block_safe_rev(block, ins) {
         bool all_null = true;

         bi_foreach_dest(ins, d) {
            if (live[ins->dest[d].value] & bi_writemask(ins, d))
               all_null = false;
         }

         if (all_null && !bi_side_effects(ins))
            bi_remove_instruction(ins);
         else
            bi_liveness_ins_update_ra(live, ins);
      }

      ralloc_free(block->live_in);
      block->live_in = live;
   }
}

/*
 * Check if the instruction requires a "tied" operand. Such instructions MUST
 * allocate their source and destination to the same register. This is a
 * constraint on RA, and may require extra moves.
 *
 * In particular, this is the case for Bifrost instructions that both read and
 * write with the staging register mechanism.
 */
static bool
bi_is_tied(const bi_instr *I)
{
   return (I->op == BI_OPCODE_TEXC || I->op == BI_OPCODE_TEXC_DUAL ||
           I->op == BI_OPCODE_ATOM_RETURN_I32 || I->op == BI_OPCODE_AXCHG_I32 ||
           I->op == BI_OPCODE_ACMPXCHG_I32) &&
          !bi_is_null(I->src[0]);
}

/*
 * For transition, coalesce tied operands together, as LCRA knows how to handle
 * non-SSA operands but doesn't know about tied operands.
 *
 * This breaks the SSA form of the program, but that doesn't matter for LCRA.
 */
static void
bi_coalesce_tied(bi_context *ctx)
{
   bi_foreach_instr_global(ctx, I) {
      if (!bi_is_tied(I))
         continue;

      bi_builder b = bi_init_builder(ctx, bi_before_instr(I));
      unsigned n = bi_count_read_registers(I, 0);

      for (unsigned i = 0; i < n; ++i) {
         bi_index dst = I->dest[0], src = I->src[0];

         assert(dst.offset == 0 && src.offset == 0);
         dst.offset = src.offset = i;

         bi_mov_i32_to(&b, dst, src);
      }

      bi_replace_src(I, 0, I->dest[0]);
   }
}

static unsigned
find_or_allocate_temp(unsigned *map, unsigned value, unsigned *alloc)
{
   if (!map[value])
      map[value] = ++(*alloc);

   assert(map[value]);
   return map[value] - 1;
}

/* Reassigns numbering to get rid of gaps in the indices and to prioritize
 * smaller register classes */

static void
squeeze_index(bi_context *ctx)
{
   unsigned *map = rzalloc_array(ctx, unsigned, ctx->ssa_alloc);
   ctx->ssa_alloc = 0;

   bi_foreach_instr_global(ctx, I) {
      bi_foreach_dest(I, d)
         I->dest[d].value =
            find_or_allocate_temp(map, I->dest[d].value, &ctx->ssa_alloc);

      bi_foreach_ssa_src(I, s)
         I->src[s].value =
            find_or_allocate_temp(map, I->src[s].value, &ctx->ssa_alloc);
   }

   ralloc_free(map);
}

/*
 * Brainless out-of-SSA pass. The eventual goal is to go out-of-SSA after RA and
 * coalesce implicitly with biased colouring in a tree scan allocator. For now,
 * this should be good enough for LCRA.
 */
static unsigned
bi_out_of_ssa(bi_context *ctx)
{
   bi_index zero = bi_fau(BIR_FAU_IMMEDIATE | 0, false);
   unsigned first_reg = ctx->ssa_alloc;

   /* Trivially lower phis */
   bi_foreach_block(ctx, block) {
      bi_foreach_instr_in_block_safe(block, I) {
         if (I->op != BI_OPCODE_PHI)
            break;

         /* Assign a register for the phi */
         bi_index reg = bi_temp(ctx);
         assert(reg.value >= first_reg);

         /* Lower to a move in each predecessor. The destinations
          * cannot interfere so these can be sequentialized
          * in arbitrary order.
          */
         bi_foreach_predecessor(block, pred) {
            bi_builder b = bi_init_builder(ctx, bi_after_block_logical(*pred));
            unsigned i = bi_predecessor_index(block, *pred);

            assert(!I->src[i].abs);
            assert(!I->src[i].neg);
            assert(I->src[i].swizzle == BI_SWIZZLE_H01);

            /* MOV of immediate needs lowering on Valhall */
            if (ctx->arch >= 9 && I->src[i].type == BI_INDEX_CONSTANT)
               bi_iadd_imm_i32_to(&b, reg, zero, I->src[i].value);
            else
               bi_mov_i32_to(&b, reg, I->src[i]);
         }

         /* Replace the phi with a move */
         bi_builder b = bi_init_builder(ctx, bi_before_instr(I));
         bi_mov_i32_to(&b, I->dest[0], reg);
         bi_remove_instruction(I);

         /* Propagate that move within the block. The destination
          * is SSA and the source is not written in this block,
          * so this is legal. The move itself will be DCE'd if
          * possible in the next pass.
          */
         bi_foreach_instr_in_block_rev(block, prop) {
            if (prop->op == BI_OPCODE_PHI)
               break;

            bi_foreach_src(prop, s) {
               if (bi_is_equiv(prop->src[s], I->dest[0])) {
                  bi_replace_src(prop, s, reg);
               }
            }
         }
      }
   }

   /* Try to locally propagate the moves we created. We need to be extra
    * careful because we're not in SSA at this point, as such this
    * algorithm is quadratic. This will go away when we go out of SSA after
    * RA.
    */
   BITSET_WORD *used =
      calloc(sizeof(BITSET_WORD), BITSET_WORDS(ctx->ssa_alloc));
   BITSET_WORD *multiple_uses =
      calloc(sizeof(BITSET_WORD), BITSET_WORDS(ctx->ssa_alloc));

   bi_foreach_instr_global(ctx, I) {
      bi_foreach_ssa_src(I, s) {
         if (BITSET_TEST(used, I->src[s].value))
            BITSET_SET(multiple_uses, I->src[s].value);
         else
            BITSET_SET(used, I->src[s].value);
      }
   }

   bi_foreach_block(ctx, block) {
      bi_foreach_instr_in_block_safe_rev(block, mov) {
         /* Match "reg = ssa" */
         if (mov->op != BI_OPCODE_MOV_I32)
            continue;
         if (mov->dest[0].type != BI_INDEX_NORMAL)
            continue;
         if (mov->dest[0].value < first_reg)
            continue;
         if (!bi_is_ssa(mov->src[0]))
            continue;
         if (mov->src[0].value >= first_reg)
            continue;
         if (BITSET_TEST(multiple_uses, mov->src[0].value))
            continue;

         bool found = false;

         /* Look locally for the write of the SSA */
         bi_foreach_instr_in_block_rev(block, I) {
            bool bail = false;

            bi_foreach_src(I, s) {
               /* Bail: write-after-read */
               if (bi_is_equiv(I->src[s], mov->dest[0]))
                  bail = true;
            }

            if (bail)
               break;

            bi_foreach_dest(I, d) {
               /* Bail: write-after-write */
               if (bi_is_equiv(I->dest[d], mov->dest[0]))
                  break;

               if (!bi_is_equiv(I->dest[d], mov->src[0]))
                  continue;

               /* We found it, replace */
               I->dest[d] = bi_replace_index(I->dest[d], mov->dest[0]);
               found = true;
               break;
            }

            if (found)
               break;
         }

         if (found)
            bi_remove_instruction(mov);
      }
   }

   free(used);
   free(multiple_uses);
   return first_reg;
}

void
bi_register_allocate(bi_context *ctx)
{
   struct lcra_state *l = NULL;
   bool success = false;

   unsigned iter_count = 1000; /* max iterations */

   /* Number of bytes of memory we've spilled into */
   unsigned spill_count = ctx->info.tls_size;

   if (ctx->arch >= 9)
      va_lower_split_64bit(ctx);

   /* Lower tied operands. SSA is broken from here on. */
   unsigned first_reg = bi_out_of_ssa(ctx);
   bi_lower_vector(ctx, first_reg);
   bi_coalesce_tied(ctx);
   squeeze_index(ctx);

   /* Try with reduced register pressure to improve thread count */
   if (ctx->arch >= 7) {
      l = bi_allocate_registers(ctx, &success, false);

      if (success) {
         ctx->info.work_reg_count = 32;
      } else {
         lcra_free(l);
         l = NULL;
      }
   }

   /* Otherwise, use the register file and spill until we succeed */
   while (!success && ((iter_count--) > 0)) {
      l = bi_allocate_registers(ctx, &success, true);

      if (success) {
         ctx->info.work_reg_count = 64;
      } else {
         signed spill_node = bi_choose_spill_node(ctx, l);
         lcra_free(l);
         l = NULL;

         if (spill_node == -1)
            unreachable("Failed to choose spill node\n");

         if (ctx->inputs->is_blend)
            unreachable("Blend shaders may not spill");

         /* By default, we use packed TLS addressing on Valhall.
          * We cannot cross 16 byte boundaries with packed TLS
          * addressing. Align to ensure this doesn't happen. This
          * could be optimized a bit.
          */
         if (ctx->arch >= 9)
            spill_count = ALIGN_POT(spill_count, 16);

         spill_count +=
            bi_spill_register(ctx, bi_get_index(spill_node), spill_count);

         /* In case the spill affected an instruction with tied
          * operands, we need to fix up.
          */
         bi_coalesce_tied(ctx);
      }
   }

   assert(success);
   assert(l != NULL);

   ctx->info.tls_size = spill_count;
   bi_install_registers(ctx, l);

   lcra_free(l);
}
