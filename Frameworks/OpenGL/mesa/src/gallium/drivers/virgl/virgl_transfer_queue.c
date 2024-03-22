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

#include "util/u_box.h"
#include "util/u_inlines.h"

#include "virtio-gpu/virgl_protocol.h"
#include "virgl_context.h"
#include "virgl_screen.h"
#include "virgl_encode.h"
#include "virgl_resource.h"
#include "virgl_transfer_queue.h"

struct list_action_args
{
   void *data;
   struct virgl_transfer *queued;
   struct virgl_transfer *current;
};

typedef bool (*compare_transfers_t)(struct virgl_transfer *queued,
                                    struct virgl_transfer *current);

typedef void (*list_action_t)(struct virgl_transfer_queue *queue,
                              struct list_action_args *args);

struct list_iteration_args
{
   void *data;
   list_action_t action;
   compare_transfers_t compare;
   struct virgl_transfer *current;
};

static int
transfer_dim(const struct virgl_transfer *xfer)
{
   switch (xfer->base.resource->target) {
   case PIPE_BUFFER:
   case PIPE_TEXTURE_1D:
      return 1;
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
      return 2;
   default:
      return 3;
   }
}

static void
box_min_max(const struct pipe_box *box, int dim, int *min, int *max)
{
   switch (dim) {
   case 0:
      if (box->width > 0) {
         *min = box->x;
         *max = box->x + box->width;
      } else {
         *max = box->x;
         *min = box->x + box->width;
      }
      break;
   case 1:
      if (box->height > 0) {
         *min = box->y;
         *max = box->y + box->height;
      } else {
         *max = box->y;
         *min = box->y + box->height;
      }
      break;
   default:
      if (box->depth > 0) {
         *min = box->z;
         *max = box->z + box->depth;
      } else {
         *max = box->z;
         *min = box->z + box->depth;
      }
      break;
   }
}

static bool
transfer_overlap(const struct virgl_transfer *xfer,
                 const struct virgl_hw_res *hw_res,
                 unsigned level,
                 const struct pipe_box *box,
                 bool include_touching)
{
   const int dim_count = transfer_dim(xfer);

   if (xfer->hw_res != hw_res || xfer->base.level != level)
      return false;

   for (int dim = 0; dim < dim_count; dim++) {
      int xfer_min;
      int xfer_max;
      int box_min;
      int box_max;

      box_min_max(&xfer->base.box, dim, &xfer_min, &xfer_max);
      box_min_max(box, dim, &box_min, &box_max);

      if (include_touching) {
         /* touching is considered overlapping */
         if (xfer_min > box_max || xfer_max < box_min)
            return false;
      } else {
         /* touching is not considered overlapping */
         if (xfer_min >= box_max || xfer_max <= box_min)
            return false;
      }
   }

   return true;
}

static struct virgl_transfer *
virgl_transfer_queue_find_overlap(const struct virgl_transfer_queue *queue,
                                  const struct virgl_hw_res *hw_res,
                                  unsigned level,
                                  const struct pipe_box *box,
                                  bool include_touching)
{
   struct virgl_transfer *xfer;
   LIST_FOR_EACH_ENTRY(xfer, &queue->transfer_list, queue_link) {
      if (transfer_overlap(xfer, hw_res, level, box, include_touching))
         return xfer;
   }

   return NULL;
}

static bool transfers_intersect(struct virgl_transfer *queued,
                                struct virgl_transfer *current)
{
   return transfer_overlap(queued, current->hw_res, current->base.level,
         &current->base.box, true);
}

static void remove_transfer(struct virgl_transfer_queue *queue,
                            struct virgl_transfer *queued)
{
   list_del(&queued->queue_link);
   virgl_resource_destroy_transfer(queue->vctx, queued);
}

static void replace_unmapped_transfer(struct virgl_transfer_queue *queue,
                                      struct list_action_args *args)
{
   struct virgl_transfer *current = args->current;
   struct virgl_transfer *queued = args->queued;

   u_box_union_2d(&current->base.box, &current->base.box, &queued->base.box);
   current->offset = current->base.box.x;

   remove_transfer(queue, queued);
   queue->num_dwords -= (VIRGL_TRANSFER3D_SIZE + 1);
}

static void transfer_put(struct virgl_transfer_queue *queue,
                         struct list_action_args *args)
{
   struct virgl_transfer *queued = args->queued;

   queue->vs->vws->transfer_put(queue->vs->vws, queued->hw_res,
                                &queued->base.box,
                                queued->base.stride, queued->l_stride,
                                queued->offset, queued->base.level);

   remove_transfer(queue, queued);
}

static void transfer_write(struct virgl_transfer_queue *queue,
                           struct list_action_args *args)
{
   struct virgl_transfer *queued = args->queued;
   struct virgl_cmd_buf *buf = args->data;

   // Takes a reference on the HW resource, which is released after
   // the exec buffer command.
   virgl_encode_transfer(queue->vs, buf, queued, VIRGL_TRANSFER_TO_HOST);

   remove_transfer(queue, queued);
}

static void compare_and_perform_action(struct virgl_transfer_queue *queue,
                                       struct list_iteration_args *iter)
{
   struct list_action_args args;
   struct virgl_transfer *queued, *tmp;

