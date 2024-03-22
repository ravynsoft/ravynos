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
#include "gl_nir_linker.h"
#include "compiler/glsl/ir_uniform.h" /* for gl_uniform_storage */
#include "main/shader_types.h"
#include "main/consts_exts.h"

struct set_opaque_binding_closure {
   struct gl_shader_program *shader_prog;
   struct gl_program *prog;
   const nir_variable *var;
   int binding;
   int location;
};

static void
set_opaque_binding(struct set_opaque_binding_closure *data,
                   const struct glsl_type *type)
{
   if (glsl_type_is_array(type) &&
       glsl_type_is_array(glsl_get_array_element(type))) {
      const struct glsl_type *element_type = glsl_get_array_element(type);

      for (unsigned int i = 0; i < glsl_get_length(type); i++)
         set_opaque_binding(data, element_type);

      return;
   }

   if (data->location < 0 ||
       data->location >= data->prog->sh.data->NumUniformStorage)
      return;

   struct gl_uniform_storage *storage =
      data->prog->sh.data->UniformStorage + data->location++;

   const unsigned elements = MAX2(storage->array_elements, 1);

   for (unsigned int i = 0; i < elements; i++)
      storage->storage[i].i = data->binding++;

   for (int sh = 0; sh < MESA_SHADER_STAGES; sh++) {
      struct gl_linked_shader *shader = data->shader_prog->_LinkedShaders[sh];

      if (!shader)
         continue;
      if (!storage->opaque[sh].active)
         continue;

      if (glsl_type_is_sampler(storage->type)) {
         for (unsigned i = 0; i < elements; i++) {
            const unsigned index = storage->opaque[sh].index + i;

            if (storage->is_bindless) {
               if (index >= shader->Program->sh.NumBindlessSamplers)
                  break;
               shader->Program->sh.BindlessSamplers[index].unit =
                  storage->storage[i].i;
               shader->Program->sh.BindlessSamplers[index].bound = true;
               shader->Program->sh.HasBoundBindlessSampler = true;
            } else {
               if (index >= ARRAY_SIZE(shader->Program->SamplerUnits))
                  break;
               shader->Program->SamplerUnits[index] =
                  storage->storage[i].i;
            }
         }
      } else if (glsl_type_is_image(storage->type)) {
         for (unsigned i = 0; i < elements; i++) {
            const unsigned index = storage->opaque[sh].index + i;

            if (storage->is_bindless) {
               if (index >= shader->Program->sh.NumBindlessImages)
                  break;
               shader->Program->sh.BindlessImages[index].unit =
                  storage->storage[i].i;
               shader->Program->sh.BindlessImages[index].bound = true;
               shader->Program->sh.HasBoundBindlessImage = true;
            } else {
               if (index >= ARRAY_SIZE(shader->Program->sh.ImageUnits))
                  break;
               shader->Program->sh.ImageUnits[index] =
                  storage->storage[i].i;
            }
         }
      }
   }
}

static void
copy_constant_to_storage(union gl_constant_value *storage,
                         const nir_constant *val,
                         const struct glsl_type *type,
                         unsigned int boolean_true)
{
   const enum glsl_base_type base_type = glsl_get_base_type(type);
   const unsigned n_columns = glsl_get_matrix_columns(type);
   const unsigned n_rows = glsl_get_vector_elements(type);
   unsigned dmul = glsl_base_type_is_64bit(base_type) ? 2 : 1;
   int i = 0;

   if (n_columns > 1) {
      const struct glsl_type *column_type = glsl_get_column_type(type);
      for (unsigned int column = 0; column < n_columns; column++) {
         copy_constant_to_storage(&storage[i], val->elements[column],
                                  column_type, boolean_true);
         i += n_rows * dmul;
      }
   } else {
      for (unsigned int row = 0; row < n_rows; row++) {
         switch (base_type) {
         case GLSL_TYPE_UINT:
            storage[i].u = val->values[row].u32;
            break;
         case GLSL_TYPE_INT:
         case GLSL_TYPE_SAMPLER:
            storage[i].i = val->values[row].i32;
            break;
         case GLSL_TYPE_FLOAT:
            storage[i].f = val->values[row].f32;
            break;
         case GLSL_TYPE_DOUBLE:
         case GLSL_TYPE_UINT64:
         case GLSL_TYPE_INT64:
            /* XXX need to check on big-endian */
            memcpy(&storage[i].u, &val->values[row].f64, sizeof(double));
            break;
         case GLSL_TYPE_BOOL:
            storage[i].b = val->values[row].u32 ? boolean_true : 0;
            break;
         case GLSL_TYPE_ARRAY:
         case GLSL_TYPE_STRUCT:
         case GLSL_TYPE_TEXTURE:
         case GLSL_TYPE_IMAGE:
         case GLSL_TYPE_ATOMIC_UINT:
         case GLSL_TYPE_INTERFACE:
         case GLSL_TYPE_VOID:
         case GLSL_TYPE_SUBROUTINE:
         case GLSL_TYPE_ERROR:
         case GLSL_TYPE_UINT16:
         case GLSL_TYPE_INT16:
         case GLSL_TYPE_UINT8:
         case GLSL_TYPE_INT8:
         case GLSL_TYPE_FLOAT16:
            /* All other types should have already been filtered by other
             * paths in the caller.
             */
            assert(!"Should not get here.");
            break;
         case GLSL_TYPE_COOPERATIVE_MATRIX:
            unreachable("unsupported base type cooperative matrix");
         }
         i += dmul;
      }
   }
}

