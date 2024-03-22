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
 * @file crocus_batch.c
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

#include "crocus_batch.h"
#include "crocus_bufmgr.h"
#include "crocus_context.h"
#include "crocus_fence.h"

#include "drm-uapi/i915_drm.h"

#include "intel/common/intel_gem.h"
#include "util/hash_table.h"
#include "util/set.h"
#include "util/u_upload_mgr.h"

#include <errno.h>
#include <xf86drm.h>

#if HAVE_VALGRIND
#include <memcheck.h>
#include <valgrind.h>
#define VG(x) x
#else
#define VG(x)
#endif

#define FILE_DEBUG_FLAG DEBUG_BUFMGR

/* Terminating the batch takes either 4 bytes for MI_BATCH_BUFFER_END
 * or 12 bytes for MI_BATCH_BUFFER_START (when chaining).  Plus, we may
 * need an extra 4 bytes to pad out to the nearest QWord.  So reserve 16.
 */
#define BATCH_RESERVED(devinfo) ((devinfo)->platform == INTEL_PLATFORM_HSW ? 32 : 16)

static void crocus_batch_reset(struct crocus_batch *batch);

static unsigned
num_fences(struct crocus_batch *batch)
{
   return util_dynarray_num_elements(&batch->exec_fences,
                                     struct drm_i915_gem_exec_fence);
}

/**
 * Debugging code to dump the fence list, used by INTEL_DEBUG=submit.
 */
static void
dump_fence_list(struct crocus_batch *batch)
{
   fprintf(stderr, "Fence list (length %u):      ", num_fences(batch));

   util_dynarray_foreach(&batch->exec_fences,
                         struct drm_i915_gem_exec_fence, f) {
      fprintf(stderr, "%s%u%s ",
              (f->flags & I915_EXEC_FENCE_WAIT) ? "..." : "",
              f->handle,
              (f->flags & I915_EXEC_FENCE_SIGNAL) ? "!" : "");
   }

   fprintf(stderr, "\n");
}

/**
 * Debugging code to dump the validation list, used by INTEL_DEBUG=submit.
 */
static void
dump_validation_list(struct crocus_batch *batch)
{
   fprintf(stderr, "Validation list (length %d):\n", batch->exec_count);

   for (int i = 0; i < batch->exec_count; i++) {
      uint64_t flags = batch->validation_list[i].flags;
      assert(batch->validation_list[i].handle ==
             batch->exec_bos[i]->gem_handle);
      fprintf(stderr,
              "[%2d]: %2d %-14s @ 0x%"PRIx64" (%" PRIu64 "B)\t %2d refs %s\n", i,
              batch->validation_list[i].handle, batch->exec_bos[i]->name,
              (uint64_t)batch->validation_list[i].offset, batch->exec_bos[i]->size,
              batch->exec_bos[i]->refcount,
              (flags & EXEC_OBJECT_WRITE) ? " (write)" : "");
   }
}

/**
 * Return BO information to the batch decoder (for debugging).
 */
static struct intel_batch_decode_bo
decode_get_bo(void *v_batch, bool ppgtt, uint64_t address)
{
   struct crocus_batch *batch = v_batch;

   for (int i = 0; i < batch->exec_count; i++) {
      struct crocus_bo *bo = batch->exec_bos[i];
      /* The decoder zeroes out the top 16 bits, so we need to as well */
      uint64_t bo_address = bo->gtt_offset & (~0ull >> 16);

      if (address >= bo_address && address < bo_address + bo->size) {
         return (struct intel_batch_decode_bo){
            .addr = address,
            .size = bo->size,
            .map = crocus_bo_map(batch->dbg, bo, MAP_READ) +
                   (address - bo_address),
         };
      }
   }

   return (struct intel_batch_decode_bo) { };
}

static unsigned
decode_get_state_size(void *v_batch, uint64_t address,
                      uint64_t base_address)
{
   struct crocus_batch *batch = v_batch;

   /* The decoder gives us offsets from a base address, which is not great.
    * Binding tables are relative to surface state base address, and other
    * state is relative to dynamic state base address.  These could alias,
    * but in practice it's unlikely because surface offsets are always in
    * the [0, 64K) range, and we assign dynamic state addresses starting at
    * the top of the 4GB range.  We should fix this but it's likely good
    * enough for now.
    */
   unsigned size = (uintptr_t)
      _mesa_hash_table_u64_search(batch->state_sizes, address - base_address);

   return size;
}

/**
 * Decode the current batch.
 */
