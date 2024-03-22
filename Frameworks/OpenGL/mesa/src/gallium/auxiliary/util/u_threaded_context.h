/**************************************************************************
 *
 * Copyright 2017 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/* This is a wrapper for pipe_context that executes all pipe_context calls
 * in another thread.
 *
 *
 * Guidelines for adopters and deviations from Gallium
 * ---------------------------------------------------
 *
 * 1) pipe_context is wrapped. pipe_screen isn't wrapped. All pipe_screen
 *    driver functions that take a context (fence_finish, texture_get_handle)
 *    should manually unwrap pipe_context by doing:
 *      pipe = threaded_context_unwrap_sync(pipe);
 *
 *    pipe_context::priv is used to unwrap the context, so drivers and state
 *    trackers shouldn't use it.
 *
 *    No other objects are wrapped.
 *
 * 2) Drivers must subclass and initialize these structures:
 *    - threaded_resource for pipe_resource (use threaded_resource_init/deinit)
 *    - threaded_query for pipe_query (zero memory)
 *    - threaded_transfer for pipe_transfer (zero memory)
 *
 * 3) The threaded context must not be enabled for contexts that can use video
 *    codecs.
 *
 * 4) Changes in driver behavior:
 *    - begin_query and end_query always return true; return values from
 *      the driver are ignored.
 *    - generate_mipmap uses is_format_supported to determine success;
 *      the return value from the driver is ignored.
 *    - resource_commit always returns true; failures are ignored.
 *    - set_debug_callback is skipped if the callback is synchronous.
 *
 *
 * Thread-safety requirements on context functions
 * -----------------------------------------------
 *
 * These pipe_context functions are executed directly, so they shouldn't use
 * pipe_context in an unsafe way. They are de-facto screen functions now:
 * - create_query
 * - create_batch_query
 * - create_*_state (all CSOs and shaders)
 *     - Make sure the shader compiler doesn't use any per-context stuff.
 *       (e.g. LLVM target machine)
 *     - Only pipe_context's debug callback for shader dumps is guaranteed to
 *       be up to date, because set_debug_callback synchronizes execution.
 * - create_surface
 * - surface_destroy
 * - create_sampler_view
 * - sampler_view_destroy
 * - stream_output_target_destroy
 * - transfer_map (only unsychronized buffer mappings)
 * - get_query_result (when threaded_query::flushed == true)
 * - create_stream_output_target
 * - get_sample_position
 *
 *
 * Transfer_map rules for buffer mappings
 * --------------------------------------
 *
 * 1) If transfer_map has PIPE_MAP_UNSYNCHRONIZED, the call is made
 *    in the non-driver thread without flushing the queue. The driver will
 *    receive TC_TRANSFER_MAP_THREADED_UNSYNC in addition to PIPE_MAP_-
 *    UNSYNCHRONIZED to indicate this.
 *    Note that transfer_unmap is always enqueued and called from the driver
 *    thread.
 *
 * 2) The driver isn't allowed to infer unsychronized mappings by tracking
 *    the valid buffer range. The threaded context always sends TC_TRANSFER_-
 *    MAP_NO_INFER_UNSYNCHRONIZED to indicate this. Ignoring the flag will lead
 *    to failures.
 *    The threaded context does its own detection of unsynchronized mappings.
 *
 * 3) The driver isn't allowed to do buffer invalidations by itself under any
 *    circumstances. This is necessary for unsychronized maps to map the latest
 *    version of the buffer. (because invalidations can be queued, while
 *    unsychronized maps are not queued and they should return the latest
 *    storage after invalidation). The threaded context always sends
 *    TC_TRANSFER_MAP_NO_INVALIDATE into transfer_map and buffer_subdata to
 *    indicate this. Ignoring the flag will lead to failures.
 *    The threaded context uses its own buffer invalidation mechanism.
 *    Do NOT use pipe_buffer_write, as this may trigger invalidation;
 *    use tc_buffer_write instead.
 *
 * 4) PIPE_MAP_ONCE can no longer be used to infer that a buffer will not be mapped
 *    a second time before it is unmapped.
 *
 *
 * Rules for fences
 * ----------------
 *
 * Flushes will be executed asynchronously in the driver thread if a
 * create_fence callback is provided. This affects fence semantics as follows.
 *
 * When the threaded context wants to perform an asynchronous flush, it will
 * use the create_fence callback to pre-create the fence from the calling
 * thread. This pre-created fence will be passed to pipe_context::flush
 * together with the TC_FLUSH_ASYNC flag.
 *
 * The callback receives the unwrapped context as a parameter, but must use it
 * in a thread-safe way because it is called from a non-driver thread.
 *
 * If the threaded_context does not immediately flush the current batch, the
 * callback also receives a tc_unflushed_batch_token. If fence_finish is called
 * on the returned fence in the context that created the fence,
 * threaded_context_flush must be called.
 *
 * The driver must implement pipe_context::fence_server_sync properly, since
 * the threaded context handles PIPE_FLUSH_ASYNC.
 *
 *
 * Additional requirements
 * -----------------------
 *
 * get_query_result:
 *    If threaded_query::flushed == true, get_query_result should assume that
 *    it's called from a non-driver thread, in which case the driver shouldn't
 *    use the context in an unsafe way.
 *
 * replace_buffer_storage:
 *    The driver has to implement this callback, which will be called when
 *    the threaded context wants to replace a resource's backing storage with
 *    another resource's backing storage. The threaded context uses it to
 *    implement buffer invalidation. This call is always queued.
 *    Note that 'minimum_num_rebinds' specifies only the minimum number of rebinds
 *    which must be managed by the driver; if a buffer is bound multiple times in
 *    the same binding point (e.g., vertex buffer slots 0,1,2), this will be counted
 *    as a single rebind.
 *
 *
 * Optional resource busy callbacks for better performance
 * -------------------------------------------------------
 *
 * This adds checking whether a resource is used by the GPU and whether
 * a resource is referenced by an unflushed command buffer. If neither is true,
 * the threaded context will map the buffer as UNSYNCHRONIZED without flushing
 * or synchronizing the thread and will skip any buffer invalidations
 * (reallocations) because invalidating an idle buffer has no benefit.
 *
 * There are 1 driver callback and 1 TC callback:
 *
 * 1) is_resource_busy: It returns true when a resource is busy. If this is NULL,
 *    the resource is considered always busy.
 *
 * 2) tc_driver_internal_flush_notify: If the driver set
 *    driver_calls_flush_notify = true in threaded_context_create, it should
 *    call this after every internal driver flush. The threaded context uses it
 *    to track internal driver flushes for the purpose of tracking which
 *    buffers are referenced by an unflushed command buffer.
 *
 * If is_resource_busy is set, threaded_resource::buffer_id_unique must be
 * generated by the driver, and the replace_buffer_storage callback should
 * delete the buffer ID passed to it. The driver should use
 * util_idalloc_mt_init_tc.
 *
 *
 * How it works (queue architecture)
 * ---------------------------------
 *
 * There is a multithreaded queue consisting of batches, each batch containing
 * 8-byte slots. Calls can occupy 1 or more slots.
 *
 * Once a batch is full and there is no space for the next call, it's flushed,
 * meaning that it's added to the queue for execution in the other thread.
 * The batches are ordered in a ring and reused once they are idle again.
 * The batching is necessary for low queue/mutex overhead.
 */