struct set_uniform_initializer_closure {
   struct gl_shader_program *shader_prog;
   struct gl_program *prog;
   const nir_variable *var;
   int location;
   unsigned int boolean_true;
};

static void
set_uniform_initializer(struct set_uniform_initializer_closure *data,
                        const struct glsl_type *type,
                        const nir_constant *val)
{
   const struct glsl_type *t_without_array = glsl_without_array(type);

   if (glsl_type_is_struct_or_ifc(type)) {
      for (unsigned int i = 0; i < glsl_get_length(type); i++) {
         const struct glsl_type *field_type = glsl_get_struct_field(type, i);
         set_uniform_initializer(data, field_type, val->elements[i]);
      }
      return;
   }

   if (glsl_type_is_struct_or_ifc(t_without_array) ||
       (glsl_type_is_array(type) &&
        glsl_type_is_array(glsl_get_array_element(type)))) {
      const struct glsl_type *element_type = glsl_get_array_element(type);

      for (unsigned int i = 0; i < glsl_get_length(type); i++)
         set_uniform_initializer(data, element_type, val->elements[i]);

      return;
   }

   if (data->location < 0 ||
       data->location >= data->prog->sh.data->NumUniformStorage)
      return;

   struct gl_uniform_storage *storage =
      data->prog->sh.data->UniformStorage + data->location++;

   if (glsl_type_is_array(type)) {
      const struct glsl_type *element_type = glsl_get_array_element(type);
      const enum glsl_base_type base_type = glsl_get_base_type(element_type);
      const unsigned int elements = glsl_get_components(element_type);
      unsigned int idx = 0;
      unsigned dmul = glsl_base_type_is_64bit(base_type) ? 2 : 1;

      assert(glsl_get_length(type) >= storage->array_elements);
      for (unsigned int i = 0; i < storage->array_elements; i++) {
         copy_constant_to_storage(&storage->storage[idx],
                                  val->elements[i],
                                  element_type,
                                  data->boolean_true);

         idx += elements * dmul;
      }
   } else {
      copy_constant_to_storage(storage->storage,
                               val,
                               type,
                               data->boolean_true);

      if (glsl_type_is_sampler(storage->type)) {
         for (int sh = 0; sh < MESA_SHADER_STAGES; sh++) {
            struct gl_linked_shader *shader =
               data->shader_prog->_LinkedShaders[sh];

            if (shader && storage->opaque[sh].active) {
               unsigned index = storage->opaque[sh].index;

               shader->Program->SamplerUnits[index] = storage->storage[0].i;
            }
         }
      }
   }
}

void
gl_nir_set_uniform_initializers(const struct gl_constants *consts,
                                struct gl_shader_program *prog)
{
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      struct gl_linked_shader *sh = prog->_LinkedShaders[i];
      if (!sh)
         continue;

      nir_shader *nir = sh->Program->nir;
      assert(nir);

      nir_foreach_gl_uniform_variable(var, nir) {
         if (var->constant_initializer) {
            struct set_uniform_initializer_closure data = {
               .shader_prog = prog,
               .prog = sh->Program,
               .var = var,
               .location = var->data.location,
               .boolean_true = consts->UniformBooleanTrue
            };
            set_uniform_initializer(&data,
                                    var->type,
                                    var->constant_initializer);
         } else if (var->data.explicit_binding) {

            if (nir_variable_is_in_block(var)) {
               /* This case is handled by link_uniform_blocks */
               continue;
            }

            const struct glsl_type *without_array =
               glsl_without_array(var->type);

            if (glsl_type_is_sampler(without_array) ||
                glsl_type_is_image(without_array)) {
               struct set_opaque_binding_closure data = {
                  .shader_prog = prog,
                  .prog = sh->Program,
                  .var = var,
                  .binding = var->data.binding,
                  .location = var->data.location
               };
               set_opaque_binding(&data, var->type);
            }
         }
      }
   }
   memcpy(prog->data->UniformDataDefaults, prog->data->UniformDataSlots,
          sizeof(union gl_constant_value) * prog->data->NumUniformDataSlots);

}
