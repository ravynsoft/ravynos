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
#include "util/half_float.h"
#include "util/bitscan.h"

#include "ppir.h"
#include "codegen.h"
#include "lima_context.h"

static unsigned encode_swizzle(uint8_t *swizzle, int shift, int dest_shift)
{
   unsigned ret = 0;
   for (int i = 0; i < 4; i++)
      ret |= ((swizzle[i] + shift) & 0x3) << ((i + dest_shift) * 2);
   return ret;
}

static int get_scl_reg_index(ppir_src *src, int component)
{
   int ret = ppir_target_get_src_reg_index(src);
   ret += src->swizzle[component];
   return ret;
}

static void ppir_codegen_encode_varying(ppir_node *node, void *code)
{
   ppir_codegen_field_varying *f = code;
   ppir_load_node *load = ppir_node_to_load(node);
   ppir_dest *dest = &load->dest;
   int index = ppir_target_get_dest_reg_index(dest);
   int num_components = load->num_components;

   if (node->op != ppir_op_load_coords_reg) {
      assert(node->op == ppir_op_load_varying ||
             node->op == ppir_op_load_coords ||
             node->op == ppir_op_load_fragcoord ||
             node->op == ppir_op_load_pointcoord ||
             node->op == ppir_op_load_frontface);

      f->imm.dest = index >> 2;
      f->imm.mask = dest->write_mask << (index & 0x3);

      int alignment = num_components == 3 ? 3 : num_components - 1;
      f->imm.alignment = alignment;

      if (load->num_src) {
         index = ppir_target_get_src_reg_index(&load->src);
         f->imm.offset_vector = index >> 2;
         f->imm.offset_scalar = index & 0x3;
      } else
         f->imm.offset_vector = 0xf;

      if (alignment == 3)
         f->imm.index = load->index >> 2;
      else
         f->imm.index = load->index >> alignment;

      switch (node->op) {
         case ppir_op_load_fragcoord:
            f->imm.source_type = 2;
            f->imm.perspective = 3;
            break;
         case ppir_op_load_pointcoord:
            f->imm.source_type = 3;
            break;
         case ppir_op_load_frontface:
            f->imm.source_type = 3;
            f->imm.perspective = 1;
            break;
         case ppir_op_load_coords:
            if (load->sampler_dim == GLSL_SAMPLER_DIM_CUBE)
               f->imm.source_type = 2;

            switch (load->perspective) {
            case ppir_perspective_none:
               f->imm.perspective = 0;
               break;
            case ppir_perspective_z:
               f->imm.perspective = 2;
               break;
            case ppir_perspective_w:
               f->imm.perspective = 3;
               break;
            }
            break;
         default:
            break;
      }
   }
   else {  /* node->op == ppir_op_load_coords_reg */
      f->reg.dest = index >> 2;
      f->reg.mask = dest->write_mask << (index & 0x3);

      if (load->num_src) {
         if (load->sampler_dim == GLSL_SAMPLER_DIM_CUBE) {
            f->reg.source_type = 2;
            f->reg.perspective = 1;
         } else {
            f->reg.source_type = 1;
            switch (load->perspective) {
            case ppir_perspective_none:
               f->reg.perspective = 0;
               break;
            case ppir_perspective_z:
               f->reg.perspective = 2;
               break;
            case ppir_perspective_w:
               f->reg.perspective = 3;
               break;
            }
         }
         ppir_src *src = &load->src;
         index = ppir_target_get_src_reg_index(src);
         f->reg.source = index >> 2;
         f->reg.negate = src->negate;
         f->reg.absolute = src->absolute;
         f->reg.swizzle = encode_swizzle(src->swizzle, index & 0x3, 0);
      }
   }
}

