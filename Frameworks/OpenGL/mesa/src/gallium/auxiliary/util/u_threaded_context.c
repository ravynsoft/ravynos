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

#include "util/u_threaded_context.h"
#include "util/u_cpu_detect.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_upload_mgr.h"
#include "driver_trace/tr_context.h"
#include "util/log.h"
#include "util/perf/cpu_trace.h"
#include "compiler/shader_info.h"

#if TC_DEBUG >= 1
#define tc_assert assert
#else
#define tc_assert(x)
#endif

#if TC_DEBUG >= 2
#define tc_printf mesa_logi
#define tc_asprintf asprintf
#define tc_strcmp strcmp
#else
#define tc_printf(...)
#define tc_asprintf(...) 0
#define tc_strcmp(...) 0
#endif

#define TC_SENTINEL 0x5ca1ab1e

enum tc_call_id {
#define CALL(name) TC_CALL_##name,
#include "u_threaded_context_calls.h"
#undef CALL
   TC_NUM_CALLS,
};

#if TC_DEBUG >= 3 || defined(TC_TRACE)
static const char *tc_call_names[] = {
#define CALL(name) #name,
#include "u_threaded_context_calls.h"
#undef CALL
};
#endif

#ifdef TC_TRACE
#  define TC_TRACE_SCOPE(call_id) MESA_TRACE_SCOPE(tc_call_names[call_id])
#else
#  define TC_TRACE_SCOPE(call_id)
#endif

typedef uint16_t (*tc_execute)(struct pipe_context *pipe, void *call);

static const tc_execute execute_func[TC_NUM_CALLS];

static void
tc_buffer_subdata(struct pipe_context *_pipe,
                  struct pipe_resource *resource,
                  unsigned usage, unsigned offset,
                  unsigned size, const void *data);

static void
tc_batch_check(UNUSED struct tc_batch *batch)
{
   tc_assert(batch->sentinel == TC_SENTINEL);
   tc_assert(batch->num_total_slots <= TC_SLOTS_PER_BATCH);
}

static void
tc_debug_check(struct threaded_context *tc)
{
   for (unsigned i = 0; i < TC_MAX_BATCHES; i++) {
      tc_batch_check(&tc->batch_slots[i]);
      tc_assert(tc->batch_slots[i].tc == tc);
   }
}

static void
tc_set_driver_thread(struct threaded_context *tc)
{
#ifndef NDEBUG
   tc->driver_thread = thrd_current();
#endif
}

static void
tc_clear_driver_thread(struct threaded_context *tc)
{
#ifndef NDEBUG
   memset(&tc->driver_thread, 0, sizeof(tc->driver_thread));
#endif
}

struct tc_batch_rp_info {
   /* this is what drivers can see */
   struct tc_renderpass_info info;
   /* determines whether the info can be "safely" read by drivers or if it may still be in use */
   struct util_queue_fence ready;
   /* when a batch is full, the rp info rollsover onto 'next' */
   struct tc_batch_rp_info *next;
   /* when rp info has rolled over onto this struct, 'prev' is used to update pointers for realloc */
   struct tc_batch_rp_info *prev;
};

static struct tc_batch_rp_info *
tc_batch_rp_info(struct tc_renderpass_info *info)
{
   return (struct tc_batch_rp_info *)info;
}

static void
tc_sanitize_renderpass_info(struct threaded_context *tc)
{
   tc->renderpass_info_recording->cbuf_invalidate = 0;
   tc->renderpass_info_recording->zsbuf_invalidate = false;
   tc->renderpass_info_recording->cbuf_load |= (~tc->renderpass_info_recording->cbuf_clear) & BITFIELD_MASK(PIPE_MAX_COLOR_BUFS);
   if (tc->fb_resources[PIPE_MAX_COLOR_BUFS] && !tc_renderpass_info_is_zsbuf_used(tc->renderpass_info_recording))
      /* this should be a "safe" way to indicate to the driver that both loads and stores are required;
      * driver can always detect invalidation
      */
      tc->renderpass_info_recording->zsbuf_clear_partial = true;
   if (tc->num_queries_active)
      tc->renderpass_info_recording->has_query_ends = true;
}

/* ensure the batch's array of renderpass data is large enough for the current index */
static void
tc_batch_renderpass_infos_resize(struct threaded_context *tc, struct tc_batch *batch)
{
   unsigned size = batch->renderpass_infos.capacity;
   unsigned cur_num = MAX2(batch->renderpass_info_idx, 0);

   if (size / sizeof(struct tc_batch_rp_info) > cur_num)
      return;

   struct tc_batch_rp_info *infos = batch->renderpass_infos.data;
   unsigned old_idx = batch->renderpass_info_idx - 1;
   bool redo = tc->renderpass_info_recording &&
               tc->renderpass_info_recording == &infos[old_idx].info;
   if (!util_dynarray_resize(&batch->renderpass_infos, struct tc_batch_rp_info, cur_num + 10))
      mesa_loge("tc: memory alloc fail!");

   if (size != batch->renderpass_infos.capacity) {
      /* zero new allocation region */
      uint8_t *data = batch->renderpass_infos.data;
      memset(data + size, 0, batch->renderpass_infos.capacity - size);
      unsigned start = size / sizeof(struct tc_batch_rp_info);
      unsigned count = (batch->renderpass_infos.capacity - size) /
                       sizeof(struct tc_batch_rp_info);
      infos = batch->renderpass_infos.data;
      if (infos->prev)
         infos->prev->next = infos;
      for (unsigned i = 0; i < count; i++)
         util_queue_fence_init(&infos[start + i].ready);
      /* re-set current recording info on resize */
      if (redo)
         tc->renderpass_info_recording = &infos[old_idx].info;
   }
}

/* signal that the renderpass info is "ready" for use by drivers and will no longer be updated */
static void
tc_signal_renderpass_info_ready(struct threaded_context *tc)
{
   if (tc->renderpass_info_recording &&
       !util_queue_fence_is_signalled(&tc_batch_rp_info(tc->renderpass_info_recording)->ready))
      util_queue_fence_signal(&tc_batch_rp_info(tc->renderpass_info_recording)->ready);
}

/* increment the current renderpass info struct for recording
 * 'full_copy' is used for preserving data across non-blocking tc batch flushes
 */
static void
tc_batch_increment_renderpass_info(struct threaded_context *tc, unsigned batch_idx, bool full_copy)
{
   struct tc_batch *batch = &tc->batch_slots[batch_idx];
   struct tc_batch_rp_info *tc_info = batch->renderpass_infos.data;

   if (tc_info[0].next || batch->num_total_slots) {
      /* deadlock condition detected: all batches are in flight, renderpass hasn't ended
       * (probably a cts case)
       */
      struct tc_batch_rp_info *info = tc_batch_rp_info(tc->renderpass_info_recording);
      if (!util_queue_fence_is_signalled(&info->ready)) {
         /* this batch is actively executing and the driver is waiting on the recording fence to signal */
         /* force all buffer usage to avoid data loss */
         info->info.cbuf_load = ~(BITFIELD_MASK(8) & info->info.cbuf_clear);
         info->info.zsbuf_clear_partial = true;
         info->info.has_query_ends = tc->num_queries_active > 0;
         /* ensure threaded_context_get_renderpass_info() won't deadlock */
         info->next = NULL;
         util_queue_fence_signal(&info->ready);
      }
      /* always wait on the batch to finish since this will otherwise overwrite thread data */
      util_queue_fence_wait(&batch->fence);
   }
   /* increment rp info and initialize it */
   batch->renderpass_info_idx++;
   tc_batch_renderpass_infos_resize(tc, batch);
   tc_info = batch->renderpass_infos.data;

   if (full_copy) {
      /* this should only be called when changing batches */
      assert(batch->renderpass_info_idx == 0);
      /* copy the previous data in its entirety: this is still the same renderpass */
      if (tc->renderpass_info_recording) {
         tc_info[batch->renderpass_info_idx].info.data = tc->renderpass_info_recording->data;
         tc_batch_rp_info(tc->renderpass_info_recording)->next = &tc_info[batch->renderpass_info_idx];
         tc_info[batch->renderpass_info_idx].prev = tc_batch_rp_info(tc->renderpass_info_recording);
         /* guard against deadlock scenario */
         assert(&tc_batch_rp_info(tc->renderpass_info_recording)->next->info != tc->renderpass_info_recording);
      } else {
         tc_info[batch->renderpass_info_idx].info.data = 0;
         tc_info[batch->renderpass_info_idx].prev = NULL;
      }
   } else {
      /* selectively copy: only the CSO metadata is copied, and a new framebuffer state will be added later */
      tc_info[batch->renderpass_info_idx].info.data = 0;
      if (tc->renderpass_info_recording) {
         tc_info[batch->renderpass_info_idx].info.data16[2] = tc->renderpass_info_recording->data16[2];
         tc_batch_rp_info(tc->renderpass_info_recording)->next = NULL;
         tc_info[batch->renderpass_info_idx].prev = NULL;
      }
   }

   assert(!full_copy || !tc->renderpass_info_recording || tc_batch_rp_info(tc->renderpass_info_recording)->next);
   /* signal existing info since it will not be used anymore */
   tc_signal_renderpass_info_ready(tc);
   util_queue_fence_reset(&tc_info[batch->renderpass_info_idx].ready);
   /* guard against deadlock scenario */
   assert(tc->renderpass_info_recording != &tc_info[batch->renderpass_info_idx].info);
   /* this is now the current recording renderpass info */
   tc->renderpass_info_recording = &tc_info[batch->renderpass_info_idx].info;
   batch->max_renderpass_info_idx = batch->renderpass_info_idx;
}

static ALWAYS_INLINE struct tc_renderpass_info *
tc_get_renderpass_info(struct threaded_context *tc)
{
   return tc->renderpass_info_recording;
}

/* update metadata at draw time */
static void
tc_parse_draw(struct threaded_context *tc)
{
   struct tc_renderpass_info *info = tc_get_renderpass_info(tc);

   if (info) {
      /* all buffers that aren't cleared are considered loaded */
      info->cbuf_load |= ~info->cbuf_clear;
      if (!info->zsbuf_clear)
         info->zsbuf_load = true;
      /* previous invalidates are no longer relevant */
      info->cbuf_invalidate = 0;
      info->zsbuf_invalidate = false;
      info->has_draw = true;
      info->has_query_ends |= tc->query_ended;
   }

   tc->in_renderpass = true;
   tc->seen_fb_state = true;
   tc->query_ended = false;
}

static void *
to_call_check(void *ptr, unsigned num_slots)
{
#if TC_DEBUG >= 1
   struct tc_call_base *call = ptr;
   tc_assert(call->num_slots == num_slots);
#endif
   return ptr;
}
#define to_call(ptr, type) ((struct type *)to_call_check((void *)(ptr), call_size(type)))

#define size_to_slots(size)      DIV_ROUND_UP(size, 8)
#define call_size(type)          size_to_slots(sizeof(struct type))
#define call_size_with_slots(type, num_slots) size_to_slots( \
   sizeof(struct type) + sizeof(((struct type*)NULL)->slot[0]) * (num_slots))
#define get_next_call(ptr, type) ((struct type*)((uint64_t*)ptr + call_size(type)))

ALWAYS_INLINE static void
tc_set_resource_batch_usage(struct threaded_context *tc, struct pipe_resource *pres)
{
   /* ignore batch usage when persistent */
   if (threaded_resource(pres)->last_batch_usage != INT8_MAX)
      threaded_resource(pres)->last_batch_usage = tc->next;
   threaded_resource(pres)->batch_generation = tc->batch_generation;
}

ALWAYS_INLINE static void
tc_set_resource_batch_usage_persistent(struct threaded_context *tc, struct pipe_resource *pres, bool enable)
{
   if (!pres)
      return;
   /* mark with special value to block any unsynchronized access */
   threaded_resource(pres)->last_batch_usage = enable ? INT8_MAX : tc->next;
   threaded_resource(pres)->batch_generation = tc->batch_generation;
}

/* this can ONLY be used to check against the currently recording batch */
ALWAYS_INLINE static bool
tc_resource_batch_usage_test_busy(const struct threaded_context *tc, const struct pipe_resource *pres)
{
   const struct threaded_resource *tbuf = (const struct threaded_resource*)pres;

   if (!tc->options.unsynchronized_texture_subdata)
      return true;

   /* resource has persistent access: assume always busy */
   if (tbuf->last_batch_usage == INT8_MAX)
      return true;

   /* resource has never been seen */
   if (tbuf->last_batch_usage == -1)
      return false;

   /* resource has been seen but no batches have executed */
   if (tc->last_completed == -1)
      return true;

   /* begin comparisons checking number of times batches have cycled */
   unsigned diff = tc->batch_generation - tbuf->batch_generation;
   /* resource has been seen, batches have fully cycled at least once */
   if (diff > 1)
      return false;

   /* resource has been seen in current batch cycle: return whether batch has definitely completed */
   if (diff == 0)
      return tc->last_completed >= tbuf->last_batch_usage;

   /* resource has been seen within one batch cycle: check for batch wrapping */
   if (tc->last_completed >= tbuf->last_batch_usage)
      /* this or a subsequent pre-wrap batch was the last to definitely complete: resource is idle */
      return false;

   /* batch execution has not definitely wrapped: resource is definitely not idle */
   if (tc->last_completed > tc->next)
      return true;

   /* resource was seen pre-wrap, batch execution has definitely wrapped: idle */
   if (tbuf->last_batch_usage > tc->last_completed)
      return false;

   /* tc->last_completed is not an exact measurement, so anything else is considered busy */
   return true;
}

/* Assign src to dst while dst is uninitialized. */
static inline void
tc_set_resource_reference(struct pipe_resource **dst, struct pipe_resource *src)
{
   *dst = src;
   pipe_reference(NULL, &src->reference); /* only increment refcount */
}

/* Assign src to dst while dst is uninitialized. */
static inline void
tc_set_vertex_state_reference(struct pipe_vertex_state **dst,
                              struct pipe_vertex_state *src)
{
   *dst = src;
   pipe_reference(NULL, &src->reference); /* only increment refcount */
}

/* Unreference dst but don't touch the dst pointer. */
static inline void
tc_drop_resource_reference(struct pipe_resource *dst)
{
   if (pipe_reference(&dst->reference, NULL)) /* only decrement refcount */
      pipe_resource_destroy(dst);
}

/* Unreference dst but don't touch the dst pointer. */
static inline void
tc_drop_surface_reference(struct pipe_surface *dst)
{
   if (pipe_reference(&dst->reference, NULL)) /* only decrement refcount */
      dst->context->surface_destroy(dst->context, dst);
}

/* Unreference dst but don't touch the dst pointer. */
static inline void
tc_drop_so_target_reference(struct pipe_stream_output_target *dst)
{
   if (pipe_reference(&dst->reference, NULL)) /* only decrement refcount */
      dst->context->stream_output_target_destroy(dst->context, dst);
}

/**
 * Subtract the given number of references.
 */
static inline void
tc_drop_vertex_state_references(struct pipe_vertex_state *dst, int num_refs)
{
   int count = p_atomic_add_return(&dst->reference.count, -num_refs);

   assert(count >= 0);
   /* Underflows shouldn't happen, but let's be safe. */
   if (count <= 0)
      dst->screen->vertex_state_destroy(dst->screen, dst);
}

/* We don't want to read or write min_index and max_index, because
 * it shouldn't be needed by drivers at this point.
 */
#define DRAW_INFO_SIZE_WITHOUT_MIN_MAX_INDEX \
   offsetof(struct pipe_draw_info, min_index)

ALWAYS_INLINE static struct tc_renderpass_info *
incr_rp_info(struct tc_renderpass_info *tc_info)
{
   struct tc_batch_rp_info *info = tc_batch_rp_info(tc_info);
   return &info[1].info;
}

ALWAYS_INLINE static void
batch_execute(struct tc_batch *batch, struct pipe_context *pipe, uint64_t *last, bool parsing)
{
   /* if the framebuffer state is persisting from a previous batch,
    * begin incrementing renderpass info on the first set_framebuffer_state call
    */
   bool first = !batch->first_set_fb;
   for (uint64_t *iter = batch->slots; iter != last;) {
      struct tc_call_base *call = (struct tc_call_base *)iter;

      tc_assert(call->sentinel == TC_SENTINEL);

#if TC_DEBUG >= 3
      tc_printf("CALL: %s", tc_call_names[call->call_id]);
#endif

      TC_TRACE_SCOPE(call->call_id);

      iter += execute_func[call->call_id](pipe, call);

      if (parsing) {
         if (call->call_id == TC_CALL_flush) {
            /* always increment renderpass info for non-deferred flushes */
            batch->tc->renderpass_info = incr_rp_info(batch->tc->renderpass_info);
            /* if a flush happens, renderpass info is always incremented after */
            first = false;
         } else if (call->call_id == TC_CALL_set_framebuffer_state) {
            /* the renderpass info pointer is already set at the start of the batch,
             * so don't increment on the first set_framebuffer_state call
             */
            if (!first)
               batch->tc->renderpass_info = incr_rp_info(batch->tc->renderpass_info);
            first = false;
         } else if (call->call_id >= TC_CALL_draw_single &&
                    call->call_id <= TC_CALL_draw_vstate_multi) {
            /* if a draw happens before a set_framebuffer_state on this batch,
             * begin incrementing renderpass data 
             */
            first = false;
         }
      }
   }
}

static void
tc_batch_execute(void *job, UNUSED void *gdata, int thread_index)
{
   struct tc_batch *batch = job;
   struct pipe_context *pipe = batch->tc->pipe;
   uint64_t *last = &batch->slots[batch->num_total_slots];

   tc_batch_check(batch);
   tc_set_driver_thread(batch->tc);

   assert(!batch->token);

   /* setup renderpass info */
   batch->tc->renderpass_info = batch->renderpass_infos.data;

   if (batch->tc->options.parse_renderpass_info) {
      batch_execute(batch, pipe, last, true);

      struct tc_batch_rp_info *info = batch->renderpass_infos.data;
      for (unsigned i = 0; i < batch->max_renderpass_info_idx + 1; i++) {
         if (info[i].next)
            info[i].next->prev = NULL;
         info[i].next = NULL;
      }
   } else {
      batch_execute(batch, pipe, last, false);
   }

   /* Add the fence to the list of fences for the driver to signal at the next
    * flush, which we use for tracking which buffers are referenced by
    * an unflushed command buffer.
    */
   struct threaded_context *tc = batch->tc;
   struct util_queue_fence *fence =
      &tc->buffer_lists[batch->buffer_list_index].driver_flushed_fence;

   if (tc->options.driver_calls_flush_notify) {
      tc->signal_fences_next_flush[tc->num_signal_fences_next_flush++] = fence;

      /* Since our buffer lists are chained as a ring, we need to flush
       * the context twice as we go around the ring to make the driver signal
       * the buffer list fences, so that the producer thread can reuse the buffer
       * list structures for the next batches without waiting.
       */
      unsigned half_ring = TC_MAX_BUFFER_LISTS / 2;
      if (batch->buffer_list_index % half_ring == half_ring - 1)
         pipe->flush(pipe, NULL, PIPE_FLUSH_ASYNC);
   } else {
      util_queue_fence_signal(fence);
   }

   tc_clear_driver_thread(batch->tc);
   tc_batch_check(batch);
   batch->num_total_slots = 0;
   batch->last_mergeable_call = NULL;
   batch->first_set_fb = false;
   batch->max_renderpass_info_idx = 0;
   batch->tc->last_completed = batch->batch_idx;
}

static void
tc_begin_next_buffer_list(struct threaded_context *tc)
{
   tc->next_buf_list = (tc->next_buf_list + 1) % TC_MAX_BUFFER_LISTS;

   tc->batch_slots[tc->next].buffer_list_index = tc->next_buf_list;

   /* Clear the buffer list in the new empty batch. */
   struct tc_buffer_list *buf_list = &tc->buffer_lists[tc->next_buf_list];
   assert(util_queue_fence_is_signalled(&buf_list->driver_flushed_fence));
   util_queue_fence_reset(&buf_list->driver_flushed_fence); /* set to unsignalled */
   BITSET_ZERO(buf_list->buffer_list);

   tc->add_all_gfx_bindings_to_buffer_list = true;
   tc->add_all_compute_bindings_to_buffer_list = true;
}

static void
tc_add_call_end(struct tc_batch *next)
{
   /* Add a dummy last call that won't be executed, but will indicate the end
    * of the batch. It's for calls that always look at the next call and this
    * stops them looking farther ahead.
    */
   assert(next->num_total_slots < TC_SLOTS_PER_BATCH);
   struct tc_call_base *call =
      (struct tc_call_base*)&next->slots[next->num_total_slots];
   call->call_id = TC_NUM_CALLS;
   call->num_slots = 1;
}

static void
tc_batch_flush(struct threaded_context *tc, bool full_copy)
{
   struct tc_batch *next = &tc->batch_slots[tc->next];
   unsigned next_id = (tc->next + 1) % TC_MAX_BATCHES;

   tc_assert(next->num_total_slots != 0);
   tc_add_call_end(next);

   tc_batch_check(next);
   tc_debug_check(tc);
   tc->bytes_mapped_estimate = 0;
   p_atomic_add(&tc->num_offloaded_slots, next->num_total_slots);

   if (next->token) {
      next->token->tc = NULL;
      tc_unflushed_batch_token_reference(&next->token, NULL);
   }
   /* reset renderpass info index for subsequent use */
   next->renderpass_info_idx = -1;

   /* always increment renderpass info on batch flush;
    * renderpass info can only be accessed by its owner batch during execution
    */
   if (tc->renderpass_info_recording) {
      tc->batch_slots[next_id].first_set_fb = full_copy;
      tc_batch_increment_renderpass_info(tc, next_id, full_copy);
   }

   util_queue_add_job(&tc->queue, next, &next->fence, tc_batch_execute,
                      NULL, 0);
   tc->last = tc->next;
   tc->next = next_id;
   if (next_id == 0)
      tc->batch_generation++;
   tc_begin_next_buffer_list(tc);

}

/* This is the function that adds variable-sized calls into the current
 * batch. It also flushes the batch if there is not enough space there.
 * All other higher-level "add" functions use it.
 */
static void *
tc_add_sized_call(struct threaded_context *tc, enum tc_call_id id,
                  unsigned num_slots)
{
   TC_TRACE_SCOPE(id);
   struct tc_batch *next = &tc->batch_slots[tc->next];
   assert(num_slots <= TC_SLOTS_PER_BATCH - 1);
   tc_debug_check(tc);

   if (unlikely(next->num_total_slots + num_slots > TC_SLOTS_PER_BATCH - 1)) {
      /* copy existing renderpass info during flush */
      tc_batch_flush(tc, true);
      next = &tc->batch_slots[tc->next];
      tc_assert(next->num_total_slots == 0);
      tc_assert(next->last_mergeable_call == NULL);
   }

   tc_assert(util_queue_fence_is_signalled(&next->fence));

   struct tc_call_base *call = (struct tc_call_base*)&next->slots[next->num_total_slots];
   next->num_total_slots += num_slots;

#if !defined(NDEBUG) && TC_DEBUG >= 1
   call->sentinel = TC_SENTINEL;
#endif
   call->call_id = id;
   call->num_slots = num_slots;

#if TC_DEBUG >= 3
   tc_printf("ENQUEUE: %s", tc_call_names[id]);
#endif

   tc_debug_check(tc);
   return call;
}

#define tc_add_call(tc, execute, type) \
   ((struct type*)tc_add_sized_call(tc, execute, call_size(type)))

#define tc_add_slot_based_call(tc, execute, type, num_slots) \
   ((struct type*)tc_add_sized_call(tc, execute, \
                                    call_size_with_slots(type, num_slots)))

/* Returns the last mergeable call that was added to the unflushed
 * batch, or NULL if the address of that call is not currently known
 * or no such call exists in the unflushed batch.
 */
