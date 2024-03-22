/*
 * Copyright © 2020 Raspberry Pi Ltd
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
#include <math.h>

/**
 * Lowers line smoothing by modifying the alpha component of fragment outputs
 * using the distance from the center of the line.
 */

struct lower_line_smooth_state {
        nir_shader *shader;
        nir_variable *coverage;
};

static void
lower_line_smooth_intrinsic(struct lower_line_smooth_state *state,
                            nir_builder *b,
                            nir_intrinsic_instr *intr)
{
        b->cursor = nir_before_instr(&intr->instr);

        nir_def *one = nir_imm_float(b, 1.0f);

        nir_def *coverage = nir_load_var(b, state->coverage);

        nir_def *new_val = nir_fmul(b, nir_vec4(b, one, one, one, coverage),
                                        intr->src[0].ssa);

        nir_src_rewrite(&intr->src[0], new_val);
}

static bool
lower_line_smooth_func(struct lower_line_smooth_state *state,
                       nir_function_impl *impl)
{
        bool progress = false;

        nir_builder b = nir_builder_create(impl);

        nir_foreach_block(block, impl) {
                nir_foreach_instr_safe(instr, block) {
                        if (instr->type != nir_instr_type_intrinsic)
                                continue;

                        nir_intrinsic_instr *intr =
                                nir_instr_as_intrinsic(instr);

                        if (intr->intrinsic != nir_intrinsic_store_output ||
                            nir_intrinsic_base(intr) != 0 ||
                            intr->num_components != 4)
                                continue;

                        lower_line_smooth_intrinsic(state, &b, intr);
                        progress = true;
                }
        }

        return progress;
}

static void
initialise_coverage_var(struct lower_line_smooth_state *state,
                        nir_function_impl *impl)
{
        nir_builder b = nir_builder_at(nir_before_impl(impl));

        nir_def *line_width = nir_load_line_width(&b);

        nir_def *real_line_width = nir_load_aa_line_width(&b);

        /* According to the PRM, the line coord varies from 0.0 to 1.0 across
         * the width of the line. But actually, when a perspective projection
         * is used, it is also applied to the line coords, so the values end
         * up being between [min_coord, 1], based on the Wc coordinate.  We
         * need to re-map the values to be between [0.0, 1.0].
         */
        nir_def *line_coord = nir_load_line_coord(&b);
        nir_def *wc = nir_load_fep_w_v3d(&b, 32);
        nir_def *min_coord_val = nir_fsub(&b, nir_imm_float(&b, 1.0f), wc);
        nir_def *normalized_line_coord = nir_fdiv(&b,
                                                  nir_fsub(&b, line_coord, min_coord_val),
                                                  nir_fsub_imm(&b, 1.0, min_coord_val));;

        /* fabs(line_coord - 0.5) * real_line_width */
        nir_def *pixels_from_center =
                nir_fmul(&b, real_line_width,
                         nir_fabs(&b, nir_fsub(&b, normalized_line_coord,
                                               nir_imm_float(&b, 0.5f))));

        /* 0.5 - 1/√2 * (pixels_from_center - line_width * 0.5) */
        nir_def *coverage =
                nir_fsub(&b,
                         nir_imm_float(&b, 0.5f),
                         nir_fmul(&b,
                                  nir_imm_float(&b, 1.0f / M_SQRT2),
                                  nir_fsub(&b, pixels_from_center,
                                           nir_fmul_imm(&b,
                                                        line_width,
                                                        0.5f))));

        /* Discard fragments that aren’t covered at all by the line */
        nir_def *outside = nir_fle_imm(&b, coverage, 0.0f);

        nir_discard_if(&b, outside);

        /* Clamp to at most 1.0. If it was less than 0.0 then the fragment will
         * be discarded so we don’t need to handle that.
         */
        nir_def *clamped = nir_fmin(&b, coverage, nir_imm_float(&b, 1.0f));

        nir_store_var(&b, state->coverage, clamped, 0x1 /* writemask */);
}

static nir_variable *
make_coverage_var(nir_shader *s)
{
        nir_variable *var = nir_variable_create(s,
                                                nir_var_shader_temp,
                                                glsl_float_type(),
                                                "line_coverage");
        var->data.how_declared = nir_var_hidden;

        return var;
}

bool
v3d_nir_lower_line_smooth(nir_shader *s)
{
        bool progress = false;

        assert(s->info.stage == MESA_SHADER_FRAGMENT);

        struct lower_line_smooth_state state = {
                .shader = s,
                .coverage = make_coverage_var(s),
        };

        nir_foreach_function_with_impl(function, impl, s) {
                if (function->is_entrypoint)
                        initialise_coverage_var(&state, impl);

                progress |= lower_line_smooth_func(&state, impl);

                if (progress) {
                        nir_metadata_preserve(impl,
                                              nir_metadata_block_index |
                                              nir_metadata_dominance);
                } else {
                        nir_metadata_preserve(impl, nir_metadata_all);
                }
        }

        return progress;
}
