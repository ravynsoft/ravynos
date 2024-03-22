/*
 * Copyright (C) 2018 Jonathan Marek <jonathan@marek.ca>
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
 *    Jonathan Marek <jonathan@marek.ca>
 *    Rob Clark <robclark@freedesktop.org>
 */

/* NOTE: perfcntrs are 48-bits but we only have 32-bit accumulate (?)
 * so we work with 32-bits only. we accumulate start/stop separately,
 * which differs from a5xx but works with only accumulate (no add/neg)
 */

#include "freedreno_query_acc.h"
#include "freedreno_resource.h"

#include "fd2_context.h"
#include "fd2_query.h"

struct PACKED fd2_query_sample {
   struct fd_acc_query_sample base;
   uint32_t start;
   uint32_t stop;
};
DEFINE_CAST(fd_acc_query_sample, fd2_query_sample);

/* offset of a single field of an array of fd2_query_sample: */
#define query_sample_idx(aq, idx, field)                                       \
   fd_resource((aq)->prsc)->bo,                                                \
      (idx * sizeof(struct fd2_query_sample)) +                                \
         offsetof(struct fd2_query_sample, field),                             \
      0, 0

/* offset of a single field of fd2_query_sample: */
#define query_sample(aq, field) query_sample_idx(aq, 0, field)

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
   struct fd_batch_query_data *data = aq->query_data;
   struct fd_screen *screen = data->screen;
   struct fd_ringbuffer *ring = batch->draw;

   unsigned counters_per_group[screen->num_perfcntr_groups];
   memset(counters_per_group, 0, sizeof(counters_per_group));

   fd_wfi(batch, ring);

   /* configure performance counters for the requested queries: */
   for (unsigned i = 0; i < data->num_query_entries; i++) {
      struct fd_batch_query_entry *entry = &data->query_entries[i];
      const struct fd_perfcntr_group *g = &screen->perfcntr_groups[entry->gid];
      unsigned counter_idx = counters_per_group[entry->gid]++;

      assert(counter_idx < g->num_counters);

      OUT_PKT0(ring, g->counters[counter_idx].select_reg, 1);
      OUT_RING(ring, g->countables[entry->cid].selector);
   }

   memset(counters_per_group, 0, sizeof(counters_per_group));

   /* and snapshot the start values */
   for (unsigned i = 0; i < data->num_query_entries; i++) {
      struct fd_batch_query_entry *entry = &data->query_entries[i];
      const struct fd_perfcntr_group *g = &screen->perfcntr_groups[entry->gid];
      unsigned counter_idx = counters_per_group[entry->gid]++;
      const struct fd_perfcntr_counter *counter = &g->counters[counter_idx];

      OUT_PKT3(ring, CP_REG_TO_MEM, 2);
      OUT_RING(ring, counter->counter_reg_lo | CP_REG_TO_MEM_0_ACCUMULATE);
      OUT_RELOC(ring, query_sample_idx(aq, i, start));
   }
}

static void
perfcntr_pause(struct fd_acc_query *aq, struct fd_batch *batch) assert_dt
{
   struct fd_batch_query_data *data = aq->query_data;
   struct fd_screen *screen = data->screen;
   struct fd_ringbuffer *ring = batch->draw;

   unsigned counters_per_group[screen->num_perfcntr_groups];
   memset(counters_per_group, 0, sizeof(counters_per_group));

   fd_wfi(batch, ring);

   /* TODO do we need to bother to turn anything off? */

   /* snapshot the end values: */
   for (unsigned i = 0; i < data->num_query_entries; i++) {
      struct fd_batch_query_entry *entry = &data->query_entries[i];
      const struct fd_perfcntr_group *g = &screen->perfcntr_groups[entry->gid];
      unsigned counter_idx = counters_per_group[entry->gid]++;
      const struct fd_perfcntr_counter *counter = &g->counters[counter_idx];

      OUT_PKT3(ring, CP_REG_TO_MEM, 2);
      OUT_RING(ring, counter->counter_reg_lo | CP_REG_TO_MEM_0_ACCUMULATE);
      OUT_RELOC(ring, query_sample_idx(aq, i, stop));
   }
}

static void
perfcntr_accumulate_result(struct fd_acc_query *aq,
                           struct fd_acc_query_sample *s,
                           union pipe_query_result *result)
{
   struct fd_batch_query_data *data = aq->query_data;
   struct fd2_query_sample *sp = fd2_query_sample(s);

   for (unsigned i = 0; i < data->num_query_entries; i++)
      result->batch[i].u64 = sp[i].stop - sp[i].start;
}

static const struct fd_acc_sample_provider perfcntr = {
   .query_type = FD_QUERY_FIRST_PERFCNTR,
   .always = true,
   .resume = perfcntr_resume,
   .pause = perfcntr_pause,
   .result = perfcntr_accumulate_result,
};

static struct pipe_query *
fd2_create_batch_query(struct pipe_context *pctx, unsigned num_queries,
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
   aq->size = num_queries * sizeof(struct fd2_query_sample);
   aq->query_data = data;

   return (struct pipe_query *)q;

error:
   free(data);
   return NULL;
}

void
fd2_query_context_init(struct pipe_context *pctx) disable_thread_safety_analysis
{
   struct fd_context *ctx = fd_context(pctx);

   ctx->create_query = fd_acc_create_query;
   ctx->query_update_batch = fd_acc_query_update_batch;

   pctx->create_batch_query = fd2_create_batch_query;
}
