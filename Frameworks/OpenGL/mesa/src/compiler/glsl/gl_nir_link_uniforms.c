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

#include "nir.h"
#include "nir_deref.h"
#include "gl_nir_linker.h"
#include "compiler/glsl/ir_uniform.h" /* for gl_uniform_storage */
#include "linker_util.h"
#include "util/u_dynarray.h"
#include "util/u_math.h"
#include "main/consts_exts.h"
#include "main/shader_types.h"

/**
 * This file do the common link for GLSL uniforms, using NIR, instead of IR as
 * the counter-part glsl/link_uniforms.cpp
 */

#define UNMAPPED_UNIFORM_LOC ~0u

struct uniform_array_info {
   /** List of dereferences of the uniform array. */
   struct util_dynarray *deref_list;

   /** Set of bit-flags to note which array elements have been accessed. */
   BITSET_WORD *indices;
};

static unsigned
uniform_storage_size(const struct glsl_type *type)
{
   switch (glsl_get_base_type(type)) {
   case GLSL_TYPE_STRUCT:
   case GLSL_TYPE_INTERFACE: {
      unsigned size = 0;
      for (unsigned i = 0; i < glsl_get_length(type); i++)
         size += uniform_storage_size(glsl_get_struct_field(type, i));
      return size;
   }
   case GLSL_TYPE_ARRAY: {
      const struct glsl_type *e_type = glsl_get_array_element(type);
      enum glsl_base_type e_base_type = glsl_get_base_type(e_type);
      if (e_base_type == GLSL_TYPE_STRUCT ||
          e_base_type == GLSL_TYPE_INTERFACE ||
          e_base_type == GLSL_TYPE_ARRAY) {
         unsigned length = !glsl_type_is_unsized_array(type) ?
            glsl_get_length(type) : 1;
         return length * uniform_storage_size(e_type);
      } else
         return 1;
   }
   default:
      return 1;
   }
}

/**
 * Update the sizes of linked shader uniform arrays to the maximum
 * array index used.
 *
 * From page 81 (page 95 of the PDF) of the OpenGL 2.1 spec:
 *
 *     If one or more elements of an array are active,
 *     GetActiveUniform will return the name of the array in name,
 *     subject to the restrictions listed above. The type of the array
 *     is returned in type. The size parameter contains the highest
 *     array element index used, plus one. The compiler or linker
 *     determines the highest index used.  There will be only one
 *     active uniform reported by the GL per uniform array.
 */
static void
update_array_sizes(struct gl_shader_program *prog, nir_variable *var,
                   struct hash_table **referenced_uniforms,
                   unsigned current_var_stage)
{
   /* For now we only resize 1D arrays.
    * TODO: add support for resizing more complex array types ??
    */
   if (!glsl_type_is_array(var->type) ||
       glsl_type_is_array(glsl_get_array_element(var->type)))
      return;

   /* GL_ARB_uniform_buffer_object says that std140 uniforms
    * will not be eliminated.  Since we always do std140, just
    * don't resize arrays in UBOs.
    *
    * Atomic counters are supposed to get deterministic
    * locations assigned based on the declaration ordering and
    * sizes, array compaction would mess that up.
    *
    * Subroutine uniforms are not removed.
    */
   if (nir_variable_is_in_block(var) || glsl_contains_atomic(var->type) ||
       glsl_get_base_type(glsl_without_array(var->type)) == GLSL_TYPE_SUBROUTINE ||
       var->constant_initializer)
      return;

   struct uniform_array_info *ainfo = NULL;
   int words = BITSET_WORDS(glsl_array_size(var->type));
   int max_array_size = 0;
   for (unsigned stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      struct gl_linked_shader *sh = prog->_LinkedShaders[stage];
      if (!sh)
         continue;

      struct hash_entry *entry =
         _mesa_hash_table_search(referenced_uniforms[stage], var->name);
      if (entry) {
         ainfo = (struct uniform_array_info *)  entry->data;
         max_array_size = MAX2(BITSET_LAST_BIT_SIZED(ainfo->indices, words),
                               max_array_size);
      }

      if (max_array_size == glsl_array_size(var->type))
         return;
   }

   if (max_array_size != glsl_array_size(var->type)) {
      /* If this is a built-in uniform (i.e., it's backed by some
       * fixed-function state), adjust the number of state slots to
       * match the new array size.  The number of slots per array entry
       * is not known.  It seems safe to assume that the total number of
       * slots is an integer multiple of the number of array elements.
       * Determine the number of slots per array element by dividing by
       * the old (total) size.
       */
      const unsigned num_slots = var->num_state_slots;
      if (num_slots > 0) {
         var->num_state_slots =
            (max_array_size * (num_slots / glsl_array_size(var->type)));
      }

      var->type = glsl_array_type(glsl_get_array_element(var->type),
                                  max_array_size, 0);

      /* Update the types of dereferences in case we changed any. */
      struct hash_entry *entry =
         _mesa_hash_table_search(referenced_uniforms[current_var_stage], var->name);
      if (entry) {
         struct uniform_array_info *ainfo =
            (struct uniform_array_info *) entry->data;
         util_dynarray_foreach(ainfo->deref_list, nir_deref_instr *, deref) {
            (*deref)->type = var->type;
         }
      }
   }
}

static void
nir_setup_uniform_remap_tables(const struct gl_constants *consts,
                               struct gl_shader_program *prog)
{
   unsigned total_entries = prog->NumExplicitUniformLocations;

   /* For glsl this may have been allocated by reserve_explicit_locations() so
    * that we can keep track of unused uniforms with explicit locations.
    */
   assert(!prog->data->spirv ||
          (prog->data->spirv && !prog->UniformRemapTable));
   if (!prog->UniformRemapTable) {
      prog->UniformRemapTable = rzalloc_array(prog,
                                              struct gl_uniform_storage *,
                                              prog->NumUniformRemapTable);
   }

   union gl_constant_value *data =
      rzalloc_array(prog->data,
                    union gl_constant_value, prog->data->NumUniformDataSlots);
   if (!prog->UniformRemapTable || !data) {
      linker_error(prog, "Out of memory during linking.\n");
      return;
   }
   prog->data->UniformDataSlots = data;

   prog->data->UniformDataDefaults =
         rzalloc_array(prog->data->UniformDataSlots,
                       union gl_constant_value, prog->data->NumUniformDataSlots);

   unsigned data_pos = 0;

   /* Reserve all the explicit locations of the active uniforms. */
   for (unsigned i = 0; i < prog->data->NumUniformStorage; i++) {
      struct gl_uniform_storage *uniform = &prog->data->UniformStorage[i];

      if (uniform->hidden)
         continue;

      if (uniform->is_shader_storage ||
          glsl_get_base_type(uniform->type) == GLSL_TYPE_SUBROUTINE)
         continue;

      if (prog->data->UniformStorage[i].remap_location == UNMAPPED_UNIFORM_LOC)
         continue;

      /* How many new entries for this uniform? */
      const unsigned entries = MAX2(1, uniform->array_elements);
      unsigned num_slots = glsl_get_component_slots(uniform->type);

      uniform->storage = &data[data_pos];

      /* Set remap table entries point to correct gl_uniform_storage. */
      for (unsigned j = 0; j < entries; j++) {
         unsigned element_loc = uniform->remap_location + j;
         prog->UniformRemapTable[element_loc] = uniform;

         data_pos += num_slots;
      }
   }

   /* Reserve locations for rest of the uniforms. */
   if (prog->data->spirv)
      link_util_update_empty_uniform_locations(prog);

