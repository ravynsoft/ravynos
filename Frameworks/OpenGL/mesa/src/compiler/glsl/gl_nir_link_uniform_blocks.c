/*
 * Copyright © 2019 Intel Corporation
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
#include "ir_uniform.h" /* for gl_uniform_storage */
#include "linker_util.h"
#include "main/consts_exts.h"
#include "main/shader_types.h"
#include "util/u_math.h"

/**
 * This file contains code to do a nir-based linking for uniform blocks. This
 * includes ubos and ssbos.
 *
 * For the case of ARB_gl_spirv there are some differences compared with GLSL:
 *
 * 1. Linking doesn't use names: GLSL linking use names as core concept. But
 *    on SPIR-V, uniform block name, fields names, and other names are
 *    considered optional debug infor so could not be present. So the linking
 *    should work without it, and it is optional to not handle them at
 *    all. From ARB_gl_spirv spec.
 *
 *    "19. How should the program interface query operations behave for program
 *         objects created from SPIR-V shaders?
 *
 *     DISCUSSION: we previously said we didn't need reflection to work for
 *     SPIR-V shaders (at least for the first version), however we are left
 *     with specifying how it should "not work". The primary issue is that
 *     SPIR-V binaries are not required to have names associated with
 *     variables. They can be associated in debug information, but there is no
 *     requirement for that to be present, and it should not be relied upon.
 *
 *     Options:
 *
 *     <skip>
 *
 *    C) Allow as much as possible to work "naturally". You can query for the
 *    number of active resources, and for details about them. Anything that
 *    doesn't query by name will work as expected. Queries for maximum length
 *    of names return one. Queries for anything "by name" return INVALID_INDEX
 *    (or -1). Querying the name property of a resource returns an empty
 *    string. This may allow many queries to work, but it's not clear how
 *    useful it would be if you can't actually know which specific variable
 *    you are retrieving information on. If everything is specified a-priori
 *    by location/binding/offset/index/component in the shader, this may be
 *    sufficient.
 *
 *    RESOLVED.  Pick (c), but also allow debug names to be returned if an
 *    implementation wants to."
 *
 * When linking SPIR-V shaders this implemention doesn't care for the names,
 * as the main objective is functional, and not support optional debug
 * features.
 *
 * 2. Terminology: this file handles both UBO and SSBO, including both as
 *    "uniform blocks" analogously to what is done in the GLSL (IR) path.
 *
 *    From ARB_gl_spirv spec:
 *      "Mapping of Storage Classes:
 *       <skip>
 *       uniform blockN { ... } ...;  -> Uniform, with Block decoration
 *       <skip>
 *       buffer  blockN { ... } ...;  -> Uniform, with BufferBlock decoration"
 *
 * 3. Explicit data: for the SPIR-V path the code assumes that all structure
 *    members have an Offset decoration, all arrays have an ArrayStride and
 *    all matrices have a MatrixStride, even for nested structures. That way
 *    we don’t have to worry about the different layout modes. This is
 *    explicitly required in the SPIR-V spec:
 *
 *    "Composite objects in the UniformConstant, Uniform, and PushConstant
 *     Storage Classes must be explicitly laid out. The following apply to all
 *     the aggregate and matrix types describing such an object, recursively
 *     through their nested types:
 *
 *    – Each structure-type member must have an Offset Decoration.
 *    – Each array type must have an ArrayStride Decoration.
 *    – Each structure-type member that is a matrix or array-of-matrices must
 *      have be decorated with a MatrixStride Decoration, and one of the
 *      RowMajor or ColMajor Decorations."
 *
 *    Additionally, the structure members are expected to be presented in
 *    increasing offset order:
 *
 *   "a structure has lower-numbered members appearing at smaller offsets than
 *    higher-numbered members"
 */

enum block_type {
   BLOCK_UBO,
   BLOCK_SSBO
};

struct uniform_block_array_elements {
   unsigned *array_elements;
   unsigned num_array_elements;
   /**
    * Size of the array before array-trimming optimizations.
    *
    * Locations are only assigned to active array elements, but the location
    * values are calculated as if all elements are active. The total number
    * of elements in an array including the elements in arrays of arrays before
    * inactive elements are removed is needed to be perform that calculation.
    */
   unsigned aoa_size;

   struct uniform_block_array_elements *array;
};

struct link_uniform_block_active {
   const struct glsl_type *type;
   nir_variable *var;

   struct uniform_block_array_elements *array;

   unsigned binding;

   bool has_instance_name;
   bool has_binding;
   bool is_shader_storage;
};

/*
 * It is worth to note that ARB_gl_spirv spec doesn't require us to do this
 * validation, but at the same time, it allow us to do it.
 */
static bool
link_blocks_are_compatible(const struct gl_uniform_block *a,
                           const struct gl_uniform_block *b)
{
   /*
    *   "7.4.2. SPIR-V Shader Interface Matching":
    *    "Uniform and shader storage block variables must also be decorated
    *     with a Binding"
    */
   if (a->Binding != b->Binding)
      return false;

   assert((a->name.string == NULL && b->name.string == NULL) ||
          strcmp(a->name.string, b->name.string) == 0);

   if (a->NumUniforms != b->NumUniforms)
      return false;

   if (a->_Packing != b->_Packing)
      return false;

   if (a->_RowMajor != b->_RowMajor)
      return false;

   for (unsigned i = 0; i < a->NumUniforms; i++) {
      if (a->Uniforms[i].Name != NULL && b->Uniforms[i].Name != NULL &&
          strcmp(a->Uniforms[i].Name, b->Uniforms[i].Name) != 0)
         return false;

      if (a->Uniforms[i].Type != b->Uniforms[i].Type)
         return false;

      if (a->Uniforms[i].RowMajor != b->Uniforms[i].RowMajor)
         return false;

      if (a->Uniforms[i].Offset != b->Uniforms[i].Offset)
         return false;
   }

   return true;
}

