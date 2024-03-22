#include "zink_query.h"

#include "zink_context.h"
#include "zink_clear.h"
#include "zink_program.h"
#include "zink_resource.h"
#include "zink_screen.h"

#include "util/u_dump.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"

#define NUM_QUERIES 500

struct zink_query_pool {
   struct list_head list;
   VkQueryType vk_query_type;
   VkQueryPipelineStatisticFlags pipeline_stats;
   VkQueryPool query_pool;
   unsigned last_range;
   unsigned refcount;
};

struct zink_query_buffer {
   struct list_head list;
   unsigned num_results;
   struct pipe_resource *buffers[PIPE_MAX_VERTEX_STREAMS];
};

struct zink_vk_query {
   struct zink_query_pool *pool;
   unsigned query_id;
   bool needs_reset;
   bool started;
   uint32_t refcount;
};

struct zink_query_start {
   union {
      struct {
         bool have_gs;
         bool have_xfb;
         bool was_line_loop;
      };
      uint32_t data;
   };
   struct zink_vk_query *vkq[PIPE_MAX_VERTEX_STREAMS];
};

struct zink_query {
   struct threaded_query base;
   enum pipe_query_type type;

   /* Everytime the gallium query needs
    * another vulkan query, add a new start.
    */
   struct util_dynarray starts;
   unsigned start_offset;

   VkQueryType vkqtype;
   unsigned index;
   bool precise;

   bool active; /* query is considered active by vk */
   bool needs_reset; /* query is considered active by vk and cannot be destroyed */
   bool dead; /* query should be destroyed when its fence finishes */
   bool needs_update; /* query needs to update its qbos */
   bool needs_rast_discard_workaround; /* query needs discard disabled */
   bool suspended;
   bool started_in_rp; //needs to be stopped in rp

   struct list_head active_list;

   struct list_head stats_list; /* when active, statistics queries are added to ctx->primitives_generated_queries */
   bool has_draws; /* have_gs and have_xfb are valid for idx=curr_query */

   struct zink_batch_usage *batch_uses; //batch that the query was started in

   struct list_head buffers;
   union {
      struct zink_query_buffer *curr_qbo;
      struct pipe_fence_handle *fence; //PIPE_QUERY_GPU_FINISHED
   };

   struct zink_resource *predicate;
   bool predicate_dirty;
};

static inline int
get_num_starts(struct zink_query *q)
{
   return util_dynarray_num_elements(&q->starts, struct zink_query_start);
}

static void
update_query_id(struct zink_context *ctx, struct zink_query *q);


static VkQueryPipelineStatisticFlags
pipeline_statistic_convert(enum pipe_statistics_query_index idx)
{
   unsigned map[] = {
      [PIPE_STAT_QUERY_IA_VERTICES] = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT,
      [PIPE_STAT_QUERY_IA_PRIMITIVES] = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT,
      [PIPE_STAT_QUERY_VS_INVOCATIONS] = VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT,
      [PIPE_STAT_QUERY_GS_INVOCATIONS] = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT,
      [PIPE_STAT_QUERY_GS_PRIMITIVES] = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT,
      [PIPE_STAT_QUERY_C_INVOCATIONS] = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT,
      [PIPE_STAT_QUERY_C_PRIMITIVES] = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT,
      [PIPE_STAT_QUERY_PS_INVOCATIONS] = VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT,
      [PIPE_STAT_QUERY_HS_INVOCATIONS] = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT,
      [PIPE_STAT_QUERY_DS_INVOCATIONS] = VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT,
      [PIPE_STAT_QUERY_CS_INVOCATIONS] = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT
   };
   assert(idx < ARRAY_SIZE(map));
   return map[idx];
}

static void
begin_vk_query_indexed(struct zink_context *ctx, struct zink_vk_query *vkq, int index,
                       VkQueryControlFlags flags)
{
   struct zink_batch *batch = &ctx->batch;
   if (!vkq->started) {
      VKCTX(CmdBeginQueryIndexedEXT)(batch->state->cmdbuf,
                                     vkq->pool->query_pool,
                                     vkq->query_id,
                                     flags,
                                     index);
      vkq->started = true;
   }
}

static void
end_vk_query_indexed(struct zink_context *ctx, struct zink_vk_query *vkq, int index)
{
   struct zink_batch *batch = &ctx->batch;
   if (vkq->started) {
      VKCTX(CmdEndQueryIndexedEXT)(batch->state->cmdbuf,
                                   vkq->pool->query_pool,
                                   vkq->query_id, index);
      vkq->started = false;
   }
}

static void
reset_vk_query_pool(struct zink_context *ctx, struct zink_vk_query *vkq)
{
   struct zink_batch *batch = &ctx->batch;
   if (vkq->needs_reset) {
      VKCTX(CmdResetQueryPool)(batch->state->reordered_cmdbuf, vkq->pool->query_pool, vkq->query_id, 1);
      batch->state->has_barriers = true;
   }
   vkq->needs_reset = false;
}

void
zink_context_destroy_query_pools(struct zink_context *ctx)
{
   struct zink_screen *screen = zink_screen(ctx->base.screen);
   list_for_each_entry_safe(struct zink_query_pool, pool, &ctx->query_pools, list) {
      VKSCR(DestroyQueryPool)(screen->dev, pool->query_pool, NULL);
      list_del(&pool->list);
      FREE(pool);
   }
}

static struct zink_query_pool *
find_or_allocate_qp(struct zink_context *ctx, struct zink_query *q, unsigned idx)
{
   VkQueryPipelineStatisticFlags pipeline_stats = 0;
   if (q->type == PIPE_QUERY_PRIMITIVES_GENERATED && q->vkqtype != VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT)
      pipeline_stats = VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT |
                       VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT;
   else if (q->type == PIPE_QUERY_PIPELINE_STATISTICS_SINGLE)
      pipeline_stats = pipeline_statistic_convert(q->index);

   VkQueryType vk_query_type = q->vkqtype;
   /* if xfb is active, we need to use an xfb query, otherwise we need pipeline statistics */
   if (q->type == PIPE_QUERY_PRIMITIVES_GENERATED && idx == 1) {
      vk_query_type = VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
      pipeline_stats = 0;
   }

   struct zink_screen *screen = zink_screen(ctx->base.screen);
   list_for_each_entry(struct zink_query_pool, pool, &ctx->query_pools, list) {
      if (pool->vk_query_type == vk_query_type) {
         if (vk_query_type == VK_QUERY_TYPE_PIPELINE_STATISTICS) {
            if (pool->pipeline_stats == pipeline_stats)
               return pool;
         } else
            return pool;
      }
   }

   struct zink_query_pool *new_pool = CALLOC_STRUCT(zink_query_pool);
   if (!new_pool)
      return NULL;

   new_pool->vk_query_type = vk_query_type;
   new_pool->pipeline_stats = pipeline_stats;

   VkQueryPoolCreateInfo pool_create = {0};
   pool_create.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
   pool_create.queryType = vk_query_type;
   pool_create.queryCount = NUM_QUERIES;
   pool_create.pipelineStatistics = pipeline_stats;

   VkResult status = VKSCR(CreateQueryPool)(screen->dev, &pool_create, NULL, &new_pool->query_pool);
   if (status != VK_SUCCESS) {
      mesa_loge("ZINK: vkCreateQueryPool failed (%s)", vk_Result_to_str(status));
      FREE(new_pool);
      return NULL;
   }

   list_addtail(&new_pool->list, &ctx->query_pools);
   return new_pool;
}