static struct tc_call_base *
tc_get_last_mergeable_call(struct threaded_context *tc)
{
   struct tc_batch *batch = &tc->batch_slots[tc->next];
   struct tc_call_base *call = batch->last_mergeable_call;

   tc_assert(call == NULL || call->num_slots <= batch->num_total_slots);

   if (call && (uint64_t *)call == &batch->slots[batch->num_total_slots - call->num_slots])
      return call;
   else
      return NULL;
}

/* Increases the size of the last call in the unflushed batch to the
 * given number of slots, if possible, without changing the call's data.
 */
static bool
tc_enlarge_last_mergeable_call(struct threaded_context *tc, unsigned desired_num_slots)
{
   struct tc_batch *batch = &tc->batch_slots[tc->next];
   struct tc_call_base *call = tc_get_last_mergeable_call(tc);

   tc_assert(call);
   tc_assert(desired_num_slots >= call->num_slots);

   unsigned added_slots = desired_num_slots - call->num_slots;

   if (unlikely(batch->num_total_slots + added_slots > TC_SLOTS_PER_BATCH - 1))
      return false;

   batch->num_total_slots += added_slots;
   call->num_slots += added_slots;

   return true;
}

static void
tc_mark_call_mergeable(struct threaded_context *tc, struct tc_call_base *call)
{
   struct tc_batch *batch = &tc->batch_slots[tc->next];
   tc_assert(call->num_slots <= batch->num_total_slots);
   tc_assert((uint64_t *)call == &batch->slots[batch->num_total_slots - call->num_slots]);
   batch->last_mergeable_call = call;
}

static bool
tc_is_sync(struct threaded_context *tc)
{
   struct tc_batch *last = &tc->batch_slots[tc->last];
   struct tc_batch *next = &tc->batch_slots[tc->next];

   return util_queue_fence_is_signalled(&last->fence) &&
          !next->num_total_slots;
}

static void
_tc_sync(struct threaded_context *tc, UNUSED const char *info, UNUSED const char *func)
{
   struct tc_batch *last = &tc->batch_slots[tc->last];
   struct tc_batch *next = &tc->batch_slots[tc->next];
   bool synced = false;

   MESA_TRACE_SCOPE(func);

   tc_debug_check(tc);

   if (tc->options.parse_renderpass_info && tc->in_renderpass && !tc->flushing) {
      /* corner case: if tc syncs for any reason but a driver flush during a renderpass,
       * then the current renderpass info MUST be signaled to avoid deadlocking the driver
       *
       * this is not a "complete" signal operation, however, as it's unknown what calls may
       * come after this one, which means that framebuffer attachment data is unreliable
       * 
       * to avoid erroneously passing bad state to the driver (e.g., allowing zsbuf elimination),
       * force all attachments active and assume the app was going to get bad perf here anyway
       */
      tc_sanitize_renderpass_info(tc);
   }
   tc_signal_renderpass_info_ready(tc);

   /* Only wait for queued calls... */
   if (!util_queue_fence_is_signalled(&last->fence)) {
      util_queue_fence_wait(&last->fence);
      synced = true;
   }

   tc_debug_check(tc);

   if (next->token) {
      next->token->tc = NULL;
      tc_unflushed_batch_token_reference(&next->token, NULL);
   }

   /* .. and execute unflushed calls directly. */
   if (next->num_total_slots) {
      p_atomic_add(&tc->num_direct_slots, next->num_total_slots);
      tc->bytes_mapped_estimate = 0;
      tc_add_call_end(next);
      tc_batch_execute(next, NULL, 0);
      tc_begin_next_buffer_list(tc);
      synced = true;
   }

   if (synced) {
      p_atomic_inc(&tc->num_syncs);

      if (tc_strcmp(func, "tc_destroy") != 0) {
         tc_printf("sync %s %s", func, info);
      }
   }

   tc_debug_check(tc);

   if (tc->options.parse_renderpass_info) {
      int renderpass_info_idx = next->renderpass_info_idx;
      if (renderpass_info_idx > 0) {
         /* don't reset if fb state is unflushed */
         bool fb_no_draw = tc->seen_fb_state && !tc->renderpass_info_recording->has_draw;
         uint32_t fb_info = tc->renderpass_info_recording->data32[0];
         next->renderpass_info_idx = -1;
         tc_batch_increment_renderpass_info(tc, tc->next, false);
         if (fb_no_draw)
            tc->renderpass_info_recording->data32[0] = fb_info;
      } else if (tc->renderpass_info_recording->has_draw) {
         tc->renderpass_info_recording->data32[0] = 0;
      }
      tc->seen_fb_state = false;
      tc->query_ended = false;
   }
}

#define tc_sync(tc) _tc_sync(tc, "", __func__)
#define tc_sync_msg(tc, info) _tc_sync(tc, info, __func__)

/**
 * Call this from fence_finish for same-context fence waits of deferred fences
 * that haven't been flushed yet.
 *
 * The passed pipe_context must be the one passed to pipe_screen::fence_finish,
 * i.e., the wrapped one.
 */
void
threaded_context_flush(struct pipe_context *_pipe,
                       struct tc_unflushed_batch_token *token,
                       bool prefer_async)
{
   struct threaded_context *tc = threaded_context(_pipe);

   /* This is called from the gallium frontend / application thread. */
   if (token->tc && token->tc == tc) {
      struct tc_batch *last = &tc->batch_slots[tc->last];

      /* Prefer to do the flush in the driver thread if it is already
       * running. That should be better for cache locality.
       */
      if (prefer_async || !util_queue_fence_is_signalled(&last->fence))
         tc_batch_flush(tc, false);
      else
         tc_sync(token->tc);
   }
}

static void
tc_add_to_buffer_list(struct tc_buffer_list *next, struct pipe_resource *buf)
{
   uint32_t id = threaded_resource(buf)->buffer_id_unique;
   BITSET_SET(next->buffer_list, id & TC_BUFFER_ID_MASK);
}

/* Set a buffer binding and add it to the buffer list. */
static void
tc_bind_buffer(uint32_t *binding, struct tc_buffer_list *next, struct pipe_resource *buf)
{
   uint32_t id = threaded_resource(buf)->buffer_id_unique;
   *binding = id;
   BITSET_SET(next->buffer_list, id & TC_BUFFER_ID_MASK);
}

/* Reset a buffer binding. */
static void
tc_unbind_buffer(uint32_t *binding)
{
   *binding = 0;
}

/* Reset a range of buffer binding slots. */
static void
tc_unbind_buffers(uint32_t *binding, unsigned count)
{
   if (count)
      memset(binding, 0, sizeof(*binding) * count);
}

static void
tc_add_bindings_to_buffer_list(BITSET_WORD *buffer_list, const uint32_t *bindings,
                               unsigned count)
{
   for (unsigned i = 0; i < count; i++) {
      if (bindings[i])
         BITSET_SET(buffer_list, bindings[i] & TC_BUFFER_ID_MASK);
   }
}

static bool
tc_rebind_bindings(uint32_t old_id, uint32_t new_id, uint32_t *bindings,
                   unsigned count)
{
   unsigned rebind_count = 0;

   for (unsigned i = 0; i < count; i++) {
      if (bindings[i] == old_id) {
         bindings[i] = new_id;
         rebind_count++;
      }
   }
   return rebind_count;
}

static void
tc_add_shader_bindings_to_buffer_list(struct threaded_context *tc,
                                      BITSET_WORD *buffer_list,
                                      enum pipe_shader_type shader)
{
   tc_add_bindings_to_buffer_list(buffer_list, tc->const_buffers[shader],
                                  tc->max_const_buffers);
   if (tc->seen_shader_buffers[shader]) {
      tc_add_bindings_to_buffer_list(buffer_list, tc->shader_buffers[shader],
                                     tc->max_shader_buffers);
   }
   if (tc->seen_image_buffers[shader]) {
      tc_add_bindings_to_buffer_list(buffer_list, tc->image_buffers[shader],
                                     tc->max_images);
   }
   if (tc->seen_sampler_buffers[shader]) {
      tc_add_bindings_to_buffer_list(buffer_list, tc->sampler_buffers[shader],
                                     tc->max_samplers);
   }
}

static unsigned
tc_rebind_shader_bindings(struct threaded_context *tc, uint32_t old_id,
                          uint32_t new_id, enum pipe_shader_type shader, uint32_t *rebind_mask)
{
   unsigned ubo = 0, ssbo = 0, img = 0, sampler = 0;

   ubo = tc_rebind_bindings(old_id, new_id, tc->const_buffers[shader],
                            tc->max_const_buffers);
   if (ubo)
      *rebind_mask |= BITFIELD_BIT(TC_BINDING_UBO_VS) << shader;
   if (tc->seen_shader_buffers[shader]) {
      ssbo = tc_rebind_bindings(old_id, new_id, tc->shader_buffers[shader],
                                tc->max_shader_buffers);
      if (ssbo)
         *rebind_mask |= BITFIELD_BIT(TC_BINDING_SSBO_VS) << shader;
   }
   if (tc->seen_image_buffers[shader]) {
      img = tc_rebind_bindings(old_id, new_id, tc->image_buffers[shader],
                               tc->max_images);
      if (img)
         *rebind_mask |= BITFIELD_BIT(TC_BINDING_IMAGE_VS) << shader;
   }
   if (tc->seen_sampler_buffers[shader]) {
      sampler = tc_rebind_bindings(old_id, new_id, tc->sampler_buffers[shader],
                                   tc->max_samplers);
      if (sampler)
         *rebind_mask |= BITFIELD_BIT(TC_BINDING_SAMPLERVIEW_VS) << shader;
   }
   return ubo + ssbo + img + sampler;
}

/* Add all bound buffers used by VS/TCS/TES/GS/FS to the buffer list.
 * This is called by the first draw call in a batch when we want to inherit
 * all bindings set by the previous batch.
 */
static void
tc_add_all_gfx_bindings_to_buffer_list(struct threaded_context *tc)
{
   BITSET_WORD *buffer_list = tc->buffer_lists[tc->next_buf_list].buffer_list;

   tc_add_bindings_to_buffer_list(buffer_list, tc->vertex_buffers, tc->max_vertex_buffers);
   if (tc->seen_streamout_buffers)
      tc_add_bindings_to_buffer_list(buffer_list, tc->streamout_buffers, PIPE_MAX_SO_BUFFERS);

   tc_add_shader_bindings_to_buffer_list(tc, buffer_list, PIPE_SHADER_VERTEX);
   tc_add_shader_bindings_to_buffer_list(tc, buffer_list, PIPE_SHADER_FRAGMENT);

   if (tc->seen_tcs)
      tc_add_shader_bindings_to_buffer_list(tc, buffer_list, PIPE_SHADER_TESS_CTRL);
   if (tc->seen_tes)
      tc_add_shader_bindings_to_buffer_list(tc, buffer_list, PIPE_SHADER_TESS_EVAL);
   if (tc->seen_gs)
      tc_add_shader_bindings_to_buffer_list(tc, buffer_list, PIPE_SHADER_GEOMETRY);

   tc->add_all_gfx_bindings_to_buffer_list = false;
}

/* Add all bound buffers used by compute to the buffer list.
 * This is called by the first compute call in a batch when we want to inherit
 * all bindings set by the previous batch.
 */
static void
tc_add_all_compute_bindings_to_buffer_list(struct threaded_context *tc)
{
   BITSET_WORD *buffer_list = tc->buffer_lists[tc->next_buf_list].buffer_list;

   tc_add_shader_bindings_to_buffer_list(tc, buffer_list, PIPE_SHADER_COMPUTE);
   tc->add_all_compute_bindings_to_buffer_list = false;
}

static unsigned
tc_rebind_buffer(struct threaded_context *tc, uint32_t old_id, uint32_t new_id, uint32_t *rebind_mask)
{
   unsigned vbo = 0, so = 0;

   vbo = tc_rebind_bindings(old_id, new_id, tc->vertex_buffers,
                            tc->max_vertex_buffers);
   if (vbo)
      *rebind_mask |= BITFIELD_BIT(TC_BINDING_VERTEX_BUFFER);

   if (tc->seen_streamout_buffers) {
      so = tc_rebind_bindings(old_id, new_id, tc->streamout_buffers,
                              PIPE_MAX_SO_BUFFERS);
      if (so)
         *rebind_mask |= BITFIELD_BIT(TC_BINDING_STREAMOUT_BUFFER);
   }
   unsigned rebound = vbo + so;

   rebound += tc_rebind_shader_bindings(tc, old_id, new_id, PIPE_SHADER_VERTEX, rebind_mask);
   rebound += tc_rebind_shader_bindings(tc, old_id, new_id, PIPE_SHADER_FRAGMENT, rebind_mask);

   if (tc->seen_tcs)
      rebound += tc_rebind_shader_bindings(tc, old_id, new_id, PIPE_SHADER_TESS_CTRL, rebind_mask);
   if (tc->seen_tes)
      rebound += tc_rebind_shader_bindings(tc, old_id, new_id, PIPE_SHADER_TESS_EVAL, rebind_mask);
   if (tc->seen_gs)
      rebound += tc_rebind_shader_bindings(tc, old_id, new_id, PIPE_SHADER_GEOMETRY, rebind_mask);

   rebound += tc_rebind_shader_bindings(tc, old_id, new_id, PIPE_SHADER_COMPUTE, rebind_mask);

   if (rebound)
      BITSET_SET(tc->buffer_lists[tc->next_buf_list].buffer_list, new_id & TC_BUFFER_ID_MASK);
   return rebound;
}

static bool
tc_is_buffer_bound_with_mask(uint32_t id, uint32_t *bindings, unsigned binding_mask)
{
   while (binding_mask) {
      if (bindings[u_bit_scan(&binding_mask)] == id)
         return true;
   }
   return false;
}

static bool
tc_is_buffer_shader_bound_for_write(struct threaded_context *tc, uint32_t id,
                                    enum pipe_shader_type shader)
{
   if (tc->seen_shader_buffers[shader] &&
       tc_is_buffer_bound_with_mask(id, tc->shader_buffers[shader],
                                    tc->shader_buffers_writeable_mask[shader]))
      return true;

   if (tc->seen_image_buffers[shader] &&
       tc_is_buffer_bound_with_mask(id, tc->image_buffers[shader],
                                    tc->image_buffers_writeable_mask[shader]))
      return true;

   return false;
}

static bool
tc_is_buffer_bound_for_write(struct threaded_context *tc, uint32_t id)
{
   if (tc->seen_streamout_buffers &&
       tc_is_buffer_bound_with_mask(id, tc->streamout_buffers,
                                    BITFIELD_MASK(PIPE_MAX_SO_BUFFERS)))
      return true;

   if (tc_is_buffer_shader_bound_for_write(tc, id, PIPE_SHADER_VERTEX) ||
       tc_is_buffer_shader_bound_for_write(tc, id, PIPE_SHADER_FRAGMENT) ||
       tc_is_buffer_shader_bound_for_write(tc, id, PIPE_SHADER_COMPUTE))
      return true;

   if (tc->seen_tcs &&
       tc_is_buffer_shader_bound_for_write(tc, id, PIPE_SHADER_TESS_CTRL))
      return true;

   if (tc->seen_tes &&
       tc_is_buffer_shader_bound_for_write(tc, id, PIPE_SHADER_TESS_EVAL))
      return true;

   if (tc->seen_gs &&
       tc_is_buffer_shader_bound_for_write(tc, id, PIPE_SHADER_GEOMETRY))
      return true;

   return false;
}

static bool
tc_is_buffer_busy(struct threaded_context *tc, struct threaded_resource *tbuf,
                  unsigned map_usage)
{
   if (!tc->options.is_resource_busy)
      return true;

   uint32_t id_hash = tbuf->buffer_id_unique & TC_BUFFER_ID_MASK;

   for (unsigned i = 0; i < TC_MAX_BUFFER_LISTS; i++) {
      struct tc_buffer_list *buf_list = &tc->buffer_lists[i];

      /* If the buffer is referenced by a batch that hasn't been flushed (by tc or the driver),
       * then the buffer is considered busy. */
      if (!util_queue_fence_is_signalled(&buf_list->driver_flushed_fence) &&
          BITSET_TEST(buf_list->buffer_list, id_hash))
         return true;
   }

   /* The buffer isn't referenced by any unflushed batch: we can safely ask to the driver whether
    * this buffer is busy or not. */
   return tc->options.is_resource_busy(tc->pipe->screen, tbuf->latest, map_usage);
}

/**
 * allow_cpu_storage should be false for user memory and imported buffers.
 */
void
threaded_resource_init(struct pipe_resource *res, bool allow_cpu_storage)
{
   struct threaded_resource *tres = threaded_resource(res);

   tres->latest = &tres->b;
   tres->cpu_storage = NULL;
   util_range_init(&tres->valid_buffer_range);
   tres->is_shared = false;
   tres->is_user_ptr = false;
   tres->buffer_id_unique = 0;
   tres->pending_staging_uploads = 0;
   tres->last_batch_usage = -1;
   util_range_init(&tres->pending_staging_uploads_range);

   if (allow_cpu_storage &&
       !(res->flags & (PIPE_RESOURCE_FLAG_MAP_PERSISTENT |
                       PIPE_RESOURCE_FLAG_SPARSE |
                       PIPE_RESOURCE_FLAG_ENCRYPTED)) &&
       /* We need buffer invalidation and buffer busyness tracking for the CPU
        * storage, which aren't supported with pipe_vertex_state. */
       !(res->bind & PIPE_BIND_VERTEX_STATE))
      tres->allow_cpu_storage = true;
   else
      tres->allow_cpu_storage = false;
}

void
threaded_resource_deinit(struct pipe_resource *res)
{
   struct threaded_resource *tres = threaded_resource(res);

   if (tres->latest != &tres->b)
           pipe_resource_reference(&tres->latest, NULL);
   util_range_destroy(&tres->valid_buffer_range);
   util_range_destroy(&tres->pending_staging_uploads_range);
   align_free(tres->cpu_storage);
}

struct pipe_context *
threaded_context_unwrap_sync(struct pipe_context *pipe)
{
   if (!pipe || !pipe->priv)
      return pipe;

   tc_sync(threaded_context(pipe));
   return (struct pipe_context*)pipe->priv;
}


/********************************************************************
 * simple functions
 */

#define TC_FUNC1(func, qualifier, type, deref, addr, ...) \
   struct tc_call_##func { \
      struct tc_call_base base; \
      type state; \
   }; \
   \
   static uint16_t \
   tc_call_##func(struct pipe_context *pipe, void *call) \
   { \
      pipe->func(pipe, addr(to_call(call, tc_call_##func)->state)); \
      return call_size(tc_call_##func); \
   } \
   \
   static void \
   tc_##func(struct pipe_context *_pipe, qualifier type deref param) \
   { \
      struct threaded_context *tc = threaded_context(_pipe); \
      struct tc_call_##func *p = (struct tc_call_##func*) \
                     tc_add_call(tc, TC_CALL_##func, tc_call_##func); \
      p->state = deref(param); \
      __VA_ARGS__; \
   }

TC_FUNC1(set_active_query_state, , bool, , )

TC_FUNC1(set_blend_color, const, struct pipe_blend_color, *, &)
TC_FUNC1(set_stencil_ref, const, struct pipe_stencil_ref, , )
TC_FUNC1(set_clip_state, const, struct pipe_clip_state, *, &)
TC_FUNC1(set_sample_mask, , unsigned, , )
TC_FUNC1(set_min_samples, , unsigned, , )
TC_FUNC1(set_polygon_stipple, const, struct pipe_poly_stipple, *, &)

TC_FUNC1(texture_barrier, , unsigned, , )
TC_FUNC1(memory_barrier, , unsigned, , )
TC_FUNC1(delete_texture_handle, , uint64_t, , )
TC_FUNC1(delete_image_handle, , uint64_t, , )
TC_FUNC1(set_frontend_noop, , bool, , )


/********************************************************************
 * queries
 */

static struct pipe_query *
tc_create_query(struct pipe_context *_pipe, unsigned query_type,
                unsigned index)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   return pipe->create_query(pipe, query_type, index);
}

static struct pipe_query *
tc_create_batch_query(struct pipe_context *_pipe, unsigned num_queries,
                      unsigned *query_types)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   return pipe->create_batch_query(pipe, num_queries, query_types);
}

struct tc_query_call {
   struct tc_call_base base;
   struct pipe_query *query;
};

static uint16_t
tc_call_destroy_query(struct pipe_context *pipe, void *call)
{
   struct pipe_query *query = to_call(call, tc_query_call)->query;
   struct threaded_query *tq = threaded_query(query);

   if (list_is_linked(&tq->head_unflushed))
      list_del(&tq->head_unflushed);

   pipe->destroy_query(pipe, query);
   return call_size(tc_query_call);
}

static void
tc_destroy_query(struct pipe_context *_pipe, struct pipe_query *query)
{
   struct threaded_context *tc = threaded_context(_pipe);

   tc_add_call(tc, TC_CALL_destroy_query, tc_query_call)->query = query;
}

static uint16_t
tc_call_begin_query(struct pipe_context *pipe, void *call)
{
   pipe->begin_query(pipe, to_call(call, tc_query_call)->query);
   return call_size(tc_query_call);
}

static bool
tc_begin_query(struct pipe_context *_pipe, struct pipe_query *query)
{
   struct threaded_context *tc = threaded_context(_pipe);
   tc->num_queries_active++;

   tc_add_call(tc, TC_CALL_begin_query, tc_query_call)->query = query;
   return true; /* we don't care about the return value for this call */
}

struct tc_end_query_call {
   struct tc_call_base base;
   struct threaded_context *tc;
   struct pipe_query *query;
};

static uint16_t
tc_call_end_query(struct pipe_context *pipe, void *call)
{
   struct tc_end_query_call *p = to_call(call, tc_end_query_call);
   struct threaded_query *tq = threaded_query(p->query);

   if (!list_is_linked(&tq->head_unflushed))
      list_add(&tq->head_unflushed, &p->tc->unflushed_queries);

   pipe->end_query(pipe, p->query);
   return call_size(tc_end_query_call);
}

static bool
tc_end_query(struct pipe_context *_pipe, struct pipe_query *query)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct threaded_query *tq = threaded_query(query);
   struct tc_end_query_call *call =
      tc_add_call(tc, TC_CALL_end_query, tc_end_query_call);
   tc->num_queries_active--;

   call->tc = tc;
   call->query = query;

   tq->flushed = false;
   tc->query_ended = true;

   return true; /* we don't care about the return value for this call */
}

static bool
tc_get_query_result(struct pipe_context *_pipe,
                    struct pipe_query *query, bool wait,
                    union pipe_query_result *result)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct threaded_query *tq = threaded_query(query);
   struct pipe_context *pipe = tc->pipe;
   bool flushed = tq->flushed;

   if (!flushed) {
      tc_sync_msg(tc, wait ? "wait" : "nowait");
      tc_set_driver_thread(tc);
   }

   bool success = pipe->get_query_result(pipe, query, wait, result);

   if (!flushed)
      tc_clear_driver_thread(tc);

   if (success) {
      tq->flushed = true;
      if (list_is_linked(&tq->head_unflushed)) {
         /* This is safe because it can only happen after we sync'd. */
         list_del(&tq->head_unflushed);
      }
   }
   return success;
}

struct tc_query_result_resource {
   struct tc_call_base base;
   enum pipe_query_flags flags:8;
   enum pipe_query_value_type result_type:8;
   int8_t index; /* it can be -1 */
   unsigned offset;
   struct pipe_query *query;
   struct pipe_resource *resource;
};

static uint16_t
tc_call_get_query_result_resource(struct pipe_context *pipe, void *call)
{
   struct tc_query_result_resource *p = to_call(call, tc_query_result_resource);

   pipe->get_query_result_resource(pipe, p->query, p->flags, p->result_type,
                                   p->index, p->resource, p->offset);
   tc_drop_resource_reference(p->resource);
   return call_size(tc_query_result_resource);
}

static void
tc_get_query_result_resource(struct pipe_context *_pipe,
                             struct pipe_query *query,
                             enum pipe_query_flags flags,
                             enum pipe_query_value_type result_type, int index,
                             struct pipe_resource *resource, unsigned offset)
{
   struct threaded_context *tc = threaded_context(_pipe);

   tc_buffer_disable_cpu_storage(resource);

   struct tc_query_result_resource *p =
      tc_add_call(tc, TC_CALL_get_query_result_resource,
                  tc_query_result_resource);
   p->query = query;
   p->flags = flags;
   p->result_type = result_type;
   p->index = index;
   tc_set_resource_reference(&p->resource, resource);
   tc_add_to_buffer_list(&tc->buffer_lists[tc->next_buf_list], resource);
   p->offset = offset;
}

struct tc_render_condition {
   struct tc_call_base base;
   bool condition;
   unsigned mode;
   struct pipe_query *query;
};

static uint16_t
tc_call_render_condition(struct pipe_context *pipe, void *call)
{
   struct tc_render_condition *p = to_call(call, tc_render_condition);
   pipe->render_condition(pipe, p->query, p->condition, p->mode);
   return call_size(tc_render_condition);
}

static void
tc_render_condition(struct pipe_context *_pipe,
                    struct pipe_query *query, bool condition,
                    enum pipe_render_cond_flag mode)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_render_condition *p =
      tc_add_call(tc, TC_CALL_render_condition, tc_render_condition);

   p->query = query;
   p->condition = condition;
   p->mode = mode;
}