/**
 * Merges a buffer block into an array of buffer blocks that may or may not
 * already contain a copy of it.
 *
 * Returns the index of the block in the array (new if it was needed, or the
 * index of the copy of it). -1 if there are two incompatible block
 * definitions with the same binding.
 *
 */
static int
link_cross_validate_uniform_block(void *mem_ctx,
                                  struct gl_uniform_block **linked_blocks,
                                  unsigned int *num_linked_blocks,
                                  struct gl_uniform_block *new_block,
                                  bool is_spirv)
{
   /* We first check if new_block was already linked */
   for (unsigned int i = 0; i < *num_linked_blocks; i++) {
      struct gl_uniform_block *old_block = &(*linked_blocks)[i];

      if ((is_spirv && old_block->Binding == new_block->Binding) ||
          (!is_spirv && (strcmp(old_block->name.string, new_block->name.string) == 0)))
         return link_blocks_are_compatible(old_block, new_block) ? i : -1;
   }

   *linked_blocks = reralloc(mem_ctx, *linked_blocks,
                             struct gl_uniform_block,
                             *num_linked_blocks + 1);
   int linked_block_index = (*num_linked_blocks)++;
   struct gl_uniform_block *linked_block = &(*linked_blocks)[linked_block_index];

   memcpy(linked_block, new_block, sizeof(*new_block));
   linked_block->Uniforms = ralloc_array(*linked_blocks,
                                         struct gl_uniform_buffer_variable,
                                         linked_block->NumUniforms);

   memcpy(linked_block->Uniforms,
          new_block->Uniforms,
          sizeof(*linked_block->Uniforms) * linked_block->NumUniforms);

   /* If we mem copied a pointer to a string above we need to create our own
    * copy of the string.
    */
   if (linked_block->name.string) {
      linked_block->name.string =
         ralloc_strdup(*linked_blocks, linked_block->name.string);
      resource_name_updated(&linked_block->name);

      for (unsigned int i = 0; i < linked_block->NumUniforms; i++) {
        struct gl_uniform_buffer_variable *ubo_var =
           &linked_block->Uniforms[i];

         if (ubo_var->Name == ubo_var->IndexName) {
            ubo_var->Name = ralloc_strdup(*linked_blocks, ubo_var->Name);
            ubo_var->IndexName = ubo_var->Name;
         } else {
            ubo_var->Name = ralloc_strdup(*linked_blocks, ubo_var->Name);
            ubo_var->IndexName =
               ralloc_strdup(*linked_blocks, ubo_var->IndexName);
         }
      }
   }

   return linked_block_index;
}


/**
 * Accumulates the array of buffer blocks and checks that all definitions of
 * blocks agree on their contents.
 */
static bool
nir_interstage_cross_validate_uniform_blocks(struct gl_shader_program *prog,
                                             enum block_type block_type)
{
   int *interfaceBlockStageIndex[MESA_SHADER_STAGES];
   struct gl_uniform_block *blks = NULL;
   unsigned *num_blks = block_type == BLOCK_SSBO ? &prog->data->NumShaderStorageBlocks :
      &prog->data->NumUniformBlocks;

   unsigned max_num_buffer_blocks = 0;
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (prog->_LinkedShaders[i]) {
         if (block_type == BLOCK_SSBO) {
            max_num_buffer_blocks +=
               prog->_LinkedShaders[i]->Program->info.num_ssbos;
         } else {
            max_num_buffer_blocks +=
               prog->_LinkedShaders[i]->Program->info.num_ubos;
         }
      }
   }

   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      struct gl_linked_shader *sh = prog->_LinkedShaders[i];

      interfaceBlockStageIndex[i] = malloc(max_num_buffer_blocks * sizeof(int));
      for (unsigned int j = 0; j < max_num_buffer_blocks; j++)
         interfaceBlockStageIndex[i][j] = -1;

      if (sh == NULL)
         continue;

      unsigned sh_num_blocks;
      struct gl_uniform_block **sh_blks;
      if (block_type == BLOCK_SSBO) {
         sh_num_blocks = prog->_LinkedShaders[i]->Program->info.num_ssbos;
         sh_blks = sh->Program->sh.ShaderStorageBlocks;
      } else {
         sh_num_blocks = prog->_LinkedShaders[i]->Program->info.num_ubos;
         sh_blks = sh->Program->sh.UniformBlocks;
      }

      for (unsigned int j = 0; j < sh_num_blocks; j++) {
         int index = link_cross_validate_uniform_block(prog->data, &blks,
                                                       num_blks, sh_blks[j],
                                                       !!prog->data->spirv);

         if (index == -1) {
            /* We use the binding as we are ignoring the names */
            linker_error(prog, "buffer block with binding `%i' has mismatching "
                         "definitions\n", sh_blks[j]->Binding);

            for (unsigned k = 0; k <= i; k++) {
               free(interfaceBlockStageIndex[k]);
            }

            /* Reset the block count. This will help avoid various segfaults
             * from api calls that assume the array exists due to the count
             * being non-zero.
             */
            *num_blks = 0;
            return false;
         }

         interfaceBlockStageIndex[i][index] = j;
      }
   }

   /* Update per stage block pointers to point to the program list.
    */
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      for (unsigned j = 0; j < *num_blks; j++) {
         int stage_index = interfaceBlockStageIndex[i][j];

         if (stage_index != -1) {
            struct gl_linked_shader *sh = prog->_LinkedShaders[i];

            struct gl_uniform_block **sh_blks = block_type == BLOCK_SSBO ?
               sh->Program->sh.ShaderStorageBlocks :
               sh->Program->sh.UniformBlocks;

            blks[j].stageref |= sh_blks[stage_index]->stageref;
            sh_blks[stage_index] = &blks[j];
         }
      }
   }

   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      free(interfaceBlockStageIndex[i]);
   }

   if (block_type == BLOCK_SSBO)
      prog->data->ShaderStorageBlocks = blks;
   else {
      prog->data->NumUniformBlocks = *num_blks;
      prog->data->UniformBlocks = blks;
   }

   return true;
}

