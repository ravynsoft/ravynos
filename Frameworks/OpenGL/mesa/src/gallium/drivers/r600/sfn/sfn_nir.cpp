/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2019 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "sfn_nir.h"

#include "../r600_asm.h"
#include "../r600_pipe.h"
#include "../r600_shader.h"

#include "nir.h"
#include "nir_builder.h"
#include "nir_intrinsics.h"
#include "sfn_assembler.h"
#include "sfn_debug.h"
#include "sfn_instr_tex.h"
#include "sfn_liverangeevaluator.h"
#include "sfn_nir_lower_alu.h"
#include "sfn_nir_lower_fs_out_to_vector.h"
#include "sfn_nir_lower_tex.h"
#include "sfn_optimizer.h"
#include "sfn_ra.h"
#include "sfn_scheduler.h"
#include "sfn_shader.h"
#include "sfn_split_address_loads.h"
#include "util/u_debug.h"
#include "util/u_prim.h"

#include <vector>

namespace r600 {

using std::vector;

NirLowerInstruction::NirLowerInstruction():
    b(nullptr)
{
}

bool
NirLowerInstruction::filter_instr(const nir_instr *instr, const void *data)
{
   auto me = reinterpret_cast<const NirLowerInstruction *>(data);
   return me->filter(instr);
}

nir_def *
NirLowerInstruction::lower_instr(nir_builder *b, nir_instr *instr, void *data)
{
   auto me = reinterpret_cast<NirLowerInstruction *>(data);
   me->set_builder(b);
   return me->lower(instr);
}

bool
NirLowerInstruction::run(nir_shader *shader)
{
   return nir_shader_lower_instructions(shader, filter_instr, lower_instr, (void *)this);
}

AssemblyFromShader::~AssemblyFromShader() {}

bool
AssemblyFromShader::lower(const Shader& ir)
{
   return do_lower(ir);
}

static void
r600_nir_lower_scratch_address_impl(nir_builder *b, nir_intrinsic_instr *instr)
{
   b->cursor = nir_before_instr(&instr->instr);

   int address_index = 0;
   int align;

   if (instr->intrinsic == nir_intrinsic_store_scratch) {
      align = instr->src[0].ssa->num_components;
      address_index = 1;
   } else {
      align = instr->def.num_components;
   }

   nir_def *address = instr->src[address_index].ssa;
   nir_def *new_address = nir_ishr_imm(b, address, 4 * align);

   nir_src_rewrite(&instr->src[address_index], new_address);
}

bool
r600_lower_scratch_addresses(nir_shader *shader)
{
   bool progress = false;
   nir_foreach_function_impl(impl, shader)
   {
      nir_builder build = nir_builder_create(impl);

      nir_foreach_block(block, impl)
      {
         nir_foreach_instr(instr, block)
         {
            if (instr->type != nir_instr_type_intrinsic)
               continue;
            nir_intrinsic_instr *op = nir_instr_as_intrinsic(instr);
            if (op->intrinsic != nir_intrinsic_load_scratch &&
                op->intrinsic != nir_intrinsic_store_scratch)
               continue;
            r600_nir_lower_scratch_address_impl(&build, op);
            progress = true;
         }
      }
   }
   return progress;
}

static void
insert_uniform_sorted(struct exec_list *var_list, nir_variable *new_var)
{
   nir_foreach_variable_in_list(var, var_list)
   {
      if (var->data.binding > new_var->data.binding ||
          (var->data.binding == new_var->data.binding &&
           var->data.offset > new_var->data.offset)) {
         exec_node_insert_node_before(&var->node, &new_var->node);
         return;
      }
   }
   exec_list_push_tail(var_list, &new_var->node);
}

void
sort_uniforms(nir_shader *shader)
{
   struct exec_list new_list;
   exec_list_make_empty(&new_list);

   nir_foreach_uniform_variable_safe(var, shader)
   {
      exec_node_remove(&var->node);
      insert_uniform_sorted(&new_list, var);
   }
   exec_list_append(&shader->variables, &new_list);
}

static void
insert_fsoutput_sorted(struct exec_list *var_list, nir_variable *new_var)
{

   nir_foreach_variable_in_list(var, var_list)
   {
      if ((var->data.location >= FRAG_RESULT_DATA0 ||
          var->data.location == FRAG_RESULT_COLOR) &&
          (new_var->data.location < FRAG_RESULT_COLOR ||
           new_var->data.location == FRAG_RESULT_SAMPLE_MASK)) {
         exec_node_insert_after(&var->node, &new_var->node);
         return;
      } else if ((new_var->data.location >= FRAG_RESULT_DATA0 ||
                  new_var->data.location == FRAG_RESULT_COLOR) &&
                 (var->data.location < FRAG_RESULT_COLOR ||
                  var->data.location == FRAG_RESULT_SAMPLE_MASK)) {
         exec_node_insert_node_before(&var->node, &new_var->node);
         return;
      } else if (var->data.location > new_var->data.location ||
          (var->data.location == new_var->data.location &&
           var->data.index > new_var->data.index)) {
         exec_node_insert_node_before(&var->node, &new_var->node);
         return;
      }
   }

   exec_list_push_tail(var_list, &new_var->node);
}

void
sort_fsoutput(nir_shader *shader)
{
   struct exec_list new_list;
   exec_list_make_empty(&new_list);

   nir_foreach_shader_out_variable_safe(var, shader)
   {
      exec_node_remove(&var->node);
      insert_fsoutput_sorted(&new_list, var);
   }

   unsigned driver_location = 0;
   nir_foreach_variable_in_list(var, &new_list) var->data.driver_location =
      driver_location++;

   exec_list_append(&shader->variables, &new_list);
}

class LowerClipvertexWrite : public NirLowerInstruction {

public:
   LowerClipvertexWrite(int noutputs, pipe_stream_output_info& so_info):
       m_clipplane1(noutputs),
       m_clipvtx(noutputs + 1),
       m_so_info(so_info)
   {
   }

private:
   bool filter(const nir_instr *instr) const override
   {
      if (instr->type != nir_instr_type_intrinsic)
         return false;

      auto intr = nir_instr_as_intrinsic(instr);
      if (intr->intrinsic != nir_intrinsic_store_output)
         return false;

      return nir_intrinsic_io_semantics(intr).location == VARYING_SLOT_CLIP_VERTEX;
   }

