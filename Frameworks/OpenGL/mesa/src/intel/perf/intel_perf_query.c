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
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <unistd.h>
#include <poll.h>

#include "common/intel_gem.h"

#include "dev/intel_debug.h"
#include "dev/intel_device_info.h"

#include "perf/intel_perf.h"
#include "perf/intel_perf_mdapi.h"
#include "perf/intel_perf_private.h"
#include "perf/intel_perf_query.h"
#include "perf/intel_perf_regs.h"

#include "drm-uapi/i915_drm.h"

#include "util/compiler.h"
#include "util/u_math.h"

#define FILE_DEBUG_FLAG DEBUG_PERFMON

#define MI_RPC_BO_SIZE                (4096)
#define MI_FREQ_OFFSET_BYTES          (256)
#define MI_PERF_COUNTERS_OFFSET_BYTES (260)

#define MAP_READ  (1 << 0)
#define MAP_WRITE (1 << 1)

/**
 * Periodic OA samples are read() into these buffer structures via the
 * i915 perf kernel interface and appended to the
 * perf_ctx->sample_buffers linked list. When we process the
 * results of an OA metrics query we need to consider all the periodic
 * samples between the Begin and End MI_REPORT_PERF_COUNT command
 * markers.
 *
 * 'Periodic' is a simplification as there are other automatic reports
 * written by the hardware also buffered here.
 *
 * Considering three queries, A, B and C:
 *
 *  Time ---->
 *                ________________A_________________
 *                |                                |
 *                | ________B_________ _____C___________
 *                | |                | |           |   |
 *
 * And an illustration of sample buffers read over this time frame:
 * [HEAD ][     ][     ][     ][     ][     ][     ][     ][TAIL ]
 *
 * These nodes may hold samples for query A:
 * [     ][     ][  A  ][  A  ][  A  ][  A  ][  A  ][     ][     ]
 *
 * These nodes may hold samples for query B:
 * [     ][     ][  B  ][  B  ][  B  ][     ][     ][     ][     ]
 *
 * These nodes may hold samples for query C:
 * [     ][     ][     ][     ][     ][  C  ][  C  ][  C  ][     ]
 *
 * The illustration assumes we have an even distribution of periodic
 * samples so all nodes have the same size plotted against time:
 *
 * Note, to simplify code, the list is never empty.
 *
 * With overlapping queries we can see that periodic OA reports may
 * relate to multiple queries and care needs to be take to keep
 * track of sample buffers until there are no queries that might
 * depend on their contents.
 *
 * We use a node ref counting system where a reference ensures that a
 * node and all following nodes can't be freed/recycled until the
 * reference drops to zero.
 *
 * E.g. with a ref of one here:
 * [  0  ][  0  ][  1  ][  0  ][  0  ][  0  ][  0  ][  0  ][  0  ]
 *
 * These nodes could be freed or recycled ("reaped"):
 * [  0  ][  0  ]
 *
 * These must be preserved until the leading ref drops to zero:
 *               [  1  ][  0  ][  0  ][  0  ][  0  ][  0  ][  0  ]
 *
 * When a query starts we take a reference on the current tail of
 * the list, knowing that no already-buffered samples can possibly
 * relate to the newly-started query. A pointer to this node is
 * also saved in the query object's ->oa.samples_head.
 *
 * E.g. starting query A while there are two nodes in .sample_buffers:
 *                ________________A________
 *                |
 *
 * [  0  ][  1  ]
 *           ^_______ Add a reference and store pointer to node in
 *                    A->oa.samples_head
 *
 * Moving forward to when the B query starts with no new buffer nodes:
 * (for reference, i915 perf reads() are only done when queries finish)
 *                ________________A_______
 *                | ________B___
 *                | |
 *
 * [  0  ][  2  ]
 *           ^_______ Add a reference and store pointer to
 *                    node in B->oa.samples_head
 *
 * Once a query is finished, after an OA query has become 'Ready',
 * once the End OA report has landed and after we we have processed
 * all the intermediate periodic samples then we drop the
 * ->oa.samples_head reference we took at the start.
 *
 * So when the B query has finished we have:
 *                ________________A________
 *                | ______B___________
 *                | |                |
 * [  0  ][  1  ][  0  ][  0  ][  0  ]
 *           ^_______ Drop B->oa.samples_head reference
 *
 * We still can't free these due to the A->oa.samples_head ref:
 *        [  1  ][  0  ][  0  ][  0  ]
 *
 * When the A query finishes: (note there's a new ref for C's samples_head)
 *                ________________A_________________
 *                |                                |
 *                |                    _____C_________
 *                |                    |           |
 * [  0  ][  0  ][  0  ][  0  ][  1  ][  0  ][  0  ]
 *           ^_______ Drop A->oa.samples_head reference
 *
 * And we can now reap these nodes up to the C->oa.samples_head:
 * [  X  ][  X  ][  X  ][  X  ]
 *                  keeping -> [  1  ][  0  ][  0  ]
 *
 * We reap old sample buffers each time we finish processing an OA
 * query by iterating the sample_buffers list from the head until we
 * find a referenced node and stop.
 *
 * Reaped buffers move to a perfquery.free_sample_buffers list and
 * when we come to read() we first look to recycle a buffer from the
 * free_sample_buffers list before allocating a new buffer.
 */
struct oa_sample_buf {
   struct exec_node link;
   int refcount;
   int len;
   uint8_t buf[I915_PERF_OA_SAMPLE_SIZE * 10];
   uint32_t last_timestamp;
};

/**
 * gen representation of a performance query object.
 *
 * NB: We want to keep this structure relatively lean considering that
 * applications may expect to allocate enough objects to be able to
 * query around all draw calls in a frame.
 */
struct intel_perf_query_object
{
   const struct intel_perf_query_info *queryinfo;

   /* See query->kind to know which state below is in use... */
   union {
      struct {

         /**
          * BO containing OA counter snapshots at query Begin/End time.
          */
         void *bo;

         /**
          * Address of mapped of @bo
          */
         void *map;

         /**
          * The MI_REPORT_PERF_COUNT command lets us specify a unique
          * ID that will be reflected in the resulting OA report
          * that's written by the GPU. This is the ID we're expecting
          * in the begin report and the the end report should be
          * @begin_report_id + 1.
          */
         int begin_report_id;

         /**
          * Reference the head of the brw->perfquery.sample_buffers
          * list at the time that the query started (so we only need
          * to look at nodes after this point when looking for samples
          * related to this query)
          *
          * (See struct brw_oa_sample_buf description for more details)
          */
         struct exec_node *samples_head;

         /**
          * false while in the unaccumulated_elements list, and set to
          * true when the final, end MI_RPC snapshot has been
          * accumulated.
          */
         bool results_accumulated;

         /**
          * Accumulated OA results between begin and end of the query.
          */
         struct intel_perf_query_result result;
      } oa;