   for (unsigned i = 0; i < prog->data->NumUniformStorage; i++) {
      struct gl_uniform_storage *uniform = &prog->data->UniformStorage[i];

      if (uniform->hidden)
         continue;

      if (uniform->is_shader_storage ||
          glsl_get_base_type(uniform->type) == GLSL_TYPE_SUBROUTINE)
         continue;

      /* Built-in uniforms should not get any location. */
      if (uniform->builtin)
         continue;

      /* Explicit ones have been set already. */
      if (uniform->remap_location != UNMAPPED_UNIFORM_LOC)
         continue;

      /* How many entries for this uniform? */
      const unsigned entries = MAX2(1, uniform->array_elements);

      /* Add new entries to the total amount for checking against MAX_UNIFORM-
       * _LOCATIONS. This only applies to the default uniform block (-1),
       * because locations of uniform block entries are not assignable.
       */
      if (prog->data->UniformStorage[i].block_index == -1)
         total_entries += entries;

      unsigned location =
         link_util_find_empty_block(prog, &prog->data->UniformStorage[i]);

      if (location == -1) {
         location = prog->NumUniformRemapTable;

         /* resize remap table to fit new entries */
         prog->UniformRemapTable =
            reralloc(prog,
                     prog->UniformRemapTable,
                     struct gl_uniform_storage *,
                     prog->NumUniformRemapTable + entries);
         prog->NumUniformRemapTable += entries;
      }

      /* set the base location in remap table for the uniform */
      uniform->remap_location = location;

      unsigned num_slots = glsl_get_component_slots(uniform->type);

      if (uniform->block_index == -1)
         uniform->storage = &data[data_pos];

      /* Set remap table entries point to correct gl_uniform_storage. */
      for (unsigned j = 0; j < entries; j++) {
         unsigned element_loc = uniform->remap_location + j;
         prog->UniformRemapTable[element_loc] = uniform;

         if (uniform->block_index == -1)
            data_pos += num_slots;
      }
   }

   /* Verify that total amount of entries for explicit and implicit locations
    * is less than MAX_UNIFORM_LOCATIONS.
    */
   if (total_entries > consts->MaxUserAssignableUniformLocations) {
      linker_error(prog, "count of uniform locations > MAX_UNIFORM_LOCATIONS"
                   "(%u > %u)", total_entries,
                   consts->MaxUserAssignableUniformLocations);
   }

   /* Reserve all the explicit locations of the active subroutine uniforms. */
   for (unsigned i = 0; i < prog->data->NumUniformStorage; i++) {
      struct gl_uniform_storage *uniform = &prog->data->UniformStorage[i];

      if (glsl_get_base_type(uniform->type) != GLSL_TYPE_SUBROUTINE)
         continue;

      if (prog->data->UniformStorage[i].remap_location == UNMAPPED_UNIFORM_LOC)
         continue;

      /* How many new entries for this uniform? */
      const unsigned entries =
         MAX2(1, prog->data->UniformStorage[i].array_elements);

      uniform->storage = &data[data_pos];

      unsigned num_slots = glsl_get_component_slots(uniform->type);
      unsigned mask = prog->data->linked_stages;
      while (mask) {
         const int j = u_bit_scan(&mask);
         struct gl_program *p = prog->_LinkedShaders[j]->Program;

         if (!prog->data->UniformStorage[i].opaque[j].active)
            continue;

         /* Set remap table entries point to correct gl_uniform_storage. */
         for (unsigned k = 0; k < entries; k++) {
            unsigned element_loc =
               prog->data->UniformStorage[i].remap_location + k;
            p->sh.SubroutineUniformRemapTable[element_loc] =
               &prog->data->UniformStorage[i];

            data_pos += num_slots;
         }
      }
   }

   /* reserve subroutine locations */
   for (unsigned i = 0; i < prog->data->NumUniformStorage; i++) {
      struct gl_uniform_storage *uniform = &prog->data->UniformStorage[i];

      if (glsl_get_base_type(uniform->type) != GLSL_TYPE_SUBROUTINE)
         continue;

      if (prog->data->UniformStorage[i].remap_location !=
          UNMAPPED_UNIFORM_LOC)
         continue;

      const unsigned entries =
         MAX2(1, prog->data->UniformStorage[i].array_elements);

      uniform->storage = &data[data_pos];

      unsigned num_slots = glsl_get_component_slots(uniform->type);
      unsigned mask = prog->data->linked_stages;
      while (mask) {
         const int j = u_bit_scan(&mask);
         struct gl_program *p = prog->_LinkedShaders[j]->Program;

         if (!prog->data->UniformStorage[i].opaque[j].active)
            continue;

         p->sh.SubroutineUniformRemapTable =
            reralloc(p,
                     p->sh.SubroutineUniformRemapTable,
                     struct gl_uniform_storage *,
                     p->sh.NumSubroutineUniformRemapTable + entries);

         for (unsigned k = 0; k < entries; k++) {
            p->sh.SubroutineUniformRemapTable[p->sh.NumSubroutineUniformRemapTable + k] =
               &prog->data->UniformStorage[i];

            data_pos += num_slots;
         }
         prog->data->UniformStorage[i].remap_location =
            p->sh.NumSubroutineUniformRemapTable;
         p->sh.NumSubroutineUniformRemapTable += entries;
      }
   }

   /* assign storage to hidden uniforms */
   for (unsigned i = 0; i < prog->data->NumUniformStorage; i++) {
      struct gl_uniform_storage *uniform = &prog->data->UniformStorage[i];

      if (!uniform->hidden ||
          glsl_get_base_type(uniform->type) == GLSL_TYPE_SUBROUTINE)
         continue;

      const unsigned entries =
         MAX2(1, prog->data->UniformStorage[i].array_elements);

      uniform->storage = &data[data_pos];

      unsigned num_slots = glsl_get_component_slots(uniform->type);
      for (unsigned k = 0; k < entries; k++)
         data_pos += num_slots;
   }
}

static void
add_var_use_deref(nir_deref_instr *deref, struct hash_table *live,
                  struct array_deref_range **derefs, unsigned *derefs_size)
{
   nir_deref_path path;
   nir_deref_path_init(&path, deref, NULL);

   deref = path.path[0];
   if (deref->deref_type != nir_deref_type_var ||
       !nir_deref_mode_is_one_of(deref, nir_var_uniform |
                                        nir_var_mem_ubo |
                                        nir_var_mem_ssbo |
                                        nir_var_image)) {
      nir_deref_path_finish(&path);
      return;
   }

   /* Number of derefs used in current processing. */
   unsigned num_derefs = 0;

   const struct glsl_type *deref_type = deref->var->type;
   nir_deref_instr **p = &path.path[1];
   for (; *p; p++) {
      if ((*p)->deref_type == nir_deref_type_array) {

         /* Skip matrix derefences */
         if (!glsl_type_is_array(deref_type))
            break;

         if ((num_derefs + 1) * sizeof(struct array_deref_range) > *derefs_size) {
            void *ptr = reralloc_size(NULL, *derefs, *derefs_size + 4096);

            if (ptr == NULL) {
               nir_deref_path_finish(&path);
               return;
            }

            *derefs_size += 4096;
            *derefs = (struct array_deref_range *)ptr;
         }

         struct array_deref_range *dr = &(*derefs)[num_derefs];
         num_derefs++;

         dr->size = glsl_get_length(deref_type);

         if (nir_src_is_const((*p)->arr.index)) {
            dr->index = nir_src_as_uint((*p)->arr.index);
         } else {
            /* An unsized array can occur at the end of an SSBO.  We can't track
             * accesses to such an array, so bail.
             */
            if (dr->size == 0) {
               nir_deref_path_finish(&path);
               return;
            }

            dr->index = dr->size;
         }

         deref_type = glsl_get_array_element(deref_type);
      } else if ((*p)->deref_type == nir_deref_type_struct) {
         /* We have reached the end of the array. */
         break;
      }
   }

   nir_deref_path_finish(&path);


   struct uniform_array_info *ainfo = NULL;

   struct hash_entry *entry =
      _mesa_hash_table_search(live, deref->var->name);
   if (!entry && glsl_type_is_array(deref->var->type)) {
      ainfo = ralloc(live, struct uniform_array_info);

      unsigned num_bits = MAX2(1, glsl_get_aoa_size(deref->var->type));
      ainfo->indices = rzalloc_array(live, BITSET_WORD, BITSET_WORDS(num_bits));

      ainfo->deref_list = ralloc(live, struct util_dynarray);
      util_dynarray_init(ainfo->deref_list, live);
   }

   if (entry)
      ainfo = (struct uniform_array_info *) entry->data;

   if (glsl_type_is_array(deref->var->type)) {
      /* Count the "depth" of the arrays-of-arrays. */
      unsigned array_depth = 0;
      for (const struct glsl_type *type = deref->var->type;
           glsl_type_is_array(type);
           type = glsl_get_array_element(type)) {
         array_depth++;
      }

      link_util_mark_array_elements_referenced(*derefs, num_derefs, array_depth,
                                               ainfo->indices);

      util_dynarray_append(ainfo->deref_list, nir_deref_instr *, deref);
   }

