/*
 * Copyright (C) 2022 Collabora Ltd.
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

/* Bottom-up local scheduler to reduce register pressure */

#include "util/dag.h"
#include "compiler.h"

struct sched_ctx {
   /* Dependency graph */
   struct dag *dag;

   /* Live set */
   BITSET_WORD *live;
};

struct sched_node {
   struct dag_node dag;

   /* Instruction this node represents */
   bi_instr *instr;
};

static void
add_dep(struct sched_node *a, struct sched_node *b)
{
   if (a && b)
      dag_add_edge(&a->dag, &b->dag, 0);
}

static struct dag *
create_dag(bi_context *ctx, bi_block *block, void *memctx)
{
   struct dag *dag = dag_create(ctx);

   struct sched_node **last_write =
      calloc(ctx->ssa_alloc, sizeof(struct sched_node *));
   struct sched_node *coverage = NULL;
   struct sched_node *preload = NULL;

   /* Last memory load, to serialize stores against */
   struct sched_node *memory_load = NULL;

   /* Last memory store, to serialize loads and stores against */
   struct sched_node *memory_store = NULL;

   bi_foreach_instr_in_block(block, I) {
      /* Leave branches at the end */
      if (I->op == BI_OPCODE_JUMP || bi_opcode_props[I->op].branch)
         break;

      assert(I->branch_target == NULL);

      struct sched_node *node = rzalloc(memctx, struct sched_node);
      node->instr = I;
      dag_init_node(dag, &node->dag);

      /* Reads depend on writes, no other hazards in SSA */
      bi_foreach_ssa_src(I, s)
         add_dep(node, last_write[I->src[s].value]);

      bi_foreach_dest(I, d)
         last_write[I->dest[d].value] = node;

      switch (bi_opcode_props[I->op].message) {
      case BIFROST_MESSAGE_LOAD:
         /* Regular memory loads needs to be serialized against
          * other memory access. However, UBO memory is read-only
          * so it can be moved around freely.
          */
         if (I->seg != BI_SEG_UBO) {
            add_dep(node, memory_store);
            memory_load = node;
         }

         break;

      case BIFROST_MESSAGE_ATTRIBUTE:
         /* Regular attribute loads can be reordered, but
          * writeable attributes can't be. Our one use of
          * writeable attributes are images.
          */
         if ((I->op == BI_OPCODE_LD_TEX) || (I->op == BI_OPCODE_LD_TEX_IMM) ||
             (I->op == BI_OPCODE_LD_ATTR_TEX)) {
            add_dep(node, memory_store);
            memory_load = node;
         }

         break;

      case BIFROST_MESSAGE_STORE:
         assert(I->seg != BI_SEG_UBO);
         add_dep(node, memory_load);
         add_dep(node, memory_store);
         memory_store = node;
         break;

      case BIFROST_MESSAGE_ATOMIC:
      case BIFROST_MESSAGE_BARRIER:
         add_dep(node, memory_load);
         add_dep(node, memory_store);
         memory_load = node;
         memory_store = node;
         break;

      case BIFROST_MESSAGE_BLEND:
      case BIFROST_MESSAGE_Z_STENCIL:
      case BIFROST_MESSAGE_TILE:
         add_dep(node, coverage);
         coverage = node;
         break;

      case BIFROST_MESSAGE_ATEST:
         /* ATEST signals the end of shader side effects */
         add_dep(node, memory_store);
         memory_store = node;

         /* ATEST also updates coverage */
         add_dep(node, coverage);
         coverage = node;
         break;
      default:
         break;
      }

      add_dep(node, preload);

      if (I->op == BI_OPCODE_DISCARD_F32) {
         /* Serialize against ATEST */
         add_dep(node, coverage);
         coverage = node;

         /* Also serialize against memory and barriers */
         add_dep(node, memory_load);
         add_dep(node, memory_store);
         memory_load = node;
         memory_store = node;
      } else if ((I->op == BI_OPCODE_PHI) ||
                 (I->op == BI_OPCODE_MOV_I32 &&
                  I->src[0].type == BI_INDEX_REGISTER)) {
         preload = node;
      }
   }

   free(last_write);

   return dag;
}