      struct {
         /**
          * BO containing starting and ending snapshots for the
          * statistics counters.
          */
         void *bo;
      } pipeline_stats;
   };
};

struct intel_perf_context {
   struct intel_perf_config *perf;

   void * mem_ctx; /* ralloc context */
   void * ctx;  /* driver context (eg, brw_context) */
   void * bufmgr;
   const struct intel_device_info *devinfo;

   uint32_t hw_ctx;
   int drm_fd;

   /* The i915 perf stream we open to setup + enable the OA counters */
   int oa_stream_fd;

   /* An i915 perf stream fd gives exclusive access to the OA unit that will
    * report counter snapshots for a specific counter set/profile in a
    * specific layout/format so we can only start OA queries that are
    * compatible with the currently open fd...
    */
   int current_oa_metrics_set_id;
   int current_oa_format;

   /* List of buffers containing OA reports */
   struct exec_list sample_buffers;

   /* Cached list of empty sample buffers */
   struct exec_list free_sample_buffers;

   int n_active_oa_queries;
   int n_active_pipeline_stats_queries;

   /* The number of queries depending on running OA counters which
    * extends beyond brw_end_perf_query() since we need to wait until
    * the last MI_RPC command has parsed by the GPU.
    *
    * Accurate accounting is important here as emitting an
    * MI_REPORT_PERF_COUNT command while the OA unit is disabled will
    * effectively hang the gpu.
    */
   int n_oa_users;

   /* To help catch an spurious problem with the hardware or perf
    * forwarding samples, we emit each MI_REPORT_PERF_COUNT command
    * with a unique ID that we can explicitly check for...
    */
   int next_query_start_report_id;

   /**
    * An array of queries whose results haven't yet been assembled
    * based on the data in buffer objects.
    *
    * These may be active, or have already ended.  However, the
    * results have not been requested.
    */
   struct intel_perf_query_object **unaccumulated;
   int unaccumulated_elements;
   int unaccumulated_array_size;

   /* The total number of query objects so we can relinquish
    * our exclusive access to perf if the application deletes
    * all of its objects. (NB: We only disable perf while
    * there are no active queries)
    */
   int n_query_instances;

   int period_exponent;
};

static bool
inc_n_users(struct intel_perf_context *perf_ctx)
{
   if (perf_ctx->n_oa_users == 0 &&
       intel_ioctl(perf_ctx->oa_stream_fd, I915_PERF_IOCTL_ENABLE, 0) < 0)
   {
      return false;
   }
   ++perf_ctx->n_oa_users;

   return true;
}

static void
dec_n_users(struct intel_perf_context *perf_ctx)
{
   /* Disabling the i915 perf stream will effectively disable the OA
    * counters.  Note it's important to be sure there are no outstanding
    * MI_RPC commands at this point since they could stall the CS
    * indefinitely once OACONTROL is disabled.
    */
   --perf_ctx->n_oa_users;
   if (perf_ctx->n_oa_users == 0 &&
       intel_ioctl(perf_ctx->oa_stream_fd, I915_PERF_IOCTL_DISABLE, 0) < 0)
   {
      DBG("WARNING: Error disabling gen perf stream: %m\n");
   }
}

void
intel_perf_close(struct intel_perf_context *perfquery,
                 const struct intel_perf_query_info *query)
{
   if (perfquery->oa_stream_fd != -1) {
      close(perfquery->oa_stream_fd);
      perfquery->oa_stream_fd = -1;
   }
   if (query && query->kind == INTEL_PERF_QUERY_TYPE_RAW) {
      struct intel_perf_query_info *raw_query =
         (struct intel_perf_query_info *) query;
      raw_query->oa_metrics_set_id = 0;
   }
}

bool
intel_perf_open(struct intel_perf_context *perf_ctx,
                int metrics_set_id,
                int report_format,
                int period_exponent,
                int drm_fd,
                uint32_t ctx_id,
                bool enable)
{
   uint64_t properties[DRM_I915_PERF_PROP_MAX * 2];
   uint32_t p = 0;

   /* Single context sampling if valid context id. */
   if (ctx_id != INTEL_PERF_INVALID_CTX_ID) {
      properties[p++] = DRM_I915_PERF_PROP_CTX_HANDLE;
      properties[p++] = ctx_id;
   }

   /* Include OA reports in samples */
   properties[p++] = DRM_I915_PERF_PROP_SAMPLE_OA;
   properties[p++] = true;

   /* OA unit configuration */
   properties[p++] = DRM_I915_PERF_PROP_OA_METRICS_SET;
   properties[p++] = metrics_set_id;

   properties[p++] = DRM_I915_PERF_PROP_OA_FORMAT;
   properties[p++] = report_format;

   properties[p++] = DRM_I915_PERF_PROP_OA_EXPONENT;
   properties[p++] = period_exponent;

   /* If global SSEU is available, pin it to the default. This will ensure on
    * Gfx11 for instance we use the full EU array. Initially when perf was
    * enabled we would use only half on Gfx11 because of functional
    * requirements.
    *
    * Temporary disable this option on Gfx12.5+, kernel doesn't appear to
    * support it.
    */
   if (intel_perf_has_global_sseu(perf_ctx->perf) &&
       perf_ctx->devinfo->verx10 < 125) {
      properties[p++] = DRM_I915_PERF_PROP_GLOBAL_SSEU;
      properties[p++] = to_user_pointer(&perf_ctx->perf->sseu);
   }

   assert(p <= ARRAY_SIZE(properties));

   struct drm_i915_perf_open_param param = {
      .flags = I915_PERF_FLAG_FD_CLOEXEC |
               I915_PERF_FLAG_FD_NONBLOCK |
               (enable ? 0 : I915_PERF_FLAG_DISABLED),
      .num_properties = p / 2,
      .properties_ptr = (uintptr_t) properties,
   };
   int fd = intel_ioctl(drm_fd, DRM_IOCTL_I915_PERF_OPEN, &param);
   if (fd == -1) {
      DBG("Error opening gen perf OA stream: %m\n");
      return false;
   }

   perf_ctx->oa_stream_fd = fd;

   perf_ctx->current_oa_metrics_set_id = metrics_set_id;
   perf_ctx->current_oa_format = report_format;

   if (enable)
      ++perf_ctx->n_oa_users;

   return true;
}

