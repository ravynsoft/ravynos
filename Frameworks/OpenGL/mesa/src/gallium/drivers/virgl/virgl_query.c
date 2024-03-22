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

#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "virgl_context.h"
#include "virgl_encode.h"
#include "virtio-gpu/virgl_protocol.h"
#include "virgl_resource.h"
#include "virgl_screen.h"

struct virgl_query {
   struct virgl_resource *buf;
   uint32_t handle;
   uint32_t result_size;
   uint32_t pipeline_stats;

   bool ready;
   uint64_t result;
};

#define VIRGL_QUERY_OCCLUSION_COUNTER     0
#define VIRGL_QUERY_OCCLUSION_PREDICATE   1
#define VIRGL_QUERY_TIMESTAMP             2
#define VIRGL_QUERY_TIMESTAMP_DISJOINT    3
#define VIRGL_QUERY_TIME_ELAPSED          4
#define VIRGL_QUERY_PRIMITIVES_GENERATED  5
#define VIRGL_QUERY_PRIMITIVES_EMITTED    6
#define VIRGL_QUERY_SO_STATISTICS         7
#define VIRGL_QUERY_SO_OVERFLOW_PREDICATE 8
#define VIRGL_QUERY_GPU_FINISHED          9
#define VIRGL_QUERY_PIPELINE_STATISTICS  10
#define VIRGL_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE 11
#define VIRGL_QUERY_SO_OVERFLOW_ANY_PREDICATE 12

static const int pquery_map[] =
{
   VIRGL_QUERY_OCCLUSION_COUNTER,
   VIRGL_QUERY_OCCLUSION_PREDICATE,
   VIRGL_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE,
   VIRGL_QUERY_TIMESTAMP,
   VIRGL_QUERY_TIMESTAMP_DISJOINT,
   VIRGL_QUERY_TIME_ELAPSED,
   VIRGL_QUERY_PRIMITIVES_GENERATED,
   VIRGL_QUERY_PRIMITIVES_EMITTED,
   VIRGL_QUERY_SO_STATISTICS,
   VIRGL_QUERY_SO_OVERFLOW_PREDICATE,
   VIRGL_QUERY_SO_OVERFLOW_ANY_PREDICATE,
   VIRGL_QUERY_GPU_FINISHED,
   VIRGL_QUERY_PIPELINE_STATISTICS,
};

static int pipe_to_virgl_query(enum pipe_query_type ptype)
{
   return pquery_map[ptype];
}

static const enum virgl_statistics_query_index stats_index_map[] = {
   [PIPE_STAT_QUERY_IA_VERTICES] = VIRGL_STAT_QUERY_IA_VERTICES,
   [PIPE_STAT_QUERY_IA_PRIMITIVES] = VIRGL_STAT_QUERY_IA_PRIMITIVES,
   [PIPE_STAT_QUERY_VS_INVOCATIONS] = VIRGL_STAT_QUERY_VS_INVOCATIONS,
   [PIPE_STAT_QUERY_GS_INVOCATIONS] = VIRGL_STAT_QUERY_GS_INVOCATIONS,
   [PIPE_STAT_QUERY_GS_PRIMITIVES] = VIRGL_STAT_QUERY_GS_PRIMITIVES,
   [PIPE_STAT_QUERY_C_INVOCATIONS] = VIRGL_STAT_QUERY_C_INVOCATIONS,
   [PIPE_STAT_QUERY_C_PRIMITIVES] = VIRGL_STAT_QUERY_C_PRIMITIVES,
   [PIPE_STAT_QUERY_PS_INVOCATIONS] = VIRGL_STAT_QUERY_PS_INVOCATIONS,
   [PIPE_STAT_QUERY_HS_INVOCATIONS] = VIRGL_STAT_QUERY_HS_INVOCATIONS,
   [PIPE_STAT_QUERY_DS_INVOCATIONS] = VIRGL_STAT_QUERY_DS_INVOCATIONS,
   [PIPE_STAT_QUERY_CS_INVOCATIONS] = VIRGL_STAT_QUERY_CS_INVOCATIONS,
};

