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
#include "compiler/nir/nir.h"
#include "pipe/p_state.h"


#include "gpir.h"
#include "lima_context.h"

gpir_reg *gpir_create_reg(gpir_compiler *comp)
{
   gpir_reg *reg = ralloc(comp, gpir_reg);
   reg->index = comp->cur_reg++;
   list_addtail(&reg->list, &comp->reg_list);
   return reg;
}

/* Register the given gpir_node as providing the given NIR destination, so
 * that gpir_node_find() will return it. Also insert any stores necessary if
 * the destination will be used after the end of this basic block. The node
 * must already be inserted.
 */
static void register_node_ssa(gpir_block *block, gpir_node *node, nir_def *ssa)
{
   block->comp->node_for_ssa[ssa->index] = node;
   snprintf(node->name, sizeof(node->name), "ssa%d", ssa->index);

   /* If any uses are outside the current block, we'll need to create a
    * register and store to it.
    */
   bool needs_register = false;
   nir_foreach_use(use, ssa) {
      if (nir_src_parent_instr(use)->block != ssa->parent_instr->block) {
         needs_register = true;
         break;
      }
   }

   if (!needs_register) {
      nir_foreach_if_use(use, ssa) {
         if (nir_cf_node_prev(&nir_src_parent_if(use)->cf_node) !=
             &ssa->parent_instr->block->cf_node) {
            needs_register = true;
            break;
         }
      }
   }

   if (needs_register) {
      gpir_store_node *store = gpir_node_create(block, gpir_op_store_reg);
      store->child = node;
      store->reg = gpir_create_reg(block->comp);
      gpir_node_add_dep(&store->node, node, GPIR_DEP_INPUT);
      list_addtail(&store->node.list, &block->node_list);
      block->comp->reg_for_ssa[ssa->index] = store->reg;
   }
}

static void register_node_reg(gpir_block *block, gpir_node *node, int index)
{
   block->comp->node_for_ssa[index] = node;
   gpir_store_node *store = gpir_node_create(block, gpir_op_store_reg);

   snprintf(store->node.name, sizeof(node->name), "reg%d", index);

   store->child = node;
   store->reg = block->comp->reg_for_ssa[index];
   gpir_node_add_dep(&store->node, node, GPIR_DEP_INPUT);

   list_addtail(&store->node.list, &block->node_list);
}

static gpir_node *gpir_node_find(gpir_block *block, nir_src *src,
                                 int channel)
{
   gpir_reg *reg = NULL;
   gpir_node *pred = NULL;
   if (src->ssa->num_components > 1) {
      for (int i = 0; i < GPIR_VECTOR_SSA_NUM; i++) {
         if (block->comp->vector_ssa[i].ssa == src->ssa->index) {
            return block->comp->vector_ssa[i].nodes[channel];
         }
      }
   } else {
      gpir_node *pred = block->comp->node_for_ssa[src->ssa->index];
      if (pred && pred->block == block)
         return pred;
      reg = block->comp->reg_for_ssa[src->ssa->index];
   }

   assert(reg);
   pred = gpir_node_create(block, gpir_op_load_reg);
   gpir_load_node *load = gpir_node_to_load(pred);
   load->reg = reg;
   list_addtail(&pred->list, &block->node_list);

   return pred;
}

static int nir_to_gpir_opcodes[nir_num_opcodes] = {
   [nir_op_fmul] = gpir_op_mul,
   [nir_op_fadd] = gpir_op_add,
   [nir_op_fneg] = gpir_op_neg,
   [nir_op_fmin] = gpir_op_min,
   [nir_op_fmax] = gpir_op_max,
   [nir_op_frcp] = gpir_op_rcp,
   [nir_op_frsq] = gpir_op_rsqrt,
   [nir_op_fexp2] = gpir_op_exp2,
   [nir_op_flog2] = gpir_op_log2,
   [nir_op_slt] = gpir_op_lt,
   [nir_op_sge] = gpir_op_ge,
   [nir_op_fcsel] = gpir_op_select,
   [nir_op_ffloor] = gpir_op_floor,
   [nir_op_fsign] = gpir_op_sign,
   [nir_op_seq] = gpir_op_eq,
   [nir_op_sne] = gpir_op_ne,
   [nir_op_fabs] = gpir_op_abs,
};