static void ppir_codegen_encode_texld(ppir_node *node, void *code)
{
   ppir_codegen_field_sampler *f = code;
   ppir_load_texture_node *ldtex = ppir_node_to_load_texture(node);

   f->index = ldtex->sampler;

   f->lod_bias_en = ldtex->lod_bias_en;
   f->explicit_lod = ldtex->explicit_lod;
   if (ldtex->lod_bias_en)
      f->lod_bias = ppir_target_get_src_reg_index(&ldtex->src[1]);

   switch (ldtex->sampler_dim) {
   case GLSL_SAMPLER_DIM_2D:
   case GLSL_SAMPLER_DIM_3D:
   case GLSL_SAMPLER_DIM_RECT:
   case GLSL_SAMPLER_DIM_EXTERNAL:
      f->type = ppir_codegen_sampler_type_generic;
      break;
   case GLSL_SAMPLER_DIM_CUBE:
      f->type = ppir_codegen_sampler_type_cube;
      break;
   default:
      break;
   }

   f->offset_en = 0;
   f->unknown_2 = 0x39001;
}

static void ppir_codegen_encode_uniform(ppir_node *node, void *code)
{
   ppir_codegen_field_uniform *f = code;
   ppir_load_node *load = ppir_node_to_load(node);

   switch (node->op) {
      case ppir_op_load_uniform:
         f->source = ppir_codegen_uniform_src_uniform;
         break;
      case ppir_op_load_temp:
         f->source = ppir_codegen_uniform_src_temporary;
         break;
      default:
         assert(0);
   }

   /* Uniforms are always aligned to vec4 boundary */
   f->alignment = 2;
   f->index = load->index;

   if (load->num_src) {
      f->offset_en = 1;
      f->offset_reg = ppir_target_get_src_reg_index(&load->src);
   }
}

static ppir_codegen_outmod ppir_codegen_get_outmod(ppir_outmod outmod)
{
   switch (outmod) {
      case ppir_outmod_none:
         return ppir_codegen_outmod_none;
      case ppir_outmod_clamp_fraction:
         return ppir_codegen_outmod_clamp_fraction;
      case ppir_outmod_clamp_positive:
         return ppir_codegen_outmod_clamp_positive;
      case ppir_outmod_round:
         return ppir_codegen_outmod_round;
      default:
         unreachable("invalid ppir_outmod");
   }
}

static unsigned shift_to_op(int shift)
{
   assert(shift >= -3 && shift <= 3);
   return shift < 0 ? shift + 8 : shift;
}

static void ppir_codegen_encode_vec_mul(ppir_node *node, void *code)
{
   ppir_codegen_field_vec4_mul *f = code;
   ppir_alu_node *alu = ppir_node_to_alu(node);

   ppir_dest *dest = &alu->dest;
   int dest_shift = 0;
   if (dest->type != ppir_target_pipeline) {
      int index = ppir_target_get_dest_reg_index(dest);
      dest_shift = index & 0x3;
      f->dest = index >> 2;
      f->mask = dest->write_mask << dest_shift;
   }
   f->dest_modifier = ppir_codegen_get_outmod(dest->modifier);

   switch (node->op) {
   case ppir_op_mul:
      f->op = shift_to_op(alu->shift);
      break;
   case ppir_op_mov:
      f->op = ppir_codegen_vec4_mul_op_mov;
      break;
   case ppir_op_max:
      f->op = ppir_codegen_vec4_mul_op_max;
      break;
   case ppir_op_min:
      f->op = ppir_codegen_vec4_mul_op_min;
      break;
   case ppir_op_and:
      f->op = ppir_codegen_vec4_mul_op_and;
      break;
   case ppir_op_or:
      f->op = ppir_codegen_vec4_mul_op_or;
      break;
   case ppir_op_xor:
      f->op = ppir_codegen_vec4_mul_op_xor;
      break;
   case ppir_op_gt:
      f->op = ppir_codegen_vec4_mul_op_gt;
      break;
   case ppir_op_ge:
      f->op = ppir_codegen_vec4_mul_op_ge;
      break;
   case ppir_op_eq:
      f->op = ppir_codegen_vec4_mul_op_eq;
      break;
   case ppir_op_ne:
      f->op = ppir_codegen_vec4_mul_op_ne;
      break;
   case ppir_op_not:
      f->op = ppir_codegen_vec4_mul_op_not;
      break;
   default:
      break;
   }

   ppir_src *src = alu->src;
   int index = ppir_target_get_src_reg_index(src);
   f->arg0_source = index >> 2;
   f->arg0_swizzle = encode_swizzle(src->swizzle, index & 0x3, dest_shift);
   f->arg0_absolute = src->absolute;
   f->arg0_negate = src->negate;

   if (alu->num_src == 2) {
      src = alu->src + 1;
      index = ppir_target_get_src_reg_index(src);
      f->arg1_source = index >> 2;
      f->arg1_swizzle = encode_swizzle(src->swizzle, index & 0x3, dest_shift);
      f->arg1_absolute = src->absolute;
      f->arg1_negate = src->negate;
   }
}