   assert(deref->modes == deref->var->data.mode);
   _mesa_hash_table_insert(live, deref->var->name, ainfo);
}

/* Iterate over the shader and collect infomation about uniform use */
static void
add_var_use_shader(nir_shader *shader, struct hash_table *live)
{
   /* Currently allocated buffer block of derefs. */
   struct array_deref_range *derefs = NULL;

   /* Size of the derefs buffer in bytes. */
   unsigned derefs_size = 0;

   nir_foreach_function_impl(impl, shader) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            if (instr->type == nir_instr_type_intrinsic) {
               nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
               switch (intr->intrinsic) {
               case nir_intrinsic_atomic_counter_read_deref:
               case nir_intrinsic_atomic_counter_inc_deref:
               case nir_intrinsic_atomic_counter_pre_dec_deref:
               case nir_intrinsic_atomic_counter_post_dec_deref:
               case nir_intrinsic_atomic_counter_add_deref:
               case nir_intrinsic_atomic_counter_min_deref:
               case nir_intrinsic_atomic_counter_max_deref:
               case nir_intrinsic_atomic_counter_and_deref:
               case nir_intrinsic_atomic_counter_or_deref:
               case nir_intrinsic_atomic_counter_xor_deref:
               case nir_intrinsic_atomic_counter_exchange_deref:
               case nir_intrinsic_atomic_counter_comp_swap_deref:
               case nir_intrinsic_image_deref_load:
               case nir_intrinsic_image_deref_store:
               case nir_intrinsic_image_deref_atomic:
               case nir_intrinsic_image_deref_atomic_swap:
               case nir_intrinsic_image_deref_size:
               case nir_intrinsic_image_deref_samples:
               case nir_intrinsic_load_deref:
               case nir_intrinsic_store_deref:
                  add_var_use_deref(nir_src_as_deref(intr->src[0]), live,
                                    &derefs, &derefs_size);
                  break;

               default:
                  /* Nothing to do */
                  break;
               }
            } else if (instr->type == nir_instr_type_tex) {
               nir_tex_instr *tex_instr = nir_instr_as_tex(instr);
               int sampler_idx =
                  nir_tex_instr_src_index(tex_instr,
                                          nir_tex_src_sampler_deref);
               int texture_idx =
                  nir_tex_instr_src_index(tex_instr,
                                          nir_tex_src_texture_deref);

               if (sampler_idx >= 0) {
                  nir_deref_instr *deref =
                     nir_src_as_deref(tex_instr->src[sampler_idx].src);
                  add_var_use_deref(deref, live, &derefs, &derefs_size);
               }

               if (texture_idx >= 0) {
                  nir_deref_instr *deref =
                     nir_src_as_deref(tex_instr->src[texture_idx].src);
                  add_var_use_deref(deref, live, &derefs, &derefs_size);
               }
            }
         }
      }
   }

   ralloc_free(derefs);
}

static void
mark_stage_as_active(struct gl_uniform_storage *uniform,
                     unsigned stage)
{
   uniform->active_shader_mask |= 1 << stage;
}

/* Used to build a tree representing the glsl_type so that we can have a place
 * to store the next index for opaque types. Array types are expanded so that
 * they have a single child which is used for all elements of the array.
 * Struct types have a child for each member. The tree is walked while
 * processing a uniform so that we can recognise when an opaque type is
 * encountered a second time in order to reuse the same range of indices that
 * was reserved the first time. That way the sampler indices can be arranged
 * so that members of an array are placed sequentially even if the array is an
 * array of structs containing other opaque members.
 */
struct type_tree_entry {
   /* For opaque types, this will be the next index to use. If we haven’t
    * encountered this member yet, it will be UINT_MAX.
    */
   unsigned next_index;
   unsigned array_size;
   struct type_tree_entry *parent;
   struct type_tree_entry *next_sibling;
   struct type_tree_entry *children;
};

struct nir_link_uniforms_state {
   /* per-whole program */
   unsigned num_hidden_uniforms;
   unsigned num_values;
   unsigned max_uniform_location;

   /* per-shader stage */
   unsigned next_bindless_image_index;
   unsigned next_bindless_sampler_index;
   unsigned next_image_index;
   unsigned next_sampler_index;
   unsigned next_subroutine;
   unsigned num_shader_samplers;
   unsigned num_shader_images;
   unsigned num_shader_uniform_components;
   unsigned shader_samplers_used;
   unsigned shader_shadow_samplers;
   unsigned shader_storage_blocks_write_access;
   struct gl_program_parameter_list *params;

   /* per-variable */
   nir_variable *current_var;
   const struct glsl_type *current_ifc_type;
   int offset;
   bool var_is_in_block;
   bool set_top_level_array;
   int top_level_array_size;
   int top_level_array_stride;

   struct type_tree_entry *current_type;
   struct hash_table *referenced_uniforms[MESA_SHADER_STAGES];
   struct hash_table *uniform_hash;
};

static void
add_parameter(struct gl_uniform_storage *uniform,
              const struct gl_constants *consts,
              struct gl_shader_program *prog,
              const struct glsl_type *type,
              struct nir_link_uniforms_state *state)
{
   /* Builtin uniforms are backed by PROGRAM_STATE_VAR, so don't add them as
    * uniforms.
    */
   if (uniform->builtin)
      return;

   if (!state->params || uniform->is_shader_storage ||
       (glsl_contains_opaque(type) && !state->current_var->data.bindless))
      return;

   unsigned num_params = glsl_get_aoa_size(type);
   num_params = MAX2(num_params, 1);
   num_params *= glsl_get_matrix_columns(glsl_without_array(type));

   bool is_dual_slot = glsl_type_is_dual_slot(glsl_without_array(type));
   if (is_dual_slot)
      num_params *= 2;

   struct gl_program_parameter_list *params = state->params;
   int base_index = params->NumParameters;
   _mesa_reserve_parameter_storage(params, num_params, num_params);

   if (consts->PackedDriverUniformStorage) {
      for (unsigned i = 0; i < num_params; i++) {
         unsigned dmul = glsl_type_is_64bit(glsl_without_array(type)) ? 2 : 1;
         unsigned comps = glsl_get_vector_elements(glsl_without_array(type)) * dmul;
         if (is_dual_slot) {
            if (i & 0x1)
               comps -= 4;
            else
               comps = 4;
         }

         /* TODO: This will waste space with 1 and 3 16-bit components. */
         if (glsl_type_is_16bit(glsl_without_array(type)))
            comps = DIV_ROUND_UP(comps, 2);

         _mesa_add_parameter(params, PROGRAM_UNIFORM, uniform->name.string, comps,
                             glsl_get_gl_type(type), NULL, NULL, false);
      }
   } else {
      for (unsigned i = 0; i < num_params; i++) {
         _mesa_add_parameter(params, PROGRAM_UNIFORM, uniform->name.string, 4,
                             glsl_get_gl_type(type), NULL, NULL, true);
      }
   }

   /* Each Parameter will hold the index to the backing uniform storage.
    * This avoids relying on names to match parameters and uniform
    * storages.
    */
   for (unsigned i = 0; i < num_params; i++) {
      struct gl_program_parameter *param = &params->Parameters[base_index + i];
      param->UniformStorageIndex = uniform - prog->data->UniformStorage;
      param->MainUniformStorageIndex = state->current_var->data.location;
   }
}

static unsigned
get_next_index(struct nir_link_uniforms_state *state,
               const struct gl_uniform_storage *uniform,
               unsigned *next_index, bool *initialised)
{
   /* If we’ve already calculated an index for this member then we can just
    * offset from there.
    */
   if (state->current_type->next_index == UINT_MAX) {
      /* Otherwise we need to reserve enough indices for all of the arrays
       * enclosing this member.
       */

      unsigned array_size = 1;

      for (const struct type_tree_entry *p = state->current_type;
           p;
           p = p->parent) {
         array_size *= p->array_size;
      }

      state->current_type->next_index = *next_index;
      *next_index += array_size;
      *initialised = true;
   } else
      *initialised = false;

   unsigned index = state->current_type->next_index;

   state->current_type->next_index += MAX2(1, uniform->array_elements);

   return index;
}