   memset(&args, 0, sizeof(args));
   args.current = iter->current;
   args.data = iter->data;

   LIST_FOR_EACH_ENTRY_SAFE(queued, tmp, &queue->transfer_list, queue_link) {
      if (iter->compare(queued, iter->current)) {
         args.queued = queued;
         iter->action(queue, &args);
      }
   }
}

static void perform_action(struct virgl_transfer_queue *queue,
                           struct list_iteration_args *iter)
{
   struct list_action_args args;
   struct virgl_transfer *queued, *tmp;

   memset(&args, 0, sizeof(args));
   args.data = iter->data;

   LIST_FOR_EACH_ENTRY_SAFE(queued, tmp, &queue->transfer_list, queue_link) {
      args.queued = queued;
      iter->action(queue, &args);
   }
}

static void add_internal(struct virgl_transfer_queue *queue,
                         struct virgl_transfer *transfer)
{
   uint32_t dwords = VIRGL_TRANSFER3D_SIZE + 1;
   if (queue->tbuf) {
      if (queue->num_dwords + dwords >= VIRGL_MAX_TBUF_DWORDS) {
         struct list_iteration_args iter;
         struct virgl_winsys *vws = queue->vs->vws;

         memset(&iter, 0, sizeof(iter));
         iter.action = transfer_write;
         iter.data = queue->tbuf;
         perform_action(queue, &iter);

         vws->submit_cmd(vws, queue->tbuf, NULL);
         queue->num_dwords = 0;
      }
   }

   list_addtail(&transfer->queue_link, &queue->transfer_list);
   queue->num_dwords += dwords;
}

void virgl_transfer_queue_init(struct virgl_transfer_queue *queue,
                               struct virgl_context *vctx)
{
   struct virgl_screen *vs = virgl_screen(vctx->base.screen);

   queue->vs = vs;
   queue->vctx = vctx;
   queue->num_dwords = 0;

   list_inithead(&queue->transfer_list);

   if ((vs->caps.caps.v2.capability_bits & VIRGL_CAP_TRANSFER) &&
        vs->vws->supports_encoded_transfers)
      queue->tbuf = vs->vws->cmd_buf_create(vs->vws, VIRGL_MAX_TBUF_DWORDS);
   else
      queue->tbuf = NULL;
}

void virgl_transfer_queue_fini(struct virgl_transfer_queue *queue)
{
   struct virgl_winsys *vws = queue->vs->vws;
   struct list_iteration_args iter;

   memset(&iter, 0, sizeof(iter));

   iter.action = transfer_put;
   perform_action(queue, &iter);

   if (queue->tbuf)
      vws->cmd_buf_destroy(queue->tbuf);

   queue->vs = NULL;
   queue->vctx = NULL;
   queue->tbuf = NULL;
   queue->num_dwords = 0;
}

int virgl_transfer_queue_unmap(struct virgl_transfer_queue *queue,
                               struct virgl_transfer *transfer)
{
   struct list_iteration_args iter;

   /* We don't support copy transfers in the transfer queue. */
   assert(!transfer->copy_src_hw_res);

   /* Attempt to merge multiple intersecting transfers into a single one. */
   if (transfer->base.resource->target == PIPE_BUFFER) {
      memset(&iter, 0, sizeof(iter));
      iter.current = transfer;
      iter.compare = transfers_intersect;
      iter.action = replace_unmapped_transfer;
      compare_and_perform_action(queue, &iter);
   }

   add_internal(queue, transfer);
   return 0;
}

int virgl_transfer_queue_clear(struct virgl_transfer_queue *queue,
                               struct virgl_cmd_buf *cbuf)
{
   struct list_iteration_args iter;

   memset(&iter, 0, sizeof(iter));
   if (queue->tbuf) {
      uint32_t prior_num_dwords = cbuf->cdw;
      cbuf->cdw = 0;

      iter.action = transfer_write;
      iter.data = cbuf;
      perform_action(queue, &iter);

      virgl_encode_end_transfers(cbuf);
      cbuf->cdw = prior_num_dwords;
   } else {
      iter.action = transfer_put;
      perform_action(queue, &iter);
   }

   queue->num_dwords = 0;

   return 0;
}

bool virgl_transfer_queue_is_queued(struct virgl_transfer_queue *queue,
                                    struct virgl_transfer *transfer)
{
   return virgl_transfer_queue_find_overlap(queue,
                                            transfer->hw_res,
                                            transfer->base.level,
                                            &transfer->base.box,
                                            false);
}

bool
virgl_transfer_queue_extend_buffer(struct virgl_transfer_queue *queue,
                                   const struct virgl_hw_res *hw_res,
                                   unsigned offset, unsigned size,
                                   const void *data)
{
   struct virgl_transfer *queued;
   struct pipe_box box;

   u_box_1d(offset, size, &box);
   queued = virgl_transfer_queue_find_overlap(queue, hw_res, 0, &box, true);
   if (!queued)
      return false;

   assert(queued->base.resource->target == PIPE_BUFFER);
   assert(queued->hw_res_map);

   memcpy(queued->hw_res_map + offset, data, size);
   u_box_union_2d(&queued->base.box, &queued->base.box, &box);
   queued->offset = queued->base.box.x;

   return true;
}
