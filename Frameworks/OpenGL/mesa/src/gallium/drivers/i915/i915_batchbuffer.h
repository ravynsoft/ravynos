/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
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

#ifndef I915_BATCHBUFFER_H
#define I915_BATCHBUFFER_H

#include "util/u_debug.h"
#include "i915_winsys.h"

struct i915_context;

static inline size_t
i915_winsys_batchbuffer_space(struct i915_winsys_batchbuffer *batch)
{
   return batch->size - (batch->ptr - batch->map);
}

static inline bool
i915_winsys_batchbuffer_check(struct i915_winsys_batchbuffer *batch,
                              size_t dwords)
{
   return dwords * 4 <= i915_winsys_batchbuffer_space(batch);
}

static inline void
i915_winsys_batchbuffer_dword_unchecked(struct i915_winsys_batchbuffer *batch,
                                        unsigned dword)
{
   *(unsigned *)batch->ptr = dword;
   batch->ptr += 4;
}

static inline void
i915_winsys_batchbuffer_float(struct i915_winsys_batchbuffer *batch, float f)
{
   union {
      float f;
      unsigned int ui;
   } uif;
   uif.f = f;
   assert(i915_winsys_batchbuffer_space(batch) >= 4);
   i915_winsys_batchbuffer_dword_unchecked(batch, uif.ui);
}

static inline void
i915_winsys_batchbuffer_dword(struct i915_winsys_batchbuffer *batch,
                              unsigned dword)
{
   assert(i915_winsys_batchbuffer_space(batch) >= 4);
   i915_winsys_batchbuffer_dword_unchecked(batch, dword);
}

static inline void
i915_winsys_batchbuffer_write(struct i915_winsys_batchbuffer *batch, void *data,
                              size_t size)
{
   assert(i915_winsys_batchbuffer_space(batch) >= size);

   memcpy(batch->ptr, data, size);
   batch->ptr += size;
}

static inline bool
i915_winsys_validate_buffers(struct i915_winsys_batchbuffer *batch,
                             struct i915_winsys_buffer **buffers,
                             int num_of_buffers)
{
   return batch->iws->validate_buffers(batch, buffers, num_of_buffers);
}

static inline int
i915_winsys_batchbuffer_reloc(struct i915_winsys_batchbuffer *batch,
                              struct i915_winsys_buffer *buffer,
                              enum i915_winsys_buffer_usage usage,
                              size_t offset, bool fenced)
{
   return batch->iws->batchbuffer_reloc(batch, buffer, usage, offset, fenced);
}

#endif
