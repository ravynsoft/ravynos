/*
 * Copyright © 2018 Intel Corporation
 * Copyright © 2023 Collabora, Ltd.
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

#include "util/bitscan.h"
#include "util/u_math.h"
#include "nir_builder.h"

static nir_intrinsic_instr *
dup_mem_intrinsic(nir_builder *b, nir_intrinsic_instr *intrin,
                  nir_def *offset,
                  unsigned align_mul, unsigned align_offset,
                  nir_def *data,
                  unsigned num_components, unsigned bit_size)
{
   const nir_intrinsic_info *info = &nir_intrinsic_infos[intrin->intrinsic];

   nir_intrinsic_instr *dup =
      nir_intrinsic_instr_create(b->shader, intrin->intrinsic);

   nir_src *intrin_offset_src = nir_get_io_offset_src(intrin);
   for (unsigned i = 0; i < info->num_srcs; i++) {
      if (i == 0 && data != NULL) {
         assert(!info->has_dest);
         assert(&intrin->src[i] != intrin_offset_src);
         dup->src[i] = nir_src_for_ssa(data);
      } else if (&intrin->src[i] == intrin_offset_src) {
         dup->src[i] = nir_src_for_ssa(offset);
      } else {
         dup->src[i] = nir_src_for_ssa(intrin->src[i].ssa);
      }
   }

   dup->num_components = num_components;
   for (unsigned i = 0; i < info->num_indices; i++)
      dup->const_index[i] = intrin->const_index[i];

   nir_intrinsic_set_align(dup, align_mul, align_offset);

   if (info->has_dest) {
      nir_def_init(&dup->instr, &dup->def, num_components, bit_size);
   } else {
      nir_intrinsic_set_write_mask(dup, (1 << num_components) - 1);
   }

   nir_builder_instr_insert(b, &dup->instr);

   return dup;
}

static bool
lower_mem_load(nir_builder *b, nir_intrinsic_instr *intrin,
               nir_lower_mem_access_bit_sizes_cb mem_access_size_align_cb,
               const void *cb_data)
{
   const unsigned bit_size = intrin->def.bit_size;
   const unsigned num_components = intrin->def.num_components;
   const unsigned bytes_read = num_components * (bit_size / 8);
   const uint32_t align_mul = nir_intrinsic_align_mul(intrin);
   const uint32_t whole_align_offset = nir_intrinsic_align_offset(intrin);
   const uint32_t whole_align = nir_intrinsic_align(intrin);
   nir_src *offset_src = nir_get_io_offset_src(intrin);
   const bool offset_is_const = nir_src_is_const(*offset_src);
   nir_def *offset = offset_src->ssa;

   nir_mem_access_size_align requested =
      mem_access_size_align_cb(intrin->intrinsic, bytes_read,
                               bit_size, align_mul, whole_align_offset,
                               offset_is_const, cb_data);

   assert(util_is_power_of_two_nonzero(align_mul));
   assert(util_is_power_of_two_nonzero(requested.align));
   if (requested.num_components == num_components &&
       requested.bit_size == bit_size &&
       requested.align <= whole_align)
      return false;

   /* Otherwise, we have to break it into chunks.  We could end up with as
    * many as 32 chunks if we're loading a u64vec16 as individual dwords.
    */
   nir_def *chunks[32];
   unsigned num_chunks = 0;
   unsigned chunk_start = 0;
   while (chunk_start < bytes_read) {
      const unsigned bytes_left = bytes_read - chunk_start;
      const uint32_t chunk_align_offset =
         (whole_align_offset + chunk_start) % align_mul;
      const uint32_t chunk_align =
         nir_combined_align(align_mul, chunk_align_offset);
      requested = mem_access_size_align_cb(intrin->intrinsic, bytes_left,
                                           bit_size, align_mul, chunk_align_offset,
                                           offset_is_const, cb_data);

      unsigned chunk_bytes;
      assert(util_is_power_of_two_nonzero(requested.align));
      if (align_mul < requested.align) {
         /* For this case, we need to be able to shift the value so we assume
          * the alignment is less than the size of a single component.  This
          * ensures that we don't need to upcast in order to shift.
          */
         assert(requested.bit_size >= requested.align * 8);

         uint64_t align_mask = requested.align - 1;
         nir_def *chunk_offset = nir_iadd_imm(b, offset, chunk_start);
         nir_def *pad = nir_iand_imm(b, chunk_offset, align_mask);
         chunk_offset = nir_iand_imm(b, chunk_offset, ~align_mask);

         nir_intrinsic_instr *load =
            dup_mem_intrinsic(b, intrin, chunk_offset,
                              requested.align, 0, NULL,
                              requested.num_components, requested.bit_size);

         unsigned max_pad = requested.align - chunk_align;
         unsigned requested_bytes =
            requested.num_components * requested.bit_size / 8;
         chunk_bytes = MIN2(bytes_left, requested_bytes - max_pad);

         nir_def *shift = nir_imul_imm(b, pad, 8);
         nir_def *shifted = nir_ushr(b, &load->def, shift);

         if (load->def.num_components > 1) {
            nir_def *rev_shift =
               nir_isub_imm(b, load->def.bit_size, shift);
            nir_def *rev_shifted = nir_ishl(b, &load->def, rev_shift);

            nir_def *comps[NIR_MAX_VEC_COMPONENTS];
            for (unsigned i = 1; i < load->def.num_components; i++)
               comps[i - 1] = nir_channel(b, rev_shifted, i);

            comps[load->def.num_components - 1] =
               nir_imm_zero(b, 1, load->def.bit_size);

            rev_shifted = nir_vec(b, comps, load->def.num_components);
            shifted = nir_bcsel(b, nir_ieq_imm(b, shift, 0), &load->def,
                                nir_ior(b, shifted, rev_shifted));
         }

         unsigned chunk_bit_size = MIN2(8 << (ffs(chunk_bytes) - 1), bit_size);
         unsigned chunk_num_components = chunk_bytes / (chunk_bit_size / 8);

         /* There's no guarantee that chunk_num_components is a valid NIR
          * vector size, so just loop one chunk component at a time
          */
         for (unsigned i = 0; i < chunk_num_components; i++) {
            assert(num_chunks < ARRAY_SIZE(chunks));
            chunks[num_chunks++] =
               nir_extract_bits(b, &shifted, 1, i * chunk_bit_size,
                                1, chunk_bit_size);
         }
      } else if (chunk_align_offset % requested.align) {
         /* In this case, we know how much to adjust the offset */
         uint32_t delta = chunk_align_offset % requested.align;
         nir_def *load_offset =
            nir_iadd_imm(b, offset, chunk_start - (int)delta);

         const uint32_t load_align_offset =
            (chunk_align_offset - delta) % align_mul;

         nir_intrinsic_instr *load =
            dup_mem_intrinsic(b, intrin, load_offset,
                              align_mul, load_align_offset, NULL,
                              requested.num_components, requested.bit_size);

         assert(requested.bit_size >= 8);
         chunk_bytes = requested.num_components * (requested.bit_size / 8);
         assert(chunk_bytes > delta);
         chunk_bytes -= delta;

         unsigned chunk_bit_size = MIN2(8 << (ffs(chunk_bytes) - 1), bit_size);
         unsigned chunk_num_components = chunk_bytes / (chunk_bit_size / 8);

         /* There's no guarantee that chunk_num_components is a valid NIR
          * vector size, so just loop one chunk component at a time
          */
         nir_def *chunk_data = &load->def;
         for (unsigned i = 0; i < chunk_num_components; i++) {
            assert(num_chunks < ARRAY_SIZE(chunks));
            chunks[num_chunks++] =
               nir_extract_bits(b, &chunk_data, 1,
                                delta * 8 + i * chunk_bit_size,
                                1, chunk_bit_size);
         }
      } else {
         nir_def *chunk_offset = nir_iadd_imm(b, offset, chunk_start);
         nir_intrinsic_instr *load =
            dup_mem_intrinsic(b, intrin, chunk_offset,
                              align_mul, chunk_align_offset, NULL,
                              requested.num_components, requested.bit_size);

         chunk_bytes = requested.num_components * (requested.bit_size / 8);
         assert(num_chunks < ARRAY_SIZE(chunks));
         chunks[num_chunks++] = &load->def;
      }

      chunk_start += chunk_bytes;
   }

   nir_def *result = nir_extract_bits(b, chunks, num_chunks, 0,
                                      num_components, bit_size);
   nir_def_rewrite_uses(&intrin->def, result);
   nir_instr_remove(&intrin->instr);

   return true;
}

