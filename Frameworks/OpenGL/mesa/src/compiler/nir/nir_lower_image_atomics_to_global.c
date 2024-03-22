/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "util/format/u_format.h"
#include "nir_builder.h"

/*
 * If shader images are uncompressed, dedicated image atomics are unnecessary.
 * Instead, there may be a "load texel address" instruction that does all the
 * addressing math, and then regular global atomics may be used with the
 * calculated address. This pass lowers image atomics to image_texel_address +
 * global atomics.
 */

static bool
lower(nir_builder *b, nir_intrinsic_instr *intr, UNUSED void *_)
{
   nir_intrinsic_op address_op;
   bool swap;

#define CASE(storage)                                                  \
   case nir_intrinsic_##storage##_atomic:                              \
   case nir_intrinsic_##storage##_atomic_swap:                         \
      address_op = nir_intrinsic_##storage##_texel_address;            \
      swap = intr->intrinsic == nir_intrinsic_##storage##_atomic_swap; \
      break;

   switch (intr->intrinsic) {
      CASE(image)
      CASE(bindless_image)
      CASE(image_deref)
   default:
      return false;
   }
#undef CASE

   b->cursor = nir_before_instr(&intr->instr);
   nir_atomic_op atomic_op = nir_intrinsic_atomic_op(intr);
   enum pipe_format format = nir_intrinsic_format(intr);
   unsigned bit_size = intr->def.bit_size;

   /* Even for "formatless" access, we know the size of the texel accessed,
    * since it's the size of the atomic. We can use that to synthesize a
    * compatible format, which is good enough for texel address computations.
    */
   if (format == PIPE_FORMAT_NONE) {
      nir_alu_type type_ = nir_atomic_op_type(atomic_op);
      enum util_format_type format_type;
      if (type_ == nir_type_float)
         format_type = UTIL_FORMAT_TYPE_FLOAT;
      else if (type_ == nir_type_int)
         format_type = UTIL_FORMAT_TYPE_SIGNED;
      else
         format_type = UTIL_FORMAT_TYPE_UNSIGNED;

      format = util_format_get_array(format_type, bit_size, 1, false,
                                     type_ != nir_type_float);
   }

   /* Get the relevant texel address */
   nir_def *address = nir_image_texel_address(
      b, 64, intr->src[0].ssa, intr->src[1].ssa, intr->src[2].ssa,
      .image_dim = nir_intrinsic_image_dim(intr),
      .image_array = nir_intrinsic_image_array(intr),
      .format = format,
      .access = nir_intrinsic_access(intr));

   nir_instr *address_instr = address->parent_instr;
   nir_intrinsic_instr *address_intr = nir_instr_as_intrinsic(address_instr);

   address_intr->intrinsic = address_op;
   if (address_op == nir_intrinsic_image_texel_address) {
      nir_intrinsic_set_range_base(address_intr,
                                   nir_intrinsic_range_base(intr));
   }

   /* Build the global atomic */
   nir_def *global;
   if (swap) {
      global = nir_global_atomic_swap(b, bit_size, address, intr->src[3].ssa,
                                      intr->src[4].ssa, .atomic_op = atomic_op);
   } else {
      global = nir_global_atomic(b, bit_size, address, intr->src[3].ssa,
                                 .atomic_op = atomic_op);
   }

   /* Replace the image atomic with the global atomic. Remove the image
    * explicitly because it has side effects so is not DCE'd.
    */
   nir_def_rewrite_uses(&intr->def, global);
   nir_instr_remove(&intr->instr);
   return true;
}

bool
nir_lower_image_atomics_to_global(nir_shader *shader)
{
   return nir_shader_intrinsics_pass(shader, lower,
                                     nir_metadata_block_index |
                                        nir_metadata_dominance,
                                     NULL);
}
