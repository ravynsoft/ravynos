/*
 * Copyright © 2018 Intel Corporation
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

#ifndef INTEL_PERF_SETUP_H
#define INTEL_PERF_SETUP_H

#include "perf/intel_perf.h"

#define MIN(a, b) ((a < b) ? (a) : (b))
#define MAX(a, b) ((a > b) ? (a) : (b))

static struct intel_perf_query_info *
intel_query_alloc(struct intel_perf_config *perf, int ncounters)
{
   struct intel_perf_query_info *query = rzalloc(perf, struct intel_perf_query_info);
   query->perf = perf;
   query->kind = INTEL_PERF_QUERY_TYPE_OA;
   query->n_counters = 0;
   query->oa_metrics_set_id = 0; /* determined at runtime, via sysfs */
   query->counters = rzalloc_array(query, struct intel_perf_query_counter, ncounters);

   /* Accumulation buffer offsets... */
   if (perf->devinfo.verx10 <= 75) {
      query->oa_format = I915_OA_FORMAT_A45_B8_C8;
      query->gpu_time_offset = 0;
      query->a_offset = query->gpu_time_offset + 1;
      query->b_offset = query->a_offset + 45;
      query->c_offset = query->b_offset + 8;
      query->perfcnt_offset = query->c_offset + 8;
      query->rpstat_offset = query->perfcnt_offset + 2;
   } else if (perf->devinfo.verx10 <= 120) {
      query->oa_format = I915_OA_FORMAT_A32u40_A4u32_B8_C8;
      query->gpu_time_offset = 0;
      query->gpu_clock_offset = query->gpu_time_offset + 1;
      query->a_offset = query->gpu_clock_offset + 1;
      query->b_offset = query->a_offset + 36;
      query->c_offset = query->b_offset + 8;
      query->perfcnt_offset = query->c_offset + 8;
      query->rpstat_offset = query->perfcnt_offset + 2;
   } else {
      query->oa_format = I915_OA_FORMAT_A24u40_A14u32_B8_C8;
      query->gpu_time_offset = 0;
      query->gpu_clock_offset = query->gpu_time_offset + 1;
      query->a_offset = query->gpu_clock_offset + 1;
      query->b_offset = query->a_offset + 38;
      query->c_offset = query->b_offset + 8;
      query->perfcnt_offset = query->c_offset + 8;
      query->rpstat_offset = query->perfcnt_offset + 2;
   }

   return query;
}

struct intel_perf_query_counter_data {
   uint32_t name_idx;
   uint32_t desc_idx;
   uint32_t symbol_name_idx;
   uint32_t category_idx;
   enum intel_perf_counter_type type;
   enum intel_perf_counter_data_type data_type;
   enum intel_perf_counter_units units;
};

#endif /* INTEL_PERF_SETUP_H */