static bool gpir_emit_alu(gpir_block *block, nir_instr *ni)
{
   nir_alu_instr *instr = nir_instr_as_alu(ni);

   /* gpir_op_mov is useless before the final scheduler, and the scheduler
    * currently doesn't expect us to emit it. Just register the destination of
    * this instruction with its source. This will also emit any necessary
    * register loads/stores for things like "r0 = mov ssa_0" or
    * "ssa_0 = mov r0".
    */
   if (instr->op == nir_op_mov) {
      gpir_node *child = gpir_node_find(block, &instr->src[0].src,
                                        instr->src[0].swizzle[0]);
      register_node_ssa(block, child, &instr->def);
      return true;
   }

   int op = nir_to_gpir_opcodes[instr->op];

   if (op == gpir_op_unsupported) {
      gpir_error("unsupported nir_op: %s\n", nir_op_infos[instr->op].name);
      return false;
   }

   gpir_alu_node *node = gpir_node_create(block, op);
   if (unlikely(!node))
      return false;

   unsigned num_child = nir_op_infos[instr->op].num_inputs;
   assert(num_child <= ARRAY_SIZE(node->children));
   node->num_child = num_child;

   for (int i = 0; i < num_child; i++) {
      nir_alu_src *src = instr->src + i;

      gpir_node *child = gpir_node_find(block, &src->src, src->swizzle[0]);
      node->children[i] = child;

      gpir_node_add_dep(&node->node, child, GPIR_DEP_INPUT);
   }

   list_addtail(&node->node.list, &block->node_list);
   register_node_ssa(block, &node->node, &instr->def);

   return true;
}

static gpir_node *gpir_create_load(gpir_block *block, nir_def *def,
                                   int op, int index, int component)
{
   gpir_load_node *load = gpir_node_create(block, op);
   if (unlikely(!load))
      return NULL;

   load->index = index;
   load->component = component;
   list_addtail(&load->node.list, &block->node_list);
   register_node_ssa(block, &load->node, def);
   return &load->node;
}

static bool gpir_create_vector_load(gpir_block *block, nir_def *def, int index)
{
   assert(index < GPIR_VECTOR_SSA_NUM);

   block->comp->vector_ssa[index].ssa = def->index;

   for (int i = 0; i < def->num_components; i++) {
      gpir_node *node = gpir_create_load(block, def, gpir_op_load_uniform,
                                         block->comp->constant_base + index, i);
      if (!node)
         return false;

      block->comp->vector_ssa[index].nodes[i] = node;
      snprintf(node->name, sizeof(node->name), "ssa%d.%c", def->index, "xyzw"[i]);
   }

   return true;
}

static bool gpir_emit_intrinsic(gpir_block *block, nir_instr *ni)
{
   nir_intrinsic_instr *instr = nir_instr_as_intrinsic(ni);

   switch (instr->intrinsic) {
   case nir_intrinsic_decl_reg:
   {
      gpir_reg *reg = gpir_create_reg(block->comp);
      block->comp->reg_for_ssa[instr->def.index] = reg;
      return true;
   }
   case nir_intrinsic_load_reg:
   {
      gpir_node *node = gpir_node_find(block, &instr->src[0], 0);
      assert(node);
      block->comp->node_for_ssa[instr->def.index] = node;
      return true;
   }
   case nir_intrinsic_store_reg:
   {
      gpir_node *child = gpir_node_find(block, &instr->src[0], 0);
      assert(child);
      register_node_reg(block, child, instr->src[1].ssa->index);
      return true;
   }
   case nir_intrinsic_load_input:
      return gpir_create_load(block, &instr->def,
                              gpir_op_load_attribute,
                              nir_intrinsic_base(instr),
                              nir_intrinsic_component(instr)) != NULL;
   case nir_intrinsic_load_uniform:
   {
      int offset = nir_intrinsic_base(instr);

      if (!nir_src_is_const(instr->src[0])) {
         gpir_error("indirect indexing for uniforms is not implemented\n");
         return false;
      }
      offset += (int)nir_src_as_float(instr->src[0]);

      return gpir_create_load(block, &instr->def,
                              gpir_op_load_uniform,
                              offset / 4, offset % 4) != NULL;
   }
   case nir_intrinsic_load_viewport_scale:
      return gpir_create_vector_load(block, &instr->def, GPIR_VECTOR_SSA_VIEWPORT_SCALE);
   case nir_intrinsic_load_viewport_offset:
      return gpir_create_vector_load(block, &instr->def, GPIR_VECTOR_SSA_VIEWPORT_OFFSET);
   case nir_intrinsic_store_output:
   {
      gpir_store_node *store = gpir_node_create(block, gpir_op_store_varying);
      if (unlikely(!store))
         return false;
      gpir_node *child = gpir_node_find(block, instr->src, 0);
      store->child = child;
      store->index = nir_intrinsic_base(instr);
      store->component = nir_intrinsic_component(instr);

      gpir_node_add_dep(&store->node, child, GPIR_DEP_INPUT);
      list_addtail(&store->node.list, &block->node_list);

      return true;
   }
   default:
      gpir_error("unsupported nir_intrinsic_instr %s\n",
                 nir_intrinsic_infos[instr->intrinsic].name);
      return false;
   }
}