static void ppir_codegen_encode_scl_mul(ppir_node *node, void *code)
{
   ppir_codegen_field_float_mul *f = code;
   ppir_alu_node *alu = ppir_node_to_alu(node);

   ppir_dest *dest = &alu->dest;
   int dest_component = ffs(dest->write_mask) - 1;
   assert(dest_component >= 0);

   if (dest->type != ppir_target_pipeline) {
      f->dest = ppir_target_get_dest_reg_index(dest) + dest_component;
      f->output_en = true;
   }
   f->dest_modifier = ppir_codegen_get_outmod(dest->modifier);

   switch (node->op) {
   case ppir_op_mul:
      f->op = shift_to_op(alu->shift);
      break;
   case ppir_op_mov:
      f->op = ppir_codegen_float_mul_op_mov;
      break;
   case ppir_op_max:
      f->op = ppir_codegen_float_mul_op_max;
      break;
   case ppir_op_min:
      f->op = ppir_codegen_float_mul_op_min;
      break;
   case ppir_op_and:
      f->op = ppir_codegen_float_mul_op_and;
      break;
   case ppir_op_or:
      f->op = ppir_codegen_float_mul_op_or;
      break;
   case ppir_op_xor:
      f->op = ppir_codegen_float_mul_op_xor;
      break;
   case ppir_op_gt:
      f->op = ppir_codegen_float_mul_op_gt;
      break;
   case ppir_op_ge:
      f->op = ppir_codegen_float_mul_op_ge;
      break;
   case ppir_op_eq:
      f->op = ppir_codegen_float_mul_op_eq;
      break;
   case ppir_op_ne:
      f->op = ppir_codegen_float_mul_op_ne;
      break;
   case ppir_op_not:
      f->op = ppir_codegen_float_mul_op_not;
      break;
   default:
      break;
   }

   ppir_src *src = alu->src;
   f->arg0_source = get_scl_reg_index(src, dest_component);
   f->arg0_absolute = src->absolute;
   f->arg0_negate = src->negate;

   if (alu->num_src == 2) {
      src = alu->src + 1;
      f->arg1_source = get_scl_reg_index(src, dest_component);
      f->arg1_absolute = src->absolute;
      f->arg1_negate = src->negate;
   }
}

