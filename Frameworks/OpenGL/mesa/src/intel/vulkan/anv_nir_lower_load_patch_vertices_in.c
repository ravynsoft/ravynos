/*
 * Copyright © 2023 Intel Corporation
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

/**
 * This file implements the lowering required for
 * VK_EXT_extended_dynamic_state2 extendedDynamicState2PatchControlPoints.
 *
 * When VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT is set on a pipeline, we
 * need to compile the TCS shader assuming the max (32) number of control
 * points. The actually value is provided through push constants.
 */

#include "anv_nir.h"
#include "nir_builder.h"

#define sizeof_field(type, field) sizeof(((type *)0)->field)

static bool
lower_patch_vertices_in_instr(nir_builder *b, nir_intrinsic_instr *load,
                              UNUSED void *_data)
{
   if (load->intrinsic != nir_intrinsic_load_patch_vertices_in)
      return false;

   b->cursor = nir_before_instr(&load->instr);

   nir_def_rewrite_uses(
      &load->def,
      nir_load_push_constant(
         b, 1, 32,
         nir_imm_int(b, 0),
         .base = offsetof(struct anv_push_constants, gfx.tcs_input_vertices),
         .range = sizeof_field(struct anv_push_constants, gfx.tcs_input_vertices)));
   nir_instr_remove(&load->instr);

   return true;
}

bool
anv_nir_lower_load_patch_vertices_in(nir_shader *shader)
{
   return nir_shader_intrinsics_pass(shader, lower_patch_vertices_in_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       NULL);
}
