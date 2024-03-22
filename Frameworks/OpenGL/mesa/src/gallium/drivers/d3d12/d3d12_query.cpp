/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "d3d12_query.h"
#include "d3d12_compiler.h"
#include "d3d12_compute_transforms.h"
#include "d3d12_context.h"
#include "d3d12_resource.h"
#include "d3d12_screen.h"
#include "d3d12_fence.h"

#include "util/u_dump.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_threaded_context.h"

#include <dxguids/dxguids.h>

static unsigned
num_sub_queries(unsigned query_type, unsigned index)
{
   switch (query_type) {
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      return index == 0 ? 3 : 1;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      return 4;
   default:
      return 1;
   }
}

static D3D12_QUERY_HEAP_TYPE
d3d12_query_heap_type(unsigned query_type, unsigned sub_query)
{
   switch (query_type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      return sub_query == 0 ?
         D3D12_QUERY_HEAP_TYPE_SO_STATISTICS :
         D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
   case PIPE_QUERY_SO_STATISTICS:
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      return D3D12_QUERY_HEAP_TYPE_SO_STATISTICS;
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIME_ELAPSED:
      return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

   default:
      debug_printf("unknown query: %s\n",
                   util_str_query_type(query_type, true));
      unreachable("d3d12: unknown query type");
   }
}

static D3D12_QUERY_TYPE
d3d12_query_type(unsigned query_type, unsigned sub_query, unsigned index)
{
   switch (query_type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
      return D3D12_QUERY_TYPE_OCCLUSION;
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      return D3D12_QUERY_TYPE_BINARY_OCCLUSION;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      return D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      if (sub_query > 0)
         return D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
      FALLTHROUGH;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
   case PIPE_QUERY_SO_STATISTICS:
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      return (D3D12_QUERY_TYPE)(D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0 + index);
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      return (D3D12_QUERY_TYPE)(D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0 + sub_query);
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIME_ELAPSED:
      return D3D12_QUERY_TYPE_TIMESTAMP;
   default:
      debug_printf("unknown query: %s\n",
                   util_str_query_type(query_type, true));
      unreachable("d3d12: unknown query type");
   }
}

static struct pipe_query *
d3d12_create_query(struct pipe_context *pctx,
                   unsigned query_type, unsigned index)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_screen *screen = d3d12_screen(pctx->screen);
   struct d3d12_query *query = CALLOC_STRUCT(d3d12_query);
   D3D12_QUERY_HEAP_DESC desc = {};

   if (!query)
      return NULL;

   pipe_reference_init(&query->reference, 1);
   query->type = (pipe_query_type)query_type;
   query->index = index;
   for (unsigned i = 0; i < num_sub_queries(query_type, index); ++i) {
      assert(i < MAX_SUBQUERIES);
      query->subqueries[i].d3d12qtype = d3d12_query_type(query_type, i, index);
      query->subqueries[i].num_queries = 16;

      /* With timer queries we want a few more queries, especially since we need two slots
       * per query for TIME_ELAPSED queries
       * For TIMESTAMP, we don't need more than one slot, since there's nothing to accumulate */
      if (unlikely(query_type == PIPE_QUERY_TIME_ELAPSED))
         query->subqueries[i].num_queries = 64;
      else if (query_type == PIPE_QUERY_TIMESTAMP)
         query->subqueries[i].num_queries = 1;

      query->subqueries[i].curr_query = 0;
      desc.Count = query->subqueries[i].num_queries;
      desc.Type = d3d12_query_heap_type(query_type, i);

      switch (desc.Type) {
      case D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS:
         query->subqueries[i].query_size = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);
         break;
      case D3D12_QUERY_HEAP_TYPE_SO_STATISTICS:
         query->subqueries[i].query_size = sizeof(D3D12_QUERY_DATA_SO_STATISTICS);
         break;
      default:
         query->subqueries[i].query_size = sizeof(uint64_t);
         break;
      }
      if (FAILED(screen->dev->CreateQueryHeap(&desc,
                                              IID_PPV_ARGS(&query->subqueries[i].query_heap)))) {
         FREE(query);
         return NULL;
      }

      /* Query result goes into a readback buffer */
      size_t buffer_size = query->subqueries[i].query_size * query->subqueries[i].num_queries;
      u_suballocator_alloc(&ctx->query_allocator, buffer_size, 256,
                           &query->subqueries[i].buffer_offset, &query->subqueries[i].buffer);

      query->subqueries[i].active = (query_type == PIPE_QUERY_TIMESTAMP);
   }

   return (struct pipe_query *)query;
}

