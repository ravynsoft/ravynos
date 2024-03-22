/*
 * Copyright © 2017 Intel Corporation
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

/**
 * @file crocus_query.c
 *
 * ============================= GENXML CODE =============================
 *              [This file is compiled once per generation.]
 * =======================================================================
 *
 * Query object support.  This allows measuring various simple statistics
 * via counters on the GPU.  We use GenX code for MI_MATH calculations.
 */

#include <stdio.h>
#include <errno.h>
#include "perf/intel_perf.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/u_inlines.h"
#include "util/u_upload_mgr.h"
#include "crocus_context.h"
#include "crocus_defines.h"
#include "crocus_fence.h"
#include "crocus_monitor.h"
#include "crocus_resource.h"
#include "crocus_screen.h"

#include "crocus_genx_macros.h"

#if GFX_VER == 6
// TOOD: Add these to genxml?
#define SO_PRIM_STORAGE_NEEDED(n) (0x2280)
#define SO_NUM_PRIMS_WRITTEN(n)   (0x2288)

// TODO: remove HS/DS/CS
#define GFX6_IA_VERTICES_COUNT_num          0x2310
#define GFX6_IA_PRIMITIVES_COUNT_num        0x2318
#define GFX6_VS_INVOCATION_COUNT_num        0x2320
#define GFX6_HS_INVOCATION_COUNT_num        0x2300
#define GFX6_DS_INVOCATION_COUNT_num        0x2308
#define GFX6_GS_INVOCATION_COUNT_num        0x2328
#define GFX6_GS_PRIMITIVES_COUNT_num        0x2330
#define GFX6_CL_INVOCATION_COUNT_num        0x2338
#define GFX6_CL_PRIMITIVES_COUNT_num        0x2340
#define GFX6_PS_INVOCATION_COUNT_num        0x2348
#define GFX6_CS_INVOCATION_COUNT_num        0x2290
#define GFX6_PS_DEPTH_COUNT_num             0x2350

#elif GFX_VER >= 7
#define SO_PRIM_STORAGE_NEEDED(n) (GENX(SO_PRIM_STORAGE_NEEDED0_num) + (n) * 8)
#define SO_NUM_PRIMS_WRITTEN(n)   (GENX(SO_NUM_PRIMS_WRITTEN0_num) + (n) * 8)
#endif

struct crocus_query {
   struct threaded_query b;

   enum pipe_query_type type;
   int index;

   bool ready;

   bool stalled;

   uint64_t result;

   struct crocus_state_ref query_state_ref;
   struct crocus_query_snapshots *map;
   struct crocus_syncobj *syncobj;

   int batch_idx;

   struct crocus_monitor_object *monitor;

   /* Fence for PIPE_QUERY_GPU_FINISHED. */
   struct pipe_fence_handle *fence;
};

struct crocus_query_snapshots {
   /** crocus_render_condition's saved MI_PREDICATE_RESULT value. */
   uint64_t predicate_result;

   /** Have the start/end snapshots landed? */
   uint64_t snapshots_landed;

   /** Starting and ending counter snapshots */
   uint64_t start;
   uint64_t end;
};

struct crocus_query_so_overflow {
   uint64_t predicate_result;
   uint64_t snapshots_landed;

   struct {
      uint64_t prim_storage_needed[2];
      uint64_t num_prims[2];
   } stream[4];
};

#if GFX_VERx10 >= 75
static struct mi_value
query_mem64(struct crocus_query *q, uint32_t offset)
{
   return mi_mem64(rw_bo(crocus_resource_bo(q->query_state_ref.res),
                         q->query_state_ref.offset + offset));
}
#endif

/**
 * Is this type of query written by PIPE_CONTROL?
 */
static bool
crocus_is_query_pipelined(struct crocus_query *q)
{
   switch (q->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIMESTAMP_DISJOINT:
   case PIPE_QUERY_TIME_ELAPSED:
      return true;

   default:
      return false;
   }
}