static void
decode_batch(struct crocus_batch *batch)
{
   void *map = crocus_bo_map(batch->dbg, batch->exec_bos[0], MAP_READ);
   intel_print_batch(&batch->decoder, map, batch->primary_batch_size,
                     batch->exec_bos[0]->gtt_offset, false);
}

static void
init_reloc_list(struct crocus_reloc_list *rlist, int count)
{
   rlist->reloc_count = 0;
   rlist->reloc_array_size = count;
   rlist->relocs = malloc(rlist->reloc_array_size *
                          sizeof(struct drm_i915_gem_relocation_entry));
}

void
crocus_init_batch(struct crocus_context *ice,
                  enum crocus_batch_name name,
                  int priority)
{
   struct crocus_batch *batch = &ice->batches[name];
   struct crocus_screen *screen = (struct crocus_screen *)ice->ctx.screen;
   struct intel_device_info *devinfo = &screen->devinfo;

   batch->ice = ice;
   batch->screen = screen;
   batch->dbg = &ice->dbg;
   batch->reset = &ice->reset;
   batch->name = name;
   batch->contains_fence_signal = false;

   if (devinfo->ver >= 7) {
      batch->fine_fences.uploader =
         u_upload_create(&ice->ctx, 4096, PIPE_BIND_CUSTOM,
                         PIPE_USAGE_STAGING, 0);
   }
   crocus_fine_fence_init(batch);

   batch->hw_ctx_id = crocus_create_hw_context(screen->bufmgr);
   assert(batch->hw_ctx_id);

   crocus_hw_context_set_priority(screen->bufmgr, batch->hw_ctx_id, priority);

   batch->valid_reloc_flags = EXEC_OBJECT_WRITE;
   if (devinfo->ver == 6)
      batch->valid_reloc_flags |= EXEC_OBJECT_NEEDS_GTT;

   if (INTEL_DEBUG(DEBUG_BATCH)) {
      /* The shadow doesn't get relocs written so state decode fails. */
      batch->use_shadow_copy = false;
   } else
      batch->use_shadow_copy = !devinfo->has_llc;

   util_dynarray_init(&batch->exec_fences, ralloc_context(NULL));
   util_dynarray_init(&batch->syncobjs, ralloc_context(NULL));

   init_reloc_list(&batch->command.relocs, 250);
   init_reloc_list(&batch->state.relocs, 250);

   batch->exec_count = 0;
   batch->exec_array_size = 100;
   batch->exec_bos =
      malloc(batch->exec_array_size * sizeof(batch->exec_bos[0]));
   batch->validation_list =
      malloc(batch->exec_array_size * sizeof(batch->validation_list[0]));

   batch->cache.render = _mesa_hash_table_create(NULL, NULL,
                                                 _mesa_key_pointer_equal);
   batch->cache.depth = _mesa_set_create(NULL, NULL,
                                         _mesa_key_pointer_equal);

   memset(batch->other_batches, 0, sizeof(batch->other_batches));

   for (int i = 0, j = 0; i < ice->batch_count; i++) {
      if (i != name)
         batch->other_batches[j++] = &ice->batches[i];
   }

   if (INTEL_DEBUG(DEBUG_BATCH)) {

      batch->state_sizes = _mesa_hash_table_u64_create(NULL);
      const unsigned decode_flags = INTEL_BATCH_DECODE_DEFAULT_FLAGS |
         (INTEL_DEBUG(DEBUG_COLOR) ? INTEL_BATCH_DECODE_IN_COLOR : 0);

      intel_batch_decode_ctx_init(&batch->decoder, &screen->compiler->isa,
                                  &screen->devinfo, stderr,
                                  decode_flags, NULL, decode_get_bo,
                                  decode_get_state_size, batch);
      batch->decoder.max_vbo_decoded_lines = 32;
   }

   crocus_batch_reset(batch);
}

static int
find_exec_index(struct crocus_batch *batch, struct crocus_bo *bo)
{
   unsigned index = READ_ONCE(bo->index);

   if (index < batch->exec_count && batch->exec_bos[index] == bo)
      return index;

   /* May have been shared between multiple active batches */
   for (index = 0; index < batch->exec_count; index++) {
      if (batch->exec_bos[index] == bo)
	 return index;
   }
   return -1;
}

static struct drm_i915_gem_exec_object2 *
find_validation_entry(struct crocus_batch *batch, struct crocus_bo *bo)
{
   int index = find_exec_index(batch, bo);

   if (index == -1)
      return NULL;
   return &batch->validation_list[index];
}