static void ppir_codegen_encode_vec_add(ppir_node *node, void *code)
{
   ppir_codegen_field_vec4_acc *f = code;
   ppir_alu_node *alu = ppir_node_to_alu(node);

   ppir_dest *dest = &alu->dest;
   int index = ppir_target_get_dest_reg_index(dest);
   int dest_shift = index & 0x3;
   f->dest = index >> 2;
   f->mask = dest->write_mask << dest_shift;
   f->dest_modifier = ppir_codegen_get_outmod(dest->modifier);

   switch (node->op) {
   case ppir_op_add:
      f->op = ppir_codegen_vec4_acc_op_add;
      break;
   case ppir_op_mov:
      f->op = ppir_codegen_vec4_acc_op_mov;
      break;
   case ppir_op_sum3:
      f->op = ppir_codegen_vec4_acc_op_sum3;
      dest_shift = 0;
      break;
   case ppir_op_sum4:
      f->op = ppir_codegen_vec4_acc_op_sum4;
      dest_shift = 0;
      break;
   case ppir_op_floor:
      f->op = ppir_codegen_vec4_acc_op_floor;
      break;
   case ppir_op_ceil:
      f->op = ppir_codegen_vec4_acc_op_ceil;
      break;
   case ppir_op_fract:
      f->op = ppir_codegen_vec4_acc_op_fract;
      break;
   case ppir_op_gt:
      f->op = ppir_codegen_vec4_acc_op_gt;
      break;
   case ppir_op_ge:
      f->op = ppir_codegen_vec4_acc_op_ge;
      break;
   case ppir_op_eq:
      f->op = ppir_codegen_vec4_acc_op_eq;
      break;
   case ppir_op_ne:
      f->op = ppir_codegen_vec4_acc_op_ne;
      break;
   case ppir_op_select:
      f->op = ppir_codegen_vec4_acc_op_sel;
      break;
   case ppir_op_max:
      f->op = ppir_codegen_vec4_acc_op_max;
      break;
   case ppir_op_min:
      f->op = ppir_codegen_vec4_acc_op_min;
      break;
   case ppir_op_ddx:
      f->op = ppir_codegen_vec4_acc_op_dFdx;
      break;
   case ppir_op_ddy:
      f->op = ppir_codegen_vec4_acc_op_dFdy;
      break;
   default:
      break;
   }

   ppir_src *src = node->op == ppir_op_select ? alu->src + 1 : alu->src;
   index = ppir_target_get_src_reg_index(src);

   if (src->type == ppir_target_pipeline &&
       src->pipeline == ppir_pipeline_reg_vmul)
      f->mul_in = true;
   else
      f->arg0_source = index >> 2;

   f->arg0_swizzle = encode_swizzle(src->swizzle, index & 0x3, dest_shift);
   f->arg0_absolute = src->absolute;
   f->arg0_negate = src->negate;

   if (++src < alu->src + alu->num_src) {
      index = ppir_target_get_src_reg_index(src);
      f->arg1_source = index >> 2;
      f->arg1_swizzle = encode_swizzle(src->swizzle, index & 0x3, dest_shift);
      f->arg1_absolute = src->absolute;
      f->arg1_negate = src->negate;
   }
}

static void ppir_codegen_encode_scl_add(ppir_node *node, void *code)
{
   ppir_codegen_field_float_acc *f = code;
   ppir_alu_node *alu = ppir_node_to_alu(node);

   ppir_dest *dest = &alu->dest;
   int dest_component = ffs(dest->write_mask) - 1;
   assert(dest_component >= 0);

   f->dest = ppir_target_get_dest_reg_index(dest) + dest_component;
   f->output_en = true;
   f->dest_modifier = ppir_codegen_get_outmod(dest->modifier);

   switch (node->op) {
   case ppir_op_add:
      f->op = shift_to_op(alu->shift);
      break;
   case ppir_op_mov:
      f->op = ppir_codegen_float_acc_op_mov;
      break;
   case ppir_op_max:
      f->op = ppir_codegen_float_acc_op_max;
      break;
   case ppir_op_min:
      f->op = ppir_codegen_float_acc_op_min;
      break;
   case ppir_op_floor:
      f->op = ppir_codegen_float_acc_op_floor;
      break;
   case ppir_op_ceil:
      f->op = ppir_codegen_float_acc_op_ceil;
      break;
   case ppir_op_fract:
      f->op = ppir_codegen_float_acc_op_fract;
      break;
   case ppir_op_gt:
      f->op = ppir_codegen_float_acc_op_gt;
      break;
   case ppir_op_ge:
      f->op = ppir_codegen_float_acc_op_ge;
      break;
   case ppir_op_eq:
      f->op = ppir_codegen_float_acc_op_eq;
      break;
   case ppir_op_ne:
      f->op = ppir_codegen_float_acc_op_ne;
      break;
   case ppir_op_select:
      f->op = ppir_codegen_float_acc_op_sel;
      break;
   case ppir_op_ddx:
      f->op = ppir_codegen_float_acc_op_dFdx;
      break;
   case ppir_op_ddy:
      f->op = ppir_codegen_float_acc_op_dFdy;
      break;
   default:
      break;
   }

   ppir_src *src = node->op == ppir_op_select ? alu->src + 1: alu->src;
   if (src->type == ppir_target_pipeline &&
       src->pipeline == ppir_pipeline_reg_fmul)
      f->mul_in = true;
   else
      f->arg0_source = get_scl_reg_index(src, dest_component);
   f->arg0_absolute = src->absolute;
   f->arg0_negate = src->negate;

   if (++src < alu->src + alu->num_src) {
      f->arg1_source = get_scl_reg_index(src, dest_component);
      f->arg1_absolute = src->absolute;
      f->arg1_negate = src->negate;
   }
}

