/*
 * Copyright © 2023 Valve Corporation
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

#ifndef LP_SAMPLER_MATRIX
#define LP_SAMPLER_MATRIX

#include "util/bitset.h"
#include "util/u_dynarray.h"
#include "util/format/u_format.h"
#include "util/simple_mtx.h"
#include "gallivm/lp_bld_sample.h"
#include "gallivm/lp_bld_jit_sample.h"

#define LP_SAMPLE_KEY_COUNT (1 << 11)

struct lp_sampler_matrix {
   struct lp_texture_functions **textures;
   struct lp_static_sampler_state *samplers;

   uint32_t texture_count;
   uint32_t sampler_count;

   uint32_t sampler_keys[LP_SAMPLE_KEY_COUNT];
   BITSET_DECLARE(image_ops, LP_TOTAL_IMAGE_OP_COUNT);

   struct util_dynarray gallivms;
};

void llvmpipe_init_sampler_matrix(struct llvmpipe_context *ctx);

void llvmpipe_sampler_matrix_destroy(struct llvmpipe_context *ctx);

void llvmpipe_register_shader(struct pipe_context *ctx, const struct pipe_shader_state *shader, bool unregister);

#endif /* LP_SAMPLER_MATRIX */