static void
mark_available(struct crocus_context *ice, struct crocus_query *q)
{
#if GFX_VERx10 >= 75
   struct crocus_batch *batch = &ice->batches[q->batch_idx];
   struct crocus_screen *screen = batch->screen;
   unsigned flags = PIPE_CONTROL_WRITE_IMMEDIATE;
   unsigned offset = offsetof(struct crocus_query_snapshots, snapshots_landed);
   struct crocus_bo *bo = crocus_resource_bo(q->query_state_ref.res);
   offset += q->query_state_ref.offset;

   if (!crocus_is_query_pipelined(q)) {
      screen->vtbl.store_data_imm64(batch, bo, offset, true);
   } else {
      /* Order available *after* the query results. */
      flags |= PIPE_CONTROL_FLUSH_ENABLE;
      crocus_emit_pipe_control_write(batch, "query: mark available",
                                     flags, bo, offset, true);
   }
#endif
}

/**
 * Write PS_DEPTH_COUNT to q->(dest) via a PIPE_CONTROL.
 */
static void
crocus_pipelined_write(struct crocus_batch *batch,
                       struct crocus_query *q,
                       enum pipe_control_flags flags,
                       unsigned offset)
{
   struct crocus_bo *bo = crocus_resource_bo(q->query_state_ref.res);

   crocus_emit_pipe_control_write(batch, "query: pipelined snapshot write",
                                  flags,
                                  bo, offset, 0ull);
}

static void
write_value(struct crocus_context *ice, struct crocus_query *q, unsigned offset)
{
   struct crocus_batch *batch = &ice->batches[q->batch_idx];
#if GFX_VER >= 6
   struct crocus_screen *screen = batch->screen;
   struct crocus_bo *bo = crocus_resource_bo(q->query_state_ref.res);
#endif

   if (!crocus_is_query_pipelined(q)) {
      crocus_emit_pipe_control_flush(batch,
                                     "query: non-pipelined snapshot write",
                                     PIPE_CONTROL_CS_STALL |
                                     PIPE_CONTROL_STALL_AT_SCOREBOARD);
      q->stalled = true;
   }

   switch (q->type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      crocus_pipelined_write(&ice->batches[CROCUS_BATCH_RENDER], q,
                             PIPE_CONTROL_WRITE_DEPTH_COUNT |
                             PIPE_CONTROL_DEPTH_STALL,
                             offset);
      break;
   case PIPE_QUERY_TIME_ELAPSED:
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIMESTAMP_DISJOINT:
      crocus_pipelined_write(&ice->batches[CROCUS_BATCH_RENDER], q,
                             PIPE_CONTROL_WRITE_TIMESTAMP,
                             offset);
      break;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
#if GFX_VER >= 6
      screen->vtbl.store_register_mem64(batch,
                                        q->index == 0 ?
                                        GENX(CL_INVOCATION_COUNT_num) :
                                        SO_PRIM_STORAGE_NEEDED(q->index),
                                        bo, offset, false);
#endif
      break;
   case PIPE_QUERY_PRIMITIVES_EMITTED:
#if GFX_VER >= 6
      screen->vtbl.store_register_mem64(batch,
                                        SO_NUM_PRIMS_WRITTEN(q->index),
                                        bo, offset, false);
#endif
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS_SINGLE: {
#if GFX_VER >= 6
      static const uint32_t index_to_reg[] = {
         GENX(IA_VERTICES_COUNT_num),
         GENX(IA_PRIMITIVES_COUNT_num),
         GENX(VS_INVOCATION_COUNT_num),
         GENX(GS_INVOCATION_COUNT_num),
         GENX(GS_PRIMITIVES_COUNT_num),
         GENX(CL_INVOCATION_COUNT_num),
         GENX(CL_PRIMITIVES_COUNT_num),
         GENX(PS_INVOCATION_COUNT_num),
         GENX(HS_INVOCATION_COUNT_num),
         GENX(DS_INVOCATION_COUNT_num),
         GENX(CS_INVOCATION_COUNT_num),
      };
      uint32_t reg = index_to_reg[q->index];

#if GFX_VER == 6
      /* Gfx6 GS code counts full primitives, that is, it won't count individual
       * triangles in a triangle strip. Use CL_INVOCATION_COUNT for that.
       */
      if (q->index == PIPE_STAT_QUERY_GS_PRIMITIVES)
         reg = GENX(CL_INVOCATION_COUNT_num);
#endif

      screen->vtbl.store_register_mem64(batch, reg, bo, offset, false);
#endif
      break;
   }
   default:
      assert(false);
   }
}