/********************************************************************
 * constant (immutable) states
 */

#define TC_CSO_CREATE(name, sname) \
   static void * \
   tc_create_##name##_state(struct pipe_context *_pipe, \
                            const struct pipe_##sname##_state *state) \
   { \
      struct pipe_context *pipe = threaded_context(_pipe)->pipe; \
      return pipe->create_##name##_state(pipe, state); \
   }

#define TC_CSO_BIND(name, ...) TC_FUNC1(bind_##name##_state, , void *, , , ##__VA_ARGS__)
#define TC_CSO_DELETE(name) TC_FUNC1(delete_##name##_state, , void *, , )

#define TC_CSO(name, sname, ...) \
   TC_CSO_CREATE(name, sname) \
   TC_CSO_BIND(name, ##__VA_ARGS__) \
   TC_CSO_DELETE(name)

#define TC_CSO_WHOLE(name) TC_CSO(name, name)
#define TC_CSO_SHADER(name) TC_CSO(name, shader)
#define TC_CSO_SHADER_TRACK(name) TC_CSO(name, shader, tc->seen_##name = true;)

TC_CSO_WHOLE(blend)
TC_CSO_WHOLE(rasterizer)
TC_CSO_CREATE(depth_stencil_alpha, depth_stencil_alpha)
TC_CSO_BIND(depth_stencil_alpha,
   if (param && tc->options.parse_renderpass_info) {
      /* dsa info is only ever added during a renderpass;
       * changes outside of a renderpass reset the data
       */
      if (!tc->in_renderpass) {
         tc_get_renderpass_info(tc)->zsbuf_write_dsa = 0;
         tc_get_renderpass_info(tc)->zsbuf_read_dsa = 0;
      }
      /* let the driver parse its own state */
      tc->options.dsa_parse(param, tc_get_renderpass_info(tc));
   }
)
TC_CSO_DELETE(depth_stencil_alpha)
TC_CSO_WHOLE(compute)
TC_CSO_CREATE(fs, shader)
TC_CSO_BIND(fs,
   if (param && tc->options.parse_renderpass_info) {
      /* fs info is only ever added during a renderpass;
       * changes outside of a renderpass reset the data
       */
      if (!tc->in_renderpass) {
         tc_get_renderpass_info(tc)->cbuf_fbfetch = 0;
         tc_get_renderpass_info(tc)->zsbuf_write_fs = 0;
      }
      /* let the driver parse its own state */
      tc->options.fs_parse(param, tc_get_renderpass_info(tc));
   }
)
TC_CSO_DELETE(fs)
TC_CSO_SHADER(vs)
TC_CSO_SHADER_TRACK(gs)
TC_CSO_SHADER_TRACK(tcs)
TC_CSO_SHADER_TRACK(tes)
TC_CSO_CREATE(sampler, sampler)
TC_CSO_DELETE(sampler)
TC_CSO_BIND(vertex_elements)
TC_CSO_DELETE(vertex_elements)

static void *
tc_create_vertex_elements_state(struct pipe_context *_pipe, unsigned count,
                                const struct pipe_vertex_element *elems)
{
   struct pipe_context *pipe = threaded_context(_pipe)->pipe;

   return pipe->create_vertex_elements_state(pipe, count, elems);
}

struct tc_sampler_states {
   struct tc_call_base base;
   uint8_t shader, start, count;
   void *slot[0]; /* more will be allocated if needed */
};

static uint16_t
tc_call_bind_sampler_states(struct pipe_context *pipe, void *call)
{
   struct tc_sampler_states *p = (struct tc_sampler_states *)call;

   pipe->bind_sampler_states(pipe, p->shader, p->start, p->count, p->slot);
   return p->base.num_slots;
}

static void
tc_bind_sampler_states(struct pipe_context *_pipe,
                       enum pipe_shader_type shader,
                       unsigned start, unsigned count, void **states)
{
   if (!count)
      return;

   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_sampler_states *p =
      tc_add_slot_based_call(tc, TC_CALL_bind_sampler_states, tc_sampler_states, count);

   p->shader = shader;
   p->start = start;
   p->count = count;
   memcpy(p->slot, states, count * sizeof(states[0]));
}

static void
tc_link_shader(struct pipe_context *_pipe, void **shaders)
{
   struct threaded_context *tc = threaded_context(_pipe);
   tc->pipe->link_shader(tc->pipe, shaders);
}
/********************************************************************
 * immediate states
 */

struct tc_framebuffer {
   struct tc_call_base base;
   struct pipe_framebuffer_state state;
};

static uint16_t
tc_call_set_framebuffer_state(struct pipe_context *pipe, void *call)
{
   struct pipe_framebuffer_state *p = &to_call(call, tc_framebuffer)->state;

   pipe->set_framebuffer_state(pipe, p);

   unsigned nr_cbufs = p->nr_cbufs;
   for (unsigned i = 0; i < nr_cbufs; i++)
      tc_drop_surface_reference(p->cbufs[i]);
   tc_drop_surface_reference(p->zsbuf);
   tc_drop_resource_reference(p->resolve);
   return call_size(tc_framebuffer);
}

static void
tc_set_framebuffer_state(struct pipe_context *_pipe,
                         const struct pipe_framebuffer_state *fb)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_framebuffer *p =
      tc_add_call(tc, TC_CALL_set_framebuffer_state, tc_framebuffer);
   unsigned nr_cbufs = fb->nr_cbufs;

   p->state.width = fb->width;
   p->state.height = fb->height;
   p->state.samples = fb->samples;
   p->state.layers = fb->layers;
   p->state.nr_cbufs = nr_cbufs;

   /* when unbinding, mark attachments as used for the current batch */
   for (unsigned i = 0; i < tc->nr_cbufs; i++) {
      tc_set_resource_batch_usage_persistent(tc, tc->fb_resources[i], false);
      pipe_resource_reference(&tc->fb_resources[i], NULL);
   }
   tc_set_resource_batch_usage_persistent(tc, tc->fb_resources[PIPE_MAX_COLOR_BUFS], false);
   tc_set_resource_batch_usage_persistent(tc, tc->fb_resolve, false);

   for (unsigned i = 0; i < nr_cbufs; i++) {
      p->state.cbufs[i] = NULL;
      pipe_surface_reference(&p->state.cbufs[i], fb->cbufs[i]);
      /* full tracking requires storing the fb attachment resources */
      if (fb->cbufs[i])
         pipe_resource_reference(&tc->fb_resources[i], fb->cbufs[i]->texture);
      tc_set_resource_batch_usage_persistent(tc, tc->fb_resources[i], true);
   }
   tc->nr_cbufs = nr_cbufs;
   if (tc->options.parse_renderpass_info) {
      /* ensure this is treated as the first fb set if no fb activity has occurred */
      if (!tc->renderpass_info_recording->has_draw &&
          !tc->renderpass_info_recording->cbuf_clear &&
          !tc->renderpass_info_recording->cbuf_load &&
          !tc->renderpass_info_recording->zsbuf_load &&
          !tc->renderpass_info_recording->zsbuf_clear_partial)
         tc->batch_slots[tc->next].first_set_fb = false;
      /* store existing zsbuf data for possible persistence */
      uint8_t zsbuf = tc->renderpass_info_recording->has_draw ?
                      0 :
                      tc->renderpass_info_recording->data8[3];
      bool zsbuf_changed = tc->fb_resources[PIPE_MAX_COLOR_BUFS] !=
                           (fb->zsbuf ? fb->zsbuf->texture : NULL);

      if (tc->seen_fb_state) {
         /* this is the end of a renderpass, so increment the renderpass info */
         tc_batch_increment_renderpass_info(tc, tc->next, false);
         /* if zsbuf hasn't changed (i.e., possibly just adding a color buffer):
          * keep zsbuf usage data
          */
         if (!zsbuf_changed)
            tc->renderpass_info_recording->data8[3] = zsbuf;
      } else {
         /* this is the first time a set_framebuffer_call is triggered;
          * just increment the index and keep using the existing info for recording
          */
         tc->batch_slots[tc->next].renderpass_info_idx = 0;
      }
      /* future fb state changes will increment the index */
      tc->seen_fb_state = true;
   }
   pipe_resource_reference(&tc->fb_resources[PIPE_MAX_COLOR_BUFS],
                           fb->zsbuf ? fb->zsbuf->texture : NULL);
   pipe_resource_reference(&tc->fb_resolve, fb->resolve);
   tc_set_resource_batch_usage_persistent(tc, tc->fb_resources[PIPE_MAX_COLOR_BUFS], true);
   tc_set_resource_batch_usage_persistent(tc, tc->fb_resolve, true);
   tc->in_renderpass = false;
   p->state.zsbuf = NULL;
   pipe_surface_reference(&p->state.zsbuf, fb->zsbuf);
   p->state.resolve = NULL;
   pipe_resource_reference(&p->state.resolve, fb->resolve);
}

struct tc_tess_state {
   struct tc_call_base base;
   float state[6];
};

static uint16_t
tc_call_set_tess_state(struct pipe_context *pipe, void *call)
{
   float *p = to_call(call, tc_tess_state)->state;

   pipe->set_tess_state(pipe, p, p + 4);
   return call_size(tc_tess_state);
}

static void
tc_set_tess_state(struct pipe_context *_pipe,
                  const float default_outer_level[4],
                  const float default_inner_level[2])
{
   struct threaded_context *tc = threaded_context(_pipe);
   float *p = tc_add_call(tc, TC_CALL_set_tess_state, tc_tess_state)->state;

   memcpy(p, default_outer_level, 4 * sizeof(float));
   memcpy(p + 4, default_inner_level, 2 * sizeof(float));
}

struct tc_patch_vertices {
   struct tc_call_base base;
   uint8_t patch_vertices;
};

static uint16_t
tc_call_set_patch_vertices(struct pipe_context *pipe, void *call)
{
   uint8_t patch_vertices = to_call(call, tc_patch_vertices)->patch_vertices;

   pipe->set_patch_vertices(pipe, patch_vertices);
   return call_size(tc_patch_vertices);
}

static void
tc_set_patch_vertices(struct pipe_context *_pipe, uint8_t patch_vertices)
{
   struct threaded_context *tc = threaded_context(_pipe);

   tc_add_call(tc, TC_CALL_set_patch_vertices,
               tc_patch_vertices)->patch_vertices = patch_vertices;
}

struct tc_constant_buffer_base {
   struct tc_call_base base;
   uint8_t shader, index;
   bool is_null;
};

struct tc_constant_buffer {
   struct tc_constant_buffer_base base;
   struct pipe_constant_buffer cb;
};

static uint16_t
tc_call_set_constant_buffer(struct pipe_context *pipe, void *call)
{
   struct tc_constant_buffer *p = (struct tc_constant_buffer *)call;

   if (unlikely(p->base.is_null)) {
      pipe->set_constant_buffer(pipe, p->base.shader, p->base.index, false, NULL);
      return call_size(tc_constant_buffer_base);
   }

   pipe->set_constant_buffer(pipe, p->base.shader, p->base.index, true, &p->cb);
   return call_size(tc_constant_buffer);
}

static void
tc_set_constant_buffer(struct pipe_context *_pipe,
                       enum pipe_shader_type shader, uint index,
                       bool take_ownership,
                       const struct pipe_constant_buffer *cb)
{
   struct threaded_context *tc = threaded_context(_pipe);

   if (unlikely(!cb || (!cb->buffer && !cb->user_buffer))) {
      struct tc_constant_buffer_base *p =
         tc_add_call(tc, TC_CALL_set_constant_buffer, tc_constant_buffer_base);
      p->shader = shader;
      p->index = index;
      p->is_null = true;
      tc_unbind_buffer(&tc->const_buffers[shader][index]);
      return;
   }

   struct pipe_resource *buffer;
   unsigned offset;

   if (cb->user_buffer) {
      /* This must be done before adding set_constant_buffer, because it could
       * generate e.g. transfer_unmap and flush partially-uninitialized
       * set_constant_buffer to the driver if it was done afterwards.
       */
      buffer = NULL;
      u_upload_data(tc->base.const_uploader, 0, cb->buffer_size,
                    tc->ubo_alignment, cb->user_buffer, &offset, &buffer);
      u_upload_unmap(tc->base.const_uploader);
      take_ownership = true;
   } else {
      buffer = cb->buffer;
      offset = cb->buffer_offset;
   }

   struct tc_constant_buffer *p =
      tc_add_call(tc, TC_CALL_set_constant_buffer, tc_constant_buffer);
   p->base.shader = shader;
   p->base.index = index;
   p->base.is_null = false;
   p->cb.user_buffer = NULL;
   p->cb.buffer_offset = offset;
   p->cb.buffer_size = cb->buffer_size;

   if (take_ownership)
      p->cb.buffer = buffer;
   else
      tc_set_resource_reference(&p->cb.buffer, buffer);

   if (buffer) {
      tc_bind_buffer(&tc->const_buffers[shader][index],
                     &tc->buffer_lists[tc->next_buf_list], buffer);
   } else {
      tc_unbind_buffer(&tc->const_buffers[shader][index]);
   }
}

struct tc_inlinable_constants {
   struct tc_call_base base;
   uint8_t shader;
   uint8_t num_values;
   uint32_t values[MAX_INLINABLE_UNIFORMS];
};

static uint16_t
tc_call_set_inlinable_constants(struct pipe_context *pipe, void *call)
{
   struct tc_inlinable_constants *p = to_call(call, tc_inlinable_constants);

   pipe->set_inlinable_constants(pipe, p->shader, p->num_values, p->values);
   return call_size(tc_inlinable_constants);
}

static void
tc_set_inlinable_constants(struct pipe_context *_pipe,
                           enum pipe_shader_type shader,
                           uint num_values, uint32_t *values)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_inlinable_constants *p =
      tc_add_call(tc, TC_CALL_set_inlinable_constants, tc_inlinable_constants);
   p->shader = shader;
   p->num_values = num_values;
   memcpy(p->values, values, num_values * 4);
}

struct tc_sample_locations {
   struct tc_call_base base;
   uint16_t size;
   uint8_t slot[0];
};


static uint16_t
tc_call_set_sample_locations(struct pipe_context *pipe, void *call)
{
   struct tc_sample_locations *p = (struct tc_sample_locations *)call;

   pipe->set_sample_locations(pipe, p->size, p->slot);
   return p->base.num_slots;
}

static void
tc_set_sample_locations(struct pipe_context *_pipe, size_t size, const uint8_t *locations)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_sample_locations *p =
      tc_add_slot_based_call(tc, TC_CALL_set_sample_locations,
                             tc_sample_locations, size);

   p->size = size;
   memcpy(p->slot, locations, size);
}

struct tc_scissors {
   struct tc_call_base base;
   uint8_t start, count;
   struct pipe_scissor_state slot[0]; /* more will be allocated if needed */
};

static uint16_t
tc_call_set_scissor_states(struct pipe_context *pipe, void *call)
{
   struct tc_scissors *p = (struct tc_scissors *)call;

   pipe->set_scissor_states(pipe, p->start, p->count, p->slot);
   return p->base.num_slots;
}

static void
tc_set_scissor_states(struct pipe_context *_pipe,
                      unsigned start, unsigned count,
                      const struct pipe_scissor_state *states)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_scissors *p =
      tc_add_slot_based_call(tc, TC_CALL_set_scissor_states, tc_scissors, count);

   p->start = start;
   p->count = count;
   memcpy(&p->slot, states, count * sizeof(states[0]));
}

struct tc_viewports {
   struct tc_call_base base;
   uint8_t start, count;
   struct pipe_viewport_state slot[0]; /* more will be allocated if needed */
};

static uint16_t
tc_call_set_viewport_states(struct pipe_context *pipe, void *call)
{
   struct tc_viewports *p = (struct tc_viewports *)call;

   pipe->set_viewport_states(pipe, p->start, p->count, p->slot);
   return p->base.num_slots;
}

static void
tc_set_viewport_states(struct pipe_context *_pipe,
                       unsigned start, unsigned count,
                       const struct pipe_viewport_state *states)
{
   if (!count)
      return;

   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_viewports *p =
      tc_add_slot_based_call(tc, TC_CALL_set_viewport_states, tc_viewports, count);

   p->start = start;
   p->count = count;
   memcpy(&p->slot, states, count * sizeof(states[0]));
}

struct tc_window_rects {
   struct tc_call_base base;
   bool include;
   uint8_t count;
   struct pipe_scissor_state slot[0]; /* more will be allocated if needed */
};

static uint16_t
tc_call_set_window_rectangles(struct pipe_context *pipe, void *call)
{
   struct tc_window_rects *p = (struct tc_window_rects *)call;

   pipe->set_window_rectangles(pipe, p->include, p->count, p->slot);
   return p->base.num_slots;
}

static void
tc_set_window_rectangles(struct pipe_context *_pipe, bool include,
                         unsigned count,
                         const struct pipe_scissor_state *rects)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_window_rects *p =
      tc_add_slot_based_call(tc, TC_CALL_set_window_rectangles, tc_window_rects, count);

   p->include = include;
   p->count = count;
   memcpy(p->slot, rects, count * sizeof(rects[0]));
}

struct tc_sampler_views {
   struct tc_call_base base;
   uint8_t shader, start, count, unbind_num_trailing_slots;
   struct pipe_sampler_view *slot[0]; /* more will be allocated if needed */
};

static uint16_t
tc_call_set_sampler_views(struct pipe_context *pipe, void *call)
{
   struct tc_sampler_views *p = (struct tc_sampler_views *)call;

   pipe->set_sampler_views(pipe, p->shader, p->start, p->count,
                           p->unbind_num_trailing_slots, true, p->slot);
   return p->base.num_slots;
}

static void
tc_set_sampler_views(struct pipe_context *_pipe,
                     enum pipe_shader_type shader,
                     unsigned start, unsigned count,
                     unsigned unbind_num_trailing_slots, bool take_ownership,
                     struct pipe_sampler_view **views)
{
   if (!count && !unbind_num_trailing_slots)
      return;

   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_sampler_views *p =
      tc_add_slot_based_call(tc, TC_CALL_set_sampler_views, tc_sampler_views,
                             views ? count : 0);

   p->shader = shader;
   p->start = start;

   if (views) {
      struct tc_buffer_list *next = &tc->buffer_lists[tc->next_buf_list];

      p->count = count;
      p->unbind_num_trailing_slots = unbind_num_trailing_slots;

      if (take_ownership) {
         memcpy(p->slot, views, sizeof(*views) * count);

         for (unsigned i = 0; i < count; i++) {
            if (views[i]) {
               if (views[i]->target == PIPE_BUFFER)
                  tc_bind_buffer(&tc->sampler_buffers[shader][start + i], next,
                                 views[i]->texture);
               else
                  tc_set_resource_batch_usage(tc, views[i]->texture);
            } else {
               tc_unbind_buffer(&tc->sampler_buffers[shader][start + i]);
            }
         }
      } else {
         for (unsigned i = 0; i < count; i++) {
            p->slot[i] = NULL;
            pipe_sampler_view_reference(&p->slot[i], views[i]);

            if (views[i]) {
               if (views[i]->target == PIPE_BUFFER)
                  tc_bind_buffer(&tc->sampler_buffers[shader][start + i], next,
                                 views[i]->texture);
               else
                  tc_set_resource_batch_usage(tc, views[i]->texture);
            } else {
               tc_unbind_buffer(&tc->sampler_buffers[shader][start + i]);
            }
         }
      }

      tc_unbind_buffers(&tc->sampler_buffers[shader][start + count],
                        unbind_num_trailing_slots);
      tc->seen_sampler_buffers[shader] = true;
   } else {
      p->count = 0;
      p->unbind_num_trailing_slots = count + unbind_num_trailing_slots;

      tc_unbind_buffers(&tc->sampler_buffers[shader][start],
                        count + unbind_num_trailing_slots);
   }
}

struct tc_shader_images {
   struct tc_call_base base;
   uint8_t shader, start, count;
   uint8_t unbind_num_trailing_slots;
   struct pipe_image_view slot[0]; /* more will be allocated if needed */
};

static uint16_t
tc_call_set_shader_images(struct pipe_context *pipe, void *call)
{
   struct tc_shader_images *p = (struct tc_shader_images *)call;
   unsigned count = p->count;

   if (!p->count) {
      pipe->set_shader_images(pipe, p->shader, p->start, 0,
                              p->unbind_num_trailing_slots, NULL);
      return call_size(tc_shader_images);
   }

   pipe->set_shader_images(pipe, p->shader, p->start, p->count,
                           p->unbind_num_trailing_slots, p->slot);

   for (unsigned i = 0; i < count; i++)
      tc_drop_resource_reference(p->slot[i].resource);

   return p->base.num_slots;
}

static void
tc_set_shader_images(struct pipe_context *_pipe,
                     enum pipe_shader_type shader,
                     unsigned start, unsigned count,
                     unsigned unbind_num_trailing_slots,
                     const struct pipe_image_view *images)
{
   if (!count && !unbind_num_trailing_slots)
      return;

   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_shader_images *p =
      tc_add_slot_based_call(tc, TC_CALL_set_shader_images, tc_shader_images,
                             images ? count : 0);
   unsigned writable_buffers = 0;

   p->shader = shader;
   p->start = start;

   if (images) {
      p->count = count;
      p->unbind_num_trailing_slots = unbind_num_trailing_slots;

      struct tc_buffer_list *next = &tc->buffer_lists[tc->next_buf_list];

      for (unsigned i = 0; i < count; i++) {
         struct pipe_resource *resource = images[i].resource;

         tc_set_resource_reference(&p->slot[i].resource, resource);

         if (resource) {
            if (resource->target == PIPE_BUFFER) {
               tc_bind_buffer(&tc->image_buffers[shader][start + i], next, resource);

               if (images[i].access & PIPE_IMAGE_ACCESS_WRITE) {
                  struct threaded_resource *tres = threaded_resource(resource);

                  tc_buffer_disable_cpu_storage(resource);
                  util_range_add(&tres->b, &tres->valid_buffer_range,
                                 images[i].u.buf.offset,
                                 images[i].u.buf.offset + images[i].u.buf.size);
                  writable_buffers |= BITFIELD_BIT(start + i);
               }
            } else {
               tc_set_resource_batch_usage(tc, resource);
            }
         } else {
            tc_unbind_buffer(&tc->image_buffers[shader][start + i]);
         }
      }
      memcpy(p->slot, images, count * sizeof(images[0]));

      tc_unbind_buffers(&tc->image_buffers[shader][start + count],
                        unbind_num_trailing_slots);
      tc->seen_image_buffers[shader] = true;
   } else {
      p->count = 0;
      p->unbind_num_trailing_slots = count + unbind_num_trailing_slots;

      tc_unbind_buffers(&tc->image_buffers[shader][start],
                        count + unbind_num_trailing_slots);
   }

   tc->image_buffers_writeable_mask[shader] &= ~BITFIELD_RANGE(start, count);
   tc->image_buffers_writeable_mask[shader] |= writable_buffers;
}

