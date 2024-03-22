/*
 * Copyright © 2013 Intel Corporation
 * Copyright © 2023 Valve Corporation
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

/**
 *
 * This lowering pass converts all interface blocks with instance names
 * into interface blocks without an instance name.
 *
 * For example, the following shader:
 *
 *   out block {
 *     float block_var;
 *   } inst_name;
 *
 *   main()
 *   {
 *     inst_name.block_var = 0.0;
 *   }
 *
 * Is rewritten to:
 *
 *   out block {
 *     float block_var;
 *   };
 *
 *   main()
 *   {
 *     block_var = 0.0;
 *   }
 *
 * This takes place after the shader code has already been verified with
 * the interface name in place.
 *
 * The linking phase will use the interface block name rather than the
 * interface's instance name when linking interfaces.
 *
 * This modification to the ir allows our currently existing dead code
 * elimination to work with interface blocks without changes.
 */

#include "nir.h"
#include "nir_builder.h"
#include "nir_deref.h"
#include "gl_nir.h"
#include "glsl_types.h"

#include "main/shader_types.h"

struct lower_named_block_state {
   void *mem_ctx;
   struct hash_table *interface_namespace;
};

static const struct glsl_type *
process_array_type(const struct glsl_type *type, unsigned idx)
{
   const struct glsl_type *element_type = glsl_get_array_element(type);
   unsigned length = glsl_get_length(type);
   if (glsl_type_is_array(element_type)) {
      const struct glsl_type *new_array_type =
         process_array_type(element_type, idx);
      return  glsl_array_type(new_array_type, length, 0);
   } else {
      return glsl_array_type(glsl_get_struct_field(element_type, idx),
                             length, 0);
   }
}

static nir_deref_instr *
process_derefs(nir_builder *b, nir_deref_instr **p, nir_deref_instr *parent)
{
   bool found_ifc = false;
   for (; *p; p++) {
      if ((*p)->deref_type == nir_deref_type_array) {
         parent = nir_build_deref_array(b, parent, (*p)->arr.index.ssa);
      } else if ((*p)->deref_type == nir_deref_type_struct) {
         if (!found_ifc) {
            /* We found the interface block so just skip it */
            found_ifc = true;
            continue;
         } else {
            parent = nir_build_deref_struct(b, parent, (*p)->strct.index);
         }
      }
   }

   return parent;
}

/* Disable packing on varyings used by interpolate functions. This must be
 * called after lowering blocks to avoid disabling packing on the entire
 * block.
 */
static void
disable_varying_packing_when_used_by_interpolate_functions(nir_intrinsic_instr *intr,
                                                           nir_variable *var)
{
   if (intr->intrinsic == nir_intrinsic_interp_deref_at_centroid ||
       intr->intrinsic == nir_intrinsic_interp_deref_at_sample ||
       intr->intrinsic == nir_intrinsic_interp_deref_at_offset) {

      /* This disables varying packing for this input. */
      var->data.must_be_shader_input = 1;
   }
}

static char *
create_ifc_field_name_str(void *mem_ctx, nir_variable *var,
                          const struct glsl_type *iface_t, unsigned f_idx)
{
   char *iface_field_name =
      ralloc_asprintf(mem_ctx, "%s %s.%s.%s",
                      var->data.mode == nir_var_shader_in ? "in" : "out",
                      glsl_get_type_name(iface_t), var->name,
                      glsl_get_struct_elem_name(iface_t, f_idx));

   return iface_field_name;
}

