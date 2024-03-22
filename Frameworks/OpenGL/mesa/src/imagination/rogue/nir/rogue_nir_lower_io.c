/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "nir/nir.h"
#include "nir/nir_builder.h"
#include "nir/nir_search_helpers.h"
#include "rogue.h"
#include "util/macros.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * \file rogue_nir_lower_io.c
 *
 * \brief Contains the rogue_nir_lower_io pass.
 */

static void lower_vulkan_resource_index(nir_builder *b,
                                        nir_intrinsic_instr *intr)
{
   /* Pass along the desc_set, binding, desc_type. */
   unsigned desc_set = nir_intrinsic_desc_set(intr);
   unsigned binding = nir_intrinsic_binding(intr);
   unsigned desc_type = nir_intrinsic_desc_type(intr);

   nir_def *def = nir_vec3(b,
                               nir_imm_int(b, desc_set),
                               nir_imm_int(b, binding),
                               nir_imm_int(b, desc_type));
   nir_def_rewrite_uses(&intr->def, def);
   nir_instr_remove(&intr->instr);
}

static void lower_load_global_constant_to_scalar(nir_builder *b,
                                                 nir_intrinsic_instr *intr)
{
   /* Scalarize the load_global_constant. */
   b->cursor = nir_before_instr(&intr->instr);

   assert(intr->num_components > 1);

   nir_def *loads[NIR_MAX_VEC_COMPONENTS];

   for (uint8_t i = 0; i < intr->num_components; i++) {
      nir_intrinsic_instr *chan_intr =
         nir_intrinsic_instr_create(b->shader, intr->intrinsic);
      nir_def_init(&chan_intr->instr, &chan_intr->def, 1,
                   intr->def.bit_size);
      chan_intr->num_components = 1;

      nir_intrinsic_set_access(chan_intr, nir_intrinsic_access(intr));
      nir_intrinsic_set_align_mul(chan_intr, nir_intrinsic_align_mul(intr));
      nir_intrinsic_set_align_offset(chan_intr,
                                     nir_intrinsic_align_offset(intr));

      /* Address. */
      chan_intr->src[0] =
         nir_src_for_ssa(nir_iadd_imm(b, intr->src[0].ssa, i * 4));

      nir_builder_instr_insert(b, &chan_intr->instr);

      loads[i] = &chan_intr->def;
   }

   nir_def_rewrite_uses(&intr->def,
                            nir_vec(b, loads, intr->num_components));
   nir_instr_remove(&intr->instr);
}

static bool lower_intrinsic(nir_builder *b, nir_intrinsic_instr *instr)
{
   switch (instr->intrinsic) {
   case nir_intrinsic_vulkan_resource_index:
      lower_vulkan_resource_index(b, instr);
      return true;

   case nir_intrinsic_load_global_constant:
      lower_load_global_constant_to_scalar(b, instr);
      return true;

   default:
      break;
   }

   return false;
}

static bool lower_impl(nir_function_impl *impl)
{
   bool progress = false;
   nir_builder b = nir_builder_create(impl);

   nir_foreach_block (block, impl) {
      nir_foreach_instr_safe (instr, block) {
         b.cursor = nir_before_instr(instr);
         switch (instr->type) {
         case nir_instr_type_intrinsic:
            progress |= lower_intrinsic(&b, nir_instr_as_intrinsic(instr));
            break;

         default:
            break;
         }
      }
   }

   if (progress)
      nir_metadata_preserve(impl, nir_metadata_none);
   else
      nir_metadata_preserve(impl, nir_metadata_all);

   return progress;
}

PUBLIC
bool rogue_nir_lower_io(nir_shader *shader)
{
   bool progress = false;

   nir_foreach_function (function, shader) {
      if (function->impl)
         progress |= lower_impl(function->impl);
   }

   if (progress)
      nir_opt_dce(shader);

   return progress;
}