   nir_def *lower(nir_instr *instr) override
   {

      auto intr = nir_instr_as_intrinsic(instr);
      nir_def *output[8] = {nullptr};

      auto buf_id = nir_imm_int(b, R600_BUFFER_INFO_CONST_BUFFER);

      auto clip_vtx = intr->src[0].ssa;

      for (int i = 0; i < 8; ++i) {
         auto sel = nir_imm_int(b, i);
         auto mrow = nir_load_ubo_vec4(b, 4, 32, buf_id, sel);
         output[i] = nir_fdot4(b, clip_vtx, mrow);
      }

      unsigned clip_vertex_index = nir_intrinsic_base(intr);

      for (int i = 0; i < 2; ++i) {
         auto clip_i = nir_vec(b, &output[4 * i], 4);
         auto store = nir_store_output(b, clip_i, intr->src[1].ssa);
         nir_intrinsic_set_write_mask(store, 0xf);
         nir_intrinsic_set_base(store, clip_vertex_index);
         nir_io_semantics semantic = nir_intrinsic_io_semantics(intr);
         semantic.location = VARYING_SLOT_CLIP_DIST0 + i;
         semantic.no_varying = 1;

         if (i > 0)
            nir_intrinsic_set_base(store, m_clipplane1);
         nir_intrinsic_set_write_mask(store, 0xf);
         nir_intrinsic_set_io_semantics(store, semantic);
      }
      nir_intrinsic_set_base(intr, m_clipvtx);

      nir_def *result = NIR_LOWER_INSTR_PROGRESS_REPLACE;
      for (unsigned i = 0; i < m_so_info.num_outputs; ++i) {
         if (m_so_info.output[i].register_index == clip_vertex_index) {
            m_so_info.output[i].register_index = m_clipvtx;
            result = NIR_LOWER_INSTR_PROGRESS;
         }
      }
      return result;
   }
   int m_clipplane1;
   int m_clipvtx;
   pipe_stream_output_info& m_so_info;
};

/* lower_uniforms_to_ubo adds a 1 to the UBO buffer ID.
 * If the buffer ID is a non-constant value we end up
 * with "iadd bufid, 1", bot on r600 we can put that constant
 * "1" as constant cache ID into the CF instruction and don't need
 * to execute that extra ADD op, so eliminate the addition here
 * again and move the buffer base ID into the base value of
 * the intrinsic that is not used otherwise */
class OptIndirectUBOLoads : public NirLowerInstruction {
private:
   bool filter(const nir_instr *instr) const override
   {
      if (instr->type != nir_instr_type_intrinsic)
         return false;

      auto intr = nir_instr_as_intrinsic(instr);
      if (intr->intrinsic != nir_intrinsic_load_ubo_vec4)
         return false;

      if (nir_src_as_const_value(intr->src[0]) != nullptr)
         return false;

      return nir_intrinsic_base(intr) == 0;
   }

