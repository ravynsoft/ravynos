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
 * @file iris_pipe_control.c
 *
 * PIPE_CONTROL is the main flushing and synchronization primitive on Intel
 * GPUs.  It can invalidate caches, stall until rendering reaches various
 * stages of completion, write to memory, and other things.  In a way, it's
 * a swiss army knife command - it has all kinds of capabilities, but some
 * significant limitations as well.
 *
 * Unfortunately, it's notoriously complicated and difficult to use.  Many
 * sub-commands can't be used together.  Some are meant to be used at the
 * top of the pipeline (invalidating caches before drawing), while some are
 * meant to be used at the end (stalling or flushing after drawing).
 *
 * Also, there's a list of restrictions a mile long, which vary by generation.
 * Do this before doing that, or suffer the consequences (usually a GPU hang).
 *
 * This file contains helpers for emitting them safely.  You can simply call
 * iris_emit_pipe_control_flush() with the desired operations (as logical
 * PIPE_CONTROL_* bits), and it will take care of splitting it into multiple
 * PIPE_CONTROL commands as necessary.  The per-generation workarounds are
 * applied in iris_emit_raw_pipe_control() in iris_state.c.
 */

#include "iris_context.h"
#include "util/hash_table.h"
#include "util/set.h"

/**
 * Emit a PIPE_CONTROL with various flushing flags.
 *
 * The caller is responsible for deciding what flags are appropriate for the
 * given generation.
 */
void
iris_emit_pipe_control_flush(struct iris_batch *batch,
                             const char *reason,
                             uint32_t flags)
{
   if ((flags & PIPE_CONTROL_CACHE_FLUSH_BITS) &&
       (flags & PIPE_CONTROL_CACHE_INVALIDATE_BITS)) {
      /* A pipe control command with flush and invalidate bits set
       * simultaneously is an inherently racy operation on Gfx6+ if the
       * contents of the flushed caches were intended to become visible from
       * any of the invalidated caches.  Split it in two PIPE_CONTROLs, the
       * first one should stall the pipeline to make sure that the flushed R/W
       * caches are coherent with memory once the specified R/O caches are
       * invalidated.  On pre-Gfx6 hardware the (implicit) R/O cache
       * invalidation seems to happen at the bottom of the pipeline together
       * with any write cache flush, so this shouldn't be a concern.  In order
       * to ensure a full stall, we do an end-of-pipe sync.
       */
      iris_emit_end_of_pipe_sync(batch, reason,
                                 flags & PIPE_CONTROL_CACHE_FLUSH_BITS);
      flags &= ~(PIPE_CONTROL_CACHE_FLUSH_BITS | PIPE_CONTROL_CS_STALL);
   }

   batch->screen->vtbl.emit_raw_pipe_control(batch, reason, flags, NULL, 0, 0);
}

/**
 * Emit a PIPE_CONTROL that writes to a buffer object.
 *
 * \p flags should contain one of the following items:
 *  - PIPE_CONTROL_WRITE_IMMEDIATE
 *  - PIPE_CONTROL_WRITE_TIMESTAMP
 *  - PIPE_CONTROL_WRITE_DEPTH_COUNT
 */
void
iris_emit_pipe_control_write(struct iris_batch *batch,
                             const char *reason, uint32_t flags,
                             struct iris_bo *bo, uint32_t offset,
                             uint64_t imm)
{
   batch->screen->vtbl.emit_raw_pipe_control(batch, reason, flags, bo, offset, imm);
}