struct tc_shader_buffers {
   struct tc_call_base base;
   uint8_t shader, start, count;
   bool unbind;
   unsigned writable_bitmask;
   struct pipe_shader_buffer slot[0]; /* more will be allocated if needed */
};

static uint16_t
tc_call_set_shader_buffers(struct pipe_context *pipe, void *call)
{
   struct tc_shader_buffers *p = (struct tc_shader_buffers *)call;
   unsigned count = p->count;

   if (p->unbind) {
      pipe->set_shader_buffers(pipe, p->shader, p->start, p->count, NULL, 0);
      return call_size(tc_shader_buffers);
   }

   pipe->set_shader_buffers(pipe, p->shader, p->start, p->count, p->slot,
                            p->writable_bitmask);

   for (unsigned i = 0; i < count; i++)
      tc_drop_resource_reference(p->slot[i].buffer);

   return p->base.num_slots;
}

static void
tc_set_shader_buffers(struct pipe_context *_pipe,
                      enum pipe_shader_type shader,
                      unsigned start, unsigned count,
                      const struct pipe_shader_buffer *buffers,
                      unsigned writable_bitmask)
{
   if (!count)
      return;

   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_shader_buffers *p =
      tc_add_slot_based_call(tc, TC_CALL_set_shader_buffers, tc_shader_buffers,
                             buffers ? count : 0);

   p->shader = shader;
   p->start = start;
   p->count = count;
   p->unbind = buffers == NULL;
   p->writable_bitmask = writable_bitmask;

   if (buffers) {
      struct tc_buffer_list *next = &tc->buffer_lists[tc->next_buf_list];

      for (unsigned i = 0; i < count; i++) {
         struct pipe_shader_buffer *dst = &p->slot[i];
         const struct pipe_shader_buffer *src = buffers + i;

         tc_set_resource_reference(&dst->buffer, src->buffer);
         dst->buffer_offset = src->buffer_offset;
         dst->buffer_size = src->buffer_size;

         if (src->buffer) {
            struct threaded_resource *tres = threaded_resource(src->buffer);

            tc_bind_buffer(&tc->shader_buffers[shader][start + i], next, &tres->b);

            if (writable_bitmask & BITFIELD_BIT(i)) {
               tc_buffer_disable_cpu_storage(src->buffer);
               util_range_add(&tres->b, &tres->valid_buffer_range,
                              src->buffer_offset,
                              src->buffer_offset + src->buffer_size);
            }
         } else {
            tc_unbind_buffer(&tc->shader_buffers[shader][start + i]);
         }
      }
      tc->seen_shader_buffers[shader] = true;
   } else {
      tc_unbind_buffers(&tc->shader_buffers[shader][start], count);
   }

   tc->shader_buffers_writeable_mask[shader] &= ~BITFIELD_RANGE(start, count);
   tc->shader_buffers_writeable_mask[shader] |= writable_bitmask << start;
}

struct tc_vertex_buffers {
   struct tc_call_base base;
   uint8_t count;
   uint8_t unbind_num_trailing_slots;
   struct pipe_vertex_buffer slot[0]; /* more will be allocated if needed */
};

static uint16_t
tc_call_set_vertex_buffers(struct pipe_context *pipe, void *call)
{
   struct tc_vertex_buffers *p = (struct tc_vertex_buffers *)call;
   unsigned count = p->count;

   if (!count) {
      pipe->set_vertex_buffers(pipe, 0, p->unbind_num_trailing_slots, false, NULL);
      return call_size(tc_vertex_buffers);
   }

   for (unsigned i = 0; i < count; i++)
      tc_assert(!p->slot[i].is_user_buffer);

   pipe->set_vertex_buffers(pipe, count, p->unbind_num_trailing_slots, true, p->slot);
   return p->base.num_slots;
}

static void
tc_set_vertex_buffers(struct pipe_context *_pipe,
                      unsigned count,
                      unsigned unbind_num_trailing_slots,
                      bool take_ownership,
                      const struct pipe_vertex_buffer *buffers)
{
   struct threaded_context *tc = threaded_context(_pipe);

   if (!count && !unbind_num_trailing_slots)
      return;

   if (count && buffers) {
      struct tc_vertex_buffers *p =
         tc_add_slot_based_call(tc, TC_CALL_set_vertex_buffers, tc_vertex_buffers, count);
      p->count = count;
      p->unbind_num_trailing_slots = unbind_num_trailing_slots;

      struct tc_buffer_list *next = &tc->buffer_lists[tc->next_buf_list];

      if (take_ownership) {
         memcpy(p->slot, buffers, count * sizeof(struct pipe_vertex_buffer));

         for (unsigned i = 0; i < count; i++) {
            struct pipe_resource *buf = buffers[i].buffer.resource;

            if (buf) {
               tc_bind_buffer(&tc->vertex_buffers[i], next, buf);
            } else {
               tc_unbind_buffer(&tc->vertex_buffers[i]);
            }
         }
      } else {
         for (unsigned i = 0; i < count; i++) {
            struct pipe_vertex_buffer *dst = &p->slot[i];
            const struct pipe_vertex_buffer *src = buffers + i;
            struct pipe_resource *buf = src->buffer.resource;

            tc_assert(!src->is_user_buffer);
            dst->is_user_buffer = false;
            tc_set_resource_reference(&dst->buffer.resource, buf);
            dst->buffer_offset = src->buffer_offset;

            if (buf) {
               tc_bind_buffer(&tc->vertex_buffers[i], next, buf);
            } else {
               tc_unbind_buffer(&tc->vertex_buffers[i]);
            }
         }
      }

      tc_unbind_buffers(&tc->vertex_buffers[count],
                        unbind_num_trailing_slots);
   } else {
      struct tc_vertex_buffers *p =
         tc_add_slot_based_call(tc, TC_CALL_set_vertex_buffers, tc_vertex_buffers, 0);
      p->count = 0;
      p->unbind_num_trailing_slots = count + unbind_num_trailing_slots;

      tc_unbind_buffers(&tc->vertex_buffers[0],
                        count + unbind_num_trailing_slots);
   }
}

struct tc_stream_outputs {
   struct tc_call_base base;
   unsigned count;
   struct pipe_stream_output_target *targets[PIPE_MAX_SO_BUFFERS];
   unsigned offsets[PIPE_MAX_SO_BUFFERS];
};

static uint16_t
tc_call_set_stream_output_targets(struct pipe_context *pipe, void *call)
{
   struct tc_stream_outputs *p = to_call(call, tc_stream_outputs);
   unsigned count = p->count;

   pipe->set_stream_output_targets(pipe, count, p->targets, p->offsets);
   for (unsigned i = 0; i < count; i++)
      tc_drop_so_target_reference(p->targets[i]);

   return call_size(tc_stream_outputs);
}

static void
tc_set_stream_output_targets(struct pipe_context *_pipe,
                             unsigned count,
                             struct pipe_stream_output_target **tgs,
                             const unsigned *offsets)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_stream_outputs *p =
      tc_add_call(tc, TC_CALL_set_stream_output_targets, tc_stream_outputs);
   struct tc_buffer_list *next = &tc->buffer_lists[tc->next_buf_list];

   for (unsigned i = 0; i < count; i++) {
      p->targets[i] = NULL;
      pipe_so_target_reference(&p->targets[i], tgs[i]);
      if (tgs[i]) {
         tc_buffer_disable_cpu_storage(tgs[i]->buffer);
         tc_bind_buffer(&tc->streamout_buffers[i], next, tgs[i]->buffer);
      } else {
         tc_unbind_buffer(&tc->streamout_buffers[i]);
      }
   }
   p->count = count;
   memcpy(p->offsets, offsets, count * sizeof(unsigned));

   tc_unbind_buffers(&tc->streamout_buffers[count], PIPE_MAX_SO_BUFFERS - count);
   if (count)
      tc->seen_streamout_buffers = true;
}

static void
tc_set_compute_resources(struct pipe_context *_pipe, unsigned start,
                         unsigned count, struct pipe_surface **resources)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc);
   pipe->set_compute_resources(pipe, start, count, resources);
}

static void
tc_set_global_binding(struct pipe_context *_pipe, unsigned first,
                      unsigned count, struct pipe_resource **resources,
                      uint32_t **handles)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc);
   pipe->set_global_binding(pipe, first, count, resources, handles);
}


/********************************************************************
 * views
 */

static struct pipe_surface *
tc_create_surface(struct pipe_context *_pipe,
                  struct pipe_resource *resource,
                  const struct pipe_surface *surf_tmpl)
{
   struct pipe_context *pipe = threaded_context(_pipe)->pipe;
   struct pipe_surface *view =
         pipe->create_surface(pipe, resource, surf_tmpl);

   if (view)
      view->context = _pipe;
   return view;
}

static void
tc_surface_destroy(struct pipe_context *_pipe,
                   struct pipe_surface *surf)
{
   struct pipe_context *pipe = threaded_context(_pipe)->pipe;

   pipe->surface_destroy(pipe, surf);
}

static struct pipe_sampler_view *
tc_create_sampler_view(struct pipe_context *_pipe,
                       struct pipe_resource *resource,
                       const struct pipe_sampler_view *templ)
{
   struct pipe_context *pipe = threaded_context(_pipe)->pipe;
   struct pipe_sampler_view *view =
         pipe->create_sampler_view(pipe, resource, templ);

   if (view)
      view->context = _pipe;
   return view;
}

static void
tc_sampler_view_destroy(struct pipe_context *_pipe,
                        struct pipe_sampler_view *view)
{
   struct pipe_context *pipe = threaded_context(_pipe)->pipe;

   pipe->sampler_view_destroy(pipe, view);
}

static struct pipe_stream_output_target *
tc_create_stream_output_target(struct pipe_context *_pipe,
                               struct pipe_resource *res,
                               unsigned buffer_offset,
                               unsigned buffer_size)
{
   struct pipe_context *pipe = threaded_context(_pipe)->pipe;
   struct threaded_resource *tres = threaded_resource(res);
   struct pipe_stream_output_target *view;

   util_range_add(&tres->b, &tres->valid_buffer_range, buffer_offset,
                  buffer_offset + buffer_size);

   view = pipe->create_stream_output_target(pipe, res, buffer_offset,
                                            buffer_size);
   if (view)
      view->context = _pipe;
   return view;
}

static void
tc_stream_output_target_destroy(struct pipe_context *_pipe,
                                struct pipe_stream_output_target *target)
{
   struct pipe_context *pipe = threaded_context(_pipe)->pipe;

   pipe->stream_output_target_destroy(pipe, target);
}


/********************************************************************
 * bindless
 */

static uint64_t
tc_create_texture_handle(struct pipe_context *_pipe,
                         struct pipe_sampler_view *view,
                         const struct pipe_sampler_state *state)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc);
   return pipe->create_texture_handle(pipe, view, state);
}

struct tc_make_texture_handle_resident {
   struct tc_call_base base;
   bool resident;
   uint64_t handle;
};

static uint16_t
tc_call_make_texture_handle_resident(struct pipe_context *pipe, void *call)
{
   struct tc_make_texture_handle_resident *p =
      to_call(call, tc_make_texture_handle_resident);

   pipe->make_texture_handle_resident(pipe, p->handle, p->resident);
   return call_size(tc_make_texture_handle_resident);
}

static void
tc_make_texture_handle_resident(struct pipe_context *_pipe, uint64_t handle,
                                bool resident)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_make_texture_handle_resident *p =
      tc_add_call(tc, TC_CALL_make_texture_handle_resident,
                  tc_make_texture_handle_resident);

   p->handle = handle;
   p->resident = resident;
}

static uint64_t
tc_create_image_handle(struct pipe_context *_pipe,
                       const struct pipe_image_view *image)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   if (image->resource->target == PIPE_BUFFER)
      tc_buffer_disable_cpu_storage(image->resource);

   tc_sync(tc);
   return pipe->create_image_handle(pipe, image);
}

struct tc_make_image_handle_resident {
   struct tc_call_base base;
   bool resident;
   unsigned access;
   uint64_t handle;
};

static uint16_t
tc_call_make_image_handle_resident(struct pipe_context *pipe, void *call)
{
   struct tc_make_image_handle_resident *p =
      to_call(call, tc_make_image_handle_resident);

   pipe->make_image_handle_resident(pipe, p->handle, p->access, p->resident);
   return call_size(tc_make_image_handle_resident);
}

static void
tc_make_image_handle_resident(struct pipe_context *_pipe, uint64_t handle,
                              unsigned access, bool resident)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_make_image_handle_resident *p =
      tc_add_call(tc, TC_CALL_make_image_handle_resident,
                  tc_make_image_handle_resident);

   p->handle = handle;
   p->access = access;
   p->resident = resident;
}


/********************************************************************
 * transfer
 */

struct tc_replace_buffer_storage {
   struct tc_call_base base;
   uint16_t num_rebinds;
   uint32_t rebind_mask;
   uint32_t delete_buffer_id;
   struct pipe_resource *dst;
   struct pipe_resource *src;
   tc_replace_buffer_storage_func func;
};

static uint16_t
tc_call_replace_buffer_storage(struct pipe_context *pipe, void *call)
{
   struct tc_replace_buffer_storage *p = to_call(call, tc_replace_buffer_storage);

   p->func(pipe, p->dst, p->src, p->num_rebinds, p->rebind_mask, p->delete_buffer_id);

   tc_drop_resource_reference(p->dst);
   tc_drop_resource_reference(p->src);
   return call_size(tc_replace_buffer_storage);
}

/* Return true if the buffer has been invalidated or is idle. */
static bool
tc_invalidate_buffer(struct threaded_context *tc,
                     struct threaded_resource *tbuf)
{
   if (!tc_is_buffer_busy(tc, tbuf, PIPE_MAP_READ_WRITE)) {
      /* It's idle, so invalidation would be a no-op, but we can still clear
       * the valid range because we are technically doing invalidation, but
       * skipping it because it's useless.
       *
       * If the buffer is bound for write, we can't invalidate the range.
       */
      if (!tc_is_buffer_bound_for_write(tc, tbuf->buffer_id_unique))
         util_range_set_empty(&tbuf->valid_buffer_range);
      return true;
   }

   struct pipe_screen *screen = tc->base.screen;
   struct pipe_resource *new_buf;

   /* Shared, pinned, and sparse buffers can't be reallocated. */
   if (tbuf->is_shared ||
       tbuf->is_user_ptr ||
       tbuf->b.flags & (PIPE_RESOURCE_FLAG_SPARSE | PIPE_RESOURCE_FLAG_UNMAPPABLE))
      return false;

   /* Allocate a new one. */
   new_buf = screen->resource_create(screen, &tbuf->b);
   if (!new_buf)
      return false;

   /* Replace the "latest" pointer. */
   if (tbuf->latest != &tbuf->b)
      pipe_resource_reference(&tbuf->latest, NULL);

   tbuf->latest = new_buf;

   uint32_t delete_buffer_id = tbuf->buffer_id_unique;

   /* Enqueue storage replacement of the original buffer. */
   struct tc_replace_buffer_storage *p =
      tc_add_call(tc, TC_CALL_replace_buffer_storage,
                  tc_replace_buffer_storage);

   p->func = tc->replace_buffer_storage;
   tc_set_resource_reference(&p->dst, &tbuf->b);
   tc_set_resource_reference(&p->src, new_buf);
   p->delete_buffer_id = delete_buffer_id;
   p->rebind_mask = 0;

   /* Treat the current buffer as the new buffer. */
   bool bound_for_write = tc_is_buffer_bound_for_write(tc, tbuf->buffer_id_unique);
   p->num_rebinds = tc_rebind_buffer(tc, tbuf->buffer_id_unique,
                                     threaded_resource(new_buf)->buffer_id_unique,
                                     &p->rebind_mask);

   /* If the buffer is not bound for write, clear the valid range. */
   if (!bound_for_write)
      util_range_set_empty(&tbuf->valid_buffer_range);

   tbuf->buffer_id_unique = threaded_resource(new_buf)->buffer_id_unique;
   threaded_resource(new_buf)->buffer_id_unique = 0;

   return true;
}

static unsigned
tc_improve_map_buffer_flags(struct threaded_context *tc,
                            struct threaded_resource *tres, unsigned usage,
                            unsigned offset, unsigned size)
{
   /* Never invalidate inside the driver and never infer "unsynchronized". */
   unsigned tc_flags = TC_TRANSFER_MAP_NO_INVALIDATE |
                       TC_TRANSFER_MAP_NO_INFER_UNSYNCHRONIZED;

   /* Prevent a reentry. */
   if (usage & tc_flags)
      return usage;

   /* Use the staging upload if it's preferred. */
   if (usage & (PIPE_MAP_DISCARD_RANGE |
                PIPE_MAP_DISCARD_WHOLE_RESOURCE) &&
       !(usage & PIPE_MAP_PERSISTENT) &&
       tres->b.flags & PIPE_RESOURCE_FLAG_DONT_MAP_DIRECTLY &&
       tc->use_forced_staging_uploads) {
      usage &= ~(PIPE_MAP_DISCARD_WHOLE_RESOURCE |
                 PIPE_MAP_UNSYNCHRONIZED);

      return usage | tc_flags | PIPE_MAP_DISCARD_RANGE;
   }

   /* Sparse buffers can't be mapped directly and can't be reallocated
    * (fully invalidated). That may just be a radeonsi limitation, but
    * the threaded context must obey it with radeonsi.
    */
   if (tres->b.flags & (PIPE_RESOURCE_FLAG_SPARSE | PIPE_RESOURCE_FLAG_UNMAPPABLE)) {
      /* We can use DISCARD_RANGE instead of full discard. This is the only
       * fast path for sparse buffers that doesn't need thread synchronization.
       */
      if (usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE)
         usage |= PIPE_MAP_DISCARD_RANGE;

      /* Allow DISCARD_WHOLE_RESOURCE and infering UNSYNCHRONIZED in drivers.
       * The threaded context doesn't do unsychronized mappings and invalida-
       * tions of sparse buffers, therefore a correct driver behavior won't
       * result in an incorrect behavior with the threaded context.
       */
      return usage;
   }

   usage |= tc_flags;

   /* Handle CPU reads trivially. */
   if (usage & PIPE_MAP_READ) {
      if (usage & PIPE_MAP_UNSYNCHRONIZED)
         usage |= TC_TRANSFER_MAP_THREADED_UNSYNC; /* don't sync */

      /* Drivers aren't allowed to do buffer invalidations. */
      return usage & ~PIPE_MAP_DISCARD_WHOLE_RESOURCE;
   }

   /* See if the buffer range being mapped has never been initialized or
    * the buffer is idle, in which case it can be mapped unsynchronized. */
   if (!(usage & PIPE_MAP_UNSYNCHRONIZED) &&
       ((!tres->is_shared &&
         !util_ranges_intersect(&tres->valid_buffer_range, offset, offset + size)) ||
        !tc_is_buffer_busy(tc, tres, usage)))
      usage |= PIPE_MAP_UNSYNCHRONIZED;

   if (!(usage & PIPE_MAP_UNSYNCHRONIZED)) {
      /* If discarding the entire range, discard the whole resource instead. */
      if (usage & PIPE_MAP_DISCARD_RANGE &&
          offset == 0 && size == tres->b.width0)
         usage |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;

      /* Discard the whole resource if needed. */
      if (usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE) {
         if (tc_invalidate_buffer(tc, tres))
            usage |= PIPE_MAP_UNSYNCHRONIZED;
         else
            usage |= PIPE_MAP_DISCARD_RANGE; /* fallback */
      }
   }

   /* We won't need this flag anymore. */
   /* TODO: We might not need TC_TRANSFER_MAP_NO_INVALIDATE with this. */
   usage &= ~PIPE_MAP_DISCARD_WHOLE_RESOURCE;

   /* GL_AMD_pinned_memory and persistent mappings can't use staging
    * buffers. */
   if (usage & (PIPE_MAP_UNSYNCHRONIZED |
                PIPE_MAP_PERSISTENT) ||
       tres->is_user_ptr)
      usage &= ~PIPE_MAP_DISCARD_RANGE;

   /* Unsychronized buffer mappings don't have to synchronize the thread. */
   if (usage & PIPE_MAP_UNSYNCHRONIZED) {
      usage &= ~PIPE_MAP_DISCARD_RANGE;
      usage |= TC_TRANSFER_MAP_THREADED_UNSYNC; /* notify the driver */
   }

   return usage;
}

static void *
tc_buffer_map(struct pipe_context *_pipe,
              struct pipe_resource *resource, unsigned level,
              unsigned usage, const struct pipe_box *box,
              struct pipe_transfer **transfer)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct threaded_resource *tres = threaded_resource(resource);
   struct pipe_context *pipe = tc->pipe;

   /* PIPE_MAP_THREAD_SAFE is for glthread, which shouldn't use the CPU storage and
    * this shouldn't normally be necessary because glthread only uses large buffers.
    */
   if (usage & PIPE_MAP_THREAD_SAFE)
      tc_buffer_disable_cpu_storage(resource);

   usage = tc_improve_map_buffer_flags(tc, tres, usage, box->x, box->width);

   /* If the CPU storage is enabled, return it directly. */
   if (tres->allow_cpu_storage && !(usage & TC_TRANSFER_MAP_UPLOAD_CPU_STORAGE)) {
      /* We can't let resource_copy_region disable the CPU storage. */
      assert(!(tres->b.flags & PIPE_RESOURCE_FLAG_DONT_MAP_DIRECTLY));

      if (!tres->cpu_storage) {
         tres->cpu_storage = align_malloc(resource->width0, tc->map_buffer_alignment);

         if (tres->cpu_storage && tres->valid_buffer_range.end) {
            /* The GPU buffer contains valid data. Copy them to the CPU storage. */
            struct pipe_box box2;
            struct pipe_transfer *transfer2;

            unsigned valid_range_len = tres->valid_buffer_range.end - tres->valid_buffer_range.start;
            u_box_1d(tres->valid_buffer_range.start, valid_range_len, &box2);

            tc_sync_msg(tc, "cpu storage GPU -> CPU copy");
            tc_set_driver_thread(tc);

            void *ret = pipe->buffer_map(pipe, tres->latest ? tres->latest : resource,
                                         0, PIPE_MAP_READ, &box2, &transfer2);
            memcpy(&((uint8_t*)tres->cpu_storage)[tres->valid_buffer_range.start],
                   ret,
                   valid_range_len);
            pipe->buffer_unmap(pipe, transfer2);

            tc_clear_driver_thread(tc);
         }
      }

      if (tres->cpu_storage) {
         struct threaded_transfer *ttrans = slab_zalloc(&tc->pool_transfers);
         ttrans->b.resource = resource;
         ttrans->b.usage = usage;
         ttrans->b.box = *box;
         ttrans->valid_buffer_range = &tres->valid_buffer_range;
         ttrans->cpu_storage_mapped = true;
         *transfer = &ttrans->b;

         return (uint8_t*)tres->cpu_storage + box->x;
      } else {
         tres->allow_cpu_storage = false;
      }
   }

   /* Do a staging transfer within the threaded context. The driver should
    * only get resource_copy_region.
    */
   if (usage & PIPE_MAP_DISCARD_RANGE) {
      struct threaded_transfer *ttrans = slab_zalloc(&tc->pool_transfers);
      uint8_t *map;

      u_upload_alloc(tc->base.stream_uploader, 0,
                     box->width + (box->x % tc->map_buffer_alignment),
                     tc->map_buffer_alignment, &ttrans->b.offset,
                     &ttrans->staging, (void**)&map);
      if (!map) {
         slab_free(&tc->pool_transfers, ttrans);
         return NULL;
      }

      ttrans->b.resource = resource;
      ttrans->b.level = 0;
      ttrans->b.usage = usage;
      ttrans->b.box = *box;
      ttrans->b.stride = 0;
      ttrans->b.layer_stride = 0;
      ttrans->valid_buffer_range = &tres->valid_buffer_range;
      ttrans->cpu_storage_mapped = false;
      *transfer = &ttrans->b;

      p_atomic_inc(&tres->pending_staging_uploads);
      util_range_add(resource, &tres->pending_staging_uploads_range,
                     box->x, box->x + box->width);

      return map + (box->x % tc->map_buffer_alignment);
   }

   if (usage & PIPE_MAP_UNSYNCHRONIZED &&
       p_atomic_read(&tres->pending_staging_uploads) &&
       util_ranges_intersect(&tres->pending_staging_uploads_range, box->x, box->x + box->width)) {
      /* Write conflict detected between a staging transfer and the direct mapping we're
       * going to do. Resolve the conflict by ignoring UNSYNCHRONIZED so the direct mapping
       * will have to wait for the staging transfer completion.
       * Note: The conflict detection is only based on the mapped range, not on the actual
       * written range(s).
       */
      usage &= ~PIPE_MAP_UNSYNCHRONIZED & ~TC_TRANSFER_MAP_THREADED_UNSYNC;
      tc->use_forced_staging_uploads = false;
   }

   /* Unsychronized buffer mappings don't have to synchronize the thread. */
   if (!(usage & TC_TRANSFER_MAP_THREADED_UNSYNC)) {
      tc_sync_msg(tc, usage & PIPE_MAP_DISCARD_RANGE ? "  discard_range" :
                      usage & PIPE_MAP_READ ? "  read" : "  staging conflict");
      tc_set_driver_thread(tc);
   }

   tc->bytes_mapped_estimate += box->width;

   void *ret = pipe->buffer_map(pipe, tres->latest ? tres->latest : resource,
                                level, usage, box, transfer);
   threaded_transfer(*transfer)->valid_buffer_range = &tres->valid_buffer_range;
   threaded_transfer(*transfer)->cpu_storage_mapped = false;

   if (!(usage & TC_TRANSFER_MAP_THREADED_UNSYNC))
      tc_clear_driver_thread(tc);

   return ret;
}

