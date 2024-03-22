/*
 * Copyright © 2020 Google, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _U_TRACE_H
#define _U_TRACE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "util/macros.h"
#include "util/u_atomic.h"
#include "util/u_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A trace mechanism (very) loosely inspired by the linux kernel tracepoint
 * mechanism, in that it allows for defining driver specific (or common)
 * tracepoints, which generate 'trace_$name()' functions that can be
 * called at various points in commandstream emit.
 *
 * Currently a printf backend is implemented, but the expectation is to
 * also implement a perfetto backend for shipping out traces to a tool like
 * AGI.
 *
 * Notable differences:
 *
 *  - GPU timestamps!  A driver provided callback is used to emit timestamps
 *    to a buffer.  At a later point in time (when stalling to wait for the
 *    GPU is not required), the timestamps are re-united with the trace
 *    payload.  This makes the trace mechanism suitable for profiling.
 *
 *  - Instead of a systemwide trace ringbuffer, buffering of un-retired
 *    tracepoints is split into two stages.  Traces are emitted to a
 *    'u_trace' instance, and at a later time flushed to a 'u_trace_context'
 *    instance.  This avoids the requirement that commandstream containing
 *    tracepoints is emitted in the same order as it is generated.
 *
 *    If the hw has multiple parallel "engines" (for example, 3d/blit/compute)
 *    then a `u_trace_context` per-engine should be used.
 *
 *  - Unlike kernel tracepoints, u_trace tracepoints are defined in py
 *    from which header and src files are generated.  Since we already have
 *    a build dependency on python+mako, this gives more flexibility than
 *    clunky preprocessor macro magic.
 *
 */

struct u_trace_context;
struct u_trace;
struct u_trace_chunk;
struct u_trace_printer;

/**
 * Special reserved value to indicate that no timestamp was captured,
 * and that the timestamp of the previous trace should be reused.
 */
#define U_TRACE_NO_TIMESTAMP ((uint64_t) 0)

/**
 * Driver provided callback to create a timestamp buffer which will be
 * read by u_trace_read_ts function.
 */
typedef void *(*u_trace_create_ts_buffer)(struct u_trace_context *utctx,
                                          uint32_t timestamps_count);

/**
 * Driver provided callback to delete a timestamp buffer.
 */
typedef void (*u_trace_delete_ts_buffer)(struct u_trace_context *utctx,
                                         void *timestamps);

/**
 * Driver provided callback to emit commands into the soecified command
 * stream to capture a 64b timestamp into the specified timestamps buffer,
 * at the specified index.
 *
 * The hw counter that the driver records should be something that runs at
 * a fixed rate, even as the GPU freq changes.  The same source used for
 * GL_TIMESTAMP queries should be appropriate.
 */
typedef void (*u_trace_record_ts)(struct u_trace *ut,
                                  void *cs,
                                  void *timestamps,
                                  unsigned idx,
                                  bool end_of_pipe);

/**
 * Driver provided callback to read back a previously recorded timestamp.
 * If necessary, this should block until the GPU has finished writing back
 * the timestamps.  (The timestamps will be read back in order, so it is
 * safe to only synchronize on idx==0.)
 *
 * flush_data is data provided by the driver via u_trace_flush.
 *
 * The returned timestamp should be in units of nanoseconds.  The same
 * timebase as GL_TIMESTAMP queries should be used.
 *
 * The driver can return the special U_TRACE_NO_TIMESTAMP value to indicate
 * that no timestamp was captured and the timestamp from the previous trace
 * will be re-used.  (The first trace in the u_trace buf may not do this.)
 * This allows the driver to detect cases where multiple tracepoints are
 * emitted with no other intervening cmdstream, to avoid pointlessly
 * capturing the same timestamp multiple times in a row.
 */
typedef uint64_t (*u_trace_read_ts)(struct u_trace_context *utctx,
                                    void *timestamps,
                                    unsigned idx,
                                    void *flush_data);

/**
 * Driver provided callback to delete flush data.
 */
