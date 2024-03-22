/*
 * Copyright 2011 Christoph Bumiller
 * Copyright 2015 Samuel Pitoiset
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#define NVC0_PUSH_EXPLICIT_SPACE_CHECKING

#include "nvc0/nvc0_context.h"
#include "nvc0/nvc0_query_hw.h"
#include "nvc0/nvc0_query_hw_metric.h"
#include "nvc0/nvc0_query_hw_sm.h"

#define NVC0_HW_QUERY_ALLOC_SPACE 256

bool
nvc0_hw_query_allocate(struct nvc0_context *nvc0, struct nvc0_query *q,
                       int size)
{
   struct nvc0_hw_query *hq = nvc0_hw_query(q);
   struct nvc0_screen *screen = nvc0->screen;
   int ret;

   if (hq->bo) {
      nouveau_bo_ref(NULL, &hq->bo);
      if (hq->mm) {
         if (hq->state == NVC0_HW_QUERY_STATE_READY)
            nouveau_mm_free(hq->mm);
         else
            nouveau_fence_work(nvc0->base.fence,
                               nouveau_mm_free_work, hq->mm);
      }
   }
   if (size) {
      hq->mm = nouveau_mm_allocate(screen->base.mm_GART, size, &hq->bo,
                                   &hq->base_offset);
      if (!hq->bo)
         return false;
      hq->offset = hq->base_offset;

      ret = BO_MAP(&screen->base, hq->bo, 0, nvc0->base.client);
      if (ret) {
         nvc0_hw_query_allocate(nvc0, q, 0);
         return false;
      }
      hq->data = (uint32_t *)((uint8_t *)hq->bo->map + hq->base_offset);
   }
   return true;
}

static void
nvc0_hw_query_get(struct nouveau_pushbuf *push, struct nvc0_query *q,
                  unsigned offset, uint32_t get)
{
   struct nvc0_hw_query *hq = nvc0_hw_query(q);

   offset += hq->offset;

   PUSH_SPACE(push, 5);
   PUSH_REF1 (push, hq->bo, NOUVEAU_BO_GART | NOUVEAU_BO_WR);
   BEGIN_NVC0(push, NVC0_3D(QUERY_ADDRESS_HIGH), 4);
   PUSH_DATAh(push, hq->bo->offset + offset);
   PUSH_DATA (push, hq->bo->offset + offset);
   PUSH_DATA (push, hq->sequence);
   PUSH_DATA (push, get);
}

static void
nvc0_hw_query_rotate(struct nvc0_context *nvc0, struct nvc0_query *q)
{
   struct nvc0_hw_query *hq = nvc0_hw_query(q);

   hq->offset += hq->rotate;
   hq->data += hq->rotate / sizeof(*hq->data);
   if (hq->offset - hq->base_offset == NVC0_HW_QUERY_ALLOC_SPACE)
      nvc0_hw_query_allocate(nvc0, q, NVC0_HW_QUERY_ALLOC_SPACE);
}

static inline void
nvc0_hw_query_update(struct nouveau_client *cli, struct nvc0_query *q)
{
   struct nvc0_hw_query *hq = nvc0_hw_query(q);

   if (hq->is64bit) {
      if (nouveau_fence_signalled(hq->fence))
         hq->state = NVC0_HW_QUERY_STATE_READY;
   } else {
      if (hq->data[0] == hq->sequence)
         hq->state = NVC0_HW_QUERY_STATE_READY;
   }
}

static void
nvc0_hw_destroy_query(struct nvc0_context *nvc0, struct nvc0_query *q)
{
   struct nvc0_hw_query *hq = nvc0_hw_query(q);

   if (hq->funcs && hq->funcs->destroy_query) {
      hq->funcs->destroy_query(nvc0, hq);
      return;
   }

   nvc0_hw_query_allocate(nvc0, q, 0);
   nouveau_fence_ref(NULL, &hq->fence);
   FREE(hq);
}

static void
nvc0_hw_query_write_compute_invocations(struct nvc0_context *nvc0,
                                        struct nvc0_hw_query *hq,
                                        uint32_t offset)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;

   PUSH_SPACE_EX(push, 16, 0, 8);
   PUSH_REF1(push, hq->bo, NOUVEAU_BO_GART | NOUVEAU_BO_WR);
   BEGIN_1IC0(push, NVC0_3D(MACRO_COMPUTE_COUNTER_TO_QUERY), 4);
   PUSH_DATA (push, nvc0->compute_invocations);
   PUSH_DATAh(push, nvc0->compute_invocations);
   PUSH_DATAh(push, hq->bo->offset + hq->offset + offset);
   PUSH_DATA (push, hq->bo->offset + hq->offset + offset);
}

static bool
nvc0_hw_begin_query(struct nvc0_context *nvc0, struct nvc0_query *q)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_hw_query *hq = nvc0_hw_query(q);
   bool ret = true;

   if (hq->funcs && hq->funcs->begin_query)
      return hq->funcs->begin_query(nvc0, hq);

   /* For occlusion queries we have to change the storage, because a previous
    * query might set the initial render condition to false even *after* we re-
    * initialized it to true.
    */
   if (hq->rotate) {
      nvc0_hw_query_rotate(nvc0, q);

      /* XXX: can we do this with the GPU, and sync with respect to a previous
       *  query ?
       */
      hq->data[0] = hq->sequence; /* initialize sequence */
      hq->data[1] = 1; /* initial render condition = true */
      hq->data[4] = hq->sequence + 1; /* for comparison COND_MODE */
      hq->data[5] = 0;
   }
   hq->sequence++;

   switch (q->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      if (nvc0->screen->num_occlusion_queries_active++) {
         nvc0_hw_query_get(push, q, 0x10, 0x0100f002);
      } else {
         PUSH_SPACE(push, 3);
         BEGIN_NVC0(push, NVC0_3D(COUNTER_RESET), 1);
         PUSH_DATA (push, NVC0_3D_COUNTER_RESET_SAMPLECNT);
         IMMED_NVC0(push, NVC0_3D(SAMPLECNT_ENABLE), 1);
         /* Given that the counter is reset, the contents at 0x10 are
          * equivalent to doing the query -- we would get hq->sequence as the
          * payload and 0 as the reported value. This is already set up above
          * as in the hq->rotate case.
          */
      }
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      nvc0_hw_query_get(push, q, 0x10, 0x09005002 | (q->index << 5));
      break;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      nvc0_hw_query_get(push, q, 0x10, 0x05805002 | (q->index << 5));
      break;
   case PIPE_QUERY_SO_STATISTICS:
      nvc0_hw_query_get(push, q, 0x20, 0x05805002 | (q->index << 5));
      nvc0_hw_query_get(push, q, 0x30, 0x06805002 | (q->index << 5));
      break;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      nvc0_hw_query_get(push, q, 0x10, 0x03005002 | (q->index << 5));
      break;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      /* XXX: This get actually writes the number of overflowed streams */
      nvc0_hw_query_get(push, q, 0x10, 0x0f005002);
      break;
   case PIPE_QUERY_TIME_ELAPSED:
      nvc0_hw_query_get(push, q, 0x10, 0x00005002);
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      nvc0_hw_query_get(push, q, 0xc0 + 0x00, 0x00801002); /* VFETCH, VERTICES */
      nvc0_hw_query_get(push, q, 0xc0 + 0x10, 0x01801002); /* VFETCH, PRIMS */
      nvc0_hw_query_get(push, q, 0xc0 + 0x20, 0x02802002); /* VP, LAUNCHES */
      nvc0_hw_query_get(push, q, 0xc0 + 0x30, 0x03806002); /* GP, LAUNCHES */
      nvc0_hw_query_get(push, q, 0xc0 + 0x40, 0x04806002); /* GP, PRIMS_OUT */
      nvc0_hw_query_get(push, q, 0xc0 + 0x50, 0x07804002); /* RAST, PRIMS_IN */
      nvc0_hw_query_get(push, q, 0xc0 + 0x60, 0x08804002); /* RAST, PRIMS_OUT */
      nvc0_hw_query_get(push, q, 0xc0 + 0x70, 0x0980a002); /* ROP, PIXELS */
      nvc0_hw_query_get(push, q, 0xc0 + 0x80, 0x0d808002); /* TCP, LAUNCHES */
      nvc0_hw_query_get(push, q, 0xc0 + 0x90, 0x0e809002); /* TEP, LAUNCHES */
      nvc0_hw_query_write_compute_invocations(nvc0, hq, 0xc0 + 0xa0);
      break;
   default:
      break;
   }
   hq->state = NVC0_HW_QUERY_STATE_ACTIVE;
   return ret;
}