static void
ensure_exec_obj_space(struct crocus_batch *batch, uint32_t count)
{
   while (batch->exec_count + count > batch->exec_array_size) {
      batch->exec_array_size *= 2;
      batch->exec_bos = realloc(
         batch->exec_bos, batch->exec_array_size * sizeof(batch->exec_bos[0]));
      batch->validation_list =
         realloc(batch->validation_list,
                 batch->exec_array_size * sizeof(batch->validation_list[0]));
   }
}

static struct drm_i915_gem_exec_object2 *
crocus_use_bo(struct crocus_batch *batch, struct crocus_bo *bo, bool writable)
{
   assert(bo->bufmgr == batch->command.bo->bufmgr);

   struct drm_i915_gem_exec_object2 *existing_entry =
      find_validation_entry(batch, bo);

   if (existing_entry) {
      /* The BO is already in the validation list; mark it writable */
      if (writable)
         existing_entry->flags |= EXEC_OBJECT_WRITE;
      return existing_entry;
   }

   if (bo != batch->command.bo && bo != batch->state.bo) {
      /* This is the first time our batch has seen this BO.  Before we use it,
       * we may need to flush and synchronize with other batches.
       */
      for (int b = 0; b < ARRAY_SIZE(batch->other_batches); b++) {

         if (!batch->other_batches[b])
            continue;
         struct drm_i915_gem_exec_object2 *other_entry =
            find_validation_entry(batch->other_batches[b], bo);

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
         if (other_entry &&
             ((other_entry->flags & EXEC_OBJECT_WRITE) || writable)) {
            crocus_batch_flush(batch->other_batches[b]);
            crocus_batch_add_syncobj(batch,
                                     batch->other_batches[b]->last_fence->syncobj,
                                     I915_EXEC_FENCE_WAIT);
         }
      }
   }

   /* Bump the ref count since the batch is now using this bo. */
   crocus_bo_reference(bo);

   ensure_exec_obj_space(batch, 1);

   batch->validation_list[batch->exec_count] =
      (struct drm_i915_gem_exec_object2) {
         .handle = bo->gem_handle,
         .offset = bo->gtt_offset,
         .flags = bo->kflags | (writable ? EXEC_OBJECT_WRITE : 0),
      };

   bo->index = batch->exec_count;
   batch->exec_bos[batch->exec_count] = bo;
   batch->aperture_space += bo->size;

   batch->exec_count++;

   return &batch->validation_list[batch->exec_count - 1];
}

static uint64_t
emit_reloc(struct crocus_batch *batch,
           struct crocus_reloc_list *rlist, uint32_t offset,
           struct crocus_bo *target, int32_t target_offset,
           unsigned int reloc_flags)
{
   assert(target != NULL);

   if (target == batch->ice->workaround_bo)
      reloc_flags &= ~RELOC_WRITE;

   bool writable = reloc_flags & RELOC_WRITE;

   struct drm_i915_gem_exec_object2 *entry =
      crocus_use_bo(batch, target, writable);

   if (rlist->reloc_count == rlist->reloc_array_size) {
      rlist->reloc_array_size *= 2;
      rlist->relocs = realloc(rlist->relocs,
                              rlist->reloc_array_size *
                              sizeof(struct drm_i915_gem_relocation_entry));
   }

   if (reloc_flags & RELOC_32BIT) {
      /* Restrict this buffer to the low 32 bits of the address space.
       *
       * Altering the validation list flags restricts it for this batch,
       * but we also alter the BO's kflags to restrict it permanently
       * (until the BO is destroyed and put back in the cache).  Buffers
       * may stay bound across batches, and we want keep it constrained.
       */
      target->kflags &= ~EXEC_OBJECT_SUPPORTS_48B_ADDRESS;
      entry->flags &= ~EXEC_OBJECT_SUPPORTS_48B_ADDRESS;

      /* RELOC_32BIT is not an EXEC_OBJECT_* flag, so get rid of it. */
      reloc_flags &= ~RELOC_32BIT;
   }

   if (reloc_flags)
      entry->flags |= reloc_flags & batch->valid_reloc_flags;

   rlist->relocs[rlist->reloc_count++] =
      (struct drm_i915_gem_relocation_entry) {
         .offset = offset,
         .delta = target_offset,
         .target_handle = find_exec_index(batch, target),
         .presumed_offset = entry->offset,
      };

   /* Using the old buffer offset, write in what the right data would be, in
    * case the buffer doesn't move and we can short-circuit the relocation
    * processing in the kernel
    */
   return entry->offset + target_offset;
}