   nir_def *lower(nir_instr *instr) override
   {
      auto intr = nir_instr_as_intrinsic(instr);
      assert(intr->intrinsic == nir_intrinsic_load_ubo_vec4);

      auto parent = intr->src[0].ssa->parent_instr;

      if (parent->type != nir_instr_type_alu)
         return nullptr;

      auto alu = nir_instr_as_alu(parent);

      if (alu->op != nir_op_iadd)
         return nullptr;

      int new_base = 0;
      nir_src *new_bufid = nullptr;
      auto src0 = nir_src_as_const_value(alu->src[0].src);
      if (src0) {
         new_bufid = &alu->src[1].src;
         new_base = src0->i32;
      } else if (auto src1 = nir_src_as_const_value(alu->src[1].src)) {
         new_bufid = &alu->src[0].src;
         new_base = src1->i32;
      } else {
         return nullptr;
      }

      nir_intrinsic_set_base(intr, new_base);
      nir_src_rewrite(&intr->src[0], new_bufid->ssa);
      return &intr->def;
   }
};

} // namespace r600

static nir_intrinsic_op
r600_map_atomic(nir_intrinsic_op op)
{
   switch (op) {
   case nir_intrinsic_atomic_counter_read_deref:
      return nir_intrinsic_atomic_counter_read;
   case nir_intrinsic_atomic_counter_inc_deref:
      return nir_intrinsic_atomic_counter_inc;
   case nir_intrinsic_atomic_counter_pre_dec_deref:
      return nir_intrinsic_atomic_counter_pre_dec;
   case nir_intrinsic_atomic_counter_post_dec_deref:
      return nir_intrinsic_atomic_counter_post_dec;
   case nir_intrinsic_atomic_counter_add_deref:
      return nir_intrinsic_atomic_counter_add;
   case nir_intrinsic_atomic_counter_min_deref:
      return nir_intrinsic_atomic_counter_min;
   case nir_intrinsic_atomic_counter_max_deref:
      return nir_intrinsic_atomic_counter_max;
   case nir_intrinsic_atomic_counter_and_deref:
      return nir_intrinsic_atomic_counter_and;
   case nir_intrinsic_atomic_counter_or_deref:
      return nir_intrinsic_atomic_counter_or;
   case nir_intrinsic_atomic_counter_xor_deref:
      return nir_intrinsic_atomic_counter_xor;
   case nir_intrinsic_atomic_counter_exchange_deref:
      return nir_intrinsic_atomic_counter_exchange;
   case nir_intrinsic_atomic_counter_comp_swap_deref:
      return nir_intrinsic_atomic_counter_comp_swap;
   default:
      return nir_num_intrinsics;
   }
}

static bool
r600_lower_deref_instr(nir_builder *b, nir_intrinsic_instr *instr,
                       UNUSED void *cb_data)
{
   nir_intrinsic_op op = r600_map_atomic(instr->intrinsic);
   if (nir_num_intrinsics == op)
      return false;

   nir_deref_instr *deref = nir_src_as_deref(instr->src[0]);
   nir_variable *var = nir_deref_instr_get_variable(deref);

   if (var->data.mode != nir_var_uniform && var->data.mode != nir_var_mem_ssbo &&
       var->data.mode != nir_var_mem_shared)
      return false; /* atomics passed as function arguments can't be lowered */

   const unsigned idx = var->data.binding;

   b->cursor = nir_before_instr(&instr->instr);

   nir_def *offset = nir_imm_int(b, 0);
   for (nir_deref_instr *d = deref; d->deref_type != nir_deref_type_var;
        d = nir_deref_instr_parent(d)) {
      assert(d->deref_type == nir_deref_type_array);

      unsigned array_stride = 1;
      if (glsl_type_is_array(d->type))
         array_stride *= glsl_get_aoa_size(d->type);

      offset =
         nir_iadd(b, offset, nir_imul_imm(b, d->arr.index.ssa, array_stride));
   }

   /* Since the first source is a deref and the first source in the lowered
    * instruction is the offset, we can just swap it out and change the
    * opcode.
    */
   instr->intrinsic = op;
   nir_src_rewrite(&instr->src[0], offset);
   nir_intrinsic_set_base(instr, idx);
   nir_intrinsic_set_range_base(instr, var->data.index);

   nir_deref_instr_remove_if_unused(deref);

   return true;
}

