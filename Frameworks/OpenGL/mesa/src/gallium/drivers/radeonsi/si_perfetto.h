/*
 * Copyright 2023 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SI_PERFETTO_H
#define SI_PERFETTO_H

#include <stdint.h>

#include "util/macros.h"
#include "util/perf/u_trace.h"
#include "util/u_vector.h"

#include "amd/common/ac_gpu_info.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Perfetto collects TracePackets from the application and/or drivers. It is the root object of a 
 * Perfetto trace. A Perfetto trace is a linear sequence of TracePackets.
 * TracePackets contains timestamp and timestamp_clock_id along with lots of other data 
 * like gpu_counter_event and gpu_render_stage_event.
 * gpu_render_stage_event contains data such as event_id, duration, gpu_id, stage_iid, context etc.
 * So a render stage can be named as "draw" which will collect start timestamp and end timestamp 
 * along with other payload data of each draw call from OpenGL
 */

enum amd_ds_api {
   AMD_DS_API_OPENGL,
   AMD_DS_API_VULKAN,
};

enum si_ds_queue_stage {
   SI_DS_QUEUE_STAGE_QUEUE,
   SI_DS_QUEUE_STAGE_COMPUTE,
   SI_DS_QUEUE_STAGE_DRAW,
   SI_DS_QUEUE_STAGE_N_STAGES,
};

struct si_ds_device {
   const struct radeon_info *info;

   /* API of this device */
   enum amd_ds_api api;

   /* GPU identifier domain:bus:device:func:pci_id */
   uint32_t gpu_id;

   /* Clock identifier for this device. */
   uint32_t gpu_clock_id;

   /* The timestamp at the point where we first emitted the clock_sync..
    * this  will be a *later* timestamp that the first GPU traces (since
    * we capture the first clock_sync from the CPU *after* the first GPU
    * tracepoints happen).  To avoid confusing perfetto we need to drop
    * the GPU traces with timestamps before this.
    */
   uint64_t sync_gpu_ts;

   /* Next timestamp after which we should resend a clock correlation. */
   uint64_t next_clock_sync_ns;

   /* Unique perfetto identifier for the context */
   uint64_t iid;

   /* Event ID generator (manipulate only inside
    * SIRenderpassDataSource::Trace)
    */
   uint64_t event_id;

   struct u_trace_context trace_context;

   /* List of si_ds_queue */
   struct list_head queues;
};

struct si_ds_stage {
   /* Unique hw_queue IID */
   uint64_t queue_iid;

   /* Unique stage IID */
   uint64_t stage_iid;

   /* Start timestamp of the last work element. We have a array indexed by
    * level so that we can track multi levels of events (like
    * primary/secondary command buffers).
    */
   uint64_t start_ns[5];

   /* Current number of valid elements in start_ns */
   uint32_t level;
};

struct si_ds_queue {
   struct list_head link;

   /* Device this queue belongs to */
   struct si_ds_device *device;

   /* Unique name of the queue */
   char name[80];

   /* Counter incremented on each si_ds_end_submit() call */
   uint64_t submission_id;

   struct si_ds_stage stages[SI_DS_QUEUE_STAGE_N_STAGES];
};

struct si_ds_flush_data {
   struct si_ds_queue *queue;

   /* u_trace element in which we copy other traces in case we deal with
    * reusable command buffers.
    */
   struct u_trace trace;

   /* Unique submission ID associated with the trace */
   uint64_t submission_id;
};

void si_driver_ds_init(void);

void si_ds_device_init(struct si_ds_device *device, const struct radeon_info *devinfo,
                       uint32_t gpu_id, enum amd_ds_api api);
void si_ds_device_fini(struct si_ds_device *device);

struct si_ds_queue *si_ds_device_init_queue(struct si_ds_device *device, struct si_ds_queue *queue,
                                            const char *fmt_name, ...);

void si_ds_flush_data_init(struct si_ds_flush_data *data, struct si_ds_queue *queue,
                           uint64_t submission_id);

void si_ds_flush_data_fini(struct si_ds_flush_data *data);

#ifdef HAVE_PERFETTO
uint64_t si_ds_begin_submit(struct si_ds_queue *queue);
void si_ds_end_submit(struct si_ds_queue *queue,
                         uint64_t start_ts);

#else
static inline uint64_t si_ds_begin_submit(struct si_ds_queue *queue)
{
   return 0;
}

static inline void si_ds_end_submit(struct si_ds_queue *queue, uint64_t start_ts)
{
}

#endif /* HAVE_PERFETTO */

#ifdef __cplusplus
}
#endif

#endif /* SI_PERFETTO_H */
