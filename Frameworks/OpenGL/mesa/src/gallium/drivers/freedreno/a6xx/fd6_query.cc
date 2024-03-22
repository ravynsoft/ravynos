/*
 * Copyright (C) 2017 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#define FD_BO_NO_HARDPIN 1

/* NOTE: see https://gitlab.freedesktop.org/freedreno/freedreno/-/wikis/A5xx-Queries */

#include "freedreno_query_acc.h"
#include "freedreno_resource.h"

#include "fd6_context.h"
#include "fd6_emit.h"
#include "fd6_query.h"

/* g++ is a picky about offsets that cannot be resolved at compile time, so
 * roll our own __offsetof()
 */
#define __offsetof(type, field)                                                \
   ({ type _x = {}; ((uint8_t *)&_x.field) - ((uint8_t *)&_x);})

struct PACKED fd6_query_sample {
   struct fd_acc_query_sample base;

   /* The RB_SAMPLE_COUNT_ADDR destination needs to be 16-byte aligned: */
   uint64_t pad;

   uint64_t start;
   uint64_t result;
   uint64_t stop;
};
DEFINE_CAST(fd_acc_query_sample, fd6_query_sample);

/* offset of a single field of an array of fd6_query_sample: */
#define query_sample_idx(aq, idx, field)                                       \
   fd_resource((aq)->prsc)->bo,                                                \
      (idx * sizeof(struct fd6_query_sample)) +                                \
         offsetof(struct fd6_query_sample, field),                             \
      0, 0

/* offset of a single field of fd6_query_sample: */
#define query_sample(aq, field) query_sample_idx(aq, 0, field)

/*
 * Occlusion Query:
 *
 * OCCLUSION_COUNTER and OCCLUSION_PREDICATE differ only in how they
 * interpret results
 */

static void
occlusion_resume(struct fd_acc_query *aq, struct fd_batch *batch)
{
   struct fd_ringbuffer *ring = batch->draw;

   ASSERT_ALIGNED(struct fd6_query_sample, start, 16);

   OUT_PKT4(ring, REG_A6XX_RB_SAMPLE_COUNT_CONTROL, 1);
   OUT_RING(ring, A6XX_RB_SAMPLE_COUNT_CONTROL_COPY);

   OUT_PKT4(ring, REG_A6XX_RB_SAMPLE_COUNT_ADDR, 2);
   OUT_RELOC(ring, query_sample(aq, start));

   fd6_event_write(batch, ring, ZPASS_DONE, false);
}

static void
occlusion_pause(struct fd_acc_query *aq, struct fd_batch *batch) assert_dt
{
   struct fd_ringbuffer *ring = batch->draw;

   OUT_PKT7(ring, CP_MEM_WRITE, 4);
   OUT_RELOC(ring, query_sample(aq, stop));
   OUT_RING(ring, 0xffffffff);
   OUT_RING(ring, 0xffffffff);

   OUT_PKT7(ring, CP_WAIT_MEM_WRITES, 0);

   OUT_PKT4(ring, REG_A6XX_RB_SAMPLE_COUNT_CONTROL, 1);
   OUT_RING(ring, A6XX_RB_SAMPLE_COUNT_CONTROL_COPY);

   ASSERT_ALIGNED(struct fd6_query_sample, stop, 16);

   OUT_PKT4(ring, REG_A6XX_RB_SAMPLE_COUNT_ADDR, 2);
   OUT_RELOC(ring, query_sample(aq, stop));

   fd6_event_write(batch, ring, ZPASS_DONE, false);

   /* To avoid stalling in the draw buffer, emit code the code to compute the
    * counter delta in the epilogue ring.
    */
   struct fd_ringbuffer *epilogue = fd_batch_get_tile_epilogue(batch);

   OUT_PKT7(epilogue, CP_WAIT_REG_MEM, 6);
   OUT_RING(epilogue, CP_WAIT_REG_MEM_0_FUNCTION(WRITE_NE) |
                      CP_WAIT_REG_MEM_0_POLL(POLL_MEMORY));
   OUT_RELOC(epilogue, query_sample(aq, stop));
   OUT_RING(epilogue, CP_WAIT_REG_MEM_3_REF(0xffffffff));
   OUT_RING(epilogue, CP_WAIT_REG_MEM_4_MASK(0xffffffff));
   OUT_RING(epilogue, CP_WAIT_REG_MEM_5_DELAY_LOOP_CYCLES(16));

   /* result += stop - start: */
   OUT_PKT7(epilogue, CP_MEM_TO_MEM, 9);
   OUT_RING(epilogue, CP_MEM_TO_MEM_0_DOUBLE | CP_MEM_TO_MEM_0_NEG_C);
   OUT_RELOC(epilogue, query_sample(aq, result)); /* dst */
   OUT_RELOC(epilogue, query_sample(aq, result)); /* srcA */
   OUT_RELOC(epilogue, query_sample(aq, stop));   /* srcB */
   OUT_RELOC(epilogue, query_sample(aq, start));  /* srcC */
}

