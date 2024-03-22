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

#include "util/ralloc.h"

#include "gpir.h"
#include "lima_context.h"

static bool gpir_lower_const(gpir_compiler *comp)
{
   int num_constant = 0;
   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      list_for_each_entry_safe(gpir_node, node, &block->node_list, list) {
         if (node->op == gpir_op_const) {
            if (gpir_node_is_root(node))
               gpir_node_delete(node);
            else
               num_constant++;
         }
      }
   }

   if (num_constant) {
      union fi *constant = ralloc_array(comp->prog, union fi, num_constant);
      if (!constant)
         return false;

      comp->prog->constant = constant;
      comp->prog->state.constant_size = num_constant * sizeof(union fi);

      int index = 0;
      list_for_each_entry(gpir_block, block, &comp->block_list, list) {
         list_for_each_entry_safe(gpir_node, node, &block->node_list, list) {
            if (node->op == gpir_op_const) {
               gpir_const_node *c = gpir_node_to_const(node);

               if (!gpir_node_is_root(node)) {
                  gpir_load_node *load = gpir_node_create(block, gpir_op_load_uniform);
                  if (unlikely(!load))
                     return false;

                  load->index = comp->constant_base + (index >> 2);
                  load->component = index % 4;
                  constant[index++] = c->value;

                  gpir_node_replace_succ(&load->node, node);

                  list_addtail(&load->node.list, &node->list);

                  gpir_debug("lower const create uniform %d for const %d\n",
                             load->node.index, node->index);
               }

               gpir_node_delete(node);
            }
         }
      }
   }

   return true;
}

/* duplicate load to all its successors */
static bool gpir_lower_load(gpir_compiler *comp)
{
   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      list_for_each_entry_safe(gpir_node, node, &block->node_list, list) {
         if (node->type == gpir_node_type_load) {
            gpir_load_node *load = gpir_node_to_load(node);

            bool first = true;
            gpir_node_foreach_succ_safe(node, dep) {
               gpir_node *succ = dep->succ;

               if (first) {
                  first = false;
                  continue;
               }

               gpir_node *new = gpir_node_create(succ->block, node->op);
               if (unlikely(!new))
                  return false;
               list_addtail(&new->list, &succ->list);

               gpir_debug("lower load create %d from %d for succ %d\n",
                          new->index, node->index, succ->index);

               gpir_load_node *nload = gpir_node_to_load(new);
               nload->index = load->index;
               nload->component = load->component;
               nload->reg = load->reg;

               gpir_node_replace_pred(dep, new);
               gpir_node_replace_child(succ, node, new);
            }
         }
      }
   }

   return true;
}

static bool gpir_lower_neg(gpir_block *block, gpir_node *node)
{
   gpir_alu_node *neg = gpir_node_to_alu(node);
   gpir_node *child = neg->children[0];

   /* check if child can dest negate */
   if (child->type == gpir_node_type_alu) {
      /* negate must be its only successor */
      if (list_is_singular(&child->succ_list) &&
          gpir_op_infos[child->op].dest_neg) {
         gpir_alu_node *alu = gpir_node_to_alu(child);
         alu->dest_negate = !alu->dest_negate;

         gpir_node_replace_succ(child, node);
         gpir_node_delete(node);
         return true;
      }
   }

   /* check if child can src negate */
   gpir_node_foreach_succ_safe(node, dep) {
      gpir_node *succ = dep->succ;
      if (succ->type != gpir_node_type_alu)
         continue;

      bool success = true;
      gpir_alu_node *alu = gpir_node_to_alu(dep->succ);
      for (int i = 0; i < alu->num_child; i++) {
         if (alu->children[i] == node) {
            if (gpir_op_infos[succ->op].src_neg[i]) {
               alu->children_negate[i] = !alu->children_negate[i];
               alu->children[i] = child;
            }
            else
               success = false;
         }
      }

      if (success)
         gpir_node_replace_pred(dep, child);
   }

   if (gpir_node_is_root(node))
      gpir_node_delete(node);

   return true;
}

