/*
 * Copyright © 2018 Intel Corporation
 * Copyright © 2018 Broadcom
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

#include "v3d_compiler.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_format_convert.h"

/** @file v3d_nir_lower_scratch.c
 *
 * Swizzles around the addresses of
 * nir_intrinsic_load_scratch/nir_intrinsic_store_scratch so that a QPU stores
 * a cacheline at a time per dword of scratch access, scalarizing and removing
 * writemasks in the process.
 */

static nir_def *
v3d_nir_scratch_offset(nir_builder *b, nir_intrinsic_instr *instr)
{
        bool is_store = instr->intrinsic == nir_intrinsic_store_scratch;
        nir_def *offset = instr->src[is_store ? 1 : 0].ssa;

        assert(nir_intrinsic_align_mul(instr) >= 4);
        assert(nir_intrinsic_align_offset(instr) == 0);

        /* The spill_offset register will already have the subgroup ID (EIDX)
         * shifted and ORed in at bit 2, so all we need to do is to move the
         * dword index up above V3D_CHANNELS.
         */
        return nir_imul_imm(b, offset, V3D_CHANNELS);
}

static void
v3d_nir_lower_load_scratch(nir_builder *b, nir_intrinsic_instr *instr)
{
        b->cursor = nir_before_instr(&instr->instr);

        nir_def *offset = v3d_nir_scratch_offset(b,instr);

        nir_def *chans[NIR_MAX_VEC_COMPONENTS];
        for (int i = 0; i < instr->num_components; i++) {
                nir_def *chan_offset =
                        nir_iadd_imm(b, offset, V3D_CHANNELS * i * 4);

                nir_intrinsic_instr *chan_instr =
                        nir_intrinsic_instr_create(b->shader, instr->intrinsic);
                chan_instr->num_components = 1;
                nir_def_init(&chan_instr->instr, &chan_instr->def, 1,
                             instr->def.bit_size);

                chan_instr->src[0] = nir_src_for_ssa(chan_offset);

                nir_intrinsic_set_align(chan_instr, 4, 0);

                nir_builder_instr_insert(b, &chan_instr->instr);

                chans[i] = &chan_instr->def;
        }

        nir_def *result = nir_vec(b, chans, instr->num_components);
        nir_def_rewrite_uses(&instr->def, result);
        nir_instr_remove(&instr->instr);
}

static void
v3d_nir_lower_store_scratch(nir_builder *b, nir_intrinsic_instr *instr)
{
        b->cursor = nir_before_instr(&instr->instr);

        nir_def *offset = v3d_nir_scratch_offset(b, instr);
        nir_def *value = instr->src[0].ssa;

        for (int i = 0; i < instr->num_components; i++) {
                if (!(nir_intrinsic_write_mask(instr) & (1 << i)))
                        continue;

                nir_def *chan_offset =
                        nir_iadd_imm(b, offset, V3D_CHANNELS * i * 4);

                nir_intrinsic_instr *chan_instr =
                        nir_intrinsic_instr_create(b->shader, instr->intrinsic);
                chan_instr->num_components = 1;

                chan_instr->src[0] = nir_src_for_ssa(nir_channel(b,
                                                                 value,
                                                                 i));
                chan_instr->src[1] = nir_src_for_ssa(chan_offset);
                nir_intrinsic_set_write_mask(chan_instr, 0x1);
                nir_intrinsic_set_align(chan_instr, 4, 0);

                nir_builder_instr_insert(b, &chan_instr->instr);
        }

        nir_instr_remove(&instr->instr);
}

static bool
v3d_nir_lower_scratch_cb(nir_builder *b,
                         nir_intrinsic_instr *intr,
                         void *_state)
{
        switch (intr->intrinsic) {
        case nir_intrinsic_load_scratch:
                v3d_nir_lower_load_scratch(b, intr);
                return true;
        case nir_intrinsic_store_scratch:
                v3d_nir_lower_store_scratch(b, intr);
                return true;
        default:
                return false;
        }

        return false;
}

bool
v3d_nir_lower_scratch(nir_shader *s)
{
        return nir_shader_intrinsics_pass(s, v3d_nir_lower_scratch_cb,
                                            nir_metadata_block_index |
                                            nir_metadata_dominance, NULL);
}
