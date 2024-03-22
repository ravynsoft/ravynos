/*
 * Copyright © 2018 Intel Corporation
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

#include "nir_deref.h"
#include "util/hash_table.h"
#include "nir.h"
#include "nir_builder.h"

bool
nir_deref_cast_is_trivial(nir_deref_instr *cast)
{
   assert(cast->deref_type == nir_deref_type_cast);

   nir_deref_instr *parent = nir_src_as_deref(cast->parent);
   if (!parent)
      return false;

   return cast->modes == parent->modes &&
          cast->type == parent->type &&
          cast->def.num_components == parent->def.num_components &&
          cast->def.bit_size == parent->def.bit_size;
}

void
nir_deref_path_init(nir_deref_path *path,
                    nir_deref_instr *deref, void *mem_ctx)
{
   assert(deref != NULL);

   /* The length of the short path is at most ARRAY_SIZE - 1 because we need
    * room for the NULL terminator.
    */
   static const int max_short_path_len = ARRAY_SIZE(path->_short_path) - 1;

   int count = 0;

   nir_deref_instr **tail = &path->_short_path[max_short_path_len];
   nir_deref_instr **head = tail;

   *tail = NULL;
   for (nir_deref_instr *d = deref; d; d = nir_deref_instr_parent(d)) {
      if (d->deref_type == nir_deref_type_cast && nir_deref_cast_is_trivial(d))
         continue;
      count++;
      if (count <= max_short_path_len)
         *(--head) = d;
   }

   if (count <= max_short_path_len) {
      /* If we're under max_short_path_len, just use the short path. */
      path->path = head;
      goto done;
   }

#ifndef NDEBUG
   /* Just in case someone uses short_path by accident */
   for (unsigned i = 0; i < ARRAY_SIZE(path->_short_path); i++)
      path->_short_path[i] = (void *)(uintptr_t)0xdeadbeef;
#endif

   path->path = ralloc_array(mem_ctx, nir_deref_instr *, count + 1);
   head = tail = path->path + count;
   *tail = NULL;
   for (nir_deref_instr *d = deref; d; d = nir_deref_instr_parent(d)) {
      if (d->deref_type == nir_deref_type_cast && nir_deref_cast_is_trivial(d))
         continue;
      *(--head) = d;
   }

done:
   assert(head == path->path);
   assert(tail == head + count);
   assert(*tail == NULL);
}

void
nir_deref_path_finish(nir_deref_path *path)
{
   if (path->path < &path->_short_path[0] ||
       path->path > &path->_short_path[ARRAY_SIZE(path->_short_path) - 1])
      ralloc_free(path->path);
}

/**
 * Recursively removes unused deref instructions
 */
bool
nir_deref_instr_remove_if_unused(nir_deref_instr *instr)
{
   bool progress = false;

   for (nir_deref_instr *d = instr; d; d = nir_deref_instr_parent(d)) {
      /* If anyone is using this deref, leave it alone */
      if (!nir_def_is_unused(&d->def))
         break;

      nir_instr_remove(&d->instr);
      progress = true;
   }

   return progress;
}

bool
nir_deref_instr_has_indirect(nir_deref_instr *instr)
{
   while (instr->deref_type != nir_deref_type_var) {
      /* Consider casts to be indirects */
      if (instr->deref_type == nir_deref_type_cast)
         return true;

      if ((instr->deref_type == nir_deref_type_array ||
           instr->deref_type == nir_deref_type_ptr_as_array) &&
          !nir_src_is_const(instr->arr.index))
         return true;

      instr = nir_deref_instr_parent(instr);
   }

   return false;
}

bool
nir_deref_instr_is_known_out_of_bounds(nir_deref_instr *instr)
{
   for (; instr; instr = nir_deref_instr_parent(instr)) {
      if (instr->deref_type == nir_deref_type_array &&
          nir_src_is_const(instr->arr.index) &&
          nir_src_as_uint(instr->arr.index) >=
             glsl_get_length(nir_deref_instr_parent(instr)->type))
         return true;
   }

   return false;
}

bool
nir_deref_instr_has_complex_use(nir_deref_instr *deref,
                                nir_deref_instr_has_complex_use_options opts)
{
   nir_foreach_use_including_if(use_src, &deref->def) {
      if (nir_src_is_if(use_src))
         return true;

      nir_instr *use_instr = nir_src_parent_instr(use_src);

      switch (use_instr->type) {
      case nir_instr_type_deref: {
         nir_deref_instr *use_deref = nir_instr_as_deref(use_instr);

         /* A var deref has no sources */
         assert(use_deref->deref_type != nir_deref_type_var);

         /* If a deref shows up in an array index or something like that, it's
          * a complex use.
          */
         if (use_src != &use_deref->parent)
            return true;

         /* Anything that isn't a basic struct or array deref is considered to
          * be a "complex" use.  In particular, we don't allow ptr_as_array
          * because we assume that opt_deref will turn any non-complex
          * ptr_as_array derefs into regular array derefs eventually so passes
          * which only want to handle simple derefs will pick them up in a
          * later pass.
          */
         if (use_deref->deref_type != nir_deref_type_struct &&
             use_deref->deref_type != nir_deref_type_array_wildcard &&
             use_deref->deref_type != nir_deref_type_array)
            return true;

         if (nir_deref_instr_has_complex_use(use_deref, opts))
            return true;

         continue;
      }

      case nir_instr_type_intrinsic: {
         nir_intrinsic_instr *use_intrin = nir_instr_as_intrinsic(use_instr);
         switch (use_intrin->intrinsic) {
         case nir_intrinsic_load_deref:
            assert(use_src == &use_intrin->src[0]);
            continue;

         case nir_intrinsic_copy_deref:
            assert(use_src == &use_intrin->src[0] ||
                   use_src == &use_intrin->src[1]);
            continue;

         case nir_intrinsic_store_deref:
            /* A use in src[1] of a store means we're taking that pointer and
             * writing it to a variable.  Because we have no idea who will
             * read that variable and what they will do with the pointer, it's
             * considered a "complex" use.  A use in src[0], on the other
             * hand, is a simple use because we're just going to dereference
             * it and write a value there.
             */
            if (use_src == &use_intrin->src[0])
               continue;
            return true;

         case nir_intrinsic_memcpy_deref:
            if (use_src == &use_intrin->src[0] &&
                (opts & nir_deref_instr_has_complex_use_allow_memcpy_dst))
               continue;
            if (use_src == &use_intrin->src[1] &&
                (opts & nir_deref_instr_has_complex_use_allow_memcpy_src))
               continue;
            return true;

         case nir_intrinsic_deref_atomic:
         case nir_intrinsic_deref_atomic_swap:
            if (opts & nir_deref_instr_has_complex_use_allow_atomics)
               continue;
            return true;

         default:
            return true;
         }
         unreachable("Switch default failed");
      }

      default:
         return true;
      }
   }

   return false;
}

