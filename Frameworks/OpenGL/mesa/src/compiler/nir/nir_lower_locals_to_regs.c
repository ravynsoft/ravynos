/*
 * Copyright © 2014 Intel Corporation
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

#include "nir.h"
#include "nir_builder.h"
#include "nir_builder_opcodes.h"
#include "nir_intrinsics_indices.h"

struct locals_to_regs_state {
   nir_builder builder;

   /* A hash table mapping derefs to register handles */
   struct hash_table *regs_table;

   /* Bit size to use for boolean registers */
   uint8_t bool_bitsize;

   bool progress;
};

/* The following two functions implement a hash and equality check for
 * variable dreferences.  When the hash or equality function encounters an
 * array, it ignores the offset and whether it is direct or indirect
 * entirely.
 */
static uint32_t
hash_deref(const void *void_deref)
{
   uint32_t hash = 0;

   for (const nir_deref_instr *deref = void_deref; deref;
        deref = nir_deref_instr_parent(deref)) {
      switch (deref->deref_type) {
      case nir_deref_type_var:
         return XXH32(&deref->var, sizeof(deref->var), hash);

      case nir_deref_type_array:
         continue; /* Do nothing */

      case nir_deref_type_struct:
         hash = XXH32(&deref->strct.index, sizeof(deref->strct.index), hash);
         continue;

      default:
         unreachable("Invalid deref type");
      }
   }

   unreachable("We should have hit a variable dereference");
}

static bool
derefs_equal(const void *void_a, const void *void_b)
{
   for (const nir_deref_instr *a = void_a, *b = void_b; a || b;
        a = nir_deref_instr_parent(a), b = nir_deref_instr_parent(b)) {
      if (a->deref_type != b->deref_type)
         return false;

      switch (a->deref_type) {
      case nir_deref_type_var:
         return a->var == b->var;

      case nir_deref_type_array:
         continue; /* Do nothing */

      case nir_deref_type_struct:
         if (a->strct.index != b->strct.index)
            return false;
         continue;

      default:
         unreachable("Invalid deref type");
      }
   }

   unreachable("We should have hit a variable dereference");
}

static nir_def *
get_reg_for_deref(nir_deref_instr *deref, struct locals_to_regs_state *state)
{
   uint32_t hash = hash_deref(deref);

   assert(nir_deref_instr_get_variable(deref)->constant_initializer == NULL &&
          nir_deref_instr_get_variable(deref)->pointer_initializer == NULL);

   struct hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(state->regs_table, hash, deref);
   if (entry)
      return entry->data;

   unsigned array_size = 1;
   for (nir_deref_instr *d = deref; d; d = nir_deref_instr_parent(d)) {
      if (d->deref_type == nir_deref_type_array)
         array_size *= glsl_get_length(nir_deref_instr_parent(d)->type);
   }

   assert(glsl_type_is_vector_or_scalar(deref->type));

   uint8_t bit_size = glsl_get_bit_size(deref->type);
   if (bit_size == 1)
      bit_size = state->bool_bitsize;

   nir_def *reg = nir_decl_reg(&state->builder,
                               glsl_get_vector_elements(deref->type),
                               bit_size, array_size > 1 ? array_size : 0);

   _mesa_hash_table_insert_pre_hashed(state->regs_table, hash, deref, reg);

   return reg;
}

struct reg_location {
   nir_def *reg;
   nir_def *indirect;
   unsigned base_offset;
};

static struct reg_location
get_deref_reg_location(nir_deref_instr *deref,
                       struct locals_to_regs_state *state)
{
   nir_builder *b = &state->builder;

   nir_def *reg = get_reg_for_deref(deref, state);
   nir_intrinsic_instr *decl = nir_instr_as_intrinsic(reg->parent_instr);

   /* It is possible for a user to create a shader that has an array with a
    * single element and then proceed to access it indirectly.  Indirectly
    * accessing a non-array register is not allowed in NIR.  In order to
    * handle this case we just convert it to a direct reference.
    */
   if (nir_intrinsic_num_array_elems(decl) == 0)
      return (struct reg_location){ .reg = reg };

   nir_def *indirect = NULL;
   unsigned base_offset = 0;