static bool
r600_lower_clipvertex_to_clipdist(nir_shader *sh, pipe_stream_output_info& so_info)
{
   if (!(sh->info.outputs_written & VARYING_BIT_CLIP_VERTEX))
      return false;

   int noutputs = util_bitcount64(sh->info.outputs_written);
   bool result = r600::LowerClipvertexWrite(noutputs, so_info).run(sh);
   return result;
}

static bool
r600_nir_lower_atomics(nir_shader *shader)
{
   /* In Hardware we start at a zero index for each new
    * binding, and we use an offset of one per counter. We also
    * need to sort the atomics according to binding and offset. */
   std::map<unsigned, unsigned> binding_offset;
   std::map<unsigned, nir_variable *> sorted_var;

   nir_foreach_variable_with_modes_safe(var, shader, nir_var_uniform) {
      if (glsl_contains_atomic(var->type)) {
         sorted_var[(var->data.binding << 16) | var->data.offset] = var;
         exec_node_remove(&var->node);
      }
   }

   for (auto& [dummy, var] : sorted_var) {
      auto iindex = binding_offset.find(var->data.binding);
      unsigned offset_update = glsl_atomic_size(var->type) / 4; /* ATOMIC_COUNTER_SIZE */
      if (iindex == binding_offset.end()) {
         var->data.index = 0;
         binding_offset[var->data.binding] = offset_update;
      } else {
         var->data.index = iindex->second;
         iindex->second += offset_update;
      }
      shader->variables.push_tail(&var->node);
   }

   return nir_shader_intrinsics_pass(shader, r600_lower_deref_instr,
                                     nir_metadata_block_index | nir_metadata_dominance,
                                     NULL);
}
using r600::r600_lower_fs_out_to_vector;
using r600::r600_lower_scratch_addresses;
using r600::r600_lower_ubo_to_align16;

int
r600_glsl_type_size(const struct glsl_type *type, bool is_bindless)
{
   return glsl_count_vec4_slots(type, false, is_bindless);
}

void
r600_get_natural_size_align_bytes(const struct glsl_type *type,
                                  unsigned *size,
                                  unsigned *align)
{
   if (type->base_type != GLSL_TYPE_ARRAY) {
      *align = 1;
      *size = 1;
   } else {
      unsigned elem_size, elem_align;
      glsl_get_natural_size_align_bytes(type->fields.array, &elem_size, &elem_align);
      *align = 1;
      *size = type->length;
   }
}

static bool
r600_lower_shared_io_impl(nir_function_impl *impl)
{
   nir_builder b = nir_builder_create(impl);

   bool progress = false;
   nir_foreach_block(block, impl)
   {
      nir_foreach_instr_safe(instr, block)
      {

         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *op = nir_instr_as_intrinsic(instr);
         if (op->intrinsic != nir_intrinsic_load_shared &&
             op->intrinsic != nir_intrinsic_store_shared)
            continue;

         b.cursor = nir_before_instr(instr);

         if (op->intrinsic == nir_intrinsic_load_shared) {
            nir_def *addr = op->src[0].ssa;

            switch (op->def.num_components) {
            case 2: {
               auto addr2 = nir_iadd_imm(&b, addr, 4);
               addr = nir_vec2(&b, addr, addr2);
               break;
            }
            case 3: {
               auto addr2 = nir_iadd(&b, addr, nir_imm_ivec2(&b, 4, 8));
               addr =
                  nir_vec3(&b, addr, nir_channel(&b, addr2, 0), nir_channel(&b, addr2, 1));
               break;
            }
            case 4: {
               addr = nir_iadd(&b, addr, nir_imm_ivec4(&b, 0, 4, 8, 12));
               break;
            }
            }

            auto load =
               nir_intrinsic_instr_create(b.shader, nir_intrinsic_load_local_shared_r600);
            load->num_components = op->def.num_components;
            load->src[0] = nir_src_for_ssa(addr);
            nir_def_init(&load->instr, &load->def, load->num_components,
                         32);
            nir_def_rewrite_uses(&op->def, &load->def);
            nir_builder_instr_insert(&b, &load->instr);
         } else {
            nir_def *addr = op->src[1].ssa;
            for (int i = 0; i < 2; ++i) {
               unsigned test_mask = (0x3 << 2 * i);
               if (!(nir_intrinsic_write_mask(op) & test_mask))
                  continue;

               auto store =
                  nir_intrinsic_instr_create(b.shader,
                                             nir_intrinsic_store_local_shared_r600);
               unsigned writemask = nir_intrinsic_write_mask(op) & test_mask;
               nir_intrinsic_set_write_mask(store, writemask);
               store->src[0] = nir_src_for_ssa(op->src[0].ssa);
               store->num_components = store->src[0].ssa->num_components;
               bool start_even = (writemask & (1u << (2 * i)));

               auto addr2 =
                  nir_iadd_imm(&b, addr, 8 * i + (start_even ? 0 : 4));
               store->src[1] = nir_src_for_ssa(addr2);

               nir_builder_instr_insert(&b, &store->instr);
            }
         }
         nir_instr_remove(instr);
         progress = true;
      }
   }
   return progress;
}

