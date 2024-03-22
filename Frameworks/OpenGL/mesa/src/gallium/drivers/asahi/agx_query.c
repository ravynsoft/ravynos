/*
 * Copyright 2022 Alyssa Rosenzweig
 * Copyright 2019-2020 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include "pipe/p_defines.h"
#include "util/macros.h"
#include "util/u_inlines.h"
#include "util/u_prim.h"
#include "agx_device.h"
#include "agx_state.h"
#include "pool.h"

static struct pipe_query *
agx_create_query(struct pipe_context *ctx, unsigned query_type, unsigned index)
{
   struct agx_query *query = calloc(1, sizeof(struct agx_query));

   query->type = query_type;
   query->index = index;

   return (struct pipe_query *)query;
}

static bool
is_occlusion(struct agx_query *query)
{
   switch (query->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      return true;
   default:
      return false;
   }
}

static bool
is_timer(struct agx_query *query)
{
   switch (query->type) {
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIME_ELAPSED:
      return true;
   default:
      return false;
   }
}

static void
agx_destroy_query(struct pipe_context *pctx, struct pipe_query *pquery)
{
   struct agx_context *ctx = agx_context(pctx);
   struct agx_query *query = (struct agx_query *)pquery;

   /* It is legal for the query to be destroyed before its value is read,
    * particularly during application teardown. In this case, don't leave a
    * dangling reference to the query.
    */
   if (query->writer) {
      assert(!is_timer(query) && "single writer not used here");

      struct agx_batch *writer = query->writer;
      struct util_dynarray *array = is_occlusion(query)
                                       ? &writer->occlusion_queries
                                       : &writer->nonocclusion_queries;
      struct agx_query **ptr =
         util_dynarray_element(array, struct agx_query *, query->writer_index);

      assert((*ptr) == query && "data structure invariant");
      *ptr = NULL;
   } else if (is_timer(query)) {
      /* Potentially has many writers! We need them all to synchronize so they
       * don't have dangling references. Syncing will destroy batches that hold
       * references as required.
       *
       * TODO: Optimize this, timestamp queries are bonkers on tilers.
       */
      agx_flush_all(ctx, "Destroying time query");
      agx_sync_all(ctx, "Destroying time query");
   }

   free(query);
}

static bool
agx_begin_query(struct pipe_context *pctx, struct pipe_query *pquery)
{
   struct agx_context *ctx = agx_context(pctx);
   struct agx_query *query = (struct agx_query *)pquery;

   ctx->dirty |= AGX_DIRTY_QUERY;

   switch (query->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      ctx->occlusion_query = query;
      break;

   case PIPE_QUERY_PRIMITIVES_GENERATED:
      ctx->prims_generated[query->index] = query;
      break;

   case PIPE_QUERY_PRIMITIVES_EMITTED:
      ctx->tf_prims_generated[query->index] = query;
      break;

   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      ctx->tf_overflow[query->index] = query;
      break;

   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      ctx->tf_any_overflow = query;
      break;

   case PIPE_QUERY_TIME_ELAPSED:
      ctx->time_elapsed = query;
      query->timestamp_begin = UINT64_MAX;
      query->timestamp_end = 0;
      return true;

   case PIPE_QUERY_TIMESTAMP:
      /* No-op */
      break;

   default:
      return false;
   }

   /* begin_query zeroes, flush so we can do that write. If anything (i.e.
    * other than piglit) actually hits this, we could shadow the query to
    * avoid the flush.
    */
   if (query->writer) {
      agx_flush_batch_for_reason(ctx, query->writer, "Query overwritten");
      agx_sync_batch_for_reason(ctx, query->writer, "Query overwrriten");
   }

   assert(query->writer == NULL);
   query->value = 0;
   return true;
}

static bool
agx_end_query(struct pipe_context *pctx, struct pipe_query *pquery)
{
   struct agx_context *ctx = agx_context(pctx);
   struct agx_device *dev = agx_device(pctx->screen);
   struct agx_query *query = (struct agx_query *)pquery;

   ctx->dirty |= AGX_DIRTY_QUERY;

   switch (query->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      ctx->occlusion_query = NULL;
      return true;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      ctx->prims_generated[query->index] = NULL;
      return true;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      ctx->tf_prims_generated[query->index] = NULL;
      return true;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      ctx->tf_overflow[query->index] = NULL;
      return true;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      ctx->tf_any_overflow = NULL;
      return true;
   case PIPE_QUERY_TIME_ELAPSED:
      ctx->time_elapsed = NULL;
      return true;
   case PIPE_QUERY_TIMESTAMP:
      /* Timestamp logically written now, set up batches to MAX their finish
       * time in. If there are no batches, it's just the current time stamp.
       */
      agx_add_timestamp_end_query(ctx, query);

      query->timestamp_end = agx_get_gpu_timestamp(dev);

      return true;
   default:
      return false;
   }
}