static bool gpir_emit_load_const(gpir_block *block, nir_instr *ni)
{
   nir_load_const_instr *instr = nir_instr_as_load_const(ni);
   gpir_const_node *node = gpir_node_create(block, gpir_op_const);
   if (unlikely(!node))
      return false;

   assert(instr->def.bit_size == 32);
   assert(instr->def.num_components == 1);

   node->value.i = instr->value[0].i32;

   list_addtail(&node->node.list, &block->node_list);
   register_node_ssa(block, &node->node, &instr->def);
   return true;
}

static bool gpir_emit_ssa_undef(gpir_block *block, nir_instr *ni)
{
   gpir_error("nir_undef_instr is not supported\n");
   return false;
}

static bool gpir_emit_tex(gpir_block *block, nir_instr *ni)
{
   gpir_error("texture operations are not supported\n");
   return false;
}

static bool gpir_emit_jump(gpir_block *block, nir_instr *ni)
{
   /* Jumps are emitted at the end of the basic block, so do nothing. */
   return true;
}

static bool (*gpir_emit_instr[nir_instr_type_phi])(gpir_block *, nir_instr *) = {
   [nir_instr_type_alu]        = gpir_emit_alu,
   [nir_instr_type_intrinsic]  = gpir_emit_intrinsic,
   [nir_instr_type_load_const] = gpir_emit_load_const,
   [nir_instr_type_undef]      = gpir_emit_ssa_undef,
   [nir_instr_type_tex]        = gpir_emit_tex,
   [nir_instr_type_jump]       = gpir_emit_jump,
};

static bool gpir_emit_function(gpir_compiler *comp, nir_function_impl *impl)
{
   nir_index_blocks(impl);
   comp->blocks = ralloc_array(comp, gpir_block *, impl->num_blocks);

   nir_foreach_block(block_nir, impl) {
      gpir_block *block = ralloc(comp, gpir_block);
      if (!block)
         return false;

      list_inithead(&block->node_list);
      list_inithead(&block->instr_list);

      list_addtail(&block->list, &comp->block_list);
      block->comp = comp;
      comp->blocks[block_nir->index] = block;
   }

   nir_foreach_block(block_nir, impl) {
      gpir_block *block = comp->blocks[block_nir->index];
      nir_foreach_instr(instr, block_nir) {
         assert(instr->type < nir_instr_type_phi);
         if (!gpir_emit_instr[instr->type](block, instr))
            return false;
      }

      if (block_nir->successors[0] == impl->end_block)
         block->successors[0] = NULL;
      else
         block->successors[0] = comp->blocks[block_nir->successors[0]->index];
      block->successors[1] = NULL;

      if (block_nir->successors[1] != NULL) {
         nir_if *nif = nir_cf_node_as_if(nir_cf_node_next(&block_nir->cf_node));
         gpir_alu_node *cond = gpir_node_create(block, gpir_op_not);
         cond->children[0] = gpir_node_find(block, &nif->condition, 0);

         gpir_node_add_dep(&cond->node, cond->children[0], GPIR_DEP_INPUT);
         list_addtail(&cond->node.list, &block->node_list);

         gpir_branch_node *branch = gpir_node_create(block, gpir_op_branch_cond);
         list_addtail(&branch->node.list, &block->node_list);

         branch->dest = comp->blocks[block_nir->successors[1]->index];
         block->successors[1] = branch->dest;

         branch->cond = &cond->node;
         gpir_node_add_dep(&branch->node, &cond->node, GPIR_DEP_INPUT);

         assert(block_nir->successors[0]->index == block_nir->index + 1);
      } else if (block_nir->successors[0]->index != block_nir->index + 1) {
         gpir_branch_node *branch = gpir_node_create(block, gpir_op_branch_uncond);
         list_addtail(&branch->node.list, &block->node_list);

         branch->dest = comp->blocks[block_nir->successors[0]->index];
      }
   }

   return true;
}