static bool
r600_lower_shared_io(nir_shader *nir)
{
   bool progress = false;
   nir_foreach_function_impl(impl, nir)
   {
      if (r600_lower_shared_io_impl(impl))
         progress = true;
   }
   return progress;
}

static nir_def *
r600_lower_fs_pos_input_impl(nir_builder *b, nir_instr *instr, void *_options)
{
   (void)_options;
   auto old_ir = nir_instr_as_intrinsic(instr);
   auto load = nir_intrinsic_instr_create(b->shader, nir_intrinsic_load_input);
   nir_def_init(&load->instr, &load->def,
                old_ir->def.num_components, old_ir->def.bit_size);
   nir_intrinsic_set_io_semantics(load, nir_intrinsic_io_semantics(old_ir));

   nir_intrinsic_set_base(load, nir_intrinsic_base(old_ir));
   nir_intrinsic_set_component(load, nir_intrinsic_component(old_ir));
   nir_intrinsic_set_dest_type(load, nir_type_float32);
   load->num_components = old_ir->num_components;
   load->src[0] = old_ir->src[1];
   nir_builder_instr_insert(b, &load->instr);
   return &load->def;
}

bool
r600_lower_fs_pos_input_filter(const nir_instr *instr, const void *_options)
{
   (void)_options;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   auto ir = nir_instr_as_intrinsic(instr);
   if (ir->intrinsic != nir_intrinsic_load_interpolated_input)
      return false;

   return nir_intrinsic_io_semantics(ir).location == VARYING_SLOT_POS;
}

/* Strip the interpolator specification, it is not needed and irritates */
bool
r600_lower_fs_pos_input(nir_shader *shader)
{
   return nir_shader_lower_instructions(shader,
                                        r600_lower_fs_pos_input_filter,
                                        r600_lower_fs_pos_input_impl,
                                        nullptr);
};

bool
r600_opt_indirect_fbo_loads(nir_shader *shader)
{
   return r600::OptIndirectUBOLoads().run(shader);
}

static bool
optimize_once(nir_shader *shader)
{
   bool progress = false;
   NIR_PASS(progress, shader, nir_lower_alu_to_scalar, r600_lower_to_scalar_instr_filter, NULL);
   NIR_PASS(progress, shader, nir_lower_vars_to_ssa);
   NIR_PASS(progress, shader, nir_copy_prop);
   NIR_PASS(progress, shader, nir_opt_dce);
   NIR_PASS(progress, shader, nir_opt_algebraic);
   NIR_PASS(progress, shader, nir_opt_constant_folding);
   NIR_PASS(progress, shader, nir_opt_copy_prop_vars);
   NIR_PASS(progress, shader, nir_opt_remove_phis);

   if (nir_opt_loop(shader)) {
      progress = true;
      NIR_PASS(progress, shader, nir_copy_prop);
      NIR_PASS(progress, shader, nir_opt_dce);
   }

   NIR_PASS(progress, shader, nir_opt_if, nir_opt_if_optimize_phi_true_false);
   NIR_PASS(progress, shader, nir_opt_dead_cf);
   NIR_PASS(progress, shader, nir_opt_cse);
   NIR_PASS(progress, shader, nir_opt_peephole_select, 200, true, true);

   NIR_PASS(progress, shader, nir_opt_conditional_discard);
   NIR_PASS(progress, shader, nir_opt_dce);
   NIR_PASS(progress, shader, nir_opt_undef);
   NIR_PASS(progress, shader, nir_opt_loop_unroll);
   return progress;
}

