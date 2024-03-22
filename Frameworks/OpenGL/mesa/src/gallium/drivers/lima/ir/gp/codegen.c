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
#include "codegen.h"
#include "lima_context.h"

static gpir_codegen_src gpir_get_alu_input(gpir_node *parent, gpir_node *child)
{
   static const int slot_to_src[GPIR_INSTR_SLOT_NUM][3] = {
      [GPIR_INSTR_SLOT_MUL0] = {
         gpir_codegen_src_unused, gpir_codegen_src_p1_mul_0, gpir_codegen_src_p2_mul_0 },
      [GPIR_INSTR_SLOT_MUL1] = {
         gpir_codegen_src_unused, gpir_codegen_src_p1_mul_1, gpir_codegen_src_p2_mul_1 },

      [GPIR_INSTR_SLOT_ADD0] = {
         gpir_codegen_src_unused, gpir_codegen_src_p1_acc_0, gpir_codegen_src_p2_acc_0 },
      [GPIR_INSTR_SLOT_ADD1] = {
         gpir_codegen_src_unused, gpir_codegen_src_p1_acc_1, gpir_codegen_src_p2_acc_1 },

      [GPIR_INSTR_SLOT_COMPLEX] = {
         gpir_codegen_src_unused, gpir_codegen_src_p1_complex, gpir_codegen_src_unused },
      [GPIR_INSTR_SLOT_PASS] = {
         gpir_codegen_src_unused, gpir_codegen_src_p1_pass, gpir_codegen_src_p2_pass },

      [GPIR_INSTR_SLOT_REG0_LOAD0] = {
         gpir_codegen_src_attrib_x, gpir_codegen_src_p1_attrib_x, gpir_codegen_src_unused },
      [GPIR_INSTR_SLOT_REG0_LOAD1] = {
         gpir_codegen_src_attrib_y, gpir_codegen_src_p1_attrib_y, gpir_codegen_src_unused },
      [GPIR_INSTR_SLOT_REG0_LOAD2] = {
         gpir_codegen_src_attrib_z, gpir_codegen_src_p1_attrib_z, gpir_codegen_src_unused },
      [GPIR_INSTR_SLOT_REG0_LOAD3] = {
         gpir_codegen_src_attrib_w, gpir_codegen_src_p1_attrib_w, gpir_codegen_src_unused },

      [GPIR_INSTR_SLOT_REG1_LOAD0] = {
         gpir_codegen_src_register_x, gpir_codegen_src_unused, gpir_codegen_src_unused},
      [GPIR_INSTR_SLOT_REG1_LOAD1] = {
         gpir_codegen_src_register_y, gpir_codegen_src_unused, gpir_codegen_src_unused},
      [GPIR_INSTR_SLOT_REG1_LOAD2] = {
         gpir_codegen_src_register_z, gpir_codegen_src_unused, gpir_codegen_src_unused},
      [GPIR_INSTR_SLOT_REG1_LOAD3] = {
         gpir_codegen_src_register_w, gpir_codegen_src_unused, gpir_codegen_src_unused},

      [GPIR_INSTR_SLOT_MEM_LOAD0] = {
         gpir_codegen_src_load_x, gpir_codegen_src_unused, gpir_codegen_src_unused },
      [GPIR_INSTR_SLOT_MEM_LOAD1] = {
         gpir_codegen_src_load_y, gpir_codegen_src_unused, gpir_codegen_src_unused },
      [GPIR_INSTR_SLOT_MEM_LOAD2] = {
         gpir_codegen_src_load_z, gpir_codegen_src_unused, gpir_codegen_src_unused },
      [GPIR_INSTR_SLOT_MEM_LOAD3] = {
         gpir_codegen_src_load_w, gpir_codegen_src_unused, gpir_codegen_src_unused },
   };

   int diff = child->sched.instr->index - parent->sched.instr->index;
   assert(diff < 3);
   assert(diff >= 0);

   int src = slot_to_src[child->sched.pos][diff];
   assert(src != gpir_codegen_src_unused);
   return src;
}

