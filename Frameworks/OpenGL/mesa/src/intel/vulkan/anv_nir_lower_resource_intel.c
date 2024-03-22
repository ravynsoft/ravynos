/*
 * Copyright © 2022 Intel Corporation
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

#include "anv_nir.h"
#include "nir_builder.h"

/* This pass updates the block index in the resource_intel intrinsics if the
 * array index is constant.
 *
 * This pass must be run before anv_nir_compute_push_layout().
 */
static bool
update_resource_intel_block(nir_builder *b, nir_intrinsic_instr *intrin,
                            UNUSED void *data)
{
   if (intrin->intrinsic != nir_intrinsic_resource_intel)
      return false;

   /* If the array index in the descriptor binding is not const, we won't be
    * able to turn this load_ubo into a push constant.
    *
    * Also if not pushable, set the block to 0xffffffff.
    *
    * Otherwise we need to update the block index by adding the array index so
    * that when anv_nir_compute_push_layout() uses the block value it uses the
    * right surface in the array of the binding.
    */
   if (!nir_src_is_const(intrin->src[2]) ||
       !(nir_intrinsic_resource_access_intel(intrin) &
         nir_resource_intel_pushable)) {
      nir_intrinsic_set_resource_block_intel(intrin, 0xffffffff);
      nir_intrinsic_set_resource_access_intel(
         intrin,
         nir_intrinsic_resource_access_intel(intrin) &
         ~nir_resource_intel_pushable);
   } else {
      nir_intrinsic_set_resource_block_intel(
         intrin,
         nir_intrinsic_resource_block_intel(intrin) +
         nir_src_as_uint(intrin->src[2]));
   }

   return true;
}

bool
anv_nir_update_resource_intel_block(nir_shader *shader)
{
   return nir_shader_intrinsics_pass(shader, update_resource_intel_block,
                                       nir_metadata_all,
                                       NULL);
}

struct lower_resource_state {
   enum anv_descriptor_set_layout_type desc_type;
   const struct anv_physical_device *device;
};

/* This pass lower resource_intel surface_index source, combining the
 * descriptor set offset with the surface offset in the descriptor set.
 *
 * This pass must be run after anv_nir_compute_push_layout() because we want
 * the push constant selection to tell if the surface offset is constant. Once
 * combined the constant detection does not work anymore.
 */
static bool
lower_resource_intel(nir_builder *b, nir_intrinsic_instr *intrin, void *data)
{
   if (intrin->intrinsic != nir_intrinsic_resource_intel)
      return false;

   const bool is_bindless =
      (nir_intrinsic_resource_access_intel(intrin) &
       nir_resource_intel_bindless) != 0;
   const bool is_sampler =
      (nir_intrinsic_resource_access_intel(intrin) &
       nir_resource_intel_sampler) != 0;
   const struct lower_resource_state *state = data;

   if (!is_bindless)
      return true;

   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *set_offset = intrin->src[0].ssa;
   nir_def *binding_offset = intrin->src[1].ssa;

   /* When using indirect descriptor, the surface handles are loaded from the
    * descriptor buffer and do not need any offset.
    */
   if (state->desc_type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT) {
      if (!state->device->uses_ex_bso) {
         /* We're trying to reduce the number of instructions in the shaders
          * to compute surface handles. The assumption is that we're using
          * more surface handles than sampler handles (UBO, SSBO, images,
          * etc...) so it's worth optimizing that case.
          *
          * Surface handles in the extended descriptor message have to be
          * shifted left by 6 prior to ex_bso (bits 31:12 in extended
          * descriptor, match bits 25:6 of the surface handle). We have to
          * combine 2 parts in the shader to build the final surface handle,
          * base offset of the descriptor set (in the push constant, located
          * in resource_intel::src[0]) and the relative descriptor offset
          * (resource_intel::src[1]).
          *
          * For convenience, up to here, resource_intel::src[1] is in bytes.
          * We now have to shift it left by 6 to match the shifted left by 6
          * done for the push constant value provided in
          * resource_intel::src[0]. That way the shader can just do a single
          * ADD and get the surface handle.
          *
          * Samplers have a 4Gb heap and in the message they're in bits 31:6
          * of the component 3 of the sampler message header. But since we
          * push only a single offset for the base offset of the descriptor
          * set, resource_intel::src[0] has to be shifted right by 6 (bringing
          * it back in bytes).
          */
         if (!is_sampler)
            binding_offset = nir_ishl_imm(b, binding_offset, 6);
      }

      nir_src_rewrite(&intrin->src[1],
                      nir_iadd(b, set_offset, binding_offset));
   }

   /* Now unused values : set offset, array index */
   nir_src_rewrite(&intrin->src[0], nir_imm_int(b, 0xdeaddeed));
   nir_src_rewrite(&intrin->src[2], nir_imm_int(b, 0xdeaddeed));

   return true;
}

bool
anv_nir_lower_resource_intel(nir_shader *shader,
                             const struct anv_physical_device *device,
                             enum anv_descriptor_set_layout_type desc_type)
{
   struct lower_resource_state state = {
      .desc_type = desc_type,
      .device = device,
   };
   return nir_shader_intrinsics_pass(shader, lower_resource_intel,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       &state);
}