static bool
r600_is_last_vertex_stage(nir_shader *nir, const r600_shader_key& key)
{
   if (nir->info.stage == MESA_SHADER_GEOMETRY)
      return true;

   if (nir->info.stage == MESA_SHADER_TESS_EVAL && !key.tes.as_es)
      return true;

   if (nir->info.stage == MESA_SHADER_VERTEX && !key.vs.as_es && !key.vs.as_ls)
      return true;

   return false;
}

extern "C" bool
r600_lower_to_scalar_instr_filter(const nir_instr *instr, const void *)
{
   if (instr->type != nir_instr_type_alu)
      return true;

   auto alu = nir_instr_as_alu(instr);
   switch (alu->op) {
   case nir_op_bany_fnequal3:
   case nir_op_bany_fnequal4:
   case nir_op_ball_fequal3:
   case nir_op_ball_fequal4:
   case nir_op_bany_inequal3:
   case nir_op_bany_inequal4:
   case nir_op_ball_iequal3:
   case nir_op_ball_iequal4:
   case nir_op_fdot2:
   case nir_op_fdot3:
   case nir_op_fdot4:
   case nir_op_fddx:
   case nir_op_fddx_coarse:
   case nir_op_fddx_fine:
   case nir_op_fddy:
   case nir_op_fddy_coarse:
   case nir_op_fddy_fine:
      return nir_src_bit_size(alu->src[0].src) == 64;
   default:
      return true;
   }
}

void
r600_finalize_nir_common(nir_shader *nir, enum amd_gfx_level gfx_level)
{
   const int nir_lower_flrp_mask = 16 | 32 | 64;

   NIR_PASS_V(nir, nir_lower_flrp, nir_lower_flrp_mask, false);

   nir_lower_idiv_options idiv_options = {0};
   NIR_PASS_V(nir, nir_lower_idiv, &idiv_options);

   NIR_PASS_V(nir, r600_nir_lower_trigen, gfx_level);
   NIR_PASS_V(nir, nir_lower_phis_to_scalar, false);
   NIR_PASS_V(nir, nir_lower_undef_to_zero);

   struct nir_lower_tex_options lower_tex_options = {0};
   lower_tex_options.lower_txp = ~0u;
   lower_tex_options.lower_txf_offset = true;
   lower_tex_options.lower_invalid_implicit_lod = true;
   lower_tex_options.lower_tg4_offsets = true;

   NIR_PASS_V(nir, nir_lower_tex, &lower_tex_options);
   NIR_PASS_V(nir, r600_nir_lower_txl_txf_array_or_cube);
   NIR_PASS_V(nir, r600_nir_lower_cube_to_2darray);

   NIR_PASS_V(nir, r600_nir_lower_pack_unpack_2x16);

   NIR_PASS_V(nir, r600_lower_shared_io);
   NIR_PASS_V(nir, r600_nir_lower_atomics);

   if (gfx_level == CAYMAN)
      NIR_PASS_V(nir, r600_legalize_image_load_store);

   while (optimize_once(nir))
      ;
}

DEBUG_GET_ONCE_NUM_OPTION(skip_opt_start, "R600_SFN_SKIP_OPT_START", -1);
DEBUG_GET_ONCE_NUM_OPTION(skip_opt_end, "R600_SFN_SKIP_OPT_END", -1);

void
r600_lower_and_optimize_nir(nir_shader *sh,
                            const union r600_shader_key *key,
                            enum amd_gfx_level gfx_level,
                            struct pipe_stream_output_info *so_info)
{
   bool lower_64bit =
      gfx_level < CAYMAN &&
      (sh->options->lower_int64_options || sh->options->lower_doubles_options) &&
      ((sh->info.bit_sizes_float | sh->info.bit_sizes_int) & 64);

   r600::sort_uniforms(sh);

   while (optimize_once(sh))
      ;


   if (sh->info.stage == MESA_SHADER_VERTEX)
      NIR_PASS_V(sh, r600_vectorize_vs_inputs);