static uint64_t
get_metric_id(struct intel_perf_config *perf,
              const struct intel_perf_query_info *query)
{
   /* These queries are know not to ever change, their config ID has been
    * loaded upon the first query creation. No need to look them up again.
    */
   if (query->kind == INTEL_PERF_QUERY_TYPE_OA)
      return query->oa_metrics_set_id;

   assert(query->kind == INTEL_PERF_QUERY_TYPE_RAW);

   /* Raw queries can be reprogrammed up by an external application/library.
    * When a raw query is used for the first time it's id is set to a value !=
    * 0. When it stops being used the id returns to 0. No need to reload the
    * ID when it's already loaded.
    */
   if (query->oa_metrics_set_id != 0) {
      DBG("Raw query '%s' guid=%s using cached ID: %"PRIu64"\n",
          query->name, query->guid, query->oa_metrics_set_id);
      return query->oa_metrics_set_id;
   }

   struct intel_perf_query_info *raw_query = (struct intel_perf_query_info *)query;
   if (!intel_perf_load_metric_id(perf, query->guid,
                                &raw_query->oa_metrics_set_id)) {
      DBG("Unable to read query guid=%s ID, falling back to test config\n", query->guid);
      raw_query->oa_metrics_set_id = perf->fallback_raw_oa_metric;
   } else {
      DBG("Raw query '%s'guid=%s loaded ID: %"PRIu64"\n",
          query->name, query->guid, query->oa_metrics_set_id);
   }
   return query->oa_metrics_set_id;
}

static struct oa_sample_buf *
get_free_sample_buf(struct intel_perf_context *perf_ctx)
{
   struct exec_node *node = exec_list_pop_head(&perf_ctx->free_sample_buffers);
   struct oa_sample_buf *buf;

   if (node)
      buf = exec_node_data(struct oa_sample_buf, node, link);
   else {
      buf = ralloc_size(perf_ctx->perf, sizeof(*buf));

      exec_node_init(&buf->link);
      buf->refcount = 0;
   }
   buf->len = 0;

   return buf;
}

static void
reap_old_sample_buffers(struct intel_perf_context *perf_ctx)
{
   struct exec_node *tail_node =
      exec_list_get_tail(&perf_ctx->sample_buffers);
   struct oa_sample_buf *tail_buf =
      exec_node_data(struct oa_sample_buf, tail_node, link);

   /* Remove all old, unreferenced sample buffers walking forward from
    * the head of the list, except always leave at least one node in
    * the list so we always have a node to reference when we Begin
    * a new query.
    */
   foreach_list_typed_safe(struct oa_sample_buf, buf, link,
                           &perf_ctx->sample_buffers)
   {
      if (buf->refcount == 0 && buf != tail_buf) {
         exec_node_remove(&buf->link);
         exec_list_push_head(&perf_ctx->free_sample_buffers, &buf->link);
      } else
         return;
   }
}

static void
free_sample_bufs(struct intel_perf_context *perf_ctx)
{
   foreach_list_typed_safe(struct oa_sample_buf, buf, link,
                           &perf_ctx->free_sample_buffers)
      ralloc_free(buf);

   exec_list_make_empty(&perf_ctx->free_sample_buffers);
}


struct intel_perf_query_object *
intel_perf_new_query(struct intel_perf_context *perf_ctx, unsigned query_index)
{
   const struct intel_perf_query_info *query =
      &perf_ctx->perf->queries[query_index];

   switch (query->kind) {
   case INTEL_PERF_QUERY_TYPE_OA:
   case INTEL_PERF_QUERY_TYPE_RAW:
      if (perf_ctx->period_exponent == 0)
         return NULL;
      break;
   case INTEL_PERF_QUERY_TYPE_PIPELINE:
      break;
   }

   struct intel_perf_query_object *obj =
      calloc(1, sizeof(struct intel_perf_query_object));

   if (!obj)
      return NULL;

   obj->queryinfo = query;

   perf_ctx->n_query_instances++;
   return obj;
}

int
intel_perf_active_queries(struct intel_perf_context *perf_ctx,
                          const struct intel_perf_query_info *query)
{
   assert(perf_ctx->n_active_oa_queries == 0 || perf_ctx->n_active_pipeline_stats_queries == 0);

   switch (query->kind) {
   case INTEL_PERF_QUERY_TYPE_OA:
   case INTEL_PERF_QUERY_TYPE_RAW:
      return perf_ctx->n_active_oa_queries;
      break;

   case INTEL_PERF_QUERY_TYPE_PIPELINE:
      return perf_ctx->n_active_pipeline_stats_queries;
      break;

   default:
      unreachable("Unknown query type");
      break;
   }
}

const struct intel_perf_query_info*
intel_perf_query_info(const struct intel_perf_query_object *query)
{
   return query->queryinfo;
}

struct intel_perf_context *
intel_perf_new_context(void *parent)
{
   struct intel_perf_context *ctx = rzalloc(parent, struct intel_perf_context);
   if (! ctx)
      fprintf(stderr, "%s: failed to alloc context\n", __func__);
   return ctx;
}

struct intel_perf_config *
intel_perf_config(struct intel_perf_context *ctx)
{
   return ctx->perf;
}

void
intel_perf_init_context(struct intel_perf_context *perf_ctx,
                        struct intel_perf_config *perf_cfg,
                        void * mem_ctx, /* ralloc context */
                        void * ctx,  /* driver context (eg, brw_context) */
                        void * bufmgr,  /* eg brw_bufmgr */
                        const struct intel_device_info *devinfo,
                        uint32_t hw_ctx,
                        int drm_fd)
{
   perf_ctx->perf = perf_cfg;
   perf_ctx->mem_ctx = mem_ctx;
   perf_ctx->ctx = ctx;
   perf_ctx->bufmgr = bufmgr;
   perf_ctx->drm_fd = drm_fd;
   perf_ctx->hw_ctx = hw_ctx;
   perf_ctx->devinfo = devinfo;

   perf_ctx->unaccumulated =
      ralloc_array(mem_ctx, struct intel_perf_query_object *, 2);
   perf_ctx->unaccumulated_elements = 0;
   perf_ctx->unaccumulated_array_size = 2;

   exec_list_make_empty(&perf_ctx->sample_buffers);
   exec_list_make_empty(&perf_ctx->free_sample_buffers);

   /* It's convenient to guarantee that this linked list of sample
    * buffers is never empty so we add an empty head so when we
    * Begin an OA query we can always take a reference on a buffer
    * in this list.
    */
   struct oa_sample_buf *buf = get_free_sample_buf(perf_ctx);
   exec_list_push_head(&perf_ctx->sample_buffers, &buf->link);

   perf_ctx->oa_stream_fd = -1;
   perf_ctx->next_query_start_report_id = 1000;

   /* The period_exponent gives a sampling period as follows:
    *   sample_period = timestamp_period * 2^(period_exponent + 1)
    *
    * The timestamps increments every 80ns (HSW), ~52ns (GFX9LP) or
    * ~83ns (GFX8/9).
    *
    * The counter overflow period is derived from the EuActive counter
    * which reads a counter that increments by the number of clock
    * cycles multiplied by the number of EUs. It can be calculated as:
    *
    * 2^(number of bits in A counter) / (n_eus * max_intel_freq * 2)
    *
    * (E.g. 40 EUs @ 1GHz = ~53ms)
    *
    * We select a sampling period inferior to that overflow period to
    * ensure we cannot see more than 1 counter overflow, otherwise we
    * could loose information.
    */

   int a_counter_in_bits = 32;
   if (devinfo->ver >= 8)
      a_counter_in_bits = 40;

   uint64_t overflow_period = pow(2, a_counter_in_bits) / (perf_cfg->sys_vars.n_eus *
       /* drop 1GHz freq to have units in nanoseconds */
       2);

   DBG("A counter overflow period: %"PRIu64"ns, %"PRIu64"ms (n_eus=%"PRIu64")\n",
       overflow_period, overflow_period / 1000000ul, perf_cfg->sys_vars.n_eus);

   int period_exponent = 0;
   uint64_t prev_sample_period, next_sample_period;
   for (int e = 0; e < 30; e++) {
      prev_sample_period = 1000000000ull * pow(2, e + 1) / devinfo->timestamp_frequency;
      next_sample_period = 1000000000ull * pow(2, e + 2) / devinfo->timestamp_frequency;

      /* Take the previous sampling period, lower than the overflow
       * period.
       */
      if (prev_sample_period < overflow_period &&
          next_sample_period > overflow_period)
         period_exponent = e + 1;
   }

   perf_ctx->period_exponent = period_exponent;

   if (period_exponent == 0) {
      DBG("WARNING: enable to find a sampling exponent\n");
   } else {
      DBG("OA sampling exponent: %i ~= %"PRIu64"ms\n", period_exponent,
            prev_sample_period / 1000000ul);
   }
}

