/*
 * Copyright © 2023 Intel Corporation
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

#include "compiler/nir/nir_builder.h"
#include "util/u_dynarray.h"

#include "brw_nir.h"

static bool
nir_instr_is_resource_intel(nir_instr *instr)
{
   return instr->type == nir_instr_type_intrinsic &&
      nir_instr_as_intrinsic(instr)->intrinsic == nir_intrinsic_resource_intel;
}

static bool
add_src_instr(nir_src *src, void *state)
{
   struct util_dynarray *inst_array = state;
   util_dynarray_foreach(inst_array, nir_instr *, instr_ptr) {
      if (*instr_ptr == src->ssa->parent_instr)
         return true;
   }

   util_dynarray_append(inst_array, nir_instr *, src->ssa->parent_instr);

   return true;
}

static nir_intrinsic_instr *
find_resource_intel(struct util_dynarray *inst_array,
                    nir_def *def)
{
   /* If resouce_intel is already directly in front of the instruction, there
    * is nothing to do.
    */
   if (nir_instr_is_resource_intel(def->parent_instr))
      return NULL;

   util_dynarray_append(inst_array, nir_instr *, def->parent_instr);

   unsigned idx = 0, scan_index = 0;
   while (idx < util_dynarray_num_elements(inst_array, nir_instr *)) {
      nir_instr *instr = *util_dynarray_element(inst_array, nir_instr *, idx++);

      for (; scan_index < util_dynarray_num_elements(inst_array, nir_instr *); scan_index++) {
         nir_instr *scan_instr = *util_dynarray_element(inst_array, nir_instr *, scan_index);
         if (nir_instr_is_resource_intel(scan_instr))
            return nir_instr_as_intrinsic(scan_instr);
      }

      nir_foreach_src(instr, add_src_instr, inst_array);
   }

   return NULL;
}

static bool
brw_nir_lower_non_uniform_intrinsic(nir_builder *b,
                                    nir_intrinsic_instr *intrin,
                                    struct util_dynarray *inst_array)
{
   unsigned source;
   switch (intrin->intrinsic) {
   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_get_ssbo_size:
   case nir_intrinsic_ssbo_atomic:
   case nir_intrinsic_ssbo_atomic_swap:
   case nir_intrinsic_load_ssbo_block_intel:
   case nir_intrinsic_store_ssbo_block_intel:
   case nir_intrinsic_load_ubo_uniform_block_intel:
   case nir_intrinsic_load_ssbo_uniform_block_intel:
   case nir_intrinsic_image_load_raw_intel:
   case nir_intrinsic_image_store_raw_intel:
   case nir_intrinsic_image_load:
   case nir_intrinsic_image_store:
   case nir_intrinsic_image_atomic:
   case nir_intrinsic_image_atomic_swap:
   case nir_intrinsic_bindless_image_load:
   case nir_intrinsic_bindless_image_store:
   case nir_intrinsic_bindless_image_atomic:
   case nir_intrinsic_bindless_image_atomic_swap:
   case nir_intrinsic_image_size:
   case nir_intrinsic_bindless_image_size:
      source = 0;
      break;

   case nir_intrinsic_store_ssbo:
      source = 1;
      break;

   default:
      return false;
   }

   b->cursor = nir_before_instr(&intrin->instr);

   util_dynarray_clear(inst_array);

   nir_intrinsic_instr *old_resource_intel =
      find_resource_intel(inst_array, intrin->src[source].ssa);
   if (old_resource_intel == NULL)
      return false;

   nir_instr *new_instr =
      nir_instr_clone(b->shader, &old_resource_intel->instr);

   nir_instr_insert(b->cursor, new_instr);

   nir_intrinsic_instr *new_resource_intel =
      nir_instr_as_intrinsic(new_instr);

   nir_src_rewrite(&new_resource_intel->src[1], intrin->src[source].ssa);
   nir_src_rewrite(&intrin->src[source], &new_resource_intel->def);

   return true;
}

static bool
brw_nir_lower_non_uniform_tex(nir_builder *b,
                              nir_tex_instr *tex,
                              struct util_dynarray *inst_array)
{
   b->cursor = nir_before_instr(&tex->instr);

   bool progress = false;
   for (unsigned s = 0; s < tex->num_srcs; s++) {
      if (tex->src[s].src_type != nir_tex_src_texture_handle &&
          tex->src[s].src_type != nir_tex_src_sampler_handle)
         continue;

      util_dynarray_clear(inst_array);

      nir_intrinsic_instr *old_resource_intel =
         find_resource_intel(inst_array, tex->src[s].src.ssa);
      if (old_resource_intel == NULL)
         continue;

      nir_instr *new_instr =
         nir_instr_clone(b->shader, &old_resource_intel->instr);

      nir_instr_insert(b->cursor, new_instr);

      nir_intrinsic_instr *new_resource_intel =
         nir_instr_as_intrinsic(new_instr);

      nir_src_rewrite(&new_resource_intel->src[1], tex->src[s].src.ssa);
      nir_src_rewrite(&tex->src[s].src, &new_resource_intel->def);

      progress = true;
   }