static bool
lower_mem_store(nir_builder *b, nir_intrinsic_instr *intrin,
                nir_lower_mem_access_bit_sizes_cb mem_access_size_align_cb,
                const void *cb_data, bool allow_unaligned_stores_as_atomics)
{
   nir_def *value = intrin->src[0].ssa;

   assert(intrin->num_components == value->num_components);
   const unsigned bit_size = value->bit_size;
   const unsigned byte_size = bit_size / 8;
   const unsigned num_components = intrin->num_components;
   const unsigned bytes_written = num_components * byte_size;
   const uint32_t align_mul = nir_intrinsic_align_mul(intrin);
   const uint32_t whole_align_offset = nir_intrinsic_align_offset(intrin);
   const uint32_t whole_align = nir_intrinsic_align(intrin);
   nir_src *offset_src = nir_get_io_offset_src(intrin);
   const bool offset_is_const = nir_src_is_const(*offset_src);
   nir_def *offset = offset_src->ssa;

   nir_component_mask_t writemask = nir_intrinsic_write_mask(intrin);
   assert(writemask < (1 << num_components));

   nir_mem_access_size_align requested =
      mem_access_size_align_cb(intrin->intrinsic, bytes_written,
                               bit_size, align_mul, whole_align_offset,
                               offset_is_const, cb_data);

   assert(util_is_power_of_two_nonzero(align_mul));
   assert(util_is_power_of_two_nonzero(requested.align));
   if (requested.num_components == num_components &&
       requested.bit_size == bit_size &&
       requested.align <= whole_align &&
       writemask == BITFIELD_MASK(num_components))
      return false;

   assert(byte_size <= sizeof(uint64_t));
   BITSET_DECLARE(mask, NIR_MAX_VEC_COMPONENTS * sizeof(uint64_t));
   BITSET_ZERO(mask);

   for (unsigned i = 0; i < num_components; i++) {
      if (writemask & (1u << i)) {
         BITSET_SET_RANGE_INSIDE_WORD(mask, i * byte_size,
                                      ((i + 1) * byte_size) - 1);
      }
   }

   while (BITSET_FFS(mask) != 0) {
      const uint32_t chunk_start = BITSET_FFS(mask) - 1;

      uint32_t end;
      for (end = chunk_start + 1; end < bytes_written; end++) {
         if (!(BITSET_TEST(mask, end)))
            break;
      }
      /* The size of the current contiguous chunk in bytes */
      const uint32_t max_chunk_bytes = end - chunk_start;
      const uint32_t chunk_align_offset =
         (whole_align_offset + chunk_start) % align_mul;
      const uint32_t chunk_align =
         nir_combined_align(align_mul, chunk_align_offset);

      requested = mem_access_size_align_cb(intrin->intrinsic, max_chunk_bytes,
                                           bit_size, align_mul, chunk_align_offset,
                                           offset_is_const, cb_data);

      uint32_t chunk_bytes = requested.num_components * (requested.bit_size / 8);

      assert(util_is_power_of_two_nonzero(requested.align));
      if (chunk_align < requested.align ||
          chunk_bytes > max_chunk_bytes) {
         /* Otherwise the caller made a mistake with their return values. */
         assert(chunk_bytes <= 4);
         assert(allow_unaligned_stores_as_atomics);

         /* We'll turn this into a pair of 32-bit atomics to modify only the right
          * bits of memory.
          */
         requested = (nir_mem_access_size_align){
            .align = 4,
            .bit_size = 32,
            .num_components = 1,
         };

         uint64_t align_mask = requested.align - 1;
         nir_def *chunk_offset = nir_iadd_imm(b, offset, chunk_start);
         nir_def *pad = chunk_align < 4 ? nir_iand_imm(b, chunk_offset, align_mask) : nir_imm_intN_t(b, 0, chunk_offset->bit_size);
         chunk_offset = nir_iand_imm(b, chunk_offset, ~align_mask);

         unsigned max_pad = chunk_align < requested.align ? requested.align - chunk_align : 0;
         unsigned requested_bytes =
            requested.num_components * requested.bit_size / 8;
         chunk_bytes = MIN2(max_chunk_bytes, requested_bytes - max_pad);
         unsigned chunk_bits = chunk_bytes * 8;

         nir_def *data;
         if (chunk_bits == 24) {
            /* This is a bit of a special case because we don't have 24-bit integers */
            data = nir_extract_bits(b, &value, 1, chunk_start * 8, 3, 8);
            data = nir_pack_bits(b, nir_pad_vector_imm_int(b, data, 0, 4), 32);
         } else {
            data = nir_extract_bits(b, &value, 1, chunk_start * 8, 1, chunk_bits);
            data = nir_u2u32(b, data);
         }

         nir_def *iand_mask = nir_imm_int(b, (1 << chunk_bits) - 1);

         if (chunk_align < requested.align) {
            nir_def *shift = nir_u2u32(b, nir_imul_imm(b, pad, 8));
            data = nir_ishl(b, data, shift);
            iand_mask = nir_ishl(b, iand_mask, shift);
         }

         iand_mask = nir_inot(b, iand_mask);

         switch (intrin->intrinsic) {
         case nir_intrinsic_store_ssbo:
            nir_ssbo_atomic(b, 32, intrin->src[1].ssa, chunk_offset, iand_mask,
                            .atomic_op = nir_atomic_op_iand,
                            .access = nir_intrinsic_access(intrin));
            nir_ssbo_atomic(b, 32, intrin->src[1].ssa, chunk_offset, data,
                            .atomic_op = nir_atomic_op_ior,
                            .access = nir_intrinsic_access(intrin));
            break;
         case nir_intrinsic_store_global:
            nir_global_atomic(b, 32, chunk_offset, iand_mask,
                              .atomic_op = nir_atomic_op_iand);
            nir_global_atomic(b, 32, chunk_offset, data,
                              .atomic_op = nir_atomic_op_ior);
            break;
         case nir_intrinsic_store_shared:
            nir_shared_atomic(b, 32, chunk_offset, iand_mask,
                              .atomic_op = nir_atomic_op_iand,
                              .base = nir_intrinsic_base(intrin));
            nir_shared_atomic(b, 32, chunk_offset, data,
                              .atomic_op = nir_atomic_op_ior,
                              .base = nir_intrinsic_base(intrin));
            break;
         default:
            unreachable("Unsupported unaligned store");
         }
      } else {
         nir_def *packed = nir_extract_bits(b, &value, 1, chunk_start * 8,
                                            requested.num_components,
                                            requested.bit_size);

         nir_def *chunk_offset = nir_iadd_imm(b, offset, chunk_start);
         dup_mem_intrinsic(b, intrin, chunk_offset,
                           align_mul, chunk_align_offset, packed,
                           requested.num_components, requested.bit_size);
      }
      BITSET_CLEAR_RANGE(mask, chunk_start, (chunk_start + chunk_bytes - 1));
   }

   nir_instr_remove(&intrin->instr);

   return true;
}

