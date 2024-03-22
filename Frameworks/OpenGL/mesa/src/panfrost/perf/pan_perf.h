/*
 * Copyright © 2021 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef PAN_PERF_H
#define PAN_PERF_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define PAN_PERF_MAX_CATEGORIES 4
#define PAN_PERF_MAX_COUNTERS   64

struct panfrost_device;
struct panfrost_perf_category;
struct panfrost_perf;

enum panfrost_perf_counter_units {
   PAN_PERF_COUNTER_UNITS_CYCLES,
   PAN_PERF_COUNTER_UNITS_JOBS,
   PAN_PERF_COUNTER_UNITS_TASKS,
   PAN_PERF_COUNTER_UNITS_PRIMITIVES,
   PAN_PERF_COUNTER_UNITS_BEATS,
   PAN_PERF_COUNTER_UNITS_REQUESTS,
   PAN_PERF_COUNTER_UNITS_WARPS,
   PAN_PERF_COUNTER_UNITS_QUADS,
   PAN_PERF_COUNTER_UNITS_TILES,
   PAN_PERF_COUNTER_UNITS_INSTRUCTIONS,
   PAN_PERF_COUNTER_UNITS_TRANSACTIONS,
   PAN_PERF_COUNTER_UNITS_THREADS,
   PAN_PERF_COUNTER_UNITS_BYTES,
   PAN_PERF_COUNTER_UNITS_PIXELS,
   PAN_PERF_COUNTER_UNITS_ISSUES,
};

struct panfrost_perf_counter {
   const char *name;
   const char *desc;
   const char *symbol_name;
   enum panfrost_perf_counter_units units;
   // Offset of this counter's value within the category
   uint32_t offset;
   unsigned category_index;
};

struct panfrost_perf_category {
   const char *name;

   struct panfrost_perf_counter counters[PAN_PERF_MAX_COUNTERS];
   uint32_t n_counters;

   /* Offset of this category within the counters memory block */
   unsigned offset;
};

struct panfrost_perf_config {
   const char *name;

   struct panfrost_perf_category categories[PAN_PERF_MAX_CATEGORIES];
   uint32_t n_categories;
};

struct panfrost_perf {
   struct panfrost_device *dev;

   const struct panfrost_perf_config *cfg;

   // Memory where to dump counter values
   uint32_t *counter_values;
   uint32_t n_counter_values;

   /* Offsets of categories */
   unsigned category_offset[PAN_PERF_MAX_CATEGORIES];
};

uint32_t panfrost_perf_counter_read(const struct panfrost_perf_counter *counter,
                                    const struct panfrost_perf *perf);

void panfrost_perf_init(struct panfrost_perf *perf,
                        struct panfrost_device *dev);

int panfrost_perf_enable(struct panfrost_perf *perf);

int panfrost_perf_disable(struct panfrost_perf *perf);

int panfrost_perf_dump(struct panfrost_perf *perf);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // PAN_PERF_H