static unsigned
type_scalar_size_bytes(const struct glsl_type *type)
{
   assert(glsl_type_is_vector_or_scalar(type) ||
          glsl_type_is_matrix(type));
   return glsl_type_is_boolean(type) ? 4 : glsl_get_bit_size(type) / 8;
}

unsigned
nir_deref_instr_array_stride(nir_deref_instr *deref)
{
   switch (deref->deref_type) {
   case nir_deref_type_array:
   case nir_deref_type_array_wildcard: {
      const struct glsl_type *arr_type = nir_deref_instr_parent(deref)->type;
      unsigned stride = glsl_get_explicit_stride(arr_type);

      if ((glsl_type_is_matrix(arr_type) &&
           glsl_matrix_type_is_row_major(arr_type)) ||
          (glsl_type_is_vector(arr_type) && stride == 0))
         stride = type_scalar_size_bytes(arr_type);

      return stride;
   }
   case nir_deref_type_ptr_as_array:
      return nir_deref_instr_array_stride(nir_deref_instr_parent(deref));
   case nir_deref_type_cast:
      return deref->cast.ptr_stride;
   default:
      return 0;
   }
}

static unsigned
type_get_array_stride(const struct glsl_type *elem_type,
                      glsl_type_size_align_func size_align)
{
   unsigned elem_size, elem_align;
   size_align(elem_type, &elem_size, &elem_align);
   return ALIGN_POT(elem_size, elem_align);
}

static unsigned
struct_type_get_field_offset(const struct glsl_type *struct_type,
                             glsl_type_size_align_func size_align,
                             unsigned field_idx)
{
   assert(glsl_type_is_struct_or_ifc(struct_type));
   unsigned offset = 0;
   for (unsigned i = 0; i <= field_idx; i++) {
      unsigned elem_size, elem_align;
      size_align(glsl_get_struct_field(struct_type, i), &elem_size, &elem_align);
      offset = ALIGN_POT(offset, elem_align);
      if (i < field_idx)
         offset += elem_size;
   }
   return offset;
}

unsigned
nir_deref_instr_get_const_offset(nir_deref_instr *deref,
                                 glsl_type_size_align_func size_align)
{
   nir_deref_path path;
   nir_deref_path_init(&path, deref, NULL);

   unsigned offset = 0;
   for (nir_deref_instr **p = &path.path[1]; *p; p++) {
      switch ((*p)->deref_type) {
      case nir_deref_type_array:
         offset += nir_src_as_uint((*p)->arr.index) *
                   type_get_array_stride((*p)->type, size_align);
         break;
      case nir_deref_type_struct: {
         /* p starts at path[1], so this is safe */
         nir_deref_instr *parent = *(p - 1);
         offset += struct_type_get_field_offset(parent->type, size_align,
                                                (*p)->strct.index);
         break;
      }
      case nir_deref_type_cast:
         /* A cast doesn't contribute to the offset */
         break;
      default:
         unreachable("Unsupported deref type");
      }
   }

   nir_deref_path_finish(&path);

   return offset;
}

nir_def *
nir_build_deref_offset(nir_builder *b, nir_deref_instr *deref,
                       glsl_type_size_align_func size_align)
{
   nir_deref_path path;
   nir_deref_path_init(&path, deref, NULL);

   nir_def *offset = nir_imm_intN_t(b, 0, deref->def.bit_size);
   for (nir_deref_instr **p = &path.path[1]; *p; p++) {
      switch ((*p)->deref_type) {
      case nir_deref_type_array:
      case nir_deref_type_ptr_as_array: {
         nir_def *index = (*p)->arr.index.ssa;
         int stride = type_get_array_stride((*p)->type, size_align);
         offset = nir_iadd(b, offset, nir_amul_imm(b, index, stride));
         break;
      }
      case nir_deref_type_struct: {
         /* p starts at path[1], so this is safe */
         nir_deref_instr *parent = *(p - 1);
         unsigned field_offset =
            struct_type_get_field_offset(parent->type, size_align,
                                         (*p)->strct.index);
         offset = nir_iadd_imm(b, offset, field_offset);
         break;
      }
      case nir_deref_type_cast:
         /* A cast doesn't contribute to the offset */
         break;
      default:
         unreachable("Unsupported deref type");
      }
   }

   nir_deref_path_finish(&path);

   return offset;
}

bool
nir_remove_dead_derefs_impl(nir_function_impl *impl)
{
   bool progress = false;

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type == nir_instr_type_deref &&
             nir_deref_instr_remove_if_unused(nir_instr_as_deref(instr)))
            progress = true;
      }
   }

   if (progress) {
      nir_metadata_preserve(impl, nir_metadata_block_index |
                                     nir_metadata_dominance);
   } else {
      nir_metadata_preserve(impl, nir_metadata_all);
   }

   return progress;
}

bool
nir_remove_dead_derefs(nir_shader *shader)
{
   bool progress = false;
   nir_foreach_function_impl(impl, shader) {
      if (nir_remove_dead_derefs_impl(impl))
         progress = true;
   }

   return progress;
}

