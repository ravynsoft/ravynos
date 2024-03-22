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

#include "iris_monitor.h"

#include <xf86drm.h>

#include "iris_screen.h"
#include "iris_context.h"
#include "iris_perf.h"

struct iris_monitor_object {
   int num_active_counters;
   int *active_counters;

   size_t result_size;
   unsigned char *result_buffer;

   struct intel_perf_query_object *query;
};

int
iris_get_monitor_info(struct pipe_screen *pscreen, unsigned index,
                      struct pipe_driver_query_info *info)
{
   struct iris_screen *screen = (struct iris_screen *)pscreen;
   struct intel_perf_config *perf_cfg = screen->perf_cfg;
   assert(perf_cfg);
   if (!perf_cfg)
      return 0;

   if (!info) {
      /* return the number of metrics */
      return perf_cfg->n_counters;
   }

   struct intel_perf_query_counter_info *counter_info = &perf_cfg->counter_infos[index];
   struct intel_perf_query_info *query_info =
      &perf_cfg->queries[intel_perf_query_counter_info_first_query(counter_info)];
   struct intel_perf_query_counter *counter = counter_info->counter;
   struct intel_perf_query_result results;

   intel_perf_query_result_clear(&results);

   info->group_id = counter_info->location.group_idx;
   info->name = INTEL_DEBUG(DEBUG_PERF_SYMBOL_NAMES) ?
      counter->symbol_name : counter->name;
   info->query_type = PIPE_QUERY_DRIVER_SPECIFIC + index;

   if (counter->type == INTEL_PERF_COUNTER_TYPE_THROUGHPUT)
      info->result_type = PIPE_DRIVER_QUERY_RESULT_TYPE_AVERAGE;
   else
      info->result_type = PIPE_DRIVER_QUERY_RESULT_TYPE_CUMULATIVE;
   switch (counter->data_type) {
   case INTEL_PERF_COUNTER_DATA_TYPE_BOOL32:
   case INTEL_PERF_COUNTER_DATA_TYPE_UINT32: {
      info->type = PIPE_DRIVER_QUERY_TYPE_UINT;
      uint64_t val =
         counter->oa_counter_max_uint64 ?
         counter->oa_counter_max_uint64(perf_cfg, query_info, &results) : 0;
      assert(val <= UINT32_MAX);
      info->max_value.u32 = (uint32_t)val;
      break;
   }
   case INTEL_PERF_COUNTER_DATA_TYPE_UINT64:
      info->type = PIPE_DRIVER_QUERY_TYPE_UINT64;
      info->max_value.u64 =
         counter->oa_counter_max_uint64 ?
         counter->oa_counter_max_uint64(perf_cfg, query_info, &results) : 0;
      break;
   case INTEL_PERF_COUNTER_DATA_TYPE_FLOAT:
   case INTEL_PERF_COUNTER_DATA_TYPE_DOUBLE:
      info->type = PIPE_DRIVER_QUERY_TYPE_FLOAT;
      info->max_value.f =
         counter->oa_counter_max_float ?
         counter->oa_counter_max_float(perf_cfg, query_info, &results) : 0.0f;
      break;
   default:
      assert(false);
      break;
   }

   /* indicates that this is an OA query, not a pipeline statistics query */
   info->flags = PIPE_DRIVER_QUERY_FLAG_BATCH;
   return 1;
}

static bool
iris_monitor_init_metrics(struct iris_screen *screen)
{
   struct intel_perf_config *perf_cfg = intel_perf_new(screen);
   if (unlikely(!perf_cfg))
      return false;

   screen->perf_cfg = perf_cfg;

   iris_perf_init_vtbl(perf_cfg);

   intel_perf_init_metrics(perf_cfg, screen->devinfo, screen->fd,
                           true /* pipeline stats*/,
                           true /* register snapshots */);

   return perf_cfg->n_counters > 0;
}

int
iris_get_monitor_group_info(struct pipe_screen *pscreen,
                            unsigned group_index,
                            struct pipe_driver_query_group_info *info)
{
   struct iris_screen *screen = (struct iris_screen *)pscreen;
   if (!screen->perf_cfg) {
      if (!iris_monitor_init_metrics(screen))
         return 0;
   }

   const struct intel_perf_config *perf_cfg = screen->perf_cfg;

   if (!info) {
      /* return the count that can be queried */
      return perf_cfg->n_queries;
   }

   if (group_index >= perf_cfg->n_queries) {
      /* out of range */
      return 0;
   }

   struct intel_perf_query_info *query = &perf_cfg->queries[group_index];

   info->name = query->name;
   info->max_active_queries = query->n_counters;
   info->num_queries = query->n_counters;

   return 1;
}

static void
iris_init_monitor_ctx(struct iris_context *ice)
{
   struct iris_screen *screen = (struct iris_screen *) ice->ctx.screen;

   ice->perf_ctx = intel_perf_new_context(ice);
   if (unlikely(!ice->perf_ctx))
      return;

   struct intel_perf_context *perf_ctx = ice->perf_ctx;
   struct intel_perf_config *perf_cfg = screen->perf_cfg;
   intel_perf_init_context(perf_ctx,
                         perf_cfg,
                         ice,
                         ice,
                         screen->bufmgr,
                         screen->devinfo,
                         ice->batches[IRIS_BATCH_RENDER].i915.ctx_id,
                         screen->fd);
}