static bool
agx_get_query_result(struct pipe_context *pctx, struct pipe_query *pquery,
                     bool wait, union pipe_query_result *vresult)
{
   struct agx_query *query = (struct agx_query *)pquery;
   struct agx_context *ctx = agx_context(pctx);
   struct agx_device *dev = agx_device(pctx->screen);

   /* For GPU queries, flush the writer. When the writer is flushed, the GPU
    * will write the value, and when we wait for the writer, the CPU will read
    * the value into query->value.
    */
   if (query->writer != NULL) {
      /* Querying the result forces a query to finish in finite time, so we
       * need to flush. Furthermore, we need all earlier queries
       * to finish before this query, so we sync unconditionally (so we can
       * maintain the lie that all queries are finished when read).
       *
       * TODO: Optimize based on wait flag.
       */
      struct agx_batch *writer = query->writer;
      agx_flush_batch_for_reason(ctx, writer, "GPU query");
      agx_sync_batch_for_reason(ctx, writer, "GPU query");
   } else if (query->type == PIPE_QUERY_TIMESTAMP ||
              query->type == PIPE_QUERY_TIME_ELAPSED) {
      /* TODO: Optimize this... timestamp queries are bonkers on tilers. */
      agx_flush_all(ctx, "Timestamp query");
      agx_sync_all(ctx, "Timestamp query");
   }

   /* After syncing, there is no writer left, so query->value is ready */
   assert(query->writer == NULL && "cleared when cleaning up batch");

   switch (query->type) {
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      vresult->b = query->value;
      return true;

   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      vresult->b = query->value > 0;
      return true;

   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      vresult->u64 = query->value;
      return true;

   case PIPE_QUERY_TIMESTAMP:
      vresult->u64 = agx_gpu_time_to_ns(dev, query->timestamp_end);
      return true;

   case PIPE_QUERY_TIME_ELAPSED:
      vresult->u64 =
         agx_gpu_time_to_ns(dev, query->timestamp_end - query->timestamp_begin);
      return true;

   default:
      unreachable("Other queries not yet supported");
   }
}

static void
agx_get_query_result_resource(struct pipe_context *pipe, struct pipe_query *q,
                              enum pipe_query_flags flags,
                              enum pipe_query_value_type result_type, int index,
                              struct pipe_resource *resource, unsigned offset)
{
   struct agx_query *query = (struct agx_query *)q;

   /* TODO: Don't cheat XXX */
   struct agx_context *ctx = agx_context(pipe);
   agx_sync_all(ctx, "Stubbed QBOs");

   union pipe_query_result result;
   if (index < 0) {
      /* availability */
      result.u64 = 1;
   } else {
      bool ready = agx_get_query_result(pipe, q, true, &result);
      assert(ready);
   }

   switch (query->type) {
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      result.u32 = result.b;
      break;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      result.u32 = (bool)(result.u32 > 0);
      break;
   default:
      break;
   }

   /* Clamp to type, arb_query_buffer_object-qbo tests */
   if (result_type == PIPE_QUERY_TYPE_U32) {
      result.u32 = MIN2(result.u64, u_uintN_max(32));
   } else if (result_type == PIPE_QUERY_TYPE_I32) {
      int64_t x = result.u64;
      x = MAX2(MIN2(x, u_intN_max(32)), u_intN_min(32));
      result.u32 = x;
   }

   if (result_type <= PIPE_QUERY_TYPE_U32)
      pipe_buffer_write(pipe, resource, offset, 4, &result.u32);
   else
      pipe_buffer_write(pipe, resource, offset, 8, &result.u64);
}

static void
agx_set_active_query_state(struct pipe_context *pipe, bool enable)
{
   struct agx_context *ctx = agx_context(pipe);

   ctx->active_queries = enable;
   ctx->dirty |= AGX_DIRTY_QUERY;
}

static uint16_t
agx_add_query_to_batch(struct agx_batch *batch, struct agx_query *query,
                       struct util_dynarray *array)
{
   /* If written by another batch, flush it now. If this affects real apps, we
    * could avoid this flush by merging query results.
    */
   if (query->writer && query->writer != batch) {
      agx_flush_batch_for_reason(batch->ctx, query->writer,
                                 "Multiple query writers");
   }