/**
 * Add a query to the global list of "unaccumulated queries."
 *
 * Queries are tracked here until all the associated OA reports have
 * been accumulated via accumulate_oa_reports() after the end
 * MI_REPORT_PERF_COUNT has landed in query->oa.bo.
 */
static void
add_to_unaccumulated_query_list(struct intel_perf_context *perf_ctx,
                                struct intel_perf_query_object *obj)
{
   if (perf_ctx->unaccumulated_elements >=
       perf_ctx->unaccumulated_array_size)
   {
      perf_ctx->unaccumulated_array_size *= 1.5;
      perf_ctx->unaccumulated =
         reralloc(perf_ctx->mem_ctx, perf_ctx->unaccumulated,
                  struct intel_perf_query_object *,
                  perf_ctx->unaccumulated_array_size);
   }

   perf_ctx->unaccumulated[perf_ctx->unaccumulated_elements++] = obj;
}

/**
 * Emit MI_STORE_REGISTER_MEM commands to capture all of the
 * pipeline statistics for the performance query object.
 */
static void
snapshot_statistics_registers(struct intel_perf_context *ctx,
                              struct intel_perf_query_object *obj,
                              uint32_t offset_in_bytes)
{
   struct intel_perf_config *perf = ctx->perf;
   const struct intel_perf_query_info *query = obj->queryinfo;
   const int n_counters = query->n_counters;

   for (int i = 0; i < n_counters; i++) {
      const struct intel_perf_query_counter *counter = &query->counters[i];

      assert(counter->data_type == INTEL_PERF_COUNTER_DATA_TYPE_UINT64);

      perf->vtbl.store_register_mem(ctx->ctx, obj->pipeline_stats.bo,
                                    counter->pipeline_stat.reg, 8,
                                    offset_in_bytes + counter->offset);
   }
}

static void
snapshot_query_layout(struct intel_perf_context *perf_ctx,
                      struct intel_perf_query_object *query,
                      bool end_snapshot)
{
   struct intel_perf_config *perf_cfg = perf_ctx->perf;
   const struct intel_perf_query_field_layout *layout = &perf_cfg->query_layout;
   uint32_t offset = end_snapshot ? align(layout->size, layout->alignment) : 0;

   for (uint32_t f = 0; f < layout->n_fields; f++) {
      const struct intel_perf_query_field *field =
         &layout->fields[end_snapshot ? f : (layout->n_fields - 1 - f)];

      switch (field->type) {
      case INTEL_PERF_QUERY_FIELD_TYPE_MI_RPC:
         perf_cfg->vtbl.emit_mi_report_perf_count(perf_ctx->ctx, query->oa.bo,
                                                  offset + field->location,
                                                  query->oa.begin_report_id +
                                                  (end_snapshot ? 1 : 0));
         break;
      case INTEL_PERF_QUERY_FIELD_TYPE_SRM_PERFCNT:
      case INTEL_PERF_QUERY_FIELD_TYPE_SRM_RPSTAT:
      case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_A:
      case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_B:
      case INTEL_PERF_QUERY_FIELD_TYPE_SRM_OA_C:
         perf_cfg->vtbl.store_register_mem(perf_ctx->ctx, query->oa.bo,
                                           field->mmio_offset, field->size,
                                           offset + field->location);
         break;
      default:
         unreachable("Invalid field type");
      }
   }
}

bool
intel_perf_begin_query(struct intel_perf_context *perf_ctx,
                       struct intel_perf_query_object *query)
{
   struct intel_perf_config *perf_cfg = perf_ctx->perf;
   const struct intel_perf_query_info *queryinfo = query->queryinfo;

   /* XXX: We have to consider that the command parser unit that parses batch
    * buffer commands and is used to capture begin/end counter snapshots isn't
    * implicitly synchronized with what's currently running across other GPU
    * units (such as the EUs running shaders) that the performance counters are
    * associated with.
    *
    * The intention of performance queries is to measure the work associated
    * with commands between the begin/end delimiters and so for that to be the
    * case we need to explicitly synchronize the parsing of commands to capture
    * Begin/End counter snapshots with what's running across other parts of the
    * GPU.
    *
    * When the command parser reaches a Begin marker it effectively needs to
    * drain everything currently running on the GPU until the hardware is idle
    * before capturing the first snapshot of counters - otherwise the results
    * would also be measuring the effects of earlier commands.
    *
    * When the command parser reaches an End marker it needs to stall until
    * everything currently running on the GPU has finished before capturing the
    * end snapshot - otherwise the results won't be a complete representation
    * of the work.
    *
    * To achieve this, we stall the pipeline at pixel scoreboard (prevent any
    * additional work to be processed by the pipeline until all pixels of the
    * previous draw has be completed).
    *
    * N.B. The final results are based on deltas of counters between (inside)
    * Begin/End markers so even though the total wall clock time of the
    * workload is stretched by larger pipeline bubbles the bubbles themselves
    * are generally invisible to the query results. Whether that's a good or a
    * bad thing depends on the use case. For a lower real-time impact while
    * capturing metrics then periodic sampling may be a better choice than
    * INTEL_performance_query.
    *
    *
    * This is our Begin synchronization point to drain current work on the
    * GPU before we capture our first counter snapshot...
    */
   perf_cfg->vtbl.emit_stall_at_pixel_scoreboard(perf_ctx->ctx);

