/*
 * Copyright (C) 2021 Collabora, Ltd.
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

#include "bi_builder.h"
#include "compiler.h"

/* This optimization pass, intended to run once after code emission but before
 * copy propagation, analyzes direct word-aligned UBO reads and promotes a
 * subset to moves from FAU. It is the sole populator of the UBO push data
 * structure returned back to the command stream. */

static bool
bi_is_ubo(bi_instr *ins)
{
   return (bi_opcode_props[ins->op].message == BIFROST_MESSAGE_LOAD) &&
          (ins->seg == BI_SEG_UBO);
}

static bool
bi_is_direct_aligned_ubo(bi_instr *ins)
{
   return bi_is_ubo(ins) && (ins->src[0].type == BI_INDEX_CONSTANT) &&
          (ins->src[1].type == BI_INDEX_CONSTANT) &&
          ((ins->src[0].value & 0x3) == 0);
}

/* Represents use data for a single UBO */

#define MAX_UBO_WORDS (65536 / 16)

struct bi_ubo_block {
   BITSET_DECLARE(pushed, MAX_UBO_WORDS);
   uint8_t range[MAX_UBO_WORDS];
};

struct bi_ubo_analysis {
   /* Per block analysis */
   unsigned nr_blocks;
   struct bi_ubo_block *blocks;
};

static struct bi_ubo_analysis
bi_analyze_ranges(bi_context *ctx)
{
   struct bi_ubo_analysis res = {
      .nr_blocks = ctx->nir->info.num_ubos + 1,
   };

   res.blocks = calloc(res.nr_blocks, sizeof(struct bi_ubo_block));

   bi_foreach_instr_global(ctx, ins) {
      if (!bi_is_direct_aligned_ubo(ins))
         continue;

      unsigned ubo = ins->src[1].value;
      unsigned word = ins->src[0].value / 4;
      unsigned channels = bi_opcode_props[ins->op].sr_count;

      assert(ubo < res.nr_blocks);
      assert(channels > 0 && channels <= 4);

      if (word >= MAX_UBO_WORDS)
         continue;

      /* Must use max if the same base is read with different channel
       * counts, which is possible with nir_opt_shrink_vectors */
      uint8_t *range = res.blocks[ubo].range;
      range[word] = MAX2(range[word], channels);
   }

   return res;
}

/* Select UBO words to push. A sophisticated implementation would consider the
 * number of uses and perhaps the control flow to estimate benefit. This is not
 * sophisticated. Select from the last UBO first to prioritize sysvals. */

static void
bi_pick_ubo(struct panfrost_ubo_push *push, struct bi_ubo_analysis *analysis)
{
   for (signed ubo = analysis->nr_blocks - 1; ubo >= 0; --ubo) {
      struct bi_ubo_block *block = &analysis->blocks[ubo];

      for (unsigned r = 0; r < MAX_UBO_WORDS; ++r) {
         unsigned range = block->range[r];

         /* Don't push something we don't access */
         if (range == 0)
            continue;

         /* Don't push more than possible */
         if (push->count > PAN_MAX_PUSH - range)
            return;

         for (unsigned offs = 0; offs < range; ++offs) {
            struct panfrost_ubo_word word = {
               .ubo = ubo,
               .offset = (r + offs) * 4,
            };

            push->words[push->count++] = word;
         }

         /* Mark it as pushed so we can rewrite */
         BITSET_SET(block->pushed, r);
      }
   }
}

void
bi_opt_push_ubo(bi_context *ctx)
{
   struct bi_ubo_analysis analysis = bi_analyze_ranges(ctx);
   bi_pick_ubo(ctx->info.push, &analysis);

   ctx->ubo_mask = 0;

   bi_foreach_instr_global_safe(ctx, ins) {
      if (!bi_is_ubo(ins))
         continue;

      unsigned ubo = ins->src[1].value;
      unsigned offset = ins->src[0].value;

      if (!bi_is_direct_aligned_ubo(ins)) {
         /* The load can't be pushed, so this UBO needs to be
          * uploaded conventionally */
         if (ins->src[1].type == BI_INDEX_CONSTANT)
            ctx->ubo_mask |= BITSET_BIT(ubo);
         else
            ctx->ubo_mask = ~0;

         continue;
      }

      /* Check if we decided to push this */
      assert(ubo < analysis.nr_blocks);
      if (!BITSET_TEST(analysis.blocks[ubo].pushed, offset / 4)) {
         ctx->ubo_mask |= BITSET_BIT(ubo);
         continue;
      }

      /* Replace the UBO load with moves from FAU */
      bi_builder b = bi_init_builder(ctx, bi_after_instr(ins));

      unsigned nr = bi_opcode_props[ins->op].sr_count;
      bi_instr *vec = bi_collect_i32_to(&b, ins->dest[0], nr);

      bi_foreach_src(vec, w) {
         /* FAU is grouped in pairs (2 x 4-byte) */
         unsigned base =
            pan_lookup_pushed_ubo(ctx->info.push, ubo, (offset + 4 * w));

         unsigned fau_idx = (base >> 1);
         unsigned fau_hi = (base & 1);

         vec->src[w] = bi_fau(BIR_FAU_UNIFORM | fau_idx, fau_hi);
      }

      bi_remove_instruction(ins);
   }

   free(analysis.blocks);
}

typedef struct {
   BITSET_DECLARE(row, PAN_MAX_PUSH);
} adjacency_row;

