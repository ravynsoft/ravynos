/*
 * Copyright 2014, 2015 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "virgl_context.h"
#include "virgl_encode.h"
#include "virgl_resource.h"
#include "virgl_screen.h"

void virgl_buffer_transfer_unmap(struct pipe_context *ctx,
                                 struct pipe_transfer *transfer)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_transfer *trans = virgl_transfer(transfer);
   bool persistent_coherent = trans->base.usage & (PIPE_MAP_PERSISTENT |
                                                   PIPE_MAP_COHERENT);

   if ((trans->base.usage & PIPE_MAP_WRITE) && !persistent_coherent) {
      if (transfer->usage & PIPE_MAP_FLUSH_EXPLICIT) {
         if (trans->range.end <= trans->range.start) {
            virgl_resource_destroy_transfer(vctx, trans);
            return;
         }

         transfer->box.x += trans->range.start;
         transfer->box.width = trans->range.end - trans->range.start;
         trans->offset = transfer->box.x;
      }

      if (trans->copy_src_hw_res && trans->direction == VIRGL_TRANSFER_TO_HOST) {
         virgl_encode_copy_transfer(vctx, trans);
         virgl_resource_destroy_transfer(vctx, trans);
      } else if (trans->copy_src_hw_res && trans->direction == VIRGL_TRANSFER_FROM_HOST) {
         // if it is readback, then we have already encoded transfer
         virgl_resource_destroy_transfer(vctx, trans);
      } else {
         virgl_transfer_queue_unmap(&vctx->queue, trans);
      }
   } else
      virgl_resource_destroy_transfer(vctx, trans);
}

void virgl_buffer_transfer_flush_region(struct pipe_context *ctx,
                                        struct pipe_transfer *transfer,
                                        const struct pipe_box *box)
{
   struct virgl_transfer *trans = virgl_transfer(transfer);

   /*
    * FIXME: This is not optimal.  For example,
    *
    * glMapBufferRange(.., 0, 100, GL_MAP_FLUSH_EXPLICIT_BIT)
    * glFlushMappedBufferRange(.., 25, 30)
    * glFlushMappedBufferRange(.., 65, 70)
    *
    * We'll end up flushing 25 --> 70.
    */
   util_range_add(transfer->resource, &trans->range, box->x, box->x + box->width);
}

void virgl_buffer_init(struct virgl_resource *res)
{
}