static void
occlusion_counter_result(struct fd_acc_query *aq,
                         struct fd_acc_query_sample *s,
                         union pipe_query_result *result)
{
   struct fd6_query_sample *sp = fd6_query_sample(s);
   result->u64 = sp->result;
}

static void
occlusion_counter_result_resource(struct fd_acc_query *aq, struct fd_ringbuffer *ring,
                                  enum pipe_query_value_type result_type,
                                  int index, struct fd_resource *dst,
                                  unsigned offset)
{
   copy_result(ring, result_type, dst, offset, fd_resource(aq->prsc),
               offsetof(struct fd6_query_sample, result));
}

static void
occlusion_predicate_result(struct fd_acc_query *aq,
                           struct fd_acc_query_sample *s,
                           union pipe_query_result *result)
{
   struct fd6_query_sample *sp = fd6_query_sample(s);
   result->b = !!sp->result;
}

static void
occlusion_predicate_result_resource(struct fd_acc_query *aq, struct fd_ringbuffer *ring,
                                    enum pipe_query_value_type result_type,
                                    int index, struct fd_resource *dst,
                                    unsigned offset)
{
   /* This is a bit annoying but we need to turn the result into a one or
    * zero.. to do this use a CP_COND_WRITE to overwrite the result with
    * a one if it is non-zero.  This doesn't change the results if the
    * query is also read on the CPU (ie. occlusion_predicate_result()).
    */
   OUT_PKT7(ring, CP_COND_WRITE5, 9);
   OUT_RING(ring, CP_COND_WRITE5_0_FUNCTION(WRITE_NE) |
                  CP_WAIT_REG_MEM_0_POLL(POLL_MEMORY) |
                  CP_COND_WRITE5_0_WRITE_MEMORY);
   OUT_RELOC(ring, query_sample(aq, result)); /* POLL_ADDR_LO/HI */
   OUT_RING(ring, CP_COND_WRITE5_3_REF(0));
   OUT_RING(ring, CP_COND_WRITE5_4_MASK(~0));
   OUT_RELOC(ring, query_sample(aq, result)); /* WRITE_ADDR_LO/HI */
   OUT_RING(ring, 1);
   OUT_RING(ring, 0);

   copy_result(ring, result_type, dst, offset, fd_resource(aq->prsc),
               offsetof(struct fd6_query_sample, result));
}

static const struct fd_acc_sample_provider occlusion_counter = {
   .query_type = PIPE_QUERY_OCCLUSION_COUNTER,
   .size = sizeof(struct fd6_query_sample),
   .resume = occlusion_resume,
   .pause = occlusion_pause,
   .result = occlusion_counter_result,
   .result_resource = occlusion_counter_result_resource,
};

static const struct fd_acc_sample_provider occlusion_predicate = {
   .query_type = PIPE_QUERY_OCCLUSION_PREDICATE,
   .size = sizeof(struct fd6_query_sample),
   .resume = occlusion_resume,
   .pause = occlusion_pause,
   .result = occlusion_predicate_result,
   .result_resource = occlusion_predicate_result_resource,
};

static const struct fd_acc_sample_provider occlusion_predicate_conservative = {
   .query_type = PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE,
   .size = sizeof(struct fd6_query_sample),
   .resume = occlusion_resume,
   .pause = occlusion_pause,
   .result = occlusion_predicate_result,
   .result_resource = occlusion_predicate_result_resource,
};

/*
 * Timestamp Queries:
 */

static void
timestamp_resume(struct fd_acc_query *aq, struct fd_batch *batch)
{
   struct fd_ringbuffer *ring = batch->draw;

   OUT_PKT7(ring, CP_EVENT_WRITE, 4);
   OUT_RING(ring,
            CP_EVENT_WRITE_0_EVENT(RB_DONE_TS) | CP_EVENT_WRITE_0_TIMESTAMP);
   OUT_RELOC(ring, query_sample(aq, start));
   OUT_RING(ring, 0x00000000);
}

static void
time_elapsed_pause(struct fd_acc_query *aq, struct fd_batch *batch) assert_dt
{
   struct fd_ringbuffer *ring = batch->draw;

   OUT_PKT7(ring, CP_EVENT_WRITE, 4);
   OUT_RING(ring,
            CP_EVENT_WRITE_0_EVENT(RB_DONE_TS) | CP_EVENT_WRITE_0_TIMESTAMP);
   OUT_RELOC(ring, query_sample(aq, stop));
   OUT_RING(ring, 0x00000000);

   OUT_WFI5(ring);

   /* result += stop - start: */
   OUT_PKT7(ring, CP_MEM_TO_MEM, 9);
   OUT_RING(ring, CP_MEM_TO_MEM_0_DOUBLE | CP_MEM_TO_MEM_0_NEG_C);
   OUT_RELOC(ring, query_sample(aq, result)); /* dst */
   OUT_RELOC(ring, query_sample(aq, result)); /* srcA */
   OUT_RELOC(ring, query_sample(aq, stop));   /* srcB */
   OUT_RELOC(ring, query_sample(aq, start));  /* srcC */
}