   switch (queryinfo->kind) {
   case INTEL_PERF_QUERY_TYPE_OA:
   case INTEL_PERF_QUERY_TYPE_RAW: {

      /* Opening an i915 perf stream implies exclusive access to the OA unit
       * which will generate counter reports for a specific counter set with a
       * specific layout/format so we can't begin any OA based queries that
       * require a different counter set or format unless we get an opportunity
       * to close the stream and open a new one...
       */
      uint64_t metric_id = get_metric_id(perf_ctx->perf, queryinfo);

      if (perf_ctx->oa_stream_fd != -1 &&
          perf_ctx->current_oa_metrics_set_id != metric_id) {

         if (perf_ctx->n_oa_users != 0) {
            DBG("WARNING: Begin failed already using perf config=%i/%"PRIu64"\n",
                perf_ctx->current_oa_metrics_set_id, metric_id);
            return false;
         } else
            intel_perf_close(perf_ctx, queryinfo);
      }

      /* If the OA counters aren't already on, enable them. */
      if (perf_ctx->oa_stream_fd == -1) {
         assert(perf_ctx->period_exponent != 0);

         if (!intel_perf_open(perf_ctx, metric_id, queryinfo->oa_format,
                            perf_ctx->period_exponent, perf_ctx->drm_fd,
                            perf_ctx->hw_ctx, false))
            return false;
      } else {
         assert(perf_ctx->current_oa_metrics_set_id == metric_id &&
                perf_ctx->current_oa_format == queryinfo->oa_format);
      }

      if (!inc_n_users(perf_ctx)) {
         DBG("WARNING: Error enabling i915 perf stream: %m\n");
         return false;
      }

      if (query->oa.bo) {
         perf_cfg->vtbl.bo_unreference(query->oa.bo);
         query->oa.bo = NULL;
      }

      query->oa.bo = perf_cfg->vtbl.bo_alloc(perf_ctx->bufmgr,
                                             "perf. query OA MI_RPC bo",
                                             MI_RPC_BO_SIZE);
#ifdef DEBUG
      /* Pre-filling the BO helps debug whether writes landed. */
      void *map = perf_cfg->vtbl.bo_map(perf_ctx->ctx, query->oa.bo, MAP_WRITE);
      memset(map, 0x80, MI_RPC_BO_SIZE);
      perf_cfg->vtbl.bo_unmap(query->oa.bo);
#endif

      query->oa.begin_report_id = perf_ctx->next_query_start_report_id;
      perf_ctx->next_query_start_report_id += 2;

      snapshot_query_layout(perf_ctx, query, false /* end_snapshot */);

      ++perf_ctx->n_active_oa_queries;

      /* No already-buffered samples can possibly be associated with this query
       * so create a marker within the list of sample buffers enabling us to
       * easily ignore earlier samples when processing this query after
       * completion.
       */
      assert(!exec_list_is_empty(&perf_ctx->sample_buffers));
      query->oa.samples_head = exec_list_get_tail(&perf_ctx->sample_buffers);

      struct oa_sample_buf *buf =
         exec_node_data(struct oa_sample_buf, query->oa.samples_head, link);

      /* This reference will ensure that future/following sample
       * buffers (that may relate to this query) can't be freed until
       * this drops to zero.
       */
      buf->refcount++;

      intel_perf_query_result_clear(&query->oa.result);
      query->oa.results_accumulated = false;

      add_to_unaccumulated_query_list(perf_ctx, query);
      break;
   }

   case INTEL_PERF_QUERY_TYPE_PIPELINE:
      if (query->pipeline_stats.bo) {
         perf_cfg->vtbl.bo_unreference(query->pipeline_stats.bo);
         query->pipeline_stats.bo = NULL;
      }

      query->pipeline_stats.bo =
         perf_cfg->vtbl.bo_alloc(perf_ctx->bufmgr,
                                 "perf. query pipeline stats bo",
                                 STATS_BO_SIZE);

      /* Take starting snapshots. */
      snapshot_statistics_registers(perf_ctx, query, 0);

      ++perf_ctx->n_active_pipeline_stats_queries;
      break;

   default:
      unreachable("Unknown query type");
      break;
   }

   return true;
}

void
intel_perf_end_query(struct intel_perf_context *perf_ctx,
                     struct intel_perf_query_object *query)
{
   struct intel_perf_config *perf_cfg = perf_ctx->perf;

   /* Ensure that the work associated with the queried commands will have
    * finished before taking our query end counter readings.
    *
    * For more details see comment in brw_begin_perf_query for
    * corresponding flush.
    */
   perf_cfg->vtbl.emit_stall_at_pixel_scoreboard(perf_ctx->ctx);

   switch (query->queryinfo->kind) {
   case INTEL_PERF_QUERY_TYPE_OA:
   case INTEL_PERF_QUERY_TYPE_RAW:

      /* NB: It's possible that the query will have already been marked
       * as 'accumulated' if an error was seen while reading samples
       * from perf. In this case we mustn't try and emit a closing
       * MI_RPC command in case the OA unit has already been disabled
       */
      if (!query->oa.results_accumulated)
         snapshot_query_layout(perf_ctx, query, true /* end_snapshot */);

      --perf_ctx->n_active_oa_queries;

      /* NB: even though the query has now ended, it can't be accumulated
       * until the end MI_REPORT_PERF_COUNT snapshot has been written
       * to query->oa.bo
       */
      break;

   case INTEL_PERF_QUERY_TYPE_PIPELINE:
      snapshot_statistics_registers(perf_ctx, query,
                                    STATS_BO_END_OFFSET_BYTES);
      --perf_ctx->n_active_pipeline_stats_queries;
      break;

   default:
      unreachable("Unknown query type");
      break;
   }
}

bool intel_perf_oa_stream_ready(struct intel_perf_context *perf_ctx)
{
   struct pollfd pfd;

   pfd.fd = perf_ctx->oa_stream_fd;
   pfd.events = POLLIN;
   pfd.revents = 0;

   if (poll(&pfd, 1, 0) < 0) {
      DBG("Error polling OA stream\n");
      return false;
   }

   if (!(pfd.revents & POLLIN))
      return false;

   return true;
}

ssize_t
intel_perf_read_oa_stream(struct intel_perf_context *perf_ctx,
                          void* buf,
                          size_t nbytes)
{
   return read(perf_ctx->oa_stream_fd, buf, nbytes);
}

enum OaReadStatus {
   OA_READ_STATUS_ERROR,
   OA_READ_STATUS_UNFINISHED,
   OA_READ_STATUS_FINISHED,
};