void
d3d12_destroy_query(struct d3d12_query *query)
{
   pipe_resource *predicate = &query->predicate->base.b;
   pipe_resource_reference(&predicate, NULL);
   for (unsigned i = 0; i < num_sub_queries(query->type, query->index); ++i) {
      query->subqueries[i].query_heap->Release();
      pipe_resource_reference(&query->subqueries[i].buffer, NULL);
   }
   FREE(query);
}

static void
d3d12_release_query(struct pipe_context *pctx,
                    struct pipe_query *q)
{
   struct d3d12_query *query = (struct d3d12_query *)q;
   if (pipe_reference(&query->reference, nullptr)) {
      d3d12_destroy_query(query);
   }
}

static bool
accumulate_subresult_cpu(struct d3d12_context *ctx, struct d3d12_query *q_parent,
                         unsigned sub_query,
                         union pipe_query_result *result)
{
   struct pipe_transfer *transfer = NULL;
   struct d3d12_screen *screen = d3d12_screen(ctx->base.screen);
   struct d3d12_query_impl *q = &q_parent->subqueries[sub_query];
   unsigned access = PIPE_MAP_READ;
   void *results;

   access |= PIPE_MAP_UNSYNCHRONIZED;

   results = pipe_buffer_map_range(&ctx->base, q->buffer, q->buffer_offset,
                                   q->num_queries * q->query_size,
                                   access, &transfer);

   if (results == NULL)
      return false;

   uint64_t *results_u64 = (uint64_t *)results;
   D3D12_QUERY_DATA_PIPELINE_STATISTICS *results_stats = (D3D12_QUERY_DATA_PIPELINE_STATISTICS *)results;
   D3D12_QUERY_DATA_SO_STATISTICS *results_so = (D3D12_QUERY_DATA_SO_STATISTICS *)results;

   memset(result, 0, sizeof(*result));
   for (unsigned i = 0; i < q->curr_query; ++i) {
      switch (q->d3d12qtype) {
      case D3D12_QUERY_TYPE_BINARY_OCCLUSION:
         result->b |= results_u64[i] != 0;
         break;

      case D3D12_QUERY_TYPE_OCCLUSION:
         result->u64 += results_u64[i];
         break;

      case D3D12_QUERY_TYPE_TIMESTAMP:
         if (q_parent->type == PIPE_QUERY_TIME_ELAPSED)
            result->u64 += results_u64[2 * i + 1] - results_u64[2 * i];
         else
            result->u64 = results_u64[i];
         break;

      case D3D12_QUERY_TYPE_PIPELINE_STATISTICS:
         result->pipeline_statistics.ia_vertices += results_stats[i].IAVertices;
         result->pipeline_statistics.ia_primitives += results_stats[i].IAPrimitives;
         result->pipeline_statistics.vs_invocations += results_stats[i].VSInvocations;
         result->pipeline_statistics.gs_invocations += results_stats[i].GSInvocations;
         result->pipeline_statistics.gs_primitives += results_stats[i].GSPrimitives;
         result->pipeline_statistics.c_invocations += results_stats[i].CInvocations;
         result->pipeline_statistics.c_primitives += results_stats[i].CPrimitives;
         result->pipeline_statistics.ps_invocations += results_stats[i].PSInvocations;
         result->pipeline_statistics.hs_invocations += results_stats[i].HSInvocations;
         result->pipeline_statistics.ds_invocations += results_stats[i].DSInvocations;
         result->pipeline_statistics.cs_invocations += results_stats[i].CSInvocations;
         break;

      case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0:
      case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM1:
      case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM2:
      case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM3:
         if (q_parent->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE ||
             q_parent->type == PIPE_QUERY_SO_OVERFLOW_PREDICATE) {
            result->b = results_so[i].NumPrimitivesWritten != results_so[i].PrimitivesStorageNeeded;
         } else {
            result->so_statistics.num_primitives_written += results_so[i].NumPrimitivesWritten;
            result->so_statistics.primitives_storage_needed += results_so[i].PrimitivesStorageNeeded;
         }
         break;

      default:
         debug_printf("unsupported query type: %s\n",
                      util_str_query_type(q_parent->type, true));
         unreachable("unexpected query type");
      }
   }

