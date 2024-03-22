/*
 * Copyright © 2016 Intel Corporation
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
#include "lvp_lower_vulkan_resource.h"

static nir_def *
load_frag_coord(nir_builder *b)
{
   nir_variable *pos =
      nir_find_variable_with_location(b->shader, nir_var_shader_in,
                                      VARYING_SLOT_POS);
   if (pos == NULL) {
      pos = nir_variable_create(b->shader, nir_var_shader_in,
                                           glsl_vec4_type(), NULL);
      pos->data.location = VARYING_SLOT_POS;
   }
   /**
    * From Vulkan spec:
    *   "The OriginLowerLeft execution mode must not be used; fragment entry
    *    points must declare OriginUpperLeft."
    *
    * So at this point origin_upper_left should be true
    */
   assert(b->shader->info.fs.origin_upper_left == true);

   return nir_load_var(b, pos);
}

static bool
try_lower_input_load(nir_intrinsic_instr *load, bool use_fragcoord_sysval)
{
   nir_deref_instr *deref = nir_src_as_deref(load->src[0]);
   assert(glsl_type_is_image(deref->type));

   enum glsl_sampler_dim image_dim = glsl_get_sampler_dim(deref->type);
   if (image_dim != GLSL_SAMPLER_DIM_SUBPASS &&
       image_dim != GLSL_SAMPLER_DIM_SUBPASS_MS)
      return false;

   nir_builder b = nir_builder_at(nir_before_instr(&load->instr));

   nir_def *frag_coord = use_fragcoord_sysval ? nir_load_frag_coord(&b)
                                                  : load_frag_coord(&b);
   frag_coord = nir_f2i32(&b, frag_coord);
   nir_def *offset = nir_trim_vector(&b, load->src[1].ssa, 2);
   nir_def *pos = nir_iadd(&b, frag_coord, offset);

   nir_def *layer = nir_load_view_index(&b);
   nir_def *coord =
      nir_vec4(&b, nir_channel(&b, pos, 0), nir_channel(&b, pos, 1), layer, nir_imm_int(&b, 0));

   nir_src_rewrite(&load->src[1], coord);

   return true;
}

bool
lvp_lower_input_attachments(nir_shader *shader, bool use_fragcoord_sysval)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *load = nir_instr_as_intrinsic(instr);

            if (load->intrinsic != nir_intrinsic_image_deref_load)
               continue;

            progress |= try_lower_input_load(load, use_fragcoord_sysval);
         }
      }
   }

   return progress;
}
