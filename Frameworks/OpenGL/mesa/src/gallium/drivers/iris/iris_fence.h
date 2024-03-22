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

#ifndef IRIS_FENCE_H
#define IRIS_FENCE_H

#include "util/u_inlines.h"

struct pipe_screen;
struct iris_batch;
struct iris_bufmgr;

/**
 * A refcounted DRM Sync Object (drm_syncobj).
 */
struct iris_syncobj {
   struct pipe_reference ref;
   uint32_t handle;
};

struct iris_syncobj *iris_create_syncobj(struct iris_bufmgr *bufmgr);
void iris_syncobj_destroy(struct iris_bufmgr *, struct iris_syncobj *);
void iris_syncobj_signal(struct iris_bufmgr *, struct iris_syncobj *);

void iris_batch_add_syncobj(struct iris_batch *batch,
                            struct iris_syncobj *syncobj,
                            uint32_t flags);
bool iris_wait_syncobj(struct iris_bufmgr *bufmgr,
                       struct iris_syncobj *syncobj,
                       int64_t timeout_nsec);

static inline void
iris_syncobj_reference(struct iris_bufmgr *bufmgr,
                       struct iris_syncobj **dst,
                       struct iris_syncobj *src)
{
   if (pipe_reference(*dst ? &(*dst)->ref : NULL,
                      src ? &src->ref : NULL))
      iris_syncobj_destroy(bufmgr, *dst);

   *dst = src;
}

/* ------------------------------------------------------------------- */

void iris_init_context_fence_functions(struct pipe_context *ctx);
void iris_init_screen_fence_functions(struct pipe_screen *screen);

#endif
