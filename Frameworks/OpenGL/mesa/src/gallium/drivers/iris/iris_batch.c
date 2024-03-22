/*
 * Copyright © 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file iris_batch.c
 *
 * Batchbuffer and command submission module.
 *
 * Every API draw call results in a number of GPU commands, which we
 * collect into a "batch buffer".  Typically, many draw calls are grouped
 * into a single batch to amortize command submission overhead.
 *
 * We submit batches to the kernel using the I915_GEM_EXECBUFFER2 ioctl.
 * One critical piece of data is the "validation list", which contains a
 * list of the buffer objects (BOs) which the commands in the GPU need.
 * The kernel will make sure these are resident and pinned at the correct
 * virtual memory address before executing our batch.  If a BO is not in
 * the validation list, it effectively does not exist, so take care.
 */

#include "iris_batch.h"
#include "iris_bufmgr.h"
#include "iris_context.h"
#include "iris_fence.h"
#include "iris_kmd_backend.h"
#include "iris_utrace.h"
#include "i915/iris_batch.h"
#include "xe/iris_batch.h"

#include "common/intel_aux_map.h"
#include "common/intel_defines.h"
#include "intel/common/intel_gem.h"
#include "intel/ds/intel_tracepoints.h"
#include "util/hash_table.h"
#include "util/u_debug.h"
#include "util/set.h"
#include "util/u_upload_mgr.h"

#include <errno.h>
#include <xf86drm.h>

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#define VG(x) x
#else
#define VG(x)
#endif

#define FILE_DEBUG_FLAG DEBUG_BUFMGR

static void
iris_batch_reset(struct iris_batch *batch);

unsigned
iris_batch_num_fences(struct iris_batch *batch)
{
   return util_dynarray_num_elements(&batch->exec_fences,
                                     struct iris_batch_fence);
}

/**
 * Debugging code to dump the fence list, used by INTEL_DEBUG=submit.
 */
void
iris_dump_fence_list(struct iris_batch *batch)
{
   fprintf(stderr, "Fence list (length %u):      ", iris_batch_num_fences(batch));

   util_dynarray_foreach(&batch->exec_fences, struct iris_batch_fence, f) {
      fprintf(stderr, "%s%u%s ",
              (f->flags & IRIS_BATCH_FENCE_WAIT) ? "..." : "",
              f->handle,
              (f->flags & IRIS_BATCH_FENCE_SIGNAL) ? "!" : "");
   }

   fprintf(stderr, "\n");
}

/**
 * Debugging code to dump the validation list, used by INTEL_DEBUG=submit.
 */
void
iris_dump_bo_list(struct iris_batch *batch)
{
   fprintf(stderr, "BO list (length %d):\n", batch->exec_count);

   for (int i = 0; i < batch->exec_count; i++) {
      struct iris_bo *bo = batch->exec_bos[i];
      struct iris_bo *backing = iris_get_backing_bo(bo);
      bool written = BITSET_TEST(batch->bos_written, i);
      bool exported = iris_bo_is_exported(bo);
      bool imported = iris_bo_is_imported(bo);

      fprintf(stderr, "[%2d]: %3d (%3d) %-14s @ 0x%016"PRIx64" (%-15s %8"PRIu64"B) %2d refs %s%s%s\n",
              i,
              bo->gem_handle,
              backing->gem_handle,
              bo->name,
              bo->address,
              iris_heap_to_string[backing->real.heap],
              bo->size,
              bo->refcount,
              written ? " write" : "",
              exported ? " exported" : "",
              imported ? " imported" : "");
   }
}

/**
 * Return BO information to the batch decoder (for debugging).
 */
static struct intel_batch_decode_bo
decode_get_bo(void *v_batch, bool ppgtt, uint64_t address)
{
   struct iris_batch *batch = v_batch;

   assert(ppgtt);

   for (int i = 0; i < batch->exec_count; i++) {
      struct iris_bo *bo = batch->exec_bos[i];
      /* The decoder zeroes out the top 16 bits, so we need to as well */
      uint64_t bo_address = bo->address & (~0ull >> 16);

      if (address >= bo_address && address < bo_address + bo->size) {
         if (bo->real.mmap_mode == IRIS_MMAP_NONE)
            return (struct intel_batch_decode_bo) { };

         return (struct intel_batch_decode_bo) {
            .addr = bo_address,
            .size = bo->size,
            .map = iris_bo_map(batch->dbg, bo, MAP_READ | MAP_ASYNC),
         };
      }
   }

   return (struct intel_batch_decode_bo) { };
}

