/*
 * Copyright © 2019 Red Hat Inc.
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
 * \file
 *
 * Lower image operations by turning the image_deref_* into a image_* on an
 * index number or bindless_image_* intrinsic on a load_deref of the previous
 * deref source. All applicable indicies are also set so that fetching the
 * variable in the backend wouldn't be needed anymore.
 */

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_deref.h"

#include "compiler/glsl/gl_nir.h"

static void
type_size_align_1(const struct glsl_type *type, unsigned *size, unsigned *align)
{
   unsigned s;

   if (glsl_type_is_array(type))
      s = glsl_get_aoa_size(type);
   else
      s = 1;

   *size = s;
   *align = s;
}

static bool
lower_instr(nir_builder *b, nir_instr *instr, void *cb_data)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   bool bindless_only = *(bool *)cb_data;

   nir_intrinsic_instr *intrinsic = nir_instr_as_intrinsic(instr);

   nir_deref_instr *deref;
   nir_variable *var;

   switch (intrinsic->intrinsic) {
   case nir_intrinsic_image_deref_atomic:
   case nir_intrinsic_image_deref_atomic_swap:
   case nir_intrinsic_image_deref_load:
   case nir_intrinsic_image_deref_samples:
   case nir_intrinsic_image_deref_size:
   case nir_intrinsic_image_deref_samples_identical:
   case nir_intrinsic_image_deref_descriptor_amd:
   case nir_intrinsic_image_deref_store: {
      deref = nir_src_as_deref(intrinsic->src[0]);
      var = nir_deref_instr_get_variable(deref);
      break;
   }
   default:
      return false;
   }

   bool bindless = var->data.mode != nir_var_image || var->data.bindless;
   if (bindless_only && !bindless)
      return false;

   b->cursor = nir_before_instr(instr);

   nir_def *src;
   int range_base = 0;
   if (bindless) {
      src = nir_load_deref(b, deref);
   } else if (b->shader->options->lower_image_offset_to_range_base) {
      src = nir_build_deref_offset(b, deref, type_size_align_1);
      range_base = var->data.driver_location;
   } else {
      src = nir_iadd_imm(b,
                         nir_build_deref_offset(b, deref, type_size_align_1),
                         var->data.driver_location);
   }
   nir_rewrite_image_intrinsic(intrinsic, src, bindless);
   if (!bindless)
      nir_intrinsic_set_range_base(intrinsic, range_base);

   return true;
}

bool
gl_nir_lower_images(nir_shader *shader, bool bindless_only)
{
   return nir_shader_instructions_pass(shader, lower_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       &bindless_only);
}
