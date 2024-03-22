/*
 * Copyright (c) 2017 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "gpir.h"
#include "util/u_dynarray.h"

/* Per-register information */

struct reg_info {
   BITSET_WORD *conflicts;
   struct util_dynarray conflict_list;

   unsigned num_conflicts;

   int assigned_color;

   bool visited;
};

struct regalloc_ctx {
   unsigned bitset_words;
   struct reg_info *registers;

   /* Reusable scratch liveness array */
   BITSET_WORD *live;

   unsigned *worklist;
   unsigned worklist_start, worklist_end;

   unsigned *stack;
   unsigned stack_size;

   gpir_compiler *comp;
   void *mem_ctx;
};

/* Liveness analysis */

static void propagate_liveness_node(gpir_node *node, BITSET_WORD *live,
                                    gpir_compiler *comp)
{
   /* KILL */
   if (node->type == gpir_node_type_store) {
      if (node->op == gpir_op_store_reg) {
         gpir_store_node *store = gpir_node_to_store(node);
         BITSET_CLEAR(live, store->reg->index);
      }
   }

   /* GEN */
   if (node->type == gpir_node_type_load) {
      if (node->op == gpir_op_load_reg) {
         gpir_load_node *load = gpir_node_to_load(node);
         BITSET_SET(live, load->reg->index);
      }
   }
}

static bool propagate_liveness_block(gpir_block *block, struct regalloc_ctx *ctx)
{
   for (unsigned i = 0; i < 2; i++) {
      if (block->successors[i]) {
         for (unsigned j = 0; j < ctx->bitset_words; j++)
            block->live_out[j] |= block->successors[i]->live_in[j];
      }
   }

   memcpy(ctx->live, block->live_out, ctx->bitset_words * sizeof(BITSET_WORD));

   list_for_each_entry_rev(gpir_node, node, &block->node_list, list) {
      propagate_liveness_node(node, ctx->live, block->comp);
   }

   bool changed = false;
   for (unsigned i = 0; i < ctx->bitset_words; i++) {
      changed |= (block->live_in[i] != ctx->live[i]);
      block->live_in[i] = ctx->live[i];
   }
   return changed;
}

static void calc_def_block(gpir_block *block)
{
   list_for_each_entry(gpir_node, node, &block->node_list, list) {
      if (node->op == gpir_op_store_reg) {
         gpir_store_node *store = gpir_node_to_store(node);
         BITSET_SET(block->def_out, store->reg->index);
      }
   }
}

static void calc_liveness(struct regalloc_ctx *ctx)
{
   bool changed = true;
   while (changed) {
      changed = false;
      list_for_each_entry_rev(gpir_block, block, &ctx->comp->block_list, list) {
         changed |= propagate_liveness_block(block, ctx);
      }
   }

   list_for_each_entry(gpir_block, block, &ctx->comp->block_list, list) {
      calc_def_block(block);
   }

   changed = true;
   while (changed) {
      changed = false;
      list_for_each_entry(gpir_block, block, &ctx->comp->block_list, list) {
         for (unsigned i = 0; i < 2; i++) {
            gpir_block *succ = block->successors[i];
            if (!succ)
               continue;

            for (unsigned j = 0; j < ctx->bitset_words; j++) {
               BITSET_WORD new = block->def_out[j] & ~succ->def_out[j];
               changed |= (new != 0);
               succ->def_out[j] |= block->def_out[j];
            }
         }
      }
   }
}

/* Interference calculation */

static void add_interference(struct regalloc_ctx *ctx, unsigned i, unsigned j)
{
   if (i == j)
      return;

   struct reg_info *a = &ctx->registers[i];
   struct reg_info *b = &ctx->registers[j];

   if (BITSET_TEST(a->conflicts, j))
      return;

   BITSET_SET(a->conflicts, j);
   BITSET_SET(b->conflicts, i);

   a->num_conflicts++;
   b->num_conflicts++;
   util_dynarray_append(&a->conflict_list, unsigned, j);
   util_dynarray_append(&b->conflict_list, unsigned, i);
}

/* Make the register or node "i" interfere with all the other live registers
 * and nodes.
 */