   pipe_buffer_unmap(&ctx->base, transfer);

   if (q->d3d12qtype == D3D12_QUERY_TYPE_TIMESTAMP)
      result->u64 = static_cast<uint64_t>(screen->timestamp_multiplier * result->u64);

   return true;
}

static bool
accumulate_result_cpu(struct d3d12_context *ctx, struct d3d12_query *q,
                      union pipe_query_result *result)
{
   union pipe_query_result local_result;

   switch (q->type) {
   case PIPE_QUERY_PRIMITIVES_GENERATED:
      if (!accumulate_subresult_cpu(ctx, q, 0, &local_result))
         return false;
      result->u64 = local_result.so_statistics.primitives_storage_needed;

      if (q->index == 0) {
         if (!accumulate_subresult_cpu(ctx, q, 1, &local_result))
            return false;
         result->u64 += local_result.pipeline_statistics.gs_primitives;

         if (!accumulate_subresult_cpu(ctx, q, 2, &local_result))
            return false;
         result->u64 += local_result.pipeline_statistics.ia_primitives;
      }
      return true;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      if (!accumulate_subresult_cpu(ctx, q, 0, &local_result))
         return false;
      result->u64 = local_result.so_statistics.num_primitives_written;
      return true;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      result->b = false;
      for (uint32_t i = 0; i < num_sub_queries(q->type, q->index); ++i) {
         if (!accumulate_subresult_cpu(ctx, q, i, &local_result))
            return false;
         result->b |= local_result.b;
      }
      return true;
   default:
      assert(num_sub_queries(q->type, q->index) == 1);
      return accumulate_subresult_cpu(ctx, q, 0, result);
   }
}

static bool
subquery_should_be_active(struct d3d12_context *ctx, struct d3d12_query *q, unsigned sub_query)
{
   switch (q->type) {
   case PIPE_QUERY_PRIMITIVES_GENERATED: {
      bool has_xfb = !!ctx->gfx_pipeline_state.num_so_targets;
      struct d3d12_shader_selector *gs = ctx->gfx_stages[PIPE_SHADER_GEOMETRY];
      bool has_gs = gs && !gs->is_variant;
      switch (sub_query) {
      case 0: return has_xfb;
      case 1: return !has_xfb && has_gs;
      case 2: return !has_xfb && !has_gs;
      default: unreachable("Invalid subquery for primitives generated");
      }
      break;
   }
   default:
      return true;
   }
}

static bool 
query_ensure_ready(struct d3d12_screen* screen, struct d3d12_context* ctx, struct d3d12_query* query, bool wait)
{
   // If the query is not flushed, it won't have 
   // been submitted yet, and won't have a waitable 
   // fence value
   if (query->fence_value == UINT64_MAX) {
      d3d12_flush_cmdlist(ctx);
   }

   if (screen->fence->GetCompletedValue() < query->fence_value){
      if (!wait)
         return false;

      screen->fence->SetEventOnCompletion(query->fence_value, NULL);
   }

   return true;
}

