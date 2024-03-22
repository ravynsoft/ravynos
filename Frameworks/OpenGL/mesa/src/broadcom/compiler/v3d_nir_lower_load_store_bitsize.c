/*
 * Copyright © 2021 Raspberry Pi Ltd
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

#include "compiler/v3d_compiler.h"
#include "compiler/nir/nir_builder.h"

/**
 * The V3D TMU unit can only do 32-bit general vector access so for anything
 * else we need to split vector load/store instructions to scalar.
 *
 * Note that a vectorization pass after this lowering may be able to
 * re-vectorize some of these using 32-bit load/store instructions instead,
 * which we do support.
 */

static int
value_src(nir_intrinsic_op intrinsic)
{
   switch (intrinsic) {
   case nir_intrinsic_store_ssbo:
   case nir_intrinsic_store_scratch:
   case nir_intrinsic_store_global_2x32:
      return 0;
   default:
      unreachable("Unsupported intrinsic");
   }
}

static int
offset_src(nir_intrinsic_op intrinsic)
{
   switch (intrinsic) {
   case nir_intrinsic_load_uniform:
   case nir_intrinsic_load_shared:
   case nir_intrinsic_load_scratch:
   case nir_intrinsic_load_global_2x32:
      return 0;
   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_store_scratch:
   case nir_intrinsic_store_global_2x32:
      return 1;
   case nir_intrinsic_store_ssbo:
      return 2;
   default:
      unreachable("Unsupported intrinsic");
   }
}

static nir_intrinsic_instr *
init_scalar_intrinsic(nir_builder *b,
                      nir_intrinsic_instr *intr,
                      uint32_t component,
                      nir_def *offset,
                      uint32_t bit_size,
                      nir_def **scalar_offset)
{

        nir_intrinsic_instr *new_intr =
                nir_intrinsic_instr_create(b->shader, intr->intrinsic);

        nir_intrinsic_copy_const_indices(new_intr, intr);

        const int offset_units = bit_size / 8;
        assert(offset_units >= 1);

        if (nir_intrinsic_has_align_mul(intr)) {
                assert(nir_intrinsic_has_align_offset(intr));
                unsigned align_mul = nir_intrinsic_align_mul(intr);
                unsigned align_off = nir_intrinsic_align_offset(intr);

                align_off += offset_units * component;
                align_off = align_off % align_mul;

                nir_intrinsic_set_align(new_intr, align_mul, align_off);
        }

        *scalar_offset = offset;
        unsigned offset_adj = offset_units * component;
        if (nir_intrinsic_has_base(intr)) {
                nir_intrinsic_set_base(
                        new_intr, nir_intrinsic_base(intr) + offset_adj);
        } else {
                *scalar_offset =
                        nir_iadd(b, offset,
                                 nir_imm_intN_t(b, offset_adj,
                                                offset->bit_size));
        }

        new_intr->num_components = 1;

        return new_intr;
}

static bool
lower_load_bitsize(nir_builder *b,
                   nir_intrinsic_instr *intr)
{
        uint32_t bit_size = intr->def.bit_size;
        if (bit_size == 32)
                return false;

        /* No need to split if it is already scalar */
        int num_comp = nir_intrinsic_dest_components(intr);
        if (num_comp <= 1)
                return false;

        b->cursor = nir_before_instr(&intr->instr);

        /* For global 2x32 we ignore Y component because it must be zero */
        unsigned offset_idx = offset_src(intr->intrinsic);
        nir_def *offset = nir_trim_vector(b, intr->src[offset_idx].ssa, 1);

        /* Split vector store to multiple scalar loads */
        nir_def *dest_components[4] = { NULL };
        const nir_intrinsic_info *info = &nir_intrinsic_infos[intr->intrinsic];
        for (int component = 0; component < num_comp; component++) {
                nir_def *scalar_offset;
                nir_intrinsic_instr *new_intr =
                        init_scalar_intrinsic(b, intr, component, offset,
                                              bit_size, &scalar_offset);

                for (unsigned i = 0; i < info->num_srcs; i++) {
                        if (i == offset_idx) {
                                nir_def *final_offset;
                                final_offset = intr->intrinsic != nir_intrinsic_load_global_2x32 ?
                                        scalar_offset :
                                        nir_vec2(b, scalar_offset,
                                                 nir_imm_int(b, 0));
                                new_intr->src[i] = nir_src_for_ssa(final_offset);
                        } else {
                                new_intr->src[i] = intr->src[i];
                        }
                }

                nir_def_init(&new_intr->instr, &new_intr->def, 1,
                             bit_size);
                dest_components[component] = &new_intr->def;

                nir_builder_instr_insert(b, &new_intr->instr);
        }

        nir_def *new_dst = nir_vec(b, dest_components, num_comp);
        nir_def_rewrite_uses(&intr->def, new_dst);

        nir_instr_remove(&intr->instr);
        return true;
}