static enum virgl_statistics_query_index
pipe_stats_query_to_virgl(enum pipe_statistics_query_index index)
{
   return stats_index_map[index];
}

static inline struct virgl_query *virgl_query(struct pipe_query *q)
{
   return (struct virgl_query *)q;
}

static void virgl_render_condition(struct pipe_context *ctx,
                                  struct pipe_query *q,
                                  bool condition,
                                  enum pipe_render_cond_flag mode)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_query *query = virgl_query(q);
   uint32_t handle = 0;
   if (q)
      handle = query->handle;
   virgl_encoder_render_condition(vctx, handle, condition, mode);
}

static struct pipe_query *virgl_create_query(struct pipe_context *ctx,
                                            unsigned query_type, unsigned index)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_query *query;

   query = CALLOC_STRUCT(virgl_query);
   if (!query)
      return NULL;

   query->buf = (struct virgl_resource *)
      pipe_buffer_create(ctx->screen, PIPE_BIND_CUSTOM, PIPE_USAGE_STAGING,
                         sizeof(struct virgl_host_query_state));
   if (!query->buf) {
      FREE(query);
      return NULL;
   }

   query->handle = virgl_object_assign_handle();
   query->result_size = (query_type == PIPE_QUERY_TIMESTAMP ||
                         query_type == PIPE_QUERY_TIME_ELAPSED) ? 8 : 4;

   if (query_type == PIPE_QUERY_PIPELINE_STATISTICS) {
      query->pipeline_stats = index;

      index = pipe_stats_query_to_virgl(index);
   } else {
      query->pipeline_stats = ~0;
   }

   util_range_add(&query->buf->b, &query->buf->valid_buffer_range, 0,
                  sizeof(struct virgl_host_query_state));
   virgl_resource_dirty(query->buf, 0);

   virgl_encoder_create_query(vctx, query->handle,
         pipe_to_virgl_query(query_type), index, query->buf, 0);

   return (struct pipe_query *)query;
}

static void virgl_destroy_query(struct pipe_context *ctx,
                        struct pipe_query *q)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_query *query = virgl_query(q);

   virgl_encode_delete_object(vctx, query->handle, VIRGL_OBJECT_QUERY);

   pipe_resource_reference((struct pipe_resource **)&query->buf, NULL);
   FREE(query);
}

static bool virgl_begin_query(struct pipe_context *ctx,
                             struct pipe_query *q)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_query *query = virgl_query(q);

   virgl_encoder_begin_query(vctx, query->handle);

   return true;
}

static bool virgl_end_query(struct pipe_context *ctx,
                           struct pipe_query *q)
{
   struct virgl_screen *vs = virgl_screen(ctx->screen);
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_query *query = virgl_query(q);
   struct virgl_host_query_state *host_state;

   host_state = vs->vws->resource_map(vs->vws, query->buf->hw_res);
   if (!host_state)
      return false;

   host_state->query_state = VIRGL_QUERY_STATE_WAIT_HOST;
   query->ready = false;

   virgl_encoder_end_query(vctx, query->handle);

   /* start polling now */
   virgl_encoder_get_query_result(vctx, query->handle, 0);
   vs->vws->emit_res(vs->vws, vctx->cbuf, query->buf->hw_res, false);

   return true;
}

static bool virgl_get_query_result(struct pipe_context *ctx,
                                   struct pipe_query *q,
                                   bool wait,
                                   union pipe_query_result *result)
{
   struct virgl_query *query = virgl_query(q);