static void
timestamp_pause(struct fd_acc_query *aq, struct fd_batch *batch)
{
   /* We captured a timestamp in timestamp_resume(), nothing to do here. */
}

/* timestamp logging for u_trace: */
static void
record_timestamp(struct fd_ringbuffer *ring, struct fd_bo *bo, unsigned offset)
{
   OUT_PKT7(ring, CP_EVENT_WRITE, 4);
   OUT_RING(ring,
            CP_EVENT_WRITE_0_EVENT(RB_DONE_TS) | CP_EVENT_WRITE_0_TIMESTAMP);
   OUT_RELOC(ring, bo, offset, 0, 0);
   OUT_RING(ring, 0x00000000);
}

static void
time_elapsed_accumulate_result(struct fd_acc_query *aq,
                               struct fd_acc_query_sample *s,
                               union pipe_query_result *result)
{
   struct fd6_query_sample *sp = fd6_query_sample(s);
   result->u64 = ticks_to_ns(sp->result);
}

static void
time_elapsed_result_resource(struct fd_acc_query *aq, struct fd_ringbuffer *ring,
                             enum pipe_query_value_type result_type,
                             int index, struct fd_resource *dst,
                             unsigned offset)
{
   // TODO ticks_to_ns conversion would require spinning up a compute shader?
   copy_result(ring, result_type, dst, offset, fd_resource(aq->prsc),
               offsetof(struct fd6_query_sample, result));
}

static void
timestamp_accumulate_result(struct fd_acc_query *aq,
                            struct fd_acc_query_sample *s,
                            union pipe_query_result *result)
{
   struct fd6_query_sample *sp = fd6_query_sample(s);
   result->u64 = ticks_to_ns(sp->start);
}

static void
timestamp_result_resource(struct fd_acc_query *aq, struct fd_ringbuffer *ring,
                          enum pipe_query_value_type result_type,
                          int index, struct fd_resource *dst,
                          unsigned offset)
{
   // TODO ticks_to_ns conversion would require spinning up a compute shader?
   copy_result(ring, result_type, dst, offset, fd_resource(aq->prsc),
               offsetof(struct fd6_query_sample, start));
}

static const struct fd_acc_sample_provider time_elapsed = {
   .query_type = PIPE_QUERY_TIME_ELAPSED,
   .always = true,
   .size = sizeof(struct fd6_query_sample),
   .resume = timestamp_resume,
   .pause = time_elapsed_pause,
   .result = time_elapsed_accumulate_result,
   .result_resource = time_elapsed_result_resource,
};

/* NOTE: timestamp query isn't going to give terribly sensible results
 * on a tiler.  But it is needed by qapitrace profile heatmap.  If you
 * add in a binning pass, the results get even more non-sensical.  So
 * we just return the timestamp on the last tile and hope that is
 * kind of good enough.
 */

static const struct fd_acc_sample_provider timestamp = {
   .query_type = PIPE_QUERY_TIMESTAMP,
   .always = true,
   .size = sizeof(struct fd6_query_sample),
   .resume = timestamp_resume,
   .pause = timestamp_pause,
   .result = timestamp_accumulate_result,
   .result_resource = timestamp_result_resource,
};

struct PACKED fd6_pipeline_stats_sample {
   struct fd_acc_query_sample base;

   uint64_t start, stop, result;
};
DEFINE_CAST(fd_acc_query_sample, fd6_pipeline_stats_sample);

#define stats_reloc(ring, aq, field)                                           \
   OUT_RELOC(ring, fd_resource((aq)->prsc)->bo,                                \
             __offsetof(struct fd6_pipeline_stats_sample, field), 0, 0);

/* Mapping of counters to pipeline stats:
 *
 *   Gallium (PIPE_STAT_QUERY_x) | Vulkan (VK_QUERY_PIPELINE_STATISTIC_x_BIT) | hw counter
 *   ----------------------------+--------------------------------------------+----------------
 *   IA_VERTICES                 | INPUT_ASSEMBLY_VERTICES                    | RBBM_PRIMCTR_0
 *   IA_PRIMITIVES               | INPUT_ASSEMBLY_PRIMITIVES                  | RBBM_PRIMCTR_1
 *   VS_INVOCATIONS              | VERTEX_SHADER_INVOCATIONS                  | RBBM_PRIMCTR_0
 *   GS_INVOCATIONS              | GEOMETRY_SHADER_INVOCATIONS                | RBBM_PRIMCTR_5
 *   GS_PRIMITIVES               | GEOMETRY_SHADER_PRIMITIVES                 | RBBM_PRIMCTR_6
 *   C_INVOCATIONS               | CLIPPING_INVOCATIONS                       | RBBM_PRIMCTR_7
 *   C_PRIMITIVES                | CLIPPING_PRIMITIVES                        | RBBM_PRIMCTR_8
 *   PS_INVOCATIONS              | FRAGMENT_SHADER_INVOCATIONS                | RBBM_PRIMCTR_9
 *   HS_INVOCATIONS              | TESSELLATION_CONTROL_SHADER_PATCHES        | RBBM_PRIMCTR_2
 *   DS_INVOCATIONS              | TESSELLATION_EVALUATION_SHADER_INVOCATIONS | RBBM_PRIMCTR_4
 *   CS_INVOCATIONS              | COMPUTE_SHADER_INVOCATIONS                 | RBBM_PRIMCTR_10
 *
 * Note that "Vertices corresponding to incomplete primitives may contribute to the count.",
 * in our case they do not, so IA_VERTICES and VS_INVOCATIONS are the same thing.
 */