static unsigned
decode_get_state_size(void *v_batch,
                      uint64_t address,
                      UNUSED uint64_t base_address)
{
   struct iris_batch *batch = v_batch;
   unsigned size = (uintptr_t)
      _mesa_hash_table_u64_search(batch->state_sizes, address);

   return size;
}

/**
 * Decode the current batch.
 */
void
iris_batch_decode_batch(struct iris_batch *batch)
{
   void *map = iris_bo_map(batch->dbg, batch->exec_bos[0], MAP_READ);
   intel_print_batch(&batch->decoder, map, batch->primary_batch_size,
                     batch->exec_bos[0]->address, false);
}

static void
iris_init_batch(struct iris_context *ice,
                enum iris_batch_name name)
{
   struct iris_batch *batch = &ice->batches[name];
   struct iris_screen *screen = (void *) ice->ctx.screen;

   /* Note: screen, ctx_id, exec_flags and has_engines_context fields are
    * initialized at an earlier phase when contexts are created.
    *
    * See iris_init_batches(), which calls either iris_init_engines_context()
    * or iris_init_non_engine_contexts().
    */

   batch->dbg = &ice->dbg;
   batch->reset = &ice->reset;
   batch->state_sizes = ice->state.sizes;
   batch->name = name;
   batch->ice = ice;
   batch->screen = screen;
   batch->contains_fence_signal = false;

   batch->fine_fences.uploader =
      u_upload_create(&ice->ctx, 4096, PIPE_BIND_CUSTOM,
                      PIPE_USAGE_STAGING, 0);
   iris_fine_fence_init(batch);

   util_dynarray_init(&batch->exec_fences, ralloc_context(NULL));
   util_dynarray_init(&batch->syncobjs, ralloc_context(NULL));

   batch->exec_count = 0;
   batch->max_gem_handle = 0;
   batch->exec_array_size = 128;
   batch->exec_bos =
      malloc(batch->exec_array_size * sizeof(batch->exec_bos[0]));
   batch->bos_written =
      rzalloc_array(NULL, BITSET_WORD, BITSET_WORDS(batch->exec_array_size));

   batch->bo_aux_modes = _mesa_hash_table_create(NULL, _mesa_hash_pointer,
                                                 _mesa_key_pointer_equal);

   batch->num_other_batches = 0;
   memset(batch->other_batches, 0, sizeof(batch->other_batches));

   iris_foreach_batch(ice, other_batch) {
      if (batch != other_batch)
         batch->other_batches[batch->num_other_batches++] = other_batch;
   }

   if (INTEL_DEBUG(DEBUG_BATCH | DEBUG_BATCH_STATS)) {
      const unsigned decode_flags = INTEL_BATCH_DECODE_DEFAULT_FLAGS |
         (INTEL_DEBUG(DEBUG_COLOR) ? INTEL_BATCH_DECODE_IN_COLOR : 0);

      intel_batch_decode_ctx_init(&batch->decoder, &screen->compiler->isa,
                                  screen->devinfo,
                                  stderr, decode_flags, NULL,
                                  decode_get_bo, decode_get_state_size, batch);
      batch->decoder.dynamic_base = IRIS_MEMZONE_DYNAMIC_START;
      batch->decoder.instruction_base = IRIS_MEMZONE_SHADER_START;
      batch->decoder.surface_base = IRIS_MEMZONE_BINDER_START;
      batch->decoder.max_vbo_decoded_lines = 32;
      if (batch->name == IRIS_BATCH_BLITTER)
         batch->decoder.engine = INTEL_ENGINE_CLASS_COPY;
   }

   iris_init_batch_measure(ice, batch);

   u_trace_init(&batch->trace, &ice->ds.trace_context);

   iris_batch_reset(batch);
}