static void ppir_codegen_encode_combine(ppir_node *node, void *code)
{
   ppir_codegen_field_combine *f = code;
   ppir_alu_node *alu = ppir_node_to_alu(node);

   switch (node->op) {
   case ppir_op_rsqrt:
   case ppir_op_log2:
   case ppir_op_exp2:
   case ppir_op_rcp:
   case ppir_op_sqrt:
   case ppir_op_sin:
   case ppir_op_cos:
   {
      f->scalar.dest_vec = false;
      f->scalar.arg1_en = false;

      ppir_dest *dest = &alu->dest;
      int dest_component = ffs(dest->write_mask) - 1;
      assert(dest_component >= 0);
      f->scalar.dest = ppir_target_get_dest_reg_index(dest) + dest_component;
      f->scalar.dest_modifier = ppir_codegen_get_outmod(dest->modifier);

      ppir_src *src = alu->src;
      f->scalar.arg0_src = get_scl_reg_index(src, dest_component);
      f->scalar.arg0_absolute = src->absolute;
      f->scalar.arg0_negate = src->negate;

      switch (node->op) {
      case ppir_op_rsqrt:
         f->scalar.op = ppir_codegen_combine_scalar_op_rsqrt;
         break;
      case ppir_op_log2:
         f->scalar.op = ppir_codegen_combine_scalar_op_log2;
         break;
      case ppir_op_exp2:
         f->scalar.op = ppir_codegen_combine_scalar_op_exp2;
         break;
      case ppir_op_rcp:
         f->scalar.op = ppir_codegen_combine_scalar_op_rcp;
         break;
      case ppir_op_sqrt:
         f->scalar.op = ppir_codegen_combine_scalar_op_sqrt;
         break;
      case ppir_op_sin:
         f->scalar.op = ppir_codegen_combine_scalar_op_sin;
         break;
      case ppir_op_cos:
         f->scalar.op = ppir_codegen_combine_scalar_op_cos;
         break;
      default:
         break;
      }
      break;
   }
   default:
      break;
   }
}

static void ppir_codegen_encode_store_temp(ppir_node *node, void *code)
{
   assert(node->op == ppir_op_store_temp);

   ppir_codegen_field_temp_write *f = code;
   ppir_store_node *snode = ppir_node_to_store(node);
   int num_components = snode->num_components;

   f->temp_write.dest = 0x03; // 11 - temporary
   f->temp_write.source = snode->src.reg->index;

   int alignment = num_components == 4 ? 2 : num_components - 1;
   f->temp_write.alignment = alignment;
   f->temp_write.index = snode->index << (2 - alignment);

   f->temp_write.offset_reg = snode->index >> 2;
}

static void ppir_codegen_encode_const(ppir_const *constant, uint16_t *code)
{
   for (int i = 0; i < constant->num; i++)
      code[i] = _mesa_float_to_half(constant->value[i].f);
}

static void ppir_codegen_encode_discard(ppir_node *node, void *code)
{
   ppir_codegen_field_branch *b = code;
   assert(node->op == ppir_op_discard);

   b->discard.word0 = PPIR_CODEGEN_DISCARD_WORD0;
   b->discard.word1 = PPIR_CODEGEN_DISCARD_WORD1;
   b->discard.word2 = PPIR_CODEGEN_DISCARD_WORD2;
}