static void
accumulate_subresult_gpu(struct d3d12_context *ctx, struct d3d12_query *q_parent,
                         unsigned sub_query)
{
   d3d12_compute_transform_save_restore save;
   d3d12_save_compute_transform_state(ctx, &save);

   d3d12_compute_transform_key key;
   memset(&key, 0, sizeof(key));
   key.type = d3d12_compute_transform_type::query_resolve;
   key.query_resolve.is_64bit = true;
   key.query_resolve.is_resolve_in_place = true;
   key.query_resolve.num_subqueries = 1;
   key.query_resolve.pipe_query_type = q_parent->type;
   key.query_resolve.single_subquery_index = sub_query;
   key.query_resolve.is_signed = false;
   key.query_resolve.timestamp_multiplier = 1.0;
   ctx->base.bind_compute_state(&ctx->base, d3d12_get_compute_transform(ctx, &key));

   ctx->transform_state_vars[0] = q_parent->subqueries[sub_query].curr_query;
   ctx->transform_state_vars[1] = 0;
   ctx->transform_state_vars[2] = 0;
   ctx->transform_state_vars[3] = 0;
   ctx->transform_state_vars[4] = 0;

   pipe_shader_buffer new_cs_ssbos[1];
   new_cs_ssbos[0].buffer = q_parent->subqueries[sub_query].buffer;
   new_cs_ssbos[0].buffer_offset = q_parent->subqueries[sub_query].buffer_offset;
   new_cs_ssbos[0].buffer_size = q_parent->subqueries[sub_query].query_size * q_parent->subqueries[sub_query].num_queries;
   ctx->base.set_shader_buffers(&ctx->base, PIPE_SHADER_COMPUTE, 0, 1, new_cs_ssbos, 1);

   pipe_grid_info grid = {};
   grid.block[0] = grid.block[1] = grid.block[2] = 1;
   grid.grid[0] = grid.grid[1] = grid.grid[2] = 1;
   ctx->base.launch_grid(&ctx->base, &grid);

   d3d12_restore_compute_transform_state(ctx, &save);
}

static void
accumulate_result_gpu(struct d3d12_context *ctx, struct d3d12_query *q,
                      struct pipe_resource *dst, uint32_t dst_offset,
                      int index, enum pipe_query_value_type result_type)
{
   d3d12_compute_transform_save_restore save;
   d3d12_save_compute_transform_state(ctx, &save);

   d3d12_compute_transform_key key;
   memset(&key, 0, sizeof(key));
   key.type = d3d12_compute_transform_type::query_resolve;
   key.query_resolve.is_64bit = result_type == PIPE_QUERY_TYPE_I64 || result_type == PIPE_QUERY_TYPE_U64;
   key.query_resolve.is_resolve_in_place = false;
   key.query_resolve.num_subqueries = num_sub_queries(q->type, q->index);
   key.query_resolve.pipe_query_type = q->type;
   key.query_resolve.single_result_field_offset = index;
   key.query_resolve.is_signed = result_type == PIPE_QUERY_TYPE_I32 || result_type == PIPE_QUERY_TYPE_I64;
   key.query_resolve.timestamp_multiplier = d3d12_screen(ctx->base.screen)->timestamp_multiplier;
   ctx->base.bind_compute_state(&ctx->base, d3d12_get_compute_transform(ctx, &key));

   pipe_shader_buffer new_cs_ssbos[5];
   uint32_t num_ssbos = 0;
   for (uint32_t i = 0; i < key.query_resolve.num_subqueries; ++i) {
      ctx->transform_state_vars[i] = q->subqueries[i].curr_query;
      new_cs_ssbos[num_ssbos].buffer = q->subqueries[i].buffer;
      new_cs_ssbos[num_ssbos].buffer_offset = q->subqueries[i].buffer_offset;
      new_cs_ssbos[num_ssbos].buffer_size = q->subqueries[i].query_size * q->subqueries[i].num_queries;
      num_ssbos++;
   }

   assert(dst_offset % (key.query_resolve.is_64bit ? 8 : 4) == 0);
   ctx->transform_state_vars[4] = dst_offset / (key.query_resolve.is_64bit ? 8 : 4);

   new_cs_ssbos[num_ssbos].buffer = dst;
   new_cs_ssbos[num_ssbos].buffer_offset = 0;
   new_cs_ssbos[num_ssbos].buffer_size = dst->width0;
   num_ssbos++;
   
   ctx->base.set_shader_buffers(&ctx->base, PIPE_SHADER_COMPUTE, 0, num_ssbos, new_cs_ssbos, 1 << (num_ssbos - 1));

   pipe_grid_info grid = {};
   grid.block[0] = grid.block[1] = grid.block[2] = 1;
   grid.grid[0] = grid.grid[1] = grid.grid[2] = 1;
   ctx->base.launch_grid(&ctx->base, &grid);

   d3d12_restore_compute_transform_state(ctx, &save);
}