static gl_texture_index
texture_index_for_type(const struct glsl_type *type)
{
   const bool sampler_array = glsl_sampler_type_is_array(type);
   switch (glsl_get_sampler_dim(type)) {
   case GLSL_SAMPLER_DIM_1D:
      return sampler_array ? TEXTURE_1D_ARRAY_INDEX : TEXTURE_1D_INDEX;
   case GLSL_SAMPLER_DIM_2D:
      return sampler_array ? TEXTURE_2D_ARRAY_INDEX : TEXTURE_2D_INDEX;
   case GLSL_SAMPLER_DIM_3D:
      return TEXTURE_3D_INDEX;
   case GLSL_SAMPLER_DIM_CUBE:
      return sampler_array ? TEXTURE_CUBE_ARRAY_INDEX : TEXTURE_CUBE_INDEX;
   case GLSL_SAMPLER_DIM_RECT:
      return TEXTURE_RECT_INDEX;
   case GLSL_SAMPLER_DIM_BUF:
      return TEXTURE_BUFFER_INDEX;
   case GLSL_SAMPLER_DIM_EXTERNAL:
      return TEXTURE_EXTERNAL_INDEX;
   case GLSL_SAMPLER_DIM_MS:
      return sampler_array ? TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX :
                             TEXTURE_2D_MULTISAMPLE_INDEX;
   default:
      assert(!"Should not get here.");
      return TEXTURE_BUFFER_INDEX;
   }
}

/* Update the uniforms info for the current shader stage */
static void
update_uniforms_shader_info(struct gl_shader_program *prog,
                            struct nir_link_uniforms_state *state,
                            struct gl_uniform_storage *uniform,
                            const struct glsl_type *type,
                            unsigned stage)
{
   unsigned values = glsl_get_component_slots(type);
   const struct glsl_type *type_no_array = glsl_without_array(type);

   if (glsl_type_is_sampler(type_no_array)) {
      bool init_idx;
      /* ARB_bindless_texture spec says:
       *
       *    "When used as shader inputs, outputs, uniform block members,
       *     or temporaries, the value of the sampler is a 64-bit unsigned
       *     integer handle and never refers to a texture image unit."
       */
      bool is_bindless = state->current_var->data.bindless || state->var_is_in_block;
      unsigned *next_index = is_bindless ?
         &state->next_bindless_sampler_index :
         &state->next_sampler_index;
      int sampler_index = get_next_index(state, uniform, next_index, &init_idx);
      struct gl_linked_shader *sh = prog->_LinkedShaders[stage];

      if (is_bindless) {
         if (init_idx) {
            sh->Program->sh.BindlessSamplers =
               rerzalloc(sh->Program, sh->Program->sh.BindlessSamplers,
                         struct gl_bindless_sampler,
                         sh->Program->sh.NumBindlessSamplers,
                         state->next_bindless_sampler_index);

            for (unsigned j = sh->Program->sh.NumBindlessSamplers;
                 j < state->next_bindless_sampler_index; j++) {
               sh->Program->sh.BindlessSamplers[j].target =
                  texture_index_for_type(type_no_array);
            }

            sh->Program->sh.NumBindlessSamplers =
               state->next_bindless_sampler_index;
         }

         if (!state->var_is_in_block)
            state->num_shader_uniform_components += values;
      } else {
         /* Samplers (bound or bindless) are counted as two components
          * as specified by ARB_bindless_texture.
          */
         state->num_shader_samplers += values / 2;

         if (init_idx) {
            const unsigned shadow = glsl_sampler_type_is_shadow(type_no_array);
            for (unsigned i = sampler_index;
                 i < MIN2(state->next_sampler_index, MAX_SAMPLERS); i++) {
               sh->Program->sh.SamplerTargets[i] =
                  texture_index_for_type(type_no_array);
               state->shader_samplers_used |= 1U << i;
               state->shader_shadow_samplers |= shadow << i;
            }
         }
      }

      uniform->opaque[stage].active = true;
      uniform->opaque[stage].index = sampler_index;
   } else if (glsl_type_is_image(type_no_array)) {
      struct gl_linked_shader *sh = prog->_LinkedShaders[stage];

      /* Set image access qualifiers */
      enum gl_access_qualifier image_access =
         state->current_var->data.access;

      int image_index;
      if (state->current_var->data.bindless) {
         image_index = state->next_bindless_image_index;
         state->next_bindless_image_index += MAX2(1, uniform->array_elements);

         sh->Program->sh.BindlessImages =
            rerzalloc(sh->Program, sh->Program->sh.BindlessImages,
                      struct gl_bindless_image,
                      sh->Program->sh.NumBindlessImages,
                      state->next_bindless_image_index);

         for (unsigned j = sh->Program->sh.NumBindlessImages;
              j < state->next_bindless_image_index; j++) {
            sh->Program->sh.BindlessImages[j].image_access = image_access;
         }

         sh->Program->sh.NumBindlessImages = state->next_bindless_image_index;

      } else {
         image_index = state->next_image_index;
         state->next_image_index += MAX2(1, uniform->array_elements);

         /* Images (bound or bindless) are counted as two components as
          * specified by ARB_bindless_texture.
          */
         state->num_shader_images += values / 2;

         for (unsigned i = image_index;
              i < MIN2(state->next_image_index, MAX_IMAGE_UNIFORMS); i++) {
            sh->Program->sh.image_access[i] = image_access;
         }
      }

      uniform->opaque[stage].active = true;
      uniform->opaque[stage].index = image_index;

      if (!uniform->is_shader_storage)
         state->num_shader_uniform_components += values;
   } else {
      if (glsl_get_base_type(type_no_array) == GLSL_TYPE_SUBROUTINE) {
         struct gl_linked_shader *sh = prog->_LinkedShaders[stage];

         uniform->opaque[stage].index = state->next_subroutine;
         uniform->opaque[stage].active = true;

         sh->Program->sh.NumSubroutineUniforms++;

         /* Increment the subroutine index by 1 for non-arrays and by the
          * number of array elements for arrays.
          */
         state->next_subroutine += MAX2(1, uniform->array_elements);
      }

      if (!state->var_is_in_block)
         state->num_shader_uniform_components += values;
   }
}

static bool
find_and_update_named_uniform_storage(const struct gl_constants *consts,
                                      struct gl_shader_program *prog,
                                      struct nir_link_uniforms_state *state,
                                      nir_variable *var, char **name,
                                      size_t name_length,
                                      const struct glsl_type *type,
                                      unsigned stage, bool *first_element)
{
   /* gl_uniform_storage can cope with one level of array, so if the type is a
    * composite type or an array where each element occupies more than one
    * location than we need to recursively process it.
    */
   if (glsl_type_is_struct_or_ifc(type) ||
       (glsl_type_is_array(type) &&
        (glsl_type_is_array(glsl_get_array_element(type)) ||
         glsl_type_is_struct_or_ifc(glsl_get_array_element(type))))) {

      struct type_tree_entry *old_type = state->current_type;
      state->current_type = old_type->children;

      /* Shader storage block unsized arrays: add subscript [0] to variable
       * names.
       */
      unsigned length = glsl_get_length(type);
      if (glsl_type_is_unsized_array(type))
         length = 1;

      bool result = false;
      for (unsigned i = 0; i < length; i++) {
         const struct glsl_type *field_type;
         size_t new_length = name_length;

         if (glsl_type_is_struct_or_ifc(type)) {
            field_type = glsl_get_struct_field(type, i);

            /* Append '.field' to the current variable name. */
            if (name) {
               ralloc_asprintf_rewrite_tail(name, &new_length, ".%s",
                                            glsl_get_struct_elem_name(type, i));
            }
         } else {
            field_type = glsl_get_array_element(type);

            /* Append the subscript to the current variable name */
            if (name)
               ralloc_asprintf_rewrite_tail(name, &new_length, "[%u]", i);
         }

         result = find_and_update_named_uniform_storage(consts, prog, state,
                                                        var, name, new_length,
                                                        field_type, stage,
                                                        first_element);

         if (glsl_type_is_struct_or_ifc(type))
            state->current_type = state->current_type->next_sibling;

         if (!result) {
            state->current_type = old_type;
            return false;
         }
      }

      state->current_type = old_type;

      return result;
   } else {
      struct hash_entry *entry =
         _mesa_hash_table_search(state->uniform_hash, *name);
      if (entry) {
         unsigned i = (unsigned) (intptr_t) entry->data;
         struct gl_uniform_storage *uniform = &prog->data->UniformStorage[i];

         if (*first_element && !state->var_is_in_block) {
            *first_element = false;
            var->data.location = uniform - prog->data->UniformStorage;
         }

         update_uniforms_shader_info(prog, state, uniform, type, stage);

         const struct glsl_type *type_no_array = glsl_without_array(type);
         struct hash_entry *entry = prog->data->spirv ? NULL :
            _mesa_hash_table_search(state->referenced_uniforms[stage],
                                    state->current_var->name);
         if (entry != NULL ||
             glsl_get_base_type(type_no_array) == GLSL_TYPE_SUBROUTINE ||
             prog->data->spirv)
            uniform->active_shader_mask |= 1 << stage;

         if (!state->var_is_in_block)
            add_parameter(uniform, consts, prog, type, state);

         return true;
      }
   }

   return false;
}