static void ppir_codegen_encode_branch(ppir_node *node, void *code)
{
   ppir_codegen_field_branch *b = code;
   ppir_branch_node *branch;
   ppir_instr *target_instr;
   ppir_block *target;
   if (node->op == ppir_op_discard) {
      ppir_codegen_encode_discard(node, code);
      return;
   }

   assert(node->op == ppir_op_branch);
   branch = ppir_node_to_branch(node);

   b->branch.unknown_0 = 0x0;
   b->branch.unknown_1 = 0x0;

   if (branch->num_src == 2) {
      b->branch.arg0_source = get_scl_reg_index(&branch->src[0], 0);
      b->branch.arg1_source = get_scl_reg_index(&branch->src[1], 0);
      b->branch.cond_gt = branch->cond_gt;
      b->branch.cond_eq = branch->cond_eq;
      b->branch.cond_lt = branch->cond_lt;
   } else if (branch->num_src == 0) {
      /* Unconditional branch */
      b->branch.arg0_source = 0;
      b->branch.arg1_source = 0;
      b->branch.cond_gt = true;
      b->branch.cond_eq = true;
      b->branch.cond_lt = true;
   } else {
      assert(false);
   }

   target = branch->target;
   while (list_is_empty(&target->instr_list)) {
      if (!target->list.next)
         break;
      target = list_entry(target->list.next, ppir_block, list);
   }

   assert(!list_is_empty(&target->instr_list));

   target_instr = list_first_entry(&target->instr_list, ppir_instr, list);
   b->branch.target = target_instr->offset - node->instr->offset;
   b->branch.next_count = target_instr->encode_size;
}

typedef void (*ppir_codegen_instr_slot_encode_func)(ppir_node *, void *);

static const ppir_codegen_instr_slot_encode_func
ppir_codegen_encode_slot[PPIR_INSTR_SLOT_NUM] = {
   [PPIR_INSTR_SLOT_VARYING] = ppir_codegen_encode_varying,
   [PPIR_INSTR_SLOT_TEXLD] = ppir_codegen_encode_texld,
   [PPIR_INSTR_SLOT_UNIFORM] = ppir_codegen_encode_uniform,
   [PPIR_INSTR_SLOT_ALU_VEC_MUL] = ppir_codegen_encode_vec_mul,
   [PPIR_INSTR_SLOT_ALU_SCL_MUL] = ppir_codegen_encode_scl_mul,
   [PPIR_INSTR_SLOT_ALU_VEC_ADD] = ppir_codegen_encode_vec_add,
   [PPIR_INSTR_SLOT_ALU_SCL_ADD] = ppir_codegen_encode_scl_add,
   [PPIR_INSTR_SLOT_ALU_COMBINE] = ppir_codegen_encode_combine,
   [PPIR_INSTR_SLOT_STORE_TEMP] = ppir_codegen_encode_store_temp,
   [PPIR_INSTR_SLOT_BRANCH] = ppir_codegen_encode_branch,
};

static const int ppir_codegen_field_size[] = {
   34, 62, 41, 43, 30, 44, 31, 30, 41, 73
};

static inline int align_to_word(int size)
{
   return ((size + 0x1f) >> 5);
}

static int get_instr_encode_size(ppir_instr *instr)
{
   int size = 0;

   for (int i = 0; i < PPIR_INSTR_SLOT_NUM; i++) {
      if (instr->slots[i])
         size += ppir_codegen_field_size[i];
   }

   for (int i = 0; i < 2; i++) {
      if (instr->constant[i].num)
         size += 64;
   }

   return align_to_word(size) + 1;
}

static void bitcopy(void *dst, int dst_offset, void *src, int src_size)
{
   unsigned char *cpy_dst = dst, *cpy_src = src;
   int off1 = dst_offset & 0x07;

   cpy_dst += (dst_offset >> 3);

   if (off1) {
      int off2 = 0x08 - off1;
      int cpy_size = 0;
      while (1) {
         *cpy_dst |= *cpy_src << off1;
         cpy_dst++;

         cpy_size += off2;
         if (cpy_size >= src_size)
            break;

         *cpy_dst |= *cpy_src >> off2;
         cpy_src++;

         cpy_size += off1;
         if (cpy_size >= src_size)
            break;
      }
   }
   else
      memcpy(cpy_dst, cpy_src, align_to_word(src_size) * 4);
}