static void *
tc_texture_map(struct pipe_context *_pipe,
               struct pipe_resource *resource, unsigned level,
               unsigned usage, const struct pipe_box *box,
               struct pipe_transfer **transfer)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct threaded_resource *tres = threaded_resource(resource);
   struct pipe_context *pipe = tc->pipe;

   tc_sync_msg(tc, "texture");
   tc_set_driver_thread(tc);
   /* block all unsync texture subdata during map */
   tc_set_resource_batch_usage_persistent(tc, resource, true);

   tc->bytes_mapped_estimate += box->width;

   void *ret = pipe->texture_map(pipe, tres->latest ? tres->latest : resource,
                                 level, usage, box, transfer);

   if (!(usage & TC_TRANSFER_MAP_THREADED_UNSYNC))
      tc_clear_driver_thread(tc);

   return ret;
}

struct tc_transfer_flush_region {
   struct tc_call_base base;
   struct pipe_box box;
   struct pipe_transfer *transfer;
};

static uint16_t
tc_call_transfer_flush_region(struct pipe_context *pipe, void *call)
{
   struct tc_transfer_flush_region *p = to_call(call, tc_transfer_flush_region);

   pipe->transfer_flush_region(pipe, p->transfer, &p->box);
   return call_size(tc_transfer_flush_region);
}

struct tc_resource_copy_region {
   struct tc_call_base base;
   unsigned dst_level;
   unsigned dstx, dsty, dstz;
   unsigned src_level;
   struct pipe_box src_box;
   struct pipe_resource *dst;
   struct pipe_resource *src;
};

static void
tc_resource_copy_region(struct pipe_context *_pipe,
                        struct pipe_resource *dst, unsigned dst_level,
                        unsigned dstx, unsigned dsty, unsigned dstz,
                        struct pipe_resource *src, unsigned src_level,
                        const struct pipe_box *src_box);

static void
tc_buffer_do_flush_region(struct threaded_context *tc,
                          struct threaded_transfer *ttrans,
                          const struct pipe_box *box)
{
   struct threaded_resource *tres = threaded_resource(ttrans->b.resource);

   if (ttrans->staging) {
      struct pipe_box src_box;

      u_box_1d(ttrans->b.offset + ttrans->b.box.x % tc->map_buffer_alignment +
               (box->x - ttrans->b.box.x),
               box->width, &src_box);

      /* Copy the staging buffer into the original one. */
      tc_resource_copy_region(&tc->base, ttrans->b.resource, 0, box->x, 0, 0,
                              ttrans->staging, 0, &src_box);
   }

   /* Don't update the valid range when we're uploading the CPU storage
    * because it includes the uninitialized range too.
    */
   if (!(ttrans->b.usage & TC_TRANSFER_MAP_UPLOAD_CPU_STORAGE)) {
      util_range_add(&tres->b, ttrans->valid_buffer_range,
                     box->x, box->x + box->width);
   }
}

static void
tc_transfer_flush_region(struct pipe_context *_pipe,
                         struct pipe_transfer *transfer,
                         const struct pipe_box *rel_box)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct threaded_transfer *ttrans = threaded_transfer(transfer);
   struct threaded_resource *tres = threaded_resource(transfer->resource);
   unsigned required_usage = PIPE_MAP_WRITE |
                             PIPE_MAP_FLUSH_EXPLICIT;

   if (tres->b.target == PIPE_BUFFER) {
      if ((transfer->usage & required_usage) == required_usage) {
         struct pipe_box box;

         u_box_1d(transfer->box.x + rel_box->x, rel_box->width, &box);
         tc_buffer_do_flush_region(tc, ttrans, &box);
      }

      /* Staging transfers don't send the call to the driver.
       *
       * Transfers using the CPU storage shouldn't call transfer_flush_region
       * in the driver because the buffer is not really mapped on the driver
       * side and the CPU storage always re-uploads everything (flush_region
       * makes no difference).
       */
      if (ttrans->staging || ttrans->cpu_storage_mapped)
         return;
   }

   struct tc_transfer_flush_region *p =
      tc_add_call(tc, TC_CALL_transfer_flush_region, tc_transfer_flush_region);
   p->transfer = transfer;
   p->box = *rel_box;
}

static void
tc_flush(struct pipe_context *_pipe, struct pipe_fence_handle **fence,
         unsigned flags);

struct tc_buffer_unmap {
   struct tc_call_base base;
   bool was_staging_transfer;
   union {
      struct pipe_transfer *transfer;
      struct pipe_resource *resource;
   };
};

static uint16_t
tc_call_buffer_unmap(struct pipe_context *pipe, void *call)
{
   struct tc_buffer_unmap *p = to_call(call, tc_buffer_unmap);

   if (p->was_staging_transfer) {
      struct threaded_resource *tres = threaded_resource(p->resource);
      /* Nothing to do except keeping track of staging uploads */
      assert(tres->pending_staging_uploads > 0);
      p_atomic_dec(&tres->pending_staging_uploads);
      tc_drop_resource_reference(p->resource);
   } else {
      pipe->buffer_unmap(pipe, p->transfer);
   }

   return call_size(tc_buffer_unmap);
}

static void
tc_buffer_unmap(struct pipe_context *_pipe, struct pipe_transfer *transfer)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct threaded_transfer *ttrans = threaded_transfer(transfer);
   struct threaded_resource *tres = threaded_resource(transfer->resource);

   /* PIPE_MAP_THREAD_SAFE is only valid with UNSYNCHRONIZED. It can be
    * called from any thread and bypasses all multithreaded queues.
    */
   if (transfer->usage & PIPE_MAP_THREAD_SAFE) {
      assert(transfer->usage & PIPE_MAP_UNSYNCHRONIZED);
      assert(!(transfer->usage & (PIPE_MAP_FLUSH_EXPLICIT |
                                  PIPE_MAP_DISCARD_RANGE)));

      struct pipe_context *pipe = tc->pipe;
      util_range_add(&tres->b, ttrans->valid_buffer_range,
                      transfer->box.x, transfer->box.x + transfer->box.width);

      pipe->buffer_unmap(pipe, transfer);
      return;
   }

   if (transfer->usage & PIPE_MAP_WRITE &&
       !(transfer->usage & PIPE_MAP_FLUSH_EXPLICIT))
      tc_buffer_do_flush_region(tc, ttrans, &transfer->box);

   if (ttrans->cpu_storage_mapped) {
      /* GL allows simultaneous GPU stores with mapped buffers as long as GPU stores don't
       * touch the mapped range. That's a problem because GPU stores free the CPU storage.
       * If that happens, we just ignore the unmap call and don't upload anything to prevent
       * a crash.
       *
       * Disallow the CPU storage in the driver to work around this.
       */
      assert(tres->cpu_storage);

      if (tres->cpu_storage) {
         tc_invalidate_buffer(tc, tres);
         tc_buffer_subdata(&tc->base, &tres->b,
                           PIPE_MAP_UNSYNCHRONIZED |
                           TC_TRANSFER_MAP_UPLOAD_CPU_STORAGE,
                           0, tres->b.width0, tres->cpu_storage);
         /* This shouldn't have been freed by buffer_subdata. */
         assert(tres->cpu_storage);
      } else {
         static bool warned_once = false;
         if (!warned_once) {
            fprintf(stderr, "This application is incompatible with cpu_storage.\n");
            fprintf(stderr, "Use tc_max_cpu_storage_size=0 to disable it and report this issue to Mesa.\n");
            warned_once = true;
         }
      }

      tc_drop_resource_reference(ttrans->staging);
      slab_free(&tc->pool_transfers, ttrans);
      return;
   }

   bool was_staging_transfer = false;

   if (ttrans->staging) {
      was_staging_transfer = true;

      tc_drop_resource_reference(ttrans->staging);
      slab_free(&tc->pool_transfers, ttrans);
   }

   struct tc_buffer_unmap *p = tc_add_call(tc, TC_CALL_buffer_unmap,
                                           tc_buffer_unmap);
   if (was_staging_transfer) {
      tc_set_resource_reference(&p->resource, &tres->b);
      p->was_staging_transfer = true;
   } else {
      p->transfer = transfer;
      p->was_staging_transfer = false;
   }

   /* tc_buffer_map directly maps the buffers, but tc_buffer_unmap
    * defers the unmap operation to the batch execution.
    * bytes_mapped_estimate is an estimation of the map/unmap bytes delta
    * and if it goes over an optional limit the current batch is flushed,
    * to reclaim some RAM. */
   if (!ttrans->staging && tc->bytes_mapped_limit &&
       tc->bytes_mapped_estimate > tc->bytes_mapped_limit) {
      tc_flush(_pipe, NULL, PIPE_FLUSH_ASYNC);
   }
}

struct tc_texture_unmap {
   struct tc_call_base base;
   struct pipe_transfer *transfer;
};

static uint16_t
tc_call_texture_unmap(struct pipe_context *pipe, void *call)
{
   struct tc_texture_unmap *p = (struct tc_texture_unmap *) call;

   pipe->texture_unmap(pipe, p->transfer);
   return call_size(tc_texture_unmap);
}

static void
tc_texture_unmap(struct pipe_context *_pipe, struct pipe_transfer *transfer)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct threaded_transfer *ttrans = threaded_transfer(transfer);

   /* enable subdata again once resource is no longer mapped */
   tc_set_resource_batch_usage_persistent(tc, transfer->resource, false);

   tc_add_call(tc, TC_CALL_texture_unmap, tc_texture_unmap)->transfer = transfer;

   /* tc_texture_map directly maps the textures, but tc_texture_unmap
    * defers the unmap operation to the batch execution.
    * bytes_mapped_estimate is an estimation of the map/unmap bytes delta
    * and if it goes over an optional limit the current batch is flushed,
    * to reclaim some RAM. */
   if (!ttrans->staging && tc->bytes_mapped_limit &&
       tc->bytes_mapped_estimate > tc->bytes_mapped_limit) {
      tc_flush(_pipe, NULL, PIPE_FLUSH_ASYNC);
   }
}

struct tc_buffer_subdata {
   struct tc_call_base base;
   unsigned usage, offset, size;
   struct pipe_resource *resource;
   char slot[0]; /* more will be allocated if needed */
};

static uint16_t
tc_call_buffer_subdata(struct pipe_context *pipe, void *call)
{
   struct tc_buffer_subdata *p = (struct tc_buffer_subdata *)call;

   pipe->buffer_subdata(pipe, p->resource, p->usage, p->offset, p->size,
                        p->slot);
   tc_drop_resource_reference(p->resource);
   return p->base.num_slots;
}

static bool
is_mergeable_buffer_subdata(const struct tc_call_base *previous_call,
                            unsigned usage, unsigned offset,
                            struct pipe_resource *resource)
{
   if (!previous_call || previous_call->call_id != TC_CALL_buffer_subdata)
      return false;

   struct tc_buffer_subdata *subdata = (struct tc_buffer_subdata *)previous_call;

   return subdata->usage == usage && subdata->resource == resource
          && (subdata->offset + subdata->size) == offset;
}

static void
tc_buffer_subdata(struct pipe_context *_pipe,
                  struct pipe_resource *resource,
                  unsigned usage, unsigned offset,
                  unsigned size, const void *data)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct threaded_resource *tres = threaded_resource(resource);

   if (!size)
      return;

   usage |= PIPE_MAP_WRITE;

   /* PIPE_MAP_DIRECTLY supresses implicit DISCARD_RANGE. */
   if (!(usage & PIPE_MAP_DIRECTLY))
      usage |= PIPE_MAP_DISCARD_RANGE;

   usage = tc_improve_map_buffer_flags(tc, tres, usage, offset, size);

   /* Unsychronized and big transfers should use transfer_map. Also handle
    * full invalidations, because drivers aren't allowed to do them.
    */
   if (usage & (PIPE_MAP_UNSYNCHRONIZED |
                PIPE_MAP_DISCARD_WHOLE_RESOURCE) ||
       size > TC_MAX_SUBDATA_BYTES ||
       tres->cpu_storage) {
      struct pipe_transfer *transfer;
      struct pipe_box box;
      uint8_t *map = NULL;

      u_box_1d(offset, size, &box);

      /* CPU storage is only useful for partial updates. It can add overhead
       * on glBufferData calls so avoid using it.
       */
      if (!tres->cpu_storage && offset == 0 && size == resource->width0)
         usage |= TC_TRANSFER_MAP_UPLOAD_CPU_STORAGE;

      map = tc_buffer_map(_pipe, resource, 0, usage, &box, &transfer);
      if (map) {
         memcpy(map, data, size);
         tc_buffer_unmap(_pipe, transfer);
      }
      return;
   }

   util_range_add(&tres->b, &tres->valid_buffer_range, offset, offset + size);

   /* We can potentially merge this subdata call with the previous one (if any),
    * if the application does a whole-buffer upload piecewise. */
   {
      struct tc_call_base *last_call = tc_get_last_mergeable_call(tc);
      struct tc_buffer_subdata *merge_dest = (struct tc_buffer_subdata *)last_call;

      if (is_mergeable_buffer_subdata(last_call, usage, offset, resource) &&
         tc_enlarge_last_mergeable_call(tc, call_size_with_slots(tc_buffer_subdata, merge_dest->size + size))) {
         memcpy(merge_dest->slot + merge_dest->size, data, size);
         merge_dest->size += size;

         /* TODO: We *could* do an invalidate + upload here if we detect that
          * the merged subdata call overwrites the entire buffer. However, that's
          * a little complicated since we can't add further calls to our batch
          * until we have removed the merged subdata call, which means that
          * calling tc_invalidate_buffer before we have removed the call will
          * blow things up.
          * 
          * Just leave a large, merged subdata call in the batch for now, which is
          * at least better than tons of tiny subdata calls.
          */

         return;
      }
   }

   /* The upload is small. Enqueue it. */
   struct tc_buffer_subdata *p =
      tc_add_slot_based_call(tc, TC_CALL_buffer_subdata, tc_buffer_subdata, size);

   tc_set_resource_reference(&p->resource, resource);
   /* This is will always be busy because if it wasn't, tc_improve_map_buffer-
    * _flags would set UNSYNCHRONIZED and we wouldn't get here.
    */
   tc_add_to_buffer_list(&tc->buffer_lists[tc->next_buf_list], resource);
   p->usage = usage;
   p->offset = offset;
   p->size = size;
   memcpy(p->slot, data, size);

   tc_mark_call_mergeable(tc, &p->base);
}

struct tc_texture_subdata {
   struct tc_call_base base;
   unsigned level, usage, stride;
   struct pipe_box box;
   struct pipe_resource *resource;
   uintptr_t layer_stride;
   char slot[0]; /* more will be allocated if needed */
};

static uint16_t
tc_call_texture_subdata(struct pipe_context *pipe, void *call)
{
   struct tc_texture_subdata *p = (struct tc_texture_subdata *)call;

   pipe->texture_subdata(pipe, p->resource, p->level, p->usage, &p->box,
                         p->slot, p->stride, p->layer_stride);
   tc_drop_resource_reference(p->resource);
   return p->base.num_slots;
}

static void
tc_texture_subdata(struct pipe_context *_pipe,
                   struct pipe_resource *resource,
                   unsigned level, unsigned usage,
                   const struct pipe_box *box,
                   const void *data, unsigned stride,
                   uintptr_t layer_stride)
{
   struct threaded_context *tc = threaded_context(_pipe);
   uint64_t size;

   assert(box->height >= 1);
   assert(box->depth >= 1);

   size = (box->depth - 1) * layer_stride +
          (box->height - 1) * (uint64_t)stride +
          box->width * util_format_get_blocksize(resource->format);
   if (!size)
      return;

   /* Small uploads can be enqueued, big uploads must sync. */
   if (size <= TC_MAX_SUBDATA_BYTES) {
      struct tc_texture_subdata *p =
         tc_add_slot_based_call(tc, TC_CALL_texture_subdata, tc_texture_subdata, size);

      tc_set_resource_batch_usage(tc, resource);
      tc_set_resource_reference(&p->resource, resource);
      p->level = level;
      p->usage = usage;
      p->box = *box;
      p->stride = stride;
      p->layer_stride = layer_stride;
      memcpy(p->slot, data, size);
   } else {
      struct pipe_context *pipe = tc->pipe;
      struct threaded_resource *tres = threaded_resource(resource);
      unsigned unsync_usage = TC_TRANSFER_MAP_THREADED_UNSYNC | PIPE_MAP_UNSYNCHRONIZED | PIPE_MAP_WRITE;
      bool can_unsync = !tc_resource_batch_usage_test_busy(tc, resource) &&
                        tc->options.is_resource_busy &&
                        !tc->options.is_resource_busy(tc->pipe->screen, tres->latest, usage | unsync_usage);

      if (!can_unsync && resource->usage != PIPE_USAGE_STAGING &&
          tc->options.parse_renderpass_info && tc->in_renderpass) {
         enum pipe_format format = resource->format;
         if (usage & PIPE_MAP_DEPTH_ONLY)
            format = util_format_get_depth_only(format);
         else if (usage & PIPE_MAP_STENCIL_ONLY)
            format = PIPE_FORMAT_S8_UINT;

         unsigned fmt_stride = util_format_get_stride(format, box->width);
         uint64_t fmt_layer_stride = util_format_get_2d_size(format, stride, box->height);
         assert(fmt_layer_stride * box->depth <= UINT32_MAX);

         struct pipe_resource *pres = pipe_buffer_create(pipe->screen, 0, PIPE_USAGE_STREAM, layer_stride * box->depth);
         pipe->buffer_subdata(pipe, pres, unsync_usage, 0, layer_stride * box->depth, data);
         struct pipe_box src_box = *box;
         src_box.x = src_box.y = src_box.z = 0;

         if (fmt_stride == stride && fmt_layer_stride == layer_stride) {
            /* if stride matches, single copy is fine*/
            tc->base.resource_copy_region(&tc->base, resource, level, box->x, box->y, box->z, pres, 0, &src_box);
         } else {
            /* if stride doesn't match, inline util_copy_box on the GPU and assume the driver will optimize */
            src_box.depth = 1;
            for (unsigned z = 0; z < box->depth; ++z, src_box.x = z * layer_stride) {
               unsigned dst_x = box->x, dst_y = box->y, width = box->width, height = box->height, dst_z = box->z + z;
               int blocksize = util_format_get_blocksize(format);
               int blockwidth = util_format_get_blockwidth(format);
               int blockheight = util_format_get_blockheight(format);

               assert(blocksize > 0);
               assert(blockwidth > 0);
               assert(blockheight > 0);

               dst_x /= blockwidth;
               dst_y /= blockheight;
               width = DIV_ROUND_UP(width, blockwidth);
               height = DIV_ROUND_UP(height, blockheight);

               width *= blocksize;

               if (width == fmt_stride && width == (unsigned)stride) {
                  ASSERTED uint64_t size = (uint64_t)height * width;

                  assert(size <= SIZE_MAX);
                  assert(dst_x + src_box.width < u_minify(pres->width0, level));
                  assert(dst_y + src_box.height < u_minify(pres->height0, level));
                  assert(pres->target != PIPE_TEXTURE_3D ||  z + src_box.depth < u_minify(pres->depth0, level));
                  tc->base.resource_copy_region(&tc->base, resource, level, dst_x, dst_y, dst_z, pres, 0, &src_box);
               } else {
                  src_box.height = 1;
                  for (unsigned i = 0; i < height; i++, dst_y++, src_box.x += stride)
                     tc->base.resource_copy_region(&tc->base, resource, level, dst_x, dst_y, dst_z, pres, 0, &src_box);
               }
            }
         }

         pipe_resource_reference(&pres, NULL);
      } else {
         if (can_unsync) {
            usage |= unsync_usage;
         } else {
            tc_sync(tc);
            tc_set_driver_thread(tc);
         }
         pipe->texture_subdata(pipe, resource, level, usage, box, data,
                              stride, layer_stride);
         if (!can_unsync)
            tc_clear_driver_thread(tc);
      }
   }
}


/********************************************************************
 * miscellaneous
 */

#define TC_FUNC_SYNC_RET0(ret_type, func) \
   static ret_type \
   tc_##func(struct pipe_context *_pipe) \
   { \
      struct threaded_context *tc = threaded_context(_pipe); \
      struct pipe_context *pipe = tc->pipe; \
      tc_sync(tc); \
      return pipe->func(pipe); \
   }

TC_FUNC_SYNC_RET0(uint64_t, get_timestamp)

static void
tc_get_sample_position(struct pipe_context *_pipe,
                       unsigned sample_count, unsigned sample_index,
                       float *out_value)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   pipe->get_sample_position(pipe, sample_count, sample_index,
                             out_value);
}

static enum pipe_reset_status
tc_get_device_reset_status(struct pipe_context *_pipe)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   if (!tc->options.unsynchronized_get_device_reset_status)
      tc_sync(tc);

   return pipe->get_device_reset_status(pipe);
}

static void
tc_set_device_reset_callback(struct pipe_context *_pipe,
                             const struct pipe_device_reset_callback *cb)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc);
   pipe->set_device_reset_callback(pipe, cb);
}

struct tc_string_marker {
   struct tc_call_base base;
   int len;
   char slot[0]; /* more will be allocated if needed */
};

