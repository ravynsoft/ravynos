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

/* Author:
 *    Keith Whitwell <keithw@vmware.com>
 */

#include "draw/draw_context.h"
#include "pipe/p_defines.h"
#include "i915_batch.h"
#include "i915_context.h"
#include "i915_debug.h"
#include "i915_reg.h"

static void
i915_flush_pipe(struct pipe_context *pipe, struct pipe_fence_handle **fence,
                unsigned flags)
{
   struct i915_context *i915 = i915_context(pipe);
   enum i915_winsys_flush_flags winsys_flags = I915_FLUSH_ASYNC;

   if (!i915->batch)
      return;

   /* Only shortcut this if we have no fence, otherwise we must flush the
    * empty batchbuffer to get our fence back.
    */
   if (!fence && (i915->batch->map == i915->batch->ptr)) {
      return;
   }

   if (flags == PIPE_FLUSH_END_OF_FRAME)
      winsys_flags = I915_FLUSH_END_OF_FRAME;

   FLUSH_BATCH(fence, winsys_flags);

   I915_DBG(DBG_FLUSH, "%s: #####\n", __func__);
}

void
i915_init_flush_functions(struct i915_context *i915)
{
   i915->base.flush = i915_flush_pipe;
}

/**
 * Here we handle all the notifications that needs to go out on a flush.
 * XXX might move above function to i915_pipe_flush.c and leave this here.
 */
void
i915_flush(struct i915_context *i915, struct pipe_fence_handle **fence,
           unsigned flags)
{
   struct i915_winsys_batchbuffer *batch = i915->batch;

   batch->iws->batchbuffer_flush(batch, fence, flags);
   i915->vbo_flushed = 1;
   i915->hardware_dirty = ~0;
   i915->immediate_dirty = ~0;
   i915->dynamic_dirty = ~0;
   i915->static_dirty = ~0;
   /* kernel emits flushes in between batchbuffers */
   i915->flush_dirty = 0;
   i915->fired_vertices += i915->queued_vertices;
   i915->queued_vertices = 0;
}
