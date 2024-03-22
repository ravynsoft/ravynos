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

#ifndef CROCUS_BATCH_DOT_H
#define CROCUS_BATCH_DOT_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "util/u_dynarray.h"

#include "common/intel_decoder.h"
#include "drm-uapi/i915_drm.h"

#include "crocus_fence.h"
#include "crocus_fine_fence.h"

#include "crocus_bufmgr.h"
/* The kernel assumes batchbuffers are smaller than 256kB. */
#define MAX_BATCH_SIZE (256 * 1024)

/* 3DSTATE_BINDING_TABLE_POINTERS has a U16 offset from Surface State Base
 * Address, which means that we can't put binding tables beyond 64kB.  This
 * effectively limits the maximum statebuffer size to 64kB.
 */
#define MAX_STATE_SIZE (64 * 1024)

/* Our target batch size - flush approximately at this point. */
#define BATCH_SZ (20 * 1024)
#define STATE_SZ (16 * 1024)

enum crocus_batch_name {
   CROCUS_BATCH_RENDER,
   CROCUS_BATCH_COMPUTE,
};

#define CROCUS_BATCH_COUNT 2

struct crocus_address {
   struct crocus_bo *bo;
   int32_t offset;
   uint32_t reloc_flags;
};

struct crocus_reloc_list {
   struct drm_i915_gem_relocation_entry *relocs;
   int reloc_count;
   int reloc_array_size;
};

struct crocus_growing_bo {
   struct crocus_bo *bo;
   void *map;
   void *map_next;
   struct crocus_bo *partial_bo;
   void *partial_bo_map;
   unsigned partial_bytes;
   struct crocus_reloc_list relocs;
   unsigned used;
};

struct crocus_batch {
   struct crocus_context *ice;
   struct crocus_screen *screen;
   struct util_debug_callback *dbg;
   struct pipe_device_reset_callback *reset;

   /** What batch is this? (e.g. CROCUS_BATCH_RENDER/COMPUTE) */
   enum crocus_batch_name name;

   /** buffers: command, state */
   struct crocus_growing_bo command, state;

   /** Size of the primary batch if we've moved on to a secondary. */
   unsigned primary_batch_size;

   bool state_base_address_emitted;
   uint8_t pipe_controls_since_last_cs_stall;

   uint32_t hw_ctx_id;

   uint32_t valid_reloc_flags;

   bool use_shadow_copy;
   bool no_wrap;

   /** The validation list */
   struct drm_i915_gem_exec_object2 *validation_list;
   struct crocus_bo **exec_bos;
   int exec_count;
   int exec_array_size;

   /** Whether INTEL_BLACKHOLE_RENDER is enabled in the batch (aka first
    * instruction is a MI_BATCH_BUFFER_END).
    */
   bool noop_enabled;

   /**
    * A list of crocus_syncobjs associated with this batch.
    *
    * The first list entry will always be a signalling sync-point, indicating
    * that this batch has completed.  The others are likely to be sync-points
    * to wait on before executing the batch.
    */
   struct util_dynarray syncobjs;

   /** A list of drm_i915_exec_fences to have execbuf signal or wait on */
   struct util_dynarray exec_fences;

   /** The amount of aperture space (in bytes) used by all exec_bos */
   int aperture_space;

   struct {
      /** Uploader to use for sequence numbers */
      struct u_upload_mgr *uploader;

      /** GPU buffer and CPU map where our seqno's will be written. */
      struct crocus_state_ref ref;
      uint32_t *map;

      /** The sequence number to write the next time we add a fence. */
      uint32_t next;
   } fine_fences;

   /** A seqno (and syncobj) for the last batch that was submitted. */
   struct crocus_fine_fence *last_fence;

   /** List of other batches which we might need to flush to use a BO */
   struct crocus_batch *other_batches[CROCUS_BATCH_COUNT - 1];

   struct {
      /**
       * Set of struct brw_bo * that have been rendered to within this
       * batchbuffer and would need flushing before being used from another
       * cache domain that isn't coherent with it (i.e. the sampler).
       */
      struct hash_table *render;

      /**
       * Set of struct brw_bo * that have been used as a depth buffer within
       * this batchbuffer and would need flushing before being used from
       * another cache domain that isn't coherent with it (i.e. the sampler).
       */
      struct set *depth;
   } cache;

   struct intel_batch_decode_ctx decoder;
   struct hash_table_u64 *state_sizes;

   /** Have we emitted any draw calls to this batch? */
   bool contains_draw;

   /** Batch contains fence signal operation. */
   bool contains_fence_signal;
};

static inline bool
batch_has_fine_fence(struct crocus_batch *batch)
{
   return !!batch->fine_fences.uploader;
}

#define BATCH_HAS_FINE_FENCES(batch) (!!(batch)->fine_fences.uploader)
void crocus_init_batch(struct crocus_context *ctx,
                       enum crocus_batch_name name,
                       int priority);
void crocus_batch_free(struct crocus_batch *batch);
void crocus_batch_maybe_flush(struct crocus_batch *batch, unsigned estimate);

