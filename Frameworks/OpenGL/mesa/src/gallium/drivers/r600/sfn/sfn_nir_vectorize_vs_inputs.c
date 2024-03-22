/*
 * Copyright © 2018 Timothy Arceri
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "nir.h"
#include "nir_builder.h"
#include "nir_deref.h"
#include "util/u_dynarray.h"
#include "util/u_math.h"
#define XXH_INLINE_ALL
#include "util/xxhash.h"

/** @file nir_opt_vectorize_io.c
 *
 * Replaces scalar nir_load_input/nir_store_output operations with
 * vectorized instructions.
 */
bool
r600_vectorize_vs_inputs(nir_shader *shader);

static nir_deref_instr *
r600_clone_deref_array(nir_builder *b,
                       nir_deref_instr *dst_tail,
                       const nir_deref_instr *src_head)
{
   const nir_deref_instr *parent = nir_deref_instr_parent(src_head);

   if (!parent)
      return dst_tail;

   assert(src_head->deref_type == nir_deref_type_array);

   dst_tail = r600_clone_deref_array(b, dst_tail, parent);

   return nir_build_deref_array(b, dst_tail, src_head->arr.index.ssa);
}

static bool
r600_variable_can_rewrite(nir_variable *var)
{

   /* Skip complex types we don't split in the first place */
   if (!glsl_type_is_vector_or_scalar(glsl_without_array(var->type)))
      return false;

   /* TODO: add 64/16bit support ? */
   if (glsl_get_bit_size(glsl_without_array(var->type)) != 32)
      return false;

   /* We only check VSand attribute imputs */
   return (var->data.location >= VERT_ATTRIB_GENERIC0 &&
           var->data.location <= VERT_ATTRIB_GENERIC15);
}

static bool
r600_instr_can_rewrite(nir_instr *instr)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   if (intr->num_components > 3)
      return false;

   if (intr->intrinsic != nir_intrinsic_load_deref)
      return false;

   nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
   if (!nir_deref_mode_is(deref, nir_var_shader_in))
      return false;

   return r600_variable_can_rewrite(nir_deref_instr_get_variable(deref));
}

static bool
r600_io_access_same_var(const nir_instr *instr1, const nir_instr *instr2)
{
   assert(instr1->type == nir_instr_type_intrinsic &&
          instr2->type == nir_instr_type_intrinsic);

   nir_intrinsic_instr *intr1 = nir_instr_as_intrinsic(instr1);
   nir_intrinsic_instr *intr2 = nir_instr_as_intrinsic(instr2);

   nir_variable *var1 = nir_intrinsic_get_var(intr1, 0);
   nir_variable *var2 = nir_intrinsic_get_var(intr2, 0);

   /* We don't handle combining vars of different base types, so skip those */
   if (glsl_get_base_type(var1->type) != glsl_get_base_type(var2->type))
      return false;

   if (var1->data.location != var2->data.location)
      return false;

   return true;
}

static struct util_dynarray *
r600_vec_instr_stack_create(void *mem_ctx)
{
   struct util_dynarray *stack = ralloc(mem_ctx, struct util_dynarray);
   util_dynarray_init(stack, mem_ctx);
   return stack;
}

static void
r600_vec_instr_stack_push(struct util_dynarray *stack, nir_instr *instr)
{
   util_dynarray_append(stack, nir_instr *, instr);
}

static unsigned
r600_correct_location(nir_variable *var)
{
   return var->data.location - VERT_ATTRIB_GENERIC0;
}

static void
r600_create_new_load(nir_builder *b,
                     nir_intrinsic_instr *intr,
                     nir_variable *var,
                     unsigned comp,
                     unsigned num_comps,
                     unsigned old_num_comps)
{
   unsigned channels[4];

   b->cursor = nir_before_instr(&intr->instr);

   nir_intrinsic_instr *new_intr = nir_intrinsic_instr_create(b->shader, intr->intrinsic);
   nir_def_init(&new_intr->instr, &new_intr->def, num_comps,
                intr->def.bit_size);
   new_intr->num_components = num_comps;

   nir_deref_instr *deref = nir_build_deref_var(b, var);
   deref = r600_clone_deref_array(b, deref, nir_src_as_deref(intr->src[0]));

   new_intr->src[0] = nir_src_for_ssa(&deref->def);

   if (intr->intrinsic == nir_intrinsic_interp_deref_at_offset ||
       intr->intrinsic == nir_intrinsic_interp_deref_at_sample)
      new_intr->src[1] = nir_src_for_ssa(intr->src[1].ssa);

   nir_builder_instr_insert(b, &new_intr->instr);

   for (unsigned i = 0; i < old_num_comps; ++i)
      channels[i] = comp - var->data.location_frac + i;
   nir_def *load = nir_swizzle(b, &new_intr->def, channels, old_num_comps);
   nir_def_rewrite_uses(&intr->def, load);

   /* Remove the old load intrinsic */
   nir_instr_remove(&intr->instr);
}