static void
nvc0_hw_end_query(struct nvc0_context *nvc0, struct nvc0_query *q)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_hw_query *hq = nvc0_hw_query(q);

   if (hq->funcs && hq->funcs->end_query) {
      hq->funcs->end_query(nvc0, hq);
      return;
   }

   if (hq->state != NVC0_HW_QUERY_STATE_ACTIVE) {
      /* some queries don't require 'begin' to be called (e.g. GPU_FINISHED) */
      if (hq->rotate)
         nvc0_hw_query_rotate(nvc0, q);
      hq->sequence++;
   }
   hq->state = NVC0_HW_QUERY_STATE_ENDED;

   switch (q->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      nvc0_hw_query_get(push, q, 0, 0x0100f002);
      if (--nvc0->screen->num_occlusion_queries_active == 0) {
         PUSH_SPACE(push, 1);
         IMMED_NVC0(push, NVC0_3D(SAMPLECNT_ENABLE), 0);
      }
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      nvc0_hw_query_get(push, q, 0, 0x09005002 | (q->index << 5));
      break;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      nvc0_hw_query_get(push, q, 0, 0x05805002 | (q->index << 5));
      break;
   case PIPE_QUERY_SO_STATISTICS:
      nvc0_hw_query_get(push, q, 0x00, 0x05805002 | (q->index << 5));
      nvc0_hw_query_get(push, q, 0x10, 0x06805002 | (q->index << 5));
      break;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      nvc0_hw_query_get(push, q, 0x00, 0x03005002 | (q->index << 5));
      break;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      /* XXX: This get actually writes the number of overflowed streams */
      nvc0_hw_query_get(push, q, 0x00, 0x0f005002);
      break;
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIME_ELAPSED:
      nvc0_hw_query_get(push, q, 0, 0x00005002);
      break;
   case PIPE_QUERY_GPU_FINISHED:
      nvc0_hw_query_get(push, q, 0, 0x1000f010);
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      nvc0_hw_query_get(push, q, 0x00, 0x00801002); /* VFETCH, VERTICES */
      nvc0_hw_query_get(push, q, 0x10, 0x01801002); /* VFETCH, PRIMS */
      nvc0_hw_query_get(push, q, 0x20, 0x02802002); /* VP, LAUNCHES */
      nvc0_hw_query_get(push, q, 0x30, 0x03806002); /* GP, LAUNCHES */
      nvc0_hw_query_get(push, q, 0x40, 0x04806002); /* GP, PRIMS_OUT */
      nvc0_hw_query_get(push, q, 0x50, 0x07804002); /* RAST, PRIMS_IN */
      nvc0_hw_query_get(push, q, 0x60, 0x08804002); /* RAST, PRIMS_OUT */
      nvc0_hw_query_get(push, q, 0x70, 0x0980a002); /* ROP, PIXELS */
      nvc0_hw_query_get(push, q, 0x80, 0x0d808002); /* TCP, LAUNCHES */
      nvc0_hw_query_get(push, q, 0x90, 0x0e809002); /* TEP, LAUNCHES */
      nvc0_hw_query_write_compute_invocations(nvc0, hq, 0xa0);
      break;
   case PIPE_QUERY_TIMESTAMP_DISJOINT:
      /* This query is not issued on GPU because disjoint is forced to false */
      hq->state = NVC0_HW_QUERY_STATE_READY;
      break;
   case NVC0_HW_QUERY_TFB_BUFFER_OFFSET:
      /* indexed by TFB buffer instead of by vertex stream */
      nvc0_hw_query_get(push, q, 0x00, 0x0d005002 | (q->index << 5));
      break;
   default:
      break;
   }
   if (hq->is64bit)
      nouveau_fence_ref(nvc0->base.fence, &hq->fence);
}

