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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "iris_batch.h"
#include "iris_context.h"
#include "iris_utrace.h"

#include "util/u_trace_gallium.h"

#include "ds/intel_driver_ds.h"

#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/** Timestamp structure format */
union iris_utrace_timestamp {
   /* Timestamp writtem by either 2 * MI_STORE_REGISTER_MEM or
    * PIPE_CONTROL.
    */
   uint64_t timestamp;

   /* Timestamp written by COMPUTE_WALKER::PostSync
    *
    * Layout is described in PRMs.
    * ATSM PRMs, Volume 2d: Command Reference: Structures, POSTSYNC_DATA:
    *
    *    "The timestamp layout :
    *        [0] = 32b Context Timestamp Start
    *        [1] = 32b Global Timestamp Start
    *        [2] = 32b Context Timestamp End
    *        [3] = 32b Global Timestamp End"
    */
   uint32_t compute_walker[4];
};

static void *
iris_utrace_create_ts_buffer(struct u_trace_context *utctx, uint32_t size)
{
   struct iris_context *ice =
      container_of(utctx, struct iris_context, ds.trace_context);
   struct pipe_context *ctx = &ice->ctx;
   struct iris_screen *screen = (struct iris_screen *)ctx->screen;
   uint32_t iris_size =
      (size / sizeof(uint64_t)) * sizeof(union iris_utrace_timestamp);

   struct iris_bo *bo =
      iris_bo_alloc(screen->bufmgr, "utrace timestamps",
                    iris_size, 16 /* alignment */,
                    IRIS_MEMZONE_OTHER,
                    BO_ALLOC_COHERENT | BO_ALLOC_SMEM);

   void *ptr = iris_bo_map(NULL, bo, MAP_READ | MAP_WRITE);
   memset(ptr, 0, iris_size);

   return bo;
}

static void
iris_utrace_delete_ts_buffer(struct u_trace_context *utctx, void *timestamps)
{
   struct iris_bo *bo = timestamps;
   iris_bo_unreference(bo);
}

static void
iris_utrace_record_ts(struct u_trace *trace, void *cs,
                      void *timestamps, unsigned idx,
                      bool end_of_pipe)
{
   struct iris_batch *batch = container_of(trace, struct iris_batch, trace);
   struct iris_context *ice = batch->ice;
   struct iris_bo *bo = timestamps;
   uint32_t ts_offset = idx * sizeof(union iris_utrace_timestamp);

   iris_use_pinned_bo(batch, bo, true, IRIS_DOMAIN_NONE);

   const bool is_end_compute =
      (cs == NULL && ice->utrace.last_compute_walker != NULL && end_of_pipe);
   if (is_end_compute) {
      batch->screen->vtbl.rewrite_compute_walker_pc(
         batch, ice->utrace.last_compute_walker, bo, ts_offset);
      ice->utrace.last_compute_walker = NULL;
   } else if (end_of_pipe) {
      iris_emit_pipe_control_write(batch, "query: pipelined snapshot write",
                                   PIPE_CONTROL_WRITE_TIMESTAMP,
                                   bo, ts_offset, 0ull);
   } else {
      batch->screen->vtbl.store_register_mem64(batch, 0x2358,
                                               bo, ts_offset,
                                               false);
   }
}

static uint64_t
iris_utrace_read_ts(struct u_trace_context *utctx,
                    void *timestamps, unsigned idx, void *flush_data)
{
   struct iris_context *ice =
      container_of(utctx, struct iris_context, ds.trace_context);
   struct pipe_context *ctx = &ice->ctx;
   struct iris_screen *screen = (struct iris_screen *)ctx->screen;
   struct iris_bo *bo = timestamps;

   if (idx == 0)
      iris_bo_wait_rendering(bo);

   union iris_utrace_timestamp *ts = iris_bo_map(NULL, bo, MAP_READ);

   /* Don't translate the no-timestamp marker: */
   if (ts[idx].timestamp == U_TRACE_NO_TIMESTAMP)
      return U_TRACE_NO_TIMESTAMP;

   /* Detect a 16bytes timestamp write */
   if (ts[idx].compute_walker[2] != 0 || ts[idx].compute_walker[3] != 0) {
      /* The timestamp written by COMPUTE_WALKER::PostSync only as 32bits. We
       * need to rebuild the full 64bits using the previous timestamp. We
       * assume that utrace is reading the timestamp in order. Anyway
       * timestamp rollover on 32bits in a few minutes so in most cases that
       * should be correct.
       */
      uint64_t timestamp =
         (ice->utrace.last_full_timestamp & 0xffffffff00000000) |
         (uint64_t) ts[idx].compute_walker[3];

      return intel_device_info_timebase_scale(screen->devinfo, timestamp);
   }

   ice->utrace.last_full_timestamp = ts[idx].timestamp;

   return intel_device_info_timebase_scale(screen->devinfo, ts[idx].timestamp);
}