static bool
nir_fixup_deref_modes_instr(UNUSED struct nir_builder *b, nir_instr *instr, UNUSED void *data)
{
   if (instr->type != nir_instr_type_deref)
      return false;

   nir_deref_instr *deref = nir_instr_as_deref(instr);
   nir_variable_mode parent_modes;
   if (deref->deref_type == nir_deref_type_var) {
      parent_modes = deref->var->data.mode;
   } else {
      nir_deref_instr *parent = nir_src_as_deref(deref->parent);
      if (parent == NULL) {
         /* Cast to some non-deref value, nothing to propagate. */
         assert(deref->deref_type == nir_deref_type_cast);
         return false;
      }

      /* It's safe to propagate a specific mode into a more generic one
       * but never the other way around.
       */
      if (util_bitcount(parent->modes) != 1)
         return false;

      parent_modes = parent->modes;
   }

   if (deref->modes == parent_modes)
      return false;

   deref->modes = parent_modes;
   return true;
}

void
nir_fixup_deref_modes(nir_shader *shader)
{
   nir_shader_instructions_pass(shader, nir_fixup_deref_modes_instr,
                                nir_metadata_block_index |
                                   nir_metadata_dominance |
                                   nir_metadata_live_defs |
                                   nir_metadata_instr_index,
                                NULL);
}

static bool
nir_fixup_deref_types_instr(UNUSED struct nir_builder *b, nir_instr *instr, UNUSED void *data)
{
   if (instr->type != nir_instr_type_deref)
      return false;

   nir_deref_instr *deref = nir_instr_as_deref(instr);
   const struct glsl_type *parent_derived_type;
   if (deref->deref_type == nir_deref_type_var) {
      parent_derived_type = deref->var->type;
   } else if (deref->deref_type == nir_deref_type_array ||
              deref->deref_type == nir_deref_type_struct) {
      nir_deref_instr *parent = nir_src_as_deref(deref->parent);
      if (deref->deref_type == nir_deref_type_array) {
         parent_derived_type = glsl_get_array_element(parent->type);
      } else if (deref->deref_type == nir_deref_type_struct) {
         parent_derived_type =
            glsl_get_struct_field(parent->type, deref->strct.index);
      } else {
         unreachable("Unsupported deref type");
      }
   } else {
      unreachable("Unsupported deref type");
   }

   if (deref->type == parent_derived_type)
      return false;

   deref->type = parent_derived_type;
   return true;
}

/* Update deref types when array sizes have changed. */
void
nir_fixup_deref_types(nir_shader *shader)
{
   nir_shader_instructions_pass(shader, nir_fixup_deref_types_instr,
                                nir_metadata_block_index |
                                   nir_metadata_dominance |
                                   nir_metadata_live_defs |
                                   nir_metadata_instr_index,
                                NULL);
}

static bool
modes_may_alias(nir_variable_mode a, nir_variable_mode b)
{
   /* Generic pointers can alias with SSBOs */
   if ((a & (nir_var_mem_ssbo | nir_var_mem_global)) &&
       (b & (nir_var_mem_ssbo | nir_var_mem_global)))
      return true;

   /* Pointers can only alias if they share a mode. */
   return a & b;
}

ALWAYS_INLINE static nir_deref_compare_result
compare_deref_paths(nir_deref_path *a_path, nir_deref_path *b_path,
                    unsigned *i, bool (*stop_fn)(const nir_deref_instr *))
{
   /* Start off assuming they fully compare.  We ignore equality for now.  In
    * the end, we'll determine that by containment.
    */
   nir_deref_compare_result result = nir_derefs_may_alias_bit |
                                     nir_derefs_a_contains_b_bit |
                                     nir_derefs_b_contains_a_bit;

   nir_deref_instr **a = a_path->path;
   nir_deref_instr **b = b_path->path;

   for (; a[*i] != NULL; (*i)++) {
      if (a[*i] != b[*i])
         break;

      if (stop_fn && stop_fn(a[*i]))
         break;
   }

   /* We're at either the tail or the divergence point between the two deref
    * paths.  Look to see if either contains cast or a ptr_as_array deref.  If
    * it does we don't know how to safely make any inferences.  Hopefully,
    * nir_opt_deref will clean most of these up and we can start inferring
    * things again.
    *
    * In theory, we could do a bit better.  For instance, we could detect the
    * case where we have exactly one ptr_as_array deref in the chain after the
    * divergence point and it's matched in both chains and the two chains have
    * different constant indices.
    */
   for (unsigned j = *i; a[j] != NULL; j++) {
      if (stop_fn && stop_fn(a[j]))
         break;

      if (a[j]->deref_type == nir_deref_type_cast ||
          a[j]->deref_type == nir_deref_type_ptr_as_array)
         return nir_derefs_may_alias_bit;
   }
   for (unsigned j = *i; b[j] != NULL; j++) {
      if (stop_fn && stop_fn(b[j]))
         break;

      if (b[j]->deref_type == nir_deref_type_cast ||
          b[j]->deref_type == nir_deref_type_ptr_as_array)
         return nir_derefs_may_alias_bit;
   }

   for (; a[*i] != NULL && b[*i] != NULL; (*i)++) {
      if (stop_fn && (stop_fn(a[*i]) || stop_fn(b[*i])))
         break;

      switch (a[*i]->deref_type) {
      case nir_deref_type_array:
      case nir_deref_type_array_wildcard: {
         assert(b[*i]->deref_type == nir_deref_type_array ||
                b[*i]->deref_type == nir_deref_type_array_wildcard);

         if (a[*i]->deref_type == nir_deref_type_array_wildcard) {
            if (b[*i]->deref_type != nir_deref_type_array_wildcard)
               result &= ~nir_derefs_b_contains_a_bit;
         } else if (b[*i]->deref_type == nir_deref_type_array_wildcard) {
            if (a[*i]->deref_type != nir_deref_type_array_wildcard)
               result &= ~nir_derefs_a_contains_b_bit;
         } else {
            assert(a[*i]->deref_type == nir_deref_type_array &&
                   b[*i]->deref_type == nir_deref_type_array);

            if (nir_src_is_const(a[*i]->arr.index) &&
                nir_src_is_const(b[*i]->arr.index)) {
               /* If they're both direct and have different offsets, they
                * don't even alias much less anything else.
                */
               if (nir_src_as_uint(a[*i]->arr.index) !=
                   nir_src_as_uint(b[*i]->arr.index))
                  return nir_derefs_do_not_alias;
            } else if (a[*i]->arr.index.ssa == b[*i]->arr.index.ssa) {
               /* They're the same indirect, continue on */
            } else {
               /* They're not the same index so we can't prove anything about
                * containment.
                */
               result &= ~(nir_derefs_a_contains_b_bit | nir_derefs_b_contains_a_bit);
            }
         }
         break;
      }

      case nir_deref_type_struct: {
         /* If they're different struct members, they don't even alias */
         if (a[*i]->strct.index != b[*i]->strct.index)
            return nir_derefs_do_not_alias;
         break;
      }

      default:
         unreachable("Invalid deref type");
      }
   }

   /* If a is longer than b, then it can't contain b.  If neither a[i] nor
    * b[i] are NULL then we aren't at the end of the chain and we know nothing
    * about containment.
    */
   if (a[*i] != NULL)
      result &= ~nir_derefs_a_contains_b_bit;
   if (b[*i] != NULL)
      result &= ~nir_derefs_b_contains_a_bit;

   /* If a contains b and b contains a they must be equal. */
   if ((result & nir_derefs_a_contains_b_bit) &&
       (result & nir_derefs_b_contains_a_bit))
      result |= nir_derefs_equal_bit;

   return result;
}