static bool
r600_vec_instr_stack_pop(nir_builder *b,
                         struct util_dynarray *stack,
                         nir_instr *instr,
                         nir_variable *updated_vars[16][4])
{
   nir_instr *last = util_dynarray_pop(stack, nir_instr *);

   assert(last == instr);
   assert(last->type == nir_instr_type_intrinsic);

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(last);
   nir_variable *var = nir_intrinsic_get_var(intr, 0);
   unsigned loc = r600_correct_location(var);

   nir_variable *new_var;
   new_var = updated_vars[loc][var->data.location_frac];

   unsigned num_comps = glsl_get_vector_elements(glsl_without_array(new_var->type));

   unsigned old_num_comps = glsl_get_vector_elements(glsl_without_array(var->type));

   /* Don't bother walking the stack if this component can't be vectorised. */
   if (old_num_comps > 3) {
      return false;
   }

   if (new_var == var) {
      return false;
   }

   r600_create_new_load(
      b, intr, new_var, var->data.location_frac, num_comps, old_num_comps);
   return true;
}

static bool
r600_cmp_func(const void *data1, const void *data2)
{
   const struct util_dynarray *arr1 = data1;
   const struct util_dynarray *arr2 = data2;

   const nir_instr *instr1 = *(nir_instr **)util_dynarray_begin(arr1);
   const nir_instr *instr2 = *(nir_instr **)util_dynarray_begin(arr2);

   return r600_io_access_same_var(instr1, instr2);
}

#define HASH(hash, data) XXH32(&(data), sizeof(data), (hash))

static uint32_t
r600_hash_instr(const nir_instr *instr)
{
   assert(instr->type == nir_instr_type_intrinsic);

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   nir_variable *var = nir_intrinsic_get_var(intr, 0);

   uint32_t hash = 0;

   hash = HASH(hash, var->type);
   return HASH(hash, var->data.location);
}

static uint32_t
r600_hash_stack(const void *data)
{
   const struct util_dynarray *stack = data;
   const nir_instr *first = *(nir_instr **)util_dynarray_begin(stack);
   return r600_hash_instr(first);
}

static struct set *
r600_vec_instr_set_create(void)
{
   return _mesa_set_create(NULL, r600_hash_stack, r600_cmp_func);
}

static void
r600_vec_instr_set_destroy(struct set *instr_set)
{
   _mesa_set_destroy(instr_set, NULL);
}

static void
r600_vec_instr_set_add(struct set *instr_set, nir_instr *instr)
{
   if (!r600_instr_can_rewrite(instr)) {
      return;
   }

   struct util_dynarray *new_stack = r600_vec_instr_stack_create(instr_set);
   r600_vec_instr_stack_push(new_stack, instr);

   struct set_entry *entry = _mesa_set_search(instr_set, new_stack);

   if (entry) {
      ralloc_free(new_stack);
      struct util_dynarray *stack = (struct util_dynarray *)entry->key;
      r600_vec_instr_stack_push(stack, instr);
      return;
   }

   _mesa_set_add(instr_set, new_stack);

   return;
}

static bool
r600_vec_instr_set_remove(nir_builder *b,
                          struct set *instr_set,
                          nir_instr *instr,
                          nir_variable *updated_vars[16][4])
{
   if (!r600_instr_can_rewrite(instr)) {
      return false;
   }
   /*
    * It's pretty unfortunate that we have to do this, but it's a side effect
    * of the hash set interfaces. The hash set assumes that we're only
    * interested in storing one equivalent element at a time, and if we try to
    * insert a duplicate element it will remove the original. We could hack up
    * the comparison function to "know" which input is an instruction we
    * passed in and which is an array that's part of the entry, but that
    * wouldn't work because we need to pass an array to _mesa_set_add() in
    * vec_instr_add() above, and _mesa_set_add() will call our comparison
    * function as well.
    */
   struct util_dynarray *temp = r600_vec_instr_stack_create(instr_set);
   r600_vec_instr_stack_push(temp, instr);
   struct set_entry *entry = _mesa_set_search(instr_set, temp);
   ralloc_free(temp);

   if (entry) {
      struct util_dynarray *stack = (struct util_dynarray *)entry->key;
      bool progress = r600_vec_instr_stack_pop(b, stack, instr, updated_vars);

      if (!util_dynarray_num_elements(stack, nir_instr *))
         _mesa_set_remove(instr_set, entry);

      return progress;
   }

   return false;
}

