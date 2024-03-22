/*
 * Copyright © 2020 Google LLC
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file
 *
 * Removes unused components of SSA defs.
 *
 * Due to various optimization passes (or frontend implementations,
 * particularly prog_to_nir), we may have instructions generating vectors
 * whose components don't get read by any instruction.
 *
 * For memory loads, while it can be tricky to eliminate unused low components
 * or channels in the middle of a writemask (you might need to increment some
 * offset from a load_uniform, for example), it is trivial to just drop the
 * trailing components.
 * For vector ALU and load_const, only used by other ALU instructions,
 * this pass eliminates arbitrary channels as well as duplicate channels,
 * and reswizzles the uses.
 *
 * This pass is probably only of use to vector backends -- scalar backends
 * typically get unused def channel trimming by scalarizing and dead code
 * elimination.
 */

#include "util/u_math.h"
#include "nir.h"
#include "nir_builder.h"

/*
 * Round up a vector size to a vector size that's valid in NIR. At present, NIR
 * supports only vec2-5, vec8, and vec16. Attempting to generate other sizes
 * will fail validation.
 */
static unsigned
round_up_components(unsigned n)
{
   return (n > 5) ? util_next_power_of_two(n) : n;
}

static bool
shrink_dest_to_read_mask(nir_def *def)
{
   /* early out if there's nothing to do. */
   if (def->num_components == 1)
      return false;

   /* don't remove any channels if used by an intrinsic */
   nir_foreach_use(use_src, def) {
      if (nir_src_parent_instr(use_src)->type == nir_instr_type_intrinsic)
         return false;
   }

   unsigned mask = nir_def_components_read(def);
   int last_bit = util_last_bit(mask);

   /* If nothing was read, leave it up to DCE. */
   if (!mask)
      return false;

   unsigned rounded = round_up_components(last_bit);
   assert(rounded <= def->num_components);
   last_bit = rounded;

   if (def->num_components > last_bit) {
      def->num_components = last_bit;
      return true;
   }

   return false;
}

static bool
shrink_intrinsic_to_non_sparse(nir_intrinsic_instr *instr)
{
   unsigned mask = nir_def_components_read(&instr->def);
   int last_bit = util_last_bit(mask);

   /* If the sparse component is used, do nothing. */
   if (last_bit == instr->def.num_components)
      return false;

   instr->def.num_components -= 1;
   instr->num_components = instr->def.num_components;

   /* Switch to the non-sparse intrinsic. */
   switch (instr->intrinsic) {
   case nir_intrinsic_image_sparse_load:
      instr->intrinsic = nir_intrinsic_image_load;
      break;
   case nir_intrinsic_bindless_image_sparse_load:
      instr->intrinsic = nir_intrinsic_bindless_image_load;
      break;
   case nir_intrinsic_image_deref_sparse_load:
      instr->intrinsic = nir_intrinsic_image_deref_load;
      break;
   default:
      break;
   }

   return true;
}

static void
reswizzle_alu_uses(nir_def *def, uint8_t *reswizzle)
{
   nir_foreach_use(use_src, def) {
      /* all uses must be ALU instructions */
      assert(nir_src_parent_instr(use_src)->type == nir_instr_type_alu);
      nir_alu_src *alu_src = (nir_alu_src *)use_src;

      /* reswizzle ALU sources */
      for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; i++)
         alu_src->swizzle[i] = reswizzle[alu_src->swizzle[i]];
   }
}

static bool
is_only_used_by_alu(nir_def *def)
{
   nir_foreach_use(use_src, def) {
      if (nir_src_parent_instr(use_src)->type != nir_instr_type_alu)
         return false;
   }

   return true;
}