/*
 * Iterates @type in order to compute how many individual leaf variables
 * contains.
 */
static void
iterate_type_count_variables(const struct glsl_type *type,
                             unsigned int *num_variables)
{
   unsigned length = glsl_get_length(type);
   if (glsl_type_is_unsized_array(type))
      length = 1;

   for (unsigned i = 0; i < length; i++) {
      const struct glsl_type *field_type;

      if (glsl_type_is_struct_or_ifc(type))
         field_type = glsl_get_struct_field(type, i);
      else
         field_type = glsl_get_array_element(type);

      if (glsl_type_is_leaf(field_type))
         (*num_variables)++;
      else
         iterate_type_count_variables(field_type, num_variables);
   }
}

static void
fill_individual_variable(void *mem_ctx, const char *name,
                         const struct glsl_type *type,
                         struct gl_uniform_buffer_variable *variables,
                         unsigned int *variable_index,
                         unsigned int *offset,
                         unsigned *buffer_size,
                         struct gl_shader_program *prog,
                         struct gl_uniform_block *block,
                         const enum glsl_interface_packing packing,
                         bool is_array_instance,
                         bool last_field)
{
   struct gl_uniform_buffer_variable *v = &variables[*variable_index];
   v->Type = type;

   const struct glsl_type *t_without_array = glsl_without_array(type);
   if (glsl_type_is_matrix(glsl_without_array(t_without_array))) {
      v->RowMajor = glsl_matrix_type_is_row_major(t_without_array);
   } else {
      /* default value, better that potential meaningless garbage */
      v->RowMajor = false;
   }

   if (!prog->data->spirv) {
      v->Name = ralloc_strdup(mem_ctx, name);

      if (is_array_instance) {
         v->IndexName = ralloc_strdup(mem_ctx, name);

         char *open_bracket = strchr(v->IndexName, '[');
         assert(open_bracket != NULL);

         char *close_bracket = strchr(open_bracket, '.') - 1;
         assert(close_bracket != NULL);

         /* Length of the tail without the ']' but with the NUL. */
         unsigned len = strlen(close_bracket + 1) + 1;

         memmove(open_bracket, close_bracket + 1, len);
      } else {
         v->IndexName = v->Name;
      }

      unsigned alignment = 0;
      unsigned size = 0;

      /* The ARB_program_interface_query spec says:
       *
       *    If the final member of an active shader storage block is array
       *    with no declared size, the minimum buffer size is computed
       *    assuming the array was declared as an array with one element.
       *
       * For that reason, we use the base type of the unsized array to
       * calculate its size. We don't need to check if the unsized array is
       * the last member of a shader storage block (that check was already
       * done by the parser).
       */
      const struct glsl_type *type_for_size = type;
      if (glsl_type_is_unsized_array(type)) {
         if (!last_field) {
            linker_error(prog, "unsized array `%s' definition: "
                         "only last member of a shader storage block "
                         "can be defined as unsized array",
                         name);
         }

         type_for_size = glsl_get_array_element(type);
      }

      if (packing == GLSL_INTERFACE_PACKING_STD430) {
         alignment = glsl_get_std430_base_alignment(type, v->RowMajor);
         size = glsl_get_std430_size(type_for_size, v->RowMajor);
      } else {
         alignment = glsl_get_std140_base_alignment(type, v->RowMajor);
         size = glsl_get_std140_size(type_for_size, v->RowMajor);
      }

      *offset = align(*offset, alignment);
      v->Offset = *offset;

      *offset += size;

      /* The ARB_uniform_buffer_object spec says:
       *
       *    For uniform blocks laid out according to [std140] rules, the
       *    minimum buffer object size returned by the UNIFORM_BLOCK_DATA_SIZE
       *    query is derived by taking the offset of the last basic machine
       *    unit consumed by the last uniform of the uniform block (including
       *    any end-of-array or end-of-structure padding), adding one, and
       *    rounding up to the next multiple of the base alignment required
       *    for a vec4.
       */
      *buffer_size = align(*offset, 16);
   } else {
      /**
       * Although ARB_gl_spirv points that the offsets need to be included
       * (see "Mappings of layouts"), in the end those are only valid for
       * root-variables, and we would need to recompute offsets when we iterate
       * over non-trivial types, like aoa. So we compute the offset always.
       */
      v->Offset = *offset;
      (*offset) += glsl_get_explicit_size(type, true);
   }

   (*variable_index)++;
}

static void
enter_or_leave_record(struct gl_uniform_block *block, unsigned *offset,
                      const struct gl_constants *consts,
                      const struct glsl_type *type,
                      bool row_major,
                      enum glsl_interface_packing internal_packing)
{
   assert(glsl_type_is_struct(type));