/**
 * Finds, returns, and updates the stage info for any uniform in UniformStorage
 * defined by @var. For GLSL this is done using the name, for SPIR-V in general
 * is this done using the explicit location, except:
 *
 * * UBOs/SSBOs: as they lack explicit location, binding is used to locate
 *   them. That means that more that one entry at the uniform storage can be
 *   found. In that case all of them are updated, and the first entry is
 *   returned, in order to update the location of the nir variable.
 *
 * * Special uniforms: like atomic counters. They lack a explicit location,
 *   so they are skipped. They will be handled and assigned a location later.
 *
 */
static bool
find_and_update_previous_uniform_storage(const struct gl_constants *consts,
                                         struct gl_shader_program *prog,
                                         struct nir_link_uniforms_state *state,
                                         nir_variable *var, char *name,
                                         const struct glsl_type *type,
                                         unsigned stage)
{
   if (!prog->data->spirv) {
      bool first_element = true;
      char *name_tmp = ralloc_strdup(NULL, name);
      bool r = find_and_update_named_uniform_storage(consts, prog, state, var,
                                                     &name_tmp,
                                                     strlen(name_tmp), type,
                                                     stage, &first_element);
      ralloc_free(name_tmp);

      return r;
   }

   if (nir_variable_is_in_block(var)) {
      struct gl_uniform_storage *uniform = NULL;

      ASSERTED unsigned num_blks = nir_variable_is_in_ubo(var) ?
         prog->data->NumUniformBlocks :
         prog->data->NumShaderStorageBlocks;

      struct gl_uniform_block *blks = nir_variable_is_in_ubo(var) ?
         prog->data->UniformBlocks : prog->data->ShaderStorageBlocks;

      bool result = false;
      for (unsigned i = 0; i < prog->data->NumUniformStorage; i++) {
         /* UniformStorage contains both variables from ubos and ssbos */
         if ( prog->data->UniformStorage[i].is_shader_storage !=
              nir_variable_is_in_ssbo(var))
            continue;

         int block_index = prog->data->UniformStorage[i].block_index;
         if (block_index != -1) {
            assert(block_index < num_blks);

            if (var->data.binding == blks[block_index].Binding) {
               if (!uniform)
                  uniform = &prog->data->UniformStorage[i];
               mark_stage_as_active(&prog->data->UniformStorage[i],
                                      stage);
               result = true;
            }
         }
      }

      if (result)
         var->data.location = uniform - prog->data->UniformStorage;
      return result;
   }

   /* Beyond blocks, there are still some corner cases of uniforms without
    * location (ie: atomic counters) that would have a initial location equal
    * to -1. We just return on that case. Those uniforms will be handled
    * later.
    */
   if (var->data.location == -1)
      return false;

   /* TODO: following search can be problematic with shaders with a lot of
    * uniforms. Would it be better to use some type of hash
    */
   for (unsigned i = 0; i < prog->data->NumUniformStorage; i++) {
      if (prog->data->UniformStorage[i].remap_location == var->data.location) {
         mark_stage_as_active(&prog->data->UniformStorage[i], stage);

         struct gl_uniform_storage *uniform = &prog->data->UniformStorage[i];
         var->data.location = uniform - prog->data->UniformStorage;
         add_parameter(uniform, consts, prog, var->type, state);
         return true;
      }
   }

   return false;
}

static struct type_tree_entry *
build_type_tree_for_type(const struct glsl_type *type)
{
   struct type_tree_entry *entry = malloc(sizeof *entry);

   entry->array_size = 1;
   entry->next_index = UINT_MAX;
   entry->children = NULL;
   entry->next_sibling = NULL;
   entry->parent = NULL;

   if (glsl_type_is_array(type)) {
      entry->array_size = glsl_get_length(type);
      entry->children = build_type_tree_for_type(glsl_get_array_element(type));
      entry->children->parent = entry;
   } else if (glsl_type_is_struct_or_ifc(type)) {
      struct type_tree_entry *last = NULL;

      for (unsigned i = 0; i < glsl_get_length(type); i++) {
         const struct glsl_type *field_type = glsl_get_struct_field(type, i);
         struct type_tree_entry *field_entry =
            build_type_tree_for_type(field_type);

         if (last == NULL)
            entry->children = field_entry;
         else
            last->next_sibling = field_entry;

         field_entry->parent = entry;

         last = field_entry;
      }
   }

   return entry;
}

static void
free_type_tree(struct type_tree_entry *entry)
{
   struct type_tree_entry *p, *next;

   for (p = entry->children; p; p = next) {
      next = p->next_sibling;
      free_type_tree(p);
   }

   free(entry);
}

static void
hash_free_uniform_name(struct hash_entry *entry)
{
   free((void*)entry->key);
}

static void
enter_record(struct nir_link_uniforms_state *state,
             const struct gl_constants *consts,
             const struct glsl_type *type,
             bool row_major)
{
   assert(glsl_type_is_struct(type));
   if (!state->var_is_in_block)
      return;

   bool use_std430 = consts->UseSTD430AsDefaultPacking;
   const enum glsl_interface_packing packing =
      glsl_get_internal_ifc_packing(state->current_var->interface_type,
                                    use_std430);

   if (packing == GLSL_INTERFACE_PACKING_STD430)
      state->offset = align(
         state->offset, glsl_get_std430_base_alignment(type, row_major));
   else
      state->offset = align(
         state->offset, glsl_get_std140_base_alignment(type, row_major));
}

static void
leave_record(struct nir_link_uniforms_state *state,
             const struct gl_constants *consts,
             const struct glsl_type *type,
             bool row_major)
{
   assert(glsl_type_is_struct(type));
   if (!state->var_is_in_block)
      return;

   bool use_std430 = consts->UseSTD430AsDefaultPacking;
   const enum glsl_interface_packing packing =
      glsl_get_internal_ifc_packing(state->current_var->interface_type,
                                    use_std430);

   if (packing == GLSL_INTERFACE_PACKING_STD430)
      state->offset = align(
         state->offset, glsl_get_std430_base_alignment(type, row_major));
   else
      state->offset = align(
         state->offset, glsl_get_std140_base_alignment(type, row_major));
}

/**
 * Creates the neccessary entries in UniformStorage for the uniform. Returns
 * the number of locations used or -1 on failure.
 */
