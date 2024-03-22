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

#include "sfn_nir_lower_fs_out_to_vector.h"

#include "nir_builder.h"
#include "nir_deref.h"
#include "util/u_math.h"

#include <algorithm>
#include <array>
#include <set>
#include <vector>

namespace r600 {

using std::array;
using std::multiset;
using std::vector;

struct nir_intrinsic_instr_less {
   bool operator()(const nir_intrinsic_instr *lhs, const nir_intrinsic_instr *rhs) const
   {
      nir_variable *vlhs = nir_intrinsic_get_var(lhs, 0);
      nir_variable *vrhs = nir_intrinsic_get_var(rhs, 0);

      auto ltype = glsl_get_base_type(vlhs->type);
      auto rtype = glsl_get_base_type(vrhs->type);

      if (ltype != rtype)
         return ltype < rtype;
      return vlhs->data.location < vrhs->data.location;
   }
};

class NirLowerIOToVector {
public:
   NirLowerIOToVector(int base_slot);
   bool run(nir_function_impl *shader);

protected:
   bool var_can_merge(const nir_variable *lhs, const nir_variable *rhs);
   bool var_can_rewrite(nir_variable *var) const;
   void create_new_io_vars(nir_shader *shader);
   void create_new_io_var(nir_shader *shader, unsigned location, unsigned comps);

   nir_deref_instr *clone_deref_array(nir_builder *b,
                                      nir_deref_instr *dst_tail,
                                      const nir_deref_instr *src_head);

   bool vectorize_block(nir_builder *b, nir_block *block);
   bool instr_can_rewrite(nir_instr *instr);
   bool vec_instr_set_remove(nir_builder *b, nir_instr *instr);

   using InstrSet = multiset<nir_intrinsic_instr *, nir_intrinsic_instr_less>;
   using InstrSubSet = std::pair<InstrSet::iterator, InstrSet::iterator>;

   bool
   vec_instr_stack_pop(nir_builder *b, InstrSubSet& ir_set, nir_intrinsic_instr *instr);

   array<array<nir_variable *, 4>, 16> m_vars;
   InstrSet m_block_io;
   int m_next_index;

private:
   virtual nir_variable_mode get_io_mode(nir_shader *shader) const = 0;
   virtual bool instr_can_rewrite_type(nir_intrinsic_instr *intr) const = 0;
   virtual bool var_can_rewrite_slot(nir_variable *var) const = 0;
   virtual void create_new_io(nir_builder *b,
                              nir_intrinsic_instr *intr,
                              nir_variable *var,
                              nir_def **srcs,
                              unsigned first_comp,
                              unsigned num_comps) = 0;

   int m_base_slot;
};

class NirLowerFSOutToVector : public NirLowerIOToVector {
public:
   NirLowerFSOutToVector();

private:
   nir_variable_mode get_io_mode(nir_shader *shader) const override;
   bool var_can_rewrite_slot(nir_variable *var) const override;
   void create_new_io(nir_builder *b,
                      nir_intrinsic_instr *intr,
                      nir_variable *var,
                      nir_def **srcs,
                      unsigned first_comp,
                      unsigned num_comps) override;
   bool instr_can_rewrite_type(nir_intrinsic_instr *intr) const override;

   nir_def *create_combined_vector(nir_builder *b,
                                       nir_def **srcs,
                                       int first_comp,
                                       int num_comp);
};

bool
r600_lower_fs_out_to_vector(nir_shader *shader)
{
   NirLowerFSOutToVector processor;

   assert(shader->info.stage == MESA_SHADER_FRAGMENT);
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      progress |= processor.run(impl);
   }
   return progress;
}

NirLowerIOToVector::NirLowerIOToVector(int base_slot):
    m_next_index(0),
    m_base_slot(base_slot)
{
   for (auto& a : m_vars)
      for (auto& aa : a)
         aa = nullptr;
}

bool
NirLowerIOToVector::run(nir_function_impl *impl)
{
   nir_builder b = nir_builder_create(impl);

   nir_metadata_require(impl, nir_metadata_dominance);
   create_new_io_vars(impl->function->shader);

   bool progress = vectorize_block(&b, nir_start_block(impl));
   if (progress) {
      nir_metadata_preserve(impl, nir_metadata_block_index | nir_metadata_dominance);
   } else {
      nir_metadata_preserve(impl, nir_metadata_all);
   }
   return progress;
}