   if (internal_packing == GLSL_INTERFACE_PACKING_STD430) {
      *offset = align(
         *offset, glsl_get_std430_base_alignment(type, row_major));
   } else
      *offset = align(
         *offset, glsl_get_std140_base_alignment(type, row_major));
}

static void
iterate_type_fill_variables(void *mem_ctx, char **name,
                            size_t name_length,
                            const struct gl_constants *consts,
                            const struct glsl_type *type,
                            struct gl_uniform_buffer_variable *variables,
                            unsigned int *variable_index,
                            unsigned int *offset,
                            unsigned *buffer_size,
                            struct gl_shader_program *prog,
                            struct gl_uniform_block *block,
                            const struct glsl_type *blk_type,
                            bool is_array_instance, bool row_major,
                            enum glsl_interface_packing internal_packing)
{
   unsigned struct_base_offset;

   bool struct_or_ifc = glsl_type_is_struct_or_ifc(type);
   if (struct_or_ifc)
      struct_base_offset = *offset;

   /* Handle shader storage block unsized arrays */
   unsigned length = glsl_get_length(type);
      if (glsl_type_is_unsized_array(type))
         length = 1;

   if (glsl_type_is_struct(type) && !prog->data->spirv)
      enter_or_leave_record(block, offset, consts, type, row_major,
                            internal_packing);

