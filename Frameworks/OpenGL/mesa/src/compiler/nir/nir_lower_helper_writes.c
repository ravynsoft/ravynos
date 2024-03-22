/*
 * Copyright (C) 2020-2021 Collabora, Ltd.
 * Copyright © 2020 Valve Corporation
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

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"

static bool
lower(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   bool *lower_plain_stores = data;

   switch (intr->intrinsic) {
   case nir_intrinsic_global_atomic:
   case nir_intrinsic_global_atomic_swap:
   case nir_intrinsic_image_atomic:
   case nir_intrinsic_image_atomic_swap:
   case nir_intrinsic_bindless_image_atomic:
   case nir_intrinsic_bindless_image_atomic_swap:
      break;
   case nir_intrinsic_store_global:
   case nir_intrinsic_image_store:
   case nir_intrinsic_bindless_image_store:
      if (!(*lower_plain_stores))
         return false;
      else
         break;
   default:
      return false;
   }

   b->cursor = nir_before_instr(&intr->instr);
   bool has_dest = nir_intrinsic_infos[intr->intrinsic].has_dest;
   nir_def *undef = NULL;

   nir_def *helper = nir_load_helper_invocation(b, 1);
   nir_push_if(b, nir_inot(b, helper));
   nir_instr_remove(&intr->instr);
   nir_builder_instr_insert(b, &intr->instr);

   /* Per the spec, it does not matter what we return for helper threads.
    * Represent this by an ssa_undef in the hopes the backend will be clever
    * enough to optimize out the phi.
    *
    *    Fragment shader helper invocations execute the same shader code as
    *    non-helper invocations, but will not have side effects that modify the
    *    framebuffer or other shader-accessible memory. In particular:
    *
    *       ...
    *
    *       Atomic operations to image, buffer, or atomic counter variables
    *       performed by helper invocations have no effect on the underlying
    *       image or buffer memory. The values returned by such atomic
    *       operations are undefined.
    */
   if (has_dest) {
      nir_push_else(b, NULL);
      undef = nir_undef(b, intr->def.num_components,
                        intr->def.bit_size);
   }

   nir_pop_if(b, NULL);

   if (has_dest) {
      nir_def *phi = nir_if_phi(b, &intr->def, undef);

      /* We can't use nir_def_rewrite_uses_after on phis, so use the global
       * version and fixup the phi manually
       */
      nir_def_rewrite_uses(&intr->def, phi);

      nir_instr *phi_instr = phi->parent_instr;
      nir_phi_instr *phi_as_phi = nir_instr_as_phi(phi_instr);
      nir_phi_src *phi_src = nir_phi_get_src_from_block(phi_as_phi,
                                                        intr->instr.block);
      nir_src_rewrite(&phi_src->src, &intr->def);
   }

   return true;
}

bool
nir_lower_helper_writes(nir_shader *shader, bool lower_plain_stores)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);
   return nir_shader_intrinsics_pass(shader, lower, nir_metadata_none,
                                       &lower_plain_stores);
}