/*
 * From Sandybridge PRM, volume 2, "1.7.2 End-of-Pipe Synchronization":
 *
 *  Write synchronization is a special case of end-of-pipe
 *  synchronization that requires that the render cache and/or depth
 *  related caches are flushed to memory, where the data will become
 *  globally visible. This type of synchronization is required prior to
 *  SW (CPU) actually reading the result data from memory, or initiating
 *  an operation that will use as a read surface (such as a texture
 *  surface) a previous render target and/or depth/stencil buffer
 *
 * From Haswell PRM, volume 2, part 1, "End-of-Pipe Synchronization":
 *
 *  Exercising the write cache flush bits (Render Target Cache Flush
 *  Enable, Depth Cache Flush Enable, DC Flush) in PIPE_CONTROL only
 *  ensures the write caches are flushed and doesn't guarantee the data
 *  is globally visible.
 *
 *  SW can track the completion of the end-of-pipe-synchronization by
 *  using "Notify Enable" and "PostSync Operation - Write Immediate
 *  Data" in the PIPE_CONTROL command.
 */
void
iris_emit_end_of_pipe_sync(struct iris_batch *batch,
                           const char *reason, uint32_t flags)
{
   /* From Sandybridge PRM, volume 2, "1.7.3.1 Writing a Value to Memory":
    *
    *    "The most common action to perform upon reaching a synchronization
    *    point is to write a value out to memory. An immediate value
    *    (included with the synchronization command) may be written."
    *
    * From Broadwell PRM, volume 7, "End-of-Pipe Synchronization":
    *
    *    "In case the data flushed out by the render engine is to be read
    *    back in to the render engine in coherent manner, then the render
    *    engine has to wait for the fence completion before accessing the
    *    flushed data. This can be achieved by following means on various
    *    products: PIPE_CONTROL command with CS Stall and the required
    *    write caches flushed with Post-Sync-Operation as Write Immediate
    *    Data.
    *
    *    Example:
    *       - Workload-1 (3D/GPGPU/MEDIA)
    *       - PIPE_CONTROL (CS Stall, Post-Sync-Operation Write Immediate
    *         Data, Required Write Cache Flush bits set)
    *       - Workload-2 (Can use the data produce or output by Workload-1)
    */
   iris_emit_pipe_control_write(batch, reason,
                                flags | PIPE_CONTROL_CS_STALL |
                                PIPE_CONTROL_WRITE_IMMEDIATE,
                                batch->screen->workaround_address.bo,
                                batch->screen->workaround_address.offset, 0);
}

/**
 * Emits appropriate flushes and invalidations for any previous memory
 * operations on \p bo to be strictly ordered relative to any subsequent
 * memory operations performed from the caching domain \p access.
 *
 * This is useful because the GPU has separate incoherent caches for the
 * render target, sampler, etc., which need to be explicitly invalidated or
 * flushed in order to obtain the expected memory ordering in cases where the
 * same surface is accessed through multiple caches (e.g. due to
 * render-to-texture).
 *
 * This provides the expected memory ordering guarantees whether or not the
 * previous access was performed from the same batch or a different one, but
 * only the former case needs to be handled explicitly here, since the kernel
 * already inserts implicit flushes and synchronization in order to guarantee
 * that any data dependencies between batches are satisfied.
 *
 * Even though no flushing nor invalidation is required in order to account
 * for concurrent updates from other batches, we provide the guarantee that a
 * required synchronization operation due to a previous batch-local update
 * will never be omitted due to the influence of another thread accessing the
 * same buffer concurrently from the same caching domain: Such a concurrent
 * update will only ever change the seqno of the last update to a value
 * greater than the local value (see iris_bo_bump_seqno()), which means that
 * we will always emit at least as much flushing and invalidation as we would
 * have for the local seqno (see the coherent_seqnos comparisons below).
 */
void
iris_emit_buffer_barrier_for(struct iris_batch *batch,
                             struct iris_bo *bo,
                             enum iris_domain access)
{
   const struct intel_device_info *devinfo = batch->screen->devinfo;
   const struct brw_compiler *compiler = batch->screen->compiler;

   const bool access_via_l3 = iris_domain_is_l3_coherent(devinfo, access);