static void add_all_interferences(struct regalloc_ctx *ctx,
                                  unsigned i,
                                  BITSET_WORD *live_regs)
{
   int live_reg;
   BITSET_FOREACH_SET(live_reg, ctx->live, ctx->comp->cur_reg) {
      add_interference(ctx, i, live_reg);
   }

}

static void print_liveness(struct regalloc_ctx *ctx,
                           BITSET_WORD *live_reg)
{
   if (!(lima_debug & LIMA_DEBUG_GP))
      return;

   int live_idx;
   BITSET_FOREACH_SET(live_idx, live_reg, ctx->comp->cur_reg) {
      printf("reg%d ", live_idx);
   }
   printf("\n");
}

static void calc_interference(struct regalloc_ctx *ctx)
{
   list_for_each_entry(gpir_block, block, &ctx->comp->block_list, list) {
      /* Initialize liveness at the end of the block, but exclude values that
       * definitely aren't defined by the end. This helps out with
       * partially-defined registers, like:
       *
       * if (condition) {
       *    foo = ...;
       * }
       * if (condition) {
       *    ... = foo;
       * }
       *
       * If we naively propagated liveness backwards, foo would be live from
       * the beginning of the program, but if we're not inside a loop then
       * its value is undefined before the first if and we don't have to
       * consider it live. Mask out registers like foo here.
       */
      for (unsigned i = 0; i < ctx->bitset_words; i++) {
         ctx->live[i] = block->live_out[i] & block->def_out[i];
      }

      list_for_each_entry_rev(gpir_node, node, &block->node_list, list) {
         gpir_debug("processing node %d\n", node->index);
         print_liveness(ctx, ctx->live);
         if (node->op == gpir_op_store_reg) {
            gpir_store_node *store = gpir_node_to_store(node);
            add_all_interferences(ctx, store->reg->index, ctx->live);

            /* KILL */
            BITSET_CLEAR(ctx->live, store->reg->index);
         } else if (node->op == gpir_op_load_reg) {
            /* GEN */
            gpir_load_node *load = gpir_node_to_load(node);
            BITSET_SET(ctx->live, load->reg->index);
         }
      }
   }
}

/* Register allocation */

static bool can_simplify(struct regalloc_ctx *ctx, unsigned i)
{
   struct reg_info *info = &ctx->registers[i];
   return info->num_conflicts < GPIR_PHYSICAL_REG_NUM;
}

static void push_stack(struct regalloc_ctx *ctx, unsigned i)
{
   ctx->stack[ctx->stack_size++] = i;
   gpir_debug("pushing reg%u\n", i);

   struct reg_info *info = &ctx->registers[i];
   assert(info->visited);

   util_dynarray_foreach(&info->conflict_list, unsigned, conflict) {
      struct reg_info *conflict_info = &ctx->registers[*conflict];
      assert(conflict_info->num_conflicts > 0);
      conflict_info->num_conflicts--;
      if (!ctx->registers[*conflict].visited && can_simplify(ctx, *conflict)) {
         ctx->worklist[ctx->worklist_end++] = *conflict;
         ctx->registers[*conflict].visited = true;
      }
   }
}