static bool
nvc0_hw_get_query_result(struct nvc0_context *nvc0, struct nvc0_query *q,
                         bool wait, union pipe_query_result *result)
{
   struct nvc0_hw_query *hq = nvc0_hw_query(q);
   uint64_t *res64 = (uint64_t*)result;
   uint32_t *res32 = (uint32_t*)result;
   uint8_t *res8 = (uint8_t*)result;
   uint64_t *data64 = (uint64_t *)hq->data;
   unsigned i;

   if (hq->funcs && hq->funcs->get_query_result)
      return hq->funcs->get_query_result(nvc0, hq, wait, result);

   if (hq->state != NVC0_HW_QUERY_STATE_READY)
      nvc0_hw_query_update(nvc0->base.client, q);

   if (hq->state != NVC0_HW_QUERY_STATE_READY) {
      if (!wait) {
         if (hq->state != NVC0_HW_QUERY_STATE_FLUSHED) {
            hq->state = NVC0_HW_QUERY_STATE_FLUSHED;
            /* flush for silly apps that spin on GL_QUERY_RESULT_AVAILABLE */
            PUSH_KICK(nvc0->base.pushbuf);
         }
         return false;
      }
      if (BO_WAIT(&nvc0->screen->base, hq->bo, NOUVEAU_BO_RD, nvc0->base.client))
         return false;
      NOUVEAU_DRV_STAT(&nvc0->screen->base, query_sync_count, 1);
   }
   hq->state = NVC0_HW_QUERY_STATE_READY;

   switch (q->type) {
   case PIPE_QUERY_GPU_FINISHED:
      res8[0] = true;
      break;
   case PIPE_QUERY_OCCLUSION_COUNTER: /* u32 sequence, u32 count, u64 time */
      res64[0] = hq->data[1] - hq->data[5];
      break;
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      res8[0] = hq->data[1] != hq->data[5];
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED: /* u64 count, u64 time */
   case PIPE_QUERY_PRIMITIVES_EMITTED: /* u64 count, u64 time */
      res64[0] = data64[0] - data64[2];
      break;
   case PIPE_QUERY_SO_STATISTICS:
      res64[0] = data64[0] - data64[4];
      res64[1] = data64[2] - data64[6];
      break;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      res8[0] = data64[0] != data64[2];
      break;
   case PIPE_QUERY_TIMESTAMP:
      res64[0] = data64[1];
      break;
   case PIPE_QUERY_TIMESTAMP_DISJOINT:
      res64[0] = 1000000000;
      res8[8] = false;
      break;
   case PIPE_QUERY_TIME_ELAPSED:
      res64[0] = data64[1] - data64[3];
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      for (i = 0; i < 11; ++i)
         res64[i] = data64[i * 2] - data64[24 + i * 2];
      break;
   case NVC0_HW_QUERY_TFB_BUFFER_OFFSET:
      res32[0] = hq->data[1];
      break;
   default:
      assert(0); /* can't happen, we don't create queries with invalid type */
      return false;
   }

   return true;
}