   /* Allocate if needed */
   if (query->writer == NULL) {
      query->writer = batch;
      query->writer_index =
         util_dynarray_num_elements(array, struct agx_query *);

      util_dynarray_append(array, struct agx_query *, query);
   }

   assert(query->writer == batch);
   assert(*util_dynarray_element(array, struct agx_query *,
                                 query->writer_index) == query);

   return query->writer_index;
}

uint16_t
agx_get_oq_index(struct agx_batch *batch, struct agx_query *query)
{
   assert(is_occlusion(query));

   return agx_add_query_to_batch(batch, query, &batch->occlusion_queries);
}

uint64_t
agx_get_query_address(struct agx_batch *batch, struct agx_query *query)
{
   assert(!is_occlusion(query));

   agx_add_query_to_batch(batch, query, &batch->nonocclusion_queries);

   /* Allocate storage for the query in the batch */
   if (!query->ptr.cpu) {
      query->ptr = agx_pool_alloc_aligned(&batch->pool, sizeof(uint64_t),
                                          sizeof(uint64_t));

      uint64_t *value = query->ptr.cpu;
      *value = 0;
   }

   return query->ptr.gpu;
}

void
agx_finish_batch_queries(struct agx_batch *batch, uint64_t begin_ts,
                         uint64_t end_ts)
{
   uint64_t *occlusion = (uint64_t *)batch->occlusion_buffer.cpu;

   util_dynarray_foreach(&batch->occlusion_queries, struct agx_query *, it) {
      struct agx_query *query = *it;

      /* Skip queries that have since been destroyed */
      if (query == NULL)
         continue;

      assert(query->writer == batch);

      /* Get the result for this batch. If occlusion is NULL, it means that no
       * draws actually enabled any occlusion queries, so there's no change.
       */
      if (occlusion != NULL) {
         uint64_t result = *(occlusion++);

         /* Accumulate with the previous result (e.g. in case we split a frame
          * into multiple batches so an API-level query spans multiple batches).
          */
         if (query->type == PIPE_QUERY_OCCLUSION_COUNTER)
            query->value += result;
         else
            query->value |= (!!result);
      }

      query->writer = NULL;
      query->writer_index = 0;
   }

   /* Now handle non-occlusion queries in a similar way */
   util_dynarray_foreach(&batch->nonocclusion_queries, struct agx_query *, it) {
      struct agx_query *query = *it;
      if (query == NULL)
         continue;

      assert(query->writer == batch);

      /* Accumulate */
      uint64_t *value = query->ptr.cpu;
      query->value += (*value);
      query->writer = NULL;
      query->writer_index = 0;
      query->ptr.cpu = NULL;
      query->ptr.gpu = 0;
   }

   util_dynarray_foreach(&batch->timestamp_queries, struct agx_query *, it) {
      struct agx_query *query = *it;
      if (query == NULL)
         continue;

      query->timestamp_begin = MIN2(query->timestamp_begin, begin_ts);
      query->timestamp_end = MAX2(query->timestamp_end, end_ts);
   }
}

static void
agx_render_condition(struct pipe_context *pipe, struct pipe_query *query,
                     bool condition, enum pipe_render_cond_flag mode)
{
   struct agx_context *ctx = agx_context(pipe);

   ctx->cond_query = query;
   ctx->cond_cond = condition;
   ctx->cond_mode = mode;
}

bool
agx_render_condition_check_inner(struct agx_context *ctx)
{
   assert(ctx->cond_query != NULL && "precondition");

   perf_debug_ctx(ctx, "Implementing conditional rendering on the CPU");

   union pipe_query_result res = {0};
   bool wait = ctx->cond_mode != PIPE_RENDER_COND_NO_WAIT &&
               ctx->cond_mode != PIPE_RENDER_COND_BY_REGION_NO_WAIT;

   struct pipe_query *pq = (struct pipe_query *)ctx->cond_query;

   if (agx_get_query_result(&ctx->base, pq, wait, &res))
      return res.u64 != ctx->cond_cond;

   return true;
}

void
agx_init_query_functions(struct pipe_context *pctx)
{
   pctx->create_query = agx_create_query;
   pctx->destroy_query = agx_destroy_query;
   pctx->begin_query = agx_begin_query;
   pctx->end_query = agx_end_query;
   pctx->get_query_result = agx_get_query_result;
   pctx->get_query_result_resource = agx_get_query_result_resource;
   pctx->set_active_query_state = agx_set_active_query_state;
   pctx->render_condition = agx_render_condition;

   /* By default queries are active */
   agx_context(pctx)->active_queries = true;
}