static bool
opt_shrink_vector(nir_builder *b, nir_alu_instr *instr)
{
   nir_def *def = &instr->def;
   unsigned mask = nir_def_components_read(def);

   /* If nothing was read, leave it up to DCE. */
   if (mask == 0)
      return false;

   /* don't remove any channels if used by non-ALU */
   if (!is_only_used_by_alu(def))
      return false;

   uint8_t reswizzle[NIR_MAX_VEC_COMPONENTS] = { 0 };
   nir_scalar srcs[NIR_MAX_VEC_COMPONENTS] = { 0 };
   unsigned num_components = 0;
   for (unsigned i = 0; i < def->num_components; i++) {
      if (!((mask >> i) & 0x1))
         continue;

      nir_scalar scalar = nir_get_scalar(instr->src[i].src.ssa, instr->src[i].swizzle[0]);

      /* Try reuse a component with the same value */
      unsigned j;
      for (j = 0; j < num_components; j++) {
         if (nir_scalar_equal(scalar, srcs[j])) {
            reswizzle[i] = j;
            break;
         }
      }

      /* Otherwise, just append the value */
      if (j == num_components) {
         srcs[num_components] = scalar;
         reswizzle[i] = num_components++;
      }
   }

   /* return if no component was removed */
   if (num_components == def->num_components)
      return false;

   /* create new vecN and replace uses */
   nir_def *new_vec = nir_vec_scalars(b, srcs, num_components);
   nir_def_rewrite_uses(def, new_vec);
   reswizzle_alu_uses(new_vec, reswizzle);

   return true;
}

static bool
opt_shrink_vectors_alu(nir_builder *b, nir_alu_instr *instr)
{
   nir_def *def = &instr->def;

   /* Nothing to shrink */
   if (def->num_components == 1)
      return false;

   switch (instr->op) {
   /* don't use nir_op_is_vec() as not all vector sizes are supported. */
   case nir_op_vec4:
   case nir_op_vec3:
   case nir_op_vec2:
      return opt_shrink_vector(b, instr);
   default:
      if (nir_op_infos[instr->op].output_size != 0)
         return false;
      break;
   }

   /* don't remove any channels if used by non-ALU */
   if (!is_only_used_by_alu(def))
      return false;

   unsigned mask = nir_def_components_read(def);
   /* return, if there is nothing to do */
   if (mask == 0)
      return false;

   uint8_t reswizzle[NIR_MAX_VEC_COMPONENTS] = { 0 };
   unsigned num_components = 0;
   bool progress = false;
   for (unsigned i = 0; i < def->num_components; i++) {
      /* skip unused components */
      if (!((mask >> i) & 0x1))
         continue;

      /* Try reuse a component with the same swizzles */
      unsigned j;
      for (j = 0; j < num_components; j++) {
         bool duplicate_channel = true;
         for (unsigned k = 0; k < nir_op_infos[instr->op].num_inputs; k++) {
            if (nir_op_infos[instr->op].input_sizes[k] != 0 ||
                instr->src[k].swizzle[i] != instr->src[k].swizzle[j]) {
               duplicate_channel = false;
               break;
            }
         }

         if (duplicate_channel) {
            reswizzle[i] = j;
            progress = true;
            break;
         }
      }

      /* Otherwise, just append the value */
      if (j == num_components) {
         for (int k = 0; k < nir_op_infos[instr->op].num_inputs; k++) {
            instr->src[k].swizzle[num_components] = instr->src[k].swizzle[i];
         }
         if (i != num_components)
            progress = true;
         reswizzle[i] = num_components++;
      }
   }

   /* update uses */
   if (progress)
      reswizzle_alu_uses(def, reswizzle);

   unsigned rounded = round_up_components(num_components);
   assert(rounded <= def->num_components);
   if (rounded < def->num_components)
      progress = true;

   /* update dest */
   def->num_components = rounded;

   return progress;
}