static uint16_t
tc_call_emit_string_marker(struct pipe_context *pipe, void *call)
{
   struct tc_string_marker *p = (struct tc_string_marker *)call;
   pipe->emit_string_marker(pipe, p->slot, p->len);
   return p->base.num_slots;
}

static void
tc_emit_string_marker(struct pipe_context *_pipe,
                      const char *string, int len)
{
   struct threaded_context *tc = threaded_context(_pipe);

   if (len <= TC_MAX_STRING_MARKER_BYTES) {
      struct tc_string_marker *p =
         tc_add_slot_based_call(tc, TC_CALL_emit_string_marker, tc_string_marker, len);

      memcpy(p->slot, string, len);
      p->len = len;
   } else {
      struct pipe_context *pipe = tc->pipe;

      tc_sync(tc);
      tc_set_driver_thread(tc);
      pipe->emit_string_marker(pipe, string, len);
      tc_clear_driver_thread(tc);
   }
}

static void
tc_dump_debug_state(struct pipe_context *_pipe, FILE *stream,
                    unsigned flags)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc);
   pipe->dump_debug_state(pipe, stream, flags);
}

static void
tc_set_debug_callback(struct pipe_context *_pipe,
                      const struct util_debug_callback *cb)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc);

   /* Drop all synchronous debug callbacks. Drivers are expected to be OK
    * with this. shader-db will use an environment variable to disable
    * the threaded context.
    */
   if (cb && !cb->async)
      pipe->set_debug_callback(pipe, NULL);
   else
      pipe->set_debug_callback(pipe, cb);
}

static void
tc_set_log_context(struct pipe_context *_pipe, struct u_log_context *log)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc);
   pipe->set_log_context(pipe, log);
}

static void
tc_create_fence_fd(struct pipe_context *_pipe,
                   struct pipe_fence_handle **fence, int fd,
                   enum pipe_fd_type type)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   if (!tc->options.unsynchronized_create_fence_fd)
      tc_sync(tc);

   pipe->create_fence_fd(pipe, fence, fd, type);
}

struct tc_fence_call {
   struct tc_call_base base;
   struct pipe_fence_handle *fence;
};

static uint16_t
tc_call_fence_server_sync(struct pipe_context *pipe, void *call)
{
   struct pipe_fence_handle *fence = to_call(call, tc_fence_call)->fence;

   pipe->fence_server_sync(pipe, fence);
   pipe->screen->fence_reference(pipe->screen, &fence, NULL);
   return call_size(tc_fence_call);
}

static void
tc_fence_server_sync(struct pipe_context *_pipe,
                     struct pipe_fence_handle *fence)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_screen *screen = tc->pipe->screen;
   struct tc_fence_call *call = tc_add_call(tc, TC_CALL_fence_server_sync,
                                            tc_fence_call);

   call->fence = NULL;
   screen->fence_reference(screen, &call->fence, fence);
}

static void
tc_fence_server_signal(struct pipe_context *_pipe,
                           struct pipe_fence_handle *fence)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;
   tc_sync(tc);
   pipe->fence_server_signal(pipe, fence);
}

static struct pipe_video_codec *
tc_create_video_codec(UNUSED struct pipe_context *_pipe,
                      UNUSED const struct pipe_video_codec *templ)
{
   unreachable("Threaded context should not be enabled for video APIs");
   return NULL;
}

static struct pipe_video_buffer *
tc_create_video_buffer(UNUSED struct pipe_context *_pipe,
                       UNUSED const struct pipe_video_buffer *templ)
{
   unreachable("Threaded context should not be enabled for video APIs");
   return NULL;
}

struct tc_context_param {
   struct tc_call_base base;
   enum pipe_context_param param;
   unsigned value;
};

static uint16_t
tc_call_set_context_param(struct pipe_context *pipe, void *call)
{
   struct tc_context_param *p = to_call(call, tc_context_param);

   if (pipe->set_context_param)
      pipe->set_context_param(pipe, p->param, p->value);

   return call_size(tc_context_param);
}

static void
tc_set_context_param(struct pipe_context *_pipe,
                           enum pipe_context_param param,
                           unsigned value)
{
   struct threaded_context *tc = threaded_context(_pipe);

   if (param == PIPE_CONTEXT_PARAM_PIN_THREADS_TO_L3_CACHE) {
      /* Pin the gallium thread as requested. */
      util_set_thread_affinity(tc->queue.threads[0],
                               util_get_cpu_caps()->L3_affinity_mask[value],
                               NULL, util_get_cpu_caps()->num_cpu_mask_bits);

      /* Execute this immediately (without enqueuing).
       * It's required to be thread-safe.
       */
      struct pipe_context *pipe = tc->pipe;
      if (pipe->set_context_param)
         pipe->set_context_param(pipe, param, value);
      return;
   }

   if (tc->pipe->set_context_param) {
      struct tc_context_param *call =
         tc_add_call(tc, TC_CALL_set_context_param, tc_context_param);

      call->param = param;
      call->value = value;
   }
}


/********************************************************************
 * draw, launch, clear, blit, copy, flush
 */

struct tc_flush_deferred_call {
   struct tc_call_base base;
   unsigned flags;
   struct pipe_fence_handle *fence;
};

struct tc_flush_call {
   struct tc_call_base base;
   unsigned flags;
   struct pipe_fence_handle *fence;
   struct threaded_context *tc;
};

static void
tc_flush_queries(struct threaded_context *tc)
{
   struct threaded_query *tq, *tmp;
   LIST_FOR_EACH_ENTRY_SAFE(tq, tmp, &tc->unflushed_queries, head_unflushed) {
      list_del(&tq->head_unflushed);

      /* Memory release semantics: due to a possible race with
       * tc_get_query_result, we must ensure that the linked list changes
       * are visible before setting tq->flushed.
       */
      p_atomic_set(&tq->flushed, true);
   }
}

static uint16_t
tc_call_flush_deferred(struct pipe_context *pipe, void *call)
{
   struct tc_flush_deferred_call *p = to_call(call, tc_flush_deferred_call);
   struct pipe_screen *screen = pipe->screen;

   pipe->flush(pipe, p->fence ? &p->fence : NULL, p->flags);
   screen->fence_reference(screen, &p->fence, NULL);

   return call_size(tc_flush_deferred_call);
}

static uint16_t
tc_call_flush(struct pipe_context *pipe, void *call)
{
   struct tc_flush_call *p = to_call(call, tc_flush_call);
   struct pipe_screen *screen = pipe->screen;

   pipe->flush(pipe, p->fence ? &p->fence : NULL, p->flags);
   screen->fence_reference(screen, &p->fence, NULL);

   tc_flush_queries(p->tc);

   return call_size(tc_flush_call);
}

static void
tc_flush(struct pipe_context *_pipe, struct pipe_fence_handle **fence,
         unsigned flags)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;
   struct pipe_screen *screen = pipe->screen;
   bool async = flags & (PIPE_FLUSH_DEFERRED | PIPE_FLUSH_ASYNC);
   bool deferred = (flags & PIPE_FLUSH_DEFERRED) > 0;

   if (!deferred || !fence)
      tc->in_renderpass = false;

   if (async && tc->options.create_fence) {
      if (fence) {
         struct tc_batch *next = &tc->batch_slots[tc->next];

         if (!next->token) {
            next->token = malloc(sizeof(*next->token));
            if (!next->token)
               goto out_of_memory;

            pipe_reference_init(&next->token->ref, 1);
            next->token->tc = tc;
         }

         screen->fence_reference(screen, fence,
                                 tc->options.create_fence(pipe, next->token));
         if (!*fence)
            goto out_of_memory;
      }

      struct tc_flush_call *p;
      if (deferred) {
         /* these have identical fields */
         p = (struct tc_flush_call *)tc_add_call(tc, TC_CALL_flush_deferred, tc_flush_deferred_call);
      } else {
         p = tc_add_call(tc, TC_CALL_flush, tc_flush_call);
         p->tc = tc;
      }
      p->fence = fence ? *fence : NULL;
      p->flags = flags | TC_FLUSH_ASYNC;

      if (!deferred) {
         /* non-deferred async flushes indicate completion of existing renderpass info */
         tc_signal_renderpass_info_ready(tc);
         tc_batch_flush(tc, false);
         tc->seen_fb_state = false;
      }

      return;
   }

out_of_memory:
   tc->flushing = true;
   /* renderpass info is signaled during sync */
   tc_sync_msg(tc, flags & PIPE_FLUSH_END_OF_FRAME ? "end of frame" :
                   flags & PIPE_FLUSH_DEFERRED ? "deferred fence" : "normal");

   if (!deferred) {
      tc_flush_queries(tc);
      tc->seen_fb_state = false;
      tc->query_ended = false;
   }
   tc_set_driver_thread(tc);
   pipe->flush(pipe, fence, flags);
   tc_clear_driver_thread(tc);
   tc->flushing = false;
}

struct tc_draw_single_drawid {
   struct tc_draw_single base;
   unsigned drawid_offset;
};

static uint16_t
tc_call_draw_single_drawid(struct pipe_context *pipe, void *call)
{
   struct tc_draw_single_drawid *info_drawid = to_call(call, tc_draw_single_drawid);
   struct tc_draw_single *info = &info_drawid->base;

   /* u_threaded_context stores start/count in min/max_index for single draws. */
   /* Drivers using u_threaded_context shouldn't use min/max_index. */
   struct pipe_draw_start_count_bias draw;

   draw.start = info->info.min_index;
   draw.count = info->info.max_index;
   draw.index_bias = info->index_bias;

   info->info.index_bounds_valid = false;
   info->info.has_user_indices = false;
   info->info.take_index_buffer_ownership = false;

   pipe->draw_vbo(pipe, &info->info, info_drawid->drawid_offset, NULL, &draw, 1);
   if (info->info.index_size)
      tc_drop_resource_reference(info->info.index.resource);

   return call_size(tc_draw_single_drawid);
}

static void
simplify_draw_info(struct pipe_draw_info *info)
{
   /* Clear these fields to facilitate draw merging.
    * Drivers shouldn't use them.
    */
   info->has_user_indices = false;
   info->index_bounds_valid = false;
   info->take_index_buffer_ownership = false;
   info->index_bias_varies = false;
   info->_pad = 0;

   /* This shouldn't be set when merging single draws. */
   info->increment_draw_id = false;

   if (info->index_size) {
      if (!info->primitive_restart)
         info->restart_index = 0;
   } else {
      assert(!info->primitive_restart);
      info->primitive_restart = false;
      info->restart_index = 0;
      info->index.resource = NULL;
   }
}

static bool
is_next_call_a_mergeable_draw(struct tc_draw_single *first,
                              struct tc_draw_single *next)
{
   if (next->base.call_id != TC_CALL_draw_single)
      return false;

   STATIC_ASSERT(offsetof(struct pipe_draw_info, min_index) ==
                 sizeof(struct pipe_draw_info) - 8);
   STATIC_ASSERT(offsetof(struct pipe_draw_info, max_index) ==
                 sizeof(struct pipe_draw_info) - 4);
   /* All fields must be the same except start and count. */
   /* u_threaded_context stores start/count in min/max_index for single draws. */
   return memcmp((uint32_t*)&first->info, (uint32_t*)&next->info,
                 DRAW_INFO_SIZE_WITHOUT_MIN_MAX_INDEX) == 0;
}

static uint16_t
tc_call_draw_single(struct pipe_context *pipe, void *call)
{
   /* Draw call merging. */
   struct tc_draw_single *first = to_call(call, tc_draw_single);
   struct tc_draw_single *next = get_next_call(first, tc_draw_single);

   /* If at least 2 consecutive draw calls can be merged... */
   if (next->base.call_id == TC_CALL_draw_single) {
      if (is_next_call_a_mergeable_draw(first, next)) {
         /* The maximum number of merged draws is given by the batch size. */
         struct pipe_draw_start_count_bias multi[TC_SLOTS_PER_BATCH / call_size(tc_draw_single)];
         unsigned num_draws = 2;
         bool index_bias_varies = first->index_bias != next->index_bias;

         /* u_threaded_context stores start/count in min/max_index for single draws. */
         multi[0].start = first->info.min_index;
         multi[0].count = first->info.max_index;
         multi[0].index_bias = first->index_bias;
         multi[1].start = next->info.min_index;
         multi[1].count = next->info.max_index;
         multi[1].index_bias = next->index_bias;

         /* Find how many other draws can be merged. */
         next = get_next_call(next, tc_draw_single);
         for (; is_next_call_a_mergeable_draw(first, next);
              next = get_next_call(next, tc_draw_single), num_draws++) {
            /* u_threaded_context stores start/count in min/max_index for single draws. */
            multi[num_draws].start = next->info.min_index;
            multi[num_draws].count = next->info.max_index;
            multi[num_draws].index_bias = next->index_bias;
            index_bias_varies |= first->index_bias != next->index_bias;
         }

         first->info.index_bias_varies = index_bias_varies;
         pipe->draw_vbo(pipe, &first->info, 0, NULL, multi, num_draws);

         /* Since all draws use the same index buffer, drop all references at once. */
         if (first->info.index_size)
            pipe_drop_resource_references(first->info.index.resource, num_draws);

         return call_size(tc_draw_single) * num_draws;
      }
   }

   /* u_threaded_context stores start/count in min/max_index for single draws. */
   /* Drivers using u_threaded_context shouldn't use min/max_index. */
   struct pipe_draw_start_count_bias draw;

   draw.start = first->info.min_index;
   draw.count = first->info.max_index;
   draw.index_bias = first->index_bias;

   first->info.index_bounds_valid = false;
   first->info.has_user_indices = false;
   first->info.take_index_buffer_ownership = false;

   pipe->draw_vbo(pipe, &first->info, 0, NULL, &draw, 1);
   if (first->info.index_size)
      tc_drop_resource_reference(first->info.index.resource);

   return call_size(tc_draw_single);
}

struct tc_draw_indirect {
   struct tc_call_base base;
   struct pipe_draw_start_count_bias draw;
   struct pipe_draw_info info;
   struct pipe_draw_indirect_info indirect;
};

static uint16_t
tc_call_draw_indirect(struct pipe_context *pipe, void *call)
{
   struct tc_draw_indirect *info = to_call(call, tc_draw_indirect);

   info->info.index_bounds_valid = false;
   info->info.take_index_buffer_ownership = false;

   pipe->draw_vbo(pipe, &info->info, 0, &info->indirect, &info->draw, 1);
   if (info->info.index_size)
      tc_drop_resource_reference(info->info.index.resource);

   tc_drop_resource_reference(info->indirect.buffer);
   tc_drop_resource_reference(info->indirect.indirect_draw_count);
   tc_drop_so_target_reference(info->indirect.count_from_stream_output);
   return call_size(tc_draw_indirect);
}

struct tc_draw_multi {
   struct tc_call_base base;
   unsigned num_draws;
   struct pipe_draw_info info;
   struct pipe_draw_start_count_bias slot[]; /* variable-sized array */
};

static uint16_t
tc_call_draw_multi(struct pipe_context *pipe, void *call)
{
   struct tc_draw_multi *info = (struct tc_draw_multi*)call;

   info->info.has_user_indices = false;
   info->info.index_bounds_valid = false;
   info->info.take_index_buffer_ownership = false;

   pipe->draw_vbo(pipe, &info->info, 0, NULL, info->slot, info->num_draws);
   if (info->info.index_size)
      tc_drop_resource_reference(info->info.index.resource);

   return info->base.num_slots;
}

#define DRAW_INFO_SIZE_WITHOUT_INDEXBUF_AND_MIN_MAX_INDEX \
   offsetof(struct pipe_draw_info, index)

/* Single draw with drawid_offset == 0. */
static void
tc_draw_single(struct pipe_context *_pipe, const struct pipe_draw_info *info,
               unsigned drawid_offset,
               const struct pipe_draw_indirect_info *indirect,
               const struct pipe_draw_start_count_bias *draws,
               unsigned num_draws)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_draw_single *p =
      tc_add_call(tc, TC_CALL_draw_single, tc_draw_single);

   if (info->index_size) {
      if (!info->take_index_buffer_ownership) {
         tc_set_resource_reference(&p->info.index.resource,
                                   info->index.resource);
      }
      tc_add_to_buffer_list(&tc->buffer_lists[tc->next_buf_list], info->index.resource);
   }
   memcpy(&p->info, info, DRAW_INFO_SIZE_WITHOUT_MIN_MAX_INDEX);
   /* u_threaded_context stores start/count in min/max_index for single draws. */
   p->info.min_index = draws[0].start;
   p->info.max_index = draws[0].count;
   p->index_bias = draws[0].index_bias;
   simplify_draw_info(&p->info);
}

/* Single draw with drawid_offset > 0. */
static void
tc_draw_single_draw_id(struct pipe_context *_pipe,
                       const struct pipe_draw_info *info,
                       unsigned drawid_offset,
                       const struct pipe_draw_indirect_info *indirect,
                       const struct pipe_draw_start_count_bias *draws,
                       unsigned num_draws)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_draw_single *p =
      &tc_add_call(tc, TC_CALL_draw_single_drawid, tc_draw_single_drawid)->base;

   if (info->index_size) {
      if (!info->take_index_buffer_ownership) {
         tc_set_resource_reference(&p->info.index.resource,
                                   info->index.resource);
      }
      tc_add_to_buffer_list(&tc->buffer_lists[tc->next_buf_list], info->index.resource);
   }
   ((struct tc_draw_single_drawid*)p)->drawid_offset = drawid_offset;
   memcpy(&p->info, info, DRAW_INFO_SIZE_WITHOUT_MIN_MAX_INDEX);
   /* u_threaded_context stores start/count in min/max_index for single draws. */
   p->info.min_index = draws[0].start;
   p->info.max_index = draws[0].count;
   p->index_bias = draws[0].index_bias;
   simplify_draw_info(&p->info);
}

/* Single draw with user indices and drawid_offset == 0. */
static void
tc_draw_user_indices_single(struct pipe_context *_pipe,
                            const struct pipe_draw_info *info,
                            unsigned drawid_offset,
                            const struct pipe_draw_indirect_info *indirect,
                            const struct pipe_draw_start_count_bias *draws,
                            unsigned num_draws)
{
   struct threaded_context *tc = threaded_context(_pipe);
   unsigned index_size = info->index_size;
   unsigned size = draws[0].count * index_size;
   struct pipe_resource *buffer = NULL;
   unsigned offset;

   if (!size)
      return;

   /* This must be done before adding draw_vbo, because it could generate
    * e.g. transfer_unmap and flush partially-uninitialized draw_vbo
    * to the driver if it was done afterwards.
    */
   u_upload_data(tc->base.stream_uploader, 0, size, 4,
                 (uint8_t*)info->index.user + draws[0].start * index_size,
                 &offset, &buffer);
   if (unlikely(!buffer))
      return;

   struct tc_draw_single *p =
      tc_add_call(tc, TC_CALL_draw_single, tc_draw_single);
   memcpy(&p->info, info, DRAW_INFO_SIZE_WITHOUT_INDEXBUF_AND_MIN_MAX_INDEX);
   p->info.index.resource = buffer;
   /* u_threaded_context stores start/count in min/max_index for single draws. */
   p->info.min_index = offset >> util_logbase2(index_size);
   p->info.max_index = draws[0].count;
   p->index_bias = draws[0].index_bias;
   simplify_draw_info(&p->info);
}

/* Single draw with user indices and drawid_offset > 0. */
static void
tc_draw_user_indices_single_draw_id(struct pipe_context *_pipe,
                                    const struct pipe_draw_info *info,
                                    unsigned drawid_offset,
                                    const struct pipe_draw_indirect_info *indirect,
                                    const struct pipe_draw_start_count_bias *draws,
                                    unsigned num_draws)
{
   struct threaded_context *tc = threaded_context(_pipe);
   unsigned index_size = info->index_size;
   unsigned size = draws[0].count * index_size;
   struct pipe_resource *buffer = NULL;
   unsigned offset;

   if (!size)
      return;

   /* This must be done before adding draw_vbo, because it could generate
    * e.g. transfer_unmap and flush partially-uninitialized draw_vbo
    * to the driver if it was done afterwards.
    */
   u_upload_data(tc->base.stream_uploader, 0, size, 4,
                 (uint8_t*)info->index.user + draws[0].start * index_size,
                 &offset, &buffer);
   if (unlikely(!buffer))
      return;

   struct tc_draw_single *p =
      &tc_add_call(tc, TC_CALL_draw_single_drawid, tc_draw_single_drawid)->base;
   memcpy(&p->info, info, DRAW_INFO_SIZE_WITHOUT_INDEXBUF_AND_MIN_MAX_INDEX);
   p->info.index.resource = buffer;
   ((struct tc_draw_single_drawid*)p)->drawid_offset = drawid_offset;
   /* u_threaded_context stores start/count in min/max_index for single draws. */
   p->info.min_index = offset >> util_logbase2(index_size);
   p->info.max_index = draws[0].count;
   p->index_bias = draws[0].index_bias;
   simplify_draw_info(&p->info);
}

#define DRAW_OVERHEAD_BYTES sizeof(struct tc_draw_multi)
#define ONE_DRAW_SLOT_BYTES sizeof(((struct tc_draw_multi*)NULL)->slot[0])

#define SLOTS_FOR_ONE_DRAW \
   DIV_ROUND_UP(DRAW_OVERHEAD_BYTES + ONE_DRAW_SLOT_BYTES, \
                sizeof(struct tc_call_base))

static void
tc_draw_multi(struct pipe_context *_pipe, const struct pipe_draw_info *info,
              unsigned drawid_offset,
              const struct pipe_draw_indirect_info *indirect,
              const struct pipe_draw_start_count_bias *draws,
              unsigned num_draws)
{
   struct threaded_context *tc = threaded_context(_pipe);
   int total_offset = 0;
   bool take_index_buffer_ownership = info->take_index_buffer_ownership;

   while (num_draws) {
      struct tc_batch *next = &tc->batch_slots[tc->next];

      int nb_slots_left = TC_SLOTS_PER_BATCH - 1 - next->num_total_slots;
      /* If there isn't enough place for one draw, try to fill the next one */
      if (nb_slots_left < SLOTS_FOR_ONE_DRAW)
         nb_slots_left = TC_SLOTS_PER_BATCH - 1;
      const int size_left_bytes = nb_slots_left * sizeof(struct tc_call_base);

      /* How many draws can we fit in the current batch */
      const int dr = MIN2(num_draws, (size_left_bytes - DRAW_OVERHEAD_BYTES) /
                          ONE_DRAW_SLOT_BYTES);

      /* Non-indexed call or indexed with a real index buffer. */
      struct tc_draw_multi *p =
         tc_add_slot_based_call(tc, TC_CALL_draw_multi, tc_draw_multi,
                                dr);
      if (info->index_size) {
         if (!take_index_buffer_ownership) {
            tc_set_resource_reference(&p->info.index.resource,
                                      info->index.resource);
         }
         tc_add_to_buffer_list(&tc->buffer_lists[tc->next_buf_list], info->index.resource);
      }
      take_index_buffer_ownership = false;
      memcpy(&p->info, info, DRAW_INFO_SIZE_WITHOUT_MIN_MAX_INDEX);
      p->num_draws = dr;
      memcpy(p->slot, &draws[total_offset], sizeof(draws[0]) * dr);
      num_draws -= dr;

      total_offset += dr;
   }
}