   if (!query->ready) {
      struct virgl_screen *vs = virgl_screen(ctx->screen);
      struct virgl_context *vctx = virgl_context(ctx);
      volatile struct virgl_host_query_state *host_state;
      struct pipe_transfer *transfer = NULL;

      if (vs->vws->res_is_referenced(vs->vws, vctx->cbuf, query->buf->hw_res))
         ctx->flush(ctx, NULL, 0);

      if (wait)
         vs->vws->resource_wait(vs->vws, query->buf->hw_res);
      else if (vs->vws->resource_is_busy(vs->vws, query->buf->hw_res))
         return false;

      host_state = vs->vws->resource_map(vs->vws, query->buf->hw_res);

      /* The resource is idle and the result should be available at this point,
       * unless we are dealing with an older host.  In that case,
       * VIRGL_CCMD_GET_QUERY_RESULT is not fenced, the buffer is not
       * coherent, and transfers are unsynchronized.  We have to repeatedly
       * transfer until we get the result back.
       */
      while (host_state->query_state != VIRGL_QUERY_STATE_DONE) {
         debug_printf("VIRGL: get_query_result is forced blocking\n");

         if (transfer) {
            pipe_buffer_unmap(ctx, transfer);
            if (!wait)
               return false;
         }

         host_state = pipe_buffer_map(ctx, &query->buf->b,
               PIPE_MAP_READ, &transfer);
      }

      if (query->result_size == 8)
         query->result = host_state->result;
      else
         query->result = (uint32_t) host_state->result;

      if (transfer)
         pipe_buffer_unmap(ctx, transfer);

      query->ready = true;
   }

   switch (query->pipeline_stats) {
   case PIPE_STAT_QUERY_IA_VERTICES: result->pipeline_statistics.ia_vertices = query->result; break;
   case PIPE_STAT_QUERY_IA_PRIMITIVES: result->pipeline_statistics.ia_primitives = query->result; break;
   case PIPE_STAT_QUERY_VS_INVOCATIONS: result->pipeline_statistics.vs_invocations = query->result; break;
   case PIPE_STAT_QUERY_GS_INVOCATIONS: result->pipeline_statistics.gs_invocations = query->result; break;
   case PIPE_STAT_QUERY_GS_PRIMITIVES: result->pipeline_statistics.gs_primitives = query->result; break;
   case PIPE_STAT_QUERY_PS_INVOCATIONS: result->pipeline_statistics.ps_invocations = query->result; break;
   case PIPE_STAT_QUERY_HS_INVOCATIONS: result->pipeline_statistics.hs_invocations = query->result; break;
   case PIPE_STAT_QUERY_CS_INVOCATIONS: result->pipeline_statistics.cs_invocations = query->result; break;
   case PIPE_STAT_QUERY_C_INVOCATIONS: result->pipeline_statistics.c_invocations = query->result; break;
   case PIPE_STAT_QUERY_C_PRIMITIVES: result->pipeline_statistics.c_primitives = query->result; break;
   case PIPE_STAT_QUERY_DS_INVOCATIONS: result->pipeline_statistics.ds_invocations = query->result; break;
   default:
      result->u64 = query->result;
   }

   return true;
}

static void
virgl_set_active_query_state(struct pipe_context *pipe, bool enable)
{
}

static void
virgl_get_query_result_resource(struct pipe_context *ctx,
                                struct pipe_query *q,
                                enum pipe_query_flags flags,
                                enum pipe_query_value_type result_type,
                                int index,
                                struct pipe_resource *resource,
                                unsigned offset)
{
   struct virgl_context *vctx = virgl_context(ctx);
   struct virgl_query *query = virgl_query(q);
   struct virgl_resource *qbo = (struct virgl_resource *)resource;

   virgl_resource_dirty(qbo, 0);
   virgl_encode_get_query_result_qbo(vctx, query->handle, qbo, (flags & PIPE_QUERY_WAIT), result_type, offset, index);
}

void virgl_init_query_functions(struct virgl_context *vctx)
{
   vctx->base.render_condition = virgl_render_condition;
   vctx->base.create_query = virgl_create_query;
   vctx->base.destroy_query = virgl_destroy_query;
   vctx->base.begin_query = virgl_begin_query;
   vctx->base.end_query = virgl_end_query;
   vctx->base.get_query_result = virgl_get_query_result;
   vctx->base.set_active_query_state = virgl_set_active_query_state;
   vctx->base.get_query_result_resource = virgl_get_query_result_resource;
}