static enum OaReadStatus
read_oa_samples_until(struct intel_perf_context *perf_ctx,
                      uint32_t start_timestamp,
                      uint32_t end_timestamp)
{
   struct exec_node *tail_node =
      exec_list_get_tail(&perf_ctx->sample_buffers);
   struct oa_sample_buf *tail_buf =
      exec_node_data(struct oa_sample_buf, tail_node, link);
   uint32_t last_timestamp =
      tail_buf->len == 0 ? start_timestamp : tail_buf->last_timestamp;

   while (1) {
      struct oa_sample_buf *buf = get_free_sample_buf(perf_ctx);
      uint32_t offset;
      int len;

      while ((len = read(perf_ctx->oa_stream_fd, buf->buf,
                         sizeof(buf->buf))) < 0 && errno == EINTR)
         ;

      if (len <= 0) {
         exec_list_push_tail(&perf_ctx->free_sample_buffers, &buf->link);

         if (len == 0) {
            DBG("Spurious EOF reading i915 perf samples\n");
            return OA_READ_STATUS_ERROR;
         }

         if (errno != EAGAIN) {
            DBG("Error reading i915 perf samples: %m\n");
            return OA_READ_STATUS_ERROR;
         }

         if ((last_timestamp - start_timestamp) >= INT32_MAX)
            return OA_READ_STATUS_UNFINISHED;

         if ((last_timestamp - start_timestamp) <
              (end_timestamp - start_timestamp))
            return OA_READ_STATUS_UNFINISHED;

         return OA_READ_STATUS_FINISHED;
      }

      buf->len = len;
      exec_list_push_tail(&perf_ctx->sample_buffers, &buf->link);

      /* Go through the reports and update the last timestamp. */
      offset = 0;
      while (offset < buf->len) {
         const struct drm_i915_perf_record_header *header =
            (const struct drm_i915_perf_record_header *) &buf->buf[offset];
         uint32_t *report = (uint32_t *) (header + 1);

         if (header->type == DRM_I915_PERF_RECORD_SAMPLE)
            last_timestamp = report[1];

         offset += header->size;
      }

      buf->last_timestamp = last_timestamp;
   }

   unreachable("not reached");
   return OA_READ_STATUS_ERROR;
}

/**
 * Try to read all the reports until either the delimiting timestamp
 * or an error arises.
 */
static bool
read_oa_samples_for_query(struct intel_perf_context *perf_ctx,
                          struct intel_perf_query_object *query,
                          void *current_batch)
{
   uint32_t *start;
   uint32_t *last;
   uint32_t *end;
   struct intel_perf_config *perf_cfg = perf_ctx->perf;

   /* We need the MI_REPORT_PERF_COUNT to land before we can start
    * accumulate. */
   assert(!perf_cfg->vtbl.batch_references(current_batch, query->oa.bo) &&
          !perf_cfg->vtbl.bo_busy(query->oa.bo));

   /* Map the BO once here and let accumulate_oa_reports() unmap
    * it. */
   if (query->oa.map == NULL)
      query->oa.map = perf_cfg->vtbl.bo_map(perf_ctx->ctx, query->oa.bo, MAP_READ);

   start = last = query->oa.map;
   end = query->oa.map + perf_ctx->perf->query_layout.size;

   if (start[0] != query->oa.begin_report_id) {
      DBG("Spurious start report id=%"PRIu32"\n", start[0]);
      return true;
   }
   if (end[0] != (query->oa.begin_report_id + 1)) {
      DBG("Spurious end report id=%"PRIu32"\n", end[0]);
      return true;
   }

   /* Read the reports until the end timestamp. */
   switch (read_oa_samples_until(perf_ctx, start[1], end[1])) {
   case OA_READ_STATUS_ERROR:
      FALLTHROUGH; /* Let accumulate_oa_reports() deal with the error. */
   case OA_READ_STATUS_FINISHED:
      return true;
   case OA_READ_STATUS_UNFINISHED:
      return false;
   }

   unreachable("invalid read status");
   return false;
}

void
intel_perf_wait_query(struct intel_perf_context *perf_ctx,
                      struct intel_perf_query_object *query,
                      void *current_batch)
{
   struct intel_perf_config *perf_cfg = perf_ctx->perf;
   struct brw_bo *bo = NULL;

   switch (query->queryinfo->kind) {
   case INTEL_PERF_QUERY_TYPE_OA:
   case INTEL_PERF_QUERY_TYPE_RAW:
      bo = query->oa.bo;
      break;

   case INTEL_PERF_QUERY_TYPE_PIPELINE:
      bo = query->pipeline_stats.bo;
      break;

   default:
      unreachable("Unknown query type");
      break;
   }

   if (bo == NULL)
      return;

   /* If the current batch references our results bo then we need to
    * flush first...
    */
   if (perf_cfg->vtbl.batch_references(current_batch, bo))
      perf_cfg->vtbl.batchbuffer_flush(perf_ctx->ctx, __FILE__, __LINE__);

   perf_cfg->vtbl.bo_wait_rendering(bo);
}

bool
intel_perf_is_query_ready(struct intel_perf_context *perf_ctx,
                          struct intel_perf_query_object *query,
                          void *current_batch)
{
   struct intel_perf_config *perf_cfg = perf_ctx->perf;

   switch (query->queryinfo->kind) {
   case INTEL_PERF_QUERY_TYPE_OA:
   case INTEL_PERF_QUERY_TYPE_RAW:
      return (query->oa.results_accumulated ||
              (query->oa.bo &&
               !perf_cfg->vtbl.batch_references(current_batch, query->oa.bo) &&
               !perf_cfg->vtbl.bo_busy(query->oa.bo)));

   case INTEL_PERF_QUERY_TYPE_PIPELINE:
      return (query->pipeline_stats.bo &&
              !perf_cfg->vtbl.batch_references(current_batch, query->pipeline_stats.bo) &&
              !perf_cfg->vtbl.bo_busy(query->pipeline_stats.bo));

   default:
      unreachable("Unknown query type");
      break;
   }

   return false;
}

/**
 * Remove a query from the global list of unaccumulated queries once
 * after successfully accumulating the OA reports associated with the
 * query in accumulate_oa_reports() or when discarding unwanted query
 * results.
 */
static void
drop_from_unaccumulated_query_list(struct intel_perf_context *perf_ctx,
                                   struct intel_perf_query_object *query)
{
   for (int i = 0; i < perf_ctx->unaccumulated_elements; i++) {
      if (perf_ctx->unaccumulated[i] == query) {
         int last_elt = --perf_ctx->unaccumulated_elements;

         if (i == last_elt)
            perf_ctx->unaccumulated[i] = NULL;
         else {
            perf_ctx->unaccumulated[i] =
               perf_ctx->unaccumulated[last_elt];
         }

         break;
      }
   }

   /* Drop our samples_head reference so that associated periodic
    * sample data buffers can potentially be reaped if they aren't
    * referenced by any other queries...
    */

   struct oa_sample_buf *buf =
      exec_node_data(struct oa_sample_buf, query->oa.samples_head, link);

   assert(buf->refcount > 0);
   buf->refcount--;

   query->oa.samples_head = NULL;

   reap_old_sample_buffers(perf_ctx);
}

/* In general if we see anything spurious while accumulating results,
 * we don't try and continue accumulating the current query, hoping
 * for the best, we scrap anything outstanding, and then hope for the
 * best with new queries.
 */
static void
discard_all_queries(struct intel_perf_context *perf_ctx)
{
   while (perf_ctx->unaccumulated_elements) {
      struct intel_perf_query_object *query = perf_ctx->unaccumulated[0];

      query->oa.results_accumulated = true;
      drop_from_unaccumulated_query_list(perf_ctx, query);

      dec_n_users(perf_ctx);
   }
}