static void gpir_codegen_mul0_slot(gpir_codegen_instr *code, gpir_instr *instr)
{
   gpir_node *node = instr->slots[GPIR_INSTR_SLOT_MUL0];

   if (!node) {
      code->mul0_src0 = gpir_codegen_src_unused;
      code->mul0_src1 = gpir_codegen_src_unused;
      return;
   }

   gpir_alu_node *alu = gpir_node_to_alu(node);

   switch (node->op) {
   case gpir_op_mul:
      code->mul0_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->mul0_src1 = gpir_get_alu_input(node, alu->children[1]);
      if (code->mul0_src1 == gpir_codegen_src_p1_complex) {
         /* Will get confused with gpir_codegen_src_ident, so need to swap inputs */
         code->mul0_src1 = code->mul0_src0;
         code->mul0_src0 = gpir_codegen_src_p1_complex;
      }

      code->mul0_neg = alu->dest_negate;
      if (alu->children_negate[0])
         code->mul0_neg = !code->mul0_neg;
      if (alu->children_negate[1])
         code->mul0_neg = !code->mul0_neg;
      break;

   case gpir_op_neg:
      code->mul0_neg = true;
      FALLTHROUGH;
   case gpir_op_mov:
      code->mul0_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->mul0_src1 = gpir_codegen_src_ident;
      break;

   case gpir_op_complex1:
      code->mul0_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->mul0_src1 = gpir_get_alu_input(node, alu->children[1]);
      code->mul_op = gpir_codegen_mul_op_complex1;
      break;

   case gpir_op_complex2:
      code->mul0_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->mul0_src1 = code->mul0_src0;
      code->mul_op = gpir_codegen_mul_op_complex2;
      break;

   case gpir_op_select:
      code->mul0_src0 = gpir_get_alu_input(node, alu->children[2]);
      code->mul0_src1 = gpir_get_alu_input(node, alu->children[0]);
      code->mul_op = gpir_codegen_mul_op_select;
      break;

   default:
      assert(0);
   }
}

static void gpir_codegen_mul1_slot(gpir_codegen_instr *code, gpir_instr *instr)
{
   gpir_node *node = instr->slots[GPIR_INSTR_SLOT_MUL1];

   if (!node) {
      code->mul1_src0 = gpir_codegen_src_unused;
      code->mul1_src1 = gpir_codegen_src_unused;
      return;
   }

   gpir_alu_node *alu = gpir_node_to_alu(node);

   switch (node->op) {
   case gpir_op_mul:
      code->mul1_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->mul1_src1 = gpir_get_alu_input(node, alu->children[1]);
      if (code->mul1_src1 == gpir_codegen_src_p1_complex) {
         /* Will get confused with gpir_codegen_src_ident, so need to swap inputs */
         code->mul1_src1 = code->mul1_src0;
         code->mul1_src0 = gpir_codegen_src_p1_complex;
      }

      code->mul1_neg = alu->dest_negate;
      if (alu->children_negate[0])
         code->mul1_neg = !code->mul1_neg;
      if (alu->children_negate[1])
         code->mul1_neg = !code->mul1_neg;
      break;

   case gpir_op_neg:
      code->mul1_neg = true;
      FALLTHROUGH;
   case gpir_op_mov:
      code->mul1_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->mul1_src1 = gpir_codegen_src_ident;
      break;

   case gpir_op_complex1:
      code->mul1_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->mul1_src1 = gpir_get_alu_input(node, alu->children[2]);
      break;

   case gpir_op_select:
      code->mul1_src0 = gpir_get_alu_input(node, alu->children[1]);
      code->mul1_src1 = gpir_codegen_src_unused;
      break;

   default:
      assert(0);
   }
}