static void
nvc0_hw_get_query_result_resource(struct nvc0_context *nvc0,
                                  struct nvc0_query *q,
                                  enum pipe_query_flags flags,
                                  enum pipe_query_value_type result_type,
                                  int index,
                                  struct pipe_resource *resource,
                                  unsigned offset)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_hw_query *hq = nvc0_hw_query(q);
   struct nv04_resource *buf = nv04_resource(resource);
   unsigned qoffset = 0, stride;

   assert(!hq->funcs || !hq->funcs->get_query_result);

   if (index == -1) {
      /* TODO: Use a macro to write the availability of the query */
      if (hq->state != NVC0_HW_QUERY_STATE_READY)
         nvc0_hw_query_update(nvc0->base.client, q);
      uint32_t ready[2] = {hq->state == NVC0_HW_QUERY_STATE_READY};
      nvc0->base.push_cb(&nvc0->base, buf, offset,
                         result_type >= PIPE_QUERY_TYPE_I64 ? 2 : 1,
                         ready);

      util_range_add(&buf->base, &buf->valid_buffer_range, offset,
                     offset + (result_type >= PIPE_QUERY_TYPE_I64 ? 8 : 4));

      nvc0_resource_validate(nvc0, buf, NOUVEAU_BO_WR);

      return;
   }

   /* If the fence guarding this query has not been emitted, that makes a lot
    * of the following logic more complicated.
    */
   if (hq->is64bit)
      nouveau_fence_next_if_current(&nvc0->base, hq->fence);

   /* We either need to compute a 32- or 64-bit difference between 2 values,
    * and then store the result as either a 32- or 64-bit value. As such let's
    * treat all inputs as 64-bit (and just push an extra 0 for the 32-bit
    * ones), and have one macro that clamps result to i32, u32, or just
    * outputs the difference (no need to worry about 64-bit clamping).
    */
   if (hq->state != NVC0_HW_QUERY_STATE_READY)
      nvc0_hw_query_update(nvc0->base.client, q);

   if ((flags & PIPE_QUERY_WAIT) && hq->state != NVC0_HW_QUERY_STATE_READY)
      nvc0_hw_query_fifo_wait(nvc0, q);

   PUSH_SPACE_EX(push, 32, 2, 3);
   PUSH_REF1 (push, hq->bo, NOUVEAU_BO_GART | NOUVEAU_BO_RD);
   PUSH_REF1 (push, buf->bo, buf->domain | NOUVEAU_BO_WR);
   BEGIN_1IC0(push, NVC0_3D(MACRO_QUERY_BUFFER_WRITE), 9);
   switch (q->type) {
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE: /* XXX what if 64-bit? */
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      PUSH_DATA(push, 0x00000001);
      break;
   default:
      if (result_type == PIPE_QUERY_TYPE_I32)
         PUSH_DATA(push, 0x7fffffff);
      else if (result_type == PIPE_QUERY_TYPE_U32)
         PUSH_DATA(push, 0xffffffff);
      else
         PUSH_DATA(push, 0x00000000);
      break;
   }

   switch (q->type) {
   case PIPE_QUERY_SO_STATISTICS:
      stride = 2;
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      stride = 12;
      break;
   case PIPE_QUERY_TIME_ELAPSED:
   case PIPE_QUERY_TIMESTAMP:
      qoffset = 8;
      FALLTHROUGH;
   default:
      assert(index == 0);
      stride = 1;
      break;
   }

   if (hq->is64bit || qoffset) {
      nouveau_pushbuf_data(push, hq->bo, hq->offset + qoffset + 16 * index,
                           8 | NVC0_IB_ENTRY_1_NO_PREFETCH);
      if (q->type == PIPE_QUERY_TIMESTAMP) {
         PUSH_DATA(push, 0);
         PUSH_DATA(push, 0);
      } else {
         nouveau_pushbuf_data(push, hq->bo, hq->offset + qoffset +
                              16 * (index + stride),
                              8 | NVC0_IB_ENTRY_1_NO_PREFETCH);
      }
   } else {
      nouveau_pushbuf_data(push, hq->bo, hq->offset + 4,
                           4 | NVC0_IB_ENTRY_1_NO_PREFETCH);
      PUSH_DATA(push, 0);
      nouveau_pushbuf_data(push, hq->bo, hq->offset + 16 + 4,
                           4 | NVC0_IB_ENTRY_1_NO_PREFETCH);
      PUSH_DATA(push, 0);
   }

   if ((flags & PIPE_QUERY_WAIT) || hq->state == NVC0_HW_QUERY_STATE_READY) {
      PUSH_DATA(push, 0);
      PUSH_DATA(push, 0);
   } else if (hq->is64bit) {
      PUSH_DATA(push, hq->fence->sequence);
      nouveau_pushbuf_data(push, nvc0->screen->fence.bo, 0,
                           4 | NVC0_IB_ENTRY_1_NO_PREFETCH);
   } else {
      PUSH_DATA(push, hq->sequence);
      nouveau_pushbuf_data(push, hq->bo, hq->offset,
                           4 | NVC0_IB_ENTRY_1_NO_PREFETCH);
   }
   PUSH_DATAh(push, buf->address + offset);
   PUSH_DATA (push, buf->address + offset);

   util_range_add(&buf->base, &buf->valid_buffer_range, offset,
                  offset + (result_type >= PIPE_QUERY_TYPE_I64 ? 8 : 4));

   nvc0_resource_validate(nvc0, buf, NOUVEAU_BO_WR);
}