void
iris_init_batches(struct iris_context *ice)
{
   struct iris_screen *screen = (struct iris_screen *)ice->ctx.screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   const struct intel_device_info *devinfo = iris_bufmgr_get_device_info(bufmgr);

   switch (devinfo->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      iris_i915_init_batches(ice);
      break;
   case INTEL_KMD_TYPE_XE:
      iris_xe_init_batches(ice);
      break;
   default:
      unreachable("missing");
   }

   iris_foreach_batch(ice, batch)
      iris_init_batch(ice, batch - &ice->batches[0]);
}

static int
find_exec_index(struct iris_batch *batch, struct iris_bo *bo)
{
   unsigned index = READ_ONCE(bo->index);

   if (index == -1)
      return -1;

   if (index < batch->exec_count && batch->exec_bos[index] == bo)
      return index;

   /* May have been shared between multiple active batches */
   for (index = 0; index < batch->exec_count; index++) {
      if (batch->exec_bos[index] == bo)
         return index;
   }

   return -1;
}

static void
ensure_exec_obj_space(struct iris_batch *batch, uint32_t count)
{
   while (batch->exec_count + count > batch->exec_array_size) {
      unsigned old_size = batch->exec_array_size;

      batch->exec_array_size *= 2;
      batch->exec_bos =
         realloc(batch->exec_bos,
                 batch->exec_array_size * sizeof(batch->exec_bos[0]));
      batch->bos_written =
         rerzalloc(NULL, batch->bos_written, BITSET_WORD,
                   BITSET_WORDS(old_size),
                   BITSET_WORDS(batch->exec_array_size));
   }
}

static void
add_bo_to_batch(struct iris_batch *batch, struct iris_bo *bo, bool writable)
{
   assert(batch->exec_array_size > batch->exec_count);

   iris_bo_reference(bo);

   batch->exec_bos[batch->exec_count] = bo;

   if (writable)
      BITSET_SET(batch->bos_written, batch->exec_count);

   bo->index = batch->exec_count;
   batch->exec_count++;
   batch->aperture_space += bo->size;

   batch->max_gem_handle =
      MAX2(batch->max_gem_handle, iris_get_backing_bo(bo)->gem_handle);
}

static void
flush_for_cross_batch_dependencies(struct iris_batch *batch,
                                   struct iris_bo *bo,
                                   bool writable)
{
   if (batch->measure && bo == batch->measure->bo)
      return;

   /* When a batch uses a buffer for the first time, or newly writes a buffer
    * it had already referenced, we may need to flush other batches in order
    * to correctly synchronize them.
    */
   for (int b = 0; b < batch->num_other_batches; b++) {
      struct iris_batch *other_batch = batch->other_batches[b];
      int other_index = find_exec_index(other_batch, bo);

      /* If the buffer is referenced by another batch, and either batch
       * intends to write it, then flush the other batch and synchronize.
       *
       * Consider these cases:
       *
       * 1. They read, we read   =>  No synchronization required.
       * 2. They read, we write  =>  Synchronize (they need the old value)
       * 3. They write, we read  =>  Synchronize (we need their new value)
       * 4. They write, we write =>  Synchronize (order writes)
       *
       * The read/read case is very common, as multiple batches usually
       * share a streaming state buffer or shader assembly buffer, and
       * we want to avoid synchronizing in this case.
       */
      if (other_index != -1 &&
          (writable || BITSET_TEST(other_batch->bos_written, other_index)))
         iris_batch_flush(other_batch);
   }
}

/**
 * Add a buffer to the current batch's validation list.
 *
 * You must call this on any BO you wish to use in this batch, to ensure
 * that it's resident when the GPU commands execute.
 */
void
iris_use_pinned_bo(struct iris_batch *batch,
                   struct iris_bo *bo,
                   bool writable, enum iris_domain access)
{
   assert(iris_get_backing_bo(bo)->real.kflags & EXEC_OBJECT_PINNED);
   assert(bo != batch->bo);

   /* Never mark the workaround BO with EXEC_OBJECT_WRITE.  We don't care
    * about the order of any writes to that buffer, and marking it writable
    * would introduce data dependencies between multiple batches which share
    * the buffer. It is added directly to the batch using add_bo_to_batch()
    * during batch reset time.
    */
   if (bo == batch->screen->workaround_bo)
      return;

   if (access < NUM_IRIS_DOMAINS) {
      assert(batch->sync_region_depth);
      iris_bo_bump_seqno(bo, batch->next_seqno, access);
   }

   int existing_index = find_exec_index(batch, bo);

   if (existing_index == -1) {
      flush_for_cross_batch_dependencies(batch, bo, writable);

      ensure_exec_obj_space(batch, 1);
      add_bo_to_batch(batch, bo, writable);
   } else if (writable && !BITSET_TEST(batch->bos_written, existing_index)) {
      flush_for_cross_batch_dependencies(batch, bo, writable);

      /* The BO is already in the list; mark it writable */
      BITSET_SET(batch->bos_written, existing_index);
   }
}