static void
begin_subquery(struct d3d12_context *ctx, struct d3d12_query *q_parent, unsigned sub_query)
{
   struct d3d12_query_impl *q = &q_parent->subqueries[sub_query];
   if (q->curr_query == q->num_queries) {
      /* Accumulate current results and store in first slot */
      accumulate_subresult_gpu(ctx, q_parent, sub_query);
      q->curr_query = 1;
   }

   ctx->cmdlist->BeginQuery(q->query_heap, q->d3d12qtype, q->curr_query);
   q->active = true;
}

static void
begin_query(struct d3d12_context *ctx, struct d3d12_query *q_parent, bool restart)
{
   for (unsigned i = 0; i < num_sub_queries(q_parent->type, q_parent->index); ++i) {
      if (restart)
         q_parent->subqueries[i].curr_query = 0;

      if (!subquery_should_be_active(ctx, q_parent, i))
         continue;

      begin_subquery(ctx, q_parent, i);
   }
}


static void
begin_timer_query(struct d3d12_context *ctx, struct d3d12_query *q_parent, bool restart)
{
   struct d3d12_query_impl *q = &q_parent->subqueries[0];

   /* For PIPE_QUERY_TIME_ELAPSED we record one time with BeginQuery and one in
    * EndQuery, so we need two query slots */
   unsigned query_index = 2 * q->curr_query;

   if (restart) {
      q->curr_query = 0;
      query_index = 0;
   } else if (query_index == q->num_queries) {
      /* Accumulate current results and store in first slot */
      accumulate_subresult_gpu(ctx, q_parent, 0);
      q->curr_query = 1;
   }

   ctx->cmdlist->EndQuery(q->query_heap, q->d3d12qtype, query_index);
   q->active = true;
}

static bool
d3d12_begin_query(struct pipe_context *pctx,
                  struct pipe_query *q)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_query *query = (struct d3d12_query *)q;

   assert(query->type != PIPE_QUERY_TIMESTAMP);

   if (unlikely(query->type == PIPE_QUERY_TIME_ELAPSED))
      begin_timer_query(ctx, query, true);
   else {
      begin_query(ctx, query, true);
      list_addtail(&query->active_list, &ctx->active_queries);
   }

   return true;
}