static void
tc_draw_user_indices_multi(struct pipe_context *_pipe,
                           const struct pipe_draw_info *info,
                           unsigned drawid_offset,
                           const struct pipe_draw_indirect_info *indirect,
                           const struct pipe_draw_start_count_bias *draws,
                           unsigned num_draws)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_resource *buffer = NULL;
   unsigned buffer_offset, total_count = 0;
   unsigned index_size_shift = util_logbase2(info->index_size);
   uint8_t *ptr = NULL;

   /* Get the total count. */
   for (unsigned i = 0; i < num_draws; i++)
      total_count += draws[i].count;

   if (!total_count)
      return;

   /* Allocate space for all index buffers.
    *
    * This must be done before adding draw_vbo, because it could generate
    * e.g. transfer_unmap and flush partially-uninitialized draw_vbo
    * to the driver if it was done afterwards.
    */
   u_upload_alloc(tc->base.stream_uploader, 0,
                  total_count << index_size_shift, 4,
                  &buffer_offset, &buffer, (void**)&ptr);
   if (unlikely(!buffer))
      return;

   int total_offset = 0;
   unsigned offset = 0;
   while (num_draws) {
      struct tc_batch *next = &tc->batch_slots[tc->next];

      int nb_slots_left = TC_SLOTS_PER_BATCH - 1 - next->num_total_slots;
      /* If there isn't enough place for one draw, try to fill the next one */
      if (nb_slots_left < SLOTS_FOR_ONE_DRAW)
         nb_slots_left = TC_SLOTS_PER_BATCH - 1;
      const int size_left_bytes = nb_slots_left * sizeof(struct tc_call_base);

      /* How many draws can we fit in the current batch */
      const int dr = MIN2(num_draws, (size_left_bytes - DRAW_OVERHEAD_BYTES) /
                          ONE_DRAW_SLOT_BYTES);

      struct tc_draw_multi *p =
         tc_add_slot_based_call(tc, TC_CALL_draw_multi, tc_draw_multi,
                                dr);
      memcpy(&p->info, info, DRAW_INFO_SIZE_WITHOUT_INDEXBUF_AND_MIN_MAX_INDEX);

      if (total_offset == 0)
         /* the first slot inherits the reference from u_upload_alloc() */
         p->info.index.resource = buffer;
      else
         /* all following slots need a new reference */
         tc_set_resource_reference(&p->info.index.resource, buffer);

      p->num_draws = dr;

      /* Upload index buffers. */
      for (unsigned i = 0; i < dr; i++) {
         unsigned count = draws[i + total_offset].count;

         if (!count) {
            p->slot[i].start = 0;
            p->slot[i].count = 0;
            p->slot[i].index_bias = 0;
            continue;
         }

         unsigned size = count << index_size_shift;
         memcpy(ptr + offset,
                (uint8_t*)info->index.user +
                (draws[i + total_offset].start << index_size_shift), size);
         p->slot[i].start = (buffer_offset + offset) >> index_size_shift;
         p->slot[i].count = count;
         p->slot[i].index_bias = draws[i + total_offset].index_bias;
         offset += size;
      }

      total_offset += dr;
      num_draws -= dr;
   }
}

static void
tc_draw_indirect(struct pipe_context *_pipe, const struct pipe_draw_info *info,
                 unsigned drawid_offset,
                 const struct pipe_draw_indirect_info *indirect,
                 const struct pipe_draw_start_count_bias *draws,
                 unsigned num_draws)
{
   struct threaded_context *tc = threaded_context(_pipe);
   assert(!info->has_user_indices);
   assert(num_draws == 1);

   struct tc_draw_indirect *p =
      tc_add_call(tc, TC_CALL_draw_indirect, tc_draw_indirect);
   struct tc_buffer_list *next = &tc->buffer_lists[tc->next_buf_list];

   if (info->index_size) {
      if (!info->take_index_buffer_ownership) {
         tc_set_resource_reference(&p->info.index.resource,
                                   info->index.resource);
      }
      tc_add_to_buffer_list(next, info->index.resource);
   }
   memcpy(&p->info, info, DRAW_INFO_SIZE_WITHOUT_MIN_MAX_INDEX);

   tc_set_resource_reference(&p->indirect.buffer, indirect->buffer);
   tc_set_resource_reference(&p->indirect.indirect_draw_count,
                             indirect->indirect_draw_count);
   p->indirect.count_from_stream_output = NULL;
   pipe_so_target_reference(&p->indirect.count_from_stream_output,
                            indirect->count_from_stream_output);

   if (indirect->buffer)
      tc_add_to_buffer_list(next, indirect->buffer);
   if (indirect->indirect_draw_count)
      tc_add_to_buffer_list(next, indirect->indirect_draw_count);
   if (indirect->count_from_stream_output)
      tc_add_to_buffer_list(next, indirect->count_from_stream_output->buffer);

   memcpy(&p->indirect, indirect, sizeof(*indirect));
   p->draw.start = draws[0].start;
}

/* Dispatch table for tc_draw_vbo:
 *
 * Indexed by:
 *    [is_indirect * 8 + index_size_and_has_user_indices * 4 +
 *     is_multi_draw * 2 + non_zero_draw_id]
 */
static pipe_draw_func draw_funcs[16] = {
   tc_draw_single,
   tc_draw_single_draw_id,
   tc_draw_multi,
   tc_draw_multi,
   tc_draw_user_indices_single,
   tc_draw_user_indices_single_draw_id,
   tc_draw_user_indices_multi,
   tc_draw_user_indices_multi,
   tc_draw_indirect,
   tc_draw_indirect,
   tc_draw_indirect,
   tc_draw_indirect,
   tc_draw_indirect,
   tc_draw_indirect,
   tc_draw_indirect,
   tc_draw_indirect,
};

void
tc_draw_vbo(struct pipe_context *_pipe, const struct pipe_draw_info *info,
            unsigned drawid_offset,
            const struct pipe_draw_indirect_info *indirect,
            const struct pipe_draw_start_count_bias *draws,
            unsigned num_draws)
{
   STATIC_ASSERT(DRAW_INFO_SIZE_WITHOUT_INDEXBUF_AND_MIN_MAX_INDEX +
                 sizeof(intptr_t) == offsetof(struct pipe_draw_info, min_index));

   struct threaded_context *tc = threaded_context(_pipe);
   if (tc->options.parse_renderpass_info)
      tc_parse_draw(tc);

   /* Use a function table to call the desired variant of draw_vbo. */
   unsigned index = (indirect != NULL) * 8 +
                    (info->index_size && info->has_user_indices) * 4 +
                    (num_draws > 1) * 2 + (drawid_offset != 0);
   draw_funcs[index](_pipe, info, drawid_offset, indirect, draws, num_draws);

   /* This must be after tc_add_*call, which can flush the batch. */
   if (unlikely(tc->add_all_gfx_bindings_to_buffer_list))
      tc_add_all_gfx_bindings_to_buffer_list(tc);
}

struct tc_draw_single *
tc_add_draw_single_call(struct pipe_context *_pipe,
                        struct pipe_resource *index_bo)
{
   struct threaded_context *tc = threaded_context(_pipe);

   if (tc->options.parse_renderpass_info)
      tc_parse_draw(tc);

   struct tc_draw_single *p =
      tc_add_call(tc, TC_CALL_draw_single, tc_draw_single);

   if (index_bo)
      tc_add_to_buffer_list(&tc->buffer_lists[tc->next_buf_list], index_bo);

   /* This must be after tc_add_*call, which can flush the batch. */
   if (unlikely(tc->add_all_gfx_bindings_to_buffer_list))
      tc_add_all_gfx_bindings_to_buffer_list(tc);

   return p;
}

struct tc_draw_vstate_single {
   struct tc_call_base base;
   struct pipe_draw_start_count_bias draw;

   /* The following states must be together without holes because they are
    * compared by draw merging.
    */
   struct pipe_vertex_state *state;
   uint32_t partial_velem_mask;
   struct pipe_draw_vertex_state_info info;
};

static bool
is_next_call_a_mergeable_draw_vstate(struct tc_draw_vstate_single *first,
                                     struct tc_draw_vstate_single *next)
{
   if (next->base.call_id != TC_CALL_draw_vstate_single)
      return false;

   return !memcmp(&first->state, &next->state,
                  offsetof(struct tc_draw_vstate_single, info) +
                  sizeof(struct pipe_draw_vertex_state_info) -
                  offsetof(struct tc_draw_vstate_single, state));
}

static uint16_t
tc_call_draw_vstate_single(struct pipe_context *pipe, void *call)
{
   /* Draw call merging. */
   struct tc_draw_vstate_single *first = to_call(call, tc_draw_vstate_single);
   struct tc_draw_vstate_single *next = get_next_call(first, tc_draw_vstate_single);

   /* If at least 2 consecutive draw calls can be merged... */
   if (is_next_call_a_mergeable_draw_vstate(first, next)) {
      /* The maximum number of merged draws is given by the batch size. */
      struct pipe_draw_start_count_bias draws[TC_SLOTS_PER_BATCH /
                                              call_size(tc_draw_vstate_single)];
      unsigned num_draws = 2;

      draws[0] = first->draw;
      draws[1] = next->draw;

      /* Find how many other draws can be merged. */
      next = get_next_call(next, tc_draw_vstate_single);
      for (; is_next_call_a_mergeable_draw_vstate(first, next);
           next = get_next_call(next, tc_draw_vstate_single),
           num_draws++)
         draws[num_draws] = next->draw;

      pipe->draw_vertex_state(pipe, first->state, first->partial_velem_mask,
                              first->info, draws, num_draws);
      /* Since all draws use the same state, drop all references at once. */
      tc_drop_vertex_state_references(first->state, num_draws);

      return call_size(tc_draw_vstate_single) * num_draws;
   }

   pipe->draw_vertex_state(pipe, first->state, first->partial_velem_mask,
                           first->info, &first->draw, 1);
   tc_drop_vertex_state_references(first->state, 1);
   return call_size(tc_draw_vstate_single);
}

struct tc_draw_vstate_multi {
   struct tc_call_base base;
   uint32_t partial_velem_mask;
   struct pipe_draw_vertex_state_info info;
   unsigned num_draws;
   struct pipe_vertex_state *state;
   struct pipe_draw_start_count_bias slot[0];
};

static uint16_t
tc_call_draw_vstate_multi(struct pipe_context *pipe, void *call)
{
   struct tc_draw_vstate_multi *info = (struct tc_draw_vstate_multi*)call;

   pipe->draw_vertex_state(pipe, info->state, info->partial_velem_mask,
                           info->info, info->slot, info->num_draws);
   tc_drop_vertex_state_references(info->state, 1);
   return info->base.num_slots;
}

static void
tc_draw_vertex_state(struct pipe_context *_pipe,
                     struct pipe_vertex_state *state,
                     uint32_t partial_velem_mask,
                     struct pipe_draw_vertex_state_info info,
                     const struct pipe_draw_start_count_bias *draws,
                     unsigned num_draws)
{
   struct threaded_context *tc = threaded_context(_pipe);
   if (tc->options.parse_renderpass_info)
      tc_parse_draw(tc);

   if (num_draws == 1) {
      /* Single draw. */
      struct tc_draw_vstate_single *p =
         tc_add_call(tc, TC_CALL_draw_vstate_single, tc_draw_vstate_single);
      p->partial_velem_mask = partial_velem_mask;
      p->draw = draws[0];
      p->info.mode = info.mode;
      p->info.take_vertex_state_ownership = false;

      /* This should be always 0 for simplicity because we assume that
       * index_bias doesn't vary.
       */
      assert(draws[0].index_bias == 0);

      if (!info.take_vertex_state_ownership)
         tc_set_vertex_state_reference(&p->state, state);
      else
         p->state = state;


      /* This must be after tc_add_*call, which can flush the batch. */
      if (unlikely(tc->add_all_gfx_bindings_to_buffer_list))
         tc_add_all_gfx_bindings_to_buffer_list(tc);
      return;
   }

   const int draw_overhead_bytes = sizeof(struct tc_draw_vstate_multi);
   const int one_draw_slot_bytes = sizeof(((struct tc_draw_vstate_multi*)NULL)->slot[0]);
   const int slots_for_one_draw = DIV_ROUND_UP(draw_overhead_bytes + one_draw_slot_bytes,
                                               sizeof(struct tc_call_base));
   /* Multi draw. */
   int total_offset = 0;
   bool take_vertex_state_ownership = info.take_vertex_state_ownership;
   while (num_draws) {
      struct tc_batch *next = &tc->batch_slots[tc->next];

      int nb_slots_left = TC_SLOTS_PER_BATCH - 1 - next->num_total_slots;
      /* If there isn't enough place for one draw, try to fill the next one */
      if (nb_slots_left < slots_for_one_draw)
         nb_slots_left = TC_SLOTS_PER_BATCH - 1;
      const int size_left_bytes = nb_slots_left * sizeof(struct tc_call_base);

      /* How many draws can we fit in the current batch */
      const int dr = MIN2(num_draws, (size_left_bytes - draw_overhead_bytes) / one_draw_slot_bytes);

      /* Non-indexed call or indexed with a real index buffer. */
      struct tc_draw_vstate_multi *p =
         tc_add_slot_based_call(tc, TC_CALL_draw_vstate_multi, tc_draw_vstate_multi, dr);

      if (!take_vertex_state_ownership)
         tc_set_vertex_state_reference(&p->state, state);
      else
         p->state = state;

      take_vertex_state_ownership = false;
      p->partial_velem_mask = partial_velem_mask;
      p->info.mode = info.mode;
      p->info.take_vertex_state_ownership = false;
      p->num_draws = dr;
      memcpy(p->slot, &draws[total_offset], sizeof(draws[0]) * dr);
      num_draws -= dr;

      total_offset += dr;
   }


   /* This must be after tc_add_*call, which can flush the batch. */
   if (unlikely(tc->add_all_gfx_bindings_to_buffer_list))
      tc_add_all_gfx_bindings_to_buffer_list(tc);
}

struct tc_launch_grid_call {
   struct tc_call_base base;
   struct pipe_grid_info info;
};

static uint16_t
tc_call_launch_grid(struct pipe_context *pipe, void *call)
{
   struct pipe_grid_info *p = &to_call(call, tc_launch_grid_call)->info;

   pipe->launch_grid(pipe, p);
   tc_drop_resource_reference(p->indirect);
   return call_size(tc_launch_grid_call);
}

static void
tc_launch_grid(struct pipe_context *_pipe,
               const struct pipe_grid_info *info)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_launch_grid_call *p = tc_add_call(tc, TC_CALL_launch_grid,
                                               tc_launch_grid_call);
   assert(info->input == NULL);

   tc_set_resource_reference(&p->info.indirect, info->indirect);
   memcpy(&p->info, info, sizeof(*info));

   if (info->indirect)
      tc_add_to_buffer_list(&tc->buffer_lists[tc->next_buf_list], info->indirect);

   /* This must be after tc_add_*call, which can flush the batch. */
   if (unlikely(tc->add_all_compute_bindings_to_buffer_list))
      tc_add_all_compute_bindings_to_buffer_list(tc);
}

static uint16_t
tc_call_resource_copy_region(struct pipe_context *pipe, void *call)
{
   struct tc_resource_copy_region *p = to_call(call, tc_resource_copy_region);

   pipe->resource_copy_region(pipe, p->dst, p->dst_level, p->dstx, p->dsty,
                              p->dstz, p->src, p->src_level, &p->src_box);
   tc_drop_resource_reference(p->dst);
   tc_drop_resource_reference(p->src);
   return call_size(tc_resource_copy_region);
}

static void
tc_resource_copy_region(struct pipe_context *_pipe,
                        struct pipe_resource *dst, unsigned dst_level,
                        unsigned dstx, unsigned dsty, unsigned dstz,
                        struct pipe_resource *src, unsigned src_level,
                        const struct pipe_box *src_box)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct threaded_resource *tdst = threaded_resource(dst);
   struct tc_resource_copy_region *p =
      tc_add_call(tc, TC_CALL_resource_copy_region,
                  tc_resource_copy_region);

   if (dst->target == PIPE_BUFFER)
      tc_buffer_disable_cpu_storage(dst);

   tc_set_resource_batch_usage(tc, dst);
   tc_set_resource_reference(&p->dst, dst);
   p->dst_level = dst_level;
   p->dstx = dstx;
   p->dsty = dsty;
   p->dstz = dstz;
   tc_set_resource_batch_usage(tc, src);
   tc_set_resource_reference(&p->src, src);
   p->src_level = src_level;
   p->src_box = *src_box;

   if (dst->target == PIPE_BUFFER) {
      struct tc_buffer_list *next = &tc->buffer_lists[tc->next_buf_list];

      tc_add_to_buffer_list(next, src);
      tc_add_to_buffer_list(next, dst);

      util_range_add(&tdst->b, &tdst->valid_buffer_range,
                     dstx, dstx + src_box->width);
   }
}

struct tc_blit_call {
   struct tc_call_base base;
   struct pipe_blit_info info;
};

static uint16_t
tc_call_blit(struct pipe_context *pipe, void *call)
{
   struct pipe_blit_info *blit = &to_call(call, tc_blit_call)->info;

   pipe->blit(pipe, blit);
   tc_drop_resource_reference(blit->dst.resource);
   tc_drop_resource_reference(blit->src.resource);
   return call_size(tc_blit_call);
}

static void
tc_blit(struct pipe_context *_pipe, const struct pipe_blit_info *info)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_blit_call *blit = tc_add_call(tc, TC_CALL_blit, tc_blit_call);

   tc_set_resource_batch_usage(tc, info->dst.resource);
   tc_set_resource_reference(&blit->info.dst.resource, info->dst.resource);
   tc_set_resource_batch_usage(tc, info->src.resource);
   tc_set_resource_reference(&blit->info.src.resource, info->src.resource);
   memcpy(&blit->info, info, sizeof(*info));
   if (tc->options.parse_renderpass_info) {
      tc->renderpass_info_recording->has_resolve = info->src.resource->nr_samples > 1 &&
                                                   info->dst.resource->nr_samples <= 1 &&
                                                   tc->fb_resolve == info->dst.resource;
   }
}

struct tc_generate_mipmap {
   struct tc_call_base base;
   enum pipe_format format;
   unsigned base_level;
   unsigned last_level;
   unsigned first_layer;
   unsigned last_layer;
   struct pipe_resource *res;
};

static uint16_t
tc_call_generate_mipmap(struct pipe_context *pipe, void *call)
{
   struct tc_generate_mipmap *p = to_call(call, tc_generate_mipmap);
   ASSERTED bool result = pipe->generate_mipmap(pipe, p->res, p->format,
                                                    p->base_level,
                                                    p->last_level,
                                                    p->first_layer,
                                                    p->last_layer);
   assert(result);
   tc_drop_resource_reference(p->res);
   return call_size(tc_generate_mipmap);
}

static bool
tc_generate_mipmap(struct pipe_context *_pipe,
                   struct pipe_resource *res,
                   enum pipe_format format,
                   unsigned base_level,
                   unsigned last_level,
                   unsigned first_layer,
                   unsigned last_layer)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;
   struct pipe_screen *screen = pipe->screen;
   unsigned bind = PIPE_BIND_SAMPLER_VIEW;

   if (util_format_is_depth_or_stencil(format))
      bind = PIPE_BIND_DEPTH_STENCIL;
   else
      bind = PIPE_BIND_RENDER_TARGET;

   if (!screen->is_format_supported(screen, format, res->target,
                                    res->nr_samples, res->nr_storage_samples,
                                    bind))
      return false;

   struct tc_generate_mipmap *p =
      tc_add_call(tc, TC_CALL_generate_mipmap, tc_generate_mipmap);

   tc_set_resource_batch_usage(tc, res);
   tc_set_resource_reference(&p->res, res);
   p->format = format;
   p->base_level = base_level;
   p->last_level = last_level;
   p->first_layer = first_layer;
   p->last_layer = last_layer;
   return true;
}

struct tc_resource_call {
   struct tc_call_base base;
   struct pipe_resource *resource;
};

static uint16_t
tc_call_flush_resource(struct pipe_context *pipe, void *call)
{
   struct pipe_resource *resource = to_call(call, tc_resource_call)->resource;

   pipe->flush_resource(pipe, resource);
   tc_drop_resource_reference(resource);
   return call_size(tc_resource_call);
}

static void
tc_flush_resource(struct pipe_context *_pipe, struct pipe_resource *resource)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_resource_call *call = tc_add_call(tc, TC_CALL_flush_resource,
                                               tc_resource_call);

   tc_set_resource_batch_usage(tc, resource);
   tc_set_resource_reference(&call->resource, resource);
}

static uint16_t
tc_call_invalidate_resource(struct pipe_context *pipe, void *call)
{
   struct pipe_resource *resource = to_call(call, tc_resource_call)->resource;

   pipe->invalidate_resource(pipe, resource);
   tc_drop_resource_reference(resource);
   return call_size(tc_resource_call);
}

static void
tc_invalidate_resource(struct pipe_context *_pipe,
                       struct pipe_resource *resource)
{
   struct threaded_context *tc = threaded_context(_pipe);

   if (resource->target == PIPE_BUFFER) {
      tc_invalidate_buffer(tc, threaded_resource(resource));
      return;
   }

   struct tc_resource_call *call = tc_add_call(tc, TC_CALL_invalidate_resource,
                                               tc_resource_call);
   tc_set_resource_batch_usage(tc, resource);
   tc_set_resource_reference(&call->resource, resource);

   struct tc_renderpass_info *info = tc_get_renderpass_info(tc);
   if (info) {
      if (tc->fb_resources[PIPE_MAX_COLOR_BUFS] == resource) {
         info->zsbuf_invalidate = true;
      } else {
         for (unsigned i = 0; i < PIPE_MAX_COLOR_BUFS; i++) {
            if (tc->fb_resources[i] == resource)
               info->cbuf_invalidate |= BITFIELD_BIT(i);
         }
      }
   }
}

struct tc_clear {
   struct tc_call_base base;
   bool scissor_state_set;
   uint8_t stencil;
   uint16_t buffers;
   float depth;
   struct pipe_scissor_state scissor_state;
   union pipe_color_union color;
};

static uint16_t
tc_call_clear(struct pipe_context *pipe, void *call)
{
   struct tc_clear *p = to_call(call, tc_clear);

   pipe->clear(pipe, p->buffers, p->scissor_state_set ? &p->scissor_state : NULL, &p->color, p->depth, p->stencil);
   return call_size(tc_clear);
}

static void
tc_clear(struct pipe_context *_pipe, unsigned buffers, const struct pipe_scissor_state *scissor_state,
         const union pipe_color_union *color, double depth,
         unsigned stencil)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_clear *p = tc_add_call(tc, TC_CALL_clear, tc_clear);

   p->buffers = buffers;
   if (scissor_state) {
      p->scissor_state = *scissor_state;
      struct tc_renderpass_info *info = tc_get_renderpass_info(tc);
      /* partial clear info is useful for drivers to know whether any zs writes occur;
       * drivers are responsible for optimizing partial clear -> full clear
       */
      if (info && buffers & PIPE_CLEAR_DEPTHSTENCIL)
         info->zsbuf_clear_partial |= !info->zsbuf_clear;
   } else {
      struct tc_renderpass_info *info = tc_get_renderpass_info(tc);
      if (info) {
         /* full clears use a different load operation, but are only valid if draws haven't occurred yet */
         info->cbuf_clear |= (buffers >> 2) & ~info->cbuf_load;
         if (buffers & PIPE_CLEAR_DEPTHSTENCIL) {
            if (!info->zsbuf_load && !info->zsbuf_clear_partial)
               info->zsbuf_clear = true;
            else if (!info->zsbuf_clear)
               /* this is a clear that occurred after a draw: flag as partial to ensure it isn't ignored */
               info->zsbuf_clear_partial = true;
         }
      }
   }
   p->scissor_state_set = !!scissor_state;
   p->color = *color;
   p->depth = depth;
   p->stencil = stencil;
}

struct tc_clear_render_target {
   struct tc_call_base base;
   bool render_condition_enabled;
   unsigned dstx;
   unsigned dsty;
   unsigned width;
   unsigned height;
   union pipe_color_union color;
   struct pipe_surface *dst;
};