static void
create_batch(struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;

   /* TODO: We probably could suballocate batches... */
   batch->bo = iris_bo_alloc(bufmgr, "command buffer",
                             BATCH_SZ + BATCH_RESERVED, 8,
                             IRIS_MEMZONE_OTHER, BO_ALLOC_NO_SUBALLOC);
   iris_get_backing_bo(batch->bo)->real.kflags |= EXEC_OBJECT_CAPTURE;
   batch->map = iris_bo_map(NULL, batch->bo, MAP_READ | MAP_WRITE);
   batch->map_next = batch->map;

   ensure_exec_obj_space(batch, 1);
   add_bo_to_batch(batch, batch->bo, false);
}

static void
iris_batch_maybe_noop(struct iris_batch *batch)
{
   /* We only insert the NOOP at the beginning of the batch. */
   assert(iris_batch_bytes_used(batch) == 0);

   if (batch->noop_enabled) {
      /* Emit MI_BATCH_BUFFER_END to prevent any further command to be
       * executed.
       */
      uint32_t *map = batch->map_next;

      map[0] = (0xA << 23);

      batch->map_next += 4;
   }
}

static void
iris_batch_reset(struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   const struct intel_device_info *devinfo = screen->devinfo;

   u_trace_fini(&batch->trace);

   iris_bo_unreference(batch->bo);
   batch->primary_batch_size = 0;
   batch->total_chained_batch_size = 0;
   batch->contains_draw = false;
   batch->contains_fence_signal = false;
   if (devinfo->ver < 11)
      batch->decoder.surface_base = batch->last_binder_address;
   else
      batch->decoder.bt_pool_base = batch->last_binder_address;

   create_batch(batch);
   assert(batch->bo->index == 0);

   memset(batch->bos_written, 0,
          sizeof(BITSET_WORD) * BITSET_WORDS(batch->exec_array_size));

   struct iris_syncobj *syncobj = iris_create_syncobj(bufmgr);
   iris_batch_add_syncobj(batch, syncobj, IRIS_BATCH_FENCE_SIGNAL);
   iris_syncobj_reference(bufmgr, &syncobj, NULL);

   assert(!batch->sync_region_depth);
   iris_batch_sync_boundary(batch);
   iris_batch_mark_reset_sync(batch);

   /* Always add the workaround BO, it contains a driver identifier at the
    * beginning quite helpful to debug error states.
    */
   add_bo_to_batch(batch, screen->workaround_bo, false);

   iris_batch_maybe_noop(batch);

   u_trace_init(&batch->trace, &batch->ice->ds.trace_context);
   batch->begin_trace_recorded = false;
}

static void
iris_batch_free(const struct iris_context *ice, struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   const struct intel_device_info *devinfo = iris_bufmgr_get_device_info(bufmgr);

   for (int i = 0; i < batch->exec_count; i++) {
      iris_bo_unreference(batch->exec_bos[i]);
   }
   free(batch->exec_bos);
   ralloc_free(batch->bos_written);

   ralloc_free(batch->exec_fences.mem_ctx);

   pipe_resource_reference(&batch->fine_fences.ref.res, NULL);

   util_dynarray_foreach(&batch->syncobjs, struct iris_syncobj *, s)
      iris_syncobj_reference(bufmgr, s, NULL);
   ralloc_free(batch->syncobjs.mem_ctx);

   iris_fine_fence_reference(batch->screen, &batch->last_fence, NULL);
   u_upload_destroy(batch->fine_fences.uploader);

   iris_bo_unreference(batch->bo);
   batch->bo = NULL;
   batch->map = NULL;
   batch->map_next = NULL;

   switch (devinfo->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      iris_i915_destroy_batch(batch);
      break;
   case INTEL_KMD_TYPE_XE:
      iris_xe_destroy_batch(batch);
      break;
   default:
      unreachable("missing");
   }

   iris_destroy_batch_measure(batch->measure);
   batch->measure = NULL;

   u_trace_fini(&batch->trace);

   _mesa_hash_table_destroy(batch->bo_aux_modes, NULL);

   if (INTEL_DEBUG(DEBUG_BATCH | DEBUG_BATCH_STATS))
      intel_batch_decode_ctx_finish(&batch->decoder);
}