   bool has_block_name = *name ? strcmp(*name, "") : false;
   for (unsigned i = 0; i < length; i++) {
      const struct glsl_type *field_type;
      size_t new_length = name_length;
      bool field_row_major = row_major;

      if (struct_or_ifc) {
         field_type = glsl_get_struct_field(type, i);

         if (prog->data->spirv) {
            *offset = struct_base_offset + glsl_get_struct_field_offset(type, i);

         } else if (glsl_get_struct_field_offset(type, i) != -1 &&
                    type == glsl_without_array(blk_type)) {
            *offset = glsl_get_struct_field_offset(type, i);
         }

         /* Append '.field' to the current variable name. */
         if (*name) {
            ralloc_asprintf_rewrite_tail(name, &new_length,
                                         has_block_name ? ".%s" : "%s",
                                         glsl_get_struct_elem_name(type, i));
         }

         /* The layout of structures at the top level of the block is set
          * during parsing.  For matrices contained in multiple levels of
          * structures in the block, the inner structures have no layout.
          * These cases inherit the layout from the outer levels.
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
         if (*name) {
            ralloc_asprintf_rewrite_tail(name, &new_length, "[%u]", i);
         }
      }

      if (glsl_type_is_leaf(field_type)) {
         fill_individual_variable(mem_ctx, *name, field_type, variables,
                                  variable_index, offset, buffer_size, prog,
                                  block, internal_packing, is_array_instance,
                                  (i + 1) == glsl_get_length(type));
      } else {
         iterate_type_fill_variables(mem_ctx, name, new_length, consts, field_type, variables,
                                     variable_index, offset, buffer_size,
                                     prog, block, blk_type, is_array_instance,
                                     field_row_major, internal_packing);
      }
   }

   if (glsl_type_is_struct(type) && !prog->data->spirv)
      enter_or_leave_record(block, offset, consts, type, row_major,
                            internal_packing);
}

static struct link_uniform_block_active *
process_block(void *mem_ctx, struct hash_table *ht, nir_variable *var)
{
   const struct hash_entry *existing_block =
      _mesa_hash_table_search(ht, glsl_get_type_name(var->interface_type));

   bool is_interface_instance =
      glsl_without_array(var->type) == var->interface_type;
   const struct glsl_type *block_type = is_interface_instance ?
         var->type : var->interface_type;

   /* If a block with this block-name has not previously been seen, add it.
    * If a block with this block-name has been seen, it must be identical to
    * the block currently being examined.
    */
   if (existing_block == NULL) {
      struct link_uniform_block_active *b =
         rzalloc(mem_ctx, struct link_uniform_block_active);

      b->var = var;
      b->type = block_type;
      b->has_instance_name = is_interface_instance;
      b->is_shader_storage = var->data.mode == nir_var_mem_ssbo;

      if (var->data.explicit_binding) {
         b->has_binding = true;
         b->binding = var->data.binding;
      } else {
         b->has_binding = false;
         b->binding = 0;
      }

      _mesa_hash_table_insert(ht, glsl_get_type_name(var->interface_type),
                              (void *) b);
      return b;
   } else {
      struct link_uniform_block_active *b =
         (struct link_uniform_block_active *) existing_block->data;

      if (b->type != block_type ||
          b->has_instance_name != is_interface_instance)
         return NULL;
      else
         return b;
   }

   assert(!"Should not get here.");
   return NULL;
}

/* For arrays of arrays this function will give us a middle ground between
 * detecting inactive uniform blocks and structuring them in a way that makes
 * it easy to calculate the offset for indirect indexing.
 *
 * For example given the shader:
 *
 *   uniform ArraysOfArraysBlock
 *   {
 *      vec4 a;
 *   } i[3][4][5];
 *
 *   void main()
 *   {
 *      vec4 b = i[0][1][1].a;
 *      gl_Position = i[2][2][3].a + b;
 *   }
 *
 * There are only 2 active blocks above but for the sake of indirect indexing
 * and not over complicating the code we will end up with a count of 8.  Here
 * each dimension has 2 different indices counted so we end up with 2*2*2
 */
static void
process_arrays(void *mem_ctx, nir_deref_instr *deref,
               struct link_uniform_block_active *block)
{
   if (!glsl_type_is_array(block->type))
      return;

   nir_deref_path path;
   nir_deref_path_init(&path, deref, NULL);

   assert(path.path[0]->deref_type == nir_deref_type_var);
   nir_deref_instr **p = &path.path[1];
   assert((*p)->deref_type == nir_deref_type_array);

   const struct glsl_type *type = block->type;
   struct uniform_block_array_elements **ub_array_ptr = &block->array;
   for (; *p; p++) {
      if ((*p)->deref_type == nir_deref_type_array) {
         if (*ub_array_ptr == NULL) {
            *ub_array_ptr = rzalloc(mem_ctx,
                                    struct uniform_block_array_elements);
            (*ub_array_ptr)->aoa_size = glsl_get_aoa_size(type);
         }

         struct uniform_block_array_elements *ub_array = *ub_array_ptr;
         if (nir_src_is_const((*p)->arr.index)) {
            /* Index is a constant, so mark just that element used, if not
             * already.
             */
            const unsigned idx = nir_src_as_uint((*p)->arr.index);

            unsigned i;
            for (i = 0; i < ub_array->num_array_elements; i++) {
               if (ub_array->array_elements[i] == idx)
                  break;
            }

            if (i == ub_array->num_array_elements) {
               ub_array->array_elements = reralloc(mem_ctx,
                                                   ub_array->array_elements,
                                                   unsigned,
                                                   ub_array->num_array_elements + 1);

               ub_array->array_elements[ub_array->num_array_elements] = idx;
               ub_array->num_array_elements++;
            }
         } else {
            /* The array index is not a constant, so mark the entire array used.
             */
            assert(glsl_type_is_array((*p)->type));
            if (ub_array->num_array_elements < glsl_get_length(type)) {
               ub_array->num_array_elements = glsl_get_length(type);
               ub_array->array_elements = reralloc(mem_ctx,
                                                   ub_array->array_elements,
                                                   unsigned,
                                                   ub_array->num_array_elements);

               for (unsigned i = 0; i < ub_array->num_array_elements; i++) {
                  ub_array->array_elements[i] = i;
               }
            }
         }
         ub_array_ptr = &ub_array->array;
         type = glsl_get_array_element(type);
      } else {
         /* We found the block so break out of loop */
         assert((*p)->deref_type == nir_deref_type_struct);
         break;
      }
   }

   nir_deref_path_finish(&path);
}

/* This function resizes the array types of the block so that later we can use
 * this new size to correctly calculate the offest for indirect indexing.
 */
static const struct glsl_type *
resize_block_array(const struct glsl_type *type,
                   struct uniform_block_array_elements *ub_array)
{
   if (glsl_type_is_array(type)) {
      struct uniform_block_array_elements *child_array =
         glsl_type_is_array(glsl_get_array_element(type)) ? ub_array->array : NULL;

      const struct glsl_type *new_child_type =
         resize_block_array(glsl_get_array_element(type), child_array);
      const struct glsl_type *new_type =
         glsl_array_type(new_child_type, ub_array->num_array_elements, 0);

      return new_type;
   } else {
      assert(glsl_type_is_struct_or_ifc(type));
      return type;
   }
}

static void
count_block(const struct glsl_type *blk_type, unsigned *num_blocks,
            unsigned *num_variables)
{
   const struct glsl_type *type = glsl_without_array(blk_type);
   unsigned aoa_size = glsl_get_aoa_size(blk_type);
   unsigned buffer_count = aoa_size == 0 ? 1 : aoa_size;

   *num_blocks += buffer_count;

   unsigned int block_variables = 0;
   iterate_type_count_variables(type, &block_variables);

   *num_variables += block_variables * buffer_count;
}

static bool
gather_packed_block_info(void *mem_ctx, struct gl_shader_program *prog,
                         struct hash_table *block_hash,
                         nir_deref_instr *deref, enum block_type block_type)
{

   nir_variable_mode mask = nir_var_mem_ubo | nir_var_mem_ssbo;

   if (!nir_deref_mode_is_one_of(deref, mask))
      return true;

   nir_variable *var = nir_deref_instr_get_variable(deref);

   if (block_type == BLOCK_UBO && !nir_variable_is_in_ubo(var))
      return true;

   if (block_type == BLOCK_SSBO && !nir_variable_is_in_ssbo(var))
      return true;

   /* Process the block.  Bail if there was an error. */
   struct link_uniform_block_active *b =
      process_block(mem_ctx, block_hash, var);
   if (b == NULL) {
      linker_error(prog,
                   "uniform block `%s' has mismatching definitions",
                   glsl_without_array(var->type) == var->interface_type ?
                      glsl_get_type_name(var->type) :
                      glsl_get_type_name(var->interface_type));
      return false;
   }

   assert(b->type != NULL);

   /* If the block was declared with a shared or std140 layout
    * qualifier, all its instances have been already marked as used.
    */
   if (glsl_get_ifc_packing(glsl_without_array(b->type)) ==
       GLSL_INTERFACE_PACKING_PACKED) {
      process_arrays(mem_ctx, deref, b);
   }

   return true;
}

static bool
gather_packed_blocks_info(void *mem_ctx, struct gl_shader_program *prog,
                          nir_shader *shader, struct hash_table *block_hash,
                          enum block_type block_type)
{
   bool success = true;
   nir_foreach_function_impl(impl, shader) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic != nir_intrinsic_copy_deref &&
                intr->intrinsic != nir_intrinsic_load_deref &&
                intr->intrinsic != nir_intrinsic_store_deref &&
                intr->intrinsic != nir_intrinsic_deref_buffer_array_length)
               continue;

            nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
            success |=
               gather_packed_block_info(mem_ctx, prog, block_hash, deref,
                                        block_type);

            if (intr->intrinsic == nir_intrinsic_copy_deref) {
               deref = nir_src_as_deref(intr->src[1]);
               success |=
                  gather_packed_block_info(mem_ctx, prog, block_hash, deref,
                                           block_type);
            }
         }
      }
   }

   return success;
}

