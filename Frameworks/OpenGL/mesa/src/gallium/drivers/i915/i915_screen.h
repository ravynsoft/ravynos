/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef I915_SCREEN_H
#define I915_SCREEN_H

#include "pipe/p_screen.h"
#include "pipe/p_state.h"

struct i915_winsys;

/**
 * Subclass of pipe_screen
 */
struct i915_screen {
   struct pipe_screen base;

   struct i915_winsys *iws;

   bool is_i945;

   struct {
      bool tiling;
      bool use_blitter;
   } debug;
};

/*
 * Cast wrappers
 */

static inline struct i915_screen *
i915_screen(struct pipe_screen *pscreen)
{
   return (struct i915_screen *)pscreen;
}

bool
i915_is_format_supported(struct pipe_screen *screen, enum pipe_format format,
                         enum pipe_texture_target target, unsigned sample_count,
                         unsigned storage_sample_count, unsigned tex_usage);

#endif /* I915_SCREEN_H */