/*
 * Calculate the change in register pressure from scheduling a given
 * instruction. Equivalently, calculate the difference in the number of live
 * registers before and after the instruction, given the live set after the
 * instruction. This calculation follows immediately from the dataflow
 * definition of liveness:
 *
 *      live_in = (live_out - KILL) + GEN
 */
static signed
calculate_pressure_delta(bi_instr *I, BITSET_WORD *live)
{
   signed delta = 0;

   /* Destinations must be unique */
   bi_foreach_dest(I, d) {
      if (BITSET_TEST(live, I->dest[d].value))
         delta -= bi_count_write_registers(I, d);
   }

   bi_foreach_ssa_src(I, src) {
      /* Filter duplicates */
      bool dupe = false;

      for (unsigned i = 0; i < src; ++i) {
         if (bi_is_equiv(I->src[i], I->src[src])) {
            dupe = true;
            break;
         }
      }

      if (!dupe && !BITSET_TEST(live, I->src[src].value))
         delta += bi_count_read_registers(I, src);
   }

   return delta;
}

/*
 * Choose the next instruction, bottom-up. For now we use a simple greedy
 * heuristic: choose the instruction that has the best effect on liveness.
 */
static struct sched_node *
choose_instr(struct sched_ctx *s)
{
   int32_t min_delta = INT32_MAX;
   struct sched_node *best = NULL;

   list_for_each_entry(struct sched_node, n, &s->dag->heads, dag.link) {
      int32_t delta = calculate_pressure_delta(n->instr, s->live);

      if (delta < min_delta) {
         best = n;
         min_delta = delta;
      }
   }

   return best;
}

static void
pressure_schedule_block(bi_context *ctx, bi_block *block, struct sched_ctx *s)
{
   /* off by a constant, that's ok */
   signed pressure = 0;
   signed orig_max_pressure = 0;
   unsigned nr_ins = 0;

   memcpy(s->live, block->ssa_live_out,
          BITSET_WORDS(ctx->ssa_alloc) * sizeof(BITSET_WORD));

   bi_foreach_instr_in_block_rev(block, I) {
      pressure += calculate_pressure_delta(I, s->live);
      orig_max_pressure = MAX2(pressure, orig_max_pressure);
      bi_liveness_ins_update_ssa(s->live, I);
      nr_ins++;
   }

   memcpy(s->live, block->ssa_live_out,
          BITSET_WORDS(ctx->ssa_alloc) * sizeof(BITSET_WORD));

   /* off by a constant, that's ok */
   signed max_pressure = 0;
   pressure = 0;

   struct sched_node **schedule = calloc(nr_ins, sizeof(struct sched_node *));
   nr_ins = 0;

   while (!list_is_empty(&s->dag->heads)) {
      struct sched_node *node = choose_instr(s);
      pressure += calculate_pressure_delta(node->instr, s->live);
      max_pressure = MAX2(pressure, max_pressure);
      dag_prune_head(s->dag, &node->dag);

      schedule[nr_ins++] = node;
      bi_liveness_ins_update_ssa(s->live, node->instr);
   }

   /* Bail if it looks like it's worse */
   if (max_pressure >= orig_max_pressure) {
      free(schedule);
      return;
   }

   /* Apply the schedule */
   for (unsigned i = 0; i < nr_ins; ++i) {
      bi_remove_instruction(schedule[i]->instr);
      list_add(&schedule[i]->instr->link, &block->instructions);
   }

   free(schedule);
}

void
bi_pressure_schedule(bi_context *ctx)
{
   bi_compute_liveness_ssa(ctx);
   void *memctx = ralloc_context(ctx);
   BITSET_WORD *live =
      ralloc_array(memctx, BITSET_WORD, BITSET_WORDS(ctx->ssa_alloc));

   bi_foreach_block(ctx, block) {
      struct sched_ctx sctx = {.dag = create_dag(ctx, block, memctx),
                               .live = live};

      pressure_schedule_block(ctx, block, &sctx);
   }

   ralloc_free(memctx);
}