void _crocus_batch_flush(struct crocus_batch *batch, const char *file, int line);
#define crocus_batch_flush(batch) _crocus_batch_flush((batch), __FILE__, __LINE__)

bool crocus_batch_references(struct crocus_batch *batch, struct crocus_bo *bo);

bool crocus_batch_prepare_noop(struct crocus_batch *batch, bool noop_enable);

#define RELOC_WRITE EXEC_OBJECT_WRITE
#define RELOC_NEEDS_GGTT EXEC_OBJECT_NEEDS_GTT
/* Inverted meaning, but using the same bit...emit_reloc will flip it. */
#define RELOC_32BIT EXEC_OBJECT_SUPPORTS_48B_ADDRESS

void crocus_use_pinned_bo(struct crocus_batch *batch, struct crocus_bo *bo,
                          bool writable);
uint64_t crocus_command_reloc(struct crocus_batch *batch, uint32_t batch_offset,
                              struct crocus_bo *target, uint32_t target_offset,
                              unsigned int reloc_flags);
uint64_t crocus_state_reloc(struct crocus_batch *batch, uint32_t batch_offset,
                            struct crocus_bo *target, uint32_t target_offset,
                            unsigned int reloc_flags);

enum pipe_reset_status crocus_batch_check_for_reset(struct crocus_batch *batch);

void crocus_grow_buffer(struct crocus_batch *batch, bool grow_state,
                        unsigned used, unsigned new_size);

static inline unsigned
crocus_batch_bytes_used(struct crocus_batch *batch)
{
   return batch->command.map_next - batch->command.map;
}

/**
 * Ensure the current command buffer has \param size bytes of space
 * remaining.  If not, this creates a secondary batch buffer and emits
 * a jump from the primary batch to the start of the secondary.
 *
 * Most callers want crocus_get_command_space() instead.
 */
static inline void
crocus_require_command_space(struct crocus_batch *batch, unsigned size)
{
   const unsigned required_bytes = crocus_batch_bytes_used(batch) + size;
   unsigned used = crocus_batch_bytes_used(batch);
   if (required_bytes >= BATCH_SZ && !batch->no_wrap) {
      crocus_batch_flush(batch);
   } else if (used + size >= batch->command.bo->size) {
      const unsigned new_size =
         MIN2(batch->command.bo->size + batch->command.bo->size / 2,
              MAX_BATCH_SIZE);

      crocus_grow_buffer(batch, false, used, new_size);
      batch->command.map_next = (void *)batch->command.map + used;
      assert(crocus_batch_bytes_used(batch) + size < batch->command.bo->size);
   }
}

/**
 * Allocate space in the current command buffer, and return a pointer
 * to the mapped area so the caller can write commands there.
 *
 * This should be called whenever emitting commands.
 */
static inline void *
crocus_get_command_space(struct crocus_batch *batch, unsigned bytes)
{
   crocus_require_command_space(batch, bytes);
   void *map = batch->command.map_next;
   batch->command.map_next += bytes;
   return map;
}

/**
 * Helper to emit GPU commands - allocates space, copies them there.
 */
static inline void
crocus_batch_emit(struct crocus_batch *batch, const void *data, unsigned size)
{
   void *map = crocus_get_command_space(batch, size);
   memcpy(map, data, size);
}

/**
 * Get a pointer to the batch's signalling syncobj.  Does not refcount.
 */
static inline struct crocus_syncobj *
crocus_batch_get_signal_syncobj(struct crocus_batch *batch)
{
   /* The signalling syncobj is the first one in the list. */
   struct crocus_syncobj *syncobj =
      ((struct crocus_syncobj **)util_dynarray_begin(&batch->syncobjs))[0];
   return syncobj;
}

/**
 * Take a reference to the batch's signalling syncobj.
 *
 * Callers can use this to wait for the the current batch under construction
 * to complete (after flushing it).
 */
static inline void
crocus_batch_reference_signal_syncobj(struct crocus_batch *batch,
                                      struct crocus_syncobj **out_syncobj)
{
   struct crocus_syncobj *syncobj = crocus_batch_get_signal_syncobj(batch);
   crocus_syncobj_reference(batch->screen, out_syncobj, syncobj);
}

/**
 * Record the size of a piece of state for use in INTEL_DEBUG=bat printing.
 */
static inline void
crocus_record_state_size(struct hash_table_u64 *ht, uint32_t offset_from_base,
                         uint32_t size)
{
   if (ht) {
      _mesa_hash_table_u64_insert(ht, offset_from_base,
                                  (void *)(uintptr_t)size);
   }
}

static inline bool
crocus_ptr_in_state_buffer(struct crocus_batch *batch, void *p)
{
   return (char *)p >= (char *)batch->state.map &&
          (char *)p < (char *)batch->state.map + batch->state.bo->size;
}

static inline void
crocus_require_statebuffer_space(struct crocus_batch *batch, int size)
{
   if (batch->state.used + size >= STATE_SZ)
      crocus_batch_flush(batch);
}
#endif