#if GFX_VER >= 6
static void
write_overflow_values(struct crocus_context *ice, struct crocus_query *q, bool end)
{
   struct crocus_batch *batch = &ice->batches[CROCUS_BATCH_RENDER];
   struct crocus_screen *screen = batch->screen;
   uint32_t count = q->type == PIPE_QUERY_SO_OVERFLOW_PREDICATE ? 1 : 4;
   struct crocus_bo *bo = crocus_resource_bo(q->query_state_ref.res);
   uint32_t offset = q->query_state_ref.offset;
   crocus_emit_pipe_control_flush(batch,
                                  "query: write SO overflow snapshots",
                                  PIPE_CONTROL_CS_STALL |
                                  PIPE_CONTROL_STALL_AT_SCOREBOARD);
   for (uint32_t i = 0; i < count; i++) {
      int s = q->index + i;
      int g_idx = offset + offsetof(struct crocus_query_so_overflow,
                                    stream[s].num_prims[end]);
      int w_idx = offset + offsetof(struct crocus_query_so_overflow,
                                    stream[s].prim_storage_needed[end]);
      screen->vtbl.store_register_mem64(batch, SO_NUM_PRIMS_WRITTEN(s),
                                        bo, g_idx, false);
      screen->vtbl.store_register_mem64(batch, SO_PRIM_STORAGE_NEEDED(s),
                                        bo, w_idx, false);
   }
}
#endif
static uint64_t
crocus_raw_timestamp_delta(uint64_t time0, uint64_t time1)
{
   if (time0 > time1) {
      return (1ULL << TIMESTAMP_BITS) + time1 - time0;
   } else {
      return time1 - time0;
   }
}

static bool
stream_overflowed(struct crocus_query_so_overflow *so, int s)
{
   return (so->stream[s].prim_storage_needed[1] -
           so->stream[s].prim_storage_needed[0]) !=
          (so->stream[s].num_prims[1] - so->stream[s].num_prims[0]);
}

static void
calculate_result_on_cpu(const struct intel_device_info *devinfo,
                        struct crocus_query *q)
{
   switch (q->type) {
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      q->result = q->map->end != q->map->start;
      break;
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIMESTAMP_DISJOINT:
      /* The timestamp is the single starting snapshot. */
      q->result = intel_device_info_timebase_scale(devinfo, q->map->start);
      q->result &= (1ull << TIMESTAMP_BITS) - 1;
      break;
   case PIPE_QUERY_TIME_ELAPSED:
      q->result = crocus_raw_timestamp_delta(q->map->start, q->map->end);
      q->result = intel_device_info_timebase_scale(devinfo, q->result);
      q->result &= (1ull << TIMESTAMP_BITS) - 1;
      break;
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      q->result = stream_overflowed((void *) q->map, q->index);
      break;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      q->result = false;
      for (int i = 0; i < PIPE_MAX_VERTEX_STREAMS; i++)
         q->result |= stream_overflowed((void *) q->map, i);
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS_SINGLE:
      q->result = q->map->end - q->map->start;

      /* WaDividePSInvocationCountBy4:HSW,BDW */
      if (GFX_VERx10 >= 75 && q->index == PIPE_STAT_QUERY_PS_INVOCATIONS)
         q->result /= 4;
      break;
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
   default:
      q->result = q->map->end - q->map->start;
      break;
   }

   q->ready = true;
}

#if GFX_VERx10 >= 75
/**
 * Calculate the streamout overflow for stream \p idx:
 *
 * (num_prims[1] - num_prims[0]) - (storage_needed[1] - storage_needed[0])
 */
static struct mi_value
calc_overflow_for_stream(struct mi_builder *b,
                         struct crocus_query *q,
                         int idx)
{
#define C(counter, i) query_mem64(q, \
   offsetof(struct crocus_query_so_overflow, stream[idx].counter[i]))

   return mi_isub(b, mi_isub(b, C(num_prims, 1), C(num_prims, 0)),
                  mi_isub(b, C(prim_storage_needed, 1),
                          C(prim_storage_needed, 0)));
#undef C
}

/**
 * Calculate whether any stream has overflowed.
 */
static struct mi_value
calc_overflow_any_stream(struct mi_builder *b, struct crocus_query *q)
{
   struct mi_value stream_result[PIPE_MAX_VERTEX_STREAMS];
   for (int i = 0; i < PIPE_MAX_VERTEX_STREAMS; i++)
      stream_result[i] = calc_overflow_for_stream(b, q, i);

   struct mi_value result = stream_result[0];
   for (int i = 1; i < PIPE_MAX_VERTEX_STREAMS; i++)
      result = mi_ior(b, result, stream_result[i]);

   return result;
}


