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

#ifndef TEGRA_RESOURCE_H
#define TEGRA_RESOURCE_H

#include "pipe/p_state.h"

struct winsys_handle;

struct tegra_resource {
   struct pipe_resource base;
   struct pipe_resource *gpu;
   unsigned int refcount;

   uint64_t modifier;
   uint32_t stride;
   uint32_t handle;
   size_t size;
};

static inline struct tegra_resource *
to_tegra_resource(struct pipe_resource *resource)
{
   return (struct tegra_resource *)resource;
}

static inline struct pipe_resource *
tegra_resource_unwrap(struct pipe_resource *resource)
{
   if (!resource)
      return NULL;

   return to_tegra_resource(resource)->gpu;
}

struct tegra_surface {
   struct pipe_surface base;
   struct pipe_surface *gpu;
};

static inline struct tegra_surface *
to_tegra_surface(struct pipe_surface *surface)
{
   return (struct tegra_surface *)surface;
}

static inline struct pipe_surface *
tegra_surface_unwrap(struct pipe_surface *surface)
{
   if (!surface)
      return NULL;

   return to_tegra_surface(surface)->gpu;
}

#endif /* TEGRA_RESOURCE_H */