static void gpir_codegen_add0_slot(gpir_codegen_instr *code, gpir_instr *instr)
{
   gpir_node *node = instr->slots[GPIR_INSTR_SLOT_ADD0];

   if (!node) {
      code->acc0_src0 = gpir_codegen_src_unused;
      code->acc0_src1 = gpir_codegen_src_unused;
      return;
   }

   gpir_alu_node *alu = gpir_node_to_alu(node);

   switch (node->op) {
   case gpir_op_add:
   case gpir_op_min:
   case gpir_op_max:
   case gpir_op_lt:
   case gpir_op_ge:
      code->acc0_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->acc0_src1 = gpir_get_alu_input(node, alu->children[1]);

      code->acc0_src0_neg = alu->children_negate[0];
      code->acc0_src1_neg = alu->children_negate[1];

      switch (node->op) {
      case gpir_op_add:
         code->acc_op = gpir_codegen_acc_op_add;
         if (code->acc0_src1 == gpir_codegen_src_p1_complex) {
            code->acc0_src1 = code->acc0_src0;
            code->acc0_src0 = gpir_codegen_src_p1_complex;

            bool tmp = code->acc0_src0_neg;
            code->acc0_src0_neg = code->acc0_src1_neg;
            code->acc0_src1_neg = tmp;
         }
         break;
      case gpir_op_min:
         code->acc_op = gpir_codegen_acc_op_min;
         break;
      case gpir_op_max:
         code->acc_op = gpir_codegen_acc_op_max;
         break;
      case gpir_op_lt:
         code->acc_op = gpir_codegen_acc_op_lt;
         break;
      case gpir_op_ge:
         code->acc_op = gpir_codegen_acc_op_ge;
         break;
      default:
         assert(0);
      }

      break;

   case gpir_op_floor:
   case gpir_op_sign:
      code->acc0_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->acc0_src0_neg = alu->children_negate[0];
      switch (node->op) {
      case gpir_op_floor:
         code->acc_op = gpir_codegen_acc_op_floor;
         break;
      case gpir_op_sign:
         code->acc_op = gpir_codegen_acc_op_sign;
         break;
      default:
         assert(0);
      }
      break;

   case gpir_op_neg:
      code->acc0_src0_neg = true;
      FALLTHROUGH;
   case gpir_op_mov:
      code->acc_op = gpir_codegen_acc_op_add;
      code->acc0_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->acc0_src1 = gpir_codegen_src_ident;
      code->acc0_src1_neg = true;
      break;

   default:
      assert(0);
   }
}

static void gpir_codegen_add1_slot(gpir_codegen_instr *code, gpir_instr *instr)
{
   gpir_node *node = instr->slots[GPIR_INSTR_SLOT_ADD1];

   if (!node) {
      code->acc1_src0 = gpir_codegen_src_unused;
      code->acc1_src1 = gpir_codegen_src_unused;
      return;
   }

   gpir_alu_node *alu = gpir_node_to_alu(node);

   switch (node->op) {
   case gpir_op_add:
   case gpir_op_min:
   case gpir_op_max:
   case gpir_op_lt:
   case gpir_op_ge:
      code->acc1_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->acc1_src1 = gpir_get_alu_input(node, alu->children[1]);

      code->acc1_src0_neg = alu->children_negate[0];
      code->acc1_src1_neg = alu->children_negate[1];

      switch (node->op) {
      case gpir_op_add:
         code->acc_op = gpir_codegen_acc_op_add;
         if (code->acc1_src1 == gpir_codegen_src_p1_complex) {
            code->acc1_src1 = code->acc1_src0;
            code->acc1_src0 = gpir_codegen_src_p1_complex;

            bool tmp = code->acc1_src0_neg;
            code->acc1_src0_neg = code->acc1_src1_neg;
            code->acc1_src1_neg = tmp;
         }
         break;
      case gpir_op_min:
         code->acc_op = gpir_codegen_acc_op_min;
         break;
      case gpir_op_max:
         code->acc_op = gpir_codegen_acc_op_max;
         break;
      case gpir_op_lt:
         code->acc_op = gpir_codegen_acc_op_lt;
         break;
      case gpir_op_ge:
         code->acc_op = gpir_codegen_acc_op_ge;
         break;
      default:
         assert(0);
      }

      break;

   case gpir_op_floor:
   case gpir_op_sign:
      code->acc1_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->acc1_src0_neg = alu->children_negate[0];
      switch (node->op) {
      case gpir_op_floor:
         code->acc_op = gpir_codegen_acc_op_floor;
         break;
      case gpir_op_sign:
         code->acc_op = gpir_codegen_acc_op_sign;
         break;
      default:
         assert(0);
      }
      break;

   case gpir_op_neg:
      code->acc1_src0_neg = true;
      FALLTHROUGH;
   case gpir_op_mov:
      code->acc_op = gpir_codegen_acc_op_add;
      code->acc1_src0 = gpir_get_alu_input(node, alu->children[0]);
      code->acc1_src1 = gpir_codegen_src_ident;
      code->acc1_src1_neg = true;
      break;

   default:
      assert(0);
   }
}