static bool
is_interface_struct_deref(const nir_deref_instr *deref)
{
   if (deref->deref_type == nir_deref_type_struct) {
      assert(glsl_type_is_struct_or_ifc(nir_deref_instr_parent(deref)->type));
      return true;
   } else {
      return false;
   }
}

nir_deref_compare_result
nir_compare_deref_paths(nir_deref_path *a_path,
                        nir_deref_path *b_path)
{
   if (!modes_may_alias(b_path->path[0]->modes, a_path->path[0]->modes))
      return nir_derefs_do_not_alias;

   if (a_path->path[0]->deref_type != b_path->path[0]->deref_type)
      return nir_derefs_may_alias_bit;

   unsigned path_idx = 1;
   if (a_path->path[0]->deref_type == nir_deref_type_var) {
      const nir_variable *a_var = a_path->path[0]->var;
      const nir_variable *b_var = b_path->path[0]->var;

      /* If we got here, the two variables must have the same mode.  The
       * only way modes_may_alias() can return true for two different modes
       * is if one is global and the other ssbo.  However, Global variables
       * only exist in OpenCL and SSBOs don't exist there.  No API allows
       * both for variables.
       */
      assert(a_var->data.mode == b_var->data.mode);

      switch (a_var->data.mode) {
      case nir_var_mem_ssbo: {
         nir_deref_compare_result binding_compare;
         if (a_var == b_var) {
            binding_compare = compare_deref_paths(a_path, b_path, &path_idx,
                                                  is_interface_struct_deref);
         } else {
            binding_compare = nir_derefs_do_not_alias;
         }

         if (binding_compare & nir_derefs_equal_bit)
            break;

         /* If the binding derefs can't alias and at least one is RESTRICT,
          * then we know they can't alias.
          */
         if (!(binding_compare & nir_derefs_may_alias_bit) &&
             ((a_var->data.access & ACCESS_RESTRICT) ||
              (b_var->data.access & ACCESS_RESTRICT)))
            return nir_derefs_do_not_alias;

         return nir_derefs_may_alias_bit;
      }

      case nir_var_mem_shared:
         if (a_var == b_var)
            break;

         /* Per SPV_KHR_workgroup_memory_explicit_layout and
          * GL_EXT_shared_memory_block, shared blocks alias each other.
          * We will have either all blocks or all non-blocks.
          */
         if (glsl_type_is_interface(a_var->type) ||
             glsl_type_is_interface(b_var->type)) {
            assert(glsl_type_is_interface(a_var->type) &&
                   glsl_type_is_interface(b_var->type));
            return nir_derefs_may_alias_bit;
         }

         /* Otherwise, distinct shared vars don't alias */
         return nir_derefs_do_not_alias;

      default:
         /* For any other variable types, if we can chase them back to the
          * variable, and the variables are different, they don't alias.
          */
         if (a_var == b_var)
            break;

         return nir_derefs_do_not_alias;
      }
   } else {
      assert(a_path->path[0]->deref_type == nir_deref_type_cast);
      /* If they're not exactly the same cast, it's hard to compare them so we
       * just assume they alias.  Comparing casts is tricky as there are lots
       * of things such as mode, type, etc. to make sure work out; for now, we
       * just assume nit_opt_deref will combine them and compare the deref
       * instructions.
       *
       * TODO: At some point in the future, we could be clever and understand
       * that a float[] and int[] have the same layout and aliasing structure
       * but double[] and vec3[] do not and we could potentially be a bit
       * smarter here.
       */
      if (a_path->path[0] != b_path->path[0])
         return nir_derefs_may_alias_bit;
   }

   return compare_deref_paths(a_path, b_path, &path_idx, NULL);
}

nir_deref_compare_result
nir_compare_derefs(nir_deref_instr *a, nir_deref_instr *b)
{
   if (a == b) {
      return nir_derefs_equal_bit | nir_derefs_may_alias_bit |
             nir_derefs_a_contains_b_bit | nir_derefs_b_contains_a_bit;
   }

   nir_deref_path a_path, b_path;
   nir_deref_path_init(&a_path, a, NULL);
   nir_deref_path_init(&b_path, b, NULL);
   assert(a_path.path[0]->deref_type == nir_deref_type_var ||
          a_path.path[0]->deref_type == nir_deref_type_cast);
   assert(b_path.path[0]->deref_type == nir_deref_type_var ||
          b_path.path[0]->deref_type == nir_deref_type_cast);

   nir_deref_compare_result result = nir_compare_deref_paths(&a_path, &b_path);

   nir_deref_path_finish(&a_path);
   nir_deref_path_finish(&b_path);

   return result;
}

nir_deref_path *
nir_get_deref_path(void *mem_ctx, nir_deref_and_path *deref)
{
   if (!deref->_path) {
      deref->_path = ralloc(mem_ctx, nir_deref_path);
      nir_deref_path_init(deref->_path, deref->instr, mem_ctx);
   }
   return deref->_path;
}