static bool
query_is_boolean(enum pipe_query_type type)
{
   switch (type) {
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      return true;
   default:
      return false;
   }
}

/**
 * Calculate the result using MI_MATH.
 */
static struct mi_value
calculate_result_on_gpu(const struct intel_device_info *devinfo,
                        struct mi_builder *b,
                        struct crocus_query *q)
{
   struct mi_value result;
   struct mi_value start_val =
      query_mem64(q, offsetof(struct crocus_query_snapshots, start));
   struct mi_value end_val =
      query_mem64(q, offsetof(struct crocus_query_snapshots, end));

   switch (q->type) {
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      result = calc_overflow_for_stream(b, q, q->index);
      break;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      result = calc_overflow_any_stream(b, q);
      break;
   case PIPE_QUERY_TIMESTAMP: {
      /* TODO: This discards any fractional bits of the timebase scale.
       * We would need to do a bit of fixed point math on the CS ALU, or
       * launch an actual shader to calculate this with full precision.
       */
      uint32_t scale = 1000000000ull / devinfo->timestamp_frequency;
      result = mi_iand(b, mi_imm((1ull << 36) - 1),
                       mi_imul_imm(b, start_val, scale));
      break;
   }
   case PIPE_QUERY_TIME_ELAPSED: {
      /* TODO: This discards fractional bits (see above). */
      uint32_t scale = 1000000000ull / devinfo->timestamp_frequency;
      result = mi_imul_imm(b, mi_isub(b, end_val, start_val), scale);
      break;
   }
   default:
      result = mi_isub(b, end_val, start_val);
      break;
   }
   /* WaDividePSInvocationCountBy4:HSW,BDW */
   if (GFX_VERx10 >= 75 &&
       q->type == PIPE_QUERY_PIPELINE_STATISTICS_SINGLE &&
       q->index == PIPE_STAT_QUERY_PS_INVOCATIONS)
      result = mi_ushr32_imm(b, result, 2);

   if (query_is_boolean(q->type))
      result = mi_iand(b, mi_nz(b, result), mi_imm(1));

   return result;
}
#endif

static struct pipe_query *
crocus_create_query(struct pipe_context *ctx,
                    unsigned query_type,
                    unsigned index)
{
   struct crocus_query *q = calloc(1, sizeof(struct crocus_query));

   q->type = query_type;
   q->index = index;
   q->monitor = NULL;

   if (q->type == PIPE_QUERY_PIPELINE_STATISTICS_SINGLE &&
       q->index == PIPE_STAT_QUERY_CS_INVOCATIONS)
      q->batch_idx = CROCUS_BATCH_COMPUTE;
   else
      q->batch_idx = CROCUS_BATCH_RENDER;
   return (struct pipe_query *) q;
}

static struct pipe_query *
crocus_create_batch_query(struct pipe_context *ctx,
                          unsigned num_queries,
                          unsigned *query_types)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_query *q = calloc(1, sizeof(struct crocus_query));
   if (unlikely(!q))
      return NULL;
   q->type = PIPE_QUERY_DRIVER_SPECIFIC;
   q->index = -1;
   q->monitor = crocus_create_monitor_object(ice, num_queries, query_types);
   if (unlikely(!q->monitor)) {
      free(q);
      return NULL;
   }

   return (struct pipe_query *) q;
}

static void
crocus_destroy_query(struct pipe_context *ctx, struct pipe_query *p_query)
{
   struct crocus_query *query = (void *) p_query;
   struct crocus_screen *screen = (void *) ctx->screen;
   if (query->monitor) {
      crocus_destroy_monitor_object(ctx, query->monitor);
      query->monitor = NULL;
   } else {
      crocus_syncobj_reference(screen, &query->syncobj, NULL);
      screen->base.fence_reference(ctx->screen, &query->fence, NULL);
   }
   pipe_resource_reference(&query->query_state_ref.res, NULL);
   free(query);
}


