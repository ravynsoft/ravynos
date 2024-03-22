/*
 * Copyright © 2017 Timothy Arceri
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef ST_SHADER_CACHE_H
#define ST_SHADER_CACHE_H

#include "st_context.h"
#include "pipe/p_state.h"
#include "util/blob.h"
#include "util/disk_cache.h"
#include "util/mesa-sha1.h"

#ifdef __cplusplus
extern "C" {
#endif

void
st_get_program_binary_driver_sha1(struct gl_context *ctx, uint8_t *sha1);

void
st_serialise_nir_program(struct gl_context *ctx, struct gl_program *prog);

void
st_serialise_nir_program_binary(struct gl_context *ctx,
                                struct gl_shader_program *shProg,
                                struct gl_program *prog);

void
st_deserialise_nir_program(struct gl_context *ctx,
                           struct gl_shader_program *shProg,
                           struct gl_program *prog);

bool
st_load_nir_from_disk_cache(struct gl_context *ctx,
                            struct gl_shader_program *prog);

void
st_store_nir_in_disk_cache(struct st_context *st, struct gl_program *prog);

#ifdef __cplusplus
}
#endif

#endif