typedef void (*u_trace_delete_flush_data)(struct u_trace_context *utctx,
                                          void *flush_data);

enum u_trace_type {
   U_TRACE_TYPE_PRINT = 1u << 0,
   U_TRACE_TYPE_JSON = 1u << 1,
   U_TRACE_TYPE_PERFETTO_ACTIVE = 1u << 2,
   U_TRACE_TYPE_PERFETTO_ENV = 1u << 3,
   U_TRACE_TYPE_MARKERS = 1u << 4,

   U_TRACE_TYPE_PRINT_JSON = U_TRACE_TYPE_PRINT | U_TRACE_TYPE_JSON,
   U_TRACE_TYPE_PERFETTO =
      U_TRACE_TYPE_PERFETTO_ACTIVE | U_TRACE_TYPE_PERFETTO_ENV,

   /*
    * A mask of traces that require appending to the tracepoint chunk list.
    */
   U_TRACE_TYPE_REQUIRE_QUEUING = U_TRACE_TYPE_PRINT | U_TRACE_TYPE_PERFETTO,
   /*
    * A mask of traces that require processing the tracepoint chunk list.
    */
   U_TRACE_TYPE_REQUIRE_PROCESSING =
      U_TRACE_TYPE_PRINT | U_TRACE_TYPE_PERFETTO_ACTIVE,
};

/**
 * The trace context provides tracking for "in-flight" traces, once the
 * cmdstream that records timestamps has been flushed.
 */
struct u_trace_context {
   /* All traces enabled in this context */
   enum u_trace_type enabled_traces;

   void *pctx;

   u_trace_create_ts_buffer create_timestamp_buffer;
   u_trace_delete_ts_buffer delete_timestamp_buffer;
   u_trace_record_ts record_timestamp;
   u_trace_read_ts read_timestamp;
   u_trace_delete_flush_data delete_flush_data;

   FILE *out;
   struct u_trace_printer *out_printer;

   /* Once u_trace_flush() is called u_trace_chunk's are queued up to
    * render tracepoints on a queue.  The per-chunk queue jobs block until
    * timestamps are available.
    */
   struct util_queue queue;

#ifdef HAVE_PERFETTO
   /* node in global list of trace contexts. */
   struct list_head node;
#endif

   /* State to accumulate time across N chunks associated with a single
    * batch (u_trace).
    */
   uint64_t last_time_ns;
   uint64_t first_time_ns;

   uint32_t frame_nr;
   uint32_t batch_nr;
   uint32_t event_nr;
   bool start_of_frame;

   /* list of unprocessed trace chunks in fifo order: */
   struct list_head flushed_trace_chunks;
};

/**
 * The u_trace ptr is passed as the first arg to generated tracepoints.
 * It provides buffering for tracepoint payload until the corresponding
 * driver cmdstream containing the emitted commands to capture is
 * flushed.
 *
 * Individual tracepoints emitted to u_trace are expected to be "executed"
 * (ie. timestamp captured) in FIFO order with respect to other tracepoints
 * emitted to the same u_trace.  But the order WRT other u_trace instances
 * is undefined util u_trace_flush().
 */
struct u_trace {
   struct u_trace_context *utctx;

   uint32_t num_traces;

   struct list_head
      trace_chunks; /* list of unflushed trace chunks in fifo order */
};

void u_trace_context_init(struct u_trace_context *utctx,
                          void *pctx,
                          u_trace_create_ts_buffer create_timestamp_buffer,
                          u_trace_delete_ts_buffer delete_timestamp_buffer,
                          u_trace_record_ts record_timestamp,
                          u_trace_read_ts read_timestamp,
                          u_trace_delete_flush_data delete_flush_data);
void u_trace_context_fini(struct u_trace_context *utctx);

/**
 * Flush (trigger processing) of traces previously flushed to the
 * trace-context by u_trace_flush().
 *
 * This should typically be called in the driver's pctx->flush().
 */
void u_trace_context_process(struct u_trace_context *utctx, bool eof);