void
NirLowerIOToVector::create_new_io_vars(nir_shader *shader)
{
   nir_variable_mode mode = get_io_mode(shader);

   bool can_rewrite_vars = false;
   nir_foreach_variable_with_modes(var, shader, mode)
   {
      if (var_can_rewrite(var)) {
         can_rewrite_vars = true;
         unsigned loc = var->data.location - m_base_slot;
         m_vars[loc][var->data.location_frac] = var;
      }
   }

   if (!can_rewrite_vars)
      return;

   /* We don't handle combining vars of different type e.g. different array
    * lengths.
    */
   for (unsigned i = 0; i < 16; i++) {
      unsigned comps = 0;

      for (unsigned j = 0; j < 3; j++) {
         if (!m_vars[i][j])
            continue;

         for (unsigned k = j + 1; k < 4; k++) {
            if (!m_vars[i][k])
               continue;

            if (!var_can_merge(m_vars[i][j], m_vars[i][k]))
               continue;

            /* Set comps */
            for (unsigned n = 0; n < glsl_get_components(m_vars[i][j]->type); ++n)
               comps |= 1 << (m_vars[i][j]->data.location_frac + n);

            for (unsigned n = 0; n < glsl_get_components(m_vars[i][k]->type); ++n)
               comps |= 1 << (m_vars[i][k]->data.location_frac + n);
         }
      }
      if (comps)
         create_new_io_var(shader, i, comps);
   }
}

bool
NirLowerIOToVector::var_can_merge(const nir_variable *lhs, const nir_variable *rhs)
{
   return (glsl_get_base_type(lhs->type) == glsl_get_base_type(rhs->type));
}

void
NirLowerIOToVector::create_new_io_var(nir_shader *shader,
                                      unsigned location,
                                      unsigned comps)
{
   unsigned num_comps = util_bitcount(comps);
   assert(num_comps > 1);

   /* Note: u_bit_scan() strips a component of the comps bitfield here */
   unsigned first_comp = u_bit_scan(&comps);

   nir_variable *var = nir_variable_clone(m_vars[location][first_comp], shader);
   var->data.location_frac = first_comp;
   var->type = glsl_replace_vector_type(var->type, num_comps);

   nir_shader_add_variable(shader, var);

   m_vars[location][first_comp] = var;

   while (comps) {
      const int comp = u_bit_scan(&comps);
      if (m_vars[location][comp]) {
         m_vars[location][comp] = var;
      }
   }
}

bool
NirLowerIOToVector::var_can_rewrite(nir_variable *var) const
{
   /* Skip complex types we don't split in the first place */
   if (!glsl_type_is_vector_or_scalar(glsl_without_array(var->type)))
      return false;

   if (glsl_get_bit_size(glsl_without_array(var->type)) != 32)
      return false;

   return var_can_rewrite_slot(var);
}

bool
NirLowerIOToVector::vectorize_block(nir_builder *b, nir_block *block)
{
   bool progress = false;

   nir_foreach_instr_safe(instr, block)
   {
      if (instr_can_rewrite(instr)) {
         instr->index = m_next_index++;
         nir_intrinsic_instr *ir = nir_instr_as_intrinsic(instr);
         m_block_io.insert(ir);
      }
   }

   for (unsigned i = 0; i < block->num_dom_children; i++) {
      nir_block *child = block->dom_children[i];
      progress |= vectorize_block(b, child);
   }

   nir_foreach_instr_reverse_safe(instr, block)
   {
      progress |= vec_instr_set_remove(b, instr);
   }
   m_block_io.clear();

   return progress;
}

bool
NirLowerIOToVector::instr_can_rewrite(nir_instr *instr)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   if (intr->num_components > 3)
      return false;

   return instr_can_rewrite_type(intr);
}

bool
NirLowerIOToVector::vec_instr_set_remove(nir_builder *b, nir_instr *instr)
{
   if (!instr_can_rewrite(instr))
      return false;

   nir_intrinsic_instr *ir = nir_instr_as_intrinsic(instr);
   auto entry = m_block_io.equal_range(ir);
   if (entry.first != m_block_io.end()) {
      vec_instr_stack_pop(b, entry, ir);
   }
   return true;
}

nir_deref_instr *
NirLowerIOToVector::clone_deref_array(nir_builder *b,
                                      nir_deref_instr *dst_tail,
                                      const nir_deref_instr *src_head)
{
   const nir_deref_instr *parent = nir_deref_instr_parent(src_head);

   if (!parent)
      return dst_tail;

   assert(src_head->deref_type == nir_deref_type_array);

   dst_tail = clone_deref_array(b, dst_tail, parent);

   return nir_build_deref_array(b, dst_tail, src_head->arr.index.ssa);
}

NirLowerFSOutToVector::NirLowerFSOutToVector():
    NirLowerIOToVector(FRAG_RESULT_COLOR)
{
}

bool
NirLowerFSOutToVector::var_can_rewrite_slot(nir_variable *var) const
{
   return ((var->data.mode == nir_var_shader_out) &&
           ((var->data.location == FRAG_RESULT_COLOR) ||
            ((var->data.location >= FRAG_RESULT_DATA0) &&
             (var->data.location <= FRAG_RESULT_DATA7))));
}