static bool
flatten_named_interface_deref(void *mem_ctx, nir_builder *b,
                              nir_deref_instr *deref,
                              nir_intrinsic_instr *intr,
                              struct hash_table *interface_namespace,
                              bool is_src0)
{
   nir_variable_mode mask = nir_var_shader_in | nir_var_shader_out;

   if (!nir_deref_mode_is_one_of(deref, mask))
      return false;

   nir_variable *var = nir_deref_instr_get_variable(deref);

   const struct glsl_type * iface_t = glsl_without_array(var->type);
   if (iface_t != var->interface_type)
      return false;

   nir_deref_path path;
   nir_deref_path_init(&path, deref, NULL);

   assert(path.path[0]->deref_type == nir_deref_type_var);
   nir_deref_instr **p = &path.path[1];

   char *iface_field_name = NULL;
   for (; *p; p++) {
      if ((*p)->deref_type == nir_deref_type_struct) {
         iface_field_name =
            create_ifc_field_name_str(mem_ctx, var, iface_t,
                                      (*p)->strct.index);
         break;
      }
   }
   assert(iface_field_name);

   /* Find the variable in the set of flattened interface blocks */
   struct hash_entry *entry =
      _mesa_hash_table_search(interface_namespace, iface_field_name);
   assert(entry);

   nir_variable *found_var = (nir_variable *) entry->data;

   if (intr->intrinsic == nir_intrinsic_store_deref ||
       (intr->intrinsic == nir_intrinsic_copy_deref && is_src0))
      found_var->data.assigned = 1;

   b->cursor = nir_before_instr(&intr->instr);
   nir_deref_instr *deref_var = nir_build_deref_var(b, found_var);
   if (glsl_type_is_array(found_var->type) ||
       glsl_type_is_struct(found_var->type) ||
       glsl_type_is_matrix(found_var->type)) {
      p = &path.path[1];
      deref_var = process_derefs(b, p, deref_var);
   }

   disable_varying_packing_when_used_by_interpolate_functions(intr,
                                                              found_var);

   nir_deref_path_finish(&path);

   nir_def_rewrite_uses(&deref->def, &deref_var->def);

   return true;
}

static bool
flatten_named_interface_derefs(nir_builder *b, nir_intrinsic_instr *intr,
                               void *cb_data)
{
   struct lower_named_block_state *state =
      (struct lower_named_block_state *) cb_data;

   if (intr->intrinsic != nir_intrinsic_copy_deref &&
       intr->intrinsic != nir_intrinsic_load_deref &&
       intr->intrinsic != nir_intrinsic_store_deref &&
       intr->intrinsic != nir_intrinsic_interp_deref_at_centroid &&
       intr->intrinsic != nir_intrinsic_interp_deref_at_sample &&
       intr->intrinsic != nir_intrinsic_interp_deref_at_offset &&
       intr->intrinsic != nir_intrinsic_interp_deref_at_vertex)
      return false;

   nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
   bool progress =
      flatten_named_interface_deref(state->mem_ctx, b, deref, intr,
                                    state->interface_namespace, true);

   if (intr->intrinsic == nir_intrinsic_copy_deref) {
      deref = nir_src_as_deref(intr->src[1]);
      progress |=
         flatten_named_interface_deref(state->mem_ctx, b, deref, intr,
                                       state->interface_namespace, false);
   }

   return progress;
}