   const uint32_t all_flush_bits = (PIPE_CONTROL_CACHE_FLUSH_BITS |
                                    PIPE_CONTROL_STALL_AT_SCOREBOARD |
                                    PIPE_CONTROL_FLUSH_ENABLE);
   const uint32_t flush_bits[NUM_IRIS_DOMAINS] = {
      [IRIS_DOMAIN_RENDER_WRITE] = PIPE_CONTROL_RENDER_TARGET_FLUSH,
      [IRIS_DOMAIN_DEPTH_WRITE] = PIPE_CONTROL_DEPTH_CACHE_FLUSH,
      [IRIS_DOMAIN_DATA_WRITE] = PIPE_CONTROL_FLUSH_HDC,
      /* OTHER_WRITE includes "VF Cache Invalidate" to make sure that any
       * stream output writes are finished.  CS stall is added implicitly.
       */
      [IRIS_DOMAIN_OTHER_WRITE] = PIPE_CONTROL_FLUSH_ENABLE | PIPE_CONTROL_VF_CACHE_INVALIDATE,
      [IRIS_DOMAIN_VF_READ] = PIPE_CONTROL_STALL_AT_SCOREBOARD,
      [IRIS_DOMAIN_SAMPLER_READ] = PIPE_CONTROL_STALL_AT_SCOREBOARD,
      [IRIS_DOMAIN_PULL_CONSTANT_READ] = PIPE_CONTROL_STALL_AT_SCOREBOARD,
      [IRIS_DOMAIN_OTHER_READ] = PIPE_CONTROL_STALL_AT_SCOREBOARD,
   };
   const uint32_t invalidate_bits[NUM_IRIS_DOMAINS] = {
      [IRIS_DOMAIN_RENDER_WRITE] = PIPE_CONTROL_RENDER_TARGET_FLUSH,
      [IRIS_DOMAIN_DEPTH_WRITE] = PIPE_CONTROL_DEPTH_CACHE_FLUSH,
      [IRIS_DOMAIN_DATA_WRITE] = PIPE_CONTROL_FLUSH_HDC,
      [IRIS_DOMAIN_OTHER_WRITE] = PIPE_CONTROL_FLUSH_ENABLE,
      [IRIS_DOMAIN_VF_READ] = PIPE_CONTROL_VF_CACHE_INVALIDATE,
      [IRIS_DOMAIN_SAMPLER_READ] = PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE,
      [IRIS_DOMAIN_PULL_CONSTANT_READ] = PIPE_CONTROL_CONST_CACHE_INVALIDATE |
         (compiler->indirect_ubos_use_sampler ?
          PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE :
          PIPE_CONTROL_DATA_CACHE_FLUSH),
   };
   const uint32_t l3_flush_bits[NUM_IRIS_DOMAINS] = {
      [IRIS_DOMAIN_RENDER_WRITE] = PIPE_CONTROL_TILE_CACHE_FLUSH,
      [IRIS_DOMAIN_DEPTH_WRITE] = PIPE_CONTROL_TILE_CACHE_FLUSH,
      [IRIS_DOMAIN_DATA_WRITE] = PIPE_CONTROL_DATA_CACHE_FLUSH,
   };
   uint32_t bits = 0;

   /* Iterate over all read/write domains first in order to handle RaW
    * and WaW dependencies, which might involve flushing the domain of
    * the previous access and invalidating the specified domain.
    */
   for (unsigned i = 0; i < IRIS_DOMAIN_OTHER_WRITE; i++) {
      assert(!iris_domain_is_read_only(i));
      assert(iris_domain_is_l3_coherent(devinfo, i));

      if (i != access) {
         const uint64_t seqno = READ_ONCE(bo->last_seqnos[i]);

         /* Invalidate unless the most recent read/write access from
          * this domain is already guaranteed to be visible to the
          * specified domain.  Flush if the most recent access from
          * this domain occurred after its most recent flush.
          */
         if (seqno > batch->coherent_seqnos[access][i]) {
            bits |= invalidate_bits[access];

            if (access_via_l3) {
               /* Both domains share L3.  If the most recent read/write access
                * in domain `i' isn't visible to L3, then flush it to L3.
                */
               if (seqno > batch->l3_coherent_seqnos[i])
                  bits |= flush_bits[i];
            } else {
               /* Domain `i` is L3 coherent but the specified domain is not.
                * Flush both this cache and L3 out to memory.
                */
               if (seqno > batch->coherent_seqnos[i][i])
                  bits |= flush_bits[i] | l3_flush_bits[i];
            }
         }
      }
   }