nir_deref_compare_result
nir_compare_derefs_and_paths(void *mem_ctx,
                             nir_deref_and_path *a,
                             nir_deref_and_path *b)
{
   if (a->instr == b->instr) /* nir_compare_derefs has a fast path if a == b */
      return nir_compare_derefs(a->instr, b->instr);

   return nir_compare_deref_paths(nir_get_deref_path(mem_ctx, a),
                                  nir_get_deref_path(mem_ctx, b));
}

struct rematerialize_deref_state {
   bool progress;
   nir_builder builder;
   nir_block *block;
};

static nir_deref_instr *
rematerialize_deref_in_block(nir_deref_instr *deref,
                             struct rematerialize_deref_state *state)
{
   if (deref->instr.block == state->block)
      return deref;

   nir_builder *b = &state->builder;
   nir_deref_instr *new_deref =
      nir_deref_instr_create(b->shader, deref->deref_type);
   new_deref->modes = deref->modes;
   new_deref->type = deref->type;

   if (deref->deref_type == nir_deref_type_var) {
      new_deref->var = deref->var;
   } else {
      nir_deref_instr *parent = nir_src_as_deref(deref->parent);
      if (parent) {
         parent = rematerialize_deref_in_block(parent, state);
         new_deref->parent = nir_src_for_ssa(&parent->def);
      } else {
         new_deref->parent = nir_src_for_ssa(deref->parent.ssa);
      }
   }

   switch (deref->deref_type) {
   case nir_deref_type_var:
   case nir_deref_type_array_wildcard:
      /* Nothing more to do */
      break;

   case nir_deref_type_cast:
      new_deref->cast.ptr_stride = deref->cast.ptr_stride;
      new_deref->cast.align_mul = deref->cast.align_mul;
      new_deref->cast.align_offset = deref->cast.align_offset;
      break;

   case nir_deref_type_array:
   case nir_deref_type_ptr_as_array:
      assert(!nir_src_as_deref(deref->arr.index));
      new_deref->arr.index = nir_src_for_ssa(deref->arr.index.ssa);
      break;

   case nir_deref_type_struct:
      new_deref->strct.index = deref->strct.index;
      break;

   default:
      unreachable("Invalid deref instruction type");
   }

   nir_def_init(&new_deref->instr, &new_deref->def,
                deref->def.num_components, deref->def.bit_size);
   nir_builder_instr_insert(b, &new_deref->instr);

   return new_deref;
}

static bool
rematerialize_deref_src(nir_src *src, void *_state)
{
   struct rematerialize_deref_state *state = _state;

   nir_deref_instr *deref = nir_src_as_deref(*src);
   if (!deref)
      return true;

   nir_deref_instr *block_deref = rematerialize_deref_in_block(deref, state);
   if (block_deref != deref) {
      nir_src_rewrite(src, &block_deref->def);
      nir_deref_instr_remove_if_unused(deref);
      state->progress = true;
   }

   return true;
}

bool
nir_rematerialize_deref_in_use_blocks(nir_deref_instr *instr)
{
   if (nir_deref_instr_remove_if_unused(instr))
      return true;

   struct rematerialize_deref_state state = {
      .builder = nir_builder_create(nir_cf_node_get_function(&instr->instr.block->cf_node)),
   };

   nir_foreach_use_safe(use, &instr->def) {
      nir_instr *parent = nir_src_parent_instr(use);
      if (parent->block == instr->instr.block)
         continue;

      /* If a deref is used in a phi, we can't rematerialize it, as the new
       * derefs would appear before the phi, which is not valid.
       */
      if (parent->type == nir_instr_type_phi)
         continue;

      state.block = parent->block;
      state.builder.cursor = nir_before_instr(parent);
      rematerialize_deref_src(use, &state);
   }

   return state.progress;
}

/** Re-materialize derefs in every block
 *
 * This pass re-materializes deref instructions in every block in which it is
 * used.  After this pass has been run, every use of a deref will be of a
 * deref in the same block as the use.  Also, all unused derefs will be
 * deleted as a side-effect.
 *
 * Derefs used as sources of phi instructions are not rematerialized.
 */
bool
nir_rematerialize_derefs_in_use_blocks_impl(nir_function_impl *impl)
{
   bool progress = false;
   nir_foreach_block_unstructured(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type == nir_instr_type_deref) {
            nir_deref_instr *deref = nir_instr_as_deref(instr);
            progress |= nir_rematerialize_deref_in_use_blocks(deref);
         }
      }

#ifndef NDEBUG
      nir_if *following_if = nir_block_get_following_if(block);
      if (following_if)
         assert(!nir_src_as_deref(following_if->condition));
#endif
   }

   return progress;
}

static void
nir_deref_instr_fixup_child_types(nir_deref_instr *parent)
{
   nir_foreach_use(use, &parent->def) {
      if (nir_src_parent_instr(use)->type != nir_instr_type_deref)
         continue;

      nir_deref_instr *child = nir_instr_as_deref(nir_src_parent_instr(use));
      switch (child->deref_type) {
      case nir_deref_type_var:
         unreachable("nir_deref_type_var cannot be a child");

      case nir_deref_type_array:
      case nir_deref_type_array_wildcard:
         child->type = glsl_get_array_element(parent->type);
         break;

      case nir_deref_type_ptr_as_array:
         child->type = parent->type;
         break;

      case nir_deref_type_struct:
         child->type = glsl_get_struct_field(parent->type,
                                             child->strct.index);
         break;

      case nir_deref_type_cast:
         /* We stop the recursion here */
         continue;
      }

      /* Recurse into children */
      nir_deref_instr_fixup_child_types(child);
   }
}

static bool
opt_alu_of_cast(nir_alu_instr *alu)
{
   bool progress = false;

   for (unsigned i = 0; i < nir_op_infos[alu->op].num_inputs; i++) {
      nir_instr *src_instr = alu->src[i].src.ssa->parent_instr;
      if (src_instr->type != nir_instr_type_deref)
         continue;

      nir_deref_instr *src_deref = nir_instr_as_deref(src_instr);
      if (src_deref->deref_type != nir_deref_type_cast)
         continue;

      nir_src_rewrite(&alu->src[i].src, src_deref->parent.ssa);
      progress = true;
   }

   return progress;
}