static void
update_qbo(struct zink_context *ctx, struct zink_query *q);
static void
reset_qbos(struct zink_context *ctx, struct zink_query *q);


static bool
is_emulated_primgen(const struct zink_query *q)
{
   return q->type == PIPE_QUERY_PRIMITIVES_GENERATED &&
          q->vkqtype != VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
}

static inline unsigned
get_num_query_pools(struct zink_query *q)
{
   if (is_emulated_primgen(q))
      return 2;
   return 1;
}

static inline unsigned
get_num_queries(struct zink_query *q)
{
   if (is_emulated_primgen(q))
      return 2;
   if (q->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE)
      return PIPE_MAX_VERTEX_STREAMS;
   return 1;
}

static inline unsigned
get_num_results(struct zink_query *q)
{
   if (q->vkqtype == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT)
      return 1;
   switch (q->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
   case PIPE_QUERY_TIME_ELAPSED:
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_PIPELINE_STATISTICS_SINGLE:
      return 1;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      return 2;
   default:
      debug_printf("unknown query: %s\n",
                   util_str_query_type(q->type, true));
      unreachable("zink: unknown query type");
   }
}

static void
timestamp_to_nanoseconds(struct zink_screen *screen, uint64_t *timestamp)
{
   /* The number of valid bits in a timestamp value is determined by
    * the VkQueueFamilyProperties::timestampValidBits property of the queue on which the timestamp is written.
    * - 17.5. Timestamp Queries
    */
   if (screen->timestamp_valid_bits < 64)
      *timestamp &= (1ull << screen->timestamp_valid_bits) - 1;

   /* The number of nanoseconds it takes for a timestamp value to be incremented by 1
    * can be obtained from VkPhysicalDeviceLimits::timestampPeriod
    * - 17.5. Timestamp Queries
    */
   *timestamp *= (double)screen->info.props.limits.timestampPeriod;
}

static VkQueryType
convert_query_type(struct zink_screen *screen, enum pipe_query_type query_type, bool *precise)
{
   *precise = false;
   switch (query_type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
      *precise = true;
      FALLTHROUGH;
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      return VK_QUERY_TYPE_OCCLUSION;
   case PIPE_QUERY_TIME_ELAPSED:
   case PIPE_QUERY_TIMESTAMP:
      return VK_QUERY_TYPE_TIMESTAMP;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      return screen->info.have_EXT_primitives_generated_query ?
             VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT :
             VK_QUERY_TYPE_PIPELINE_STATISTICS;
   case PIPE_QUERY_PIPELINE_STATISTICS_SINGLE:
      return VK_QUERY_TYPE_PIPELINE_STATISTICS;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      return VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT;
   default:
      debug_printf("unknown query: %s\n",
                   util_str_query_type(query_type, true));
      unreachable("zink: unknown query type");
   }
}

static bool
needs_stats_list(struct zink_query *query)
{
   return is_emulated_primgen(query) ||
          query->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE ||
          query->type == PIPE_QUERY_SO_OVERFLOW_PREDICATE;
}

static bool
is_time_query(struct zink_query *query)
{
   return query->type == PIPE_QUERY_TIMESTAMP || query->type == PIPE_QUERY_TIME_ELAPSED;
}

static bool
is_so_overflow_query(struct zink_query *query)
{
   return query->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE || query->type == PIPE_QUERY_SO_OVERFLOW_PREDICATE;
}

static bool
is_bool_query(struct zink_query *query)
{
   return is_so_overflow_query(query) ||
          query->type == PIPE_QUERY_OCCLUSION_PREDICATE ||
          query->type == PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE ||
          query->type == PIPE_QUERY_GPU_FINISHED;
}

static bool
qbo_append(struct pipe_screen *screen, struct zink_query *query)
{
   if (query->curr_qbo && query->curr_qbo->list.next)
      return true;
   struct zink_query_buffer *qbo = CALLOC_STRUCT(zink_query_buffer);
   if (!qbo)
      return false;
   int num_buffers = get_num_queries(query);

   for (unsigned i = 0; i < num_buffers; i++) {
      qbo->buffers[i] = pipe_buffer_create(screen, PIPE_BIND_QUERY_BUFFER,
                                           PIPE_USAGE_STAGING,
                                           /* this is the maximum possible size of the results in a given buffer */
                                           (query->type == PIPE_QUERY_TIMESTAMP ? 1 : NUM_QUERIES) * get_num_results(query) * sizeof(uint64_t));
      if (!qbo->buffers[i])
         goto fail;
   }
   list_addtail(&qbo->list, &query->buffers);

   return true;
fail:
   for (unsigned i = 0; i < num_buffers; i++)
      pipe_resource_reference(&qbo->buffers[i], NULL);
   FREE(qbo);
   return false;
}

static void
unref_vk_pool(struct zink_context *ctx, struct zink_query_pool *pool)
{
   if (!pool || --pool->refcount)
      return;
   util_dynarray_append(&ctx->batch.state->dead_querypools, VkQueryPool, pool->query_pool);
   if (list_is_linked(&pool->list))
      list_del(&pool->list);
   FREE(pool);
}

static void
unref_vk_query(struct zink_context *ctx, struct zink_vk_query *vkq)
{
   if (!vkq)
      return;
   unref_vk_pool(ctx, vkq->pool);
   vkq->refcount--;
   if (vkq->refcount == 0)
      FREE(vkq);
}

static void
destroy_query(struct zink_context *ctx, struct zink_query *query)
{
   ASSERTED struct zink_screen *screen = zink_screen(ctx->base.screen);
   assert(zink_screen_usage_check_completion(screen, query->batch_uses));
   struct zink_query_buffer *qbo, *next;

   struct zink_query_start *starts = query->starts.data;
   unsigned num_starts = query->starts.capacity / sizeof(struct zink_query_start);
   for (unsigned j = 0; j < num_starts; j++) {
      for (unsigned i = 0; i < PIPE_MAX_VERTEX_STREAMS; i++) {
         unref_vk_query(ctx, starts[j].vkq[i]);
      }
   }

   util_dynarray_fini(&query->starts);
   LIST_FOR_EACH_ENTRY_SAFE(qbo, next, &query->buffers, list) {
      for (unsigned i = 0; i < ARRAY_SIZE(qbo->buffers); i++)
         pipe_resource_reference(&qbo->buffers[i], NULL);
      FREE(qbo);
   }
   pipe_resource_reference((struct pipe_resource**)&query->predicate, NULL);
   FREE(query);
}

static void
reset_qbo(struct zink_query *q)
{
   q->curr_qbo = list_first_entry(&q->buffers, struct zink_query_buffer, list);
   q->curr_qbo->num_results = 0;
}

