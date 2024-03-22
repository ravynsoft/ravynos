/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "nir.h"
#include "nir_builder.h"
#include "radv_nir.h"

static nir_variable *
find_layer_out_var(nir_shader *nir)
{
   nir_variable *var = nir_find_variable_with_location(nir, nir_var_shader_out, VARYING_SLOT_LAYER);
   if (var != NULL)
      return var;

   var = nir_variable_create(nir, nir_var_shader_out, glsl_int_type(), "layer id");
   var->data.location = VARYING_SLOT_LAYER;
   var->data.interpolation = INTERP_MODE_NONE;

   return var;
}

bool
radv_nir_export_multiview(nir_shader *nir)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   bool progress = false;

   nir_builder b = nir_builder_create(impl);

   /* This pass is not suitable for mesh shaders, because it can't know the mapping between API mesh
    * shader invocations and output primitives. Needs to be handled in ac_nir_lower_ngg.
    */
   assert(nir->info.stage == MESA_SHADER_VERTEX || nir->info.stage == MESA_SHADER_TESS_EVAL ||
          nir->info.stage == MESA_SHADER_GEOMETRY);

   /* Iterate in reverse order since there should be only one deref store to POS after
    * lower_io_to_temporaries for vertex shaders and inject the layer there. For geometry shaders,
    * the layer is injected right before every emit_vertex_with_counter.
    */
   nir_variable *layer = NULL;
   nir_foreach_block_reverse (block, impl) {
      nir_foreach_instr_reverse (instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         if (nir->info.stage == MESA_SHADER_GEOMETRY) {
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic != nir_intrinsic_emit_vertex_with_counter)
               continue;

            b.cursor = nir_before_instr(instr);
         } else {
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic != nir_intrinsic_store_deref)
               continue;

            nir_variable *var = nir_intrinsic_get_var(intr, 0);
            if (var->data.mode != nir_var_shader_out || var->data.location != VARYING_SLOT_POS)
               continue;

            b.cursor = nir_after_instr(instr);
         }

         if (!layer)
            layer = find_layer_out_var(nir);

         nir_store_var(&b, layer, nir_load_view_index(&b), 1);

         /* Update outputs_written to reflect that the pass added a new output. */
         nir->info.outputs_written |= BITFIELD64_BIT(VARYING_SLOT_LAYER);

         progress = true;
         if (nir->info.stage == MESA_SHADER_VERTEX)
            break;
      }
      if (nir->info.stage == MESA_SHADER_VERTEX && progress)
         break;
   }

   if (progress)
      nir_metadata_preserve(impl, nir_metadata_block_index | nir_metadata_dominance);
   else
      nir_metadata_preserve(impl, nir_metadata_all);

   return progress;
}
