/*
 * Copyright © 2021 Google, Inc.
 * SPDX-License-Identifier: MIT
 */

#ifndef TU_PERFETTO_H_
#define TU_PERFETTO_H_

#ifdef HAVE_PERFETTO

/* we can't include tu_common.h because ir3 headers are not C++-compatible */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TU_PERFETTO_MAX_STACK_DEPTH 8

struct tu_device;
struct tu_u_trace_submission_data;

struct tu_perfetto_stage {
   int stage_id;
   /* dynamically allocated stage iid, for app_events.  0 if stage_id should be
    * used instead.
    */
   uint64_t stage_iid;
   uint64_t start_ts;
   const void* payload;
   void* start_payload_function;
};

struct tu_perfetto_state {
   struct tu_perfetto_stage stages[TU_PERFETTO_MAX_STACK_DEPTH];
   unsigned stage_depth;
   unsigned skipped_depth;
};

void tu_perfetto_init(void);

struct tu_perfetto_clocks
{
   uint64_t cpu;
   uint64_t gpu_ts;
   uint64_t gpu_ts_offset;
};

struct tu_perfetto_clocks
tu_perfetto_submit(struct tu_device *dev,
                   uint32_t submission_id,
                   struct tu_perfetto_clocks *clocks);

#ifdef __cplusplus
}
#endif

#endif /* HAVE_PERFETTO */

#endif /* TU_PERFETTO_H_ */