uint64_t
crocus_command_reloc(struct crocus_batch *batch, uint32_t batch_offset,
                     struct crocus_bo *target, uint32_t target_offset,
                     unsigned int reloc_flags)
{
   assert(batch_offset <= batch->command.bo->size - sizeof(uint32_t));

   return emit_reloc(batch, &batch->command.relocs, batch_offset,
                     target, target_offset, reloc_flags);
}

uint64_t
crocus_state_reloc(struct crocus_batch *batch, uint32_t state_offset,
                   struct crocus_bo *target, uint32_t target_offset,
                   unsigned int reloc_flags)
{
   assert(state_offset <= batch->state.bo->size - sizeof(uint32_t));

   return emit_reloc(batch, &batch->state.relocs, state_offset,
                     target, target_offset, reloc_flags);
}

static void
recreate_growing_buffer(struct crocus_batch *batch,
                        struct crocus_growing_bo *grow,
                        const char *name, unsigned size)
{
   struct crocus_screen *screen = batch->screen;
   struct crocus_bufmgr *bufmgr = screen->bufmgr;
   grow->bo = crocus_bo_alloc(bufmgr, name, size);
   grow->bo->kflags |= EXEC_OBJECT_CAPTURE;
   grow->partial_bo = NULL;
   grow->partial_bo_map = NULL;
   grow->partial_bytes = 0;
   if (batch->use_shadow_copy)
      grow->map = realloc(grow->map, grow->bo->size);
   else
      grow->map = crocus_bo_map(NULL, grow->bo, MAP_READ | MAP_WRITE);
   grow->map_next = grow->map;
}

static void
create_batch(struct crocus_batch *batch)
{
   struct crocus_screen *screen = batch->screen;

   recreate_growing_buffer(batch, &batch->command,
                           "command buffer",
                           BATCH_SZ + BATCH_RESERVED(&screen->devinfo));

   crocus_use_bo(batch, batch->command.bo, false);

   /* Always add workaround_bo which contains a driver identifier to be
    * recorded in error states.
    */
   crocus_use_bo(batch, batch->ice->workaround_bo, false);

   recreate_growing_buffer(batch, &batch->state,
                           "state buffer",
                           STATE_SZ);

   batch->state.used = 1;
   crocus_use_bo(batch, batch->state.bo, false);
}

static void
crocus_batch_maybe_noop(struct crocus_batch *batch)
{
   /* We only insert the NOOP at the beginning of the batch. */
   assert(crocus_batch_bytes_used(batch) == 0);

   if (batch->noop_enabled) {
      /* Emit MI_BATCH_BUFFER_END to prevent any further command to be
       * executed.
       */
      uint32_t *map = batch->command.map_next;

      map[0] = (0xA << 23);

      batch->command.map_next += 4;
   }
}

static void
crocus_batch_reset(struct crocus_batch *batch)
{
   struct crocus_screen *screen = batch->screen;

   crocus_bo_unreference(batch->command.bo);
   crocus_bo_unreference(batch->state.bo);
   batch->primary_batch_size = 0;
   batch->contains_draw = false;
   batch->contains_fence_signal = false;
   batch->state_base_address_emitted = false;
   batch->screen->vtbl.batch_reset_dirty(batch);

   create_batch(batch);
   assert(batch->command.bo->index == 0);

   if (batch->state_sizes)
      _mesa_hash_table_u64_clear(batch->state_sizes);
   struct crocus_syncobj *syncobj = crocus_create_syncobj(screen);
   crocus_batch_add_syncobj(batch, syncobj, I915_EXEC_FENCE_SIGNAL);
   crocus_syncobj_reference(screen, &syncobj, NULL);

   crocus_cache_sets_clear(batch);
}

void
crocus_batch_free(struct crocus_batch *batch)
{
   struct crocus_screen *screen = batch->screen;
   struct crocus_bufmgr *bufmgr = screen->bufmgr;

   if (batch->use_shadow_copy) {
      free(batch->command.map);
      free(batch->state.map);
   }

   for (int i = 0; i < batch->exec_count; i++) {
      crocus_bo_unreference(batch->exec_bos[i]);
   }

   pipe_resource_reference(&batch->fine_fences.ref.res, NULL);

   free(batch->command.relocs.relocs);
   free(batch->state.relocs.relocs);
   free(batch->exec_bos);
   free(batch->validation_list);

   ralloc_free(batch->exec_fences.mem_ctx);

   util_dynarray_foreach(&batch->syncobjs, struct crocus_syncobj *, s)
      crocus_syncobj_reference(screen, s, NULL);
   ralloc_free(batch->syncobjs.mem_ctx);

   crocus_fine_fence_reference(batch->screen, &batch->last_fence, NULL);
   if (batch_has_fine_fence(batch))
      u_upload_destroy(batch->fine_fences.uploader);

   crocus_bo_unreference(batch->command.bo);
   crocus_bo_unreference(batch->state.bo);
   batch->command.bo = NULL;
   batch->command.map = NULL;
   batch->command.map_next = NULL;

   crocus_destroy_hw_context(bufmgr, batch->hw_ctx_id);

   _mesa_hash_table_destroy(batch->cache.render, NULL);
   _mesa_set_destroy(batch->cache.depth, NULL);

   if (batch->state_sizes) {
      _mesa_hash_table_u64_destroy(batch->state_sizes);
      intel_batch_decode_ctx_finish(&batch->decoder);
   }
}