static void
lower_named_interface_blocks(struct gl_linked_shader *sh)
{
   void *mem_ctx = ralloc_context(NULL);

   struct hash_table *interface_namespace =
      _mesa_hash_table_create(mem_ctx, _mesa_hash_string,
                              _mesa_key_string_equal);

   /* First pass: adjust instance block variables with an instance name
    * to not have an instance name.
    *
    * The interface block variables are stored in the interface_namespace
    * hash table so they can be used in the second pass.
    */
   nir_foreach_variable_with_modes_safe(var, sh->Program->nir,
                                        nir_var_shader_in | nir_var_shader_out) {
      const struct glsl_type * iface_t = glsl_without_array(var->type);
      if (iface_t != var->interface_type)
         continue;

      for (unsigned i = 0; i < iface_t->length; i++) {
         const char *field_name = glsl_get_struct_elem_name(iface_t, i);
         char *iface_field_name =
            create_ifc_field_name_str(mem_ctx, var, iface_t, i);

         struct hash_entry *entry = _mesa_hash_table_search(interface_namespace,
                                                            iface_field_name);
         nir_variable *found_var = entry ? (nir_variable *) entry->data : NULL;
         if (!found_var) {
            const struct glsl_struct_field *field_data =
               glsl_get_struct_field_data(iface_t, i);

            nir_variable *new_var = rzalloc(sh->Program->nir, nir_variable);
            new_var->name = ralloc_strdup(new_var, field_name);
            if (!glsl_type_is_array(var->type)) {
               new_var->type =  glsl_get_struct_field(iface_t, i);
            } else {
               new_var->type = process_array_type(var->type, i);
            }
            new_var->data.mode = var->data.mode;
            new_var->data.location = field_data->location;
            new_var->data.location_frac = field_data->component >= 0 ?
               field_data->component : 0;
            new_var->data.explicit_location = (new_var->data.location >= 0);
            new_var->data.offset = field_data->offset;
            new_var->data.explicit_offset = (field_data->offset >= 0);
            new_var->data.xfb.buffer = field_data->xfb_buffer;
            new_var->data.explicit_xfb_buffer = field_data->explicit_xfb_buffer;
            new_var->data.interpolation = field_data->interpolation;
            new_var->data.centroid = field_data->centroid;
            new_var->data.sample = field_data->sample;
            new_var->data.patch = field_data->patch;
            new_var->data.stream = var->data.stream;
            new_var->data.how_declared = var->data.how_declared;
            new_var->data.from_named_ifc_block = 1;

            new_var->interface_type = var->type;
            _mesa_hash_table_insert(interface_namespace, iface_field_name,
                                    new_var);

            nir_shader_add_variable(sh->Program->nir, new_var);
         }
      }
   }

   /* Second pass: redirect dereferences to the new vars. */
   struct lower_named_block_state state;
   state.mem_ctx = mem_ctx;
   state.interface_namespace = interface_namespace;
   nir_shader_intrinsics_pass(sh->Program->nir, flatten_named_interface_derefs,
                              nir_metadata_block_index |
                              nir_metadata_dominance, &state);

   /* Third pass: Mark now lowered blks as ordinary globals to be dead code
    * eliminated. Also use this oppotunity to set the compact flag where
    * needed now that the default interface block has been lowered away.
    */
   nir_foreach_variable_with_modes(var, sh->Program->nir,
                                   nir_var_shader_in | nir_var_shader_out) {

      if (var->data.mode == nir_var_shader_in) {
         if (sh->Program->nir->info.stage == MESA_SHADER_TESS_EVAL &&
             (var->data.location == VARYING_SLOT_TESS_LEVEL_INNER ||
              var->data.location == VARYING_SLOT_TESS_LEVEL_OUTER)) {
            var->data.compact =
               glsl_type_is_scalar(glsl_without_array(var->type));
         }

         if (sh->Program->nir->info.stage > MESA_SHADER_VERTEX &&
             var->data.location >= VARYING_SLOT_CLIP_DIST0 &&
             var->data.location <= VARYING_SLOT_CULL_DIST1) {
            var->data.compact =
               glsl_type_is_scalar(glsl_without_array(var->type));
         }
      } else {
         assert(var->data.mode == nir_var_shader_out);

         if (sh->Program->nir->info.stage == MESA_SHADER_TESS_CTRL &&
             (var->data.location == VARYING_SLOT_TESS_LEVEL_INNER ||
              var->data.location == VARYING_SLOT_TESS_LEVEL_OUTER)) {
            var->data.compact =
               glsl_type_is_scalar(glsl_without_array(var->type));
         }

         if (sh->Program->nir->info.stage <= MESA_SHADER_GEOMETRY &&
             var->data.location >= VARYING_SLOT_CLIP_DIST0 &&
             var->data.location <= VARYING_SLOT_CULL_DIST1) {
            var->data.compact =
               glsl_type_is_scalar(glsl_without_array(var->type));
         }
      }

      const struct glsl_type * iface_t = glsl_without_array(var->type);
      if (!(iface_t == var->interface_type))
         continue;

      var->data.mode = nir_var_shader_temp;
   }
   nir_fixup_deref_modes(sh->Program->nir);

   ralloc_free(mem_ctx);
}

void
gl_nir_lower_named_interface_blocks(struct gl_shader_program *prog)
{
   for (unsigned int i = 0; i < MESA_SHADER_STAGES; i++) {
      if (prog->_LinkedShaders[i] != NULL)
         lower_named_interface_blocks(prog->_LinkedShaders[i]);
   }
}

