/*
 * Copyright © 2015 Broadcom
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

/** @file v3d_nir_lower_txf_ms.c
 *
 * V3D's MSAA surfaces are laid out in UIF textures where each pixel is a 2x2
 * quad in the texture.  This pass lowers the txf_ms with a ms_index source to
 * a plain txf with the sample_index pulling out the correct texel from the
 * 2x2 quad.
 */

static nir_def *
v3d_nir_lower_txf_ms_instr(nir_builder *b, nir_instr *in_instr, void *data)
{
        nir_tex_instr *instr = nir_instr_as_tex(in_instr);

        b->cursor = nir_before_instr(&instr->instr);

        nir_def *coord = nir_steal_tex_src(instr, nir_tex_src_coord);
        nir_def *sample = nir_steal_tex_src(instr, nir_tex_src_ms_index);

        nir_def *one = nir_imm_int(b, 1);
        nir_def *x = nir_iadd(b,
                                  nir_ishl(b, nir_channel(b, coord, 0), one),
                                  nir_iand(b, sample, one));
        nir_def *y = nir_iadd(b,
                                  nir_ishl(b, nir_channel(b, coord, 1), one),
                                  nir_iand(b, nir_ushr(b, sample, one), one));
        if (instr->is_array)
                coord = nir_vec3(b, x, y, nir_channel(b, coord, 2));
        else
                coord = nir_vec2(b, x, y);

        nir_tex_instr_add_src(instr, nir_tex_src_coord, coord);
        instr->op = nir_texop_txf;
        instr->sampler_dim = GLSL_SAMPLER_DIM_2D;

        return NIR_LOWER_INSTR_PROGRESS;
}

static bool
v3d_nir_lower_txf_ms_filter(const nir_instr *instr, const void *data)
{
        return (instr->type == nir_instr_type_tex &&
                nir_instr_as_tex(instr)->op == nir_texop_txf_ms);
}

bool
v3d_nir_lower_txf_ms(nir_shader *s)
{
        return nir_shader_lower_instructions(s,
                                             v3d_nir_lower_txf_ms_filter,
                                             v3d_nir_lower_txf_ms_instr,
                                             NULL);
}
