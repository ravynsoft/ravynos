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

#include "util/u_math.h"
#include "util/ralloc.h"

#include "gpir.h"

const gpir_op_info gpir_op_infos[] = {
   [gpir_op_unsupported] = {
      .name = "unsupported",
   },
   [gpir_op_mov] = {
      .name = "mov",
      .slots = (int []) {
         GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_MUL1,
         GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_MUL0,
         GPIR_INSTR_SLOT_PASS, GPIR_INSTR_SLOT_COMPLEX,
         GPIR_INSTR_SLOT_END
      },
   },
   [gpir_op_mul] = {
      .name = "mul",
      .dest_neg = true,
      .slots = (int []) { GPIR_INSTR_SLOT_MUL1, GPIR_INSTR_SLOT_MUL0, GPIR_INSTR_SLOT_END },
   },
   [gpir_op_select] = {
      .name = "select",
      .dest_neg = true,
      .slots = (int []) { GPIR_INSTR_SLOT_MUL0, GPIR_INSTR_SLOT_END },
      .may_consume_two_slots = true,
   },
   [gpir_op_complex1] = {
      .name = "complex1",
      .slots = (int []) { GPIR_INSTR_SLOT_MUL0, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .may_consume_two_slots = true,
   },
   [gpir_op_complex2] = {
      .name = "complex2",
      .slots = (int []) { GPIR_INSTR_SLOT_MUL0, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .schedule_first = true,
   },
   [gpir_op_add] = {
      .name = "add",
      .src_neg = {true, true, false, false},
      .slots = (int []) { GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_END },
   },
   [gpir_op_floor] = {
      .name = "floor",
      .src_neg = {true, false, false, false},
      .slots = (int []) { GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .may_consume_two_slots = true,
   },
   [gpir_op_sign] = {
      .name = "sign",
      .src_neg = {true, false, false, false},
      .slots = (int []) { GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .may_consume_two_slots = true,
   },
   [gpir_op_ge] = {
      .name = "ge",
      .src_neg = {true, true, false, false},
      .slots = (int []) { GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .may_consume_two_slots = true,
   },
   [gpir_op_lt] = {
      .name = "lt",
      .src_neg = {true, true, false, false},
      .slots = (int []) { GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .may_consume_two_slots = true,
   },
   [gpir_op_min] = {
      .name = "min",
      .src_neg = {true, true, false, false},
      .slots = (int []) { GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .may_consume_two_slots = true,
   },
   [gpir_op_max] = {
      .name = "max",
      .src_neg = {true, true, false, false},
      .slots = (int []) { GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .may_consume_two_slots = true,
   },
   [gpir_op_abs] = {
      .name = "abs",
      .src_neg = {true, true, false, false},
   },
   [gpir_op_neg] = {
      .name = "neg",
      .slots = (int []) {
         GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_MUL1,
         GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_MUL0,
         GPIR_INSTR_SLOT_END
      },
   },
   [gpir_op_not] = {
      .name = "not",
      .src_neg = {true, true, false, false},
      .slots = (int []) { GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_END },
   },
   [gpir_op_eq] = {
      .name = "eq",
      .slots = (int []) {
         GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_END
      },
   },
   [gpir_op_ne] = {
      .name = "ne",
      .slots = (int []) {
         GPIR_INSTR_SLOT_ADD0, GPIR_INSTR_SLOT_ADD1, GPIR_INSTR_SLOT_END
      },
   },
   [gpir_op_clamp_const] = {
      .name = "clamp_const",
   },
   [gpir_op_preexp2] = {
      .name = "preexp2",
      .slots = (int []) { GPIR_INSTR_SLOT_PASS, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .schedule_first = true,
   },
   [gpir_op_postlog2] = {
      .name = "postlog2",
      .slots = (int []) { GPIR_INSTR_SLOT_PASS, GPIR_INSTR_SLOT_END },
   },
   [gpir_op_exp2_impl] = {
      .name = "exp2_impl",
      .slots = (int []) { GPIR_INSTR_SLOT_COMPLEX, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .schedule_first = true,
   },
   [gpir_op_log2_impl] = {
      .name = "log2_impl",
      .slots = (int []) { GPIR_INSTR_SLOT_COMPLEX, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .schedule_first = true,
   },
   [gpir_op_rcp_impl] = {
      .name = "rcp_impl",
      .slots = (int []) { GPIR_INSTR_SLOT_COMPLEX, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .schedule_first = true,
   },
   [gpir_op_rsqrt_impl] = {
      .name = "rsqrt_impl",
      .slots = (int []) { GPIR_INSTR_SLOT_COMPLEX, GPIR_INSTR_SLOT_END },
      .spillless = true,
      .schedule_first = true,
   },
   [gpir_op_load_uniform] = {
      .name = "ld_uni",
      .slots = (int []) {
         GPIR_INSTR_SLOT_MEM_LOAD0, GPIR_INSTR_SLOT_MEM_LOAD1,
         GPIR_INSTR_SLOT_MEM_LOAD2, GPIR_INSTR_SLOT_MEM_LOAD3,
         GPIR_INSTR_SLOT_END
      },
      .type = gpir_node_type_load,
   },
   [gpir_op_load_temp] = {
      .name = "ld_tmp",
      .type = gpir_node_type_load,
   },
   [gpir_op_load_attribute] = {
      .name = "ld_att",
      .slots = (int []) {
         GPIR_INSTR_SLOT_REG0_LOAD0, GPIR_INSTR_SLOT_REG0_LOAD1,
         GPIR_INSTR_SLOT_REG0_LOAD2, GPIR_INSTR_SLOT_REG0_LOAD3,
         GPIR_INSTR_SLOT_END
      },
      .type = gpir_node_type_load,
   },
   [gpir_op_load_reg] = {
      .name = "ld_reg",
      .slots = (int []) {
         GPIR_INSTR_SLOT_REG1_LOAD0, GPIR_INSTR_SLOT_REG1_LOAD1,
         GPIR_INSTR_SLOT_REG1_LOAD2, GPIR_INSTR_SLOT_REG1_LOAD3,
         GPIR_INSTR_SLOT_REG0_LOAD0, GPIR_INSTR_SLOT_REG0_LOAD1,
         GPIR_INSTR_SLOT_REG0_LOAD2, GPIR_INSTR_SLOT_REG0_LOAD3,
         GPIR_INSTR_SLOT_END
      },
      .type = gpir_node_type_load,
      .spillless = true,
   },
   [gpir_op_store_temp] = {
      .name = "st_tmp",
      .type = gpir_node_type_store,
   },
   [gpir_op_store_reg] = {
      .name = "st_reg",
      .slots = (int []) {
         GPIR_INSTR_SLOT_STORE0, GPIR_INSTR_SLOT_STORE1,
         GPIR_INSTR_SLOT_STORE2, GPIR_INSTR_SLOT_STORE3,
         GPIR_INSTR_SLOT_END
      },
      .type = gpir_node_type_store,
      .spillless = true,
   },
   [gpir_op_store_varying] = {
      .name = "st_var",
      .slots = (int []) {
         GPIR_INSTR_SLOT_STORE0, GPIR_INSTR_SLOT_STORE1,
         GPIR_INSTR_SLOT_STORE2, GPIR_INSTR_SLOT_STORE3,
         GPIR_INSTR_SLOT_END
      },
      .type = gpir_node_type_store,
      .spillless = true,
   },
   [gpir_op_store_temp_load_off0] = {
      .name = "st_of0",
      .type = gpir_node_type_store,
   },
   [gpir_op_store_temp_load_off1] = {
      .name = "st_of1",
      .type = gpir_node_type_store,
   },
   [gpir_op_store_temp_load_off2] = {
      .name = "st_of2",
      .type = gpir_node_type_store,
   },
   [gpir_op_branch_cond] = {
      .name = "branch_cond",
      .type = gpir_node_type_branch,
      .schedule_first = true,
      .slots = (int []) { GPIR_INSTR_SLOT_PASS, GPIR_INSTR_SLOT_END },
   },
   [gpir_op_const] = {
      .name = "const",
      .type = gpir_node_type_const,
   },
   [gpir_op_exp2] = {
      .name = "exp2",
   },
   [gpir_op_log2] = {
      .name = "log2",
   },
   [gpir_op_rcp] = {
      .name = "rcp",
   },
   [gpir_op_rsqrt] = {
      .name = "rsqrt",
   },
   [gpir_op_ceil] = {
      .name = "ceil",
   },
   [gpir_op_exp] = {
      .name = "exp",
   },
   [gpir_op_log] = {
      .name = "log",
   },
   [gpir_op_sin] = {
      .name = "sin",
   },
   [gpir_op_cos] = {
      .name = "cos",
   },
   [gpir_op_tan] = {
      .name = "tan",
   },
   [gpir_op_dummy_f] = {
      .name = "dummy_f",
      .type = gpir_node_type_alu,
      .spillless = true,
   },
   [gpir_op_dummy_m] = {
      .name = "dummy_m",
      .type = gpir_node_type_alu,
   },
   [gpir_op_branch_uncond] = {
      .name = "branch_uncond",
      .type = gpir_node_type_branch,
   },
};

void *gpir_node_create(gpir_block *block, gpir_op op)
{
   static const int node_size[] = {
      [gpir_node_type_alu] = sizeof(gpir_alu_node),
      [gpir_node_type_const] = sizeof(gpir_const_node),
      [gpir_node_type_load] = sizeof(gpir_load_node),
      [gpir_node_type_store] = sizeof(gpir_store_node),
      [gpir_node_type_branch] = sizeof(gpir_branch_node),
   };

   gpir_node_type type = gpir_op_infos[op].type;
   int size = node_size[type];
   gpir_node *node = rzalloc_size(block, size);
   if (unlikely(!node))
      return NULL;

   snprintf(node->name, sizeof(node->name), "new");

   list_inithead(&node->succ_list);
   list_inithead(&node->pred_list);

   node->op = op;
   node->type = type;
   node->index = block->comp->cur_index++;
   node->block = block;

   return node;
}

gpir_dep *gpir_node_add_dep(gpir_node *succ, gpir_node *pred, int type)
{
   /* don't add dep for two nodes from different block */
   if (succ->block != pred->block)
      return NULL;

   /* don't add self loop dep */
   if (succ == pred)
      return NULL;

   /* don't add duplicated dep */
   gpir_node_foreach_pred(succ, dep) {
      if (dep->pred == pred) {
         /* use stronger dependency */
         if (dep->type > type)
            dep->type = type;
         return dep;
      }
   }

   gpir_dep *dep = ralloc(succ, gpir_dep);
   dep->type = type;
   dep->pred = pred;
   dep->succ = succ;
   list_addtail(&dep->pred_link, &succ->pred_list);
   list_addtail(&dep->succ_link, &pred->succ_list);
   return dep;
}

void gpir_node_remove_dep(gpir_node *succ, gpir_node *pred)
{
   gpir_node_foreach_pred(succ, dep) {
      if (dep->pred == pred) {
         list_del(&dep->succ_link);
         list_del(&dep->pred_link);
         ralloc_free(dep);
         return;
      }
   }
}

void gpir_node_replace_child(gpir_node *parent, gpir_node *old_child,
                             gpir_node *new_child)
{
   if (parent->type == gpir_node_type_alu) {
      gpir_alu_node *alu = gpir_node_to_alu(parent);
      for (int i = 0; i < alu->num_child; i++) {
         if (alu->children[i] == old_child)
            alu->children[i] = new_child;
      }
   }
   else if (parent->type == gpir_node_type_store) {
      gpir_store_node *store = gpir_node_to_store(parent);
      if (store->child == old_child)
         store->child = new_child;
   } else if (parent->type == gpir_node_type_branch) {
      gpir_branch_node *branch = gpir_node_to_branch(parent);
      if (branch->cond == old_child)
         branch->cond = new_child;
   }
}

void gpir_node_replace_pred(gpir_dep *dep, gpir_node *new_pred)
{
   list_del(&dep->succ_link);
   dep->pred = new_pred;
   list_addtail(&dep->succ_link, &new_pred->succ_list);
}

void gpir_node_replace_succ(gpir_node *dst, gpir_node *src)
{
   gpir_node_foreach_succ_safe(src, dep) {
      if (dep->type != GPIR_DEP_INPUT)
         continue;

      gpir_node_replace_pred(dep, dst);
      gpir_node_replace_child(dep->succ, src, dst);
   }
}

void gpir_node_insert_child(gpir_node *parent, gpir_node *child,
                            gpir_node *insert_child)
{
   gpir_node_foreach_pred(parent, dep) {
      if (dep->pred == child) {
         gpir_node_replace_pred(dep, insert_child);
         gpir_node_replace_child(parent, child, insert_child);
         break;
      }
   }
}

void gpir_node_delete(gpir_node *node)
{
   gpir_node_foreach_succ_safe(node, dep) {
      list_del(&dep->succ_link);
      list_del(&dep->pred_link);
      ralloc_free(dep);
   }

   gpir_node_foreach_pred_safe(node, dep) {
      list_del(&dep->succ_link);
      list_del(&dep->pred_link);
      ralloc_free(dep);
   }

   list_del(&node->list);
   ralloc_free(node);
}

static void gpir_node_print_node(gpir_node *node, int type, int space)
{
   static char *dep_name[] = {
      [GPIR_DEP_INPUT] = "input",
      [GPIR_DEP_OFFSET] = "offset",
      [GPIR_DEP_READ_AFTER_WRITE] = "RaW",
      [GPIR_DEP_WRITE_AFTER_READ] = "WaR",
   };

   for (int i = 0; i < space; i++)
      printf(" ");
   printf("%s%s %d %s %s\n", node->printed && !gpir_node_is_leaf(node) ? "+" : "",
          gpir_op_infos[node->op].name, node->index, node->name, dep_name[type]);

   if (!node->printed) {
      gpir_node_foreach_pred(node, dep) {
         gpir_node_print_node(dep->pred, dep->type, space + 2);
      }

      node->printed = true;
   }
}

void gpir_node_print_prog_dep(gpir_compiler *comp)
{
   if (!(lima_debug & LIMA_DEBUG_GP))
      return;

   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      list_for_each_entry(gpir_node, node, &block->node_list, list) {
         node->printed = false;
      }
   }

   printf("======== node prog dep ========\n");
   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      list_for_each_entry(gpir_node, node, &block->node_list, list) {
         if (gpir_node_is_root(node))
            gpir_node_print_node(node, GPIR_DEP_INPUT, 0);
      }
      printf("----------------------------\n");
   }
}

void gpir_node_print_prog_seq(gpir_compiler *comp)
{
   if (!(lima_debug & LIMA_DEBUG_GP))
      return;

   int index = 0;
   printf("======== node prog seq ========\n");
   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      list_for_each_entry(gpir_node, node, &block->node_list, list) {
         printf("%03d: %s %d %s pred", index++, gpir_op_infos[node->op].name,
                node->index, node->name);
         gpir_node_foreach_pred(node, dep) {
            printf(" %d", dep->pred->index);
         }
         printf(" succ");
         gpir_node_foreach_succ(node, dep) {
            printf(" %d", dep->succ->index);
         }
         printf("\n");
      }
      printf("----------------------------\n");
   }
}