static bool do_regalloc(struct regalloc_ctx *ctx)
{
   ctx->worklist_start = 0;
   ctx->worklist_end = 0;
   ctx->stack_size = 0;

   /* Step 1: find the initially simplifiable registers */
   for (int i = 0; i < ctx->comp->cur_reg; i++) {
      if (can_simplify(ctx, i)) {
         ctx->worklist[ctx->worklist_end++] = i;
         ctx->registers[i].visited = true;
      }
   }

   while (true) {
      /* Step 2: push onto the stack whatever we can */
      while (ctx->worklist_start != ctx->worklist_end) {
         push_stack(ctx, ctx->worklist[ctx->worklist_start++]);
      }

      if (ctx->stack_size < ctx->comp->cur_reg) {
         /* If there are still unsimplifiable nodes left, we need to
          * optimistically push a node onto the stack.  Choose the one with
          * the smallest number of current neighbors, since that's the most
          * likely to succeed.
          */
         unsigned min_conflicts = UINT_MAX;
         unsigned best_reg = 0;
         for (int reg = 0; reg < ctx->comp->cur_reg; reg++) {
            struct reg_info *info = &ctx->registers[reg];
            if (info->visited)
               continue;
            if (info->num_conflicts < min_conflicts) {
               best_reg = reg;
               min_conflicts = info->num_conflicts;
            }
         }
         gpir_debug("optimistic triggered\n");
         ctx->registers[best_reg].visited = true;
         push_stack(ctx, best_reg);
      } else {
         break;
      }
   }

   /* Step 4: pop off the stack and assign colors */
   for (int i = ctx->comp->cur_reg - 1; i >= 0; i--) {
      unsigned idx = ctx->stack[i];
      struct reg_info *reg = &ctx->registers[idx];

      bool found = false;
      unsigned start = i % GPIR_PHYSICAL_REG_NUM;
      for (unsigned j = 0; j < GPIR_PHYSICAL_REG_NUM; j++) {
         unsigned candidate = (j + start) % GPIR_PHYSICAL_REG_NUM;
         bool available = true;
         util_dynarray_foreach(&reg->conflict_list, unsigned, conflict_idx) {
            struct reg_info *conflict = &ctx->registers[*conflict_idx];
            if (conflict->assigned_color >= 0 &&
                conflict->assigned_color == (int) candidate) {
               available = false;
               break;
            }
         }

         if (available) {
            reg->assigned_color = candidate;
            found = true;
            break;
         }
      }

      /* TODO: spilling */
      if (!found) {
         gpir_error("Failed to allocate registers\n");
         return false;
      }
   }

   return true;
}

static void assign_regs(struct regalloc_ctx *ctx)
{
   list_for_each_entry(gpir_block, block, &ctx->comp->block_list, list) {
      list_for_each_entry(gpir_node, node, &block->node_list, list) {
         if (node->op == gpir_op_load_reg) {
            gpir_load_node *load = gpir_node_to_load(node);
            unsigned color = ctx->registers[load->reg->index].assigned_color;
            load->index = color / 4;
            load->component = color % 4;
         }

         if (node->op == gpir_op_store_reg) {
            gpir_store_node *store = gpir_node_to_store(node);
            unsigned color = ctx->registers[store->reg->index].assigned_color;
            store->index = color / 4;
            store->component = color % 4;
            node->value_reg = color;
         }
      }

      block->live_out_phys = 0;

      int reg_idx;
      BITSET_FOREACH_SET(reg_idx, block->live_out, ctx->comp->cur_reg) {
         if (BITSET_TEST(block->def_out, reg_idx)) {
            block->live_out_phys |= (1ull << ctx->registers[reg_idx].assigned_color);
         }
      }
   }
}

/* Value register allocation */

/* Define a special token for when the register is occupied by a preallocated
 * physical register (i.e. load_reg/store_reg). Normally entries in the "live"
 * array points to the definition of the value, but there may be multiple
 * definitions in this case, and they will certainly come from other basic
 * blocks, so it doesn't make sense to do that here.
 */
static gpir_node __physreg_live;
#define PHYSREG_LIVE (&__physreg_live)

struct value_regalloc_ctx {
   gpir_node *last_written[GPIR_VALUE_REG_NUM + GPIR_PHYSICAL_REG_NUM];
   gpir_node *complex1_last_written[GPIR_VALUE_REG_NUM + GPIR_PHYSICAL_REG_NUM];
   gpir_node *live[GPIR_VALUE_REG_NUM + GPIR_PHYSICAL_REG_NUM];
   gpir_node *last_complex1;
   unsigned alloc_start;
};

static unsigned find_free_value_reg(struct value_regalloc_ctx *ctx)
{
   /* Implement round-robin allocation */
   unsigned reg_offset = ctx->alloc_start++;
   if (ctx->alloc_start == GPIR_PHYSICAL_REG_NUM + GPIR_VALUE_REG_NUM)
      ctx->alloc_start = 0;

   unsigned reg = UINT_MAX;
   for (unsigned reg_base = 0;
        reg_base < GPIR_PHYSICAL_REG_NUM + GPIR_VALUE_REG_NUM;
        reg_base++) {
      unsigned cur_reg = (reg_base + reg_offset) % (GPIR_PHYSICAL_REG_NUM + GPIR_VALUE_REG_NUM);
      if (!ctx->live[cur_reg]) {
         reg = cur_reg;
         break;
      }
   }

   return reg;
}