/* entry point for GenPerfMonitorsAMD */
struct iris_monitor_object *
iris_create_monitor_object(struct iris_context *ice,
                           unsigned num_queries,
                           unsigned *query_types)
{
   struct iris_screen *screen = (struct iris_screen *) ice->ctx.screen;
   struct intel_perf_config *perf_cfg = screen->perf_cfg;
   struct intel_perf_query_object *query_obj = NULL;

   /* initialize perf context if this has not already been done.  This
    * function is the first entry point that carries the gl context.
    */
   if (ice->perf_ctx == NULL) {
      iris_init_monitor_ctx(ice);
   }
   struct intel_perf_context *perf_ctx = ice->perf_ctx;

   assert(num_queries > 0);
   int query_index = query_types[0] - PIPE_QUERY_DRIVER_SPECIFIC;
   assert(query_index <= perf_cfg->n_counters);
   const int group = perf_cfg->counter_infos[query_index].location.group_idx;

   struct iris_monitor_object *monitor =
      calloc(1, sizeof(struct iris_monitor_object));
   if (unlikely(!monitor))
      goto allocation_failure;

   monitor->num_active_counters = num_queries;
   monitor->active_counters = calloc(num_queries, sizeof(int));
   if (unlikely(!monitor->active_counters))
      goto allocation_failure;

   for (int i = 0; i < num_queries; ++i) {
      unsigned current_query = query_types[i];
      unsigned current_query_index = current_query - PIPE_QUERY_DRIVER_SPECIFIC;

      /* all queries must be in the same group */
      assert(current_query_index <= perf_cfg->n_counters);
      assert(perf_cfg->counter_infos[current_query_index].location.group_idx == group);
      monitor->active_counters[i] =
         perf_cfg->counter_infos[current_query_index].location.counter_idx;
   }

   /* create the intel_perf_query */
   query_obj = intel_perf_new_query(perf_ctx, group);
   if (unlikely(!query_obj))
      goto allocation_failure;

   monitor->query = query_obj;
   monitor->result_size = perf_cfg->queries[group].data_size;
   monitor->result_buffer = calloc(1, monitor->result_size);
   if (unlikely(!monitor->result_buffer))
      goto allocation_failure;

   return monitor;

allocation_failure:
   if (monitor) {
      free(monitor->active_counters);
      free(monitor->result_buffer);
   }
   free(query_obj);
   free(monitor);
   return NULL;
}

void
iris_destroy_monitor_object(struct pipe_context *ctx,
                            struct iris_monitor_object *monitor)
{
   struct iris_context *ice = (struct iris_context *)ctx;

   intel_perf_delete_query(ice->perf_ctx, monitor->query);
   free(monitor->result_buffer);
   monitor->result_buffer = NULL;
   free(monitor->active_counters);
   monitor->active_counters = NULL;
   free(monitor);
}

bool
iris_begin_monitor(struct pipe_context *ctx,
                   struct iris_monitor_object *monitor)
{
   struct iris_context *ice = (void *) ctx;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;

   return intel_perf_begin_query(perf_ctx, monitor->query);
}

bool
iris_end_monitor(struct pipe_context *ctx,
                 struct iris_monitor_object *monitor)
{
   struct iris_context *ice = (void *) ctx;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;

   intel_perf_end_query(perf_ctx, monitor->query);
   return true;
}

bool
iris_get_monitor_result(struct pipe_context *ctx,
                        struct iris_monitor_object *monitor,
                        bool wait,
                        union pipe_numeric_type_union *result)
{
   struct iris_context *ice = (void *) ctx;
   struct intel_perf_context *perf_ctx = ice->perf_ctx;
   struct iris_batch *batch = &ice->batches[IRIS_BATCH_RENDER];

   bool monitor_ready =
      intel_perf_is_query_ready(perf_ctx, monitor->query, batch);

   if (!monitor_ready) {
      if (!wait)
         return false;
      intel_perf_wait_query(perf_ctx, monitor->query, batch);
   }

   assert(intel_perf_is_query_ready(perf_ctx, monitor->query, batch));

   unsigned bytes_written;
   intel_perf_get_query_data(perf_ctx, monitor->query, batch,
                           monitor->result_size,
                           (unsigned*) monitor->result_buffer,
                           &bytes_written);
   if (bytes_written != monitor->result_size)
      return false;

   /* copy metrics into the batch result */
   for (int i = 0; i < monitor->num_active_counters; ++i) {
      int current_counter = monitor->active_counters[i];
      const struct intel_perf_query_info *info =
         intel_perf_query_info(monitor->query);
      const struct intel_perf_query_counter *counter =
         &info->counters[current_counter];
      assert(intel_perf_query_counter_get_size(counter));
      switch (counter->data_type) {
      case INTEL_PERF_COUNTER_DATA_TYPE_UINT64:
         result[i].u64 = *(uint64_t*)(monitor->result_buffer + counter->offset);
         break;
      case INTEL_PERF_COUNTER_DATA_TYPE_FLOAT:
         result[i].f = *(float*)(monitor->result_buffer + counter->offset);
         break;
      case INTEL_PERF_COUNTER_DATA_TYPE_UINT32:
      case INTEL_PERF_COUNTER_DATA_TYPE_BOOL32:
         result[i].u64 = *(uint32_t*)(monitor->result_buffer + counter->offset);
         break;
      case INTEL_PERF_COUNTER_DATA_TYPE_DOUBLE: {
         double v = *(double*)(monitor->result_buffer + counter->offset);
         result[i].f = v;
         break;
      }
      default:
         unreachable("unexpected counter data type");
      }
   }
   return true;
}