   if (sh->info.stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS_V(sh, nir_lower_fragcoord_wtrans);
      NIR_PASS_V(sh, r600_lower_fs_out_to_vector);
      NIR_PASS_V(sh, nir_opt_dce);
      NIR_PASS_V(sh, nir_remove_dead_variables, nir_var_shader_out, 0);
      r600::sort_fsoutput(sh);
   }
   nir_variable_mode io_modes = nir_var_uniform | nir_var_shader_in | nir_var_shader_out;

   NIR_PASS_V(sh, nir_opt_combine_stores, nir_var_shader_out);
   NIR_PASS_V(sh,
              nir_lower_io,
              io_modes,
              r600_glsl_type_size,
              nir_lower_io_lower_64bit_to_32);

   if (sh->info.stage == MESA_SHADER_FRAGMENT)
      NIR_PASS_V(sh, r600_lower_fs_pos_input);

   /**/
   if (lower_64bit)
      NIR_PASS_V(sh, nir_lower_indirect_derefs, nir_var_function_temp, 10);

   NIR_PASS_V(sh, nir_opt_constant_folding);
   NIR_PASS_V(sh, nir_io_add_const_offset_to_base, io_modes);

   NIR_PASS_V(sh, nir_lower_alu_to_scalar, r600_lower_to_scalar_instr_filter, NULL);
   NIR_PASS_V(sh, nir_lower_phis_to_scalar, false);
   if (lower_64bit)
      NIR_PASS_V(sh, r600::r600_nir_split_64bit_io);
   NIR_PASS_V(sh, nir_lower_alu_to_scalar, r600_lower_to_scalar_instr_filter, NULL);
   NIR_PASS_V(sh, nir_lower_phis_to_scalar, false);
   NIR_PASS_V(sh, nir_lower_alu_to_scalar, r600_lower_to_scalar_instr_filter, NULL);
   NIR_PASS_V(sh, nir_copy_prop);
   NIR_PASS_V(sh, nir_opt_dce);



   if (r600_is_last_vertex_stage(sh, *key))
      r600_lower_clipvertex_to_clipdist(sh, *so_info);

   if (sh->info.stage == MESA_SHADER_TESS_CTRL ||
       sh->info.stage == MESA_SHADER_TESS_EVAL ||
       (sh->info.stage == MESA_SHADER_VERTEX && key->vs.as_ls)) {
      auto prim_type = sh->info.stage == MESA_SHADER_TESS_EVAL
                          ? u_tess_prim_from_shader(sh->info.tess._primitive_mode)
                          : (mesa_prim)key->tcs.prim_mode;
      NIR_PASS_V(sh, r600_lower_tess_io, static_cast<mesa_prim>(prim_type));
   }

   if (sh->info.stage == MESA_SHADER_TESS_CTRL)
      NIR_PASS_V(sh, r600_append_tcs_TF_emission, (mesa_prim)key->tcs.prim_mode);

   if (sh->info.stage == MESA_SHADER_TESS_EVAL) {
      NIR_PASS_V(sh, nir_lower_tess_coord_z,
                 sh->info.tess._primitive_mode == TESS_PRIMITIVE_TRIANGLES);
   }

   NIR_PASS_V(sh, nir_lower_alu_to_scalar, r600_lower_to_scalar_instr_filter, NULL);
   NIR_PASS_V(sh, nir_lower_phis_to_scalar, false);
   NIR_PASS_V(sh, nir_lower_alu_to_scalar, r600_lower_to_scalar_instr_filter, NULL);
   NIR_PASS_V(sh, r600_nir_lower_int_tg4);
   NIR_PASS_V(sh, r600::r600_nir_lower_tex_to_backend, gfx_level);

   if ((sh->info.bit_sizes_float | sh->info.bit_sizes_int) & 64) {
      NIR_PASS_V(sh, r600::r600_nir_split_64bit_io);
      NIR_PASS_V(sh, r600::r600_split_64bit_alu_and_phi);
      NIR_PASS_V(sh, nir_split_64bit_vec3_and_vec4);
      NIR_PASS_V(sh, nir_lower_int64);
   }

   NIR_PASS_V(sh, nir_lower_ubo_vec4);
   NIR_PASS_V(sh, r600_opt_indirect_fbo_loads);

   if (lower_64bit)
      NIR_PASS_V(sh, r600::r600_nir_64_to_vec2);

   if ((sh->info.bit_sizes_float | sh->info.bit_sizes_int) & 64)
      NIR_PASS_V(sh, r600::r600_split_64bit_uniforms_and_ubo);

   /* Lower to scalar to let some optimization work out better */
   while (optimize_once(sh))
      ;

