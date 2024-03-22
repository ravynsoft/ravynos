/*
 * Copyright © 2019 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <xf86drm.h>

#include "iris_context.h"
#include "iris_perf.h"

#include "main/mtypes.h"

struct iris_perf_query {
   struct gl_perf_query_object base;
   struct intel_perf_query_object *query;
   bool begin_succeeded;
};

static unsigned
iris_init_perf_query_info(struct pipe_context *pipe)
{
   struct iris_context *ice = (void *) pipe;
   struct iris_screen *screen = (struct iris_screen *) ice->ctx.screen;
   struct intel_perf_config *perf_cfg = NULL;

   /* make sure pipe perf counter type/data-type enums are matched with intel_perf's */
   STATIC_ASSERT(PIPE_PERF_COUNTER_TYPE_EVENT == (enum pipe_perf_counter_type)INTEL_PERF_COUNTER_TYPE_EVENT);
   STATIC_ASSERT(PIPE_PERF_COUNTER_TYPE_DURATION_NORM == (enum pipe_perf_counter_type)INTEL_PERF_COUNTER_TYPE_DURATION_NORM);
   STATIC_ASSERT(PIPE_PERF_COUNTER_TYPE_DURATION_RAW == (enum pipe_perf_counter_type)INTEL_PERF_COUNTER_TYPE_DURATION_RAW);
   STATIC_ASSERT(PIPE_PERF_COUNTER_TYPE_THROUGHPUT == (enum pipe_perf_counter_type)INTEL_PERF_COUNTER_TYPE_THROUGHPUT);
   STATIC_ASSERT(PIPE_PERF_COUNTER_TYPE_RAW == (enum pipe_perf_counter_type)INTEL_PERF_COUNTER_TYPE_RAW);

   STATIC_ASSERT(PIPE_PERF_COUNTER_DATA_TYPE_BOOL32 == (enum pipe_perf_counter_data_type)INTEL_PERF_COUNTER_DATA_TYPE_BOOL32);
   STATIC_ASSERT(PIPE_PERF_COUNTER_DATA_TYPE_UINT32 == (enum pipe_perf_counter_data_type)INTEL_PERF_COUNTER_DATA_TYPE_UINT32);
   STATIC_ASSERT(PIPE_PERF_COUNTER_DATA_TYPE_UINT64 == (enum pipe_perf_counter_data_type)INTEL_PERF_COUNTER_DATA_TYPE_UINT64);
   STATIC_ASSERT(PIPE_PERF_COUNTER_DATA_TYPE_FLOAT == (enum pipe_perf_counter_data_type)INTEL_PERF_COUNTER_DATA_TYPE_FLOAT);
   STATIC_ASSERT(PIPE_PERF_COUNTER_DATA_TYPE_DOUBLE == (enum pipe_perf_counter_data_type)INTEL_PERF_COUNTER_DATA_TYPE_DOUBLE);

   if (!ice->perf_ctx)
      ice->perf_ctx = intel_perf_new_context(ice);

   if (unlikely(!ice->perf_ctx))
      return 0;

   perf_cfg = intel_perf_config(ice->perf_ctx);

   if (perf_cfg)
      return perf_cfg->n_queries;

   perf_cfg = intel_perf_new(ice->perf_ctx);

   iris_perf_init_vtbl(perf_cfg);

   intel_perf_init_metrics(perf_cfg, screen->devinfo, screen->fd,
                           true /* pipeline_statistics */,
                           true /* register snapshots */);

   intel_perf_init_context(ice->perf_ctx,
                         perf_cfg,
                         ice,
                         ice,
                         screen->bufmgr,
                         screen->devinfo,
                         ice->batches[IRIS_BATCH_RENDER].i915.ctx_id,
                         screen->fd);

   return perf_cfg->n_queries;
}

static struct pipe_query *
iris_new_perf_query_obj(struct pipe_context *pipe, unsigned query_index)
{
   struct iris_context *ice = (void *) pipe;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;
   struct intel_perf_query_object * obj =
      intel_perf_new_query(perf_ctx, query_index);
   if (unlikely(!obj))
      return NULL;

   struct iris_perf_query *q = calloc(1, sizeof(struct iris_perf_query));
   if (unlikely(!q)) {
      intel_perf_delete_query(perf_ctx, obj);
      return NULL;
   }

   q->query = obj;
   return (struct pipe_query *)&q->base;
}

static bool
iris_begin_perf_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct iris_context *ice = (void *) pipe;
   struct iris_perf_query *perf_query= (struct iris_perf_query *) q;
   struct intel_perf_query_object *obj = perf_query->query;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;

   return (perf_query->begin_succeeded = intel_perf_begin_query(perf_ctx, obj));
}

static void
iris_end_perf_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct iris_context *ice = (void *) pipe;
   struct iris_perf_query *perf_query = (struct iris_perf_query *) q;
   struct intel_perf_query_object *obj = perf_query->query;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;

   if (perf_query->begin_succeeded)
      intel_perf_end_query(perf_ctx, obj);
}

