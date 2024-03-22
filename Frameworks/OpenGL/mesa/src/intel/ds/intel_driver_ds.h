/*
 * Copyright © 2021 Intel Corporation
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
 */

#ifndef INTEL_DRIVER_DS_H
#define INTEL_DRIVER_DS_H

#include <stdint.h>

#include "util/macros.h"
#include "util/perf/u_trace.h"
#include "util/u_vector.h"

#include "dev/intel_device_info.h"

#ifdef __cplusplus
extern "C" {
#endif

enum intel_ds_api {
   INTEL_DS_API_OPENGL,
   INTEL_DS_API_VULKAN,
};

enum intel_ds_stall_flag {
   INTEL_DS_DEPTH_CACHE_FLUSH_BIT            = BITFIELD_BIT(0),
   INTEL_DS_DATA_CACHE_FLUSH_BIT             = BITFIELD_BIT(1),
   INTEL_DS_HDC_PIPELINE_FLUSH_BIT           = BITFIELD_BIT(2),
   INTEL_DS_RENDER_TARGET_CACHE_FLUSH_BIT    = BITFIELD_BIT(3),
   INTEL_DS_TILE_CACHE_FLUSH_BIT             = BITFIELD_BIT(4),
   INTEL_DS_STATE_CACHE_INVALIDATE_BIT       = BITFIELD_BIT(5),
   INTEL_DS_CONST_CACHE_INVALIDATE_BIT       = BITFIELD_BIT(6),
   INTEL_DS_VF_CACHE_INVALIDATE_BIT          = BITFIELD_BIT(7),
   INTEL_DS_TEXTURE_CACHE_INVALIDATE_BIT     = BITFIELD_BIT(8),
   INTEL_DS_INST_CACHE_INVALIDATE_BIT        = BITFIELD_BIT(9),
   INTEL_DS_STALL_AT_SCOREBOARD_BIT          = BITFIELD_BIT(10),
   INTEL_DS_DEPTH_STALL_BIT                  = BITFIELD_BIT(11),
   INTEL_DS_CS_STALL_BIT                     = BITFIELD_BIT(12),
   INTEL_DS_UNTYPED_DATAPORT_CACHE_FLUSH_BIT = BITFIELD_BIT(13),
   INTEL_DS_PSS_STALL_SYNC_BIT               = BITFIELD_BIT(14),
   INTEL_DS_END_OF_PIPE_BIT                  = BITFIELD_BIT(15),
   INTEL_DS_CCS_CACHE_FLUSH_BIT              = BITFIELD_BIT(16),
};

/* Convert internal driver PIPE_CONTROL stall bits to intel_ds_stall_flag. */
typedef enum intel_ds_stall_flag (*intel_ds_stall_cb_t)(uint32_t flags);

enum intel_ds_queue_stage {
   INTEL_DS_QUEUE_STAGE_QUEUE,
   INTEL_DS_QUEUE_STAGE_FRAME,
   INTEL_DS_QUEUE_STAGE_CMD_BUFFER,
   INTEL_DS_QUEUE_STAGE_INTERNAL_OPS,
   INTEL_DS_QUEUE_STAGE_STALL,
   INTEL_DS_QUEUE_STAGE_COMPUTE,
   INTEL_DS_QUEUE_STAGE_AS,
   INTEL_DS_QUEUE_STAGE_RT,
   INTEL_DS_QUEUE_STAGE_RENDER_PASS,
   INTEL_DS_QUEUE_STAGE_BLORP,
   INTEL_DS_QUEUE_STAGE_DRAW,
   INTEL_DS_QUEUE_STAGE_DRAW_MESH,
   INTEL_DS_QUEUE_STAGE_N_STAGES,
};

struct intel_ds_device {
   struct intel_device_info info;

   /* DRM fd */
   int fd;

   /* API of this device */
   enum intel_ds_api api;

   /* GPU identifier (minor number) */
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
    * IntelRenderpassDataSource::Trace)
    */
   uint64_t event_id;

   /* Tracepoint name perfetto identifiers for each of the events. */
   uint64_t tracepoint_iids[96];

   /* Protects submissions of u_trace data to trace_context */
   simple_mtx_t trace_context_mutex;

   struct u_trace_context trace_context;

   /* List of intel_ds_queue */
   struct list_head queues;
};

struct intel_ds_stage {
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

struct intel_ds_queue {
   struct list_head link;

   /* Device this queue belongs to */
   struct intel_ds_device *device;

   /* Unique name of the queue */
   char name[80];

   /* Counter incremented on each intel_ds_end_submit() call */
   uint64_t submission_id;

   struct intel_ds_stage stages[INTEL_DS_QUEUE_STAGE_N_STAGES];
};

struct intel_ds_flush_data {
   struct intel_ds_queue *queue;

   /* u_trace element in which we copy other traces in case we deal with
    * reusable command buffers.
    */
   struct u_trace trace;

   /* Unique submission ID associated with the trace */
   uint64_t submission_id;
};

void intel_driver_ds_init(void);

void intel_ds_device_init(struct intel_ds_device *device,
                          const struct intel_device_info *devinfo,
                          int drm_fd,
                          uint32_t gpu_id,
                          enum intel_ds_api api);
void intel_ds_device_fini(struct intel_ds_device *device);

struct intel_ds_queue *
intel_ds_device_init_queue(struct intel_ds_device *device,
                           struct intel_ds_queue *queue,
                           const char *fmt_name,
                           ...);

void intel_ds_flush_data_init(struct intel_ds_flush_data *data,
                              struct intel_ds_queue *queue,
                              uint64_t submission_id);

void intel_ds_flush_data_fini(struct intel_ds_flush_data *data);

void intel_ds_queue_flush_data(struct intel_ds_queue *queue,
                               struct u_trace *ut,
                               struct intel_ds_flush_data *data,
                               bool free_data);

void intel_ds_device_process(struct intel_ds_device *device, bool eof);

#ifdef HAVE_PERFETTO

uint64_t intel_ds_begin_submit(struct intel_ds_queue *queue);
void intel_ds_end_submit(struct intel_ds_queue *queue,
                         uint64_t start_ts);

#else

static inline uint64_t intel_ds_begin_submit(struct intel_ds_queue *queue)
{
   return 0;
}

static inline void intel_ds_end_submit(struct intel_ds_queue *queue,
                                       uint64_t start_ts)
{
}

#endif /* HAVE_PERFETTO */

#ifdef __cplusplus
}
#endif

#endif /* INTEL_DRIVER_DS_H */