void
iris_destroy_batches(struct iris_context *ice)
{
   iris_foreach_batch(ice, batch)
      iris_batch_free(ice, batch);
}

void iris_batch_maybe_begin_frame(struct iris_batch *batch)
{
   struct iris_context *ice = batch->ice;

   if (ice->utrace.begin_frame != ice->frame) {
      trace_intel_begin_frame(&batch->trace, batch);
      ice->utrace.begin_frame = ice->utrace.end_frame = ice->frame;
   }
}

/**
 * If we've chained to a secondary batch, or are getting near to the end,
 * then flush.  This should only be called between draws.
 */
void
iris_batch_maybe_flush(struct iris_batch *batch, unsigned estimate)
{
   if (batch->bo != batch->exec_bos[0] ||
       iris_batch_bytes_used(batch) + estimate >= BATCH_SZ) {
      iris_batch_flush(batch);
   }
}

static void
record_batch_sizes(struct iris_batch *batch)
{
   unsigned batch_size = iris_batch_bytes_used(batch);

   VG(VALGRIND_CHECK_MEM_IS_DEFINED(batch->map, batch_size));

   if (batch->bo == batch->exec_bos[0])
      batch->primary_batch_size = batch_size;

   batch->total_chained_batch_size += batch_size;
}

void
iris_chain_to_new_batch(struct iris_batch *batch)
{
   uint32_t *cmd = batch->map_next;
   uint64_t *addr = batch->map_next + 4;
   batch->map_next += 12;

   record_batch_sizes(batch);

   /* No longer held by batch->bo, still held by validation list */
   iris_bo_unreference(batch->bo);
   create_batch(batch);

   /* Emit MI_BATCH_BUFFER_START to chain to another batch. */
   *cmd = (0x31 << 23) | (1 << 8) | (3 - 2);
   *addr = batch->bo->address;
}

static void
add_aux_map_bos_to_batch(struct iris_batch *batch)
{
   void *aux_map_ctx = iris_bufmgr_get_aux_map_context(batch->screen->bufmgr);
   if (!aux_map_ctx)
      return;

   uint32_t count = intel_aux_map_get_num_buffers(aux_map_ctx);
   ensure_exec_obj_space(batch, count);
   intel_aux_map_fill_bos(aux_map_ctx,
                          (void**)&batch->exec_bos[batch->exec_count], count);
   for (uint32_t i = 0; i < count; i++) {
      struct iris_bo *bo = batch->exec_bos[batch->exec_count];
      add_bo_to_batch(batch, bo, false);
   }
}

static void
finish_seqno(struct iris_batch *batch)
{
   struct iris_fine_fence *sq = iris_fine_fence_new(batch);
   if (!sq)
      return;

   iris_fine_fence_reference(batch->screen, &batch->last_fence, sq);
   iris_fine_fence_reference(batch->screen, &sq, NULL);
}

/**
 * Terminate a batch with MI_BATCH_BUFFER_END.
 */
static void
iris_finish_batch(struct iris_batch *batch)
{
   const struct intel_device_info *devinfo = batch->screen->devinfo;

   if (devinfo->ver == 12 && batch->name == IRIS_BATCH_RENDER) {
      /* We re-emit constants at the beginning of every batch as a hardware
       * bug workaround, so invalidate indirect state pointers in order to
       * save ourselves the overhead of restoring constants redundantly when
       * the next render batch is executed.
       */
      iris_emit_pipe_control_flush(batch, "ISP invalidate at batch end",
                                   PIPE_CONTROL_INDIRECT_STATE_POINTERS_DISABLE |
                                   PIPE_CONTROL_STALL_AT_SCOREBOARD |
                                   PIPE_CONTROL_CS_STALL);
   }

   add_aux_map_bos_to_batch(batch);

   finish_seqno(batch);

   trace_intel_end_batch(&batch->trace, batch->name);

   struct iris_context *ice = batch->ice;
   if (ice->utrace.end_frame != ice->frame) {
      trace_intel_end_frame(&batch->trace, batch, ice->utrace.end_frame);
      ice->utrace.end_frame = ice->frame;
   }

   /* Emit MI_BATCH_BUFFER_END to finish our batch. */
   uint32_t *map = batch->map_next;

   map[0] = (0xA << 23);

   batch->map_next += 4;

   record_batch_sizes(batch);
}