static const struct nvc0_query_funcs hw_query_funcs = {
   .destroy_query = nvc0_hw_destroy_query,
   .begin_query = nvc0_hw_begin_query,
   .end_query = nvc0_hw_end_query,
   .get_query_result = nvc0_hw_get_query_result,
   .get_query_result_resource = nvc0_hw_get_query_result_resource,
};

struct nvc0_query *
nvc0_hw_create_query(struct nvc0_context *nvc0, unsigned type, unsigned index)
{
   struct nvc0_hw_query *hq;
   struct nvc0_query *q;
   unsigned space = NVC0_HW_QUERY_ALLOC_SPACE;

   hq = nvc0_hw_sm_create_query(nvc0, type);
   if (hq) {
      hq->base.funcs = &hw_query_funcs;
      return (struct nvc0_query *)hq;
   }

   hq = nvc0_hw_metric_create_query(nvc0, type);
   if (hq) {
      hq->base.funcs = &hw_query_funcs;
      return (struct nvc0_query *)hq;
   }

   hq = CALLOC_STRUCT(nvc0_hw_query);
   if (!hq)
      return NULL;

   q = &hq->base;
   q->funcs = &hw_query_funcs;
   q->type = type;
   q->index = index;

   switch (q->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      hq->rotate = 32;
      space = NVC0_HW_QUERY_ALLOC_SPACE;
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      hq->is64bit = true;
      space = 512;
      break;
   case PIPE_QUERY_SO_STATISTICS:
      hq->is64bit = true;
      space = 64;
      break;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      hq->is64bit = true;
      space = 32;
      break;
   case PIPE_QUERY_TIME_ELAPSED:
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIMESTAMP_DISJOINT:
   case PIPE_QUERY_GPU_FINISHED:
      space = 32;
      break;
   case NVC0_HW_QUERY_TFB_BUFFER_OFFSET:
      space = 16;
      break;
   default:
      debug_printf("invalid query type: %u\n", type);
      FREE(q);
      return NULL;
   }

   if (!nvc0_hw_query_allocate(nvc0, q, space)) {
      FREE(hq);
      return NULL;
   }

   if (hq->rotate) {
      /* we advance before query_begin ! */
      hq->offset -= hq->rotate;
      hq->data -= hq->rotate / sizeof(*hq->data);
   } else
   if (!hq->is64bit)
      hq->data[0] = 0; /* initialize sequence */

   return q;
}