static bool gpir_lower_complex(gpir_block *block, gpir_node *node)
{
   gpir_alu_node *alu = gpir_node_to_alu(node);
   gpir_node *child = alu->children[0];

   if (node->op == gpir_op_exp2) {
      gpir_alu_node *preexp2 = gpir_node_create(block, gpir_op_preexp2);
      if (unlikely(!preexp2))
         return false;

      preexp2->children[0] = child;
      preexp2->num_child = 1;
      gpir_node_add_dep(&preexp2->node, child, GPIR_DEP_INPUT);
      list_addtail(&preexp2->node.list, &node->list);

      child = &preexp2->node;
   }

   gpir_alu_node *complex2 = gpir_node_create(block, gpir_op_complex2);
   if (unlikely(!complex2))
      return false;

   complex2->children[0] = child;
   complex2->num_child = 1;
   gpir_node_add_dep(&complex2->node, child, GPIR_DEP_INPUT);
   list_addtail(&complex2->node.list, &node->list);

   int impl_op = 0;
   switch (node->op) {
   case gpir_op_rcp:
      impl_op = gpir_op_rcp_impl;
      break;
   case gpir_op_rsqrt:
      impl_op = gpir_op_rsqrt_impl;
      break;
   case gpir_op_exp2:
      impl_op = gpir_op_exp2_impl;
      break;
   case gpir_op_log2:
      impl_op = gpir_op_log2_impl;
      break;
   default:
      assert(0);
   }

   gpir_alu_node *impl = gpir_node_create(block, impl_op);
   if (unlikely(!impl))
      return false;

   impl->children[0] = child;
   impl->num_child = 1;
   gpir_node_add_dep(&impl->node, child, GPIR_DEP_INPUT);
   list_addtail(&impl->node.list, &node->list);

   gpir_alu_node *complex1 = gpir_node_create(block, gpir_op_complex1);
   complex1->children[0] = &impl->node;
   complex1->children[1] = &complex2->node;
   complex1->children[2] = child;
   complex1->num_child = 3;
   gpir_node_add_dep(&complex1->node, child, GPIR_DEP_INPUT);
   gpir_node_add_dep(&complex1->node, &impl->node, GPIR_DEP_INPUT);
   gpir_node_add_dep(&complex1->node, &complex2->node, GPIR_DEP_INPUT);
   list_addtail(&complex1->node.list, &node->list);

   gpir_node *result = &complex1->node;

   if (node->op == gpir_op_log2) {
      gpir_alu_node *postlog2 = gpir_node_create(block, gpir_op_postlog2);
      if (unlikely(!postlog2))
         return false;

      postlog2->children[0] = result;
      postlog2->num_child = 1;
      gpir_node_add_dep(&postlog2->node, result, GPIR_DEP_INPUT);
      list_addtail(&postlog2->node.list, &node->list);

      result = &postlog2->node;
   }

   gpir_node_replace_succ(result, node);
   gpir_node_delete(node);

   return true;
}

static bool gpir_lower_node_may_consume_two_slots(gpir_compiler *comp)
{
   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      list_for_each_entry_safe(gpir_node, node, &block->node_list, list) {
         if (gpir_op_infos[node->op].may_consume_two_slots) {
            /* dummy_f/m are auxiliary nodes for value reg alloc:
             * 1. before reg alloc, create fake nodes dummy_f, dummy_m,
             *    so the tree become: (dummy_m (node dummy_f))
             *    dummy_m can be spilled, but other nodes in the tree can't
             *    be spilled.
             * 2. After reg allocation and fake dep add, merge all deps of
             *    dummy_m and dummy_f to node and remove dummy_m & dummy_f
             *
             * We may also not use dummy_f/m, but alloc two value reg for
             * node. But that means we need to make sure there're 2 free
             * slot after the node successors, but we just need one slot
             * after to be able to schedule it because we can use one move for
             * the two slot node. It's also not easy to handle the spill case
             * for the alloc 2 value method.
             *
             * With the dummy_f/m method, there's no such requirement, the
             * node can be scheduled only when there's two slots for it,
             * otherwise a move. And the node can be spilled with one reg.
             */
            gpir_node *dummy_m = gpir_node_create(block, gpir_op_dummy_m);
            if (unlikely(!dummy_m))
               return false;
            list_add(&dummy_m->list, &node->list);

            gpir_node *dummy_f = gpir_node_create(block, gpir_op_dummy_f);
            if (unlikely(!dummy_f))
               return false;
            list_add(&dummy_f->list, &node->list);

            gpir_alu_node *alu = gpir_node_to_alu(dummy_m);
            alu->children[0] = node;
            alu->children[1] = dummy_f;
            alu->num_child = 2;

            gpir_node_replace_succ(dummy_m, node);
            gpir_node_add_dep(dummy_m, node, GPIR_DEP_INPUT);
            gpir_node_add_dep(dummy_m, dummy_f, GPIR_DEP_INPUT);

         }
      }
   }

   return true;
}

/*
 * There are no 'equal' or 'not-equal' opcodes.
 * eq (a == b) is lowered to and(a >= b, b >= a)
 * ne (a != b) is lowered to or(a < b, b < a)
 */
