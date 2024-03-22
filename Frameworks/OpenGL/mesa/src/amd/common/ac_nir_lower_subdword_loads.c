/*
 * Copyright © 2022 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/* Convert 8-bit and 16-bit loads to 32 bits. This is for drivers that don't
 * support non-32-bit loads.
 *
 * This pass only transforms load intrinsics lowered by nir_lower_explicit_io,
 * so this pass should run after it.
 *
 * nir_opt_load_store_vectorize should be run before this because it analyzes
 * offset calculations and recomputes align_mul and align_offset.
 *
 * nir_opt_algebraic and (optionally) ALU scalarization are recommended to be
 * run after this.
 *
 * Running nir_opt_load_store_vectorize after this pass may lead to further
 * vectorization, e.g. adjacent 2x16-bit and 1x32-bit loads will become
 * 2x32-bit loads.
 */

#include "util/u_math.h"
#include "ac_nir.h"

static bool
lower_subdword_loads(nir_builder *b, nir_instr *instr, void *data)
{
   ac_nir_lower_subdword_options *options = data;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   unsigned num_components = intr->num_components;
   nir_variable_mode modes =
      num_components == 1 ? options->modes_1_comp
                          : options->modes_N_comps;

   switch (intr->intrinsic) {
   case nir_intrinsic_load_ubo:
      if (!(modes & nir_var_mem_ubo))
         return false;
      break;
   case nir_intrinsic_load_ssbo:
      if (!(modes & nir_var_mem_ssbo))
         return false;
      break;
   case nir_intrinsic_load_global:
      if (!(modes & nir_var_mem_global))
         return false;
      break;
   default:
      return false;
   }

   unsigned bit_size = intr->def.bit_size;
   if (bit_size >= 32)
      return false;

   assert(bit_size == 8 || bit_size == 16);

   unsigned component_size = bit_size / 8;
   unsigned comp_per_dword = 4 / component_size;

   /* Get the offset alignment relative to the closest dword. */
   unsigned align_mul = MIN2(nir_intrinsic_align_mul(intr), 4);
   unsigned align_offset = nir_intrinsic_align_offset(intr) % align_mul;

   nir_src *src_offset = nir_get_io_offset_src(intr);
   nir_def *offset = src_offset->ssa;
   nir_def *result = &intr->def;

   /* Change the load to 32 bits per channel, update the channel count,
    * and increase the declared load alignment.
    */
   intr->def.bit_size = 32;

   if (align_mul == 4 && align_offset == 0) {
      intr->num_components = intr->def.num_components =
         DIV_ROUND_UP(num_components, comp_per_dword);

      /* Aligned loads. Just bitcast the vector and trim it if there are
       * trailing unused elements.
       */
      b->cursor = nir_after_instr(instr);
      result = nir_extract_bits(b, &result, 1, 0, num_components, bit_size);

      nir_def_rewrite_uses_after(&intr->def, result,
                                     result->parent_instr);
      return true;
   }

   /* Multi-component unaligned loads may straddle the dword boundary.
    * E.g. for 2 components, we need to load an extra dword, and so on.
    */
   intr->num_components = intr->def.num_components =
      DIV_ROUND_UP(4 - align_mul + align_offset + num_components * component_size, 4);

   nir_intrinsic_set_align(intr,
                           MAX2(nir_intrinsic_align_mul(intr), 4),
                           nir_intrinsic_align_offset(intr) & ~0x3);

   if (align_mul == 4) {
      /* Unaligned loads with an aligned non-constant base offset (which is
       * X * align_mul) and a constant added offset (align_offset).
       */
      assert(align_offset <= 3);
      assert(align_offset % component_size == 0);
      unsigned comp_offset = align_offset / component_size;

      /* There is a good probability that the offset is "iadd" adding
       * align_offset. Subtracting align_offset should eliminate it.
       */
      b->cursor = nir_before_instr(instr);
      nir_src_rewrite(src_offset, nir_iadd_imm(b, offset, -align_offset));

      b->cursor = nir_after_instr(instr);
      result = nir_extract_bits(b, &result, 1, comp_offset * bit_size,
                                num_components, bit_size);

      nir_def_rewrite_uses_after(&intr->def, result,
                                     result->parent_instr);
      return true;
   }

   /* Fully unaligned loads. We overfetch by up to 1 dword and then bitshift
    * the whole vector.
    */
   assert(align_mul <= 2 && align_offset <= 3);

   /* Round down by masking out the bits. */
   b->cursor = nir_before_instr(instr);
   nir_src_rewrite(src_offset, nir_iand_imm(b, offset, ~0x3));

   /* We need to shift bits in the loaded vector by this number. */
   b->cursor = nir_after_instr(instr);
   nir_def *shift = nir_ishl_imm(b, nir_iand_imm(b, offset, 0x3), 3);
   nir_def *rev_shift32 = nir_isub_imm(b, 32, shift);

   nir_def *elems[NIR_MAX_VEC_COMPONENTS];

   /* "shift" can be only be one of: 0, 8, 16, 24
    *
    * When we shift by (32 - shift) and shift is 0, resulting in a shift by 32,
    * which is the same as a shift by 0, we need to convert the shifted number
    * to u64 to get the shift by 32 that we want.
    *
    * The following algorithms are used to shift the vector.
    *
    * 64-bit variant (shr64 + shl64 + or32 per 2 elements):
    *    for (i = 0; i < num_components / 2 - 1; i++) {
    *       qword1 = pack(src[i * 2 + 0], src[i * 2 + 1]) >> shift;
    *       dword2 = u2u32(u2u64(src[i * 2 + 2]) << (32 - shift));
    *       dst[i * 2 + 0] = unpack_64_2x32_x(qword1);
    *       dst[i * 2 + 1] = unpack_64_2x32_y(qword1) | dword2;
    *    }
    *    i *= 2;
    *
    * 32-bit variant (shr32 + shl64 + or32 per element):
    *    for (; i < num_components - 1; i++)
    *       dst[i] = (src[i] >> shift) |
    *                u2u32(u2u64(src[i + 1]) << (32 - shift));
    */
   unsigned i = 0;

   if (intr->num_components >= 2) {
      /* Use the 64-bit algorithm as described above. */
      for (i = 0; i < intr->num_components / 2 - 1; i++) {
         nir_def *qword1, *dword2;

         qword1 = nir_pack_64_2x32_split(b,
                                         nir_channel(b, result, i * 2 + 0),
                                         nir_channel(b, result, i * 2 + 1));
         qword1 = nir_ushr(b, qword1, shift);
         dword2 = nir_ishl(b, nir_u2u64(b, nir_channel(b, result, i * 2 + 2)),
                           rev_shift32);
         dword2 = nir_u2u32(b, dword2);

         elems[i * 2 + 0] = nir_unpack_64_2x32_split_x(b, qword1);
         elems[i * 2 + 1] =
            nir_ior(b, nir_unpack_64_2x32_split_y(b, qword1), dword2);
      }
      i *= 2;

      /* Use the 32-bit algorithm for the remainder of the vector. */
      for (; i < intr->num_components - 1; i++) {
         elems[i] =
            nir_ior(b,
                    nir_ushr(b, nir_channel(b, result, i), shift),
                    nir_u2u32(b,
                        nir_ishl(b, nir_u2u64(b, nir_channel(b, result, i + 1)),
                                 rev_shift32)));
      }
   }

   /* Shift the last element. */
   elems[i] = nir_ushr(b, nir_channel(b, result, i), shift);

   result = nir_vec(b, elems, intr->num_components);
   result = nir_extract_bits(b, &result, 1, 0, num_components, bit_size);

   nir_def_rewrite_uses_after(&intr->def, result,
                                  result->parent_instr);
   return true;
}

bool
ac_nir_lower_subdword_loads(nir_shader *nir, ac_nir_lower_subdword_options options)
{
   return nir_shader_instructions_pass(nir, lower_subdword_loads,
                                       nir_metadata_dominance |
                                       nir_metadata_block_index, &options);
}