/**
 * Replace our current GEM context with a new one (in case it got banned).
 */
static bool
replace_kernel_ctx(struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   const struct intel_device_info *devinfo = iris_bufmgr_get_device_info(bufmgr);

   threaded_context_unwrap_sync(&batch->ice->ctx);

   switch (devinfo->kmd_type) {
   case INTEL_KMD_TYPE_I915:
      return iris_i915_replace_batch(batch);
   case INTEL_KMD_TYPE_XE:
      return iris_xe_replace_batch(batch);
   default:
      unreachable("missing");
      return false;
   }
}

enum pipe_reset_status
iris_batch_check_for_reset(struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   struct iris_context *ice = batch->ice;
   const struct iris_kmd_backend *backend;
   enum pipe_reset_status status = PIPE_NO_RESET;

   /* Banned context was already signalled to application */
   if (ice->context_reset_signaled)
      return status;

   backend = iris_bufmgr_get_kernel_driver_backend(bufmgr);
   status = backend->batch_check_for_reset(batch);

   if (status != PIPE_NO_RESET)
      ice->context_reset_signaled = true;

   return status;
}

static void
move_syncobj_to_batch(struct iris_batch *batch,
                      struct iris_syncobj **p_syncobj,
                      uint32_t flags)
{
   struct iris_bufmgr *bufmgr = batch->screen->bufmgr;

   if (!*p_syncobj)
      return;

   bool found = false;
   util_dynarray_foreach(&batch->syncobjs, struct iris_syncobj *, s) {
      if (*p_syncobj == *s) {
         found = true;
         break;
      }
   }

   if (!found)
      iris_batch_add_syncobj(batch, *p_syncobj, flags);

   iris_syncobj_reference(bufmgr, p_syncobj, NULL);
}

static void
update_bo_syncobjs(struct iris_batch *batch, struct iris_bo *bo, bool write)
{
   struct iris_screen *screen = batch->screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   struct iris_context *ice = batch->ice;

   simple_mtx_assert_locked(iris_bufmgr_get_bo_deps_lock(bufmgr));

   /* Make sure bo->deps is big enough */
   if (screen->id >= bo->deps_size) {
      int new_size = screen->id + 1;
      bo->deps = realloc(bo->deps, new_size * sizeof(bo->deps[0]));
      assert(bo->deps);
      memset(&bo->deps[bo->deps_size], 0,
             sizeof(bo->deps[0]) * (new_size - bo->deps_size));

      bo->deps_size = new_size;
   }

   /* When it comes to execbuf submission of non-shared buffers, we only need
    * to care about the reads and writes done by the other batches of our own
    * screen, and we also don't care about the reads and writes done by our
    * own batch, although we need to track them. Just note that other places of
    * our code may need to care about all the operations done by every batch
    * on every screen.
    */
   struct iris_bo_screen_deps *bo_deps = &bo->deps[screen->id];
   int batch_idx = batch->name;

   /* Make our batch depend on additional syncobjs depending on what other
    * batches have been doing to this bo.
    *
    * We also look at the dependencies set by our own batch since those could
    * have come from a different context, and apps don't like it when we don't
    * do inter-context tracking.
    */
   iris_foreach_batch(ice, batch_i) {
      unsigned i = batch_i->name;

      /* If the bo is being written to by others, wait for them. */
      if (bo_deps->write_syncobjs[i])
         move_syncobj_to_batch(batch, &bo_deps->write_syncobjs[i],
                               IRIS_BATCH_FENCE_WAIT);

      /* If we're writing to the bo, wait on the reads from other batches. */
      if (write)
         move_syncobj_to_batch(batch, &bo_deps->read_syncobjs[i],
                               IRIS_BATCH_FENCE_WAIT);
   }

   struct iris_syncobj *batch_syncobj =
      iris_batch_get_signal_syncobj(batch);

   /* Update bo_deps depending on what we're doing with the bo in this batch
    * by putting the batch's syncobj in the bo_deps lists accordingly. Only
    * keep track of the last time we wrote to or read the BO.
    */
   if (write) {
      iris_syncobj_reference(bufmgr, &bo_deps->write_syncobjs[batch_idx],
                             batch_syncobj);
   } else {
      iris_syncobj_reference(bufmgr, &bo_deps->read_syncobjs[batch_idx],
                             batch_syncobj);
   }
}