enum stats_type {
   STATS_PRIMITIVE,
   STATS_FRAGMENT,
   STATS_COMPUTE,
};

static const struct {
   enum vgt_event_type start, stop;
} stats_counter_events[] = {
      [STATS_PRIMITIVE] = { START_PRIMITIVE_CTRS, STOP_PRIMITIVE_CTRS },
      [STATS_FRAGMENT]  = { START_FRAGMENT_CTRS,  STOP_FRAGMENT_CTRS },
      [STATS_COMPUTE]   = { START_COMPUTE_CTRS,   STOP_COMPUTE_CTRS },
};

static enum stats_type
get_stats_type(struct fd_acc_query *aq)
{
   if (aq->provider->query_type == PIPE_QUERY_PRIMITIVES_GENERATED)
      return STATS_PRIMITIVE;

   switch (aq->base.index) {
   case PIPE_STAT_QUERY_PS_INVOCATIONS: return STATS_FRAGMENT;
   case PIPE_STAT_QUERY_CS_INVOCATIONS: return STATS_COMPUTE;
   default:
      return STATS_PRIMITIVE;
   }
}

static unsigned
stats_counter_index(struct fd_acc_query *aq)
{
   if (aq->provider->query_type == PIPE_QUERY_PRIMITIVES_GENERATED)
      return 7;

   switch (aq->base.index) {
   case PIPE_STAT_QUERY_IA_VERTICES:    return 0;
   case PIPE_STAT_QUERY_IA_PRIMITIVES:  return 1;
   case PIPE_STAT_QUERY_VS_INVOCATIONS: return 0;
   case PIPE_STAT_QUERY_GS_INVOCATIONS: return 5;
   case PIPE_STAT_QUERY_GS_PRIMITIVES:  return 6;
   case PIPE_STAT_QUERY_C_INVOCATIONS:  return 7;
   case PIPE_STAT_QUERY_C_PRIMITIVES:   return 8;
   case PIPE_STAT_QUERY_PS_INVOCATIONS: return 9;
   case PIPE_STAT_QUERY_HS_INVOCATIONS: return 2;
   case PIPE_STAT_QUERY_DS_INVOCATIONS: return 4;
   case PIPE_STAT_QUERY_CS_INVOCATIONS: return 10;
   default:
      return 0;
   }
}

static void
log_pipeline_stats(struct fd6_pipeline_stats_sample *ps, unsigned idx)
{
#ifdef DEBUG_COUNTERS
   const char *labels[] = {
      "VS_INVOCATIONS",
      "IA_PRIMITIVES",
      "HS_INVOCATIONS",
      "??",
      "DS_INVOCATIONS",
      "GS_INVOCATIONS",
      "GS_PRIMITIVES",
      "C_INVOCATIONS",
      "C_PRIMITIVES",
      "PS_INVOCATIONS",
      "CS_INVOCATIONS",
   };

   mesa_logd("  counter\t\tstart\t\t\tstop\t\t\tdiff");
   mesa_logd("  RBBM_PRIMCTR_%d\t0x%016" PRIx64 "\t0x%016" PRIx64 "\t%" PRIi64 "\t%s",
             idx, ps->start, ps->stop, ps->stop - ps->start, labels[idx]);
#endif
}

static void
pipeline_stats_resume(struct fd_acc_query *aq, struct fd_batch *batch)
   assert_dt
{
   struct fd_ringbuffer *ring = batch->draw;
   enum stats_type type = get_stats_type(aq);
   unsigned idx = stats_counter_index(aq);
   unsigned reg = REG_A6XX_RBBM_PRIMCTR_0_LO + (2 * idx);

   OUT_WFI5(ring);

   OUT_PKT7(ring, CP_REG_TO_MEM, 3);
   OUT_RING(ring, CP_REG_TO_MEM_0_64B |
                  CP_REG_TO_MEM_0_CNT(2) |
                  CP_REG_TO_MEM_0_REG(reg));
   stats_reloc(ring, aq, start);

   assert(type < ARRAY_SIZE(batch->pipeline_stats_queries_active));

   if (!batch->pipeline_stats_queries_active[type])
      fd6_event_write(batch, ring, stats_counter_events[type].start, false);
   batch->pipeline_stats_queries_active[type]++;
}

