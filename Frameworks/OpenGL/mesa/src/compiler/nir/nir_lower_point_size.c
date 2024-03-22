/*
 * Copyright © 2019 Raspberry Pi Ltd
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

#include "nir_builder.h"

/** @file nir_lower_point_size.c
 *
 * The OpenGL spec requires that implementations clamp gl_PointSize to an
 * implementation-dependant point size range. The OpenGL ES 3.0 spec further
 * requires that this range must match GL_ALIASED_POINT_SIZE_RANGE.
 * Some hardware such as V3D don't clamp to a valid range automatically so
 * the driver must clamp the point size written by the shader manually to a
 * valid range.
 */
static bool
lower_point_size_intrin(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   float *minmax = (float *)data;

   if (intr->intrinsic != nir_intrinsic_store_deref)
      return false;

   nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
   nir_variable *var = nir_deref_instr_get_variable(deref);
   if (var->data.location != VARYING_SLOT_PSIZ)
      return false;

   b->cursor = nir_before_instr(&intr->instr);

   assert(intr->src[1].ssa->num_components == 1);
   nir_def *psiz = intr->src[1].ssa;

   if (minmax[0] > 0.0f)
      psiz = nir_fmax(b, psiz, nir_imm_float(b, minmax[0]));

   if (minmax[1] > 0.0f)
      psiz = nir_fmin(b, psiz, nir_imm_float(b, minmax[1]));

   nir_src_rewrite(&intr->src[1], psiz);

   return true;
}

/**
 * Clamps gl_PointSize to the range [min, max]. If either min or max are not
 * greater than 0 then no clamping is done for that side of the range.
 */
bool
nir_lower_point_size(nir_shader *s, float min, float max)
{
   assert(s->info.stage != MESA_SHADER_FRAGMENT &&
          s->info.stage != MESA_SHADER_COMPUTE);

   assert(min > 0.0f || max > 0.0f);
   assert(min <= 0.0f || max <= 0.0f || min <= max);

   float minmax[] = { min, max };
   return nir_shader_intrinsics_pass(s, lower_point_size_intrin,
                                     nir_metadata_block_index |
                                        nir_metadata_dominance,
                                     minmax);
}