static bool
opt_shrink_vectors_intrinsic(nir_builder *b, nir_intrinsic_instr *instr)
{
   switch (instr->intrinsic) {
   case nir_intrinsic_load_uniform:
   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_input:
   case nir_intrinsic_load_input_vertex:
   case nir_intrinsic_load_per_vertex_input:
   case nir_intrinsic_load_interpolated_input:
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_load_push_constant:
   case nir_intrinsic_load_constant:
   case nir_intrinsic_load_shared:
   case nir_intrinsic_load_global:
   case nir_intrinsic_load_global_constant:
   case nir_intrinsic_load_kernel_input:
   case nir_intrinsic_load_scratch: {
      /* Must be a vectorized intrinsic that we can resize. */
      assert(instr->num_components != 0);

      /* Trim the dest to the used channels */
      if (!shrink_dest_to_read_mask(&instr->def))
         return false;

      instr->num_components = instr->def.num_components;
      return true;
   }
   case nir_intrinsic_image_sparse_load:
   case nir_intrinsic_bindless_image_sparse_load:
   case nir_intrinsic_image_deref_sparse_load:
      return shrink_intrinsic_to_non_sparse(instr);
   default:
      return false;
   }
}

static bool
opt_shrink_vectors_tex(nir_builder *b, nir_tex_instr *tex)
{
   if (!tex->is_sparse)
      return false;

   unsigned mask = nir_def_components_read(&tex->def);
   int last_bit = util_last_bit(mask);

   /* If the sparse component is used, do nothing. */
   if (last_bit == tex->def.num_components)
      return false;

   tex->def.num_components -= 1;
   tex->is_sparse = false;

   return true;
}

static bool
opt_shrink_vectors_load_const(nir_load_const_instr *instr)
{
   nir_def *def = &instr->def;

   /* early out if there's nothing to do. */
   if (def->num_components == 1)
      return false;

   /* don't remove any channels if used by non-ALU */
   if (!is_only_used_by_alu(def))
      return false;

   unsigned mask = nir_def_components_read(def);

   /* If nothing was read, leave it up to DCE. */
   if (!mask)
      return false;

   uint8_t reswizzle[NIR_MAX_VEC_COMPONENTS] = { 0 };
   unsigned num_components = 0;
   bool progress = false;
   for (unsigned i = 0; i < def->num_components; i++) {
      if (!((mask >> i) & 0x1))
         continue;

      /* Try reuse a component with the same constant */
      unsigned j;
      for (j = 0; j < num_components; j++) {
         if (instr->value[i].u64 == instr->value[j].u64) {
            reswizzle[i] = j;
            progress = true;
            break;
         }
      }

      /* Otherwise, just append the value */
      if (j == num_components) {
         instr->value[num_components] = instr->value[i];
         if (i != num_components)
            progress = true;
         reswizzle[i] = num_components++;
      }
   }

   if (progress)
      reswizzle_alu_uses(def, reswizzle);

   unsigned rounded = round_up_components(num_components);
   assert(rounded <= def->num_components);
   if (rounded < def->num_components)
      progress = true;

   def->num_components = rounded;

   return progress;
}

static bool
opt_shrink_vectors_ssa_undef(nir_undef_instr *instr)
{
   return shrink_dest_to_read_mask(&instr->def);
}