static bool
is_trivial_array_deref_cast(nir_deref_instr *cast)
{
   assert(nir_deref_cast_is_trivial(cast));

   nir_deref_instr *parent = nir_src_as_deref(cast->parent);

   if (parent->deref_type == nir_deref_type_array) {
      return cast->cast.ptr_stride ==
             glsl_get_explicit_stride(nir_deref_instr_parent(parent)->type);
   } else if (parent->deref_type == nir_deref_type_ptr_as_array) {
      return cast->cast.ptr_stride ==
             nir_deref_instr_array_stride(parent);
   } else {
      return false;
   }
}

static bool
is_deref_ptr_as_array(nir_instr *instr)
{
   return instr->type == nir_instr_type_deref &&
          nir_instr_as_deref(instr)->deref_type == nir_deref_type_ptr_as_array;
}

static bool
opt_remove_restricting_cast_alignments(nir_deref_instr *cast)
{
   assert(cast->deref_type == nir_deref_type_cast);
   if (cast->cast.align_mul == 0)
      return false;

   nir_deref_instr *parent = nir_src_as_deref(cast->parent);
   if (parent == NULL)
      return false;

   /* Don't use any default alignment for this check.  We don't want to fall
    * back to type alignment too early in case we find out later that we're
    * somehow a child of a packed struct.
    */
   uint32_t parent_mul, parent_offset;
   if (!nir_get_explicit_deref_align(parent, false /* default_to_type_align */,
                                     &parent_mul, &parent_offset))
      return false;

   /* If this cast increases the alignment, we want to keep it.
    *
    * There is a possibility that the larger alignment provided by this cast
    * somehow disagrees with the smaller alignment further up the deref chain.
    * In that case, we choose to favor the alignment closer to the actual
    * memory operation which, in this case, is the cast and not its parent so
    * keeping the cast alignment is the right thing to do.
    */
   if (parent_mul < cast->cast.align_mul)
      return false;

   /* If we've gotten here, we have a parent deref with an align_mul at least
    * as large as ours so we can potentially throw away the alignment
    * information on this deref.  There are two cases to consider here:
    *
    *  1. We can chase the deref all the way back to the variable.  In this
    *     case, we have "perfect" knowledge, modulo indirect array derefs.
    *     Unless we've done something wrong in our indirect/wildcard stride
    *     calculations, our knowledge from the deref walk is better than the
    *     client's.
    *
    *  2. We can't chase it all the way back to the variable.  In this case,
    *     because our call to nir_get_explicit_deref_align(parent, ...) above
    *     above passes default_to_type_align=false, the only way we can even
    *     get here is if something further up the deref chain has a cast with
    *     an alignment which can only happen if we get an alignment from the
    *     client (most likely a decoration in the SPIR-V).  If the client has
    *     provided us with two conflicting alignments in the deref chain,
    *     that's their fault and we can do whatever we want.
    *
    * In either case, we should be without our rights, at this point, to throw
    * away the alignment information on this deref.  However, to be "nice" to
    * weird clients, we do one more check.  It really shouldn't happen but
    * it's possible that the parent's alignment offset disagrees with the
    * cast's alignment offset.  In this case, we consider the cast as
    * providing more information (or at least more valid information) and keep
    * it even if the align_mul from the parent is larger.
    */
   assert(cast->cast.align_mul <= parent_mul);
   if (parent_offset % cast->cast.align_mul != cast->cast.align_offset)
      return false;

   /* If we got here, the parent has better alignment information than the
    * child and we can get rid of the child alignment information.
    */
   cast->cast.align_mul = 0;
   cast->cast.align_offset = 0;
   return true;
}

/**
 * Remove casts that just wrap other casts.
 */
static bool
opt_remove_cast_cast(nir_deref_instr *cast)
{
   nir_deref_instr *parent = nir_deref_instr_parent(cast);
   if (parent == NULL || parent->deref_type != nir_deref_type_cast)
      return false;

   /* Copy align info from the parent cast if needed
    *
    * In the case that align_mul = 0, the alignment for this cast is inhereted
    * from the parent deref (if any). If we aren't careful, removing our
    * parent cast from the chain may lose alignment information so we need to
    * copy the parent's alignment information (if any).
    *
    * opt_remove_restricting_cast_alignments() above is run before this pass
    * and will will have cleared our alignment (set align_mul = 0) in the case
    * where the parent's alignment information is somehow superior.
    */
   if (cast->cast.align_mul == 0) {
      cast->cast.align_mul = parent->cast.align_mul;
      cast->cast.align_offset = parent->cast.align_offset;
   }

   nir_src_rewrite(&cast->parent, parent->parent.ssa);
   return true;
}

/* Restrict variable modes in casts.
 *
 * If we know from something higher up the deref chain that the deref has a
 * specific mode, we can cast to more general and back but we can never cast
 * across modes.  For non-cast derefs, we should only ever do anything here if
 * the parent eventually comes from a cast that we restricted earlier.
 */
static bool
opt_restrict_deref_modes(nir_deref_instr *deref)
{
   if (deref->deref_type == nir_deref_type_var) {
      assert(deref->modes == deref->var->data.mode);
      return false;
   }

   nir_deref_instr *parent = nir_src_as_deref(deref->parent);
   if (parent == NULL || parent->modes == deref->modes)
      return false;

   assert(parent->modes & deref->modes);
   deref->modes &= parent->modes;
   return true;
}

static bool
opt_remove_sampler_cast(nir_deref_instr *cast)
{
   assert(cast->deref_type == nir_deref_type_cast);
   nir_deref_instr *parent = nir_src_as_deref(cast->parent);
   if (parent == NULL)
      return false;

   /* Strip both types down to their non-array type and bail if there are any
    * discrepancies in array lengths.
    */
   const struct glsl_type *parent_type = parent->type;
   const struct glsl_type *cast_type = cast->type;
   while (glsl_type_is_array(parent_type) && glsl_type_is_array(cast_type)) {
      if (glsl_get_length(parent_type) != glsl_get_length(cast_type))
         return false;
      parent_type = glsl_get_array_element(parent_type);
      cast_type = glsl_get_array_element(cast_type);
   }

   if (!glsl_type_is_sampler(parent_type))
      return false;

   if (cast_type != glsl_bare_sampler_type() &&
       (glsl_type_is_bare_sampler(parent_type) ||
        cast_type != glsl_sampler_type_to_texture(parent_type)))
      return false;

   /* We're a cast from a more detailed sampler type to a bare sampler or a
    * texture type with the same dimensionality.
    */
   nir_def_rewrite_uses(&cast->def,
                        &parent->def);
   nir_instr_remove(&cast->instr);

   /* Recursively crawl the deref tree and clean up types */
   nir_deref_instr_fixup_child_types(parent);

   return true;
}