static void
query_pool_get_range(struct zink_context *ctx, struct zink_query *q)
{
   bool is_timestamp = q->type == PIPE_QUERY_TIMESTAMP;
   struct zink_query_start *start;
   int num_queries = get_num_queries(q);
   if (!is_timestamp || get_num_starts(q) == 0) {
      size_t size = q->starts.capacity;
      start = util_dynarray_grow(&q->starts, struct zink_query_start, 1);
      if (size != q->starts.capacity) {
         /* when resizing, always zero the new data to avoid garbage */
         uint8_t *data = q->starts.data;
         memset(data + size, 0, q->starts.capacity - size);
      }
   } else {
      start = util_dynarray_top_ptr(&q->starts, struct zink_query_start);
   }
   start->data = 0;

   unsigned num_pools = get_num_query_pools(q);
   for (unsigned i = 0; i < num_queries; i++) {
      int pool_idx = num_pools > 1 ? i : 0;
      /* try and find the active query for this */
      struct zink_vk_query *vkq;
      int xfb_idx = num_queries == 4 ? i : q->index;
      if ((q->vkqtype == VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT ||
           (pool_idx == 1)) && ctx->curr_xfb_queries[xfb_idx]) {
         vkq = ctx->curr_xfb_queries[xfb_idx];
         vkq->refcount++;
         vkq->pool->refcount++;
      } else {
         struct zink_query_pool *pool = find_or_allocate_qp(ctx, q, pool_idx);
         if (pool->last_range == NUM_QUERIES) {
            list_del(&pool->list);
            pool = find_or_allocate_qp(ctx, q, pool_idx);
         }
         vkq = CALLOC_STRUCT(zink_vk_query);
         if (!vkq) {
            mesa_loge("ZINK: failed to allocate vkq!");
            return;
         }

         pool->refcount++;
         vkq->refcount = 1;
         vkq->needs_reset = true;
         vkq->pool = pool;
         vkq->started = false;
         vkq->query_id = pool->last_range++;
      }
      unref_vk_query(ctx, start->vkq[i]);
      start->vkq[i] = vkq;
   }
}

static struct pipe_query *
zink_create_query(struct pipe_context *pctx,
                  unsigned query_type, unsigned index)
{
   struct zink_screen *screen = zink_screen(pctx->screen);
   struct zink_query *query = CALLOC_STRUCT(zink_query);

   if (!query)
      return NULL;
   list_inithead(&query->buffers);

   query->index = index;
   query->type = query_type;
   if (query->type == PIPE_QUERY_GPU_FINISHED || query->type == PIPE_QUERY_TIMESTAMP_DISJOINT)
      return (struct pipe_query *)query;
   query->vkqtype = convert_query_type(screen, query_type, &query->precise);
   if (query->vkqtype == -1)
      return NULL;

   util_dynarray_init(&query->starts, NULL);

   assert(!query->precise || query->vkqtype == VK_QUERY_TYPE_OCCLUSION);

   /* use emulated path for drivers without full support */
   if (query->vkqtype == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT && index &&
       !screen->info.primgen_feats.primitivesGeneratedQueryWithNonZeroStreams)
      query->vkqtype = VK_QUERY_TYPE_PIPELINE_STATISTICS;

   if (query->vkqtype == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
      query->needs_rast_discard_workaround = !screen->info.primgen_feats.primitivesGeneratedQueryWithRasterizerDiscard;
   } else if (query_type == PIPE_QUERY_PRIMITIVES_GENERATED) {
      query->needs_rast_discard_workaround = true;
   }

   if (!qbo_append(pctx->screen, query))
      goto fail;
   struct zink_batch *batch = &zink_context(pctx)->batch;
   batch->has_work = true;
   query->needs_reset = true;
   query->predicate_dirty = true;
   if (query->type == PIPE_QUERY_TIMESTAMP) {
      query->active = true;
      /* defer pool reset until end_query since we're guaranteed to be threadsafe then */
      reset_qbo(query);
   }
   return (struct pipe_query *)query;
fail:
   destroy_query(zink_context(pctx), query);
   return NULL;
}

static void
zink_destroy_query(struct pipe_context *pctx,
                   struct pipe_query *q)
{
   struct zink_query *query = (struct zink_query *)q;

   /* only destroy if this query isn't active on any batches,
    * otherwise just mark dead and wait
    */
   if (query->batch_uses) {
      query->dead = true;
      return;
   }

   destroy_query(zink_context(pctx), query);
}

void
zink_prune_query(struct zink_batch_state *bs, struct zink_query *query)
{
   if (!zink_batch_usage_matches(query->batch_uses, bs))
      return;
   query->batch_uses = NULL;
   if (query->dead)
      destroy_query(bs->ctx, query);
}

static void
check_query_results(struct zink_query *query, union pipe_query_result *result,
                    int num_starts, uint64_t *results, uint64_t *xfb_results)
{
   uint64_t last_val = 0;
   int result_size = get_num_results(query);
   int idx = 0;
   util_dynarray_foreach(&query->starts, struct zink_query_start, start) {
      unsigned i = idx * result_size;
      idx++;
      switch (query->type) {
      case PIPE_QUERY_OCCLUSION_PREDICATE:
      case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      case PIPE_QUERY_GPU_FINISHED:
         result->b |= results[i] != 0;
         break;

      case PIPE_QUERY_TIME_ELAPSED:
      case PIPE_QUERY_TIMESTAMP:
         /* the application can sum the differences between all N queries to determine the total execution time.
          * - 17.5. Timestamp Queries
          */
         if (query->type != PIPE_QUERY_TIME_ELAPSED || i)
            result->u64 += results[i] - last_val;
         last_val = results[i];
         break;
      case PIPE_QUERY_OCCLUSION_COUNTER:
         result->u64 += results[i];
         break;
      case PIPE_QUERY_PRIMITIVES_GENERATED:
         if (query->vkqtype == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT)
            result->u64 += results[i];
         else if (start->have_xfb || query->index)
            result->u64 += xfb_results[i + 1];
         else
            /* if a given draw had a geometry shader, we need to use the first result */
            result->u64 += results[i + !start->have_gs];
         break;
      case PIPE_QUERY_PRIMITIVES_EMITTED:
         /* A query pool created with this type will capture 2 integers -
          * numPrimitivesWritten and numPrimitivesNeeded -
          * for the specified vertex stream output from the last vertex processing stage.
          * - from VK_EXT_transform_feedback spec
          */
         result->u64 += results[i];
         break;
      case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
         /* A query pool created with this type will capture 2 integers -
          * numPrimitivesWritten and numPrimitivesNeeded -
          * for the specified vertex stream output from the last vertex processing stage.
          * - from VK_EXT_transform_feedback spec
          */
         if (start->have_xfb)
            result->b |= results[i] != results[i + 1];
         break;
      case PIPE_QUERY_PIPELINE_STATISTICS_SINGLE:
         switch (query->index) {
         case PIPE_STAT_QUERY_IA_VERTICES:
            result->u64 += start->was_line_loop ? results[i] / 2 : results[i];
            break;
         default:
            result->u64 += results[i];
            break;
         }
         break;

      default:
         debug_printf("unhandled query type: %s\n",
                      util_str_query_type(query->type, true));
         unreachable("unexpected query type");
      }
   }
}