static void
end_subquery(struct d3d12_context *ctx, struct d3d12_query *q_parent, unsigned sub_query)
{
   struct d3d12_query_impl *q = &q_parent->subqueries[sub_query];

   uint64_t offset = 0;
   struct d3d12_batch *batch = d3d12_current_batch(ctx);
   struct d3d12_resource *res = (struct d3d12_resource *)q->buffer;
   ID3D12Resource *d3d12_res = d3d12_resource_underlying(res, &offset);

   /* For TIMESTAMP, there's only one slot */
   if (q_parent->type == PIPE_QUERY_TIMESTAMP)
      q->curr_query = 0;

   /* With QUERY_TIME_ELAPSED we have recorded one value at
      * (2 * q->curr_query), and now we record a value at (2 * q->curr_query + 1)
      * and when resolving the query we subtract the latter from the former */

   unsigned resolve_count = q_parent->type == PIPE_QUERY_TIME_ELAPSED ? 2 : 1;
   unsigned resolve_index = resolve_count * q->curr_query;
   unsigned end_index = resolve_index + resolve_count - 1;

   offset += q->buffer_offset + resolve_index * q->query_size;
   ctx->cmdlist->EndQuery(q->query_heap, q->d3d12qtype, end_index);
   d3d12_transition_resource_state(ctx, res, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_TRANSITION_FLAG_INVALIDATE_BINDINGS);
   d3d12_apply_resource_states(ctx, false);
   ctx->cmdlist->ResolveQueryData(q->query_heap, q->d3d12qtype, resolve_index,
      resolve_count, d3d12_res, offset);

   d3d12_batch_reference_object(batch, q->query_heap);
   d3d12_batch_reference_resource(batch, res, true);

   assert(q->curr_query < q->num_queries);
   q->curr_query++;
   q->active = (q_parent->type == PIPE_QUERY_TIMESTAMP);
}

static void
end_query(struct d3d12_context *ctx, struct d3d12_query *q_parent)
{
   for (unsigned i = 0; i < num_sub_queries(q_parent->type, q_parent->index); ++i) {
      struct d3d12_query_impl *q = &q_parent->subqueries[i];
      if (!q->active)
         continue;

      end_subquery(ctx, q_parent, i);
   }
}

static bool
d3d12_end_query(struct pipe_context *pctx,
               struct pipe_query *q)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_query *query = (struct d3d12_query *)q;

   // Assign the sentinel and track now that the query is ended
   query->fence_value = UINT64_MAX;
   d3d12_batch_reference_query(d3d12_current_batch(ctx), query);

   end_query(ctx, query);

   if (query->type != PIPE_QUERY_TIMESTAMP &&
       query->type != PIPE_QUERY_TIME_ELAPSED)
      list_delinit(&query->active_list);
   return true;
}

static bool
d3d12_get_query_result(struct pipe_context *pctx,
                      struct pipe_query *q,
                      bool wait,
                      union pipe_query_result *result)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_screen *screen = d3d12_screen(ctx->base.screen);
   struct d3d12_query *query = (struct d3d12_query *)q;

   if (!query_ensure_ready(screen, ctx, query, wait))
      return false;

   return accumulate_result_cpu(ctx, query, result);
}

static void
d3d12_get_query_result_resource(struct pipe_context *pctx,
                                struct pipe_query *q,
                                enum pipe_query_flags flags,
                                enum pipe_query_value_type result_type,
                                int index,
                                struct pipe_resource *resource,
                                unsigned offset)
{
   struct d3d12_context *ctx = d3d12_context(pctx);

   if (index == -1) {
      /* Write the "available" bit, which is always true */
      struct d3d12_resource *res = d3d12_resource(resource);
      d3d12_transition_resource_state(ctx, res, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_TRANSITION_FLAG_NONE);
      d3d12_apply_resource_states(ctx, false);

      D3D12_GPU_VIRTUAL_ADDRESS gpuva_base = d3d12_resource_gpu_virtual_address(res) + offset;
      D3D12_WRITEBUFFERIMMEDIATE_PARAMETER params[2] = {
         { gpuva_base, 1 },
         { gpuva_base + sizeof(uint32_t), 0 },
      };
      D3D12_WRITEBUFFERIMMEDIATE_MODE modes[2] = { D3D12_WRITEBUFFERIMMEDIATE_MODE_DEFAULT, D3D12_WRITEBUFFERIMMEDIATE_MODE_DEFAULT };
      ctx->cmdlist8->WriteBufferImmediate(result_type == PIPE_QUERY_TYPE_I64 || result_type == PIPE_QUERY_TYPE_U64 ? 2 : 1,
                                          params, modes);
      return;
   }

   struct d3d12_query *query = (struct d3d12_query *)q;
   accumulate_result_gpu(ctx, query, resource, offset, index, result_type);
}

