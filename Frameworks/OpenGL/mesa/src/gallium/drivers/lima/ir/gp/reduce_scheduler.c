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

#include <limits.h>

#include "gpir.h"

/* Register sensitive schedule algorithm from paper:
 * "Register-Sensitive Selection, Duplication, and Sequencing of Instructions"
 * Author: Vivek Sarkar,  Mauricio J. Serrano,  Barbara B. Simons
 */

static int cmp_float(const void *a, const void *b)
{
   const float *fa = (const float *) a;
   const float *fb = (const float *) b;
   return (*fa > *fb) - (*fa < *fb);
}

static void schedule_calc_sched_info(gpir_node *node)
{
   int n = 0;
   float extra_reg = 1.0f;

   /* update all children's sched info */
   gpir_node_foreach_pred(node, dep) {
      gpir_node *pred = dep->pred;

      if (pred->rsched.reg_pressure < 0)
         schedule_calc_sched_info(pred);

      int est = pred->rsched.est + 1;
      if (node->rsched.est < est)
         node->rsched.est = est;

      float reg_weight = 1.0f - 1.0f / list_length(&pred->succ_list);
      if (extra_reg > reg_weight)
         extra_reg = reg_weight;

      n++;
   }

   /* leaf instr */
   if (!n) {
      node->rsched.reg_pressure = 0;
      return;
   }

   int i = 0;
   float reg[n];
   gpir_node_foreach_pred(node, dep) {
      gpir_node *pred = dep->pred;
      reg[i++] = pred->rsched.reg_pressure;
   }

   /* sort */
   qsort(reg, n, sizeof(reg[0]), cmp_float);

   for (i = 0; i < n; i++) {
      float pressure = reg[i] + n - (i + 1);
      if (pressure > node->rsched.reg_pressure)
         node->rsched.reg_pressure = pressure;
   }

   /* If all children of this node have multi parents, then this
    * node need an extra reg to store its result. For example,
    * it's not fair for parent has the same reg pressure as child
    * if n==1 and child's successor>1, because we need 2 reg for
    * this.
    *
    * But we can't add a full reg to the reg_pressure, because the
    * last parent of a multi-successor child doesn't need an extra
    * reg. For example, a single child (with multi successor) node
    * should has less reg pressure than a two children (with single
    * successor) instr.
    *
    * extra reg = min(all child)(1.0 - 1.0 / num successor)
    */
   node->rsched.reg_pressure += extra_reg;
}

static void schedule_insert_ready_list(struct list_head *ready_list,
                                       gpir_node *insert_node)
{
   struct list_head *insert_pos = ready_list;

   list_for_each_entry(gpir_node, node, ready_list, list) {
      if (gpir_op_infos[node->op].schedule_first) {
         continue;
      }

      if (gpir_op_infos[insert_node->op].schedule_first ||
          insert_node->rsched.parent_index < node->rsched.parent_index ||
          (insert_node->rsched.parent_index == node->rsched.parent_index &&
           (insert_node->rsched.reg_pressure < node->rsched.reg_pressure ||
            (insert_node->rsched.reg_pressure == node->rsched.reg_pressure &&
             (insert_node->rsched.est >= node->rsched.est))))) {
         insert_pos = &node->list;
         if (node == insert_node)
            return;
         break;
      }
   }

   list_del(&insert_node->list);
   list_addtail(&insert_node->list, insert_pos);
}

static void schedule_ready_list(gpir_block *block, struct list_head *ready_list)
{
   if (list_is_empty(ready_list))
      return;

   gpir_node *node = list_first_entry(ready_list, gpir_node, list);
   list_del(&node->list);

   /* schedule the node to the block node list */
   list_add(&node->list, &block->node_list);
   node->rsched.scheduled = true;
   block->rsched.node_index--;

   gpir_node_foreach_pred(node, dep) {
      gpir_node *pred = dep->pred;
      pred->rsched.parent_index = block->rsched.node_index;

      bool ready = true;
      gpir_node_foreach_succ(pred, dep) {
         gpir_node *succ = dep->succ;
         if (!succ->rsched.scheduled) {
            ready = false;
            break;
         }
      }
      /* all successor have been scheduled */
      if (ready)
         schedule_insert_ready_list(ready_list, pred);
   }

   schedule_ready_list(block, ready_list);
}

static void schedule_block(gpir_block *block)
{
   /* move all nodes to node_list, block->node_list will
    * contain schedule result */
   struct list_head node_list;
   list_replace(&block->node_list, &node_list);
   list_inithead(&block->node_list);

   /* step 2 & 3 */
   list_for_each_entry(gpir_node, node, &node_list, list) {
      if (gpir_node_is_root(node))
         schedule_calc_sched_info(node);
      block->rsched.node_index++;
   }

   /* step 4 */
   struct list_head ready_list;
   list_inithead(&ready_list);

   /* step 5 */
   list_for_each_entry_safe(gpir_node, node, &node_list, list) {
      if (gpir_node_is_root(node)) {
         node->rsched.parent_index = INT_MAX;
         schedule_insert_ready_list(&ready_list, node);
      }
   }

   /* step 6 */
   schedule_ready_list(block, &ready_list);
}

/* Due to how we translate from NIR, we never read a register written in the
 * same block (we just pass the node through instead), so we don't have to
 * worry about read-after-write dependencies. We do have to worry about
 * write-after-read though, so we add those dependencies now. For example in a
 * loop like this we need a dependency between the write and the read of i:
 *
 * i = ...
 * while (...) {
 *    ... = i;
 *    i = i + 1;
 * }
 */

static void add_false_dependencies(gpir_compiler *comp)
{
   /* Make sure we allocate this only once, in case there are many values and
    * many blocks.
    */
   gpir_node **last_written = calloc(comp->cur_reg, sizeof(gpir_node *));

   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      list_for_each_entry_rev(gpir_node, node, &block->node_list, list) {
         if (node->op == gpir_op_load_reg) {
            gpir_load_node *load = gpir_node_to_load(node);
            gpir_node *store = last_written[load->reg->index];
            if (store && store->block == block) {
               gpir_node_add_dep(store, node, GPIR_DEP_WRITE_AFTER_READ);
            }
         } else if (node->op == gpir_op_store_reg) {
            gpir_store_node *store = gpir_node_to_store(node);
            last_written[store->reg->index] = node;
         }
      }
   }

   free(last_written);
}

bool gpir_reduce_reg_pressure_schedule_prog(gpir_compiler *comp)
{
   add_false_dependencies(comp);

   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      block->rsched.node_index = 0;
      list_for_each_entry_safe(gpir_node, node, &block->node_list, list) {
         node->rsched.reg_pressure = -1;
         node->rsched.est = 0;
         node->rsched.scheduled = false;
      }
   }

   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      schedule_block(block);
   }

   gpir_debug("after reduce scheduler\n");
   gpir_node_print_prog_seq(comp);
   return true;
}