int
nvc0_hw_get_driver_query_info(struct nvc0_screen *screen, unsigned id,
                              struct pipe_driver_query_info *info)
{
   int num_hw_sm_queries = 0, num_hw_metric_queries = 0;

   num_hw_sm_queries = nvc0_hw_sm_get_driver_query_info(screen, 0, NULL);
   num_hw_metric_queries =
      nvc0_hw_metric_get_driver_query_info(screen, 0, NULL);

   if (!info)
      return num_hw_sm_queries + num_hw_metric_queries;

   if (id < num_hw_sm_queries)
      return nvc0_hw_sm_get_driver_query_info(screen, id, info);

   return nvc0_hw_metric_get_driver_query_info(screen,
                                               id - num_hw_sm_queries, info);
}

void
nvc0_hw_query_pushbuf_submit(struct nouveau_pushbuf *push,
                             struct nvc0_query *q, unsigned result_offset)
{
   struct nvc0_hw_query *hq = nvc0_hw_query(q);

   PUSH_REF1(push, hq->bo, NOUVEAU_BO_RD | NOUVEAU_BO_GART);
   nouveau_pushbuf_data(push, hq->bo, hq->offset + result_offset, 4 |
                        NVC0_IB_ENTRY_1_NO_PREFETCH);
}

void
nvc0_hw_query_fifo_wait(struct nvc0_context *nvc0, struct nvc0_query *q)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_hw_query *hq = nvc0_hw_query(q);
   unsigned offset = hq->offset;

   /* ensure the query's fence has been emitted */
   if (hq->is64bit)
      nouveau_fence_next_if_current(&nvc0->base, hq->fence);

   PUSH_SPACE(push, 5);
   PUSH_REF1 (push, hq->bo, NOUVEAU_BO_GART | NOUVEAU_BO_RD);
   BEGIN_NVC0(push, SUBC_3D(NV84_SUBCHAN_SEMAPHORE_ADDRESS_HIGH), 4);
   if (hq->is64bit) {
      PUSH_DATAh(push, nvc0->screen->fence.bo->offset);
      PUSH_DATA (push, nvc0->screen->fence.bo->offset);
      PUSH_DATA (push, hq->fence->sequence);
   } else {
      PUSH_DATAh(push, hq->bo->offset + offset);
      PUSH_DATA (push, hq->bo->offset + offset);
      PUSH_DATA (push, hq->sequence);
   }
   PUSH_DATA (push, (1 << 12) |
              NV84_SUBCHAN_SEMAPHORE_TRIGGER_ACQUIRE_GEQUAL);
}