   return progress;
}

static bool
brw_nir_lower_non_uniform_instr(nir_builder *b,
                                nir_instr *instr,
                                void *cb_data)
{
   struct util_dynarray *inst_array = cb_data;

   switch (instr->type) {
   case nir_instr_type_intrinsic:
      return brw_nir_lower_non_uniform_intrinsic(b,
                                                 nir_instr_as_intrinsic(instr),
                                                 inst_array);

   case nir_instr_type_tex:
      return brw_nir_lower_non_uniform_tex(b,
                                           nir_instr_as_tex(instr),
                                           inst_array);

   default:
      return false;
   }
}

/** This pass rematerializes resource_intel intrinsics closer to their use.
 *
 * For example will turn this :
 *    ssa_1 = iadd ...
 *    ssa_2 = resource_intel ..., ssa_1, ...
 *    ssa_3 = read_first_invocation ssa_2
 *    ssa_4 = load_ssbo ssa_3, ...
 *
 * into this :
 *    ssa_1 = iadd ...
 *    ssa_3 = read_first_invocation ssa_1
 *    ssa_5 = resource_intel ..., ssa_3, ...
 *    ssa_4 = load_ssbo ssa_5, ...
 *
 * The goal is to have the resource_intel immediately before its use so that
 * the backend compiler can know how the load_ssbo should be compiled (binding
 * table or bindless access, etc...).
 */
bool
brw_nir_lower_non_uniform_resource_intel(nir_shader *shader)
{
   void *mem_ctx = ralloc_context(NULL);

   struct util_dynarray inst_array;
   util_dynarray_init(&inst_array, mem_ctx);

   bool ret = nir_shader_instructions_pass(shader,
                                           brw_nir_lower_non_uniform_instr,
                                           nir_metadata_block_index |
                                           nir_metadata_dominance,
                                           &inst_array);

   ralloc_free(mem_ctx);

   return ret;
}

static bool
skip_resource_intel_cleanup(nir_instr *instr)
{
   switch (instr->type) {
   case nir_instr_type_tex:
      return true;

   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin =
         nir_instr_as_intrinsic(instr);
      switch (intrin->intrinsic) {
      case nir_intrinsic_load_ubo:
      case nir_intrinsic_load_ssbo:
      case nir_intrinsic_store_ssbo:
      case nir_intrinsic_get_ssbo_size:
      case nir_intrinsic_ssbo_atomic:
      case nir_intrinsic_ssbo_atomic_swap:
      case nir_intrinsic_load_ssbo_block_intel:
      case nir_intrinsic_store_ssbo_block_intel:
      case nir_intrinsic_load_ssbo_uniform_block_intel:
      case nir_intrinsic_image_load_raw_intel:
      case nir_intrinsic_image_store_raw_intel:
      case nir_intrinsic_image_load:
      case nir_intrinsic_image_store:
      case nir_intrinsic_image_atomic:
      case nir_intrinsic_image_atomic_swap:
      case nir_intrinsic_bindless_image_load:
      case nir_intrinsic_bindless_image_store:
      case nir_intrinsic_bindless_image_atomic:
      case nir_intrinsic_bindless_image_atomic_swap:
      case nir_intrinsic_image_size:
      case nir_intrinsic_bindless_image_size:
         return true;

      default:
         return false;
      }
   }

   default:
      return false;
   }
}

static bool
brw_nir_cleanup_resource_intel_instr(nir_builder *b,
                                     nir_intrinsic_instr *intrin,
                                     void *cb_data)
{
   if (intrin->intrinsic != nir_intrinsic_resource_intel)
      return false;

   bool progress = false;
   nir_foreach_use_safe(src, &intrin->def) {
      if (!nir_src_is_if(src) && skip_resource_intel_cleanup(nir_src_parent_instr(src)))
         continue;

      progress = true;
      nir_src_rewrite(src, intrin->src[1].ssa);
   }

   return progress;
}

/** This pass removes unnecessary resource_intel intrinsics
 *
 * This pass must not be run before brw_nir_lower_non_uniform_resource_intel.
 */
bool
brw_nir_cleanup_resource_intel(nir_shader *shader)
{
   void *mem_ctx = ralloc_context(NULL);

   bool ret = nir_shader_intrinsics_pass(shader,
                                           brw_nir_cleanup_resource_intel_instr,
                                           nir_metadata_block_index |
                                           nir_metadata_dominance,
                                           NULL);

   ralloc_free(mem_ctx);

   return ret;
}