#ifndef U_THREADED_CONTEXT_H
#define U_THREADED_CONTEXT_H

#include "c11/threads.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "util/bitset.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_queue.h"
#include "util/u_range.h"
#include "util/u_thread.h"
#include "util/slab.h"
#include "util/u_dynarray.h"

#ifdef __cplusplus
extern "C" {
#endif

struct threaded_context;
struct tc_unflushed_batch_token;

/* 0 = disabled, 1 = assertions, 2 = printfs, 3 = logging */
#define TC_DEBUG 0

/* This is an internal flag not sent to the driver. */
#define TC_TRANSFER_MAP_UPLOAD_CPU_STORAGE   (1u << 28)
/* These are map flags sent to drivers. */
/* Never infer whether it's safe to use unsychronized mappings: */
#define TC_TRANSFER_MAP_NO_INFER_UNSYNCHRONIZED (1u << 29)
/* Don't invalidate buffers: */
#define TC_TRANSFER_MAP_NO_INVALIDATE        (1u << 30)
/* transfer_map is called from a non-driver thread: */
#define TC_TRANSFER_MAP_THREADED_UNSYNC      (1u << 31)

/* Custom flush flags sent to drivers. */
/* fence is pre-populated with a fence created by the create_fence callback */
#define TC_FLUSH_ASYNC        (1u << 31)

/* Size of the queue = number of batch slots in memory.
 * - 1 batch is always idle and records new commands
 * - 1 batch is being executed
 * so the queue size is TC_MAX_BATCHES - 2 = number of waiting batches.
 *
 * Use a size as small as possible for low CPU L2 cache usage but large enough
 * so that the queue isn't stalled too often for not having enough idle batch
 * slots.
 */
#define TC_MAX_BATCHES        10

/* The size of one batch. Non-trivial calls (i.e. not setting a CSO pointer)
 * can occupy multiple call slots.
 *
 * The idea is to have batches as small as possible but large enough so that
 * the queuing and mutex overhead is negligible.
 */
#define TC_SLOTS_PER_BATCH    1536

/* The buffer list queue is much deeper than the batch queue because buffer
 * lists need to stay around until the driver internally flushes its command
 * buffer.
 */
#define TC_MAX_BUFFER_LISTS   (TC_MAX_BATCHES * 4)

/* This mask is used to get a hash of a buffer ID. It's also the bit size of
 * the buffer list - 1. It must be 2^n - 1. The size should be as low as
 * possible to minimize memory usage, but high enough to minimize hash
 * collisions.
 */
#define TC_BUFFER_ID_MASK      BITFIELD_MASK(14)

/* Threshold for when to use the queue or sync. */
#define TC_MAX_STRING_MARKER_BYTES  512

/* Threshold for when to enqueue buffer/texture_subdata as-is.
 * If the upload size is greater than this, it will do instead:
 * - for buffers: DISCARD_RANGE is done by the threaded context
 * - for textures: sync and call the driver directly
 */
#define TC_MAX_SUBDATA_BYTES        320

enum tc_binding_type {
   TC_BINDING_VERTEX_BUFFER,
   TC_BINDING_STREAMOUT_BUFFER,
   TC_BINDING_UBO_VS,
   TC_BINDING_UBO_FS,
   TC_BINDING_UBO_GS,
   TC_BINDING_UBO_TCS,
   TC_BINDING_UBO_TES,
   TC_BINDING_UBO_CS,
   TC_BINDING_SAMPLERVIEW_VS,
   TC_BINDING_SAMPLERVIEW_FS,
   TC_BINDING_SAMPLERVIEW_GS,
   TC_BINDING_SAMPLERVIEW_TCS,
   TC_BINDING_SAMPLERVIEW_TES,
   TC_BINDING_SAMPLERVIEW_CS,
   TC_BINDING_SSBO_VS,
   TC_BINDING_SSBO_FS,
   TC_BINDING_SSBO_GS,
   TC_BINDING_SSBO_TCS,
   TC_BINDING_SSBO_TES,
   TC_BINDING_SSBO_CS,
   TC_BINDING_IMAGE_VS,
   TC_BINDING_IMAGE_FS,
   TC_BINDING_IMAGE_GS,
   TC_BINDING_IMAGE_TCS,
   TC_BINDING_IMAGE_TES,
   TC_BINDING_IMAGE_CS,
};

typedef void (*tc_replace_buffer_storage_func)(struct pipe_context *ctx,
                                               struct pipe_resource *dst,
                                               struct pipe_resource *src,
                                               unsigned minimum_num_rebinds,
                                               uint32_t rebind_mask,
                                               uint32_t delete_buffer_id);
typedef struct pipe_fence_handle *(*tc_create_fence_func)(struct pipe_context *ctx,
                                                          struct tc_unflushed_batch_token *token);
typedef bool (*tc_is_resource_busy)(struct pipe_screen *screen,
                                    struct pipe_resource *resource,
                                    unsigned usage);

struct threaded_resource {
   struct pipe_resource b;

   /* Since buffer invalidations are queued, we can't use the base resource
    * for unsychronized mappings. This points to the latest version of
    * the buffer after the latest invalidation. It's only used for unsychro-
    * nized mappings in the non-driver thread. Initially it's set to &b.
    */
   struct pipe_resource *latest;

   /* Optional CPU storage of the buffer. When we get partial glBufferSubData(implemented by
    * copy_buffer) + glDrawElements, we don't want to drain the gfx pipeline before executing
    * the copy. For ideal pipelining, we upload to this CPU storage and then reallocate
    * the GPU storage completely and reupload everything without copy_buffer.
    */
   void *cpu_storage;

   /* The buffer range which is initialized (with a write transfer, streamout,
    * or writable shader resources). The remainder of the buffer is considered
    * invalid and can be mapped unsynchronized.
    *
    * This allows unsychronized mapping of a buffer range which hasn't been
    * used yet. It's for applications which forget to use the unsynchronized
    * map flag and expect the driver to figure it out.
    *
    * Drivers should set this to the full range for buffers backed by user
    * memory.
    */
   struct util_range valid_buffer_range;

   /* Drivers are required to update this for shared resources and user
    * pointers. */
   bool is_shared;
   bool is_user_ptr;
   bool allow_cpu_storage;

   /* internal tag for tc indicating which batch last touched this resource */
   int8_t last_batch_usage;
   /* for disambiguating last_batch_usage across batch cycles */
   uint32_t batch_generation;

   /* Unique buffer ID. Drivers must set it to non-zero for buffers and it must
    * be unique. Textures must set 0. Low bits are used as a hash of the ID.
    * Use util_idalloc_mt to generate these IDs.
    */
   uint32_t buffer_id_unique;

   /* If positive, then a staging transfer is in progress.
    */
   int pending_staging_uploads;

   /* If staging uploads are pending, this will hold the union of the mapped
    * ranges.
    */
   struct util_range pending_staging_uploads_range;
};

struct threaded_transfer {
   struct pipe_transfer b;

   /* Staging buffer for DISCARD_RANGE transfers. */
   struct pipe_resource *staging;

   /* If b.resource is not the base instance of the buffer, but it's one of its
    * reallocations (set in "latest" of the base instance), this points to
    * the valid range of the base instance. It's used for transfers after
    * a buffer invalidation, because such transfers operate on "latest", not
    * the base instance. Initially it's set to &b.resource->valid_buffer_range.
    */
   struct util_range *valid_buffer_range;

   bool cpu_storage_mapped;
};

struct threaded_query {
   /* The query is added to the list in end_query and removed in flush. */
   struct list_head head_unflushed;

   /* Whether pipe->flush has been called in non-deferred mode after end_query. */
   bool flushed;
};

struct tc_call_base {
#if !defined(NDEBUG) && TC_DEBUG >= 1
   uint32_t sentinel;
#endif
   uint16_t num_slots;
   uint16_t call_id;
};

struct tc_draw_single {
   struct tc_call_base base;
   unsigned index_bias;
   struct pipe_draw_info info;
};

/**
 * A token representing an unflushed batch.
 *
 * See the general rules for fences for an explanation.
 */
struct tc_unflushed_batch_token {
   struct pipe_reference ref;
   struct threaded_context *tc;
};

struct tc_renderpass_info {
   union {
      struct {
         /* bitmask of full-cleared color buffers */
         uint8_t cbuf_clear;
         /* bitmask of not-full-cleared color buffers */
         uint8_t cbuf_load;
         /* bitmask of color buffers that have their stores invalidated */
         uint8_t cbuf_invalidate;
         /* whether the zsbuf is full-cleared */
         bool zsbuf_clear : 1;
         /* whether the zsbuf is partial-cleared */
         bool zsbuf_clear_partial : 1;
         /* whether the zsbuf is not-full-cleared */
         bool zsbuf_load : 1;
         /* whether the zsbuf is invalidated */
         bool zsbuf_invalidate : 1;
         /* whether a draw occurs */
         bool has_draw : 1;
         /* whether a framebuffer resolve occurs on cbuf[0] */
         bool has_resolve : 1;
         /* whether queries are ended during this renderpass */
         bool has_query_ends : 1;
         uint8_t pad : 1;
         /* 32 bits offset */
         /* bitmask of color buffers using fbfetch */
         uint8_t cbuf_fbfetch;
         /* whether the fragment shader writes to the zsbuf */
         bool zsbuf_write_fs : 1;
         /* whether the DSA state writes to the zsbuf */
         bool zsbuf_write_dsa : 1;
         /* whether the DSA state reads the zsbuf */
         bool zsbuf_read_dsa : 1;
         /* whether the zsbuf is used for fbfetch */
         bool zsbuf_fbfetch : 1;
         uint8_t pad2 : 4;
         uint16_t pad3;
      };
      uint64_t data;
      /* fb info is in data32[0] */
      uint32_t data32[2];
      /* cso info is in data16[2] */
      uint16_t data16[4];
      /* zsbuf fb info is in data8[3] */
      uint8_t data8[8];
   };
};

static inline bool
tc_renderpass_info_is_zsbuf_used(const struct tc_renderpass_info *info)
{
   return info->zsbuf_clear ||
          info->zsbuf_clear_partial ||
          info->zsbuf_write_fs ||
          info->zsbuf_write_dsa ||
          info->zsbuf_read_dsa ||
          info->zsbuf_fbfetch;
}

/* if a driver ends a renderpass early for some reason,
 * this function can be called to reset any stored renderpass info
 * to a "safe" state that will avoid data loss on framebuffer attachments
 * 
 * note: ending a renderpass early if invalidate hints are applied will
 * result in data loss
 */
static inline void
tc_renderpass_info_reset(struct tc_renderpass_info *info)
{
   info->data32[0] = 0;
   info->cbuf_load = BITFIELD_MASK(8);
   info->zsbuf_clear_partial = true;
   info->has_draw = true;
   info->has_query_ends = true;
}

struct tc_batch {
   struct threaded_context *tc;
#if !defined(NDEBUG) && TC_DEBUG >= 1
   unsigned sentinel;
#endif
   uint16_t num_total_slots;
   uint16_t buffer_list_index;
   /* the index of the current renderpass info for recording */
   int16_t renderpass_info_idx;
   uint16_t max_renderpass_info_idx;

   /* The last mergeable call that was added to this batch (i.e.
    * buffer subdata). This might be out-of-date or NULL.
    */
   struct tc_call_base *last_mergeable_call;

   struct util_queue_fence fence;
   /* whether the first set_framebuffer_state call has been seen by this batch */
   bool first_set_fb;
   uint8_t batch_idx;
   struct tc_unflushed_batch_token *token;
   uint64_t slots[TC_SLOTS_PER_BATCH];
   struct util_dynarray renderpass_infos;
};

struct tc_buffer_list {
   /* Signalled by the driver after it flushes its internal command buffer. */
   struct util_queue_fence driver_flushed_fence;

   /* Buffer list where bit N means whether ID hash N is in the list. */
   BITSET_DECLARE(buffer_list, TC_BUFFER_ID_MASK + 1);
};

/**
 * Optional TC parameters/callbacks.
 */
struct threaded_context_options {
   tc_create_fence_func create_fence;
   tc_is_resource_busy is_resource_busy;
   bool driver_calls_flush_notify;

   /**
    * If true, ctx->get_device_reset_status() will be called without
    * synchronizing with driver thread.  Drivers can enable this to avoid
    * TC syncs if their implementation of get_device_reset_status() is
    * safe to call without synchronizing with driver thread.
    */
   bool unsynchronized_get_device_reset_status;

   /* If true, create_fence_fd doesn't access the context in the driver. */
   bool unsynchronized_create_fence_fd;
   /* if true, texture_subdata calls may occur unsynchronized with PIPE_MAP_UNSYNCHRONIZED */
   bool unsynchronized_texture_subdata;
   /* if true, parse and track renderpass info during execution */
   bool parse_renderpass_info;
   /* callbacks for drivers to read their DSA/FS state and update renderpass info accordingly
    * note: drivers must ONLY append to renderpass info using |=
    */
   void (*dsa_parse)(void *state, struct tc_renderpass_info *info);
   void (*fs_parse)(void *state, struct tc_renderpass_info *info);
};

struct threaded_context {
   struct pipe_context base;
   struct pipe_context *pipe;
   struct slab_child_pool pool_transfers;
   tc_replace_buffer_storage_func replace_buffer_storage;
   struct threaded_context_options options;
   unsigned map_buffer_alignment;
   unsigned ubo_alignment;

   struct list_head unflushed_queries;

   /* Counters for the HUD. */
   unsigned num_offloaded_slots;
   unsigned num_direct_slots;
   unsigned num_syncs;

   bool use_forced_staging_uploads;
   bool add_all_gfx_bindings_to_buffer_list;
   bool add_all_compute_bindings_to_buffer_list;
   uint8_t num_queries_active;

   /* Estimation of how much vram/gtt bytes are mmap'd in
    * the current tc_batch.
    */
   uint64_t bytes_mapped_estimate;
   uint64_t bytes_mapped_limit;

   struct util_queue queue;
   struct util_queue_fence *fence;

#ifndef NDEBUG
   /**
    * The driver thread is normally the queue thread, but
    * there are cases where the queue is flushed directly
    * from the frontend thread
    */
   thrd_t driver_thread;
#endif

   bool seen_tcs;
   bool seen_tes;
   bool seen_gs;
   /* whether the current renderpass has seen a set_framebuffer_state call */
   bool seen_fb_state;
   /* whether a renderpass is currently active */
   bool in_renderpass;
   /* whether a query has ended more recently than a draw */
   bool query_ended;
   /* whether pipe_context::flush has been called */
   bool flushing;

   bool seen_streamout_buffers;
   bool seen_shader_buffers[PIPE_SHADER_TYPES];
   bool seen_image_buffers[PIPE_SHADER_TYPES];
   bool seen_sampler_buffers[PIPE_SHADER_TYPES];

   int8_t last_completed;

   unsigned max_vertex_buffers;
   unsigned max_const_buffers;
   unsigned max_shader_buffers;
   unsigned max_images;
   unsigned max_samplers;
   unsigned nr_cbufs;

   unsigned last, next, next_buf_list, batch_generation;

   /* The list fences that the driver should signal after the next flush.
    * If this is empty, all driver command buffers have been flushed.
    */
   struct util_queue_fence *signal_fences_next_flush[TC_MAX_BUFFER_LISTS];
   unsigned num_signal_fences_next_flush;

   /* Bound buffers are tracked here using threaded_resource::buffer_id_hash.
    * 0 means unbound.
    */
   uint32_t vertex_buffers[PIPE_MAX_ATTRIBS];
   uint32_t streamout_buffers[PIPE_MAX_SO_BUFFERS];
   uint32_t const_buffers[PIPE_SHADER_TYPES][PIPE_MAX_CONSTANT_BUFFERS];
   uint32_t shader_buffers[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_BUFFERS];
   uint32_t image_buffers[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_IMAGES];
   uint32_t shader_buffers_writeable_mask[PIPE_SHADER_TYPES];
   uint64_t image_buffers_writeable_mask[PIPE_SHADER_TYPES];
   uint32_t sampler_buffers[PIPE_SHADER_TYPES][PIPE_MAX_SHADER_SAMPLER_VIEWS];

   struct tc_batch batch_slots[TC_MAX_BATCHES];
   struct tc_buffer_list buffer_lists[TC_MAX_BUFFER_LISTS];
   /* the current framebuffer attachments; [PIPE_MAX_COLOR_BUFS] is the zsbuf */
   struct pipe_resource *fb_resources[PIPE_MAX_COLOR_BUFS + 1];
   struct pipe_resource *fb_resolve;
   /* accessed by main thread; preserves info across batches */
   struct tc_renderpass_info *renderpass_info_recording;
   /* accessed by driver thread */
   struct tc_renderpass_info *renderpass_info;
};


void threaded_resource_init(struct pipe_resource *res, bool allow_cpu_storage);
void threaded_resource_deinit(struct pipe_resource *res);
struct pipe_context *threaded_context_unwrap_sync(struct pipe_context *pipe);
void tc_driver_internal_flush_notify(struct threaded_context *tc);

/** function for getting the current renderpass info:
 * - renderpass info is always non-null
 *
 * Rules:
 * - threaded context must have been created with parse_renderpass_info=true
 * - must be called after the driver receives a pipe_context::set_framebuffer_state callback
 * - must be called after the driver receives a non-deferrable pipe_context::flush callback
 * - renderpass info must not be used during any internal driver operations (e.g., u_blitter)
 * - must not be called before the driver receives its first pipe_context::set_framebuffer_state callback
 * - renderpass info is invalidated only for non-deferrable flushes and new framebuffer states
 */
const struct tc_renderpass_info *
threaded_context_get_renderpass_info(struct threaded_context *tc);

struct pipe_context *
threaded_context_create(struct pipe_context *pipe,
                        struct slab_parent_pool *parent_transfer_pool,
                        tc_replace_buffer_storage_func replace_buffer,
                        const struct threaded_context_options *options,
                        struct threaded_context **out);

void
threaded_context_init_bytes_mapped_limit(struct threaded_context *tc, unsigned divisor);

void
threaded_context_flush(struct pipe_context *_pipe,
                       struct tc_unflushed_batch_token *token,
                       bool prefer_async);

struct tc_draw_single *
tc_add_draw_single_call(struct pipe_context *_pipe,
                        struct pipe_resource *index_bo);

void
tc_draw_vbo(struct pipe_context *_pipe, const struct pipe_draw_info *info,
            unsigned drawid_offset,
            const struct pipe_draw_indirect_info *indirect,
            const struct pipe_draw_start_count_bias *draws,
            unsigned num_draws);

static inline struct threaded_context *
threaded_context(struct pipe_context *pipe)
{
   return (struct threaded_context*)pipe;
}

static inline struct threaded_resource *
threaded_resource(struct pipe_resource *res)
{
   return (struct threaded_resource*)res;
}

static inline struct threaded_query *
threaded_query(struct pipe_query *q)
{
   return (struct threaded_query*)q;
}

static inline struct threaded_transfer *
threaded_transfer(struct pipe_transfer *transfer)
{
   return (struct threaded_transfer*)transfer;
}

static inline void
tc_unflushed_batch_token_reference(struct tc_unflushed_batch_token **dst,
                                   struct tc_unflushed_batch_token *src)
{
   if (pipe_reference((struct pipe_reference *)*dst, (struct pipe_reference *)src))
      free(*dst);
   *dst = src;
}

/**
 * Helper for !NDEBUG builds to assert that it is called from driver
 * thread.  This is to help drivers ensure that various code-paths
 * are not hit indirectly from pipe entry points that are called from
 * front-end/state-tracker thread.
 */
static inline void
tc_assert_driver_thread(struct threaded_context *tc)
{
   if (!tc)
      return;
#ifndef NDEBUG
   assert(u_thread_is_self(tc->driver_thread));
#endif
}

/**
 * This is called before GPU stores to disable the CPU storage because
 * the CPU storage doesn't mirror the GPU storage.
 *
 * Drivers should also call it before exporting a DMABUF of a buffer.
 */
static inline void
tc_buffer_disable_cpu_storage(struct pipe_resource *buf)
{
   struct threaded_resource *tres = threaded_resource(buf);

   if (tres->cpu_storage) {
      align_free(tres->cpu_storage);
      tres->cpu_storage = NULL;
   }
   tres->allow_cpu_storage = false;
}

static inline void
tc_buffer_write(struct pipe_context *pipe,
                struct pipe_resource *buf,
                unsigned offset,
                unsigned size,
                const void *data)
{
   pipe->buffer_subdata(pipe, buf, PIPE_MAP_WRITE | TC_TRANSFER_MAP_NO_INVALIDATE, offset, size, data);
}

#ifdef __cplusplus
}
#endif

#endif
