/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
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

#ifndef FREEDRENO_BATCH_H_
#define FREEDRENO_BATCH_H_

#include "util/list.h"
#include "util/simple_mtx.h"
#include "util/u_inlines.h"
#include "util/u_queue.h"
#include "util/perf/u_trace.h"

#include "freedreno_context.h"
#include "freedreno_fence.h"
#include "freedreno_util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fd_resource;
struct fd_batch_key;
struct fd_batch_result;

/**
 * A subpass is a fragment of a batch potentially starting with a clear.
 * If the app does a mid-batch clear, that clear and subsequent draws
 * can be split out into another sub-pass.  At gmem time, the appropriate
 * sysmem or gmem clears can be interleaved with the CP_INDIRECT_BUFFER
 * to the subpass's draw cmdstream.
 *
 * For depth clears, a replacement LRZ buffer can be allocated (clear
 * still inserted into the prologue cmdstream since it needs be executed
 * even in sysmem or if we aren't binning, since later batches could
 * depend in the LRZ state).  The alternative would be to invalidate
 * LRZ for draws after the start of the new subpass.
 */
struct fd_batch_subpass {
   struct list_head node;

   /** draw pass cmdstream: */
   struct fd_ringbuffer *draw;

   /** for the gmem code to stash per tile per subpass clears */
   struct fd_ringbuffer *subpass_clears;

   BITMASK_ENUM(fd_buffer_mask) fast_cleared;

   union pipe_color_union clear_color[MAX_RENDER_TARGETS];
   double clear_depth;
   unsigned clear_stencil;

   /**
    * The number of draws emitted to this subpass.  If it is greater than
    * zero, a clear triggers creating a new subpass (because clears must
    * always come at the start of a subpass).
    */
   unsigned num_draws;

   /**
    * If a subpass starts with a LRZ clear, it gets a new LRZ buffer.
    * The fd_resource::lrz always tracks the current lrz buffer, but at
    * binning/gmem time we need to know what was the current lrz buffer
    * at the time draws were emitted to the subpass.  Which is tracked
    * here.
    */
   struct fd_bo *lrz;
};

/**
 * A batch tracks everything about a cmdstream batch/submit, including the
 * ringbuffers used for binning, draw, and gmem cmds, list of associated
 * fd_resource-s, etc.
 */
struct fd_batch {
   struct pipe_reference reference;
   unsigned seqno;
   unsigned idx; /* index into cache->batches[] */

   struct u_trace trace;

   /* To detect cases where we can skip cmdstream to record timestamp: */
   uint32_t *last_timestamp_cmd;

   int in_fence_fd;
   struct pipe_fence_handle *fence;

   struct fd_context *ctx;

   /* do we need to mem2gmem before rendering.  We don't, if for example,
    * there was a glClear() that invalidated the entire previous buffer
    * contents.  Keep track of which buffer(s) are cleared, or needs
    * restore.  Masks of PIPE_CLEAR_*
    *
    * The 'cleared' bits will be set for buffers which are *entirely*
    * cleared.
    *
    * The 'invalidated' bits are set for cleared buffers, and buffers
    * where the contents are undefined, ie. what we don't need to restore
    * to gmem.
    */
   BITMASK_ENUM(fd_buffer_mask) invalidated, cleared, restore, resolve;

   /* is this a non-draw batch (ie compute/blit which has no pfb state)? */
   bool nondraw : 1;
   bool needs_flush : 1;
   bool flushed : 1;
   bool tessellation : 1; /* tessellation used in batch */

   /* Keep track if WAIT_FOR_IDLE is needed for registers we need
    * to update via RMW:
    */
   bool needs_wfi : 1;

   /* To decide whether to render to system memory, keep track of the
    * number of draws, and whether any of them require multisample,
    * depth_test (or depth write), stencil_test, blending, and
    * color_logic_Op (since those functions are disabled when by-
    * passing GMEM.
    */
   BITMASK_ENUM(fd_gmem_reason) gmem_reason;

   /* At submit time, once we've decided that this batch will use GMEM
    * rendering, the appropriate gmem state is looked up:
    */
   const struct fd_gmem_stateobj *gmem_state;

   /* Driver specific barrier/flush flags: */
   unsigned barrier;

   /* A calculated "draw cost" value for the batch, which tries to
    * estimate the bandwidth-per-sample of all the draws according
    * to:
    *
    *    foreach_draw (...) {
    *      cost += num_mrt;
    *      if (blend_enabled)
    *        cost += num_mrt;
    *      if (depth_test_enabled)
    *        cost++;
    *      if (depth_write_enabled)
    *        cost++;
    *    }
    *
    * The idea is that each sample-passed minimally does one write
    * per MRT.  If blend is enabled, the hw will additionally do
    * a framebuffer read per sample-passed (for each MRT with blend
    * enabled).  If depth-test is enabled, the hw will additionally
    * a depth buffer read.  If depth-write is enable, the hw will
    * additionally do a depth buffer write.
    *
    * This does ignore depth buffer traffic for samples which do not
    * pass do to depth-test fail, and some other details.  But it is
    * just intended to be a rough estimate that is easy to calculate.
    */
   unsigned cost;