static void gpir_codegen_complex_slot(gpir_codegen_instr *code, gpir_instr *instr)
{
   gpir_node *node = instr->slots[GPIR_INSTR_SLOT_COMPLEX];

   if (!node) {
      code->complex_src = gpir_codegen_src_unused;
      return;
   }

   switch (node->op) {
   case gpir_op_mov:
   case gpir_op_rcp_impl:
   case gpir_op_rsqrt_impl:
   case gpir_op_exp2_impl:
   case gpir_op_log2_impl:
   {
      gpir_alu_node *alu = gpir_node_to_alu(node);
      code->complex_src = gpir_get_alu_input(node, alu->children[0]);
      break;
   }
   default:
      assert(0);
   }

   switch (node->op) {
   case gpir_op_mov:
      code->complex_op = gpir_codegen_complex_op_pass;
      break;
   case gpir_op_rcp_impl:
      code->complex_op = gpir_codegen_complex_op_rcp;
      break;
   case gpir_op_rsqrt_impl:
      code->complex_op = gpir_codegen_complex_op_rsqrt;
      break;
   case gpir_op_exp2_impl:
      code->complex_op = gpir_codegen_complex_op_exp2;
      break;
   case gpir_op_log2_impl:
      code->complex_op = gpir_codegen_complex_op_log2;
      break;
   default:
      assert(0);
   }
}

static void gpir_codegen_pass_slot(gpir_codegen_instr *code, gpir_instr *instr)
{
   gpir_node *node = instr->slots[GPIR_INSTR_SLOT_PASS];

   if (!node) {
      code->pass_op = gpir_codegen_pass_op_pass;
      code->pass_src = gpir_codegen_src_unused;
      return;
   }

   if (node->op == gpir_op_branch_cond) {
      gpir_branch_node *branch = gpir_node_to_branch(node);

      code->pass_op = gpir_codegen_pass_op_pass;
      code->pass_src = gpir_get_alu_input(node, branch->cond);

      /* Fill out branch information */
      unsigned offset = branch->dest->instr_offset;
      assert(offset < 0x200);
      code->branch = true;
      code->branch_target = offset & 0xff;
      code->branch_target_lo = !(offset >> 8);
      code->unknown_1 = 13;
      return;
   }

   gpir_alu_node *alu = gpir_node_to_alu(node);
   code->pass_src = gpir_get_alu_input(node, alu->children[0]);

   switch (node->op) {
   case gpir_op_mov:
      code->pass_op = gpir_codegen_pass_op_pass;
      break;
   case gpir_op_preexp2:
      code->pass_op = gpir_codegen_pass_op_preexp2;
      break;
   case gpir_op_postlog2:
      code->pass_op = gpir_codegen_pass_op_postlog2;
      break;
   default:
      assert(0);
   }

}

static void gpir_codegen_reg0_slot(gpir_codegen_instr *code, gpir_instr *instr)
{
   if (!instr->reg0_use_count)
      return;

   code->register0_attribute = instr->reg0_is_attr;
   code->register0_addr = instr->reg0_index;
}

static void gpir_codegen_reg1_slot(gpir_codegen_instr *code, gpir_instr *instr)
{
   if (!instr->reg1_use_count)
      return;

   code->register1_addr = instr->reg1_index;
}

static void gpir_codegen_mem_slot(gpir_codegen_instr *code, gpir_instr *instr)
{
   if (!instr->mem_use_count) {
      code->load_offset = gpir_codegen_load_off_none;
      return;
   }

   code->load_addr = instr->mem_index;
   code->load_offset = gpir_codegen_load_off_none;
}

static gpir_codegen_store_src gpir_get_store_input(gpir_node *node)
{
   static int slot_to_src[GPIR_INSTR_SLOT_NUM] = {
      [GPIR_INSTR_SLOT_MUL0] = gpir_codegen_store_src_mul_0,
      [GPIR_INSTR_SLOT_MUL1] = gpir_codegen_store_src_mul_1,
      [GPIR_INSTR_SLOT_ADD0] = gpir_codegen_store_src_acc_0,
      [GPIR_INSTR_SLOT_ADD1] = gpir_codegen_store_src_acc_1,
      [GPIR_INSTR_SLOT_COMPLEX] = gpir_codegen_store_src_complex,
      [GPIR_INSTR_SLOT_PASS] = gpir_codegen_store_src_pass,
      [GPIR_INSTR_SLOT_REG0_LOAD0...GPIR_INSTR_SLOT_STORE3] = gpir_codegen_store_src_none,
   };

   gpir_store_node *store = gpir_node_to_store(node);
   return slot_to_src[store->child->sched.pos];
}