/**
 * Is this casting a struct to a contained struct.
 * struct a { struct b field0 };
 * ssa_5 is structa;
 * deref_cast (structb *)ssa_5 (function_temp structb);
 * converts to
 * deref_struct &ssa_5->field0 (function_temp structb);
 * This allows subsequent copy propagation to work.
 */
static bool
opt_replace_struct_wrapper_cast(nir_builder *b, nir_deref_instr *cast)
{
   nir_deref_instr *parent = nir_src_as_deref(cast->parent);
   if (!parent)
      return false;

   if (cast->cast.align_mul > 0)
      return false;

   if (!glsl_type_is_struct(parent->type))
      return false;

   /* Empty struct */
   if (glsl_get_length(parent->type) < 1)
      return false;

   if (glsl_get_struct_field_offset(parent->type, 0) != 0)
      return false;

   const struct glsl_type *field_type = glsl_get_struct_field(parent->type, 0);
   if (cast->type != field_type)
      return false;

   /* we can't drop the stride information */
   if (cast->cast.ptr_stride != glsl_get_explicit_stride(field_type))
      return false;

   nir_deref_instr *replace = nir_build_deref_struct(b, parent, 0);
   nir_def_rewrite_uses(&cast->def, &replace->def);
   nir_deref_instr_remove_if_unused(cast);
   return true;
}

static bool
opt_deref_cast(nir_builder *b, nir_deref_instr *cast)
{
   bool progress = false;

   progress |= opt_remove_restricting_cast_alignments(cast);

   if (opt_replace_struct_wrapper_cast(b, cast))
      return true;

   if (opt_remove_sampler_cast(cast))
      return true;

   progress |= opt_remove_cast_cast(cast);
   if (!nir_deref_cast_is_trivial(cast))
      return progress;

   /* If this deref still contains useful alignment information, we don't want
    * to delete it.
    */
   if (cast->cast.align_mul > 0)
      return progress;

   bool trivial_array_cast = is_trivial_array_deref_cast(cast);

   nir_foreach_use_including_if_safe(use_src, &cast->def) {
      assert(!nir_src_is_if(use_src) && "there cannot be if-uses");

      /* If this isn't a trivial array cast, we can't propagate into
       * ptr_as_array derefs.
       */
      if (is_deref_ptr_as_array(nir_src_parent_instr(use_src)) &&
          !trivial_array_cast)
         continue;

      nir_src_rewrite(use_src, cast->parent.ssa);
      progress = true;
   }

   if (nir_deref_instr_remove_if_unused(cast))
      progress = true;

   return progress;
}

static bool
opt_deref_ptr_as_array(nir_builder *b, nir_deref_instr *deref)
{
   assert(deref->deref_type == nir_deref_type_ptr_as_array);

   nir_deref_instr *parent = nir_deref_instr_parent(deref);

   if (nir_src_is_const(deref->arr.index) &&
       nir_src_as_int(deref->arr.index) == 0) {
      /* If it's a ptr_as_array deref with an index of 0, it does nothing
       * and we can just replace its uses with its parent, unless it has
       * alignment information.
       *
       * The source of a ptr_as_array deref always has a deref_type of
       * nir_deref_type_array or nir_deref_type_cast.  If it's a cast, it
       * may be trivial and we may be able to get rid of that too.  Any
       * trivial cast of trivial cast cases should be handled already by
       * opt_deref_cast() above.
       */
      if (parent->deref_type == nir_deref_type_cast &&
          parent->cast.align_mul == 0 &&
          nir_deref_cast_is_trivial(parent))
         parent = nir_deref_instr_parent(parent);
      nir_def_rewrite_uses(&deref->def,
                           &parent->def);
      nir_instr_remove(&deref->instr);
      return true;
   }

   if (parent->deref_type != nir_deref_type_array &&
       parent->deref_type != nir_deref_type_ptr_as_array)
      return false;

   deref->arr.in_bounds &= parent->arr.in_bounds;

   nir_def *new_idx = nir_iadd(b, parent->arr.index.ssa,
                               deref->arr.index.ssa);

   deref->deref_type = parent->deref_type;
   nir_src_rewrite(&deref->parent, parent->parent.ssa);
   nir_src_rewrite(&deref->arr.index, new_idx);
   return true;
}

static bool
is_vector_bitcast_deref(nir_deref_instr *cast,
                        nir_component_mask_t mask,
                        bool is_write)
{
   if (cast->deref_type != nir_deref_type_cast)
      return false;

   /* Don't throw away useful alignment information */
   if (cast->cast.align_mul > 0)
      return false;

   /* It has to be a cast of another deref */
   nir_deref_instr *parent = nir_src_as_deref(cast->parent);
   if (parent == NULL)
      return false;

   /* The parent has to be a vector or scalar */
   if (!glsl_type_is_vector_or_scalar(parent->type))
      return false;

   /* Don't bother with 1-bit types */
   unsigned cast_bit_size = glsl_get_bit_size(cast->type);
   unsigned parent_bit_size = glsl_get_bit_size(parent->type);
   if (cast_bit_size == 1 || parent_bit_size == 1)
      return false;

   /* A strided vector type means it's not tightly packed */
   if (glsl_get_explicit_stride(cast->type) ||
       glsl_get_explicit_stride(parent->type))
      return false;

   assert(cast_bit_size > 0 && cast_bit_size % 8 == 0);
   assert(parent_bit_size > 0 && parent_bit_size % 8 == 0);
   unsigned bytes_used = util_last_bit(mask) * (cast_bit_size / 8);
   unsigned parent_bytes = glsl_get_vector_elements(parent->type) *
                           (parent_bit_size / 8);
   if (bytes_used > parent_bytes)
      return false;

   if (is_write && !nir_component_mask_can_reinterpret(mask, cast_bit_size,
                                                       parent_bit_size))
      return false;

   return true;
}