bool
NirLowerIOToVector::vec_instr_stack_pop(nir_builder *b,
                                        InstrSubSet& ir_set,
                                        nir_intrinsic_instr *instr)
{
   vector<nir_intrinsic_instr *> ir_sorted_set(ir_set.first, ir_set.second);
   std::sort(ir_sorted_set.begin(),
             ir_sorted_set.end(),
             [](const nir_intrinsic_instr *lhs, const nir_intrinsic_instr *rhs) {
                return lhs->instr.index > rhs->instr.index;
             });

   nir_intrinsic_instr *intr = *ir_sorted_set.begin();
   nir_variable *var = nir_intrinsic_get_var(intr, 0);

   unsigned loc = var->data.location - m_base_slot;

   nir_variable *new_var = m_vars[loc][var->data.location_frac];
   unsigned num_comps = glsl_get_vector_elements(glsl_without_array(new_var->type));
   unsigned old_num_comps = glsl_get_vector_elements(glsl_without_array(var->type));

   /* Don't bother walking the stack if this component can't be vectorised. */
   if (old_num_comps > 3) {
      return false;
   }

   if (new_var == var) {
      return false;
   }

   b->cursor = nir_after_instr(&intr->instr);
   nir_undef_instr *instr_undef = nir_undef_instr_create(b->shader, 1, 32);
   nir_builder_instr_insert(b, &instr_undef->instr);

   nir_def *srcs[4];
   for (int i = 0; i < 4; i++) {
      srcs[i] = &instr_undef->def;
   }
   srcs[var->data.location_frac] = intr->src[1].ssa;

   for (auto k = ir_sorted_set.begin() + 1; k != ir_sorted_set.end(); ++k) {
      nir_intrinsic_instr *intr2 = *k;
      nir_variable *var2 = nir_intrinsic_get_var(intr2, 0);
      unsigned loc2 = var->data.location - m_base_slot;

      if (m_vars[loc][var->data.location_frac] !=
          m_vars[loc2][var2->data.location_frac]) {
         continue;
      }

      assert(glsl_get_vector_elements(glsl_without_array(var2->type)) < 4);

      if (srcs[var2->data.location_frac] == &instr_undef->def) {
         assert(intr2->src[1].ssa);
         srcs[var2->data.location_frac] = intr2->src[1].ssa;
      }
      nir_instr_remove(&intr2->instr);
   }

   create_new_io(b, intr, new_var, srcs, new_var->data.location_frac, num_comps);
   return true;
}

nir_variable_mode
NirLowerFSOutToVector::get_io_mode(nir_shader *shader) const
{
   return nir_var_shader_out;
}

void
NirLowerFSOutToVector::create_new_io(nir_builder *b,
                                     nir_intrinsic_instr *intr,
                                     nir_variable *var,
                                     nir_def **srcs,
                                     unsigned first_comp,
                                     unsigned num_comps)
{
   b->cursor = nir_before_instr(&intr->instr);

   nir_intrinsic_instr *new_intr = nir_intrinsic_instr_create(b->shader, intr->intrinsic);
   new_intr->num_components = num_comps;

   nir_intrinsic_set_write_mask(new_intr, (1 << num_comps) - 1);

   nir_deref_instr *deref = nir_build_deref_var(b, var);
   deref = clone_deref_array(b, deref, nir_src_as_deref(intr->src[0]));

   new_intr->src[0] = nir_src_for_ssa(&deref->def);
   new_intr->src[1] =
      nir_src_for_ssa(create_combined_vector(b, srcs, first_comp, num_comps));

   nir_builder_instr_insert(b, &new_intr->instr);

   /* Remove the old store intrinsic */
   nir_instr_remove(&intr->instr);
}

bool
NirLowerFSOutToVector::instr_can_rewrite_type(nir_intrinsic_instr *intr) const
{
   if (intr->intrinsic != nir_intrinsic_store_deref)
      return false;

   nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
   if (!nir_deref_mode_is(deref, nir_var_shader_out))
      return false;

   return var_can_rewrite(nir_deref_instr_get_variable(deref));
}

nir_def *
NirLowerFSOutToVector::create_combined_vector(nir_builder *b,
                                              nir_def **srcs,
                                              int first_comp,
                                              int num_comp)
{
   nir_op op;
   switch (num_comp) {
   case 2:
      op = nir_op_vec2;
      break;
   case 3:
      op = nir_op_vec3;
      break;
   case 4:
      op = nir_op_vec4;
      break;
   default:
      unreachable("combined vector must have 2 to 4 components");
   }
   nir_alu_instr *instr = nir_alu_instr_create(b->shader, op);
   instr->exact = b->exact;

   int i = 0;
   unsigned k = 0;
   while (i < num_comp) {
      nir_def *s = srcs[first_comp + k];
      for (uint8_t kk = 0; kk < s->num_components && i < num_comp; ++kk) {
         instr->src[i].src = nir_src_for_ssa(s);
         instr->src[i].swizzle[0] = kk;
         ++i;
      }
      k += s->num_components;
   }

   nir_def_init(&instr->instr, &instr->def, num_comp, 32);
   nir_builder_instr_insert(b, &instr->instr);
   return &instr->def;
}

} // namespace r600
