/*
 * Copyright © 2022 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "nir.h"
#include "nir_builder.h"

#include "ir3/ir3_descriptor.h"

static bool
lower_intrinsic(nir_builder *b, nir_intrinsic_instr *intr)
{
   unsigned desc_offset;

   switch (intr->intrinsic) {
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_store_ssbo:
   case nir_intrinsic_ssbo_atomic:
   case nir_intrinsic_ssbo_atomic_swap:
   case nir_intrinsic_get_ssbo_size:
      desc_offset = IR3_BINDLESS_SSBO_OFFSET;
      break;
   case nir_intrinsic_image_load:
   case nir_intrinsic_image_store:
   case nir_intrinsic_image_atomic:
   case nir_intrinsic_image_atomic_swap:
   case nir_intrinsic_image_size:
   case nir_intrinsic_image_samples:
      desc_offset = IR3_BINDLESS_IMAGE_OFFSET;
      break;
   default:
      return false;
   }

   unsigned buffer_src;
   if (intr->intrinsic == nir_intrinsic_store_ssbo) {
      /* store_ssbo has the value first, and ssbo src as 2nd src: */
      buffer_src = 1;
   } else {
      /* the rest have ssbo src as 1st src: */
      buffer_src = 0;
   }

   unsigned set = ir3_shader_descriptor_set(b->shader->info.stage);
   nir_def *src = intr->src[buffer_src].ssa;
   src = nir_iadd_imm(b, src, desc_offset);
   /* An out-of-bounds index into an SSBO/image array can cause a GPU fault
    * on access to the descriptor (I don't see any hw mechanism to bound the
    * access).  We could just allow the resulting iova fault (it is a read
    * fault, so shouldn't corrupt anything), but at the cost of one extra
    * instruction (as long as IR3_BINDLESS_DESC_COUNT is a power-of-two) we
    * can avoid the dmesg spam and users thinking this is a driver bug:
    */
   src = nir_umod_imm(b, src, IR3_BINDLESS_DESC_COUNT);
   nir_def *bindless = nir_bindless_resource_ir3(b, 32, src, set);
   nir_src_rewrite(&intr->src[buffer_src], bindless);

   return true;
}

static bool
lower_instr(nir_builder *b, nir_instr *instr, void *cb_data)
{
   b->cursor = nir_before_instr(instr);
   switch (instr->type) {
   case nir_instr_type_intrinsic:
      return lower_intrinsic(b, nir_instr_as_intrinsic(instr));
   default:
      return false;
   }
}

/**
 * Lower bindful image/SSBO to bindless
 */
bool
ir3_nir_lower_io_to_bindless(nir_shader *shader)
{
   /* Note: We don't currently support API level bindless, as we assume we
    * can remap bindful images/SSBOs to bindless while controlling the entire
    * descriptor set space.
    *
    * If we needed to support API level bindless, we could probably just remap
    * bindful ops to a range of the descriptor set space that does not conflict
    * with what we advertise for bindless descriptors?  But I'm not sure that
    * ARB_bindless_texture is of too much value to care about, especially for
    * GLES
    */
   assert(!shader->info.uses_bindless);

   return nir_shader_instructions_pass(shader, lower_instr, nir_metadata_none, NULL);
}