static void
pipeline_stats_pause(struct fd_acc_query *aq, struct fd_batch *batch)
   assert_dt
{
   struct fd_ringbuffer *ring = batch->draw;
   enum stats_type type = get_stats_type(aq);
   unsigned idx = stats_counter_index(aq);
   unsigned reg = REG_A6XX_RBBM_PRIMCTR_0_LO + (2 * idx);

   OUT_WFI5(ring);

   /* snapshot the end values: */
   OUT_PKT7(ring, CP_REG_TO_MEM, 3);
   OUT_RING(ring, CP_REG_TO_MEM_0_64B |
                  CP_REG_TO_MEM_0_CNT(2) |
                  CP_REG_TO_MEM_0_REG(reg));
   stats_reloc(ring, aq, stop);

   assert(type < ARRAY_SIZE(batch->pipeline_stats_queries_active));
   assert(batch->pipeline_stats_queries_active[type] > 0);

   batch->pipeline_stats_queries_active[type]--;
   if (batch->pipeline_stats_queries_active[type])
      fd6_event_write(batch, ring, stats_counter_events[type].stop, false);

   /* result += stop - start: */
   OUT_PKT7(ring, CP_MEM_TO_MEM, 9);
   OUT_RING(ring, CP_MEM_TO_MEM_0_DOUBLE | CP_MEM_TO_MEM_0_NEG_C | 0x40000000);
   stats_reloc(ring, aq, result);
   stats_reloc(ring, aq, result);
   stats_reloc(ring, aq, stop)
   stats_reloc(ring, aq, start);
}

static void
pipeline_stats_result(struct fd_acc_query *aq,
                      struct fd_acc_query_sample *s,
                      union pipe_query_result *result)
{
   struct fd6_pipeline_stats_sample *ps = fd6_pipeline_stats_sample(s);

   log_pipeline_stats(ps, stats_counter_index(aq));

   result->u64 = ps->result;
}

static void
pipeline_stats_result_resource(struct fd_acc_query *aq,
                               struct fd_ringbuffer *ring,
                               enum pipe_query_value_type result_type,
                               int index, struct fd_resource *dst,
                               unsigned offset)
{
   copy_result(ring, result_type, dst, offset, fd_resource(aq->prsc),
               offsetof(struct fd6_pipeline_stats_sample, result));
}

static const struct fd_acc_sample_provider primitives_generated = {
   .query_type = PIPE_QUERY_PRIMITIVES_GENERATED,
   .size = sizeof(struct fd6_pipeline_stats_sample),
   .resume = pipeline_stats_resume,
   .pause = pipeline_stats_pause,
   .result = pipeline_stats_result,
   .result_resource = pipeline_stats_result_resource,
};

static const struct fd_acc_sample_provider pipeline_statistics_single = {
   .query_type = PIPE_QUERY_PIPELINE_STATISTICS_SINGLE,
   .size = sizeof(struct fd6_pipeline_stats_sample),
   .resume = pipeline_stats_resume,
   .pause = pipeline_stats_pause,
   .result = pipeline_stats_result,
   .result_resource = pipeline_stats_result_resource,
};

struct PACKED fd6_primitives_sample {
   struct fd_acc_query_sample base;

   /* VPC_SO_STREAM_COUNTS dest address must be 32b aligned: */
   uint64_t pad[3];

   struct {
      uint64_t emitted, generated;
   } start[4], stop[4], result;
};
DEFINE_CAST(fd_acc_query_sample, fd6_primitives_sample);

#define primitives_reloc(ring, aq, field)                                      \
   OUT_RELOC(ring, fd_resource((aq)->prsc)->bo,                                \
             __offsetof(struct fd6_primitives_sample, field), 0, 0);

static void
log_primitives_sample(struct fd6_primitives_sample *ps)
{
#ifdef DEBUG_COUNTERS
   mesa_logd("  so counts");
   for (int i = 0; i < ARRAY_SIZE(ps->start); i++) {
      mesa_logd("  CHANNEL %d emitted\t0x%016" PRIx64 "\t0x%016" PRIx64
             "\t%" PRIi64,
             i, ps->start[i].generated, ps->stop[i].generated,
             ps->stop[i].generated - ps->start[i].generated);
      mesa_logd("  CHANNEL %d generated\t0x%016" PRIx64 "\t0x%016" PRIx64
             "\t%" PRIi64,
             i, ps->start[i].emitted, ps->stop[i].emitted,
             ps->stop[i].emitted - ps->start[i].emitted);
   }

   mesa_logd("generated %" PRIu64 ", emitted %" PRIu64, ps->result.generated,
          ps->result.emitted);
#endif
}

static void
primitives_emitted_resume(struct fd_acc_query *aq,
                          struct fd_batch *batch) assert_dt
{
   struct fd_ringbuffer *ring = batch->draw;

   OUT_WFI5(ring);

   ASSERT_ALIGNED(struct fd6_primitives_sample, start[0], 32);

   OUT_PKT4(ring, REG_A6XX_VPC_SO_STREAM_COUNTS, 2);
   primitives_reloc(ring, aq, start[0]);

   fd6_event_write(batch, ring, WRITE_PRIMITIVE_COUNTS, false);
}