static bool
r600_vectorize_block(nir_builder *b,
                     nir_block *block,
                     struct set *instr_set,
                     nir_variable *updated_vars[16][4])
{
   bool progress = false;

   nir_foreach_instr_safe(instr, block) { r600_vec_instr_set_add(instr_set, instr); }

   for (unsigned i = 0; i < block->num_dom_children; i++) {
      nir_block *child = block->dom_children[i];
      progress |= r600_vectorize_block(b, child, instr_set, updated_vars);
   }

   nir_foreach_instr_reverse_safe(instr, block)
   {
      progress |= r600_vec_instr_set_remove(b, instr_set, instr, updated_vars);
   }

   return progress;
}

static void
r600_create_new_io_var(nir_shader *shader,
                       nir_variable *vars[16][4],
                       unsigned location,
                       unsigned comps)
{
   unsigned num_comps = util_bitcount(comps);
   assert(num_comps > 1);

   /* Note: u_bit_scan() strips a component of the comps bitfield here */
   unsigned first_comp = u_bit_scan(&comps);

   nir_variable *var = nir_variable_clone(vars[location][first_comp], shader);
   var->data.location_frac = first_comp;
   var->type = glsl_replace_vector_type(var->type, num_comps);

   nir_shader_add_variable(shader, var);

   vars[location][first_comp] = var;

   while (comps) {
      const int comp = u_bit_scan(&comps);
      if (vars[location][comp]) {
         vars[location][comp] = var;
      }
   }
}

static inline bool
r600_variables_can_merge(const nir_variable *lhs, const nir_variable *rhs)
{
   return (glsl_get_base_type(lhs->type) == glsl_get_base_type(rhs->type));
}

static void
r600_create_new_io_vars(nir_shader *shader,
                        nir_variable_mode mode,
                        nir_variable *vars[16][4])
{
   bool can_rewrite_vars = false;
   nir_foreach_variable_with_modes(var, shader, mode)
   {
      if (r600_variable_can_rewrite(var)) {
         can_rewrite_vars = true;
         unsigned loc = r600_correct_location(var);
         vars[loc][var->data.location_frac] = var;
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

         if (!vars[i][j])
            continue;

         for (unsigned k = j + 1; k < 4; k++) {
            if (!vars[i][k])
               continue;

            if (!r600_variables_can_merge(vars[i][j], vars[i][k]))
               continue;

            /* Set comps */
            for (unsigned n = 0; n < glsl_get_components(vars[i][j]->type); ++n)
               comps |= 1 << (vars[i][j]->data.location_frac + n);

            for (unsigned n = 0; n < glsl_get_components(vars[i][k]->type); ++n)
               comps |= 1 << (vars[i][k]->data.location_frac + n);
         }
      }
      if (comps)
         r600_create_new_io_var(shader, vars, i, comps);
   }
}

static bool
r600_vectorize_io_impl(nir_function_impl *impl)
{
   nir_builder b = nir_builder_create(impl);

   nir_metadata_require(impl, nir_metadata_dominance);

   nir_shader *shader = impl->function->shader;
   nir_variable *updated_vars[16][4] = {0};

   r600_create_new_io_vars(shader, nir_var_shader_in, updated_vars);

   struct set *instr_set = r600_vec_instr_set_create();
   bool progress =
      r600_vectorize_block(&b, nir_start_block(impl), instr_set, updated_vars);

   if (progress) {
      nir_metadata_preserve(impl, nir_metadata_block_index | nir_metadata_dominance);
   } else {
      nir_metadata_preserve(impl, nir_metadata_all);
   }

   r600_vec_instr_set_destroy(instr_set);
   return false;
}

bool
r600_vectorize_vs_inputs(nir_shader *shader)
{
   bool progress = false;

   if (shader->info.stage != MESA_SHADER_VERTEX)
      return false;

   nir_foreach_function_impl(impl, shader)
   {
      progress |= r600_vectorize_io_impl(impl);
   }

   return progress;
}