   unsigned inner_array_size = 1;
   for (const nir_deref_instr *d = deref; d; d = nir_deref_instr_parent(d)) {
      if (d->deref_type != nir_deref_type_array)
         continue;

      if (nir_src_is_const(d->arr.index) && !indirect) {
         base_offset += nir_src_as_uint(d->arr.index) * inner_array_size;
      } else {
         if (indirect) {
            assert(base_offset == 0);
         } else {
            indirect = nir_imm_int(b, base_offset);
            base_offset = 0;
         }

         nir_def *index = nir_i2iN(b, d->arr.index.ssa, 32);
         nir_def *offset = nir_imul_imm(b, index, inner_array_size);

         /* Avoid emitting iadd with 0, which is otherwise common, since this
          * pass runs late enough that nothing will clean it up.
          */
         nir_scalar scal = nir_get_scalar(indirect, 0);
         if (nir_scalar_is_const(scal))
            indirect = nir_iadd_imm(b, offset, nir_scalar_as_uint(scal));
         else
            indirect = nir_iadd(b, offset, indirect);
      }

      inner_array_size *= glsl_get_length(nir_deref_instr_parent(d)->type);
   }

   return (struct reg_location){
      .reg = reg,
      .indirect = indirect,
      .base_offset = base_offset
   };
}

static bool
lower_locals_to_regs_block(nir_block *block,
                           struct locals_to_regs_state *state)
{
   nir_builder *b = &state->builder;

   nir_foreach_instr_safe(instr, block) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

      switch (intrin->intrinsic) {
      case nir_intrinsic_load_deref: {
         nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
         if (!nir_deref_mode_is(deref, nir_var_function_temp))
            continue;

         b->cursor = nir_after_instr(&intrin->instr);
         struct reg_location loc = get_deref_reg_location(deref, state);
         nir_intrinsic_instr *decl = nir_reg_get_decl(loc.reg);

         nir_def *value;
         unsigned num_array_elems = nir_intrinsic_num_array_elems(decl);
         unsigned num_components = nir_intrinsic_num_components(decl);
         unsigned bit_size = nir_intrinsic_bit_size(decl);

         if (loc.base_offset >= MAX2(num_array_elems, 1)) {
            /* out-of-bounds read, return 0 instead. */
            value = nir_imm_zero(b, num_components, bit_size);
         } else if (loc.indirect != NULL) {
            value = nir_load_reg_indirect(b, num_components, bit_size,
                                          loc.reg, loc.indirect,
                                          .base = loc.base_offset);
         } else {
            value = nir_build_load_reg(b, num_components, bit_size,
                                       loc.reg, .base = loc.base_offset);
         }

         nir_def_rewrite_uses(&intrin->def, value);
         nir_instr_remove(&intrin->instr);
         state->progress = true;
         break;
      }

      case nir_intrinsic_store_deref: {
         nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
         if (!nir_deref_mode_is(deref, nir_var_function_temp))
            continue;

         b->cursor = nir_before_instr(&intrin->instr);

         struct reg_location loc = get_deref_reg_location(deref, state);
         nir_intrinsic_instr *decl = nir_reg_get_decl(loc.reg);

         nir_def *val = intrin->src[1].ssa;
         unsigned num_array_elems = nir_intrinsic_num_array_elems(decl);
         unsigned write_mask = nir_intrinsic_write_mask(intrin);

         if (loc.base_offset >= MAX2(num_array_elems, 1)) {
            /* Out of bounds write, just eliminate it. */
         } else if (loc.indirect) {
            nir_store_reg_indirect(b, val, loc.reg, loc.indirect,
                                   .base = loc.base_offset,
                                   .write_mask = write_mask);
         } else {
            nir_build_store_reg(b, val, loc.reg, .base = loc.base_offset,
                                .write_mask = write_mask);
         }

         nir_instr_remove(&intrin->instr);
         state->progress = true;
         break;
      }

      case nir_intrinsic_copy_deref:
         unreachable("There should be no copies whatsoever at this point");
         break;

      default:
         continue;
      }
   }

   return true;
}

static bool
impl(nir_function_impl *impl, uint8_t bool_bitsize)
{
   struct locals_to_regs_state state;

   state.builder = nir_builder_create(impl);
   state.progress = false;
   state.regs_table = _mesa_hash_table_create(NULL, hash_deref, derefs_equal);
   state.bool_bitsize = bool_bitsize;

   nir_metadata_require(impl, nir_metadata_dominance);

   nir_foreach_block(block, impl) {
      lower_locals_to_regs_block(block, &state);
   }

   nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);

   _mesa_hash_table_destroy(state.regs_table, NULL);

   return state.progress;
}

bool
nir_lower_locals_to_regs(nir_shader *shader, uint8_t bool_bitsize)
{
   bool progress = false;

   nir_foreach_function_impl(func_impl, shader) {
      progress = impl(func_impl, bool_bitsize) || progress;
   }

   return progress;
}
