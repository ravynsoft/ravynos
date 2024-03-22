/*
 * Copyright (c) 2017 Etnaviv Project
 * Copyright (C) 2017 Zodiac Inflight Innovations
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include "util/u_memory.h"

#include "etnaviv_context.h"
#include "etnaviv_debug.h"
#include "etnaviv_emit.h"
#include "etnaviv_query_acc.h"

#define MAX_PERFMON_SAMPLES 1022 /* (4KB / 4Byte/sample) - 1 reserved seqno */

struct etna_pm_query
{
   struct etna_acc_query base;

   struct etna_perfmon_signal *signal;
   unsigned sequence;
   bool multiply_with_8;
};

static inline struct etna_pm_query *
etna_pm_query(struct etna_acc_query *aq)
{
   return (struct etna_pm_query *)aq;
}

static inline void
pm_add_signal(struct etna_pm_query *pq, struct etna_perfmon *perfmon,
              const struct etna_perfmon_config *cfg)
{
   struct etna_perfmon_signal *signal = etna_pm_query_signal(perfmon, cfg->source);

   pq->signal = signal;
}

static void
pm_query(struct etna_context *ctx, struct etna_acc_query *aq, unsigned flags)
{
   struct etna_cmd_stream *stream = ctx->stream;
   struct etna_pm_query *pq = etna_pm_query(aq);
   unsigned offset;
   assert(flags);

   if (aq->samples > MAX_PERFMON_SAMPLES) {
      aq->samples = MAX_PERFMON_SAMPLES;
      BUG("samples overflow perfmon");
   }

   /* offset 0 is reserved for seq number */
   offset = 1 + aq->samples;

   pq->sequence++;

   /* skip seq number of 0 as the buffer got zeroed out */
   pq->sequence = MAX2(pq->sequence, 1);

   struct etna_perf p = {
      .flags = flags,
      .sequence = pq->sequence,
      .bo = etna_resource(aq->prsc)->bo,
      .signal = pq->signal,
      .offset = offset
   };

   etna_cmd_stream_perf(stream, &p);
   resource_written(ctx, aq->prsc);

   /* force a flush in !wait case in etna_acc_get_query_result(..) */
   aq->no_wait_cnt = 10;
}

static bool
perfmon_supports(unsigned query_type)
{
   return !!etna_pm_query_config(query_type);
}

static struct etna_acc_query *
perfmon_allocate(struct etna_context *ctx, unsigned query_type)
{
   struct etna_pm_query *pq;
   const struct etna_perfmon_config *cfg;

   cfg = etna_pm_query_config(query_type);
   if (!cfg)
      return false;

   if (!etna_pm_cfg_supported(ctx->screen->perfmon, cfg))
      return false;

   pq = CALLOC_STRUCT(etna_pm_query);
   if (!pq)
      return NULL;

   pm_add_signal(pq, ctx->screen->perfmon, cfg);
   pq->multiply_with_8 = cfg->multiply_with_8;

   return &pq->base;
}

static void
perfmon_resume(struct etna_acc_query *aq, struct etna_context *ctx)
{
   pm_query(ctx, aq, ETNA_PM_PROCESS_PRE);
   aq->samples++;
}

static void
perfmon_suspend(struct etna_acc_query *aq, struct etna_context *ctx)
{
   pm_query(ctx, aq, ETNA_PM_PROCESS_POST);
   aq->samples++;
}

static bool
perfmon_result(struct etna_acc_query *aq, void *buf,
               union pipe_query_result *result)
{
   const struct etna_pm_query *pq = etna_pm_query(aq);
   uint32_t sum = 0;
   uint32_t *ptr = (uint32_t *)buf;

   /* check seq number */
   if (pq->sequence > ptr[0])
      return false;

   /* jump over seq number */
   ptr++;

   assert(aq->samples % 2 == 0);

   /* each pair has a start and end value */
   for (unsigned i = 0; i < aq->samples; i += 2)
      sum += *(ptr + i + 1) - *(ptr + i);

   result->u32 = sum;

   if (pq->multiply_with_8)
      result->u32 *= 8;

   return true;
}

const struct etna_acc_sample_provider perfmon_provider = {
   .supports = perfmon_supports,
   .allocate = perfmon_allocate,
   .resume = perfmon_resume,
   .suspend = perfmon_suspend,
   .result = perfmon_result,
};