   /* Tells the gen specific backend where to write stats used for
    * the autotune module.
    *
    * Pointer only valid during gmem emit code.
    */
   struct fd_batch_result *autotune_result;

   unsigned num_draws;    /* number of draws in current batch */
   unsigned num_vertices; /* number of vertices in current batch */

   /* Currently only used on a6xx, to calculate vsc prim/draw stream
    * sizes:
    */
   unsigned num_bins_per_pipe;
   unsigned prim_strm_bits;
   unsigned draw_strm_bits;

   /* Track the maximal bounds of the scissor of all the draws within a
    * batch.  Used at the tile rendering step (fd_gmem_render_tiles(),
    * mem2gmem/gmem2mem) to avoid needlessly moving data in/out of gmem.
    *
    * Note that unlike gallium state, maxx/maxy are inclusive (for
    * fully covered 512x512 the scissor would be 0,0+511,511)
    */
   struct pipe_scissor_state max_scissor;

   /* Keep track of DRAW initiators that need to be patched up depending
    * on whether we using binning or not:
    */
   struct util_dynarray draw_patches;

   /* texture state that needs patching for fb_read: */
   struct util_dynarray fb_read_patches;

   /* Keep track of writes to RB_RENDER_CONTROL which need to be patched
    * once we know whether or not to use GMEM, and GMEM tile pitch.
    *
    * (only for a3xx.. but having gen specific subclasses of fd_batch
    * seemed overkill for now)
    */
   struct util_dynarray rbrc_patches;

   /* Keep track of GMEM related values that need to be patched up once we
    * know the gmem layout:
    */
   struct util_dynarray gmem_patches;

   /* Keep track of pointer to start of MEM exports for a20x binning shaders
    *
    * this is so the end of the shader can be cut off at the right point
    * depending on the GMEM configuration
    */
   struct util_dynarray shader_patches;

   struct pipe_framebuffer_state framebuffer;

   struct fd_submit *submit;

   /**
    * List of fd_batch_subpass.
    */
   struct list_head subpasses;

#define foreach_subpass(subpass, batch) \
   list_for_each_entry (struct fd_batch_subpass, subpass, &batch->subpasses, node)
#define foreach_subpass_safe(subpass, batch) \
   list_for_each_entry_safe (struct fd_batch_subpass, subpass, &batch->subpasses, node)

   /**
    * The current subpass.
    */
   struct fd_batch_subpass *subpass;

   /**
    * just a reference to the current subpass's draw cmds for backwards compat.
    */
   struct fd_ringbuffer *draw;
   /** binning pass cmdstream: */
   struct fd_ringbuffer *binning;
   /** tiling/gmem (IB0) cmdstream: */
   struct fd_ringbuffer *gmem;

   /** preemble cmdstream (executed once before first tile): */
   struct fd_ringbuffer *prologue;

   /** epilogue cmdstream (executed after each tile): */
   struct fd_ringbuffer *tile_epilogue;

   /** epilogue cmdstream (executed after all tiles): */
   struct fd_ringbuffer *epilogue;

   struct fd_ringbuffer *tile_loads;
   struct fd_ringbuffer *tile_store;

   /**
    * hw query related state:
    */
   /*@{*/
   /* next sample offset.. incremented for each sample in the batch/
    * submit, reset to zero on next submit.
    */
   uint32_t next_sample_offset;

   /* The # of pipeline-stats queries running.  In case of nested
    * queries using {START/STOP}_{PRIMITIVE,FRAGMENT,COMPUTE}_CNTRS,
    * we need to start only on the first one and stop only on the
    * last one.
    */
   uint8_t pipeline_stats_queries_active[3];

   /* cached samples (in case multiple queries need to reference
    * the same sample snapshot)
    */
   struct fd_hw_sample *sample_cache[MAX_HW_SAMPLE_PROVIDERS];

   /* which sample providers were used in the current batch: */
   uint32_t query_providers_used;

   /* which sample providers are currently enabled in the batch: */
   uint32_t query_providers_active;

   /* list of samples in current batch: */
   struct util_dynarray samples;

   /* current query result bo and tile stride: */
   struct pipe_resource *query_buf;
   uint32_t query_tile_stride;
   /*@}*/

   /* Set of resources used by currently-unsubmitted batch (read or
    * write).. does not hold a reference to the resource.
    */
   struct set *resources;

   /** key in batch-cache (if not null): */
   struct fd_batch_key *key;
   uint32_t hash;

   /** set of dependent batches.. holds refs to dependent batches: */
   uint32_t dependents_mask;
};

struct fd_batch *fd_batch_create(struct fd_context *ctx, bool nondraw);

struct fd_batch_subpass *fd_batch_create_subpass(struct fd_batch *batch) assert_dt;

void fd_batch_set_fb(struct fd_batch *batch, const struct pipe_framebuffer_state *pfb) assert_dt;