   /* All read-only domains can be considered mutually coherent since
    * the order of read-only memory operations is immaterial.  If the
    * specified domain is read/write we need to iterate over them too,
    * in order to handle any WaR dependencies.
    */
   if (!iris_domain_is_read_only(access)) {
      for (unsigned i = IRIS_DOMAIN_VF_READ; i < NUM_IRIS_DOMAINS; i++) {
         assert(iris_domain_is_read_only(i));
         const uint64_t seqno = READ_ONCE(bo->last_seqnos[i]);

         const uint64_t last_visible_seqno =
            iris_domain_is_l3_coherent(devinfo, i) ?
            batch->l3_coherent_seqnos[i] : batch->coherent_seqnos[i][i];

         /* Flush if the most recent access from this domain occurred
          * after its most recent flush.
          */
         if (seqno > last_visible_seqno)
            bits |= flush_bits[i];
      }
   }

   /* The IRIS_DOMAIN_OTHER_WRITE kitchen-sink domain cannot be
    * considered coherent with itself since it's really a collection
    * of multiple incoherent read/write domains, so we special-case it
    * here.
    */
   const unsigned i = IRIS_DOMAIN_OTHER_WRITE;
   const uint64_t seqno = READ_ONCE(bo->last_seqnos[i]);

   assert(!iris_domain_is_l3_coherent(devinfo, i));

   /* Invalidate unless the most recent read/write access from this
    * domain is already guaranteed to be visible to the specified
    * domain.  Flush if the most recent access from this domain
    * occurred after its most recent flush.
    */
   if (seqno > batch->coherent_seqnos[access][i]) {
      bits |= invalidate_bits[access];

      /* There is a non-L3-coherent write that isn't visible to the
       * specified domain.  If the access is via L3, then it might see
       * stale L3 data that was loaded before that write.  In this case,
       * we try to invalidate all read-only sections of the L3 cache.
       */
      if (access_via_l3 && seqno > batch->l3_coherent_seqnos[i])
         bits |= PIPE_CONTROL_L3_RO_INVALIDATE_BITS;

      if (seqno > batch->coherent_seqnos[i][i])
         bits |= flush_bits[i];
   }

   if (bits) {
      /* Stall-at-scoreboard is not supported by the compute pipeline, use the
       * documented sequence of two PIPE_CONTROLs with PIPE_CONTROL_FLUSH_ENABLE
       * set in the second PIPE_CONTROL in order to obtain a similar effect.
       */
      const bool compute_stall_sequence = batch->name == IRIS_BATCH_COMPUTE &&
         (bits & PIPE_CONTROL_STALL_AT_SCOREBOARD) &&
         !(bits & PIPE_CONTROL_CACHE_FLUSH_BITS);

      /* Stall-at-scoreboard is not expected to work in combination with other
       * flush bits.
       */
      if (bits & PIPE_CONTROL_CACHE_FLUSH_BITS)
         bits &= ~PIPE_CONTROL_STALL_AT_SCOREBOARD;

      if (batch->name == IRIS_BATCH_COMPUTE)
         bits &= ~PIPE_CONTROL_GRAPHICS_BITS;

      /* Emit any required flushes and invalidations. */
      if ((bits & all_flush_bits) || compute_stall_sequence)
         iris_emit_end_of_pipe_sync(batch, "cache tracker: flush",
                                    bits & all_flush_bits);

      if ((bits & ~all_flush_bits) || compute_stall_sequence)
         iris_emit_pipe_control_flush(batch, "cache tracker: invalidate",
                                      (bits & ~all_flush_bits) |
                                      (compute_stall_sequence ?
                                       PIPE_CONTROL_FLUSH_ENABLE : 0));
   }
}