void u_trace_init(struct u_trace *ut, struct u_trace_context *utctx);
void u_trace_fini(struct u_trace *ut);

void u_trace_state_init(void);
bool u_trace_is_enabled(enum u_trace_type type);

bool u_trace_has_points(struct u_trace *ut);

struct u_trace_iterator {
   struct u_trace *ut;
   struct u_trace_chunk *chunk;
   uint32_t event_idx;
};

struct u_trace_iterator u_trace_begin_iterator(struct u_trace *ut);

struct u_trace_iterator u_trace_end_iterator(struct u_trace *ut);

bool u_trace_iterator_equal(struct u_trace_iterator a,
                            struct u_trace_iterator b);

typedef void (*u_trace_copy_ts_buffer)(struct u_trace_context *utctx,
                                       void *cmdstream,
                                       void *ts_from,
                                       uint32_t from_offset,
                                       void *ts_to,
                                       uint32_t to_offset,
                                       uint32_t count);

/**
 * Clones tracepoints range into target u_trace.
 * Provides callback for driver to copy timestamps on GPU from
 * one buffer to another.
 *
 * It allows:
 * - Tracing re-usable command buffer in Vulkan, by copying tracepoints
 *   each time it is submitted.
 * - Per-tile tracing for tiling GPUs, by copying a range of tracepoints
 *   corresponding to a tile.
 */
void u_trace_clone_append(struct u_trace_iterator begin_it,
                          struct u_trace_iterator end_it,
                          struct u_trace *into,
                          void *cmdstream,
                          u_trace_copy_ts_buffer copy_ts_buffer);

void u_trace_disable_event_range(struct u_trace_iterator begin_it,
                                 struct u_trace_iterator end_it);

/**
 * Flush traces to the parent trace-context.  At this point, the expectation
 * is that all the tracepoints are "executed" by the GPU following any
 * previously flushed u_trace batch.
 *
 * flush_data is a way for driver to pass additional data, which becomes
 * available only at the point of flush, to the u_trace_read_ts callback and
 * perfetto. The typical example of such data would be a fence to wait on in
 * u_trace_read_ts, and a submission_id to pass into perfetto. The destruction
 * of the data is done via u_trace_delete_flush_data.
 *
 * This should typically be called when the corresponding cmdstream
 * (containing the timestamp reads) is flushed to the kernel.
 */
void u_trace_flush(struct u_trace *ut, void *flush_data, bool free_data);

#ifdef HAVE_PERFETTO
static ALWAYS_INLINE bool
u_trace_perfetto_active(struct u_trace_context *utctx)
{
   return p_atomic_read_relaxed(&utctx->enabled_traces) &
          U_TRACE_TYPE_PERFETTO_ACTIVE;
}

void u_trace_perfetto_start(void);
void u_trace_perfetto_stop(void);
#else
static ALWAYS_INLINE bool
u_trace_perfetto_active(UNUSED struct u_trace_context *utctx)
{
   return false;
}
#endif

/**
 * Return whether utrace is enabled at all or not, this can be used to
 * gate any expensive traces.
 */
static ALWAYS_INLINE bool
u_trace_enabled(struct u_trace_context *utctx)
{
   return p_atomic_read_relaxed(&utctx->enabled_traces) != 0;
}

/**
 * Return whether chunks should be processed or not.
 */
static ALWAYS_INLINE bool
u_trace_should_process(struct u_trace_context *utctx)
{
   return p_atomic_read_relaxed(&utctx->enabled_traces) &
          U_TRACE_TYPE_REQUIRE_PROCESSING;
}

/**
 * Return whether to emit markers into the command stream even if the queue
 * isn't active.
 */
static ALWAYS_INLINE bool
u_trace_markers_enabled(struct u_trace_context *utctx)
{
   return p_atomic_read_relaxed(&utctx->enabled_traces) &
          U_TRACE_TYPE_MARKERS;
}

#ifdef __cplusplus
}
#endif

#endif /* _U_TRACE_H */