static int
nir_link_uniform(const struct gl_constants *consts,
                 struct gl_shader_program *prog,
                 struct gl_program *stage_program,
                 gl_shader_stage stage,
                 const struct glsl_type *type,
                 unsigned index_in_parent,
                 int location,
                 struct nir_link_uniforms_state *state,
                 char **name, size_t name_length, bool row_major)
{
   struct gl_uniform_storage *uniform = NULL;

   if (state->set_top_level_array &&
       nir_variable_is_in_ssbo(state->current_var)) {
      /* Type is the top level SSBO member */
      if (glsl_type_is_array(type) &&
          (glsl_type_is_array(glsl_get_array_element(type)) ||
           glsl_type_is_struct_or_ifc(glsl_get_array_element(type)))) {
         /* Type is a top-level array (array of aggregate types) */
         state->top_level_array_size = glsl_get_length(type);
         state->top_level_array_stride = glsl_get_explicit_stride(type);
      } else {
         state->top_level_array_size = 1;
         state->top_level_array_stride = 0;
      }

      state->set_top_level_array = false;
   }

   /* gl_uniform_storage can cope with one level of array, so if the type is a
    * composite type or an array where each element occupies more than one
    * location than we need to recursively process it.
    */
   if (glsl_type_is_struct_or_ifc(type) ||
       (glsl_type_is_array(type) &&
        (glsl_type_is_array(glsl_get_array_element(type)) ||
         glsl_type_is_struct_or_ifc(glsl_get_array_element(type))))) {
      int location_count = 0;
      struct type_tree_entry *old_type = state->current_type;
      unsigned int struct_base_offset = state->offset;

      state->current_type = old_type->children;

      /* Shader storage block unsized arrays: add subscript [0] to variable
       * names.
       */
      unsigned length = glsl_get_length(type);
      if (glsl_type_is_unsized_array(type))
         length = 1;

      if (glsl_type_is_struct(type) && !prog->data->spirv)
         enter_record(state, consts, type, row_major);

      for (unsigned i = 0; i < length; i++) {
         const struct glsl_type *field_type;
         size_t new_length = name_length;
         bool field_row_major = row_major;

         if (glsl_type_is_struct_or_ifc(type)) {
            field_type = glsl_get_struct_field(type, i);
            /* Use the offset inside the struct only for variables backed by
             * a buffer object. For variables not backed by a buffer object,
             * offset is -1.
             */
            if (state->var_is_in_block) {
               if (prog->data->spirv) {
                  state->offset =
                     struct_base_offset + glsl_get_struct_field_offset(type, i);
               } else if (glsl_get_struct_field_offset(type, i) != -1 &&
                          type == state->current_ifc_type) {
                  state->offset = glsl_get_struct_field_offset(type, i);
               }

               if (glsl_type_is_interface(type))
                  state->set_top_level_array = true;
            }

            /* Append '.field' to the current variable name. */
            if (name) {
               ralloc_asprintf_rewrite_tail(name, &new_length, ".%s",
                                            glsl_get_struct_elem_name(type, i));
            }


            /* The layout of structures at the top level of the block is set
             * during parsing.  For matrices contained in multiple levels of
             * structures in the block, the inner structures have no layout.
             * These cases must potentially inherit the layout from the outer
             * levels.
             */
            const enum glsl_matrix_layout matrix_layout =
               glsl_get_struct_field_data(type, i)->matrix_layout;
            if (matrix_layout == GLSL_MATRIX_LAYOUT_ROW_MAJOR) {
               field_row_major = true;
            } else if (matrix_layout == GLSL_MATRIX_LAYOUT_COLUMN_MAJOR) {
               field_row_major = false;
            }
         } else {
            field_type = glsl_get_array_element(type);

            /* Append the subscript to the current variable name */
            if (name)
               ralloc_asprintf_rewrite_tail(name, &new_length, "[%u]", i);
         }

         int entries = nir_link_uniform(consts, prog, stage_program, stage,
                                        field_type, i, location,
                                        state, name, new_length,
                                        field_row_major);

         if (entries == -1)
            return -1;

         if (location != -1)
            location += entries;
         location_count += entries;

         if (glsl_type_is_struct_or_ifc(type))
            state->current_type = state->current_type->next_sibling;
      }

      if (glsl_type_is_struct(type) && !prog->data->spirv)
         leave_record(state, consts, type, row_major);

      state->current_type = old_type;

      return location_count;
   } else {
      /* TODO: reallocating storage is slow, we should figure out a way to
       * allocate storage up front for spirv like we do for GLSL.
       */
      if (prog->data->spirv) {
         /* Create a new uniform storage entry */
         prog->data->UniformStorage =
            reralloc(prog->data,
                     prog->data->UniformStorage,
                     struct gl_uniform_storage,
                     prog->data->NumUniformStorage + 1);
         if (!prog->data->UniformStorage) {
            linker_error(prog, "Out of memory during linking.\n");
            return -1;
         }
      }

      uniform = &prog->data->UniformStorage[prog->data->NumUniformStorage];
      prog->data->NumUniformStorage++;

      /* Initialize its members */
      memset(uniform, 0x00, sizeof(struct gl_uniform_storage));

      uniform->name.string =
         name ? ralloc_strdup(prog->data->UniformStorage, *name) : NULL;
      resource_name_updated(&uniform->name);

      const struct glsl_type *type_no_array = glsl_without_array(type);
      if (glsl_type_is_array(type)) {
         uniform->type = type_no_array;
         uniform->array_elements = glsl_get_length(type);
      } else {
         uniform->type = type;
         uniform->array_elements = 0;
      }
      uniform->top_level_array_size = state->top_level_array_size;
      uniform->top_level_array_stride = state->top_level_array_stride;

      struct hash_entry *entry = prog->data->spirv ? NULL :
         _mesa_hash_table_search(state->referenced_uniforms[stage],
                                 state->current_var->name);
      if (entry != NULL ||
          glsl_get_base_type(type_no_array) == GLSL_TYPE_SUBROUTINE ||
          prog->data->spirv)
         uniform->active_shader_mask |= 1 << stage;

      if (location >= 0) {
         /* Uniform has an explicit location */
         uniform->remap_location = location;
      } else {
         uniform->remap_location = UNMAPPED_UNIFORM_LOC;
      }

      uniform->hidden = state->current_var->data.how_declared == nir_var_hidden;
      if (uniform->hidden)
         state->num_hidden_uniforms++;

      uniform->is_shader_storage = nir_variable_is_in_ssbo(state->current_var);
      uniform->is_bindless = state->current_var->data.bindless;

      /* Set fields whose default value depend on the variable being inside a
       * block.
       *
       * From the OpenGL 4.6 spec, 7.3 Program objects:
       *
       * "For the property ARRAY_STRIDE, ... For active variables not declared
       * as an array of basic types, zero is written to params. For active
       * variables not backed by a buffer object, -1 is written to params,
       * regardless of the variable type."
       *
       * "For the property MATRIX_STRIDE, ... For active variables not declared
       * as a matrix or array of matrices, zero is written to params. For active
       * variables not backed by a buffer object, -1 is written to params,
       * regardless of the variable type."
       *
       * For the property IS_ROW_MAJOR, ... For active variables backed by a
       * buffer object, declared as a single matrix or array of matrices, and
       * stored in row-major order, one is written to params. For all other
       * active variables, zero is written to params.
       */
      uniform->array_stride = -1;
      uniform->matrix_stride = -1;
      uniform->row_major = false;

      if (state->var_is_in_block) {
         uniform->array_stride = glsl_type_is_array(type) ?
            glsl_get_explicit_stride(type) : 0;

         if (glsl_type_is_matrix(uniform->type)) {
            uniform->matrix_stride = glsl_get_explicit_stride(uniform->type);
            uniform->row_major = glsl_matrix_type_is_row_major(uniform->type);
         } else {
            uniform->matrix_stride = 0;
         }

         if (!prog->data->spirv) {
            bool use_std430 = consts->UseSTD430AsDefaultPacking;
            const enum glsl_interface_packing packing =
               glsl_get_internal_ifc_packing(state->current_var->interface_type,
                                             use_std430);

            unsigned alignment =
               glsl_get_std140_base_alignment(type, uniform->row_major);
            if (packing == GLSL_INTERFACE_PACKING_STD430) {
               alignment =
                  glsl_get_std430_base_alignment(type, uniform->row_major);
            }
            state->offset = align(state->offset, alignment);
         }
      }

      uniform->offset = state->var_is_in_block ? state->offset : -1;

      int buffer_block_index = -1;
      /* If the uniform is inside a uniform block determine its block index by
       * comparing the bindings, we can not use names.
       */
      if (state->var_is_in_block) {
         struct gl_uniform_block *blocks = nir_variable_is_in_ssbo(state->current_var) ?
            prog->data->ShaderStorageBlocks : prog->data->UniformBlocks;

         int num_blocks = nir_variable_is_in_ssbo(state->current_var) ?
            prog->data->NumShaderStorageBlocks : prog->data->NumUniformBlocks;

         if (!prog->data->spirv) {
            bool is_interface_array =
               glsl_without_array(state->current_var->type) == state->current_var->interface_type &&
               glsl_type_is_array(state->current_var->type);

            const char *ifc_name =
               glsl_get_type_name(state->current_var->interface_type);
            if (is_interface_array) {
               unsigned l = strlen(ifc_name);
               for (unsigned i = 0; i < num_blocks; i++) {
                  if (strncmp(ifc_name, blocks[i].name.string, l) == 0 &&
                      blocks[i].name.string[l] == '[') {
                     buffer_block_index = i;
                     break;
                  }
               }
            } else {
               for (unsigned i = 0; i < num_blocks; i++) {
                  if (strcmp(ifc_name, blocks[i].name.string) == 0) {
                     buffer_block_index = i;
                     break;
                  }
               }
            }

            /* Compute the next offset. */
            bool use_std430 = consts->UseSTD430AsDefaultPacking;
            const enum glsl_interface_packing packing =
               glsl_get_internal_ifc_packing(state->current_var->interface_type,
                                             use_std430);
            if (packing == GLSL_INTERFACE_PACKING_STD430)
               state->offset += glsl_get_std430_size(type, uniform->row_major);
            else
               state->offset += glsl_get_std140_size(type, uniform->row_major);
         } else {
            for (unsigned i = 0; i < num_blocks; i++) {
               if (state->current_var->data.binding == blocks[i].Binding) {
                  buffer_block_index = i;
                  break;
               }
            }

            /* Compute the next offset. */
            state->offset += glsl_get_explicit_size(type, true);
         }
         assert(buffer_block_index >= 0);
      }

      uniform->block_index = buffer_block_index;
      uniform->builtin = is_gl_identifier(uniform->name.string);
      uniform->atomic_buffer_index = -1;

      /* The following are not for features not supported by ARB_gl_spirv */
      uniform->num_compatible_subroutines = 0;

      unsigned entries = MAX2(1, uniform->array_elements);
      unsigned values = glsl_get_component_slots(type);

      update_uniforms_shader_info(prog, state, uniform, type, stage);

      if (uniform->remap_location != UNMAPPED_UNIFORM_LOC &&
          state->max_uniform_location < uniform->remap_location + entries)
         state->max_uniform_location = uniform->remap_location + entries;

      if (!state->var_is_in_block)
         add_parameter(uniform, consts, prog, type, state);

      if (name) {
         _mesa_hash_table_insert(state->uniform_hash, strdup(*name),
                                 (void *) (intptr_t)
                                    (prog->data->NumUniformStorage - 1));
      }

      if (!is_gl_identifier(uniform->name.string) && !uniform->is_shader_storage &&
          !state->var_is_in_block)
         state->num_values += values;

      return MAX2(uniform->array_elements, 1);
   }
}

