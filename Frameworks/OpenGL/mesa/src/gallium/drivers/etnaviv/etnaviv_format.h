/*
 * Copyright (c) 2016 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#ifndef ETNAVIV_FORMAT_H_
#define ETNAVIV_FORMAT_H_

#include "util/format/u_format.h"
#include "pipe/p_state.h"
#include <stdint.h>

#define ETNA_NO_MATCH (~0)
#define EXT_FORMAT (1 << 31)
#define ASTC_FORMAT (1 << 30)

uint32_t
translate_texture_format(enum pipe_format fmt);

bool
texture_use_int_filter(const struct pipe_sampler_view *sv,
                       const struct pipe_sampler_state *ss,
                       bool tex_desc);

bool
texture_format_needs_swiz(enum pipe_format fmt);

uint32_t
get_texture_swiz(enum pipe_format fmt, unsigned swizzle_r,
                 unsigned swizzle_g, unsigned swizzle_b, unsigned swizzle_a);

uint32_t
translate_pe_format(enum pipe_format fmt);

int
translate_pe_format_rb_swap(enum pipe_format fmt);

uint32_t
translate_vertex_format_type(enum pipe_format fmt);

#endif /* ETNAVIV_FORMAT_H_ */