static void gpir_codegen_store_slot(gpir_codegen_instr *code, gpir_instr *instr)
{

   gpir_node *node = instr->slots[GPIR_INSTR_SLOT_STORE0];
   if (node)
      code->store0_src_x = gpir_get_store_input(node);
   else
      code->store0_src_x = gpir_codegen_store_src_none;

   node = instr->slots[GPIR_INSTR_SLOT_STORE1];
   if (node)
      code->store0_src_y = gpir_get_store_input(node);
   else
      code->store0_src_y = gpir_codegen_store_src_none;

   node = instr->slots[GPIR_INSTR_SLOT_STORE2];
   if (node)
      code->store1_src_z = gpir_get_store_input(node);
   else
      code->store1_src_z = gpir_codegen_store_src_none;

   node = instr->slots[GPIR_INSTR_SLOT_STORE3];
   if (node)
      code->store1_src_w = gpir_get_store_input(node);
   else
      code->store1_src_w = gpir_codegen_store_src_none;

   if (instr->store_content[0] == GPIR_INSTR_STORE_TEMP) {
      code->store0_temporary = true;
      code->unknown_1 = 12;
   }
   else {
      code->store0_varying = instr->store_content[0] == GPIR_INSTR_STORE_VARYING;
      code->store0_addr = instr->store_index[0];
   }

   if (instr->store_content[1] == GPIR_INSTR_STORE_TEMP) {
      code->store1_temporary = true;
      code->unknown_1 = 12;
   }
   else {
      code->store1_varying = instr->store_content[1] == GPIR_INSTR_STORE_VARYING;
      code->store1_addr = instr->store_index[1];
   }
}

static void gpir_codegen(gpir_codegen_instr *code, gpir_instr *instr)
{
   gpir_codegen_mul0_slot(code, instr);
   gpir_codegen_mul1_slot(code, instr);

   gpir_codegen_add0_slot(code, instr);
   gpir_codegen_add1_slot(code, instr);

   gpir_codegen_complex_slot(code, instr);
   gpir_codegen_pass_slot(code, instr);

   gpir_codegen_reg0_slot(code, instr);
   gpir_codegen_reg1_slot(code, instr);
   gpir_codegen_mem_slot(code, instr);

   gpir_codegen_store_slot(code, instr);
}

static void gpir_codegen_print_prog(gpir_compiler *comp)
{
   uint32_t *data = comp->prog->shader;
   int num_dword_per_instr = sizeof(gpir_codegen_instr) / sizeof(uint32_t);

   for (int i = 0; i < comp->num_instr; i++) {
      printf("%03d: ", i);
      for (int j = 0; j < num_dword_per_instr; j++)
         printf("%08x ", data[i * num_dword_per_instr + j]);
      printf("\n");
   }
}

bool gpir_codegen_prog(gpir_compiler *comp)
{
   int num_instr = 0;
   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      block->instr_offset = num_instr;
      num_instr += list_length(&block->instr_list);
   }

   assert(num_instr <= 512);

   gpir_codegen_instr *code = rzalloc_array(comp->prog, gpir_codegen_instr, num_instr);
   if (!code)
      return false;

   int instr_index = 0;
   list_for_each_entry(gpir_block, block, &comp->block_list, list) {
      list_for_each_entry(gpir_instr, instr, &block->instr_list, list) {
         gpir_codegen(code + instr_index, instr);
         instr_index++;
      }
   }

   for (int i = 0; i < num_instr; i++) {
      if (code[i].register0_attribute)
         comp->prog->state.prefetch = i;
   }

   comp->prog->shader = code;
   comp->prog->state.shader_size = num_instr * sizeof(gpir_codegen_instr);

   if (lima_debug & LIMA_DEBUG_GP) {
      gpir_codegen_print_prog(comp);
      gpir_disassemble_program(code, num_instr, stdout);
   }

   return true;
}

static gpir_codegen_acc_op gpir_codegen_get_acc_op(gpir_op op)
{
   switch (op) {
   case gpir_op_add:
   case gpir_op_neg:
   case gpir_op_mov:
      return gpir_codegen_acc_op_add;
   case gpir_op_min:
      return gpir_codegen_acc_op_min;
   case gpir_op_max:
      return gpir_codegen_acc_op_max;
   case gpir_op_lt:
      return gpir_codegen_acc_op_lt;
   case gpir_op_ge:
      return gpir_codegen_acc_op_ge;
   case gpir_op_floor:
      return gpir_codegen_acc_op_floor;
   case gpir_op_sign:
      return gpir_codegen_acc_op_sign;
   default:
      assert(0);
   }
   return -1;
}

bool gpir_codegen_acc_same_op(gpir_op op1, gpir_op op2)
{
   return gpir_codegen_get_acc_op(op1) == gpir_codegen_get_acc_op(op2);
}