static nir_variable_mode
intrin_to_variable_mode(nir_intrinsic_op intrin)
{
   switch (intrin) {
   case nir_intrinsic_load_ubo:
      return nir_var_mem_ubo;

   case nir_intrinsic_load_global:
   case nir_intrinsic_store_global:
      return nir_var_mem_global;

   case nir_intrinsic_load_global_constant:
      return nir_var_mem_constant;

   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_store_ssbo:
      return nir_var_mem_ssbo;

   case nir_intrinsic_load_shared:
   case nir_intrinsic_store_shared:
      return nir_var_mem_shared;

   case nir_intrinsic_load_scratch:
   case nir_intrinsic_store_scratch:
      return nir_var_shader_temp | nir_var_function_temp;

   case nir_intrinsic_load_task_payload:
   case nir_intrinsic_store_task_payload:
      return nir_var_mem_task_payload;

   default:
      return 0;
   }
}

static bool
lower_mem_access_instr(nir_builder *b, nir_instr *instr, void *_data)
{
   const nir_lower_mem_access_bit_sizes_options *state = _data;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   if (!(state->modes & intrin_to_variable_mode(intrin->intrinsic)))
      return false;

   b->cursor = nir_after_instr(instr);

   switch (intrin->intrinsic) {
   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_global:
   case nir_intrinsic_load_global_constant:
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_load_shared:
   case nir_intrinsic_load_scratch:
   case nir_intrinsic_load_task_payload:
      return lower_mem_load(b, intrin, state->callback, state->cb_data);

   case nir_intrinsic_store_global:
   case nir_intrinsic_store_ssbo:
   case nir_intrinsic_store_shared:
   case nir_intrinsic_store_scratch:
   case nir_intrinsic_store_task_payload:
      return lower_mem_store(b, intrin, state->callback, state->cb_data,
                             state->may_lower_unaligned_stores_to_atomics);

   default:
      return false;
   }
}

bool
nir_lower_mem_access_bit_sizes(nir_shader *shader,
                               const nir_lower_mem_access_bit_sizes_options *options)
{
   return nir_shader_instructions_pass(shader, lower_mem_access_instr,
                                       nir_metadata_block_index |
                                          nir_metadata_dominance,
                                       (void *)options);
}
