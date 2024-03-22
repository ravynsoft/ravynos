/*
 * Copyright (c) 2012-2013 Etnaviv Project
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
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#ifndef H_ETNAVIV_CLEAR_BLIT
#define H_ETNAVIV_CLEAR_BLIT

#include <stdint.h>

struct etna_context;

#include "pipe/p_context.h"

void
etna_copy_resource(struct pipe_context *pctx, struct pipe_resource *dst,
                   struct pipe_resource *src, int first_level, int last_level);

void
etna_copy_resource_box(struct pipe_context *pctx, struct pipe_resource *dst,
                       struct pipe_resource *src, int dst_level, int src_level,
                       struct pipe_box *box);

void
etna_blit_save_state(struct etna_context *ctx, bool render_cond);

uint64_t
etna_clear_blit_pack_rgba(enum pipe_format format, const union pipe_color_union *color);

void
etna_clear_blit_init(struct pipe_context *pctx);

#endif