static bool
crocus_begin_query(struct pipe_context *ctx, struct pipe_query *query)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_query *q = (void *) query;

   if (q->monitor)
      return crocus_begin_monitor(ctx, q->monitor);

   void *ptr = NULL;
   uint32_t size;

   if (q->type == PIPE_QUERY_SO_OVERFLOW_PREDICATE ||
       q->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE)
      size = sizeof(struct crocus_query_so_overflow);
   else
      size = sizeof(struct crocus_query_snapshots);

   u_upload_alloc(ice->query_buffer_uploader, 0,
                  size, util_next_power_of_two(size), &q->query_state_ref.offset,
                  &q->query_state_ref.res, &ptr);

   if (!q->query_state_ref.res)
      return false;
   if (!crocus_resource_bo(q->query_state_ref.res))
      return false;

   q->map = ptr;
   if (!q->map)
      return false;

   q->result = 0ull;
   q->ready = false;
   WRITE_ONCE(q->map->snapshots_landed, false);

   if (q->type == PIPE_QUERY_PRIMITIVES_GENERATED && q->index == 0) {
      ice->state.prims_generated_query_active = true;
      ice->state.dirty |= CROCUS_DIRTY_STREAMOUT | CROCUS_DIRTY_CLIP;
   }

#if GFX_VER <= 5
   if (q->type == PIPE_QUERY_OCCLUSION_COUNTER ||
       q->type == PIPE_QUERY_OCCLUSION_PREDICATE) {
      ice->state.stats_wm++;
      ice->state.dirty |= CROCUS_DIRTY_WM | CROCUS_DIRTY_COLOR_CALC_STATE;
   }
#endif
#if GFX_VER >= 6
   if (q->type == PIPE_QUERY_SO_OVERFLOW_PREDICATE ||
       q->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE)
      write_overflow_values(ice, q, false);
   else
#endif
      write_value(ice, q,
                  q->query_state_ref.offset +
                  offsetof(struct crocus_query_snapshots, start));

   return true;
}

static bool
crocus_end_query(struct pipe_context *ctx, struct pipe_query *query)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_query *q = (void *) query;

   if (q->monitor)
      return crocus_end_monitor(ctx, q->monitor);

   if (q->type == PIPE_QUERY_GPU_FINISHED) {
      ctx->flush(ctx, &q->fence, PIPE_FLUSH_DEFERRED);
      return true;
   }

   struct crocus_batch *batch = &ice->batches[q->batch_idx];

   if (q->type == PIPE_QUERY_TIMESTAMP) {
      crocus_begin_query(ctx, query);
      crocus_batch_reference_signal_syncobj(batch, &q->syncobj);
      mark_available(ice, q);
      return true;
   }

#if GFX_VER <= 5
   if (q->type == PIPE_QUERY_OCCLUSION_COUNTER ||
       q->type == PIPE_QUERY_OCCLUSION_PREDICATE) {
      ice->state.stats_wm--;
      ice->state.dirty |= CROCUS_DIRTY_WM | CROCUS_DIRTY_COLOR_CALC_STATE;
   }
#endif
   if (q->type == PIPE_QUERY_PRIMITIVES_GENERATED && q->index == 0) {
      ice->state.prims_generated_query_active = false;
      ice->state.dirty |= CROCUS_DIRTY_STREAMOUT | CROCUS_DIRTY_CLIP;
   }

#if GFX_VER >= 6
   if (q->type == PIPE_QUERY_SO_OVERFLOW_PREDICATE ||
       q->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE)
      write_overflow_values(ice, q, true);
   else
#endif
      write_value(ice, q,
                  q->query_state_ref.offset +
                  offsetof(struct crocus_query_snapshots, end));

   crocus_batch_reference_signal_syncobj(batch, &q->syncobj);
   mark_available(ice, q);

   return true;
}

/**
 * See if the snapshots have landed for a query, and if so, compute the
 * result and mark it ready.  Does not flush (unlike crocus_get_query_result).
 */
static void
crocus_check_query_no_flush(struct crocus_context *ice, struct crocus_query *q)
{
   struct crocus_screen *screen = (void *) ice->ctx.screen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   if (!q->ready && READ_ONCE(q->map->snapshots_landed)) {
      calculate_result_on_cpu(devinfo, q);
   }
}