void
iris_batch_update_syncobjs(struct iris_batch *batch)
{
   for (int i = 0; i < batch->exec_count; i++) {
      struct iris_bo *bo = batch->exec_bos[i];
      bool write = BITSET_TEST(batch->bos_written, i);

      if (bo == batch->screen->workaround_bo)
         continue;

      update_bo_syncobjs(batch, bo, write);
   }
}

/**
 * Convert the syncobj which will be signaled when this batch completes
 * to a SYNC_FILE object, for use with import/export sync ioctls.
 */
bool
iris_batch_syncobj_to_sync_file_fd(struct iris_batch *batch, int *out_fd)
{
   int drm_fd = batch->screen->fd;

   struct iris_syncobj *batch_syncobj =
      iris_batch_get_signal_syncobj(batch);

   struct drm_syncobj_handle syncobj_to_fd_ioctl = {
      .handle = batch_syncobj->handle,
      .flags = DRM_SYNCOBJ_HANDLE_TO_FD_FLAGS_EXPORT_SYNC_FILE,
      .fd = -1,
   };
   if (intel_ioctl(drm_fd, DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD,
                   &syncobj_to_fd_ioctl)) {
      fprintf(stderr, "DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD ioctl failed (%d)\n",
              errno);
      return false;
   }

   assert(syncobj_to_fd_ioctl.fd >= 0);
   *out_fd = syncobj_to_fd_ioctl.fd;

   return true;
}

const char *
iris_batch_name_to_string(enum iris_batch_name name)
{
   const char *names[IRIS_BATCH_COUNT] = {
      [IRIS_BATCH_RENDER]  = "render",
      [IRIS_BATCH_COMPUTE] = "compute",
      [IRIS_BATCH_BLITTER] = "blitter",
   };
   return names[name];
}

static inline bool
context_or_exec_queue_was_banned(struct iris_bufmgr *bufmgr, int ret)
{
   enum intel_kmd_type kmd_type = iris_bufmgr_get_device_info(bufmgr)->kmd_type;

   /* In i915 EIO means our context is banned, while on Xe ECANCELED means
    * our exec queue was banned
    */
   if ((kmd_type == INTEL_KMD_TYPE_I915 && ret == -EIO) ||
       (kmd_type == INTEL_KMD_TYPE_XE && ret == -ECANCELED))
      return true;

   return false;
}

/**
 * Flush the batch buffer, submitting it to the GPU and resetting it so
 * we're ready to emit the next batch.
 */