/**
 * If we've chained to a secondary batch, or are getting near to the end,
 * then flush.  This should only be called between draws.
 */
void
crocus_batch_maybe_flush(struct crocus_batch *batch, unsigned estimate)
{
   if (batch->command.bo != batch->exec_bos[0] ||
       crocus_batch_bytes_used(batch) + estimate >= BATCH_SZ) {
      crocus_batch_flush(batch);
   }
}

/**
 * Finish copying the old batch/state buffer's contents to the new one
 * after we tried to "grow" the buffer in an earlier operation.
 */
static void
finish_growing_bos(struct crocus_growing_bo *grow)
{
   struct crocus_bo *old_bo = grow->partial_bo;
   if (!old_bo)
      return;

   memcpy(grow->map, grow->partial_bo_map, grow->partial_bytes);

   grow->partial_bo = NULL;
   grow->partial_bo_map = NULL;
   grow->partial_bytes = 0;

   crocus_bo_unreference(old_bo);
}

void
crocus_grow_buffer(struct crocus_batch *batch, bool grow_state,
                   unsigned used,
                   unsigned new_size)
{
   struct crocus_screen *screen = batch->screen;
   struct crocus_bufmgr *bufmgr = screen->bufmgr;
   struct crocus_growing_bo *grow = grow_state ? &batch->state : &batch->command;
   struct crocus_bo *bo = grow->bo;

   if (grow->partial_bo) {
      /* We've already grown once, and now we need to do it again.
       * Finish our last grow operation so we can start a new one.
       * This should basically never happen.
       */
      finish_growing_bos(grow);
   }

   struct crocus_bo *new_bo = crocus_bo_alloc(bufmgr, bo->name, new_size);

   /* Copy existing data to the new larger buffer */
   grow->partial_bo_map = grow->map;

   if (batch->use_shadow_copy) {
      /* We can't safely use realloc, as it may move the existing buffer,
       * breaking existing pointers the caller may still be using.  Just
       * malloc a new copy and memcpy it like the normal BO path.
       *
       * Use bo->size rather than new_size because the bufmgr may have
       * rounded up the size, and we want the shadow size to match.
       */
      grow->map = malloc(new_bo->size);
   } else {
      grow->map = crocus_bo_map(NULL, new_bo, MAP_READ | MAP_WRITE);
   }
   /* Try to put the new BO at the same GTT offset as the old BO (which
    * we're throwing away, so it doesn't need to be there).
    *
    * This guarantees that our relocations continue to work: values we've
    * already written into the buffer, values we're going to write into the
    * buffer, and the validation/relocation lists all will match.
    *
    * Also preserve kflags for EXEC_OBJECT_CAPTURE.
    */
   new_bo->gtt_offset = bo->gtt_offset;
   new_bo->index = bo->index;
   new_bo->kflags = bo->kflags;

   /* Batch/state buffers are per-context, and if we've run out of space,
    * we must have actually used them before, so...they will be in the list.
    */
   assert(bo->index < batch->exec_count);
   assert(batch->exec_bos[bo->index] == bo);

   /* Update the validation list to use the new BO. */
   batch->validation_list[bo->index].handle = new_bo->gem_handle;
   /* Exchange the two BOs...without breaking pointers to the old BO.
    *
    * Consider this scenario:
    *
    * 1. Somebody calls brw_state_batch() to get a region of memory, and
    *    and then creates a brw_address pointing to brw->batch.state.bo.
    * 2. They then call brw_state_batch() a second time, which happens to
    *    grow and replace the state buffer.  They then try to emit a
    *    relocation to their first section of memory.
    *
    * If we replace the brw->batch.state.bo pointer at step 2, we would
    * break the address created in step 1.  They'd have a pointer to the
    * old destroyed BO.  Emitting a relocation would add this dead BO to
    * the validation list...causing /both/ statebuffers to be in the list,
    * and all kinds of disasters.
    *
    * This is not a contrived case - BLORP vertex data upload hits this.
    *
    * There are worse scenarios too.  Fences for GL sync objects reference
    * brw->batch.batch.bo.  If we replaced the batch pointer when growing,
    * we'd need to chase down every fence and update it to point to the
    * new BO.  Otherwise, it would refer to a "batch" that never actually
    * gets submitted, and would fail to trigger.
    *
    * To work around both of these issues, we transmutate the buffers in
    * place, making the existing struct brw_bo represent the new buffer,
    * and "new_bo" represent the old BO.  This is highly unusual, but it
    * seems like a necessary evil.
    *
    * We also defer the memcpy of the existing batch's contents.  Callers
    * may make multiple brw_state_batch calls, and retain pointers to the
    * old BO's map.  We'll perform the memcpy in finish_growing_bo() when
    * we finally submit the batch, at which point we've finished uploading
    * state, and nobody should have any old references anymore.
    *
    * To do that, we keep a reference to the old BO in grow->partial_bo,
    * and store the number of bytes to copy in grow->partial_bytes.  We
    * can monkey with the refcounts directly without atomics because these
    * are per-context BOs and they can only be touched by this thread.
    */
   assert(new_bo->refcount == 1);
   new_bo->refcount = bo->refcount;
   bo->refcount = 1;

   struct crocus_bo tmp;
   memcpy(&tmp, bo, sizeof(struct crocus_bo));
   memcpy(bo, new_bo, sizeof(struct crocus_bo));
   memcpy(new_bo, &tmp, sizeof(struct crocus_bo));

   grow->partial_bo = new_bo; /* the one reference of the OLD bo */
   grow->partial_bytes = used;
}