static bool
crocus_get_query_result(struct pipe_context *ctx,
                        struct pipe_query *query,
                        bool wait,
                        union pipe_query_result *result)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_query *q = (void *) query;

   if (q->monitor)
      return crocus_get_monitor_result(ctx, q->monitor, wait, result->batch);

   struct crocus_screen *screen = (void *) ctx->screen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   if (unlikely(screen->devinfo.no_hw)) {
      result->u64 = 0;
      return true;
   }

   if (!q->ready) {
      struct crocus_batch *batch = &ice->batches[q->batch_idx];
      if (q->syncobj == crocus_batch_get_signal_syncobj(batch))
         crocus_batch_flush(batch);

#if GFX_VERx10 >= 75
      while (!READ_ONCE(q->map->snapshots_landed)) {
         if (wait)
            crocus_wait_syncobj(ctx->screen, q->syncobj, INT64_MAX);
         else
            return false;
      }
      assert(READ_ONCE(q->map->snapshots_landed));
#else
      if (crocus_wait_syncobj(ctx->screen, q->syncobj, wait ? INT64_MAX : 0)) {
         /* if we've waited and timedout, just set the query to ready to avoid infinite loop */
         if (wait)
            q->ready = true;
         return false;
      }
#endif
      calculate_result_on_cpu(devinfo, q);
   }

   assert(q->ready);

   result->u64 = q->result;

   return true;
}

#if GFX_VER >= 7
static void
crocus_get_query_result_resource(struct pipe_context *ctx,
                                 struct pipe_query *query,
                                 enum pipe_query_flags flags,
                                 enum pipe_query_value_type result_type,
                                 int index,
                                 struct pipe_resource *p_res,
                                 unsigned offset)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_query *q = (void *) query;
   struct crocus_batch *batch = &ice->batches[q->batch_idx];
   struct crocus_screen *screen = batch->screen;
   const struct intel_device_info *devinfo = &batch->screen->devinfo;
   struct crocus_resource *res = (void *) p_res;
   struct crocus_bo *query_bo = crocus_resource_bo(q->query_state_ref.res);
   struct crocus_bo *dst_bo = crocus_resource_bo(p_res);
   unsigned snapshots_landed_offset =
      offsetof(struct crocus_query_snapshots, snapshots_landed);

   res->bind_history |= PIPE_BIND_QUERY_BUFFER;

   if (index == -1) {
      /* They're asking for the availability of the result.  If we still
       * have commands queued up which produce the result, submit them
       * now so that progress happens.  Either way, copy the snapshots
       * landed field to the destination resource.
       */
      if (q->syncobj == crocus_batch_get_signal_syncobj(batch))
         crocus_batch_flush(batch);

      screen->vtbl.copy_mem_mem(batch, dst_bo, offset,
                                query_bo, snapshots_landed_offset,
                                result_type <= PIPE_QUERY_TYPE_U32 ? 4 : 8);
      return;
   }

   if (!q->ready && READ_ONCE(q->map->snapshots_landed)) {
      /* The final snapshots happen to have landed, so let's just compute
       * the result on the CPU now...
       */
      calculate_result_on_cpu(devinfo, q);
   }

   if (q->ready) {
      /* We happen to have the result on the CPU, so just copy it. */
      if (result_type <= PIPE_QUERY_TYPE_U32) {
         screen->vtbl.store_data_imm32(batch, dst_bo, offset, q->result);
      } else {
         screen->vtbl.store_data_imm64(batch, dst_bo, offset, q->result);
      }

      /* Make sure the result lands before they use bind the QBO elsewhere
       * and use the result.
       */
      // XXX: Why?  i965 doesn't do this.
      crocus_emit_pipe_control_flush(batch,
                                     "query: unknown QBO flushing hack",
                                     PIPE_CONTROL_CS_STALL);
      return;
   }

#if GFX_VERx10 >= 75
   bool predicated = !(flags & PIPE_QUERY_WAIT) && !q->stalled;

   struct mi_builder b;
   mi_builder_init(&b, &batch->screen->devinfo, batch);

   struct mi_value result = calculate_result_on_gpu(devinfo, &b, q);
   struct mi_value dst =
      result_type <= PIPE_QUERY_TYPE_U32 ? mi_mem32(rw_bo(dst_bo, offset))
                                         : mi_mem64(rw_bo(dst_bo, offset));

   if (predicated) {
      mi_store(&b, mi_reg32(MI_PREDICATE_RESULT),
                   mi_mem64(ro_bo(query_bo, snapshots_landed_offset)));
      mi_store_if(&b, dst, result);
   } else {
      mi_store(&b, dst, result);
   }