static uint16_t
tc_call_clear_render_target(struct pipe_context *pipe, void *call)
{
   struct tc_clear_render_target *p = to_call(call, tc_clear_render_target);

   pipe->clear_render_target(pipe, p->dst, &p->color, p->dstx, p->dsty, p->width, p->height,
                             p->render_condition_enabled);
   tc_drop_surface_reference(p->dst);
   return call_size(tc_clear_render_target);
}

static void
tc_clear_render_target(struct pipe_context *_pipe,
                       struct pipe_surface *dst,
                       const union pipe_color_union *color,
                       unsigned dstx, unsigned dsty,
                       unsigned width, unsigned height,
                       bool render_condition_enabled)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_clear_render_target *p = tc_add_call(tc, TC_CALL_clear_render_target, tc_clear_render_target);
   p->dst = NULL;
   pipe_surface_reference(&p->dst, dst);
   p->color = *color;
   p->dstx = dstx;
   p->dsty = dsty;
   p->width = width;
   p->height = height;
   p->render_condition_enabled = render_condition_enabled;
}


struct tc_clear_depth_stencil {
   struct tc_call_base base;
   bool render_condition_enabled;
   float depth;
   unsigned clear_flags;
   unsigned stencil;
   unsigned dstx;
   unsigned dsty;
   unsigned width;
   unsigned height;
   struct pipe_surface *dst;
};


static uint16_t
tc_call_clear_depth_stencil(struct pipe_context *pipe, void *call)
{
   struct tc_clear_depth_stencil *p = to_call(call, tc_clear_depth_stencil);

   pipe->clear_depth_stencil(pipe, p->dst, p->clear_flags, p->depth, p->stencil,
                             p->dstx, p->dsty, p->width, p->height,
                             p->render_condition_enabled);
   tc_drop_surface_reference(p->dst);
   return call_size(tc_clear_depth_stencil);
}

static void
tc_clear_depth_stencil(struct pipe_context *_pipe,
                       struct pipe_surface *dst, unsigned clear_flags,
                       double depth, unsigned stencil, unsigned dstx,
                       unsigned dsty, unsigned width, unsigned height,
                       bool render_condition_enabled)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_clear_depth_stencil *p = tc_add_call(tc, TC_CALL_clear_depth_stencil, tc_clear_depth_stencil);
   p->dst = NULL;
   pipe_surface_reference(&p->dst, dst);
   p->clear_flags = clear_flags;
   p->depth = depth;
   p->stencil = stencil;
   p->dstx = dstx;
   p->dsty = dsty;
   p->width = width;
   p->height = height;
   p->render_condition_enabled = render_condition_enabled;
}

struct tc_clear_buffer {
   struct tc_call_base base;
   uint8_t clear_value_size;
   unsigned offset;
   unsigned size;
   char clear_value[16];
   struct pipe_resource *res;
};

static uint16_t
tc_call_clear_buffer(struct pipe_context *pipe, void *call)
{
   struct tc_clear_buffer *p = to_call(call, tc_clear_buffer);

   pipe->clear_buffer(pipe, p->res, p->offset, p->size, p->clear_value,
                      p->clear_value_size);
   tc_drop_resource_reference(p->res);
   return call_size(tc_clear_buffer);
}

static void
tc_clear_buffer(struct pipe_context *_pipe, struct pipe_resource *res,
                unsigned offset, unsigned size,
                const void *clear_value, int clear_value_size)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct threaded_resource *tres = threaded_resource(res);
   struct tc_clear_buffer *p =
      tc_add_call(tc, TC_CALL_clear_buffer, tc_clear_buffer);

   tc_buffer_disable_cpu_storage(res);

   tc_set_resource_reference(&p->res, res);
   tc_add_to_buffer_list(&tc->buffer_lists[tc->next_buf_list], res);
   p->offset = offset;
   p->size = size;
   memcpy(p->clear_value, clear_value, clear_value_size);
   p->clear_value_size = clear_value_size;

   util_range_add(&tres->b, &tres->valid_buffer_range, offset, offset + size);
}

struct tc_clear_texture {
   struct tc_call_base base;
   unsigned level;
   struct pipe_box box;
   char data[16];
   struct pipe_resource *res;
};

static uint16_t
tc_call_clear_texture(struct pipe_context *pipe, void *call)
{
   struct tc_clear_texture *p = to_call(call, tc_clear_texture);

   pipe->clear_texture(pipe, p->res, p->level, &p->box, p->data);
   tc_drop_resource_reference(p->res);
   return call_size(tc_clear_texture);
}

static void
tc_clear_texture(struct pipe_context *_pipe, struct pipe_resource *res,
                 unsigned level, const struct pipe_box *box, const void *data)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_clear_texture *p =
      tc_add_call(tc, TC_CALL_clear_texture, tc_clear_texture);

   tc_set_resource_batch_usage(tc, res);
   tc_set_resource_reference(&p->res, res);
   p->level = level;
   p->box = *box;
   memcpy(p->data, data,
          util_format_get_blocksize(res->format));
}

struct tc_resource_commit {
   struct tc_call_base base;
   bool commit;
   unsigned level;
   struct pipe_box box;
   struct pipe_resource *res;
};

static uint16_t
tc_call_resource_commit(struct pipe_context *pipe, void *call)
{
   struct tc_resource_commit *p = to_call(call, tc_resource_commit);

   pipe->resource_commit(pipe, p->res, p->level, &p->box, p->commit);
   tc_drop_resource_reference(p->res);
   return call_size(tc_resource_commit);
}

static bool
tc_resource_commit(struct pipe_context *_pipe, struct pipe_resource *res,
                   unsigned level, struct pipe_box *box, bool commit)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct tc_resource_commit *p =
      tc_add_call(tc, TC_CALL_resource_commit, tc_resource_commit);

   tc_set_resource_reference(&p->res, res);
   tc_set_resource_batch_usage(tc, res);
   p->level = level;
   p->box = *box;
   p->commit = commit;
   return true; /* we don't care about the return value for this call */
}

static unsigned
tc_init_intel_perf_query_info(struct pipe_context *_pipe)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   return pipe->init_intel_perf_query_info(pipe);
}

static void
tc_get_intel_perf_query_info(struct pipe_context *_pipe,
                             unsigned query_index,
                             const char **name,
                             uint32_t *data_size,
                             uint32_t *n_counters,
                             uint32_t *n_active)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc); /* n_active vs begin/end_intel_perf_query */
   pipe->get_intel_perf_query_info(pipe, query_index, name, data_size,
         n_counters, n_active);
}

static void
tc_get_intel_perf_query_counter_info(struct pipe_context *_pipe,
                                     unsigned query_index,
                                     unsigned counter_index,
                                     const char **name,
                                     const char **desc,
                                     uint32_t *offset,
                                     uint32_t *data_size,
                                     uint32_t *type_enum,
                                     uint32_t *data_type_enum,
                                     uint64_t *raw_max)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   pipe->get_intel_perf_query_counter_info(pipe, query_index, counter_index,
         name, desc, offset, data_size, type_enum, data_type_enum, raw_max);
}

static struct pipe_query *
tc_new_intel_perf_query_obj(struct pipe_context *_pipe, unsigned query_index)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   return pipe->new_intel_perf_query_obj(pipe, query_index);
}

static uint16_t
tc_call_begin_intel_perf_query(struct pipe_context *pipe, void *call)
{
   (void)pipe->begin_intel_perf_query(pipe, to_call(call, tc_query_call)->query);
   return call_size(tc_query_call);
}

static bool
tc_begin_intel_perf_query(struct pipe_context *_pipe, struct pipe_query *q)
{
   struct threaded_context *tc = threaded_context(_pipe);

   tc_add_call(tc, TC_CALL_begin_intel_perf_query, tc_query_call)->query = q;

   /* assume success, begin failure can be signaled from get_intel_perf_query_data */
   return true;
}

static uint16_t
tc_call_end_intel_perf_query(struct pipe_context *pipe, void *call)
{
   pipe->end_intel_perf_query(pipe, to_call(call, tc_query_call)->query);
   return call_size(tc_query_call);
}

static void
tc_end_intel_perf_query(struct pipe_context *_pipe, struct pipe_query *q)
{
   struct threaded_context *tc = threaded_context(_pipe);

   tc_add_call(tc, TC_CALL_end_intel_perf_query, tc_query_call)->query = q;
}

static void
tc_delete_intel_perf_query(struct pipe_context *_pipe, struct pipe_query *q)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc); /* flush potentially pending begin/end_intel_perf_queries */
   pipe->delete_intel_perf_query(pipe, q);
}

static void
tc_wait_intel_perf_query(struct pipe_context *_pipe, struct pipe_query *q)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc); /* flush potentially pending begin/end_intel_perf_queries */
   pipe->wait_intel_perf_query(pipe, q);
}

static bool
tc_is_intel_perf_query_ready(struct pipe_context *_pipe, struct pipe_query *q)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc); /* flush potentially pending begin/end_intel_perf_queries */
   return pipe->is_intel_perf_query_ready(pipe, q);
}

static bool
tc_get_intel_perf_query_data(struct pipe_context *_pipe,
                             struct pipe_query *q,
                             size_t data_size,
                             uint32_t *data,
                             uint32_t *bytes_written)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   tc_sync(tc); /* flush potentially pending begin/end_intel_perf_queries */
   return pipe->get_intel_perf_query_data(pipe, q, data_size, data, bytes_written);
}

/********************************************************************
 * callback
 */

struct tc_callback_call {
   struct tc_call_base base;
   void (*fn)(void *data);
   void *data;
};

static uint16_t
tc_call_callback(UNUSED struct pipe_context *pipe, void *call)
{
   struct tc_callback_call *p = to_call(call, tc_callback_call);

   p->fn(p->data);
   return call_size(tc_callback_call);
}

static void
tc_callback(struct pipe_context *_pipe, void (*fn)(void *), void *data,
            bool asap)
{
   struct threaded_context *tc = threaded_context(_pipe);

   if (asap && tc_is_sync(tc)) {
      fn(data);
      return;
   }

   struct tc_callback_call *p =
      tc_add_call(tc, TC_CALL_callback, tc_callback_call);
   p->fn = fn;
   p->data = data;
}


/********************************************************************
 * create & destroy
 */

static void
tc_destroy(struct pipe_context *_pipe)
{
   struct threaded_context *tc = threaded_context(_pipe);
   struct pipe_context *pipe = tc->pipe;

   if (tc->base.const_uploader &&
       tc->base.stream_uploader != tc->base.const_uploader)
      u_upload_destroy(tc->base.const_uploader);

   if (tc->base.stream_uploader)
      u_upload_destroy(tc->base.stream_uploader);

   tc_sync(tc);

   if (util_queue_is_initialized(&tc->queue)) {
      util_queue_destroy(&tc->queue);

      for (unsigned i = 0; i < TC_MAX_BATCHES; i++) {
         util_queue_fence_destroy(&tc->batch_slots[i].fence);
         util_dynarray_fini(&tc->batch_slots[i].renderpass_infos);
         assert(!tc->batch_slots[i].token);
      }
   }

   slab_destroy_child(&tc->pool_transfers);
   assert(tc->batch_slots[tc->next].num_total_slots == 0);
   pipe->destroy(pipe);

   for (unsigned i = 0; i < TC_MAX_BUFFER_LISTS; i++) {
      if (!util_queue_fence_is_signalled(&tc->buffer_lists[i].driver_flushed_fence))
         util_queue_fence_signal(&tc->buffer_lists[i].driver_flushed_fence);
      util_queue_fence_destroy(&tc->buffer_lists[i].driver_flushed_fence);
   }

   for (unsigned i = 0; i < ARRAY_SIZE(tc->fb_resources); i++)
      pipe_resource_reference(&tc->fb_resources[i], NULL);
   pipe_resource_reference(&tc->fb_resolve, NULL);

   FREE(tc);
}

static const tc_execute execute_func[TC_NUM_CALLS] = {
#define CALL(name) tc_call_##name,
#include "u_threaded_context_calls.h"
#undef CALL
};

void tc_driver_internal_flush_notify(struct threaded_context *tc)
{
   /* Allow drivers to call this function even for internal contexts that
    * don't have tc. It simplifies drivers.
    */
   if (!tc)
      return;

   /* Signal fences set by tc_batch_execute. */
   for (unsigned i = 0; i < tc->num_signal_fences_next_flush; i++)
      util_queue_fence_signal(tc->signal_fences_next_flush[i]);

   tc->num_signal_fences_next_flush = 0;
}

/**
 * Wrap an existing pipe_context into a threaded_context.
 *
 * \param pipe                 pipe_context to wrap
 * \param parent_transfer_pool parent slab pool set up for creating pipe_-
 *                             transfer objects; the driver should have one
 *                             in pipe_screen.
 * \param replace_buffer  callback for replacing a pipe_resource's storage
 *                        with another pipe_resource's storage.
 * \param options         optional TC options/callbacks
 * \param out  if successful, the threaded_context will be returned here in
 *             addition to the return value if "out" != NULL
 */
struct pipe_context *
threaded_context_create(struct pipe_context *pipe,
                        struct slab_parent_pool *parent_transfer_pool,
                        tc_replace_buffer_storage_func replace_buffer,
                        const struct threaded_context_options *options,
                        struct threaded_context **out)
{
   struct threaded_context *tc;

   if (!pipe)
      return NULL;

   if (!debug_get_bool_option("GALLIUM_THREAD", true))
      return pipe;

   tc = CALLOC_STRUCT(threaded_context);
   if (!tc) {
      pipe->destroy(pipe);
      return NULL;
   }

   if (options) {
      /* this is unimplementable */
      assert(!(options->parse_renderpass_info && options->driver_calls_flush_notify));
      tc->options = *options;
   }

   pipe = trace_context_create_threaded(pipe->screen, pipe, &replace_buffer, &tc->options);

   /* The driver context isn't wrapped, so set its "priv" to NULL. */
   pipe->priv = NULL;

   tc->pipe = pipe;
   tc->replace_buffer_storage = replace_buffer;
   tc->map_buffer_alignment =
      pipe->screen->get_param(pipe->screen, PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT);
   tc->ubo_alignment =
      MAX2(pipe->screen->get_param(pipe->screen, PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT), 64);
   tc->base.priv = pipe; /* priv points to the wrapped driver context */
   tc->base.screen = pipe->screen;
   tc->base.destroy = tc_destroy;
   tc->base.callback = tc_callback;

   tc->base.stream_uploader = u_upload_clone(&tc->base, pipe->stream_uploader);
   if (pipe->stream_uploader == pipe->const_uploader)
      tc->base.const_uploader = tc->base.stream_uploader;
   else
      tc->base.const_uploader = u_upload_clone(&tc->base, pipe->const_uploader);

   if (!tc->base.stream_uploader || !tc->base.const_uploader)
      goto fail;

   tc->use_forced_staging_uploads = true;

   /* The queue size is the number of batches "waiting". Batches are removed
    * from the queue before being executed, so keep one tc_batch slot for that
    * execution. Also, keep one unused slot for an unflushed batch.
    */
   if (!util_queue_init(&tc->queue, "gdrv", TC_MAX_BATCHES - 2, 1, 0, NULL))
      goto fail;

   tc->last_completed = -1;
   for (unsigned i = 0; i < TC_MAX_BATCHES; i++) {
#if !defined(NDEBUG) && TC_DEBUG >= 1
      tc->batch_slots[i].sentinel = TC_SENTINEL;
#endif
      tc->batch_slots[i].tc = tc;
      tc->batch_slots[i].batch_idx = i;
      util_queue_fence_init(&tc->batch_slots[i].fence);
      tc->batch_slots[i].renderpass_info_idx = -1;
      if (tc->options.parse_renderpass_info) {
         util_dynarray_init(&tc->batch_slots[i].renderpass_infos, NULL);
         tc_batch_renderpass_infos_resize(tc, &tc->batch_slots[i]);
      }
   }
   for (unsigned i = 0; i < TC_MAX_BUFFER_LISTS; i++)
      util_queue_fence_init(&tc->buffer_lists[i].driver_flushed_fence);

   list_inithead(&tc->unflushed_queries);

   slab_create_child(&tc->pool_transfers, parent_transfer_pool);

   /* If you have different limits in each shader stage, set the maximum. */
   struct pipe_screen *screen = pipe->screen;;
   tc->max_vertex_buffers =
      screen->get_param(screen, PIPE_CAP_MAX_VERTEX_BUFFERS);
   tc->max_const_buffers =
      screen->get_shader_param(screen, PIPE_SHADER_FRAGMENT,
                               PIPE_SHADER_CAP_MAX_CONST_BUFFERS);
   tc->max_shader_buffers =
      screen->get_shader_param(screen, PIPE_SHADER_FRAGMENT,
                               PIPE_SHADER_CAP_MAX_SHADER_BUFFERS);
   tc->max_images =
      screen->get_shader_param(screen, PIPE_SHADER_FRAGMENT,
                               PIPE_SHADER_CAP_MAX_SHADER_IMAGES);
   tc->max_samplers =
      screen->get_shader_param(screen, PIPE_SHADER_FRAGMENT,
                               PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS);

   tc->base.set_context_param = tc_set_context_param; /* always set this */

#define CTX_INIT(_member) \
   tc->base._member = tc->pipe->_member ? tc_##_member : NULL

   CTX_INIT(flush);
   CTX_INIT(draw_vbo);
   CTX_INIT(draw_vertex_state);
   CTX_INIT(launch_grid);
   CTX_INIT(resource_copy_region);
   CTX_INIT(blit);
   CTX_INIT(clear);
   CTX_INIT(clear_render_target);
   CTX_INIT(clear_depth_stencil);
   CTX_INIT(clear_buffer);
   CTX_INIT(clear_texture);
   CTX_INIT(flush_resource);
   CTX_INIT(generate_mipmap);
   CTX_INIT(render_condition);
   CTX_INIT(create_query);
   CTX_INIT(create_batch_query);
   CTX_INIT(destroy_query);
   CTX_INIT(begin_query);
   CTX_INIT(end_query);
   CTX_INIT(get_query_result);
   CTX_INIT(get_query_result_resource);
   CTX_INIT(set_active_query_state);
   CTX_INIT(create_blend_state);
   CTX_INIT(bind_blend_state);
   CTX_INIT(delete_blend_state);
   CTX_INIT(create_sampler_state);
   CTX_INIT(bind_sampler_states);
   CTX_INIT(delete_sampler_state);
   CTX_INIT(create_rasterizer_state);
   CTX_INIT(bind_rasterizer_state);
   CTX_INIT(delete_rasterizer_state);
   CTX_INIT(create_depth_stencil_alpha_state);
   CTX_INIT(bind_depth_stencil_alpha_state);
   CTX_INIT(delete_depth_stencil_alpha_state);
   CTX_INIT(link_shader);
   CTX_INIT(create_fs_state);
   CTX_INIT(bind_fs_state);
   CTX_INIT(delete_fs_state);
   CTX_INIT(create_vs_state);
   CTX_INIT(bind_vs_state);
   CTX_INIT(delete_vs_state);
   CTX_INIT(create_gs_state);
   CTX_INIT(bind_gs_state);
   CTX_INIT(delete_gs_state);
   CTX_INIT(create_tcs_state);
   CTX_INIT(bind_tcs_state);
   CTX_INIT(delete_tcs_state);
   CTX_INIT(create_tes_state);
   CTX_INIT(bind_tes_state);
   CTX_INIT(delete_tes_state);
   CTX_INIT(create_compute_state);
   CTX_INIT(bind_compute_state);
   CTX_INIT(delete_compute_state);
   CTX_INIT(create_vertex_elements_state);
   CTX_INIT(bind_vertex_elements_state);
   CTX_INIT(delete_vertex_elements_state);
   CTX_INIT(set_blend_color);
   CTX_INIT(set_stencil_ref);
   CTX_INIT(set_sample_mask);
   CTX_INIT(set_min_samples);
   CTX_INIT(set_clip_state);
   CTX_INIT(set_constant_buffer);
   CTX_INIT(set_inlinable_constants);
   CTX_INIT(set_framebuffer_state);
   CTX_INIT(set_polygon_stipple);
   CTX_INIT(set_sample_locations);
   CTX_INIT(set_scissor_states);
   CTX_INIT(set_viewport_states);
   CTX_INIT(set_window_rectangles);
   CTX_INIT(set_sampler_views);
   CTX_INIT(set_tess_state);
   CTX_INIT(set_patch_vertices);
   CTX_INIT(set_shader_buffers);
   CTX_INIT(set_shader_images);
   CTX_INIT(set_vertex_buffers);
   CTX_INIT(create_stream_output_target);
   CTX_INIT(stream_output_target_destroy);
   CTX_INIT(set_stream_output_targets);
   CTX_INIT(create_sampler_view);
   CTX_INIT(sampler_view_destroy);
   CTX_INIT(create_surface);
   CTX_INIT(surface_destroy);
   CTX_INIT(buffer_map);
   CTX_INIT(texture_map);
   CTX_INIT(transfer_flush_region);
   CTX_INIT(buffer_unmap);
   CTX_INIT(texture_unmap);
   CTX_INIT(buffer_subdata);
   CTX_INIT(texture_subdata);
   CTX_INIT(texture_barrier);
   CTX_INIT(memory_barrier);
   CTX_INIT(resource_commit);
   CTX_INIT(create_video_codec);
   CTX_INIT(create_video_buffer);
   CTX_INIT(set_compute_resources);
   CTX_INIT(set_global_binding);
   CTX_INIT(get_sample_position);
   CTX_INIT(invalidate_resource);
   CTX_INIT(get_device_reset_status);
   CTX_INIT(set_device_reset_callback);
   CTX_INIT(dump_debug_state);
   CTX_INIT(set_log_context);
   CTX_INIT(emit_string_marker);
   CTX_INIT(set_debug_callback);
   CTX_INIT(create_fence_fd);
   CTX_INIT(fence_server_sync);
   CTX_INIT(fence_server_signal);
   CTX_INIT(get_timestamp);
   CTX_INIT(create_texture_handle);
   CTX_INIT(delete_texture_handle);
   CTX_INIT(make_texture_handle_resident);
   CTX_INIT(create_image_handle);
   CTX_INIT(delete_image_handle);
   CTX_INIT(make_image_handle_resident);
   CTX_INIT(set_frontend_noop);
   CTX_INIT(init_intel_perf_query_info);
   CTX_INIT(get_intel_perf_query_info);
   CTX_INIT(get_intel_perf_query_counter_info);
   CTX_INIT(new_intel_perf_query_obj);
   CTX_INIT(begin_intel_perf_query);
   CTX_INIT(end_intel_perf_query);
   CTX_INIT(delete_intel_perf_query);
   CTX_INIT(wait_intel_perf_query);
   CTX_INIT(is_intel_perf_query_ready);
   CTX_INIT(get_intel_perf_query_data);
#undef CTX_INIT

   if (out)
      *out = tc;

   tc_begin_next_buffer_list(tc);
   if (tc->options.parse_renderpass_info)
      tc_batch_increment_renderpass_info(tc, tc->next, false);
   return &tc->base;

fail:
   tc_destroy(&tc->base);
   return NULL;
}

void
threaded_context_init_bytes_mapped_limit(struct threaded_context *tc, unsigned divisor)
{
   uint64_t total_ram;
   if (os_get_total_physical_memory(&total_ram)) {
      tc->bytes_mapped_limit = total_ram / divisor;
      if (sizeof(void*) == 4)
         tc->bytes_mapped_limit = MIN2(tc->bytes_mapped_limit, 512*1024*1024UL);
   }
}

const struct tc_renderpass_info *
threaded_context_get_renderpass_info(struct threaded_context *tc)
{
   assert(tc->renderpass_info && tc->options.parse_renderpass_info);
   struct tc_batch_rp_info *info = tc_batch_rp_info(tc->renderpass_info);
   while (1) {
      util_queue_fence_wait(&info->ready);
      if (!info->next)
         return &info->info;
      info = info->next;
   }
}