static bool
get_query_result(struct pipe_context *pctx,
                      struct pipe_query *q,
                      bool wait,
                      union pipe_query_result *result)
{
   struct zink_screen *screen = zink_screen(pctx->screen);
   struct zink_query *query = (struct zink_query *)q;
   unsigned flags = PIPE_MAP_READ;

   if (!wait)
      flags |= PIPE_MAP_DONTBLOCK;
   if (query->base.flushed)
      /* this is not a context-safe operation; ensure map doesn't use slab alloc */
      flags |= PIPE_MAP_THREAD_SAFE;

   util_query_clear_result(result, query->type);

   int num_starts = get_num_starts(query);
   /* no results: return zero */
   if (!num_starts)
      return true;
   int result_size = get_num_results(query) * sizeof(uint64_t);
   int num_maps = get_num_queries(query);

   struct zink_query_buffer *qbo;
   struct pipe_transfer *xfer[PIPE_MAX_VERTEX_STREAMS] = { 0 };
   LIST_FOR_EACH_ENTRY(qbo, &query->buffers, list) {
      uint64_t *results[PIPE_MAX_VERTEX_STREAMS] = { NULL, NULL };
      bool is_timestamp = query->type == PIPE_QUERY_TIMESTAMP;
      if (!qbo->num_results)
         continue;

      for (unsigned i = 0; i < num_maps; i++) {
         results[i] = pipe_buffer_map_range(pctx, qbo->buffers[i], 0,
                                            (is_timestamp ? 1 : qbo->num_results) * result_size, flags, &xfer[i]);
         if (!results[i]) {
            if (wait)
               debug_printf("zink: qbo read failed!");
            goto fail;
         }
      }
      if (query->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE) {
         for (unsigned i = 0; i < PIPE_MAX_VERTEX_STREAMS && !result->b; i++) {
            check_query_results(query, result, num_starts, results[i], NULL);
         }
      } else
         check_query_results(query, result, num_starts, results[0], results[1]);

      for (unsigned i = 0 ; i < num_maps; i++)
         pipe_buffer_unmap(pctx, xfer[i]);

      /* if overflow is detected we can stop */
      if (query->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE && result->b)
         break;
   }

   if (is_time_query(query))
      timestamp_to_nanoseconds(screen, &result->u64);

   return true;
fail:
   for (unsigned i = 0 ; i < num_maps; i++)
      if (xfer[i])
         pipe_buffer_unmap(pctx, xfer[i]);
   return false;
}

static void
force_cpu_read(struct zink_context *ctx, struct pipe_query *pquery, enum pipe_query_value_type result_type, struct pipe_resource *pres, unsigned offset)
{
   struct pipe_context *pctx = &ctx->base;
   unsigned result_size = result_type <= PIPE_QUERY_TYPE_U32 ? sizeof(uint32_t) : sizeof(uint64_t);
   struct zink_query *query = (struct zink_query*)pquery;
   union pipe_query_result result;

   if (query->needs_update)
      update_qbo(ctx, query);

   bool success = get_query_result(pctx, pquery, true, &result);
   if (!success) {
      debug_printf("zink: getting query result failed\n");
      return;
   }

   if (result_type <= PIPE_QUERY_TYPE_U32) {
      uint32_t u32;
      uint32_t limit;
      if (result_type == PIPE_QUERY_TYPE_I32)
         limit = INT_MAX;
      else
         limit = UINT_MAX;
      if (is_bool_query(query))
         u32 = result.b;
      else
         u32 = MIN2(limit, result.u64);
      tc_buffer_write(pctx, pres, offset, result_size, &u32);
   } else {
      uint64_t u64;
      if (is_bool_query(query))
         u64 = result.b;
      else
         u64 = result.u64;
      tc_buffer_write(pctx, pres, offset, result_size, &u64);
   }
}

static void
copy_pool_results_to_buffer(struct zink_context *ctx, struct zink_query *query, VkQueryPool pool,
                            unsigned query_id, struct zink_resource *res, unsigned offset,
                            int num_results, VkQueryResultFlags flags)
{
   struct zink_batch *batch = &ctx->batch;
   unsigned type_size = (flags & VK_QUERY_RESULT_64_BIT) ? sizeof(uint64_t) : sizeof(uint32_t);
   unsigned base_result_size = get_num_results(query) * type_size;
   unsigned result_size = base_result_size * num_results;
   if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)
      result_size += type_size;

   bool marker = zink_cmd_debug_marker_begin(ctx, VK_NULL_HANDLE, "update_qbo(%s: id=%u, num_results=%d)", vk_QueryType_to_str(query->vkqtype), query_id, num_results);

   zink_batch_no_rp(ctx);
   /* if it's a single query that doesn't need special handling, we can copy it and be done */
   zink_batch_reference_resource_rw(batch, res, true);
   res->obj->access = VK_ACCESS_TRANSFER_WRITE_BIT;
   res->obj->access_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
   util_range_add(&res->base.b, &res->valid_buffer_range, offset, offset + result_size);
   assert(query_id < NUM_QUERIES);
   res->obj->unordered_read = res->obj->unordered_write = false;
   VKCTX(CmdCopyQueryPoolResults)(batch->state->cmdbuf, pool, query_id, num_results, res->obj->buffer,
                                  offset, base_result_size, flags);
   zink_cmd_debug_marker_end(ctx, batch->state->cmdbuf, marker);
}

static void
copy_results_to_buffer(struct zink_context *ctx, struct zink_query *query, struct zink_resource *res, unsigned offset, int num_results, VkQueryResultFlags flags)
{
   struct zink_query_start *start = util_dynarray_top_ptr(&query->starts, struct zink_query_start);
   copy_pool_results_to_buffer(ctx, query, start->vkq[0]->pool->query_pool, start->vkq[0]->query_id, res, offset, num_results, flags);
}


static void
reset_query_range(struct zink_context *ctx, struct zink_query *q)
{
   int num_queries = get_num_queries(q);
   struct zink_query_start *start = util_dynarray_top_ptr(&q->starts, struct zink_query_start);
   for (unsigned i = 0; i < num_queries; i++) {
      reset_vk_query_pool(ctx, start->vkq[i]);
   }
}

static void
reset_qbos(struct zink_context *ctx, struct zink_query *q)
{
   if (q->needs_update)
      update_qbo(ctx, q);

   q->needs_reset = false;
   /* create new qbo for non-timestamp queries:
    * timestamp queries should never need more than 2 entries in the qbo
    */
   if (q->type == PIPE_QUERY_TIMESTAMP)
      return;
   if (qbo_append(ctx->base.screen, q))
      reset_qbo(q);
   else
      debug_printf("zink: qbo alloc failed on reset!");
}

static inline unsigned
get_buffer_offset(struct zink_query *q)
{
   return (get_num_starts(q) - 1) * get_num_results(q) * sizeof(uint64_t);
}