#endif
}
#endif

static void
crocus_set_active_query_state(struct pipe_context *ctx, bool enable)
{
   struct crocus_context *ice = (void *) ctx;

   if (ice->state.statistics_counters_enabled == enable)
      return;

   // XXX: most packets aren't paying attention to this yet, because it'd
   // have to be done dynamically at draw time, which is a pain
   ice->state.statistics_counters_enabled = enable;
   ice->state.dirty |= CROCUS_DIRTY_CLIP |
                       CROCUS_DIRTY_RASTER |
                       CROCUS_DIRTY_STREAMOUT |
                       CROCUS_DIRTY_WM;
   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_GS |
                             CROCUS_STAGE_DIRTY_TCS |
                             CROCUS_STAGE_DIRTY_TES |
                             CROCUS_STAGE_DIRTY_VS;
}

static void
set_predicate_enable(struct crocus_context *ice, bool value)
{
   if (value)
      ice->state.predicate = CROCUS_PREDICATE_STATE_RENDER;
   else
      ice->state.predicate = CROCUS_PREDICATE_STATE_DONT_RENDER;
}

#if GFX_VER >= 7
static void
set_predicate_for_result(struct crocus_context *ice,
                         struct crocus_query *q,
                         bool inverted)
{
   struct crocus_batch *batch = &ice->batches[CROCUS_BATCH_RENDER];
   struct crocus_bo *bo = crocus_resource_bo(q->query_state_ref.res);

#if GFX_VERx10 < 75
   /* IVB doesn't have enough MI for this */
   if (q->type == PIPE_QUERY_SO_OVERFLOW_PREDICATE ||
       q->type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE) {
      ice->state.predicate = CROCUS_PREDICATE_STATE_STALL_FOR_QUERY;
      return;
   }
#endif

   /* The CPU doesn't have the query result yet; use hardware predication */
   ice->state.predicate = CROCUS_PREDICATE_STATE_USE_BIT;

   /* Ensure the memory is coherent for MI_LOAD_REGISTER_* commands. */
   crocus_emit_pipe_control_flush(batch,
                                  "conditional rendering: set predicate",
                                  PIPE_CONTROL_FLUSH_ENABLE);
   q->stalled = true;

#if GFX_VERx10 < 75
   struct crocus_screen *screen = batch->screen;
   screen->vtbl.load_register_mem64(batch, MI_PREDICATE_SRC0, bo,
                                    q->query_state_ref.offset + offsetof(struct crocus_query_snapshots, start));
   screen->vtbl.load_register_mem64(batch, MI_PREDICATE_SRC1, bo,
                                    q->query_state_ref.offset + offsetof(struct crocus_query_snapshots, end));

   uint32_t mi_predicate = MI_PREDICATE | MI_PREDICATE_COMBINEOP_SET |
      MI_PREDICATE_COMPAREOP_SRCS_EQUAL;
   if (inverted)
      mi_predicate |= MI_PREDICATE_LOADOP_LOAD;
   else
      mi_predicate |= MI_PREDICATE_LOADOP_LOADINV;
   crocus_batch_emit(batch, &mi_predicate, sizeof(uint32_t));
#else
   struct mi_builder b;
   mi_builder_init(&b, &batch->screen->devinfo, batch);

   struct mi_value result;

   switch (q->type) {
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      result = calc_overflow_for_stream(&b, q, q->index);
      break;
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
      result = calc_overflow_any_stream(&b, q);
      break;
   default: {
      /* PIPE_QUERY_OCCLUSION_* */
      struct mi_value start =
         query_mem64(q, offsetof(struct crocus_query_snapshots, start));
      struct mi_value end =
         query_mem64(q, offsetof(struct crocus_query_snapshots, end));
      result = mi_isub(&b, end, start);
      break;
   }
   }

   result = inverted ? mi_z(&b, result) : mi_nz(&b, result);
   result = mi_iand(&b, result, mi_imm(1));

   /* We immediately set the predicate on the render batch, as all the
    * counters come from 3D operations.  However, we may need to predicate
    * a compute dispatch, which executes in a different GEM context and has
    * a different MI_PREDICATE_RESULT register.  So, we save the result to
    * memory and reload it in crocus_launch_grid.
    */
   mi_value_ref(&b, result);

   mi_store(&b, mi_reg64(MI_PREDICATE_SRC0), result);
   mi_store(&b, mi_reg64(MI_PREDICATE_SRC1), mi_imm(0));

   unsigned mi_predicate = MI_PREDICATE | MI_PREDICATE_LOADOP_LOADINV |
      MI_PREDICATE_COMBINEOP_SET |
      MI_PREDICATE_COMPAREOP_SRCS_EQUAL;

   crocus_batch_emit(batch, &mi_predicate, sizeof(uint32_t));
   mi_store(&b, query_mem64(q, offsetof(struct crocus_query_snapshots,
                                        predicate_result)), result);
#endif
   ice->state.compute_predicate = bo;
}
#endif