static bool
opt_shrink_vectors_phi(nir_builder *b, nir_phi_instr *instr)
{
   nir_def *def = &instr->def;

   /* early out if there's nothing to do. */
   if (def->num_components == 1)
      return false;

   /* Ignore large vectors for now. */
   if (def->num_components > 4)
      return false;

   /* Check the uses. */
   nir_component_mask_t mask = 0;
   nir_foreach_use(src, def) {
      if (nir_src_parent_instr(src)->type != nir_instr_type_alu)
         return false;

      nir_alu_instr *alu = nir_instr_as_alu(nir_src_parent_instr(src));

      nir_alu_src *alu_src = exec_node_data(nir_alu_src, src, src);
      int src_idx = alu_src - &alu->src[0];
      nir_component_mask_t src_read_mask = nir_alu_instr_src_read_mask(alu, src_idx);

      nir_def *alu_def = &alu->def;

      /* We don't mark the channels used if the only reader is the original phi.
       * This can happen in the case of loops.
       */
      nir_foreach_use(alu_use_src, alu_def) {
         if (nir_src_parent_instr(alu_use_src) != &instr->instr) {
            mask |= src_read_mask;
         }
      }

      /* However, even if the instruction only points back at the phi, we still
       * need to check that the swizzles are trivial.
       */
      if (nir_op_is_vec(alu->op)) {
         if (src_idx != alu->src[src_idx].swizzle[0]) {
            mask |= src_read_mask;
         }
      } else if (!nir_alu_src_is_trivial_ssa(alu, src_idx)) {
         mask |= src_read_mask;
      }
   }

   /* DCE will handle this. */
   if (mask == 0)
      return false;

   /* Nothing to shrink? */
   if (BITFIELD_MASK(def->num_components) == mask)
      return false;

   /* Set up the reswizzles. */
   unsigned num_components = 0;
   uint8_t reswizzle[NIR_MAX_VEC_COMPONENTS] = { 0 };
   uint8_t src_reswizzle[NIR_MAX_VEC_COMPONENTS] = { 0 };
   for (unsigned i = 0; i < def->num_components; i++) {
      if (!((mask >> i) & 0x1))
         continue;
      src_reswizzle[num_components] = i;
      reswizzle[i] = num_components++;
   }

   /* Shrink the phi, this part is simple. */
   def->num_components = num_components;

   /* We can't swizzle phi sources directly so just insert extra mov
    * with the correct swizzle and let the other parts of nir_shrink_vectors
    * do its job on the original source instruction. If the original source was
    * used only in the phi, the movs will disappear later after copy propagate.
    */
   nir_foreach_phi_src(phi_src, instr) {
      b->cursor = nir_after_instr_and_phis(phi_src->src.ssa->parent_instr);

      nir_alu_src alu_src = {
         .src = nir_src_for_ssa(phi_src->src.ssa)
      };

      for (unsigned i = 0; i < num_components; i++)
         alu_src.swizzle[i] = src_reswizzle[i];
      nir_def *mov = nir_mov_alu(b, alu_src, num_components);

      nir_src_rewrite(&phi_src->src, mov);
   }
   b->cursor = nir_before_instr(&instr->instr);

   /* Reswizzle readers. */
   reswizzle_alu_uses(def, reswizzle);

   return true;
}

static bool
opt_shrink_vectors_instr(nir_builder *b, nir_instr *instr)
{
   b->cursor = nir_before_instr(instr);

   switch (instr->type) {
   case nir_instr_type_alu:
      return opt_shrink_vectors_alu(b, nir_instr_as_alu(instr));

   case nir_instr_type_tex:
      return opt_shrink_vectors_tex(b, nir_instr_as_tex(instr));

   case nir_instr_type_intrinsic:
      return opt_shrink_vectors_intrinsic(b, nir_instr_as_intrinsic(instr));

   case nir_instr_type_load_const:
      return opt_shrink_vectors_load_const(nir_instr_as_load_const(instr));

   case nir_instr_type_undef:
      return opt_shrink_vectors_ssa_undef(nir_instr_as_undef(instr));

   case nir_instr_type_phi:
      return opt_shrink_vectors_phi(b, nir_instr_as_phi(instr));

   default:
      return false;
   }

   return true;
}

bool
nir_opt_shrink_vectors(nir_shader *shader)
{
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      nir_builder b = nir_builder_create(impl);

      nir_foreach_block_reverse(block, impl) {
         nir_foreach_instr_reverse(instr, block) {
            progress |= opt_shrink_vectors_instr(&b, instr);
         }
      }

      if (progress) {
         nir_metadata_preserve(impl,
                               nir_metadata_block_index |
                                  nir_metadata_dominance);
      } else {
         nir_metadata_preserve(impl, nir_metadata_all);
      }
   }

   return progress;
}