static void
update_qbo(struct zink_context *ctx, struct zink_query *q)
{
   struct zink_query_buffer *qbo = q->curr_qbo;
   unsigned num_starts = get_num_starts(q);
   struct zink_query_start *starts = q->starts.data;
   bool is_timestamp = q->type == PIPE_QUERY_TIMESTAMP;
   /* timestamp queries just write to offset 0 always */
   int num_queries = get_num_queries(q);
   unsigned num_results = qbo->num_results;
   for (unsigned i = 0; i < num_queries; i++) {
      unsigned start_offset = q->start_offset;
      while (start_offset < num_starts) {
         unsigned num_merged_copies = 0;
         VkQueryPool qp = starts[start_offset].vkq[i]->pool->query_pool;
         unsigned base_id = starts[start_offset].vkq[i]->query_id;
         /* iterate over all the starts to see how many can be merged */
         for (unsigned j = start_offset; j < num_starts; j++, num_merged_copies++) {
            if (starts[j].vkq[i]->pool->query_pool != qp || starts[j].vkq[i]->query_id != base_id + num_merged_copies)
               break;
         }
         assert(num_merged_copies);
         unsigned cur_offset = start_offset * get_num_results(q) * sizeof(uint64_t);
         unsigned offset = is_timestamp ? 0 : cur_offset;
         copy_pool_results_to_buffer(ctx, q, starts[start_offset].vkq[i]->pool->query_pool, starts[start_offset].vkq[i]->query_id,
                                    zink_resource(qbo->buffers[i]),
                                    offset,
                                    num_merged_copies,
                                    /*
                                       there is an implicit execution dependency from
                                       each such query command to all query commands previously submitted to the same queue. There
                                       is one significant exception to this; if the flags parameter of vkCmdCopyQueryPoolResults does not
                                       include VK_QUERY_RESULT_WAIT_BIT, execution of vkCmdCopyQueryPoolResults may happen-before
                                       the results of vkCmdEndQuery are available.

                                    * - Chapter 18. Queries
                                    */
                                    VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
         if (!is_timestamp)
            q->curr_qbo->num_results += num_merged_copies;
         start_offset += num_merged_copies;
      }
   }
   q->start_offset += q->curr_qbo->num_results - num_results;


   if (is_timestamp)
      q->curr_qbo->num_results = 1;

   q->needs_update = false;
}

static void
begin_query(struct zink_context *ctx, struct zink_batch *batch, struct zink_query *q)
{
   VkQueryControlFlags flags = 0;

   if (q->type == PIPE_QUERY_TIMESTAMP_DISJOINT)
      return;

   if (q->type == PIPE_QUERY_PIPELINE_STATISTICS_SINGLE && q->index == PIPE_STAT_QUERY_CS_INVOCATIONS && ctx->batch.in_rp) {
      /* refuse to start CS queries in renderpasses */
      if (!list_is_linked(&q->active_list))
         list_addtail(&q->active_list, &ctx->suspended_queries);
      q->suspended = true;
      return;
   }

   zink_flush_dgc_if_enabled(ctx);

   update_query_id(ctx, q);
   q->predicate_dirty = true;
   if (q->needs_reset)
      reset_qbos(ctx, q);
   reset_query_range(ctx, q);
   q->active = true;
   batch->has_work = true;

   struct zink_query_start *start = util_dynarray_top_ptr(&q->starts, struct zink_query_start);
   if (q->type == PIPE_QUERY_TIME_ELAPSED) {
      VKCTX(CmdWriteTimestamp)(batch->state->cmdbuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, start->vkq[0]->pool->query_pool, start->vkq[0]->query_id);
      if (!batch->in_rp)
         update_qbo(ctx, q);
      zink_batch_usage_set(&q->batch_uses, batch->state);
      _mesa_set_add(&batch->state->active_queries, q);
   }
   /* ignore the rest of begin_query for timestamps */
   if (is_time_query(q))
      return;

   /* A query must either begin and end inside the same subpass of a render pass
      instance, or must both begin and end outside of a render pass instance
      (i.e. contain entire render pass instances).
      - 18.2. Query Operation
    */
   q->started_in_rp = ctx->batch.in_rp;

   if (q->precise)
      flags |= VK_QUERY_CONTROL_PRECISE_BIT;

   if (q->type == PIPE_QUERY_PRIMITIVES_EMITTED ||
       is_emulated_primgen(q) ||
       q->type == PIPE_QUERY_SO_OVERFLOW_PREDICATE) {
      struct zink_vk_query *vkq = start->vkq[1] ? start->vkq[1] : start->vkq[0];
      assert(!ctx->curr_xfb_queries[q->index] || ctx->curr_xfb_queries[q->index] == vkq);
      ctx->curr_xfb_queries[q->index] = vkq;

      begin_vk_query_indexed(ctx, vkq, q->index, flags);
   } else if (q->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE) {
      for (unsigned i = 0; i < PIPE_MAX_VERTEX_STREAMS; i++) {
         assert(!ctx->curr_xfb_queries[i] || ctx->curr_xfb_queries[i] == start->vkq[i]);
         ctx->curr_xfb_queries[i] = start->vkq[i];

         begin_vk_query_indexed(ctx, start->vkq[i], i, flags);
      }
   } else if (q->vkqtype == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
      begin_vk_query_indexed(ctx, start->vkq[0], q->index, flags);
   }
   if (q->vkqtype != VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT && q->vkqtype != VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT)
      VKCTX(CmdBeginQuery)(batch->state->cmdbuf, start->vkq[0]->pool->query_pool, start->vkq[0]->query_id, flags);
   if (q->type == PIPE_QUERY_PIPELINE_STATISTICS_SINGLE && q->index == PIPE_STAT_QUERY_IA_VERTICES)  {
      assert(!ctx->vertices_query);
      ctx->vertices_query = q;
   }
   if (needs_stats_list(q))
      list_addtail(&q->stats_list, &ctx->primitives_generated_queries);
   zink_batch_usage_set(&q->batch_uses, batch->state);
   _mesa_set_add(&batch->state->active_queries, q);
   if (q->needs_rast_discard_workaround) {
      ctx->primitives_generated_active = true;
      if (zink_set_rasterizer_discard(ctx, true))
         zink_set_null_fs(ctx);
   }
}

static bool
zink_begin_query(struct pipe_context *pctx,
                 struct pipe_query *q)
{
   struct zink_query *query = (struct zink_query *)q;
   struct zink_context *ctx = zink_context(pctx);
   struct zink_batch *batch = &ctx->batch;

   /* drop all past results */
   reset_qbo(query);

   if (query->vkqtype == VK_QUERY_TYPE_OCCLUSION)
      ctx->occlusion_query_active = true;
   if (query->type == PIPE_QUERY_PIPELINE_STATISTICS_SINGLE && query->index == PIPE_STAT_QUERY_PS_INVOCATIONS)
      ctx->fs_query_active = true;

   query->predicate_dirty = true;

   util_dynarray_clear(&query->starts);
   query->start_offset = 0;

   if (batch->in_rp) {
      begin_query(ctx, batch, query);
   } else {
      /* never directly start queries out of renderpass, always defer */
      list_addtail(&query->active_list, &ctx->suspended_queries);
      query->suspended = true;
      if (query->type == PIPE_QUERY_PRIMITIVES_GENERATED)
         ctx->primitives_generated_suspended = query->needs_rast_discard_workaround;
   }

   return true;
}

static void
update_query_id(struct zink_context *ctx, struct zink_query *q)
{
   query_pool_get_range(ctx, q);
   ctx->batch.has_work = true;
   q->has_draws = false;
}

static void
end_query(struct zink_context *ctx, struct zink_batch *batch, struct zink_query *q)
{
   if (q->type == PIPE_QUERY_TIMESTAMP_DISJOINT)
      return;

   zink_flush_dgc_if_enabled(ctx);

   ASSERTED struct zink_query_buffer *qbo = q->curr_qbo;
   assert(qbo);
   assert(!is_time_query(q));
   q->active = false;
   assert(q->started_in_rp == batch->in_rp);
   struct zink_query_start *start = util_dynarray_top_ptr(&q->starts, struct zink_query_start);

   if (q->type == PIPE_QUERY_PRIMITIVES_EMITTED ||
       is_emulated_primgen(q) ||
       q->type == PIPE_QUERY_SO_OVERFLOW_PREDICATE) {
      struct zink_vk_query *vkq = start->vkq[1] ? start->vkq[1] : start->vkq[0];

      end_vk_query_indexed(ctx, vkq, q->index);
      ctx->curr_xfb_queries[q->index] = NULL;
   }
   else if (q->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE) {
      for (unsigned i = 0; i < PIPE_MAX_VERTEX_STREAMS; i++) {
         end_vk_query_indexed(ctx, start->vkq[i], i);
         ctx->curr_xfb_queries[i] = NULL;
      }
   } else if (q->vkqtype == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
      end_vk_query_indexed(ctx, start->vkq[0], q->index);
   }
   if (q->vkqtype != VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT &&
       q->vkqtype != VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT && !is_time_query(q))
      VKCTX(CmdEndQuery)(batch->state->cmdbuf, start->vkq[0]->pool->query_pool, start->vkq[0]->query_id);

   if (q->type == PIPE_QUERY_PIPELINE_STATISTICS_SINGLE &&
       q->index == PIPE_STAT_QUERY_IA_VERTICES)
      ctx->vertices_query = NULL;

   if (needs_stats_list(q))
      list_delinit(&q->stats_list);

   q->needs_update = true;
   if (q->needs_rast_discard_workaround) {
      ctx->primitives_generated_active = false;
      if (zink_set_rasterizer_discard(ctx, false))
         zink_set_null_fs(ctx);
   }
}

static bool
zink_end_query(struct pipe_context *pctx,
               struct pipe_query *q)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_query *query = (struct zink_query *)q;
   struct zink_batch *batch = &ctx->batch;

   if (query->type == PIPE_QUERY_TIMESTAMP_DISJOINT)
      return true;

   if (query->type == PIPE_QUERY_GPU_FINISHED) {
      pctx->flush(pctx, &query->fence, PIPE_FLUSH_DEFERRED);
      return true;
   }

   /* FIXME: this can be called from a thread, but it needs to write to the cmdbuf */
   threaded_context_unwrap_sync(pctx);

   if (query->vkqtype == VK_QUERY_TYPE_OCCLUSION)
      ctx->occlusion_query_active = true;
   if (query->type == PIPE_QUERY_PIPELINE_STATISTICS_SINGLE && query->index == PIPE_STAT_QUERY_PS_INVOCATIONS)
      ctx->fs_query_active = true;

   bool unset_null_fs = query->type == PIPE_QUERY_PRIMITIVES_GENERATED && (ctx->primitives_generated_suspended || ctx->primitives_generated_active);
   if (query->type == PIPE_QUERY_PRIMITIVES_GENERATED)
      ctx->primitives_generated_suspended = false;

   if (list_is_linked(&query->stats_list))
      list_delinit(&query->stats_list);
   if (query->suspended) {
      list_delinit(&query->active_list);
      query->suspended = false;
   }
   if (is_time_query(query)) {
      update_query_id(ctx, query);
      if (query->needs_reset)
         reset_qbos(ctx, query);
      reset_query_range(ctx, query);
      struct zink_query_start *start = util_dynarray_top_ptr(&query->starts, struct zink_query_start);
      VKCTX(CmdWriteTimestamp)(batch->state->cmdbuf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                               start->vkq[0]->pool->query_pool, start->vkq[0]->query_id);
      zink_batch_usage_set(&query->batch_uses, batch->state);
      _mesa_set_add(&batch->state->active_queries, query);
      query->needs_update = true;
   } else if (query->active) {
      /* this should be a tc-optimized query end that doesn't split a renderpass */
      if (!query->started_in_rp)
         zink_batch_no_rp(ctx);
      end_query(ctx, batch, query);
   }

   if (unset_null_fs)
      zink_set_null_fs(ctx);

   return true;
}