bool
gl_nir_link_uniforms(const struct gl_constants *consts,
                     struct gl_shader_program *prog,
                     bool fill_parameters)
{
   /* First free up any previous UniformStorage items */
   ralloc_free(prog->data->UniformStorage);
   prog->data->UniformStorage = NULL;
   prog->data->NumUniformStorage = 0;

   /* Iterate through all linked shaders */
   struct nir_link_uniforms_state state = {0,};

   if (!prog->data->spirv) {
      /* Gather information on uniform use */
      for (unsigned stage = 0; stage < MESA_SHADER_STAGES; stage++) {
         struct gl_linked_shader *sh = prog->_LinkedShaders[stage];
         if (!sh)
            continue;

         state.referenced_uniforms[stage] =
            _mesa_hash_table_create(NULL, _mesa_hash_string,
                                    _mesa_key_string_equal);

         nir_shader *nir = sh->Program->nir;
         add_var_use_shader(nir, state.referenced_uniforms[stage]);
      }

      if(!consts->DisableUniformArrayResize) {
         /* Resize uniform arrays based on the maximum array index */
         for (unsigned stage = 0; stage < MESA_SHADER_STAGES; stage++) {
            struct gl_linked_shader *sh = prog->_LinkedShaders[stage];
            if (!sh)
               continue;

            nir_foreach_gl_uniform_variable(var, sh->Program->nir)
               update_array_sizes(prog, var, state.referenced_uniforms, stage);
         }
      }
   }

   /* Count total number of uniforms and allocate storage */
   unsigned storage_size = 0;
   if (!prog->data->spirv) {
      struct set *storage_counted =
         _mesa_set_create(NULL, _mesa_hash_string, _mesa_key_string_equal);
      for (unsigned stage = 0; stage < MESA_SHADER_STAGES; stage++) {
         struct gl_linked_shader *sh = prog->_LinkedShaders[stage];
         if (!sh)
            continue;

         nir_foreach_gl_uniform_variable(var, sh->Program->nir) {
            const struct glsl_type *type = var->type;
            const char *name = var->name;
            if (nir_variable_is_in_block(var) &&
                glsl_without_array(type) == var->interface_type) {
               type = glsl_without_array(var->type);
               name = glsl_get_type_name(type);
            }

            struct set_entry *entry = _mesa_set_search(storage_counted, name);
            if (!entry) {
               storage_size += uniform_storage_size(type);
               _mesa_set_add(storage_counted, name);
            }
         }
      }
      _mesa_set_destroy(storage_counted, NULL);

      prog->data->UniformStorage = rzalloc_array(prog->data,
                                                 struct gl_uniform_storage,
                                                 storage_size);
      if (!prog->data->UniformStorage) {
         linker_error(prog, "Out of memory while linking uniforms.\n");
         return false;
      }
   }

   /* Iterate through all linked shaders */
   state.uniform_hash = _mesa_hash_table_create(NULL, _mesa_hash_string,
                                                _mesa_key_string_equal);

   for (unsigned shader_type = 0; shader_type < MESA_SHADER_STAGES; shader_type++) {
      struct gl_linked_shader *sh = prog->_LinkedShaders[shader_type];
      if (!sh)
         continue;

      nir_shader *nir = sh->Program->nir;
      assert(nir);

      state.next_bindless_image_index = 0;
      state.next_bindless_sampler_index = 0;
      state.next_image_index = 0;
      state.next_sampler_index = 0;
      state.num_shader_samplers = 0;
      state.num_shader_images = 0;
      state.num_shader_uniform_components = 0;
      state.shader_storage_blocks_write_access = 0;
      state.shader_samplers_used = 0;
      state.shader_shadow_samplers = 0;
      state.params = fill_parameters ? sh->Program->Parameters : NULL;

      nir_foreach_gl_uniform_variable(var, nir) {
         state.current_var = var;
         state.current_ifc_type = NULL;
         state.offset = 0;
         state.var_is_in_block = nir_variable_is_in_block(var);
         state.set_top_level_array = false;
         state.top_level_array_size = 0;
         state.top_level_array_stride = 0;

         /*
          * From ARB_program_interface spec, issue (16):
          *
          * "RESOLVED: We will follow the default rule for enumerating block
          *  members in the OpenGL API, which is:
          *
          *  * If a variable is a member of an interface block without an
          *    instance name, it is enumerated using just the variable name.
          *
          *  * If a variable is a member of an interface block with an
          *    instance name, it is enumerated as "BlockName.Member", where
          *    "BlockName" is the name of the interface block (not the
          *    instance name) and "Member" is the name of the variable.
          *
          * For example, in the following code:
          *
          * uniform Block1 {
          *   int member1;
          * };
          * uniform Block2 {
          *   int member2;
          * } instance2;
          * uniform Block3 {
          *  int member3;
          * } instance3[2];  // uses two separate buffer bindings
          *
          * the three uniforms (if active) are enumerated as "member1",
          * "Block2.member2", and "Block3.member3"."
          *
          * Note that in the last example, with an array of ubo, only one
          * uniform is generated. For that reason, while unrolling the
          * uniforms of a ubo, or the variables of a ssbo, we need to treat
          * arrays of instance as a single block.
          */
         char *name;
         const struct glsl_type *type = var->type;
         if (state.var_is_in_block &&
             ((!prog->data->spirv && glsl_without_array(type) == var->interface_type) ||
              (prog->data->spirv && type == var->interface_type))) {
            type = glsl_without_array(var->type);
            state.current_ifc_type = type;
            name = ralloc_strdup(NULL, glsl_get_type_name(type));
         } else {
            state.set_top_level_array = true;
            name = ralloc_strdup(NULL, var->name);
         }

         struct type_tree_entry *type_tree =
            build_type_tree_for_type(type);
         state.current_type = type_tree;

         int location = var->data.location;

         struct gl_uniform_block *blocks = NULL;
         int num_blocks = 0;
         int buffer_block_index = -1;
         bool is_interface_array = false;
         if (state.var_is_in_block) {
            /* If the uniform is inside a uniform block determine its block index by
             * comparing the bindings, we can not use names.
             */
            blocks = nir_variable_is_in_ssbo(state.current_var) ?
               prog->data->ShaderStorageBlocks : prog->data->UniformBlocks;
            num_blocks = nir_variable_is_in_ssbo(state.current_var) ?
               prog->data->NumShaderStorageBlocks : prog->data->NumUniformBlocks;

            is_interface_array =
               glsl_without_array(state.current_var->type) == state.current_var->interface_type &&
               glsl_type_is_array(state.current_var->type);

            const char *ifc_name =
               glsl_get_type_name(state.current_var->interface_type);

            if (is_interface_array && !prog->data->spirv) {
               unsigned l = strlen(ifc_name);

               /* Even when a match is found, do not "break" here.  As this is
                * an array of instances, all elements of the array need to be
                * marked as referenced.
                */
               for (unsigned i = 0; i < num_blocks; i++) {
                  if (strncmp(ifc_name, blocks[i].name.string, l) == 0 &&
                      blocks[i].name.string[l] == '[') {
                     if (buffer_block_index == -1)
                        buffer_block_index = i;

                     struct hash_entry *entry =
                        _mesa_hash_table_search(state.referenced_uniforms[shader_type],
                                                var->name);
                     if (entry) {
                        struct uniform_array_info *ainfo =
                           (struct uniform_array_info *) entry->data;
                        if (BITSET_TEST(ainfo->indices, blocks[i].linearized_array_index))
                           blocks[i].stageref |= 1U << shader_type;
                     }
                  }
               }
            } else {
               for (unsigned i = 0; i < num_blocks; i++) {
                  bool match = false;
                  if (!prog->data->spirv) {
                     match = strcmp(ifc_name, blocks[i].name.string) == 0;
                  } else {
                     match = var->data.binding == blocks[i].Binding;
                  }
                  if (match) {
                     buffer_block_index = i;

                     if (!prog->data->spirv) {
                        struct hash_entry *entry =
                           _mesa_hash_table_search(state.referenced_uniforms[shader_type],
                                                   var->name);
                        if (entry)
                           blocks[i].stageref |= 1U << shader_type;
                     }

                     break;
                  }
               }
            }
         }

         if (nir_variable_is_in_ssbo(var) &&
             !(var->data.access & ACCESS_NON_WRITEABLE)) {
            unsigned array_size = is_interface_array ?
               glsl_get_length(var->type) : 1;

            STATIC_ASSERT(MAX_SHADER_STORAGE_BUFFERS <= 32);

            /* Buffers from each stage are pointers to the one stored in the program. We need
             * to account for this before computing the mask below otherwise the mask will be
             * incorrect.
             *    sh->Program->sh.SSBlocks: [a][b][c][d][e][f]
             *    VS prog->data->SSBlocks : [a][b][c]
             *    FS prog->data->SSBlocks : [d][e][f]
             * eg for FS buffer 1, buffer_block_index will be 4 but sh_block_index will be 1.
             */
            int base = 0;
            base = sh->Program->sh.ShaderStorageBlocks[0] - prog->data->ShaderStorageBlocks;

            assert(base >= 0);

            int sh_block_index = buffer_block_index - base;
            /* Shaders that use too many SSBOs will fail to compile, which
             * we don't care about.
             *
             * This is true for shaders that do not use too many SSBOs:
             */
            if (sh_block_index + array_size <= 32) {
               state.shader_storage_blocks_write_access |=
                  u_bit_consecutive(sh_block_index, array_size);
            }
         }

         if (blocks && !prog->data->spirv && state.var_is_in_block) {
            if (glsl_without_array(state.current_var->type) != state.current_var->interface_type) {
               /* this is nested at some offset inside the block */
               bool found = false;
               char sentinel = '\0';

               if (glsl_type_is_struct(state.current_var->type)) {
                  sentinel = '.';
               } else if (glsl_type_is_array(state.current_var->type) &&
                          (glsl_type_is_array(glsl_get_array_element(state.current_var->type))
                           || glsl_type_is_struct(glsl_without_array(state.current_var->type)))) {
                 sentinel = '[';
               }

               const unsigned l = strlen(state.current_var->name);
               for (unsigned i = 0; i < num_blocks; i++) {
                  for (unsigned j = 0; j < blocks[i].NumUniforms; j++) {
                    if (sentinel) {
                        const char *begin = blocks[i].Uniforms[j].Name;
                        const char *end = strchr(begin, sentinel);

                        if (end == NULL)
                           continue;

                        if ((ptrdiff_t) l != (end - begin))
                           continue;
                        found = strncmp(state.current_var->name, begin, l) == 0;
                     } else {
                        found = strcmp(state.current_var->name, blocks[i].Uniforms[j].Name) == 0;
                     }

                     if (found) {
                        location = j;

                        struct hash_entry *entry =
                           _mesa_hash_table_search(state.referenced_uniforms[shader_type], var->name);
                        if (entry)
                           blocks[i].stageref |= 1U << shader_type;

                        break;
                     }
                  }

                  if (found)
                     break;
               }
               assert(found);
               var->data.location = location;
            } else {
               /* this is the base block offset */
               var->data.location = buffer_block_index;
               location = 0;
            }
            assert(buffer_block_index >= 0);
            const struct gl_uniform_block *const block =
               &blocks[buffer_block_index];
            assert(location >= 0 && location < block->NumUniforms);

            const struct gl_uniform_buffer_variable *const ubo_var =
               &block->Uniforms[location];

            state.offset = ubo_var->Offset;
         }

         /* Check if the uniform has been processed already for
          * other stage. If so, validate they are compatible and update
          * the active stage mask.
          */
         if (find_and_update_previous_uniform_storage(consts, prog, &state, var,
                                                      name, type, shader_type)) {
            ralloc_free(name);
            free_type_tree(type_tree);
            continue;
         }

         /* From now on the variable’s location will be its uniform index */
         if (!state.var_is_in_block)
            var->data.location = prog->data->NumUniformStorage;
         else
            location = -1;

         bool row_major =
            var->data.matrix_layout == GLSL_MATRIX_LAYOUT_ROW_MAJOR;
         int res = nir_link_uniform(consts, prog, sh->Program, shader_type, type,
                                    0, location,
                                    &state,
                                    !prog->data->spirv ? &name : NULL,
                                    !prog->data->spirv ? strlen(name) : 0,
                                    row_major);

         free_type_tree(type_tree);
         ralloc_free(name);

         if (res == -1)
            return false;
      }

      if (!prog->data->spirv) {
         _mesa_hash_table_destroy(state.referenced_uniforms[shader_type],
                                  NULL);
      }

      if (state.num_shader_samplers >
          consts->Program[shader_type].MaxTextureImageUnits) {
         linker_error(prog, "Too many %s shader texture samplers\n",
                      _mesa_shader_stage_to_string(shader_type));
         continue;
      }

      if (state.num_shader_images >
          consts->Program[shader_type].MaxImageUniforms) {
         linker_error(prog, "Too many %s shader image uniforms (%u > %u)\n",
                      _mesa_shader_stage_to_string(shader_type),
                      state.num_shader_images,
                      consts->Program[shader_type].MaxImageUniforms);
         continue;
      }

      sh->Program->SamplersUsed = state.shader_samplers_used;
      sh->Program->sh.ShaderStorageBlocksWriteAccess =
         state.shader_storage_blocks_write_access;
      sh->shadow_samplers = state.shader_shadow_samplers;
      sh->Program->info.num_textures = state.num_shader_samplers;
      sh->Program->info.num_images = state.num_shader_images;
      sh->num_uniform_components = state.num_shader_uniform_components;
      sh->num_combined_uniform_components = sh->num_uniform_components;
   }

   prog->data->NumHiddenUniforms = state.num_hidden_uniforms;
   prog->data->NumUniformDataSlots = state.num_values;

   assert(prog->data->spirv || prog->data->NumUniformStorage == storage_size);

   if (prog->data->spirv)
      prog->NumUniformRemapTable = state.max_uniform_location;

   nir_setup_uniform_remap_tables(consts, prog);
   gl_nir_set_uniform_initializers(consts, prog);

   _mesa_hash_table_destroy(state.uniform_hash, hash_free_uniform_name);

   return true;
}
