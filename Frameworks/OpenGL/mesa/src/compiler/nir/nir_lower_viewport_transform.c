/*
 * Copyright (C) 2019 Alyssa Rosenzweig
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

/* On some hardware (particularly, all current versions of Mali GPUs),
 * vertex shaders do not output gl_Position in world-space. Instead, they
 * output gl_Position in transformed screen space via the "pseudo"
 * position varying. Thus, this pass finds writes to gl_Position and
 * changes them to transformed writes, still to gl_Position. The
 * outputted screen space is still written back to VARYING_SLOT_POS,
 * which is semantically ambiguous but nevertheless a good match for
 * Gallium/NIR/Mali.
 *
 * Implements coordinate transformation as defined in section 12.5
 * "Coordinate Transformation" of the OpenGL ES 3.2 full specification.
 *
 * This pass must run before lower_vars/lower_io such that derefs are
 * still in place.
 */

#include "nir/nir.h"
#include "nir/nir_builder.h"

static bool
lower_viewport_transform_instr(nir_builder *b, nir_intrinsic_instr *intr,
                               void *data)
{
   if (intr->intrinsic != nir_intrinsic_store_deref)
      return false;

   nir_variable *var = nir_intrinsic_get_var(intr, 0);
   if (var->data.mode != nir_var_shader_out ||
       var->data.location != VARYING_SLOT_POS)
      return false;

   b->cursor = nir_before_instr(&intr->instr);

   /* Grab the source and viewport */
   nir_def *input_point = intr->src[1].ssa;
   nir_def *scale = nir_load_viewport_scale(b);
   nir_def *offset = nir_load_viewport_offset(b);

   /* World space to normalised device coordinates to screen space */

   nir_def *w_recip = nir_frcp(b, nir_channel(b, input_point, 3));

   nir_def *ndc_point = nir_fmul(b, nir_trim_vector(b, input_point, 3),
                                 w_recip);

   nir_def *screen = nir_fadd(b, nir_fmul(b, ndc_point, scale),
                              offset);

   /* gl_Position will be written out in screenspace xyz, with w set to
    * the reciprocal we computed earlier. The transformed w component is
    * then used for perspective-correct varying interpolation. The
    * transformed w component must preserve its original sign; this is
    * used in depth clipping computations
    */

   nir_def *screen_space = nir_vec4(b,
                                    nir_channel(b, screen, 0),
                                    nir_channel(b, screen, 1),
                                    nir_channel(b, screen, 2),
                                    w_recip);

   nir_src_rewrite(&intr->src[1], screen_space);
   return true;
}

bool
nir_lower_viewport_transform(nir_shader *shader)
{
   assert((shader->info.stage == MESA_SHADER_VERTEX) || (shader->info.stage == MESA_SHADER_GEOMETRY) || (shader->info.stage == MESA_SHADER_TESS_EVAL));

   return nir_shader_intrinsics_pass(shader, lower_viewport_transform_instr,
                                       nir_metadata_block_index |
                                          nir_metadata_dominance,
                                       NULL);
}