static bool
zink_get_query_result(struct pipe_context *pctx,
                      struct pipe_query *q,
                      bool wait,
                      union pipe_query_result *result)
{
   struct zink_query *query = (void*)q;
   struct zink_context *ctx = zink_context(pctx);

   if (query->type == PIPE_QUERY_TIMESTAMP_DISJOINT) {
      result->timestamp_disjoint.frequency = zink_screen(pctx->screen)->info.props.limits.timestampPeriod * 1000000.0;
      result->timestamp_disjoint.disjoint = false;
      return true;
   }

   if (query->type == PIPE_QUERY_GPU_FINISHED) {
      struct pipe_screen *screen = pctx->screen;

      result->b = screen->fence_finish(screen, query->base.flushed ? NULL : pctx,
                                        query->fence, wait ? OS_TIMEOUT_INFINITE : 0);
      return result->b;
   }

   if (query->needs_update) {
      assert(!ctx->tc || !threaded_query(q)->flushed);
      update_qbo(ctx, query);
   }

   if (zink_batch_usage_is_unflushed(query->batch_uses)) {
      if (!threaded_query(q)->flushed)
         pctx->flush(pctx, NULL, 0);
      if (!wait)
         return false;
   }

   return get_query_result(pctx, q, wait, result);
}

static void
suspend_query(struct zink_context *ctx, struct zink_query *query)
{
   /* if a query isn't active here then we don't need to reactivate it on the next batch */
   if (query->active && !is_time_query(query))
      end_query(ctx, &ctx->batch, query);
   if (query->needs_update && !ctx->batch.in_rp)
      update_qbo(ctx, query);
}

static void
suspend_queries(struct zink_context *ctx, bool rp_only)
{
   set_foreach(&ctx->batch.state->active_queries, entry) {
      struct zink_query *query = (void*)entry->key;
      if (query->suspended || (rp_only && !query->started_in_rp))
         continue;
      if (query->active && !is_time_query(query)) {
         /* the fence is going to steal the set off the batch, so we have to copy
          * the active queries onto a list
          */
         list_addtail(&query->active_list, &ctx->suspended_queries);
         query->suspended = true;
         if (query->type == PIPE_QUERY_PRIMITIVES_GENERATED)
            ctx->primitives_generated_suspended = query->needs_rast_discard_workaround;
      }
      suspend_query(ctx, query);
   }
}

