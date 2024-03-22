/*
 * Copyright (C) 2016 Miklós Máté
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ST_ATIFS_TO_NIR_H
#define ST_ATIFS_TO_NIR_H

#include "util/glheader.h"
#include "pipe/p_defines.h"
#include "compiler/nir/nir.h"

#if defined __cplusplus
extern "C" {
#endif

struct gl_context;
struct gl_program;
struct ati_fragment_shader;
struct st_fp_variant_key;

nir_shader *
st_translate_atifs_program(struct ati_fragment_shader *atifs,
                           struct gl_program *program,
                           const nir_shader_compiler_options *options);

bool st_nir_lower_atifs_samplers(struct nir_shader *s, const uint8_t *texture_index);

#if defined __cplusplus
} /* extern "C" */
#endif

#endif /* ST_ATIFS_TO_NIR_H */