static gpir_compiler *gpir_compiler_create(void *prog, unsigned num_ssa)
{
   gpir_compiler *comp = rzalloc(prog, gpir_compiler);

   list_inithead(&comp->block_list);
   list_inithead(&comp->reg_list);

   for (int i = 0; i < GPIR_VECTOR_SSA_NUM; i++)
      comp->vector_ssa[i].ssa = -1;

   comp->node_for_ssa = rzalloc_array(comp, gpir_node *, num_ssa);
   comp->reg_for_ssa = rzalloc_array(comp, gpir_reg *, num_ssa);
   comp->prog = prog;
   return comp;
}

static int gpir_glsl_type_size(enum glsl_base_type type)
{
   /* only support GLSL_TYPE_FLOAT */
   assert(type == GLSL_TYPE_FLOAT);
   return 4;
}

static void gpir_print_shader_db(struct nir_shader *nir, gpir_compiler *comp,
                                 struct util_debug_callback *debug)
{
   const struct shader_info *info = &nir->info;
   char *shaderdb;
   ASSERTED int ret = asprintf(&shaderdb,
                               "%s shader: %d inst, %d loops, %d:%d spills:fills\n",
                               gl_shader_stage_name(info->stage),
                               comp->num_instr,
                               comp->num_loops,
                               comp->num_spills,
                               comp->num_fills);
   assert(ret >= 0);

   if (lima_debug & LIMA_DEBUG_SHADERDB)
      fprintf(stderr, "SHADER-DB: %s\n", shaderdb);

   util_debug_message(debug, SHADER_INFO, "%s", shaderdb);
   free(shaderdb);
}

bool gpir_compile_nir(struct lima_vs_compiled_shader *prog, struct nir_shader *nir,
                      struct util_debug_callback *debug)
{
   nir_function_impl *func = nir_shader_get_entrypoint(nir);
   gpir_compiler *comp = gpir_compiler_create(prog, func->ssa_alloc);
   if (!comp)
      return false;

   comp->constant_base = nir->num_uniforms;
   prog->state.uniform_size = nir->num_uniforms * 16;
   prog->state.gl_pos_idx = 0;
   prog->state.point_size_idx = -1;

   if (!gpir_emit_function(comp, func))
      goto err_out0;

   gpir_node_print_prog_seq(comp);
   gpir_node_print_prog_dep(comp);

   /* increase for viewport uniforms */
   comp->constant_base += GPIR_VECTOR_SSA_NUM;

   if (!gpir_optimize(comp))
      goto err_out0;

   if (!gpir_pre_rsched_lower_prog(comp))
      goto err_out0;

   if (!gpir_reduce_reg_pressure_schedule_prog(comp))
      goto err_out0;

   if (!gpir_regalloc_prog(comp))
      goto err_out0;

   if (!gpir_schedule_prog(comp))
      goto err_out0;

   if (!gpir_codegen_prog(comp))
      goto err_out0;

   /* initialize to support accumulating below */
   nir_foreach_shader_out_variable(var, nir) {
      struct lima_varying_info *v = prog->state.varying + var->data.driver_location;
      v->components = 0;
   }

   nir_foreach_shader_out_variable(var, nir) {
      bool varying = true;
      switch (var->data.location) {
      case VARYING_SLOT_POS:
         prog->state.gl_pos_idx = var->data.driver_location;
         varying = false;
         break;
      case VARYING_SLOT_PSIZ:
         prog->state.point_size_idx = var->data.driver_location;
         varying = false;
         break;
      }

      struct lima_varying_info *v = prog->state.varying + var->data.driver_location;
      if (!v->components) {
         v->component_size = gpir_glsl_type_size(glsl_get_base_type(var->type));
         prog->state.num_outputs++;
         if (varying)
            prog->state.num_varyings++;
      }

      v->components += glsl_get_components(var->type);
   }

   gpir_print_shader_db(nir, comp, debug);

   ralloc_free(comp);
   return true;

err_out0:
   ralloc_free(comp);
   return false;
}