/* Looks for the validity bit of context ID (dword 2) of an OA report. */
static bool
oa_report_ctx_id_valid(const struct intel_device_info *devinfo,
                       const uint32_t *report)
{
   assert(devinfo->ver >= 8);
   if (devinfo->ver == 8)
      return (report[0] & (1 << 25)) != 0;
   return (report[0] & (1 << 16)) != 0;
}

/**
 * Accumulate raw OA counter values based on deltas between pairs of
 * OA reports.
 *
 * Accumulation starts from the first report captured via
 * MI_REPORT_PERF_COUNT (MI_RPC) by brw_begin_perf_query() until the
 * last MI_RPC report requested by brw_end_perf_query(). Between these
 * two reports there may also some number of periodically sampled OA
 * reports collected via the i915 perf interface - depending on the
 * duration of the query.
 *
 * These periodic snapshots help to ensure we handle counter overflow
 * correctly by being frequent enough to ensure we don't miss multiple
 * overflows of a counter between snapshots. For Gfx8+ the i915 perf
 * snapshots provide the extra context-switch reports that let us
 * subtract out the progress of counters associated with other
 * contexts running on the system.
 */
static void
accumulate_oa_reports(struct intel_perf_context *perf_ctx,
                      struct intel_perf_query_object *query)
{
   const struct intel_device_info *devinfo = perf_ctx->devinfo;
   uint32_t *start;
   uint32_t *last;
   uint32_t *end;
   struct exec_node *first_samples_node;
   bool last_report_ctx_match = true;
   int out_duration = 0;

   assert(query->oa.map != NULL);

   start = last = query->oa.map;
   end = query->oa.map + perf_ctx->perf->query_layout.size;

   if (start[0] != query->oa.begin_report_id) {
      DBG("Spurious start report id=%"PRIu32"\n", start[0]);
      goto error;
   }
   if (end[0] != (query->oa.begin_report_id + 1)) {
      DBG("Spurious end report id=%"PRIu32"\n", end[0]);
      goto error;
   }

   /* On Gfx12+ OA reports are sourced from per context counters, so we don't
    * ever have to look at the global OA buffer. Yey \o/
    */
   if (perf_ctx->devinfo->ver >= 12) {
      last = start;
      goto end;
   }

   /* See if we have any periodic reports to accumulate too... */

   /* N.B. The oa.samples_head was set when the query began and
    * pointed to the tail of the perf_ctx->sample_buffers list at
    * the time the query started. Since the buffer existed before the
    * first MI_REPORT_PERF_COUNT command was emitted we therefore know
    * that no data in this particular node's buffer can possibly be
    * associated with the query - so skip ahead one...
    */
   first_samples_node = query->oa.samples_head->next;

   foreach_list_typed_from(struct oa_sample_buf, buf, link,
                           &perf_ctx->sample_buffers,
                           first_samples_node)
   {
      int offset = 0;

      while (offset < buf->len) {
         const struct drm_i915_perf_record_header *header =
            (const struct drm_i915_perf_record_header *)(buf->buf + offset);

         assert(header->size != 0);
         assert(header->size <= buf->len);

         offset += header->size;

         switch (header->type) {
         case DRM_I915_PERF_RECORD_SAMPLE: {
            uint32_t *report = (uint32_t *)(header + 1);
            bool report_ctx_match = true;
            bool add = true;

            /* Ignore reports that come before the start marker.
             * (Note: takes care to allow overflow of 32bit timestamps)
             */
            if (intel_device_info_timebase_scale(devinfo,
                                               report[1] - start[1]) > 5000000000) {
               continue;
            }

            /* Ignore reports that come after the end marker.
             * (Note: takes care to allow overflow of 32bit timestamps)
             */
            if (intel_device_info_timebase_scale(devinfo,
                                               report[1] - end[1]) <= 5000000000) {
               goto end;
            }

            /* For Gfx8+ since the counters continue while other
             * contexts are running we need to discount any unrelated
             * deltas. The hardware automatically generates a report
             * on context switch which gives us a new reference point
             * to continuing adding deltas from.
             *
             * For Haswell we can rely on the HW to stop the progress
             * of OA counters while any other context is acctive.
             */
            if (devinfo->ver >= 8) {
               /* Consider that the current report matches our context only if
                * the report says the report ID is valid.
                */
               report_ctx_match = oa_report_ctx_id_valid(devinfo, report) &&
                  report[2] == start[2];
               if (report_ctx_match)
                  out_duration = 0;
               else
                  out_duration++;

               /* Only add the delta between <last, report> if the last report
                * was clearly identified as our context, or if we have at most
                * 1 report without a matching ID.
                *
                * The OA unit will sometimes label reports with an invalid
                * context ID when i915 rewrites the execlist submit register
                * with the same context as the one currently running. This
                * happens when i915 wants to notify the HW of ringbuffer tail
                * register update. We have to consider this report as part of
                * our context as the 3d pipeline behind the OACS unit is still
                * processing the operations started at the previous execlist
                * submission.
                */
               add = last_report_ctx_match && out_duration < 2;
            }

            if (add) {
               intel_perf_query_result_accumulate(&query->oa.result,
                                                query->queryinfo,
                                                last, report);
            } else {
               /* We're not adding the delta because we've identified it's not
                * for the context we filter for. We can consider that the
                * query was split.
                */
               query->oa.result.query_disjoint = true;
            }

            last = report;
            last_report_ctx_match = report_ctx_match;

            break;
         }

         case DRM_I915_PERF_RECORD_OA_BUFFER_LOST:
             DBG("i915 perf: OA error: all reports lost\n");
             goto error;
         case DRM_I915_PERF_RECORD_OA_REPORT_LOST:
             DBG("i915 perf: OA report lost\n");
             break;
         }
      }
   }

end:

   intel_perf_query_result_accumulate(&query->oa.result, query->queryinfo,
                                    last, end);

   query->oa.results_accumulated = true;
   drop_from_unaccumulated_query_list(perf_ctx, query);
   dec_n_users(perf_ctx);

   return;

error:

   discard_all_queries(perf_ctx);
}

void
intel_perf_delete_query(struct intel_perf_context *perf_ctx,
                        struct intel_perf_query_object *query)
{
   struct intel_perf_config *perf_cfg = perf_ctx->perf;

   /* We can assume that the frontend waits for a query to complete
    * before ever calling into here, so we don't have to worry about
    * deleting an in-flight query object.
    */
   switch (query->queryinfo->kind) {
   case INTEL_PERF_QUERY_TYPE_OA:
   case INTEL_PERF_QUERY_TYPE_RAW:
      if (query->oa.bo) {
         if (!query->oa.results_accumulated) {
            drop_from_unaccumulated_query_list(perf_ctx, query);
            dec_n_users(perf_ctx);
         }

         perf_cfg->vtbl.bo_unreference(query->oa.bo);
         query->oa.bo = NULL;
      }

      query->oa.results_accumulated = false;
      break;

   case INTEL_PERF_QUERY_TYPE_PIPELINE:
      if (query->pipeline_stats.bo) {
         perf_cfg->vtbl.bo_unreference(query->pipeline_stats.bo);
         query->pipeline_stats.bo = NULL;
      }
      break;

   default:
      unreachable("Unknown query type");
      break;
   }