void
_iris_batch_flush(struct iris_batch *batch, const char *file, int line)
{
   struct iris_screen *screen = batch->screen;
   struct iris_context *ice = batch->ice;
   struct iris_bufmgr *bufmgr = screen->bufmgr;

   /* If a fence signals we need to flush it. */
   if (iris_batch_bytes_used(batch) == 0 && !batch->contains_fence_signal)
      return;

   iris_measure_batch_end(ice, batch);

   iris_finish_batch(batch);

   if (INTEL_DEBUG(DEBUG_BATCH | DEBUG_SUBMIT | DEBUG_PIPE_CONTROL)) {
      const char *basefile = strstr(file, "iris/");
      if (basefile)
         file = basefile + 5;

      enum intel_kmd_type kmd_type = iris_bufmgr_get_device_info(bufmgr)->kmd_type;
      uint32_t batch_ctx_id = kmd_type == INTEL_KMD_TYPE_I915 ?
                              batch->i915.ctx_id : batch->xe.exec_queue_id;
      fprintf(stderr, "%19s:%-3d: %s batch [%u] flush with %5db (%0.1f%%) "
              "(cmds), %4d BOs (%0.1fMb aperture)\n",
              file, line, iris_batch_name_to_string(batch->name),
              batch_ctx_id, batch->total_chained_batch_size,
              100.0f * batch->total_chained_batch_size / BATCH_SZ,
              batch->exec_count,
              (float) batch->aperture_space / (1024 * 1024));

   }

   uint64_t start_ts = intel_ds_begin_submit(&batch->ds);
   uint64_t submission_id = batch->ds.submission_id;
   int ret = iris_bufmgr_get_kernel_driver_backend(bufmgr)->batch_submit(batch);
   intel_ds_end_submit(&batch->ds, start_ts);

   /* When batch submission fails, our end-of-batch syncobj remains
    * unsignalled, and in fact is not even considered submitted.
    *
    * In the hang recovery case (-EIO) or -ENOMEM, we recreate our context and
    * attempt to carry on.  In that case, we need to signal our syncobj,
    * dubiously claiming that this batch completed, because future batches may
    * depend on it.  If we don't, then execbuf would fail with -EINVAL for
    * those batches, because they depend on a syncobj that's considered to be
    * "never submitted".  This would lead to an abort().  So here, we signal
    * the failing batch's syncobj to try and allow further progress to be
    * made, knowing we may have broken our dependency tracking.
    */
   if (ret < 0)
      iris_syncobj_signal(screen->bufmgr, iris_batch_get_signal_syncobj(batch));

   batch->exec_count = 0;
   batch->max_gem_handle = 0;
   batch->aperture_space = 0;

   util_dynarray_foreach(&batch->syncobjs, struct iris_syncobj *, s)
      iris_syncobj_reference(screen->bufmgr, s, NULL);
   util_dynarray_clear(&batch->syncobjs);

   util_dynarray_clear(&batch->exec_fences);

   if (INTEL_DEBUG(DEBUG_SYNC)) {
      dbg_printf("waiting for idle\n");
      iris_bo_wait_rendering(batch->bo); /* if execbuf failed; this is a nop */
   }

   if (u_trace_should_process(&ice->ds.trace_context))
      iris_utrace_flush(batch, submission_id);

   /* Start a new batch buffer. */
   iris_batch_reset(batch);

   /* Check if context or engine was banned, if yes try to replace it
    * with a new logical context, and inform iris_context that all state
    * has been lost and needs to be re-initialized.  If this succeeds,
    * dubiously claim success...
    */
   if (ret && context_or_exec_queue_was_banned(bufmgr, ret)) {
      enum pipe_reset_status status = iris_batch_check_for_reset(batch);

      if (status != PIPE_NO_RESET || ice->context_reset_signaled)
         replace_kernel_ctx(batch);

      if (batch->reset->reset) {
         /* Tell gallium frontends the device is lost and it was our fault. */
         batch->reset->reset(batch->reset->data, status);
      }

      ret = 0;
   }

   if (ret < 0) {
#ifdef DEBUG
      const bool color = INTEL_DEBUG(DEBUG_COLOR);
      fprintf(stderr, "%siris: Failed to submit batchbuffer: %-80s%s\n",
              color ? "\e[1;41m" : "", strerror(-ret), color ? "\e[0m" : "");
#endif
      abort();
   }
}

/**
 * Does the current batch refer to the given BO?
 *
 * (In other words, is the BO in the current batch's validation list?)
 */
bool
iris_batch_references(struct iris_batch *batch, struct iris_bo *bo)
{
   return find_exec_index(batch, bo) != -1;
}

/**
 * Updates the state of the noop feature.  Returns true if there was a noop
 * transition that led to state invalidation.
 */
bool
iris_batch_prepare_noop(struct iris_batch *batch, bool noop_enable)
{
   if (batch->noop_enabled == noop_enable)
      return 0;

   batch->noop_enabled = noop_enable;

   iris_batch_flush(batch);

   /* If the batch was empty, flush had no effect, so insert our noop. */
   if (iris_batch_bytes_used(batch) == 0)
      iris_batch_maybe_noop(batch);

   /* We only need to update the entire state if we transition from noop ->
    * not-noop.
    */
   return !batch->noop_enabled;
}
