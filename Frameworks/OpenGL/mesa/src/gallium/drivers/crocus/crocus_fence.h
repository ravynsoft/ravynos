/*
 * Copyright © 2018 Intel Corporation
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

#ifndef CROCUS_FENCE_H
#define CROCUS_FENCE_H

#include "util/u_inlines.h"

struct pipe_screen;
struct crocus_screen;
struct crocus_batch;

struct crocus_syncobj {
   struct pipe_reference ref;
   uint32_t handle;
};

void crocus_init_context_fence_functions(struct pipe_context *ctx);
void crocus_init_screen_fence_functions(struct pipe_screen *screen);

struct crocus_syncobj *crocus_create_syncobj(struct crocus_screen *screen);
void crocus_syncobj_destroy(struct crocus_screen *, struct crocus_syncobj *);
void crocus_batch_add_syncobj(struct crocus_batch *batch,
                              struct crocus_syncobj *syncobj,
                              unsigned flags);
bool crocus_wait_syncobj(struct pipe_screen *screen,
                         struct crocus_syncobj *syncobj,
                         int64_t timeout_nsec);
static inline void
crocus_syncobj_reference(struct crocus_screen *screen,
                         struct crocus_syncobj **dst,
                         struct crocus_syncobj *src)
{
   if (pipe_reference(&(*dst)->ref, &src->ref))
      crocus_syncobj_destroy(screen, *dst);

   *dst = src;
}

#endif