void
d3d12_suspend_queries(struct d3d12_context *ctx)
{
   list_for_each_entry(struct d3d12_query, query, &ctx->active_queries, active_list) {
      end_query(ctx, query);
   }
}

void
d3d12_resume_queries(struct d3d12_context *ctx)
{
   list_for_each_entry(struct d3d12_query, query, &ctx->active_queries, active_list) {
      begin_query(ctx, query, false);
   }
}

void
d3d12_validate_queries(struct d3d12_context *ctx)
{
   /* Nothing to do, all queries are suspended */
   if (ctx->queries_disabled)
      return;

   list_for_each_entry(struct d3d12_query, query, &ctx->active_queries, active_list) {
      for (unsigned i = 0; i < num_sub_queries(query->type, query->index); ++i) {
         if (query->subqueries[i].active && !subquery_should_be_active(ctx, query, i))
            end_subquery(ctx, query, i);
         else if (!query->subqueries[i].active && subquery_should_be_active(ctx, query, i))
            begin_subquery(ctx, query, i);
      }
   }
}

static void
d3d12_set_active_query_state(struct pipe_context *pctx, bool enable)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   ctx->queries_disabled = !enable;

   if (enable)
      d3d12_resume_queries(ctx);
   else
      d3d12_suspend_queries(ctx);
}

static void
d3d12_render_condition(struct pipe_context *pctx,
                       struct pipe_query *pquery,
                       bool condition,
                       enum pipe_render_cond_flag mode)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_query *query = (struct d3d12_query *)pquery;

   if (query == nullptr) {
      ctx->cmdlist->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
      ctx->current_predication = nullptr;
      return;
   }

   if (!query->predicate)
      query->predicate = d3d12_resource(pipe_buffer_create(pctx->screen, 0,
                                                           PIPE_USAGE_DEFAULT, sizeof(uint64_t)));

   accumulate_result_gpu(ctx, query, &query->predicate->base.b, 0, 0, PIPE_QUERY_TYPE_U64);

   d3d12_transition_resource_state(ctx, query->predicate, D3D12_RESOURCE_STATE_PREDICATION, D3D12_TRANSITION_FLAG_NONE);
   d3d12_apply_resource_states(ctx, false);

   ctx->current_predication = query->predicate;
   ctx->predication_condition = condition;
   d3d12_enable_predication(ctx);
}

void
d3d12_enable_predication(struct d3d12_context *ctx)
{
   /* documentation of ID3D12GraphicsCommandList::SetPredication method:
      * "resource manipulation commands are _not_ actually performed
      *  if the resulting predicate data of the predicate is equal to
      *  the operation specified."
      */
   ctx->cmdlist->SetPredication(d3d12_resource_resource(ctx->current_predication), 0,
                                ctx->predication_condition ? D3D12_PREDICATION_OP_NOT_EQUAL_ZERO :
                                D3D12_PREDICATION_OP_EQUAL_ZERO);
}

void
d3d12_context_query_init(struct pipe_context *pctx)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   list_inithead(&ctx->active_queries);

   u_suballocator_init(&ctx->query_allocator, &ctx->base, 4096, 0, PIPE_USAGE_STAGING,
                         0, true);

   pctx->create_query = d3d12_create_query;
   pctx->destroy_query = d3d12_release_query;
   pctx->begin_query = d3d12_begin_query;
   pctx->end_query = d3d12_end_query;
   pctx->get_query_result = d3d12_get_query_result;
   pctx->get_query_result_resource = d3d12_get_query_result_resource;
   pctx->set_active_query_state = d3d12_set_active_query_state;
   pctx->render_condition = d3d12_render_condition;
}