static void
finish_seqno(struct crocus_batch *batch)
{
   struct crocus_fine_fence *sq = crocus_fine_fence_new(batch, CROCUS_FENCE_END);
   if (!sq)
      return;

   crocus_fine_fence_reference(batch->screen, &batch->last_fence, sq);
   crocus_fine_fence_reference(batch->screen, &sq, NULL);
}

/**
 * Terminate a batch with MI_BATCH_BUFFER_END.
 */
static void
crocus_finish_batch(struct crocus_batch *batch)
{

   batch->no_wrap = true;
   if (batch->screen->vtbl.finish_batch)
      batch->screen->vtbl.finish_batch(batch);

   finish_seqno(batch);

   /* Emit MI_BATCH_BUFFER_END to finish our batch. */
   uint32_t *map = batch->command.map_next;

   map[0] = (0xA << 23);

   batch->command.map_next += 4;
   VG(VALGRIND_CHECK_MEM_IS_DEFINED(batch->command.map, crocus_batch_bytes_used(batch)));

   if (batch->command.bo == batch->exec_bos[0])
      batch->primary_batch_size = crocus_batch_bytes_used(batch);
   batch->no_wrap = false;
}

/**
 * Replace our current GEM context with a new one (in case it got banned).
 */
static bool
replace_hw_ctx(struct crocus_batch *batch)
{
   struct crocus_screen *screen = batch->screen;
   struct crocus_bufmgr *bufmgr = screen->bufmgr;

   uint32_t new_ctx = crocus_clone_hw_context(bufmgr, batch->hw_ctx_id);
   if (!new_ctx)
      return false;

   crocus_destroy_hw_context(bufmgr, batch->hw_ctx_id);
   batch->hw_ctx_id = new_ctx;

   /* Notify the context that state must be re-initialized. */
   crocus_lost_context_state(batch);

   return true;
}

enum pipe_reset_status
crocus_batch_check_for_reset(struct crocus_batch *batch)
{
   struct crocus_screen *screen = batch->screen;
   enum pipe_reset_status status = PIPE_NO_RESET;
   struct drm_i915_reset_stats stats = { .ctx_id = batch->hw_ctx_id };

   if (drmIoctl(screen->fd, DRM_IOCTL_I915_GET_RESET_STATS, &stats))
      DBG("DRM_IOCTL_I915_GET_RESET_STATS failed: %s\n", strerror(errno));

   if (stats.batch_active != 0) {
      /* A reset was observed while a batch from this hardware context was
       * executing.  Assume that this context was at fault.
       */
      status = PIPE_GUILTY_CONTEXT_RESET;
   } else if (stats.batch_pending != 0) {
      /* A reset was observed while a batch from this context was in progress,
       * but the batch was not executing.  In this case, assume that the
       * context was not at fault.
       */
      status = PIPE_INNOCENT_CONTEXT_RESET;
   }

   if (status != PIPE_NO_RESET) {
      /* Our context is likely banned, or at least in an unknown state.
       * Throw it away and start with a fresh context.  Ideally this may
       * catch the problem before our next execbuf fails with -EIO.
       */
      replace_hw_ctx(batch);
   }

   return status;
}

