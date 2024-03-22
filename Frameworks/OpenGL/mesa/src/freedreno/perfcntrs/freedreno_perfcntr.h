/*
 * Copyright (C) 2018 Rob Clark <robclark@freedesktop.org>
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

#ifndef FREEDRENO_PERFCNTR_H_
#define FREEDRENO_PERFCNTR_H_

#include "util/macros.h"

#include "freedreno_dev_info.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Mapping very closely to the AMD_performance_monitor extension, adreno has
 * groups of performance counters where each group has N counters, which can
 * select from M different countables (things that can be counted), where
 * generally M > N.
 */

/* Describes a single counter: */
struct fd_perfcntr_counter {
   /* offset of the select register to choose what to count: */
   unsigned select_reg;
   /* offset of the lo/hi 32b to read current counter value: */
   unsigned counter_reg_lo;
   unsigned counter_reg_hi;
   /* Optional, most counters don't have enable/clear registers: */
   unsigned enable;
   unsigned clear;
};

enum fd_perfcntr_type {
   FD_PERFCNTR_TYPE_UINT64,
   FD_PERFCNTR_TYPE_UINT,
   FD_PERFCNTR_TYPE_FLOAT,
   FD_PERFCNTR_TYPE_PERCENTAGE,
   FD_PERFCNTR_TYPE_BYTES,
   FD_PERFCNTR_TYPE_MICROSECONDS,
   FD_PERFCNTR_TYPE_HZ,
   FD_PERFCNTR_TYPE_DBM,
   FD_PERFCNTR_TYPE_TEMPERATURE,
   FD_PERFCNTR_TYPE_VOLTS,
   FD_PERFCNTR_TYPE_AMPS,
   FD_PERFCNTR_TYPE_WATTS,
};

/* Whether an average value per frame or a cumulative value should be
 * displayed.
 */
enum fd_perfcntr_result_type {
   FD_PERFCNTR_RESULT_TYPE_AVERAGE,
   FD_PERFCNTR_RESULT_TYPE_CUMULATIVE,
};

/* Describes a single countable: */
struct fd_perfcntr_countable {
   const char *name;
   /* selector register enum value to select this countable: */
   unsigned selector;

   /* description of the countable: */
   enum fd_perfcntr_type query_type;
   enum fd_perfcntr_result_type result_type;
};

/* Describes an entire counter group: */
struct fd_perfcntr_group {
   const char *name;
   unsigned num_counters;
   const struct fd_perfcntr_counter *counters;
   unsigned num_countables;
   const struct fd_perfcntr_countable *countables;
};

const struct fd_perfcntr_group *fd_perfcntrs(const struct fd_dev_id *id, unsigned *count);

#define COUNTER(_sel, _lo, _hi) {                                              \
      .select_reg = REG(_sel), .counter_reg_lo = REG(_lo),                     \
      .counter_reg_hi = REG(_hi),                                              \
   }

#define COUNTER2(_sel, _lo, _hi, _en, _clr) {                                  \
      .select_reg = REG(_sel), .counter_reg_lo = REG(_lo),                     \
      .counter_reg_hi = REG(_hi), .enable = REG(_en), .clear = REG(_clr),      \
   }

#define COUNTABLE(_selector, _query_type, _result_type) {                      \
      .name = #_selector, .selector = _selector,                               \
      .query_type = FD_PERFCNTR_TYPE_##_query_type,                            \
      .result_type = FD_PERFCNTR_RESULT_TYPE_##_result_type,                   \
   }

#define GROUP(_name, _counters, _countables) {                                 \
      .name = _name, .num_counters = ARRAY_SIZE(_counters),                    \
      .counters = _counters, .num_countables = ARRAY_SIZE(_countables),        \
      .countables = _countables,                                               \
   }

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* FREEDRENO_PERFCNTR_H_ */