static void add_fake_dep(gpir_node *node, gpir_node *src,
                         struct value_regalloc_ctx *ctx)
{
   assert(src->value_reg >= 0);
   if (ctx->last_written[src->value_reg] &&
       ctx->last_written[src->value_reg] != node) {
      gpir_node_add_dep(ctx->last_written[src->value_reg], node,
                        GPIR_DEP_WRITE_AFTER_READ);
   }

   /* For a sequence of schedule_first nodes right before a complex1
    * node, add any extra fake dependencies necessary so that the
    * schedule_first nodes can be scheduled right after the complex1 is
    * scheduled. We have to save the last_written before complex1 happens to
    * avoid adding dependencies to children of the complex1 node which would
    * create a cycle.
    */

   if (gpir_op_infos[node->op].schedule_first &&
       ctx->last_complex1 &&
       ctx->complex1_last_written[src->value_reg]) {
      gpir_node_add_dep(ctx->complex1_last_written[src->value_reg],
                        ctx->last_complex1,
                        GPIR_DEP_WRITE_AFTER_READ);
   }
}

static bool handle_value_read(gpir_node *node, gpir_node *src,
                              struct value_regalloc_ctx *ctx)
{
   /* If already allocated, don't allocate it */
   if (src->value_reg < 0) {
      unsigned reg = find_free_value_reg(ctx);
      if (reg == UINT_MAX)
         return false;

      src->value_reg = reg;
      ctx->live[reg] = src;
   }

   /* Add any fake dependencies. Note: this is the actual result of value
    * register allocation. We throw away node->value_reg afterwards, since
    * it's really the fake dependencies which constrain the post-RA scheduler
    * enough to make sure it never needs to spill to temporaries.
    */
   add_fake_dep(node, src, ctx);

   return true;
}

static bool handle_reg_read(gpir_load_node *load,
                            struct value_regalloc_ctx *ctx)
{
   unsigned idx = load->index * 4 + load->component;
   if (!ctx->live[idx]) {
      ctx->live[idx] = PHYSREG_LIVE;
   } else if (ctx->live[idx] != PHYSREG_LIVE) {
      /* This slot is occupied by some other value register, so we need to
       * evict it. This effectively splits the live range of the value
       * register. NB: since we create fake dependencies on the fly, and the
       * fake dependencies are the only output of this pass, we don't actually
       * have to record where the split happened or that this value was
       * assigned to two different registers. Any actual live range splitting
       * happens in the post-RA scheduler, which moves the value to and from
       * the register file. This will just cause some reads of the value
       * register to have different fake dependencies.
       */
      unsigned new_reg = find_free_value_reg(ctx);
      if (new_reg == UINT_MAX)
         return false;
      ctx->live[new_reg] = ctx->live[idx];
      ctx->live[new_reg]->value_reg = new_reg;
      ctx->live[idx] = PHYSREG_LIVE;
   }

   if (ctx->last_written[idx]) {
      gpir_node_add_dep(ctx->last_written[idx], &load->node,
                        GPIR_DEP_WRITE_AFTER_READ);
   }

   return true;
}

static void handle_reg_write(gpir_store_node *store,
                             struct value_regalloc_ctx *ctx)
{
   unsigned idx = store->index * 4 + store->component;
   store->node.value_reg = idx;
   ctx->last_written[idx] = &store->node;
   ctx->live[idx] = NULL;
}

static void handle_value_write(gpir_node *node,
                               struct value_regalloc_ctx *ctx)
{
   /* TODO: why does an uninitialized node->value_reg
    * sometimes end up here? */
   if (node->value_reg < 0)
      return;

   ctx->last_written[node->value_reg] = node;
   ctx->live[node->value_reg] = NULL;
}