static void
accumultate_primitives_emitted(struct fd_acc_query *aq,
                               struct fd_ringbuffer *ring,
                               int idx)
{
   /* result += stop - start: */
   OUT_PKT7(ring, CP_MEM_TO_MEM, 9);
   OUT_RING(ring, CP_MEM_TO_MEM_0_DOUBLE | CP_MEM_TO_MEM_0_NEG_C | 0x80000000);
   primitives_reloc(ring, aq, result.emitted);
   primitives_reloc(ring, aq, result.emitted);
   primitives_reloc(ring, aq, stop[idx].emitted);
   primitives_reloc(ring, aq, start[idx].emitted);
}

static void
accumultate_primitives_generated(struct fd_acc_query *aq,
                                 struct fd_ringbuffer *ring,
                                 int idx)
{
   /* result += stop - start: */
   OUT_PKT7(ring, CP_MEM_TO_MEM, 9);
   OUT_RING(ring, CP_MEM_TO_MEM_0_DOUBLE | CP_MEM_TO_MEM_0_NEG_C | 0x80000000);
   primitives_reloc(ring, aq, result.generated);
   primitives_reloc(ring, aq, result.generated);
   primitives_reloc(ring, aq, stop[idx].generated);
   primitives_reloc(ring, aq, start[idx].generated);
}

static void
primitives_emitted_pause(struct fd_acc_query *aq,
                         struct fd_batch *batch) assert_dt
{
   struct fd_ringbuffer *ring = batch->draw;

   OUT_WFI5(ring);

   ASSERT_ALIGNED(struct fd6_primitives_sample, stop[0], 32);

   OUT_PKT4(ring, REG_A6XX_VPC_SO_STREAM_COUNTS, 2);
   primitives_reloc(ring, aq, stop[0]);

   fd6_event_write(batch, ring, WRITE_PRIMITIVE_COUNTS, false);

   fd6_event_write(batch, batch->draw, CACHE_FLUSH_TS, true);

   if (aq->provider->query_type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE) {
      /* Need results from all channels: */
      for (int i = 0; i < PIPE_MAX_SO_BUFFERS; i++) {
         accumultate_primitives_emitted(aq, ring, i);
         accumultate_primitives_generated(aq, ring, i);
      }
   } else {
      accumultate_primitives_emitted(aq, ring, aq->base.index);
      /* Only need primitives generated counts for the overflow queries: */
      if (aq->provider->query_type == PIPE_QUERY_SO_OVERFLOW_PREDICATE)
         accumultate_primitives_generated(aq, ring, aq->base.index);
   }
}

static void
primitives_emitted_result(struct fd_acc_query *aq,
                          struct fd_acc_query_sample *s,
                          union pipe_query_result *result)
{
   struct fd6_primitives_sample *ps = fd6_primitives_sample(s);

   log_primitives_sample(ps);

   result->u64 = ps->result.emitted;
}

static void
primitives_emitted_result_resource(struct fd_acc_query *aq,
                                   struct fd_ringbuffer *ring,
                                   enum pipe_query_value_type result_type,
                                   int index, struct fd_resource *dst,
                                   unsigned offset)
{
   copy_result(ring, result_type, dst, offset, fd_resource(aq->prsc),
               offsetof(struct fd6_primitives_sample, result.emitted));
}

static void
so_overflow_predicate_result(struct fd_acc_query *aq,
                             struct fd_acc_query_sample *s,
                             union pipe_query_result *result)
{
   struct fd6_primitives_sample *ps = fd6_primitives_sample(s);

   log_primitives_sample(ps);

   result->b = ps->result.emitted != ps->result.generated;
}

static void
so_overflow_predicate_result_resource(struct fd_acc_query *aq,
                                      struct fd_ringbuffer *ring,
                                      enum pipe_query_value_type result_type,
                                      int index, struct fd_resource *dst,
                                      unsigned offset)
{
   fd_ringbuffer_attach_bo(ring, dst->bo);
   fd_ringbuffer_attach_bo(ring, fd_resource(aq->prsc)->bo);

   /* result = generated - emitted: */
   OUT_PKT7(ring, CP_MEM_TO_MEM, 7);
   OUT_RING(ring, CP_MEM_TO_MEM_0_NEG_B |
            COND(result_type >= PIPE_QUERY_TYPE_I64, CP_MEM_TO_MEM_0_DOUBLE));
   OUT_RELOC(ring, dst->bo, offset, 0, 0);
   primitives_reloc(ring, aq, result.generated);
   primitives_reloc(ring, aq, result.emitted);

   /* This is a bit awkward, but glcts expects the result to be 1 or 0
    * rather than non-zero vs zero:
    */
   OUT_PKT7(ring, CP_COND_WRITE5, 9);
   OUT_RING(ring, CP_COND_WRITE5_0_FUNCTION(WRITE_NE) |
                  CP_COND_WRITE5_0_POLL(POLL_MEMORY) |
                  CP_COND_WRITE5_0_WRITE_MEMORY);
   OUT_RELOC(ring, dst->bo, offset, 0, 0);    /* POLL_ADDR_LO/HI */
   OUT_RING(ring, CP_COND_WRITE5_3_REF(0));
   OUT_RING(ring, CP_COND_WRITE5_4_MASK(~0));
   OUT_RELOC(ring, dst->bo, offset, 0, 0);    /* WRITE_ADDR_LO/HI */
   OUT_RING(ring, 1);
   OUT_RING(ring, 0);
}