static void
iris_delete_perf_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct iris_context *ice = (void *) pipe;
   struct iris_perf_query *perf_query = (struct iris_perf_query *) q;
   struct intel_perf_query_object *obj = perf_query->query;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;

   intel_perf_delete_query(perf_ctx, obj);
   free(q);
}

static void
iris_get_perf_query_info(struct pipe_context *pipe,
                         unsigned query_index,
                         const char **name,
                         uint32_t *data_size,
                         uint32_t *n_counters,
                         uint32_t *n_active)
{
   struct iris_context *ice = (void *) pipe;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;
   struct intel_perf_config *perf_cfg = intel_perf_config(perf_ctx);
   const struct intel_perf_query_info *info = &perf_cfg->queries[query_index];

   *name = info->name;
   *data_size = info->data_size;
   *n_counters = info->n_counters;
   *n_active = intel_perf_active_queries(perf_ctx, info);
}

static void
iris_get_perf_counter_info(struct pipe_context *pipe,
                           unsigned query_index,
                           unsigned counter_index,
                           const char **name,
                           const char **desc,
                           uint32_t *offset,
                           uint32_t *data_size,
                           uint32_t *type_enum,
                           uint32_t *data_type_enum,
                           uint64_t *raw_max)
{
   struct iris_context *ice = (void *) pipe;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;
   struct intel_perf_config *perf_cfg = intel_perf_config(perf_ctx);
   const struct intel_perf_query_info *info = &perf_cfg->queries[query_index];
   const struct intel_perf_query_counter *counter =
      &info->counters[counter_index];
   struct intel_perf_query_result results;

   intel_perf_query_result_clear(&results);

   *name = INTEL_DEBUG(DEBUG_PERF_SYMBOL_NAMES) ?
      counter->symbol_name : counter->name;
   *desc = counter->desc;
   *offset = counter->offset;
   *data_size = intel_perf_query_counter_get_size(counter);
   *type_enum = counter->type;
   *data_type_enum = counter->data_type;

   if (counter->oa_counter_max_uint64) {
      if (counter->data_type == INTEL_PERF_COUNTER_DATA_TYPE_FLOAT ||
          counter->data_type == INTEL_PERF_COUNTER_DATA_TYPE_DOUBLE)
         *raw_max = counter->oa_counter_max_float(perf_cfg, info, &results);
      else
         *raw_max = counter->oa_counter_max_uint64(perf_cfg, info, &results);
   } else {
      *raw_max = 0;
   }
}

static void
iris_wait_perf_query(struct pipe_context *pipe, struct pipe_query *q)
{
   struct iris_context *ice = (void *) pipe;
   struct iris_perf_query *perf_query = (struct iris_perf_query *) q;
   struct intel_perf_query_object *obj = perf_query->query;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;

   if (perf_query->begin_succeeded)
      intel_perf_wait_query(perf_ctx, obj, &ice->batches[IRIS_BATCH_RENDER]);
}

static bool
iris_is_perf_query_ready(struct pipe_context *pipe, struct pipe_query *q)
{
   struct iris_context *ice = (void *) pipe;
   struct iris_perf_query *perf_query = (struct iris_perf_query *) q;
   struct intel_perf_query_object *obj = perf_query->query;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;

   if (perf_query->base.Ready)
      return true;
   if (!perf_query->begin_succeeded)
      return true;

   return intel_perf_is_query_ready(perf_ctx, obj,
                                    &ice->batches[IRIS_BATCH_RENDER]);
}

static bool
iris_get_perf_query_data(struct pipe_context *pipe,
                         struct pipe_query *q,
                         size_t data_size,
                         uint32_t *data,
                         uint32_t *bytes_written)
{
   struct iris_context *ice = (void *) pipe;
   struct iris_perf_query *perf_query = (struct iris_perf_query *) q;
   struct intel_perf_query_object *obj = perf_query->query;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;

   if (perf_query->begin_succeeded) {
      intel_perf_get_query_data(perf_ctx, obj, &ice->batches[IRIS_BATCH_RENDER],
            data_size, data, bytes_written);
   }

   return perf_query->begin_succeeded;
}

void
iris_init_perfquery_functions(struct pipe_context *ctx)
{
   ctx->init_intel_perf_query_info = iris_init_perf_query_info;
   ctx->get_intel_perf_query_info = iris_get_perf_query_info;
   ctx->get_intel_perf_query_counter_info = iris_get_perf_counter_info;
   ctx->new_intel_perf_query_obj = iris_new_perf_query_obj;
   ctx->begin_intel_perf_query = iris_begin_perf_query;
   ctx->end_intel_perf_query = iris_end_perf_query;
   ctx->delete_intel_perf_query = iris_delete_perf_query;
   ctx->wait_intel_perf_query = iris_wait_perf_query;
   ctx->is_intel_perf_query_ready = iris_is_perf_query_ready;
   ctx->get_intel_perf_query_data = iris_get_perf_query_data;
}
