/*
 * Copyright (c) 2012-2015 Etnaviv Project
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
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef ETNAVIV_FENCE_H_
#define ETNAVIV_FENCE_H_

#include "pipe/p_context.h"

void
etna_create_fence_fd(struct pipe_context *pctx,
                     struct pipe_fence_handle **pfence, int fd,
                     enum pipe_fd_type type);

void
etna_fence_server_sync(struct pipe_context *pctx,
                       struct pipe_fence_handle *fence);

int
etna_fence_get_fd(struct pipe_screen *pscreen,
                  struct pipe_fence_handle *pfence);

struct pipe_fence_handle *
etna_fence_create(struct pipe_context *pctx, int fence_fd);

void
etna_fence_screen_init(struct pipe_screen *pscreen);

#endif
