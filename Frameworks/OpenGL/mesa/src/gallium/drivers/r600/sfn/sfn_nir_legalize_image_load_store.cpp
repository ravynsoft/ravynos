/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2021 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "nir.h"
#include "nir_builder.h"
#include "nir_intrinsics.h"
#include "nir_intrinsics_indices.h"
#include "sfn_nir.h"

static nir_def *
r600_legalize_image_load_store_impl(nir_builder *b,
                                    nir_instr *instr,
                                    UNUSED void *_options)
{
   b->cursor = nir_before_instr(instr);
   auto ir = nir_instr_as_intrinsic(instr);

   nir_def *default_value = nir_imm_vec4(b, 0.0, 0.0, 0.0, 0.0);

   nir_def *result = NIR_LOWER_INSTR_PROGRESS_REPLACE;

   bool load_value = ir->intrinsic != nir_intrinsic_image_store;

   if (load_value)
      default_value =
         nir_imm_zero(b, ir->def.num_components, ir->def.bit_size);

   auto image_exists =
      nir_ult_imm(b, ir->src[0].ssa, b->shader->info.num_images);

   nir_if *if_exists = nir_push_if(b, image_exists);

   nir_if *load_if = nullptr;

   if (ir->intrinsic != nir_intrinsic_image_size) {

      /*  Image exists start */
      auto new_index =
         nir_umin(b,
                  ir->src[0].ssa,
                  nir_imm_int(b, b->shader->info.num_images - 1));
      nir_src_rewrite(&ir->src[0], new_index);

      enum glsl_sampler_dim dim = nir_intrinsic_image_dim(ir);

      unsigned num_components = 2;
      switch (dim) {
      case GLSL_SAMPLER_DIM_BUF:
      case GLSL_SAMPLER_DIM_1D:
         num_components = 1;
         break;
      case GLSL_SAMPLER_DIM_2D:
      case GLSL_SAMPLER_DIM_MS:
      case GLSL_SAMPLER_DIM_RECT:
      case GLSL_SAMPLER_DIM_CUBE:
         num_components = 2;
         break;
      case GLSL_SAMPLER_DIM_3D:
         num_components = 3;
         break;
      default:
         unreachable("Unexpected image size");
      }

      if (num_components < 3 && nir_intrinsic_image_array(ir))
         num_components++;

      auto img_size = nir_image_size(b,
                                     num_components,
                                     32,
                                     ir->src[0].ssa,
                                     nir_imm_int(b, 0),
                                     dim,
                                     nir_intrinsic_image_array(ir),
                                     nir_intrinsic_format(ir),
                                     nir_intrinsic_access(ir),
                                     nir_intrinsic_range_base(ir));

      unsigned mask = (1 << num_components) - 1;
      unsigned num_src1_comp = MIN2(ir->src[1].ssa->num_components, num_components);
      unsigned src1_mask = (1 << num_src1_comp) - 1;

      auto in_range = nir_ult(b,
                              nir_channels(b, ir->src[1].ssa, src1_mask),
                              nir_channels(b, img_size, mask));

      switch (num_components) {
      case 2:
         in_range = nir_iand(b, nir_channel(b, in_range, 0), nir_channel(b, in_range, 1));
         break;
      case 3: {
         auto tmp = nir_iand(b, nir_channel(b, in_range, 0), nir_channel(b, in_range, 1));
         in_range = nir_iand(b, tmp, nir_channel(b, in_range, 2));
         break;
      }
      }

      /*  Access is in range start */
      load_if = nir_push_if(b, in_range);
   }

   auto new_load = nir_instr_clone(b->shader, instr);
   auto new_load_ir = nir_instr_as_intrinsic(new_load);

   nir_builder_instr_insert(b, new_load);

   if (load_value)
      result = &new_load_ir->def;

   if (ir->intrinsic != nir_intrinsic_image_size) {
      /*  Access is out of range start */
      nir_if *load_else = nir_push_else(b, load_if);

      nir_pop_if(b, load_else);
      /* End range check */

      if (load_value)
         result = nir_if_phi(b, result, default_value);
   }

   /* Start image doesn't exists */
   nir_if *else_exists = nir_push_else(b, if_exists);

   /* Nothing to do, default is already set */
   nir_pop_if(b, else_exists);

   if (load_value)
      result = nir_if_phi(b, result, default_value);

   if (load_value)
      b->cursor = nir_after_instr(result->parent_instr);
   else
      b->cursor = nir_after_cf_node(&else_exists->cf_node);

   return result;
}

static bool
r600_legalize_image_load_store_filter(const nir_instr *instr, UNUSED const void *_options)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   auto ir = nir_instr_as_intrinsic(instr);
   switch (ir->intrinsic) {
   case nir_intrinsic_image_store:
   case nir_intrinsic_image_load:
   case nir_intrinsic_image_atomic:
   case nir_intrinsic_image_atomic_swap:
   case nir_intrinsic_image_size:
      return true;
   default:
      return false;
   }
}

/* This pass makes sure only existing images are accessd and
 * the access is within range, if not zero is returned by all
 * image ops that return a value.
 */
bool
r600_legalize_image_load_store(nir_shader *shader)
{
   bool progress = nir_shader_lower_instructions(shader,
                                        r600_legalize_image_load_store_filter,
                                        r600_legalize_image_load_store_impl,
                                        nullptr);
   return progress;
};