/**
 * Submit the batch to the GPU via execbuffer2.
 */
static int
submit_batch(struct crocus_batch *batch)
{

   if (batch->use_shadow_copy) {
      void *bo_map = crocus_bo_map(batch->dbg, batch->command.bo, MAP_WRITE);
      memcpy(bo_map, batch->command.map, crocus_batch_bytes_used(batch));

      bo_map = crocus_bo_map(batch->dbg, batch->state.bo, MAP_WRITE);
      memcpy(bo_map, batch->state.map, batch->state.used);
   }

   crocus_bo_unmap(batch->command.bo);
   crocus_bo_unmap(batch->state.bo);

   /* The requirement for using I915_EXEC_NO_RELOC are:
    *
    *   The addresses written in the objects must match the corresponding
    *   reloc.gtt_offset which in turn must match the corresponding
    *   execobject.offset.
    *
    *   Any render targets written to in the batch must be flagged with
    *   EXEC_OBJECT_WRITE.
    *
    *   To avoid stalling, execobject.offset should match the current
    *   address of that object within the active context.
    */
   /* Set statebuffer relocations */
   const unsigned state_index = batch->state.bo->index;
   if (state_index < batch->exec_count &&
       batch->exec_bos[state_index] == batch->state.bo) {
      struct drm_i915_gem_exec_object2 *entry =
         &batch->validation_list[state_index];
      assert(entry->handle == batch->state.bo->gem_handle);
      entry->relocation_count = batch->state.relocs.reloc_count;
      entry->relocs_ptr = (uintptr_t)batch->state.relocs.relocs;
   }

   /* Set batchbuffer relocations */
   struct drm_i915_gem_exec_object2 *entry = &batch->validation_list[0];
   assert(entry->handle == batch->command.bo->gem_handle);
   entry->relocation_count = batch->command.relocs.reloc_count;
   entry->relocs_ptr = (uintptr_t)batch->command.relocs.relocs;

   struct drm_i915_gem_execbuffer2 execbuf = {
      .buffers_ptr = (uintptr_t)batch->validation_list,
      .buffer_count = batch->exec_count,
      .batch_start_offset = 0,
      /* This must be QWord aligned. */
      .batch_len = ALIGN(batch->primary_batch_size, 8),
      .flags = I915_EXEC_RENDER |
               I915_EXEC_NO_RELOC |
               I915_EXEC_BATCH_FIRST |
               I915_EXEC_HANDLE_LUT,
      .rsvd1 = batch->hw_ctx_id, /* rsvd1 is actually the context ID */
   };

   if (num_fences(batch)) {
      execbuf.flags |= I915_EXEC_FENCE_ARRAY;
      execbuf.num_cliprects = num_fences(batch);
      execbuf.cliprects_ptr =
         (uintptr_t)util_dynarray_begin(&batch->exec_fences);
   }

   int ret = 0;
   if (!batch->screen->devinfo.no_hw &&
       intel_ioctl(batch->screen->fd, DRM_IOCTL_I915_GEM_EXECBUFFER2, &execbuf))
      ret = -errno;

   for (int i = 0; i < batch->exec_count; i++) {
      struct crocus_bo *bo = batch->exec_bos[i];

      bo->idle = false;
      bo->index = -1;

      /* Update brw_bo::gtt_offset */
      if (batch->validation_list[i].offset != bo->gtt_offset) {
         DBG("BO %d migrated: 0x%" PRIx64 " -> 0x%" PRIx64 "\n",
             bo->gem_handle, bo->gtt_offset,
             (uint64_t)batch->validation_list[i].offset);
         assert(!(bo->kflags & EXEC_OBJECT_PINNED));
         bo->gtt_offset = batch->validation_list[i].offset;
      }
   }

   return ret;
}

static const char *
batch_name_to_string(enum crocus_batch_name name)
{
   const char *names[CROCUS_BATCH_COUNT] = {
      [CROCUS_BATCH_RENDER] = "render",
      [CROCUS_BATCH_COMPUTE] = "compute",
   };
   return names[name];
}