   if (lower_64bit)
      NIR_PASS_V(sh, r600::r600_merge_vec2_stores);

   NIR_PASS_V(sh, nir_remove_dead_variables, nir_var_shader_in, NULL);
   NIR_PASS_V(sh, nir_remove_dead_variables, nir_var_shader_out, NULL);

   NIR_PASS_V(sh,
              nir_lower_vars_to_scratch,
              nir_var_function_temp,
              40,
              r600_get_natural_size_align_bytes);

   while (optimize_once(sh))
      ;

   if ((sh->info.bit_sizes_float | sh->info.bit_sizes_int) & 64)
      NIR_PASS_V(sh, r600::r600_split_64bit_alu_and_phi);

   bool late_algebraic_progress;
   do {
      late_algebraic_progress = false;
      NIR_PASS(late_algebraic_progress, sh, nir_opt_algebraic_late);
      NIR_PASS(late_algebraic_progress, sh, nir_opt_constant_folding);
      NIR_PASS(late_algebraic_progress, sh, nir_copy_prop);
      NIR_PASS(late_algebraic_progress, sh, nir_opt_dce);
      NIR_PASS(late_algebraic_progress, sh, nir_opt_cse);
   } while (late_algebraic_progress);

   NIR_PASS_V(sh, nir_lower_bool_to_int32);

   NIR_PASS_V(sh, nir_lower_locals_to_regs, 32);
   NIR_PASS_V(sh, nir_convert_from_ssa, true);
   NIR_PASS_V(sh, nir_opt_dce);
}

void
r600_finalize_and_optimize_shader(r600::Shader *shader)
{
   if (r600::sfn_log.has_debug_flag(r600::SfnLog::steps)) {
      std::cerr << "Shader after conversion from nir\n";
      shader->print(std::cerr);
   }

   auto sfn_skip_opt_start = debug_get_option_skip_opt_start();
   auto sfn_skip_opt_end = debug_get_option_skip_opt_end();
   bool skip_shader_opt_per_id = sfn_skip_opt_start >= 0 &&
                                 sfn_skip_opt_start <= shader->shader_id() &&
                                 sfn_skip_opt_end >= shader->shader_id();

   bool skip_shader_opt = r600::sfn_log.has_debug_flag(r600::SfnLog::noopt) ||
                          skip_shader_opt_per_id;

   if (!skip_shader_opt) {
      optimize(*shader);
      if (r600::sfn_log.has_debug_flag(r600::SfnLog::steps)) {
         std::cerr << "Shader after optimization\n";
         shader->print(std::cerr);
      }
   }

   split_address_loads(*shader);
   
   if (r600::sfn_log.has_debug_flag(r600::SfnLog::steps)) {
      std::cerr << "Shader after splitting address loads\n";
      shader->print(std::cerr);
   }
   
   if (!skip_shader_opt) {
      optimize(*shader);
      if (r600::sfn_log.has_debug_flag(r600::SfnLog::steps)) {
         std::cerr << "Shader after optimization\n";
         shader->print(std::cerr);
      }
   }
}

r600::Shader *
r600_schedule_shader(r600::Shader *shader)
{
   auto scheduled_shader = r600::schedule(shader);
   if (r600::sfn_log.has_debug_flag(r600::SfnLog::steps)) {
      std::cerr << "Shader after scheduling\n";
      scheduled_shader->print(std::cerr);
   }

   if (!r600::sfn_log.has_debug_flag(r600::SfnLog::nomerge)) {

      if (r600::sfn_log.has_debug_flag(r600::SfnLog::merge)) {
         r600::sfn_log << r600::SfnLog::merge << "Shader before RA\n";
         scheduled_shader->print(std::cerr);
      }

      r600::sfn_log << r600::SfnLog::trans << "Merge registers\n";
      auto lrm = r600::LiveRangeEvaluator().run(*scheduled_shader);

      if (!r600::register_allocation(lrm)) {
         R600_ERR("%s: Register allocation failed\n", __func__);
         /* For now crash if the shader could not be benerated */
         assert(0);
         return nullptr;
      } else if (r600::sfn_log.has_debug_flag(r600::SfnLog::merge) ||
                 r600::sfn_log.has_debug_flag(r600::SfnLog::steps)) {
         r600::sfn_log << "Shader after RA\n";
         scheduled_shader->print(std::cerr);
      }
   }

   return scheduled_shader;
}