static const struct fd_acc_sample_provider primitives_emitted = {
   .query_type = PIPE_QUERY_PRIMITIVES_EMITTED,
   .size = sizeof(struct fd6_primitives_sample),
   .resume = primitives_emitted_resume,
   .pause = primitives_emitted_pause,
   .result = primitives_emitted_result,
   .result_resource = primitives_emitted_result_resource,
};

static const struct fd_acc_sample_provider so_overflow_any_predicate = {
   .query_type = PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE,
   .size = sizeof(struct fd6_primitives_sample),
   .resume = primitives_emitted_resume,
   .pause = primitives_emitted_pause,
   .result = so_overflow_predicate_result,
   .result_resource = so_overflow_predicate_result_resource,
};

static const struct fd_acc_sample_provider so_overflow_predicate = {
   .query_type = PIPE_QUERY_SO_OVERFLOW_PREDICATE,
   .size = sizeof(struct fd6_primitives_sample),
   .resume = primitives_emitted_resume,
   .pause = primitives_emitted_pause,
   .result = so_overflow_predicate_result,
   .result_resource = so_overflow_predicate_result_resource,
};

/*
 * Performance Counter (batch) queries:
 *
 * Only one of these is active at a time, per design of the gallium
 * batch_query API design.  On perfcntr query tracks N query_types,
 * each of which has a 'fd_batch_query_entry' that maps it back to
 * the associated group and counter.
 */

struct fd_batch_query_entry {
   uint8_t gid; /* group-id */
   uint8_t cid; /* countable-id within the group */
};

struct fd_batch_query_data {
   struct fd_screen *screen;
   unsigned num_query_entries;
   struct fd_batch_query_entry query_entries[];
};

static void
perfcntr_resume(struct fd_acc_query *aq, struct fd_batch *batch) assert_dt
{
   struct fd_batch_query_data *data = (struct fd_batch_query_data *)aq->query_data;
   struct fd_screen *screen = data->screen;
   struct fd_ringbuffer *ring = batch->draw;

   unsigned counters_per_group[screen->num_perfcntr_groups];
   memset(counters_per_group, 0, sizeof(counters_per_group));

   OUT_WFI5(ring);

   /* configure performance counters for the requested queries: */
   for (unsigned i = 0; i < data->num_query_entries; i++) {
      struct fd_batch_query_entry *entry = &data->query_entries[i];
      const struct fd_perfcntr_group *g = &screen->perfcntr_groups[entry->gid];
      unsigned counter_idx = counters_per_group[entry->gid]++;

      assert(counter_idx < g->num_counters);

      OUT_PKT4(ring, g->counters[counter_idx].select_reg, 1);
      OUT_RING(ring, g->countables[entry->cid].selector);
   }

   memset(counters_per_group, 0, sizeof(counters_per_group));

   /* and snapshot the start values */
   for (unsigned i = 0; i < data->num_query_entries; i++) {
      struct fd_batch_query_entry *entry = &data->query_entries[i];
      const struct fd_perfcntr_group *g = &screen->perfcntr_groups[entry->gid];
      unsigned counter_idx = counters_per_group[entry->gid]++;
      const struct fd_perfcntr_counter *counter = &g->counters[counter_idx];

      OUT_PKT7(ring, CP_REG_TO_MEM, 3);
      OUT_RING(ring, CP_REG_TO_MEM_0_64B |
                        CP_REG_TO_MEM_0_REG(counter->counter_reg_lo));
      OUT_RELOC(ring, query_sample_idx(aq, i, start));
   }
}

static void
perfcntr_pause(struct fd_acc_query *aq, struct fd_batch *batch) assert_dt
{
   struct fd_batch_query_data *data = (struct fd_batch_query_data *)aq->query_data;
   struct fd_screen *screen = data->screen;
   struct fd_ringbuffer *ring = batch->draw;

   unsigned counters_per_group[screen->num_perfcntr_groups];
   memset(counters_per_group, 0, sizeof(counters_per_group));

   OUT_WFI5(ring);

   /* TODO do we need to bother to turn anything off? */

   /* snapshot the end values: */
   for (unsigned i = 0; i < data->num_query_entries; i++) {
      struct fd_batch_query_entry *entry = &data->query_entries[i];
      const struct fd_perfcntr_group *g = &screen->perfcntr_groups[entry->gid];
      unsigned counter_idx = counters_per_group[entry->gid]++;
      const struct fd_perfcntr_counter *counter = &g->counters[counter_idx];

      OUT_PKT7(ring, CP_REG_TO_MEM, 3);
      OUT_RING(ring, CP_REG_TO_MEM_0_64B |
                        CP_REG_TO_MEM_0_REG(counter->counter_reg_lo));
      OUT_RELOC(ring, query_sample_idx(aq, i, stop));
   }

   /* and compute the result: */
   for (unsigned i = 0; i < data->num_query_entries; i++) {
      /* result += stop - start: */
      OUT_PKT7(ring, CP_MEM_TO_MEM, 9);
      OUT_RING(ring, CP_MEM_TO_MEM_0_DOUBLE | CP_MEM_TO_MEM_0_NEG_C);
      OUT_RELOC(ring, query_sample_idx(aq, i, result)); /* dst */
      OUT_RELOC(ring, query_sample_idx(aq, i, result)); /* srcA */
      OUT_RELOC(ring, query_sample_idx(aq, i, stop));   /* srcB */
      OUT_RELOC(ring, query_sample_idx(aq, i, start));  /* srcC */
   }
}