/**
 * Flush the batch buffer, submitting it to the GPU and resetting it so
 * we're ready to emit the next batch.
 *
 * \param in_fence_fd is ignored if -1.  Otherwise, this function takes
 * ownership of the fd.
 *
 * \param out_fence_fd is ignored if NULL.  Otherwise, the caller must
 * take ownership of the returned fd.
 */
void
_crocus_batch_flush(struct crocus_batch *batch, const char *file, int line)
{
   struct crocus_screen *screen = batch->screen;

   /* If a fence signals we need to flush it. */
   if (crocus_batch_bytes_used(batch) == 0 && !batch->contains_fence_signal)
      return;

   assert(!batch->no_wrap);
   crocus_finish_batch(batch);

   finish_growing_bos(&batch->command);
   finish_growing_bos(&batch->state);
   int ret = submit_batch(batch);

   if (INTEL_DEBUG(DEBUG_BATCH | DEBUG_SUBMIT | DEBUG_PIPE_CONTROL)) {
      int bytes_for_commands = crocus_batch_bytes_used(batch);
      int second_bytes = 0;
      if (batch->command.bo != batch->exec_bos[0]) {
         second_bytes = bytes_for_commands;
         bytes_for_commands += batch->primary_batch_size;
      }
      fprintf(stderr, "%19s:%-3d: %s batch [%u] flush with %5d+%5db (%0.1f%%) "
              "(cmds), %4d BOs (%0.1fMb aperture),"
              " %4d command relocs, %4d state relocs\n",
              file, line, batch_name_to_string(batch->name), batch->hw_ctx_id,
              batch->primary_batch_size, second_bytes,
              100.0f * bytes_for_commands / BATCH_SZ,
              batch->exec_count,
              (float) batch->aperture_space / (1024 * 1024),
              batch->command.relocs.reloc_count,
              batch->state.relocs.reloc_count);

      if (INTEL_DEBUG(DEBUG_BATCH | DEBUG_SUBMIT)) {
         dump_fence_list(batch);
         dump_validation_list(batch);
      }

      if (INTEL_DEBUG(DEBUG_BATCH)) {
         decode_batch(batch);
      }
   }

   for (int i = 0; i < batch->exec_count; i++) {
      struct crocus_bo *bo = batch->exec_bos[i];
      crocus_bo_unreference(bo);
   }

   batch->command.relocs.reloc_count = 0;
   batch->state.relocs.reloc_count = 0;
   batch->exec_count = 0;
   batch->aperture_space = 0;

   util_dynarray_foreach(&batch->syncobjs, struct crocus_syncobj *, s)
      crocus_syncobj_reference(screen, s, NULL);
   util_dynarray_clear(&batch->syncobjs);

   util_dynarray_clear(&batch->exec_fences);

   if (INTEL_DEBUG(DEBUG_SYNC)) {
      dbg_printf("waiting for idle\n");
      crocus_bo_wait_rendering(batch->command.bo); /* if execbuf failed; this is a nop */
   }

   /* Start a new batch buffer. */
   crocus_batch_reset(batch);

   /* EIO means our context is banned.  In this case, try and replace it
    * with a new logical context, and inform crocus_context that all state
    * has been lost and needs to be re-initialized.  If this succeeds,
    * dubiously claim success...
    */
   if (ret == -EIO && replace_hw_ctx(batch)) {
      if (batch->reset->reset) {
         /* Tell the state tracker the device is lost and it was our fault. */
         batch->reset->reset(batch->reset->data, PIPE_GUILTY_CONTEXT_RESET);
      }

      ret = 0;
   }

   if (ret < 0) {
#ifdef DEBUG
      const bool color = INTEL_DEBUG(DEBUG_COLOR);
      fprintf(stderr, "%scrocus: Failed to submit batchbuffer: %-80s%s\n",
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
crocus_batch_references(struct crocus_batch *batch, struct crocus_bo *bo)
{
   return find_validation_entry(batch, bo) != NULL;
}

/**
 * Updates the state of the noop feature.  Returns true if there was a noop
 * transition that led to state invalidation.
 */
bool
crocus_batch_prepare_noop(struct crocus_batch *batch, bool noop_enable)
{
   if (batch->noop_enabled == noop_enable)
      return 0;

   batch->noop_enabled = noop_enable;

   crocus_batch_flush(batch);

   /* If the batch was empty, flush had no effect, so insert our noop. */
   if (crocus_batch_bytes_used(batch) == 0)
      crocus_batch_maybe_noop(batch);

   /* We only need to update the entire state if we transition from noop ->
    * not-noop.
    */
   return !batch->noop_enabled;
}
