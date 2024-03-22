/*
 * Copyright 2018 Chromium.
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

#ifndef VIRGL_TRANSFER_QUEUE_H
#define VIRGL_TRANSFER_QUEUE_H

#include "pipe/p_defines.h"
#include "util/list.h"

struct virgl_cmd_buf;
struct virgl_screen;
struct virgl_context;
struct virgl_transfer;

struct virgl_transfer_queue {
   struct list_head transfer_list;
   struct virgl_screen *vs;
   struct virgl_context *vctx;
   struct virgl_cmd_buf *tbuf;
   uint32_t num_dwords;
};

void virgl_transfer_queue_init(struct virgl_transfer_queue *queue,
                               struct virgl_context *vctx);

void virgl_transfer_queue_fini(struct virgl_transfer_queue *queue);

int virgl_transfer_queue_unmap(struct virgl_transfer_queue *queue,
                               struct virgl_transfer *transfer);

int virgl_transfer_queue_clear(struct virgl_transfer_queue *queue,
                               struct virgl_cmd_buf *buf);

bool virgl_transfer_queue_is_queued(struct virgl_transfer_queue *queue,
                                    struct virgl_transfer *transfer);

/*
 * Search the transfer queue for a transfer suitable for extension and
 * extend it to include the specified data.
 */
bool virgl_transfer_queue_extend_buffer(struct virgl_transfer_queue *queue,
                                        const struct virgl_hw_res *hw_res,
                                        unsigned offset, unsigned size,
                                        const void *data);

#endif /* VIRGL_TRANSFER_QUEUE_H */