/* Find the connected component containing `node` with depth-first search */
static void
bi_find_component(adjacency_row *adjacency, BITSET_WORD *visited,
                  unsigned *component, unsigned *size, unsigned node)
{
   unsigned neighbour;

   BITSET_SET(visited, node);
   component[(*size)++] = node;

   BITSET_FOREACH_SET(neighbour, adjacency[node].row, PAN_MAX_PUSH) {
      if (!BITSET_TEST(visited, neighbour)) {
         bi_find_component(adjacency, visited, component, size, neighbour);
      }
   }
}

static bool
bi_is_uniform(bi_index idx)
{
   return (idx.type == BI_INDEX_FAU) && (idx.value & BIR_FAU_UNIFORM);
}

/* Get the index of a uniform in 32-bit words from the start of FAU-RAM */
static unsigned
bi_uniform_word(bi_index idx)
{
   assert(bi_is_uniform(idx));
   assert(idx.offset <= 1);

   return ((idx.value & ~BIR_FAU_UNIFORM) << 1) | idx.offset;
}

/*
 * Create an undirected graph where nodes are 32-bit uniform indices and edges
 * represent that two nodes are used in the same instruction.
 *
 * The graph is constructed as an adjacency matrix stored in adjacency.
 */
static void
bi_create_fau_interference_graph(bi_context *ctx, adjacency_row *adjacency)
{
   bi_foreach_instr_global(ctx, I) {
      unsigned nodes[BI_MAX_SRCS] = {};
      unsigned node_count = 0;

      /* Set nodes[] to 32-bit uniforms accessed */
      bi_foreach_src(I, s) {
         if (bi_is_uniform(I->src[s])) {
            unsigned word = bi_uniform_word(I->src[s]);

            if (word >= ctx->info.push_offset)
               nodes[node_count++] = word;
         }
      }

      /* Create clique connecting nodes[] */
      for (unsigned i = 0; i < node_count; ++i) {
         for (unsigned j = 0; j < node_count; ++j) {
            if (i == j)
               continue;

            unsigned x = nodes[i], y = nodes[j];
            assert(MAX2(x, y) < ctx->info.push->count);

            /* Add undirected edge between the nodes */
            BITSET_SET(adjacency[x].row, y);
            BITSET_SET(adjacency[y].row, x);
         }
      }
   }
}

/*
 * Optimization pass to reorder uniforms. The goal is to reduce the number of
 * moves we emit when lowering FAU. The pass groups uniforms used by the same
 * instruction.
 *
 * The pass works by creating a graph of pushed uniforms, where edges denote the
 * "both 32-bit uniforms required by the same instruction" relationship. We
 * perform depth-first search on this graph to find the connected components,
 * where each connected component is a cluster of uniforms that are used
 * together. We then select pairs of uniforms from each connected component.
 * The remaining unpaired uniforms (from components of odd sizes) are paired
 * together arbitrarily.
 *
 * After a new ordering is selected, pushed uniforms in the program and the
 * panfrost_ubo_push data structure must be remapped to use the new ordering.
 */
void
bi_opt_reorder_push(bi_context *ctx)
{
   adjacency_row adjacency[PAN_MAX_PUSH] = {0};
   BITSET_DECLARE(visited, PAN_MAX_PUSH) = {0};

   unsigned ordering[PAN_MAX_PUSH] = {0};
   unsigned unpaired[PAN_MAX_PUSH] = {0};
   unsigned pushed = 0, unpaired_count = 0;

   struct panfrost_ubo_push *push = ctx->info.push;
   unsigned push_offset = ctx->info.push_offset;

   bi_create_fau_interference_graph(ctx, adjacency);

   for (unsigned i = push_offset; i < push->count; ++i) {
      if (BITSET_TEST(visited, i))
         continue;

      unsigned component[PAN_MAX_PUSH] = {0};
      unsigned size = 0;
      bi_find_component(adjacency, visited, component, &size, i);

      /* If there is an odd number of uses, at least one use must be
       * unpaired. Arbitrarily take the last one.
       */
      if (size % 2)
         unpaired[unpaired_count++] = component[--size];

      /* The rest of uses are paired */
      assert((size % 2) == 0);

      /* Push the paired uses */
      memcpy(ordering + pushed, component, sizeof(unsigned) * size);
      pushed += size;
   }

   /* Push unpaired nodes at the end */
   memcpy(ordering + pushed, unpaired, sizeof(unsigned) * unpaired_count);
   pushed += unpaired_count;

   /* Ordering is a permutation. Invert it for O(1) lookup. */
   unsigned old_to_new[PAN_MAX_PUSH] = {0};

   for (unsigned i = 0; i < push_offset; ++i) {
      old_to_new[i] = i;
   }

   for (unsigned i = 0; i < pushed; ++i) {
      assert(ordering[i] >= push_offset);
      old_to_new[ordering[i]] = push_offset + i;
   }

   /* Use new ordering throughout the program */
   bi_foreach_instr_global(ctx, I) {
      bi_foreach_src(I, s) {
         if (bi_is_uniform(I->src[s])) {
            unsigned node = bi_uniform_word(I->src[s]);
            unsigned new_node = old_to_new[node];
            I->src[s].value = BIR_FAU_UNIFORM | (new_node >> 1);
            I->src[s].offset = new_node & 1;
         }
      }
   }

   /* Use new ordering for push */
   struct panfrost_ubo_push old = *push;
   for (unsigned i = 0; i < pushed; ++i)
      push->words[push_offset + i] = old.words[ordering[i]];

   push->count = push_offset + pushed;
}