static void
iris_utrace_delete_flush_data(struct u_trace_context *utctx,
                              void *flush_data)
{
   free(flush_data);
}

void iris_utrace_flush(struct iris_batch *batch, uint64_t submission_id)
{
   struct intel_ds_flush_data *flush_data = malloc(sizeof(*flush_data));
   intel_ds_flush_data_init(flush_data, &batch->ds, submission_id);
   intel_ds_queue_flush_data(&batch->ds, &batch->trace, flush_data, false);
}

void iris_utrace_init(struct iris_context *ice)
{
   struct iris_screen *screen = (struct iris_screen *)ice->ctx.screen;

   struct stat st;
   uint32_t minor;

   if (fstat(screen->fd, &st) == 0)
      minor = minor(st.st_rdev);
   else
      minor = 0;

   intel_ds_device_init(&ice->ds, screen->devinfo, screen->fd, minor,
                        INTEL_DS_API_OPENGL);

   u_trace_context_init(&ice->ds.trace_context, &ice->ctx,
                        iris_utrace_create_ts_buffer,
                        iris_utrace_delete_ts_buffer,
                        iris_utrace_record_ts,
                        iris_utrace_read_ts,
                        iris_utrace_delete_flush_data);

   for (int i = 0; i < IRIS_BATCH_COUNT; i++) {
      intel_ds_device_init_queue(&ice->ds, &ice->batches[i].ds, "%s",
                                 iris_batch_name_to_string(i));
   }
}

void iris_utrace_fini(struct iris_context *ice)
{
   intel_ds_device_fini(&ice->ds);
}

enum intel_ds_stall_flag
iris_utrace_pipe_flush_bit_to_ds_stall_flag(uint32_t flags)
{
   static const struct {
      uint32_t iris;
      enum intel_ds_stall_flag ds;
   } iris_to_ds_flags[] = {
      { .iris = PIPE_CONTROL_DEPTH_CACHE_FLUSH,            .ds = INTEL_DS_DEPTH_CACHE_FLUSH_BIT, },
      { .iris = PIPE_CONTROL_DATA_CACHE_FLUSH,             .ds = INTEL_DS_DATA_CACHE_FLUSH_BIT, },
      { .iris = PIPE_CONTROL_TILE_CACHE_FLUSH,             .ds = INTEL_DS_TILE_CACHE_FLUSH_BIT, },
      { .iris = PIPE_CONTROL_RENDER_TARGET_FLUSH,          .ds = INTEL_DS_RENDER_TARGET_CACHE_FLUSH_BIT, },
      { .iris = PIPE_CONTROL_STATE_CACHE_INVALIDATE,       .ds = INTEL_DS_STATE_CACHE_INVALIDATE_BIT, },
      { .iris = PIPE_CONTROL_CONST_CACHE_INVALIDATE,       .ds = INTEL_DS_CONST_CACHE_INVALIDATE_BIT, },
      { .iris = PIPE_CONTROL_VF_CACHE_INVALIDATE,          .ds = INTEL_DS_VF_CACHE_INVALIDATE_BIT, },
      { .iris = PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE,     .ds = INTEL_DS_TEXTURE_CACHE_INVALIDATE_BIT, },
      { .iris = PIPE_CONTROL_INSTRUCTION_INVALIDATE,       .ds = INTEL_DS_INST_CACHE_INVALIDATE_BIT, },
      { .iris = PIPE_CONTROL_DEPTH_STALL,                  .ds = INTEL_DS_DEPTH_STALL_BIT, },
      { .iris = PIPE_CONTROL_CS_STALL,                     .ds = INTEL_DS_CS_STALL_BIT, },
      { .iris = PIPE_CONTROL_FLUSH_HDC,                    .ds = INTEL_DS_HDC_PIPELINE_FLUSH_BIT, },
      { .iris = PIPE_CONTROL_STALL_AT_SCOREBOARD,          .ds = INTEL_DS_STALL_AT_SCOREBOARD_BIT, },
      { .iris = PIPE_CONTROL_UNTYPED_DATAPORT_CACHE_FLUSH, .ds = INTEL_DS_UNTYPED_DATAPORT_CACHE_FLUSH_BIT, },
      { .iris = PIPE_CONTROL_CCS_CACHE_FLUSH,              .ds = INTEL_DS_CCS_CACHE_FLUSH_BIT, },
   };

   enum intel_ds_stall_flag ret = 0;
   for (uint32_t i = 0; i < ARRAY_SIZE(iris_to_ds_flags); i++) {
      if (iris_to_ds_flags[i].iris & flags)
         ret |= iris_to_ds_flags[i].ds;
   }

   assert(ret != 0);

   return ret;
}