void
zink_suspend_queries(struct zink_context *ctx, struct zink_batch *batch)
{
   suspend_queries(ctx, false);
}

void
zink_resume_queries(struct zink_context *ctx, struct zink_batch *batch)
{
   struct zink_query *query, *next;
   LIST_FOR_EACH_ENTRY_SAFE(query, next, &ctx->suspended_queries, active_list) {
      list_delinit(&query->active_list);
      query->suspended = false;
      if (query->type == PIPE_QUERY_PRIMITIVES_GENERATED)
         ctx->primitives_generated_suspended = false;
      if (query->needs_update && !ctx->batch.in_rp)
         update_qbo(ctx, query);
      begin_query(ctx, batch, query);
   }
}

void
zink_resume_cs_query(struct zink_context *ctx)
{
   struct zink_query *query, *next;
   LIST_FOR_EACH_ENTRY_SAFE(query, next, &ctx->suspended_queries, active_list) {
      if (query->type == PIPE_QUERY_PIPELINE_STATISTICS_SINGLE && query->index == PIPE_STAT_QUERY_CS_INVOCATIONS) {
         list_delinit(&query->active_list);
         query->suspended = false;
         begin_query(ctx, &ctx->batch, query);
      }
   }
}

void
zink_query_renderpass_suspend(struct zink_context *ctx)
{
   suspend_queries(ctx, true);
}

void
zink_query_update_gs_states(struct zink_context *ctx)
{
   struct zink_query *query;
   bool suspendall = false;
   bool have_gs = !!ctx->gfx_stages[MESA_SHADER_GEOMETRY];
   bool have_xfb = !!ctx->num_so_targets;

   LIST_FOR_EACH_ENTRY(query, &ctx->primitives_generated_queries, stats_list) {
      struct zink_query_start *last_start = util_dynarray_top_ptr(&query->starts, struct zink_query_start);
      assert(query->active);
      if (query->has_draws) {
         if (last_start->have_gs != have_gs ||
             last_start->have_xfb != have_xfb) {
            suspendall = true;
         }
      }
   }

   if (ctx->vertices_query) {
      query = ctx->vertices_query;
      struct zink_query_start *last_start = util_dynarray_top_ptr(&query->starts, struct zink_query_start);
      assert(query->active);
      if (last_start->was_line_loop != ctx->was_line_loop) {
         suspendall = true;
      }
   }
   if (suspendall) {
     zink_suspend_queries(ctx, &ctx->batch);
     zink_resume_queries(ctx, &ctx->batch);
   }

   LIST_FOR_EACH_ENTRY(query, &ctx->primitives_generated_queries, stats_list) {
      struct zink_query_start *last_start = util_dynarray_top_ptr(&query->starts, struct zink_query_start);
      last_start->have_gs = have_gs;
      last_start->have_xfb = have_xfb;
      query->has_draws = true;
   }
   if (ctx->vertices_query) {
      query = ctx->vertices_query;
      struct zink_query_start *last_start = util_dynarray_top_ptr(&query->starts, struct zink_query_start);
      last_start->was_line_loop = ctx->was_line_loop;
      query->has_draws = true;
   }
}

static void
zink_set_active_query_state(struct pipe_context *pctx, bool enable)
{
   struct zink_context *ctx = zink_context(pctx);
   /* unordered blits already disable queries */
   if (ctx->unordered_blitting)
      return;
   ctx->queries_disabled = !enable;

   struct zink_batch *batch = &ctx->batch;
   if (ctx->queries_disabled)
      zink_suspend_queries(ctx, batch);
   else if (ctx->batch.in_rp)
      zink_resume_queries(ctx, batch);
}

void
zink_query_sync(struct zink_context *ctx, struct zink_query *query)
{
   if (query->needs_update)
      update_qbo(ctx, query);
}

void
zink_start_conditional_render(struct zink_context *ctx)
{
   if (unlikely(!zink_screen(ctx->base.screen)->info.have_EXT_conditional_rendering) || ctx->render_condition.active)
      return;
   struct zink_batch *batch = &ctx->batch;
   VkConditionalRenderingFlagsEXT begin_flags = 0;
   if (ctx->render_condition.inverted)
      begin_flags = VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT;
   VkConditionalRenderingBeginInfoEXT begin_info = {0};
   begin_info.sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
   begin_info.buffer = ctx->render_condition.query->predicate->obj->buffer;
   begin_info.flags = begin_flags;
   ctx->render_condition.query->predicate->obj->unordered_read = false;
   VKCTX(CmdBeginConditionalRenderingEXT)(batch->state->cmdbuf, &begin_info);
   zink_batch_reference_resource_rw(batch, ctx->render_condition.query->predicate, false);
   ctx->render_condition.active = true;
}

void
zink_stop_conditional_render(struct zink_context *ctx)
{
   zink_flush_dgc_if_enabled(ctx);
   struct zink_batch *batch = &ctx->batch;
   zink_clear_apply_conditionals(ctx);
   if (unlikely(!zink_screen(ctx->base.screen)->info.have_EXT_conditional_rendering) || !ctx->render_condition.active)
      return;
   VKCTX(CmdEndConditionalRenderingEXT)(batch->state->cmdbuf);
   ctx->render_condition.active = false;
}

static void
zink_render_condition(struct pipe_context *pctx,
                      struct pipe_query *pquery,
                      bool condition,
                      enum pipe_render_cond_flag mode)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_query *query = (struct zink_query *)pquery;
   zink_batch_no_rp(ctx);
   VkQueryResultFlagBits flags = 0;

   zink_flush_dgc_if_enabled(ctx);
   if (query == NULL) {
      /* force conditional clears if they exist */
      if (ctx->clears_enabled && !ctx->batch.in_rp)
         zink_batch_rp(ctx);
      zink_stop_conditional_render(ctx);
      ctx->render_condition_active = false;
      ctx->render_condition.query = NULL;
      return;
   }

   if (!query->predicate) {
      struct pipe_resource *pres;

      /* need to create a vulkan buffer to copy the data into */
      pres = pipe_buffer_create(pctx->screen, PIPE_BIND_QUERY_BUFFER, PIPE_USAGE_DEFAULT, sizeof(uint64_t));
      if (!pres)
         return;

      query->predicate = zink_resource(pres);
   }
   if (query->predicate_dirty) {
      struct zink_resource *res = query->predicate;

      if (mode == PIPE_RENDER_COND_WAIT || mode == PIPE_RENDER_COND_BY_REGION_WAIT)
         flags |= VK_QUERY_RESULT_WAIT_BIT;

      flags |= VK_QUERY_RESULT_64_BIT;
      int num_results = get_num_starts(query);
      if (num_results) {
         if (!is_emulated_primgen(query) &&
            !is_so_overflow_query(query) &&
            num_results == 1) {
            copy_results_to_buffer(ctx, query, res, 0, num_results, flags);
         } else {
            /* these need special handling */
            force_cpu_read(ctx, pquery, PIPE_QUERY_TYPE_U32, &res->base.b, 0);
         }
      } else {
         uint64_t zero = 0;
         tc_buffer_write(pctx, &res->base.b, 0, sizeof(zero), &zero);
      }
      zink_screen(ctx->base.screen)->buffer_barrier(ctx, res, VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT, VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT);
      query->predicate_dirty = false;
   }
   ctx->render_condition.inverted = condition;
   ctx->render_condition_active = true;
   ctx->render_condition.query = query;
   if (ctx->batch.in_rp)
      zink_start_conditional_render(ctx);
}