static void
allocate_uniform_blocks(void *mem_ctx, struct hash_table *block_hash,
                        struct gl_shader_program *prog,
                        struct gl_linked_shader *shader,
                        struct gl_uniform_block **out_blks, unsigned *num_blocks,
                        struct gl_uniform_buffer_variable **out_variables,
                        unsigned *num_variables,
                        enum block_type block_type)
{
   *num_variables = 0;
   *num_blocks = 0;

   /* Section 2.11.6 (Uniform Variables) of the OpenGL ES 3.0.3 spec says:
    *
    *     "All members of a named uniform block declared with a shared or
    *     std140 layout qualifier are considered active, even if they are not
    *     referenced in any shader in the program. The uniform block itself is
    *     also considered active, even if no member of the block is
    *     referenced."
    *
    * So for blocks not defined as packed we simply iterate over the type to
    * establish a count of active blocks.
    */
   nir_foreach_variable_in_shader(var, shader->Program->nir) {
      if (block_type == BLOCK_UBO && !nir_variable_is_in_ubo(var))
         continue;

      if (block_type == BLOCK_SSBO && !nir_variable_is_in_ssbo(var))
         continue;

      if (prog->data->spirv) {
         count_block(var->type, num_blocks, num_variables);
      } else {
         /* Process the block.  Bail if there was an error. */
         struct link_uniform_block_active *b =
            process_block(mem_ctx, block_hash, var);
         if (b == NULL) {
            linker_error(prog, "uniform block `%s' has mismatching definitions",
                         glsl_get_type_name(var->interface_type));
            return;
         }

         assert(b->array == NULL);
         assert(b->type != NULL);
         assert(!glsl_type_is_array(b->type) || b->has_instance_name);

         /* For uniform block arrays declared with a shared or std140 layout
          * qualifier, mark all its instances as used.
          */
         if (glsl_get_ifc_packing(glsl_without_array(b->type)) ==
             GLSL_INTERFACE_PACKING_PACKED)
            continue;

         const struct glsl_type *type = b->type;
         struct uniform_block_array_elements **ub_array = &b->array;
         while (glsl_type_is_array(type)) {
            assert(glsl_get_length(b->type) > 0);

            *ub_array = rzalloc(mem_ctx, struct uniform_block_array_elements);
            (*ub_array)->num_array_elements = glsl_get_length(type);
            (*ub_array)->array_elements = reralloc(mem_ctx,
                                                   (*ub_array)->array_elements,
                                                   unsigned,
                                                   (*ub_array)->num_array_elements);
            (*ub_array)->aoa_size = glsl_get_aoa_size(type);

            for (unsigned i = 0; i < (*ub_array)->num_array_elements; i++) {
               (*ub_array)->array_elements[i] = i;
            }
            ub_array = &(*ub_array)->array;
            type = glsl_get_array_element(type);
         }
      }
   }

   if (!prog->data->spirv) {
      /* Gather packed ubo information by looping over derefs */
      if (!gather_packed_blocks_info(mem_ctx, prog, shader->Program->nir,
                                     block_hash, block_type))
         return;

      /* Count the number of active uniform blocks.  Count the total number of
       * active slots in those uniform blocks.
       */
      hash_table_foreach(block_hash, entry) {
         struct link_uniform_block_active *const b =
            (struct link_uniform_block_active *) entry->data;

         assert((b->array != NULL) == glsl_type_is_array(b->type));

         if (b->array != NULL &&
             (glsl_get_ifc_packing(glsl_without_array(b->type)) ==
              GLSL_INTERFACE_PACKING_PACKED)) {
            b->type = resize_block_array(b->type, b->array);
            b->var->type = b->type;
         }

         count_block(b->type, num_blocks, num_variables);
      }
   }

   if (*num_blocks == 0) {
      assert(*num_variables == 0);
      return;
   }

   nir_fixup_deref_types(shader->Program->nir);

   assert(*num_variables != 0);

   struct gl_uniform_block *blocks =
      rzalloc_array(mem_ctx, struct gl_uniform_block, *num_blocks);

   struct gl_uniform_buffer_variable *variables =
      rzalloc_array(blocks, struct gl_uniform_buffer_variable, *num_variables);

   *out_blks = blocks;
   *out_variables = variables;
}

static void
fill_block(void *mem_ctx, const struct gl_constants *consts, const char *name,
           struct gl_uniform_block *blocks, unsigned *block_index,
           nir_variable *var,
           struct gl_uniform_buffer_variable *variables,
           unsigned *variable_index,
           unsigned binding_offset,
           unsigned linearized_index,
           struct gl_shader_program *prog,
           const gl_shader_stage stage,
           enum block_type block_type)
{
   struct gl_uniform_block *block = &blocks[*block_index];

   bool is_spirv = prog->data->spirv;

   bool is_interface_instance =
      glsl_without_array(var->type) == var->interface_type;
   const struct glsl_type *blk_type = is_interface_instance ?
         var->type : var->interface_type;
   const struct glsl_type *type = glsl_without_array(blk_type);

   block->name.string = is_spirv ? NULL : ralloc_strdup(blocks, name);
   resource_name_updated(&block->name);

   /* From ARB_gl_spirv spec:
    *    "Vulkan uses only one binding point for a resource array,
    *     while OpenGL still uses multiple binding points, so binding
    *     numbers are counted differently for SPIR-V used in Vulkan
    *     and OpenGL
    */
   block->Binding =
      var->data.explicit_binding ? var->data.binding + binding_offset : 0;

