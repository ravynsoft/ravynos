/*
 * Copyright © 2014-2018 NVIDIA Corporation
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

#ifndef TEGRA_CONTEXT_H
#define TEGRA_CONTEXT_H

#include "pipe/p_context.h"
#include "pipe/p_state.h"

struct tegra_screen;

struct tegra_context {
   struct pipe_context base;
   struct pipe_context *gpu;
};

static inline struct tegra_context *
to_tegra_context(struct pipe_context *context)
{
   return (struct tegra_context *)context;
}

struct pipe_context *
tegra_screen_context_create(struct pipe_screen *pscreen, void *priv,
                            unsigned int flags);

struct tegra_sampler_view {
   struct pipe_sampler_view base;
   struct pipe_sampler_view *gpu;
   unsigned int refcount;
};

static inline struct tegra_sampler_view *
to_tegra_sampler_view(struct pipe_sampler_view *view)
{
   return (struct tegra_sampler_view *)view;
}

static inline struct pipe_sampler_view *
tegra_sampler_view_unwrap(struct pipe_sampler_view *view)
{
   if (!view)
      return NULL;

   return to_tegra_sampler_view(view)->gpu;
}

struct tegra_transfer {
   struct pipe_transfer base;
   struct pipe_transfer *gpu;

   unsigned int count;
   void *map;
};

static inline struct tegra_transfer *
to_tegra_transfer(struct pipe_transfer *transfer)
{
   return (struct tegra_transfer *)transfer;
}

#endif /* TEGRA_SCREEN_H */