static void
zink_get_query_result_resource(struct pipe_context *pctx,
                               struct pipe_query *pquery,
                               enum pipe_query_flags flags,
                               enum pipe_query_value_type result_type,
                               int index,
                               struct pipe_resource *pres,
                               unsigned offset)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_screen *screen = zink_screen(pctx->screen);
   struct zink_query *query = (struct zink_query*)pquery;
   struct zink_resource *res = zink_resource(pres);
   unsigned result_size = result_type <= PIPE_QUERY_TYPE_U32 ? sizeof(uint32_t) : sizeof(uint64_t);
   VkQueryResultFlagBits size_flags = result_type <= PIPE_QUERY_TYPE_U32 ? 0 : VK_QUERY_RESULT_64_BIT;
   unsigned num_queries = get_num_starts(query);

   /* it's possible that a query may have no data at all: write out zeroes to the buffer and return */
   uint64_t u64[4] = {0};
   unsigned src_offset = result_size * get_num_results(query);
   if (!num_queries) {
      tc_buffer_write(pctx, pres, offset, result_size, (unsigned char*)u64 + src_offset);
      return;
   }

   if (index == -1) {
      /* VK_QUERY_RESULT_WITH_AVAILABILITY_BIT will ALWAYS write some kind of result data
       * in addition to the availability result, which is a problem if we're just trying to get availability data
       *
       * if we know that there's no valid buffer data in the preceding buffer range, then we can just
       * stomp on it with a glorious queued buffer copy instead of forcing a stall to manually write to the
       * buffer
       */

      VkQueryResultFlags flag = is_time_query(query) ? 0 : VK_QUERY_RESULT_PARTIAL_BIT;
      if (zink_batch_usage_check_completion(ctx, query->batch_uses)) {
         struct zink_query_start *start = util_dynarray_top_ptr(&query->starts, struct zink_query_start);
         unsigned query_id = start->vkq[0]->query_id;
         VkResult result = VKCTX(GetQueryPoolResults)(screen->dev, start->vkq[0]->pool->query_pool, query_id, 1,
                                   sizeof(u64), u64, 0, size_flags | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | flag);
         if (result == VK_SUCCESS) {
            tc_buffer_write(pctx, pres, offset, result_size, (unsigned char*)u64 + src_offset);
            return;
         } else {
            mesa_loge("ZINK: vkGetQueryPoolResults failed (%s)", vk_Result_to_str(result));
         }
      }
      struct pipe_resource *staging = pipe_buffer_create(pctx->screen, 0, PIPE_USAGE_STAGING, src_offset + result_size);
      copy_results_to_buffer(ctx, query, zink_resource(staging), 0, 1, size_flags | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT | flag);
      zink_copy_buffer(ctx, res, zink_resource(staging), offset, result_size * get_num_results(query), result_size);
      pipe_resource_reference(&staging, NULL);
      return;
   }

   /*
      there is an implicit execution dependency from
      each such query command to all query commands previously submitted to the same queue. There
      is one significant exception to this; if the flags parameter of vkCmdCopyQueryPoolResults does not
      include VK_QUERY_RESULT_WAIT_BIT, execution of vkCmdCopyQueryPoolResults may happen-before
      the results of vkCmdEndQuery are available.

    * - Chapter 18. Queries
    */
   size_flags |= VK_QUERY_RESULT_WAIT_BIT;
   if (!is_time_query(query) && !is_bool_query(query)) {
      if (num_queries == 1 && !is_emulated_primgen(query) &&
                              query->type != PIPE_QUERY_PRIMITIVES_EMITTED &&
                              !is_bool_query(query)) {
         if (size_flags == VK_QUERY_RESULT_64_BIT) {
            if (query->needs_update)
               update_qbo(ctx, query);
            /* internal qbo always writes 64bit value so we can just direct copy */
            zink_copy_buffer(ctx, res, zink_resource(query->curr_qbo->buffers[0]), offset,
                             get_buffer_offset(query),
                             result_size);
         } else
            /* have to do a new copy for 32bit */
            copy_results_to_buffer(ctx, query, res, offset, 1, size_flags);
         return;
      }
   }

   /* TODO: use CS to aggregate results */

   /* unfortunately, there's no way to accumulate results from multiple queries on the gpu without either
    * clobbering all but the last result or writing the results sequentially, so we have to manually write the result
    */
   force_cpu_read(ctx, pquery, result_type, pres, offset);
}

uint64_t
zink_get_timestamp(struct pipe_screen *pscreen)
{
   struct zink_screen *screen = zink_screen(pscreen);
   uint64_t timestamp, deviation;
   if (screen->info.have_EXT_calibrated_timestamps) {
      VkCalibratedTimestampInfoEXT cti = {0};
      cti.sType = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT;
      cti.timeDomain = VK_TIME_DOMAIN_DEVICE_EXT;
      VkResult result = VKSCR(GetCalibratedTimestampsEXT)(screen->dev, 1, &cti, &timestamp, &deviation);
      if (result != VK_SUCCESS) {
         mesa_loge("ZINK: vkGetCalibratedTimestampsEXT failed (%s)", vk_Result_to_str(result));
      }
   } else {
      zink_screen_lock_context(screen);
      struct pipe_context *pctx = &screen->copy_context->base;
      struct pipe_query *pquery = pctx->create_query(pctx, PIPE_QUERY_TIMESTAMP, 0);
      if (!pquery)
         return 0;
      union pipe_query_result result = {0};
      pctx->begin_query(pctx, pquery);
      pctx->end_query(pctx, pquery);
      pctx->get_query_result(pctx, pquery, true, &result);
      pctx->destroy_query(pctx, pquery);
      zink_screen_unlock_context(screen);
      timestamp = result.u64;
   }
   timestamp_to_nanoseconds(screen, &timestamp);
   return timestamp;
}

void
zink_context_query_init(struct pipe_context *pctx)
{
   struct zink_context *ctx = zink_context(pctx);
   list_inithead(&ctx->suspended_queries);
   list_inithead(&ctx->primitives_generated_queries);

   pctx->create_query = zink_create_query;
   pctx->destroy_query = zink_destroy_query;
   pctx->begin_query = zink_begin_query;
   pctx->end_query = zink_end_query;
   pctx->get_query_result = zink_get_query_result;
   pctx->get_query_result_resource = zink_get_query_result_resource;
   pctx->set_active_query_state = zink_set_active_query_state;
   pctx->render_condition = zink_render_condition;
}