static void
perfcntr_accumulate_result(struct fd_acc_query *aq,
                           struct fd_acc_query_sample *s,
                           union pipe_query_result *result)
{
   struct fd_batch_query_data *data =
         (struct fd_batch_query_data *)aq->query_data;
   struct fd6_query_sample *sp = fd6_query_sample(s);

   for (unsigned i = 0; i < data->num_query_entries; i++) {
      result->batch[i].u64 = sp[i].result;
   }
}

static const struct fd_acc_sample_provider perfcntr = {
   .query_type = FD_QUERY_FIRST_PERFCNTR,
   .always = true,
   .resume = perfcntr_resume,
   .pause = perfcntr_pause,
   .result = perfcntr_accumulate_result,
};

static struct pipe_query *
fd6_create_batch_query(struct pipe_context *pctx, unsigned num_queries,
                       unsigned *query_types)
{
   struct fd_context *ctx = fd_context(pctx);
   struct fd_screen *screen = ctx->screen;
   struct fd_query *q;
   struct fd_acc_query *aq;
   struct fd_batch_query_data *data;

   data = CALLOC_VARIANT_LENGTH_STRUCT(
      fd_batch_query_data, num_queries * sizeof(data->query_entries[0]));

   data->screen = screen;
   data->num_query_entries = num_queries;

   /* validate the requested query_types and ensure we don't try
    * to request more query_types of a given group than we have
    * counters:
    */
   unsigned counters_per_group[screen->num_perfcntr_groups];
   memset(counters_per_group, 0, sizeof(counters_per_group));

   for (unsigned i = 0; i < num_queries; i++) {
      unsigned idx = query_types[i] - FD_QUERY_FIRST_PERFCNTR;

      /* verify valid query_type, ie. is it actually a perfcntr? */
      if ((query_types[i] < FD_QUERY_FIRST_PERFCNTR) ||
          (idx >= screen->num_perfcntr_queries)) {
         mesa_loge("invalid batch query query_type: %u", query_types[i]);
         goto error;
      }

      struct fd_batch_query_entry *entry = &data->query_entries[i];
      struct pipe_driver_query_info *pq = &screen->perfcntr_queries[idx];

      entry->gid = pq->group_id;

      /* the perfcntr_queries[] table flattens all the countables
       * for each group in series, ie:
       *
       *   (G0,C0), .., (G0,Cn), (G1,C0), .., (G1,Cm), ...
       *
       * So to find the countable index just step back through the
       * table to find the first entry with the same group-id.
       */
      while (pq > screen->perfcntr_queries) {
         pq--;
         if (pq->group_id == entry->gid)
            entry->cid++;
      }

      if (counters_per_group[entry->gid] >=
          screen->perfcntr_groups[entry->gid].num_counters) {
         mesa_loge("too many counters for group %u", entry->gid);
         goto error;
      }

      counters_per_group[entry->gid]++;
   }

   q = fd_acc_create_query2(ctx, 0, 0, &perfcntr);
   aq = fd_acc_query(q);

   /* sample buffer size is based on # of queries: */
   aq->size = num_queries * sizeof(struct fd6_query_sample);
   aq->query_data = data;

   return (struct pipe_query *)q;

error:
   free(data);
   return NULL;
}

void
fd6_query_context_init(struct pipe_context *pctx) disable_thread_safety_analysis
{
   struct fd_context *ctx = fd_context(pctx);

   ctx->create_query = fd_acc_create_query;
   ctx->query_update_batch = fd_acc_query_update_batch;

   ctx->record_timestamp = record_timestamp;
   ctx->ts_to_ns = ticks_to_ns;

   pctx->create_batch_query = fd6_create_batch_query;

   fd_acc_query_register_provider(pctx, &occlusion_counter);
   fd_acc_query_register_provider(pctx, &occlusion_predicate);
   fd_acc_query_register_provider(pctx, &occlusion_predicate_conservative);

   fd_acc_query_register_provider(pctx, &time_elapsed);
   fd_acc_query_register_provider(pctx, &timestamp);

   fd_acc_query_register_provider(pctx, &primitives_generated);
   fd_acc_query_register_provider(pctx, &pipeline_statistics_single);

   fd_acc_query_register_provider(pctx, &primitives_emitted);
   fd_acc_query_register_provider(pctx, &so_overflow_any_predicate);
   fd_acc_query_register_provider(pctx, &so_overflow_predicate);
}