static void
crocus_render_condition(struct pipe_context *ctx,
                        struct pipe_query *query,
                        bool condition,
                        enum pipe_render_cond_flag mode)
{
   struct crocus_context *ice = (void *) ctx;
   struct crocus_query *q = (void *) query;

   /* The old condition isn't relevant; we'll update it if necessary */
   ice->state.compute_predicate = NULL;
   ice->condition.query = q;
   ice->condition.condition = condition;
   ice->condition.mode = mode;

   if (!q) {
      ice->state.predicate = CROCUS_PREDICATE_STATE_RENDER;
      return;
   }

   crocus_check_query_no_flush(ice, q);

   if (q->result || q->ready) {
      set_predicate_enable(ice, (q->result != 0) ^ condition);
   } else {
      if (mode == PIPE_RENDER_COND_NO_WAIT ||
          mode == PIPE_RENDER_COND_BY_REGION_NO_WAIT) {
         perf_debug(&ice->dbg, "Conditional rendering demoted from "
                    "\"no wait\" to \"wait\".");
      }
#if GFX_VER >= 7
      set_predicate_for_result(ice, q, condition);
#else
      ice->state.predicate = CROCUS_PREDICATE_STATE_STALL_FOR_QUERY;
#endif
   }
}

static void
crocus_resolve_conditional_render(struct crocus_context *ice)
{
   struct pipe_context *ctx = (void *) ice;
   struct crocus_query *q = ice->condition.query;
   struct pipe_query *query = (void *) q;
   union pipe_query_result result;

   if (ice->state.predicate != CROCUS_PREDICATE_STATE_USE_BIT)
      return;

   assert(q);

   crocus_get_query_result(ctx, query, true, &result);
   set_predicate_enable(ice, (q->result != 0) ^ ice->condition.condition);
}

#if GFX_VER >= 7
static void
crocus_emit_compute_predicate(struct crocus_batch *batch)
{
   struct crocus_context *ice = batch->ice;
   struct crocus_screen *screen = batch->screen;
   screen->vtbl.load_register_mem32(batch, MI_PREDICATE_SRC0,
                                    ice->state.compute_predicate, 0);
   screen->vtbl.load_register_imm32(batch, MI_PREDICATE_SRC1, 0);
   unsigned mi_predicate = MI_PREDICATE | MI_PREDICATE_LOADOP_LOADINV |
      MI_PREDICATE_COMBINEOP_SET |
      MI_PREDICATE_COMPAREOP_SRCS_EQUAL;

   crocus_batch_emit(batch, &mi_predicate, sizeof(uint32_t));
}
#endif

void
genX(crocus_init_screen_query)(struct crocus_screen *screen)
{
   screen->vtbl.resolve_conditional_render = crocus_resolve_conditional_render;
#if GFX_VER >= 7
   screen->vtbl.emit_compute_predicate = crocus_emit_compute_predicate;
#endif
}

void
genX(crocus_init_query)(struct crocus_context *ice)
{
   struct pipe_context *ctx = &ice->ctx;

   ctx->create_query = crocus_create_query;
   ctx->create_batch_query = crocus_create_batch_query;
   ctx->destroy_query = crocus_destroy_query;
   ctx->begin_query = crocus_begin_query;
   ctx->end_query = crocus_end_query;
   ctx->get_query_result = crocus_get_query_result;
#if GFX_VER >= 7
   ctx->get_query_result_resource = crocus_get_query_result_resource;
#endif
   ctx->set_active_query_state = crocus_set_active_query_state;
   ctx->render_condition = crocus_render_condition;

}