static int encode_instr(ppir_instr *instr, void *code, void *last_code)
{
   int size = 0;
   ppir_codegen_ctrl *ctrl = code;

   for (int i = 0; i < PPIR_INSTR_SLOT_NUM; i++) {
      if (instr->slots[i]) {
         /* max field size (73), align to dword */
         uint8_t output[12] = {0};

         ppir_codegen_encode_slot[i](instr->slots[i], output);
         bitcopy(ctrl + 1, size, output, ppir_codegen_field_size[i]);

         size += ppir_codegen_field_size[i];
         ctrl->fields |= 1 << i;
      }
   }

   if (instr->slots[PPIR_INSTR_SLOT_TEXLD])
      ctrl->sync = true;

   if (instr->slots[PPIR_INSTR_SLOT_ALU_VEC_ADD]) {
      ppir_node *node = instr->slots[PPIR_INSTR_SLOT_ALU_VEC_ADD];
      if (node->op == ppir_op_ddx || node->op == ppir_op_ddy)
         ctrl->sync = true;
   }

   if (instr->slots[PPIR_INSTR_SLOT_ALU_SCL_ADD]) {
      ppir_node *node = instr->slots[PPIR_INSTR_SLOT_ALU_SCL_ADD];
      if (node->op == ppir_op_ddx || node->op == ppir_op_ddy)
         ctrl->sync = true;
   }

   for (int i = 0; i < 2; i++) {
      if (instr->constant[i].num) {
         uint16_t output[4] = {0};

         ppir_codegen_encode_const(instr->constant + i, output);
         bitcopy(ctrl + 1, size, output, instr->constant[i].num * 16);

         size += 64;
         ctrl->fields |= 1 << (ppir_codegen_field_shift_vec4_const_0 + i);
      }
   }

   size = align_to_word(size) + 1;

   ctrl->count = size;
   if (instr->stop)
      ctrl->stop = true;

   if (last_code) {
      ppir_codegen_ctrl *last_ctrl = last_code;
      last_ctrl->next_count = size;
      last_ctrl->prefetch = true;
   }

   return size;
}

static void ppir_codegen_print_prog(ppir_compiler *comp)
{
   uint32_t *prog = comp->prog->shader;
   unsigned offset = 0;

   printf("========ppir codegen========\n");
   list_for_each_entry(ppir_block, block, &comp->block_list, list) {
      list_for_each_entry(ppir_instr, instr, &block->instr_list, list) {
         printf("%03d (@%6d): ", instr->index, instr->offset);
         int n = prog[0] & 0x1f;
         for (int i = 0; i < n; i++) {
            if (i && i % 6 == 0)
               printf("\n    ");
            printf("%08x ", prog[i]);
         }
         printf("\n");
         ppir_disassemble_instr(prog, offset, stdout);
         prog += n;
         offset += n;
      }
   }
   printf("-----------------------\n");
}

bool ppir_codegen_prog(ppir_compiler *comp)
{
   int size = 0;
   list_for_each_entry(ppir_block, block, &comp->block_list, list) {
      list_for_each_entry(ppir_instr, instr, &block->instr_list, list) {
         instr->offset = size;
         instr->encode_size = get_instr_encode_size(instr);
         size += instr->encode_size;
      }
      /* Set stop flag for the last instruction if block has stop flag */
      if (block->stop) {
         ppir_instr *instr = list_last_entry(&block->instr_list, ppir_instr, list);
         instr->stop = true;
      }
   }

   uint32_t *prog = rzalloc_size(comp->prog, size * sizeof(uint32_t));
   if (!prog)
      return false;

   uint32_t *code = prog, *last_code = NULL;
   list_for_each_entry(ppir_block, block, &comp->block_list, list) {
      list_for_each_entry(ppir_instr, instr, &block->instr_list, list) {
         int offset = encode_instr(instr, code, last_code);
         last_code = code;
         code += offset;
      }
   }

   if (comp->prog->shader)
      ralloc_free(comp->prog->shader);

   comp->prog->shader = prog;
   comp->prog->state.shader_size = size * sizeof(uint32_t);

   if (lima_debug & LIMA_DEBUG_PP)
      ppir_codegen_print_prog(comp);

   return true;
}