static bool gpir_lower_eq_ne(gpir_block *block, gpir_node *node)
{
   gpir_op cmp_node_op;
   gpir_op node_new_op;
   switch (node->op) {
      case gpir_op_eq:
         cmp_node_op = gpir_op_ge;
         node_new_op = gpir_op_min; /* and */
         break;
      case gpir_op_ne:
         cmp_node_op = gpir_op_lt;
         node_new_op = gpir_op_max; /* or */
         break;
      default:
         unreachable("bad node op");
   }

   gpir_alu_node *e = gpir_node_to_alu(node);

   gpir_alu_node *cmp1 = gpir_node_create(block, cmp_node_op);
   list_addtail(&cmp1->node.list, &node->list);
   gpir_alu_node *cmp2 = gpir_node_create(block, cmp_node_op);
   list_addtail(&cmp2->node.list, &node->list);

   cmp1->children[0] = e->children[0];
   cmp1->children[1] = e->children[1];
   cmp1->num_child = 2;

   cmp2->children[0] = e->children[1];
   cmp2->children[1] = e->children[0];
   cmp2->num_child = 2;

   gpir_node_add_dep(&cmp1->node, e->children[0], GPIR_DEP_INPUT);
   gpir_node_add_dep(&cmp1->node, e->children[1], GPIR_DEP_INPUT);

   gpir_node_add_dep(&cmp2->node, e->children[0], GPIR_DEP_INPUT);
   gpir_node_add_dep(&cmp2->node, e->children[1], GPIR_DEP_INPUT);

   gpir_node_foreach_pred_safe(node, dep) {
      gpir_node_remove_dep(node, dep->pred);
   }

   gpir_node_add_dep(node, &cmp1->node, GPIR_DEP_INPUT);
   gpir_node_add_dep(node, &cmp2->node, GPIR_DEP_INPUT);

   node->op = node_new_op;
   e->children[0] = &cmp1->node;
   e->children[1] = &cmp2->node;
   e->num_child = 2;

   return true;
}

/*
 * There is no 'abs' opcode.
 * abs(a) is lowered to max(a, -a)
 */
static bool gpir_lower_abs(gpir_block *block, gpir_node *node)
{
   gpir_alu_node *alu = gpir_node_to_alu(node);

   assert(node->op == gpir_op_abs);

   node->op = gpir_op_max;

   alu->children[1] = alu->children[0];
   alu->children_negate[1] = true;
   alu->num_child = 2;

   return true;
}

/*
 * There is no 'not' opcode.
 * not(a) is lowered to add(1, -a)
 */
static bool gpir_lower_not(gpir_block *block, gpir_node *node)
{
   gpir_alu_node *alu = gpir_node_to_alu(node);

   assert(alu->node.op == gpir_op_not);

   node->op = gpir_op_add;

   gpir_node *node_const = gpir_node_create(block, gpir_op_const);
   gpir_const_node *c = gpir_node_to_const(node_const);

   assert(c->node.op == gpir_op_const);

   list_addtail(&c->node.list, &node->list);
   c->value.f = 1.0f;
   gpir_node_add_dep(&alu->node, &c->node, GPIR_DEP_INPUT);

   alu->children_negate[1] = !alu->children_negate[0];
   alu->children[1] = alu->children[0];
   alu->children[0] = &c->node;
   alu->num_child = 2;

   return true;
}

/* There is no unconditional branch instruction, so we have to lower it to a
 * conditional branch with a condition of 1.0.
 */

static bool gpir_lower_branch_uncond(gpir_block *block, gpir_node *node)
{
   gpir_branch_node *branch = gpir_node_to_branch(node);

   gpir_node *node_const = gpir_node_create(block, gpir_op_const);
   gpir_const_node *c = gpir_node_to_const(node_const);

   list_addtail(&c->node.list, &node->list);
   c->value.f = 1.0f;
   gpir_node_add_dep(&branch->node, &c->node, GPIR_DEP_INPUT);

   branch->node.op = gpir_op_branch_cond;
   branch->cond = node_const;

   return true;
}

static bool (*gpir_pre_rsched_lower_funcs[gpir_op_num])(gpir_block *, gpir_node *) = {
   [gpir_op_not] = gpir_lower_not,
   [gpir_op_neg] = gpir_lower_neg,
   [gpir_op_rcp] = gpir_lower_complex,
   [gpir_op_rsqrt] = gpir_lower_complex,
   [gpir_op_exp2] = gpir_lower_complex,
   [gpir_op_log2] = gpir_lower_complex,
   [gpir_op_eq] = gpir_lower_eq_ne,
   [gpir_op_ne] = gpir_lower_eq_ne,
   [gpir_op_abs] = gpir_lower_abs,
   [gpir_op_branch_uncond] = gpir_lower_branch_uncond,
};

bool gpir_pre_rsched_lower_prog(gpir_compiler *comp)
{
   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      list_for_each_entry_safe(gpir_node, node, &block->node_list, list) {
         if (gpir_pre_rsched_lower_funcs[node->op] &&
             !gpir_pre_rsched_lower_funcs[node->op](block, node))
            return false;
      }
   }

   if (!gpir_lower_const(comp))
      return false;

   if (!gpir_lower_load(comp))
      return false;

   if (!gpir_lower_node_may_consume_two_slots(comp))
      return false;

   gpir_debug("pre rsched lower prog\n");
   gpir_node_print_prog_seq(comp);
   return true;
}