/**
 * Flush and invalidate all caches (for debugging purposes).
 */
void
iris_flush_all_caches(struct iris_batch *batch)
{
   iris_emit_pipe_control_flush(batch, "debug: flush all caches",
                                PIPE_CONTROL_CS_STALL |
                                PIPE_CONTROL_DATA_CACHE_FLUSH |
                                PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                PIPE_CONTROL_TILE_CACHE_FLUSH |
                                PIPE_CONTROL_VF_CACHE_INVALIDATE |
                                PIPE_CONTROL_INSTRUCTION_INVALIDATE |
                                PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
                                PIPE_CONTROL_CONST_CACHE_INVALIDATE |
                                PIPE_CONTROL_STATE_CACHE_INVALIDATE);
}

static void
iris_texture_barrier(struct pipe_context *ctx, unsigned flags)
{
   struct iris_context *ice = (void *) ctx;
   struct iris_batch *render_batch = &ice->batches[IRIS_BATCH_RENDER];
   struct iris_batch *compute_batch = &ice->batches[IRIS_BATCH_COMPUTE];

   if (render_batch->contains_draw) {
      iris_batch_maybe_flush(render_batch, 48);
      iris_emit_pipe_control_flush(render_batch,
                                   "API: texture barrier (1/2)",
                                   PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                   PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                   PIPE_CONTROL_CS_STALL);
      iris_emit_pipe_control_flush(render_batch,
                                   "API: texture barrier (2/2)",
                                   PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE);
   }

   if (compute_batch->contains_draw) {
      iris_batch_maybe_flush(compute_batch, 48);
      iris_emit_pipe_control_flush(compute_batch,
                                   "API: texture barrier (1/2)",
                                   PIPE_CONTROL_CS_STALL);
      iris_emit_pipe_control_flush(compute_batch,
                                   "API: texture barrier (2/2)",
                                   PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE);
   }
}

static void
iris_memory_barrier(struct pipe_context *ctx, unsigned flags)
{
   struct iris_context *ice = (void *) ctx;
   unsigned bits = PIPE_CONTROL_DATA_CACHE_FLUSH | PIPE_CONTROL_CS_STALL;

   if (flags & (PIPE_BARRIER_VERTEX_BUFFER |
                PIPE_BARRIER_INDEX_BUFFER |
                PIPE_BARRIER_INDIRECT_BUFFER)) {
      bits |= PIPE_CONTROL_VF_CACHE_INVALIDATE;
   }

   if (flags & PIPE_BARRIER_CONSTANT_BUFFER) {
      bits |= PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
              PIPE_CONTROL_CONST_CACHE_INVALIDATE;
   }

   if (flags & PIPE_BARRIER_TEXTURE)
      bits |= PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE;

   if (flags & PIPE_BARRIER_FRAMEBUFFER) {
      /* The caller may have issued a render target read and a data cache data
       * port write in the same draw call. Depending on the hardware, iris
       * performs render target reads with either the sampler or the render
       * cache data port. If the next framebuffer access is a render target
       * read, the previously affected caches must be invalidated.
       */
      bits |= PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
              PIPE_CONTROL_RENDER_TARGET_FLUSH;
   }

   iris_foreach_batch(ice, batch) {
      const unsigned allowed_bits =
         batch->name == IRIS_BATCH_COMPUTE ? ~PIPE_CONTROL_GRAPHICS_BITS : ~0u;

      if (batch->contains_draw) {
         iris_batch_maybe_flush(batch, 24);
         iris_emit_pipe_control_flush(batch,
                                      "API: memory barrier",
                                      bits & allowed_bits);
      }
   }
}

void
iris_init_flush_functions(struct pipe_context *ctx)
{
   ctx->memory_barrier = iris_memory_barrier;
   ctx->texture_barrier = iris_texture_barrier;
}