static bool
lower_store_bitsize(nir_builder *b,
                    nir_intrinsic_instr *intr)
{
        /* No need to split if it is already scalar */
        int value_idx = value_src(intr->intrinsic);
        int num_comp = nir_intrinsic_src_components(intr, value_idx);
        if (num_comp <= 1)
                return false;

        /* No need to split if it is 32-bit */
        if (nir_src_bit_size(intr->src[value_idx]) == 32)
                return false;

        nir_def *value = intr->src[value_idx].ssa;

        b->cursor = nir_before_instr(&intr->instr);

        /* For global 2x32 we ignore Y component because it must be zero */
        unsigned offset_idx = offset_src(intr->intrinsic);
        nir_def *offset = nir_trim_vector(b, intr->src[offset_idx].ssa, 1);

        /* Split vector store to multiple scalar stores */
        const nir_intrinsic_info *info = &nir_intrinsic_infos[intr->intrinsic];
        unsigned wrmask = nir_intrinsic_write_mask(intr);
        while (wrmask) {
                unsigned component = ffs(wrmask) - 1;

                nir_def *scalar_offset;
                nir_intrinsic_instr *new_intr =
                        init_scalar_intrinsic(b, intr, component, offset,
                                              value->bit_size, &scalar_offset);

                nir_intrinsic_set_write_mask(new_intr, 0x1);

                for (unsigned i = 0; i < info->num_srcs; i++) {
                        if (i == value_idx) {
                                nir_def *scalar_value =
                                        nir_channels(b, value, 1 << component);
                                new_intr->src[i] = nir_src_for_ssa(scalar_value);
                        } else if (i == offset_idx) {
                                nir_def *final_offset;
                                final_offset = intr->intrinsic != nir_intrinsic_store_global_2x32 ?
                                        scalar_offset :
                                        nir_vec2(b, scalar_offset,
                                                 nir_imm_int(b, 0));
                                new_intr->src[i] = nir_src_for_ssa(final_offset);
                        } else {
                                new_intr->src[i] = intr->src[i];
                        }
                }

                nir_builder_instr_insert(b, &new_intr->instr);

                wrmask &= ~(1 << component);
        }

        nir_instr_remove(&intr->instr);
        return true;
}

static bool
lower_load_store_bitsize(nir_builder *b, nir_intrinsic_instr *intr,
                         void *data)
{
        switch (intr->intrinsic) {
        case nir_intrinsic_load_ssbo:
        case nir_intrinsic_load_ubo:
        case nir_intrinsic_load_uniform:
        case nir_intrinsic_load_scratch:
        case nir_intrinsic_load_global_2x32:
               return lower_load_bitsize(b, intr);

        case nir_intrinsic_store_ssbo:
        case nir_intrinsic_store_scratch:
        case nir_intrinsic_store_global_2x32:
                return lower_store_bitsize(b, intr);

        default:
                return false;
        }
}

bool
v3d_nir_lower_load_store_bitsize(nir_shader *s)
{
        return nir_shader_intrinsics_pass(s, lower_load_store_bitsize,
                                            nir_metadata_block_index |
                                            nir_metadata_dominance,
                                            NULL);
}