   /* As an indication that the INTEL_performance_query extension is no
    * longer in use, it's a good time to free our cache of sample
    * buffers and close any current i915-perf stream.
    */
   if (--perf_ctx->n_query_instances == 0) {
      free_sample_bufs(perf_ctx);
      intel_perf_close(perf_ctx, query->queryinfo);
   }

   free(query);
}

static int
get_oa_counter_data(struct intel_perf_context *perf_ctx,
                    struct intel_perf_query_object *query,
                    size_t data_size,
                    uint8_t *data)
{
   struct intel_perf_config *perf_cfg = perf_ctx->perf;
   const struct intel_perf_query_info *queryinfo = query->queryinfo;
   int n_counters = queryinfo->n_counters;
   int written = 0;

   for (int i = 0; i < n_counters; i++) {
      const struct intel_perf_query_counter *counter = &queryinfo->counters[i];
      uint64_t *out_uint64;
      float *out_float;
      size_t counter_size = intel_perf_query_counter_get_size(counter);

      if (counter_size) {
         switch (counter->data_type) {
         case INTEL_PERF_COUNTER_DATA_TYPE_UINT64:
            out_uint64 = (uint64_t *)(data + counter->offset);
            *out_uint64 =
               counter->oa_counter_read_uint64(perf_cfg, queryinfo,
                                               &query->oa.result);
            break;
         case INTEL_PERF_COUNTER_DATA_TYPE_FLOAT:
            out_float = (float *)(data + counter->offset);
            *out_float =
               counter->oa_counter_read_float(perf_cfg, queryinfo,
                                              &query->oa.result);
            break;
         default:
            /* So far we aren't using uint32, double or bool32... */
            unreachable("unexpected counter data type");
         }

         if (counter->offset + counter_size > written)
            written = counter->offset + counter_size;
      }
   }

   return written;
}

static int
get_pipeline_stats_data(struct intel_perf_context *perf_ctx,
                        struct intel_perf_query_object *query,
                        size_t data_size,
                        uint8_t *data)

{
   struct intel_perf_config *perf_cfg = perf_ctx->perf;
   const struct intel_perf_query_info *queryinfo = query->queryinfo;
   int n_counters = queryinfo->n_counters;
   uint8_t *p = data;

   uint64_t *start = perf_cfg->vtbl.bo_map(perf_ctx->ctx, query->pipeline_stats.bo, MAP_READ);
   uint64_t *end = start + (STATS_BO_END_OFFSET_BYTES / sizeof(uint64_t));

   for (int i = 0; i < n_counters; i++) {
      const struct intel_perf_query_counter *counter = &queryinfo->counters[i];
      uint64_t value = end[i] - start[i];

      if (counter->pipeline_stat.numerator !=
          counter->pipeline_stat.denominator) {
         value *= counter->pipeline_stat.numerator;
         value /= counter->pipeline_stat.denominator;
      }

      *((uint64_t *)p) = value;
      p += 8;
   }

   perf_cfg->vtbl.bo_unmap(query->pipeline_stats.bo);

   return p - data;
}

void
intel_perf_get_query_data(struct intel_perf_context *perf_ctx,
                          struct intel_perf_query_object *query,
                          void *current_batch,
                          int data_size,
                          unsigned *data,
                          unsigned *bytes_written)
{
   struct intel_perf_config *perf_cfg = perf_ctx->perf;
   int written = 0;

   switch (query->queryinfo->kind) {
   case INTEL_PERF_QUERY_TYPE_OA:
   case INTEL_PERF_QUERY_TYPE_RAW:
      if (!query->oa.results_accumulated) {
         /* Due to the sampling frequency of the OA buffer by the i915-perf
          * driver, there can be a 5ms delay between the Mesa seeing the query
          * complete and i915 making all the OA buffer reports available to us.
          * We need to wait for all the reports to come in before we can do
          * the post processing removing unrelated deltas.
          * There is a i915-perf series to address this issue, but it's
          * not been merged upstream yet.
          */
         while (!read_oa_samples_for_query(perf_ctx, query, current_batch))
            ;

         uint32_t *begin_report = query->oa.map;
         uint32_t *end_report = query->oa.map + perf_cfg->query_layout.size;
         intel_perf_query_result_accumulate_fields(&query->oa.result,
                                                 query->queryinfo,
                                                 begin_report,
                                                 end_report,
                                                 true /* no_oa_accumulate */);
         accumulate_oa_reports(perf_ctx, query);
         assert(query->oa.results_accumulated);

         perf_cfg->vtbl.bo_unmap(query->oa.bo);
         query->oa.map = NULL;
      }
      if (query->queryinfo->kind == INTEL_PERF_QUERY_TYPE_OA) {
         written = get_oa_counter_data(perf_ctx, query, data_size, (uint8_t *)data);
      } else {
         const struct intel_device_info *devinfo = perf_ctx->devinfo;

         written = intel_perf_query_result_write_mdapi((uint8_t *)data, data_size,
                                                     devinfo, query->queryinfo,
                                                     &query->oa.result);
      }
      break;

   case INTEL_PERF_QUERY_TYPE_PIPELINE:
      written = get_pipeline_stats_data(perf_ctx, query, data_size, (uint8_t *)data);
      break;

   default:
      unreachable("Unknown query type");
      break;
   }

   if (bytes_written)
      *bytes_written = written;
}

void
intel_perf_dump_query_count(struct intel_perf_context *perf_ctx)
{
   DBG("Queries: (Open queries = %d, OA users = %d)\n",
       perf_ctx->n_active_oa_queries, perf_ctx->n_oa_users);
}

void
intel_perf_dump_query(struct intel_perf_context *ctx,
                      struct intel_perf_query_object *obj,
                      void *current_batch)
{
   switch (obj->queryinfo->kind) {
   case INTEL_PERF_QUERY_TYPE_OA:
   case INTEL_PERF_QUERY_TYPE_RAW:
      DBG("BO: %-4s OA data: %-10s %-15s\n",
          obj->oa.bo ? "yes," : "no,",
          intel_perf_is_query_ready(ctx, obj, current_batch) ? "ready," : "not ready,",
          obj->oa.results_accumulated ? "accumulated" : "not accumulated");
      break;
   case INTEL_PERF_QUERY_TYPE_PIPELINE:
      DBG("BO: %-4s\n",
          obj->pipeline_stats.bo ? "yes" : "no");
      break;
   default:
      unreachable("Unknown query type");
      break;
   }
}