   block->Uniforms = &variables[*variable_index];

   /* FIXME: This sets stageref when a block is declared in a spirv shader
    * even when it is not referenced.
    */
   if (is_spirv)
      block->stageref = 1U << stage;

   block->_Packing = glsl_get_ifc_packing(type);
   block->_RowMajor = glsl_matrix_type_is_row_major(type);

   block->linearized_array_index = linearized_index;

   const char *ifc_name = is_interface_instance ? block->name.string : "";
   char *ifc_name_dup = NULL;
   size_t ifc_name_length = 0;
   if (!is_spirv) {
      ifc_name_dup = ralloc_strdup(NULL, ifc_name);
      ifc_name_length = strlen(ifc_name_dup);
   }

   unsigned old_variable_index = *variable_index;
   unsigned offset = 0;
   unsigned buffer_size = 0;
   bool is_array_instance =
      is_interface_instance && glsl_type_is_array(var->type);
   enum glsl_interface_packing packing =
      glsl_get_internal_ifc_packing(type, consts->UseSTD430AsDefaultPacking);

   iterate_type_fill_variables(mem_ctx, &ifc_name_dup, ifc_name_length, consts, type, variables, variable_index,
                               &offset, &buffer_size, prog, block, blk_type, is_array_instance, block->_RowMajor,
                               packing);
   ralloc_free(ifc_name_dup);
   block->NumUniforms = *variable_index - old_variable_index;

   if (is_spirv) {
      block->UniformBufferSize =  glsl_get_explicit_size(type, false);

      /* From OpenGL 4.6 spec, section 7.6.2.3, "SPIR-V Uniform Offsets and
       * strides"
       *
       *   "If the variable is decorated as a BufferBlock , its offsets and
       *    strides must not contradict std430 alignment and minimum offset
       *    requirements. Otherwise, its offsets and strides must not contradict
       *    std140 alignment and minimum offset requirements."
       *
       * So although we are computing the size based on the offsets and
       * array/matrix strides, at the end we need to ensure that the alignment is
       * the same that with std140. From ARB_uniform_buffer_object spec:
       *
       *   "For uniform blocks laid out according to [std140] rules, the minimum
       *    buffer object size returned by the UNIFORM_BLOCK_DATA_SIZE query is
       *    derived by taking the offset of the last basic machine unit consumed
       *    by the last uniform of the uniform block (including any end-of-array
       *    or end-of-structure padding), adding one, and rounding up to the next
       *    multiple of the base alignment required for a vec4."
       */
      block->UniformBufferSize = align(block->UniformBufferSize, 16);
   } else {
      block->UniformBufferSize = buffer_size;
   }

   /* Check SSBO size is lower than maximum supported size for SSBO */
   if (block_type == BLOCK_SSBO &&
       buffer_size > consts->MaxShaderStorageBlockSize) {
      linker_error(prog, "shader storage block `%s' has size %d, "
                   "which is larger than the maximum allowed (%d)",
                   type == var->interface_type ?
                      glsl_get_type_name(var->type) :
                      glsl_get_type_name(var->interface_type),
                   buffer_size,
                   consts->MaxShaderStorageBlockSize);
   }

   *block_index += 1;
}

static void
fill_block_array(struct uniform_block_array_elements *ub_array,
                 const struct gl_constants *consts, char **name,
                 size_t name_length, struct gl_uniform_block *blks,
                 nir_variable *var,
                 struct gl_uniform_buffer_variable *variables,
                 unsigned *variable_index, unsigned binding_offset,
                 struct gl_shader_program *prog,
                 const gl_shader_stage stage, enum block_type block_type,
                 unsigned *block_index, unsigned first_index)
{
   for (unsigned j = 0; j < ub_array->num_array_elements; j++) {
      size_t new_length = name_length;

      unsigned int element_idx = ub_array->array_elements[j];
      /* Append the subscript to the current variable name */
      ralloc_asprintf_rewrite_tail(name, &new_length, "[%u]", element_idx);

      if (ub_array->array) {
         unsigned binding_stride = binding_offset +
            (element_idx * ub_array->array->aoa_size);
         fill_block_array(ub_array->array, consts, name, new_length, blks, var, variables,
                          variable_index, binding_stride, prog, stage, block_type, block_index, first_index);
      } else {
         fill_block(blks, consts, *name,
                    blks, block_index, var, variables,
                    variable_index, binding_offset + element_idx, *block_index - first_index, prog, stage,
                    block_type);
      }
   }
}

/*
 * Link ubos/ssbos for a given linked_shader/stage.
 */
static void
link_linked_shader_uniform_blocks(void *mem_ctx,
                                  const struct gl_constants *consts,
                                  struct gl_shader_program *prog,
                                  struct gl_linked_shader *shader,
                                  struct gl_uniform_block **blocks,
                                  unsigned *num_blocks,
                                  enum block_type block_type)
{
   struct gl_uniform_buffer_variable *variables = NULL;
   unsigned num_variables = 0;

   /* This hash table will track all of the uniform blocks that have been
    * encountered.  Since blocks with the same block-name must be the same,
    * the hash is organized by block-name.
    */
   struct hash_table *block_hash =
      _mesa_hash_table_create(mem_ctx, _mesa_hash_string,
                              _mesa_key_string_equal);

   allocate_uniform_blocks(mem_ctx, block_hash, prog, shader,
                           blocks, num_blocks,
                           &variables, &num_variables,
                           block_type);
   if (!prog->data->LinkStatus)
      return;