void fd_batch_flush(struct fd_batch *batch) assert_dt;
bool fd_batch_has_dep(struct fd_batch *batch, struct fd_batch *dep) assert_dt;
void fd_batch_add_dep(struct fd_batch *batch, struct fd_batch *dep) assert_dt;
void fd_batch_resource_write(struct fd_batch *batch,
                             struct fd_resource *rsc) assert_dt;
void fd_batch_resource_read_slowpath(struct fd_batch *batch,
                                     struct fd_resource *rsc) assert_dt;
void fd_batch_check_size(struct fd_batch *batch) assert_dt;

uint32_t fd_batch_key_hash(const void *_key);
bool fd_batch_key_equals(const void *_a, const void *_b);
struct fd_batch_key *fd_batch_key_clone(void *mem_ctx,
                                        const struct fd_batch_key *key);

/* not called directly: */
void __fd_batch_describe(char *buf, const struct fd_batch *batch) assert_dt;
void __fd_batch_destroy_locked(struct fd_batch *batch);
void __fd_batch_destroy(struct fd_batch *batch);

/*
 * NOTE the rule is, you need to hold the screen->lock when destroying
 * a batch..  so either use fd_batch_reference() (which grabs the lock
 * for you) if you don't hold the lock, or fd_batch_reference_locked()
 * if you do hold the lock.
 *
 * WARNING the _locked() version can briefly drop the lock.  Without
 * recursive mutexes, I'm not sure there is much else we can do (since
 * __fd_batch_destroy() needs to unref resources)
 *
 * WARNING you must acquire the screen->lock and use the _locked()
 * version in case that the batch being ref'd can disappear under
 * you.
 */

static inline void
fd_batch_reference_locked(struct fd_batch **ptr, struct fd_batch *batch)
{
   struct fd_batch *old_batch = *ptr;

   /* only need lock if a reference is dropped: */
   if (old_batch)
      fd_screen_assert_locked(old_batch->ctx->screen);

   if (pipe_reference_described(
          &(*ptr)->reference, &batch->reference,
          (debug_reference_descriptor)__fd_batch_describe))
      __fd_batch_destroy_locked(old_batch);

   *ptr = batch;
}

static inline void
fd_batch_reference(struct fd_batch **ptr, struct fd_batch *batch)
{
   struct fd_batch *old_batch = *ptr;

   if (pipe_reference_described(
          &(*ptr)->reference, &batch->reference,
          (debug_reference_descriptor)__fd_batch_describe))
      __fd_batch_destroy(old_batch);

   *ptr = batch;
}

/**
 * Mark the batch as having something worth flushing (rendering, blit, query,
 * etc)
 */
static inline void
fd_batch_needs_flush(struct fd_batch *batch)
{
   batch->needs_flush = true;
   fd_pipe_fence_ref(&batch->ctx->last_fence, NULL);
}

/* Since we reorder batches and can pause/resume queries (notably for disabling
 * queries dueing some meta operations), we update the current query state for
 * the batch before each draw.
 */
static inline void
fd_batch_update_queries(struct fd_batch *batch) assert_dt
{
   struct fd_context *ctx = batch->ctx;

   if (!(ctx->dirty & FD_DIRTY_QUERY))
      return;

   ctx->query_update_batch(batch, false);
}

static inline void
fd_batch_finish_queries(struct fd_batch *batch) assert_dt
{
   struct fd_context *ctx = batch->ctx;

   ctx->query_update_batch(batch, true);
}

static inline void
fd_reset_wfi(struct fd_batch *batch)
{
   batch->needs_wfi = true;
}

void fd_wfi(struct fd_batch *batch, struct fd_ringbuffer *ring) assert_dt;

/* emit a CP_EVENT_WRITE:
 */
static inline void
fd_event_write(struct fd_batch *batch, struct fd_ringbuffer *ring,
               enum vgt_event_type evt)
{
   OUT_PKT3(ring, CP_EVENT_WRITE, 1);
   OUT_RING(ring, evt);
   fd_reset_wfi(batch);
}

/* Get per-tile epilogue */
static inline struct fd_ringbuffer *
fd_batch_get_tile_epilogue(struct fd_batch *batch)
{
   if (batch->tile_epilogue == NULL) {
      batch->tile_epilogue = fd_submit_new_ringbuffer(batch->submit, 0x1000,
                                                 FD_RINGBUFFER_GROWABLE);
   }

   return batch->tile_epilogue;
}

/* Get epilogue run after all tiles*/
static inline struct fd_ringbuffer *
fd_batch_get_epilogue(struct fd_batch *batch)
{
   if (batch->epilogue == NULL) {
      batch->epilogue = fd_submit_new_ringbuffer(batch->submit, 0x1000,
                                                 FD_RINGBUFFER_GROWABLE);
   }

   return batch->epilogue;
}

struct fd_ringbuffer *fd_batch_get_prologue(struct fd_batch *batch);

#ifdef __cplusplus
}
#endif

#endif /* FREEDRENO_BATCH_H_ */
