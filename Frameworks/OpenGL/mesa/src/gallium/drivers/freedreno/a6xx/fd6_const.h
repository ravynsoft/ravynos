/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef FD6_CONST_H
#define FD6_CONST_H

#include "fd6_emit.h"

struct fd_ringbuffer *fd6_build_tess_consts(struct fd6_emit *emit) assert_dt;
unsigned fd6_user_consts_cmdstream_size(const struct ir3_shader_variant *v);

template <fd6_pipeline_type PIPELINE>
struct fd_ringbuffer *fd6_build_user_consts(struct fd6_emit *emit) assert_dt;

template <fd6_pipeline_type PIPELINE>
struct fd_ringbuffer *
fd6_build_driver_params(struct fd6_emit *emit) assert_dt;

void fd6_emit_cs_driver_params(struct fd_context *ctx,
                               struct fd_ringbuffer *ring,
                               struct fd6_compute_state *cs,
                               const struct pipe_grid_info *info) assert_dt;
void fd6_emit_cs_user_consts(struct fd_context *ctx,
                             struct fd_ringbuffer *ring,
                             struct fd6_compute_state *cs) assert_dt;
void fd6_emit_immediates(const struct ir3_shader_variant *v,
                         struct fd_ringbuffer *ring) assert_dt;
void fd6_emit_link_map(const struct ir3_shader_variant *producer,
                       const struct ir3_shader_variant *consumer,
                       struct fd_ringbuffer *ring) assert_dt;

#endif /* FD6_CONST_H */