static nir_def *
resize_vector(nir_builder *b, nir_def *data, unsigned num_components)
{
   if (num_components == data->num_components)
      return data;

   unsigned swiz[NIR_MAX_VEC_COMPONENTS] = {
      0,
   };
   for (unsigned i = 0; i < MIN2(num_components, data->num_components); i++)
      swiz[i] = i;

   return nir_swizzle(b, data, swiz, num_components);
}

static bool
opt_load_vec_deref(nir_builder *b, nir_intrinsic_instr *load)
{
   nir_deref_instr *deref = nir_src_as_deref(load->src[0]);
   nir_component_mask_t read_mask =
      nir_def_components_read(&load->def);

   /* LLVM loves take advantage of the fact that vec3s in OpenCL are
    * vec4-aligned and so it can just read/write them as vec4s.  This
    * results in a LOT of vec4->vec3 casts on loads and stores.
    */
   if (is_vector_bitcast_deref(deref, read_mask, false)) {
      const unsigned old_num_comps = load->def.num_components;
      const unsigned old_bit_size = load->def.bit_size;

      nir_deref_instr *parent = nir_src_as_deref(deref->parent);
      const unsigned new_num_comps = glsl_get_vector_elements(parent->type);
      const unsigned new_bit_size = glsl_get_bit_size(parent->type);

      /* Stomp it to reference the parent */
      nir_src_rewrite(&load->src[0], &parent->def);
      load->def.bit_size = new_bit_size;
      load->def.num_components = new_num_comps;
      load->num_components = new_num_comps;

      b->cursor = nir_after_instr(&load->instr);
      nir_def *data = &load->def;
      if (old_bit_size != new_bit_size)
         data = nir_bitcast_vector(b, &load->def, old_bit_size);
      data = resize_vector(b, data, old_num_comps);

      nir_def_rewrite_uses_after(&load->def, data,
                                 data->parent_instr);
      return true;
   }

   return false;
}

static bool
opt_store_vec_deref(nir_builder *b, nir_intrinsic_instr *store)
{
   nir_deref_instr *deref = nir_src_as_deref(store->src[0]);
   nir_component_mask_t write_mask = nir_intrinsic_write_mask(store);

   /* LLVM loves take advantage of the fact that vec3s in OpenCL are
    * vec4-aligned and so it can just read/write them as vec4s.  This
    * results in a LOT of vec4->vec3 casts on loads and stores.
    */
   if (is_vector_bitcast_deref(deref, write_mask, true)) {
      nir_def *data = store->src[1].ssa;

      const unsigned old_bit_size = data->bit_size;

      nir_deref_instr *parent = nir_src_as_deref(deref->parent);
      const unsigned new_num_comps = glsl_get_vector_elements(parent->type);
      const unsigned new_bit_size = glsl_get_bit_size(parent->type);

      nir_src_rewrite(&store->src[0], &parent->def);

      /* Restrict things down as needed so the bitcast doesn't fail */
      data = nir_trim_vector(b, data, util_last_bit(write_mask));
      if (old_bit_size != new_bit_size)
         data = nir_bitcast_vector(b, data, new_bit_size);
      data = resize_vector(b, data, new_num_comps);
      nir_src_rewrite(&store->src[1], data);
      store->num_components = new_num_comps;

      /* Adjust the write mask */
      write_mask = nir_component_mask_reinterpret(write_mask, old_bit_size,
                                                  new_bit_size);
      nir_intrinsic_set_write_mask(store, write_mask);
      return true;
   }

   return false;
}

static bool
opt_known_deref_mode_is(nir_builder *b, nir_intrinsic_instr *intrin)
{
   nir_variable_mode modes = nir_intrinsic_memory_modes(intrin);
   nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
   if (deref == NULL)
      return false;

   nir_def *deref_is = NULL;

   if (nir_deref_mode_must_be(deref, modes))
      deref_is = nir_imm_true(b);

   if (!nir_deref_mode_may_be(deref, modes))
      deref_is = nir_imm_false(b);

   if (deref_is == NULL)
      return false;

   nir_def_rewrite_uses(&intrin->def, deref_is);
   nir_instr_remove(&intrin->instr);
   return true;
}

bool
nir_opt_deref_impl(nir_function_impl *impl)
{
   bool progress = false;

   nir_builder b = nir_builder_create(impl);

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         b.cursor = nir_before_instr(instr);

         switch (instr->type) {
         case nir_instr_type_alu: {
            nir_alu_instr *alu = nir_instr_as_alu(instr);
            if (opt_alu_of_cast(alu))
               progress = true;
            break;
         }

         case nir_instr_type_deref: {
            nir_deref_instr *deref = nir_instr_as_deref(instr);

            if (opt_restrict_deref_modes(deref))
               progress = true;

            switch (deref->deref_type) {
            case nir_deref_type_ptr_as_array:
               if (opt_deref_ptr_as_array(&b, deref))
                  progress = true;
               break;

            case nir_deref_type_cast:
               if (opt_deref_cast(&b, deref))
                  progress = true;
               break;

            default:
               /* Do nothing */
               break;
            }
            break;
         }

         case nir_instr_type_intrinsic: {
            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            switch (intrin->intrinsic) {
            case nir_intrinsic_load_deref:
               if (opt_load_vec_deref(&b, intrin))
                  progress = true;
               break;

            case nir_intrinsic_store_deref:
               if (opt_store_vec_deref(&b, intrin))
                  progress = true;
               break;

            case nir_intrinsic_deref_mode_is:
               if (opt_known_deref_mode_is(&b, intrin))
                  progress = true;
               break;

            default:
               /* Do nothing */
               break;
            }
            break;
         }

         default:
            /* Do nothing */
            break;
         }
      }
   }

   if (progress) {
      nir_metadata_preserve(impl, nir_metadata_block_index |
                                     nir_metadata_dominance);
   } else {
      nir_metadata_preserve(impl, nir_metadata_all);
   }

   return progress;
}

bool
nir_opt_deref(nir_shader *shader)
{
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      if (nir_opt_deref_impl(impl))
         progress = true;
   }

   return progress;
}