static bool regalloc_value_regs(gpir_block *block)
{
   struct value_regalloc_ctx ctx = { { 0 } };

   list_for_each_entry(gpir_node, node, &block->node_list, list) {
      node->value_reg = -1;
   }

   list_for_each_entry_rev(gpir_node, node, &block->node_list, list) {
      if (node->op == gpir_op_complex1) {
         ctx.last_complex1 = node;
         memcpy(ctx.complex1_last_written, ctx.last_written,
                sizeof(ctx.complex1_last_written));
      }

      if (node->type != gpir_node_type_store &&
          node->type != gpir_node_type_branch) {
         handle_value_write(node, &ctx);
      } else if (node->op == gpir_op_store_reg) {
         handle_reg_write(gpir_node_to_store(node), &ctx);
      }

      if (node->type == gpir_node_type_store) {
         gpir_store_node *store = gpir_node_to_store(node);
         if (!handle_value_read(&store->node, store->child, &ctx))
            return false;
      } else if (node->type == gpir_node_type_alu) {
         gpir_alu_node *alu = gpir_node_to_alu(node);
         for (int i = 0; i < alu->num_child; i++) {
            if (!handle_value_read(&alu->node, alu->children[i], &ctx))
               return false;
         }
      } else if (node->type == gpir_node_type_branch) {
         /* At the end of a block the top 11 values are always free, so
          * branches should always succeed.
          */
         gpir_branch_node *branch = gpir_node_to_branch(node);
         ASSERTED bool result = handle_value_read(&branch->node,
                                                  branch->cond, &ctx);
         assert(result);
      } else if (node->op == gpir_op_load_reg) {
         gpir_load_node *load = gpir_node_to_load(node);
         if (!handle_reg_read(load, &ctx))
             return false;
      }
   }

   return true;
}

static void regalloc_print_result(gpir_compiler *comp)
{
   if (!(lima_debug & LIMA_DEBUG_GP))
      return;

   int index = 0;
   printf("======== regalloc ========\n");
   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      list_for_each_entry(gpir_node, node, &block->node_list, list) {
         printf("%03d: %d/%d %s ", index++, node->index, node->value_reg,
                gpir_op_infos[node->op].name);
         gpir_node_foreach_pred(node, dep) {
            gpir_node *pred = dep->pred;
            printf(" %d/%d", pred->index, pred->value_reg);
         }
         if (node->op == gpir_op_load_reg) {
            gpir_load_node *load = gpir_node_to_load(node);
            printf(" -/%d", 4 * load->index + load->component);
            printf(" (%d)", load->reg->index);
         } else if (node->op == gpir_op_store_reg) {
            gpir_store_node *store = gpir_node_to_store(node);
            printf(" (%d)", store->reg->index);
         }
         printf("\n");
      }
      printf("----------------------------\n");
   }
}

bool gpir_regalloc_prog(gpir_compiler *comp)
{
   struct regalloc_ctx ctx;

   ctx.mem_ctx = ralloc_context(NULL);
   ctx.bitset_words = BITSET_WORDS(comp->cur_reg);
   ctx.live = ralloc_array(ctx.mem_ctx, BITSET_WORD, ctx.bitset_words);
   ctx.worklist = ralloc_array(ctx.mem_ctx, unsigned, comp->cur_reg);
   ctx.stack = ralloc_array(ctx.mem_ctx, unsigned, comp->cur_reg);
   ctx.comp = comp;

   ctx.registers = rzalloc_array(ctx.mem_ctx, struct reg_info, comp->cur_reg);
   for (int i = 0; i < comp->cur_reg; i++) {
      ctx.registers[i].conflicts = rzalloc_array(ctx.mem_ctx, BITSET_WORD,
                                                 ctx.bitset_words);
      util_dynarray_init(&ctx.registers[i].conflict_list, ctx.mem_ctx);
   }

   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      block->live_out = rzalloc_array(ctx.mem_ctx, BITSET_WORD, ctx.bitset_words);
      block->live_in = rzalloc_array(ctx.mem_ctx, BITSET_WORD, ctx.bitset_words);
      block->def_out = rzalloc_array(ctx.mem_ctx, BITSET_WORD, ctx.bitset_words);
   }

   calc_liveness(&ctx);
   calc_interference(&ctx);
   if (!do_regalloc(&ctx)) {
      ralloc_free(ctx.mem_ctx);
      return false;
   }
   assign_regs(&ctx);

   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      if (!regalloc_value_regs(block))
         return false;
   }

   regalloc_print_result(comp);
   ralloc_free(ctx.mem_ctx);
   return true;
}