   /* Fill the content of uniforms and variables */
   unsigned block_index = 0;
   unsigned variable_index = 0;
   struct gl_uniform_block *blks = *blocks;


   if (!prog->data->spirv) {
      hash_table_foreach(block_hash, entry) {
         struct link_uniform_block_active *const b =
            (struct link_uniform_block_active *) entry->data;

         const struct glsl_type *blk_type =
            glsl_without_array(b->var->type) == b->var->interface_type ?
               b->var->type : b->var->interface_type;

         if (glsl_type_is_array(blk_type)) {
             char *name =
               ralloc_strdup(NULL,
                             glsl_get_type_name(glsl_without_array(blk_type)));
            size_t name_length = strlen(name);

            assert(b->has_instance_name);
            fill_block_array(b->array, consts, &name, name_length,
                             blks, b->var, variables, &variable_index, 0,
                             prog, shader->Stage, block_type, &block_index, block_index);
            ralloc_free(name);
         } else {
            fill_block(blks, consts, glsl_get_type_name(blk_type), blks, &block_index, b->var,
                       variables, &variable_index, 0, 0, prog, shader->Stage,
                       block_type);
         }
      }
   } else {
      nir_foreach_variable_in_shader(var, shader->Program->nir) {
         if (block_type == BLOCK_UBO && !nir_variable_is_in_ubo(var))
            continue;

         if (block_type == BLOCK_SSBO && !nir_variable_is_in_ssbo(var))
            continue;

         unsigned aoa_size = glsl_get_aoa_size(var->type);
         unsigned buffer_count = aoa_size == 0 ? 1 : aoa_size;
         for (unsigned array_index = 0; array_index < buffer_count; array_index++) {
            fill_block(NULL, consts, NULL, blks, &block_index, var, variables,
                       &variable_index, array_index, array_index, prog, shader->Stage,
                       block_type);
         }
      }
   }

   assert(block_index == *num_blocks);
   assert(variable_index == num_variables);
}

bool
gl_nir_link_uniform_blocks(const struct gl_constants *consts,
                           struct gl_shader_program *prog)
{
   void *mem_ctx = ralloc_context(NULL);
   bool ret = false;
   for (int stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      struct gl_linked_shader *const linked = prog->_LinkedShaders[stage];
      struct gl_uniform_block *ubo_blocks = NULL;
      unsigned num_ubo_blocks = 0;
      struct gl_uniform_block *ssbo_blocks = NULL;
      unsigned num_ssbo_blocks = 0;

      if (!linked)
         continue;

      link_linked_shader_uniform_blocks(mem_ctx, consts, prog, linked,
                                        &ubo_blocks, &num_ubo_blocks,
                                        BLOCK_UBO);

      link_linked_shader_uniform_blocks(mem_ctx, consts, prog, linked,
                                        &ssbo_blocks, &num_ssbo_blocks,
                                        BLOCK_SSBO);

      const unsigned max_uniform_blocks =
         consts->Program[linked->Stage].MaxUniformBlocks;
      if (num_ubo_blocks > max_uniform_blocks) {
         linker_error(prog, "Too many %s uniform blocks (%d/%d)\n",
                      _mesa_shader_stage_to_string(linked->Stage),
                      num_ubo_blocks, max_uniform_blocks);
      }

      const unsigned max_shader_storage_blocks =
         consts->Program[linked->Stage].MaxShaderStorageBlocks;
      if (num_ssbo_blocks > max_shader_storage_blocks) {
         linker_error(prog, "Too many %s shader storage blocks (%d/%d)\n",
                      _mesa_shader_stage_to_string(linked->Stage),
                      num_ssbo_blocks, max_shader_storage_blocks);
      }

      if (!prog->data->LinkStatus) {
         goto out;
      }

      prog->data->linked_stages |= 1 << stage;

      /* Copy ubo blocks to linked shader list */
      linked->Program->sh.UniformBlocks =
         ralloc_array(linked, struct gl_uniform_block *, num_ubo_blocks);
      ralloc_steal(linked, ubo_blocks);
      linked->Program->sh.NumUniformBlocks = num_ubo_blocks;
      for (unsigned i = 0; i < num_ubo_blocks; i++) {
         linked->Program->sh.UniformBlocks[i] = &ubo_blocks[i];
      }

      /* We need to set it twice to avoid the value being overwritten by the
       * one from nir in brw_shader_gather_info. TODO: get a way to set the
       * info once, and being able to gather properly the info.
       */
      linked->Program->nir->info.num_ubos = num_ubo_blocks;
      linked->Program->info.num_ubos = num_ubo_blocks;

      /* Copy ssbo blocks to linked shader list */
      linked->Program->sh.ShaderStorageBlocks =
         ralloc_array(linked, struct gl_uniform_block *, num_ssbo_blocks);
      ralloc_steal(linked, ssbo_blocks);
      for (unsigned i = 0; i < num_ssbo_blocks; i++) {
         linked->Program->sh.ShaderStorageBlocks[i] = &ssbo_blocks[i];
      }

      /* See previous comment on num_ubo_blocks */
      linked->Program->nir->info.num_ssbos = num_ssbo_blocks;
      linked->Program->info.num_ssbos = num_ssbo_blocks;
   }

   if (!nir_interstage_cross_validate_uniform_blocks(prog, BLOCK_UBO))
      goto out;

   if (!nir_interstage_cross_validate_uniform_blocks(prog, BLOCK_SSBO))
      goto out;

   ret = true;
out:
   ralloc_free(mem_ctx);
   return ret;
}
